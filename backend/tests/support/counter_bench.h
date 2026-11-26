#ifndef COUNTER_BENCH_H
#define COUNTER_BENCH_H

/*
 * counter_test: the counter kind's sync and ripple models against the
 * function tables of the devices that use them (plan-10-D section 6).
 * Fixture devices copy the real devices' pinouts and keys from the spec
 * sections (no counter part file exists before plan-10-F), with 10 ns
 * delays (ripple: 5 ns per stage); levels are checked 50 ns after each step.
 * Cases 30-53 (plan-10-E) cover the johnson, seg7, divn and rate models.
 * The "[part]" cases (plan-10-F) re-run some on real parts from counter.json.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "kind.h"
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
Bench *open_joined(const char *device, const char *const *joins);
void step(Bench *b);
void set(Bench *b, const char *pin, LogicValue v);
void pulse(Bench *b, const char *pin);
void rise(Bench *b, const char *pin);
void fall(Bench *b, const char *pin);
LogicValue bit(unsigned value, unsigned i);
void set_pins(Bench *b, const char *const *names, unsigned n, unsigned value);
void release_pins(Bench *b, const char *const *names, unsigned n);
int pins_value(Bench *b, const char *const *names, unsigned n, unsigned value);
int pins_all(Bench *b, const char *const *names, unsigned n, LogicValue v);
int is(Bench *b, const char *pin, LogicValue v);
extern const char *const QAD[];
extern const char *const ABCD[];
extern const char *const Q8[];
extern const char *const D8[];
extern const char *const Q14[];
void check_161(void);
void check_dual_clock(const char *device, const char *up, const char *down,
                             const char *loadpin, LogicValue load_on, const char *clr,
                             const char *co, const char *bo, const char *const *q,
                             const char *const *data, const char *what);
void check_590(void);
void check_8154(void);
extern const char *const J8[];
int expect_error(const char *text, const char *want);
void real(const char *part, SimTime step, void (*check)(void));
extern const char *const Y8[];
void selects(Bench *b, int which);

#endif
