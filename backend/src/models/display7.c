/*
 * 7seg: LED 7-segment displays (ported from backend/led7.c, behavior
 * unchanged). A display is a set of LEDs, each between one segment pin and
 * its digit's common pin(s). An LED is on while its anode reads 1 and its
 * cathode reads 0 (common anode: anode = common; common cathode: cathode =
 * common). A digit's common reads as the first of its common pins that is
 * not Z. Two common pins of one digit are joined inside the part (a closed
 * contact). The model drives nothing. Lights are tracked with lights.c, so
 * a multiplexed digit scanned during the app's last step counts as lit.
 */

#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "lights.h"

#define D7_MAX_LEDS 64
#define D7_MAX_COMMONS 2
#define D7_EXTRAS 9u   /* the colon/dot group (L1-L3); "extras" / digit 0 in JSON */

static const char *const DEVICE_KEYS[] = {
    "digits", "commonAnode", "colonAfter", "dotAfter", "color", "size", "chip", "leds",
    "segmentPins", "commonPins", "segmentSide", "commonSide", NULL
};

static const char *const SEGMENTS[] = {
    "A", "B", "C", "D", "E", "F", "G", "DP", "LDP", "HJ", "L1", "L2", "L3", NULL
};

typedef struct {
    const char *name;
    unsigned digit;      /* 0 = every digit */
    unsigned segments;   /* bit per SEGMENTS index */
} SegmentPin;

typedef struct {
    const char *name;
    unsigned digit;      /* 1.., or D7_EXTRAS */
} CommonPin;

typedef struct {
    unsigned digit;      /* 1.., or D7_EXTRAS */
    unsigned segment;    /* SEGMENTS index */
} LedDef;

typedef struct {
    PartDisplay display_template;   /* everything except the pin lists */
    PartDisplayLed *display_leds;
    LedDef leds[D7_MAX_LEDS];
    size_t led_count;
    SegmentPin *segment_pins;
    size_t segment_pin_count;
    CommonPin *common_pins;
    size_t common_pin_count;
    const char *const *segment_side;
    size_t segment_side_count;
    const char *const *common_side;
    size_t common_side_count;
} DisplayDevice;

typedef struct {
    size_t segment_pin;
    size_t common_pins[D7_MAX_COMMONS];
    size_t common_count;
} LedPins;

typedef struct {
    PartDisplay display;
    LedPins leds[D7_MAX_LEDS];
    size_t led_count;
    size_t contacts[D7_EXTRAS][2];   /* digit groups 1..9 with two commons */
    size_t contact_count;
    int common_anode;
} DisplayPins;

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

static int load_leds(KindContext *ctx, const cJSON *json, DisplayDevice *device) {
    static const char *const KEYS[] = {"digit", "segment", NULL};
    const cJSON *leds = kind_member(ctx, json, "leds", 1);
    const cJSON *item;
    size_t n = 0;

    if (leds == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(leds) || cJSON_GetArraySize(leds) == 0 ||
        cJSON_GetArraySize(leds) > D7_MAX_LEDS) {
        return kind_error(ctx, "\"leds\" must be an array of 1 to %d LEDs", D7_MAX_LEDS);
    }
    device->display_leds = kind_alloc(ctx, (size_t)cJSON_GetArraySize(leds) * sizeof(PartDisplayLed));
    if (device->display_leds == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(item, leds) {
        const char *segment;
        unsigned digit;

        kind_push(ctx, "leds[%zu]", n);
        if (kind_check_keys(ctx, item, KEYS, NULL) != 0 ||
            load_uint(ctx, item, "digit", device->display_template.digits, &digit) != 0 ||
            kind_string(ctx, item, "segment", &segment) != 0 ||
            segment_index(ctx, segment, &device->leds[n].segment) != 0) {
            return -1;
        }
        device->leds[n].digit = digit == 0 ? D7_EXTRAS : digit;
        device->display_leds[n].digit = digit;
        device->display_leds[n].segment = SEGMENTS[device->leds[n].segment];
        kind_pop(ctx);
        n++;
    }
    device->led_count = n;
    return 0;
}

static int load_segment_pins(KindContext *ctx, const cJSON *json, DisplayDevice *device) {
    static const char *const KEYS[] = {"digit", "segments", NULL};
    const cJSON *pins = kind_member(ctx, json, "segmentPins", 1);
    const cJSON *item;
    size_t n = 0;

    if (pins == NULL) {
        return -1;
    }
    if (!cJSON_IsObject(pins) || pins->child == NULL) {
        return kind_error(ctx, "\"segmentPins\" must be a non-empty object");
    }
    device->segment_pins = kind_alloc(ctx, (size_t)cJSON_GetArraySize(pins) * sizeof(SegmentPin));
    if (device->segment_pins == NULL || kind_check_keys(ctx, pins, NULL, NULL) != 0) {
        return -1;
    }
    cJSON_ArrayForEach(item, pins) {
        SegmentPin *pin = &device->segment_pins[n];
        const char *const *segments;
        size_t count;
        size_t i;

        kind_push(ctx, "segmentPins.%s", item->string);
        pin->name = kind_strdup(ctx, item->string);
        if (pin->name == NULL || kind_check_keys(ctx, item, KEYS, NULL) != 0 ||
            load_uint(ctx, item, "digit", device->display_template.digits, &pin->digit) != 0 ||
            kind_names(ctx, item, "segments", &segments, &count) != 0) {
            return -1;
        }
        if (count == 0) {
            return kind_error(ctx, "\"segments\" is empty");
        }
        for (i = 0; i < count; i++) {
            unsigned segment;

            if (segment_index(ctx, segments[i], &segment) != 0) {
                return -1;
            }
            pin->segments |= 1u << segment;
        }
        kind_pop(ctx);
        n++;
    }
    device->segment_pin_count = n;
    return 0;
}

static int load_common_pins(KindContext *ctx, const cJSON *json, DisplayDevice *device) {
    const cJSON *pins = kind_member(ctx, json, "commonPins", 1);
    const cJSON *item;
    size_t n = 0;

    if (pins == NULL) {
        return -1;
    }
    if (!cJSON_IsObject(pins) || pins->child == NULL) {
        return kind_error(ctx, "\"commonPins\" must be a non-empty object");
    }
    device->common_pins = kind_alloc(ctx, (size_t)cJSON_GetArraySize(pins) * sizeof(CommonPin));
    if (device->common_pins == NULL || kind_check_keys(ctx, pins, NULL, NULL) != 0) {
        return -1;
    }
    cJSON_ArrayForEach(item, pins) {
        CommonPin *pin = &device->common_pins[n];

        kind_push(ctx, "commonPins.%s", item->string);
        pin->name = kind_strdup(ctx, item->string);
        if (pin->name == NULL) {
            return -1;
        }
        if (cJSON_IsString(item) && strcmp(item->valuestring, "extras") == 0) {
            pin->digit = D7_EXTRAS;
        } else if (cJSON_IsNumber(item) && item->valuedouble >= 1 &&
                   item->valuedouble <= device->display_template.digits &&
                   item->valuedouble == (double)(unsigned)item->valuedouble) {
            pin->digit = (unsigned)item->valuedouble;
        } else {
            return kind_error(ctx, "a common pin is a digit from 1 to %u or \"extras\"",
                              device->display_template.digits);
        }
        kind_pop(ctx);
        n++;
    }
    device->common_pin_count = n;
    return 0;
}

static void *display_load_device(KindContext *ctx, const cJSON *json) {
    DisplayDevice *device = kind_alloc(ctx, sizeof(*device));
    PartDisplay *d;

    if (device == NULL) {
        return NULL;
    }
    d = &device->display_template;
    if (load_uint(ctx, json, "digits", 8, &d->digits) != 0 ||
        kind_bool(ctx, json, "commonAnode", &d->common_anode) != 0 ||
        load_uint(ctx, json, "colonAfter", 8, &d->colon_after) != 0 ||
        load_uint(ctx, json, "dotAfter", 8, &d->dot_after) != 0 ||
        kind_string(ctx, json, "color", &d->color) != 0 ||
        kind_string(ctx, json, "size", &d->size) != 0 ||
        kind_string(ctx, json, "chip", &d->chip) != 0 ||
        load_leds(ctx, json, device) != 0 || load_segment_pins(ctx, json, device) != 0 ||
        load_common_pins(ctx, json, device) != 0 ||
        kind_names(ctx, json, "segmentSide", &device->segment_side, &device->segment_side_count) != 0 ||
        kind_names(ctx, json, "commonSide", &device->common_side, &device->common_side_count) != 0) {
        return NULL;
    }
    d->leds = device->display_leds;
    d->led_count = device->led_count;
    return device;
}

/* Expands side names to pin numbers; every name must be a pin of the right kind. */
static unsigned *expand_side(KindContext *ctx, const char *key, const char *const *names,
                             size_t name_count, int commons, const DisplayDevice *device,
                             const PartPin *pins, size_t pin_count, size_t *count) {
    unsigned *numbers = kind_alloc(ctx, pin_count * sizeof(*numbers) + 1);
    size_t n = 0;
    size_t i;

    if (numbers == NULL) {
        return NULL;
    }
    for (i = 0; i < name_count; i++) {
        size_t indices[64];
        size_t found;
        size_t j;
        int known = 0;

        kind_push(ctx, "%s[%zu]", key, i);
        for (j = 0; j < (commons ? device->common_pin_count : device->segment_pin_count); j++) {
            const char *name = commons ? device->common_pins[j].name : device->segment_pins[j].name;

            if (strcmp(name, names[i]) == 0) {
                known = 1;
            }
        }
        if (!known) {
            kind_error(ctx, "\"%s\" is not a %s pin", names[i], commons ? "common" : "segment");
            return NULL;
        }
        found = kind_pins(ctx, pins, pin_count, names[i], indices, 64);
        if (found == 0) {
            return NULL;
        }
        for (j = 0; j < found && n < pin_count; j++) {
            numbers[n++] = pins[indices[j]].number;
        }
        kind_pop(ctx);
    }
    *count = n;
    return numbers;
}

static int pin_is(const PartPin *pin, const char *name) {
    return strcmp(pin->name, name) == 0;
}

static void *display_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                 size_t pin_count) {
    const DisplayDevice *device = raw;
    DisplayPins *bound = kind_alloc(ctx, sizeof(*bound));
    size_t i;
    size_t p;
    unsigned digit;

    if (bound == NULL) {
        return NULL;
    }
    /* Every named pin exists (shared names: every pin with the name). */
    for (i = 0; i < device->segment_pin_count; i++) {
        size_t indices[64];

        kind_push(ctx, "segmentPins.%s", device->segment_pins[i].name);
        if (kind_pins(ctx, pins, pin_count, device->segment_pins[i].name, indices, 64) == 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    for (i = 0; i < device->common_pin_count; i++) {
        size_t indices[64];

        kind_push(ctx, "commonPins.%s", device->common_pins[i].name);
        if (kind_pins(ctx, pins, pin_count, device->common_pins[i].name, indices, 64) == 0) {
            return NULL;
        }
        kind_pop(ctx);
    }

    /* Each LED: the first segment pin (pin order) that drives it, and the
       first two common pins of its digit (as led7.c). */
    for (i = 0; i < device->led_count; i++) {
        const LedDef *led = &device->leds[i];
        LedPins *out = &bound->leds[i];
        int have_segment = 0;

        for (p = 0; p < pin_count; p++) {
            size_t s;

            for (s = 0; s < device->segment_pin_count && !have_segment; s++) {
                const SegmentPin *sp = &device->segment_pins[s];

                if (pin_is(&pins[p], sp->name) && (sp->segments & (1u << led->segment)) &&
                    (sp->digit == 0 || sp->digit == led->digit)) {
                    out->segment_pin = p;
                    have_segment = 1;
                }
            }
            for (s = 0; s < device->common_pin_count; s++) {
                if (pin_is(&pins[p], device->common_pins[s].name) &&
                    device->common_pins[s].digit == led->digit &&
                    out->common_count < D7_MAX_COMMONS) {
                    out->common_pins[out->common_count++] = p;
                    break;
                }
            }
        }
        if (!have_segment || out->common_count == 0) {
            kind_error(ctx, "leds[%zu] (%s): no %s pin", i, SEGMENTS[led->segment],
                       have_segment ? "common" : "segment");
            return NULL;
        }
    }
    bound->led_count = device->led_count;

    /* Commons the datasheet lists twice are joined inside the part. */
    for (digit = 1; digit <= D7_EXTRAS; digit++) {
        size_t found[D7_MAX_COMMONS];
        size_t count = 0;

        for (p = 0; p < pin_count && count < D7_MAX_COMMONS; p++) {
            size_t s;

            for (s = 0; s < device->common_pin_count; s++) {
                if (pin_is(&pins[p], device->common_pins[s].name) &&
                    device->common_pins[s].digit == digit) {
                    found[count++] = p;
                    break;
                }
            }
        }
        if (count == 2) {
            bound->contacts[bound->contact_count][0] = found[0];
            bound->contacts[bound->contact_count][1] = found[1];
            bound->contact_count++;
        }
    }

    bound->display = device->display_template;
    bound->common_anode = device->display_template.common_anode;
    bound->display.segment_pins = expand_side(ctx, "segmentSide", device->segment_side,
                                              device->segment_side_count, 0, device, pins,
                                              pin_count, &bound->display.segment_pin_count);
    bound->display.common_pins = expand_side(ctx, "commonSide", device->common_side,
                                             device->common_side_count, 1, device, pins,
                                             pin_count, &bound->display.common_pin_count);
    if (bound->display.segment_pins == NULL || bound->display.common_pins == NULL) {
        return NULL;
    }
    return bound;
}

static int display_load_part(KindContext *ctx, const void *device, const void *bound,
                             const cJSON *json, Part *out) {
    (void)ctx;
    (void)device;
    (void)json;
    out->display = &((const DisplayPins *)bound)->display;
    return 0;
}

/* ---- Model --------------------------------------------------------------- */

typedef struct {
    const DisplayPins *pins;
    Light lights[D7_MAX_LEDS];
} DisplayState;

static size_t display_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *display_create(const void *config) {
    DisplayState *state = calloc(1, sizeof(*state));

    if (state != NULL) {
        state->pins = part_internal(config)->bound;
    }
    return state;
}

static void display_destroy(void *state) {
    free(state);
}

static LogicValue common_value(const Sim *sim, const SimInstance *instance, const LedPins *led) {
    size_t i;

    for (i = 0; i < led->common_count; i++) {
        LogicValue value = sim_read_pin(sim, instance, led->common_pins[i]);

        if (value != LOGIC_Z) {
            return value;
        }
    }
    return LOGIC_Z;
}

static void update_leds(Sim *sim, SimInstance *instance, DisplayState *state) {
    SimTime now = sim_now(sim);
    size_t i;

    for (i = 0; i < state->pins->led_count; i++) {
        const LedPins *led = &state->pins->leds[i];
        LogicValue common = common_value(sim, instance, led);
        LogicValue segment = sim_read_pin(sim, instance, led->segment_pin);
        LogicValue anode = state->pins->common_anode ? common : segment;
        LogicValue cathode = state->pins->common_anode ? segment : common;

        lights_set(state->lights, i, anode == LOGIC_1 && cathode == LOGIC_0, now);
    }
}

static void display_start(Sim *sim, SimInstance *instance, void *raw) {
    DisplayState *state = raw;
    size_t i;

    for (i = 0; i < state->pins->contact_count; i++) {
        sim_set_contact(sim, instance, state->pins->contacts[i][0], state->pins->contacts[i][1], 1);
    }
    update_leds(sim, instance, state);
}

static void display_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                                LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    update_leds(sim, instance, state);
}

static size_t display_lights(const Sim *sim, const SimInstance *instance, const void *raw,
                             SimTime since, unsigned char *out, size_t cap) {
    const DisplayState *state = raw;

    (void)sim;
    (void)instance;
    return lights_read(state->lights, state->pins->led_count, since, out, cap);
}

static const SimComponentType DISPLAY_MODEL = {
    "7seg", display_pin_count, display_create, display_destroy, display_start,
    display_pin_changed, NULL, display_lights, NULL, NULL, NULL
};

const PartKindOps DISPLAY7_KIND = {
    PART_7SEG, "7seg", &DISPLAY_MODEL, DEVICE_KEYS, NULL,
    display_load_device, display_bind_pinout, display_load_part
};
