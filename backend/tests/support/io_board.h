#ifndef IO_BOARD_H
#define IO_BOARD_H

/*
 * The W65C02S I/O board (plan-14-E section 3.4): a W65C02S6TPG-14, an
 * AT28C256-15PU, an AS6C62256-55PCN, a W65C22S6TPG-14 VIA, a W65C21N6TPG-14
 * PIA, a W65C51N6TPG-14 ACIA on an ECS-2100A-018 (1.8432 MHz), three
 * SN74HC00N decoders, a PULLUP-5V and the 5V and GND supplies, all library
 * parts on the engine. A bench drives PHI2 (1 MHz, 50 %) and RESB (low for
 * the first 4 cycles).
 *
 *   0000-3FFF SRAM    CE# = A15 | A14
 *   4000-4FFF ACIA    CS0 = !(A13 | A12), CS1B = I/O select
 *   5000-5FFF PIA     CS0 = A12, CS1 = !A13, CS2B = I/O select
 *   6000-7FFF VIA     CS1 = A13, CS2B = I/O select
 *   8000-FFFF EEPROM  CE# = !A15
 *
 * I/O select = NAND(!A15, A14), low at 4000-7FFF. Memory OE# = !RWB, SRAM
 * WE# = NAND(PHI2, !RWB). The VIA's IRQB (totem pole) is the CPU's IRQB;
 * the PIA's and ACIA's open-drain IRQs share a pulled-up net. The ACIA's
 * TxD is wired to its RxD; CTSB, DCDB, DSRB are grounded.
 */

#include <stddef.h>
#include <stdint.h>

#include "cpu_state.h"
#include "parts.h"
#include "sim.h"

typedef struct IoBoard IoBoard;

/* The EEPROM holds image (count bytes from offset 0 = CPU 8000; the
   vectors are part of it). NULL on error (message on stderr). */
IoBoard *io_board_create(const PartLibrary *library, const uint8_t *image, size_t count);
void io_board_destroy(IoBoard *board);

/* Runs to the time (ps). 0 when the engine stopped (message on stderr). */
int io_board_run(IoBoard *board, SimTime time);

/* An SRAM byte (chip address 0-7FFF): 1 and *value when known. */
int io_board_sram(const IoBoard *board, uint32_t address, uint8_t *value);

int io_board_state(const IoBoard *board, CpuState *state);

/* A port's pins as a byte: chip "VIA" or "PIA", port 'A' or 'B'. 1 and
   *value when all 8 are 0 or 1. */
int io_board_port(const IoBoard *board, const char *chip, char port, uint8_t *value);

/* The ACIA's sent frames (the serial_sent hook): the count, and up to cap
   frames into frames. */
size_t io_board_sent(const IoBoard *board, uint16_t *frames, size_t cap);

#endif
