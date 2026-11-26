#ifndef S68_MUTATE_H
#define S68_MUTATE_H

/*
 * s68_mutate (plan-68-S): loader cases written as one change to a shipped
 * device. A case names a kind's part file (Parts/<kind>.json), one device id
 * in it and a patch; the device is copied, the patch applied, the copy given
 * a fresh id and part names (each name + "-S68-<n>"), and the one-device file
 * loaded with load_parts into the shipped library.
 *
 * A patch is a JSON object of dotted paths: "grades.14.tcyc": 0 replaces (or
 * adds) that member, null removes it; a number step indexes an array
 * ("parts.0.grade", "pinouts.p40.3.role"); an index past the end appends; "key+"
 * adds a second member of that name. ' stands for " in a patch.
 */

#include "parts.h"

/* The shipped library, loaded on first use; s68_free releases it. */
PartLibrary *s68_library(void);
void s68_free(void);

/*
 * Loads the patched device. Returns 0 and the first part's new name in
 * `name` (when not NULL; valid until the next call), or -1 with the loader
 * message in `error` (caller frees; may be NULL).
 */
int s68_load(const char *kind, const char *device, const char *patch, const char **name,
             char **error);

/* The patched device must be refused with `want` in the message: 1 when it is. */
int s68_rejects(const char *kind, const char *device, const char *patch, const char *want);

/* The patched device must load: its first part, or NULL (the error printed). */
const Part *s68_loads(const char *kind, const char *device, const char *patch);

#endif
