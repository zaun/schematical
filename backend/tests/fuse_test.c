/*
 * fuse_test (plan-47-I): the fuse kind, for fuses and PTC resettable fuses.
 * The fixture parts of support/fuse_fixture.h load next to the real part
 * files: the facts read back, each malformed device or part is refused, and
 * in circuits an intact fuse conducts like a wire, a blown one (position 1 of
 * the part switched off) does not and can be restored, the voltage check
 * counts both as one group, and a pin above the part's DC rating is refused.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "fuse_fixture.h"
#include "parts.h"
#include "read_file.h"
#include "voltage.h"

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

/* ---- facts and the loader (cases i, j) ----------------------------------- */

static void fact_cases(void) {
    const Part *f = part_library_find(library, "TESTFUSE-1A");
    const Part *p = part_library_find(library, "TESTPTC");

    expect(f != NULL && f->kind == PART_FUSE && f->fuse != NULL && f->pin_count == 2 &&
               f->vcc == NULL && f->pins[0].role == PIN_INOUT && same(f->fuse->type, "fuse") &&
               f->fuse->rated_ma == 1000 && f->fuse->trip_ma == 0 && f->fuse->ac_mv == 250000 &&
               f->fuse->dc_mv == 125000 && same(f->fuse->speed, "fast") && f->fuse->breaking_a == 35,
           "(i) TESTFUSE-1A: a fuse, 1000 mA, no trip, 250 V AC, 125 V DC, fast, 35 A");
    expect(p != NULL && p->kind == PART_FUSE && p->fuse != NULL && same(p->fuse->type, "ptc") &&
               p->fuse->rated_ma == 500 && p->fuse->trip_ma == 1000 && p->fuse->dc_mv == 6000 &&
               p->fuse->ac_mv == 0 && p->fuse->speed == NULL && p->fuse->breaking_a == 40,
           "(i) TESTPTC: a ptc, hold 500 mA, trip 1000 mA, 6 V, no AC rating, no speed, 40 A");
}

/* A one-file load that must be refused; `want` is part of the error. */
static void refused(const char *what, const char *json, const char *want) {
    PartLibrary *lib = part_library_create();
    char *error = NULL;
    int ok = lib != NULL && load_parts_dir(lib, PARTS_DIR) == 0 &&
             load_parts(lib, json, "bad-fuse.json", &error) != 0 && error != NULL &&
             strstr(error, want) != NULL;

    if (!ok) {
        printf("  error: %s\n", error != NULL ? error : "(none)");
    }
    expect(ok, what);
    free(error);
    part_library_destroy(lib);
}

#define TERM "[\"1\",\"2\"]"
#define BAD_FUSE(KEYS) FUSE_FILE(FUSE_DEVICE("bad", "fuse", TERM, FFX_TWO, "BAD", "RES-0603", KEYS))
#define BAD_PTC(KEYS) FUSE_FILE(FUSE_DEVICE("bad", "ptc", TERM, FFX_TWO, "BAD", "RES-0603", KEYS))
#define OK_FUSE ",\"amps\":1,\"speed\":\"fast\",\"voltsDc\":32"

static void rejection_cases(void) {
    refused("(j) type breaker: refused",
            FUSE_FILE(FUSE_DEVICE("bad", "breaker", TERM, FFX_TWO, "BAD", "RES-0603", OK_FUSE)),
            "unknown fuse type \"breaker\"");
    refused("(j) a fuse with no \"amps\": refused", BAD_FUSE(",\"speed\":\"fast\",\"voltsDc\":32"),
            "missing \"amps\"");
    refused("(j) speed quick: refused", BAD_FUSE(",\"amps\":1,\"speed\":\"quick\",\"voltsDc\":32"),
            "unknown fuse speed \"quick\"");
    refused("(j) no voltage key: refused", BAD_FUSE(",\"amps\":1,\"speed\":\"fast\""),
            "a fuse needs \"voltsAc\" or \"voltsDc\"");
    refused("(j) \"hold\" on a fuse: refused", BAD_FUSE(OK_FUSE ",\"hold\":1"),
            "\"hold\" is only for a ptc");
    refused("(j) \"amps\" on a ptc: refused", BAD_PTC(",\"amps\":1,\"hold\":1,\"trip\":2,\"volts\":6"),
            "\"amps\" is not a ptc key");
    refused("(j) trip not above hold: refused", BAD_PTC(",\"hold\":1,\"trip\":1,\"volts\":6"),
            "\"trip\" must be above \"hold\"");
    refused("(j) terminals with 1 name: refused",
            FUSE_FILE(FUSE_DEVICE("bad", "fuse", "[\"1\"]", FFX_TWO, "BAD", "RES-0603", OK_FUSE)),
            "\"terminals\" must name 2 pins");
    refused("(j) a terminal with role input: refused",
            FUSE_FILE(FUSE_DEVICE("bad", "fuse", TERM, FFX_PIN(1, "1", "input") "," FFX_IO(2, "2"),
                                  "BAD", "RES-0603", OK_FUSE)),
            "fuse pin \"1\" must be inout");
    refused("(j) an extra inout pin: refused",
            FUSE_FILE(FUSE_DEVICE("bad", "fuse", TERM, FFX_TWO "," FFX_IO(3, "3"), "BAD", "SOT23-3",
                                  OK_FUSE)),
            "fuse pin \"3\" must be nc");
    refused("(j) an unlisted part key: refused", BAD_FUSE(OK_FUSE ",\"henries\":1"),
            "unknown key \"henries\"");
}

/* ---- circuit helpers ----------------------------------------------------- */

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

static void run(Circuit *c, uint64_t ps) {
    if (circuit_advance(c, ps) == 0) {
        fprintf(stderr, "advance: %s\n", circuit_error(c) != NULL ? circuit_error(c) : "stopped");
        exit(1);
    }
}

static int lit(Circuit *c, size_t led) {
    unsigned char on = 0;

    return circuit_part_lights(c, led, &on, 1) == 1 ? on : -1;
}

/* ---- the fuse (cases k-o, and p for the ptc) ----------------------------- */

typedef struct {
    Circuit *c;
    size_t fuse, led, out, anode;
} LedCircuit;

/* An SN74HC04N output (input on GND, so high) through the fuse onto an LED. */
static LedCircuit led_circuit(const char *name) {
    LedCircuit k;
    size_t g = 0, vcc = 0, gnd = 0, hi = 0, lo = 0;

    k.c = circuit_create(library);
    k.fuse = add(k.c, name);
    g = add(k.c, "SN74HC04N");
    k.led = add(k.c, "GENERIC-LED");
    vcc = add(k.c, "5V");
    gnd = add(k.c, "GND");
    hi = net(k.c);
    lo = net(k.c);
    k.out = net(k.c);
    k.anode = net(k.c);
    wire(k.c, vcc, "1", hi);
    wire(k.c, gnd, "1", lo);
    wire(k.c, g, "14", hi);
    wire(k.c, g, "7", lo);
    wire(k.c, g, "1", lo);
    wire(k.c, g, "2", k.out);
    wire(k.c, k.fuse, "1", k.out);
    wire(k.c, k.fuse, "2", k.anode);
    wire(k.c, k.led, "2", k.anode);
    wire(k.c, k.led, "1", lo);
    return k;
}

static void blow_cases(const char *name) {
    char what[160];

    /* (k) intact: 5V on pin 1, pin 2 reads 1; the LED circuit lights. */
    {
        Circuit *c = circuit_create(library);
        size_t f = add(c, name), v = add(c, "5V"), a = net(c), b = net(c);
        int ok;

        wire(c, v, "1", a);
        wire(c, f, "1", a);
        wire(c, f, "2", b);
        ok = circuit_start(c) == 0;
        run(c, 1000000);
        snprintf(what, sizeof(what), "(k) %s intact, 5V on pin 1: pin 2 reads 1", name);
        expect(ok && circuit_net_value(c, b) == CIRCUIT_1, what);
        circuit_destroy(c);
    }
    {
        LedCircuit k = led_circuit(name);
        int ok = circuit_start(k.c) == 0;

        run(k.c, 1000000);
        snprintf(what, sizeof(what), "(k) %s intact: the LED behind it is lit", name);
        expect(ok && lit(k.c, k.led) == 1, what);
        /* (l) blown during the run, then restored. */
        ok = circuit_set_switch(k.c, k.fuse, 1, 0) == 0;
        run(k.c, 1000000);
        /* circuit_part_lights answers "on at any time during the last step", and the
           contact opens at the step's first instant: the LED is off in the next step. */
        run(k.c, 1000000);
        snprintf(what, sizeof(what), "(l) %s blown during the run: pin 2 reads Z and the LED is off", name);
        expect(ok && circuit_net_value(k.c, k.anode) == CIRCUIT_Z && lit(k.c, k.led) == 0, what);
        ok = circuit_set_switch(k.c, k.fuse, 1, 1) == 0;
        run(k.c, 1000000);
        snprintf(what, sizeof(what), "(l) %s restored: pin 2 reads 1 and the LED is on", name);
        expect(ok && circuit_net_value(k.c, k.anode) == CIRCUIT_1 && lit(k.c, k.led) == 1, what);
        circuit_destroy(k.c);
    }
    /* (m) blown before start. */
    {
        LedCircuit k = led_circuit(name);
        int ok = circuit_set_switch(k.c, k.fuse, 1, 0) == 0 && circuit_start(k.c) == 0;

        run(k.c, 1000000);
        snprintf(what, sizeof(what), "(m) %s blown before start: pin 2 reads Z", name);
        expect(ok && circuit_net_value(k.c, k.anode) == CIRCUIT_Z && lit(k.c, k.led) == 0, what);
        circuit_destroy(k.c);
    }
    /* (n) no position 2. */
    {
        Circuit *c = circuit_create(library);
        size_t f = add(c, name);
        char want[96];

        snprintf(want, sizeof(want), "%s has no position 2", name);
        snprintf(what, sizeof(what), "(n) %s: circuit_set_switch(2, 1) is -1, '%s'", name, want);
        expect(circuit_set_switch(c, f, 2, 1) == -1 && same(circuit_error(c), want), what);
        circuit_destroy(c);
    }
    /* (o) 5V and GND across it: smoke when intact, quiet when blown. */
    {
        Circuit *c = circuit_create(library);
        size_t f = add(c, name), v = add(c, "5V"), g = add(c, "GND"), a = net(c), b = net(c);
        int started;
        int advanced;

        wire(c, v, "1", a);
        wire(c, g, "1", b);
        wire(c, f, "1", a);
        wire(c, f, "2", b);
        started = circuit_start(c) == 0;
        advanced = circuit_advance(c, 1000000);
        snprintf(what, sizeof(what), "(o) 5V and GND across an intact %s: smoke", name);
        expect(started && advanced == 0 && circuit_error(c) != NULL &&
                   strncmp(circuit_error(c), "smoke:", 6) == 0,
               what);
        circuit_destroy(c);
    }
    {
        Circuit *c = circuit_create(library);
        size_t f = add(c, name), v = add(c, "5V"), g = add(c, "GND"), a = net(c), b = net(c);
        int ok;

        wire(c, v, "1", a);
        wire(c, g, "1", b);
        wire(c, f, "1", a);
        wire(c, f, "2", b);
        ok = circuit_set_switch(c, f, 1, 0) == 0 && circuit_start(c) == 0;
        run(c, 1000000);
        snprintf(what, sizeof(what), "(o) 5V and GND across a blown %s: the run goes on, nets keep their values", name);
        expect(ok && circuit_error(c) == NULL && circuit_net_value(c, a) == CIRCUIT_1 &&
                   circuit_net_value(c, b) == CIRCUIT_0,
               what);
        circuit_destroy(c);
    }
}

/* ---- the voltage check (cases q-t) --------------------------------------- */

static int on_volts(const char *name, const char *supply, VoltageProblem *p, char *text, size_t size) {
    Circuit *c = circuit_create(library);
    size_t f = add(c, name), v = add(c, supply), a = net(c);
    int refused;

    wire(c, v, "1", a);
    wire(c, f, "1", a);
    refused = circuit_start(c) != 0;
    text[0] = '\0';
    if (refused && circuit_voltage_problem(c, p) == 1 && circuit_error(c) != NULL) {
        snprintf(text, size, "%s", circuit_error(c));
    }
    circuit_destroy(c);
    return refused;
}

/* The (h) circuit with a fuse: `supply` on pin 1, pin 2 on an SN74HC04N's VCC. */
static Circuit *group_circuit(const char *supply, size_t *fuse, size_t *gate) {
    Circuit *c = circuit_create(library);
    size_t v;
    size_t a;
    size_t b;

    *fuse = add(c, "TESTFUSE-1A");
    *gate = add(c, "SN74HC04N");
    v = add(c, supply);
    a = net(c);
    b = net(c);
    wire(c, v, "1", a);
    wire(c, *fuse, "1", a);
    wire(c, *fuse, "2", b);
    wire(c, *gate, "14", b);
    return c;
}

static void voltage_cases(void) {
    VoltageProblem p;
    char text[256];
    size_t f;
    size_t g;

    expect(on_volts("TESTPTC", "12V", &p, text, sizeof(text)) && p.kind == VOLTAGE_FUSE_OVER &&
               p.mv == 12000 && p.max_mv == 6000 &&
               strcmp(text, "voltage: 12 V on pin 1 (1) of TESTPTC; it is rated 6 V") == 0,
           "(q) 12 V on a 6 V PTC: refused, 'voltage: 12 V on pin 1 (1) of TESTPTC; it is rated 6 V'");
    if (strcmp(text, "voltage: 12 V on pin 1 (1) of TESTPTC; it is rated 6 V") != 0) {
        printf("  text: %s\n", text);
    }
    expect(!on_volts("TESTPTC", "5V", &p, text, sizeof(text)), "(q) 5 V on the PTC: starts");
    expect(!on_volts("TESTFUSE-1A", "12V", &p, text, sizeof(text)),
           "(r) 12 V on TESTFUSE-1A (125 V DC): starts");
    {
        Circuit *c = group_circuit("12V", &f, &g);
        int refused = circuit_start(c) != 0;

        expect(refused && circuit_voltage_problem(c, &p) == 1 && p.kind == VOLTAGE_POWER_RANGE && p.part == g,
               "(s) 12 V through an intact fuse onto a 74HC04's VCC: VOLTAGE_POWER_RANGE on the 74HC04");
        circuit_destroy(c);
    }
    {
        Circuit *c = group_circuit("12V", &f, &g);
        int refused = circuit_set_switch(c, f, 1, 0) == 0 && circuit_start(c) != 0;

        expect(refused && circuit_voltage_problem(c, &p) == 1 && p.kind == VOLTAGE_POWER_RANGE && p.part == g,
               "(s) the same with the fuse blown: the same VOLTAGE_POWER_RANGE");
        circuit_destroy(c);
    }
    {
        Circuit *c = group_circuit("5V", &f, &g);
        int ok = circuit_set_switch(c, f, 1, 0) == 0 && circuit_start(c) == 0;

        run(c, 1000000);
        ok = ok && circuit_set_switch(c, f, 1, 1) == 0 && circuit_advance(c, 1000000) == 1;
        expect(ok, "(t) a fuse closing during a run on 5 V does not stop the run");
        circuit_destroy(c);
    }
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 || load_fuse_fixture(library) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    fact_cases();
    rejection_cases();
    blow_cases("TESTFUSE-1A");
    blow_cases("TESTPTC");
    voltage_cases();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("fuse_test: all passed\n");
    return 0;
}
