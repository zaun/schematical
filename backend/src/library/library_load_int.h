/* library/library_load_int.h: what the pieces of library/library.c share (plan-68-I/J split it). */
#ifndef LIBRARY_LIBRARY_LOAD_INT_H
#define LIBRARY_LIBRARY_LOAD_INT_H

#include "parts.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "footprint.h"
#include "kind.h"
#include "library_internal.h"

/* ---- Name sets: open addressing, keys borrowed --------------------------- */

typedef struct {
    const char **keys;
    const void **values;
    size_t capacity;   /* power of two, or 0 */
    size_t count;
} NameSet;

/* ---- The library ---------------------------------------------------------- */

/* A single part file's inline datasheets (arena). */
typedef struct {
    const PartDatasheet **items;
    size_t count;
} DatasheetList;

struct PartLibrary {
    const Part **parts;
    size_t count;
    size_t capacity;
    NameSet names;       /* part names */
    NameSet devices;     /* "<kind>/<device id>" */
    const PartFootprint **footprints;
    size_t footprint_count;
    NameSet footprint_ids;
    const PartDatasheet **datasheets;
    size_t datasheet_count;
    NameSet datasheet_ids;
    NameSet uuids;       /* single part files' "uuid" (load_part_file) */
    /* A single part file's inline footprints while load_part_file checks it;
       NULL otherwise. part_library_find_footprint looks here too. */
    const FootprintList *staged_footprints;
    /* The same for its inline datasheets (part_library_find_datasheet). */
    const DatasheetList *staged_datasheets;
    /* The latest load_part_file, for part_library_unload_last_part_file:
       the counts and the arena head from before it; cleared by any other
       load. */
    int last_part_file;
    size_t last_count, last_footprint_count, last_datasheet_count;
    ArenaChunk *last_arenas;
    ArenaChunk *arenas;
};
#define FLAG_COUNT (sizeof lib_FLAG_NAMES / sizeof lib_FLAG_NAMES[0])

typedef struct {
    const char *name;
    const PartPin *pins;
    size_t pin_count;
    const void *bound;
} Pinout;

/* A file's results, kept until the whole file has loaded. */
typedef struct {
    const Part **parts;
    size_t count;
    size_t capacity;
    NameSet names;
    NameSet devices;
    /* A single part file (load_part_file): its device id is not checked
       against the library's, and is not added to it, so a copy of a bundled
       device and several custom parts of one device all load. */
    int single;
} FileParts;

/* A pin's "activeLow" (plan-39-A § 3): true, or a proper non-empty subset
 * of the name's slash-separated alternatives. Sets pin->active_low. */
#define ACTIVE_LOW_MAX 32

/* A footprint list grown by `extra`, built on the side (0, or -1 out of memory). */
typedef struct {
    const PartFootprint **all;
    size_t count;
    NameSet ids;
} FootprintTable;

int lib_name_set_contains(const NameSet *set, const char *key);
int lib_name_set_add(NameSet *set, const char *key, const void *value);
int lib_name_set_merge(NameSet *to, const NameSet *from);
void lib_name_set_free(NameSet *set);
void lib_free_chunks(ArenaChunk *chunk);
extern const char *const lib_FILE_KEYS[5];
extern const char *const lib_DEVICE_KEYS[9];
extern const char *const lib_PART_KEYS[11];
extern const char *const lib_PIN_KEYS[8];
extern const char *const lib_ROLE_NAMES[7];
extern const char *const lib_FLAG_NAMES[5];
int lib_load_pinout(KindContext *ctx, const cJSON *json, Pinout *out);
int lib_load_part(KindContext *ctx, PartLibrary *library, FileParts *file,
                     const PartKindOps *ops, const cJSON *part_json, const void *device,
                     const Part *common, const Pinout *pinouts, size_t pinout_count);
int lib_load_device(KindContext *ctx, PartLibrary *library, FileParts *file,
                       const PartKindOps *ops, const cJSON *device_json);
char *lib_copy_message(const char *text);
void lib_report(KindContext *ctx, int result, char **error);
int lib_commit_parts(KindContext *ctx, PartLibrary *library, const FileParts *file,
                        const FootprintList *footprints, const DatasheetList *datasheets,
                        const char *uuid);
int lib_read_datasheet_entry(KindContext *ctx, const cJSON *item, const PartLibrary *library,
                                NameSet *ids, int allow_md, PartDatasheet **out);

#endif
