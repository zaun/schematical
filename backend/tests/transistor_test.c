/*
 * transistor_test (plan-40-F): the `transistor` kind and its switch model.
 *
 * Kind cases (section 6 a-f) load the fixture parts of
 * support/transistor_fixture.h next to the real part files: the facts read
 * back, and each malformed device or part is refused. Engine cases (g-q)
 * put one fixture part's model in an engine rig with a scripted driver (one
 * pin per net; a weak drive is a pull) and run it for a few ns: the channel
 * follows the control 1 ns later. A circuit case (o) wires an SN74HC04N, a
 * transistor, a GENERIC-RESISTOR and a GENERIC-LED. Rating cases (r-u) run
 * the voltage check of circuit_start on fixture parts across 12 V.
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
#include "sim.h"
#include "transistor_fixture.h"

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

    if (p == NULL || p->kind != PART_TRANSISTOR || p->transistor == NULL) {
        printf("  %s: not a transistor part\n", name);
        return NULL;
    }
    return p;
}

static void fact_cases(void) {
    const Part *q1 = find("TFX-Q1"), *q2 = find("TFX-Q2"), *m1 = find("TFX-M1");
    const Part *m2 = find("TFX-M2"), *dt = find("TFX-DT"), *pb = find("TFX-PB");
    const Part *arr = find("TFX-ARR"), *gen = find("TFX-GEN");
    const char *type = NULL;
    size_t control = 0, a = 0, b = 0;

    expect(q1 != NULL && strcmp(q1->transistor->type, "npn") == 0 && q1->transistor->element_count == 1 &&
               !q1->transistor->mosfet && q1->transistor->vmax_mv == 40000 && q1->transistor->imax_ma == 200 &&
               q1->transistor->power_mw == 625 && q1->transistor->hfe == 100 && q1->vcc == NULL &&
               transistor_element(q1, 0, &type, &control, &a, &b) && strcmp(type, "npn") == 0 &&
               control == 1 && a == 2 && b == 0 && !transistor_element(q1, 1, NULL, NULL, NULL, NULL),
           "(a) TFX-Q1: npn, vceo 40 V, ic 0.2 A, pd 0.625 W, hfe 100; base pin 2, collector 3, emitter 1");
    expect(q2 != NULL && strcmp(q2->transistor->type, "pnp") == 0 && q2->transistor->vmax_mv == 5000,
           "(a) TFX-Q2: pnp, vceo 5 V");
    expect(m1 != NULL && strcmp(m1->transistor->type, "nmos") == 0 && m1->transistor->mosfet &&
               m1->transistor->vmax_mv == 60000 && m1->transistor->imax_ma == 300 &&
               m1->transistor->rdson_mohm == 5000 && m1->transistor->vgsth_mv == 2100 &&
               m1->transistor->vgs_mv == 20000 && !m1->transistor->logic_level,
           "(a) TFX-M1: nmos, vds 60 V, id 0.3 A, rdson 5 ohm, vgsth 2.1 V, vgs 20 V, not logic level");
    expect(m2 != NULL && strcmp(m2->transistor->type, "pmos") == 0 && m2->transistor->logic_level &&
               m2->transistor->vmax_mv == 20000,
           "(a) TFX-M2: pmos, vds 20 V, logic level");
    expect(dt != NULL && dt->pin_count == 4 && transistor_element(dt, 0, NULL, &control, &a, &b) &&
               control == 0 && a == 1 && b == 2 && transistor_pin_limit_mv(dt, 3) == 30000,
           "(a) TFX-DT: a tab (pin 4) named D like the drain lead; both rated 30 V");
    expect(pb != NULL && pb->transistor->r1_mohm == 10000000 && pb->transistor->r2_mohm == 47000000,
           "(a) TFX-PB: r1 10 kohm, r2 47 kohm");
    expect(arr != NULL && arr->transistor->element_count == 2 && arr->transistor->mixed &&
               transistor_element(arr, 1, &type, &control, &a, &b) && strcmp(type, "pnp") == 0 &&
               control == 4 && a == 2 && b == 3,
           "(a) TFX-ARR: npn + pnp complementary pair, mixed; element 2 base pin 5, collector 3, emitter 4");
    expect(gen != NULL && gen->transistor->vmax_mv == 0 && transistor_pin_limit_mv(gen, 2) == 0,
           "(f) TFX-GEN: a generic part with no rating loads, every limit 0");
    expect(q1 != NULL && transistor_pin_limit_mv(q1, 1) == 0 && m1 != NULL &&
               transistor_pin_limit_mv(m1, 0) == 20000 && transistor_pin_limit_mv(m1, 1) == 60000,
           "pin limits: a BJT base none, a MOSFET gate vgs, a drain vds");
}

/* A one-device file that must be refused; `want` is part of the error. */
static void refused(const char *what, const char *json, const char *want) {
    PartLibrary *lib = part_library_create();
    char *error = NULL;
    int ok = lib != NULL && load_parts(lib, json, "bad-transistor.json", &error) != 0 &&
             error != NULL && strstr(error, want) != NULL;

    if (!ok) {
        printf("  error: %s\n", error != NULL ? error : "(none)");
    }
    expect(ok, what);
    free(error);
    part_library_destroy(lib);
}

#define NPN_EBC TFX_BJT("npn", "B", "C", "E")
#define BAD_BJT(KEYS) TRANSISTOR_FILE(TRANSISTOR_DEVICE("bad", TFX_EBC, NPN_EBC, "BAD", "PINHDR-1X03-V", KEYS))
#define BAD_FET(KEYS)                                                                              \
    TRANSISTOR_FILE(TRANSISTOR_DEVICE("bad", TFX_GDS, TFX_FET("nmos", "G", "D", "S"), "BAD",       \
                                      "PINHDR-1X03-V", KEYS))

static void rejection_cases(void) {
    refused("(b) an unknown type: refused",
            TRANSISTOR_FILE(TRANSISTOR_DEVICE("bad", TFX_EBC, TFX_BJT("jfet", "B", "C", "E"), "BAD",
                                              "PINHDR-1X03-V", "")),
            "unknown transistor type");
    refused("(c) a \"gate\" on a BJT: refused",
            TRANSISTOR_FILE(TRANSISTOR_DEVICE("bad", TFX_EBC,
                                              "{\"type\":\"npn\",\"gate\":\"B\",\"collector\":\"C\",\"emitter\":\"E\"}",
                                              "BAD", "PINHDR-1X03-V", "")),
            "\"gate\" is not a bjt key");
    refused("(c) a \"base\" on a MOSFET: refused",
            TRANSISTOR_FILE(TRANSISTOR_DEVICE("bad", TFX_GDS,
                                              "{\"type\":\"nmos\",\"base\":\"G\",\"drain\":\"D\",\"source\":\"S\"}",
                                              "BAD", "PINHDR-1X03-V", "")),
            "\"base\" is not a mosfet key");
    refused("(d) a base on two pins: refused",
            TRANSISTOR_FILE(TRANSISTOR_DEVICE("bad", TFX_EBC "," TFX_IN(4, "B"), NPN_EBC, "BAD",
                                              "PINHDR-1X04-V", "")),
            "must be on one pin");
    refused("(d) a base that is not an input: refused",
            TRANSISTOR_FILE(TRANSISTOR_DEVICE("bad", TFX_IO(1, "E") "," TFX_IO(2, "B") "," TFX_IO(3, "C"),
                                              NPN_EBC, "BAD", "PINHDR-1X03-V", "")),
            "must be input");
    refused("(d) a collector that is not inout: refused",
            TRANSISTOR_FILE(TRANSISTOR_DEVICE("bad", TFX_IO(1, "E") "," TFX_IN(2, "B") "," TFX_PIN(3, "C", "output"),
                                              NPN_EBC, "BAD", "PINHDR-1X03-V", "")),
            "must be inout");
    refused("(e) a pin in no element: refused",
            TRANSISTOR_FILE(TRANSISTOR_DEVICE("bad", TFX_EBC "," TFX_IO(4, "X"), NPN_EBC, "BAD",
                                              "PINHDR-1X04-V", "")),
            "is in no element");
    refused("(f) \"vds\" on a BJT: refused", BAD_BJT(",\"vds\":30"), "\"vds\" is only for a mosfet");
    refused("(f) \"vceo\" on a MOSFET: refused", BAD_FET(",\"vceo\":30"), "\"vceo\" is only for a bjt");
    refused("(f) \"vceo\" 0: refused", BAD_BJT(",\"vceo\":0"), "\"vceo\" must be above 0");
    refused("(f) \"ic\" 2000: refused", BAD_BJT(",\"ic\":2000"), "\"ic\" must be above 0");
    refused("(f) \"darlington\" on a MOSFET: refused",
            TRANSISTOR_FILE(TRANSISTOR_DEVICE("bad", TFX_GDS,
                                              "{\"type\":\"nmos\",\"gate\":\"G\",\"drain\":\"D\",\"source\":\"S\","
                                              "\"darlington\":true}",
                                              "BAD", "PINHDR-1X03-V", "")),
            "\"darlington\" is not a mosfet key");
    refused("no \"elements\": refused",
            TRANSISTOR_FILE("{\"id\":\"bad\",\"function\":\"t\",\"description\":\"f\",\"drawer\":[],"
                            "\"pinouts\":{\"p\":[" TFX_EBC "]},"
                            "\"parts\":[{\"name\":\"BAD\",\"package\":\"TO-92\",\"pinout\":\"p\","
                            "\"footprint\":\"PINHDR-1X03-V\"}]}"),
            "elements");
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
    q = sim_add_instance(c->sim, kind_model(part), "Q1", part);
    for (i = 0; i < part->pin_count; i++) {
        if (pin_net[i] != NO_NET) {
            sim_connect(c->sim, c->nets[pin_net[i]], q, i);
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

#define S DRIVE_STRONG
#define W DRIVE_WEAK
#define L0 LOGIC_0
#define L1 LOGIC_1

static void engine_cases(void) {
    /* (g) NPN low side: nets 0 E (GND), 1 B, 2 C (pulled up). */
    {
        Script s = { 3, { {0, 0, L0, S}, {2, 0, L1, W}, {1, 0, L1, S}, {1, 3000, L0, S} }, 4 };
        size_t pins[] = { 0, 1, 2 };
        Rig c;
        int ok;

        rig_build(&c, &s, "TFX-Q1", pins);
        ok = rig_run(&c, 500) && value(&c, 2) == L1;
        ok = ok && rig_run(&c, 1500) && value(&c, 2) == L0;
        ok = ok && rig_run(&c, 3500) && value(&c, 2) == L0;
        ok = ok && rig_run(&c, 4500) && value(&c, 2) == L1;
        expect(ok, "(g) NPN, base 1, collector pulled up, emitter GND: collector 0 after 1 ns; base 0: 1 again");
        sim_destroy(c.sim);
    }
    /* (h) N-channel low side: pins G D S on nets 1 2 0. */
    {
        Script s = { 3, { {0, 0, L0, S}, {2, 0, L1, W}, {1, 0, L1, S}, {1, 3000, L0, S} }, 4 };
        size_t pins[] = { 1, 2, 0 };
        Rig c;
        int ok;

        rig_build(&c, &s, "TFX-M1", pins);
        ok = rig_run(&c, 500) && value(&c, 2) == L1;
        ok = ok && rig_run(&c, 1500) && value(&c, 2) == L0;
        ok = ok && rig_run(&c, 4500) && value(&c, 2) == L1;
        expect(ok, "(h) N-channel, gate 1, drain pulled up, source GND: drain 0; gate 0: drain 1");
        sim_destroy(c.sim);
    }
    /* (i) PNP and P-channel high side: emitter / source on VCC, collector / drain alone. */
    {
        static const char *const names[] = { "TFX-Q2", "TFX-M2" };
        int k;

        for (k = 0; k < 2; k++) {
            Script s = { 3, { {0, 0, L1, S}, {1, 0, L0, S}, {1, 3000, L1, S} }, 3 };
            size_t bjt[] = { 0, 1, 2 };   /* E B C */
            size_t fet[] = { 1, 2, 0 };   /* G D S */
            Rig c;
            int ok;

            rig_build(&c, &s, names[k], k == 0 ? bjt : fet);
            ok = rig_run(&c, 1500) && value(&c, 2) == L1;
            ok = ok && rig_run(&c, 4500) && value(&c, 2) == LOGIC_Z;
            expect(ok, k == 0 ? "(i) PNP high side, base 0: collector 1; base 1: collector Z"
                              : "(i) P-channel high side, gate 0: drain 1; gate 1: drain Z");
            sim_destroy(c.sim);
        }
    }
    /* (j) base X, and base not connected: the channel stays open. */
    {
        Script s = { 3, { {0, 0, L0, S}, {2, 0, L1, W}, {1, 0, LOGIC_X, S} }, 3 };
        size_t pins[] = { 0, 1, 2 };
        size_t open[] = { 0, NO_NET, 2 };
        Rig c;
        int ok;

        rig_build(&c, &s, "TFX-Q1", pins);
        ok = rig_run(&c, 3000) && value(&c, 2) == L1;
        sim_destroy(c.sim);
        rig_build(&c, &s, "TFX-Q1", open);
        ok = ok && rig_run(&c, 3000) && value(&c, 2) == L1;
        sim_destroy(c.sim);
        expect(ok, "(j) base X, or base not connected: channel open, collector stays pulled up");
    }
    /* (k) a 0.5 ns pulse on the base never moves the channel. */
    {
        Script s = { 3, { {0, 0, L0, S}, {2, 0, L1, W}, {1, 0, L0, S}, {1, 2000, L1, S}, {1, 2500, L0, S} }, 5 };
        size_t pins[] = { 0, 1, 2 };
        Rig c;
        int ok;
        uint64_t before;

        rig_build(&c, &s, "TFX-Q1", pins);
        ok = rig_run(&c, 1900);
        before = sim_net_changes(c.nets[2]);
        ok = ok && rig_run(&c, 3200) && value(&c, 2) == L1;
        ok = ok && rig_run(&c, 5000) && value(&c, 2) == L1 && sim_net_changes(c.nets[2]) == before;
        expect(ok, "(k) a 0.5 ns pulse on the base: the collector never moves");
        sim_destroy(c.sim);
    }
    /* (l) strong 1 on the collector, strong 0 on the emitter. */
    {
        int k;

        for (k = 0; k < 2; k++) {
            Script s = { 3, { {0, 0, L0, S}, {2, 0, L1, S}, {1, 0, k == 0 ? L1 : L0, S} }, 3 };
            size_t pins[] = { 0, 1, 2 };
            Rig c;
            int ok;

            rig_build(&c, &s, "TFX-Q1", pins);
            sim_start(c.sim);
            c.started = 1;
            ok = sim_run_until(c.sim, 3000);
            if (k == 0) {
                expect(!ok && sim_error(c.sim) != NULL && strncmp(sim_error(c.sim), "smoke:", 6) == 0,
                       "(l) base 1 across a strong 1 and a strong 0: smoke");
            } else {
                expect(ok && value(&c, 2) == L1 && value(&c, 0) == L0,
                       "(l) base 0 across a strong 1 and a strong 0: no short");
            }
            sim_destroy(c.sim);
        }
    }
    /* (m) emitter follower: collector VCC, base 1, emitter pulled down. */
    {
        Script s = { 3, { {2, 0, L1, S}, {0, 0, L0, W}, {1, 0, L1, S} }, 3 };
        size_t pins[] = { 0, 1, 2 };
        Rig c;
        int ok;

        rig_build(&c, &s, "TFX-Q1", pins);
        ok = rig_run(&c, 500) && value(&c, 0) == L0;
        ok = ok && rig_run(&c, 3000) && value(&c, 0) == L1;
        expect(ok, "(m) emitter follower: collector VCC, base 1, emitter pulled down reads 1");
        sim_destroy(c.sim);
    }
    /* (n) a diode-connected NMOS: gate and drain on net 1 (pulled up), source GND. */
    {
        Script s = { 2, { {0, 0, L0, S}, {1, 0, L1, W} }, 2 };
        size_t pins[] = { 1, 1, 0 };
        Rig c;
        int ok;
        uint64_t changes;

        rig_build(&c, &s, "TFX-M1", pins);
        ok = rig_run(&c, 10500);
        changes = sim_net_changes(c.nets[1]);
        expect(ok && changes >= 8 && changes <= 12,
               "(n) a diode-connected NMOS: the drain toggles each ns and the run returns");
        if (!ok || changes < 8 || changes > 12) {
            printf("  changes %llu\n", (unsigned long long)changes);
        }
        sim_destroy(c.sim);
    }
    /* (p) a tab-tied part: the tab (pin 4) reaches the drain lead (pin 2). */
    {
        Script s = { 3, { {0, 0, L0, S}, {2, 0, L1, S} }, 2 };
        size_t pins[] = { 0, 1, NO_NET, 2 };   /* G, D lead, S unwired, D tab */
        Rig c;
        int ok;

        rig_build(&c, &s, "TFX-DT", pins);
        ok = rig_run(&c, 2000) && value(&c, 1) == L1;
        expect(ok, "(p) TFX-DT: a 1 driven on the tab reaches the drain lead");
        sim_destroy(c.sim);
    }
    /* (p) the channel from the tab alone: the drain lead unwired. */
    {
        Script s = { 3, { {0, 0, L1, S}, {1, 0, L0, S}, {2, 0, L1, W} }, 3 };
        size_t pins[] = { 0, NO_NET, 1, 2 };   /* G, D lead unwired, S, D tab */
        Rig c;
        int ok;

        rig_build(&c, &s, "TFX-DT", pins);
        ok = rig_run(&c, 2000) && value(&c, 2) == L0;
        expect(ok, "(p) TFX-DT, gate 1, only the tab wired: the tab is pulled to the source's 0");
        sim_destroy(c.sim);
    }
    /* (q) a complementary pair: each element switches on its own base.
       nets: 0 GND, 1 VCC, 2 B1, 3 C1 (pulled up), 4 B2, 5 C2 (pulled down). */
    {
        Script s = { 6, { {0, 0, L0, S}, {1, 0, L1, S}, {3, 0, L1, W}, {5, 0, L0, W},
                          {2, 0, L1, S}, {4, 0, L1, S}, {4, 3000, L0, S}, {2, 3000, L0, S} }, 8 };
        size_t pins[] = { 0, 2, 5, 1, 4, 3 };   /* E1 B1 C2 E2 B2 C1 */
        Rig c;
        int ok;

        rig_build(&c, &s, "TFX-ARR", pins);
        ok = rig_run(&c, 2000) && value(&c, 3) == L0 && value(&c, 5) == L0;
        ok = ok && rig_run(&c, 5000) && value(&c, 3) == L1 && value(&c, 5) == L1;
        expect(ok, "(q) TFX-ARR: B1 1, B2 1 -> C1 0, C2 0; B1 0, B2 0 -> C1 1, C2 1");
        sim_destroy(c.sim);
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

static int lit(Circuit *c, size_t part) {
    unsigned char on = 0;

    return circuit_part_lights(c, part, &on, 1) == 1 ? on : -1;
}

static void circuit_cases(void) {
    /* (o) SN74HC04N 1Y drives the base of TFX-Q1; 5V - R - LED - collector,
       emitter GND. 1A on GND (1Y = 1): lit; 1A on 5V (1Y = 0): dark. */
    int k;

    for (k = 0; k < 2; k++) {
        Circuit *c = circuit_create(library);
        size_t g = add(c, "SN74HC04N"), vcc = add(c, "5V"), gnd = add(c, "GND");
        size_t q = add(c, "TFX-Q1"), r = add(c, "GENERIC-RESISTOR"), led = add(c, "GENERIC-LED");
        size_t hi = net(c), lo = net(c), base = net(c), anode = net(c), col = net(c);

        wire(c, vcc, "1", hi);
        wire(c, gnd, "1", lo);
        wire(c, g, "14", hi);
        wire(c, g, "7", lo);
        wire(c, g, "1", k == 0 ? lo : hi);
        wire(c, g, "2", base);
        wire(c, q, "2", base);
        wire(c, q, "1", lo);
        wire(c, q, "3", col);
        wire(c, r, "1", hi);
        wire(c, r, "2", anode);
        wire(c, led, "2", anode);
        wire(c, led, "1", col);
        expect(circuit_start(c) == 0 && circuit_advance(c, 1000000) != 0 && lit(c, led) == (k == 0),
               k == 0 ? "(o) an inverter's 1 on an NPN's base; 5V - R - LED - collector: lit"
                      : "(o) an inverter's 0 on an NPN's base: the LED is dark");
        circuit_destroy(c);
    }
}

/* ---- rating cases -------------------------------------------------------- */

/* The circuit: part `name`, its pin `pin` on 12 V, GND on its pin `ground`
   (NULL = none). Returns 1 when refused, writing the problem and text. */
static int on_12v(const char *name, const char *pin, const char *ground, VoltageProblem *p,
                  char *text, size_t size) {
    Circuit *c = circuit_create(library);
    size_t q = add(c, name), v12 = add(c, "12V"), gnd = add(c, "GND");
    size_t hi = net(c), lo = net(c);
    int refused;

    wire(c, v12, "1", hi);
    wire(c, gnd, "1", lo);
    wire(c, q, pin, hi);
    if (ground != NULL) {
        wire(c, q, ground, lo);
    }
    refused = circuit_start(c) != 0;
    text[0] = '\0';
    if (refused && circuit_voltage_problem(c, p) == 1 && circuit_error(c) != NULL) {
        snprintf(text, size, "%s", circuit_error(c));
    }
    circuit_destroy(c);
    return refused;
}

static void rating_cases(void) {
    VoltageProblem p;
    char text[256];

    expect(on_12v("TFX-M5", "3", "2", &p, text, sizeof(text)) && p.kind == VOLTAGE_TRANSISTOR_OVER &&
               p.mv == 12000 && p.max_mv == 5000 &&
               strcmp(text, "voltage: 12 V on pin 3 (D) of TFX-M5; it is rated 5 V") == 0,
           "(r) a MOSFET with vds 5 and 12 V on its drain: refused, 'voltage: 12 V on pin 3 (D) of TFX-M5; it is rated 5 V'");
    if (strcmp(text, "voltage: 12 V on pin 3 (D) of TFX-M5; it is rated 5 V") != 0) {
        printf("  text: %s\n", text);
    }
    expect(!on_12v("TFX-Q2", "2", "1", &p, text, sizeof(text)),
           "(s) a BJT base on 12 V (vceo 5): not refused");
    expect(on_12v("TFX-M5", "1", "2", &p, text, sizeof(text)) && p.kind == VOLTAGE_TRANSISTOR_OVER &&
               p.max_mv == 8000 && p.pin == 0,
           "(t) a gate on 12 V above its vgs 8: refused");
    expect(!on_12v("TFX-M1", "2", "3", &p, text, sizeof(text)),
           "(u) 12 V on the drain of a 60 V MOSFET: accepted");
    expect(!on_12v("TFX-GEN", "3", "1", &p, text, sizeof(text)),
           "(f) a generic part with no rating on 12 V: never refused");
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 ||
        load_transistor_fixture(library) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    fact_cases();
    rejection_cases();
    engine_cases();
    circuit_cases();
    rating_cases();
    part_library_destroy(library);
    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
