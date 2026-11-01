#include "osc.h"

#include <stdlib.h>
#include <string.h>

/* ---- Pin tables (datasheet pin numbers) --------------------------------- */

/* SiT1534 2012 SMD (datasheet Table 2). */
static const ChipPin SIT_SMD_PINS[] = {
    {1, "NC", PIN_ROLE_NO_CONNECT},
    {2, "GND", PIN_ROLE_GROUND},
    {3, "CLK OUT", PIN_ROLE_OUTPUT},
    {4, "VDD", PIN_ROLE_POWER},
};

/* SiT1534 1508 CSP (datasheet Table 3). Pins 1 and 4 are both ground. */
static const ChipPin SIT_CSP_PINS[] = {
    {1, "GND", PIN_ROLE_GROUND},
    {2, "CLK OUT", PIN_ROLE_OUTPUT},
    {3, "VDD", PIN_ROLE_POWER},
    {4, "GND", PIN_ROLE_GROUND},
};

/* LMK6C 4-pin VSON (datasheet Table 5-2). */
static const ChipPin LMK6C_PINS[] = {
    {1, "OE/ST/NC", PIN_ROLE_INPUT},
    {2, "GND", PIN_ROLE_GROUND},
    {3, "OUT", PIN_ROLE_OUTPUT},
    {4, "VDD", PIN_ROLE_POWER},
};

/* LMK6D/H/P 6-pin VSON (datasheet Table 5-1). Pins 1 and 2 are function pins. */
static const ChipPin LMK6_DIFF_PINS[] = {
    {1, "OE/ST/NC", PIN_ROLE_INPUT},
    {2, "OE/ST/NC", PIN_ROLE_INPUT},
    {3, "GND", PIN_ROLE_GROUND},
    {4, "OUTP", PIN_ROLE_OUTPUT},
    {5, "OUTN", PIN_ROLE_OUTPUT},
    {6, "VDD", PIN_ROLE_POWER},
};

static int is_differential(OscFamily family) {
    return family == OSC_FAMILY_LMK6D || family == OSC_FAMILY_LMK6H ||
           family == OSC_FAMILY_LMK6P;
}

const ChipPin *osc_pins(OscFamily family, OscPackage package) {
    switch (family) {
        case OSC_FAMILY_SIT1534:
            return package == OSC_PACKAGE_1508 ? SIT_CSP_PINS : SIT_SMD_PINS;
        case OSC_FAMILY_LMK6C:
            return LMK6C_PINS;
        case OSC_FAMILY_LMK6D:
        case OSC_FAMILY_LMK6H:
        case OSC_FAMILY_LMK6P:
            return LMK6_DIFF_PINS;
        case OSC_FAMILY_COUNT:
            break;
    }
    return SIT_SMD_PINS;
}

size_t osc_pin_count(OscFamily family, OscPackage package) {
    (void)package;
    return is_differential(family) ? sizeof(LMK6_DIFF_PINS) / sizeof(LMK6_DIFF_PINS[0])
                                   : sizeof(SIT_SMD_PINS) / sizeof(SIT_SMD_PINS[0]);
}

/* Output pin indices, in pin order. Returns how many were found (1 or 2). */
static size_t output_pins(OscFamily family, OscPackage package, size_t out[2]) {
    const ChipPin *pins = osc_pins(family, package);
    size_t count = osc_pin_count(family, package);
    size_t i;
    size_t n = 0;

    for (i = 0; i < count && n < 2; i++) {
        if (pins[i].role == PIN_ROLE_OUTPUT) {
            out[n++] = i;
        }
    }
    return n;
}

/* ---- Orderable parts ---------------------------------------------------- */

typedef struct {
    const char *name;
    OscFamily family;
    OscPackage package;
    uint64_t default_hz;
    int enable_pin;
} OscPart;

static const OscPart PARTS[] = {
#include "osc_parts.inc"
};

size_t osc_part_count(void) {
    return sizeof(PARTS) / sizeof(PARTS[0]);
}

const char *osc_part_name_at(size_t index) {
    return index < osc_part_count() ? PARTS[index].name : NULL;
}

int osc_part_config_at(size_t index, OscConfig *config) {
    if (index >= osc_part_count() || config == NULL) {
        return -1;
    }
    config->family = PARTS[index].family;
    config->package = PARTS[index].package;
    config->frequency_hz = PARTS[index].default_hz;
    config->enable_pin = PARTS[index].enable_pin;
    return 0;
}

static uint64_t family_min_hz(OscFamily family) {
    return family == OSC_FAMILY_SIT1534 ? 1 : 1000000;
}

static uint64_t family_max_hz(OscFamily family) {
    switch (family) {
        case OSC_FAMILY_SIT1534: return 32768;
        case OSC_FAMILY_LMK6C: return 200000000;
        case OSC_FAMILY_LMK6D:
        case OSC_FAMILY_LMK6H:
        case OSC_FAMILY_LMK6P: return 400000000;
        case OSC_FAMILY_COUNT: break;
    }
    return 0;
}

uint64_t osc_part_min_hz(size_t index) {
    return index < osc_part_count() ? family_min_hz(PARTS[index].family) : 0;
}

uint64_t osc_part_max_hz(size_t index) {
    return index < osc_part_count() ? family_max_hz(PARTS[index].family) : 0;
}

const char *osc_family_label(OscFamily family) {
    switch (family) {
        case OSC_FAMILY_SIT1534: return "SiT1534";
        case OSC_FAMILY_LMK6C: return "LMK6C";
        case OSC_FAMILY_LMK6D: return "LMK6D";
        case OSC_FAMILY_LMK6H: return "LMK6H";
        case OSC_FAMILY_LMK6P: return "LMK6P";
        case OSC_FAMILY_COUNT: break;
    }
    return "unknown";
}

const char *osc_package_label(OscPackage package) {
    switch (package) {
        case OSC_PACKAGE_2012: return "2012";
        case OSC_PACKAGE_1508: return "1508 CSP";
        case OSC_PACKAGE_3225: return "3225 VSON";
        case OSC_PACKAGE_2520: return "2520 VSON";
        case OSC_PACKAGE_COUNT: break;
    }
    return "unknown";
}

/* ---- Simulation model --------------------------------------------------- */

typedef struct {
    size_t out_pin;
    int comp_pin;     /* complementary output, -1 when there is none */
    int enable_pin;   /* -1 when there is none */
    SimTime half_period;
    LogicValue level;
} OscState;

static size_t osc_type_pin_count(const void *raw) {
    const OscConfig *config = raw;
    return osc_pin_count(config->family, config->package);
}

static void *osc_create(const void *raw) {
    const OscConfig *config = raw;
    OscState *state;
    size_t out[2];
    size_t n;

    if (config->frequency_hz == 0 ||
        (config->enable_pin >= 0 &&
         (size_t)config->enable_pin >= osc_pin_count(config->family, config->package))) {
        return NULL;
    }

    n = output_pins(config->family, config->package, out);
    if (n == 0) {
        return NULL;
    }

    state = malloc(sizeof(*state));
    if (state == NULL) {
        return NULL;
    }

    /* Half period in picoseconds, rounded to the nearest whole picosecond. */
    state->half_period = (1000000000000ULL + config->frequency_hz) /
                         (2 * config->frequency_hz);
    state->out_pin = out[0];
    state->comp_pin = n == 2 ? (int)out[1] : -1;
    state->enable_pin = config->enable_pin;
    state->level = LOGIC_0;
    return state;
}

static void osc_destroy(void *raw) {
    free(raw);
}

/*
 * Drives the outputs from the current level. A LOW on the function pin puts
 * every output in high impedance; NC or a driven HIGH keeps the clock running.
 */
static void drive_outputs(Sim *sim, SimInstance *instance, const OscState *state) {
    int enabled = state->enable_pin < 0 ||
                  sim_read_pin(sim, instance, (size_t)state->enable_pin) != LOGIC_0;
    LogicValue primary = enabled ? state->level : LOGIC_Z;
    LogicValue complement = !enabled ? LOGIC_Z
                          : state->level == LOGIC_0 ? LOGIC_1 : LOGIC_0;

    sim_schedule_drive(sim, instance, state->out_pin, primary, DRIVE_STRONG, 0);
    if (state->comp_pin >= 0) {
        sim_schedule_drive(sim, instance, (size_t)state->comp_pin, complement,
                           DRIVE_STRONG, 0);
    }
}

static void osc_start(Sim *sim, SimInstance *instance, void *raw) {
    OscState *state = raw;

    state->level = LOGIC_0;
    drive_outputs(sim, instance, state);
    sim_schedule_timer(sim, instance, 0, state->half_period);
}

static void osc_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t timer_id) {
    OscState *state = raw;

    (void)timer_id;
    state->level = state->level == LOGIC_0 ? LOGIC_1 : LOGIC_0;
    drive_outputs(sim, instance, state);
    sim_schedule_timer(sim, instance, 0, state->half_period);
}

static void osc_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                            LogicValue old_value, LogicValue new_value) {
    const OscState *state = raw;

    (void)old_value;
    (void)new_value;
    if (state->enable_pin >= 0 && pin == (size_t)state->enable_pin) {
        drive_outputs(sim, instance, state);
    }
}

const SimComponentType OSC_TYPE = {
    "OSC",
    osc_type_pin_count,
    osc_create,
    osc_destroy,
    osc_start,
    osc_pin_changed,
    osc_timer,
};

const SimComponentType *osc_lookup(const char *part_number, OscConfig *config) {
    size_t i;

    if (part_number == NULL) {
        return NULL;
    }

    for (i = 0; i < osc_part_count(); i++) {
        if (strcmp(PARTS[i].name, part_number) == 0) {
            if (config != NULL) {
                osc_part_config_at(i, config);
            }
            return &OSC_TYPE;
        }
    }

    return NULL;
}

/* ---- Frequency bands ---------------------------------------------------- */

typedef struct {
    const char *label;
    uint64_t low_hz;   /* inclusive */
    uint64_t high_hz;  /* exclusive; UINT64_MAX for the top band */
} FrequencyBand;

static const FrequencyBand BANDS[] = {
    {"Under 1 MHz", 0, 1000000},
    {"1 MHz to 10 MHz", 1000000, 10000000},
    {"10 MHz to 100 MHz", 10000000, 100000000},
    {"100 MHz and up", 100000000, UINT64_MAX},
};

size_t osc_group_count(void) {
    return sizeof(BANDS) / sizeof(BANDS[0]);
}

const char *osc_group_label(size_t group) {
    return group < osc_group_count() ? BANDS[group].label : NULL;
}

/* A part is listed in the band that holds its own frequency. */
int osc_part_in_group(size_t part, size_t group) {
    if (part >= osc_part_count() || group >= osc_group_count()) {
        return 0;
    }
    return PARTS[part].default_hz >= BANDS[group].low_hz &&
           PARTS[part].default_hz < BANDS[group].high_hz;
}
