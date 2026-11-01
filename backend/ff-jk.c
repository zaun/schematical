#include "ff-jk.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define FF_NO_PIN ((size_t)-1)
#define FF_MAX_UNITS 2
#define FF_COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define FF_PIN(number) FF_JK_PIN(number)

/* One flip-flop inside a package. Unused pins are FF_NO_PIN. */
typedef struct {
    size_t j;
    size_t k;
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
    int k_inverted;      /* K input is KBAR (109) */
    int pre_active_high;
    int clr_active_high;
    int negative_edge;
} FfLayout;

typedef struct {
    LogicFamily family;
    SimTime rise_delay;
    SimTime fall_delay;
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
    LogicValue q[FF_MAX_UNITS];
    LogicValue qn[FF_MAX_UNITS];
    /* Values last handed to the simulator, so unchanged outputs are not rescheduled. */
    int scheduled[FF_MAX_UNITS];
    LogicValue driven_q[FF_MAX_UNITS];
    LogicValue driven_qn[FF_MAX_UNITS];
} FfState;

#define FF_PART_ENTRY(name, family, package, flags) {name, family, package}
#define FF_PART_NAME(name, family, package, flags) name
#define FF_PART_FLAGS(name, family, package, flags) flags

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

    if (pre < 0 || clr < 0) {
        set_unit(state, i, LOGIC_X, LOGIC_X);
    } else if (pre == 1 && clr == 1) {
        set_unit(state, i, LOGIC_1, LOGIC_1);
    } else if (clr == 1) {
        set_unit(state, i, LOGIC_0, LOGIC_1);
    } else if (pre == 1) {
        set_unit(state, i, LOGIC_1, LOGIC_0);
    }
}

/* Next Q for known J and K levels (0 or 1). */
static LogicValue jk_next(LogicValue q, int j, int k) {
    if (j && k) {
        if (q == LOGIC_0) {
            return LOGIC_1;
        }
        if (q == LOGIC_1) {
            return LOGIC_0;
        }
        return LOGIC_X;
    }
    if (j) {
        return LOGIC_1;
    }
    if (k) {
        return LOGIC_0;
    }
    return q == LOGIC_0 || q == LOGIC_1 ? q : LOGIC_X;
}

/*
 * Clocks one flip-flop. An unknown J or K (X or Z) gives X unless every
 * level it could have leads to the same Q, e.g. J=H with K unknown while Q=H.
 */
static void capture(Sim *sim, SimInstance *instance, FfState *state, size_t i) {
    const FfLayout *layout = state->layout;
    const FfUnit *unit = &layout->units[i];
    LogicValue j = sim_read_pin(sim, instance, unit->j);
    LogicValue k = sim_read_pin(sim, instance, unit->k);
    int j_lo = j == LOGIC_1 ? 1 : 0;
    int j_hi = j == LOGIC_0 ? 0 : 1;
    int k_lo;
    int k_hi;
    LogicValue next = LOGIC_X;
    int first = 1;
    int jv;
    int kv;

    if (layout->k_inverted) {
        k_lo = k == LOGIC_0 ? 1 : 0;
        k_hi = k == LOGIC_1 ? 0 : 1;
    } else {
        k_lo = k == LOGIC_1 ? 1 : 0;
        k_hi = k == LOGIC_0 ? 0 : 1;
    }

    for (jv = j_lo; jv <= j_hi; jv++) {
        for (kv = k_lo; kv <= k_hi; kv++) {
            LogicValue value = jk_next(state->q[i], jv, kv);

            if (first) {
                next = value;
                first = 0;
            } else if (value != next) {
                next = LOGIC_X;
            }
        }
    }

    if (next == LOGIC_0) {
        set_unit(state, i, LOGIC_0, LOGIC_1);
    } else if (next == LOGIC_1) {
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
    } else if (value == LOGIC_1) {
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
    static const unsigned NAME##_FLAGS[] = { PART_LIST(FF_PART_FLAGS) }; \
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
        NAME##_lookup, GATES, NAME##_FLAGS \
    };

/* ---- 14-pin dual with clear, 73 pinout (1CLK 1 ... 1J 14) --------------- */

static const FfUnit UNITS_DUAL73[] = {
    /* j, k, clk, pre, clr, q, qn */
    {FF_PIN(14), FF_PIN(3), FF_PIN(1), FF_NO_PIN, FF_PIN(2), FF_PIN(12), FF_PIN(13)},
    {FF_PIN(7), FF_PIN(10), FF_PIN(5), FF_NO_PIN, FF_PIN(6), FF_PIN(9), FF_PIN(8)},
};

static const FfLayout LAYOUT_DUAL73 = {14, 2, UNITS_DUAL73, 0, 0, 0, 1};

static const ChipPin PINS_DUAL73[] = {
    {1, "1CLK", PIN_ROLE_INPUT},
    {2, "1CLR", PIN_ROLE_INPUT},
    {3, "1K", PIN_ROLE_INPUT},
    {4, "VCC", PIN_ROLE_POWER},
    {5, "2CLK", PIN_ROLE_INPUT},
    {6, "2CLR", PIN_ROLE_INPUT},
    {7, "2J", PIN_ROLE_INPUT},
    {8, "2QBAR", PIN_ROLE_OUTPUT},
    {9, "2Q", PIN_ROLE_OUTPUT},
    {10, "2K", PIN_ROLE_INPUT},
    {11, "GND", PIN_ROLE_GROUND},
    {12, "1Q", PIN_ROLE_OUTPUT},
    {13, "1QBAR", PIN_ROLE_OUTPUT},
    {14, "1J", PIN_ROLE_INPUT},
};

static const FfFamily FAMILIES_74_73[] = {
    {LOGIC_FAMILY_LS, 15000, 15000},   /* SN74LS73A: typical at 5 V, 25 C (15 pF) */
    {LOGIC_FAMILY_HC, 13000, 13000},   /* CD74HC73: typical at 5 V, 25 C (15 pF) */
    {LOGIC_FAMILY_HCT, 38000, 38000},  /* CD74HCT73: max at 4.5 V, 25 C (50 pF); no typical given */
};

#define PARTS_74_73(X) \
    X("5962-8515301CA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("CD54HC73F", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("CD54HC73F3A", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("CD74HC73E", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP, 0), \
    X("CD74HC73M", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("CD74HC73M96", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC, 0), \
    X("CD74HC73MT", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("CD74HCT73E", LOGIC_FAMILY_HCT, PACKAGE_TYPE_DIP, 0), \
    X("CD74HCT73M", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("CD74HCT73M96", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC, 0), \
    X("5962-9675101QCA", LOGIC_FAMILY_LS, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("5962-9675101QDA", LOGIC_FAMILY_LS, PACKAGE_TYPE_CFP, PART_FLAG_MILITARY), \
    X("SN54LS73AJ", LOGIC_FAMILY_LS, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SN74LS73AD", LOGIC_FAMILY_LS, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("SN74LS73ADR", LOGIC_FAMILY_LS, PACKAGE_TYPE_SOIC, 0), \
    X("SN74LS73AN", LOGIC_FAMILY_LS, PACKAGE_TYPE_DIP, 0), \
    X("SN74LS73ANE4", LOGIC_FAMILY_LS, PACKAGE_TYPE_DIP, 0), \
    X("SNJ54LS73AJ", LOGIC_FAMILY_LS, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SNJ54LS73AW", LOGIC_FAMILY_LS, PACKAGE_TYPE_CFP, PART_FLAG_MILITARY)

FF_DEFINE_CHIP(FF_74_73, "74_73", "JK flip-flop",
    "Dual negative-edge-triggered J-K flip-flop with clear",
    LAYOUT_DUAL73, PINS_DUAL73, FAMILIES_74_73, PARTS_74_73, 2)

/* ---- 16-pin dual with preset and clear, 76 pinout (1CLK 1 ... 1K 16) ---- */

static const FfUnit UNITS_DUAL76[] = {
    /* j, k, clk, pre, clr, q, qn */
    {FF_PIN(4), FF_PIN(16), FF_PIN(1), FF_PIN(2), FF_PIN(3), FF_PIN(15), FF_PIN(14)},
    {FF_PIN(9), FF_PIN(12), FF_PIN(6), FF_PIN(7), FF_PIN(8), FF_PIN(11), FF_PIN(10)},
};

static const FfLayout LAYOUT_DUAL76 = {16, 2, UNITS_DUAL76, 0, 0, 0, 1};

static const ChipPin PINS_DUAL76[] = {
    {1, "1CLK", PIN_ROLE_INPUT},
    {2, "1PRE", PIN_ROLE_INPUT},
    {3, "1CLR", PIN_ROLE_INPUT},
    {4, "1J", PIN_ROLE_INPUT},
    {5, "VCC", PIN_ROLE_POWER},
    {6, "2CLK", PIN_ROLE_INPUT},
    {7, "2PRE", PIN_ROLE_INPUT},
    {8, "2CLR", PIN_ROLE_INPUT},
    {9, "2J", PIN_ROLE_INPUT},
    {10, "2QBAR", PIN_ROLE_OUTPUT},
    {11, "2Q", PIN_ROLE_OUTPUT},
    {12, "2K", PIN_ROLE_INPUT},
    {13, "GND", PIN_ROLE_GROUND},
    {14, "1QBAR", PIN_ROLE_OUTPUT},
    {15, "1Q", PIN_ROLE_OUTPUT},
    {16, "1K", PIN_ROLE_INPUT},
};

static const FfFamily FAMILIES_74_76[] = {
    {LOGIC_FAMILY_TTL, 16000, 25000},  /* SN5476: typical at 5 V, 25 C (15 pF) */
    {LOGIC_FAMILY_LS, 15000, 15000},   /* SN54LS76A: typical at 5 V, 25 C (15 pF) */
};

#define PARTS_74_76(X) \
    X("5962-9557501QEA", LOGIC_FAMILY_TTL, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("7601301EA", LOGIC_FAMILY_LS, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SN5476J", LOGIC_FAMILY_TTL, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SN54LS76AJ", LOGIC_FAMILY_LS, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SNJ5476J", LOGIC_FAMILY_TTL, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SNJ54LS76AJ", LOGIC_FAMILY_LS, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY)

FF_DEFINE_CHIP(FF_74_76, "74_76", "JK flip-flop",
    "Dual J-K flip-flop with preset and clear",
    LAYOUT_DUAL76, PINS_DUAL76, FAMILIES_74_76, PARTS_74_76, 2)

/* ---- 14-pin dual with clear, 107 pinout (1J 1 ... VCC 14) --------------- */

static const FfUnit UNITS_DUAL107[] = {
    /* j, k, clk, pre, clr, q, qn */
    {FF_PIN(1), FF_PIN(4), FF_PIN(12), FF_NO_PIN, FF_PIN(13), FF_PIN(3), FF_PIN(2)},
    {FF_PIN(8), FF_PIN(11), FF_PIN(9), FF_NO_PIN, FF_PIN(10), FF_PIN(5), FF_PIN(6)},
};

static const FfLayout LAYOUT_DUAL107 = {14, 2, UNITS_DUAL107, 0, 0, 0, 1};

static const ChipPin PINS_DUAL107[] = {
    {1, "1J", PIN_ROLE_INPUT},
    {2, "1QBAR", PIN_ROLE_OUTPUT},
    {3, "1Q", PIN_ROLE_OUTPUT},
    {4, "1K", PIN_ROLE_INPUT},
    {5, "2Q", PIN_ROLE_OUTPUT},
    {6, "2QBAR", PIN_ROLE_OUTPUT},
    {7, "GND", PIN_ROLE_GROUND},
    {8, "2J", PIN_ROLE_INPUT},
    {9, "2CLK", PIN_ROLE_INPUT},
    {10, "2CLR", PIN_ROLE_INPUT},
    {11, "2K", PIN_ROLE_INPUT},
    {12, "1CLK", PIN_ROLE_INPUT},
    {13, "1CLR", PIN_ROLE_INPUT},
    {14, "VCC", PIN_ROLE_POWER},
};

static const FfFamily FAMILIES_74_107[] = {
    {LOGIC_FAMILY_TTL, 16000, 25000},  /* SN54107: typical at 5 V, 25 C (15 pF) */
    {LOGIC_FAMILY_LS, 15000, 15000},   /* SN74LS107A: typical at 5 V, 25 C (15 pF) */
    {LOGIC_FAMILY_HC, 14000, 14000},   /* CD74HC107: typical at 5 V, 25 C (15 pF) */
    {LOGIC_FAMILY_HCT, 18000, 18000},  /* CD74HCT107: typical at 5 V, 25 C (15 pF) */
};

#define PARTS_74_107(X) \
    X("5962-8515401CA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("CD54HC107F3A", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("CD74HC107E", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP, 0), \
    X("CD74HC107EE4", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP, 0), \
    X("CD74HC107M", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("CD74HC107M96", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC, 0), \
    X("CD74HC107MT", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("CD74HCT107E", LOGIC_FAMILY_HCT, PACKAGE_TYPE_DIP, 0), \
    X("SN54107J", LOGIC_FAMILY_TTL, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SN74LS107AD", LOGIC_FAMILY_LS, PACKAGE_TYPE_SOIC, 0), \
    X("SN74LS107AN", LOGIC_FAMILY_LS, PACKAGE_TYPE_DIP, 0), \
    X("SN74LS107ANSR", LOGIC_FAMILY_LS, PACKAGE_TYPE_SOP, 0), \
    X("SNJ54107J", LOGIC_FAMILY_TTL, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY)

FF_DEFINE_CHIP(FF_74_107, "74_107", "JK flip-flop",
    "Dual J-K flip-flop with clear",
    LAYOUT_DUAL107, PINS_DUAL107, FAMILIES_74_107, PARTS_74_107, 2)

/* ---- 16-pin dual J-KBAR, 109 pinout (1CLR 1 ... VCC 16) ----------------- */

static const FfUnit UNITS_DUAL109[] = {
    /* j, k (KBAR), clk, pre, clr, q, qn */
    {FF_PIN(2), FF_PIN(3), FF_PIN(4), FF_PIN(5), FF_PIN(1), FF_PIN(6), FF_PIN(7)},
    {FF_PIN(14), FF_PIN(13), FF_PIN(12), FF_PIN(11), FF_PIN(15), FF_PIN(10), FF_PIN(9)},
};

static const FfLayout LAYOUT_DUAL109 = {16, 2, UNITS_DUAL109, 1, 0, 0, 0};

static const ChipPin PINS_DUAL109[] = {
    {1, "1CLR", PIN_ROLE_INPUT},
    {2, "1J", PIN_ROLE_INPUT},
    {3, "1KBAR", PIN_ROLE_INPUT},
    {4, "1CLK", PIN_ROLE_INPUT},
    {5, "1PRE", PIN_ROLE_INPUT},
    {6, "1Q", PIN_ROLE_OUTPUT},
    {7, "1QBAR", PIN_ROLE_OUTPUT},
    {8, "GND", PIN_ROLE_GROUND},
    {9, "2QBAR", PIN_ROLE_OUTPUT},
    {10, "2Q", PIN_ROLE_OUTPUT},
    {11, "2PRE", PIN_ROLE_INPUT},
    {12, "2CLK", PIN_ROLE_INPUT},
    {13, "2KBAR", PIN_ROLE_INPUT},
    {14, "2J", PIN_ROLE_INPUT},
    {15, "2CLR", PIN_ROLE_INPUT},
    {16, "VCC", PIN_ROLE_POWER},
};

static const FfFamily FAMILIES_74_109[] = {
    {LOGIC_FAMILY_LS, 13000, 25000},   /* SN74LS109A: typical at 5 V, 25 C (15 pF) */
    {LOGIC_FAMILY_HC, 15000, 15000},   /* SN74HC109: typical at 4.5 V, 25 C (50 pF) */
    {LOGIC_FAMILY_HCT, 17000, 17000},  /* CD74HCT109: typical at 5 V, 25 C (15 pF) */
    {LOGIC_FAMILY_AC, 9400, 9400},     /* CD74AC109: max at 5 V, -40 to 85 C (50 pF); no typical given */
    {LOGIC_FAMILY_ACT, 9400, 9400},    /* CD74ACT109: max at 5 V, -40 to 85 C (50 pF); no typical given */
    {LOGIC_FAMILY_F, 4900, 5800},      /* SN74F109: typical at 5 V, 25 C (50 pF) */
    {LOGIC_FAMILY_ALS, 16000, 18000},  /* SN74ALS109A: max, 0 to 70 C (50 pF); no typical given */
    {LOGIC_FAMILY_AS, 9000, 9000},     /* SN74AS109A: max, 0 to 70 C (50 pF); no typical given */
};

#define PARTS_74_109(X) \
    X("CD54AC109F3A", LOGIC_FAMILY_AC, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("CD74AC109E", LOGIC_FAMILY_AC, PACKAGE_TYPE_DIP, 0), \
    X("CD74AC109M96", LOGIC_FAMILY_AC, PACKAGE_TYPE_SOIC, 0), \
    X("CD54ACT109F3A", LOGIC_FAMILY_ACT, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("CD74ACT109E", LOGIC_FAMILY_ACT, PACKAGE_TYPE_DIP, 0), \
    X("CD74ACT109M", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("CD74ACT109M96", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SOIC, 0), \
    X("5962-9070101MEA", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("CD54HC109F3A", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("CD54HCT109F3A", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("CD74HC109E", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP, 0), \
    X("CD74HC109M", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("CD74HC109M96", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC, 0), \
    X("CD74HCT109E", LOGIC_FAMILY_HCT, PACKAGE_TYPE_DIP, 0), \
    X("CD74HCT109M", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("CD74HCT109M96", LOGIC_FAMILY_HCT, PACKAGE_TYPE_SOIC, 0), \
    X("8400001EA", LOGIC_FAMILY_ALS, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SN54ALS109AJ", LOGIC_FAMILY_ALS, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SN74ALS109AD", LOGIC_FAMILY_ALS, PACKAGE_TYPE_SOIC, 0), \
    X("SN74ALS109AN", LOGIC_FAMILY_ALS, PACKAGE_TYPE_DIP, 0), \
    X("SN74ALS109ANSR", LOGIC_FAMILY_ALS, PACKAGE_TYPE_SOP, 0), \
    X("SN74AS109AD", LOGIC_FAMILY_AS, PACKAGE_TYPE_SOIC, 0), \
    X("SN74AS109AN", LOGIC_FAMILY_AS, PACKAGE_TYPE_DIP, 0), \
    X("SN74AS109ANSR", LOGIC_FAMILY_AS, PACKAGE_TYPE_SOP, 0), \
    X("SNJ54ALS109AJ", LOGIC_FAMILY_ALS, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("5962-9758001QEA", LOGIC_FAMILY_F, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("5962-9758001QFA", LOGIC_FAMILY_F, PACKAGE_TYPE_CFP, PART_FLAG_MILITARY), \
    X("JM38510/34102BEA", LOGIC_FAMILY_F, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("JM38510/34102BFA", LOGIC_FAMILY_F, PACKAGE_TYPE_CFP, PART_FLAG_MILITARY), \
    X("M38510/34102BEA", LOGIC_FAMILY_F, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("M38510/34102BFA", LOGIC_FAMILY_F, PACKAGE_TYPE_CFP, PART_FLAG_MILITARY), \
    X("SN74F109D", LOGIC_FAMILY_F, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("SN74F109DR", LOGIC_FAMILY_F, PACKAGE_TYPE_SOIC, 0), \
    X("SN74F109N", LOGIC_FAMILY_F, PACKAGE_TYPE_DIP, 0), \
    X("SNJ54F109J", LOGIC_FAMILY_F, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SNJ54F109W", LOGIC_FAMILY_F, PACKAGE_TYPE_CFP, PART_FLAG_MILITARY), \
    X("5962-8415001VFA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP, PART_FLAG_MILITARY), \
    X("8415001EA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("8415001FA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP, PART_FLAG_MILITARY), \
    X("JM38510/65304BEA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("M38510/65304BEA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SN54HC109J", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SN74HC109D", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("SN74HC109DR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC, 0), \
    X("SN74HC109DRG4", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC, 0), \
    X("SN74HC109N", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP, 0), \
    X("SN74HC109NE4", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP, 0), \
    X("SN74HC109NSR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP, 0), \
    X("SNJ54HC109J", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SNJ54HC109W", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP, PART_FLAG_MILITARY), \
    X("SN54LS109AJ", LOGIC_FAMILY_LS, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SN74LS109AD", LOGIC_FAMILY_LS, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("SN74LS109ADR", LOGIC_FAMILY_LS, PACKAGE_TYPE_SOIC, 0), \
    X("SN74LS109AN", LOGIC_FAMILY_LS, PACKAGE_TYPE_DIP, 0), \
    X("SN74LS109ANE4", LOGIC_FAMILY_LS, PACKAGE_TYPE_DIP, 0), \
    X("SN74LS109ANSR", LOGIC_FAMILY_LS, PACKAGE_TYPE_SOP, 0), \
    X("SNJ54LS109AJ", LOGIC_FAMILY_LS, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SNJ54LS109AW", LOGIC_FAMILY_LS, PACKAGE_TYPE_CFP, PART_FLAG_MILITARY)

FF_DEFINE_CHIP(FF_74_109, "74_109", "JK flip-flop",
    "Dual J-KBAR positive-edge-triggered flip-flop with preset and clear",
    LAYOUT_DUAL109, PINS_DUAL109, FAMILIES_74_109, PARTS_74_109, 2)

/* ---- 16-pin dual, 112 pinout (1CLK 1 ... VCC 16) ------------------------ */

static const FfUnit UNITS_DUAL112[] = {
    /* j, k, clk, pre, clr, q, qn */
    {FF_PIN(3), FF_PIN(2), FF_PIN(1), FF_PIN(4), FF_PIN(15), FF_PIN(5), FF_PIN(6)},
    {FF_PIN(11), FF_PIN(12), FF_PIN(13), FF_PIN(10), FF_PIN(14), FF_PIN(9), FF_PIN(7)},
};

static const FfLayout LAYOUT_DUAL112 = {16, 2, UNITS_DUAL112, 0, 0, 0, 1};

static const ChipPin PINS_DUAL112[] = {
    {1, "1CLK", PIN_ROLE_INPUT},
    {2, "1K", PIN_ROLE_INPUT},
    {3, "1J", PIN_ROLE_INPUT},
    {4, "1PRE", PIN_ROLE_INPUT},
    {5, "1Q", PIN_ROLE_OUTPUT},
    {6, "1QBAR", PIN_ROLE_OUTPUT},
    {7, "2QBAR", PIN_ROLE_OUTPUT},
    {8, "GND", PIN_ROLE_GROUND},
    {9, "2Q", PIN_ROLE_OUTPUT},
    {10, "2PRE", PIN_ROLE_INPUT},
    {11, "2J", PIN_ROLE_INPUT},
    {12, "2K", PIN_ROLE_INPUT},
    {13, "2CLK", PIN_ROLE_INPUT},
    {14, "2CLR", PIN_ROLE_INPUT},
    {15, "1CLR", PIN_ROLE_INPUT},
    {16, "VCC", PIN_ROLE_POWER},
};

static const FfFamily FAMILIES_74_112[] = {
    {LOGIC_FAMILY_LS, 15000, 15000},   /* SN74LS112A: typical at 5 V, 25 C (15 pF) */
    {LOGIC_FAMILY_S, 4000, 5000},      /* SN74S112A: typical at 5 V, 25 C (15 pF) */
    {LOGIC_FAMILY_HC, 16000, 16000},   /* SN74HC112: typical at 4.5 V, 25 C (50 pF) */
    {LOGIC_FAMILY_HCT, 14000, 14000},  /* CD74HCT112: typical at 5 V, 25 C (15 pF) */
    {LOGIC_FAMILY_AC, 9400, 9400},     /* CD74AC112: max at 5 V, -40 to 85 C (50 pF); no typical given */
    {LOGIC_FAMILY_ACT, 9400, 9400},    /* CD74ACT112: max at 5 V, -40 to 85 C (50 pF); no typical given */
    {LOGIC_FAMILY_F, 4600, 4600},      /* SN74F112: typical at 5 V, 25 C (50 pF) */
    {LOGIC_FAMILY_ALS, 15000, 19000},  /* SN74ALS112A: max, 0 to 70 C (50 pF); no typical given */
    {LOGIC_FAMILY_LVC, 3500, 3500},    /* SN74LVC112A: typical at 3.3 V, 25 C (50 pF) */
};

#define PARTS_74_112(X) \
    X("CD54AC112F3A", LOGIC_FAMILY_AC, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("CD74AC112E", LOGIC_FAMILY_AC, PACKAGE_TYPE_DIP, 0), \
    X("CD74AC112M", LOGIC_FAMILY_AC, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("CD74AC112M96", LOGIC_FAMILY_AC, PACKAGE_TYPE_SOIC, 0), \
    X("CD54ACT112F3A", LOGIC_FAMILY_ACT, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("CD74ACT112M", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("CD74ACT112M96", LOGIC_FAMILY_ACT, PACKAGE_TYPE_SOIC, 0), \
    X("5962-8970201EA", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("CD54HC112F3A", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("CD54HCT112F3A", LOGIC_FAMILY_HCT, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("CD74HC112E", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP, 0), \
    X("CD74HC112M96", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC, 0), \
    X("CD74HC112MT", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("CD74HC112NSR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOP, 0), \
    X("CD74HC112PW", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP, PART_FLAG_OBSOLETE), \
    X("CD74HC112PWR", LOGIC_FAMILY_HC, PACKAGE_TYPE_TSSOP, 0), \
    X("CD74HCT112E", LOGIC_FAMILY_HCT, PACKAGE_TYPE_DIP, 0), \
    X("8400002EA", LOGIC_FAMILY_ALS, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("JM38510/37103BEA", LOGIC_FAMILY_ALS, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("M38510/37103BEA", LOGIC_FAMILY_ALS, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SN54ALS112AJ", LOGIC_FAMILY_ALS, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SN74ALS112AD", LOGIC_FAMILY_ALS, PACKAGE_TYPE_SOIC, 0), \
    X("SN74ALS112AN", LOGIC_FAMILY_ALS, PACKAGE_TYPE_DIP, 0), \
    X("SN74ALS112ANSR", LOGIC_FAMILY_ALS, PACKAGE_TYPE_SOP, 0), \
    X("SNJ54ALS112AJ", LOGIC_FAMILY_ALS, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SN74F112D", LOGIC_FAMILY_F, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("SN74F112DR", LOGIC_FAMILY_F, PACKAGE_TYPE_SOIC, 0), \
    X("SN74F112N", LOGIC_FAMILY_F, PACKAGE_TYPE_DIP, 0), \
    X("SN74F112NE4", LOGIC_FAMILY_F, PACKAGE_TYPE_DIP, 0), \
    X("SN74F112NSR", LOGIC_FAMILY_F, PACKAGE_TYPE_SOP, 0), \
    X("8408801EA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("8408801FA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP, PART_FLAG_MILITARY), \
    X("JM38510/65305BEA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("M38510/65305BEA", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SN54HC112J", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SN74HC112D", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("SN74HC112DR", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC, 0), \
    X("SN74HC112DRG4", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC, 0), \
    X("SN74HC112DT", LOGIC_FAMILY_HC, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("SN74HC112N", LOGIC_FAMILY_HC, PACKAGE_TYPE_DIP, 0), \
    X("SNJ54HC112J", LOGIC_FAMILY_HC, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SNJ54HC112W", LOGIC_FAMILY_HC, PACKAGE_TYPE_CFP, PART_FLAG_MILITARY), \
    X("SN54LS112AJ", LOGIC_FAMILY_LS, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SN54S112J", LOGIC_FAMILY_S, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SN74LS112AD", LOGIC_FAMILY_LS, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("SN74LS112ADR", LOGIC_FAMILY_LS, PACKAGE_TYPE_SOIC, 0), \
    X("SN74LS112AN", LOGIC_FAMILY_LS, PACKAGE_TYPE_DIP, 0), \
    X("SN74LS112ANSR", LOGIC_FAMILY_LS, PACKAGE_TYPE_SOP, 0), \
    X("SN74S112AN", LOGIC_FAMILY_S, PACKAGE_TYPE_DIP, 0), \
    X("SNJ54LS112AJ", LOGIC_FAMILY_LS, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SNJ54LS112AW", LOGIC_FAMILY_LS, PACKAGE_TYPE_CFP, PART_FLAG_MILITARY), \
    X("SNJ54S112J", LOGIC_FAMILY_S, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("SNJ54S112W", LOGIC_FAMILY_S, PACKAGE_TYPE_CFP, PART_FLAG_MILITARY), \
    X("SN74LVC112AD", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOIC, 0), \
    X("SN74LVC112ADBR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SSOP, 0), \
    X("SN74LVC112ADGVR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_TVSOP, 0), \
    X("SN74LVC112ADR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOIC, 0), \
    X("SN74LVC112ADT", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOIC, 0), \
    X("SN74LVC112ANSR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_SOP, 0), \
    X("SN74LVC112APW", LOGIC_FAMILY_LVC, PACKAGE_TYPE_TSSOP, 0), \
    X("SN74LVC112APWR", LOGIC_FAMILY_LVC, PACKAGE_TYPE_TSSOP, 0), \
    X("SN74LVC112APWRG4", LOGIC_FAMILY_LVC, PACKAGE_TYPE_TSSOP, 0), \
    X("SN74LVC112APWT", LOGIC_FAMILY_LVC, PACKAGE_TYPE_TSSOP, 0)

FF_DEFINE_CHIP(FF_74_112, "74_112", "JK flip-flop",
    "Dual negative-edge-triggered J-K flip-flop with preset and clear",
    LAYOUT_DUAL112, PINS_DUAL112, FAMILIES_74_112, PARTS_74_112, 2)

/* ---- CMOS 4000 series --------------------------------------------------- */

/* CD4027B: SET and RESET are active high. Pins: 1 2Q ... 16 VDD. */
static const FfUnit UNITS_4027B[] = {
    /* j, k, clk, set, reset, q, qn */
    {FF_PIN(10), FF_PIN(11), FF_PIN(13), FF_PIN(9), FF_PIN(12), FF_PIN(15), FF_PIN(14)},
    {FF_PIN(6), FF_PIN(5), FF_PIN(3), FF_PIN(7), FF_PIN(4), FF_PIN(1), FF_PIN(2)},
};

static const FfLayout LAYOUT_4027B = {16, 2, UNITS_4027B, 0, 1, 1, 0};

static const ChipPin PINS_4027B[] = {
    {1, "2Q", PIN_ROLE_OUTPUT},
    {2, "2QBAR", PIN_ROLE_OUTPUT},
    {3, "2CLK", PIN_ROLE_INPUT},
    {4, "2RESET", PIN_ROLE_INPUT},
    {5, "2K", PIN_ROLE_INPUT},
    {6, "2J", PIN_ROLE_INPUT},
    {7, "2SET", PIN_ROLE_INPUT},
    {8, "VSS", PIN_ROLE_GROUND},
    {9, "1SET", PIN_ROLE_INPUT},
    {10, "1J", PIN_ROLE_INPUT},
    {11, "1K", PIN_ROLE_INPUT},
    {12, "1RESET", PIN_ROLE_INPUT},
    {13, "1CLK", PIN_ROLE_INPUT},
    {14, "1QBAR", PIN_ROLE_OUTPUT},
    {15, "1Q", PIN_ROLE_OUTPUT},
    {16, "VDD", PIN_ROLE_POWER},
};

static const FfFamily FAMILIES_4027B[] = {
    {LOGIC_FAMILY_C, 150000, 150000},  /* typical at VDD 5 V, 25 C (50 pF) */
};

#define PARTS_4027B(X) \
    X("CD4027BE", LOGIC_FAMILY_C, PACKAGE_TYPE_DIP, 0), \
    X("CD4027BEE4", LOGIC_FAMILY_C, PACKAGE_TYPE_DIP, 0), \
    X("CD4027BF", LOGIC_FAMILY_C, PACKAGE_TYPE_CDIP, 0), \
    X("CD4027BF3A", LOGIC_FAMILY_C, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("CD4027BM", LOGIC_FAMILY_C, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("CD4027BM96", LOGIC_FAMILY_C, PACKAGE_TYPE_SOIC, 0), \
    X("CD4027BM96E4", LOGIC_FAMILY_C, PACKAGE_TYPE_SOIC, 0), \
    X("CD4027BMT", LOGIC_FAMILY_C, PACKAGE_TYPE_SOIC, PART_FLAG_OBSOLETE), \
    X("CD4027BNSR", LOGIC_FAMILY_C, PACKAGE_TYPE_SOP, 0), \
    X("CD4027BPW", LOGIC_FAMILY_C, PACKAGE_TYPE_TSSOP, PART_FLAG_OBSOLETE), \
    X("CD4027BPWR", LOGIC_FAMILY_C, PACKAGE_TYPE_TSSOP, 0), \
    X("JM38510/05152BEA", LOGIC_FAMILY_C, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY), \
    X("M38510/05152BEA", LOGIC_FAMILY_C, PACKAGE_TYPE_CDIP, PART_FLAG_MILITARY)

FF_DEFINE_CHIP(FF_4027B, "4027B", "JK flip-flop",
    "CMOS dual J-K flip-flop with active-high set and reset",
    LAYOUT_4027B, PINS_4027B, FAMILIES_4027B, PARTS_4027B, 2)

/* ---- Alternate packages ------------------------------------------------- */
/*
 * Same flip-flops in the 20-pin LCCC (FK). Wiring is the 16-pin layout
 * remapped to the package pins from the datasheet FK drawings (SN54HC109,
 * SN54ALS109A, SN54F109; SN54HC112, SN54LS112A, SN54ALS112A). NC on pins 1,
 * 6, 11 and 16.
 */

static const FfUnit UNITS_DUAL109_FK[] = {
    /* j, k (KBAR), clk, pre, clr, q, qn */
    {FF_PIN(3), FF_PIN(4), FF_PIN(5), FF_PIN(7), FF_PIN(2), FF_PIN(8), FF_PIN(9)},
    {FF_PIN(18), FF_PIN(17), FF_PIN(15), FF_PIN(14), FF_PIN(19), FF_PIN(13), FF_PIN(12)},
};
static const FfLayout LAYOUT_DUAL109_FK = {20, 2, UNITS_DUAL109_FK, 1, 0, 0, 0};
static const ChipPin PINS_DUAL109_FK[] = {
    {1, "NC", PIN_ROLE_NO_CONNECT},
    {2, "1CLR", PIN_ROLE_INPUT},
    {3, "1J", PIN_ROLE_INPUT},
    {4, "1KBAR", PIN_ROLE_INPUT},
    {5, "1CLK", PIN_ROLE_INPUT},
    {6, "NC", PIN_ROLE_NO_CONNECT},
    {7, "1PRE", PIN_ROLE_INPUT},
    {8, "1Q", PIN_ROLE_OUTPUT},
    {9, "1QBAR", PIN_ROLE_OUTPUT},
    {10, "GND", PIN_ROLE_GROUND},
    {11, "NC", PIN_ROLE_NO_CONNECT},
    {12, "2QBAR", PIN_ROLE_OUTPUT},
    {13, "2Q", PIN_ROLE_OUTPUT},
    {14, "2PRE", PIN_ROLE_INPUT},
    {15, "2CLK", PIN_ROLE_INPUT},
    {16, "NC", PIN_ROLE_NO_CONNECT},
    {17, "2KBAR", PIN_ROLE_INPUT},
    {18, "2J", PIN_ROLE_INPUT},
    {19, "2CLR", PIN_ROLE_INPUT},
    {20, "VCC", PIN_ROLE_POWER},
};
#define PARTS_74_109_FK(X) \
    X("84000012A", LOGIC_FAMILY_ALS, PACKAGE_TYPE_LCCC, PART_FLAG_MILITARY), \
    X("SNJ54ALS109AFK", LOGIC_FAMILY_ALS, PACKAGE_TYPE_LCCC, PART_FLAG_MILITARY), \
    X("5962-9758001Q2A", LOGIC_FAMILY_F, PACKAGE_TYPE_LCCC, PART_FLAG_MILITARY), \
    X("JM38510/34102B2A", LOGIC_FAMILY_F, PACKAGE_TYPE_LCCC, PART_FLAG_MILITARY), \
    X("M38510/34102B2A", LOGIC_FAMILY_F, PACKAGE_TYPE_LCCC, PART_FLAG_MILITARY), \
    X("SNJ54F109FK", LOGIC_FAMILY_F, PACKAGE_TYPE_LCCC, PART_FLAG_MILITARY), \
    X("84150012A", LOGIC_FAMILY_HC, PACKAGE_TYPE_LCCC, PART_FLAG_MILITARY), \
    X("SNJ54HC109FK", LOGIC_FAMILY_HC, PACKAGE_TYPE_LCCC, PART_FLAG_MILITARY)
FF_DEFINE_CHIP(FF_74_109_FK, "74_109_FK", "JK flip-flop",
    "Dual J-KBAR positive-edge-triggered flip-flop with preset and clear, 20-pin LCCC (FK)",
    LAYOUT_DUAL109_FK, PINS_DUAL109_FK, FAMILIES_74_109, PARTS_74_109_FK, 2)

static const FfUnit UNITS_DUAL112_FK[] = {
    /* j, k, clk, pre, clr, q, qn */
    {FF_PIN(4), FF_PIN(3), FF_PIN(2), FF_PIN(5), FF_PIN(19), FF_PIN(7), FF_PIN(8)},
    {FF_PIN(14), FF_PIN(15), FF_PIN(17), FF_PIN(13), FF_PIN(18), FF_PIN(12), FF_PIN(9)},
};
static const FfLayout LAYOUT_DUAL112_FK = {20, 2, UNITS_DUAL112_FK, 0, 0, 0, 1};
static const ChipPin PINS_DUAL112_FK[] = {
    {1, "NC", PIN_ROLE_NO_CONNECT},
    {2, "1CLK", PIN_ROLE_INPUT},
    {3, "1K", PIN_ROLE_INPUT},
    {4, "1J", PIN_ROLE_INPUT},
    {5, "1PRE", PIN_ROLE_INPUT},
    {6, "NC", PIN_ROLE_NO_CONNECT},
    {7, "1Q", PIN_ROLE_OUTPUT},
    {8, "1QBAR", PIN_ROLE_OUTPUT},
    {9, "2QBAR", PIN_ROLE_OUTPUT},
    {10, "GND", PIN_ROLE_GROUND},
    {11, "NC", PIN_ROLE_NO_CONNECT},
    {12, "2Q", PIN_ROLE_OUTPUT},
    {13, "2PRE", PIN_ROLE_INPUT},
    {14, "2J", PIN_ROLE_INPUT},
    {15, "2K", PIN_ROLE_INPUT},
    {16, "NC", PIN_ROLE_NO_CONNECT},
    {17, "2CLK", PIN_ROLE_INPUT},
    {18, "2CLR", PIN_ROLE_INPUT},
    {19, "1CLR", PIN_ROLE_INPUT},
    {20, "VCC", PIN_ROLE_POWER},
};
#define PARTS_74_112_FK(X) \
    X("JM38510/37103B2A", LOGIC_FAMILY_ALS, PACKAGE_TYPE_LCCC, PART_FLAG_MILITARY), \
    X("M38510/37103B2A", LOGIC_FAMILY_ALS, PACKAGE_TYPE_LCCC, PART_FLAG_MILITARY), \
    X("84088012A", LOGIC_FAMILY_HC, PACKAGE_TYPE_LCCC, PART_FLAG_MILITARY), \
    X("SNJ54HC112FK", LOGIC_FAMILY_HC, PACKAGE_TYPE_LCCC, PART_FLAG_MILITARY), \
    X("SNJ54LS112AFK", LOGIC_FAMILY_LS, PACKAGE_TYPE_LCCC, PART_FLAG_MILITARY), \
    X("SNJ54S112FK", LOGIC_FAMILY_S, PACKAGE_TYPE_LCCC, PART_FLAG_MILITARY)
FF_DEFINE_CHIP(FF_74_112_FK, "74_112_FK", "JK flip-flop",
    "Dual negative-edge-triggered J-K flip-flop with preset and clear, 20-pin LCCC (FK)",
    LAYOUT_DUAL112_FK, PINS_DUAL112_FK, FAMILIES_74_112, PARTS_74_112_FK, 2)
