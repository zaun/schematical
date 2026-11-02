#ifndef NS16550_INT_H
#define NS16550_INT_H

/*
 * The ns16550 model's internal interface (plan-12-C section 3.6, D, E): the
 * device data, the per-channel and per-chip state, and the small helpers
 * the model's files share. ns16550.c is the core (bus, registers, FIFOs,
 * interrupts, line); ns16550_ext.c the enhanced register banks, flow
 * control, sleep, IrDA, RS-485, prescaler and fractional divisors;
 * ns16550_lpt.c the printer port.
 */

#include <stdint.h>

#include "serial.h"

enum { NS_MAX_FIFO = 128 };

enum {
    T_RVD, T_HZ, T_RDD, T_BLD, T_RINT, T_HR, T_MDO, T_SIM, T_RIM, T_WXI, T_WRR, T_IR, T_CSO
};




typedef enum {
    CH_TX, CH_RX, CH_RTS, CH_DTR, CH_CTS, CH_DSR, CH_DCD, CH_RI, CH_OUT1, CH_OUT2, CH_INT,
    CH_TXRDY, CH_RXRDY, CH_PIN_COUNT
} ChannelPin;


/* MSR status bits 4..7, in order: CTS, DSR, RI, DCD. */
enum { M_CTS, M_DSR, M_RI, M_DCD };

/* ---- Device data ---------------------------------------------------------------- */

typedef enum { BROADCAST_NONE, BROADCAST_MULTI_CS, BROADCAST_AFR_CONC } NsBroadcast;
typedef enum { DMA_PINS_NAMED, DMA_PINS_SHARED_AND, DMA_PINS_SHARED_WIRE_OR } NsDmaPins;
typedef enum {
    INT_DRIVEN, INT_OUT2_GATED, INT_INTN, INT_INTSEL, INT_MCR5_OPEN_SOURCE
} NsIntPin;
typedef enum { IIR_STANDARD, IIR_NXP_EQUAL } NsIirPriority;

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
    /* Per-channel chip selects (TL16C554, TL16C2550: one CS per channel). */
    const char *chan_select[SERIAL_MAX_CHANNELS][SERIAL_BUS_SELECTS];
    unsigned chan_select_count[SERIAL_MAX_CHANNELS];
    const char *chan_select_high[SERIAL_MAX_CHANNELS][SERIAL_BUS_SELECTS];
    unsigned chan_select_high_count[SERIAL_MAX_CHANNELS];
    int per_channel_select;
    const char *channel_pin;       /* "channelSelect": one CS plus a channel pin */
    int a_is_high;
    NsBroadcast broadcast;
    unsigned fifo;                 /* 0 = 16450; with fifo750, the 16-byte size */
    int fifo750;                   /* "fifo": "750": 16 or 64 by FCR[5] */
    unsigned rx_trigger[4];
    unsigned rx_trigger64[4];
    unsigned tx_trigger[4];
    int has_tx_trigger;
    unsigned tlr;                  /* 0, 4 or 8 */
    NsDmaPins dma_pins;
    const char *dma_txrdy, *dma_rxrdy;
    char dma_class;                /* 'P', 'N', 'T', 'X', 'B' */
    NsIntPin int_pin;
    const char *int_control;       /* INTN / INTSEL pin */
    uint8_t lcr_reset;
    int spr_reset;                 /* -1 = MR leaves SPR */
    uint8_t tcr_reset;
    NsIirPriority iir_priority;
    int lsr7_on_read;              /* "lsr7Clear": "lsrRead" */
    int stop_both;
    int status_defer;
    int bus_sync;
    int majority;
    int timeout_4w12;
} NsDevice;

typedef struct {
    SerialBusPins bus;
    size_t reset, clock_in, clock_out, receive, baudout;
    size_t channel[SERIAL_MAX_CHANNELS][CH_PIN_COUNT];
    size_t chan_select[SERIAL_MAX_CHANNELS][SERIAL_BUS_SELECTS];
    size_t chan_select_high[SERIAL_MAX_CHANNELS][SERIAL_BUS_SELECTS];
    size_t channel_pin;
    size_t dma_txrdy, dma_rxrdy;
    size_t int_control;
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
    int fifo64;                    /* "750": 64-byte mode (FCR[5]) */
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
    uint8_t lsr_seen;              /* "statusReadDefer": the LSR error bits a read showed */
    uint8_t delta_seen;            /* and the MSR deltas */
    int reading_status;            /* an LSR or MSR read is in progress (deferred status) */
    uint8_t shown, shown_k;        /* the byte that read shows */
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
    int bus_dirty;                 /* "busSync": bus pins changed since the last clock edge */
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

extern const ChannelPin MODEM_PIN[4];

/* ---- Small helpers ------------------------------------------------------------------ */

static inline SimTime t_of(const NsState *s, unsigned i) {
    return s->grade->t[i];
}

static inline unsigned depth(const NsChannel *c) {
    if (!c->fifo_on) {
        return 1;
    }
    return c->owner->dev->fifo750 && c->fifo64 ? 64 : c->owner->dev->fifo;
}

/* The RX trigger level FCR[7:6] picks. */
static inline unsigned rx_trigger_level(const NsChannel *c) {
    static const unsigned DEFAULT_TRIGGER[4] = {1, 4, 8, 14};
    const NsDevice *d = c->owner->dev;
    const unsigned *table = d->rx_trigger[0] != 0 ? d->rx_trigger : DEFAULT_TRIGGER;

    if (d->fifo750 && c->fifo64 && d->rx_trigger64[0] != 0) {
        table = d->rx_trigger64;
    }
    return table[c->trigger & 3u];
}

static inline LogicValue bit_of(Reg r, unsigned bit) {
    if (!((r.k >> bit) & 1u)) {
        return LOGIC_X;
    }
    return (r.v >> bit) & 1u ? LOGIC_1 : LOGIC_0;
}

static inline LogicValue not3(LogicValue a) {
    return a == LOGIC_0 ? LOGIC_1 : a == LOGIC_1 ? LOGIC_0 : LOGIC_X;
}

static inline LogicValue and3(LogicValue a, LogicValue b) {
    if (a == LOGIC_0 || b == LOGIC_0) {
        return LOGIC_0;
    }
    return a == LOGIC_1 && b == LOGIC_1 ? LOGIC_1 : LOGIC_X;
}

static inline LogicValue or3(LogicValue a, LogicValue b) {
    if (a == LOGIC_1 || b == LOGIC_1) {
        return LOGIC_1;
    }
    return a == LOGIC_0 && b == LOGIC_0 ? LOGIC_0 : LOGIC_X;
}

static inline LogicValue bool3(int b) {
    return b ? LOGIC_1 : LOGIC_0;
}

#endif
