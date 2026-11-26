/*
 * logic_alloc: every allocation the backend makes, through one place that a
 * test can make fail (plan-68-K). The counters are atomic: the app may use
 * the backend from more than one thread, and only tests read them.
 */
#include "logic_alloc.h"

#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"

static _Atomic long attempts;
static _Atomic long fail_at;
static _Atomic long live;
static _Atomic long unmatched;
static _Atomic int hooked;
static void *_Atomic failed_site;

/* Counts an attempt; nonzero when this one is the one that must fail. */
static int failing(void) {
    long n = atomic_fetch_add_explicit(&attempts, 1, memory_order_relaxed) + 1;
    long k = atomic_load_explicit(&fail_at, memory_order_relaxed);
    return k != 0 && n == k;
}

static void *counted(void *p) {
    if (p != NULL) {
        atomic_fetch_add_explicit(&live, 1, memory_order_relaxed);
    }
    return p;
}

/* A failed allocation: remembers who asked. */
static void *refuse(void *site) {
    atomic_store_explicit(&failed_site, site, memory_order_relaxed);
    return NULL;
}

void *logic_malloc(size_t size) {
    return failing() ? refuse(__builtin_return_address(0)) : counted(malloc(size));
}

void *logic_calloc(size_t count, size_t size) {
    return failing() ? refuse(__builtin_return_address(0)) : counted(calloc(count, size));
}

void *logic_realloc(void *p, size_t size) {
    if (p == NULL) {
        return failing() ? refuse(__builtin_return_address(0)) : counted(malloc(size));
    }
    return failing() ? refuse(__builtin_return_address(0)) : realloc(p, size);
}

char *logic_strdup(const char *text) {
    size_t n = strlen(text) + 1;
    char *copy = failing() ? refuse(__builtin_return_address(0)) : counted(malloc(n));

    if (copy != NULL) {
        memcpy(copy, text, n);
    }
    return copy;
}

void logic_free(void *p) {
    if (p == NULL) {
        return;
    }
    if (atomic_fetch_sub_explicit(&live, 1, memory_order_relaxed) <= 0) {
        atomic_fetch_add_explicit(&live, 1, memory_order_relaxed);
        atomic_fetch_add_explicit(&unmatched, 1, memory_order_relaxed);
    }
    free(p);
}

void *logic_alloc_disown(void *p) {
    if (p != NULL) {
        atomic_fetch_sub_explicit(&live, 1, memory_order_relaxed);
    }
    return p;
}

void logic_alloc_init(void) {
    if (atomic_exchange_explicit(&hooked, 1, memory_order_relaxed) == 0) {
        cJSON_Hooks hooks = {logic_malloc, logic_free};
        cJSON_InitHooks(&hooks);
    }
}

void *logic_alloc_failed_site(void) {
    return atomic_load_explicit(&failed_site, memory_order_relaxed);
}

void logic_alloc_fail_at(long k) {
    if (k != 0) {
        atomic_store_explicit(&failed_site, NULL, memory_order_relaxed);
    }
    atomic_store_explicit(&attempts, 0, memory_order_relaxed);
    atomic_store_explicit(&fail_at, k, memory_order_relaxed);
}

long logic_alloc_count(void) {
    return atomic_load_explicit(&attempts, memory_order_relaxed);
}

long logic_alloc_live(void) {
    return atomic_load_explicit(&live, memory_order_relaxed);
}

long logic_alloc_unmatched(void) {
    return atomic_load_explicit(&unmatched, memory_order_relaxed);
}
