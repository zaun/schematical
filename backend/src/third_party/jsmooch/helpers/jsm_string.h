/*
 * Stub for jsmooch's helpers/jsm_string.h (plan-14-G): a fixed-size string
 * the 65816 disassembler appends to. Ours, not jsmooch's (README.md).
 */
#ifndef JSMOOCH_STUB_JSM_STRING_H
#define JSMOOCH_STUB_JSM_STRING_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "int.h"

struct jsm_string {
    char *ptr;      /* the text, NUL-terminated */
    size_t len;     /* characters in ptr */
    size_t cap;     /* bytes allocated */
};

static inline void jsm_string_init(struct jsm_string *s, size_t cap) {
    s->ptr = calloc(cap > 0 ? cap : 1, 1);
    s->len = 0;
    s->cap = s->ptr != NULL ? (cap > 0 ? cap : 1) : 0;
}

static inline void jsm_string_delete(struct jsm_string *s) {
    free(s->ptr);
    s->ptr = NULL;
    s->len = s->cap = 0;
}

static inline void jsm_string_quickempty(struct jsm_string *s) {
    s->len = 0;
    if (s->ptr != NULL) s->ptr[0] = '\0';
}

/* Appends; returns the characters appended (cut at the buffer's end). */
static inline u32 jsm_string_sprintf(struct jsm_string *s, const char *format, ...) {
    va_list args;
    int n;

    if (s->ptr == NULL || s->len + 1 >= s->cap) return 0;
    va_start(args, format);
    n = vsnprintf(s->ptr + s->len, s->cap - s->len, format, args);
    va_end(args);
    if (n < 0) return 0;
    if ((size_t)n >= s->cap - s->len) n = (int)(s->cap - s->len - 1);
    s->len += (size_t)n;
    return (u32)n;
}

#endif
