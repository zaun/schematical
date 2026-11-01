#include "supply.h"

#include <string.h>

static const ChipPin VCC_PINS[] = {
    {1, "VCC", PIN_ROLE_POWER},
};

static const ChipPin GND_PINS[] = {
    {1, "GND", PIN_ROLE_GROUND},
};

typedef struct {
    const char *name;
    SupplyConfig config;
} SupplyPart;

static const SupplyPart PARTS[] = {
    {"VCC", {SUPPLY_VCC}},
    {"GND", {SUPPLY_GND}},
};

const ChipPin *supply_pins(SupplyKind kind) {
    return kind == SUPPLY_GND ? GND_PINS : VCC_PINS;
}

size_t supply_part_count(void) {
    return sizeof(PARTS) / sizeof(PARTS[0]);
}

const char *supply_part_name_at(size_t index) {
    return index < supply_part_count() ? PARTS[index].name : NULL;
}

int supply_part_config_at(size_t index, SupplyConfig *config) {
    if (index >= supply_part_count() || config == NULL) {
        return -1;
    }
    *config = PARTS[index].config;
    return 0;
}

/* ---- Simulation models -------------------------------------------------- */

static size_t supply_pin_count(const void *config) {
    (void)config;
    return 1;
}

static void vcc_start(Sim *sim, SimInstance *instance, void *state) {
    (void)state;
    sim_schedule_drive(sim, instance, 0, LOGIC_1, DRIVE_STRONG, 0);
}

static void gnd_start(Sim *sim, SimInstance *instance, void *state) {
    (void)state;
    sim_schedule_drive(sim, instance, 0, LOGIC_0, DRIVE_STRONG, 0);
}

/* Stateless: the drive is set once at start and never changes. */
const SimComponentType SUPPLY_VCC_TYPE = {
    "VCC",
    supply_pin_count,
    NULL,
    NULL,
    vcc_start,
    NULL,
    NULL,
};

const SimComponentType SUPPLY_GND_TYPE = {
    "GND",
    supply_pin_count,
    NULL,
    NULL,
    gnd_start,
    NULL,
    NULL,
};

const SimComponentType *supply_lookup(const char *part_number, SupplyConfig *config) {
    size_t i;

    if (part_number == NULL) {
        return NULL;
    }

    for (i = 0; i < supply_part_count(); i++) {
        if (strcmp(PARTS[i].name, part_number) == 0) {
            if (config != NULL) {
                *config = PARTS[i].config;
            }
            return PARTS[i].config.kind == SUPPLY_GND ? &SUPPLY_GND_TYPE
                                                       : &SUPPLY_VCC_TYPE;
        }
    }

    return NULL;
}
