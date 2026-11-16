/*
 * nixie_test: the nixie kind (models/nixie.c) on fixture tubes loaded next
 * to the real part files: the loader's rules, the model (a glyph glows
 * while the anode reads 1 and its cathode reads 0), and the voltage check
 * (the anode's level carried to the cathodes; an anode below the firing
 * voltage is VOLTAGE_NIXIE_UNDER).
 *
 *   TESTNIXIE     1 A, 2 K0, 3 K1, 4 KDP; strike 170 V
 *   TESTNIXIE-2A  1 A, 2 K0, 3 A, 4 K1 (two anode pins, joined inside)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
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

#define NFX_PIN(N, NAME) "{\"number\":" #N ",\"name\":\"" NAME "\",\"role\":\"input\"}"
#define NFX_HEAD "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"nixie\",\"devices\":["
#define NFX_FACTS "\"color\":\"Orange\",\"size\":\"18 mm\",\"view\":\"side\","
#define NFX_VOLTS "\"strike\":170,\"sustain\":145,\"current\":[1,2.5],"
#define NFX_PINS4 "\"pinouts\":{\"p4\":[" NFX_PIN(1, "A") "," NFX_PIN(2, "K0") "," NFX_PIN(3, "K1") "," \
                  NFX_PIN(4, "KDP") "]},"
#define NFX_CATHODES "\"cathodes\":[{\"pin\":\"K0\",\"glyph\":\"0\"},{\"pin\":\"K1\",\"glyph\":\"1\"}," \
                     "{\"pin\":\"KDP\",\"glyph\":\"DP\"}],"
#define NFX_PART(NAME) "\"parts\":[{\"name\":\"" NAME "\",\"package\":\"Tube\",\"pinout\":\"p4\"," \
                       "\"footprint\":\"LED-DIP4\"}]}"
#define NFX_DEVICE(ID) "{\"id\":\"" ID "\",\"function\":\"Nixie\",\"description\":\"fixture\",\"drawer\":[],"

static const char NIXIE_FIXTURE[] =
    NFX_HEAD
    NFX_DEVICE("nfx") NFX_PINS4 "\"anode\":\"A\"," NFX_CATHODES NFX_FACTS NFX_VOLTS NFX_PART("TESTNIXIE") ","
    NFX_DEVICE("nfx-2a")
    "\"pinouts\":{\"p4\":[" NFX_PIN(1, "A") "," NFX_PIN(2, "K0") "," NFX_PIN(3, "A") "," NFX_PIN(4, "K1") "]},"
    "\"anode\":\"A\",\"cathodes\":[{\"pin\":\"K0\",\"glyph\":\"0\"},{\"pin\":\"K1\",\"glyph\":\"1\"}],"
    NFX_FACTS NFX_VOLTS NFX_PART("TESTNIXIE-2A")
    "]}";

/* ---- Loader ---------------------------------------------------------------- */

/* A one-device file with these kind keys must be rejected with `want` in its message. */
static void rejects(const char *keys, const char *want, const char *what) {
    char json[2048];
    char *error = NULL;
    size_t before = part_library_count(library);
    int result;

    snprintf(json, sizeof json, NFX_HEAD NFX_DEVICE("bad") NFX_PINS4 "%s" NFX_PART("BADNIXIE") "]}", keys);
    result = load_parts(library, json, "bad.json", &error);
    if (result == 0 || error == NULL || strstr(error, want) == NULL) {
        printf("  got: %s\n  want: %s\n", error != NULL ? error : "(loaded)", want);
    }
    expect(result != 0 && error != NULL && strstr(error, want) != NULL &&
               part_library_count(library) == before, what);
    free(error);
}

static void check_loader(void) {
    const Part *part = part_library_find(library, "TESTNIXIE");
    const Part *two = part_library_find(library, "TESTNIXIE-2A");

    expect(part != NULL && part->kind == PART_NIXIE && part->nixie != NULL && part->vcc == NULL,
           "a nixie part loads, with no vcc");
    if (part == NULL || two == NULL) {
        exit(1);
    }
    expect(part->nixie->glyph_count == 3 && strcmp(part->nixie->glyphs[0], "0") == 0 &&
               strcmp(part->nixie->glyphs[2], "DP") == 0 && part->nixie->cathode_pins[0] == 1 &&
               part->nixie->cathode_pins[2] == 3,
           "glyphs and their cathode pins, in file order");
    expect(part->nixie->anode_pin_count == 1 && part->nixie->anode_pins[0] == 0 &&
               two->nixie->anode_pin_count == 2 && two->nixie->anode_pins[1] == 2,
           "one anode pin, or two of one name");
    expect(part->nixie->strike_mv == 170000 && part->nixie->sustain_mv == 145000 &&
               part->nixie->current_min_ua == 1000 && part->nixie->current_max_ua == 2500 &&
               strcmp(part->nixie->view, "side") == 0 && strcmp(part->nixie->color, "Orange") == 0 &&
               strcmp(part->nixie->size, "18 mm") == 0,
           "the facts: strike, sustain, current, view, color, size");

    rejects("\"anode\":\"A\"," NFX_CATHODES "\"color\":\"Orange\",\"size\":\"18 mm\",\"view\":\"top\"," NFX_VOLTS,
            "\"view\" must be \"side\" or \"end\"", "an unknown view is rejected");
    rejects("\"anode\":\"A\"," NFX_CATHODES NFX_FACTS "\"strike\":150,\"sustain\":170,\"current\":[1,2.5],",
            "0 < \"sustain\" <= \"strike\"", "sustain above strike is rejected");
    rejects("\"anode\":\"A\"," NFX_CATHODES NFX_FACTS "\"strike\":170,\"sustain\":145,\"current\":[3,2.5],",
            "\"current\": 0 < min <= max", "a current range upside down is rejected");
    rejects("\"anode\":\"A\",\"cathodes\":[{\"pin\":\"K0\",\"glyph\":\"0\"},{\"pin\":\"K1\",\"glyph\":\"0\"}],"
            NFX_FACTS NFX_VOLTS,
            "glyph \"0\" is used twice", "a glyph on two cathodes is rejected");
    rejects("\"anode\":\"A\",\"cathodes\":[{\"pin\":\"K0\",\"glyph\":\"0\"},{\"pin\":\"K0\",\"glyph\":\"1\"}],"
            NFX_FACTS NFX_VOLTS,
            "pin \"K0\" is used twice", "a cathode pin used twice is rejected");
    rejects("\"anode\":\"K0\"," NFX_CATHODES NFX_FACTS NFX_VOLTS,
            "is the anode and a cathode", "the anode as a cathode is rejected");
    rejects("\"anode\":\"A\",\"cathodes\":[]," NFX_FACTS NFX_VOLTS,
            "\"cathodes\" must be an array of 1 to 16 cathodes", "no cathodes is rejected");
    rejects("\"anode\":\"A\",\"cathodes\":[{\"pin\":\"K7\",\"glyph\":\"7\"}]," NFX_FACTS NFX_VOLTS,
            "no pin named \"K7\"", "a cathode pin the pinout lacks is rejected");
    rejects("\"anode\":\"A\"," NFX_CATHODES NFX_FACTS NFX_VOLTS "\"vf\":[1,2],",
            "vf", "a key of another kind is rejected");
}

/* ---- Model ----------------------------------------------------------------- */

static SimInstance *add_instance(Sim *sim, const char *name) {
    const Part *part = part_library_find(library, name);

    return part != NULL ? sim_add_instance(sim, kind_model(part), name, part) : NULL;
}

/*
 * One tube, the 170 V supply on net 0 and GND on net 1. Each wire is (tube
 * pin number, net). Runs one 1 ns step and returns the lights as '0'/'1'.
 */
static int glow_of(const char *tube, const unsigned (*wires)[2], size_t wire_count, char *out,
                   size_t cap) {
    Sim *sim = sim_create();
    SimNet *nets[2] = {sim_add_net(sim, "HV"), sim_add_net(sim, "GND")};
    SimInstance *part = add_instance(sim, tube);
    SimInstance *hv = add_instance(sim, "170V");
    SimInstance *gnd = add_instance(sim, "GND");
    size_t i;
    int ok = part != NULL && hv != NULL && gnd != NULL && sim_connect(sim, nets[0], hv, 0) &&
             sim_connect(sim, nets[1], gnd, 0);

    for (i = 0; ok && i < wire_count; i++) {
        ok = sim_connect(sim, nets[wires[i][1]], part, wires[i][0] - 1);
    }
    ok = ok && sim_start(sim) && sim_run_until(sim, 1000);
    if (ok) {
        const SimComponentType *type = sim_instance_type(part);
        unsigned char values[32];
        size_t count = type->lights(sim, part, sim_instance_state(part), 0, values, sizeof values);

        for (i = 0; i < count && i + 1 < cap; i++) {
            out[i] = values[i] ? '1' : '0';
        }
        out[i] = '\0';
    }
    sim_destroy(sim);
    return ok;
}

static void check_model(void) {
    char lights[32];

    {
        static const unsigned W[][2] = {{1, 0}, {3, 1}};
        expect(glow_of("TESTNIXIE", W, 2, lights, sizeof lights) && strcmp(lights, "010") == 0,
               "anode high, cathode K1 low: glyph 1 glows alone");
    }
    {
        static const unsigned W[][2] = {{1, 0}, {2, 1}, {4, 1}};
        expect(glow_of("TESTNIXIE", W, 3, lights, sizeof lights) && strcmp(lights, "101") == 0,
               "two cathodes low: the numeral and the decimal point glow");
    }
    {
        static const unsigned W[][2] = {{2, 1}, {3, 1}};
        expect(glow_of("TESTNIXIE", W, 2, lights, sizeof lights) && strcmp(lights, "000") == 0,
               "a floating anode lights nothing");
    }
    {
        static const unsigned W[][2] = {{1, 0}, {2, 0}};
        expect(glow_of("TESTNIXIE", W, 2, lights, sizeof lights) && strcmp(lights, "000") == 0,
               "a cathode held high stays dark");
    }
    {
        static const unsigned W[][2] = {{1, 1}, {2, 1}};
        expect(glow_of("TESTNIXIE", W, 2, lights, sizeof lights) && strcmp(lights, "000") == 0,
               "an anode held low lights nothing");
    }
    {
        static const unsigned W[][2] = {{3, 0}, {4, 1}};
        expect(glow_of("TESTNIXIE-2A", W, 2, lights, sizeof lights) && strcmp(lights, "01") == 0,
               "the second anode pin works alone (the two are joined inside)");
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

/* A supply on a new net; returns the net. */
static size_t supply(Netlist *n, const char *name) {
    size_t s = add(n, name);
    size_t net_index = n->net_count++;

    wire(n, s, "1", net_index);
    return net_index;
}

static size_t check(Netlist *n) {
    n->problem_count = voltage_check(n->parts, n->part_count, n->net_count, n->connections,
                                     n->connection_count, NULL, 0, n->problems,
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

    /* The anode straight on 170 V, a cathode on ground. */
    memset(&n, 0, sizeof n);
    tube = add(&n, "TESTNIXIE");
    wire(&n, tube, "1", supply(&n, "170V"));
    wire(&n, tube, "2", supply(&n, "GND"));
    expect(check(&n) == 0, "170 V on the anode of a 170 V tube is fine");

    /* The anode through its resistor (a pull-up to the 170 V rail). */
    memset(&n, 0, sizeof n);
    tube = add(&n, "TESTNIXIE");
    wire(&n, tube, "1", supply(&n, "PULLUP-170V"));
    wire(&n, tube, "2", supply(&n, "GND"));
    expect(check(&n) == 0, "the anode on a pull-up to 170 V is fine");

    /* A logic supply cannot light the tube. */
    memset(&n, 0, sizeof n);
    tube = add(&n, "TESTNIXIE");
    wire(&n, tube, "1", supply(&n, "5V"));
    wire(&n, tube, "2", supply(&n, "GND"));
    expect(check(&n) == 1 && n.problems[0].kind == VOLTAGE_NIXIE_UNDER && n.problems[0].mv == 5000 &&
               n.problems[0].min_mv == 170000 && n.problems[0].part == tube && n.problems[0].pin == 0 &&
               text_is(&n, 0, "voltage: 5 V on pin 1 (A) of TESTNIXIE; it needs at least 170 V to light"),
           "5 V on the anode is NIXIE_UNDER");

    /* Either anode pin of a two-anode tube is checked. */
    memset(&n, 0, sizeof n);
    tube = add(&n, "TESTNIXIE-2A");
    wire(&n, tube, "3", supply(&n, "12V"));
    expect(check(&n) == 1 && n.problems[0].kind == VOLTAGE_NIXIE_UNDER && n.problems[0].pin == 2,
           "12 V on the second anode pin is NIXIE_UNDER");

    /* A cathode is never checked against the firing voltage. */
    memset(&n, 0, sizeof n);
    tube = add(&n, "TESTNIXIE");
    wire(&n, tube, "2", supply(&n, "5V"));
    expect(check(&n) == 0, "5 V on a cathode is no problem");

    /* The anode's 170 V reaches a logic input wired to a cathode. */
    memset(&n, 0, sizeof n);
    tube = add(&n, "TESTNIXIE");
    gate = add(&n, "VFX-HC1G00");
    wire(&n, tube, "1", supply(&n, "170V"));
    wire(&n, gate, "5", supply(&n, "5V"));
    wire(&n, gate, "3", supply(&n, "GND"));
    cathode = n.net_count++;
    wire(&n, tube, "3", cathode);
    wire(&n, gate, "1", cathode);
    expect(check(&n) == 1 && n.problems[0].kind == VOLTAGE_INPUT_OVER && n.problems[0].mv == 170000 &&
               n.problems[0].part == gate && n.problems[0].net == cathode,
           "the anode voltage is carried to a cathode net: a 5 V logic input there is INPUT_OVER");

    /* A cathode net with its own level keeps it. */
    memset(&n, 0, sizeof n);
    tube = add(&n, "TESTNIXIE");
    gate = add(&n, "VFX-HC1G00");
    wire(&n, tube, "1", supply(&n, "170V"));
    wire(&n, gate, "5", supply(&n, "5V"));
    wire(&n, gate, "3", supply(&n, "GND"));
    cathode = n.net_count++;
    wire(&n, tube, "3", cathode);
    wire(&n, gate, "4", cathode);
    expect(check(&n) == 0, "a cathode driven by a powered output keeps the output's level");
}

int main(void) {
    char *error = NULL;

    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 ||
        load_voltage_fixture(library) != 0) {
        return 1;
    }
    if (load_parts(library, NIXIE_FIXTURE, "nixie-fixture.json", &error) != 0) {
        fprintf(stderr, "nixie fixture: %s\n", error != NULL ? error : "(none)");
        free(error);
        return 1;
    }
    check_loader();
    check_model();
    check_voltage();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d checks failed\n", failures);
        return 1;
    }
    printf("nixie_test: all checks passed\n");
    return 0;
}
