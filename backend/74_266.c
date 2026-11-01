#include "74_266.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    size_t input_a;
    size_t input_b;
    size_t output;
} GatePins;

typedef struct {
    size_t pin_count;
    const GatePins *gates;
} PackageInfo;

typedef struct {
    SimTime rise_delay;
    SimTime fall_delay;
} FamilyTiming;

typedef struct {
    const GatePins *gates;
    FamilyTiming timing;
} Sn74_266State;

/* 14-pin layout from the SN74HC266 pin table. */
static const GatePins gates_14[] = {
    {0, 1, 2},
    {4, 5, 3},
    {7, 8, 9},
    {11, 12, 10},
};

static const PackageInfo package_14 = {14, gates_14};

static void unsupported(void) {
    assert(0 && "unsupported SN74_266 configuration");
    abort();
}

/*
 * Orderable part numbers from the datasheets, one entry per name. Lookup is by
 * exact name; there is no suffix decoding and no ".A" variant.
 */
typedef struct {
    const char *name;
    LogicFamily family;
    PackageType package;
} PartEntry;

#define PART(name, family, package) {name, family, package},
static const PartEntry PARTS[] = {
#include "74_266_parts.inc"
};
#undef PART

static int part_exists(LogicFamily family, PackageType package) {
    size_t i;

    for (i = 0; i < sizeof(PARTS) / sizeof(PARTS[0]); i++) {
        if (PARTS[i].family == family && PARTS[i].package == package) {
            return 1;
        }
    }
    return 0;
}

static int lookup(const char *name, Sn74_266Config *config) {
    size_t i;

    for (i = 0; i < sizeof(PARTS) / sizeof(PARTS[0]); i++) {
        if (strcmp(PARTS[i].name, name) == 0) {
            config->family = PARTS[i].family;
            config->package = PARTS[i].package;
            return 1;
        }
    }
    return 0;
}

static const PackageInfo *package_info(PackageType package) {
    switch (package) {
        case PACKAGE_TYPE_SOIC:
        case PACKAGE_TYPE_SOP:
        case PACKAGE_TYPE_SSOP:
        case PACKAGE_TYPE_DIP:
        case PACKAGE_TYPE_TSSOP:
        case PACKAGE_TYPE_TVSOP:
        case PACKAGE_TYPE_CDIP:
        case PACKAGE_TYPE_CFP:
        case PACKAGE_TYPE_WQFN:
        case PACKAGE_TYPE_VQFN:
            return &package_14;

        default:
            unsupported();
            return NULL;
    }
}

static FamilyTiming family_timing(LogicFamily family) {
    switch (family) {
        case LOGIC_FAMILY_HC:
            /* Typical at 4.5 V and 25 C (50 pF load). */
            return (FamilyTiming){13000, 13000};

        case LOGIC_FAMILY_LS:
            /* Typical at 5 V and 25 C (15 pF load). */
            return (FamilyTiming){18000, 18000};

        case LOGIC_FAMILY_HCS:
            /* Typical at 4.5 V and 25 C (50 pF load), SN74HCS266. */
            return (FamilyTiming){6000, 6000};

        default:
            unsupported();
            return (FamilyTiming){0, 0};
    }
}

static void validate_config(const Sn74_266Config *config) {
    /* Only family and package pairs that some orderable part uses are valid. */
    if (config == NULL || !part_exists(config->family, config->package)) {
        unsupported();
        return;
    }
    (void)package_info(config->package);
    (void)family_timing(config->family);
}

static size_t pin_count(const void *config) {
    const Sn74_266Config *chip = config;

    validate_config(chip);
    return package_info(chip->package)->pin_count;
}

static void *create(const void *config) {
    const Sn74_266Config *chip = config;
    Sn74_266State *state;

    validate_config(chip);

    state = malloc(sizeof(*state));
    if (state == NULL) {
        return NULL;
    }

    state->gates = package_info(chip->package)->gates;
    state->timing = family_timing(chip->family);
    return state;
}

static void destroy(void *state) {
    free(state);
}

static LogicValue xnor_value(LogicValue a, LogicValue b) {
    if ((a != LOGIC_0 && a != LOGIC_1) || (b != LOGIC_0 && b != LOGIC_1)) {
        return LOGIC_X;
    }

    return a == b ? LOGIC_1 : LOGIC_0;
}

static void evaluate_gate(
    Sim *sim,
    SimInstance *instance,
    const Sn74_266State *state,
    const GatePins *gate
) {
    LogicValue a = sim_read_pin(sim, instance, gate->input_a);
    LogicValue b = sim_read_pin(sim, instance, gate->input_b);
    LogicValue xnor = xnor_value(a, b);
    LogicValue output;
    SimTime delay;

    /*
     * Open-drain output: a logic 1 releases the pin (Z) and the external
     * pull-up takes it high, so the rising edge uses the release delay.
     */
    if (xnor == LOGIC_1) {
        output = LOGIC_Z;
        delay = state->timing.rise_delay;
    } else if (xnor == LOGIC_0) {
        output = LOGIC_0;
        delay = state->timing.fall_delay;
    } else {
        output = LOGIC_X;
        delay = state->timing.rise_delay > state->timing.fall_delay
            ? state->timing.rise_delay
            : state->timing.fall_delay;
    }

    sim_schedule_inertial_drive(
        sim,
        instance,
        gate->output,
        output,
        DRIVE_STRONG,
        delay
    );
}

static void start(Sim *sim, SimInstance *instance, void *state) {
    const Sn74_266State *chip = state;
    size_t i;

    for (i = 0; i < 4; i++) {
        evaluate_gate(sim, instance, chip, &chip->gates[i]);
    }
}

static void pin_changed(
    Sim *sim,
    SimInstance *instance,
    void *state,
    size_t pin,
    LogicValue old_value,
    LogicValue new_value
) {
    const Sn74_266State *chip = state;
    size_t i;

    (void)old_value;
    (void)new_value;

    for (i = 0; i < 4; i++) {
        const GatePins *gate = &chip->gates[i];

        if (pin == gate->input_a || pin == gate->input_b) {
            evaluate_gate(sim, instance, chip, gate);
            return;
        }
    }
}

const SimComponentType SN74_266_TYPE = {
    "SN74_266",
    pin_count,
    create,
    destroy,
    start,
    pin_changed,
    NULL
};

const SimComponentType *sn74_266_lookup(
    const char *part_number,
    Sn74_266Config *config
) {
    if (part_number == NULL || config == NULL) {
        return NULL;
    }

    return lookup(part_number, config) ? &SN74_266_TYPE : NULL;
}

/* ---- Catalog definition ------------------------------------------------- */

#define PART(name, family, package) name,
static const char *const SN74_266_PART_NUMBERS[] = {
#include "74_266_parts.inc"
};
#undef PART

static const ChipPin SN74_266_PINS[] = {
    {1, "1A", PIN_ROLE_INPUT},
    {2, "1B", PIN_ROLE_INPUT},
    {3, "1Y", PIN_ROLE_OUTPUT},
    {4, "2Y", PIN_ROLE_OUTPUT},
    {5, "2A", PIN_ROLE_INPUT},
    {6, "2B", PIN_ROLE_INPUT},
    {7, "GND", PIN_ROLE_GROUND},
    {8, "3A", PIN_ROLE_INPUT},
    {9, "3B", PIN_ROLE_INPUT},
    {10, "3Y", PIN_ROLE_OUTPUT},
    {11, "4Y", PIN_ROLE_OUTPUT},
    {12, "4A", PIN_ROLE_INPUT},
    {13, "4B", PIN_ROLE_INPUT},
    {14, "VCC", PIN_ROLE_POWER},
};

const ChipDefinition SN74_266_DEFINITION = {
    "74_266",
    "XNOR",
    "Quad 2-input XNOR gate, open-drain outputs",
    "Logic",
    SN74_266_PART_NUMBERS,
    sizeof(SN74_266_PART_NUMBERS) / sizeof(SN74_266_PART_NUMBERS[0]),
    SN74_266_PINS,
    sizeof(SN74_266_PINS) / sizeof(SN74_266_PINS[0]),
    sn74_266_lookup,
    4,
};
