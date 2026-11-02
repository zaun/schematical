/*
 * bench: a pin-driving bench for model tests (bench.h).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "kind.h"

struct Bench {
    Sim *sim;
    SimInstance *part;
    SimInstance *driver;
    const Part *config;
    size_t pin_count;
};

static size_t driver_pin_count(const void *config) {
    return ((const Bench *)config)->pin_count;
}

static void *driver_create(const void *config) {
    return (void *)config;
}

static void driver_start(Sim *sim, SimInstance *instance, void *state) {
    const Bench *b = state;
    size_t i;

    for (i = 0; i < b->pin_count; i++) {
        PinRole role = b->config->pins[i].role;

        if (role == PIN_POWER) {
            sim_schedule_drive(sim, instance, i, LOGIC_1, DRIVE_STRONG, 0);
        } else if (role == PIN_GROUND) {
            sim_schedule_drive(sim, instance, i, LOGIC_0, DRIVE_STRONG, 0);
        }
    }
}

static const SimComponentType DRIVER_TYPE = {
    .name = "bench", .pin_count = driver_pin_count, .create = driver_create,
    .start = driver_start
};

Bench *bench_create(const Part *part) {
    Bench *b = calloc(1, sizeof(*b));
    size_t i;

    if (b == NULL || part == NULL) {
        fprintf(stderr, "bench: no part or out of memory\n");
        exit(2);
    }
    b->config = part;
    b->pin_count = part->pin_count;
    b->sim = sim_create();
    b->part = sim_add_instance(b->sim, kind_model(part), part->name, part);
    b->driver = sim_add_instance(b->sim, &DRIVER_TYPE, "bench", b);
    if (b->sim == NULL || b->part == NULL || b->driver == NULL) {
        fprintf(stderr, "bench: cannot create %s\n", part->name);
        exit(2);
    }
    for (i = 0; i < b->pin_count; i++) {
        PinRole role = part->pins[i].role;
        SimNet *net = sim_add_net(b->sim, "n");

        sim_connect(b->sim, net, b->part, i);
        if (role == PIN_INPUT || role == PIN_INOUT || role == PIN_POWER || role == PIN_GROUND) {
            sim_connect(b->sim, net, b->driver, i);
        }
    }
    sim_start(b->sim);
    return b;
}

static size_t pin_named(const Bench *b, const char *name) {
    size_t i;

    for (i = 0; i < b->pin_count; i++) {
        if (strcmp(b->config->pins[i].name, name) == 0) {
            return i;
        }
    }
    fprintf(stderr, "bench: no pin %s on %s\n", name, b->config->name);
    exit(2);
}

void bench_set(Bench *b, const char *pin_name, LogicValue v) {
    sim_schedule_drive(b->sim, b->driver, pin_named(b, pin_name), v, DRIVE_STRONG, 0);
}

void bench_run(Bench *b, SimTime ps) {
    sim_run_until(b->sim, sim_now(b->sim) + ps);
}

LogicValue bench_get(Bench *b, const char *pin_name) {
    return sim_read_pin(b->sim, b->part, pin_named(b, pin_name));
}

const char *bench_error(const Bench *b) {
    return sim_error(b->sim);
}

void bench_destroy(Bench *b) {
    if (b != NULL) {
        sim_destroy(b->sim);
        free(b);
    }
}
