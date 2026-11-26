/*
 * regulator (plan-42-G): voltage regulators, one to four regulators
 * (elements) per package, linear (fixed, adjustable, LDO) or switching with
 * an internal switch. Each element is a regulated supply for a logic
 * simulator: while its input pin reads 1 and its enable pin (when it has
 * one) is active, its output pin is driven strong 1; no analog behaviour
 * (no ramp, load, limit, ripple or thermal shutdown). The output voltage is
 * only for the voltage check (voltage.c): the part's `vout`, or for an
 * adjustable element Vref worked out from the divider on its adjust pin.
 *
 * A change at the input or the enable reaches the output 1 ns later
 * (REGULATOR_DELAY_PS, from a timer that re-reads the pins), so a pulse
 * shorter than that never moves it. A floating enable is inactive for both
 * polarities. The optional power-good pin (open drain) is pulled low while
 * the input is there and the element is disabled.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "model_pins.h"
#include "logic_alloc.h"

#define REGULATOR_DELAY_PS 1000u

static const char *const DEVICE_KEYS[] = {"regulators", "analog", NULL};
static const char *const PART_KEYS[] = {"vout", "vref", "voutMax", "vinMin", "vinMax", "dropout",
                                        "iout", "iq", "fsw", NULL};

typedef struct {
    const char *type;      /* "linear", "buck", "boost", "buck-boost", "charge-pump", "sepic" */
    int linear;
    const char *in;
    const char *out;
    const char *enable;    /* NULL = none */
    int enable_low;
    const char *pg;        /* NULL = none */
    const char *adjust;    /* NULL = fixed */
    int adjust_output;     /* adjustTo "output": Vref sits between OUT and ADJ */
} RegulatorElement;

typedef struct {
    size_t count;
    RegulatorElement *elements;
    const char *const *analog;
    size_t analog_count;
    int mixed;
    int adjustable;
} RegulatorDevice;

/* A pinout's pins as indices into Part.pins: per element the first pin of
   each name (SIZE_MAX when the element has none), every pin of the input
   and of the output name, and per pin its analog / open-drain flags. */
typedef struct {
    size_t count;
    const RegulatorDevice *device;
    size_t *in_first, *out_first, *enable, *pg, *adjust;
    size_t *in_from, *in_pins;     /* element k's input pins: in_pins[in_from[k] .. in_from[k + 1]) */
    size_t *out_from, *out_pins;
    size_t *pg_from, *pg_pins;
    unsigned char *analog;         /* per pin */
    unsigned char *open_drain;     /* per pin: a power-good pin */
} RegulatorPins;

static const char *const TYPES[] = {"linear", "buck", "boost", "buck-boost", "charge-pump", "sepic", NULL};

static void *regulator_load_device(KindContext *ctx, const cJSON *json) {
    static const char *const ELEMENT_KEYS[] = {"type", "in", "out", "enable", "enableActive", "pg",
                                               "adjust", "adjustTo", NULL};
    RegulatorDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *elements;
    const cJSON *item;
    size_t count = 0;

    if (device == NULL) {
        return NULL;
    }
    elements = kind_member(ctx, json, "regulators", 1);
    if (elements == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(elements) || cJSON_GetArraySize(elements) < 1 || cJSON_GetArraySize(elements) > 4) {
        kind_error(ctx, "\"regulators\" must have 1 to 4 entries");
        return NULL;
    }
    device->elements = kind_alloc(ctx, (size_t)cJSON_GetArraySize(elements) * sizeof(*device->elements));
    if (device->elements == NULL) {
        return NULL;
    }
    cJSON_ArrayForEach(item, elements) {
        RegulatorElement *e = &device->elements[count];
        size_t t;
        int known = 0;

        kind_push(ctx, "regulators[%zu]", count);
        if (!cJSON_IsObject(item)) {
            kind_error(ctx, "a regulator is {\"type\": …, \"in\": …, \"out\": …}");
            return NULL;
        }
        if (kind_check_keys(ctx, item, ELEMENT_KEYS, NULL) != 0 || kind_string(ctx, item, "type", &e->type) != 0) {
            return NULL;
        }
        for (t = 0; TYPES[t] != NULL; t++) {
            known |= strcmp(e->type, TYPES[t]) == 0;
        }
        if (!known) {
            kind_error(ctx, "unknown regulator type \"%s\" (linear, buck, boost, buck-boost, charge-pump, sepic)",
                       e->type);
            return NULL;
        }
        e->linear = strcmp(e->type, "linear") == 0;
        if (kind_string(ctx, item, "in", &e->in) != 0 || kind_string(ctx, item, "out", &e->out) != 0) {
            return NULL;
        }
        if (cJSON_GetObjectItemCaseSensitive(item, "enable") != NULL &&
            kind_string(ctx, item, "enable", &e->enable) != 0) {
            return NULL;
        }
        if (cJSON_GetObjectItemCaseSensitive(item, "enableActive") != NULL) {
            const char *level;

            if (e->enable == NULL) {
                kind_error(ctx, "\"enableActive\" needs \"enable\"");
                return NULL;
            }
            if (kind_string(ctx, item, "enableActive", &level) != 0) {
                return NULL;
            }
            if (strcmp(level, "high") != 0 && strcmp(level, "low") != 0) {
                kind_error(ctx, "\"enableActive\" must be \"high\" or \"low\"");
                return NULL;
            }
            e->enable_low = strcmp(level, "low") == 0;
        }
        if (cJSON_GetObjectItemCaseSensitive(item, "pg") != NULL && kind_string(ctx, item, "pg", &e->pg) != 0) {
            return NULL;
        }
        if (cJSON_GetObjectItemCaseSensitive(item, "adjust") != NULL &&
            kind_string(ctx, item, "adjust", &e->adjust) != 0) {
            return NULL;
        }
        if (cJSON_GetObjectItemCaseSensitive(item, "adjustTo") != NULL) {
            const char *to;

            if (e->adjust == NULL) {
                kind_error(ctx, "\"adjustTo\" needs \"adjust\"");
                return NULL;
            }
            if (kind_string(ctx, item, "adjustTo", &to) != 0) {
                return NULL;
            }
            if (strcmp(to, "ground") != 0 && strcmp(to, "output") != 0) {
                kind_error(ctx, "\"adjustTo\" must be \"ground\" or \"output\"");
                return NULL;
            }
            e->adjust_output = strcmp(to, "output") == 0;
        }
        if (strcmp(e->in, e->out) == 0) {
            kind_error(ctx, "a regulator's input and output must have different names");
            return NULL;
        }
        if (count > 0 && strcmp(e->type, device->elements[0].type) != 0) {
            device->mixed = 1;
        }
        device->adjustable |= e->adjust != NULL;
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

/* Every pin with this name must have one of the roles; the element key is for the message. */
static int check_roles(KindContext *ctx, const PartPin *pins, const size_t *list, size_t n, const char *key,
                       PinRole a, PinRole b) {
    size_t i;

    for (i = 0; i < n; i++) {
        if (pins[list[i]].role != a && pins[list[i]].role != b) {
            return kind_error(ctx, "regulator pin \"%s\" (%s) must be %s", pins[list[i]].name, key,
                              a == PIN_POWER ? "power" : a == PIN_OUTPUT ? "output" : "input");
        }
    }
    return 0;
}

static void *regulator_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins, size_t pin_count) {
    const RegulatorDevice *device = raw;
    RegulatorPins *bound = kind_alloc(ctx, sizeof(*bound));
    size_t *scratch = kind_alloc(ctx, (pin_count + 1) * sizeof(*scratch));
    unsigned char *used = kind_alloc(ctx, pin_count + 1);
    size_t cap = device->count * pin_count + 1;
    size_t ni = 0, no = 0, np = 0;
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
    bound->pg = kind_alloc(ctx, (device->count + 1) * sizeof(size_t));
    bound->adjust = kind_alloc(ctx, (device->count + 1) * sizeof(size_t));
    bound->in_from = kind_alloc(ctx, (device->count + 1) * sizeof(size_t));
    bound->out_from = kind_alloc(ctx, (device->count + 1) * sizeof(size_t));
    bound->pg_from = kind_alloc(ctx, (device->count + 1) * sizeof(size_t));
    bound->in_pins = kind_alloc(ctx, cap * sizeof(size_t));
    bound->out_pins = kind_alloc(ctx, cap * sizeof(size_t));
    bound->pg_pins = kind_alloc(ctx, cap * sizeof(size_t));
    bound->analog = kind_alloc(ctx, pin_count + 1);
    bound->open_drain = kind_alloc(ctx, pin_count + 1);
    if (bound->in_first == NULL || bound->out_first == NULL || bound->enable == NULL || bound->pg == NULL ||
        bound->adjust == NULL || bound->in_from == NULL || bound->out_from == NULL || bound->pg_from == NULL ||
        bound->in_pins == NULL || bound->out_pins == NULL || bound->pg_pins == NULL || bound->analog == NULL ||
        bound->open_drain == NULL) {
        return NULL;
    }
    /* The analog list first: it is what an adjust pin must be in. */
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
        const RegulatorElement *e = &device->elements[k];
        size_t n;
        size_t x;

        bound->in_from[k] = ni;
        bound->out_from[k] = no;
        bound->pg_from[k] = np;
        bound->enable[k] = bound->pg[k] = bound->adjust[k] = SIZE_MAX;
        n = kind_pins(ctx, pins, pin_count, e->in, scratch, pin_count);
        if (n == 0 || check_roles(ctx, pins, scratch, n, "in", PIN_POWER, PIN_POWER) != 0) {
            return NULL;
        }
        bound->in_first[k] = scratch[0];
        for (x = 0; x < n; x++) {
            bound->in_pins[ni++] = scratch[x];
            used[scratch[x]] = 1;
        }
        n = kind_pins(ctx, pins, pin_count, e->out, scratch, pin_count);
        if (n == 0 || check_roles(ctx, pins, scratch, n, "out", PIN_OUTPUT, PIN_OUTPUT) != 0) {
            return NULL;
        }
        bound->out_first[k] = scratch[0];
        for (x = 0; x < n; x++) {
            bound->out_pins[no++] = scratch[x];
            used[scratch[x]] = 1;
        }
        if (e->enable != NULL) {
            n = kind_pins(ctx, pins, pin_count, e->enable, scratch, pin_count);
            if (n == 0 || check_roles(ctx, pins, scratch, n, "enable", PIN_INPUT, PIN_INPUT) != 0) {
                return NULL;
            }
            bound->enable[k] = scratch[0];
            for (x = 0; x < n; x++) {
                used[scratch[x]] = 1;
            }
        }
        if (e->pg != NULL) {
            n = kind_pins(ctx, pins, pin_count, e->pg, scratch, pin_count);
            if (n == 0 || check_roles(ctx, pins, scratch, n, "pg", PIN_OUTPUT, PIN_OUTPUT) != 0) {
                return NULL;
            }
            bound->pg[k] = scratch[0];
            for (x = 0; x < n; x++) {
                bound->pg_pins[np++] = scratch[x];
                bound->open_drain[scratch[x]] = 1;
                used[scratch[x]] = 1;
            }
        }
        if (e->adjust != NULL) {
            n = kind_pins(ctx, pins, pin_count, e->adjust, scratch, pin_count);
            if (n == 0 || check_roles(ctx, pins, scratch, n, "adjust", PIN_INPUT, PIN_INOUT) != 0) {
                return NULL;
            }
            bound->adjust[k] = scratch[0];
            for (x = 0; x < n; x++) {
                if (!bound->analog[scratch[x]]) {
                    kind_error(ctx, "regulator pin \"%s\" (adjust) must be in \"analog\"", e->adjust);
                    return NULL;
                }
                used[scratch[x]] = 1;
            }
        }
    }
    bound->in_from[device->count] = ni;
    bound->out_from[device->count] = no;
    bound->pg_from[device->count] = np;
    for (i = 0; i < pin_count; i++) {
        if (pins[i].role != PIN_NC && pins[i].role != PIN_ABSENT && pins[i].role != PIN_GROUND && !used[i]) {
            kind_error(ctx, "pin \"%s\" is in no regulator and not analog", pins[i].name);
            return NULL;
        }
    }
    return bound;
}

/* A number of volts in (0, max_v] (zero_ok: [0, max_v]) as a milli-unit count. */
static int volts_value(KindContext *ctx, const cJSON *value, const char *key, double max_v, int zero_ok,
                       uint32_t *mv) {
    if (!cJSON_IsNumber(value) || !(zero_ok ? value->valuedouble >= 0 : value->valuedouble > 0) ||
        value->valuedouble > max_v) {
        return kind_error(ctx, "\"%s\" must be %s 0 and at most %g", key, zero_ok ? "at least" : "above", max_v);
    }
    *mv = (uint32_t)llround(value->valuedouble * 1000);
    return 0;
}

/* One value per element: a number holds for every element, an array has one
   entry per element, a null entry is "none for this element". */
static int per_element(KindContext *ctx, const cJSON *json, const char *key, double max_v, size_t count,
                       uint32_t *out) {
    const cJSON *item = kind_member(ctx, json, key, 0);
    size_t k;

    memset(out, 0, count * sizeof(*out));
    if (item == NULL) {
        return 0;
    }
    if (cJSON_IsArray(item)) {
        const cJSON *entry;

        if ((size_t)cJSON_GetArraySize(item) != count) {
            return kind_error(ctx, "\"%s\" must have one value per regulator (%zu)", key, count);
        }
        k = 0;
        cJSON_ArrayForEach(entry, item) {
            if (!cJSON_IsNull(entry) && volts_value(ctx, entry, key, max_v, 0, &out[k]) != 0) {
                return -1;
            }
            k++;
        }
        return 0;
    }
    if (volts_value(ctx, item, key, max_v, 0, &out[0]) != 0) {
        return -1;
    }
    for (k = 1; k < count; k++) {
        out[k] = out[0];
    }
    return 0;
}

static int optional_scaled(KindContext *ctx, const cJSON *json, const char *key, double max, double scale,
                           int zero_ok, uint64_t *out) {
    const cJSON *item = kind_member(ctx, json, key, 0);

    *out = 0;
    if (item == NULL) {
        return 0;
    }
    if (!cJSON_IsNumber(item) || !(zero_ok ? item->valuedouble >= 0 : item->valuedouble > 0) ||
        item->valuedouble > max) {
        return kind_error(ctx, "\"%s\" must be %s 0 and at most %g", key, zero_ok ? "at least" : "above", max);
    }
    *out = (uint64_t)llround(item->valuedouble * scale);
    return 0;
}

static int regulator_load_part(KindContext *ctx, const void *raw, const void *bound, const cJSON *json,
                               Part *out) {
    const RegulatorDevice *device = raw;
    PartRegulator *info = kind_alloc(ctx, sizeof(*info));
    uint32_t *vout = kind_alloc(ctx, (device->count + 1) * sizeof(*vout));
    uint32_t *vref = kind_alloc(ctx, (device->count + 1) * sizeof(*vref));
    uint32_t vout_max = 0, vin_min = 0, vin_max = 0, dropout = 0;
    uint64_t iout = 0, iq = 0, fsw = 0;
    int any_linear = 0, any_switching = 0;
    size_t k;

    (void)bound;
    if (info == NULL || vout == NULL || vref == NULL) {
        return -1;
    }
    for (k = 0; k < device->count; k++) {
        any_linear |= device->elements[k].linear;
        any_switching |= !device->elements[k].linear;
    }
    if (per_element(ctx, json, "vout", 100, device->count, vout) != 0 ||
        per_element(ctx, json, "vref", 10, device->count, vref) != 0) {
        return -1;
    }
    for (k = 0; k < device->count; k++) {
        if (device->elements[k].adjust != NULL) {
            if (vout[k] != 0) {
                return kind_error(ctx, "\"vout\" is not for a regulator with \"adjust\"");
            }
            if (vref[k] == 0) {
                return kind_error(ctx, "\"vref\" is required for a regulator with \"adjust\"");
            }
        } else {
            if (vref[k] != 0) {
                return kind_error(ctx, "\"vref\" is only for a regulator with \"adjust\"");
            }
            if (vout[k] == 0) {
                return kind_error(ctx, "\"vout\" is required for a regulator without \"adjust\"");
            }
        }
    }
    /* A part mixing fixed and adjustable regulators has given "vout" and "vref" as arrays: a single
       "vout" is every element's (per_element), so the adjustable one refused it above, and with no
       "vout" the fixed one did. */
    if (kind_member(ctx, json, "voutMax", 0) != NULL &&
        (volts_value(ctx, kind_member(ctx, json, "voutMax", 0), "voutMax", 1000, 0, &vout_max) != 0)) {
        return -1;
    }
    if (kind_member(ctx, json, "vinMin", 0) != NULL &&
        (volts_value(ctx, kind_member(ctx, json, "vinMin", 0), "vinMin", 100, 0, &vin_min) != 0)) {
        return -1;
    }
    if (kind_member(ctx, json, "vinMax", 0) != NULL &&
        (volts_value(ctx, kind_member(ctx, json, "vinMax", 0), "vinMax", 1000, 0, &vin_max) != 0)) {
        return -1;
    }
    if (kind_member(ctx, json, "dropout", 0) != NULL) {
        if (!any_linear) {
            return kind_error(ctx, "\"dropout\" is only for a linear regulator");
        }
        if (volts_value(ctx, kind_member(ctx, json, "dropout", 0), "dropout", 10, 1, &dropout) != 0) {
            return -1;
        }
    }
    if (optional_scaled(ctx, json, "iout", 100, 1000, 0, &iout) != 0 ||
        optional_scaled(ctx, json, "iq", 1, 1e6, 0, &iq) != 0) {
        return -1;
    }
    if (kind_member(ctx, json, "fsw", 0) != NULL) {
        if (!any_switching) {
            return kind_error(ctx, "\"fsw\" is only for a switching regulator");
        }
        if (optional_scaled(ctx, json, "fsw", 1e8, 1, 0, &fsw) != 0) {
            return -1;
        }
    }
    info->type = device->elements[0].type;
    info->regulator_count = device->count;
    info->mixed = (uint8_t)device->mixed;
    info->adjustable = (uint8_t)device->adjustable;
    info->vout_mv = vout;
    info->vref_mv = vref;
    info->vout_max_mv = vout_max;
    info->vin_min_mv = vin_min;
    info->vin_max_mv = vin_max;
    info->dropout_mv = dropout;
    info->iout_ma = (uint32_t)iout;
    info->iq_ua = (uint32_t)iq;
    info->fsw_hz = fsw;
    out->regulator = info;
    return 0;
}

/* ---- Facts for the voltage check ---------------------------------------- */

int regulator_element(const Part *part, size_t k, const char **type, size_t *in, size_t *out, size_t *enable,
                      int *enable_low, size_t *pg, size_t *adjust, int *adjust_to_output) {
    const RegulatorPins *bound;
    const RegulatorElement *e;

    if (part == NULL || part->kind != PART_REGULATOR) {
        return 0;
    }
    bound = part_internal(part)->bound;
    if (k >= bound->count) {
        return 0;
    }
    e = &bound->device->elements[k];
    if (type != NULL) {
        *type = e->type;
    }
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
    if (pg != NULL) {
        *pg = bound->pg[k];
    }
    if (adjust != NULL) {
        *adjust = bound->adjust[k];
    }
    if (adjust_to_output != NULL) {
        *adjust_to_output = e->adjust_output;
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

int regulator_pin_in(const Part *part, size_t k, size_t pin) {
    const RegulatorPins *bound;

    if (part == NULL || part->kind != PART_REGULATOR) {
        return 0;
    }
    bound = part_internal(part)->bound;
    return k < bound->count && pin_listed(bound->in_pins, bound->in_from[k], bound->in_from[k + 1], pin);
}

int regulator_pin_out(const Part *part, size_t k, size_t pin) {
    const RegulatorPins *bound;

    if (part == NULL || part->kind != PART_REGULATOR) {
        return 0;
    }
    bound = part_internal(part)->bound;
    return k < bound->count && pin_listed(bound->out_pins, bound->out_from[k], bound->out_from[k + 1], pin);
}

uint32_t regulator_input_max_mv(const Part *part) {
    if (part == NULL || part->kind != PART_REGULATOR || part->regulator == NULL) {
        return 0;
    }
    return part->regulator->vin_max_mv;
}

static int regulator_pin_open_drain(const Part *part, size_t pin) {
    const RegulatorPins *bound = part_internal(part)->bound;

    return bound->open_drain[pin];
}

static int regulator_pin_analog(const Part *part, size_t pin) {
    const RegulatorPins *bound = part_internal(part)->bound;

    return bound->analog[pin];
}

/* ---- Model --------------------------------------------------------------- */

/* What an element drives: its output pins and its power-good pins. */
enum { REG_NONE, REG_ON, REG_OFF, REG_X, REG_XX };

typedef struct {
    const RegulatorPins *pins;
    unsigned char state[];
} RegulatorState;

static size_t regulator_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *regulator_create(const void *config) {
    const RegulatorPins *pins = part_internal(config)->bound;
    RegulatorState *state = logic_calloc(1, sizeof(*state) + pins->count + 1);

    if (state != NULL) {
        state->pins = pins;
    }
    return state;
}

static void regulator_destroy(void *state) {
    logic_free(state);
}

static void regulator_start(Sim *sim, SimInstance *instance, void *raw) {
    RegulatorState *state = raw;
    size_t k;

    for (k = 0; k < state->pins->count; k++) {
        sim_schedule_timer(sim, instance, (uint32_t)k, REGULATOR_DELAY_PS);
    }
}

static void regulator_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin, LogicValue old_value,
                                  LogicValue new_value) {
    RegulatorState *state = raw;
    const RegulatorPins *pins = state->pins;
    size_t k;

    (void)old_value;
    (void)new_value;
    for (k = 0; k < pins->count; k++) {
        if (pin_listed(pins->in_pins, pins->in_from[k], pins->in_from[k + 1], pin) ||
            (pins->enable[k] != SIZE_MAX && pins->enable[k] == pin)) {
            sim_schedule_timer(sim, instance, (uint32_t)k, REGULATOR_DELAY_PS);
        }
    }
}

/* The reading of an element's input: 1 if any pin of the name reads 1 (and
   none X), X if any reads X, 0 if one reads 0, else Z. */
static LogicValue read_input(const Sim *sim, const SimInstance *instance, const RegulatorPins *pins, size_t k) {
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

static void regulator_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t timer_id) {
    RegulatorState *state = raw;
    const RegulatorPins *pins = state->pins;
    size_t k = timer_id;   /* < count: start and pin_changed schedule only an element k < count */
    LogicValue vin;
    unsigned char want;
    size_t i;

    vin = read_input(sim, instance, pins, k);
    if (vin == LOGIC_X) {
        want = REG_X;
    } else if (vin != LOGIC_1) {
        want = REG_NONE;
    } else if (pins->enable[k] == SIZE_MAX) {
        want = REG_ON;
    } else {
        LogicValue en = sim_read_pin(sim, instance, pins->enable[k]);
        int low = pins->device->elements[k].enable_low;

        if (en == LOGIC_X) {
            want = REG_XX;
        } else if (en == (low ? LOGIC_0 : LOGIC_1)) {
            want = REG_ON;
        } else {
            want = REG_OFF;
        }
    }
    if (want == state->state[k]) {
        return;
    }
    state->state[k] = want;
    for (i = pins->out_from[k]; i < pins->out_from[k + 1]; i++) {
        LogicValue v = want == REG_ON ? LOGIC_1 : want == REG_X || want == REG_XX ? LOGIC_X : LOGIC_Z;

        sim_schedule_drive(sim, instance, pins->out_pins[i], v, DRIVE_STRONG, 0);
    }
    for (i = pins->pg_from[k]; i < pins->pg_from[k + 1]; i++) {
        LogicValue v = want == REG_OFF ? LOGIC_0 : want == REG_XX ? LOGIC_X : LOGIC_Z;

        sim_schedule_drive(sim, instance, pins->pg_pins[i], v, DRIVE_STRONG, 0);
    }
}

static const SimComponentType REGULATOR_MODEL = {
    .name = "regulator",
    .pin_count = regulator_pin_count,
    .create = regulator_create,
    .destroy = regulator_destroy,
    .start = regulator_start,
    .pin_changed = regulator_pin_changed,
    .timer = regulator_timer,
};

const PartKindOps REGULATOR_KIND = {
    PART_REGULATOR, "regulator", &REGULATOR_MODEL, DEVICE_KEYS, PART_KEYS,
    regulator_load_device, regulator_bind_pinout, regulator_load_part,
    regulator_pin_open_drain, regulator_pin_analog
};
