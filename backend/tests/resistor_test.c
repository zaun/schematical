/*
 * resistor_test (plan-34-E): two-way weak resistor links in the engine, the
 * `resistor` kind, and resistors in circuits of library parts.
 *
 * Engine cases (plan-34-E section 6) use a scripted driver with one pin per
 * net and fixture link components: a resistor (pins 0 a, 1 b) whose
 * `resistors` hook returns one link, and a diode (pin 0 anode, 1 cathode)
 * for the diode-OR case. Each end of a resistor sees the other end's level
 * as a weak drive; a resistor never passes a strong value, so two strong
 * drivers on its ends are no short.
 *
 * Kind cases load the fixture parts of support/resistor_fixture.h next to
 * the real part files: the facts read back, an array binds its pairs, and
 * each malformed device or part is refused. Circuit cases wire fixture and
 * library parts: a pull-up into an SN74HC04N input, an inverter lighting a
 * GENERIC-LED through a resistor, and a 5 V rail through a resistor refused
 * by a 3.3 V part's input.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "model_pins.h"
#include "net.h"
#include "parts.h"
#include "read_file.h"
#include "resistor_fixture.h"
#include "sim.h"

static int failures;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

/* ---- engine fixtures ---------------------------------------------------- */

#define MAX_NETS 8
#define MAX_DRIVES 16
#define MAX_LINKS 8

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

static size_t two_pins(const void *config) {
    (void)config;
    return 2;
}

static size_t one_link(const void *state, size_t *a, size_t *b, size_t cap) {
    (void)state;
    if (cap > 0) {
        a[0] = 0;
        b[0] = 1;
    }
    return 1;
}

static const SimComponentType RESISTOR_TYPE = {
    .name = "test resistor",
    .pin_count = two_pins,
    .create = pass_config,
    .resistors = one_link,
};

static const SimComponentType DIODE_TYPE = {
    .name = "test diode",
    .pin_count = two_pins,
    .create = pass_config,
    .diodes = one_link,
};

/* A link between nets a and b: a resistor, or a diode (a anode, b cathode). */
typedef struct {
    size_t a;
    size_t b;
    int diode;
} Link;

typedef struct {
    Sim *sim;
    SimNet *nets[MAX_NETS];
    int started;
} Rig;

static void rig_build(Rig *c, const Script *script, const Link *links, size_t link_count) {
    SimInstance *driver;
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
    for (i = 0; i < link_count; i++) {
        const SimComponentType *type = links[i].diode ? &DIODE_TYPE : &RESISTOR_TYPE;
        char name[8];
        SimInstance *r;

        snprintf(name, sizeof(name), "%c%zu", links[i].diode ? 'D' : 'R', i + 1);
        r = sim_add_instance(c->sim, type, name, type);
        sim_connect(c->sim, c->nets[links[i].a], r, 0);
        sim_connect(c->sim, c->nets[links[i].b], r, 1);
    }
}

/* Starts (the first time) and runs to t; 1 when the engine reported no error. */
static int rig_run(Rig *c, SimTime t) {
    if (!c->started) {
        c->started = 1;
        if (!sim_start(c->sim)) {
            printf("  engine error: %s\n", sim_error(c->sim));
            return 0;
        }
    }
    if (!sim_run_until(c->sim, t)) {
        printf("  engine error: %s\n", sim_error(c->sim));
        return 0;
    }
    return 1;
}

static LogicValue value(const Rig *c, size_t net) {
    return sim_net_value(c->nets[net]);
}

#define S DRIVE_STRONG
#define W DRIVE_WEAK

/* One resistor between net 0 (a) and net 1 (b). */
static void one_resistor(const char *what, const Drive *drives, size_t drive_count,
                         LogicValue want_a, LogicValue want_b) {
    Script script = { 2, {{0}}, 0 };
    Link link = { 0, 1, 0 };
    Rig c;
    size_t i;

    for (i = 0; i < drive_count; i++) {
        script.drives[i] = drives[i];
    }
    script.drive_count = drive_count;
    rig_build(&c, &script, &link, 1);
    expect(rig_run(&c, 1000) && value(&c, 0) == want_a && value(&c, 1) == want_b, what);
    sim_destroy(c.sim);
}

static void engine_cases(void) {
    {
        Drive d[] = { {0, 0, LOGIC_1, S} };
        one_resistor("strong 1 at a, nothing at b: b reads 1", d, 1, LOGIC_1, LOGIC_1);
    }
    {
        Drive d[] = { {0, 0, LOGIC_0, S} };
        one_resistor("strong 0 at a, nothing at b: b reads 0", d, 1, LOGIC_0, LOGIC_0);
    }
    {
        Drive d[] = { {0, 0, LOGIC_1, S}, {1, 0, LOGIC_0, S} };
        one_resistor("strong 1 at a, strong 0 at b: a 1, b 0, no smoke", d, 2, LOGIC_1, LOGIC_0);
    }
    {
        Drive d[] = { {0, 0, LOGIC_1, S}, {1, 0, LOGIC_0, W} };
        one_resistor("strong 1 at a, pull-down at b: b reads X", d, 2, LOGIC_1, LOGIC_X);
    }
    one_resistor("nothing at either end: both Z", NULL, 0, LOGIC_Z, LOGIC_Z);

    /* A chain: 0 - R - 1 - R - 2, strong 1 at 0. */
    {
        Script script = { 3, { {0, 0, LOGIC_1, S} }, 1 };
        Link links[] = { {0, 1, 0}, {1, 2, 0} };
        Rig c;

        rig_build(&c, &script, links, 2);
        expect(rig_run(&c, 1000) && value(&c, 1) == LOGIC_1 && value(&c, 2) == LOGIC_1,
               "a - R - b - R - c, strong 1 at a: b and c read 1");
        sim_destroy(c.sim);
    }

    /* A strong 1 at a; b driven strong 0 from 100 ps, released at 200 ps. */
    {
        Script script = { 2, { {0, 0, LOGIC_1, S}, {1, 100, LOGIC_0, S}, {1, 200, LOGIC_Z, S} }, 3 };
        Link link = { 0, 1, 0 };
        Rig c;
        int ok;

        rig_build(&c, &script, &link, 1);
        ok = rig_run(&c, 50) && value(&c, 1) == LOGIC_1;
        ok = ok && rig_run(&c, 150) && value(&c, 1) == LOGIC_0 && value(&c, 0) == LOGIC_1;
        ok = ok && rig_run(&c, 250) && value(&c, 1) == LOGIC_1;
        expect(ok, "strong 1 at a, b driven 0 then released: b follows to 0, then back to 1");
        sim_destroy(c.sim);
    }

    /* Diode-OR: gates on nets 0 and 1 into n (net 2); n - R - m (net 3). */
    {
        static const LogicValue in[4][2] = {
            {LOGIC_0, LOGIC_0}, {LOGIC_0, LOGIC_1}, {LOGIC_1, LOGIC_0}, {LOGIC_1, LOGIC_1}
        };
        Link links[] = { {0, 2, 1}, {1, 2, 1}, {2, 3, 0} };
        int ok = 1;
        size_t k;

        for (k = 0; k < 4; k++) {
            Script script = { 4, { {0, 0, in[k][0], S}, {1, 0, in[k][1], S}, {2, 0, LOGIC_0, W} }, 3 };
            Rig c;
            LogicValue want = (in[k][0] == LOGIC_1 || in[k][1] == LOGIC_1) ? LOGIC_1 : LOGIC_0;

            rig_build(&c, &script, links, 3);
            if (!rig_run(&c, 1000) || value(&c, 3) != want || value(&c, 2) != want) {
                printf("  inputs %d %d: n %d, m %d\n", (int)in[k][0], (int)in[k][1],
                       (int)value(&c, 2), (int)value(&c, 3));
                ok = 0;
            }
            sim_destroy(c.sim);
        }
        expect(ok, "diode-OR of two gates into n, n - R - m: m reads the OR");
    }
}

/* ---- kind cases ---------------------------------------------------------- */

static PartLibrary *library;

static void fact_cases(void) {
    const Part *r = part_library_find(library, "RFX-R1");
    const Part *a = part_library_find(library, "RFX-RA4");
    const Part *j = part_library_find(library, "RFX-RB3");
    int ok;
    size_t k;

    expect(r != NULL && r->kind == PART_RESISTOR && r->resistor != NULL && r->vcc == NULL &&
               r->pin_count == 2 && r->resistor->resistor_count == 1 &&
               r->resistor->milliohms == 4700000 && r->resistor->tolerance_ppm == 10000 &&
               r->resistor->power_mw == 100,
           "RFX-R1: resistors=1 milliohms=4700000 tol=10000 power=100, no vcc");
    ok = a != NULL && a->resistor != NULL && a->resistor->resistor_count == 4 &&
         a->resistor->power_mw == 63;
    for (k = 0; ok && k < 4; k++) {
        size_t x = 0;
        size_t y = 0;

        ok = resistor_pair(a, k, &x, &y) && x == k && y == 7 - k;
    }
    expect(ok && a != NULL && !resistor_pair(a, 4, &(size_t){0}, &(size_t){0}),
           "RFX-RA4: a 4-element isolated array binds pairs 1-8, 2-7, 3-6, 4-5");
    expect(j != NULL && j->resistor != NULL && j->resistor->milliohms == 0 &&
               j->resistor->tolerance_ppm == 0 && j->resistor->power_mw == 0 &&
               j->resistor->resistor_count == 3,
           "RFX-RB3: 0 ohms (a jumper value), no tolerance or power");
}

/* A one-device file that must be refused; `want` is part of the error. */
static void refused(const char *what, const char *json, const char *want) {
    PartLibrary *lib = part_library_create();
    char *error = NULL;
    int ok = lib != NULL && load_parts(lib, json, "bad-resistor.json", &error) != 0 &&
             error != NULL && strstr(error, want) != NULL;

    if (!ok) {
        printf("  error: %s\n", error != NULL ? error : "(none)");
    }
    expect(ok, what);
    free(error);
    part_library_destroy(lib);
}

#define TWO_PINS RFX_IO(1, "1") "," RFX_IO(2, "2")
#define ONE_PAIR "\"resistors\":[" RFX_PAIR("1", "2") "],"
#define BAD_PART(KEYS) RESISTOR_FILE(RESISTOR_DEVICE("bad", TWO_PINS, ONE_PAIR, "BAD", "RES-0603", KEYS))

static void rejection_cases(void) {
    refused("no \"resistors\": refused",
            RESISTOR_FILE(RESISTOR_DEVICE("bad", TWO_PINS, "", "BAD", "RES-0603", ",\"ohms\":1")),
            "resistors");
    refused("a = b: refused",
            RESISTOR_FILE(RESISTOR_DEVICE("bad", TWO_PINS, "\"resistors\":[" RFX_PAIR("1", "1") "],",
                                          "BAD", "RES-0603", ",\"ohms\":1")),
            "different pins");
    refused("an output pin: refused",
            RESISTOR_FILE(RESISTOR_DEVICE("bad", RFX_IO(1, "1") "," RFX_PIN(2, "2", "output"),
                                          ONE_PAIR, "BAD", "RES-0603", ",\"ohms\":1")),
            "must be inout");
    refused("an inout pin in no pair: refused",
            RESISTOR_FILE(RESISTOR_DEVICE("bad", TWO_PINS "," RFX_IO(3, "3"), ONE_PAIR, "BAD",
                                          "PINHDR-1X03-V", ",\"ohms\":1")),
            "is in no resistor");
    refused("no \"ohms\": refused", BAD_PART(""), "ohms");
    refused("ohms -1: refused", BAD_PART(",\"ohms\":-1"), "\"ohms\" must be");
    refused("ohms 2e9: refused", BAD_PART(",\"ohms\":2e9"), "\"ohms\" must be");
    refused("tolerance 0: refused", BAD_PART(",\"ohms\":1,\"tolerance\":0"), "\"tolerance\" must be");
    refused("tolerance 25: refused", BAD_PART(",\"ohms\":1,\"tolerance\":25"), "\"tolerance\" must be");
    refused("power 0: refused", BAD_PART(",\"ohms\":1,\"power\":0"), "\"power\" must be");
}

/* ---- circuit cases ------------------------------------------------------- */

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

static int lit(Circuit *c, size_t part) {
    unsigned char on = 0;

    return circuit_part_lights(c, part, &on, 1) == 1 ? on : -1;
}

/* An SN74HC04N on 5V and GND, its nets returned through hi and lo. */
static size_t powered_hc04(Circuit *c, size_t *hi, size_t *lo) {
    size_t g = add(c, "SN74HC04N"), vcc = add(c, "5V"), gnd = add(c, "GND");

    *hi = net(c);
    *lo = net(c);
    wire(c, vcc, "1", *hi);
    wire(c, gnd, "1", *lo);
    wire(c, g, "14", *hi);
    wire(c, g, "7", *lo);
    return g;
}

static void circuit_cases(void) {
    /* 5V - R - 1A: the input reads 1 and 1Y is 0. */
    {
        Circuit *c = circuit_create(library);
        size_t hi, lo;
        size_t g = powered_hc04(c, &hi, &lo), r = add(c, "RFX-R1");
        size_t in = net(c), out = net(c);

        wire(c, r, "1", hi);
        wire(c, r, "2", in);
        wire(c, g, "1", in);
        wire(c, g, "2", out);
        expect(circuit_start(c) == 0 && circuit_advance(c, 1000000) != 0 &&
                   circuit_net_value(c, in) == CIRCUIT_1 && circuit_net_value(c, out) == CIRCUIT_0,
               "5V through a resistor to an SN74HC04N input: the input reads 1, 1Y is 0");
        circuit_destroy(c);
    }
    /* 1Y - R - LED anode, cathode on GND; 1A on GND (1Y = 1), then on 5V. */
    {
        int k;

        for (k = 0; k < 2; k++) {
            Circuit *c = circuit_create(library);
            size_t hi, lo;
            size_t g = powered_hc04(c, &hi, &lo), r = add(c, "RFX-R1"), led = add(c, "GENERIC-LED");
            size_t out = net(c), anode = net(c);

            wire(c, g, "1", k == 0 ? lo : hi);
            wire(c, g, "2", out);
            wire(c, r, "1", out);
            wire(c, r, "2", anode);
            wire(c, led, "2", anode);
            wire(c, led, "1", lo);
            expect(circuit_start(c) == 0 && circuit_advance(c, 1000000) != 0 &&
                       lit(c, led) == (k == 0),
                   k == 0 ? "SN74HC04N 1Y = 1 through a resistor to a GENERIC-LED: lit"
                          : "SN74HC04N 1Y = 0 through a resistor to a GENERIC-LED: dark");
            circuit_destroy(c);
        }
    }
    /* 5V - R - A of a 3.3 V part (on 3V3): refused by the voltage check. */
    {
        Circuit *c = circuit_create(library);
        size_t g = add(c, "RFX-LV1G04"), r = add(c, "RFX-R1");
        size_t v5 = add(c, "5V"), v33 = add(c, "3V3"), gnd = add(c, "GND");
        size_t n5 = net(c), n33 = net(c), lo = net(c), in = net(c);
        VoltageProblem p;

        wire(c, v5, "1", n5);
        wire(c, v33, "1", n33);
        wire(c, gnd, "1", lo);
        wire(c, g, "4", n33);
        wire(c, g, "2", lo);
        wire(c, r, "1", n5);
        wire(c, r, "2", in);
        wire(c, g, "1", in);
        expect(circuit_start(c) != 0 && circuit_voltage_problem(c, &p) == 1 &&
                   p.kind == VOLTAGE_INPUT_OVER && p.part == g && p.mv == 5000 &&
                   circuit_error(c) != NULL && strncmp(circuit_error(c), "voltage:", 8) == 0,
               "5V through a resistor to a 3.3 V part's input (inputMax 3.6 V): refused");
        circuit_destroy(c);
    }
    /* A bussed array: C is pins 1 and 5, one node; 5V on pin 5 reaches pin 1
       strongly and pins 2-4 weakly (a pull-down on pin 2 makes it X). */
    {
        Circuit *c = circuit_create(library);
        size_t r = add(c, "RFX-RB3"), vcc = add(c, "5V"), pd = add(c, "PULLDOWN");
        size_t top = net(c), c1 = net(c), n2 = net(c), n3 = net(c);

        wire(c, vcc, "1", top);
        wire(c, r, "5", top);
        wire(c, r, "1", c1);
        wire(c, r, "2", n2);
        wire(c, pd, "1", n2);
        wire(c, r, "3", n3);
        expect(circuit_start(c) == 0 && circuit_advance(c, 1000) != 0 &&
                   circuit_net_value(c, c1) == CIRCUIT_1 && circuit_net_value(c, n3) == CIRCUIT_1 &&
                   circuit_net_value(c, n2) == CIRCUIT_X,
               "bussed array: 5V on C (pin 5) is 1 on C (pin 1) and on pin 3; X on pulled-down pin 2");
        circuit_destroy(c);
    }
}

int main(void) {
    engine_cases();
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 ||
        load_resistor_fixture(library) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    fact_cases();
    rejection_cases();
    circuit_cases();
    part_library_destroy(library);
    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
