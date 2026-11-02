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
/* Drives the pin strong now (LOGIC_Z releases it). */
void bench_set(Bench *b, const char *pin_name, LogicValue v);
/* Advances the simulation by ps. */
void bench_run(Bench *b, SimTime ps);
/* The value on the pin's net. */
LogicValue bench_get(Bench *b, const char *pin_name);
/* The sim's error, or NULL. */
const char *bench_error(const Bench *b);
void bench_destroy(Bench *b);

#endif
