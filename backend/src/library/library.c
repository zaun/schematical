#include "parts.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "kind.h"
#include "library_internal.h"

/* ---- Name sets: open addressing, keys borrowed --------------------------- */

typedef struct {
    const char **keys;
    const void **values;
    size_t capacity;   /* power of two, or 0 */
    size_t count;
} NameSet;

static uint64_t hash_name(const char *text) {
    uint64_t hash = 0xcbf29ce484222325ull;

    for (; *text != '\0'; text++) {
        hash ^= (unsigned char)*text;
        hash *= 0x100000001b3ull;
    }
    return hash;
}

static size_t name_slot(const NameSet *set, const char *key) {
    size_t mask = set->capacity - 1;
    size_t slot = (size_t)hash_name(key) & mask;

    while (set->keys[slot] != NULL && strcmp(set->keys[slot], key) != 0) {
        slot = (slot + 1) & mask;
    }
    return slot;
}

static int name_set_contains(const NameSet *set, const char *key) {
    return set->capacity > 0 && set->keys[name_slot(set, key)] != NULL;
}

static const void *name_set_get(const NameSet *set, const char *key) {
    size_t slot;

    if (set->capacity == 0) {
        return NULL;
    }
    slot = name_slot(set, key);
    return set->keys[slot] != NULL ? set->values[slot] : NULL;
}

/* Returns -1 when out of memory. */
static int name_set_add(NameSet *set, const char *key, const void *value) {
    if ((set->count + 1) * 2 > set->capacity) {
        NameSet bigger = {0};
        size_t i;

        bigger.capacity = set->capacity == 0 ? 64 : set->capacity * 2;
        bigger.keys = calloc(bigger.capacity, sizeof(*bigger.keys));
        bigger.values = calloc(bigger.capacity, sizeof(*bigger.values));
        if (bigger.keys == NULL || bigger.values == NULL) {
            free(bigger.keys);
            free(bigger.values);
            return -1;
        }
        for (i = 0; i < set->capacity; i++) {
            if (set->keys[i] != NULL) {
                size_t slot = name_slot(&bigger, set->keys[i]);

                bigger.keys[slot] = set->keys[i];
                bigger.values[slot] = set->values[i];
                bigger.count++;
            }
        }
        free(set->keys);
        free(set->values);
        *set = bigger;
    }
    {
        size_t slot = name_slot(set, key);

        if (set->keys[slot] == NULL) {
            set->keys[slot] = key;
            set->values[slot] = value;
            set->count++;
        }
    }
    return 0;
}

/* Adds every entry of `from` to `to`. */
static int name_set_merge(NameSet *to, const NameSet *from) {
    size_t i;

    for (i = 0; i < from->capacity; i++) {
        if (from->keys[i] != NULL && name_set_add(to, from->keys[i], from->values[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

static void name_set_free(NameSet *set) {
    free(set->keys);
    free(set->values);
    memset(set, 0, sizeof(*set));
}

/* ---- The library ---------------------------------------------------------- */

struct PartLibrary {
    const Part **parts;
    size_t count;
    size_t capacity;
    NameSet names;       /* part names */
    NameSet devices;     /* "<kind>/<device id>" */
    ArenaChunk *arenas;
};

PartLibrary *part_library_create(void) {
    return calloc(1, sizeof(PartLibrary));
}

static void free_chunks(ArenaChunk *chunk) {
    while (chunk != NULL) {
        ArenaChunk *next = chunk->next;

        free(chunk);
        chunk = next;
    }
}

void part_library_destroy(PartLibrary *library) {
    if (library == NULL) {
        return;
    }
    free(library->parts);
    name_set_free(&library->names);
    name_set_free(&library->devices);
    free_chunks(library->arenas);
    free(library);
}

size_t part_library_count(const PartLibrary *library) {
    return library != NULL ? library->count : 0;
}

const Part *part_library_at(const PartLibrary *library, size_t index) {
    return library != NULL && index < library->count ? library->parts[index] : NULL;
}

const Part *part_library_find(const PartLibrary *library, const char *name) {
    if (library == NULL || name == NULL) {
        return NULL;
    }
    return name_set_get(&library->names, name);
}

/* ---- Loading -------------------------------------------------------------- */

static const char *const FILE_KEYS[] = {"format", "version", "kind", "devices", NULL};
static const char *const DEVICE_KEYS[] = {
    "id", "function", "description", "drawer", "note", "pinouts", "parts", NULL
};
static const char *const PART_KEYS[] = {"name", "package", "pinout", "flags", NULL};
static const char *const PIN_KEYS[] = {"number", "name", "role", "group", NULL};

static const char *const ROLE_NAMES[] = {
    "input", "output", "inout", "power", "ground", "nc", "absent"
};
static const char *const FLAG_NAMES[] = {"obsolete", "military", "automotive", "enhanced"};

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
} FileParts;

static int file_add_part(FileParts *file, const Part *part) {
    if (file->count == file->capacity) {
        size_t capacity = file->capacity == 0 ? 256 : file->capacity * 2;
        const Part **parts = realloc(file->parts, capacity * sizeof(*parts));

        if (parts == NULL) {
            return -1;
        }
        file->parts = parts;
        file->capacity = capacity;
    }
    file->parts[file->count++] = part;
    return 0;
}

static int load_pinout(KindContext *ctx, const cJSON *json, Pinout *out) {
    const cJSON *pin_json;
    PartPin *pins;
    size_t n = 0;

    if (!cJSON_IsArray(json) || cJSON_GetArraySize(json) == 0) {
        return kind_error(ctx, "a pinout must be a non-empty array of pins");
    }
    pins = kind_alloc(ctx, (size_t)cJSON_GetArraySize(json) * sizeof(*pins));
    if (pins == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(pin_json, json) {
        PartPin *pin = &pins[n];
        const char *role;
        uint64_t number;
        size_t r;

        kind_push(ctx, "pin %zu", n + 1);
        if (kind_check_keys(ctx, pin_json, PIN_KEYS, NULL) != 0 ||
            kind_uint(ctx, pin_json, "number", 1000000, &number) != 0 ||
            kind_string(ctx, pin_json, "name", &pin->name) != 0 ||
            kind_string(ctx, pin_json, "role", &role) != 0) {
            return -1;
        }
        if (number != n + 1) {
            return kind_error(ctx, "pin numbers must be 1..N in order (found %llu)",
                              (unsigned long long)number);
        }
        pin->number = (unsigned)number;
        for (r = 0; r < sizeof ROLE_NAMES / sizeof ROLE_NAMES[0]; r++) {
            if (strcmp(role, ROLE_NAMES[r]) == 0) {
                break;
            }
        }
        if (r == sizeof ROLE_NAMES / sizeof ROLE_NAMES[0]) {
            return kind_error(ctx, "unknown role \"%s\"", role);
        }
        pin->role = (PinRole)r;
        if (cJSON_GetObjectItemCaseSensitive(pin_json, "group") != NULL) {
            if (kind_string(ctx, pin_json, "group", &pin->group) != 0) {
                return -1;
            }
        } else {
            pin->group = "default";
        }
        kind_pop(ctx);
        n++;
    }
    out->pins = pins;
    out->pin_count = n;
    return 0;
}

static int load_flags(KindContext *ctx, const cJSON *part_json, unsigned *flags) {
    const cJSON *json = cJSON_GetObjectItemCaseSensitive(part_json, "flags");
    const cJSON *flag;

    *flags = 0;
    if (json == NULL) {
        return 0;
    }
    if (!cJSON_IsArray(json)) {
        return kind_error(ctx, "\"flags\" must be an array");
    }
    cJSON_ArrayForEach(flag, json) {
        size_t f;

        if (!cJSON_IsString(flag)) {
            return kind_error(ctx, "\"flags\" must be an array of strings");
        }
        for (f = 0; f < 4; f++) {
            if (strcmp(flag->valuestring, FLAG_NAMES[f]) == 0) {
                break;
            }
        }
        if (f == 4) {
            return kind_error(ctx, "unknown flag \"%s\"", flag->valuestring);
        }
        *flags |= 1u << f;
    }
    return 0;
}

static int load_part(KindContext *ctx, PartLibrary *library, FileParts *file,
                     const PartKindOps *ops, const cJSON *part_json, const void *device,
                     const Part *common, const Pinout *pinouts, size_t pinout_count) {
    Part *part;
    PartInternal *internal;
    const char *pinout_name;
    size_t p;

    if (kind_check_keys(ctx, part_json, PART_KEYS, ops->part_keys) != 0) {
        return -1;
    }
    part = kind_alloc(ctx, sizeof(*part));
    internal = kind_alloc(ctx, sizeof(*internal));
    if (part == NULL || internal == NULL) {
        return -1;
    }
    *part = *common;
    if (kind_string(ctx, part_json, "name", &part->name) != 0 ||
        kind_string(ctx, part_json, "package", &part->package) != 0 ||
        kind_string(ctx, part_json, "pinout", &pinout_name) != 0 ||
        load_flags(ctx, part_json, &part->flags) != 0) {
        return -1;
    }
    if (part->name[0] == '\0') {
        return kind_error(ctx, "empty part name");
    }
    if (name_set_contains(&library->names, part->name) ||
        name_set_contains(&file->names, part->name)) {
        return kind_error(ctx, "duplicate part name \"%s\"", part->name);
    }
    for (p = 0; p < pinout_count; p++) {
        if (strcmp(pinouts[p].name, pinout_name) == 0) {
            break;
        }
    }
    if (p == pinout_count) {
        return kind_error(ctx, "no pinout \"%s\"", pinout_name);
    }
    part->pins = pinouts[p].pins;
    part->pin_count = pinouts[p].pin_count;
    internal->ops = ops;
    internal->device = device;
    internal->bound = pinouts[p].bound;
    part->internal = internal;
    if (ops->load_part(ctx, device, pinouts[p].bound, part_json, part) != 0 || kind_failed(ctx)) {
        return kind_error(ctx, "invalid part");
    }
    if (name_set_add(&file->names, part->name, part) != 0 || file_add_part(file, part) != 0) {
        return kind_error(ctx, "out of memory");
    }
    return 0;
}

static int load_device(KindContext *ctx, PartLibrary *library, FileParts *file,
                       const PartKindOps *ops, const cJSON *device_json) {
    Part common = {0};
    const char *id;
    const char *const *drawer;
    size_t drawer_count;
    const cJSON *pinouts_json;
    const cJSON *parts_json;
    const cJSON *item;
    Pinout *pinouts;
    size_t pinout_count = 0;
    void *device;
    char *device_key;
    size_t i;

    if (kind_check_keys(ctx, device_json, DEVICE_KEYS, ops->device_keys) != 0 ||
        kind_string(ctx, device_json, "id", &id) != 0) {
        return -1;
    }
    {
        /* "devices[3]" becomes "devices[3] (74_00)" for later messages. */
        char segment[sizeof ctx->path[0]];

        snprintf(segment, sizeof segment, "%s",
                 ctx->depth - 1 < KIND_PATH_DEPTH ? ctx->path[ctx->depth - 1] : "");
        kind_pop(ctx);
        kind_push(ctx, "%s (%s)", segment, id);
    }
    if (kind_string(ctx, device_json, "function", &common.function) != 0 ||
        kind_string(ctx, device_json, "description", &common.description) != 0 ||
        kind_names(ctx, device_json, "drawer", &drawer, &drawer_count) != 0) {
        return -1;
    }
    if (drawer_count > 2) {
        return kind_error(ctx, "\"drawer\" has at most 2 levels");
    }
    item = cJSON_GetObjectItemCaseSensitive(device_json, "note");
    if (item != NULL && !cJSON_IsString(item)) {
        return kind_error(ctx, "\"note\" must be a string");
    }

    device_key = kind_alloc(ctx, strlen(ops->name) + strlen(id) + 2);
    if (device_key == NULL) {
        return -1;
    }
    sprintf(device_key, "%s/%s", ops->name, id);
    if (name_set_contains(&library->devices, device_key) ||
        name_set_contains(&file->devices, device_key)) {
        return kind_error(ctx, "duplicate device id \"%s\"", id);
    }
    if (name_set_add(&file->devices, device_key, device_key) != 0) {
        return kind_error(ctx, "out of memory");
    }

    common.kind = ops->kind;
    common.drawer = drawer;
    common.drawer_count = drawer_count;

    pinouts_json = kind_member(ctx, device_json, "pinouts", 1);
    if (pinouts_json == NULL) {
        return -1;
    }
    if (!cJSON_IsObject(pinouts_json) || pinouts_json->child == NULL) {
        return kind_error(ctx, "\"pinouts\" must be a non-empty object");
    }
    pinouts = kind_alloc(ctx, (size_t)cJSON_GetArraySize(pinouts_json) * sizeof(*pinouts));
    if (pinouts == NULL) {
        return -1;
    }
    for (item = pinouts_json->child; item != NULL; item = item->next) {
        const cJSON *other;

        for (other = pinouts_json->child; other != item; other = other->next) {
            if (strcmp(other->string, item->string) == 0) {
                return kind_error(ctx, "duplicate pinout \"%s\"", item->string);
            }
        }
        kind_push(ctx, "pinouts.%s", item->string);
        pinouts[pinout_count].name = kind_strdup(ctx, item->string);
        if (pinouts[pinout_count].name == NULL ||
            load_pinout(ctx, item, &pinouts[pinout_count]) != 0) {
            return -1;
        }
        kind_pop(ctx);
        pinout_count++;
    }

    device = ops->load_device(ctx, device_json);
    if (device == NULL || kind_failed(ctx)) {
        return kind_error(ctx, "invalid device");
    }
    for (i = 0; i < pinout_count; i++) {
        kind_push(ctx, "pinouts.%s", pinouts[i].name);
        pinouts[i].bound = ops->bind_pinout(ctx, device, pinouts[i].pins, pinouts[i].pin_count);
        if (pinouts[i].bound == NULL || kind_failed(ctx)) {
            return kind_error(ctx, "invalid pinout");
        }
        kind_pop(ctx);
    }

    parts_json = kind_member(ctx, device_json, "parts", 1);
    if (parts_json == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(parts_json)) {
        return kind_error(ctx, "\"parts\" must be an array");
    }
    i = 0;
    cJSON_ArrayForEach(item, parts_json) {
        const cJSON *name = cJSON_GetObjectItemCaseSensitive(item, "name");

        if (cJSON_IsString(name)) {
            kind_push(ctx, "parts[%zu] (%s)", i, name->valuestring);
        } else {
            kind_push(ctx, "parts[%zu]", i);
        }
        if (load_part(ctx, library, file, ops, item, device, &common, pinouts, pinout_count) != 0) {
            return -1;
        }
        kind_pop(ctx);
        i++;
    }
    return 0;
}

static int load_file(KindContext *ctx, PartLibrary *library, FileParts *file, const cJSON *root) {
    const cJSON *item;
    const char *format;
    const char *kind_name;
    uint64_t version;
    const PartKindOps *ops;
    size_t i = 0;

    if (kind_check_keys(ctx, root, FILE_KEYS, NULL) != 0 ||
        kind_string(ctx, root, "format", &format) != 0) {
        return -1;
    }
    if (strcmp(format, "schematical-parts") != 0) {
        return kind_error(ctx, "\"format\" must be \"schematical-parts\", not \"%s\"", format);
    }
    if (kind_uint(ctx, root, "version", UINT32_MAX, &version) != 0) {
        return -1;
    }
    if (version != 1) {
        return kind_error(ctx, "unsupported version %llu (expected 1)",
                          (unsigned long long)version);
    }
    if (kind_string(ctx, root, "kind", &kind_name) != 0) {
        return -1;
    }
    ops = kind_find(kind_name);
    if (ops == NULL) {
        return kind_error(ctx, "unknown kind \"%s\"", kind_name);
    }
    item = kind_member(ctx, root, "devices", 1);
    if (item == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(item)) {
        return kind_error(ctx, "\"devices\" must be an array");
    }
    for (item = item->child; item != NULL; item = item->next) {
        kind_push(ctx, "devices[%zu]", i);
        if (load_device(ctx, library, file, ops, item) != 0) {
            return -1;
        }
        kind_pop(ctx);
        i++;
    }
    return 0;
}

static char *copy_message(const char *text) {
    char *copy = malloc(strlen(text) + 1);

    if (copy != NULL) {
        strcpy(copy, text);
    }
    return copy;
}

int load_parts(PartLibrary *library, const char *json, const char *source, char **error) {
    KindContext ctx;
    FileParts file = {0};
    cJSON *root;
    int result = -1;

    if (error != NULL) {
        *error = NULL;
    }
    if (library == NULL || json == NULL) {
        if (error != NULL) {
            *error = copy_message("load_parts: no library or text");
        }
        return -1;
    }
    memset(&ctx, 0, sizeof ctx);
    ctx.source = source != NULL ? source : "parts";

    root = cJSON_ParseWithLength(json, strlen(json));
    if (root == NULL) {
        const char *where = cJSON_GetErrorPtr();

        kind_error(&ctx, "invalid JSON at byte %ld",
                   where != NULL ? (long)(where - json) : -1L);
    } else if (load_file(&ctx, library, &file, root) == 0 && !kind_failed(&ctx)) {
        size_t needed = library->count + file.count;

        if (needed > library->capacity) {
            size_t capacity = library->capacity == 0 ? 1024 : library->capacity;
            const Part **parts;

            while (capacity < needed) {
                capacity *= 2;
            }
            parts = realloc(library->parts, capacity * sizeof(*parts));
            if (parts == NULL) {
                kind_error(&ctx, "out of memory");
            } else {
                library->parts = parts;
                library->capacity = capacity;
            }
        }
        if (!kind_failed(&ctx)) {
            NameSet names = {0};
            NameSet devices = {0};
            /* Build the grown sets on the side so a failure leaves the library as it was. */
            int ok = name_set_merge(&names, &library->names) == 0 &&
                     name_set_merge(&names, &file.names) == 0 &&
                     name_set_merge(&devices, &library->devices) == 0 &&
                     name_set_merge(&devices, &file.devices) == 0;

            if (!ok) {
                name_set_free(&names);
                name_set_free(&devices);
                kind_error(&ctx, "out of memory");
            } else {
                ArenaChunk *last = ctx.arena;

                name_set_free(&library->names);
                name_set_free(&library->devices);
                library->names = names;
                library->devices = devices;
                memcpy(library->parts + library->count, file.parts, file.count * sizeof(*file.parts));
                library->count += file.count;
                if (last != NULL) {
                    while (last->next != NULL) {
                        last = last->next;
                    }
                    last->next = library->arenas;
                    library->arenas = ctx.arena;
                    ctx.arena = NULL;
                }
                result = 0;
            }
        }
    }

    cJSON_Delete(root);
    free(file.parts);
    name_set_free(&file.names);
    name_set_free(&file.devices);
    free_chunks(ctx.arena);
    if (result != 0) {
        if (error != NULL) {
            *error = ctx.error != NULL ? ctx.error : copy_message("out of memory");
        } else {
            free(ctx.error);
        }
    } else {
        free(ctx.error);
    }
    return result;
}
