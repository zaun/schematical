#ifndef SHIFT_BENCH_H
#define SHIFT_BENCH_H

/*
 * shift_test: the shift kind's shift model against the function tables of
 * the devices that use it (plan-10-G section 5). Fixture devices copy the
 * real devices' pinouts and keys from the spec sections (no shift part file
 * exists before plan-10-I), with 10 ns delays; levels are checked 50 ns
 * after each step. Cases 24-43 (plan-10-H) cover the ls673, tap64,
 * bus-register, fifo and serial-in models. The "[part]" cases (plan-10-I)
 * re-run some on real parts from shift.json.
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
extern PartLibrary *real_library;
extern const char *real_part;
extern SimTime step_ps;
void expect(int ok, const char *what);
extern char *FIXTURE;
extern char *FIXTURE_FOOTPRINTS;
char *dequote(const char *text);
char *load(PartLibrary *lib, const char *text, const char *source);
PartLibrary *fixture_library(void);
extern const LogicValue L0, L1, LZ;
const Part *fx_part(const char *device);
Bench *open_fx(const char *device);
void step(Bench *b);
void set(Bench *b, const char *pin, LogicValue v);
void pulse(Bench *b, const char *pin);
LogicValue bit(unsigned value, unsigned i);
void set_pins(Bench *b, const char *const *names, unsigned n, unsigned value);
void release_pins(Bench *b, const char *const *names, unsigned n);
int pins_value(Bench *b, const char *const *names, unsigned n, unsigned value);
int pins_all(Bench *b, const char *const *names, unsigned n, LogicValue v);
int is(Bench *b, const char *pin, LogicValue v);
void shift_in(Bench *b, const char *serial, const char *clock, unsigned n, unsigned value);
extern const char *const Q8[];
extern const char *const D8[];
unsigned reversed(unsigned value, unsigned n);
void check_165(void);
void check_299(void);
void check_595(void);
void check_tpic6b595(void);
void check_4094(void);
extern const char *const Q4[];
void real(const char *part, SimTime step, void (*check)(void));
extern const char *const IO299[];
extern const char *const DRAIN[];
int drains(Bench *b, unsigned value);

#endif
