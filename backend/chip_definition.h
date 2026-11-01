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

/*
 * Pins in the same group are drawn together on the schematic block, e.g. an
 * SRAM's address pins. NULL means the "default" group, which every pin table
 * written before groups existed uses by leaving the field out.
 */
#define CHIP_PIN_GROUP_DEFAULT "default"

typedef struct {
    unsigned number;   /* package pin number, 1-based */
    const char *name;  /* datasheet pin name, e.g. "1A" or "VCC" */
    PinRole role;
    const char *group; /* pin group, NULL = CHIP_PIN_GROUP_DEFAULT */
} ChipPin;

/*
 * Part flags, so the app can filter the catalog. A part can carry several.
 *   OBSOLETE   the datasheet ordering table lists the part as Obsolete
 *   MILITARY   SN54/SNJ54/SNV54/CD54 names, 5962- and (J)M38510/ numbers,
 *              SMD drawing numbers (start with a digit), CD4000 ...F3A parts
 *   AUTOMOTIVE AEC-Q100 qualified (-Q1) parts: names ending in Q1
 *   ENHANCED   Enhanced Product (-EP) parts: names ending in EP, and V62/ numbers
 */
enum {
    PART_FLAG_OBSOLETE = 1u << 0,
    PART_FLAG_MILITARY = 1u << 1,
    PART_FLAG_AUTOMOTIVE = 1u << 2,
    PART_FLAG_ENHANCED = 1u << 3
};

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
    const unsigned *part_flags;      /* PART_FLAG_* per part number, same order; NULL = none */
    /*
     * Optional level between category and function in the parts drawer, e.g.
     * "SRAM" for Memory -> SRAM -> 32 KB. NULL for chips listed directly
     * under their category.
     */
    const char *subcategory;
} ChipDefinition;

#endif
