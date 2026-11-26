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

static int failures = 0;
static PartLibrary *library;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

/* Generated from the spec sections (plan-10-data/latch-a.md, latch-b.md). */
/* The fixture parts: tests/fixtures/latch/parts.json (moved out of C by plan-68-H). */
static char *FIXTURE;

/* Dual-row fixture footprints FX<n>, one per pin count above. */
/* Their footprints: tests/fixtures/latch/footprints.json. */
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

/* Loads text (single-quote style); returns the error (caller frees) or NULL. */
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

static const LogicValue L0 = LOGIC_0, L1 = LOGIC_1, LX = LOGIC_X, LZ = LOGIC_Z;

static Bench *open_fx(const char *device) {
    char name[64];
    const Part *part;

    snprintf(name, sizeof name, "FX-%s", device);
    part = part_library_find(library, name);
    if (part == NULL || part->kind != PART_LATCH) {
        fprintf(stderr, "no latch part %s\n", name);
        exit(1);
    }
    return bench_create(part);
}

static void step(Bench *b) {
    bench_run(b, NS(50));
}

static void set(Bench *b, const char *pin, LogicValue v) {
    bench_set(b, pin, v);
}

static LogicValue bit(unsigned value, unsigned i) {
    return (value >> i) & 1u ? LOGIC_1 : LOGIC_0;
}

/* Sets pins fmt % (i + base) for i < n from value's bits. */
static void set_bus(Bench *b, const char *fmt, unsigned base, unsigned n, unsigned value) {
    char name[32];
    unsigned i;

    for (i = 0; i < n; i++) {
        snprintf(name, sizeof name, fmt, i + base);
        set(b, name, bit(value, i));
    }
}

/* 1 when every pin fmt % (i + base) shows want(i) (values[i]). */
static int bus_is(Bench *b, const char *fmt, unsigned base, unsigned n, const LogicValue *want) {
    char name[32];
    unsigned i;
    int ok = 1;

    for (i = 0; i < n; i++) {
        LogicValue got;

        snprintf(name, sizeof name, fmt, i + base);
        got = bench_get(b, name);
        if (got != want[i]) {
            printf("  %s = %c, want %c\n", name, "01XZ"[got], "01XZ"[want[i]]);
            ok = 0;
        }
    }
    return ok;
}

static int bus_value(Bench *b, const char *fmt, unsigned base, unsigned n, unsigned value) {
    LogicValue want[64];
    unsigned i;

    for (i = 0; i < n; i++) {
        want[i] = bit(value, i);
    }
    return bus_is(b, fmt, base, n, want);
}

static int bus_all(Bench *b, const char *fmt, unsigned base, unsigned n, LogicValue v) {
    LogicValue want[64];
    unsigned i;

    for (i = 0; i < n; i++) {
        want[i] = v;
    }
    return bus_is(b, fmt, base, n, want);
}

static int is(Bench *b, const char *pin, LogicValue v) {
    LogicValue got = bench_get(b, pin);

    if (got != v) {
        printf("  %s = %c, want %c\n", pin, "01XZ"[got], "01XZ"[v]);
        return 0;
    }
    return 1;
}

/* ---- transparent ---------------------------------------------------------- */

static void check_373(void) {
    Bench *b = open_fx("74_373");
    int ok;

    set(b, "OE", L0);
    set(b, "LE", L0);
    set_bus(b, "%uD", 1, 8, 0xFF);
    step(b);
    expect(bus_value(b, "%uQ", 1, 8, 0x00), "1 74_373 power-on: all Q = 0 (LE low, D high)");

    set(b, "LE", L1);
    step(b);
    ok = bus_value(b, "%uQ", 1, 8, 0xFF);
    set_bus(b, "%uD", 1, 8, 0xA5);
    step(b);
    ok &= bus_value(b, "%uQ", 1, 8, 0xA5);
    set_bus(b, "%uD", 1, 8, 0x5A);
    step(b);
    expect(ok && bus_value(b, "%uQ", 1, 8, 0x5A), "2 74_373 LE high: Q follows D (each bit)");

    set(b, "LE", L0);
    step(b);
    set_bus(b, "%uD", 1, 8, 0x0F);
    step(b);
    expect(bus_value(b, "%uQ", 1, 8, 0x5A), "3 74_373 LE falls: Q holds while D changes");

    set(b, "OE", L1);
    step(b);
    ok = bus_all(b, "%uQ", 1, 8, LZ);
    set(b, "LE", L1);
    step(b);
    set(b, "LE", L0);
    step(b);
    ok &= bus_all(b, "%uQ", 1, 8, LZ);
    set(b, "OE", L0);
    step(b);
    expect(ok && bus_value(b, "%uQ", 1, 8, 0x0F),
           "4 74_373 OE high: Q = Z; LE high meanwhile loads; OE low shows the new data");

    /* stored 0x0F; D = 0x3C differs in bits 2, 3 (now 1 -> 0 ... ) and 4, 5. */
    set_bus(b, "%uD", 1, 8, 0x3C);
    step(b);
    set(b, "LE", LX);
    step(b);
    {
        LogicValue want[8] = {L1, L1, L1, L1, LX, LX, L0, L0};

        /* bits 0,1: stored 1, D 0 -> X; bits 2,3: 1 = 1; bits 4,5: 0 vs 1 -> X; 6,7: 0 = 0 */
        want[0] = LX;
        want[1] = LX;
        expect(bus_is(b, "%uQ", 1, 8, want),
               "5 74_373 LE = X: Q = X where D differs from the stored bit, unchanged where equal");
    }
    set(b, "LE", L0);
    set_bus(b, "%uD", 1, 8, 0x00);
    step(b);
    set(b, "LE", L1);
    step(b);
    set(b, "LE", L0);
    step(b);
    set(b, "OE", LX);
    step(b);
    ok = bus_all(b, "%uQ", 1, 8, LX);
    set(b, "OE", LZ);
    step(b);
    expect(ok && bus_all(b, "%uQ", 1, 8, LX), "6 74_373 OE = X -> Q = X; OE = Z -> Q = X");
    bench_destroy(b);
}

static void check_533(void) {
    Bench *b = open_fx("74_533");

    set(b, "OE", L0);
    set(b, "LE", L1);
    set_bus(b, "%uD", 1, 8, 0x3C);
    step(b);
    expect(bus_value(b, "%uQ", 1, 8, 0xC3), "7 74_533 outputs are NOT D");
    bench_destroy(b);
}

static void check_843(void) {
    Bench *b = open_fx("74_843");
    int ok;

    set(b, "OE", L0);
    set(b, "PRE", L1);
    set(b, "CLR", L1);
    set(b, "LE", L1);
    set_bus(b, "%uD", 1, 9, 0x1FF);
    step(b);
    ok = bus_value(b, "%uQ", 1, 9, 0x1FF);
    set(b, "CLR", L0);
    step(b);
    ok &= bus_value(b, "%uQ", 1, 9, 0x000);
    set(b, "LE", L0);
    step(b);
    ok &= bus_value(b, "%uQ", 1, 9, 0x000);
    set(b, "CLR", L1);
    step(b);
    ok &= bus_value(b, "%uQ", 1, 9, 0x000);
    set(b, "PRE", L0);
    step(b);
    ok &= bus_value(b, "%uQ", 1, 9, 0x1FF);
    set(b, "PRE", L1);
    set_bus(b, "%uD", 1, 9, 0x000);
    step(b);
    ok &= bus_value(b, "%uQ", 1, 9, 0x1FF);
    set(b, "PRE", L0);
    set(b, "CLR", L0);
    step(b);
    expect(ok && bus_value(b, "%uQ", 1, 9, 0x1FF),
           "8 74_843 CLR low -> 0 regardless of LE; PRE low -> 1; both low -> 1 (presetWins)");
    bench_destroy(b);
}

static void check_16373(void) {
    Bench *b = open_fx("74_16373");
    int ok;

    set(b, "1OE", L0);
    set(b, "2OE", L0);
    set(b, "1LE", L1);
    set(b, "2LE", L1);
    set_bus(b, "1D%u", 1, 8, 0x00);
    set_bus(b, "2D%u", 1, 8, 0xFF);
    step(b);
    set(b, "2LE", L0);
    step(b);
    set_bus(b, "1D%u", 1, 8, 0x96);
    set_bus(b, "2D%u", 1, 8, 0x00);
    step(b);
    ok = bus_value(b, "1Q%u", 1, 8, 0x96) && bus_value(b, "2Q%u", 1, 8, 0xFF);
    set(b, "1OE", L1);
    step(b);
    expect(ok && bus_all(b, "1Q%u", 1, 8, LZ) && bus_value(b, "2Q%u", 1, 8, 0xFF),
           "9 74_16373 bank 1 LE (and OE) do not move bank 2");
    bench_destroy(b);
}

static void check_75(void) {
    Bench *b = open_fx("74_75");
    int ok;

    set(b, "1C2C", L1);
    set(b, "3C4C", L1);
    set_bus(b, "%uD", 1, 4, 0x5);
    step(b);
    ok = bus_value(b, "%uQ", 1, 4, 0x5) && bus_value(b, "%uQBAR", 1, 4, 0xA);
    set(b, "3C4C", L0);
    step(b);
    set_bus(b, "%uD", 1, 4, 0xA);
    step(b);
    ok &= bus_value(b, "%uQ", 1, 4, 0x6) && bus_value(b, "%uQBAR", 1, 4, 0x9);
    set(b, "1C2C", L0);
    set(b, "3C4C", L1);
    step(b);
    set_bus(b, "%uD", 1, 4, 0x0);
    step(b);
    expect(ok && bus_value(b, "%uQ", 1, 4, 0x2) && bus_value(b, "%uQBAR", 1, 4, 0xD),
           "10 74_75 Q and QBAR complementary; pair enables independent");
    bench_destroy(b);
}

static void check_4042(void) {
    Bench *b = open_fx("4042B");
    int ok;

    set(b, "POLARITY", L0);
    set(b, "CLOCK", L0);
    set_bus(b, "D%u", 1, 4, 0x9);
    step(b);
    ok = bus_value(b, "Q%u", 1, 4, 0x9) && bus_value(b, "Q%uBAR", 1, 4, 0x6);
    set(b, "CLOCK", L1);
    step(b);
    set_bus(b, "D%u", 1, 4, 0x6);
    step(b);
    ok &= bus_value(b, "Q%u", 1, 4, 0x9);
    set(b, "POLARITY", L1);   /* CLOCK steady at 1: the latch opens */
    step(b);
    ok &= bus_value(b, "Q%u", 1, 4, 0x6);
    set(b, "CLOCK", L0);
    step(b);
    set_bus(b, "D%u", 1, 4, 0xF);
    step(b);
    ok &= bus_value(b, "Q%u", 1, 4, 0x6);
    set(b, "CLOCK", L1);
    step(b);
    ok &= bus_value(b, "Q%u", 1, 4, 0xF);
    set(b, "POLARITY", L0);   /* CLOCK steady at 1: the latch closes */
    step(b);
    set_bus(b, "D%u", 1, 4, 0x0);
    step(b);
    expect(ok && bus_value(b, "Q%u", 1, 4, 0xF),
           "11 4042B POLARITY 0: transparent at CLOCK 0; POLARITY 1 at CLOCK 1; "
           "POLARITY change with CLOCK steady opens/closes");
    bench_destroy(b);
}

static void check_4508(void) {
    Bench *b = open_fx("4508B");
    int ok;

    set(b, "ODA", L0);
    set(b, "ODB", L0);
    set(b, "RESETA", L0);
    set(b, "RESETB", L0);
    set(b, "STROBEA", L1);
    set(b, "STROBEB", L1);
    set_bus(b, "D%uA", 0, 4, 0xF);
    set_bus(b, "D%uB", 0, 4, 0xF);
    step(b);
    set(b, "STROBEA", L0);
    set(b, "STROBEB", L0);
    step(b);
    set(b, "RESETA", L1);
    step(b);
    ok = bus_value(b, "Q%uA", 0, 4, 0x0) && bus_value(b, "Q%uB", 0, 4, 0xF);
    set(b, "RESETA", L0);
    step(b);
    ok &= bus_value(b, "Q%uA", 0, 4, 0x0);   /* the stored bits were cleared */
    set(b, "ODB", L1);
    step(b);
    ok &= bus_all(b, "Q%uB", 0, 4, LZ);
    set(b, "ODB", L0);
    step(b);
    expect(ok && bus_value(b, "Q%uB", 0, 4, 0xF),
           "12 4508B RESET high -> 0 and clears the stored bit; DISABLE high -> Z, storage intact");
    bench_destroy(b);
}

/* ---- addressable ---------------------------------------------------------- */

static void address(Bench *b, const char *fmt, unsigned a) {
    set_bus(b, fmt, 0, 3, a);
}

static void check_259(void) {
    Bench *b = open_fx("74_259");
    int ok = 1;
    unsigned i;

    set(b, "CLR", L1);
    set(b, "G", L1);
    set(b, "D", L0);
    address(b, "S%u", 0);
    step(b);
    for (i = 0; i < 8; i += 2) {   /* write 1 to latches 0, 2, 4, 6 */
        address(b, "S%u", i);
        step(b);
        set(b, "D", L1);
        set(b, "G", L0);
        step(b);
        set(b, "G", L1);
        step(b);
        set(b, "D", L0);
    }
    ok = bus_value(b, "Q%u", 0, 8, 0x55);
    address(b, "S%u", 3);
    set(b, "G", L0);
    set(b, "D", L1);
    step(b);
    ok &= bus_value(b, "Q%u", 0, 8, 0x5D);
    set(b, "D", L0);
    step(b);
    expect(ok && bus_value(b, "Q%u", 0, 8, 0x55),
           "13 74_259 addressable mode: only the addressed latch follows D");

    set(b, "G", L1);
    step(b);
    set(b, "D", L1);
    address(b, "S%u", 7);
    step(b);
    expect(bus_value(b, "Q%u", 0, 8, 0x55), "14 74_259 memory mode: G high holds all");

    set(b, "G", L0);
    set(b, "CLR", L0);
    step(b);
    ok = bus_value(b, "Q%u", 0, 8, 0x80);
    set(b, "D", L0);
    step(b);
    expect(ok && bus_value(b, "Q%u", 0, 8, 0x00),
           "15 74_259 demux mode: CLR low, G low -> addressed = D, others 0");

    set(b, "CLR", L1);
    set(b, "D", L1);
    step(b);
    set(b, "G", L1);
    step(b);
    ok = bus_value(b, "Q%u", 0, 8, 0x80);
    set(b, "CLR", L0);
    step(b);
    expect(ok && bus_value(b, "Q%u", 0, 8, 0x00), "16 74_259 clear mode: CLR low, G high -> all 0");

    set(b, "CLR", L1);
    address(b, "S%u", 1);
    set(b, "D", L1);
    step(b);
    set(b, "G", L0);
    step(b);
    address(b, "S%u", 5);   /* G low: the transparent latch moves */
    step(b);
    ok = bus_value(b, "Q%u", 0, 8, 0x22);
    set(b, "D", L0);
    step(b);
    expect(ok && bus_value(b, "Q%u", 0, 8, 0x02),
           "17 74_259 address change with G low moves the transparent latch; the old one keeps "
           "its value");
    bench_destroy(b);
}

static void check_4099(void) {
    Bench *b = open_fx("4099B");
    int ok;

    set(b, "RESET", L0);
    set(b, "WD", L1);
    address(b, "A%u", 2);
    set(b, "DATA", L1);
    step(b);
    set(b, "WD", L0);
    step(b);
    set(b, "WD", L1);
    address(b, "A%u", 6);
    step(b);
    set(b, "WD", L0);
    step(b);
    set(b, "WD", L1);
    step(b);
    ok = bus_value(b, "Q%u", 0, 8, 0x44);
    set(b, "RESET", L1);
    step(b);
    ok &= bus_value(b, "Q%u", 0, 8, 0x00);
    address(b, "A%u", 1);
    set(b, "WD", L0);
    step(b);
    ok &= bus_value(b, "Q%u", 0, 8, 0x02);
    address(b, "A%u", 3);
    step(b);
    expect(ok && bus_value(b, "Q%u", 0, 8, 0x08),
           "18 4099B RESET high + WD high clears all; RESET high + WD low = demux");
    bench_destroy(b);
}

static void check_tpic6259(void) {
    Bench *b = open_fx("TPIC6259");
    LogicValue want[8] = {LZ, L0, LZ, LZ, LZ, LZ, LZ, L0};

    set(b, "CLR", L1);
    set(b, "G", L1);
    set(b, "D", L1);
    address(b, "S%u", 1);
    step(b);
    set(b, "G", L0);
    step(b);
    set(b, "G", L1);
    address(b, "S%u", 7);
    step(b);
    set(b, "G", L0);
    step(b);
    set(b, "G", L1);
    step(b);
    expect(bus_is(b, "DRAIN%u", 0, 8, want),
           "19 TPIC6259 bit 1 -> DRAIN 0; bit 0 -> DRAIN Z (open drain, inverted)");
    bench_destroy(b);
}

/* ---- sr ------------------------------------------------------------------- */

static void check_279(void) {
    Bench *b = open_fx("74_279");
    int ok;

    set(b, "1S1", L1);
    set(b, "1S2", L1);
    set(b, "1R", L1);
    step(b);
    ok = is(b, "1Q", L0);
    set(b, "1S2", L0);   /* set through the second S */
    step(b);
    ok &= is(b, "1Q", L1);
    set(b, "1S2", L1);
    step(b);
    ok &= is(b, "1Q", L1);
    set(b, "1R", L0);   /* reset */
    step(b);
    ok &= is(b, "1Q", L0);
    set(b, "1S1", L0);   /* both low -> 1 */
    step(b);
    ok &= is(b, "1Q", L1);
    set(b, "1S1", L1);   /* release S first -> 0 */
    step(b);
    ok &= is(b, "1Q", L0);
    set(b, "1S1", L0);
    step(b);
    set(b, "1R", L1);   /* release R first -> 1 */
    step(b);
    ok &= is(b, "1Q", L1);
    set(b, "1S1", L1);
    step(b);
    expect(ok && is(b, "1Q", L1),
           "20 74_279 set, reset, both low -> 1, release S first -> 0, release R first -> 1");
    bench_destroy(b);
}

static void check_4043(void) {
    Bench *b = open_fx("4043B");
    int ok;

    set(b, "ENABLE", L1);
    set(b, "S1", L1);
    set(b, "R1", L1);
    set(b, "S2", L0);
    set(b, "R2", L0);
    step(b);
    ok = is(b, "Q1", L1) && is(b, "Q2", L0);
    set(b, "ENABLE", L0);
    set(b, "S1", L0);
    step(b);   /* R1 alone now resets latch 1 while the outputs are off */
    ok &= is(b, "Q1", LZ) && is(b, "Q2", LZ);
    set(b, "R1", L0);
    set(b, "S2", L1);
    step(b);
    set(b, "S2", L0);
    step(b);
    set(b, "ENABLE", L1);
    step(b);
    expect(ok && is(b, "Q1", L0) && is(b, "Q2", L1),
           "21 4043B S=R=1 -> 1; ENABLE 0 -> Z while S/R still act");
    bench_destroy(b);
}

static void check_4044(void) {
    Bench *b = open_fx("4044B");
    int ok;

    set(b, "ENABLE", L1);
    set(b, "S1", L0);
    set(b, "R1", L1);
    step(b);
    ok = is(b, "Q1", L1);
    set(b, "R1", L0);
    step(b);
    expect(ok && is(b, "Q1", L0), "22 4044B S=R=0 -> 0");
    bench_destroy(b);
}

/* ---- bcd7seg -------------------------------------------------------------- */

static const char *const SEGMENTS = "abcdefg";

static int segments_are(Bench *b, const char *glyph) {
    char name[2] = {0, 0};
    unsigned j;
    int ok = 1;

    for (j = 0; j < 7; j++) {
        name[0] = SEGMENTS[j];
        ok &= is(b, name, glyph[j] == '1' ? L1 : L0);
    }
    return ok;
}

static void check_4511(void) {
    static const char *const GLYPHS[16] = {
        "1111110", "0110000", "1101101", "1111001", "0110011", "1011011", "0011111", "1110000",
        "1111111", "1110011", "0000000", "0000000", "0000000", "0000000", "0000000", "0000000"
    };
    Bench *b = open_fx("74_4511");
    unsigned code;
    int ok = 1;

    set(b, "LE", L0);
    set(b, "LT", L1);
    set(b, "BL", L1);
    for (code = 0; code < 16; code++) {
        set_bus(b, "D%u", 0, 4, code);
        step(b);
        if (!segments_are(b, GLYPHS[code])) {
            printf("  code %u\n", code);
            ok = 0;
        }
    }
    set_bus(b, "D%u", 0, 4, 2);
    set(b, "LT", L0);
    step(b);
    ok &= segments_are(b, "1111111");
    set(b, "LT", L1);
    set(b, "BL", L0);
    step(b);
    ok &= segments_are(b, "0000000");
    set(b, "LT", L0);
    step(b);
    expect(ok && segments_are(b, "1111111"),
           "23 74_4511 codes 0-15 -> the glyph table; LT low -> all 1; BL low -> all 0; LT beats BL");

    set(b, "LT", L1);
    set(b, "BL", L1);
    set_bus(b, "D%u", 0, 4, 7);
    step(b);
    set(b, "LE", L1);
    step(b);
    set_bus(b, "D%u", 0, 4, 3);
    step(b);
    ok = segments_are(b, GLYPHS[7]);
    set(b, "LE", L0);
    step(b);
    expect(ok && segments_are(b, GLYPHS[3]), "24 74_4511 LE high holds the code while inputs change");
    bench_destroy(b);
}

/* ---- bus-mux -------------------------------------------------------------- */

static void check_16260(void) {
    Bench *b = open_fx("74_16260");
    int ok;

    set(b, "OEA", L1);
    set(b, "OE1B", L1);
    set(b, "OE2B", L1);
    set(b, "LE1B", L0);
    set(b, "LE2B", L0);
    set(b, "LEA2B", L0);
    set(b, "SEL", L0);
    set(b, "LEA1B", L1);
    set_bus(b, "A%u", 1, 12, 0xA5C);
    step(b);
    set(b, "LEA1B", L0);
    step(b);
    set_bus(b, "A%u", 1, 12, 0x000);
    step(b);
    ok = bus_all(b, "1B%u", 1, 12, LZ);
    set(b, "OE1B", L0);
    step(b);
    expect(ok && bus_value(b, "1B%u", 1, 12, 0xA5C) && bus_all(b, "2B%u", 1, 12, LZ),
           "25 74_16260 A->B: LEA1B high loads the 1B latch from A; OE1B low drives 1B");

    /* B -> A: release A and the B outputs, load 1B and 2B latches from the B pins. */
    set(b, "OE1B", L1);
    step(b);   /* the part releases 1B before the bench drives it */
    {
        char name[8];
        unsigned i;

        for (i = 1; i <= 12; i++) {
            snprintf(name, sizeof name, "A%u", i);
            set(b, name, LZ);
        }
    }
    set_bus(b, "1B%u", 1, 12, 0x123);
    set_bus(b, "2B%u", 1, 12, 0xFED);
    set(b, "LE1B", L1);
    set(b, "LE2B", L1);
    step(b);
    set(b, "LE1B", L0);
    set(b, "LE2B", L0);
    step(b);
    set(b, "OEA", L0);
    set(b, "SEL", L1);
    step(b);
    ok = bus_value(b, "A%u", 1, 12, 0x123);
    set(b, "SEL", L0);
    step(b);
    expect(ok && bus_value(b, "A%u", 1, 12, 0xFED) && bench_error(b) == NULL,
           "26 74_16260 B->A: SEL selects the 1B / 2B latch onto A when OEA low");
    bench_destroy(b);
}

/* ---- Real parts (plan-10-C section 4.3) ----------------------------------- */

static PartLibrary *real_library;

static Bench *open_real(const char *name) {
    const Part *part = part_library_find(real_library, name);

    if (part == NULL || part->kind != PART_LATCH) {
        fprintf(stderr, "no latch part %s\n", name);
        exit(1);
    }
    return bench_create(part);
}

/* Real parts have real delays (up to hundreds of ns): 1 us per step. */
static void slow(Bench *b) {
    bench_run(b, NS(1000));
}

static void check_real(void) {
    Bench *b;
    int ok;

    b = open_real("SN74HC373N");
    set(b, "OE", L0);
    set(b, "LE", L1);
    set_bus(b, "%uD", 1, 8, 0xA5);
    slow(b);
    ok = bus_value(b, "%uQ", 1, 8, 0xA5);
    set(b, "LE", L0);
    slow(b);
    set_bus(b, "%uD", 1, 8, 0x0F);
    slow(b);
    ok &= bus_value(b, "%uQ", 1, 8, 0xA5);
    set(b, "OE", L1);
    set(b, "LE", L1);
    slow(b);
    ok &= bus_all(b, "%uQ", 1, 8, LZ);
    set(b, "LE", L0);
    set(b, "OE", L0);
    slow(b);
    expect(ok && bus_value(b, "%uQ", 1, 8, 0x0F),
           "29 SN74HC373N: LE high follows D, LE low holds, OE high Z while loading");
    bench_destroy(b);

    b = open_real("SN74LVC1G373DBVR");
    set(b, "OE", L0);
    set(b, "LE", L1);
    set(b, "D", L1);
    slow(b);
    ok = is(b, "Q", L1);
    set(b, "LE", L0);
    slow(b);
    set(b, "D", L0);
    slow(b);
    expect(ok && is(b, "Q", L1), "30 SN74LVC1G373DBVR: transparent, then holds");
    bench_destroy(b);

    b = open_real("SN74HC259N");
    set(b, "CLR", L0);
    set(b, "G", L0);
    set(b, "D", L1);
    address(b, "S%u", 6);
    slow(b);
    expect(bus_value(b, "Q%u", 0, 8, 0x40), "31 SN74HC259N demux mode: addressed = D, others 0");
    bench_destroy(b);

    b = open_real("TPIC6B259N");
    {
        LogicValue want[8] = {LZ, LZ, LZ, L0, LZ, LZ, LZ, LZ};

        set(b, "CLR", L1);
        set(b, "G", L1);
        set(b, "D", L1);
        address(b, "S%u", 3);
        slow(b);
        set(b, "G", L0);
        slow(b);
        set(b, "G", L1);
        slow(b);
        expect(bus_is(b, "DRAIN%u", 0, 8, want),
               "32 TPIC6B259N: stored 1 pulls DRAIN low, stored 0 releases it");
    }
    bench_destroy(b);

    b = open_real("CD74HC4511E");
    set(b, "LE", L0);
    set(b, "LT", L1);
    set(b, "BL", L1);
    set_bus(b, "D%u", 0, 4, 7);
    slow(b);
    expect(segments_are(b, "1110000"), "33 CD74HC4511E: code 7 -> segments 1110000");
    bench_destroy(b);

    b = open_real("SN74ALVCH16260DL");
    set(b, "OEA", L1);
    set(b, "OE1B", L1);
    set(b, "OE2B", L1);
    set(b, "LEA1B", L0);
    set(b, "LEA2B", L0);
    set(b, "LE1B", L1);
    set(b, "LE2B", L1);
    set_bus(b, "1B%u", 1, 12, 0x5A3);
    set_bus(b, "2B%u", 1, 12, 0x0FF);
    slow(b);
    set(b, "LE1B", L0);
    set(b, "LE2B", L0);
    slow(b);
    set(b, "SEL", L1);
    set(b, "OEA", L0);
    slow(b);
    ok = bus_value(b, "A%u", 1, 12, 0x5A3);
    set(b, "SEL", L0);
    slow(b);
    expect(ok && bus_value(b, "A%u", 1, 12, 0x0FF) && bench_error(b) == NULL,
           "34 SN74ALVCH16260DL: SEL selects the 1B latch onto A (and the 2B latch when low)");
    bench_destroy(b);

    b = open_real("CD4511BE");
    set(b, "LE", L0);
    set(b, "LT", L1);
    set(b, "BL", L1);
    set(b, "A", L0);
    set(b, "B", L1);
    set(b, "C", L1);
    set(b, "D", L0);
    slow(b);
    ok = segments_are(b, "0011111");
    set(b, "A", L1);
    set(b, "D", L1);
    set(b, "B", L0);
    set(b, "C", L0);
    slow(b);
    ok &= segments_are(b, "1110011");
    set(b, "B", L1);
    slow(b);
    expect(ok && segments_are(b, "0000000"), "35 CD4511BE: 6 has no top segment, 9 no bottom, code 11 blank");
    bench_destroy(b);

    /* CD4054B: OUT = latched IN XOR DF while the strobe is low it holds */
    b = open_real("CD4054BE");
    set(b, "DF", L0);
    set(b, "STROBE1", L1);
    set(b, "IN1", L1);
    set(b, "STROBE2", L1);
    set(b, "IN2", L0);
    slow(b);
    ok = is(b, "OUT1", L1) && is(b, "OUT2", L0);
    set(b, "DF", L1);
    slow(b);
    ok &= is(b, "OUT1", L0) && is(b, "OUT2", L1);
    set(b, "STROBE1", L0);
    set(b, "IN1", L0);
    slow(b);
    ok &= is(b, "OUT1", L0);
    set(b, "DF", L0);
    slow(b);
    expect(ok && is(b, "OUT1", L1) && is(b, "OUT2", L0),
           "36 CD4054BE: DF inverts the held or passed bit; a low strobe holds IN");
    bench_destroy(b);
}

/* ---- Loader and voltage --------------------------------------------------- */

static void expect_error(const char *what, const char *text, const char *want) {
    PartLibrary *lib = fixture_library();
    char *error = load(lib, text, "bad.json");
    int ok = error != NULL && strstr(error, want) != NULL;

    if (!ok) {
        printf("  error: %s\n  want: %s\n", error != NULL ? error : "(none)", want);
    }
    if (!ok) {
        printf("  %s\n", what);
        failures++;
    }
    free(error);
    part_library_destroy(lib);
}

#define BAD_HEAD \
    "{'format':'schematical-parts','version':1,'kind':'latch','devices':[{'id':'bad'," \
    "'function':'Latch','description':'d','drawer':[],'pinouts':{'p4':[" \
    "{'number':1,'name':'D','role':'input'},{'number':2,'name':'LE','role':'input'}," \
    "{'number':3,'name':'Q','role':'output'},{'number':4,'name':'POL','role':'input'}]},"
#define BAD_TAIL "'timing':{'HC':{'rise':1,'fall':1,'vcc':[2,6]}},'parts':[]}]}"

static void check_loader(void) {
    int before = failures;
    int ok;

    expect_error("27a loader: unknown model", BAD_HEAD "'model':'nope'," BAD_TAIL,
                 "unknown model \"nope\"");
    expect_error("27b loader: a key of another model",
                 BAD_HEAD "'model':'transparent','units':[{'d':'D','le':'LE','q':'Q'}],"
                 "'enableActive':'high','address':['D']," BAD_TAIL,
                 "\"address\" is not a key of model \"transparent\"");
    expect_error("27c loader: enableActive with polarity",
                 BAD_HEAD "'model':'transparent','units':[{'d':'D','le':'LE','q':'Q'}],"
                 "'enableActive':'high','polarity':'POL'," BAD_TAIL,
                 "\"enableActive\" and \"polarity\" cannot both be given");
    ok = failures == before;
    failures = before;
    expect(ok,
           "27 loader: unknown model, another model's key, enableActive with polarity are errors");
}

static size_t pin_index(const Part *part, const char *name) {
    size_t i;

    for (i = 0; i < part->pin_count; i++) {
        if (strcmp(part->pins[i].name, name) == 0) {
            return i;
        }
    }
    return SIZE_MAX;
}

static void check_open_drain(void) {
    const Part *tpic = part_library_find(library, "FX-TPIC6259");
    const Part *ls = part_library_find(library, "FX-74_259");

    expect(tpic != NULL && ls != NULL && part_pin_open_drain(tpic, pin_index(tpic, "DRAIN0")) &&
               !part_pin_open_drain(ls, pin_index(ls, "Q0")) &&
               !part_pin_open_drain(tpic, pin_index(tpic, "D")),
           "28 voltage: FX-TPIC6259 DRAIN0 is open drain; FX-74_259 Q0 is not");
}

int main(void) {
    FIXTURE = read_file(GOLDEN_DIR "/../fixtures/latch/parts.json");
    FIXTURE_FOOTPRINTS = read_file(GOLDEN_DIR "/../fixtures/latch/footprints.json");
    if (FIXTURE == NULL || FIXTURE_FOOTPRINTS == NULL) {
        printf("FAIL: cannot read the fixture files\n");
        return 1;
    }
    char *error;

    library = fixture_library();
    error = load(library, FIXTURE, "latch_fixture.json");
    if (error != NULL) {
        printf("FAIL: fixture: %s\n", error);
        free(error);
        return 1;
    }
    check_373();
    check_533();
    check_843();
    check_16373();
    check_75();
    check_4042();
    check_4508();
    check_259();
    check_4099();
    check_tpic6259();
    check_279();
    check_4043();
    check_4044();
    check_4511();
    check_16260();
    check_loader();
    check_open_drain();
    real_library = part_library_create();
    if (real_library == NULL || load_parts_dir(real_library, PARTS_DIR) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    check_real();
    part_library_destroy(real_library);
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("latch_test: all passed\n");
    return 0;
}
