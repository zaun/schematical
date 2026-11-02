/*
 * max3100: the MAX3100 SPI/MICROWIRE UART and the MAX3140 (the same UART
 * with an RS-485/RS-422 transceiver on the die) (plan-12-J section 3.1;
 * plan-12-data/specs/maxim.md sections MAX3100 and MAX3140, with
 * decisions.md section MAX3100 / MAX3140 and the planner decisions).
 *
 * The host port is an SPI target (serial_target.h), mode 0: one 16-bit
 * word per CS-low frame, DIN sampled on SCLK rising, DOUT shifted on SCLK
 * falling, bit 15 on CS falling. DIN bits 15-14 pick the operation: 11
 * Write Config, 01 Read Config, 10 Write Data, 00 Read Data. Most effects
 * happen on CS rising (a frame of other than 16 bits is aborted); the
 * interrupt masks of a Write Config take effect at the 16th SCLK rising
 * edge; a Read or Write Data pops the receive word at the 16th SCLK falling
 * edge (R clears then if nothing else is waiting: MAX3140 p17, applied to
 * both); the T interrupt latch clears on CS rising of a Read or Write Data
 * (planner decision: CS rising; MAX3140 p23: Read or Write Data).
 *
 * UART: baud = X1 / (16 x BRD), BRD from the B3-B0 table; 7 or 8 data bits,
 * the 9th bit (Pt/Pr) when PE = 1 (no parity is computed), 1 or 2 stop bits
 * sent, one checked; 3-sample majority (the 7th, 8th and 9th 16x samples);
 * an 8-word receive FIFO (FEN = 1: one word), cleared when it overflows
 * (RA/FE set: decisions.md, P1); FE set by a 0 stop bit, cleared by the
 * next good frame; after a framing error the receiver waits for a new start
 * bit. IrDA timing (IR = 1): a 0 bit is a TX low pulse of 3/16 bit (TX idles
 * high); RX idles low and a high pulse of at least one 16x sample is a 0
 * bit. Shutdown: the SHDN pin (immediate, aborts the transmitter) or SHDNi
 * (after the transmitter is idle); it clears the receiver and sets T, stops
 * the clock, and RA reports any RX transition. Test mode (Read Config with
 * TEST = 1, until a Read Config with TEST = 0), while CS is low: MAX3100
 * (testMode "loopback"): RTS outputs the 16x clock and TX loops to RX;
 * MAX3140 (testMode "txLow"): TX is held low (planner decision).
 *
 * MAX3140 transceiver (key "transceiver"; its logic side is independent of
 * the UART, max3140 p1): driver Y = DI xor TXP and Z = not Y while DE = 1,
 * both Z while DE = 0; receiver RO = (A - B >= -50 mV) xor RXP while RE = 0,
 * Z while RE = 1, reading Y/Z instead of A/B when H/F = 1. Digital reading
 * of the differential pair: A != B gives A; A = B (shorted) and both Z
 * (open) give 1 (true fail-safe); an X or one open input gives X. H/F, TXP
 * and RXP have internal pull-downs (Z reads 0); SRL picks the delays: 0
 * 10 Mbps, 1 500 kbps, Z 115 kbps. RE = 1 and DE = 0 held tshdn enter the
 * transceiver's shutdown, whose enable times are longer.
 *
 * Terminal: the channel's RX pin (MAX3100). The MAX3140's terminal drives
 * the transceiver's A (true) and B (complement) (plan-12-J section 3.3) and
 * its UART receiver hears the typed waveform directly (plan-12-C rule).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serial.h"
#include "serial_target.h"

enum {
    T_DV, T_TR, T_DO,
    T_DP115, T_DP500, T_DP10M, T_DZ115, T_DZ500, T_DZ10M, T_DD, T_RP, T_RP10M, T_RZ,
    T_SHDN, T_DZS115, T_DZS500, T_DZS10M, T_RZS
};
static const char *const TIMING[] = {
    "tdv", "ttr", "tdo",
    "tdp115", "tdp500", "tdp10m", "tdz115", "tdz500", "tdz10m", "tdd", "trp", "trp10m", "trz",
    "tshdn", "tdzs115", "tdzs500", "tdzs10m", "trzs",
    NULL
};

static const char *const KEYS[] = {
    "bus", "clock", "channels", "shutdown", "testMode", "transceiver", NULL
};

typedef enum { CH_TX, CH_RX, CH_RTS, CH_CTS, CH_INT, CH_PIN_COUNT } ChannelPin;
static const char *const CHANNEL_KEYS[] = {"tx", "rx", "rts", "cts", "int", NULL};

typedef enum {
    X_Y, X_Z, X_A, X_B, X_DE, X_RE, X_RO, X_DI, X_HF, X_SRL, X_TXP, X_RXP, X_PIN_COUNT
} XcvrPin;
static const char *const XCVR_KEYS[] = {
    "y", "z", "a", "b", "de", "re", "ro", "di", "hf", "srl", "txp", "rxp", NULL
};
enum { XCVR_REQUIRED = X_HF + 1 };   /* y..hf required, srl/txp/rxp optional */

/* Baud rate divisors by B3..B0 (max3100 p12 Table 7). */
static const uint16_t BRD[16] = {1, 2, 4, 8, 16, 32, 64, 128, 3, 6, 12, 24, 48, 96, 192, 384};

enum { M_FIFO = 8 };

/* Timers. */
enum { TIMER_XSHDN = 1 };

/* Config word bits 13..0 (Write Config DIN, Read Config DOUT). */
enum {
    C_FEN = 1u << 13, C_SHDN = 1u << 12, C_TM = 1u << 11, C_RM = 1u << 10, C_PM = 1u << 9,
    C_RAM = 1u << 8, C_IR = 1u << 7, C_ST = 1u << 6, C_PE = 1u << 5, C_L = 1u << 4,
    C_MASKS = C_TM | C_RM | C_PM | C_RAM
};

typedef enum { OP_READ_DATA = 0, OP_READ_CONFIG = 1, OP_WRITE_DATA = 2, OP_WRITE_CONFIG = 3 } Op;

/* ---- Device data ------------------------------------------------------------------- */

typedef struct {
    SerialHostDevice host;
    const char *clock_in, *clock_out, *shutdown;
    const char *channel[CH_PIN_COUNT];
    int test_tx_low;          /* testMode "txLow" (MAX3140) or "loopback" (MAX3100) */
    int xcvr;
    const char *xcvr_pin[X_PIN_COUNT];
} M3Device;

typedef struct {
    SerialHostPins host;
    size_t clock_in, clock_out, shutdown;
    size_t channel[CH_PIN_COUNT];
    size_t xcvr[X_PIN_COUNT];
} M3Pins;

/* ---- State --------------------------------------------------------------------------- */

typedef struct {
    uint16_t data;           /* D7..D0, Pr at bit 8 */
    uint16_t known;
} Word;

typedef struct {
    SerialBase base;
    const M3Device *dev;
    const M3Pins *pins;
    const SerialGrade *grade;
    Sim *sim;
    SimInstance *instance;
    SerialHost host;
    SerialSpiEvents spi_events;
    SerialI2cEvents i2c_events;
    SerialLine line;
    SerialBaud baud;
    SerialFormat format;
    LogicValue *out;
    size_t pin_count;

    /* Configuration: as written (Read Config) and as in effect. */
    uint16_t cfg;
    uint16_t cfg_k;
    uint16_t act;            /* FEN SHDNi IR ST PE L B3..B0 in effect */
    uint16_t act_k;
    int cfg_pending;         /* a written config waits for the transmitter */

    /* Receiver. */
    Word fifo[M_FIFO];
    unsigned head;
    unsigned count;
    LogicValue rafe;         /* RA (shutdown) / FE (and FIFO overflow) */
    int fe_seen;             /* a framing error was delivered this tick */
    unsigned ir_zero;        /* IrDA decoder: 16x ticks of 0 left */

    /* Transmitter. */
    int tbuf_full;
    Word tbuf;
    int t_latch;             /* the T interrupt source (SR latch) */
    LogicValue tx_level;     /* the transmitter's output */
    LogicValue rts_bit;      /* the RTS register bit (1 drives the pin low) */

    /* Shutdown, test mode. */
    int sw_shutdown;         /* SHDNi in effect */
    LogicValue shdn_pin;     /* 1 = the SHDN pin asks for shutdown */
    int shutdown;            /* the UART is shut down */
    int test;                /* test mode */
    int unknown;             /* SHDN is X */
    LogicValue clock_level;

    /* The frame in progress. */
    uint16_t word_in;
    uint16_t word_in_k;
    int op;                  /* -1 unknown */
    Word snap;               /* the receive word shown in this frame */
    LogicValue snap_r, snap_t, snap_cts, snap_rafe;

    /* Transceiver. */
    int xcvr_shdn;
    uint32_t xcvr_gen;
    LogicValue b_out;        /* the terminal's complement on B */
} M3State;

/* ---- Helpers ----------------------------------------------------------------------------- */

static SimTime t_of(const M3State *s, unsigned i) {
    return s->grade->t[i];
}

static LogicValue not3(LogicValue a) {
    return a == LOGIC_0 ? LOGIC_1 : a == LOGIC_1 ? LOGIC_0 : LOGIC_X;
}

static LogicValue xor3(LogicValue a, LogicValue b) {
    if ((a != LOGIC_0 && a != LOGIC_1) || (b != LOGIC_0 && b != LOGIC_1)) {
        return LOGIC_X;
    }
    return a == b ? LOGIC_0 : LOGIC_1;
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

static LogicValue bit3(uint16_t v, uint16_t k, uint16_t mask) {
    if (!(k & mask)) {
        return LOGIC_X;
    }
    return v & mask ? LOGIC_1 : LOGIC_0;
}

static LogicValue level(const M3State *s, size_t pin) {
    return serial_level(s->sim, s->instance, pin, LOGIC_X);
}

/* An input with an internal pull-down: Z reads 0. */
static LogicValue level_pd(const M3State *s, size_t pin) {
    return serial_level(s->sim, s->instance, pin, LOGIC_0);
}

static void drive(M3State *s, size_t pin, LogicValue value, SimTime delay) {
    if (pin == SERIAL_NO_PIN || s->out[pin] == value) {
        return;
    }
    s->out[pin] = value;
    sim_schedule_inertial_drive(s->sim, s->instance, pin, value, DRIVE_STRONG, delay);
}

static unsigned depth(const M3State *s) {
    return (s->act & C_FEN) ? 1 : M_FIFO;
}

static unsigned data_bits(const M3State *s) {
    return (s->act & C_L) ? 7 : 8;
}

static void update_format(M3State *s) {
    SerialFormat f = serial_format_default();

    f.data_bits = data_bits(s) + ((s->act & C_PE) ? 1 : 0);
    f.parity = SERIAL_PARITY_NONE;
    f.stop_halves = (s->act & C_ST) ? 4 : 2;
    f.majority = 1;
    f.format_x = (s->act_k & (C_ST | C_PE | C_L)) != (C_ST | C_PE | C_L);
    s->format = f;
}

static void load_baud(M3State *s) {
    serial_baud_load(&s->baud, BRD[s->act & 15u], (s->act_k & 15u) == 15u);
}

/* CS is low now (test mode effects). */
static int cs_low(const M3State *s) {
    return s->host.spi.cs == LOGIC_0;
}

/* ---- Outputs ------------------------------------------------------------------------------ */

/* Pr of the oldest word (0 when empty or PE = 0). */
static LogicValue pr_now(const M3State *s) {
    if (s->count == 0) {
        return LOGIC_0;
    }
    return bit3(s->fifo[s->head].data, s->fifo[s->head].known, 0x100);
}

static LogicValue irq_level(const M3State *s) {
    LogicValue r = and3(bool3(s->count > 0), bit3(s->cfg, s->cfg_k, C_RM));
    LogicValue t = and3(bool3(s->t_latch), bit3(s->cfg, s->cfg_k, C_TM));
    LogicValue p = and3(pr_now(s), bit3(s->cfg, s->cfg_k, C_PM));
    LogicValue a = and3(s->rafe, bit3(s->cfg, s->cfg_k, C_RAM));

    if (s->unknown) {
        return LOGIC_X;
    }
    return or3(or3(r, t), or3(p, a));
}

static LogicValue tx_pin_level(const M3State *s) {
    LogicValue v = s->tx_level;

    if (s->unknown) {
        return LOGIC_X;
    }
    if (s->test && cs_low(s) && s->dev->test_tx_low) {
        return LOGIC_0;   /* MAX3140 test mode */
    }
    return v;
}

static void update_outputs(M3State *s) {
    const size_t *pin = s->pins->channel;
    LogicValue irq = irq_level(s);
    LogicValue rts;

    /* IRQ: open drain, active low. */
    drive(s, pin[CH_INT], irq == LOGIC_1 ? LOGIC_0 : irq == LOGIC_0 ? LOGIC_Z : LOGIC_X, 0);
    drive(s, pin[CH_TX], tx_pin_level(s), 0);
    if (s->test && cs_low(s) && !s->dev->test_tx_low) {
        rts = s->baud.baudout;   /* MAX3100 test mode: the 16x clock */
    } else {
        rts = not3(s->rts_bit);
    }
    drive(s, pin[CH_RTS], rts, 0);
}

/* ---- Receiver ------------------------------------------------------------------------------ */

static void clear_rx(M3State *s) {
    s->head = 0;
    s->count = 0;
}

static void on_received(void *context, const SerialRxChar *c) {
    M3State *s = context;
    unsigned n = data_bits(s);
    Word w;

    if (c->flags & SERIAL_RX_FE) {
        s->rafe = LOGIC_1;
        s->fe_seen = 1;
    } else if (!(c->flags_known & SERIAL_RX_FE)) {
        s->rafe = LOGIC_X;
    } else if (!s->shutdown) {
        s->rafe = LOGIC_0;   /* the next properly framed character clears FE */
    }
    w.data = (uint16_t)(c->data & ((1u << n) - 1));
    w.known = (uint16_t)((c->known & ((1u << n) - 1)) | (n == 7 ? 0x80u : 0u));
    if (s->act & C_PE) {
        if ((c->data >> n) & 1u) {
            w.data |= 0x100;
        }
        if ((c->known >> n) & 1u) {
            w.known |= 0x100;
        }
    } else {
        w.known |= 0x100;
    }
    if (s->count >= depth(s)) {
        /* "The FIFO is cleared if it overflows" (p7); RA/FE reports it. */
        clear_rx(s);
        s->rafe = LOGIC_1;
        return;
    }
    s->fifo[(s->head + s->count) % M_FIFO] = w;
    s->count++;
}

static LogicValue irda_decode(M3State *s, LogicValue pin) {
    /* RX idles low; a high pulse is a 0 bit. */
    if (pin == LOGIC_X) {
        return LOGIC_X;
    }
    if (pin == LOGIC_1 && s->ir_zero == 0) {
        s->ir_zero = 16;
    }
    if (s->ir_zero > 0) {
        s->ir_zero--;
        return LOGIC_0;
    }
    return LOGIC_1;
}

static void mirror_b(M3State *s) {
    LogicValue v;

    if (!s->dev->xcvr) {
        return;
    }
    v = s->line.sender.level == LOGIC_Z ? LOGIC_Z : not3(s->line.sender.level);
    if (v != s->b_out) {
        s->b_out = v;
        sim_schedule_drive(s->sim, s->instance, s->pins->xcvr[X_B], v, DRIVE_STRONG, 0);
    }
}

static void rx_tick(M3State *s) {
    SerialRxEvents events = {s, on_received, NULL};
    LogicValue pin = level(s, s->pins->channel[CH_RX]);
    LogicValue in;

    in = serial_line_rx_input(&s->line, s->sim, s->instance, pin, &s->format);
    mirror_b(s);
    if (s->line.sender.active) {
        /* The typed waveform (NRZ) reaches the receiver directly. */
    } else if (s->act & C_IR) {
        in = irda_decode(s, pin);
    }
    if (s->test && cs_low(s) && !s->dev->test_tx_low) {
        in = s->tx_level;   /* digital loopback */
    }
    s->fe_seen = 0;
    serial_rx_tick(&s->line.rx, in, &s->format, &events);
    if (s->fe_seen) {
        /* After a framing error the receiver looks for a new start bit. */
        s->line.rx.phase = SERIAL_RX_IDLE;
    }
}

/* ---- Transmitter --------------------------------------------------------------------------- */

static void apply_config(M3State *s);

static int tx_fetch(void *context, uint16_t *data, uint16_t *known) {
    M3State *s = context;

    if (!s->tbuf_full) {
        return 0;
    }
    *data = s->tbuf.data;
    *known = s->tbuf.known;
    s->tbuf_full = 0;
    s->t_latch = 1;          /* the buffer became empty */
    return 1;
}

static void tx_done(void *context, uint16_t data, uint16_t known) {
    M3State *s = context;

    (void)known;
    serial_line_log(&s->line, data);
    if (s->cfg_pending && !s->tbuf_full) {
        apply_config(s);
    }
}

static uint16_t tx_word(const M3State *s, Word w, uint16_t *known) {
    unsigned n = data_bits(s);
    uint16_t mask = (uint16_t)((1u << n) - 1);
    uint16_t data = w.data & mask;

    *known = w.known & mask;
    if (s->act & C_PE) {
        data |= (uint16_t)(((w.data >> 8) & 1u) << n);
        *known |= (uint16_t)(((w.known >> 8) & 1u) << n);
    }
    return data;
}

static void tx_tick(M3State *s) {
    SerialTxEvents events = {s, tx_fetch, NULL, tx_done};
    LogicValue v = serial_tx_tick(&s->line.tx, &s->format, &events);

    if ((s->act & C_IR) && s->line.tx.count >= 3) {
        v = LOGIC_1;         /* IrDA: a 0 is a 3/16-bit low pulse */
    }
    s->tx_level = v;
}

/* ---- Shutdown and configuration ----------------------------------------------------------- */

static void enter_shutdown(M3State *s, int abort_tx) {
    s->shutdown = 1;
    clear_rx(s);
    s->rafe = LOGIC_0;
    s->tbuf_full = 0;
    s->tbuf.data = 0;
    s->tbuf.known = 0;
    s->t_latch = 1;
    s->cfg_pending = 0;
    if (abort_tx) {
        serial_tx_reset(&s->line.tx);
        s->tx_level = LOGIC_1;
    }
    serial_rx_reset(&s->line.rx);
}

static void update_shutdown(M3State *s) {
    int want = s->shdn_pin == LOGIC_1 || s->sw_shutdown;

    if (want && !s->shutdown) {
        enter_shutdown(s, s->shdn_pin == LOGIC_1);
    } else if (!want && s->shutdown) {
        s->shutdown = 0;
        s->rafe = LOGIC_0;   /* RA clears on leaving shutdown */
    }
}

/* The written configuration takes effect (transmitter idle). */
static void apply_config(M3State *s) {
    s->cfg_pending = 0;
    s->act = (uint16_t)(s->cfg & ~C_MASKS);
    s->act_k = (uint16_t)(s->cfg_k | C_MASKS);
    update_format(s);
    load_baud(s);
    s->sw_shutdown = (s->cfg & C_SHDN) != 0;
    update_shutdown(s);
}

static int tx_idle(const M3State *s) {
    return !s->tbuf_full && !serial_tx_busy(&s->line.tx);
}

static void write_config(M3State *s, uint16_t word, uint16_t known) {
    s->cfg = (uint16_t)(word & 0x3FFF);
    s->cfg_k = (uint16_t)(known & 0x3FFF);
    /* Clears the receive FIFO, R, RA/FE, D0r-D7r, Pr, the transmit buffer
       and Pt; T = 1 with no data pending (decisions.md). */
    clear_rx(s);
    s->rafe = LOGIC_0;
    s->tbuf_full = 0;
    s->tbuf.data = 0;
    s->tbuf.known = 0;
    s->t_latch = 1;
    if (!(s->cfg & C_SHDN) && (s->cfg_k & C_SHDN)) {
        s->sw_shutdown = 0;   /* leaving software shutdown restarts at once */
        update_shutdown(s);
    }
    if (tx_idle(s) || s->shutdown) {
        apply_config(s);
    } else {
        s->cfg_pending = 1;
    }
}

/* ---- The host port -------------------------------------------------------------------------- */

static void frame_x(M3State *s) {
    /* An unknown operation: every register it could have written is X. */
    s->cfg_k = 0;
    s->act_k = 0;
    update_format(s);
    load_baud(s);
    s->rts_bit = LOGIC_X;
}

static void spi_begin(void *context) {
    M3State *s = context;

    s->word_in = 0;
    s->word_in_k = 0;
    s->op = -1;
    s->snap_r = s->unknown ? LOGIC_X : bool3(s->count > 0);
    s->snap_t = s->unknown ? LOGIC_X : bool3(!s->tbuf_full);
}

static SerialSpiByte out_byte(uint16_t v, uint16_t k) {
    SerialSpiByte b;

    b.data = (uint8_t)v;
    b.known = (uint8_t)k;
    b.drive = 0xFF;
    return b;
}

/* The DOUT word for the operation (bits 13..0), once it is known. */
static void dout_word(M3State *s, uint16_t *v, uint16_t *k) {
    *v = 0;
    *k = 0x3FFF;
    if (s->unknown || s->op < 0) {
        *k = 0;
        return;
    }
    switch ((Op)s->op) {
        case OP_WRITE_CONFIG:
            break;
        case OP_READ_CONFIG:
            *v = (uint16_t)(s->cfg & ~C_SHDN);
            *k = (uint16_t)(s->cfg_k | C_SHDN);
            if (s->shutdown) {
                *v |= C_SHDN;   /* SHDNo */
            }
            break;
        case OP_WRITE_DATA:
        case OP_READ_DATA: {
            Word w = s->count > 0 ? s->fifo[s->head] : (Word){0, 0x1FF};
            uint16_t lowk = w.known & 0x1FF;
            LogicValue cts = not3(level(s, s->pins->channel[CH_CTS]));

            *v = (uint16_t)(w.data & 0x1FF);
            *k = (uint16_t)(0x3800 | lowk);
            if (!(s->act & C_PE) && (s->act_k & C_PE)) {
                *v &= (uint16_t)~0x100;
                *k |= 0x100;
            }
            if (s->rafe == LOGIC_1) *v |= 0x400;
            if (s->rafe != LOGIC_X) *k |= 0x400;
            if (cts == LOGIC_1) *v |= 0x200;
            if (cts != LOGIC_X) *k |= 0x200;
            break;
        }
    }
}

static SerialSpiByte spi_next(void *context, unsigned index) {
    M3State *s = context;
    uint16_t v;
    uint16_t k;

    if (index == 0) {
        /* R and T at CS fall; the rest once the operation is known. */
        v = (uint16_t)((s->snap_r == LOGIC_1 ? 0x80u : 0u) | (s->snap_t == LOGIC_1 ? 0x40u : 0u));
        k = (uint16_t)((s->snap_r != LOGIC_X ? 0x80u : 0u) | (s->snap_t != LOGIC_X ? 0x40u : 0u));
        return out_byte(v, k);
    }
    if (index == 1) {
        dout_word(s, &v, &k);
        return out_byte(v & 0xFF, k & 0xFF);
    }
    return out_byte(0, 0);   /* past the 16-bit word: undefined (P2) */
}

static void spi_bit(void *context, unsigned index, unsigned bits, uint8_t data, uint8_t known,
                    SerialSpiByte *out) {
    M3State *s = context;
    uint16_t v;
    uint16_t k;

    if (index != 0 || bits != 2) {
        return;
    }
    s->op = (known & 3u) == 3u ? (int)(data & 3u) : -1;
    dout_word(s, &v, &k);
    out->data = (uint8_t)((out->data & 0xC0u) | ((v >> 8) & 0x3Fu));
    out->known = (uint8_t)((out->known & 0xC0u) | ((k >> 8) & 0x3Fu));
}

static void spi_received(void *context, unsigned index, uint8_t data, uint8_t known) {
    M3State *s = context;

    if (index > 1) {
        return;
    }
    s->word_in = (uint16_t)((s->word_in << 8) | data);
    s->word_in_k = (uint16_t)((s->word_in_k << 8) | known);
    if (index == 1 && s->op == OP_WRITE_CONFIG && !s->unknown) {
        /* The masks take effect after the 16th SCLK rising edge (p8). */
        s->cfg = (uint16_t)((s->cfg & ~C_MASKS) | (s->word_in & C_MASKS));
        s->cfg_k = (uint16_t)((s->cfg_k & ~C_MASKS) | (s->word_in_k & C_MASKS));
        update_outputs(s);
    }
}

static void spi_shift(void *context, unsigned bits) {
    M3State *s = context;

    /* The falling edge of the 16th SCLK: a data operation pops its word. */
    if (bits == 16 && !s->unknown && (s->op == OP_READ_DATA || s->op == OP_WRITE_DATA) &&
        s->count > 0) {
        s->head = (s->head + 1) % M_FIFO;
        s->count--;
        update_outputs(s);
    }
}

static void spi_end(void *context, unsigned bits) {
    M3State *s = context;
    uint16_t w = s->word_in;
    uint16_t k = s->word_in_k;

    if (bits == 16 && !s->unknown) {
        switch (s->op) {
            case OP_WRITE_CONFIG:
                write_config(s, w, k);
                break;
            case OP_READ_CONFIG:
                if (k & 1u) {
                    s->test = w & 1u;
                }
                break;
            case OP_WRITE_DATA:
                /* RTS (bit 9: 1 drives the pin low); TE (bit 10) = 1 updates
                   only RTS; else Pt (bit 8) and D7t-D0t go to the buffer. */
                s->rts_bit = bit3(w, k, 0x200);
                if (!(k & 0x400)) {
                    s->tbuf_full = 1;
                    s->tbuf.data = 0;
                    s->tbuf.known = 0;
                } else if (!(w & 0x400) && !s->shutdown) {
                    s->tbuf.data = (uint16_t)(w & 0x1FF);
                    s->tbuf.known = (uint16_t)(k & 0x1FF);
                    s->tbuf.data = tx_word(s, s->tbuf, &s->tbuf.known);
                    s->tbuf_full = 1;
                }
                s->t_latch = 0;
                break;
            case OP_READ_DATA:
                s->t_latch = 0;
                break;
            default:
                frame_x(s);
                break;
        }
    }
    update_outputs(s);
}

/* ---- Transceiver ------------------------------------------------------------------------------- */

/* SRL: 0 -> 10 Mbps, 1 -> 500 kbps, Z (open) or X -> 115 kbps. */
static unsigned srl_speed(const M3State *s) {
    LogicValue v = s->pins->xcvr[X_SRL] == SERIAL_NO_PIN
                       ? LOGIC_Z
                       : sim_read_pin(s->sim, s->instance, s->pins->xcvr[X_SRL]);

    return v == LOGIC_0 ? 2 : v == LOGIC_1 ? 1 : 0;
}

/* The differential pair read digitally (true fail-safe). */
static LogicValue differential(const M3State *s, size_t p, size_t n) {
    LogicValue a = sim_read_pin(s->sim, s->instance, p);
    LogicValue b = sim_read_pin(s->sim, s->instance, n);

    if (a == LOGIC_Z && b == LOGIC_Z) {
        return LOGIC_1;    /* open */
    }
    if (a == LOGIC_X || b == LOGIC_X || a == LOGIC_Z || b == LOGIC_Z) {
        return LOGIC_X;
    }
    return a == b ? LOGIC_1 : a;   /* shorted -> 1; else the A side */
}

static void update_driver(M3State *s, SimTime delay) {
    const size_t *x = s->pins->xcvr;
    LogicValue de = level(s, x[X_DE]);
    LogicValue y;

    if (de == LOGIC_1) {
        y = xor3(level(s, x[X_DI]), level_pd(s, x[X_TXP]));
        drive(s, x[X_Y], y, delay);
        drive(s, x[X_Z], not3(y), delay);
    } else {
        LogicValue off = de == LOGIC_0 ? LOGIC_Z : LOGIC_X;

        drive(s, x[X_Y], off, delay);
        drive(s, x[X_Z], off, delay);
    }
}

static void update_receiver(M3State *s, SimTime delay) {
    const size_t *x = s->pins->xcvr;
    LogicValue re = level(s, x[X_RE]);
    LogicValue hf = level_pd(s, x[X_HF]);
    LogicValue v;

    if (re == LOGIC_1) {
        drive(s, x[X_RO], LOGIC_Z, delay);
        return;
    }
    if (hf == LOGIC_1) {
        v = differential(s, x[X_Y], x[X_Z]);
    } else if (hf == LOGIC_0) {
        v = differential(s, x[X_A], x[X_B]);
    } else {
        LogicValue full = differential(s, x[X_A], x[X_B]);
        LogicValue half = differential(s, x[X_Y], x[X_Z]);

        v = full == half ? full : LOGIC_X;
    }
    v = xor3(v, level_pd(s, x[X_RXP]));
    drive(s, x[X_RO], re == LOGIC_0 ? v : LOGIC_X, delay);
}

static void xcvr_shutdown_check(M3State *s) {
    const size_t *x = s->pins->xcvr;

    s->xcvr_gen++;
    if (level(s, x[X_RE]) == LOGIC_1 && level(s, x[X_DE]) == LOGIC_0) {
        sim_schedule_timer(s->sim, s->instance, TIMER_XSHDN | (s->xcvr_gen << 4), t_of(s, T_SHDN));
    }
}

static void xcvr_pin(M3State *s, size_t pin) {
    const size_t *x = s->pins->xcvr;
    unsigned speed = srl_speed(s);
    static const unsigned DP[3] = {T_DP115, T_DP500, T_DP10M};
    static const unsigned DZ[3] = {T_DZ115, T_DZ500, T_DZ10M};
    static const unsigned DZS[3] = {T_DZS115, T_DZS500, T_DZS10M};

    if (pin == x[X_DE]) {
        LogicValue de = level(s, x[X_DE]);
        SimTime delay = de == LOGIC_1 ? t_of(s, s->xcvr_shdn ? DZS[speed] : DZ[speed])
                                      : t_of(s, T_DD);

        if (de == LOGIC_1) {
            s->xcvr_shdn = 0;
        }
        update_driver(s, delay);
        xcvr_shutdown_check(s);
    }
    if (pin == x[X_DI] || pin == x[X_TXP] || pin == x[X_SRL]) {
        update_driver(s, t_of(s, DP[speed]));
    }
    if (pin == x[X_RE]) {
        LogicValue re = level(s, x[X_RE]);
        SimTime delay = t_of(s, re == LOGIC_0 && s->xcvr_shdn ? T_RZS : T_RZ);

        if (re == LOGIC_0) {
            s->xcvr_shdn = 0;
        }
        update_receiver(s, delay);
        xcvr_shutdown_check(s);
    }
    if (pin == x[X_A] || pin == x[X_B] || pin == x[X_Y] || pin == x[X_Z] || pin == x[X_HF] ||
        pin == x[X_RXP] || pin == x[X_SRL]) {
        update_receiver(s, t_of(s, speed == 2 ? T_RP10M : T_RP));
    }
}

/* ---- Engine callbacks ------------------------------------------------------------------------------ */

static void clock_edge(M3State *s, LogicValue v) {
    int rising = s->clock_level == LOGIC_0 && v == LOGIC_1;
    int falling = s->clock_level == LOGIC_1 && v == LOGIC_0;

    s->clock_level = v;
    if ((!rising && !falling) || s->shutdown || s->unknown) {
        return;
    }
    if (serial_baud_edge(&s->baud, rising, v)) {
        tx_tick(s);
        rx_tick(s);
    }
    update_outputs(s);
}

static void shdn_changed(M3State *s) {
    LogicValue v = serial_level(s->sim, s->instance, s->pins->shutdown, LOGIC_1);

    s->unknown = v == LOGIC_X;
    s->shdn_pin = v == LOGIC_0 ? LOGIC_1 : v == LOGIC_1 ? LOGIC_0 : LOGIC_X;
    if (!s->unknown) {
        update_shutdown(s);
    }
    update_outputs(s);
}

static void m3_start(Sim *sim, SimInstance *instance, void *raw) {
    M3State *s = raw;

    s->sim = sim;
    s->instance = instance;
    s->clock_level = level(s, s->pins->clock_in);
    serial_host_start(&s->host, sim, instance);
    shdn_changed(s);
    if (s->dev->xcvr) {
        update_driver(s, 0);
        update_receiver(s, 0);
        xcvr_shutdown_check(s);
    }
    update_outputs(s);
}

static void m3_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    M3State *s = raw;

    (void)old_value;
    (void)new_value;
    s->sim = sim;
    s->instance = instance;
    if (pin == s->pins->shutdown) {
        shdn_changed(s);
    }
    if (pin == s->pins->clock_in) {
        clock_edge(s, level(s, pin));
    }
    if (serial_host_has_pin(&s->dev->host, &s->pins->host, pin)) {
        serial_host_pin(&s->host, sim, instance, pin, &s->spi_events, &s->i2c_events);
        if (pin == s->pins->host.spi.cs) {
            update_outputs(s);   /* test mode follows CS */
        }
    }
    if (pin == s->pins->channel[CH_RX] && s->shutdown && !s->unknown) {
        s->rafe = LOGIC_1;      /* RA: receiver activity in shutdown */
        update_outputs(s);
    }
    if (s->dev->xcvr) {
        xcvr_pin(s, pin);
    }
}

static void m3_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t id) {
    M3State *s = raw;
    const size_t *x = s->pins->xcvr;

    s->sim = sim;
    s->instance = instance;
    if ((id & 15u) == TIMER_XSHDN && (id >> 4) == (s->xcvr_gen & 0x0FFFFFFFu) &&
        level(s, x[X_RE]) == LOGIC_1 && level(s, x[X_DE]) == LOGIC_0) {
        s->xcvr_shdn = 1;
    }
}

/* ---- Terminal ---------------------------------------------------------------------------------------- */

static unsigned m3_channels(const void *raw) {
    (void)raw;
    return 1;
}

static SerialLine *m3_line(void *raw, unsigned channel) {
    (void)channel;
    return &((M3State *)raw)->line;
}

static int m3_async(const void *raw, unsigned channel) {
    const M3State *s = raw;

    (void)channel;
    return !(s->act & C_IR);   /* IrDA pulses are not an NRZ waveform */
}

/* ---- Create / destroy -------------------------------------------------------------------------------- */

static void *m3_create(const Part *part, const void *data, const void *pins,
                       const SerialGrade *grade, const void *options);

static void m3_destroy(void *raw) {
    M3State *s = raw;

    serial_line_free(&s->line);
    free(s->out);
    free(s);
}

static int m3_open_drain(const void *data, const void *pins, const void *options, size_t pin) {
    const M3Pins *p = pins;

    (void)data;
    (void)options;
    return pin != SERIAL_NO_PIN && pin == p->channel[CH_INT];
}

/* ---- Loader -------------------------------------------------------------------------------------------- */

static void *m3_load_device(KindContext *ctx, const cJSON *json, unsigned *channels) {
    static const char *const CLOCK_KEYS[] = {"in", "out", NULL};
    M3Device *d = kind_alloc(ctx, sizeof *d);
    const cJSON *bus;
    const cJSON *clock;
    const cJSON *list;
    const cJSON *item;
    const char *mode;
    unsigned k;

    if (d == NULL) {
        return NULL;
    }
    bus = kind_member(ctx, json, "bus", 1);
    if (bus == NULL) {
        return NULL;
    }
    kind_push(ctx, "bus");
    if (serial_host_load(ctx, bus, NULL, &d->host) != 0) {
        return NULL;
    }
    if (d->host.style != SERIAL_HOST_SPI || d->host.mode != 0) {
        kind_error(ctx, "the max3100 host port is \"spi\" mode 0");
        return NULL;
    }
    kind_pop(ctx);
    clock = kind_member(ctx, json, "clock", 1);
    if (clock == NULL) {
        return NULL;
    }
    kind_push(ctx, "clock");
    if (kind_check_keys(ctx, clock, CLOCK_KEYS, NULL) != 0 ||
        kind_string(ctx, clock, "in", &d->clock_in) != 0 ||
        serial_optional_name(ctx, clock, "out", &d->clock_out) != 0) {
        return NULL;
    }
    kind_pop(ctx);
    list = kind_member(ctx, json, "channels", 1);
    if (list == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) != 1) {
        kind_error(ctx, "\"channels\" must be an array of 1 channel");
        return NULL;
    }
    item = list->child;
    kind_push(ctx, "channels[0]");
    if (kind_check_keys(ctx, item, CHANNEL_KEYS, NULL) != 0) {
        return NULL;
    }
    for (k = 0; k < CH_PIN_COUNT; k++) {
        if (kind_string(ctx, item, CHANNEL_KEYS[k], &d->channel[k]) != 0) {
            return NULL;
        }
    }
    kind_pop(ctx);
    if (kind_string(ctx, json, "shutdown", &d->shutdown) != 0 ||
        kind_string(ctx, json, "testMode", &mode) != 0) {
        return NULL;
    }
    if (strcmp(mode, "txLow") == 0) {
        d->test_tx_low = 1;
    } else if (strcmp(mode, "loopback") != 0) {
        kind_error(ctx, "\"testMode\" is \"loopback\" or \"txLow\"");
        return NULL;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "transceiver") != NULL) {
        const cJSON *x = kind_member(ctx, json, "transceiver", 1);

        kind_push(ctx, "transceiver");
        if (kind_check_keys(ctx, x, XCVR_KEYS, NULL) != 0) {
            return NULL;
        }
        for (k = 0; k < X_PIN_COUNT; k++) {
            if ((k < XCVR_REQUIRED ? kind_string(ctx, x, XCVR_KEYS[k], &d->xcvr_pin[k])
                                   : serial_optional_name(ctx, x, XCVR_KEYS[k], &d->xcvr_pin[k])) != 0) {
                return NULL;
            }
        }
        kind_pop(ctx);
        d->xcvr = 1;
    }
    *channels = 1;
    return d;
}

static void *m3_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t pin_count) {
    const M3Device *d = raw;
    M3Pins *p = kind_alloc(ctx, sizeof *p);
    char key[32];
    unsigned k;

    if (p == NULL || serial_host_bind(ctx, &d->host, pins, pin_count, &p->host) != 0) {
        return NULL;
    }
    if (serial_bind_pin(ctx, pins, pin_count, "clock.in", d->clock_in, &p->clock_in) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "clock.out", d->clock_out, &p->clock_out) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "shutdown", d->shutdown, &p->shutdown) != 0) {
        return NULL;
    }
    for (k = 0; k < CH_PIN_COUNT; k++) {
        snprintf(key, sizeof key, "channels[0].%s", CHANNEL_KEYS[k]);
        if (serial_bind_pin(ctx, pins, pin_count, key, d->channel[k], &p->channel[k]) != 0) {
            return NULL;
        }
    }
    for (k = 0; k < X_PIN_COUNT; k++) {
        p->xcvr[k] = SERIAL_NO_PIN;
        snprintf(key, sizeof key, "transceiver.%s", XCVR_KEYS[k]);
        if (d->xcvr && serial_bind_pin(ctx, pins, pin_count, key, d->xcvr_pin[k], &p->xcvr[k]) != 0) {
            return NULL;
        }
    }
    return p;
}

const SerialModel MAX3100_MODEL = {
    "max3100", KEYS, TIMING, NULL,
    m3_load_device, m3_bind, NULL, m3_open_drain,
    m3_create, m3_destroy, m3_start, m3_pin_changed, m3_timer,
    m3_channels, m3_line, m3_async
};

static void *m3_create(const Part *part, const void *data, const void *pins,
                       const SerialGrade *grade, const void *options) {
    M3State *s = calloc(1, sizeof *s);
    size_t i;

    (void)options;
    if (s == NULL) {
        return NULL;
    }
    s->base.model = &MAX3100_MODEL;
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
    serial_host_init(&s->host, &s->dev->host, &s->pins->host);
    s->host.spi.t_valid = grade->t[T_DV];
    s->host.spi.t_out = grade->t[T_DO];
    s->host.spi.t_float = grade->t[T_TR];
    s->spi_events.context = s;
    s->spi_events.begin = spi_begin;
    s->spi_events.next = spi_next;
    s->spi_events.bit = spi_bit;
    s->spi_events.received = spi_received;
    s->spi_events.shift = spi_shift;
    s->spi_events.end = spi_end;
    s->i2c_events.context = s;
    serial_line_init(&s->line, s->dev->xcvr ? s->pins->xcvr[X_A] : s->pins->channel[CH_RX]);
    /* Power-on reset (p9-11): configuration 0, FIFO empty, T = 1, RTS pin high. */
    s->cfg = 0;
    s->cfg_k = 0x3FFF;
    s->act = 0;
    s->act_k = 0x3FFF;
    update_format(s);
    load_baud(s);
    s->rafe = LOGIC_0;
    s->t_latch = 1;
    s->tx_level = LOGIC_1;
    s->rts_bit = LOGIC_0;
    s->shdn_pin = LOGIC_0;
    s->clock_level = LOGIC_X;
    s->op = -1;
    s->b_out = LOGIC_Z;
    return s;
}
