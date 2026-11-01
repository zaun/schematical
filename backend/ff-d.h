#ifndef FF_D_H
#define FF_D_H

/*
 * D-type flip-flops. Every definition shares one engine (ff-d.c); each one
 * fixes a pinout (layout) and lists the orderable names and family timings
 * that go with it. Pin numbers below are the datasheet package pins.
 *
 * Clocking: positive-edge triggered unless noted. Q follows D on the active
 * clock edge. Q and QBAR are the complementary outputs where the part has
 * them.
 *
 * Asynchronous PRE/CLR are active low unless noted. When both are asserted,
 * the 74-series parts drive Q=H and QBAR=H; the AUP/AUC parts drive Q=L
 * (CLR wins). Outputs with an OE pin are active-low enabled and go to Z when
 * disabled.
 *
 * Timing is one rise/fall pair per family. Values are the datasheet typical
 * or, where only max is given, the max, at the condition noted in ff-d.c.
 *
 * Not included: CD74 and CD54 names, RGY/BGA/QFN parts with a different pin
 * numbering from the table below, the 16-bit LVC16374A, and Q1/EP variants.
 */

#include "chip_definition.h"
#include "sim.h"

#define FF_PIN(number) ((size_t)((number) - 1))

/* 14-pin dual D flip-flops, PRE and CLR per flip-flop. */
extern const ChipDefinition FF_74_74_DEFINITION;
/* 14-pin dual D flip-flop, negative-edge triggered (SN74HCS72). */
extern const ChipDefinition FF_74_72_DEFINITION;
/* 16-pin hex D flip-flop, common CLR, Q outputs only. */
extern const ChipDefinition FF_74_174_DEFINITION;
/* 16-pin quad D flip-flop, common CLR. */
extern const ChipDefinition FF_74_175_DEFINITION;
/* 20-pin octal D flip-flop, common CLR, Q outputs only. */
extern const ChipDefinition FF_74_273_DEFINITION;
/* 20-pin octal D flip-flop with 3-state outputs (OE). */
extern const ChipDefinition FF_74_374_DEFINITION;
/* 20-pin octal D flip-flop with 3-state outputs, 574 pinout. */
extern const ChipDefinition FF_74_574_DEFINITION;
/* 14-pin CMOS dual D flip-flop with active-high SET and RESET (CD4013B). */
extern const ChipDefinition FF_4013B_DEFINITION;
/* 16-pin CMOS quad D flip-flop with common CLEAR (CD40175B). */
extern const ChipDefinition FF_40175B_DEFINITION;
/* 8-pin single D flip-flop with PRE and CLR. */
extern const ChipDefinition FF_1G74_DEFINITION;
/* 5-pin single D flip-flop, no PRE or CLR. */
extern const ChipDefinition FF_1G79_DEFINITION;
/* 6-pin single D flip-flop, CLR, Q output. */
extern const ChipDefinition FF_1G175_DEFINITION;
/* 6-pin single D flip-flop with OE. */
extern const ChipDefinition FF_1G374_DEFINITION;
/* 8-pin dual D flip-flop, no PRE or CLR. */
extern const ChipDefinition FF_2G79_DEFINITION;

#endif
