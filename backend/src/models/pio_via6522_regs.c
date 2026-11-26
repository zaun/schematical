/* models/pio_via6522_regs.c: part of the model in models/pio_via6522.c (split by plan-68-I/J). */
#include "pio_via6522_int.h"
/* ---- Reads ------------------------------------------------------------------------- */

int v22_recent(const ViaSide *p, int bit, SimTime now, SimTime window) {
    return p->port_seen[bit] && now - p->port_at[bit] < window;
}

/* The pins of a port as read now (tpcr rule), one bit. */
static LogicValue port_pin_read(const Via *s, int side, int bit, SimTime now) {
    const ViaSide *p = &s->side[side];
    LogicValue v = v22_pin_level(s, v22_port_pins(s, side)[bit]);

    return v22_recent(p, bit, now, s->t[T_TPCR]) ? LOGIC_X : v;
}

static void read_input(Via *s, int side, uint8_t *value, uint8_t *known) {
    ViaSide *p = &s->side[side];
    LogicValue latching = v22_acr_bit(s, side);
    SimTime now = sim_now(s->sim);
    int i;

    *value = 0;
    *known = 0;
    for (i = 0; i < PIO_PORT_BITS; i++) {
        LogicValue v;

        if (side == 1) {
            LogicValue dir = v22_bitv(p->ddr, p->ddr_k, i);

            if (i == 7 && v22_acr_bit(s, 7) != LOGIC_0) {
                v22_set_bit(value, known, i, v22_acr_bit(s, 7) == LOGIC_1 ? s->pb7 : LOGIC_X);
                continue;
            }
            if (dir == LOGIC_1) {
                v22_set_bit(value, known, i, v22_bitv(p->out, p->out_k, i));
                continue;
            }
            if (dir == LOGIC_X) continue;
        }
        if (latching == LOGIC_1 && p->latched) {
            v = v22_bitv(p->latch, p->latch_k, i);
        } else if (latching == LOGIC_X && p->latched) {
            v = port_pin_read(s, side, i, now);
            if (v != v22_bitv(p->latch, p->latch_k, i)) v = LOGIC_X;
        } else {
            v = port_pin_read(s, side, i, now);
        }
        v22_set_bit(value, known, i, v);
    }
}

static void word_byte(Word w, int high, uint8_t *value, uint8_t *known) {
    *value = (uint8_t)(high ? w.v >> 8 : w.v);
    *known = (uint8_t)(high ? w.k >> 8 : w.k);
}

int v22_via_read(void *model, unsigned reg, uint8_t *value, uint8_t *known) {
    Via *s = model;

    switch (reg) {
        case R_ORB:
            read_input(s, 1, value, known);
            s->side[1].read_or = 1;
            break;
        case R_ORA:
        case R_ORA_NH:
            read_input(s, 0, value, known);
            if (reg == R_ORA) s->side[0].read_or = 1;
            s->side[0].latched = 0;   /* transparent until the next CA1 edge */
            break;
        case R_DDRB: *value = s->side[1].ddr; *known = s->side[1].ddr_k; break;
        case R_DDRA: *value = s->side[0].ddr; *known = s->side[0].ddr_k; break;
        case R_T1CL: word_byte(s->t1c, 0, value, known); s->pending_clear |= F_T1; break;
        case R_T1CH: word_byte(s->t1c, 1, value, known); break;
        case R_T1LL: word_byte(s->t1l, 0, value, known); break;
        case R_T1LH: word_byte(s->t1l, 1, value, known); break;
        case R_T2CL: word_byte(s->t2c, 0, value, known); s->pending_clear |= F_T2; break;
        case R_T2CH: word_byte(s->t2c, 1, value, known); break;
        case R_SR:
            *value = s->sr;
            *known = s->sr_k;
            s->pending_clear |= F_SR;
            s->sr_access = 1;
            break;
        case R_ACR: *value = s->acr; *known = s->acr_k; break;
        case R_PCR: *value = s->pcr; *known = s->pcr_k; break;
        case R_IFR:
            *value = s->ifr;
            *known = s->ifr_k;
            v22_set_bit(value, known, 7, v22_irq_bit(s));
            break;
        case R_IER:
            *value = (uint8_t)(s->ier | 0x80);
            *known = (uint8_t)(s->ier_k | 0x80);
            break;
    }
    if (reg == R_ORB) s->side[1].latched = 0;
    return 1;
}

void v22_via_read_unknown(void *model) {
    Via *s = model;

    /* Any read side effect may have happened. */
    s->pending_x |= F_CA1 | F_CA2 | F_CB1 | F_CB2 | F_T1 | F_T2 | F_SR;
    if (v22_c2_mode(s, 0) == C2_HANDSHAKE || v22_c2_mode(s, 0) == C2_PULSE) {
        v22_set_c2_out(s, 0, LOGIC_X, s->t[T_TCA2]);
    }
}

/* ---- Writes ------------------------------------------------------------------------ */

/* An ORA (RS 1) / ORB access: clears C1's flag and C2's unless independent. */
uint8_t v22_or_access_flags(const Via *s, int side) {
    int mode = v22_c2_mode(s, side);
    uint8_t c1 = side == 0 ? F_CA1 : F_CB1;
    uint8_t c2 = side == 0 ? F_CA2 : F_CB2;

    if (mode == C2_IND_NEG || mode == C2_IND_POS) return c1;
    return (uint8_t)(c1 | c2);
}

static void enter_c2_mode(Via *s, int side, int old_mode) {
    ViaSide *p = &s->side[side];
    int mode = v22_c2_mode(s, side);
    int out = mode == C2_HANDSHAKE || mode == C2_PULSE;

    if (out && old_mode != C2_HANDSHAKE && old_mode != C2_PULSE) {
        p->c2_out = LOGIC_1;
        p->arm = p->pulse_fall = p->pulse_rise = 0;
    } else if (!out) {
        p->arm = p->pulse_fall = p->pulse_rise = 0;
    }
    v22_drive_c2(s, side, 0);
}

static void word_set(Word *w, int high, uint8_t value, uint8_t known) {
    if (high) {
        w->v = (uint16_t)((w->v & 0x00FF) | (value & known) << 8);
        w->k = (uint16_t)((w->k & 0x00FF) | known << 8);
    } else {
        w->v = (uint16_t)((w->v & 0xFF00) | (value & known));
        w->k = (uint16_t)((w->k & 0xFF00) | known);
    }
}

static void write_or(Via *s, int side, uint8_t value, uint8_t known, int handshake) {
    ViaSide *p = &s->side[side];

    p->out = value & known;
    p->out_k = known;
    if (handshake) {
        if (known == 0) {
            v22_flag_maybe_clear(s, v22_or_access_flags(s, side));
        } else {
            v22_flag_clear(s, v22_or_access_flags(s, side));
        }
        if (v22_c2_mode(s, side) == C2_HANDSHAKE || v22_c2_mode(s, side) == C2_PULSE) {
            if (known == 0) v22_set_c2_out(s, side, LOGIC_X, 0);
            else p->arm = 1;
        }
    }
    v22_drive_ports(s, side, s->t[T_TCPW]);
}

static void t1_start(Via *s, int known) {
    s->t1c = s->t1l;
    s->t1_run = known ? 1 : -1;
    s->t1_loaded = 1;
    s->t1_armed = known ? 1 : -1;
    s->t1_timeout = s->t1_reload = 0;
    if (known) v22_flag_clear(s, F_T1); else v22_flag_maybe_clear(s, F_T1);
    if (v22_acr_bit(s, 7) != LOGIC_0) {
        s->pb7 = known ? LOGIC_0 : LOGIC_X;
        v22_drive_pb7(s, s->t[T_TCPW]);
    }
}

static void t2_start(Via *s, int known) {
    s->t2c.v = (uint16_t)((s->t2c.v & 0xFF00) | s->t2ll);
    s->t2c.k = (uint16_t)((s->t2c.k & 0xFF00) | s->t2ll_k);
    s->t2_run = known ? 1 : -1;
    s->t2_loaded = 1;
    s->t2_armed = known ? 1 : -1;
    s->t2_timeout = 0;
    if (known) v22_flag_clear(s, F_T2); else v22_flag_maybe_clear(s, F_T2);
}

void v22_via_write(void *model, unsigned reg, uint8_t value, uint8_t known) {
    Via *s = model;
    int full = known == 0xFF;

    if (s->reset != LOGIC_1) {
        return;   /* held in reset, or X: already unknown */
    }
    switch (reg) {
        case R_ORB: write_or(s, 1, value, known, 1); break;
        case R_ORA: write_or(s, 0, value, known, 1); break;
        case R_ORA_NH: write_or(s, 0, value, known, 0); break;
        case R_DDRB:
        case R_DDRA: {
            int side = reg == R_DDRB ? 1 : 0;

            s->side[side].ddr = value & known;
            s->side[side].ddr_k = known;
            v22_drive_ports(s, side, s->t[T_TCPW]);
            break;
        }
        case R_T1CL:
        case R_T1LL: word_set(&s->t1l, 0, value, known); break;
        case R_T1CH:
            word_set(&s->t1l, 1, value, known);
            t1_start(s, full);
            break;
        case R_T1LH:
            word_set(&s->t1l, 1, value, known);
            if (full) v22_flag_clear(s, F_T1); else v22_flag_maybe_clear(s, F_T1);
            break;
        case R_T2CL:
            s->t2ll = value & known;
            s->t2ll_k = known;
            break;
        case R_T2CH:
            word_set(&s->t2c, 1, value, known);
            t2_start(s, full);
            break;
        case R_SR:
            s->sr = value & known;
            s->sr_k = known;
            if (full) v22_flag_clear(s, F_SR); else v22_flag_maybe_clear(s, F_SR);
            s->sr_access = 1;
            break;
        case R_ACR: {
            int old_sr = v22_sr_mode(s);

            s->acr = value & known;
            s->acr_k = known;
            if (v22_sr_mode(s) != old_sr) {
                s->sr_run = 0;
                s->sr_div = 0;
                s->sr_count = 0;
                s->sr_sample_pending = s->sr_shift_pending = s->sr_ext = 0;
                s->cb1_out = LOGIC_1;
                if (v22_sr_mode(s) == SR_OUT_FREE) s->sr_run = 1;
                if (v22_sr_mode(s) == SR_OFF) v22_flag_clear(s, F_SR);
                else if (v22_sr_mode(s) < 0) v22_flag_maybe_clear(s, F_SR);
                v22_drive_cb1(s, 0);
                v22_drive_c2(s, 1, 0);
            }
            v22_drive_ports(s, 1, s->t[T_TCPW]);
            break;
        }
        case R_PCR: {
            int old_a = v22_c2_mode(s, 0);
            int old_b = v22_c2_mode(s, 1);

            s->pcr = value & known;
            s->pcr_k = known;
            enter_c2_mode(s, 0, old_a);
            enter_c2_mode(s, 1, old_b);
            break;
        }
        case R_IFR: {
            /* A 1 clears that flag; an unknown bit may. Bit 7 is ignored. */
            uint8_t ones = value & known & 0x7F;
            uint8_t x = (uint8_t)(~known & 0x7F);

            v22_flag_clear(s, ones);
            v22_flag_maybe_clear(s, x);
            break;
        }
        case R_IER: {
            uint8_t ones = value & known & 0x7F;
            uint8_t x = (uint8_t)(~known & 0x7F);
            LogicValue set = v22_bitv(value, known, 7);

            if (set == LOGIC_1) {
                s->ier |= ones;
                s->ier_k |= ones;
            } else if (set == LOGIC_0) {
                s->ier &= (uint8_t)~ones;
                s->ier_k |= ones;
            } else {
                x |= ones;
            }
            s->ier_k &= (uint8_t)~x;
            s->ier &= s->ier_k;
            break;
        }
    }
    v22_update_irq(s);
}

/* ---- Timers ------------------------------------------------------------------------- */

/* One PHI2 fall: decrement (wrap from 0 reports 1). */
static int word_dec(Word *w) {
    if (w->k != 0xFFFF) {
        w->v = 0;
        w->k = 0;
        return 0;
    }
    w->v = (uint16_t)(w->v - 1);
    return w->v == 0xFFFF;
}

void v22_t1_fall(Via *s) {
    if (s->t1_run == 0) {
        return;
    }
    if (s->t1_run < 0) {
        s->t1c.k = 0;
        s->t1c.v = 0;
        if (s->t1_armed != 0) v22_flag_set(s, F_T1, 1);
        if (v22_acr_bit(s, 7) != LOGIC_0) {
            s->pb7 = LOGIC_X;
            v22_drive_pb7(s, 0);
        }
        return;
    }
    if (s->t1_loaded) {
        s->t1_loaded = 0;
        return;
    }
    if (s->t1_reload) {
        s->t1c = s->t1l;
        s->t1_reload = 0;
        return;
    }
    if (s->t1c.k != 0xFFFF) {
        /* unknown count: the time-out is unknown */
        if (s->t1_armed != 0) v22_flag_set(s, F_T1, 1);
        if (v22_acr_bit(s, 7) != LOGIC_0) {
            s->pb7 = LOGIC_X;
            v22_drive_pb7(s, 0);
        }
        word_dec(&s->t1c);
        return;
    }
    if (word_dec(&s->t1c)) {
        s->t1_timeout = 1;
    }
}

void v22_t1_rise(Via *s) {
    LogicValue free_run = v22_acr_bit(s, 6);

    if (!s->t1_timeout) {
        return;
    }
    s->t1_timeout = 0;
    if (free_run == LOGIC_1) {
        v22_flag_set(s, F_T1, 0);
        s->t1_reload = 1;
        s->pb7 = s->pb7 == LOGIC_0 ? LOGIC_1 : s->pb7 == LOGIC_1 ? LOGIC_0 : LOGIC_X;
    } else if (free_run == LOGIC_0) {
        /* A time-out needs t1_run 1, so t1_armed is 1 or 0 here: every
           path that makes it -1 (t1_start unknown, reset unknown) also makes
           t1_run -1 and clears t1_timeout. */
        if (s->t1_armed == 1) {
            v22_flag_set(s, F_T1, 0);
            s->pb7 = LOGIC_1;
        }
        s->t1_armed = 0;
    } else {
        v22_flag_set(s, F_T1, 1);
        s->pb7 = LOGIC_X;
        s->t1_reload = 1;
    }
    v22_drive_pb7(s, 0);
    v22_update_irq(s);
}

void v22_t2_fall(Via *s) {
    LogicValue counting = v22_acr_bit(s, 5);

    if (s->t2_run == 0 || counting == LOGIC_1) {
        s->t2_loaded = 0;
        return;
    }
    if (s->t2_run < 0 || counting == LOGIC_X) {
        s->t2c.k = 0;
        s->t2c.v = 0;
        if (s->t2_armed != 0) v22_flag_set(s, F_T2, 1);
        return;
    }
    if (s->t2_loaded) {
        s->t2_loaded = 0;
        return;
    }
    if (s->t2c.k != 0xFFFF) {
        if (s->t2_armed != 0) v22_flag_set(s, F_T2, 1);
        word_dec(&s->t2c);
        return;
    }
    if (word_dec(&s->t2c)) {
        s->t2_timeout = 1;
    }
}

void v22_t2_rise(Via *s) {
    if (v22_acr_bit(s, 5) == LOGIC_1) {
        /* Pulse counting: a PB6 low at this rise after a high one counts. */
        LogicValue pb6 = v22_pin_level(s, s->pins->p.pb[6]);

        if (s->t2_run == 1 && s->pb6_rise == LOGIC_1 && pb6 == LOGIC_0) {
            if (s->t2c.k != 0xFFFF) {
                if (s->t2_armed != 0) v22_flag_set(s, F_T2, 1);
            } else {
                s->t2c.v = (uint16_t)(s->t2c.v - 1);
                if (s->t2c.v == 0 && s->t2_armed == 1) {
                    v22_flag_set(s, F_T2, 0);
                    s->t2_armed = 0;
                }
            }
        } else if (s->t2_run == 1 && (pb6 == LOGIC_X || s->pb6_rise == LOGIC_X) &&
                   s->pb6_rise != pb6) {
            s->t2c.k = 0;
            s->t2c.v = 0;
            if (s->t2_armed != 0) v22_flag_set(s, F_T2, 1);
        }
        s->pb6_rise = pb6;
        v22_update_irq(s);
        return;
    }
    s->pb6_rise = v22_pin_level(s, s->pins->p.pb[6]);
    if (!s->t2_timeout) {
        return;
    }
    s->t2_timeout = 0;
    if (v22_sr_t2(v22_sr_mode(s))) {
        return;   /* T2 times the shift clock: no IFR5 */
    }
    /* t2_armed is 1 or 0 here, as for T1 (t2_start, reset unknown). */
    if (s->t2_armed == 1) {
        v22_flag_set(s, F_T2, 0);
    }
    s->t2_armed = 0;
    v22_update_irq(s);
}

/* A PB6 edge in pulse-counting mode: a pulse narrower than two PHI2
   cycles (tipw), or one that starts less than two cycles after the last
   ended (tips), makes the count unknown. */
void v22_pb6_edge(Via *s, LogicValue v) {
    SimTime now = sim_now(s->sim);
    SimTime min = 2 * (s->period > 0 ? s->period : s->t[T_TCYC]);
    int bad = 0;

    if (v == LOGIC_0) {
        bad = s->pb6_seen_high && now - s->pb6_high_at < min;
        s->pb6_fall_at = now;
        s->pb6_seen_fall = 1;
    } else if (v == LOGIC_1) {
        bad = s->pb6_seen_fall && now - s->pb6_fall_at < min;
        s->pb6_high_at = now;
        s->pb6_seen_high = 1;
    }
    if (bad && v22_acr_bit(s, 5) == LOGIC_1 && s->t2_run == 1) {
        s->t2c.k = 0;
        s->t2c.v = 0;
    }
}

/* ---- Shift register ------------------------------------------------------------------ */

static void sr_shift_in(Via *s, LogicValue bit) {
    s->sr = (uint8_t)(s->sr << 1);
    s->sr_k = (uint8_t)(s->sr_k << 1 | 1);
    v22_set_bit(&s->sr, &s->sr_k, 0, bit);
}

/* Bit 7 out on CB2 (tsr1 after this fall), rotated into bit 0. */
static void sr_shift_out(Via *s) {
    LogicValue b = v22_bitv(s->sr, s->sr_k, 7);

    s->sr = (uint8_t)(s->sr << 1);
    s->sr_k = (uint8_t)(s->sr_k << 1 | 1);
    v22_set_bit(&s->sr, &s->sr_k, 0, b);
    s->cb2_sr = b;
    v22_drive_c2(s, 1, s->t[T_TSR1]);
}

/* One bit done; 8 sets IFR2. Returns 1 at the 8th. */
static int sr_count_bit(Via *s) {
    if (s->sr_count < 0) {
        v22_flag_set(s, F_SR, 1);
        return 0;
    }
    if (++s->sr_count < 8) {
        return 0;
    }
    v22_flag_set(s, F_SR, 0);
    return 1;
}

static void sr_toggle(Via *s, int m) {
    if (s->cb1_out != LOGIC_0) {
        s->cb1_out = LOGIC_0;
        v22_drive_cb1(s, 0);
        if (m >= SR_OUT_FREE) sr_shift_out(s);
        return;
    }
    s->cb1_out = LOGIC_1;
    v22_drive_cb1(s, 0);
    if (m < SR_OUT_FREE) {
        s->sr_sample_pending = 1;
    } else if (m != SR_OUT_FREE && sr_count_bit(s)) {
        s->sr_run = 0;            /* 8 out: CB1 stays high, CB2 holds */
    }
}

/* Every PHI2 fall. */
void v22_sr_fall(Via *s) {
    int m = v22_sr_mode(s);

    if (m == SR_OFF) {
        return;
    }
    if (m < 0) {
        s->sr = s->sr_k = 0;
        return;
    }
    if (s->sr_shift_pending) {
        s->sr_shift_pending = 0;
        sr_shift_in(s, s->sr_sample);
        if (sr_count_bit(s)) {
            s->sr_run = 0;        /* 8 in: CB1 stays high */
            return;
        }
    }
    if (m == SR_IN_EXT || m == SR_OUT_EXT) {
        if (!s->sr_ext) return;
        s->sr_ext = 0;
        if (sim_now(s->sim) - s->sr_ext_at < s->t[T_TPCR]) {
            /* An edge too close to PHI2 low: shifted or not, unknown (p.19). */
            s->sr = s->sr_k = 0;
            s->sr_count = -1;
            if (m == SR_OUT_EXT) {
                s->cb2_sr = LOGIC_X;
                v22_drive_c2(s, 1, s->t[T_TSR1]);
            }
            v22_flag_set(s, F_SR, 1);
            return;
        }
        if (m == SR_IN_EXT) {
            LogicValue v = v22_pin_level(s, s->pins->p.cb2);

            sr_shift_in(s, v);
        } else {
            sr_shift_out(s);
        }
        if (sr_count_bit(s)) s->sr_count = 0;   /* keeps shifting */
        return;
    }
    if (!s->sr_run) {
        return;
    }
    if (v22_sr_t2(m)) {
        unsigned n = s->t2ll_k == 0xFF ? s->t2ll : 0;

        if (++s->sr_div < n + 2) return;
        s->sr_div = 0;
        s->t2c.v = (uint16_t)((s->t2c.v & 0xFF00) | s->t2ll);
        s->t2c.k = (uint16_t)((s->t2c.k & 0xFF00) | s->t2ll_k);
    }
    sr_toggle(s, m);
}

/* An SR access, at its PHI2 fall: restart the count and the clock. */
void v22_sr_start(Via *s) {
    int m = v22_sr_mode(s);

    s->sr_access = 0;
    if (m <= SR_OFF) {
        return;
    }
    s->sr_count = 0;
    s->sr_sample_pending = s->sr_shift_pending = 0;
    if (!v22_sr_internal(m)) {
        return;
    }
    s->sr_run = 1;
    s->sr_div = 0;
    s->cb1_out = LOGIC_1;
    if (m == SR_IN_PHI2 || m == SR_OUT_PHI2) {
        sr_toggle(s, m);          /* CB1 low from the cycle after the access */
    } else {
        v22_drive_cb1(s, 0);
    }
}

/* Every PHI2 rise: an input sample after a CB1 rise. */
void v22_sr_rise(Via *s) {
    LogicValue v;

    if (!s->sr_sample_pending) {
        return;
    }
    s->sr_sample_pending = 0;
    v = v22_pin_level(s, s->pins->p.cb2);
    if (s->cb2_seen && sim_now(s->sim) - s->cb2_at < s->t[T_TSR2]) v = LOGIC_X;
    s->sr_sample = v;
    s->sr_shift_pending = 1;
}

