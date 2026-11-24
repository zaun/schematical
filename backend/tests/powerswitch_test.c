/*
 * powerswitch_test (plan-55-A): the `powerswitch` kind, its output model and
 * its voltage rules.
 *
 * Kind cases (a, b) load the fixture parts of support/powerswitch_fixture.h
 * next to the real part files: the facts read back, and each malformed
 * device or part is refused. Engine cases (c-m) put one fixture part's model
 * in an engine rig with a scripted driver (one pin per net; a weak drive is
 * a pull) and run it for a few ns: the output follows the input and the
 * enable 1 ns later. Voltage cases (n-v) build circuits with real supplies
 * and run the voltage check of circuit_start: a switched level is observed
 * through the clash it causes with a 1.2 V supply on the same net (the
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
#include "powerswitch_fixture.h"
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

    if (p == NULL || p->kind != PART_POWERSWITCH || p->powerswitch == NULL) {
        printf("  %s: not a power switch part\n", name);
        return NULL;
    }
    return p;
}

static void fact_cases(void) {
    const Part *load = find("PFX-LOAD"), *ilim = find("PFX-ILIM"), *low = find("PFX-LOW");
    const Part *diode = find("PFX-DIODE"), *dual = find("PFX-DUAL"), *quad = find("PFX-QUAD");
    const Part *free_ = find("PFX-FREE");
    size_t in = 0, out = 0, enable = 0, flag = 0;
    int enable_low = 0, ref_input = 0, flag_disabled = 0;

    expect(load != NULL && strcmp(load->powerswitch->type, "load") == 0 && load->powerswitch->switch_count == 1 &&
               load->powerswitch->vin_min_mv == 1000 && load->powerswitch->vin_max_mv == 5500 &&
               load->powerswitch->ilimit_ma == 0 && load->vcc == NULL &&
               powerswitch_element(load, 0, &in, &out, &enable, &enable_low, &ref_input, &flag, &flag_disabled) &&
               in == 0 && out == 3 && enable == 2 && !enable_low && !ref_input && flag == SIZE_MAX &&
               !powerswitch_element(load, 1, NULL, NULL, NULL, NULL, NULL, NULL, NULL),
           "(a) PFX-LOAD: load, one switch, vinMin 1, vinMax 5.5; VIN pin 1, VOUT pin 4, ON pin 3 active high");
    expect(ilim != NULL && strcmp(ilim->powerswitch->type, "limited") == 0 && ilim->powerswitch->ilimit_ma == 500 &&
               ilim->powerswitch->vin_max_mv == 6000 && powerswitch_input_max_mv(ilim) == 6000 &&
               powerswitch_element(ilim, 0, NULL, NULL, NULL, NULL, NULL, &flag, &flag_disabled) && flag == 4 &&
               !flag_disabled,
           "(a) PFX-ILIM: limited, ilimit 0.5 A, vinMax 6; FLG pin 5 is a fault flag");
    expect(low != NULL && powerswitch_element(low, 0, NULL, NULL, &enable, &enable_low, NULL, NULL, NULL) &&
               enable == 2 && enable_low,
           "(a) PFX-LOW: EN pin 3 active low");
    expect(diode != NULL && strcmp(diode->powerswitch->type, "ideal-diode") == 0 &&
               diode->powerswitch->vin_min_mv == 1500 &&
               powerswitch_element(diode, 0, &in, &out, &enable, &enable_low, &ref_input, &flag, &flag_disabled) &&
               in == 0 && out == 4 && enable == 2 && enable_low && ref_input && flag == 3 && flag_disabled,
           "(a) PFX-DIODE: ideal diode, vinMin 1.5; CE pin 3 active low against the input; ST pin 4 low while disabled");
    expect(dual != NULL && dual->powerswitch->switch_count == 2 &&
               powerswitch_element(dual, 1, &in, &out, &enable, &enable_low, NULL, NULL, NULL) && in == 1 &&
               out == 5 && enable == 3 && enable_low && powerswitch_pin_in(dual, 1, 1) &&
               !powerswitch_pin_in(dual, 1, 0) && powerswitch_pin_out(dual, 0, 5) && powerswitch_pin_out(dual, 1, 5) &&
               !powerswitch_pin_out(dual, 0, 4),
           "(a) PFX-DUAL: two switches into one VOUT (pin 6); switch 2: VIN2 pin 2, ON pin 4 active low");
    expect(quad != NULL && quad->powerswitch->switch_count == 4 && powerswitch_pin_in(quad, 0, 0) &&
               powerswitch_pin_in(quad, 1, 0) && !powerswitch_pin_in(quad, 2, 0) && powerswitch_pin_in(quad, 3, 1) &&
               powerswitch_pin_out(quad, 3, 6) &&
               powerswitch_element(quad, 3, &in, &out, &enable, NULL, NULL, &flag, NULL) && in == 1 && out == 6 &&
               enable == 10 && flag == 14,
           "(a) PFX-QUAD: four switches; IN1 serves OUT1 and OUT2, IN2 OUT3 and OUT4; switch 4: EN4 pin 11, OC4 pin 15");
    expect(free_ != NULL && free_->powerswitch->vin_max_mv == 0 && powerswitch_input_max_mv(free_) == 0 &&
               powerswitch_element(free_, 0, NULL, NULL, &enable, NULL, NULL, NULL, NULL) && enable == SIZE_MAX,
           "(a) PFX-FREE: no enable, no vinMax (the maximum is 0, never refused)");
}

/* A one-device file that must be refused; `want` is part of the error. */
static void refused(const char *what, const char *json, const char *want) {
    PartLibrary *lib = part_library_create();
    char *error = NULL;
    int ok = lib != NULL && load_parts(lib, json, "bad-powerswitch.json", &error) != 0 &&
             error != NULL && strstr(error, want) != NULL;

    if (!ok) {
        printf("  error: %s\n", error != NULL ? error : "(none)");
    }
    expect(ok, what);
    free(error);
    part_library_destroy(lib);
}

#define PINS3 PFX_PWR(1, "IN") "," PFX_GND(2, "GND") "," PFX_OUT(3, "OUT")
#define PINS4 PFX_PWR(1, "IN") "," PFX_GND(2, "GND") "," PFX_OUT(3, "OUT") "," PFX_IN(4, "EN")
#define SW1 PFX_SW("IN", "OUT", "")
#define BAD(TYPE, PINS, SWITCHES, ANALOG, KEYS) \
    POWERSWITCH_FILE(POWERSWITCH_DEVICE("bad", TYPE, PINS, SWITCHES, ANALOG, "BAD", "PINHDR-1X04-V", KEYS))

static void rejection_cases(void) {
    refused("(b) no switches: refused", BAD("load", PINS3, "", "", ""), "\"switches\" must have 1 to 4 entries");
    refused("(b) five switches: refused",
            BAD("load",
                PFX_PWR(1, "A") "," PFX_OUT(2, "B") "," PFX_PWR(3, "C") "," PFX_OUT(4, "D") "," PFX_PWR(5, "E") ","
                PFX_OUT(6, "F") "," PFX_PWR(7, "G") "," PFX_OUT(8, "H") "," PFX_PWR(9, "I") "," PFX_OUT(10, "J"),
                PFX_SW("A", "B", "") "," PFX_SW("C", "D", "") "," PFX_SW("E", "F", "") "," PFX_SW("G", "H", "") ","
                PFX_SW("I", "J", ""),
                "", ""),
            "\"switches\" must have 1 to 4 entries");
    refused("(b) an unknown type: refused", BAD("mux", PINS3, SW1, "", ""), "unknown power switch type");
    refused("(b) \"ilimit\" on a load switch: refused", BAD("load", PINS3, SW1, "", ",\"ilimit\":1"),
            "\"ilimit\" is only for a current-limited switch");
    refused("(b) \"enableActive\" without \"enable\": refused",
            BAD("load", PINS3, PFX_SW("IN", "OUT", ",\"enableActive\":\"low\""), "", ""),
            "\"enableActive\" needs \"enable\"");
    refused("(b) \"enableRef\" without \"enable\": refused",
            BAD("load", PINS3, PFX_SW("IN", "OUT", ",\"enableRef\":\"logic\""), "", ""),
            "\"enableRef\" needs \"enable\"");
    refused("(b) \"flagWhen\" without \"flag\": refused",
            BAD("load", PINS3, PFX_SW("IN", "OUT", ",\"flagWhen\":\"fault\""), "", ""),
            "\"flagWhen\" needs \"flag\"");
    refused("(b) \"enableRef\": \"input\" with an active-high enable: refused",
            BAD("load", PINS4, PFX_SW("IN", "OUT", ",\"enable\":\"EN\",\"enableRef\":\"input\""), "", ""),
            "\"enableRef\": \"input\" needs \"enableActive\": \"low\"");
    refused("(b) an unknown \"enableActive\": refused",
            BAD("load", PINS4, PFX_SW("IN", "OUT", ",\"enable\":\"EN\",\"enableActive\":\"up\""), "", ""),
            "\"enableActive\" must be \"high\" or \"low\"");
    refused("(b) an \"in\" pin that is not power: refused",
            BAD("load", PFX_IN(1, "IN") "," PFX_GND(2, "GND") "," PFX_OUT(3, "OUT"), SW1, "", ""),
            "power switch pin \"IN\" (in) must be power");
    refused("(b) an \"out\" pin that is not an output: refused",
            BAD("load", PFX_PWR(1, "IN") "," PFX_GND(2, "GND") "," PFX_IN(3, "OUT"), SW1, "", ""),
            "power switch pin \"OUT\" (out) must be output");
    refused("(b) a pin in no switch and not analog: refused",
            BAD("load", PINS3 "," PFX_IN(4, "X"), SW1, "", ""), "pin \"X\" is in no switch and not analog");
    refused("(b) \"vinMax\" below \"vinMin\": refused", BAD("load", PINS3, SW1, "", ",\"vinMin\":5,\"vinMax\":3"),
            "\"vinMax\" must be at least \"vinMin\"");
    refused("(b) \"vinMax\" above 100: refused", BAD("load", PINS3, SW1, "", ",\"vinMax\":101"),
            "\"vinMax\" must be above 0 and at most 100");
    refused("(b) a name that names no pin: refused", BAD("load", PINS3, PFX_SW("IN", "NOPE", ""), "", ""), "NOPE");
    refused("(b) an analog name that is no pin: refused", BAD("load", PINS3, SW1, "\"NOPE\"", ""), "NOPE");
}

/* ---- engine cases -------------------------------------------------------- */

#define MAX_NETS 16
#define MAX_DRIVES 24
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

/* One rig run to the end of a script: `net` holds the value at t (ps). */
static int reads(const Script *script, const char *part, const size_t *pins, SimTime t, size_t net,
                 LogicValue want) {
    Rig c;
    int ok;

    rig_build(&c, script, part, pins);
    ok = rig_run(&c, t, 0) && value(&c, net) == want;
    sim_destroy(c.sim);
    return ok;
}

static void engine_cases(void) {
    /* PFX-LOAD: pins 1 VIN, 2 GND, 3 ON, 4 VOUT on nets 0 VIN, 1 (GND), 2 ON, 3 VOUT. */
    size_t load[] = { 0, 1, 2, 3 };

    /* (c) VIN 1, ON 1: VOUT reads 1 after 1 ns; ON 0: Z; ON Z: Z. */
    {
        Script s = { 4, { {1, 0, L0, S}, {0, 0, L1, S}, {2, 0, L1, S}, {2, 3000, L0, S} }, 4 };
        Rig c;
        int ok;

        rig_build(&c, &s, "PFX-LOAD", load);
        ok = rig_run(&c, 500, 0) && value(&c, 3) == LZ;
        ok = ok && rig_run(&c, 1500, 0) && value(&c, 3) == L1;
        ok = ok && rig_run(&c, 3500, 0) && value(&c, 3) == L1;
        ok = ok && rig_run(&c, 4500, 0) && value(&c, 3) == LZ;
        expect(ok, "(c) PFX-LOAD: VIN 1, ON 1: VOUT 1 after 1 ns; ON 0 (1 ns later): VOUT Z");
        sim_destroy(c.sim);
    }
    {
        Script s = { 4, { {1, 0, L0, S}, {0, 0, L1, S} }, 2 };

        expect(reads(&s, "PFX-LOAD", load, 3000, 3, LZ), "(c) PFX-LOAD: VIN 1, ON floating: VOUT Z (a floating enable is inactive)");
    }
    /* (d) VIN 0, then Z: VOUT Z. */
    {
        Script s = { 4, { {1, 0, L0, S}, {0, 0, L0, S}, {2, 0, L1, S} }, 3 };
        Script z = { 4, { {1, 0, L0, S}, {2, 0, L1, S} }, 2 };

        expect(reads(&s, "PFX-LOAD", load, 3000, 3, LZ) && reads(&z, "PFX-LOAD", load, 3000, 3, LZ),
               "(d) PFX-LOAD: VIN 0, VIN floating, ON 1: VOUT Z");
    }
    /* (e) VIN X: VOUT X; VIN 1, ON X: VOUT X. */
    {
        Script a = { 4, { {1, 0, L0, S}, {0, 0, LX, S}, {2, 0, L1, S} }, 3 };
        Script b = { 4, { {1, 0, L0, S}, {0, 0, L1, S}, {2, 0, LX, S} }, 3 };

        expect(reads(&a, "PFX-LOAD", load, 3000, 3, LX) && reads(&b, "PFX-LOAD", load, 3000, 3, LX),
               "(e) PFX-LOAD: VIN X: VOUT X; VIN 1, ON X: VOUT X");
    }
    /* (f) A 0.5 ns pulse on ON never moves VOUT. */
    {
        Script s = { 4, { {1, 0, L0, S}, {0, 0, L1, S}, {2, 0, L0, S}, {2, 2000, L1, S}, {2, 2500, L0, S} }, 5 };

        expect(reads(&s, "PFX-LOAD", load, 5000, 3, LZ), "(f) PFX-LOAD: a 0.5 ns pulse on ON never moves VOUT");
    }
    /* (g) PFX-ILIM with a pull-up on FLG: FLG reads 1 at every input and enable value. Pins 1 IN, 2 GND, 3 EN,
       4 OUT, 5 FLG on nets 0, 1, 2, 3, 4. */
    {
        size_t ilim[] = { 0, 1, 2, 3, 4 };
        LogicValue ins[] = { L0, L1, LX };
        LogicValue ens[] = { L0, L1, LX };
        int ok = 1;
        size_t i, e;

        for (i = 0; i < 3; i++) {
            for (e = 0; e < 3; e++) {
                Script s = { 5, { {1, 0, L0, S}, {0, 0, ins[i], S}, {2, 0, ens[e], S}, {4, 0, L1, W} }, 4 };

                ok = ok && reads(&s, "PFX-ILIM", ilim, 3000, 4, L1);
            }
        }
        expect(ok, "(g) PFX-ILIM: FLG (a fault flag, open drain) reads 1 at every input and enable value");
    }
    /* (h) PFX-LOW: EN 0 -> OUT 1, EN 1 -> OUT Z. */
    {
        size_t low[] = { 0, 1, 2, 3, 4 };
        Script on = { 5, { {1, 0, L0, S}, {0, 0, L1, S}, {2, 0, L0, S} }, 3 };
        Script off = { 5, { {1, 0, L0, S}, {0, 0, L1, S}, {2, 0, L1, S} }, 3 };

        expect(reads(&on, "PFX-LOW", low, 3000, 3, L1) && reads(&off, "PFX-LOW", low, 3000, 3, LZ),
               "(h) PFX-LOW: EN 0: OUT 1; EN 1: OUT Z");
    }
    /* (i) PFX-DIODE: pins 1 VIN, 2 GND, 3 CE, 4 ST, 5 VOUT; CE on VOUT (net 2 is VOUT's net), ST (net 3) pulled up. */
    {
        size_t diode[] = { 0, 1, 2, 3, 2 };
        Script on = { 4, { {1, 0, L0, S}, {0, 0, L1, S}, {3, 0, L1, W} }, 3 };
        Script held = { 4, { {1, 0, L0, S}, {0, 0, L0, S}, {2, 0, L1, S}, {3, 0, L1, W} }, 4 };
        Script none = { 4, { {1, 0, L0, S}, {0, 0, L0, S}, {2, 0, L0, S}, {3, 0, L1, W} }, 4 };
        Rig c;
        int ok;

        rig_build(&c, &on, "PFX-DIODE", diode);
        ok = rig_run(&c, 3000, 0) && value(&c, 2) == L1 && value(&c, 3) == L1;
        ok = ok && rig_run(&c, 9000, 0) && value(&c, 2) == L1;   /* stays 1: no oscillation */
        sim_destroy(c.sim);
        rig_build(&c, &held, "PFX-DIODE", diode);
        ok = ok && rig_run(&c, 3000, 0) && value(&c, 2) == L1 && value(&c, 3) == L0;
        sim_destroy(c.sim);
        rig_build(&c, &none, "PFX-DIODE", diode);
        ok = ok && rig_run(&c, 3000, 0) && value(&c, 2) == L0 && value(&c, 3) == L1;
        sim_destroy(c.sim);
        expect(ok,
               "(i) PFX-DIODE, CE on VOUT: VIN 1: VOUT 1 and stays; VIN 0, VOUT held 1: ST 0, VOUT not driven; "
               "VIN 0, VOUT 0: ST Z");
    }
    /* (j) PFX-DUAL: pins 1 VIN1, 2 VIN2, 3 GND, 4 ON, 5 ST, 6 VOUT on nets 0 VIN1, 1 VIN2, 2 GND, 3 ON, 4 ST,
       5 VOUT. */
    {
        size_t dual[] = { 0, 1, 2, 3, 4, 5 };
        Script a = { 6, { {2, 0, L0, S}, {0, 0, L1, S}, {1, 0, L0, S}, {3, 0, L0, S} }, 4 };
        Script b = { 6, { {2, 0, L0, S}, {0, 0, L1, S}, {1, 0, L1, S}, {3, 0, L0, S} }, 4 };
        Script n = { 6, { {2, 0, L0, S}, {0, 0, L0, S}, {1, 0, L0, S}, {3, 0, L0, S} }, 4 };
        Script off = { 6, { {2, 0, L0, S}, {0, 0, L1, S}, {1, 0, L1, S}, {3, 0, L1, S} }, 4 };
        Script d2 = { 6, { {2, 0, L0, S}, {0, 0, L0, S}, {1, 0, L1, S}, {3, 0, L0, S} }, 4 };

        expect(reads(&a, "PFX-DUAL", dual, 3000, 5, L1) && reads(&b, "PFX-DUAL", dual, 3000, 5, L1) &&
                   reads(&d2, "PFX-DUAL", dual, 3000, 5, L1) && reads(&n, "PFX-DUAL", dual, 3000, 5, LZ) &&
                   reads(&off, "PFX-DUAL", dual, 3000, 5, LZ),
               "(j) PFX-DUAL: VIN1 1, VIN2 0: VOUT 1; both 1: 1; VIN2 only: 1; both 0: Z; ON 1: Z");
    }
    /* (k) PFX-QUAD: nets 0 IN1, 1 IN2, 2 GND, 3-6 OUT1-4, 7-10 EN1-4, 11-14 OC1-4 (pins in order). */
    {
        size_t quad[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
        Script s = { 15, { {2, 0, L0, S}, {0, 0, L1, S}, {1, 0, L1, S}, {7, 0, L1, S}, {8, 0, L0, S}, {9, 0, L1, S},
                           {10, 0, L1, S}, {1, 4000, L0, S} }, 8 };
        Rig c;
        int ok;

        rig_build(&c, &s, "PFX-QUAD", quad);
        ok = rig_run(&c, 3000, 0) && value(&c, 3) == L1 && value(&c, 4) == LZ && value(&c, 5) == L1 &&
             value(&c, 6) == L1;
        ok = ok && rig_run(&c, 6000, 0) && value(&c, 3) == L1 && value(&c, 4) == LZ && value(&c, 5) == LZ &&
             value(&c, 6) == LZ;
        expect(ok, "(k) PFX-QUAD: EN1 1, EN2 0: OUT1 1, OUT2 Z; IN2 0: OUT3 and OUT4 Z with OUT1 still 1");
        sim_destroy(c.sim);
    }
    /* (l) PFX-FREE with VIN 1: VOUT 1. */
    {
        size_t free_pins[] = { 0, 1, 2 };
        Script s = { 3, { {1, 0, L0, S}, {0, 0, L1, S} }, 2 };

        expect(reads(&s, "PFX-FREE", free_pins, 3000, 2, L1), "(l) PFX-FREE: VIN 1, no enable: VOUT 1");
    }
    /* (m) PFX-LOAD's VOUT on the same net as GND: the engine's short, a smoke error. */
    {
        size_t short_pins[] = { 0, 1, 2, 1 };
        Script s = { 3, { {1, 0, L0, S}, {0, 0, L1, S}, {2, 0, L1, S} }, 3 };
        Rig c;
        const char *error;
        int ok;

        rig_build(&c, &s, "PFX-LOAD", short_pins);
        ok = !rig_run(&c, 3000, 1);
        error = sim_error(c.sim);
        ok = ok && error != NULL && strncmp(error, "smoke:", 6) == 0;
        expect(ok, "(m) PFX-LOAD's VOUT on GND with VIN 1, ON 1: a smoke error (the engine's short)");
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

/* The level on out_net, read through a clash with a 1.2 V supply on it: returns the switched level in
   millivolts, 0 when the circuit starts (the net has no switched level), 1 when it is refused for any
   other reason. A level of 1.2 V reads 0 here: read that one through a part's supply range instead. */
static unsigned level_through_clash(Circuit *c, size_t out_net) {
    VoltageProblem p;
    char text[256];
    size_t probe = add(c, "1V2");

    wire(c, probe, "1", out_net);
    if (!refused_start(c, &p, text, sizeof text)) {
        return 0;
    }
    if (p.kind != VOLTAGE_SUPPLY_CLASH || p.min_mv != 1200) {
        return 1;
    }
    return (unsigned)p.mv;
}

/* A rig for one switch: the supply `vin` (a part name) on net hi, GND on net lo; returns the part. The
   caller wires its pins (PFX-LOAD: 1 VIN, 2 GND, 3 ON, 4 VOUT). */
typedef struct {
    Circuit *c;
    size_t part, hi, lo, out;
} Rig1;

static Rig1 rig1(const char *part, const char *vin) {
    Rig1 r;
    size_t v, gnd;

    r.c = circuit_create(library);
    r.part = add(r.c, part);
    v = add(r.c, vin);
    gnd = add(r.c, "GND");
    r.hi = net(r.c);
    r.lo = net(r.c);
    r.out = net(r.c);
    wire(r.c, v, "1", r.hi);
    wire(r.c, gnd, "1", r.lo);
    return r;
}

/* PFX-LOAD from `vin`, ON on the input (or on `en_net` when given), VOUT on r.out. */
static Rig1 load_rig(const char *vin, int enable_on_ground) {
    Rig1 r = rig1("PFX-LOAD", vin);

    wire(r.c, r.part, "1", r.hi);
    wire(r.c, r.part, "2", r.lo);
    wire(r.c, r.part, "3", enable_on_ground ? r.lo : r.hi);
    wire(r.c, r.part, "4", r.out);
    return r;
}

static void voltage_cases(void) {
    VoltageProblem p;
    char text[256];

    /* (n) 5V -> PFX-LOAD (ON on VIN): the output net is 5000 mV; a 74HC00 (2-6 V) on it: no problem; VIN at 3V3:
       3300 mV. */
    {
        Rig1 r = load_rig("5V", 0);
        unsigned level = level_through_clash(r.c, r.out);

        expect(level == 5000, "(n) 5 V -> PFX-LOAD: the output net is 5000 mV");
        circuit_destroy(r.c);
        r = load_rig("5V", 0);
        {
            size_t chip = add(r.c, "SN74HC00N");

            wire(r.c, chip, "14", r.out);
            wire(r.c, chip, "7", r.lo);
        }
        expect(circuit_start(r.c) == 0, "(n) 5 V -> PFX-LOAD -> a 74HC00 (2-6 V): no voltage problem");
        circuit_destroy(r.c);
        r = load_rig("3V3", 0);
        level = level_through_clash(r.c, r.out);
        expect(level == 3300, "(n) 3.3 V -> PFX-LOAD: the output net is 3300 mV");
        circuit_destroy(r.c);
    }
    /* (o) ORing: two ideal diodes (or a load switch and an ideal diode) at 5 V and at 3.3 V into one net: no
       problem; the net reads 5000 mV. PFX-DIODE: 1 VIN, 2 GND, 3 CE, 4 ST, 5 VOUT. */
    {
        int k;

        for (k = 0; k < 4; k++) {
            Circuit *c = circuit_create(library);
            size_t a = add(c, k < 2 ? "PFX-DIODE" : "PFX-LOAD"), b = add(c, "PFX-DIODE");
            size_t v5 = add(c, "5V"), v3 = add(c, "3V3"), gnd = add(c, "GND");
            size_t n5 = net(c), n3 = net(c), lo = net(c), out = net(c);

            wire(c, v5, "1", n5);
            wire(c, v3, "1", n3);
            wire(c, gnd, "1", lo);
            if (k < 2) {
                wire(c, a, "1", n5);
                wire(c, a, "2", lo);
                wire(c, a, "5", out);
            } else {
                wire(c, a, "1", n5);
                wire(c, a, "2", lo);
                wire(c, a, "3", n5);
                wire(c, a, "4", out);
            }
            wire(c, b, "1", n3);
            wire(c, b, "2", lo);
            wire(c, b, "5", out);
            if (k % 2 == 0) {
                expect(circuit_start(c) == 0, k == 0 ? "(o) two PFX-DIODE (5 V and 3.3 V) on one net: no problem"
                                                     : "(o) PFX-LOAD (5 V) and PFX-DIODE (3.3 V) on one net: no problem");
            } else {
                unsigned level = level_through_clash(c, out);

                expect(level == 5000, k == 1 ? "(o) the two-diode net reads 5000 mV (the higher level)"
                                             : "(o) the load switch + diode net reads 5000 mV (the higher level)");
            }
            circuit_destroy(c);
        }
    }
    /* (p) PFX-LOAD VIN at 5V with VOUT on the 3V3 supply: one clash; VOUT on the 5V supply: no problem. */
    {
        Rig1 r = load_rig("5V", 0);
        size_t v3 = add(r.c, "3V3");

        wire(r.c, v3, "1", r.out);
        expect(refused_start(r.c, &p, text, sizeof text) && p.kind == VOLTAGE_SUPPLY_CLASH && p.mv == 5000 &&
                   p.min_mv == 3300 && p.max_mv == 3300 && p.other_part == r.part &&
                   strcmp(text, "voltage: 3.3 V and 5 V supplies on one net (3V3, PFX-LOAD)") == 0,
               "(p) PFX-LOAD at 5 V with VOUT on the 3V3 supply: one VOLTAGE_SUPPLY_CLASH, 5000 against 3300");
        if (strcmp(text, "voltage: 3.3 V and 5 V supplies on one net (3V3, PFX-LOAD)") != 0) {
            printf("  text: %s\n", text);
        }
        circuit_destroy(r.c);
        r = load_rig("5V", 0);
        {
            size_t v5 = add(r.c, "5V");

            wire(r.c, v5, "1", r.out);
        }
        expect(circuit_start(r.c) == 0, "(p) PFX-LOAD at 5 V with VOUT on the 5V supply: no problem");
        circuit_destroy(r.c);
    }
    /* (q) PFX-LOAD at 5V and RFX-LDO33 (5V -> 3.3 V) outputs on one net: one clash, the regulator the lower. */
    {
        Rig1 r = load_rig("5V", 0);
        size_t reg = add(r.c, "RFX-LDO33");

        wire(r.c, reg, "1", r.hi);
        wire(r.c, reg, "3", r.hi);
        wire(r.c, reg, "2", r.lo);
        wire(r.c, reg, "5", r.out);
        expect(refused_start(r.c, &p, text, sizeof text) && p.kind == VOLTAGE_SUPPLY_CLASH && p.mv == 5000 &&
                   p.min_mv == 3300 && p.part == reg && p.other_part == r.part &&
                   strcmp(text, "voltage: 3.3 V and 5 V supplies on one net (RFX-LDO33, PFX-LOAD)") == 0,
               "(q) PFX-LOAD (5 V) and RFX-LDO33 (3.3 V) outputs on one net: one VOLTAGE_SUPPLY_CLASH");
        if (strcmp(text, "voltage: 3.3 V and 5 V supplies on one net (RFX-LDO33, PFX-LOAD)") != 0) {
            printf("  text: %s\n", text);
        }
        circuit_destroy(r.c);
    }
    /* (r) Chains: 12V -> RFX-LDO5 -> PFX-LOAD -> a 74HC00 is powered at 5 V; RFX-LDO33 -> PFX-DIODE ->
       PFX-LOAD -> a 0.8-2.7 V part is refused with 3.3 V (the level passes through two switches). */
    {
        Circuit *c = circuit_create(library);
        size_t ldo = add(c, "RFX-LDO5"), load = add(c, "PFX-LOAD"), chip = add(c, "SN74HC00N");
        size_t v12 = add(c, "12V"), gnd = add(c, "GND");
        size_t top = net(c), lo = net(c), mid = net(c), out = net(c);

        wire(c, v12, "1", top);
        wire(c, gnd, "1", lo);
        wire(c, ldo, "1", top);
        wire(c, ldo, "2", lo);
        wire(c, ldo, "3", mid);
        wire(c, load, "1", mid);
        wire(c, load, "2", lo);
        wire(c, load, "3", mid);
        wire(c, load, "4", out);
        wire(c, chip, "14", out);
        wire(c, chip, "7", lo);
        expect(circuit_start(c) == 0, "(r) 12 V -> RFX-LDO5 -> PFX-LOAD -> a 74HC00 (2-6 V): no voltage problem");
        circuit_destroy(c);
    }
    {
        Circuit *c = circuit_create(library);
        size_t ldo = add(c, "RFX-LDO33"), diode = add(c, "PFX-DIODE"), load = add(c, "PFX-LOAD");
        size_t chip = add(c, "SN74AUC00RGYR"), v5 = add(c, "5V"), gnd = add(c, "GND");
        size_t hi = net(c), lo = net(c), reg_out = net(c), mid = net(c), out = net(c);

        wire(c, v5, "1", hi);
        wire(c, gnd, "1", lo);
        wire(c, ldo, "1", hi);
        wire(c, ldo, "3", hi);
        wire(c, ldo, "2", lo);
        wire(c, ldo, "5", reg_out);
        wire(c, diode, "1", reg_out);
        wire(c, diode, "2", lo);
        wire(c, diode, "5", mid);
        wire(c, load, "1", mid);
        wire(c, load, "2", lo);
        wire(c, load, "3", mid);
        wire(c, load, "4", out);
        wire(c, chip, "14", out);
        wire(c, chip, "7", lo);
        expect(refused_start(c, &p, text, sizeof text) && p.kind == VOLTAGE_POWER_RANGE && p.mv == 3300 &&
                   p.part == chip &&
                   strcmp(text, "voltage: 3.3 V on pin 14 (VCC) of SN74AUC00RGYR; it needs 0.8-2.7 V") == 0,
               "(r) RFX-LDO33 -> PFX-DIODE -> PFX-LOAD -> a 0.8-2.7 V part: refused with 3.3 V");
        if (strcmp(text, "voltage: 3.3 V on pin 14 (VCC) of SN74AUC00RGYR; it needs 0.8-2.7 V") != 0) {
            printf("  text: %s\n", text);
        }
        circuit_destroy(c);
    }
    /* (s) PFX-LOAD at 1.2 V (vinMin 1): the output is 1200 mV (read through a 2-6 V part's range); at 0.5 V
       (below vinMin): no level; ON tied to GND: no level. */
    {
        Rig1 r = load_rig("1V2", 0);
        size_t chip = add(r.c, "SN74HC00N");

        wire(r.c, chip, "14", r.out);
        wire(r.c, chip, "7", r.lo);
        expect(refused_start(r.c, &p, text, sizeof text) && p.kind == VOLTAGE_POWER_RANGE && p.mv == 1200 &&
                   p.part == chip,
               "(s) PFX-LOAD at 1.2 V (vinMin 1): the output is 1200 mV (a 2-6 V part there is refused with 1.2 V)");
        circuit_destroy(r.c);
        r = load_rig("SFX-0V5", 0);
        {
            size_t c2 = add(r.c, "SN74HC00N");

            wire(r.c, c2, "14", r.out);
            wire(r.c, c2, "7", r.lo);
        }
        expect(circuit_start(r.c) == 0, "(s) PFX-LOAD at 0.5 V (below vinMin): the output has no level");
        circuit_destroy(r.c);
        r = load_rig("5V", 1);
        expect(level_through_clash(r.c, r.out) == 0, "(s) PFX-LOAD with ON tied to GND: the output has no level");
        circuit_destroy(r.c);
    }
    /* (t) PFX-DIODE with CE on a 5V supply and VIN at 3V3: no level; CE on its own VOUT: 3300 mV. */
    {
        Rig1 r = rig1("PFX-DIODE", "3V3");
        size_t v5 = add(r.c, "5V");
        size_t n5 = net(r.c);

        wire(r.c, v5, "1", n5);
        wire(r.c, r.part, "1", r.hi);
        wire(r.c, r.part, "2", r.lo);
        wire(r.c, r.part, "3", n5);
        wire(r.c, r.part, "5", r.out);
        expect(level_through_clash(r.c, r.out) == 0, "(t) PFX-DIODE, CE on 5 V and VIN at 3.3 V: the output has no level");
        circuit_destroy(r.c);
        r = rig1("PFX-DIODE", "3V3");
        wire(r.c, r.part, "1", r.hi);
        wire(r.c, r.part, "2", r.lo);
        wire(r.c, r.part, "3", r.out);
        wire(r.c, r.part, "5", r.out);
        expect(level_through_clash(r.c, r.out) == 3300, "(t) PFX-DIODE with CE on its own VOUT: the output is 3300 mV");
        circuit_destroy(r.c);
    }
    /* (u) 12V on PFX-LOAD's VIN: the input rating; ON on 12V likewise; PFX-FREE (no vinMax) from 12V: 12000 mV. */
    {
        Rig1 r = load_rig("12V", 0);

        expect(refused_start(r.c, &p, text, sizeof text) && p.kind == VOLTAGE_POWERSWITCH_INPUT_OVER &&
                   p.max_mv == 5500 && p.mv == 12000 && p.part == r.part && p.pin == 0 &&
                   strcmp(text, "voltage: 12 V on pin 1 (VIN) of PFX-LOAD; its input max is 5.5 V") == 0,
               "(u) 12 V on PFX-LOAD's VIN: VOLTAGE_POWERSWITCH_INPUT_OVER, max 5500, 'its input max is 5.5 V'");
        if (strcmp(text, "voltage: 12 V on pin 1 (VIN) of PFX-LOAD; its input max is 5.5 V") != 0) {
            printf("  text: %s\n", text);
        }
        circuit_destroy(r.c);
        r = rig1("PFX-LOAD", "5V");
        {
            size_t v12 = add(r.c, "12V");
            size_t top = net(r.c);

            wire(r.c, v12, "1", top);
            wire(r.c, r.part, "1", r.hi);
            wire(r.c, r.part, "2", r.lo);
            wire(r.c, r.part, "3", top);
        }
        expect(refused_start(r.c, &p, text, sizeof text) && p.kind == VOLTAGE_POWERSWITCH_INPUT_OVER && p.pin == 2 &&
                   p.mv == 12000,
               "(u) 12 V on PFX-LOAD's ON: refused too (the enable is rated to the input maximum)");
        circuit_destroy(r.c);
        r = rig1("PFX-FREE", "12V");
        wire(r.c, r.part, "1", r.hi);
        wire(r.c, r.part, "2", r.lo);
        wire(r.c, r.part, "3", r.out);
        expect(level_through_clash(r.c, r.out) == 12000,
               "(u) PFX-FREE (no vinMax) from 12 V: no input problem, the output is 12000 mV");
        circuit_destroy(r.c);
    }
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 || load_regulator_fixture(library) != 0 ||
        load_powerswitch_fixture(library) != 0) {
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
