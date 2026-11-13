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

int main(void) {
    engine_cases();
    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
