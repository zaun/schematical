#include "ns16550_exar_int.h"

/* ---- Small helpers -------------------------------------------------------------- */

const NsExarDevice *exs_xd(const NsChannel *c) {
    return c->owner->dev->x;
}

NsExarStyle exs_style_of(const NsChannel *c) {
    return c->owner->dev->x->style;
}

int exs_kb(Reg r, unsigned bit) {
    return bit_of(r, bit) == LOGIC_1;
}

static int lcr_bf(const NsChannel *c) {
    return c->lcr.k == 0xFF && c->lcr.v == 0xBF;
}

uint8_t exs_word_mask(const NsChannel *c) {
    return (uint8_t)((1u << (5 + (c->lcr.v & 3u))) - 1u);
}

int exs_has_bank(const NsChannel *c) {
    NsExarStyle st = exs_style_of(c);

    return st == XS_650 || st == XS_854 || st == XS_M752 || st == XS_5G;
}

/* The enhanced-bit gate: IER[5] (ier5) or EFR[4]. */
static int enhanced(const NsChannel *c) {
    /* XS_NONE: no EFR bank, EFR stays 0 from reset, so EFR[4] is 0 too. */
    return exs_style_of(c) == XS_IER5 ? exs_kb(c->ier, 5) : exs_kb(c->efr, 4);
}

int exs_loopback(const NsChannel *c) {
    return exs_kb(c->mcr, 4);
}

unsigned exs_tx_free(const NsChannel *c) {
    return depth(c) - c->tx_count;
}

int exs_mode1(const NsChannel *c) {
    return c->fifo_on && c->dma_mode;
}

/* The active-low RXRDY/TXRDY levels: the core rules for classes N, B, T, P. */
LogicValue exs_rxrdy_pin(const NsChannel *c) {
    return ns_rxrdy_level(c, exs_mode1(c));
}

LogicValue exs_txrdy_pin(const NsChannel *c) {
    return ns_txrdy_level(c, exs_mode1(c));
}

void exs_set_format_rate(NsChannel *c) {
    c->format.factor = ns_x_factor(c);
    c->format.sample = ns_x_start_sample(c, c->format.factor);
}

/* ---- Binding and part options ------------------------------------------------------ */

int ns_x_bind(KindContext *ctx, const NsDevice *d, const PartPin *pins, size_t pin_count,
              NsPins *p, const SerialAbsent *absent) {
    const NsExarDevice *x = d->x;
    NsExarPins *xp = kind_alloc(ctx, sizeof *xp);
    char key[32];
    unsigned i;

    if (xp == NULL) {
        return -1;
    }
#define BIND(k, name, slot) \
    if (serial_bind_pin_absent(ctx, pins, pin_count, k, name, absent, slot) != 0) return -1
    BIND("bus.modePin", x->mode_pin, &xp->mode);
    BIND("bus.rw", x->rw, &xp->rw);
    BIND("bus.strobe", x->strobe, &xp->strobe);
    BIND("bus.channelBits", x->chan_bit[0], &xp->chan_bit[0]);
    BIND("bus.channelBits", x->chan_bit[1], &xp->chan_bit[1]);
    BIND("bus.irq", x->irq, &xp->irq);
    BIND("interrupt", x->interrupt, &xp->interrupt);
    BIND("resetOut", x->reset_out, &xp->reset_out);
    BIND("powerSave.pin", x->pwrsave, &xp->pwrsave);
    BIND("fstat", x->fstat, &xp->fstat);
    BIND("dmaHost.aen", x->aen, &xp->aen);
    BIND("dmaHost.tc", x->tc, &xp->tc);
    BIND("chcClock", x->chc_clock, &xp->chc_clock);
    BIND("irdaStrap", x->enir, &xp->enir);
    BIND("timer.clock", x->tmrck, &xp->tmrck);
    for (i = 0; i < 4; i++) {
        BIND("dmaHost.dack", x->dack[i], &xp->dack[i]);
    }
    for (i = 0; i < 8; i++) {
        snprintf(key, sizeof key, "channels[%u].bclk", i);
        BIND(key, x->bclk[i], &xp->bclk[i]);
        snprintf(key, sizeof key, "channels[%u].irtx", i);
        BIND(key, x->irtx[i], &xp->irtx[i]);
    }
#undef BIND
    /* With no strobe named, Motorola's CS is the first chip select. */
    if (xp->strobe == SERIAL_NO_PIN && x->motorola_bus) {
        xp->strobe = d->select_count > 0 ? p->bus.select[0]
                   : d->chan_select_count[0] > 0 ? p->chan_select[0][0] : SERIAL_NO_PIN;
    }
    p->xp = xp;
    return 0;
}

/* Part key "intPin": the D variants (INTSEL bonded high) drive INT always. */
const void *ns_x_load_part(KindContext *ctx, const void *data, const cJSON *json) {
    static const char *const INT_PINS[] = {
        "driven", "out2-gated", "intn", "intsel", "mcr5-open-source", NULL
    };
    NsExarPart *part;
    int choice;

    (void)data;
    if (exs_opt_choice(ctx, json, "intPin", INT_PINS, &choice) != 0 || choice < 0) {
        return NULL;
    }
    if (choice == INT_INTN || choice == INT_INTSEL || choice == INT_MCR5_OPEN_SOURCE) {
        kind_error(ctx, "part \"intPin\" must be \"driven\" or \"out2-gated\"");
        return NULL;
    }
    part = kind_alloc(ctx, sizeof *part);
    if (part != NULL) {
        part->int_pin = choice;
    }
    return part;
}

int ns_x_open_drain(const void *data, const void *pins, const void *options, size_t pin) {
    const NsDevice *d = data;
    const NsPins *p = pins;

    (void)options;
    return d->x != NULL && p->xp != NULL && p->xp->interrupt == pin && pin != SERIAL_NO_PIN;
}

/* ---- Reset ------------------------------------------------------------------------- */

void exs_sample_straps(NsState *s, NsChannel *c) {
    const NsExarDevice *x = s->dev->x;

    if (s->sim == NULL) {
        return;
    }
    /* CLKSEL: GND = ÷4, VCC = ÷1 (absent = ÷1); MCR[7] still reads 0 (P1). */
    if (s->dev->prescaler == PRESCALE_CLKSEL) {
        LogicValue clksel = serial_level(s->sim, s->instance, s->pins->clock_select, LOGIC_1);

        c->xc.prescale4 = clksel == LOGIC_0;
    }
    /* 5G ENIR: MCR[6] (IrDA) preset. */
    if (x->enir != NULL) {
        LogicValue enir = serial_level(s->sim, s->instance, s->pins->xp->enir, LOGIC_X);

        c->mcr.v = (uint8_t)((c->mcr.v & ~0x40u) | (enir == LOGIC_1 ? 0x40u : 0));
        c->mcr.k = (uint8_t)((c->mcr.k & ~0x40u) | (enir == LOGIC_X ? 0 : 0x40u));
    }
}

static void reset_globals(NsState *s) {
    NsExarState *g = &s->xs;

    g->mode8x = 0;
    g->sleep = 0;
    g->regb = 0;
    g->timer_reload = 0;
    g->timer_count = 0;
    g->timer_running = 0;
    g->timer_oneshot = 0;
    g->timer_ext = 0;
    g->timer_int_en = 0;
    g->timer_int = LOGIC_0;
    g->timercntl = 0;
}

void ns_x_reset(NsState *s, NsChannel *c, int power_on) {
    const NsDevice *d = s->dev;
    const NsExarDevice *x = d->x;
    NsExarChannel *xc = &c->xc;

    c->efr = (Reg){0, 0xFF};
    c->tcr = (Reg){d->tcr_reset, 0xFF};
    c->tlr = (Reg){0, 0xFF};
    c->afr = (Reg){d->afr_reset, 0xFF};
    /* Xon/Xoff: reset to 0 (Exar reset tables), the XR16M752's at power-up only. */
    if (x->style != XS_M752 || power_on) {
        Reg *xon[4] = {&c->xon1, &c->xon2, &c->xoff1, &c->xoff2};
        unsigned i;

        for (i = 0; i < 4; i++) {
            *xon[i] = x->style == XS_M752 && d->xon_power[i] < 0 ? (Reg){0, 0}
                    : (Reg){(uint8_t)(d->xon_power[i] >= 0 ? d->xon_power[i] : 0), 0xFF};
        }
    }
    if (d->mcr_reset >= 0) {
        c->mcr = (Reg){(uint8_t)d->mcr_reset, 0xFF};
    }
    if (power_on && x->divisor_power_up >= 0) {
        c->dll = (Reg){(uint8_t)x->divisor_power_up, 0xFF};
        c->dlm = (Reg){0, 0xFF};
    }
    /* XR16V798: RST# and the soft reset set the divisor to 1 (Table 19). */
    if (x->style == XS_5G && x->v798) {
        c->dll = (Reg){1, 0xFF};
        c->dlm = (Reg){0, 0xFF};
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
    xc->dld = (Reg){0, 0xFF};
    xc->fctr = (Reg){0, 0xFF};
    xc->emsr = (Reg){0, 0xFF};
    xc->msrw = (Reg){0, 0xFF};
    xc->xchar = (Reg){0, 0xFF};
    xc->trg_rx = 0;
    xc->trg_tx = 0;
    xc->trg_rx_bad = x->style == XS_854;   /* TRG = 0: X when table D is chosen */
    xc->trg_tx_bad = x->style == XS_854;
    xc->prescale4 = 0;
    xc->filled = 0;
    xc->xoff_delay = 0;
    xc->wake_int = 0;
    xc->ls_new = 0;
    xc->powered_down = 0;
    xc->rs485 = 0;
    xc->rs485_hold = 0;
    xc->imm_armed = 0;
    xc->imm_valid = 0;
    xc->flvl_tx_next = 0;
    xc->rx_allow = 0;
    xc->drq_tx = 0;
    xc->drq_rx = 0;
    xc->tc_tx = 0;
    xc->tc_rx = 0;
    xc->dsr_last = LOGIC_X;
    exs_sample_straps(s, c);
    if (c->index == 0 && !s->xs.soft) {
        reset_globals(s);
        s->xs.isolated = 0;
    }
    ns_x_load_divisor(c);
}

/* The 0xBF bank could be selected: LCR's known bits all agree with 0xBF. */
static int bf_possible(const NsChannel *c) {
    return ((c->lcr.v ^ 0xBFu) & c->lcr.k) == 0;
}

static int dl_zero(const NsChannel *c) {
    return c->dll.k == 0xFF && c->dlm.k == 0xFF && c->dll.v == 0 && c->dlm.v == 0;
}

static XReg decode_bf(const NsChannel *c, unsigned a, int write) {
    const NsExarDevice *x = exs_xd(c);

    switch (a) {
        case 0:
            if (x->style == XS_854) {
                return write ? XR_TRG : XR_FC;
            }
            return XR_X;   /* decisions.md: 000/001 at 0xBF: X, writes ignored (P2) */
        case 1:
            return x->style == XS_854 ? XR_FCTR : XR_X;
        case 2:
            return XR_EFR;
        case 3:
            return XR_CORE;
        default:
            if (!write && x->xon_write_only) {
                return XR_CORE;   /* XR16L580: the reads follow the normal bank */
            }
            return (XReg)(XR_XON1 + (a - 4));
    }
}

/* 650 / 854 / m752 styles and the plain style, by LCR, EFR[4] and MCR. */
static XReg decode_bank(const NsChannel *c, unsigned a, int write) {
    const NsDevice *d = c->owner->dev;
    const NsExarDevice *x = d->x;
    LogicValue lcr7 = bit_of(c->lcr, 7);
    LogicValue efr4 = bit_of(c->efr, 4);

    if (exs_has_bank(c) && lcr_bf(c)) {
        return decode_bf(c, a, write);
    }
    if (exs_has_bank(c) && c->lcr.k != 0xFF && bf_possible(c) && a != 3) {
        return XR_X;   /* could be the 0xBF bank or not */
    }
    if (lcr7 == LOGIC_X && a != 3) {
        /* The core reads X for DLL/DLM vs RBR/IER; the rest is X where LCR[7]
           changes the register. */
        if (a <= 1) {
            return XR_CORE;
        }
        if ((a == 2 && (d->afr == AFR_EXAR || (x->dld && exs_has_bank(c)))) || d->dlab_hides_upper ||
            ((a == 5 || a == 6) && x->dlab_hides_status && !write)) {
            return XR_X;
        }
        lcr7 = LOGIC_0;
    }
    if (lcr7 == LOGIC_1) {
        switch (a) {
            case 0:
                return !write && d->drev >= 0 && dl_zero(c) ? XR_DREV : XR_CORE;
            case 1:
                return !write && d->dvid >= 0 && dl_zero(c) ? XR_DVID : XR_CORE;
            case 2:
                if (x->dld && exs_has_bank(c)) {
                    if (efr4 == LOGIC_X) {
                        return XR_X;
                    }
                    if (efr4 == LOGIC_1) {
                        return XR_DLD;
                    }
                }
                if (d->afr == AFR_EXAR) {
                    return XR_AFR;
                }
                return d->dlab_hides_upper ? XR_X : XR_CORE;
            case 3:
                return XR_CORE;
            default:
                if (d->dlab_hides_upper) {
                    return XR_X;
                }
                if (!write && (a == 5 || a == 6) && x->dlab_hides_status) {
                    return XR_X;
                }
                break;
        }
    }
    if (write && (a == 5 || a == 6) && x->status_read_only) {
        return XR_X;
    }
    if (x->style == XS_854 && a == 7 && lcr7 == LOGIC_0) {
        LogicValue swap = bit_of(c->xc.fctr, 6);

        if (swap == LOGIC_X) {
            return XR_X;
        }
        if (swap == LOGIC_1) {
            return write ? XR_EMSR : XR_FLVL;
        }
    }
    if (x->style == XS_M752 && (a == 6 || a == 7)) {
        LogicValue tcr = and3(efr4, bit_of(c->mcr, 6));
        LogicValue rdy = and3(bit_of(c->mcr, 2), not3(bit_of(c->mcr, 4)));

        if (a == 7 && !write) {
            /* FIFO Rdy wins over SPR and TLR (Table 12). */
            if (rdy == LOGIC_1) {
                return XR_FIFORDY;
            }
            if (rdy == LOGIC_X) {
                return XR_X;
            }
        }
        if (tcr == LOGIC_X) {
            return XR_X;
        }
        if (tcr == LOGIC_1) {
            return a == 6 ? XR_TCR : XR_TLR;
        }
    }
    return XR_CORE;
}

/* The 5G channel map (A3..A0); no 0xBF bank. */
static XReg decode_5g(const NsChannel *c, unsigned address, int write) {
    const NsExarDevice *x = exs_xd(c);
    unsigned a = address & 0x0Fu;
    LogicValue lcr7 = bit_of(c->lcr, 7);

    switch (a) {
        case 0:
            if (write && lcr7 == LOGIC_0 && c->xc.imm_armed) {
                return XR_THR_IMM;
            }
            return XR_CORE;
        case 2:
            if (x->dld) {
                if (lcr7 == LOGIC_X) {
                    return XR_X;
                }
                if (lcr7 == LOGIC_1) {
                    return XR_DLD;
                }
            }
            return XR_CORE;
        case 5:
            return write ? XR_X : XR_CORE;   /* LSR read-only (decisions.md) */
        case 6:
            return write ? XR_MSRW : XR_CORE;
        case 8: return XR_FCTR;
        case 9: return XR_EFR;
        case 10: return write ? XR_TXTRG : XR_TXCNT;
        case 11: return write ? XR_RXTRG : XR_RXCNT;
        case 12: return write ? XR_XOFF1 : XR_XCHAR;
        case 13: return write ? XR_XOFF2 : XR_X;
        case 14: return write ? XR_XON1 : XR_X;
        case 15: return write ? XR_XON2 : XR_X;
        default:
            return XR_CORE;
    }
}

XReg exs_decode(const NsChannel *c, unsigned address, int write) {
    switch (exs_style_of(c)) {
        case XS_5G: return decode_5g(c, address, write);
        default: return decode_bank(c, address & 7u, write);
    }
}

/* ---- Register values ----------------------------------------------------------------- */

/* M752 FIFO Rdy: 1 = ready = NOT the TXRDYx/RXRDYx pin (decisions.md). */
static uint8_t fifo_ready(const NsState *s) {
    uint8_t v = 0;
    unsigned i;

    for (i = 0; i < s->dev->channel_count && i < 4; i++) {
        if (exs_txrdy_pin(&s->ch[i]) == LOGIC_0) {
            v |= (uint8_t)(1u << i);
        }
        if (exs_rxrdy_pin(&s->ch[i]) == LOGIC_0) {
            v |= (uint8_t)(0x10u << i);
        }
    }
    return v;
}

int ns_x_peek(NsChannel *c, unsigned address, uint8_t *v, uint8_t *k) {
    const NsDevice *d = c->owner->dev;
    NsExarChannel *xc = &c->xc;
    int flvl_tx = (xc->emsr.v & 3u) == 1 || ((xc->emsr.v & 3u) == 3 && xc->flvl_tx_next);

    switch (exs_decode(c, address, 0)) {
        default:   /* XR_CORE, XR_THR_IMM (the core's registers) */
            return 0;
        case XR_EFR: *v = c->efr.v; *k = c->efr.k; return 1;
        case XR_XON1: *v = c->xon1.v; *k = c->xon1.k; return 1;
        case XR_XON2: *v = c->xon2.v; *k = c->xon2.k; return 1;
        case XR_XOFF1: *v = c->xoff1.v; *k = c->xoff1.k; return 1;
        case XR_XOFF2: *v = c->xoff2.v; *k = c->xoff2.k; return 1;
        case XR_DLD:
            *v = xc->dld.v;
            *k = xc->dld.k;
            if (exs_style_of(c) == XS_5G) {
                *v &= 0x0F;   /* b7-4 read 0 */
                *k |= 0xF0;
            }
            return 1;
        case XR_AFR: *v = c->afr.v & 0x07; *k = (uint8_t)(c->afr.k | 0xF8); return 1;
        case XR_DREV: *v = (uint8_t)d->drev; *k = 0xFF; return 1;
        case XR_DVID: *v = (uint8_t)d->dvid; *k = 0xFF; return 1;
        case XR_TCR: *v = c->tcr.v; *k = c->tcr.k; return 1;
        case XR_TLR: *v = c->tlr.v; *k = c->tlr.k; return 1;
        case XR_FIFORDY: *v = fifo_ready(c->owner); *k = 0xFF; return 1;
        case XR_FCTR: *v = xc->fctr.v; *k = xc->fctr.k; return 1;
        case XR_FC:
            /* FC: RX count (FCTR[7] = 0) or TX count. */
            if (bit_of(xc->fctr, 7) == LOGIC_X) {
                *v = 0; *k = 0;
            } else {
                *v = (uint8_t)(exs_kb(xc->fctr, 7) ? c->tx_count : c->rx_count);
                *k = 0xFF;
            }
            return 1;
        case XR_FLVL:
            /* EMSR[1:0]: 01 TX count, 11 RX and TX in turn, else RX. */
            if ((xc->emsr.k & 3u) != 3u) {
                *v = 0; *k = 0;
            } else {
                *v = (uint8_t)(flvl_tx ? c->tx_count : c->rx_count);
                *k = 0xFF;
            }
            return 1;
        case XR_TXCNT: *v = (uint8_t)c->tx_count; *k = 0xFF; return 1;
        case XR_RXCNT: *v = (uint8_t)c->rx_count; *k = 0xFF; return 1;
        case XR_XCHAR: *v = xc->xchar.v; *k = xc->xchar.k; return 1;
        case XR_X:
        case XR_TRG:
        case XR_EMSR:
        case XR_TXTRG:
        case XR_RXTRG:
        case XR_MSRW:
            *v = 0; *k = 0;
            return 1;
    }
}

/* A read of one of the registers above ended. */
void ns_x_read_done(NsChannel *c, unsigned address) {
    NsExarChannel *xc = &c->xc;

    switch (exs_decode(c, address, 0)) {
        case XR_FLVL:
            if ((xc->emsr.v & 3u) == 3u) {
                xc->flvl_tx_next = !xc->flvl_tx_next;   /* alternate RX, TX, RX, ... */
            }
            break;
        case XR_XCHAR:
            xc->xchar.v = 0;   /* each bit clears once read */
            break;
        default:
            break;
    }
}

static void queue_flow(NsChannel *c, uint8_t ch) {
    if (c->flow_out_count < sizeof c->flow_out) {
        c->flow_out[c->flow_out_count++] = ch;
    }
}

/* Xon (1) or Xoff (0) for a TX mode EFR[3:2]; 5G XCHAR b3/b2 note it. */
void exs_queue_flow_pair(NsChannel *c, unsigned txmode, int xon) {
    Reg first = xon ? c->xon1 : c->xoff1;
    Reg second = xon ? c->xon2 : c->xoff2;

    switch (txmode) {
        case 2: queue_flow(c, first.v); break;
        case 1: queue_flow(c, second.v); break;
        case 3: queue_flow(c, first.v); queue_flow(c, second.v); break;
        default: return;
    }
    if (exs_style_of(c) == XS_5G && exs_xd(c)->v798) {
        c->xc.xchar.v = (uint8_t)((c->xc.xchar.v & ~0x0Cu) | (xon ? 0x08u : 0x04u));
    }
}

int ns_x_write(NsState *s, NsChannel *c, unsigned address, uint8_t v, uint8_t k, Cause *cause) {
    const NsDevice *d = s->dev;
    const NsExarDevice *x = d->x;
    NsExarChannel *xc = &c->xc;
    unsigned i;

    *cause = CAUSE_WRITE;
    switch (exs_decode(c, address, 1)) {
        default:   /* XR_CORE: the core's register */
            return 0;
        case XR_X:
        case XR_FIFORDY:
        case XR_DREV:
        case XR_DVID:
        case XR_FC:
        case XR_FLVL:
        case XR_TXCNT:
        case XR_RXCNT:
        case XR_XCHAR:
            return 1;
        case XR_EFR: {
            unsigned old_tx = (c->efr.v >> 2) & 3u;

            c->efr = (Reg){v, k};
            if (c->xoff_sent && ((v >> 2) & 3u) != old_tx) {
                exs_queue_flow_pair(c, old_tx, 1);   /* the owed Xon still goes out */
                c->xoff_sent = 0;
            }
            if ((v & 0x0Fu) == 0) {
                xc->xchar.v = 0;          /* 5G: cleared when flow control is off */
                xc->xoff_delay = 0;
                c->tx_halted = 0;
            }
            ns_x_levels(s, c);
            return 1;
        }
        case XR_XON1: c->xon1 = (Reg){v, k}; return 1;
        case XR_XON2: c->xon2 = (Reg){v, k}; return 1;
        case XR_XOFF1: c->xoff1 = (Reg){v, k}; return 1;
        case XR_XOFF2: c->xoff2 = (Reg){v, k}; return 1;
        case XR_DLD:
            xc->dld = (Reg){v, k};
            if (x->style == XS_5G) {
                xc->dld.v &= 0x0F;
                xc->dld.k |= 0xF0;
            }
            ns_x_load_divisor(c);
            exs_set_format_rate(c);
            return 1;
        case XR_AFR:
            c->afr = (Reg){(uint8_t)(v & 0x07), (uint8_t)(k | 0xF8)};
            if (!x->afr0_per_channel && (k & 1u)) {
                /* One concurrent-write bit, set or cleared from either channel. */
                s->conc = v & 1u;
                for (i = 0; i < d->channel_count; i++) {
                    s->ch[i].afr.v = (uint8_t)((s->ch[i].afr.v & ~1u) | (v & 1u));
                }
            }
            *cause = CAUSE_MCR_WRITE;   /* the MF pins follow */
            return 1;
        case XR_TCR: c->tcr = (Reg){v, k}; ns_x_levels(s, c); return 1;
        case XR_TLR: c->tlr = (Reg){v, k}; ns_x_levels(s, c); return 1;
        case XR_FCTR:
            xc->fctr = (Reg){v, k};
            ns_x_levels(s, c);
            return 1;
        case XR_TRG:
            /* TRG: table D's RX (FCTR[7] = 0) or TX level, 1-128 (decisions.md). */
            if (bit_of(xc->fctr, 7) == LOGIC_1) {
                xc->trg_tx = v;
                xc->trg_tx_bad = k != 0xFF || v == 0 || v > 128;
            } else if (bit_of(xc->fctr, 7) == LOGIC_0) {
                xc->trg_rx = v;
                xc->trg_rx_bad = k != 0xFF || v == 0 || v > 128;
            } else {
                xc->trg_rx_bad = 1;
                xc->trg_tx_bad = 1;
            }
            ns_x_levels(s, c);
            return 1;
        case XR_EMSR:
            xc->emsr = (Reg){v, k};
            xc->flvl_tx_next = 0;   /* the first FLVL read after an EMSR write is RX */
            *cause = CAUSE_MCR_WRITE;
            return 1;
        case XR_TXTRG:
            xc->trg_tx = v > 64 ? 64 : v;
            xc->trg_tx_bad = k != 0xFF;
            return 1;
        case XR_RXTRG:
            xc->trg_rx = v > 64 ? 64 : v;
            xc->trg_rx_bad = k != 0xFF;
            ns_x_levels(s, c);
            return 1;
        case XR_MSRW: {
            /* 5G MSR write: [7:4] RS-485 delay, [3] TX disable, [2] RX disable
               (XR16V798; the XR16L784 has [7:4] only); EFR[4] enables it. */
            uint8_t mask = x->v798 ? 0xFC : 0xF0;
            int rx_was = exs_kb(xc->msrw, 2);

            if (!enhanced(c)) {
                return 1;
            }
            xc->msrw = (Reg){(uint8_t)((xc->msrw.v & ~mask) | (v & mask)),
                             (uint8_t)((xc->msrw.k & ~mask) | (k & mask))};
            if (!rx_was && exs_kb(xc->msrw, 2)) {
                /* RX disable: the character in progress finishes; idle, one more comes in. */
                xc->rx_allow = serial_rx_busy(&c->line.rx) || c->pending_count > 0 ? 0 : 1;
            }
            return 1;
        }
        case XR_THR_IMM:
            /* Send-immediate: this byte goes out ahead of the FIFO. */
            xc->imm_armed = 0;
            xc->imm_valid = 1;
            xc->imm_data = v;
            xc->imm_known = k;
            c->mcr.v &= (uint8_t)~0x08u;   /* MCR[3] self-clears */
            ns_x_wake(c);
            *cause = CAUSE_THR_WRITE;
            return 1;
    }
}

/* ---- Write masks, FCR, MCR ---------------------------------------------------------- */

uint8_t ns_x_ier_mask(const NsChannel *c) {
    switch (exs_style_of(c)) {
        case XS_IER5: return 0x2F;   /* IER[5] special mode; [7:6], [4] reserved */
        case XS_650:
        case XS_854:
        case XS_M752: return enhanced(c) ? 0xFF : 0x0F;
        case XS_5G: return enhanced(c) ? 0xEF : 0x0F;   /* IER[4] reserved (SLEEP register) */
        default: return 0x0F;
    }
}

uint8_t ns_x_mcr_mask(const NsChannel *c) {
    const NsExarDevice *x = exs_xd(c);

    switch (x->style) {
        case XS_IER5:
            /* MCR[7] (power-down) only with IER[5] = 1. */
            return exs_kb(c->ier, 5) ? 0x9F : 0x1F;
        case XS_650:
        case XS_854:
        case XS_M752:
            if (enhanced(c)) {
                return 0xFF;
            }
            return x->mcr2_enhanced ? 0x1B : 0x1F;
        case XS_5G:
            return enhanced(c) ? 0xFF : 0x13;   /* MCR[7:5], [3:2] are enhanced bits */
        default:
            return c->owner->dev->prescaler != PRESCALE_NONE ? 0x9F : 0x1F;
    }
}

void ns_x_mcr_written(NsChannel *c, uint8_t mask) {
    const NsExarDevice *x = exs_xd(c);

    if (x->style == XS_IER5) {
        if (mask & 0x80u) {
            c->xc.powered_down = exs_kb(c->mcr, 7);
            if (!c->xc.powered_down) {
                c->asleep = 0;
            }
        }
        return;
    }
    /* MCR[7] written: the prescaler follows it (an override of CLKSEL). */
    if ((mask & 0x80u) && c->owner->dev->prescaler != PRESCALE_NONE) {
        c->xc.prescale4 = exs_kb(c->mcr, 7);
    }
    /* 5G send-immediate: MCR[3] = 1 arms it (not in loopback, nor with TX disabled). */
    if (x->style == XS_5G && x->v798 && (mask & 0x08u) && exs_kb(c->mcr, 3) && !exs_loopback(c) &&
        !exs_kb(c->xc.msrw, 3)) {
        c->xc.imm_armed = 1;
    }
}

void ns_x_fcr(NsChannel *c, uint8_t v, uint8_t k) {
    const NsDevice *d = c->owner->dev;

    /* FCR[5:4]: the TX trigger, written with the enhanced gate (IER[5] or EFR[4]);
       the 854 and 5G tables take it as written. */
    if ((k & 0x30u) != 0x30u) {
        return;
    }
    switch (exs_style_of(c)) {
        case XS_IER5:
        case XS_650:
        case XS_M752:
            if (d->has_tx_trigger && enhanced(c)) {
                c->tx_trigger = (v >> 4) & 3u;
            }
            break;
        case XS_854:
        case XS_5G:
            if (enhanced(c)) {
                c->tx_trigger = (v >> 4) & 3u;
            }
            break;
        default:
            break;
    }
}

/* ---- Trigger levels ---------------------------------------------------------------- */

/* The 854 (FCTR[5:4]) / 5G (FCTR[7:6]) table: 0-2 = A-C, 3 = D; -1 = X. */
int exs_table_of(const NsChannel *c) {
    unsigned shift = exs_style_of(c) == XS_5G ? 6 : 4;
    uint8_t bits = (uint8_t)(3u << shift);

    if ((c->xc.fctr.k & bits) != bits) {
        return -1;
    }
    return (c->xc.fctr.v >> shift) & 3;
}

/* 1 when table D's RX level is X (TRG out of range). */
int exs_rx_trigger_x(const NsChannel *c) {
    if (exs_style_of(c) != XS_854 && exs_style_of(c) != XS_5G) {
        return 0;
    }
    return exs_table_of(c) < 0 || (exs_table_of(c) == 3 && c->xc.trg_rx_bad);
}

unsigned ns_x_rx_trigger(const NsChannel *c, unsigned fcr_level) {
    const NsDevice *d = c->owner->dev;
    int t;

    switch (exs_style_of(c)) {
        case XS_854:
        case XS_5G:
            t = exs_table_of(c);
            if (t < 0) {
                return fcr_level;   /* X: the sources hook makes the level X */
            }
            if (t < 3) {
                return exs_xd(c)->rx_table[t][c->trigger & 3u];
            }
            return c->xc.trg_rx != 0 ? c->xc.trg_rx : 1;
        case XS_M752: {
            unsigned tlr = (c->tlr.v >> 4) & 0x0Fu;

            return d->tlr != 0 && tlr != 0 ? tlr * d->tlr : fcr_level;
        }
        default:
            return fcr_level;
    }
}

/* The Exar TX trigger in bytes (0 = none: the interrupt is "empty"). */
unsigned exs_tx_level_bytes(const NsChannel *c) {
    const NsDevice *d = c->owner->dev;
    int t;

    switch (exs_style_of(c)) {
        case XS_IER5:
            return d->has_tx_trigger && exs_kb(c->ier, 5) ? d->tx_trigger[c->tx_trigger & 3u] : 0;
        case XS_650:
            return d->has_tx_trigger ? d->tx_trigger[c->tx_trigger & 3u] : 0;
        case XS_854:
        case XS_5G:
            t = exs_table_of(c);
            if (t < 0) {
                return 1;
            }
            if (t < 3) {
                return exs_xd(c)->tx_table[t][c->tx_trigger & 3u];
            }
            return c->xc.trg_tx != 0 && !c->xc.trg_tx_bad ? c->xc.trg_tx : 1;
        default:
            return 0;
    }
}

unsigned ns_x_tx_threshold(const NsChannel *c) {
    const NsDevice *d = c->owner->dev;
    unsigned level;
    unsigned deep = depth(c);

    if (!c->fifo_on) {
        return 0;
    }
    if (exs_style_of(c) == XS_M752) {
        /* TI style: free spaces (TLR × 4, else FCR[5:4]). */
        unsigned tlr = c->tlr.v & 0x0Fu;

        if (d->tlr != 0 && tlr != 0) {
            return tlr * d->tlr;
        }
        return d->has_tx_trigger ? d->tx_trigger[c->tx_trigger & 3u] : 0;
    }
    level = exs_tx_level_bytes(c);
    if (level == 0) {
        return 0;
    }
    if (level > deep) {
        level = deep;
    }
    return deep - level + 1;   /* count < level  <=>  free >= depth - level + 1 */
}

/* The FIFO emptied (ns_set_thre): the THR interrupt too, unless the FIFO
   reached the trigger since the last reload (the 5G interrupts twice). */
int ns_x_thre_at_empty(NsChannel *c) {
    int fire;

    /* The one caller (ns_set_thre) asks only while a TX threshold is set. */
    fire = !c->xc.filled || exs_style_of(c) == XS_5G;
    c->xc.filled = 0;
    return fire;
}

