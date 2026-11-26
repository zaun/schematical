/* library/library_load.c: part of the model in library/library.c (split by plan-68-I/J). */
#include "library_load_int.h"
#include "logic_alloc.h"
static int file_add_part(FileParts *file, const Part *part) {
    if (file->count == file->capacity) {
        size_t capacity = file->capacity == 0 ? 256 : file->capacity * 2;
        const Part **parts = logic_realloc(file->parts, capacity * sizeof(*parts));

        if (parts == NULL) {
            return -1;
        }
        file->parts = parts;
        file->capacity = capacity;
    }
    file->parts[file->count++] = part;
    return 0;
}
static int load_active_low(KindContext *ctx, const cJSON *pin_json, PartPin *pin) {
    const cJSON *json = cJSON_GetObjectItemCaseSensitive(pin_json, "activeLow");
    const char *alts[ACTIVE_LOW_MAX];
    size_t lengths[ACTIVE_LOW_MAX];
    size_t count = 0, i;
    const char *start = pin->name;
    uint32_t all, mask = 0;
    const cJSON *item;

    pin->active_low = 0;
    if (json == NULL) {
        return 0;
    }
    if (pin->role != PIN_INPUT && pin->role != PIN_OUTPUT && pin->role != PIN_INOUT) {
        return kind_error(ctx, "\"activeLow\" is for input, output and inout pins");
    }
    if (!cJSON_IsTrue(json) && !cJSON_IsArray(json)) {
        return kind_error(ctx, "\"activeLow\" must be true or a list of the name's alternatives");
    }
    for (;;) {
        const char *slash = strchr(start, '/');
        size_t length = slash != NULL ? (size_t)(slash - start) : strlen(start);

        if (count < ACTIVE_LOW_MAX) {
            alts[count] = start;
            lengths[count] = length;
        }
        count++;
        if (slash == NULL) {
            break;
        }
        start = slash + 1;
    }
    if (cJSON_IsTrue(json)) {
        pin->active_low = count >= ACTIVE_LOW_MAX ? 0xFFFFFFFFu
                                                  : (uint32_t)((1u << count) - 1u);
        return 0;
    }
    cJSON_ArrayForEach(item, json) {
        if (!cJSON_IsString(item)) {
            return kind_error(ctx, "\"activeLow\" must be true or a list of the name's alternatives");
        }
    }
    if (cJSON_GetArraySize(json) == 0) {
        return kind_error(ctx, "\"activeLow\" is empty");
    }
    if (count > ACTIVE_LOW_MAX) {
        return kind_error(ctx, "\"activeLow\" is for names of at most 32 alternatives");
    }
    all = count == ACTIVE_LOW_MAX ? 0xFFFFFFFFu : (uint32_t)((1u << count) - 1u);
    cJSON_ArrayForEach(item, json) {
        const char *text = item->valuestring;
        uint32_t bits = 0;
        const cJSON *other;

        for (other = json->child; other != item; other = other->next) {
            if (strcmp(other->valuestring, text) == 0) {
                return kind_error(ctx, "\"activeLow\" lists \"%s\" twice", text);
            }
        }
        for (i = 0; i < count; i++) {
            if (strlen(text) == lengths[i] && strncmp(alts[i], text, lengths[i]) == 0) {
                bits |= 1u << i;
            }
        }
        if (bits == 0) {
            return kind_error(ctx, "\"activeLow\" has no alternative \"%s\" in \"%s\"", text,
                              pin->name);
        }
        mask |= bits;
    }
    if (mask == all) {
        return kind_error(ctx, "\"activeLow\" lists every alternative of \"%s\"; write true",
                          pin->name);
    }
    pin->active_low = mask;
    return 0;
}

int lib_load_pinout(KindContext *ctx, const cJSON *json, Pinout *out) {
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
        if (kind_check_keys(ctx, pin_json, lib_PIN_KEYS, NULL) != 0) {
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
        for (r = 0; r < sizeof lib_ROLE_NAMES / sizeof lib_ROLE_NAMES[0]; r++) {
            if (strcmp(role, lib_ROLE_NAMES[r]) == 0) {
                break;
            }
        }
        if (r == sizeof lib_ROLE_NAMES / sizeof lib_ROLE_NAMES[0]) {
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
        if (load_active_low(ctx, pin_json, pin) != 0) {
            return -1;
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
            if (strcmp(flag->valuestring, lib_FLAG_NAMES[f]) == 0) {
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

    /* The one caller calls only when "footprintChoices" is there. */
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

static const char FITS_SHAPE[] = "\"fits\" must be a non-empty array of footprint ids";

/* "fits" (plan-51-A): the chip footprints a socket holds, by id. Required on
   a socket part, rejected on every other kind. Each footprint must exist, be
   listed once, have the part's pin count and a pad for every pin. */
static int load_fits(KindContext *ctx, const PartLibrary *library, const cJSON *part_json,
                     const PartKindOps *ops, Part *part) {
    const cJSON *array = cJSON_GetObjectItemCaseSensitive(part_json, "fits");
    const cJSON *entry;
    const PartFootprint **fits;
    size_t n = 0, k;

    if (ops->kind != PART_SOCKET) {
        return array == NULL ? 0 : kind_error(ctx, "\"fits\" is only for a socket");
    }
    if (array == NULL) {
        return kind_error(ctx, "missing \"fits\"");
    }
    if (!cJSON_IsArray(array) || cJSON_GetArraySize(array) == 0) {
        return kind_error(ctx, "%s", FITS_SHAPE);
    }
    fits = kind_alloc(ctx, (size_t)cJSON_GetArraySize(array) * sizeof(*fits));
    if (fits == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(entry, array) {
        const PartFootprint *footprint;

        if (!cJSON_IsString(entry)) {
            return kind_error(ctx, "%s", FITS_SHAPE);
        }
        footprint = part_library_find_footprint(library, entry->valuestring);
        if (footprint == NULL) {
            return kind_error(ctx, "unknown fits footprint \"%s\"", entry->valuestring);
        }
        for (k = 0; k < n; k++) {
            if (fits[k] == footprint) {
                return kind_error(ctx, "fits footprint \"%s\" listed twice", entry->valuestring);
            }
        }
        if (footprint->pins != part->pin_count) {
            return kind_error(ctx, "fits footprint \"%s\" has %u pins; the part has %zu",
                              entry->valuestring, footprint->pins, part->pin_count);
        }
        if (check_pads(ctx, part, footprint, "fits footprint", entry->valuestring) != 0) {
            return -1;
        }
        fits[n++] = footprint;
    }
    part->fits = fits;
    part->fit_count = n;
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

int lib_load_part(KindContext *ctx, PartLibrary *library, FileParts *file,
                     const PartKindOps *ops, const cJSON *part_json, const void *device,
                     const Part *common, const Pinout *pinouts, size_t pinout_count) {
    Part *part;
    PartInternal *internal;
    const char *pinout_name;
    size_t p;
    int is_pull;

    if (kind_check_keys(ctx, part_json, lib_PART_KEYS, ops->part_keys) != 0) {
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
    if (lib_name_set_contains(&library->names, part->name) ||
        lib_name_set_contains(&file->names, part->name)) {
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
    if (load_fits(ctx, library, part_json, ops, part) != 0) {
        return -1;
    }
    if (lib_name_set_add(&file->names, part->name, part) != 0 || file_add_part(file, part) != 0) {
        return kind_error(ctx, "out of memory");
    }
    return 0;
}

