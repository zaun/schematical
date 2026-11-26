/*
 * shift_test: split from shift_test.c by plan-68-H; the shared helpers are
 * support/shift_bench.c.
 */

#include "shift_bench.h"

/* ---- 164 / 264 / 165 / 166 ------------------------------------------------- */

static void check_164(void) {
    Bench *b = open_fx("74_164");
    int ok;

    set(b, "CLR", L1);
    set(b, "B", L1);
    set(b, "CLK", L0);
    step(b);
    shift_in(b, "A", "CLK", 8, 0xB5);
    ok = pins_value(b, Q8, 8, reversed(0xB5, 8));
    set(b, "A", L1);
    set(b, "B", L0);
    step(b);
    pulse(b, "CLK");   /* A AND B = 0 */
    ok &= pins_value(b, Q8, 8, (reversed(0xB5, 8) << 1) & 0xFF);
    set(b, "CLR", L0);
    step(b);
    expect(ok && pins_value(b, Q8, 8, 0), "1 74_164 shifts A AND B in on CLK rising; CLR low async clears");
    bench_destroy(b);
}

static void check_264(void) {
    Bench *b = open_fx("74_264");
    int ok;

    set(b, "CLR", L1);
    set(b, "B", L1);
    set(b, "CLK", L0);
    step(b);
    shift_in(b, "A", "CLK", 8, 0x0F);
    ok = pins_value(b, Q8, 8, ~reversed(0x0F, 8) & 0xFF);
    set(b, "CLR", L0);
    step(b);
    expect(ok && pins_all(b, Q8, 8, L1), "2 74_264 outputs inverted; after clear all 1");
    bench_destroy(b);
}

static void check_166(void) {
    Bench *b = open_fx("74_166");
    int ok;

    set(b, "CLR", L1);
    set(b, "CLKINH", L0);
    set(b, "CLK", L0);
    set(b, "SER", L0);
    set_pins(b, D8, 8, 0x80);
    set(b, "SH/LD", L0);
    step(b);
    ok = is(b, "QH", L0);   /* synchronous: nothing yet */
    pulse(b, "CLK");
    ok &= is(b, "QH", L1);
    set(b, "CLR", L0);
    step(b);
    expect(ok && is(b, "QH", L0), "4 74_166 SH/LD low loads at the edge; CLR async");
    bench_destroy(b);
}

/* ---- 194 / 195 --------------------------------------------------------------- */

static const char *const QAD[] = {"QA", "QB", "QC", "QD"};

static void s194(Bench *b, int s0, int s1) {
    set(b, "S0", s0 ? L1 : L0);
    set(b, "S1", s1 ? L1 : L0);
    step(b);
}

static void check_194(void) {
    Bench *b = open_fx("74_194");
    int ok;

    set(b, "CLR", L1);
    set(b, "CLK", L0);
    set(b, "SRSER", L1);
    set(b, "SLSER", L0);
    set_pins(b, D8, 4, 0x6);
    s194(b, 1, 1);
    pulse(b, "CLK");
    ok = pins_value(b, QAD, 4, 0x6);   /* load */
    s194(b, 0, 0);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 0x6);   /* hold */
    s194(b, 1, 0);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 0xD);   /* right: QA <- SR (1), QB <- QA ... */
    s194(b, 0, 1);
    pulse(b, "CLK");
    expect(ok && pins_value(b, QAD, 4, 0x6), "5 74_194 each S1 S0 row: hold, right, left, load");
    bench_destroy(b);
}

static void check_195(void) {
    static const char *const Q[] = {"Q0", "Q1", "Q2", "Q3"};
    static const char *const D[] = {"D0", "D1", "D2", "D3"};
    Bench *b = open_fx("74_195");
    int ok;

    set(b, "MR", L1);
    set(b, "PE", L1);
    set(b, "CP", L0);
    set(b, "J", L1);
    set(b, "K", L1);
    step(b);
    pulse(b, "CP");   /* J = 1, K' = 1: set */
    ok = is(b, "Q0", L1);
    set(b, "J", L0);
    set(b, "K", L0);
    step(b);
    pulse(b, "CP");   /* reset */
    ok &= is(b, "Q0", L0) && is(b, "Q1", L1);
    set(b, "J", L1);
    step(b);
    pulse(b, "CP");   /* J = 1, K' = 0: toggle (0 -> 1) */
    ok &= is(b, "Q0", L1);
    pulse(b, "CP");   /* toggle (1 -> 0) */
    ok &= is(b, "Q0", L0);
    set(b, "J", L0);
    set(b, "K", L1);
    step(b);
    pulse(b, "CP");   /* keep */
    ok &= is(b, "Q0", L0);
    set_pins(b, D, 4, 0x9);
    set(b, "PE", L0);
    step(b);
    pulse(b, "CP");
    expect(ok && pins_value(b, Q, 4, 0x9) && is(b, "Q3BAR", L0),
           "6 74_195 J/K' rows: set, reset, toggle, keep; PE low loads");
    bench_destroy(b);
}

static void check_323(void) {
    static const char *const IO[] = {"I/O0", "I/O1", "I/O2", "I/O3", "I/O4", "I/O5", "I/O6",
                                     "I/O7"};
    Bench *b = open_fx("74_323");
    int ok;

    set(b, "MR", L1);
    set(b, "CP", L0);
    set(b, "OE1", L1);
    set(b, "OE2", L1);
    set(b, "S0", L1);
    set(b, "S1", L1);
    set_pins(b, IO, 8, 0x81);
    step(b);
    pulse(b, "CP");
    release_pins(b, IO, 8);
    set(b, "S1", L0);
    set(b, "S0", L0);   /* hold */
    step(b);
    set(b, "MR", L0);
    step(b);
    ok = is(b, "Q0", L1) && is(b, "Q7", L1);   /* MR low but no edge yet */
    pulse(b, "CP");
    expect(ok && is(b, "Q0", L0) && is(b, "Q7", L0), "8 74_323 MR clears only at the edge");
    bench_destroy(b);
}

/* ---- CD4000 ------------------------------------------------------------------------ */

static const char *const PI8[] = {"PI-1", "PI-2", "PI-3", "PI-4", "PI-5", "PI-6", "PI-7", "PI-8"};
static const char *const Q678[] = {"Q6", "Q7", "Q8"};

static void check_4014(void) {
    Bench *b = open_fx("4014B");
    Bench *c = open_fx("4021B");
    int ok;

    set(b, "CLOCK", L0);
    set(b, "SERIALIN", L0);
    set_pins(b, PI8, 8, 0xE0);
    set(b, "P/S", L1);
    step(b);
    ok = pins_value(b, Q678, 3, 0);   /* 4014B: synchronous */
    pulse(b, "CLOCK");
    ok &= pins_value(b, Q678, 3, 7);
    set(c, "CLOCK", L0);
    set(c, "SERIALIN", L0);
    set_pins(c, PI8, 8, 0xE0);
    set(c, "P/S", L1);
    step(c);
    ok &= pins_value(c, Q678, 3, 7);   /* 4021B: asynchronous */
    set(c, "P/S", L0);
    step(c);
    pulse(c, "CLOCK");
    expect(ok && pins_value(c, Q678, 3, 6), "9 4014B / 4021B synchronous vs asynchronous parallel load");
    bench_destroy(b);
    bench_destroy(c);
}

static void check_4015(void) {
    static const char *const QA4[] = {"Q1A", "Q2A", "Q3A", "Q4A"};
    static const char *const QB4[] = {"Q1B", "Q2B", "Q3B", "Q4B"};
    Bench *b = open_fx("4015B");
    int ok;

    set(b, "RESETA", L0);
    set(b, "RESETB", L0);
    set(b, "CLOCKA", L0);
    set(b, "CLOCKB", L0);
    set(b, "DATAB", L0);
    step(b);
    shift_in(b, "DATAA", "CLOCKA", 4, 0xA);
    ok = pins_value(b, QA4, 4, reversed(0xA, 4)) && pins_value(b, QB4, 4, 0);
    set(b, "RESETA", L1);
    step(b);
    expect(ok && pins_value(b, QA4, 4, 0), "10 4015B two independent units; RESET high clears");
    bench_destroy(b);
}

static void check_4035(void) {
    static const char *const Q[] = {"Q1", "Q2", "Q3", "Q4"};
    static const char *const PI[] = {"PI-1", "PI-2", "PI-3", "PI-4"};
    Bench *b = open_fx("4035B");
    int ok;

    set(b, "RESET", L0);
    set(b, "CLOCK", L0);
    set(b, "J", L0);
    set(b, "K", L0);
    set(b, "T/C", L1);
    set_pins(b, PI, 4, 0x5);
    set(b, "P/S", L1);
    step(b);
    pulse(b, "CLOCK");
    ok = pins_value(b, Q, 4, 0x5);
    set(b, "T/C", L0);
    step(b);
    expect(ok && pins_value(b, Q, 4, 0xA), "11 4035B T/C low inverts the outputs at once");
    bench_destroy(b);
}

static void check_4031(void) {
    Bench *b = open_fx("4031B");
    int ok = 1;
    unsigned k;

    set(b, "MODE", L0);
    set(b, "CL", L0);
    set(b, "RECIRC", L0);
    set(b, "DATA", L1);
    step(b);
    pulse(b, "CL");   /* a single 1 enters stage 1 */
    set(b, "DATA", L0);
    step(b);
    for (k = 2; k <= 64; k++) {
        set(b, "CL", L1);
        step(b);
        ok &= is(b, "CLD", L1) && is(b, "Q", k == 64 ? L1 : L0) && is(b, "Q'", L0);
        set(b, "CL", L0);
        step(b);
        ok &= is(b, "CLD", L0);
    }
    ok &= is(b, "Q", L1) && is(b, "QBAR", L0) && is(b, "Q'", L1);   /* Q' half a clock later */
    set(b, "MODE", L1);   /* recirculate: Q -> stage 1 */
    set(b, "RECIRC", L0);
    step(b);
    for (k = 1; k <= 64; k++) {
        set(b, "RECIRC", bench_get(b, "Q"));
        step(b);
        pulse(b, "CL");
    }
    expect(ok && is(b, "Q", L1),
           "12 4031B 64-clock delay to Q; Q' half a clock later; CLD follows CL; MODE 1 "
           "recirculates");
    bench_destroy(b);
}

static void check_594(void) {
    Bench *b = open_fx("74_594");
    int ok;

    set(b, "SRCLR", L1);
    set(b, "RCLR", L1);
    set(b, "SRCLK", L0);
    set(b, "RCLK", L0);
    step(b);
    shift_in(b, "SER", "SRCLK", 8, 0xFF);
    pulse(b, "RCLK");
    ok = pins_value(b, Q8, 8, 0xFF);
    set(b, "RCLR", L0);
    step(b);
    ok &= pins_value(b, Q8, 8, 0) && is(b, "QH'", L1);
    expect(ok, "15 74_594 RCLR clears the storage only");
    bench_destroy(b);
}

static void check_596(void) {
    Bench *b = open_fx("74_596");
    int ok;

    set(b, "SRCLR", L1);
    set(b, "OE", L0);
    set(b, "SRCLK", L0);
    set(b, "RCLK", L0);
    step(b);
    shift_in(b, "SER", "SRCLK", 8, 0x0F);
    pulse(b, "RCLK");
    {
        unsigned v = reversed(0x0F, 8);
        unsigned i;

        ok = 1;
        for (i = 0; i < 8; i++) {
            ok &= is(b, Q8[i], (v >> i) & 1u ? LZ : L0);
        }
    }
    expect(ok, "16 74_596 storage bit 0 -> output 0, bit 1 -> Z (open drain)");
    bench_destroy(b);
}

static void check_597(void) {
    Bench *b = open_fx("74_597");
    int ok;

    set(b, "SRCLR", L1);
    set(b, "SRLOAD", L1);
    set(b, "SRCK", L0);
    set(b, "RCK", L0);
    set(b, "SER", L0);
    set_pins(b, D8, 8, 0x80);
    step(b);
    ok = is(b, "QH'", L0);
    pulse(b, "RCK");
    ok &= is(b, "QH'", L0);   /* only the input register */
    set(b, "SRLOAD", L0);
    step(b);
    ok &= is(b, "QH'", L1);
    set(b, "SRCLR", L0);   /* clear beats load */
    step(b);
    expect(ok && is(b, "QH'", L0),
           "17 74_597 RCK rising loads the input register; SRLOAD low copies it; clear beats load");
    bench_destroy(b);
}

static const char *const IO598[] = {"A/QA", "B/QB", "C/QC", "D/QD", "E/QE", "F/QF", "G/QG",
                                    "H/QH"};

static void check_598(void) {
    Bench *b = open_fx("74_598");
    int ok;

    set(b, "SRCLR", L1);
    set(b, "SRLOAD", L1);
    set(b, "SRCKEN", L0);
    set(b, "G", L1);
    set(b, "SRCK", L0);
    set(b, "RCK", L0);
    set(b, "SER0", L0);
    set(b, "SER1", L1);
    set(b, "DS", L1);
    step(b);
    pulse(b, "SRCK");   /* DS high: SER1 (1) */
    set(b, "DS", L0);
    step(b);
    pulse(b, "SRCK");   /* DS low: SER0 (0) */
    set(b, "SRCKEN", L1);
    step(b);
    pulse(b, "SRCK");   /* blocked */
    set(b, "G", L0);
    step(b);
    ok = pins_value(b, IO598, 8, 0x02);
    expect(ok && bench_error(b) == NULL,
           "18 74_598 DS selects SER0/SER1; SRCKEN high blocks shifting; G low drives the stages "
           "onto the pins");
    bench_destroy(b);
}

static void check_tpic6b596(void) {
    Bench *b = open_fx("TPIC6B596");
    int ok;
    unsigned k;

    set(b, "SRCLR", L1);
    set(b, "G", L0);
    set(b, "SRCK", L0);
    set(b, "RCK", L0);
    set(b, "SERIN", L1);
    step(b);
    for (k = 0; k < 7; k++) {
        pulse(b, "SRCK");
    }
    ok = is(b, "SEROUT", L0);
    set(b, "SRCK", L1);   /* the 8th rising edge: stage 8 becomes 1 */
    step(b);
    ok &= is(b, "SEROUT", L0);
    set(b, "SRCK", L0);   /* falling: SER OUT takes it */
    step(b);
    ok &= is(b, "SEROUT", L1);
    pulse(b, "RCK");
    ok &= drains(b, 0xFF);
    set(b, "SRCLR", L0);   /* clears the shift register, the storage and SER OUT */
    step(b);
    expect(ok && drains(b, 0) && is(b, "SEROUT", L0),
           "20 TPIC6B596 SER OUT changes on SRCK falling; SRCLR clears the storage too");
    bench_destroy(b);
}

/* ---- Loader and voltage ------------------------------------------------------------ */

static int expect_error(const char *text, const char *want) {
    PartLibrary *lib = fixture_library();
    char *error = load(lib, text, "bad.json");
    int ok = error != NULL && strstr(error, want) != NULL;

    if (!ok) {
        printf("  error: %s\n  want: %s\n", error != NULL ? error : "(none)", want);
    }
    free(error);
    part_library_destroy(lib);
    return ok;
}

#define BAD_HEAD \
    "{'format':'schematical-parts','version':1,'kind':'shift','devices':[{'id':'bad'," \
    "'function':'Shift register','description':'d','drawer':[],'pinouts':{'p4':[" \
    "{'number':1,'name':'A','role':'input'},{'number':2,'name':'B','role':'input'}," \
    "{'number':3,'name':'Q','role':'output'},{'number':4,'name':'C','role':'input'}]}," \
    "'model':'shift','units':[{"
#define BAD_TAIL "}],'timing':{'HC':{'rise':1,'fall':1,'vcc':[2,6]}},'parts':[]}]}"

static void check_loader(void) {
    int ok = expect_error(BAD_HEAD "'stages':4,'clock':{'pin':'A','edge':'rising'},'serial':['B'],"
                          "'serialJK':{'j':'B','kbar':'C'}" BAD_TAIL,
                          "exactly one of \"serial\", \"serialJK\" and \"serialSelect\"");

    ok &= expect_error(BAD_HEAD "'stages':4,'clock':{'pin':'A','edge':'rising'},'serial':['B'],"
                       "'storage':{'clock':{'pin':'C','edge':'rising'},'latch':{'pin':'C',"
                       "'active':'high'},'outputs':['Q','Q','Q','Q']}" BAD_TAIL,
                       "a storage has at most one of \"clock\" and \"latch\"");
    ok &= expect_error(BAD_HEAD "'stages':65,'clock':{'pin':'A','edge':'rising'},'serial':['B']"
                       BAD_TAIL, "\"stages\" must be 1 to 64");
    expect(ok, "22 loader: serial with serialJK, a storage with both clock and latch, and 65 stages "
               "are errors");
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
    const Part *tpic = part_library_find(library, "FX-TPIC6B595");
    const Part *od = part_library_find(library, "FX-74_596");
    const Part *pp = part_library_find(library, "FX-74_595");

    expect(tpic != NULL && od != NULL && pp != NULL &&
               part_pin_open_drain(tpic, pin_index(tpic, "DRAIN0")) &&
               part_pin_open_drain(od, pin_index(od, "QA")) &&
               !part_pin_open_drain(pp, pin_index(pp, "QA")) &&
               !part_pin_open_drain(tpic, pin_index(tpic, "SEROUT")),
           "23 voltage: open drain on FX-TPIC6B595 DRAIN0 and FX-74_596 QA, not FX-74_595 QA");
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
    check_164();
    check_264();
    check_165();
    check_166();
    check_194();
    check_195();
    check_299();
    check_323();
    check_4014();
    check_4015();
    check_4035();
    check_4031();
    check_595();
    check_594();
    check_596();
    check_597();
    check_598();
    check_tpic6b595();
    check_tpic6b596();
    check_4094();
    check_loader();
    check_open_drain();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("shift_test: all passed\n");
    return 0;
}
