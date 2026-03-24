/*
 * wows_decrypt.c — Pure C implementation of WoWS 4-stage .pyc decryption.
 *
 * Only dependency on Python: PyMarshal_ReadObjectFromString() for
 * unmarshalling code objects. All crypto, base64, zlib done in C.
 */
#include "wows_decrypt.h"
#include "marshal.h"
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

/* ── Base64 decoder ──────────────────────────────────────────────────── */

static const unsigned char b64_table[256] = {
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
    52,53,54,55,56,57,58,59,60,61,64,64,64,65,64,64,
    64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
    15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,
    64,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
    41,42,43,44,45,46,47,48,49,50,51,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
};

/*
 * Base64 decode. Skips whitespace/newlines.
 * Caller must free() the result. Sets *out_len.
 */
static unsigned char *
b64_decode(const unsigned char *in, size_t in_len, size_t *out_len)
{
    /* Upper bound: 3 output bytes per 4 input */
    unsigned char *out = (unsigned char *)malloc((in_len / 4 + 1) * 3);
    size_t o = 0;
    unsigned int buf = 0;
    int nbits = 0;
    size_t i;

    if (out == NULL) return NULL;

    for (i = 0; i < in_len; i++) {
        unsigned char c = in[i];
        unsigned char v = b64_table[c];
        if (v == 64) continue;  /* skip invalid (whitespace, etc.) */
        if (v == 65) break;     /* '=' padding — stop */
        buf = (buf << 6) | v;
        nbits += 6;
        if (nbits >= 8) {
            nbits -= 8;
            out[o++] = (unsigned char)(buf >> nbits);
            buf &= (1 << nbits) - 1;
        }
    }

    *out_len = o;
    return out;
}


/* ── Zlib decompress (raw or auto-detect) ────────────────────────────── */

/*
 * Decompress zlib-wrapped data. Caller must free() result.
 * Returns NULL on error.
 */
static unsigned char *
zlib_decompress(const unsigned char *in, size_t in_len, size_t *out_len)
{
    z_stream strm;
    unsigned char *out = NULL;
    size_t out_alloc = in_len * 4;  /* initial guess */
    int ret;

    if (out_alloc < 4096) out_alloc = 4096;

    memset(&strm, 0, sizeof(strm));
    /* 15 + 32 = auto-detect zlib or gzip header */
    ret = inflateInit2(&strm, 15 + 32);
    if (ret != Z_OK) return NULL;

    strm.next_in = (Bytef *)in;
    strm.avail_in = (uInt)in_len;

    out = (unsigned char *)malloc(out_alloc);
    if (out == NULL) { inflateEnd(&strm); return NULL; }

    strm.next_out = out;
    strm.avail_out = (uInt)out_alloc;

    while (1) {
        ret = inflate(&strm, Z_NO_FLUSH);
        if (ret == Z_STREAM_END)
            break;
        if (ret != Z_OK && ret != Z_BUF_ERROR) {
            free(out);
            inflateEnd(&strm);
            return NULL;
        }
        if (strm.avail_out == 0) {
            size_t new_alloc = out_alloc * 2;
            unsigned char *new_out = (unsigned char *)realloc(out, new_alloc);
            if (new_out == NULL) {
                free(out);
                inflateEnd(&strm);
                return NULL;
            }
            out = new_out;
            strm.next_out = out + out_alloc;
            strm.avail_out = (uInt)(new_alloc - out_alloc);
            out_alloc = new_alloc;
        }
    }

    *out_len = strm.total_out;
    inflateEnd(&strm);
    return out;
}


/* ── Helpers ─────────────────────────────────────────────────────────── */

static void
reverse_bytes(unsigned char *buf, size_t len)
{
    size_t i;
    for (i = 0; i < len / 2; i++) {
        unsigned char tmp = buf[i];
        buf[i] = buf[len - 1 - i];
        buf[len - 1 - i] = tmp;
    }
}

static Py_ssize_t
find_bytes(const unsigned char *hay, size_t hlen,
           const unsigned char *needle, size_t nlen)
{
    size_t i;
    if (nlen > hlen) return -1;
    for (i = 0; i <= hlen - nlen; i++) {
        if (memcmp(hay + i, needle, nlen) == 0)
            return (Py_ssize_t)i;
    }
    return -1;
}


/* ── Main decryption pipeline ────────────────────────────────────────── */

PyObject *
wows_decrypt_pyc(const unsigned char *pyc_data, Py_ssize_t pyc_len)
{
    PyObject *outer_code = NULL;
    PyObject *inner_code = NULL;
    PyObject *stage3_code = NULL;
    PyObject *final_code = NULL;

    /* Skip 8-byte .pyc header, unmarshal outer code object */
    if (pyc_len < 8) {
        PyErr_SetString(PyExc_ValueError, "pyc data too short");
        return NULL;
    }
    outer_code = PyMarshal_ReadObjectFromString((char *)(pyc_data + 8),
                                                pyc_len - 8);
    if (outer_code == NULL || !PyCode_Check(outer_code))
        goto error;

    {
        /* key = outer_code.co_code (the XOR key and cipher input) */
        const unsigned char *key = (const unsigned char *)
            PyString_AS_STRING(((PyCodeObject *)outer_code)->co_code);
        Py_ssize_t key_len =
            PyString_GET_SIZE(((PyCodeObject *)outer_code)->co_code);

        /* ── Stage 1: XOR → base64 decode → zlib decompress → unmarshal ── */
        {
            PyObject *co_consts = ((PyCodeObject *)outer_code)->co_consts;
            Py_ssize_t nconsts = PyTuple_GET_SIZE(co_consts);
            PyObject *enc_obj = PyTuple_GET_ITEM(co_consts, nconsts - 1);
            const unsigned char *enc = (const unsigned char *)PyString_AS_STRING(enc_obj);
            Py_ssize_t enc_len = PyString_GET_SIZE(enc_obj);

            /* XOR with repeating key */
            unsigned char *xored = (unsigned char *)malloc(enc_len);
            if (!xored) { PyErr_NoMemory(); goto error; }
            {
                Py_ssize_t i;
                for (i = 0; i < enc_len; i++)
                    xored[i] = enc[i] ^ key[i % key_len];
            }

            /* Base64 decode */
            size_t b64_len;
            unsigned char *b64_out = b64_decode(xored, enc_len, &b64_len);
            free(xored);
            if (!b64_out) { PyErr_SetString(PyExc_ValueError, "b64 decode failed"); goto error; }

            /* Zlib decompress */
            size_t decomp_len;
            unsigned char *decomp = zlib_decompress(b64_out, b64_len, &decomp_len);
            free(b64_out);
            if (!decomp) { PyErr_SetString(PyExc_ValueError, "zlib decompress failed at stage 1"); goto error; }

            /* Unmarshal → inner_code */
            inner_code = PyMarshal_ReadObjectFromString((char *)decomp, decomp_len);
            free(decomp);
            if (!inner_code || !PyCode_Check(inner_code)) goto error;
        }

        /* ── Stage 2+3: swap_map substitution + bit cipher → unmarshal ── */
        {
            PyObject *inner_consts = ((PyCodeObject *)inner_code)->co_consts;
            PyObject *f123 = PyTuple_GET_ITEM(inner_consts, 8);
            if (!PyCode_Check(f123)) {
                PyErr_SetString(PyExc_ValueError, "stage2: co_consts[8] not code");
                goto error;
            }
            /* swap_map can be a dict {int→int} or a tuple/list */
            PyObject *swap_map = PyTuple_GET_ITEM(
                ((PyCodeObject *)f123)->co_consts, 1);

            unsigned char *ciphered = (unsigned char *)malloc(key_len);
            if (!ciphered) { PyErr_NoMemory(); goto error; }

            {
                Py_ssize_t i;
                int is_dict = PyDict_Check(swap_map);
                for (i = 0; i < key_len; i++) {
                    long mapped;
                    if (is_dict) {
                        PyObject *k = PyInt_FromLong(key[i]);
                        PyObject *v = PyDict_GetItem(swap_map, k);
                        Py_DECREF(k);
                        mapped = (v != NULL) ? PyInt_AsLong(v) : 0;
                    } else {
                        mapped = PyInt_AsLong(
                            PySequence_GetItem(swap_map, key[i]));
                    }
                    unsigned char vv = (unsigned char)(mapped ^ 38);
                    vv = ((vv & 126) | ((vv >> 7) & 1) | ((vv & 1) << 7));
                    ciphered[i] = vv ^ 89;
                }
            }

            /* Reverse */
            reverse_bytes(ciphered, key_len);

            /* Unmarshal → stage3_code */
            stage3_code = PyMarshal_ReadObjectFromString(
                (char *)ciphered, key_len);
            free(ciphered);
            if (!stage3_code || !PyCode_Check(stage3_code)) goto error;
        }

        /* ── Stage 4: split by <<<>>>, reverse, base64, zlib, unmarshal ── */
        {
            const unsigned char *bc = (const unsigned char *)
                PyString_AS_STRING(((PyCodeObject *)stage3_code)->co_code);
            Py_ssize_t bc_len =
                PyString_GET_SIZE(((PyCodeObject *)stage3_code)->co_code);

            static const unsigned char delim[] = "<<<>>>";
            Py_ssize_t pos = find_bytes(bc, bc_len, delim, 6);
            if (pos < 0) {
                PyErr_SetString(PyExc_ValueError, "<<<>>> not found");
                goto error;
            }

            const unsigned char *after = bc + pos + 6;
            Py_ssize_t after_len = bc_len - pos - 6;

            /* Find second delimiter (payload ends there or at end) */
            Py_ssize_t pos2 = find_bytes(after, after_len, delim, 6);
            Py_ssize_t payload_len = (pos2 >= 0) ? pos2 : after_len;

            /* Reverse the payload */
            unsigned char *payload = (unsigned char *)malloc(payload_len);
            if (!payload) { PyErr_NoMemory(); goto error; }
            {
                Py_ssize_t i;
                for (i = 0; i < payload_len; i++)
                    payload[i] = after[payload_len - 1 - i];
            }

            /* Base64 decode */
            size_t b64_len;
            unsigned char *b64_out = b64_decode(payload, payload_len, &b64_len);
            free(payload);
            if (!b64_out) {
                PyErr_SetString(PyExc_ValueError, "b64 decode failed at stage 4");
                goto error;
            }

            /* Zlib decompress */
            size_t decomp_len;
            unsigned char *decomp = zlib_decompress(b64_out, b64_len, &decomp_len);
            free(b64_out);
            if (!decomp) {
                PyErr_SetString(PyExc_ValueError, "zlib decompress failed at stage 4");
                goto error;
            }

            /* Unmarshal → final code object */
            final_code = PyMarshal_ReadObjectFromString(
                (char *)decomp, decomp_len);
            free(decomp);
            if (!final_code || !PyCode_Check(final_code)) goto error;
        }
    }

    Py_XDECREF(outer_code);
    Py_XDECREF(inner_code);
    Py_XDECREF(stage3_code);
    return final_code;

error:
    Py_XDECREF(outer_code);
    Py_XDECREF(inner_code);
    Py_XDECREF(stage3_code);
    Py_XDECREF(final_code);
    return NULL;
}
