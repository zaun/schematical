/*
 * powerswitch (plan-55-A): power switches, one to four switches (elements)
 * per package: load switches, current-limited and USB power switches, and
 * integrated ideal diodes. Each element is a gate for a supply in a logic
 * simulator: while its input pin reads 1 and its enable pin (when it has
 * one) is active, its output pin is driven strong 1; no analog behaviour
 * (no ramp, on-resistance drop, current limit, thermal shutdown, quick
 * output discharge or reverse-current measurement). What a switch passes on
 * (the input's level, ORed with another switch's on the same net) is for the
 * voltage check (voltage.c); the facts are for the app.
 *
 * A change at the input or the enable reaches the output 1 ns later
 * (POWERSWITCH_DELAY_PS, from a timer that re-reads the pins), so a pulse
 * shorter than that never moves it. A floating logic enable is inactive for
 * both polarities. An enable that is a comparator against the input
 * ("enableRef": "input", the LM66100's CE) never turns the element off while
 * the input reads 1: a logic simulator sees 1 as "not above" it. The optional
 * status pin (open drain) is pulled low while the element is disabled
 * ("flagWhen": "disabled") or never ("fault": the part reports an overcurrent
 * or over-temperature fault, which the model never has).
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "model_pins.h"

#define POWERSWITCH_DELAY_PS 1000u

static const char *const DEVICE_KEYS[] = {"type", "switches", "analog", NULL};
static const char *const PART_KEYS[] = {"vinMin", "vinMax", "iout", "ilimit", "ron", "iq", NULL};

typedef struct {
    const char *in;
    const char *out;
    const char *enable;        /* NULL = none */
    int enable_low;
    int enable_ref_input;      /* the enable is a comparator against the input */
    const char *flag;          /* NULL = none */
    int flag_disabled;         /* "disabled": low while off; else a fault flag, never low */
} PowerSwitchElement;

typedef struct {
    const char *type;          /* "load", "limited", "ideal-diode" */
    size_t count;
    PowerSwitchElement *elements;
    const char *const *analog;
    size_t analog_count;
} PowerSwitchDevice;

/* A pinout's pins as indices into Part.pins: per element the first pin of
   each name (SIZE_MAX when the element has none), every pin of the input,
   output and flag names, and per pin its analog / open-drain flags. */
typedef struct {
    size_t count;
    const PowerSwitchDevice *device;
    size_t *in_first, *out_first, *enable, *flag;
    size_t *in_from, *in_pins;     /* element k's input pins: in_pins[in_from[k] .. in_from[k + 1]) */
    size_t *out_from, *out_pins;
    size_t *flag_from, *flag_pins;
    unsigned char *analog;         /* per pin */
    unsigned char *open_drain;     /* per pin: a flag pin */
} PowerSwitchPins;

static const char *const TYPES[] = {"load", "limited", "ideal-diode", NULL};

static int level_key(KindContext *ctx, const cJSON *item, const char *key, const char *a, const char *b,
                     int *second) {
    const char *value;

    if (kind_string(ctx, item, key, &value) != 0) {
        return -1;
    }
    if (strcmp(value, a) != 0 && strcmp(value, b) != 0) {
        return kind_error(ctx, "\"%s\" must be \"%s\" or \"%s\"", key, a, b);
    }
    *second = strcmp(value, b) == 0;
    return 0;
}

static void *powerswitch_load_device(KindContext *ctx, const cJSON *json) {
    static const char *const ELEMENT_KEYS[] = {"in", "out", "enable", "enableActive", "enableRef", "flag",
                                               "flagWhen", NULL};
    PowerSwitchDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *elements;
    const cJSON *item;
    size_t count = 0;
    size_t t;
    int known = 0;

    if (device == NULL) {
        return NULL;
    }
    if (kind_string(ctx, json, "type", &device->type) != 0) {
        return NULL;
    }
    for (t = 0; TYPES[t] != NULL; t++) {
        known |= strcmp(device->type, TYPES[t]) == 0;
    }
    if (!known) {
        kind_error(ctx, "unknown power switch type \"%s\" (load, limited, ideal-diode)", device->type);
        return NULL;
    }
    elements = kind_member(ctx, json, "switches", 1);
    if (elements == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(elements) || cJSON_GetArraySize(elements) < 1 || cJSON_GetArraySize(elements) > 4) {
        kind_error(ctx, "\"switches\" must have 1 to 4 entries");
        return NULL;
    }
    device->elements = kind_alloc(ctx, (size_t)cJSON_GetArraySize(elements) * sizeof(*device->elements));
    if (device->elements == NULL) {
        return NULL;
    }
    cJSON_ArrayForEach(item, elements) {
        PowerSwitchElement *e = &device->elements[count];

        kind_push(ctx, "switches[%zu]", count);
        if (!cJSON_IsObject(item)) {
            kind_error(ctx, "a switch is {\"in\": …, \"out\": …}");
            return NULL;
        }
        if (kind_check_keys(ctx, item, ELEMENT_KEYS, NULL) != 0 || kind_string(ctx, item, "in", &e->in) != 0 ||
            kind_string(ctx, item, "out", &e->out) != 0) {
            return NULL;
        }
        if (cJSON_GetObjectItemCaseSensitive(item, "enable") != NULL &&
            kind_string(ctx, item, "enable", &e->enable) != 0) {
            return NULL;
        }
        if (cJSON_GetObjectItemCaseSensitive(item, "enableActive") != NULL) {
            if (e->enable == NULL) {
                kind_error(ctx, "\"enableActive\" needs \"enable\"");
                return NULL;
            }
            if (level_key(ctx, item, "enableActive", "high", "low", &e->enable_low) != 0) {
                return NULL;
            }
        }
        if (cJSON_GetObjectItemCaseSensitive(item, "enableRef") != NULL) {
            if (e->enable == NULL) {
                kind_error(ctx, "\"enableRef\" needs \"enable\"");
                return NULL;
            }
            if (level_key(ctx, item, "enableRef", "logic", "input", &e->enable_ref_input) != 0) {
                return NULL;
            }
            if (e->enable_ref_input && !e->enable_low) {
                kind_error(ctx, "\"enableRef\": \"input\" needs \"enableActive\": \"low\"");
                return NULL;
            }
        }
        if (cJSON_GetObjectItemCaseSensitive(item, "flag") != NULL &&
            kind_string(ctx, item, "flag", &e->flag) != 0) {
            return NULL;
        }
        if (cJSON_GetObjectItemCaseSensitive(item, "flagWhen") != NULL) {
            if (e->flag == NULL) {
                kind_error(ctx, "\"flagWhen\" needs \"flag\"");
                return NULL;
            }
            if (level_key(ctx, item, "flagWhen", "fault", "disabled", &e->flag_disabled) != 0) {
                return NULL;
            }
        }
        if (strcmp(e->in, e->out) == 0) {
            kind_error(ctx, "a switch's input and output must have different names");
            return NULL;
        }
        kind_pop(ctx);
        count++;
    }
    device->count = count;
    if (cJSON_GetObjectItemCaseSensitive(json, "analog") != NULL &&
        kind_names(ctx, json, "analog", &device->analog, &device->analog_count) != 0) {
        return NULL;
    }
    return device;
}

/* Every pin with this name must have the role; the element key is for the message. */
static int check_role(KindContext *ctx, const PartPin *pins, const size_t *list, size_t n, const char *key,
                      PinRole role) {
    size_t i;

    for (i = 0; i < n; i++) {
        if (pins[list[i]].role != role) {
            return kind_error(ctx, "power switch pin \"%s\" (%s) must be %s", pins[list[i]].name, key,
                              role == PIN_POWER ? "power" : role == PIN_OUTPUT ? "output" : "input");
        }
    }
    return 0;
}

static void *powerswitch_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins, size_t pin_count) {
    const PowerSwitchDevice *device = raw;
    PowerSwitchPins *bound = kind_alloc(ctx, sizeof(*bound));
    size_t *scratch = kind_alloc(ctx, (pin_count + 1) * sizeof(*scratch));
    unsigned char *used = kind_alloc(ctx, pin_count + 1);
    size_t cap = device->count * pin_count + 1;
    size_t ni = 0, no = 0, nf = 0;
    size_t k;
    size_t i;

    if (bound == NULL || scratch == NULL || used == NULL) {
        return NULL;
    }
    bound->count = device->count;
    bound->device = device;
    bound->in_first = kind_alloc(ctx, (device->count + 1) * sizeof(size_t));
    bound->out_first = kind_alloc(ctx, (device->count + 1) * sizeof(size_t));
    bound->enable = kind_alloc(ctx, (device->count + 1) * sizeof(size_t));
    bound->flag = kind_alloc(ctx, (device->count + 1) * sizeof(size_t));
    bound->in_from = kind_alloc(ctx, (device->count + 1) * sizeof(size_t));
    bound->out_from = kind_alloc(ctx, (device->count + 1) * sizeof(size_t));
    bound->flag_from = kind_alloc(ctx, (device->count + 1) * sizeof(size_t));
    bound->in_pins = kind_alloc(ctx, cap * sizeof(size_t));
    bound->out_pins = kind_alloc(ctx, cap * sizeof(size_t));
    bound->flag_pins = kind_alloc(ctx, cap * sizeof(size_t));
    bound->analog = kind_alloc(ctx, pin_count + 1);
    bound->open_drain = kind_alloc(ctx, pin_count + 1);
    if (bound->in_first == NULL || bound->out_first == NULL || bound->enable == NULL || bound->flag == NULL ||
        bound->in_from == NULL || bound->out_from == NULL || bound->flag_from == NULL ||
        bound->in_pins == NULL || bound->out_pins == NULL || bound->flag_pins == NULL ||
        bound->analog == NULL || bound->open_drain == NULL) {
        return NULL;
    }
    for (i = 0; i < device->analog_count; i++) {
        size_t n = kind_pins(ctx, pins, pin_count, device->analog[i], scratch, pin_count);
        size_t x;

        if (n == 0) {
            return NULL;
        }
        for (x = 0; x < n; x++) {
            bound->analog[scratch[x]] = 1;
            used[scratch[x]] = 1;
        }
    }
    for (k = 0; k < device->count; k++) {
        const PowerSwitchElement *e = &device->elements[k];
        size_t n;
        size_t x;

        bound->in_from[k] = ni;
        bound->out_from[k] = no;
        bound->flag_from[k] = nf;
        bound->enable[k] = bound->flag[k] = SIZE_MAX;
        n = kind_pins(ctx, pins, pin_count, e->in, scratch, pin_count);
        if (n == 0 || check_role(ctx, pins, scratch, n, "in", PIN_POWER) != 0) {
            return NULL;
        }
        bound->in_first[k] = scratch[0];
        for (x = 0; x < n; x++) {
            bound->in_pins[ni++] = scratch[x];
            used[scratch[x]] = 1;
        }
        n = kind_pins(ctx, pins, pin_count, e->out, scratch, pin_count);
        if (n == 0 || check_role(ctx, pins, scratch, n, "out", PIN_OUTPUT) != 0) {
            return NULL;
        }
        bound->out_first[k] = scratch[0];
        for (x = 0; x < n; x++) {
            bound->out_pins[no++] = scratch[x];
            used[scratch[x]] = 1;
        }
        if (e->enable != NULL) {
            n = kind_pins(ctx, pins, pin_count, e->enable, scratch, pin_count);
            if (n == 0 || check_role(ctx, pins, scratch, n, "enable", PIN_INPUT) != 0) {
                return NULL;
            }
            bound->enable[k] = scratch[0];
            for (x = 0; x < n; x++) {
                used[scratch[x]] = 1;
            }
        }
        if (e->flag != NULL) {
            n = kind_pins(ctx, pins, pin_count, e->flag, scratch, pin_count);
            if (n == 0 || check_role(ctx, pins, scratch, n, "flag", PIN_OUTPUT) != 0) {
                return NULL;
            }
            bound->flag[k] = scratch[0];
            for (x = 0; x < n; x++) {
                bound->flag_pins[nf++] = scratch[x];
                bound->open_drain[scratch[x]] = 1;
                used[scratch[x]] = 1;
            }
        }
    }
    bound->in_from[device->count] = ni;
    bound->out_from[device->count] = no;
    bound->flag_from[device->count] = nf;
    for (i = 0; i < pin_count; i++) {
        if (pins[i].role != PIN_NC && pins[i].role != PIN_ABSENT && pins[i].role != PIN_GROUND && !used[i]) {
            kind_error(ctx, "pin \"%s\" is in no switch and not analog", pins[i].name);
            return NULL;
        }
    }
    return bound;
}

/* A number in (0, max] times scale as a count; 0 when the key is absent. */
static int optional_scaled(KindContext *ctx, const cJSON *json, const char *key, double max, double scale,
                           uint64_t *out) {
    const cJSON *item = kind_member(ctx, json, key, 0);

    *out = 0;
    if (item == NULL) {
        return 0;
    }
    if (!cJSON_IsNumber(item) || !(item->valuedouble > 0) || item->valuedouble > max) {
        return kind_error(ctx, "\"%s\" must be above 0 and at most %g", key, max);
    }
    *out = (uint64_t)llround(item->valuedouble * scale);
    return 0;
}

static int powerswitch_load_part(KindContext *ctx, const void *raw, const void *bound, const cJSON *json,
                                 Part *out) {
    const PowerSwitchDevice *device = raw;
    PartPowerSwitch *info = kind_alloc(ctx, sizeof(*info));
    uint64_t vin_min = 0, vin_max = 0, iout = 0, ilimit = 0, ron = 0, iq = 0;

    (void)bound;
    if (info == NULL) {
        return -1;
    }
    if (optional_scaled(ctx, json, "vinMin", 100, 1000, &vin_min) != 0 ||
        optional_scaled(ctx, json, "vinMax", 100, 1000, &vin_max) != 0 ||
        optional_scaled(ctx, json, "iout", 100, 1000, &iout) != 0 ||
        optional_scaled(ctx, json, "ilimit", 100, 1000, &ilimit) != 0 ||
        optional_scaled(ctx, json, "ron", 10, 1000, &ron) != 0 ||
        optional_scaled(ctx, json, "iq", 1, 1e6, &iq) != 0) {
        return -1;
    }
    if (vin_min != 0 && vin_max != 0 && vin_max < vin_min) {
        return kind_error(ctx, "\"vinMax\" must be at least \"vinMin\"");
    }
    if (ilimit != 0 && strcmp(device->type, "limited") != 0) {
        return kind_error(ctx, "\"ilimit\" is only for a current-limited switch");
    }
    info->type = device->type;
    info->switch_count = device->count;
    info->vin_min_mv = (uint32_t)vin_min;
    info->vin_max_mv = (uint32_t)vin_max;
    info->iout_ma = (uint32_t)iout;
    info->ilimit_ma = (uint32_t)ilimit;
    info->ron_mohm = (uint32_t)ron;
    info->iq_ua = (uint32_t)iq;
    out->powerswitch = info;
    return 0;
}

/* ---- Facts for the voltage check ---------------------------------------- */

int powerswitch_element(const Part *part, size_t k, size_t *in, size_t *out, size_t *enable, int *enable_low,
                        int *enable_ref_input, size_t *flag, int *flag_disabled) {
    const PowerSwitchPins *bound;
    const PowerSwitchElement *e;

    if (part == NULL || part->kind != PART_POWERSWITCH) {
        return 0;
    }
    bound = part_internal(part)->bound;
    if (k >= bound->count) {
        return 0;
    }
    e = &bound->device->elements[k];
    if (in != NULL) {
        *in = bound->in_first[k];
    }
    if (out != NULL) {
        *out = bound->out_first[k];
    }
    if (enable != NULL) {
        *enable = bound->enable[k];
    }
    if (enable_low != NULL) {
        *enable_low = e->enable_low;
    }
    if (enable_ref_input != NULL) {
        *enable_ref_input = e->enable_ref_input;
    }
    if (flag != NULL) {
        *flag = bound->flag[k];
    }
    if (flag_disabled != NULL) {
        *flag_disabled = e->flag_disabled;
    }
    return 1;
}

static int pin_listed(const size_t *list, size_t from, size_t to, size_t pin) {
    size_t i;

    for (i = from; i < to; i++) {
        if (list[i] == pin) {
            return 1;
        }
    }
    return 0;
}

int powerswitch_pin_in(const Part *part, size_t k, size_t pin) {
    const PowerSwitchPins *bound;

    if (part == NULL || part->kind != PART_POWERSWITCH) {
        return 0;
    }
    bound = part_internal(part)->bound;
    return k < bound->count && pin_listed(bound->in_pins, bound->in_from[k], bound->in_from[k + 1], pin);
}

int powerswitch_pin_out(const Part *part, size_t k, size_t pin) {
    const PowerSwitchPins *bound;

    if (part == NULL || part->kind != PART_POWERSWITCH) {
        return 0;
    }
    bound = part_internal(part)->bound;
    return k < bound->count && pin_listed(bound->out_pins, bound->out_from[k], bound->out_from[k + 1], pin);
}

uint32_t powerswitch_input_max_mv(const Part *part) {
    if (part == NULL || part->kind != PART_POWERSWITCH || part->powerswitch == NULL) {
        return 0;
    }
    return part->powerswitch->vin_max_mv;
}

static int powerswitch_pin_open_drain(const Part *part, size_t pin) {
    const PowerSwitchPins *bound = part_internal(part)->bound;

    return bound->open_drain[pin];
}

static int powerswitch_pin_analog(const Part *part, size_t pin) {
    const PowerSwitchPins *bound = part_internal(part)->bound;

    return bound->analog[pin];
}

/* ---- Model --------------------------------------------------------------- */

/* What an element is doing (the table of plan-55-A section 3.3). */
enum { PSW_NONE, PSW_ON, PSW_OFF, PSW_X, PSW_XX };

typedef struct {
    const PowerSwitchPins *pins;
    unsigned char state[];
} PowerSwitchState;

static size_t powerswitch_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *powerswitch_create(const void *config) {
    const PowerSwitchPins *pins = part_internal(config)->bound;
    PowerSwitchState *state = calloc(1, sizeof(*state) + pins->count + 1);

    if (state != NULL) {
        state->pins = pins;
    }
    return state;
}

static void powerswitch_destroy(void *state) {
    free(state);
}

static void powerswitch_start(Sim *sim, SimInstance *instance, void *raw) {
    PowerSwitchState *state = raw;
    size_t k;

    for (k = 0; k < state->pins->count; k++) {
        sim_schedule_timer(sim, instance, (uint32_t)k, POWERSWITCH_DELAY_PS);
    }
}

static void powerswitch_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin, LogicValue old_value,
                                    LogicValue new_value) {
    PowerSwitchState *state = raw;
    const PowerSwitchPins *pins = state->pins;
    size_t k;

    (void)old_value;
    (void)new_value;
    for (k = 0; k < pins->count; k++) {
        if (pin_listed(pins->in_pins, pins->in_from[k], pins->in_from[k + 1], pin) ||
            (pins->enable[k] != SIZE_MAX && pins->enable[k] == pin)) {
            sim_schedule_timer(sim, instance, (uint32_t)k, POWERSWITCH_DELAY_PS);
        }
    }
}

/* The reading of an element's input: 1 if any pin of the name reads 1 (and
   none X), X if any reads X, 0 if one reads 0, else Z. */
static LogicValue read_input(const Sim *sim, const SimInstance *instance, const PowerSwitchPins *pins, size_t k) {
    int one = 0, unknown = 0, zero = 0;
    size_t i;

    for (i = pins->in_from[k]; i < pins->in_from[k + 1]; i++) {
        LogicValue v = sim_read_pin(sim, instance, pins->in_pins[i]);

        one |= v == LOGIC_1;
        zero |= v == LOGIC_0;
        unknown |= v == LOGIC_X;
    }
    return unknown ? LOGIC_X : one ? LOGIC_1 : zero ? LOGIC_0 : LOGIC_Z;
}

/* What element j makes of a pin it drives: an output is 1 while on, X while
   unknown; a flag is 0 while off (a "disabled" flag), X while the enable is
   unknown. */
static LogicValue out_value(unsigned char state) {
    return state == PSW_ON ? LOGIC_1 : state == PSW_X || state == PSW_XX ? LOGIC_X : LOGIC_Z;
}

static LogicValue flag_value(const PowerSwitchPins *pins, size_t j, unsigned char state) {
    if (!pins->device->elements[j].flag_disabled) {
        return LOGIC_Z;
    }
    return state == PSW_OFF ? LOGIC_0 : state == PSW_XX ? LOGIC_X : LOGIC_Z;
}

/* Several elements may name one output pin (the LM66100's twin VOUT) or one
   flag pin: the pin takes the strongest of them, 1 over X over Z for an
   output and 0 over X over Z for a flag. */
static LogicValue combined(const PowerSwitchState *state, const size_t *list, const size_t *from, size_t pin,
                           int flag) {
    const PowerSwitchPins *pins = state->pins;
    LogicValue best = LOGIC_Z;
    size_t j;

    for (j = 0; j < pins->count; j++) {
        LogicValue v;

        if (!pin_listed(list, from[j], from[j + 1], pin)) {
            continue;
        }
        v = flag ? flag_value(pins, j, state->state[j]) : out_value(state->state[j]);
        if (v == LOGIC_X && best == LOGIC_Z) {
            best = LOGIC_X;
        } else if (v == (flag ? LOGIC_0 : LOGIC_1)) {
            best = v;
        }
    }
    return best;
}

static void powerswitch_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t timer_id) {
    PowerSwitchState *state = raw;
    const PowerSwitchPins *pins = state->pins;
    const PowerSwitchElement *e;
    size_t k = timer_id;
    LogicValue vin;
    unsigned char want;
    size_t i;

    if (k >= pins->count) {
        return;
    }
    e = &pins->device->elements[k];
    vin = read_input(sim, instance, pins, k);
    if (vin == LOGIC_X) {
        want = PSW_X;
    } else if (pins->enable[k] == SIZE_MAX) {
        want = vin == LOGIC_1 ? PSW_ON : PSW_NONE;
    } else {
        LogicValue en = sim_read_pin(sim, instance, pins->enable[k]);

        if (e->enable_ref_input) {
            /* A comparator against the input: a 1 is "not above" an input at 1, "above" one at 0 or Z. */
            if (vin == LOGIC_1) {
                want = en == LOGIC_X ? PSW_XX : PSW_ON;
            } else {
                want = en == LOGIC_1 ? PSW_OFF : PSW_NONE;
            }
        } else if (vin != LOGIC_1) {
            want = PSW_NONE;
        } else if (en == LOGIC_X) {
            want = PSW_XX;
        } else if (en == (e->enable_low ? LOGIC_0 : LOGIC_1)) {
            want = PSW_ON;
        } else {
            want = PSW_OFF;
        }
    }
    if (want == state->state[k]) {
        return;
    }
    state->state[k] = want;
    for (i = pins->out_from[k]; i < pins->out_from[k + 1]; i++) {
        sim_schedule_drive(sim, instance, pins->out_pins[i],
                           combined(state, pins->out_pins, pins->out_from, pins->out_pins[i], 0),
                           DRIVE_STRONG, 0);
    }
    for (i = pins->flag_from[k]; i < pins->flag_from[k + 1]; i++) {
        sim_schedule_drive(sim, instance, pins->flag_pins[i],
                           combined(state, pins->flag_pins, pins->flag_from, pins->flag_pins[i], 1),
                           DRIVE_STRONG, 0);
    }
}

static const SimComponentType POWERSWITCH_MODEL = {
    .name = "powerswitch",
    .pin_count = powerswitch_pin_count,
    .create = powerswitch_create,
    .destroy = powerswitch_destroy,
    .start = powerswitch_start,
    .pin_changed = powerswitch_pin_changed,
    .timer = powerswitch_timer,
};

const PartKindOps POWERSWITCH_KIND = {
    PART_POWERSWITCH, "powerswitch", &POWERSWITCH_MODEL, DEVICE_KEYS, PART_KEYS,
    powerswitch_load_device, powerswitch_bind_pinout, powerswitch_load_part,
    powerswitch_pin_open_drain, powerswitch_pin_analog
};
