#ifndef BENCH_H
#define BENCH_H

/*
 * A bench for model tests (plan-10-B section 6.1): one net per pin of a
 * part, the part's instance, and a driver on every input and inout pin.
 * Power pins are driven 1 and ground pins 0; every other driver starts
 * released (Z). Pins are named by PartPin.name (the first pin of that name);
 * an unknown name exits with a message.
 */

#include "parts.h"
#include "sim.h"

typedef struct Bench Bench;

Bench *bench_create(const Part *part);
/*
 * As bench_create, with pairs of pins wired together (one net), e.g.
 * {"QA", "CKB", NULL}: an external wire from an output to an input.
 */
Bench *bench_create_joined(const Part *part, const char *const *joins);
/* Drives the pin strong now (LOGIC_Z releases it). */
void bench_set(Bench *b, const char *pin_name, LogicValue v);
/* Advances the simulation by ps. */
void bench_run(Bench *b, SimTime ps);
/* The value on the pin's net. */
LogicValue bench_get(Bench *b, const char *pin_name);
/* The sim's error, or NULL. */
const char *bench_error(const Bench *b);
void bench_destroy(Bench *b);
/* The bench's simulation and the part's model state, for model hooks such
   as SimComponentType.lcd_state. */
Sim *bench_sim(Bench *b);
void *bench_state(Bench *b);
/* 1 when the part has a pin with this name. */
int bench_has_pin(const Bench *b, const char *pin_name);

/*
 * 65xx peripheral bus cycles (plan-14-B), on a part with pins PHI2, RWB,
 * D0-D7, RS0.. and selects CS0, CS1 (active high) and CS2B or CS1B (active
 * low), whichever it has. Each call is one 1 MHz PHI2 cycle and starts with
 * PHI2 low, at the fall that ended the previous cycle (or after
 * bench_bus65_begin): at +30 ns the data pins are released and RWB, RS and
 * the selects set; PHI2 rises at +500 ns; a write drives the data from
 * +600 ns; a read samples it at +900 ns; PHI2 falls at +1000 ns, where the
 * call returns (the fall already applied).
 */
void bench_bus65_begin(Bench *b);
void bench_bus65_write(Bench *b, unsigned reg, uint8_t value);
/* The byte read; *unknown (may be NULL) gets a bit per data pin that was X or Z. */
uint8_t bench_bus65_read(Bench *b, unsigned reg, uint8_t *unknown);
/* PHI2 cycles with the chip not selected. */
void bench_bus65_idle(Bench *b, unsigned cycles);

#endif
