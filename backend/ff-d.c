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

    /*
     * An undriven PRE/CLR (Z) reads as not asserted, as a pulled-up input
     * would. Reading it as unknown let power-on, before supplies have
     * driven their nets, push the flip-flop to X.
     */
    if (value == LOGIC_Z) {
        return 0;
    }
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

/*
 * Logic-sim edges, as in Verilog posedge/negedge: any move into 1 from a
 * non-1 value (0, X or Z) is a rising edge, and any move into 0 from a
 * non-0 value is a falling edge. The start from an unknown or undriven clock
 * (X or Z to a defined level) therefore counts as an edge, which is what lets
 * a flip-flop clocked by its own output begin toggling.
 */
static int is_clock_edge(const FfLayout *layout, LogicValue old_value, LogicValue new_value) {
    if (layout->negative_edge) {
        return old_value != LOGIC_0 && new_value == LOGIC_0;
    }
    return old_value != LOGIC_1 && new_value == LOGIC_1;
}

static void ff_start(Sim *sim, SimInstance *instance, void *state_ptr) {
    FfState *state = state_ptr;
    size_t i;

    /*
     * Power-on state: Q low, QN high, until a clock edge or an asserted
     * PRE/CLR changes it. This is the flip-flop's own choice, not a sim rule.
     */
    for (i = 0; i < state->layout->unit_count; i++) {
        set_unit(state, i, LOGIC_0, LOGIC_1);
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
    {LOGIC_FAMILY_S, 6000, 6000, 0},       /* SN74S74: typical at 5 V, 25 C (15 pF) */
};

#define PARTS_74_74(X) \
    X("SN74HC74D", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC74DB", LOGIC_FAMILY_HC, PACKAGE_TYPE_SSOP), \
    X("SN74HC74N", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
    X("SN74HC74NS", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP), \
    X("SN74HC74PW", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HCT74D", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN74HCT74DB", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SSOP), \
    X("SN74HCT74N", LOGIC_FAMILY_HCT, PACKAGE_TYPE_DIP), \
    X("SN74HCT74NS", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOP), \
    X("SN74HCT74PW", LOGIC_FAMILY_HCT, PACKAGE_TYPE_TSSOP), \
    X("SN74AHC74D", LOGIC_FAMILY_AHC, PACKAGE_TYPE_SOIC), \
    X("SN74AHC74DB", LOGIC_FAMILY_AHC, PACKAGE_TYPE_SSOP), \
    X("SN74AHC74DGV", LOGIC_FAMILY_AHC, PACKAGE_TYPE_TVSOP), \
    X("SN74AHC74N", LOGIC_FAMILY_AHC, PACKAGE_TYPE_DIP), \
    X("SN74AHC74PW", LOGIC_FAMILY_AHC, PACKAGE_TYPE_TSSOP), \
    X("SN74AHCT74D", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SOIC), \
    X("SN74AHCT74DB", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SSOP), \
    X("SN74AHCT74DGV", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_TVSOP), \
    X("SN74AHCT74N", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_DIP), \
    X("SN74AHCT74NS", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SOP), \
    X("SN74AHCT74PW", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_TSSOP), \
    X("SN74AC74D", LOGIC_FAMILY_AC, PACKAGE_TYPE_SOIC), \
    X("SN74AC74DB", LOGIC_FAMILY_AC, PACKAGE_TYPE_SSOP), \
    X("SN74AC74N", LOGIC_FAMILY_AC, PACKAGE_TYPE_DIP), \
    X("SN74AC74PW", LOGIC_FAMILY_AC, PACKAGE_TYPE_TSSOP), \
    X("SN74ACT74D", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SOIC), \
    X("SN74ACT74DB", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SSOP), \
    X("SN74ACT74N", LOGIC_FAMILY_ACT, PACKAGE_TYPE_DIP), \
    X("SN74ACT74NS", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SOP), \
    X("SN74ACT74PW", LOGIC_FAMILY_ACT, PACKAGE_TYPE_TSSOP), \
    X("SN74F74D", LOGIC_FAMILY_F, PACKAGE_TYPE_SOIC), \
    X("SN74F74N", LOGIC_FAMILY_F, PACKAGE_TYPE_DIP), \
    X("SN74F74NS", LOGIC_FAMILY_F, PACKAGE_TYPE_SOP), \
    X("SN74LS74AD", LOGIC_FAMILY_LS, PACKAGE_TYPE_SOIC), \
    X("SN74LS74ADB", LOGIC_FAMILY_LS, PACKAGE_TYPE_SSOP), \
    X("SN74LS74AN", LOGIC_FAMILY_LS, PACKAGE_TYPE_DIP), \
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
    X("SN74HCS74PW", LOGIC_FAMILY_HCS, PACKAGE_TYPE_TSSOP), \
    X("CD54ACT74F3A", LOGIC_FAMILY_ACT, PACKAGE_TYPE_CDIP), \
    X("CD54HC74F", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("CD54HC74F3A", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("CD74ACT74E", LOGIC_FAMILY_ACT, PACKAGE_TYPE_DIP), \
    X("CD74ACT74M96", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SOIC), \
    X("CD74ACT74M96G4", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SOIC), \
    X("CD74HC74E", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
    X("CD74HC74EE4", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
    X("CD74HC74M96", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("CD74HC74M96G4", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("CD74HC74MT", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN54F74J", LOGIC_FAMILY_F, PACKAGE_TYPE_CDIP), \
    X("SN54HC74J", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("SN54LS74AJ", LOGIC_FAMILY_LS, PACKAGE_TYPE_CDIP), \
    X("SN74AC74BQAR", LOGIC_FAMILY_AC, PACKAGE_TYPE_WQFN), \
    X("SN74AC74DBR", LOGIC_FAMILY_AC, PACKAGE_TYPE_SSOP), \
    X("SN74AC74DR", LOGIC_FAMILY_AC, PACKAGE_TYPE_SOIC), \
    X("SN74AC74NE4", LOGIC_FAMILY_AC, PACKAGE_TYPE_DIP), \
    X("SN74AC74NSR", LOGIC_FAMILY_AC, PACKAGE_TYPE_SOP), \
    X("SN74AC74PWR", LOGIC_FAMILY_AC, PACKAGE_TYPE_TSSOP), \
    X("SN74AC74PWRG4", LOGIC_FAMILY_AC, PACKAGE_TYPE_TSSOP), \
    X("SN74ACT74BQAR", LOGIC_FAMILY_ACT, PACKAGE_TYPE_WQFN), \
    X("SN74ACT74DBR", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SSOP), \
    X("SN74ACT74DR", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SOIC), \
    X("SN74ACT74DR1G4", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SOIC), \
    X("SN74ACT74NE4", LOGIC_FAMILY_ACT, PACKAGE_TYPE_DIP), \
    X("SN74ACT74NSR", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SOP), \
    X("SN74ACT74PWR", LOGIC_FAMILY_ACT, PACKAGE_TYPE_TSSOP), \
    X("SN74ACT74PWRG4", LOGIC_FAMILY_ACT, PACKAGE_TYPE_TSSOP), \
    X("SN74AHC74BQAR", LOGIC_FAMILY_AHC, PACKAGE_TYPE_WQFN), \
    X("SN74AHC74DBR", LOGIC_FAMILY_AHC, PACKAGE_TYPE_SSOP), \
    X("SN74AHC74DGVR", LOGIC_FAMILY_AHC, PACKAGE_TYPE_TVSOP), \
    X("SN74AHC74DR", LOGIC_FAMILY_AHC, PACKAGE_TYPE_SOIC), \
    X("SN74AHC74DRG4", LOGIC_FAMILY_AHC, PACKAGE_TYPE_SOIC), \
    X("SN74AHC74NSR", LOGIC_FAMILY_AHC, PACKAGE_TYPE_SOP), \
    X("SN74AHC74PWR", LOGIC_FAMILY_AHC, PACKAGE_TYPE_TSSOP), \
    X("SN74AHC74QDRG4Q1", LOGIC_FAMILY_AHC, PACKAGE_TYPE_SOIC), \
    X("SN74AHC74QDRQ1", LOGIC_FAMILY_AHC, PACKAGE_TYPE_SOIC), \
    X("SN74AHC74QPWRG4Q1", LOGIC_FAMILY_AHC, PACKAGE_TYPE_TSSOP), \
    X("SN74AHC74QPWRQ1", LOGIC_FAMILY_AHC, PACKAGE_TYPE_TSSOP), \
    X("SN74AHC74QWBQARQ1", LOGIC_FAMILY_AHC, PACKAGE_TYPE_WQFN), \
    X("SN74AHC74RGYR", LOGIC_FAMILY_AHC, PACKAGE_TYPE_VQFN), \
    X("SN74AHC74RGYRG4", LOGIC_FAMILY_AHC, PACKAGE_TYPE_VQFN), \
    X("SN74AHCT74BQAR", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_WQFN), \
    X("SN74AHCT74DBR", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SSOP), \
    X("SN74AHCT74DGVR", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_TVSOP), \
    X("SN74AHCT74DR", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SOIC), \
    X("SN74AHCT74DRG4", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SOIC), \
    X("SN74AHCT74NSR", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SOP), \
    X("SN74AHCT74PWR", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_TSSOP), \
    X("SN74AHCT74PWRG4", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_TSSOP), \
    X("SN74AHCT74RGYR", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_VQFN), \
    X("SN74AUC74RGYR", LOGIC_FAMILY_AUC, PACKAGE_TYPE_VQFN), \
    X("SN74F74DR", LOGIC_FAMILY_F, PACKAGE_TYPE_SOIC), \
    X("SN74F74DRG4", LOGIC_FAMILY_F, PACKAGE_TYPE_SOIC), \
    X("SN74F74NE4", LOGIC_FAMILY_F, PACKAGE_TYPE_DIP), \
    X("SN74F74NSR", LOGIC_FAMILY_F, PACKAGE_TYPE_SOP), \
    X("SN74HC74DBR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SSOP), \
    X("SN74HC74DBRG4", LOGIC_FAMILY_HC, PACKAGE_TYPE_SSOP), \
    X("SN74HC74DR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC74DRG4", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC74NE4", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
    X("SN74HC74NSR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP), \
    X("SN74HC74NSRG4", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP), \
    X("SN74HC74PWR", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HC74PWR1G4", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HC74QDRG4Q1", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC74QDRQ1", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC74QPWRG4Q1", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HC74QPWRQ1", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HCS74BQAR", LOGIC_FAMILY_HCS, PACKAGE_TYPE_WQFN), \
    X("SN74HCS74DR", LOGIC_FAMILY_HCS, PACKAGE_TYPE_SOIC), \
    X("SN74HCS74PWR", LOGIC_FAMILY_HCS, PACKAGE_TYPE_TSSOP), \
    X("SN74HCS74QBQARQ1", LOGIC_FAMILY_HCS, PACKAGE_TYPE_WQFN), \
    X("SN74HCS74QDRQ1", LOGIC_FAMILY_HCS, PACKAGE_TYPE_SOIC), \
    X("SN74HCS74QPWRQ1", LOGIC_FAMILY_HCS, PACKAGE_TYPE_TSSOP), \
    X("SN74HCT74DBR", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SSOP), \
    X("SN74HCT74DR", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN74HCT74DRE4", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN74HCT74NE4", LOGIC_FAMILY_HCT, PACKAGE_TYPE_DIP), \
    X("SN74HCT74NSR", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOP), \
    X("SN74HCT74PWR", LOGIC_FAMILY_HCT, PACKAGE_TYPE_TSSOP), \
    X("SN74HCT74PWRG4", LOGIC_FAMILY_HCT, PACKAGE_TYPE_TSSOP), \
    X("SN74LS74ADBR", LOGIC_FAMILY_LS, PACKAGE_TYPE_SSOP), \
    X("SN74LS74ADR", LOGIC_FAMILY_LS, PACKAGE_TYPE_SOIC), \
    X("SN74LS74ADRG4", LOGIC_FAMILY_LS, PACKAGE_TYPE_SOIC), \
    X("SN74LS74ANE4", LOGIC_FAMILY_LS, PACKAGE_TYPE_DIP), \
    X("SN74LS74ANSR", LOGIC_FAMILY_LS, PACKAGE_TYPE_SOP), \
    X("SN74LS74ANSRG4", LOGIC_FAMILY_LS, PACKAGE_TYPE_SOP), \
    X("SN74LV74ADBR", LOGIC_FAMILY_LV, PACKAGE_TYPE_SSOP), \
    X("SN74LV74ADGVR", LOGIC_FAMILY_LV, PACKAGE_TYPE_TVSOP), \
    X("SN74LV74ADR", LOGIC_FAMILY_LV, PACKAGE_TYPE_SOIC), \
    X("SN74LV74ANSR", LOGIC_FAMILY_LV, PACKAGE_TYPE_SOP), \
    X("SN74LV74APWR", LOGIC_FAMILY_LV, PACKAGE_TYPE_TSSOP), \
    X("SN74LV74APWRG4", LOGIC_FAMILY_LV, PACKAGE_TYPE_TSSOP), \
    X("SN74LV74ARGYR", LOGIC_FAMILY_LV, PACKAGE_TYPE_VQFN), \
    X("SN74LV74ARGYRG4", LOGIC_FAMILY_LV, PACKAGE_TYPE_VQFN), \
    X("SN74LVC74ABQAR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_WQFN), \
    X("SN74LVC74ADBR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SSOP), \
    X("SN74LVC74ADBRG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SSOP), \
    X("SN74LVC74ADR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOIC), \
    X("SN74LVC74ADRE4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOIC), \
    X("SN74LVC74ADT", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOIC), \
    X("SN74LVC74ANSR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOP), \
    X("SN74LVC74ANSR1G4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOP), \
    X("SN74LVC74APWG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_TSSOP), \
    X("SN74LVC74APWR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_TSSOP), \
    X("SN74LVC74APWRE4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_TSSOP), \
    X("SN74LVC74APWRG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_TSSOP), \
    X("SN74LVC74APWT", LOGIC_FAMILY_LVC, PACKAGE_TYPE_TSSOP), \
    X("SN74LVC74APWTG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_TSSOP), \
    X("SN74LVC74ARGYR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_VQFN), \
    X("SN74LVC74ARGYRG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_VQFN), \
    X("SNJ54AC74J", LOGIC_FAMILY_AC, PACKAGE_TYPE_CDIP), \
    X("SNJ54AC74W", LOGIC_FAMILY_AC, PACKAGE_TYPE_CFP), \
    X("SNJ54ACT74J", LOGIC_FAMILY_ACT, PACKAGE_TYPE_CDIP), \
    X("SNJ54ACT74W", LOGIC_FAMILY_ACT, PACKAGE_TYPE_CFP), \
    X("SNJ54AHC74J", LOGIC_FAMILY_AHC, PACKAGE_TYPE_CDIP), \
    X("SNJ54AHC74W", LOGIC_FAMILY_AHC, PACKAGE_TYPE_CFP), \
    X("SNJ54AHCT74J", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_CDIP), \
    X("SNJ54AHCT74W", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_CFP), \
    X("SNJ54F74J", LOGIC_FAMILY_F, PACKAGE_TYPE_CDIP), \
    X("SNJ54F74W", LOGIC_FAMILY_F, PACKAGE_TYPE_CFP), \
    X("SNJ54HC74J", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("SNJ54HC74W", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP), \
    X("SNJ54LS74AJ", LOGIC_FAMILY_LS, PACKAGE_TYPE_CDIP), \
    X("SNJ54LS74AW", LOGIC_FAMILY_LS, PACKAGE_TYPE_CFP), \
    X("SNJ54LVC74AJ", LOGIC_FAMILY_LVC, PACKAGE_TYPE_CDIP), \
    X("SNJ54LVC74AW", LOGIC_FAMILY_LVC, PACKAGE_TYPE_CFP), \
    X("5962-8405601VCA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("5962-8405601VDA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP), \
    X("5962-8752501MCA", LOGIC_FAMILY_ACT, PACKAGE_TYPE_CDIP), \
    X("5962-8752501MDA", LOGIC_FAMILY_ACT, PACKAGE_TYPE_CFP), \
    X("5962-8852001CA", LOGIC_FAMILY_AC, PACKAGE_TYPE_CDIP), \
    X("5962-8852001DA", LOGIC_FAMILY_AC, PACKAGE_TYPE_CFP), \
    X("5962-8852001VDA", LOGIC_FAMILY_AC, PACKAGE_TYPE_CFP), \
    X("5962-9686001QCA", LOGIC_FAMILY_AHC, PACKAGE_TYPE_CDIP), \
    X("5962-9686001QDA", LOGIC_FAMILY_AHC, PACKAGE_TYPE_CFP), \
    X("5962-9686101QCA", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_CDIP), \
    X("5962-9686101QDA", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_CFP), \
    X("5962-9759201QCA", LOGIC_FAMILY_F, PACKAGE_TYPE_CDIP), \
    X("5962-9759201QDA", LOGIC_FAMILY_F, PACKAGE_TYPE_CFP), \
    X("5962-9761601QCA", LOGIC_FAMILY_LVC, PACKAGE_TYPE_CDIP), \
    X("5962-9761601QDA", LOGIC_FAMILY_LVC, PACKAGE_TYPE_CFP), \
    X("5962-9761601VDA", LOGIC_FAMILY_LVC, PACKAGE_TYPE_CFP), \
    X("8405601CA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("8405601DA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP), \
    X("CD74ACT74M", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SOIC), \
    X("CD74HC74M", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN54S74J", LOGIC_FAMILY_S, PACKAGE_TYPE_CDIP), \
    X("SN74HC74DT", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC74PWT", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74LV74APWT", LOGIC_FAMILY_LV, PACKAGE_TYPE_TSSOP), \
    X("SN74S74D", LOGIC_FAMILY_S, PACKAGE_TYPE_SOIC), \
    X("SN74S74N", LOGIC_FAMILY_S, PACKAGE_TYPE_DIP), \
    X("SNJ54S74J", LOGIC_FAMILY_S, PACKAGE_TYPE_CDIP), \
    X("SNJ54S74W", LOGIC_FAMILY_S, PACKAGE_TYPE_CFP), \
    X("JM38510/34101BCA", LOGIC_FAMILY_F, PACKAGE_TYPE_CDIP), \
    X("JM38510/34101BDA", LOGIC_FAMILY_F, PACKAGE_TYPE_CFP), \
    X("JM38510/65302BCA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("JM38510/65302BDA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP), \
    X("JM38510/65352BCA", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP), \
    X("JM38510/65352BDA", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CFP), \
    X("M38510/34101BCA", LOGIC_FAMILY_F, PACKAGE_TYPE_CDIP), \
    X("M38510/34101BDA", LOGIC_FAMILY_F, PACKAGE_TYPE_CFP), \
    X("M38510/65302BCA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("M38510/65302BDA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP), \
    X("M38510/65352BCA", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP), \
    X("M38510/65352BDA", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CFP)

FF_DEFINE_CHIP(FF_74_74, "74_74", "D flip-flop",
    "Dual positive-edge-triggered D flip-flop with preset and clear",
    LAYOUT_DUAL74, PINS_DUAL74, FAMILIES_74_74, PARTS_74_74, 2)

static const FfFamily FAMILIES_74_72[] = {
    {LOGIC_FAMILY_HCS, 8000, 8000, 0},
};

#define PARTS_74_72(X) \
    X("SN74HCS72D", LOGIC_FAMILY_HCS, PACKAGE_TYPE_SOIC), \
    X("SN74HCS72PW", LOGIC_FAMILY_HCS, PACKAGE_TYPE_TSSOP), \
    X("SN74HCS72DR", LOGIC_FAMILY_HCS, PACKAGE_TYPE_SOIC), \
    X("SN74HCS72PWR", LOGIC_FAMILY_HCS, PACKAGE_TYPE_TSSOP), \
    X("SN74HCS72QDRQ1", LOGIC_FAMILY_HCS, PACKAGE_TYPE_SOIC), \
    X("SN74HCS72QPWRQ1", LOGIC_FAMILY_HCS, PACKAGE_TYPE_TSSOP)

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
    {LOGIC_FAMILY_HCT, 40000, 40000, 0},   /* CD74HCT174: max at 4.5 V, 25 C; no typical there */
};

#define PARTS_74_174(X) \
    X("SN74HC174D", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC174DB", LOGIC_FAMILY_HC, PACKAGE_TYPE_SSOP), \
    X("SN74HC174N", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
    X("SN74HC174NS", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP), \
    X("SN74HC174PW", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("CD54HC174F", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("CD54HC174F3A", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("CD74HC174E", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
    X("CD74HC174M96", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("CD74HC174M96E4", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN54HC174J", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("SN74HC174DBR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SSOP), \
    X("SN74HC174DR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC174DRG4", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC174NE4", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
    X("SN74HC174NSR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP), \
    X("SN74HC174PWR", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HC174PWRG4", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SNJ54HC174J", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("SNJ54HC174W", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP), \
    X("5962-8974301EA", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP), \
    X("8407301EA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("8407301FA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP), \
    X("CD54HCT174F", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP), \
    X("CD54HCT174F3A", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP), \
    X("CD74HC174M", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("CD74HCT174E", LOGIC_FAMILY_HCT, PACKAGE_TYPE_DIP), \
    X("CD74HCT174M", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("CD74HCT174M96", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("CD74HCT174MT", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("JM38510/65307BEA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("M38510/65307BEA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP)

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
    {LOGIC_FAMILY_HCT, 33000, 33000, 0},   /* CD74HCT175: max at 4.5 V, 25 C; no typical there */
};

#define PARTS_74_175(X) \
    X("SN74HC175D", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC175DB", LOGIC_FAMILY_HC, PACKAGE_TYPE_SSOP), \
    X("SN74HC175N", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
    X("SN74HC175NS", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP), \
    X("SN74HC175PW", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("CD54HC175F3A", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("CD74HC175E", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
    X("CD74HC175EE4", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
    X("CD74HC175M96", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN54HC175J", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("SN74HC175DBR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SSOP), \
    X("SN74HC175DR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC175DRG4", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC175NE4", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
    X("SN74HC175NSR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP), \
    X("SN74HC175PWR", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SNJ54HC175J", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("SNJ54HC175W", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP), \
    X("5962-8970101EA", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP), \
    X("8408901EA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("8408901FA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP), \
    X("CD54HCT175F3A", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP), \
    X("CD74HC175M", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("CD74HCT175E", LOGIC_FAMILY_HCT, PACKAGE_TYPE_DIP), \
    X("CD74HCT175EE4", LOGIC_FAMILY_HCT, PACKAGE_TYPE_DIP), \
    X("CD74HCT175M", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("CD74HCT175M96", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("CD74HCT175M96G4", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("CD74HCT175MT", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("JM38510/65308BEA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("M38510/65308BEA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP)

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
    {LOGIC_FAMILY_HCT, 12000, 12000, 0},
    {LOGIC_FAMILY_ACT, 12300, 12300, 0},
    {LOGIC_FAMILY_AC, 12300, 12300, 0},
};

#define PARTS_74_273(X) \
    X("SN74HC273DW", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC273DB", LOGIC_FAMILY_HC, PACKAGE_TYPE_SSOP), \
    X("SN74HC273N", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
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
    X("SN74AHCT273N", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_DIP), \
    X("SN74AHCT273NS", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SOP), \
    X("SN74AHCT273PW", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_TSSOP), \
    X("CD54AC273F3A", LOGIC_FAMILY_AC, PACKAGE_TYPE_CDIP), \
    X("CD54ACT273F3A", LOGIC_FAMILY_ACT, PACKAGE_TYPE_CDIP), \
    X("CD54HC273F", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("CD54HC273F3A", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("CD54HCT273F", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP), \
    X("CD54HCT273F3A", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP), \
    X("CD74AC273E", LOGIC_FAMILY_AC, PACKAGE_TYPE_DIP), \
    X("CD74AC273M96", LOGIC_FAMILY_AC, PACKAGE_TYPE_SOIC), \
    X("CD74ACT273E", LOGIC_FAMILY_ACT, PACKAGE_TYPE_DIP), \
    X("CD74ACT273EE4", LOGIC_FAMILY_ACT, PACKAGE_TYPE_DIP), \
    X("CD74ACT273M96", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SOIC), \
    X("CD74ACT273M96E4", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SOIC), \
    X("CD74ACT273PWR", LOGIC_FAMILY_ACT, PACKAGE_TYPE_TSSOP), \
    X("CD74ACT273SM96", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SSOP), \
    X("CD74HC273E", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
    X("CD74HC273M96", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("CD74HC273M96E4", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("CD74HCT273E", LOGIC_FAMILY_HCT, PACKAGE_TYPE_DIP), \
    X("CD74HCT273EE4", LOGIC_FAMILY_HCT, PACKAGE_TYPE_DIP), \
    X("CD74HCT273M96", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN54HC273J", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("SN74AHC273DBR", LOGIC_FAMILY_AHC, PACKAGE_TYPE_SSOP), \
    X("SN74AHC273DGSR", LOGIC_FAMILY_AHC, PACKAGE_TYPE_VSSOP), \
    X("SN74AHC273DGVR", LOGIC_FAMILY_AHC, PACKAGE_TYPE_TVSOP), \
    X("SN74AHC273DWR", LOGIC_FAMILY_AHC, PACKAGE_TYPE_SOIC), \
    X("SN74AHC273N", LOGIC_FAMILY_AHC, PACKAGE_TYPE_DIP), \
    X("SN74AHC273NSR", LOGIC_FAMILY_AHC, PACKAGE_TYPE_SOP), \
    X("SN74AHC273PWR", LOGIC_FAMILY_AHC, PACKAGE_TYPE_TSSOP), \
    X("SN74AHC273RKSR", LOGIC_FAMILY_AHC, PACKAGE_TYPE_VQFN), \
    X("SN74AHCT273DBR", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SSOP), \
    X("SN74AHCT273DGSR", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_VSSOP), \
    X("SN74AHCT273DWR", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SOIC), \
    X("SN74AHCT273NSR", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SOP), \
    X("SN74AHCT273PWR", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_TSSOP), \
    X("SN74AHCT273RKSR", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_VQFN), \
    X("SN74HC273DBR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SSOP), \
    X("SN74HC273DWR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC273DWRE4", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC273DWRG4", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC273NE4", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
    X("SN74HC273NSR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP), \
    X("SN74HC273PWR", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HC273PWRG4", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HCT273DBR", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SSOP), \
    X("SN74HCT273DWR", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN74HCT273DWRG4", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN74HCT273N", LOGIC_FAMILY_HCT, PACKAGE_TYPE_DIP), \
    X("SN74HCT273NSR", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOP), \
    X("SN74HCT273PWR", LOGIC_FAMILY_HCT, PACKAGE_TYPE_TSSOP), \
    X("SNJ54AHC273J", LOGIC_FAMILY_AHC, PACKAGE_TYPE_CDIP), \
    X("SNJ54AHC273W", LOGIC_FAMILY_AHC, PACKAGE_TYPE_CFP), \
    X("SNJ54HC273FK", LOGIC_FAMILY_HC, PACKAGE_TYPE_LCCC), \
    X("SNJ54HC273J", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("SNJ54HC273W", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP), \
    X("5962-8409901VRA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("5962-8409901VSA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP), \
    X("5962-8772501RA", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP), \
    X("5962-9853001QRA", LOGIC_FAMILY_AHC, PACKAGE_TYPE_CDIP), \
    X("5962-9853001QSA", LOGIC_FAMILY_AHC, PACKAGE_TYPE_CFP), \
    X("84099012A", LOGIC_FAMILY_HC, PACKAGE_TYPE_LCCC), \
    X("8409901RA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("8409901SA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP), \
    X("CD74ACT273M", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SOIC), \
    X("CD74ACT273PW", LOGIC_FAMILY_ACT, PACKAGE_TYPE_TSSOP), \
    X("CD74HC273M", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("CD74HCT273M", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN74HC273PWT", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HCT273DW", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN74HCT273PW", LOGIC_FAMILY_HCT, PACKAGE_TYPE_TSSOP), \
    X("SN74HCT273PWT", LOGIC_FAMILY_HCT, PACKAGE_TYPE_TSSOP), \
    X("JM38510/65601BRA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("JM38510/65601BSA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP), \
    X("M38510/65601BRA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("M38510/65601BSA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP)

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
    X("SN74HC374N", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
    X("SN74HC374NS", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP), \
    X("SN74HC374PW", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HCT374DW", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN74HCT374DB", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SSOP), \
    X("SN74HCT374N", LOGIC_FAMILY_HCT, PACKAGE_TYPE_DIP), \
    X("SN74HCT374NS", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOP), \
    X("SN74HCT374PW", LOGIC_FAMILY_HCT, PACKAGE_TYPE_TSSOP), \
    X("SN74AHCT374DW", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SOIC), \
    X("SN74AHCT374DB", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SSOP), \
    X("SN74AHCT374N", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_DIP), \
    X("SN74AHCT374PW", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_TSSOP), \
    X("CD54HC374F3A", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("CD54HCT374F3A", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP), \
    X("CD74HC374E", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
    X("CD74HC374M96", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("CD74HC374M96E4", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("CD74HCT374E", LOGIC_FAMILY_HCT, PACKAGE_TYPE_DIP), \
    X("CD74HCT374EE4", LOGIC_FAMILY_HCT, PACKAGE_TYPE_DIP), \
    X("CD74HCT374M96", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN54HC374J", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("SN54HCT374J", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP), \
    X("SN74AHCT374DBR", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SSOP), \
    X("SN74AHCT374DGSR", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_VSSOP), \
    X("SN74AHCT374DWR", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_SOIC), \
    X("SN74AHCT374PWR", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_TSSOP), \
    X("SN74AHCT374RKSR", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_VQFN), \
    X("SN74HC374DBR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SSOP), \
    X("SN74HC374DWR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC374DWRG4", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC374NE4", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
    X("SN74HC374NSR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP), \
    X("SN74HC374PWR", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HCT374DBR", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SSOP), \
    X("SN74HCT374DWR", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN74HCT374DWRE4", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN74HCT374NSR", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOP), \
    X("SN74HCT374PWR", LOGIC_FAMILY_HCT, PACKAGE_TYPE_TSSOP), \
    X("SNJ54AHCT374FK", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_LCCC), \
    X("SNJ54AHCT374J", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_CDIP), \
    X("SNJ54AHCT374W", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_CFP), \
    X("SNJ54HC374FK", LOGIC_FAMILY_HC, PACKAGE_TYPE_LCCC), \
    X("SNJ54HC374J", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("SNJ54HC374W", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP), \
    X("SNJ54HCT374FK", LOGIC_FAMILY_HCT, PACKAGE_TYPE_LCCC), \
    X("SNJ54HCT374J", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP), \
    X("5962-8407101VRA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("5962-8407101VSA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP), \
    X("5962-9686501Q2A", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_LCCC), \
    X("5962-9686501QRA", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_CDIP), \
    X("5962-9686501QSA", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_CFP), \
    X("84071012A", LOGIC_FAMILY_HC, PACKAGE_TYPE_LCCC), \
    X("8407101RA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("8407101SA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP), \
    X("85507012A", LOGIC_FAMILY_HCT, PACKAGE_TYPE_LCCC), \
    X("8550701RA", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP), \
    X("CD74HC374M", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("CD74HCT374M", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN74HC374PWT", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HCT374PWT", LOGIC_FAMILY_HCT, PACKAGE_TYPE_TSSOP), \
    X("JM38510/65602BRA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("JM38510/65652BRA", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP), \
    X("M38510/65602BRA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("M38510/65652BRA", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP)

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
    X("SN74HC574N", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
    X("SN74HC574NS", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP), \
    X("SN74HC574PW", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HCT574DW", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN74HCT574DB", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SSOP), \
    X("SN74HCT574N", LOGIC_FAMILY_HCT, PACKAGE_TYPE_DIP), \
    X("SN74HCT574NS", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOP), \
    X("SN74HCT574PW", LOGIC_FAMILY_HCT, PACKAGE_TYPE_TSSOP), \
    X("SN74AC574DW", LOGIC_FAMILY_AC, PACKAGE_TYPE_SOIC), \
    X("SN74AC574DB", LOGIC_FAMILY_AC, PACKAGE_TYPE_SSOP), \
    X("SN74AC574N", LOGIC_FAMILY_AC, PACKAGE_TYPE_DIP), \
    X("SN74AC574PW", LOGIC_FAMILY_AC, PACKAGE_TYPE_TSSOP), \
    X("CD54HC574F", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("CD54HC574F3A", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("CD54HCT574F", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP), \
    X("CD54HCT574F3A", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP), \
    X("CD74HC574E", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
    X("CD74HC574M96", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("CD74HCT574E", LOGIC_FAMILY_HCT, PACKAGE_TYPE_DIP), \
    X("CD74HCT574M96", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("CD74HCT574PWR", LOGIC_FAMILY_HCT, PACKAGE_TYPE_TSSOP), \
    X("SN54HC574J", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("SN74AC574DBR", LOGIC_FAMILY_AC, PACKAGE_TYPE_SSOP), \
    X("SN74AC574DWR", LOGIC_FAMILY_AC, PACKAGE_TYPE_SOIC), \
    X("SN74AC574PWR", LOGIC_FAMILY_AC, PACKAGE_TYPE_TSSOP), \
    X("SN74AC574PWRG4", LOGIC_FAMILY_AC, PACKAGE_TYPE_TSSOP), \
    X("SN74HC574APWR", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HC574DBR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SSOP), \
    X("SN74HC574DBRG4", LOGIC_FAMILY_HC, PACKAGE_TYPE_SSOP), \
    X("SN74HC574DWR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("SN74HC574NE4", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP), \
    X("SN74HC574NSR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP), \
    X("SN74HC574PWR", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HC574PWRG4", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HCT574DBR", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SSOP), \
    X("SN74HCT574DGSR", LOGIC_FAMILY_HCT, PACKAGE_TYPE_VSSOP), \
    X("SN74HCT574DWR", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN74HCT574DWRG4", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN74HCT574NE4", LOGIC_FAMILY_HCT, PACKAGE_TYPE_DIP), \
    X("SN74HCT574NSR", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOP), \
    X("SN74HCT574PWR", LOGIC_FAMILY_HCT, PACKAGE_TYPE_TSSOP), \
    X("SNJ54AC574FK", LOGIC_FAMILY_AC, PACKAGE_TYPE_LCCC), \
    X("SNJ54AC574J", LOGIC_FAMILY_AC, PACKAGE_TYPE_CDIP), \
    X("SNJ54AC574W", LOGIC_FAMILY_AC, PACKAGE_TYPE_CFP), \
    X("SNJ54HC574FK", LOGIC_FAMILY_HC, PACKAGE_TYPE_LCCC), \
    X("SNJ54HC574J", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("SNJ54HC574W", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP), \
    X("5962-8974201RA", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP), \
    X("5962-9677301Q2A", LOGIC_FAMILY_AC, PACKAGE_TYPE_LCCC), \
    X("5962-9677301QRA", LOGIC_FAMILY_AC, PACKAGE_TYPE_CDIP), \
    X("5962-9677301QSA", LOGIC_FAMILY_AC, PACKAGE_TYPE_CFP), \
    X("CD74HC574M", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC), \
    X("CD74HCT574M", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC), \
    X("SN74HC574PWT", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP), \
    X("SN74HCT574PWT", LOGIC_FAMILY_HCT, PACKAGE_TYPE_TSSOP), \
    X("JM38510/65604BRA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP), \
    X("M38510/65604BRA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP)

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
    X("CD4013BE", LOGIC_FAMILY_C, PACKAGE_TYPE_DIP), \
    X("CD4013BM96", LOGIC_FAMILY_C, PACKAGE_TYPE_SOIC), \
    X("CD4013BNSR", LOGIC_FAMILY_C, PACKAGE_TYPE_SOP), \
    X("CD4013BPWR", LOGIC_FAMILY_C, PACKAGE_TYPE_TSSOP), \
    X("CD4013BEE4", LOGIC_FAMILY_C, PACKAGE_TYPE_DIP), \
    X("CD4013BF", LOGIC_FAMILY_C, PACKAGE_TYPE_CDIP), \
    X("CD4013BF3A", LOGIC_FAMILY_C, PACKAGE_TYPE_CDIP), \
    X("CD4013BM", LOGIC_FAMILY_C, PACKAGE_TYPE_SOIC), \
    X("CD4013BM96E4", LOGIC_FAMILY_C, PACKAGE_TYPE_SOIC), \
    X("CD4013BM96G4", LOGIC_FAMILY_C, PACKAGE_TYPE_SOIC), \
    X("CD4013BME4", LOGIC_FAMILY_C, PACKAGE_TYPE_SOIC), \
    X("CD4013BMG4", LOGIC_FAMILY_C, PACKAGE_TYPE_SOIC), \
    X("CD4013BPWRE4", LOGIC_FAMILY_C, PACKAGE_TYPE_TSSOP), \
    X("CD4013BPWRG4", LOGIC_FAMILY_C, PACKAGE_TYPE_TSSOP), \
    X("CD4013BMT", LOGIC_FAMILY_C, PACKAGE_TYPE_SOIC), \
    X("CD4013BPW", LOGIC_FAMILY_C, PACKAGE_TYPE_TSSOP), \
    X("JM38510/05151BCA", LOGIC_FAMILY_C, PACKAGE_TYPE_CDIP), \
    X("M38510/05151BCA", LOGIC_FAMILY_C, PACKAGE_TYPE_CDIP)

FF_DEFINE_CHIP(FF_4013B, "4013B", "D flip-flop",
    "CMOS dual D-type flip-flop with active-high set and reset",
    LAYOUT_4013B, PINS_4013B, FAMILIES_4013B, PARTS_4013B, 2)

/* CD40175B: same pinout as 74_175, CMOS timing. */
static const FfFamily FAMILIES_40175B[] = {
    {LOGIC_FAMILY_C, 220000, 220000, 0},
};

#define PARTS_40175B(X) \
    X("CD40175BE", LOGIC_FAMILY_C, PACKAGE_TYPE_DIP), \
    X("CD40175BM96", LOGIC_FAMILY_C, PACKAGE_TYPE_SOIC), \
    X("CD40175BNSR", LOGIC_FAMILY_C, PACKAGE_TYPE_SOP), \
    X("CD40175BPWR", LOGIC_FAMILY_C, PACKAGE_TYPE_TSSOP), \
    X("CD40175BEE4", LOGIC_FAMILY_C, PACKAGE_TYPE_DIP), \
    X("CD40175BF3A", LOGIC_FAMILY_C, PACKAGE_TYPE_CDIP), \
    X("CD40175BM", LOGIC_FAMILY_C, PACKAGE_TYPE_SOIC), \
    X("CD40175BMT", LOGIC_FAMILY_C, PACKAGE_TYPE_SOIC)

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
    {LOGIC_FAMILY_LVC, 5900, 5900, 0},
};

#define PARTS_1G74(X) \
    X("SN74AUP1G74DCU", LOGIC_FAMILY_AUP, PACKAGE_TYPE_VSSOP), \
    X("SN74AUC1G74DCT", LOGIC_FAMILY_AUC, PACKAGE_TYPE_SSOP), \
    X("SN74AUC1G74DCU", LOGIC_FAMILY_AUC, PACKAGE_TYPE_VSSOP), \
    X("SN74AUC1G74DCTR", LOGIC_FAMILY_AUC, PACKAGE_TYPE_SSOP), \
    X("SN74AUC1G74DCUR", LOGIC_FAMILY_AUC, PACKAGE_TYPE_VSSOP), \
    X("SN74AUC1G74DCURE4", LOGIC_FAMILY_AUC, PACKAGE_TYPE_VSSOP), \
    X("SN74AUC1G74DCURG4", LOGIC_FAMILY_AUC, PACKAGE_TYPE_VSSOP), \
    X("SN74AUC1G74RSER", LOGIC_FAMILY_AUC, PACKAGE_TYPE_UQFN), \
    X("SN74AUC1G74YZPR", LOGIC_FAMILY_AUC, PACKAGE_TYPE_DSBGA), \
    X("SN74AUP1G74DCUR", LOGIC_FAMILY_AUP, PACKAGE_TYPE_VSSOP), \
    X("SN74AUP1G74DCURG4", LOGIC_FAMILY_AUP, PACKAGE_TYPE_VSSOP), \
    X("SN74AUP1G74DQER", LOGIC_FAMILY_AUP, PACKAGE_TYPE_X2SON), \
    X("SN74AUP1G74RSER", LOGIC_FAMILY_AUP, PACKAGE_TYPE_UQFN), \
    X("SN74AUP1G74YFPR", LOGIC_FAMILY_AUP, PACKAGE_TYPE_DSBGA), \
    X("SN74AUP1G74YZPR", LOGIC_FAMILY_AUP, PACKAGE_TYPE_DSBGA), \
    X("SN74LVC1G74DCTR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SSOP), \
    X("SN74LVC1G74DCUR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_VSSOP), \
    X("SN74LVC1G74DCURG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_VSSOP), \
    X("SN74LVC1G74DCUT", LOGIC_FAMILY_LVC, PACKAGE_TYPE_VSSOP), \
    X("SN74LVC1G74DQER", LOGIC_FAMILY_LVC, PACKAGE_TYPE_X2SON), \
    X("SN74LVC1G74DQERG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_X2SON), \
    X("SN74LVC1G74RSE2", LOGIC_FAMILY_LVC, PACKAGE_TYPE_UQFN), \
    X("SN74LVC1G74RSE2G4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_UQFN), \
    X("SN74LVC1G74RSER", LOGIC_FAMILY_LVC, PACKAGE_TYPE_UQFN), \
    X("SN74LVC1G74RSERG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_UQFN)

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
    X("SN74AUP1G79DCK", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SC70), \
    X("SN74AUP1G79DBVR", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SOT23), \
    X("SN74AUP1G79DBVT", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SOT23), \
    X("SN74AUP1G79DBVTG4", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SOT23), \
    X("SN74AUP1G79DCKR", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SC70), \
    X("SN74AUP1G79DCKT", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SC70), \
    X("SN74AUP1G79DPWR", LOGIC_FAMILY_AUP, PACKAGE_TYPE_X2SON), \
    X("SN74AUP1G79DRLR", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SOT5X3), \
    X("SN74LVC1G79DBVR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOT23), \
    X("SN74LVC1G79DBVT", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOT23), \
    X("SN74LVC1G79DBVTG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOT23), \
    X("SN74LVC1G79DCKR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("SN74LVC1G79DCKRG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("SN74LVC1G79DCKT", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("SN74LVC1G79DCKTG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("SN74LVC1G79DRLR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOT5X3), \
    X("SN74LVC1G79QDCKRQ1", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("SN74LVC1G79QDCKTQ1", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("SN74LVC1G79YZPR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_DSBGA)

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
    X("SN74LVC1G175DCK", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("SN74LVC1G175DBVR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOT23), \
    X("SN74LVC1G175DBVT", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOT23), \
    X("SN74LVC1G175DBVTG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOT23), \
    X("SN74LVC1G175DCKR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("SN74LVC1G175DCKT", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("SN74LVC1G175DRYR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SON), \
    X("SN74LVC1G175YZPR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_DSBGA), \
    X("74LVC1G175DBVRE4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOT23), \
    X("74LVC1G175DBVRG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOT23), \
    X("74LVC1G175DCKRG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("74LVC1G175DCKTG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("CLVC1G175MDCKREP", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("CLVC1G175MDCKREPG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("V62/06633-01XE", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70)

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
    X("SN74LVC1G374DCK", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("SN74LVC1G374DBVR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOT23), \
    X("SN74LVC1G374DCKR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("SN74LVC1G374YZPR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_DSBGA), \
    X("74LVC1G374DCKRG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70)

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
    X("SN74AUP2G79DCU", LOGIC_FAMILY_AUP, PACKAGE_TYPE_VSSOP), \
    X("SN74AUP2G79DCUR", LOGIC_FAMILY_AUP, PACKAGE_TYPE_VSSOP), \
    X("SN74AUP2G79DCURG4", LOGIC_FAMILY_AUP, PACKAGE_TYPE_VSSOP), \
    X("SN74AUP2G79DQER", LOGIC_FAMILY_AUP, PACKAGE_TYPE_X2SON), \
    X("SN74AUP2G79RSER", LOGIC_FAMILY_AUP, PACKAGE_TYPE_UQFN)

FF_DEFINE_CHIP(FF_2G79, "2G79", "D flip-flop",
    "Dual positive-edge-triggered D flip-flop, no preset or clear",
    LAYOUT_2G79, PINS_2G79, FAMILIES_2G79, PARTS_2G79, 2)

/* ---- Alternate packages ------------------------------------------------- */
/*
 * Same flip-flops in packages with a different pin count. Wiring is the
 * base layout remapped to the package pins from the datasheet pin diagrams:
 * 14-pin to 20-pin LCCC (SN54HC74 FK column), 16-pin to 20-pin LCCC (SN54HC174
 * and SN54HC175 FK drawings), and the 6-pin SON of the 1G79 (NC on pin 5).
 */

static const FfUnit UNITS_DUAL74_FK[] = {
    /* d, clk, pre, clr, q, qn */
    {FF_PIN(3), FF_PIN(4), FF_PIN(6), FF_PIN(2), FF_PIN(8), FF_PIN(9)},
    {FF_PIN(18), FF_PIN(16), FF_PIN(14), FF_PIN(19), FF_PIN(13), FF_PIN(12)},
};
static const FfLayout LAYOUT_DUAL74_FK = {20, 2, UNITS_DUAL74_FK, FF_NO_PIN, 0, 0, 0};
static const ChipPin PINS_DUAL74_FK[] = {
    {1, "NC", PIN_ROLE_NO_CONNECT},
    {2, "1CLR", PIN_ROLE_INPUT},
    {3, "1D", PIN_ROLE_INPUT},
    {4, "1CLK", PIN_ROLE_INPUT},
    {5, "NC", PIN_ROLE_NO_CONNECT},
    {6, "1PRE", PIN_ROLE_INPUT},
    {7, "NC", PIN_ROLE_NO_CONNECT},
    {8, "1Q", PIN_ROLE_OUTPUT},
    {9, "1QBAR", PIN_ROLE_OUTPUT},
    {10, "GND", PIN_ROLE_GROUND},
    {11, "NC", PIN_ROLE_NO_CONNECT},
    {12, "2QBAR", PIN_ROLE_OUTPUT},
    {13, "2Q", PIN_ROLE_OUTPUT},
    {14, "2PRE", PIN_ROLE_INPUT},
    {15, "NC", PIN_ROLE_NO_CONNECT},
    {16, "2CLK", PIN_ROLE_INPUT},
    {17, "NC", PIN_ROLE_NO_CONNECT},
    {18, "2D", PIN_ROLE_INPUT},
    {19, "2CLR", PIN_ROLE_INPUT},
    {20, "VCC", PIN_ROLE_POWER},
};
#define PARTS_74_74_FK(X) \
    X("5962-88520012A", LOGIC_FAMILY_AC, PACKAGE_TYPE_LCCC), \
    X("SNJ54AC74FK", LOGIC_FAMILY_AC, PACKAGE_TYPE_LCCC), \
    X("5962-8752501M2A", LOGIC_FAMILY_ACT, PACKAGE_TYPE_LCCC), \
    X("SNJ54ACT74FK", LOGIC_FAMILY_ACT, PACKAGE_TYPE_LCCC), \
    X("5962-9686001Q2A", LOGIC_FAMILY_AHC, PACKAGE_TYPE_LCCC), \
    X("SNJ54AHC74FK", LOGIC_FAMILY_AHC, PACKAGE_TYPE_LCCC), \
    X("5962-9686101Q2A", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_LCCC), \
    X("SNJ54AHCT74FK", LOGIC_FAMILY_AHCT, PACKAGE_TYPE_LCCC), \
    X("5962-9759201Q2A", LOGIC_FAMILY_F, PACKAGE_TYPE_LCCC), \
    X("SNJ54F74FK", LOGIC_FAMILY_F, PACKAGE_TYPE_LCCC), \
    X("84056012A", LOGIC_FAMILY_HC, PACKAGE_TYPE_LCCC), \
    X("SNJ54HC74FK", LOGIC_FAMILY_HC, PACKAGE_TYPE_LCCC), \
    X("SNJ54LS74AFK", LOGIC_FAMILY_LS, PACKAGE_TYPE_LCCC), \
    X("5962-9761601Q2A", LOGIC_FAMILY_LVC, PACKAGE_TYPE_LCCC), \
    X("SNJ54LVC74AFK", LOGIC_FAMILY_LVC, PACKAGE_TYPE_LCCC), \
    X("JM38510/34101B2A", LOGIC_FAMILY_F, PACKAGE_TYPE_LCCC), \
    X("JM38510/65302B2A", LOGIC_FAMILY_HC, PACKAGE_TYPE_LCCC), \
    X("JM38510/65352B2A", LOGIC_FAMILY_HCT, PACKAGE_TYPE_LCCC), \
    X("M38510/34101B2A", LOGIC_FAMILY_F, PACKAGE_TYPE_LCCC), \
    X("M38510/65302B2A", LOGIC_FAMILY_HC, PACKAGE_TYPE_LCCC), \
    X("M38510/65352B2A", LOGIC_FAMILY_HCT, PACKAGE_TYPE_LCCC)
FF_DEFINE_CHIP(FF_74_74_FK, "74_74_FK", "D flip-flop",
    "Dual positive-edge-triggered D flip-flop with preset and clear, 20-pin LCCC (FK)",
    LAYOUT_DUAL74_FK, PINS_DUAL74_FK, FAMILIES_74_74, PARTS_74_74_FK, 2)

static const FfUnit UNITS_HEX174_FK[] = {
    {FF_PIN(4), FF_PIN(12), FF_NO_PIN, FF_PIN(2), FF_PIN(3), FF_NO_PIN},
    {FF_PIN(5), FF_PIN(12), FF_NO_PIN, FF_PIN(2), FF_PIN(7), FF_NO_PIN},
    {FF_PIN(8), FF_PIN(12), FF_NO_PIN, FF_PIN(2), FF_PIN(9), FF_NO_PIN},
    {FF_PIN(14), FF_PIN(12), FF_NO_PIN, FF_PIN(2), FF_PIN(13), FF_NO_PIN},
    {FF_PIN(17), FF_PIN(12), FF_NO_PIN, FF_PIN(2), FF_PIN(15), FF_NO_PIN},
    {FF_PIN(18), FF_PIN(12), FF_NO_PIN, FF_PIN(2), FF_PIN(19), FF_NO_PIN},
};
static const FfLayout LAYOUT_HEX174_FK = {20, 6, UNITS_HEX174_FK, FF_NO_PIN, 0, 0, 0};
static const ChipPin PINS_HEX174_FK[] = {
    {1, "NC", PIN_ROLE_NO_CONNECT},
    {2, "CLR", PIN_ROLE_INPUT},
    {3, "1Q", PIN_ROLE_OUTPUT},
    {4, "1D", PIN_ROLE_INPUT},
    {5, "2D", PIN_ROLE_INPUT},
    {6, "NC", PIN_ROLE_NO_CONNECT},
    {7, "2Q", PIN_ROLE_OUTPUT},
    {8, "3D", PIN_ROLE_INPUT},
    {9, "3Q", PIN_ROLE_OUTPUT},
    {10, "GND", PIN_ROLE_GROUND},
    {11, "NC", PIN_ROLE_NO_CONNECT},
    {12, "CLK", PIN_ROLE_INPUT},
    {13, "4Q", PIN_ROLE_OUTPUT},
    {14, "4D", PIN_ROLE_INPUT},
    {15, "5Q", PIN_ROLE_OUTPUT},
    {16, "NC", PIN_ROLE_NO_CONNECT},
    {17, "5D", PIN_ROLE_INPUT},
    {18, "6D", PIN_ROLE_INPUT},
    {19, "6Q", PIN_ROLE_OUTPUT},
    {20, "VCC", PIN_ROLE_POWER},
};
#define PARTS_74_174_FK(X) \
    X("84073012A", LOGIC_FAMILY_HC, PACKAGE_TYPE_LCCC), \
    X("SNJ54HC174FK", LOGIC_FAMILY_HC, PACKAGE_TYPE_LCCC)
FF_DEFINE_CHIP(FF_74_174_FK, "74_174_FK", "D flip-flop",
    "Hex positive-edge-triggered D flip-flop with common clear, 20-pin LCCC (FK)",
    LAYOUT_HEX174_FK, PINS_HEX174_FK, FAMILIES_74_174, PARTS_74_174_FK, 6)

static const FfUnit UNITS_QUAD175_FK[] = {
    {FF_PIN(5), FF_PIN(12), FF_NO_PIN, FF_PIN(2), FF_PIN(3), FF_PIN(4)},
    {FF_PIN(7), FF_PIN(12), FF_NO_PIN, FF_PIN(2), FF_PIN(9), FF_PIN(8)},
    {FF_PIN(15), FF_PIN(12), FF_NO_PIN, FF_PIN(2), FF_PIN(13), FF_PIN(14)},
    {FF_PIN(17), FF_PIN(12), FF_NO_PIN, FF_PIN(2), FF_PIN(19), FF_PIN(18)},
};
static const FfLayout LAYOUT_QUAD175_FK = {20, 4, UNITS_QUAD175_FK, FF_NO_PIN, 0, 0, 0};
static const ChipPin PINS_QUAD175_FK[] = {
    {1, "NC", PIN_ROLE_NO_CONNECT},
    {2, "CLR", PIN_ROLE_INPUT},
    {3, "1Q", PIN_ROLE_OUTPUT},
    {4, "1QBAR", PIN_ROLE_OUTPUT},
    {5, "1D", PIN_ROLE_INPUT},
    {6, "NC", PIN_ROLE_NO_CONNECT},
    {7, "2D", PIN_ROLE_INPUT},
    {8, "2QBAR", PIN_ROLE_OUTPUT},
    {9, "2Q", PIN_ROLE_OUTPUT},
    {10, "GND", PIN_ROLE_GROUND},
    {11, "NC", PIN_ROLE_NO_CONNECT},
    {12, "CLK", PIN_ROLE_INPUT},
    {13, "3Q", PIN_ROLE_OUTPUT},
    {14, "3QBAR", PIN_ROLE_OUTPUT},
    {15, "3D", PIN_ROLE_INPUT},
    {16, "NC", PIN_ROLE_NO_CONNECT},
    {17, "4D", PIN_ROLE_INPUT},
    {18, "4QBAR", PIN_ROLE_OUTPUT},
    {19, "4Q", PIN_ROLE_OUTPUT},
    {20, "VCC", PIN_ROLE_POWER},
};
#define PARTS_74_175_FK(X) \
    X("84089012A", LOGIC_FAMILY_HC, PACKAGE_TYPE_LCCC), \
    X("SNJ54HC175FK", LOGIC_FAMILY_HC, PACKAGE_TYPE_LCCC)
FF_DEFINE_CHIP(FF_74_175_FK, "74_175_FK", "D flip-flop",
    "Quad positive-edge-triggered D flip-flop with common clear, 20-pin LCCC (FK)",
    LAYOUT_QUAD175_FK, PINS_QUAD175_FK, FAMILIES_74_175, PARTS_74_175_FK, 4)

static const FfUnit UNITS_1G79_SON[] = {
    {FF_PIN(1), FF_PIN(2), FF_NO_PIN, FF_NO_PIN, FF_PIN(4), FF_NO_PIN},
};
static const FfLayout LAYOUT_1G79_SON = {6, 1, UNITS_1G79_SON, FF_NO_PIN, 0, 0, 0};
static const ChipPin PINS_1G79_SON[] = {
    {1, "D", PIN_ROLE_INPUT},
    {2, "CLK", PIN_ROLE_INPUT},
    {3, "GND", PIN_ROLE_GROUND},
    {4, "Q", PIN_ROLE_OUTPUT},
    {5, "NC", PIN_ROLE_NO_CONNECT},
    {6, "VCC", PIN_ROLE_POWER},
};
#define PARTS_1G79_SON(X) \
    X("SN74AUP1G79DRYR", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SON), \
    X("SN74AUP1G79DRYRG4", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SON), \
    X("SN74AUP1G79DSFR", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SON)
FF_DEFINE_CHIP(FF_1G79_SON, "1G79_SON", "D flip-flop",
    "Single positive-edge-triggered D flip-flop, 6-pin SON (DRY, DSF)",
    LAYOUT_1G79_SON, PINS_1G79_SON, FAMILIES_1G79, PARTS_1G79_SON, 1)

/* ---- Inverted-output parts and the single LVC2G74 ---------------------- */
/*
 * Timing basis matches the entries above: AUP is typical at 3.3 V, 25 C,
 * CL = 15 pF; LVC is max at 3.3 V, -40 to 85 C, CL = 50 pF (no typical given).
 */

/* 5-pin single, inverted output: 1 D, 2 CLK, 3 GND, 4 QBAR, 5 VCC (SN74AUP1G80, SN74LVC1G80). */
static const FfUnit UNITS_1G80[] = {
    {FF_PIN(1), FF_PIN(2), FF_NO_PIN, FF_NO_PIN, FF_NO_PIN, FF_PIN(4)},
};
static const FfLayout LAYOUT_1G80 = {5, 1, UNITS_1G80, FF_NO_PIN, 0, 0, 0};
static const ChipPin PINS_1G80[] = {
    {1, "D", PIN_ROLE_INPUT},
    {2, "CLK", PIN_ROLE_INPUT},
    {3, "GND", PIN_ROLE_GROUND},
    {4, "QBAR", PIN_ROLE_OUTPUT},
    {5, "VCC", PIN_ROLE_POWER},
};
static const FfFamily FAMILIES_1G80[] = {
    {LOGIC_FAMILY_AUP, 3100, 3100, 0},   /* SN74AUP1G80: typ 3.1 ns */
    {LOGIC_FAMILY_LVC, 5200, 5200, 0},   /* SN74LVC1G80: max 5.2 ns */
};
#define PARTS_1G80(X) \
    X("SN74AUP1G80DBVR", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SOT23), \
    X("SN74AUP1G80DBVT", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SOT23), \
    X("SN74AUP1G80DBVTG4", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SOT23), \
    X("SN74AUP1G80DCKR", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SC70), \
    X("SN74AUP1G80DCKR1G4", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SC70), \
    X("SN74AUP1G80DCKRE4", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SC70), \
    X("SN74AUP1G80DCKRG4", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SC70), \
    X("SN74AUP1G80DCKT", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SC70), \
    X("SN74AUP1G80DCKTG4", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SC70), \
    X("SN74AUP1G80DPWR", LOGIC_FAMILY_AUP, PACKAGE_TYPE_X2SON), \
    X("SN74LVC1G80DBVR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOT23), \
    X("SN74LVC1G80DBVRE4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOT23), \
    X("SN74LVC1G80DBVRG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOT23), \
    X("SN74LVC1G80DBVT", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOT23), \
    X("SN74LVC1G80DBVTG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOT23), \
    X("SN74LVC1G80DCKR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("SN74LVC1G80DCKRG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("SN74LVC1G80DCKT", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("SN74LVC1G80QDCKRQ1", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("SN74LVC1G80QDCKTQ1", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SC70), \
    X("SN74LVC1G80YZPR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_DSBGA)
FF_DEFINE_CHIP(FF_1G80, "1G80", "D flip-flop",
    "Single positive-edge-triggered D flip-flop, inverted output",
    LAYOUT_1G80, PINS_1G80, FAMILIES_1G80, PARTS_1G80, 1)

/* 6-pin SON (DRY, DSF): NC on pin 5, VCC on pin 6. */
static const FfLayout LAYOUT_1G80_SON = {6, 1, UNITS_1G80, FF_NO_PIN, 0, 0, 0};
static const ChipPin PINS_1G80_SON[] = {
    {1, "D", PIN_ROLE_INPUT},
    {2, "CLK", PIN_ROLE_INPUT},
    {3, "GND", PIN_ROLE_GROUND},
    {4, "QBAR", PIN_ROLE_OUTPUT},
    {5, "NC", PIN_ROLE_NO_CONNECT},
    {6, "VCC", PIN_ROLE_POWER},
};
#define PARTS_1G80_SON(X) \
    X("SN74AUP1G80DRYR", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SON), \
    X("SN74AUP1G80DSFR", LOGIC_FAMILY_AUP, PACKAGE_TYPE_SON)
FF_DEFINE_CHIP(FF_1G80_SON, "1G80_SON", "D flip-flop",
    "Single positive-edge-triggered D flip-flop, inverted output, 6-pin SON (DRY, DSF)",
    LAYOUT_1G80_SON, PINS_1G80_SON, FAMILIES_1G80, PARTS_1G80_SON, 1)

/* 8-pin dual, inverted outputs: 1 1CLK, 2 1D, 3 2QBAR, 4 GND, 5 2CLK, 6 2D, 7 1QBAR, 8 VCC (DCU, DQE). */
static const FfUnit UNITS_2G80[] = {
    {FF_PIN(2), FF_PIN(1), FF_NO_PIN, FF_NO_PIN, FF_NO_PIN, FF_PIN(7)},
    {FF_PIN(6), FF_PIN(5), FF_NO_PIN, FF_NO_PIN, FF_NO_PIN, FF_PIN(3)},
};
static const FfLayout LAYOUT_2G80 = {8, 2, UNITS_2G80, FF_NO_PIN, 0, 0, 0};
static const ChipPin PINS_2G80[] = {
    {1, "1CLK", PIN_ROLE_INPUT},
    {2, "1D", PIN_ROLE_INPUT},
    {3, "2QBAR", PIN_ROLE_OUTPUT},
    {4, "GND", PIN_ROLE_GROUND},
    {5, "2CLK", PIN_ROLE_INPUT},
    {6, "2D", PIN_ROLE_INPUT},
    {7, "1QBAR", PIN_ROLE_OUTPUT},
    {8, "VCC", PIN_ROLE_POWER},
};
static const FfFamily FAMILIES_2G80[] = {
    {LOGIC_FAMILY_AUP, 3100, 3100, 0},   /* SN74AUP2G80: typ 3.1 ns */
};
#define PARTS_2G80(X) \
    X("SN74AUP2G80DCUR", LOGIC_FAMILY_AUP, PACKAGE_TYPE_VSSOP), \
    X("SN74AUP2G80DCURG4", LOGIC_FAMILY_AUP, PACKAGE_TYPE_VSSOP), \
    X("SN74AUP2G80DQER", LOGIC_FAMILY_AUP, PACKAGE_TYPE_X2SON)
FF_DEFINE_CHIP(FF_2G80, "2G80", "D flip-flop",
    "Dual positive-edge-triggered D flip-flop, inverted outputs",
    LAYOUT_2G80, PINS_2G80, FAMILIES_2G80, PARTS_2G80, 2)

/* UQFN (RSE): 1 1QBAR, 2 2D, 3 2CLK, 4 GND, 5 2QBAR, 6 1D, 7 1CLK, 8 VCC. */
static const FfUnit UNITS_2G80_RSE[] = {
    {FF_PIN(6), FF_PIN(7), FF_NO_PIN, FF_NO_PIN, FF_NO_PIN, FF_PIN(1)},
    {FF_PIN(2), FF_PIN(3), FF_NO_PIN, FF_NO_PIN, FF_NO_PIN, FF_PIN(5)},
};
static const FfLayout LAYOUT_2G80_RSE = {8, 2, UNITS_2G80_RSE, FF_NO_PIN, 0, 0, 0};
static const ChipPin PINS_2G80_RSE[] = {
    {1, "1QBAR", PIN_ROLE_OUTPUT},
    {2, "2D", PIN_ROLE_INPUT},
    {3, "2CLK", PIN_ROLE_INPUT},
    {4, "GND", PIN_ROLE_GROUND},
    {5, "2QBAR", PIN_ROLE_OUTPUT},
    {6, "1D", PIN_ROLE_INPUT},
    {7, "1CLK", PIN_ROLE_INPUT},
    {8, "VCC", PIN_ROLE_POWER},
};
#define PARTS_2G80_RSE(X) \
    X("SN74AUP2G80RSER", LOGIC_FAMILY_AUP, PACKAGE_TYPE_UQFN)
FF_DEFINE_CHIP(FF_2G80_RSE, "2G80_RSE", "D flip-flop",
    "Dual positive-edge-triggered D flip-flop, inverted outputs, UQFN (RSE)",
    LAYOUT_2G80_RSE, PINS_2G80_RSE, FAMILIES_2G80, PARTS_2G80_RSE, 2)

/* SN74LVC2G74 is a single flip-flop with the 1G74 pinout. PRE and CLR both low: Q = H, QBAR = H. */
static const FfFamily FAMILIES_2G74[] = {
    {LOGIC_FAMILY_LVC, 5900, 5900, 0},   /* SN74LVC2G74: max 5.9 ns CLK to Q */
};
#define PARTS_2G74(X) \
    X("CLVC2G74QDCURG4Q1", LOGIC_FAMILY_LVC, PACKAGE_TYPE_VSSOP), \
    X("SN74LVC2G74DCT3", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SSOP), \
    X("SN74LVC2G74DCTR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SSOP), \
    X("SN74LVC2G74DCTRE4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SSOP), \
    X("SN74LVC2G74DCTRE6", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SSOP), \
    X("SN74LVC2G74DCTRG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SSOP), \
    X("SN74LVC2G74DCUR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_VSSOP), \
    X("SN74LVC2G74DCURE4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_VSSOP), \
    X("SN74LVC2G74DCURG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_VSSOP), \
    X("SN74LVC2G74DCUT", LOGIC_FAMILY_LVC, PACKAGE_TYPE_VSSOP), \
    X("SN74LVC2G74DCUTE4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_VSSOP), \
    X("SN74LVC2G74DCUTG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_VSSOP), \
    X("SN74LVC2G74MDCUTEP", LOGIC_FAMILY_LVC, PACKAGE_TYPE_VSSOP), \
    X("SN74LVC2G74QDCURQ1", LOGIC_FAMILY_LVC, PACKAGE_TYPE_VSSOP), \
    X("SN74LVC2G74YZPR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_DSBGA), \
    X("V62/08617-01XE", LOGIC_FAMILY_LVC, PACKAGE_TYPE_VSSOP)
FF_DEFINE_CHIP(FF_2G74, "2G74", "D flip-flop",
    "Single positive-edge-triggered D flip-flop with preset and clear (2G74)",
    LAYOUT_1G74, PINS_1G74, FAMILIES_2G74, PARTS_2G74, 1)
