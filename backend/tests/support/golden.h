#ifndef GOLDEN_H
#define GOLDEN_H

/*
 * The plan-01-A goldens (tests/golden/parts.txt and traces.txt), read for
 * listing_check and trace_check. Lines are sorted by name in both files.
 */

#include <stddef.h>

typedef struct {
    char *name;
    char *kind;     /* from parts.txt */
    char *line;     /* the whole parts.txt line, no newline */
    char *hash;     /* from traces.txt, 16 hex digits */
} GoldenPart;

/* Reads both files; exits on any error. The array is sorted by name. */
size_t golden_read(const char *dir, GoldenPart **parts);

/* Whether a listing kind name is registered in the library's KINDS[]. */
int golden_kind_registered(const char *kind);

/* Every registered kind's listing name is in KINDS[]; count of them. */
size_t golden_registered_count(void);

#endif
