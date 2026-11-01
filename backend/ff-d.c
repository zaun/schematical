#include "ff-d.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define FF_NO_PIN ((size_t)-1)
#define FF_MAX_UNITS 8
#define FF_COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* One flip-flop inside a package. Unused pins are FF_NO_PIN. */
typedef struct {
    size_t d;
    size_t clk;
    size_t pre;
    size_t clr;
    size_t q;
    size_t qn;
} FfUnit;

typedef struct {
    size_t pin_count;
    size_t unit_count;
    const FfUnit *units;
    size_t oe;           /* active-low output enable, FF_NO_PIN if none */
    int pre_active_high;
    int clr_active_high;
    int negative_edge;
} FfLayout;

typedef struct {
    LogicFamily family;
    SimTime rise_delay;
    SimTime fall_delay;
    int clr_wins;        /* PRE and CLR both asserted: 1 => Q=L, 0 => Q=H, QBAR=H */
} FfFamily;

typedef struct {
    const char *name;
    LogicFamily family;
    PackageType package;
} FfPart;

typedef struct {
    const FfLayout *layout;
    const FfFamily *families;
    size_t family_count;
    const FfPart *parts;
    size_t part_count;
} FfDefinitionData;

typedef struct {
    const FfLayout *layout;
    SimTime rise_delay;
    SimTime fall_delay;
    int clr_wins;
    LogicValue q[FF_MAX_UNITS];
    LogicValue qn[FF_MAX_UNITS];
    /* Values last handed to the simulator, so unchanged outputs are not rescheduled. */
    int scheduled[FF_MAX_UNITS];
    LogicValue driven_q[FF_MAX_UNITS];
    LogicValue driven_qn[FF_MAX_UNITS];
} FfState;

#define FF_PART_ENTRY(name, family, package) {name, family, package}
#define FF_PART_NAME(name, family, package) name

static void unsupported(void) {
    assert(0 && "unsupported flip-flop configuration");
    abort();
}

/* ---- Lookup and validation ---------------------------------------------- */

static const FfFamily *find_family(const FfDefinitionData *def, LogicFamily family) {
    size_t i;

    for (i = 0; i < def->family_count; i++) {
        if (def->families[i].family == family) {
            return &def->families[i];
        }
    }
    return NULL;
}

static int part_exists(const FfDefinitionData *def, LogicFamily family, PackageType package) {
    size_t i;

    for (i = 0; i < def->part_count; i++) {
        if (def->parts[i].family == family && def->parts[i].package == package) {
            return 1;
        }
    }
    return 0;
}

static int lookup(const FfDefinitionData *def, const char *name, SimLogicConfig *config) {
    size_t i;

    for (i = 0; i < def->part_count; i++) {
        if (strcmp(def->parts[i].name, name) == 0) {
            config->family = def->parts[i].family;
            config->package = def->parts[i].package;
            return 1;
        }
    }
    return 0;
}

static void validate(const FfDefinitionData *def, const void *config) {
    const SimLogicConfig *chip = config;

    if (chip == NULL || find_family(def, chip->family) == NULL ||
        !part_exists(def, chip->family, chip->package)) {
        unsupported();
    }
}

static size_t ff_pin_count(const FfDefinitionData *def, const void *config) {
    validate(def, config);
    return def->layout->pin_count;
}

static void *ff_create(const FfDefinitionData *def, const void *config) {
    const SimLogicConfig *chip = config;
    const FfFamily *family;
    FfState *state;
    size_t i;

    validate(def, config);
    family = find_family(def, chip->family);

    state = malloc(sizeof(*state));
    if (state == NULL) {
        return NULL;
    }

    state->layout = def->layout;
    state->rise_delay = family->rise_delay;
    state->fall_delay = family->fall_delay;
    state->clr_wins = family->clr_wins;
    for (i = 0; i < FF_MAX_UNITS; i++) {
        state->q[i] = LOGIC_X;
        state->qn[i] = LOGIC_X;
        state->scheduled[i] = 0;
    }
    return state;
}

static void ff_destroy(void *state) {
    free(state);
}

/* ---- Behaviour ----------------------------------------------------------- */

/* 1 when asserted, 0 when not, -1 when the level is unknown. */
static int asserted(LogicValue value, int active_high) {
    LogicValue level;

    if (value != LOGIC_0 && value != LOGIC_1) {
        return -1;
    }
    level = active_high ? LOGIC_1 : LOGIC_0;
    return value == level;
}

static void async_status(
    Sim *sim,
    SimInstance *instance,
    const FfLayout *layout,
    const FfUnit *unit,
    int *pre,
    int *clr
) {
    *pre = unit->pre == FF_NO_PIN
        ? 0
        : asserted(sim_read_pin(sim, instance, unit->pre), layout->pre_active_high);
    *clr = unit->clr == FF_NO_PIN
        ? 0
        : asserted(sim_read_pin(sim, instance, unit->clr), layout->clr_active_high);
}

static void set_unit(FfState *state, size_t i, LogicValue q, LogicValue qn) {
    state->q[i] = q;
    state->qn[i] = qn;
}

/* Applies PRE/CLR to one flip-flop. Does nothing while neither is asserted. */
static void apply_async(Sim *sim, SimInstance *instance, FfState *state, size_t i) {
    const FfUnit *unit = &state->layout->units[i];
    int pre;
    int clr;

    async_status(sim, instance, state->layout, unit, &pre, &clr);

    if (clr == 1 && state->clr_wins) {
        set_unit(state, i, LOGIC_0, LOGIC_1);
    } else if (pre < 0 || clr < 0) {
        set_unit(state, i, LOGIC_X, LOGIC_X);
    } else if (pre == 1 && clr == 1) {
        set_unit(state, i, LOGIC_1, LOGIC_1);
    } else if (clr == 1) {
        set_unit(state, i, LOGIC_0, LOGIC_1);
    } else if (pre == 1) {
        set_unit(state, i, LOGIC_1, LOGIC_0);
    }
}

static void capture(Sim *sim, SimInstance *instance, FfState *state, size_t i) {
    LogicValue d = sim_read_pin(sim, instance, state->layout->units[i].d);

    if (d == LOGIC_0) {
        set_unit(state, i, LOGIC_0, LOGIC_1);
    } else if (d == LOGIC_1) {
        set_unit(state, i, LOGIC_1, LOGIC_0);
    } else {
        set_unit(state, i, LOGIC_X, LOGIC_X);
    }
}

static void schedule_pin(
    Sim *sim,
    SimInstance *instance,
    const FfState *state,
    size_t pin,
    LogicValue value
) {
    SimTime delay;

    if (pin == FF_NO_PIN) {
        return;
    }

    if (value == LOGIC_0) {
        delay = state->fall_delay;
    } else if (value == LOGIC_1 || value == LOGIC_Z) {
        delay = state->rise_delay;
    } else {
        delay = state->rise_delay > state->fall_delay
            ? state->rise_delay
            : state->fall_delay;
    }

    sim_schedule_inertial_drive(sim, instance, pin, value, DRIVE_STRONG, delay);
}

static void schedule_unit(Sim *sim, SimInstance *instance, FfState *state, size_t i) {
    const FfUnit *unit = &state->layout->units[i];
    LogicValue q = state->q[i];
    LogicValue qn = state->qn[i];

    if (state->layout->oe != FF_NO_PIN) {
        LogicValue oe = sim_read_pin(sim, instance, state->layout->oe);

        if (oe == LOGIC_1) {
            q = LOGIC_Z;
            qn = LOGIC_Z;
        } else if (oe != LOGIC_0) {
            q = LOGIC_X;
            qn = LOGIC_X;
        }
    }

    if (state->scheduled[i] && state->driven_q[i] == q && state->driven_qn[i] == qn) {
        return;
    }
    state->scheduled[i] = 1;
    state->driven_q[i] = q;
    state->driven_qn[i] = qn;

    schedule_pin(sim, instance, state, unit->q, q);
    schedule_pin(sim, instance, state, unit->qn, qn);
}

static int is_clock_edge(const FfLayout *layout, LogicValue old_value, LogicValue new_value) {
    if (layout->negative_edge) {
        return old_value == LOGIC_1 && new_value == LOGIC_0;
    }
    return old_value == LOGIC_0 && new_value == LOGIC_1;
}

static void ff_start(Sim *sim, SimInstance *instance, void *state_ptr) {
    FfState *state = state_ptr;
    size_t i;

    for (i = 0; i < state->layout->unit_count; i++) {
        set_unit(state, i, LOGIC_X, LOGIC_X);
        apply_async(sim, instance, state, i);
        schedule_unit(sim, instance, state, i);
    }
}

static void ff_pin_changed(
    Sim *sim,
    SimInstance *instance,
    void *state_ptr,
    size_t pin,
    LogicValue old_value,
    LogicValue new_value
) {
    FfState *state = state_ptr;
    const FfLayout *layout = state->layout;
    size_t i;

    if (pin == layout->oe) {
        for (i = 0; i < layout->unit_count; i++) {
            schedule_unit(sim, instance, state, i);
        }
        return;
    }

    for (i = 0; i < layout->unit_count; i++) {
        const FfUnit *unit = &layout->units[i];

        if (pin == unit->pre || pin == unit->clr) {
            apply_async(sim, instance, state, i);
            schedule_unit(sim, instance, state, i);
        } else if (pin == unit->clk && is_clock_edge(layout, old_value, new_value)) {
            int pre;
            int clr;

            async_status(sim, instance, layout, unit, &pre, &clr);
            if (pre == 0 && clr == 0) {
                capture(sim, instance, state, i);
                schedule_unit(sim, instance, state, i);
            }
        }
    }
}

/* ---- Definitions --------------------------------------------------------- */

/*
 * Each definition gets its own SimComponentType, because the model callbacks
 * receive only the configuration and need the layout from their definition.
 */
#define FF_DEFINE_CHIP(NAME, ID, FUNCTION, DESCRIPTION, LAYOUT, PINS, FAMILIES, PART_LIST, GATES) \
    static const FfPart NAME##_PARTS[] = { PART_LIST(FF_PART_ENTRY) }; \
    static const char *const NAME##_NAMES[] = { PART_LIST(FF_PART_NAME) }; \
    static const FfDefinitionData NAME##_DATA = { \
        &LAYOUT, FAMILIES, FF_COUNT(FAMILIES), NAME##_PARTS, FF_COUNT(NAME##_PARTS) \
    }; \
    static size_t NAME##_pin_count(const void *config) { \
        return ff_pin_count(&NAME##_DATA, config); \
    } \
    static void *NAME##_create(const void *config) { \
        return ff_create(&NAME##_DATA, config); \
    } \
    static void NAME##_start(Sim *sim, SimInstance *instance, void *state) { \
        ff_start(sim, instance, state); \
    } \
    static void NAME##_pin_changed( \
        Sim *sim, SimInstance *instance, void *state, size_t pin, \
        LogicValue old_value, LogicValue new_value \
    ) { \
        ff_pin_changed(sim, instance, state, pin, old_value, new_value); \
    } \
    static const SimComponentType NAME##_TYPE = { \
        ID, NAME##_pin_count, NAME##_create, ff_destroy, \
        NAME##_start, NAME##_pin_changed, NULL \
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
        NAME##_NAMES, FF_COUNT(NAME##_NAMES), \
        PINS, FF_COUNT(PINS), \
        NAME##_lookup, GATES \
    };

/* ---- 14-pin dual, 74 pinout (1CLR 1 ... VCC 14) ------------------------- */

static const FfUnit UNITS_DUAL74[] = {
    /* d, clk, pre, clr, q, qn */
    {FF_PIN(2), FF_PIN(3), FF_PIN(4), FF_PIN(1), FF_PIN(5), FF_PIN(6)},
    {FF_PIN(12), FF_PIN(11), FF_PIN(10), FF_PIN(13), FF_PIN(9), FF_PIN(8)},
};

static const FfLayout LAYOUT_DUAL74 = {14, 2, UNITS_DUAL74, FF_NO_PIN, 0, 0, 0};
static const FfLayout LAYOUT_DUAL74_NEG = {14, 2, UNITS_DUAL74, FF_NO_PIN, 0, 0, 1};

static const ChipPin PINS_DUAL74[] = {
    {1, "1CLR", PIN_ROLE_INPUT},
    {2, "1D", PIN_ROLE_INPUT},
    {3, "1CLK", PIN_ROLE_INPUT},
    {4, "1PRE", PIN_ROLE_INPUT},
    {5, "1Q", PIN_ROLE_OUTPUT},
    {6, "1QBAR", PIN_ROLE_OUTPUT},
    {7, "GND", PIN_ROLE_GROUND},
    {8, "2QBAR", PIN_ROLE_OUTPUT},
    {9, "2Q", PIN_ROLE_OUTPUT},
    {10, "2PRE", PIN_ROLE_INPUT},
    {11, "2CLK", PIN_ROLE_INPUT},
    {12, "2D", PIN_ROLE_INPUT},
    {13, "2CLR", PIN_ROLE_INPUT},
    {14, "VCC", PIN_ROLE_POWER},
};

static const FfFamily FAMILIES_74_74[] = {
    {LOGIC_FAMILY_HC, 20000, 20000, 0},
    {LOGIC_FAMILY_HCT, 20000, 20000, 0},
    {LOGIC_FAMILY_AHC, 6100, 6100, 0},
    {LOGIC_FAMILY_AHCT, 6300, 6300, 0},
    {LOGIC_FAMILY_AC, 6000, 6000, 0},
    {LOGIC_FAMILY_ACT, 7500, 6000, 0},
    {LOGIC_FAMILY_F, 4900, 5800, 0},
    {LOGIC_FAMILY_LS, 14000, 20000, 0},
    {LOGIC_FAMILY_LV, 10200, 10200, 0},
    {LOGIC_FAMILY_LVC, 5200, 5200, 0},
    {LOGIC_FAMILY_AUC, 1400, 1400, 1},
    {LOGIC_FAMILY_HCS, 8000, 8000, 0},
};

#define PARTS_74_74(X) \
    X("SN74HC74D", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC74DB", LOGIC_FAMILY_HC, PACKAGE_TYPE_SSOP), \
    X("SN74HC74N", LOGIC_FAMILY_HC, PACKAGE_TYPE_PDIP), \
    X("SN74HC74NS", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP), \
    X("SN74HC74PW", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HCT74D", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN74HCT74DB", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SSOP), \
    X("SN74HCT74N", LOGIC_FAMILY_HCT, PACKAGE_TYPE_PDIP), \
    X("SN74HCT74NS", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOP), \
    X("SN74HCT74PW", LOGIC_FAMILY_HCT, PACKAGE_TYPE_TSSOP), \
    X("SN74AHC74D", LOGIC_FAMILY_AHC, PACKAGE_TYPE_SOIC), \
    X("SN74AHC74DB", LOGIC_FAMILY_AHC, PACKAGE_TYPE_SSOP), \
    X("SN74AHC74DGV", LOGIC_FAMILY_AHC, PACKAGE_TYPE_TVSOP), \
    X("SN74AHC74N", LOGIC_FAMILY_AHC, PACKAGE_TYPE_PDIP), \
    X("SN74AHC74PW", LOGIC_FAMILY_AHC, PACKAGE_TYPE_TSSOP), \
    X("SN74AHCT74D", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SOIC), \
    X("SN74AHCT74DB", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SSOP), \
    X("SN74AHCT74DGV", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_TVSOP), \
    X("SN74AHCT74N", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_PDIP), \
    X("SN74AHCT74NS", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SOP), \
    X("SN74AHCT74PW", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_TSSOP), \
    X("SN74AC74D", LOGIC_FAMILY_AC, PACKAGE_TYPE_SOIC), \
    X("SN74AC74DB", LOGIC_FAMILY_AC, PACKAGE_TYPE_SSOP), \
    X("SN74AC74N", LOGIC_FAMILY_AC, PACKAGE_TYPE_PDIP), \
    X("SN74AC74PW", LOGIC_FAMILY_AC, PACKAGE_TYPE_TSSOP), \
    X("SN74ACT74D", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SOIC), \
    X("SN74ACT74DB", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SSOP), \
    X("SN74ACT74N", LOGIC_FAMILY_ACT, PACKAGE_TYPE_PDIP), \
    X("SN74ACT74NS", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SOP), \
    X("SN74ACT74PW", LOGIC_FAMILY_ACT, PACKAGE_TYPE_TSSOP), \
    X("SN74F74D", LOGIC_FAMILY_F, PACKAGE_TYPE_SOIC), \
    X("SN74F74N", LOGIC_FAMILY_F, PACKAGE_TYPE_PDIP), \
    X("SN74F74NS", LOGIC_FAMILY_F, PACKAGE_TYPE_SOP), \
    X("SN74LS74AD", LOGIC_FAMILY_LS, PACKAGE_TYPE_SOIC), \
    X("SN74LS74ADB", LOGIC_FAMILY_LS, PACKAGE_TYPE_SSOP), \
    X("SN74LS74AN", LOGIC_FAMILY_LS, PACKAGE_TYPE_PDIP), \
    X("SN74LS74ANS", LOGIC_FAMILY_LS, PACKAGE_TYPE_SOP), \
    X("SN74LV74AD", LOGIC_FAMILY_LV, PACKAGE_TYPE_SOIC), \
    X("SN74LV74ADB", LOGIC_FAMILY_LV, PACKAGE_TYPE_SSOP), \
    X("SN74LV74ADGV", LOGIC_FAMILY_LV, PACKAGE_TYPE_TVSOP), \
    X("SN74LV74APW", LOGIC_FAMILY_LV, PACKAGE_TYPE_TSSOP), \
    X("SN74LVC74AD", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOIC), \
    X("SN74LVC74ADB", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SSOP), \
    X("SN74LVC74ANS", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOP), \
    X("SN74LVC74APW", LOGIC_FAMILY_LVC, PACKAGE_TYPE_TSSOP), \
    X("SN74AUC74RGY", LOGIC_FAMILY_AUC, PACKAGE_TYPE_VQFN), \
    X("SN74HCS74D", LOGIC_FAMILY_HCS, PACKAGE_TYPE_SOIC), \
    X("SN74HCS74PW", LOGIC_FAMILY_HCS, PACKAGE_TYPE_TSSOP)

FF_DEFINE_CHIP(FF_74_74, "74_74", "D flip-flop",
    "Dual positive-edge-triggered D flip-flop with preset and clear",
    LAYOUT_DUAL74, PINS_DUAL74, FAMILIES_74_74, PARTS_74_74, 2)

static const FfFamily FAMILIES_74_72[] = {
    {LOGIC_FAMILY_HCS, 8000, 8000, 0},
};

#define PARTS_74_72(X) \
    X("SN74HCS72D", LOGIC_FAMILY_HCS, PACKAGE_TYPE_SOIC), \
    X("SN74HCS72PW", LOGIC_FAMILY_HCS, PACKAGE_TYPE_TSSOP)

FF_DEFINE_CHIP(FF_74_72, "74_72", "D flip-flop",
    "Dual negative-edge-triggered D flip-flop with preset and clear",
    LAYOUT_DUAL74_NEG, PINS_DUAL74, FAMILIES_74_72, PARTS_74_72, 2)

/* ---- 16-pin hex, common CLR ---------------------------------------------- */

static const FfUnit UNITS_HEX174[] = {
    {FF_PIN(3), FF_PIN(9), FF_NO_PIN, FF_PIN(1), FF_PIN(2), FF_NO_PIN},
    {FF_PIN(4), FF_PIN(9), FF_NO_PIN, FF_PIN(1), FF_PIN(5), FF_NO_PIN},
    {FF_PIN(6), FF_PIN(9), FF_NO_PIN, FF_PIN(1), FF_PIN(7), FF_NO_PIN},
    {FF_PIN(11), FF_PIN(9), FF_NO_PIN, FF_PIN(1), FF_PIN(10), FF_NO_PIN},
    {FF_PIN(13), FF_PIN(9), FF_NO_PIN, FF_PIN(1), FF_PIN(12), FF_NO_PIN},
    {FF_PIN(14), FF_PIN(9), FF_NO_PIN, FF_PIN(1), FF_PIN(15), FF_NO_PIN},
};

static const FfLayout LAYOUT_HEX174 = {16, 6, UNITS_HEX174, FF_NO_PIN, 0, 0, 0};

static const ChipPin PINS_HEX174[] = {
    {1, "CLR", PIN_ROLE_INPUT},
    {2, "1Q", PIN_ROLE_OUTPUT},
    {3, "1D", PIN_ROLE_INPUT},
    {4, "2D", PIN_ROLE_INPUT},
    {5, "2Q", PIN_ROLE_OUTPUT},
    {6, "3D", PIN_ROLE_INPUT},
    {7, "3Q", PIN_ROLE_OUTPUT},
    {8, "GND", PIN_ROLE_GROUND},
    {9, "CLK", PIN_ROLE_INPUT},
    {10, "4Q", PIN_ROLE_OUTPUT},
    {11, "4D", PIN_ROLE_INPUT},
    {12, "5Q", PIN_ROLE_OUTPUT},
    {13, "5D", PIN_ROLE_INPUT},
    {14, "6D", PIN_ROLE_INPUT},
    {15, "6Q", PIN_ROLE_OUTPUT},
    {16, "VCC", PIN_ROLE_POWER},
};

static const FfFamily FAMILIES_74_174[] = {
    {LOGIC_FAMILY_HC, 17000, 17000, 0},
};

#define PARTS_74_174(X) \
    X("SN74HC174D", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC174DB", LOGIC_FAMILY_HC, PACKAGE_TYPE_SSOP), \
    X("SN74HC174N", LOGIC_FAMILY_HC, PACKAGE_TYPE_PDIP), \
    X("SN74HC174NS", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP), \
    X("SN74HC174PW", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP)

FF_DEFINE_CHIP(FF_74_174, "74_174", "D flip-flop",
    "Hex positive-edge-triggered D flip-flop with common clear",
    LAYOUT_HEX174, PINS_HEX174, FAMILIES_74_174, PARTS_74_174, 6)

/* ---- 16-pin quad, common CLR (74 pinout; CD40175B shares it) ------------ */

static const FfUnit UNITS_QUAD175[] = {
    {FF_PIN(4), FF_PIN(9), FF_NO_PIN, FF_PIN(1), FF_PIN(2), FF_PIN(3)},
    {FF_PIN(5), FF_PIN(9), FF_NO_PIN, FF_PIN(1), FF_PIN(7), FF_PIN(6)},
    {FF_PIN(12), FF_PIN(9), FF_NO_PIN, FF_PIN(1), FF_PIN(10), FF_PIN(11)},
    {FF_PIN(13), FF_PIN(9), FF_NO_PIN, FF_PIN(1), FF_PIN(15), FF_PIN(14)},
};

static const FfLayout LAYOUT_QUAD175 = {16, 4, UNITS_QUAD175, FF_NO_PIN, 0, 0, 0};

static const ChipPin PINS_QUAD175[] = {
    {1, "CLR", PIN_ROLE_INPUT},
    {2, "1Q", PIN_ROLE_OUTPUT},
    {3, "1QBAR", PIN_ROLE_OUTPUT},
    {4, "1D", PIN_ROLE_INPUT},
    {5, "2D", PIN_ROLE_INPUT},
    {6, "2QBAR", PIN_ROLE_OUTPUT},
    {7, "2Q", PIN_ROLE_OUTPUT},
    {8, "GND", PIN_ROLE_GROUND},
    {9, "CLK", PIN_ROLE_INPUT},
    {10, "3Q", PIN_ROLE_OUTPUT},
    {11, "3QBAR", PIN_ROLE_OUTPUT},
    {12, "3D", PIN_ROLE_INPUT},
    {13, "4D", PIN_ROLE_INPUT},
    {14, "4QBAR", PIN_ROLE_OUTPUT},
    {15, "4Q", PIN_ROLE_OUTPUT},
    {16, "VCC", PIN_ROLE_POWER},
};

static const FfFamily FAMILIES_74_175[] = {
    {LOGIC_FAMILY_HC, 16000, 16000, 0},
};

#define PARTS_74_175(X) \
    X("SN74HC175D", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC175DB", LOGIC_FAMILY_HC, PACKAGE_TYPE_SSOP), \
    X("SN74HC175N", LOGIC_FAMILY_HC, PACKAGE_TYPE_PDIP), \
    X("SN74HC175NS", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP), \
    X("SN74HC175PW", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP)

FF_DEFINE_CHIP(FF_74_175, "74_175", "D flip-flop",
    "Quad positive-edge-triggered D flip-flop with common clear",
    LAYOUT_QUAD175, PINS_QUAD175, FAMILIES_74_175, PARTS_74_175, 4)

/* ---- 20-pin octal, common CLR (273) ------------------------------------- */

static const FfUnit UNITS_OCT_CLR[] = {
    {FF_PIN(3), FF_PIN(11), FF_NO_PIN, FF_PIN(1), FF_PIN(2), FF_NO_PIN},
    {FF_PIN(4), FF_PIN(11), FF_NO_PIN, FF_PIN(1), FF_PIN(5), FF_NO_PIN},
    {FF_PIN(7), FF_PIN(11), FF_NO_PIN, FF_PIN(1), FF_PIN(6), FF_NO_PIN},
    {FF_PIN(8), FF_PIN(11), FF_NO_PIN, FF_PIN(1), FF_PIN(9), FF_NO_PIN},
    {FF_PIN(13), FF_PIN(11), FF_NO_PIN, FF_PIN(1), FF_PIN(12), FF_NO_PIN},
    {FF_PIN(14), FF_PIN(11), FF_NO_PIN, FF_PIN(1), FF_PIN(15), FF_NO_PIN},
    {FF_PIN(17), FF_PIN(11), FF_NO_PIN, FF_PIN(1), FF_PIN(16), FF_NO_PIN},
    {FF_PIN(18), FF_PIN(11), FF_NO_PIN, FF_PIN(1), FF_PIN(19), FF_NO_PIN},
};

static const FfLayout LAYOUT_OCT_CLR = {20, 8, UNITS_OCT_CLR, FF_NO_PIN, 0, 0, 0};

static const ChipPin PINS_OCT_CLR[] = {
    {1, "CLR", PIN_ROLE_INPUT},
    {2, "1Q", PIN_ROLE_OUTPUT},
    {3, "1D", PIN_ROLE_INPUT},
    {4, "2D", PIN_ROLE_INPUT},
    {5, "2Q", PIN_ROLE_OUTPUT},
    {6, "3Q", PIN_ROLE_OUTPUT},
    {7, "3D", PIN_ROLE_INPUT},
    {8, "4D", PIN_ROLE_INPUT},
    {9, "4Q", PIN_ROLE_OUTPUT},
    {10, "GND", PIN_ROLE_GROUND},
    {11, "CLK", PIN_ROLE_INPUT},
    {12, "5Q", PIN_ROLE_OUTPUT},
    {13, "5D", PIN_ROLE_INPUT},
    {14, "6D", PIN_ROLE_INPUT},
    {15, "6Q", PIN_ROLE_OUTPUT},
    {16, "7Q", PIN_ROLE_OUTPUT},
    {17, "7D", PIN_ROLE_INPUT},
    {18, "8D", PIN_ROLE_INPUT},
    {19, "8Q", PIN_ROLE_OUTPUT},
    {20, "VCC", PIN_ROLE_POWER},
};

static const FfFamily FAMILIES_74_273[] = {
    {LOGIC_FAMILY_HC, 15000, 15000, 0},
    {LOGIC_FAMILY_AHC, 7300, 7300, 0},
    {LOGIC_FAMILY_AHCT, 6500, 6800, 0},
};

#define PARTS_74_273(X) \
    X("SN74HC273DW", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC273DB", LOGIC_FAMILY_HC, PACKAGE_TYPE_SSOP), \
    X("SN74HC273N", LOGIC_FAMILY_HC, PACKAGE_TYPE_PDIP), \
    X("SN74HC273NS", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP), \
    X("SN74HC273PW", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74AHC273DW", LOGIC_FAMILY_AHC, PACKAGE_TYPE_SOIC), \
    X("SN74AHC273DB", LOGIC_FAMILY_AHC, PACKAGE_TYPE_SSOP), \
    X("SN74AHC273DGS", LOGIC_FAMILY_AHC, PACKAGE_TYPE_VSSOP), \
    X("SN74AHC273DGV", LOGIC_FAMILY_AHC, PACKAGE_TYPE_TVSOP), \
    X("SN74AHC273NS", LOGIC_FAMILY_AHC, PACKAGE_TYPE_SOP), \
    X("SN74AHC273PW", LOGIC_FAMILY_AHC, PACKAGE_TYPE_TSSOP), \
    X("SN74AHCT273DW", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SOIC), \
    X("SN74AHCT273DB", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SSOP), \
    X("SN74AHCT273N", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_PDIP), \
    X("SN74AHCT273NS", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SOP), \
    X("SN74AHCT273PW", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_TSSOP)

FF_DEFINE_CHIP(FF_74_273, "74_273", "D flip-flop",
    "Octal positive-edge-triggered D flip-flop with common clear",
    LAYOUT_OCT_CLR, PINS_OCT_CLR, FAMILIES_74_273, PARTS_74_273, 8)

/* ---- 20-pin octal with OE (374) ----------------------------------------- */

static const FfUnit UNITS_OCT_374[] = {
    {FF_PIN(3), FF_PIN(11), FF_NO_PIN, FF_NO_PIN, FF_PIN(2), FF_NO_PIN},
    {FF_PIN(4), FF_PIN(11), FF_NO_PIN, FF_NO_PIN, FF_PIN(5), FF_NO_PIN},
    {FF_PIN(7), FF_PIN(11), FF_NO_PIN, FF_NO_PIN, FF_PIN(6), FF_NO_PIN},
    {FF_PIN(8), FF_PIN(11), FF_NO_PIN, FF_NO_PIN, FF_PIN(9), FF_NO_PIN},
    {FF_PIN(13), FF_PIN(11), FF_NO_PIN, FF_NO_PIN, FF_PIN(12), FF_NO_PIN},
    {FF_PIN(14), FF_PIN(11), FF_NO_PIN, FF_NO_PIN, FF_PIN(15), FF_NO_PIN},
    {FF_PIN(17), FF_PIN(11), FF_NO_PIN, FF_NO_PIN, FF_PIN(16), FF_NO_PIN},
    {FF_PIN(18), FF_PIN(11), FF_NO_PIN, FF_NO_PIN, FF_PIN(19), FF_NO_PIN},
};

static const FfLayout LAYOUT_OCT_374 = {20, 8, UNITS_OCT_374, FF_PIN(1), 0, 0, 0};

static const ChipPin PINS_OCT_374[] = {
    {1, "OE", PIN_ROLE_INPUT},
    {2, "1Q", PIN_ROLE_OUTPUT},
    {3, "1D", PIN_ROLE_INPUT},
    {4, "2D", PIN_ROLE_INPUT},
    {5, "2Q", PIN_ROLE_OUTPUT},
    {6, "3Q", PIN_ROLE_OUTPUT},
    {7, "3D", PIN_ROLE_INPUT},
    {8, "4D", PIN_ROLE_INPUT},
    {9, "4Q", PIN_ROLE_OUTPUT},
    {10, "GND", PIN_ROLE_GROUND},
    {11, "CLK", PIN_ROLE_INPUT},
    {12, "5Q", PIN_ROLE_OUTPUT},
    {13, "5D", PIN_ROLE_INPUT},
    {14, "6D", PIN_ROLE_INPUT},
    {15, "6Q", PIN_ROLE_OUTPUT},
    {16, "7Q", PIN_ROLE_OUTPUT},
    {17, "7D", PIN_ROLE_INPUT},
    {18, "8D", PIN_ROLE_INPUT},
    {19, "8Q", PIN_ROLE_OUTPUT},
    {20, "VCC", PIN_ROLE_POWER},
};

static const FfFamily FAMILIES_74_374[] = {
    {LOGIC_FAMILY_HC, 17000, 17000, 0},
    {LOGIC_FAMILY_HCT, 30000, 30000, 0},
    {LOGIC_FAMILY_AHCT, 6400, 6400, 0},
};

#define PARTS_74_374(X) \
    X("SN74HC374DW", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC374DB", LOGIC_FAMILY_HC, PACKAGE_TYPE_SSOP), \
    X("SN74HC374N", LOGIC_FAMILY_HC, PACKAGE_TYPE_PDIP), \
    X("SN74HC374NS", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP), \
    X("SN74HC374PW", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HCT374DW", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN74HCT374DB", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SSOP), \
    X("SN74HCT374N", LOGIC_FAMILY_HCT, PACKAGE_TYPE_PDIP), \
    X("SN74HCT374NS", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOP), \
    X("SN74HCT374PW", LOGIC_FAMILY_HCT, PACKAGE_TYPE_TSSOP), \
    X("SN74AHCT374DW", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SOIC), \
    X("SN74AHCT374DB", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SSOP), \
    X("SN74AHCT374N", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_PDIP), \
    X("SN74AHCT374PW", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_TSSOP)

FF_DEFINE_CHIP(FF_74_374, "74_374", "D flip-flop",
    "Octal positive-edge-triggered D flip-flop with 3-state outputs",
    LAYOUT_OCT_374, PINS_OCT_374, FAMILIES_74_374, PARTS_74_374, 8)

/* ---- 20-pin octal with OE, 574 pinout ----------------------------------- */

static const FfUnit UNITS_OCT_574[] = {
    {FF_PIN(2), FF_PIN(11), FF_NO_PIN, FF_NO_PIN, FF_PIN(19), FF_NO_PIN},
    {FF_PIN(3), FF_PIN(11), FF_NO_PIN, FF_NO_PIN, FF_PIN(18), FF_NO_PIN},
    {FF_PIN(4), FF_PIN(11), FF_NO_PIN, FF_NO_PIN, FF_PIN(17), FF_NO_PIN},
    {FF_PIN(5), FF_PIN(11), FF_NO_PIN, FF_NO_PIN, FF_PIN(16), FF_NO_PIN},
    {FF_PIN(6), FF_PIN(11), FF_NO_PIN, FF_NO_PIN, FF_PIN(15), FF_NO_PIN},
    {FF_PIN(7), FF_PIN(11), FF_NO_PIN, FF_NO_PIN, FF_PIN(14), FF_NO_PIN},
    {FF_PIN(8), FF_PIN(11), FF_NO_PIN, FF_NO_PIN, FF_PIN(13), FF_NO_PIN},
    {FF_PIN(9), FF_PIN(11), FF_NO_PIN, FF_NO_PIN, FF_PIN(12), FF_NO_PIN},
};

static const FfLayout LAYOUT_OCT_574 = {20, 8, UNITS_OCT_574, FF_PIN(1), 0, 0, 0};

static const ChipPin PINS_OCT_574[] = {
    {1, "OE", PIN_ROLE_INPUT},
    {2, "1D", PIN_ROLE_INPUT},
    {3, "2D", PIN_ROLE_INPUT},
    {4, "3D", PIN_ROLE_INPUT},
    {5, "4D", PIN_ROLE_INPUT},
    {6, "5D", PIN_ROLE_INPUT},
    {7, "6D", PIN_ROLE_INPUT},
    {8, "7D", PIN_ROLE_INPUT},
    {9, "8D", PIN_ROLE_INPUT},
    {10, "GND", PIN_ROLE_GROUND},
    {11, "CLK", PIN_ROLE_INPUT},
    {12, "8Q", PIN_ROLE_OUTPUT},
    {13, "7Q", PIN_ROLE_OUTPUT},
    {14, "6Q", PIN_ROLE_OUTPUT},
    {15, "5Q", PIN_ROLE_OUTPUT},
    {16, "4Q", PIN_ROLE_OUTPUT},
    {17, "3Q", PIN_ROLE_OUTPUT},
    {18, "2Q", PIN_ROLE_OUTPUT},
    {19, "1Q", PIN_ROLE_OUTPUT},
    {20, "VCC", PIN_ROLE_POWER},
};

static const FfFamily FAMILIES_74_574[] = {
    {LOGIC_FAMILY_HC, 28000, 28000, 0},
    {LOGIC_FAMILY_HCT, 30000, 30000, 0},
    {LOGIC_FAMILY_AC, 6000, 5500, 0},
};

#define PARTS_74_574(X) \
    X("SN74HC574DW", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC574DB", LOGIC_FAMILY_HC, PACKAGE_TYPE_SSOP), \
    X("SN74HC574N", LOGIC_FAMILY_HC, PACKAGE_TYPE_PDIP), \
    X("SN74HC574NS", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP), \
    X("SN74HC574PW", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HCT574DW", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN74HCT574DB", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SSOP), \
    X("SN74HCT574N", LOGIC_FAMILY_HCT, PACKAGE_TYPE_PDIP), \
    X("SN74HCT574NS", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOP), \
    X("SN74HCT574PW", LOGIC_FAMILY_HCT, PACKAGE_TYPE_TSSOP), \
    X("SN74AC574DW", LOGIC_FAMILY_AC, PACKAGE_TYPE_SOIC), \
    X("SN74AC574DB", LOGIC_FAMILY_AC, PACKAGE_TYPE_SSOP), \
    X("SN74AC574N", LOGIC_FAMILY_AC, PACKAGE_TYPE_PDIP), \
    X("SN74AC574PW", LOGIC_FAMILY_AC, PACKAGE_TYPE_TSSOP)

FF_DEFINE_CHIP(FF_74_574, "74_574", "D flip-flop",
    "Octal positive-edge-triggered D flip-flop with 3-state outputs, 574 pinout",
    LAYOUT_OCT_574, PINS_OCT_574, FAMILIES_74_574, PARTS_74_574, 8)

/* ---- CMOS 4000 series --------------------------------------------------- */

/* CD4013B: SET and RESET are active high. Pins: 1 Q1 ... 14 VDD. */
static const FfUnit UNITS_4013B[] = {
    {FF_PIN(5), FF_PIN(3), FF_PIN(6), FF_PIN(4), FF_PIN(1), FF_PIN(2)},
    {FF_PIN(9), FF_PIN(11), FF_PIN(8), FF_PIN(10), FF_PIN(13), FF_PIN(12)},
};

static const FfLayout LAYOUT_4013B = {14, 2, UNITS_4013B, FF_NO_PIN, 1, 1, 0};

static const ChipPin PINS_4013B[] = {
    {1, "1Q", PIN_ROLE_OUTPUT},
    {2, "1QBAR", PIN_ROLE_OUTPUT},
    {3, "1CLK", PIN_ROLE_INPUT},
    {4, "1RESET", PIN_ROLE_INPUT},
    {5, "1D", PIN_ROLE_INPUT},
    {6, "1SET", PIN_ROLE_INPUT},
    {7, "VSS", PIN_ROLE_GROUND},
    {8, "2SET", PIN_ROLE_INPUT},
    {9, "2D", PIN_ROLE_INPUT},
    {10, "2RESET", PIN_ROLE_INPUT},
    {11, "2CLK", PIN_ROLE_INPUT},
    {12, "2QBAR", PIN_ROLE_OUTPUT},
    {13, "2Q", PIN_ROLE_OUTPUT},
    {14, "VDD", PIN_ROLE_POWER},
};

static const FfFamily FAMILIES_4013B[] = {
    {LOGIC_FAMILY_C, 150000, 150000, 0},
};

#define PARTS_4013B(X) \
    X("CD4013BE", LOGIC_FAMILY_C, PACKAGE_TYPE_PDIP), \
    X("CD4013BM96", LOGIC_FAMILY_C, PACKAGE_TYPE_SOIC), \
    X("CD4013BNSR", LOGIC_FAMILY_C, PACKAGE_TYPE_SOP), \
    X("CD4013BPWR", LOGIC_FAMILY_C, PACKAGE_TYPE_TSSOP)

FF_DEFINE_CHIP(FF_4013B, "4013B", "D flip-flop",
    "CMOS dual D-type flip-flop with active-high set and reset",
    LAYOUT_4013B, PINS_4013B, FAMILIES_4013B, PARTS_4013B, 2)

/* CD40175B: same pinout as 74_175, CMOS timing. */
static const FfFamily FAMILIES_40175B[] = {
    {LOGIC_FAMILY_C, 220000, 220000, 0},
};

#define PARTS_40175B(X) \
    X("CD40175BE", LOGIC_FAMILY_C, PACKAGE_TYPE_PDIP), \
    X("CD40175BM96", LOGIC_FAMILY_C, PACKAGE_TYPE_SOIC), \
    X("CD40175BNSR", LOGIC_FAMILY_C, PACKAGE_TYPE_SOP), \
    X("CD40175BPWR", LOGIC_FAMILY_C, PACKAGE_TYPE_TSSOP)

static const ChipPin PINS_40175B[] = {
    {1, "CLEAR", PIN_ROLE_INPUT},
    {2, "1Q", PIN_ROLE_OUTPUT},
    {3, "1QBAR", PIN_ROLE_OUTPUT},
    {4, "1D", PIN_ROLE_INPUT},
    {5, "2D", PIN_ROLE_INPUT},
    {6, "2QBAR", PIN_ROLE_OUTPUT},
    {7, "2Q", PIN_ROLE_OUTPUT},
    {8, "VSS", PIN_ROLE_GROUND},
    {9, "CLOCK", PIN_ROLE_INPUT},
    {10, "3Q", PIN_ROLE_OUTPUT},
    {11, "3QBAR", PIN_ROLE_OUTPUT},
    {12, "3D", PIN_ROLE_INPUT},
    {13, "4D", PIN_ROLE_INPUT},
    {14, "4QBAR", PIN_ROLE_OUTPUT},
    {15, "4Q", PIN_ROLE_OUTPUT},
    {16, "VDD", PIN_ROLE_POWER},
};

FF_DEFINE_CHIP(FF_40175B, "40175B", "D flip-flop",
    "CMOS quad D-type flip-flop with common clear",
    LAYOUT_QUAD175, PINS_40175B, FAMILIES_40175B, PARTS_40175B, 4)

/* ---- Single and dual small-package parts -------------------------------- */

/* 8-pin single: 1 CLK, 2 D, 3 QBAR, 4 GND, 5 Q, 6 CLR, 7 PRE, 8 VCC. */
static const FfUnit UNITS_1G74[] = {
    {FF_PIN(2), FF_PIN(1), FF_PIN(7), FF_PIN(6), FF_PIN(5), FF_PIN(3)},
};

static const FfLayout LAYOUT_1G74 = {8, 1, UNITS_1G74, FF_NO_PIN, 0, 0, 0};

static const ChipPin PINS_1G74[] = {
    {1, "CLK", PIN_ROLE_INPUT},
    {2, "D", PIN_ROLE_INPUT},
    {3, "QBAR", PIN_ROLE_OUTPUT},
    {4, "GND", PIN_ROLE_GROUND},
    {5, "Q", PIN_ROLE_OUTPUT},
    {6, "CLR", PIN_ROLE_INPUT},
    {7, "PRE", PIN_ROLE_INPUT},
    {8, "VCC", PIN_ROLE_POWER},
};

static const FfFamily FAMILIES_1G74[] = {
    {LOGIC_FAMILY_AUP, 4000, 4000, 1},
    {LOGIC_FAMILY_AUC, 1200, 1200, 1},
};

#define PARTS_1G74(X) \
    X("SN74AUP1G74DCU", LOGIC_FAMILY_AUP, PACKAGE_TYPE_VSSOP), \
    X("SN74AUC1G74DCT", LOGIC_FAMILY_AUC, PACKAGE_TYPE_SSOP), \
    X("SN74AUC1G74DCU", LOGIC_FAMILY_AUC, PACKAGE_TYPE_VSSOP)

FF_DEFINE_CHIP(FF_1G74, "1G74", "D flip-flop",
    "Single positive-edge-triggered D flip-flop with preset and clear",
    LAYOUT_1G74, PINS_1G74, FAMILIES_1G74, PARTS_1G74, 1)

/* 5-pin single: 1 D, 2 CLK, 3 GND, 4 Q, 5 VCC. */
static const FfUnit UNITS_1G79[] = {
    {FF_PIN(1), FF_PIN(2), FF_NO_PIN, FF_NO_PIN, FF_PIN(4), FF_NO_PIN},
};

static const FfLayout LAYOUT_1G79 = {5, 1, UNITS_1G79, FF_NO_PIN, 0, 0, 0};

static const ChipPin PINS_1G79[] = {
    {1, "D", PIN_ROLE_INPUT},
    {2, "CLK", PIN_ROLE_INPUT},
    {3, "GND", PIN_ROLE_GROUND},
    {4, "Q", PIN_ROLE_OUTPUT},
    {5, "VCC", PIN_ROLE_POWER},
};

static const FfFamily FAMILIES_1G79[] = {
    {LOGIC_FAMILY_LVC, 5000, 5000, 0},
    {LOGIC_FAMILY_AUP, 3000, 3000, 0},
};

#define PARTS_1G79(X) \
    X("SN74LVC1G79DBV", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOT23), \
    X("SN74LVC1G79DCK", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("SN74AUP1G79DBV", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SOT23), \
    X("SN74AUP1G79DCK", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SC70)

FF_DEFINE_CHIP(FF_1G79, "1G79", "D flip-flop",
    "Single positive-edge-triggered D flip-flop",
    LAYOUT_1G79, PINS_1G79, FAMILIES_1G79, PARTS_1G79, 1)

/* 6-pin single with CLR: 1 CLK, 2 GND, 3 D, 4 Q, 5 VCC, 6 CLR. */
static const FfUnit UNITS_1G175[] = {
    {FF_PIN(3), FF_PIN(1), FF_NO_PIN, FF_PIN(6), FF_PIN(4), FF_NO_PIN},
};

static const FfLayout LAYOUT_1G175 = {6, 1, UNITS_1G175, FF_NO_PIN, 0, 0, 0};

static const ChipPin PINS_1G175[] = {
    {1, "CLK", PIN_ROLE_INPUT},
    {2, "GND", PIN_ROLE_GROUND},
    {3, "D", PIN_ROLE_INPUT},
    {4, "Q", PIN_ROLE_OUTPUT},
    {5, "VCC", PIN_ROLE_POWER},
    {6, "CLR", PIN_ROLE_INPUT},
};

static const FfFamily FAMILIES_1G175[] = {
    {LOGIC_FAMILY_LVC, 5700, 5700, 0},
};

#define PARTS_1G175(X) \
    X("SN74LVC1G175DBV", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOT23), \
    X("SN74LVC1G175DCK", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70)

FF_DEFINE_CHIP(FF_1G175, "1G175", "D flip-flop",
    "Single positive-edge-triggered D flip-flop with clear",
    LAYOUT_1G175, PINS_1G175, FAMILIES_1G175, PARTS_1G175, 1)

/* 6-pin single with OE: 1 CLK, 2 GND, 3 D, 4 Q, 5 VCC, 6 OE. */
static const FfUnit UNITS_1G374[] = {
    {FF_PIN(3), FF_PIN(1), FF_NO_PIN, FF_NO_PIN, FF_PIN(4), FF_NO_PIN},
};

static const FfLayout LAYOUT_1G374 = {6, 1, UNITS_1G374, FF_PIN(6), 0, 0, 0};

static const ChipPin PINS_1G374[] = {
    {1, "CLK", PIN_ROLE_INPUT},
    {2, "GND", PIN_ROLE_GROUND},
    {3, "D", PIN_ROLE_INPUT},
    {4, "Q", PIN_ROLE_OUTPUT},
    {5, "VCC", PIN_ROLE_POWER},
    {6, "OE", PIN_ROLE_INPUT},
};

static const FfFamily FAMILIES_1G374[] = {
    {LOGIC_FAMILY_LVC, 6000, 6000, 0},
};

#define PARTS_1G374(X) \
    X("SN74LVC1G374DBV", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOT23), \
    X("SN74LVC1G374DCK", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70)

FF_DEFINE_CHIP(FF_1G374, "1G374", "D flip-flop",
    "Single positive-edge-triggered D flip-flop with 3-state output",
    LAYOUT_1G374, PINS_1G374, FAMILIES_1G374, PARTS_1G374, 1)

/* 8-pin dual, no PRE or CLR: 1 1CLK, 2 1D, 3 2Q, 4 GND, 5 2CLK, 6 2D, 7 1Q, 8 VCC. */
static const FfUnit UNITS_2G79[] = {
    {FF_PIN(2), FF_PIN(1), FF_NO_PIN, FF_NO_PIN, FF_PIN(7), FF_NO_PIN},
    {FF_PIN(6), FF_PIN(5), FF_NO_PIN, FF_NO_PIN, FF_PIN(3), FF_NO_PIN},
};

static const FfLayout LAYOUT_2G79 = {8, 2, UNITS_2G79, FF_NO_PIN, 0, 0, 0};

static const ChipPin PINS_2G79[] = {
    {1, "1CLK", PIN_ROLE_INPUT},
    {2, "1D", PIN_ROLE_INPUT},
    {3, "2Q", PIN_ROLE_OUTPUT},
    {4, "GND", PIN_ROLE_GROUND},
    {5, "2CLK", PIN_ROLE_INPUT},
    {6, "2D", PIN_ROLE_INPUT},
    {7, "1Q", PIN_ROLE_OUTPUT},
    {8, "VCC", PIN_ROLE_POWER},
};

static const FfFamily FAMILIES_2G79[] = {
    {LOGIC_FAMILY_AUP, 3000, 3000, 0},
};

#define PARTS_2G79(X) \
    X("SN74AUP2G79DCU", LOGIC_FAMILY_AUP, PACKAGE_TYPE_VSSOP)

FF_DEFINE_CHIP(FF_2G79, "2G79", "D flip-flop",
    "Dual positive-edge-triggered D flip-flop, no preset or clear",
    LAYOUT_2G79, PINS_2G79, FAMILIES_2G79, PARTS_2G79, 2)
