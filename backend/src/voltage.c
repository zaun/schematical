/*
 * The voltage check (include/voltage.h, plan-07-B section 4.2). A pure
 * function over arrays: no Circuit, no Sim.
 *
 *   1. Groups: nets joined by closed switch contacts and by a part's fixed
 *      contacts (an inductor's or a fuse's terminals) (union-find).
 *   2. Supply voltage S per group: the highest supply on it; two different
 *      non-zero supplies on one group are a clash. Pull parts (supply with
 *      "pull") are not supplies here: pull level P = the highest pull-up on
 *      the group; a pull never clashes.
 *   3. Fixed point: a part with a vcc range is powered at the highest known
 *      voltage on its power pins; driven D = the highest powered output on
 *      a group (open-drain outputs skipped); carried C = an LED's or a
 *      nixie's anode voltage on its cathode's group. Effective E = S, else D, else P,
 *      else C.
 *   3b. A regulator (plan-42-G): each element whose input has the headroom
 *      (a linear one needs its output voltage plus its dropout, a switching
 *      one its minimum input) and whose enable is not inactive raises the
 *      regulated level R of its output pins' groups to the part's output
 *      voltage; for an adjustable element Vref is worked out from the two
 *      resistors of its feedback divider (divider_mv). R is between S and D in
 *      the order of levels: S, else R, else D, else P, else C. A group whose
 *      R differs from its S, or that holds two different R, is a clash.
 *   3d. A power switch (plan-55-A): each switch whose input is known, at
 *      least the part's vinMin and non-zero, and whose enable is not
 *      inactive (a logic enable at its inactive level; a comparator enable
 *      "against the input" above the input) raises the switched level W of
 *      its output pins' groups to its input level. W sits between R and D:
 *      S, else R, else W, else D, else P, else C. Two switch outputs on one
 *      group never clash (the group reads the higher level); a W that
 *      differs from a supply's or a regulated level is a clash.
 *   3a. A part with supply domains (dual-supply translators, plan-20-F):
 *      each domain is powered at the highest known voltage on its own power
 *      pins, and the domain's outputs drive that level.
 *   4. Checks on every connection whose group has a known E; a pin of a
 *      domain part is judged against its own domain's range and input max.
 */

#include "voltage.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "model_pins.h"
#include "logic_alloc.h"

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

/* E: the supply level, else the regulated one, else the switched one, else the
   driven one, else the pulled one, else the carried one. */
static Level effective(const Level *s, const Level *r, const Level *w, const Level *d, const Level *p,
                       const Level *c, size_t g) {
    if (s[g].known) {
        return s[g];
    }
    if (r[g].known) {
        return r[g];
    }
    if (w[g].known) {
        return w[g];
    }
    if (d[g].known) {
        return d[g];
    }
    if (p[g].known) {
        return p[g];
    }
    return c[g];
}

/* Joins the nets on pins a and b of part (every connection of each). */
static void join_pins(size_t *parent, size_t net_count, const VoltageConnection *connections,
                      size_t connection_count, size_t part, size_t a, size_t b) {
    size_t j;
    size_t k;

    for (j = 0; j < connection_count; j++) {
        if (connections[j].part != part || connections[j].pin != a || connections[j].net >= net_count) {
            continue;
        }
        for (k = 0; k < connection_count; k++) {
            if (connections[k].part == part && connections[k].pin == b && connections[k].net < net_count) {
                join(parent, connections[j].net, connections[k].net);
            }
        }
    }
}

/* A fuse's or PTC's DC voltage rating, else its AC one (a fuse rated only for
   AC is held to the same number); 0 = none given (plan-47-I). */
static uint32_t fuse_limit_mv(const Part *part) {
    if (part->kind != PART_FUSE || part->fuse == NULL) {
        return 0;
    }
    return part->fuse->dc_mv > 0 ? part->fuse->dc_mv : part->fuse->ac_mv;
}

/* The group a pin's connection puts it on; 0 when the pin is not connected
   (found = 0). */
static size_t pin_group(size_t *parent, size_t net_count, const VoltageConnection *connections,
                        size_t connection_count, size_t part, size_t pin, int *found) {
    size_t j;

    for (j = 0; j < connection_count; j++) {
        if (connections[j].part == part && connections[j].pin == pin && connections[j].net < net_count) {
            *found = 1;
            return find(parent, connections[j].net);
        }
    }
    *found = 0;
    return 0;
}

/* The output level of an adjustable regulator element, in millivolts, from
   its feedback divider (plan-42-G section 3.3). O is the group of the
   element's output pin and A the group of its adjust pin; ground groups are
   those whose supply level is known and 0. Every resistor link with an end
   on A is a side of the divider: its other end on O is Rtop, on a ground
   group Rbottom (links in parallel combine); a link whose other end is
   anywhere else, or of value 0, leaves the level unknown, and both sides
   must have a link. A link whose other end is not connected is ignored. 0
   when the level is unknown. adjust_output: Vref sits between OUT and ADJ
   (LM317 style), v = Vref * (Rtop + Rbottom) / Rtop; else the adjust pin is
   Vref above ground, v = Vref * (Rtop + Rbottom) / Rbottom. */
static uint32_t divider_mv(const Part *const *parts, size_t part_count, size_t net_count,
                           const VoltageConnection *connections, size_t connection_count,
                           const uint64_t *milliohms, size_t *parent, const Level *supply, size_t o_group,
                           size_t a_group, uint32_t vref_mv, int adjust_output) {
    double g_top = 0, g_bottom = 0;
    int top = 0, bottom = 0;
    size_t i;
    size_t k;

    for (i = 0; i < part_count; i++) {
        size_t a;
        size_t b;

        if (parts[i]->kind != PART_RESISTOR || parts[i]->resistor == NULL) {
            continue;
        }
        for (k = 0; resistor_pair(parts[i], k, &a, &b); k++) {
            int fa;
            int fb;
            size_t ga = pin_group(parent, net_count, connections, connection_count, i, a, &fa);
            size_t gb = pin_group(parent, net_count, connections, connection_count, i, b, &fb);
            size_t other;
            int found_other;
            uint64_t value;

            if ((!fa || ga != a_group) && (!fb || gb != a_group)) {
                continue;               /* neither end on the adjust net */
            }
            if (fa && fb && ga == a_group && gb == a_group) {
                continue;               /* both ends on it: no divider side */
            }
            other = fa && ga == a_group ? gb : ga;
            found_other = fa && ga == a_group ? fb : fa;
            if (!found_other) {
                continue;
            }
            value = milliohms != NULL && milliohms[i] != 0 ? milliohms[i] : parts[i]->resistor->milliohms;
            if (value == 0) {
                return 0;
            }
            if (other == o_group) {
                g_top += 1.0 / (double)value;
                top = 1;
            } else if (supply[other].known && supply[other].mv == 0) {
                g_bottom += 1.0 / (double)value;
                bottom = 1;
            } else {
                return 0;
            }
        }
    }
    if (!top || !bottom) {
        return 0;
    }
    {
        double r_top = 1.0 / g_top;
        double r_bottom = 1.0 / g_bottom;
        double v = (double)vref_mv * (r_top + r_bottom) / (adjust_output ? r_top : r_bottom);

        if (v < 1 || v > 4e9) {
            return 0;
        }
        return (uint32_t)llround(v);
    }
}

/* 1 when pin is an input or an enable pin of any element of a regulator. */
static int regulator_vin_pin(const Part *part, size_t pin) {
    size_t k;
    size_t enable;

    for (k = 0; regulator_element(part, k, NULL, NULL, NULL, &enable, NULL, NULL, NULL, NULL); k++) {
        if (regulator_pin_in(part, k, pin) || (enable != SIZE_MAX && enable == pin)) {
            return 1;
        }
    }
    return 0;
}

/* 1 when pin is an input or an enable pin of any switch of a power switch. */
static int powerswitch_vin_pin(const Part *part, size_t pin) {
    size_t k;
    size_t enable;

    for (k = 0; powerswitch_element(part, k, NULL, NULL, &enable, NULL, NULL, NULL, NULL); k++) {
        if (powerswitch_pin_in(part, k, pin) || (enable != SIZE_MAX && enable == pin)) {
            return 1;
        }
    }
    return 0;
}

/* A nixie's or a dekatron's anode links and firing voltage (nixie.c, dekatron.c). */
static int tube_link(const Part *part, size_t k, size_t *anode, size_t *cathode) {
    return part->kind == PART_DEKATRON ? dekatron_link(part, k, anode, cathode)
                                       : nixie_link(part, k, anode, cathode);
}

static uint32_t tube_anode_min_mv(const Part *part, size_t pin) {
    return part->kind == PART_DEKATRON ? dekatron_anode_min_mv(part, pin) : nixie_anode_min_mv(part, pin);
}

size_t voltage_check(const Part *const *parts, size_t part_count, size_t net_count,
                     const VoltageConnection *connections, size_t connection_count,
                     const VoltageContact *closed, size_t closed_count,
                     const uint64_t *milliohms, VoltageProblem *out, size_t cap) {
    size_t *parent = logic_malloc((net_count + 1) * sizeof(*parent));
    Level *supply = logic_calloc(net_count + 1, sizeof(*supply));
    Level *driven = logic_calloc(net_count + 1, sizeof(*driven));
    Level *carried = logic_calloc(net_count + 1, sizeof(*carried));
    Level *pulled = logic_calloc(net_count + 1, sizeof(*pulled));
    Level *regulated = logic_calloc(net_count + 1, sizeof(*regulated));
    Level *switched = logic_calloc(net_count + 1, sizeof(*switched));
    size_t *sw_high_part = logic_malloc((net_count + 1) * sizeof(*sw_high_part));
    size_t *sw_high_pin = logic_malloc((net_count + 1) * sizeof(*sw_high_pin));
    size_t *sw_high_net = logic_malloc((net_count + 1) * sizeof(*sw_high_net));
    size_t *reg_low_part = logic_malloc((net_count + 1) * sizeof(*reg_low_part));
    size_t *reg_low_pin = logic_malloc((net_count + 1) * sizeof(*reg_low_pin));
    size_t *reg_low_net = logic_malloc((net_count + 1) * sizeof(*reg_low_net));
    size_t *reg_high_part = logic_malloc((net_count + 1) * sizeof(*reg_high_part));
    uint32_t *reg_low_mv = logic_calloc(net_count + 1, sizeof(*reg_low_mv));
    size_t *clash_first = logic_malloc((net_count + 1) * sizeof(*clash_first));
    size_t *clash_conn = logic_malloc((net_count + 1) * sizeof(*clash_conn));
    unsigned char *clash_done = logic_calloc(net_count + 1, 1);
    VoltageProblem *found = logic_malloc((connection_count + net_count + 1) * sizeof(*found));
    size_t found_count = 0;
    size_t i;
    size_t j;
    size_t round;
    int changed;

    if (parent == NULL || supply == NULL || driven == NULL || carried == NULL || pulled == NULL ||
        regulated == NULL || reg_low_part == NULL || reg_low_pin == NULL || reg_low_net == NULL ||
        reg_high_part == NULL || reg_low_mv == NULL || switched == NULL || sw_high_part == NULL ||
        sw_high_pin == NULL || sw_high_net == NULL ||
        clash_first == NULL || clash_conn == NULL || clash_done == NULL || found == NULL) {
        logic_free(parent); logic_free(supply); logic_free(driven); logic_free(carried); logic_free(pulled);
        logic_free(regulated); logic_free(reg_low_part); logic_free(reg_low_pin); logic_free(reg_low_net); logic_free(reg_high_part);
        logic_free(reg_low_mv); logic_free(switched); logic_free(sw_high_part); logic_free(sw_high_pin); logic_free(sw_high_net);
        logic_free(clash_first); logic_free(clash_conn); logic_free(clash_done); logic_free(found);
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
        join_pins(parent, net_count, connections, connection_count, closed[i].part, a, b);
    }
    /* A part's fixed contacts (an inductor's or a fuse's terminals, plan-47-I)
       join their nets whatever the circuit does; a blown fuse still counts. */
    for (i = 0; i < part_count; i++) {
        size_t a;
        size_t b;
        size_t k;

        for (k = 0; part_fixed_contact(parts[i], k, &a, &b); k++) {
            join_pins(parent, net_count, connections, connection_count, i, a, b);
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
            size_t anode = 0;
            size_t cathode = 0;

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
                            Level e = effective(supply, regulated, switched, driven, pulled, carried, find(parent, connections[j].net));

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
            /* A nixie or a dekatron: the anode's level is carried to every
               cathode net (an unlit cathode floats up towards the anode). They
               have no vcc. */
            if (part->kind == PART_NIXIE || part->kind == PART_DEKATRON) {
                size_t k;

                for (k = 0; tube_link(part, k, &anode, &cathode); k++) {
                    Level from = {0, 0};

                    for (j = 0; j < connection_count; j++) {
                        if (connections[j].part == i && connections[j].pin == anode &&
                            connections[j].net < net_count) {
                            Level e = effective(supply, regulated, switched, driven, pulled, carried, find(parent, connections[j].net));

                            if (e.known) {
                                level_raise(&from, e.mv);
                            }
                        }
                    }
                    for (j = 0; from.known && j < connection_count; j++) {
                        if (connections[j].part == i && connections[j].pin == cathode &&
                            connections[j].net < net_count) {
                            size_t g = find(parent, connections[j].net);
                            Level to = carried[g];

                            level_raise(&carried[g], from.mv);
                            changed |= to.known != carried[g].known || to.mv != carried[g].mv;
                        }
                    }
                }
                continue;
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
                            Level e = effective(supply, regulated, switched, driven, pulled, carried, find(parent, connections[j].net));

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
            /* A resistor (plan-34-E): each link carries each end's level to
               the other end's net, both ways. A resistor has no vcc. */
            if (part->kind == PART_RESISTOR) {
                size_t k;
                int way;

                for (k = 0; resistor_pair(part, k, &anode, &cathode); k++) {
                    for (way = 0; way < 2; way++) {
                        size_t from_pin = way == 0 ? anode : cathode;
                        size_t to_pin = way == 0 ? cathode : anode;
                        Level from = {0, 0};
                        Level to;

                        for (j = 0; j < connection_count; j++) {
                            if (connections[j].part == i && connections[j].pin == from_pin &&
                                connections[j].net < net_count) {
                                Level e = effective(supply, regulated, switched, driven, pulled, carried,
                                                    find(parent, connections[j].net));

                                if (e.known) {
                                    level_raise(&from, e.mv);
                                }
                            }
                        }
                        for (j = 0; from.known && j < connection_count; j++) {
                            if (connections[j].part == i && connections[j].pin == to_pin &&
                                connections[j].net < net_count) {
                                size_t g = find(parent, connections[j].net);

                                to = carried[g];
                                level_raise(&carried[g], from.mv);
                                changed |= to.known != carried[g].known || to.mv != carried[g].mv;
                            }
                        }
                    }
                }
                continue;
            }
            /* A regulator (3b): see the header. */
            if (part->kind == PART_REGULATOR && part->regulator != NULL) {
                const PartRegulator *reg = part->regulator;
                size_t k;
                const char *type;
                size_t in_pin, out_pin, en_pin, pg_pin, adj_pin;
                int en_low, adj_out;

                for (k = 0; regulator_element(part, k, &type, &in_pin, &out_pin, &en_pin, &en_low, &pg_pin,
                                              &adj_pin, &adj_out);
                     k++) {
                    Level vin = {0, 0};
                    uint32_t v;
                    int linear = type[0] == 'l';
                    int pin_found;

                    (void)pg_pin;
                    for (j = 0; j < connection_count; j++) {
                        if (connections[j].part == i && connections[j].net < net_count &&
                            regulator_pin_in(part, k, connections[j].pin)) {
                            Level e = effective(supply, regulated, switched, driven, pulled, carried,
                                                find(parent, connections[j].net));

                            if (e.known) {
                                level_raise(&vin, e.mv);
                            }
                        }
                    }
                    if (!vin.known || vin.mv == 0) {
                        continue;
                    }
                    if (en_pin != SIZE_MAX) {
                        size_t eg = pin_group(parent, net_count, connections, connection_count, i, en_pin,
                                              &pin_found);

                        if (pin_found) {
                            Level e = effective(supply, regulated, switched, driven, pulled, carried, eg);

                            if (e.known && (en_low ? e.mv > 0 : e.mv == 0)) {
                                continue;    /* disabled */
                            }
                        }
                    }
                    if (adj_pin != SIZE_MAX) {
                        int fo;
                        int fa;
                        size_t og = pin_group(parent, net_count, connections, connection_count, i, out_pin, &fo);
                        size_t ag = pin_group(parent, net_count, connections, connection_count, i, adj_pin, &fa);

                        if (!fo || !fa) {
                            continue;
                        }
                        v = divider_mv(parts, part_count, net_count, connections, connection_count, milliohms,
                                       parent, supply, og, ag, reg->vref_mv[k], adj_out);
                    } else {
                        v = reg->vout_mv[k];
                    }
                    if (v == 0) {
                        continue;
                    }
                    if ((linear ? vin.mv < v + reg->dropout_mv : 0) ||
                        (reg->vin_min_mv != 0 && vin.mv < reg->vin_min_mv)) {
                        continue;            /* no headroom: unpowered */
                    }
                    for (j = 0; j < connection_count; j++) {
                        const VoltageConnection *c = &connections[j];

                        if (c->part == i && c->net < net_count && regulator_pin_out(part, k, c->pin)) {
                            size_t g = find(parent, c->net);
                            Level before = regulated[g];

                            level_raise(&regulated[g], v);
                            if (!before.known) {
                                reg_low_part[g] = reg_high_part[g] = i;
                                reg_low_mv[g] = v;
                                reg_low_pin[g] = c->pin;
                                reg_low_net[g] = c->net;
                            } else {
                                if (v < reg_low_mv[g]) {
                                    reg_low_part[g] = i;
                                    reg_low_mv[g] = v;
                                    reg_low_pin[g] = c->pin;
                                    reg_low_net[g] = c->net;
                                }
                                if (v > before.mv) {
                                    reg_high_part[g] = i;
                                }
                            }
                            changed |= before.known != regulated[g].known || before.mv != regulated[g].mv;
                        }
                    }
                }
                continue;
            }
            /* A power switch (3d): see the header. */
            if (part->kind == PART_POWERSWITCH && part->powerswitch != NULL) {
                size_t k;
                size_t enable;
                int enable_low, ref_input;

                for (k = 0; powerswitch_element(part, k, NULL, NULL, &enable, &enable_low, &ref_input, NULL, NULL);
                     k++) {
                    Level vin = {0, 0};
                    int pin_found;

                    for (j = 0; j < connection_count; j++) {
                        if (connections[j].part == i && connections[j].net < net_count &&
                            powerswitch_pin_in(part, k, connections[j].pin)) {
                            Level e = effective(supply, regulated, switched, driven, pulled, carried,
                                                find(parent, connections[j].net));

                            if (e.known) {
                                level_raise(&vin, e.mv);
                            }
                        }
                    }
                    if (!vin.known || vin.mv == 0 || vin.mv < part->powerswitch->vin_min_mv) {
                        continue;
                    }
                    if (enable != SIZE_MAX) {
                        size_t eg = pin_group(parent, net_count, connections, connection_count, i, enable,
                                              &pin_found);

                        if (pin_found) {
                            Level e = effective(supply, regulated, switched, driven, pulled, carried, eg);

                            if (e.known && (ref_input ? e.mv > vin.mv : (enable_low ? e.mv > 0 : e.mv == 0))) {
                                continue;    /* disabled */
                            }
                        }
                    }
                    for (j = 0; j < connection_count; j++) {
                        const VoltageConnection *c = &connections[j];

                        if (c->part == i && c->net < net_count && powerswitch_pin_out(part, k, c->pin)) {
                            size_t g = find(parent, c->net);
                            Level before = switched[g];

                            level_raise(&switched[g], vin.mv);
                            if (!before.known || vin.mv > before.mv) {
                                sw_high_part[g] = i;
                                sw_high_pin[g] = c->pin;
                                sw_high_net[g] = c->net;
                            }
                            changed |= before.known != switched[g].known || before.mv != switched[g].mv;
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
                            Level e = effective(supply, regulated, switched, driven, pulled, carried, find(parent, c->net));

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
                    Level e = effective(supply, regulated, switched, driven, pulled, carried, find(parent, c->net));

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

    /* 3c. A regulated level that differs from the supply level of its group,
       or two regulators with different levels on one group: one clash per
       group, a supply clash found in step 2 first. Ground (S = 0) on an
       output is the engine's short, not a voltage problem. */
    for (i = 0; i < net_count; i++) {
        VoltageProblem *p;

        if (find(parent, i) != i || !regulated[i].known || clash_done[i]) {
            continue;
        }
        if (supply[i].known && supply[i].mv > 0 && clash_first[i] != SIZE_MAX &&
            supply[i].mv != regulated[i].mv) {
            const VoltageConnection *s = &connections[clash_first[i]];

            p = &found[found_count++];
            p->kind = VOLTAGE_SUPPLY_CLASH;
            p->net = s->net;
            p->part = s->part;
            p->other_part = reg_high_part[i];
            p->pin = s->pin;
            p->mv = supply[i].mv > regulated[i].mv ? supply[i].mv : regulated[i].mv;
            p->min_mv = p->max_mv = supply[i].mv < regulated[i].mv ? supply[i].mv : regulated[i].mv;
            clash_done[i] = 1;
        } else if (!supply[i].known && reg_low_mv[i] != regulated[i].mv) {
            p = &found[found_count++];
            p->kind = VOLTAGE_SUPPLY_CLASH;
            p->net = reg_low_net[i];
            p->part = reg_low_part[i];
            p->other_part = reg_high_part[i];
            p->pin = reg_low_pin[i];
            p->mv = regulated[i].mv;
            p->min_mv = p->max_mv = reg_low_mv[i];
            clash_done[i] = 1;
        }
    }

    /* 3d. A switched level that differs from the supply level or from the
       regulated level of its group: one clash per group, those above first.
       Two switch outputs on one group never clash: the group reads the
       higher level. Ground (S = 0) on an output is the engine's short. */
    for (i = 0; i < net_count; i++) {
        VoltageProblem *p;

        if (find(parent, i) != i || !switched[i].known || clash_done[i]) {
            continue;
        }
        if (supply[i].known && supply[i].mv > 0 && clash_first[i] != SIZE_MAX &&
            supply[i].mv != switched[i].mv) {
            const VoltageConnection *s = &connections[clash_first[i]];

            p = &found[found_count++];
            p->kind = VOLTAGE_SUPPLY_CLASH;
            p->net = s->net;
            p->part = s->part;
            p->other_part = sw_high_part[i];
            p->pin = s->pin;
            p->mv = supply[i].mv > switched[i].mv ? supply[i].mv : switched[i].mv;
            p->min_mv = p->max_mv = supply[i].mv < switched[i].mv ? supply[i].mv : switched[i].mv;
        } else if (!supply[i].known && regulated[i].known && regulated[i].mv != switched[i].mv) {
            int regulator_lower = regulated[i].mv < switched[i].mv;

            p = &found[found_count++];
            p->kind = VOLTAGE_SUPPLY_CLASH;
            p->net = regulator_lower ? reg_low_net[i] : sw_high_net[i];
            p->part = regulator_lower ? reg_low_part[i] : sw_high_part[i];
            p->other_part = regulator_lower ? sw_high_part[i] : reg_high_part[i];
            p->pin = regulator_lower ? reg_low_pin[i] : sw_high_pin[i];
            p->mv = regulated[i].mv > switched[i].mv ? regulated[i].mv : switched[i].mv;
            p->min_mv = p->max_mv = regulated[i].mv < switched[i].mv ? regulated[i].mv : switched[i].mv;
        } else {
            continue;
        }
        clash_done[i] = 1;
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
        e = effective(supply, regulated, switched, driven, pulled, carried, find(parent, c->net));
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
        } else if (part->kind == PART_CAPACITOR && part->capacitor != NULL &&
                   part->capacitor->rated_mv > 0 && e.mv > part->capacitor->rated_mv) {
            /* Either pin: a pin's net level against ground is the voltage
               across a part whose other pin is on ground (plan-33-E § 3.3). */
            p.kind = VOLTAGE_CAPACITOR_OVER;
            p.min_mv = 0;
            p.max_mv = part->capacitor->rated_mv;
        } else if (part->kind == PART_TRANSISTOR && transistor_pin_limit_mv(part, c->pin) > 0 &&
                   e.mv > transistor_pin_limit_mv(part, c->pin)) {
            /* A channel pin against Vceo / Vds, a MOSFET gate against Vgs;
               a BJT base is never checked (plan-40-F section 3.3). */
            p.kind = VOLTAGE_TRANSISTOR_OVER;
            p.min_mv = 0;
            p.max_mv = transistor_pin_limit_mv(part, c->pin);
        } else if (fuse_limit_mv(part) > 0 && e.mv > fuse_limit_mv(part)) {
            /* Either terminal, by the capacitor's reasoning above (plan-47-I). */
            p.kind = VOLTAGE_FUSE_OVER;
            p.min_mv = 0;
            p.max_mv = fuse_limit_mv(part);
        } else if (part->kind == PART_REGULATOR && regulator_input_max_mv(part) > 0 &&
                   regulator_vin_pin(part, c->pin) && e.mv > regulator_input_max_mv(part)) {
            /* An input or enable pin above the part's input maximum (plan-42-G). */
            p.kind = VOLTAGE_REGULATOR_INPUT_OVER;
            p.min_mv = 0;
            p.max_mv = regulator_input_max_mv(part);
        } else if (part->kind == PART_POWERSWITCH && powerswitch_input_max_mv(part) > 0 &&
                   powerswitch_vin_pin(part, c->pin) && e.mv > powerswitch_input_max_mv(part)) {
            /* An input or enable pin above the part's input maximum (plan-55-A). */
            p.kind = VOLTAGE_POWERSWITCH_INPUT_OVER;
            p.min_mv = 0;
            p.max_mv = powerswitch_input_max_mv(part);
        } else if ((part->kind == PART_NIXIE || part->kind == PART_DEKATRON) &&
                   e.mv < tube_anode_min_mv(part, c->pin)) {
            /* The anode supply must reach the firing voltage, or the tube
               never lights. */
            p.kind = VOLTAGE_NIXIE_UNDER;
            p.min_mv = tube_anode_min_mv(part, c->pin);
            p.max_mv = 0;
        } else {
            continue;
        }
        found[found_count++] = p;
    }

    qsort(found, found_count, sizeof(*found), compare_problems);
    for (i = 0; i < found_count && i < cap; i++) {
        out[i] = found[i];
    }
    logic_free(parent); logic_free(supply); logic_free(driven); logic_free(carried); logic_free(pulled);
    logic_free(regulated); logic_free(reg_low_part); logic_free(reg_low_pin); logic_free(reg_low_net); logic_free(reg_high_part);
    logic_free(reg_low_mv); logic_free(switched); logic_free(sw_high_part); logic_free(sw_high_pin); logic_free(sw_high_net);
    logic_free(clash_first); logic_free(clash_conn); logic_free(clash_done); logic_free(found);
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
            /* A supply part is judged by its voltage; a regulator part (the
               clash of two regulators) is the lower one (plan-42-G). */
            const Part *low = part->supply != NULL ? (part->supply->mv <= problem->min_mv ? part : other) : part;
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
        case VOLTAGE_CAPACITOR_OVER:
        case VOLTAGE_TRANSISTOR_OVER:
        case VOLTAGE_FUSE_OVER:
            return snprintf(buffer, size, "voltage: %g V on pin %s (%s) of %s; it is rated %g V", v,
                            pin_designator(part, problem->pin), pin_name(part, problem->pin),
                            part->name, problem->max_mv / 1000.0);
        case VOLTAGE_REGULATOR_INPUT_OVER:
        case VOLTAGE_POWERSWITCH_INPUT_OVER:
            return snprintf(buffer, size, "voltage: %g V on pin %s (%s) of %s; its input max is %g V", v,
                            pin_designator(part, problem->pin), pin_name(part, problem->pin),
                            part->name, problem->max_mv / 1000.0);
        case VOLTAGE_NIXIE_UNDER:
            return snprintf(buffer, size,
                            "voltage: %g V on pin %s (%s) of %s; it needs at least %g V to light", v,
                            pin_designator(part, problem->pin), pin_name(part, problem->pin),
                            part->name, problem->min_mv / 1000.0);
    }
    return snprintf(buffer, size, "voltage: problem");
}
