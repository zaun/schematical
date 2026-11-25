/*
 * opamp_test (plan-48-F): the opamp kind on fixture parts next to the real
 * part files (support/opamp_fixture.h). Cases (a)-(f): the facts and the
 * loader's rules; (g)-(j): an op-amp in a circuit drives nothing, its supply
 * pin is range-checked, its amplifier pins are not, and it does not stop the
 * simulated parts around it.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "kind.h"
#include "model_pins.h"
#include "opamp_fixture.h"
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

/* ---- facts and the loader (cases a-f) ------------------------------------ */

static void fact_cases(void) {
    const Part *a = part_library_find(library, "TESTOPAMP2");
    const Part *a2 = part_library_find(library, "TESTOPAMP2A");
    const Part *b = part_library_find(library, "TESTOPAMP1");
    size_t pin;
    int ok = 1;

    expect(a != NULL && a->kind == PART_OPAMP && a->opamp != NULL && a->simulated == 1 && a->pin_count == 8 &&
               same(a->opamp->type, "general") && a->opamp->amplifier_count == 2 && a->opamp->gbw_hz == 1000000 &&
               a->opamp->slew_mv_us == 500 && a->opamp->vos_uv == 7000 && a->opamp->rail_in == 0 &&
               a->opamp->rail_out == 0 && a->vcc != NULL && a->vcc->min_mv == 3000 && a->vcc->max_mv == 32000 &&
               a2 != NULL && a2->opamp->vos_uv == 3000,
           "(a) TESTOPAMP2: kind opamp, simulated, 8 pins, general, 2 amplifiers, 1 MHz, 500 mV/us, 7000 uV, "
           "3000-32000 mV; TESTOPAMP2A: 3000 uV");
    expect(b != NULL && same(b->opamp->type, "cmos") && b->opamp->amplifier_count == 1 && b->opamp->rail_in == 1 &&
               b->opamp->rail_out == 1 && b->opamp->gbw_hz == 0,
           "(b) TESTOPAMP1: cmos, 1 amplifier, rail-to-rail in and out, no gbw");
    for (pin = 0; pin < 8; pin++) {
        int want = pin == 0 || pin == 1 || pin == 2 || pin == 4 || pin == 5 || pin == 6;

        ok = ok && part_pin_analog(a, pin) == want;
    }
    expect(ok, "(c) part_pin_analog: 1 for pins 1, 2, 3, 5, 6, 7 of TESTOPAMP2, 0 for pins 4 and 8");
}

static void refused(const char *what, const char *json, const char *want) {
    PartLibrary *lib = part_library_create();
    char *error = NULL;
    int ok = lib != NULL && load_parts_dir(lib, PARTS_DIR) == 0 &&
             load_parts(lib, json, "bad-opamp.json", &error) != 0 && error != NULL &&
             strstr(error, want) != NULL;

    if (!ok) {
        printf("  error: %s\n", error != NULL ? error : "(none)");
    }
    expect(ok, what);
    free(error);
    part_library_destroy(lib);
}

#define OKKEYS ",\"vcc\":[3,32]"
#define BAD(TYPE, AMPS, PINS, KEYS, PARTS) OPAMP_FILE(OPAMP_DEVICE("bad", TYPE, AMPS, PINS, KEYS, PARTS))
#define BADPART OFX_PART("BAD", "PDIP-8", "")

static void rejection_cases(void) {
    refused("(d) type bjt: refused", BAD("bjt", OFX_AMP1 "," OFX_AMP2, OFX_DUAL8, OKKEYS, BADPART),
            "unknown op-amp type \"bjt\"");
    refused("(d) three amplifiers: refused",
            BAD("general", OFX_AMP1 "," OFX_AMP2 "," OFX_AMP2, OFX_DUAL8, OKKEYS, BADPART),
            "\"amplifiers\" must have 1, 2 or 4 entries");
    refused("(d) an entry without output: refused",
            BAD("general", "{\"plus\":\"IN1+\",\"minus\":\"IN1-\"}", OFX_DUAL8, OKKEYS, BADPART),
            "missing \"output\"");
    refused("(e) a plus pin with role output: refused",
            BAD("general", OFX_AMP1 "," OFX_AMP2,
                OFX_PIN(1, "OUT1", "output") "," OFX_PIN(2, "IN1-", "input") "," OFX_PIN(3, "IN1+", "output") ","
                OFX_PIN(4, "V-", "ground") "," OFX_PIN(5, "IN2+", "input") "," OFX_PIN(6, "IN2-", "input") ","
                OFX_PIN(7, "OUT2", "output") "," OFX_PIN(8, "V+", "power"),
                OKKEYS, BADPART),
            "op-amp pin \"IN1+\" must be an input");
    refused("(e) an output pin with role input: refused",
            BAD("general", OFX_AMP1 "," OFX_AMP2,
                OFX_PIN(1, "OUT1", "input") "," OFX_PIN(2, "IN1-", "input") "," OFX_PIN(3, "IN1+", "input") ","
                OFX_PIN(4, "V-", "ground") "," OFX_PIN(5, "IN2+", "input") "," OFX_PIN(6, "IN2-", "input") ","
                OFX_PIN(7, "OUT2", "output") "," OFX_PIN(8, "V+", "power"),
                OKKEYS, BADPART),
            "op-amp pin \"OUT1\" must be an output");
    refused("(e) a pin named by two entries: refused",
            BAD("general", OFX_AMP1 "," OFX_AMP1, OFX_DUAL8, OKKEYS, BADPART), "op-amp pin \"IN1+\" is named twice");
    refused("(e) no ground pin: refused",
            BAD("general", OFX_AMP1 "," OFX_AMP2,
                OFX_PIN(1, "OUT1", "output") "," OFX_PIN(2, "IN1-", "input") "," OFX_PIN(3, "IN1+", "input") ","
                OFX_PIN(4, "V-", "nc") "," OFX_PIN(5, "IN2+", "input") "," OFX_PIN(6, "IN2-", "input") ","
                OFX_PIN(7, "OUT2", "output") "," OFX_PIN(8, "V+", "power"),
                OKKEYS, BADPART),
            "an op-amp needs a power pin and a ground pin");
    refused("(f) slewVus -1: refused", BAD("general", OFX_AMP1 "," OFX_AMP2, OFX_DUAL8, ",\"slewVus\":-1" OKKEYS, BADPART),
            "\"slewVus\" must be from 0 to 100000");
    refused("(f) vosMv 2000: refused", BAD("general", OFX_AMP1 "," OFX_AMP2, OFX_DUAL8, ",\"vosMv\":2000" OKKEYS, BADPART),
            "\"vosMv\" must be from 0 to 1000");
    refused("(f) an unlisted part key: refused",
            BAD("general", OFX_AMP1 "," OFX_AMP2, OFX_DUAL8, OKKEYS, OFX_PART("BAD", "PDIP-8", ",\"gbwHz\":5")),
            "unknown key \"gbwHz\"");
}

/* ---- circuit cases (g-j) ------------------------------------------------ */

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

/* TESTOPAMP1 with V+ on `supply`, V- on GND, IN+ on `plus` and IN- on GND; returns its OUT net. */
static size_t opamp_rig(Circuit *c, const char *supply, const char *plus, size_t *part) {
    size_t q = add(c, "TESTOPAMP1"), vcc = net(c), gnd = net(c), in = net(c), out = net(c);

    wire(c, add(c, supply), "1", vcc);
    wire(c, add(c, "GND"), "1", gnd);
    wire(c, add(c, plus), "1", in);
    wire(c, q, "5", vcc);
    wire(c, q, "2", gnd);
    wire(c, q, "3", in);
    wire(c, q, "4", gnd);
    wire(c, q, "1", out);
    *part = q;
    return out;
}

static void circuit_cases(void) {
    Circuit *c = circuit_create(library);
    size_t q, out;
    VoltageProblem problem;
    int ok;

    out = opamp_rig(c, "5V", "5V", &q);
    ok = circuit_start(c) == 0 && circuit_advance(c, 1000000000ULL) == 1 && circuit_net_value(c, out) == CIRCUIT_Z &&
         circuit_net_changes(c, out) == 0;
    expect(ok, "(g) TESTOPAMP1 on 5 V: starts and runs; OUT reads Z, no change in 1 ms");
    circuit_destroy(c);

    c = circuit_create(library);
    out = opamp_rig(c, "12V", "5V", &q);
    (void)out;
    ok = circuit_start(c) == -1 && circuit_voltage_problem(c, &problem) == 1 && problem.kind == VOLTAGE_POWER_RANGE &&
         problem.part == q && problem.pin == 4;
    expect(ok, "(h) V+ on 12 V: refused, VOLTAGE_POWER_RANGE on the V+ pin");
    circuit_destroy(c);

    c = circuit_create(library);
    out = opamp_rig(c, "5V", "12V", &q);
    ok = circuit_start(c) == 0 && circuit_voltage_problem(c, &problem) == 0;
    expect(ok, "(i) IN+ on 12 V with V+ on 5 V: starts (an analog pin is not checked)");
    circuit_destroy(c);

    /* (j) (g) plus an inverter with an LED on its output */
    c = circuit_create(library);
    out = opamp_rig(c, "5V", "5V", &q);
    {
        size_t gate = add(c, "SN74HC04N"), led = add(c, "GENERIC-LED"), vcc = net(c), gnd = net(c), in = net(c),
               y = net(c);
        unsigned char lit = 0;

        wire(c, add(c, "5V"), "1", vcc);
        wire(c, add(c, "GND"), "1", gnd);
        wire(c, gate, "14", vcc);
        wire(c, gate, "7", gnd);
        wire(c, gate, "1", in);
        wire(c, add(c, "GND"), "1", in);
        wire(c, gate, "2", y);
        wire(c, led, "2", y);
        wire(c, led, "1", gnd);
        ok = circuit_start(c) == 0 && circuit_advance(c, 1000000000ULL) == 1 && circuit_net_value(c, y) == CIRCUIT_1 &&
             circuit_part_lights(c, led, &lit, 1) == 1 && lit == 1 && circuit_net_value(c, out) == CIRCUIT_Z;
        expect(ok, "(j) an op-amp next to an inverter with an LED: both start, the inverter output is 1 and lights the LED");
    }
    circuit_destroy(c);
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 || load_opamp_fixture(library) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    fact_cases();
    rejection_cases();
    circuit_cases();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("opamp_test: all passed\n");
    return 0;
}
