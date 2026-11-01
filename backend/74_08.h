#ifndef SN74_08_H
#define SN74_08_H

/**
 * | Name | MODE_1 | MODE_2 | I/O    | Description         |
 * |------|--------|--------|--------|---------------------|
 * | 1A   | 1      | 2      | Input  | Channel 1, Input A  |
 * | 1B   | 2      | 3      | Input  | Channel 1, Input B  |
 * | 1Y   | 3      | 4      | Output | Channel 1, Output Y |
 * | 2A   | 4      | 6      | Input  | Channel 2, Input A  |
 * | 2B   | 5      | 8      | Input  | Channel 2, Input B  |
 * | 2Y   | 6      | 9      | Output | Channel 2, Output Y |
 * | GND  | 7      | 10     | —      | Ground              |
 * | 3Y   | 8      | 12     | Output | Channel 3, Output Y |
 * | 3A   | 9      | 13     | Input  | Channel 3, Input A  |
 * | 3B   | 10     | 14     | Input  | Channel 3, Input B  |
 * | 4Y   | 11     | 16     | Output | Channel 4, Output Y |
 * | 4A   | 12     | 18     | Input  | Channel 4, Input A  |
 * | 4B   | 13     | 19     | Input  | Channel 4, Input B  |
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
 * | H | H | H |
 * | L | X | L |
 * | X | L | L |
 */

#include "chip_definition.h"
#include "sim.h"

#define SN74_08_PIN(number) ((size_t)((number) - 1))

typedef SimLogicConfig Sn74_08Config;

extern const SimComponentType SN74_08_TYPE;

/*
 * Resolves an orderable SN74x08 part number to its simulation component and
 * configuration. Returns NULL when the part number is not supported.
 */
const SimComponentType *sn74_08_lookup(
    const char *part_number,
    Sn74_08Config *config
);

extern const ChipDefinition SN74_08_DEFINITION;

/* The same gate in the 20-pin LCCC (FK) package. */
extern const ChipDefinition SN74_08_FK_DEFINITION;

#endif
