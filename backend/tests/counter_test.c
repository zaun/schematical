/*
 * counter_test: split from counter_test.c by plan-68-H; the shared helpers are
 * support/counter_bench.c.
 */

#include "counter_bench.h"


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

/* ---- Loader ----------------------------------------------------------------- */

#define BAD_HEAD \
    "{'format':'schematical-parts','version':1,'kind':'counter','devices':[{'id':'bad'," \
    "'function':'Counter','description':'d','drawer':[],'pinouts':{'p4':[" \
    "{'number':1,'name':'CLK','role':'input'},{'number':2,'name':'S','role':'input'}," \
    "{'number':3,'name':'Q','role':'output'},{'number':4,'name':'VCC','role':'power'}]}," \
    "'model':'sync','units':[{"
#define BAD_TAIL "'clock':{'pin':'CLK','edge':'rising'},'count':'up'}]," \
    "'timing':{'HC':{'rise':1,'fall':1,'vcc':[2,6]}},'parts':[]}]}"

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
    check_loader();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("counter_test: all passed\n");
    return 0;
}
