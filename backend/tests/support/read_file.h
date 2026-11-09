#ifndef READ_FILE_H
#define READ_FILE_H

#include "parts.h"

/* A whole file as a NUL-terminated malloc'd string, or NULL. */
char *read_file(const char *path);

/*
 * Loads datasheets.json from a directory with load_datasheets. Prints any
 * error to stderr and returns -1; 0 when loaded.
 */
int load_datasheets_file(PartLibrary *library, const char *dir);

/*
 * Loads datasheets.json and footprints.json (both required) with
 * load_datasheets / load_footprints, then every other *.json file in a
 * directory (sorted by name) with load_parts. Prints any
 * load error to stderr and returns -1; 0 when all loaded.
 */
int load_parts_dir(PartLibrary *library, const char *dir);

#endif
