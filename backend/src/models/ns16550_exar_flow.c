/* models/ns16550_exar_flow.c: part of the model in models/ns16550_exar.c (split by plan-68-I/J). */
#include "ns16550_exar_int.h"
/* ---- Flow control: receive side ------------------------------------------------------ */

static void resume_tx(NsChannel *c) {
    c->tx_halted = 0;
    c->iir_xoff = 0;
}

int ns_x_rx_char(NsState *s, NsChannel *c, const SerialRxChar *rc) {
    const NsDevice *d = s->dev;
    const NsExarDevice *x = d->x;
    NsExarChannel *xc = &c->xc;
    uint8_t mask = exs_word_mask(c);
    uint8_t data;
    unsigned rxmode = c->efr.v & 3u;
    unsigned txmode = (c->efr.v >> 2) & 3u;
    int sequential;
    int xon = 0;
    int xoff = 0;

    if (xc->rx_allow > 0) {
        xc->rx_allow--;   /* 5G RX disable: the one character still let in */
    }
    if (!exs_has_bank(c)) {
        return 1;
    }
    /* The special-character interrupt clears with the next character
       (XR16M752: only by an ISR read). */
    if (x->style != XS_M752) {
        c->iir_special = 0;
    }
    if ((rc->known & mask) != mask ||
        (rc->flags & (SERIAL_RX_PE | SERIAL_RX_FE | SERIAL_RX_BI)) != 0 ||
        (rc->flags_known & 7u) != 7u) {
        c->seq_wait = 0;
        return 1;   /* errored or unknown characters are data */
    }
    data = (uint8_t)(rc->data & mask);

    /* The second of a sequential pair. */
    if (c->seq_wait != 0) {
        int waiting = c->seq_wait;
        SerialRxChar held = c->seq_held;

        c->seq_wait = 0;
        if (waiting == 1 && data == (c->xon2.v & mask)) {
            resume_tx(c);
            xc->xchar.v |= 0x02;
            return 0;
        }
        if (waiting == 2 && data == (c->xoff2.v & mask)) {
            c->tx_halted = 1;
            c->iir_xoff = 1;
            xc->xchar.v |= 0x01;
            return 0;
        }
        ns_store_char(c, &held);   /* not a pair: the first goes to the FIFO */
    }

    /* EFR[3:0] (exarOr): 1011/0111 compare either character; 1111 and 0011 the pairs. */
    sequential = rxmode == 3 && (txmode == 3 || txmode == 0 || d->sw_flow == SW_NXP_AND);
    switch (rxmode) {
        case 2:
            xon = data == (c->xon1.v & mask);
            xoff = data == (c->xoff1.v & mask);
            break;
        case 1:
            xon = data == (c->xon2.v & mask);
            xoff = data == (c->xoff2.v & mask);
            break;
        case 3:
            if (sequential) {
                if (data == (c->xoff1.v & mask)) {
                    c->seq_wait = 2;
                    c->seq_held = *rc;
                    return 0;
                }
                if (data == (c->xon1.v & mask)) {
                    c->seq_wait = 1;
                    c->seq_held = *rc;
                    return 0;
                }
            } else {
                xon = data == (c->xon1.v & mask) || data == (c->xon2.v & mask);
                xoff = data == (c->xoff1.v & mask) || data == (c->xoff2.v & mask);
            }
            break;
        default:
            break;
    }
    if (xoff) {
        c->tx_halted = 1;
        c->iir_xoff = 1;
        xc->xchar.v |= 0x01;
        /* EFR[1:0] = 01 with EFR[5]: Xoff2 is a flow character, not stored, and
           raises the special-character interrupt too. */
        if (d->special_char && exs_kb(c->efr, 5) && rxmode == 1) {
            c->iir_special = 1;
        }
        return 0;
    }
    if (xon) {
        resume_tx(c);
        xc->xchar.v |= 0x02;
        return 0;
    }
    /* Xon-any (MCR[5]): any other character resumes; it is stored. */
    if (c->tx_halted && x->xon_any && exs_kb(c->mcr, 5)) {
        resume_tx(c);
    }
    /* Special character (EFR[5]): Xoff2 is stored and flags ISR[4]. */
    if (d->special_char && exs_kb(c->efr, 5) && data == (c->xoff2.v & mask)) {
        c->iir_special = 1;
    }
    return 1;
}

/* ---- Flow control: levels (auto-RTS/DTR, Xoff/Xon sending) ------------------------- */

/* Table D hysteresis (EMSR[5:4]:FCTR[1:0] on the 854, FCTR[3:0] on the 5G). */
static unsigned hysteresis(const NsChannel *c) {
    static const unsigned TABLE[16] = {0, 4, 6, 8, 8, 16, 24, 32, 40, 44, 48, 52, 12, 20, 28, 36};
    unsigned i = exs_style_of(c) == XS_5G ? (c->xc.fctr.v & 0x0Fu)
                                      : ((c->xc.emsr.v >> 2) & 0x0Cu) | (c->xc.fctr.v & 3u);

    return TABLE[i];
}

/* RTS off/on levels and Xoff/Xon send levels for the current trigger. */
static void flow_levels(const NsChannel *c, unsigned *rts_off, unsigned *rts_on, unsigned *xoff_at,
                        unsigned *xon_at) {
    const NsDevice *d = c->owner->dev;
    const NsExarDevice *x = d->x;
    unsigned i = c->trigger & 3u;
    unsigned m = d->tlr != 0 ? d->tlr : 4;

    *rts_off = *rts_on = *xoff_at = *xon_at = 0;
    switch (x->style) {
        case XS_M752:
            *rts_off = *xoff_at = (c->tcr.v & 0x0Fu) * m;
            *rts_on = *xon_at = ((c->tcr.v >> 4) & 0x0Fu) * m;
            break;
        case XS_854:
        case XS_5G: {
            int t = exs_table_of(c);

            if (t < 0) {
                t = 0;
            }
            if (t < 3) {
                const unsigned *tab = x->rx_table[t];

                *xoff_at = tab[i];
                *xon_at = i > 0 ? tab[i - 1] : 0;
                *rts_off = i < 3 ? tab[i + 1] : tab[3];
                *rts_on = *xon_at;
            } else {
                unsigned trg = ns_rx_level(c);
                unsigned h = hysteresis(c);

                *xoff_at = trg;
                *rts_off = trg + h;
                /* On again below trigger - hysteresis (decisions.md: 0 = at RXTRG). */
                *rts_on = *xon_at = trg > h ? trg - h - 1 : 0;
            }
            break;
        }
        default:
            if (d->auto_flow == FLOW_TABLE) {
                *rts_off = d->rts_table[i][0];
                *rts_on = d->rts_table[i][1];
            }
            if (d->has_xoff_table) {
                *xoff_at = d->xoff_table[i][0];
                *xon_at = d->xoff_table[i][1];
            }
            break;
    }
}

static int flow_allowed(const NsChannel *c) {
    return !exs_loopback(c) || exs_xd(c)->flow_loopback;
}

int exs_auto_rts_on(const NsChannel *c) {
    return c->owner->dev->auto_flow != FLOW_NONE && exs_has_bank(c) && exs_kb(c->efr, 6) &&
           flow_allowed(c);
}

static int auto_cts_on(const NsChannel *c) {
    return c->owner->dev->auto_flow != FLOW_NONE && exs_has_bank(c) && exs_kb(c->efr, 7) &&
           flow_allowed(c);
}

/* 5G: MCR[2] = 1 moves auto flow to DTR/DSR. */
int exs_flow_on_dtr(const NsChannel *c) {
    return c->owner->dev->auto_flow == FLOW_DTR_DSR && exs_kb(c->mcr, 2);
}

static unsigned char_ticks(const NsChannel *c) {
    return serial_format_char_bits(&c->format) * c->format.factor;
}

/* XR16C864 DMA requests (decisions.md): TXDRQ while the TX FIFO is below its
   trigger, until TC or the FIFO fills to it; RXDRQ at the RX trigger or a
   timeout, until TC or the FIFO empties. */
void exs_dma_requests(NsChannel *c) {
    NsExarChannel *xc = &c->xc;
    unsigned level = exs_tx_level_bytes(c);

    if (level == 0) {
        level = 1;
    }
    if (c->tx_count >= level) {
        xc->drq_tx = 0;
        xc->tc_tx = 0;
    } else if (!xc->tc_tx) {
        xc->drq_tx = 1;
    }
    if (c->rx_count == 0) {
        xc->drq_rx = 0;
        xc->tc_rx = 0;
    } else if (!xc->tc_rx && (c->rx_count >= (unsigned)ns_rx_level(c) || c->timeout)) {
        xc->drq_rx = 1;
    }
}

void ns_x_levels(NsState *s, NsChannel *c) {
    const NsExarDevice *x = s->dev->x;
    unsigned level = c->rx_count;
    unsigned txmode = (c->efr.v >> 2) & 3u;
    unsigned rts_off;
    unsigned rts_on;
    unsigned xoff_at;
    unsigned xon_at;
    int was_off = c->rts_off;

    flow_levels(c, &rts_off, &rts_on, &xoff_at, &xon_at);
    /* Auto-RTS (or DTR): inactive at the off level, active again at the on level. */
    if (exs_auto_rts_on(c)) {
        if (rts_off != 0 && level >= rts_off) {
            c->rts_off = 1;
        } else if (level <= rts_on) {
            c->rts_off = 0;
        }
    } else {
        c->rts_off = 0;
    }
    if (!was_off && c->rts_off && exs_kb(c->ier, 6) && exs_kb(c->mcr, exs_flow_on_dtr(c) ? 0 : 1)) {
        c->iir_ctsrts = 1;   /* the RTS (DTR) pin went low -> high */
    }
    /* Xoff / Xon sending: Xoff two character times after the trigger is
       crossed (XR16M752: at the TCR level), Xon at the level below. */
    if (exs_has_bank(c) && txmode != 0 && c->fifo_on && xoff_at != 0) {
        if (!c->xoff_sent && c->xc.xoff_delay == 0 && level >= xoff_at) {
            if (x->style == XS_M752) {
                exs_queue_flow_pair(c, txmode, 0);
                c->xoff_sent = 1;
            } else {
                c->xc.xoff_delay = 2 * char_ticks(c);
            }
        } else if (c->xc.xoff_delay != 0 && level <= xon_at) {
            c->xc.xoff_delay = 0;   /* drained before the Xoff went out */
        } else if (c->xoff_sent && level <= xon_at) {
            exs_queue_flow_pair(c, txmode, 1);
            c->xoff_sent = 0;
        }
    }
    if (x->dma_host) {
        exs_dma_requests(c);
    }
    ns_x_sleep_check(s, c);
}

/* RS-485 direction: the pin, the turnaround delay in bit times. */
int exs_rs485_enabled(const NsChannel *c) {
    switch (c->owner->dev->rs485) {
        case RS485_FCTR_RTS:
        case RS485_FCTR_MCR2: return exs_kb(c->xc.fctr, 5);
        case RS485_DLD6: return exs_kb(c->xc.dld, 6);
        case RS485_FCTR3_OP1: return exs_kb(c->xc.fctr, 3);
        default: return 0;
    }
}

static unsigned rs485_delay_bits(const NsChannel *c) {
    switch (c->owner->dev->rs485) {
        case RS485_FCTR_RTS:
        case RS485_FCTR_MCR2: return (c->xc.msrw.v >> 4) & 0x0Fu;   /* 0-15 bit times */
        case RS485_DLD6: return 1;   /* released one bit after the last stop bit */
        default: return 0;
    }
}

/* One baud tick of the channel (after the transmitter's). */
void ns_x_tick(NsState *s, NsChannel *c) {
    NsExarChannel *xc = &c->xc;

    (void)s;
    if (xc->xoff_delay != 0 && --xc->xoff_delay == 0) {
        exs_queue_flow_pair(c, (c->efr.v >> 2) & 3u, 0);
        c->xoff_sent = 1;
    }
    if (xc->rs485 && !serial_tx_busy(&c->line.tx) && c->tx_count == 0 && c->flow_out_count == 0 &&
        !xc->imm_valid) {
        if (xc->rs485_hold == 0) {
            xc->rs485_hold = rs485_delay_bits(c) * c->format.factor + 1;
        }
        if (--xc->rs485_hold == 0) {
            xc->rs485 = 0;
        }
    }
}

/* ---- Flow control: transmit side ----------------------------------------------------- */

int ns_x_tx_flow(NsChannel *c, uint16_t *data) {
    if (c->flow_out_count > 0) {
        *data = c->flow_out[0];
        memmove(c->flow_out, c->flow_out + 1, --c->flow_out_count);
        return 1;
    }
    /* 5G send-immediate: after the flow characters, ahead of the FIFO. */
    if (c->xc.imm_valid && !exs_kb(c->xc.msrw, 3)) {
        *data = c->xc.imm_data;
        c->xc.imm_valid = 0;
        return 1;
    }
    return 0;
}

int ns_x_tx_blocked(NsState *s, NsChannel *c) {
    if (c->tx_halted) {
        return 1;   /* Xoff received */
    }
    if (exs_style_of(c) == XS_5G && exs_kb(c->xc.msrw, 3)) {
        return 1;   /* TX disable: the TSR halts, the FIFO keeps its data */
    }
    if (auto_cts_on(c)) {
        int dsr = exs_flow_on_dtr(c);
        LogicValue cts;

        if (exs_loopback(c)) {
            /* XR16L580: auto flow works in loopback (RTS -> CTS inside). */
            cts = and3(bit_of(c->mcr, dsr ? 0 : 1), bool3(!c->rts_off));
        } else {
            cts = not3(serial_level(s->sim, s->instance,
                                    s->pins->channel[c->index][dsr ? CH_DSR : CH_CTS], LOGIC_X));
        }
        if (cts != LOGIC_1) {
            return 1;
        }
    }
    return 0;
}

/* ---- Interrupts -------------------------------------------------------------------- */

LogicValue ns_x_xoff_source(const NsChannel *c) {
    if (!exs_has_bank(c)) {
        return LOGIC_0;
    }
    return and3(bool3(c->iir_xoff || c->iir_special), bit_of(c->ier, 5));
}

LogicValue ns_x_ctsrts_source(const NsChannel *c) {
    if (!exs_has_bank(c)) {
        return LOGIC_0;
    }
    return and3(bool3(c->iir_ctsrts), or3(bit_of(c->ier, 6), bit_of(c->ier, 7)));
}

/* The TX interrupt waits for the TSR (RS-485 modes, XR16C854 FCTR[3]). */
static int thre_waits_tsr(const NsChannel *c) {
    switch (exs_style_of(c)) {
        case XS_854: return exs_kb(c->xc.fctr, 3);
        case XS_5G: return exs_kb(c->xc.fctr, 5);
        case XS_M752: return exs_kb(c->xc.dld, 6);
        default: return 0;
    }
}

void ns_x_sources(const NsChannel *c, LogicValue *ls, LogicValue *rda, LogicValue *thre,
                  LogicValue *wake) {
    if (thre_waits_tsr(c) && serial_tx_busy(&c->line.tx)) {
        *thre = LOGIC_0;
    }
    /* 5G: an errored byte interrupts as it comes in, not only at the top. */
    if (c->xc.ls_new) {
        *ls = or3(*ls, bit_of(c->ier, 2));
    }
    /* Table D with TRG out of range: whether the trigger is reached is X. */
    if (exs_rx_trigger_x(c) && c->rx_count > 0) {
        *rda = and3(LOGIC_X, bit_of(c->ier, 0));
    }
    *wake = bool3(c->xc.wake_int);
}

/* ST16C1450/1550 ISR[5:4] with IER[5] = 1: TXRDY in [4], RXRDY in [5]. */
void ns_x_iir(const NsChannel *c, uint8_t *v, uint8_t *k) {
    int tx;
    int rx;

    (void)k;
    if (exs_style_of(c) != XS_IER5 || bit_of(c->ier, 5) != LOGIC_1) {
        return;
    }
    tx = exs_txrdy_pin(c) == LOGIC_0;
    rx = exs_rxrdy_pin(c) == LOGIC_0;
    if (exs_xd(c)->isr_inverted) {
        tx = !tx;
        rx = !rx;
    }
    *v = (uint8_t)((*v & ~0x30u) | (tx ? 0x10u : 0) | (rx ? 0x20u : 0));
}

void ns_x_iir_read(NsChannel *c, uint8_t iir) {
    const NsDevice *d = c->owner->dev;
    const NsExarDevice *x = d->x;

    if ((iir & 0x3F) == 0x10) {
        c->iir_special = 0;
        if (x->style != XS_M752) {
            c->iir_xoff = 0;   /* Exar: an ISR read clears the Xoff interrupt (TX stays halted) */
        }
    }
    if ((iir & 0x3F) == 0x20 &&
        (d->ctsrts_clear == CLEAR_IIR || d->ctsrts_clear == CLEAR_IIR_OR_MSR)) {
        c->iir_ctsrts = 0;
    }
    /* ST16C1450/1550: an ISR read alone does not clear TXRDY (only a THR write,
       or IER[1] = 0 after the read). */
    if ((iir & 0x0F) == 0x02 && x->thr_ier1 && exs_kb(c->ier, 1)) {
        c->thre_int = 1;
    }
    /* The wake-up indicator clears with an ISR read (XR16L784: an INT0 read). */
    if ((iir & 0x3F) == 0x01 && !(x->style == XS_5G && !x->v798)) {
        c->xc.wake_int = 0;
    }
}

void ns_x_status_read(NsChannel *c, int msr) {
    const NsDevice *d = c->owner->dev;

    if (!msr && d->special_clear == CLEAR_LSR) {
        c->iir_special = 0;
    }
    if (msr && (d->ctsrts_clear == CLEAR_MSR || d->ctsrts_clear == CLEAR_IIR_OR_MSR)) {
        c->iir_ctsrts = 0;
    }
}

void ns_x_error_in(NsChannel *c) {
    if (exs_style_of(c) == XS_5G) {
        c->xc.ls_new = 1;
    }
}

/* XR16L784: the timeout interrupt clears when the FIFO empties, not on a read. */
int ns_x_timeout_holds(const NsChannel *c) {
    return exs_style_of(c) == XS_5G && !exs_xd(c)->v798 && c->rx_count > 0;
}

void ns_x_lsr_read(NsChannel *c) {
    c->xc.ls_new = 0;
    /* LSR[4] stays while RX is still low after the break (Exar text). */
    if (exs_xd(c)->break_hold && c->line.rx.phase == SERIAL_RX_WAIT_MARK) {
        c->lsr_err |= 0x10;
        c->lsr_err_k |= 0x10;
    }
}

/* ---- The 5G timer ------------------------------------------------------------------ */

void exs_timer_step(NsState *s) {
    NsExarState *g = &s->xs;

    if (!g->timer_running || s->in_reset || s->unknown) {
        return;
    }
    if (g->timer_reload < 2) {
        g->timer_int = LOGIC_X;   /* N = 0 or 1 (decisions.md) */
        return;
    }
    if (g->timer_count > 0 && --g->timer_count == 0) {
        g->timer_int = LOGIC_1;
        if (g->timer_oneshot) {
            g->timer_running = 0;
        } else {
            g->timer_count = g->timer_reload;
        }
        ns_update_outputs(s, CAUSE_SERIAL);
    }
}

static int all_asleep(const NsState *s) {
    unsigned i;

    for (i = 0; i < s->dev->channel_count; i++) {
        if (!s->ch[i].asleep) {
            return 0;
        }
    }
    return 1;
}

void ns_x_clock_edge(NsState *s, int rising) {
    /* The oscillator stops while every channel sleeps. */
    if (rising && s->dev->x->tmrck != NULL && !s->xs.timer_ext && !all_asleep(s)) {
        exs_timer_step(s);
    }
}

/* ---- Sleep, wake, PowerSave ------------------------------------------------------- */

static int sleep_enabled(const NsChannel *c) {
    switch (c->owner->dev->sleep) {
        case SLEEP_IER4_EXAR:
        case SLEEP_IER4_EXAR_ALL: return exs_kb(c->ier, 4);
        case SLEEP_GLOBAL: return (c->owner->xs.sleep >> c->index) & 1u;
        default: return 0;
    }
}

/* Nothing pending, no modem delta, RX idle high, nothing to send. */
static int channel_idle(const NsState *s, const NsChannel *c) {
    uint8_t iir;
    uint8_t iir_k;
    size_t rx = s->pins->channel[c->index][CH_RX];

    if (serial_rx_busy(&c->line.rx) || c->pending_count > 0 || c->rx_count > 0 ||
        c->tx_count > 0 || serial_tx_busy(&c->line.tx) || c->flow_out_count > 0 ||
        c->xc.imm_valid || exs_loopback(c) || (c->delta & 0x0F) != 0 || c->xc.wake_int ||
        c->xc.xoff_delay != 0) {
        return 0;
    }
    if (serial_level(s->sim, s->instance, rx, LOGIC_1) != LOGIC_1) {
        return 0;
    }
    if (exs_xd(c)->sleep_divisor && (!c->baud.known || c->baud.divisor == 0)) {
        return 0;
    }
    ns_iir_now(c, &iir, &iir_k);
    return iir_k == 0xFF && (iir & 1u);
}

void exs_update_isolation(NsState *s) {
    const NsExarPins *xp = s->pins->xp;
    int was = s->xs.isolated;

    s->xs.isolated = xp->pwrsave != SERIAL_NO_PIN && all_asleep(s) &&
                     serial_level(s->sim, s->instance, xp->pwrsave, LOGIC_X) == LOGIC_1;
    if (was != s->xs.isolated && s->sim != NULL) {
        serial_bus_update(&s->bus, s->sim, s->instance, &s->events);
    }
}

void ns_x_sleep_check(NsState *s, NsChannel *c) {
    const NsDevice *d = s->dev;
    unsigned i;

    if (d->x->style == XS_IER5) {
        c->asleep = c->xc.powered_down;   /* MCR[7] power-down */
        return;
    }
    if (d->sleep < SLEEP_IER4_EXAR || s->sim == NULL || s->in_reset || s->unknown) {
        return;
    }
    if (d->sleep == SLEEP_GLOBAL) {
        /* 5G: each channel with its SLEEP bit sleeps when idle. */
        if (!c->asleep && sleep_enabled(c) && channel_idle(s, c)) {
            c->asleep = 1;
        }
    } else {
        /* Exar IER[4]: the chip sleeps when every channel asks and is idle
           (one channel on the single parts). */
        for (i = 0; i < d->channel_count; i++) {
            if (!sleep_enabled(&s->ch[i]) || !channel_idle(s, &s->ch[i])) {
                return;
            }
        }
        for (i = 0; i < d->channel_count; i++) {
            s->ch[i].asleep = 1;
        }
    }
    exs_update_isolation(s);
}

static void wake_all(NsChannel *by) {
    NsState *s = by->owner;
    int was_asleep = 0;
    unsigned i;

    for (i = 0; i < s->dev->channel_count; i++) {
        was_asleep |= s->ch[i].asleep;
        s->ch[i].asleep = 0;
    }
    /* The wake-up indicator: INT asserts, ISR reads "none" (0x01). */
    if (was_asleep && (s->dev->x->wake_indicator || s->dev->sleep == SLEEP_GLOBAL)) {
        by->xc.wake_int = 1;
    }
    if (was_asleep) {
        exs_update_isolation(s);
    }
}

void ns_x_wake(NsChannel *c) {
    NsState *s = c->owner;
    const NsDevice *d = s->dev;
    unsigned threshold = ns_x_tx_threshold(c);

    /* A THR write (bytes waiting): the trigger bookkeeping, RS-485 direction. */
    if (c->tx_count > 0 || c->xc.imm_valid) {
        if (threshold != 0 && exs_tx_free(c) < threshold) {
            c->xc.filled = 1;
        }
        if (exs_rs485_enabled(c)) {
            c->xc.rs485 = 1;   /* before the start bit */
            c->xc.rs485_hold = 0;
        }
    }
    if (d->x->style == XS_IER5 || d->sleep < SLEEP_IER4_EXAR) {
        return;
    }
    /* Wake on a THR write or an RX start bit (RX low). */
    if (c->tx_count > 0 || c->xc.imm_valid ||
        serial_level(s->sim, s->instance, s->pins->channel[c->index][CH_RX], LOGIC_1) == LOGIC_0) {
        wake_all(c);
    }
}

int ns_x_cts_changed(NsChannel *c, LogicValue cts_active) {
    NsState *s = c->owner;
    int auto_cts = auto_cts_on(c);

    if (exs_flow_on_dtr(c)) {
        LogicValue dsr = not3(serial_level(s->sim, s->instance,
                                           s->pins->channel[c->index][CH_DSR], LOGIC_X));

        if (auto_cts && c->xc.dsr_last == LOGIC_1 && dsr == LOGIC_0 && exs_kb(c->ier, 7)) {
            c->iir_ctsrts = 1;   /* DSR pin low -> high */
        }
        c->xc.dsr_last = dsr;
    } else if (auto_cts && c->cts_last == LOGIC_1 && cts_active == LOGIC_0 && exs_kb(c->ier, 7)) {
        c->iir_ctsrts = 1;   /* CTS pin low -> high */
    }
    c->cts_last = cts_active;
    if (s->dev->sleep >= SLEEP_IER4_EXAR) {
        wake_all(c);   /* a modem input change wakes (also from PowerSave) */
    }
    return 1;
}

