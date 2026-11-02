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

#include "serial.h"

enum { NS_MAX_FIFO = 128 };

enum {
    T_RVD, T_HZ, T_RDD, T_BLD, T_RINT, T_HR, T_MDO, T_SIM, T_RIM, T_WXI, T_WRR, T_IR
};
static const char *const TIMING[] = {
    "trvd", "thz", "trdd", "tbld", "trint", "thr", "tmdo", "tsim", "trim", "twxi", "twrr", "tir",
    NULL
};

static const char *const KEYS[] = {
    "bus", "clock", "channels", "fifo", "rxTrigger", "intPin", "dmaClass", "timeout", NULL
};

typedef enum {
    CH_TX, CH_RX, CH_RTS, CH_DTR, CH_CTS, CH_DSR, CH_DCD, CH_RI, CH_OUT1, CH_OUT2, CH_INT,
    CH_TXRDY, CH_RXRDY, CH_PIN_COUNT
} ChannelPin;
static const char *const CHANNEL_KEYS[] = {
    "tx", "rx", "rts", "dtr", "cts", "dsr", "dcd", "ri", "out1", "out2", "int", "txrdy", "rxrdy",
    NULL
};

/* MSR status bits 4..7, in order: CTS, DSR, RI, DCD. */
enum { M_CTS, M_DSR, M_RI, M_DCD };
static const ChannelPin MODEM_PIN[4] = {CH_CTS, CH_DSR, CH_RI, CH_DCD};

/* ---- Device data ---------------------------------------------------------------- */

typedef struct {
    const char *data[SERIAL_BUS_DATA];
    const char *address[SERIAL_BUS_ADDRESS];
    unsigned address_count;
    const char *read, *read_high, *write, *write_high;
    const char *select[SERIAL_BUS_SELECTS];
    unsigned select_count;
    const char *select_high[SERIAL_BUS_SELECTS];
    unsigned select_high_count;
    const char *address_strobe, *driver_disable, *select_out, *reset;
    int reset_active_high;
    const char *clock_in, *clock_out, *receive, *baudout;
    const char *channel[SERIAL_MAX_CHANNELS][CH_PIN_COUNT];
    unsigned channel_count;
    unsigned fifo;                 /* 0 = 16450 */
    unsigned rx_trigger[4];
} NsDevice;

typedef struct {
    SerialBusPins bus;
    size_t reset, clock_in, clock_out, receive, baudout;
    size_t channel[SERIAL_MAX_CHANNELS][CH_PIN_COUNT];
} NsPins;

/* ---- State ------------------------------------------------------------------------ */

typedef struct {
    uint8_t v;
    uint8_t k;   /* known bits */
} Reg;

typedef struct NsState NsState;

typedef struct {
    NsState *owner;
    unsigned index;
    SerialLine line;
    SerialBaud baud;
    SerialFormat format;
    Reg ier, lcr, mcr, scr, dll, dlm, rbr;
    int fifo_on;
    int fifo_x;                    /* FCR[0] was written X */
    int dma_mode;
    unsigned trigger;              /* FCR[7:6] */
    /* Receive FIFO (depth 1 in 16450 mode). */
    SerialRxChar rx[NS_MAX_FIFO];
    unsigned rx_head;
    unsigned rx_count;
    struct { SerialRxChar c; unsigned ticks; } pending[8];
    unsigned pending_count;
    uint8_t lsr_err;               /* LSR[4:1], sticky until an LSR read */
    uint8_t lsr_err_k;
    int lsr7;
    /* Transmit FIFO (depth 1 in 16450 mode). */
    uint8_t tx_data[NS_MAX_FIFO];
    uint8_t tx_known[NS_MAX_FIFO];
    unsigned tx_head;
    unsigned tx_count;
    int thre;                      /* LSR[5] as indicated */
    int thre_wait;                 /* the indication waits for the last stop bit */
    int thre_int;                  /* the THRE interrupt source (latched) */
    int two_seen;                  /* 2+ bytes in the TX FIFO since the last THRE */
    int immediate;                 /* the next THRE indication is immediate (FCR[0] changed) */
    LogicValue tsr;                /* the transmitter's output, before break and loopback */
    /* Modem status bits 4..7 (1 = active) and the deltas. */
    LogicValue msr[4];
    uint8_t delta;
    uint8_t delta_k;
    /* Timeout. */
    uint64_t idle_ticks;
    int timeout;
    /* DMA mode 1 latches. */
    int rxrdy1;
    int txrdy1;
    uint8_t iir_seen;              /* the IIR a read in progress reported */
    uint8_t iir_seen_k;
} NsChannel;

struct NsState {
    SerialBase base;
    const Part *part;
    const NsDevice *dev;
    const NsPins *pins;
    const SerialGrade *grade;
    SerialBus bus;
    SerialBusEvents events;
    Sim *sim;
    SimInstance *instance;
    NsChannel ch[SERIAL_MAX_CHANNELS];
    int in_reset;
    int unknown;                   /* MR was X: everything unknown until a clean reset */
    LogicValue clock_level;
    LogicValue receive_level;
    LogicValue *out;               /* per pin: the level last set to drive */
    size_t pin_count;
};

/* What changed, for the output delays. */
typedef enum {
    CAUSE_SERIAL,
    CAUSE_RBR_READ,
    CAUSE_LSR_READ,
    CAUSE_IIR_READ,
    CAUSE_MSR_READ,
    CAUSE_THR_WRITE,
    CAUSE_MCR_WRITE,
    CAUSE_MODEM_IN,
    CAUSE_WRITE
} Cause;

/* ---- Small helpers ------------------------------------------------------------------ */

static SimTime t_of(const NsState *s, unsigned i) {
    return s->grade->t[i];
}

static unsigned depth(const NsChannel *c) {
    return c->fifo_on ? c->owner->dev->fifo : 1;
}

static LogicValue bit_of(Reg r, unsigned bit) {
    if (!((r.k >> bit) & 1u)) {
        return LOGIC_X;
    }
    return (r.v >> bit) & 1u ? LOGIC_1 : LOGIC_0;
}

static LogicValue not3(LogicValue a) {
    return a == LOGIC_0 ? LOGIC_1 : a == LOGIC_1 ? LOGIC_0 : LOGIC_X;
}

static LogicValue and3(LogicValue a, LogicValue b) {
    if (a == LOGIC_0 || b == LOGIC_0) {
        return LOGIC_0;
    }
    return a == LOGIC_1 && b == LOGIC_1 ? LOGIC_1 : LOGIC_X;
}

static LogicValue or3(LogicValue a, LogicValue b) {
    if (a == LOGIC_1 || b == LOGIC_1) {
        return LOGIC_1;
    }
    return a == LOGIC_0 && b == LOGIC_0 ? LOGIC_0 : LOGIC_X;
}

static LogicValue bool3(int b) {
    return b ? LOGIC_1 : LOGIC_0;
}

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
        static const unsigned DEFAULT_TRIGGER[4] = {1, 4, 8, 14};
        const unsigned *table = c->owner->dev->rx_trigger[0] != 0 ? c->owner->dev->rx_trigger
                                                                  : DEFAULT_TRIGGER;

        rda = bool3(c->rx_count >= table[c->trigger & 3u]);
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
    const LogicValue order[5] = {s.ls, s.timeout, s.rda, s.thre, s.modem};
    static const uint8_t ID[5] = {0x06, 0x0C, 0x04, 0x02, 0x00};
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
        drive(s, pin[CH_INT], LOGIC_0, 0);
        drive(s, pin[CH_RXRDY], LOGIC_1, 0);
        drive(s, pin[CH_TXRDY], LOGIC_0, 0);
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

    /* INTR: active high, always driven (intPin "driven"). */
    intr = int_level(c);
    drive(s, pin[CH_INT], intr, int_delay);

    /* RXRDY / TXRDY (active low). */
    rxrdy = mode1 ? bool3(!c->rxrdy1) : bool3(c->rx_count == 0);
    txrdy = mode1 ? bool3(!c->txrdy1) : bool3(c->tx_count != 0);
    drive(s, pin[CH_RXRDY], rxrdy, rxrdy_delay);
    drive(s, pin[CH_TXRDY], txrdy, txrdy_delay);
}

static void update_outputs(NsState *s, Cause cause) {
    unsigned i;

    for (i = 0; i < s->dev->channel_count; i++) {
        update_channel_outputs(s, &s->ch[i], cause);
    }
    serial_bus_refresh(&s->bus, s->sim, s->instance, &s->events);
}

static void update_dma_latches(NsChannel *c) {
    static const unsigned DEFAULT_TRIGGER[4] = {1, 4, 8, 14};
    const unsigned *table = c->owner->dev->rx_trigger[0] != 0 ? c->owner->dev->rx_trigger
                                                              : DEFAULT_TRIGGER;

    if (c->rx_count == 0) {
        c->rxrdy1 = 0;
    } else if (c->rx_count >= table[c->trigger & 3u] || c->timeout) {
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
    c->lcr = (Reg){0, 0xFF};
    c->mcr = (Reg){0, 0xFF};
    c->scr = (Reg){0, 0xFF};   /* decisions.md PC16550D: SCR = 0x00 after MR (P3) */
    c->fifo_on = 0;
    c->fifo_x = 0;
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

    /* Timeout: 4 character times (from RCLK) plus 8 RCLKs with data waiting. */
    if (c->fifo_on && c->rx_count > 0 && !c->timeout) {
        uint64_t limit = 4ull * serial_format_char_bits(&c->format) * c->format.factor + 8;

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
        case 5: lsr_of(c, v, k); break;
        case 6: msr_of(c, v, k); break;
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

            c->lsr_err = 0;
            c->lsr_err_k = 0xFF;
            /* LSR[7] stays while errors remain further down the FIFO. */
            c->lsr7 = 0;
            for (i = 1; i < c->rx_count; i++) {
                if (char_has_error(&c->rx[(c->rx_head + i) % NS_MAX_FIFO])) {
                    c->lsr7 = 1;
                }
            }
            return CAUSE_LSR_READ;
        }
        case 6:
            c->delta = 0;
            c->delta_k = 0xFF;
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
    if (serial_bus_has_pin(&s->bus, pin)) {
        serial_bus_update(&s->bus, sim, instance, &s->events);
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
    s->events.select = NULL;
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
    static const char *const CLOCK_KEYS[] = {"in", "out", "receive", "baudOut", NULL};
    const cJSON *clock = kind_member(ctx, json, "clock", 1);

    if (clock == NULL) {
        return -1;
    }
    kind_push(ctx, "clock");
    if (kind_check_keys(ctx, clock, CLOCK_KEYS, NULL) != 0 ||
        kind_string(ctx, clock, "in", &d->clock_in) != 0 ||
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
    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) != 1) {
        return kind_error(ctx, "\"channels\" must be an array of 1 channel");
    }
    cJSON_ArrayForEach(item, list) {
        unsigned k;

        kind_push(ctx, "channels[%u]", n);
        if (kind_check_keys(ctx, item, CHANNEL_KEYS, NULL) != 0) {
            return -1;
        }
        for (k = 0; k < CH_PIN_COUNT; k++) {
            if (serial_optional_name(ctx, item, CHANNEL_KEYS[k], &d->channel[n][k]) != 0) {
                return -1;
            }
        }
        if (d->channel[n][CH_TX] == NULL || d->channel[n][CH_RX] == NULL ||
            d->channel[n][CH_INT] == NULL) {
            return kind_error(ctx, "a channel needs \"tx\", \"rx\" and \"int\"");
        }
        kind_pop(ctx);
        n++;
    }
    d->channel_count = n;
    return 0;
}

static void *ns_load_device(KindContext *ctx, const cJSON *json, unsigned *channels) {
    NsDevice *d = kind_alloc(ctx, sizeof *d);
    uint64_t fifo;
    const char *text;

    if (d == NULL || load_bus(ctx, json, d) != 0 || load_clock(ctx, json, d) != 0 ||
        load_channels(ctx, json, d) != 0 || kind_uint(ctx, json, "fifo", 255, &fifo) != 0) {
        return NULL;
    }
    if (fifo != 0 && fifo != 16) {
        kind_error(ctx, "\"fifo\" must be 0 or 16");
        return NULL;
    }
    d->fifo = (unsigned)fifo;
    if (cJSON_GetObjectItemCaseSensitive(json, "rxTrigger") != NULL) {
        const cJSON *list = cJSON_GetObjectItemCaseSensitive(json, "rxTrigger");
        const cJSON *item;
        unsigned i = 0;

        if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) != 4) {
            kind_error(ctx, "\"rxTrigger\" must be 4 numbers");
            return NULL;
        }
        cJSON_ArrayForEach(item, list) {
            if (!cJSON_IsNumber(item) || item->valuedouble < 1 || item->valuedouble > d->fifo ||
                item->valuedouble != (double)(unsigned)item->valuedouble) {
                kind_error(ctx, "\"rxTrigger\" values must be whole numbers from 1 to the FIFO size");
                return NULL;
            }
            d->rx_trigger[i++] = (unsigned)item->valuedouble;
        }
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "intPin") != NULL) {
        if (kind_string(ctx, json, "intPin", &text) != 0) {
            return NULL;
        }
        if (strcmp(text, "driven") != 0) {
            kind_error(ctx, "unknown \"intPin\" \"%s\"", text);
            return NULL;
        }
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "dmaClass") != NULL) {
        if (kind_string(ctx, json, "dmaClass", &text) != 0) {
            return NULL;
        }
        if (strcmp(text, "P") != 0) {
            kind_error(ctx, "unknown \"dmaClass\" \"%s\"", text);
            return NULL;
        }
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "timeout") != NULL) {
        if (kind_string(ctx, json, "timeout", &text) != 0) {
            return NULL;
        }
        if (strcmp(text, "4ch") != 0) {
            kind_error(ctx, "unknown \"timeout\" \"%s\"", text);
            return NULL;
        }
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
    }
    return p;
}

const SerialModel NS16550_MODEL = {
    "ns16550", KEYS, TIMING, NULL,
    ns_load_device, ns_bind, NULL, NULL,
    ns_create, ns_destroy, ns_start, ns_pin_changed, NULL,
    ns_channels, ns_line, ns_async
};
