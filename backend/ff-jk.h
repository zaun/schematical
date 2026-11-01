#ifndef FF_JK_H
#define FF_JK_H

/*
 * J-K flip-flops. Every definition shares one engine (ff-jk.c); each one
 * fixes a pinout (layout) and lists the orderable names and family timings
 * that go with it. Pin numbers below are the datasheet package pins.
 *
 * On the active clock edge: J=L K=L holds, J=H K=L sets Q, J=L K=H clears Q,
 * J=H K=H toggles. The 109 has an inverted K input (KBAR), so there J=H
 * KBAR=L toggles and J=L KBAR=H holds.
 *
 * Clocking: the 73, 76, 107 and 112 are negative-edge triggered; the 109 and
 * CD4027B are positive-edge triggered. The standard-TTL 5476 and 54107 are
 * pulse-triggered master-slave parts (J and K are read while the clock is
 * high, and Q changes on its falling edge); they are modelled as
 * negative-edge triggered, which matches their function table as long as J
 * and K stay steady while the clock is high.
 *
 * Asynchronous PRE/CLR are active low; the CD4027B SET/RESET are active high.
 * When both are asserted every part here drives Q=H and QBAR=H.
 *
 * Timing is one rise/fall pair per family. Values are the datasheet typical
 * or, where only max is given, the max, at the condition noted in ff-jk.c.
 *
 * Part lists come from the orderable tables of the datasheets in
 * datasheets/ff-jk/: every row except the .A/.B duplicates, obsolete parts kept.
 * Military numbers take their family from the part marking, or from the
 * datasheet when it covers one family.
 *
 * Not included: JM38510/M38510 numbers in datasheets that cover two families,
 * where the datasheet does not say which family a number belongs to (5476 and
 * LS76A, 54107 and LS107A, 109 and LS109A, LS112A and S112, ALS109A and
 * AS109A); and SNJ5476W / 5962-9557501QFA, whose CFP pinout the datasheet
 * does not show.
 */

#include "chip_definition.h"
#include "sim.h"

#define FF_JK_PIN(number) ((size_t)((number) - 1))

/* 14-pin dual J-K flip-flop with clear, VCC on pin 4 (73). */
extern const ChipDefinition FF_74_73_DEFINITION;
/* 16-pin dual J-K flip-flop with preset and clear, VCC on pin 5 (76). */
extern const ChipDefinition FF_74_76_DEFINITION;
/* 14-pin dual J-K flip-flop with clear (107). */
extern const ChipDefinition FF_74_107_DEFINITION;
/* 16-pin dual J-KBAR positive-edge flip-flop with preset and clear (109). */
extern const ChipDefinition FF_74_109_DEFINITION;
/* 16-pin dual J-K negative-edge flip-flop with preset and clear (112). */
extern const ChipDefinition FF_74_112_DEFINITION;
/* 16-pin CMOS dual J-K flip-flop with active-high SET and RESET (CD4027B). */
extern const ChipDefinition FF_4027B_DEFINITION;

/* Alternate packages of the parts above. */
/* Dual J-KBAR positive-edge flip-flop with preset and clear, 20-pin LCCC (FK). */
extern const ChipDefinition FF_74_109_FK_DEFINITION;
/* Dual J-K negative-edge flip-flop with preset and clear, 20-pin LCCC (FK). */
extern const ChipDefinition FF_74_112_FK_DEFINITION;

#endif
