#ifndef CPU_BOARD_H
#define CPU_BOARD_H

/*
 * Whole-system CPU boards for cpu_test and cpu_bench (plan-09-D section 5):
 * a CPU, an AT28C256-15PU EEPROM, an AS6C62256-55PCN SRAM and an SN74HC00N
 * decoder, all library parts on the engine, plus a bench that drives the
 * clock (1 MHz, 50 %), RESET (low for the first 4 cycles) and a logic-1 net.
 *
 * 6502 family: EEPROM at 8000-FFFF (CE# = NAND(A15, A15), OE# = gate 1 =
 * NAND(R/W, R/W), WE# = 1); SRAM at 0000-7FFF (CE# = A15, OE# = gate 1,
 * WE# = NAND(clock, gate 1)).
 *
 * Z80: EEPROM at 0000-7FFF, SRAM at 8000-FFFF; gate 1 = NAND(MREQ#, MREQ#),
 * gate 2 = NAND(A15, A15), EEPROM CE# = gate 3 = NAND(gate 1, gate 2), SRAM
 * CE# = gate 4 = NAND(gate 1, A15); OE# of both = RD#, SRAM WE# = WR#.
 */

#include <stddef.h>
#include <stdint.h>

#include "cpu_state.h"
#include "parts.h"
#include "sim.h"

typedef struct CpuBoard CpuBoard;

/* The EEPROM holds `image` (count bytes from EEPROM offset 0, plus the 6502
   reset vector at offset 7FFC when vector >= 0). NULL on error (message on
   stderr). */
CpuBoard *cpu_board_create(const PartLibrary *library, const char *cpu, const uint8_t *image,
                           size_t count, int vector);
void cpu_board_destroy(CpuBoard *board);

/* Runs to the time (ps). 0 when the engine stopped (message on stderr). */
int cpu_board_run(CpuBoard *board, SimTime time);

/* An SRAM byte (chip address 0-7FFF): 1 and *value when known, else 0. */
int cpu_board_sram(const CpuBoard *board, uint32_t address, uint8_t *value);

int cpu_board_state(const CpuBoard *board, CpuState *state);

#endif
