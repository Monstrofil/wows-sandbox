/*
 * wows_decrypt.h — Decrypt WoWS obfuscated .pyc files.
 */
#ifndef WOWS_DECRYPT_H
#define WOWS_DECRYPT_H

#include "Python.h"

/*
 * Decrypt a WoWS .pyc buffer through all 4 obfuscation stages.
 * Returns a new reference to a PyCodeObject, or NULL on error.
 *
 * pyc_data: raw bytes of the .pyc file
 * pyc_len:  length of pyc_data
 */
PyObject *wows_decrypt_pyc(const unsigned char *pyc_data, Py_ssize_t pyc_len);

#endif /* WOWS_DECRYPT_H */
