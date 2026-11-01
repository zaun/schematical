#ifndef SN74_266_H
#define SN74_266_H

/**
 * | Name | MODE_1 | I/O    | Description                 |
 * |------|--------|--------|-----------------------------|
 * | 1A   | 1      | Input  | Gate 1, Input A             |
 * | 1B   | 2      | Input  | Gate 1, Input B             |
 * | 1Y   | 3      | Output | Gate 1, Open-drain Output Y |
 * | 2Y   | 4      | Output | Gate 2, Open-drain Output Y |
 * | 2A   | 5      | Input  | Gate 2, Input A             |
 * | 2B   | 6      | Input  | Gate 2, Input B             |
 * | GND  | 7      | —      | Ground                      |
 * | 3A   | 8      | Input  | Gate 3, Input A             |
 * | 3B   | 9      | Input  | Gate 3, Input B             |
 * | 3Y   | 10     | Output | Gate 3, Open-drain Output Y |
 * | 4Y   | 11     | Output | Gate 4, Open-drain Output Y |
 * | 4A   | 12     | Input  | Gate 4, Input A             |
 * | 4B   | 13     | Input  | Gate 4, Input B             |
 * | VCC  | 14     | —      | Positive supply             |
 *
 * MODE_1 = SOIC, SOP, DIP, CDIP, CFP
 */

/**
 * Y = A XNOR B. Outputs are open-drain: a logic 1 releases the output (Z), so
 * a pull-up resistor is required to read a high level.
 *
 * | A | B | Y |
 * |---|---|---|
 * | L | L | H (released) |
 * | L | H | L |
 * | H | L | L |
 * | H | H | H (released) |
 */

#include "chip_definition.h"
#include "sim.h"

#define SN74_266_PIN(number) ((size_t)((number) - 1))

typedef SimLogicConfig Sn74_266Config;

extern const SimComponentType SN74_266_TYPE;

/*
 * Resolves an orderable SN74x266 part number to its simulation component and
 * configuration. Returns NULL when the part number is not supported.
 */
const SimComponentType *sn74_266_lookup(
    const char *part_number,
    Sn74_266Config *config
);

extern const ChipDefinition SN74_266_DEFINITION;

#endif
