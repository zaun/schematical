/* models/eeprom_kind.c: part of the model in models/eeprom.c (split by plan-68-I/J). */
#include "eeprom_int.h"
#include "logic_alloc.h"
/* ---- Kind: devices, pinouts, parts --------------------------------------- */

static const char *const DEVICE_KEYS[] = {
    "address", "data", "chipEnable", "outputEnable", "writeEnable", "pageSize",
    "commandAddresses", "protection", "grades", NULL
};
static const char *const PART_KEYS[] = {"grade", "vcc", "inputMax", NULL};

/* The timing names, in EepromTiming order. */
static const char *const TIMING_NAMES[] = {
    "tacc", "tce", "toe", "tdf", "toh", "tas", "tah", "tcs", "tch", "twp", "twph", "tds", "tdh",
    "toes", "toeh", "tblc", "twc", "note", "vcc", "inputMax", NULL
};

static int load_grade(KindContext *ctx, const cJSON *json, EepromTiming *t) {
    SimTime *fields[] = {
        &t->tacc, &t->tce, &t->toe, &t->tdf, &t->toh, &t->tas, &t->tah, &t->tcs, &t->tch,
        &t->twp, &t->twph, &t->tds, &t->tdh, &t->toes, &t->toeh, &t->tblc, &t->twc
    };
    const cJSON *note = cJSON_GetObjectItemCaseSensitive(json, "note");
    size_t i;

    if (!cJSON_IsObject(json)) {
        return kind_error(ctx, "a grade is an object of timing values");
    }
    if (kind_check_keys(ctx, json, TIMING_NAMES, NULL) != 0) {
        return -1;
    }
    for (i = 0; i < EE_COUNT(fields); i++) {
        const cJSON *value = cJSON_GetObjectItemCaseSensitive(json, TIMING_NAMES[i]);

        *fields[i] = 0;
        if (value != NULL && kind_ns(ctx, value, TIMING_NAMES[i], fields[i]) != 0) {
            return -1;
        }
    }
    if (t->tblc == 0) {
        return kind_error(ctx, "\"tblc\" must be more than 0");
    }
    if (t->twc == 0) {
        return kind_error(ctx, "\"twc\" must be more than 0");
    }
    if (note != NULL && !cJSON_IsString(note)) {
        return kind_error(ctx, "\"note\" must be a string");
    }
    return 0;
}

/* A hex address of [0-9A-F]+ below words. */
static int load_command_address(KindContext *ctx, const cJSON *item, size_t index, uint32_t words,
                                uint32_t *out) {
    const char *text = cJSON_IsString(item) ? item->valuestring : NULL;
    uint64_t value = 0;
    size_t i;

    if (text == NULL || text[0] == '\0') {
        return kind_error(ctx, "commandAddresses[%zu] must be a hex string", index);
    }
    for (i = 0; text[i] != '\0'; i++) {
        char c = text[i];

        if (c >= '0' && c <= '9') {
            value = value * 16 + (uint64_t)(c - '0');
        } else if (c >= 'A' && c <= 'F') {
            value = value * 16 + (uint64_t)(c - 'A' + 10);
        } else {
            return kind_error(ctx, "commandAddresses[%zu] \"%s\" is not a hex address (0-9, A-F)",
                              index, text);
        }
        if (value >= words) {
            break;
        }
    }
    if (value >= words) {
        return kind_error(ctx, "commandAddresses[%zu] \"%s\" is past the last address (%X)", index,
                          text, (unsigned)(words - 1));
    }
    *out = (uint32_t)value;
    return 0;
}

static void *eeprom_load_device(KindContext *ctx, const cJSON *json) {
    EepromDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *grades = kind_member(ctx, json, "grades", 1);
    const cJSON *commands;
    const char *protection;
    uint64_t page_size;
    uint32_t words;
    const cJSON *item;
    size_t n = 0;

    if (device == NULL || grades == NULL ||
        kind_names(ctx, json, "address", &device->address, &device->address_count) != 0) {
        return NULL;
    }
    if (device->address_count < 1 || device->address_count > EE_MAX_ADDRESS_BITS) {
        kind_error(ctx, "\"address\" needs 1 to %d pin names", EE_MAX_ADDRESS_BITS);
        return NULL;
    }
    words = (uint32_t)1 << device->address_count;
    if (kind_names(ctx, json, "data", &device->data, &device->data_count) != 0) {
        return NULL;
    }
    if (device->data_count != EE_DATA_BITS) {
        kind_error(ctx, "\"data\" needs exactly %d pin names", EE_DATA_BITS);
        return NULL;
    }
    if (kind_string(ctx, json, "chipEnable", &device->ce) != 0 ||
        kind_string(ctx, json, "outputEnable", &device->oe) != 0 ||
        kind_string(ctx, json, "writeEnable", &device->we) != 0 ||
        kind_uint(ctx, json, "pageSize", (uint64_t)1 << EE_MAX_ADDRESS_BITS, &page_size) != 0) {
        return NULL;
    }
    if (page_size == 0 || (page_size & (page_size - 1)) != 0) {
        kind_error(ctx, "\"pageSize\" must be a power of two");
        return NULL;
    }
    if (page_size > words) {
        kind_error(ctx, "\"pageSize\" %llu is more than the %u words",
                   (unsigned long long)page_size, (unsigned)words);
        return NULL;
    }
    device->page_size = (uint32_t)page_size;

    commands = kind_member(ctx, json, "commandAddresses", 1);
    if (commands == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(commands) || cJSON_GetArraySize(commands) != 2) {
        kind_error(ctx, "\"commandAddresses\" needs two hex strings");
        return NULL;
    }
    if (load_command_address(ctx, cJSON_GetArrayItem(commands, 0), 0, words, &device->command[0]) != 0 ||
        load_command_address(ctx, cJSON_GetArrayItem(commands, 1), 1, words, &device->command[1]) != 0) {
        return NULL;
    }

    if (kind_string(ctx, json, "protection", &protection) != 0) {
        return NULL;
    }
    if (strcmp(protection, "off") == 0) {
        device->protection = PROTECTION_OFF;
    } else if (strcmp(protection, "always") == 0) {
        device->protection = PROTECTION_ALWAYS;
    } else {
        kind_error(ctx, "\"protection\" must be \"off\" or \"always\", not \"%s\"", protection);
        return NULL;
    }

    if (!cJSON_IsObject(grades) || grades->child == NULL) {
        kind_error(ctx, "\"grades\" must be a non-empty object");
        return NULL;
    }
    device->grade_count = (size_t)cJSON_GetArraySize(grades);
    device->grade_names = kind_alloc(ctx, device->grade_count * sizeof(*device->grade_names));
    device->grades = kind_alloc(ctx, device->grade_count * sizeof(*device->grades));
    if (device->grade_names == NULL || device->grades == NULL) {
        return NULL;
    }
    cJSON_ArrayForEach(item, grades) {
        EepromGrade *grade = &device->grades[n];
        const cJSON *other;

        for (other = grades->child; other != item; other = other->next) {
            if (strcmp(other->string, item->string) == 0) {
                kind_error(ctx, "duplicate key \"%s\"", item->string);
                return NULL;
            }
        }
        kind_push(ctx, "grades.%s", item->string);
        device->grade_names[n] = kind_strdup(ctx, item->string);
        if (device->grade_names[n] == NULL || load_grade(ctx, item, &grade->timing) != 0 ||
            kind_voltage(ctx, item, NULL, &grade->vcc) != 0) {
            return NULL;
        }
        kind_pop(ctx);
        grade->info.words = words;
        grade->info.bits = EE_DATA_BITS;
        grade->info.access_ps = grade->timing.tacc;
        grade->info.grade = device->grade_names[n];
        grade->info.page_bytes = device->page_size;
        grade->info.write_cycle_ps = grade->timing.twc;
        n++;
    }
    return device;
}

static int bind_list(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *key,
                     const char *const *names, size_t count, size_t *out) {
    size_t i;

    for (i = 0; i < count; i++) {
        kind_push(ctx, "%s[%zu]", key, i);
        if (kind_pin(ctx, pins, pin_count, names[i], &out[i]) != 0) {
            return -1;
        }
        kind_pop(ctx);
    }
    return 0;
}

static int bind_one(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *key,
                    const char *name, size_t *out) {
    kind_push(ctx, "%s", key);
    if (kind_pin(ctx, pins, pin_count, name, out) != 0) {
        return -1;
    }
    kind_pop(ctx);
    return 0;
}

static void *eeprom_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                size_t pin_count) {
    const EepromDevice *device = raw;
    EepromPins *bound = kind_alloc(ctx, sizeof(*bound));

    if (bound == NULL ||
        bind_list(ctx, pins, pin_count, "address", device->address, device->address_count, bound->addr) != 0 ||
        bind_list(ctx, pins, pin_count, "data", device->data, device->data_count, bound->dq) != 0 ||
        bind_one(ctx, pins, pin_count, "chipEnable", device->ce, &bound->ce) != 0 ||
        bind_one(ctx, pins, pin_count, "outputEnable", device->oe, &bound->oe) != 0 ||
        bind_one(ctx, pins, pin_count, "writeEnable", device->we, &bound->we) != 0) {
        return NULL;
    }
    bound->device = device;
    return bound;
}

static int eeprom_load_part(KindContext *ctx, const void *raw, const void *bound,
                            const cJSON *json, Part *out) {
    const EepromDevice *device = raw;
    const char *grade;
    size_t i;

    (void)bound;
    if (kind_string(ctx, json, "grade", &grade) != 0) {
        return -1;
    }
    for (i = 0; i < device->grade_count; i++) {
        if (strcmp(device->grade_names[i], grade) == 0) {
            out->eeprom = &device->grades[i].info;
            kind_set_part_data(out, &device->grades[i]);
            return kind_part_voltage(ctx, json, device->grades[i].vcc, &out->vcc);
        }
    }
    return kind_error(ctx, "grade \"%s\" is not in \"grades\"", grade);
}

/* ---- Model creation ------------------------------------------------------ */

static size_t eeprom_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void eeprom_destroy(void *raw);

static void *eeprom_create(const void *config) {
    const PartInternal *internal = part_internal(config);
    const EepromPins *pins = internal->bound;
    const EepromDevice *device = pins->device;
    const EepromGrade *grade = internal->data;
    EepromState *state = logic_calloc(1, sizeof(*state));
    size_t i;

    if (state == NULL) {
        return NULL;
    }
    state->t = &grade->timing;
    state->address_bits = (unsigned)device->address_count;
    state->word_count = (uint32_t)1 << device->address_count;
    state->page_size = device->page_size;
    state->command_a = device->command[0];
    state->command_b = device->command[1];
    state->protection = device->protection;
    for (i = 0; i < EE_MAX_ADDRESS_BITS; i++) {
        state->addr_pin[i] = i < device->address_count ? pins->addr[i] : EE_NO_PIN;
    }
    for (i = 0; i < EE_DATA_BITS; i++) {
        state->dq_pin[i] = pins->dq[i];
    }
    state->ce_pin = pins->ce;
    state->oe_pin = pins->oe;
    state->we_pin = pins->we;

    state->value = logic_malloc(state->word_count);
    state->known = logic_malloc(state->word_count);
    state->buf_value = logic_calloc(state->page_size, 1);
    state->buf_known = logic_calloc(state->page_size, 1);
    state->buf_loaded = logic_calloc(state->page_size, 1);
    if (state->value == NULL || state->known == NULL || state->buf_value == NULL ||
        state->buf_known == NULL || state->buf_loaded == NULL) {
        eeprom_destroy(state);
        return NULL;
    }
    /* Erased: every byte FF. */
    memset(state->value, 0xFF, state->word_count);
    memset(state->known, 0xFF, state->word_count);
    return state;
}

/* Never NULL: the engine destroys only states create returned, and eeprom_create passes its own. */
static void eeprom_destroy(void *raw) {
    EepromState *state = raw;

    logic_free(state->value);
    logic_free(state->known);
    logic_free(state->buf_value);
    logic_free(state->buf_known);
    logic_free(state->buf_loaded);
    logic_free(state);
}

static const SimComponentType EEPROM_MODEL = {
    "eeprom", eeprom_pin_count, eeprom_create, eeprom_destroy, eep_eeprom_start, eep_eeprom_pin_changed,
    eep_eeprom_timer, NULL, NULL, eep_eeprom_memory_size, eep_eeprom_memory_read, NULL, eep_eeprom_memory_load
};

const PartKindOps EEPROM_KIND = {
    PART_EEPROM, "eeprom", &EEPROM_MODEL, DEVICE_KEYS, PART_KEYS,
    eeprom_load_device, eeprom_bind_pinout, eeprom_load_part
};
