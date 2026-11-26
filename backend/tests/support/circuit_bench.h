#ifndef CIRCUIT_BENCH_H
#define CIRCUIT_BENCH_H

/*
 * circuit_check: the circuit API (circuit.h) with parts from the JSON part
 * files. Every scenario the old netlist-level tests covered
 * (netlist_check.c, switch_check.c check_netlist, sram_check.c
 * check_netlist_inspect), plus the new error cases.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "parts.h"
#include "read_file.h"
#include "voltage_fixture.h"
#include "serial_fixture.h"
#include "serial_fixture_classic.h"

extern int failures;
extern PartLibrary *library;
void expect(int ok, const char *what);
size_t part(Circuit *c, const char *name);
size_t net(Circuit *c);
int light(Circuit *c, size_t index);
void check_lit_led(void);
void check_header_passes_nets(void);
void check_dark_led(void);
void check_short(void);
void check_pulls(void);
const char *pin_named(const char *part_name, const char *name);

/* SFX-16550 driven by two DIP switches with pull-ups: every switch position
   pulls its line to GND when on. Positions: TE-GDS0804 1-8 = D0-D7,
   TE-GDS0604 1-3 = A0-A2, 4 = WRBAR, 5 = RDBAR. */
typedef struct {
    Circuit *c;
    size_t uart, s8, s6;
    size_t data[8];
} SerialRig;
void expect_error(Circuit *c, int result, const char *message, const char *what);

typedef struct {
    Circuit *c;
    size_t v, g;
    size_t nets[64];
    char names[64][8];
    size_t net_count;
} Board;

#endif
