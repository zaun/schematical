#ifndef GATES_H
#define GATES_H

/*
 * Non-quad logic gates, generated from the TI datasheets in datasheets/gate-<function>/ (gate-and, gate-nand, gate-or, gate-nor, gate-xor):
 * single and dual little-logic parts (1G, 2G, LV1T), triple 3-input (10, 11, 27,
 * 4075), dual 4-input (20, 21, 4002), 8- and 13-input (30, 133), and the CD4000
 * multi-input parts (4002B, 4012B, 4023B, 4025B, 4068B, 4072B, 4073B, 4075B,
 * 4078B, 4082B). Package variants with a different pin numbering (20-pin LCCC,
 * X2SON DPW, 6-pin SON, UQFN RSE) are separate definitions.
 */

#include "chip_definition.h"
#include "sim.h"

extern const ChipDefinition SN74_10_DEFINITION; /* Triple 3-input NAND gate */
extern const ChipDefinition SN74_10_FK_DEFINITION; /* Triple 3-input NAND gate, 20-pin LCCC (FK) */
extern const ChipDefinition SN74_11_DEFINITION; /* Triple 3-input AND gate */
extern const ChipDefinition SN74_11_FK_DEFINITION; /* Triple 3-input AND gate, 20-pin LCCC (FK) */
extern const ChipDefinition SN74_27_DEFINITION; /* Triple 3-input NOR gate */
extern const ChipDefinition SN74_27_FK_DEFINITION; /* Triple 3-input NOR gate, 20-pin LCCC (FK) */
extern const ChipDefinition SN74_20_DEFINITION; /* Dual 4-input NAND gate */
extern const ChipDefinition SN74_20_FK_DEFINITION; /* Dual 4-input NAND gate, 20-pin LCCC (FK) */
extern const ChipDefinition SN74_21_DEFINITION; /* Dual 4-input AND gate */
extern const ChipDefinition SN74_21_FK_DEFINITION; /* Dual 4-input AND gate, 20-pin LCCC (FK) */
extern const ChipDefinition SN74_30_DEFINITION; /* Single 8-input NAND gate */
extern const ChipDefinition SN74_30_FK_DEFINITION; /* Single 8-input NAND gate, 20-pin LCCC (FK) */
extern const ChipDefinition SN74_133_DEFINITION; /* Single 13-input NAND gate */
extern const ChipDefinition SN74_133_FK_DEFINITION; /* Single 13-input NAND gate, 20-pin LCCC (FK) */
extern const ChipDefinition SN74_4002_DEFINITION; /* Dual 4-input NOR gate */
extern const ChipDefinition SN74_4002_FK_DEFINITION; /* Dual 4-input NOR gate, 20-pin LCCC (FK) */
extern const ChipDefinition SN74_4075_DEFINITION; /* Triple 3-input OR gate */
extern const ChipDefinition SN74_4075_FK_DEFINITION; /* Triple 3-input OR gate, 20-pin LCCC (FK) */
extern const ChipDefinition LL_1G00_DEFINITION; /* Single 2-input NAND gate */
extern const ChipDefinition LL_1G00_DPW_DEFINITION; /* Single 2-input NAND gate, X2SON (DPW) */
extern const ChipDefinition LL_1G00_SON_DEFINITION; /* Single 2-input NAND gate, 6-pin SON (DRY, DSF) */
extern const ChipDefinition LL_1G02_DEFINITION; /* Single 2-input NOR gate */
extern const ChipDefinition LL_1G02_DPW_DEFINITION; /* Single 2-input NOR gate, X2SON (DPW) */
extern const ChipDefinition LL_1G02_SON_DEFINITION; /* Single 2-input NOR gate, 6-pin SON (DRY, DSF) */
extern const ChipDefinition LL_1G08_DEFINITION; /* Single 2-input AND gate */
extern const ChipDefinition LL_1G08_DPW_DEFINITION; /* Single 2-input AND gate, X2SON (DPW) */
extern const ChipDefinition LL_1G08_SON_DEFINITION; /* Single 2-input AND gate, 6-pin SON (DRY, DSF) */
extern const ChipDefinition LL_1G32_DEFINITION; /* Single 2-input OR gate */
extern const ChipDefinition LL_1G32_DPW_DEFINITION; /* Single 2-input OR gate, X2SON (DPW) */
extern const ChipDefinition LL_1G32_SON_DEFINITION; /* Single 2-input OR gate, 6-pin SON (DRY, DSF) */
extern const ChipDefinition LL_1G86_DEFINITION; /* Single 2-input XOR gate */
extern const ChipDefinition LL_2G00_DEFINITION; /* Dual 2-input NAND gate */
extern const ChipDefinition LL_2G00_RSE_DEFINITION; /* Dual 2-input NAND gate, UQFN (RSE) */
extern const ChipDefinition LL_2G02_DEFINITION; /* Dual 2-input NOR gate */
extern const ChipDefinition LL_2G02_RSE_DEFINITION; /* Dual 2-input NOR gate, UQFN (RSE) */
extern const ChipDefinition LL_2G08_DEFINITION; /* Dual 2-input AND gate */
extern const ChipDefinition LL_2G08_RSE_DEFINITION; /* Dual 2-input AND gate, UQFN (RSE) */
extern const ChipDefinition LL_2G32_DEFINITION; /* Dual 2-input OR gate */
extern const ChipDefinition LL_2G32_RSE_DEFINITION; /* Dual 2-input OR gate, UQFN (RSE) */
extern const ChipDefinition LL_2G86_DEFINITION; /* Dual 2-input XOR gate */
extern const ChipDefinition LL_1G10_DEFINITION; /* Single 3-input NAND gate, 6-pin SON (DRY, DSF) */
extern const ChipDefinition LL_1G11_DEFINITION; /* Single 3-input AND gate, 6-pin SON (DRY, DSF) */
extern const ChipDefinition LL_1G27_DEFINITION; /* Single 3-input NOR gate, 6-pin SON (DRY, DSF) */
extern const ChipDefinition LL_1G332_DEFINITION; /* Single 3-input OR gate, 6-pin SON (DRY, DSF) */
extern const ChipDefinition LL_1G386_DEFINITION; /* Single 3-input XOR (odd parity) gate, 6-pin SON (DRY, DSF) */
extern const ChipDefinition LL_1T00_DEFINITION; /* Single 2-input NAND gate with level shifting */
extern const ChipDefinition LL_1T02_DEFINITION; /* Single 2-input NOR gate with level shifting */
extern const ChipDefinition LL_1T08_DEFINITION; /* Single 2-input AND gate with level shifting */
extern const ChipDefinition LL_1T32_DEFINITION; /* Single 2-input OR gate with level shifting */
extern const ChipDefinition LL_1T86_DEFINITION; /* Single 2-input XOR gate with level shifting */
extern const ChipDefinition CD4002B_DEFINITION; /* Dual 4-input NOR gate */
extern const ChipDefinition CD4012B_DEFINITION; /* Dual 4-input NAND gate */
extern const ChipDefinition CD4023B_DEFINITION; /* Triple 3-input NAND gate */
extern const ChipDefinition CD4025B_DEFINITION; /* Triple 3-input NOR gate */
extern const ChipDefinition CD4068B_DEFINITION; /* 8-input NAND/AND gate */
extern const ChipDefinition CD4072B_DEFINITION; /* Dual 4-input OR gate */
extern const ChipDefinition CD4073B_DEFINITION; /* Triple 3-input AND gate */
extern const ChipDefinition CD4075B_DEFINITION; /* Triple 3-input OR gate */
extern const ChipDefinition CD4078B_DEFINITION; /* 8-input NOR/OR gate */
extern const ChipDefinition CD4082B_DEFINITION; /* Dual 4-input AND gate */

#endif
