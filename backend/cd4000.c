#include "cd4000.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "cd4000_parts.inc"

#define CD_PIN_COUNT 14

typedef enum {
    CD_OP_NOR,
    CD_OP_NAND,
    CD_OP_XOR,
    CD_OP_XNOR,
    CD_OP_OR,
    CD_OP_AND
} CdOp;

typedef struct {
    size_t input_a;
    size_t input_b;
    size_t output;
} CdGate;

/* 0-based pin indices: J = f(A, B), K = f(C, D), L = f(E, F), M = f(G, H). */
static const CdGate GATES[4] = {
    {0, 1, 2},
    {4, 5, 3},
    {7, 8, 9},
    {11, 12, 10},
};

static const ChipPin CD_PINS[CD_PIN_COUNT] = {
    {1, "A", PIN_ROLE_INPUT},
    {2, "B", PIN_ROLE_INPUT},
    {3, "J", PIN_ROLE_OUTPUT},
    {4, "K", PIN_ROLE_OUTPUT},
    {5, "C", PIN_ROLE_INPUT},
    {6, "D", PIN_ROLE_INPUT},
    {7, "VSS", PIN_ROLE_GROUND},
    {8, "E", PIN_ROLE_INPUT},
    {9, "F", PIN_ROLE_INPUT},
    {10, "L", PIN_ROLE_OUTPUT},
    {11, "M", PIN_ROLE_OUTPUT},
    {12, "G", PIN_ROLE_INPUT},
    {13, "H", PIN_ROLE_INPUT},
    {14, "VDD", PIN_ROLE_POWER},
};

typedef struct {
    const char *name;
    PackageType package;
} CdPart;

typedef struct {
    CdOp op;
    SimTime delay;   /* tPHL = tPLH, picoseconds */
    const CdPart *parts;
    size_t part_count;
} CdData;

static void unsupported(void) {
    assert(0 && "unsupported CD4000 configuration");
    abort();
}

static int part_exists(const CdData *data, PackageType package) {
    size_t i;

    for (i = 0; i < data->part_count; i++) {
        if (data->parts[i].package == package) {
            return 1;
        }
    }
    return 0;
}

static int lookup(const CdData *data, const char *name, SimLogicConfig *config) {
    size_t i;

    for (i = 0; i < data->part_count; i++) {
        if (strcmp(data->parts[i].name, name) == 0) {
            config->family = LOGIC_FAMILY_C;
            config->package = data->parts[i].package;
            return 1;
        }
    }
    return 0;
}

static void validate(const CdData *data, const void *raw) {
    const SimLogicConfig *config = raw;

    if (config == NULL || config->family != LOGIC_FAMILY_C ||
        !part_exists(data, config->package)) {
        unsupported();
    }
}

static int known(LogicValue v) {
    return v == LOGIC_0 || v == LOGIC_1;
}

static LogicValue gate_value(CdOp op, LogicValue a, LogicValue b) {
    switch (op) {
        case CD_OP_NAND:
            if (a == LOGIC_0 || b == LOGIC_0) return LOGIC_1;
            return a == LOGIC_1 && b == LOGIC_1 ? LOGIC_0 : LOGIC_X;
        case CD_OP_AND:
            if (a == LOGIC_0 || b == LOGIC_0) return LOGIC_0;
            return a == LOGIC_1 && b == LOGIC_1 ? LOGIC_1 : LOGIC_X;
        case CD_OP_NOR:
            if (a == LOGIC_1 || b == LOGIC_1) return LOGIC_0;
            return a == LOGIC_0 && b == LOGIC_0 ? LOGIC_1 : LOGIC_X;
        case CD_OP_OR:
            if (a == LOGIC_1 || b == LOGIC_1) return LOGIC_1;
            return a == LOGIC_0 && b == LOGIC_0 ? LOGIC_0 : LOGIC_X;
        case CD_OP_XOR:
            if (!known(a) || !known(b)) return LOGIC_X;
            return a != b ? LOGIC_1 : LOGIC_0;
        case CD_OP_XNOR:
            if (!known(a) || !known(b)) return LOGIC_X;
            return a == b ? LOGIC_1 : LOGIC_0;
    }
    return LOGIC_X;
}

typedef struct {
    const CdData *data;
} CdState;

static void *cd_create(const CdData *data, const void *config) {
    CdState *state;

    validate(data, config);
    state = malloc(sizeof(*state));
    if (state != NULL) {
        state->data = data;
    }
    return state;
}

static void cd_destroy(void *state) {
    free(state);
}

static void evaluate(Sim *sim, SimInstance *instance, const CdState *state, const CdGate *gate) {
    LogicValue a = sim_read_pin(sim, instance, gate->input_a);
    LogicValue b = sim_read_pin(sim, instance, gate->input_b);

    sim_schedule_inertial_drive(sim, instance, gate->output,
                                gate_value(state->data->op, a, b),
                                DRIVE_STRONG, state->data->delay);
}

static void cd_start(Sim *sim, SimInstance *instance, void *raw) {
    const CdState *state = raw;
    size_t i;

    for (i = 0; i < 4; i++) {
        evaluate(sim, instance, state, &GATES[i]);
    }
}

static void cd_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    const CdState *state = raw;
    size_t i;

    (void)old_value;
    (void)new_value;
    for (i = 0; i < 4; i++) {
        if (pin == GATES[i].input_a || pin == GATES[i].input_b) {
            evaluate(sim, instance, state, &GATES[i]);
            return;
        }
    }
}

/*
 * Each definition gets its own SimComponentType, because the model callbacks
 * receive only the configuration and need the gate data from their definition.
 */
#define CD_ENTRY(name, package, flags) {name, package},
#define CD_NAME(name, package, flags) name,
#define CD_FLAGS(name, package, flags) flags,

#define CD_DEFINE(NAME, ID, FUNCTION, DESCRIPTION, OP, DELAY_PS)                 \
    static const CdPart NAME##_PARTS[] = { NAME##_PARTS_LIST(CD_ENTRY) };         \
    static const char *const NAME##_NAMES[] = { NAME##_PARTS_LIST(CD_NAME) };     \
    static const unsigned NAME##_FLAGS[] = { NAME##_PARTS_LIST(CD_FLAGS) };       \
    static const CdData NAME##_DATA = {                                           \
        OP, DELAY_PS, NAME##_PARTS, sizeof(NAME##_PARTS) / sizeof(NAME##_PARTS[0]) \
    };                                                                            \
    static size_t NAME##_pin_count(const void *config) {                          \
        validate(&NAME##_DATA, config);                                           \
        return CD_PIN_COUNT;                                                      \
    }                                                                             \
    static void *NAME##_create(const void *config) {                              \
        return cd_create(&NAME##_DATA, config);                                   \
    }                                                                             \
    static const SimComponentType NAME##_TYPE = {                                 \
        ID, NAME##_pin_count, NAME##_create, cd_destroy,                          \
        cd_start, cd_pin_changed, NULL                                            \
    };                                                                            \
    static const SimComponentType *NAME##_lookup(                                 \
        const char *part_number, SimLogicConfig *config                           \
    ) {                                                                           \
        if (part_number == NULL || config == NULL) {                              \
            return NULL;                                                          \
        }                                                                         \
        return lookup(&NAME##_DATA, part_number, config) ? &NAME##_TYPE : NULL;   \
    }                                                                             \
    const ChipDefinition NAME##_DEFINITION = {                                    \
        ID, FUNCTION, DESCRIPTION, "Logic",                                       \
        NAME##_NAMES, sizeof(NAME##_NAMES) / sizeof(NAME##_NAMES[0]),             \
        CD_PINS, CD_PIN_COUNT, NAME##_lookup, 4, NAME##_FLAGS                     \
    };

#define CD4001B_PARTS_LIST CD4001B_PARTS
#define CD4011B_PARTS_LIST CD4011B_PARTS
#define CD4030B_PARTS_LIST CD4030B_PARTS
#define CD4070B_PARTS_LIST CD4070B_PARTS
#define CD4071B_PARTS_LIST CD4071B_PARTS
#define CD4077B_PARTS_LIST CD4077B_PARTS
#define CD4081B_PARTS_LIST CD4081B_PARTS
#define CD4093B_PARTS_LIST CD4093B_PARTS

CD_DEFINE(CD4001B, "4001B", "NOR", "CMOS quad 2-input NOR gate", CD_OP_NOR, 125000)
CD_DEFINE(CD4011B, "4011B", "NAND", "CMOS quad 2-input NAND gate", CD_OP_NAND, 125000)
CD_DEFINE(CD4030B, "4030B", "XOR", "CMOS quad 2-input XOR gate", CD_OP_XOR, 140000)
CD_DEFINE(CD4070B, "4070B", "XOR", "CMOS quad 2-input XOR gate", CD_OP_XOR, 140000)
CD_DEFINE(CD4071B, "4071B", "OR", "CMOS quad 2-input OR gate", CD_OP_OR, 125000)
CD_DEFINE(CD4077B, "4077B", "XNOR", "CMOS quad 2-input XNOR gate", CD_OP_XNOR, 140000)
CD_DEFINE(CD4081B, "4081B", "AND", "CMOS quad 2-input AND gate", CD_OP_AND, 125000)
CD_DEFINE(CD4093B, "4093B", "NAND", "CMOS quad 2-input NAND Schmitt trigger", CD_OP_NAND, 190000)
