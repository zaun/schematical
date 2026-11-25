/*
 * dekatron_test: the dekatron kind (models/dekatron.c) on fixture tubes
 * loaded next to the real part files: the loader's rules, the model (a glow
 * that steps round the ring when the guides are pulsed, forward or back, on
 * a two-guide tube and on a single-pulse tube) and the voltage check (the
 * anode's level carried to the cathodes; an anode below the tube's minimum
 * supply is VOLTAGE_NIXIE_UNDER). The model runs on the real GC10B (two
 * guides, K0 on its own pin) and OG-8 (one guide, outputs on 0 3 5 9).
 *
 *   TESTDK    1 K (1-9), 2 G1, 3 A, 4 G2, 5 K0; two guides
 *   TESTDK-S  1 K (1-9), 2 G, 3 A, 4 K0; single pulse
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "parts.h"
#include "read_file.h"
#include "sim.h"
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

#define DFX_PIN(N, NAME) "{\"number\":" #N ",\"name\":\"" NAME "\",\"role\":\"input\"}"
#define DFX_HEAD "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"dekatron\",\"devices\":["
#define DFX_FACTS "\"color\":\"Orange\",\"size\":\"30 mm\",\"view\":\"end\","
#define DFX_VOLTS "\"strike\":350,\"sustain\":190,\"current\":[0.25,0.55],\"rate\":4000,"
#define DFX_PINS5 "\"pinouts\":{\"p5\":[" DFX_PIN(1, "K") "," DFX_PIN(2, "G1") "," DFX_PIN(3, "A") "," \
                  DFX_PIN(4, "G2") "," DFX_PIN(5, "K0") "]},"
#define DFX_CATHODES "\"cathodes\":{\"K0\":[0],\"K\":[1,2,3,4,5,6,7,8,9]},"
#define DFX_PART(NAME) "\"parts\":[{\"name\":\"" NAME "\",\"package\":\"Tube\",\"pinout\":\"p5\"," \
                       "\"footprint\":\"SOT23-5\"}]}"
#define DFX_DEVICE(ID) "{\"id\":\"" ID "\",\"function\":\"Dekatron\",\"description\":\"fixture\",\"drawer\":[],"

static const char DEKATRON_FIXTURE[] =
    DFX_HEAD
    DFX_DEVICE("dfx") DFX_PINS5 "\"anode\":\"A\"," DFX_CATHODES "\"guides\":[\"G1\",\"G2\"],\"drive\":\"double\","
    DFX_FACTS DFX_VOLTS DFX_PART("TESTDK") ","
    DFX_DEVICE("dfx-s")
    "\"pinouts\":{\"p4\":[" DFX_PIN(1, "K") "," DFX_PIN(2, "G") "," DFX_PIN(3, "A") "," DFX_PIN(4, "K0") "]},"
    "\"anode\":\"A\",\"cathodes\":{\"K0\":[0],\"K\":[1,2,3,4,5,6,7,8,9]},\"guides\":[\"G\"],\"drive\":\"single\","
    DFX_FACTS DFX_VOLTS
    "\"parts\":[{\"name\":\"TESTDK-S\",\"package\":\"Tube\",\"pinout\":\"p4\",\"footprint\":\"LED-DIP4\"}]}"
    "]}";

static const char SUPPLY_FIXTURE[] =
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"supply\",\"devices\":["
    "{\"id\":\"v450\",\"function\":\"450V\",\"description\":\"fixture\",\"drawer\":[],"
    "\"pinouts\":{\"pin\":[{\"number\":1,\"name\":\"VCC\",\"role\":\"power\"}]},\"volts\":450,"
    "\"parts\":[{\"name\":\"450V\",\"manufacturer\":\"Generic\",\"package\":\"\",\"pinout\":\"pin\"}]}"
    "]}";

/* ---- Loader ---------------------------------------------------------------- */

/* A one-device file with these kind keys must be rejected with `want` in its message. */
static void rejects(const char *keys, const char *want, const char *what) {
    char json[2048];
    char *error = NULL;
    size_t before = part_library_count(library);
    int result;

    snprintf(json, sizeof json, DFX_HEAD DFX_DEVICE("bad") DFX_PINS5 "%s" DFX_PART("BADDK") "]}", keys);
    result = load_parts(library, json, "bad.json", &error);
    if (result == 0 || error == NULL || strstr(error, want) == NULL) {
        printf("  got: %s\n  want: %s\n", error != NULL ? error : "(loaded)", want);
    }
    expect(result != 0 && error != NULL && strstr(error, want) != NULL &&
               part_library_count(library) == before, what);
    free(error);
}

static void check_loader(void) {
    const Part *part = part_library_find(library, "TESTDK");
    const Part *single = part_library_find(library, "TESTDK-S");
    size_t p;
    int ring = 1;

    expect(part != NULL && part->kind == PART_DEKATRON && part->dekatron != NULL && part->vcc == NULL,
           "a dekatron part loads, with no vcc");
    if (part == NULL || single == NULL) {
        exit(1);
    }
    expect(part->dekatron->two_guide == 1 && part->dekatron->guide_count == 2 &&
               part->dekatron->guide_pins[0] == 1 && part->dekatron->guide_pins[1] == 3 &&
               part->dekatron->anode_pin == 2 && single->dekatron->two_guide == 0 &&
               single->dekatron->guide_count == 1,
           "the drive, the guides and the anode");
    for (p = 0; p < 10; p++) {
        ring &= part->dekatron->position_pin[p] == (p == 0 ? 4u : 0u);
    }
    expect(ring && part->dekatron->cathode_pin_count == 2 && part->dekatron->cathode_pins[0] == 4 &&
               part->dekatron->cathode_pins[1] == 0,
           "position 0 is pin 5, positions 1-9 pin 1, and the cathode pins are in file order");
    expect(part->dekatron->strike_mv == 350000 && part->dekatron->sustain_mv == 190000 &&
               part->dekatron->current_min_ua == 250 && part->dekatron->current_max_ua == 550 &&
               part->dekatron->rate_hz == 4000 && strcmp(part->dekatron->view, "end") == 0 &&
               strcmp(part->dekatron->color, "Orange") == 0 && strcmp(part->dekatron->size, "30 mm") == 0,
           "the facts: strike, sustain, current, rate, view, color, size");

    rejects("\"anode\":\"A\"," DFX_CATHODES "\"guides\":[\"G1\"],\"drive\":\"double\"," DFX_FACTS DFX_VOLTS,
            "a \"double\" tube has exactly 2 \"guides\"", "a two-guide tube with one guide is rejected");
    rejects("\"anode\":\"A\"," DFX_CATHODES "\"guides\":[\"G1\",\"G2\"],\"drive\":\"triple\"," DFX_FACTS DFX_VOLTS,
            "\"drive\" must be \"double\" or \"single\"", "an unknown drive is rejected");
    rejects("\"anode\":\"A\"," DFX_CATHODES "\"guides\":[\"G1\",\"G2\"],\"drive\":\"double\","
            "\"color\":\"Orange\",\"size\":\"30 mm\",\"view\":\"top\"," DFX_VOLTS,
            "\"view\" must be \"side\" or \"end\"", "an unknown view is rejected");
    rejects("\"anode\":\"A\",\"cathodes\":{\"K0\":[0],\"K\":[1,2,3,4,5,6,7,8]},\"guides\":[\"G1\",\"G2\"],"
            "\"drive\":\"double\"," DFX_FACTS DFX_VOLTS,
            "\"cathodes\" must cover the positions 0 to 9 once each", "a missing position is rejected");
    rejects("\"anode\":\"A\",\"cathodes\":{\"K0\":[0,1],\"K\":[1,2,3,4,5,6,7,8,9]},\"guides\":[\"G1\",\"G2\"],"
            "\"drive\":\"double\"," DFX_FACTS DFX_VOLTS,
            "position 1 is on two cathodes", "a position on two cathodes is rejected");
    rejects("\"anode\":\"A\",\"cathodes\":{\"K0\":[0],\"K\":[1,2,3,4,5,6,7,8,10]},\"guides\":[\"G1\",\"G2\"],"
            "\"drive\":\"double\"," DFX_FACTS DFX_VOLTS,
            "a position is a whole number from 0 to 9", "position 10 is rejected");
    rejects("\"anode\":\"G1\"," DFX_CATHODES "\"guides\":[\"G1\",\"G2\"],\"drive\":\"double\"," DFX_FACTS DFX_VOLTS,
            "is the anode and a guide", "the anode as a guide is rejected");
    rejects("\"anode\":\"A\"," DFX_CATHODES "\"guides\":[\"K\",\"G2\"],\"drive\":\"double\"," DFX_FACTS DFX_VOLTS,
            "is a guide and a cathode", "a guide that is a cathode is rejected");
    rejects("\"anode\":\"A\"," DFX_CATHODES "\"guides\":[\"G1\",\"G1\"],\"drive\":\"double\"," DFX_FACTS DFX_VOLTS,
            "guide \"G1\" is used twice", "a guide used twice is rejected");
    rejects("\"anode\":\"A\"," DFX_CATHODES "\"guides\":[\"G1\",\"G2\"],\"drive\":\"double\"," DFX_FACTS
            "\"strike\":150,\"sustain\":170,\"current\":[0.25,0.55],\"rate\":4000,",
            "0 < \"sustain\" <= \"strike\"", "sustain above strike is rejected");
    rejects("\"anode\":\"A\"," DFX_CATHODES "\"guides\":[\"G1\",\"G2\"],\"drive\":\"double\"," DFX_FACTS
            "\"strike\":350,\"sustain\":190,\"current\":[3,2],\"rate\":4000,",
            "\"current\": 0 < min <= max", "a current range upside down is rejected");
    rejects("\"anode\":\"A\"," DFX_CATHODES "\"guides\":[\"G1\",\"G2\"],\"drive\":\"double\"," DFX_FACTS
            "\"strike\":350,\"sustain\":190,\"current\":[0.25,0.55],\"rate\":0,",
            "\"rate\" must be at least 1", "a rate of 0 is rejected");
    rejects("\"anode\":\"A\"," DFX_CATHODES "\"guides\":[\"G1\",\"G7\"],\"drive\":\"double\"," DFX_FACTS DFX_VOLTS,
            "no pin named \"G7\"", "a guide the pinout lacks is rejected");
    rejects("\"anode\":\"A\"," DFX_CATHODES "\"guides\":[\"G1\",\"G2\"],\"drive\":\"double\"," DFX_FACTS DFX_VOLTS
            "\"vf\":[1,2],",
            "vf", "a key of another kind is rejected");
}

/* ---- Model ----------------------------------------------------------------- */

/* A scripted driver: one pin per net, drives at given times. */
#define MAX_DRIVES 96

typedef struct {
    size_t pin;
    SimTime time;
    LogicValue value;
} Drive;

typedef struct {
    size_t pin_count;
    Drive drives[MAX_DRIVES];
    size_t drive_count;
} Script;

static void *pass_config(const void *config) {
    return (void *)config;
}

static size_t driver_pin_count(const void *config) {
    return ((const Script *)config)->pin_count;
}

static void driver_start(Sim *sim, SimInstance *instance, void *state) {
    const Script *script = state;
    size_t i;

    for (i = 0; i < script->drive_count; i++) {
        sim_schedule_drive(sim, instance, script->drives[i].pin, script->drives[i].value, DRIVE_STRONG,
                           script->drives[i].time);
    }
}

static const SimComponentType DRIVER_TYPE = {
    "test driver", driver_pin_count, pass_config, NULL, driver_start, NULL, NULL
};

typedef struct {
    Script script;
    const char *tube;
    size_t anode_pin, g1_pin, g2_pin;    /* pin indices of the tube */
} Bench;

static void at(Script *script, size_t pin, SimTime time, LogicValue value) {
    Drive *d = &script->drives[script->drive_count++];

    d->pin = pin;
    d->time = time;
    d->value = value;
}

/*
 * Runs the tube with the script's drives on three nets (anode, guide 1,
 * guide 2) until `until` and returns its lights, as '0'/'1' characters.
 */
static int run(const Bench *b, SimTime until, char *out, size_t cap) {
    Sim *sim = sim_create();
    SimNet *nets[3] = {sim_add_net(sim, "A"), sim_add_net(sim, "G1"), sim_add_net(sim, "G2")};
    SimInstance *driver = sim_add_instance(sim, &DRIVER_TYPE, "TB", &b->script);
    const Part *part = part_library_find(library, b->tube);
    SimInstance *tube = part != NULL ? sim_add_instance(sim, kind_model(part), b->tube, part) : NULL;
    int ok = driver != NULL && tube != NULL && sim_connect(sim, nets[0], driver, 0) &&
             sim_connect(sim, nets[1], driver, 1) && sim_connect(sim, nets[2], driver, 2) &&
             sim_connect(sim, nets[0], tube, b->anode_pin) && sim_connect(sim, nets[1], tube, b->g1_pin);

    if (ok && b->g2_pin != SIZE_MAX) {
        ok = sim_connect(sim, nets[2], tube, b->g2_pin);
    }
    ok = ok && sim_start(sim) && sim_run_until(sim, until);
    if (ok) {
        const SimComponentType *type = sim_instance_type(tube);
        unsigned char values[32];
        size_t count = type->lights(sim, tube, sim_instance_state(tube), until - 1, values, sizeof values);
        size_t i;

        for (i = 0; i < count && i + 1 < cap; i++) {
            out[i] = values[i] ? '1' : '0';
        }
        out[i] = '\0';
    }
    sim_destroy(sim);
    return ok;
}

static Bench two_guide_bench(const char *tube) {
    Bench b;

    memset(&b, 0, sizeof b);
    b.script.pin_count = 3;
    b.tube = tube;
    b.g2_pin = SIZE_MAX;
    return b;
}

/* The two-guide tubes: TESTDK has A on pin 3, G1 on 2, G2 on 4; GC10B has A on 4, G1 on 3, G2 on 5. */
static Bench gc10b(void) {
    Bench b = two_guide_bench("GC10B");

    b.anode_pin = 3;
    b.g1_pin = 2;
    b.g2_pin = 4;
    at(&b.script, 0, 0, LOGIC_1);
    at(&b.script, 1, 0, LOGIC_1);
    at(&b.script, 2, 0, LOGIC_1);
    return b;
}

/* One forward count: guide 1 low, guide 2 low, guide 1 high, guide 2 high, starting at `t`. */
static void forward(Bench *b, SimTime t) {
    at(&b->script, 1, t, LOGIC_0);
    at(&b->script, 2, t + 100, LOGIC_0);
    at(&b->script, 1, t + 200, LOGIC_1);
    at(&b->script, 2, t + 300, LOGIC_1);
}

/* One backward count: the same pulses the other way round. */
static void backward(Bench *b, SimTime t) {
    at(&b->script, 2, t, LOGIC_0);
    at(&b->script, 1, t + 100, LOGIC_0);
    at(&b->script, 2, t + 200, LOGIC_1);
    at(&b->script, 1, t + 300, LOGIC_1);
}

static void check_model(void) {
    char lights[32];
    Bench b;

    b = gc10b();
    expect(run(&b, 1000, lights, sizeof lights) && strcmp(lights, "1000000000") == 0,
           "the glow strikes on cathode 0");

    b = gc10b();
    forward(&b, 1000);
    expect(run(&b, 3000, lights, sizeof lights) && strcmp(lights, "0100000000") == 0,
           "two overlapping guide pulses, guide 1 first, count one step forward");

    b = gc10b();
    forward(&b, 1000);
    forward(&b, 2000);
    forward(&b, 3000);
    expect(run(&b, 5000, lights, sizeof lights) && strcmp(lights, "0001000000") == 0,
           "three forward counts: cathode 3");

    b = gc10b();
    backward(&b, 1000);
    expect(run(&b, 3000, lights, sizeof lights) && strcmp(lights, "0000000001") == 0,
           "the same pulses guide 2 first count one step backward, round to cathode 9");

    b = gc10b();
    forward(&b, 1000);
    forward(&b, 2000);
    backward(&b, 3000);
    expect(run(&b, 5000, lights, sizeof lights) && strcmp(lights, "0100000000") == 0,
           "two forward and one backward: cathode 1");

    b = gc10b();
    {
        int i;

        for (i = 0; i < 10; i++) {
            forward(&b, 1000 + (SimTime)i * 1000);
        }
    }
    expect(run(&b, 12000, lights, sizeof lights) && strcmp(lights, "1000000000") == 0,
           "ten forward counts come round to cathode 0");

    b = gc10b();
    at(&b.script, 1, 1000, LOGIC_0);
    at(&b.script, 1, 1200, LOGIC_1);
    expect(run(&b, 3000, lights, sizeof lights) && strcmp(lights, "1000000000") == 0,
           "a single guide pulse counts nothing");

    b = gc10b();
    at(&b.script, 1, 1000, LOGIC_0);
    at(&b.script, 1, 1100, LOGIC_1);
    at(&b.script, 2, 1200, LOGIC_0);
    at(&b.script, 2, 1300, LOGIC_1);
    expect(run(&b, 3000, lights, sizeof lights) && strcmp(lights, "1000000000") == 0,
           "two guide pulses that do not overlap count nothing");

    b = gc10b();
    forward(&b, 1000);
    at(&b.script, 0, 2000, LOGIC_0);
    expect(run(&b, 3000, lights, sizeof lights) && strcmp(lights, "0000000000") == 0,
           "the anode going low puts the glow out");

    b = gc10b();
    forward(&b, 1000);
    at(&b.script, 0, 2000, LOGIC_0);
    at(&b.script, 0, 2500, LOGIC_1);
    expect(run(&b, 3000, lights, sizeof lights) && strcmp(lights, "1000000000") == 0,
           "the anode back on strikes at cathode 0 again (the reset)");

    b = two_guide_bench("GC10B");
    b.anode_pin = 3;
    b.g1_pin = 2;
    b.g2_pin = 4;
    at(&b.script, 1, 0, LOGIC_1);
    at(&b.script, 2, 0, LOGIC_1);
    expect(run(&b, 1000, lights, sizeof lights) && strcmp(lights, "0000000000") == 0,
           "a floating anode lights nothing");

    /* OG-8: one guide (pin 4), anode pin 3; outputs on cathodes 0 3 5 9. */
    memset(&b, 0, sizeof b);
    b.script.pin_count = 3;
    b.tube = "OG-8";
    b.anode_pin = 2;
    b.g1_pin = 3;
    b.g2_pin = SIZE_MAX;
    at(&b.script, 0, 0, LOGIC_1);
    at(&b.script, 1, 0, LOGIC_1);
    at(&b.script, 1, 1000, LOGIC_0);
    at(&b.script, 1, 1500, LOGIC_1);
    expect(run(&b, 3000, lights, sizeof lights) && strcmp(lights, "0100000000") == 0,
           "single pulse: one negative pulse on the guide counts one step forward");

    {
        int i;

        memset(&b, 0, sizeof b);
        b.script.pin_count = 3;
        b.tube = "OG-8";
        b.anode_pin = 2;
        b.g1_pin = 3;
        b.g2_pin = SIZE_MAX;
        at(&b.script, 0, 0, LOGIC_1);
        at(&b.script, 1, 0, LOGIC_1);
        for (i = 0; i < 12; i++) {
            at(&b.script, 1, 1000 + (SimTime)i * 1000, LOGIC_0);
            at(&b.script, 1, 1500 + (SimTime)i * 1000, LOGIC_1);
        }
        expect(run(&b, 14000, lights, sizeof lights) && strcmp(lights, "0010000000") == 0,
               "single pulse: twelve pulses come round to cathode 2");
    }

    memset(&b, 0, sizeof b);
    b.script.pin_count = 3;
    b.tube = "OG-8";
    b.anode_pin = 2;
    b.g1_pin = 3;
    b.g2_pin = SIZE_MAX;
    at(&b.script, 0, 0, LOGIC_1);
    at(&b.script, 1, 0, LOGIC_1);
    at(&b.script, 1, 1000, LOGIC_0);
    expect(run(&b, 3000, lights, sizeof lights) && strcmp(lights, "0000000000") == 0,
           "single pulse: while the guide is held low the glow is between the cathodes");
}

/* ---- The real parts ------------------------------------------------------- */

static void check_parts(void) {
    static const char *const NAMES[] = {"GC10B", "Z303C", "GC10D", "OG-4", "OG-7", "OG-8", "A-102",
                                        "CK7978", "Z504S", "Z505S", "GS10C", "GS10D", "Z502S"};
    size_t i;
    int good = 1;

    for (i = 0; i < sizeof NAMES / sizeof NAMES[0]; i++) {
        const Part *part = part_library_find(library, NAMES[i]);
        size_t p;

        if (part == NULL || part->kind != PART_DEKATRON || part->dekatron == NULL ||
            part->dekatron->strike_mv < part->dekatron->sustain_mv) {
            printf("  %s: not a dekatron\n", NAMES[i]);
            good = 0;
            continue;
        }
        for (p = 0; p < 10; p++) {
            if (part->dekatron->position_pin[p] >= part->pin_count) {
                printf("  %s: position %zu has no pin\n", NAMES[i], p);
                good = 0;
            }
        }
    }
    expect(good, "every shipped dekatron loads with all ten positions on pins");
    {
        const Part *gs = part_library_find(library, "GS10C");
        const Part *og8 = part_library_find(library, "OG-8");

        expect(gs != NULL && og8 != NULL && gs->dekatron->cathode_pin_count == 10 &&
                   og8->dekatron->cathode_pin_count == 5 && og8->dekatron->two_guide == 0 &&
                   og8->dekatron->position_pin[3] != og8->dekatron->position_pin[4] &&
                   og8->dekatron->position_pin[1] == og8->dekatron->position_pin[2],
               "a selector has a pin per cathode; the OG-8 has pins for 0 3 5 9 and the rest in common");
    }
}

/* ---- Voltage check --------------------------------------------------------- */

typedef struct {
    const Part *parts[16];
    size_t part_count;
    size_t net_count;
    VoltageConnection connections[64];
    size_t connection_count;
    VoltageProblem problems[16];
    size_t problem_count;
} Netlist;

static size_t add(Netlist *n, const char *name) {
    const Part *part = part_library_find(library, name);

    if (part == NULL) {
        printf("  no part %s\n", name);
        exit(1);
    }
    n->parts[n->part_count] = part;
    return n->part_count++;
}

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

static size_t supply(Netlist *n, const char *name) {
    size_t s = add(n, name);
    size_t net_index = n->net_count++;

    wire(n, s, "1", net_index);
    return net_index;
}

static size_t check(Netlist *n) {
    n->problem_count = voltage_check(n->parts, n->part_count, n->net_count, n->connections,
                                     n->connection_count, NULL, 0, NULL, n->problems,
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

static void check_voltage(void) {
    Netlist n;
    size_t tube;
    size_t gate;
    size_t cathode;

    memset(&n, 0, sizeof n);
    tube = add(&n, "GC10B");
    wire(&n, tube, "4", supply(&n, "450V"));
    wire(&n, tube, "7", supply(&n, "GND"));
    expect(check(&n) == 0, "450 V on the anode of a GC10B (350 V minimum) is fine");

    memset(&n, 0, sizeof n);
    tube = add(&n, "GC10B");
    wire(&n, tube, "4", supply(&n, "5V"));
    expect(check(&n) == 1 && n.problems[0].kind == VOLTAGE_NIXIE_UNDER && n.problems[0].mv == 5000 &&
               n.problems[0].min_mv == 350000 && n.problems[0].part == tube && n.problems[0].pin == 3 &&
               text_is(&n, 0, "voltage: 5 V on pin 4 (A) of GC10B; it needs at least 350 V to light"),
           "5 V on the anode is NIXIE_UNDER, 350 V needed");

    memset(&n, 0, sizeof n);
    tube = add(&n, "GC10B");
    wire(&n, tube, "7", supply(&n, "5V"));
    wire(&n, tube, "3", supply(&n, "5V"));
    expect(check(&n) == 0, "5 V on a cathode or a guide is no problem");

    memset(&n, 0, sizeof n);
    tube = add(&n, "GC10B");
    gate = add(&n, "VFX-HC1G00");
    wire(&n, tube, "4", supply(&n, "450V"));
    wire(&n, gate, "5", supply(&n, "5V"));
    wire(&n, gate, "3", supply(&n, "GND"));
    cathode = n.net_count++;
    wire(&n, tube, "7", cathode);
    wire(&n, gate, "1", cathode);
    expect(check(&n) == 1 && n.problems[0].kind == VOLTAGE_INPUT_OVER && n.problems[0].mv == 450000 &&
               n.problems[0].part == gate && n.problems[0].net == cathode,
           "the anode voltage is carried to the cathode net: a 5 V logic input there is INPUT_OVER");
}

int main(void) {
    char *error = NULL;

    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 ||
        load_voltage_fixture(library) != 0) {
        return 1;
    }
    if (load_parts(library, SUPPLY_FIXTURE, "supply-fixture.json", &error) != 0 ||
        load_parts(library, DEKATRON_FIXTURE, "dekatron-fixture.json", &error) != 0) {
        fprintf(stderr, "dekatron fixture: %s\n", error != NULL ? error : "(none)");
        free(error);
        return 1;
    }
    check_loader();
    check_model();
    check_parts();
    check_voltage();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d checks failed\n", failures);
        return 1;
    }
    printf("dekatron_test: all checks passed\n");
    return 0;
}
