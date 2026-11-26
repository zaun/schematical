/* models/ns16550_io.c: part of the model in models/ns16550.c (split by plan-68-I/J). */
#include "ns16550_core_int.h"
/* ---- Receiving ------------------------------------------------------------------------------ */

static void deliver_char(NsChannel *c, const SerialRxChar *rc) {
    unsigned d = depth(c);

    c->idle_ticks = 0;
    c->timeout = 0;
    /* NS16C2552/2752: the LSR error bits also reset when the next valid
       character loads into the RBR (LSR text). */
    if (c->owner->dev->lsr_clear_on_good && !nsc_char_has_error(rc) && c->rx_count == 0) {
        c->lsr_err &= 0x02;
        c->lsr_err_k |= 0x1C;
    }
    if (c->rx_count >= d) {
        /* Overrun: 16450 mode overwrites the RBR; FIFO mode drops the
           character in the shift register. */
        c->lsr_err |= 0x02;
        c->lsr_err_k |= 0x02;
        if (!c->fifo_on) {
            c->rx[c->rx_head] = *rc;
            c->rbr = (Reg){(uint8_t)rc->data, (uint8_t)rc->known};
            nsc_show_errors(c, rc);
        }
        return;
    }
    c->rx[(c->rx_head + c->rx_count) % NS_MAX_FIFO] = *rc;
    c->rx_count++;
    if (c->rx_count == 1) {
        c->rbr = (Reg){(uint8_t)rc->data, (uint8_t)rc->known};
        nsc_show_errors(c, rc);
    }
    if (c->fifo_on && nsc_char_has_error(rc)) {
        c->lsr7 = 1;
        if (c->owner->dev->x != NULL) {
            ns_x_error_in(c);   /* plan-12-E */
        }
    }
}

/* A character goes to the RBR/FIFO (also a held flow character). */
void ns_store_char(NsChannel *c, const SerialRxChar *rc) {
    deliver_char(c, rc);
}

/* A received character after its delay: flow control first. */
static void deliver(NsState *s, NsChannel *c, const SerialRxChar *rc) {
    c->idle_ticks = 0;
    if (ns_ext_rx_char(s, c, rc)) {
        deliver_char(c, rc);
    }
    ns_ext_levels(s, c);
}

static void on_received(void *context, const SerialRxChar *rc) {
    NsChannel *c = context;

    if (c->pending_count < sizeof c->pending / sizeof c->pending[0]) {
        c->pending[c->pending_count].c = *rc;
        c->pending[c->pending_count].ticks = c->fifo_on ? 3 : 1;
        c->pending_count++;
    }
}

void nsc_rx_tick(NsState *s, NsChannel *c) {
    SerialRxEvents events = {c, on_received, NULL};
    LogicValue pin_level;
    LogicValue level;
    unsigned i;
    int changed = 0;

    if (s->in_reset || s->unknown) {
        return;
    }
    /* Characters whose delay ends now reach the RBR/FIFO. */
    for (i = 0; i < c->pending_count;) {
        if (--c->pending[i].ticks == 0) {
            deliver(s, c, &c->pending[i].c);
            memmove(&c->pending[i], &c->pending[i + 1],
                    (c->pending_count - i - 1) * sizeof c->pending[0]);
            c->pending_count--;
            changed = 1;
        } else {
            i++;
        }
    }
    pin_level = serial_level(s->sim, s->instance, s->pins->channel[c->index][CH_RX], LOGIC_X);
    pin_level = ns_ext_rx_decode(c, pin_level);
    level = serial_line_rx_input(&c->line, s->sim, s->instance, pin_level, &c->format);
    if (nsc_loopback(c)) {
        level = c->tsr;   /* the transmitter's output, before break */
    }
    serial_rx_tick(&c->line.rx, level, &c->format, &events);

    /* Timeout: 4 character times (from RCLK) plus 8 RCLKs with data waiting;
       "4w12": 4 characters plus 12 bit times. */
    /* TI AFR RCVEN = 0: no timeout at all (Table 21). */
    if (c->fifo_on && c->rx_count > 0 && !c->timeout &&
        !(s->dev->afr == AFR_TI752 && bit_of(c->afr, 4) == LOGIC_0) &&
        (s->dev->bridge == NULL || ns_bridge_timeout_on(c))) {
        uint64_t limit = 4ull * serial_format_char_bits(&c->format) * c->format.factor +
                         s->dev->timeout_extra;

        if (s->dev->timeout_4w12) {
            limit = (4ull * serial_format_char_bits(&c->format) + 12) * c->format.factor;
        }

        if (++c->idle_ticks >= limit) {
            c->timeout = 1;
            changed = 1;
        }
    }
    if (changed || s->dev->auto_flow == FLOW_AFE) {
        ns_ext_levels(s, c);
        nsc_update_dma_latches(c);
        nsc_update_channel_outputs(s, c, CAUSE_SERIAL);
        serial_bus_refresh(&s->bus, s->sim, s->instance, &s->events);
    }
}

/* ---- Transmitting -------------------------------------------------------------------------- */

static int tx_fetch(void *context, uint16_t *data, uint16_t *known) {
    NsChannel *c = context;
    unsigned threshold = ns_ext_tx_threshold(c);

    /* Xon/Xoff go first, even while the data is held. */
    if (ns_ext_tx_flow(c, data)) {
        *known = 0xFF;
        return 1;
    }
    if (c->tx_count == 0 || ns_ext_tx_blocked(c->owner, c)) {
        return 0;
    }
    *data = c->tx_data[c->tx_head];
    *known = c->tx_known[c->tx_head];
    /* plan-12-I: XR20M1280 SFR[7], a byte sent with its 9th bit 1 (the
       format is restored after this tick, tx_tick). */
    if (c->owner->dev->bridge != NULL && ns_bridge_tx_fetch(c)) {
        c->format.parity = SERIAL_PARITY_MARK;
    }
    c->tx_head = (c->tx_head + 1) % NS_MAX_FIFO;
    c->tx_count--;
    /* TX trigger: the THR interrupt as the free spaces reach the level. */
    if (threshold != 0 && nsc_tx_free(c) >= threshold && nsc_tx_free(c) - 1 < threshold) {
        c->thr_level_int = 1;
        c->thre_int = 1;
    }
    if (c->tx_count == 0) {
        /* PC16550D p18: in FIFO mode the empty indication is delayed one
           character (to the last stop bit) unless two bytes were in the
           FIFO at once since the last THRE, or FCR[0] just changed. */
        if (c->fifo_on && !c->two_seen && !c->immediate) {
            c->thre_wait = 1;
        } else {
            ns_set_thre(c);
        }
    }
    return 1;
}

static void tx_last_stop(void *context) {
    NsChannel *c = context;

    if (c->thre_wait) {
        ns_set_thre(c);
    }
}

static void tx_done(void *context, uint16_t data, uint16_t known) {
    NsChannel *c = context;

    (void)known;
    serial_line_log(&c->line, data);
}

void nsc_tx_tick(NsState *s, NsChannel *c) {
    SerialTxEvents events = {c, tx_fetch, tx_last_stop, tx_done};

    if (s->in_reset || s->unknown) {
        return;
    }
    c->tsr = serial_tx_tick(&c->line.tx, &c->format, &events);
    if (s->dev->bridge != NULL && c->format.parity == SERIAL_PARITY_MARK) {
        nsc_update_format(c);   /* plan-12-I: undo a 9th-bit override (tx_fetch) */
    }
    /* RS-485: DTR stays DLY bit times after the last stop bit, then drops. */
    if (c->rs485_on && c->rs485_lead == 0 && !serial_tx_busy(&c->line.tx) && c->tx_count == 0 &&
        c->flow_out_count == 0) {
        if (c->rs485_hold == 0) {
            c->rs485_hold = ((c->afr.v >> 5) & 7u) * c->format.factor + 1;
        }
        if (--c->rs485_hold == 0) {
            c->rs485_on = 0;
        }
    }
    if (s->dev->x != NULL) {
        ns_x_tick(s, c);   /* plan-12-E: RS-485 turnaround, Xoff delay, DMA requests */
    }
    ns_ext_levels(s, c);
    nsc_update_dma_latches(c);
    nsc_update_channel_outputs(s, c, CAUSE_SERIAL);
    serial_bus_refresh(&s->bus, s->sim, s->instance, &s->events);
}

/* ---- Registers -------------------------------------------------------------------------------- */

static void lsr_of(const NsChannel *c, uint8_t *v, uint8_t *k) {
    int temt = c->thre && c->tx_count == 0 && !serial_tx_busy(&c->line.tx);
    uint8_t err = c->lsr_err & 0x1E;
    uint8_t err_k = c->lsr_err_k & 0x1E;

    if (c->owner->dev->lsr_head) {
        /* LSR[4:2] are the head byte's error bits (0 when empty), not stored. */
        err &= 0x02;
        err_k = (uint8_t)((err_k & 0x02) | 0x1C);
        if (c->rx_count > 0) {
            uint8_t hv;
            uint8_t hk;

            nsc_char_lsr_bits(&c->rx[c->rx_head], &hv, &hk);
            err |= hv & hk;
            err_k = (uint8_t)((err_k & ~0x1C) | (hk | (hv & hk)));
        }
    }
    *v = (uint8_t)((c->rx_count > 0 ? 0x01 : 0) | err | (c->thre ? 0x20 : 0) |
                   (temt ? 0x40 : 0) | (c->fifo_on && c->lsr7 ? 0x80 : 0));
    *k = (uint8_t)(0xE1 | err_k);
}

static void msr_of(const NsChannel *c, uint8_t *v, uint8_t *k) {
    unsigned i;

    *v = c->delta & 0x0F;
    *k = c->delta_k & 0x0F;
    for (i = 0; i < 4; i++) {
        if (c->msr[i] == LOGIC_1) {
            *v |= (uint8_t)(0x10u << i);
        }
        if (c->msr[i] != LOGIC_X) {
            *k |= (uint8_t)(0x10u << i);
        }
    }
}

void nsc_reg_peek(NsChannel *c, unsigned address, uint8_t *v, uint8_t *k) {
    LogicValue dlab = bit_of(c->lcr, 7);

    if (ns_ext_peek(c, address, v, k)) {
        return;
    }
    switch (address & 7u) {
        case 0:
            if (dlab == LOGIC_X) {
                *v = 0; *k = 0;
            } else if (dlab == LOGIC_1) {
                *v = c->dll.v; *k = c->dll.k;
            } else {
                *v = c->rx_count > 0 ? (uint8_t)c->rx[c->rx_head].data : c->rbr.v;
                *k = c->rx_count > 0 ? (uint8_t)c->rx[c->rx_head].known : c->rbr.k;
            }
            break;
        case 1:
            if (dlab == LOGIC_X) {
                *v = 0; *k = 0;
            } else if (dlab == LOGIC_1) {
                *v = c->dlm.v; *k = c->dlm.k;
            } else {
                *v = c->ier.v; *k = c->ier.k;
            }
            break;
        case 2:
            nsc_iir_of(c, v, k);
            c->iir_seen = *v;
            c->iir_seen_k = *k;
            break;
        case 3: *v = c->lcr.v; *k = c->lcr.k; break;
        case 4: *v = c->mcr.v; *k = c->mcr.k; break;
        case 5:
            if (c->reading_status) {
                *v = c->shown;
                *k = c->shown_k;
                break;
            }
            lsr_of(c, v, k);
            c->lsr_seen = c->lsr_err;
            c->shown = *v;
            c->shown_k = *k;
            c->reading_status = c->owner->dev->status_defer;
            break;
        case 6:
            if (c->reading_status) {
                *v = c->shown;
                *k = c->shown_k;
                break;
            }
            msr_of(c, v, k);
            c->delta_seen = c->delta;
            c->shown = *v;
            c->shown_k = *k;
            c->reading_status = c->owner->dev->status_defer;
            break;
        default: *v = c->scr.v; *k = c->scr.k; break;
    }
}

/* A read ended: its side effects. Returns the cause for the output delays. */
Cause nsc_reg_read_done(NsChannel *c, unsigned address) {
    LogicValue dlab = bit_of(c->lcr, 7);
    uint8_t ext_v;
    uint8_t ext_k;

    Cause cause;

    /* plan-12-I: a bridge register with read side effects (IOState, GPIOLVL, FC). */
    if (c->owner->dev->bridge != NULL && ns_bridge_read_done(c, address, &cause)) {
        return cause;
    }
    /* A register of the enhanced banks: no core side effects. */
    if (ns_ext_peek(c, address, &ext_v, &ext_k)) {
        if (c->owner->dev->x != NULL) {
            ns_x_read_done(c, address);   /* plan-12-E */
        }
        return CAUSE_WRITE;
    }
    switch (address & 7u) {
        case 0:
            if (dlab != LOGIC_0) {
                return CAUSE_WRITE;
            }
            if (c->rx_count > 0) {
                c->rbr = (Reg){(uint8_t)c->rx[c->rx_head].data, (uint8_t)c->rx[c->rx_head].known};
                c->rx_head = (c->rx_head + 1) % NS_MAX_FIFO;
                c->rx_count--;
                /* The next character's errors show as it reaches the top. */
                if (c->rx_count > 0 && c->fifo_on && !c->owner->dev->lsr_head) {
                    nsc_show_errors(c, &c->rx[c->rx_head]);
                }
                if (c->owner->dev->lsr_head) {
                    unsigned i;

                    c->lsr7 = 0;
                    for (i = 0; i < c->rx_count; i++) {
                        if (nsc_char_has_error(&c->rx[(c->rx_head + i) % NS_MAX_FIFO])) {
                            c->lsr7 = 1;
                        }
                    }
                }
            }
            if (c->owner->dev->x == NULL || !ns_x_timeout_holds(c)) {   /* plan-12-E */
                c->timeout = 0;
            }
            c->idle_ticks = 0;
            nsc_update_dma_latches(c);
            return CAUSE_RBR_READ;
        case 2:
            /* An IIR read that reported THRE clears it. */
            if (c->iir_seen_k == 0xFF && (c->iir_seen & 0x0F) == 0x02) {
                c->thre_int = 0;
                c->thr_level_int = 0;
            }
            if (c->iir_seen_k == 0xFF) {
                ns_ext_iir_read(c, c->iir_seen);
            }
            return CAUSE_IIR_READ;
        case 5: {
            unsigned i;

            /* "statusReadDefer": bits that set during the read stay set.
               The 752 style stores only OE (LSR[4:2] are the head byte's). */
            if (c->owner->dev->lsr_head) {
                c->lsr_err &= (uint8_t)~0x02;
                c->lsr_err_k |= 0x02;
            } else if (c->owner->dev->status_defer) {
                c->lsr_err &= (uint8_t)~c->lsr_seen;
                c->lsr_err_k |= c->lsr_seen;
            } else {
                c->lsr_err = 0;
                c->lsr_err_k = 0xFF;
            }
            c->reading_status = 0;
            /* LSR[7] stays while errors remain further down the FIFO
               ("lsr7Clear": "lsrRead" clears it on any LSR read). */
            c->lsr7 = 0;
            for (i = 1; i < c->rx_count && !c->owner->dev->lsr7_on_read; i++) {
                if (nsc_char_has_error(&c->rx[(c->rx_head + i) % NS_MAX_FIFO])) {
                    c->lsr7 = 1;
                }
            }
            /* The 752 style: LSR[7] while any errored byte remains, the head too. */
            if (c->owner->dev->lsr_head && c->rx_count > 0 &&
                nsc_char_has_error(&c->rx[c->rx_head])) {
                c->lsr7 = 1;
            }
            if (c->owner->dev->x != NULL) {
                ns_x_lsr_read(c);   /* plan-12-E: break hold, 5G new-error interrupt */
            }
            ns_ext_status_read(c, 0);
            return CAUSE_LSR_READ;
        }
        case 6:
            if (c->owner->dev->status_defer) {
                c->delta &= (uint8_t)~c->delta_seen;
                c->delta_k |= c->delta_seen;
            } else {
                c->delta = 0;
                c->delta_k = 0xFF;
            }
            c->reading_status = 0;
            ns_ext_status_read(c, 1);
            return CAUSE_MSR_READ;
        default:
            return CAUSE_WRITE;
    }
}

static void write_thr(NsChannel *c, uint8_t data, uint8_t known) {
    unsigned d = depth(c);

    if (c->tx_count < d) {
        unsigned at = (c->tx_head + c->tx_count) % NS_MAX_FIFO;

        c->tx_data[at] = data;
        c->tx_known[at] = known;
        c->tx_count++;
        if (c->owner->dev->bridge != NULL) {
            ns_bridge_thr_written(c, at);   /* plan-12-I: 9th bit, send immediately */
        }
    } else if (!c->fifo_on) {
        /* 16450: a THR write while it is full replaces the waiting byte. */
        c->tx_data[c->tx_head] = data;
        c->tx_known[c->tx_head] = known;
    }
    if (c->tx_count >= 2) {
        c->two_seen = 1;
    }
    c->thre = 0;
    c->thre_wait = 0;
    c->thre_int = 0;
    c->thr_level_int = 0;
    c->rs485_hold = 0;   /* any THR write cancels the RS-485 turn-off delay */
    ns_ext_wake(c);
    nsc_update_dma_latches(c);
}

static void write_fcr(NsChannel *c, uint8_t data, uint8_t known) {
    int on = data & 1u;

    if (c->owner->dev->fifo == 0) {
        return;   /* 16450: no FCR */
    }
    if (!(known & 1u)) {
        /* FIFO enable unknown: the FIFOs are cleared and the mode is X. */
        ns_clear_rx(c);
        ns_clear_tx(c);
        c->fifo_on = 0;
        c->fifo_x = 1;
        return;
    }
    c->fifo_x = 0;
    if (on != c->fifo_on) {
        ns_clear_rx(c);
        ns_clear_tx(c);
        c->fifo_on = on;
        c->immediate = 1;
    }
    if (!on) {
        return;   /* "FCR0 must be a 1 when other FCR bits are written" */
    }
    /* TL16C750: FCR[5] picks 64-byte mode, writable only while LCR[7] = 1. */
    if (c->owner->dev->fifo750 && bit_of(c->lcr, 7) == LOGIC_1 && (known & 0x20u)) {
        int mode64 = (data >> 5) & 1u;

        if (mode64 != c->fifo64) {
            ns_clear_rx(c);
            ns_clear_tx(c);
            c->fifo64 = mode64;
        }
    }
    if (data & 2u) {
        ns_clear_rx(c);
    }
    if (data & 4u) {
        ns_clear_tx(c);
    }
    c->dma_mode = c->owner->dev->no_dma_mode1 ? 0 : (data >> 3) & 1u;
    c->trigger = (data >> 6) & 3u;
    ns_ext_fcr(c, data, known);
    nsc_update_dma_latches(c);
}

static void write_mcr(NsState *s, NsChannel *c, uint8_t data, uint8_t known) {
    LogicValue now[4];

    uint8_t mask = ns_ext_mcr_mask(c);

    c->mcr = (Reg){(uint8_t)((c->mcr.v & ~mask) | (data & mask)),
                   (uint8_t)((c->mcr.k & ~mask) | (known & mask))};
    if (s->dev->x != NULL) {
        ns_x_mcr_written(c, mask);   /* plan-12-E: prescaler override, send-immediate */
    }
    if (s->dev->prescaler != PRESCALE_NONE) {
        nsc_load_divisor(c);
    }
    nsc_modem_now(s, c, now);
    nsc_set_modem(c, now, 0x0F);
}

Cause nsc_reg_write(NsState *s, NsChannel *c, unsigned address, uint8_t data, uint8_t known) {
    LogicValue dlab = bit_of(c->lcr, 7);
    Cause cause;

    if (ns_ext_write(s, c, address, data, known, &cause)) {
        return cause;
    }
    switch (address & 7u) {
        case 0:
            if (dlab != LOGIC_0) {
                c->dll = (Reg){data, known};
                nsc_load_divisor(c);
            }
            if (dlab != LOGIC_1) {
                write_thr(c, data, known);
                return CAUSE_THR_WRITE;
            }
            return CAUSE_WRITE;
        case 1:
            if (dlab != LOGIC_0) {
                c->dlm = (Reg){data, known};
                nsc_load_divisor(c);
            }
            if (dlab != LOGIC_1) {
                int was = c->ier.v & 2u;
                uint8_t mask = ns_ext_ier_mask(c);
                unsigned threshold;

                c->ier = (Reg){(uint8_t)((c->ier.v & ~mask) | (data & mask)),
                               (uint8_t)((c->ier.k & ~mask) | (known & mask) | ~mask)};
                c->ier.v &= mask | (uint8_t)(c->ier.v & 0xF0);
                /* Enabling ETBEI while THRE is 1 (or the TX level is at its
                   trigger) raises the THR interrupt. */
                threshold = ns_ext_tx_threshold(c);
                if (!was && (c->ier.v & 2u) &&
                    (threshold == 0 ? c->thre : nsc_tx_free(c) >= threshold) &&
                    (s->dev->bridge == NULL || ns_bridge_thr_on_enable(c))) {
                    c->thre_int = 1;
                }
                ns_ext_sleep_check(s, c);
            }
            return CAUSE_WRITE;
        case 2:
            write_fcr(c, data, known);
            return CAUSE_WRITE;
        case 3:
            c->lcr = (Reg){data, known};
            nsc_update_format(c);
            return CAUSE_WRITE;
        case 4:
            write_mcr(s, c, data, known);
            return CAUSE_MCR_WRITE;
        case 5:
            /* Factory test only (p16): in 16450 mode LSR[4:1] take the
               written bits; in FIFO mode only OE can be written. */
            if (c->fifo_on) {
                c->lsr_err = (uint8_t)((c->lsr_err & ~0x02) | (data & 0x02));
                c->lsr_err_k = (uint8_t)((c->lsr_err_k & ~0x02) | (known & 0x02));
            } else {
                c->lsr_err = data & 0x1E;
                c->lsr_err_k = (uint8_t)(known | 0xE1);
            }
            return CAUSE_WRITE;
        case 6:
            return CAUSE_WRITE;   /* MSR is read-only */
        default:
            c->scr = (Reg){data, known};
            return CAUSE_WRITE;
    }
}

/* ---- Bus events ------------------------------------------------------------------------------ */

/* All of a set of selects active: 0, 1 or X (Z reads inactive). */
static LogicValue selects_active(const NsState *s, const size_t *low, unsigned low_count,
                                 const size_t *high, unsigned high_count) {
    LogicValue all = LOGIC_1;
    unsigned i;

    for (i = 0; i < low_count; i++) {
        all = and3(all, not3(serial_level(s->sim, s->instance, low[i], LOGIC_1)));
    }
    for (i = 0; i < high_count; i++) {
        all = and3(all, serial_level(s->sim, s->instance, high[i], LOGIC_0));
    }
    return all;
}

/* The decode for more than one channel: per-channel chip selects (several
   at once: a broadcast write when the device has one, else X), or one chip
   select plus a channel pin. */
LogicValue nsc_bus_select(void *context, const Sim *sim, const SimInstance *instance,
                             unsigned address, unsigned address_known, uint32_t *units) {
    NsState *s = context;
    const NsDevice *d = s->dev;
    const SerialBusPins *bp = &s->pins->bus;
    LogicValue common = selects_active(s, bp->select, bp->select_count, bp->select_high,
                                       bp->select_high_count);
    unsigned i;

    LogicValue x_result;

    (void)sim;
    (void)instance;
    (void)address;
    (void)address_known;
    *units = 1;
    if (d->x != NULL && ns_x_select(s, address, address_known, units, &x_result)) {
        return x_result;   /* plan-12-E: Motorola, PowerSave, 5G map, FSTAT, DMA */
    }
    if (d->lpt != NULL) {
        LogicValue port = not3(serial_level(s->sim, s->instance, s->pins->lpt->select, LOGIC_1));
        LogicValue uart = common;
        uint32_t on = 0;
        int unknown = 0;

        for (i = 0; i < d->channel_count; i++) {
            LogicValue v = selects_active(s, s->pins->chan_select[i], d->chan_select_count[i],
                                          s->pins->chan_select_high[i],
                                          d->chan_select_high_count[i]);

            if (!d->per_channel_select) {
                v = i == 0 ? uart : LOGIC_0;
            }
            if (v == LOGIC_1) {
                on |= 1u << i;
            }
            unknown |= v == LOGIC_X;
        }
        if (port == LOGIC_1) {
            on |= 1u << d->channel_count;
        }
        unknown |= port == LOGIC_X;
        if (unknown) {
            return LOGIC_X;
        }
        *units = on;
        return bool3(on != 0);
    }
    if (d->channel_pin != NULL) {
        LogicValue pin = serial_level(s->sim, s->instance, s->pins->channel_pin, LOGIC_X);

        if (common != LOGIC_0 && pin == LOGIC_X) {
            return LOGIC_X;
        }
        *units = (pin == LOGIC_1) == d->a_is_high ? 1u : 2u;
        return common;
    }
    if (d->per_channel_select) {
        uint32_t on = 0;
        int unknown = 0;

        for (i = 0; i < d->channel_count; i++) {
            LogicValue v = selects_active(s, s->pins->chan_select[i], d->chan_select_count[i],
                                          s->pins->chan_select_high[i],
                                          d->chan_select_high_count[i]);

            if (v == LOGIC_1) {
                on |= 1u << i;
            }
            unknown |= v == LOGIC_X;
        }
        if (common == LOGIC_0 || (on == 0 && !unknown)) {
            return LOGIC_0;
        }
        if (unknown || common == LOGIC_X) {
            return LOGIC_X;
        }
        if ((on & (on - 1u)) != 0 && d->broadcast == BROADCAST_NONE) {
            return LOGIC_X;   /* several selects at once, no broadcast: P2 */
        }
        *units = on;
        return LOGIC_1;
    }
    return common;
}

void nsc_bus_peek(void *context, unsigned unit, unsigned address, uint8_t *data, uint8_t *known) {
    NsState *s = context;

    *data = 0;
    *known = 0;
    if (s->unknown) {
        return;
    }
    if (s->dev->x != NULL && ns_x_unit_peek(s, unit, address, data, known)) {
        return;   /* plan-12-E */
    }
    if (unit == s->dev->channel_count && s->lpt != NULL) {
        ns_lpt_peek(s, address, data, known);
        return;
    }
    if (unit < s->dev->channel_count) {
        nsc_reg_peek(&s->ch[unit], address, data, known);
    }
}

void nsc_bus_read_done(void *context, unsigned unit, unsigned address) {
    NsState *s = context;
    Cause cause;

    if (s->unknown) {
        return;
    }
    if (s->dev->x != NULL && ns_x_unit_read_done(s, unit, address)) {
        return;   /* plan-12-E */
    }
    if (unit == s->dev->channel_count && s->lpt != NULL) {
        ns_lpt_read_done(s, address);
        return;
    }
    if (unit >= s->dev->channel_count) {
        return;
    }
    cause = nsc_reg_read_done(&s->ch[unit], address);
    ns_ext_levels(s, &s->ch[unit]);
    nsc_update_channel_outputs(s, &s->ch[unit], cause);
    nsc_update_shared_dma(s, cause);
}

void nsc_bus_write(void *context, unsigned unit, unsigned address, uint8_t data, uint8_t known) {
    NsState *s = context;
    Cause cause;

    unsigned i;

    if (s->unknown) {
        return;
    }
    if (s->dev->x != NULL && ns_x_unit_write(s, unit, address, data, known)) {
        return;   /* plan-12-E */
    }
    if (unit == s->dev->channel_count && s->lpt != NULL) {
        ns_lpt_write(s, address, data, known);
        return;
    }
    if (unit >= s->dev->channel_count) {
        return;
    }
    /* AFR concurrent write: every channel takes it. */
    if (s->dev->afr != AFR_NONE &&
        (s->dev->x != NULL ? ns_x_conc(s, unit) : s->conc)) {   /* plan-12-E */
        for (i = 0; i < s->dev->channel_count; i++) {
            cause = nsc_reg_write(s, &s->ch[i], address, data, known);
            nsc_update_channel_outputs(s, &s->ch[i], cause);
        }
        nsc_update_shared_dma(s, CAUSE_WRITE);
        return;
    }
    cause = nsc_reg_write(s, &s->ch[unit], address, data, known);
    nsc_update_channel_outputs(s, &s->ch[unit], cause);
    nsc_update_shared_dma(s, cause);
}

/* ---- plan-12-I: register access from a serial host port -------------------------------------- */

void ns_host_peek(NsState *s, unsigned ch, unsigned address, uint8_t *v, uint8_t *k) {
    nsc_bus_peek(s, ch, address, v, k);
}

void ns_host_read_done(NsState *s, unsigned ch, unsigned address) {
    nsc_bus_read_done(s, ch, address);
}

void ns_host_write(NsState *s, unsigned ch, unsigned address, uint8_t v, uint8_t k) {
    nsc_bus_write(s, ch, address, v, k);
}

void ns_tx_tick(NsState *s, NsChannel *c) {
    nsc_tx_tick(s, c);
}

void ns_rx_tick(NsState *s, NsChannel *c) {
    nsc_rx_tick(s, c);
}

void ns_update_format(NsChannel *c) {
    nsc_update_format(c);
}

