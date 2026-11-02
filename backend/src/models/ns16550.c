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
    "tcso", NULL
};
static const char *const KEYS[] = {
    "bus", "clock", "channels", "fifo", "rxTrigger", "intPin", "dmaClass", "timeout",
    /* plan-12-D */
    "channelSelect", "broadcastWrite", "rxTrigger64", "txTrigger", "tlr", "dmaPins", "dma",
    "intControl", "lcrReset", "sprReset", "tcrReset", "iirPriority", "lsr7Clear", "stopCheck",
    "statusReadDefer", "busSync", "majorityVote",
    NULL
};
static const char *const CHANNEL_KEYS[] = {
    "tx", "rx", "rts", "dtr", "cts", "dsr", "dcd", "ri", "out1", "out2", "int", "txrdy", "rxrdy",
    NULL
};
/* Keys of a channel object besides its pins. */
static const char *const CHANNEL_EXTRA_KEYS[] = {"select", "selectHigh", NULL};


const ChannelPin MODEM_PIN[4] = {CH_CTS, CH_DSR, CH_RI, CH_DCD};

static void drive(NsState *s, size_t pin, LogicValue value, SimTime delay) {
    if (pin == SERIAL_NO_PIN || s->out[pin] == value) {
        return;
    }
    s->out[pin] = value;
    sim_schedule_inertial_drive(s->sim, s->instance, pin, value, DRIVE_STRONG, delay);
}

/* BAUDOUT: a clock, so a plain propagation delay (an inertial drive would
   swallow pulses shorter than tbld). */
static void drive_baudout(NsState *s, LogicValue value, SimTime delay) {
    size_t pin = s->pins->baudout;

    if (pin == SERIAL_NO_PIN || s->out[pin] == value) {
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
    c->format = f;
}

static void load_divisor(NsChannel *c) {
    serial_baud_load(&c->baud, (uint32_t)c->dll.v | ((uint32_t)c->dlm.v << 8),
                     c->dll.k == 0xFF && c->dlm.k == 0xFF);
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

static void clear_rx(NsChannel *c) {
    c->rx_head = 0;
    c->rx_count = 0;
    c->timeout = 0;
    c->idle_ticks = 0;
    c->rxrdy1 = 0;
}

static void set_thre(NsChannel *c) {
    c->thre = 1;
    c->thre_wait = 0;
    c->thre_int = 1;
    c->two_seen = 0;
    c->immediate = 0;
}

static void clear_tx(NsChannel *c) {
    c->tx_head = 0;
    c->tx_count = 0;
    if (!c->thre) {
        set_thre(c);
    }
}

/* ---- Interrupts ------------------------------------------------------------------------ */

/* Each source: pending AND enabled, as 0 / 1 / X. */
typedef struct {
    LogicValue ls, rda, timeout, thre, modem;
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

    if (c->fifo_on) {
        rda = bool3(c->rx_count >= rx_trigger_level(c));
    } else {
        rda = bool3(c->rx_count > 0);
    }
    r.ls = and3(any_bits(c->lsr_err, c->lsr_err_k, 0x1E), bit_of(c->ier, 2));
    r.rda = and3(rda, bit_of(c->ier, 0));
    r.timeout = and3(bool3(c->fifo_on && c->timeout), bit_of(c->ier, 0));
    r.thre = and3(bool3(c->thre_int), bit_of(c->ier, 1));
    r.modem = and3(any_bits(c->delta, c->delta_k, 0x0F), bit_of(c->ier, 3));
    return r;
}

/* IIR: the highest pending source. Unknown when an X decides it. */
static void iir_of(const NsChannel *c, uint8_t *v, uint8_t *k) {
    Sources s = sources(c);
    /* Standard: RX data (trigger reached) above the timeout; nxpEqual: one
       level, shown as the timeout while one is pending. */
    int timeout_first = c->owner->dev->iir_priority == IIR_NXP_EQUAL;
    const LogicValue order[5] = {s.ls, timeout_first ? s.timeout : s.rda,
                                 timeout_first ? s.rda : s.timeout, s.thre, s.modem};
    const uint8_t ID[5] = {0x06, timeout_first ? 0x0C : 0x04, timeout_first ? 0x04 : 0x0C,
                           0x02, 0x00};
    unsigned i;

    *v = 0x01;
    *k = 0xFF;
    for (i = 0; i < 5; i++) {
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
}

static LogicValue int_level(const NsChannel *c) {
    Sources s = sources(c);

    return or3(or3(or3(s.ls, s.rda), or3(s.timeout, s.thre)), s.modem);
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
            unsigned level = c->owner->dev->has_tx_trigger ? c->owner->dev->tx_trigger[0] : 1;

            return bool3(free_slots < level);
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

static void update_channel_outputs(NsState *s, NsChannel *c, Cause cause) {
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

    if (s->unknown) {
        for (i = 0; i < CH_PIN_COUNT; i++) {
            if (i != CH_RX && i != CH_CTS && i != CH_DSR && i != CH_DCD && i != CH_RI) {
                drive(s, pin[i], LOGIC_X, 0);
            }
        }
        return;
    }
    if (s->in_reset) {
        drive(s, pin[CH_TX], LOGIC_1, 0);
        drive(s, pin[CH_RTS], LOGIC_1, 0);
        drive(s, pin[CH_DTR], LOGIC_1, 0);
        drive(s, pin[CH_OUT1], LOGIC_1, 0);
        drive(s, pin[CH_OUT2], LOGIC_1, 0);
        drive(s, pin[CH_INT], int_pin_level(s, c, LOGIC_0), 0);
        /* decisions.md (TL16C554, P3): RXRDY and TXRDY inactive during reset. */
        drive(s, pin[CH_RXRDY], LOGIC_1, 0);
        drive(s, pin[CH_TXRDY], LOGIC_1, 0);
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
    if (loop == LOGIC_1) {
        sout = LOGIC_1;
    } else if (loop == LOGIC_X && sout != LOGIC_1) {
        sout = LOGIC_X;
    }
    drive(s, pin[CH_TX], sout, 0);

    /* Modem outputs: MCR bits inverted; forced high (inactive) in loopback. */
    {
        static const ChannelPin OUT_PIN[4] = {CH_DTR, CH_RTS, CH_OUT1, CH_OUT2};

        for (i = 0; i < 4; i++) {
            LogicValue level = or3(not3(bit_of(c->mcr, i)), loop);

            drive(s, pin[OUT_PIN[i]], level, modem_delay);
        }
    }

    /* INT: active high; driven, gated or open per "intPin". */
    intr = int_pin_level(s, c, int_level(c));
    drive(s, pin[CH_INT], intr, int_delay);

    /* RXRDY / TXRDY (active low). */
    rxrdy = rxrdy_level(c, mode1);
    txrdy = txrdy_level(c, mode1);
    drive(s, pin[CH_RXRDY], rxrdy, rxrdy_delay);
    drive(s, pin[CH_TXRDY], txrdy, txrdy_delay);
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
    drive(s, s->pins->dma_rxrdy, rx, rx_delay);
    drive(s, s->pins->dma_txrdy, tx, tx_delay);
}

static void update_outputs(NsState *s, Cause cause) {
    unsigned i;

    for (i = 0; i < s->dev->channel_count; i++) {
        update_channel_outputs(s, &s->ch[i], cause);
    }
    update_shared_dma(s, cause);
    serial_bus_refresh(&s->bus, s->sim, s->instance, &s->events);
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
static void set_modem(NsChannel *c, const LogicValue now[4]) {
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
        /* Absent pins read X (decisions.md); present ones are active low. */
        LogicValue from_pin = pin == SERIAL_NO_PIN ? LOGIC_X
                                                   : not3(serial_level(s->sim, s->instance, pin, LOGIC_X));
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

static void reset_channel(NsState *s, NsChannel *c) {
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
    clear_rx(c);
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
    update_format(c);
    if (s->sim != NULL) {
        modem_now(s, c, now);
        memcpy(c->msr, now, sizeof now);
    }
}

static void reset_all(NsState *s) {
    unsigned i;

    for (i = 0; i < s->dev->channel_count; i++) {
        reset_channel(s, &s->ch[i]);
    }
}

/* ---- Receiving ------------------------------------------------------------------------------ */

static void deliver(NsChannel *c, const SerialRxChar *rc) {
    unsigned d = depth(c);

    c->idle_ticks = 0;
    c->timeout = 0;
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
            deliver(c, &c->pending[i].c);
            memmove(&c->pending[i], &c->pending[i + 1],
                    (c->pending_count - i - 1) * sizeof c->pending[0]);
            c->pending_count--;
            changed = 1;
        } else {
            i++;
        }
    }
    pin_level = serial_level(s->sim, s->instance, s->pins->channel[c->index][CH_RX], LOGIC_X);
    level = serial_line_rx_input(&c->line, s->sim, s->instance, pin_level, &c->format);
    if (loopback(c)) {
        level = c->tsr;   /* the transmitter's output, before break */
    }
    serial_rx_tick(&c->line.rx, level, &c->format, &events);

    /* Timeout: 4 character times (from RCLK) plus 8 RCLKs with data waiting;
       "4w12": 4 characters plus 12 bit times. */
    if (c->fifo_on && c->rx_count > 0 && !c->timeout) {
        uint64_t limit = 4ull * serial_format_char_bits(&c->format) * c->format.factor + 8;

        if (s->dev->timeout_4w12) {
            limit = (4ull * serial_format_char_bits(&c->format) + 12) * c->format.factor;
        }

        if (++c->idle_ticks >= limit) {
            c->timeout = 1;
            changed = 1;
        }
    }
    if (changed) {
        update_dma_latches(c);
        update_channel_outputs(s, c, CAUSE_SERIAL);
        serial_bus_refresh(&s->bus, s->sim, s->instance, &s->events);
    }
}

/* ---- Transmitting -------------------------------------------------------------------------- */

static int tx_fetch(void *context, uint16_t *data, uint16_t *known) {
    NsChannel *c = context;

    if (c->tx_count == 0) {
        return 0;
    }
    *data = c->tx_data[c->tx_head];
    *known = c->tx_known[c->tx_head];
    c->tx_head = (c->tx_head + 1) % NS_MAX_FIFO;
    c->tx_count--;
    if (c->tx_count == 0) {
        /* PC16550D p18: in FIFO mode the empty indication is delayed one
           character (to the last stop bit) unless two bytes were in the
           FIFO at once since the last THRE, or FCR[0] just changed. */
        if (c->fifo_on && !c->two_seen && !c->immediate) {
            c->thre_wait = 1;
        } else {
            set_thre(c);
        }
    }
    return 1;
}

static void tx_last_stop(void *context) {
    NsChannel *c = context;

    if (c->thre_wait) {
        set_thre(c);
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
    update_dma_latches(c);
    update_channel_outputs(s, c, CAUSE_SERIAL);
    serial_bus_refresh(&s->bus, s->sim, s->instance, &s->events);
}

/* ---- Registers -------------------------------------------------------------------------------- */

static void lsr_of(const NsChannel *c, uint8_t *v, uint8_t *k) {
    int temt = c->thre && c->tx_count == 0 && !serial_tx_busy(&c->line.tx);

    *v = (uint8_t)((c->rx_count > 0 ? 0x01 : 0) | (c->lsr_err & 0x1E) | (c->thre ? 0x20 : 0) |
                   (temt ? 0x40 : 0) | (c->fifo_on && c->lsr7 ? 0x80 : 0));
    *k = (uint8_t)(0xE1 | (c->lsr_err_k & 0x1E));
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
                *v = c->ier.v & 0x0F; *k = (uint8_t)(c->ier.k | 0xF0);
            }
            break;
        case 2:
            iir_of(c, v, k);
            c->iir_seen = *v;
            c->iir_seen_k = *k;
            break;
        case 3: *v = c->lcr.v; *k = c->lcr.k; break;
        case 4: *v = c->mcr.v & 0x1F; *k = (uint8_t)(c->mcr.k | 0xE0); break;
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
                if (c->rx_count > 0 && c->fifo_on) {
                    show_errors(c, &c->rx[c->rx_head]);
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
            }
            return CAUSE_IIR_READ;
        case 5: {
            unsigned i;

            /* "statusReadDefer": bits that set during the read stay set. */
            if (c->owner->dev->status_defer) {
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
    update_dma_latches(c);
}

static void write_fcr(NsChannel *c, uint8_t data, uint8_t known) {
    int on = data & 1u;

    if (c->owner->dev->fifo == 0) {
        return;   /* 16450: no FCR */
    }
    if (!(known & 1u)) {
        /* FIFO enable unknown: the FIFOs are cleared and the mode is X. */
        clear_rx(c);
        clear_tx(c);
        c->fifo_on = 0;
        c->fifo_x = 1;
        return;
    }
    c->fifo_x = 0;
    if (on != c->fifo_on) {
        clear_rx(c);
        clear_tx(c);
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
            clear_rx(c);
            clear_tx(c);
            c->fifo64 = mode64;
        }
    }
    if (data & 2u) {
        clear_rx(c);
    }
    if (data & 4u) {
        clear_tx(c);
    }
    c->dma_mode = (data >> 3) & 1u;
    c->trigger = (data >> 6) & 3u;
    update_dma_latches(c);
}

static void write_mcr(NsState *s, NsChannel *c, uint8_t data, uint8_t known) {
    LogicValue now[4];

    c->mcr = (Reg){(uint8_t)(data & 0x1F), (uint8_t)(known | 0xE0)};
    modem_now(s, c, now);
    set_modem(c, now);
}

static Cause reg_write(NsState *s, NsChannel *c, unsigned address, uint8_t data, uint8_t known) {
    LogicValue dlab = bit_of(c->lcr, 7);

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

                c->ier = (Reg){(uint8_t)(data & 0x0F), (uint8_t)(known | 0xF0)};
                /* Enabling ETBEI while THRE is 1 raises the THRE interrupt. */
                if (!was && (c->ier.v & 2u) && c->thre) {
                    c->thre_int = 1;
                }
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

    if (s->unknown || unit >= s->dev->channel_count) {
        *data = 0;
        *known = 0;
        return;
    }
    reg_peek(&s->ch[unit], address, data, known);
}

static void bus_read_done(void *context, unsigned unit, unsigned address) {
    NsState *s = context;
    Cause cause;

    if (s->unknown || unit >= s->dev->channel_count) {
        return;
    }
    cause = reg_read_done(&s->ch[unit], address);
    update_channel_outputs(s, &s->ch[unit], cause);
}

static void bus_write(void *context, unsigned unit, unsigned address, uint8_t data, uint8_t known) {
    NsState *s = context;
    Cause cause;

    if (s->unknown || unit >= s->dev->channel_count) {
        return;
    }
    cause = reg_write(s, &s->ch[unit], address, data, known);
    update_channel_outputs(s, &s->ch[unit], cause);
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
        int tick = serial_baud_edge(&c->baud, rising, level);

        if (i == 0) {
            drive_baudout(s, s->unknown ? LOGIC_X : c->baud.baudout, t_of(s, T_BLD));
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
    set_modem(c, now);
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
    serial_bus_start(&s->bus, sim, instance, &s->events);
    apply_reset(s);
}

static int is_select_pin(const NsState *s, size_t pin) {
    unsigned i;
    unsigned k;

    if (pin == SERIAL_NO_PIN) {
        return 0;
    }
    if (pin == s->pins->channel_pin) {
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
        } else {
            serial_bus_update(&s->bus, sim, instance, &s->events);
        }
    }
    if (pin == s->pins->int_control) {
        update_outputs(s, CAUSE_SERIAL);
    }
    for (i = 0; i < s->dev->channel_count; i++) {
        for (j = 0; j < 4; j++) {
            if (pin == s->pins->channel[i][MODEM_PIN[j]]) {
                modem_changed(s, &s->ch[i]);
            }
        }
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
    return d->channel_pin != NULL || d->per_channel_select;
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
        c->scr = (Reg){0, 0};
        c->msr[0] = c->msr[1] = c->msr[2] = c->msr[3] = LOGIC_X;
        reset_channel(s, c);
        load_divisor(c);
    }
    s->clock_level = LOGIC_X;
    s->receive_level = LOGIC_X;
    return s;
}

static void ns_destroy(void *raw) {
    NsState *s = raw;
    unsigned i;

    for (i = 0; i < s->dev->channel_count; i++) {
        serial_line_free(&s->ch[i].line);
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
    if (kind_check_keys(ctx, bus, BUS_KEYS, NULL) != 0 || kind_string(ctx, bus, "style", &style) != 0) {
        return -1;
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
    static const char *const CLOCK_KEYS[] = {"in", "out", "receive", "baudOut", "single", NULL};
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
    if (serial_optional_name(ctx, clock, "out", &d->clock_out) != 0 ||
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

    if (d == NULL || load_bus(ctx, json, d) != 0 || load_clock(ctx, json, d) != 0 ||
        load_channels(ctx, json, d) != 0) {
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
    if (d->channel_count > 1 && !d->per_channel_select && d->channel_pin == NULL) {
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
    *channels = d->channel_count;
    return d;
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
    for (i = 0; i < SERIAL_BUS_DATA; i++) {
        snprintf(key, sizeof key, "bus.data[%u]", i);
        if (serial_bind_pin(ctx, pins, pin_count, key, d->data[i], &p->bus.data[i]) != 0) {
            return NULL;
        }
    }
    for (i = 0; i < d->address_count; i++) {
        snprintf(key, sizeof key, "bus.address[%u]", i);
        if (serial_bind_pin(ctx, pins, pin_count, key, d->address[i], &p->bus.address[i]) != 0) {
            return NULL;
        }
    }
    p->bus.address_count = d->address_count;
    for (i = 0; i < d->select_count; i++) {
        if (serial_bind_pin(ctx, pins, pin_count, "bus.select", d->select[i], &p->bus.select[i]) != 0) {
            return NULL;
        }
    }
    p->bus.select_count = d->select_count;
    for (i = 0; i < d->select_high_count; i++) {
        if (serial_bind_pin(ctx, pins, pin_count, "bus.selectHigh", d->select_high[i],
                            &p->bus.select_high[i]) != 0) {
            return NULL;
        }
    }
    p->bus.select_high_count = d->select_high_count;
    if (serial_bind_pin(ctx, pins, pin_count, "bus.read", d->read, &p->bus.read) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.readHigh", d->read_high, &p->bus.read_high) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.write", d->write, &p->bus.write) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.writeHigh", d->write_high, &p->bus.write_high) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.addressStrobe", d->address_strobe,
                        &p->bus.address_strobe) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.driverDisable", d->driver_disable,
                        &p->bus.driver_disable) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.selectOut", d->select_out, &p->bus.select_out) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.reset", d->reset, &p->reset) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "clock.in", d->clock_in, &p->clock_in) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "clock.out", d->clock_out, &p->clock_out) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "clock.receive", d->receive, &p->receive) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "clock.baudOut", d->baudout, &p->baudout) != 0) {
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
            if (serial_bind_pin(ctx, pins, pin_count, key, d->channel[i][k], &p->channel[i][k]) != 0) {
                return NULL;
            }
        }
        for (k = 0; k < d->chan_select_count[i]; k++) {
            snprintf(key, sizeof key, "channels[%u].select", i);
            if (serial_bind_pin(ctx, pins, pin_count, key, d->chan_select[i][k],
                                &p->chan_select[i][k]) != 0) {
                return NULL;
            }
        }
        for (k = 0; k < d->chan_select_high_count[i]; k++) {
            snprintf(key, sizeof key, "channels[%u].selectHigh", i);
            if (serial_bind_pin(ctx, pins, pin_count, key, d->chan_select_high[i][k],
                                &p->chan_select_high[i][k]) != 0) {
                return NULL;
            }
        }
    }
    if (serial_bind_pin(ctx, pins, pin_count, "channelSelect.pin", d->channel_pin,
                        &p->channel_pin) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "dma.txrdy", d->dma_txrdy, &p->dma_txrdy) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "dma.rxrdy", d->dma_rxrdy, &p->dma_rxrdy) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "intControl", d->int_control, &p->int_control) != 0) {
        return NULL;
    }
    return p;
}

const SerialModel NS16550_MODEL = {
    "ns16550", KEYS, TIMING, NULL,
    ns_load_device, ns_bind, NULL, NULL,
    ns_create, ns_destroy, ns_start, ns_pin_changed, NULL,
    ns_channels, ns_line, ns_async
};
