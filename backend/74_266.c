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

typedef struct {
    const char *prefix;
    LogicFamily family;
} FamilyPrefix;

typedef struct {
    const char *suffix;
    PackageType package;
} PackageSuffix;

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

static int name_matches(const char *name, const char *part_number) {
    size_t length;

    if (strcmp(name, part_number) == 0) {
        return 1;
    }

    length = strlen(part_number);
    return strncmp(name, part_number, length) == 0 &&
        strcmp(name + length, ".A") == 0;
}

static const char *family_suffix(const char *name, LogicFamily *family) {
    static const FamilyPrefix families[] = {
        {"SN74HC266", LOGIC_FAMILY_HC},
        {"SN74LS266", LOGIC_FAMILY_LS},
        {"SN54LS266", LOGIC_FAMILY_LS},
        {"SNJ54LS266", LOGIC_FAMILY_LS}
    };
    size_t i;

    for (i = 0; i < sizeof(families) / sizeof(families[0]); i++) {
        size_t length = strlen(families[i].prefix);

        if (strncmp(name, families[i].prefix, length) == 0) {
            *family = families[i].family;
            return name + length;
        }
    }

    return NULL;
}

static int package_from_suffix(const char *suffix, PackageType *package) {
    static const PackageSuffix packages[] = {
        {"J", PACKAGE_TYPE_CDIP},
        {"W", PACKAGE_TYPE_CFP},
        {"DRG4", PACKAGE_TYPE_SOIC},
        {"DRE4", PACKAGE_TYPE_SOIC},
        {"DRG3", PACKAGE_TYPE_SOIC},
        {"DR", PACKAGE_TYPE_SOIC},
        {"DT", PACKAGE_TYPE_SOIC},
        {"D", PACKAGE_TYPE_SOIC},
        {"NSR", PACKAGE_TYPE_SOP},
        {"NS", PACKAGE_TYPE_SOP},
        {"NE4", PACKAGE_TYPE_PDIP},
        {"N", PACKAGE_TYPE_PDIP}
    };
    size_t i;

    for (i = 0; i < sizeof(packages) / sizeof(packages[0]); i++) {
        if (name_matches(suffix, packages[i].suffix)) {
            *package = packages[i].package;
            return 1;
        }
    }

    return 0;
}

static int lookup(const char *name, Sn74_266Config *config) {
    LogicFamily family;
    PackageType package;
    const char *suffix;

    suffix = family_suffix(name, &family);
    if (suffix == NULL || !package_from_suffix(suffix, &package)) {
        return 0;
    }

    *config = (Sn74_266Config){
        .package = package,
        .family = family
    };
    return 1;
}

static const PackageInfo *package_info(PackageType package) {
    switch (package) {
        case PACKAGE_TYPE_SOIC:
        case PACKAGE_TYPE_SOP:
        case PACKAGE_TYPE_PDIP:
        case PACKAGE_TYPE_CDIP:
        case PACKAGE_TYPE_CFP:
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

        default:
            unsupported();
            return (FamilyTiming){0, 0};
    }
}

static void validate_config(const Sn74_266Config *config) {
    if (config == NULL) {
        unsupported();
        return;
    }

    switch (config->family) {
        case LOGIC_FAMILY_HC:
            switch (config->package) {
                case PACKAGE_TYPE_SOIC:
                case PACKAGE_TYPE_SOP:
                case PACKAGE_TYPE_PDIP:
                    (void)package_info(config->package);
                    break;

                default:
                    unsupported();
            }
            break;

        case LOGIC_FAMILY_LS:
            switch (config->package) {
                case PACKAGE_TYPE_SOIC:
                case PACKAGE_TYPE_PDIP:
                case PACKAGE_TYPE_CDIP:
                case PACKAGE_TYPE_CFP:
                    (void)package_info(config->package);
                    break;

                default:
                    unsupported();
            }
            break;

        default:
            unsupported();
    }
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

static const char *const SN74_266_PART_NUMBERS[] = {
    "SN74HC266N", "SN74HC266D",
    "SN74LS266N", "SN74LS266D",
};

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
