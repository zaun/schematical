#include "74_02.h"

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
} Sn74_02State;

/* NOR gate outputs are on pins 1, 4, 10 and 13 in the 14-pin layout. */
static const GatePins gates_14[] = {
    {1, 2, 0},
    {4, 5, 3},
    {7, 8, 9},
    {10, 11, 12},
};

/* 20-pin (LCCC) layout from the FK pin table. */
static const GatePins gates_20[] = {
    {2, 3, 1},
    {7, 8, 5},
    {11, 12, 13},
    {15, 17, 18},
};

static const PackageInfo package_14 = {14, gates_14};
static const PackageInfo package_20 = {20, gates_20};

static void unsupported(void) {
    assert(0 && "unsupported SN74_02 configuration");
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

#define PART(name, family, package, flags) {name, family, package},
#define PART_FK(name, family, package, flags) {name, family, package},
static const PartEntry PARTS[] = {
#include "74_02_parts.inc"
};
#undef PART
#undef PART_FK

static int part_exists(LogicFamily family, PackageType package) {
    size_t i;

    for (i = 0; i < sizeof(PARTS) / sizeof(PARTS[0]); i++) {
        if (PARTS[i].family == family && PARTS[i].package == package) {
            return 1;
        }
    }
    return 0;
}

static int lookup(const char *name, Sn74_02Config *config) {
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
        case PACKAGE_TYPE_LCCC:
            return &package_20;

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
        case LOGIC_FAMILY_AC:
            /* Typical at 5 V and 25 C (50 pF load). */
            return (FamilyTiming){5400, 5100};

        case LOGIC_FAMILY_ACT:
            /* Typical at 5 V and 25 C (50 pF load). */
            return (FamilyTiming){5800, 4200};

        case LOGIC_FAMILY_AHC:
            /* Typical at 5 V and 25 C (50 pF load). */
            return (FamilyTiming){5100, 5100};

        case LOGIC_FAMILY_AHCT:
            /* Typical at 5 V and 25 C (50 pF load). */
            return (FamilyTiming){3400, 4500};

        case LOGIC_FAMILY_ALS:
            /* Maximum at 5 V over the recommended temperature range. */
            return (FamilyTiming){12000, 10000};

        case LOGIC_FAMILY_AS:
            /* Maximum at 5 V over the recommended temperature range. */
            return (FamilyTiming){4500, 4500};

        case LOGIC_FAMILY_F:
            /* Typical at 5 V and 25 C (50 pF load). */
            return (FamilyTiming){4000, 2800};

        case LOGIC_FAMILY_HC:
            /* Typical at 4.5 V and 25 C (50 pF load). */
            return (FamilyTiming){9000, 9000};

        case LOGIC_FAMILY_HCT:
            /* Typical at 4.5 V and 25 C (50 pF load). */
            return (FamilyTiming){11000, 11000};

        case LOGIC_FAMILY_LS:
            /* Typical at 5 V and 25 C (15 pF load). */
            return (FamilyTiming){10000, 10000};

        case LOGIC_FAMILY_AUC:
            /* Typical at 1.8 V and 25 C (15 pF load), SN74AUC02. */
            return (FamilyTiming){900, 900};

        case LOGIC_FAMILY_HCS:
            /* Typical at 4.5 V and 25 C, SN74HCS02. */
            return (FamilyTiming){7000, 7000};

        case LOGIC_FAMILY_LV:
            /* Typical at 3.3 V and 25 C (50 pF load), SN74LV02A. */
            return (FamilyTiming){7600, 7600};

        case LOGIC_FAMILY_LVC:
            /* Typical at 3.3 V and 25 C (50 pF load), SN74LVC02A. */
            return (FamilyTiming){3600, 3600};

        case LOGIC_FAMILY_S:
            /* Typical at 5 V and 25 C (15 pF load), SN74S02. */
            return (FamilyTiming){3500, 3500};

        default:
            unsupported();
            return (FamilyTiming){0, 0};
    }
}

static void validate_config(const Sn74_02Config *config) {
    /* Only family and package pairs that some orderable part uses are valid. */
    if (config == NULL || !part_exists(config->family, config->package)) {
        unsupported();
        return;
    }
    (void)package_info(config->package);
    (void)family_timing(config->family);
}

static size_t pin_count(const void *config) {
    const Sn74_02Config *chip = config;

    validate_config(chip);
    return package_info(chip->package)->pin_count;
}

static void *create(const void *config) {
    const Sn74_02Config *chip = config;
    Sn74_02State *state;

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

static LogicValue nor_value(LogicValue a, LogicValue b) {
    if (a == LOGIC_1 || b == LOGIC_1) {
        return LOGIC_0;
    }

    if (a == LOGIC_0 && b == LOGIC_0) {
        return LOGIC_1;
    }

    return LOGIC_X;
}

static SimTime output_delay(
    const FamilyTiming *timing,
    LogicValue value
) {
    if (value == LOGIC_1) {
        return timing->rise_delay;
    }

    if (value == LOGIC_0) {
        return timing->fall_delay;
    }

    /* Use the longer delay for an unknown output. */
    return timing->rise_delay > timing->fall_delay
        ? timing->rise_delay
        : timing->fall_delay;
}

static void evaluate_gate(
    Sim *sim,
    SimInstance *instance,
    const Sn74_02State *state,
    const GatePins *gate
) {
    LogicValue a = sim_read_pin(sim, instance, gate->input_a);
    LogicValue b = sim_read_pin(sim, instance, gate->input_b);
    LogicValue output = nor_value(a, b);

    sim_schedule_inertial_drive(
        sim,
        instance,
        gate->output,
        output,
        DRIVE_STRONG,
        output_delay(&state->timing, output)
    );
}

static void start(Sim *sim, SimInstance *instance, void *state) {
    const Sn74_02State *chip = state;
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
    const Sn74_02State *chip = state;
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

const SimComponentType SN74_02_TYPE = {
    "SN74_02",
    pin_count,
    create,
    destroy,
    start,
    pin_changed,
    NULL
};

const SimComponentType *sn74_02_lookup(
    const char *part_number,
    Sn74_02Config *config
) {
    if (part_number == NULL || config == NULL) {
        return NULL;
    }

    return lookup(part_number, config) ? &SN74_02_TYPE : NULL;
}

/* ---- Catalog definition ------------------------------------------------- */

/* 14-pin packages. The 20-pin LCCC parts have their own definition below. */
#define PART(name, family, package, flags) name,
#define PART_FK(name, family, package, flags)
static const char *const SN74_02_PART_NUMBERS[] = {
#include "74_02_parts.inc"
};
#undef PART
#undef PART_FK
#define PART(name, family, package, flags) flags,
#define PART_FK(name, family, package, flags)
static const unsigned SN74_02_PART_FLAGS[] = {
#include "74_02_parts.inc"
};
#undef PART
#undef PART_FK

static const ChipPin SN74_02_PINS[] = {
    {1, "1Y", PIN_ROLE_OUTPUT},
    {2, "1A", PIN_ROLE_INPUT},
    {3, "1B", PIN_ROLE_INPUT},
    {4, "2Y", PIN_ROLE_OUTPUT},
    {5, "2A", PIN_ROLE_INPUT},
    {6, "2B", PIN_ROLE_INPUT},
    {7, "GND", PIN_ROLE_GROUND},
    {8, "3A", PIN_ROLE_INPUT},
    {9, "3B", PIN_ROLE_INPUT},
    {10, "3Y", PIN_ROLE_OUTPUT},
    {11, "4A", PIN_ROLE_INPUT},
    {12, "4B", PIN_ROLE_INPUT},
    {13, "4Y", PIN_ROLE_OUTPUT},
    {14, "VCC", PIN_ROLE_POWER},
};

const ChipDefinition SN74_02_DEFINITION = {
    "74_02",
    "NOR",
    "Quad 2-input NOR gate",
    "Logic",
    SN74_02_PART_NUMBERS,
    sizeof(SN74_02_PART_NUMBERS) / sizeof(SN74_02_PART_NUMBERS[0]),
    SN74_02_PINS,
    sizeof(SN74_02_PINS) / sizeof(SN74_02_PINS[0]),
    sn74_02_lookup,
    4,
    SN74_02_PART_FLAGS,
};

/* ---- 20-pin LCCC (FK) definition --------------------------------------- */

#define PART(name, family, package, flags)
#define PART_FK(name, family, package, flags) name,
static const char *const SN74_02_FK_PART_NUMBERS[] = {
#include "74_02_parts.inc"
};
#undef PART
#undef PART_FK
#define PART(name, family, package, flags)
#define PART_FK(name, family, package, flags) flags,
static const unsigned SN74_02_FK_PART_FLAGS[] = {
#include "74_02_parts.inc"
};
#undef PART
#undef PART_FK

/* MODE_2 pinout from 74_02.h. */
static const ChipPin SN74_02_FK_PINS[] = {
    {1, "NC", PIN_ROLE_NO_CONNECT},
    {2, "1Y", PIN_ROLE_OUTPUT},
    {3, "1A", PIN_ROLE_INPUT},
    {4, "1B", PIN_ROLE_INPUT},
    {5, "NC", PIN_ROLE_NO_CONNECT},
    {6, "2Y", PIN_ROLE_OUTPUT},
    {7, "NC", PIN_ROLE_NO_CONNECT},
    {8, "2A", PIN_ROLE_INPUT},
    {9, "2B", PIN_ROLE_INPUT},
    {10, "GND", PIN_ROLE_GROUND},
    {11, "NC", PIN_ROLE_NO_CONNECT},
    {12, "3A", PIN_ROLE_INPUT},
    {13, "3B", PIN_ROLE_INPUT},
    {14, "3Y", PIN_ROLE_OUTPUT},
    {15, "NC", PIN_ROLE_NO_CONNECT},
    {16, "4A", PIN_ROLE_INPUT},
    {17, "NC", PIN_ROLE_NO_CONNECT},
    {18, "4B", PIN_ROLE_INPUT},
    {19, "4Y", PIN_ROLE_OUTPUT},
    {20, "VCC", PIN_ROLE_POWER},
};

const ChipDefinition SN74_02_FK_DEFINITION = {
    "74_02_FK",
    "NOR",
    "Quad 2-input NOR gate, 20-pin LCCC (FK)",
    "Logic",
    SN74_02_FK_PART_NUMBERS,
    sizeof(SN74_02_FK_PART_NUMBERS) / sizeof(SN74_02_FK_PART_NUMBERS[0]),
    SN74_02_FK_PINS,
    sizeof(SN74_02_FK_PINS) / sizeof(SN74_02_FK_PINS[0]),
    sn74_02_lookup,
    4,
    SN74_02_FK_PART_FLAGS,
};
