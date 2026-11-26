#include "pio_via6522_int.h"
#include "logic_alloc.h"

static const char *const TIMING[] = {
    "tcyc", "tpwh", "tpwl", "tacr", "tcar", "tcdr", "thr", "tpcr", "tacw", "tcaw", "tdcw", "thw",
    "tcpw", "tca2", "trs1", "trs2", "twhs", "tds", "trs3", "trs4", "t21", "til", "tsr1", "tsr2",
    "tal", "tpdh", NULL
};

static const char *const KEYS[] = {
    "phi2", "rw", "reset", "data", "csHigh", "csLow", "rs", "pa", "pb",
    "ca1", "ca2", "cb1", "cb2", "irq", "irqOpenDrain", "busHold", NULL
};

/* ---- Three-valued helpers ------------------------------------------------------- */

LogicValue v22_bitv(uint8_t v, uint8_t k, int bit) {
    if (!(k >> bit & 1)) return LOGIC_X;
    return (v >> bit & 1) ? LOGIC_1 : LOGIC_0;
}

void v22_set_bit(uint8_t *v, uint8_t *k, int bit, LogicValue x) {
    uint8_t m = (uint8_t)(1u << bit);

    if (x == LOGIC_0 || x == LOGIC_1) {
        *k |= m;
        if (x == LOGIC_1) *v |= m; else *v &= (uint8_t)~m;
    } else {
        *k &= (uint8_t)~m;
        *v &= (uint8_t)~m;
    }
}

/* Sets the flags in mask (to 1, or X when x). */
void v22_flag_set(Via *s, uint8_t mask, int x) {
    if (x) {
        /* a flag already 1 stays 1 */
        uint8_t ones = s->ifr & s->ifr_k & mask;

        s->ifr_k &= (uint8_t)~(mask & ~ones);
        s->ifr &= s->ifr_k;
    } else {
        s->ifr |= mask;
        s->ifr_k |= mask;
    }
}

void v22_flag_clear(Via *s, uint8_t mask) {
    s->ifr &= (uint8_t)~mask;
    s->ifr_k |= mask;
}

/* Flags in mask that are not known 0 become X. */
void v22_flag_maybe_clear(Via *s, uint8_t mask) {
    uint8_t zeros = (uint8_t)(~s->ifr & s->ifr_k & mask);

    s->ifr_k &= (uint8_t)~(mask & ~zeros);
    s->ifr &= s->ifr_k;
}

/* ---- Pins -------------------------------------------------------------------------- */

LogicValue v22_pin_level(const Via *s, size_t pin) {
    LogicValue v = sim_read_pin(s->sim, s->instance, pin);

    if (v == LOGIC_Z) {
        return s->pins->device->bus_hold && pin < MAX_PINS ? s->held[pin] : LOGIC_X;
    }
    return v;
}

const size_t *v22_port_pins(const Via *s, int side) {
    return side == 0 ? s->pins->p.pa : s->pins->p.pb;
}

static size_t c1_pin(const Via *s, int side) {
    return side == 0 ? s->pins->p.ca1 : s->pins->p.cb1;
}

static size_t c2_pin(const Via *s, int side) {
    return side == 0 ? s->pins->p.ca2 : s->pins->p.cb2;
}

int v22_c2_mode(const Via *s, int side) {
    int shift = side == 0 ? 1 : 5;
    uint8_t m = (uint8_t)(7u << shift);

    if ((s->pcr_k & m) != m) return C2_UNKNOWN;
    return (s->pcr >> shift) & 7;
}

void v22_update_irq(Via *s) {
    uint8_t on = s->ifr & s->ifr_k & s->ier & s->ier_k & 0x7F;
    /* a bit could be on when neither factor is known 0 */
    uint8_t maybe = (uint8_t)((s->ifr | ~s->ifr_k) & (s->ier | ~s->ier_k) & 0x7F);
    LogicValue v;

    if (on) v = LOGIC_0;
    else if (maybe) v = LOGIC_X;
    else v = s->pins->device->irq_open_drain ? LOGIC_Z : LOGIC_1;
    sim_schedule_inertial_drive(s->sim, s->instance, s->pins->irq, v, DRIVE_STRONG, 0);
}

/* IFR bit 7 as read. */
LogicValue v22_irq_bit(const Via *s) {
    uint8_t on = s->ifr & s->ifr_k & s->ier & s->ier_k & 0x7F;
    uint8_t maybe = (uint8_t)((s->ifr | ~s->ifr_k) & (s->ier | ~s->ier_k) & 0x7F);

    return on ? LOGIC_1 : maybe ? LOGIC_X : LOGIC_0;
}

LogicValue v22_acr_bit(const Via *s, int bit) {
    return v22_bitv(s->acr, s->acr_k, bit);
}

void v22_drive_ports(Via *s, int side, SimTime delay) {
    const ViaSide *p = &s->side[side];
    const size_t *pins = v22_port_pins(s, side);
    int i;

    for (i = 0; i < PIO_PORT_BITS; i++) {
        LogicValue dir = v22_bitv(p->ddr, p->ddr_k, i);
        LogicValue v = dir == LOGIC_1 ? v22_bitv(p->out, p->out_k, i) : dir == LOGIC_0 ? LOGIC_Z : LOGIC_X;

        if (side == 1 && i == 7) {
            LogicValue t1 = v22_acr_bit(s, 7);

            if (t1 == LOGIC_1) v = s->pb7;
            else if (t1 == LOGIC_X) v = LOGIC_X;
        }
        sim_schedule_inertial_drive(s->sim, s->instance, pins[i], v, DRIVE_STRONG, delay);
    }
}

void v22_drive_pb7(Via *s, SimTime delay) {
    if (v22_acr_bit(s, 7) == LOGIC_1) {
        sim_schedule_inertial_drive(s->sim, s->instance, s->pins->p.pb[7], s->pb7, DRIVE_STRONG,
                                    delay);
    }
}

int v22_sr_mode(const Via *s) {
    if ((s->acr_k & 0x1C) != 0x1C) return -1;
    return (s->acr >> 2) & 7;
}

int v22_sr_internal(int m) {
    return m == SR_IN_T2 || m == SR_IN_PHI2 || m == SR_OUT_FREE || m == SR_OUT_T2 ||
           m == SR_OUT_PHI2;
}

int v22_sr_t2(int m) {
    return m == SR_IN_T2 || m == SR_OUT_FREE || m == SR_OUT_T2;
}

/* CB1: the shift clock output in the internal-clock modes, else an input. */
void v22_drive_cb1(Via *s, SimTime delay) {
    int m = v22_sr_mode(s);
    LogicValue v = m < 0 ? LOGIC_X : v22_sr_internal(m) ? s->cb1_out : LOGIC_Z;

    sim_schedule_inertial_drive(s->sim, s->instance, s->pins->p.cb1, v, DRIVE_STRONG, delay);
}

void v22_drive_c2(Via *s, int side, SimTime delay) {
    int mode = v22_c2_mode(s, side);
    LogicValue v;

    if (side == 1 && v22_sr_mode(s) != SR_OFF) {
        /* CB2 is the serial line. */
        int m = v22_sr_mode(s);

        v = m < 0 ? LOGIC_X : m >= SR_OUT_FREE ? s->cb2_sr : LOGIC_Z;
        sim_schedule_inertial_drive(s->sim, s->instance, c2_pin(s, side), v, DRIVE_STRONG, delay);
        return;
    }
    switch (mode) {
        case C2_HANDSHAKE:
        case C2_PULSE: v = s->side[side].c2_out; break;
        case C2_LOW: v = LOGIC_0; break;
        case C2_HIGH: v = LOGIC_1; break;
        case C2_UNKNOWN: v = LOGIC_X; break;
        default: v = LOGIC_Z; break;
    }
    sim_schedule_inertial_drive(s->sim, s->instance, c2_pin(s, side), v, DRIVE_STRONG, delay);
}

void v22_set_c2_out(Via *s, int side, LogicValue v, SimTime delay) {
    s->side[side].c2_out = v;
    v22_drive_c2(s, side, delay);
}

/* ---- Reset -------------------------------------------------------------------------- */

static void clear_side(ViaSide *p, int known) {
    uint8_t k = known ? 0xFF : 0;

    p->out = p->ddr = p->latch = 0;
    p->out_k = p->ddr_k = k;
    p->latch_k = 0;
    p->latched = 0;
    p->c2_out = known ? LOGIC_1 : LOGIC_X;
    p->arm = p->pulse_fall = p->pulse_rise = p->strobe_read = p->read_or = 0;
}

/* RESB low (known) or X (unknown); power-up is the unknown case with the
   timers and SR unknown too. */
static void reset_registers(Via *s, int known, int timers_too) {
    uint8_t k = known ? 0xFF : 0;
    int side;

    for (side = 0; side < 2; side++) clear_side(&s->side[side], known);
    s->acr = s->pcr = s->ifr = s->ier = 0;
    s->acr_k = s->pcr_k = k;
    s->ifr_k = s->ier_k = (uint8_t)(k & 0x7F);
    s->pending_clear = s->pending_x = 0;
    s->t1_run = s->t2_run = known ? 0 : -1;
    s->t1_loaded = s->t2_loaded = 0;
    s->t1_armed = s->t2_armed = known ? 0 : -1;
    s->t1_timeout = s->t1_reload = s->t2_timeout = 0;
    s->pb7 = known ? LOGIC_1 : LOGIC_X;
    s->sr_run = 0;
    s->sr_div = 0;
    s->sr_count = 0;
    s->cb1_out = s->cb2_sr = known ? LOGIC_1 : LOGIC_X;
    s->sr_sample_pending = s->sr_shift_pending = s->sr_access = s->sr_ext = 0;
    if (timers_too) {
        s->t1c.v = s->t1l.v = s->t2c.v = 0;
        s->t1c.k = s->t1l.k = s->t2c.k = 0;
        s->t2ll = s->t2ll_k = 0;
        s->sr = s->sr_k = 0;
    }
    for (side = 0; side < 2; side++) {
        v22_drive_ports(s, side, 0);
        v22_drive_c2(s, side, 0);
    }
    v22_drive_cb1(s, 0);
    v22_update_irq(s);
}

/* ---- Cycle hooks -------------------------------------------------------------------- */

/* Every PHI2 fall: read side effects, the CA2 read strobe, the timers. */
static void via_cycle(void *model) {
    Via *s = model;
    int side;

    if (s->reset != LOGIC_1) {
        s->side[0].read_or = s->side[1].read_or = 0;
        s->pending_clear = s->pending_x = 0;
        s->sr_access = 0;
        return;
    }
    for (side = 0; side < 2; side++) {
        ViaSide *p = &s->side[side];

        if (p->read_or) {
            s->pending_clear |= v22_or_access_flags(s, side);
        }
    }
    v22_flag_clear(s, s->pending_clear);
    v22_flag_maybe_clear(s, (uint8_t)(s->pending_x & ~s->pending_clear));

    /* CA2: the read handshake and pulse. */
    {
        ViaSide *a = &s->side[0];
        int mode = v22_c2_mode(s, 0);

        if (mode == C2_HANDSHAKE || mode == C2_PULSE) {
            if (a->read_or) {
                v22_set_c2_out(s, 0, LOGIC_0, s->t[T_TCA2]);
                a->strobe_read = 1;
                a->pulse_fall = mode == C2_PULSE;
            } else if (a->pulse_fall) {
                v22_set_c2_out(s, 0, LOGIC_1, s->t[T_TRS1]);
                a->pulse_fall = 0;
            }
        }
    }
    s->side[0].read_or = s->side[1].read_or = 0;
    s->pending_clear = s->pending_x = 0;

    v22_t1_fall(s);
    v22_t2_fall(s);
    v22_sr_fall(s);
    if (s->sr_access) v22_sr_start(s);
    v22_update_irq(s);
}

/* Every PHI2 rise: the write strobes, the time-outs, PB6 sampling. */
static void via_rise(Via *s) {
    SimTime now = sim_now(s->sim);
    int side;

    if (s->last_rise > 0 || s->period > 0) s->period = now - s->last_rise;
    s->last_rise = now;
    for (side = 0; side < 2; side++) {
        ViaSide *p = &s->side[side];
        int mode = v22_c2_mode(s, side);

        if (mode == C2_HANDSHAKE || mode == C2_PULSE) {
            if (p->arm) {
                v22_set_c2_out(s, side, LOGIC_0, s->t[T_TWHS]);
                p->strobe_read = 0;
                p->pulse_rise = mode == C2_PULSE;
            } else if (p->pulse_rise) {
                v22_set_c2_out(s, side, LOGIC_1, s->t[T_TRS3]);
                p->pulse_rise = 0;
            }
        }
        p->arm = 0;
    }
    v22_t1_rise(s);
    v22_t2_rise(s);
    v22_sr_rise(s);
}

static const Bus65Ops VIA_BUS = {v22_via_read, v22_via_write, via_cycle, v22_via_read_unknown};

/* ---- Control line inputs ------------------------------------------------------- */

static void latch_port(Via *s, int side) {
    ViaSide *p = &s->side[side];
    SimTime now = sim_now(s->sim);
    int i;

    p->latch = p->latch_k = 0;
    for (i = 0; i < PIO_PORT_BITS; i++) {
        LogicValue v = v22_pin_level(s, v22_port_pins(s, side)[i]);

        if (v22_recent(p, i, now, s->t[T_TIL])) v = LOGIC_X;
        v22_set_bit(&p->latch, &p->latch_k, i, v);
    }
    p->latched = 1;
    p->latch_at = now;
}

static void c1_edge(Via *s, int side, LogicValue old_v, LogicValue new_v) {
    ViaSide *p = &s->side[side];
    LogicValue edge = v22_bitv(s->pcr, s->pcr_k, side == 0 ? 0 : 4);
    uint8_t flag = side == 0 ? F_CA1 : F_CB1;
    int mode = v22_c2_mode(s, side);

    if (old_v == LOGIC_X || new_v == LOGIC_X || edge == LOGIC_X) {
        v22_flag_set(s, flag, 1);
        if (v22_acr_bit(s, side) != LOGIC_0) {
            p->latch_k = 0;
            p->latched = 1;
        }
        if (mode == C2_HANDSHAKE && p->c2_out != LOGIC_1) v22_set_c2_out(s, side, LOGIC_X, 0);
        v22_update_irq(s);
        return;
    }
    if ((new_v == LOGIC_1) != (edge == LOGIC_1)) {
        return;
    }
    v22_flag_set(s, flag, 0);
    if (v22_acr_bit(s, side) == LOGIC_1) {
        latch_port(s, side);
    } else if (v22_acr_bit(s, side) == LOGIC_X) {
        latch_port(s, side);
        p->latch_k = 0;
    }
    if (mode == C2_HANDSHAKE) {
        v22_set_c2_out(s, side, LOGIC_1, p->strobe_read ? s->t[T_TRS2] : s->t[T_TRS4]);
    }
    v22_update_irq(s);
}

static void c2_edge(Via *s, int side, LogicValue old_v, LogicValue new_v) {
    int mode = v22_c2_mode(s, side);
    uint8_t flag = side == 0 ? F_CA2 : F_CB2;
    int rising;

    if (side == 1 && v22_sr_mode(s) != SR_OFF) {
        return;   /* the serial line, not a PCR input */
    }

    if (mode == C2_UNKNOWN) {
        v22_flag_set(s, flag, 1);
        v22_update_irq(s);
        return;
    }
    if (mode >= C2_HANDSHAKE) {
        return;   /* an output */
    }
    rising = mode == C2_IN_POS || mode == C2_IND_POS;
    if (old_v == LOGIC_X || new_v == LOGIC_X) {
        v22_flag_set(s, flag, 1);
    } else if ((new_v == LOGIC_1) == rising) {
        v22_flag_set(s, flag, 0);
    } else {
        return;
    }
    v22_update_irq(s);
}

/* ---- Engine hooks ------------------------------------------------------------------ */

static void *via_create(const Part *part, const void *data, const void *pins,
                        const PioGrade *grade) {
    Via *s = logic_calloc(1, sizeof(*s));
    Bus65Timing timing;
    size_t i;

    (void)data;
    if (s == NULL) {
        return NULL;
    }
    s->base.model = &VIA6522_MODEL;
    s->pins = pins;
    s->t = grade->t;
    memset(&timing, 0, sizeof timing);
    timing.tacr = grade->t[T_TACR];
    timing.tcar = grade->t[T_TCAR];
    timing.tcdr = grade->t[T_TCDR];
    timing.thr = grade->t[T_THR];
    timing.tacw = grade->t[T_TACW];
    timing.tcaw = grade->t[T_TCAW];
    timing.tdcw = grade->t[T_TDCW];
    timing.thw = grade->t[T_THW];
    timing.tcyc = grade->t[T_TCYC];
    timing.tpwh = grade->t[T_TPWH];
    timing.tpwl = grade->t[T_TPWL];
    bus65_init(&s->bus, &s->pins->p.bus, &timing, &VIA_BUS, s);
    s->bus.bus_hold = s->pins->device->bus_hold;
    for (i = 0; i < MAX_PINS; i++) s->held[i] = LOGIC_X;
    (void)part;
    return s;
}

static void via_destroy(void *state) {
    logic_free(state);
}

static LogicValue reset_value(const Via *s, LogicValue v) {
    if (v == LOGIC_Z) {
        return s->pins->device->bus_hold ? s->held[s->pins->p.reset] : LOGIC_X;
    }
    return v;
}

static void via_start(Sim *sim, SimInstance *instance, void *state) {
    Via *s = state;
    int side;

    s->sim = sim;
    s->instance = instance;
    bus65_start(&s->bus, sim, instance);
    s->phi2 = sim_read_pin(sim, instance, s->pins->p.bus.phi2);
    s->reset = reset_value(s, sim_read_pin(sim, instance, s->pins->p.reset));
    for (side = 0; side < 2; side++) {
        s->side[side].c1 = v22_pin_level(s, c1_pin(s, side));
        s->side[side].c2 = v22_pin_level(s, c2_pin(s, side));
    }
    s->pb6_rise = LOGIC_X;
    reset_registers(s, 0, 1);
    if (s->reset == LOGIC_0) reset_registers(s, 1, 0);
}

static int port_bit(const Via *s, int side, size_t pin) {
    const size_t *pins = v22_port_pins(s, side);
    int i;

    for (i = 0; i < PIO_PORT_BITS; i++) {
        if (pins[i] == pin) return i;
    }
    return -1;
}

static void via_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                            LogicValue old_value, LogicValue new_value) {
    Via *s = state;
    const PioPins *p = &s->pins->p;
    int side;
    LogicValue v;

    s->sim = sim;
    s->instance = instance;
    if (pin < MAX_PINS && (new_value == LOGIC_0 || new_value == LOGIC_1)) {
        s->held[pin] = new_value;
    }
    if (pin == p->reset) {
        v = reset_value(s, new_value);
        if (v == s->reset) return;
        s->reset = v;
        if (v == LOGIC_0) reset_registers(s, 1, 0);
        else if (v == LOGIC_X) reset_registers(s, 0, 1);
        return;
    }
    if (bus65_pin_changed(&s->bus, sim, instance, pin, old_value, new_value)) {
        if (pin == p->bus.phi2) {
            LogicValue phi2 = new_value == LOGIC_Z ? LOGIC_X : new_value;

            if (s->phi2 == LOGIC_0 && phi2 == LOGIC_1 && s->reset == LOGIC_1) via_rise(s);
            s->phi2 = phi2;
        }
        return;
    }
    v = v22_pin_level(s, pin);
    for (side = 0; side < 2; side++) {
        ViaSide *ps = &s->side[side];
        int bit = port_bit(s, side, pin);

        if (bit >= 0) {
            SimTime now = sim_now(sim);

            ps->port_at[bit] = now;
            ps->port_seen[bit] = 1;
            /* Latch hold: a change within tpdh of the latching edge. */
            if (ps->latched && now - ps->latch_at < s->t[T_TPDH]) {
                v22_set_bit(&ps->latch, &ps->latch_k, bit, LOGIC_X);
            }
            if (side == 1 && bit == 6 && s->reset == LOGIC_1) v22_pb6_edge(s, v);
            return;
        }
        if (pin == c1_pin(s, side)) {
            LogicValue old_v = ps->c1;

            ps->c1 = v;
            if (s->reset == LOGIC_1 && old_v != v) {
                c1_edge(s, side, old_v, v);
                if (side == 1) {
                    int m = v22_sr_mode(s);

                    if ((m == SR_IN_EXT || m == SR_OUT_EXT) &&
                        (v == LOGIC_X || old_v == LOGIC_X)) {
                        s->sr = s->sr_k = 0;        /* an unknown clock edge */
                        s->sr_count = -1;
                    } else if ((m == SR_IN_EXT && v == LOGIC_1) ||
                               (m == SR_OUT_EXT && v == LOGIC_0)) {
                        s->sr_ext = 1;
                        s->sr_ext_at = sim_now(sim);
                    }
                }
            }
            return;
        }
        if (pin == c2_pin(s, side)) {
            LogicValue old_v = ps->c2;

            if (side == 1) {
                s->cb2_at = sim_now(sim);
                s->cb2_seen = 1;
            }

            ps->c2 = v;
            if (s->reset == LOGIC_1 && old_v != v) c2_edge(s, side, old_v, v);
            return;
        }
    }
}

static void via_timer(Sim *sim, SimInstance *instance, void *state, uint32_t timer_id) {
    Via *s = state;

    s->sim = sim;
    s->instance = instance;
    bus65_timer(&s->bus, sim, instance, timer_id);
}

/* ---- Loader --------------------------------------------------------------------- */

static void *via_load_device(KindContext *ctx, const cJSON *json) {
    ViaDevice *d = kind_alloc(ctx, sizeof(*d));

    if (d == NULL || pio_load_names(ctx, json, &d->names, 1, 1, 4) != 0 ||
        kind_string(ctx, json, "irq", &d->irq) != 0 ||
        kind_bool(ctx, json, "irqOpenDrain", &d->irq_open_drain) != 0 ||
        kind_bool(ctx, json, "busHold", &d->bus_hold) != 0) {
        return NULL;
    }
    return d;
}

static void *via_bind(KindContext *ctx, const void *data, const PartPin *pins, size_t pin_count) {
    const ViaDevice *d = data;
    ViaPins *out = kind_alloc(ctx, sizeof(*out));

    if (out == NULL) {
        return NULL;
    }
    if (pin_count > MAX_PINS) {
        kind_error(ctx, "a via6522 has at most %d pins", MAX_PINS);
        return NULL;
    }
    if (pio_bind_names(ctx, &d->names, pins, pin_count, &out->p) != 0 ||
        pio_bind_pin(ctx, pins, pin_count, "irq", d->irq, &out->irq) != 0) {
        return NULL;
    }
    out->device = d;
    return out;
}

static int via_pin_open_drain(const void *data, const void *pins, size_t pin) {
    const ViaDevice *d = data;
    const ViaPins *p = pins;

    return d->irq_open_drain && pin == p->irq;
}

const PioModel VIA6522_MODEL = {
    "via6522", KEYS, TIMING, 3,
    via_load_device, via_bind, via_pin_open_drain,
    via_create, via_destroy, via_start, via_pin_changed, via_timer
};
