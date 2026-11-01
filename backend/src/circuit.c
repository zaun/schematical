#include "circuit.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "net.h"
#include "sim.h"
#include "voltage.h"

struct Circuit {
    const PartLibrary *library;
    Sim *sim;
    const Part **parts;
    SimInstance **instances;
    size_t part_count;
    size_t part_capacity;
    size_t instance_capacity;
    SimNet **nets;
    size_t net_count;
    size_t net_capacity;
    int started;
    SimTime step_start;
    char error[256];
    int has_error;
    /* For the voltage check (plan-07-B): every connection, the closed contacts. */
    VoltageConnection *connections;
    size_t connection_count;
    size_t connection_capacity;
    VoltageContact *closed;
    size_t closed_count;
    size_t closed_capacity;
    int voltage_stopped;      /* a closing switch failed the check during a run */
    int has_problem;
    VoltageProblem problem;   /* the first problem, when has_problem */
};

static int fail(Circuit *circuit, const char *format, ...) {
    va_list args;

    va_start(args, format);
    vsnprintf(circuit->error, sizeof circuit->error, format, args);
    va_end(args);
    circuit->has_error = 1;
    return -1;
}

/* Copies the engine's error (if any) into the circuit's. */
static int engine_failed(Circuit *circuit) {
    const char *error = sim_error(circuit->sim);

    if (error != NULL) {
        snprintf(circuit->error, sizeof circuit->error, "%s", error);
        circuit->has_error = 1;
        return 1;
    }
    return 0;
}

static int grow(void **array, size_t *capacity, size_t count, size_t size) {
    void *bigger;
    size_t new_capacity;

    if (count < *capacity) {
        return 0;
    }
    new_capacity = *capacity == 0 ? 16 : *capacity * 2;
    bigger = realloc(*array, new_capacity * size);
    if (bigger == NULL) {
        return -1;
    }
    *array = bigger;
    *capacity = new_capacity;
    return 0;
}

Circuit *circuit_create(const PartLibrary *library) {
    Circuit *circuit = calloc(1, sizeof(*circuit));

    if (circuit == NULL) {
        return NULL;
    }
    circuit->library = library;
    circuit->sim = sim_create();
    if (circuit->sim == NULL) {
        free(circuit);
        return NULL;
    }
    return circuit;
}

void circuit_destroy(Circuit *circuit) {
    if (circuit == NULL) {
        return;
    }
    sim_destroy(circuit->sim);
    free(circuit->parts);
    free(circuit->instances);
    free(circuit->nets);
    free(circuit->connections);
    free(circuit->closed);
    free(circuit);
}

const char *circuit_error(const Circuit *circuit) {
    if (circuit == NULL) {
        return "no circuit";
    }
    return circuit->has_error ? circuit->error : NULL;
}

static int check_building(Circuit *circuit) {
    if (circuit->started) {
        return fail(circuit, "circuit already started");
    }
    return 0;
}

int circuit_add_part(Circuit *circuit, const char *part_name, size_t *index) {
    const Part *part;
    SimInstance *instance;
    char name[32];

    if (check_building(circuit) != 0) {
        return -1;
    }
    part = part_library_find(circuit->library, part_name != NULL ? part_name : "");
    if (part == NULL) {
        return fail(circuit, "unknown part \"%s\"", part_name != NULL ? part_name : "");
    }
    if (grow((void **)&circuit->parts, &circuit->part_capacity, circuit->part_count,
             sizeof(*circuit->parts)) != 0 ||
        grow((void **)&circuit->instances, &circuit->instance_capacity, circuit->part_count,
             sizeof(*circuit->instances)) != 0) {
        return fail(circuit, "out of memory");
    }
    snprintf(name, sizeof name, "U%zu", circuit->part_count);
    instance = sim_add_instance(circuit->sim, kind_model(part), name, part);
    if (instance == NULL) {
        engine_failed(circuit);
        return fail(circuit, "cannot create part \"%s\"", part->name);
    }
    circuit->parts[circuit->part_count] = part;
    circuit->instances[circuit->part_count] = instance;
    if (index != NULL) {
        *index = circuit->part_count;
    }
    circuit->part_count++;
    return 0;
}

int circuit_add_net(Circuit *circuit, size_t *index) {
    SimNet *net;
    char name[32];

    if (check_building(circuit) != 0) {
        return -1;
    }
    if (grow((void **)&circuit->nets, &circuit->net_capacity, circuit->net_count,
             sizeof(*circuit->nets)) != 0) {
        return fail(circuit, "out of memory");
    }
    snprintf(name, sizeof name, "N%zu", circuit->net_count);
    net = sim_add_net(circuit->sim, name);
    if (net == NULL) {
        engine_failed(circuit);
        return fail(circuit, "cannot create a net");
    }
    circuit->nets[circuit->net_count] = net;
    if (index != NULL) {
        *index = circuit->net_count;
    }
    circuit->net_count++;
    return 0;
}

static int check_part(Circuit *circuit, size_t part) {
    if (part >= circuit->part_count) {
        return fail(circuit, "no part %zu", part);
    }
    return 0;
}

int circuit_connect(Circuit *circuit, size_t part, const char *pin, size_t net) {
    const Part *info;
    size_t i;

    if (check_building(circuit) != 0 || check_part(circuit, part) != 0) {
        return -1;
    }
    if (pin == NULL) {
        return fail(circuit, "no pin given");
    }
    info = circuit->parts[part];
    for (i = 0; i < info->pin_count; i++) {
        if (strcmp(info->pins[i].designator, pin) == 0) {
            break;
        }
    }
    if (i == info->pin_count) {
        return fail(circuit, "%s has no pin %s", info->name, pin);
    }
    if (info->pins[i].role == PIN_ABSENT) {
        return fail(circuit, "pin %s of %s has no lead", pin, info->name);
    }
    if (net >= circuit->net_count) {
        return fail(circuit, "no net %zu", net);
    }
    if (grow((void **)&circuit->connections, &circuit->connection_capacity,
             circuit->connection_count, sizeof(*circuit->connections)) != 0) {
        return fail(circuit, "out of memory");
    }
    if (!sim_connect(circuit->sim, circuit->nets[net], circuit->instances[part], i)) {
        engine_failed(circuit);
        return fail(circuit, "cannot connect pin %s of part %zu", pin, part);
    }
    circuit->connections[circuit->connection_count].part = part;
    circuit->connections[circuit->connection_count].pin = i;
    circuit->connections[circuit->connection_count].net = net;
    circuit->connection_count++;
    return 0;
}

/*
 * Runs the voltage check over the circuit as it is. On a problem: the first
 * one is stored, circuit_error is its text, and the result is 1.
 */
static int voltage_failed(Circuit *circuit) {
    VoltageProblem first;
    size_t count = voltage_check((const Part *const *)circuit->parts, circuit->part_count,
                                 circuit->net_count, circuit->connections,
                                 circuit->connection_count, circuit->closed,
                                 circuit->closed_count, &first, 1);

    if (count == 0) {
        return 0;
    }
    circuit->problem = first;
    circuit->has_problem = 1;
    voltage_describe((const Part *const *)circuit->parts, &first, circuit->error,
                     sizeof circuit->error);
    circuit->has_error = 1;
    return 1;
}

/* Adds or removes a closed contact; 0, or -1 when out of memory. */
static int record_contact(Circuit *circuit, size_t part, unsigned position, int on) {
    size_t i;

    for (i = 0; i < circuit->closed_count; i++) {
        if (circuit->closed[i].part == part && circuit->closed[i].position == position) {
            break;
        }
    }
    if (!on) {
        if (i < circuit->closed_count) {
            circuit->closed[i] = circuit->closed[--circuit->closed_count];
        }
        return 0;
    }
    if (i < circuit->closed_count) {
        return 0;
    }
    if (grow((void **)&circuit->closed, &circuit->closed_capacity, circuit->closed_count,
             sizeof(*circuit->closed)) != 0) {
        return -1;
    }
    circuit->closed[circuit->closed_count].part = part;
    circuit->closed[circuit->closed_count].position = position;
    circuit->closed_count++;
    return 0;
}

int circuit_set_switch(Circuit *circuit, size_t part, unsigned position, int on) {
    SimInstance *instance;
    const SimComponentType *type;

    if (check_part(circuit, part) != 0) {
        return -1;
    }
    instance = circuit->instances[part];
    type = sim_instance_type(instance);
    if (type->set_switch == NULL) {
        return fail(circuit, "part %zu is not a switch", part);
    }
    if (!type->set_switch(circuit->sim, instance, sim_instance_state(instance), position, on)) {
        return fail(circuit, "%s has no position %u", circuit->parts[part]->name, position);
    }
    if (record_contact(circuit, part, position, on) != 0) {
        return fail(circuit, "out of memory");
    }
    /* A contact closing during a run that fails the check stops the run, like
       a short; the contact itself is still applied. */
    if (circuit->started && on && !circuit->voltage_stopped && voltage_failed(circuit)) {
        circuit->voltage_stopped = 1;
    }
    return 0;
}

int circuit_start(Circuit *circuit) {
    if (check_building(circuit) != 0) {
        return -1;
    }
    circuit->has_problem = 0;
    if (voltage_failed(circuit)) {
        return -1;
    }
    circuit->started = 1;
    if (!sim_start(circuit->sim)) {
        engine_failed(circuit);
        return -1;
    }
    return 0;
}

int circuit_advance(Circuit *circuit, uint64_t picoseconds) {
    SimTime now;

    if (!circuit->started) {
        fail(circuit, "circuit not started");
        return 0;
    }
    if (circuit->voltage_stopped) {
        return 0;
    }
    if (sim_error(circuit->sim) != NULL) {
        engine_failed(circuit);
        return 0;
    }
    now = sim_now(circuit->sim);
    if (picoseconds > UINT64_MAX - now) {
        fail(circuit, "time overflow");
        return 0;
    }
    circuit->step_start = now;
    if (!sim_run_until(circuit->sim, now + picoseconds)) {
        engine_failed(circuit);
        return 0;
    }
    return 1;
}

int circuit_voltage_problem(const Circuit *circuit, VoltageProblem *out) {
    if (circuit == NULL || !circuit->has_problem) {
        return 0;
    }
    if (out != NULL) {
        *out = circuit->problem;
    }
    return 1;
}

uint64_t circuit_time(const Circuit *circuit) {
    return sim_now(circuit->sim);
}

CircuitValue circuit_net_value(const Circuit *circuit, size_t net) {
    if (net >= circuit->net_count) {
        return CIRCUIT_X;
    }
    switch (sim_net_value(circuit->nets[net])) {
        case LOGIC_0: return CIRCUIT_0;
        case LOGIC_1: return CIRCUIT_1;
        case LOGIC_X: return CIRCUIT_X;
        case LOGIC_Z: return CIRCUIT_Z;
    }
    return CIRCUIT_X;
}

uint64_t circuit_net_changes(const Circuit *circuit, size_t net) {
    if (net >= circuit->net_count) {
        return 0;
    }
    return sim_net_changes(circuit->nets[net]);
}

int circuit_shorted_net(const Circuit *circuit, size_t *net) {
    const SimNet *shorted = sim_shorted_net(circuit->sim);
    size_t i;

    if (shorted == NULL) {
        return 0;
    }
    for (i = 0; i < circuit->net_count; i++) {
        if (circuit->nets[i] == shorted) {
            if (net != NULL) {
                *net = i;
            }
            return 1;
        }
    }
    return 0;
}

size_t circuit_part_lights(const Circuit *circuit, size_t part, unsigned char *on, size_t cap) {
    SimInstance *instance;
    const SimComponentType *type;

    if (part >= circuit->part_count) {
        return 0;
    }
    instance = circuit->instances[part];
    type = sim_instance_type(instance);
    if (type->lights == NULL) {
        return 0;
    }
    return type->lights(circuit->sim, instance, sim_instance_state(instance),
                        circuit->step_start, on, cap);
}

size_t circuit_part_light_time(const Circuit *circuit, size_t part, uint64_t *on_ps, size_t cap) {
    SimInstance *instance;
    const SimComponentType *type;

    if (part >= circuit->part_count) {
        return 0;
    }
    instance = circuit->instances[part];
    type = sim_instance_type(instance);
    if (type->light_time == NULL) {
        return 0;
    }
    return type->light_time(circuit->sim, instance, sim_instance_state(instance), on_ps, cap);
}

int circuit_memory_size(const Circuit *circuit, size_t part, uint32_t *words, unsigned *bits) {
    SimInstance *instance;
    const SimComponentType *type;

    if (part >= circuit->part_count) {
        return 0;
    }
    instance = circuit->instances[part];
    type = sim_instance_type(instance);
    return type->memory_size != NULL ? type->memory_size(sim_instance_state(instance), words, bits)
                                     : 0;
}

int circuit_memory_load(Circuit *circuit, size_t part, uint32_t start,
                        const uint32_t *values, size_t count) {
    SimInstance *instance;
    const SimComponentType *type;
    void *state;
    uint32_t words = 0;
    unsigned bits = 0;
    size_t i;

    if (check_building(circuit) != 0 || check_part(circuit, part) != 0) {
        return -1;
    }
    instance = circuit->instances[part];
    type = sim_instance_type(instance);
    state = sim_instance_state(instance);
    if (type->memory_load == NULL || type->memory_size == NULL ||
        !type->memory_size(state, &words, &bits)) {
        return fail(circuit, "part %zu (%s) has no loadable memory", part, circuit->parts[part]->name);
    }
    if (count > words || start > words - count) {
        return fail(circuit, "memory load of %zu words at %u is past the end of %s (%u words)", count,
                    (unsigned)start, circuit->parts[part]->name, (unsigned)words);
    }
    for (i = 0; i < count; i++) {
        if (bits < 32 && (values[i] >> bits) != 0) {
            return fail(circuit, "word %u: 0x%X does not fit %u bits", (unsigned)(start + i),
                        (unsigned)values[i], bits);
        }
    }
    if (count > 0) {
        type->memory_load(state, start, values, count);
    }
    return 0;
}

size_t circuit_memory_read(const Circuit *circuit, size_t part, uint32_t start, size_t count,
                           uint32_t *values, uint32_t *known) {
    SimInstance *instance;
    const SimComponentType *type;

    if (part >= circuit->part_count) {
        return 0;
    }
    instance = circuit->instances[part];
    type = sim_instance_type(instance);
    return type->memory_read != NULL
        ? type->memory_read(sim_instance_state(instance), start, count, values, known)
        : 0;
}
