/* engine/sim_int.h: what the pieces of engine/sim.c share (plan-68-I/J split it). */
#ifndef ENGINE_SIM_INT_H
#define ENGINE_SIM_INT_H

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

/* A link between two nets, collected by sim_start: a one-way diode link
 * (weak 0), or a resistor's two-way weak link (weak 1; anode and cathode are
 * then its two ends a and b, plan-34-E). */
typedef struct {
    SimNet *anode;
    SimNet *cathode;
    int weak;
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

void simc_set_error(Sim *sim, const char *message);
void *simc_grow_array(void *array, size_t *capacity, size_t element_size);
int simc_append_event(Sim *sim, const Event *event);
void simc_heap_pop(Sim *sim, Event *out);
void simc_drop_stale(Sim *sim);

#endif
