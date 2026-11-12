#include "parts.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "footprint.h"
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
    const PartFootprint **footprints;
    size_t footprint_count;
    NameSet footprint_ids;
    const PartDatasheet **datasheets;
    size_t datasheet_count;
    NameSet datasheet_ids;
    NameSet uuids;       /* single part files' "uuid" (load_part_file) */
    /* A single part file's inline footprints while load_part_file checks it;
       NULL otherwise. part_library_find_footprint looks here too. */
    const FootprintList *staged_footprints;
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
    free(library->footprints);
    name_set_free(&library->footprint_ids);
    free(library->datasheets);
    name_set_free(&library->datasheet_ids);
    name_set_free(&library->names);
    name_set_free(&library->devices);
    name_set_free(&library->uuids);
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

const Part *part_library_find_uuid(const PartLibrary *library, const char *uuid) {
    if (library == NULL || uuid == NULL) {
        return NULL;
    }
    return name_set_get(&library->uuids, uuid);
}

size_t part_library_footprint_count(const PartLibrary *library) {
    return library != NULL ? library->footprint_count : 0;
}

const PartFootprint *part_library_footprint_at(const PartLibrary *library, size_t index) {
    return library != NULL && index < library->footprint_count ? library->footprints[index] : NULL;
}

const PartFootprint *part_library_find_footprint(const PartLibrary *library, const char *id) {
    const PartFootprint *found;

    if (library == NULL || id == NULL) {
        return NULL;
    }
    found = name_set_get(&library->footprint_ids, id);
    if (found == NULL && library->staged_footprints != NULL) {
        size_t i;

        for (i = 0; i < library->staged_footprints->count; i++) {
            if (strcmp(library->staged_footprints->items[i]->id, id) == 0) {
                return library->staged_footprints->items[i];
            }
        }
    }
    return found;
}

size_t part_library_datasheet_count(const PartLibrary *library) {
    return library != NULL ? library->datasheet_count : 0;
}

const PartDatasheet *part_library_datasheet_at(const PartLibrary *library, size_t index) {
    if (library == NULL || index >= library->datasheet_count) {
        return NULL;
    }
    return library->datasheets[index];
}

const PartDatasheet *part_library_find_datasheet(const PartLibrary *library, const char *id) {
    if (library == NULL || id == NULL) {
        return NULL;
    }
    return name_set_get(&library->datasheet_ids, id);
}

/* ---- Loading -------------------------------------------------------------- */

static const char *const FILE_KEYS[] = {"format", "version", "kind", "devices", NULL};
static const char *const DEVICE_KEYS[] = {
    "id", "function", "description", "drawer", "note", "pinouts", "parts", NULL
};
static const char *const PART_KEYS[] = {
    "name", "package", "pinout", "flags", "footprint", "connectorFootprint",
    "footprintChoices", "manufacturer", "datasheet", NULL
};
static const char *const PIN_KEYS[] = {"number", "ball", "name", "role", "group", "side", NULL};

static const char *const ROLE_NAMES[] = {
    "input", "output", "inout", "power", "ground", "nc", "absent"
};
static const char *const FLAG_NAMES[] = {
    "obsolete", "military", "automotive", "enhanced", "ceramic"
};
#define FLAG_COUNT (sizeof FLAG_NAMES / sizeof FLAG_NAMES[0])

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
    /* A single part file (load_part_file): its device id is not checked
       against the library's, and is not added to it, so a copy of a bundled
       device and several custom parts of one device all load. */
    int single;
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
    int balls = 0;

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
        int has_number, has_ball;
        size_t r;

        kind_push(ctx, "pin %zu", n + 1);
        if (kind_check_keys(ctx, pin_json, PIN_KEYS, NULL) != 0) {
            return -1;
        }
        has_number = cJSON_GetObjectItemCaseSensitive(pin_json, "number") != NULL;
        has_ball = cJSON_GetObjectItemCaseSensitive(pin_json, "ball") != NULL;
        if (has_number && has_ball) {
            return kind_error(ctx, "a pin has \"number\" or \"ball\", not both");
        }
        if (!has_number && !has_ball) {
            return kind_error(ctx, "a pin needs \"number\" or \"ball\"");
        }
        if (n == 0) {
            balls = has_ball;
        } else if (balls != has_ball) {
            return kind_error(ctx, "every pin of a pinout uses \"number\", "
                                   "or every pin uses \"ball\"");
        }
        if (has_number) {
            uint64_t number;
            char text[24];

            if (kind_uint(ctx, pin_json, "number", 1000000, &number) != 0) {
                return -1;
            }
            if (number != n + 1) {
                return kind_error(ctx, "pin numbers must be 1..N in order (found %llu)",
                                  (unsigned long long)number);
            }
            snprintf(text, sizeof text, "%llu", (unsigned long long)number);
            pin->designator = kind_strdup(ctx, text);
            if (pin->designator == NULL) {
                return -1;
            }
        } else {
            size_t other;

            if (kind_string(ctx, pin_json, "ball", &pin->designator) != 0) {
                return -1;
            }
            if (!kind_is_ball_name(pin->designator)) {
                return kind_error(ctx, "\"%s\" is not a ball name", pin->designator);
            }
            for (other = 0; other < n; other++) {
                if (strcmp(pins[other].designator, pin->designator) == 0) {
                    return kind_error(ctx, "ball \"%s\" is used twice", pin->designator);
                }
            }
        }
        if (kind_string(ctx, pin_json, "name", &pin->name) != 0 ||
            kind_string(ctx, pin_json, "role", &role) != 0) {
            return -1;
        }
        for (r = 0; r < sizeof ROLE_NAMES / sizeof ROLE_NAMES[0]; r++) {
            if (strcmp(role, ROLE_NAMES[r]) == 0) {
                break;
            }
        }
        if (r == sizeof ROLE_NAMES / sizeof ROLE_NAMES[0]) {
            return kind_error(ctx, "unknown role \"%s\"", role);
        }
        pin->role = (PinRole)r;
        if (balls && pin->role == PIN_ABSENT) {
            return kind_error(ctx, "a ball pinout lists only real balls; "
                                   "leave \"absent\" positions out");
        }
        if (cJSON_GetObjectItemCaseSensitive(pin_json, "group") != NULL) {
            if (kind_string(ctx, pin_json, "group", &pin->group) != 0) {
                return -1;
            }
        } else {
            pin->group = "default";
        }
        pin->side = PIN_SIDE_AUTO;
        if (cJSON_GetObjectItemCaseSensitive(pin_json, "side") != NULL) {
            const char *side;

            if (kind_string(ctx, pin_json, "side", &side) != 0) {
                return -1;
            }
            if (strcmp(side, "in") == 0) {
                pin->side = PIN_SIDE_IN;
            } else if (strcmp(side, "out") == 0) {
                pin->side = PIN_SIDE_OUT;
            } else {
                return kind_error(ctx, "unknown side \"%s\" (in or out)", side);
            }
            if (pin->role != PIN_INPUT && pin->role != PIN_OUTPUT && pin->role != PIN_INOUT) {
                return kind_error(ctx, "\"side\" is for input, output and inout pins");
            }
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
        for (f = 0; f < FLAG_COUNT; f++) {
            if (strcmp(flag->valuestring, FLAG_NAMES[f]) == 0) {
                break;
            }
        }
        if (f == FLAG_COUNT) {
            return kind_error(ctx, "unknown flag \"%s\"", flag->valuestring);
        }
        *flags |= 1u << f;
    }
    return 0;
}

/* Checks that every pin of `part` has a pad of its designator in `footprint`
   (`what` and `id` name it in the message). */
static int check_pads(KindContext *ctx, const Part *part, const PartFootprint *footprint,
                      const char *what, const char *id) {
    size_t i, j;

    for (i = 0; i < part->pin_count; i++) {
        for (j = 0; j < footprint->pad_count; j++) {
            const char *designator = footprint->pads[j].designator;

            if (designator != NULL && strcmp(designator, part->pins[i].designator) == 0) {
                break;
            }
        }
        if (j == footprint->pad_count) {
            return kind_error(ctx, "pin \"%s\" has no pad in %s \"%s\"",
                              part->pins[i].designator, what, id);
        }
    }
    return 0;
}

/* Resolves the footprint id under `key` ("footprint" or "connectorFootprint")
   into *out and checks that every pin has a pad of its designator. The
   footprint has one pad per pin, or (extra_pads set: a pull resistor, whose
   pad 2 is the rail end) at least one per pin. */
static int load_part_footprint(KindContext *ctx, const PartLibrary *library, const cJSON *part_json,
                               const char *key, const char *pinout_name, const Part *part,
                               int extra_pads, const PartFootprint **out) {
    const PartFootprint *footprint;
    const char *id;

    if (kind_string(ctx, part_json, key, &id) != 0) {
        return -1;
    }
    footprint = part_library_find_footprint(library, id);
    if (footprint == NULL) {
        return kind_error(ctx, "unknown %s \"%s\"", key, id);
    }
    if (extra_pads ? footprint->pins < part->pin_count : footprint->pins != part->pin_count) {
        return kind_error(ctx, "%s \"%s\" has %u pads, pinout \"%s\" has %zu pins", key, id,
                          footprint->pins, pinout_name, part->pin_count);
    }
    if (check_pads(ctx, part, footprint, key, id) != 0) {
        return -1;
    }
    *out = footprint;
    return 0;
}

static const char FOOTPRINT_CHOICES_SHAPE[] =
    "\"footprintChoices\" must be a non-empty array of {\"package\", \"footprint\"}";

/* Optional "footprintChoices": the footprints a placed part may use. Each
   must have a pad for every pin; it may have more pads (unconnected copper).
   The part's own footprint (already loaded) must be one of them, with the
   part's package. */
static int load_footprint_choices(KindContext *ctx, const PartLibrary *library,
                                  const cJSON *part_json, Part *part) {
    const cJSON *array = cJSON_GetObjectItemCaseSensitive(part_json, "footprintChoices");
    const cJSON *entry;
    PartFootprintChoice *choices;
    size_t n = 0, k, default_index = (size_t)-1;

    if (array == NULL) {
        return 0;
    }
    if (!cJSON_IsArray(array) || cJSON_GetArraySize(array) == 0) {
        return kind_error(ctx, "%s", FOOTPRINT_CHOICES_SHAPE);
    }
    choices = kind_alloc(ctx, (size_t)cJSON_GetArraySize(array) * sizeof(*choices));
    if (choices == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(entry, array) {
        const cJSON *package = cJSON_GetObjectItemCaseSensitive(entry, "package");
        const cJSON *id = cJSON_GetObjectItemCaseSensitive(entry, "footprint");
        const PartFootprint *footprint;

        if (!cJSON_IsObject(entry) || cJSON_GetArraySize(entry) != 2 ||
            !cJSON_IsString(package) || package->valuestring[0] == '\0' ||
            !cJSON_IsString(id)) {
            return kind_error(ctx, "%s", FOOTPRINT_CHOICES_SHAPE);
        }
        footprint = part_library_find_footprint(library, id->valuestring);
        if (footprint == NULL) {
            return kind_error(ctx, "unknown footprintChoices footprint \"%s\"", id->valuestring);
        }
        if (check_pads(ctx, part, footprint, "footprintChoices footprint", id->valuestring) != 0) {
            return -1;
        }
        for (k = 0; k < n; k++) {
            if (choices[k].footprint == footprint) {
                return kind_error(ctx, "footprintChoices footprint \"%s\" listed twice",
                                  id->valuestring);
            }
        }
        choices[n].package = kind_strdup(ctx, package->valuestring);
        if (choices[n].package == NULL) {
            return -1;
        }
        choices[n].footprint = footprint;
        if (footprint == part->footprint) {
            default_index = n;
        }
        n++;
    }
    if (default_index == (size_t)-1) {
        return kind_error(ctx, "\"footprint\" \"%s\" is not in footprintChoices",
                          part->footprint->id);
    }
    if (strcmp(choices[default_index].package, part->package) != 0) {
        return kind_error(ctx, "\"package\" \"%s\" does not match footprintChoices \"%s\" (\"%s\")",
                          part->package, part->footprint->id, choices[default_index].package);
    }
    part->footprint_choices = choices;
    part->footprint_choice_count = n;
    return 0;
}

/* The optional "manufacturer" and "datasheet" keys: a datasheet needs a
   manufacturer, and a "Generic" part has none. */
static int load_part_source(KindContext *ctx, const PartLibrary *library,
                            const cJSON *part_json, Part *part) {
    const char *id;

    part->manufacturer = NULL;
    part->datasheet = NULL;
    if (cJSON_GetObjectItemCaseSensitive(part_json, "manufacturer") != NULL) {
        if (kind_string(ctx, part_json, "manufacturer", &part->manufacturer) != 0) {
            return -1;
        }
        if (part->manufacturer[0] == '\0') {
            return kind_error(ctx, "empty \"manufacturer\"");
        }
    }
    if (cJSON_GetObjectItemCaseSensitive(part_json, "datasheet") == NULL) {
        return 0;
    }
    if (part->manufacturer == NULL) {
        return kind_error(ctx, "\"datasheet\" needs a \"manufacturer\"");
    }
    if (strcmp(part->manufacturer, "Generic") == 0) {
        return kind_error(ctx, "a Generic part has no datasheet");
    }
    if (kind_string(ctx, part_json, "datasheet", &id) != 0) {
        return -1;
    }
    part->datasheet = part_library_find_datasheet(library, id);
    if (part->datasheet == NULL) {
        return kind_error(ctx, "unknown datasheet \"%s\"", id);
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
    int is_pull;

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
    if (load_part_source(ctx, library, part_json, part) != 0) {
        return -1;
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
    /* Runs before the footprint keys so part->supply is set (no kind reads
       the footprint). */
    if (ops->load_part(ctx, device, pinouts[p].bound, part_json, part) != 0 || kind_failed(ctx)) {
        return kind_error(ctx, "invalid part");
    }
    is_pull = ops->kind == PART_SUPPLY && part->supply != NULL && part->supply->pull;
    /* Every part but VCC/GND names its footprint; a pull part is a resistor. */
    if (ops->kind == PART_SUPPLY && !is_pull &&
        (cJSON_GetObjectItemCaseSensitive(part_json, "footprint") != NULL ||
         cJSON_GetObjectItemCaseSensitive(part_json, "connectorFootprint") != NULL ||
         cJSON_GetObjectItemCaseSensitive(part_json, "footprintChoices") != NULL)) {
        return kind_error(ctx, "a supply has no footprint");
    }
    if (is_pull && cJSON_GetObjectItemCaseSensitive(part_json, "connectorFootprint") != NULL) {
        return kind_error(ctx, "a pull part has no connectorFootprint");
    }
    if (cJSON_GetObjectItemCaseSensitive(part_json, "footprint") != NULL &&
        load_part_footprint(ctx, library, part_json, "footprint", pinout_name, part, is_pull,
                            &part->footprint) != 0) {
        return -1;
    }
    if (cJSON_GetObjectItemCaseSensitive(part_json, "footprintChoices") != NULL) {
        if (part->footprint == NULL) {
            return kind_error(ctx, "missing \"footprint\"");
        }
        if (load_footprint_choices(ctx, library, part_json, part) != 0) {
            return -1;
        }
    }
    /* Optional: the second footprint the PCB Style option switches to. */
    if (cJSON_GetObjectItemCaseSensitive(part_json, "connectorFootprint") != NULL &&
        load_part_footprint(ctx, library, part_json, "connectorFootprint", pinout_name, part, 0,
                            &part->connector_footprint) != 0) {
        return -1;
    }
    if ((ops->kind != PART_SUPPLY || is_pull) && part->footprint == NULL) {
        return kind_error(ctx, "missing \"footprint\"");
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
    if ((!file->single && name_set_contains(&library->devices, device_key)) ||
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

/* Hands a loaded file's arena to the library. */
static void adopt_arena(PartLibrary *library, KindContext *ctx) {
    ArenaChunk *last = ctx->arena;

    if (last != NULL) {
        while (last->next != NULL) {
            last = last->next;
        }
        last->next = library->arenas;
        library->arenas = ctx->arena;
        ctx->arena = NULL;
    }
}

/* Sets *error (or frees the message) the way load_parts and load_footprints report. */
static void report(KindContext *ctx, int result, char **error) {
    if (result != 0 && error != NULL) {
        *error = ctx->error != NULL ? ctx->error : copy_message("out of memory");
    } else {
        free(ctx->error);
    }
}

/* A footprint list grown by `extra`, built on the side (0, or -1 out of memory). */
typedef struct {
    const PartFootprint **all;
    size_t count;
    NameSet ids;
} FootprintTable;

static int footprint_table_build(const PartLibrary *library, const FootprintList *extra,
                                 FootprintTable *out) {
    size_t needed = library->footprint_count + extra->count;
    size_t i;
    int ok;

    memset(out, 0, sizeof *out);
    out->all = malloc((needed > 0 ? needed : 1) * sizeof(*out->all));
    ok = out->all != NULL && name_set_merge(&out->ids, &library->footprint_ids) == 0;
    for (i = 0; ok && i < extra->count; i++) {
        ok = name_set_add(&out->ids, extra->items[i]->id, extra->items[i]) == 0;
    }
    if (!ok) {
        free(out->all);
        name_set_free(&out->ids);
        memset(out, 0, sizeof *out);
        return -1;
    }
    if (library->footprint_count > 0) {
        memcpy(out->all, library->footprints, library->footprint_count * sizeof(*out->all));
    }
    if (extra->count > 0) {
        memcpy(out->all + library->footprint_count, extra->items, extra->count * sizeof(*out->all));
    }
    out->count = needed;
    return 0;
}

static void footprint_table_adopt(PartLibrary *library, FootprintTable *table) {
    free(library->footprints);
    name_set_free(&library->footprint_ids);
    library->footprints = table->all;
    library->footprint_count = table->count;
    library->footprint_ids = table->ids;
}

/*
 * Adds a loaded file's parts (and, for a single part file, its footprints and
 * its uuid) to the library, all or nothing: every grown table is built on the
 * side first, so an out-of-memory failure leaves the library as it was. Takes
 * the context's arena on success.
 */
static int commit_parts(KindContext *ctx, PartLibrary *library, const FileParts *file,
                        const FootprintList *footprints, const char *uuid) {
    size_t needed = library->count + file->count;
    NameSet names = {0};
    NameSet devices = {0};
    NameSet uuids = {0};
    FootprintTable table = {0};
    int ok;

    if (needed > library->capacity) {
        size_t capacity = library->capacity == 0 ? 1024 : library->capacity;
        const Part **parts;

        while (capacity < needed) {
            capacity *= 2;
        }
        /* Growing the array alone changes nothing the library reports. */
        parts = realloc(library->parts, capacity * sizeof(*parts));
        if (parts == NULL) {
            return kind_error(ctx, "out of memory");
        }
        library->parts = parts;
        library->capacity = capacity;
    }
    ok = name_set_merge(&names, &library->names) == 0 &&
         name_set_merge(&names, &file->names) == 0 &&
         name_set_merge(&devices, &library->devices) == 0 &&
         (file->single || name_set_merge(&devices, &file->devices) == 0) &&
         name_set_merge(&uuids, &library->uuids) == 0 &&
         (uuid == NULL || file->count != 1 || name_set_add(&uuids, uuid, file->parts[0]) == 0) &&
         (footprints == NULL || footprints->count == 0 ||
          footprint_table_build(library, footprints, &table) == 0);
    if (!ok) {
        name_set_free(&names);
        name_set_free(&devices);
        name_set_free(&uuids);
        free(table.all);
        name_set_free(&table.ids);
        return kind_error(ctx, "out of memory");
    }
    name_set_free(&library->names);
    name_set_free(&library->devices);
    name_set_free(&library->uuids);
    library->names = names;
    library->devices = devices;
    library->uuids = uuids;
    if (table.all != NULL) {
        footprint_table_adopt(library, &table);
    }
    memcpy(library->parts + library->count, file->parts, file->count * sizeof(*file->parts));
    library->count += file->count;
    adopt_arena(library, ctx);
    return 0;
}

int load_footprints(PartLibrary *library, const char *json, const char *source, char **error) {
    KindContext ctx;
    FootprintList file = {0};
    cJSON *root;
    int result = -1;

    if (error != NULL) {
        *error = NULL;
    }
    if (library == NULL || json == NULL) {
        if (error != NULL) {
            *error = copy_message("load_footprints: no library or text");
        }
        return -1;
    }
    memset(&ctx, 0, sizeof ctx);
    ctx.source = source != NULL ? source : "footprints";

    root = cJSON_ParseWithLength(json, strlen(json));
    if (root == NULL) {
        const char *where = cJSON_GetErrorPtr();

        kind_error(&ctx, "invalid JSON at byte %ld", where != NULL ? (long)(where - json) : -1L);
    } else if (footprint_read_file(&ctx, root, library, &file) == 0 && !kind_failed(&ctx)) {
        FootprintTable table;

        if (footprint_table_build(library, &file, &table) != 0) {
            kind_error(&ctx, "out of memory");
        } else {
            footprint_table_adopt(library, &table);
            adopt_arena(library, &ctx);
            result = 0;
        }
    }

    cJSON_Delete(root);
    free(file.items);
    free_chunks(ctx.arena);
    report(&ctx, result, error);
    return result;
}

static const char *const DATASHEET_FILE_KEYS[] = {"format", "version", "datasheets", NULL};
static const char *const DATASHEET_KEYS[] = {"id", "url", "md", NULL};

/* Reads every entry of a datasheets file into `items` (arena) and `ids`. */
static int read_datasheets(KindContext *ctx, const cJSON *root, const PartLibrary *library,
                           const PartDatasheet ***items, size_t *count, NameSet *ids) {
    const char *format;
    uint64_t version;
    const cJSON *list;
    const cJSON *item;
    size_t n = 0;

    if (kind_check_keys(ctx, root, DATASHEET_FILE_KEYS, NULL) != 0 ||
        kind_string(ctx, root, "format", &format) != 0) {
        return -1;
    }
    if (strcmp(format, "schematical-datasheets") != 0) {
        return kind_error(ctx, "\"format\" must be \"schematical-datasheets\", not \"%s\"",
                          format);
    }
    if (kind_uint(ctx, root, "version", UINT32_MAX, &version) != 0) {
        return -1;
    }
    if (version != 1) {
        return kind_error(ctx, "unsupported version %llu (expected 1)",
                          (unsigned long long)version);
    }
    list = kind_member(ctx, root, "datasheets", 1);
    if (list == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(list)) {
        return kind_error(ctx, "\"datasheets\" must be an array");
    }
    *items = kind_alloc(ctx, ((size_t)cJSON_GetArraySize(list) + 1) * sizeof(**items));
    if (*items == NULL) {
        return -1;
    }
    for (item = list->child; item != NULL; item = item->next) {
        PartDatasheet *sheet;

        kind_push(ctx, "datasheets[%zu]", n);
        sheet = kind_alloc(ctx, sizeof(*sheet));
        if (sheet == NULL || kind_check_keys(ctx, item, DATASHEET_KEYS, NULL) != 0 ||
            kind_string(ctx, item, "id", &sheet->id) != 0 ||
            kind_string(ctx, item, "url", &sheet->url) != 0) {
            return -1;
        }
        if (sheet->id[0] == '\0') {
            return kind_error(ctx, "empty datasheet id");
        }
        if (strncmp(sheet->url, "https://", 8) != 0 || sheet->url[8] == '\0') {
            return kind_error(ctx, "datasheet \"%s\": \"url\" must start with https://",
                              sheet->id);
        }
        sheet->md = NULL;
        if (cJSON_GetObjectItemCaseSensitive(item, "md") != NULL) {
            size_t len;

            if (kind_string(ctx, item, "md", &sheet->md) != 0) {
                return -1;
            }
            /* A relative path to a .md file: "<group>/<id>.md". */
            len = strlen(sheet->md);
            if (len < 4 || strcmp(sheet->md + len - 3, ".md") != 0 || sheet->md[0] == '/' ||
                strstr(sheet->md, "..") != NULL) {
                return kind_error(ctx, "datasheet \"%s\": \"md\" must be a relative path "
                                  "to a .md file", sheet->id);
            }
        }
        if (name_set_contains(&library->datasheet_ids, sheet->id) ||
            name_set_contains(ids, sheet->id)) {
            return kind_error(ctx, "duplicate datasheet id \"%s\"", sheet->id);
        }
        if (name_set_add(ids, sheet->id, sheet) != 0) {
            return kind_error(ctx, "out of memory");
        }
        (*items)[n++] = sheet;
        kind_pop(ctx);
    }
    *count = n;
    return 0;
}

int load_datasheets(PartLibrary *library, const char *json, const char *source, char **error) {
    KindContext ctx;
    const PartDatasheet **items = NULL;
    size_t count = 0;
    NameSet ids = {0};
    cJSON *root;
    int result = -1;

    if (error != NULL) {
        *error = NULL;
    }
    if (library == NULL || json == NULL) {
        if (error != NULL) {
            *error = copy_message("load_datasheets: no library or text");
        }
        return -1;
    }
    memset(&ctx, 0, sizeof ctx);
    ctx.source = source != NULL ? source : "datasheets";

    root = cJSON_ParseWithLength(json, strlen(json));
    if (root == NULL) {
        const char *where = cJSON_GetErrorPtr();

        kind_error(&ctx, "invalid JSON at byte %ld", where != NULL ? (long)(where - json) : -1L);
    } else if (read_datasheets(&ctx, root, library, &items, &count, &ids) == 0 &&
               !kind_failed(&ctx)) {
        size_t needed = library->datasheet_count + count;
        const PartDatasheet **all = malloc((needed > 0 ? needed : 1) * sizeof(*all));
        NameSet merged = {0};

        if (all == NULL || name_set_merge(&merged, &library->datasheet_ids) != 0 ||
            name_set_merge(&merged, &ids) != 0) {
            free(all);
            name_set_free(&merged);
            kind_error(&ctx, "out of memory");
        } else {
            if (library->datasheet_count > 0) {
                memcpy(all, library->datasheets, library->datasheet_count * sizeof(*all));
            }
            if (count > 0) {
                memcpy(all + library->datasheet_count, items, count * sizeof(*all));
            }
            free(library->datasheets);
            name_set_free(&library->datasheet_ids);
            library->datasheets = all;
            library->datasheet_count = needed;
            library->datasheet_ids = merged;
            adopt_arena(library, &ctx);
            result = 0;
        }
    }

    cJSON_Delete(root);
    name_set_free(&ids);
    free_chunks(ctx.arena);
    report(&ctx, result, error);
    return result;
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
    } else if (load_file(&ctx, library, &file, root) == 0 && !kind_failed(&ctx) &&
               commit_parts(&ctx, library, &file, NULL, NULL) == 0) {
        result = 0;
    }

    cJSON_Delete(root);
    free(file.parts);
    name_set_free(&file.names);
    name_set_free(&file.devices);
    free_chunks(ctx.arena);
    report(&ctx, result, error);
    return result;
}

/* ---- Single part files (format "schematical-part") ------------------------ */

static const char *const PART_FILE_KEYS[] = {
    "format", "version", "uuid", "kind", "footprints", "devices", NULL
};

/* 8-4-4-4-12 uppercase hexadecimal digits: Foundation's UUID.uuidString. */
static int valid_uuid(const char *text) {
    size_t i;

    if (strlen(text) != 36) {
        return 0;
    }
    for (i = 0; i < 36; i++) {
        char c = text[i];

        if (i == 8 || i == 13 || i == 18 || i == 23) {
            if (c != '-') {
                return 0;
            }
        } else if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) {
            return 0;
        }
    }
    return 1;
}

/* Checks one part file and loads its footprints and its device into the
   context's arena (`file`, `footprints`); the library itself is not changed. */
static int read_part_file(KindContext *ctx, PartLibrary *library, const cJSON *root,
                          FileParts *file, FootprintList *footprints, const char **uuid) {
    const cJSON *devices;
    const cJSON *device;
    const cJSON *parts;
    const cJSON *inline_footprints;
    const char *format;
    const char *kind_name;
    const PartKindOps *ops;
    uint64_t version;
    int status;

    if (!cJSON_IsObject(root)) {
        return kind_error(ctx, "expected an object");
    }
    if (kind_check_keys(ctx, root, PART_FILE_KEYS, NULL) != 0 ||
        kind_string(ctx, root, "format", &format) != 0) {
        return -1;
    }
    if (strcmp(format, "schematical-part") != 0) {
        return kind_error(ctx, "\"format\" must be \"schematical-part\", not \"%s\"", format);
    }
    if (kind_uint(ctx, root, "version", UINT32_MAX, &version) != 0) {
        return -1;
    }
    if (version != 1) {
        return kind_error(ctx, "unsupported version %llu (expected 1)",
                          (unsigned long long)version);
    }
    if (kind_string(ctx, root, "uuid", uuid) != 0) {
        return -1;
    }
    if (!valid_uuid(*uuid)) {
        return kind_error(ctx, "\"uuid\" must be an uppercase UUID "
                          "(XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX), not \"%s\"", *uuid);
    }
    if (name_set_contains(&library->uuids, *uuid)) {
        return kind_error(ctx, "uuid \"%s\" is already loaded", *uuid);
    }
    if (kind_string(ctx, root, "kind", &kind_name) != 0) {
        return -1;
    }
    ops = kind_find(kind_name);
    if (ops == NULL) {
        return kind_error(ctx, "unknown kind \"%s\"", kind_name);
    }
    inline_footprints = cJSON_GetObjectItemCaseSensitive(root, "footprints");
    if (inline_footprints != NULL &&
        (footprint_read_entries(ctx, inline_footprints, library, footprints) != 0 ||
         kind_failed(ctx))) {
        return -1;
    }
    devices = kind_member(ctx, root, "devices", 1);
    if (devices == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(devices) || cJSON_GetArraySize(devices) != 1) {
        return kind_error(ctx, "\"devices\" must be an array of exactly one device");
    }
    device = devices->child;
    kind_push(ctx, "devices[0]");
    parts = cJSON_IsObject(device) ? cJSON_GetObjectItemCaseSensitive(device, "parts") : NULL;
    if (cJSON_IsArray(parts) && cJSON_GetArraySize(parts) != 1) {
        return kind_error(ctx, "\"parts\" must be an array of exactly one part");
    }
    /* The inline footprints resolve like bundled ones while the part loads. */
    library->staged_footprints = footprints;
    status = load_device(ctx, library, file, ops, device);
    library->staged_footprints = NULL;
    if (status != 0 || kind_failed(ctx)) {
        return -1;
    }
    kind_pop(ctx);
    if (file->count != 1) {
        return kind_error(ctx, "\"devices\" must hold exactly one part");
    }
    ((Part *)file->parts[0])->uuid = *uuid;
    return 0;
}

int load_part_file(PartLibrary *library, const char *json, const char *source, char **error) {
    KindContext ctx;
    FileParts file = {0};
    FootprintList footprints = {0};
    const char *uuid = NULL;
    cJSON *root;
    int result = -1;

    if (error != NULL) {
        *error = NULL;
    }
    if (library == NULL || json == NULL) {
        if (error != NULL) {
            *error = copy_message("load_part_file: no library or text");
        }
        return -1;
    }
    memset(&ctx, 0, sizeof ctx);
    ctx.source = source != NULL ? source : "part";
    file.single = 1;

    root = cJSON_ParseWithLength(json, strlen(json));
    if (root == NULL) {
        const char *where = cJSON_GetErrorPtr();

        kind_error(&ctx, "invalid JSON at byte %ld", where != NULL ? (long)(where - json) : -1L);
    } else if (read_part_file(&ctx, library, root, &file, &footprints, &uuid) == 0 &&
               !kind_failed(&ctx) && commit_parts(&ctx, library, &file, &footprints, uuid) == 0) {
        result = 0;
    }

    library->staged_footprints = NULL;
    cJSON_Delete(root);
    free(file.parts);
    free(footprints.items);
    name_set_free(&file.names);
    name_set_free(&file.devices);
    free_chunks(ctx.arena);
    report(&ctx, result, error);
    return result;
}
