/*
 * The timing analysis of the `timer` kind (plan-48-E section 4). One call
 * looks at one section of one timer: it collects the resistor, pull,
 * diode and capacitor links of the whole circuit, then follows the steps of
 * the plan in order (latch, supply, control pin, the four circuits, values,
 * times, speed limit). Nets are the circuit's net indices; an unwired pin
 * has none (NONE).
 */

#include <math.h>
#include <stdlib.h>

#include "model_pins.h"
#include "timer.h"
#include "timer_rc.h"

#define NONE SIZE_MAX
/* The far end of a pull part: the rail it ties its pin to. */
#define RAIL_UP (SIZE_MAX - 1)
#define RAIL_DOWN (SIZE_MAX - 2)
#define RAIL_OTHER (SIZE_MAX - 3)

#define LN2 0.6931471805599453
#define LN3 1.0986122886681098
#define DEFAULT_PULL_MILLIOHMS 1000000ULL

typedef struct { size_t a, b; uint64_t value; } Link;           /* a resistor, or a pull part (b = a rail) */
typedef struct { size_t anode, cathode; } DiodeLink;
typedef struct { size_t part; size_t a, b; uint64_t value; } CapLink;

typedef struct {
    const Part *const *parts;
    const uint64_t *values;
    size_t part_count;
    const VoltageConnection *conn;
    size_t conn_count;
    size_t part;                  /* the timer part */
    uint32_t ref_mv;              /* the supply on the VCC pin's net */
    size_t *pin_net;              /* per (part, pin): the net, NONE when unwired */
    size_t *offset;               /* per part: its first pin_net entry */
    Link *links; size_t link_count;
    DiodeLink *diodes; size_t diode_count;
    CapLink *caps; size_t cap_count;
} Ctx;

const char *timer_reason_text(TimerReason reason) {
    switch (reason) {
    case TIMER_NO_SUPPLY: return "its VCC and GND pins are not both on a supply";
    case TIMER_CONTROL: return "the CONT pin is wired to something other than a capacitor to ground";
    case TIMER_SHAPE:
        return "the resistors and capacitors on TRIG, THRES and DISCH do not form an astable or a one-shot circuit";
    case TIMER_NO_VALUE: return "a timing resistor or capacitor has no value";
    case TIMER_TOO_FAST: return "the timing is faster than this part allows";
    case TIMER_OK: break;
    }
    return "";
}

static size_t net_of(const Ctx *x, size_t part, size_t pin) {
    return x->pin_net[x->offset[part] + pin];
}

/* A supply part's net carries `level` 0 / 1 and no pull. */
static int carries_supply(const Ctx *x, size_t net, int level, int use_ref) {
    size_t i;

    if (net == NONE) {
        return 0;
    }
    for (i = 0; i < x->conn_count; i++) {
        const Part *p;

        if (x->conn[i].net != net) {
            continue;
        }
        p = x->parts[x->conn[i].part];
        if (p->kind == PART_SUPPLY && !p->supply->pull && p->supply->level == level &&
            (!use_ref || p->supply->mv == x->ref_mv)) {
            return 1;
        }
    }
    return 0;
}

static int is_ground(const Ctx *x, size_t net) {
    return net == RAIL_DOWN || carries_supply(x, net, 0, 0);
}

static int is_supply(const Ctx *x, size_t net) {
    return net == RAIL_UP || carries_supply(x, net, 1, 1);
}

static uint64_t resistor_value(const Ctx *x, size_t part) {
    return x->values[part] != 0 ? x->values[part] : x->parts[part]->resistor->milliohms;
}

static int add_link(Ctx *x, size_t *cap, size_t a, size_t b, uint64_t value) {
    if (x->link_count == *cap) {
        size_t n = *cap == 0 ? 16 : *cap * 2;
        Link *grown = realloc(x->links, n * sizeof(*grown));

        if (grown == NULL) {
            return -1;
        }
        x->links = grown;
        *cap = n;
    }
    x->links[x->link_count].a = a;
    x->links[x->link_count].b = b;
    x->links[x->link_count].value = value;
    x->link_count++;
    return 0;
}

/* Builds the pin -> net table and the diode and capacitor lists. 0, or -1 when out of memory. */
static int build(Ctx *x) {
    size_t total = 0, i, part, diode_cap = 0, cap_cap = 0;

    x->offset = malloc((x->part_count + 1) * sizeof(*x->offset));
    if (x->offset == NULL) {
        return -1;
    }
    for (part = 0; part < x->part_count; part++) {
        x->offset[part] = total;
        total += x->parts[part]->pin_count;
    }
    x->pin_net = malloc((total + 1) * sizeof(*x->pin_net));
    if (x->pin_net == NULL) {
        return -1;
    }
    for (i = 0; i < total; i++) {
        x->pin_net[i] = NONE;
    }
    for (i = 0; i < x->conn_count; i++) {
        x->pin_net[x->offset[x->conn[i].part] + x->conn[i].pin] = x->conn[i].net;
    }
    for (part = 0; part < x->part_count; part++) {
        const Part *p = x->parts[part];
        size_t k, a, b;

        if (p->kind == PART_DIODE) {
            for (k = 0; diode_pair(p, k, &a, &b); k++) {
                if (x->diode_count == diode_cap) {
                    size_t n = diode_cap == 0 ? 8 : diode_cap * 2;
                    DiodeLink *grown = realloc(x->diodes, n * sizeof(*grown));

                    if (grown == NULL) {
                        return -1;
                    }
                    x->diodes = grown;
                    diode_cap = n;
                }
                x->diodes[x->diode_count].anode = net_of(x, part, a);
                x->diodes[x->diode_count].cathode = net_of(x, part, b);
                x->diode_count++;
            }
        } else if (p->kind == PART_CAPACITOR && p->pin_count >= 2) {
            if (x->cap_count == cap_cap) {
                size_t n = cap_cap == 0 ? 8 : cap_cap * 2;
                CapLink *grown = realloc(x->caps, n * sizeof(*grown));

                if (grown == NULL) {
                    return -1;
                }
                x->caps = grown;
                cap_cap = n;
            }
            x->caps[x->cap_count].part = part;
            x->caps[x->cap_count].a = net_of(x, part, 0);
            x->caps[x->cap_count].b = net_of(x, part, 1);
            x->caps[x->cap_count].value = x->values[part] != 0 ? x->values[part] : p->capacitor->capacitance_ff;
            x->cap_count++;
        }
    }
    return 0;
}

/* The resistor and pull links; the pull-ups' rail depends on ref_mv. 0, or -1 when out of memory. */
static int build_links(Ctx *x) {
    size_t part, cap = 0;

    free(x->links);
    x->links = NULL;
    x->link_count = 0;
    for (part = 0; part < x->part_count; part++) {
        const Part *p = x->parts[part];
        size_t k, a, b;

        if (p->kind == PART_RESISTOR) {
            for (k = 0; resistor_pair(p, k, &a, &b); k++) {
                if (add_link(x, &cap, net_of(x, part, a), net_of(x, part, b), resistor_value(x, part)) != 0) {
                    return -1;
                }
            }
        } else if (p->kind == PART_SUPPLY && p->supply->pull && p->pin_count > 0) {
            size_t rail = p->supply->level ? (p->supply->mv == x->ref_mv ? RAIL_UP : RAIL_OTHER) : RAIL_DOWN;

            if (add_link(x, &cap, net_of(x, part, 0), rail,
                         x->values[part] != 0 ? x->values[part] : DEFAULT_PULL_MILLIOHMS) != 0) {
                return -1;
            }
        }
    }
    return 0;
}

static void release(Ctx *x) {
    free(x->pin_net);
    free(x->offset);
    free(x->links);
    free(x->diodes);
    free(x->caps);
}

/* ---- Queries ---------------------------------------------------------------- */

/* A foreign driver on the net: a pin of another part that drives it. */
static int has_driver(const Ctx *x, size_t net) {
    size_t i;

    for (i = 0; i < x->conn_count; i++) {
        const Part *p;

        if (x->conn[i].net != net || x->conn[i].part == x->part) {
            continue;
        }
        p = x->parts[x->conn[i].part];
        if (p->kind == PART_RESISTOR || p->kind == PART_CAPACITOR || p->kind == PART_DIODE) {
            continue;
        }
        if (p->kind == PART_SUPPLY) {
            if (!p->supply->pull) {
                return 1;
            }
            continue;
        }
        if (p->pins[x->conn[i].pin].role == PIN_OUTPUT || p->pins[x->conn[i].pin].role == PIN_INOUT) {
            return 1;
        }
    }
    return 0;
}

/* Resistor and pull links with an end on `net`. */
static size_t links_on(const Ctx *x, size_t net) {
    size_t i, n = 0;

    for (i = 0; i < x->link_count; i++) {
        if (x->links[i].a == net || x->links[i].b == net) {
            n++;
        }
    }
    return n;
}

/* The links between `net` and a far end: `other` (a net) or, when
   other == RAIL_UP, any supply net. `*value` is their parallel resistance
   (0 when one of them is 0 ohms); returns their number. */
static size_t links_between(const Ctx *x, size_t net, size_t other, uint64_t *value) {
    size_t i, n = 0;
    double conductance = 0;
    int zero = 0;
    uint64_t single = 0;

    for (i = 0; i < x->link_count; i++) {
        size_t far;
        int match;

        if (x->links[i].a == net) {
            far = x->links[i].b;
        } else if (x->links[i].b == net) {
            far = x->links[i].a;
        } else {
            continue;
        }
        match = other == RAIL_UP ? (far != net && is_supply(x, far)) : (far == other && far != NONE);
        if (!match) {
            continue;
        }
        n++;
        single = x->links[i].value;
        if (single == 0) {
            zero = 1;
        } else {
            conductance += 1.0 / (double)single;
        }
    }
    if (n == 1) {
        *value = single;
    } else {
        *value = (n == 0 || zero || conductance <= 0) ? 0 : (uint64_t)llround(1.0 / conductance);
    }
    return n;
}

/* The capacitors with one end on `net` and the other on a ground net: their
   number and their sum in femtofarads. */
static size_t capacitors_on(const Ctx *x, size_t net, uint64_t *sum) {
    size_t i, n = 0;

    *sum = 0;
    for (i = 0; i < x->cap_count; i++) {
        const CapLink *c = &x->caps[i];

        if ((c->a == net && is_ground(x, c->b)) || (c->b == net && is_ground(x, c->a))) {
            n++;
            *sum += c->value;
        }
    }
    return n;
}

/* Every capacitor that touches net `n1` or `n2` goes from that net to ground. */
static int capacitors_clean(const Ctx *x, size_t n1, size_t n2) {
    size_t i;

    for (i = 0; i < x->cap_count; i++) {
        const CapLink *c = &x->caps[i];
        int touches1 = c->a == n1 || c->b == n1;
        int touches2 = n2 != NONE && (c->a == n2 || c->b == n2);

        if (!touches1 && !touches2) {
            continue;
        }
        if (!(touches1 && ((c->a == n1 && is_ground(x, c->b)) || (c->b == n1 && is_ground(x, c->a))))) {
            return 0;
        }
    }
    return 1;
}

/* A diode with an end on net a or b is not the diode (anode D, cathode C): the count of
   conforming ones, or -1 when another touches either. */
static int diodes_between(const Ctx *x, size_t c, size_t d) {
    size_t i;
    int n = 0;

    for (i = 0; i < x->diode_count; i++) {
        const DiodeLink *k = &x->diodes[i];
        int touches = (k->anode != NONE && (k->anode == c || k->anode == d)) ||
                      (k->cathode != NONE && (k->cathode == c || k->cathode == d));

        if (!touches) {
            continue;
        }
        if (d != NONE && k->anode == d && k->cathode == c) {
            n++;
        } else {
            return -1;
        }
    }
    return n;
}

static int any_capacitor_part_on(const Ctx *x, size_t net) {
    size_t i;

    if (net == NONE) {
        return 0;
    }
    for (i = 0; i < x->cap_count; i++) {
        if (x->caps[i].a == net || x->caps[i].b == net) {
            return 1;
        }
    }
    return 0;
}

/* ---- The steps -------------------------------------------------------------- */

static void stop(TimerState *out, TimerReason reason) {
    out->mode = TIMER_STOPPED;
    out->reason = reason;
}

static uint64_t period_ps(double k, uint64_t milliohms, uint64_t femtofarads) {
    return (uint64_t)llround(k * (double)milliohms * (double)femtofarads * 1e-6);
}

static void analyze(Ctx *x, unsigned section, TimerState *out) {
    const Part *part = x->parts[x->part];
    TimerPins pins;
    size_t t, r, d, o, k, c, i;
    uint64_t caps = 0;
    uint64_t rb = 0, ra = 0, r1 = 0;

    out->mode = TIMER_LATCH;
    out->reason = TIMER_OK;
    out->first_ps = out->high_ps = out->low_ps = 0;
    if (!timer_section_pins(part, section, &pins)) {
        return;
    }
    t = net_of(x, x->part, pins.thres);
    r = net_of(x, x->part, pins.trig);
    d = net_of(x, x->part, pins.disch);
    o = net_of(x, x->part, pins.out);
    k = pins.cont == SIZE_MAX ? NONE : net_of(x, x->part, pins.cont);

    /* 1: no capacitor on THRES, TRIG or DISCH: a latch. */
    if (!any_capacitor_part_on(x, t) && !any_capacitor_part_on(x, r) && !any_capacitor_part_on(x, d)) {
        return;
    }
    /* 2: the supply. */
    {
        size_t v = net_of(x, x->part, pins.vcc), g = net_of(x, x->part, pins.gnd);

        x->ref_mv = 0;
        if (v != NONE) {
            for (i = 0; i < x->conn_count; i++) {
                const Part *p = x->parts[x->conn[i].part];

                if (x->conn[i].net == v && p->kind == PART_SUPPLY && !p->supply->pull && p->supply->level == 1) {
                    x->ref_mv = p->supply->mv;
                    break;
                }
            }
        }
        if (x->ref_mv == 0 || !is_ground(x, g)) {
            stop(out, TIMER_NO_SUPPLY);
            return;
        }
    }
    /* The links depend on the reference voltage (a pull-up of another voltage is no supply). */
    if (build_links(x) != 0) {
        stop(out, TIMER_SHAPE);
        return;
    }
    /* 3: the CONT pin carries nothing but capacitors to ground. */
    if (k != NONE) {
        for (i = 0; i < x->conn_count; i++) {
            const Part *p;
            size_t j, ok = 0;

            if (x->conn[i].net != k || (x->conn[i].part == x->part && x->conn[i].pin == pins.cont)) {
                continue;
            }
            p = x->parts[x->conn[i].part];
            if (p->kind == PART_CAPACITOR) {
                for (j = 0; j < x->cap_count; j++) {
                    if (x->caps[j].part == x->conn[i].part &&
                        ((x->caps[j].a == k && is_ground(x, x->caps[j].b)) ||
                         (x->caps[j].b == k && is_ground(x, x->caps[j].a)))) {
                        ok = 1;
                    }
                }
            }
            if (!ok) {
                stop(out, TIMER_CONTROL);
                return;
            }
        }
    }
    /* 4: one of the four circuits. */
    c = NONE;
    if (t != NONE && t == r && d != NONE && d != t && !has_driver(x, t) && !has_driver(x, d) &&
        capacitors_clean(x, t, d) && capacitors_on(x, t, &caps) >= 1) {
        /* A and A-diode */
        size_t nb = links_between(x, t, d, &rb), na = links_between(x, d, RAIL_UP, &ra);
        int diodes = diodes_between(x, t, d);

        if (nb >= 1 && na >= 1 && links_on(x, t) == nb && links_on(x, d) == nb + na && diodes >= 0) {
            c = t;
            if (rb == 0 || ra == 0 || caps == 0) {
                stop(out, TIMER_NO_VALUE);
                return;
            }
            if (diodes > 0) {
                out->first_ps = period_ps(LN3, ra, caps);
                out->high_ps = period_ps(LN2, ra, caps);
            } else {
                out->first_ps = period_ps(LN3, ra + rb, caps);
                out->high_ps = period_ps(LN2, ra + rb, caps);
            }
            out->low_ps = period_ps(LN2, rb, caps);
            out->mode = TIMER_ASTABLE;
        }
    }
    if (c == NONE && t != NONE && t == r && d != t && o != NONE && !has_driver(x, t) &&
        capacitors_clean(x, t, NONE) && capacitors_on(x, t, &caps) >= 1 && diodes_between(x, t, NONE) == 0) {
        /* B: 50 % astable */
        size_t n1 = links_between(x, t, o, &r1);

        if (n1 >= 1 && links_on(x, t) == n1) {
            c = t;
            if (r1 == 0 || caps == 0) {
                stop(out, TIMER_NO_VALUE);
                return;
            }
            out->first_ps = period_ps(LN3, r1, caps);
            out->high_ps = period_ps(LN2, r1, caps);
            out->low_ps = out->high_ps;
            out->mode = TIMER_ASTABLE;
        }
    }
    if (c == NONE && t != NONE && t == d && r != t && !has_driver(x, t) && capacitors_clean(x, t, NONE) &&
        capacitors_on(x, t, &caps) >= 1 && diodes_between(x, t, NONE) == 0) {
        /* M: one-shot */
        size_t n1 = links_between(x, t, RAIL_UP, &r1);

        if (n1 >= 1 && links_on(x, t) == n1) {
            c = t;
            if (r1 == 0 || caps == 0) {
                stop(out, TIMER_NO_VALUE);
                return;
            }
            out->first_ps = 0;
            out->high_ps = period_ps(LN3, r1, caps);
            out->low_ps = 0;
            out->mode = TIMER_ONE_SHOT;
        }
    }
    if (c == NONE) {
        stop(out, TIMER_SHAPE);
        return;
    }
    /* 6: a time that rounds to nothing */
    if (out->high_ps == 0 || (out->mode == TIMER_ASTABLE && (out->low_ps == 0 || out->first_ps == 0))) {
        out->first_ps = out->high_ps = out->low_ps = 0;
        stop(out, TIMER_NO_VALUE);
        return;
    }
    /* 7: faster than the part allows */
    if (out->mode == TIMER_ASTABLE && out->high_ps + out->low_ps < 1000000000000ULL / part->timer->max_hz) {
        out->first_ps = out->high_ps = out->low_ps = 0;
        stop(out, TIMER_TOO_FAST);
        return;
    }
    if (out->mode == TIMER_ONE_SHOT && out->high_ps < 2 * part->timer->tpd_ps) {
        out->first_ps = out->high_ps = out->low_ps = 0;
        stop(out, TIMER_TOO_FAST);
    }
}

void timer_rc_analyze(const Part *const *parts, const uint64_t *values, size_t part_count,
                      const VoltageConnection *connections, size_t connection_count,
                      size_t part, unsigned section, TimerState *out) {
    Ctx x = {0};

    x.parts = parts;
    x.values = values;
    x.part_count = part_count;
    x.conn = connections;
    x.conn_count = connection_count;
    x.part = part;
    out->mode = TIMER_STOPPED;
    out->reason = TIMER_SHAPE;
    out->first_ps = out->high_ps = out->low_ps = 0;
    if (build(&x) == 0) {
        analyze(&x, section, out);
    }
    release(&x);
}
