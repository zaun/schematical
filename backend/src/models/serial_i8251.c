/*
 * i8251: the Intel 8251A USART (plan-12-H section 3.2; facts in
 * plan-12-data/specs/classic-b.md section 8251A, decisions.md section
 * 8251A).
 *
 * Bus: CS low, C/D picks data (0) or control/status (1), RD and WR active
 * low; a write is taken on WR's rising edge (serial_bus.c). After a reset
 * the first control write is the mode instruction; a synchronous mode then
 * takes SYNC1 (and SYNC2 for double sync); every later control write is a
 * command instruction until a reset or the command's IR bit.
 *
 * Asynchronous mode: the line engine with TxC (falling edges shift TxD) and
 * RxC (rising edges sample RxD) divided by the baud factor (1x, 16x, 64x;
 * 1x runs the engine at factor 2 with two ticks per edge, serial_util.h).
 * Synchronous mode: one bit per TxC/RxC edge, characters of L data bits
 * plus parity, no start or stop bits; the receiver hunts bit by bit for
 * SYNC1 (then SYNC2) or follows the SYNDET input (external sync); the
 * transmitter fills with the sync character(s) when the CPU is late.
 *
 * Signal rules (plan-12-A section 3.5): power-on is the reset state (mode
 * format expected, transmitter and receiver disabled, DTR/RTS high, TxRDY
 * and RxRDY low, TxEMPTY high, SYNDET low); the mode, sync and receive
 * registers power on X. CS and RESET read inactive at Z.
 * CLK sets only maximum delays (tCY multiples), which are not modeled (P5).
 */

#include <stdlib.h>
#include <string.h>

#include "serial_util.h"

enum { T_RD, T_DF, T_CLR, T_DTX, T_WC };
static const char *const TIMING[] = {"trd", "tdf", "tclr", "tdtx", "twc", NULL};

static const char *const KEYS[] = {"bus", "clock", "serial", NULL};

enum { B_CD, B_READ, B_WRITE, B_SELECT, B_RESET, B_COUNT };
static const char *const BUS_KEYS[] = {"data", "controlData", "read", "write", "select", "reset",
                                       NULL};

enum { P_TX, P_RX, P_TXC, P_RXC, P_TXRDY, P_RXRDY, P_TXEMPTY, P_SYNDET, P_DSR, P_DTR, P_CTS,
       P_RTS, P_COUNT };
static const char *const SERIAL_KEYS[] = {
    "tx", "rx", "txClock", "rxClock", "txrdy", "rxrdy", "txempty", "syndet", "dsr", "dtr", "cts",
    "rts", NULL
};

typedef struct {
    const char *data[8];
    const char *bus[B_COUNT];
    const char *clock;
    const char *pin[P_COUNT];
} I8Device;

typedef struct {
    SerialBusPins bus;
    size_t reset, clock;
    size_t pin[P_COUNT];
} I8Pins;

typedef enum { PH_MODE, PH_SYNC1, PH_SYNC2, PH_COMMAND, PH_X } Phase;

typedef struct {
    SerialBase base;
    const I8Device *dev;
    const I8Pins *pins;
    const SerialGrade *grade;
    Sim *sim;
    SimInstance *instance;
    SuOutputs o;
    SerialBus bus;
    SerialBusEvents events;
    SerialLine line;
    SerialFormat format;

    int in_reset;
    int unknown;
    Phase phase;
    SuReg mode, sync1, sync2, command;
    LogicValue txc_level, rxc_level, rd_level, wr_level;

    /* Transmitter. */
    SuReg tx_buf;
    int tx_full;
    int committed;               /* written while enabled: sent even if disabled */
    LogicValue tsr;              /* the transmitter's output */
    int txrdy_hold;              /* a data write's WR is low */
    /* Synchronous transmitter. */
    int sync_started;
    uint16_t sh_data, sh_known;
    unsigned sh_left, sh_bits;
    int sh_fill;
    unsigned fill_next;          /* 0: SYNC1 next, 1: SYNC2 next */
    int filling;

    /* Receiver. */
    SuReg rx_buf;
    LogicValue rxrdy, pe, oe, fe;
    int rxrdy_hold;
    LogicValue syndet;           /* the SYNDET (sync) / BRKDET (async) flip-flop */
    unsigned long low_ticks;     /* async: RxD low run, for BRKDET */
    int resync;
    /* Synchronous receiver. */
    int hunt;
    int synced;
    int sync2_wait;
    uint16_t rs_data, rs_known;  /* the last n bits, newest at the top */
    unsigned rs_count;           /* bits of the current character so far */
    int pair_first;              /* double sync: the last character was SYNC1 */
    LogicValue syndet_in;        /* the SYNDET pin as an input (external sync) */
} I8State;

static SimTime t_of(const I8State *s, unsigned i) {
    return s->grade->t[i];
}

/* ---- Mode decode ------------------------------------------------------------------------ */

static int mode_known(const I8State *s) {
    return s->phase != PH_MODE && s->phase != PH_X;
}

/* 1 async, 0 sync, -1 unknown. */
static int is_async(const I8State *s) {
    if (!mode_known(s) || (s->mode.k & 3u) != 3u) {
        return -1;
    }
    return (s->mode.v & 3u) != 0;
}

static unsigned char_length(const I8State *s) {
    return 5u + ((s->mode.v >> 2) & 3u);
}

static int parity_on(const I8State *s) {
    return (s->mode.v >> 4) & 1u;
}

static void update_format(I8State *s) {
    SerialFormat f = serial_format_default();
    unsigned b = s->mode.v & 3u;
    unsigned stops = (s->mode.v >> 6) & 3u;

    f.data_bits = char_length(s);
    f.parity = !parity_on(s) ? SERIAL_PARITY_NONE
             : (s->mode.v & 0x20u) ? SERIAL_PARITY_EVEN : SERIAL_PARITY_ODD;
    f.stop_halves = stops == 1 ? 2 : stops == 2 ? 3 : 4;
    f.factor = b == 1 ? SU_ONE_X_FACTOR : b == 2 ? 16 : 64;
    f.format_x = is_async(s) != 1 || s->mode.k != 0xFF || stops == 0;
    s->format = f;
}

static LogicValue cmd(const I8State *s, unsigned bit) {
    if (s->phase != PH_COMMAND && s->phase != PH_X) {
        return LOGIC_0;   /* before the first command: everything off */
    }
    return su_bit(s->command, bit);
}

/* ---- Outputs ------------------------------------------------------------------------------ */

static LogicValue cts_active(const I8State *s) {
    return su_not(serial_level(s->sim, s->instance, s->pins->pin[P_CTS], LOGIC_X));
}

static LogicValue tx_enabled(const I8State *s) {
    return su_and(cmd(s, 0), cts_active(s));
}

static int tx_busy(const I8State *s) {
    return is_async(s) == 1 ? serial_tx_busy(&s->line.tx) : s->sync_started;
}

static LogicValue txempty_level(const I8State *s) {
    if (is_async(s) == 0) {
        /* Sync: high while no data character is loaded (fill or idle). */
        return su_bool(!s->tx_full && (s->sh_fill || !s->sync_started));
    }
    if (s->tx_full && cmd(s, 0) != LOGIC_0) {
        return cmd(s, 0) == LOGIC_1 ? LOGIC_0 : LOGIC_X;
    }
    return su_bool(!tx_busy(s));
}

static void status_of(const I8State *s, uint8_t *v, uint8_t *k) {
    LogicValue bits[8];
    unsigned i;

    bits[0] = su_bool(!s->tx_full);
    bits[1] = s->rxrdy;
    bits[2] = txempty_level(s);
    bits[3] = s->pe;
    bits[4] = s->oe;
    bits[5] = s->fe;
    bits[6] = s->syndet;
    bits[7] = su_not(serial_level(s->sim, s->instance, s->pins->pin[P_DSR], LOGIC_X));
    *v = 0;
    *k = 0;
    for (i = 0; i < 8; i++) {
        if (bits[i] == LOGIC_1) *v |= (uint8_t)(1u << i);
        if (bits[i] != LOGIC_X) *k |= (uint8_t)(1u << i);
    }
}

static int syndet_is_input(const I8State *s) {
    return is_async(s) == 0 && (s->mode.v & 0x40u);
}

static void update_outputs(I8State *s, SimTime delay) {
    const size_t *p = s->pins->pin;
    LogicValue txd;
    LogicValue txrdy;
    LogicValue sbrk;
    unsigned i;

    if (s->unknown) {
        for (i = 0; i < P_COUNT; i++) {
            if (i == P_TX || i == P_TXRDY || i == P_RXRDY || i == P_TXEMPTY || i == P_SYNDET ||
                i == P_DTR || i == P_RTS) {
                su_drive(&s->o, s->sim, s->instance, p[i], LOGIC_X, 0);
            }
        }
        return;
    }
    if (s->in_reset) {
        su_drive(&s->o, s->sim, s->instance, p[P_TX], LOGIC_1, 0);
        su_drive(&s->o, s->sim, s->instance, p[P_TXRDY], LOGIC_0, 0);
        su_drive(&s->o, s->sim, s->instance, p[P_RXRDY], LOGIC_0, 0);
        su_drive(&s->o, s->sim, s->instance, p[P_TXEMPTY], LOGIC_1, 0);
        su_drive(&s->o, s->sim, s->instance, p[P_SYNDET], LOGIC_0, 0);
        su_drive(&s->o, s->sim, s->instance, p[P_DTR], LOGIC_1, 0);
        su_drive(&s->o, s->sim, s->instance, p[P_RTS], LOGIC_1, 0);
        return;
    }
    /* TxD: SBRK forces it low; else the transmitter (mark when idle). */
    sbrk = cmd(s, 3);
    txd = s->tsr;
    if (sbrk == LOGIC_1) {
        txd = LOGIC_0;
    } else if (sbrk == LOGIC_X && txd != LOGIC_0) {
        txd = LOGIC_X;
    }
    su_drive(&s->o, s->sim, s->instance, p[P_TX], txd, t_of(s, T_DTX));
    /* TxRDY pin: buffer empty, CTS low and TxEN. */
    txrdy = su_and(su_bool(!s->tx_full && !s->txrdy_hold), tx_enabled(s));
    su_drive(&s->o, s->sim, s->instance, p[P_TXRDY], txrdy, s->txrdy_hold ? t_of(s, T_CLR) : delay);
    su_drive(&s->o, s->sim, s->instance, p[P_RXRDY],
             s->rxrdy_hold ? LOGIC_0 : su_and(s->rxrdy, cmd(s, 2)),
             s->rxrdy_hold ? t_of(s, T_CLR) : delay);
    su_drive(&s->o, s->sim, s->instance, p[P_TXEMPTY], txempty_level(s), delay);
    if (syndet_is_input(s)) {
        su_drive(&s->o, s->sim, s->instance, p[P_SYNDET], LOGIC_Z, 0);
    } else {
        su_drive(&s->o, s->sim, s->instance, p[P_SYNDET], s->syndet, delay);
    }
    su_drive(&s->o, s->sim, s->instance, p[P_DTR], su_not(cmd(s, 1)), t_of(s, T_WC));
    su_drive(&s->o, s->sim, s->instance, p[P_RTS], su_not(cmd(s, 5)), t_of(s, T_WC));
}

static void refresh(I8State *s, SimTime delay) {
    update_outputs(s, delay);
    serial_bus_refresh(&s->bus, s->sim, s->instance, &s->events);
}

/* ---- Reset ------------------------------------------------------------------------------- */

static void reset_state(I8State *s) {
    s->phase = PH_MODE;
    s->command = su_reg(0, 0xFF);
    s->tx_full = 0;
    s->committed = 0;
    s->tsr = LOGIC_1;
    s->txrdy_hold = 0;
    s->sync_started = 0;
    s->sh_left = 0;
    s->sh_fill = 0;
    s->fill_next = 0;
    s->filling = 0;
    s->rxrdy = s->pe = s->oe = s->fe = LOGIC_0;
    s->rxrdy_hold = 0;
    s->syndet = LOGIC_0;
    s->low_ticks = 0;
    s->hunt = 0;
    s->synced = 0;
    s->sync2_wait = 0;
    s->rs_count = 0;
    s->pair_first = 0;
    serial_tx_reset(&s->line.tx);
    serial_rx_reset(&s->line.rx);
    update_format(s);
}

static void apply_reset(I8State *s) {
    LogicValue r = serial_level(s->sim, s->instance, s->pins->reset, LOGIC_0);

    if (r == LOGIC_X) {
        s->unknown = 1;
        s->in_reset = 0;
        s->bus.enabled = 0;
    } else if (r == LOGIC_1) {
        s->unknown = 0;
        s->in_reset = 1;
        s->bus.enabled = 0;
        reset_state(s);
        serial_line_sender_reset(&s->line, s->sim, s->instance);
    } else {
        if (s->unknown) {
            /* Out of an X reset: the control state is unknown. */
            reset_state(s);
            s->phase = PH_X;
            s->command = su_reg(0, 0);
            s->mode = su_reg(0, 0);
            update_format(s);
        }
        s->unknown = 0;
        s->in_reset = 0;
        s->bus.enabled = 1;
    }
    serial_bus_update(&s->bus, s->sim, s->instance, &s->events);
    refresh(s, 0);
}

/* ---- Control writes --------------------------------------------------------------------- */

static void enter_hunt(I8State *s) {
    unsigned n = char_length(s);

    s->hunt = 1;
    s->synced = 0;
    s->sync2_wait = 0;
    s->rs_count = 0;
    /* EH sets the used bits of the receive buffer to ones (p13). */
    s->rx_buf = su_reg((uint8_t)((1u << n) - 1), 0xFF);
}

static void write_command(I8State *s, uint8_t v, uint8_t k) {
    LogicValue ir = su_vk(v, k, 6);
    LogicValue was_en = cmd(s, 0);

    if (ir == LOGIC_1) {
        /* Internal reset: idle, mode format next (p3). */
        SuReg mode = s->mode;

        reset_state(s);
        s->mode = mode;
        update_format(s);
        return;
    }
    if (ir == LOGIC_X) {
        s->phase = PH_X;
        s->command = su_reg(0, 0);
        return;
    }
    s->command = su_reg(v, k);
    /* ER: reset PE, OE, FE. */
    if (su_bit(s->command, 4) == LOGIC_1) {
        s->pe = s->oe = s->fe = LOGIC_0;
    } else if (su_bit(s->command, 4) == LOGIC_X) {
        s->pe = s->oe = s->fe = LOGIC_X;
    }
    /* EH: sync only. */
    if (is_async(s) == 0 && su_bit(s->command, 7) == LOGIC_1) {
        enter_hunt(s);
    }
    /* A disable while busy: what was written before still goes out (p3, p5). */
    if (was_en == LOGIC_1 && cmd(s, 0) != LOGIC_1 && s->tx_full && tx_busy(s)) {
        s->committed = 1;
    }
    if (cmd(s, 2) == LOGIC_0) {
        s->rxrdy = LOGIC_0;   /* RxEnable off holds RxRDY reset */
    }
}

static void write_control(I8State *s, uint8_t v, uint8_t k) {
    switch (s->phase) {
        case PH_MODE:
            s->mode = su_reg(v, k);
            if ((k & 3u) != 3u) {
                s->phase = PH_X;
            } else if ((v & 3u) == 0) {
                s->phase = PH_SYNC1;
            } else {
                s->phase = PH_COMMAND;
            }
            s->command = su_reg(0, 0xFF);
            update_format(s);
            break;
        case PH_SYNC1:
            s->sync1 = su_reg(v, k);
            s->phase = (s->mode.k & 0x80u) == 0 ? PH_X : (s->mode.v & 0x80u) ? PH_COMMAND : PH_SYNC2;
            break;
        case PH_SYNC2:
            s->sync2 = su_reg(v, k);
            s->phase = PH_COMMAND;
            break;
        case PH_COMMAND:
            write_command(s, v, k);
            break;
        case PH_X:
            /* The state is unknown: an IR command would get out, but which
               register this write is cannot be told. */
            break;
    }
}

/* ---- Bus events ------------------------------------------------------------------------- */

static void bus_peek(void *context, unsigned unit, unsigned address, uint8_t *data, uint8_t *known) {
    I8State *s = context;

    (void)unit;
    if (s->unknown) {
        *data = 0;
        *known = 0;
        return;
    }
    if (address & 1u) {
        status_of(s, data, known);
    } else {
        *data = s->rx_buf.v;
        *known = s->rx_buf.k;
    }
}

static void bus_read_done(void *context, unsigned unit, unsigned address) {
    I8State *s = context;

    (void)unit;
    if (s->unknown) {
        return;
    }
    if (address & 1u) {
        /* A status read resets the SYNDET flip-flop (not BRKDET). */
        if (is_async(s) == 0) {
            s->syndet = LOGIC_0;
        }
    } else {
        s->rxrdy = LOGIC_0;
        s->rxrdy_hold = 0;
    }
    refresh(s, t_of(s, T_CLR));
}

static void bus_write(void *context, unsigned unit, unsigned address, uint8_t data, uint8_t known) {
    I8State *s = context;

    (void)unit;
    if (s->unknown) {
        return;
    }
    if (address & 1u) {
        write_control(s, data, known);
    } else {
        s->tx_buf = su_reg(data, known);
        s->tx_full = 1;
        s->committed = tx_enabled(s) == LOGIC_1;
        s->txrdy_hold = 0;
    }
    refresh(s, t_of(s, T_WC));
}

/* ---- Transmitter --------------------------------------------------------------------------- */

static int may_send(I8State *s) {
    return tx_enabled(s) == LOGIC_1 || s->committed;
}

static int tx_fetch(void *context, uint16_t *data, uint16_t *known) {
    I8State *s = context;

    if (!s->tx_full || !may_send(s)) {
        return 0;
    }
    *data = s->tx_buf.v;
    *known = s->tx_buf.k;
    s->tx_full = 0;
    s->committed = 0;
    return 1;
}

static void tx_done(void *context, uint16_t data, uint16_t known) {
    I8State *s = context;

    (void)known;
    serial_line_log(&s->line, data);
}

/* Loads the synchronous shift register: data, else a fill character. */
static int sync_load(I8State *s) {
    unsigned n = char_length(s);
    LogicValue par = LOGIC_0;
    uint16_t data;
    uint16_t known;
    unsigned i;
    unsigned ones = 0;

    if (s->tx_full && may_send(s)) {
        data = s->tx_buf.v;
        known = s->tx_buf.k;
        s->tx_full = 0;
        s->committed = 0;
        s->sh_fill = 0;
        s->fill_next = 0;
    } else if (s->sync_started && tx_enabled(s) == LOGIC_1) {
        /* The CPU is late: the sync character(s) fill (p11). */
        int dbl = !(s->mode.v & 0x80u);
        SuReg c = s->fill_next == 1 && dbl ? s->sync2 : s->sync1;

        data = c.v;
        known = c.k;
        s->sh_fill = 1;
        s->fill_next = dbl ? s->fill_next ^ 1u : 0u;
    } else {
        return 0;
    }
    data &= (uint16_t)((1u << n) - 1);
    known |= (uint16_t)~((1u << n) - 1);
    if (parity_on(s)) {
        if ((known & ((1u << n) - 1)) != (1u << n) - 1) {
            par = LOGIC_X;
        } else {
            for (i = 0; i < n; i++) {
                ones += (data >> i) & 1u;
            }
            /* EP = 1 even: the parity bit makes the ones even. */
            par = su_bool((s->mode.v & 0x20u) ? ones % 2 : ones % 2 == 0);
        }
        if (par == LOGIC_1) {
            data |= (uint16_t)(1u << n);
        }
        if (par == LOGIC_X) {
            known &= (uint16_t)~(1u << n);
        }
    }
    s->sh_data = data;
    s->sh_known = known;
    s->sh_bits = n + (parity_on(s) ? 1u : 0u);
    s->sh_left = s->sh_bits;
    s->sync_started = 1;
    return 1;
}

static void sync_tx_edge(I8State *s) {
    unsigned bit;

    if (s->sh_left == 0) {
        if (s->sync_started && s->sh_bits != 0) {
            serial_line_log(&s->line, (uint16_t)(s->sh_data & ((1u << char_length(s)) - 1)));
        }
        if (!sync_load(s)) {
            s->sync_started = 0;
            s->sh_bits = 0;
            s->tsr = LOGIC_1;   /* marking until the first character */
            return;
        }
    }
    bit = s->sh_bits - s->sh_left;
    s->tsr = su_vk(s->sh_data, s->sh_known, bit);
    s->sh_left--;
}

static void tx_edge(I8State *s) {
    SerialTxEvents events = {s, tx_fetch, NULL, tx_done};
    int async = is_async(s);

    if (s->in_reset || s->unknown) {
        return;
    }
    if (async == 1) {
        unsigned n = (s->mode.v & 3u) == 1 ? 2u : 1u;
        unsigned i;

        for (i = 0; i < n; i++) {
            s->tsr = serial_tx_tick(&s->line.tx, &s->format, &events);
        }
    } else if (async == 0) {
        sync_tx_edge(s);
    } else if (mode_known(s)) {
        s->tsr = LOGIC_X;
    }
    refresh(s, 0);
}

/* ---- Receiver ---------------------------------------------------------------------------------- */

static void load_char(I8State *s, uint16_t data, uint16_t known, LogicValue pe, LogicValue fe) {
    unsigned n = char_length(s);
    uint8_t mask = (uint8_t)((1u << n) - 1);

    if (cmd(s, 2) == LOGIC_1) {
        if (s->rxrdy != LOGIC_0) {
            s->oe = s->rxrdy == LOGIC_1 ? LOGIC_1 : su_or(s->oe, LOGIC_X);
        }
        s->rxrdy = LOGIC_1;
    } else if (cmd(s, 2) == LOGIC_X) {
        s->rxrdy = LOGIC_X;
    }
    /* Unused bits read 0 (p10). */
    s->rx_buf = su_reg((uint8_t)(data & mask), (uint8_t)((known & mask) | ~mask));
    if (pe != LOGIC_0) {
        s->pe = pe == LOGIC_1 ? LOGIC_1 : su_or(s->pe, LOGIC_X);
    }
    if (fe != LOGIC_0) {
        s->fe = fe == LOGIC_1 ? LOGIC_1 : su_or(s->fe, LOGIC_X);
    }
}

static void on_received(void *context, const SerialRxChar *c) {
    I8State *s = context;
    LogicValue pe = (c->flags_known & SERIAL_RX_PE) ? su_bool(c->flags & SERIAL_RX_PE) : LOGIC_X;
    LogicValue fe = (c->flags_known & SERIAL_RX_FE) ? su_bool(c->flags & SERIAL_RX_FE) : LOGIC_X;

    if (fe == LOGIC_1 && !(c->flags & SERIAL_RX_BI)) {
        s->resync = 1;
    }
    load_char(s, c->data, s->format.format_x ? 0 : c->known, pe, fe);
}

/* Bits of a sync character; 1 when the received character equals reg. */
static int sync_match(const I8State *s, SuReg reg) {
    unsigned n = char_length(s);
    uint16_t mask = (uint16_t)((1u << n) - 1);

    return (s->rs_known & mask) == mask && (reg.k & mask) == mask &&
           (s->rs_data & mask) == (reg.v & mask);
}

static LogicValue sync_parity_error(const I8State *s) {
    unsigned n = char_length(s);
    unsigned i;
    unsigned ones = 0;

    if (!parity_on(s)) {
        return LOGIC_0;
    }
    if ((s->rs_known & ((1u << (n + 1)) - 1)) != (1u << (n + 1)) - 1) {
        return LOGIC_X;
    }
    for (i = 0; i <= n; i++) {
        ones += (s->rs_data >> i) & 1u;
    }
    /* Even parity: data + parity has an even count of ones. */
    return su_bool((s->mode.v & 0x20u) ? ones % 2 != 0 : ones % 2 == 0);
}

static void sync_rx_edge(I8State *s, LogicValue level) {
    unsigned n = char_length(s) + (parity_on(s) ? 1u : 0u);
    int external = (s->mode.v & 0x40u) != 0;
    int single = (s->mode.v & 0x80u) != 0;
    uint16_t top = (uint16_t)(1u << (n - 1));

    if (!s->hunt && !s->synced) {
        return;   /* no character boundary yet (EH not given) */
    }
    /* Shift in at the top; after n bits, bit 0 is the first one received. */
    s->rs_data = (uint16_t)((s->rs_data >> 1) | (level == LOGIC_1 ? top : 0));
    s->rs_known = (uint16_t)((s->rs_known >> 1) | (level != LOGIC_X ? top : 0));
    if (s->hunt) {
        if (external) {
            return;   /* external sync: SYNDET (the input) ends the hunt */
        }
        if (s->sync2_wait) {
            if (++s->rs_count < n) {
                return;
            }
            s->sync2_wait = 0;
            if (sync_match(s, s->sync2)) {
                s->rs_count = 0;
                s->hunt = 0;
                s->synced = 1;
                s->syndet = LOGIC_1;
                return;
            }
            /* Not SYNC2: back to the bit-by-bit hunt, this window included. */
            s->rs_count = n;
        }
        if (s->rs_count < n) {
            s->rs_count++;   /* fill the register before comparing */
        }
        if (s->rs_count >= n && sync_match(s, s->sync1)) {
            s->rs_count = 0;
            if (single) {
                s->hunt = 0;
                s->synced = 1;
                s->syndet = LOGIC_1;
            } else {
                s->sync2_wait = 1;
            }
        }
        return;
    }
    if (++s->rs_count < n) {
        return;
    }
    s->rs_count = 0;
    /* A character boundary: load it; parity is checked regardless of RxE. */
    load_char(s, s->rs_data, s->rs_known, sync_parity_error(s), LOGIC_0);
    if (!external) {
        /* Out of hunt, sync detection still works at the boundaries. */
        if (single ? sync_match(s, s->sync1) : (s->pair_first && sync_match(s, s->sync2))) {
            s->syndet = LOGIC_1;
        }
        s->pair_first = !single && sync_match(s, s->sync1);
    }
}

static void rx_edge(I8State *s) {
    SerialRxEvents events = {s, on_received, NULL};
    LogicValue pin;
    LogicValue level;
    int async = is_async(s);

    if (s->in_reset || s->unknown) {
        return;
    }
    pin = serial_level(s->sim, s->instance, s->pins->pin[P_RX], LOGIC_X);
    if (async == 1) {
        unsigned n = (s->mode.v & 3u) == 1 ? 2u : 1u;
        unsigned i;

        for (i = 0; i < n; i++) {
            level = serial_line_rx_input(&s->line, s->sim, s->instance, pin, &s->format);
            /* BRKDET: low through two whole characters (p8). */
            if (level == LOGIC_0) {
                if (++s->low_ticks >= 2ul * serial_format_char_bits(&s->format) * s->format.factor) {
                    s->syndet = LOGIC_1;
                }
            } else if (level == LOGIC_1) {
                s->low_ticks = 0;
                s->syndet = LOGIC_0;
            }
            if (cmd(s, 2) != LOGIC_1) {
                serial_rx_reset(&s->line.rx);
                s->line.rx.last = level;
                continue;
            }
            s->resync = 0;
            serial_rx_tick(&s->line.rx, level, &s->format, &events);
            if (s->resync) {
                su_rx_no_resync(&s->line.rx, level);
            }
        }
    } else if (async == 0) {
        sync_rx_edge(s, pin);
    }
    refresh(s, 0);
}

static void syndet_input(I8State *s) {
    LogicValue v = serial_level(s->sim, s->instance, s->pins->pin[P_SYNDET], LOGIC_X);
    LogicValue was = s->syndet_in;

    s->syndet_in = v;
    if (!syndet_is_input(s) || s->in_reset || s->unknown) {
        return;
    }
    if (was != LOGIC_1 && v == LOGIC_1 && s->hunt) {
        /* External sync: out of hunt; assembly starts at the next RxC. */
        s->hunt = 0;
        s->synced = 1;
        s->rs_count = 0;
        s->syndet = LOGIC_1;
        refresh(s, 0);
    } else if (v == LOGIC_1) {
        s->syndet = LOGIC_1;
        refresh(s, 0);
    }
}

/* ---- Engine callbacks -------------------------------------------------------------------- */

static int falling(LogicValue *last, LogicValue now) {
    int f = *last == LOGIC_1 && now == LOGIC_0;

    *last = now;
    return f;
}

static int rising(LogicValue *last, LogicValue now) {
    int r = *last == LOGIC_0 && now == LOGIC_1;

    *last = now;
    return r;
}

static void i8_start(Sim *sim, SimInstance *instance, void *raw) {
    I8State *s = raw;

    s->sim = sim;
    s->instance = instance;
    s->txc_level = serial_level(sim, instance, s->pins->pin[P_TXC], LOGIC_X);
    s->rxc_level = serial_level(sim, instance, s->pins->pin[P_RXC], LOGIC_X);
    s->rd_level = serial_level(sim, instance, s->pins->bus.read, LOGIC_X);
    s->wr_level = serial_level(sim, instance, s->pins->bus.write, LOGIC_X);
    s->syndet_in = LOGIC_X;
    serial_bus_start(&s->bus, sim, instance, &s->events);
    apply_reset(s);
}

/* The leading edges of data strobes (TxRDY / RxRDY reset, p5, p7). */
static void strobe_edges(I8State *s, size_t pin) {
    LogicValue cs = serial_level(s->sim, s->instance, s->pins->bus.select[0], LOGIC_1);
    LogicValue cd = serial_level(s->sim, s->instance, s->pins->bus.address[0], LOGIC_X);
    int active = !s->in_reset && !s->unknown && cs == LOGIC_0 && cd == LOGIC_0;

    if (pin == s->pins->bus.write &&
        falling(&s->wr_level, serial_level(s->sim, s->instance, pin, LOGIC_X)) && active) {
        s->txrdy_hold = 1;
    }
    if (pin == s->pins->bus.read &&
        falling(&s->rd_level, serial_level(s->sim, s->instance, pin, LOGIC_X)) && active) {
        s->rxrdy_hold = 1;
    }
}

static void i8_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    I8State *s = raw;
    const size_t *p = s->pins->pin;

    (void)old_value;
    (void)new_value;
    s->sim = sim;
    s->instance = instance;
    if (pin == s->pins->reset) {
        apply_reset(s);
        return;
    }
    if (pin == p[P_TXC] && falling(&s->txc_level, serial_level(sim, instance, pin, LOGIC_X))) {
        tx_edge(s);
    }
    if (pin == p[P_RXC] && rising(&s->rxc_level, serial_level(sim, instance, pin, LOGIC_X))) {
        rx_edge(s);
    }
    if (pin == p[P_SYNDET]) {
        syndet_input(s);
    }
    if (serial_bus_has_pin(&s->bus, pin)) {
        strobe_edges(s, pin);
        serial_bus_update(&s->bus, sim, instance, &s->events);
        refresh(s, t_of(s, T_CLR));
    }
    if (pin == p[P_CTS] || pin == p[P_DSR]) {
        refresh(s, 0);
    }
}

/* ---- Terminal ------------------------------------------------------------------------------- */

static unsigned i8_channels(const void *raw) {
    (void)raw;
    return 1;
}

static SerialLine *i8_line(void *raw, unsigned channel) {
    (void)channel;
    return &((I8State *)raw)->line;
}

static int i8_async(const void *raw, unsigned channel) {
    const I8State *s = raw;

    (void)channel;
    return !s->unknown && is_async(s) == 1;
}

/* ---- Create / destroy --------------------------------------------------------------------------- */

static void *i8_create(const Part *part, const void *data, const void *pins,
                       const SerialGrade *grade, const void *options) {
    I8State *s = calloc(1, sizeof *s);
    size_t i;

    (void)options;
    if (s == NULL) {
        return NULL;
    }
    s->base.model = &I8251_MODEL;
    s->dev = data;
    s->pins = pins;
    s->grade = grade;
    s->o.count = part->pin_count;
    s->o.out = malloc(part->pin_count * sizeof *s->o.out);
    if (s->o.out == NULL) {
        free(s);
        return NULL;
    }
    for (i = 0; i < part->pin_count; i++) {
        s->o.out[i] = LOGIC_Z;
    }
    {
        SerialBusTiming timing = {grade->t[T_RD], grade->t[T_DF], 0, 0};

        serial_bus_init(&s->bus, &s->pins->bus, &timing);
    }
    s->events.context = s;
    s->events.select = NULL;
    s->events.peek = bus_peek;
    s->events.read_done = bus_read_done;
    s->events.write = bus_write;
    serial_line_init(&s->line, s->pins->pin[P_RX]);
    s->mode = su_reg(0, 0);
    s->sync1 = su_reg(0, 0);
    s->sync2 = su_reg(0, 0);
    s->rx_buf = su_reg(0, 0);
    s->tx_buf = su_reg(0, 0);
    reset_state(s);
    s->txc_level = s->rxc_level = s->rd_level = s->wr_level = LOGIC_X;
    return s;
}

static void i8_destroy(void *raw) {
    I8State *s = raw;

    serial_line_free(&s->line);
    free(s->o.out);
    free(s);
}

/* ---- Loader -------------------------------------------------------------------------------------- */

static void *i8_load_device(KindContext *ctx, const cJSON *json, unsigned *channels) {
    I8Device *d = kind_alloc(ctx, sizeof *d);
    const cJSON *bus;

    if (d == NULL) {
        return NULL;
    }
    bus = kind_member(ctx, json, "bus", 1);
    if (bus == NULL) {
        return NULL;
    }
    kind_push(ctx, "bus");
    if (kind_check_keys(ctx, bus, BUS_KEYS, NULL) != 0 ||
        su_load_names(ctx, bus, "data", 1, d->data, 8, 8, NULL) != 0) {
        return NULL;
    }
    {
        unsigned i;

        for (i = 0; i < B_COUNT; i++) {
            if (kind_string(ctx, bus, BUS_KEYS[i + 1], &d->bus[i]) != 0) {
                return NULL;
            }
        }
    }
    kind_pop(ctx);
    if (kind_string(ctx, json, "clock", &d->clock) != 0 ||
        su_load_pin_object(ctx, json, "serial", 1, SERIAL_KEYS, d->pin, 0xFFFu) != 0) {
        return NULL;
    }
    *channels = 1;
    return d;
}

static void *i8_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t pin_count) {
    const I8Device *d = raw;
    I8Pins *p = kind_alloc(ctx, sizeof *p);

    if (p == NULL ||
        su_bind_names(ctx, pins, pin_count, "bus.data", d->data, p->bus.data, 8) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.controlData", d->bus[B_CD], &p->bus.address[0]) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.read", d->bus[B_READ], &p->bus.read) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.write", d->bus[B_WRITE], &p->bus.write) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.select", d->bus[B_SELECT], &p->bus.select[0]) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.reset", d->bus[B_RESET], &p->reset) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "clock", d->clock, &p->clock) != 0 ||
        su_bind_object(ctx, pins, pin_count, "serial", SERIAL_KEYS, d->pin, p->pin) != 0) {
        return NULL;
    }
    p->bus.address_count = 1;
    p->bus.select_count = 1;
    p->bus.select_high_count = 0;
    p->bus.read_high = SERIAL_NO_PIN;
    p->bus.write_high = SERIAL_NO_PIN;
    p->bus.address_strobe = SERIAL_NO_PIN;
    p->bus.driver_disable = SERIAL_NO_PIN;
    p->bus.select_out = SERIAL_NO_PIN;
    return p;
}

const SerialModel I8251_MODEL = {
    "i8251", KEYS, TIMING, NULL,
    i8_load_device, i8_bind, NULL, NULL,
    i8_create, i8_destroy, i8_start, i8_pin_changed, NULL,
    i8_channels, i8_line, i8_async
};
