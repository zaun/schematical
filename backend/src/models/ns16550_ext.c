/*
 * ns16550 enhanced features (plan-12-D section 3.1, Phase 2): the register
 * banks of the 650/752/NS-EFR styles (EFR, Xon/Xoff, TCR, TLR, FIFORdy, AFR,
 * DLF, DREV), software and hardware flow control, TX/RX trigger levels,
 * sleep, IrDA, RS-485 direction control, the prescaler and the fractional
 * divisor. The core (ns16550.c) calls in through the ns_ext_* hooks.
 *
 * Sources: plan-12-data/specs/ti-a.md (TL16C550C AFE), ti-b.md (TL16C750,
 * TL16C750E, TL16C754B/C), ti-c.md (TL16C2552, TL16C752C, NS16C2552/2752),
 * exar-d.md (SC16C650B, SC16C752B), with decisions.md.
 */

#include <string.h>

#include "ns16550_int.h"

/* ---- Small helpers -------------------------------------------------------------- */

static int bank_style(const NsDevice *d) {
    return d->extended == EXT_650 || d->extended == EXT_752 || d->extended == EXT_NS_EFR;
}

static int known_bit(Reg r, unsigned bit) {
    return bit_of(r, bit) == LOGIC_1;
}

/* EFR[4]: the enhanced functions. */
static int enhanced(const NsChannel *c) {
    return bank_style(c->owner->dev) && known_bit(c->efr, 4);
}

static int lcr_is_bf(const NsChannel *c) {
    return c->lcr.k == 0xFF && c->lcr.v == 0xBF;
}

static uint8_t word_mask(const NsChannel *c) {
    return (uint8_t)((1u << (5 + (c->lcr.v & 3u))) - 1u);
}

/* TCR/TLR access (752 style): EFR[4] = 1 and MCR[6] = 1. */
static LogicValue tcr_condition(const NsChannel *c) {
    if (c->owner->dev->extended != EXT_752) {
        return LOGIC_0;
    }
    return and3(bool3(enhanced(c)), bit_of(c->mcr, 6));
}

/* FIFORdy read access (752 with a FIFO ready register): MCR[2] = 1, MCR[4] = 0. */
static LogicValue fifordy_condition(const NsChannel *c) {
    if (c->owner->dev->extended != EXT_752) {
        return LOGIC_0;
    }
    return and3(bit_of(c->mcr, 2), not3(bit_of(c->mcr, 4)));
}

/* DLF access (TL16C750E): LCR[7] = 1, LCR != 0xBF, EFR[4] = 1, and not the
   TLR/FIFORdy MCR pattern x1x0x1xx. */
static int dlf_condition(const NsChannel *c) {
    return c->owner->dev->dlf && enhanced(c) &&
           !(known_bit(c->mcr, 6) && !known_bit(c->mcr, 4) && known_bit(c->mcr, 2));
}

/* ---- Reset ------------------------------------------------------------------------ */

void ns_ext_reset(NsState *s, NsChannel *c, int power_on) {
    const NsDevice *d = s->dev;

    c->efr = (Reg){0, 0xFF};
    c->tcr = (Reg){d->tcr_reset, 0xFF};
    c->tlr = (Reg){0, 0xFF};
    c->afr = (Reg){d->afr_reset, 0xFF};
    if (power_on) {
        Reg *xon[4] = {&c->xon1, &c->xon2, &c->xoff1, &c->xoff2};
        unsigned i;

        for (i = 0; i < 4; i++) {
            *xon[i] = d->xon_power[i] >= 0 ? (Reg){(uint8_t)d->xon_power[i], 0xFF} : (Reg){0, 0};
        }
        c->dlf = (Reg){0, 0xFF};
    }
    if (d->mcr_reset >= 0) {
        c->mcr = (Reg){(uint8_t)d->mcr_reset, 0xFF};
    }
    /* TL16C754B: MCR[7] = NOT CLKSEL at the end of reset. */
    if (d->prescaler == PRESCALE_MCR7_CLKSEL && s->sim != NULL) {
        LogicValue clksel = serial_level(s->sim, s->instance, s->pins->clock_select, LOGIC_X);

        c->mcr.v = (uint8_t)((c->mcr.v & 0x7F) | (clksel == LOGIC_0 ? 0x80 : 0));
        c->mcr.k = (uint8_t)((c->mcr.k & 0x7F) | (clksel == LOGIC_X ? 0 : 0x80));
    }
    c->tx_trigger = 0;
    c->thr_level_int = 0;
    c->tx_halted = 0;
    c->xoff_sent = 0;
    c->flow_out_count = 0;
    c->seq_wait = 0;
    c->iir_xoff = 0;
    c->iir_special = 0;
    c->iir_ctsrts = 0;
    c->rts_off = 0;
    c->rts_pin_last = LOGIC_X;
    c->cts_last = LOGIC_X;
    c->asleep = 0;
    c->low_power = 0;
    c->ir_low = 0;
    c->ir_last = LOGIC_X;
    c->rs485_on = 0;
    c->rs485_lead = 0;
    c->rs485_hold = 0;
    c->prescale_count = 0;
}

/* ---- Register decode ---------------------------------------------------------------- */

typedef enum {
    R_CORE, R_X, R_IGNORE, R_EFR, R_XON1, R_XON2, R_XOFF1, R_XOFF2, R_TCR, R_TLR, R_FIFORDY,
    R_AFR, R_DLF, R_DREV, R_DVID
} ExtReg;

static ExtReg decode(const NsChannel *c, unsigned address, int write) {
    const NsDevice *d = c->owner->dev;
    LogicValue lcr7 = bit_of(c->lcr, 7);
    unsigned a = address & 7u;

    if (bank_style(d) && lcr_is_bf(c)) {
        static const ExtReg BANK[8] = {R_CORE, R_CORE, R_EFR, R_CORE, R_XON1, R_XON2, R_XOFF1,
                                       R_XOFF2};

        /* Xoff1/Xoff2 and TCR/TLR both qualifying: X / ignored (decisions). */
        if ((a == 6 || a == 7) && tcr_condition(c) != LOGIC_0) {
            return write ? R_IGNORE : R_X;
        }
        return BANK[a];
    }
    if (bank_style(d) && c->lcr.k != 0xFF && lcr7 != LOGIC_0 &&
        (a == 2 || a >= 4)) {
        return write ? R_IGNORE : R_X;   /* could be the bank or not */
    }
    if (lcr7 == LOGIC_1) {
        int dl_zero = c->dll.k == 0xFF && c->dlm.k == 0xFF && c->dll.v == 0 && c->dlm.v == 0;

        if (a == 0 && !write && d->drev >= 0 && dl_zero) {
            return R_DREV;
        }
        if (a == 1 && !write && d->dvid >= 0 && dl_zero) {
            return R_DVID;
        }
        if (a == 2) {
            int afr = (d->afr == AFR_TI752 && (c->lcr.v & 0x60) == 0) ||
                      d->afr == AFR_TI2552 || d->afr == AFR_NS;

            if (afr) {
                return R_AFR;
            }
            if (bank_style(d) || d->afr != AFR_NONE) {
                return write ? R_IGNORE : R_X;   /* other LCR[7] = 1 values (decisions) */
            }
            return R_CORE;
        }
        if (d->dlab_hides_upper && a >= 4) {
            return write ? R_IGNORE : R_X;
        }
        if (a == 7 && d->dlf) {
            int dlf = dlf_condition(c);
            int tlr = tcr_condition(c) == LOGIC_1;

            if (dlf && tlr) {
                return write ? R_IGNORE : R_X;
            }
            if (dlf) {
                return R_DLF;
            }
        }
        if ((a == 6 || a == 7) && tcr_condition(c) != LOGIC_0) {
            if (d->tcr_needs_dlab0) {
                return write ? R_IGNORE : R_X;
            }
            return a == 6 ? R_TCR : R_TLR;
        }
        return R_CORE;
    }
    if (lcr7 == LOGIC_X) {
        return R_CORE;   /* the core reads X for DLAB-dependent addresses */
    }
    /* LCR[7] = 0. */
    if (a == 6) {
        LogicValue tcr = tcr_condition(c);

        if (tcr == LOGIC_X) {
            return write ? R_IGNORE : R_X;
        }
        return tcr == LOGIC_1 ? R_TCR : R_CORE;
    }
    if (a == 7) {
        LogicValue tlr = tcr_condition(c);
        LogicValue fr = fifordy_condition(c);

        if (write) {
            if (tlr == LOGIC_X) {
                return R_IGNORE;
            }
            return tlr == LOGIC_1 ? R_TLR : R_CORE;
        }
        if (tlr == LOGIC_X || fr == LOGIC_X || (tlr == LOGIC_1 && fr == LOGIC_1)) {
            return R_X;
        }
        if (tlr == LOGIC_1) {
            return R_TLR;
        }
        if (fr == LOGIC_1) {
            return R_FIFORDY;
        }
    }
    return R_CORE;
}

uint8_t ns_ext_fifo_ready(const NsState *s) {
    uint8_t v = 0;
    unsigned i;

    for (i = 0; i < s->dev->channel_count && i < 4; i++) {
        const NsChannel *c = &s->ch[i];
        unsigned threshold = ns_ext_tx_threshold(c);
        unsigned free_slots = (c->fifo_on ? (c->owner->dev->fifo) : 1) - c->tx_count;

        if (threshold == 0 ? c->tx_count == 0 : free_slots >= threshold) {
            v |= (uint8_t)(1u << i);
        }
        if (c->rx_count >= ns_rx_level(c) || c->timeout) {
            if (c->rx_count > 0) {
                v |= (uint8_t)(0x10u << i);
            }
        }
    }
    return v;
}

int ns_ext_peek(NsChannel *c, unsigned address, uint8_t *v, uint8_t *k) {
    const NsDevice *d = c->owner->dev;

    switch (decode(c, address, 0)) {
        case R_CORE: return 0;
        case R_X:
        case R_IGNORE: *v = 0; *k = 0; return 1;
        case R_EFR: *v = c->efr.v; *k = c->efr.k; return 1;
        case R_XON1: *v = c->xon1.v; *k = c->xon1.k; return 1;
        case R_XON2: *v = c->xon2.v; *k = c->xon2.k; return 1;
        case R_XOFF1: *v = c->xoff1.v; *k = c->xoff1.k; return 1;
        case R_XOFF2: *v = c->xoff2.v; *k = c->xoff2.k; return 1;
        case R_TCR: *v = c->tcr.v; *k = c->tcr.k; return 1;
        case R_TLR: *v = c->tlr.v; *k = c->tlr.k; return 1;
        case R_FIFORDY: *v = ns_ext_fifo_ready(c->owner); *k = 0xFF; return 1;
        case R_AFR:
            *v = c->afr.v;
            *k = c->afr.k;
            if (d->afr == AFR_TI2552 || d->afr == AFR_NS) {
                *v &= 0x07;
                *k |= 0xF8;
            }
            return 1;
        case R_DLF: *v = c->dlf.v; *k = c->dlf.k; return 1;
        case R_DREV: *v = (uint8_t)d->drev; *k = 0xFF; return 1;
        case R_DVID: *v = (uint8_t)d->dvid; *k = 0xFF; return 1;
    }
    return 0;
}

static void queue_flow(NsChannel *c, uint8_t ch) {
    if (c->flow_out_count < sizeof c->flow_out) {
        c->flow_out[c->flow_out_count++] = ch;
    }
}

/* Xon (1) or Xoff (0) characters for a transmit mode (EFR[3:2]). */
static void queue_flow_pair(NsChannel *c, unsigned txmode, int xon) {
    Reg first = xon ? c->xon1 : c->xoff1;
    Reg second = xon ? c->xon2 : c->xoff2;

    switch (txmode) {
        case 2: queue_flow(c, first.v); break;
        case 1: queue_flow(c, second.v); break;
        case 3: queue_flow(c, first.v); queue_flow(c, second.v); break;
        default: break;
    }
}

int ns_ext_write(NsState *s, NsChannel *c, unsigned address, uint8_t v, uint8_t k, Cause *cause) {
    const NsDevice *d = s->dev;
    unsigned i;

    *cause = CAUSE_WRITE;
    switch (decode(c, address, 1)) {
        case R_CORE: return 0;
        case R_X:
        case R_IGNORE: return 1;
        case R_EFR: {
            unsigned old_tx = (c->efr.v >> 2) & 3u;

            c->efr = (Reg){v, k};
            /* Software flow turned off or changed after an Xoff went out: the
               old Xon goes out (and the new Xoff if still above the level). */
            if (c->xoff_sent && ((v >> 2) & 3u) != old_tx) {
                queue_flow_pair(c, old_tx, 1);
                c->xoff_sent = 0;
            }
            ns_ext_levels(s, c);
            return 1;
        }
        case R_XON1: c->xon1 = (Reg){v, k}; return 1;
        case R_XON2: c->xon2 = (Reg){v, k}; return 1;
        case R_XOFF1: c->xoff1 = (Reg){v, k}; return 1;
        case R_XOFF2: c->xoff2 = (Reg){v, k}; return 1;
        case R_TCR: c->tcr = (Reg){v, k}; ns_ext_levels(s, c); return 1;
        case R_TLR: c->tlr = (Reg){v, k}; ns_ext_levels(s, c); return 1;
        case R_FIFORDY:
        case R_DREV:
        case R_DVID:
            return 1;
        case R_AFR:
            c->afr = (Reg){v, k};
            /* The concurrent-write bit is shared: either channel sets or clears it. */
            if (k & 1u) {
                s->conc = v & 1u;
                for (i = 0; i < d->channel_count; i++) {
                    s->ch[i].afr.v = (uint8_t)((s->ch[i].afr.v & ~1u) | (v & 1u));
                }
            }
            return 1;
        case R_DLF:
            c->dlf = (Reg){v, k};
            ns_ext_load_divisor(c);
            return 1;
    }
    return 0;
}

uint8_t ns_ext_ier_mask(const NsChannel *c) {
    switch (c->owner->dev->extended) {
        case EXT_750: return 0x3F;
        case EXT_650:
        case EXT_752:
        case EXT_NS_EFR: return enhanced(c) ? 0xFF : 0x0F;
        default: return 0x0F;
    }
}

uint8_t ns_ext_mcr_mask(const NsChannel *c) {
    const NsDevice *d = c->owner->dev;

    switch (d->extended) {
        case EXT_AFE:
        case EXT_750: return 0x3F;
        case EXT_650:
        case EXT_752:
        case EXT_NS_EFR: return enhanced(c) ? 0xFF : 0x1F;
        default: return d->prescaler != PRESCALE_NONE ? 0x9F : 0x1F;
    }
}

void ns_ext_fcr(NsChannel *c, uint8_t v, uint8_t k) {
    /* FCR[5:4]: the TX trigger, written only with EFR[4] = 1. */
    if (c->owner->dev->has_tx_trigger && enhanced(c) && (k & 0x30u) == 0x30u) {
        c->tx_trigger = (v >> 4) & 3u;
    }
}

/* ---- Trigger levels ------------------------------------------------------------------ */

unsigned ns_ext_rx_trigger(const NsChannel *c, unsigned fcr_level) {
    const NsDevice *d = c->owner->dev;
    unsigned tlr = (c->tlr.v >> 4) & 0x0Fu;

    if (d->tlr != 0 && tlr != 0) {
        return tlr * d->tlr;
    }
    return fcr_level;
}

unsigned ns_ext_tx_threshold(const NsChannel *c) {
    const NsDevice *d = c->owner->dev;
    unsigned tlr = c->tlr.v & 0x0Fu;

    if (!c->fifo_on || !d->has_tx_trigger || !enhanced(c)) {
        return 0;
    }
    if (d->tlr != 0 && tlr != 0) {
        return tlr * d->tlr;
    }
    return d->tx_trigger[c->tx_trigger & 3u];
}

/* ---- Flow control: receive side ------------------------------------------------------- */

static void resume_tx(NsChannel *c) {
    c->tx_halted = 0;
    c->iir_xoff = 0;
}

int ns_ext_rx_char(NsState *s, NsChannel *c, const SerialRxChar *rc) {
    const NsDevice *d = s->dev;
    uint8_t mask = word_mask(c);
    uint8_t data;
    unsigned rxmode = c->efr.v & 3u;
    unsigned txmode = (c->efr.v >> 2) & 3u;
    int sequential;
    int xon;
    int xoff;

    if (!bank_style(d)) {
        return 1;
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
            return 0;
        }
        if (waiting == 2 && data == (c->xoff2.v & mask)) {
            c->tx_halted = 1;
            c->iir_xoff = 1;
            return 0;
        }
        ns_store_char(c, &held);   /* not a pair: the first goes to the FIFO */
    }

    sequential = rxmode == 3 && (txmode == 3 || txmode == 0 || d->sw_flow == SW_NXP_AND);
    xon = 0;
    xoff = 0;
    if (enhanced(c) || d->extended == EXT_NS_EFR || bank_style(d)) {
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
    }
    if (xoff) {
        c->tx_halted = 1;
        c->iir_xoff = 1;
        return 0;
    }
    if (xon) {
        resume_tx(c);
        return 0;
    }
    /* Xon Any (MCR[5]): any other character resumes; it is stored. */
    if (c->tx_halted && known_bit(c->mcr, 5) && enhanced(c)) {
        resume_tx(c);
    }
    /* Special character (EFR[5]): matching Xoff2 flags IIR[4]; it is stored. */
    if (d->special_char && known_bit(c->efr, 5) && data == (c->xoff2.v & mask) &&
        (!d->special_needs_no_flow || (c->efr.v & 0x0Fu) == 0)) {
        c->iir_special = 1;
    }
    return 1;
}

/* ---- Flow control: levels (auto-RTS, Xoff/Xon sending) -------------------------------- */

static unsigned trigger_index(const NsChannel *c) {
    return c->trigger & 3u;
}

static int auto_rts_enabled(const NsChannel *c) {
    const NsDevice *d = c->owner->dev;

    switch (d->auto_flow) {
        case FLOW_AFE:
            return c->fifo_on && known_bit(c->mcr, 5) && known_bit(c->mcr, 1);
        case FLOW_TCR:
            return known_bit(c->efr, 6);
        case FLOW_TABLE:
            return known_bit(c->efr, 6) &&
                   (d->extended != EXT_NS_EFR || known_bit(c->mcr, 1));
        default:
            return 0;
    }
}

static int auto_cts_enabled(const NsChannel *c) {
    const NsDevice *d = c->owner->dev;

    switch (d->auto_flow) {
        case FLOW_AFE:
            return c->fifo_on && known_bit(c->mcr, 5);
        case FLOW_TCR:
        case FLOW_TABLE:
            return known_bit(c->efr, 7);
        default:
            return 0;
    }
}

void ns_ext_levels(NsState *s, NsChannel *c) {
    const NsDevice *d = s->dev;
    unsigned level = c->rx_count;
    unsigned txmode = (c->efr.v >> 2) & 3u;
    unsigned m = d->tlr != 0 ? d->tlr : 4;
    int was_off = c->rts_off;

    /* Auto-RTS. */
    if (auto_rts_enabled(c)) {
        unsigned off;
        unsigned on;

        switch (d->auto_flow) {
            case FLOW_AFE: {
                unsigned trig = ns_rx_level(c);

                if (!d->fifo750 && trigger_index(c) == 3) {
                    /* Trigger 14: off once the 16th character is coming in;
                       back on with a free byte. */
                    if (level >= 15 && serial_rx_busy(&c->line.rx)) {
                        c->rts_off = 1;
                    } else if (level < 16 && !(level >= 15 && serial_rx_busy(&c->line.rx))) {
                        c->rts_off = c->rts_off && level >= 16;
                    }
                    break;
                }
                if (level >= trig) {
                    c->rts_off = 1;
                } else if (level == 0) {
                    c->rts_off = 0;
                }
                break;
            }
            case FLOW_TCR:
                off = (c->tcr.v & 0x0Fu) * m;
                on = ((c->tcr.v >> 4) & 0x0Fu) * m;
                if (off != 0 && level >= off) {
                    c->rts_off = 1;
                } else if (level <= on) {
                    c->rts_off = 0;
                }
                break;
            case FLOW_TABLE:
                off = d->rts_table[trigger_index(c)][0];
                on = d->rts_table[trigger_index(c)][1];
                if (level >= off) {
                    c->rts_off = 1;
                } else if (level <= on) {
                    c->rts_off = 0;
                }
                break;
            default:
                break;
        }
    } else {
        c->rts_off = 0;
    }
    if (!was_off && c->rts_off && known_bit(c->ier, 6)) {
        c->iir_ctsrts = 1;   /* RTS output low -> high */
    }

    /* Xoff / Xon sending. */
    if (bank_style(d) && txmode != 0 && c->fifo_on) {
        unsigned halt;
        unsigned resume;

        if (d->has_xoff_table) {
            halt = d->xoff_table[trigger_index(c)][0];
            resume = d->xoff_table[trigger_index(c)][1];
        } else {
            halt = (c->tcr.v & 0x0Fu) * m;
            resume = ((c->tcr.v >> 4) & 0x0Fu) * m;
        }
        if (!c->xoff_sent && halt != 0 && level >= halt) {
            queue_flow_pair(c, txmode, 0);
            c->xoff_sent = 1;
        } else if (c->xoff_sent && level <= resume) {
            queue_flow_pair(c, txmode, 1);
            c->xoff_sent = 0;
        }
    }
    ns_ext_sleep_check(s, c);
}

/* ---- Flow control: transmit side ------------------------------------------------------ */

int ns_ext_tx_flow(NsChannel *c, uint16_t *data) {
    if (c->flow_out_count == 0) {
        return 0;
    }
    *data = c->flow_out[0];
    memmove(c->flow_out, c->flow_out + 1, --c->flow_out_count);
    return 1;
}

int ns_ext_tx_blocked(NsState *s, NsChannel *c) {
    LogicValue cts;

    if (c->tx_halted) {
        return 1;   /* Xoff received */
    }
    if (auto_cts_enabled(c)) {
        cts = not3(serial_level(s->sim, s->instance, s->pins->channel[c->index][CH_CTS], LOGIC_X));
        if (bit_of(c->mcr, 4) == LOGIC_1) {
            cts = bit_of(c->mcr, 1);   /* loopback: RTS -> CTS */
        }
        if (cts != LOGIC_1) {
            return 1;
        }
    }
    /* RS-485: DTR leads the first start bit by one bit time. */
    if (s->dev->rs485 == RS485_AFR_DTR && known_bit(c->afr, 2)) {
        if (!c->rs485_on) {
            c->rs485_on = 1;
            c->rs485_lead = c->format.factor;
            c->rs485_hold = 0;
            return 1;
        }
        if (c->rs485_lead > 0) {
            c->rs485_lead--;
            return c->rs485_lead > 0;
        }
        c->rs485_hold = 0;
    }
    return 0;
}

/* ---- Interrupts ---------------------------------------------------------------------- */

LogicValue ns_ext_xoff_source(const NsChannel *c) {
    if (!bank_style(c->owner->dev)) {
        return LOGIC_0;
    }
    return and3(bool3(c->iir_xoff || c->iir_special), bit_of(c->ier, 5));
}

LogicValue ns_ext_ctsrts_source(const NsChannel *c) {
    if (!bank_style(c->owner->dev)) {
        return LOGIC_0;
    }
    return and3(bool3(c->iir_ctsrts), or3(bit_of(c->ier, 6), bit_of(c->ier, 7)));
}

void ns_ext_iir_read(NsChannel *c, uint8_t iir) {
    const NsDevice *d = c->owner->dev;

    if ((iir & 0x3F) == 0x10 && d->special_clear == CLEAR_IIR) {
        c->iir_special = 0;
    }
    if ((iir & 0x3F) == 0x20 &&
        (d->ctsrts_clear == CLEAR_IIR || d->ctsrts_clear == CLEAR_IIR_OR_MSR)) {
        c->iir_ctsrts = 0;
    }
}

void ns_ext_status_read(NsChannel *c, int msr) {
    const NsDevice *d = c->owner->dev;

    if (!msr && d->special_clear == CLEAR_LSR) {
        c->iir_special = 0;
    }
    if (msr && (d->ctsrts_clear == CLEAR_MSR || d->ctsrts_clear == CLEAR_IIR_OR_MSR)) {
        c->iir_ctsrts = 0;
    }
}

int ns_ext_cts_changed(NsChannel *c, LogicValue cts_active) {
    const NsDevice *d = c->owner->dev;
    int auto_cts = auto_cts_enabled(c);

    if (auto_cts && c->cts_last == LOGIC_1 && cts_active == LOGIC_0 && known_bit(c->ier, 7) &&
        bank_style(d)) {
        c->iir_ctsrts = 1;   /* CTS pin low -> high */
    }
    c->cts_last = cts_active;
    ns_ext_wake(c);
    /* TL16C550C/554A/750: no ΔCTS while auto-CTS is on (decisions.md). */
    return !(auto_cts && d->auto_flow == FLOW_AFE);
}

/* ---- Pins ------------------------------------------------------------------------------ */

LogicValue ns_ext_rts(const NsChannel *c, LogicValue normal) {
    if (auto_rts_enabled(c) && bit_of(c->mcr, 4) != LOGIC_1) {
        return bool3(c->rts_off);
    }
    return normal;
}

LogicValue ns_ext_dtr(const NsChannel *c, LogicValue normal) {
    if (c->owner->dev->rs485 == RS485_AFR_DTR && known_bit(c->afr, 2)) {
        return bool3(c->rs485_on);
    }
    return normal;
}

static int irda_on(const NsChannel *c) {
    switch (c->owner->dev->irda) {
        case IRDA_MCR6: return known_bit(c->mcr, 6);
        case IRDA_AFR1: return known_bit(c->afr, 1);
        default: return 0;
    }
}

LogicValue ns_ext_tx_pin(const NsChannel *c, LogicValue normal) {
    unsigned f;
    unsigned at;

    if (!irda_on(c)) {
        return normal;
    }
    /* SIR: a 1 (and idle) is no light (TX low); a 0 is a pulse from the 7th
       to the 10th 16x clock of its bit (3/16 of a bit). */
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

LogicValue ns_ext_rx_decode(NsChannel *c, LogicValue pin) {
    LogicValue level;

    /* RS-485/IrDA with RCVEN = 0 (TI AFR): the receiver is off while sending. */
    if (c->owner->dev->afr == AFR_TI752 && !known_bit(c->afr, 4) &&
        (known_bit(c->afr, 2) || known_bit(c->afr, 1)) && serial_tx_busy(&c->line.tx)) {
        return LOGIC_1;
    }
    if (!irda_on(c)) {
        return pin;
    }
    /* A falling edge (the end of a light pulse either way up) is a 0 for 16 clocks. */
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

void ns_ext_pins(NsState *s, NsChannel *c, Cause cause) {
    const NsDevice *d = s->dev;
    size_t op = s->pins->op[c->index];
    size_t mf = s->pins->mf[c->index];
    LogicValue loop = bit_of(c->mcr, 4);
    LogicValue op_level = or3(not3(bit_of(c->mcr, 3)), loop);
    SimTime delay = cause == CAUSE_MCR_WRITE ? c->owner->grade->t[6] : 0;

    if (s->unknown) {
        ns_drive(s, op, LOGIC_X, 0);
        ns_drive(s, mf, LOGIC_X, 0);
        return;
    }
    if (s->in_reset) {
        op_level = bool3(d->mcr_reset < 0 || !(d->mcr_reset & 0x08));
    }
    ns_drive(s, op, op_level, delay);
    if (mf != SERIAL_NO_PIN) {
        unsigned sel = (c->afr.v >> 1) & 3u;

        switch (sel) {
            case 0: ns_drive(s, mf, op_level, delay); break;
            case 1: break;   /* BAUDOUT: driven on clock edges by the core */
            case 2: {
                int mode1 = c->fifo_on && c->dma_mode;

                ns_drive(s, mf, mode1 ? bool3(!c->rxrdy1) : bool3(c->rx_count == 0), 0);
                break;
            }
            default:
                ns_drive(s, mf, d->afr == AFR_NS ? LOGIC_1 : LOGIC_X, 0);
                break;
        }
    }
}

/* ---- Clocks ------------------------------------------------------------------------------- */

int ns_ext_prescale(NsState *s, NsChannel *c, int rising) {
    (void)s;
    if (c->asleep) {
        return 0;
    }
    if (c->owner->dev->prescaler != PRESCALE_NONE && known_bit(c->mcr, 7)) {
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

void ns_ext_load_divisor(NsChannel *c) {
    unsigned frac = 0;

    if (c->owner->dev->dlf && enhanced(c) && c->dlf.k == 0xFF) {
        frac = c->dlf.v & 0x3Fu;
    }
    serial_baud_load(&c->baud, (uint32_t)c->dll.v | ((uint32_t)c->dlm.v << 8),
                     c->dll.k == 0xFF && c->dlm.k == 0xFF);
    c->baud.frac = frac;
    c->baud.frac_den = 64;
}

unsigned ns_ext_factor(const NsChannel *c) {
    if (c->owner->dev->dlf && enhanced(c) && known_bit(c->dlf, 7)) {
        return 8;
    }
    return 16;
}

/* ---- Sleep ---------------------------------------------------------------------------------- */

void ns_ext_wake(NsChannel *c) {
    c->asleep = 0;
}

void ns_ext_sleep_check(NsState *s, NsChannel *c) {
    const NsDevice *d = s->dev;
    int want;
    uint8_t iir;
    uint8_t iir_k;

    switch (d->extended) {
        case EXT_750:
            want = known_bit(c->ier, 4) || known_bit(c->ier, 5);
            break;
        case EXT_650:
        case EXT_752:
        case EXT_NS_EFR:
            want = d->sleep != SLEEP_NONE && enhanced(c) && known_bit(c->ier, 4);
            break;
        default:
            want = 0;
    }
    if (!want || c->asleep) {
        return;
    }
    /* Idle: receiver idle, nothing to send, RX FIFO empty, not in loopback,
       no modem delta, and no interrupt pending except THR / timeout. */
    if (serial_rx_busy(&c->line.rx) || c->pending_count > 0 || c->tx_count > 0 ||
        serial_tx_busy(&c->line.tx) || c->rx_count > 0 || known_bit(c->mcr, 4) ||
        (c->delta & 0x0F) != 0 || c->flow_out_count > 0) {
        return;
    }
    if (d->extended == EXT_NS_EFR && c->baud.divisor == 0) {
        return;
    }
    ns_iir_now(c, &iir, &iir_k);
    if (iir_k != 0xFF || ((iir & 1u) == 0 && (iir & 0x0E) != 0x02 && (iir & 0x0E) != 0x0C)) {
        return;
    }
    if (d->sleep == SLEEP_IER4_ALL) {
        unsigned i;

        for (i = 0; i < d->channel_count; i++) {
            if (!known_bit(s->ch[i].ier, 4)) {
                return;
            }
        }
    }
    c->asleep = 1;
}
