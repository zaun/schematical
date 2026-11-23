/*
 * inductor_test (plan-47-I): the inductor kind. The fixture parts of
 * support/inductor_fixture.h load next to the real part files: the facts read
 * back, each malformed device or part is refused, and in circuits an inductor
 * conducts like a wire (its terminals, and the pads of one terminal, are one
 * net), a gate output drives an LED through it, 5 V against GND across it is
 * a short, and the voltage check sees both terminals as one group.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "inductor_fixture.h"
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

/* ---- facts and the loader (case a) --------------------------------------- */

static void fact_cases(void) {
    const Part *l = part_library_find(library, "TESTIND-10U");
    const Part *n = part_library_find(library, "TESTIND-1N");
    const Part *f = part_library_find(library, "TESTIND-4PAD");

    expect(l != NULL && l->kind == PART_INDUCTOR && l->inductor != NULL && l->pin_count == 2 &&
               l->vcc == NULL && l->pins[0].role == PIN_INOUT && l->pins[1].role == PIN_INOUT &&
               l->inductor->ph == 10000000ULL && same(l->inductor->tolerance, "+-20%") &&
               l->inductor->rated_ma == 1200 && l->inductor->isat_ma == 1500 &&
               l->inductor->dcr_uohm == 150000 && same(l->inductor->shielding, "shielded"),
           "(a) TESTIND-10U: an inductor, 2 inout pins, no vcc, 10 uH = 10000000 pH, +-20%, 1200 mA, "
           "1500 mA, 150000 uohm, shielded");
    expect(n != NULL && n->kind == PART_INDUCTOR && n->inductor != NULL &&
               n->inductor->ph == 1000 && n->inductor->tolerance == NULL &&
               n->inductor->rated_ma == 0 && n->inductor->isat_ma == 0 &&
               n->inductor->dcr_uohm == 0 && n->inductor->shielding == NULL,
           "(a) TESTIND-1N: 1 nH = 1000 pH, every optional fact 0 or NULL");
    expect(f != NULL && f->kind == PART_INDUCTOR && f->pin_count == 4 &&
               f->pins[0].role == PIN_INOUT && f->pins[3].role == PIN_INOUT &&
               f->inductor != NULL && f->inductor->ph == 2000000ULL,
           "(a) TESTIND-4PAD: 4 inout pins, 2 uH");
}

/* ---- rejections (case b) ------------------------------------------------- */

/* A one-file load that must be refused; `want` is part of the error. */
static void refused(const char *what, const char *json, const char *want) {
    PartLibrary *lib = part_library_create();
    char *error = NULL;
    int ok = lib != NULL && load_parts_dir(lib, PARTS_DIR) == 0 &&
             load_parts(lib, json, "bad-inductor.json", &error) != 0 && error != NULL &&
             strstr(error, want) != NULL;

    if (!ok) {
        printf("  error: %s\n", error != NULL ? error : "(none)");
    }
    expect(ok, what);
    free(error);
    part_library_destroy(lib);
}

#define TERM "[\"1\",\"2\"]"
#define BAD_PART(KEYS) IND_FILE(IND_DEVICE("bad", TERM, IFX_TWO, "BAD", "RES-0603", KEYS))

static void rejection_cases(void) {
    refused("(b) no \"henries\": refused", BAD_PART(""), "missing \"henries\"");
    refused("(b) henries 0: refused", BAD_PART(",\"henries\":0"),
            "\"henries\" must be above 0 and at most 100");
    refused("(b) henries 200: refused", BAD_PART(",\"henries\":200"),
            "\"henries\" must be above 0 and at most 100");
    refused("(b) shielding open: refused", BAD_PART(",\"henries\":1,\"shielding\":\"open\""),
            "unknown shielding \"open\"");
    refused("(b) amps 0: refused", BAD_PART(",\"henries\":1,\"amps\":0"),
            "\"amps\" must be above 0 and at most 1000");
    refused("(b) terminals with 1 name: refused",
            IND_FILE(IND_DEVICE("bad", "[\"1\"]", IFX_TWO, "BAD", "RES-0603", ",\"henries\":1")),
            "\"terminals\" must name 2 pins");
    refused("(b) a terminal with role input: refused",
            IND_FILE(IND_DEVICE("bad", TERM, IFX_PIN(1, "1", "input") "," IFX_IO(2, "2"), "BAD",
                                "RES-0603", ",\"henries\":1")),
            "inductor pin \"1\" must be inout");
    refused("(b) an extra inout pin: refused",
            IND_FILE(IND_DEVICE("bad", TERM, IFX_TWO "," IFX_IO(3, "3"), "BAD", "SOT23-3",
                                ",\"henries\":1")),
            "inductor pin \"3\" must be nc");
    refused("(b) an unlisted part key: refused", BAD_PART(",\"henries\":1,\"volts\":5"),
            "unknown key \"volts\"");
}

/* ---- circuit cases (c-h) ------------------------------------------------- */

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

static void wire_cases(void) {
    /* (c) 5V on pin 1; pin 2's net reads 1; with GND instead, 0. */
    {
        Circuit *c = circuit_create(library);
        size_t l = add(c, "TESTIND-10U"), v = add(c, "5V"), a = net(c), b = net(c);
        int ok;

        wire(c, v, "1", a);
        wire(c, l, "1", a);
        wire(c, l, "2", b);
        ok = circuit_start(c) == 0;
        run(c, 1000000);
        expect(ok && circuit_net_value(c, b) == CIRCUIT_1, "(c) 5V on pin 1: pin 2's net reads 1");
        circuit_destroy(c);
    }
    {
        Circuit *c = circuit_create(library);
        size_t l = add(c, "TESTIND-10U"), g = add(c, "GND"), a = net(c), b = net(c);
        int ok;

        wire(c, g, "1", a);
        wire(c, l, "1", a);
        wire(c, l, "2", b);
        ok = circuit_start(c) == 0;
        run(c, 1000000);
        expect(ok && circuit_net_value(c, b) == CIRCUIT_0, "(c) GND on pin 1: pin 2's net reads 0");
        circuit_destroy(c);
    }
    /* (d) an SN74HC04N output through the inductor lights an LED. */
    {
        int input;

        for (input = 0; input < 2; input++) {
            Circuit *c = circuit_create(library);
            size_t l = add(c, "TESTIND-10U"), g = add(c, "SN74HC04N"), led = add(c, "GENERIC-LED");
            size_t vcc = add(c, "5V"), gnd = add(c, "GND");
            size_t hi = net(c), lo = net(c), out = net(c), anode = net(c);
            int ok;

            wire(c, vcc, "1", hi);
            wire(c, gnd, "1", lo);
            wire(c, g, "14", hi);
            wire(c, g, "7", lo);
            wire(c, g, "1", input ? hi : lo);
            wire(c, g, "2", out);
            wire(c, l, "1", out);
            wire(c, l, "2", anode);
            wire(c, led, "2", anode);
            wire(c, led, "1", lo);
            ok = circuit_start(c) == 0;
            run(c, 1000000);
            expect(ok && lit(c, led) == (input ? 0 : 1),
                   input ? "(d) gate input on 5V: the LED behind the inductor is off"
                         : "(d) gate input on GND: the LED behind the inductor is lit");
            circuit_destroy(c);
        }
    }
    /* (e) 5V and GND across the inductor: smoke. */
    {
        Circuit *c = circuit_create(library);
        size_t l = add(c, "TESTIND-10U"), v = add(c, "5V"), g = add(c, "GND"), a = net(c), b = net(c);
        int started;
        int advanced;
        const char *error;

        wire(c, v, "1", a);
        wire(c, g, "1", b);
        wire(c, l, "1", a);
        wire(c, l, "2", b);
        started = circuit_start(c) == 0;
        advanced = circuit_advance(c, 1000000);
        error = circuit_error(c);
        expect(started && advanced == 0 && error != NULL && strncmp(error, "smoke:", 6) == 0,
               "(e) 5V and GND across the inductor: circuit_advance returns 0, the error is 'smoke:'");
        circuit_destroy(c);
    }
    /* (f) four pads: pad 2 on 5V; pads 1 (tie), 3 and 4 (the contact, the tie) read 1. */
    {
        Circuit *c = circuit_create(library);
        size_t l = add(c, "TESTIND-4PAD"), v = add(c, "5V"), n1 = net(c), n2 = net(c), n3 = net(c),
               n4 = net(c);
        int ok;

        wire(c, v, "1", n2);
        wire(c, l, "2", n2);
        wire(c, l, "1", n1);
        wire(c, l, "3", n3);
        wire(c, l, "4", n4);
        ok = circuit_start(c) == 0;
        run(c, 1000000);
        expect(ok && circuit_net_value(c, n1) == CIRCUIT_1 && circuit_net_value(c, n3) == CIRCUIT_1 &&
                   circuit_net_value(c, n4) == CIRCUIT_1,
               "(f) TESTIND-4PAD, 5V on pad 2: pads 1, 3 and 4 read 1");
        circuit_destroy(c);
    }
    {
        Circuit *c = circuit_create(library);
        size_t l = add(c, "TESTIND-4PAD"), v = add(c, "5V"), n2 = net(c);

        wire(c, v, "1", n2);
        wire(c, l, "2", n2);
        expect(circuit_start(c) == 0 && circuit_advance(c, 1000000) != 0 && circuit_error(c) == NULL,
               "(f) TESTIND-4PAD with pads 1, 3 and 4 unconnected starts and advances");
        circuit_destroy(c);
    }
    /* (g) circuit_set_switch on an inductor. */
    {
        Circuit *c = circuit_create(library);
        size_t l = add(c, "TESTIND-10U");
        char want[64];
        int ok;

        snprintf(want, sizeof(want), "part %zu is not a switch", l);
        ok = circuit_set_switch(c, l, 1, 0) == -1 && same(circuit_error(c), want);
        expect(ok, "(g) circuit_set_switch(inductor, 1, 0) is -1, 'part N is not a switch'");
        circuit_destroy(c);
    }
}

/* (h) 12 V (or 5 V) on pin 1, pin 2 on the VCC pin of an SN74HC04N. */
static void voltage_cases(void) {
    int volts;

    for (volts = 12; volts >= 5; volts -= 7) {
        Circuit *c = circuit_create(library);
        size_t l = add(c, "TESTIND-10U"), g = add(c, "SN74HC04N");
        size_t v = add(c, volts == 12 ? "12V" : "5V"), a = net(c), b = net(c);
        VoltageProblem p;
        int refused;

        wire(c, v, "1", a);
        wire(c, l, "1", a);
        wire(c, l, "2", b);
        wire(c, g, "14", b);
        refused = circuit_start(c) != 0;
        if (volts == 12) {
            expect(refused && circuit_voltage_problem(c, &p) == 1 && p.kind == VOLTAGE_POWER_RANGE &&
                       p.part == g,
                   "(h) 12 V through an inductor onto a 74HC04's VCC: VOLTAGE_POWER_RANGE on the 74HC04");
        } else {
            expect(!refused, "(h) the same on 5 V starts");
        }
        circuit_destroy(c);
    }
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 ||
        load_inductor_fixture(library) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    fact_cases();
    rejection_cases();
    wire_cases();
    voltage_cases();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("inductor_test: all passed\n");
    return 0;
}
