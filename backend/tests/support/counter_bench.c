/* counter_bench: shared by the programs split from counter_test.c (plan-68-H). */
#include "counter_bench.h"


int failures = 0;
PartLibrary *library;

/* Real-part runs (plan-10-F section 4.3): the cases open this part from the
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
/* The fixture parts: tests/fixtures/counter/parts.json (moved out of C by plan-68-H). */
char *FIXTURE;

/* Dual-row fixture footprints FX<n>, one per pin count above. */
/* Their footprints: tests/fixtures/counter/footprints.json. */
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
    if (part == NULL || part->kind != PART_COUNTER) {
        fprintf(stderr, "no counter part %s\n", name);
        exit(1);
    }
    return part;
}

Bench *open_fx(const char *device) {
    return bench_create(fx_part(device));
}

Bench *open_joined(const char *device, const char *const *joins) {
    return bench_create_joined(fx_part(device), joins);
}

void step(Bench *b) {
    bench_run(b, step_ps);
}

void set(Bench *b, const char *pin, LogicValue v) {
    bench_set(b, pin, v);
}

/* A full clock pulse on pin: up, step, down, step. */
void pulse(Bench *b, const char *pin) {
    set(b, pin, L1);
    step(b);
    set(b, pin, L0);
    step(b);
}

/* A rising edge only (the pin was low). */
void rise(Bench *b, const char *pin) {
    set(b, pin, L1);
    step(b);
}

void fall(Bench *b, const char *pin) {
    set(b, pin, L0);
    step(b);
}

LogicValue bit(unsigned value, unsigned i) {
    return (value >> i) & 1u ? LOGIC_1 : LOGIC_0;
}

/* Drives the named pins from value's bits (names[0] = bit 0). */
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
            printf("  %s = %c, want %c (value %u)\n", names[i], "01XZ"[got],
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

const char *const QAD[] = {"QA", "QB", "QC", "QD"};
const char *const ABCD[] = {"A", "B", "C", "D"};
const char *const Q8[] = {"QA", "QB", "QC", "QD", "QE", "QF", "QG", "QH"};
const char *const D8[] = {"A", "B", "C", "D", "E", "F", "G", "H"};
const char *const Q14[] = {"Q1", "Q2", "Q3", "Q4"};

/* ---- 74_161 / 163 / 169 -------------------------------------------------- */

void check_161(void) {
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

void check_dual_clock(const char *device, const char *up, const char *down,
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

/* ---- 74_590 / 592 / 593 / 697 / 8154 ------------------------------------- */

void check_590(void) {
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

void check_8154(void) {
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

const char *const J8[] = {"J0", "J1", "J2", "J3", "J4", "J5", "J6", "J7"};

int expect_error(const char *text, const char *want) {
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

void real(const char *part, SimTime step, void (*check)(void)) {
    real_part = part;
    step_ps = step;
    check();
    real_part = NULL;
    step_ps = NS(50);
}

const char *const Y8[] = {"Y0", "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7"};

void selects(Bench *b, int which) {
    static const char *const G[] = {"GAL", "GAU", "GBL", "GBU"};
    int i;

    for (i = 0; i < 4; i++) {
        set(b, G[i], i == which ? L0 : L1);
    }
    step(b);
}
