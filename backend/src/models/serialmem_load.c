/* models/serialmem_load.c: part of the model in models/serialmem.c (split by plan-68-I/J). */
#include "serialmem_int.h"
/* ---- Loader -------------------------------------------------------------------------------- */

static const cJSON *member(const cJSON *json, const char *key) {
    return cJSON_GetObjectItemCaseSensitive(json, key);
}

/* An optional time in ns (up to 1e11, more than kind_ns takes: a chip erase lasts seconds). */
static int load_time(KindContext *ctx, const cJSON *json, const char *key, SimTime *out) {
    const cJSON *item = member(json, key);

    if (item == NULL) {
        return 0;
    }
    if (!cJSON_IsNumber(item) || !(item->valuedouble >= 0) || item->valuedouble > 1e11) {
        return kind_error(ctx, "\"%s\" must be a time in ns from 0 to 1e11", key);
    }
    *out = (SimTime)llround(item->valuedouble * 1000.0);
    return 0;
}

static int load_uint(KindContext *ctx, const cJSON *json, const char *key, uint64_t max,
                     uint64_t *out) {
    if (member(json, key) == NULL) {
        return 0;
    }
    return kind_uint(ctx, json, key, max, out);
}

static int load_flag(KindContext *ctx, const cJSON *json, const char *key, int *out) {
    if (member(json, key) == NULL) {
        return 0;
    }
    return kind_bool(ctx, json, key, out);
}

static int power_of_two(uint64_t v) {
    return v != 0 && (v & (v - 1)) == 0;
}

/* A clock in MHz (a number above 0) as kHz. */
static int load_clock(KindContext *ctx, const cJSON *json, uint32_t *khz) {
    const cJSON *item = member(json, "maxClock");

    if (item == NULL) {
        return 0;
    }
    if (!cJSON_IsNumber(item) || !(item->valuedouble > 0) || item->valuedouble > 4000) {
        return kind_error(ctx, "\"maxClock\" must be a clock in MHz above 0 and at most 4000");
    }
    *khz = (uint32_t)llround(item->valuedouble * 1000.0);
    return 0;
}

static int hex_digit(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

static int load_jedec(KindContext *ctx, const cJSON *json, SerialmemDevice *d) {
    const cJSON *item = member(json, "jedecId");
    size_t length, i;

    if (item == NULL) {
        return 0;
    }
    if (!cJSON_IsString(item)) {
        return kind_error(ctx, "\"jedecId\" must be a string of hex digits");
    }
    length = strlen(item->valuestring);
    if (length == 0 || length % 2 != 0 || length > 2 * SM_MAX_ID) {
        return kind_error(ctx, "\"jedecId\" must be 1 to %d bytes of hex digits", SM_MAX_ID);
    }
    for (i = 0; i < length; i++) {
        if (hex_digit(item->valuestring[i]) < 0) {
            return kind_error(ctx, "\"jedecId\" \"%s\" is not hex digits", item->valuestring);
        }
    }
    for (i = 0; i < length / 2; i++) {
        d->jedec[i] = (uint8_t)(hex_digit(item->valuestring[2 * i]) * 16 +
                                hex_digit(item->valuestring[2 * i + 1]));
    }
    d->jedec_len = (unsigned)(length / 2);
    d->facts.jedec_id = kind_strdup(ctx, item->valuestring);
    return d->facts.jedec_id != NULL ? 0 : -1;
}

static int choice(KindContext *ctx, const cJSON *json, const char *key, const char *const *names,
                  int *out) {
    const char *text;
    int i;

    if (kind_string(ctx, json, key, &text) != 0) {
        return -1;
    }
    for (i = 0; names[i] != NULL; i++) {
        if (strcmp(names[i], text) == 0) {
            *out = i;
            return 0;
        }
    }
    return kind_error(ctx, "unknown %s \"%s\"", key, text);
}

static int check_simulated(KindContext *ctx, const SerialmemDevice *d) {
    int flash = d->type == SM_FLASH;

    if (!power_of_two(d->size) || d->size < 4 || d->size > SM_MAX_SIZE) {
        return kind_error(ctx, "\"size\" must be a power of two from 4 to %llu bytes",
                          (unsigned long long)SM_MAX_SIZE);
    }
    if (d->pin_name[SM_CS] == NULL || d->pin_name[SM_SCK] == NULL || d->pin_name[SM_SI] == NULL ||
        d->pin_name[SM_SO] == NULL) {
        return kind_error(ctx, "a simulated device needs \"chipSelect\", \"clock\", \"dataIn\" and "
                               "\"dataOut\"");
    }
    if (d->pin_name[SM_RESET] != NULL) {
        return kind_error(ctx, "\"reset\" is only for a device with \"simulated\": false (no "
                               "simulated chip has a RESET# pin)");
    }
    if (flash) {
        if (d->address_bytes != 3) {
            return kind_error(ctx, "a flash device takes 3 address bytes");
        }
        if (d->a8) {
            return kind_error(ctx, "\"a8InOpcode\" is for an eeprom or fram device");
        }
        if (d->t_page_prog == 0 || d->t_byte_prog == 0 || d->t_sector == 0 || d->t_block32 == 0 ||
            d->t_chip == 0) {
            return kind_error(ctx, "a flash device needs \"pageProgram\", \"byteProgram\", "
                                   "\"sectorErase\", \"blockErase32\" and \"chipErase\" (more than 0)");
        }
        if (d->size < 65536 && d->t_block64 > 0) {
            return kind_error(ctx, "\"blockErase64\" is more than the %u bytes of the array",
                              (unsigned)d->size);
        }
        if (d->prot != PROT_SECTOR && d->prot != PROT_BLOCK) {
            return kind_error(ctx, "a flash device's \"protection\" is \"sector\" or \"block\"");
        }
        if (d->jedec_len == 0) {
            return kind_error(ctx, "a flash device needs \"jedecId\"");
        }
        if (!power_of_two(d->page_size) || d->page_size > d->size || d->size < 4096) {
            return kind_error(ctx, "a flash device's \"pageSize\" is a power of two within the "
                                   "array, and the array is at least 4096 bytes");
        }
        if (d->prot == PROT_SECTOR &&
            (!power_of_two(d->protect_size) || d->protect_size < d->page_size ||
             d->protect_size > d->size)) {
            return kind_error(ctx, "\"protectSize\" must be a power of two from the page size to "
                                   "the array size");
        }
        if (d->sleep || d->fast_read || d->t_write > 0) {
            return kind_error(ctx, "\"sleep\", \"fastRead\" and \"writeCycle\" are not flash keys");
        }
        return 0;
    }
    if (d->prot != PROT_WPEN && d->prot != PROT_WP) {
        return kind_error(ctx, "an eeprom or fram device's \"protection\" is \"wpen\" or \"wp\"");
    }
    if (d->address_bytes < 1 || d->address_bytes > 3) {
        return kind_error(ctx, "\"addressBytes\" must be 1, 2 or 3");
    }
    if (d->a8 && d->address_bytes != 1) {
        return kind_error(ctx, "\"a8InOpcode\" needs \"addressBytes\" 1");
    }
    if ((uint64_t)d->size > ((uint64_t)1 << (8 * d->address_bytes + (d->a8 ? 1 : 0)))) {
        return kind_error(ctx, "\"size\" %u does not fit %u address byte%s%s", (unsigned)d->size,
                          d->address_bytes, d->address_bytes == 1 ? "" : "s",
                          d->a8 ? " and address bit 8 in the opcode" : "");
    }
    if (d->protect_size != 0 || d->t_page_prog || d->t_byte_prog || d->t_page_erase || d->t_sector ||
        d->t_block32 || d->t_block64 || d->t_chip || d->t_wrsr) {
        return kind_error(ctx, "the flash timing keys and \"protectSize\" are for a flash device");
    }
    if (d->type == SM_EEPROM) {
        if (d->t_write == 0) {
            return kind_error(ctx, "an eeprom device needs \"writeCycle\" (more than 0)");
        }
        if (!power_of_two(d->page_size) || d->page_size > d->size / 4) {
            return kind_error(ctx, "\"pageSize\" must be a power of two, at most a quarter of the "
                                   "array");
        }
    } else {
        if (d->t_write != 0) {
            return kind_error(ctx, "a fram device has no write cycle: leave out \"writeCycle\"");
        }
        if (d->page_size != 0) {
            return kind_error(ctx, "a fram device has no write page: leave out \"pageSize\"");
        }
    }
    if (d->type == SM_EEPROM && (d->sleep || d->fast_read)) {
        return kind_error(ctx, "\"sleep\" and \"fastRead\" are for a fram device");
    }
    if (d->sleep && d->t_wake == 0) {
        return kind_error(ctx, "\"sleep\" needs \"wake\" (more than 0)");
    }
    return 0;
}

static void *sm_load_device(KindContext *ctx, const cJSON *json) {
    SerialmemDevice *d = kind_alloc(ctx, sizeof *d);
    const cJSON *simulated = member(json, "simulated");
    uint64_t v = 0;
    int type = 0, prot = 0;
    size_t i;

    if (d == NULL) {
        return NULL;
    }
    d->simulated = simulated == NULL || cJSON_IsTrue(simulated);
    if (choice(ctx, json, "type", smem_TYPE_NAMES, &type) != 0) {
        return NULL;
    }
    d->type = (SmType)type;
    if (d->simulated && (d->type == SM_DATAFLASH || d->type == SM_PAGEEEPROM)) {
        kind_error(ctx, "a \"%s\" device has no simulation model: \"simulated\" must be false",
                   smem_TYPE_NAMES[type]);
        return NULL;
    }
    if (kind_uint(ctx, json, "size", UINT32_MAX, &v) != 0) {
        return NULL;
    }
    if (v == 0) {
        kind_error(ctx, "\"size\" must be more than 0");
        return NULL;
    }
    d->size = (uint32_t)v;
    d->prot = d->type == SM_FLASH ? PROT_SECTOR : PROT_WPEN;
    if (member(json, "protection") != NULL) {
        if (choice(ctx, json, "protection", smem_PROT_NAMES, &prot) != 0) {
            return NULL;
        }
        d->prot = (SmProt)prot;
    } else if (d->simulated && d->type == SM_FLASH) {
        kind_error(ctx, "missing \"protection\"");
        return NULL;
    }
    d->address_bytes = d->type == SM_FLASH ? 3 : 0;
    if (load_uint(ctx, json, "addressBytes", 3, &v) != 0) {
        return NULL;
    }
    if (member(json, "addressBytes") != NULL) {
        d->address_bytes = (unsigned)v;
    } else if (d->simulated && d->type != SM_FLASH) {
        kind_error(ctx, "missing \"addressBytes\"");
        return NULL;
    }
    d->page_size = d->type == SM_FLASH ? 256 : 0;
    if (load_uint(ctx, json, "pageSize", UINT32_MAX, &v) != 0) {
        return NULL;
    }
    if (member(json, "pageSize") != NULL) {
        d->page_size = (uint32_t)v;
        if (v == 0) {
            kind_error(ctx, "\"pageSize\" must be more than 0");
            return NULL;
        }
    } else if (d->simulated && d->type == SM_EEPROM) {
        kind_error(ctx, "missing \"pageSize\"");
        return NULL;
    }
    if (load_uint(ctx, json, "protectSize", UINT32_MAX, &v) != 0) {
        return NULL;
    }
    d->protect_size = member(json, "protectSize") != NULL ? (uint32_t)v : 0;
    if (d->simulated && d->type == SM_FLASH && d->prot == PROT_SECTOR && d->protect_size == 0) {
        kind_error(ctx, "missing \"protectSize\"");
        return NULL;
    }
    if (load_flag(ctx, json, "a8InOpcode", &d->a8) != 0 ||
        load_flag(ctx, json, "fastRead", &d->fast_read) != 0 ||
        load_flag(ctx, json, "sleep", &d->sleep) != 0 ||
        load_flag(ctx, json, "pullUp", &d->pull_up) != 0) {
        return NULL;
    }
    for (i = 0; i < SM_PINS; i++) {
        if (member(json, smem_PIN_KEY_NAMES[i]) != NULL &&
            kind_string(ctx, json, smem_PIN_KEY_NAMES[i], &d->pin_name[i]) != 0) {
            return NULL;
        }
    }
    if (load_time(ctx, json, "writeCycle", &d->t_write) != 0 ||
        load_time(ctx, json, "pageProgram", &d->t_page_prog) != 0 ||
        load_time(ctx, json, "byteProgram", &d->t_byte_prog) != 0 ||
        load_time(ctx, json, "pageErase", &d->t_page_erase) != 0 ||
        load_time(ctx, json, "sectorErase", &d->t_sector) != 0 ||
        load_time(ctx, json, "blockErase32", &d->t_block32) != 0 ||
        load_time(ctx, json, "blockErase64", &d->t_block64) != 0 ||
        load_time(ctx, json, "chipErase", &d->t_chip) != 0 ||
        load_time(ctx, json, "writeStatus", &d->t_wrsr) != 0 ||
        load_time(ctx, json, "outputValid", &d->t_valid) != 0 ||
        load_time(ctx, json, "outputDisable", &d->t_disable) != 0 ||
        load_time(ctx, json, "wake", &d->t_wake) != 0) {
        return NULL;
    }
    if (load_uint(ctx, json, "statusOnes", 255, &v) != 0) {
        return NULL;
    }
    d->status_ones = member(json, "statusOnes") != NULL ? (uint8_t)v : 0;
    if (load_uint(ctx, json, "busyOnes", 255, &v) != 0) {
        return NULL;
    }
    d->busy_ones = member(json, "busyOnes") != NULL ? (uint8_t)v : 0;
    d->facts.type = smem_TYPE_NAMES[type];
    d->facts.size = d->size;
    d->facts.address_bytes = d->address_bytes;
    d->facts.page_bytes = d->page_size;
    d->facts.write_ps = d->type == SM_FLASH ? d->t_page_prog : d->t_write;
    d->facts.sector_erase_ps = d->t_sector;
    d->facts.chip_erase_ps = d->t_chip;
    if (load_jedec(ctx, json, d) != 0 || load_clock(ctx, json, &d->facts.max_clock_khz) != 0) {
        return NULL;
    }
    if (load_uint(ctx, json, "endurance", (uint64_t)1 << 53, &v) != 0) {
        return NULL;
    }
    d->facts.endurance = member(json, "endurance") != NULL ? v : 0;
    if (load_uint(ctx, json, "retention", 1000000, &v) != 0) {
        return NULL;
    }
    d->facts.retention_years = member(json, "retention") != NULL ? (uint32_t)v : 0;
    if (kind_voltage(ctx, json, NULL, &d->vcc) != 0) {
        return NULL;
    }
    if (d->simulated && check_simulated(ctx, d) != 0) {
        return NULL;
    }
    return d;
}

static int bind_one(KindContext *ctx, const PartPin *pins, size_t pin_count,
                    const SerialmemDevice *d, int which, size_t *out) {
    const char *name = d->pin_name[which];
    PinRole role;

    *out = SM_NONE;
    if (name == NULL) {
        return 0;
    }
    kind_push(ctx, "%s", smem_PIN_KEY_NAMES[which]);
    if (kind_pin(ctx, pins, pin_count, name, out) != 0) {
        return -1;
    }
    role = pins[*out].role;
    if (which == SM_SO ? (role != PIN_OUTPUT && role != PIN_INOUT)
                       : (role != PIN_INPUT && role != PIN_INOUT)) {
        return kind_error(ctx, "pin \"%s\" must be %s", name,
                          which == SM_SO ? "an output or inout pin" : "an input or inout pin");
    }
    kind_pop(ctx);
    return 0;
}

static void *sm_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                            size_t pin_count) {
    const SerialmemDevice *d = raw;
    SerialmemPins *bound = kind_alloc(ctx, sizeof *bound);
    int i;

    if (bound == NULL) {
        return NULL;
    }
    for (i = 0; i < SM_PINS; i++) {
        if (bind_one(ctx, pins, pin_count, d, i, &bound->pin[i]) != 0) {
            return NULL;
        }
    }
    bound->device = d;
    return bound;
}

static int sm_load_part(KindContext *ctx, const void *raw, const void *bound, const cJSON *json,
                        Part *out) {
    const SerialmemDevice *d = raw;
    PartSerialmem *facts = kind_alloc(ctx, sizeof *facts);

    (void)bound;
    if (facts == NULL) {
        return -1;
    }
    *facts = d->facts;
    if (load_clock(ctx, json, &facts->max_clock_khz) != 0) {
        return -1;
    }
    out->serialmem = facts;
    return kind_part_voltage(ctx, json, d->vcc, &out->vcc);
}

const PartKindOps SERIALMEM_KIND = {
    PART_SERIALMEM, "serialmem", &smem_SERIALMEM_MODEL, smem_DEVICE_KEYS, smem_PART_KEYS,
    sm_load_device, sm_bind_pinout, sm_load_part
};
