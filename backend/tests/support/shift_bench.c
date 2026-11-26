/* shift_bench: shared by the programs split from shift_test.c (plan-68-H). */
#include "shift_bench.h"


int failures = 0;
PartLibrary *library;

/* Real-part runs (plan-10-I section 4.3): the cases open this part from the
   real library instead of their fixture, and step this long. */
PartLibrary *real_library;
const char *real_part;
SimTime step_ps = NS(50);

void expect(int ok, const char *what) {
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
/* The fixture parts: tests/fixtures/shift/parts.json (moved out of C by plan-68-H). */
char *FIXTURE;

/* Dual-row fixture footprints FX<n>, one per pin count above. */
/* Their footprints: tests/fixtures/shift/footprints.json. */
char *FIXTURE_FOOTPRINTS;

char *dequote(const char *text) {
    size_t n = strlen(text);
    char *copy = malloc(n + 1);
    size_t i;

    for (i = 0; i <= n; i++) {
        copy[i] = text[i] == '\'' ? '"' : text[i];
    }
    return copy;
}

char *load(PartLibrary *lib, const char *text, const char *source) {
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

PartLibrary *fixture_library(void) {
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

const LogicValue L0 = LOGIC_0, L1 = LOGIC_1, LZ = LOGIC_Z;

const Part *fx_part(const char *device) {
    char name[64];
    const Part *part;

    snprintf(name, sizeof name, "FX-%s", device);
    part = real_part != NULL ? part_library_find(real_library, real_part)
                             : part_library_find(library, name);
    if (part == NULL || part->kind != PART_SHIFT) {
        fprintf(stderr, "no shift part %s\n", name);
        exit(1);
    }
    return part;
}

Bench *open_fx(const char *device) {
    return bench_create(fx_part(device));
}

void step(Bench *b) {
    bench_run(b, step_ps);
}

void set(Bench *b, const char *pin, LogicValue v) {
    bench_set(b, pin, v);
}

void pulse(Bench *b, const char *pin) {
    set(b, pin, L1);
    step(b);
    set(b, pin, L0);
    step(b);
}

LogicValue bit(unsigned value, unsigned i) {
    return (value >> i) & 1u ? LOGIC_1 : LOGIC_0;
}

void set_pins(Bench *b, const char *const *names, unsigned n, unsigned value) {
    unsigned i;

    for (i = 0; i < n; i++) {
        set(b, names[i], bit(value, i));
    }
}

void release_pins(Bench *b, const char *const *names, unsigned n) {
    unsigned i;

    for (i = 0; i < n; i++) {
        set(b, names[i], LZ);
    }
}

int pins_value(Bench *b, const char *const *names, unsigned n, unsigned value) {
    unsigned i;
    int ok = 1;

    for (i = 0; i < n; i++) {
        LogicValue got = bench_get(b, names[i]);

        if (got != bit(value, i)) {
            printf("  %s = %c, want %c (value 0x%X)\n", names[i], "01XZ"[got],
                   "01XZ"[bit(value, i)], value);
            ok = 0;
        }
    }
    return ok;
}

int pins_all(Bench *b, const char *const *names, unsigned n, LogicValue v) {
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

int is(Bench *b, const char *pin, LogicValue v) {
    LogicValue got = bench_get(b, pin);

    if (got != v) {
        printf("  %s = %c, want %c\n", pin, "01XZ"[got], "01XZ"[v]);
        return 0;
    }
    return 1;
}

/* Shifts the bits of value in, LSB first, through pin on clock pulses. */
void shift_in(Bench *b, const char *serial, const char *clock, unsigned n, unsigned value) {
    unsigned i;

    for (i = 0; i < n; i++) {
        set(b, serial, bit(value, i));
        step(b);
        pulse(b, clock);
    }
}

const char *const Q8[] = {"QA", "QB", "QC", "QD", "QE", "QF", "QG", "QH"};
const char *const D8[] = {"A", "B", "C", "D", "E", "F", "G", "H"};

/* After shifting bits b0..b(n-1) in (b0 first), stage k holds b(n-1-k). */
unsigned reversed(unsigned value, unsigned n) {
    unsigned r = 0, i;

    for (i = 0; i < n; i++) {
        r |= ((value >> i) & 1u) << (n - 1 - i);
    }
    return r;
}


void check_165(void) {
    Bench *b = open_fx("74_165");
    int ok;

    set(b, "CLKINH", L0);
    set(b, "CLK", L0);
    set(b, "SER", L0);
    set_pins(b, D8, 8, 0x80);   /* H = 1 */
    set(b, "SH/LD", L0);
    step(b);
    ok = is(b, "QH", L1) && is(b, "QHBAR", L0);
    set(b, "H", L0);   /* transparent while SH/LD is low */
    step(b);
    ok &= is(b, "QH", L0) && is(b, "QHBAR", L1);
    set(b, "G", L1);   /* data 0x40: G = 1, H = 0 */
    step(b);
    set(b, "SH/LD", L1);
    step(b);
    ok &= is(b, "QH", L0);
    set(b, "CLKINH", L1);   /* CLK INH rising with CLK low: one shift, G -> H */
    step(b);
    ok &= is(b, "QH", L1);
    pulse(b, "CLK");   /* CLK INH high blocks (F = 0 would move in) */
    expect(ok && is(b, "QH", L1),
           "3 74_165 SH/LD low loads and is transparent; CLK INH rising with CLK low shifts; CLK INH "
           "high blocks");
    bench_destroy(b);
}

void check_299(void) {
    Bench *b = open_fx("74_299");
    int ok;

    set(b, "CLR", L1);
    set(b, "CLK", L0);
    set(b, "SR", L1);
    set(b, "SL", L0);
    set(b, "OE1", L1);
    set(b, "OE2", L0);
    set(b, "S0", L1);
    set(b, "S1", L1);   /* load from the pins */
    set_pins(b, IO299, 8, 0x3C);
    step(b);
    pulse(b, "CLK");
    ok = is(b, "QA'", L0) && is(b, "QH'", L0);
    release_pins(b, IO299, 8);
    set(b, "S1", L0);   /* right shift; outputs still off (OE1 high) */
    step(b);
    ok &= pins_all(b, IO299, 8, LZ);
    set(b, "OE1", L0);
    step(b);
    ok &= pins_value(b, IO299, 8, 0x3C);
    pulse(b, "CLK");
    ok &= pins_value(b, IO299, 8, 0x79) && is(b, "QA'", L1);
    set(b, "S1", L1);   /* both S high: load mode, the pins are inputs */
    step(b);
    ok &= pins_all(b, IO299, 8, LZ) && is(b, "QH'", L0);
    expect(ok && bench_error(b) == NULL,
           "7 74_299 I/O pins driven only with OE1 = OE2 = 0 and not both S high; load reads the "
           "pins; QA'/QH' always driven");
    bench_destroy(b);
}

/* ---- 595 / 594 / 596 / 597 / 598 ------------------------------------------------- */

void check_595(void) {
    static const char *const JOIN[] = {"SRCLK", "RCLK", NULL};
    Bench *b = open_fx("74_595");
    int ok;

    set(b, "SRCLR", L1);
    set(b, "OE", L0);
    set(b, "SRCLK", L0);
    set(b, "RCLK", L0);
    step(b);
    shift_in(b, "SER", "SRCLK", 8, 0x2D);
    ok = pins_value(b, Q8, 8, 0) && is(b, "QH'", bit(0x2D, 0));
    pulse(b, "RCLK");
    ok &= pins_value(b, Q8, 8, reversed(0x2D, 8));
    set(b, "OE", L1);
    step(b);
    ok &= pins_all(b, Q8, 8, LZ) && is(b, "QH'", bit(0x2D, 0));
    set(b, "OE", L0);
    set(b, "SRCLR", L0);
    step(b);
    ok &= pins_value(b, Q8, 8, reversed(0x2D, 8)) && is(b, "QH'", L0);
    expect(ok, "13 74_595 shift and storage separate; OE high -> Z, QH' still driven; SRCLR clears "
               "the shift register only");
    bench_destroy(b);

    b = bench_create_joined(fx_part("74_595"), JOIN);
    set(b, "SRCLR", L1);
    set(b, "OE", L0);
    set(b, "SRCLK", L0);
    set(b, "SER", L1);
    step(b);
    pulse(b, "SRCLK");
    ok = pins_value(b, Q8, 8, 0x00);   /* storage took the stages before the shift */
    pulse(b, "SRCLK");
    expect(ok && pins_value(b, Q8, 8, 0x01), "14 74_595 SRCLK and RCLK tied: storage one clock behind");
    bench_destroy(b);
}

void check_tpic6b595(void) {
    Bench *b = open_fx("TPIC6B595");
    int ok;

    set(b, "SRCLR", L1);
    set(b, "G", L0);
    set(b, "SRCK", L0);
    set(b, "RCK", L0);
    step(b);
    shift_in(b, "SERIN", "SRCK", 8, 0x96);
    pulse(b, "RCK");
    ok = drains(b, reversed(0x96, 8));
    set(b, "G", L1);
    step(b);
    ok &= drains(b, 0);
    set(b, "G", L0);
    set(b, "SRCLR", L0);
    step(b);
    ok &= drains(b, reversed(0x96, 8)) && is(b, "SEROUT", L0);
    expect(ok, "19 TPIC6B595 DRAIN low exactly when G is low and the storage bit is 1; SRCLR clears "
               "the shift register, not the storage");
    bench_destroy(b);
}


/* ---- 4094 ------------------------------------------------------------------------ */

void check_4094(void) {
    static const char *const Q[] = {"Q1", "Q2", "Q3", "Q4", "Q5", "Q6", "Q7", "Q8"};
    Bench *b = open_fx("4094B");
    int ok;
    unsigned k;

    set(b, "OE", L1);
    set(b, "STROBE", L0);
    set(b, "CLOCK", L0);
    set(b, "DATA", L1);
    step(b);
    pulse(b, "CLOCK");
    ok = pins_value(b, Q, 8, 0);   /* STROBE low: storage holds */
    set(b, "STROBE", L1);
    step(b);
    ok &= pins_value(b, Q, 8, 0x01);   /* follows */
    set(b, "DATA", L0);
    step(b);
    for (k = 0; k < 6; k++) {
        pulse(b, "CLOCK");
    }
    ok &= pins_value(b, Q, 8, 0x40) && is(b, "QS", L0);   /* 7 edges: the 1 is in stage 7 */
    set(b, "CLOCK", L1);   /* 8th rising: stage 8 -> 1 on QS; Q'S waits for the fall */
    step(b);
    ok &= is(b, "QS", L1) && is(b, "Q'S", L0);
    set(b, "CLOCK", L0);
    step(b);
    ok &= is(b, "Q'S", L1);
    set(b, "OE", L0);
    step(b);
    expect(ok && pins_all(b, Q, 8, LZ) && is(b, "QS", L1),
           "21 4094B STROBE high: storage follows; OE low -> Z; QS on CLOCK rising, Q'S on CLOCK "
           "falling");
    bench_destroy(b);
}
const char *const Q4[] = {"Q0", "Q1", "Q2", "Q3"};

void real(const char *part, SimTime step, void (*check)(void)) {
    real_part = part;
    step_ps = step;
    check();
    real_part = NULL;
    step_ps = NS(50);
}

/* ---- 299 / 323 ------------------------------------------------------------------ */

const char *const IO299[] = {"A/QA", "B/QB", "C/QC", "D/QD", "E/QE", "F/QF", "G/QG",
                                    "H/QH"};

/* ---- TPIC ------------------------------------------------------------------------ */

const char *const DRAIN[] = {"DRAIN0", "DRAIN1", "DRAIN2", "DRAIN3",
                                    "DRAIN4", "DRAIN5", "DRAIN6", "DRAIN7"};

/* DRAINn: 0 where the bit of value is 1, else Z. */
int drains(Bench *b, unsigned value) {
    unsigned i;
    int ok = 1;

    for (i = 0; i < 8; i++) {
        ok &= is(b, DRAIN[i], (value >> i) & 1u ? L0 : LZ);
    }
    return ok;
}
