#ifndef READ_FILE_H
#define READ_FILE_H

#include "parts.h"

/* A whole file as a NUL-terminated malloc'd string, or NULL. */
char *read_file(const char *path);

/*
 * Loads every *.json file in a directory (sorted by name) into the library.
 * Prints any load error to stderr and returns -1; 0 when all loaded.
 */
int load_parts_dir(PartLibrary *library, const char *dir);

#endif
