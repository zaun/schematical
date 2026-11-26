#ifndef Q_COV_BENCH_H
#define Q_COV_BENCH_H

/*
 * q_cov_bench (plan-68-Q): loads one device of a part file with a few edits
 * applied, for the logic-model coverage programs (*_cov_test.c).
 *
 * A device is named by its part file (relative to PARTS_DIR, or a path
 * starting with "fx:" relative to tests/fixtures, e.g. "fx:latch/parts.json")
 * and its "id". Edits are {path, value, path, value, ..., NULL}: a path is
 * dot-separated object keys and array indexes ("units.0.q",
 * "timing.HC.note"); the value is JSON with single quotes for double quotes,
 * or NULL to delete that key or element. An index one past an array's end
 * appends. The file's footprints (the footprints.json next to it) are loaded
 * first. A missing device or path exits the program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "cJSON.h"
#include "parts.h"
#include "sim.h"

extern int failures;

/* Prints PASS/FAIL what and counts failures. */
void qc_expect(int ok, const char *what);

/* Loading the edited device fails with `expect` in the message, and adds no part. */
void qc_fail(const char *file, const char *id, const char *const *edits, const char *expect);

/* Loads the edited device into a fresh library (*library; destroy it) and
   returns its part named `part` (NULL: its first part). A load error is a
   FAIL and returns NULL. */
const Part *qc_load(PartLibrary **library, const char *file, const char *id,
                    const char *const *edits, const char *part);

/* A bench on qc_load's part (its library in *library), or NULL. */
Bench *qc_bench(PartLibrary **library, const char *file, const char *id,
                const char *const *edits, const char *part);

/* As qc_bench, with pins wired together (bench_create_joined). */
Bench *qc_bench_joined(PartLibrary **library, const char *file, const char *id,
                       const char *const *edits, const char *const *joins);

/* Bench helpers: drive, advance 100 ns, read. */
void qc_set(Bench *b, const char *pin, LogicValue v);
void qc_step(Bench *b);
int qc_is(Bench *b, const char *pin, LogicValue v);
/* Drives names[0..n) from value's bits (names[0] = bit 0). */
void qc_set_bus(Bench *b, const char *const *names, unsigned n, unsigned value);
/* 1 when names[0..n) read value's bits. */
int qc_bus(Bench *b, const char *const *names, unsigned n, unsigned value);
/* 1 when every one of names[0..n) reads v. */
int qc_bus_all(Bench *b, const char *const *names, unsigned n, LogicValue v);
/* A pulse 0 -> 1 -> 0 (each level held one step). */
void qc_pulse(Bench *b, const char *pin);
/* The index of a pin named `name` in part, or exits. */
size_t qc_pin(const Part *part, const char *name);

#define L0 LOGIC_0
#define L1 LOGIC_1
#define LX LOGIC_X
#define LZ LOGIC_Z

#endif
