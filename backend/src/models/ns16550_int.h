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
#include "serial_target.h"

enum { NS_MAX_FIFO = 128 };

enum {
    T_RVD, T_HZ, T_RDD, T_BLD, T_RINT, T_HR, T_MDO, T_SIM, T_RIM, T_WXI, T_WRR, T_IR, T_CSO,
    /* plan-12-I: the serial host ports and GPIO of the bridges */
    T_DO, T_TR, T_VD, T_GPIO
};




typedef enum {
    CH_TX, CH_RX, CH_RTS, CH_DTR, CH_CTS, CH_DSR, CH_DCD, CH_RI, CH_OUT1, CH_OUT2, CH_INT,
    CH_TXRDY, CH_RXRDY, CH_PIN_COUNT
} ChannelPin;


/* MSR status bits 4..7, in order: CTS, DSR, RI, DCD. */
enum { M_CTS, M_DSR, M_RI, M_DCD };

/* ---- Device data ---------------------------------------------------------------- */

typedef enum {
    BROADCAST_NONE, BROADCAST_MULTI_CS, BROADCAST_AFR_CONC,
    BROADCAST_AFR0_PER_CHANNEL   /* plan-12-E */
} NsBroadcast;
typedef enum { DMA_PINS_NAMED, DMA_PINS_SHARED_AND, DMA_PINS_SHARED_WIRE_OR } NsDmaPins;
typedef enum {
    INT_DRIVEN, INT_OUT2_GATED, INT_INTN, INT_INTSEL, INT_MCR5_OPEN_SOURCE
} NsIntPin;
typedef enum { IIR_STANDARD, IIR_NXP_EQUAL, IIR_EXAR_TIMEOUT_FIRST /* plan-12-E */ } NsIirPriority;

/* plan-12-D Phase 2 options (ns16550_ext.c). */
typedef enum {
    EXT_NONE, EXT_AFE, EXT_750, EXT_650, EXT_752, EXT_NS_EFR,
    /* plan-12-I bridges (ns16550_bridge.c, ns16550_xr1280.c) */
    EXT_SC16IS, EXT_SC16IS_SFR, EXT_SC16IS_DLD, EXT_XR1280,
    /* plan-12-E (ns16550_exar.c) */
    EXT_IER5, EXT_EXAR650, EXT_EXAR854, EXT_M752, EXT_5G
} NsExtended;
typedef enum { AFR_NONE, AFR_TI752, AFR_TI2552, AFR_NS, AFR_EXAR /* plan-12-E */ } NsAfr;
typedef enum {
    FLOW_NONE, FLOW_AFE, FLOW_TCR, FLOW_TABLE,
    FLOW_EXAR_TABLE, FLOW_DTR_DSR   /* plan-12-E */
} NsAutoFlow;
typedef enum { SW_NONE, SW_NXP_AND, SW_TI, SW_EXAR_OR /* plan-12-E */ } NsSwFlow;
typedef enum {
    SLEEP_NONE, SLEEP_IER4, SLEEP_IER4_ALL,
    SLEEP_IER4_EXAR, SLEEP_IER4_EXAR_ALL, SLEEP_GLOBAL   /* plan-12-E */
} NsSleep;
typedef enum { IRDA_NONE, IRDA_MCR6, IRDA_AFR1, IRDA_MCR6_EXAR, IRDA_DLD7 /* plan-12-E */ } NsIrda;
typedef enum {
    RS485_NONE, RS485_AFR_DTR,
    RS485_FCTR_RTS, RS485_FCTR_MCR2, RS485_DLD6, RS485_FCTR3_OP1   /* plan-12-E */
} NsRs485;
typedef enum {
    PRESCALE_NONE, PRESCALE_MCR7, PRESCALE_MCR7_CLKSEL, PRESCALE_CLKSEL /* plan-12-E */
} NsPrescaler;
typedef enum { CLEAR_IIR, CLEAR_LSR, CLEAR_MSR, CLEAR_IIR_OR_MSR } NsClear;

/* The printer port (ns16550_lpt.c, plan-12-D section 3.2). */
typedef enum { LPT_TI451, LPT_TI552, LPT_COM92C451 } NsLptStyle;
typedef struct NsLptDevice {
    NsLptStyle style;
    const char *data[8];
    const char *select;            /* the port's chip select, active low */
    const char *strobe, *auto_feed, *init, *select_in;   /* open-drain outputs */
    const char *ack, *busy, *paper_end, *slct, *error;  /* inputs */
    const char *irq;               /* INT2 / IRQ0 */
    const char *input_enable;      /* TL16C451/452 LPTOE */
    const char *pemd, *enirq;      /* TL16C552 */
} NsLptDevice;
typedef struct NsLptPins {
    size_t data[8];
    size_t select, strobe, auto_feed, init, select_in, ack, busy, paper_end, slct, error, irq,
        input_enable, pemd, enirq;
} NsLptPins;

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
    unsigned timeout_extra;        /* RCLKs added to the 4ch timeout (8 default) */
    int baudout_low2;              /* BAUDOUT low for 2 clocks when the divisor is > 3 */
    /* plan-12-D Phase 2 */
    NsExtended extended;
    NsAfr afr;
    uint8_t afr_reset;
    int dlf;
    NsAutoFlow auto_flow;
    uint8_t rts_table[4][2];       /* per RX trigger: [off at, on at] (FLOW_TABLE) */
    uint8_t xoff_table[4][2];      /* per RX trigger: [Xoff sent at, Xon sent at] */
    int has_xoff_table;
    NsSwFlow sw_flow;
    int special_char;
    int special_needs_no_flow;     /* SC16C650B: special only with EFR[3:0] = 0 */
    NsClear special_clear;
    NsClear ctsrts_clear;
    NsSleep sleep;
    NsIrda irda;
    NsRs485 rs485;
    NsPrescaler prescaler;
    const char *clock_select;      /* CLKSEL pin */
    int lsr_head;                  /* "lsrErrors": "head": LSR[4:2] are the head byte's */
    int mcr_reset;                 /* -1: 0x00 */
    int xon_power[4];              /* power-up Xon1, Xon2, Xoff1, Xoff2; -1 = X */
    int tcr_needs_dlab0;
    int dlab_hides_upper;          /* SC16C650B: with LCR[7] = 1 only DLL, DLM, LCR */
    int drev;                      /* "idRegs": DREV/DVID at 0/1 with DLL = DLM = 0; -1 none */
    int dvid;
    int xon_any_stores;            /* NS: an Xon-any character is stored */
    const NsLptDevice *lpt;        /* "printerPort" */
    unsigned divisor_zero;         /* COM92C451: divisor 0 divides by this (0 = stops) */
    const char *tri;               /* TL16C552 TRI pin: every output floats while high */
    const char *op[SERIAL_MAX_CHANNELS];
    const char *mf[SERIAL_MAX_CHANNELS];
    SerialAbsent absent;           /* "absentTies", "optionalPins" */
    int spr_power_up;              /* "sprPowerUp": SPR at power-up; -1 = X */
    int no_dma_mode1;              /* "dmaMode1": false: FCR[3] ignored */
    int lsr_clear_on_good;         /* "lsrClearOnGood": NS: LSR[4:1] clear on the next good byte */
    unsigned break_mark_ticks;     /* "breakMarkTicks": mark ticks needed after a break */
    /* plan-12-I: an I2C/SPI host port instead of the parallel bus (NULL: none). */
    const struct NsBridgeDevice *bridge;
    const struct NsExarDevice *x;  /* plan-12-E: the Exar keys (ns16550_exar.c); NULL = none */
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
    size_t clock_select;
    size_t op[SERIAL_MAX_CHANNELS];   /* OP pins (NOT MCR[3]) */
    size_t mf[SERIAL_MAX_CHANNELS];   /* MF pins (AFR[2:1]) */
    const NsLptPins *lpt;
    size_t tri;
    const struct NsBridgePins *bridge;   /* plan-12-I */
    const struct NsExarPins *xp;   /* plan-12-E */
} NsPins;

/* ---- State ------------------------------------------------------------------------ */

typedef struct {
    uint8_t v;
    uint8_t k;   /* known bits */
} Reg;

typedef struct NsState NsState;

#include "ns16550_exar.h"   /* plan-12-E: NsExarChannel, NsExarState */

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
    /* plan-12-D Phase 2 (ns16550_ext.c) */
    Reg efr, xon1, xon2, xoff1, xoff2, tcr, tlr, afr, dlf;
    unsigned tx_trigger;           /* FCR[5:4] */
    int thr_level_int;             /* THR interrupt by TX level (enhanced mode) */
    int tx_halted;                 /* an Xoff came in */
    int xoff_sent;                 /* we sent Xoff; an Xon is owed */
    uint8_t flow_out[4];           /* flow characters waiting to go out, before data */
    unsigned flow_out_count;
    int seq_wait;                  /* 1: Xon1 seen, 2: Xoff1 seen (sequential pairs) */
    SerialRxChar seq_held;
    int iir_xoff;                  /* IIR[4]: Xoff received */
    int iir_special;               /* IIR[4]: special character */
    int iir_ctsrts;                /* IIR[5]: CTS/RTS low -> high */
    int rts_off;                   /* auto-RTS has taken RTS inactive */
    LogicValue rts_pin_last;
    LogicValue cts_last;
    int asleep;
    int low_power;
    unsigned ir_low;               /* IrDA decoder: ticks left of a decoded 0 */
    LogicValue ir_last;
    int rs485_on;                  /* DTR driven for RS-485 */
    unsigned rs485_lead;           /* ticks of DTR before the start bit */
    unsigned rs485_hold;           /* ticks of DTR after the last stop bit */
    unsigned prescale_count;
    int xon_seen;                  /* plan-12-I: an Xon resumed the transmitter (PI7C9X ASR) */
    NsExarChannel xc;              /* plan-12-E */
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
    int conc;                      /* AFR concurrent write */
    int tri;                       /* TRI is high */
    struct NsLpt *lpt;
    struct NsBridge *bridge;       /* plan-12-I: the host port's state */
    NsExarState xs;                /* plan-12-E */
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

/* ---- Shared between the model's files ---------------------------------------------- */

void ns_drive(NsState *s, size_t pin, LogicValue value, SimTime delay);
void ns_store_char(NsChannel *c, const SerialRxChar *rc);
void ns_iir_now(const NsChannel *c, uint8_t *v, uint8_t *k);
void ns_update_outputs(NsState *s, Cause cause);
void ns_update_channel(NsState *s, NsChannel *c, Cause cause);
void ns_set_thre(NsChannel *c);
void ns_clear_rx(NsChannel *c);
void ns_clear_tx(NsChannel *c);
int ns_rx_level(const NsChannel *c);
/* plan-12-E: core entry points ns16550_exar.c uses. */
LogicValue ns_int_level(const NsChannel *c);
void ns_reg_peek(NsChannel *c, unsigned address, uint8_t *v, uint8_t *k);
Cause ns_reg_read_done(NsChannel *c, unsigned address);
Cause ns_reg_write(NsState *s, NsChannel *c, unsigned address, uint8_t v, uint8_t k);
void ns_reset_channel(NsState *s, NsChannel *c);
void ns_tx_tick(NsState *s, NsChannel *c);
void ns_rx_tick(NsState *s, NsChannel *c);

/* ns16550_ext.c: the enhanced registers and their features. */
void ns_ext_reset(NsState *s, NsChannel *c, int power_on);
/* The registers of the enhanced banks (plan-12-I: the bridges' decode
   names them too; codes from NS_REG_BRIDGE on are the bridge's own). */
typedef enum {
    R_CORE, R_X, R_IGNORE, R_EFR, R_XON1, R_XON2, R_XOFF1, R_XOFF2, R_TCR, R_TLR, R_FIFORDY,
    R_AFR, R_DLF, R_DREV, R_DVID,
    NS_REG_BRIDGE = 64
} ExtReg;
void ns_ext_peek_reg(NsChannel *c, int reg, uint8_t *v, uint8_t *k);
void ns_ext_write_reg(NsState *s, NsChannel *c, int reg, uint8_t v, uint8_t k);
/* Register access for the current LCR/EFR/MCR: 1 = handled. */
int ns_ext_peek(NsChannel *c, unsigned address, uint8_t *v, uint8_t *k);
int ns_ext_write(NsState *s, NsChannel *c, unsigned address, uint8_t v, uint8_t k, Cause *cause);
/* Bits of IER, MCR and FCR a write may change now. */
uint8_t ns_ext_ier_mask(const NsChannel *c);
uint8_t ns_ext_mcr_mask(const NsChannel *c);
void ns_ext_fcr(NsChannel *c, uint8_t v, uint8_t k);
/* Received character: 1 = store it, 0 = it was a flow character. */
int ns_ext_rx_char(NsState *s, NsChannel *c, const SerialRxChar *rc);
/* RX FIFO level or TX state changed: auto-RTS, Xon/Xoff sending, sleep. */
void ns_ext_levels(NsState *s, NsChannel *c);
/* Transmit: a flow character to send first (1), or 0 = none. */
int ns_ext_tx_flow(NsChannel *c, uint16_t *data);
/* 1 when the transmitter may not take the next data byte now. */
int ns_ext_tx_blocked(NsState *s, NsChannel *c);
/* Extra interrupt sources: IIR[4] (Xoff/special) and IIR[5] (CTS/RTS). */
LogicValue ns_ext_xoff_source(const NsChannel *c);
LogicValue ns_ext_ctsrts_source(const NsChannel *c);
/* An IIR read that reported code `iir` ended; an LSR/MSR read ended. */
void ns_ext_iir_read(NsChannel *c, uint8_t iir);
void ns_ext_status_read(NsChannel *c, int msr);
/* Pins: RTS and DTR (overrides), TX encode, RX decode, OP/MF. */
LogicValue ns_ext_rts(const NsChannel *c, LogicValue normal);
LogicValue ns_ext_dtr(const NsChannel *c, LogicValue normal);
LogicValue ns_ext_tx_pin(const NsChannel *c, LogicValue normal);
LogicValue ns_ext_rx_decode(NsChannel *c, LogicValue pin);
void ns_ext_pins(NsState *s, NsChannel *c, Cause cause);
/* Modem input changed: auto-CTS interrupt; wake. 0 = suppress the delta (auto-CTS). */
int ns_ext_cts_changed(NsChannel *c, LogicValue cts_active);
/* Clock: 1 when this edge reaches the baud generator (prescaler; sleep). */
int ns_ext_prescale(NsState *s, NsChannel *c, int rising);
/* Load the divisor with DLF / prescaler; the sampling factor. */
void ns_ext_load_divisor(NsChannel *c);
unsigned ns_ext_factor(const NsChannel *c);
/* Sleep: wake on activity; enter when idle. */
void ns_ext_wake(NsChannel *c);
void ns_ext_sleep_check(NsState *s, NsChannel *c);
/* The TX trigger in free spaces, 0 = the THR interrupt is "empty". */
unsigned ns_ext_tx_threshold(const NsChannel *c);
unsigned ns_ext_rx_trigger(const NsChannel *c, unsigned fcr_level);
/* FIFO ready register (752), and the AFR's concurrent write. */
uint8_t ns_ext_fifo_ready(const NsState *s);

/* ---- plan-12-I: I2C/SPI bridges (ns16550_bridge.c, ns16550_xr1280.c) ---------- */

/* Core entry points for the host port: a register access by channel and
   address (0-15), with the core's side effects and output updates. */
void ns_host_peek(NsState *s, unsigned ch, unsigned address, uint8_t *v, uint8_t *k);
void ns_host_read_done(NsState *s, unsigned ch, unsigned address);
void ns_host_write(NsState *s, unsigned ch, unsigned address, uint8_t v, uint8_t k);
/* Every channel to its reset state, as the reset pin (software reset). */
void ns_reset_all(NsState *s);
/* The modem status bits re-read without deltas (a pin's function changed). */
void ns_modem_refresh(NsState *s, NsChannel *c);
/* The interrupt sources, pending AND enabled: line status, RX data, timeout,
   THR, modem, Xoff/special, CTS/RTS, GPIO. */
void ns_sources(const NsChannel *c, LogicValue out[8]);
/* One transmit / receive tick of a channel (independent generators). */
void ns_tx_tick(NsState *s, NsChannel *c);
void ns_rx_tick(NsState *s, NsChannel *c);
void ns_update_format(NsChannel *c);

int ns_bridge_load(KindContext *ctx, const cJSON *json, const cJSON *bus, NsDevice *d);
int ns_bridge_bind(KindContext *ctx, const NsDevice *d, const PartPin *pins, size_t pin_count,
                   NsPins *p);
int ns_bridge_open_drain(const NsDevice *d, const NsPins *p, size_t pin);
int ns_bridge_create(NsState *s);
void ns_bridge_destroy(NsState *s);
void ns_bridge_start(NsState *s);
/* The reset pin: 1 active, 0 released, X unknown. */
void ns_bridge_reset(NsState *s, LogicValue active);
void ns_bridge_pin(NsState *s, size_t pin);
void ns_bridge_timer(NsState *s, uint32_t id);
/* After a channel's outputs: the device IRQ, the GPIO and modem pins. */
void ns_bridge_outputs(NsState *s, Cause cause);
/* 1 when the bridge drives this pin (the core leaves it alone). */
int ns_bridge_owns(const NsState *s, size_t pin);
/* 0 when a modem input pin is a GPIO now (its MSR bit reads X); *z_reads:
   what the pin reads at Z (the GPIO pins' pull-up). */
int ns_bridge_modem_input(const NsState *s, unsigned ch, size_t pin, LogicValue *z_reads);
/* Register decode: an ExtReg code (R_CORE: the core's register). */
int ns_bridge_decode(const NsChannel *c, unsigned address, int write);
void ns_bridge_peek_reg(NsChannel *c, int reg, uint8_t *v, uint8_t *k);
void ns_bridge_write_reg(NsState *s, NsChannel *c, int reg, uint8_t v, uint8_t k);
/* A read of a bridge register ended: 1 = handled (its cause in *cause). */
int ns_bridge_read_done(NsChannel *c, unsigned address, Cause *cause);
/* Hooks into the core and ext features; -1 = no say. */
int ns_bridge_rx_char(NsState *s, NsChannel *c, const SerialRxChar *rc);
int ns_bridge_tx_blocked(NsState *s, NsChannel *c);
int ns_bridge_levels(NsState *s, NsChannel *c);
LogicValue ns_bridge_rts(const NsChannel *c, LogicValue normal);
LogicValue ns_bridge_tx_pin(const NsChannel *c, LogicValue normal, int irda);
int ns_bridge_irda(const NsChannel *c);
int ns_bridge_ir_quarter(const NsChannel *c);
int ns_bridge_prescale(NsChannel *c, int rising);
int ns_bridge_load_divisor(NsChannel *c);
int ns_bridge_factor(const NsChannel *c);
int ns_bridge_tx_threshold(const NsChannel *c, unsigned *out);
int ns_bridge_rx_trigger(const NsChannel *c, unsigned *out);
int ns_bridge_ier_mask(const NsChannel *c, uint8_t *mask);
int ns_bridge_mcr_mask(const NsChannel *c, uint8_t *mask);
int ns_bridge_thr_on_enable(const NsChannel *c);
LogicValue ns_bridge_gpio_source(const NsChannel *c);
LogicValue ns_bridge_ls_source(const NsChannel *c, LogicValue normal);
LogicValue ns_bridge_thr_source(const NsChannel *c, LogicValue normal);
int ns_bridge_timeout_on(const NsChannel *c);
void ns_bridge_iir_bits(const NsChannel *c, uint8_t *v, uint8_t *k);
void ns_bridge_thr_written(NsChannel *c, unsigned at);
int ns_bridge_tx_fetch(NsChannel *c);
int ns_bridge_clock(NsState *s, NsChannel *c, int rising, LogicValue level);
int ns_bridge_sleep_check(NsState *s, NsChannel *c);
void ns_bridge_sleep_changed(NsState *s, NsChannel *c);

/* ns16550_exar.c (plan-12-E): the Exar/MaxLinear options. The ns_ext_*
   functions hand every call for a device with Exar keys (dev->x) to their
   ns_x_* twin; the core calls the rest directly. */
int ns_x_load(KindContext *ctx, const cJSON *json, NsDevice *d);
int ns_x_load_bus(KindContext *ctx, const cJSON *bus, NsDevice *d, const char *style);
int ns_x_load_channel(KindContext *ctx, const cJSON *item, NsDevice *d, unsigned n);
int ns_x_bind(KindContext *ctx, const NsDevice *d, const PartPin *pins, size_t pin_count,
              NsPins *p, const SerialAbsent *absent);
const void *ns_x_load_part(KindContext *ctx, const void *data, const cJSON *json);
int ns_x_open_drain(const void *data, const void *pins, const void *options, size_t pin);
void ns_x_reset(NsState *s, NsChannel *c, int power_on);
int ns_x_peek(NsChannel *c, unsigned address, uint8_t *v, uint8_t *k);
void ns_x_read_done(NsChannel *c, unsigned address);
int ns_x_write(NsState *s, NsChannel *c, unsigned address, uint8_t v, uint8_t k, Cause *cause);
uint8_t ns_x_ier_mask(const NsChannel *c);
uint8_t ns_x_mcr_mask(const NsChannel *c);
void ns_x_mcr_written(NsChannel *c, uint8_t mask);
void ns_x_fcr(NsChannel *c, uint8_t v, uint8_t k);
int ns_x_rx_char(NsState *s, NsChannel *c, const SerialRxChar *rc);
void ns_x_levels(NsState *s, NsChannel *c);
int ns_x_tx_flow(NsChannel *c, uint16_t *data);
int ns_x_tx_blocked(NsState *s, NsChannel *c);
LogicValue ns_x_xoff_source(const NsChannel *c);
LogicValue ns_x_ctsrts_source(const NsChannel *c);
void ns_x_iir_read(NsChannel *c, uint8_t iir);
void ns_x_status_read(NsChannel *c, int msr);
LogicValue ns_x_rts(const NsChannel *c, LogicValue normal);
LogicValue ns_x_dtr(const NsChannel *c, LogicValue normal);
LogicValue ns_x_out1(const NsChannel *c, LogicValue normal);
LogicValue ns_x_tx_pin(const NsChannel *c, LogicValue normal);
LogicValue ns_x_rx_decode(NsChannel *c, LogicValue pin);
void ns_x_pins(NsState *s, NsChannel *c, Cause cause);
int ns_x_cts_changed(NsChannel *c, LogicValue cts_active);
int ns_x_prescale(NsState *s, NsChannel *c, int rising);
void ns_x_load_divisor(NsChannel *c);
unsigned ns_x_factor(const NsChannel *c);
unsigned ns_x_start_sample(const NsChannel *c, unsigned factor);
void ns_x_wake(NsChannel *c);
void ns_x_sleep_check(NsState *s, NsChannel *c);
unsigned ns_x_tx_threshold(const NsChannel *c);
unsigned ns_x_rx_trigger(const NsChannel *c, unsigned fcr_level);
int ns_x_thre_at_empty(NsChannel *c);
void ns_x_sources(const NsChannel *c, LogicValue *ls, LogicValue *rda, LogicValue *thre,
                  LogicValue *wake);
void ns_x_iir(const NsChannel *c, uint8_t *v, uint8_t *k);
int ns_x_int_pin(const NsState *s, const NsChannel *c, LogicValue level, LogicValue *out);
void ns_x_dma_pins(const NsChannel *c, LogicValue *rxrdy, LogicValue *txrdy);
void ns_x_device_pins(NsState *s, Cause cause);
void ns_x_error_in(NsChannel *c);
int ns_x_timeout_holds(const NsChannel *c);
void ns_x_lsr_read(NsChannel *c);
void ns_x_tick(NsState *s, NsChannel *c);
int ns_x_needs_select(const NsDevice *d);
int ns_x_select(NsState *s, unsigned address, unsigned address_known, uint32_t *units,
                LogicValue *result);
int ns_x_unit_peek(NsState *s, unsigned unit, unsigned address, uint8_t *v, uint8_t *k);
int ns_x_unit_read_done(NsState *s, unsigned unit, unsigned address);
int ns_x_unit_write(NsState *s, unsigned unit, unsigned address, uint8_t v, uint8_t k);
int ns_x_conc(const NsState *s, unsigned unit);
int ns_x_reset_active_high(const NsState *s);
int ns_x_own_clock(const NsState *s, unsigned channel);
void ns_x_clock_edge(NsState *s, int rising);
void ns_x_clock_out(NsState *s, NsChannel *c, LogicValue baudout);
int ns_x_is_select_pin(const NsState *s, size_t pin);
int ns_x_pin_changed(NsState *s, size_t pin);
void ns_x_start(NsState *s);

/* ns16550_lpt.c: the printer port. */
struct NsLpt *ns_lpt_create(const NsState *s);
void ns_lpt_destroy(struct NsLpt *lpt);
void ns_lpt_reset(NsState *s);
int ns_lpt_selected(const NsState *s);
void ns_lpt_peek(NsState *s, unsigned address, uint8_t *v, uint8_t *k);
void ns_lpt_write(NsState *s, unsigned address, uint8_t v, uint8_t k);
void ns_lpt_read_done(NsState *s, unsigned address);
void ns_lpt_pins(NsState *s);
int ns_lpt_has_pin(const NsState *s, size_t pin);

#endif
