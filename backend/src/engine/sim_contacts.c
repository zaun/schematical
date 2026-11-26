/* engine/sim_contacts.c: part of the model in engine/sim.c (split by plan-68-I/J). */
#include "sim_int.h"
#include "logic_alloc.h"
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
            Contact *grown = simc_grow_array(sim->contacts, &sim->contact_capacity,
                                        sizeof(*sim->contacts));

            if (grown == NULL) {
                simc_set_error(sim, "out of memory");
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
        simc_set_error(sim, "invalid contact");
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
    simc_append_event(sim, &event);
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

            if (!sim->diodes[d].weak && a != c && pass_link(&strong[a], &strong[c])) {
                grew = 1;
            }
        }
    } while (grew);

    do {
        grew = 0;
        for (d = sim->link_head[root]; d != sim->diode_count; d = sim->link_next[d]) {
            size_t a = sim->group_root[sim->diodes[d].anode->index];
            size_t c = sim->group_root[sim->diodes[d].cathode->index];

            if (a == c) {
                continue;
            }
            if (sim->diodes[d].weak) {
                /* A resistor (plan-34-E section 3.1): each end's level, strong
                   if it has one, reaches the other end as a weak value. */
                unsigned char from_a = strong[a] != 0 ? strong[a] : weak[a];
                unsigned char from_c = strong[c] != 0 ? strong[c] : weak[c];

                if ((weak[c] | from_a) != weak[c]) {
                    weak[c] |= from_a;
                    grew = 1;
                }
                if ((weak[a] | from_c) != weak[a]) {
                    weak[a] |= from_c;
                    grew = 1;
                }
            } else if (strong[a] == 0 && strong[c] == 0 &&
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

    sim->dirty = logic_malloc(n * sizeof(*sim->dirty));
    sim->is_dirty = logic_calloc(n, sizeof(*sim->is_dirty));
    sim->old_values = logic_malloc(n * sizeof(*sim->old_values));
    sim->group_parent = logic_malloc(n * sizeof(*sim->group_parent));
    sim->group_root = logic_malloc(n * sizeof(*sim->group_root));
    sim->group_head = logic_malloc(n * sizeof(*sim->group_head));
    sim->group_next = logic_malloc(n * sizeof(*sim->group_next));
    sim->group_nets = logic_malloc(n * sizeof(*sim->group_nets));

    if (sim->dirty == NULL || sim->is_dirty == NULL ||
        sim->old_values == NULL || sim->group_parent == NULL ||
        sim->group_root == NULL || sim->group_head == NULL ||
        sim->group_next == NULL || sim->group_nets == NULL) {
        simc_set_error(sim, "out of memory");
        return 0;
    }
    if (sim->diode_count > 0) {
        size_t links = sim->diode_count;

        sim->cluster_parent = logic_malloc(n * sizeof(*sim->cluster_parent));
        sim->cluster_root = logic_malloc(n * sizeof(*sim->cluster_root));
        sim->cluster_head = logic_malloc(n * sizeof(*sim->cluster_head));
        sim->cluster_next = logic_malloc(n * sizeof(*sim->cluster_next));
        sim->link_head = logic_malloc(n * sizeof(*sim->link_head));
        sim->link_next = logic_malloc(links * sizeof(*sim->link_next));
        sim->node_strong = logic_malloc(n);
        sim->node_weak = logic_malloc(n);
        if (sim->cluster_parent == NULL || sim->cluster_root == NULL ||
            sim->cluster_head == NULL || sim->cluster_next == NULL ||
            sim->link_head == NULL || sim->link_next == NULL ||
            sim->node_strong == NULL || sim->node_weak == NULL) {
            simc_set_error(sim, "out of memory");
            return 0;
        }
    }
    return 1;
}

/*
 * Collects every instance's links whose two pins are on two different nets:
 * the diode links (the `diodes` hook), then the resistor links (the
 * `resistors` hook, weak). 0 (error set) on failure.
 */
static int collect_links(Sim *sim) {
    size_t i;
    int weak;

    for (weak = 0; weak <= 1; weak++)
    for (i = 0; i < sim->instance_count; i++) {
        SimInstance *instance = sim->instances[i];
        size_t (*hook)(const void *, size_t *, size_t *, size_t) =
            weak ? instance->type->resistors : instance->type->diodes;
        size_t local_anode[16];
        size_t local_cathode[16];
        size_t *anode = local_anode;
        size_t *cathode = local_cathode;
        size_t count;
        size_t k;

        if (hook == NULL) {
            continue;
        }
        count = hook(instance->state, anode, cathode, 16);
        if (count > 16) {
            anode = logic_malloc(count * sizeof(*anode));
            cathode = logic_malloc(count * sizeof(*cathode));
            if (anode == NULL || cathode == NULL) {
                logic_free(anode);
                logic_free(cathode);
                simc_set_error(sim, "out of memory");
                return 0;
            }
            count = hook(instance->state, anode, cathode, count);
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
                Diode *grown = simc_grow_array(sim->diodes, &sim->diode_capacity,
                                          sizeof(*sim->diodes));

                if (grown == NULL) {
                    if (anode != local_anode) {
                        logic_free(anode);
                        logic_free(cathode);
                    }
                    simc_set_error(sim, "out of memory");
                    return 0;
                }
                sim->diodes = grown;
            }
            sim->diodes[sim->diode_count].anode = a;
            sim->diodes[sim->diode_count].cathode = c;
            sim->diodes[sim->diode_count].weak = weak;
            sim->diode_count++;
        }
        if (anode != local_anode) {
            logic_free(anode);
            logic_free(cathode);
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
        simc_set_error(sim, "simulation already started");
        return 0;
    }

    sim->started = 1;

    if (!collect_links(sim) || !alloc_round_arrays(sim)) {
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
        simc_drop_stale(sim);
        if (sim->event_count == 0 || sim->events[0].time != time) {
            return 1;
        }
        if (*count == sim->batch_capacity) {
            Event *grown = simc_grow_array(sim->batch, &sim->batch_capacity,
                                      sizeof(*sim->batch));

            if (grown == NULL) {
                simc_set_error(sim, "out of memory");
                return 0;
            }
            sim->batch = grown;
        }
        simc_heap_pop(sim, &sim->batch[(*count)++]);
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
            simc_set_error(sim, "smoke: short circuit, strong drivers disagree on a net");
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
        simc_set_error(sim, "invalid run time");
        return 0;
    }

    for (;;) {
        SimTime next_time;

        simc_drop_stale(sim);
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
            simc_set_error(sim, "circuit did not settle");
            return 0;
        }

        sim->now = next_time;

        /* a model that called sim_fail in this round stops the run here */
        if (!process_round(sim, next_time) || sim->error != NULL) {
            return 0;
        }
    }

    sim->now = time;
    return 1;
}
