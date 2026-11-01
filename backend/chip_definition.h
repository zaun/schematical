#ifndef CHIP_DEFINITION_H
#define CHIP_DEFINITION_H

#include <stddef.h>

#include "sim.h"

/*
 * Describes one chip family for the catalog. Each 74xx definition file provides
 * its own instance, so everything the schematic app knows about a chip (its
 * readable function, orderable names and pin roles) comes from the backend.
 */

typedef enum {
    PIN_ROLE_INPUT,
    PIN_ROLE_OUTPUT,
    PIN_ROLE_INOUT,
    PIN_ROLE_POWER,
    PIN_ROLE_GROUND,
    PIN_ROLE_NO_CONNECT
} PinRole;

typedef struct {
    unsigned number;   /* package pin number, 1-based */
    const char *name;  /* datasheet pin name, e.g. "1A" or "VCC" */
    PinRole role;
} ChipPin;

typedef struct {
    const char *id;                  /* stable identifier, e.g. "74_00" */
    const char *function;            /* readable function, e.g. "NAND" */
    const char *description;         /* e.g. "Quad 2-input NAND gate" */
    const char *category;            /* top-level grouping, e.g. "Logic" */
    const char *const *part_numbers; /* orderable names this definition offers */
    size_t part_number_count;
    const ChipPin *pins;             /* 14-pin (MODE_1) package pin table */
    size_t pin_count;
    const SimComponentType *(*lookup)(
        const char *part_number,
        SimLogicConfig *config
    );
    size_t gate_count;               /* logic gates per package, e.g. 4 for a quad gate */
} ChipDefinition;

#endif
