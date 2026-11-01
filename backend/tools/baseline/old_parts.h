#ifndef OLD_PARTS_H
#define OLD_PARTS_H

/*
 * Every orderable part of the old backend (catalog.h, led.h, osc.h,
 * supply.h), sorted by name. Shared by old_listing and old_trace
 * (plan-01-A section 4.3). Deleted with the old backend in plan-01-G.
 */

#include <stddef.h>

#include "chip_definition.h"
#include "led.h"
#include "osc.h"
#include "supply.h"

typedef enum { OLD_CHIP, OLD_LED, OLD_OSC, OLD_SUPPLY } OldSource;

typedef struct {
    const char *name;
    const char *kind;                 /* gate, dff, jkff, sram, switch, led, 7seg, oscillator, supply */
    OldSource source;
    const ChipDefinition *chip;       /* OLD_CHIP only */
    size_t part_index;                /* index in the chip's part list, or the LED/osc/supply index */
    const SimComponentType *type;
    SimLogicConfig logic;             /* OLD_CHIP */
    LedConfig led;                    /* OLD_LED */
    OscConfig osc;                    /* OLD_OSC */
    SupplyConfig supply;              /* OLD_SUPPLY */
    const ChipPin *pins;
    size_t pin_count;
} OldPart;

/* Fills *parts with a malloc'd sorted array; returns the count. Exits on failure. */
size_t old_parts_collect(OldPart **parts);

/* The engine config pointer for a part. */
const void *old_part_config(const OldPart *part);

#endif
