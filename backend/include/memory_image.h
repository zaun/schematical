#ifndef MEMORY_IMAGE_H
#define MEMORY_IMAGE_H

/*
 * Load files for memories (EEPROMs): turns a file's bytes into a full
 * memory image for circuit_memory_load. The backend never opens files; the
 * caller reads them.
 *
 *     uint32_t *image = malloc(words * sizeof *image);
 *     char *error = NULL;
 *     if (memory_image_parse(bytes, size, memory_image_format_for(name),
 *                            words, 8, image, &error) != 0) {
 *         ... show error; free(error);
 *     }
 */

#include <stddef.h>
#include <stdint.h>

typedef enum { MEMORY_IMAGE_BINARY, MEMORY_IMAGE_INTEL_HEX } MemoryImageFormat;

/* The format for a file name: .hex, .ihx, .ihex (any case) → Intel HEX;
   anything else → binary. */
MemoryImageFormat memory_image_format_for(const char *file_name);

/* Fills values[0 .. words) from a load file's bytes: 0xFF (erased) where the
   file has no data. bits must be 8. 0 on success; -1 and a malloc'd message
   in *error (free it). values is untouched on failure. */
int memory_image_parse(const unsigned char *data, size_t size, MemoryImageFormat format,
                       uint32_t words, unsigned bits, uint32_t *values, char **error);

#endif
