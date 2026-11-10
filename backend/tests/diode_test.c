/*
 * diode_test (plan-29-H): one-way diode links in the engine.
 *
 * Engine cases (plan-29-H section 4.3) use a scripted driver with one pin
 * per net and a fixture diode component (pin 0 anode, pin 1 cathode) whose
 * `diodes` hook returns one link. A 1 passes anode to cathode, a 0 cathode
 * to anode, nothing the other way; a strong 1 against a strong 0 across a
 * forward diode is a short.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "net.h"
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

static size_t diode_pin_count(const void *config) {
    (void)config;
    return 2;
}

static size_t diode_links(const void *state, size_t *anode, size_t *cathode, size_t cap) {
    (void)state;
    if (cap > 0) {
        anode[0] = 0;
        cathode[0] = 1;
    }
    return 1;
}

static const SimComponentType DIODE_TYPE = {
    .name = "test diode",
    .pin_count = diode_pin_count,
    .create = pass_config,
    .diodes = diode_links,
};

/* A circuit: nets 0..n-1, the driver on every net, diodes between nets. */
typedef struct {
    Sim *sim;
    SimNet *nets[MAX_NETS];
    int started;
} Circuit;

typedef struct {
    size_t anode;
    size_t cathode;
} Link;

static void circuit_build(Circuit *c, const Script *script, const Link *links, size_t link_count) {
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
        char name[8];
        SimInstance *d;

        snprintf(name, sizeof(name), "D%zu", i + 1);
        d = sim_add_instance(c->sim, &DIODE_TYPE, name, &DIODE_TYPE);
        sim_connect(c->sim, c->nets[links[i].anode], d, 0);
        sim_connect(c->sim, c->nets[links[i].cathode], d, 1);
    }
}

/* Starts (the first time) and runs to t; 1 when the engine reported no error. */
static int circuit_run(Circuit *c, SimTime t) {
    if (!c->started) {
        c->started = 1;
        if (!sim_start(c->sim)) {
            return 0;
        }
    }
    return sim_run_until(c->sim, t);
}

static LogicValue value(const Circuit *c, size_t net) {
    return sim_net_value(c->nets[net]);
}

static int is_smoke(const Circuit *c) {
    const char *e = sim_error(c->sim);

    return e != NULL && strncmp(e, "smoke:", 6) == 0;
}

#define S DRIVE_STRONG
#define W DRIVE_WEAK

/* One net pair across one diode: anode net 0, cathode net 1. */
static void one_diode(const char *what, const Drive *drives, size_t drive_count,
                      LogicValue want_anode, LogicValue want_cathode, int want_smoke) {
    Script script = { 2, {{0}}, 0 };
    Link link = { 0, 1 };
    Circuit c;
    int ok;
    size_t i;

    for (i = 0; i < drive_count; i++) {
        script.drives[i] = drives[i];
    }
    script.drive_count = drive_count;
    circuit_build(&c, &script, &link, 1);
    ok = circuit_run(&c, 1000);
    if (want_smoke) {
        expect(!ok && is_smoke(&c), what);
    } else {
        if (!ok) {
            printf("  engine error: %s\n", sim_error(c.sim));
        }
        expect(ok && value(&c, 0) == want_anode && value(&c, 1) == want_cathode, what);
    }
    sim_destroy(c.sim);
}

static void engine_cases(void) {
    {
        Drive d[] = { {0, 0, LOGIC_1, S}, {1, 0, LOGIC_0, W} };
        one_diode("strong 1 on anode, cathode pulled down: cathode 1", d, 2, LOGIC_1, LOGIC_1, 0);
    }
    {
        Drive d[] = { {0, 0, LOGIC_0, S}, {1, 0, LOGIC_0, W} };
        one_diode("strong 0 on anode, cathode pulled down: cathode 0", d, 2, LOGIC_0, LOGIC_0, 0);
    }
    {
        Drive d[] = { {0, 0, LOGIC_1, S}, {1, 0, LOGIC_0, S} };
        one_diode("strong 1 on anode, strong 0 on cathode: smoke", d, 2, LOGIC_Z, LOGIC_Z, 1);
    }
    {
        Drive d[] = { {0, 0, LOGIC_0, S}, {1, 0, LOGIC_1, S} };
        one_diode("strong 0 on anode, strong 1 on cathode: both keep their values", d, 2,
                  LOGIC_0, LOGIC_1, 0);
    }
    {
        Drive d[] = { {0, 0, LOGIC_X, S}, {1, 0, LOGIC_0, W} };
        one_diode("X driven strong on an anode: cathode X", d, 2, LOGIC_X, LOGIC_X, 0);
    }
    one_diode("nothing on either side: both Z", NULL, 0, LOGIC_Z, LOGIC_Z, 0);

    /* Diode-OR: gates on nets 0 and 1, anodes; common cathode net 2, pull-down. */
    {
        static const LogicValue in[4][2] = {
            {LOGIC_0, LOGIC_0}, {LOGIC_0, LOGIC_1}, {LOGIC_1, LOGIC_0}, {LOGIC_1, LOGIC_1}
        };
        Link links[] = { {0, 2}, {1, 2} };
        int ok = 1;
        size_t k;

        for (k = 0; k < 4; k++) {
            Script script = { 3, { {0, 0, in[k][0], S}, {1, 0, in[k][1], S}, {2, 0, LOGIC_0, W} }, 3 };
            Circuit c;
            LogicValue want = (in[k][0] == LOGIC_1 || in[k][1] == LOGIC_1) ? LOGIC_1 : LOGIC_0;

            circuit_build(&c, &script, links, 2);
            if (!circuit_run(&c, 1000) || value(&c, 2) != want ||
                value(&c, 0) != in[k][0] || value(&c, 1) != in[k][1]) {
                printf("  inputs %d %d: out %d, gates %d %d\n", (int)in[k][0], (int)in[k][1],
                       (int)value(&c, 2), (int)value(&c, 0), (int)value(&c, 1));
                ok = 0;
            }
            sim_destroy(c.sim);
        }
        expect(ok, "diode-OR: OR of the gates, neither gate's net changes");
    }

    /* Diode-AND: gates on cathodes (nets 0, 1), common anode net 2, pull-up. */
    {
        static const LogicValue in[4][2] = {
            {LOGIC_0, LOGIC_0}, {LOGIC_0, LOGIC_1}, {LOGIC_1, LOGIC_0}, {LOGIC_1, LOGIC_1}
        };
        Link links[] = { {2, 0}, {2, 1} };
        int ok = 1;
        size_t k;

        for (k = 0; k < 4; k++) {
            Script script = { 3, { {0, 0, in[k][0], S}, {1, 0, in[k][1], S}, {2, 0, LOGIC_1, W} }, 3 };
            Circuit c;
            LogicValue want = (in[k][0] == LOGIC_1 && in[k][1] == LOGIC_1) ? LOGIC_1 : LOGIC_0;

            circuit_build(&c, &script, links, 2);
            if (!circuit_run(&c, 1000) || value(&c, 2) != want ||
                value(&c, 0) != in[k][0] || value(&c, 1) != in[k][1]) {
                printf("  inputs %d %d: out %d\n", (int)in[k][0], (int)in[k][1], (int)value(&c, 2));
                ok = 0;
            }
            sim_destroy(c.sim);
        }
        expect(ok, "diode-AND: AND of the gates");
    }

    /* Two diodes in series: 0 -> 1 -> 2, strong 1 at 0, pull-down at 2. */
    {
        Script script = { 3, { {0, 0, LOGIC_1, S}, {2, 0, LOGIC_0, W} }, 2 };
        Link links[] = { {0, 1}, {1, 2} };
        Circuit c;

        circuit_build(&c, &script, links, 2);
        expect(circuit_run(&c, 1000) && value(&c, 2) == LOGIC_1 && value(&c, 1) == LOGIC_1,
               "two diodes in series: the end and the middle read 1");
        sim_destroy(c.sim);
    }

    /* Antiparallel pair between nets 0 and 1; a weak 1 on 0 until 100 ps. */
    {
        Script script = { 2, { {0, 0, LOGIC_1, W}, {0, 100, LOGIC_Z, W} }, 2 };
        Link links[] = { {0, 1}, {1, 0} };
        Circuit c;
        int ok;

        circuit_build(&c, &script, links, 2);
        ok = circuit_run(&c, 50) && value(&c, 0) == LOGIC_1 && value(&c, 1) == LOGIC_1;
        expect(ok, "antiparallel pair, weak 1 on one side: both 1");
        ok = circuit_run(&c, 200) && value(&c, 0) == LOGIC_Z && value(&c, 1) == LOGIC_Z;
        expect(ok, "antiparallel pair: both Z when the 1 goes (no latch)");
        sim_destroy(c.sim);
    }

    /* A diode-OR run: gate on net 0 toggles every 100 ps for 1000 ps. */
    {
        Script script = { 3, {{0}}, 0 };
        Link links[] = { {0, 2}, {1, 2} };
        Circuit c;
        int ok = 1;
        size_t k;

        script.drives[script.drive_count++] = (Drive){1, 0, LOGIC_0, S};
        script.drives[script.drive_count++] = (Drive){2, 0, LOGIC_0, W};
        for (k = 0; k < 10; k++) {
            script.drives[script.drive_count++] =
                (Drive){0, k * 100, (k % 2) ? LOGIC_0 : LOGIC_1, S};
        }
        circuit_build(&c, &script, links, 2);
        for (k = 0; k < 10 && ok; k++) {
            SimTime t = k * 100 + 50;
            LogicValue want = (k % 2) ? LOGIC_0 : LOGIC_1;

            ok = circuit_run(&c, t) && value(&c, 2) == want;
        }
        expect(ok, "diode-OR run: the common net follows the gate with no extra delay");
        expect(sim_net_changes(c.nets[2]) == 10, "diode-OR run: 10 edges on the common net");
        sim_destroy(c.sim);
    }
}

int main(void) {
    engine_cases();
    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
