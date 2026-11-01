#ifndef SUPPLY_H
#define SUPPLY_H

/*
 * Constant sources for the schematic:
 *
 *   VCC  drives its net to logic 1 (strong) for the whole run.
 *   GND  drives its net to logic 0 (strong) for the whole run. It acts as a
 *        sink that pulls the net low.
 *
 * Each has one pin: VCC is a power pin and GND a ground pin.
 */

#include <stddef.h>

#include "chip_definition.h"
#include "sim.h"

typedef enum {
    SUPPLY_VCC,
    SUPPLY_GND,
    SUPPLY_COUNT
} SupplyKind;

typedef struct {
    SupplyKind kind;
} SupplyConfig;

extern const SimComponentType SUPPLY_VCC_TYPE;
extern const SimComponentType SUPPLY_GND_TYPE;

/* Pin table for a supply. Always one pin. */
const ChipPin *supply_pins(SupplyKind kind);

/* Orderable supply names: "VCC" and "GND". */
size_t supply_part_count(void);
const char *supply_part_name_at(size_t index);
int supply_part_config_at(size_t index, SupplyConfig *config);

/*
 * Returns the simulation component for an orderable supply name and fills in
 * its configuration. Returns NULL when the name is not a supply.
 */
const SimComponentType *supply_lookup(const char *part_number, SupplyConfig *config);

#endif
