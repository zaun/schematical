/*
 * bus65: the 65xx peripheral bus (bus65.h, plan-14-A section 4.3).
 */

#include "bus65.h"

#include <string.h>

/* Control slots: active-high selects, active-low selects, RWB, the RS pins. */
enum {
    SLOT_CS_HIGH = 0,
    SLOT_CS_LOW = BUS65_MAX_CS,
    SLOT_RWB = 2 * BUS65_MAX_CS,
    SLOT_RS = SLOT_RWB + 1,
    SLOT_COUNT = SLOT_RS + BUS65_MAX_RS,
    SLOT_DATA = SLOT_COUNT           /* held[] only: the data pins follow the controls */
};

/* Timer ids: BUS65_TIMER | kind | seq. */
#define TIMER_RELEASE 0x20000000u
#define TIMER_SEQ 0x1FFFFFFFu

static SimTime max_time(SimTime a, SimTime b) {
    return a > b ? a : b;
}

void bus65_init(Bus65 *bus, const Bus65Pins *pins, const Bus65Timing *timing,
                const Bus65Ops *ops, void *model) {
    size_t i;

    memset(bus, 0, sizeof(*bus));
    bus->pins = pins;
    bus->timing = *timing;
    bus->ops = ops;
    bus->model = model;
    bus->phi2 = LOGIC_X;
    bus->cycle = BUS65_IDLE;
    for (i = 0; i < sizeof bus->held / sizeof bus->held[0]; i++) {
        bus->held[i] = LOGIC_X;
    }
}

unsigned bus65_reg_count(const Bus65 *bus) {
    return 1u << bus->pins->rs_count;
}

/* The slot of a control pin, or -1. */
static int control_slot(const Bus65 *bus, size_t pin) {
    const Bus65Pins *p = bus->pins;
    size_t i;

    for (i = 0; i < p->cs_high_count; i++) {
        if (p->cs_high[i] == pin) return (int)(SLOT_CS_HIGH + i);
    }
    for (i = 0; i < p->cs_low_count; i++) {
        if (p->cs_low[i] == pin) return (int)(SLOT_CS_LOW + i);
    }
    if (p->rwb == pin) return SLOT_RWB;
    for (i = 0; i < p->rs_count; i++) {
        if (p->rs[i] == pin) return (int)(SLOT_RS + i);
    }
    return -1;
}

static int data_bit(const Bus65 *bus, size_t pin) {
    int i;

    for (i = 0; i < BUS65_DATA; i++) {
        if (bus->pins->data[i] == pin) return i;
    }
    return -1;
}

static size_t slot_pin(const Bus65 *bus, int slot) {
    const Bus65Pins *p = bus->pins;

    if (slot < SLOT_CS_LOW) return p->cs_high[slot - SLOT_CS_HIGH];
    if (slot < SLOT_RWB) return p->cs_low[slot - SLOT_CS_LOW];
    if (slot == SLOT_RWB) return p->rwb;
    return p->rs[slot - SLOT_RS];
}

/* A pin's level through its held slot (-1 = none). */
static LogicValue held_level(const Bus65 *bus, const Sim *sim, const SimInstance *instance,
                             size_t pin, int held) {
    LogicValue v = sim_read_pin(sim, instance, pin);

    if (v == LOGIC_Z) {
        return bus->bus_hold && held >= 0 ? bus->held[held] : LOGIC_X;
    }
    return v;
}

LogicValue bus65_level(const Bus65 *bus, const Sim *sim, const SimInstance *instance, size_t pin) {
    int slot = control_slot(bus, pin);
    int bit = data_bit(bus, pin);

    return held_level(bus, sim, instance, pin,
                      slot >= 0 ? slot : bit >= 0 ? SLOT_DATA + bit : -1);
}

static LogicValue slot_level(const Bus65 *bus, const Sim *sim, const SimInstance *instance,
                             int slot) {
    return held_level(bus, sim, instance, slot_pin(bus, slot), slot);
}

static void drive_data(Bus65 *bus, Sim *sim, SimInstance *instance, int known_value) {
    int i;

    for (i = 0; i < BUS65_DATA; i++) {
        LogicValue v = LOGIC_X;

        if (known_value && (bus->read_known >> i & 1)) {
            v = (bus->read_value >> i & 1) ? LOGIC_1 : LOGIC_0;
        }
        sim_schedule_inertial_drive(sim, instance, bus->pins->data[i], v, DRIVE_STRONG, 0);
    }
}

static void release_data(Bus65 *bus, Sim *sim, SimInstance *instance) {
    int i;

    for (i = 0; i < BUS65_DATA; i++) {
        sim_schedule_inertial_drive(sim, instance, bus->pins->data[i], LOGIC_Z, DRIVE_STRONG, 0);
    }
    bus->driving = 0;
}

/* Starts a drive window: X now, the read's value (or X) at rise + tcdr. */
static void start_drive(Bus65 *bus, Sim *sim, SimInstance *instance, SimTime valid_in) {
    bus->seq = (bus->seq + 1) & TIMER_SEQ;
    bus->driving = 1;
    drive_data(bus, sim, instance, 0);
    sim_schedule_timer(sim, instance, BUS65_TIMER | bus->seq, valid_in);
}

static void all_unknown(Bus65 *bus) {
    unsigned r;
    unsigned n = bus65_reg_count(bus);

    for (r = 0; r < n; r++) {
        bus->ops->write(bus->model, r, 0, 0);
    }
}

/* 1 = active, 0 = inactive, X. */
static LogicValue select_active(int slot, LogicValue v) {
    if (v != LOGIC_0 && v != LOGIC_1) return LOGIC_X;
    if (slot >= SLOT_CS_LOW) return v == LOGIC_0 ? LOGIC_1 : LOGIC_0;
    return v;
}

/* The selection from the sampled selects: IDLE (some select inactive), UNKNOWN, or READ (selected). */
static Bus65Cycle selection(const Bus65 *bus) {
    const Bus65Pins *p = bus->pins;
    int any_x = 0;
    size_t i;

    for (i = 0; i < p->cs_high_count + p->cs_low_count; i++) {
        int slot = i < p->cs_high_count ? (int)(SLOT_CS_HIGH + i)
                                        : (int)(SLOT_CS_LOW + i - p->cs_high_count);
        LogicValue a = bus->cs_value[slot];

        if (a == LOGIC_0) return BUS65_IDLE;
        any_x |= a == LOGIC_X;
    }
    return any_x ? BUS65_UNKNOWN : BUS65_READ;
}

/* 1 when the control slot changed less than `setup` before now (or at now). */
static int too_recent(const Bus65 *bus, int slot, SimTime now, SimTime setup) {
    return bus->control_seen[slot] && setup > 0 && now - bus->control_at[slot] < setup;
}

static void phi2_rise(Bus65 *bus, Sim *sim, SimInstance *instance) {
    const Bus65Pins *p = bus->pins;
    const Bus65Timing *t = &bus->timing;
    SimTime now = sim_now(sim);
    int violation = 0;
    LogicValue rwb;
    SimTime setup;
    size_t i;

    if ((bus->edges & 2) && t->tpwl > 0 && now - bus->last_fall < t->tpwl) violation = 1;
    if ((bus->edges & 1) && t->tcyc > 0 && now - bus->last_rise < t->tcyc) violation = 1;
    bus->last_rise = now;
    bus->edges |= 1;
    bus->wrote = 0;

    rwb = slot_level(bus, sim, instance, SLOT_RWB);
    setup = rwb == LOGIC_1 ? t->tacr : rwb == LOGIC_0 ? t->tacw : max_time(t->tacr, t->tacw);
    bus->rwb = rwb == LOGIC_1 ? 1 : rwb == LOGIC_0 ? 0 : -1;
    if (too_recent(bus, SLOT_RWB, now, setup)) bus->rwb = -1;

    for (i = 0; i < p->cs_high_count; i++) {
        int slot = (int)(SLOT_CS_HIGH + i);
        LogicValue a = select_active(slot, slot_level(bus, sim, instance, slot));

        bus->cs_value[slot] = too_recent(bus, slot, now, setup) ? LOGIC_X : a;
    }
    for (i = 0; i < p->cs_low_count; i++) {
        int slot = (int)(SLOT_CS_LOW + i);
        LogicValue a = select_active(slot, slot_level(bus, sim, instance, slot));

        bus->cs_value[slot] = too_recent(bus, slot, now, setup) ? LOGIC_X : a;
    }

    bus->cycle = selection(bus);
    if (bus->cycle == BUS65_READ) {
        unsigned reg = 0;
        int known = bus->rwb >= 0;

        for (i = 0; i < p->rs_count; i++) {
            int slot = (int)(SLOT_RS + i);
            LogicValue v = slot_level(bus, sim, instance, slot);

            if ((v != LOGIC_0 && v != LOGIC_1) || too_recent(bus, slot, now, setup)) known = 0;
            if (v == LOGIC_1) reg |= 1u << i;
        }
        bus->reg = reg;
        bus->cycle = !known ? BUS65_UNKNOWN : bus->rwb == 1 ? BUS65_READ : BUS65_WRITE;
    }
    if (violation) {
        all_unknown(bus);
        if (bus->cycle != BUS65_IDLE) bus->cycle = BUS65_UNKNOWN;
    }

    if (bus->cycle == BUS65_READ) {
        bus->read_value = 0;
        bus->read_known = 0;
        if (bus->ops->read(bus->model, bus->reg, &bus->read_value, &bus->read_known)) {
            bus->read_valid = 1;
            start_drive(bus, sim, instance, t->tcdr);
        }
    } else if (bus->cycle == BUS65_UNKNOWN && bus->rwb != 0) {
        bus->read_valid = 0;
        start_drive(bus, sim, instance, t->tcdr);
        if (bus->ops->read_unknown != NULL) bus->ops->read_unknown(bus->model);
    }
}

static void phi2_fall(Bus65 *bus, Sim *sim, SimInstance *instance) {
    const Bus65Timing *t = &bus->timing;
    SimTime now = sim_now(sim);
    int i;

    if ((bus->edges & 1) && t->tpwh > 0 && now - bus->last_rise < t->tpwh) {
        all_unknown(bus);
        if (bus->cycle != BUS65_IDLE) bus->cycle = BUS65_UNKNOWN;
    }
    bus->last_fall = now;
    bus->edges |= 2;

    if (bus->cycle == BUS65_WRITE) {
        uint8_t value = 0;
        uint8_t known = 0;

        for (i = 0; i < BUS65_DATA; i++) {
            LogicValue v = held_level(bus, sim, instance, bus->pins->data[i], SLOT_DATA + i);
            int late = bus->data_seen[i] && t->tdcw > 0 && now - bus->data_at[i] < t->tdcw;

            if ((v == LOGIC_0 || v == LOGIC_1) && !late) {
                known |= (uint8_t)(1u << i);
                if (v == LOGIC_1) value |= (uint8_t)(1u << i);
            }
        }
        bus->ops->write(bus->model, bus->reg, value, known);
        bus->wrote = 1;
        bus->write_value = value;
        bus->write_known = known;
    } else if (bus->cycle == BUS65_UNKNOWN && bus->rwb != 1) {
        all_unknown(bus);
    }
    if (bus->driving) {
        sim_schedule_timer(sim, instance, BUS65_TIMER | TIMER_RELEASE | bus->seq, t->thr);
    }
    bus->cycle = BUS65_IDLE;
    if (bus->ops->cycle != NULL) {
        bus->ops->cycle(bus->model);
    }
}

/* PHI2 went X (or Z): every register unknown, the data X until it is known. */
static void phi2_unknown(Bus65 *bus, Sim *sim, SimInstance *instance) {
    all_unknown(bus);
    bus->seq = (bus->seq + 1) & TIMER_SEQ;
    bus->driving = 1;
    bus->read_valid = 0;
    drive_data(bus, sim, instance, 0);
    bus->cycle = BUS65_IDLE;
    bus->wrote = 0;
    bus->edges = 0;
}

/* A control changed while PHI2 is high: the hold check (rule 4). */
static void control_hold(Bus65 *bus, Sim *sim, SimInstance *instance, int slot) {
    const Bus65Timing *t = &bus->timing;
    SimTime now = sim_now(sim);
    SimTime hold;
    Bus65Cycle was = bus->cycle;

    if (bus->phi2 != LOGIC_1 || !(bus->edges & 1)) {
        return;
    }
    hold = was == BUS65_READ ? t->tcar : was == BUS65_WRITE ? t->tcaw : max_time(t->tcar, t->tcaw);
    if (now != bus->last_rise && now - bus->last_rise >= hold) {
        return;
    }
    if (was == BUS65_IDLE) {
        /* Not selected: only a select change can matter, and only when no
           other select was inactive. */
        if (slot >= SLOT_RWB) {
            return;
        }
        bus->cs_value[slot] = LOGIC_X;
        if (selection(bus) == BUS65_IDLE) {
            return;
        }
    }
    if (slot == SLOT_RWB) {
        bus->rwb = -1;
    }
    bus->cycle = BUS65_UNKNOWN;
    if (bus->rwb != 0) {
        /* A read may be in progress: X until fall + thr. */
        bus->read_valid = 0;
        if (!bus->driving) {
            bus->seq = (bus->seq + 1) & TIMER_SEQ;
            bus->driving = 1;
        }
        drive_data(bus, sim, instance, 0);
        if (bus->ops->read_unknown != NULL) bus->ops->read_unknown(bus->model);
    }
}

/* A data pin changed: the write hold check. */
static void data_hold(Bus65 *bus, Sim *sim, int bit) {
    SimTime now = sim_now(sim);
    uint8_t mask = (uint8_t)(1u << bit);

    if (!bus->wrote || !(bus->write_known & mask) || bus->phi2 != LOGIC_0) {
        return;
    }
    if (now != bus->last_fall && now - bus->last_fall >= bus->timing.thw) {
        return;
    }
    bus->write_known &= (uint8_t)~mask;
    bus->ops->write(bus->model, bus->reg, bus->write_value, bus->write_known);
}

void bus65_start(Bus65 *bus, Sim *sim, SimInstance *instance) {
    int slot;
    int i;

    for (slot = 0; slot < SLOT_COUNT; slot++) {
        int used = (slot < SLOT_CS_LOW && (size_t)(slot - SLOT_CS_HIGH) < bus->pins->cs_high_count) ||
                   (slot >= SLOT_CS_LOW && slot < SLOT_RWB &&
                    (size_t)(slot - SLOT_CS_LOW) < bus->pins->cs_low_count) ||
                   slot == SLOT_RWB ||
                   (slot >= SLOT_RS && (size_t)(slot - SLOT_RS) < bus->pins->rs_count);
        LogicValue v;

        if (!used) continue;
        v = sim_read_pin(sim, instance, slot_pin(bus, slot));
        if (v == LOGIC_0 || v == LOGIC_1) bus->held[slot] = v;
    }
    for (i = 0; i < BUS65_DATA; i++) {
        LogicValue v = sim_read_pin(sim, instance, bus->pins->data[i]);

        if (v == LOGIC_0 || v == LOGIC_1) bus->held[SLOT_DATA + i] = v;
    }
    bus->phi2 = sim_read_pin(sim, instance, bus->pins->phi2);
    if (bus->phi2 == LOGIC_Z) bus->phi2 = LOGIC_X;
}

int bus65_pin_changed(Bus65 *bus, Sim *sim, SimInstance *instance, size_t pin,
                      LogicValue old_value, LogicValue new_value) {
    int slot;
    int bit;

    (void)old_value;
    if (pin == bus->pins->phi2) {
        LogicValue v = new_value == LOGIC_Z ? LOGIC_X : new_value;
        LogicValue was = bus->phi2;

        bus->phi2 = v;
        if (v == was) {
            return 1;
        }
        if (v == LOGIC_X) {
            phi2_unknown(bus, sim, instance);
        } else if (was == LOGIC_X) {
            /* Known again: no edge; the data pins float. */
            bus->seq = (bus->seq + 1) & TIMER_SEQ;
            release_data(bus, sim, instance);
        } else if (v == LOGIC_1) {
            phi2_rise(bus, sim, instance);
        } else {
            phi2_fall(bus, sim, instance);
        }
        return 1;
    }
    slot = control_slot(bus, pin);
    if (slot >= 0) {
        bus->control_at[slot] = sim_now(sim);
        bus->control_seen[slot] = 1;
        if (new_value == LOGIC_0 || new_value == LOGIC_1) bus->held[slot] = new_value;
        control_hold(bus, sim, instance, slot);
        return 1;
    }
    bit = data_bit(bus, pin);
    if (bit >= 0) {
        bus->data_at[bit] = sim_now(sim);
        bus->data_seen[bit] = 1;
        if (new_value == LOGIC_0 || new_value == LOGIC_1) bus->held[SLOT_DATA + bit] = new_value;
        data_hold(bus, sim, bit);
        return 1;
    }
    return 0;
}

int bus65_timer(Bus65 *bus, Sim *sim, SimInstance *instance, uint32_t timer_id) {
    uint32_t seq = timer_id & TIMER_SEQ;

    if ((timer_id & BUS65_TIMER_MASK) != BUS65_TIMER) {
        return 0;
    }
    if (seq != bus->seq || !bus->driving) {
        return 1;
    }
    if (timer_id & TIMER_RELEASE) {
        release_data(bus, sim, instance);
    } else {
        drive_data(bus, sim, instance, bus->read_valid);
    }
    return 1;
}
