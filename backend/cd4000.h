#ifndef CD4000_H
#define CD4000_H

/*
 * CD4000-series CMOS quad 2-input gates. Every definition shares one engine
 * (cd4000.c) and one 14-pin layout, read from the TI datasheets in
 * datasheets/cd4000/:
 *
 * | Pin | Signal | Pin | Signal |
 * |-----|--------|-----|--------|
 * | 1   | A      | 8   | E      |
 * | 2   | B      | 9   | F      |
 * | 3   | J      | 10  | L      |
 * | 4   | K      | 11  | M      |
 * | 5   | C      | 12  | G      |
 * | 6   | D      | 13  | H      |
 * | 7   | VSS    | 14  | VDD    |
 *
 * J = f(A, B), K = f(C, D), L = f(E, F), M = f(G, H).
 *
 * Timing is the datasheet typical propagation delay at VDD = 5 V, 25 C,
 * CL = 50 pF; tPHL and tPLH share one value. CD4093B has Schmitt-trigger
 * inputs (about 0.9 V of hysteresis at 5 V); the logic model treats it as a
 * plain NAND.
 *
 * The multiple-input gates that share these datasheets (CD4002B, CD4012B,
 * CD4023B, CD4025B, CD4072B, CD4073B, CD4075B, CD4082B) and CD4068B/CD4078B are
 * in gates.c. JM38510 numbers whose datasheet rows do not name the device are
 * not included.
 */

#include "chip_definition.h"
#include "sim.h"

extern const ChipDefinition CD4001B_DEFINITION; /* NOR */
extern const ChipDefinition CD4011B_DEFINITION; /* NAND */
extern const ChipDefinition CD4030B_DEFINITION; /* XOR */
extern const ChipDefinition CD4070B_DEFINITION; /* XOR */
extern const ChipDefinition CD4071B_DEFINITION; /* OR */
extern const ChipDefinition CD4077B_DEFINITION; /* XNOR */
extern const ChipDefinition CD4081B_DEFINITION; /* AND */
extern const ChipDefinition CD4093B_DEFINITION; /* NAND, Schmitt-trigger inputs */

#endif
