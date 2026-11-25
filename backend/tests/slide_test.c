/*
 * slide_test (plan-58-B): the switch kind's "states" and "rest" keys (slide
 * switches): the loader and its errors, and a slide switch in a circuit.
 * Fixture parts (inline, on top of the library):
 *   SLIDE-SPDT    SPDT On-On,      pins C A B,             states [[1],[2]]          24 V
 *   SLIDE-SPDT3   SPDT On-Off-On,  pins C A B,             states [[1],[],[2]] rest 2
 *   SLIDE-DPDT3   DPDT On-Off-On,  pins C1 A1 B1 C2 A2 B2, states [[1,3],[],[2,4]] rest 2
 *   SLIDE-SPST    SPST On-Off,     pins A B,               states [[1],[]]     rest 2
 *   SLIDE-LV      SPDT On-On,      pins C A B,             3 V (the voltage probe)
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "parts.h"
#include "read_file.h"

static int failures;

#define CHECK(cond, what)                                   \
    do {                                                    \
        if (cond) {                                         \
            printf("ok   %s\n", what);                      \
        } else {                                            \
            printf("FAIL %s\n", what);                      \
            failures++;                                     \
        }                                                   \
    } while (0)

#define PIN(N, NAME) "{'number':" #N ",'name':'" NAME "','role':'inout'}"
#define P2 "'p2':[" PIN(1, "A") "," PIN(2, "B") "]"
#define P3 "'p3':[" PIN(1, "C") "," PIN(2, "A") "," PIN(3, "B") "]"
#define P6 "'p6':[" PIN(1, "C1") "," PIN(2, "A1") "," PIN(3, "B1") "," PIN(4, "C2") "," PIN(5, "A2") "," PIN(6, "B2") "]"
#define SPDT_CONTACTS "[['C','A'],['C','B']]"
#define DPDT_CONTACTS "[['C1','A1'],['C1','B1'],['C2','A2'],['C2','B2']]"
#define HEAD "{'format':'schematical-parts','version':1,'kind':'switch','devices':["
#define DEVICE(ID, PINOUTS, CONTACTS, EXTRA, VOLTS, NAME, PINOUT, FOOTPRINT)                              \
    "{'id':'" ID "','function':'f','description':'d','drawer':['Switches & Relays','Slide'],"           \
    "'pinouts':{" PINOUTS "},'actuator':'Standard','contacts':" CONTACTS "," EXTRA "'ratedVolts':" VOLTS "," \
    "'parts':[{'name':'" NAME "','package':'THT','pinout':'" PINOUT "','footprint':'" FOOTPRINT "'}]}"

static const char FIXTURE[] =
    HEAD
    DEVICE("spdt", P3, SPDT_CONTACTS, "'states':[[1],[2]],", "24", "SLIDE-SPDT", "p3", "PINHDR-1X03-V") ","
    DEVICE("spdt3", P3, SPDT_CONTACTS, "'states':[[1],[],[2]],'rest':2,", "24", "SLIDE-SPDT3", "p3", "PINHDR-1X03-V") ","
    DEVICE("dpdt3", P6, DPDT_CONTACTS, "'states':[[1,3],[],[2,4]],'rest':2,", "24", "SLIDE-DPDT3", "p6", "SOT23-6") ","
    DEVICE("spst", P2, "[['A','B']]", "'states':[[1],[]],'rest':2,", "24", "SLIDE-SPST", "p2", "RES-0402") ","
    DEVICE("lv", P3, SPDT_CONTACTS, "'states':[[1],[2]],", "3", "SLIDE-LV", "p3", "PINHDR-1X03-V")
    "]}";

/* One device of the bad fixtures: SPDT contacts plus EXTRA keys. */
#define BAD(EXTRA)                                                                       \
    HEAD DEVICE("bad", P3, SPDT_CONTACTS, EXTRA, "24", "SLIDE-BAD", "p3", "PINHDR-1X03-V") "]}"

static PartLibrary *library;

static char *unquote(const char *text) {
    size_t n = strlen(text) + 1, i;
    char *copy = malloc(n);

    if (copy == NULL) {
        exit(1);
    }
    for (i = 0; i < n; i++) {
        copy[i] = text[i] == '\'' ? '"' : text[i];
    }
    return copy;
}

/* Loads one bad fixture; 1 when it is refused with an error containing `message`. */
static int refused(const char *fixture, const char *message) {
    char *text = unquote(fixture);
    char *error = NULL;
    int ok;

    ok = load_parts(library, text, "slide-bad.json", &error) != 0 && error != NULL &&
         strstr(error, message) != NULL;
    if (!ok) {
        printf("     got: %s\n", error != NULL ? error : "(loaded)");
    }
    free(error);
    free(text);
    return ok;
}

static uint32_t mask_of(const char *name, unsigned state) {
    uint32_t mask = 0xFFFFFFFFu;

    return part_switch_state_contacts(part_library_find(library, name), state, &mask) ? mask : 0xFFFFFFFFu;
}

/* 1. The loader: states, rest, the masks. */
static void case_loader(void) {
    const Part *spdt = part_library_find(library, "SLIDE-SPDT");
    const Part *spdt3 = part_library_find(library, "SLIDE-SPDT3");
    const Part *dpdt3 = part_library_find(library, "SLIDE-DPDT3");
    const Part *spst = part_library_find(library, "SLIDE-SPST");
    uint32_t mask = 0;

    CHECK(spdt != NULL && spdt3 != NULL && dpdt3 != NULL && spst != NULL, "fixture: the five slide parts load");
    CHECK(spdt->dip_switch->positions == 2 && spdt->dip_switch->states == 2 && spdt->dip_switch->rest == 1,
          "SLIDE-SPDT: 2 contacts, 2 states, rest defaults to 1");
    CHECK(mask_of("SLIDE-SPDT", 1) == 1 && mask_of("SLIDE-SPDT", 2) == 2, "SLIDE-SPDT: state 1 closes contact 1, state 2 contact 2");
    CHECK(spdt3->dip_switch->states == 3 && spdt3->dip_switch->rest == 2, "SLIDE-SPDT3: 3 states, rest 2");
    CHECK(mask_of("SLIDE-SPDT3", 1) == 1 && mask_of("SLIDE-SPDT3", 2) == 0 && mask_of("SLIDE-SPDT3", 3) == 2,
          "SLIDE-SPDT3: states [[1],[],[2]]");
    CHECK(dpdt3->dip_switch->positions == 4 && dpdt3->dip_switch->states == 3 && dpdt3->dip_switch->rest == 2,
          "SLIDE-DPDT3: 4 contacts, 3 states, rest 2");
    CHECK(mask_of("SLIDE-DPDT3", 1) == 5 && mask_of("SLIDE-DPDT3", 2) == 0 && mask_of("SLIDE-DPDT3", 3) == 10,
          "SLIDE-DPDT3: states [[1,3],[],[2,4]]");
    CHECK(spst->dip_switch->states == 2 && spst->dip_switch->rest == 2 && mask_of("SLIDE-SPST", 1) == 1 &&
              mask_of("SLIDE-SPST", 2) == 0,
          "SLIDE-SPST: states [[1],[]], rest 2");
    CHECK(part_switch_state_contacts(spdt, 0, &mask) == 0 && part_switch_state_contacts(spdt, 3, &mask) == 0 &&
              part_switch_state_contacts(spdt3, 4, &mask) == 0,
          "part_switch_state_contacts: state 0 and states past the last are refused");
    CHECK(part_switch_state_contacts(part_library_find(library, "5V"), 1, &mask) == 0 &&
              part_switch_state_contacts(NULL, 1, &mask) == 0,
          "part_switch_state_contacts: a part that is not a slide switch (or NULL) is refused");
}

/* 2. The loader's errors (plan-58-B § 4.1). */
static void case_errors(void) {
    CHECK(refused(BAD("'states':[[1]],"), "\"states\" must be 2 or 3 arrays of contact numbers"),
          "error: one state");
    CHECK(refused(BAD("'states':[[1],[2],[1],[2]],"), "\"states\" must be 2 or 3 arrays of contact numbers"),
          "error: four states");
    CHECK(refused(BAD("'states':[1,2],"), "\"states\" must be 2 or 3 arrays of contact numbers"),
          "error: states that are not arrays");
    CHECK(refused(BAD("'states':[[3],[2]],"), "states[0]: contact 3 does not exist (the device has 2 contacts)"),
          "error: a contact the device does not have");
    CHECK(refused(BAD("'states':[[1],[0]],"), "states[1]: contact 0 does not exist (the device has 2 contacts)"),
          "error: contact 0");
    CHECK(refused(BAD("'states':[[1,1],[2]],"), "states[0]: contact 1 twice"), "error: a contact twice in one state");
    CHECK(refused(BAD("'states':[[1],[2]],'rest':3,"), "\"rest\" must be 1 \xE2\x80\xA6 2"), "error: rest past the last state");
    CHECK(refused(BAD("'states':[[1],[2]],'rest':0,"), "\"rest\" must be 1 \xE2\x80\xA6 2"), "error: rest 0");
    CHECK(refused(BAD("'rest':1,"), "\"rest\" needs \"states\""), "error: rest without states");
    CHECK(refused(BAD("'states':[[1],[2]],'momentary':true,"), "\"states\" cannot be combined with \"momentary\""),
          "error: states with momentary");
    CHECK(refused(BAD("'states':[[1],[2]],'normallyClosed':false,"), "\"states\" cannot be combined with \"normallyClosed\""),
          "error: states with normallyClosed");
    CHECK(refused(BAD("'states':[[1],[2]],'joined':[['A','B']],"), "\"states\" cannot be combined with \"joined\""),
          "error: states with joined");
}

/* ---- A slide switch in a circuit ---------------------------------------- */

static Circuit *make(void) {
    Circuit *c = circuit_create(library);

    if (c == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    return c;
}

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

/* A 5V supply on its own net, returned. */
static size_t supply(Circuit *c) {
    size_t vcc = add(c, "5V");
    size_t n = net(c);

    wire(c, vcc, "1", n);
    return n;
}

static CircuitValue after(Circuit *c, size_t n) {
    circuit_advance(c, 1000);
    return circuit_net_value(c, n);
}

static int error_has(const Circuit *c, const char *text) {
    const char *e = circuit_error(c);

    return e != NULL && strstr(e, text) != NULL;
}

/* A light is on when it was on at any time during the last circuit_advance, so the advance
   right after a move still contains the old state: settle runs a second one. */
static int settle(Circuit *c) {
    return circuit_advance(c, 1000) != 0 && circuit_advance(c, 1000) != 0;
}

static int lit(Circuit *c, size_t part) {
    unsigned char on = 0;

    return circuit_part_lights(c, part, &on, 1) == 1 ? on : -1;
}

/* 3. SPDT On-On between a 5 V supply and two LEDs: the knob moves the 5 V from LED 1 to LED 2. */
static void case_leds(void) {
    Circuit *c = make();
    size_t top = supply(c), gnd = add(c, "GND"), low = net(c), sw = add(c, "SLIDE-SPDT");
    size_t led1 = add(c, "GENERIC-LED"), led2 = add(c, "GENERIC-LED"), a = net(c), b = net(c);

    wire(c, gnd, "1", low);
    wire(c, sw, "1", top);   /* C */
    wire(c, sw, "2", a);     /* A */
    wire(c, sw, "3", b);     /* B */
    wire(c, led1, "2", a);
    wire(c, led1, "1", low);
    wire(c, led2, "2", b);
    wire(c, led2, "1", low);
    CHECK(circuit_start(c) == 0 && circuit_advance(c, 1000) != 0 && lit(c, led1) == 1 && lit(c, led2) == 0,
          "SLIDE-SPDT: at start LED 1 is lit (common-A joined) and LED 2 is dark");
    CHECK(circuit_set_switch_state(c, sw, 2) == 0 && settle(c) && lit(c, led1) == 0 &&
              lit(c, led2) == 1,
          "SLIDE-SPDT: state 2 moves the 5 V to LED 2 (common-B joined, common-A open)");
    CHECK(circuit_set_switch_state(c, sw, 1) == 0 && settle(c) && lit(c, led1) == 1 &&
              lit(c, led2) == 0,
          "SLIDE-SPDT: state 1 moves it back to LED 1");
    CHECK(circuit_set_switch_state(c, sw, 1) == 0 && settle(c) && lit(c, led1) == 1 &&
              lit(c, led2) == 0,
          "SLIDE-SPDT: state 1 again changes nothing");
    circuit_destroy(c);
}

/* 4. DPDT On-Off-On: starts in the middle (all four contacts open); state 1 closes
   contacts 1 and 3 (C1-A1, C2-A2), state 3 contacts 2 and 4 (C1-B1, C2-B2). */
static void case_dpdt3(void) {
    Circuit *c = make();
    size_t top = supply(c), sw = add(c, "SLIDE-DPDT3");
    size_t a1 = net(c), b1 = net(c), a2 = net(c), b2 = net(c);

    wire(c, sw, "1", top);   /* C1 */
    wire(c, sw, "2", a1);
    wire(c, sw, "3", b1);
    wire(c, sw, "4", top);   /* C2 */
    wire(c, sw, "5", a2);
    wire(c, sw, "6", b2);
    CHECK(circuit_start(c) == 0 && after(c, a1) != CIRCUIT_1 && circuit_net_value(c, b1) != CIRCUIT_1 &&
              circuit_net_value(c, a2) != CIRCUIT_1 && circuit_net_value(c, b2) != CIRCUIT_1,
          "SLIDE-DPDT3: starts in its middle position, all four contacts open");
    CHECK(circuit_set_switch_state(c, sw, 1) == 0 && after(c, a1) == CIRCUIT_1 && circuit_net_value(c, a2) == CIRCUIT_1 &&
              circuit_net_value(c, b1) != CIRCUIT_1 && circuit_net_value(c, b2) != CIRCUIT_1,
          "SLIDE-DPDT3: state 1 closes contacts 1 and 3 (A1 and A2 are 1)");
    CHECK(circuit_set_switch_state(c, sw, 3) == 0 && after(c, b1) == CIRCUIT_1 && circuit_net_value(c, b2) == CIRCUIT_1 &&
              circuit_net_value(c, a1) != CIRCUIT_1 && circuit_net_value(c, a2) != CIRCUIT_1,
          "SLIDE-DPDT3: state 3 closes contacts 2 and 4 and opens 1 and 3");
    CHECK(circuit_set_switch_state(c, sw, 2) == 0 && after(c, b1) != CIRCUIT_1 && circuit_net_value(c, b2) != CIRCUIT_1 &&
              circuit_net_value(c, a1) != CIRCUIT_1 && circuit_net_value(c, a2) != CIRCUIT_1,
          "SLIDE-DPDT3: state 2 opens all four again");
    circuit_destroy(c);
}

/* 5. SPST On-Off starts open (rest 2); state 1 closes it, state 2 opens it. */
static void case_spst(void) {
    Circuit *c = make();
    size_t top = supply(c), sw = add(c, "SLIDE-SPST"), b = net(c);

    wire(c, sw, "1", top);
    wire(c, sw, "2", b);
    CHECK(circuit_start(c) == 0 && after(c, b) != CIRCUIT_1, "SLIDE-SPST: open at start (rest 2)");
    CHECK(circuit_set_switch_state(c, sw, 1) == 0 && after(c, b) == CIRCUIT_1, "SLIDE-SPST: state 1 closes it, B is 1");
    CHECK(circuit_set_switch_state(c, sw, 2) == 0 && after(c, b) == CIRCUIT_Z, "SLIDE-SPST: state 2 opens it, B is Z");
    circuit_destroy(c);
}

/* 6. An unwired common pin does not break start-up; the contacts also work before circuit_start. */
static void case_unwired(void) {
    Circuit *c = make();
    size_t sw = add(c, "SLIDE-SPDT"), a = net(c);

    wire(c, sw, "2", a);   /* A only: C and B are unwired */
    CHECK(circuit_start(c) == 0 && circuit_advance(c, 1000) != 0, "SLIDE-SPDT: unwired common and B pins, the circuit starts and runs");
    circuit_destroy(c);

    c = make();
    {
        size_t top = supply(c), sw3 = add(c, "SLIDE-SPDT3"), b = net(c);

        wire(c, sw3, "1", top);
        wire(c, sw3, "3", b);
        CHECK(circuit_set_switch_state(c, sw3, 3) == 0 && circuit_start(c) == 0 && after(c, b) == CIRCUIT_1,
              "SLIDE-SPDT3: moved to state 3 before circuit_start, B is 1 once started");
    }
    circuit_destroy(c);
}

/* 7. A bad state, a part that is not a slide switch. */
static void case_refused(void) {
    Circuit *c = make();
    size_t sw = add(c, "SLIDE-SPDT3"), vcc = add(c, "5V");

    CHECK(circuit_set_switch_state(c, sw, 0) == -1 && error_has(c, "SLIDE-SPDT3 has no position 0 (1 \xE2\x80\xA6 3)"),
          "state 0 is refused (has no position 0 (1 … 3))");
    CHECK(circuit_set_switch_state(c, sw, 4) == -1 && error_has(c, "SLIDE-SPDT3 has no position 4 (1 \xE2\x80\xA6 3)"),
          "state 4 is refused (has no position 4 (1 … 3))");
    CHECK(circuit_set_switch_state(c, vcc, 1) == -1 && error_has(c, "5V is not a slide switch"),
          "a supply is refused (is not a slide switch)");
    CHECK(circuit_set_switch_state(c, 99, 1) == -1, "a part index past the last is refused");
    circuit_destroy(c);
}

/* 8. The voltage check sees the closed contacts: SLIDE-LV (3 V) is downstream of the switch under test. */
static void case_voltage(void) {
    Circuit *c = make();
    size_t top = supply(c), sw = add(c, "SLIDE-SPDT3"), x = net(c), lv = add(c, "SLIDE-LV");

    wire(c, sw, "1", top);
    wire(c, sw, "2", x);
    wire(c, lv, "2", x);
    CHECK(circuit_start(c) == 0 && circuit_advance(c, 1000) != 0, "voltage: SLIDE-SPDT3 open at start, the run starts");
    CHECK(circuit_set_switch_state(c, sw, 1) == 0 && circuit_advance(c, 1000) == 0 && error_has(c, "rated 3 V"),
          "voltage: moving SLIDE-SPDT3 to state 1 joins 5 V to SLIDE-LV and stops the run (rated 3 V)");
    circuit_destroy(c);

    c = make();
    top = supply(c);
    sw = add(c, "SLIDE-SPDT");
    x = net(c);
    lv = add(c, "SLIDE-LV");
    wire(c, sw, "1", top);
    wire(c, sw, "2", x);
    wire(c, lv, "2", x);
    CHECK(circuit_start(c) == -1 && error_has(c, "rated 3 V"),
          "voltage: SLIDE-SPDT starts with common-A joined, circuit_start refuses (rated 3 V)");
    circuit_destroy(c);
}

int main(void) {
    char *text = unquote(FIXTURE);
    char *error = NULL;

    library = part_library_create();
    if (text == NULL || library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
        fprintf(stderr, "cannot load %s\n", PARTS_DIR);
        return 1;
    }
    if (load_parts(library, text, "slide-fixture.json", &error) != 0) {
        fprintf(stderr, "fixture: %s\n", error != NULL ? error : "(none)");
        return 1;
    }
    free(text);
    case_loader();
    case_errors();
    case_leds();
    case_dpdt3();
    case_spst();
    case_unwired();
    case_refused();
    case_voltage();
    part_library_destroy(library);
    if (failures) {
        printf("FAIL: %d checks failed\n", failures);
        return 1;
    }
    puts("PASS: slide switches (states, rest): the loader, the knob in a circuit, the voltage check");
    return 0;
}
