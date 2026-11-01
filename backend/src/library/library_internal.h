#ifndef LIBRARY_INTERNAL_H
#define LIBRARY_INTERNAL_H

/* Shared by library.c and kind_util.c only. */

#include <stddef.h>

#define KIND_PATH_DEPTH 8

typedef struct ArenaChunk {
    struct ArenaChunk *next;
    size_t used;
    size_t size;
    /* data follows */
} ArenaChunk;

struct KindContext {
    const char *source;
    ArenaChunk *arena;
    char *error;              /* malloc'd, first error only */
    int out_of_memory;
    char path[KIND_PATH_DEPTH][96];
    size_t depth;
};

#endif
