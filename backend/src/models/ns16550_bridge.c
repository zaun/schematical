#include "ns16550_bridge_int.h"

/* ---- Small helpers --------------------------------------------------------------- */

int brs_is_xr(const NsDevice *d) {
    return d->extended == EXT_XR1280;
}

int brs_enhanced(const NsChannel *c) {
    return nb_known1(c->efr, 4);
}

void nb_drive(NsState *s, size_t pin, LogicValue value, SimTime delay) {
    if (!SERIAL_REAL_PIN(pin) || s->out[pin] == value) {
        return;
    }
    s->out[pin] = value;
    sim_schedule_inertial_drive(s->sim, s->instance, pin, value, DRIVE_STRONG, delay);
}

SimTime nb_cause_delay(const NsState *s, Cause cause) {
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

LogicValue nb_gpio_level(const NsState *s, unsigned bit) {
    size_t pin = s->pins->bridge->gpio[bit];

    if (pin == SERIAL_NO_PIN || bit >= s->dev->bridge->gpio_count) {
        return LOGIC_X;
    }
    return serial_level(s->sim, s->instance, pin, s->dev->bridge->gpio_pullup ? LOGIC_1 : LOGIC_X);
}

/* The levels of all 8 GPIO inputs: value and known masks. */
void brs_gpio_levels(const NsState *s, uint8_t *v, uint8_t *k) {
    unsigned b;

    *v = 0;
    *k = 0;
    for (b = 0; b < NB_GPIO; b++) {
        LogicValue l = nb_gpio_level(s, b);

        if (l != LOGIC_X) {
            *k |= (uint8_t)(1u << b);
            if (l == LOGIC_1) {
                *v |= (uint8_t)(1u << b);
            }
        }
    }
}

/* SC16IS: channel ch's modem pins are on (IOControl bit ch + 1). */
static LogicValue sc_modem_on(const NsState *s, unsigned ch) {
    return bit_of(s->bridge->iocontrol, ch + 1);
}

/* The GPIO bit is a modem pin now: 1, 0 or X. */
LogicValue brs_gpio_is_modem(const NsState *s, unsigned b) {
    const NsBridgePins *bp = s->pins->bridge;

    if (bp->gpio_ch[b] < 0) {
        return LOGIC_0;
    }
    if (brs_is_xr(s->dev)) {
        return bool3(xr_modem_on(s));
    }
    return sc_modem_on(s, (unsigned)bp->gpio_ch[b]);
}

/* ---- Register reset ------------------------------------------------------------------ */

void brs_reset_registers(NsState *s, int power_on) {
    NsBridge *br = s->bridge;
    const NsBridgeDevice *b = s->dev->bridge;
    unsigned i;

    br->iodir = (Reg){0, 0xFF};
    br->latch = b->state_reset >= 0 ? (Reg){(uint8_t)b->state_reset, 0xFF} : (Reg){0, 0};
    br->iointena = (Reg){0, 0xFF};
    br->iocontrol = (Reg){0, 0xFF};
    br->latched = 0;
    br->latch_pending = 0;
    if (s->sim != NULL) {
        brs_gpio_levels(s, &br->ref, &br->ref_k);
    } else {
        br->ref = 0;
        br->ref_k = 0;
    }
    for (i = 0; i < s->dev->channel_count; i++) {
        NbChannel *n = &br->ch[i];

        n->efcr = (Reg){0, 0xFF};
        n->dld = (Reg){0, 0xFF};
        n->sfren = (Reg){0, 0xFF};
        n->sfr = (Reg){0, 0xFF};
        n->cpr = (Reg){0x10, 0xFF};
        n->tidle = (Reg){0, 0xFF};
        n->trctl = (Reg){0x06, 0xFF};
        n->fc_toggle = 0;
        n->aa_on = 0;
    }
    if (brs_is_xr(s->dev)) {
        xr_reset(s, power_on);
    }
    for (i = 0; i < s->dev->channel_count; i++) {
        ns_ext_load_divisor(&s->ch[i]);
        ns_update_format(&s->ch[i]);
    }
}

/* IOControl[3] (and XR20M1280 has none): the whole device, as the reset pin. */
static void software_reset(NsState *s) {
    unsigned i;

    ns_reset_all(s);
    brs_reset_registers(s, 0);
    for (i = 0; i < s->dev->channel_count; i++) {
        ns_modem_refresh(s, &s->ch[i]);
    }
}

/* ---- Register decode (SC16IS family) --------------------------------------------- */

/* 0 general set (LCR[7] = 0), 1 special set (LCR[7] = 1, not 0xBF), 2 the
   enhanced set (LCR = 0xBF), -1 unknown. */
static int lcr_set(const NsChannel *c) {
    LogicValue l7 = bit_of(c->lcr, 7);

    if (l7 == LOGIC_0) {
        return 0;
    }
    if (l7 == LOGIC_X) {
        return -1;
    }
    if (((c->lcr.v ^ 0xBFu) & c->lcr.k) != 0) {
        return 1;   /* cannot be 0xBF */
    }
    return c->lcr.k == 0xFF ? 2 : -1;
}

static int sc_decode(const NsChannel *c, unsigned a, int write) {
    const NsDevice *d = c->owner->dev;
    const NbChannel *n = nb_ch(c);
    int x = write ? R_IGNORE : R_X;
    int set = lcr_set(c);

    if (a == 3) {
        return R_CORE;
    }
    if (set < 0) {
        return x;
    }
    if (set == 1) {
        if (a == 0 || a == 1) {
            return R_CORE;
        }
        if (a == 2 && d->extended == EXT_SC16IS_DLD && brs_enhanced(c)) {
            return B_DLD;
        }
        return x;
    }
    if (set == 2) {
        if (d->extended == EXT_SC16IS_SFR) {
            LogicValue special = bit_of(n->sfr, 2);
            static const int SPECIAL[16] = {
                -1, -1, B_ASR, -1, B_CPR, B_RFD, B_TFD, -1, B_TIDLE, B_TRCTL, -1, -1, -1, -1, -1,
                B_ISCR
            };

            if (a == 13) {
                return B_SFREN;
            }
            if (SPECIAL[a] >= 0 && special != LOGIC_0) {
                if (special == LOGIC_X) {
                    return x;
                }
                if (write && (SPECIAL[a] == B_ASR || SPECIAL[a] == B_RFD || SPECIAL[a] == B_TFD)) {
                    return R_IGNORE;   /* read-only */
                }
                return SPECIAL[a];
            }
            if (a == 7) {
                if (n->sfren.k != 0xFF) {
                    return x;
                }
                if (n->sfren.v == 0x5A) {
                    return B_SFR;
                }
            }
        }
        switch (a) {
            case 2: return R_EFR;
            case 4: return R_XON1;
            case 5: return R_XON2;
            case 6: return R_XOFF1;
            case 7: return R_XOFF2;
            default: return x;
        }
    }
    /* The general set. */
    switch (a) {
        case 5:
            return write ? R_IGNORE : R_CORE;   /* LSR is read-only */
        case 6:
        case 7: {
            LogicValue tcr = and3(bool3(brs_enhanced(c)), bit_of(c->mcr, 2));

            if (tcr == LOGIC_X) {
                return x;
            }
            if (tcr == LOGIC_1) {
                return a == 6 ? R_TCR : R_TLR;
            }
            return R_CORE;
        }
        case 8: return write ? R_IGNORE : B_TXLVL;
        case 9: return write ? R_IGNORE : B_RXLVL;
        case 10:
        case 11:
        case 12:
            if (d->bridge->gpio_count == 0) {
                return x;   /* SC16IS740: no GPIO (planner decision) */
            }
            return a == 10 ? B_IODIR : a == 11 ? B_IOSTATE : B_IOINTENA;
        case 13: return x;
        case 14:
            /* The SC16IS740 reads X here; a write still has its software reset. */
            return d->bridge->gpio_count == 0 && !write ? R_X : B_IOCONTROL;
        case 15: return B_EFCR;
        default: return R_CORE;
    }
}

/* Called by decode_any (ns16550_ext.c) for a device with a bridge only. */
int ns_bridge_decode(const NsChannel *c, unsigned address, int write) {
    if (brs_is_xr(c->owner->dev)) {
        return xr_decode(c, address, write);
    }
    return sc_decode(c, address & 15u, write);
}

/* ---- GPIO (SC16IS) ----------------------------------------------------------------------- */

/* The GPIO interrupt (IOLATCH 0: an enabled input differs from its level at
   the last IOState read; IOLATCH 1: a change was latched). */
LogicValue brs_sc_gpio_pending(const NsState *s) {
    const NsBridge *br = s->bridge;
    LogicValue latch_mode = bit_of(br->iocontrol, 0);
    LogicValue any = LOGIC_0;
    unsigned b;

    if (s->dev->bridge->gpio_style != NB_GPIO_SC16IS) {
        return LOGIC_0;
    }
    if (latch_mode == LOGIC_1) {
        return bool3(br->latch_pending);
    }
    for (b = 0; b < s->dev->bridge->gpio_count; b++) {
        LogicValue cond;
        LogicValue level;
        LogicValue diff;

        if (s->pins->bridge->gpio[b] == SERIAL_NO_PIN) {
            continue;
        }
        cond = and3(and3(not3(bit_of(br->iodir, b)), bit_of(br->iointena, b)),
                    not3(brs_gpio_is_modem(s, b)));
        if (cond == LOGIC_0) {
            continue;
        }
        level = nb_gpio_level(s, b);
        if (level == LOGIC_X || !((br->ref_k >> b) & 1u)) {
            diff = LOGIC_X;
        } else {
            diff = bool3((level == LOGIC_1) != (((br->ref >> b) & 1u) != 0));
        }
        any = or3(any, and3(cond, diff));
    }
    if (latch_mode == LOGIC_X) {
        return or3(any, bool3(br->latch_pending)) == LOGIC_0 ? LOGIC_0 : LOGIC_X;
    }
    return any;
}

/* A GPIO pin changed: IOLATCH = 1 latches an enabled input's new level. */
void brs_sc_gpio_changed(NsState *s, unsigned b) {
    NsBridge *br = s->bridge;
    LogicValue level = nb_gpio_level(s, b);

    if (bit_of(br->iocontrol, 0) != LOGIC_1 || ((br->latched >> b) & 1u)) {
        return;
    }
    if (bit_of(br->iodir, b) != LOGIC_0 || bit_of(br->iointena, b) != LOGIC_1 ||
        brs_gpio_is_modem(s, b) != LOGIC_0) {
        return;
    }
    br->latched |= (uint8_t)(1u << b);
    br->latched_v = (uint8_t)((br->latched_v & ~(1u << b)) | (level == LOGIC_1 ? 1u << b : 0));
    br->latched_k = (uint8_t)((br->latched_k & ~(1u << b)) | (level != LOGIC_X ? 1u << b : 0));
    br->latch_pending = 1;
}

/* An IOState read, an IODir write: the GPIO interrupt clears. */
static void sc_gpio_clear(NsState *s) {
    NsBridge *br = s->bridge;

    brs_gpio_levels(s, &br->ref, &br->ref_k);
    br->latched = 0;
    br->latch_pending = 0;
}

static void sc_iostate(const NsState *s, uint8_t *v, uint8_t *k) {
    const NsBridge *br = s->bridge;
    unsigned b;

    *v = 0;
    *k = 0;
    for (b = 0; b < NB_GPIO; b++) {
        uint8_t m = (uint8_t)(1u << b);
        LogicValue l;

        if ((br->latched & m) != 0) {
            l = (br->latched_k & m) ? ((br->latched_v & m) ? LOGIC_1 : LOGIC_0) : LOGIC_X;
        } else if (s->pins->bridge->gpio[b] == SERIAL_NO_PIN) {
            /* No pin in this package: an output reads its latch, an input X. */
            l = bit_of(br->iodir, b) == LOGIC_1 ? bit_of(br->latch, b) : LOGIC_X;
        } else {
            l = nb_gpio_level(s, b);
        }
        if (l != LOGIC_X) {
            *k |= m;
            if (l == LOGIC_1) {
                *v |= m;
            }
        }
    }
}

/* ---- Peek / write / read side effects ------------------------------------------------- */

static uint8_t count_errors(const NsChannel *c) {
    unsigned i;
    unsigned n = 0;

    for (i = 0; i < c->rx_count; i++) {
        const SerialRxChar *rc = &c->rx[(c->rx_head + i) % NS_MAX_FIFO];

        if ((rc->flags & 7u) != 0 || (rc->flags_known & 7u) != 7u) {
            n++;
        }
    }
    return (uint8_t)n;
}

/* PI7C9X ASR: [5:4] Xon/Xoff state, [1] we asked the remote to stop, [0]
   the remote stopped us. */
static uint8_t pi_asr(const NsChannel *c) {
    uint8_t v = 0;

    if ((c->efr.v & 0x0Fu) != 0) {
        if (c->tx_halted) {
            v |= 0x10;
        } else if (c->xoff_sent) {
            v |= 0x20;
        } else if (c->xon_seen) {
            v |= 0x30;
        }
    }
    if (c->xoff_sent || c->rts_off) {
        v |= 0x02;
    }
    if (c->tx_halted) {
        v |= 0x01;
    }
    return v;
}

static uint8_t pi_iscr(const NsChannel *c) {
    LogicValue src[8];
    static const unsigned BIT[7] = {1, 2, 3, 4, 5, 6, 7};   /* ls rda timeout thr modem xoff ctsrts */
    uint8_t v = 0;
    unsigned i;

    ns_sources(c, src);
    for (i = 0; i < 7; i++) {
        if (src[i] == LOGIC_1) {
            v |= (uint8_t)(1u << BIT[i]);
        }
    }
    return v;
}

void ns_bridge_peek_reg(NsChannel *c, int reg, uint8_t *v, uint8_t *k) {
    NsState *s = c->owner;
    NsBridge *br = s->bridge;
    NbChannel *n = nb_ch(c);

    *v = 0;
    *k = 0xFF;
    switch (reg) {
        case B_TXLVL: *v = (uint8_t)(s->dev->fifo - c->tx_count); break;
        case B_RXLVL: *v = (uint8_t)c->rx_count; break;
        case B_IODIR: *v = br->iodir.v; *k = br->iodir.k; break;
        case B_IOSTATE: sc_iostate(s, v, k); break;
        case B_IOINTENA: *v = br->iointena.v; *k = br->iointena.k; break;
        case B_IOCONTROL:
            *v = br->iocontrol.v & 0xF7u;   /* SRESET reads 0 (self-clearing) */
            *k = br->iocontrol.k | 0x08u;
            break;
        case B_EFCR: *v = n->efcr.v; *k = n->efcr.k; break;
        case B_DLD: *v = n->dld.v; *k = n->dld.k; break;
        case B_SFREN: *v = n->sfren.v; *k = n->sfren.k; break;
        case B_SFR: *v = n->sfr.v; *k = n->sfr.k; break;
        case B_ASR: *v = pi_asr(c); break;
        case B_CPR: *v = n->cpr.v; *k = n->cpr.k; break;
        case B_RFD:
            if (bit_of(n->sfr, 6) == LOGIC_X) {
                *k = 0;
            } else {
                *v = nb_known1(n->sfr, 6) ? count_errors(c) : (uint8_t)c->rx_count;
            }
            break;
        case B_TFD: *v = (uint8_t)c->tx_count; break;
        case B_TIDLE: *v = n->tidle.v; *k = n->tidle.k; break;
        case B_TRCTL: *v = n->trctl.v; *k = n->trctl.k; break;
        case B_ISCR: *v = pi_iscr(c); break;
        default:
            /* The XR20M1280's own registers: sc_decode names only the B_ ones above. */
            xr_peek_reg(c, reg, v, k);
            break;
    }
}

/* PI7C9X ISCR[0] CLSTATUS: the latched interrupt events clear. */
static void pi_clear_status(NsChannel *c) {
    c->thre_int = 0;
    c->thr_level_int = 0;
    c->timeout = 0;
    c->idle_ticks = 0;
    c->iir_xoff = 0;
    c->iir_special = 0;
    c->iir_ctsrts = 0;
    c->delta = 0;
    c->delta_k = 0xFF;
    c->lsr_err &= (uint8_t)~0x02;
    c->lsr_err_k |= 0x02;
}

void ns_bridge_write_reg(NsState *s, NsChannel *c, int reg, uint8_t v, uint8_t k) {
    NsBridge *br = s->bridge;
    NbChannel *n = nb_ch(c);
    unsigned i;

    switch (reg) {
        case B_IODIR:
            br->iodir = (Reg){v, k};
            sc_gpio_clear(s);   /* "writing IODir clears a pending GPIO interrupt" */
            break;
        case B_IOSTATE: br->latch = (Reg){v, k}; break;
        case B_IOINTENA: br->iointena = (Reg){v, k}; break;
        case B_IOCONTROL:
            if (!(k & 0x08u)) {
                s->unknown = 1;   /* SRESET written X: the reset is X (P2) */
                return;
            }
            if (v & 0x08u) {
                software_reset(s);
                return;
            }
            br->iocontrol = (Reg){(uint8_t)(v & 0x07u), (uint8_t)(k | 0xF8u)};
            if (s->dev->channel_count < 2) {
                br->iocontrol.v &= 0x03u;   /* IOControl[2] reserved on one channel */
            }
            for (i = 0; i < s->dev->channel_count; i++) {
                ns_modem_refresh(s, &s->ch[i]);
            }
            if (!nb_known1(br->iocontrol, 0)) {
                br->latched = 0;
                br->latch_pending = 0;
            }
            break;
        case B_EFCR:
            if (!nb_known1(n->efcr, 0) && (v & k & 1u)) {
                n->aa_on = 0;   /* 9-bit mode on: auto-address receiver starts off */
            }
            n->efcr = (Reg){v, k};
            break;
        case B_DLD:
            n->dld = (Reg){v, k};
            ns_ext_load_divisor(c);
            ns_update_format(c);
            break;
        case B_SFREN: n->sfren = (Reg){v, k}; break;
        case B_SFR: n->sfr = (Reg){v, k}; break;
        case B_CPR:
            n->cpr = (Reg){v, k};
            c->prescale_count = 0;
            break;
        case B_TIDLE: n->tidle = (Reg){v, k}; break;   /* stored only (planner decision) */
        case B_TRCTL:
            n->trctl = (Reg){v, k};
            ns_update_format(c);
            break;
        case B_ISCR:
            if (k & v & 1u) {
                pi_clear_status(c);
            }
            break;
        default:
            if (brs_is_xr(s->dev)) {
                xr_write_reg(s, c, reg, v, k);
            }
            break;
    }
}

/* Called by the core (ns16550_io.c) for a device with a bridge only. */
int ns_bridge_read_done(NsChannel *c, unsigned address, Cause *cause) {
    NsState *s = c->owner;
    int reg;

    reg = ns_bridge_decode(c, address, 0);
    if (reg == R_CORE && brs_is_xr(s->dev)) {
        NbChannel *n = nb_ch(c);

        if (address == 2) {
            /* An ISR read: the wake-up interrupt clears; with EMSR[7] = 1 the
               Xoff interrupt clears here (not on Xon). */
            n->wake_int = 0;
            if (c->iir_seen_k == 0xFF && (c->iir_seen & 0x3Fu) == 0x10u) {
                n->xoff_sticky = 0;
                c->iir_xoff = 0;
            }
        } else if (address == 5) {
            n->ls_now = 0;   /* EMSR[6]: an LSR read clears the immediate LSR interrupt */
        }
        return 0;
    }
    if (reg < NS_REG_BRIDGE) {
        return 0;
    }
    *cause = CAUSE_WRITE;
    if (reg == B_IOSTATE) {
        sc_gpio_clear(s);
        *cause = CAUSE_MSR_READ;
        return 1;
    }
    if (brs_is_xr(s->dev)) {
        return xr_read_done(c, reg, cause);
    }
    return 1;
}

/* ---- The host port ------------------------------------------------------------------------ */

static void sub_decode(NsBridge *br, const NsBridgeDevice *b, uint8_t data, uint8_t known) {
    uint8_t reg_mask = b->reg_bits == 4 ? 0x78 : 0x38;

    br->sub_byte = data;
    br->sub_known = known;
    br->sub_reg = (data & reg_mask) >> 3;
    br->sub_ch = (data >> 1) & 3u;
    br->sub_x = ((known & (reg_mask | 0x06)) != (reg_mask | 0x06));
    br->have_sub = 1;
}

/* X data into every register an unknown sub-address could name (P2). */
static void write_unknown(NsState *s) {
    NsBridge *br = s->bridge;
    uint8_t reg_mask = s->dev->bridge->reg_bits == 4 ? 0x78 : 0x38;
    unsigned regs = 1u << s->dev->bridge->reg_bits;
    unsigned ch;
    unsigned r;

    for (ch = 0; ch < s->dev->channel_count; ch++) {
        for (r = 0; r < regs; r++) {
            uint8_t byte = (uint8_t)((r << 3) | (ch << 1));
            uint8_t mask = (uint8_t)(br->sub_known & (reg_mask | 0x06));

            if (((byte ^ br->sub_byte) & mask) != 0) {
                continue;
            }
            if (!s->unknown) {
                ns_host_write(s, ch, r, 0, 0);
            }
        }
    }
}

static void advance(NsState *s) {
    NsBridge *br = s->bridge;
    const NsChannel *c = &s->ch[br->sub_ch];   /* host_write and host_read_done check the channel */

    /* PI7C9X SFR[4]: bursts auto-increment the sub-address (decisions.md). */
    if (s->dev->extended == EXT_SC16IS_SFR && nb_known1(nb_ch(c)->sfr, 4)) {
        br->sub_reg = (br->sub_reg + 1u) & ((1u << s->dev->bridge->reg_bits) - 1u);
    }
}

/* Does a write to this register reset the device (IOControl[3])? */
static int writes_reset(NsState *s, uint8_t data, uint8_t known) {
    NsBridge *br = s->bridge;

    return !brs_is_xr(s->dev) &&
           sc_decode(&s->ch[br->sub_ch], br->sub_reg, 1) == B_IOCONTROL && (data & known & 0x08u);
}

/* A THR write with the TX FIFO full. */
static int thr_full(NsState *s) {
    NsBridge *br = s->bridge;
    NsChannel *c = &s->ch[br->sub_ch];

    return br->sub_reg == 0 && bit_of(c->lcr, 7) == LOGIC_0 && c->tx_count >= depth(c);
}

/* A data byte for the addressed register: the I2C ACK (1), NACK (0) or X. */
static LogicValue host_write(NsState *s, uint8_t data, uint8_t known) {
    NsBridge *br = s->bridge;
    const NsBridgeDevice *b = s->dev->bridge;
    LogicValue ack = LOGIC_1;

    if (s->unknown) {
        return LOGIC_X;
    }
    if (br->sub_x) {
        write_unknown(s);
        return LOGIC_X;
    }
    if (br->sub_ch >= s->dev->channel_count) {
        return LOGIC_1;   /* a reserved channel: ignored (P2) */
    }
    if (b->nack_full && thr_full(s)) {
        return LOGIC_0;   /* the byte is refused, not written */
    }
    if (b->nack_reset && writes_reset(s, data, known)) {
        ack = LOGIC_0;
    }
    ns_host_write(s, br->sub_ch, br->sub_reg, data, known);
    advance(s);
    return ack;
}

static void host_peek(NsState *s, uint8_t *data, uint8_t *known) {
    NsBridge *br = s->bridge;

    *data = 0;
    *known = 0;
    if (s->unknown || !br->have_sub || br->sub_x || br->sub_ch >= s->dev->channel_count) {
        return;
    }
    ns_host_peek(s, br->sub_ch, br->sub_reg, data, known);
}

static void host_read_done(NsState *s) {
    NsBridge *br = s->bridge;

    if (s->unknown || !br->have_sub || br->sub_x || br->sub_ch >= s->dev->channel_count) {
        return;
    }
    ns_host_read_done(s, br->sub_ch, br->sub_reg);
    advance(s);
}

/* I2C */

void brs_i2c_begin(void *context, int repeated) {
    NsState *s = context;

    if (!repeated) {
        s->bridge->have_sub = 0;
    }
}

LogicValue brs_i2c_address(void *context, uint8_t byte, uint8_t known) {
    NsState *s = context;
    int mine = serial_host_i2c_address(&s->bridge->host);
    unsigned addr = byte >> 1;

    if (mine == -1) {
        return LOGIC_0;
    }
    if (mine < 0) {
        return LOGIC_X;   /* a strap that matches no tie */
    }
    if ((known & 0xFE) != 0xFE) {
        return ((((unsigned)mine ^ addr) & (known >> 1)) == 0) ? LOGIC_X : LOGIC_0;
    }
    return bool3((unsigned)mine == addr);
}

LogicValue brs_i2c_received(void *context, unsigned index, uint8_t data, uint8_t known) {
    NsState *s = context;
    NsBridge *br = s->bridge;

    (void)index;
    if (!br->have_sub) {
        sub_decode(br, s->dev->bridge, data, known);
        return LOGIC_1;
    }
    return host_write(s, data, known);
}

void brs_i2c_next(void *context, unsigned index, uint8_t *data, uint8_t *known) {
    (void)index;
    host_peek(context, data, known);
}

void brs_i2c_acked(void *context, unsigned index, LogicValue ack) {
    (void)index;
    (void)ack;
    host_read_done(context);
}

/* SPI (mode 0): command byte, then data on SI (write) or SO (read). */

void brs_spi_begin(void *context) {
    NsState *s = context;

    s->bridge->have_sub = 0;
    s->bridge->spi_rw = -1;
}

SerialSpiByte brs_spi_next(void *context, unsigned index) {
    NsState *s = context;
    NsBridge *br = s->bridge;
    SerialSpiByte out = {0, 0, 0};

    if (index == 0 || br->spi_rw == 0) {
        return out;   /* SO is Z during the command byte and writes */
    }
    out.drive = 0xFF;
    if (br->spi_rw < 0) {
        return out;   /* X */
    }
    host_peek(s, &out.data, &out.known);
    return out;
}

void brs_spi_received(void *context, unsigned index, uint8_t data, uint8_t known) {
    NsState *s = context;
    NsBridge *br = s->bridge;

    if (index == 0) {
        sub_decode(br, s->dev->bridge, data, known);
        br->spi_rw = (known & 0x80u) ? (int)((data >> 7) & 1u) : -1;
        return;
    }
    if (br->spi_rw == 1) {
        host_read_done(s);
    } else if (br->spi_rw == 0) {
        host_write(s, data, known);
    } else if (!s->unknown) {
        br->sub_x = 1;   /* a write could have happened: X into what it could name */
        write_unknown(s);
    }
}

