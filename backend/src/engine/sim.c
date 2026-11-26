#include "sim_int.h"

void simc_set_error(Sim *sim, const char *message) {
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

void *simc_grow_array(void *array, size_t *capacity, size_t element_size) {
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

    free(sim->contacts);
    free(sim->diodes);
    free(sim->cluster_parent);
    free(sim->cluster_root);
    free(sim->cluster_head);
    free(sim->cluster_next);
    free(sim->link_head);
    free(sim->link_next);
    free(sim->node_strong);
    free(sim->node_weak);
    free(sim->events);
    free(sim->batch);
    free(sim->dirty);
    free(sim->is_dirty);
    free(sim->old_values);
    free(sim->group_parent);
    free(sim->group_root);
    free(sim->group_head);
    free(sim->group_next);
    free(sim->group_nets);
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
        simc_set_error(sim, "invalid instance creation");
        return NULL;
    }

    pin_count = type->pin_count(config);

    if (sim->instance_count == sim->instance_capacity) {
        SimInstance **new_instances = simc_grow_array(
            sim->instances,
            &sim->instance_capacity,
            sizeof(*sim->instances)
        );

        if (new_instances == NULL) {
            simc_set_error(sim, "out of memory");
            return NULL;
        }

        sim->instances = new_instances;
    }

    instance = calloc(1, sizeof(*instance));
    if (instance == NULL) {
        simc_set_error(sim, "out of memory");
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
        simc_set_error(sim, "out of memory");
        return NULL;
    }

    if (type->create != NULL) {
        instance->state = type->create(config);

        if (instance->state == NULL) {
            free(instance->pins);
            free(instance->name);
            free(instance);
            simc_set_error(sim, "component state creation failed");
            return NULL;
        }
    }

    sim->instances[sim->instance_count++] = instance;
    return instance;
}

const SimComponentType *sim_instance_type(const SimInstance *instance) {
    return instance != NULL ? instance->type : NULL;
}

void *sim_instance_state(const SimInstance *instance) {
    return instance != NULL ? instance->state : NULL;
}

SimNet *sim_add_net(Sim *sim, const char *name) {
    SimNet *net;

    if (sim == NULL || sim->error != NULL) {
        return NULL;
    }

    if (sim->started) {
        simc_set_error(sim, "cannot add a net after starting");
        return NULL;
    }

    if (sim->net_count == sim->net_capacity) {
        SimNet **new_nets = simc_grow_array(
            sim->nets,
            &sim->net_capacity,
            sizeof(*sim->nets)
        );

        if (new_nets == NULL) {
            simc_set_error(sim, "out of memory");
            return NULL;
        }

        sim->nets = new_nets;
    }

    net = net_create(sim, name);
    if (net == NULL) {
        simc_set_error(sim, "out of memory");
        return NULL;
    }
    net->index = sim->net_count;

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
        simc_set_error(sim, "invalid connection");
        return 0;
    }

    instance_pin = &instance->pins[pin];

    if (instance_pin->net != NULL) {
        simc_set_error(sim, "pin is already connected");
        return 0;
    }

    if (!net_attach(net, instance, pin)) {
        simc_set_error(sim, "out of memory");
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

static int event_less(const Event *a, const Event *b) {
    return a->time < b->time || (a->time == b->time && a->seq < b->seq);
}

/* Adds an event to the heap, stamped with the next insertion number. */
int simc_append_event(Sim *sim, const Event *event) {
    size_t child;

    if (sim->event_count == sim->event_capacity) {
        Event *new_events = simc_grow_array(
            sim->events,
            &sim->event_capacity,
            sizeof(*sim->events)
        );

        if (new_events == NULL) {
            simc_set_error(sim, "out of memory");
            return 0;
        }

        sim->events = new_events;
    }

    child = sim->event_count++;
    sim->events[child] = *event;
    sim->events[child].seq = sim->next_seq++;

    while (child > 0) {
        size_t parent = (child - 1) / 2;
        Event swap;

        if (!event_less(&sim->events[child], &sim->events[parent])) {
            break;
        }
        swap = sim->events[parent];
        sim->events[parent] = sim->events[child];
        sim->events[child] = swap;
        child = parent;
    }
    return 1;
}

/* Removes the heap's earliest event into *out. The heap must not be empty. */
void simc_heap_pop(Sim *sim, Event *out) {
    size_t parent = 0;

    *out = sim->events[0];
    sim->event_count--;
    if (sim->event_count == 0) {
        return;
    }
    sim->events[0] = sim->events[sim->event_count];

    for (;;) {
        size_t left = 2 * parent + 1;
        size_t smallest = parent;
        Event swap;

        if (left < sim->event_count &&
            event_less(&sim->events[left], &sim->events[smallest])) {
            smallest = left;
        }
        if (left + 1 < sim->event_count &&
            event_less(&sim->events[left + 1], &sim->events[smallest])) {
            smallest = left + 1;
        }
        if (smallest == parent) {
            break;
        }
        swap = sim->events[parent];
        sim->events[parent] = sim->events[smallest];
        sim->events[smallest] = swap;
        parent = smallest;
    }
}

/* A drive cancelled by a later inertial drive on the same pin. */
static int is_stale(const Event *event) {
    return event->type == EVENT_DRIVE &&
           event->gen != event->instance->pins[event->pin].gen;
}

/* Drops cancelled events off the top of the heap. */
void simc_drop_stale(Sim *sim) {
    Event dropped;

    while (sim->event_count > 0 && is_stale(&sim->events[0])) {
        simc_heap_pop(sim, &dropped);
    }
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

    if (sim == NULL || sim->error != NULL) {
        return;
    }

    if (instance == NULL || instance->owner != sim ||
        pin >= instance->pin_count ||
        value < LOGIC_0 || value > LOGIC_Z ||
        strength < DRIVE_WEAK || strength > DRIVE_STRONG) {
        simc_set_error(sim, "invalid drive event");
        return;
    }

    if (delay > UINT64_MAX - sim->now) {
        simc_set_error(sim, "event time overflow");
        return;
    }

    /*
     * Inertial scheduling: a newer drive on this pin replaces any drive that
     * has not matured yet, so a pulse shorter than the delay is swallowed.
     * Bumping the pin's gen makes every drive scheduled on it before now
     * stale; stale events are dropped when they reach the top of the heap.
     * A drive of the value already pending, no earlier than it, keeps the
     * pending edge: the output switches when it first would have.
     */
    if (inertial) {
        InstancePin *p = &instance->pins[pin];

        if (p->has_pending && p->pending_value == value &&
            p->pending_strength == strength &&
            p->pending_time <= sim->now + delay) {
            return;
        }
        p->gen++;
        p->has_pending = 1;
        p->pending_time = sim->now + delay;
        p->pending_value = value;
        p->pending_strength = strength;
    } else {
        instance->pins[pin].has_pending = 0;
    }

    event.type = EVENT_DRIVE;
    event.time = sim->now + delay;
    event.instance = instance;
    event.pin = pin;
    event.value = value;
    event.strength = strength;
    event.gen = instance->pins[pin].gen;

    simc_append_event(sim, &event);
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
        simc_set_error(sim, "invalid timer event");
        return;
    }

    if (delay > UINT64_MAX - sim->now) {
        simc_set_error(sim, "event time overflow");
        return;
    }

    event.type = EVENT_TIMER;
    event.time = sim->now + delay;
    event.instance = instance;
    event.timer_id = timer_id;

    simc_append_event(sim, &event);
}

