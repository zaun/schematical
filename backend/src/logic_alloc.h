/*
 * logic_alloc: every allocation the backend makes (plan-68-K).
 *
 * In the app these are plain malloc / calloc / realloc / free. Tests can make
 * the k-th allocation from now fail (`logic_alloc_fail_at`), count the
 * allocations made, and check that every allocation was freed
 * (`logic_alloc_live`), which is how the out-of-memory paths are tested.
 */
#ifndef LOGIC_ALLOC_H
#define LOGIC_ALLOC_H

#include <stddef.h>

void *logic_malloc(size_t size);
void *logic_calloc(size_t count, size_t size);
/* Like realloc. A NULL `p` is a new allocation; a failed one leaves `p` as it was. */
void *logic_realloc(void *p, size_t size);
char *logic_strdup(const char *text);
void logic_free(void *p);
/* Memory handed to the caller (an error message the caller frees with free()):
   no longer counted as live. Returns p. */
void *logic_alloc_disown(void *p);

/* Makes cJSON allocate through these too (idempotent). The library and the
   circuit call it when they are created. */
void logic_alloc_init(void);

/* Tests only. */
/* The k-th allocation from now (k >= 1) returns NULL; 0 = none fails. */
void logic_alloc_fail_at(long k);
/* Allocations attempted so far. */
long logic_alloc_count(void);
/* Allocations made and not yet freed. */
long logic_alloc_live(void);
/* Frees of memory the wrapper did not count (the live count would have gone
   below zero): always 0 unless memory from elsewhere was given to logic_free. */
long logic_alloc_unmatched(void);

#endif
