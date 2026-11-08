/*
 * gate: combinational logic gates, one model for every gate part (replaces
 * the old 74_*.c, gates.c and cd4000.c).
 *
 * Each gate computes AND/NAND/OR/NOR/XOR (odd parity)/XNOR (even parity) of
 * its inputs, or NOT of its one input (inverters), X for unknown inputs unless a controlling value decides. The
 * output is driven strong, inertially, after the rise delay for 1, the fall
 * delay for 0, or the longer of the two for X. `start` evaluates every gate;
 * a changed pin re-evaluates every gate it feeds.
 *
 * Two device options:
 *   "output": "open-drain"  a 1 releases the pin (Z) after the rise delay
 *                           (74x266).
 *   timing {otherLow, otherHigh}  2-input gates whose delays depend on the
 *                           input that did not change (74x86): 0 -> otherLow,
 *                           1 -> otherHigh, X (and at start) -> the longer of
 *                           each delay.
 */

#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "model_pins.h"

#define GATE_MAX_INPUTS 13

static const char *const DEVICE_KEYS[] = {"gates", "output", "timing", "units", NULL};
static const char *const PART_KEYS[] = {"family", "vcc", "inputMax", NULL};

typedef enum { OP_AND, OP_NAND, OP_OR, OP_NOR, OP_XOR, OP_XNOR, OP_NOT, OP_BUF } GateOp;

static const char *const OP_NAMES[] = {"and", "nand", "or", "nor", "xor", "xnor", "not", "buf"};

typedef struct {
    SimTime rise;
    SimTime fall;
} Delays;

typedef struct {
    int split;            /* otherLow / otherHigh */
    Delays both;          /* when not split */
    Delays other_low;
    Delays other_high;
    PartLogic logic;      /* family + units, shared by the parts of this family */
    const PartVoltage *vcc;   /* the entry's "vcc"/"inputMax"; NULL = none */
} GateTiming;

typedef struct {
    GateOp op;
    const char *inputs[GATE_MAX_INPUTS];
    size_t input_count;
    const char *output;
} GateDef;

typedef struct {
    GateDef *gates;
    size_t gate_count;
    int open_drain;
    const char **families;   /* timing keys, canonical spelling */
    GateTiming *timings;
    size_t timing_count;
} GateDevice;

typedef struct {
    GateOp op;
    size_t inputs[GATE_MAX_INPUTS];
    size_t input_count;
    size_t output;
} GatePins;

typedef struct {
    const GatePins *gates;
    size_t gate_count;
} GateBound;

static const char *const VOLTAGE_KEYS[] = {"vcc", "inputMax", NULL};

/* extra: keys also allowed (the entry's voltage keys), or NULL. */
static int load_delays(KindContext *ctx, const cJSON *json, const char *const *extra,
                       Delays *out) {
    static const char *const KEYS[] = {"rise", "fall", "note", NULL};
    const cJSON *note = cJSON_GetObjectItemCaseSensitive(json, "note");

    if (kind_check_keys(ctx, json, KEYS, extra) != 0 ||
        kind_ns(ctx, cJSON_GetObjectItemCaseSensitive(json, "rise"), "rise", &out->rise) != 0 ||
        kind_ns(ctx, cJSON_GetObjectItemCaseSensitive(json, "fall"), "fall", &out->fall) != 0) {
        return -1;
    }
    if (note != NULL && !cJSON_IsString(note)) {
        return kind_error(ctx, "\"note\" must be a string");
    }
    return 0;
}

static int load_timing(KindContext *ctx, const cJSON *json, int two_inputs, GateTiming *out) {
    static const char *const SPLIT_KEYS[] = {"otherLow", "otherHigh", "note", "vcc", "inputMax",
                                             NULL};
    const cJSON *low = cJSON_GetObjectItemCaseSensitive(json, "otherLow");

    if (!cJSON_IsObject(json)) {
        return kind_error(ctx, "a timing entry must be an object");
    }
    if (low == NULL) {
        return load_delays(ctx, json, VOLTAGE_KEYS, &out->both) != 0 ||
                       kind_voltage(ctx, json, NULL, &out->vcc) != 0
                   ? -1
                   : 0;
    }
    if (!two_inputs) {
        return kind_error(ctx, "otherLow/otherHigh timing needs every gate to have exactly 2 inputs");
    }
    if (kind_check_keys(ctx, json, SPLIT_KEYS, NULL) != 0 ||
        kind_member(ctx, json, "otherHigh", 1) == NULL ||
        kind_voltage(ctx, json, NULL, &out->vcc) != 0) {
        return -1;
    }
    out->split = 1;
    kind_push(ctx, "otherLow");
    if (load_delays(ctx, low, NULL, &out->other_low) != 0) {
        return -1;
    }
    kind_pop(ctx);
    kind_push(ctx, "otherHigh");
    if (load_delays(ctx, cJSON_GetObjectItemCaseSensitive(json, "otherHigh"), NULL,
                    &out->other_high) != 0) {
        return -1;
    }
    kind_pop(ctx);
    return 0;
}

static int load_gate(KindContext *ctx, const cJSON *json, GateDef *gate) {
    static const char *const KEYS[] = {"op", "inputs", "output", NULL};
    const char *op;
    const char *const *inputs;
    size_t i;

    if (kind_check_keys(ctx, json, KEYS, NULL) != 0 || kind_string(ctx, json, "op", &op) != 0 ||
        kind_names(ctx, json, "inputs", &inputs, &gate->input_count) != 0 ||
        kind_string(ctx, json, "output", &gate->output) != 0) {
        return -1;
    }
    for (i = 0; i < sizeof OP_NAMES / sizeof OP_NAMES[0]; i++) {
        if (strcmp(op, OP_NAMES[i]) == 0) {
            break;
        }
    }
    if (i == sizeof OP_NAMES / sizeof OP_NAMES[0]) {
        return kind_error(ctx, "unknown op \"%s\"", op);
    }
    gate->op = (GateOp)i;
    if (gate->input_count < 1 || gate->input_count > GATE_MAX_INPUTS) {
        return kind_error(ctx, "a gate has 1 to %d inputs", GATE_MAX_INPUTS);
    }
    if ((gate->op == OP_NOT || gate->op == OP_BUF) && gate->input_count != 1) {
        return kind_error(ctx, "a \"%s\" gate has exactly 1 input", op);
    }
    memcpy(gate->inputs, inputs, gate->input_count * sizeof(*inputs));
    return 0;
}

static void *gate_load_device(KindContext *ctx, const cJSON *json) {
    GateDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *gates = kind_member(ctx, json, "gates", 1);
    const cJSON *output = cJSON_GetObjectItemCaseSensitive(json, "output");
    const cJSON *timing = kind_member(ctx, json, "timing", 1);
    const cJSON *item;
    uint64_t units;
    size_t n = 0;
    int two_inputs = 1;

    if (device == NULL || gates == NULL || timing == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(gates) || cJSON_GetArraySize(gates) == 0) {
        kind_error(ctx, "\"gates\" must be a non-empty array");
        return NULL;
    }
    device->gates = kind_alloc(ctx, (size_t)cJSON_GetArraySize(gates) * sizeof(*device->gates));
    if (device->gates == NULL) {
        return NULL;
    }
    cJSON_ArrayForEach(item, gates) {
        kind_push(ctx, "gates[%zu]", n);
        if (load_gate(ctx, item, &device->gates[n]) != 0) {
            return NULL;
        }
        kind_pop(ctx);
        if (device->gates[n].input_count != 2) {
            two_inputs = 0;
        }
        n++;
    }
    device->gate_count = n;

    if (output != NULL) {
        if (!cJSON_IsString(output) ||
            (strcmp(output->valuestring, "push-pull") != 0 &&
             strcmp(output->valuestring, "open-drain") != 0)) {
            kind_error(ctx, "\"output\" must be \"push-pull\" or \"open-drain\"");
            return NULL;
        }
        device->open_drain = strcmp(output->valuestring, "open-drain") == 0;
    }

    /* units: logic gates per package as shown; default the number of gates. */
    units = device->gate_count;
    if (cJSON_GetObjectItemCaseSensitive(json, "units") != NULL &&
        kind_uint(ctx, json, "units", 64, &units) != 0) {
        return NULL;
    }

    if (!cJSON_IsObject(timing) || timing->child == NULL) {
        kind_error(ctx, "\"timing\" must be a non-empty object");
        return NULL;
    }
    device->timing_count = (size_t)cJSON_GetArraySize(timing);
    device->families = kind_alloc(ctx, device->timing_count * sizeof(*device->families));
    device->timings = kind_alloc(ctx, device->timing_count * sizeof(*device->timings));
    if (device->families == NULL || device->timings == NULL) {
        return NULL;
    }
    n = 0;
    cJSON_ArrayForEach(item, timing) {
        const cJSON *other;

        for (other = timing->child; other != item; other = other->next) {
            if (strcmp(other->string, item->string) == 0) {
                kind_error(ctx, "duplicate key \"%s\"", item->string);
                return NULL;
            }
        }
        kind_push(ctx, "timing.%s", item->string);
        device->families[n] = kind_logic_family(ctx, item->string);
        if (device->families[n] == NULL ||
            load_timing(ctx, item, two_inputs, &device->timings[n]) != 0) {
            return NULL;
        }
        device->timings[n].logic.family = device->families[n];
        device->timings[n].logic.units = (unsigned)units;
        kind_pop(ctx);
        n++;
    }
    return device;
}

static void *gate_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                              size_t pin_count) {
    const GateDevice *device = raw;
    GateBound *bound = kind_alloc(ctx, sizeof(*bound));
    GatePins *gates = kind_alloc(ctx, device->gate_count * sizeof(*gates));
    size_t g;
    size_t i;

    if (bound == NULL || gates == NULL) {
        return NULL;
    }
    for (g = 0; g < device->gate_count; g++) {
        const GateDef *def = &device->gates[g];

        kind_push(ctx, "gates[%zu]", g);
        gates[g].op = def->op;
        gates[g].input_count = def->input_count;
        for (i = 0; i < def->input_count; i++) {
            if (kind_pin(ctx, pins, pin_count, def->inputs[i], &gates[g].inputs[i]) != 0) {
                return NULL;
            }
        }
        if (kind_pin(ctx, pins, pin_count, def->output, &gates[g].output) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    bound->gates = gates;
    bound->gate_count = device->gate_count;
    return bound;
}

static int gate_load_part(KindContext *ctx, const void *raw, const void *bound,
                          const cJSON *json, Part *out) {
    const GateDevice *device = raw;
    const char *family;
    size_t i;

    (void)bound;
    if (kind_string(ctx, json, "family", &family) != 0 || kind_logic_family(ctx, family) == NULL) {
        return -1;
    }
    for (i = 0; i < device->timing_count; i++) {
        if (strcmp(device->families[i], family) == 0) {
            out->logic = &device->timings[i].logic;
            kind_set_part_data(out, &device->timings[i]);
            return kind_part_voltage(ctx, json, device->timings[i].vcc, &out->vcc);
        }
    }
    return kind_error(ctx, "family \"%s\" has no timing entry", family);
}

/* Every output pin of an open-drain device (part_pin_open_drain checks the role). */
static int gate_pin_open_drain(const Part *part, size_t pin) {
    (void)pin;
    return ((const GateDevice *)part_internal(part)->device)->open_drain;
}

/* ---- Model --------------------------------------------------------------- */

typedef struct {
    const GateBound *bound;
    const GateTiming *timing;
    int open_drain;
} GateState;

static size_t gate_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *gate_create(const void *config) {
    const PartInternal *internal = part_internal(config);
    GateState *state = malloc(sizeof(*state));

    if (state != NULL) {
        state->bound = internal->bound;
        state->timing = internal->data;
        state->open_drain = ((const GateDevice *)internal->device)->open_drain;
    }
    return state;
}

static void gate_destroy(void *state) {
    free(state);
}

static LogicValue gate_value(const GatePins *gate, const LogicValue *in) {
    size_t i;
    int any_unknown = 0;
    int ones = 0;
    LogicValue v;

    switch (gate->op) {
        case OP_NOT:
        case OP_BUF:
        case OP_AND:
        case OP_NAND:
            v = LOGIC_1;
            for (i = 0; i < gate->input_count; i++) {
                if (in[i] == LOGIC_0) {
                    v = LOGIC_0;
                    break;
                }
                if (in[i] != LOGIC_1) {
                    v = LOGIC_X;
                }
            }
            break;
        case OP_OR:
        case OP_NOR:
            v = LOGIC_0;
            for (i = 0; i < gate->input_count; i++) {
                if (in[i] == LOGIC_1) {
                    v = LOGIC_1;
                    break;
                }
                if (in[i] != LOGIC_0) {
                    v = LOGIC_X;
                }
            }
            break;
        case OP_XOR:
        case OP_XNOR:
            for (i = 0; i < gate->input_count; i++) {
                if (in[i] == LOGIC_1) {
                    ones++;
                } else if (in[i] != LOGIC_0) {
                    any_unknown = 1;
                }
            }
            v = any_unknown ? LOGIC_X : (ones & 1) ? LOGIC_1 : LOGIC_0;
            break;
        default:
            return LOGIC_X;
    }
    if (v != LOGIC_X && (gate->op == OP_NAND || gate->op == OP_NOR || gate->op == OP_XNOR ||
                           gate->op == OP_NOT)) {
        v = v == LOGIC_1 ? LOGIC_0 : LOGIC_1;
    }
    return v;
}

static SimTime longer(SimTime a, SimTime b) {
    return a > b ? a : b;
}

/* The delays for the current value of the input that did not change. */
static Delays select_delays(const GateTiming *timing, LogicValue other) {
    Delays worst;

    if (!timing->split) {
        return timing->both;
    }
    if (other == LOGIC_0) {
        return timing->other_low;
    }
    if (other == LOGIC_1) {
        return timing->other_high;
    }
    worst.rise = longer(timing->other_low.rise, timing->other_high.rise);
    worst.fall = longer(timing->other_low.fall, timing->other_high.fall);
    return worst;
}

static void evaluate(Sim *sim, SimInstance *instance, const GateState *state,
                     const GatePins *gate, LogicValue other) {
    LogicValue in[GATE_MAX_INPUTS];
    LogicValue out;
    Delays delays = select_delays(state->timing, other);
    SimTime delay;
    size_t i;

    for (i = 0; i < gate->input_count; i++) {
        in[i] = sim_read_pin(sim, instance, gate->inputs[i]);
    }
    out = gate_value(gate, in);
    if (out == LOGIC_1) {
        delay = delays.rise;
        if (state->open_drain) {
            out = LOGIC_Z;   /* released; the external pull-up takes it high */
        }
    } else if (out == LOGIC_0) {
        delay = delays.fall;
    } else {
        delay = longer(delays.rise, delays.fall);
    }
    sim_schedule_inertial_drive(sim, instance, gate->output, out, DRIVE_STRONG, delay);
}

static void gate_start(Sim *sim, SimInstance *instance, void *raw) {
    const GateState *state = raw;
    size_t g;

    /* No edge has happened yet: the "other input" counts as unknown. */
    for (g = 0; g < state->bound->gate_count; g++) {
        evaluate(sim, instance, state, &state->bound->gates[g], LOGIC_X);
    }
}

static void gate_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                             LogicValue old_value, LogicValue new_value) {
    const GateState *state = raw;
    size_t g;
    size_t i;

    (void)old_value;
    (void)new_value;
    for (g = 0; g < state->bound->gate_count; g++) {
        const GatePins *gate = &state->bound->gates[g];

        for (i = 0; i < gate->input_count; i++) {
            if (gate->inputs[i] == pin) {
                LogicValue other = LOGIC_X;

                if (gate->input_count == 2) {
                    other = sim_read_pin(sim, instance, gate->inputs[1 - i]);
                }
                evaluate(sim, instance, state, gate, other);
                break;
            }
        }
    }
}

static const SimComponentType GATE_MODEL = {
    "gate", gate_pin_count, gate_create, gate_destroy, gate_start, gate_pin_changed, NULL,
    NULL, NULL, NULL, NULL
};

const PartKindOps GATE_KIND = {
    PART_GATE, "gate", &GATE_MODEL, DEVICE_KEYS, PART_KEYS,
    gate_load_device, gate_bind_pinout, gate_load_part, gate_pin_open_drain
};
