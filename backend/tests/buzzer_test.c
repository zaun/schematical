/*
 * buzzer_test (plan-47-J): the buzzer kind. The fixture parts of
 * support/buzzer_fixture.h load next to the real part files: the facts read
 * back, each malformed device or part is refused, and a buzzer is on (the
 * lights hooks) only while its positive pin reads 1 and its negative pin 0,
 * with the on-time counted: statically in circuits, and under a scripted 1 kHz
 * square wave on an engine rig.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buzzer_fixture.h"
#include "circuit.h"
#include "kind.h"
#include "parts.h"
#include "read_file.h"
#include "sim.h"

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

/* ---- facts and the loader (cases r, s) ----------------------------------- */

static void fact_cases(void) {
    const Part *b = part_library_find(library, "TESTBUZ-IND");
    const Part *p = part_library_find(library, "TESTBUZ-PZ");

    expect(b != NULL && b->kind == PART_BUZZER && b->buzzer != NULL && b->pin_count == 2 && b->vcc == NULL &&
               b->pins[0].role == PIN_INPUT && b->pins[1].role == PIN_INPUT &&
               same(b->buzzer->driver, "indicator") && same(b->buzzer->technology, "magnetic") &&
               b->buzzer->hz == 2300 && b->buzzer->rated_mv == 5000 && b->buzzer->min_mv == 3000 &&
               b->buzzer->max_mv == 7000 && b->buzzer->ua == 30000 && b->buzzer->spl_cdb == 8500,
           "(r) TESTBUZ-IND: a buzzer, 2 input pins, indicator, magnetic, 2300 Hz, 5000 mV (3000-7000), "
           "30000 uA, 8500 cdB");
    expect(p != NULL && p->kind == PART_BUZZER && p->pin_count == 3 && p->pins[2].role == PIN_NC &&
               same(p->buzzer->driver, "transducer") && same(p->buzzer->technology, "piezo") &&
               p->buzzer->hz == 4000 && p->buzzer->rated_mv == 0 && p->buzzer->min_mv == 0 &&
               p->buzzer->max_mv == 0 && p->buzzer->ua == 0 && p->buzzer->spl_cdb == 0,
           "(r) TESTBUZ-PZ: 3 pins, pin F nc, a piezo transducer, 4000 Hz, no other fact");
}

static void refused(const char *what, const char *json, const char *want) {
    PartLibrary *lib = part_library_create();
    char *error = NULL;
    int ok = lib != NULL && load_parts_dir(lib, PARTS_DIR) == 0 &&
             load_parts(lib, json, "bad-buzzer.json", &error) != 0 && error != NULL &&
             strstr(error, want) != NULL;

    if (!ok) {
        printf("  error: %s\n", error != NULL ? error : "(none)");
    }
    expect(ok, what);
    free(error);
    part_library_destroy(lib);
}

#define BAD(DRIVER, TECH, PINS, KEYS) \
    BUZZER_FILE(BUZZER_DEVICE("bad", DRIVER, TECH, PINS, "BAD", "PINHDR-1X02-V", KEYS))

static void rejection_cases(void) {
    refused("(s) driver self: refused", BAD("self", "piezo", BFX_TWO, ",\"hz\":1"),
            "unknown buzzer driver \"self\"");
    refused("(s) technology dynamic: refused", BAD("indicator", "dynamic", BFX_TWO, ",\"hz\":1"),
            "unknown buzzer technology \"dynamic\"");
    refused("(s) no hz: refused", BAD("indicator", "piezo", BFX_TWO, ""), "missing \"hz\"");
    refused("(s) hz 0: refused", BAD("indicator", "piezo", BFX_TWO, ",\"hz\":0"), "\"hz\" must be at least 1");
    refused("(s) a + pin with role inout: refused",
            BAD("indicator", "piezo", BFX_PIN(1, "+", "inout") "," BFX_IN(2, "-"), ",\"hz\":1"),
            "buzzer pin \"+\" must be input");
    refused("(s) voltsMin above voltsMax: refused",
            BAD("indicator", "piezo", BFX_TWO, ",\"hz\":1,\"voltsMin\":9,\"voltsMax\":5"),
            "\"voltsMin\" must be at most \"voltsMax\"");
    refused("(s) an unlisted part key: refused", BAD("indicator", "piezo", BFX_TWO, ",\"hz\":1,\"amps\":1"),
            "unknown key \"amps\"");
}

/* ---- circuit cases (t-u, w) ---------------------------------------------- */

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

static uint64_t on_time(Circuit *c, size_t part) {
    uint64_t t = 0;

    return circuit_part_light_time(c, part, &t, 1) == 1 ? t : (uint64_t)-1;
}

/* The buzzer's + on the net of `plus` ("5V", "GND" or NULL = floating), - likewise. */
static void static_case(const char *plus, const char *minus, int want_on, const char *what) {
    Circuit *c = circuit_create(library);
    size_t b = add(c, "TESTBUZ-IND");
    int ok;

    if (plus != NULL) {
        size_t s = add(c, plus), n = net(c);

        wire(c, s, "1", n);
        wire(c, b, "1", n);
    }
    if (minus != NULL) {
        size_t s = add(c, minus), n = net(c);

        wire(c, s, "1", n);
        wire(c, b, "2", n);
    }
    ok = circuit_start(c) == 0 && circuit_advance(c, 1000000000ULL) == 1;
    ok = ok && lit(c, b) == want_on && on_time(c, b) == (want_on ? 1000000000ULL : 0);
    expect(ok, what);
    circuit_destroy(c);
}

static void circuit_cases(void) {
    static_case("5V", "GND", 1, "(t) + on 5V, - on GND: on, and the on-time after 1 ms is 1000000000 ps");
    static_case("GND", "5V", 0, "(u) reversed: off, on-time 0");
    static_case(NULL, "GND", 0, "(u) + floating: off, on-time 0");
    static_case("5V", NULL, 0, "(u) - floating: off, on-time 0");
    /* (w) a buzzer between two gate outputs: on only for + = 1 and - = 0. */
    {
        int in1;
        int in2;

        for (in1 = 0; in1 < 2; in1++) {
            for (in2 = 0; in2 < 2; in2++) {
                Circuit *c = circuit_create(library);
                size_t b = add(c, "TESTBUZ-IND"), g = add(c, "SN74HC04N"), vcc = add(c, "5V"), gnd = add(c, "GND");
                size_t hi = net(c), lo = net(c), o1 = net(c), o2 = net(c);
                char what[128];
                int ok;

                wire(c, vcc, "1", hi);
                wire(c, gnd, "1", lo);
                wire(c, g, "14", hi);
                wire(c, g, "7", lo);
                wire(c, g, "1", in1 ? hi : lo);
                wire(c, g, "3", in2 ? hi : lo);
                wire(c, g, "2", o1);
                wire(c, g, "4", o2);
                wire(c, b, "1", o1);
                wire(c, b, "2", o2);
                ok = circuit_start(c) == 0 && circuit_advance(c, 1000000) == 1;
                /* o1 = !in1, o2 = !in2: on only for o1 = 1 and o2 = 0, i.e. in1 = 0 and in2 = 1 */
                ok = ok && lit(c, b) == (in1 == 0 && in2 == 1);
                snprintf(what, sizeof(what), "(w) gate outputs !%d and !%d on + and -: %s", in1, in2,
                         in1 == 0 && in2 == 1 ? "on" : "off");
                expect(ok, what);
                circuit_destroy(c);
            }
        }
    }
}

/* ---- the rig (case v) ---------------------------------------------------- */

#define MAX_DRIVES 64

typedef struct {
    size_t net;
    SimTime time;
    LogicValue value;
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
        sim_schedule_drive(sim, instance, script->drives[i].net, script->drives[i].value, DRIVE_STRONG,
                           script->drives[i].time);
    }
}

static const SimComponentType DRIVER_TYPE = {
    "test driver", driver_pin_count, pass_config, NULL, driver_start, NULL, NULL
};

/* (v) + driven with a 1 kHz square wave (0.5 ms high, 0.5 ms low) for 10 ms, - at 0. */
static void square_wave_case(void) {
    static Script script;
    Sim *sim = sim_create();
    const Part *part = part_library_find(library, "TESTBUZ-IND");
    SimInstance *driver;
    SimInstance *q;
    SimNet *plus;
    SimNet *minus;
    uint64_t t = 0;
    size_t i;
    int ok;

    script.net_count = 2;
    for (i = 0; i < 20; i++) {
        script.drives[script.drive_count].net = 0;
        script.drives[script.drive_count].time = (SimTime)i * 500000000ULL;
        script.drives[script.drive_count++].value = i % 2 == 0 ? LOGIC_1 : LOGIC_0;
    }
    script.drives[script.drive_count].net = 1;
    script.drives[script.drive_count].time = 0;
    script.drives[script.drive_count++].value = LOGIC_0;
    driver = sim_add_instance(sim, &DRIVER_TYPE, "driver", &script);
    plus = sim_add_net(sim, "P");
    minus = sim_add_net(sim, "M");
    sim_connect(sim, plus, driver, 0);
    sim_connect(sim, minus, driver, 1);
    q = sim_add_instance(sim, kind_model(part), "B1", part);
    sim_connect(sim, plus, q, 0);
    sim_connect(sim, minus, q, 1);
    ok = sim_start(sim) != 0 && sim_run_until(sim, 10000000000ULL) != 0;
    ok = ok && sim_instance_type(q)->light_time != NULL &&
         sim_instance_type(q)->light_time(sim, q, sim_instance_state(q), &t, 1) == 1 && t == 5000000000ULL;
    if (!ok) {
        printf("  on-time %llu ps\n", (unsigned long long)t);
    }
    expect(ok, "(v) a 1 kHz square wave on + for 10 ms, - at 0: the on-time is 5 ms (5000000000 ps)");
    sim_destroy(sim);
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 || load_buzzer_fixture(library) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    fact_cases();
    rejection_cases();
    circuit_cases();
    square_wave_case();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("buzzer_test: all passed\n");
    return 0;
}
