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

static int failures = 0;
static PartLibrary *library;

/* Real-part runs (plan-10-F section 4.3): the cases open this part from the
   real library instead of their fixture, and step this long. */
static PartLibrary *real_library;
static const char *real_part;
static SimTime step_ps = NS(50);

static void expect(int ok, const char *what) {
    if (real_part != NULL) {
        printf("%s: [%s] %s\n", ok ? "PASS" : "FAIL", real_part, what);
    } else {
        printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    }
    if (!ok) {
        failures++;
    }
}

/* Generated from the spec sections (planning/plan-10-data). */
/* The fixture parts: tests/fixtures/counter/parts.json (moved out of C by plan-68-H). */
static char *FIXTURE;

/* Dual-row fixture footprints FX<n>, one per pin count above. */
/* Their footprints: tests/fixtures/counter/footprints.json. */
static char *FIXTURE_FOOTPRINTS;

static char *dequote(const char *text) {
    size_t n = strlen(text);
    char *copy = malloc(n + 1);
    size_t i;

    for (i = 0; i <= n; i++) {
        copy[i] = text[i] == '\'' ? '"' : text[i];
    }
    return copy;
}

static char *load(PartLibrary *lib, const char *text, const char *source) {
    char *json = dequote(text);
    char *error = NULL;

    if (load_parts(lib, json, source, &error) == 0) {
        free(error);
        error = NULL;
    } else if (error == NULL) {
        error = strdup("(no message)");
    }
    free(json);
    return error;
}

static PartLibrary *fixture_library(void) {
    PartLibrary *lib = part_library_create();
    char *json = dequote(FIXTURE_FOOTPRINTS);
    char *error = NULL;

    if (lib == NULL || load_footprints(lib, json, "fx-footprints.json", &error) != 0) {
        fprintf(stderr, "fixture footprints: %s\n", error != NULL ? error : "(no message)");
        exit(1);
    }
    free(json);
    free(error);
    return lib;
}

/* ---- Bench helpers -------------------------------------------------------- */

static const LogicValue L0 = LOGIC_0, L1 = LOGIC_1, LZ = LOGIC_Z;

static const Part *fx_part(const char *device) {
    char name[64];
    const Part *part;

    snprintf(name, sizeof name, "FX-%s", device);
    part = real_part != NULL ? part_library_find(real_library, real_part)
                             : part_library_find(library, name);
    if (part == NULL || part->kind != PART_COUNTER) {
        fprintf(stderr, "no counter part %s\n", name);
        exit(1);
    }
    return part;
}

static Bench *open_fx(const char *device) {
    return bench_create(fx_part(device));
}

static Bench *open_joined(const char *device, const char *const *joins) {
    return bench_create_joined(fx_part(device), joins);
}

static void step(Bench *b) {
    bench_run(b, step_ps);
}

static void set(Bench *b, const char *pin, LogicValue v) {
    bench_set(b, pin, v);
}

/* A full clock pulse on pin: up, step, down, step. */
static void pulse(Bench *b, const char *pin) {
    set(b, pin, L1);
    step(b);
    set(b, pin, L0);
    step(b);
}

/* A rising edge only (the pin was low). */
static void rise(Bench *b, const char *pin) {
    set(b, pin, L1);
    step(b);
}

static void fall(Bench *b, const char *pin) {
    set(b, pin, L0);
    step(b);
}

static LogicValue bit(unsigned value, unsigned i) {
    return (value >> i) & 1u ? LOGIC_1 : LOGIC_0;
}

/* Drives the named pins from value's bits (names[0] = bit 0). */
static void set_pins(Bench *b, const char *const *names, unsigned n, unsigned value) {
    unsigned i;

    for (i = 0; i < n; i++) {
        set(b, names[i], bit(value, i));
    }
}

static void release_pins(Bench *b, const char *const *names, unsigned n) {
    unsigned i;

    for (i = 0; i < n; i++) {
        set(b, names[i], LZ);
    }
}

static int pins_value(Bench *b, const char *const *names, unsigned n, unsigned value) {
    unsigned i;
    int ok = 1;

    for (i = 0; i < n; i++) {
        LogicValue got = bench_get(b, names[i]);

        if (got != bit(value, i)) {
            printf("  %s = %c, want %c (value %u)\n", names[i], "01XZ"[got],
                   "01XZ"[bit(value, i)], value);
            ok = 0;
        }
    }
    return ok;
}

static int pins_all(Bench *b, const char *const *names, unsigned n, LogicValue v) {
    unsigned i;
    int ok = 1;

    for (i = 0; i < n; i++) {
        LogicValue got = bench_get(b, names[i]);

        if (got != v) {
            printf("  %s = %c, want %c\n", names[i], "01XZ"[got], "01XZ"[v]);
            ok = 0;
        }
    }
    return ok;
}

static int is(Bench *b, const char *pin, LogicValue v) {
    LogicValue got = bench_get(b, pin);

    if (got != v) {
        printf("  %s = %c, want %c\n", pin, "01XZ"[got], "01XZ"[v]);
        return 0;
    }
    return 1;
}

static const char *const QAD[] = {"QA", "QB", "QC", "QD"};
static const char *const ABCD[] = {"A", "B", "C", "D"};
static const char *const Q8[] = {"QA", "QB", "QC", "QD", "QE", "QF", "QG", "QH"};
static const char *const D8[] = {"A", "B", "C", "D", "E", "F", "G", "H"};
static const char *const Q14[] = {"Q1", "Q2", "Q3", "Q4"};

/* ---- 74_161 / 163 / 169 -------------------------------------------------- */

static void check_161(void) {
    Bench *b = open_fx("74_161");
    int ok;
    unsigned k;

    set(b, "CLR", L1);
    set(b, "LOAD", L1);
    set(b, "ENP", L1);
    set(b, "ENT", L1);
    set_pins(b, ABCD, 4, 0);
    set(b, "CLK", L0);
    step(b);
    ok = pins_value(b, QAD, 4, 0) && is(b, "RCO", L0);
    for (k = 1; k <= 16; k++) {
        pulse(b, "CLK");
        ok &= pins_value(b, QAD, 4, k % 16) && is(b, "RCO", k % 16 == 15 ? L1 : L0);
    }
    expect(ok, "1 74_161 power-on 0; counts 0->15->0 on CLK rising with ENP = ENT = 1; RCO high only "
               "at 15");

    for (k = 0; k < 15; k++) {
        pulse(b, "CLK");
    }
    ok = pins_value(b, QAD, 4, 15) && is(b, "RCO", L1);
    set(b, "CLR", L0);
    step(b);
    expect(ok && pins_value(b, QAD, 4, 0) && is(b, "RCO", L0),
           "2 74_161 CLR low clears at once (no clock); RCO falls with it");

    set(b, "CLR", L1);
    set(b, "ENP", L0);
    set(b, "ENT", L0);
    set_pins(b, ABCD, 4, 0xA);
    set(b, "LOAD", L0);
    step(b);
    ok = pins_value(b, QAD, 4, 0);
    pulse(b, "CLK");
    expect(ok && pins_value(b, QAD, 4, 0xA),
           "3 74_161 LOAD low loads at the edge regardless of ENP/ENT");

    set_pins(b, ABCD, 4, 15);
    pulse(b, "CLK");
    set(b, "LOAD", L1);
    set(b, "ENT", L1);
    step(b);
    ok = pins_value(b, QAD, 4, 15) && is(b, "RCO", L1);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 15) && is(b, "RCO", L1);
    set(b, "ENP", L1);
    set(b, "ENT", L0);
    step(b);
    ok &= is(b, "RCO", L0);
    pulse(b, "CLK");
    expect(ok && pins_value(b, QAD, 4, 15) && is(b, "RCO", L0),
           "4 74_161 ENP low holds, RCO still follows ENT; ENT low holds and RCO low");
    bench_destroy(b);
}

static void check_163(void) {
    Bench *b = open_fx("74_163");
    int ok;

    set(b, "CLR", L1);
    set(b, "LOAD", L1);
    set(b, "ENP", L1);
    set(b, "ENT", L1);
    set(b, "CLK", L0);
    step(b);
    pulse(b, "CLK");
    pulse(b, "CLK");
    pulse(b, "CLK");
    set(b, "CLR", L0);
    set(b, "LOAD", L0);
    set_pins(b, ABCD, 4, 9);
    step(b);
    ok = pins_value(b, QAD, 4, 3);
    pulse(b, "CLK");
    expect(ok && pins_value(b, QAD, 4, 0), "5 74_163 CLR low clears only at the edge; beats LOAD");
    bench_destroy(b);
}

static void check_169(void) {
    Bench *b = open_fx("74_169");
    int ok;

    set(b, "LOAD", L1);
    set(b, "ENP", L0);
    set(b, "ENT", L0);
    set(b, "U/D", L0);
    set(b, "CLK", L0);
    step(b);
    ok = pins_value(b, QAD, 4, 0) && is(b, "RCO", L0);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 15) && is(b, "RCO", L1);
    set(b, "U/D", L1);
    step(b);
    ok &= is(b, "RCO", L0);
    set(b, "ENT", L1);
    step(b);
    expect(ok && is(b, "RCO", L1),
           "6 74_169 down count 0 -> 15; RCO low at 0 down, at 15 up, only with ENT low");
    bench_destroy(b);
}

/* ---- 74_190 / 191 / 193 --------------------------------------------------- */

static void check_190(void) {
    Bench *b = open_fx("74_190");
    int ok = 1;
    unsigned k;

    set(b, "LOAD", L1);
    set(b, "CTEN", L0);
    set(b, "D/U", L0);
    set(b, "CLK", L0);
    step(b);
    for (k = 1; k <= 10; k++) {
        pulse(b, "CLK");
        ok &= pins_value(b, QAD, 4, k % 10);
    }
    set(b, "D/U", L1);
    step(b);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 9);
    set(b, "D/U", L0);
    set_pins(b, ABCD, 4, 12);
    set(b, "LOAD", L0);
    step(b);
    set(b, "LOAD", L1);
    step(b);
    ok &= pins_value(b, QAD, 4, 12);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 9);
    pulse(b, "CLK");
    expect(ok && pins_value(b, QAD, 4, 0),
           "7 74_190 9 -> 0 up, 0 -> 9 down; loaded 12, counts up to 9 then 0 (the illegal table)");
    bench_destroy(b);
}

static void check_191(void) {
    Bench *b = open_fx("74_191");
    int ok;

    set(b, "CTEN", L0);
    set(b, "D/U", L0);
    set(b, "CLK", L0);
    set(b, "LOAD", L0);
    set_pins(b, ABCD, 4, 5);
    step(b);
    ok = pins_value(b, QAD, 4, 5);
    set_pins(b, ABCD, 4, 6);
    step(b);
    ok &= pins_value(b, QAD, 4, 6);
    set_pins(b, ABCD, 4, 15);
    step(b);
    rise(b, "CLK");   /* LOAD still low: no count */
    set(b, "LOAD", L1);
    step(b);
    ok &= pins_value(b, QAD, 4, 15) && is(b, "RCO", L1) && is(b, "MAX/MIN", L1);
    fall(b, "CLK");
    ok &= is(b, "RCO", L0);
    set(b, "CTEN", L1);
    step(b);
    expect(ok && is(b, "RCO", L1),
           "8 74_191 LOAD low is transparent (data changes follow); RCO low only during CLK low at "
           "max with CTEN low");
    bench_destroy(b);
}

static void check_dual_clock(const char *device, const char *up, const char *down,
                             const char *loadpin, LogicValue load_on, const char *clr,
                             const char *co, const char *bo, const char *const *q,
                             const char *const *data, const char *what) {
    Bench *b = open_fx(device);
    LogicValue load_off = load_on == L0 ? L1 : L0;
    int ok;

    /* DOWN high before UP rises (an UP edge with DOWN floating is rightly X), then a clear. */
    set(b, loadpin, load_off);
    set(b, down, L1);
    step(b);
    set(b, up, L1);
    step(b);
    set(b, clr, L1);
    step(b);
    set(b, clr, L0);
    step(b);
    fall(b, up);
    rise(b, up);
    ok = pins_value(b, q, 4, 1);
    fall(b, up);
    rise(b, up);
    ok &= pins_value(b, q, 4, 2);
    set_pins(b, data, 4, 15);
    set(b, loadpin, load_on);
    step(b);
    set(b, loadpin, load_off);
    step(b);
    ok &= pins_value(b, q, 4, 15) && is(b, co, L1);
    fall(b, up);
    ok &= is(b, co, L0) && pins_value(b, q, 4, 15);
    rise(b, up);
    ok &= pins_value(b, q, 4, 0) && is(b, co, L1) && is(b, bo, L1);
    fall(b, down);
    ok &= is(b, bo, L0);
    rise(b, down);
    ok &= pins_value(b, q, 4, 15);
    set_pins(b, data, 4, 7);
    set(b, loadpin, load_on);
    set(b, clr, L1);
    step(b);
    expect(ok && pins_value(b, q, 4, 0), what);
    bench_destroy(b);
}

/* ---- 74_561 / 569 / 867 / 869 ------------------------------------------- */

static void check_561(void) {
    Bench *b = open_fx("74_561");
    int ok;

    set(b, "ACLR", L1);
    set(b, "ALOAD", L1);
    set(b, "SCLR", L1);
    set(b, "SLOAD", L1);
    set(b, "ENP", L1);
    set(b, "ENT", L1);
    set(b, "OE", L0);
    set(b, "CLK", L0);
    step(b);
    pulse(b, "CLK");
    pulse(b, "CLK");
    pulse(b, "CLK");
    set_pins(b, ABCD, 4, 9);
    set(b, "ACLR", L0);
    set(b, "ALOAD", L0);
    step(b);
    ok = pins_value(b, QAD, 4, 0);
    set(b, "ACLR", L1);
    step(b);
    ok &= pins_value(b, QAD, 4, 9);
    set(b, "ALOAD", L1);
    set(b, "SCLR", L0);
    set(b, "SLOAD", L0);
    set_pins(b, ABCD, 4, 5);
    step(b);
    ok &= pins_value(b, QAD, 4, 9);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 0);
    set(b, "SCLR", L1);
    step(b);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 5);
    set(b, "SLOAD", L1);
    step(b);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 6);
    set(b, "OE", L1);
    step(b);
    ok &= pins_all(b, QAD, 4, LZ) && is(b, "RCO", L0);
    set(b, "OE", L0);
    set_pins(b, ABCD, 4, 15);
    set(b, "CLK", L1);
    step(b);
    set(b, "ALOAD", L0);
    step(b);
    set(b, "ALOAD", L1);
    step(b);
    ok &= pins_value(b, QAD, 4, 15) && is(b, "RCO", L1) && is(b, "CCO", L0);
    fall(b, "CLK");
    expect(ok && is(b, "CCO", L1) && is(b, "RCO", L1),
           "10 74_561 ACLR > ALOAD > SCLR > SLOAD > count; OE high -> Q = Z, RCO driven; CCO high only "
           "during CLK low");
    bench_destroy(b);
}

static void check_569(void) {
    Bench *b = open_fx("74_569");
    int ok;
    unsigned k;

    set(b, "ACLR", L1);
    set(b, "SCLR", L1);
    set(b, "LOAD", L1);
    set(b, "ENP", L0);
    set(b, "ENT", L0);
    set(b, "U/D", L1);
    set(b, "OE", L0);
    set(b, "CLK", L0);
    step(b);
    for (k = 0; k < 5; k++) {
        pulse(b, "CLK");
    }
    set(b, "SCLR", L0);
    step(b);
    ok = pins_value(b, QAD, 4, 5);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 0);
    set(b, "SCLR", L1);
    set(b, "LOAD", L0);
    set_pins(b, ABCD, 4, 15);
    step(b);
    rise(b, "CLK");
    set(b, "LOAD", L1);
    step(b);
    ok &= pins_value(b, QAD, 4, 15) && is(b, "CCO", L1) && is(b, "RCO", L0);
    fall(b, "CLK");
    ok &= is(b, "CCO", L0);
    set(b, "ENP", L1);
    step(b);
    expect(ok && is(b, "CCO", L1) && is(b, "RCO", L0),
           "11 74_569 SCLR at the edge; CCO low pulse at terminal with both enables low");
    bench_destroy(b);
}

static void check_867(void) {
    Bench *b = open_fx("74_867");
    Bench *c = open_fx("74_869");
    Bench *as = open_fx("74_867_AS");
    int ok;

    set(b, "ENP", L0);
    set(b, "ENT", L0);
    set(b, "CLK", L0);
    set(b, "S1", L1);
    set(b, "S0", L0);
    set_pins(b, D8, 8, 0x5A);
    step(b);
    pulse(b, "CLK");
    ok = pins_value(b, Q8, 8, 0x5A);
    set(b, "S0", L1);   /* 11: up */
    step(b);
    pulse(b, "CLK");
    ok &= pins_value(b, Q8, 8, 0x5B);
    set(b, "S1", L0);   /* 01: down */
    step(b);
    pulse(b, "CLK");
    ok &= pins_value(b, Q8, 8, 0x5A);
    set(b, "S0", L0);   /* 00: clear at once */
    step(b);
    ok &= pins_value(b, Q8, 8, 0x00);
    set(b, "S1", L1);   /* 10: load FF */
    set_pins(b, D8, 8, 0xFF);
    step(b);
    pulse(b, "CLK");
    ok &= pins_value(b, Q8, 8, 0xFF) && is(b, "RCO", L1);
    set(b, "S0", L1);   /* 11: up, at max */
    step(b);
    ok &= is(b, "RCO", L0);
    set(b, "ENP", L1);
    step(b);
    ok &= is(b, "RCO", L0);

    set(c, "ENP", L0);
    set(c, "ENT", L0);
    set(c, "CLK", L0);
    set(c, "S1", L1);
    set(c, "S0", L0);
    set_pins(c, D8, 8, 0x33);
    step(c);
    pulse(c, "CLK");
    set(c, "S1", L0);   /* 00 on the 869: clear only at the edge */
    step(c);
    ok &= pins_value(c, Q8, 8, 0x33);
    pulse(c, "CLK");
    ok &= pins_value(c, Q8, 8, 0x00);

    set(as, "ENP", L0);
    set(as, "ENT", L0);
    set(as, "CLK", L0);
    set(as, "S1", L1);
    set(as, "S0", L0);
    set_pins(as, D8, 8, 0xFF);
    step(as);
    pulse(as, "CLK");
    set(as, "S0", L1);
    step(as);
    ok &= is(as, "RCO", L0);
    set(as, "ENP", L1);
    step(as);
    expect(ok && is(as, "RCO", L1),
           "12 74_867 modes: 00 clears at once (869: at the edge); 10 loads; 01/11 count; RCO only in "
           "count modes; AS: RCO needs ENP low too");
    bench_destroy(b);
    bench_destroy(c);
    bench_destroy(as);
}

/* ---- 74_590 / 592 / 593 / 697 / 8154 ------------------------------------- */

static void check_590(void) {
    Bench *b = open_fx("74_590");
    int ok;

    set(b, "CCKEN", L0);
    set(b, "CCLR", L1);
    set(b, "G", L0);
    set(b, "RCK", L0);
    set(b, "CCK", L0);
    step(b);
    pulse(b, "CCK");
    pulse(b, "CCK");
    pulse(b, "CCK");
    ok = pins_value(b, Q8, 8, 0);
    pulse(b, "RCK");
    ok &= pins_value(b, Q8, 8, 3);
    set(b, "CCK", L1);
    set(b, "RCK", L1);
    step(b);
    ok &= pins_value(b, Q8, 8, 3);
    set(b, "CCK", L0);
    set(b, "RCK", L0);
    step(b);
    set(b, "CCK", L1);
    set(b, "RCK", L1);
    step(b);
    ok &= pins_value(b, Q8, 8, 4);
    set(b, "CCK", L0);
    set(b, "RCK", L0);
    set(b, "G", L1);
    step(b);
    ok &= pins_all(b, Q8, 8, LZ);
    set(b, "G", L0);
    set(b, "CCLR", L0);
    step(b);
    ok &= pins_value(b, Q8, 8, 4);
    set(b, "CCLR", L1);
    step(b);
    pulse(b, "RCK");
    expect(ok && pins_value(b, Q8, 8, 0),
           "13 74_590 count and register separate; RCK rising copies; tied clocks -> register one "
           "behind; G high -> Z; CCLR clears the counter only");
    bench_destroy(b);
}

static void check_592(void) {
    Bench *b = open_fx("74_592");
    int ok;

    set(b, "CCKEN", L0);
    set(b, "CCLR", L1);
    set(b, "CLOAD", L1);
    set(b, "RCK", L0);
    set(b, "CCK", L0);
    set_pins(b, D8, 8, 0xFE);
    step(b);
    pulse(b, "RCK");
    set(b, "CLOAD", L0);
    step(b);
    ok = is(b, "RCO", L1);
    set_pins(b, D8, 8, 0xFF);
    step(b);
    ok &= is(b, "RCO", L1);   /* the input register has not taken FF yet */
    pulse(b, "RCK");
    ok &= is(b, "RCO", L0);   /* RCK during CLOAD low passes through */
    set(b, "CCLR", L0);
    step(b);
    ok &= is(b, "RCO", L1);   /* CCLR wins over CLOAD */
    set(b, "CCLR", L1);
    step(b);
    ok &= is(b, "RCO", L0);
    set(b, "CLOAD", L1);
    step(b);
    pulse(b, "CCK");
    expect(ok && is(b, "RCO", L1),
           "14 74_592 CLOAD low copies the input register continuously; RCK rising during CLOAD low "
           "passes through; CCLR wins over CLOAD");
    bench_destroy(b);
}

static const char *const BUS593[] = {"A/QA", "B/QB", "C/QC", "D/QD",
                                     "E/QE", "F/QF", "G/QG", "H/QH"};

static void check_593(void) {
    Bench *b = open_fx("74_593");
    int ok;

    set(b, "CCKEN", L0);
    set(b, "CCKENBAR", L1);
    set(b, "CCLR", L1);
    set(b, "CLOAD", L1);
    set(b, "RCKEN", L1);
    set(b, "RCK", L0);
    set(b, "CCK", L0);
    set(b, "G", L1);
    set(b, "GBAR", L0);
    step(b);
    ok = pins_value(b, BUS593, 8, 0);
    set(b, "G", L0);
    step(b);
    ok &= pins_all(b, BUS593, 8, LZ);
    set(b, "G", L1);
    set(b, "GBAR", L1);
    step(b);
    ok &= pins_all(b, BUS593, 8, LZ);
    set_pins(b, BUS593, 8, 0x42);
    set(b, "RCKEN", L0);
    step(b);
    pulse(b, "RCK");
    set(b, "RCKEN", L1);
    release_pins(b, BUS593, 8);
    set(b, "CLOAD", L0);
    step(b);
    set(b, "CLOAD", L1);
    set(b, "GBAR", L0);
    step(b);
    ok &= pins_value(b, BUS593, 8, 0x42);
    set(b, "CCKEN", L1);
    step(b);
    pulse(b, "CCK");
    ok &= pins_value(b, BUS593, 8, 0x43);
    set(b, "CCKEN", L0);
    set(b, "CCKENBAR", L0);
    step(b);
    pulse(b, "CCK");
    ok &= pins_value(b, BUS593, 8, 0x44);
    set(b, "CCKENBAR", L1);
    step(b);
    pulse(b, "CCK");
    expect(ok && pins_value(b, BUS593, 8, 0x44) && bench_error(b) == NULL,
           "15 74_593 bus driven only with G high and GBAR low; register loads from the pins with "
           "RCKEN low; counts with CCKEN high or CCKENBAR low");
    bench_destroy(b);
}

static void check_697(void) {
    Bench *b = open_fx("74_697");
    int ok;

    set(b, "CCLR", L1);
    set(b, "ENP", L1);
    set(b, "ENT", L1);
    set(b, "U/D", L1);
    set(b, "G", L0);
    set(b, "R/C", L0);
    set(b, "RCK", L0);
    set(b, "CCK", L0);
    set(b, "LOAD", L0);
    set_pins(b, ABCD, 4, 9);
    step(b);
    pulse(b, "CCK");
    set(b, "LOAD", L1);
    step(b);
    ok = pins_value(b, QAD, 4, 9);
    set(b, "R/C", L1);
    step(b);
    ok &= pins_value(b, QAD, 4, 0);
    pulse(b, "RCK");
    expect(ok && pins_value(b, QAD, 4, 9),
           "16 74_697 R/C high shows the register, low the counter; LOAD at the edge ignores the "
           "enables");
    bench_destroy(b);
}

static const char *const Y8[] = {"Y0", "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7"};

static void selects(Bench *b, int which) {
    static const char *const G[] = {"GAL", "GAU", "GBL", "GBU"};
    int i;

    for (i = 0; i < 4; i++) {
        set(b, G[i], i == which ? L0 : L1);
    }
    step(b);
}

static void check_8154(void) {
    static const char *const JOIN[] = {"RCOA", "CLKBEN", "CLKA", "CLKB", NULL};
    Bench *b = open_fx("74_8154");
    Bench *c;
    int ok;
    unsigned k;

    set(b, "CCLR", L1);
    set(b, "CLKBEN", L0);
    set(b, "CLKA", L0);
    set(b, "CLKB", L0);
    set(b, "RCLK", L0);
    selects(b, -1);
    ok = pins_all(b, Y8, 8, LZ);
    for (k = 0; k < 3; k++) {
        pulse(b, "CLKA");
    }
    for (k = 0; k < 0x0102; k++) {
        pulse(b, "CLKB");
    }
    pulse(b, "RCLK");
    selects(b, 0);
    ok &= pins_value(b, Y8, 8, 3);
    selects(b, 1);
    ok &= pins_value(b, Y8, 8, 0);
    selects(b, 2);
    ok &= pins_value(b, Y8, 8, 0x02);
    selects(b, 3);
    ok &= pins_value(b, Y8, 8, 0x01);
    bench_destroy(b);

    c = open_joined("74_8154", JOIN);
    set(c, "CCLR", L0);
    set(c, "CLKA", L0);
    set(c, "RCLK", L0);
    selects(c, 0);
    set(c, "CCLR", L1);
    step(c);
    for (k = 0; k < 0xFFFF; k++) {
        pulse(c, "CLKA");
    }
    ok &= is(c, "RCOA", L0);
    for (k = 0; k < 6; k++) {
        pulse(c, "CLKA");
    }
    ok &= is(c, "RCOA", L1);
    pulse(c, "RCLK");
    ok &= pins_value(c, Y8, 8, 5);
    selects(c, 2);
    ok &= pins_value(c, Y8, 8, 1);   /* B took the carry: a 32-bit count of 0x10005 */
    set(c, "CCLR", L0);
    step(c);
    ok &= pins_value(c, Y8, 8, 1);   /* CCLR clears the counters, not the register */
    set(c, "CCLR", L1);
    step(c);
    pulse(c, "RCLK");
    expect(ok && pins_value(c, Y8, 8, 0),
           "17 74_8154 GAL shows A low byte, GBU B upper; RCOA low at FFFF; RCOA -> CLKBEN cascade "
           "counts 32 bits; CCLR clears both counters, not the register");
    bench_destroy(c);
}

/* ---- CD4000 sync counters --------------------------------------------------- */

static const char *const JAM[] = {"JAM1", "JAM2", "JAM3", "JAM4"};
static const char *const P14[] = {"P1", "P2", "P3", "P4"};

static void check_4029(void) {
    Bench *b = open_fx("4029B");
    int ok;

    set(b, "PE", L0);
    set(b, "CARRYIN", L0);
    set(b, "UP/DOWN", L1);
    set(b, "BINARY/DECADE", L1);
    set(b, "CLOCK", L0);
    set_pins(b, JAM, 4, 9);
    step(b);
    set(b, "PE", L1);
    step(b);
    ok = pins_value(b, Q14, 4, 9);
    set(b, "PE", L0);
    step(b);
    pulse(b, "CLOCK");
    ok &= pins_value(b, Q14, 4, 10);   /* binary */
    set(b, "BINARY/DECADE", L0);
    set_pins(b, JAM, 4, 9);
    set(b, "PE", L1);
    step(b);
    set(b, "PE", L0);
    step(b);
    pulse(b, "CLOCK");
    ok &= pins_value(b, Q14, 4, 0);    /* decade */
    set(b, "UP/DOWN", L0);
    step(b);
    ok &= is(b, "CARRYOUT", L0);       /* 0 down: terminal */
    pulse(b, "CLOCK");
    ok &= pins_value(b, Q14, 4, 9) && is(b, "CARRYOUT", L1);
    set(b, "BINARY/DECADE", L1);
    set(b, "UP/DOWN", L1);
    set_pins(b, JAM, 4, 15);
    set(b, "PE", L1);
    step(b);
    set(b, "PE", L0);
    step(b);
    ok &= is(b, "CARRYOUT", L0);
    set(b, "CARRYIN", L1);
    step(b);
    expect(ok && is(b, "CARRYOUT", L1),
           "18 4029B binary/decade by the pin; up/down; PRESET ENABLE async; CARRY OUT low at "
           "terminal with CARRY IN low");
    bench_destroy(b);
}

static void check_4510(void) {
    Bench *b = open_fx("4510B");
    int ok;

    set(b, "CARRYIN", L0);
    set(b, "UP/DOWN", L1);
    set(b, "CLOCK", L0);
    set_pins(b, P14, 4, 5);
    set(b, "PE", L1);
    set(b, "RESET", L1);
    step(b);
    ok = pins_value(b, Q14, 4, 0);
    set(b, "RESET", L0);
    step(b);
    expect(ok && pins_value(b, Q14, 4, 5), "19 4510B RESET beats PRESET ENABLE");
    bench_destroy(b);
}

static const char *const Q4A[] = {"Q1A", "Q2A", "Q3A", "Q4A"};

static void check_4518(void) {
    Bench *b = open_fx("4518B");
    int ok;

    set(b, "RESETA", L0);
    set(b, "ENABLEA", L1);
    set(b, "CLOCKA", L0);
    step(b);
    rise(b, "CLOCKA");
    ok = pins_value(b, Q4A, 4, 1);
    fall(b, "CLOCKA");
    fall(b, "ENABLEA");   /* ENABLE falling with CLOCK low */
    ok &= pins_value(b, Q4A, 4, 2);
    rise(b, "CLOCKA");    /* CLOCK rising with ENABLE low */
    ok &= pins_value(b, Q4A, 4, 2);
    fall(b, "CLOCKA");
    rise(b, "ENABLEA");
    ok &= pins_value(b, Q4A, 4, 2);
    set(b, "RESETA", L1);
    step(b);
    expect(ok && pins_value(b, Q4A, 4, 0),
           "20 4518B counts on CLOCK rising with ENABLE high and on ENABLE falling with CLOCK low; "
           "not on CLOCK rising with ENABLE low");
    bench_destroy(b);
}

static const char *const J8[] = {"J0", "J1", "J2", "J3", "J4", "J5", "J6", "J7"};

/* Clocks until CO/ZD goes low; returns the number of clocks (or 1000 if never). */
static unsigned clocks_to_zero(Bench *b) {
    unsigned k;

    for (k = 0; k < 1000 && bench_get(b, "CO/ZD") != L0; k++) {
        pulse(b, "CLOCK");
    }
    return k;
}

static void check_40103(void) {
    Bench *b = open_fx("40103B");
    int ok;
    unsigned n;

    set(b, "CI/CE", L0);
    set(b, "APE", L1);
    set(b, "SPE", L1);
    set(b, "CLOCK", L0);
    set(b, "CLR", L0);
    step(b);
    set(b, "CLR", L1);
    step(b);
    ok = is(b, "CO/ZD", L1);
    n = clocks_to_zero(b);
    ok &= n == 255;
    if (n != 255) {
        printf("  %u clocks from 255 to 0\n", n);
    }
    set_pins(b, J8, 8, 1);
    set(b, "APE", L0);
    step(b);
    ok &= is(b, "CO/ZD", L1);
    set(b, "APE", L1);
    step(b);
    pulse(b, "CLOCK");
    ok &= is(b, "CO/ZD", L0);
    set_pins(b, J8, 8, 3);
    set(b, "SPE", L0);
    step(b);
    ok &= is(b, "CO/ZD", L0);
    pulse(b, "CLOCK");
    set(b, "SPE", L1);
    step(b);
    ok &= is(b, "CO/ZD", L1) && clocks_to_zero(b) == 3;
    pulse(b, "CLOCK");   /* 0 -> 255 */
    ok &= is(b, "CO/ZD", L1);
    expect(ok && clocks_to_zero(b) == 255,
           "21 40103B CLR -> 255; APE async load; SPE at the edge; CO/ZD low at 0 with CI/CE low; "
           "wraps 0 -> 255");
    bench_destroy(b);
}

static void check_40102(void) {
    Bench *b = open_fx("40102B");
    int ok;

    set(b, "CI/CE", L0);
    set(b, "APE", L1);
    set(b, "SPE", L1);
    set(b, "CLOCK", L0);
    set(b, "CLR", L0);
    step(b);
    set(b, "CLR", L1);
    step(b);
    ok = clocks_to_zero(b) == 99;
    set_pins(b, J8, 8, 0x10);   /* BCD 10 */
    set(b, "APE", L0);
    step(b);
    set(b, "APE", L1);
    step(b);
    expect(ok && clocks_to_zero(b) == 10, "22 40102B CLR -> 99; BCD down count 10 -> 09 -> ... -> 0");
    bench_destroy(b);
}

/* ---- 4522B ---------------------------------------------------------------- */

static void check_4522(void) {
    static const char *const JOIN[] = {"ZERO", "PE", NULL};
    static const char *const P[] = {"P0", "P1", "P2", "P3"};
    Bench *b = open_joined("4522B", JOIN);
    unsigned k, pulses = 0;
    int ok = 1;
    LogicValue last = LOGIC_0;

    set(b, "MR", L1);
    set(b, "CF", L1);
    set(b, "CI", L0);
    set(b, "CL", L0);
    set_pins(b, P, 4, 5);
    step(b);
    set(b, "MR", L0);
    step(b);
    step(b);
    last = bench_get(b, "ZERO");
    for (k = 0; k < 20; k++) {
        LogicValue high, low;

        set(b, "CL", L1);
        bench_run(b, NS(200));
        high = bench_get(b, "ZERO");
        set(b, "CL", L0);
        bench_run(b, NS(200));
        low = bench_get(b, "ZERO");
        if (high == L1 && last != L1) {
            pulses++;
            ok &= (k + 1) % 5 == 0;
        }
        ok &= low == L0;
        last = low;
    }
    if (pulses != 4) {
        printf("  %u pulses in 20 clocks\n", pulses);
    }
    expect(ok && pulses == 4,
           "24 4522B N = 5 with ZERO tied to PE: one ZERO pulse per 5 clocks, each during the clock's "
           "high time");
    bench_destroy(b);
}

/* ---- ripple ----------------------------------------------------------------- */

static void check_393(void) {
    static const char *const Q[] = {"1QA", "1QB", "1QC", "1QD"};
    Bench *b = open_fx("74_393");
    int ok = 1;
    unsigned k;

    set(b, "1CLR", L1);
    set(b, "1A", L1);
    step(b);
    set(b, "1CLR", L0);
    step(b);
    for (k = 1; k <= 7; k++) {
        pulse(b, "1A");
        ok &= pins_value(b, Q, 4, k);
    }
    set(b, "1A", L1);
    step(b);
    set(b, "1A", L0);   /* 7 -> 8: QA falls at 10 ns, QD rises at 10 + 3 x 5 ns */
    bench_run(b, NS(15));
    ok &= is(b, "1QA", L0) && is(b, "1QD", L0);
    bench_run(b, NS(15));
    ok &= is(b, "1QD", L1);
    step(b);
    set(b, "1CLR", L1);
    step(b);
    expect(ok && pins_value(b, Q, 4, 0),
           "25 74_393 falling-edge ripple; Q3 changes 3 x stage after Q0; CLR high async");
    bench_destroy(b);
}

static void check_90(void) {
    static const char *const JOIN[] = {"QA", "CKB", NULL};
    Bench *b = open_joined("74_90", JOIN);
    int ok = 1;
    unsigned k;

    set(b, "R0(1)", L1);
    set(b, "R0(2)", L1);
    set(b, "R9(1)", L0);
    set(b, "R9(2)", L0);
    set(b, "CKA", L1);
    step(b);
    set(b, "R0(1)", L0);
    step(b);
    for (k = 1; k <= 10; k++) {
        pulse(b, "CKA");
        set(b, "CKA", L1);
        step(b);
        ok &= pins_value(b, QAD, 4, k % 10);
    }
    set(b, "R9(1)", L1);
    set(b, "R9(2)", L1);
    step(b);
    ok &= pins_value(b, QAD, 4, 9);
    set(b, "R0(1)", L1);
    step(b);
    ok &= pins_value(b, QAD, 4, 9);   /* R9 beats R0 */
    set(b, "R9(1)", L0);
    step(b);
    expect(ok && pins_value(b, QAD, 4, 0),
           "26 74_90 BCD with QA -> CKB (an external wire); R9 sets 9; R9 beats R0");
    bench_destroy(b);
}

static void check_92(void) {
    static const unsigned SEQ[12] = {0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13};
    static const char *const JOIN[] = {"QA", "CKB", NULL};
    Bench *b = open_joined("74_92", JOIN);
    int ok = 1;
    unsigned k;

    set(b, "R0(1)", L1);
    set(b, "R0(2)", L1);
    set(b, "CKA", L1);
    step(b);
    set(b, "R0(1)", L0);
    step(b);
    for (k = 1; k <= 12; k++) {
        fall(b, "CKA");
        rise(b, "CKA");
        ok &= pins_value(b, QAD, 4, SEQ[k % 12]);
    }
    expect(ok, "27 74_92 divide-by-12 sequence (0-5, 8-13 in QD QC QB QA)");
    bench_destroy(b);
}

static void check_4020(void) {
    Bench *b = open_fx("4020B");
    int ok = 1;
    unsigned k;

    set(b, "RESET", L1);
    set(b, "CLK", L1);
    step(b);
    set(b, "RESET", L0);
    step(b);
    for (k = 1; k <= 32; k++) {
        fall(b, "CLK");
        rise(b, "CLK");
        ok &= is(b, "Q4", (k >> 3) & 1u ? L1 : L0) && is(b, "Q1", (k & 1u) ? L1 : L0);
    }
    expect(ok, "28 4020B Q2/Q3 have no pins: Q4 toggles every 8 clocks");
    bench_destroy(b);
}

/* ---- Loader ----------------------------------------------------------------- */

#define BAD_HEAD \
    "{'format':'schematical-parts','version':1,'kind':'counter','devices':[{'id':'bad'," \
    "'function':'Counter','description':'d','drawer':[],'pinouts':{'p4':[" \
    "{'number':1,'name':'CLK','role':'input'},{'number':2,'name':'S','role':'input'}," \
    "{'number':3,'name':'Q','role':'output'},{'number':4,'name':'VCC','role':'power'}]}," \
    "'model':'sync','units':[{"
#define BAD_TAIL "'clock':{'pin':'CLK','edge':'rising'},'count':'up'}]," \
    "'timing':{'HC':{'rise':1,'fall':1,'vcc':[2,6]}},'parts':[]}]}"

static int expect_error(const char *text, const char *want) {
    PartLibrary *lib = fixture_library();
    char *error = load(lib, text, "bad.json");
    int ok = want == NULL ? error == NULL : error != NULL && strstr(error, want) != NULL;

    if (!ok) {
        printf("  error: %s\n  want: %s\n", error != NULL ? error : "(none)",
               want != NULL ? want : "(none)");
    }
    free(error);
    part_library_destroy(lib);
    return ok;
}

static void check_loader(void) {
    int ok = expect_error(BAD_HEAD "'stages':0,'code':'binary'," BAD_TAIL,
                          "\"stages\" must be 1 to 32");

    ok &= expect_error(BAD_HEAD "'stages':6,'code':'bcd'," BAD_TAIL,
                       "a bcd unit's \"stages\" must be a multiple of 4");
    ok &= expect_error(BAD_HEAD "'stages':4,'code':'binary','illegal':{'up':[],'down':[]}," BAD_TAIL,
                       "\"illegal\" is only for a bcd unit");
    ok &= expect_error(BAD_HEAD "'stages':4,'codeSelect':{'pin':'S','binary':'high'}," BAD_TAIL,
                       NULL);
    expect(ok, "29 loader: stages 0, bcd with 6 stages and illegal on a binary unit are errors; "
               "codeSelect with no code loads");
}

/* ---- plan-10-E: johnson ---------------------------------------------------- */

/* The decoded output that is high, or -1 when none or several are. */
static int decoded(Bench *b, unsigned n) {
    char name[4];
    unsigned i;
    int which = -1;

    for (i = 0; i < n; i++) {
        snprintf(name, sizeof name, "%u", i);
        if (bench_get(b, name) == L1) {
            if (which >= 0) {
                return -1;
            }
            which = (int)i;
        } else if (bench_get(b, name) != L0) {
            return -1;
        }
    }
    return which;
}

static void johnson_cycle(const char *device, const char *clock, const char *gate,
                          const char *reset, const char *carry, unsigned n, const char *what) {
    Bench *b = open_fx(device);
    int ok;
    unsigned k;

    set(b, gate, L0);
    set(b, clock, L0);
    set(b, reset, L1);
    step(b);
    set(b, reset, L0);
    step(b);
    ok = decoded(b, n) == 0 && is(b, carry, L1);
    for (k = 1; k <= 2 * n; k++) {
        pulse(b, clock);
        if (decoded(b, n) != (int)(k % n) || bench_get(b, carry) != ((k % n) < n / 2 ? L1 : L0)) {
            printf("  after %u clocks: output %d, carry %c\n", k, decoded(b, n),
                   "01XZ"[bench_get(b, carry)]);
            ok = 0;
        }
    }
    expect(ok, what);
    bench_destroy(b);
}

static void check_4017(void) {
    Bench *b;
    int ok;

    johnson_cycle("4017B", "CLOCK", "CLOCKINHIBIT", "RESET", "CARRYOUT", 10,
                  "30 4017B each output high for one clock in order 0-9; carry high for 0-4");

    b = open_fx("4017B");
    set(b, "CLOCKINHIBIT", L0);
    set(b, "CLOCK", L0);
    set(b, "RESET", L1);
    step(b);
    set(b, "RESET", L0);
    step(b);
    set(b, "CLOCKINHIBIT", L1);
    step(b);
    pulse(b, "CLOCK");
    ok = decoded(b, 10) == 0;
    rise(b, "CLOCK");
    fall(b, "CLOCKINHIBIT");   /* inhibit falling with CLOCK high advances */
    ok &= decoded(b, 10) == 1;
    fall(b, "CLOCK");
    expect(ok && decoded(b, 10) == 1,
           "31 4017B INHIBIT high blocks; INHIBIT falling with CLOCK high advances");
    pulse(b, "CLOCK");
    pulse(b, "CLOCK");
    ok = decoded(b, 10) == 3;
    set(b, "RESET", L1);
    step(b);
    expect(ok && decoded(b, 10) == 0 && is(b, "CARRYOUT", L1), "32 4017B RESET async -> output 0 high");
    bench_destroy(b);
}

static const char *const QBAR18[] = {"Q1", "Q2", "Q3", "Q4", "Q5"};

/* The 4018B's stages Q1..Q5 (pins show NOT the stage) as bits, Q1 = bit 0; -1 if unknown. */
static int stages_4018(Bench *b) {
    int v = 0;
    unsigned i;

    for (i = 0; i < 5; i++) {
        LogicValue p = bench_get(b, QBAR18[i]);

        if (p != L0 && p != L1) {
            return -1;
        }
        v |= (p == L0 ? 1 : 0) << i;
    }
    return v;
}

/* 1 when v (Q1 = bit 0) is one of the 10 states of the decade cycle. */
static int legal_johnson5(int v) {
    static const int LEGAL[10] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x1E, 0x1C, 0x18, 0x10};
    int i;

    for (i = 0; i < 10; i++) {
        if (LEGAL[i] == v) {
            return 1;
        }
    }
    return 0;
}

static const char *const JAM18[] = {"JAM1", "JAM2", "JAM3", "JAM4", "JAM5"};

static void check_4018(void) {
    static const char *const JOIN[] = {"Q5", "DATA", NULL};
    Bench *b = open_joined("4018B", JOIN);
    int ok = 1;
    int seen11111 = 0;
    unsigned k;
    int first;

    /* 33: preset an illegal state, then clock with DATA = Q5 pin (NOT stage 5). */
    set(b, "RESET", L0);
    set(b, "CLOCK", L0);
    set_pins(b, JAM18, 5, 0x04);   /* stage 3 only */
    set(b, "PE", L1);
    step(b);
    ok = stages_4018(b) == 0x04;
    set(b, "PE", L0);
    step(b);
    for (k = 0; k < 10; k++) {
        pulse(b, "CLOCK");
    }
    ok &= legal_johnson5(stages_4018(b));
    expect(ok, "33 4018B illegal preset 00100 reaches the legal cycle within 10 clocks (anti-lock)");

    /* 36a: DATA = Q5 pin: divide by 10. */
    first = stages_4018(b);
    ok = 1;
    for (k = 1; k <= 10; k++) {
        pulse(b, "CLOCK");
        ok &= (stages_4018(b) == first) == (k == 10);
    }
    bench_destroy(b);

    /* 36b: DATA = Q4 pin AND Q5 pin (the bench's AND): divide by 9, skipping 11111. */
    b = open_fx("4018B");
    set(b, "RESET", L1);
    set(b, "PE", L0);
    set(b, "CLOCK", L0);
    step(b);
    set(b, "RESET", L0);
    step(b);
    first = stages_4018(b);
    for (k = 1; k <= 18; k++) {
        LogicValue d = bench_get(b, "Q4") == L1 && bench_get(b, "Q5") == L1 ? L1 : L0;

        set(b, "DATA", d);
        step(b);
        pulse(b, "CLOCK");
        seen11111 |= stages_4018(b) == 0x1F;
        if (k == 9 || k == 18) {
            ok &= stages_4018(b) == first;
        } else {
            ok &= stages_4018(b) != first;
        }
    }
    expect(ok && !seen11111,
           "36 4018B DATA from Q5: divide by 10; DATA from Q4 AND Q5: divide by 9 skipping 11111");

    /* 37: RESET beats PRESET ENABLE. */
    set_pins(b, JAM18, 5, 0x1F);
    set(b, "PE", L1);
    set(b, "RESET", L1);
    step(b);
    ok = stages_4018(b) == 0;
    set(b, "RESET", L0);
    step(b);
    expect(ok && stages_4018(b) == 0x1F, "37 4018B RESET beats PRESET ENABLE");
    bench_destroy(b);
}

static void check_4022(void) {
    johnson_cycle("4022B", "CLOCK", "CLOCKINHIBIT", "RESET", "CARRYOUT", 8,
                  "34 4022B 8 decoded outputs; carry high for 0-3");
}

static void check_74_4017(void) {
    Bench *b = open_fx("74_4017");
    int ok;

    set(b, "CE", L0);
    set(b, "CP", L0);
    set(b, "MR", L1);
    step(b);
    set(b, "MR", L0);
    step(b);
    pulse(b, "CP");
    pulse(b, "CP");
    ok = decoded(b, 10) == 2;
    set(b, "CE", L1);
    step(b);
    pulse(b, "CP");
    ok &= decoded(b, 10) == 2;
    set(b, "MR", L1);
    step(b);
    expect(ok && decoded(b, 10) == 0 && is(b, "TC", L1), "35 74_4017 CE low counts; MR high resets");
    bench_destroy(b);
}

/* ---- plan-10-E: seg7 ------------------------------------------------------- */

static const char *const GLYPH7[10] = {"1111110", "0110000", "1101101", "1111001", "0110011",
                                       "1011011", "1011111", "1110000", "1111111", "1111011"};

static int seg7_is(Bench *b, const char *glyph) {
    static const char *const S[7] = {"a", "b", "c", "d", "e", "f", "g"};
    unsigned j;
    int ok = 1;

    for (j = 0; j < 7; j++) {
        ok &= is(b, S[j], glyph[j] == '1' ? L1 : L0);
    }
    return ok;
}

static void check_4026(void) {
    Bench *b = open_fx("4026B");
    int ok;
    unsigned k;

    set(b, "CLOCKINHIBIT", L0);
    set(b, "CLOCK", L0);
    set(b, "DEI", L1);
    set(b, "RESET", L1);
    step(b);
    set(b, "RESET", L0);
    step(b);
    ok = seg7_is(b, GLYPH7[0]) && is(b, "CARRYOUT", L1);
    for (k = 1; k <= 10; k++) {
        pulse(b, "CLOCK");
        ok &= seg7_is(b, GLYPH7[k % 10]) && is(b, "CARRYOUT", (k % 10) <= 4 ? L1 : L0);
    }
    set(b, "DEI", L0);
    step(b);
    ok &= seg7_is(b, "0000000") && is(b, "DEO", L0) && is(b, "UNGATEDC", L1);
    pulse(b, "CLOCK");
    pulse(b, "CLOCK");   /* count 2: c is the only segment off */
    expect(ok && seg7_is(b, "0000000") && is(b, "UNGATEDC", L0),
           "38 4026B glyph per count 0-9; DEI low -> segments 0, ungated C still follows");
    bench_destroy(b);
}

static void check_4033(void) {
    Bench *b = open_fx("4033B");
    int ok;

    set(b, "CLOCKINHIBIT", L0);
    set(b, "CLOCK", L0);
    set(b, "LT", L0);
    set(b, "RBI", L0);
    set(b, "RESET", L1);
    step(b);
    set(b, "RESET", L0);
    step(b);
    ok = seg7_is(b, "0000000") && is(b, "RBO", L0);
    set(b, "RBI", L1);
    step(b);
    ok &= seg7_is(b, GLYPH7[0]) && is(b, "RBO", L1);
    set(b, "RBI", L0);
    step(b);
    pulse(b, "CLOCK");
    ok &= seg7_is(b, GLYPH7[1]) && is(b, "RBO", L1);
    set(b, "LT", L1);
    step(b);
    expect(ok && seg7_is(b, "1111111"),
           "39 4033B RBI low at 0 -> blank and RBO low; LAMP TEST high -> all 1");
    bench_destroy(b);
}

static void check_40110(void) {
    Bench *b = open_fx("40110B");
    int ok;
    unsigned k;

    set(b, "TE", L0);
    set(b, "LE", L0);
    set(b, "CLOCKUP", L0);
    set(b, "CLOCKDOWN", L0);
    set(b, "RESET", L1);
    step(b);
    set(b, "RESET", L0);
    step(b);
    ok = seg7_is(b, GLYPH7[0]) && is(b, "CARRY", L1) && is(b, "BORROW", L1);
    for (k = 1; k <= 9; k++) {
        pulse(b, "CLOCKUP");
    }
    ok &= seg7_is(b, GLYPH7[9]);
    set(b, "CLOCKUP", L1);   /* 9 -> 0: a 230 ns CARRY pulse from 10 ns */
    bench_run(b, NS(120));
    ok &= is(b, "CARRY", L0) && seg7_is(b, GLYPH7[0]);
    bench_run(b, NS(130));
    ok &= is(b, "CARRY", L1);
    set(b, "CLOCKUP", L0);
    step(b);
    set(b, "CLOCKDOWN", L1);   /* 0 -> 9: a 275 ns BORROW pulse */
    bench_run(b, NS(270));
    ok &= is(b, "BORROW", L0) && seg7_is(b, GLYPH7[9]);
    bench_run(b, NS(30));
    ok &= is(b, "BORROW", L1) && is(b, "CARRY", L1);
    expect(ok, "40 40110B CLOCK UP and CLOCK DOWN count; 9->0 gives a 230 ns CARRY low pulse; 0->9 a "
               "275 ns BORROW pulse");

    set(b, "CLOCKDOWN", L0);
    set(b, "LE", L1);
    step(b);
    pulse(b, "CLOCKUP");
    ok = seg7_is(b, GLYPH7[9]);   /* count 0, display held at 9 */
    set(b, "LE", L0);
    step(b);
    ok &= seg7_is(b, GLYPH7[0]);
    set(b, "TE", L1);
    step(b);
    pulse(b, "CLOCKUP");
    expect(ok && seg7_is(b, GLYPH7[0]),
           "41 40110B LATCH ENABLE high holds the display while the count moves; TOGGLE ENABLE high "
           "blocks counting");
    bench_destroy(b);
}

/* ---- plan-10-E: divn ----------------------------------------------------------- */

static const char *const J16[] = {"J1", "J2", "J3", "J4", "J5", "J6", "J7", "J8",
                                  "J9", "J10", "J11", "J12", "J13", "J14", "J15", "J16"};

static void k_pins(Bench *b, int ka, int kb, int kc) {
    set(b, "Ka", ka ? L1 : L0);
    set(b, "Kb", kb ? L1 : L0);
    set(b, "Kc", kc ? L1 : L0);
    step(b);
}

/* Clocks n edges; records in edges[] (up to cap) the edge numbers (from 1) after which the
   output is high in the clock's high half. Returns the number recorded. */
static unsigned divn_pulses(Bench *b, const char *clock, const char *out, unsigned n,
                            unsigned *edges, unsigned cap) {
    unsigned k, count = 0;

    for (k = 1; k <= n; k++) {
        set(b, clock, L1);
        bench_run(b, 2 * step_ps);
        if (bench_get(b, out) == L1 && count < cap) {
            edges[count] = k;
            count++;
        }
        set(b, clock, L0);
        bench_run(b, 2 * step_ps);
    }
    return count;
}

/* Master preset with the given mode pins as the release, then n edges. */
static unsigned divn_run(Bench *b, int ka, int kb, int kc, unsigned jam, unsigned n,
                         unsigned *edges, unsigned cap) {
    set(b, "L", L0);
    set(b, "CL", L0);
    set_pins(b, J16, 16, jam);
    k_pins(b, ka, 0, 0);
    divn_pulses(b, "CL", "OUT", 3, edges, 0);
    k_pins(b, ka, kb, kc);
    return divn_pulses(b, "CL", "OUT", n, edges, cap);
}

static void check_4059_n3(void) {
    unsigned edges[8];
    Bench *b = open_fx("4059A");
    unsigned n;
    int ok;

    n = divn_run(b, 0, 0, 1, 0x0003, 12, edges, 8);   /* ÷8 mode, N = 3 */
    ok = n == 3 && edges[0] == 4 && edges[1] == 7 && edges[2] == 10;
    if (!ok) {
        printf("  %u pulses, first at %u, %u, %u\n", n, n > 0 ? edges[0] : 0, n > 1 ? edges[1] : 0,
               n > 2 ? edges[2] : 0);
    }
    expect(ok, "42 4059A divide-by-8 mode, N = 3: master preset, release, first pulse on the 4th edge, "
               "then every 3rd");
    bench_destroy(b);
}

static void check_4059(void) {
    unsigned edges[8];
    Bench *b;
    unsigned n;
    int ok;

    check_4059_n3();

    /* Example A: mode 5, J1..J16 = 0011 1010 1001 0110 (J1 first) -> N = 8479. */
    b = open_fx("4059A");
    n = divn_run(b, 1, 0, 1, 0x0C | (0x5u << 4) | (0x9u << 8) | (0x6u << 12), 2 * 8479 + 2, edges,
                 8);
    ok = n == 2 && edges[1] - edges[0] == 8479;
    if (!ok) {
        printf("  %u pulses, at %u and %u\n", n, n > 0 ? edges[0] : 0, n > 1 ? edges[1] : 0);
    }
    expect(ok, "43 4059A datasheet example A: mode 5, J = 0011 1010 1001 0110 -> pulses 8479 edges "
               "apart");
    bench_destroy(b);

    b = open_fx("4059A");
    n = divn_run(b, 0, 0, 1, 0x0003, 4, edges, 8);
    ok = n == 1;
    set(b, "L", L1);
    step(b);
    divn_pulses(b, "CL", "OUT", 1, edges, 0);   /* edge 5 would end the pulse */
    ok &= is(b, "OUT", L1);
    divn_pulses(b, "CL", "OUT", 4, edges, 0);
    ok &= is(b, "OUT", L1);
    set(b, "L", L0);
    step(b);
    expect(ok && is(b, "OUT", L0), "44 4059A latch enable 1 keeps the output high after a pulse");
    bench_destroy(b);

    b = open_fx("4059A");
    n = divn_run(b, 1, 1, 0, 0x0005 | (0x2u << 4), 2 + 25 * 3, edges, 8);   /* ÷10, N = 25 */
    ok = n == 3 && edges[1] - edges[0] == 25 && edges[2] - edges[1] == 25;
    if (!ok) {
        printf("  %u pulses, at %u, %u, %u\n", n, n > 0 ? edges[0] : 0, n > 1 ? edges[1] : 0,
               n > 2 ? edges[2] : 0);
    }
    expect(ok, "45 4059A divide-by-10 mode, N = 25: pulses every 25 edges");
    bench_destroy(b);
}

/* ---- plan-10-E: rate ------------------------------------------------------------- */

/* Counts the OUT pulses (out high in the clock-low half) over n clocks; *bad set when out is
   high in a clock-high half. */
static unsigned rate_pulses(Bench *b, const char *clock, const char *out, unsigned n, int *bad) {
    unsigned k, count = 0;

    for (k = 0; k < n; k++) {
        set(b, clock, L1);
        step(b);
        if (bench_get(b, out) != L0) {
            *bad = 1;
        }
        set(b, clock, L0);
        step(b);
        count += bench_get(b, out) == L1;
    }
    return count;
}

static const char *const RATE4[] = {"A", "B", "C", "D"};

static Bench *open_rate(const char *device) {
    Bench *b = open_fx(device);

    set(b, "CLOCK", L0);
    set(b, "INHIN", L0);
    set(b, "STROBE", L0);
    set(b, "CASCADE", L0);
    set(b, device[2] == '8' ? "SET15" : "SET9", L0);
    set(b, "CLEAR", L1);
    step(b);
    set(b, "CLEAR", L0);
    step(b);
    return b;
}

static void check_4089(void) {
    Bench *b = open_rate("4089B");
    int bad = 0;
    unsigned n;
    int ok;

    set_pins(b, RATE4, 4, 13);
    step(b);
    n = rate_pulses(b, "CLOCK", "OUT", 16, &bad);
    expect(n == 13 && !bad, "46 4089B rate 13 -> 13 OUT pulses per 16 clocks, each during clock low");
    if (n != 13) {
        printf("  %u pulses\n", n);
    }
    set(b, "STROBE", L1);
    step(b);
    n = rate_pulses(b, "CLOCK", "OUT", 16, &bad);
    ok = n == 0 && is(b, "OUTBAR", L1);
    set(b, "STROBE", L0);
    set(b, "CASCADE", L1);
    step(b);
    expect(ok && is(b, "OUT", L1), "47 4089B STROBE high -> no pulses, OUTBAR high; CASCADE high -> "
                                   "OUT high");
    set(b, "CASCADE", L0);
    set(b, "SET15", L1);
    step(b);
    ok = is(b, "15OUT", L1) && is(b, "INHOUT", L0);
    set(b, "CLEAR", L1);
    step(b);
    ok &= is(b, "15OUT", L1);   /* set beats clear */
    set(b, "SET15", L0);
    step(b);
    expect(ok && is(b, "15OUT", L0) && is(b, "INHOUT", L1),
           "48 4089B SET -> count 15, \"15\" high, INHIBIT OUT low; CLEAR -> 0");
    bench_destroy(b);
}

static void check_4527(void) {
    static const unsigned WANT[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 9, 8, 9, 8, 9};
    int ok = 1;
    unsigned rate;

    for (rate = 0; rate < 16; rate++) {
        Bench *b = open_rate("4527B");
        int bad = 0;
        unsigned n;

        set_pins(b, RATE4, 4, rate);
        step(b);
        n = rate_pulses(b, "CLOCK", "OUT", 10, &bad);
        if (n != WANT[rate] || bad) {
            printf("  rate %u: %u pulses per 10 (want %u)%s\n", rate, n, WANT[rate],
                   bad ? ", one during clock high" : "");
            ok = 0;
        }
        bench_destroy(b);
    }
    expect(ok, "49 4527B rate 8 -> 8 pulses per 10; rates 10-15 -> 8 or 9 per the truth table");
}

static const char *const RATE6[] = {"B0", "B1", "B2", "B3", "B4", "B5"};

static Bench *check_97_rate(void) {
    Bench *b = open_fx("74_97");
    int bad = 0;
    unsigned n;
    int ok;

    set(b, "CLK", L0);
    set(b, "ENIN", L0);
    set(b, "STRB", L0);
    set(b, "UNITY/CAS", L1);
    set(b, "CLR", L1);
    step(b);
    set(b, "CLR", L0);
    set_pins(b, RATE6, 6, 40);
    step(b);
    n = rate_pulses(b, "CLK", "Y", 64, &bad);
    ok = n == 40 && !bad;
    if (n != 40) {
        printf("  %u Y pulses per 64\n", n);
    }
    set(b, "UNITY/CAS", L0);
    step(b);
    expect(ok && is(b, "Y", L1), "50 74_97 M = 40 -> 40 Y pulses per 64 clocks; UNITY/CAS low -> Y "
                                 "high");
    return b;
}

static void check_97(void) {
    Bench *b = check_97_rate();
    unsigned k;
    int ok;

    set(b, "UNITY/CAS", L1);
    for (k = 0; k < 63; k++) {
        pulse(b, "CLK");
    }
    ok = is(b, "ENOUT", L0);
    set(b, "ENIN", L1);
    step(b);
    expect(ok && is(b, "ENOUT", L1), "51 74_97 ENABLE out low at count 63 with EN low");
    bench_destroy(b);
}

/* ---- plan-10-E: loader -------------------------------------------------------- */

#define E_HEAD(PINS) \
    "{'format':'schematical-parts','version':1,'kind':'counter','devices':[{'id':'bad'," \
    "'function':'Counter','description':'d','drawer':[],'pinouts':{'p4':[" PINS "]},"
#define E_PINS \
    "{'number':1,'name':'A','role':'input'},{'number':2,'name':'B','role':'input'}," \
    "{'number':3,'name':'C','role':'output'},{'number':4,'name':'D','role':'input'}"
#define E_TAIL "'timing':{'HC':{'rise':1,'fall':1,'vcc':[2,6]}},'parts':[]}]}"

static void check_e_loader(void) {
    int ok = expect_error(E_HEAD(E_PINS) "'model':'divn','clock':'A','jam':['A','A','A','A','A',"
                          "'A','A','A','A','A','A','A','A','A','A'],'modes':['A','B','D'],"
                          "'latchEnable':'B','output':'C'," E_TAIL,
                          "\"jam\" must have 16 names");

    ok &= expect_error(E_HEAD(E_PINS) "'model':'rate','variant':'x'," E_TAIL,
                       "\"variant\" must be \"binary4\", \"bcd\" or \"binary6\", not \"x\"");
    expect(ok, "52 loader: divn with 15 jam pins and rate variant \"x\" are errors");
    ok = expect_error(E_HEAD(E_PINS) "'model':'seg7','clock':'A','reset':{'pin':'B','active':'high'},"
                      "'segments':['C','C','C','C','C','C','C'],'glyphs':['0000000','0000000',"
                      "'0000000','0000000','0000000','0000000','0000000','0000000','0000000'],"
                      E_TAIL,
                      "\"glyphs\" must have 10 names");
    expect(ok, "53 loader: seg7 with 9 glyphs is an error");

    /* plan-56-F: the SN74143 keys (counter_sn74143_test runs the part itself) */
#define SEG7_BASE \
    E_HEAD(E_PINS) "'model':'seg7','clock':'A','reset':{'pin':'B','active':'high'}," \
    "'segments':['C','C','C','C','C','C','C'],'glyphs':['0000000','0000000','0000000','0000000'," \
    "'0000000','0000000','0000000','0000000','0000000','0000000'],"
    ok = expect_error(SEG7_BASE "'segmentsOutput':'open-drain'," E_TAIL,
                      "open-drain \"segmentsOutput\" needs \"segmentsActive\" low");
    ok &= expect_error(SEG7_BASE "'segmentsActive':'low','segmentsOutput':'open-drain'," E_TAIL, NULL);
    ok &= expect_error(SEG7_BASE "'segmentsActive':'mid'," E_TAIL, "\"segmentsActive\" must be \"high\" or \"low\"");
    ok &= expect_error(SEG7_BASE "'segmentsOutput':'weak'," E_TAIL,
                       "\"segmentsOutput\" must be \"push-pull\" or \"open-drain\"");
    ok &= expect_error(SEG7_BASE "'rbi':'A'," E_TAIL, "\"rbi\" and \"rbo\" go together");
    ok &= expect_error(SEG7_BASE "'rbi':'A','rbo':'D','rippleBlank':{'in':'A','out':'C'}," E_TAIL,
                       "alternatives");
    ok &= expect_error(SEG7_BASE "'latchOutputs':['A','B','C']," E_TAIL, "\"latchOutputs\" must have 4 names");
    ok &= expect_error(SEG7_BASE "'toggleEnable':[]," E_TAIL, "must have 1 to 4 controls");
    ok &= expect_error(SEG7_BASE "'toggleEnable':[{'pin':'A','active':'low'},{'pin':'D','active':'low'}]," E_TAIL,
                       NULL);
    ok &= expect_error(SEG7_BASE "'maxCount':{'pin':'C'}," E_TAIL, "enable");
    ok &= expect_error(SEG7_BASE "'decimalPoint':{'in':'A'}," E_TAIL, "out");
    expect(ok, "54 loader: seg7 segmentsActive/segmentsOutput/rbi+rbo/latchOutputs/toggleEnable/maxCount/decimalPoint rules");
}

/* ---- plan-10-F: real parts ------------------------------------------------- */

static void check_4040_real(void) {
    Bench *b = bench_create(part_library_find(real_library, "CD4040BE"));
    int ok = 1;
    unsigned k;

    set(b, "RESET", L1);
    set(b, "CLK", L1);
    step(b);
    set(b, "RESET", L0);
    step(b);
    for (k = 1; k <= 4096; k++) {
        fall(b, "CLK");
        rise(b, "CLK");
        if (k % 2048 == 0 || k % 2048 == 2047 || k == 1) {
            ok &= is(b, "Q12", (k >> 11) & 1u ? L1 : L0);
        }
    }
    expect(ok, "Q12 toggles every 2048 clocks");
    bench_destroy(b);
}

static void real(const char *part, SimTime step, void (*check)(void)) {
    real_part = part;
    step_ps = step;
    check();
    real_part = NULL;
    step_ps = NS(50);
}

static void check_193_real(void) {
    static const char *const Q[] = {"QA", "QB", "QC", "QD"};

    check_dual_clock("74_193", "UP", "DOWN", "LOAD", L0, "CLR", "CO", "BO", Q, ABCD,
                     "9 UP rising counts with DOWN high; CO low while UP low at 15; BO low while "
                     "DOWN low at 0; CLR high beats LOAD");
}

static void check_4017_real(void) {
    johnson_cycle("4017B", "CLOCK", "CLOCKINHIBIT", "RESET", "CARRYOUT", 10,
                  "30 each output high for one clock in order 0-9; carry high for 0-4");
}

static void check_97_real(void) {
    bench_destroy(check_97_rate());
}

static void check_real_parts(void) {
    real_library = part_library_create();
    if (real_library == NULL || load_parts_dir(real_library, PARTS_DIR) != 0) {
        printf("FAIL: cannot load the part library\n");
        failures++;
        return;
    }
    real("SN74HC161N", NS(100), check_161);
    real("SN74LS193N", NS(100), check_193_real);
    real("SN74HC590AN", NS(100), check_590);
    real("SN74LV8154N", NS(100), check_8154);
    real_part = "CD4040BE";
    step_ps = NS(2000);
    check_4040_real();
    real_part = NULL;
    step_ps = NS(50);
    real("CD4017BE", NS(1000), check_4017_real);
    real("CD4059AM", NS(1000), check_4059_n3);
    real("SN7497N", NS(100), check_97_real);
    part_library_destroy(real_library);
}

int main(void) {
    FIXTURE = read_file(GOLDEN_DIR "/../fixtures/counter/parts.json");
    FIXTURE_FOOTPRINTS = read_file(GOLDEN_DIR "/../fixtures/counter/footprints.json");
    if (FIXTURE == NULL || FIXTURE_FOOTPRINTS == NULL) {
        printf("FAIL: cannot read the fixture files\n");
        return 1;
    }
    static const char *const Q193[] = {"QA", "QB", "QC", "QD"};
    static const char *const J14[] = {"J1", "J2", "J3", "J4"};
    char *error;

    library = fixture_library();
    error = load(library, FIXTURE, "counter_fixture.json");
    if (error != NULL) {
        printf("FAIL: fixture: %s\n", error);
        free(error);
        return 1;
    }
    check_161();
    check_163();
    check_169();
    check_190();
    check_191();
    check_dual_clock("74_193", "UP", "DOWN", "LOAD", L0, "CLR", "CO", "BO", Q193, ABCD,
                     "9 74_193 UP rising counts with DOWN high; CO low while UP low at 15; BO low "
                     "while DOWN low at 0; CLR high beats LOAD");
    check_561();
    check_569();
    check_867();
    check_590();
    check_592();
    check_593();
    check_697();
    check_8154();
    check_4029();
    check_4510();
    check_4518();
    check_40103();
    check_40102();
    check_dual_clock("40193B", "CLOCKUP", "CLOCKDOWN", "PE", L0, "RESET", "CARRY", "BORROW", Q14,
                     J14, "23 40193B as 74_193 with an active-low PRESET ENABLE");
    check_4522();
    check_393();
    check_90();
    check_92();
    check_4020();
    check_loader();
    check_4017();
    check_4018();
    check_4022();
    check_74_4017();
    check_4026();
    check_4033();
    check_40110();
    check_4059();
    check_4089();
    check_4527();
    check_97();
    check_e_loader();
    check_real_parts();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("counter_test: all passed\n");
    return 0;
}
