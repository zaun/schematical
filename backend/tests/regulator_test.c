/*
 * regulator_test (plan-42-G): the `regulator` kind, its output model and its
 * voltage rules.
 *
 * Kind cases (a, b) load the fixture parts of support/regulator_fixture.h
 * next to the real part files: the facts read back, and each malformed
 * device or part is refused. Engine cases (c-i) put one fixture part's model
 * in an engine rig with a scripted driver (one pin per net; a weak drive is
 * a pull) and run it for a few ns: the output follows the input and the
 * enable 1 ns later. Voltage cases (j-v) build circuits with real supplies
 * and run the voltage check of circuit_start: a regulated level is observed
 * through the refusals it causes (a part whose supply range it is outside
 * of) and through the clash with a 1.2 V supply on the same net (the
 * problem reports both voltages).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "kind.h"
#include "model_pins.h"
#include "net.h"
#include "parts.h"
#include "read_file.h"
#include "regulator_fixture.h"
#include "sim.h"

static int failures;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

static PartLibrary *library;

/* ---- kind cases ---------------------------------------------------------- */

static const Part *find(const char *name) {
    const Part *p = part_library_find(library, name);

    if (p == NULL || p->kind != PART_REGULATOR || p->regulator == NULL) {
        printf("  %s: not a regulator part\n", name);
        return NULL;
    }
    return p;
}

static void fact_cases(void) {
    const Part *l33 = find("RFX-LDO33"), *l5 = find("RFX-LDO5"), *adj = find("RFX-ADJ"), *fb = find("RFX-FB");
    const Part *boost = find("RFX-BOOST"), *dual = find("RFX-DUAL"), *gen = find("RFX-GEN");
    const char *type = NULL;
    size_t in = 0, out = 0, enable = 0, pg = 0, adjust = 0;
    int low = 0, to_output = 0;

    expect(l33 != NULL && strcmp(l33->regulator->type, "linear") == 0 && l33->regulator->regulator_count == 1 &&
               !l33->regulator->mixed && !l33->regulator->adjustable && l33->regulator->vout_mv[0] == 3300 &&
               l33->regulator->vref_mv[0] == 0 && l33->regulator->dropout_mv == 300 &&
               l33->regulator->vin_max_mv == 6000 && l33->regulator->vin_min_mv == 0 && l33->vcc == NULL &&
               regulator_element(l33, 0, &type, &in, &out, &enable, &low, &pg, &adjust, &to_output) &&
               in == 0 && out == 4 && enable == 2 && !low && pg == 3 && adjust == SIZE_MAX && !to_output &&
               !regulator_element(l33, 1, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
           "(a) RFX-LDO33: linear, 3.3 V, dropout 0.3, vinMax 6; IN pin 1, OUT pin 5, EN pin 3, PG pin 4");
    expect(l5 != NULL && l5->regulator->vout_mv[0] == 5000 && l5->regulator->dropout_mv == 2000 &&
               l5->regulator->vin_max_mv == 35000 && regulator_input_max_mv(l5) == 35000 &&
               regulator_element(l5, 0, NULL, &in, &out, &enable, NULL, &pg, NULL, NULL) && enable == SIZE_MAX &&
               pg == SIZE_MAX,
           "(a) RFX-LDO5: 5 V, dropout 2, vinMax 35, no enable and no power good");
    expect(adj != NULL && adj->regulator->adjustable && adj->regulator->vout_mv[0] == 0 &&
               adj->regulator->vref_mv[0] == 1250 &&
               regulator_element(adj, 0, NULL, &in, &out, NULL, NULL, NULL, &adjust, &to_output) && adjust == 0 &&
               in == 2 && out == 1 && to_output,
           "(a) RFX-ADJ: adjustable, vref 1.25, ADJ pin 1, adjustTo output");
    expect(fb != NULL && fb->regulator->adjustable && fb->regulator->vref_mv[0] == 800 &&
               regulator_element(fb, 0, NULL, NULL, NULL, &enable, &low, NULL, &adjust, &to_output) &&
               enable == 4 && low && adjust == 2 && !to_output,
           "(a) RFX-FB: vref 0.8, FB pin 3, EN pin 5 active low, adjustTo ground");
    expect(boost != NULL && strcmp(boost->regulator->type, "boost") == 0 && boost->regulator->vin_min_mv == 900 &&
               boost->regulator->vin_max_mv == 5500 && boost->regulator->dropout_mv == 0,
           "(a) RFX-BOOST: boost, 5 V, vinMin 0.9, vinMax 5.5");
    expect(dual != NULL && dual->regulator->regulator_count == 2 && dual->regulator->vout_mv[0] == 1800 &&
               dual->regulator->vout_mv[1] == 3300 &&
               regulator_element(dual, 1, NULL, &in, &out, &enable, NULL, NULL, NULL, NULL) && in == 1 &&
               out == 3 && enable == 5 && regulator_pin_in(dual, 1, 1) && !regulator_pin_in(dual, 1, 0) &&
               regulator_pin_out(dual, 0, 2) && !regulator_pin_out(dual, 0, 3),
           "(a) RFX-DUAL: two linear, vout 1.8 and 3.3; element 2: IN2 pin 2, OUT2 pin 4, EN2 pin 6");
    expect(gen != NULL && gen->regulator->vin_max_mv == 0 && regulator_input_max_mv(gen) == 0,
           "(u) RFX-GEN: no vinMax loads, the maximum is 0 (never refused)");
}

/* A one-device file that must be refused; `want` is part of the error. */
static void refused(const char *what, const char *json, const char *want) {
    PartLibrary *lib = part_library_create();
    char *error = NULL;
    int ok = lib != NULL && load_parts(lib, json, "bad-regulator.json", &error) != 0 &&
             error != NULL && strstr(error, want) != NULL;

    if (!ok) {
        printf("  error: %s\n", error != NULL ? error : "(none)");
    }
    expect(ok, what);
    free(error);
    part_library_destroy(lib);
}

#define PINS3 RFX_PWR(1, "IN") "," RFX_GND(2, "GND") "," RFX_OUT(3, "OUT")
#define LIN_EL RFX_EL("linear", "IN", "OUT", "")
#define BAD(PINS, ELEMENTS, ANALOG, KEYS) \
    REGULATOR_FILE(REGULATOR_DEVICE("bad", PINS, ELEMENTS, ANALOG, "BAD", "PINHDR-1X03-V", KEYS))

static void rejection_cases(void) {
    refused("(b) no regulators: refused", BAD(PINS3, "", "", ",\"vout\":3.3"),
            "\"regulators\" must have 1 to 4 entries");
    refused("(b) five regulators: refused",
            BAD(RFX_PWR(1, "A") "," RFX_OUT(2, "B") "," RFX_PWR(3, "C") "," RFX_OUT(4, "D") "," RFX_PWR(5, "E") ","
                RFX_OUT(6, "F") "," RFX_PWR(7, "G") "," RFX_OUT(8, "H") "," RFX_PWR(9, "I") "," RFX_OUT(10, "J"),
                RFX_EL("linear", "A", "B", "") "," RFX_EL("linear", "C", "D", "") "," RFX_EL("linear", "E", "F", "")
                "," RFX_EL("linear", "G", "H", "") "," RFX_EL("linear", "I", "J", ""),
                "", ",\"vout\":1"),
            "\"regulators\" must have 1 to 4 entries");
    refused("(b) an unknown type: refused", BAD(PINS3, RFX_EL("inverting", "IN", "OUT", ""), "", ",\"vout\":3.3"),
            "unknown regulator type");
    refused("(b) \"vout\" on an element with \"adjust\": refused",
            BAD(RFX_PWR(1, "IN") "," RFX_GND(2, "GND") "," RFX_OUT(3, "OUT") "," RFX_IN(4, "ADJ"),
                RFX_EL("linear", "IN", "OUT", ",\"adjust\":\"ADJ\""), "\"ADJ\"", ",\"vout\":3.3,\"vref\":1.2"),
            "\"vout\" is not for a regulator with \"adjust\"");
    refused("(b) \"vref\" without \"adjust\": refused", BAD(PINS3, LIN_EL, "", ",\"vout\":3.3,\"vref\":1.2"),
            "\"vref\" is only for a regulator with \"adjust\"");
    refused("(b) no \"vout\" on a fixed element: refused", BAD(PINS3, LIN_EL, "", ",\"dropout\":0.3"),
            "\"vout\" is required for a regulator without \"adjust\"");
    refused("(b) an array of the wrong length: refused", BAD(PINS3, LIN_EL, "", ",\"vout\":[3.3,5]"),
            "must have one value per regulator");
    refused("(b) \"vout\" above 100: refused", BAD(PINS3, LIN_EL, "", ",\"vout\":101"),
            "\"vout\" must be above 0 and at most 100");
    refused("(b) \"dropout\" on a switching element: refused",
            BAD(RFX_PWR(1, "IN") "," RFX_GND(2, "GND") "," RFX_OUT(3, "OUT"), RFX_EL("buck", "IN", "OUT", ""), "",
                ",\"vout\":3.3,\"dropout\":0.3"),
            "\"dropout\" is only for a linear regulator");
    refused("(b) \"fsw\" on a linear element: refused", BAD(PINS3, LIN_EL, "", ",\"vout\":3.3,\"fsw\":1000000"),
            "\"fsw\" is only for a switching regulator");
    refused("(b) an \"in\" pin that is not power: refused",
            BAD(RFX_IN(1, "IN") "," RFX_GND(2, "GND") "," RFX_OUT(3, "OUT"), LIN_EL, "", ",\"vout\":3.3"),
            "regulator pin \"IN\" (in) must be power");
    refused("(b) an \"out\" pin that is not an output: refused",
            BAD(RFX_PWR(1, "IN") "," RFX_GND(2, "GND") "," RFX_IN(3, "OUT"), LIN_EL, "", ",\"vout\":3.3"),
            "regulator pin \"OUT\" (out) must be output");
    refused("(b) a pin in no regulator and not analog: refused",
            BAD(PINS3 "," RFX_IN(4, "X"), LIN_EL, "", ",\"vout\":3.3"), "pin \"X\" is in no regulator and not analog");
    refused("(b) an \"adjust\" pin that is not analog: refused",
            BAD(PINS3 "," RFX_IN(4, "ADJ"), RFX_EL("linear", "IN", "OUT", ",\"adjust\":\"ADJ\""), "", ",\"vref\":1.2"),
            "(adjust) must be in \"analog\"");
    refused("(b) \"enableActive\" without \"enable\": refused",
            BAD(PINS3, RFX_EL("linear", "IN", "OUT", ",\"enableActive\":\"low\""), "", ",\"vout\":3.3"),
            "\"enableActive\" needs \"enable\"");
    refused("(b) an analog name that is no pin: refused", BAD(PINS3, LIN_EL, "\"NOPE\"", ",\"vout\":3.3"), "NOPE");
}

/* ---- engine cases -------------------------------------------------------- */

#define MAX_NETS 8
#define MAX_DRIVES 16
#define NO_NET ((size_t)-1)

typedef struct {
    size_t net;
    SimTime time;
    LogicValue value;
    DriveStrength strength;
} Drive;

typedef struct {
    size_t net_count;
    Drive drives[MAX_DRIVES];
    size_t drive_count;
} Script;

static void *pass_config(const void *config) {
    return (void *)config;
}

static size_t driver_pin_count(const void *config) {
    return ((const Script *)config)->net_count;
}

static void driver_start(Sim *sim, SimInstance *instance, void *state) {
    const Script *script = state;
    size_t i;

    for (i = 0; i < script->drive_count; i++) {
        const Drive *d = &script->drives[i];

        sim_schedule_drive(sim, instance, d->net, d->value, d->strength, d->time);
    }
}

static const SimComponentType DRIVER_TYPE = {
    "test driver", driver_pin_count, pass_config, NULL, driver_start, NULL, NULL
};

typedef struct {
    Sim *sim;
    SimNet *nets[MAX_NETS];
    int started;
} Rig;

/* A driver on nets 0 .. net_count - 1, and the part with its pin i on net
   pin_net[i] (NO_NET: not connected). */
static void rig_build(Rig *c, const Script *script, const char *part_name, const size_t *pin_net) {
    const Part *part = part_library_find(library, part_name);
    SimInstance *driver;
    SimInstance *q;
    size_t i;

    memset(c, 0, sizeof(*c));
    c->sim = sim_create();
    driver = sim_add_instance(c->sim, &DRIVER_TYPE, "driver", script);
    for (i = 0; i < script->net_count; i++) {
        char name[8];

        snprintf(name, sizeof(name), "N%zu", i);
        c->nets[i] = sim_add_net(c->sim, name);
        sim_connect(c->sim, c->nets[i], driver, i);
    }
    q = sim_add_instance(c->sim, kind_model(part), "U1", part);
    for (i = 0; i < part->pin_count; i++) {
        if (pin_net[i] != NO_NET) {
            sim_connect(c->sim, c->nets[pin_net[i]], q, i);
        }
    }
}

/* Starts (the first time) and runs to t (ps); 1 when the engine reported no error. */
static int rig_run(Rig *c, SimTime t, int quiet) {
    if (!c->started) {
        c->started = 1;
        if (!sim_start(c->sim)) {
            if (!quiet) {
                printf("  engine error: %s\n", sim_error(c->sim));
            }
            return 0;
        }
    }
    if (!sim_run_until(c->sim, t)) {
        if (!quiet) {
            printf("  engine error: %s\n", sim_error(c->sim));
        }
        return 0;
    }
    return 1;
}

static LogicValue value(const Rig *c, size_t net) {
    return sim_net_value(c->nets[net]);
}

#define S DRIVE_STRONG
#define W DRIVE_WEAK
#define L0 LOGIC_0
#define L1 LOGIC_1
#define LX LOGIC_X
#define LZ LOGIC_Z

static void engine_cases(void) {
    /* RFX-LDO33: pins 1 IN, 2 GND, 3 EN, 4 PG, 5 OUT on nets 0 IN, 1 (GND), 2 EN, 3 PG (pulled up), 4 OUT. */
    size_t pins[] = { 0, 1, 2, 3, 4 };

    /* (c) IN 1, EN 1: OUT reads 1 after 1 ns, PG Z (the pull-up reads 1); EN 0: OUT Z, PG 0. */
    {
        Script s = { 5, { {1, 0, L0, S}, {0, 0, L1, S}, {2, 0, L1, S}, {3, 0, L1, W}, {2, 3000, L0, S} }, 5 };
        Rig c;
        int ok;

        rig_build(&c, &s, "RFX-LDO33", pins);
        ok = rig_run(&c, 500, 0) && value(&c, 4) == LZ;
        ok = ok && rig_run(&c, 1500, 0) && value(&c, 4) == L1 && value(&c, 3) == L1;
        ok = ok && rig_run(&c, 3500, 0) && value(&c, 4) == L1;
        ok = ok && rig_run(&c, 4500, 0) && value(&c, 4) == LZ && value(&c, 3) == L0;
        expect(ok, "(c) RFX-LDO33: IN 1, EN 1: OUT 1 after 1 ns, PG Z; EN 0 (1 ns later): OUT Z, PG 0");
        sim_destroy(c.sim);
    }
    /* (d) IN not connected: OUT Z, PG Z. */
    {
        Script s = { 5, { {1, 0, L0, S}, {2, 0, L1, S}, {3, 0, L1, W} }, 3 };
        size_t open[] = { NO_NET, 1, 2, 3, 4 };
        Rig c;
        int ok;

        rig_build(&c, &s, "RFX-LDO33", open);
        ok = rig_run(&c, 3000, 0) && value(&c, 4) == LZ && value(&c, 3) == L1;
        expect(ok, "(d) RFX-LDO33, IN not connected: OUT Z, PG Z (the pull-up wins)");
        sim_destroy(c.sim);
    }
    /* (e) EN Z (nothing drives it): OUT Z, PG 0: a floating enable is inactive. */
    {
        Script s = { 5, { {1, 0, L0, S}, {0, 0, L1, S}, {3, 0, L1, W} }, 3 };
        Rig c;
        int ok;

        rig_build(&c, &s, "RFX-LDO33", pins);
        ok = rig_run(&c, 3000, 0) && value(&c, 4) == LZ && value(&c, 3) == L0;
        expect(ok, "(e) RFX-LDO33, EN floating: OUT Z, PG 0 (a floating enable is inactive)");
        sim_destroy(c.sim);
    }
    /* (f) IN X: OUT X. */
    {
        Script s = { 5, { {1, 0, L0, S}, {0, 0, LX, S}, {2, 0, L1, S} }, 3 };
        Rig c;
        int ok;

        rig_build(&c, &s, "RFX-LDO33", pins);
        ok = rig_run(&c, 3000, 0) && value(&c, 4) == LX;
        expect(ok, "(f) RFX-LDO33, IN X: OUT X");
        sim_destroy(c.sim);
    }
    /* (g) a 0.5 ns pulse on EN never moves OUT. */
    {
        Script s = { 5, { {1, 0, L0, S}, {0, 0, L1, S}, {2, 0, L0, S}, {2, 2000, L1, S}, {2, 2500, L0, S} }, 5 };
        Rig c;
        int ok;

        rig_build(&c, &s, "RFX-LDO33", pins);
        ok = rig_run(&c, 5000, 0) && value(&c, 4) == LZ;
        expect(ok, "(g) RFX-LDO33: a 0.5 ns pulse on EN never moves OUT");
        sim_destroy(c.sim);
    }
    /* (h) RFX-DUAL: pins 1 IN1, 2 IN2, 3 OUT1, 4 OUT2, 5 EN1, 6 EN2, 7 GND; EN1 1, EN2 0. */
    {
        Script s = { 6, { {0, 0, L1, S}, {4, 0, L1, S}, {5, 0, L0, S} }, 3 };
        size_t dual[] = { 0, 0, 1, 2, 4, 5, 3 };
        Rig c;
        int ok;

        rig_build(&c, &s, "RFX-DUAL", dual);
        ok = rig_run(&c, 3000, 0) && value(&c, 1) == L1 && value(&c, 2) == LZ;
        expect(ok, "(h) RFX-DUAL: EN1 1, EN2 0: OUT1 1, OUT2 Z");
        sim_destroy(c.sim);
    }
    /* (i) RFX-LDO5's OUT on a net a strong 0 holds: the engine's short, a smoke error. */
    {
        Script s = { 3, { {0, 0, L1, S}, {1, 0, L0, S}, {2, 0, L0, S} }, 3 };
        size_t short_pins[] = { 0, 1, 2 };   /* IN, GND, OUT on a net held at 0 */
        Rig c;
        const char *error;
        int ok;

        rig_build(&c, &s, "RFX-LDO5", short_pins);
        ok = !rig_run(&c, 3000, 1);
        error = sim_error(c.sim);
        ok = ok && error != NULL && strncmp(error, "smoke:", 6) == 0;
        expect(ok, "(i) RFX-LDO5's OUT on GND with IN 1: a smoke error (the engine's short)");
        sim_destroy(c.sim);
    }
}

/* ---- voltage cases ------------------------------------------------------- */

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

/* Starts the circuit and returns 1 when the voltage check refused it, with the problem and text. */
static int refused_start(Circuit *c, VoltageProblem *p, char *text, size_t size) {
    int refused = circuit_start(c) != 0;

    text[0] = '\0';
    memset(p, 0, sizeof(*p));
    if (refused && circuit_voltage_problem(c, p) == 1 && circuit_error(c) != NULL) {
        snprintf(text, size, "%s", circuit_error(c));
    }
    return refused;
}

/* A regulator part `name` with IN (pin `in`) on a supply, GND (pin `gnd`) on GND, optionally EN (pin `en`)
   tied to the input, its OUT (pin `out`) on a net `out_net` returned through the pointer. */
typedef struct {
    Circuit *c;
    size_t gnd_net, out_net;
} Bench;

/* The level on out_net, read through a clash with a 1.2 V supply on it: returns the regulated level in
   millivolts, 0 when the circuit starts (the net has no regulated level), 1 when it is refused for any
   other reason. */
static unsigned level_through_clash(Circuit *c, size_t out_net) {
    VoltageProblem p;
    char text[256];
    size_t probe = add(c, "1V2");

    wire(c, probe, "1", out_net);
    if (!refused_start(c, &p, text, sizeof text)) {
        return 0;               /* started: no regulated level on the net */
    }
    if (p.kind != VOLTAGE_SUPPLY_CLASH || p.min_mv != 1200) {
        return 1;               /* refused for another reason: not a level */
    }
    return (unsigned)p.mv;
}

static void voltage_cases(void) {
    VoltageProblem p;
    char text[256];

    /* (j) 5V -> RFX-LDO33 (EN on IN): a 74HC00 (2-6 V) on OUT is fine; a 0.8-2.7 V part is refused with 3.3 V. */
    {
        int k;

        for (k = 0; k < 2; k++) {
            Circuit *c = circuit_create(library);
            size_t reg = add(c, "RFX-LDO33"), v5 = add(c, "5V"), gnd = add(c, "GND");
            size_t chip = add(c, k == 0 ? "SN74HC00N" : "SN74AUC00RGYR");
            size_t hi = net(c), lo = net(c), out = net(c);

            wire(c, v5, "1", hi);
            wire(c, gnd, "1", lo);
            wire(c, reg, "1", hi);
            wire(c, reg, "3", hi);
            wire(c, reg, "2", lo);
            wire(c, reg, "5", out);
            wire(c, chip, "14", out);
            wire(c, chip, "7", lo);
            if (k == 0) {
                expect(circuit_start(c) == 0, "(j) 5 V -> RFX-LDO33 -> a 74HC00 (2-6 V): no voltage problem");
            } else {
                expect(refused_start(c, &p, text, sizeof text) && p.kind == VOLTAGE_POWER_RANGE && p.mv == 3300 &&
                           p.part == chip &&
                           strcmp(text, "voltage: 3.3 V on pin 14 (VCC) of SN74AUC00RGYR; it needs 0.8-2.7 V") == 0,
                       "(j) the same circuit with a 0.8-2.7 V part on OUT: refused with 3.3 V");
                if (strcmp(text, "voltage: 3.3 V on pin 14 (VCC) of SN74AUC00RGYR; it needs 0.8-2.7 V") != 0) {
                    printf("  text: %s\n", text);
                }
            }
            circuit_destroy(c);
        }
    }
    /* (k) 3V3 -> RFX-LDO33: 3.3 < 3.3 + 0.3, no regulated level: the 0.8-2.7 V part is not refused. */
    {
        Circuit *c = circuit_create(library);
        size_t reg = add(c, "RFX-LDO33"), v = add(c, "3V3"), gnd = add(c, "GND"), chip = add(c, "SN74AUC00RGYR");
        size_t hi = net(c), lo = net(c), out = net(c);

        wire(c, v, "1", hi);
        wire(c, gnd, "1", lo);
        wire(c, reg, "1", hi);
        wire(c, reg, "3", hi);
        wire(c, reg, "2", lo);
        wire(c, reg, "5", out);
        wire(c, chip, "14", out);
        wire(c, chip, "7", lo);
        expect(circuit_start(c) == 0, "(k) 3.3 V -> RFX-LDO33 (below 3.3 + 0.3): OUT has no level, a 0.8-2.7 V part there is not refused");
        circuit_destroy(c);
    }
    /* (l) 12V -> RFX-LDO33: the input is above its maximum. */
    {
        Circuit *c = circuit_create(library);
        size_t reg = add(c, "RFX-LDO33"), v = add(c, "12V"), gnd = add(c, "GND");
        size_t hi = net(c), lo = net(c);

        wire(c, v, "1", hi);
        wire(c, gnd, "1", lo);
        wire(c, reg, "1", hi);
        wire(c, reg, "2", lo);
        expect(refused_start(c, &p, text, sizeof text) && p.kind == VOLTAGE_REGULATOR_INPUT_OVER && p.max_mv == 6000 &&
                   p.mv == 12000 && p.part == reg && p.pin == 0 &&
                   strcmp(text, "voltage: 12 V on pin 1 (IN) of RFX-LDO33; its input max is 6 V") == 0,
               "(l) 12 V on RFX-LDO33's IN: VOLTAGE_REGULATOR_INPUT_OVER, max 6000, 'its input max is 6 V'");
        if (strcmp(text, "voltage: 12 V on pin 1 (IN) of RFX-LDO33; its input max is 6 V") != 0) {
            printf("  text: %s\n", text);
        }
        circuit_destroy(c);
    }
    /* (l2) the enable pin is rated like the input. */
    {
        Circuit *c = circuit_create(library);
        size_t reg = add(c, "RFX-LDO33"), v = add(c, "12V"), gnd = add(c, "GND");
        size_t hi = net(c), lo = net(c);

        wire(c, v, "1", hi);
        wire(c, gnd, "1", lo);
        wire(c, reg, "3", hi);
        wire(c, reg, "2", lo);
        expect(refused_start(c, &p, text, sizeof text) && p.kind == VOLTAGE_REGULATOR_INPUT_OVER && p.pin == 2,
               "(l) 12 V on RFX-LDO33's EN: refused too (the enable is rated to the input maximum)");
        circuit_destroy(c);
    }
    /* (m) RFX-LDO33's OUT on the 5V rail: a supply clash. */
    {
        Circuit *c = circuit_create(library);
        size_t reg = add(c, "RFX-LDO33"), v = add(c, "5V"), gnd = add(c, "GND");
        size_t hi = net(c), lo = net(c);

        wire(c, v, "1", hi);
        wire(c, gnd, "1", lo);
        wire(c, reg, "1", hi);
        wire(c, reg, "3", hi);
        wire(c, reg, "2", lo);
        wire(c, reg, "5", hi);
        expect(refused_start(c, &p, text, sizeof text) && p.kind == VOLTAGE_SUPPLY_CLASH && p.mv == 5000 &&
                   p.min_mv == 3300 && p.max_mv == 3300 &&
                   strcmp(text, "voltage: 3.3 V and 5 V supplies on one net (RFX-LDO33, 5V)") == 0,
               "(m) RFX-LDO33's OUT on the 5V rail: VOLTAGE_SUPPLY_CLASH, 5000 against 3300");
        if (strcmp(text, "voltage: 3.3 V and 5 V supplies on one net (RFX-LDO33, 5V)") != 0) {
            printf("  text: %s\n", text);
        }
        circuit_destroy(c);
    }
    /* (n) RFX-FB from 5V, Rtop (OUT-FB) 10 kohm, Rbottom (FB-GND) 4.7 kohm, maker resistor parts: OUT 2502 mV;
       two 10 kohm in parallel on top: 1651 mV. Pins: 1 IN, 2 OUT, 3 FB, 4 GND, 5 EN (active low, on GND). */
    {
        int top_parts;

        for (top_parts = 1; top_parts <= 2; top_parts++) {
            Circuit *c = circuit_create(library);
            size_t reg = add(c, "RFX-FB"), v = add(c, "5V"), gnd = add(c, "GND");
            size_t rt = add(c, "CRCW020110K0FKED"), rb = add(c, "CRCW02014K70FNED");
            size_t hi = net(c), lo = net(c), out = net(c), fb = net(c);
            unsigned level;

            wire(c, v, "1", hi);
            wire(c, gnd, "1", lo);
            wire(c, reg, "1", hi);
            wire(c, reg, "4", lo);
            wire(c, reg, "5", lo);
            wire(c, reg, "2", out);
            wire(c, reg, "3", fb);
            wire(c, rt, "1", out);
            wire(c, rt, "2", fb);
            wire(c, rb, "1", fb);
            wire(c, rb, "2", lo);
            if (top_parts == 2) {
                size_t rt2 = add(c, "CRCW020110K0FKED");

                wire(c, rt2, "1", out);
                wire(c, rt2, "2", fb);
            }
            level = level_through_clash(c, out);
            expect(level == (top_parts == 1 ? 2502u : 1651u),
                   top_parts == 1 ? "(n) RFX-FB from 5V, Rtop 10 kohm, Rbottom 4.7 kohm: OUT is 2502 mV"
                                  : "(n) two 10 kohm in parallel on top: OUT is 1651 mV");
            if (level != (top_parts == 1 ? 2502u : 1651u)) {
                printf("  level: %u\n", level);
            }
            circuit_destroy(c);
        }
    }
    /* (o) RFX-ADJ from 12V, R1 1 kohm (OUT-ADJ), R2 4.7 kohm (ADJ-GND): 1.25 * (1 + 4.7) = 7125 mV. Pins 1 ADJ, 2 OUT, 3 IN. */
    {
        Circuit *c = circuit_create(library);
        size_t reg = add(c, "RFX-ADJ"), v = add(c, "12V"), gnd = add(c, "GND");
        size_t r1 = add(c, "CR-02JL6----1K"), r2 = add(c, "CRCW02014K70FNED");
        size_t hi = net(c), lo = net(c), out = net(c), adj = net(c);
        unsigned level;

        wire(c, v, "1", hi);
        wire(c, gnd, "1", lo);
        wire(c, reg, "3", hi);
        wire(c, reg, "2", out);
        wire(c, reg, "1", adj);
        wire(c, r1, "1", out);
        wire(c, r1, "2", adj);
        wire(c, r2, "1", adj);
        wire(c, r2, "2", lo);
        level = level_through_clash(c, out);
        expect(level == 7125, "(o) RFX-ADJ from 12 V, R1 1 kohm (OUT-ADJ), R2 4.7 kohm (ADJ-GND): OUT is 7125 mV");
        if (level != 7125) {
            printf("  level: %u\n", level);
        }
        circuit_destroy(c);
    }
    /* (p) RFX-FB with only Rtop, or with a third resistor from FB to the 5V rail: no level. */
    {
        int k;

        for (k = 0; k < 2; k++) {
            Circuit *c = circuit_create(library);
            size_t reg = add(c, "RFX-FB"), v = add(c, "5V"), gnd = add(c, "GND");
            size_t rt = add(c, "CRCW020110K0FKED");
            size_t hi = net(c), lo = net(c), out = net(c), fb = net(c);

            wire(c, v, "1", hi);
            wire(c, gnd, "1", lo);
            wire(c, reg, "1", hi);
            wire(c, reg, "4", lo);
            wire(c, reg, "5", lo);
            wire(c, reg, "2", out);
            wire(c, reg, "3", fb);
            wire(c, rt, "1", out);
            wire(c, rt, "2", fb);
            if (k == 1) {
                size_t rb = add(c, "CRCW02014K70FNED"), rh = add(c, "CRCW02014K70FNED");

                wire(c, rb, "1", fb);
                wire(c, rb, "2", lo);
                wire(c, rh, "1", fb);
                wire(c, rh, "2", hi);
            }
            expect(level_through_clash(c, out) == 0,
                   k == 0 ? "(p) RFX-FB with only Rtop: OUT has no level"
                          : "(p) RFX-FB with a third resistor from FB to the 5V rail: OUT has no level");
            circuit_destroy(c);
        }
    }
    /* (q) GENERIC-RESISTOR on both sides: the placed values set the divider. */
    {
        int step;

        for (step = 0; step < 3; step++) {
            Circuit *c = circuit_create(library);
            size_t reg = add(c, "RFX-FB"), v = add(c, "5V"), gnd = add(c, "GND");
            size_t rt = add(c, "GENERIC-RESISTOR"), rb = add(c, "GENERIC-RESISTOR");
            size_t hi = net(c), lo = net(c), out = net(c), fb = net(c);
            unsigned level;
            unsigned want;

            wire(c, v, "1", hi);
            wire(c, gnd, "1", lo);
            wire(c, reg, "1", hi);
            wire(c, reg, "4", lo);
            wire(c, reg, "5", lo);
            wire(c, reg, "2", out);
            wire(c, reg, "3", fb);
            wire(c, rt, "1", out);
            wire(c, rt, "2", fb);
            wire(c, rb, "1", fb);
            wire(c, rb, "2", lo);
            if (step >= 1) {
                circuit_set_resistance(c, rt, 10000);
                circuit_set_resistance(c, rb, 10000);
            }
            if (step == 2) {
                circuit_set_resistance(c, rt, 30000);
            }
            want = step == 2 ? 3200 : 1600;
            level = level_through_clash(c, out);
            expect(level == want,
                   step == 0 ? "(q) two GENERIC-RESISTORs unset (1 kohm each): OUT is 1600 mV"
                   : step == 1 ? "(q) both set to 10 kohm with circuit_set_resistance: OUT is 1600 mV"
                               : "(q) the top set to 30 kohm: OUT is 3200 mV");
            if (level != want) {
                printf("  level: %u\n", level);
            }
            circuit_destroy(c);
        }
    }
    /* (q) circuit_set_resistance's refusals. */
    {
        Circuit *c = circuit_create(library);
        size_t generic = add(c, "GENERIC-RESISTOR"), maker = add(c, "CRCW020110K0FKED");

        expect(circuit_set_resistance(c, maker, 100) == -1 && circuit_error(c) != NULL &&
                   strstr(circuit_error(c), "is not a generic resistor") != NULL,
               "(q) circuit_set_resistance on a maker resistor: 'part N is not a generic resistor'");
        expect(circuit_set_resistance(c, generic, 0) == -1 && circuit_error(c) != NULL &&
                   strstr(circuit_error(c), "the resistance is out of range") != NULL,
               "(q) 0 ohm: 'the resistance is out of range'");
        expect(circuit_set_resistance(c, generic, 2e9) == -1, "(q) 2e9 ohm: refused");
        expect(circuit_set_resistance(c, 99, 100) == -1, "(q) a part that does not exist: refused");
        expect(circuit_set_resistance(c, generic, 4700) == 0, "(q) a generic resistor takes 4.7 kohm");
        circuit_destroy(c);
    }
    /* (r) RFX-BOOST from 3V3: OUT 5000 mV; from 0.5 V (below vinMin 0.9): nothing. Pins 1 VIN, 3 VOUT, 4 GND. */
    {
        int k;

        for (k = 0; k < 2; k++) {
            Circuit *c = circuit_create(library);
            size_t reg = add(c, "RFX-BOOST"), v = add(c, k == 0 ? "3V3" : "SFX-0V5"), gnd = add(c, "GND");
            size_t hi = net(c), lo = net(c), out = net(c);
            unsigned level;

            wire(c, v, "1", hi);
            wire(c, gnd, "1", lo);
            wire(c, reg, "1", hi);
            wire(c, reg, "4", lo);
            wire(c, reg, "3", out);
            level = level_through_clash(c, out);
            expect(level == (k == 0 ? 5000u : 0u),
                   k == 0 ? "(r) RFX-BOOST from 3.3 V: OUT is 5000 mV"
                          : "(r) RFX-BOOST from 0.5 V (below vinMin 0.9): OUT has no level");
            circuit_destroy(c);
        }
    }
    /* (s) EN of RFX-LDO33 on GND: no level; RFX-FB's active-low EN on 5V: no level. */
    {
        int k;

        for (k = 0; k < 2; k++) {
            Circuit *c = circuit_create(library);
            size_t reg = add(c, k == 0 ? "RFX-LDO33" : "RFX-FB"), v = add(c, "5V"), gnd = add(c, "GND");
            size_t hi = net(c), lo = net(c), out = net(c), fb = net(c);
            size_t rt = 0, rb = 0;

            wire(c, v, "1", hi);
            wire(c, gnd, "1", lo);
            if (k == 0) {
                wire(c, reg, "1", hi);
                wire(c, reg, "3", lo);
                wire(c, reg, "2", lo);
                wire(c, reg, "5", out);
            } else {
                rt = add(c, "CRCW020110K0FKED");
                rb = add(c, "CRCW02014K70FNED");
                wire(c, reg, "1", hi);
                wire(c, reg, "4", lo);
                wire(c, reg, "5", hi);
                wire(c, reg, "2", out);
                wire(c, reg, "3", fb);
                wire(c, rt, "1", out);
                wire(c, rt, "2", fb);
                wire(c, rb, "1", fb);
                wire(c, rb, "2", lo);
            }
            expect(level_through_clash(c, out) == 0,
                   k == 0 ? "(s) RFX-LDO33's EN on GND: OUT has no level"
                          : "(s) RFX-FB's active-low EN on 5 V: OUT has no level");
            circuit_destroy(c);
        }
    }
    /* (t) 12V -> RFX-LDO5 -> RFX-LDO33 -> a 0.8-2.7 V part: powered at 3.3 V (so refused with 3.3 V). */
    {
        Circuit *c = circuit_create(library);
        size_t a = add(c, "RFX-LDO5"), b = add(c, "RFX-LDO33"), v = add(c, "12V"), gnd = add(c, "GND");
        size_t chip = add(c, "SN74AUC00RGYR");
        size_t hi = net(c), lo = net(c), mid = net(c), out = net(c);

        wire(c, v, "1", hi);
        wire(c, gnd, "1", lo);
        wire(c, a, "1", hi);
        wire(c, a, "2", lo);
        wire(c, a, "3", mid);
        wire(c, b, "1", mid);
        wire(c, b, "3", mid);
        wire(c, b, "2", lo);
        wire(c, b, "5", out);
        wire(c, chip, "14", out);
        wire(c, chip, "7", lo);
        expect(refused_start(c, &p, text, sizeof text) && p.kind == VOLTAGE_POWER_RANGE && p.mv == 3300 && p.part == chip,
               "(t) 12 V -> RFX-LDO5 -> RFX-LDO33 -> a 0.8-2.7 V part: the chain delivers 3.3 V");
        circuit_destroy(c);
    }
    /* (u) RFX-GEN from 12V (no vinMax): no input problem, OUT 5000 mV. */
    {
        Circuit *c = circuit_create(library);
        size_t reg = add(c, "RFX-GEN"), v = add(c, "12V"), gnd = add(c, "GND");
        size_t hi = net(c), lo = net(c), out = net(c);

        wire(c, v, "1", hi);
        wire(c, gnd, "1", lo);
        wire(c, reg, "1", hi);
        wire(c, reg, "2", lo);
        wire(c, reg, "3", out);
        expect(level_through_clash(c, out) == 5000, "(u) RFX-GEN from 12 V (no vinMax): OUT is 5000 mV, no input problem");
        circuit_destroy(c);
    }
    /* (v) Two RFX-LDO33 outputs on one net: no clash; RFX-LDO33 and RFX-LDO5 outputs on one net: one clash. */
    {
        Circuit *c = circuit_create(library);
        size_t a = add(c, "RFX-LDO33"), b = add(c, "RFX-LDO33"), v = add(c, "5V"), gnd = add(c, "GND");
        size_t hi = net(c), lo = net(c), out = net(c);

        wire(c, v, "1", hi);
        wire(c, gnd, "1", lo);
        wire(c, a, "1", hi);
        wire(c, a, "3", hi);
        wire(c, a, "2", lo);
        wire(c, a, "5", out);
        wire(c, b, "1", hi);
        wire(c, b, "3", hi);
        wire(c, b, "2", lo);
        wire(c, b, "5", out);
        expect(circuit_start(c) == 0, "(v) two RFX-LDO33 outputs on one net: no clash");
        circuit_destroy(c);
    }
    {
        Circuit *c = circuit_create(library);
        size_t a = add(c, "RFX-LDO33"), b = add(c, "RFX-LDO5"), v12 = add(c, "12V"), v5 = add(c, "5V"), gnd = add(c, "GND");
        size_t hi = net(c), top = net(c), lo = net(c), out = net(c);

        wire(c, v5, "1", hi);
        wire(c, v12, "1", top);
        wire(c, gnd, "1", lo);
        wire(c, a, "1", hi);
        wire(c, a, "3", hi);
        wire(c, a, "2", lo);
        wire(c, a, "5", out);
        wire(c, b, "1", top);
        wire(c, b, "2", lo);
        wire(c, b, "3", out);
        expect(refused_start(c, &p, text, sizeof text) && p.kind == VOLTAGE_SUPPLY_CLASH && p.mv == 5000 &&
                   p.min_mv == 3300 && p.part == a && p.other_part == b &&
                   strcmp(text, "voltage: 3.3 V and 5 V supplies on one net (RFX-LDO33, RFX-LDO5)") == 0,
               "(v) RFX-LDO33's and RFX-LDO5's outputs on one net: one VOLTAGE_SUPPLY_CLASH, 3.3 V first");
        if (strcmp(text, "voltage: 3.3 V and 5 V supplies on one net (RFX-LDO33, RFX-LDO5)") != 0) {
            printf("  text: %s\n", text);
        }
        circuit_destroy(c);
    }
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 || load_regulator_fixture(library) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    fact_cases();
    rejection_cases();
    engine_cases();
    voltage_cases();
    part_library_destroy(library);
    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
