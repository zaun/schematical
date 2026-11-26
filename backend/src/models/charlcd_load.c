/* models/charlcd_load.c: part of the model in models/charlcd.c (split by plan-68-I/J). */
#include "charlcd_int.h"
/* ---- Loader --------------------------------------------------------------- */

static int load_times(KindContext *ctx, const cJSON *json, const char *key,
                      const char *const *names, SimTime *const *fields) {
    const cJSON *object = kind_member(ctx, json, key, 1);
    char what[48];
    size_t i;

    if (object == NULL) {
        return -1;
    }
    if (!cJSON_IsObject(object)) {
        return kind_error(ctx, "\"%s\" must be an object of times in ns", key);
    }
    if (kind_check_keys(ctx, object, names, NULL) != 0) {
        return -1;
    }
    for (i = 0; names[i] != NULL; i++) {
        snprintf(what, sizeof what, "%s.%s", key, names[i]);
        if (kind_ns(ctx, cJSON_GetObjectItemCaseSensitive(object, names[i]), what, fields[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

static int more_than_zero(KindContext *ctx, SimTime value, const char *what) {
    return value > 0 ? 0 : kind_error(ctx, "\"%s\" must be more than 0", what);
}

/* "bus".e: one pin name, or two (E1, E2). */
static int load_enables(KindContext *ctx, const cJSON *bus, CharLcdDevice *device) {
    const cJSON *e = kind_member(ctx, bus, "e", 1);

    if (e == NULL) {
        return -1;
    }
    if (cJSON_IsString(e)) {
        device->enables = 1;
        return kind_string(ctx, bus, "e", &device->e[0]);
    }
    if (cJSON_IsArray(e) && cJSON_GetArraySize(e) == LCD_MAX_ENABLES) {
        const char *const *names;
        size_t count;

        if (kind_names(ctx, bus, "e", &names, &count) != 0) {
            return -1;
        }
        if (count == LCD_MAX_ENABLES) {
            device->enables = LCD_MAX_ENABLES;
            device->e[0] = names[0];
            device->e[1] = names[1];
            return 0;
        }
    }
    return kind_error(ctx, "\"e\" must be a pin name or 2 pin names");
}

/* "backlight": one LED object, or an array of 1-3. */
static int load_backlight(KindContext *ctx, const cJSON *backlight, CharLcdDevice *device) {
    int array = cJSON_IsArray(backlight);
    int count = array ? cJSON_GetArraySize(backlight) : cJSON_IsObject(backlight) ? 1 : 0;
    int i;

    if (count < 1 || count > LCD_MAX_LEDS) {
        return kind_error(ctx, "\"backlight\" must be an object or 1 to 3 objects");
    }
    for (i = 0; i < count; i++) {
        const cJSON *led = array ? cJSON_GetArrayItem(backlight, i) : backlight;
        int failed;

        if (!cJSON_IsObject(led)) {
            return kind_error(ctx, "\"backlight\" must be an object or 1 to 3 objects");
        }
        if (array) {
            kind_push(ctx, "backlight[%d]", i);
        }
        failed = kind_check_keys(ctx, led, clcd_BACKLIGHT_KEYS, NULL) != 0 ||
                 kind_string(ctx, led, "anode", &device->anode[i]) != 0 ||
                 kind_string(ctx, led, "cathode", &device->cathode[i]) != 0 ||
                 kind_string(ctx, led, "color", &device->colors[i]) != 0;
        if (array) {
            kind_pop(ctx);
        }
        if (failed) {
            return -1;
        }
    }
    device->leds = (size_t)count;
    device->backlight_color = device->colors[0];
    return 0;
}

void *clcd_charlcd_load_device(KindContext *ctx, const cJSON *json) {
    CharLcdDevice *device = kind_alloc(ctx, sizeof(*device));
    CharLcdTiming *t;
    const cJSON *bus;
    const cJSON *backlight;
    uint64_t columns, rows;
    size_t data_count;

    if (device == NULL) {
        return NULL;
    }
    t = &device->t;
    if (kind_uint(ctx, json, "columns", 1000000, &columns) != 0) {
        return NULL;
    }
    if (columns < 1 || columns > 40) {
        kind_error(ctx, "\"columns\" must be 1 to 40");
        return NULL;
    }
    if (kind_uint(ctx, json, "rows", 1000000, &rows) != 0) {
        return NULL;
    }
    if (rows != 1 && rows != 2 && rows != 4) {
        kind_error(ctx, "\"rows\" must be 1, 2 or 4");
        return NULL;
    }
    device->columns = (unsigned)columns;
    device->rows = (unsigned)rows;
    if (kind_string(ctx, json, "controller", &device->controller) != 0 ||
        kind_string(ctx, json, "rom", &device->rom) != 0) {
        return NULL;
    }
    if (strcmp(device->rom, "A00") != 0) {
        kind_error(ctx, "unknown rom \"%s\"", device->rom);
        return NULL;
    }
    if (kind_string(ctx, json, "panel", &device->panel) != 0) {
        return NULL;
    }
    if (strcmp(device->panel, "yellow-green") != 0 && strcmp(device->panel, "blue-negative") != 0 &&
        strcmp(device->panel, "gray") != 0 && strcmp(device->panel, "white") != 0) {
        kind_error(ctx, "\"panel\" must be yellow-green, blue-negative, gray or white");
        return NULL;
    }

    bus = kind_member(ctx, json, "bus", 1);
    if (bus == NULL) {
        return NULL;
    }
    if (!cJSON_IsObject(bus)) {
        kind_error(ctx, "\"bus\" must be an object of pin names");
        return NULL;
    }
    if (kind_check_keys(ctx, bus, clcd_BUS_KEYS, NULL) != 0 ||
        kind_string(ctx, bus, "rs", &device->rs) != 0 ||
        kind_string(ctx, bus, "rw", &device->rw) != 0 ||
        load_enables(ctx, bus, device) != 0) {
        return NULL;
    }
    if (device->enables == 2 && device->rows != 4) {
        kind_error(ctx, "two enables need \"rows\": 4");
        return NULL;
    }
    if (device->enables == 1 && device->rows == 4 && device->columns > 20) {
        kind_error(ctx, "4 rows on one controller need \"columns\" of 20 or less");
        return NULL;
    }
    {
        const cJSON *data = cJSON_GetObjectItemCaseSensitive(bus, "data");

        if (data == NULL || !cJSON_IsArray(data) || cJSON_GetArraySize(data) != LCD_DATA_BITS ||
            kind_names(ctx, bus, "data", &device->data, &data_count) != 0 ||
            data_count != LCD_DATA_BITS) {
            if (!kind_failed(ctx)) {
                kind_error(ctx, "\"data\" must be 8 pin names, DB0 first");
            }
            return NULL;
        }
    }
    if (kind_string(ctx, json, "contrast", &device->contrast) != 0) {
        return NULL;
    }

    backlight = cJSON_GetObjectItemCaseSensitive(json, "backlight");
    if (backlight != NULL && load_backlight(ctx, backlight, device) != 0) {
        return NULL;
    }

    {
        SimTime *const timing[] = {&t->tc, &t->tpw, &t->tas, &t->tah, &t->tdsw, &t->th,
                                   &t->tddr, &t->tdhr};
        SimTime *const exec[] = {&t->instruction, &t->data, &t->clear_home, &t->power_on};
        size_t i;

        if (load_times(ctx, json, "timing", clcd_TIMING_KEYS, timing) != 0 ||
            more_than_zero(ctx, t->tc, "timing.tc") != 0 ||
            more_than_zero(ctx, t->tpw, "timing.tpw") != 0 ||
            load_times(ctx, json, "exec", clcd_EXEC_KEYS, exec) != 0) {
            return NULL;
        }
        for (i = 0; clcd_EXEC_KEYS[i] != NULL; i++) {
            char what[32];

            snprintf(what, sizeof what, "exec.%s", clcd_EXEC_KEYS[i]);
            if (more_than_zero(ctx, *exec[i], what) != 0) {
                return NULL;
            }
        }
    }
    if (kind_voltage(ctx, json, NULL, &device->vcc) != 0) {
        return NULL;
    }
    return device;
}

void *clcd_charlcd_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                 size_t pin_count) {
    const CharLcdDevice *device = raw;
    CharLcdPins *bound = kind_alloc(ctx, sizeof(*bound));
    size_t i, n;

    if (bound == NULL ||
        kind_pin(ctx, pins, pin_count, device->rs, &bound->rs) != 0 ||
        kind_pin(ctx, pins, pin_count, device->rw, &bound->rw) != 0) {
        return NULL;
    }
    for (i = 0; i < device->enables; i++) {
        if (kind_pin(ctx, pins, pin_count, device->e[i], &bound->e[i]) != 0) {
            return NULL;
        }
    }
    if (kind_pin(ctx, pins, pin_count, device->contrast, &bound->contrast) != 0) {
        return NULL;
    }
    bound->enables = device->enables;
    for (i = 0; i < LCD_DATA_BITS; i++) {
        if (kind_pin(ctx, pins, pin_count, device->data[i], &bound->db[i]) != 0) {
            return NULL;
        }
    }
    for (i = 0; i < device->leds; i++) {
        if (kind_pin(ctx, pins, pin_count, device->anode[i], &bound->anode[i]) != 0 ||
            kind_pin(ctx, pins, pin_count, device->cathode[i], &bound->cathode[i]) != 0) {
            return NULL;
        }
    }
    bound->leds = device->leds;
    /* RS, R/W, E (or E1, E2), DB0-DB7. */
    n = 0;
    bound->bus[n++] = bound->rs;
    bound->bus[n++] = bound->rw;
    for (i = 0; i < device->enables; i++) {
        bound->bus[n++] = bound->e[i];
    }
    for (i = 0; i < LCD_DATA_BITS; i++) {
        bound->bus[n++] = bound->db[i];
    }
    bound->info.columns = device->columns;
    bound->info.rows = device->rows;
    bound->info.controller = device->controller;
    bound->info.rom = device->rom;
    bound->info.panel = device->panel;
    bound->info.backlight_color = device->backlight_color;
    bound->info.bus_pins = bound->bus;
    bound->info.bus_pin_count = n;
    bound->info.enables = device->enables;
    bound->info.backlight_count = device->leds;
    bound->info.backlight_colors = device->leds > 0 ? device->colors : NULL;
    bound->device = device;
    return bound;
}

int clcd_charlcd_load_part(KindContext *ctx, const void *raw, const void *bound,
                             const cJSON *json, Part *out) {
    const CharLcdDevice *device = raw;

    out->charlcd = &((const CharLcdPins *)bound)->info;
    return kind_part_voltage(ctx, json, device->vcc, &out->vcc);
}

int charlcd_backlight_pins(const Part *part, size_t index, size_t *anode, size_t *cathode) {
    const CharLcdPins *pins;

    if (part == NULL || part->kind != PART_CHARLCD) {
        return 0;
    }
    pins = part_internal(part)->bound;
    if (index >= pins->leds) {
        return 0;
    }
    *anode = pins->anode[index];
    *cathode = pins->cathode[index];
    return 1;
}

