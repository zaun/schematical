/*
 * tactile_test (plan-27-D): the switch kind's momentary, normallyClosed and
 * joined keys in a circuit. Fixture parts (inline, on top of the library):
 *   TACT-T2     NO, pins A B,                       24 V
 *   TACT-T4NC   NC, pins A1 A2 B1 B2, joined pairs, 24 V
 *   TACT-T4     NO, same pins and pairs,            24 V
 *   TACT-T2LV   NO, pins A B,                        3 V (the voltage probe)
 */
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
#define T2_PINS "'p2':[" PIN(1, "A") "," PIN(2, "B") "]"
#define T4_PINS "'p4':[" PIN(1, "A1") "," PIN(2, "A2") "," PIN(3, "B1") "," PIN(4, "B2") "]"
#define JOINED ",'joined':[['A1','A2'],['B1','B2']]"

static const char FIXTURE[] =
    "{'format':'schematical-parts','version':1,'kind':'switch','devices':["
    "{'id':'t2','function':'Normal Open','description':'d','drawer':['Switch','Tactile'],"
    "'pinouts':{" T2_PINS "},'actuator':'Standard','contacts':[['A','B']],'momentary':true,"
    "'ratedVolts':24,'parts':[{'name':'TACT-T2','package':'SMD','pinout':'p2','footprint':'RES-0402'}]},"
    "{'id':'t4nc','function':'Normal Close','description':'d','drawer':['Switch','Tactile'],"
    "'pinouts':{" T4_PINS "},'actuator':'Standard','contacts':[['A1','B1']]" JOINED ","
    "'momentary':true,'normallyClosed':true,'ratedVolts':24,"
    "'parts':[{'name':'TACT-T4NC','package':'THT','pinout':'p4','footprint':'SW-DIP-4'}]},"
    "{'id':'t4','function':'Normal Open','description':'d','drawer':['Switch','Tactile'],"
    "'pinouts':{" T4_PINS "},'actuator':'Standard','contacts':[['A1','B1']]" JOINED ","
    "'momentary':true,'ratedVolts':24,"
    "'parts':[{'name':'TACT-T4','package':'THT','pinout':'p4','footprint':'SW-DIP-4'}]},"
    "{'id':'t2lv','function':'Normal Open','description':'d','drawer':['Switch','Tactile'],"
    "'pinouts':{" T2_PINS "},'actuator':'Standard','contacts':[['A','B']],'momentary':true,"
    "'ratedVolts':3,'parts':[{'name':'TACT-T2LV','package':'SMD','pinout':'p2','footprint':'RES-0402'}]}"
    "]}";

static PartLibrary *library;

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

/* 1. A two-terminal normally-open switch conducts only while pressed. */
static void case_no2(void) {
    Circuit *c = make();
    size_t top = supply(c), sw = add(c, "TACT-T2"), b = net(c);
    wire(c, sw, "1", top);
    wire(c, sw, "2", b);
    CHECK(circuit_start(c) == 0 && after(c, b) != CIRCUIT_1, "TACT-T2: side B not driven at rest");
    CHECK(circuit_set_switch(c, sw, 1, 1) == 0 && after(c, b) == CIRCUIT_1, "TACT-T2: pressed, B is 1");
    CHECK(circuit_set_switch(c, sw, 1, 0) == 0 && after(c, b) == CIRCUIT_Z, "TACT-T2: released, B is Z");
    circuit_destroy(c);
}

/* 2. A normally-closed switch conducts at start and opens while pressed. */
static void case_nc4(void) {
    Circuit *c = make();
    size_t top = supply(c), sw = add(c, "TACT-T4NC"), b = net(c);
    wire(c, sw, "1", top);
    wire(c, sw, "3", b);
    CHECK(circuit_start(c) == 0 && after(c, b) == CIRCUIT_1, "TACT-T4NC: B is 1 at start");
    CHECK(circuit_set_switch(c, sw, 1, 0) == 0 && after(c, b) == CIRCUIT_Z, "TACT-T4NC: pressed (off), B is Z");
    CHECK(circuit_set_switch(c, sw, 1, 1) == 0 && after(c, b) == CIRCUIT_1, "TACT-T4NC: released (on), B is 1");
    /* 4. A joined pair is not a position. */
    CHECK(circuit_set_switch(c, sw, 2, 1) == -1 && error_has(c, "has no position 2"),
          "TACT-T4NC: position 2 is refused (has no position 2)");
    circuit_destroy(c);
}

/* 3. A four-terminal normally-open switch: A1-A2 joined at rest, B2 driven only while pressed. */
static void case_no4(void) {
    Circuit *c = make();
    size_t top = supply(c), sw = add(c, "TACT-T4"), a2 = net(c), b2 = net(c);
    wire(c, sw, "1", top);
    wire(c, sw, "2", a2);
    wire(c, sw, "4", b2);
    CHECK(circuit_start(c) == 0 && after(c, a2) == CIRCUIT_1, "TACT-T4: A2 joined to A1 at rest");
    CHECK(circuit_net_value(c, b2) == CIRCUIT_Z, "TACT-T4: B2 is Z at rest");
    CHECK(circuit_set_switch(c, sw, 1, 1) == 0 && after(c, b2) == CIRCUIT_1, "TACT-T4: pressed, B2 is 1 (B1-B2 joined)");
    CHECK(circuit_set_switch(c, sw, 1, 0) == 0 && after(c, a2) == CIRCUIT_1 && circuit_net_value(c, b2) == CIRCUIT_Z,
          "TACT-T4: released, A2 still joined, B2 Z");
    circuit_destroy(c);
}

/* 5. The voltage check sees the same closed contacts: a 3 V probe switch sits on
   the far side of the switch under test (the check compares every switch pin's
   voltage with its rating, open or closed, so the probe is downstream). */
static void case_voltage(void) {
    Circuit *c = make();
    size_t top = supply(c), sw = add(c, "TACT-T2"), x = net(c), lv = add(c, "TACT-T2LV");
    wire(c, sw, "1", top);
    wire(c, sw, "2", x);
    wire(c, lv, "1", x);
    CHECK(circuit_start(c) == 0 && circuit_advance(c, 1000) != 0, "voltage: TACT-T2 open at start, run starts");
    circuit_set_switch(c, sw, 1, 1);
    CHECK(circuit_advance(c, 1000) == 0 && error_has(c, "rated 3 V"),
          "voltage: pressing TACT-T2 stops the run (TACT-T2LV rated 3 V)");
    circuit_destroy(c);

    c = make();
    top = supply(c);
    sw = add(c, "TACT-T4NC");
    x = net(c);
    lv = add(c, "TACT-T2LV");
    wire(c, sw, "1", top);
    wire(c, sw, "3", x);
    wire(c, lv, "1", x);
    CHECK(circuit_start(c) == -1 && error_has(c, "rated 3 V"),
          "voltage: TACT-T4NC closed by default, circuit_start refuses (rated 3 V)");
    circuit_destroy(c);

    c = make();
    top = supply(c);
    sw = add(c, "TACT-T4");
    x = net(c);
    lv = add(c, "TACT-T2LV");
    wire(c, sw, "1", top);
    wire(c, sw, "2", x);
    wire(c, lv, "1", x);
    CHECK(circuit_start(c) == -1 && error_has(c, "rated 3 V"),
          "voltage: TACT-T4's joined A1-A2 carries 5 V, circuit_start refuses (rated 3 V)");
    circuit_destroy(c);
}

int main(void) {
    char *text = malloc(sizeof FIXTURE);
    char *error = NULL;
    size_t i;

    library = part_library_create();
    if (text == NULL || library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
        fprintf(stderr, "cannot load %s\n", PARTS_DIR);
        return 1;
    }
    for (i = 0; i < sizeof FIXTURE; i++) {
        text[i] = FIXTURE[i] == '\'' ? '"' : FIXTURE[i];
    }
    if (load_parts(library, text, "tactile-fixture.json", &error) != 0) {
        fprintf(stderr, "fixture: %s\n", error != NULL ? error : "(none)");
        return 1;
    }
    free(text);
    case_no2();
    case_nc4();
    case_no4();
    case_voltage();
    part_library_destroy(library);
    if (failures) {
        printf("FAIL: %d checks failed\n", failures);
        return 1;
    }
    puts("PASS: tactile switches (momentary, normally closed, joined) in a circuit and the voltage check");
    return 0;
}
