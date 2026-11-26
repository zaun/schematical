/* models/ns16550_exar_pins.c: part of the model in models/ns16550_exar.c (split by plan-68-I/J). */
#include "ns16550_exar_int.h"
/* ---- Pins ----------------------------------------------------------------------------- */

LogicValue ns_x_rts(const NsChannel *c, LogicValue normal) {
    const NsDevice *d = c->owner->dev;

    /* RS-485 on RTS: high while transmitting (XR16M752 DLD[6]; 5G FCTR[5],
       on DTR instead when the XR16V798's MCR[2] = 1). */
    if (exs_rs485_enabled(c) && (d->rs485 == RS485_DLD6 || d->rs485 == RS485_FCTR_RTS ||
                             (d->rs485 == RS485_FCTR_MCR2 && !exs_kb(c->mcr, 2)))) {
        return bool3(c->xc.rs485);
    }
    if (exs_auto_rts_on(c) && !exs_flow_on_dtr(c) && c->rts_off) {
        return LOGIC_1;
    }
    return normal;
}

LogicValue ns_x_dtr(const NsChannel *c, LogicValue normal) {
    const NsDevice *d = c->owner->dev;

    if (exs_rs485_enabled(c) && d->rs485 == RS485_FCTR_MCR2 && exs_kb(c->mcr, 2)) {
        return bool3(c->xc.rs485);
    }
    if (exs_auto_rts_on(c) && exs_flow_on_dtr(c) && c->rts_off) {
        return LOGIC_1;
    }
    return normal;
}

/* XR16C864 FCTR[3]: OP1 is the RS-485 direction, low while transmitting. */
LogicValue ns_x_out1(const NsChannel *c, LogicValue normal) {
    if (c->owner->dev->rs485 == RS485_FCTR3_OP1 && exs_rs485_enabled(c)) {
        return bool3(!c->xc.rs485);
    }
    return normal;
}

static int irda_on(const NsChannel *c) {
    switch (c->owner->dev->irda) {
        case IRDA_MCR6:
        case IRDA_MCR6_EXAR: return exs_kb(c->mcr, 6);
        case IRDA_DLD7: return exs_kb(c->xc.dld, 7);
        default: return 0;
    }
}

/* SIR: a 0 bit is a 3/16-bit pulse from the 7th to the 10th 16x clock; idle low. */
static LogicValue ir_encode(const NsChannel *c, LogicValue normal) {
    unsigned f;
    unsigned at;

    if (normal == LOGIC_1) {
        return LOGIC_0;
    }
    if (normal == LOGIC_X) {
        return LOGIC_X;
    }
    if (!serial_tx_busy(&c->line.tx)) {
        return LOGIC_0;
    }
    f = c->line.tx.format.factor;
    at = c->line.tx.count;
    return bool3(at >= 7 * f / 16 && at < 10 * f / 16);
}

LogicValue ns_x_tx_pin(const NsChannel *c, LogicValue normal) {
    return irda_on(c) ? ir_encode(c, normal) : normal;
}

/* RX: the IrDA polarity (MCR[2] on the XR16L580/M/V parts, FCTR[2] on the 854,
   FCTR[4] on the 5G), echo suppression (5G IrDA) and RX disable (XR16V798). */
static int ir_inverted(const NsChannel *c) {
    switch (exs_style_of(c)) {
        case XS_854: return exs_kb(c->xc.fctr, 2);
        case XS_5G: return exs_kb(c->xc.fctr, 4);
        default: return exs_xd(c)->irda_polarity && exs_kb(c->mcr, 2);
    }
}

LogicValue ns_x_rx_decode(NsChannel *c, LogicValue pin) {
    LogicValue level;

    if (exs_style_of(c) == XS_5G) {
        if (exs_kb(c->xc.msrw, 2) && !serial_rx_busy(&c->line.rx) && c->xc.rx_allow == 0) {
            return LOGIC_1;   /* RX disabled */
        }
        if (irda_on(c) && serial_tx_busy(&c->line.tx)) {
            return LOGIC_1;   /* the receiver is off while IrDA transmits */
        }
    }
    if (!irda_on(c)) {
        return pin;
    }
    if (ir_inverted(c)) {
        pin = not3(pin);
    }
    /* A pulse's falling edge is a 0 for one bit. */
    if (c->ir_last == LOGIC_1 && pin == LOGIC_0) {
        c->ir_low = c->format.factor;
    }
    c->ir_last = pin;
    level = c->ir_low > 0 ? LOGIC_0 : LOGIC_1;
    if (c->ir_low > 0) {
        c->ir_low--;
    }
    return level;
}

void ns_x_pins(NsState *s, NsChannel *c, Cause cause) {
    const NsExarDevice *x = s->dev->x;
    const NsExarPins *xp = s->pins->xp;
    size_t op = s->pins->op[c->index];
    size_t mf = s->pins->mf[c->index];
    size_t irtx = xp->irtx[c->index];
    LogicValue loop = bit_of(c->mcr, 4);
    LogicValue op_level = or3(not3(bit_of(c->mcr, 3)), loop);
    SimTime delay = cause == CAUSE_MCR_WRITE ? t_of(s, T_MDO) : 0;

    if (s->unknown) {
        ns_drive(s, op, LOGIC_X, 0);
        ns_drive(s, mf, LOGIC_X, 0);
        ns_drive(s, irtx, LOGIC_X, 0);
        return;
    }
    if (s->in_reset) {
        op_level = LOGIC_1;
    }
    ns_drive(s, op, op_level, delay);
    /* MF: AFR[2:1] picks OP2, BAUDOUT (driven on clock edges by the core),
       RXRDY or the reserved code. */
    if (mf != SERIAL_NO_PIN) {
        NsMfFunction f = x->has_mf ? x->mf[(c->afr.v >> 1) & 3u] : MF_OP2;

        switch (f) {
            case MF_OP2: ns_drive(s, mf, op_level, delay); break;
            case MF_BAUDOUT: break;
            case MF_RXRDY: ns_drive(s, mf, s->in_reset ? LOGIC_1 : exs_rxrdy_pin(c), 0); break;
            default: ns_drive(s, mf, LOGIC_X, 0); break;
        }
    }
    /* IRTX (XR16C854/864 QFP100): always IrDA-encoded, idle low. */
    if (irtx != SERIAL_NO_PIN) {
        LogicValue tx = c->tsr;

        if (exs_kb(c->lcr, 6)) {
            tx = LOGIC_0;
        }
        ns_drive(s, irtx, s->in_reset ? LOGIC_0 : ir_encode(c, tx), 0);
    }
}

/* XR16C864 TXDRQ/RXDRQ (active high) on the TXRDY/RXRDY pins by EMSR[2]/[3]. */
void ns_x_dma_pins(const NsChannel *c, LogicValue *rxrdy, LogicValue *txrdy) {
    if (!exs_xd(c)->dma_host || c->owner->in_reset) {
        return;
    }
    /* The requests follow the FIFO levels as the pins are driven (every bus
       access and tick ends here). */
    exs_dma_requests((NsChannel *)c);
    /* Class X: with the requests off the pins are TXRDY/RXRDY, mode-1 TXRDY
       low while a slot is free (decisions.md). */
    if (exs_mode1(c) && !exs_kb(c->xc.emsr, 2)) {
        *txrdy = bool3(exs_tx_free(c) == 0);
    }
    if (exs_kb(c->xc.emsr, 3)) {
        *rxrdy = bool3(c->xc.drq_rx);
    }
    if (exs_kb(c->xc.emsr, 2)) {
        *txrdy = bool3(c->xc.drq_tx);
    }
}

/* Open drain: 0 while any level is 1, X while one is X, else released. */
static LogicValue open_drain(LogicValue any) {
    return any == LOGIC_1 ? LOGIC_0 : any == LOGIC_X ? LOGIC_X : LOGIC_Z;
}

static LogicValue all_channels(const NsState *s) {
    LogicValue any = LOGIC_0;
    unsigned i;

    for (i = 0; i < s->dev->channel_count; i++) {
        any = or3(any, ns_int_level(&s->ch[i]));
    }
    return any;
}

static LogicValue timer_level(const NsState *s) {
    return s->xs.timer_int_en ? s->xs.timer_int : LOGIC_0;
}

int ns_x_int_pin(const NsState *s, const NsChannel *c, LogicValue level, LogicValue *out) {
    const NsExarDevice *x = s->dev->x;
    size_t pin = s->pins->channel[c->index][CH_INT];
    int mode;

    if (x != NULL && x->motorola_bus && s->xs.motorola != 0) {
        if (s->xs.motorola < 0) {
            *out = LOGIC_X;
        } else if (pin == s->pins->xp->irq) {
            /* Motorola: one open-drain IRQ for every channel, not gated by MCR[3]. */
            *out = s->in_reset ? LOGIC_Z : open_drain(all_channels(s));
        } else {
            *out = x->irq_others_low ? LOGIC_0 : LOGIC_Z;
        }
        return 1;
    }
    if (s->xs.part == NULL || s->xs.part->int_pin < 0) {
        return 0;
    }
    /* The part's "intPin" (D variants: always driven). */
    mode = s->xs.part->int_pin;
    if (mode == INT_DRIVEN) {
        *out = level;
    } else {
        LogicValue out2 = bit_of(c->mcr, 3);

        *out = out2 == LOGIC_1 ? level : out2 == LOGIC_0 ? LOGIC_Z : LOGIC_X;
    }
    return 1;
}

static SimTime cause_delay(const NsState *s, Cause cause) {
    switch (cause) {
        case CAUSE_RBR_READ:
        case CAUSE_LSR_READ: return t_of(s, T_RINT);
        case CAUSE_IIR_READ: return t_of(s, T_IR);
        case CAUSE_MSR_READ: return t_of(s, T_RIM);
        case CAUSE_THR_WRITE: return t_of(s, T_HR);
        case CAUSE_MCR_WRITE: return t_of(s, T_MDO);
        case CAUSE_MODEM_IN: return t_of(s, T_SIM);
        default: return 0;
    }
}

/* Shared DMA pins (XR16V554/564/654, XR16C854/864): the AND of the channels'
   active-low levels, refreshed on every channel update (receive events too). */
static void shared_dma(NsState *s, Cause cause) {
    LogicValue rx = LOGIC_1;
    LogicValue tx = LOGIC_1;
    unsigned i;

    if (s->dev->dma_pins != DMA_PINS_SHARED_AND) {
        return;
    }
    if (s->unknown) {
        rx = tx = LOGIC_X;
    } else if (!s->in_reset) {
        for (i = 0; i < s->dev->channel_count; i++) {
            rx = and3(rx, exs_rxrdy_pin(&s->ch[i]));
            tx = and3(tx, exs_txrdy_pin(&s->ch[i]));
        }
    }
    ns_drive(s, s->pins->dma_rxrdy, rx, cause == CAUSE_RBR_READ ? t_of(s, T_WRR) : 0);
    ns_drive(s, s->pins->dma_txrdy, tx, cause == CAUSE_THR_WRITE ? t_of(s, T_WXI) : 0);
}

/* Device-level pins: the Motorola IRQ (all channels), the 5G INT#, RST. */
void ns_x_device_pins(NsState *s, Cause cause) {
    const NsExarDevice *x = s->dev->x;
    const NsExarPins *xp = s->pins->xp;
    SimTime delay = cause_delay(s, cause);

    shared_dma(s, cause);
    if (x->motorola_bus && s->xs.motorola == 1 && !s->unknown) {
        ns_drive(s, xp->irq, s->in_reset ? LOGIC_Z : open_drain(all_channels(s)), delay);
    }
    if (xp->interrupt != SERIAL_NO_PIN) {
        LogicValue level = or3(all_channels(s), timer_level(s));

        ns_drive(s, xp->interrupt,
                 s->unknown ? LOGIC_X : s->in_reset ? LOGIC_Z : open_drain(level), delay);
    }
    /* ST16C1450/1550 RST = RESET OR (IER[5] AND MCR[2]); loopback cuts MCR[2] off. */
    if (xp->reset_out != SERIAL_NO_PIN) {
        const NsChannel *c = &s->ch[0];
        LogicValue reset = serial_level(s->sim, s->instance, s->pins->reset,
                                        s->dev->reset_active_high ? LOGIC_0 : LOGIC_1);
        LogicValue soft = and3(and3(bit_of(c->ier, 5), bit_of(c->mcr, 2)), not3(bit_of(c->mcr, 4)));

        if (!s->dev->reset_active_high) {
            reset = not3(reset);
        }
        ns_drive(s, xp->reset_out, s->unknown ? LOGIC_X : or3(reset, s->in_reset ? LOGIC_0 : soft),
                 delay);
    }
}

/* ---- Clocks ------------------------------------------------------------------------- */

int ns_x_prescale(NsState *s, NsChannel *c, int rising) {
    (void)s;
    if (c->asleep || c->xc.powered_down) {
        return 0;   /* sleep / power-down: the baud clock stops */
    }
    if (c->xc.prescale4) {
        if (!rising) {
            return 0;
        }
        if (++c->prescale_count < 4) {
            return 0;
        }
        c->prescale_count = 0;
    }
    return 1;
}

void ns_x_load_divisor(NsChannel *c) {
    serial_baud_load(&c->baud, (uint32_t)c->dll.v | ((uint32_t)c->dlm.v << 8),
                     c->dll.k == 0xFF && c->dlm.k == 0xFF);
    /* DLD[3:0]: the fraction n/16. */
    if (exs_xd(c)->dld && (c->xc.dld.k & 0x0Fu) == 0x0Fu) {
        c->baud.frac = c->xc.dld.v & 0x0Fu;
        c->baud.frac_den = 16;
    }
}

unsigned ns_x_factor(const NsChannel *c) {
    const NsExarDevice *x = exs_xd(c);

    if (x->dld_sampling && (c->xc.dld.k & 0x30u) == 0x30u) {
        unsigned rate = (c->xc.dld.v >> 4) & 3u;

        return rate == 0 ? 16 : rate == 1 ? 8 : 4;   /* DLD[5:4]: 16x, 8x, 4x */
    }
    if (x->style == XS_5G && ((c->owner->xs.mode8x >> c->index) & 1u)) {
        return 8;   /* 8XMODE */
    }
    return 16;
}

/* "startSample": 7.5 checks the start bit half a clock earlier: tick 7 of 16. */
unsigned ns_x_start_sample(const NsChannel *c, unsigned factor) {
    return exs_xd(c)->start_75 ? factor * 15 / 32 : 0;
}

int ns_x_own_clock(const NsState *s, unsigned channel) {
    return s->dev->x->chc_clock != NULL && channel == 2 &&
           s->pins->xp->chc_clock != SERIAL_NO_PIN;
}

/* BCLK (XR16C864): the channel's 16x clock. */
void ns_x_clock_out(NsState *s, NsChannel *c, LogicValue baudout) {
    size_t pin = s->pins->xp->bclk[c->index];

    if (!SERIAL_REAL_PIN(pin) || s->out[pin] == baudout) {
        return;
    }
    s->out[pin] = baudout;
    sim_schedule_drive(s->sim, s->instance, pin, baudout, DRIVE_STRONG, t_of(s, T_BLD));
}

/* An edge of a channel's own clock pin (CHCCLK): the core's per-channel step. */
static void own_clock_edge(NsState *s, NsChannel *c, LogicValue level) {
    LogicValue last = c->xc.clock_level;
    int rising = last == LOGIC_0 && level == LOGIC_1;
    int tick = 0;

    c->xc.clock_level = level;
    if (level == last || (!rising && !(last == LOGIC_1 && level == LOGIC_0))) {
        return;
    }
    if (ns_x_prescale(s, c, rising) || (!rising && !c->asleep && !c->xc.prescale4)) {
        tick = serial_baud_edge(&c->baud, rising, level);
    }
    ns_x_clock_out(s, c, s->unknown || c->asleep ? LOGIC_X : c->baud.baudout);
    if (tick) {
        ns_tx_tick(s, c);
        if (s->pins->receive == SERIAL_NO_PIN) {
            ns_rx_tick(s, c);
        }
    }
}

/* ---- Bus: select decode and the extra units ------------------------------------------ */

/* Units past the channels: the 5G global block, then the reserved / FSTAT
   unit, then one DMA unit per channel (XR16C864). */
static unsigned unit_global(const NsState *s) { return s->dev->channel_count; }
static unsigned unit_other(const NsState *s) { return s->dev->channel_count + 1; }
static unsigned unit_dma(const NsState *s, unsigned ch) { return s->dev->channel_count + 2 + ch; }

int ns_x_needs_select(const NsDevice *d) {
    const NsExarDevice *x = d->x;

    return x->motorola_bus || x->pwrsave != NULL || x->style == XS_5G || x->fstat != NULL ||
           x->dma_host;
}

static LogicValue low_active(const NsState *s, size_t pin) {
    return not3(serial_level(s->sim, s->instance, pin, LOGIC_1));
}

/* The device's own chip selects (5G CS; the single-CS Motorola parts). */
static LogicValue bus_cs(const NsState *s) {
    const SerialBusPins *bp = &s->pins->bus;
    LogicValue all = LOGIC_1;
    unsigned i;

    for (i = 0; i < bp->select_count; i++) {
        all = and3(all, low_active(s, bp->select[i]));
    }
    for (i = 0; i < bp->select_high_count; i++) {
        all = and3(all, serial_level(s->sim, s->instance, bp->select_high[i], LOGIC_0));
    }
    return all;
}

static int select_5g(NsState *s, unsigned address, unsigned known, uint32_t *units,
                     LogicValue *result) {
    const NsExarDevice *x = s->dev->x;
    LogicValue cs = bus_cs(s);
    unsigned cc = s->dev->channel_count;
    unsigned ch_bits = x->v798 ? 0x70u : 0x30u;

    if (cs != LOGIC_1) {
        *result = cs;
        return 1;
    }
    *result = LOGIC_1;
    if (!(known & 0x80u)) {
        *result = LOGIC_X;
        return 1;
    }
    if (address & 0x80u) {
        *units = (address & 0x70u) == 0 ? 1u << unit_global(s) : 1u << unit_other(s);
        return 1;
    }
    if (!x->v798 && (known & 0x40u) && (address & 0x40u)) {
        *units = 1u << unit_other(s);   /* XR16L784: 0x40-0x7F reserved */
        return 1;
    }
    /* REGB[0]: a channel-register write goes to every channel. */
    if (s->xs.regb & 1u) {
        *units = (1u << cc) - 1u;
        return 1;
    }
    if ((known & ch_bits) != ch_bits || (!x->v798 && !(known & 0x40u))) {
        *result = LOGIC_X;
        return 1;
    }
    *units = 1u << ((address & ch_bits) >> 4);
    return 1;
}

int ns_x_select(NsState *s, unsigned address, unsigned address_known, uint32_t *units,
                LogicValue *result) {
    const NsExarDevice *x = s->dev->x;
    const NsExarPins *xp = s->pins->xp;
    unsigned cc = s->dev->channel_count;
    unsigned i;

    /* PowerSave: the bus inputs are isolated. */
    if (s->xs.isolated) {
        *result = LOGIC_0;
        return 1;
    }
    /* XR16C864 DMA cycle: DACKx low and AEN high; CS and A2..A0 are ignored. */
    if (x->dma_host) {
        LogicValue aen = serial_level(s->sim, s->instance, xp->aen, LOGIC_0);

        for (i = 0; i < cc; i++) {
            LogicValue dack = low_active(s, xp->dack[i]);
            LogicValue cycle = and3(aen, dack);

            if (cycle == LOGIC_X) {
                *result = LOGIC_X;
                return 1;
            }
            if (cycle == LOGIC_1) {
                *units = 1u << unit_dma(s, i);
                *result = LOGIC_1;
                return 1;
            }
        }
    }
    /* FSTAT (FSRS low): any address. */
    if (xp->fstat != SERIAL_NO_PIN) {
        LogicValue fsrs = low_active(s, xp->fstat);

        if (fsrs != LOGIC_0) {
            *units = 1u << unit_other(s);
            *result = fsrs;
            return 1;
        }
    }
    if (x->style == XS_5G) {
        return select_5g(s, address, address_known, units, result);
    }
    if (x->motorola_bus && s->xs.motorola != 0) {
        LogicValue cs = s->xs.motorola < 0 ? LOGIC_X : low_active(s, xp->strobe);
        unsigned ch = 0;

        *result = cs;
        if (cs != LOGIC_1) {
            return 1;
        }
        /* A3 (and A4) on the Intel select pins pick the channel. */
        for (i = 0; i < x->chan_bits; i++) {
            LogicValue bit = serial_level(s->sim, s->instance, xp->chan_bit[i], LOGIC_X);

            if (bit == LOGIC_X) {
                *result = LOGIC_X;
                return 1;
            }
            ch |= (bit == LOGIC_1 ? 1u : 0u) << i;
        }
        *units = ch < cc ? 1u << ch : 1u << unit_other(s);
        return 1;
    }
    if (!s->dev->per_channel_select && s->dev->channel_pin == NULL && s->dev->lpt == NULL) {
        *result = bus_cs(s);   /* one channel, the bus's own selects */
        return 1;
    }
    return 0;
}

/* The 5G per-channel 3-bit interrupt code for INT1-INT3. */
static int int_code(const NsState *s, const NsChannel *c) {
    uint8_t iir;
    uint8_t k;

    ns_iir_now(c, &iir, &k);
    if (k != 0xFF) {
        return -1;
    }
    if (iir & 1u) {
        if (c->index == 0 && timer_level(s) == LOGIC_1) {
            return 7;
        }
        return c->index == 0 && timer_level(s) == LOGIC_X ? -1 : 0;
    }
    switch (iir & 0x3Eu) {
        case 0x06:
        case 0x04: return 1;
        case 0x0C: return 2;
        case 0x02: return 3;
        default: return 4;
    }
}

static void global_peek(NsState *s, unsigned address, uint8_t *v, uint8_t *k) {
    const NsDevice *d = s->dev;
    const NsExarDevice *x = d->x;
    NsExarState *g = &s->xs;
    unsigned cc = d->channel_count;
    unsigned i;

    *v = 0;
    *k = 0xFF;
    switch (address & 0x0Fu) {
        case 0x0:
            for (i = 0; i < cc; i++) {
                LogicValue level = ns_int_level(&s->ch[i]);

                if (i == 0) {
                    level = or3(level, timer_level(s));
                }
                if (level == LOGIC_1) {
                    *v |= (uint8_t)(1u << i);
                } else if (level == LOGIC_X) {
                    *k &= (uint8_t)~(1u << i);
                }
            }
            break;
        case 0x1:
        case 0x2:
        case 0x3: {
            uint32_t value = 0;
            uint32_t known = 0xFFFFFFu;
            unsigned shift = ((address & 0x0Fu) - 1u) * 8u;

            for (i = 0; i < cc; i++) {
                int code = int_code(s, &s->ch[i]);

                if (code < 0) {
                    known &= ~(7u << (3 * i));
                } else {
                    value |= (uint32_t)code << (3 * i);
                }
            }
            *v = (uint8_t)(value >> shift);
            *k = (uint8_t)(known >> shift);
            break;
        }
        case 0x4:
            if (x->timer_commands) {
                /* XR16V798: 01 while the timer interrupt is pending. */
                *v = g->timer_int == LOGIC_1 ? 1 : 0;
                *k = g->timer_int == LOGIC_X ? 0xFE : 0xFF;
            } else {
                *v = g->timercntl & 0x0Fu;
            }
            break;
        case 0x6: *v = (uint8_t)g->timer_reload; break;
        case 0x7: *v = (uint8_t)(g->timer_reload >> 8); break;
        case 0x8: *v = g->mode8x; break;
        case 0xB: *v = g->sleep; break;
        case 0xC: *v = d->drev >= 0 ? (uint8_t)d->drev : 0; break;
        case 0xD: *v = d->dvid >= 0 ? (uint8_t)d->dvid : 0; break;
        case 0xE: *v = g->regb; break;
        case 0xF: *k = 0; break;   /* not listed: X */
        default: break;            /* TIMER, REGA, RESET read 00 */
    }
}

static void global_read_done(NsState *s, unsigned address) {
    const NsExarDevice *x = s->dev->x;
    unsigned i;

    switch (address & 0x0Fu) {
        case 0x0:
            /* XR16L784: an INT0 read clears the wake-up indicator. */
            if (!x->v798) {
                for (i = 0; i < s->dev->channel_count; i++) {
                    s->ch[i].xc.wake_int = 0;
                }
            }
            break;
        case 0x4:
            if (s->xs.timer_int != LOGIC_X) {
                s->xs.timer_int = LOGIC_0;   /* a TIMERCNTL read clears the timer interrupt */
            }
            break;
        default:
            return;
    }
    ns_update_outputs(s, CAUSE_IIR_READ);
}

static void timer_start(NsExarState *g) {
    g->timer_running = 1;
    g->timer_count = g->timer_reload;
}

static void global_write(NsState *s, unsigned address, uint8_t v, uint8_t k) {
    const NsDevice *d = s->dev;
    const NsExarDevice *x = d->x;
    NsExarState *g = &s->xs;
    uint8_t chans = (uint8_t)((1u << d->channel_count) - 1u);
    unsigned i;

    if (k != 0xFF) {
        v = 0;   /* an X write: the global registers take nothing (P2) */
    }
    switch (address & 0x0Fu) {
        case 0x4:
            if (x->timer_commands) {
                switch (v & 0x0Fu) {
                    case 0x1: g->timer_int_en = 1; break;
                    case 0x2: g->timer_int_en = 0; break;
                    case 0x3: g->timer_oneshot = 1; break;
                    case 0x4: g->timer_oneshot = 0; break;
                    case 0x5: g->timer_ext = 0; break;
                    case 0x6: g->timer_ext = 1; break;
                    case 0x9: timer_start(g); break;
                    case 0xA: g->timer_running = 0; break;
                    case 0xB:
                        g->timer_running = 0;
                        g->timer_count = g->timer_reload;
                        g->timer_int = LOGIC_0;
                        break;
                    default: break;   /* 0000, 0111, 1000, 1100-1111 reserved */
                }
            } else {
                /* XR16L784: b0 INT enable, b1 start/stop, b2 one-shot, b3 TMRCK. */
                if ((v & 2u) && !g->timer_running) {
                    timer_start(g);
                }
                g->timer_running = (v & 2u) != 0;
                g->timer_int_en = v & 1u;
                g->timer_oneshot = (v & 4u) != 0;
                g->timer_ext = (v & 8u) != 0;
                g->timercntl = v & 0x0Fu;
            }
            break;
        case 0x6: g->timer_reload = (uint16_t)((g->timer_reload & 0xFF00u) | v); break;
        case 0x7: g->timer_reload = (uint16_t)((g->timer_reload & 0x00FFu) | (v << 8)); break;
        case 0x8:
            g->mode8x = v & chans;
            for (i = 0; i < d->channel_count; i++) {
                exs_set_format_rate(&s->ch[i]);
            }
            break;
        case 0xA:
            /* RESET: soft-reset the channels whose bits are 1 (self-clearing). */
            g->soft = 1;
            for (i = 0; i < d->channel_count; i++) {
                if ((v >> i) & 1u) {
                    ns_reset_channel(s, &s->ch[i]);
                    ns_x_load_divisor(&s->ch[i]);
                    exs_set_format_rate(&s->ch[i]);
                }
            }
            g->soft = 0;
            break;
        case 0xB:
            g->sleep = v & chans;
            for (i = 0; i < d->channel_count; i++) {
                if (!((g->sleep >> i) & 1u)) {
                    s->ch[i].asleep = 0;
                }
                ns_x_sleep_check(s, &s->ch[i]);
            }
            break;
        case 0xE: g->regb = v & 1u; break;
        default: break;   /* read-only or reserved */
    }
    ns_update_outputs(s, CAUSE_WRITE);
}

/* FSTAT: b3-0 = NOT TXRDYD..A, b7-4 = RXRDYD..A pins. */
uint8_t exs_fstat(const NsState *s) {
    uint8_t v = 0;
    unsigned i;

    for (i = 0; i < s->dev->channel_count && i < 4; i++) {
        if (exs_txrdy_pin(&s->ch[i]) == LOGIC_0) {
            v |= (uint8_t)(1u << i);
        }
        if (exs_rxrdy_pin(&s->ch[i]) == LOGIC_1) {
            v |= (uint8_t)(0x10u << i);
        }
    }
    return v;
}

/* A DMA cycle reads the RX FIFO / writes the TX FIFO whatever LCR[7] is. */
static Reg dma_begin(NsChannel *c) {
    Reg saved = c->lcr;

    c->lcr.v &= 0x7F;
    c->lcr.k |= 0x80;
    return saved;
}

int ns_x_unit_peek(NsState *s, unsigned unit, unsigned address, uint8_t *v, uint8_t *k) {
    const NsExarDevice *x = s->dev->x;

    if (unit < s->dev->channel_count) {
        return 0;
    }
    *v = 0;
    *k = 0;
    if (x->style == XS_5G && unit == unit_global(s)) {
        global_peek(s, address, v, k);
    } else if (unit == unit_other(s) && x->fstat != NULL) {
        *v = exs_fstat(s);
        *k = 0xFF;
    } else if (x->dma_host && unit >= unit_dma(s, 0)) {
        NsChannel *c = &s->ch[unit - unit_dma(s, 0)];
        Reg saved = dma_begin(c);

        ns_reg_peek(c, 0, v, k);
        c->lcr = saved;
    }
    return 1;
}

int ns_x_unit_read_done(NsState *s, unsigned unit, unsigned address) {
    const NsExarDevice *x = s->dev->x;

    if (unit < s->dev->channel_count) {
        return 0;
    }
    if (x->style == XS_5G && unit == unit_global(s)) {
        global_read_done(s, address);
    } else if (x->dma_host && unit >= unit_dma(s, 0)) {
        NsChannel *c = &s->ch[unit - unit_dma(s, 0)];
        Reg saved = dma_begin(c);
        Cause cause = ns_reg_read_done(c, 0);

        c->lcr = saved;
        ns_x_levels(s, c);
        ns_update_channel(s, c, cause);
    }
    return 1;
}

int ns_x_unit_write(NsState *s, unsigned unit, unsigned address, uint8_t v, uint8_t k) {
    const NsExarDevice *x = s->dev->x;

    if (unit < s->dev->channel_count) {
        return 0;
    }
    if (x->style == XS_5G && unit == unit_global(s)) {
        global_write(s, address, v, k);
    } else if (x->dma_host && unit >= unit_dma(s, 0)) {
        NsChannel *c = &s->ch[unit - unit_dma(s, 0)];
        Reg saved = dma_begin(c);
        Cause cause = ns_reg_write(s, c, 0, v, k);

        c->lcr = saved;
        ns_x_levels(s, c);
        ns_update_channel(s, c, cause);
    }
    return 1;   /* FSTAT and reserved addresses take no writes */
}

/* AFR[0] concurrent write: one shared bit, or (XR16L2552) the addressed channel's. */
int ns_x_conc(const NsState *s, unsigned unit) {
    if (s->dev->x->afr0_per_channel) {
        return unit < s->dev->channel_count && exs_kb(s->ch[unit].afr, 0);
    }
    return s->conc;
}

/* Motorola mode: the reset pin is active low. */
int ns_x_reset_active_high(const NsState *s) {
    if (s->dev->x->motorola_bus && s->xs.motorola == 1) {
        return 0;
    }
    return s->dev->reset_active_high;
}

/* ---- Pin changes, start -------------------------------------------------------------- */

static void read_mode(NsState *s) {
    const NsExarPins *xp = s->pins->xp;
    LogicValue mode;

    if (!s->dev->x->motorola_bus) {
        return;
    }
    /* 16/68: high = Intel (16), low = Motorola (68). */
    mode = serial_level(s->sim, s->instance, xp->mode, LOGIC_X);
    s->xs.motorola = mode == LOGIC_1 ? 0 : mode == LOGIC_0 ? 1 : -1;
    s->bus.motorola = s->xs.motorola;
    s->bus.rw = xp->rw;
}

/* `pin` is a real pin (the caller is is_select_pin, from ns_pin_changed). */
int ns_x_is_select_pin(const NsState *s, size_t pin) {
    const NsExarPins *xp = s->pins->xp;
    unsigned i;

    if (pin == xp->mode || pin == xp->strobe || pin == xp->chan_bit[0] || pin == xp->chan_bit[1] ||
        pin == xp->pwrsave || pin == xp->fstat || pin == xp->aen) {
        return 1;
    }
    for (i = 0; i < 4; i++) {
        if (pin == xp->dack[i]) {
            return 1;
        }
    }
    return 0;
}

/* 1 when the bus mode changed (the core re-applies the reset level). `pin`
   is a real pin (the one caller is ns_pin_changed). */
int ns_x_pin_changed(NsState *s, size_t pin) {
    const NsExarPins *xp = s->pins->xp;
    int before = s->xs.motorola;
    unsigned i;

    if (pin == xp->mode) {
        read_mode(s);
        return s->xs.motorola != before;
    }
    if (pin == xp->pwrsave) {
        exs_update_isolation(s);
    }
    if (pin == xp->tmrck) {
        LogicValue level = serial_level(s->sim, s->instance, pin, LOGIC_X);

        if (s->xs.tmrck_level == LOGIC_0 && level == LOGIC_1 && s->xs.timer_ext) {
            exs_timer_step(s);
        }
        s->xs.tmrck_level = level;
    }
    if (pin == xp->chc_clock) {
        own_clock_edge(s, &s->ch[2], serial_level(s->sim, s->instance, pin, LOGIC_X));
    }
    if (pin == xp->tc) {
        LogicValue level = serial_level(s->sim, s->instance, pin, LOGIC_0);

        /* TC pulse: ends the requests of the channel whose DACK is low. */
        if (s->xs.tc_level == LOGIC_0 && level == LOGIC_1) {
            for (i = 0; i < s->dev->channel_count; i++) {
                if (low_active(s, xp->dack[i]) == LOGIC_1) {
                    NsExarChannel *xc = &s->ch[i].xc;

                    if (xc->drq_tx) {
                        xc->drq_tx = 0;
                        xc->tc_tx = 1;
                    }
                    if (xc->drq_rx) {
                        xc->drq_rx = 0;
                        xc->tc_rx = 1;
                    }
                }
            }
            ns_update_outputs(s, CAUSE_SERIAL);
        }
        s->xs.tc_level = level;
    }
    return 0;
}

void ns_x_start(NsState *s) {
    unsigned i;

    read_mode(s);
    s->xs.tmrck_level = serial_level(s->sim, s->instance, s->pins->xp->tmrck, LOGIC_X);
    s->xs.tc_level = serial_level(s->sim, s->instance, s->pins->xp->tc, LOGIC_0);
    for (i = 0; i < s->dev->channel_count; i++) {
        s->ch[i].xc.clock_level = serial_level(s->sim, s->instance, s->pins->xp->chc_clock,
                                               LOGIC_X);
        exs_sample_straps(s, &s->ch[i]);   /* CLKSEL and ENIR at power-up */
    }
}
