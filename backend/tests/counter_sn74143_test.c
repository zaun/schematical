/*
 * counter_sn74143_test (plan-56-F): the SN74143 (4-bit decade counter / latch with a seven-segment
 * constant-current LED driver, datasheets/counters/sn74143.pdf) on the real part SN74143N, covering the
 * keys the counter kind's `seg7` model gained for it: segmentsActive, segmentsOutput (open drain),
 * toggleEnable as an array, maxCount, latchOutputs, blank, rbi + rbo, decimalPoint. Rows are those of the
 * p.7 function table; the bench plan is planning/plan-56-data/specs/pcf1179-sn74143.md § 2.
 *
 * Lit segments and dp are pins pulled low, off ones are released (Z). The bench leaves BI/RBO unpulled
 * (a released RBO reads Z), except where two digits are cascaded (the real chip's internal pull-up is a
 * weak 1 there).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "model_pins.h"
#include "parts.h"
#include "read_file.h"
#include "sim.h"

static int failures;
static PartLibrary *library;

static void check(int ok, const char *what) {
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

#define NS ((SimTime)1000)
#define MAXCHIPS 2
#define STRIDE 32

typedef struct {
    size_t count;
} PinCount;

static size_t drv_pins(const void *config) {
    return ((const PinCount *)config)->count;
}

static void *drv_create(const void *config) {
    PinCount *c = malloc(sizeof *c);

    *c = *(const PinCount *)config;
    return c;
}

static const SimComponentType DRIVER = {"tb", drv_pins, drv_create, free, NULL, NULL, NULL,
                                        NULL, NULL, NULL, NULL, NULL};

typedef struct {
    Sim *sim;
    SimInstance *chip[MAXCHIPS], *drv, *pull;
    const Part *part;
    unsigned chips;
    PinCount pins;
} Rig;

static size_t pin_of(const Part *part, const char *name) {
    size_t i;

    for (i = 0; i < part->pin_count; i++) {
        if (strcmp(part->pins[i].name, name) == 0) {
            return i;
        }
    }
    fprintf(stderr, "no pin %s\n", name);
    exit(2);
}

/*
 * One or two SN74143N. joins: {chipA, "PIN", chipB, "PIN", chipA, "PIN", ...} as a flat list
 * (a chip index as a small integer cast to a string pointer is not used: the lists below are arrays of
 * the Join struct).
 */
typedef struct {
    unsigned chip_a;
    const char *pin_a;
    unsigned chip_b;
    const char *pin_b;
    int pull_up;   /* a weak 1 on the joined net */
} Join;

static Rig *rig(unsigned chips, const Join *joins, size_t join_count) {
    Rig *r = calloc(1, sizeof *r);
    SimNet *nets[MAXCHIPS][STRIDE] = {{NULL}};
    size_t i;
    unsigned c;

    r->part = part_library_find(library, "SN74143N");
    if (r->part == NULL) {
        fprintf(stderr, "no part SN74143N\n");
        exit(2);
    }
    r->chips = chips;
    r->pins.count = chips * STRIDE;
    r->sim = sim_create();
    r->drv = sim_add_instance(r->sim, &DRIVER, "TB", &r->pins);
    r->pull = sim_add_instance(r->sim, &DRIVER, "PU", &r->pins);
    for (c = 0; c < chips; c++) {
        r->chip[c] = sim_add_instance(r->sim, kind_model(r->part), c == 0 ? "U1" : "U2", r->part);
    }
    for (i = 0; i < join_count; i++) {
        size_t pa = pin_of(r->part, joins[i].pin_a), pb = pin_of(r->part, joins[i].pin_b);
        SimNet *net = sim_add_net(r->sim, "joined");

        nets[joins[i].chip_a][pa] = net;
        nets[joins[i].chip_b][pb] = net;
        if (joins[i].pull_up) {
            sim_connect(r->sim, net, r->pull, joins[i].chip_a * STRIDE + pa);
            sim_connect(r->sim, net, r->pull, joins[i].chip_b * STRIDE + pb);
        }
    }
    for (c = 0; c < chips; c++) {
        for (i = 0; i < r->part->pin_count; i++) {
            SimNet *net = nets[c][i] != NULL ? nets[c][i] : sim_add_net(r->sim, r->part->pins[i].name);

            sim_connect(r->sim, net, r->chip[c], i);
            sim_connect(r->sim, net, r->drv, c * STRIDE + i);
        }
    }
    for (i = 0; i < join_count; i++) {
        if (joins[i].pull_up) {
            sim_schedule_drive(r->sim, r->pull, joins[i].chip_a * STRIDE + pin_of(r->part, joins[i].pin_a),
                               LOGIC_1, DRIVE_WEAK, 0);
        }
    }
    if (!sim_start(r->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(r->sim));
        exit(2);
    }
    for (c = 0; c < chips; c++) {
        sim_schedule_drive(r->sim, r->drv, c * STRIDE + pin_of(r->part, "VCC"), LOGIC_1, DRIVE_STRONG, 0);
        sim_schedule_drive(r->sim, r->drv, c * STRIDE + pin_of(r->part, "GND"), LOGIC_0, DRIVE_STRONG, 0);
    }
    return r;
}

static void rig_close(Rig *r) {
    sim_destroy(r->sim);
    free(r);
}

static void run(Rig *r, SimTime ps) {
    if (!sim_run_until(r->sim, sim_now(r->sim) + ps)) {
        fprintf(stderr, "sim stopped: %s\n", sim_error(r->sim));
        exit(2);
    }
}

static void setc(Rig *r, unsigned chip, const char *name, LogicValue v) {
    sim_schedule_drive(r->sim, r->drv, chip * STRIDE + pin_of(r->part, name), v, DRIVE_STRONG, 0);
}

static void set(Rig *r, const char *name, LogicValue v) {
    unsigned c;

    for (c = 0; c < r->chips; c++) {
        setc(r, c, name, v);
    }
}

static LogicValue getc_(Rig *r, unsigned chip, const char *name) {
    return sim_read_pin(r->sim, r->chip[chip], pin_of(r->part, name));
}

static void settle(Rig *r) {
    run(r, 200 * NS);
}

/* One clock pulse on every chip: high 100 ns, low 100 ns. */
static void pulse(Rig *r) {
    set(r, "CLK", LOGIC_1);
    run(r, 100 * NS);
    set(r, "CLK", LOGIC_0);
    run(r, 100 * NS);
}

/* The normal operating inputs of every chip. */
static void inputs(Rig *r) {
    set(r, "CLK", LOGIC_0);
    set(r, "CLR", LOGIC_1);
    set(r, "RBI", LOGIC_1);
    set(r, "BI", LOGIC_0);
    set(r, "DP", LOGIC_0);
    set(r, "SCEI", LOGIC_0);
    set(r, "PCEI", LOGIC_0);
    set(r, "STRB", LOGIC_0);
}

static void clear(Rig *r) {
    set(r, "CLR", LOGIC_0);
    run(r, 100 * NS);
    set(r, "CLR", LOGIC_1);
    settle(r);
}

static const char *const SEG[7] = {"a", "b", "c", "d", "e", "f", "g"};

/* Lit segments as "abcdefg" 1/0 (a lit one is pulled low, an off one released); "x" for anything else. */
static void segs(Rig *r, unsigned chip, char *out) {
    unsigned j;

    for (j = 0; j < 7; j++) {
        LogicValue v = getc_(r, chip, SEG[j]);

        out[j] = v == LOGIC_0 ? '1' : v == LOGIC_Z ? '0' : 'x';
    }
    out[7] = 0;
}

static int shows(Rig *r, unsigned chip, const char *glyph) {
    char s[8];

    segs(r, chip, s);
    return strcmp(s, glyph) == 0;
}

static const char *const GLYPH[10] = {"1111110", "0110000", "1101101", "1111001", "0110011",
                                      "1011011", "1011111", "1110000", "1111111", "1111011"};
static const char *const BLANK = "0000000";

/* The latched Q code, or -1 when any bit is not 0 or 1. */
static int qcode(Rig *r, unsigned chip) {
    static const char *const Q[4] = {"QA", "QB", "QC", "QD"};
    int code = 0, i;

    for (i = 0; i < 4; i++) {
        LogicValue v = getc_(r, chip, Q[i]);

        if (v != LOGIC_0 && v != LOGIC_1) {
            return -1;
        }
        code |= (v == LOGIC_1) << i;
    }
    return code;
}

static int is(Rig *r, unsigned chip, const char *name, LogicValue v) {
    return getc_(r, chip, name) == v;
}

/* dp pin: lit (pulled low) -> 1, released -> 0, else -1. */
static int dp_lit(Rig *r, unsigned chip) {
    LogicValue v = getc_(r, chip, "dp");

    return v == LOGIC_0 ? 1 : v == LOGIC_Z ? 0 : -1;
}

/* ---- Tests -------------------------------------------------------------------------------------- */

static void test_listing(void) {
    const Part *p = part_library_find(library, "SN74143N");
    int ok = p != NULL;

    if (ok) {
        const char *od[] = {"a", "b", "c", "d", "e", "f", "g", "dp", "BI/RBO"};
        const char *pp[] = {"QA", "QB", "QC", "QD", "MAX", "SCEI", "PCEI", "CLK", "CLR", "RBI", "BI",
                            "DP", "STRB"};
        size_t i;

        for (i = 0; i < sizeof od / sizeof *od; i++) {
            ok &= part_pin_open_drain(p, pin_of(p, od[i])) == 1;
        }
        for (i = 0; i < sizeof pp / sizeof *pp; i++) {
            ok &= part_pin_open_drain(p, pin_of(p, pp[i])) == 0;
        }
        ok &= p->pin_count == 24 && p->pins[pin_of(p, "BI/RBO")].role == PIN_INOUT;
    }
    check(ok, "SN74143N: 24 pins; segments, dp and BI/RBO are open drain, Q/MAX/inputs are not");
}

static void test_power_on(void) {
    Rig *r = rig(1, NULL, 0);
    char s[8];
    int ok;

    inputs(r);
    settle(r);
    segs(r, 0, s);
    ok = shows(r, 0, GLYPH[0]) && qcode(r, 0) == 0 && is(r, 0, "MAX", LOGIC_1) &&
         is(r, 0, "BI/RBO", LOGIC_Z) && dp_lit(r, 0) == 0;
    check(ok, "1 power-on: shows 0 (a-f low, g released), Q 0000, MAX high, RBO released, dp off");

    pulse(r);
    pulse(r);
    pulse(r);
    set(r, "CLR", LOGIC_0);
    run(r, 100 * NS);
    ok = shows(r, 0, GLYPH[0]) && qcode(r, 0) == 0 && is(r, 0, "BI/RBO", LOGIC_Z);
    check(ok, "1 CLR low (RBI high): count and latch go to 0, shows 0");
    set(r, "RBI", LOGIC_0);
    run(r, 100 * NS);
    ok = shows(r, 0, BLANK) && is(r, 0, "BI/RBO", LOGIC_0) && qcode(r, 0) == 0 && is(r, 0, "MAX", LOGIC_1);
    check(ok, "1 CLR low with RBI low: all outputs off, RBO low, Q 0000, MAX high (function table row 1)");
    pulse(r);
    ok = shows(r, 0, BLANK) && qcode(r, 0) == 0;
    check(ok, "1 clock pulses while CLR is low do not count");
    set(r, "CLR", LOGIC_1);
    set(r, "RBI", LOGIC_1);
    settle(r);
    ok = shows(r, 0, GLYPH[0]) && is(r, 0, "BI/RBO", LOGIC_Z);
    check(ok, "1 CLR and RBI high again: shows 0, RBO released (no latch-up on its own low)");
    rig_close(r);
}

static void test_count(void) {
    Rig *r = rig(1, NULL, 0);
    int ok = 1, k;
    char what[96];

    inputs(r);
    settle(r);
    for (k = 1; k <= 20; k++) {
        int expect_count = k % 10;
        int g;

        pulse(r);
        g = shows(r, 0, GLYPH[expect_count]) && qcode(r, 0) == expect_count &&
            is(r, 0, "MAX", expect_count == 9 ? LOGIC_0 : LOGIC_1) && dp_lit(r, 0) == 0 &&
            is(r, 0, "BI/RBO", LOGIC_Z);
        if (!g) {
            snprintf(what, sizeof what, "  count %d: Q=%d MAX=%d", expect_count, qcode(r, 0),
                     (int)getc_(r, 0, "MAX"));
            printf("%s\n", what);
        }
        ok &= g;
    }
    check(ok, "2 20 clock pulses: counts 1..9, wraps to 0 twice; glyph, Q code and MAX (low only at 9) each step");

    clear(r);
    set(r, "CLK", LOGIC_1);
    run(r, 20 * NS);
    ok = qcode(r, 0) == 0;
    run(r, 100 * NS);
    set(r, "CLK", LOGIC_0);
    run(r, 100 * NS);
    ok &= qcode(r, 0) == 1;
    check(ok, "2 Q and the segments follow the clock edge after the 28 ns delay (not before)");

    clear(r);
    {
        int i;

        for (i = 0; i < 8; i++) {
            pulse(r);
        }
    }
    set(r, "CLK", LOGIC_1);
    run(r, 20 * NS);
    ok = is(r, 0, "MAX", LOGIC_1);
    run(r, 20 * NS);
    ok &= is(r, 0, "MAX", LOGIC_0);
    set(r, "CLK", LOGIC_0);
    run(r, 100 * NS);
    set(r, "CLK", LOGIC_1);
    run(r, 20 * NS);
    ok &= is(r, 0, "MAX", LOGIC_0);
    run(r, 20 * NS);
    ok &= is(r, 0, "MAX", LOGIC_1) && shows(r, 0, GLYPH[0]);
    set(r, "CLK", LOGIC_0);
    run(r, 100 * NS);
    check(ok, "2 MAX falls 29 ns and rises 26 ns after the clock edge (maxFall / maxRise)");
    rig_close(r);
}

static void test_enables(void) {
    Rig *r = rig(1, NULL, 0);
    int ok;
    int i;

    inputs(r);
    settle(r);
    pulse(r);
    pulse(r);
    pulse(r);
    set(r, "SCEI", LOGIC_1);
    settle(r);
    pulse(r);
    pulse(r);
    ok = qcode(r, 0) == 3 && shows(r, 0, GLYPH[3]) && is(r, 0, "MAX", LOGIC_1);
    check(ok, "2 SCEI high: the clock does not count (count 3 holds)");
    set(r, "SCEI", LOGIC_0);
    set(r, "PCEI", LOGIC_1);
    settle(r);
    pulse(r);
    pulse(r);
    ok = qcode(r, 0) == 3;
    check(ok, "2 PCEI high: the clock does not count");
    set(r, "PCEI", LOGIC_0);
    settle(r);
    pulse(r);
    ok = qcode(r, 0) == 4;
    check(ok, "2 both enables low again: counts");

    for (i = 0; i < 5; i++) {
        pulse(r);
    }
    ok = qcode(r, 0) == 9 && is(r, 0, "MAX", LOGIC_0);
    set(r, "SCEI", LOGIC_1);
    settle(r);
    ok &= is(r, 0, "MAX", LOGIC_1);
    pulse(r);
    ok &= qcode(r, 0) == 9 && is(r, 0, "MAX", LOGIC_1);
    set(r, "SCEI", LOGIC_0);
    set(r, "PCEI", LOGIC_1);
    settle(r);
    ok &= is(r, 0, "MAX", LOGIC_0);
    pulse(r);
    ok &= qcode(r, 0) == 9 && is(r, 0, "MAX", LOGIC_0);
    check(ok, "2 at count 9: SCEI high forces MAX high and blocks counting; PCEI high leaves MAX low (it depends on SCEI only)");
    set(r, "PCEI", LOGIC_0);
    settle(r);
    pulse(r);
    ok = qcode(r, 0) == 0 && is(r, 0, "MAX", LOGIC_1);
    check(ok, "2 enables low: 9 wraps to 0, MAX high");

    /* unknown enables: X with the other one inhibiting is a known hold; X with the other asserted is unknown */
    set(r, "SCEI", LOGIC_X);
    set(r, "PCEI", LOGIC_1);
    settle(r);
    pulse(r);
    ok = qcode(r, 0) == 0 && shows(r, 0, GLYPH[0]);
    check(ok, "2 SCEI unknown with PCEI high: the count holds (known inhibit)");
    set(r, "PCEI", LOGIC_0);
    settle(r);
    ok = is(r, 0, "MAX", LOGIC_1);   /* count 0 -> MAX high whatever SCEI is */
    pulse(r);
    ok &= qcode(r, 0) == -1 && !shows(r, 0, GLYPH[0]);
    check(ok, "2 SCEI unknown with PCEI low at a clock edge: the count and Q become unknown; MAX high at 0 regardless");
    clear(r);
    rig_close(r);
}

static void test_strobe(void) {
    Rig *r = rig(1, NULL, 0);
    int ok, i;

    inputs(r);
    settle(r);
    for (i = 0; i < 5; i++) {
        pulse(r);
    }
    set(r, "STRB", LOGIC_1);
    settle(r);
    ok = qcode(r, 0) == 5 && shows(r, 0, GLYPH[5]);
    pulse(r);
    ok &= qcode(r, 0) == 5 && shows(r, 0, GLYPH[5]) && is(r, 0, "MAX", LOGIC_1);
    pulse(r);
    ok &= qcode(r, 0) == 5 && shows(r, 0, GLYPH[5]);
    check(ok, "3 STRB high at count 5: Q and segments hold 5 through counts 6 and 7");
    pulse(r);
    pulse(r);
    ok = qcode(r, 0) == 5 && shows(r, 0, GLYPH[5]) && is(r, 0, "MAX", LOGIC_0);
    check(ok, "3 the counter keeps counting under the strobe: at count 9 MAX is low while the latch still shows 5");
    set(r, "STRB", LOGIC_0);
    settle(r);
    ok = qcode(r, 0) == 9 && shows(r, 0, GLYPH[9]) && is(r, 0, "MAX", LOGIC_0);
    check(ok, "3 STRB low: the latch follows the counter (jumps to 9)");
    pulse(r);
    ok = qcode(r, 0) == 0 && shows(r, 0, GLYPH[0]);
    check(ok, "3 transparent again: the next count shows at once");
    /* the latched 0 blanks under RBI regardless of where the counter is */
    set(r, "STRB", LOGIC_1);
    settle(r);
    set(r, "RBI", LOGIC_0);
    pulse(r);
    ok = qcode(r, 0) == 0 && shows(r, 0, BLANK) && is(r, 0, "BI/RBO", LOGIC_0);
    check(ok, "3 ripple blanking follows the latched 0 (counter at 1, latch 0, RBI low: blank, RBO low)");
    set(r, "RBI", LOGIC_1);
    set(r, "STRB", LOGIC_0);
    settle(r);
    ok = qcode(r, 0) == 1 && shows(r, 0, GLYPH[1]) && is(r, 0, "BI/RBO", LOGIC_Z);
    check(ok, "3 strobe low, RBI high: shows 1, RBO released");
    rig_close(r);
}

static void test_dp(void) {
    Rig *r = rig(1, NULL, 0);
    int ok;

    inputs(r);
    settle(r);
    pulse(r);
    pulse(r);
    pulse(r);
    set(r, "DP", LOGIC_1);
    settle(r);
    ok = dp_lit(r, 0) == 1 && shows(r, 0, GLYPH[3]) && is(r, 0, "dp", LOGIC_0);
    check(ok, "4 DP high with count 3: dp pin pulled low (lit), the segments still show 3");
    set(r, "DP", LOGIC_0);
    settle(r);
    ok = dp_lit(r, 0) == 0 && is(r, 0, "dp", LOGIC_Z);
    check(ok, "4 DP low: dp released (no pull-up)");
    set(r, "DP", LOGIC_1);
    set(r, "BI", LOGIC_1);
    settle(r);
    ok = dp_lit(r, 0) == 0 && shows(r, 0, BLANK);
    check(ok, "4 DP high with BI high: dp off with every segment");
    set(r, "BI", LOGIC_0);
    setc(r, 0, "BI/RBO", LOGIC_0);
    settle(r);
    ok = dp_lit(r, 0) == 0 && shows(r, 0, BLANK);
    check(ok, "4 DP high while BI/RBO is held low externally: dp off with every segment");
    setc(r, 0, "BI/RBO", LOGIC_Z);
    settle(r);
    ok = dp_lit(r, 0) == 1 && shows(r, 0, GLYPH[3]);
    check(ok, "4 releasing BI/RBO: dp and the glyph come back");
    clear(r);
    set(r, "RBI", LOGIC_0);
    settle(r);
    ok = dp_lit(r, 0) == 0 && shows(r, 0, BLANK);
    check(ok, "4 DP high with RBI low at code 0: dp off with every segment (function table: ripple blank)");
    set(r, "RBI", LOGIC_1);
    settle(r);
    ok = dp_lit(r, 0) == 1 && shows(r, 0, GLYPH[0]);
    check(ok, "4 RBI high at code 0: 0 and dp lit (function table 'Decimal 0')");
    pulse(r);
    set(r, "RBI", LOGIC_0);
    settle(r);
    ok = dp_lit(r, 0) == 1 && shows(r, 0, GLYPH[1]);
    check(ok, "4 RBI low at code 1: no ripple blanking, dp stays lit");
    set(r, "DP", LOGIC_X);
    settle(r);
    ok = dp_lit(r, 0) == -1 && shows(r, 0, GLYPH[1]);
    check(ok, "4 DP unknown: dp pin unknown, segments unaffected");
    rig_close(r);
}

static void test_blank(void) {
    Rig *r = rig(1, NULL, 0);
    int ok, i;
    char s[8];

    inputs(r);
    settle(r);
    for (i = 0; i < 7; i++) {
        pulse(r);
    }
    set(r, "BI", LOGIC_1);
    settle(r);
    segs(r, 0, s);
    ok = shows(r, 0, BLANK) && dp_lit(r, 0) == 0 && is(r, 0, "BI/RBO", LOGIC_0) && qcode(r, 0) == 7 &&
         is(r, 0, "MAX", LOGIC_1);
    check(ok, "5 BI high: every segment and dp released, BI/RBO pulled low, Q and MAX unaffected");
    pulse(r);
    pulse(r);
    pulse(r);
    ok = shows(r, 0, BLANK) && qcode(r, 0) == 0 && is(r, 0, "BI/RBO", LOGIC_0);
    set(r, "BI", LOGIC_0);
    settle(r);
    ok &= shows(r, 0, GLYPH[0]) && is(r, 0, "BI/RBO", LOGIC_Z);
    check(ok, "5 counting continues under BI; releasing BI shows the current digit, RBO released");

    /* RBI low with the latched code: 0 blanks and pulls RBO low, 1 does not */
    set(r, "RBI", LOGIC_0);
    settle(r);
    ok = shows(r, 0, BLANK) && is(r, 0, "BI/RBO", LOGIC_0);
    pulse(r);
    ok &= shows(r, 0, GLYPH[1]) && is(r, 0, "BI/RBO", LOGIC_Z);
    check(ok, "5 RBI low: code 0 blanks and pulls RBO low; code 1 shows and releases RBO");

    /* an external low on BI/RBO blanks every output; released, they return */
    set(r, "RBI", LOGIC_1);
    settle(r);
    ok = shows(r, 0, GLYPH[1]);
    setc(r, 0, "BI/RBO", LOGIC_0);
    settle(r);
    ok &= shows(r, 0, BLANK) && dp_lit(r, 0) == 0 && qcode(r, 0) == 1;
    check(ok, "5 BI/RBO held low externally (RBI high, BI low): every segment blank");
    setc(r, 0, "BI/RBO", LOGIC_X);
    settle(r);
    segs(r, 0, s);
    ok = strcmp(s, "0xx0000") == 0;
    check(ok, "5 BI/RBO unknown: the lit segments of 1 (b, c) turn unknown, the dark ones stay released");
    setc(r, 0, "BI/RBO", LOGIC_Z);
    settle(r);
    ok = shows(r, 0, GLYPH[1]) && is(r, 0, "BI/RBO", LOGIC_Z);
    check(ok, "5 BI/RBO released again: shows 1");

    /* BI wins over everything and also with BI/RBO pulled low from outside */
    set(r, "BI", LOGIC_X);
    settle(r);
    segs(r, 0, s);
    ok = s[1] == 'x' && s[2] == 'x' && s[0] == '0' && s[6] == '0' && is(r, 0, "BI/RBO", LOGIC_X);
    check(ok, "5 BI unknown: lit segments b, c unknown, dark ones released, RBO unknown");
    rig_close(r);
}

/* Two digits (p.6): digit 0 = units, digit 1 = tens; common CLK, CLR, STRB; digit 0's MAX is digit 1's SCEI;
   digit 1's BI/RBO goes to digit 0's RBI (with a pull-up: RBO is open collector) and digit 1's RBI is low. */
static void test_cascade(void) {
    static const Join JOINS[] = {
        {0, "MAX", 1, "SCEI", 0},
        {1, "BI/RBO", 0, "RBI", 1},
    };
    Rig *r = rig(2, JOINS, 2);
    int ok = 1, n;

    inputs(r);
    setc(r, 1, "RBI", LOGIC_0);
    setc(r, 0, "SCEI", LOGIC_0);
    setc(r, 0, "RBI", LOGIC_Z);
    setc(r, 1, "SCEI", LOGIC_Z);
    setc(r, 1, "BI/RBO", LOGIC_Z);
    settle(r);
    clear(r);
    ok = shows(r, 0, BLANK) && shows(r, 1, BLANK) && is(r, 1, "BI/RBO", LOGIC_0) &&
         is(r, 0, "BI/RBO", LOGIC_0);
    check(ok, "6 two digits, 00: both blank by leading-zero blanking; both RBO low");
    for (n = 1; n <= 25; n++) {
        int units = n % 10, tens = n / 10;
        int g;

        pulse(r);
        g = qcode(r, 0) == units && qcode(r, 1) == tens && is(r, 0, "MAX", units == 9 ? LOGIC_0 : LOGIC_1) &&
            shows(r, 0, GLYPH[units]) && shows(r, 1, tens == 0 ? BLANK : GLYPH[tens]) &&
            is(r, 1, "SCEI", units == 9 ? LOGIC_0 : LOGIC_1) && is(r, 1, "BI/RBO", tens == 0 ? LOGIC_0 : LOGIC_1);
        if (!g) {
            printf("  count %d: units Q=%d tens Q=%d\n", n, qcode(r, 0), qcode(r, 1));
        }
        ok &= g;
    }
    check(ok, "6 two digits count 1..25: the tens digit steps once per 9 to 0 of the units (look-ahead), "
              "its leading zero blanks, the units keeps its 0 at 10 and 20");
    set(r, "STRB", LOGIC_1);
    settle(r);
    pulse(r);
    pulse(r);
    pulse(r);
    pulse(r);
    pulse(r);
    pulse(r);
    ok = qcode(r, 0) == 5 && qcode(r, 1) == 2 && shows(r, 0, GLYPH[5]) && shows(r, 1, GLYPH[2]);
    check(ok, "6 STRB on both digits holds the display (25) while they count");
    set(r, "STRB", LOGIC_0);
    settle(r);
    ok = qcode(r, 0) == 1 && qcode(r, 1) == 3 && shows(r, 0, GLYPH[1]) && shows(r, 1, GLYPH[3]);
    check(ok, "6 STRB low: both latches jump to the count (31)");
    rig_close(r);
}

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL loading the parts\n");
        return 1;
    }
    test_listing();
    test_power_on();
    test_count();
    test_enables();
    test_strobe();
    test_dp();
    test_blank();
    test_cascade();
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("counter_sn74143_test: all passed\n");
    return 0;
}
