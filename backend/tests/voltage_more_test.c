/*
 * voltage_more_test (plan-68-L): the voltage check's text for every kind of
 * problem; connections that name a part or net the check was not given;
 * and the feedback divider of an adjustable regulator (RFX-FB, vref 0.8 V,
 * FB pin 3) with resistors the divider does not count, with a 0 ohm top and
 * with a ratio past any real voltage, and two regulators on one net.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "parts.h"
#include "read_file.h"
#include "regulator_fixture.h"
#include "voltage.h"

static int failures;
static PartLibrary *library;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

static const Part *find(const char *name) {
    const Part *part = part_library_find(library, name);

    if (part == NULL) {
        printf("FAIL: no part %s\n", name);
        exit(1);
    }
    return part;
}

static size_t add(Circuit *c, const char *name) {
    size_t index = 0;

    if (circuit_add_part(c, name, &index) != 0) {
        printf("FAIL: add %s: %s\n", name, circuit_error(c));
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
        printf("FAIL: connect %s: %s\n", pin, circuit_error(c));
        exit(1);
    }
}

/* Each problem kind's text. */
static void check_texts(void) {
    const Part *parts[2] = {find("SN74HC00N"), find("5V")};
    VoltageProblem p;
    char text[256];

    memset(&p, 0, sizeof p);
    p.part = 0;
    p.pin = 13;
    p.other_part = 0;
    p.mv = 5000;
    p.min_mv = 2000;
    p.max_mv = 6000;
    p.kind = VOLTAGE_POWER_RANGE;
    voltage_describe(parts, &p, text, sizeof text);
    expect(strcmp(text, "voltage: 5 V on pin 14 (VCC) of SN74HC00N; it needs 2-6 V") == 0, text);
    p.kind = VOLTAGE_INPUT_OVER;
    voltage_describe(parts, &p, text, sizeof text);
    expect(strcmp(text, "voltage: 5 V on pin 14 (VCC) of SN74HC00N; its inputs take at most 6 V") == 0, text);
    p.kind = VOLTAGE_FUSE_OVER;
    voltage_describe(parts, &p, text, sizeof text);
    expect(strcmp(text, "voltage: 5 V on pin 14 (VCC) of SN74HC00N; it is rated 6 V") == 0, text);
    p.kind = VOLTAGE_REGULATOR_INPUT_OVER;
    voltage_describe(parts, &p, text, sizeof text);
    expect(strcmp(text, "voltage: 5 V on pin 14 (VCC) of SN74HC00N; its input max is 6 V") == 0, text);
    p.kind = VOLTAGE_NIXIE_UNDER;
    voltage_describe(parts, &p, text, sizeof text);
    expect(strcmp(text, "voltage: 5 V on pin 14 (VCC) of SN74HC00N; it needs at least 2 V to light") == 0, text);
    p.pin = 99;
    voltage_describe(parts, &p, text, sizeof text);
    expect(strstr(text, "on pin ? (?)") != NULL, "a pin the part lacks is \"?\"");
    p.kind = VOLTAGE_SUPPLY_CLASH;
    p.part = 1;
    p.other_part = 0;
    p.min_mv = 3300;
    voltage_describe(parts, &p, text, sizeof text);
    expect(strcmp(text, "voltage: 3.3 V and 5 V supplies on one net (SN74HC00N, 5V)") == 0, text);
}

/* Connections that name a part or net past the check's counts are left out. */
static void check_out_of_range(void) {
    const Part *parts[1] = {find("SN74HC00N")};
    VoltageConnection connections[2] = {{5, 0, 0}, {0, 0, 7}};
    VoltageProblem out[4];

    expect(voltage_check(parts, 1, 1, connections, 2, NULL, 0, NULL, out, 4) == 0,
           "connections past the part or net count give no problem");
}

/* The level the regulator's OUT net has, read through a clash with a 1.2 V
   supply: 0 when the circuit starts (OUT has no level). */
static unsigned level_through_clash(Circuit *c, size_t out_net) {
    VoltageProblem p;
    size_t probe = add(c, "1V2");

    wire(c, probe, "1", out_net);
    memset(&p, 0, sizeof p);
    if (circuit_start(c) == 0 || circuit_voltage_problem(c, &p) != 1 || p.kind != VOLTAGE_SUPPLY_CLASH) {
        return 0;
    }
    return p.mv;
}

/* RFX-FB from 5 V: IN 1, OUT 2, FB 3, GND 4, EN 5 (active low, on GND). */
typedef struct {
    Circuit *c;
    size_t reg, hi, lo, out, fb;
} Fb;

static Fb fb_rig(void) {
    Fb r;
    size_t v, gnd;

    r.c = circuit_create(library);
    r.reg = add(r.c, "RFX-FB");
    v = add(r.c, "5V");
    gnd = add(r.c, "GND");
    r.hi = net(r.c);
    r.lo = net(r.c);
    r.out = net(r.c);
    r.fb = net(r.c);
    wire(r.c, v, "1", r.hi);
    wire(r.c, gnd, "1", r.lo);
    wire(r.c, r.reg, "1", r.hi);
    wire(r.c, r.reg, "4", r.lo);
    wire(r.c, r.reg, "5", r.lo);
    wire(r.c, r.reg, "2", r.out);
    wire(r.c, r.reg, "3", r.fb);
    return r;
}

/* A resistor from `a` to `b` (SIZE_MAX: unconnected) of `ohms` (0: the part's own). */
static void resistor(Fb *r, const char *part, size_t a, size_t b, double ohms) {
    size_t index = add(r->c, part);

    if (a != SIZE_MAX) wire(r->c, index, "1", a);
    if (b != SIZE_MAX) wire(r->c, index, "2", b);
    if (ohms > 0) circuit_set_resistance(r->c, index, ohms);
}

static void check_divider(void) {
    /* 10k top, 10k bottom: 1.6 V. A resistor from OUT to GND, one with both
       ends on FB and one from FB to nothing change nothing. */
    {
        Fb r = fb_rig();

        resistor(&r, "GENERIC-RESISTOR", r.out, r.fb, 10000);
        resistor(&r, "GENERIC-RESISTOR", r.fb, r.lo, 10000);
        resistor(&r, "GENERIC-RESISTOR", r.out, r.lo, 1000);
        resistor(&r, "GENERIC-RESISTOR", r.fb, r.fb, 1000);
        resistor(&r, "GENERIC-RESISTOR", r.fb, SIZE_MAX, 1000);
        expect(level_through_clash(r.c, r.out) == 1600,
               "resistors not between FB and another net leave the divider at 1.6 V");
        circuit_destroy(r.c);
    }
    /* A 0 ohm top: no divider level. */
    {
        Fb r = fb_rig();

        resistor(&r, "CRCW06030000Z0EBC", r.out, r.fb, 0);
        resistor(&r, "GENERIC-RESISTOR", r.fb, r.lo, 10000);
        expect(level_through_clash(r.c, r.out) == 0, "a 0 ohm top gives OUT no level");
        circuit_destroy(r.c);
    }
    /* 10 Mohm over 1 ohm: 8 000 000 V, past any real level. */
    {
        Fb r = fb_rig();

        resistor(&r, "GENERIC-RESISTOR", r.out, r.fb, 1e7);
        resistor(&r, "GENERIC-RESISTOR", r.fb, r.lo, 1);
        expect(level_through_clash(r.c, r.out) == 0, "a ratio past any real voltage gives OUT no level");
        circuit_destroy(r.c);
    }
    /* OUT unconnected: no regulated net. */
    {
        Circuit *c = circuit_create(library);
        size_t reg = add(c, "RFX-FB"), v = add(c, "5V"), gnd = add(c, "GND");
        size_t hi = net(c), lo = net(c);

        wire(c, v, "1", hi);
        wire(c, gnd, "1", lo);
        wire(c, reg, "1", hi);
        wire(c, reg, "4", lo);
        wire(c, reg, "5", lo);
        expect(circuit_start(c) == 0, "a regulator with OUT and FB open starts");
        circuit_destroy(c);
    }
}

/* RFX-LDO5 (from 12 V) and RFX-LDO33 (from 5 V) outputs on one net: they
   clash, 3.3 V the lower. */
static void check_two_regulators(void) {
    Circuit *c = circuit_create(library);
    size_t five = add(c, "RFX-LDO5"), three = add(c, "RFX-LDO33");
    size_t v12 = add(c, "12V"), v5 = add(c, "5V"), gnd = add(c, "GND");
    size_t hi12 = net(c), hi5 = net(c), lo = net(c), out = net(c);
    VoltageProblem p;

    wire(c, v12, "1", hi12);
    wire(c, v5, "1", hi5);
    wire(c, gnd, "1", lo);
    wire(c, five, "1", hi12);
    wire(c, five, "2", lo);
    wire(c, five, "3", out);
    wire(c, three, "1", hi5);
    wire(c, three, "2", lo);
    wire(c, three, "3", hi5);
    wire(c, three, "5", out);
    memset(&p, 0, sizeof p);
    expect(circuit_start(c) != 0 && circuit_voltage_problem(c, &p) == 1 && p.kind == VOLTAGE_SUPPLY_CLASH &&
               p.min_mv == 3300 && p.mv == 5000,
           "5 V and 3.3 V regulators on one net clash, 3.3 V the lower");
    if (p.kind != VOLTAGE_SUPPLY_CLASH || p.min_mv != 3300) {
        printf("  kind %d, %u and %u mV: %s\n", (int)p.kind, p.min_mv, p.mv, circuit_error(c));
    }
    circuit_destroy(c);
}

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0 || load_regulator_fixture(library) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    check_texts();
    check_out_of_range();
    check_divider();
    check_two_regulators();
    part_library_destroy(library);
    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
