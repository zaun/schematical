/*
 * charlcd: HD44780-compatible character LCD modules (plan-11-B). Each module
 * has its controller, character ROM, glass and backlight.
 *
 * Loader: the device keys of docs/parts-format.md § charlcd. The model is
 * filled in by plan-11-B Phase 3 onwards.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"

#define LCD_DATA_BITS 8
#define LCD_BUS_PINS (3 + LCD_DATA_BITS)

static const char *const DEVICE_KEYS[] = {
    "columns", "rows", "controller", "rom", "panel", "bus", "contrast", "backlight", "timing",
    "exec", "vcc", "inputMax", NULL
};
static const char *const PART_KEYS[] = {"vcc", "inputMax", NULL};
static const char *const BUS_KEYS[] = {"rs", "rw", "e", "data", NULL};
static const char *const BACKLIGHT_KEYS[] = {"anode", "cathode", "color", NULL};
static const char *const TIMING_KEYS[] = {
    "tc", "tpw", "tas", "tah", "tdsw", "th", "tddr", "tdhr", NULL
};
static const char *const EXEC_KEYS[] = {"instruction", "data", "clearHome", "powerOn", NULL};

/* Bus timing and execution times, in picoseconds. */
typedef struct {
    SimTime tc, tpw, tas, tah, tdsw, th, tddr, tdhr;
    SimTime instruction, data, clear_home, power_on;
} CharLcdTiming;

typedef struct {
    unsigned columns, rows;
    const char *controller, *rom, *panel, *backlight_color;
    const char *rs, *rw, *e;
    const char *const *data;
    const char *contrast;
    const char *anode, *cathode;      /* NULL without a backlight */
    CharLcdTiming t;
    const PartVoltage *vcc;
} CharLcdDevice;

/* Pin indices for one pinout. */
typedef struct {
    size_t rs, rw, e, db[LCD_DATA_BITS], contrast, anode, cathode;
    int has_backlight;
    size_t bus[LCD_BUS_PINS];
    PartCharLcd info;
    const CharLcdDevice *device;
} CharLcdPins;

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

static void *charlcd_load_device(KindContext *ctx, const cJSON *json) {
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
    if (rows < 1 || rows > 2) {
        kind_error(ctx, "\"rows\" must be 1 or 2");
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
    if (strcmp(device->panel, "yellow-green") != 0 && strcmp(device->panel, "blue-negative") != 0) {
        kind_error(ctx, "\"panel\" must be yellow-green or blue-negative");
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
    if (kind_check_keys(ctx, bus, BUS_KEYS, NULL) != 0 ||
        kind_string(ctx, bus, "rs", &device->rs) != 0 ||
        kind_string(ctx, bus, "rw", &device->rw) != 0 ||
        kind_string(ctx, bus, "e", &device->e) != 0) {
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
    if (backlight != NULL) {
        if (!cJSON_IsObject(backlight)) {
            kind_error(ctx, "\"backlight\" must be an object");
            return NULL;
        }
        if (kind_check_keys(ctx, backlight, BACKLIGHT_KEYS, NULL) != 0 ||
            kind_string(ctx, backlight, "anode", &device->anode) != 0 ||
            kind_string(ctx, backlight, "cathode", &device->cathode) != 0 ||
            kind_string(ctx, backlight, "color", &device->backlight_color) != 0) {
            return NULL;
        }
    }

    {
        SimTime *const timing[] = {&t->tc, &t->tpw, &t->tas, &t->tah, &t->tdsw, &t->th,
                                   &t->tddr, &t->tdhr};
        SimTime *const exec[] = {&t->instruction, &t->data, &t->clear_home, &t->power_on};
        size_t i;

        if (load_times(ctx, json, "timing", TIMING_KEYS, timing) != 0 ||
            more_than_zero(ctx, t->tc, "timing.tc") != 0 ||
            more_than_zero(ctx, t->tpw, "timing.tpw") != 0 ||
            load_times(ctx, json, "exec", EXEC_KEYS, exec) != 0) {
            return NULL;
        }
        for (i = 0; EXEC_KEYS[i] != NULL; i++) {
            char what[32];

            snprintf(what, sizeof what, "exec.%s", EXEC_KEYS[i]);
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

static void *charlcd_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                 size_t pin_count) {
    const CharLcdDevice *device = raw;
    CharLcdPins *bound = kind_alloc(ctx, sizeof(*bound));
    size_t i;

    if (bound == NULL ||
        kind_pin(ctx, pins, pin_count, device->rs, &bound->rs) != 0 ||
        kind_pin(ctx, pins, pin_count, device->rw, &bound->rw) != 0 ||
        kind_pin(ctx, pins, pin_count, device->e, &bound->e) != 0 ||
        kind_pin(ctx, pins, pin_count, device->contrast, &bound->contrast) != 0) {
        return NULL;
    }
    for (i = 0; i < LCD_DATA_BITS; i++) {
        if (kind_pin(ctx, pins, pin_count, device->data[i], &bound->db[i]) != 0) {
            return NULL;
        }
    }
    if (device->anode != NULL) {
        if (kind_pin(ctx, pins, pin_count, device->anode, &bound->anode) != 0 ||
            kind_pin(ctx, pins, pin_count, device->cathode, &bound->cathode) != 0) {
            return NULL;
        }
        bound->has_backlight = 1;
    }
    bound->bus[0] = bound->rs;
    bound->bus[1] = bound->rw;
    bound->bus[2] = bound->e;
    for (i = 0; i < LCD_DATA_BITS; i++) {
        bound->bus[3 + i] = bound->db[i];
    }
    bound->info.columns = device->columns;
    bound->info.rows = device->rows;
    bound->info.controller = device->controller;
    bound->info.rom = device->rom;
    bound->info.panel = device->panel;
    bound->info.backlight_color = device->backlight_color;
    bound->info.bus_pins = bound->bus;
    bound->info.bus_pin_count = LCD_BUS_PINS;
    bound->device = device;
    return bound;
}

static int charlcd_load_part(KindContext *ctx, const void *raw, const void *bound,
                             const cJSON *json, Part *out) {
    const CharLcdDevice *device = raw;

    out->charlcd = &((const CharLcdPins *)bound)->info;
    return kind_part_voltage(ctx, json, device->vcc, &out->vcc);
}

/* ---- Model ---------------------------------------------------------------- */

typedef struct {
    CharLcdPins pins;
} CharLcdState;

static size_t charlcd_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *charlcd_create(const void *config) {
    CharLcdState *state = calloc(1, sizeof(*state));

    if (state != NULL) {
        state->pins = *(const CharLcdPins *)part_internal(config)->bound;
    }
    return state;
}

static void charlcd_destroy(void *state) {
    free(state);
}

static void charlcd_start(Sim *sim, SimInstance *instance, void *state) {
    (void)sim;
    (void)instance;
    (void)state;
}

static void charlcd_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                                LogicValue old_value, LogicValue new_value) {
    (void)sim;
    (void)instance;
    (void)state;
    (void)pin;
    (void)old_value;
    (void)new_value;
}

static const SimComponentType CHARLCD_MODEL = {
    .name = "charlcd", .pin_count = charlcd_pin_count, .create = charlcd_create,
    .destroy = charlcd_destroy, .start = charlcd_start, .pin_changed = charlcd_pin_changed
};

const PartKindOps CHARLCD_KIND = {
    PART_CHARLCD, "charlcd", &CHARLCD_MODEL, DEVICE_KEYS, PART_KEYS,
    charlcd_load_device, charlcd_bind_pinout, charlcd_load_part
};
