/*
 * Compiled into every test program (plan-68-K): when the program ends,
 * everything the backend allocated must have been freed, and nothing the
 * backend did not allocate may have been given to logic_free. A program
 * that leaks fails, even when its cases passed.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "logic_alloc.h"

static void check_allocations(void) {
    long live = logic_alloc_live();
    long unmatched = logic_alloc_unmatched();

    if (live != 0 || unmatched != 0) {
        fprintf(stderr, "FAIL: %ld backend allocations not freed, %ld frees of memory it did not allocate\n",
                live, unmatched);
        fflush(stderr);
        _exit(1);
    }
}

__attribute__((constructor)) static void install_allocation_check(void) {
    atexit(check_allocations);
}
