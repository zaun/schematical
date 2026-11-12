/*
 * The voltage check (include/voltage.h, plan-07-B section 4.2). A pure
 * function over arrays: no Circuit, no Sim.
 *
 *   1. Groups: nets joined by closed switch contacts (union-find).
 *   2. Supply voltage S per group: the highest supply on it; two different
 *      non-zero supplies on one group are a clash. Pull parts (supply with
 *      "pull") are not supplies here: pull level P = the highest pull-up on
 *      the group; a pull never clashes.
 *   3. Fixed point: a part with a vcc range is powered at the highest known
 *      voltage on its power pins; driven D = the highest powered output on
 *      a group (open-drain outputs skipped); carried C = an LED's anode
 *      voltage on its cathode's group. Effective E = S, else D, else P,
 *      else C.
 *   3a. A part with supply domains (dual-supply translators, plan-20-F):
 *      each domain is powered at the highest known voltage on its own power
 *      pins, and the domain's outputs drive that level.
 *   4. Checks on every connection whose group has a known E; a pin of a
 *      domain part is judged against its own domain's range and input max.
 */

#include "voltage.h"

#include <stdio.h>
#include <stdlib.h>

#include "model_pins.h"

typedef struct {
    int known;
    uint32_t mv;
} Level;

static size_t find(size_t *parent, size_t x) {
    while (parent[x] != x) {
        parent[x] = parent[parent[x]];
        x = parent[x];
    }
    return x;
}

static void join(size_t *parent, size_t a, size_t b) {
    a = find(parent, a);
    b = find(parent, b);
    if (a != b) {
        parent[a < b ? b : a] = a < b ? a : b;
    }
}

static void level_raise(Level *level, uint32_t mv) {
    if (!level->known || mv > level->mv) {
        level->known = 1;
        level->mv = mv;
    }
}

static int compare_problems(const void *a, const void *b) {
    const VoltageProblem *x = a;
    const VoltageProblem *y = b;

    if (x->net != y->net) {
        return x->net < y->net ? -1 : 1;
    }
    if (x->part != y->part) {
        return x->part < y->part ? -1 : 1;
    }
    if (x->pin != y->pin) {
        return x->pin < y->pin ? -1 : 1;
    }
    return (int)x->kind - (int)y->kind;
}

/* The domain of a dual-supply part that pin belongs to (as a power pin when
   *power is set), or -1. */
static int domain_of(const Part *part, size_t pin, int *power) {
    size_t k;
    size_t i;

    for (k = 0; k < part->domain_count; k++) {
        const PartSupplyDomain *d = &part->domains[k];

        for (i = 0; i < d->power_pin_count; i++) {
            if (d->power_pins[i] == pin) {
                *power = 1;
                return (int)k;
            }
        }
        for (i = 0; i < d->pin_count; i++) {
            if (d->pins[i] == pin) {
                *power = 0;
                return (int)k;
            }
        }
    }
    return -1;
}

/* E: the supply level, else the driven one, else the pulled one, else the
   carried one. */
static Level effective(const Level *s, const Level *d, const Level *p, const Level *c, size_t g) {
    if (s[g].known) {
        return s[g];
    }
    if (d[g].known) {
        return d[g];
    }
    if (p[g].known) {
        return p[g];
    }
    return c[g];
}

size_t voltage_check(const Part *const *parts, size_t part_count, size_t net_count,
                     const VoltageConnection *connections, size_t connection_count,
                     const VoltageContact *closed, size_t closed_count,
                     VoltageProblem *out, size_t cap) {
    size_t *parent = malloc((net_count + 1) * sizeof(*parent));
    Level *supply = calloc(net_count + 1, sizeof(*supply));
    Level *driven = calloc(net_count + 1, sizeof(*driven));
    Level *carried = calloc(net_count + 1, sizeof(*carried));
    Level *pulled = calloc(net_count + 1, sizeof(*pulled));
    size_t *clash_first = malloc((net_count + 1) * sizeof(*clash_first));
    size_t *clash_conn = malloc((net_count + 1) * sizeof(*clash_conn));
    unsigned char *clash_done = calloc(net_count + 1, 1);
    VoltageProblem *found = malloc((connection_count + net_count + 1) * sizeof(*found));
    size_t found_count = 0;
    size_t i;
    size_t j;
    size_t round;
    int changed;

    if (parent == NULL || supply == NULL || driven == NULL || carried == NULL || pulled == NULL ||
        clash_first == NULL || clash_conn == NULL || clash_done == NULL || found == NULL) {
        free(parent); free(supply); free(driven); free(carried); free(pulled);
        free(clash_first); free(clash_conn); free(clash_done); free(found);
        return 0;
    }

    /* 1. Groups. */
    for (i = 0; i < net_count; i++) {
        parent[i] = i;
        clash_first[i] = SIZE_MAX;
    }
    for (i = 0; i < closed_count; i++) {
        size_t a;
        size_t b;

        if (closed[i].part >= part_count ||
            !switch_contact_pins(parts[closed[i].part], closed[i].position, &a, &b)) {
            continue;
        }
        for (j = 0; j < connection_count; j++) {
            size_t k;

            if (connections[j].part != closed[i].part || connections[j].pin != a ||
                connections[j].net >= net_count) {
                continue;
            }
            for (k = 0; k < connection_count; k++) {
                if (connections[k].part == closed[i].part && connections[k].pin == b &&
                    connections[k].net < net_count) {
                    join(parent, connections[j].net, connections[k].net);
                }
            }
        }
    }

    /* 2. Supplies, in connection order; a clash is the first supply against the
       first (lowest part index) supply on the group that disagrees with it. */
    for (j = 0; j < connection_count; j++) {
        const VoltageConnection *c = &connections[j];
        const Part *part;
        size_t g;

        if (c->part >= part_count || c->net >= net_count) {
            continue;
        }
        part = parts[c->part];
        if (part->kind != PART_SUPPLY || part->supply == NULL) {
            continue;
        }
        g = find(parent, c->net);
        if (part->supply->pull) {
            if (part->supply->mv > 0) {
                level_raise(&pulled[g], part->supply->mv);
            }
            continue;
        }
        level_raise(&supply[g], part->supply->mv);
        if (part->supply->mv == 0) {
            continue;
        }
        if (clash_first[g] == SIZE_MAX || c->part < connections[clash_first[g]].part) {
            clash_first[g] = j;
        }
    }
    for (j = 0; j < connection_count; j++) {
        const VoltageConnection *c = &connections[j];
        const Part *part;
        const Part *first;
        size_t g;

        if (c->part >= part_count || c->net >= net_count) {
            continue;
        }
        part = parts[c->part];
        if (part->kind != PART_SUPPLY || part->supply == NULL || part->supply->mv == 0 ||
            part->supply->pull) {
            continue;
        }
        g = find(parent, c->net);
        first = parts[connections[clash_first[g]].part];
        if (part->supply->mv != first->supply->mv &&
            (!clash_done[g] || c->part < connections[clash_conn[g]].part)) {
            clash_done[g] = 1;
            clash_conn[g] = j;
        }
    }
    for (i = 0; i < net_count; i++) {
        const VoltageConnection *a;
        const VoltageConnection *b;
        uint32_t mva;
        uint32_t mvb;
        VoltageProblem *p;

        if (!clash_done[i]) {
            continue;
        }
        a = &connections[clash_first[i]];
        b = &connections[clash_conn[i]];
        mva = parts[a->part]->supply->mv;
        mvb = parts[b->part]->supply->mv;
        p = &found[found_count++];
        p->kind = VOLTAGE_SUPPLY_CLASH;
        p->net = a->net < b->net ? a->net : b->net;
        p->part = a->part < b->part ? a->part : b->part;
        p->other_part = a->part < b->part ? b->part : a->part;
        p->pin = p->part == a->part ? a->pin : b->pin;
        p->mv = mva > mvb ? mva : mvb;
        p->min_mv = p->max_mv = mva < mvb ? mva : mvb;
    }

    /* 3. Fixed point: driven and carried levels only rise. */
    for (round = 0, changed = 1; changed && round <= part_count + net_count + 1; round++) {
        changed = 0;
        for (i = 0; i < part_count; i++) {
            const Part *part = parts[i];
            Level power = {0, 0};
            size_t anode;
            size_t cathode;

            /* An LED, or each LED of a character LCD's backlight: the
               anode's level is carried to the cathode net. Only an LED skips
               the vcc checks. */
            if (part->kind == PART_LED || part->kind == PART_CHARLCD) {
                size_t k;

                for (k = 0; part->kind == PART_LED ? k == 0 && led_pins(part, &anode, &cathode)
                                                   : charlcd_backlight_pins(part, k, &anode, &cathode);
                     k++) {
                    Level from = {0, 0};
                    Level to;

                    for (j = 0; j < connection_count; j++) {
                        if (connections[j].part == i && connections[j].pin == anode &&
                            connections[j].net < net_count) {
                            Level e = effective(supply, driven, pulled, carried, find(parent, connections[j].net));

                            if (e.known) {
                                level_raise(&from, e.mv);
                            }
                        }
                    }
                    for (j = 0; from.known && j < connection_count; j++) {
                        if (connections[j].part == i && connections[j].pin == cathode &&
                            connections[j].net < net_count) {
                            size_t g = find(parent, connections[j].net);

                            to = carried[g];
                            level_raise(&carried[g], from.mv);
                            changed |= to.known != carried[g].known || to.mv != carried[g].mv;
                        }
                    }
                }
                if (part->kind == PART_LED) {
                    continue;
                }
            }
            /* A diode: each link carries its anode's level to its cathode
               net. A diode has no vcc. */
            if (part->kind == PART_DIODE) {
                size_t k;

                for (k = 0; diode_pair(part, k, &anode, &cathode); k++) {
                    Level from = {0, 0};
                    Level to;

                    for (j = 0; j < connection_count; j++) {
                        if (connections[j].part == i && connections[j].pin == anode &&
                            connections[j].net < net_count) {
                            Level e = effective(supply, driven, pulled, carried, find(parent, connections[j].net));

                            if (e.known) {
                                level_raise(&from, e.mv);
                            }
                        }
                    }
                    for (j = 0; from.known && j < connection_count; j++) {
                        if (connections[j].part == i && connections[j].pin == cathode &&
                            connections[j].net < net_count) {
                            size_t g = find(parent, connections[j].net);

                            to = carried[g];
                            level_raise(&carried[g], from.mv);
                            changed |= to.known != carried[g].known || to.mv != carried[g].mv;
                        }
                    }
                }
                continue;
            }
            if (part->vcc == NULL) {
                continue;
            }
            if (part->domain_count > 0) {
                /* 3a. Each domain from its own power pins. */
                size_t k;

                for (k = 0; k < part->domain_count; k++) {
                    Level dp = {0, 0};

                    for (j = 0; j < connection_count; j++) {
                        const VoltageConnection *c = &connections[j];
                        int is_power;

                        if (c->part == i && c->net < net_count && c->pin < part->pin_count &&
                            domain_of(part, c->pin, &is_power) == (int)k && is_power) {
                            Level e = effective(supply, driven, pulled, carried, find(parent, c->net));

                            if (e.known) {
                                level_raise(&dp, e.mv);
                            }
                        }
                    }
                    if (!dp.known || dp.mv == 0) {
                        continue;
                    }
                    for (j = 0; j < connection_count; j++) {
                        const VoltageConnection *c = &connections[j];
                        int is_power;
                        PinRole role;

                        if (c->part != i || c->net >= net_count || c->pin >= part->pin_count ||
                            domain_of(part, c->pin, &is_power) != (int)k || is_power) {
                            continue;
                        }
                        role = part->pins[c->pin].role;
                        if ((role == PIN_INOUT || role == PIN_OUTPUT) &&
                            !part_pin_open_drain(part, c->pin) && !part_pin_analog(part, c->pin)) {
                            size_t g = find(parent, c->net);
                            Level before = driven[g];

                            level_raise(&driven[g], dp.mv);
                            changed |= before.known != driven[g].known || before.mv != driven[g].mv;
                        }
                    }
                }
                continue;
            }
            for (j = 0; j < connection_count; j++) {
                const VoltageConnection *c = &connections[j];

                if (c->part == i && c->net < net_count && c->pin < part->pin_count &&
                    part->pins[c->pin].role == PIN_POWER && !part_pin_analog(part, c->pin)) {
                    Level e = effective(supply, driven, pulled, carried, find(parent, c->net));

                    if (e.known) {
                        level_raise(&power, e.mv);
                    }
                }
            }
            if (!power.known || power.mv == 0) {
                continue;
            }
            for (j = 0; j < connection_count; j++) {
                const VoltageConnection *c = &connections[j];
                PinRole role;

                if (c->part != i || c->net >= net_count || c->pin >= part->pin_count) {
                    continue;
                }
                role = part->pins[c->pin].role;
                if ((role == PIN_INOUT || role == PIN_OUTPUT) && !part_pin_open_drain(part, c->pin) &&
                    !part_pin_analog(part, c->pin)) {
                    size_t g = find(parent, c->net);
                    Level before = driven[g];

                    level_raise(&driven[g], power.mv);
                    changed |= before.known != driven[g].known || before.mv != driven[g].mv;
                }
            }
        }
    }

    /* 4. Checks. */
    for (j = 0; j < connection_count; j++) {
        const VoltageConnection *c = &connections[j];
        const Part *part;
        Level e;
        PinRole role;
        VoltageProblem p;

        if (c->part >= part_count || c->net >= net_count) {
            continue;
        }
        part = parts[c->part];
        if (c->pin >= part->pin_count || part_pin_analog(part, c->pin)) {
            continue;
        }
        e = effective(supply, driven, pulled, carried, find(parent, c->net));
        if (!e.known) {
            continue;
        }
        role = part->pins[c->pin].role;
        p.net = c->net;
        p.part = c->part;
        p.other_part = c->part;
        p.pin = c->pin;
        p.mv = e.mv;
        if (part->domain_count > 0) {
            /* A domain part: each pin against its own domain. */
            int is_power;
            int k = domain_of(part, c->pin, &is_power);
            const PartVoltage *v = k >= 0 ? part->domains[k].vcc : NULL;

            if (v == NULL) {
                continue;
            }
            if (is_power && (e.mv < v->min_mv || e.mv > v->max_mv)) {
                p.kind = VOLTAGE_POWER_RANGE;
                p.min_mv = v->min_mv;
                p.max_mv = v->max_mv;
            } else if (!is_power && (role == PIN_INPUT || role == PIN_INOUT) &&
                       e.mv > v->input_max_mv) {
                p.kind = VOLTAGE_INPUT_OVER;
                p.min_mv = 0;
                p.max_mv = v->input_max_mv;
            } else {
                continue;
            }
            found[found_count++] = p;
            continue;
        }
        if (part->vcc != NULL && role == PIN_POWER &&
            (e.mv < part->vcc->min_mv || e.mv > part->vcc->max_mv)) {
            p.kind = VOLTAGE_POWER_RANGE;
            p.min_mv = part->vcc->min_mv;
            p.max_mv = part->vcc->max_mv;
        } else if (part->vcc != NULL && (role == PIN_INPUT || role == PIN_INOUT) &&
                   e.mv > part->vcc->input_max_mv) {
            p.kind = VOLTAGE_INPUT_OVER;
            p.min_mv = 0;
            p.max_mv = part->vcc->input_max_mv;
        } else if (part->kind == PART_SWITCH && part->dip_switch != NULL &&
                   part->dip_switch->rated_mv > 0 && role == PIN_INOUT &&
                   e.mv > part->dip_switch->rated_mv) {
            p.kind = VOLTAGE_SWITCH_OVER;
            p.min_mv = 0;
            p.max_mv = part->dip_switch->rated_mv;
        } else {
            continue;
        }
        found[found_count++] = p;
    }

    qsort(found, found_count, sizeof(*found), compare_problems);
    for (i = 0; i < found_count && i < cap; i++) {
        out[i] = found[i];
    }
    free(parent); free(supply); free(driven); free(carried); free(pulled);
    free(clash_first); free(clash_conn); free(clash_done); free(found);
    return found_count;
}

static const char *pin_designator(const Part *part, size_t pin) {
    return pin < part->pin_count ? part->pins[pin].designator : "?";
}

static const char *pin_name(const Part *part, size_t pin) {
    return pin < part->pin_count ? part->pins[pin].name : "?";
}

int voltage_describe(const Part *const *parts, const VoltageProblem *problem,
                     char *buffer, size_t size) {
    const Part *part = parts[problem->part];
    double v = problem->mv / 1000.0;

    switch (problem->kind) {
        case VOLTAGE_SUPPLY_CLASH: {
            const Part *other = parts[problem->other_part];
            const Part *low = part->supply != NULL && part->supply->mv <= problem->min_mv ? part : other;
            const Part *high = low == part ? other : part;

            return snprintf(buffer, size, "voltage: %g V and %g V supplies on one net (%s, %s)",
                            problem->min_mv / 1000.0, v, low->name, high->name);
        }
        case VOLTAGE_POWER_RANGE:
            return snprintf(buffer, size, "voltage: %g V on pin %s (%s) of %s; it needs %g-%g V", v,
                            pin_designator(part, problem->pin), pin_name(part, problem->pin),
                            part->name, problem->min_mv / 1000.0, problem->max_mv / 1000.0);
        case VOLTAGE_INPUT_OVER:
            return snprintf(buffer, size,
                            "voltage: %g V on pin %s (%s) of %s; its inputs take at most %g V", v,
                            pin_designator(part, problem->pin), pin_name(part, problem->pin),
                            part->name, problem->max_mv / 1000.0);
        case VOLTAGE_SWITCH_OVER:
            return snprintf(buffer, size, "voltage: %g V on pin %s (%s) of %s; it is rated %g V", v,
                            pin_designator(part, problem->pin), pin_name(part, problem->pin),
                            part->name, problem->max_mv / 1000.0);
    }
    return snprintf(buffer, size, "voltage: problem");
}
