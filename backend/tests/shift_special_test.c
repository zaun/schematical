/*
 * shift_special_test: split from shift_test.c by plan-68-H; the shared helpers are
 * support/shift_bench.c.
 */

#include "shift_bench.h"

/* ---- plan-10-H: ls673 / ls674 ------------------------------------------------ */

static const char *const Y16[] = {"Y0", "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7",
                                  "Y8", "Y9", "Y10", "Y11", "Y12", "Y13", "Y14", "Y15"};
static const char *const P16[] = {"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7",
                                  "P8", "P9", "P10", "P11", "P12", "P13", "P14", "P15"};

/* Write mode: shifts value's bits in through SER/Q15, LSB first, on falling clock edges. */
static void write_16(Bench *b, const char *clock, unsigned value) {
    unsigned i;

    set(b, "R/W", L0);
    step(b);
    for (i = 0; i < 16; i++) {
        set(b, "SER/Q15", bit(value, i));
        set(b, clock, L1);
        step(b);
        set(b, clock, L0);
        step(b);
    }
    set(b, "SER/Q15", LZ);
    step(b);
}

/* Read mode, MODE 0: recirculates, reading Q15 before each falling edge; returns the 16 bits
   in the order they come out (first = bit 15 of Q). */
static unsigned read_16(Bench *b, const char *clock, const char *mode) {
    unsigned i, v = 0;

    set(b, "R/W", L1);
    set(b, mode, L0);
    step(b);
    for (i = 0; i < 16; i++) {
        v |= (bench_get(b, "SER/Q15") == L1 ? 1u : 0u) << i;
        set(b, clock, L1);
        step(b);
        set(b, clock, L0);
        step(b);
    }
    return v;
}

static void check_673(void) {
    Bench *b = open_fx("74_673");
    int ok;
    unsigned v;

    set(b, "CS", L0);
    set(b, "STRCLR", L1);
    set(b, "MODE/STRCLK", L0);
    set(b, "SHCLK", L0);
    step(b);
    write_16(b, "SHCLK", 0xA5C3);
    /* The first bit in went furthest: Q15 = bit 0, so reading gives the bits back in order. */
    v = read_16(b, "SHCLK", "MODE/STRCLK");
    ok = v == 0xA5C3;
    if (!ok) {
        printf("  read 0x%04X\n", v);
    }
    expect(ok, "24 74_673 write mode shifts the SER pin in on the gated clock's falling edge");

    /* 25: store clock with R/W 0 copies Q to Y; STRCLR clears Y with CS high. */
    set(b, "R/W", L0);
    set(b, "MODE/STRCLK", L0);
    step(b);
    set(b, "MODE/STRCLK", L1);   /* store clock rising: Y <- Q */
    step(b);
    set(b, "MODE/STRCLK", L0);
    step(b);
    {
        /* After write_16(0xA5C3) and a full recirculation, stage k holds bit 15-k of the value. */
        unsigned want = 0, i;

        for (i = 0; i < 16; i++) {
            want |= ((0xA5C3u >> i) & 1u) << (15 - i);
        }
        ok = pins_value(b, Y16, 16, want);
        set(b, "CS", L1);
        set(b, "STRCLR", L0);
        step(b);
        ok &= pins_value(b, Y16, 16, 0);
        set(b, "STRCLR", L1);
        set(b, "CS", L0);
        step(b);
        expect(ok, "25 74_673 the store clock (R/W 0) copies Q to Y; STRCLR clears Y with CS high");

        /* 26: store a value, then read mode MODE 1 loads Q from Y; MODE 0 recirculates. */
        set(b, "MODE/STRCLK", L1);
        step(b);
        set(b, "MODE/STRCLK", L0);
        step(b);
        ok = pins_value(b, Y16, 16, want);
        set(b, "R/W", L1);
        set(b, "MODE/STRCLK", L1);
        step(b);
        set(b, "SHCLK", L1);
        step(b);
        set(b, "SHCLK", L0);   /* parallel load Q <- Y */
        step(b);
        ok &= is(b, "SER/Q15", bit(want, 15));
        v = read_16(b, "SHCLK", "MODE/STRCLK");
        ok &= v == 0xA5C3;
        set(b, "CS", L1);
        step(b);
        ok &= is(b, "SER/Q15", LZ);
        set(b, "CS", L0);
        set(b, "R/W", L0);
        step(b);
        expect(ok && is(b, "SER/Q15", LZ),
               "26 74_673 read mode MODE 1 loads Q from Y; MODE 0 recirculates; SER/Q15 driven only "
               "in read with CS 0");
    }
    bench_destroy(b);

    /* 27: CS rising with the shift clock high is a falling gated-clock edge. */
    b = open_fx("74_673");
    set(b, "STRCLR", L1);
    set(b, "MODE/STRCLK", L0);
    set(b, "SHCLK", L0);
    set(b, "CS", L1);
    set(b, "R/W", L0);
    set(b, "SER/Q15", L0);   /* the gated clock leaving X at power-up is a falling edge */
    step(b);
    set(b, "SER/Q15", L1);
    step(b);
    set(b, "SHCLK", L1);
    step(b);
    set(b, "CS", L0);   /* gated clock rises: no shift */
    step(b);
    set(b, "CS", L1);   /* gated clock falls: a shift of the 1 into Q0 */
    step(b);
    set(b, "SHCLK", L0);
    set(b, "SER/Q15", LZ);
    set(b, "CS", L0);
    step(b);
    set(b, "MODE/STRCLK", L1);   /* store, to see Q on Y */
    step(b);
    expect(pins_value(b, Y16, 16, 0x0001),
           "27 74_673 CS rising with the clock high is a falling gated-clock edge (the datasheet's "
           "warning)");
    bench_destroy(b);
}

static void check_674(void) {
    Bench *b = open_fx("74_674");
    unsigned v, want = 0, i;

    set(b, "CS", L0);
    set(b, "CLK", L0);
    set_pins(b, P16, 16, 0x1234);
    set(b, "R/W", L1);
    set(b, "MODE", L1);
    step(b);
    set(b, "CLK", L1);
    step(b);
    set(b, "CLK", L0);   /* parallel load */
    step(b);
    v = read_16(b, "CLK", "MODE");
    for (i = 0; i < 16; i++) {
        want |= ((0x1234u >> (15 - i)) & 1u) << i;
    }
    if (v != want) {
        printf("  read 0x%04X, want 0x%04X\n", v, want);
    }
    expect(v == want, "28 74_674 MODE 1 in read mode loads P0-P15");
    bench_destroy(b);
}

/* ---- plan-10-H: 4517B, 4034B ------------------------------------------------------ */

static void check_4517(void) {
    Bench *b = open_fx("4517B");
    int ok = 1;
    unsigned k;

    set(b, "WEA", L0);
    set(b, "CLA", L0);
    set(b, "DA", L1);
    step(b);
    pulse(b, "CLA");
    set(b, "DA", L0);
    step(b);
    for (k = 2; k <= 64; k++) {
        pulse(b, "CLA");
        ok &= is(b, "Q16A", k == 16 ? L1 : L0) && is(b, "Q64A", k == 64 ? L1 : L0);
    }
    expect(ok, "29 4517B 64 clocks from D to Q64 (and 16 to Q16)");

    set(b, "WEA", L1);
    step(b);
    ok = is(b, "Q16A", LZ) && is(b, "Q32A", LZ) && is(b, "Q48A", LZ) && is(b, "Q64A", LZ);
    set(b, "Q16A", L1);   /* the tap-16 pin feeds stage 17 */
    step(b);
    pulse(b, "CLA");
    set(b, "Q16A", LZ);
    set(b, "WEA", L0);
    step(b);
    for (k = 18; k <= 32; k++) {
        pulse(b, "CLA");
    }
    expect(ok && is(b, "Q32A", L1), "30 4517B WE 1: the taps are Z; the tap-16 pin feeds stage 17");
    bench_destroy(b);
}

static const char *const A8[] = {"A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"};
static const char *const B8[] = {"B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"};

static void check_4034(void) {
    Bench *b = open_fx("4034B");
    int ok;

    set(b, "AE", L1);
    set(b, "A/B", L1);
    set(b, "P/S", L0);
    set(b, "A/S", L0);
    set(b, "CLOCK", L0);
    step(b);
    shift_in(b, "SI", "CLOCK", 8, 0x8D);
    expect(pins_value(b, B8, 8, reversed(0x8D, 8)) && pins_all(b, A8, 8, LZ),
           "31 4034B serial shift into the B outputs (A/B 1)");

    set_pins(b, A8, 8, 0x5A);
    set(b, "P/S", L1);
    step(b);
    ok = pins_value(b, B8, 8, reversed(0x8D, 8));   /* synchronous: no change yet */
    pulse(b, "CLOCK");
    ok &= pins_value(b, B8, 8, 0x5A);
    set(b, "A/S", L1);   /* asynchronous: follows A */
    set_pins(b, A8, 8, 0x3C);
    step(b);
    ok &= pins_value(b, B8, 8, 0x3C);
    set(b, "AE", L0);   /* recirculation: A ignored */
    set_pins(b, A8, 8, 0xFF);
    step(b);
    pulse(b, "CLOCK");
    expect(ok && pins_value(b, B8, 8, 0x3C),
           "32 4034B sync parallel load from A on the edge; async load follows A; AE 0 recirculates");

    release_pins(b, A8, 8);
    set(b, "A/S", L0);
    set(b, "A/B", L0);   /* B is the input, A the output */
    step(b);
    ok = pins_all(b, B8, 8, LZ) && pins_all(b, A8, 8, LZ);   /* AE 0: A off */
    set(b, "AE", L1);
    step(b);
    ok &= pins_value(b, A8, 8, 0x3C);
    set_pins(b, B8, 8, 0x81);
    step(b);
    pulse(b, "CLOCK");
    expect(ok && pins_value(b, A8, 8, 0x81) && bench_error(b) == NULL,
           "33 4034B A/B 0: B is the input, A driven only with AE 1");
    bench_destroy(b);
}

/* ---- plan-10-H: FIFO --------------------------------------------------------------- */

static const char *const D4[] = {"D0", "D1", "D2", "D3"};

/* The fixture's fall-through is 1500 ns (100 ns per position step). */
static Bench *open_fifo(const char *device) {
    Bench *b = open_fx(device);

    set(b, "OE", L0);
    set(b, "SI", L0);
    set(b, "SO", L1);
    set(b, "MR", L1);
    step(b);
    set(b, "MR", L0);
    step(b);
    return b;
}

static void fifo_write(Bench *b, unsigned value) {
    set_pins(b, D4, 4, value);
    set(b, "SI", L1);
    bench_run(b, NS(20));
    set(b, "SI", L0);
    bench_run(b, NS(20));
}

static void fifo_unload(Bench *b) {
    set(b, "SO", L0);
    bench_run(b, NS(20));
    set(b, "SO", L1);
    bench_run(b, NS(20));
}

static void check_fifo(void) {
    Bench *b = open_fifo("40105B");
    int ok;
    unsigned k;

    ok = is(b, "DIR", L1) && is(b, "DOR", L0);
    set_pins(b, D4, 4, 0x9);
    set(b, "SI", L1);
    bench_run(b, NS(1000));
    ok &= is(b, "DOR", L0);   /* still falling through */
    bench_run(b, NS(1000));
    expect(ok && is(b, "DOR", L1) && pins_value(b, Q4, 4, 0x9),
           "34 40105B one word into an empty FIFO: DOR high and Q valid after the fall-through time");
    set(b, "SI", L0);
    step(b);

    for (k = 1; k < 16; k++) {
        fifo_write(b, k);
        bench_run(b, NS(2000));
    }
    ok = is(b, "DIR", L0);
    fifo_write(b, 0xF);   /* the 17th: ignored */
    bench_run(b, NS(2000));
    expect(ok && is(b, "DIR", L0) && pins_value(b, Q4, 4, 0x9),
           "35 40105B 16 writes fill it: DIR stays low; a 17th SI is ignored");

    set(b, "SO", L0);
    bench_run(b, NS(20));
    ok = is(b, "DOR", L0);
    set(b, "SO", L1);
    bench_run(b, NS(300));
    expect(ok && is(b, "DOR", L1) && pins_value(b, Q4, 4, 0x1),
           "36 40105B SO falling unloads; DOR low, then high when the next word arrives");

    set(b, "MR", L1);
    step(b);
    expect(is(b, "DIR", L1) && is(b, "DOR", L0), "37 40105B MR empties it: DIR 1, DOR 0");
    set(b, "MR", L0);
    step(b);

    fifo_write(b, 0x6);
    bench_run(b, NS(300));
    fifo_write(b, 0x5);
    bench_run(b, NS(3000));
    set(b, "OE", L1);   /* outputs disabled: unloading inhibited */
    step(b);
    fifo_unload(b);
    bench_run(b, NS(300));
    ok = is(b, "DOR", L1) && pins_all(b, Q4, 4, LZ);
    set(b, "SO", L0);
    step(b);
    set(b, "OE", L0);   /* enabling with SO low unloads */
    bench_run(b, NS(20));
    ok &= is(b, "DOR", L0);
    bench_run(b, NS(300));
    expect(ok && is(b, "DOR", L1) && pins_value(b, Q4, 4, 0x5),
           "38 40105B output disable blocks unloading; enabling with SO low unloads");
    bench_destroy(b);

    b = open_fifo("74_40105");
    fifo_write(b, 0x3);
    bench_run(b, NS(3000));
    set(b, "OE", L1);
    step(b);
    fifo_unload(b);   /* HC: SO alone unloads, even while disabled */
    bench_run(b, NS(20));
    ok = is(b, "DOR", L0);
    for (k = 0; k < 16; k++) {
        fifo_write(b, k);
        bench_run(b, NS(2000));
    }
    ok &= is(b, "DIR", L0);   /* full */
    set_pins(b, D4, 4, 0xC);
    set(b, "SI", L1);   /* held high */
    step(b);
    set(b, "OE", L0);
    fifo_unload(b);   /* position 1 frees after the ripple; SI still high loads 0xC */
    bench_run(b, NS(3000));
    set(b, "SI", L0);
    step(b);
    for (k = 1; k < 16; k++) {
        fifo_unload(b);
        bench_run(b, NS(2000));
    }
    expect(ok && is(b, "DOR", L1) && pins_value(b, Q4, 4, 0xC),
           "39 74_40105 SO unloads while disabled; SI held high loads when position 1 frees");
    bench_destroy(b);
}

/* ---- plan-10-H: SN74LV8153 --------------------------------------------------------- */

static const char *const Y8[] = {"Y0", "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7"};

/* One frame: ST0 0, ST1 1, A0-A2, the nibble, stop 1; period in ps. Stops after `bits` bits
   (10 = the whole frame) and leaves D at the last bit. */
static void frame(Bench *b, unsigned address, unsigned nibble, SimTime period, unsigned bits) {
    unsigned levels[10];
    unsigned k;

    levels[0] = 0;
    levels[1] = 1;
    for (k = 0; k < 3; k++) {
        levels[2 + k] = (address >> k) & 1u;
    }
    for (k = 0; k < 4; k++) {
        levels[5 + k] = (nibble >> k) & 1u;
    }
    levels[9] = 1;
    for (k = 0; k < bits; k++) {
        set(b, "D", levels[k] ? L1 : L0);
        bench_run(b, period);
    }
}

static Bench *open_8153(unsigned address) {
    static const char *const A[] = {"A0", "A1", "A2"};
    Bench *b = open_fx("74_8153");

    set(b, "D", L1);
    set(b, "OUTSEL", L1);
    set(b, "OE", L0);
    set_pins(b, A, 3, address);
    set(b, "RESET", L0);
    step(b);
    set(b, "RESET", L1);
    bench_run(b, NS(1000));
    return b;
}

static void check_8153(void) {
    const SimTime p9600 = 1000000000000ull / 9600;
    Bench *b = open_8153(5);
    int ok;

    frame(b, 5, 0x6, p9600, 10);   /* D0-D3 = 6 */
    bench_run(b, p9600);
    frame(b, 5, 0xB, p9600, 8);    /* D4-D7 = 0xB; stop just before the D7 bit */
    set(b, "D", L1);   /* D7 = 1 (0xB bit 3) */
    bench_run(b, p9600 * 4 / 10);
    ok = pins_value(b, Y8, 8, 0x00) && is(b, "SOUT", L1);
    bench_run(b, p9600 * 2 / 10);   /* past the D7 sample point */
    ok &= pins_value(b, Y8, 8, 0xB6) && is(b, "SOUT", L0);
    bench_run(b, p9600 * 8 / 10);
    ok &= is(b, "SOUT", L0);
    bench_run(b, p9600 * 4 / 10);
    expect(ok && is(b, "SOUT", L1),
           "40 74_8153 two frames at 9600 bit/s, A = 5: Y = the byte at the D7 sample point; SOUT low "
           "for one bit");

    bench_run(b, p9600 * 2);
    frame(b, 3, 0xF, p9600, 10);   /* another device's frame: ignored */
    bench_run(b, p9600);
    frame(b, 5, 0x1, p9600, 10);   /* first */
    bench_run(b, p9600);
    frame(b, 3, 0x2, p9600, 10);   /* ignored (other address): not a second frame */
    bench_run(b, p9600);
    frame(b, 5, 0x2, p9600, 10);   /* second: completes 0x21 */
    bench_run(b, p9600);
    ok = pins_value(b, Y8, 8, 0x21);
    bench_destroy(b);
    /* A pair with mismatched addresses is dropped: pins A = 5 accept both ... */
    b = open_8153(5);
    frame(b, 5, 0x7, p9600, 10);
    bench_run(b, p9600);
    frame(b, 5, 0x0, p9600, 10);   /* 0x07 */
    bench_run(b, p9600);
    ok &= pins_value(b, Y8, 8, 0x07);
    bench_destroy(b);
    expect(ok, "41 74_8153 a frame with another address is ignored; the pair completes with the next "
               "matching frame");

    b = open_8153(0);
    frame(b, 0, 0x5, p9600, 10);
    bench_run(b, p9600);
    frame(b, 0, 0xA, p9600, 10);   /* 0xA5 */
    bench_run(b, p9600);
    set(b, "OUTSEL", L0);
    step(b);
    {
        unsigned i;

        ok = 1;
        for (i = 0; i < 8; i++) {
            ok &= is(b, Y8[i], (0xA5u >> i) & 1u ? L0 : LZ);
        }
    }
    set(b, "OUTSEL", L1);
    set(b, "OE", L1);
    step(b);
    ok &= pins_all(b, Y8, 8, LZ);
    set(b, "OE", L0);
    set(b, "RESET", L0);
    step(b);
    expect(ok && pins_value(b, Y8, 8, 0x00),
           "42 74_8153 OUTSEL 0: Y = 0 for 1 bits, Z for 0 bits; OE high -> Z; RESET low -> 0");
    bench_destroy(b);

    b = open_8153(2);
    frame(b, 2, 0xC, 1000000000000ull / 2400, 10);
    bench_run(b, NS(100000));
    frame(b, 2, 0x3, 1000000000000ull / 19200, 10);   /* 0x3C */
    bench_run(b, NS(100000));
    ok = pins_value(b, Y8, 8, 0x3C);
    expect(ok, "43 74_8153 the bit rate is measured per frame (a 2400 then a 19200 bit/s frame)");
    bench_destroy(b);
}

/* ---- plan-10-I: real parts ---------------------------------------------------------- */

/* Case 34 with a real part's fall-through: DOR low halfway, high after it. */
static void check_fifo_real(void) {
    Bench *b = open_fifo("74_40105");
    int ok = is(b, "DIR", L1) && is(b, "DOR", L0);

    set_pins(b, D4, 4, 0x9);
    set(b, "SI", L1);
    bench_run(b, NS(200));
    ok &= is(b, "DOR", L0);   /* the HC fall-through is 400 ns */
    bench_run(b, NS(500));
    expect(ok && is(b, "DOR", L1) && pins_value(b, Q4, 4, 0x9),
           "34 one word into an empty FIFO: DOR high and Q valid after the fall-through time");
    bench_destroy(b);
}

/* ---- High-voltage serial drivers (real parts only) --------------------------------- */

/* HVOUT1..n all at v, except HVOUT<one> (0 = none) at other. */
static int hvouts(Bench *b, unsigned n, LogicValue v, unsigned one, LogicValue other) {
    unsigned i;
    int ok = 1;

    for (i = 1; i <= n; i++) {
        char name[16];

        snprintf(name, sizeof name, "HVOUT%u", i);
        ok &= is(b, name, i == one ? other : v);
    }
    return ok;
}

/* Clocks one 1 and then n-1 zeros in on a falling edge: the 1 ends in stage n. */
static void walk_one(Bench *b, const char *data, const char *clock, unsigned n, int falling) {
    unsigned k;

    for (k = 0; k < n; k++) {
        set(b, data, k == 0 ? L1 : L0);
        set(b, clock, falling ? L1 : L0);
        step(b);
        set(b, clock, falling ? L0 : L1);
        step(b);
    }
    set(b, clock, L0);
    step(b);
}

static void check_hv5122(void) {
    Bench *b = open_fx("HV5122");
    int ok;

    set(b, "STR", L1);
    set(b, "OE", L1);
    set(b, "CLK", L0);
    set(b, "DATA IN", L0);
    step(b);
    ok = hvouts(b, 32, LZ, 0, LZ);
    walk_one(b, "DATA IN", "CLK", 3, 1);   /* the 1 is in stage 3 */
    ok &= hvouts(b, 32, LZ, 3, L0) && is(b, "DATA OUT", L0);
    set(b, "OE", L0);   /* all off */
    step(b);
    ok &= hvouts(b, 32, LZ, 0, LZ);
    set(b, "STR", L0);   /* all on, whatever OE says */
    step(b);
    ok &= hvouts(b, 32, L0, 0, L0);
    set(b, "STR", L1);
    set(b, "OE", L1);
    walk_one(b, "DATA IN", "CLK", 29, 1);   /* 29 zeros and a new 1 behind the old one */
    expect(ok && is(b, "DATA OUT", L1) && is(b, "HVOUT32", L0) && is(b, "HVOUT29", L0) &&
           is(b, "HVOUT28", LZ) && is(b, "HVOUT30", LZ),
           "40 HV5122 an output sinks for a 1 while OE is high; OE low turns all off, STR low all "
           "on; shifts on CLK falling, DATA OUT is stage 32");
    bench_destroy(b);
}

static void check_hv5622(void) {
    Bench *b = open_fx("HV5622");
    int ok;

    set(b, "LE", L0);
    set(b, "BL", L1);
    set(b, "POL", L1);
    set(b, "CLK", L0);
    set(b, "DATA IN", L0);
    step(b);
    walk_one(b, "DATA IN", "CLK", 5, 1);
    ok = hvouts(b, 32, LZ, 0, LZ);   /* LE low: the latches hold their zeros */
    set(b, "LE", L1);
    step(b);
    set(b, "LE", L0);
    step(b);
    ok &= hvouts(b, 32, LZ, 5, L0);
    walk_one(b, "DATA IN", "CLK", 2, 1);   /* the stages move on; the latches do not */
    ok &= hvouts(b, 32, LZ, 5, L0);
    set(b, "POL", L0);   /* inverted */
    step(b);
    ok &= hvouts(b, 32, L0, 5, LZ);
    set(b, "BL", L0);   /* blank with POL low: all on */
    step(b);
    ok &= hvouts(b, 32, L0, 0, L0);
    set(b, "POL", L1);   /* blank with POL high: all off */
    step(b);
    expect(ok && hvouts(b, 32, LZ, 0, LZ),
           "41 HV5622 latches follow the stages while LE is high; POL low inverts; BL low is all "
           "off (POL high) or all on (POL low)");
    bench_destroy(b);
}

static void check_hv5812(void) {
    Bench *b = open_fx("HV5812");
    int ok;

    set(b, "STROBE", L1);
    set(b, "BLANKING", L0);
    set(b, "CLOCK", L0);
    set(b, "DATA IN", L0);
    step(b);
    ok = hvouts(b, 20, L0, 0, L0);
    walk_one(b, "DATA IN", "CLOCK", 4, 0);   /* rising edges */
    ok &= hvouts(b, 20, L0, 4, L1);
    set(b, "STROBE", L0);
    walk_one(b, "DATA IN", "CLOCK", 2, 0);
    ok &= hvouts(b, 20, L0, 4, L1);   /* held */
    set(b, "BLANKING", L1);
    step(b);
    expect(ok && hvouts(b, 20, L0, 0, L0),
           "42 HV5812 push-pull outputs follow the latches (STROBE high: transparent); BLANKING "
           "high drives all low");
    bench_destroy(b);
}

static void check_max6921(void) {
    Bench *b = open_fx("MAX6921");
    int ok;
    unsigned k;

    set(b, "LOAD", L1);
    set(b, "BLANK", L0);
    set(b, "CLK", L0);
    set(b, "DIN", L1);
    step(b);
    pulse(b, "CLK");
    set(b, "DIN", L0);
    step(b);
    ok = is(b, "OUT0", L1) && is(b, "OUT1", L0);   /* the bit enters at OUT0 */
    for (k = 0; k < 19; k++) {
        pulse(b, "CLK");
    }
    ok &= is(b, "OUT19", L1) && is(b, "OUT0", L0) && is(b, "DOUT", L1);
    set(b, "BLANK", L1);
    step(b);
    expect(ok && is(b, "OUT19", L0) && is(b, "DOUT", L1),
           "43 MAX6921 the first bit in ends at OUT19 and DOUT; BLANK high drives the outputs low");
    bench_destroy(b);
}

static void check_real_parts(void) {
    real_library = part_library_create();
    if (real_library == NULL || load_parts_dir(real_library, PARTS_DIR) != 0) {
        printf("FAIL: cannot load the part library\n");
        failures++;
        return;
    }
    real("SN74HC595N", NS(100), check_595);
    real("SN74HC165N", NS(100), check_165);
    real("SN74LS299N", NS(100), check_299);
    real("TPIC6B595N", NS(500), check_tpic6b595);
    real("CD4094BE", NS(2000), check_4094);
    real("CD74HC40105E", NS(100), check_fifo_real);
    real("SN74LV8153PW", NS(300), check_8153);   /* RESET/OE to Y is 200 ns */
    real("SN74LS673DW", NS(100), check_673);
    real("HV5122PJ-G", NS(600), check_hv5122);
    real("HV5222PG-G", NS(600), check_hv5122);
    real("HV5622PG-G", NS(600), check_hv5622);
    real("HV5812P-G", NS(600), check_hv5812);
    real("MAX6921AWI+", NS(1500), check_max6921);
    real("MAX6921AUI+", NS(1500), check_max6921);
    part_library_destroy(real_library);
}

int main(void) {
    FIXTURE = read_file(GOLDEN_DIR "/../fixtures/shift/parts.json");
    FIXTURE_FOOTPRINTS = read_file(GOLDEN_DIR "/../fixtures/shift/footprints.json");
    if (FIXTURE == NULL || FIXTURE_FOOTPRINTS == NULL) {
        printf("FAIL: cannot read the fixture files\n");
        return 1;
    }
    char *error;

    library = fixture_library();
    error = load(library, FIXTURE, "shift_fixture.json");
    if (error != NULL) {
        printf("FAIL: fixture: %s\n", error);
        free(error);
        return 1;
    }
    check_673();
    check_674();
    check_4517();
    check_4034();
    check_fifo();
    check_8153();
    check_real_parts();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("shift_test: all passed\n");
    return 0;
}
