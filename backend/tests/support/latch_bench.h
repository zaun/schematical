#ifndef LATCH_BENCH_H
#define LATCH_BENCH_H

/*
 * latch_test: the latch kind's five models against the function tables of
 * the devices that use them (plan-10-B section 6). Fixture devices copy the
 * real devices' pinouts and keys from the spec sections (no latch part file
 * exists before plan-10-C), with 10 ns delays; levels are checked 50 ns
 * after each step. Cases 29-34 repeat a few on real parts from latch.json
 * (plan-10-C section 4.3), 1 us per step.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "kind.h"
#include "model_pins.h"
#include "parts.h"
#include "read_file.h"
#include "sim.h"

#define NS(value) ((SimTime)((value) * 1000ull))

extern int failures;
extern PartLibrary *library;
void expect(int ok, const char *what);
extern char *FIXTURE;
extern char *FIXTURE_FOOTPRINTS;
char *dequote(const char *text);
char *load(PartLibrary *lib, const char *text, const char *source);
PartLibrary *fixture_library(void);
extern const LogicValue L0, L1, LX, LZ;
Bench *open_fx(const char *device);
void step(Bench *b);
void set(Bench *b, const char *pin, LogicValue v);
LogicValue bit(unsigned value, unsigned i);
void set_bus(Bench *b, const char *fmt, unsigned base, unsigned n, unsigned value);
int bus_is(Bench *b, const char *fmt, unsigned base, unsigned n, const LogicValue *want);
int bus_value(Bench *b, const char *fmt, unsigned base, unsigned n, unsigned value);
int bus_all(Bench *b, const char *fmt, unsigned base, unsigned n, LogicValue v);
int is(Bench *b, const char *pin, LogicValue v);
extern PartLibrary *real_library;

#endif
