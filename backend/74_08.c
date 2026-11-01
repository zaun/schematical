#include "74_08.h"

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
} Sn74_08State;

static const GatePins gates_14[] = {
    {0, 1, 2},
    {3, 4, 5},
    {8, 9, 7},
    {11, 12, 10},
};

static const GatePins gates_20[] = {
    {1, 2, 3},
    {5, 7, 8},
    {12, 13, 11},
    {17, 18, 15},
};

static const PackageInfo package_14 = {14, gates_14};
static const PackageInfo package_20 = {20, gates_20};

static void unsupported(void) {
    assert(0 && "unsupported SN74_08 configuration");
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
#define PART_FK(name, family, package) {name, family, package},
static const PartEntry PARTS[] = {
#include "74_08_parts.inc"
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

static int lookup(const char *name, Sn74_08Config *config) {
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
        case LOGIC_FAMILY_F:
            /* Typical at 5 V and 25 C. */
            return (FamilyTiming){3800, 3600};

        case LOGIC_FAMILY_S:
            /* Typical at 5 V and 25 C.(15 pF load) */
            return (FamilyTiming){4500, 5000};
        
        case LOGIC_FAMILY_AHCT:
            /* Typical at 5 V and 25 C (15 pF load). */
            return (FamilyTiming){5000, 5000};

        case LOGIC_FAMILY_AC:
            /* Typical at 5 V and 25 C. */
            return (FamilyTiming){5500, 5500};

        case LOGIC_FAMILY_ACT:
            /* Typical at 5 V and 25 C. */
            return (FamilyTiming){6500, 6500};

        case LOGIC_FAMILY_LS:
            /* Typical at 5 V and 25 C. (15 pF load) */
            return (FamilyTiming){8000, 10000};

        case LOGIC_FAMILY_HC:
            /* Typical at 4.5 V and 25 C. */
            return (FamilyTiming){10000, 10000};

        case LOGIC_FAMILY_HCT:
            /* Typical at 4.5 V and 25 C. */
            return (FamilyTiming){15000, 15000};

        case LOGIC_FAMILY_ALS:
            /* Maximum at 5 V over the recommended temperature range. */
            return (FamilyTiming){14000, 10000};

        case LOGIC_FAMILY_AS:
            /* Maximum at 5 V over the recommended temperature range. */
            return (FamilyTiming){5500, 5500};

        case LOGIC_FAMILY_AHC:
            /* Typical at 5 V and 25 C (50 pF load), SN74AHC08. */
            return (FamilyTiming){5800, 5800};

        case LOGIC_FAMILY_ALVC:
            /* Max at 3.3 V, -40 to 85 C (SN74ALVC08 gives no typical). */
            return (FamilyTiming){2900, 2900};

        case LOGIC_FAMILY_AUC:
            /* Typical at 1.8 V and 25 C (15 pF load), SN74AUC08. */
            return (FamilyTiming){1000, 1000};

        case LOGIC_FAMILY_HCS:
            /* Typical at 4.5 V and 25 C, SN74HCS08. */
            return (FamilyTiming){7000, 7000};

        case LOGIC_FAMILY_LV:
            /* Typical at 3.3 V and 25 C (50 pF load), SN74LV08A. */
            return (FamilyTiming){7500, 7500};

        case LOGIC_FAMILY_LVC:
            /* Typical at 3.3 V and 25 C (50 pF load), SN74LVC08A. */
            return (FamilyTiming){2600, 2600};

        default:
            unsupported();
            return (FamilyTiming){0, 0};
    }
}

static void validate_config(const Sn74_08Config *config) {
    /* Only family and package pairs that some orderable part uses are valid. */
    if (config == NULL || !part_exists(config->family, config->package)) {
        unsupported();
        return;
    }
    (void)package_info(config->package);
    (void)family_timing(config->family);
}

static size_t pin_count(const void *config) {
    const Sn74_08Config *chip = config;

    validate_config(chip);
    return package_info(chip->package)->pin_count;
}

static void *create(const void *config) {
    const Sn74_08Config *chip = config;
    Sn74_08State *state;

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

static LogicValue and_value(LogicValue a, LogicValue b) {
    if (a == LOGIC_0 || b == LOGIC_0) {
        return LOGIC_0;
    }

    if (a == LOGIC_1 && b == LOGIC_1) {
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
    const Sn74_08State *state,
    const GatePins *gate
) {
    LogicValue a = sim_read_pin(sim, instance, gate->input_a);
    LogicValue b = sim_read_pin(sim, instance, gate->input_b);
    LogicValue output = and_value(a, b);

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
    const Sn74_08State *chip = state;
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
    const Sn74_08State *chip = state;
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

const SimComponentType SN74_08_TYPE = {
    "SN74_08",
    pin_count,
    create,
    destroy,
    start,
    pin_changed,
    NULL
};

const SimComponentType *sn74_08_lookup(
    const char *part_number,
    Sn74_08Config *config
) {
    if (part_number == NULL || config == NULL) {
        return NULL;
    }

    return lookup(part_number, config) ? &SN74_08_TYPE : NULL;
}

/* ---- Catalog definition ------------------------------------------------- */

/* 14-pin packages. The 20-pin LCCC parts have their own definition below. */
#define PART(name, family, package) name,
#define PART_FK(name, family, package)
static const char *const SN74_08_PART_NUMBERS[] = {
#include "74_08_parts.inc"
};
#undef PART
#undef PART_FK

static const ChipPin SN74_08_PINS[] = {
    {1, "1A", PIN_ROLE_INPUT},
    {2, "1B", PIN_ROLE_INPUT},
    {3, "1Y", PIN_ROLE_OUTPUT},
    {4, "2A", PIN_ROLE_INPUT},
    {5, "2B", PIN_ROLE_INPUT},
    {6, "2Y", PIN_ROLE_OUTPUT},
    {7, "GND", PIN_ROLE_GROUND},
    {8, "3Y", PIN_ROLE_OUTPUT},
    {9, "3A", PIN_ROLE_INPUT},
    {10, "3B", PIN_ROLE_INPUT},
    {11, "4Y", PIN_ROLE_OUTPUT},
    {12, "4A", PIN_ROLE_INPUT},
    {13, "4B", PIN_ROLE_INPUT},
    {14, "VCC", PIN_ROLE_POWER},
};

const ChipDefinition SN74_08_DEFINITION = {
    "74_08",
    "AND",
    "Quad 2-input AND gate",
    "Logic",
    SN74_08_PART_NUMBERS,
    sizeof(SN74_08_PART_NUMBERS) / sizeof(SN74_08_PART_NUMBERS[0]),
    SN74_08_PINS,
    sizeof(SN74_08_PINS) / sizeof(SN74_08_PINS[0]),
    sn74_08_lookup,
    4,
};

/* ---- 20-pin LCCC (FK) definition --------------------------------------- */

#define PART(name, family, package)
#define PART_FK(name, family, package) name,
static const char *const SN74_08_FK_PART_NUMBERS[] = {
#include "74_08_parts.inc"
};
#undef PART
#undef PART_FK

/* MODE_2 pinout from 74_08.h. */
static const ChipPin SN74_08_FK_PINS[] = {
    {1, "NC", PIN_ROLE_NO_CONNECT},
    {2, "1A", PIN_ROLE_INPUT},
    {3, "1B", PIN_ROLE_INPUT},
    {4, "1Y", PIN_ROLE_OUTPUT},
    {5, "NC", PIN_ROLE_NO_CONNECT},
    {6, "2A", PIN_ROLE_INPUT},
    {7, "NC", PIN_ROLE_NO_CONNECT},
    {8, "2B", PIN_ROLE_INPUT},
    {9, "2Y", PIN_ROLE_OUTPUT},
    {10, "GND", PIN_ROLE_GROUND},
    {11, "NC", PIN_ROLE_NO_CONNECT},
    {12, "3Y", PIN_ROLE_OUTPUT},
    {13, "3A", PIN_ROLE_INPUT},
    {14, "3B", PIN_ROLE_INPUT},
    {15, "NC", PIN_ROLE_NO_CONNECT},
    {16, "4Y", PIN_ROLE_OUTPUT},
    {17, "NC", PIN_ROLE_NO_CONNECT},
    {18, "4A", PIN_ROLE_INPUT},
    {19, "4B", PIN_ROLE_INPUT},
    {20, "VCC", PIN_ROLE_POWER},
};

const ChipDefinition SN74_08_FK_DEFINITION = {
    "74_08_FK",
    "AND",
    "Quad 2-input AND gate, 20-pin LCCC (FK)",
    "Logic",
    SN74_08_FK_PART_NUMBERS,
    sizeof(SN74_08_FK_PART_NUMBERS) / sizeof(SN74_08_FK_PART_NUMBERS[0]),
    SN74_08_FK_PINS,
    sizeof(SN74_08_FK_PINS) / sizeof(SN74_08_FK_PINS[0]),
    sn74_08_lookup,
    4,
};
