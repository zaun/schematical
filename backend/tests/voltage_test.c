/*
 * voltage_test: voltage_check (include/voltage.h) on small netlists of the
 * fixture parts (support/voltage_fixture.h) and the real supplies, one case
 * per rule of plan-07-B section 4.2, and the supply domains of plan-20-F.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parts.h"
#include "read_file.h"
#include "voltage.h"
#include "voltage_fixture.h"

static int failures;
static PartLibrary *library;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

/* A netlist under construction. */
typedef struct {
    const Part *parts[16];
    size_t part_count;
    size_t net_count;
    VoltageConnection connections[64];
    size_t connection_count;
    VoltageContact closed[8];
    size_t closed_count;
    VoltageProblem problems[16];
    size_t problem_count;
} Netlist;

static size_t add(Netlist *n, const char *name) {
    const Part *part = part_library_find(library, name);

    if (part == NULL) {
        printf("  no part %s\n", name);
        failures++;
        exit(1);
    }
    n->parts[n->part_count] = part;
    return n->part_count++;
}

static size_t net(Netlist *n) {
    return n->net_count++;
}

/* Connects the pin with this designator to a net. */
static void wire(Netlist *n, size_t part, const char *designator, size_t net_index) {
    const Part *p = n->parts[part];
    size_t i;

    for (i = 0; i < p->pin_count; i++) {
        if (strcmp(p->pins[i].designator, designator) == 0) {
            VoltageConnection *c = &n->connections[n->connection_count++];

            c->part = part;
            c->pin = i;
            c->net = net_index;
            return;
        }
    }
    printf("  %s has no pin %s\n", p->name, designator);
    exit(1);
}

/* A supply on a new net; returns the net. */
static size_t supply(Netlist *n, const char *name) {
    size_t s = add(n, name);
    size_t net_index = net(n);

    wire(n, s, "1", net_index);
    return net_index;
}

static void close_contact(Netlist *n, size_t part, unsigned position) {
    n->closed[n->closed_count].part = part;
    n->closed[n->closed_count].position = position;
    n->closed_count++;
}

static size_t check(Netlist *n) {
    n->problem_count = voltage_check(n->parts, n->part_count, n->net_count, n->connections,
                                     n->connection_count, n->closed, n->closed_count, n->problems,
                                     sizeof n->problems / sizeof n->problems[0]);
    return n->problem_count;
}

static int text_is(Netlist *n, size_t index, const char *want) {
    char text[256];

    voltage_describe(n->parts, &n->problems[index], text, sizeof text);
    if (strcmp(text, want) != 0) {
        printf("  text \"%s\"\n  want \"%s\"\n", text, want);
        return 0;
    }
    return 1;
}

static int only(Netlist *n, VoltageProblemKind kind) {
    if (check(n) != 1 || n->problems[0].kind != kind) {
        printf("  %zu problems, first kind %d (wanted 1 of kind %d)\n", n->problem_count,
               n->problem_count > 0 ? (int)n->problems[0].kind : -1, (int)kind);
        return 0;
    }
    return 1;
}

static int none(Netlist *n) {
    if (check(n) != 0) {
        char text[256];

        voltage_describe(n->parts, &n->problems[0], text, sizeof text);
        printf("  %zu problems, first: %s\n", n->problem_count, text);
        return 0;
    }
    return 1;
}

/* A gate powered from a new supply net; returns the gate. */
static size_t powered_gate(Netlist *n, const char *gate, const char *supply_name) {
    size_t g = add(n, gate);

    wire(n, g, "5", supply(n, supply_name));
    return g;
}

static void check_power_pins(void) {
    Netlist n = {0};
    size_t g;

    g = add(&n, "VFX-HC1G00");
    wire(&n, g, "5", supply(&n, "3V3"));
    wire(&n, g, "3", supply(&n, "GND"));
    expect(none(&n), "3V3 on the 2-6 V part's VCC (GND on GND) is fine");

    memset(&n, 0, sizeof n);
    g = add(&n, "VFX-AUC1G00");
    wire(&n, g, "5", supply(&n, "5V"));
    expect(only(&n, VOLTAGE_POWER_RANGE) && n.problems[0].mv == 5000 &&
               n.problems[0].min_mv == 800 && n.problems[0].max_mv == 2700 &&
               n.problems[0].part == g && n.problems[0].pin == 4 &&
               text_is(&n, 0, "voltage: 5 V on pin 5 (VCC) of VFX-AUC1G00; it needs 0.8-2.7 V"),
           "5V on the 0.8-2.7 V part's VCC is POWER_RANGE");

    memset(&n, 0, sizeof n);
    g = add(&n, "VFX-AUC1G00");
    wire(&n, g, "5", supply(&n, "GND"));
    expect(only(&n, VOLTAGE_POWER_RANGE) && n.problems[0].mv == 0 &&
               text_is(&n, 0, "voltage: 0 V on pin 5 (VCC) of VFX-AUC1G00; it needs 0.8-2.7 V"),
           "a VCC pin on GND is POWER_RANGE at 0 V");
}

static void check_inputs(void) {
    Netlist n = {0};
    size_t auc;
    size_t hc;
    size_t od;
    size_t y;

    auc = powered_gate(&n, "VFX-AUC1G00", "1V8");
    wire(&n, auc, "1", supply(&n, "5V"));
    expect(only(&n, VOLTAGE_INPUT_OVER) && n.problems[0].max_mv == 3600 && n.problems[0].pin == 0 &&
               text_is(&n, 0, "voltage: 5 V on pin 1 (A) of VFX-AUC1G00; its inputs take at most 3.6 V"),
           "5V on an input with inputMax 3.6 is INPUT_OVER");

    memset(&n, 0, sizeof n);
    auc = powered_gate(&n, "VFX-AUC1G00", "1V8");
    hc = powered_gate(&n, "VFX-HC1G00", "5V");
    y = net(&n);
    wire(&n, hc, "4", y);
    wire(&n, auc, "1", y);
    expect(only(&n, VOLTAGE_INPUT_OVER) && n.problems[0].mv == 5000 && n.problems[0].part == auc,
           "a powered 5 V gate's output into a 3.6 V input is INPUT_OVER");

    memset(&n, 0, sizeof n);
    auc = powered_gate(&n, "VFX-AUC1G00", "1V8");
    hc = add(&n, "VFX-HC1G00");
    y = net(&n);
    wire(&n, hc, "4", y);
    wire(&n, auc, "1", y);
    expect(none(&n), "an unpowered gate's output carries no voltage");

    memset(&n, 0, sizeof n);
    auc = powered_gate(&n, "VFX-AUC1G00", "1V8");
    od = powered_gate(&n, "VFX-OD1G00", "5V");
    y = net(&n);
    wire(&n, od, "4", y);
    wire(&n, auc, "1", y);
    expect(none(&n), "an open-drain output carries no voltage");

    memset(&n, 0, sizeof n);
    auc = powered_gate(&n, "VFX-AUC1G00", "3V3");
    wire(&n, auc, "1", supply(&n, "3V3"));
    expect(only(&n, VOLTAGE_POWER_RANGE),
           "3V3 on a 3.6 V-tolerant input is fine (only the 3.3 V VCC is out of range)");

    memset(&n, 0, sizeof n);
    auc = powered_gate(&n, "VFX-AUC1G00", "1V8");
    od = powered_gate(&n, "VFX-OD74", "5V");
    y = net(&n);
    wire(&n, od, "4", y);
    wire(&n, auc, "1", y);
    expect(none(&n), "an open-drain dff's Q carries no voltage");

    memset(&n, 0, sizeof n);
    auc = powered_gate(&n, "VFX-AUC1G00", "1V8");
    od = powered_gate(&n, "VFX-PP74", "5V");
    y = net(&n);
    wire(&n, od, "4", y);
    wire(&n, auc, "1", y);
    expect(only(&n, VOLTAGE_INPUT_OVER) && n.problems[0].mv == 5000 && n.problems[0].part == auc,
           "a push-pull dff's Q at 5 V into a 3.6 V input is INPUT_OVER");
}

/* plan-12-A: pull level P, used only when no supply and no powered output set
   the group (E = S, else D, else P, else C). */
static void check_pulls(void) {
    Netlist n = {0};
    size_t auc;
    size_t od;
    size_t hc;
    size_t y;

    auc = powered_gate(&n, "VFX-AUC1G00", "1V8");
    od = powered_gate(&n, "VFX-OD1G00", "5V");
    y = net(&n);
    wire(&n, od, "4", y);
    wire(&n, auc, "1", y);
    wire(&n, add(&n, "PULLUP-5V"), "1", y);
    expect(only(&n, VOLTAGE_INPUT_OVER) && n.problems[0].mv == 5000 && n.problems[0].part == auc,
           "an open-drain output pulled up to 5 V into a 3.6 V input is INPUT_OVER");

    memset(&n, 0, sizeof n);
    auc = powered_gate(&n, "VFX-AUC1G00", "1V8");
    od = powered_gate(&n, "VFX-OD1G00", "5V");
    y = net(&n);
    wire(&n, od, "4", y);
    wire(&n, auc, "1", y);
    wire(&n, add(&n, "PULLUP-3V3"), "1", y);
    expect(none(&n), "the same output pulled up to 3.3 V is fine");

    memset(&n, 0, sizeof n);
    auc = powered_gate(&n, "VFX-AUC1G00", "1V8");
    y = net(&n);
    wire(&n, auc, "1", y);
    wire(&n, add(&n, "PULLUP-5V"), "1", y);
    wire(&n, add(&n, "3V3"), "1", y);
    expect(none(&n), "a 5 V pull-up and a 3V3 supply on one net: no clash, E = 3.3 V");

    memset(&n, 0, sizeof n);
    auc = powered_gate(&n, "VFX-AUC1G00", "1V8");
    hc = powered_gate(&n, "VFX-HC1G00", "3V3");
    y = net(&n);
    wire(&n, hc, "4", y);
    wire(&n, auc, "1", y);
    wire(&n, add(&n, "PULLUP-5V"), "1", y);
    expect(none(&n), "a push-pull 3.3 V output beats a 5 V pull-up (D wins)");
}

static void check_supplies(void) {
    Netlist n = {0};
    size_t a;
    size_t b;
    size_t shared;

    a = add(&n, "3V3");
    b = add(&n, "5V");
    shared = net(&n);
    wire(&n, a, "1", shared);
    wire(&n, b, "1", shared);
    expect(only(&n, VOLTAGE_SUPPLY_CLASH) && n.problems[0].mv == 5000 &&
               n.problems[0].min_mv == 3300 && n.problems[0].part == a &&
               n.problems[0].other_part == b &&
               text_is(&n, 0, "voltage: 3.3 V and 5 V supplies on one net (3V3, 5V)"),
           "3V3 and 5V on one net are a CLASH");

    memset(&n, 0, sizeof n);
    b = add(&n, "5V");
    a = add(&n, "3V3");
    shared = net(&n);
    wire(&n, b, "1", shared);
    wire(&n, a, "1", shared);
    expect(only(&n, VOLTAGE_SUPPLY_CLASH) &&
               text_is(&n, 0, "voltage: 3.3 V and 5 V supplies on one net (3V3, 5V)"),
           "the CLASH text names the lower voltage first");

    memset(&n, 0, sizeof n);
    a = add(&n, "5V");
    b = add(&n, "GND");
    shared = net(&n);
    wire(&n, a, "1", shared);
    wire(&n, b, "1", shared);
    expect(none(&n), "5V and GND on one net is a short, not a voltage problem");

    memset(&n, 0, sizeof n);
    a = add(&n, "5V");
    b = add(&n, "5V");
    shared = net(&n);
    wire(&n, a, "1", shared);
    wire(&n, b, "1", shared);
    expect(none(&n), "two 5 V supplies on one net agree");
}

static void check_switches(void) {
    Netlist n = {0};
    size_t auc;
    size_t sw;
    size_t inner;

    auc = add(&n, "VFX-AUC1G00");
    sw = add(&n, "VFX-SW2");
    inner = net(&n);
    wire(&n, auc, "5", inner);
    wire(&n, sw, "4", inner);                 /* 1B */
    wire(&n, sw, "1", supply(&n, "5V"));      /* 1A */
    expect(none(&n), "an open contact keeps 5V off the 0.8-2.7 V power net");
    close_contact(&n, sw, 1);
    expect(only(&n, VOLTAGE_POWER_RANGE) && n.problems[0].part == auc && n.problems[0].net == inner,
           "a closed contact joining 5V to the 0.8-2.7 V power net is POWER_RANGE");

    memset(&n, 0, sizeof n);
    sw = add(&n, "VFX-SW2");
    wire(&n, sw, "1", supply(&n, "5V"));
    expect(none(&n), "5V on a contact of the switch rated 5 V is fine");

    memset(&n, 0, sizeof n);
    sw = add(&n, "VFX-SW2");
    wire(&n, sw, "1", supply(&n, "12V"));
    expect(only(&n, VOLTAGE_SWITCH_OVER) && n.problems[0].max_mv == 5000 &&
               text_is(&n, 0, "voltage: 12 V on pin 1 (1A) of VFX-SW2; it is rated 5 V"),
           "12V on the switch rated 5 V is SWITCH_OVER");

    memset(&n, 0, sizeof n);
    sw = add(&n, "VFX-SW2");
    wire(&n, sw, "1", supply(&n, "12V"));
    close_contact(&n, sw, 1);
    wire(&n, sw, "4", net(&n));
    expect(check(&n) == 2 && n.problems[0].kind == VOLTAGE_SWITCH_OVER &&
               n.problems[1].kind == VOLTAGE_SWITCH_OVER && n.problems[1].pin == 3,
           "a closed contact carries 12V to its other pin (both rated pins report)");
}

static void check_leds(void) {
    Netlist n = {0};
    size_t auc;
    size_t led;
    size_t cathode;

    auc = powered_gate(&n, "VFX-AUC1G00", "1V8");
    led = add(&n, "GENERIC-LED");
    cathode = net(&n);
    wire(&n, led, "2", supply(&n, "5V"));     /* anode */
    wire(&n, led, "1", cathode);
    wire(&n, auc, "1", cathode);
    expect(only(&n, VOLTAGE_INPUT_OVER) && n.problems[0].net == cathode,
           "an LED carries its anode's 5 V to an input on its cathode");

    wire(&n, add(&n, "GND"), "1", cathode);
    expect(none(&n), "GND on the cathode net overrides the carried voltage");

    memset(&n, 0, sizeof n);
    auc = powered_gate(&n, "VFX-AUC1G00", "1V8");
    led = add(&n, "GENERIC-LED");
    cathode = net(&n);
    wire(&n, led, "1", supply(&n, "5V"));     /* cathode on 5V */
    wire(&n, led, "2", cathode);              /* anode net */
    wire(&n, auc, "1", cathode);
    expect(none(&n), "an LED carries voltage anode to cathode only");
}

static void check_no_data(void) {
    Netlist n = {0};
    Part copy;
    size_t g;

    g = add(&n, "VFX-AUC1G00");
    copy = *n.parts[g];
    copy.vcc = NULL;
    n.parts[g] = &copy;
    wire(&n, g, "5", supply(&n, "12V"));
    wire(&n, g, "1", supply(&n, "12V"));
    expect(none(&n), "a part with no vcc data is not checked");

    memset(&n, 0, sizeof n);
    g = add(&n, "VFX-HC1G00");
    copy = *n.parts[g];
    copy.vcc = NULL;
    n.parts[g] = &copy;
    {
        size_t auc = powered_gate(&n, "VFX-AUC1G00", "1V8");
        size_t y = net(&n);

        wire(&n, g, "5", supply(&n, "5V"));
        wire(&n, g, "4", y);
        wire(&n, auc, "1", y);
    }
    expect(none(&n), "a part with no vcc data is not a source");
}

static void check_order(void) {
    Netlist n = {0};
    size_t a;
    size_t b;
    size_t n0;
    size_t n1;
    VoltageProblem small[1];

    a = add(&n, "VFX-AUC1G00");
    b = add(&n, "VFX-AUC1G00");
    n0 = supply(&n, "5V");                   /* net 0 */
    n1 = supply(&n, "12V");                  /* net 1 */
    wire(&n, b, "2", n1);
    wire(&n, b, "5", n0);
    wire(&n, a, "1", n1);
    wire(&n, a, "5", n0);
    wire(&n, a, "2", n1);
    expect(check(&n) == 5 &&
               n.problems[0].net == 0 && n.problems[0].part == a &&
               n.problems[1].net == 0 && n.problems[1].part == b &&
               n.problems[2].net == 1 && n.problems[2].part == a && n.problems[2].pin == 0 &&
               n.problems[3].net == 1 && n.problems[3].part == a && n.problems[3].pin == 1 &&
               n.problems[4].net == 1 && n.problems[4].part == b,
           "problems are ordered by net, then part, then pin");
    expect(voltage_check(n.parts, n.part_count, n.net_count, n.connections, n.connection_count,
                         NULL, 0, small, 1) == 5 &&
               small[0].net == 0 && small[0].part == a,
           "the count is every problem; out holds the first cap");
}

/* plan-20-F: a dual-supply translator (VFX-T45: A side 1.65-3.6 V, B side
   1.65-5.5 V), each pin judged against its own side. */
static void check_domains(void) {
    Netlist n = {0};
    size_t t;
    size_t auc;
    size_t hc;
    size_t a;
    size_t b;

    /* VCCA 1.8 V, VCCB 5 V; a 1.8 V gate drives A, B feeds a 5 V HC input. */
    t = add(&n, "VFX-T45");
    wire(&n, t, "1", supply(&n, "1V8"));
    wire(&n, t, "6", supply(&n, "5V"));
    wire(&n, t, "2", supply(&n, "GND"));
    wire(&n, t, "5", supply(&n, "1V8"));
    auc = powered_gate(&n, "VFX-AUC1G00", "1V8");
    hc = powered_gate(&n, "VFX-HC1G00", "5V");
    a = net(&n);
    b = net(&n);
    wire(&n, auc, "4", a);
    wire(&n, t, "3", a);
    wire(&n, t, "4", b);
    wire(&n, hc, "1", b);
    expect(none(&n), "translator 1.8 V / 5 V: 1.8 V logic on A, 5 V logic on B: no problem");

    /* DIR low (B to A): a 5 V output drives B; A, which the translator drives
       at its 1.8 V side, feeds a 3.6 V-max input: the level shift, no problem. */
    memset(&n, 0, sizeof n);
    t = add(&n, "VFX-T45");
    wire(&n, t, "1", supply(&n, "1V8"));
    wire(&n, t, "6", supply(&n, "5V"));
    wire(&n, t, "5", supply(&n, "GND"));
    hc = powered_gate(&n, "VFX-HC1G00", "5V");
    auc = powered_gate(&n, "VFX-AUC1G00", "1V8");
    a = net(&n);
    b = net(&n);
    wire(&n, hc, "4", b);
    wire(&n, t, "4", b);
    wire(&n, t, "3", a);
    wire(&n, auc, "1", a);
    expect(none(&n), "translator B to A: 5 V on B, A into a 1.8 V-side input: no problem");

    /* A 5 V output straight into pin A: over the A side's 3.6 V input max. */
    memset(&n, 0, sizeof n);
    t = add(&n, "VFX-T45");
    wire(&n, t, "1", supply(&n, "1V8"));
    wire(&n, t, "6", supply(&n, "5V"));
    hc = powered_gate(&n, "VFX-HC1G00", "5V");
    a = net(&n);
    wire(&n, hc, "4", a);
    wire(&n, t, "3", a);
    expect(only(&n, VOLTAGE_INPUT_OVER) && n.problems[0].part == t && n.problems[0].pin == 2 &&
               n.problems[0].max_mv == 3600 &&
               text_is(&n, 0, "voltage: 5 V on pin 3 (A) of VFX-T45; its inputs take at most 3.6 V"),
           "translator: a 5 V output into pin A is INPUT_OVER (A side input max 3.6 V)");

    /* 5 V straight into pin B is fine: the B side takes up to 5.5 V. */
    memset(&n, 0, sizeof n);
    t = add(&n, "VFX-T45");
    wire(&n, t, "1", supply(&n, "1V8"));
    wire(&n, t, "6", supply(&n, "3V3"));
    wire(&n, t, "4", supply(&n, "5V"));
    expect(none(&n), "translator: 5 V into pin B at VCCB 3.3 V is within the B side's 5.5 V");

    /* Power ranges per side: 5 V is in the B range, out of the A range. */
    memset(&n, 0, sizeof n);
    t = add(&n, "VFX-T45");
    wire(&n, t, "1", supply(&n, "5V"));
    wire(&n, t, "6", supply(&n, "5V"));
    expect(only(&n, VOLTAGE_POWER_RANGE) && n.problems[0].pin == 0 && n.problems[0].max_mv == 3600,
           "translator: 5 V on VCCA is out of the A side's range; 5 V on VCCB is not");

    /* 12 V on VCCB: out of the B side's 1.65-5.5 V. */
    memset(&n, 0, sizeof n);
    t = add(&n, "VFX-T45");
    wire(&n, t, "1", supply(&n, "1V8"));
    wire(&n, t, "6", supply(&n, "12V"));
    expect(only(&n, VOLTAGE_POWER_RANGE) && n.problems[0].pin == 5 && n.problems[0].max_mv == 5500,
           "translator: 12 V on VCCB is out of the B side's 1.65-5.5 V range");
}

/* plan-20-H: the real SN74LVC1T45DBVR at VCCA 1.8 V / VCCB 5 V between a 1.8 V
   SN74LVC1G04DBVR and a 5 V SN74HC04N (A to B), with a 3.6 V-max input on
   A too: no problem (the A side sits at 1.8 V, the B side at 5 V). */
static void check_real_translator(void) {
    Netlist n = {0};
    size_t t;
    size_t lvc;
    size_t hc;
    size_t v18;
    size_t v5;
    size_t gnd;
    size_t a;
    size_t b;

    v18 = supply(&n, "1V8");
    v5 = supply(&n, "5V");
    gnd = supply(&n, "GND");
    t = add(&n, "SN74LVC1T45DBVR");
    lvc = add(&n, "SN74LVC1G04DBVR");
    hc = add(&n, "SN74HC04N");
    wire(&n, t, "1", v18);
    wire(&n, t, "6", v5);
    wire(&n, t, "2", gnd);
    wire(&n, t, "5", v18);
    wire(&n, lvc, "5", v18);
    wire(&n, lvc, "3", gnd);
    wire(&n, hc, "14", v5);
    wire(&n, hc, "7", gnd);
    a = net(&n);
    b = net(&n);
    wire(&n, lvc, "4", a);
    wire(&n, t, "3", a);
    wire(&n, t, "4", b);
    wire(&n, hc, "1", b);
    /* A 3.6 V-max input on A: one supply range would put 5 V there. */
    wire(&n, powered_gate(&n, "VFX-AUC1G00", "1V8"), "1", a);
    expect(none(&n), "SN74LVC1T45DBVR at 1.8 V / 5 V between a 1.8 V LVC1G04 and a 5 V HC04: no problem");
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 ||
        load_voltage_fixture(library) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    check_power_pins();
    check_inputs();
    check_pulls();
    check_supplies();
    check_switches();
    check_leds();
    check_no_data();
    check_order();
    check_domains();
    check_real_translator();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("voltage_test: all passed\n");
    return 0;
}
