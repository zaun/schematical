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

static size_t pin_named(const Bench *b, const char *name);

Bench *bench_create(const Part *part) {
    return bench_create_joined(part, NULL);
}

Bench *bench_create_joined(const Part *part, const char *const *joins) {
    Bench *b = calloc(1, sizeof(*b));
    SimNet **nets;
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
    nets = calloc(b->pin_count, sizeof(*nets));
    for (i = 0; joins != NULL && joins[i] != NULL && joins[i + 1] != NULL; i += 2) {
        size_t from = pin_named(b, joins[i]);
        size_t to = pin_named(b, joins[i + 1]);

        if (nets[from] == NULL) {
            nets[from] = sim_add_net(b->sim, "n");
        }
        nets[to] = nets[from];
    }
    for (i = 0; i < b->pin_count; i++) {
        PinRole role = part->pins[i].role;
        SimNet *net = nets[i] != NULL ? nets[i] : sim_add_net(b->sim, "n");

        sim_connect(b->sim, net, b->part, i);
        if (role == PIN_INPUT || role == PIN_INOUT || role == PIN_POWER || role == PIN_GROUND) {
            sim_connect(b->sim, net, b->driver, i);
        }
    }
    free(nets);
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

Sim *bench_sim(Bench *b) {
    return b->sim;
}

void *bench_state(Bench *b) {
    return sim_instance_state(b->part);
}

int bench_has_pin(const Bench *b, const char *pin_name) {
    size_t i;

    for (i = 0; i < b->pin_count; i++) {
        if (strcmp(b->config->pins[i].name, pin_name) == 0) {
            return 1;
        }
    }
    return 0;
}

/* ---- 65xx bus cycles ------------------------------------------------------------ */

#define BUS65_NS 1000ull

static const char *const BUS65_D[8] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"};
static const char *const BUS65_RS[4] = {"RS0", "RS1", "RS2", "RS3"};

static void bus65_selects(Bench *b, int selected) {
    if (bench_has_pin(b, "CS0")) bench_set(b, "CS0", LOGIC_1);
    if (bench_has_pin(b, "CS1")) bench_set(b, "CS1", LOGIC_1);
    if (bench_has_pin(b, "CS2B")) bench_set(b, "CS2B", selected ? LOGIC_0 : LOGIC_1);
    if (bench_has_pin(b, "CS1B")) bench_set(b, "CS1B", selected ? LOGIC_0 : LOGIC_1);
}

static void bus65_setup(Bench *b, int rw, unsigned reg, int selected) {
    int i;

    bench_run(b, 30 * BUS65_NS);
    for (i = 0; i < 8; i++) bench_set(b, BUS65_D[i], LOGIC_Z);
    bench_set(b, "RWB", rw ? LOGIC_1 : LOGIC_0);
    for (i = 0; i < 4 && bench_has_pin(b, BUS65_RS[i]); i++) {
        bench_set(b, BUS65_RS[i], (reg >> i & 1) ? LOGIC_1 : LOGIC_0);
    }
    bus65_selects(b, selected);
    bench_run(b, 470 * BUS65_NS);
    bench_set(b, "PHI2", LOGIC_1);
}

void bench_bus65_begin(Bench *b) {
    bench_set(b, "PHI2", LOGIC_0);
    bench_set(b, "RWB", LOGIC_1);
    bus65_selects(b, 0);
    bench_run(b, 1000 * BUS65_NS);
}

void bench_bus65_write(Bench *b, unsigned reg, uint8_t value) {
    int i;

    bus65_setup(b, 0, reg, 1);
    bench_run(b, 100 * BUS65_NS);
    for (i = 0; i < 8; i++) bench_set(b, BUS65_D[i], (value >> i & 1) ? LOGIC_1 : LOGIC_0);
    bench_run(b, 400 * BUS65_NS);
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 0);
}

uint8_t bench_bus65_read(Bench *b, unsigned reg, uint8_t *unknown) {
    uint8_t value = 0;
    uint8_t x = 0;
    int i;

    bus65_setup(b, 1, reg, 1);
    bench_run(b, 400 * BUS65_NS);
    for (i = 0; i < 8; i++) {
        LogicValue v = bench_get(b, BUS65_D[i]);

        if (v == LOGIC_1) value |= (uint8_t)(1u << i);
        else if (v != LOGIC_0) x |= (uint8_t)(1u << i);
    }
    bench_run(b, 100 * BUS65_NS);
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 0);
    if (unknown != NULL) *unknown = x;
    return value;
}

void bench_bus65_idle(Bench *b, unsigned cycles) {
    unsigned n;

    for (n = 0; n < cycles; n++) {
        bus65_setup(b, 1, 0, 0);
        bench_run(b, 500 * BUS65_NS);
        bench_set(b, "PHI2", LOGIC_0);
        bench_run(b, 0);
    }
}

void bench_destroy(Bench *b) {
    if (b != NULL) {
        sim_destroy(b->sim);
        free(b);
    }
}
