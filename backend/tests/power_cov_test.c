/*
 * power_cov_test (plan-68-R): what regulator_test and timer_test do not
 * drive, on bundled parts.
 *
 * Model cases (a bench, tests/support/bench.h): a regulator's enable read X
 * makes its output and power-good X (an unknown cause, an unknown result);
 * an ICM7555 (timer.c header) in latch mode with RESET or THRES unknown, with
 * RESET low / unknown in astable mode, an unknown Q cleared by TRIG high in
 * one-shot mode, and a RESET that drops a pending period end.
 *
 * Analysis cases (circuit API; timer_rc.c steps 4-7): a supply on the timing
 * node, a capacitor from it to VCC, a reversed or an unrelated diode, a 0 ohm
 * timing resistor in circuit B and in the one-shot, values that round a time
 * to nothing, a one-shot pulse shorter than 2 tpd, and a pull-up of another
 * voltage as the one-shot's resistor. And timer_reason_text, the app's words.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "circuit.h"
#include "kind.h"
#include "parts.h"
#include "read_file.h"
#include "timer.h"
#include "timer_state.h"

static int failures;
static PartLibrary *library;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

#define NS 1000ull
#define US 1000000ull
#define MS 1000000000ull
#define OHM 1000ull
#define KOHM (1000ull * OHM)
#define FF_PER_NF 1000000ull

static Bench *bench_of(const char *name) {
    const Part *part = part_library_find(library, name);

    if (part == NULL) {
        printf("FAIL: no part %s\n", name);
        exit(1);
    }
    return bench_create(part);
}

/* ---- regulator ---------------------------------------------------------- */

static void regulator_enable_unknown(void) {
    Bench *b = bench_of("ADP1762ACPZ-1.0-R7");

    bench_set(b, "VIN", LOGIC_1);
    bench_set(b, "EN", LOGIC_1);
    bench_run(b, 10 * NS);
    expect(bench_get(b, "VOUT") == LOGIC_1, "ADP1762: VIN and EN high: VOUT on");
    bench_set(b, "EN", LOGIC_X);
    bench_run(b, 10 * NS);
    expect(bench_get(b, "VOUT") == LOGIC_X && bench_get(b, "PG") == LOGIC_X,
           "ADP1762: EN unknown: VOUT and PG unknown");
    bench_destroy(b);
}

/* ---- timer model -------------------------------------------------------- */

static Bench *timer_bench(TimerMode mode, SimTime first, SimTime high, SimTime low) {
    Bench *b = bench_of("ICM7555ESA+");
    TimerState t;

    memset(&t, 0, sizeof t);
    t.mode = mode;
    t.reason = TIMER_OK;
    t.first_ps = first;
    t.high_ps = high;
    t.low_ps = low;
    timer_model_set(bench_state(b), 0, &t);   /* before the start timer runs */
    bench_set(b, "VCC", LOGIC_1);
    bench_set(b, "GND", LOGIC_0);
    return b;
}

static void timer_latch_unknowns(void) {
    Bench *b = timer_bench(TIMER_LATCH, 0, 0, 0);

    bench_set(b, "RESET", LOGIC_1);
    bench_set(b, "THRES", LOGIC_0);
    bench_set(b, "TRIG", LOGIC_0);
    bench_run(b, 1 * US);
    expect(bench_get(b, "OUT") == LOGIC_1, "latch: TRIG low sets Q (OUT 1)");
    bench_set(b, "TRIG", LOGIC_1);
    bench_set(b, "THRES", LOGIC_X);
    bench_run(b, 1 * US);
    expect(bench_get(b, "OUT") == LOGIC_X, "latch: Q 1, TRIG high and THRES unknown: OUT unknown (maybe reset)");
    bench_set(b, "THRES", LOGIC_1);
    bench_run(b, 1 * US);
    expect(bench_get(b, "OUT") == LOGIC_0, "latch: THRES high resets Q");
    bench_set(b, "RESET", LOGIC_X);
    bench_run(b, 1 * US);
    expect(bench_get(b, "OUT") == LOGIC_X, "latch: RESET unknown: OUT unknown");
    bench_destroy(b);
}

static void timer_astable_reset(void) {
    Bench *b = timer_bench(TIMER_ASTABLE, 1 * US, 1 * US, 1 * US);

    bench_set(b, "RESET", LOGIC_1);
    bench_run(b, 500 * NS);
    expect(bench_get(b, "OUT") == LOGIC_1, "astable: Q high in the first period");
    bench_set(b, "RESET", LOGIC_0);
    bench_run(b, 200 * NS);
    expect(bench_get(b, "OUT") == LOGIC_0, "astable: RESET low holds Q at 0");
    bench_set(b, "RESET", LOGIC_1);        /* at 700 ns: a new cycle, high from 740 ns for 1 us */
    bench_run(b, 500 * NS);
    expect(bench_get(b, "OUT") == LOGIC_1,
           "astable: the period end the RESET dropped (due at 1 us) does not end the new high phase (1.2 us)");
    bench_run(b, 700 * NS);
    expect(bench_get(b, "OUT") == LOGIC_0, "astable: the new high phase ends 1 us after it began (at 1.74 us)");
    bench_set(b, "RESET", LOGIC_X);
    bench_run(b, 200 * NS);
    expect(bench_get(b, "OUT") == LOGIC_X, "astable: RESET unknown: OUT unknown");
    bench_destroy(b);
}

static void timer_one_shot_unknown(void) {
    Bench *b = timer_bench(TIMER_ONE_SHOT, 0, 2 * US, 0);

    bench_set(b, "TRIG", LOGIC_1);
    bench_set(b, "RESET", LOGIC_X);
    bench_run(b, 500 * NS);
    expect(bench_get(b, "OUT") == LOGIC_X, "one-shot: RESET unknown: Q unknown");
    bench_set(b, "RESET", LOGIC_1);
    bench_run(b, 500 * NS);
    expect(bench_get(b, "OUT") == LOGIC_0, "one-shot: RESET released with TRIG high: the unknown Q clears to 0");
    bench_set(b, "TRIG", LOGIC_0);
    bench_run(b, 100 * NS);
    bench_set(b, "TRIG", LOGIC_1);
    bench_run(b, 1 * US);
    expect(bench_get(b, "OUT") == LOGIC_1, "one-shot: a TRIG pulse then starts the 2 us pulse");
    bench_destroy(b);
}

static void timer_model_calls(void) {
    Bench *b = timer_bench(TIMER_LATCH, 0, 0, 0);
    TimerState s;

    expect(timer_model_get(NULL, 0, &s) == 0, "timer_model_get(NULL) is 0");
    expect(timer_model_get(bench_state(b), 1, &s) == 0, "timer_model_get: a 555 has no section 1");
    expect(timer_model_get(bench_state(b), 0, &s) == 1 && s.mode == TIMER_LATCH, "timer_model_get: section 0, latch");
    bench_destroy(b);
}

/* ---- the analysis ------------------------------------------------------- */

static size_t add(Circuit *c, const char *name, uint64_t value) {
    size_t index = 0;

    if (circuit_add_part(c, name, &index) != 0 || (value != 0 && circuit_set_part_value(c, index, value) != 0)) {
        fprintf(stderr, "add %s: %s\n", name, circuit_error(c));
        exit(1);
    }
    return index;
}

static size_t net(Circuit *c) {
    size_t index = 0;

    circuit_add_net(c, &index);
    return index;
}

static void wire(Circuit *c, size_t part, const char *pin, size_t n) {
    if (circuit_connect(c, part, pin, n) != 0) {
        fprintf(stderr, "connect %s: %s\n", pin, circuit_error(c));
        exit(1);
    }
}

static void two(Circuit *c, size_t part, size_t a, size_t b) {
    wire(c, part, "1", a);
    wire(c, part, "2", b);
}

/* An ICM7555 on 5 V: RESET high, OUT on its own net; TRIG, THRES and DISCH left to the case. */
typedef struct { Circuit *c; size_t timer, vcc, gnd, out; } Rig;

static Rig rig(void) {
    Rig r;

    r.c = circuit_create(library);
    r.timer = add(r.c, "ICM7555ESA+", 0);
    r.vcc = net(r.c);
    r.gnd = net(r.c);
    r.out = net(r.c);
    wire(r.c, add(r.c, "5V", 0), "1", r.vcc);
    wire(r.c, add(r.c, "GND", 0), "1", r.gnd);
    wire(r.c, r.timer, "8", r.vcc);
    wire(r.c, r.timer, "1", r.gnd);
    wire(r.c, r.timer, "4", r.vcc);
    wire(r.c, r.timer, "3", r.out);
    return r;
}

/* Circuit A: RA vcc-D, RB D-C, C to ground; TRIG and THRES on C. */
static Rig circuit_a(uint64_t ra, uint64_t rb, uint64_t cap, size_t *node, size_t *d) {
    Rig r = rig();

    *node = net(r.c);
    *d = net(r.c);
    wire(r.c, r.timer, "2", *node);
    wire(r.c, r.timer, "6", *node);
    wire(r.c, r.timer, "7", *d);
    two(r.c, add(r.c, "GENERIC-RESISTOR", ra), r.vcc, *d);
    two(r.c, add(r.c, "GENERIC-RESISTOR", rb), *d, *node);
    two(r.c, add(r.c, "GENERIC-CAPACITOR", cap), *node, r.gnd);
    return r;
}

/* The one-shot M: THRES and DISCH on C, R from `rail` to C, C to ground, TRIG pulled up. */
static Rig one_shot(const char *resistor, uint64_t r_value, uint64_t cap) {
    Rig r = rig();
    size_t node = net(r.c), trig = net(r.c);

    wire(r.c, r.timer, "6", node);
    wire(r.c, r.timer, "7", node);
    wire(r.c, r.timer, "2", trig);
    two(r.c, add(r.c, "GENERIC-RESISTOR", 10 * KOHM), r.vcc, trig);
    if (resistor != NULL) {
        two(r.c, add(r.c, resistor, r_value), r.vcc, node);
    }
    two(r.c, add(r.c, "GENERIC-CAPACITOR", cap), node, r.gnd);
    return r;
}

static int stopped(Rig r, TimerReason reason) {
    TimerState s;
    int ok = circuit_start(r.c) == 0 && circuit_timer_state(r.c, r.timer, 0, &s) == 0 &&
             s.mode == TIMER_STOPPED && s.reason == reason;

    circuit_destroy(r.c);
    return ok;
}

static void analysis_cases(void) {
    size_t node, d;
    Rig r;
    TimerState s;

    r = circuit_a(1 * KOHM, 10 * KOHM, 100 * FF_PER_NF, &node, &d);
    wire(r.c, add(r.c, "5V", 0), "1", node);
    expect(stopped(r, TIMER_SHAPE), "circuit A with a 5 V supply on C: shape (C is driven)");

    r = circuit_a(1 * KOHM, 10 * KOHM, 100 * FF_PER_NF, &node, &d);
    two(r.c, add(r.c, "GENERIC-CAPACITOR", 10 * FF_PER_NF), node, r.vcc);
    expect(stopped(r, TIMER_SHAPE), "circuit A with a second capacitor from C to VCC: shape");

    r = circuit_a(1 * KOHM, 10 * KOHM, 100 * FF_PER_NF, &node, &d);
    two(r.c, add(r.c, "1N4148-TAP", 0), d, node);          /* pin 1 K on D, pin 2 A on C: reversed */
    expect(stopped(r, TIMER_SHAPE), "circuit A with a diode from C to D (reversed): shape");

    r = circuit_a(1 * KOHM, 10 * KOHM, 100 * FF_PER_NF, &node, &d);
    two(r.c, add(r.c, "1N4148-TAP", 0), r.out, r.gnd);     /* on neither C nor D */
    expect(circuit_start(r.c) == 0 && circuit_timer_state(r.c, r.timer, 0, &s) == 0 &&
               s.mode == TIMER_ASTABLE && s.reason == TIMER_OK,
           "circuit A with a diode on OUT (neither C nor D): still astable");
    circuit_destroy(r.c);

    /* circuit B: a 0 ohm resistor from OUT to C */
    r = rig();
    node = net(r.c);
    wire(r.c, r.timer, "2", node);
    wire(r.c, r.timer, "6", node);
    wire(r.c, r.timer, "7", net(r.c));
    two(r.c, add(r.c, "CRCW06030000Z0EBC", 0), r.out, node);
    two(r.c, add(r.c, "GENERIC-CAPACITOR", 0), node, r.gnd);
    expect(stopped(r, TIMER_NO_VALUE), "circuit B with a 0 ohm jumper from OUT to C: no value");

    expect(stopped(one_shot("CRCW06030000Z0EBC", 0, 100 * FF_PER_NF), TIMER_NO_VALUE),
           "one-shot with a 0 ohm jumper from VCC to C: no value");

    /* 1 ohm, 1 ohm, 1 fF: 0.69 x 1000 mOhm x 1 fF rounds to 0 ps */
    r = circuit_a(1 * OHM, 1 * OHM, 1, &node, &d);
    expect(stopped(r, TIMER_NO_VALUE), "circuit A of 1 ohm, 1 ohm and 1 fF: the times round to 0, no value");

    /* 1 k and 10 fF: a pulse of 1.1 x 1 k x 10 fF = 11 ps, under 2 tpd (80 ns) */
    expect(stopped(one_shot("GENERIC-RESISTOR", 1 * KOHM, 10), TIMER_TOO_FAST),
           "one-shot of 1 k and 10 fF (11 ps): shorter than 2 tpd, too fast");

    /* the one-shot's resistor is a 3.3 V pull-up on a 5 V timer: a pull-up of another voltage is no
       supply (timer_rc.c), so C has no resistor to the supply and the parts form no circuit */
    {
        Rig q = rig();
        size_t c_node = net(q.c), trig = net(q.c);

        wire(q.c, q.timer, "6", c_node);
        wire(q.c, q.timer, "7", c_node);
        wire(q.c, q.timer, "2", trig);
        two(q.c, add(q.c, "GENERIC-RESISTOR", 10 * KOHM), q.vcc, trig);
        wire(q.c, add(q.c, "PULLUP-3V3", 0), "1", c_node);
        two(q.c, add(q.c, "GENERIC-CAPACITOR", 100 * FF_PER_NF), c_node, q.gnd);
        expect(stopped(q, TIMER_SHAPE), "one-shot whose resistor is a 3.3 V pull-up on a 5 V timer: shape");
    }
}

static void reason_texts(void) {
    expect(strcmp(timer_reason_text(TIMER_OK), "") == 0, "timer_reason_text(TIMER_OK) is empty");
    expect(strstr(timer_reason_text(TIMER_NO_SUPPLY), "VCC and GND") != NULL, "TIMER_NO_SUPPLY names VCC and GND");
    expect(strstr(timer_reason_text(TIMER_CONTROL), "CONT") != NULL, "TIMER_CONTROL names the CONT pin");
    expect(strstr(timer_reason_text(TIMER_SHAPE), "astable or a one-shot") != NULL,
           "TIMER_SHAPE says the parts form no astable or one-shot");
    expect(strstr(timer_reason_text(TIMER_NO_VALUE), "no value") != NULL, "TIMER_NO_VALUE says a part has no value");
    expect(strstr(timer_reason_text(TIMER_TOO_FAST), "faster") != NULL, "TIMER_TOO_FAST says the timing is too fast");
}

int main(void) {
    setvbuf(stdout, NULL, _IOLBF, 0);   /* the leak check ends with _exit: keep the lines */
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    regulator_enable_unknown();
    timer_latch_unknowns();
    timer_astable_reset();
    timer_one_shot_unknown();
    timer_model_calls();
    analysis_cases();
    reason_texts();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("power_cov_test: all passed\n");
    return 0;
}
