#ifndef FOOTPRINT_H
#define FOOTPRINT_H

/* Shared by library.c and footprints.c only. */

#include "kind.h"

/* A file's footprints, kept until the whole file has loaded (malloc'd array). */
typedef struct {
    const PartFootprint **items;
    size_t count;
    size_t capacity;
} FootprintList;

/*
 * Checks one footprints file (format, version, every footprint) and expands
 * each footprint into the context's arena. `library` is read only, to reject
 * ids it already has. 0 or -1 (error set in ctx).
 */
int footprint_read_file(KindContext *ctx, const cJSON *root, const PartLibrary *library,
                        FootprintList *out);

/*
 * The per-footprint part of footprint_read_file: checks and expands every
 * entry of a "footprints" array (the footprints.json entry format) into
 * `out`. Ids must be new to `library` and to `out`. Used by footprints.json
 * and by a single part file's inline "footprints" (load_part_file).
 */
int footprint_read_entries(KindContext *ctx, const cJSON *array, const PartLibrary *library,
                           FootprintList *out);

#endif
