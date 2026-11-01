#include "gates.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 * Generic N-input logic gates: single and dual little-logic parts (1G, 2G, LV1T),
 * triple 3-input, dual 4-input and 8/13-input 74-series parts, and the
 * multi-input CD4000 parts. Each definition is data generated from the TI
 * datasheets (gates_defs.inc): its pinout, which pins feed which output, the
 * per-family timing, and the orderable part numbers.
 */

#define GT_MAX_INPUTS 13
#define GT_COUNT(array) (sizeof(array) / sizeof((array)[0]))

typedef enum {
    GT_AND,
    GT_NAND,
    GT_OR,
    GT_NOR,
    GT_XOR,   /* odd parity for more than two inputs */
    GT_XNOR   /* even parity */
} GtOp;

typedef struct {
    GtOp op;
    unsigned char input_count;
    unsigned char inputs[GT_MAX_INPUTS]; /* 0-based pin indices */
    unsigned char output;
} GtGate;

typedef struct {
    size_t pin_count;
    const GtGate *gates;
    size_t gate_count;
} GtLayout;

typedef struct {
    LogicFamily family;
    SimTime rise_delay;   /* tPLH, picoseconds */
    SimTime fall_delay;   /* tPHL, picoseconds */
} GtFamily;

typedef struct {
    const char *name;
    LogicFamily family;
    PackageType package;
} GtPart;

typedef struct {
    const GtLayout *layout;
    const GtFamily *families;
    size_t family_count;
    const GtPart *parts;
    size_t part_count;
} GtData;

typedef struct {
    const GtLayout *layout;
    GtFamily timing;
} GtState;

static void unsupported(void) {
    assert(0 && "unsupported gate configuration");
    abort();
}

static const GtFamily *find_family(const GtData *data, LogicFamily family) {
    size_t i;

    for (i = 0; i < data->family_count; i++) {
        if (data->families[i].family == family) {
            return &data->families[i];
        }
    }
    return NULL;
}

static int part_exists(const GtData *data, LogicFamily family, PackageType package) {
    size_t i;

    for (i = 0; i < data->part_count; i++) {
        if (data->parts[i].family == family && data->parts[i].package == package) {
            return 1;
        }
    }
    return 0;
}

static int lookup(const GtData *data, const char *name, SimLogicConfig *config) {
    size_t i;

    for (i = 0; i < data->part_count; i++) {
        if (strcmp(data->parts[i].name, name) == 0) {
            config->family = data->parts[i].family;
            config->package = data->parts[i].package;
            return 1;
        }
    }
    return 0;
}

static void validate(const GtData *data, const void *raw) {
    const SimLogicConfig *config = raw;

    if (config == NULL || find_family(data, config->family) == NULL ||
        !part_exists(data, config->family, config->package)) {
        unsupported();
    }
}

static size_t gt_pin_count(const GtData *data, const void *config) {
    validate(data, config);
    return data->layout->pin_count;
}

static void *gt_create(const GtData *data, const void *raw) {
    const SimLogicConfig *config = raw;
    GtState *state;

    validate(data, config);
    state = malloc(sizeof(*state));
    if (state != NULL) {
        state->layout = data->layout;
        state->timing = *find_family(data, config->family);
    }
    return state;
}

static void gt_destroy(void *state) {
    free(state);
}

static LogicValue gate_value(const GtGate *gate, const LogicValue *in) {
    size_t i;
    int any_unknown = 0;
    int ones = 0;
    LogicValue v;

    switch (gate->op) {
        case GT_AND:
        case GT_NAND:
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

        case GT_OR:
        case GT_NOR:
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

        case GT_XOR:
        case GT_XNOR:
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

    if (v != LOGIC_X && (gate->op == GT_NAND || gate->op == GT_NOR || gate->op == GT_XNOR)) {
        v = v == LOGIC_1 ? LOGIC_0 : LOGIC_1;
    }
    return v;
}

static void evaluate(Sim *sim, SimInstance *instance, const GtState *state, const GtGate *gate) {
    LogicValue in[GT_MAX_INPUTS];
    LogicValue out;
    SimTime delay;
    size_t i;

    for (i = 0; i < gate->input_count; i++) {
        in[i] = sim_read_pin(sim, instance, gate->inputs[i]);
    }
    out = gate_value(gate, in);
    if (out == LOGIC_1) {
        delay = state->timing.rise_delay;
    } else if (out == LOGIC_0) {
        delay = state->timing.fall_delay;
    } else {
        delay = state->timing.rise_delay > state->timing.fall_delay
            ? state->timing.rise_delay : state->timing.fall_delay;
    }
    sim_schedule_inertial_drive(sim, instance, gate->output, out, DRIVE_STRONG, delay);
}

static void gt_start(Sim *sim, SimInstance *instance, void *raw) {
    const GtState *state = raw;
    size_t g;

    for (g = 0; g < state->layout->gate_count; g++) {
        evaluate(sim, instance, state, &state->layout->gates[g]);
    }
}

/* Every gate fed by the pin is re-evaluated; some parts share inputs between outputs. */
static void gt_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    const GtState *state = raw;
    size_t g;
    size_t i;

    (void)old_value;
    (void)new_value;
    for (g = 0; g < state->layout->gate_count; g++) {
        const GtGate *gate = &state->layout->gates[g];

        for (i = 0; i < gate->input_count; i++) {
            if (gate->inputs[i] == pin) {
                evaluate(sim, instance, state, gate);
                break;
            }
        }
    }
}

/* Part lists separate entries with commas, so these expand without one. */
#define GT_PART_ENTRY(name, family, package, flags) {name, family, package}
#define GT_PART_NAME(name, family, package, flags) name
#define GT_PART_FLAGS(name, family, package, flags) flags

/*
 * Each definition gets its own SimComponentType, because the model callbacks
 * receive only the configuration and need the data from their definition.
 */
#define GT_DEFINE(NAME, ID, FUNCTION, DESCRIPTION, LAYOUT, PINS, FAMILIES, PART_LIST, GATES) \
    static const GtPart NAME##_PARTS[] = { PART_LIST(GT_PART_ENTRY) }; \
    static const char *const NAME##_NAMES[] = { PART_LIST(GT_PART_NAME) }; \
    static const unsigned NAME##_FLAGS[] = { PART_LIST(GT_PART_FLAGS) }; \
    static const GtData NAME##_DATA = { \
        &LAYOUT, FAMILIES, GT_COUNT(FAMILIES), NAME##_PARTS, GT_COUNT(NAME##_PARTS) \
    }; \
    static size_t NAME##_pin_count(const void *config) { \
        return gt_pin_count(&NAME##_DATA, config); \
    } \
    static void *NAME##_create(const void *config) { \
        return gt_create(&NAME##_DATA, config); \
    } \
    static const SimComponentType NAME##_TYPE = { \
        ID, NAME##_pin_count, NAME##_create, gt_destroy, \
        gt_start, gt_pin_changed, NULL \
    }; \
    static const SimComponentType *NAME##_lookup( \
        const char *part_number, SimLogicConfig *config \
    ) { \
        if (part_number == NULL || config == NULL) { \
            return NULL; \
        } \
        return lookup(&NAME##_DATA, part_number, config) ? &NAME##_TYPE : NULL; \
    } \
    const ChipDefinition NAME##_DEFINITION = { \
        ID, FUNCTION, DESCRIPTION, "Logic", \
        NAME##_NAMES, GT_COUNT(NAME##_NAMES), \
        PINS, GT_COUNT(PINS), \
        NAME##_lookup, GATES, NAME##_FLAGS \
    };

#include "gates_defs.inc"
