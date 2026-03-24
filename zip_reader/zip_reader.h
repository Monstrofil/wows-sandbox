/*
 * zip_reader.h — Pure C zip archive reader (read-only).
 * Uses zlib for DEFLATE decompression.
 */
#ifndef ZIP_READER_H
#define ZIP_READER_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

typedef struct zip_entry {
    char *filename;           /* null-terminated filename */
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint32_t local_header_offset;
    uint16_t compression;     /* 0=stored, 8=deflated */
} zip_entry_t;

typedef struct zip_archive {
    FILE *fp;
    zip_entry_t *entries;
    uint32_t num_entries;
} zip_archive_t;

/* Open a zip archive. Returns NULL on error. */
zip_archive_t *zip_open(const char *path);

/* Close and free a zip archive. */
void zip_close(zip_archive_t *za);

/* Find an entry by filename. Returns NULL if not found. */
const zip_entry_t *zip_find(const zip_archive_t *za, const char *filename);

/*
 * Read and decompress an entry's data.
 * Caller must free() the returned buffer.
 * Sets *out_len to the uncompressed size.
 * Returns NULL on error.
 */
unsigned char *zip_read(zip_archive_t *za, const zip_entry_t *entry,
                        size_t *out_len);

/* Get number of entries */
uint32_t zip_count(const zip_archive_t *za);

/* Get entry by index */
const zip_entry_t *zip_get(const zip_archive_t *za, uint32_t index);

#endif /* ZIP_READER_H */
