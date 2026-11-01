#include "sim.h"

#include "net.h"

#include <stdlib.h>
#include <string.h>

#define SIM_MAX_ROUNDS 10000

typedef struct {
    SimNet *net;
    size_t net_pin_index;
} InstancePin;

struct SimInstance {
    Sim *owner;
    const SimComponentType *type;
    char *name;
    void *state;
    InstancePin *pins;
    size_t pin_count;
};

typedef enum {
    EVENT_DRIVE,
    EVENT_TIMER
} EventType;

typedef struct {
    EventType type;
    SimTime time;
    SimInstance *instance;
    size_t pin;
    LogicValue value;
    DriveStrength strength;
    uint32_t timer_id;
} Event;

struct Sim {
    SimTime now;
    int started;
    const char *error;

    SimInstance **instances;
    size_t instance_count;
    size_t instance_capacity;

    SimNet **nets;
    size_t net_count;
    size_t net_capacity;

    Event *events;
    size_t event_count;
    size_t event_capacity;

    SimTime round_time;
    size_t rounds_at_time;

    const SimNet *shorted_net;
};

static void set_error(Sim *sim, const char *message) {
    if (sim != NULL && sim->error == NULL) {
        sim->error = message;
    }
}

static char *copy_string(const char *text) {
    size_t length;
    char *copy;

    if (text == NULL) {
        text = "";
    }

    length = strlen(text) + 1;
    copy = malloc(length);

    if (copy != NULL) {
        memcpy(copy, text, length);
    }

    return copy;
}

static void *grow_array(void *array, size_t *capacity, size_t element_size) {
    size_t new_capacity;
    void *new_array;

    new_capacity = *capacity == 0 ? 8 : *capacity * 2;

    if (new_capacity < *capacity ||
        new_capacity > SIZE_MAX / element_size) {
        return NULL;
    }

    new_array = realloc(array, new_capacity * element_size);
    if (new_array == NULL) {
        return NULL;
    }

    *capacity = new_capacity;
    return new_array;
}

Sim *sim_create(void) {
    return calloc(1, sizeof(Sim));
}

void sim_destroy(Sim *sim) {
    size_t i;

    if (sim == NULL) {
        return;
    }

    for (i = 0; i < sim->instance_count; i++) {
        SimInstance *instance = sim->instances[i];

        if (instance->type->destroy != NULL) {
            instance->type->destroy(instance->state);
        }

        free(instance->pins);
        free(instance->name);
        free(instance);
    }

    for (i = 0; i < sim->net_count; i++) {
        net_destroy(sim->nets[i]);
    }

    free(sim->events);
    free(sim->nets);
    free(sim->instances);
    free(sim);
}

const char *sim_error(const Sim *sim) {
    return sim != NULL ? sim->error : "simulation is NULL";
}

const SimNet *sim_shorted_net(const Sim *sim) {
    return sim != NULL ? sim->shorted_net : NULL;
}

SimTime sim_now(const Sim *sim) {
    return sim != NULL ? sim->now : 0;
}

SimInstance *sim_add_instance(
    Sim *sim,
    const SimComponentType *type,
    const char *name,
    const void *config
) {
    SimInstance *instance;
    size_t pin_count;

    if (sim == NULL || sim->error != NULL) {
        return NULL;
    }

    if (sim->started || type == NULL || type->pin_count == NULL) {
        set_error(sim, "invalid instance creation");
        return NULL;
    }

    pin_count = type->pin_count(config);

    if (sim->instance_count == sim->instance_capacity) {
        SimInstance **new_instances = grow_array(
            sim->instances,
            &sim->instance_capacity,
            sizeof(*sim->instances)
        );

        if (new_instances == NULL) {
            set_error(sim, "out of memory");
            return NULL;
        }

        sim->instances = new_instances;
    }

    instance = calloc(1, sizeof(*instance));
    if (instance == NULL) {
        set_error(sim, "out of memory");
        return NULL;
    }

    instance->owner = sim;
    instance->type = type;
    instance->pin_count = pin_count;
    instance->name = copy_string(name);

    if (pin_count > 0) {
        instance->pins = calloc(pin_count, sizeof(*instance->pins));
    }

    if (instance->name == NULL ||
        (pin_count > 0 && instance->pins == NULL)) {
        free(instance->pins);
        free(instance->name);
        free(instance);
        set_error(sim, "out of memory");
        return NULL;
    }

    if (type->create != NULL) {
        instance->state = type->create(config);

        if (instance->state == NULL) {
            free(instance->pins);
            free(instance->name);
            free(instance);
            set_error(sim, "component state creation failed");
            return NULL;
        }
    }

    sim->instances[sim->instance_count++] = instance;
    return instance;
}

SimNet *sim_add_net(Sim *sim, const char *name) {
    SimNet *net;

    if (sim == NULL || sim->error != NULL) {
        return NULL;
    }

    if (sim->started) {
        set_error(sim, "cannot add a net after starting");
        return NULL;
    }

    if (sim->net_count == sim->net_capacity) {
        SimNet **new_nets = grow_array(
            sim->nets,
            &sim->net_capacity,
            sizeof(*sim->nets)
        );

        if (new_nets == NULL) {
            set_error(sim, "out of memory");
            return NULL;
        }

        sim->nets = new_nets;
    }

    net = net_create(sim, name);
    if (net == NULL) {
        set_error(sim, "out of memory");
        return NULL;
    }

    sim->nets[sim->net_count++] = net;
    return net;
}

int sim_connect(
    Sim *sim,
    SimNet *net,
    SimInstance *instance,
    size_t pin
) {
    InstancePin *instance_pin;

    if (sim == NULL || sim->error != NULL) {
        return 0;
    }

    if (sim->started || net == NULL || instance == NULL ||
        net->owner != sim || instance->owner != sim ||
        pin >= instance->pin_count) {
        set_error(sim, "invalid connection");
        return 0;
    }

    instance_pin = &instance->pins[pin];

    if (instance_pin->net != NULL) {
        set_error(sim, "pin is already connected");
        return 0;
    }

    if (!net_attach(net, instance, pin)) {
        set_error(sim, "out of memory");
        return 0;
    }

    instance_pin->net = net;
    instance_pin->net_pin_index = net->pin_count - 1;
    return 1;
}

LogicValue sim_read_pin(
    const Sim *sim,
    const SimInstance *instance,
    size_t pin
) {
    SimNet *net;

    if (sim == NULL || instance == NULL || instance->owner != sim ||
        pin >= instance->pin_count) {
        return LOGIC_X;
    }

    net = instance->pins[pin].net;
    return net != NULL ? net->value : LOGIC_Z;
}

static int append_event(Sim *sim, const Event *event) {
    if (sim->event_count == sim->event_capacity) {
        Event *new_events = grow_array(
            sim->events,
            &sim->event_capacity,
            sizeof(*sim->events)
        );

        if (new_events == NULL) {
            set_error(sim, "out of memory");
            return 0;
        }

        sim->events = new_events;
    }

    sim->events[sim->event_count++] = *event;
    return 1;
}

static void schedule_drive(
    Sim *sim,
    SimInstance *instance,
    size_t pin,
    LogicValue value,
    DriveStrength strength,
    SimTime delay,
    int inertial
) {
    Event event = {0};
    size_t i;
    size_t kept = 0;

    if (sim == NULL || sim->error != NULL) {
        return;
    }

    if (instance == NULL || instance->owner != sim ||
        pin >= instance->pin_count ||
        value < LOGIC_0 || value > LOGIC_Z ||
        strength < DRIVE_WEAK || strength > DRIVE_STRONG) {
        set_error(sim, "invalid drive event");
        return;
    }

    if (delay > UINT64_MAX - sim->now) {
        set_error(sim, "event time overflow");
        return;
    }

    /*
     * Inertial scheduling: a newer drive on this pin replaces any drive that
     * has not matured yet, so a pulse shorter than the delay is swallowed.
     */
    if (inertial) {
        for (i = 0; i < sim->event_count; i++) {
            const Event *pending = &sim->events[i];

            if (pending->type == EVENT_DRIVE &&
                pending->instance == instance && pending->pin == pin) {
                continue;
            }

            sim->events[kept++] = *pending;
        }

        sim->event_count = kept;
    }

    event.type = EVENT_DRIVE;
    event.time = sim->now + delay;
    event.instance = instance;
    event.pin = pin;
    event.value = value;
    event.strength = strength;

    append_event(sim, &event);
}

/* Transport delay: every scheduled edge is kept. Used for test sources. */
void sim_schedule_drive(
    Sim *sim,
    SimInstance *instance,
    size_t pin,
    LogicValue value,
    DriveStrength strength,
    SimTime delay
) {
    schedule_drive(sim, instance, pin, value, strength, delay, 0);
}

/* Inertial delay: a newer drive cancels a pending drive on the same pin. */
void sim_schedule_inertial_drive(
    Sim *sim,
    SimInstance *instance,
    size_t pin,
    LogicValue value,
    DriveStrength strength,
    SimTime delay
) {
    schedule_drive(sim, instance, pin, value, strength, delay, 1);
}

void sim_schedule_timer(
    Sim *sim,
    SimInstance *instance,
    uint32_t timer_id,
    SimTime delay
) {
    Event event = {0};

    if (sim == NULL || sim->error != NULL) {
        return;
    }

    if (instance == NULL || instance->owner != sim) {
        set_error(sim, "invalid timer event");
        return;
    }

    if (delay > UINT64_MAX - sim->now) {
        set_error(sim, "event time overflow");
        return;
    }

    event.type = EVENT_TIMER;
    event.time = sim->now + delay;
    event.instance = instance;
    event.timer_id = timer_id;

    append_event(sim, &event);
}

int sim_start(Sim *sim) {
    size_t i;

    if (sim == NULL || sim->error != NULL) {
        return 0;
    }

    if (sim->started) {
        set_error(sim, "simulation already started");
        return 0;
    }

    sim->started = 1;

    for (i = 0; i < sim->net_count; i++) {
        net_resolve(sim->nets[i]);
    }

    for (i = 0; i < sim->instance_count && sim->error == NULL; i++) {
        SimInstance *instance = sim->instances[i];

        if (instance->type->start != NULL) {
            instance->type->start(sim, instance, instance->state);
        }
    }

    return sim->error == NULL;
}

static void apply_drive(const Event *event) {
    InstancePin *pin = &event->instance->pins[event->pin];

    if (pin->net != NULL) {
        SimNetPin *net_pin = &pin->net->pins[pin->net_pin_index];
        net_pin->drive = event->value;
        net_pin->strength = event->strength;
    }
}

static int process_round(Sim *sim, SimTime time) {
    Event *batch;
    LogicValue *old_values = NULL;
    size_t batch_count = 0;
    size_t remaining_count = 0;
    size_t i;

    batch = malloc(sim->event_count * sizeof(*batch));
    if (batch == NULL) {
        set_error(sim, "out of memory");
        return 0;
    }

    if (sim->net_count > 0) {
        old_values = malloc(sim->net_count * sizeof(*old_values));

        if (old_values == NULL) {
            free(batch);
            set_error(sim, "out of memory");
            return 0;
        }
    }

    for (i = 0; i < sim->event_count; i++) {
        if (sim->events[i].time == time) {
            batch[batch_count++] = sim->events[i];
        } else {
            sim->events[remaining_count++] = sim->events[i];
        }
    }

    sim->event_count = remaining_count;

    for (i = 0; i < sim->net_count; i++) {
        old_values[i] = sim->nets[i]->value;
    }

    for (i = 0; i < batch_count; i++) {
        if (batch[i].type == EVENT_DRIVE) {
            apply_drive(&batch[i]);
        }
    }

    for (i = 0; i < sim->net_count; i++) {
        net_resolve(sim->nets[i]);
    }

    /* Two strong drivers fighting (e.g. VCC tied to GND) burns the circuit. */
    for (i = 0; i < sim->net_count && sim->error == NULL; i++) {
        if (sim->nets[i]->shorted) {
            sim->shorted_net = sim->nets[i];
            set_error(sim, "smoke: short circuit, strong drivers disagree on a net");
        }
    }

    for (i = 0; i < sim->net_count && sim->error == NULL; i++) {
        SimNet *net = sim->nets[i];
        size_t j;

        if (old_values[i] == net->value) {
            continue;
        }

        for (j = 0; j < net->pin_count && sim->error == NULL; j++) {
            SimNetPin *pin = &net->pins[j];
            SimInstance *instance = pin->instance;

            if (instance->type->pin_changed != NULL) {
                instance->type->pin_changed(
                    sim,
                    instance,
                    instance->state,
                    pin->pin,
                    old_values[i],
                    net->value
                );
            }
        }
    }

    for (i = 0; i < batch_count && sim->error == NULL; i++) {
        SimInstance *instance = batch[i].instance;

        if (batch[i].type == EVENT_TIMER &&
            instance->type->timer != NULL) {
            instance->type->timer(
                sim,
                instance,
                instance->state,
                batch[i].timer_id
            );
        }
    }

    free(old_values);
    free(batch);
    return sim->error == NULL;
}

int sim_run_until(Sim *sim, SimTime time) {
    if (sim == NULL || sim->error != NULL) {
        return 0;
    }

    if (!sim->started || time < sim->now) {
        set_error(sim, "invalid run time");
        return 0;
    }

    while (sim->event_count > 0) {
        SimTime next_time = sim->events[0].time;
        size_t i;

        for (i = 1; i < sim->event_count; i++) {
            if (sim->events[i].time < next_time) {
                next_time = sim->events[i].time;
            }
        }

        if (next_time > time) {
            break;
        }

        if (sim->rounds_at_time == 0 || sim->round_time != next_time) {
            sim->round_time = next_time;
            sim->rounds_at_time = 0;
        }

        sim->rounds_at_time++;

        if (sim->rounds_at_time > SIM_MAX_ROUNDS) {
            set_error(sim, "circuit did not settle");
            return 0;
        }

        sim->now = next_time;

        if (!process_round(sim, next_time)) {
            return 0;
        }
    }

    sim->now = time;
    return 1;
}
