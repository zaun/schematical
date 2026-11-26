/*
 * engine_misuse_test (plan-68-L): the engine's API on misuse — a NULL or
 * foreign simulator, instance or net, a pin out of range, a call after
 * start, a time that runs backwards or overflows — sets the error it
 * documents and changes nothing; and the engine's own limits: a circuit
 * that never settles, diodes on pins a part does not have, growth that
 * would overflow.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "net.h"
#include "sim.h"
#include "sim_int.h"

static int failures;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

static int error_is(const Sim *sim, const char *message) {
    return sim_error(sim) != NULL && strcmp(sim_error(sim), message) == 0;
}

/* ---- Fixture parts ---- */

static size_t two_pins(const void *config) {
    (void)config;
    return 2;
}

/* A state to hand back: the engine reads a NULL state as out of memory. */
static void *no_state(const void *config) {
    static int state;

    (void)config;
    return &state;
}

/* Two pins and nothing else. */
static const SimComponentType PLAIN = {.name = "plain", .pin_count = two_pins, .create = no_state};

/* Drives pin 0 at start; whenever its pin 1 changes, drives pin 0 the other
   way at once. Wired pin 0 to pin 1 it never settles. */
static void flip_start(Sim *sim, SimInstance *instance, void *state) {
    (void)state;
    sim_schedule_drive(sim, instance, 0, LOGIC_0, DRIVE_STRONG, 0);
}

static void flip_changed(Sim *sim, SimInstance *instance, void *state, size_t pin, LogicValue old_value,
                         LogicValue new_value) {
    (void)state;
    (void)old_value;
    if (pin == 1) {
        sim_schedule_drive(sim, instance, 0, new_value == LOGIC_1 ? LOGIC_0 : LOGIC_1, DRIVE_STRONG, 0);
    }
}

static const SimComponentType FLIPPER = {
    .name = "flipper", .pin_count = two_pins, .create = no_state, .start = flip_start, .pin_changed = flip_changed};

/* A diode from pin 0 to pin 1, a diode on pin 9 (no such pin), and a diode
   between its own two pins when they share a net. */
static size_t three_diodes(const void *state, size_t *anode, size_t *cathode, size_t cap) {
    static const size_t A[] = {0, 9, 1}, C[] = {1, 0, 1};
    size_t i;

    (void)state;
    for (i = 0; i < 3 && i < cap; i++) {
        anode[i] = A[i];
        cathode[i] = C[i];
    }
    return 3;
}

static const SimComponentType DIODES = {
    .name = "diodes", .pin_count = two_pins, .create = no_state, .diodes = three_diodes};

/* ---- Cases ---- */

static void check_null_and_foreign(void) {
    Sim *sim = sim_create();
    Sim *other = sim_create();
    SimInstance *mine = sim_add_instance(sim, &PLAIN, NULL, NULL);
    SimInstance *theirs = sim_add_instance(other, &PLAIN, "x", NULL);
    SimNet *net = sim_add_net(sim, NULL);

    sim_destroy(NULL);
    expect(mine != NULL && net != NULL, "an instance and a net with no name are made");
    expect(strcmp(sim_net_name(net), "") == 0 && sim_net_name(NULL) == NULL, "a net with no name is named \"\"");
    expect(sim_add_instance(NULL, &PLAIN, "x", NULL) == NULL && sim_add_net(NULL, "n") == NULL,
           "a NULL simulator makes nothing");
    expect(sim_connect(NULL, net, mine, 0) == 0, "a NULL simulator connects nothing");
    expect(sim_read_pin(NULL, mine, 0) == LOGIC_X && sim_read_pin(sim, theirs, 0) == LOGIC_X &&
               sim_read_pin(sim, mine, 7) == LOGIC_X && sim_read_pin(sim, NULL, 0) == LOGIC_X,
           "an unknown pin reads X");
    expect(sim_contact_closed(NULL, mine, 0, 1) == 0, "a NULL simulator has no contact closed");
    net_destroy(NULL);
    sim_destroy(other);

    /* A type with no pin count. */
    {
        static const SimComponentType NO_PINS = {.name = "no pins"};

        expect(sim_add_instance(sim, &NO_PINS, "x", NULL) == NULL && error_is(sim, "invalid instance creation"),
               "a type with no pin count is refused");
    }
    expect(sim_add_instance(sim, &PLAIN, "x", NULL) == NULL && sim_add_net(sim, "n") == NULL &&
               sim_connect(sim, net, mine, 0) == 0,
           "after an error the simulator makes and connects nothing");
    sim_destroy(sim);
}

static void check_connections(void) {
    Sim *sim = sim_create();
    Sim *other = sim_create();
    SimInstance *a = sim_add_instance(sim, &PLAIN, "a", NULL);
    SimNet *net = sim_add_net(sim, "n");
    SimNet *foreign = sim_add_net(other, "f");

    expect(sim_connect(sim, net, a, 5) == 0 && error_is(sim, "invalid connection"), "a pin out of range is refused");
    sim_destroy(sim);
    sim = sim_create();
    a = sim_add_instance(sim, &PLAIN, "a", NULL);
    net = sim_add_net(sim, "n");
    expect(sim_connect(sim, foreign, a, 0) == 0 && error_is(sim, "invalid connection"), "a foreign net is refused");
    sim_destroy(sim);
    sim = sim_create();
    a = sim_add_instance(sim, &PLAIN, "a", NULL);
    net = sim_add_net(sim, "n");
    expect(sim_connect(sim, net, a, 0) == 1, "a pin connects");
    expect(sim_connect(sim, net, a, 0) == 0 && error_is(sim, "pin is already connected"),
           "a pin connects only once");
    sim_destroy(sim);
    sim_destroy(other);
}

static void check_after_start(void) {
    Sim *sim = sim_create();
    SimInstance *a = sim_add_instance(sim, &PLAIN, "a", NULL);
    SimNet *net = sim_add_net(sim, "n");

    sim_connect(sim, net, a, 0);
    expect(sim_start(sim) == 1, "it starts");
    expect(sim_start(sim) == 0 && error_is(sim, "simulation already started"), "it starts once");
    sim_destroy(sim);

    sim = sim_create();
    a = sim_add_instance(sim, &PLAIN, "a", NULL);
    sim_start(sim);
    expect(sim_add_net(sim, "late") == NULL && error_is(sim, "cannot add a net after starting"),
           "no net after start");
    sim_destroy(sim);

    sim = sim_create();
    sim_start(sim);
    expect(sim_add_instance(sim, &PLAIN, "late", NULL) == NULL && error_is(sim, "invalid instance creation"),
           "no part after start");
    sim_destroy(sim);
}

static void check_events(void) {
    Sim *sim = sim_create();
    SimInstance *a = sim_add_instance(sim, &PLAIN, "a", NULL);

    sim_start(sim);
    sim_schedule_drive(sim, a, 0, (LogicValue)99, DRIVE_STRONG, 0);
    expect(error_is(sim, "invalid drive event"), "a drive of no value is refused");
    sim_destroy(sim);

    sim = sim_create();
    a = sim_add_instance(sim, &PLAIN, "a", NULL);
    sim_start(sim);
    sim_run_until(sim, 10);
    sim_schedule_drive(sim, a, 0, LOGIC_1, DRIVE_STRONG, UINT64_MAX);
    expect(error_is(sim, "event time overflow"), "a drive past the end of time is refused");
    sim_destroy(sim);

    sim = sim_create();
    a = sim_add_instance(sim, &PLAIN, "a", NULL);
    sim_start(sim);
    sim_schedule_timer(sim, NULL, 1, 0);
    expect(error_is(sim, "invalid timer event"), "a timer for no part is refused");
    sim_destroy(sim);

    sim = sim_create();
    a = sim_add_instance(sim, &PLAIN, "a", NULL);
    sim_start(sim);
    sim_run_until(sim, 10);
    sim_schedule_timer(sim, a, 1, UINT64_MAX);
    expect(error_is(sim, "event time overflow"), "a timer past the end of time is refused");
    sim_destroy(sim);

    sim = sim_create();
    a = sim_add_instance(sim, &PLAIN, "a", NULL);
    sim_start(sim);
    sim_run_until(sim, 10);
    expect(sim_run_until(sim, 5) == 0 && error_is(sim, "invalid run time"), "time does not run backwards");
    sim_destroy(sim);
}

static void check_contacts(void) {
    Sim *sim = sim_create();
    SimInstance *a = sim_add_instance(sim, &PLAIN, "a", NULL);

    sim_set_contact(sim, a, 1, 0, 1);
    expect(sim_error(sim) == NULL && sim_contact_closed(sim, a, 0, 1) && sim_contact_closed(sim, a, 1, 0),
           "a contact named either way round is one contact");
    sim_set_contact(sim, a, 0, 0, 1);
    expect(error_is(sim, "invalid contact"), "a contact from a pin to itself is refused");
    sim_destroy(sim);
}

static void check_limits(void) {
    Sim *sim = sim_create();
    SimInstance *flipper = sim_add_instance(sim, &FLIPPER, "f", NULL);
    SimNet *loop = sim_add_net(sim, "loop");
    SimNet *net;
    size_t capacity = SIZE_MAX / 2 + 1;

    sim_connect(sim, loop, flipper, 0);
    sim_connect(sim, loop, flipper, 1);
    sim_start(sim);
    expect(sim_run_until(sim, 100) == 0 && error_is(sim, "circuit did not settle"),
           "a part that answers every change at once never settles");
    sim_destroy(sim);

    /* Diodes: one on a pin the part lacks is left out; one whose ends share
       a net joins nothing. */
    sim = sim_create();
    {
        SimInstance *d = sim_add_instance(sim, &DIODES, "d", NULL);
        SimInstance *drive = sim_add_instance(sim, &FLIPPER, "src", NULL);
        SimNet *n0 = sim_add_net(sim, "n0"), *n1 = sim_add_net(sim, "n1");

        sim_connect(sim, n0, d, 0);
        sim_connect(sim, n1, d, 1);
        sim_connect(sim, n0, drive, 0);
        expect(sim_start(sim) == 1 && sim_run_until(sim, 1000) == 1 && sim_error(sim) == NULL,
               "a diode on a missing pin is left out");
        expect(sim_net_value(n0) == LOGIC_0, "the source still drives its net");
    }
    sim_destroy(sim);

    /* A diode between two nets a closed contact joins: both ends are one
       group, so it links nothing, and the joined nets read alike. */
    sim = sim_create();
    {
        SimInstance *d = sim_add_instance(sim, &DIODES, "d", NULL);
        SimInstance *sw = sim_add_instance(sim, &PLAIN, "sw", NULL);
        SimInstance *drive = sim_add_instance(sim, &FLIPPER, "src", NULL);
        SimNet *n0 = sim_add_net(sim, "n0"), *n1 = sim_add_net(sim, "n1");

        sim_connect(sim, n0, d, 0);
        sim_connect(sim, n1, d, 1);
        sim_connect(sim, n0, sw, 0);
        sim_connect(sim, n1, sw, 1);
        sim_connect(sim, n0, drive, 0);
        sim_set_contact(sim, sw, 0, 1, 1);
        expect(sim_start(sim) == 1 && sim_run_until(sim, 1000) == 1 && sim_net_value(n0) == LOGIC_0 &&
                   sim_net_value(n1) == LOGIC_0,
               "a diode inside one contact group joins nothing; the contact carries the 0");
    }
    sim_destroy(sim);

    /* Growth that would overflow is refused before anything is allocated. */
    sim = sim_create();
    expect(simc_grow_array(NULL, &capacity, 16) == NULL && capacity == SIZE_MAX / 2 + 1,
           "an array that cannot double is not grown");
    net = sim_add_net(sim, "big");
    {
        size_t count = net->pin_count, cap = net->pin_capacity;

        net->pin_count = net->pin_capacity = SIZE_MAX / 2 + 1;
        expect(net_attach(net, NULL, 0) == 0, "a net that cannot double its pins attaches nothing");
        net->pin_count = count;
        net->pin_capacity = cap;
    }
    sim_destroy(sim);
}

int main(void) {
    check_null_and_foreign();
    check_connections();
    check_after_start();
    check_events();
    check_contacts();
    check_limits();
    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
