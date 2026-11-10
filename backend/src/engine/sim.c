#include "sim.h"

#include "net.h"

#include <stdlib.h>
#include <string.h>

#define SIM_MAX_ROUNDS 10000

typedef struct {
    SimNet *net;
    size_t net_pin_index;
    uint32_t gen;      /* bumped by each inertial drive on this pin */
    /* The inertial drive pending on this pin, if any. */
    int has_pending;
    SimTime pending_time;
    LogicValue pending_value;
    DriveStrength pending_strength;
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
    EVENT_TIMER,
    EVENT_CONTACT
} EventType;

typedef struct {
    EventType type;
    SimTime time;
    SimInstance *instance;
    size_t pin;
    LogicValue value;
    DriveStrength strength;
    uint32_t timer_id;
    int closed;        /* EVENT_CONTACT: new state of the contact (pin, pin_b) */
    size_t pin_b;
    uint64_t seq;      /* insertion order; breaks ties between equal times */
    uint32_t gen;      /* EVENT_DRIVE: the pin's gen when scheduled */
} Event;

/* A one-way link (diode) between two nets, collected by sim_start. */
typedef struct {
    SimNet *anode;
    SimNet *cathode;
} Diode;

/* A contact between two pins of one instance. Pins are stored lowest first. */
typedef struct {
    SimInstance *instance;
    size_t pin_a;
    size_t pin_b;
    int closed;
} Contact;

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

    /* Pending events: a binary min-heap ordered by (time, seq). */
    Event *events;
    size_t event_count;
    size_t event_capacity;
    uint64_t next_seq;

    /* One round's events, reused across rounds. */
    Event *batch;
    size_t batch_capacity;

    /* Per-net round state, allocated in sim_start (net_count entries). */
    size_t *dirty;              /* net indexes touched this round */
    size_t dirty_count;
    unsigned char *is_dirty;
    LogicValue *old_values;     /* valid while a net is dirty */

    /* Cached contact groups (build_groups), per net. */
    size_t *group_parent;
    size_t *group_root;
    size_t *group_head;
    size_t *group_next;
    SimNet **group_nets;        /* scratch for net_resolve_group */
    int groups_valid;

    SimTime round_time;
    size_t rounds_at_time;

    const SimNet *shorted_net;

    Contact *contacts;
    size_t contact_count;
    size_t contact_capacity;
    size_t closed_contact_count;

    /* Diode links (plan-29-H), fixed at sim_start. */
    Diode *diodes;
    size_t diode_count;
    size_t diode_capacity;

    /*
     * Clusters: contact groups joined by diode links, per net (allocated
     * only when diode_count > 0, rebuilt with the groups). cluster_root[i]
     * is net i's cluster root; members chain from cluster_head[root] through
     * cluster_next; the cluster's links chain from link_head[root] through
     * link_next (per diode). node_strong / node_weak are resolve_cluster's
     * driver masks, per group root.
     */
    size_t *cluster_parent;
    size_t *cluster_root;
    size_t *cluster_head;
    size_t *cluster_next;
    size_t *link_head;
    size_t *link_next;
    unsigned char *node_strong;
    unsigned char *node_weak;
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

static int event_less(const Event *a, const Event *b) {
    return a->time < b->time || (a->time == b->time && a->seq < b->seq);
}

/* Adds an event to the heap, stamped with the next insertion number. */
static int append_event(Sim *sim, const Event *event) {
    size_t child;

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
static void heap_pop(Sim *sim, Event *out) {
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
static void drop_stale(Sim *sim) {
    Event dropped;

    while (sim->event_count > 0 && is_stale(&sim->events[0])) {
        heap_pop(sim, &dropped);
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

/* ---- Contacts ----------------------------------------------------------- */

static Contact *find_contact(const Sim *sim, const SimInstance *instance,
                             size_t pin_a, size_t pin_b) {
    size_t i;

    for (i = 0; i < sim->contact_count; i++) {
        Contact *contact = &sim->contacts[i];

        if (contact->instance == instance &&
            contact->pin_a == pin_a && contact->pin_b == pin_b) {
            return contact;
        }
    }
    return NULL;
}

/* Sets a contact's state, adding the contact the first time it is used. */
static void apply_contact(Sim *sim, SimInstance *instance,
                          size_t pin_a, size_t pin_b, int closed) {
    Contact *contact = find_contact(sim, instance, pin_a, pin_b);

    if (contact == NULL) {
        if (!closed) {
            return; /* contacts start open */
        }
        if (sim->contact_count == sim->contact_capacity) {
            Contact *grown = grow_array(sim->contacts, &sim->contact_capacity,
                                        sizeof(*sim->contacts));

            if (grown == NULL) {
                set_error(sim, "out of memory");
                return;
            }
            sim->contacts = grown;
        }
        contact = &sim->contacts[sim->contact_count++];
        contact->instance = instance;
        contact->pin_a = pin_a;
        contact->pin_b = pin_b;
        contact->closed = 0;
    }

    if (contact->closed != (closed != 0)) {
        contact->closed = closed != 0;
        if (contact->closed) {
            sim->closed_contact_count++;
        } else {
            sim->closed_contact_count--;
        }
    }
}

void sim_set_contact(
    Sim *sim,
    SimInstance *instance,
    size_t pin_a,
    size_t pin_b,
    int closed
) {
    Event event = {0};

    if (sim == NULL || sim->error != NULL) {
        return;
    }

    if (instance == NULL || instance->owner != sim ||
        pin_a >= instance->pin_count || pin_b >= instance->pin_count ||
        pin_a == pin_b) {
        set_error(sim, "invalid contact");
        return;
    }

    if (pin_a > pin_b) {
        size_t swap = pin_a;
        pin_a = pin_b;
        pin_b = swap;
    }

    if (!sim->started) {
        apply_contact(sim, instance, pin_a, pin_b, closed);
        return;
    }

    event.type = EVENT_CONTACT;
    event.time = sim->now;
    event.instance = instance;
    event.pin = pin_a;
    event.pin_b = pin_b;
    event.closed = closed != 0;
    append_event(sim, &event);
}

int sim_contact_closed(
    const Sim *sim,
    const SimInstance *instance,
    size_t pin_a,
    size_t pin_b
) {
    const Contact *contact;

    if (sim == NULL) {
        return 0;
    }
    if (pin_a > pin_b) {
        size_t swap = pin_a;
        pin_a = pin_b;
        pin_b = swap;
    }
    contact = find_contact(sim, instance, pin_a, pin_b);
    return contact != NULL && contact->closed;
}

static size_t find_root(size_t *parent, size_t index) {
    while (parent[index] != index) {
        parent[index] = parent[parent[index]];
        index = parent[index];
    }
    return index;
}

/*
 * Joins the contact groups (group_root, already built) by every diode link
 * into clusters: cluster_root per net, member chains, and per cluster the
 * chain of its links, each in ascending order.
 */
static void build_clusters(Sim *sim) {
    size_t *parent = sim->cluster_parent;
    size_t none = sim->net_count;
    size_t i;

    for (i = 0; i < sim->net_count; i++) {
        parent[i] = sim->group_root[i];
    }
    for (i = 0; i < sim->diode_count; i++) {
        size_t root_a = find_root(parent, sim->diodes[i].anode->index);
        size_t root_c = find_root(parent, sim->diodes[i].cathode->index);

        if (root_a != root_c) {
            parent[root_a] = root_c;
        }
    }
    for (i = 0; i < sim->net_count; i++) {
        sim->cluster_root[i] = find_root(parent, i);
        sim->cluster_head[i] = none;
        sim->link_head[i] = sim->diode_count;
    }
    for (i = sim->net_count; i-- > 0;) {
        size_t root = sim->cluster_root[i];

        sim->cluster_next[i] = sim->cluster_head[root];
        sim->cluster_head[root] = i;
    }
    for (i = sim->diode_count; i-- > 0;) {
        size_t root = sim->cluster_root[sim->diodes[i].anode->index];

        sim->link_next[i] = sim->link_head[root];
        sim->link_head[root] = i;
    }
}

/*
 * Groups nets joined by closed contacts (union-find) into the cached arrays:
 * group_root[i] is net i's group root, and each root's members are chained
 * from group_head[root] through group_next, in ascending net index.
 */
static void build_groups(Sim *sim) {
    size_t *parent = sim->group_parent;
    size_t none = sim->net_count;
    size_t i;

    for (i = 0; i < sim->net_count; i++) {
        parent[i] = i;
    }

    for (i = 0; i < sim->contact_count; i++) {
        const Contact *contact = &sim->contacts[i];
        const SimNet *a;
        const SimNet *b;
        size_t root_a;
        size_t root_b;

        if (!contact->closed) {
            continue;
        }
        a = contact->instance->pins[contact->pin_a].net;
        b = contact->instance->pins[contact->pin_b].net;
        if (a == NULL || b == NULL) {
            continue;
        }
        root_a = find_root(parent, a->index);
        root_b = find_root(parent, b->index);
        if (root_a != root_b) {
            parent[root_a] = root_b;
        }
    }

    for (i = 0; i < sim->net_count; i++) {
        sim->group_root[i] = find_root(parent, i);
        sim->group_head[i] = none;
    }
    for (i = sim->net_count; i-- > 0;) {
        size_t root = sim->group_root[i];

        sim->group_next[i] = sim->group_head[root];
        sim->group_head[root] = i;
    }
    if (sim->diode_count > 0) {
        build_clusters(sim);
    }
    sim->groups_valid = 1;
}

/* Resolves the group whose root is `root` as one net. */
static void resolve_group(Sim *sim, size_t root) {
    size_t none = sim->net_count;
    size_t count = 0;
    size_t j;

    for (j = sim->group_head[root]; j != none; j = sim->group_next[j]) {
        sim->group_nets[count++] = sim->nets[j];
    }
    net_resolve_group(sim->group_nets, count);
}

/*
 * Passes values along one link's masks: a 1 or X from the anode node to the
 * cathode node, a 0 or X back. Returns whether either mask grew.
 */
static int pass_link(unsigned char *anode, unsigned char *cathode) {
    unsigned char forward = (unsigned char)(*anode & (NET_MASK(LOGIC_1) | NET_MASK(LOGIC_X)));
    unsigned char back = (unsigned char)(*cathode & (NET_MASK(LOGIC_0) | NET_MASK(LOGIC_X)));
    int grew = 0;

    if ((*cathode | forward) != *cathode) {
        *cathode |= forward;
        grew = 1;
    }
    if ((*anode | back) != *anode) {
        *anode |= back;
        grew = 1;
    }
    return grew;
}

/*
 * Resolves the cluster whose root is `root` (plan-29-H section 4.3). Nodes
 * are its contact groups; each gets the masks of its own drivers, strong
 * values pass along the links until nothing changes, then weak values pass
 * between nodes no strong value reaches. A node reads its strong values if
 * any, else its weak ones; it is shorted when its strong values hold a 0 and
 * a 1. A cluster with no link is a plain group.
 */
static void resolve_cluster(Sim *sim, size_t root) {
    size_t none = sim->net_count;
    unsigned char *strong = sim->node_strong;
    unsigned char *weak = sim->node_weak;
    size_t j;
    size_t d;
    int grew;

    if (sim->link_head[root] == sim->diode_count) {
        resolve_group(sim, root);
        return;
    }

    for (j = sim->cluster_head[root]; j != none; j = sim->cluster_next[j]) {
        size_t g = sim->group_root[j];

        strong[g] = 0;
        weak[g] = 0;
    }
    for (j = sim->cluster_head[root]; j != none; j = sim->cluster_next[j]) {
        size_t g = sim->group_root[j];
        unsigned s;
        unsigned w;

        net_drivers(&sim->nets[j], 1, &s, &w);
        strong[g] |= (unsigned char)s;
        weak[g] |= (unsigned char)w;
    }

    do {
        grew = 0;
        for (d = sim->link_head[root]; d != sim->diode_count; d = sim->link_next[d]) {
            size_t a = sim->group_root[sim->diodes[d].anode->index];
            size_t c = sim->group_root[sim->diodes[d].cathode->index];

            if (a != c && pass_link(&strong[a], &strong[c])) {
                grew = 1;
            }
        }
    } while (grew);

    do {
        grew = 0;
        for (d = sim->link_head[root]; d != sim->diode_count; d = sim->link_next[d]) {
            size_t a = sim->group_root[sim->diodes[d].anode->index];
            size_t c = sim->group_root[sim->diodes[d].cathode->index];

            if (a != c && strong[a] == 0 && strong[c] == 0 &&
                pass_link(&weak[a], &weak[c])) {
                grew = 1;
            }
        }
    } while (grew);

    for (j = sim->cluster_head[root]; j != none; j = sim->cluster_next[j]) {
        size_t g = sim->group_root[j];
        SimNet *net = sim->nets[j];

        net->value = net_mask_value(strong[g] != 0 ? strong[g] : weak[g]);
        net->shorted = net_mask_shorted(strong[g]);
    }
}

/*
 * Resolves every net (at start). Without closed contacts or diodes each net
 * stands alone; with contacts, each group of joined nets is resolved as one
 * net; with diodes, each cluster by resolve_cluster.
 */
static void resolve_nets(Sim *sim) {
    size_t i;

    if (sim->closed_contact_count == 0 && sim->diode_count == 0) {
        for (i = 0; i < sim->net_count; i++) {
            net_resolve(sim->nets[i]);
        }
        return;
    }

    build_groups(sim);
    for (i = 0; i < sim->net_count; i++) {
        if (sim->diode_count > 0) {
            if (sim->cluster_root[i] == i) {
                resolve_cluster(sim, i);
            }
        } else if (sim->group_root[i] == i) {
            resolve_group(sim, i);
        }
    }
}

/* Allocates the per-net arrays a round uses. 0 (error set) on failure. */
static int alloc_round_arrays(Sim *sim) {
    size_t n = sim->net_count > 0 ? sim->net_count : 1;

    sim->dirty = malloc(n * sizeof(*sim->dirty));
    sim->is_dirty = calloc(n, sizeof(*sim->is_dirty));
    sim->old_values = malloc(n * sizeof(*sim->old_values));
    sim->group_parent = malloc(n * sizeof(*sim->group_parent));
    sim->group_root = malloc(n * sizeof(*sim->group_root));
    sim->group_head = malloc(n * sizeof(*sim->group_head));
    sim->group_next = malloc(n * sizeof(*sim->group_next));
    sim->group_nets = malloc(n * sizeof(*sim->group_nets));

    if (sim->dirty == NULL || sim->is_dirty == NULL ||
        sim->old_values == NULL || sim->group_parent == NULL ||
        sim->group_root == NULL || sim->group_head == NULL ||
        sim->group_next == NULL || sim->group_nets == NULL) {
        set_error(sim, "out of memory");
        return 0;
    }
    if (sim->diode_count > 0) {
        size_t links = sim->diode_count;

        sim->cluster_parent = malloc(n * sizeof(*sim->cluster_parent));
        sim->cluster_root = malloc(n * sizeof(*sim->cluster_root));
        sim->cluster_head = malloc(n * sizeof(*sim->cluster_head));
        sim->cluster_next = malloc(n * sizeof(*sim->cluster_next));
        sim->link_head = malloc(n * sizeof(*sim->link_head));
        sim->link_next = malloc(links * sizeof(*sim->link_next));
        sim->node_strong = malloc(n);
        sim->node_weak = malloc(n);
        if (sim->cluster_parent == NULL || sim->cluster_root == NULL ||
            sim->cluster_head == NULL || sim->cluster_next == NULL ||
            sim->link_head == NULL || sim->link_next == NULL ||
            sim->node_strong == NULL || sim->node_weak == NULL) {
            set_error(sim, "out of memory");
            return 0;
        }
    }
    return 1;
}

/*
 * Collects every instance's diode links (the `diodes` hook) whose two pins
 * are on two different nets. 0 (error set) on failure.
 */
static int collect_diodes(Sim *sim) {
    size_t i;

    for (i = 0; i < sim->instance_count; i++) {
        SimInstance *instance = sim->instances[i];
        size_t local_anode[16];
        size_t local_cathode[16];
        size_t *anode = local_anode;
        size_t *cathode = local_cathode;
        size_t count;
        size_t k;

        if (instance->type->diodes == NULL) {
            continue;
        }
        count = instance->type->diodes(instance->state, anode, cathode, 16);
        if (count > 16) {
            anode = malloc(count * sizeof(*anode));
            cathode = malloc(count * sizeof(*cathode));
            if (anode == NULL || cathode == NULL) {
                free(anode);
                free(cathode);
                set_error(sim, "out of memory");
                return 0;
            }
            count = instance->type->diodes(instance->state, anode, cathode, count);
        }
        for (k = 0; k < count; k++) {
            SimNet *a;
            SimNet *c;

            if (anode[k] >= instance->pin_count || cathode[k] >= instance->pin_count) {
                continue;
            }
            a = instance->pins[anode[k]].net;
            c = instance->pins[cathode[k]].net;
            if (a == NULL || c == NULL || a == c) {
                continue;
            }
            if (sim->diode_count == sim->diode_capacity) {
                Diode *grown = grow_array(sim->diodes, &sim->diode_capacity,
                                          sizeof(*sim->diodes));

                if (grown == NULL) {
                    if (anode != local_anode) {
                        free(anode);
                        free(cathode);
                    }
                    set_error(sim, "out of memory");
                    return 0;
                }
                sim->diodes = grown;
            }
            sim->diodes[sim->diode_count].anode = a;
            sim->diodes[sim->diode_count].cathode = c;
            sim->diode_count++;
        }
        if (anode != local_anode) {
            free(anode);
            free(cathode);
        }
    }
    return 1;
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

    if (!collect_diodes(sim) || !alloc_round_arrays(sim)) {
        return 0;
    }

    resolve_nets(sim);

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

/* Adds net `index` to this round's dirty set, saving its value first. */
static void mark_dirty(Sim *sim, size_t index) {
    if (!sim->is_dirty[index]) {
        sim->is_dirty[index] = 1;
        sim->old_values[index] = sim->nets[index]->value;
        sim->dirty[sim->dirty_count++] = index;
    }
}

static int compare_index(const void *a, const void *b) {
    size_t x = *(const size_t *)a;
    size_t y = *(const size_t *)b;

    return x < y ? -1 : x > y;
}

static void sort_dirty(Sim *sim) {
    size_t *d = sim->dirty;
    size_t n = sim->dirty_count;
    size_t i;

    if (n > 16) {
        qsort(d, n, sizeof(*d), compare_index);
        return;
    }
    for (i = 1; i < n; i++) {
        size_t value = d[i];
        size_t j = i;

        while (j > 0 && d[j - 1] > value) {
            d[j] = d[j - 1];
            j--;
        }
        d[j] = value;
    }
}

/*
 * Moves every pending, non-stale event at `time` into sim->batch, in
 * insertion order. Returns 0 (error set) when the batch cannot grow.
 */
static int take_batch(Sim *sim, SimTime time, size_t *count) {
    *count = 0;

    for (;;) {
        drop_stale(sim);
        if (sim->event_count == 0 || sim->events[0].time != time) {
            return 1;
        }
        if (*count == sim->batch_capacity) {
            Event *grown = grow_array(sim->batch, &sim->batch_capacity,
                                      sizeof(*sim->batch));

            if (grown == NULL) {
                set_error(sim, "out of memory");
                return 0;
            }
            sim->batch = grown;
        }
        heap_pop(sim, &sim->batch[(*count)++]);
    }
}

/*
 * One round at `time`: apply every event due, re-resolve only the nets those
 * events touched (the dirty set, widened to whole contact groups), then
 * notify pins of nets whose value changed and fire timers. Dirty nets are
 * handled in ascending index, the order a full scan would visit them.
 */
static int process_round(Sim *sim, SimTime time) {
    Event *batch;
    size_t batch_count = 0;
    int contact_changed = 0;
    size_t i;

    if (!take_batch(sim, time, &batch_count)) {
        return 0;
    }
    batch = sim->batch;
    sim->dirty_count = 0;

    for (i = 0; i < batch_count; i++) {
        if (batch[i].type == EVENT_DRIVE) {
            InstancePin *pin = &batch[i].instance->pins[batch[i].pin];
            SimNet *net = pin->net;

            pin->has_pending = 0;
            if (net != NULL) {
                mark_dirty(sim, net->index);
            }
            apply_drive(&batch[i]);
        } else if (batch[i].type == EVENT_CONTACT) {
            size_t k;

            for (k = 0; k < sim->net_count; k++) {
                mark_dirty(sim, k);
            }
            apply_contact(sim, batch[i].instance, batch[i].pin,
                          batch[i].pin_b, batch[i].closed);
            contact_changed = 1;
        }
    }

    if (sim->closed_contact_count == 0 && sim->diode_count == 0) {
        for (i = 0; i < sim->dirty_count; i++) {
            net_resolve(sim->nets[sim->dirty[i]]);
        }
        if (contact_changed) {
            sim->groups_valid = 0;
        }
    } else {
        size_t touched = sim->dirty_count;
        size_t none = sim->net_count;

        if (contact_changed || !sim->groups_valid) {
            build_groups(sim);
        }
        for (i = 0; i < touched; i++) {
            size_t k = sim->dirty[i];
            size_t j;

            if (sim->diode_count > 0) {
                /* A cluster holds its groups whole. */
                size_t root = sim->cluster_root[k];

                for (j = sim->cluster_head[root]; j != none; j = sim->cluster_next[j]) {
                    mark_dirty(sim, j);
                }
            } else {
                size_t root = sim->group_root[k];

                for (j = sim->group_head[root]; j != none; j = sim->group_next[j]) {
                    mark_dirty(sim, j);
                }
            }
        }
        for (i = 0; i < sim->dirty_count; i++) {
            size_t k = sim->dirty[i];

            if (sim->diode_count > 0) {
                if (sim->cluster_root[k] == k) {
                    resolve_cluster(sim, k);
                }
            } else if (sim->group_root[k] == k) {
                resolve_group(sim, k);
            }
        }
    }

    sort_dirty(sim);

    /* Two strong drivers fighting (e.g. VCC tied to GND) burns the circuit. */
    for (i = 0; i < sim->dirty_count && sim->error == NULL; i++) {
        SimNet *net = sim->nets[sim->dirty[i]];

        if (net->shorted) {
            sim->shorted_net = net;
            set_error(sim, "smoke: short circuit, strong drivers disagree on a net");
        }
    }

    for (i = 0; i < sim->dirty_count; i++) {
        sim->is_dirty[sim->dirty[i]] = 0;
    }

    for (i = 0; i < sim->dirty_count && sim->error == NULL; i++) {
        size_t index = sim->dirty[i];
        SimNet *net = sim->nets[index];
        LogicValue old_value = sim->old_values[index];
        size_t j;

        if (old_value == net->value) {
            continue;
        }
        net->changes++;

        for (j = 0; j < net->pin_count && sim->error == NULL; j++) {
            SimNetPin *pin = &net->pins[j];
            SimInstance *instance = pin->instance;

            if (instance->type->pin_changed != NULL) {
                instance->type->pin_changed(
                    sim,
                    instance,
                    instance->state,
                    pin->pin,
                    old_value,
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

    for (;;) {
        SimTime next_time;

        drop_stale(sim);
        if (sim->event_count == 0) {
            break;
        }
        next_time = sim->events[0].time;

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
