/*
 * timer_test (plan-48-E): the timer kind on fixture parts next to the real
 * part files (support/timer_fixture.h). Cases (a)-(f): the facts and the
 * loader's rules; (g)-(j): latch mode, a timer with no capacitor on its
 * timing pins; (k)-(z): the timing analysis and the astable, one-shot and
 * stopped modes (cases of plan-48-E section 6).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "kind.h"
#include "model_pins.h"
#include "parts.h"
#include "read_file.h"
#include "timer_fixture.h"
#include "timer_state.h"

static int failures;
static PartLibrary *library;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

static int same(const char *a, const char *b) {
    return a != NULL && b != NULL && strcmp(a, b) == 0;
}

/* ---- facts and the loader (cases a-f) ------------------------------------ */

static void fact_cases(void) {
    const Part *t = part_library_find(library, "TESTTIMER");
    const Part *t2 = part_library_find(library, "TESTTIMER2");

    expect(t != NULL && t->kind == PART_TIMER && t->timer != NULL && t->pin_count == 8 &&
               t->timer->timer_count == 1 && same(t->timer->process, "bipolar") &&
               t->timer->max_hz == 500000 && t->timer->tpd_ps == 100000 && t->vcc != NULL &&
               t->vcc->min_mv == 4500 && t->vcc->max_mv == 16000 && part_pin_open_drain(t, 6) == 1 &&
               part_pin_open_drain(t, 2) == 0 && part_pin_analog(t, 4) == 1 && part_pin_analog(t, 5) == 0,
           "(a) TESTTIMER: kind timer, 8 pins, 1 section, bipolar, 500000 Hz, tpd 100000 ps, 4500-16000 mV; "
           "pin 7 open drain, pin 5 analog");
    expect(t2 != NULL && t2->kind == PART_TIMER && t2->pin_count == 14 && t2->timer->timer_count == 2 &&
               same(t2->timer->process, "cmos") && part_pin_open_drain(t2, 0) == 1 &&
               part_pin_open_drain(t2, 12) == 1 && part_pin_analog(t2, 2) == 1 && part_pin_analog(t2, 10) == 1,
           "(b) TESTTIMER2: 2 sections, cmos, 14 pins, both discharge pins open drain, both control pins analog");
}

static void refused(const char *what, const char *json, const char *want) {
    PartLibrary *lib = part_library_create();
    char *error = NULL;
    int ok = lib != NULL && load_parts_dir(lib, PARTS_DIR) == 0 &&
             load_parts(lib, json, "bad-timer.json", &error) != 0 && error != NULL &&
             strstr(error, want) != NULL;

    if (!ok) {
        printf("  error: %s\n", error != NULL ? error : "(none)");
    }
    expect(ok, what);
    free(error);
    part_library_destroy(lib);
}

#define OKKEYS ",\"maxHz\":500000,\"tpd\":100,\"vcc\":[4.5,16]"
#define BAD(PROCESS, TIMERS, PINS, KEYS) TIMER_FILE(TIMER_DEVICE("bad", PROCESS, TIMERS, PINS, KEYS, "BAD", "PDIP-8"))

static void rejection_cases(void) {
    refused("(c) process nmos: refused", BAD("nmos", TFX_SEC1, TFX_P8, OKKEYS), "unknown timer process \"nmos\"");
    refused("(c) no \"timers\": refused",
            TIMER_FILE("{\"id\":\"bad\",\"function\":\"f\",\"description\":\"d\",\"drawer\":[],"
                       "\"pinouts\":{\"p\":[" TFX_P8 "]},\"process\":\"cmos\",\"maxHz\":1,\"tpd\":1,"
                       "\"vcc\":[1,2],\"parts\":[{\"name\":\"BAD\",\"package\":\"DIP\",\"pinout\":\"p\","
                       "\"footprint\":\"PDIP-8\"}]}"),
            "missing \"timers\"");
    refused("(c) three sections: refused", BAD("cmos", TFX_SEC1 "," TFX_SEC1 "," TFX_SEC1, TFX_P8, OKKEYS),
            "\"timers\" must have 1 or 2 entries");
    refused("(d) a section without \"trigger\": refused",
            BAD("cmos", "{\"threshold\":\"THRES\",\"discharge\":\"DISCH\",\"output\":\"OUT\"}", TFX_P8, OKKEYS),
            "missing \"trigger\"");
    refused("(d) a section naming a missing pin: refused",
            BAD("cmos",
                "{\"trigger\":\"NOPE\",\"threshold\":\"THRES\",\"discharge\":\"DISCH\",\"output\":\"OUT\"}",
                TFX_P8, OKKEYS),
            "NOPE");
    refused("(d) two sections naming the same pin: refused", BAD("cmos", TFX_SEC1 "," TFX_SEC1, TFX_P8, OKKEYS),
            "timer pin \"TRIG\" is named twice");
    refused("(e) a trigger pin with role output: refused",
            BAD("cmos", TFX_SEC1,
                TFX_PIN(1, "GND", "ground") "," TFX_PIN(2, "TRIG", "output") "," TFX_PIN(3, "OUT", "output") ","
                TFX_PIN(4, "RESET", "input") "," TFX_PIN(5, "CONT", "input") "," TFX_PIN(6, "THRES", "input") ","
                TFX_PIN(7, "DISCH", "output") "," TFX_PIN(8, "VCC", "power"),
                OKKEYS),
            "timer pin \"TRIG\" must be an input");
    refused("(e) an output pin with role input: refused",
            BAD("cmos", TFX_SEC1,
                TFX_PIN(1, "GND", "ground") "," TFX_PIN(2, "TRIG", "input") "," TFX_PIN(3, "OUT", "input") ","
                TFX_PIN(4, "RESET", "input") "," TFX_PIN(5, "CONT", "input") "," TFX_PIN(6, "THRES", "input") ","
                TFX_PIN(7, "DISCH", "output") "," TFX_PIN(8, "VCC", "power"),
                OKKEYS),
            "timer pin \"OUT\" must be an output");
    refused("(e) two power pins: refused",
            BAD("cmos", TFX_SEC1,
                TFX_PIN(1, "GND", "power") "," TFX_PIN(2, "TRIG", "input") "," TFX_PIN(3, "OUT", "output") ","
                TFX_PIN(4, "RESET", "input") "," TFX_PIN(5, "CONT", "input") "," TFX_PIN(6, "THRES", "input") ","
                TFX_PIN(7, "DISCH", "output") "," TFX_PIN(8, "VCC", "power"),
                OKKEYS),
            "a timer has one power pin and one ground pin");
    refused("(f) maxHz 0: refused", BAD("cmos", TFX_SEC1, TFX_P8, ",\"maxHz\":0,\"tpd\":100,\"vcc\":[4.5,16]"),
            "\"maxHz\" must be a whole number from 1 to 100000000");
    refused("(f) tpd 0: refused", BAD("cmos", TFX_SEC1, TFX_P8, ",\"maxHz\":1,\"tpd\":0,\"vcc\":[4.5,16]"),
            "\"tpd\" must be above 0");
    refused("(f) an unlisted part key: refused",
            "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"timer\",\"devices\":[{\"id\":\"bad\","
            "\"function\":\"f\",\"description\":\"d\",\"drawer\":[],\"pinouts\":{\"p\":[" TFX_P8 "]},"
            "\"process\":\"cmos\",\"timers\":[" TFX_SEC1 "]" OKKEYS ",\"parts\":[{\"name\":\"BAD\","
            "\"package\":\"DIP\",\"pinout\":\"p\",\"footprint\":\"PDIP-8\",\"ohms\":5}]}]}",
            "unknown key \"ohms\"");
}

/* ---- circuit helpers ---------------------------------------------------- */

static size_t add(Circuit *c, const char *name) {
    size_t index = 0;

    if (circuit_add_part(c, name, &index) != 0) {
        fprintf(stderr, "add %s: %s\n", name, circuit_error(c));
        exit(1);
    }
    return index;
}

static size_t net(Circuit *c) {
    size_t index = 0;

    if (circuit_add_net(c, &index) != 0) {
        fprintf(stderr, "net: %s\n", circuit_error(c));
        exit(1);
    }
    return index;
}

static void wire(Circuit *c, size_t part, const char *pin, size_t n) {
    if (circuit_connect(c, part, pin, n) != 0) {
        fprintf(stderr, "connect %s: %s\n", pin, circuit_error(c));
        exit(1);
    }
}

/* A rig: a timer part, a 5 V net and a GND net, and nets for its output(s). */
typedef struct {
    Circuit *c;
    size_t timer, vcc, gnd;
} Rig;

static Rig rig(const char *timer, const char *vcc_pin, const char *gnd_pin) {
    Rig r;

    r.c = circuit_create(library);
    r.timer = add(r.c, timer);
    r.vcc = net(r.c);
    r.gnd = net(r.c);
    wire(r.c, add(r.c, "5V"), "1", r.vcc);
    wire(r.c, add(r.c, "GND"), "1", r.gnd);
    wire(r.c, r.timer, vcc_pin, r.vcc);
    wire(r.c, r.timer, gnd_pin, r.gnd);
    return r;
}

static void high(Rig *r, const char *pin) {
    wire(r->c, r->timer, pin, r->vcc);
}

static void low(Rig *r, const char *pin) {
    wire(r->c, r->timer, pin, r->gnd);
}

/* A net on `pin`. */
static size_t probe(Rig *r, const char *pin) {
    size_t n = net(r->c);

    wire(r->c, r->timer, pin, n);
    return n;
}

static size_t pulled(Rig *r, const char *pin) {
    size_t n = probe(r, pin);

    wire(r->c, add(r->c, "PULLUP-5V"), "1", n);
    return n;
}

static int value(Circuit *c, size_t n) {
    switch (circuit_net_value(c, n)) {
    case CIRCUIT_0: return 0;
    case CIRCUIT_1: return 1;
    case CIRCUIT_X: return 2;
    default: return 3;
    }
}

static int at(Circuit *c, uint64_t t, size_t n) {
    if (t > circuit_time(c) && circuit_advance(c, t - circuit_time(c)) != 1) {
        return -1;
    }
    return value(c, n);
}

static int state_is(Circuit *c, size_t part, unsigned section, TimerMode mode, TimerReason reason) {
    TimerState s;

    return circuit_timer_state(c, part, section, &s) == 0 && s.mode == mode && s.reason == reason;
}

/* ---- latch mode (cases g-j) --------------------------------------------- */

static void latch_cases(void) {
    Rig r;
    size_t out, disch;

    r = rig("TESTTIMER", "8", "1");
    low(&r, "2");
    low(&r, "6");
    high(&r, "4");
    out = probe(&r, "3");
    disch = pulled(&r, "7");
    expect(circuit_start(r.c) == 0 && at(r.c, 1000000, out) == 1 && value(r.c, disch) == 1 &&
               state_is(r.c, r.timer, 0, TIMER_LATCH, TIMER_OK),
           "(g) TRIG and THRES low, RESET high: OUT 1, DISCH released (pulled up to 1); mode latch, ok");
    circuit_destroy(r.c);

    r = rig("TESTTIMER", "8", "1");
    high(&r, "2");
    high(&r, "6");
    high(&r, "4");
    out = probe(&r, "3");
    disch = pulled(&r, "7");
    expect(circuit_start(r.c) == 0 && at(r.c, 1000000, out) == 0 && value(r.c, disch) == 0,
           "(h) TRIG and THRES high: OUT 0, DISCH pulled to 0");
    circuit_destroy(r.c);

    r = rig("TESTTIMER", "8", "1");
    low(&r, "2");
    high(&r, "6");
    high(&r, "4");
    out = probe(&r, "3");
    expect(circuit_start(r.c) == 0 && at(r.c, 1000000, out) == 1, "(i) TRIG low, THRES high: OUT 1 (trigger wins)");
    circuit_destroy(r.c);

    r = rig("TESTTIMER", "8", "1");
    low(&r, "2");
    low(&r, "6");
    low(&r, "4");
    out = probe(&r, "3");
    expect(circuit_start(r.c) == 0 && at(r.c, 1000000, out) == 0, "(j) as (g) with RESET low: OUT 0");
    circuit_destroy(r.c);

    r = rig("TESTTIMER2", "14", "7");
    low(&r, "6");
    low(&r, "2");
    high(&r, "4");
    high(&r, "8");
    high(&r, "12");
    high(&r, "10");
    out = probe(&r, "5");
    {
        size_t out2 = probe(&r, "9");

        expect(circuit_start(r.c) == 0 && at(r.c, 1000000, out) == 1 && value(r.c, out2) == 0 &&
                   state_is(r.c, r.timer, 0, TIMER_LATCH, TIMER_OK) &&
                   state_is(r.c, r.timer, 1, TIMER_LATCH, TIMER_OK),
               "(j) TESTTIMER2: section 1 as (g), section 2 as (h): OUT1 1, OUT2 0");
    }
    circuit_destroy(r.c);
}

/* ---- the timing circuits (cases k-z) ---------------------------------------- */

#define OHM 1000ULL                  /* milliohms per ohm */
#define KOHM (1000ULL * OHM)
#define FF_PER_NF 1000000ULL         /* femtofarads per nanofarad */
#define MS 1000000000ULL             /* picoseconds per millisecond */

static size_t value_part(Circuit *c, const char *name, uint64_t value) {
    size_t p = add(c, name);

    if (value != 0 && circuit_set_part_value(c, p, value) != 0) {
        fprintf(stderr, "set value of %s: %s\n", name, circuit_error(c));
        exit(1);
    }
    return p;
}

static void two(Circuit *c, size_t part, size_t a, size_t b) {
    wire(c, part, "1", a);
    wire(c, part, "2", b);
}

/* Circuit A of plan-48-E section 4 on TESTTIMER: RA from the supply to D, RB from D to C,
   a capacitor from C to GND, TRIG and THRES on C, RESET on the supply, CONT unwired. */
typedef struct { Circuit *c; size_t timer, vcc, gnd, d, node, out; } CircuitA;

static CircuitA circuit_a(uint64_t ra, uint64_t rb, uint64_t cap) {
    CircuitA a;

    a.c = circuit_create(library);
    a.timer = add(a.c, "TESTTIMER");
    a.vcc = net(a.c);
    a.gnd = net(a.c);
    a.d = net(a.c);
    a.node = net(a.c);
    a.out = net(a.c);
    wire(a.c, add(a.c, "5V"), "1", a.vcc);
    wire(a.c, add(a.c, "GND"), "1", a.gnd);
    wire(a.c, a.timer, "8", a.vcc);
    wire(a.c, a.timer, "1", a.gnd);
    wire(a.c, a.timer, "4", a.vcc);
    wire(a.c, a.timer, "2", a.node);
    wire(a.c, a.timer, "6", a.node);
    wire(a.c, a.timer, "7", a.d);
    wire(a.c, a.timer, "3", a.out);
    two(a.c, value_part(a.c, "GENERIC-RESISTOR", ra), a.vcc, a.d);
    two(a.c, value_part(a.c, "GENERIC-RESISTOR", rb), a.d, a.node);
    two(a.c, value_part(a.c, "GENERIC-CAPACITOR", cap), a.node, a.gnd);
    return a;
}

static int near(uint64_t got, uint64_t want) {
    return got + 1 >= want && got <= want + 1;
}

static int times_are(Circuit *c, size_t part, unsigned section, TimerMode mode, uint64_t first, uint64_t high,
                     uint64_t low) {
    TimerState s;

    return circuit_timer_state(c, part, section, &s) == 0 && s.mode == mode && s.reason == TIMER_OK &&
           near(s.first_ps, first) && near(s.high_ps, high) && near(s.low_ps, low);
}

#define A_FIRST 1208473518ULL
#define A_HIGH 762461899ULL
#define A_LOW 693147181ULL

static void astable_cases(void) {
    CircuitA a;
    size_t p;
    int ok;

    a = circuit_a(1 * KOHM, 10 * KOHM, 100 * FF_PER_NF);
    ok = circuit_start(a.c) == 0 && times_are(a.c, a.timer, 0, TIMER_ASTABLE, A_FIRST, A_HIGH, A_LOW);
    expect(ok, "(k) circuit A, 1 k + 10 k, 100 nF: astable, first 1208473518, high 762461899, low 693147181 ps");
    ok = at(a.c, 1 * MS, a.out) == 1 && at(a.c, 1500000000ULL, a.out) == 0 &&
         at(a.c, 2200000000ULL, a.out) == 1 && at(a.c, 2900000000ULL, a.out) == 0;
    expect(ok, "(l) OUT 1 at 1 ms, 0 at 1.5 ms, 1 at 2.2 ms, 0 at 2.9 ms");
    ok = circuit_advance(a.c, 20 * MS - circuit_time(a.c)) == 1 && circuit_net_changes(a.c, a.out) >= 25 &&
         circuit_net_changes(a.c, a.out) <= 27;
    if (!ok) {
        printf("  OUT changed %llu times\n", (unsigned long long)circuit_net_changes(a.c, a.out));
    }
    expect(ok, "(l) OUT changes 26 +- 1 times in 20 ms");
    circuit_destroy(a.c);

    /* (m) RA as a PULLUP-5V part on D, the capacitor's ground on a second GND part's net */
    a = circuit_a(1 * KOHM, 10 * KOHM, 100 * FF_PER_NF);
    circuit_destroy(a.c);
    {
        Circuit *c = circuit_create(library);
        size_t timer = add(c, "TESTTIMER"), vcc = net(c), gnd = net(c), gnd2 = net(c), d = net(c), node = net(c);

        wire(c, add(c, "5V"), "1", vcc);
        wire(c, add(c, "GND"), "1", gnd);
        wire(c, add(c, "GND"), "1", gnd2);
        wire(c, timer, "8", vcc);
        wire(c, timer, "1", gnd);
        wire(c, timer, "4", vcc);
        wire(c, timer, "2", node);
        wire(c, timer, "6", node);
        wire(c, timer, "7", d);
        wire(c, value_part(c, "PULLUP-5V", 1 * KOHM), "1", d);
        two(c, value_part(c, "GENERIC-RESISTOR", 10 * KOHM), d, node);
        two(c, value_part(c, "GENERIC-CAPACITOR", 100 * FF_PER_NF), node, gnd2);
        expect(circuit_start(c) == 0 && times_are(c, timer, 0, TIMER_ASTABLE, A_FIRST, A_HIGH, A_LOW),
               "(m) RA a 1 k PULLUP-5V on D, the capacitor to a second GND net: the times of (k)");
        circuit_destroy(c);
    }
    /* (n) two 20 k in parallel as RB, 47 nF + 53 nF */
    {
        Circuit *c = circuit_create(library);
        size_t timer = add(c, "TESTTIMER"), vcc = net(c), gnd = net(c), d = net(c), node = net(c);

        wire(c, add(c, "5V"), "1", vcc);
        wire(c, add(c, "GND"), "1", gnd);
        wire(c, timer, "8", vcc);
        wire(c, timer, "1", gnd);
        wire(c, timer, "4", vcc);
        wire(c, timer, "2", node);
        wire(c, timer, "6", node);
        wire(c, timer, "7", d);
        two(c, value_part(c, "GENERIC-RESISTOR", 1 * KOHM), vcc, d);
        two(c, value_part(c, "GENERIC-RESISTOR", 20 * KOHM), d, node);
        two(c, value_part(c, "GENERIC-RESISTOR", 20 * KOHM), d, node);
        two(c, value_part(c, "GENERIC-CAPACITOR", 47 * FF_PER_NF), node, gnd);
        two(c, value_part(c, "GENERIC-CAPACITOR", 53 * FF_PER_NF), node, gnd);
        expect(circuit_start(c) == 0 && times_are(c, timer, 0, TIMER_ASTABLE, A_FIRST, A_HIGH, A_LOW),
               "(n) RB two 20 k in parallel, C 47 nF + 53 nF: the times of (k)");
        circuit_destroy(c);
    }
    /* (o) RESET low: OUT 0 for ever, mode still astable */
    {
        Circuit *c = circuit_create(library);
        size_t timer = add(c, "TESTTIMER"), vcc = net(c), gnd = net(c), d = net(c), node = net(c), out = net(c);

        wire(c, add(c, "5V"), "1", vcc);
        wire(c, add(c, "GND"), "1", gnd);
        wire(c, timer, "8", vcc);
        wire(c, timer, "1", gnd);
        wire(c, timer, "4", gnd);
        wire(c, timer, "2", node);
        wire(c, timer, "6", node);
        wire(c, timer, "7", d);
        wire(c, timer, "3", out);
        two(c, value_part(c, "GENERIC-RESISTOR", 1 * KOHM), vcc, d);
        two(c, value_part(c, "GENERIC-RESISTOR", 10 * KOHM), d, node);
        two(c, value_part(c, "GENERIC-CAPACITOR", 100 * FF_PER_NF), node, gnd);
        ok = circuit_start(c) == 0 && state_is(c, timer, 0, TIMER_ASTABLE, TIMER_OK);
        ok = ok && at(c, 1 * MS, out) == 0 && at(c, 3 * MS, out) == 0 && circuit_net_changes(c, out) <= 1;
        if (!ok) {
            printf("  OUT %d, %llu changes\n", value(c, out), (unsigned long long)circuit_net_changes(c, out));
        }
        expect(ok, "(o) RESET low: OUT 0 at every time, only the first drive as a change, mode still astable");
        circuit_destroy(c);
    }
    /* (p) A-diode: a 1N4148 with its anode on D and its cathode on C */
    a = circuit_a(1 * KOHM, 10 * KOHM, 100 * FF_PER_NF);
    p = add(a.c, "1N4148");
    wire(a.c, p, "2", a.d);      /* A */
    wire(a.c, p, "1", a.node);   /* K */
    expect(circuit_start(a.c) == 0 && times_are(a.c, a.timer, 0, TIMER_ASTABLE, 109861229ULL, 69314718ULL, A_LOW),
           "(p) circuit A with a 1N4148 from D to C: first 109861229, high 69314718, low 693147181 ps");
    circuit_destroy(a.c);
    /* (q) circuit B on TESTTIMER2 section 2 */
    {
        Circuit *c = circuit_create(library);
        size_t timer = add(c, "TESTTIMER2"), vcc = net(c), gnd = net(c), node = net(c), out = net(c);

        wire(c, add(c, "5V"), "1", vcc);
        wire(c, add(c, "GND"), "1", gnd);
        wire(c, timer, "14", vcc);
        wire(c, timer, "7", gnd);
        wire(c, timer, "10", vcc);
        wire(c, timer, "8", node);
        wire(c, timer, "12", node);
        wire(c, timer, "9", out);
        two(c, value_part(c, "GENERIC-RESISTOR", 10 * KOHM), node, out);
        two(c, value_part(c, "GENERIC-CAPACITOR", 100 * FF_PER_NF), node, gnd);
        ok = circuit_start(c) == 0 && times_are(c, timer, 1, TIMER_ASTABLE, 1098612289ULL, A_LOW, A_LOW) &&
             state_is(c, timer, 0, TIMER_LATCH, TIMER_OK);
        expect(ok, "(q) circuit B on TESTTIMER2 section 2: high = low = 693147181, first 1098612289; section 1 stays a latch");
        circuit_destroy(c);
    }
}

/* Circuit M: 10 k from the supply to C, 100 nF from C to GND, THRES and DISCH on C, TRIG on a
   pulled-up net with a button to GND. */
typedef struct { Circuit *c; size_t timer, out, button; } OneShot;

static OneShot one_shot(void) {
    OneShot m;
    size_t vcc, gnd, node, trig;

    m.c = circuit_create(library);
    m.timer = add(m.c, "TESTTIMER");
    vcc = net(m.c);
    gnd = net(m.c);
    node = net(m.c);
    trig = net(m.c);
    m.out = net(m.c);
    wire(m.c, add(m.c, "5V"), "1", vcc);
    wire(m.c, add(m.c, "GND"), "1", gnd);
    wire(m.c, m.timer, "8", vcc);
    wire(m.c, m.timer, "1", gnd);
    wire(m.c, m.timer, "4", vcc);
    wire(m.c, m.timer, "6", node);
    wire(m.c, m.timer, "7", node);
    wire(m.c, m.timer, "2", trig);
    wire(m.c, m.timer, "3", m.out);
    two(m.c, value_part(m.c, "GENERIC-RESISTOR", 10 * KOHM), vcc, node);
    two(m.c, value_part(m.c, "GENERIC-CAPACITOR", 100 * FF_PER_NF), node, gnd);
    wire(m.c, add(m.c, "PULLUP-5V"), "1", trig);
    m.button = add(m.c, "TESTTACT");
    wire(m.c, m.button, "1", trig);
    wire(m.c, m.button, "2", gnd);
    return m;
}

static void press(OneShot *m, int down) {
    if (circuit_set_switch(m->c, m->button, 1, down) != 0) {
        fprintf(stderr, "switch: %s\n", circuit_error(m->c));
        exit(1);
    }
}

static void one_shot_cases(void) {
    OneShot m = one_shot();
    int ok;

    ok = circuit_start(m.c) == 0 && times_are(m.c, m.timer, 0, TIMER_ONE_SHOT, 0, 1098612289ULL, 0) &&
         at(m.c, 1 * MS, m.out) == 0;
    press(&m, 1);
    ok = ok && circuit_advance(m.c, 100000000ULL) == 1;
    press(&m, 0);
    ok = ok && at(m.c, 1500000000ULL, m.out) == 1 && at(m.c, 2200000000ULL, m.out) == 0;
    expect(ok, "(r) one-shot: 1098612289 ps; a press at 1 ms gives OUT 1 at 1.5 ms, 0 at 2.2 ms (ends at 2.0987 ms)");
    circuit_destroy(m.c);

    m = one_shot();
    ok = circuit_start(m.c) == 0 && at(m.c, 1 * MS, m.out) == 0;
    press(&m, 1);
    ok = ok && at(m.c, 2500000000ULL, m.out) == 1 && circuit_advance(m.c, 500000000ULL) == 1;
    press(&m, 0);
    ok = ok && at(m.c, 3100000000ULL, m.out) == 0;
    expect(ok, "(s) the button held from 1 ms to 3 ms: OUT 1 at 2.5 ms (TRIG still low), 0 at 3.1 ms");
    circuit_destroy(m.c);

    m = one_shot();
    ok = circuit_start(m.c) == 0 && at(m.c, 1 * MS, m.out) == 0;
    press(&m, 1);
    ok = ok && circuit_advance(m.c, 100000000ULL) == 1;
    press(&m, 0);
    ok = ok && at(m.c, 1500000000ULL, m.out) == 1;
    press(&m, 1);
    ok = ok && circuit_advance(m.c, 100000000ULL) == 1;
    press(&m, 0);
    ok = ok && at(m.c, 2200000000ULL, m.out) == 0;
    expect(ok, "(t) a second press at 1.5 ms during the pulse does not extend it: OUT 0 at 2.2 ms");
    circuit_destroy(m.c);
}

static void stopped_case(CircuitA a, TimerReason reason, const char *what) {
    int ok = circuit_start(a.c) == 0 && state_is(a.c, a.timer, 0, TIMER_STOPPED, reason) &&
             at(a.c, 5 * MS, a.out) == 0 && circuit_net_changes(a.c, a.out) <= 1;

    expect(ok, what);
    circuit_destroy(a.c);
}

static void stopped_cases(void) {
    CircuitA a;
    size_t g;

    /* (u) shapes */
    {
        Circuit *c = circuit_create(library);
        size_t timer = add(c, "TESTTIMER"), vcc = net(c), gnd = net(c), d = net(c), node = net(c), out = net(c);
        CircuitA s;

        wire(c, add(c, "5V"), "1", vcc);
        wire(c, add(c, "GND"), "1", gnd);
        wire(c, timer, "8", vcc);
        wire(c, timer, "1", gnd);
        wire(c, timer, "4", vcc);
        wire(c, timer, "2", node);
        wire(c, timer, "6", node);
        wire(c, timer, "7", d);
        wire(c, timer, "3", out);
        two(c, value_part(c, "GENERIC-RESISTOR", 1 * KOHM), vcc, d);
        two(c, value_part(c, "GENERIC-RESISTOR", 10 * KOHM), vcc, node);   /* RB to the supply, not to D */
        two(c, value_part(c, "GENERIC-CAPACITOR", 100 * FF_PER_NF), node, gnd);
        s.c = c;
        s.timer = timer;
        s.out = out;
        stopped_case(s, TIMER_SHAPE, "(u) RB between C and the supply instead of D: shape");
    }
    a = circuit_a(1 * KOHM, 10 * KOHM, 100 * FF_PER_NF);
    two(a.c, value_part(a.c, "GENERIC-RESISTOR", 10 * KOHM), a.node, a.gnd);
    stopped_case(a, TIMER_SHAPE, "(u) a second 10 k from C to GND: shape");
    a = circuit_a(1 * KOHM, 10 * KOHM, 100 * FF_PER_NF);
    g = add(a.c, "SN74HC04N");
    wire(a.c, g, "14", a.vcc);
    wire(a.c, g, "7", a.gnd);
    wire(a.c, g, "1", a.gnd);
    wire(a.c, g, "2", a.node);   /* 1Y drives C */
    stopped_case(a, TIMER_SHAPE, "(u) an SN74HC04N output on C: shape");
    a = circuit_a(1 * KOHM, 10 * KOHM, 100 * FF_PER_NF);
    g = add(a.c, "SN74HC04N");
    wire(a.c, g, "14", a.vcc);
    wire(a.c, g, "7", a.gnd);
    wire(a.c, g, "1", a.node);   /* 1A reads C */
    expect(circuit_start(a.c) == 0 && state_is(a.c, a.timer, 0, TIMER_ASTABLE, TIMER_OK),
           "(u) an SN74HC04N input on C: still astable");
    circuit_destroy(a.c);
    /* (v) the GND pin unwired */
    {
        Circuit *c = circuit_create(library);
        size_t timer = add(c, "TESTTIMER"), vcc = net(c), gnd = net(c), d = net(c), node = net(c), out = net(c);
        CircuitA s;

        wire(c, add(c, "5V"), "1", vcc);
        wire(c, add(c, "GND"), "1", gnd);
        wire(c, timer, "8", vcc);
        wire(c, timer, "4", vcc);
        wire(c, timer, "2", node);
        wire(c, timer, "6", node);
        wire(c, timer, "7", d);
        wire(c, timer, "3", out);
        two(c, value_part(c, "GENERIC-RESISTOR", 1 * KOHM), vcc, d);
        two(c, value_part(c, "GENERIC-RESISTOR", 10 * KOHM), d, node);
        two(c, value_part(c, "GENERIC-CAPACITOR", 100 * FF_PER_NF), node, gnd);
        s.c = c;
        s.timer = timer;
        s.out = out;
        stopped_case(s, TIMER_NO_SUPPLY, "(v) the timer's GND pin unwired: no supply");
    }
    /* (w) CONT */
    a = circuit_a(1 * KOHM, 10 * KOHM, 100 * FF_PER_NF);
    wire(a.c, a.timer, "5", a.vcc);
    stopped_case(a, TIMER_CONTROL, "(w) CONT on 5V: control");
    a = circuit_a(1 * KOHM, 10 * KOHM, 100 * FF_PER_NF);
    {
        size_t k = net(a.c);

        wire(a.c, a.timer, "5", k);
        two(a.c, value_part(a.c, "GENERIC-CAPACITOR", 10 * FF_PER_NF), k, a.gnd);
    }
    expect(circuit_start(a.c) == 0 && state_is(a.c, a.timer, 0, TIMER_ASTABLE, TIMER_OK),
           "(w) a 10 nF from CONT to GND: still astable");
    circuit_destroy(a.c);
    /* (x) too fast: 10 ohm + 10 ohm + 1 nF */
    a = circuit_a(10 * OHM, 10 * OHM, 1 * FF_PER_NF);
    stopped_case(a, TIMER_TOO_FAST, "(x) 10 + 10 ohm and 1 nF (period 20.8 ns on a 500 kHz part): too fast");
    /* (y) a 0 ohm library resistor as RB */
    {
        Circuit *c = circuit_create(library);
        size_t timer = add(c, "TESTTIMER"), vcc = net(c), gnd = net(c), d = net(c), node = net(c), out = net(c);
        CircuitA s;

        wire(c, add(c, "5V"), "1", vcc);
        wire(c, add(c, "GND"), "1", gnd);
        wire(c, timer, "8", vcc);
        wire(c, timer, "1", gnd);
        wire(c, timer, "4", vcc);
        wire(c, timer, "2", node);
        wire(c, timer, "6", node);
        wire(c, timer, "7", d);
        wire(c, timer, "3", out);
        two(c, value_part(c, "GENERIC-RESISTOR", 1 * KOHM), vcc, d);
        two(c, add(c, "CRCW06030000Z0EBC"), d, node);
        two(c, value_part(c, "GENERIC-CAPACITOR", 100 * FF_PER_NF), node, gnd);
        s.c = c;
        s.timer = timer;
        s.out = out;
        stopped_case(s, TIMER_NO_VALUE, "(y) a 0 ohm library resistor as RB: no value");
    }
}

static void call_cases(void) {
    Circuit *c = circuit_create(library);
    size_t timer = add(c, "TESTTIMER"), r = add(c, "GENERIC-RESISTOR");
    TimerState s;
    int ok;

    ok = circuit_set_part_value(c, timer, 5) == -1 && strstr(circuit_error(c), "has no value") != NULL;
    expect(ok, "(z) circuit_set_part_value on a timer: -1, \"part N has no value\"");
    ok = circuit_set_part_value(c, r, 0) == -1 && strstr(circuit_error(c), "the value must be above 0") != NULL;
    expect(ok, "(z) value 0: -1, \"the value must be above 0\"");
    ok = circuit_timer_state(c, r, 0, &s) == -1 && strstr(circuit_error(c), "is not a timer") != NULL;
    expect(ok, "(z) circuit_timer_state on a resistor: \"part N is not a timer\"");
    ok = circuit_timer_state(c, timer, 1, &s) == -1 && strstr(circuit_error(c), "timer 1 of part") != NULL &&
         strstr(circuit_error(c), "does not exist") != NULL;
    expect(ok, "(z) section 1 of TESTTIMER: \"timer 1 of part N does not exist\"");
    ok = circuit_timer_state(c, timer, 0, &s) == -1 && strstr(circuit_error(c), "the circuit is not running") != NULL;
    expect(ok, "(z) before circuit_start: \"the circuit is not running\"");
    ok = circuit_start(c) == 0 && circuit_set_part_value(c, r, 5) == -1 &&
         strstr(circuit_error(c), "circuit already started") != NULL;
    expect(ok, "(z) circuit_set_part_value after circuit_start: the building error");
    circuit_destroy(c);
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 || load_timer_fixture(library) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    fact_cases();
    rejection_cases();
    latch_cases();
    astable_cases();
    one_shot_cases();
    stopped_cases();
    call_cases();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("timer_test: all passed\n");
    return 0;
}
