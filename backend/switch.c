#include "switch.h"

#include <stdlib.h>
#include <string.h>

/* ---- Pin tables ---------------------------------------------------------
 * Position k joins pin k ("kA") and pin 2N + 1 - k ("kB"). Every pin is an
 * in/out: a closed contact passes values both ways.
 */

static const ChipPin PINS_2[] = {
    {1, "1A", PIN_ROLE_INOUT},
    {2, "2A", PIN_ROLE_INOUT},
    {3, "2B", PIN_ROLE_INOUT},
    {4, "1B", PIN_ROLE_INOUT},
};

static const ChipPin PINS_4[] = {
    {1, "1A", PIN_ROLE_INOUT},
    {2, "2A", PIN_ROLE_INOUT},
    {3, "3A", PIN_ROLE_INOUT},
    {4, "4A", PIN_ROLE_INOUT},
    {5, "4B", PIN_ROLE_INOUT},
    {6, "3B", PIN_ROLE_INOUT},
    {7, "2B", PIN_ROLE_INOUT},
    {8, "1B", PIN_ROLE_INOUT},
};

static const ChipPin PINS_6[] = {
    {1, "1A", PIN_ROLE_INOUT},
    {2, "2A", PIN_ROLE_INOUT},
    {3, "3A", PIN_ROLE_INOUT},
    {4, "4A", PIN_ROLE_INOUT},
    {5, "5A", PIN_ROLE_INOUT},
    {6, "6A", PIN_ROLE_INOUT},
    {7, "6B", PIN_ROLE_INOUT},
    {8, "5B", PIN_ROLE_INOUT},
    {9, "4B", PIN_ROLE_INOUT},
    {10, "3B", PIN_ROLE_INOUT},
    {11, "2B", PIN_ROLE_INOUT},
    {12, "1B", PIN_ROLE_INOUT},
};

static const ChipPin PINS_8[] = {
    {1, "1A", PIN_ROLE_INOUT},
    {2, "2A", PIN_ROLE_INOUT},
    {3, "3A", PIN_ROLE_INOUT},
    {4, "4A", PIN_ROLE_INOUT},
    {5, "5A", PIN_ROLE_INOUT},
    {6, "6A", PIN_ROLE_INOUT},
    {7, "7A", PIN_ROLE_INOUT},
    {8, "8A", PIN_ROLE_INOUT},
    {9, "8B", PIN_ROLE_INOUT},
    {10, "7B", PIN_ROLE_INOUT},
    {11, "6B", PIN_ROLE_INOUT},
    {12, "5B", PIN_ROLE_INOUT},
    {13, "4B", PIN_ROLE_INOUT},
    {14, "3B", PIN_ROLE_INOUT},
    {15, "2B", PIN_ROLE_INOUT},
    {16, "1B", PIN_ROLE_INOUT},
};

static const ChipPin PINS_10[] = {
    {1, "1A", PIN_ROLE_INOUT},
    {2, "2A", PIN_ROLE_INOUT},
    {3, "3A", PIN_ROLE_INOUT},
    {4, "4A", PIN_ROLE_INOUT},
    {5, "5A", PIN_ROLE_INOUT},
    {6, "6A", PIN_ROLE_INOUT},
    {7, "7A", PIN_ROLE_INOUT},
    {8, "8A", PIN_ROLE_INOUT},
    {9, "9A", PIN_ROLE_INOUT},
    {10, "10A", PIN_ROLE_INOUT},
    {11, "10B", PIN_ROLE_INOUT},
    {12, "9B", PIN_ROLE_INOUT},
    {13, "8B", PIN_ROLE_INOUT},
    {14, "7B", PIN_ROLE_INOUT},
    {15, "6B", PIN_ROLE_INOUT},
    {16, "5B", PIN_ROLE_INOUT},
    {17, "4B", PIN_ROLE_INOUT},
    {18, "3B", PIN_ROLE_INOUT},
    {19, "2B", PIN_ROLE_INOUT},
    {20, "1B", PIN_ROLE_INOUT},
};

#define SWITCH_COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- Series ------------------------------------------------------------- */

typedef enum {
    SWITCH_SERIES_GDS,
    SWITCH_SERIES_GDP,
    SWITCH_SERIES_GDR,
    SWITCH_SERIES_GDH,
    SWITCH_SERIES_GDHL,
    SWITCH_SERIES_ADE,
    SWITCH_SERIES_ADF,
    SWITCH_SERIES_COUNT
} SwitchSeries;

/* Actuator style, from the datasheet's series descriptions (pages 2 and 3). */
static const char *const SERIES_FAMILY[SWITCH_SERIES_COUNT] = {
    "Low profile",
    "Piano actuator",
    "Recessed rocker",
    "Half pitch",
    "Ultra low profile",
    "Extended actuator",
    "Flush actuator",
};

/* ---- Orderable parts ---------------------------------------------------- */

typedef struct {
    const char *name;
    SwitchSeries series;
    size_t positions;
    PackageType package;  /* DIP for THT, SMD for SMT */
} SwitchPart;

/* Tube parts from "Preferred Part Numbers - DIP Switches" (datasheet page 14). */
static const SwitchPart PARTS[] = {
    /* GDS series */
    {"TE-GDS0204", SWITCH_SERIES_GDS, 2, PACKAGE_TYPE_DIP},
    {"TE-GDS02S04", SWITCH_SERIES_GDS, 2, PACKAGE_TYPE_SMD},
    {"TE-GDS0404", SWITCH_SERIES_GDS, 4, PACKAGE_TYPE_DIP},
    {"TE-GDS04S04", SWITCH_SERIES_GDS, 4, PACKAGE_TYPE_SMD},
    {"TE-GDS0604", SWITCH_SERIES_GDS, 6, PACKAGE_TYPE_DIP},
    {"TE-GDS06S04", SWITCH_SERIES_GDS, 6, PACKAGE_TYPE_SMD},
    {"TE-GDS0804", SWITCH_SERIES_GDS, 8, PACKAGE_TYPE_DIP},
    {"TE-GDS08S04", SWITCH_SERIES_GDS, 8, PACKAGE_TYPE_SMD},
    {"TE-GDS1004", SWITCH_SERIES_GDS, 10, PACKAGE_TYPE_DIP},
    {"TE-GDS10S04", SWITCH_SERIES_GDS, 10, PACKAGE_TYPE_SMD},
    /* GDP series */
    {"TE-GDP0204", SWITCH_SERIES_GDP, 2, PACKAGE_TYPE_DIP},
    {"TE-GDP0404", SWITCH_SERIES_GDP, 4, PACKAGE_TYPE_DIP},
    {"TE-GDP0604", SWITCH_SERIES_GDP, 6, PACKAGE_TYPE_DIP},
    {"TE-GDP0804", SWITCH_SERIES_GDP, 8, PACKAGE_TYPE_DIP},
    {"TE-GDP1004", SWITCH_SERIES_GDP, 10, PACKAGE_TYPE_DIP},
    /* GDR series */
    {"TE-GDR0404", SWITCH_SERIES_GDR, 4, PACKAGE_TYPE_DIP},
    {"TE-GDR04S04", SWITCH_SERIES_GDR, 4, PACKAGE_TYPE_SMD},
    {"TE-GDR0804", SWITCH_SERIES_GDR, 8, PACKAGE_TYPE_DIP},
    {"TE-GDR08S04", SWITCH_SERIES_GDR, 8, PACKAGE_TYPE_SMD},
    /* GDH series */
    {"TE-GDH02S04", SWITCH_SERIES_GDH, 2, PACKAGE_TYPE_SMD},
    {"TE-GDH04S04", SWITCH_SERIES_GDH, 4, PACKAGE_TYPE_SMD},
    {"TE-GDH08S04", SWITCH_SERIES_GDH, 8, PACKAGE_TYPE_SMD},
    /* GDHL series */
    {"TE-GDHL04S", SWITCH_SERIES_GDHL, 4, PACKAGE_TYPE_SMD},
    {"TE-GDHL08S", SWITCH_SERIES_GDHL, 8, PACKAGE_TYPE_SMD},
    /* ADE series */
    {"TE-ADE0204", SWITCH_SERIES_ADE, 2, PACKAGE_TYPE_DIP},
    {"TE-ADE0404", SWITCH_SERIES_ADE, 4, PACKAGE_TYPE_DIP},
    {"TE-ADE04S04", SWITCH_SERIES_ADE, 4, PACKAGE_TYPE_SMD},
    {"TE-ADE0604", SWITCH_SERIES_ADE, 6, PACKAGE_TYPE_DIP},
    {"TE-ADE0804", SWITCH_SERIES_ADE, 8, PACKAGE_TYPE_DIP},
    {"TE-ADE08S04", SWITCH_SERIES_ADE, 8, PACKAGE_TYPE_SMD},
    {"TE-ADE1004", SWITCH_SERIES_ADE, 10, PACKAGE_TYPE_DIP},
    /* ADF series */
    {"TE-ADF0204", SWITCH_SERIES_ADF, 2, PACKAGE_TYPE_DIP},
    {"TE-ADF0404", SWITCH_SERIES_ADF, 4, PACKAGE_TYPE_DIP},
    {"TE-ADF0604", SWITCH_SERIES_ADF, 6, PACKAGE_TYPE_DIP},
    {"TE-ADF0804", SWITCH_SERIES_ADF, 8, PACKAGE_TYPE_DIP},
    {"TE-ADF1004", SWITCH_SERIES_ADF, 10, PACKAGE_TYPE_DIP},
};

const SimComponentType *switch_lookup(const char *part_number, SimLogicConfig *config) {
    size_t i;

    if (part_number == NULL) {
        return NULL;
    }
    for (i = 0; i < SWITCH_COUNT(PARTS); i++) {
        if (strcmp(PARTS[i].name, part_number) == 0) {
            if (config != NULL) {
                config->package = PARTS[i].package;
                config->family = LOGIC_FAMILY_SWITCH;
                config->model_data = &PARTS[i];
                config->variant = 0;
            }
            return &SWITCH_TYPE;
        }
    }
    return NULL;
}

static const SwitchPart *config_part(const SimLogicConfig *config) {
    if (config == NULL || config->family != LOGIC_FAMILY_SWITCH) {
        return NULL;
    }
    return config->model_data;
}

size_t switch_config_positions(const SimLogicConfig *config) {
    const SwitchPart *part = config_part(config);

    return part != NULL ? part->positions : 0;
}

const char *switch_family_label(const SimLogicConfig *config) {
    const SwitchPart *part = config_part(config);

    return part != NULL ? SERIES_FAMILY[part->series] : "unknown";
}

/* ---- Catalog definitions ------------------------------------------------
 * One per series and position count, so each has a single pin table. The
 * count shown in the drawer (gate_count) is the number of positions.
 */

static const char *const GDS_02_NAMES[] = {"TE-GDS0204", "TE-GDS02S04"};
static const ChipDefinition GDS_02_DEFINITION = {
    "te_gds_02", "GDS", "GDS low-profile DIP switch, 2 positions", "Switch",
    GDS_02_NAMES, SWITCH_COUNT(GDS_02_NAMES), PINS_2, SWITCH_COUNT(PINS_2),
    switch_lookup, 2, NULL, "DIP"
};

static const char *const GDS_04_NAMES[] = {"TE-GDS0404", "TE-GDS04S04"};
static const ChipDefinition GDS_04_DEFINITION = {
    "te_gds_04", "GDS", "GDS low-profile DIP switch, 4 positions", "Switch",
    GDS_04_NAMES, SWITCH_COUNT(GDS_04_NAMES), PINS_4, SWITCH_COUNT(PINS_4),
    switch_lookup, 4, NULL, "DIP"
};

static const char *const GDS_06_NAMES[] = {"TE-GDS0604", "TE-GDS06S04"};
static const ChipDefinition GDS_06_DEFINITION = {
    "te_gds_06", "GDS", "GDS low-profile DIP switch, 6 positions", "Switch",
    GDS_06_NAMES, SWITCH_COUNT(GDS_06_NAMES), PINS_6, SWITCH_COUNT(PINS_6),
    switch_lookup, 6, NULL, "DIP"
};

static const char *const GDS_08_NAMES[] = {"TE-GDS0804", "TE-GDS08S04"};
static const ChipDefinition GDS_08_DEFINITION = {
    "te_gds_08", "GDS", "GDS low-profile DIP switch, 8 positions", "Switch",
    GDS_08_NAMES, SWITCH_COUNT(GDS_08_NAMES), PINS_8, SWITCH_COUNT(PINS_8),
    switch_lookup, 8, NULL, "DIP"
};

static const char *const GDS_10_NAMES[] = {"TE-GDS1004", "TE-GDS10S04"};
static const ChipDefinition GDS_10_DEFINITION = {
    "te_gds_10", "GDS", "GDS low-profile DIP switch, 10 positions", "Switch",
    GDS_10_NAMES, SWITCH_COUNT(GDS_10_NAMES), PINS_10, SWITCH_COUNT(PINS_10),
    switch_lookup, 10, NULL, "DIP"
};

static const char *const GDP_02_NAMES[] = {"TE-GDP0204"};
static const ChipDefinition GDP_02_DEFINITION = {
    "te_gdp_02", "GDP", "GDP piano DIP switch, 2 positions", "Switch",
    GDP_02_NAMES, SWITCH_COUNT(GDP_02_NAMES), PINS_2, SWITCH_COUNT(PINS_2),
    switch_lookup, 2, NULL, "DIP"
};

static const char *const GDP_04_NAMES[] = {"TE-GDP0404"};
static const ChipDefinition GDP_04_DEFINITION = {
    "te_gdp_04", "GDP", "GDP piano DIP switch, 4 positions", "Switch",
    GDP_04_NAMES, SWITCH_COUNT(GDP_04_NAMES), PINS_4, SWITCH_COUNT(PINS_4),
    switch_lookup, 4, NULL, "DIP"
};

static const char *const GDP_06_NAMES[] = {"TE-GDP0604"};
static const ChipDefinition GDP_06_DEFINITION = {
    "te_gdp_06", "GDP", "GDP piano DIP switch, 6 positions", "Switch",
    GDP_06_NAMES, SWITCH_COUNT(GDP_06_NAMES), PINS_6, SWITCH_COUNT(PINS_6),
    switch_lookup, 6, NULL, "DIP"
};

static const char *const GDP_08_NAMES[] = {"TE-GDP0804"};
static const ChipDefinition GDP_08_DEFINITION = {
    "te_gdp_08", "GDP", "GDP piano DIP switch, 8 positions", "Switch",
    GDP_08_NAMES, SWITCH_COUNT(GDP_08_NAMES), PINS_8, SWITCH_COUNT(PINS_8),
    switch_lookup, 8, NULL, "DIP"
};

static const char *const GDP_10_NAMES[] = {"TE-GDP1004"};
static const ChipDefinition GDP_10_DEFINITION = {
    "te_gdp_10", "GDP", "GDP piano DIP switch, 10 positions", "Switch",
    GDP_10_NAMES, SWITCH_COUNT(GDP_10_NAMES), PINS_10, SWITCH_COUNT(PINS_10),
    switch_lookup, 10, NULL, "DIP"
};

static const char *const GDR_04_NAMES[] = {"TE-GDR0404", "TE-GDR04S04"};
static const ChipDefinition GDR_04_DEFINITION = {
    "te_gdr_04", "GDR", "GDR recessed-rocker DIP switch, 4 positions", "Switch",
    GDR_04_NAMES, SWITCH_COUNT(GDR_04_NAMES), PINS_4, SWITCH_COUNT(PINS_4),
    switch_lookup, 4, NULL, "DIP"
};

static const char *const GDR_08_NAMES[] = {"TE-GDR0804", "TE-GDR08S04"};
static const ChipDefinition GDR_08_DEFINITION = {
    "te_gdr_08", "GDR", "GDR recessed-rocker DIP switch, 8 positions", "Switch",
    GDR_08_NAMES, SWITCH_COUNT(GDR_08_NAMES), PINS_8, SWITCH_COUNT(PINS_8),
    switch_lookup, 8, NULL, "DIP"
};

static const char *const GDH_02_NAMES[] = {"TE-GDH02S04"};
static const ChipDefinition GDH_02_DEFINITION = {
    "te_gdh_02", "GDH", "GDH half-pitch DIP switch, 2 positions", "Switch",
    GDH_02_NAMES, SWITCH_COUNT(GDH_02_NAMES), PINS_2, SWITCH_COUNT(PINS_2),
    switch_lookup, 2, NULL, "DIP"
};

static const char *const GDH_04_NAMES[] = {"TE-GDH04S04"};
static const ChipDefinition GDH_04_DEFINITION = {
    "te_gdh_04", "GDH", "GDH half-pitch DIP switch, 4 positions", "Switch",
    GDH_04_NAMES, SWITCH_COUNT(GDH_04_NAMES), PINS_4, SWITCH_COUNT(PINS_4),
    switch_lookup, 4, NULL, "DIP"
};

static const char *const GDH_08_NAMES[] = {"TE-GDH08S04"};
static const ChipDefinition GDH_08_DEFINITION = {
    "te_gdh_08", "GDH", "GDH half-pitch DIP switch, 8 positions", "Switch",
    GDH_08_NAMES, SWITCH_COUNT(GDH_08_NAMES), PINS_8, SWITCH_COUNT(PINS_8),
    switch_lookup, 8, NULL, "DIP"
};

static const char *const GDHL_04_NAMES[] = {"TE-GDHL04S"};
static const ChipDefinition GDHL_04_DEFINITION = {
    "te_gdhl_04", "GDHL", "GDHL ultra-low-profile half-pitch DIP switch, 4 positions", "Switch",
    GDHL_04_NAMES, SWITCH_COUNT(GDHL_04_NAMES), PINS_4, SWITCH_COUNT(PINS_4),
    switch_lookup, 4, NULL, "DIP"
};

static const char *const GDHL_08_NAMES[] = {"TE-GDHL08S"};
static const ChipDefinition GDHL_08_DEFINITION = {
    "te_gdhl_08", "GDHL", "GDHL ultra-low-profile half-pitch DIP switch, 8 positions", "Switch",
    GDHL_08_NAMES, SWITCH_COUNT(GDHL_08_NAMES), PINS_8, SWITCH_COUNT(PINS_8),
    switch_lookup, 8, NULL, "DIP"
};

static const char *const ADE_02_NAMES[] = {"TE-ADE0204"};
static const ChipDefinition ADE_02_DEFINITION = {
    "te_ade_02", "ADE", "ADE extended-actuator DIP switch, 2 positions", "Switch",
    ADE_02_NAMES, SWITCH_COUNT(ADE_02_NAMES), PINS_2, SWITCH_COUNT(PINS_2),
    switch_lookup, 2, NULL, "DIP"
};

static const char *const ADE_04_NAMES[] = {"TE-ADE0404", "TE-ADE04S04"};
static const ChipDefinition ADE_04_DEFINITION = {
    "te_ade_04", "ADE", "ADE extended-actuator DIP switch, 4 positions", "Switch",
    ADE_04_NAMES, SWITCH_COUNT(ADE_04_NAMES), PINS_4, SWITCH_COUNT(PINS_4),
    switch_lookup, 4, NULL, "DIP"
};

static const char *const ADE_06_NAMES[] = {"TE-ADE0604"};
static const ChipDefinition ADE_06_DEFINITION = {
    "te_ade_06", "ADE", "ADE extended-actuator DIP switch, 6 positions", "Switch",
    ADE_06_NAMES, SWITCH_COUNT(ADE_06_NAMES), PINS_6, SWITCH_COUNT(PINS_6),
    switch_lookup, 6, NULL, "DIP"
};

static const char *const ADE_08_NAMES[] = {"TE-ADE0804", "TE-ADE08S04"};
static const ChipDefinition ADE_08_DEFINITION = {
    "te_ade_08", "ADE", "ADE extended-actuator DIP switch, 8 positions", "Switch",
    ADE_08_NAMES, SWITCH_COUNT(ADE_08_NAMES), PINS_8, SWITCH_COUNT(PINS_8),
    switch_lookup, 8, NULL, "DIP"
};

static const char *const ADE_10_NAMES[] = {"TE-ADE1004"};
static const ChipDefinition ADE_10_DEFINITION = {
    "te_ade_10", "ADE", "ADE extended-actuator DIP switch, 10 positions", "Switch",
    ADE_10_NAMES, SWITCH_COUNT(ADE_10_NAMES), PINS_10, SWITCH_COUNT(PINS_10),
    switch_lookup, 10, NULL, "DIP"
};

static const char *const ADF_02_NAMES[] = {"TE-ADF0204"};
static const ChipDefinition ADF_02_DEFINITION = {
    "te_adf_02", "ADF", "ADF flush-actuator DIP switch, 2 positions", "Switch",
    ADF_02_NAMES, SWITCH_COUNT(ADF_02_NAMES), PINS_2, SWITCH_COUNT(PINS_2),
    switch_lookup, 2, NULL, "DIP"
};

static const char *const ADF_04_NAMES[] = {"TE-ADF0404"};
static const ChipDefinition ADF_04_DEFINITION = {
    "te_adf_04", "ADF", "ADF flush-actuator DIP switch, 4 positions", "Switch",
    ADF_04_NAMES, SWITCH_COUNT(ADF_04_NAMES), PINS_4, SWITCH_COUNT(PINS_4),
    switch_lookup, 4, NULL, "DIP"
};

static const char *const ADF_06_NAMES[] = {"TE-ADF0604"};
static const ChipDefinition ADF_06_DEFINITION = {
    "te_adf_06", "ADF", "ADF flush-actuator DIP switch, 6 positions", "Switch",
    ADF_06_NAMES, SWITCH_COUNT(ADF_06_NAMES), PINS_6, SWITCH_COUNT(PINS_6),
    switch_lookup, 6, NULL, "DIP"
};

static const char *const ADF_08_NAMES[] = {"TE-ADF0804"};
static const ChipDefinition ADF_08_DEFINITION = {
    "te_adf_08", "ADF", "ADF flush-actuator DIP switch, 8 positions", "Switch",
    ADF_08_NAMES, SWITCH_COUNT(ADF_08_NAMES), PINS_8, SWITCH_COUNT(PINS_8),
    switch_lookup, 8, NULL, "DIP"
};

static const char *const ADF_10_NAMES[] = {"TE-ADF1004"};
static const ChipDefinition ADF_10_DEFINITION = {
    "te_adf_10", "ADF", "ADF flush-actuator DIP switch, 10 positions", "Switch",
    ADF_10_NAMES, SWITCH_COUNT(ADF_10_NAMES), PINS_10, SWITCH_COUNT(PINS_10),
    switch_lookup, 10, NULL, "DIP"
};

static const ChipDefinition *const DEFINITIONS[] = {
    &GDS_02_DEFINITION,
    &GDS_04_DEFINITION,
    &GDS_06_DEFINITION,
    &GDS_08_DEFINITION,
    &GDS_10_DEFINITION,
    &GDP_02_DEFINITION,
    &GDP_04_DEFINITION,
    &GDP_06_DEFINITION,
    &GDP_08_DEFINITION,
    &GDP_10_DEFINITION,
    &GDR_04_DEFINITION,
    &GDR_08_DEFINITION,
    &GDH_02_DEFINITION,
    &GDH_04_DEFINITION,
    &GDH_08_DEFINITION,
    &GDHL_04_DEFINITION,
    &GDHL_08_DEFINITION,
    &ADE_02_DEFINITION,
    &ADE_04_DEFINITION,
    &ADE_06_DEFINITION,
    &ADE_08_DEFINITION,
    &ADE_10_DEFINITION,
    &ADF_02_DEFINITION,
    &ADF_04_DEFINITION,
    &ADF_06_DEFINITION,
    &ADF_08_DEFINITION,
    &ADF_10_DEFINITION,
};

size_t switch_definition_count(void) {
    return SWITCH_COUNT(DEFINITIONS);
}

const ChipDefinition *switch_definition_at(size_t index) {
    return index < switch_definition_count() ? DEFINITIONS[index] : NULL;
}

/* ---- Simulation model ---------------------------------------------------
 * The switch drives nothing. Each ON position closes a contact between its
 * two pins, and the simulator resolves the two nets as one.
 */

typedef struct {
    size_t positions;
} SwitchState;

static size_t switch_pin_count(const void *config) {
    return 2 * switch_config_positions(config);
}

static void *switch_create(const void *config) {
    SwitchState *state = malloc(sizeof(*state));

    if (state != NULL) {
        state->positions = switch_config_positions(config);
    }
    return state;
}

static void switch_destroy(void *state) {
    free(state);
}

const SimComponentType SWITCH_TYPE = {
    "SWITCH",
    switch_pin_count,
    switch_create,
    switch_destroy,
    NULL,
    NULL,
    NULL,
};

size_t switch_instance_positions(const SimInstance *instance) {
    const SwitchState *state;

    if (sim_instance_type(instance) != &SWITCH_TYPE) {
        return 0;
    }
    state = sim_instance_state(instance);
    return state != NULL ? state->positions : 0;
}

/* Pin indices (0-based) of a position's two pins: k - 1 and 2N - k. */
static int position_pins(const SimInstance *instance, size_t position,
                         size_t *pin_a, size_t *pin_b) {
    size_t positions = switch_instance_positions(instance);

    if (position < 1 || position > positions) {
        return 0;
    }
    *pin_a = position - 1;
    *pin_b = 2 * positions - position;
    return 1;
}

int switch_set_position(Sim *sim, SimInstance *instance, size_t position, int on) {
    size_t pin_a;
    size_t pin_b;

    if (!position_pins(instance, position, &pin_a, &pin_b)) {
        return 0;
    }
    sim_set_contact(sim, instance, pin_a, pin_b, on);
    return 1;
}

int switch_position_on(const Sim *sim, const SimInstance *instance, size_t position) {
    size_t pin_a;
    size_t pin_b;

    if (!position_pins(instance, position, &pin_a, &pin_b)) {
        return 0;
    }
    return sim_contact_closed(sim, instance, pin_a, pin_b);
}
