/*
 * i82510: the Intel 82510 asynchronous serial controller (plan-12-H section
 * 3.3; facts in plan-12-data/specs/classic-b.md section 82510,
 * decisions.md section 82510).
 *
 * Bus: Intel RD/WR with CS low (serial_bus.c); A2..A0 and the bank pointer
 * (GIR/BANK bits 6:5) pick one of the 35 registers of Table 9; DLAB (LCR
 * bit 7) switches bank 0 A0/A1 to BAL/BAH and bank 3 A0/A1 to BBL/BBH.
 * Addresses Table 9 leaves empty read X and ignore writes (P2).
 *
 * Clocks: CLK is counted. The strap sampled as RESET falls picks the mode
 * (RTS low: crystal, pin 8 is X2; else external clock, pin 8 is OUT2) and,
 * with an external clock, DTR low disables the divide by 2 (p8). The
 * system clock feeds BRGA and BRGB (16-bit, BRG or timer mode; BRGB can run
 * from BRGA's output or SCLK). TX and RX take their 16x clock from either
 * BRG, or a 1x clock from SCLK (CLCF). Wake-up defaults (Table 4) give the
 * 8250 path: BRGA / 2 -> BRGB / 5 -> 16x.
 *
 * Receiver: a voting filter over a 3/16 or 7/16 window (RMD.SWM) in front
 * of the line engine, the start check by majority or by all samples
 * (RMD.SSM), the DPLL that nudges the bit counter on each edge (unless
 * RMD.DPD), and the noisy-character flag; then control-character and
 * uLAN address recognition, the FIFO lock, the 1- or 4-deep RX FIFO with
 * per-character flags (RXF) and the overrun rule (the oldest goes).
 * Transmitter: the 4-deep TX FIFO (11 bits per entry), 5-9 bit characters,
 * software parity, quarter-bit stop lengths (Table 13), the manual,
 * semi-automatic and automatic CTS/RTS handshakes.
 *
 * Interrupts: six blocks with source enables (RIE, MIE, TMIE), GER, the
 * GSR, GIR's priority code and IPN, automatic or manual acknowledge (INT
 * low two system clocks), and the TX FIFO hold state (GIR read).
 */

#include <stdlib.h>
#include <string.h>

#include "serial_util.h"

enum { T_RLDV, T_RHDZ, T_TXD };
static const char *const TIMING[] = {"trldv", "trhdz", "ttxd", NULL};

static const char *const KEYS[] = {"bus", "clock", "serial", NULL};
static const char *const BUS_KEYS[] = {"data", "address", "read", "write", "select", "reset", NULL};
enum { K_IN, K_OUT, K_COUNT };
static const char *const CLOCK_KEYS[] = {"in", "out", NULL};
enum { P_TX, P_RX, P_INT, P_CTS, P_RTS, P_DTR, P_RI, P_DSR, P_DCD, P_COUNT };
static const char *const SERIAL_KEYS[] = {"tx", "rx", "int", "cts", "rts", "dtr", "ri", "dsr", "dcd",
                                          NULL};

enum { FIFO_MAX = 4 };

/* Register indices. */
enum {
    R_BAL, R_BAH, R_BBL, R_BBH, R_GER, R_LCR, R_MCR, R_ACR0, R_ACR1, R_FMD, R_TMD, R_IMD, R_RIE,
    R_RMD, R_CLCF, R_BACF, R_BBCF, R_PMD, R_MIE, R_TMIE, R_TXF, R_COUNT
};
static const uint8_t RESET_VALUE[R_COUNT] = {
    0x02, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x1E,
    0x00, 0x00, 0x04, 0x84, 0xFC, 0x0F, 0x00, 0x00
};

/* RST / LSR bits. */
enum { S_CRF = 0x80, S_PCRF = 0x40, S_BKT = 0x20, S_BKD = 0x10, S_FE = 0x08, S_PE = 0x04, S_OE = 0x02 };
/* RXF bits. */
enum { F_ROK = 0x40, F_RXN = 0x20, F_RPE = 0x10, F_ACR = 0x08, F_BKF = 0x04, F_RFE = 0x02, F_RND = 0x01 };

typedef struct {
    const char *data[8], *addr[3], *read, *write, *select, *reset;
    const char *clock[K_COUNT];
    const char *pin[P_COUNT];
} I5Device;

typedef struct {
    SerialBusPins bus;
    size_t reset;
    size_t clock[K_COUNT];
    size_t pin[P_COUNT];
} I5Pins;

typedef struct {
    uint16_t data, known;
    uint8_t flags, flags_known;
} I5RxChar;

typedef struct {
    uint16_t data, known;   /* 9 data bits */
    uint8_t txf;            /* bit 7 address marker, bit 6 software parity */
} I5TxChar;

/* A BRG / timer. */
typedef struct {
    SerialBaud baud;
    int timer_run;          /* timer mode: counting */
    uint32_t timer_count;
    LogicValue out;         /* TA / TB level */
    int pulse;              /* timer mode: the output pulse (one system clock) */
} I5Brg;

typedef struct {
    SerialBase base;
    const I5Device *dev;
    const I5Pins *pins;
    const SerialGrade *grade;
    Sim *sim;
    SimInstance *instance;
    SuOutputs o;
    SerialBus bus;
    SerialBusEvents events;
    SerialLine line;
    SerialFormat txf, rxf;

    int in_reset;
    int unknown;
    int crystal;            /* strap: crystal oscillator mode */
    int div2;               /* strap: system clock = CLK / 2 */
    LogicValue strap_x;     /* the strap read X */
    SuReg r[R_COUNT];
    unsigned bank;
    SuReg tmcr;             /* TMCR: the timer gates (bits 5:4) */
    /* Status. */
    uint8_t rst;            /* RST 7:1 (LSR 4:1 shares 4:1) */
    uint8_t rst_k;
    int rfir_ack;
    uint8_t msr_delta;
    LogicValue msr_level[4]; /* CTS DSR RI DCD complements (1 = active) */
    uint8_t tmst;           /* TBEx bit 1, TAEx bit 0 */
    /* FIFOs. */
    I5RxChar rx[FIFO_MAX];
    unsigned rx_count;
    I5TxChar tx[FIFO_MAX];
    unsigned tx_count;
    int rx_enabled, rx_locked, tx_enabled;
    int tx_hold_state;      /* the TX FIFO hold state (GIR read) */
    unsigned tx_stop_hold;  /* extra stop ticks (quarter-bit stop lengths) */
    LogicValue tsr;
    /* Interrupt pin. */
    int int_force_low;      /* system clocks of INT forced low */
    int int_latched;        /* manual acknowledge: INT held */
    /* Clocks. */
    LogicValue clk, sysclk, sclk;
    I5Brg brg[2];
    /* Receiver front end. */
    LogicValue raw[7];
    unsigned raw_at;
    LogicValue filtered;
    int noisy;
    int resync;
    int breaking;           /* RX saw a break, BkT on the first mark */
    int powered_down;
    int pd_sleep;
} I5State;

static SimTime t_of(const I5State *s, unsigned i) {
    return s->grade->t[i];
}

static uint8_t rv(const I5State *s, unsigned i) {
    return s->r[i].v;
}

/* ---- Formats ---------------------------------------------------------------------------- */

static unsigned char_length(const I5State *s) {
    if (rv(s, R_TMD) & 0x20u) {
        return 9;   /* NBCL */
    }
    return 5 + (rv(s, R_LCR) & 3u);
}

static int ulan(const I5State *s) {
    return (rv(s, R_IMD) >> 1) & 1u;
}

static int soft_parity(const I5State *s) {
    return (rv(s, R_TMD) & 0x04u) && (rv(s, R_LCR) & 0x08u) && !(rv(s, R_LCR) & 0x30u);
}

static int tx_1x(const I5State *s) {
    return (rv(s, R_CLCF) >> 5) & 1u;
}

static int rx_1x(const I5State *s) {
    return (rv(s, R_CLCF) >> 7) & 1u;
}

/* Stop length in quarter bits (16x), or whole bits x 4 (1x); 0 = undefined. */
static unsigned stop_quarters(const I5State *s) {
    unsigned code = ((rv(s, R_TMD) & 3u) << 1) | ((rv(s, R_LCR) >> 2) & 1u);
    static const unsigned Q16[8] = {4, 8, 3, 4, 5, 6, 7, 8};

    if (tx_1x(s)) {
        return code < 2 ? 0 : code == 7 ? 8 : 4;
    }
    if (code == 1) {
        return char_length(s) == 5 ? 6 : 8;
    }
    return Q16[code];
}

static void update_format(I5State *s) {
    SerialFormat f = serial_format_default();
    uint8_t lcr = rv(s, R_LCR);
    unsigned q = stop_quarters(s);

    f.data_bits = char_length(s) + (ulan(s) && char_length(s) < 9 ? 1u : 0u);
    if (!(lcr & 0x08u)) {
        f.parity = SERIAL_PARITY_NONE;
    } else if (soft_parity(s)) {
        f.parity = SERIAL_PARITY_MARK;   /* per character, from TXF.SP */
    } else {
        static const SerialParity P[4] = {
            SERIAL_PARITY_ODD, SERIAL_PARITY_EVEN, SERIAL_PARITY_MARK, SERIAL_PARITY_SPACE
        };

        f.parity = P[(lcr >> 4) & 3u];
    }
    f.factor = tx_1x(s) ? SU_ONE_X_FACTOR : 16;
    f.stop_halves = q / 2 > 0 ? q / 2 : 1;
    f.format_x = s->r[R_LCR].k != 0xFF || s->r[R_TMD].k != 0xFF || q == 0 ||
                 ((rv(s, R_TMD) >> 3) & 3u) == 1;
    s->txf = f;
    f.factor = rx_1x(s) ? SU_ONE_X_FACTOR : 16;
    f.stop_halves = 2;   /* the receiver takes any stop of 3/4 bit or more */
    f.format_x = s->r[R_LCR].k != 0xFF;
    s->rxf = f;
}

/* ---- Status, GSR, GIR -------------------------------------------------------------------- */

static unsigned rx_depth(const I5State *s) {
    return (rv(s, R_IMD) & 0x04u) ? 1u : FIFO_MAX;
}

static int rfir(const I5State *s) {
    unsigned rft = (rv(s, R_FMD) >> 4) & 3u;

    return s->rx_count > rft && !s->rfir_ack;
}

static int tfir(const I5State *s) {
    unsigned tft = rv(s, R_FMD) & 3u;

    return s->tx_count <= tft;
}

static int tx_idle(const I5State *s) {
    return !s->tx_enabled || (s->tx_count == 0 && !serial_tx_busy(&s->line.tx));
}

static uint8_t gsr_of(const I5State *s) {
    uint8_t g = 0;

    if (s->tmst & rv(s, R_TMIE) & 3u) g |= 0x20;
    if (tx_idle(s)) g |= 0x10;
    if (s->msr_delta & rv(s, R_MIE) & 0x0Fu) g |= 0x08;
    if (s->rst & rv(s, R_RIE) & 0xFEu) g |= 0x04;
    if (tfir(s) && !s->tx_hold_state) g |= 0x02;
    if (rfir(s)) g |= 0x01;
    return g;
}

/* The highest enabled pending block: 5 timer .. 0 modem; -1 none. */
static int gir_block(const I5State *s) {
    static const uint8_t BIT[6] = {0x20, 0x10, 0x04, 0x01, 0x02, 0x08};
    static const int CODE[6] = {5, 4, 3, 2, 1, 0};
    uint8_t pend = gsr_of(s) & rv(s, R_GER) & 0x3Fu;
    unsigned i;

    for (i = 0; i < 6; i++) {
        if (pend & BIT[i]) {
            return CODE[i];
        }
    }
    return -1;
}

static uint8_t gir_of(const I5State *s) {
    int b = gir_block(s);

    return (uint8_t)((s->bank << 5) | (b >= 0 ? (unsigned)b << 1 : 0u) | (b >= 0 ? 0u : 1u));
}

static int auto_ack(const I5State *s) {
    return (rv(s, R_IMD) & 0x08u) != 0;
}

static LogicValue int_level(const I5State *s) {
    int pending = gir_block(s) >= 0;

    if (s->int_force_low > 0) {
        return LOGIC_0;
    }
    if (!(rv(s, R_IMD) & 0x08u)) {
        return su_bool(s->int_latched || pending);   /* manual acknowledge */
    }
    return su_bool(pending);
}

static uint8_t lsr_of(const I5State *s) {
    uint8_t g = gsr_of(s);

    return (uint8_t)(((g & 0x10) ? 0x40 : 0) | ((g & 0x02) ? 0x20 : 0) | (s->rst & 0x1Eu) |
                     (rfir(s) ? 1 : 0));
}

/* ---- Pins -------------------------------------------------------------------------------- */

static int local_loop(const I5State *s) {
    return (rv(s, R_MCR) & 0x10u) && !(rv(s, R_IMD) & 0x01u);
}

static int remote_loop(const I5State *s) {
    return (rv(s, R_MCR) & 0x10u) && (rv(s, R_IMD) & 0x01u);
}

static int auto_echo(const I5State *s) {
    return !(rv(s, R_MCR) & 0x10u) && (rv(s, R_IMD) & 0x01u);
}

/* The modem inputs as MSR status (1 = active): CTS, DSR, RI, DCD. */
static void modem_now(const I5State *s, LogicValue now[4]) {
    uint8_t pmd = rv(s, R_PMD);
    uint8_t mcr = rv(s, R_MCR);

    if (local_loop(s)) {
        /* OUT1 -> RI, OUT2 -> DCD, DTR -> DSR, RTS -> CTS. */
        now[0] = su_bool(mcr & 0x02u);
        now[1] = su_bool(mcr & 0x01u);
        now[2] = su_bool(mcr & 0x04u);
        now[3] = su_bool(mcr & 0x08u);
        return;
    }
    now[0] = su_not(serial_level(s->sim, s->instance, s->pins->pin[P_CTS], LOGIC_X));
    now[1] = (pmd & 0x20u) ? su_not(serial_level(s->sim, s->instance, s->pins->pin[P_DSR], LOGIC_X))
                           : LOGIC_X;
    now[2] = (pmd & 0x08u) ? su_not(serial_level(s->sim, s->instance, s->pins->pin[P_RI], LOGIC_X))
                           : LOGIC_X;
    now[3] = (pmd & 0x80u) ? su_not(serial_level(s->sim, s->instance, s->pins->pin[P_DCD], LOGIC_X))
                           : LOGIC_X;
}

static void track_modem(I5State *s) {
    LogicValue now[4];
    static const uint8_t DELTA[4] = {0x01, 0x02, 0x04, 0x08};
    unsigned i;

    modem_now(s, now);
    for (i = 0; i < 4; i++) {
        LogicValue was = s->msr_level[i];

        if (was != LOGIC_X && now[i] != LOGIC_X && was != now[i]) {
            /* DRI: the RI input high to low (status 0 -> 1). */
            if (i != 2 || now[i] == LOGIC_1) {
                s->msr_delta |= DELTA[i];
            }
        }
        s->msr_level[i] = now[i];
    }
}

static uint8_t msr_of(const I5State *s, uint8_t *k) {
    uint8_t v = s->msr_delta & 0x0Fu;
    static const unsigned BIT[4] = {4, 5, 6, 7};
    unsigned i;

    *k = 0x0F;
    for (i = 0; i < 4; i++) {
        if (s->msr_level[i] == LOGIC_1) v |= (uint8_t)(1u << BIT[i]);
        if (s->msr_level[i] != LOGIC_X) *k |= (uint8_t)(1u << BIT[i]);
    }
    return v;
}

static int handshake_rts(const I5State *s) {
    unsigned tm = (rv(s, R_TMD) >> 3) & 3u;

    if (tm == 2) {
        return s->tx_enabled;
    }
    if (tm == 3) {
        return s->tx_enabled && (s->tx_count > 0 || serial_tx_busy(&s->line.tx));
    }
    return 0;
}

static LogicValue txd_level(const I5State *s) {
    if (remote_loop(s)) {
        return serial_level(s->sim, s->instance, s->pins->pin[P_RX], LOGIC_X);
    }
    if (local_loop(s)) {
        return LOGIC_1;
    }
    if (rv(s, R_LCR) & 0x40u) {
        return LOGIC_0;   /* SBK */
    }
    return s->tsr;
}

static void update_outputs(I5State *s) {
    const size_t *p = s->pins->pin;
    uint8_t mcr = rv(s, R_MCR);
    uint8_t pmd = rv(s, R_PMD);
    int loop = local_loop(s);

    if (s->unknown || s->strap_x == LOGIC_X) {
        static const unsigned OUTS[] = {P_TX, P_INT, P_RTS, P_DTR};
        unsigned i;

        for (i = 0; i < sizeof OUTS / sizeof OUTS[0]; i++) {
            su_drive(&s->o, s->sim, s->instance, p[OUTS[i]], LOGIC_X, 0);
        }
        return;
    }
    if (s->in_reset) {
        /* RTS and DTR are strap inputs with weak pull-ups during RESET. */
        su_drive(&s->o, s->sim, s->instance, p[P_TX], LOGIC_1, 0);
        su_drive(&s->o, s->sim, s->instance, p[P_INT], LOGIC_0, 0);
        return;
    }
    su_drive(&s->o, s->sim, s->instance, p[P_TX], txd_level(s), t_of(s, T_TXD));
    if (!(auto_ack(s)) && gir_block(s) >= 0) {
        s->int_latched = 1;   /* manual acknowledge: INT holds until ICM.INTA */
    }
    su_drive(&s->o, s->sim, s->instance, p[P_INT], int_level(s), 0);
    /* Modem outputs (1 in MCR = pin low); inactive in local loopback (P4). */
    su_drive(&s->o, s->sim, s->instance, p[P_RTS],
             loop ? LOGIC_1 : su_bool(!((mcr & 0x02u) || handshake_rts(s))), 0);
    if (pmd & 0x04u) {
        su_drive(&s->o, s->sim, s->instance, p[P_DTR], loop ? LOGIC_1 : su_bool(!(mcr & 0x01u)), 0);
    } else {
        su_drive_clock(&s->o, s->sim, s->instance, p[P_DTR], s->brg[1].out, 0);   /* TB */
    }
    if (pmd & 0x20u) {
        su_drive(&s->o, s->sim, s->instance, p[P_DSR], LOGIC_Z, 0);
    } else if (pmd & 0x10u) {
        su_drive(&s->o, s->sim, s->instance, p[P_DSR], loop ? LOGIC_1 : su_bool(!(mcr & 0x20u)), 0);
    } else {
        su_drive_clock(&s->o, s->sim, s->instance, p[P_DSR], s->brg[0].out, 0);   /* TA */
    }
    if (pmd & 0x80u) {
        su_drive(&s->o, s->sim, s->instance, p[P_DCD], LOGIC_Z, 0);
    } else if (pmd & 0x40u) {
        su_drive(&s->o, s->sim, s->instance, p[P_DCD], loop ? LOGIC_1 : su_bool(!(mcr & 0x04u)), 0);
    } else {
        su_drive_clock(&s->o, s->sim, s->instance, p[P_DCD], s->sysclk, 0);   /* ICLK */
    }
    if (!s->crystal) {
        su_drive(&s->o, s->sim, s->instance, s->pins->clock[K_OUT],
                 loop ? LOGIC_1 : su_bool(!(mcr & 0x08u)), 0);   /* OUT2 */
    }
}

static void refresh(I5State *s) {
    track_modem(s);
    update_outputs(s);
    serial_bus_refresh(&s->bus, s->sim, s->instance, &s->events);
}

/* A service operation (automatic acknowledge): INT low two system clocks. */
static void serviced(I5State *s) {
    if (rv(s, R_IMD) & 0x08u) {
        s->int_force_low = 2;
    }
}

/* ---- BRGs and timers ---------------------------------------------------------------------- */

static uint32_t divisor_of(const I5State *s, unsigned which) {
    unsigned lo = which == 0 ? R_BAL : R_BBL;

    return (uint32_t)rv(s, lo) | ((uint32_t)rv(s, lo + 1) << 8);
}

static int brg_mode(const I5State *s, unsigned which) {
    return (rv(s, which == 0 ? R_BACF : R_BBCF) >> 2) & 1u;
}

static void brg_load(I5State *s, unsigned which) {
    unsigned lo = which == 0 ? R_BAL : R_BBL;
    uint32_t d = divisor_of(s, which);
    int known = s->r[lo].k == 0xFF && s->r[lo + 1].k == 0xFF;

    serial_baud_load(&s->brg[which].baud, d, known);   /* divisor 0: forbidden, stopped */
    s->brg[which].timer_run = 0;
    s->brg[which].out = brg_mode(s, which) ? s->brg[which].baud.baudout : LOGIC_0;
}

static void serial_tick_tx(I5State *s);
static void serial_tick_rx(I5State *s);

/* One input edge to a BRG; returns 1 on its output rising edge (a tick). */
static int brg_edge(I5State *s, unsigned which, int rising, LogicValue level) {
    I5Brg *g = &s->brg[which];
    int tick;

    if (!brg_mode(s, which)) {
        return 0;
    }
    tick = serial_baud_edge(&g->baud, rising, level);
    g->out = g->baud.baudout;
    return tick;
}

/* A system clock edge on a BRG in timer mode. */
static void timer_clock(I5State *s, unsigned which) {
    I5Brg *g = &s->brg[which];
    uint8_t tmcr_gate = which == 0 ? 0x10 : 0x20;

    g->pulse = 0;
    if (brg_mode(s, which) || !g->timer_run || !(s->tmcr.v & tmcr_gate)) {
        g->out = LOGIC_0;
        return;
    }
    if (g->timer_count > 1) {
        g->timer_count--;
    }
    if (g->timer_count <= 1) {
        s->tmst |= which == 0 ? 0x01 : 0x02;
        g->timer_run = 0;
        g->pulse = 1;
    }
    g->out = su_bool(g->pulse);
}

/* The 16x / 1x clocks a BRG tick feeds. */
static void brg_ticked(I5State *s, unsigned which) {
    uint8_t clcf = rv(s, R_CLCF);
    unsigned tx_src = (clcf >> 4) & 1u ? 0u : 1u;   /* TxCS: 1 BRGA, 0 BRGB */
    unsigned rx_src = (clcf >> 6) & 1u ? 0u : 1u;

    if (!tx_1x(s) && tx_src == which) {
        serial_tick_tx(s);
    }
    if (!rx_1x(s) && rx_src == which) {
        serial_tick_rx(s);
    }
}

static void sysclk_edge(I5State *s, LogicValue level) {
    int rising = s->sysclk == LOGIC_0 && level == LOGIC_1;
    uint8_t bacf = rv(s, R_BACF);
    uint8_t bbcf = rv(s, R_BBCF);
    unsigned i;

    s->sysclk = level;
    if (s->in_reset || s->unknown || s->powered_down) {
        return;
    }
    if (rising) {
        if (s->int_force_low > 0) {
            s->int_force_low--;
        }
        for (i = 0; i < 2; i++) {
            timer_clock(s, i);
        }
    }
    /* BRGA from the system clock (BACS = 0). */
    if (!(bacf & 0x40u) && brg_edge(s, 0, rising, level)) {
        brg_ticked(s, 0);
        if (((bbcf >> 6) & 3u) == 2u && brg_edge(s, 1, 1, LOGIC_1)) {
            brg_ticked(s, 1);
        }
    } else if (!(bacf & 0x40u) && !rising && ((bbcf >> 6) & 3u) == 2u) {
        brg_edge(s, 1, 0, LOGIC_0);   /* BRGA's output fell: BRGB's input */
    }
    if (((bbcf >> 6) & 3u) == 0u && brg_edge(s, 1, rising, level)) {
        brg_ticked(s, 1);
    }
    update_outputs(s);
}

static void sclk_edge(I5State *s, LogicValue level) {
    int rising = s->sclk == LOGIC_0 && level == LOGIC_1;
    int falling = s->sclk == LOGIC_1 && level == LOGIC_0;
    uint8_t bacf = rv(s, R_BACF);
    uint8_t bbcf = rv(s, R_BBCF);

    s->sclk = level;
    if (s->in_reset || s->unknown || s->powered_down || (!rising && !falling) ||
        (rv(s, R_PMD) & 0x08u)) {
        return;   /* the pin is RI, not SCLK */
    }
    if ((bacf & 0x40u) && brg_edge(s, 0, rising, level)) {
        brg_ticked(s, 0);
    }
    if (((bbcf >> 6) & 3u) == 1u && brg_edge(s, 1, rising, level)) {
        brg_ticked(s, 1);
    }
    /* 1x clocks: TXD changes after SCLK falls, RXD is sampled as it rises (p40). */
    if (tx_1x(s) && falling) {
        serial_tick_tx(s);
        serial_tick_tx(s);
    }
    if (rx_1x(s) && rising) {
        serial_tick_rx(s);
        serial_tick_rx(s);
    }
    refresh(s);
}

/* ---- Transmitter ------------------------------------------------------------------------- */

static int cts_ok(const I5State *s) {
    unsigned tm = (rv(s, R_TMD) >> 3) & 3u;

    if (tm < 2) {
        return 1;   /* manual: CTS is ignored by the transmitter */
    }
    return s->msr_level[0] == LOGIC_1;
}

static int tx_fetch(void *context, uint16_t *data, uint16_t *known) {
    I5State *s = context;
    I5TxChar c;
    unsigned n = char_length(s);

    if (s->tx_count == 0 || !s->tx_enabled || s->tx_stop_hold > 0 || !cts_ok(s) || remote_loop(s)) {
        return 0;
    }
    c = s->tx[0];
    memmove(&s->tx[0], &s->tx[1], (s->tx_count - 1) * sizeof s->tx[0]);
    s->tx_count--;
    *data = c.data;
    *known = c.known;
    if (ulan(s) && n < 9) {
        /* uLAN: the address marker is the bit after the data. */
        *data = (uint16_t)((*data & ((1u << n) - 1)) | ((c.txf & 0x80u) ? 1u << n : 0));
        *known |= (uint16_t)(1u << n);
    }
    if (soft_parity(s)) {
        s->txf.parity = (c.txf & 0x40u) ? SERIAL_PARITY_MARK : SERIAL_PARITY_SPACE;
    }
    return 1;
}

static void tx_done(void *context, uint16_t data, uint16_t known) {
    I5State *s = context;
    unsigned q = stop_quarters(s);

    (void)known;
    serial_line_log(&s->line, data);
    /* Odd quarter stops: the engine sends whole halves; the last quarter
       is held as mark before the next start. */
    if (!tx_1x(s) && q % 2 == 1) {
        s->tx_stop_hold = 4;
    }
}

static void echo_drain(I5State *s);

static void serial_tick_tx(I5State *s) {
    SerialTxEvents events = {s, tx_fetch, NULL, tx_done};
    unsigned before = s->tx_count;

    if (s->in_reset || s->unknown || s->powered_down) {
        return;
    }
    if (s->tx_stop_hold > 0) {
        s->tx_stop_hold--;
    }
    s->tsr = serial_tx_tick(&s->line.tx, &s->txf, &events);
    if (s->tx_count != before) {
        echo_drain(s);
    }
}

/* ---- Receiver ---------------------------------------------------------------------------- */

static int is_control(const I5State *s, unsigned c) {
    unsigned mode = ((rv(s, R_RMD) >> 6) & 1u) << 1 | ((rv(s, R_RMD) >> 7) & 1u);

    if (mode == 1) {
        return c <= 0x1F || c == 0x7F;   /* ASCII */
    }
    if (mode == 3) {
        return c <= 0x3F;   /* EBCDIC */
    }
    return 0;
}

static void rx_push(I5State *s, const I5RxChar *c) {
    unsigned depth = rx_depth(s);

    if (s->rx_locked) {
        return;   /* locked: discarded, status still reported */
    }
    if (s->rx_count >= depth) {
        /* Overrun: the oldest character goes, the new one comes in. */
        memmove(&s->rx[0], &s->rx[1], (depth - 1) * sizeof s->rx[0]);
        s->rx_count = depth - 1;
        s->rst |= S_OE;
    }
    s->rx[s->rx_count++] = *c;
    s->rfir_ack = 0;
}

static void echo_drain(I5State *s) {
    if (!auto_echo(s)) {
        return;
    }
    /* Automatic echo: characters leave the RX FIFO into the TX FIFO. */
    while (s->rx_count > 0 && s->tx_count < FIFO_MAX) {
        I5RxChar c = s->rx[0];
        int errors = !(c.flags & F_ROK);
        int control = (c.flags & F_ACR) != 0;

        memmove(&s->rx[0], &s->rx[1], (s->rx_count - 1) * sizeof s->rx[0]);
        s->rx_count--;
        if ((errors && (rv(s, R_TMD) & 0x80u)) || (control && (rv(s, R_TMD) & 0x40u))) {
            continue;   /* EED / CED: not echoed */
        }
        s->tx[s->tx_count].data = c.data & 0x1FFu;
        s->tx[s->tx_count].known = c.known;
        s->tx[s->tx_count].txf = (uint8_t)((c.flags & F_ACR) && ulan(s) ? 0x80 : 0);
        s->tx_count++;
    }
}

static void on_received(void *context, const SerialRxChar *rc) {
    I5State *s = context;
    unsigned n = char_length(s);
    unsigned bits = s->rxf.data_bits;
    uint16_t mask = (uint16_t)((1u << bits) - 1);
    uint16_t data = rc->data & mask;
    I5RxChar c;
    int pe = (rc->flags & SERIAL_RX_PE) != 0;
    int fe = (rc->flags & SERIAL_RX_FE) != 0;
    int brk = (rc->flags & SERIAL_RX_BI) != 0;
    int addr = ulan(s) && n < 9 && ((data >> n) & 1u);
    unsigned v8 = data & 0xFFu;
    unsigned mode = ((rv(s, R_RMD) >> 6) & 1u) << 1 | ((rv(s, R_RMD) >> 7) & 1u);
    int match = 0;

    memset(&c, 0, sizeof c);
    if (fe && !brk) {
        s->resync = 1;
    }
    c.data = ulan(s) && n < 9 ? (uint16_t)(data & ((1u << n) - 1)) : data;
    c.known = s->rxf.format_x ? 0 : (uint16_t)(rc->known | ~mask);
    if (soft_parity(s)) {
        if (!pe) c.flags |= F_RPE;   /* the received parity bit as is (mark = 1) */
        pe = 0;
    } else if (pe) {
        c.flags |= F_RPE;
    }
    if (rc->flags & SERIAL_RX_FE) c.flags |= F_RFE;
    if (brk) c.flags |= F_BKF | F_RFE;
    if (s->noisy && !rx_1x(s)) c.flags |= F_RXN;
    if (n == 9) c.flags |= (uint8_t)((data >> 8) & 1u);   /* RND */
    if (!pe && !fe) c.flags |= F_ROK;
    c.flags_known = 0xFF;
    /* Programmed characters (ACR0/ACR1) and the standard control sets. */
    if (!ulan(s) || addr) {
        uint8_t cmp = (uint8_t)(c.data & 0xFFu);

        match = cmp == rv(s, R_ACR0) || cmp == rv(s, R_ACR1);
    }
    if (ulan(s)) {
        if (addr) {
            c.flags |= F_ACR;
            s->rst |= S_CRF;
            if (match) {
                s->rst |= S_PCRF;
            }
            /* Semi-automatic: any address opens; automatic: a match opens,
               a mismatch locks. */
            if (mode == 1) {
                s->rx_locked = 0;
            } else if (mode == 2) {
                s->rx_locked = !match;
            }
        }
    } else {
        if (match) {
            c.flags |= F_ACR;
            s->rst |= S_PCRF;
        }
        if (is_control(s, v8)) {
            c.flags |= F_ACR;
            s->rst |= S_CRF;
        }
    }
    if (pe) s->rst |= S_PE;
    if (fe) s->rst |= S_FE;
    if (brk) {
        s->rst |= S_BKD | S_FE;
        s->breaking = 1;
        c.data = 0;
    }
    if (s->rx_enabled) {
        rx_push(s, &c);
        echo_drain(s);
    }
    s->noisy = 0;
}

static LogicValue window_vote(const I5State *s, unsigned w) {
    unsigned ones = 0;
    unsigned zeros = 0;
    unsigned i;

    for (i = 0; i < w; i++) {
        LogicValue v = s->raw[(s->raw_at + 7 - 1 - i) % 7];

        ones += v == LOGIC_1;
        zeros += v == LOGIC_0;
    }
    if (ones * 2 > w) return LOGIC_1;
    if (zeros * 2 > w) return LOGIC_0;
    return LOGIC_X;
}

static int window_same(const I5State *s, unsigned w, LogicValue v) {
    unsigned i;

    for (i = 0; i < w; i++) {
        if (s->raw[(s->raw_at + 7 - 1 - i) % 7] != v) {
            return 0;
        }
    }
    return 1;
}

static void serial_tick_rx(I5State *s) {
    SerialRxEvents events = {s, on_received, NULL};
    LogicValue pin = serial_level(s->sim, s->instance, s->pins->pin[P_RX], LOGIC_X);
    LogicValue level = serial_line_rx_input(&s->line, s->sim, s->instance, pin, &s->rxf);
    SerialRx *rx = &s->line.rx;
    SerialRxPhase before = rx->phase;
    unsigned bit_before = rx->bit;
    unsigned w = (rv(s, R_RMD) & 0x10u) ? 7u : 3u;
    LogicValue in;

    if (s->in_reset || s->unknown || s->powered_down) {
        return;
    }
    if (local_loop(s)) {
        level = s->tsr;
    }
    if (remote_loop(s)) {
        return;   /* nothing reaches the receiver */
    }
    if (rx_1x(s)) {
        in = level;
    } else {
        /* The voting filter: the majority of the window. */
        s->raw[s->raw_at] = level;
        s->raw_at = (s->raw_at + 1) % 7;
        in = window_vote(s, w);
        /* The DPLL: an edge inside a character moves the bit counter one
           tick toward the bit boundary (count 8). */
        if (!(rv(s, R_RMD) & 0x20u) && in != s->filtered && in != LOGIC_X &&
            s->filtered != LOGIC_X &&
            (rx->phase == SERIAL_RX_DATA || rx->phase == SERIAL_RX_PARITY ||
             rx->phase == SERIAL_RX_STOP)) {
            if (rx->count < 8 && rx->count > 0) {
                rx->count++;
            } else if (rx->count > 8) {
                rx->count--;
            }
        }
        s->filtered = in;
    }
    if (s->breaking && in == LOGIC_1) {
        s->breaking = 0;
        s->rst |= S_BKT;   /* break terminated at the first mark */
    }
    s->resync = 0;
    serial_rx_tick(rx, in, &s->rxf, &events);
    if (!rx_1x(s)) {
        /* The start check: all samples 0 when RMD.SSM = 1 (else majority). */
        if (before == SERIAL_RX_START && rx->phase == SERIAL_RX_DATA && (rv(s, R_RMD) & 0x08u) &&
            !window_same(s, w, LOGIC_0)) {
            su_rx_no_resync(rx, in);
        }
        /* The noisy flag: samples that differ in a bit's window. */
        if ((rx->phase != before || rx->bit != bit_before) && before != SERIAL_RX_IDLE &&
            before != SERIAL_RX_START && !window_same(s, w, s->raw[(s->raw_at + 6) % 7])) {
            s->noisy = 1;
        }
    }
    if (s->resync) {
        su_rx_no_resync(rx, in);
    }
}

/* ---- Registers -------------------------------------------------------------------------- */

static int dlab(const I5State *s) {
    return (rv(s, R_LCR) >> 7) & 1u;
}

static void reset_regs(I5State *s) {
    unsigned i;

    for (i = 0; i < R_COUNT; i++) {
        s->r[i] = su_reg(RESET_VALUE[i], 0xFF);
    }
    s->bank = 0;
    s->tmcr = su_reg(0x30, 0xFF);   /* TMCR: TMST resets to 30H, so the gates are open */
    s->rst = 0;
    s->rst_k = 0xFF;
    s->rfir_ack = 0;
    s->msr_delta = 0;
    s->tmst = 0;
    s->rx_count = 0;
    s->tx_count = 0;
    s->rx_enabled = 1;
    s->rx_locked = 0;
    s->tx_enabled = 1;
    s->tx_hold_state = 0;
    s->tx_stop_hold = 0;
    s->tsr = LOGIC_1;
    s->int_force_low = 0;
    s->int_latched = 0;
    s->noisy = 0;
    s->breaking = 0;
    s->powered_down = 0;
    s->pd_sleep = 0;
    for (i = 0; i < 7; i++) {
        s->raw[i] = LOGIC_1;
    }
    s->filtered = LOGIC_1;
    serial_tx_reset(&s->line.tx);
    serial_rx_reset(&s->line.rx);
    brg_load(s, 0);
    brg_load(s, 1);
    update_format(s);
}

static void reg_peek(I5State *s, unsigned a, uint8_t *v, uint8_t *k) {
    unsigned b = s->bank;

    *k = 0xFF;
    *v = 0;
    if (a == 2) {
        *v = gir_of(s);
        return;
    }
    switch (b) {
        case 0:
            switch (a) {
                case 0:
                    if (dlab(s)) { *v = rv(s, R_BAL); *k = s->r[R_BAL].k; }
                    else if (s->rx_count > 0) { *v = (uint8_t)s->rx[0].data; *k = (uint8_t)s->rx[0].known; }
                    else { *k = 0; }
                    return;
                case 1:
                    if (dlab(s)) { *v = rv(s, R_BAH); *k = s->r[R_BAH].k; }
                    else { *v = rv(s, R_GER) & 0x3Fu; *k = s->r[R_GER].k; }
                    return;
                case 3: *v = rv(s, R_LCR); *k = s->r[R_LCR].k; return;
                case 4: *v = rv(s, R_MCR) & 0x3Fu; *k = s->r[R_MCR].k; return;
                case 5: *v = lsr_of(s); return;
                case 6: *v = msr_of(s, k); return;
                default: *v = rv(s, R_ACR0); *k = s->r[R_ACR0].k; return;
            }
        case 1:
            switch (a) {
                case 0:
                    if (s->rx_count > 0) { *v = (uint8_t)s->rx[0].data; *k = (uint8_t)s->rx[0].known; }
                    else { *k = 0; }
                    return;
                case 1:
                    if (s->rx_count > 0) { *v = s->rx[0].flags & 0x7Fu; }   /* bit 7 unused: 0 */
                    else { *k = 0; }
                    return;
                case 3: {
                    uint8_t gates = s->tmcr.v & 0x30u;

                    *v = (uint8_t)(gates | (s->tmst & 3u));
                    return;
                }
                case 4:
                    if (s->powered_down) { *k = 0; return; }   /* invalid on exit */
                    *v = (uint8_t)(((s->rx_count & 7u) << 4) | (s->tx_count & 7u));
                    return;
                case 5: *v = (uint8_t)((s->rst & 0xFEu) | (rfir(s) ? 1 : 0)); return;
                case 6: *v = msr_of(s, k); return;
                default: *v = gsr_of(s); return;
            }
        case 2:
            switch (a) {
                case 1: *v = rv(s, R_FMD); *k = s->r[R_FMD].k; return;
                case 3: *v = rv(s, R_TMD); *k = s->r[R_TMD].k; return;
                case 4: *v = rv(s, R_IMD) & 0x0Fu; *k = s->r[R_IMD].k; return;
                case 5: *v = rv(s, R_ACR1); *k = s->r[R_ACR1].k; return;
                case 6: *v = rv(s, R_RIE); *k = s->r[R_RIE].k; return;
                case 7: *v = rv(s, R_RMD); *k = s->r[R_RMD].k; return;
                default: *k = 0; return;   /* bank 2 A0: not in Table 9 */
            }
        default:
            switch (a) {
                case 0:
                    if (dlab(s)) { *v = rv(s, R_BBL); *k = s->r[R_BBL].k; }
                    else { *v = rv(s, R_CLCF); *k = s->r[R_CLCF].k; }
                    return;
                case 1:
                    if (dlab(s)) { *v = rv(s, R_BBH); *k = s->r[R_BBH].k; }
                    else { *v = rv(s, R_BACF); *k = s->r[R_BACF].k; }
                    return;
                case 3: *v = rv(s, R_BBCF); *k = s->r[R_BBCF].k; return;
                case 4: *v = rv(s, R_PMD); *k = s->r[R_PMD].k; return;
                case 5: *v = rv(s, R_MIE); *k = s->r[R_MIE].k; return;
                case 6: *v = rv(s, R_TMIE); *k = s->r[R_TMIE].k; return;
                default: *k = 0; return;   /* bank 3 A7: not in Table 9 */
            }
    }
}

static void rx_pop(I5State *s) {
    if (s->rx_count > 0) {
        memmove(&s->rx[0], &s->rx[1], (s->rx_count - 1) * sizeof s->rx[0]);
        s->rx_count--;
        serviced(s);
    }
}

static void read_done(I5State *s, unsigned a) {
    unsigned b = s->bank;

    if (a == 2) {
        /* A GIR read reporting the TX FIFO enters the hold state (p29). */
        if (gir_block(s) == 1) {
            s->tx_hold_state = 1;
            serviced(s);
        }
        return;
    }
    if ((b == 0 && a == 0 && !dlab(s)) || (b == 1 && a == 0)) {
        rx_pop(s);
    } else if ((b == 0 || b == 1) && a == 5) {
        /* LSR / RST read: RST 7:1 and LSR 4:1 clear. */
        s->rst = 0;
        serviced(s);
    } else if ((b == 0 || b == 1) && a == 6) {
        s->msr_delta = 0;
        serviced(s);
    } else if (b == 1 && a == 3) {
        s->tmst &= (uint8_t)~3u;
        serviced(s);
    } else if (b == 1 && a == 4 && s->powered_down) {
        /* Reading FLR leaves power down (idle: registers kept). */
        s->powered_down = 0;
    }
}

static void software_reset(I5State *s) {
    reset_regs(s);
}

/* TXD: the byte and the current TXF go into the TX FIFO. */
static void push_txd(I5State *s, uint8_t v, uint8_t k) {
    if (s->tx_count < FIFO_MAX) {
        s->tx[s->tx_count].data = (uint16_t)(v | ((rv(s, R_TXF) & 0x20u) ? 0x100u : 0));
        s->tx[s->tx_count].known = (uint16_t)(k | 0x100u);
        s->tx[s->tx_count].txf = rv(s, R_TXF);
        s->tx_count++;
    }   /* a fifth character is dropped, no error (p9) */
    s->tx_hold_state = 0;
    serviced(s);
}

static void write_reg(I5State *s, unsigned a, uint8_t v, uint8_t k) {
    unsigned b = s->bank;

    if (a == 2) {
        /* BANK: only bits 6:5 change. */
        if ((k & 0x60u) == 0x60u) {
            s->bank = (v >> 5) & 3u;
        }
        return;
    }
    switch (b) {
        case 0:
            switch (a) {
                case 0:
                    if (dlab(s)) { s->r[R_BAL] = su_reg(v, k); brg_load(s, 0); }
                    else { push_txd(s, v, k); }
                    return;
                case 1:
                    if (dlab(s)) { s->r[R_BAH] = su_reg(v, k); brg_load(s, 0); }
                    else { s->r[R_GER] = su_reg(v & 0x3Fu, k); s->tx_hold_state &= (v >> 1) & 1u; }
                    return;
                case 3: s->r[R_LCR] = su_reg(v, k); update_format(s); return;
                case 4: s->r[R_MCR] = su_reg(v & 0x3Fu, k); return;
                case 5:
                    /* Writing 0 clears LSR bits (bit 0: the RX FIFO interrupt). */
                    s->rst &= (uint8_t)(v | 0xE1u);
                    if (!(v & 1u)) s->rfir_ack = 1;
                    serviced(s);
                    return;
                case 6:
                    s->msr_delta &= v;
                    serviced(s);
                    return;
                default: s->r[R_ACR0] = su_reg(v, k); return;
            }
        case 1:
            switch (a) {
                case 0:
                    push_txd(s, v, k);
                    return;
                case 1: s->r[R_TXF] = su_reg(v & 0xE0u, k); return;
                case 3:
                    s->tmcr = su_reg(v & 0x33u, k);
                    if (v & 1u) { s->brg[0].timer_run = !brg_mode(s, 0); s->brg[0].timer_count = divisor_of(s, 0); }
                    if (v & 2u) { s->brg[1].timer_run = !brg_mode(s, 1); s->brg[1].timer_count = divisor_of(s, 1); }
                    return;
                case 4: s->r[R_MCR] = su_reg(v & 0x3Fu, k); return;
                case 5:
                    /* RCM (self-clearing). */
                    if (v & 0x10u) s->rx_count = 0;
                    if (v & 0x20u) { serial_rx_reset(&s->line.rx); s->rx_enabled = 1; s->rx_locked = 0; }
                    if (v & 0x40u) s->rx_enabled = 0;
                    if (v & 0x80u) s->rx_enabled = 1;
                    if (v & 0x08u) s->rx_locked = 1;
                    if (v & 0x04u) s->rx_locked = 0;
                    return;
                case 6:
                    /* TCM. */
                    if (v & 0x04u) s->tx_count = 0;
                    if (v & 0x08u) { serial_tx_reset(&s->line.tx); s->tsr = LOGIC_1; s->tx_enabled = 1; }
                    if (v & 0x01u) s->tx_enabled = 0;   /* the current character completes */
                    if (v & 0x02u) s->tx_enabled = 1;
                    return;
                default:
                    /* ICM. */
                    if (v & 0x10u) { software_reset(s); return; }
                    if (v & 0x08u) { s->int_latched = 0; s->int_force_low = 2; }
                    if (v & 0x04u) { s->rst = 0; s->msr_delta = 0; s->tmst = 0; }
                    if (v & 0x02u) {
                        s->powered_down = 1;
                        s->pd_sleep = s->crystal;
                    }
                    return;
            }
        case 2:
            switch (a) {
                case 1: s->r[R_FMD] = su_reg(v & 0x33u, k); return;
                case 3: s->r[R_TMD] = su_reg(v, k); update_format(s); return;
                case 4: s->r[R_IMD] = su_reg(v & 0x0Fu, k); update_format(s); return;
                case 5: s->r[R_ACR1] = su_reg(v, k); return;
                case 6: s->r[R_RIE] = su_reg(v & 0xFEu, k); return;
                case 7: s->r[R_RMD] = su_reg(v & 0xF8u, k); return;
                default: return;
            }
        default:
            switch (a) {
                case 0:
                    if (dlab(s)) { s->r[R_BBL] = su_reg(v, k); brg_load(s, 1); }
                    else { s->r[R_CLCF] = su_reg(v & 0xF0u, k); update_format(s); }
                    return;
                case 1:
                    if (dlab(s)) { s->r[R_BBH] = su_reg(v, k); brg_load(s, 1); }
                    else { s->r[R_BACF] = su_reg(v & 0x44u, k); brg_load(s, 0); }
                    return;
                case 3: s->r[R_BBCF] = su_reg(v & 0xC4u, k); brg_load(s, 1); return;
                case 4: s->r[R_PMD] = su_reg(v & 0xFCu, k); return;
                case 5: s->r[R_MIE] = su_reg(v & 0x0Fu, k); return;
                case 6: s->r[R_TMIE] = su_reg(v & 0x03u, k); return;
                default: return;
            }
    }
}

/* ---- Bus events ------------------------------------------------------------------------- */

static void bus_peek(void *context, unsigned unit, unsigned address, uint8_t *data, uint8_t *known) {
    I5State *s = context;

    (void)unit;
    if (s->unknown || s->pd_sleep) {
        *data = 0;
        *known = 0;
        return;
    }
    reg_peek(s, address & 7u, data, known);
}

static void bus_read_done(void *context, unsigned unit, unsigned address) {
    I5State *s = context;

    (void)unit;
    if (s->unknown) {
        return;
    }
    if (s->pd_sleep && s->bank == 1 && (address & 7u) == 4) {
        s->pd_sleep = 0;   /* out of sleep: the registers are lost until a reset */
        s->powered_down = 0;
        {
            unsigned i;

            for (i = 0; i < R_COUNT; i++) {
                s->r[i] = su_reg(0, 0);
            }
        }
        update_format(s);
    }
    read_done(s, address & 7u);
    track_modem(s);
    update_outputs(s);
}

static void bus_write(void *context, unsigned unit, unsigned address, uint8_t data, uint8_t known) {
    I5State *s = context;

    (void)unit;
    if (s->unknown || (s->pd_sleep && !(s->bank == 1 && (address & 7u) == 7u))) {
        return;
    }
    write_reg(s, address & 7u, data, known);
    refresh(s);
}

/* ---- Reset, clocks ------------------------------------------------------------------------- */

static void apply_reset(I5State *s) {
    LogicValue r = serial_level(s->sim, s->instance, s->pins->reset, LOGIC_0);

    if (r == LOGIC_X) {
        s->unknown = 1;
        s->in_reset = 0;
        s->bus.enabled = 0;
    } else if (r == LOGIC_1) {
        if (!s->in_reset) {
            /* RTS and DTR become strap inputs with weak pull-ups (p8). */
            su_drive_weak(&s->o, s->sim, s->instance, s->pins->pin[P_RTS], LOGIC_1);
            su_drive_weak(&s->o, s->sim, s->instance, s->pins->pin[P_DTR], LOGIC_1);
        }
        s->unknown = 0;
        s->in_reset = 1;
        s->bus.enabled = 0;
        reset_regs(s);
        serial_line_sender_reset(&s->line, s->sim, s->instance);
    } else {
        if (s->in_reset) {
            /* The strap as RESET falls: RTS low = crystal; DTR low = no /2. */
            LogicValue rts = serial_level(s->sim, s->instance, s->pins->pin[P_RTS], LOGIC_X);
            LogicValue dtr = serial_level(s->sim, s->instance, s->pins->pin[P_DTR], LOGIC_X);

            s->strap_x = (rts == LOGIC_X || dtr == LOGIC_X || (rts == LOGIC_0 && dtr == LOGIC_0))
                ? LOGIC_X : LOGIC_1;
            s->crystal = rts == LOGIC_0;
            s->div2 = s->crystal || dtr != LOGIC_0;
        }
        if (s->unknown) {
            reset_regs(s);
            {
                unsigned i;

                for (i = 0; i < R_COUNT; i++) {
                    s->r[i].k = 0;
                }
            }
            update_format(s);
        }
        s->unknown = 0;
        s->in_reset = 0;
        s->bus.enabled = 1;
    }
    serial_bus_update(&s->bus, s->sim, s->instance, &s->events);
    refresh(s);
}

static void clk_edge(I5State *s, LogicValue level) {
    int rising = s->clk == LOGIC_0 && level == LOGIC_1;
    int falling = s->clk == LOGIC_1 && level == LOGIC_0;

    s->clk = level;
    if (s->strap_x == LOGIC_X) {
        return;
    }
    if (s->div2) {
        if (rising) {
            sysclk_edge(s, s->sysclk == LOGIC_1 ? LOGIC_0 : LOGIC_1);
        }
    } else if (rising || falling) {
        sysclk_edge(s, level);
    }
}

/* ---- Engine callbacks ------------------------------------------------------------------------ */

static void i5_start(Sim *sim, SimInstance *instance, void *raw) {
    I5State *s = raw;
    unsigned i;

    s->sim = sim;
    s->instance = instance;
    s->clk = serial_level(sim, instance, s->pins->clock[K_IN], LOGIC_X);
    s->sclk = serial_level(sim, instance, s->pins->pin[P_RI], LOGIC_X);
    s->sysclk = LOGIC_0;
    s->strap_x = LOGIC_1;
    s->div2 = 1;
    for (i = 0; i < 4; i++) {
        s->msr_level[i] = LOGIC_X;
    }
    track_modem(s);
    s->msr_delta = 0;
    serial_bus_start(&s->bus, sim, instance, &s->events);
    apply_reset(s);
}

static void i5_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    I5State *s = raw;
    const size_t *p = s->pins->pin;

    (void)old_value;
    (void)new_value;
    s->sim = sim;
    s->instance = instance;
    if (pin == s->pins->reset) {
        apply_reset(s);
        return;
    }
    if (pin == s->pins->clock[K_IN]) {
        clk_edge(s, serial_level(sim, instance, pin, LOGIC_X));
        return;
    }
    if (pin == p[P_RI]) {
        sclk_edge(s, serial_level(sim, instance, pin, LOGIC_X));
    }
    if (serial_bus_has_pin(&s->bus, pin)) {
        serial_bus_update(&s->bus, sim, instance, &s->events);
    }
    if (pin == p[P_CTS] || pin == p[P_DSR] || pin == p[P_DCD] || pin == p[P_RI] || pin == p[P_RX]) {
        refresh(s);
    }
}

/* ---- Terminal --------------------------------------------------------------------------------- */

static unsigned i5_channels(const void *raw) {
    (void)raw;
    return 1;
}

static SerialLine *i5_line(void *raw, unsigned channel) {
    (void)channel;
    return &((I5State *)raw)->line;
}

static int i5_async(const void *raw, unsigned channel) {
    (void)raw;
    (void)channel;
    return 1;
}

/* ---- Create / destroy / loader --------------------------------------------------------------------- */

static void *i5_create(const Part *part, const void *data, const void *pins,
                       const SerialGrade *grade, const void *options) {
    I5State *s = calloc(1, sizeof *s);
    size_t i;

    (void)options;
    if (s == NULL) {
        return NULL;
    }
    s->base.model = &I82510_MODEL;
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
        SerialBusTiming timing = {grade->t[T_RLDV], grade->t[T_RHDZ], 0, 0};

        serial_bus_init(&s->bus, &s->pins->bus, &timing);
    }
    s->events.context = s;
    s->events.select = NULL;
    s->events.peek = bus_peek;
    s->events.read_done = bus_read_done;
    s->events.write = bus_write;
    serial_line_init(&s->line, s->pins->pin[P_RX]);
    s->div2 = 1;
    s->strap_x = LOGIC_1;
    reset_regs(s);
    s->clk = s->sclk = LOGIC_X;
    s->sysclk = LOGIC_0;
    return s;
}

static void i5_destroy(void *raw) {
    I5State *s = raw;

    serial_line_free(&s->line);
    free(s->o.out);
    free(s);
}

static void *i5_load_device(KindContext *ctx, const cJSON *json, unsigned *channels) {
    I5Device *d = kind_alloc(ctx, sizeof *d);
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
        su_load_names(ctx, bus, "data", 1, d->data, 8, 8, NULL) != 0 ||
        su_load_names(ctx, bus, "address", 1, d->addr, 3, 3, NULL) != 0 ||
        kind_string(ctx, bus, "read", &d->read) != 0 ||
        kind_string(ctx, bus, "write", &d->write) != 0 ||
        kind_string(ctx, bus, "select", &d->select) != 0 ||
        kind_string(ctx, bus, "reset", &d->reset) != 0) {
        return NULL;
    }
    kind_pop(ctx);
    if (su_load_pin_object(ctx, json, "clock", 1, CLOCK_KEYS, d->clock, 0x3u) != 0 ||
        su_load_pin_object(ctx, json, "serial", 1, SERIAL_KEYS, d->pin, 0x1FFu) != 0) {
        return NULL;
    }
    *channels = 1;
    return d;
}

static void *i5_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t pin_count) {
    const I5Device *d = raw;
    I5Pins *p = kind_alloc(ctx, sizeof *p);

    if (p == NULL ||
        su_bind_names(ctx, pins, pin_count, "bus.data", d->data, p->bus.data, 8) != 0 ||
        su_bind_names(ctx, pins, pin_count, "bus.address", d->addr, p->bus.address, 3) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.read", d->read, &p->bus.read) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.write", d->write, &p->bus.write) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.select", d->select, &p->bus.select[0]) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.reset", d->reset, &p->reset) != 0 ||
        su_bind_object(ctx, pins, pin_count, "clock", CLOCK_KEYS, d->clock, p->clock) != 0 ||
        su_bind_object(ctx, pins, pin_count, "serial", SERIAL_KEYS, d->pin, p->pin) != 0) {
        return NULL;
    }
    p->bus.address_count = 3;
    p->bus.select_count = 1;
    p->bus.select_high_count = 0;
    p->bus.read_high = SERIAL_NO_PIN;
    p->bus.write_high = SERIAL_NO_PIN;
    p->bus.address_strobe = SERIAL_NO_PIN;
    p->bus.driver_disable = SERIAL_NO_PIN;
    p->bus.select_out = SERIAL_NO_PIN;
    return p;
}

const SerialModel I82510_MODEL = {
    "i82510", KEYS, TIMING, NULL,
    i5_load_device, i5_bind, NULL, NULL,
    i5_create, i5_destroy, i5_start, i5_pin_changed, NULL,
    i5_channels, i5_line, i5_async
};
