/*
 * ns16550 bridge: the XR20M1280 (plan-12-I section 3.2; bridges.md §
 * XR20M1280). Its 8-address map (sub-address bits [5:3]):
 *
 *   LCR[7] = 0     0 RHR/THR, 1 IER, 2 ISR/FCR, 4 MCR, 5 LSR / SHR (W, EFR[4]),
 *                  6 MSR / SFR (W, EFR[4]), 7 SPR, GPIOLVL (SFR[0]) or
 *                  EMSR (W) / FC (R) (FCTR[6])
 *   LCR[7] = 1     0 DLL (DREV reads X while DLL = DLM = 0), 1 DLM,
 *                  2 DLD (EFR[4] = 1) else ISR/FCR; 4-7 as above
 *   LCR = 0xBF     0 FC (R) / TRG (W), 1 FCTR, 2 EFR, 4-7 Xon1, Xon2, Xoff1,
 *                  Xoff2, or GPIOINT, GPIO3T, GPIOINV, GPIOSEL (SFR[0])
 *
 * 128-byte FIFOs with the FCTR trigger tables A-D (TRG for D), auto RTS
 * hysteresis, an Xoff sent 2 character times after the RX trigger, the
 * independent TX and RX baud generators of DLD[7:6], RS-485 direction
 * control with SHR setup and turn-around delays, the 9-bit TX address bit,
 * EMSR (Xoff interrupt mode, immediate LSR interrupt, TX/RTS/DTR 3-state,
 * send immediately, FIFO count select), the GPIO set with its interrupt in
 * ISR[6], the SLEEP/PWRDN# pin, the FCR[3] wake-up interrupt and the EN485#
 * and ENIR# power-up straps.
 */

#include <string.h>

#include "ns16550_bridge.h"

static int enhanced(const NsChannel *c) {
    return nb_known1(c->efr, 4);
}

static int lcr_set(const NsChannel *c) {
    LogicValue l7 = bit_of(c->lcr, 7);

    if (l7 == LOGIC_0) {
        return 0;
    }
    if (l7 == LOGIC_X) {
        return -1;
    }
    if (((c->lcr.v ^ 0xBFu) & c->lcr.k) != 0) {
        return 1;
    }
    return c->lcr.k == 0xFF ? 2 : -1;
}

/* ---- Decode ------------------------------------------------------------------------ */

int xr_decode(const NsChannel *c, unsigned address, int write) {
    const NbChannel *n = nb_ch(c);
    unsigned a = address & 7u;
    int x = write ? R_IGNORE : R_X;
    int set = lcr_set(c);

    if (c->owner->bridge->raw || a == 3) {
        return R_CORE;
    }
    if (set < 0) {
        return x;
    }
    if (set == 2) {
        LogicValue gpio = bit_of(n->xsfr, 0);
        static const int XON[4] = {R_XON1, R_XON2, R_XOFF1, R_XOFF2};
        static const int GPIO[4] = {X_GPIOINT, X_GPIO3T, X_GPIOINV, X_GPIOSEL};

        switch (a) {
            case 0: return X_FC_TRG;
            case 1: return X_FCTR;
            case 2: return R_EFR;
            default:
                if (gpio == LOGIC_X) {
                    return x;
                }
                return gpio == LOGIC_1 ? GPIO[a - 4] : XON[a - 4];
        }
    }
    if (set == 1) {
        if (a == 0) {
            return X_DLL;
        }
        if (a == 1) {
            return X_DLM;
        }
        if (a == 2 && enhanced(c)) {
            return X_DLD;
        }
    }
    switch (a) {
        case 2: return write ? X_FCR : R_CORE;
        case 4: return write ? X_MCR : R_CORE;
        case 5:
            if (!write) {
                return R_CORE;
            }
            return enhanced(c) ? X_SHR : R_IGNORE;
        case 6:
            if (!write) {
                return R_CORE;
            }
            return enhanced(c) ? X_SFR : R_IGNORE;
        case 7: {
            LogicValue swap = bit_of(n->fctr, 6);
            LogicValue gpio = bit_of(n->xsfr, 0);

            if (swap == LOGIC_X) {
                return x;
            }
            if (swap == LOGIC_1) {
                return X_EMSR_FC;
            }
            if (gpio == LOGIC_X) {
                return x;
            }
            return gpio == LOGIC_1 ? X_GPIOLVL : R_CORE;
        }
        default:
            return R_CORE;
    }
}

/* ---- Baud generators ------------------------------------------------------------------ */

static unsigned factor_of(Reg dld) {
    unsigned mode = (dld.v >> 4) & 3u;

    return mode == 0 ? 16 : mode == 1 ? 8 : 4;
}

void xr_load_divisors(NsChannel *c) {
    NbChannel *n = nb_ch(c);

    c->dll = n->dll_tx;
    c->dlm = n->dlm_tx;
    serial_baud_load(&c->baud, (uint32_t)n->dll_tx.v | ((uint32_t)n->dlm_tx.v << 8),
                     n->dll_tx.k == 0xFF && n->dlm_tx.k == 0xFF);
    c->baud.frac = (n->dld_tx.k & 0x0Fu) == 0x0Fu ? n->dld_tx.v & 0x0Fu : 0;
    c->baud.frac_den = 16;
    serial_baud_load(&n->rx_baud, (uint32_t)n->dll_rx.v | ((uint32_t)n->dlm_rx.v << 8),
                     n->dll_rx.k == 0xFF && n->dlm_rx.k == 0xFF);
    n->rx_baud.frac = (n->dld_rx.k & 0x0Fu) == 0x0Fu ? n->dld_rx.v & 0x0Fu : 0;
    n->rx_baud.frac_den = 16;
    n->tx_factor = factor_of(n->dld_tx);
    n->rx_factor = factor_of(n->dld_rx);
}

/* DLD[6] = 1: DLD[7] picks the generator a DLL/DLM/DLD write goes to (0 TX,
   1 RX); DLD[6] = 0: both. */
static int gen_select(const NbChannel *n) {
    if (!nb_known1(n->dld_tx, 6)) {
        return -1;   /* both */
    }
    return nb_known1(n->dld_tx, 7) ? 1 : 0;
}

int xr_clock(NsState *s, NsChannel *c, int rising) {
    NbChannel *n = nb_ch(c);
    LogicValue level = rising ? LOGIC_1 : LOGIC_0;
    int tx;
    int rx;

    if (c->asleep) {
        return 1;
    }
    if (nb_known1(c->mcr, 7)) {
        /* MCR[7]: XTAL1 / 4 into both generators. */
        if (!rising) {
            return 1;
        }
        if (++c->prescale_count < 4) {
            return 1;
        }
        c->prescale_count = 0;
    }
    tx = serial_baud_edge(&c->baud, rising, level);
    rx = serial_baud_edge(&n->rx_baud, rising, level);
    if (tx) {
        c->format.factor = n->tx_factor;
        ns_tx_tick(s, c);
        /* RS-485 turn-around: RTS stays (1 + SHR[3:0]) bit times after the
           last stop bit. */
        if (n->rts_active && n->rs_lead == 0 && c->tx_count == 0 && c->flow_out_count == 0 &&
            !serial_tx_busy(&c->line.tx)) {
            if (!n->holding) {
                n->holding = 1;
                n->rs_hold = (1u + (n->shr.v & 0x0Fu)) * n->tx_factor;
            }
            if (n->rs_hold > 0 && --n->rs_hold == 0) {
                n->rts_active = 0;
                n->holding = 0;
                ns_update_channel(s, c, CAUSE_SERIAL);
            }
        }
    }
    if (rx) {
        c->format.factor = n->rx_factor;
        ns_rx_tick(s, c);
        /* LSR[4] (break) stays while RX is still low after the break. */
        if (c->line.rx.phase == SERIAL_RX_WAIT_MARK) {
            c->lsr_err |= 0x10;
            c->lsr_err_k |= 0x10;
        }
        /* The SLEEP pin: asleep once the TSR and RSR bytes are done. */
        if (n->forced_sleep && !c->asleep && !serial_tx_busy(&c->line.tx) &&
            !serial_rx_busy(&c->line.rx)) {
            c->asleep = 1;
            xr_sleep_changed(s, c);
        }
    }
    c->format.factor = n->tx_factor;
    return 1;
}

/* ---- Trigger levels ---------------------------------------------------------------------- */

static const unsigned RX_TABLE[3][4] = {{1, 4, 8, 14}, {8, 16, 24, 28}, {8, 16, 56, 60}};
static const unsigned TX_TABLE[3][4] = {{0, 0, 0, 0}, {16, 8, 24, 30}, {8, 16, 32, 56}};

static unsigned table_of(const NsChannel *c) {
    return (nb_ch(c)->fctr.v >> 4) & 3u;
}

unsigned xr_rx_trigger(const NsChannel *c) {
    unsigned t = table_of(c);

    if (!c->fifo_on) {
        return 1;
    }
    if (t == 3) {
        unsigned trg = nb_ch(c)->trg_rx.v;

        return trg == 0 ? 1 : trg;
    }
    return RX_TABLE[t][c->trigger & 3u];
}

unsigned xr_tx_threshold(const NsChannel *c) {
    unsigned t = table_of(c);

    if (!c->fifo_on) {
        return 0;
    }
    if (t == 3) {
        return nb_ch(c)->trg_tx.v;
    }
    return TX_TABLE[t][c->tx_trigger & 3u];   /* table A: the 16C550's "empty" */
}

/* Auto RTS: off one trigger level above, on one level below (tables A-C);
   table D: TRG +- the SHR[3:0] hysteresis (Table 15). */
static void rts_levels(const NsChannel *c, unsigned *off, unsigned *on) {
    static const unsigned HYST[16] = {0, 4, 6, 8, 8, 16, 24, 32, 40, 44, 48, 52, 12, 20, 28, 36};
    const NbChannel *n = nb_ch(c);
    unsigned t = table_of(c);
    unsigned i = c->trigger & 3u;

    if (t == 3) {
        unsigned trg = xr_rx_trigger(c);
        unsigned h = HYST[n->shr.v & 0x0Fu];

        *off = trg + h;
        *on = trg > h ? trg - h : 0;
        return;
    }
    *off = RX_TABLE[t][i < 3 ? i + 1 : 3];
    *on = i > 0 ? RX_TABLE[t][i - 1] : 0;
}

static void queue_flow(NsChannel *c, int xon) {
    unsigned mode = (c->efr.v >> 2) & 3u;
    Reg first = xon ? c->xon1 : c->xoff1;
    Reg second = xon ? c->xon2 : c->xoff2;

    if ((mode == 2 || mode == 3) && c->flow_out_count < sizeof c->flow_out) {
        c->flow_out[c->flow_out_count++] = first.v;
    }
    if ((mode == 1 || mode == 3) && c->flow_out_count < sizeof c->flow_out) {
        c->flow_out[c->flow_out_count++] = second.v;
    }
}

int xr_levels(NsState *s, NsChannel *c) {
    NbChannel *n = nb_ch(c);
    unsigned level = c->rx_count;
    unsigned off;
    unsigned on;
    int was_off = c->rts_off;

    (void)s;
    rts_levels(c, &off, &on);
    if (nb_known1(c->efr, 6)) {
        if (level >= off) {
            c->rts_off = 1;
        } else if (level <= on) {
            c->rts_off = 0;
        }
    } else {
        c->rts_off = 0;
    }
    if (!was_off && c->rts_off && nb_known1(c->ier, 6)) {
        c->iir_ctsrts = 1;
    }
    /* Xoff 2 character times after the FIFO reaches the trigger; Xon when
       it falls to one level below (Table 8). */
    if (((c->efr.v >> 2) & 3u) != 0 && c->fifo_on) {
        unsigned trig = xr_rx_trigger(c);
        uint64_t wait = 2ull * serial_format_char_bits(&c->format) * n->rx_factor;

        if (!c->xoff_sent) {
            if (level >= trig) {
                if (!n->xoff_wait) {
                    n->xoff_wait = 1;
                    n->xoff_at = c->line.rx.ticks;
                } else if (c->line.rx.ticks - n->xoff_at >= wait) {
                    queue_flow(c, 0);
                    c->xoff_sent = 1;
                    n->xoff_wait = 0;
                }
            } else {
                n->xoff_wait = 0;
            }
        } else if (level <= on) {
            queue_flow(c, 1);
            c->xoff_sent = 0;
        }
    }
    /* EMSR[7] = 1: the Xoff interrupt stays until an ISR read. */
    if (nb_known1(n->emsr, 7)) {
        if (c->iir_xoff) {
            n->xoff_sticky = 1;
        }
        if (n->xoff_sticky) {
            c->iir_xoff = 1;
        }
    } else {
        n->xoff_sticky = 0;
    }
    return 1;
}

/* ---- Receive / transmit hooks ---------------------------------------------------------------- */

int xr_rx_char(NsState *s, NsChannel *c, const SerialRxChar *rc) {
    NbChannel *n = nb_ch(c);
    int rx_off = nb_known1(n->xsfr, 5);
    int address;
    int result;

    (void)s;
    /* The special-character interrupt clears with the next character. */
    c->iir_special = 0;
    if (nb_known1(n->emsr, 6) && ((rc->flags & 7u) != 0 || (rc->flags_known & 7u) != 7u)) {
        n->ls_now = 1;
    }
    if (!nb_known1(n->xsfr, 6)) {
        result = rx_off ? 0 : -1;
    } else {
        /* 9-bit mode (SFR[6]): as the XR20M1170's multidrop. */
        address = (rc->flags_known & SERIAL_RX_PE) && (rc->flags & SERIAL_RX_PE);
        if (nb_known1(c->efr, 5)) {
            if (address) {
                uint8_t mask = (uint8_t)((1u << (5 + (c->lcr.v & 3u))) - 1u);

                n->aa_on = (rc->known & mask) == mask && (rc->data & mask) == (c->xoff2.v & mask);
                result = n->aa_on;
            } else {
                result = n->aa_on ? 1 : 0;
            }
        } else if (address) {
            result = 1;
        } else {
            result = rx_off ? 0 : -1;
        }
    }
    if (result == 0) {
        n->ls_now = 0;
    }
    return result;
}

int xr_tx_blocked(NsState *s, NsChannel *c) {
    NbChannel *n = nb_ch(c);

    (void)s;
    if (nb_known1(n->xsfr, 4)) {
        return 1;   /* SFR[4] TX disable */
    }
    if (nb_known1(n->fctr, 3)) {
        if (!n->rts_active) {
            n->rts_active = 1;
            n->holding = 0;
            n->rs_lead = ((n->shr.v >> 4) & 0x0Fu) * n->tx_factor;
            ns_update_channel(c->owner, c, CAUSE_SERIAL);
        }
        n->holding = 0;
        if (n->rs_lead > 0) {
            n->rs_lead--;
            return 1;   /* SHR[7:4]: RTS leads the first start bit */
        }
    }
    return -1;
}

LogicValue xr_rts(const NsChannel *c, LogicValue normal) {
    const NbChannel *n = nb_ch(c);

    if (nb_known1(n->emsr, 4)) {
        return LOGIC_Z;   /* EMSR[4]: TX, RTS#, DTR# 3-state */
    }
    if (nb_known1(n->fctr, 3)) {
        /* Auto RS-485: RTS# low while sending (EMSR[3]: high). */
        return bool3(n->rts_active == nb_known1(n->emsr, 3));
    }
    return normal;
}

void xr_thr_written(NsChannel *c, unsigned at) {
    NbChannel *n = nb_ch(c);

    n->addr_bit[at] = (uint8_t)nb_known1(n->xsfr, 7);
    n->xsfr.v &= 0x7Fu;   /* SFR[7] self-clears */
    if (nb_known1(n->emsr, 2)) {
        /* EMSR[2]: this byte goes out next, ahead of the FIFO. */
        uint8_t data = c->tx_data[at];
        uint8_t known = c->tx_known[at];
        uint8_t addr = n->addr_bit[at];
        unsigned i;

        for (i = c->tx_count - 1; i > 0; i--) {
            unsigned to = (c->tx_head + i) % NS_MAX_FIFO;
            unsigned from = (c->tx_head + i - 1) % NS_MAX_FIFO;

            c->tx_data[to] = c->tx_data[from];
            c->tx_known[to] = c->tx_known[from];
            n->addr_bit[to] = n->addr_bit[from];
        }
        c->tx_data[c->tx_head] = data;
        c->tx_known[c->tx_head] = known;
        n->addr_bit[c->tx_head] = addr;
        n->emsr.v &= (uint8_t)~0x04u;
    }
    if (nb_known1(n->fctr, 3) && !n->rts_active) {
        n->rts_active = 1;   /* RTS# asserts as data is loaded */
        n->holding = 0;
        n->rs_lead = ((n->shr.v >> 4) & 0x0Fu) * n->tx_factor;
    }
}

int xr_tx_fetch(NsChannel *c) {
    NbChannel *n = nb_ch(c);
    int addr = n->addr_bit[c->tx_head];

    n->addr_bit[c->tx_head] = 0;
    /* The 9th bit replaces the forced-0 parity bit (LCR[5:3] = 111). */
    return addr && c->format.parity == SERIAL_PARITY_SPACE;
}

/* ---- Sleep ------------------------------------------------------------------------------- */

int xr_sleep_check(NsState *s, NsChannel *c) {
    (void)s;
    return c->baud.divisor != 0 && nb_ch(c)->rx_baud.divisor != 0;
}

void xr_sleep_changed(NsState *s, NsChannel *c) {
    NbChannel *n = nb_ch(c);

    if (!c->asleep) {
        if (n->forced_sleep) {
            c->asleep = 1;   /* only the SLEEP pin's release wakes it */
            return;
        }
        if (n->wake_en) {
            n->wake_int = 1;   /* FCR[3] wake-up interrupt */
        }
    }
    xr_gpio_outputs(s);
}

/* SLEEP (FCTR[0] = 0): high = sleep (decisions.md). */
void xr_sleep_pin(NsState *s) {
    NsChannel *c = &s->ch[0];
    NbChannel *n = nb_ch(c);
    LogicValue level = serial_level(s->sim, s->instance, s->pins->bridge->sleep_pin, LOGIC_X);
    int want = !nb_known1(n->fctr, 0) && level == LOGIC_1;

    if (want == n->forced_sleep) {
        return;
    }
    n->forced_sleep = want;
    if (!want && c->asleep) {
        ns_ext_wake(c);
        ns_update_channel(s, c, CAUSE_SERIAL);
    }
}

void xr_straps(NsState *s) {
    NsBridge *br = s->bridge;
    const NsBridgePins *bp = s->pins->bridge;

    br->en485_low = serial_level(s->sim, s->instance, bp->en485, LOGIC_X) == LOGIC_0;
    br->enir_low = serial_level(s->sim, s->instance, bp->enir, LOGIC_X) == LOGIC_0;
    if (br->en485_low) {
        br->ch[0].fctr.v |= 0x08u;   /* EN485# low: auto RS-485 from power-up */
    }
}

/* ---- GPIO ----------------------------------------------------------------------------------- */

int xr_modem_on(const NsState *s) {
    return bit_of(s->ch[0].mcr, 2) == LOGIC_0;   /* MCR[2] = 0: RI#, CD#, DSR#, DTR# */
}

LogicValue xr_gpio_int(const NsState *s) {
    return and3(bool3(s->bridge->xr_gpio_pending), bit_of(s->bridge->ch[0].xsfr, 2));
}

LogicValue xr_irq_extra(const NsState *s) {
    return or3(xr_gpio_int(s), bool3(s->bridge->ch[0].wake_int));
}

static int gpio_is_input(const NsState *s, unsigned b) {
    const NsBridgePins *bp = s->pins->bridge;

    if (bp->gpio_ch[b] >= 0 && xr_modem_on(s)) {
        return 0;   /* a modem pin, not a GPIO */
    }
    return nb_known1(s->bridge->gpiosel, b);
}

/* Edges on enabled inputs (GPIOINV 0: low -> high, 1: high -> low). */
void xr_gpio_changed(NsState *s) {
    NsBridge *br = s->bridge;
    unsigned b;
    uint8_t v = 0;
    uint8_t k = 0;

    for (b = 0; b < NB_GPIO; b++) {
        LogicValue l = s->pins->bridge->gpio[b] == SERIAL_NO_PIN ? LOGIC_X : nb_gpio_level(s, b);
        uint8_t m = (uint8_t)(1u << b);

        if (l != LOGIC_X) {
            k |= m;
            if (l == LOGIC_1) {
                v |= m;
            }
        }
        if (s->sim == NULL || !(br->xr_last_k & m) || l == LOGIC_X || !gpio_is_input(s, b) ||
            !nb_known1(br->gpioint, b)) {
            continue;
        }
        if (((br->xr_last & m) != 0) != (l == LOGIC_1)) {
            int rising = l == LOGIC_1;

            if (rising != nb_known1(br->gpioinv, b)) {
                br->xr_gpio_pending = 1;
            }
        }
    }
    br->xr_last = v;
    br->xr_last_k = k;
}

void xr_gpio_outputs(NsState *s) {
    NsBridge *br = s->bridge;
    const NsBridgePins *bp = s->pins->bridge;
    const NsChannel *c = &s->ch[0];
    SimTime delay = t_of(s, T_GPIO);
    unsigned b;

    for (b = 0; b < NB_GPIO; b++) {
        size_t pin = bp->gpio[b];
        LogicValue level;

        if (pin == SERIAL_NO_PIN) {
            continue;
        }
        if (s->unknown) {
            level = LOGIC_X;
        } else if (s->in_reset) {
            level = bp->gpio_role[b] == CH_DTR ? LOGIC_1 : LOGIC_Z;   /* DTR# high after reset */
        } else if (bp->gpio_ch[b] >= 0 && bit_of(c->mcr, 2) != LOGIC_1) {
            level = LOGIC_Z;
            if (bp->gpio_role[b] == CH_DTR) {
                level = or3(not3(bit_of(c->mcr, 0)), bit_of(c->mcr, 4));
                if (nb_known1(br->ch[0].emsr, 4)) {
                    level = LOGIC_Z;   /* EMSR[4] 3-state */
                }
            }
            if (bit_of(c->mcr, 2) == LOGIC_X && level != LOGIC_Z) {
                level = LOGIC_X;
            }
        } else {
            LogicValue in = bit_of(br->gpiosel, b);

            if (in == LOGIC_1) {
                level = LOGIC_Z;
            } else if (in == LOGIC_X) {
                level = LOGIC_X;
            } else {
                LogicValue tri = bit_of(br->gpio3t, b);

                level = tri == LOGIC_1 ? LOGIC_Z : tri == LOGIC_X ? LOGIC_X : bit_of(br->latch, b);
            }
        }
        nb_drive(s, pin, level, delay);
    }
    /* PWRDN# (FCTR[0] = 1): low while asleep; else the pin is the SLEEP input. */
    if (bp->sleep_pin != SERIAL_NO_PIN) {
        LogicValue fn = bit_of(br->ch[0].fctr, 0);
        LogicValue level;

        if (s->unknown) {
            level = LOGIC_X;
        } else if (s->in_reset || fn == LOGIC_0) {
            level = LOGIC_Z;
        } else if (fn == LOGIC_X) {
            level = LOGIC_X;
        } else {
            level = bool3(!c->asleep);
        }
        nb_drive(s, bp->sleep_pin, level, 0);
    }
}

/* GPIOLVL read: inputs as pins (inverted by GPIOINV), outputs as the latch. */
static void gpiolvl(const NsState *s, uint8_t *v, uint8_t *k) {
    const NsBridge *br = s->bridge;
    unsigned b;

    *v = 0;
    *k = 0;
    for (b = 0; b < NB_GPIO; b++) {
        uint8_t m = (uint8_t)(1u << b);
        LogicValue l;

        if (gpio_is_input(s, b) || (s->pins->bridge->gpio_ch[b] >= 0 && xr_modem_on(s))) {
            l = nb_gpio_level(s, b);
            if (l != LOGIC_X && nb_known1(br->gpioinv, b)) {
                l = not3(l);
            } else if (bit_of(br->gpioinv, b) == LOGIC_X) {
                l = LOGIC_X;
            }
        } else {
            l = bit_of(br->latch, b);
        }
        if (l != LOGIC_X) {
            *k |= m;
            if (l == LOGIC_1) {
                *v |= m;
            }
        }
    }
}

/* ---- Registers ------------------------------------------------------------------------------- */

static unsigned fc_count(const NsChannel *c, int tx) {
    return tx ? c->tx_count : c->rx_count;
}

void xr_peek_reg(NsChannel *c, int reg, uint8_t *v, uint8_t *k) {
    NsState *s = c->owner;
    NsBridge *br = s->bridge;
    NbChannel *n = nb_ch(c);
    int sel = gen_select(n);

    *v = 0;
    *k = 0xFF;
    switch (reg) {
        case X_FC_TRG:
            if (bit_of(n->fctr, 7) == LOGIC_X) {
                *k = 0;
            } else {
                *v = (uint8_t)fc_count(c, nb_known1(n->fctr, 7));
            }
            break;
        case X_FCTR: *v = n->fctr.v; *k = n->fctr.k; break;
        case X_GPIOINT: *v = br->gpioint.v; *k = br->gpioint.k; break;
        case X_GPIO3T: *v = br->gpio3t.v; *k = br->gpio3t.k; break;
        case X_GPIOINV: *v = br->gpioinv.v; *k = br->gpioinv.k; break;
        case X_GPIOSEL: *v = br->gpiosel.v; *k = br->gpiosel.k; break;
        case X_GPIOLVL: gpiolvl(s, v, k); break;
        case X_EMSR_FC: {
            unsigned mode = n->emsr.v & 3u;
            int tx = mode == 1 || (mode == 3 && n->fc_toggle);

            *v = (uint8_t)fc_count(c, tx);
            break;
        }
        case X_DLL: {
            Reg r = sel == 1 ? n->dll_rx : n->dll_tx;
            Reg m = sel == 1 ? n->dlm_rx : n->dlm_tx;

            if (r.k == 0xFF && m.k == 0xFF && r.v == 0 && m.v == 0) {
                *k = 0;   /* DREV: value not printed (decisions.md) */
            } else {
                *v = r.v;
                *k = r.k;
            }
            break;
        }
        case X_DLM: {
            Reg r = sel == 1 ? n->dlm_rx : n->dlm_tx;

            *v = r.v;
            *k = r.k;
            break;
        }
        case X_DLD: {
            Reg r = sel == 1 ? n->dld_rx : n->dld_tx;

            *v = (uint8_t)((n->dld_tx.v & 0xC0u) | (r.v & 0x3Fu));
            *k = (uint8_t)((n->dld_tx.k & 0xC0u) | (r.k & 0x3Fu));
            break;
        }
        default:
            *k = 0;   /* write-only registers read X */
            break;
    }
}

static void set_gen(NbChannel *n, Reg *tx, Reg *rx, uint8_t v, uint8_t k) {
    int sel = gen_select(n);

    if (sel != 1) {
        *tx = (Reg){v, k};
    }
    if (sel != 0) {
        *rx = (Reg){v, k};
    }
}

/* A write the core handles (MCR, FCR), with the bridge's part first. */
static void pass_to_core(NsState *s, NsChannel *c, unsigned address, uint8_t v, uint8_t k) {
    NsBridge *br = s->bridge;

    br->raw = 1;
    ns_host_write(s, c->index, address, v, k);
    br->raw = 0;
}

void xr_write_reg(NsState *s, NsChannel *c, int reg, uint8_t v, uint8_t k) {
    NsBridge *br = s->bridge;
    NbChannel *n = nb_ch(c);

    switch (reg) {
        case X_FC_TRG:
            if (bit_of(n->fctr, 7) == LOGIC_X) {
                n->trg_rx = n->trg_tx = (Reg){0, 0};
            } else if (nb_known1(n->fctr, 7)) {
                n->trg_tx = (Reg){v, k};
            } else {
                n->trg_rx = (Reg){v, k};
            }
            break;
        case X_FCTR:
            n->fctr = (Reg){v, k};
            xr_sleep_pin(s);
            break;
        case X_GPIOINT: br->gpioint = (Reg){v, k}; break;
        case X_GPIO3T: br->gpio3t = (Reg){v, k}; break;
        case X_GPIOINV: br->gpioinv = (Reg){v, k}; break;
        case X_GPIOSEL:
            br->gpiosel = (Reg){v, k};
            xr_gpio_changed(s);
            break;
        case X_GPIOLVL: br->latch = (Reg){v, k}; break;
        case X_EMSR_FC: n->emsr = (Reg){v, k}; break;
        case X_SHR: n->shr = (Reg){v, k}; break;
        case X_SFR: n->xsfr = (Reg){v, k}; break;
        case X_DLL:
            set_gen(n, &n->dll_tx, &n->dll_rx, v, k);
            xr_load_divisors(c);
            break;
        case X_DLM:
            set_gen(n, &n->dlm_tx, &n->dlm_rx, v, k);
            xr_load_divisors(c);
            break;
        case X_DLD: {
            /* DLD[7:6] (the generator select) first, then DLD[5:0] to that generator. */
            uint8_t sel_v = v & 0xC0u;
            uint8_t sel_k = k & 0xC0u;

            n->dld_tx.v = (uint8_t)((n->dld_tx.v & 0x3Fu) | sel_v);
            n->dld_tx.k = (uint8_t)((n->dld_tx.k & 0x3Fu) | sel_k);
            if (gen_select(n) != 1) {
                n->dld_tx.v = (uint8_t)((n->dld_tx.v & 0xC0u) | (v & 0x3Fu));
                n->dld_tx.k = (uint8_t)((n->dld_tx.k & 0xC0u) | (k & 0x3Fu));
            }
            if (gen_select(n) != 0) {
                n->dld_rx = (Reg){(uint8_t)(v & 0x3Fu), (uint8_t)(k | 0xC0u)};
            }
            xr_load_divisors(c);
            ns_update_format(c);
            break;
        }
        case X_MCR: {
            uint8_t mask = ns_ext_mcr_mask(c);
            Reg next = {(uint8_t)((c->mcr.v & ~mask) | (v & mask)),
                        (uint8_t)((c->mcr.k & ~mask) | (k & mask))};

            /* MCR[2] moves 4 pins between the modem and the GPIOs: their
               MSR bits change function, not level (no deltas). */
            if (((next.v ^ c->mcr.v) & 0x04u) != 0 || ((next.k ^ c->mcr.k) & 0x04u) != 0) {
                c->mcr.v = (uint8_t)((c->mcr.v & ~0x04u) | (next.v & 0x04u));
                c->mcr.k = (uint8_t)((c->mcr.k & ~0x04u) | (next.k & 0x04u));
                ns_modem_refresh(s, c);
                xr_gpio_changed(s);
            }
            pass_to_core(s, c, 4, v, k);
            break;
        }
        case X_FCR:
            if (enhanced(c) && (k & 1u) && (v & 1u)) {
                n->wake_en = nb_known1((Reg){v, k}, 3);   /* FCR[3] wake-up interrupt enable */
            }
            pass_to_core(s, c, 2, v, k);
            break;
        default:
            break;
    }
}

int xr_read_done(NsChannel *c, int reg, Cause *cause) {
    NsState *s = c->owner;
    NbChannel *n = nb_ch(c);

    *cause = CAUSE_WRITE;
    if (reg == X_GPIOLVL) {
        s->bridge->xr_gpio_pending = 0;   /* a GPIOLVL read clears the GPIO interrupt */
        *cause = CAUSE_MSR_READ;
    } else if (reg == X_EMSR_FC && (n->emsr.v & 3u) == 3u) {
        n->fc_toggle = !n->fc_toggle;   /* EMSR[1:0] = 11: RX and TX counts alternate */
    }
    return 1;
}

void xr_reset(NsState *s, int power_on) {
    NsBridge *br = s->bridge;
    unsigned i;

    (void)power_on;
    br->gpioint = (Reg){0, 0xFF};
    br->gpio3t = (Reg){0, 0xFF};
    br->gpioinv = (Reg){0, 0xFF};
    br->gpiosel = (Reg){0xFF, 0xFF};
    br->latch = (Reg){0, 0xFF};
    br->xr_gpio_pending = 0;
    for (i = 0; i < s->dev->channel_count; i++) {
        NbChannel *n = &br->ch[i];

        n->shr = (Reg){0, 0xFF};
        n->xsfr = (Reg){0, 0xFF};
        n->emsr = (Reg){0, 0xFF};
        n->fctr = (Reg){0, 0xFF};
        n->trg_rx = (Reg){1, 0xFF};
        n->trg_tx = (Reg){1, 0xFF};
        n->dld_tx = (Reg){0, 0xFF};
        n->dld_rx = (Reg){0, 0xFF};
        n->wake_en = 0;
        n->wake_int = 0;
        n->ls_now = 0;
        memset(n->addr_bit, 0, sizeof n->addr_bit);
        n->rts_active = 0;
        n->rs_lead = 0;
        n->rs_hold = 0;
        n->holding = 0;
        n->xoff_wait = 0;
        n->xoff_sticky = 0;
        n->forced_sleep = 0;
        n->fc_toggle = 0;
        n->tx_factor = n->rx_factor = 16;
    }
}
