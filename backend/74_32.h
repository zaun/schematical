#ifndef SN74_32_H
#define SN74_32_H

/**
 * | Name | MODE_1 | MODE_2 | I/O    | Description         |
 * |------|--------|--------|--------|---------------------|
 * | 1A   | 1      | 2      | Input  | Gate 1, Input A     |
 * | 1B   | 2      | 3      | Input  | Gate 1, Input B     |
 * | 1Y   | 3      | 4      | Output | Gate 1, Output Y    |
 * | 2A   | 4      | 6      | Input  | Gate 2, Input A     |
 * | 2B   | 5      | 8      | Input  | Gate 2, Input B     |
 * | 2Y   | 6      | 9      | Output | Gate 2, Output Y    |
 * | GND  | 7      | 10     | —      | Ground              |
 * | 3Y   | 8      | 12     | Output | Gate 3, Output Y    |
 * | 3A   | 9      | 13     | Input  | Gate 3, Input A     |
 * | 3B   | 10     | 14     | Input  | Gate 3, Input B     |
 * | 4Y   | 11     | 16     | Output | Gate 4, Output Y    |
 * | 4A   | 12     | 18     | Input  | Gate 4, Input A     |
 * | 4B   | 13     | 19     | Input  | Gate 4, Input B     |
 * | VCC  | 14     | 20     | —      | Positive supply     |
 * | NC   | —      | 1, 5,  | —      | Not connected       |
 * |      |        | 7, 11, |        |                     |
 * |      |        | 15, 17 |        |                     |
 *
 * MODE_1 = SOIC, SOP, SSOP, DIP, TSSOP, TVSOP, CDIP, CFP, WQFN, VQFN
 * MODE_2 = LCCC
 */

/**
 * | A | B | Y |
 * |---|---|---|
 * | H | X | H |
 * | X | H | H |
 * | L | L | L |
 */

#include "chip_definition.h"
#include "sim.h"

#define SN74_32_PIN(number) ((size_t)((number) - 1))

typedef SimLogicConfig Sn74_32Config;

extern const SimComponentType SN74_32_TYPE;

/*
 * Resolves an orderable SN74x32 part number to its simulation component and
 * configuration. Returns NULL when the part number is not supported.
 */
const SimComponentType *sn74_32_lookup(
    const char *part_number,
    Sn74_32Config *config
);

extern const ChipDefinition SN74_32_DEFINITION;

/* The same gate in the 20-pin LCCC (FK) package. */
extern const ChipDefinition SN74_32_FK_DEFINITION;

#endif
