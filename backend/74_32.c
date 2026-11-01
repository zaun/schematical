#include "74_32.h"

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
} Sn74_32State;

typedef struct {
    const char *prefix;
    LogicFamily family;
} FamilyPrefix;

typedef struct {
    const char *suffix;
    PackageType package;
} PackageSuffix;

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
    assert(0 && "unsupported SN74_32 configuration");
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
        {"SN74AC32", LOGIC_FAMILY_AC},
        {"SN54AC32", LOGIC_FAMILY_AC},
        {"SN74ACT32", LOGIC_FAMILY_ACT},
        {"SN74AHC32", LOGIC_FAMILY_AHC},
        {"SN54AHC32", LOGIC_FAMILY_AHC},
        {"SN74AHCT32", LOGIC_FAMILY_AHCT},
        {"SN54AHCT32", LOGIC_FAMILY_AHCT},
        {"SN74ALS32", LOGIC_FAMILY_ALS},
        {"SN54ALS32", LOGIC_FAMILY_ALS},
        {"SNJ54ALS32", LOGIC_FAMILY_ALS},
        {"SN74AS32", LOGIC_FAMILY_AS},
        {"SN54AS32", LOGIC_FAMILY_AS},
        {"SNJ54AS32", LOGIC_FAMILY_AS},
        {"SN74F32", LOGIC_FAMILY_F},
        {"SN54F32", LOGIC_FAMILY_F},
        {"SN74HC32", LOGIC_FAMILY_HC},
        {"SN54HC32", LOGIC_FAMILY_HC},
        {"SN74HCT32", LOGIC_FAMILY_HCT},
        {"SN54HCT32", LOGIC_FAMILY_HCT},
        {"SN74LS32", LOGIC_FAMILY_LS},
        {"SN54LS32", LOGIC_FAMILY_LS},
        {"SN74S32", LOGIC_FAMILY_S},
        {"SN54S32", LOGIC_FAMILY_S}
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
        {"FK", PACKAGE_TYPE_LCCC},
        {"J", PACKAGE_TYPE_CDIP},
        {"W", PACKAGE_TYPE_CFP},
        {"BQAR", PACKAGE_TYPE_WQFN},
        {"BQA", PACKAGE_TYPE_WQFN},
        {"RGYR", PACKAGE_TYPE_VQFN},
        {"RGY", PACKAGE_TYPE_VQFN},
        {"DGVR", PACKAGE_TYPE_TVSOP},
        {"DGV", PACKAGE_TYPE_TVSOP},
        {"DBR", PACKAGE_TYPE_SSOP},
        {"DB", PACKAGE_TYPE_SSOP},
        {"NSR", PACKAGE_TYPE_SOP},
        {"NS", PACKAGE_TYPE_SOP},
        {"PWRG4", PACKAGE_TYPE_TSSOP},
        {"PWR1G4", PACKAGE_TYPE_TSSOP},
        {"PWR", PACKAGE_TYPE_TSSOP},
        {"PWT", PACKAGE_TYPE_TSSOP},
        {"PW", PACKAGE_TYPE_TSSOP},
        {"DRE4", PACKAGE_TYPE_SOIC},
        {"DRG4", PACKAGE_TYPE_SOIC},
        {"DRG3", PACKAGE_TYPE_SOIC},
        {"DR", PACKAGE_TYPE_SOIC},
        {"DT", PACKAGE_TYPE_SOIC},
        {"D", PACKAGE_TYPE_SOIC},
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

static int lookup(const char *name, Sn74_32Config *config) {
    LogicFamily family;
    PackageType package;
    const char *suffix;

    suffix = family_suffix(name, &family);
    if (suffix == NULL || !package_from_suffix(suffix, &package)) {
        return 0;
    }

    *config = (Sn74_32Config){
        .package = package,
        .family = family
    };
    return 1;
}

static const PackageInfo *package_info(PackageType package) {
    switch (package) {
        case PACKAGE_TYPE_LCCC:
            return &package_20;

        case PACKAGE_TYPE_SOIC:
        case PACKAGE_TYPE_SOP:
        case PACKAGE_TYPE_SSOP:
        case PACKAGE_TYPE_PDIP:
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
            /* Typical at 5 V and 25 C. */
            return (FamilyTiming){5500, 5000};

        case LOGIC_FAMILY_ACT:
            /* Typical at 5 V and 25 C. */
            return (FamilyTiming){6500, 6500};

        case LOGIC_FAMILY_AHC:
            /* Typical at 5 V and 25 C (50 pF load). */
            return (FamilyTiming){5300, 5300};

        case LOGIC_FAMILY_AHCT:
            /* Typical at 5 V and 25 C (50 pF load). */
            return (FamilyTiming){5500, 5500};

        case LOGIC_FAMILY_ALS:
            /* Maximum at 5 V over the recommended temperature range. */
            return (FamilyTiming){14000, 12000};

        case LOGIC_FAMILY_AS:
            /* Maximum at 5 V over the recommended temperature range. */
            return (FamilyTiming){5800, 5800};

        case LOGIC_FAMILY_F:
            /* Typical at 5 V and 25 C. */
            return (FamilyTiming){3800, 3600};

        case LOGIC_FAMILY_HC:
            /* Typical at 4.5 V and 25 C (50 pF load). */
            return (FamilyTiming){10000, 10000};

        case LOGIC_FAMILY_HCT:
            /* Typical at 4.5 V and 25 C (50 pF load). */
            return (FamilyTiming){15000, 15000};

        case LOGIC_FAMILY_LS:
            /* Typical at 5 V and 25 C (15 pF load). */
            return (FamilyTiming){14000, 14000};

        case LOGIC_FAMILY_S:
            /* Typical at 5 V and 25 C (15 pF load). */
            return (FamilyTiming){4000, 4000};

        default:
            unsupported();
            return (FamilyTiming){0, 0};
    }
}

static void validate_config(const Sn74_32Config *config) {
    if (config == NULL) {
        unsupported();
        return;
    }

    switch (config->family) {
        case LOGIC_FAMILY_AC:
            switch (config->package) {
                case PACKAGE_TYPE_LCCC:
                case PACKAGE_TYPE_CDIP:
                case PACKAGE_TYPE_CFP:
                case PACKAGE_TYPE_SOIC:
                case PACKAGE_TYPE_SSOP:
                case PACKAGE_TYPE_TVSOP:
                case PACKAGE_TYPE_PDIP:
                case PACKAGE_TYPE_SOP:
                case PACKAGE_TYPE_TSSOP:
                    (void)package_info(config->package);
                    break;

                default:
                    unsupported();
            }
            break;

        case LOGIC_FAMILY_ACT:
            switch (config->package) {
                case PACKAGE_TYPE_WQFN:
                case PACKAGE_TYPE_SOIC:
                case PACKAGE_TYPE_SSOP:
                case PACKAGE_TYPE_TVSOP:
                case PACKAGE_TYPE_PDIP:
                case PACKAGE_TYPE_SOP:
                case PACKAGE_TYPE_TSSOP:
                case PACKAGE_TYPE_VQFN:
                    (void)package_info(config->package);
                    break;

                default:
                    unsupported();
            }
            break;

        case LOGIC_FAMILY_AHC:
        case LOGIC_FAMILY_AHCT:
            switch (config->package) {
                case PACKAGE_TYPE_LCCC:
                case PACKAGE_TYPE_CDIP:
                case PACKAGE_TYPE_CFP:
                case PACKAGE_TYPE_WQFN:
                case PACKAGE_TYPE_SOIC:
                case PACKAGE_TYPE_SSOP:
                case PACKAGE_TYPE_TVSOP:
                case PACKAGE_TYPE_PDIP:
                case PACKAGE_TYPE_SOP:
                case PACKAGE_TYPE_TSSOP:
                case PACKAGE_TYPE_VQFN:
                    (void)package_info(config->package);
                    break;

                default:
                    unsupported();
            }
            break;

        case LOGIC_FAMILY_ALS:
            switch (config->package) {
                case PACKAGE_TYPE_LCCC:
                case PACKAGE_TYPE_CDIP:
                case PACKAGE_TYPE_CFP:
                case PACKAGE_TYPE_SOIC:
                case PACKAGE_TYPE_PDIP:
                    (void)package_info(config->package);
                    break;

                default:
                    unsupported();
            }
            break;

        case LOGIC_FAMILY_AS:
            switch (config->package) {
                case PACKAGE_TYPE_LCCC:
                case PACKAGE_TYPE_CDIP:
                case PACKAGE_TYPE_SOIC:
                case PACKAGE_TYPE_PDIP:
                    (void)package_info(config->package);
                    break;

                default:
                    unsupported();
            }
            break;

        case LOGIC_FAMILY_F:
            switch (config->package) {
                case PACKAGE_TYPE_LCCC:
                case PACKAGE_TYPE_CDIP:
                case PACKAGE_TYPE_CFP:
                case PACKAGE_TYPE_SOIC:
                case PACKAGE_TYPE_PDIP:
                    (void)package_info(config->package);
                    break;

                default:
                    unsupported();
            }
            break;

        case LOGIC_FAMILY_HC:
            switch (config->package) {
                case PACKAGE_TYPE_LCCC:
                case PACKAGE_TYPE_CDIP:
                case PACKAGE_TYPE_CFP:
                case PACKAGE_TYPE_SOIC:
                case PACKAGE_TYPE_SSOP:
                case PACKAGE_TYPE_PDIP:
                case PACKAGE_TYPE_SOP:
                case PACKAGE_TYPE_TSSOP:
                    (void)package_info(config->package);
                    break;

                default:
                    unsupported();
            }
            break;

        case LOGIC_FAMILY_HCT:
            switch (config->package) {
                case PACKAGE_TYPE_LCCC:
                case PACKAGE_TYPE_SOIC:
                case PACKAGE_TYPE_SSOP:
                case PACKAGE_TYPE_PDIP:
                case PACKAGE_TYPE_SOP:
                case PACKAGE_TYPE_TSSOP:
                    (void)package_info(config->package);
                    break;

                default:
                    unsupported();
            }
            break;

        case LOGIC_FAMILY_LS:
        case LOGIC_FAMILY_S:
            switch (config->package) {
                case PACKAGE_TYPE_LCCC:
                case PACKAGE_TYPE_CDIP:
                case PACKAGE_TYPE_CFP:
                case PACKAGE_TYPE_SOIC:
                case PACKAGE_TYPE_PDIP:
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
    const Sn74_32Config *chip = config;

    validate_config(chip);
    return package_info(chip->package)->pin_count;
}

static void *create(const void *config) {
    const Sn74_32Config *chip = config;
    Sn74_32State *state;

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

static LogicValue or_value(LogicValue a, LogicValue b) {
    if (a == LOGIC_1 || b == LOGIC_1) {
        return LOGIC_1;
    }

    if (a == LOGIC_0 && b == LOGIC_0) {
        return LOGIC_0;
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
    const Sn74_32State *state,
    const GatePins *gate
) {
    LogicValue a = sim_read_pin(sim, instance, gate->input_a);
    LogicValue b = sim_read_pin(sim, instance, gate->input_b);
    LogicValue output = or_value(a, b);

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
    const Sn74_32State *chip = state;
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
    const Sn74_32State *chip = state;
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

const SimComponentType SN74_32_TYPE = {
    "SN74_32",
    pin_count,
    create,
    destroy,
    start,
    pin_changed,
    NULL
};

const SimComponentType *sn74_32_lookup(
    const char *part_number,
    Sn74_32Config *config
) {
    if (part_number == NULL || config == NULL) {
        return NULL;
    }

    return lookup(part_number, config) ? &SN74_32_TYPE : NULL;
}

/* ---- Catalog definition ------------------------------------------------- */

static const char *const SN74_32_PART_NUMBERS[] = {
    "SN74AC32N", "SN74AC32D",
    "SN74ACT32N", "SN74ACT32D",
    "SN74AHC32N", "SN74AHC32D",
    "SN74AHCT32N", "SN74AHCT32D",
    "SN74ALS32N", "SN74ALS32D",
    "SN74F32N", "SN74F32D",
    "SN74HC32N", "SN74HC32D",
    "SN74HCT32N", "SN74HCT32D",
    "SN74LS32N", "SN74LS32D",
};

static const ChipPin SN74_32_PINS[] = {
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

const ChipDefinition SN74_32_DEFINITION = {
    "74_32",
    "OR",
    "Quad 2-input OR gate",
    "Logic",
    SN74_32_PART_NUMBERS,
    sizeof(SN74_32_PART_NUMBERS) / sizeof(SN74_32_PART_NUMBERS[0]),
    SN74_32_PINS,
    sizeof(SN74_32_PINS) / sizeof(SN74_32_PINS[0]),
    sn74_32_lookup,
    4,
};
