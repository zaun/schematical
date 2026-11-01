#ifndef SN74_02_H
#define SN74_02_H

/**
 * | Name | MODE_1 | MODE_2 | I/O    | Description         |
 * |------|--------|--------|--------|---------------------|
 * | 1Y   | 1      | 2      | Output | Gate 1, Output Y    |
 * | 1A   | 2      | 3      | Input  | Gate 1, Input A     |
 * | 1B   | 3      | 4      | Input  | Gate 1, Input B     |
 * | 2Y   | 4      | 6      | Output | Gate 2, Output Y    |
 * | 2A   | 5      | 8      | Input  | Gate 2, Input A     |
 * | 2B   | 6      | 9      | Input  | Gate 2, Input B     |
 * | GND  | 7      | 10     | —      | Ground              |
 * | 3A   | 8      | 12     | Input  | Gate 3, Input A     |
 * | 3B   | 9      | 13     | Input  | Gate 3, Input B     |
 * | 3Y   | 10     | 14     | Output | Gate 3, Output Y    |
 * | 4A   | 11     | 16     | Input  | Gate 4, Input A     |
 * | 4B   | 12     | 18     | Input  | Gate 4, Input B     |
 * | 4Y   | 13     | 19     | Output | Gate 4, Output Y    |
 * | VCC  | 14     | 20     | —      | Positive supply     |
 * | NC   | —      | 1, 5,  | —      | Not connected       |
 * |      |        | 7, 11, |        |                     |
 * |      |        | 15, 17 |        |                     |
 *
 * MODE_1 = SOIC, SOP, SSOP, PDIP, TSSOP, TVSOP, CDIP, CFP, WQFN, VQFN
 * MODE_2 = LCCC
 */

/**
 * | A | B | Y |
 * |---|---|---|
 * | H | X | L |
 * | X | H | L |
 * | L | L | H |
 */

#include "chip_definition.h"
#include "sim.h"

#define SN74_02_PIN(number) ((size_t)((number) - 1))

typedef SimLogicConfig Sn74_02Config;

extern const SimComponentType SN74_02_TYPE;

/*
 * Resolves an orderable SN74x02 part number to its simulation component and
 * configuration. Returns NULL when the part number is not supported.
 */
const SimComponentType *sn74_02_lookup(
    const char *part_number,
    Sn74_02Config *config
);

extern const ChipDefinition SN74_02_DEFINITION;

#endif
