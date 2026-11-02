/*
 * ns16550: the 16450/16550 UART family (plan-12-C section 3.6). C's core is
 * the PC16550D (plan-12-data/specs/ti-a.md section PC16550D, with
 * decisions.md section PC16550D); "fifo": 0 makes it a 16450 (no FCR,
 * IIR[7:6] = 00, no timeout). D and E extend it with option keys.
 *
 * Signal rules (plan-12-A section 3.5): power-on is the MR state, then the
 * reset pin applies at start; DLL, DLM and RBR, which MR does not reset,
 * power on X. Inputs at Z read X, except MR and the chip selects (inactive).
 * Every register holds (value, known); a bus read drives X for unknown bits.
 *
 * Timing: bus delays from the grade (trvd, thz, trdd); BAUDOUT tbld after
 * the clock edge; interrupt and status pins after the bus access that
 * changed them (trint, thr, tir, trim, tmdo, twxi, twrr) or tsim after a
 * modem input; serial events act at the 16x tick they happen on. A received
 * character reaches the RBR/FIFO 1 RCLK after its stop-bit sample (3 in FIFO
 * mode, PC16550D p4 note 2); the timeout fires 4 character times plus 8
 * RCLKs after the last character or read.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ns16550_int.h"

static const char *const TIMING[] = {
    "trvd", "thz", "trdd", "tbld", "trint", "thr", "tmdo", "tsim", "trim", "twxi", "twrr", "tir",
    "tcso",
    /* plan-12-I: SPI SCLK to SO, CS to SO float, I2C SCL to SDA valid, GPIO output */
    "tdo", "ttr", "tvd", "tgpio", NULL
};
static const char *const KEYS[] = {
    "bus", "clock", "channels", "fifo", "rxTrigger", "intPin", "dmaClass", "timeout",
    /* plan-12-D */
    "channelSelect", "broadcastWrite", "rxTrigger64", "txTrigger", "tlr", "dmaPins", "dma",
    "intControl", "lcrReset", "sprReset", "tcrReset", "iirPriority", "lsr7Clear", "stopCheck",
    "statusReadDefer", "busSync", "majorityVote",
    "extended", "afr", "afrReset", "dlf", "autoFlow", "rtsTable", "xoffTable", "swFlow",
    "specialChar", "specialClear", "specialNeedsNoFlow", "ctsRtsClear", "sleep", "irda", "rs485",
    "prescaler", "lsrErrors", "mcrReset", "xonReset", "tcrNeedsDlab0", "dlabHidesUpper",
    "idRegs", "timeoutDelay", "divisorZero", "printerPort", "tristate", "absentTies",
    "optionalPins", "sprPowerUp", "dmaMode1", "lsrClearOnGood", "breakMarkTicks",
    /* plan-12-I: the I2C/SPI bridges */
    "interrupt", "gpio", "fastIrda", "dlPowerUp", "straps", "sleepPin",
    NULL
};
/* The plan-12-I keys, which need a serial host "bus". */
static const char *const BRIDGE_KEYS[] = {
    "interrupt", "gpio", "fastIrda", "dlPowerUp", "straps", "sleepPin", NULL
};
static const char *const CHANNEL_KEYS[] = {
    "tx", "rx", "rts", "dtr", "cts", "dsr", "dcd", "ri", "out1", "out2", "int", "txrdy", "rxrdy",
    NULL
};
/* Keys of a channel object besides its pins. */
static const char *const CHANNEL_EXTRA_KEYS[] = {"select", "selectHigh", "op", "mf", NULL};


const ChannelPin MODEM_PIN[4] = {CH_CTS, CH_DSR, CH_RI, CH_DCD};

void ns_drive(NsState *s, size_t pin, LogicValue value, SimTime delay) {
    if (!SERIAL_REAL_PIN(pin) || s->out[pin] == value) {
        return;
    }
    if (s->bridge != NULL && ns_bridge_owns(s, pin)) {
        return;   /* plan-12-I: a GPIO/modem pin the bridge drives */
    }
    s->out[pin] = value;
    /* TRI high (TL16C552): every output floats. */
    sim_schedule_inertial_drive(s->sim, s->instance, pin, s->tri ? LOGIC_Z : value, DRIVE_STRONG,
                                delay);
}

/* BAUDOUT: a clock, so a plain propagation delay (an inertial drive would
   swallow pulses shorter than tbld). */
static void drive_baudout(NsState *s, LogicValue value, SimTime delay) {
    size_t pin = s->pins->baudout;

    if (!SERIAL_REAL_PIN(pin) || s->out[pin] == value) {
        return;
    }
    s->out[pin] = value;
    sim_schedule_drive(s->sim, s->instance, pin, value, DRIVE_STRONG, delay);
}

static int loopback(const NsChannel *c) {
    return c->mcr.v & 0x10;
}

static void update_format(NsChannel *c) {
    uint8_t lcr = c->lcr.v;
    SerialFormat f = serial_format_default();

    f.data_bits = 5 + (lcr & 3u);
    f.stop_halves = lcr & 4u ? (f.data_bits == 5 ? 3 : 4) : 2;
    if (!(lcr & 8u)) {
        f.parity = SERIAL_PARITY_NONE;
    } else if (lcr & 0x20u) {
        f.parity = lcr & 0x10u ? SERIAL_PARITY_SPACE : SERIAL_PARITY_MARK;
    } else {
        f.parity = lcr & 0x10u ? SERIAL_PARITY_EVEN : SERIAL_PARITY_ODD;
    }
    f.format_x = (c->lcr.k & 0x3Fu) != 0x3Fu;
    f.majority = c->owner->dev->majority;
    f.check_both_stops = c->owner->dev->stop_both;
    f.mark_ticks = c->owner->dev->break_mark_ticks;
    f.factor = ns_ext_factor(c);
    c->format = f;
}

static void load_divisor(NsChannel *c) {
    c->baud.low2 = c->owner->dev->baudout_low2;
    ns_ext_load_divisor(c);
    /* COM92C451: divisor 0 divides by 3. */
    if (c->baud.known && c->baud.divisor == 0 && c->owner->dev->divisor_zero != 0) {
        c->baud.divisor = c->owner->dev->divisor_zero;
        c->baud.baudout = LOGIC_1;
    }
}

/* ---- FIFOs ---------------------------------------------------------------------------- */

static int char_has_error(const SerialRxChar *c) {
    return (c->flags & (SERIAL_RX_PE | SERIAL_RX_FE | SERIAL_RX_BI)) != 0 ||
           (c->flags_known & (SERIAL_RX_PE | SERIAL_RX_FE | SERIAL_RX_BI)) !=
               (SERIAL_RX_PE | SERIAL_RX_FE | SERIAL_RX_BI);
}

/* LSR[4:2] bits of a character: PE -> 2, FE -> 3, BI -> 4. */
static void char_lsr_bits(const SerialRxChar *rc, uint8_t *v, uint8_t *k) {
    *v = 0;
    *k = 0x1C;
    if (rc->flags & SERIAL_RX_PE) *v |= 0x04;
    if (rc->flags & SERIAL_RX_FE) *v |= 0x08;
    if (rc->flags & SERIAL_RX_BI) *v |= 0x10;
    if (!(rc->flags_known & SERIAL_RX_PE)) *k &= (uint8_t)~0x04;
    if (!(rc->flags_known & SERIAL_RX_FE)) *k &= (uint8_t)~0x08;
    if (!(rc->flags_known & SERIAL_RX_BI)) *k &= (uint8_t)~0x10;
}

/* ORs a character's error bits into LSR[4:2] (it reached the top). */
static void show_errors(NsChannel *c, const SerialRxChar *rc) {
    uint8_t v;
    uint8_t k;

    uint8_t ones;
    uint8_t unknown;

    char_lsr_bits(rc, &v, &k);
    ones = v & k;
    unknown = (uint8_t)(0x1C & ~k);
    /* A known 1 sets the bit; an unknown one makes it X unless already 1. */
    c->lsr_err |= ones;
    c->lsr_err_k |= ones;
    c->lsr_err_k &= (uint8_t)~(unknown & ~(c->lsr_err & c->lsr_err_k));
}

void ns_clear_rx(NsChannel *c) {
    c->rx_head = 0;
    c->rx_count = 0;
    c->timeout = 0;
    c->idle_ticks = 0;
    c->rxrdy1 = 0;
}

void ns_set_thre(NsChannel *c) {
    c->thre = 1;
    c->thre_wait = 0;
    /* With a TX trigger the THR interrupt comes at the trigger, not at empty. */
    if (ns_ext_tx_threshold(c) == 0) {
        c->thre_int = 1;
    }
    c->two_seen = 0;
    c->immediate = 0;
}

void ns_clear_tx(NsChannel *c) {
    c->tx_head = 0;
    c->tx_count = 0;
    c->flow_out_count = 0;
    if (!c->thre) {
        ns_set_thre(c);
    }
}

/* The RX trigger level in force: FCR (or the 750's 64-byte table), unless
   TLR overrides it; 1 with the FIFOs off. */
int ns_rx_level(const NsChannel *c) {
    if (!c->fifo_on) {
        return 1;
    }
    return (int)ns_ext_rx_trigger(c, rx_trigger_level(c));
}

/* Free TX FIFO spaces. */
static unsigned tx_free(const NsChannel *c) {
    return depth(c) - c->tx_count;
}

/* ---- Interrupts ------------------------------------------------------------------------ */

/* Each source: pending AND enabled, as 0 / 1 / X. */
typedef struct {
    LogicValue ls, rda, timeout, thre, modem, xoff, ctsrts, gpio;
} Sources;

static LogicValue any_bits(uint8_t v, uint8_t k, uint8_t mask) {
    if (v & k & mask) {
        return LOGIC_1;
    }
    return (k & mask) == mask ? LOGIC_0 : LOGIC_X;
}

static Sources sources(const NsChannel *c) {
    Sources r;
    LogicValue rda;

    const NsDevice *d = c->owner->dev;
    LogicValue thre = bool3(c->thre_int);

    rda = bool3(c->rx_count >= (unsigned)ns_rx_level(c) && c->rx_count > 0);
    if (d->lsr_head) {
        /* 752 style: OE (until an LSR read), or an errored byte anywhere in the FIFO. */
        r.ls = and3(or3(any_bits(c->lsr_err, c->lsr_err_k, 0x02), bool3(c->lsr7)),
                    bit_of(c->ier, 2));
    } else {
        r.ls = and3(any_bits(c->lsr_err, c->lsr_err_k, 0x1E), bit_of(c->ier, 2));
    }
    r.rda = and3(rda, bit_of(c->ier, 0));
    r.timeout = and3(bool3(c->fifo_on && c->timeout), bit_of(c->ier, 0));
    /* RS-485 with 485LG = 0 (TI AFR): TX interrupts wait for the TSR. */
    if (d->rs485 == RS485_AFR_DTR && bit_of(c->afr, 2) == LOGIC_1 && bit_of(c->afr, 3) != LOGIC_1 &&
        serial_tx_busy(&c->line.tx)) {
        thre = LOGIC_0;
    }
    r.thre = and3(thre, bit_of(c->ier, 1));
    r.modem = and3(any_bits(c->delta, c->delta_k, 0x0F), bit_of(c->ier, 3));
    r.xoff = ns_ext_xoff_source(c);
    r.ctsrts = ns_ext_ctsrts_source(c);
    r.gpio = LOGIC_0;
    if (d->bridge != NULL) {
        /* plan-12-I: the GPIO source; the bridges' line-status and THR rules. */
        r.gpio = ns_bridge_gpio_source(c);
        r.ls = ns_bridge_ls_source(c, r.ls);
        r.thre = ns_bridge_thr_source(c, r.thre);
    }
    return r;
}

void ns_sources(const NsChannel *c, LogicValue out[8]) {
    Sources r = sources(c);

    out[0] = r.ls;
    out[1] = r.rda;
    out[2] = r.timeout;
    out[3] = r.thre;
    out[4] = r.modem;
    out[5] = r.xoff;
    out[6] = r.ctsrts;
    out[7] = r.gpio;
}

/* IIR: the highest pending source. Unknown when an X decides it. */
static void iir_of(const NsChannel *c, uint8_t *v, uint8_t *k) {
    Sources s = sources(c);
    /* Standard: RX data (trigger reached) above the timeout; nxpEqual: one
       level, shown as the timeout while one is pending. */
    int timeout_first = c->owner->dev->iir_priority == IIR_NXP_EQUAL;
    /* plan-12-I: the SC16IS7xx GPIO source (0x30) between modem and Xoff. */
    const LogicValue order[8] = {s.ls, timeout_first ? s.timeout : s.rda,
                                 timeout_first ? s.rda : s.timeout, s.thre, s.modem, s.gpio,
                                 s.xoff, s.ctsrts};
    const uint8_t ID[8] = {0x06, timeout_first ? 0x0C : 0x04, timeout_first ? 0x04 : 0x0C,
                           0x02, 0x00, 0x30, 0x10, 0x20};
    unsigned i;

    *v = 0x01;
    *k = 0xFF;
    for (i = 0; i < 8; i++) {
        if (order[i] == LOGIC_X) {
            *k = 0;
            break;
        }
        if (order[i] == LOGIC_1) {
            *v = ID[i];
            break;
        }
    }
    if (c->fifo_on) {
        *v |= 0xC0;
        if (c->owner->dev->fifo750 && c->fifo64) {
            *v |= 0x20;   /* TL16C750: IIR[5] in 64-byte mode */
        }
    }
    if (c->fifo_x) {
        *k &= (uint8_t)~0xC0;
    }
    if (c->owner->dev->bridge != NULL) {
        ns_bridge_iir_bits(c, v, k);   /* plan-12-I: XR20M1280 ISR[6] */
    }
}

static LogicValue int_level(const NsChannel *c) {
    Sources s = sources(c);

    return or3(or3(or3(s.ls, s.rda), or3(s.timeout, s.thre)),
               or3(or3(s.modem, s.gpio), or3(s.xoff, s.ctsrts)));
}

void ns_iir_now(const NsChannel *c, uint8_t *v, uint8_t *k) {
    iir_of(c, v, k);
}

/* ---- Outputs ----------------------------------------------------------------------------- */

/* RXRDY (active low) for one channel. Mode 0: low with a character.
   Mode 1: the latch (trigger or timeout; cleared when empty); class T also
   goes inactive while LSR[7] is 1. */
static LogicValue rxrdy_level(const NsChannel *c, int mode1) {
    if (!mode1) {
        return bool3(c->rx_count == 0);
    }
    if (c->owner->dev->dma_class == 'T' && c->lsr7) {
        return LOGIC_1;
    }
    return bool3(!c->rxrdy1);
}

/* TXRDY (active low). Mode 0: low when the THR/FIFO is empty. Mode 1 by
   class: P low when empty, high when full (a latch); N and B low while a
   slot is free; T low while the free spaces reach the TX trigger. */
static LogicValue txrdy_level(const NsChannel *c, int mode1) {
    unsigned free_slots = depth(c) - c->tx_count;

    if (!mode1) {
        return bool3(c->tx_count != 0);
    }
    switch (c->owner->dev->dma_class) {
        case 'N':
        case 'B':
            return bool3(free_slots == 0);
        case 'T': {
            unsigned level = ns_ext_tx_threshold(c);

            return bool3(free_slots < (level != 0 ? level : 1));
        }
        default:
            return bool3(!c->txrdy1);
    }
}

/* The INT pin for an interrupt level, per "intPin": driven; OUT2-gated
   (3-state unless MCR[3] = 1, the register bit, loopback or not); INTN
   (pin high: driven, low or open: OUT2-gated); INTSEL (enabled by the pin OR
   MCR[3]); MCR[5] open source (SC16C650B: MCR[5] = 1 drives only the 1). */
static LogicValue int_pin_level(const NsState *s, const NsChannel *c, LogicValue level) {
    LogicValue out2 = bit_of(c->mcr, 3);
    LogicValue enable;

    switch (s->dev->int_pin) {
        case INT_DRIVEN:
            return level;
        case INT_OUT2_GATED:
            enable = out2;
            break;
        case INT_INTN:
            enable = or3(serial_level(s->sim, s->instance, s->pins->int_control, LOGIC_0), out2);
            break;
        case INT_INTSEL:
            enable = or3(serial_level(s->sim, s->instance, s->pins->int_control, LOGIC_X), out2);
            break;
        case INT_MCR5_OPEN_SOURCE: {
            LogicValue open = bit_of(c->mcr, 5);

            if (open == LOGIC_0) {
                return level;
            }
            if (open == LOGIC_1) {
                return level == LOGIC_1 ? LOGIC_1 : level == LOGIC_0 ? LOGIC_Z : LOGIC_X;
            }
            return level == LOGIC_1 ? LOGIC_1 : LOGIC_X;
        }
        default:
            return level;
    }
    if (enable == LOGIC_1) {
        return level;
    }
    return enable == LOGIC_0 ? LOGIC_Z : LOGIC_X;
}

static void update_channel_pins(NsState *s, NsChannel *c, Cause cause);

static void update_channel_outputs(NsState *s, NsChannel *c, Cause cause) {
    update_channel_pins(s, c, cause);
    if (s->bridge != NULL) {
        ns_bridge_outputs(s, cause);   /* plan-12-I: IRQ, GPIO and modem pins */
    }
}

static void update_channel_pins(NsState *s, NsChannel *c, Cause cause) {
    const size_t *pin = s->pins->channel[c->index];
    SimTime int_delay = 0;
    SimTime modem_delay = 0;
    SimTime txrdy_delay = 0;
    SimTime rxrdy_delay = 0;
    LogicValue loop = bit_of(c->mcr, 4);
    LogicValue sout;
    LogicValue intr;
    LogicValue rxrdy;
    LogicValue txrdy;
    int mode1 = c->fifo_on && c->dma_mode;
    unsigned i;

    switch (cause) {
        case CAUSE_RBR_READ: int_delay = t_of(s, T_RINT); rxrdy_delay = t_of(s, T_WRR); break;
        case CAUSE_LSR_READ: int_delay = t_of(s, T_RINT); break;
        case CAUSE_IIR_READ: int_delay = t_of(s, T_IR); break;
        case CAUSE_MSR_READ: int_delay = t_of(s, T_RIM); break;
        case CAUSE_THR_WRITE: int_delay = t_of(s, T_HR); txrdy_delay = t_of(s, T_WXI); break;
        case CAUSE_MCR_WRITE: modem_delay = t_of(s, T_MDO); int_delay = t_of(s, T_MDO); break;
        case CAUSE_MODEM_IN: int_delay = t_of(s, T_SIM); break;
        case CAUSE_SERIAL:
        case CAUSE_WRITE:
            break;
    }
    ns_ext_pins(s, c, cause);

    if (s->unknown) {
        for (i = 0; i < CH_PIN_COUNT; i++) {
            if (i != CH_RX && i != CH_CTS && i != CH_DSR && i != CH_DCD && i != CH_RI) {
                ns_drive(s, pin[i], LOGIC_X, 0);
            }
        }
        return;
    }
    if (s->in_reset) {
        ns_drive(s, pin[CH_TX], LOGIC_1, 0);
        ns_drive(s, pin[CH_RTS], LOGIC_1, 0);
        ns_drive(s, pin[CH_DTR], LOGIC_1, 0);
        ns_drive(s, pin[CH_OUT1], LOGIC_1, 0);
        ns_drive(s, pin[CH_OUT2], LOGIC_1, 0);
        ns_drive(s, pin[CH_INT], int_pin_level(s, c, LOGIC_0), 0);
        /* decisions.md (TL16C554, P3): RXRDY and TXRDY inactive during reset. */
        ns_drive(s, pin[CH_RXRDY], LOGIC_1, 0);
        ns_drive(s, pin[CH_TXRDY], LOGIC_1, 0);
        return;
    }

    /* SOUT: mark in loopback; 0 while break is set; else the transmitter. */
    sout = c->tsr;
    {
        LogicValue brk = bit_of(c->lcr, 6);

        if (brk == LOGIC_1) {
            sout = LOGIC_0;
        } else if (brk == LOGIC_X) {
            sout = sout == LOGIC_0 ? LOGIC_0 : LOGIC_X;
        }
    }
    sout = ns_ext_tx_pin(c, sout);   /* IrDA encodes after break */
    if (loop == LOGIC_1) {
        sout = LOGIC_1;
    } else if (loop == LOGIC_X && sout != LOGIC_1) {
        sout = LOGIC_X;
    }
    ns_drive(s, pin[CH_TX], sout, 0);

    /* Modem outputs: MCR bits inverted; forced high (inactive) in loopback.
       Auto-RTS drives RTS; RS-485 drives DTR. */
    {
        static const ChannelPin OUT_PIN[4] = {CH_DTR, CH_RTS, CH_OUT1, CH_OUT2};

        for (i = 0; i < 4; i++) {
            LogicValue level = not3(bit_of(c->mcr, i));

            if (i == 0) {
                level = ns_ext_dtr(c, level);
            } else if (i == 1) {
                level = ns_ext_rts(c, level);
            }
            level = or3(level, loop);
            ns_drive(s, pin[OUT_PIN[i]], level, modem_delay);
        }
    }

    /* INT: active high; driven, gated or open per "intPin". */
    intr = int_pin_level(s, c, int_level(c));
    ns_drive(s, pin[CH_INT], intr, int_delay);

    /* RXRDY / TXRDY (active low). */
    rxrdy = rxrdy_level(c, mode1);
    txrdy = txrdy_level(c, mode1);
    ns_drive(s, pin[CH_RXRDY], rxrdy, rxrdy_delay);
    ns_drive(s, pin[CH_TXRDY], txrdy, txrdy_delay);
}

/* Shared DMA pins (TL16C554: the channels' active-low signals ANDed;
   TL16C754B: wire-OR, open drain). */
static void update_shared_dma(NsState *s, Cause cause) {
    const NsDevice *d = s->dev;
    LogicValue rx = LOGIC_1;
    LogicValue tx = LOGIC_1;
    SimTime rx_delay = cause == CAUSE_RBR_READ ? t_of(s, T_WRR) : 0;
    SimTime tx_delay = cause == CAUSE_THR_WRITE ? t_of(s, T_WXI) : 0;
    unsigned i;

    if (d->dma_pins == DMA_PINS_NAMED) {
        return;
    }
    if (s->unknown) {
        rx = tx = LOGIC_X;
    } else if (!s->in_reset) {
        for (i = 0; i < d->channel_count; i++) {
            const NsChannel *c = &s->ch[i];
            int mode1 = c->fifo_on && c->dma_mode;

            rx = and3(rx, rxrdy_level(c, mode1));
            tx = and3(tx, txrdy_level(c, mode1));
        }
    }
    if (d->dma_pins == DMA_PINS_SHARED_WIRE_OR) {
        rx = rx == LOGIC_1 ? LOGIC_Z : rx;
        tx = tx == LOGIC_1 ? LOGIC_Z : tx;
    }
    ns_drive(s, s->pins->dma_rxrdy, rx, rx_delay);
    ns_drive(s, s->pins->dma_txrdy, tx, tx_delay);
}

static void update_outputs(NsState *s, Cause cause) {
    unsigned i;

    for (i = 0; i < s->dev->channel_count; i++) {
        update_channel_outputs(s, &s->ch[i], cause);
    }
    update_shared_dma(s, cause);
    ns_lpt_pins(s);
    serial_bus_refresh(&s->bus, s->sim, s->instance, &s->events);
}

void ns_update_outputs(NsState *s, Cause cause) {
    update_outputs(s, cause);
}

void ns_update_channel(NsState *s, NsChannel *c, Cause cause) {
    update_channel_outputs(s, c, cause);
}

static void update_dma_latches(NsChannel *c) {
    if (c->rx_count == 0) {
        c->rxrdy1 = 0;
    } else if (c->rx_count >= rx_trigger_level(c) || c->timeout) {
        c->rxrdy1 = 1;
    }
    if (c->tx_count == 0) {
        c->txrdy1 = 1;
    } else if (c->tx_count >= depth(c)) {
        c->txrdy1 = 0;
    }
}

/* ---- Modem status --------------------------------------------------------------------------- */

/* Sets the status bits to `now`, recording deltas (TERI: RI 1 -> 0). */
static void set_modem(NsChannel *c, const LogicValue now[4], uint8_t record) {
    unsigned i;

    for (i = 0; i < 4; i++) {
        LogicValue before = c->msr[i];
        uint8_t bit = (uint8_t)(1u << i);
        int changed;
        int unknown;

        if (before == now[i]) {
            continue;
        }
        if (i == M_RI) {
            changed = before == LOGIC_1 && now[i] == LOGIC_0;
            unknown = !changed && (before == LOGIC_X ? now[i] != LOGIC_1 : before == LOGIC_1);
        } else {
            changed = before != LOGIC_X && now[i] != LOGIC_X;
            unknown = !changed;
        }
        if (!(record & bit)) {
            c->msr[i] = now[i];
            continue;
        }
        if (changed) {
            c->delta |= bit;
            c->delta_k |= bit;
        } else if (unknown && !((c->delta & c->delta_k) & bit)) {
            c->delta_k &= (uint8_t)~bit;
        }
        c->msr[i] = now[i];
    }
}

/* The status the modem bits should have now: the pins, or MCR in loopback. */
static void modem_now(const NsState *s, const NsChannel *c, LogicValue now[4]) {
    LogicValue loop = bit_of(c->mcr, 4);
    unsigned i;

    for (i = 0; i < 4; i++) {
        size_t pin = s->pins->channel[c->index][MODEM_PIN[i]];
        /* Absent pins read X (decisions.md); present ones are active low.
           plan-12-I: a bridge's modem pin that is a GPIO now reads as absent. */
        LogicValue z_reads = LOGIC_X;
        LogicValue from_pin = pin == SERIAL_NO_PIN ||
                                      (s->bridge != NULL &&
                                       !ns_bridge_modem_input(s, c->index, pin, &z_reads))
                                  ? LOGIC_X
                                  : not3(serial_level(s->sim, s->instance, pin, z_reads));
        /* Loopback: RTS -> CTS, DTR -> DSR, OUT1 -> RI, OUT2 -> DCD. */
        static const unsigned FROM_MCR[4] = {1, 0, 2, 3};
        LogicValue from_mcr = bit_of(c->mcr, FROM_MCR[i]);

        if (loop == LOGIC_1) {
            now[i] = from_mcr;
        } else if (loop == LOGIC_0) {
            now[i] = from_pin;
        } else {
            now[i] = from_pin == from_mcr ? from_pin : LOGIC_X;
        }
    }
}

/* ---- Reset -------------------------------------------------------------------------------- */

static void reset_channel(NsState *s, NsChannel *c, int power_on) {
    LogicValue now[4];

    c->ier = (Reg){0, 0xFF};
    c->lcr = (Reg){s->dev->lcr_reset, 0xFF};
    c->mcr = (Reg){0, 0xFF};
    if (s->dev->spr_reset >= 0) {
        c->scr = (Reg){(uint8_t)s->dev->spr_reset, 0xFF};
    }
    c->fifo_on = 0;
    c->fifo_x = 0;
    c->fifo64 = 0;
    c->reading_status = 0;
    c->dma_mode = 0;
    c->trigger = 0;
    ns_clear_rx(c);
    c->pending_count = 0;
    c->lsr_err = 0;
    c->lsr_err_k = 0xFF;
    c->lsr7 = 0;
    c->tx_head = 0;
    c->tx_count = 0;
    c->thre = 1;
    c->thre_wait = 0;
    c->thre_int = 0;
    c->two_seen = 0;
    c->immediate = 0;
    c->delta = 0;
    c->delta_k = 0xFF;
    c->txrdy1 = 1;
    serial_tx_reset(&c->line.tx);
    serial_rx_reset(&c->line.rx);
    c->tsr = LOGIC_1;
    ns_ext_reset(s, c, power_on);
    update_format(c);
    if (s->sim != NULL) {
        modem_now(s, c, now);
        memcpy(c->msr, now, sizeof now);
    }
}

static void reset_all(NsState *s) {
    unsigned i;

    for (i = 0; i < s->dev->channel_count; i++) {
        reset_channel(s, &s->ch[i], 0);
    }
    ns_lpt_reset(s);
}

void ns_reset_all(NsState *s) {
    reset_all(s);
}

void ns_modem_refresh(NsState *s, NsChannel *c) {
    LogicValue now[4];

    modem_now(s, c, now);
    memcpy(c->msr, now, sizeof now);
}

/* ---- Receiving ------------------------------------------------------------------------------ */

static void deliver_char(NsChannel *c, const SerialRxChar *rc) {
    unsigned d = depth(c);

    c->idle_ticks = 0;
    c->timeout = 0;
    /* NS16C2552/2752: the LSR error bits also reset when the next valid
       character loads into the RBR (LSR text). */
    if (c->owner->dev->lsr_clear_on_good && !char_has_error(rc) && c->rx_count == 0) {
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
            show_errors(c, rc);
        }
        return;
    }
    c->rx[(c->rx_head + c->rx_count) % NS_MAX_FIFO] = *rc;
    c->rx_count++;
    if (c->rx_count == 1) {
        c->rbr = (Reg){(uint8_t)rc->data, (uint8_t)rc->known};
        show_errors(c, rc);
    }
    if (c->fifo_on && char_has_error(rc)) {
        c->lsr7 = 1;
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

static void rx_tick(NsState *s, NsChannel *c) {
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
    if (loopback(c)) {
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
        update_dma_latches(c);
        update_channel_outputs(s, c, CAUSE_SERIAL);
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
    if (threshold != 0 && tx_free(c) >= threshold && tx_free(c) - 1 < threshold) {
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

static void tx_tick(NsState *s, NsChannel *c) {
    SerialTxEvents events = {c, tx_fetch, tx_last_stop, tx_done};

    if (s->in_reset || s->unknown) {
        return;
    }
    c->tsr = serial_tx_tick(&c->line.tx, &c->format, &events);
    if (s->dev->bridge != NULL && c->format.parity == SERIAL_PARITY_MARK) {
        update_format(c);   /* plan-12-I: undo a 9th-bit override (tx_fetch) */
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
    ns_ext_levels(s, c);
    update_dma_latches(c);
    update_channel_outputs(s, c, CAUSE_SERIAL);
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

            char_lsr_bits(&c->rx[c->rx_head], &hv, &hk);
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

static void reg_peek(NsChannel *c, unsigned address, uint8_t *v, uint8_t *k) {
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
            iir_of(c, v, k);
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
static Cause reg_read_done(NsChannel *c, unsigned address) {
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
                    show_errors(c, &c->rx[c->rx_head]);
                }
                if (c->owner->dev->lsr_head) {
                    unsigned i;

                    c->lsr7 = 0;
                    for (i = 0; i < c->rx_count; i++) {
                        if (char_has_error(&c->rx[(c->rx_head + i) % NS_MAX_FIFO])) {
                            c->lsr7 = 1;
                        }
                    }
                }
            }
            c->timeout = 0;
            c->idle_ticks = 0;
            update_dma_latches(c);
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
                if (char_has_error(&c->rx[(c->rx_head + i) % NS_MAX_FIFO])) {
                    c->lsr7 = 1;
                }
            }
            /* The 752 style: LSR[7] while any errored byte remains, the head too. */
            if (c->owner->dev->lsr_head && c->rx_count > 0 &&
                char_has_error(&c->rx[c->rx_head])) {
                c->lsr7 = 1;
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
    update_dma_latches(c);
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
    update_dma_latches(c);
}

static void write_mcr(NsState *s, NsChannel *c, uint8_t data, uint8_t known) {
    LogicValue now[4];

    uint8_t mask = ns_ext_mcr_mask(c);

    c->mcr = (Reg){(uint8_t)((c->mcr.v & ~mask) | (data & mask)),
                   (uint8_t)((c->mcr.k & ~mask) | (known & mask))};
    if (s->dev->prescaler != PRESCALE_NONE) {
        load_divisor(c);
    }
    modem_now(s, c, now);
    set_modem(c, now, 0x0F);
}

static Cause reg_write(NsState *s, NsChannel *c, unsigned address, uint8_t data, uint8_t known) {
    LogicValue dlab = bit_of(c->lcr, 7);
    Cause cause;

    if (ns_ext_write(s, c, address, data, known, &cause)) {
        return cause;
    }
    switch (address & 7u) {
        case 0:
            if (dlab != LOGIC_0) {
                c->dll = (Reg){data, known};
                load_divisor(c);
            }
            if (dlab != LOGIC_1) {
                write_thr(c, data, known);
                return CAUSE_THR_WRITE;
            }
            return CAUSE_WRITE;
        case 1:
            if (dlab != LOGIC_0) {
                c->dlm = (Reg){data, known};
                load_divisor(c);
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
                    (threshold == 0 ? c->thre : tx_free(c) >= threshold) &&
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
            update_format(c);
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
static LogicValue bus_select(void *context, const Sim *sim, const SimInstance *instance,
                             unsigned address, unsigned address_known, uint32_t *units) {
    NsState *s = context;
    const NsDevice *d = s->dev;
    const SerialBusPins *bp = &s->pins->bus;
    LogicValue common = selects_active(s, bp->select, bp->select_count, bp->select_high,
                                       bp->select_high_count);
    unsigned i;

    (void)sim;
    (void)instance;
    (void)address;
    (void)address_known;
    *units = 1;
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

static void bus_peek(void *context, unsigned unit, unsigned address, uint8_t *data, uint8_t *known) {
    NsState *s = context;

    *data = 0;
    *known = 0;
    if (s->unknown) {
        return;
    }
    if (unit == s->dev->channel_count && s->lpt != NULL) {
        ns_lpt_peek(s, address, data, known);
        return;
    }
    if (unit < s->dev->channel_count) {
        reg_peek(&s->ch[unit], address, data, known);
    }
}

static void bus_read_done(void *context, unsigned unit, unsigned address) {
    NsState *s = context;
    Cause cause;

    if (s->unknown) {
        return;
    }
    if (unit == s->dev->channel_count && s->lpt != NULL) {
        ns_lpt_read_done(s, address);
        return;
    }
    if (unit >= s->dev->channel_count) {
        return;
    }
    cause = reg_read_done(&s->ch[unit], address);
    ns_ext_levels(s, &s->ch[unit]);
    update_channel_outputs(s, &s->ch[unit], cause);
    update_shared_dma(s, cause);
}

static void bus_write(void *context, unsigned unit, unsigned address, uint8_t data, uint8_t known) {
    NsState *s = context;
    Cause cause;

    unsigned i;

    if (s->unknown) {
        return;
    }
    if (unit == s->dev->channel_count && s->lpt != NULL) {
        ns_lpt_write(s, address, data, known);
        return;
    }
    if (unit >= s->dev->channel_count) {
        return;
    }
    /* AFR concurrent write: every channel takes it. */
    if (s->dev->afr != AFR_NONE && s->conc) {
        for (i = 0; i < s->dev->channel_count; i++) {
            cause = reg_write(s, &s->ch[i], address, data, known);
            update_channel_outputs(s, &s->ch[i], cause);
        }
        update_shared_dma(s, CAUSE_WRITE);
        return;
    }
    cause = reg_write(s, &s->ch[unit], address, data, known);
    update_channel_outputs(s, &s->ch[unit], cause);
    update_shared_dma(s, cause);
}

/* ---- plan-12-I: register access from a serial host port -------------------------------------- */

void ns_host_peek(NsState *s, unsigned ch, unsigned address, uint8_t *v, uint8_t *k) {
    bus_peek(s, ch, address, v, k);
}

void ns_host_read_done(NsState *s, unsigned ch, unsigned address) {
    bus_read_done(s, ch, address);
}

void ns_host_write(NsState *s, unsigned ch, unsigned address, uint8_t v, uint8_t k) {
    bus_write(s, ch, address, v, k);
}

void ns_tx_tick(NsState *s, NsChannel *c) {
    tx_tick(s, c);
}

void ns_rx_tick(NsState *s, NsChannel *c) {
    rx_tick(s, c);
}

void ns_update_format(NsChannel *c) {
    update_format(c);
}

/* ---- Engine callbacks ----------------------------------------------------------------------------- */

static LogicValue reset_level(const NsState *s) {
    LogicValue inactive = s->dev->reset_active_high ? LOGIC_0 : LOGIC_1;
    LogicValue v = serial_level(s->sim, s->instance, s->pins->reset, inactive);

    if (v == LOGIC_X) {
        return LOGIC_X;
    }
    return bool3(v == (s->dev->reset_active_high ? LOGIC_1 : LOGIC_0));
}

static void apply_reset(NsState *s) {
    LogicValue active = reset_level(s);

    if (active == LOGIC_X) {
        s->unknown = 1;
        s->in_reset = 0;
        s->bus.enabled = 0;
    } else if (active == LOGIC_1) {
        s->unknown = 0;
        s->in_reset = 1;
        s->bus.enabled = 0;
        reset_all(s);
    } else if (s->in_reset) {
        s->in_reset = 0;
        s->bus.enabled = 1;
    } else if (!s->unknown) {
        s->bus.enabled = 1;
    }
    if (s->bridge != NULL) {
        s->bus.enabled = 0;   /* plan-12-I: no parallel bus; the host port follows reset */
        ns_bridge_reset(s, active);
    }
    serial_bus_update(&s->bus, s->sim, s->instance, &s->events);
    update_outputs(s, CAUSE_SERIAL);
}

static void clock_edge(NsState *s, LogicValue level) {
    int rising = s->clock_level == LOGIC_0 && level == LOGIC_1;
    unsigned i;

    if (level == s->clock_level) {
        return;
    }
    if (!rising && !(s->clock_level == LOGIC_1 && level == LOGIC_0)) {
        /* Into or out of X: no edge; BAUDOUT follows a divisor-1 clock. */
        s->clock_level = level;
        for (i = 0; i < s->dev->channel_count; i++) {
            if (s->ch[i].baud.known && s->ch[i].baud.divisor == 1) {
                s->ch[i].baud.baudout = level == LOGIC_Z ? LOGIC_X : level;
            }
        }
        drive_baudout(s, s->ch[0].baud.baudout, t_of(s, T_BLD));
        return;
    }
    s->clock_level = level;
    if (rising && s->dev->bus_sync && s->bus_dirty) {
        s->bus_dirty = 0;
        serial_bus_update(&s->bus, s->sim, s->instance, &s->events);
    }
    for (i = 0; i < s->dev->channel_count; i++) {
        NsChannel *c = &s->ch[i];
        int tick = 0;
        LogicValue baudout;

        if (s->bridge != NULL && ns_bridge_clock(s, c, rising, level)) {
            continue;   /* plan-12-I: XR20M1280 independent TX/RX generators */
        }
        if (ns_ext_prescale(s, c, rising) || (!rising && !c->asleep &&
                                              !(s->dev->prescaler != PRESCALE_NONE &&
                                                bit_of(c->mcr, 7) == LOGIC_1))) {
            tick = serial_baud_edge(&c->baud, rising, level);
        }
        /* Asleep (or the 750's low-power mode) the baud clock stops: BAUDOUT X. */
        baudout = s->unknown || c->asleep ? LOGIC_X : c->baud.baudout;
        if (i == 0) {
            drive_baudout(s, baudout, t_of(s, T_BLD));
        }
        if (SERIAL_REAL_PIN(s->pins->mf[i]) && ((c->afr.v >> 1) & 3u) == 1 &&
            s->out[s->pins->mf[i]] != baudout) {
            s->out[s->pins->mf[i]] = baudout;
            sim_schedule_drive(s->sim, s->instance, s->pins->mf[i], s->tri ? LOGIC_Z : baudout,
                               DRIVE_STRONG, t_of(s, T_BLD));
        }
        if (tick) {
            tx_tick(s, c);
            if (s->pins->receive == SERIAL_NO_PIN) {
                rx_tick(s, c);
            }
        }
    }
}

static void modem_changed(NsState *s, NsChannel *c) {
    LogicValue now[4];

    if (s->in_reset || s->unknown) {
        modem_now(s, c, now);
        memcpy(c->msr, now, sizeof now);
        return;
    }
    modem_now(s, c, now);
    set_modem(c, now, ns_ext_cts_changed(c, now[M_CTS]) ? 0x0F : 0x0E);
    update_channel_outputs(s, c, CAUSE_MODEM_IN);
    serial_bus_refresh(&s->bus, s->sim, s->instance, &s->events);
}

static void ns_start(Sim *sim, SimInstance *instance, void *raw) {
    NsState *s = raw;
    unsigned i;

    s->sim = sim;
    s->instance = instance;
    for (i = 0; i < s->dev->channel_count; i++) {
        LogicValue now[4];

        modem_now(s, &s->ch[i], now);
        memcpy(s->ch[i].msr, now, sizeof now);
    }
    s->clock_level = serial_level(sim, instance, s->pins->clock_in, LOGIC_X);
    s->receive_level = serial_level(sim, instance, s->pins->receive, LOGIC_X);
    drive_baudout(s, s->ch[0].baud.baudout, 0);
    s->tri = serial_level(sim, instance, s->pins->tri, LOGIC_0) == LOGIC_1;
    serial_bus_start(&s->bus, sim, instance, &s->events);
    if (s->bridge != NULL) {
        ns_bridge_start(s);   /* plan-12-I */
    }
    apply_reset(s);
}

static int is_select_pin(const NsState *s, size_t pin) {
    unsigned i;
    unsigned k;

    if (pin == SERIAL_NO_PIN) {
        return 0;
    }
    if (pin == s->pins->channel_pin || (s->pins->lpt != NULL && pin == s->pins->lpt->select)) {
        return 1;
    }
    for (i = 0; i < s->dev->channel_count; i++) {
        for (k = 0; k < s->dev->chan_select_count[i]; k++) {
            if (s->pins->chan_select[i][k] == pin) {
                return 1;
            }
        }
        for (k = 0; k < s->dev->chan_select_high_count[i]; k++) {
            if (s->pins->chan_select_high[i][k] == pin) {
                return 1;
            }
        }
    }
    return 0;
}

static void ns_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    NsState *s = raw;
    unsigned i;
    unsigned j;

    (void)old_value;
    (void)new_value;
    s->sim = sim;
    s->instance = instance;
    if (pin == s->pins->reset) {
        apply_reset(s);
        return;
    }
    if (s->bridge != NULL) {
        ns_bridge_pin(s, pin);   /* plan-12-I: host port, GPIO, straps */
    }
    if (pin == s->pins->clock_in) {
        clock_edge(s, serial_level(sim, instance, pin, LOGIC_X));
    }
    if (pin == s->pins->receive) {
        LogicValue level = serial_level(sim, instance, pin, LOGIC_X);
        int rising = s->receive_level == LOGIC_0 && level == LOGIC_1;

        s->receive_level = level;
        if (rising) {
            for (i = 0; i < s->dev->channel_count; i++) {
                rx_tick(s, &s->ch[i]);
            }
        }
    }
    if (serial_bus_has_pin(&s->bus, pin) || is_select_pin(s, pin)) {
        if (s->dev->bus_sync) {
            s->bus_dirty = 1;   /* sampled at the next XIN rising edge */
            serial_bus_capture(&s->bus, sim, instance);
        } else {
            serial_bus_update(&s->bus, sim, instance, &s->events);
        }
    }
    if (pin == s->pins->int_control) {
        update_outputs(s, CAUSE_SERIAL);
    }
    if (pin == s->pins->tri) {
        LogicValue tri = serial_level(sim, instance, pin, LOGIC_0);
        size_t p;

        s->tri = tri == LOGIC_1;
        for (p = 0; p < s->pin_count; p++) {
            if (s->out[p] != LOGIC_Z || s->tri) {
                sim_schedule_drive(sim, instance, p, s->tri ? LOGIC_Z : s->out[p], DRIVE_STRONG, 0);
            }
        }
    }
    if (ns_lpt_has_pin(s, pin)) {
        ns_lpt_pins(s);
    }
    for (i = 0; i < s->dev->channel_count; i++) {
        if (pin == s->pins->channel[i][CH_RX]) {
            ns_ext_wake(&s->ch[i]);   /* sleep wakes on RX activity */
        }
    }
    for (i = 0; i < s->dev->channel_count; i++) {
        for (j = 0; j < 4; j++) {
            if (pin == s->pins->channel[i][MODEM_PIN[j]]) {
                modem_changed(s, &s->ch[i]);
            }
        }
    }
}

/* plan-12-I: the bridges' timers (the I2C SDA-low timeout). */
static void ns_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t timer_id) {
    NsState *s = raw;

    s->sim = sim;
    s->instance = instance;
    if (s->bridge != NULL) {
        ns_bridge_timer(s, timer_id);
    }
}

/* ---- Terminal ---------------------------------------------------------------------------------- */

static unsigned ns_channels(const void *raw) {
    return ((const NsState *)raw)->dev->channel_count;
}

static SerialLine *ns_line(void *raw, unsigned channel) {
    return &((NsState *)raw)->ch[channel].line;
}

static int ns_async(const void *raw, unsigned channel) {
    (void)raw;
    (void)channel;
    return 1;
}

/* ---- Create / destroy ------------------------------------------------------------------------------ */

static int d_needs_select(const NsDevice *d) {
    return d->channel_pin != NULL || d->per_channel_select || d->lpt != NULL;
}

static void *ns_create(const Part *part, const void *data, const void *pins,
                       const SerialGrade *grade, const void *options) {
    NsState *s = calloc(1, sizeof *s);
    unsigned i;

    (void)options;
    if (s == NULL) {
        return NULL;
    }
    s->base.model = &NS16550_MODEL;
    s->part = part;
    s->dev = data;
    s->pins = pins;
    s->grade = grade;
    s->pin_count = part->pin_count;
    s->out = malloc(part->pin_count * sizeof *s->out);
    if (s->out == NULL) {
        free(s);
        return NULL;
    }
    for (i = 0; i < part->pin_count; i++) {
        s->out[i] = LOGIC_Z;
    }
    if (s->dev->lpt != NULL) {
        s->lpt = ns_lpt_create(s);
    }
    {
        SerialBusTiming timing = {grade->t[T_RVD], grade->t[T_HZ], grade->t[T_RDD], 0};

        serial_bus_init(&s->bus, &s->pins->bus, &timing);
    }
    s->events.context = s;
    s->events.select = d_needs_select(s->dev) ? bus_select : NULL;
    s->events.peek = bus_peek;
    s->events.read_done = bus_read_done;
    s->events.write = bus_write;
    for (i = 0; i < s->dev->channel_count; i++) {
        NsChannel *c = &s->ch[i];

        c->owner = s;
        c->index = i;
        serial_line_init(&c->line, s->pins->channel[i][CH_RX]);
        /* Power-on: MR's state; DLL, DLM and RBR are not reset, so X. */
        c->dll = (Reg){0, 0};
        c->dlm = (Reg){0, 0};
        c->rbr = (Reg){0, 0};
        c->scr = s->dev->spr_power_up >= 0 ? (Reg){(uint8_t)s->dev->spr_power_up, 0xFF} : (Reg){0, 0};
        c->msr[0] = c->msr[1] = c->msr[2] = c->msr[3] = LOGIC_X;
        reset_channel(s, c, 1);
        load_divisor(c);
    }
    s->clock_level = LOGIC_X;
    s->receive_level = LOGIC_X;
    if (s->dev->bridge != NULL && ns_bridge_create(s) != 0) {
        ns_lpt_destroy(s->lpt);
        free(s->out);
        free(s);
        return NULL;
    }
    return s;
}

static void ns_destroy(void *raw) {
    NsState *s = raw;
    unsigned i;

    for (i = 0; i < s->dev->channel_count; i++) {
        serial_line_free(&s->ch[i].line);
    }
    ns_lpt_destroy(s->lpt);
    if (s->bridge != NULL) {
        ns_bridge_destroy(s);
    }
    free(s->out);
    free(s);
}

/* ---- Loader ---------------------------------------------------------------------------------------- */

static int load_name_list(KindContext *ctx, const cJSON *object, const char *key, int required,
                          const char **out, unsigned cap, unsigned *count, unsigned exact) {
    const char *const *names;
    size_t n;
    size_t i;

    *count = 0;
    if (cJSON_GetObjectItemCaseSensitive(object, key) == NULL) {
        if (required) {
            return kind_error(ctx, "missing \"%s\"", key);
        }
        return 0;
    }
    if (kind_names(ctx, object, key, &names, &n) != 0) {
        return -1;
    }
    if ((exact != 0 && n != exact) || n > cap) {
        if (exact != 0) {
            return kind_error(ctx, "\"%s\" needs exactly %u pin names", key, exact);
        }
        return kind_error(ctx, "\"%s\" has at most %u pin names", key, cap);
    }
    for (i = 0; i < n; i++) {
        out[i] = names[i];
    }
    *count = (unsigned)n;
    return 0;
}

/* 1 = the bus is a serial host port (plan-12-I, loaded by ns_bridge_load). */
static int load_bus(KindContext *ctx, const cJSON *json, NsDevice *d) {
    static const char *const BUS_KEYS[] = {
        "style", "data", "address", "read", "readHigh", "write", "writeHigh", "select",
        "selectHigh", "addressStrobe", "driverDisable", "selectOut", "reset", "resetActiveHigh",
        NULL
    };
    const cJSON *bus = kind_member(ctx, json, "bus", 1);
    const char *style;
    unsigned n;

    if (bus == NULL) {
        return -1;
    }
    kind_push(ctx, "bus");
    if (kind_string(ctx, bus, "style", &style) != 0) {
        return -1;
    }
    if (strcmp(style, "intel") == 0 && kind_check_keys(ctx, bus, BUS_KEYS, NULL) != 0) {
        return -1;
    }
    if (strcmp(style, "spi") == 0 || strcmp(style, "i2c") == 0 || strcmp(style, "i2c-spi") == 0) {
        kind_pop(ctx);
        return 1;   /* plan-12-I */
    }
    if (strcmp(style, "intel") != 0) {
        return kind_error(ctx, "unknown bus style \"%s\"", style);
    }
    if (load_name_list(ctx, bus, "data", 1, d->data, SERIAL_BUS_DATA, &n, SERIAL_BUS_DATA) != 0 ||
        load_name_list(ctx, bus, "address", 1, d->address, SERIAL_BUS_ADDRESS, &d->address_count,
                       0) != 0 ||
        load_name_list(ctx, bus, "select", 0, d->select, SERIAL_BUS_SELECTS, &d->select_count, 0) != 0 ||
        load_name_list(ctx, bus, "selectHigh", 0, d->select_high, SERIAL_BUS_SELECTS,
                       &d->select_high_count, 0) != 0 ||
        serial_optional_name(ctx, bus, "read", &d->read) != 0 ||
        serial_optional_name(ctx, bus, "readHigh", &d->read_high) != 0 ||
        serial_optional_name(ctx, bus, "write", &d->write) != 0 ||
        serial_optional_name(ctx, bus, "writeHigh", &d->write_high) != 0 ||
        serial_optional_name(ctx, bus, "addressStrobe", &d->address_strobe) != 0 ||
        serial_optional_name(ctx, bus, "driverDisable", &d->driver_disable) != 0 ||
        serial_optional_name(ctx, bus, "selectOut", &d->select_out) != 0 ||
        serial_optional_name(ctx, bus, "reset", &d->reset) != 0) {
        return -1;
    }
    if (d->address_count != 3) {
        return kind_error(ctx, "\"address\" needs exactly 3 pin names");
    }
    if (d->read == NULL && d->read_high == NULL) {
        return kind_error(ctx, "a bus needs \"read\" or \"readHigh\"");
    }
    if (d->write == NULL && d->write_high == NULL) {
        return kind_error(ctx, "a bus needs \"write\" or \"writeHigh\"");
    }
    d->reset_active_high = 1;
    if (cJSON_GetObjectItemCaseSensitive(bus, "resetActiveHigh") != NULL &&
        kind_bool(ctx, bus, "resetActiveHigh", &d->reset_active_high) != 0) {
        return -1;
    }
    kind_pop(ctx);
    return 0;
}

static int load_clock(KindContext *ctx, const cJSON *json, NsDevice *d) {
    static const char *const CLOCK_KEYS[] = {"in", "out", "receive", "baudOut", "single", "select",
                                             "baudOutShape", NULL};
    const cJSON *clock = kind_member(ctx, json, "clock", 1);
    const char *single = NULL;

    if (clock == NULL) {
        return -1;
    }
    kind_push(ctx, "clock");
    if (kind_check_keys(ctx, clock, CLOCK_KEYS, NULL) != 0 ||
        serial_optional_name(ctx, clock, "single", &single) != 0) {
        return -1;
    }
    /* "single": one clock input for both directions (no RCLK, no BAUDOUT). */
    if (single != NULL) {
        if (cJSON_GetObjectItemCaseSensitive(clock, "in") != NULL ||
            cJSON_GetObjectItemCaseSensitive(clock, "receive") != NULL) {
            return kind_error(ctx, "\"single\" replaces \"in\" and \"receive\"");
        }
        d->clock_in = single;
    } else if (kind_string(ctx, clock, "in", &d->clock_in) != 0) {
        return -1;
    }
    if (cJSON_GetObjectItemCaseSensitive(clock, "baudOutShape") != NULL) {
        const char *shape;

        if (kind_string(ctx, clock, "baudOutShape", &shape) != 0) {
            return -1;
        }
        if (strcmp(shape, "low2") != 0 && strcmp(shape, "half") != 0) {
            return kind_error(ctx, "\"baudOutShape\" must be \"half\" or \"low2\"");
        }
        d->baudout_low2 = strcmp(shape, "low2") == 0;
    }
    if (serial_optional_name(ctx, clock, "select", &d->clock_select) != 0 ||
        serial_optional_name(ctx, clock, "out", &d->clock_out) != 0 ||
        serial_optional_name(ctx, clock, "receive", &d->receive) != 0 ||
        serial_optional_name(ctx, clock, "baudOut", &d->baudout) != 0) {
        return -1;
    }
    kind_pop(ctx);
    return 0;
}

static int load_channels(KindContext *ctx, const cJSON *json, NsDevice *d) {
    const cJSON *list = kind_member(ctx, json, "channels", 1);
    const cJSON *item;
    unsigned n = 0;

    if (list == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) < 1 ||
        cJSON_GetArraySize(list) > SERIAL_MAX_CHANNELS) {
        return kind_error(ctx, "\"channels\" must be an array of 1 to %d channels",
                          SERIAL_MAX_CHANNELS);
    }
    cJSON_ArrayForEach(item, list) {
        unsigned k;

        kind_push(ctx, "channels[%u]", n);
        if (kind_check_keys(ctx, item, CHANNEL_KEYS, CHANNEL_EXTRA_KEYS) != 0) {
            return -1;
        }
        for (k = 0; k < CH_PIN_COUNT; k++) {
            if (serial_optional_name(ctx, item, CHANNEL_KEYS[k], &d->channel[n][k]) != 0) {
                return -1;
            }
        }
        if (d->channel[n][CH_TX] == NULL || d->channel[n][CH_RX] == NULL) {
            return kind_error(ctx, "a channel needs \"tx\" and \"rx\"");
        }
        if (serial_optional_name(ctx, item, "op", &d->op[n]) != 0 ||
            serial_optional_name(ctx, item, "mf", &d->mf[n]) != 0) {
            return -1;
        }
        if (load_name_list(ctx, item, "select", 0, d->chan_select[n], SERIAL_BUS_SELECTS,
                           &d->chan_select_count[n], 0) != 0 ||
            load_name_list(ctx, item, "selectHigh", 0, d->chan_select_high[n], SERIAL_BUS_SELECTS,
                           &d->chan_select_high_count[n], 0) != 0) {
            return -1;
        }
        if (d->chan_select_count[n] + d->chan_select_high_count[n] > 0) {
            d->per_channel_select = 1;
        }
        kind_pop(ctx);
        n++;
    }
    d->channel_count = n;
    if (d->per_channel_select) {
        for (n = 0; n < d->channel_count; n++) {
            if (d->chan_select_count[n] + d->chan_select_high_count[n] == 0) {
                return kind_error(ctx, "with per-channel selects every channel needs one");
            }
        }
    }
    return 0;
}

/* An optional string member that must be one of `values`; *out = its index,
   or `fallback` when absent. */
static int load_choice(KindContext *ctx, const cJSON *json, const char *key,
                       const char *const *values, int fallback, int *out) {
    const char *text;
    int i;

    *out = fallback;
    if (cJSON_GetObjectItemCaseSensitive(json, key) == NULL) {
        return 0;
    }
    if (kind_string(ctx, json, key, &text) != 0) {
        return -1;
    }
    for (i = 0; values[i] != NULL; i++) {
        if (strcmp(values[i], text) == 0) {
            *out = i;
            return 0;
        }
    }
    return kind_error(ctx, "unknown \"%s\" \"%s\"", key, text);
}

static int load_optional_bool(KindContext *ctx, const cJSON *json, const char *key, int *out) {
    *out = 0;
    if (cJSON_GetObjectItemCaseSensitive(json, key) == NULL) {
        return 0;
    }
    return kind_bool(ctx, json, key, out);
}

/* 4 whole numbers from 1 to max. */
static int load_levels(KindContext *ctx, const cJSON *json, const char *key, unsigned max,
                       unsigned out[4]) {
    const cJSON *list = cJSON_GetObjectItemCaseSensitive(json, key);
    const cJSON *item;
    unsigned i = 0;

    if (list == NULL) {
        return 0;
    }
    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) != 4) {
        return kind_error(ctx, "\"%s\" must be 4 numbers", key);
    }
    cJSON_ArrayForEach(item, list) {
        if (!cJSON_IsNumber(item) || item->valuedouble < 1 || item->valuedouble > max ||
            item->valuedouble != (double)(unsigned)item->valuedouble) {
            return kind_error(ctx, "\"%s\" values must be whole numbers from 1 to the FIFO size", key);
        }
        out[i++] = (unsigned)item->valuedouble;
    }
    return 0;
}

static int load_byte(KindContext *ctx, const cJSON *json, const char *key, int fallback, int *out) {
    uint64_t v;

    *out = fallback;
    if (cJSON_GetObjectItemCaseSensitive(json, key) == NULL) {
        return 0;
    }
    if (kind_uint(ctx, json, key, 255, &v) != 0) {
        return -1;
    }
    *out = (int)v;
    return 0;
}

/* [[off, on] x 4]: a per-trigger table of two levels. */
static int load_pairs(KindContext *ctx, const cJSON *json, const char *key, uint8_t out[4][2],
                      int *present) {
    const cJSON *list = cJSON_GetObjectItemCaseSensitive(json, key);
    const cJSON *row;
    unsigned i = 0;

    *present = 0;
    if (list == NULL) {
        return 0;
    }
    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) != 4) {
        return kind_error(ctx, "\"%s\" must be 4 [level, level] pairs", key);
    }
    cJSON_ArrayForEach(row, list) {
        const cJSON *a = cJSON_GetArrayItem(row, 0);
        const cJSON *b = cJSON_GetArrayItem(row, 1);

        if (!cJSON_IsArray(row) || cJSON_GetArraySize(row) != 2 || !cJSON_IsNumber(a) ||
            !cJSON_IsNumber(b) || a->valuedouble < 0 || a->valuedouble > 128 ||
            b->valuedouble < 0 || b->valuedouble > 128) {
            return kind_error(ctx, "\"%s\" must be 4 [level, level] pairs of 0 to 128", key);
        }
        out[i][0] = (uint8_t)a->valuedouble;
        out[i][1] = (uint8_t)b->valuedouble;
        i++;
    }
    *present = 1;
    return 0;
}

static int load_printer_port(KindContext *ctx, const cJSON *json, NsDevice *d) {
    static const char *const LPT_KEYS[] = {
        "style", "data", "select", "strobe", "autoFeed", "init", "selectIn", "ack", "busy",
        "paperEnd", "slct", "error", "irq", "inputEnable", "pemd", "enirq", NULL
    };
    static const char *const STYLES[] = {"ti451", "ti552", "com92c451", NULL};
    const cJSON *port = cJSON_GetObjectItemCaseSensitive(json, "printerPort");
    NsLptDevice *l;
    unsigned n;
    int style;

    if (port == NULL) {
        return 0;
    }
    l = kind_alloc(ctx, sizeof *l);
    if (l == NULL) {
        return -1;
    }
    kind_push(ctx, "printerPort");
    if (kind_check_keys(ctx, port, LPT_KEYS, NULL) != 0 ||
        load_choice(ctx, port, "style", STYLES, -1, &style) != 0) {
        return -1;
    }
    if (style < 0) {
        return kind_error(ctx, "missing \"style\"");
    }
    l->style = (NsLptStyle)style;
    if (load_name_list(ctx, port, "data", 1, l->data, 8, &n, 8) != 0 ||
        kind_string(ctx, port, "select", &l->select) != 0 ||
        kind_string(ctx, port, "strobe", &l->strobe) != 0 ||
        kind_string(ctx, port, "autoFeed", &l->auto_feed) != 0 ||
        kind_string(ctx, port, "init", &l->init) != 0 ||
        kind_string(ctx, port, "selectIn", &l->select_in) != 0 ||
        kind_string(ctx, port, "ack", &l->ack) != 0 ||
        kind_string(ctx, port, "busy", &l->busy) != 0 ||
        kind_string(ctx, port, "paperEnd", &l->paper_end) != 0 ||
        kind_string(ctx, port, "slct", &l->slct) != 0 ||
        kind_string(ctx, port, "error", &l->error) != 0 ||
        kind_string(ctx, port, "irq", &l->irq) != 0 ||
        serial_optional_name(ctx, port, "inputEnable", &l->input_enable) != 0 ||
        serial_optional_name(ctx, port, "pemd", &l->pemd) != 0 ||
        serial_optional_name(ctx, port, "enirq", &l->enirq) != 0) {
        return -1;
    }
    if ((l->style == LPT_TI451) != (l->input_enable != NULL) ||
        (l->style == LPT_TI552) != (l->pemd != NULL && l->enirq != NULL)) {
        return kind_error(ctx, "\"inputEnable\" is for ti451; \"pemd\" and \"enirq\" for ti552");
    }
    kind_pop(ctx);
    d->lpt = l;
    return 0;
}

/* "absentTies": {pin: 0 or 1} for pins a package leaves out and ties
   inside; "optionalPins": [pin] that a package may simply leave out. */
static int load_absent(KindContext *ctx, const cJSON *json, NsDevice *d) {
    const cJSON *ties = cJSON_GetObjectItemCaseSensitive(json, "absentTies");
    const cJSON *item;
    size_t n = 0;

    if (ties != NULL) {
        char **names;
        int *levels;

        if (!cJSON_IsObject(ties)) {
            return kind_error(ctx, "\"absentTies\" must be an object of pin: 0 or 1");
        }
        names = kind_alloc(ctx, ((size_t)cJSON_GetArraySize(ties) + 1) * sizeof *names);
        levels = kind_alloc(ctx, ((size_t)cJSON_GetArraySize(ties) + 1) * sizeof *levels);
        if (names == NULL || levels == NULL) {
            return -1;
        }
        cJSON_ArrayForEach(item, ties) {
            if (!cJSON_IsNumber(item) || (item->valuedouble != 0 && item->valuedouble != 1)) {
                return kind_error(ctx, "\"absentTies\" must be an object of pin: 0 or 1");
            }
            names[n] = kind_strdup(ctx, item->string);
            levels[n] = item->valuedouble == 1;
            n++;
        }
        d->absent.tie_names = (const char *const *)names;
        d->absent.tie_levels = levels;
        d->absent.tie_count = n;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "optionalPins") != NULL) {
        if (kind_names(ctx, json, "optionalPins", &d->absent.optional, &d->absent.optional_count) != 0) {
            return -1;
        }
    }
    return 0;
}

/* The plan-12-D Phase 2 keys. */
static int load_phase2(KindContext *ctx, const cJSON *json, NsDevice *d) {
    static const char *const EXTENDED[] = {"none", "afe", "750", "650", "752", "nsEfr",
                                           /* plan-12-I */
                                           "sc16is", "sc16is-sfr", "sc16is-dld", "xr1280", NULL};
    static const char *const AFRS[] = {"none", "ti752", "ti2552", "ns", NULL};
    static const char *const FLOWS[] = {"none", "afe", "tcr", "efrTable", NULL};
    static const char *const SWFLOWS[] = {"none", "nxpAnd", "tiAnd", NULL};
    static const char *const CLEARS[] = {"iir", "lsr", "msr", "iirOrMsr", NULL};
    static const char *const SLEEPS[] = {"none", "ier4", "ier4-all", NULL};
    static const char *const IRDAS[] = {"none", "mcr6", "afr1", NULL};
    static const char *const RS485S[] = {"none", "afr-dtr", NULL};
    static const char *const PRESCALERS[] = {"none", "mcr7", "mcr7-clksel", NULL};
    static const char *const LSR_ERRORS[] = {"sticky", "head", NULL};
    int choice;
    int value;
    int present;

    if (load_choice(ctx, json, "extended", EXTENDED, EXT_NONE, &choice) != 0) {
        return -1;
    }
    d->extended = (NsExtended)choice;
    if (load_choice(ctx, json, "afr", AFRS, AFR_NONE, &choice) != 0) {
        return -1;
    }
    d->afr = (NsAfr)choice;
    if (load_byte(ctx, json, "afrReset", 0, &value) != 0) {
        return -1;
    }
    d->afr_reset = (uint8_t)value;
    if (load_optional_bool(ctx, json, "dlf", &d->dlf) != 0 ||
        load_choice(ctx, json, "autoFlow", FLOWS, FLOW_NONE, &choice) != 0) {
        return -1;
    }
    d->auto_flow = (NsAutoFlow)choice;
    if (load_pairs(ctx, json, "rtsTable", d->rts_table, &present) != 0) {
        return -1;
    }
    if ((d->auto_flow == FLOW_TABLE) != present) {
        return kind_error(ctx, "\"rtsTable\" goes with \"autoFlow\": \"efrTable\"");
    }
    if (load_pairs(ctx, json, "xoffTable", d->xoff_table, &d->has_xoff_table) != 0 ||
        load_choice(ctx, json, "swFlow", SWFLOWS, SW_NONE, &choice) != 0) {
        return -1;
    }
    d->sw_flow = (NsSwFlow)choice;
    if (load_optional_bool(ctx, json, "specialChar", &d->special_char) != 0 ||
        load_optional_bool(ctx, json, "specialNeedsNoFlow", &d->special_needs_no_flow) != 0 ||
        load_choice(ctx, json, "specialClear", CLEARS, CLEAR_IIR, &choice) != 0) {
        return -1;
    }
    d->special_clear = (NsClear)choice;
    if (load_choice(ctx, json, "ctsRtsClear", CLEARS, CLEAR_IIR, &choice) != 0) {
        return -1;
    }
    d->ctsrts_clear = (NsClear)choice;
    if (load_choice(ctx, json, "sleep", SLEEPS, SLEEP_NONE, &choice) != 0) {
        return -1;
    }
    d->sleep = (NsSleep)choice;
    if (load_choice(ctx, json, "irda", IRDAS, IRDA_NONE, &choice) != 0) {
        return -1;
    }
    d->irda = (NsIrda)choice;
    if (load_choice(ctx, json, "rs485", RS485S, RS485_NONE, &choice) != 0) {
        return -1;
    }
    d->rs485 = (NsRs485)choice;
    if (load_choice(ctx, json, "prescaler", PRESCALERS, PRESCALE_NONE, &choice) != 0) {
        return -1;
    }
    d->prescaler = (NsPrescaler)choice;
    if ((d->prescaler == PRESCALE_MCR7_CLKSEL) != (d->clock_select != NULL)) {
        return kind_error(ctx, "\"mcr7-clksel\" needs \"clock\".\"select\" (and only it)");
    }
    if (load_choice(ctx, json, "lsrErrors", LSR_ERRORS, 0, &choice) != 0) {
        return -1;
    }
    d->lsr_head = choice == 1;
    if (load_byte(ctx, json, "mcrReset", -1, &d->mcr_reset) != 0 ||
        load_optional_bool(ctx, json, "tcrNeedsDlab0", &d->tcr_needs_dlab0) != 0 ||
        load_optional_bool(ctx, json, "dlabHidesUpper", &d->dlab_hides_upper) != 0) {
        return -1;
    }
    /* Power-up Xon1, Xon2, Xoff1, Xoff2 (absent: X). */
    d->xon_power[0] = d->xon_power[1] = d->xon_power[2] = d->xon_power[3] = -1;
    if (cJSON_GetObjectItemCaseSensitive(json, "xonReset") != NULL) {
        const cJSON *list = cJSON_GetObjectItemCaseSensitive(json, "xonReset");
        const cJSON *item;
        unsigned i = 0;

        if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) != 4) {
            return kind_error(ctx, "\"xonReset\" must be 4 bytes (Xon1, Xon2, Xoff1, Xoff2)");
        }
        cJSON_ArrayForEach(item, list) {
            if (!cJSON_IsNumber(item) || item->valuedouble < 0 || item->valuedouble > 255) {
                return kind_error(ctx, "\"xonReset\" must be 4 bytes (Xon1, Xon2, Xoff1, Xoff2)");
            }
            d->xon_power[i++] = (int)item->valuedouble;
        }
    }
    d->drev = -1;
    d->dvid = -1;
    if (cJSON_GetObjectItemCaseSensitive(json, "idRegs") != NULL) {
        static const char *const ID_KEYS[] = {"drev", "dvid", NULL};
        const cJSON *ids = cJSON_GetObjectItemCaseSensitive(json, "idRegs");

        kind_push(ctx, "idRegs");
        if (kind_check_keys(ctx, ids, ID_KEYS, NULL) != 0 ||
            load_byte(ctx, ids, "drev", -1, &d->drev) != 0 ||
            load_byte(ctx, ids, "dvid", -1, &d->dvid) != 0) {
            return -1;
        }
        kind_pop(ctx);
    }
    if (load_byte(ctx, json, "timeoutDelay", 8, &value) != 0) {
        return -1;
    }
    d->timeout_extra = (unsigned)value;
    if (load_byte(ctx, json, "divisorZero", 0, &value) != 0) {
        return -1;
    }
    d->divisor_zero = (unsigned)value;
    if (serial_optional_name(ctx, json, "tristate", &d->tri) != 0 ||
        load_printer_port(ctx, json, d) != 0) {
        return -1;
    }
    if (load_absent(ctx, json, d) != 0 ||
        load_byte(ctx, json, "sprPowerUp", -1, &d->spr_power_up) != 0 ||
        load_optional_bool(ctx, json, "lsrClearOnGood", &d->lsr_clear_on_good) != 0 ||
        load_byte(ctx, json, "breakMarkTicks", 0, &value) != 0) {
        return -1;
    }
    d->break_mark_ticks = (unsigned)value;
    if (cJSON_GetObjectItemCaseSensitive(json, "dmaMode1") != NULL) {
        int mode1;

        if (kind_bool(ctx, json, "dmaMode1", &mode1) != 0) {
            return -1;
        }
        d->no_dma_mode1 = !mode1;
    }
    if (d->lpt != NULL && d->channel_pin != NULL) {
        return kind_error(ctx, "a printer port needs per-channel or common selects");
    }
    return 0;
}

static void *ns_load_device(KindContext *ctx, const cJSON *json, unsigned *channels) {
    static const char *const INT_PINS[] = {
        "driven", "out2-gated", "intn", "intsel", "mcr5-open-source", NULL
    };
    static const char *const DMA_CLASSES[] = {"P", "N", "T", "X", "B", NULL};
    static const char *const DMA_PINS[] = {"perChannel", "sharedAnd", "sharedWireOr", NULL};
    static const char *const BROADCASTS[] = {"none", "multi-cs", "afr-conc", NULL};
    static const char *const TIMEOUTS[] = {"4ch", "4w12", NULL};
    static const char *const PRIORITIES[] = {"standard", "nxpEqual", NULL};
    static const char *const LSR7[] = {"noErrors", "lsrRead", NULL};
    static const char *const STOPS[] = {"first", "both", NULL};
    NsDevice *d = kind_alloc(ctx, sizeof *d);
    const cJSON *fifo;
    unsigned max_fifo;
    int choice;
    int value;
    int host;

    if (d == NULL) {
        return NULL;
    }
    host = load_bus(ctx, json, d);
    if (host < 0 || load_clock(ctx, json, d) != 0 || load_channels(ctx, json, d) != 0) {
        return NULL;
    }
    /* "fifo": 0, 16, 32, 64, 128, or "750" (16 or 64 by FCR[5]). */
    fifo = kind_member(ctx, json, "fifo", 1);
    if (fifo == NULL) {
        return NULL;
    }
    if (cJSON_IsString(fifo) && strcmp(fifo->valuestring, "750") == 0) {
        d->fifo = 16;
        d->fifo750 = 1;
    } else if (cJSON_IsNumber(fifo) && (fifo->valuedouble == 0 || fifo->valuedouble == 16 ||
                                         fifo->valuedouble == 32 || fifo->valuedouble == 64 ||
                                         fifo->valuedouble == 128)) {
        d->fifo = (unsigned)fifo->valuedouble;
    } else {
        kind_error(ctx, "\"fifo\" must be 0, 16, 32, 64, 128 or \"750\"");
        return NULL;
    }
    max_fifo = d->fifo750 ? 64 : d->fifo;
    if (load_levels(ctx, json, "rxTrigger", d->fifo, d->rx_trigger) != 0 ||
        load_levels(ctx, json, "rxTrigger64", 64, d->rx_trigger64) != 0 ||
        load_levels(ctx, json, "txTrigger", max_fifo, d->tx_trigger) != 0) {
        return NULL;
    }
    d->has_tx_trigger = d->tx_trigger[0] != 0;
    if (d->rx_trigger64[0] != 0 && !d->fifo750) {
        kind_error(ctx, "\"rxTrigger64\" needs \"fifo\": \"750\"");
        return NULL;
    }
    if (load_byte(ctx, json, "tlr", 0, &value) != 0) {
        return NULL;
    }
    if (value != 0 && value != 4 && value != 8) {
        kind_error(ctx, "\"tlr\" must be 4 or 8");
        return NULL;
    }
    d->tlr = (unsigned)value;

    if (load_choice(ctx, json, "intPin", INT_PINS, INT_DRIVEN, &choice) != 0) {
        return NULL;
    }
    d->int_pin = (NsIntPin)choice;
    if (serial_optional_name(ctx, json, "intControl", &d->int_control) != 0) {
        return NULL;
    }
    if ((d->int_pin == INT_INTN || d->int_pin == INT_INTSEL) != (d->int_control != NULL)) {
        kind_error(ctx, "\"intControl\" names the INTN/INTSEL pin, for \"intn\" and \"intsel\" only");
        return NULL;
    }
    if (load_choice(ctx, json, "dmaClass", DMA_CLASSES, 0, &choice) != 0) {
        return NULL;
    }
    d->dma_class = "PNTXB"[choice];
    if (load_choice(ctx, json, "dmaPins", DMA_PINS, DMA_PINS_NAMED, &choice) != 0) {
        return NULL;
    }
    d->dma_pins = (NsDmaPins)choice;
    if (cJSON_GetObjectItemCaseSensitive(json, "dma") != NULL) {
        static const char *const DMA_KEYS[] = {"txrdy", "rxrdy", NULL};
        const cJSON *dma = cJSON_GetObjectItemCaseSensitive(json, "dma");

        kind_push(ctx, "dma");
        if (kind_check_keys(ctx, dma, DMA_KEYS, NULL) != 0 ||
            serial_optional_name(ctx, dma, "txrdy", &d->dma_txrdy) != 0 ||
            serial_optional_name(ctx, dma, "rxrdy", &d->dma_rxrdy) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    if ((d->dma_pins != DMA_PINS_NAMED) != (d->dma_txrdy != NULL || d->dma_rxrdy != NULL)) {
        kind_error(ctx, "shared \"dmaPins\" and the \"dma\" pins go together");
        return NULL;
    }
    if (load_choice(ctx, json, "broadcastWrite", BROADCASTS, BROADCAST_NONE, &choice) != 0) {
        return NULL;
    }
    d->broadcast = (NsBroadcast)choice;
    if (cJSON_GetObjectItemCaseSensitive(json, "channelSelect") != NULL) {
        static const char *const CS_KEYS[] = {"pin", "aIsHigh", NULL};
        const cJSON *cs = cJSON_GetObjectItemCaseSensitive(json, "channelSelect");

        kind_push(ctx, "channelSelect");
        if (kind_check_keys(ctx, cs, CS_KEYS, NULL) != 0 ||
            kind_string(ctx, cs, "pin", &d->channel_pin) != 0 ||
            kind_bool(ctx, cs, "aIsHigh", &d->a_is_high) != 0) {
            return NULL;
        }
        kind_pop(ctx);
        if (d->channel_count != 2 || d->per_channel_select) {
            kind_error(ctx, "\"channelSelect\" is for 2 channels sharing one chip select");
            return NULL;
        }
    }
    if (d->channel_count > 1 && !d->per_channel_select && d->channel_pin == NULL && !host) {
        kind_error(ctx, "more than one channel needs per-channel selects or \"channelSelect\"");
        return NULL;
    }
    if (load_byte(ctx, json, "lcrReset", 0, &value) != 0) {
        return NULL;
    }
    d->lcr_reset = (uint8_t)value;
    if (load_byte(ctx, json, "sprReset", -1, &d->spr_reset) != 0 ||
        load_byte(ctx, json, "tcrReset", 0, &value) != 0) {
        return NULL;
    }
    d->tcr_reset = (uint8_t)value;
    if (load_choice(ctx, json, "timeout", TIMEOUTS, 0, &choice) != 0) {
        return NULL;
    }
    d->timeout_4w12 = choice == 1;
    if (load_choice(ctx, json, "iirPriority", PRIORITIES, IIR_STANDARD, &choice) != 0) {
        return NULL;
    }
    d->iir_priority = (NsIirPriority)choice;
    if (load_choice(ctx, json, "lsr7Clear", LSR7, 0, &choice) != 0) {
        return NULL;
    }
    d->lsr7_on_read = choice == 1;
    if (load_choice(ctx, json, "stopCheck", STOPS, 0, &choice) != 0) {
        return NULL;
    }
    d->stop_both = choice == 1;
    if (load_optional_bool(ctx, json, "statusReadDefer", &d->status_defer) != 0 ||
        load_optional_bool(ctx, json, "busSync", &d->bus_sync) != 0 ||
        load_optional_bool(ctx, json, "majorityVote", &d->majority) != 0) {
        return NULL;
    }
    if (load_phase2(ctx, json, d) != 0) {
        return NULL;
    }
    /* plan-12-I: the serial host port and the bridge keys. */
    if (host) {
        if (ns_bridge_load(ctx, json, cJSON_GetObjectItemCaseSensitive(json, "bus"), d) != 0) {
            return NULL;
        }
    } else {
        unsigned i;

        for (i = 0; BRIDGE_KEYS[i] != NULL; i++) {
            if (cJSON_GetObjectItemCaseSensitive(json, BRIDGE_KEYS[i]) != NULL) {
                kind_error(ctx, "\"%s\" needs an I2C/SPI \"bus\"", BRIDGE_KEYS[i]);
                return NULL;
            }
        }
        if (d->extended >= EXT_SC16IS) {
            kind_error(ctx, "the bridge \"extended\" styles need an I2C/SPI \"bus\"");
            return NULL;
        }
    }
    *channels = d->channel_count;
    return d;
}

/* The device being bound: its "absentTies" / "optionalPins" (binding is one
   device at a time). */
static const SerialAbsent *binding_absent;

static int bind_pin(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *key,
                    const char *name, size_t *out) {
    return serial_bind_pin_absent(ctx, pins, pin_count, key, name, binding_absent, out);
}

static void *ns_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t pin_count) {
    const NsDevice *d = raw;
    NsPins *p = kind_alloc(ctx, sizeof *p);
    char key[32];
    unsigned i;
    unsigned k;

    if (p == NULL) {
        return NULL;
    }
    binding_absent = &d->absent;
    for (i = 0; i < SERIAL_BUS_DATA; i++) {
        snprintf(key, sizeof key, "bus.data[%u]", i);
        if (bind_pin(ctx, pins, pin_count, key, d->data[i], &p->bus.data[i]) != 0) {
            return NULL;
        }
    }
    for (i = 0; i < d->address_count; i++) {
        snprintf(key, sizeof key, "bus.address[%u]", i);
        if (bind_pin(ctx, pins, pin_count, key, d->address[i], &p->bus.address[i]) != 0) {
            return NULL;
        }
    }
    p->bus.address_count = d->address_count;
    for (i = 0; i < d->select_count; i++) {
        if (bind_pin(ctx, pins, pin_count, "bus.select", d->select[i], &p->bus.select[i]) != 0) {
            return NULL;
        }
    }
    p->bus.select_count = d->select_count;
    for (i = 0; i < d->select_high_count; i++) {
        if (bind_pin(ctx, pins, pin_count, "bus.selectHigh", d->select_high[i],
                            &p->bus.select_high[i]) != 0) {
            return NULL;
        }
    }
    p->bus.select_high_count = d->select_high_count;
    if (bind_pin(ctx, pins, pin_count, "bus.read", d->read, &p->bus.read) != 0 ||
        bind_pin(ctx, pins, pin_count, "bus.readHigh", d->read_high, &p->bus.read_high) != 0 ||
        bind_pin(ctx, pins, pin_count, "bus.write", d->write, &p->bus.write) != 0 ||
        bind_pin(ctx, pins, pin_count, "bus.writeHigh", d->write_high, &p->bus.write_high) != 0 ||
        bind_pin(ctx, pins, pin_count, "bus.addressStrobe", d->address_strobe,
                        &p->bus.address_strobe) != 0 ||
        bind_pin(ctx, pins, pin_count, "bus.driverDisable", d->driver_disable,
                        &p->bus.driver_disable) != 0 ||
        bind_pin(ctx, pins, pin_count, "bus.selectOut", d->select_out, &p->bus.select_out) != 0 ||
        bind_pin(ctx, pins, pin_count, "bus.reset", d->reset, &p->reset) != 0 ||
        bind_pin(ctx, pins, pin_count, "clock.in", d->clock_in, &p->clock_in) != 0 ||
        bind_pin(ctx, pins, pin_count, "clock.out", d->clock_out, &p->clock_out) != 0 ||
        bind_pin(ctx, pins, pin_count, "clock.receive", d->receive, &p->receive) != 0 ||
        bind_pin(ctx, pins, pin_count, "clock.baudOut", d->baudout, &p->baudout) != 0) {
        return NULL;
    }
    for (i = 0; i < SERIAL_MAX_CHANNELS; i++) {
        for (k = 0; k < CH_PIN_COUNT; k++) {
            p->channel[i][k] = SERIAL_NO_PIN;
        }
    }
    for (i = 0; i < d->channel_count; i++) {
        for (k = 0; k < CH_PIN_COUNT; k++) {
            snprintf(key, sizeof key, "channels[%u].%s", i, CHANNEL_KEYS[k]);
            if (bind_pin(ctx, pins, pin_count, key, d->channel[i][k], &p->channel[i][k]) != 0) {
                return NULL;
            }
        }
        for (k = 0; k < d->chan_select_count[i]; k++) {
            snprintf(key, sizeof key, "channels[%u].select", i);
            if (bind_pin(ctx, pins, pin_count, key, d->chan_select[i][k],
                                &p->chan_select[i][k]) != 0) {
                return NULL;
            }
        }
        for (k = 0; k < d->chan_select_high_count[i]; k++) {
            snprintf(key, sizeof key, "channels[%u].selectHigh", i);
            if (bind_pin(ctx, pins, pin_count, key, d->chan_select_high[i][k],
                                &p->chan_select_high[i][k]) != 0) {
                return NULL;
            }
        }
    }
    if (bind_pin(ctx, pins, pin_count, "channelSelect.pin", d->channel_pin,
                        &p->channel_pin) != 0 ||
        bind_pin(ctx, pins, pin_count, "dma.txrdy", d->dma_txrdy, &p->dma_txrdy) != 0 ||
        bind_pin(ctx, pins, pin_count, "dma.rxrdy", d->dma_rxrdy, &p->dma_rxrdy) != 0 ||
        bind_pin(ctx, pins, pin_count, "intControl", d->int_control, &p->int_control) != 0 ||
        bind_pin(ctx, pins, pin_count, "clock.select", d->clock_select, &p->clock_select) != 0 ||
        bind_pin(ctx, pins, pin_count, "tristate", d->tri, &p->tri) != 0) {
        return NULL;
    }
    for (i = 0; i < SERIAL_MAX_CHANNELS; i++) {
        p->op[i] = SERIAL_NO_PIN;
        p->mf[i] = SERIAL_NO_PIN;
    }
    for (i = 0; i < d->channel_count; i++) {
        snprintf(key, sizeof key, "channels[%u].op", i);
        if (bind_pin(ctx, pins, pin_count, key, d->op[i], &p->op[i]) != 0) {
            return NULL;
        }
        snprintf(key, sizeof key, "channels[%u].mf", i);
        if (bind_pin(ctx, pins, pin_count, key, d->mf[i], &p->mf[i]) != 0) {
            return NULL;
        }
    }
    if (d->lpt != NULL) {
        NsLptPins *lp = kind_alloc(ctx, sizeof *lp);
        const NsLptDevice *ld = d->lpt;
        const char *const names[] = {ld->select, ld->strobe, ld->auto_feed, ld->init, ld->select_in,
                                     ld->ack, ld->busy, ld->paper_end, ld->slct, ld->error, ld->irq,
                                     ld->input_enable, ld->pemd, ld->enirq};
        size_t *const slots[] = {&lp->select, &lp->strobe, &lp->auto_feed, &lp->init, &lp->select_in,
                                 &lp->ack, &lp->busy, &lp->paper_end, &lp->slct, &lp->error, &lp->irq,
                                 &lp->input_enable, &lp->pemd, &lp->enirq};

        if (lp == NULL) {
            return NULL;
        }
        for (i = 0; i < 8; i++) {
            snprintf(key, sizeof key, "printerPort.data[%u]", i);
            if (bind_pin(ctx, pins, pin_count, key, ld->data[i], &lp->data[i]) != 0) {
                return NULL;
            }
        }
        for (i = 0; i < sizeof names / sizeof names[0]; i++) {
            if (bind_pin(ctx, pins, pin_count, "printerPort", names[i], slots[i]) != 0) {
                return NULL;
            }
        }
        p->lpt = lp;
    }
    if (d->bridge != NULL && ns_bridge_bind(ctx, d, pins, pin_count, p) != 0) {
        return NULL;   /* plan-12-I */
    }
    return p;
}

static int ns_open_drain(const void *data, const void *pins, const void *options, size_t pin) {
    const NsDevice *d = data;

    (void)options;
    return d->bridge != NULL && ns_bridge_open_drain(d, pins, pin);   /* plan-12-I */
}

const SerialModel NS16550_MODEL = {
    "ns16550", KEYS, TIMING, NULL,
    ns_load_device, ns_bind, NULL, ns_open_drain,
    ns_create, ns_destroy, ns_start, ns_pin_changed, ns_timer,
    ns_channels, ns_line, ns_async
};
