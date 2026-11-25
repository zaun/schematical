/*
 * lcd: segment liquid-crystal glass (plan-56-A). A sibling of 7seg: a set of
 * cells, each between one segment pin and one backplane pin. A cell is lit
 * while both pins are driven (0 or 1) and their levels differ: the digital
 * form of "the segment waveform is out of phase with the backplane". A
 * multiplexed driver drives the backplane of the active slot and leaves the
 * others at Z, so only that backplane's cells can light in that slot. Static
 * glass is the one-backplane case. The glass drives nothing. Lights are
 * tracked with lights.c: a cell lit at any time during the app's last step
 * counts as lit, as on a multiplexed 7seg.
 *
 * Pins named by "cells" are plain inputs. The part's PartDisplay is the same
 * struct a 7seg has (leds = the cells in light order, segment_pins and
 * common_pins = the two sides, the common pins being the backplanes), so the
 * app draws it with the 7-segment symbol.
 */

#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "lights.h"

#define LCD_MAX_DIGITS 14
#define LCD_MAX_CELLS 160

static const char *const DEVICE_KEYS[] = {
    "digits", "colonAfter", "dotAfter", "color", "size", "drive", "backplanes", "cells",
    "segmentSide", "backplaneSide", NULL
};

static const char *const DRIVES[] = {"static", "1:2", "1:3", "1:4", NULL};

/* The 7seg segment names, plus the annunciators LB (low battery) and MINUS. */
static const char *const SEGMENTS[] = {
    "A", "B", "C", "D", "E", "F", "G", "DP", "L1", "L2", "L3", "LB", "MINUS", "AM", "PM", NULL
};

typedef struct {
    unsigned digit;          /* 1.., 0 = the colon / dot / annunciator group */
    unsigned segment;        /* SEGMENTS index */
    const char *pin, *bp;
} CellDef;

typedef struct {
    PartDisplay display_template;
    PartDisplayLed *display_leds;
    CellDef cells[LCD_MAX_CELLS];
    size_t cell_count;
    const char *const *backplanes;
    size_t backplane_count;
    const char *const *segment_side;
    size_t segment_side_count;
    const char *const *backplane_side;
    size_t backplane_side_count;
} LcdDevice;

typedef struct {
    size_t segment_pin;
    size_t bp_pins[2];       /* a backplane may be on two pins (COM at both ends of a glass) */
    size_t bp_count;
} CellPins;

typedef struct {
    PartDisplay display;
    CellPins cells[LCD_MAX_CELLS];
    size_t cell_count;
} LcdPins;

static int in_list(const char *const *list, size_t n, const char *name) {
    size_t i;

    for (i = 0; i < n; i++) {
        if (strcmp(list[i], name) == 0) {
            return 1;
        }
    }
    return 0;
}

static int segment_index(KindContext *ctx, const char *name, unsigned *out) {
    unsigned i;

    for (i = 0; SEGMENTS[i] != NULL; i++) {
        if (strcmp(SEGMENTS[i], name) == 0) {
            *out = i;
            return 0;
        }
    }
    return kind_error(ctx, "unknown segment \"%s\"", name);
}

static int load_uint(KindContext *ctx, const cJSON *json, const char *key, unsigned max,
                     unsigned *out) {
    uint64_t value;

    if (kind_uint(ctx, json, key, max, &value) != 0) {
        return -1;
    }
    *out = (unsigned)value;
    return 0;
}

static int load_cells(KindContext *ctx, const cJSON *json, LcdDevice *device) {
    static const char *const KEYS[] = {"digit", "segment", "pin", "bp", NULL};
    const cJSON *cells = kind_member(ctx, json, "cells", 1);
    const cJSON *item;
    size_t n = 0;
    size_t i;

    if (cells == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(cells) || cJSON_GetArraySize(cells) == 0 ||
        cJSON_GetArraySize(cells) > LCD_MAX_CELLS) {
        return kind_error(ctx, "\"cells\" must be an array of 1 to %d cells", LCD_MAX_CELLS);
    }
    device->display_leds = kind_alloc(ctx, (size_t)cJSON_GetArraySize(cells) * sizeof(PartDisplayLed));
    if (device->display_leds == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(item, cells) {
        CellDef *c = &device->cells[n];
        const char *segment;

        kind_push(ctx, "cells[%zu]", n);
        if (kind_check_keys(ctx, item, KEYS, NULL) != 0 ||
            load_uint(ctx, item, "digit", device->display_template.digits, &c->digit) != 0 ||
            kind_string(ctx, item, "segment", &segment) != 0 ||
            segment_index(ctx, segment, &c->segment) != 0 ||
            kind_string(ctx, item, "pin", &c->pin) != 0 || kind_string(ctx, item, "bp", &c->bp) != 0) {
            return -1;
        }
        if (!in_list(device->backplanes, device->backplane_count, c->bp)) {
            return kind_error(ctx, "\"bp\" \"%s\" is not one of \"backplanes\"", c->bp);
        }
        for (i = 0; i < n; i++) {
            if (device->cells[i].digit == c->digit && device->cells[i].segment == c->segment) {
                return kind_error(ctx, "digit %u segment %s is given twice", c->digit,
                                  SEGMENTS[c->segment]);
            }
        }
        device->display_leds[n].digit = c->digit;
        device->display_leds[n].segment = SEGMENTS[c->segment];
        kind_pop(ctx);
        n++;
    }
    device->cell_count = n;
    return 0;
}

static void *lcd_load_device(KindContext *ctx, const cJSON *json) {
    LcdDevice *device = kind_alloc(ctx, sizeof(*device));
    PartDisplay *d;
    const char *drive;
    size_t i;

    if (device == NULL) {
        return NULL;
    }
    d = &device->display_template;
    if (load_uint(ctx, json, "digits", LCD_MAX_DIGITS, &d->digits) != 0 ||
        load_uint(ctx, json, "colonAfter", LCD_MAX_DIGITS, &d->colon_after) != 0 ||
        load_uint(ctx, json, "dotAfter", LCD_MAX_DIGITS, &d->dot_after) != 0 ||
        kind_string(ctx, json, "color", &d->color) != 0 ||
        kind_string(ctx, json, "size", &d->size) != 0 ||
        kind_string(ctx, json, "drive", &drive) != 0 ||
        kind_names(ctx, json, "backplanes", &device->backplanes, &device->backplane_count) != 0 ||
        kind_names(ctx, json, "segmentSide", &device->segment_side, &device->segment_side_count) != 0 ||
        kind_names(ctx, json, "backplaneSide", &device->backplane_side,
                   &device->backplane_side_count) != 0) {
        return NULL;
    }
    if (d->digits < 1) {
        kind_error(ctx, "\"digits\" must be 1 to %d", LCD_MAX_DIGITS);
        return NULL;
    }
    for (i = 0; DRIVES[i] != NULL && strcmp(DRIVES[i], drive) != 0; i++) {
    }
    if (DRIVES[i] == NULL) {
        kind_error(ctx, "\"drive\" must be \"static\", \"1:2\", \"1:3\" or \"1:4\"");
        return NULL;
    }
    if (device->backplane_count < 1 || device->backplane_count > 4) {
        kind_error(ctx, "\"backplanes\" must name 1 to 4 pins");
        return NULL;
    }
    d->type = DRIVES[i];
    if (load_cells(ctx, json, device) != 0) {
        return NULL;
    }
    d->common_anode = 0;
    d->chip = "";
    d->leds = device->display_leds;
    d->led_count = device->cell_count;
    return device;
}

static size_t *expand_side(KindContext *ctx, const char *key, const char *const *names,
                           size_t name_count, const char *const *allowed, size_t allowed_count,
                           const PartPin *pins, size_t pin_count, size_t *count) {
    size_t *out = kind_alloc(ctx, pin_count * sizeof(*out) + 1);
    size_t n = 0;
    size_t i;

    if (out == NULL) {
        return NULL;
    }
    for (i = 0; i < name_count; i++) {
        size_t indices[64];
        size_t found;
        size_t j;

        kind_push(ctx, "%s[%zu]", key, i);
        if (!in_list(allowed, allowed_count, names[i])) {
            kind_error(ctx, "\"%s\" is not a pin the cells name", names[i]);
            return NULL;
        }
        found = kind_pins(ctx, pins, pin_count, names[i], indices, 64);
        if (found == 0) {
            return NULL;
        }
        for (j = 0; j < found && n < pin_count; j++) {
            out[n++] = indices[j];
        }
        kind_pop(ctx);
    }
    *count = n;
    return out;
}

/* Up to `max` pins with this name, in pin order; returns how many. */
static size_t pins_named(const PartPin *pins, size_t pin_count, const char *name, size_t *out,
                         size_t max) {
    size_t n = 0;
    size_t i;

    for (i = 0; i < pin_count && n < max; i++) {
        if (strcmp(pins[i].name, name) == 0) {
            out[n++] = i;
        }
    }
    return n;
}

static void *lcd_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                             size_t pin_count) {
    const LcdDevice *device = raw;
    LcdPins *bound = kind_alloc(ctx, sizeof(*bound));
    const char **segment_names;
    size_t segment_name_count = 0;
    size_t i;

    if (bound == NULL) {
        return NULL;
    }
    segment_names = kind_alloc(ctx, (device->cell_count + 1) * sizeof(*segment_names));
    if (segment_names == NULL) {
        return NULL;
    }
    for (i = 0; i < device->cell_count; i++) {
        const CellDef *c = &device->cells[i];

        size_t seg[1];
        size_t k;

        kind_push(ctx, "cells[%zu]", i);
        bound->cells[i].bp_count = pins_named(pins, pin_count, c->bp, bound->cells[i].bp_pins, 2);
        if (pins_named(pins, pin_count, c->pin, seg, 1) == 0 || bound->cells[i].bp_count == 0) {
            kind_error(ctx, "pin \"%s\" or \"%s\" is not in the pinout", c->pin, c->bp);
            return NULL;
        }
        bound->cells[i].segment_pin = seg[0];
        if (pins[seg[0]].role != PIN_INPUT) {
            kind_error(ctx, "\"%s\" must have role input", c->pin);
            return NULL;
        }
        for (k = 0; k < bound->cells[i].bp_count; k++) {
            if (pins[bound->cells[i].bp_pins[k]].role != PIN_INPUT) {
                kind_error(ctx, "\"%s\" must have role input", c->bp);
                return NULL;
            }
        }
        if (!in_list(segment_names, segment_name_count, c->pin)) {
            segment_names[segment_name_count++] = c->pin;
        }
        kind_pop(ctx);
    }
    bound->cell_count = device->cell_count;
    bound->display = device->display_template;
    bound->display.segment_pins = expand_side(ctx, "segmentSide", device->segment_side,
                                              device->segment_side_count, segment_names,
                                              segment_name_count, pins, pin_count,
                                              &bound->display.segment_pin_count);
    bound->display.common_pins = expand_side(ctx, "backplaneSide", device->backplane_side,
                                             device->backplane_side_count, device->backplanes,
                                             device->backplane_count, pins, pin_count,
                                             &bound->display.common_pin_count);
    if (bound->display.segment_pins == NULL || bound->display.common_pins == NULL) {
        return NULL;
    }
    return bound;
}

static int lcd_load_part(KindContext *ctx, const void *device, const void *bound,
                         const cJSON *json, Part *out) {
    (void)ctx;
    (void)device;
    (void)json;
    out->display = &((const LcdPins *)bound)->display;
    return 0;
}

/* ---- Model --------------------------------------------------------------- */

typedef struct {
    const LcdPins *pins;
    Light lights[LCD_MAX_CELLS];
} LcdState;

static size_t lcd_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *lcd_create(const void *config) {
    LcdState *state = calloc(1, sizeof(*state));

    if (state != NULL) {
        state->pins = part_internal(config)->bound;
    }
    return state;
}

static void lcd_destroy(void *state) {
    free(state);
}

static void update_cells(Sim *sim, SimInstance *instance, LcdState *state) {
    SimTime now = sim_now(sim);
    size_t i;

    for (i = 0; i < state->pins->cell_count; i++) {
        const CellPins *c = &state->pins->cells[i];
        LogicValue segment = sim_read_pin(sim, instance, c->segment_pin);
        LogicValue bp = LOGIC_Z;
        size_t k;

        for (k = 0; k < c->bp_count && bp == LOGIC_Z; k++) {
            bp = sim_read_pin(sim, instance, c->bp_pins[k]);
        }
        int driven = (segment == LOGIC_0 || segment == LOGIC_1) && (bp == LOGIC_0 || bp == LOGIC_1);

        lights_set(state->lights, i, driven && segment != bp, now);
    }
}

static void lcd_start(Sim *sim, SimInstance *instance, void *raw) {
    update_cells(sim, instance, raw);
}

static void lcd_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                            LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    update_cells(sim, instance, state);
}

static size_t lcd_lights(const Sim *sim, const SimInstance *instance, const void *raw,
                         SimTime since, unsigned char *out, size_t cap) {
    const LcdState *state = raw;

    (void)sim;
    (void)instance;
    return lights_read(state->lights, state->pins->cell_count, since, out, cap);
}

static size_t lcd_light_time(const Sim *sim, const SimInstance *instance, const void *raw,
                             uint64_t *out, size_t cap) {
    const LcdState *state = raw;

    (void)instance;
    return lights_time(state->lights, state->pins->cell_count, sim_now(sim), out, cap);
}

static const SimComponentType LCD_MODEL = {
    "lcd", lcd_pin_count, lcd_create, lcd_destroy, lcd_start,
    lcd_pin_changed, NULL, lcd_lights, NULL, NULL, NULL, lcd_light_time
};

const PartKindOps LCD_KIND = {
    PART_LCD, "lcd", &LCD_MODEL, DEVICE_KEYS, NULL,
    lcd_load_device, lcd_bind_pinout, lcd_load_part
};
