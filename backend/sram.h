#ifndef SRAM_H
#define SRAM_H

/*
 * Asynchronous SRAMs (Alliance Memory AS6C / AS7C). Every chip shares one
 * engine (sram.c); each definition fixes a package pinout and lists the
 * orderable names and speed grades that go with it. The data comes from the
 * datasheets in datasheets/sram/ (sram_parts.inc).
 *
 * Memory: every instance has its own storage, allocated in pages as they are
 * first written. Each bit is 0, 1 or X. The chip powers up with every bit X,
 * since an SRAM's power-on contents are undefined.
 *
 * Control: the datasheet truth tables. The chip is selected while every CE#
 * is low and CE2 (where present) is high. It writes while selected with WE#
 * low and the byte enable (LB#/UB#, B0#..B3#) low; data and address are taken
 * at the end of the write, the first of WE#, CE#/CE2 or the byte enable going
 * inactive. It reads while selected with OE# low, WE# high and the byte
 * enable low. An undriven (Z) control pin reads as inactive; X reads as
 * unknown. Address and data bits that are X or Z are unknown.
 *
 * Read timing follows the datasheet window, as vendor simulation models do:
 *   - outputs leave high-Z (driving X) at tCLZ / tOLZ / tBLZ / tOW after the
 *     enable that turns them on, and go back to high-Z at tCHZ / tOHZ /
 *     tBHZ / tWHZ after the one that turns them off (X until then);
 *   - data is valid at the latest of address + tAA, CE + tACE, OE + tOE and
 *     byte enable + tBA;
 *   - after an address change the old data holds for tOH, then reads X until
 *     the new data is valid.
 * Minimum times (tOH, tCLZ, tOLZ, tBLZ, tOW) and maximum times (access and
 * high-Z times) are used, so data is only on the bus while it is guaranteed.
 *
 * Write timing is checked at the end of each write against tWP (tWP for
 * OE# low where the datasheet gives one), tCW, tBW, tAW, tAS and tDW, and
 * after it against tDH and tWR. A violation stores X: the whole byte lane for
 * pulse width and address timing, the bit for data setup and hold. An
 * address change during a write, or a write with an unknown address or
 * unknown control level, stores X at every location it could have hit.
 * tRC and tWC are not checked separately: a read cycle shorter than tRC never
 * gets valid data, and tWC is covered by tAW and tWR.
 *
 * Pin numbers: package pins. Ball-grid packages are numbered 1..N over the
 * populated balls in row-major order (A1, A2, ... B1, ...); sram_parts.inc
 * notes the ball name of each.
 */

#include <stddef.h>
#include <stdint.h>

#include "chip_definition.h"
#include "sim.h"

/* Every SRAM definition, as the catalog lists them. */
#define SRAM_CHIP(NAME) extern const ChipDefinition SRAM_##NAME##_DEFINITION;
#include "sram_catalog.inc"
#undef SRAM_CHIP

/* 1 when the instance is an SRAM; fills its word count and word width in bits. */
int sram_instance_size(
    const SimInstance *instance,
    uint32_t *word_count,
    unsigned *word_bits
);

/*
 * Copies count words starting at start. values[i] gets the word's bits and
 * known[i] has a 1 for every bit that is defined (0 = X); an X bit reads as 0
 * in values. Either array may be NULL. Returns the number of words copied,
 * fewer than count when the range runs past the end of the memory, and 0 when
 * the instance is not an SRAM.
 */
size_t sram_inspect(
    const SimInstance *instance,
    uint32_t start,
    size_t count,
    uint32_t *values,
    uint32_t *known
);

/* Address access time (tAA) of the part a config came from, in picoseconds. */
SimTime sram_access_time(const SimLogicConfig *config);

#endif
