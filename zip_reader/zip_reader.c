/*
 * zip_reader.c — Pure C zip archive reader.
 *
 * Parses the End of Central Directory record, reads the central directory,
 * and extracts entries with STORED or DEFLATE compression using zlib.
 */
#include "zip_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

/* Zip format signatures */
#define ZIP_EOCD_SIG        0x06054b50
#define ZIP_CDIR_SIG        0x02014b50
#define ZIP_LOCAL_SIG       0x04034b50
#define ZIP_EOCD_MIN_SIZE   22
#define ZIP_EOCD_MAX_SEARCH 65557  /* 64k comment + EOCD size */

/* Read a little-endian uint16 from buffer */
static uint16_t
read_u16(const unsigned char *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

/* Read a little-endian uint32 from buffer */
static uint32_t
read_u32(const unsigned char *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* Find the End of Central Directory record by scanning backwards */
static long
find_eocd(FILE *fp)
{
    unsigned char buf[ZIP_EOCD_MAX_SEARCH];
    long file_size, search_start, search_len;
    long i;

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);

    search_len = file_size < ZIP_EOCD_MAX_SEARCH ? file_size : ZIP_EOCD_MAX_SEARCH;
    search_start = file_size - search_len;

    fseek(fp, search_start, SEEK_SET);
    if (fread(buf, 1, search_len, fp) != (size_t)search_len)
        return -1;

    /* Scan backwards for the EOCD signature */
    for (i = search_len - ZIP_EOCD_MIN_SIZE; i >= 0; i--) {
        if (read_u32(buf + i) == ZIP_EOCD_SIG)
            return search_start + i;
    }

    return -1;
}


zip_archive_t *
zip_open(const char *path)
{
    FILE *fp;
    zip_archive_t *za;
    unsigned char eocd_buf[ZIP_EOCD_MIN_SIZE];
    long eocd_offset;
    uint32_t cdir_size, cdir_offset, num_entries;
    unsigned char *cdir_buf;
    uint32_t i;

    fp = fopen(path, "rb");
    if (fp == NULL) return NULL;

    /* Find and read EOCD */
    eocd_offset = find_eocd(fp);
    if (eocd_offset < 0) {
        fclose(fp);
        return NULL;
    }

    fseek(fp, eocd_offset, SEEK_SET);
    if (fread(eocd_buf, 1, ZIP_EOCD_MIN_SIZE, fp) != ZIP_EOCD_MIN_SIZE) {
        fclose(fp);
        return NULL;
    }

    num_entries = read_u16(eocd_buf + 10);  /* total entries */
    cdir_size = read_u32(eocd_buf + 12);    /* central dir size */
    cdir_offset = read_u32(eocd_buf + 16);  /* central dir offset */

    /* Read central directory */
    cdir_buf = (unsigned char *)malloc(cdir_size);
    if (cdir_buf == NULL) {
        fclose(fp);
        return NULL;
    }

    fseek(fp, cdir_offset, SEEK_SET);
    if (fread(cdir_buf, 1, cdir_size, fp) != cdir_size) {
        free(cdir_buf);
        fclose(fp);
        return NULL;
    }

    /* Allocate archive */
    za = (zip_archive_t *)calloc(1, sizeof(zip_archive_t));
    if (za == NULL) {
        free(cdir_buf);
        fclose(fp);
        return NULL;
    }
    za->fp = fp;
    za->num_entries = num_entries;
    za->entries = (zip_entry_t *)calloc(num_entries, sizeof(zip_entry_t));
    if (za->entries == NULL) {
        free(cdir_buf);
        free(za);
        fclose(fp);
        return NULL;
    }

    /* Parse central directory entries */
    {
        unsigned char *p = cdir_buf;
        for (i = 0; i < num_entries; i++) {
            uint16_t fname_len, extra_len, comment_len, compression;
            uint32_t comp_size, uncomp_size, offset;

            if (read_u32(p) != ZIP_CDIR_SIG)
                break;

            compression = read_u16(p + 10);
            comp_size = read_u32(p + 20);
            uncomp_size = read_u32(p + 24);
            fname_len = read_u16(p + 28);
            extra_len = read_u16(p + 30);
            comment_len = read_u16(p + 32);
            offset = read_u32(p + 42);

            za->entries[i].compression = compression;
            za->entries[i].compressed_size = comp_size;
            za->entries[i].uncompressed_size = uncomp_size;
            za->entries[i].local_header_offset = offset;

            za->entries[i].filename = (char *)malloc(fname_len + 1);
            if (za->entries[i].filename != NULL) {
                memcpy(za->entries[i].filename, p + 46, fname_len);
                za->entries[i].filename[fname_len] = '\0';
            }

            p += 46 + fname_len + extra_len + comment_len;
        }
        za->num_entries = i;  /* actual count parsed */
    }

    free(cdir_buf);
    return za;
}


void
zip_close(zip_archive_t *za)
{
    uint32_t i;
    if (za == NULL) return;

    for (i = 0; i < za->num_entries; i++)
        free(za->entries[i].filename);

    free(za->entries);
    if (za->fp) fclose(za->fp);
    free(za);
}


const zip_entry_t *
zip_find(const zip_archive_t *za, const char *filename)
{
    uint32_t i;
    for (i = 0; i < za->num_entries; i++) {
        if (za->entries[i].filename != NULL &&
            strcmp(za->entries[i].filename, filename) == 0)
            return &za->entries[i];
    }
    return NULL;
}


unsigned char *
zip_read(zip_archive_t *za, const zip_entry_t *entry, size_t *out_len)
{
    unsigned char local_hdr[30];
    uint16_t fname_len, extra_len;
    long data_offset;
    unsigned char *comp_data = NULL;
    unsigned char *result = NULL;

    /* Read local file header to get actual data offset */
    fseek(za->fp, entry->local_header_offset, SEEK_SET);
    if (fread(local_hdr, 1, 30, za->fp) != 30)
        return NULL;

    if (read_u32(local_hdr) != ZIP_LOCAL_SIG)
        return NULL;

    fname_len = read_u16(local_hdr + 26);
    extra_len = read_u16(local_hdr + 28);
    data_offset = entry->local_header_offset + 30 + fname_len + extra_len;

    /* Read compressed data */
    comp_data = (unsigned char *)malloc(entry->compressed_size);
    if (comp_data == NULL) return NULL;

    fseek(za->fp, data_offset, SEEK_SET);
    if (fread(comp_data, 1, entry->compressed_size, za->fp)
        != entry->compressed_size) {
        free(comp_data);
        return NULL;
    }

    if (entry->compression == 0) {
        /* Stored — no decompression needed */
        *out_len = entry->compressed_size;
        return comp_data;
    } else if (entry->compression == 8) {
        /* Deflated — use zlib */
        z_stream strm;
        int ret;

        result = (unsigned char *)malloc(entry->uncompressed_size);
        if (result == NULL) {
            free(comp_data);
            return NULL;
        }

        memset(&strm, 0, sizeof(strm));
        strm.next_in = comp_data;
        strm.avail_in = entry->compressed_size;
        strm.next_out = result;
        strm.avail_out = entry->uncompressed_size;

        /* -MAX_WBITS for raw deflate (no zlib/gzip header) */
        ret = inflateInit2(&strm, -MAX_WBITS);
        if (ret != Z_OK) {
            free(comp_data);
            free(result);
            return NULL;
        }

        ret = inflate(&strm, Z_FINISH);
        inflateEnd(&strm);
        free(comp_data);

        if (ret != Z_STREAM_END) {
            free(result);
            return NULL;
        }

        *out_len = entry->uncompressed_size;
        return result;
    }

    /* Unsupported compression */
    free(comp_data);
    return NULL;
}


uint32_t
zip_count(const zip_archive_t *za)
{
    return za->num_entries;
}


const zip_entry_t *
zip_get(const zip_archive_t *za, uint32_t index)
{
    if (index >= za->num_entries) return NULL;
    return &za->entries[index];
}
