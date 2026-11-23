/*
 * relay_test (plan-47-J): the relay kind and its model. The fixture parts of
 * support/relay_fixture.h load next to the real part files: the facts read
 * back and each malformed device, pin or part is refused (cases a-d). The
 * timed cases (e-o, q) put one fixture part's model in an engine rig with a
 * scripted driver (one pin per net) and check the contacts and the light at
 * set times: the armature moves 1 ms after the coil last changed, for a
 * non-latching coil, a polarized one, a latching relay with one coil and with
 * two, a solid-state relay and a two-channel part. Circuit cases wire an
 * SN74HC04N, a transistor and supplies: a relay at rest, a tie between two
 * pins of one name, a relay that interrupts its own coil, a low-side driver,
 * and the voltage check.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "kind.h"
#include "net.h"
#include "parts.h"
#include "read_file.h"
#include "relay_fixture.h"
#include "sim.h"
#include "transistor_fixture.h"
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

/* ---- facts and the loader (cases a-d) ------------------------------------ */

static void fact_cases(void) {
    const Part *s = part_library_find(library, "TESTRLY-SPDT");
    const Part *l = part_library_find(library, "TESTRLY-L2");
    const Part *r = part_library_find(library, "TESTSSR");
    const Part *r2 = part_library_find(library, "TESTSSR2");

    expect(s != NULL && s->kind == PART_RELAY && s->relay != NULL && s->pin_count == 5 && s->vcc == NULL &&
               s->pins[0].role == PIN_INPUT && s->pins[1].role == PIN_INPUT &&
               s->pins[2].role == PIN_INOUT && s->pins[3].role == PIN_INOUT && s->pins[4].role == PIN_INOUT &&
               same(s->relay->type, "electromechanical") && s->relay->poles == 1 && s->relay->coils == 1 &&
               s->relay->latching == 0 && s->relay->channels == 1 && s->relay->coil_mv == 5000 &&
               s->relay->coil_mohm == 125000 && s->relay->contact_ma == 10000 &&
               s->relay->contact_mv == 30000 && s->relay->operate_us == 10000 &&
               s->relay->release_us == 5000 && s->relay->on_mohm == 0,
           "(a) TESTRLY-SPDT: a relay, 5 pins (input, input, inout x 3), 1 pole, 1 coil, 5000 mV, "
           "125000 mohm, 10000 mA, 30000 mV, 10000 us, 5000 us");
    expect(l != NULL && l->relay != NULL && l->relay->coils == 2 && l->relay->latching == 1 &&
               l->relay->channels == 1,
           "(a) TESTRLY-L2: 2 coils, latching, 1 channel");
    expect(r != NULL && r->relay != NULL && same(r->relay->type, "solid-state") && r->relay->coil_mv == 0 &&
               r->relay->on_mohm == 25000 && r->relay->contact_ma == 120 && r->relay->contact_mv == 60000,
           "(a) TESTSSR: solid-state, no coil voltage, 25000 mohm, 120 mA, 60000 mV");
    expect(r2 != NULL && r2->relay != NULL && r2->relay->coils == 2 && r2->relay->poles == 2 &&
               r2->relay->channels == 2,
           "(a) TESTSSR2: 2 coils, 2 poles, 2 channels");
}

static void refused(const char *what, const char *json, const char *want) {
    PartLibrary *lib = part_library_create();
    char *error = NULL;
    int ok = lib != NULL && load_parts_dir(lib, PARTS_DIR) == 0 &&
             load_parts(lib, json, "bad-relay.json", &error) != 0 && error != NULL &&
             strstr(error, want) != NULL;

    if (!ok) {
        printf("  error: %s\n", error != NULL ? error : "(none)");
    }
    expect(ok, what);
    free(error);
    part_library_destroy(lib);
}

#define POLE RFX_POLE("COM", "NO", "NC")
#define C1 RFX_COIL("A1", "A2")
#define BAD(TYPE, EXTRA, PINS, FOOTPRINT, KEYS) \
    RELAY_FILE(RELAY_DEVICE("bad", TYPE, EXTRA, PINS, "BAD", FOOTPRINT, KEYS))
#define BAD_SPDT(EXTRA, KEYS) BAD("electromechanical", EXTRA, RFX_SPDT_PINS, "PINHDR-1X05-V", KEYS)

static void rejection_cases(void) {
    refused("(b) type contactor: refused", BAD("contactor", RFX_SPDT_EXTRA, RFX_SPDT_PINS, "PINHDR-1X05-V", RFX_PLAIN_KEYS),
            "unknown relay type \"contactor\"");
    refused("(b) no coil entry: refused", BAD_SPDT(",\"coils\":[],\"contacts\":[" POLE "]", RFX_PLAIN_KEYS),
            "\"coils\" must have 1 to 8 entries");
    refused("(b) nine coil entries: refused",
            BAD_SPDT(",\"coils\":[" C1 "," C1 "," C1 "," C1 "," C1 "," C1 "," C1 "," C1 "," C1 "],\"contacts\":[" POLE "]",
                     RFX_PLAIN_KEYS),
            "\"coils\" must have 1 to 8 entries");
    refused("(b) a latching relay with 3 coils: refused",
            BAD_SPDT(",\"latching\":true,\"coils\":[" C1 "," C1 "," C1 "],\"contacts\":[" POLE "]", RFX_PLAIN_KEYS),
            "a latching relay has 1 or 2 coils");
    refused("(b) a single-coil latching relay with polarized: refused",
            BAD_SPDT(",\"latching\":true,\"coils\":[" RFX_COIL_POL("A1", "A2") "],\"contacts\":[" POLE "]",
                     RFX_PLAIN_KEYS),
            "a single-coil latching relay is not \"polarized\"");
    refused("(b) a contact with neither no nor nc: refused",
            BAD_SPDT(",\"coils\":[" C1 "],\"contacts\":[{\"common\":\"COM\"}]", RFX_PLAIN_KEYS),
            "a contact needs \"no\" or \"nc\"");
    refused("(b) a latching solid-state relay: refused",
            BAD("solid-state", ",\"latching\":true,\"coils\":[" RFX_COIL("A", "K") "],\"contacts\":[{\"common\":\"D1\",\"no\":\"D2\"}]",
                RFX_IN(1, "A") "," RFX_IN(2, "K") "," RFX_IO(3, "D1") "," RFX_IO(4, "D2"), "PINHDR-1X04-V",
                ",\"contactAmps\":1"),
            "a solid-state relay is not latching");
    refused("(b) \"coil\": 3 on a 2-coil relay: refused",
            BAD_SPDT(",\"coils\":[" C1 "," RFX_COIL("B1", "B2") "],\"contacts\":[" RFX_POLE_COIL("COM", "NO", "NC", 3) "]",
                     RFX_PLAIN_KEYS),
            "\"coil\" must be 1 to 2");
    refused("(b) a 2-coil relay whose contacts all name coil 1: refused",
            BAD_SPDT(",\"coils\":[" C1 "," RFX_COIL("B1", "B2") "],\"contacts\":[" POLE "]", RFX_PLAIN_KEYS),
            "coil 2 has no contact");
    refused("(c) a coil pin with role inout: refused",
            BAD("electromechanical", RFX_SPDT_EXTRA,
                RFX_IO(1, "A1") "," RFX_IN(2, "A2") "," RFX_IO(3, "COM") "," RFX_IO(4, "NO") "," RFX_IO(5, "NC"),
                "PINHDR-1X05-V", RFX_PLAIN_KEYS),
            "relay pin \"A1\" must be input");
    refused("(c) a pin in two entries: refused",
            BAD("electromechanical",
                ",\"coils\":[" C1 "],\"contacts\":[" POLE "," RFX_POLE("COM", "NO2", "NC2") "]",
                RFX_SPDT_PINS "," RFX_IO(6, "NO2") "," RFX_IO(7, "NC2"), "PINHDR-1X07-V", RFX_PLAIN_KEYS),
            "pin \"COM\" is in two entries");
    refused("(c) an inout pin in no entry: refused",
            BAD("electromechanical", RFX_SPDT_EXTRA, RFX_SPDT_PINS "," RFX_IO(6, "X"), "PINHDR-1X06-V",
                RFX_PLAIN_KEYS),
            "pin \"X\" is in no entry");
    refused("(d) no \"coilVolts\": refused", BAD_SPDT(RFX_SPDT_EXTRA, ",\"contactAmps\":1"),
            "missing \"coilVolts\"");
    refused("(d) \"coilVolts\" on a solid-state relay: refused",
            BAD("solid-state", ",\"coils\":[" RFX_COIL("A", "K") "],\"contacts\":[{\"common\":\"D1\",\"no\":\"D2\"}]",
                RFX_IN(1, "A") "," RFX_IN(2, "K") "," RFX_IO(3, "D1") "," RFX_IO(4, "D2"), "PINHDR-1X04-V",
                ",\"coilVolts\":5,\"contactAmps\":1"),
            "\"coilVolts\" is not a solid-state relay key");
    refused("(d) \"onOhms\" on an electromechanical relay: refused",
            BAD_SPDT(RFX_SPDT_EXTRA, ",\"coilVolts\":5,\"contactAmps\":1,\"onOhms\":1"),
            "\"onOhms\" is only for a solid-state relay");
    refused("(d) no \"contactAmps\": refused", BAD_SPDT(RFX_SPDT_EXTRA, ",\"coilVolts\":5"),
            "missing \"contactAmps\"");
}

/* ---- the engine rig ------------------------------------------------------ */

#define MAX_NETS 8
#define MAX_DRIVES 32
#define NO_NET ((size_t)-1)
/* microseconds, as picoseconds */
#define T(us) ((SimTime)(us) * 1000000ULL)

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

static void drive(Script *s, size_t net, SimTime time, LogicValue value) {
    s->drives[s->drive_count].net = net;
    s->drives[s->drive_count].time = time;
    s->drives[s->drive_count++].value = value;
}

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

typedef struct {
    Sim *sim;
    SimNet *nets[MAX_NETS];
    SimInstance *q;
    int started;
} Rig;

/* A driver on nets 0 .. net_count - 1, and the part with its pin i on net pin_net[i]. */
static void rig_build(Rig *c, const Script *script, const char *part_name, const size_t *pin_net) {
    const Part *part = part_library_find(library, part_name);
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
    c->q = sim_add_instance(c->sim, kind_model(part), "K1", part);
    for (i = 0; i < part->pin_count; i++) {
        if (pin_net[i] != NO_NET) {
            sim_connect(c->sim, c->nets[pin_net[i]], c->q, i);
        }
    }
}

/* Starts (the first time) and runs to t (ps); 1 when the engine reported no error. */
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

/* The relay's light `i` as it is now (on now, or off at or after now). */
static int rig_light(const Rig *c, size_t i) {
    unsigned char on[MAX_NETS] = {0};
    size_t n = sim_instance_type(c->q)->lights(c->sim, c->q, sim_instance_state(c->q), sim_now(c->sim), on,
                                               MAX_NETS);

    return i < n ? on[i] : -1;
}

#define L0 LOGIC_0
#define L1 LOGIC_1
#define LZ LOGIC_Z

/* The identity pin to net map for a part of n pins. */
static void straight(size_t *pin_net, size_t n) {
    size_t i;

    for (i = 0; i < n; i++) {
        pin_net[i] = i;
    }
}

static int at_rest(const Rig *c, size_t no, size_t nc) {
    return value(c, no) == LZ && value(c, nc) == L1;
}

static int operated(const Rig *c, size_t no, size_t nc) {
    return value(c, no) == L1 && value(c, nc) == LZ;
}

/* SPDT pins: A1 0, A2 1, COM 2, NO 3, NC 4. */
static void spdt_cases(void) {
    size_t pins[5];
    Rig c;
    Script s;

    straight(pins, 5);
    /* (f) operate and (g) release. */
    memset(&s, 0, sizeof(s));
    s.net_count = 5;
    drive(&s, 0, 0, L0);
    drive(&s, 0, T(2000), L1);
    drive(&s, 0, T(5000), L0);
    drive(&s, 1, 0, L0);
    drive(&s, 2, 0, L1);
    rig_build(&c, &s, "TESTRLY-SPDT", pins);
    expect(rig_run(&c, T(1500)) && at_rest(&c, 3, 4) && rig_light(&c, 0) == 0,
           "(e) A1 and A2 at 0, 5V on COM: NC reads 1, NO Z, the light is off");
    expect(rig_run(&c, T(2900)) && at_rest(&c, 3, 4), "(f) 0.9 ms after A1 goes to 1: nothing has moved");
    expect(rig_run(&c, T(3100)) && operated(&c, 3, 4) && rig_light(&c, 0) == 1,
           "(f) 1.1 ms after A1 goes to 1: NO reads 1, NC reads Z, the light is on");
    expect(rig_run(&c, T(5900)) && operated(&c, 3, 4), "(g) 0.9 ms after A1 goes back to 0: still operated");
    expect(rig_run(&c, T(6100)) && at_rest(&c, 3, 4) && rig_light(&c, 0) == 0,
           "(g) 1.1 ms after A1 goes back to 0: NC reads 1, NO reads Z, the light is off");
    sim_destroy(c.sim);
    /* (h) a 0.5 ms pulse moves nothing. */
    memset(&s, 0, sizeof(s));
    s.net_count = 5;
    drive(&s, 0, 0, L0);
    drive(&s, 0, T(2000), L1);
    drive(&s, 0, T(2500), L0);
    drive(&s, 1, 0, L0);
    drive(&s, 2, 0, L1);
    rig_build(&c, &s, "TESTRLY-SPDT", pins);
    expect(rig_run(&c, T(5000)) && at_rest(&c, 3, 4) && rig_light(&c, 0) == 0,
           "(h) a 0.5 ms pulse on A1 moves nothing (checked at 5 ms)");
    sim_destroy(c.sim);
    /* (i) reverse drive: operates a plain coil, not a polarized one. */
    {
        const char *names[2] = {"TESTRLY-SPDT", "TESTRLY-POL"};
        int k;

        for (k = 0; k < 2; k++) {
            memset(&s, 0, sizeof(s));
            s.net_count = 5;
            drive(&s, 0, 0, L0);
            drive(&s, 1, 0, L1);
            drive(&s, 2, 0, L1);
            rig_build(&c, &s, names[k], pins);
            expect(rig_run(&c, T(1100)) && (k == 0 ? operated(&c, 3, 4) : at_rest(&c, 3, 4)),
                   k == 0 ? "(i) reverse drive (A1 = 0, A2 = 1) operates TESTRLY-SPDT"
                          : "(i) reverse drive does not operate TESTRLY-POL");
            sim_destroy(c.sim);
        }
    }
    /* (j) a floating coil pin. */
    {
        size_t open[5] = {0, NO_NET, 2, 3, 4};

        memset(&s, 0, sizeof(s));
        s.net_count = 5;
        drive(&s, 0, 0, L1);
        drive(&s, 2, 0, L1);
        rig_build(&c, &s, "TESTRLY-SPDT", open);
        expect(rig_run(&c, T(3000)) && at_rest(&c, 3, 4), "(j) A2 on no net, A1 = 1: at rest");
        sim_destroy(c.sim);
    }
}

static void other_arrangement_cases(void) {
    Rig c;
    Script s;
    size_t pins[8];

    /* (k) DPDT: A1 0, A2 1, COM1 2, NO1 3, NC1 4, COM2 5, NO2 6, NC2 7. */
    straight(pins, 8);
    memset(&s, 0, sizeof(s));
    s.net_count = 8;
    drive(&s, 0, 0, L1);
    drive(&s, 1, 0, L0);
    drive(&s, 2, 0, L1);
    drive(&s, 5, 0, L1);
    rig_build(&c, &s, "TESTRLY-DPDT", pins);
    expect(rig_run(&c, T(900)) && at_rest(&c, 3, 4) && at_rest(&c, 6, 7),
           "(k) TESTRLY-DPDT before 1 ms: both poles at rest");
    expect(rig_run(&c, T(1100)) && operated(&c, 3, 4) && operated(&c, 6, 7),
           "(k) TESTRLY-DPDT at 1.1 ms: both poles changed over");
    sim_destroy(c.sim);
    /* (l) TESTRLY-L1: A1 0, A2 1, COM 2, NO 3, NC 4. */
    straight(pins, 5);
    memset(&s, 0, sizeof(s));
    s.net_count = 5;
    drive(&s, 0, 0, L0);
    drive(&s, 0, T(2000), L1);
    drive(&s, 0, T(4000), L0);
    drive(&s, 1, 0, L0);
    drive(&s, 1, T(12000), L1);
    drive(&s, 1, T(14000), L0);
    drive(&s, 2, 0, L1);
    rig_build(&c, &s, "TESTRLY-L1", pins);
    expect(rig_run(&c, T(2500)) && at_rest(&c, 3, 4), "(l) TESTRLY-L1: 0.5 ms into a forward pulse: at rest");
    expect(rig_run(&c, T(3500)) && operated(&c, 3, 4) && rig_light(&c, 0) == 1,
           "(l) TESTRLY-L1: a 2 ms forward pulse operates it");
    expect(rig_run(&c, T(10000)) && operated(&c, 3, 4), "(l) TESTRLY-L1: both pins back at 0, still operated at 10 ms");
    expect(rig_run(&c, T(13500)) && at_rest(&c, 3, 4) && rig_light(&c, 0) == 0,
           "(l) TESTRLY-L1: a 2 ms backward pulse releases it");
    sim_destroy(c.sim);
    /* (m) TESTRLY-L2: S1 0, S2 1, R1 2, R2 3, COM 4, NO 5, NC 6. */
    straight(pins, 7);
    memset(&s, 0, sizeof(s));
    s.net_count = 7;
    drive(&s, 0, 0, L0);
    drive(&s, 0, T(2000), L1);
    drive(&s, 0, T(4000), L0);
    drive(&s, 0, T(20000), L1);
    drive(&s, 0, T(22000), L0);
    drive(&s, 0, T(30000), L1);
    drive(&s, 0, T(32000), L0);
    drive(&s, 0, T(40000), L1);
    drive(&s, 0, T(42000), L0);
    drive(&s, 1, 0, L0);
    drive(&s, 2, 0, L0);
    drive(&s, 2, T(12000), L1);
    drive(&s, 2, T(14000), L0);
    drive(&s, 2, T(20000), L1);
    drive(&s, 2, T(22000), L0);
    drive(&s, 2, T(40000), L1);
    drive(&s, 2, T(42000), L0);
    drive(&s, 3, 0, L0);
    drive(&s, 4, 0, L1);
    rig_build(&c, &s, "TESTRLY-L2", pins);
    expect(rig_run(&c, T(8000)) && operated(&c, 5, 6), "(m) TESTRLY-L2: a pulse on the set coil operates it");
    expect(rig_run(&c, T(18000)) && at_rest(&c, 5, 6), "(m) TESTRLY-L2: a pulse on the reset coil releases it");
    expect(rig_run(&c, T(26000)) && at_rest(&c, 5, 6),
           "(m) TESTRLY-L2: both coils driven at once while at rest changes nothing");
    expect(rig_run(&c, T(36000)) && operated(&c, 5, 6), "(m) TESTRLY-L2: a second set pulse operates it");
    expect(rig_run(&c, T(46000)) && operated(&c, 5, 6),
           "(m) TESTRLY-L2: both coils driven at once while operated changes nothing");
    sim_destroy(c.sim);
    /* TESTRLY-L2C: S 0, C 1, R 2, COM 3, NO 4, NC 5. */
    straight(pins, 6);
    memset(&s, 0, sizeof(s));
    s.net_count = 6;
    drive(&s, 0, 0, L0);
    drive(&s, 0, T(2000), L1);
    drive(&s, 0, T(4000), L0);
    drive(&s, 1, 0, L0);
    drive(&s, 2, 0, L0);
    drive(&s, 2, T(12000), L1);
    drive(&s, 2, T(14000), L0);
    drive(&s, 3, 0, L1);
    rig_build(&c, &s, "TESTRLY-L2C", pins);
    expect(rig_run(&c, T(8000)) && operated(&c, 4, 5), "(m) TESTRLY-L2C (common coil pin at 0): S pulsed operates it");
    expect(rig_run(&c, T(18000)) && at_rest(&c, 4, 5), "(m) TESTRLY-L2C: R pulsed releases it");
    sim_destroy(c.sim);
    /* TESTSSR2: A1 0, K1 1, A2 2, K2 3, D1A 4, D1B 5, D2A 6, D2B 7. */
    straight(pins, 8);
    {
        int ch;

        for (ch = 0; ch < 2; ch++) {
            memset(&s, 0, sizeof(s));
            s.net_count = 8;
            drive(&s, 0, 0, ch == 0 ? L1 : L0);
            drive(&s, 1, 0, L0);
            drive(&s, 2, 0, ch == 1 ? L1 : L0);
            drive(&s, 3, 0, L0);
            drive(&s, 4, 0, L1);
            drive(&s, 6, 0, L1);
            rig_build(&c, &s, "TESTSSR2", pins);
            expect(rig_run(&c, T(1100)) && value(&c, 5) == (ch == 0 ? L1 : LZ) && value(&c, 7) == (ch == 1 ? L1 : LZ) &&
                       rig_light(&c, 0) == (ch == 0) && rig_light(&c, 1) == (ch == 1),
                   ch == 0 ? "(m) TESTSSR2: A1/K1 closes only D1A-D1B and lights only the first light"
                           : "(m) TESTSSR2: A2/K2 closes only D2A-D2B and lights only the second light");
            sim_destroy(c.sim);
        }
    }
    /* (n) TESTRLY-NO: A1 0, A2 1, COM 2, NO 3. */
    straight(pins, 4);
    memset(&s, 0, sizeof(s));
    s.net_count = 4;
    drive(&s, 0, 0, L1);
    drive(&s, 0, T(3000), L0);
    drive(&s, 1, 0, L0);
    drive(&s, 2, 0, L1);
    rig_build(&c, &s, "TESTRLY-NO", pins);
    expect(rig_run(&c, T(500)) && value(&c, 3) == LZ, "(n) TESTRLY-NO before 1 ms: NO is not joined to COM");
    expect(rig_run(&c, T(2000)) && value(&c, 3) == L1, "(n) TESTRLY-NO operated: COM and NO are one net");
    expect(rig_run(&c, T(4500)) && value(&c, 3) == LZ, "(n) TESTRLY-NO released: NO is Z again");
    sim_destroy(c.sim);
    /* (q) TESTSSR: A 0, K 1, D1 2, D2 3. */
    {
        int forward;

        for (forward = 1; forward >= 0; forward--) {
            memset(&s, 0, sizeof(s));
            s.net_count = 4;
            drive(&s, 0, 0, forward ? L1 : L0);
            drive(&s, 1, 0, forward ? L0 : L1);
            drive(&s, 2, 0, L1);
            rig_build(&c, &s, "TESTSSR", pins);
            expect(rig_run(&c, T(1100)) && value(&c, 3) == (forward ? L1 : LZ),
                   forward ? "(q) TESTSSR, A = 1 and K = 0: D1 and D2 are one net after 1 ms"
                           : "(q) TESTSSR, A = 0 and K = 1: the output stays open");
            sim_destroy(c.sim);
        }
    }
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

static void circuit_cases(void) {
    /* (o) the second COM pin reaches NC through the tie. */
    {
        Circuit *c = circuit_create(library);
        size_t k = add(c, "TESTRLY-TIE"), v = add(c, "5V"), a = net(c), nc = net(c);
        int ok;

        wire(c, v, "1", a);
        wire(c, k, "6", a);
        wire(c, k, "5", nc);
        ok = circuit_start(c) == 0 && circuit_advance(c, 1000000) == 1;
        expect(ok && circuit_net_value(c, nc) == CIRCUIT_1, "(o) TESTRLY-TIE: 5V on the second COM pin reaches NC");
        circuit_destroy(c);
    }
    /* (o) only the second COM pin wired, and the relay operated: it reaches NO, not NC. */
    {
        Circuit *c = circuit_create(library);
        size_t k = add(c, "TESTRLY-TIE"), v = add(c, "5V"), g = add(c, "GND"), a = net(c), lo = net(c),
               no = net(c), nc = net(c);
        int ok;

        wire(c, v, "1", a);
        wire(c, k, "6", a);
        wire(c, g, "1", lo);
        wire(c, k, "1", a);
        wire(c, k, "2", lo);
        wire(c, k, "4", no);
        wire(c, k, "5", nc);
        ok = circuit_start(c) == 0 && circuit_advance(c, 2000000000ULL) == 1;
        expect(ok && circuit_net_value(c, no) == CIRCUIT_1 && circuit_net_value(c, nc) == CIRCUIT_Z,
               "(o) TESTRLY-TIE operated, 5V on the second COM pin only: NO reads 1, NC reads Z");
        circuit_destroy(c);
    }
    /* (p) a relay that breaks its own coil toggles every 1 ms; the run returns. */
    {
        Circuit *c = circuit_create(library);
        size_t k = add(c, "TESTRLY-SPDT"), v = add(c, "5V"), g = add(c, "GND");
        size_t vn = net(c), com = net(c), gn = net(c);
        uint64_t changes;
        int ok;

        wire(c, v, "1", vn);
        wire(c, k, "5", vn);
        wire(c, k, "3", com);
        wire(c, k, "1", com);
        wire(c, g, "1", gn);
        wire(c, k, "2", gn);
        ok = circuit_start(c) == 0 && circuit_advance(c, 10000000000ULL) == 1;
        changes = circuit_net_changes(c, com);
        if (!ok || changes + 1 < 10 || changes > 11) {
            printf("  COM changed %llu times\n", (unsigned long long)changes);
        }
        expect(ok && changes + 1 >= 10 && changes <= 11,
               "(p) a relay breaking its own coil: the run returns after 10 ms and COM changes 10 +- 1 times");
        circuit_destroy(c);
    }
    /* (p) the low-side driver: TFX-Q1 on the coil. */
    {
        Circuit *c = circuit_create(library);
        size_t k = add(c, "TESTRLY-SPDT"), q = add(c, "TFX-Q1"), v = add(c, "5V"), v2 = add(c, "5V");
        size_t g = add(c, "GND"), hi = net(c), lo = net(c), coil = net(c), com = net(c), nc = net(c), no = net(c);
        int ok;

        wire(c, v, "1", hi);
        wire(c, g, "1", lo);
        wire(c, k, "1", hi);
        wire(c, k, "2", coil);
        wire(c, q, "3", coil);
        wire(c, q, "1", lo);
        wire(c, q, "2", hi);
        wire(c, k, "3", com);
        wire(c, k, "4", no);
        wire(c, k, "5", nc);
        wire(c, v2, "1", com);
        ok = circuit_start(c) == 0 && circuit_advance(c, 990000000ULL) == 1;
        ok = ok && circuit_net_value(c, no) == CIRCUIT_Z && circuit_net_value(c, nc) == CIRCUIT_1;
        ok = ok && circuit_advance(c, 20000000ULL) == 1 && circuit_net_value(c, no) == CIRCUIT_1 &&
             circuit_net_value(c, nc) == CIRCUIT_Z;
        expect(ok, "(p) an NPN on the coil with its base at 1: at rest at 0.99 ms, operated at 1.01 ms");
        circuit_destroy(c);
    }
    /* (q) the voltage check sees the relay at rest. */
    {
        int on_nc;

        for (on_nc = 1; on_nc >= 0; on_nc--) {
            Circuit *c = circuit_create(library);
            size_t k = add(c, "TESTRLY-SPDT"), g = add(c, "SN74HC04N"), v = add(c, "12V");
            size_t a = net(c), b = net(c);
            VoltageProblem p;
            int refused_start;

            wire(c, v, "1", a);
            wire(c, k, on_nc ? "5" : "4", a);
            wire(c, k, "3", b);
            wire(c, g, "14", b);
            refused_start = circuit_start(c) != 0;
            if (on_nc) {
                expect(refused_start && circuit_voltage_problem(c, &p) == 1 && p.kind == VOLTAGE_POWER_RANGE &&
                           p.part == g,
                       "(q) 12 V on NC, COM on a 74HC04's VCC: VOLTAGE_POWER_RANGE on the 74HC04");
            } else {
                expect(!refused_start, "(q) 12 V on NO instead: the circuit starts");
            }
            circuit_destroy(c);
        }
    }
    /* (q) circuit_set_switch on a relay. */
    {
        Circuit *c = circuit_create(library);
        size_t k = add(c, "TESTRLY-SPDT");
        char want[64];

        snprintf(want, sizeof(want), "part %zu is not a switch", k);
        expect(circuit_set_switch(c, k, 1, 1) == -1 && same(circuit_error(c), want),
               "(q) circuit_set_switch(relay, 1, 1) is -1, 'part N is not a switch'");
        circuit_destroy(c);
    }
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 || load_relay_fixture(library) != 0 ||
        load_transistor_fixture(library) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    fact_cases();
    rejection_cases();
    spdt_cases();
    other_arrangement_cases();
    circuit_cases();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("relay_test: all passed\n");
    return 0;
}
