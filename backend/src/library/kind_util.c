#include "kind.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "library_internal.h"

#define ARENA_CHUNK (64 * 1024)

void *kind_alloc(KindContext *ctx, size_t size) {
    ArenaChunk *chunk = ctx->arena;
    void *memory;

    size = (size + 15u) & ~(size_t)15u;
    if (size == 0) {
        size = 16;
    }
    if (chunk == NULL || chunk->size - chunk->used < size) {
        size_t chunk_size = size > ARENA_CHUNK ? size : ARENA_CHUNK;
        size_t header = (sizeof(ArenaChunk) + 15u) & ~(size_t)15u;

        chunk = malloc(header + chunk_size);
        if (chunk == NULL) {
            ctx->out_of_memory = 1;
            kind_error(ctx, "out of memory");
            return NULL;
        }
        chunk->next = ctx->arena;
        chunk->used = header;
        chunk->size = header + chunk_size;
        ctx->arena = chunk;
    }
    memory = (char *)chunk + chunk->used;
    chunk->used += size;
    memset(memory, 0, size);
    return memory;
}

char *kind_strdup(KindContext *ctx, const char *text) {
    size_t length = strlen(text) + 1;
    char *copy = kind_alloc(ctx, length);

    if (copy != NULL) {
        memcpy(copy, text, length);
    }
    return copy;
}

int kind_error(KindContext *ctx, const char *format, ...) {
    char message[512];
    char *full;
    size_t length;
    size_t i;
    va_list args;

    if (ctx->error != NULL) {
        return -1;
    }
    va_start(args, format);
    vsnprintf(message, sizeof message, format, args);
    va_end(args);

    length = strlen(ctx->source) + strlen(message) + 3;
    for (i = 0; i < ctx->depth; i++) {
        length += strlen(ctx->path[i]) + 2;
    }
    full = malloc(length);
    if (full == NULL) {
        ctx->out_of_memory = 1;
        return -1;
    }
    strcpy(full, ctx->source);
    for (i = 0; i < ctx->depth; i++) {
        strcat(full, ": ");
        strcat(full, ctx->path[i]);
    }
    strcat(full, ": ");
    strcat(full, message);
    ctx->error = full;
    return -1;
}

int kind_failed(const KindContext *ctx) {
    return ctx->error != NULL || ctx->out_of_memory;
}

void kind_push(KindContext *ctx, const char *format, ...) {
    va_list args;

    if (ctx->depth < KIND_PATH_DEPTH) {
        va_start(args, format);
        vsnprintf(ctx->path[ctx->depth], sizeof ctx->path[0], format, args);
        va_end(args);
    }
    ctx->depth++;
}

void kind_pop(KindContext *ctx) {
    if (ctx->depth > 0) {
        ctx->depth--;
    }
}

static int in_list(const char *const *list, const char *key) {
    if (list == NULL) {
        return 0;
    }
    for (; *list != NULL; list++) {
        if (strcmp(*list, key) == 0) {
            return 1;
        }
    }
    return 0;
}

int kind_check_keys(KindContext *ctx, const cJSON *object, const char *const *common,
                    const char *const *extra) {
    const cJSON *item;

    if (!cJSON_IsObject(object)) {
        return kind_error(ctx, "expected an object");
    }
    for (item = object->child; item != NULL; item = item->next) {
        const cJSON *other;

        if ((common != NULL || extra != NULL) && !in_list(common, item->string) &&
            !in_list(extra, item->string)) {
            return kind_error(ctx, "unknown key \"%s\"", item->string);
        }
        for (other = object->child; other != item; other = other->next) {
            if (strcmp(other->string, item->string) == 0) {
                return kind_error(ctx, "duplicate key \"%s\"", item->string);
            }
        }
    }
    return 0;
}

const cJSON *kind_member(KindContext *ctx, const cJSON *object, const char *key, int required) {
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(object, key);

    if (item == NULL && required) {
        kind_error(ctx, "missing \"%s\"", key);
    }
    return item;
}

int kind_string(KindContext *ctx, const cJSON *object, const char *key, const char **out) {
    const cJSON *item = kind_member(ctx, object, key, 1);

    if (item == NULL) {
        return -1;
    }
    if (!cJSON_IsString(item)) {
        return kind_error(ctx, "\"%s\" must be a string", key);
    }
    *out = kind_strdup(ctx, item->valuestring);
    return *out != NULL ? 0 : -1;
}

static int whole_number(const cJSON *item, uint64_t max, uint64_t *out) {
    double value;

    if (!cJSON_IsNumber(item)) {
        return 0;
    }
    value = item->valuedouble;
    if (!(value >= 0) || value != floor(value) || value > 9007199254740992.0 ||
        (uint64_t)value > max) {
        return 0;
    }
    *out = (uint64_t)value;
    return 1;
}

int kind_uint(KindContext *ctx, const cJSON *object, const char *key, uint64_t max,
              uint64_t *out) {
    const cJSON *item = kind_member(ctx, object, key, 1);

    if (item == NULL) {
        return -1;
    }
    if (!whole_number(item, max, out)) {
        return kind_error(ctx, "\"%s\" must be a whole number from 0 to %llu", key,
                          (unsigned long long)max);
    }
    return 0;
}

int kind_bool(KindContext *ctx, const cJSON *object, const char *key, int *out) {
    const cJSON *item = kind_member(ctx, object, key, 1);

    if (item == NULL) {
        return -1;
    }
    if (!cJSON_IsBool(item)) {
        return kind_error(ctx, "\"%s\" must be true or false", key);
    }
    *out = cJSON_IsTrue(item) ? 1 : 0;
    return 0;
}

int kind_names(KindContext *ctx, const cJSON *object, const char *key,
               const char *const **out, size_t *count) {
    const cJSON *item = kind_member(ctx, object, key, 1);
    const cJSON *element;
    const char **names;
    size_t n = 0;

    if (item == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(item)) {
        return kind_error(ctx, "\"%s\" must be an array of strings", key);
    }
    names = kind_alloc(ctx, (size_t)cJSON_GetArraySize(item) * sizeof(*names) + 1);
    if (names == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(element, item) {
        if (!cJSON_IsString(element)) {
            return kind_error(ctx, "\"%s\" must be an array of strings", key);
        }
        names[n] = kind_strdup(ctx, element->valuestring);
        if (names[n] == NULL) {
            return -1;
        }
        n++;
    }
    *out = names;
    *count = n;
    return 0;
}

int kind_ns(KindContext *ctx, const cJSON *value, const char *what, uint64_t *ps) {
    double ns;

    if (value == NULL) {
        return kind_error(ctx, "missing \"%s\"", what);
    }
    if (!cJSON_IsNumber(value) || !(value->valuedouble >= 0) || value->valuedouble > 1e9) {
        return kind_error(ctx, "\"%s\" must be a time in ns from 0 to 1e9", what);
    }
    ns = value->valuedouble;
    *ps = (uint64_t)llround(ns * 1000.0);
    return 0;
}

int kind_pin(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *name,
             size_t *index) {
    size_t i;
    size_t found = 0;

    for (i = 0; i < pin_count; i++) {
        if (strcmp(pins[i].name, name) == 0) {
            if (found++ == 0) {
                *index = i;
            }
        }
    }
    if (found == 0) {
        return kind_error(ctx, "no pin named \"%s\"", name);
    }
    if (found > 1) {
        return kind_error(ctx, "pin name \"%s\" is used by %zu pins; one is needed", name, found);
    }
    return 0;
}

size_t kind_pins(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *name,
                 size_t *out, size_t cap) {
    size_t i;
    size_t found = 0;

    for (i = 0; i < pin_count; i++) {
        if (strcmp(pins[i].name, name) == 0) {
            if (found == cap) {
                kind_error(ctx, "too many pins named \"%s\"", name);
                return 0;
            }
            out[found++] = i;
        }
    }
    if (found == 0) {
        kind_error(ctx, "no pin named \"%s\"", name);
    }
    return found;
}

static const char *const LOGIC_FAMILIES[] = {
    "HC", "LS", "ACT", "HCT", "F", "AC", "AHCT", "ALS", "AS", "AHC", "S", "C",
    "H", "HCS", "LV", "LVC", "AUP", "AUC", "ALVC", "TTL", NULL
};

const char *kind_logic_family(KindContext *ctx, const char *text) {
    size_t i;

    for (i = 0; LOGIC_FAMILIES[i] != NULL; i++) {
        if (strcmp(LOGIC_FAMILIES[i], text) == 0) {
            return LOGIC_FAMILIES[i];
        }
    }
    kind_error(ctx, "unknown family \"%s\"", text);
    return NULL;
}
