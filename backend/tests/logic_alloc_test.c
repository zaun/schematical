/*
 * logic_alloc_test: the allocation wrapper (plan-68-K): counts, live memory,
 * the k-th allocation failing, realloc's two cases, strdup, handing memory to
 * the caller, unmatched frees, and the cJSON hooks. Built without
 * tests/support/alloc_check.c, since it makes an unmatched free on purpose.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "logic_alloc.h"

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s (line %d)\n", #cond, __LINE__); failures++; } } while (0)

int main(void) {
    long live0 = logic_alloc_live();
    void *a, *b;
    char *s;

    /* Counting and live memory. */
    logic_alloc_fail_at(0);
    a = logic_malloc(8);
    b = logic_calloc(2, 4);
    CHECK(a != NULL && b != NULL);
    CHECK(logic_alloc_count() == 2);
    CHECK(logic_alloc_live() == live0 + 2);
    logic_free(a);
    logic_free(b);
    logic_free(NULL);
    CHECK(logic_alloc_live() == live0);
    printf("ok   counts and live memory\n");

    /* The k-th allocation from now fails, and only that one. */
    logic_alloc_fail_at(2);
    a = logic_malloc(4);
    b = logic_malloc(4);
    s = logic_strdup("x");
    CHECK(a != NULL && b == NULL && s != NULL);
    CHECK(logic_alloc_count() == 3);
    logic_free(a);
    logic_free(s);
    logic_alloc_fail_at(1);
    CHECK(logic_calloc(1, 1) == NULL);
    logic_alloc_fail_at(0);
    CHECK(logic_alloc_live() == live0);
    printf("ok   the k-th allocation fails\n");

    /* realloc: NULL is a new allocation; a failed one keeps the old block. */
    a = logic_realloc(NULL, 4);
    CHECK(a != NULL && logic_alloc_live() == live0 + 1);
    memcpy(a, "abc", 4);
    logic_alloc_fail_at(1);
    CHECK(logic_realloc(a, 64) == NULL);
    CHECK(strcmp(a, "abc") == 0);
    logic_alloc_fail_at(0);
    a = logic_realloc(a, 64);
    CHECK(a != NULL && strcmp(a, "abc") == 0 && logic_alloc_live() == live0 + 1);
    logic_free(a);
    printf("ok   realloc\n");

    /* strdup copies; it fails as one allocation. */
    s = logic_strdup("hello");
    CHECK(s != NULL && strcmp(s, "hello") == 0);
    logic_free(s);
    logic_alloc_fail_at(1);
    CHECK(logic_strdup("hello") == NULL);
    logic_alloc_fail_at(0);
    printf("ok   strdup\n");

    /* Memory handed to the caller is no longer live; the caller frees it. */
    s = logic_alloc_disown(logic_strdup("message"));
    CHECK(logic_alloc_live() == live0);
    free(s);
    CHECK(logic_alloc_disown(NULL) == NULL);
    printf("ok   disown\n");

    /* A free of memory the wrapper did not count is noticed, and the live
       count does not go below zero. (Nothing is live here: live0 is 0.) */
    CHECK(live0 == 0 && logic_alloc_unmatched() == 0);
    logic_free(malloc(1));
    CHECK(logic_alloc_unmatched() == 1 && logic_alloc_live() == 0);
    printf("ok   unmatched frees\n");

    /* cJSON allocates through the wrapper once it is hooked. */
    logic_alloc_init();
    logic_alloc_init();
    {
        long before = logic_alloc_live();
        cJSON *item = cJSON_CreateObject();

        CHECK(item != NULL && logic_alloc_live() == before + 1);
        cJSON_Delete(item);
        CHECK(logic_alloc_live() == before);
    }
    printf("ok   cJSON hooks\n");

    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("PASS\n");
    return 0;
}
