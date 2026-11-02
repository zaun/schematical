/*
 * max310x: the Maxim "advanced UART" family with 128-word FIFOs, an SPI or
 * I2C host port, a PLL and a fractional baud generator (plan-12-J section
 * 3.2; plan-12-data/specs/maxim.md sections MAX3107, MAX3108, MAX3109,
 * MAX14830, with decisions.md section maxim.md).
 *
 *   "variant"   max3107 (1 UART), max3108 (1, WLP), max3109 (2), max14830 (4);
 *               the register differences of each are in VARIANTS below.
 *
 * Host port (serial_target.h, style "i2c-spi"): the I2C/SPI pin picks SPI
 * (high) or I2C (low), followed live ("When I2C/SPI is high, DOUT/SDA
 * functions as the DOUT SPI serial-data output", max3107 p12).
 *   SPI mode 0: a command byte, then data bytes. Bit 7 = 1 writes (the
 *   polarity no datasheet prints; decisions.md P11). MAX3107/3108: A6..A0;
 *   MAX3109: bit 6 = 0, bit 5 = U, A4..A0; MAX14830: U1 U0, A4..A0. MISO is
 *   Z during the command byte and during writes; on the MAX3109/MAX14830 a
 *   read drives 0 0 IRQ1 IRQ0 (IRQ3..IRQ0) on clocks 5-8 of the command
 *   byte (the GlobalIRQ bits, active low; a frame of only those 8 clocks is
 *   the "fast read"), then the data. Bursts auto-increment the register
 *   address unless it started at 0x00 (THR/RHR). With the extended map on
 *   (GloblComnd 0xCE) SPI addresses 0x00-0x0F reach 0x20-0x2F.
 *   I2C: the address from the A1/A0 strap table (bus.i2c.table) plus each
 *   UART's offset ("uartOffsets"); a register address byte (ACK only when
 *   valid), then data with the same auto-increment rule; reads after a
 *   repeated START. A read's side effects (an RHR pop, clear-on-read) happen
 *   when its data byte has gone out.
 *
 * Clock: XIN (an oscillator part; crystal or external clock is CrystalEn)
 * feeds fREF directly (PLLBypass = 1: the model counts XIN rising edges) or
 * through the predivider (1-63) and the PLL x6/x48/x96/x144 (PLLEn = 1; a
 * counted clock cannot be multiplied, so the model measures the XIN period
 * over its last two rising edges and schedules baud ticks with timers at
 * period x PreDiv / PLL x divisor, rescaling the tick in flight when the
 * period changes: plan-12-J section 3.2). PLLBypass = 0 with PLLEn = 0 has
 * no reference (max3107 p17 Figure 7: the PLL is off). The fractional
 * generator divides fREF by DIV + FRACT/16 (FRACT spread as extra fREF
 * cycles, so the average is exact); its output is 16x (8x, 4x in 2x/4x
 * mode) the baud rate. DIV = 0 stops it (P2).
 *
 * Line: 5-8 data bits, parity (odd, even, forced), 1 / 1.5 / 2 stops (the
 * receiver checks one), break, 3-sample majority with noise detection at
 * 16x (and 8x except on the MAX3107); 128-word FIFOs with per-word error
 * flags; LSR shows the flags of the word last read from RHR (p27). FIFO
 * trigger and empty interrupts, RxTimeOut, auto RTS/CTS (halt and resume
 * levels compared as MAX3108 p22: "higher than HALT", "below RESUME"),
 * XON/XOFF in both directions (one- and two-character), special
 * characters, 9-bit multidrop with address filtering, RS-485 direction
 * control with setup and hold, echo suppression, IrDA SIR/MIR (MAX3100
 * polarity: TX idles high, a 0 is a low pulse; RX idles low, a 0 is a high
 * pulse; P9), TX/RX inversion, loopback, GPIOs (push-pull or open drain;
 * inputs sampled on the reference clock, change interrupts), sleep (forced
 * and auto), the MAX3109/MAX14830 transmit triggers (GloblComnd 0xE0-0xEF,
 * TxSynch, SynchDelay, TxAutoDis), TIMER and CLKtoGPIO outputs.
 *
 * Pins: RST (active low; Z reads inactive) holds everything in reset (TX and
 * RTS high, MISO/SDA released; IRQ low, Z on the MAX3109); after it the
 * device is ready tready later (IRQ is held low until then where IRQSel
 * exists). IRQ is open drain. Internal pulls are read, not driven: RX reads
 * Z as 1 (weak pull-up), GPIO inputs read Z as 0 (weak pull-downs).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serial.h"
#include "serial_target.h"

enum { T_DO, T_FT, T_READY };
static const char *const TIMING[] = {"tdo", "tft", "tready", NULL};

static const char *const KEYS[] = {
    "variant", "bus", "reset", "clock", "channels", "interrupt", "gpio", NULL
};
static const char *const BUS_EXTRA[] = {"uartOffsets", NULL};

typedef enum { CH_TX, CH_RX, CH_RTS, CH_CTS, CH_PIN_COUNT } ChannelPin;
static const char *const CHANNEL_KEYS[] = {"tx", "rx", "rts", "cts", NULL};

enum { MX_MAX_UARTS = 4, MX_FIFO = 128, MX_GPIO = 16 };

/* ---- Variants ---------------------------------------------------------------------- */

typedef struct {
    const char *name;
    unsigned uarts;
    int revid;              /* RevID at 0x1F (MAX3107), else -1 */
    int global;             /* 0x1F GlobalIRQ / GloblComnd, extended 0x20-0x25 */
    int ext_revid;          /* RevID at 0x25 */
    int u_bits;             /* SPI command channel bits: 0, 1 (bit 5), 2 (bits 6-5) */
    int irqsel;             /* MODE1[7] IRQSel */
    int sleep;              /* MODE1[6:5] AutoSleep, ForcedSleep */
    int sts_txempty;        /* STSInt[7] TxEmptyInt */
    int sts_sleep;          /* STSInt[6] SleepInt */
    int gpi_gate;           /* STSIntEn[3:0] gate GPIxInt generation */
    int clock_en;           /* CLKSource[4] ClockEn */
    uint8_t clksource;      /* CLKSource reset */
    int rts_invert;         /* IrDA[2] RTSInvert */
    int clk_disable;        /* BRGConfig[6] CLKDisabl */
    int noise_2x;           /* 2x mode keeps the 3 samples */
    int shared_clock;       /* PLLConfig, CLKSource only through UART0 */
    int rst_per_uart;       /* MODE2[0] resets the addressed UART only */
    int irq_z_in_reset;     /* IRQ is Z while RST is low */
    int txdisabl_empty;     /* MODE1[1] reads 1 in auto direction control when empty */
    int xoff2_on_address;   /* XOFF2Int also on the own multidrop address */
    unsigned last_reg;      /* the highest valid register address */
} Variant;

static const Variant VARIANTS[] = {
    {"max3107", 1, 0xA1, 0, -1, 0, 1, 1, 0, 1, 0, 1, 0x08, 0, 0, 0, 0, 0, 0, 0, 0, 0x1F},
    {"max3108", 1, -1, 0, -1, 0, 1, 1, 1, 1, 1, 0, 0x18, 0, 0, 1, 0, 0, 0, 1, 1, 0x1E},
    {"max3109", 2, -1, 1, 0xC2, 1, 0, 1, 1, 1, 1, 0, 0x18, 0, 0, 1, 1, 1, 1, 1, 1, 0x25},
    {"max14830", 4, -1, 1, 0xB4, 2, 1, 0, 0, 0, 0, 0, 0x08, 1, 1, 1, 1, 1, 0, 0, 1, 0x25},
};

/* Registers. */
enum {
    R_RHR = 0x00, R_IRQEN, R_ISR, R_LSRINTEN, R_LSR, R_SPCLEN, R_SPCL, R_STSEN, R_STS,
    R_MODE1, R_MODE2, R_LCR, R_RXTIMEOUT, R_HDPLX, R_IRDA, R_FLOWLVL, R_FIFOTRG, R_TXLVL,
    R_RXLVL, R_FLOWCTRL, R_XON1, R_XON2, R_XOFF1, R_XOFF2, R_GPIOCFG, R_GPIODATA, R_PLLCFG,
    R_BRGCFG, R_DIVLSB, R_DIVMSB, R_CLKSRC, R_GLOBAL,
    R_TXSYNCH = 0x20, R_SYNCHDLY1, R_SYNCHDLY2, R_TIMER1, R_TIMER2, R_REVID2
};

/* MODE1 / MODE2 / LCR / CLKSource bits. */
enum {
    M1_IRQSEL = 0x80, M1_AUTOSLEEP = 0x40, M1_FORCEDSLEEP = 0x20, M1_TRNSCV = 0x10,
    M1_RTSHIZ = 0x08, M1_TXHIZ = 0x04, M1_TXDIS = 0x02, M1_RXDIS = 0x01,
    M2_ECHO = 0x80, M2_MULTIDROP = 0x40, M2_LOOP = 0x20, M2_SPECIAL = 0x10, M2_RXEMPTYINV = 0x08,
    M2_RXTRIGINV = 0x04, M2_FIFORST = 0x02, M2_RST = 0x01,
    LCR_RTS = 0x80, LCR_BREAK = 0x40, LCR_FORCE = 0x20, LCR_EVEN = 0x10, LCR_PAREN = 0x08,
    CK_CLKTORTS = 0x80, CK_CLOCKEN = 0x10, CK_BYPASS = 0x08, CK_PLLEN = 0x04, CK_CRYSTAL = 0x02,
    FC_SWFLOWEN = 0x08, FC_GPIADDR = 0x04, FC_AUTOCTS = 0x02, FC_AUTORTS = 0x01,
    IR_TXINV = 0x20, IR_RXINV = 0x10, IR_MIR = 0x08, IR_RTSINV = 0x04, IR_SIR = 0x02, IR_EN = 0x01,
    TS_CLKTOGPIO = 0x80, TS_AUTODIS = 0x40, TS_TRIGDELAY = 0x20, TS_SYNCHEN = 0x10
};

/* ISR bits; LSR bits (also each RX word's flags). */
enum {
    I_CTS = 0x80, I_RXEMPTY = 0x40, I_TXEMPTY = 0x20, I_TXTRIG = 0x10, I_RXTRIG = 0x08,
    I_STS = 0x04, I_SPCL = 0x02, I_LSR = 0x01,
    L_NOISE = 0x20, L_BREAK = 0x10, L_FRAME = 0x08, L_PARITY = 0x04, L_OVERRUN = 0x02,
    L_TIMEOUT = 0x01,
    S_XON1 = 0x01, S_XON2 = 0x02, S_XOFF1 = 0x04, S_XOFF2 = 0x08, S_BREAK = 0x10, S_MULTIDROP = 0x20,
    ST_TXEMPTY = 0x80, ST_SLEEP = 0x40, ST_CLOCKREADY = 0x20
};

/* Writable bits per register (others read 0). */
static const uint8_t WRITABLE[0x20] = {
    0xFF, 0xFF, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x6F, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF,
    0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0xFF, 0x3F, 0xFF, 0xFF, 0x9E, 0x00
};

/* Timers: id = generation << 8 | type << 4 | UART. */
enum { TM_BRG = 1, TM_HALF = 2, TM_READY = 3, TM_REFCLK = 4 };

/* ---- Device data ---------------------------------------------------------------------- */

typedef struct {
    const Variant *v;
    SerialHostDevice host;
    int offsets[MX_MAX_UARTS];       /* 7-bit I2C address offset per UART */
    const char *reset, *clock_in, *clock_out, *irq;
    const char *channel[MX_MAX_UARTS][CH_PIN_COUNT];
    const char *gpio[MX_GPIO];
    unsigned gpio_count;
} MxDevice;

typedef struct {
    SerialHostPins host;
    size_t reset, clock_in, clock_out, irq;
    size_t channel[MX_MAX_UARTS][CH_PIN_COUNT];
    size_t gpio[MX_GPIO];
} MxPins;

/* ---- State ------------------------------------------------------------------------------ */

typedef struct {
    uint8_t v;
    uint8_t k;
} Reg;

typedef struct {
    uint16_t data;           /* data bits; the 9th (multidrop) bit at bit 8 */
    uint16_t known;
    uint8_t flags;           /* L_NOISE L_BREAK L_FRAME L_PARITY */
    uint8_t flags_k;
} RxWord;

typedef enum { DIR_IDLE, DIR_SETUP, DIR_SEND, DIR_HOLD } Dir;

typedef struct MxState MxState;

typedef struct {
    MxState *owner;
    unsigned index;
    SerialLine line;
    SerialFormat txf;
    SerialFormat rxf;
    unsigned factor;         /* 16, 8 or 4 */
    Reg r[0x20];
    Reg ext[6];              /* 0x20-0x25 */
    uint8_t isr;             /* latched ISR[7:3] */
    uint8_t spcl;            /* SpclCharInt */
    uint8_t sts;             /* STSInt latched bits: [7] TxEmptyInt, [3:0] GPIxInt */
    uint8_t lsr;             /* [5:2] last read word, [1] overrun, [0] timeout */
    uint8_t lsr_k;
    RxWord rx[MX_FIFO];
    unsigned rx_head, rx_count;
    uint8_t tx_d[MX_FIFO];
    uint8_t tx_k[MX_FIFO];
    unsigned tx_head, tx_count;
    /* Baud generator. */
    int run;                 /* the generator runs */
    uint32_t brg_count;      /* edge mode: fREF edges since the last tick */
    uint32_t brg_target;
    unsigned brg_acc;        /* fractional accumulator (16ths) */
    uint32_t brg_gen;        /* measured mode: the live timer generation */
    int brg_pending;
    SimTime brg_due;
    SimTime brg_delay;
    uint64_t brg_rem;
    uint32_t half_gen;
    LogicValue clkout;
    /* Transmitter. */
    LogicValue tx_level;     /* the transmitter's output */
    int tx_halted;           /* XOFF received */
    Dir dir;
    unsigned dir_count;
    uint8_t ctl[2];          /* XON/XOFF characters to send */
    unsigned ctl_n;
    int xoff_sent;
    int rts_flow;            /* auto RTS: 1 = deasserted */
    unsigned echo_ticks;     /* echo suppression after the transmitter */
    int trig_wait;           /* ticks to the triggered start, -1 none */
    int sync_go;
    /* Receiver. */
    uint64_t idle_ticks;     /* RxTimeOut */
    unsigned ir_zero;
    int have_char;
    SerialRxChar got;
    uint8_t noise;
    int held_valid;          /* the first of a two-character XON/XOFF */
    RxWord held;
    int held_kind;           /* 1 XON1, 2 XOFF1 */
    int md_match;            /* multidrop filter: the last address was ours */
    /* Timer output. */
    uint32_t timer_count;
    LogicValue timer_out;
    /* Sleep. */
    int sleeping;
    uint64_t sleep_idle;
    /* GPIO inputs (sampled) [3:0]. */
    uint8_t gpi;
    uint8_t gpi_k;
    LogicValue cts;
    int in_rst;              /* MODE2[0] held */
} Chan;

struct MxState {
    SerialBase base;
    const MxDevice *dev;
    const MxPins *pins;
    const SerialGrade *grade;
    Sim *sim;
    SimInstance *instance;
    SerialHost host;
    SerialSpiEvents spi_events;
    SerialI2cEvents i2c_events;
    Chan ch[MX_MAX_UARTS];
    LogicValue *out;
    size_t pin_count;
    int in_reset;
    int unknown;             /* RST is X */
    int ready;
    uint32_t ready_gen;
    int ext_map;             /* GloblComnd 0xCE */
    /* Clock tree. */
    LogicValue xin;
    SimTime last_rise;
    SimTime period;          /* measured XIN period; 0 = not yet */
    unsigned rises;          /* rising edges since the reference was configured */
    int ref_mode;            /* 0 none, 1 counted (bypass), 2 measured (PLL) */
    unsigned prediv;
    unsigned mult;
    int gpio_dirty;
    LogicValue refclk;       /* the reference clock as an output (CLKtoGPIO) */
    uint32_t refclk_gen;
    uint64_t refclk_rem;
    /* Host transaction. */
    int spi_rw;              /* 1 write, 0 read, -1 X */
    int spi_x;               /* the command's address is unknown */
    unsigned spi_ch;
    int spi_addr;            /* internal address of the first data byte, -1 invalid */
    int i2c_ch;              /* -1 none */
    int i2c_ptr;             /* register pointer, -1 invalid */
    int i2c_have_ptr;
};

/* ---- Small helpers ------------------------------------------------------------------------ */

static SimTime t_of(const MxState *s, unsigned i) {
    return s->grade->t[i];
}

static LogicValue not3(LogicValue a) {
    return a == LOGIC_0 ? LOGIC_1 : a == LOGIC_1 ? LOGIC_0 : LOGIC_X;
}

static LogicValue bool3(int b) {
    return b ? LOGIC_1 : LOGIC_0;
}

static uint8_t rv(const Chan *c, unsigned reg) {
    return c->r[reg].v;
}

static int bit(const Chan *c, unsigned reg, uint8_t mask) {
    return (c->r[reg].v & c->r[reg].k & mask) != 0;
}

static LogicValue level(const MxState *s, size_t pin, LogicValue z_reads) {
    return serial_level(s->sim, s->instance, pin, z_reads);
}

static void drive(MxState *s, size_t pin, LogicValue value, SimTime delay) {
    if (pin == SERIAL_NO_PIN || s->out[pin] == value) {
        return;
    }
    s->out[pin] = value;
    sim_schedule_inertial_drive(s->sim, s->instance, pin, value, DRIVE_STRONG, delay);
}

static void drive_clock(MxState *s, size_t pin, LogicValue value) {
    if (pin == SERIAL_NO_PIN || s->out[pin] == value) {
        return;
    }
    s->out[pin] = value;
    sim_schedule_drive(s->sim, s->instance, pin, value, DRIVE_STRONG, 0);
}

static void schedule(MxState *s, unsigned type, unsigned ch, uint32_t gen, SimTime delay) {
    sim_schedule_timer(s->sim, s->instance, ((gen & 0xFFFFFFu) << 8) | (type << 4) | ch, delay);
}

static Chan *clock_chan(MxState *s) {
    return &s->ch[0];
}

/* CLKtoRTS: UART0's CLKSource where the clock registers are shared. */
static int clk_to_rts(const Chan *c) {
    const Chan *k = c->owner->dev->v->shared_clock ? &c->owner->ch[0] : c;

    return (k->r[R_CLKSRC].v & k->r[R_CLKSRC].k & CK_CLKTORTS) != 0;
}

/* The bits a register write stores (others read 0). */
static uint8_t writable(const Variant *v, unsigned addr) {
    switch (addr) {
        case R_STSEN:
            return (uint8_t)(0x2F | (v->sts_sleep ? 0x40 : 0) | (v->sts_txempty ? 0x80 : 0));
        case R_BRGCFG:
            return (uint8_t)(0x3F | (v->clk_disable ? 0x40 : 0));
        default:
            return addr < 0x20 ? WRITABLE[addr] : 0xFF;
    }
}

static unsigned trigger_rx(const Chan *c) {
    return (rv(c, R_FIFOTRG) >> 4) * 8u;
}

static unsigned trigger_tx(const Chan *c) {
    return (rv(c, R_FIFOTRG) & 15u) * 8u;
}

/* ---- Interrupts --------------------------------------------------------------------------- */

static uint8_t sts_now(const Chan *c) {
    const MxState *s = c->owner;
    const Variant *v = s->dev->v;
    uint8_t b = c->sts;

    if (v->sts_sleep && c->sleeping) {
        b |= ST_SLEEP;
    }
    if (s->ref_mode != 0 && (rv(&s->ch[0], R_CLKSRC) & CK_CRYSTAL) &&
        (s->ref_mode == 1 ? s->rises > 0 : s->period > 0) && !c->sleeping) {
        b |= ST_CLOCKREADY;
    }
    return b;
}

static uint8_t lsr_now(const Chan *c) {
    return c->lsr & 0x3F;
}

static uint8_t isr_now(const Chan *c) {
    uint8_t b = c->isr & 0xF8;

    if (sts_now(c) & rv(c, R_STSEN)) {
        b |= I_STS;
    }
    if (c->spcl & rv(c, R_SPCLEN)) {
        b |= I_SPCL;
    }
    if (lsr_now(c) & rv(c, R_LSRINTEN)) {
        b |= I_LSR;
    }
    return b;
}

/* The UART has an enabled interrupt pending. */
static int pending(const Chan *c) {
    return (isr_now(c) & rv(c, R_IRQEN)) != 0;
}

static uint8_t global_irq(const MxState *s) {
    uint8_t b = 0;
    unsigned u;

    for (u = 0; u < s->dev->v->uarts; u++) {
        if (!pending(&s->ch[u])) {
            b |= (uint8_t)(1u << u);
        }
    }
    return b;
}

static LogicValue irq_pin(const MxState *s) {
    const Variant *v = s->dev->v;
    unsigned u;

    if (s->unknown) {
        return LOGIC_X;
    }
    if (s->in_reset) {
        return v->irq_z_in_reset ? LOGIC_Z : LOGIC_0;
    }
    if (v->irqsel && !s->ready) {
        return LOGIC_0;   /* held low until the device is ready (p22) */
    }
    for (u = 0; u < v->uarts; u++) {
        const Chan *c = &s->ch[u];

        if ((!v->irqsel || bit(c, R_MODE1, M1_IRQSEL)) && pending(c)) {
            return LOGIC_0;
        }
    }
    return LOGIC_Z;
}

/* ---- Formats and the baud generator -------------------------------------------------------- */

static void update_format(Chan *c) {
    const Variant *v = c->owner->dev->v;
    uint8_t lcr = rv(c, R_LCR);
    uint8_t brg = rv(c, R_BRGCFG);
    SerialFormat f = serial_format_default();
    unsigned len = 5 + (lcr & 3u);

    c->factor = (brg & 0x20) ? 4 : (brg & 0x10) ? 8 : 16;
    f.factor = c->factor;
    f.data_bits = len;
    f.stop_halves = (lcr & 4u) ? (len == 5 ? 3 : 4) : 2;
    f.majority = c->factor == 16 || (c->factor == 8 && v->noise_2x);
    if (bit(c, R_MODE2, M2_MULTIDROP)) {
        /* The 9th bit replaces parity; no parity is generated or checked. */
        f.parity = SERIAL_PARITY_NONE;
        if (lcr & LCR_PAREN) {
            f.data_bits = len + 1;
        }
    } else if (!(lcr & LCR_PAREN)) {
        f.parity = SERIAL_PARITY_NONE;
    } else if (lcr & LCR_FORCE) {
        f.parity = (lcr & LCR_EVEN) ? SERIAL_PARITY_SPACE : SERIAL_PARITY_MARK;
    } else {
        f.parity = (lcr & LCR_EVEN) ? SERIAL_PARITY_EVEN : SERIAL_PARITY_ODD;
    }
    f.format_x = (c->r[R_LCR].k & 0x3F) != 0x3F || (c->r[R_MODE2].k & M2_MULTIDROP) == 0;
    c->txf = f;
    c->rxf = f;
}

static uint32_t divisor(const Chan *c) {
    return (uint32_t)rv(c, R_DIVLSB) | ((uint32_t)rv(c, R_DIVMSB) << 8);
}

static int divisor_known(const Chan *c) {
    return c->r[R_DIVLSB].k == 0xFF && c->r[R_DIVMSB].k == 0xFF && (c->r[R_BRGCFG].k & 0x7F) == 0x7F;
}

/* The next tick's length in fREF cycles (DIV, plus one when the fraction
   carries). */
static uint32_t next_target(Chan *c) {
    uint32_t t = divisor(c);

    c->brg_acc += rv(c, R_BRGCFG) & 15u;
    if (c->brg_acc >= 16) {
        c->brg_acc -= 16;
        t++;
    }
    return t;
}

static int chan_clock_on(const Chan *c) {
    const MxState *s = c->owner;

    return s->ref_mode != 0 && !s->in_reset && !s->unknown && !c->sleeping && !c->in_rst &&
           !(s->dev->v->clk_disable && bit(c, R_BRGCFG, 0x40)) && divisor_known(c) &&
           divisor(c) != 0;
}

static SimTime tick_delay(Chan *c) {
    MxState *s = c->owner;
    uint64_t num = (uint64_t)s->period * s->prediv * next_target(c);
    uint64_t d = num / s->mult;

    c->brg_rem += num % s->mult;
    if (c->brg_rem >= s->mult) {
        c->brg_rem -= s->mult;
        d++;
    }
    return (SimTime)d;
}

static void brg_schedule(Chan *c, SimTime delay) {
    MxState *s = c->owner;

    c->brg_gen++;
    c->brg_pending = 1;
    c->brg_delay = delay;
    c->brg_due = sim_now(s->sim) + delay;
    schedule(s, TM_BRG, c->index, c->brg_gen, delay);
}

/* The generator's configuration changed: restart it. */
static void brg_restart(Chan *c) {
    MxState *s = c->owner;
    int on = chan_clock_on(c);

    c->brg_gen++;
    c->half_gen++;
    c->brg_pending = 0;
    c->brg_count = 0;
    c->brg_acc = 0;
    c->brg_rem = 0;
    c->run = on;
    if (!on) {
        return;
    }
    c->brg_target = next_target(c);
    if (s->ref_mode == 2 && s->period > 0 && sim_now(s->sim) - s->last_rise <= 2 * s->period) {
        brg_schedule(c, tick_delay(c));
    }
}

static void update_reference(MxState *s);

/* ---- Outputs -------------------------------------------------------------------------------- */

/* The internal RTS (before RTSHiZ, inversion and loopback's pin rule). */
static LogicValue rts_internal(const Chan *c) {
    const MxState *s = c->owner;

    if (s->in_reset) {
        return LOGIC_1;
    }
    if (clk_to_rts(c)) {
        return c->clkout;
    }
    if (bit(c, R_MODE1, M1_TRNSCV)) {
        return bool3(c->dir != DIR_IDLE);
    }
    if (bit(c, R_FLOWCTRL, FC_AUTORTS)) {
        return bool3(c->rts_flow);
    }
    return (c->r[R_LCR].k & LCR_RTS) ? bool3(rv(c, R_LCR) & LCR_RTS) : LOGIC_X;
}

static LogicValue cts_internal(const Chan *c) {
    if (bit(c, R_MODE2, M2_LOOP)) {
        return rts_internal(c);
    }
    return c->cts;
}

/* The TX signal as it leaves the chip (or loops back), before 3-state. */
static LogicValue tx_signal(const Chan *c) {
    LogicValue v = c->tx_level;
    const MxState *s = c->owner;

    if (s->in_reset) {
        return LOGIC_1;
    }
    if (bit(c, R_LCR, LCR_BREAK)) {
        v = LOGIC_0;
    }
    if (bit(c, R_IRDA, IR_EN) && c->factor == 16) {
        unsigned width = bit(c, R_IRDA, IR_MIR) ? 4 : 3;

        if (v != LOGIC_1 && c->line.tx.count >= width) {
            v = LOGIC_1;
        }
    }
    if (bit(c, R_IRDA, IR_TXINV)) {
        v = not3(v);
    }
    return v;
}

static void update_gpio(MxState *s) {
    unsigned i;

    for (i = 0; i < s->dev->gpio_count; i++) {
        const Chan *c = &s->ch[i / 4];
        unsigned b = i % 4;
        uint8_t cfg = rv(c, R_GPIOCFG);
        LogicValue v;

        if (s->in_reset || c->index >= s->dev->v->uarts) {
            v = LOGIC_Z;
        } else if (s->unknown) {
            v = LOGIC_X;
        } else if (b == 0 && s->dev->v->global && (c->ext[0].v & TS_CLKTOGPIO)) {
            drive_clock(s, s->pins->gpio[i], s->refclk);
            continue;
        } else if (b == 1 && s->dev->v->global && (c->ext[4].v & 0x80)) {
            drive_clock(s, s->pins->gpio[i], c->timer_out);
            continue;
        } else if ((cfg >> b) & 1u) {
            LogicValue d = (c->r[R_GPIODATA].k >> b) & 1u
                               ? bool3((rv(c, R_GPIODATA) >> b) & 1u)
                               : LOGIC_X;

            v = ((cfg >> (4 + b)) & 1u) ? (d == LOGIC_1 ? LOGIC_Z : d) : d;
        } else {
            v = LOGIC_Z;
        }
        drive(s, s->pins->gpio[i], v, 0);
    }
}

static void update_outputs(MxState *s) {
    unsigned u;

    drive(s, s->pins->irq, irq_pin(s), 0);
    for (u = 0; u < s->dev->v->uarts; u++) {
        const Chan *c = &s->ch[u];
        const size_t *pin = s->pins->channel[u];
        LogicValue tx;
        LogicValue rts;

        if (s->unknown) {
            drive(s, pin[CH_TX], LOGIC_X, 0);
            drive(s, pin[CH_RTS], LOGIC_X, 0);
            continue;
        }
        tx = tx_signal(c);
        if (!s->in_reset && (bit(c, R_MODE1, M1_TXHIZ) || bit(c, R_MODE2, M2_LOOP))) {
            tx = LOGIC_Z;
        }
        drive(s, pin[CH_TX], tx, 0);
        if (s->in_reset) {
            rts = LOGIC_1;
        } else if (bit(c, R_MODE2, M2_LOOP)) {
            rts = (c->r[R_LCR].k & LCR_RTS) ? bool3(rv(c, R_LCR) & LCR_RTS) : LOGIC_X;
        } else {
            rts = rts_internal(c);
        }
        if (!s->in_reset && s->dev->v->rts_invert && bit(c, R_IRDA, IR_RTSINV)) {
            rts = not3(rts);
        }
        if (!s->in_reset && bit(c, R_MODE1, M1_RTSHIZ)) {
            rts = LOGIC_Z;
        }
        if (clk_to_rts(c)) {
            drive_clock(s, pin[CH_RTS], rts);
        } else {
            drive(s, pin[CH_RTS], rts, 0);
        }
    }
    update_gpio(s);
}

/* ---- FIFOs and the receive path ------------------------------------------------------------------- */

static void rx_clear(Chan *c) {
    if (c->rx_count != 0 && !bit(c, R_MODE2, M2_RXEMPTYINV)) {
        c->isr |= I_RXEMPTY;
    }
    c->rx_head = 0;
    c->rx_count = 0;
    c->idle_ticks = 0;
}

static void tx_clear(Chan *c) {
    if (c->tx_count != 0) {
        c->isr |= I_TXEMPTY;
    }
    c->tx_head = 0;
    c->tx_count = 0;
    c->isr &= (uint8_t)~I_TXTRIG;
}

/* Auto RTS and XON/XOFF sending follow the RX level (MAX3108 p22 wording). */
static void flow_update(Chan *c) {
    unsigned halt = (rv(c, R_FLOWLVL) & 15u) * 8u;
    unsigned resume = (rv(c, R_FLOWLVL) >> 4) * 8u;
    unsigned sel = rv(c, R_FLOWCTRL) >> 6;   /* SwFlow[3:2] */

    if (c->rx_count > halt) {
        c->rts_flow = 1;
    } else if (c->rx_count < resume) {
        c->rts_flow = 0;
    }
    if (!bit(c, R_FLOWCTRL, FC_SWFLOWEN) || sel == 0) {
        return;
    }
    if (c->rx_count > halt && !c->xoff_sent) {
        c->xoff_sent = 1;
        c->ctl_n = 0;
        if (sel & 2u) c->ctl[c->ctl_n++] = rv(c, R_XOFF1);
        if (sel & 1u) c->ctl[c->ctl_n++] = rv(c, R_XOFF2);
    } else if (c->rx_count < resume && c->xoff_sent) {
        c->xoff_sent = 0;
        c->ctl_n = 0;
        if (sel & 2u) c->ctl[c->ctl_n++] = rv(c, R_XON1);
        if (sel & 1u) c->ctl[c->ctl_n++] = rv(c, R_XON2);
    }
}

static void rx_push(Chan *c, const RxWord *w) {
    unsigned trig = trigger_rx(c);

    if (bit(c, R_MODE2, M2_FIFORST)) {
        return;
    }
    if (c->rx_count >= MX_FIFO) {
        c->lsr |= L_OVERRUN;   /* the new word is lost */
        return;
    }
    c->rx[(c->rx_head + c->rx_count) % MX_FIFO] = *w;
    c->rx_count++;
    if (c->rx_count == 1 && bit(c, R_MODE2, M2_RXEMPTYINV)) {
        c->isr |= I_RXEMPTY;
    }
    if (c->rx_count == trig && !bit(c, R_MODE2, M2_RXTRIGINV)) {
        c->isr |= I_RXTRIG;
    }
    flow_update(c);
}

/* The multidrop / special-character address: XOFF2, or XOFF2[7:4] with the
   GPIO inputs (GPIAddr). */
static uint8_t own_address(const Chan *c) {
    if (bit(c, R_FLOWCTRL, FC_GPIADDR)) {
        return (uint8_t)((rv(c, R_XOFF2) & 0xF0) | (c->gpi & 0x0F));
    }
    return rv(c, R_XOFF2);
}

static uint8_t xoff2_compare(const Chan *c) {
    return own_address(c);
}

/* A received word through flow control, special characters and multidrop. */
static void rx_word(Chan *c, RxWord w) {
    const Variant *v = c->owner->dev->v;
    unsigned len = 5 + (rv(c, R_LCR) & 3u);
    uint8_t byte = (uint8_t)(w.data & ((1u << len) - 1));
    int known = (w.known & ((1u << len) - 1)) == ((1u << len) - 1);
    unsigned sel = (rv(c, R_FLOWCTRL) >> 4) & 3u;   /* SwFlow[1:0] */
    int flow = bit(c, R_FLOWCTRL, FC_SWFLOWEN);
    int special = bit(c, R_MODE2, M2_SPECIAL);

    if (w.flags & L_BREAK) {
        c->spcl |= S_BREAK;
        rx_push(c, &w);
        return;
    }
    if (bit(c, R_MODE2, M2_MULTIDROP) && (c->rxf.data_bits == len + 1)) {
        int address = (w.data >> len) & 1u;

        /* The 9th bit shows as LSR[2] when the word is read. */
        w.flags = (uint8_t)((w.flags & ~L_PARITY) | (address ? L_PARITY : 0));
        w.flags_k |= L_PARITY;
        w.data &= (uint16_t)((1u << len) - 1);
        if (address) {
            c->spcl |= S_MULTIDROP;
        }
        if (special) {
            if (address) {
                c->md_match = known && byte == own_address(c);
                if (c->md_match && v->xoff2_on_address) {
                    c->spcl |= S_XOFF2;
                }
                return;   /* address words are never stored */
            }
            if (!c->md_match) {
                return;
            }
        }
        rx_push(c, &w);
        return;
    }
    if (!known) {
        rx_push(c, &w);
        return;
    }
    if (flow) {
        /* Transmit flow control: what the receiver compares. With SpecialChr
           also on, XON1/XOFF1 are the flow characters. */
        unsigned fsel = special ? 2u : sel;

        if (c->held_valid) {
            int done = 0;

            if (c->held_kind == 1 && byte == rv(c, R_XON2)) {
                c->tx_halted = 0;
                done = 1;
            } else if (c->held_kind == 2 && byte == rv(c, R_XOFF2)) {
                c->tx_halted = 1;
                done = 1;
            }
            c->held_valid = 0;
            if (done) {
                return;
            }
            rx_push(c, &c->held);
        }
        if (fsel == 3) {
            if (byte == rv(c, R_XON1) || byte == rv(c, R_XOFF1)) {
                c->held_valid = 1;
                c->held = w;
                c->held_kind = byte == rv(c, R_XON1) ? 1 : 2;
                return;
            }
        } else if (fsel == 2) {
            if (byte == rv(c, R_XON1)) { c->tx_halted = 0; return; }
            if (byte == rv(c, R_XOFF1)) { c->tx_halted = 1; return; }
        } else if (fsel == 1) {
            if (byte == rv(c, R_XON2)) { c->tx_halted = 0; return; }
            if (byte == rv(c, R_XOFF2)) { c->tx_halted = 1; return; }
        }
    }
    if (special) {
        unsigned ssel = flow ? 1u : sel;   /* with flow on: XON2/XOFF2 are special */

        if ((ssel & 2u) && byte == rv(c, R_XON1)) c->spcl |= S_XON1;
        if ((ssel & 2u) && byte == rv(c, R_XOFF1)) c->spcl |= S_XOFF1;
        if ((ssel & 1u) && byte == rv(c, R_XON2)) c->spcl |= S_XON2;
        if ((ssel & 1u) && byte == xoff2_compare(c)) c->spcl |= S_XOFF2;
    }
    rx_push(c, &w);
}

static void on_received(void *context, const SerialRxChar *rc) {
    Chan *c = context;

    c->have_char = 1;
    c->got = *rc;
}

static void finish_char(Chan *c) {
    RxWord w;
    const SerialRxChar *rc = &c->got;

    c->have_char = 0;
    if (c->echo_ticks > 0 || (bit(c, R_MODE2, M2_ECHO) && serial_tx_busy(&c->line.tx))) {
        return;   /* echo suppression */
    }
    w.data = rc->data;
    w.known = rc->known;
    w.flags = 0;
    w.flags_k = L_NOISE | L_BREAK | L_FRAME | L_PARITY;
    if (rc->flags & SERIAL_RX_PE) w.flags |= L_PARITY;
    if (rc->flags & SERIAL_RX_FE) w.flags |= L_FRAME;
    if (rc->flags & SERIAL_RX_BI) w.flags |= L_BREAK;
    if (!(rc->flags_known & SERIAL_RX_PE)) w.flags_k &= (uint8_t)~L_PARITY;
    if (!(rc->flags_known & SERIAL_RX_FE)) w.flags_k &= (uint8_t)~L_FRAME;
    if (!(rc->flags_known & SERIAL_RX_BI)) w.flags_k &= (uint8_t)~L_BREAK;
    if (c->noise) w.flags |= L_NOISE;
    c->noise = 0;
    c->idle_ticks = 0;
    c->lsr &= (uint8_t)~L_TIMEOUT;
    rx_word(c, w);
}

static LogicValue irda_decode(Chan *c, LogicValue pin) {
    if (pin == LOGIC_X) {
        return LOGIC_X;
    }
    if (pin == LOGIC_1 && c->ir_zero == 0) {
        c->ir_zero = c->factor;
    }
    if (c->ir_zero > 0) {
        c->ir_zero--;
        return LOGIC_0;
    }
    return LOGIC_1;
}

static void rx_tick(Chan *c) {
    MxState *s = c->owner;
    SerialRxEvents events = {c, on_received, NULL};
    LogicValue pin = level(s, s->pins->channel[c->index][CH_RX], LOGIC_1);
    LogicValue in = serial_line_rx_input(&c->line, s->sim, s->instance, pin, &c->rxf);
    SerialRxPhase phase = c->line.rx.phase;
    unsigned bitno = c->line.rx.bit;

    if (bit(c, R_MODE2, M2_LOOP)) {
        in = tx_signal(c);
    }
    if (!c->line.sender.active || bit(c, R_MODE2, M2_LOOP)) {
        if (bit(c, R_IRDA, IR_RXINV)) {
            in = not3(in);
        }
        if (bit(c, R_IRDA, IR_EN) && c->factor == 16) {
            in = irda_decode(c, in);
        }
    }
    if (bit(c, R_MODE1, M1_RXDIS) && !serial_rx_busy(&c->line.rx)) {
        c->line.rx.last = in;
        return;   /* the receiver is off after the current word */
    }
    serial_rx_tick(&c->line.rx, in, &c->rxf, &events);
    /* Noise: the three samples of a bit disagreed (majority modes only). */
    if (c->rxf.majority && phase >= SERIAL_RX_DATA && phase <= SERIAL_RX_STOP2 &&
        (c->line.rx.phase != phase || c->line.rx.bit != bitno)) {
        const LogicValue *h = c->line.rx.history;

        if (h[0] != h[1] || h[1] != h[2]) {
            c->noise = 1;
        }
    }
    if (c->have_char) {
        int framing = (c->got.flags & SERIAL_RX_FE) && !(c->got.flags & SERIAL_RX_BI);

        finish_char(c);
        if (framing) {
            /* After a framing error the receiver waits for a new start bit
               (the MAX3100 rule; P9). */
            c->line.rx.phase = SERIAL_RX_IDLE;
        }
    }
    if (phase == SERIAL_RX_IDLE && c->line.rx.phase == SERIAL_RX_START) {
        c->noise = 0;
    }
}

/* ---- The transmit path ------------------------------------------------------------------------------ */

static int tx_fetch(void *context, uint16_t *data, uint16_t *known) {
    Chan *c = context;
    unsigned len = 5 + (rv(c, R_LCR) & 3u);
    unsigned before;

    if (c->in_rst || bit(c, R_MODE2, M2_FIFORST)) {
        return 0;
    }
    if (bit(c, R_MODE1, M1_TXDIS) && !c->sync_go) {
        return 0;
    }
    if (c->ctl_n > 0) {
        *data = c->ctl[0];
        *known = 0xFF;
        c->ctl[0] = c->ctl[1];
        c->ctl_n--;
        goto ninth;
    }
    if (c->tx_count == 0 || c->tx_halted) {
        return 0;
    }
    if (bit(c, R_FLOWCTRL, FC_AUTOCTS) && cts_internal(c) != LOGIC_0) {
        return 0;
    }
    if (bit(c, R_MODE1, M1_TRNSCV)) {
        if (c->dir == DIR_IDLE) {
            unsigned setup = rv(c, R_HDPLX) >> 4;

            c->dir = DIR_SETUP;
            c->dir_count = setup * c->factor;
        }
        if (c->dir == DIR_SETUP && c->dir_count > 0) {
            return 0;
        }
        c->dir = DIR_SEND;
    }
    before = c->tx_count;
    *data = c->tx_d[c->tx_head];
    *known = c->tx_k[c->tx_head];
    c->tx_head = (c->tx_head + 1) % MX_FIFO;
    c->tx_count--;
    if (c->tx_count == 0) {
        c->isr |= I_TXEMPTY;   /* the transmitter takes the last word */
        if (c->owner->dev->v->global && (c->ext[0].v & TS_AUTODIS)) {
            c->r[R_MODE1].v |= M1_TXDIS;
            c->r[R_MODE1].k |= M1_TXDIS;
            c->sync_go = 0;
        }
    }
    if (before >= trigger_tx(c) && c->tx_count < trigger_tx(c)) {
        c->isr &= (uint8_t)~I_TXTRIG;
    }
ninth:
    if (bit(c, R_MODE2, M2_MULTIDROP) && c->txf.data_bits == len + 1) {
        /* The 9th bit: forced high with EvenParity = 0, low with 1 (p19). */
        if (bit(c, R_LCR, LCR_FORCE)) {
            if (!bit(c, R_LCR, LCR_EVEN)) {
                *data |= (uint16_t)(1u << len);
            }
            *known |= (uint16_t)(1u << len);
        } else {
            *known &= (uint16_t)~(1u << len);
        }
    }
    return 1;
}

static void tx_done(void *context, uint16_t data, uint16_t known) {
    Chan *c = context;

    (void)known;
    serial_line_log(&c->line, data);
    if (c->tx_count == 0 && c->ctl_n == 0) {
        if (c->owner->dev->v->sts_txempty) {
            c->sts |= ST_TXEMPTY;
        }
        if (bit(c, R_MODE1, M1_TRNSCV) && c->dir == DIR_SEND) {
            unsigned hold = rv(c, R_HDPLX) & 15u;

            c->dir = hold > 0 ? DIR_HOLD : DIR_IDLE;
            c->dir_count = hold * c->factor;
        }
        if (bit(c, R_MODE2, M2_ECHO)) {
            c->echo_ticks = (rv(c, R_HDPLX) & 15u) * c->factor;
        }
    }
}

static void tx_tick(Chan *c) {
    SerialTxEvents events = {c, tx_fetch, NULL, tx_done};

    if (c->dir == DIR_SETUP && c->dir_count > 0) {
        c->dir_count--;
    }
    if (c->dir == DIR_HOLD) {
        if (c->dir_count > 0) {
            c->dir_count--;
        }
        if (c->dir_count == 0) {
            c->dir = DIR_IDLE;
        }
    }
    c->tx_level = serial_tx_tick(&c->line.tx, &c->txf, &events);
    if (c->echo_ticks > 0 && !serial_tx_busy(&c->line.tx)) {
        c->echo_ticks--;
    }
}

/* ---- One baud tick ------------------------------------------------------------------------------------ */

static int sleep_quiet(const Chan *c) {
    return c->rx_count == 0 && c->tx_count == 0 && !pending(c) && !serial_tx_busy(&c->line.tx) &&
           !serial_rx_busy(&c->line.rx);
}

static void chan_tick(Chan *c) {
    MxState *s = c->owner;
    uint32_t timer = ((uint32_t)(c->ext[4].v & 0x7F) << 8) | c->ext[3].v;

    if (c->trig_wait > 0 && --c->trig_wait == 0) {
        c->trig_wait = -1;
        c->sync_go = 1;
        c->r[R_MODE1].v &= (uint8_t)~M1_TXDIS;
    }
    if (s->dev->v->global && timer != 0) {
        if (++c->timer_count >= 512u * timer) {
            c->timer_count = 0;
            c->timer_out = c->timer_out == LOGIC_1 ? LOGIC_0 : LOGIC_1;
        }
    }
    tx_tick(c);
    rx_tick(c);
    /* RxTimeOut: the youngest word has waited this many character times. */
    if (c->rx_count > 0 && rv(c, R_RXTIMEOUT) != 0 && !(c->lsr & L_TIMEOUT)) {
        uint64_t limit = (uint64_t)rv(c, R_RXTIMEOUT) * serial_format_char_bits(&c->rxf) * c->factor;

        if (++c->idle_ticks >= limit) {
            c->lsr |= L_TIMEOUT;
        }
    }
    if (s->dev->v->sleep && bit(c, R_MODE1, M1_AUTOSLEEP)) {
        if (sleep_quiet(c)) {
            if (++c->sleep_idle >= 65536ull * serial_format_char_bits(&c->rxf) * c->factor) {
                c->sleeping = 1;
                c->sleep_idle = 0;
                brg_restart(c);
                update_reference(s);
            }
        } else {
            c->sleep_idle = 0;
        }
    }
    update_outputs(s);
}

/* ---- The reference clock ------------------------------------------------------------------------------- */

static void update_reference(MxState *s) {
    Chan *c0 = clock_chan(s);
    uint8_t ck = rv(c0, R_CLKSRC);
    uint8_t pll = rv(c0, R_PLLCFG);
    static const unsigned MULT[4] = {6, 48, 96, 144};
    int mode = 0;
    int all_asleep = 1;
    unsigned u;

    for (u = 0; u < s->dev->v->uarts; u++) {
        all_asleep &= s->ch[u].sleeping;
    }
    if (s->in_reset || s->unknown || all_asleep || c0->r[R_CLKSRC].k != 0xFF ||
        c0->r[R_PLLCFG].k != 0xFF) {
        mode = 0;
    } else if (s->dev->v->clock_en && !(ck & CK_CLOCKEN)) {
        mode = 0;
    } else if (ck & CK_BYPASS) {
        mode = 1;
    } else if ((ck & CK_PLLEN) && (pll & 0x3F) != 0) {
        mode = 2;
    }
    if (mode != s->ref_mode || s->prediv != (pll & 0x3Fu) || s->mult != MULT[pll >> 6]) {
        s->ref_mode = mode;
        s->prediv = pll & 0x3Fu;
        s->mult = MULT[pll >> 6];
        s->rises = 0;
        if (mode != 2) {
            s->period = 0;
        }
        s->refclk_gen++;
        for (u = 0; u < s->dev->v->uarts; u++) {
            brg_restart(&s->ch[u]);
        }
    } else {
        for (u = 0; u < s->dev->v->uarts; u++) {
            if (s->ch[u].run != chan_clock_on(&s->ch[u])) {
                brg_restart(&s->ch[u]);
            }
        }
    }
}

static int refclk_wanted(const MxState *s) {
    unsigned u;

    if (!s->dev->v->global) {
        return 0;
    }
    for (u = 0; u < s->dev->v->uarts; u++) {
        if (s->ch[u].ext[0].v & TS_CLKTOGPIO) {
            return 1;
        }
    }
    return 0;
}

/* The measured reference as a clock (CLKtoGPIO in PLL mode): half periods. */
static SimTime refclk_half(MxState *s) {
    uint64_t num = (uint64_t)s->period * s->prediv;
    uint64_t den = (uint64_t)s->mult * 2;
    uint64_t d = num / den;

    s->refclk_rem += num % den;
    if (s->refclk_rem >= den) {
        s->refclk_rem -= den;
        d++;
    }
    return (SimTime)(d == 0 ? 1 : d);
}

static void sample_gpio(MxState *s) {
    unsigned i;

    s->gpio_dirty = 0;
    for (i = 0; i < s->dev->gpio_count; i++) {
        Chan *c = &s->ch[i / 4];
        unsigned b = i % 4;
        LogicValue v = level(s, s->pins->gpio[i], LOGIC_0);
        uint8_t m = (uint8_t)(1u << b);
        int input = !((rv(c, R_GPIOCFG) >> b) & 1u);
        uint8_t old_v = c->gpi & m;
        uint8_t old_k = c->gpi_k & m;

        if (c->sleeping) {
            continue;
        }
        c->gpi = (uint8_t)((c->gpi & ~m) | (v == LOGIC_1 ? m : 0));
        c->gpi_k = (uint8_t)((c->gpi_k & ~m) | (v != LOGIC_X ? m : 0));
        if (input && (old_v != (c->gpi & m) || old_k != (c->gpi_k & m))) {
            if (!s->dev->v->gpi_gate || (rv(c, R_STSEN) & m)) {
                c->sts |= m;
            }
        }
    }
}

static void brg_edge(Chan *c, int rising, LogicValue level_now) {
    if (!c->run) {
        return;
    }
    if (c->brg_target <= 1 && clk_to_rts(c)) {
        c->clkout = level_now;   /* divisor 1: the clock passes through */
    }
    if (!rising) {
        return;
    }
    if (++c->brg_count >= c->brg_target) {
        c->brg_count = 0;
        c->brg_target = next_target(c);
        c->clkout = LOGIC_1;
        chan_tick(c);
    } else if (c->brg_count >= (c->brg_target + 1) / 2) {
        c->clkout = LOGIC_0;
    }
}

static void xin_edge(MxState *s, LogicValue v) {
    int rising = s->xin == LOGIC_0 && v == LOGIC_1;
    int falling = s->xin == LOGIC_1 && v == LOGIC_0;
    SimTime now = sim_now(s->sim);
    unsigned u;

    s->xin = v;
    if (!rising && !falling) {
        return;
    }
    if (s->ref_mode == 1) {
        s->refclk = v;
    }
    if (rising) {
        SimTime p = s->rises > 0 ? now - s->last_rise : 0;

        s->last_rise = now;
        s->rises++;
        if (s->gpio_dirty && s->ref_mode != 0) {
            sample_gpio(s);
        }
        if (s->ref_mode == 2 && p > 0) {
            SimTime old = s->period;

            s->period = p;
            for (u = 0; u < s->dev->v->uarts; u++) {
                Chan *c = &s->ch[u];

                if (!c->run) {
                    continue;
                }
                if (!c->brg_pending) {
                    brg_schedule(c, tick_delay(c));
                } else if (old != 0 && old != p) {
                    /* The clock changed: rescale what is left of this tick. */
                    SimTime left = c->brg_due > now ? c->brg_due - now : 0;

                    brg_schedule(c, (SimTime)((uint64_t)left * p / old));
                }
            }
            if (refclk_wanted(s) && old == 0) {
                schedule(s, TM_REFCLK, 0, s->refclk_gen, refclk_half(s));
            }
        }
    }
    if (s->ref_mode == 1) {
        for (u = 0; u < s->dev->v->uarts; u++) {
            brg_edge(&s->ch[u], rising, v);
        }
    }
    update_outputs(s);
}

/* ---- Reset ----------------------------------------------------------------------------------------------- */

static void reset_chan(Chan *c) {
    const Variant *v = c->owner->dev->v;
    unsigned i;

    for (i = 0; i < 0x20; i++) {
        c->r[i].v = 0;
        c->r[i].k = 0xFF;
    }
    for (i = 0; i < 6; i++) {
        c->ext[i].v = 0;
        c->ext[i].k = 0xFF;
    }
    c->r[R_LCR].v = 0x05;
    c->r[R_FIFOTRG].v = 0xFF;
    c->r[R_PLLCFG].v = 0x01;
    c->r[R_DIVLSB].v = 0x01;
    c->r[R_CLKSRC].v = v->clksource;
    c->isr = I_RXEMPTY | I_TXEMPTY;
    c->spcl = 0;
    c->sts = 0;
    c->lsr = 0;
    c->lsr_k = 0xFF;
    c->rx_head = c->rx_count = 0;
    c->tx_head = c->tx_count = 0;
    c->tx_level = LOGIC_1;
    c->tx_halted = 0;
    c->dir = DIR_IDLE;
    c->dir_count = 0;
    c->ctl_n = 0;
    c->xoff_sent = 0;
    c->rts_flow = 0;
    c->echo_ticks = 0;
    c->trig_wait = -1;
    c->sync_go = 0;
    c->idle_ticks = 0;
    c->ir_zero = 0;
    c->have_char = 0;
    c->noise = 0;
    c->held_valid = 0;
    c->md_match = 0;
    c->timer_count = 0;
    c->timer_out = LOGIC_0;
    c->sleeping = 0;
    c->sleep_idle = 0;
    c->clkout = LOGIC_1;
    serial_tx_reset(&c->line.tx);
    serial_rx_reset(&c->line.rx);
    update_format(c);
}

static void ready_now(MxState *s) {
    s->ready = 1;
    update_outputs(s);
}

static void apply_reset_pin(MxState *s) {
    LogicValue v = level(s, s->pins->reset, LOGIC_1);
    unsigned u;

    if (v == LOGIC_X) {
        s->unknown = 1;
        s->in_reset = 0;
        serial_host_hold(&s->host, s->sim, s->instance, 1);
    } else if (v == LOGIC_0) {
        s->unknown = 0;
        s->in_reset = 1;
        s->ready = 0;
        s->ready_gen++;
        s->ext_map = 0;
        for (u = 0; u < s->dev->v->uarts; u++) {
            reset_chan(&s->ch[u]);
            s->ch[u].in_rst = 0;
            serial_line_sender_reset(&s->ch[u].line, s->sim, s->instance);
        }
        serial_host_hold(&s->host, s->sim, s->instance, 1);
    } else if (s->in_reset || s->unknown) {
        s->in_reset = 0;
        s->unknown = 0;
        s->ready = 0;
        s->ready_gen++;
        if (t_of(s, T_READY) == 0) {
            s->ready = 1;
        } else {
            schedule(s, TM_READY, 0, s->ready_gen, t_of(s, T_READY));
        }
        serial_host_hold(&s->host, s->sim, s->instance, 0);
    }
    update_reference(s);
    update_outputs(s);
}

/* ---- Registers ---------------------------------------------------------------------------------------------- */

static int reg_valid(const MxState *s, unsigned addr) {
    const Variant *v = s->dev->v;

    if (addr > v->last_reg) {
        return 0;
    }
    return 1;
}

/* Shared clock registers through a UART other than UART0. */
static int shared_elsewhere(const MxState *s, const Chan *c, unsigned addr) {
    return s->dev->v->shared_clock && c->index != 0 && (addr == R_PLLCFG || addr == R_CLKSRC);
}

static void reg_peek(MxState *s, Chan *c, unsigned addr, uint8_t *v, uint8_t *k) {
    const Variant *var = s->dev->v;

    *v = 0;
    *k = 0xFF;
    if (!reg_valid(s, addr) || shared_elsewhere(s, c, addr)) {
        *k = 0;
        return;
    }
    if (addr >= 0x20) {
        if (addr == R_REVID2) {
            *v = (uint8_t)var->ext_revid;
        } else {
            *v = c->ext[addr - 0x20].v;
            *k = c->ext[addr - 0x20].k;
        }
        return;
    }
    switch (addr) {
        case R_RHR:
            if (c->rx_count == 0) {
                *k = 0;
            } else {
                *v = (uint8_t)c->rx[c->rx_head].data;
                *k = (uint8_t)c->rx[c->rx_head].known;
            }
            return;
        case R_ISR: *v = isr_now(c); return;
        case R_LSR: {
            LogicValue cts = c->cts;

            *v = lsr_now(c);
            *k = (uint8_t)(c->lsr_k | 0xC3);
            if (cts == LOGIC_1) *v |= 0x80;
            if (cts == LOGIC_X) *k &= 0x7F;
            return;
        }
        case R_SPCL: *v = c->spcl; return;
        case R_STS: {
            uint8_t mask = 0x0F | ST_CLOCKREADY;

            if (var->sts_txempty) mask |= ST_TXEMPTY;
            if (var->sts_sleep) mask |= ST_SLEEP;
            *v = sts_now(c) & mask;
            return;
        }
        case R_MODE1: {
            uint8_t m = 0x1F;

            if (var->irqsel) m |= M1_IRQSEL;
            if (var->sleep) m |= M1_AUTOSLEEP | M1_FORCEDSLEEP;
            *v = rv(c, R_MODE1) & m;
            *k = (uint8_t)(c->r[R_MODE1].k | (uint8_t)~m);
            if (var->sleep) {
                *v = (uint8_t)((*v & ~M1_FORCEDSLEEP) | (c->sleeping ? M1_FORCEDSLEEP : 0));
                *k |= M1_FORCEDSLEEP;
            }
            if (var->txdisabl_empty && bit(c, R_MODE1, M1_TRNSCV) && c->tx_count == 0 &&
                !serial_tx_busy(&c->line.tx)) {
                *v |= M1_TXDIS;
                *k |= M1_TXDIS;
            }
            return;
        }
        case R_TXLVL: *v = (uint8_t)(c->tx_count > 255 ? 255 : c->tx_count); return;
        case R_RXLVL: *v = (uint8_t)(c->rx_count > 255 ? 255 : c->rx_count); return;
        case R_GPIODATA:
            *v = (uint8_t)((c->gpi << 4) | (rv(c, R_GPIODATA) & 0x0F));
            *k = (uint8_t)((c->gpi_k << 4) | (c->r[R_GPIODATA].k & 0x0F));
            return;
        case R_GLOBAL:
            if (var->global) {
                *v = global_irq(s);
            } else if (var->revid >= 0) {
                *v = (uint8_t)var->revid;
            } else {
                *k = 0;
            }
            return;
        default: {
            uint8_t m = writable(var, addr);

            *v = rv(c, addr) & m;
            *k = (uint8_t)(c->r[addr].k | (uint8_t)~m);
            return;
        }
    }
}

static void reg_read_done(MxState *s, Chan *c, unsigned addr) {
    if (!reg_valid(s, addr) || shared_elsewhere(s, c, addr)) {
        return;
    }
    switch (addr) {
        case R_RHR:
            if (c->rx_count > 0) {
                const RxWord *w = &c->rx[c->rx_head];
                unsigned trig = trigger_rx(c);

                c->lsr = (uint8_t)((c->lsr & 0x03) | (w->flags & 0x3C));
                c->lsr_k = (uint8_t)((c->lsr_k & ~0x3C) | (w->flags_k & 0x3C));
                c->rx_head = (c->rx_head + 1) % MX_FIFO;
                c->rx_count--;
                if (c->rx_count == 0 && !bit(c, R_MODE2, M2_RXEMPTYINV)) {
                    c->isr |= I_RXEMPTY;
                }
                if (c->rx_count == trig && bit(c, R_MODE2, M2_RXTRIGINV)) {
                    c->isr |= I_RXTRIG;
                }
                flow_update(c);
            }
            c->lsr &= (uint8_t)~(L_OVERRUN | L_TIMEOUT);
            c->idle_ticks = 0;
            return;
        case R_ISR:
            c->isr = 0;
            return;
        case R_LSR:
            c->lsr &= (uint8_t)~L_OVERRUN;
            return;
        case R_SPCL:
            c->spcl = 0;
            return;
        case R_STS:
            c->sts = 0;
            return;
        default:
            return;
    }
}

static void software_reset(MxState *s, Chan *c) {
    unsigned u;

    for (u = 0; u < s->dev->v->uarts; u++) {
        Chan *t = &s->ch[u];

        if (s->dev->v->rst_per_uart && t != c) {
            continue;
        }
        reset_chan(t);
        t->r[R_MODE2].v = M2_RST;
        t->in_rst = 1;
    }
}

static void global_command(MxState *s, uint8_t cmd) {
    unsigned u;

    if (cmd == 0xCE) {
        s->ext_map = 1;
    } else if (cmd == 0xCD) {
        s->ext_map = 0;
    } else if ((cmd & 0xF0) == 0xE0) {
        for (u = 0; u < s->dev->v->uarts; u++) {
            Chan *c = &s->ch[u];
            uint8_t ts = c->ext[0].v;

            if ((ts & TS_SYNCHEN) && (ts & 15u) == (cmd & 15u) && bit(c, R_MODE1, M1_TXDIS) &&
                c->tx_count > 0) {
                /* Intrinsic delay 5 to 6 UART clocks (p21), plus SynchDelay
                   bit times with TrigDelay. */
                unsigned delay = 5;

                if (ts & TS_TRIGDELAY) {
                    delay += ((unsigned)c->ext[1].v | ((unsigned)c->ext[2].v << 8)) * c->factor;
                }
                c->trig_wait = (int)delay;
            }
        }
    }
}

static void write_thr(Chan *c, uint8_t data, uint8_t known) {
    unsigned at;
    unsigned trig = trigger_tx(c);

    if (bit(c, R_MODE2, M2_FIFORST) || c->tx_count >= MX_FIFO) {
        return;
    }
    at = (c->tx_head + c->tx_count) % MX_FIFO;
    c->tx_d[at] = data;
    c->tx_k[at] = known;
    c->tx_count++;
    if (c->tx_count == trig) {
        c->isr |= I_TXTRIG;
    }
    c->sts &= (uint8_t)~ST_TXEMPTY;
}

static void reg_write(MxState *s, Chan *c, unsigned addr, uint8_t data, uint8_t known) {
    const Variant *var = s->dev->v;

    if (!reg_valid(s, addr) || shared_elsewhere(s, c, addr)) {
        return;   /* undefined writes are ignored (P2) */
    }
    if (c->in_rst && addr != R_MODE2) {
        return;   /* held in reset by MODE2[0] */
    }
    if (addr >= 0x20) {
        if (addr == R_REVID2) {
            return;
        }
        c->ext[addr - 0x20].v = data;
        c->ext[addr - 0x20].k = known;
        if (addr == R_TXSYNCH && (data & TS_SYNCHEN)) {
            c->r[R_MODE1].v |= M1_TXDIS;   /* SynchEn forces TxDisabl */
            c->r[R_MODE1].k |= M1_TXDIS;
            c->sync_go = 0;
        }
        if (addr == R_TXSYNCH && (known & TS_CLKTOGPIO) && (data & TS_CLKTOGPIO) &&
            s->ref_mode == 2 && s->period > 0) {
            s->refclk_gen++;
            schedule(s, TM_REFCLK, 0, s->refclk_gen, refclk_half(s));
        }
        return;
    }
    switch (addr) {
        case R_RHR:
            write_thr(c, data, known);
            return;
        case R_ISR: case R_LSR: case R_SPCL: case R_STS: case R_TXLVL: case R_RXLVL:
            return;
        case R_GLOBAL:
            if (var->global && known == 0xFF) {
                global_command(s, data);
            }
            return;
        case R_MODE2: {
            int was_rst = c->in_rst;

            if ((known & M2_RST) && (data & M2_RST)) {
                if (!was_rst) {
                    software_reset(s, c);
                    update_reference(s);
                }
                return;
            }
            if (was_rst) {
                unsigned u;

                for (u = 0; u < var->uarts; u++) {
                    if (s->ch[u].in_rst && (!var->rst_per_uart || &s->ch[u] == c)) {
                        s->ch[u].in_rst = 0;
                        s->ch[u].r[R_MODE2].v = 0;
                    }
                }
                update_reference(s);
            }
            c->r[R_MODE2].v = data;
            c->r[R_MODE2].k = known;
            if (data & M2_FIFORST) {
                rx_clear(c);
                tx_clear(c);
            }
            update_format(c);
            return;
        }
        case R_MODE1: {
            uint8_t m = 0x1F;

            if (var->irqsel) m |= M1_IRQSEL;
            if (var->sleep) m |= M1_AUTOSLEEP | M1_FORCEDSLEEP;
            c->r[R_MODE1].v = data & m;
            c->r[R_MODE1].k = (uint8_t)(known | (uint8_t)~m);
            if (!(data & M1_TXDIS)) {
                c->sync_go = 0;
            }
            if (var->sleep) {
                int want = (data & M1_FORCEDSLEEP) != 0;

                if (want != c->sleeping) {
                    c->sleeping = want;
                    c->sleep_idle = 0;
                    update_reference(s);
                    brg_restart(c);
                }
            }
            return;
        }
        default:
            break;
    }
    c->r[addr].v = data & writable(var, addr);
    c->r[addr].k = (uint8_t)(known | (uint8_t)~writable(var, addr));
    switch (addr) {
        case R_LCR:
        case R_BRGCFG:
            update_format(c);
            if (addr == R_BRGCFG) {
                brg_restart(c);
            }
            break;
        case R_DIVLSB:
        case R_DIVMSB:
            brg_restart(c);
            break;
        case R_PLLCFG:
        case R_CLKSRC:
            update_reference(s);
            break;
        case R_FLOWLVL:
        case R_FLOWCTRL:
            flow_update(c);
            break;
        case R_GPIOCFG:
            s->gpio_dirty = 1;
            break;
        default:
            break;
    }
}

/* ---- Host port ----------------------------------------------------------------------------------------------- */

static void after_access(MxState *s) {
    update_outputs(s);
}

/* The SPI command byte: channel and internal address. */
static void spi_decode(MxState *s, uint8_t cmd, uint8_t known) {
    const Variant *v = s->dev->v;
    unsigned a;

    s->spi_rw = (known & 0x80) ? (cmd >> 7) : -1;
    s->spi_x = 0;
    s->spi_ch = 0;
    switch (v->u_bits) {
        case 0:
            s->spi_x = (known & 0x7F) != 0x7F;
            a = cmd & 0x7Fu;
            break;
        case 1:
            s->spi_x = (known & 0x7F) != 0x7F;
            if (cmd & 0x40) {
                s->spi_addr = -1;   /* bit 6 must be 0 */
                return;
            }
            s->spi_ch = (cmd >> 5) & 1u;
            a = cmd & 0x1Fu;
            break;
        default:
            s->spi_x = (known & 0x7F) != 0x7F;
            s->spi_ch = (cmd >> 5) & 3u;
            a = cmd & 0x1Fu;
            break;
    }
    if (v->global && s->ext_map && a != R_GLOBAL) {
        /* 0010 A3..A0, A4 ignored (p53); 0x1F still reaches GloblComnd
           ("sends the 0xCD command to register 0x1F"). */
        a = 0x20u | (a & 0x0Fu);
    }
    s->spi_addr = reg_valid(s, a) ? (int)a : -1;
}

static int spi_burst_addr(const MxState *s, unsigned n) {
    unsigned a;

    if (s->spi_addr < 0) {
        return -1;
    }
    if (s->spi_addr == 0) {
        return 0;
    }
    a = (unsigned)s->spi_addr + n;
    return reg_valid(s, a) ? (int)a : -1;
}

static void spi_begin(void *context) {
    MxState *s = context;

    s->spi_rw = -1;
    s->spi_x = 1;
    s->spi_addr = -1;
}

static SerialSpiByte spi_next(void *context, unsigned index) {
    MxState *s = context;
    SerialSpiByte b = {0, 0, 0};
    int a;

    if (index == 0) {
        return b;   /* Z during the command byte (set in spi_bit) */
    }
    if (s->spi_rw == 1) {
        return b;   /* Z during writes */
    }
    b.drive = 0xFF;
    if (s->spi_rw < 0 || s->spi_x) {
        return b;   /* X */
    }
    a = spi_burst_addr(s, index - 1);
    if (a < 0) {
        return b;
    }
    reg_peek(s, &s->ch[s->spi_ch], (unsigned)a, &b.data, &b.known);
    return b;
}

static void spi_bit(void *context, unsigned index, unsigned bits, uint8_t data, uint8_t known,
                    SerialSpiByte *out) {
    MxState *s = context;

    if (index != 0 || bits != 1 || !s->dev->v->global) {
        return;
    }
    /* MAX3109 / MAX14830: a read drives the GlobalIRQ bits on clocks 5-8. */
    if (!(known & 1u)) {
        out->drive = 0x0F;
        out->known = 0;
    } else if (!(data & 1u)) {
        out->drive = 0x0F;
        out->data = (uint8_t)(global_irq(s) & 0x0F);
        out->known = 0x0F;
    }
}

static void spi_received(void *context, unsigned index, uint8_t data, uint8_t known) {
    MxState *s = context;
    unsigned u;
    int a;

    if (index == 0) {
        spi_decode(s, data, known);
        return;
    }
    if (s->spi_rw < 0 || s->spi_x) {
        /* An unknown command: a write stores X in every register it could
           name; a read has no defined side effect. */
        if (s->spi_rw != 0) {
            for (u = 0; u < s->dev->v->uarts; u++) {
                unsigned r;

                /* THR included: an X word goes into the TX FIFO. A
                   GloblComnd of X bits is no command. */
                for (r = 0; r <= s->dev->v->last_reg; r++) {
                    reg_write(s, &s->ch[u], r, 0, 0);
                }
            }
        }
        after_access(s);
        return;
    }
    a = spi_burst_addr(s, index - 1);
    if (a < 0) {
        return;
    }
    if (s->spi_rw == 1) {
        reg_write(s, &s->ch[s->spi_ch], (unsigned)a, data, known);
    } else {
        reg_read_done(s, &s->ch[s->spi_ch], (unsigned)a);
    }
    after_access(s);
}

/* I2C: which UART answers an address byte. */
static LogicValue i2c_address(void *context, uint8_t byte, uint8_t known) {
    MxState *s = context;
    int base = serial_host_i2c_address(&s->host);
    unsigned addr = byte >> 1;
    unsigned u;

    s->i2c_ch = -1;
    if (base < 0) {
        return LOGIC_X;   /* an unknown or untabled strap */
    }
    for (u = 0; u < s->dev->v->uarts; u++) {
        int mine = base + s->dev->offsets[u];

        if (mine < 0 || mine > 127) {
            continue;
        }
        if ((known & 0xFE) != 0xFE) {
            if ((((unsigned)mine ^ addr) & (known >> 1)) == 0) {
                return LOGIC_X;
            }
            continue;
        }
        if ((unsigned)mine == addr) {
            s->i2c_ch = (int)u;
            return LOGIC_1;
        }
    }
    return LOGIC_0;
}

static void i2c_begin(void *context, int repeated) {
    MxState *s = context;

    if (!repeated) {
        s->i2c_have_ptr = 0;
    }
}

static int i2c_next_ptr(int ptr) {
    return ptr == 0 ? 0 : ptr + 1;
}

static LogicValue i2c_received(void *context, unsigned index, uint8_t data, uint8_t known) {
    MxState *s = context;
    Chan *c;

    (void)index;
    if (s->i2c_ch < 0) {
        return LOGIC_0;
    }
    c = &s->ch[s->i2c_ch];
    if (!s->i2c_have_ptr) {
        s->i2c_have_ptr = 1;
        if (known != 0xFF) {
            s->i2c_ptr = -1;
            return LOGIC_X;
        }
        s->i2c_ptr = reg_valid(s, data) ? data : -1;
        return bool3(s->i2c_ptr >= 0);   /* ACK only for a valid register */
    }
    if (s->i2c_ptr < 0 || !reg_valid(s, (unsigned)s->i2c_ptr)) {
        return LOGIC_0;
    }
    reg_write(s, c, (unsigned)s->i2c_ptr, data, known);
    s->i2c_ptr = i2c_next_ptr(s->i2c_ptr);
    after_access(s);
    return LOGIC_1;
}

static void i2c_next(void *context, unsigned index, uint8_t *data, uint8_t *known) {
    MxState *s = context;

    (void)index;
    *data = 0;
    *known = 0;
    if (s->i2c_ch < 0 || s->i2c_ptr < 0 || !reg_valid(s, (unsigned)s->i2c_ptr)) {
        return;
    }
    reg_peek(s, &s->ch[s->i2c_ch], (unsigned)s->i2c_ptr, data, known);
}

static void i2c_acked(void *context, unsigned index, LogicValue ack) {
    MxState *s = context;

    (void)index;
    (void)ack;
    if (s->i2c_ch < 0 || s->i2c_ptr < 0 || !reg_valid(s, (unsigned)s->i2c_ptr)) {
        return;
    }
    reg_read_done(s, &s->ch[s->i2c_ch], (unsigned)s->i2c_ptr);
    s->i2c_ptr = i2c_next_ptr(s->i2c_ptr);
    after_access(s);
}

/* ---- Engine callbacks --------------------------------------------------------------------------------------- */

static void mx_start(Sim *sim, SimInstance *instance, void *raw) {
    MxState *s = raw;
    unsigned u;

    s->sim = sim;
    s->instance = instance;
    s->xin = level(s, s->pins->clock_in, LOGIC_X);
    for (u = 0; u < s->dev->v->uarts; u++) {
        s->ch[u].cts = level(s, s->pins->channel[u][CH_CTS], LOGIC_X);
    }
    serial_host_start(&s->host, sim, instance);
    s->in_reset = 1;   /* power-on: the reset state, then the pin applies */
    apply_reset_pin(s);
    s->gpio_dirty = 1;
    update_outputs(s);
}

static void wake(Chan *c) {
    MxState *s = c->owner;

    c->sleep_idle = 0;
    if (c->sleeping && !bit(c, R_MODE1, M1_FORCEDSLEEP)) {
        c->sleeping = 0;
        update_reference(s);
        brg_restart(c);
    }
}

static void mx_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    MxState *s = raw;
    unsigned u;
    unsigned i;

    (void)old_value;
    (void)new_value;
    s->sim = sim;
    s->instance = instance;
    if (pin == s->pins->reset) {
        apply_reset_pin(s);
        return;
    }
    if (pin == s->pins->clock_in) {
        xin_edge(s, level(s, pin, LOGIC_X));
    }
    if (serial_host_has_pin(&s->dev->host, &s->pins->host, pin)) {
        serial_host_pin(&s->host, sim, instance, pin, &s->spi_events, &s->i2c_events);
    }
    if (s->in_reset || s->unknown) {
        return;
    }
    for (u = 0; u < s->dev->v->uarts; u++) {
        Chan *c = &s->ch[u];

        if (pin == s->pins->channel[u][CH_CTS]) {
            LogicValue v = level(s, pin, LOGIC_X);

            if (v != c->cts) {
                /* CTSInt: a CTS transition (0 <-> 1; an X is no edge). */
                if (!bit(c, R_MODE2, M2_LOOP) && (v == LOGIC_0 || v == LOGIC_1) &&
                    (c->cts == LOGIC_0 || c->cts == LOGIC_1)) {
                    c->isr |= I_CTS;
                }
                c->cts = v;
                wake(c);
                update_outputs(s);
            }
        }
        if (pin == s->pins->channel[u][CH_RX]) {
            wake(c);
        }
    }
    for (i = 0; i < s->dev->gpio_count; i++) {
        if (pin == s->pins->gpio[i]) {
            s->gpio_dirty = 1;
            wake(&s->ch[i / 4]);
        }
    }
}

static void mx_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t id) {
    MxState *s = raw;
    unsigned type = (id >> 4) & 15u;
    unsigned ch = id & 15u;
    uint32_t gen = id >> 8;

    s->sim = sim;
    s->instance = instance;
    if (type == TM_READY) {
        if (gen == (s->ready_gen & 0xFFFFFFu)) {
            ready_now(s);
        }
        return;
    }
    if (type == TM_REFCLK) {
        if (gen != (s->refclk_gen & 0xFFFFFFu) || s->ref_mode != 2 || s->period == 0 ||
            !refclk_wanted(s) || sim_now(sim) - s->last_rise > 2 * s->period) {
            return;
        }
        s->refclk = s->refclk == LOGIC_1 ? LOGIC_0 : LOGIC_1;
        update_gpio(s);
        schedule(s, TM_REFCLK, 0, s->refclk_gen, refclk_half(s));
        return;
    }
    if (ch >= s->dev->v->uarts) {
        return;
    }
    if (type == TM_HALF) {
        Chan *c = &s->ch[ch];

        if (gen == (c->half_gen & 0xFFFFFFu)) {
            c->clkout = LOGIC_0;
            update_outputs(s);
        }
        return;
    }
    if (type == TM_BRG) {
        Chan *c = &s->ch[ch];
        SimTime now = sim_now(sim);

        if (gen != (c->brg_gen & 0xFFFFFFu) || !c->run) {
            return;
        }
        c->brg_pending = 0;
        c->clkout = LOGIC_1;
        chan_tick(c);
        if (!c->run || c->brg_pending) {
            return;
        }
        /* The XIN clock stopped: so does the PLL (it starts again on XIN). */
        if (s->period == 0 || now - s->last_rise > 2 * s->period) {
            return;
        }
        brg_schedule(c, tick_delay(c));
        if (clk_to_rts(c)) {
            c->half_gen++;
            schedule(s, TM_HALF, ch, c->half_gen, c->brg_delay / 2);
        }
    }
}

/* ---- Terminal ------------------------------------------------------------------------------------------------- */

static unsigned mx_channels(const void *raw) {
    return ((const MxState *)raw)->dev->v->uarts;
}

static SerialLine *mx_line(void *raw, unsigned channel) {
    return &((MxState *)raw)->ch[channel].line;
}

static int mx_async(const void *raw, unsigned channel) {
    const Chan *c = &((const MxState *)raw)->ch[channel];

    /* IrDA pulses and inverted RX are not the terminal's NRZ waveform. */
    return !bit(c, R_IRDA, IR_EN) && !bit(c, R_IRDA, IR_RXINV);
}

/* ---- Create / destroy ------------------------------------------------------------------------------------------- */

const SerialModel MAX310X_MODEL;

static void *mx_create(const Part *part, const void *data, const void *pins,
                       const SerialGrade *grade, const void *options) {
    MxState *s = calloc(1, sizeof *s);
    size_t i;
    unsigned u;

    (void)options;
    if (s == NULL) {
        return NULL;
    }
    s->base.model = &MAX310X_MODEL;
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
    s->host.spi.t_out = grade->t[T_DO];
    s->host.spi.t_float = grade->t[T_FT];
    s->spi_events.context = s;
    s->spi_events.begin = spi_begin;
    s->spi_events.next = spi_next;
    s->spi_events.bit = spi_bit;
    s->spi_events.received = spi_received;
    s->i2c_events.context = s;
    s->i2c_events.begin = i2c_begin;
    s->i2c_events.address = i2c_address;
    s->i2c_events.received = i2c_received;
    s->i2c_events.next = i2c_next;
    s->i2c_events.acked = i2c_acked;
    s->i2c_ch = -1;
    s->i2c_ptr = -1;
    s->xin = LOGIC_X;
    s->refclk = LOGIC_0;
    for (u = 0; u < s->dev->v->uarts; u++) {
        Chan *c = &s->ch[u];

        c->owner = s;
        c->index = u;
        serial_line_init(&c->line, s->pins->channel[u][CH_RX]);
        reset_chan(c);
        c->cts = LOGIC_X;
    }
    s->in_reset = 1;
    return s;
}

static void mx_destroy(void *raw) {
    MxState *s = raw;
    unsigned u;

    for (u = 0; u < s->dev->v->uarts; u++) {
        serial_line_free(&s->ch[u].line);
    }
    free(s->out);
    free(s);
}

static int mx_open_drain(const void *data, const void *pins, const void *options, size_t pin) {
    const MxDevice *d = data;
    const MxPins *p = pins;

    (void)options;
    if (pin == SERIAL_NO_PIN) {
        return 0;
    }
    return pin == p->irq || serial_host_open_drain(&d->host, &p->host, pin);
}

/* ---- Loader ------------------------------------------------------------------------------------------------------ */

static void *mx_load_device(KindContext *ctx, const cJSON *json, unsigned *channels) {
    static const char *const CLOCK_KEYS[] = {"in", "out", NULL};
    MxDevice *d = kind_alloc(ctx, sizeof *d);
    const cJSON *bus;
    const cJSON *clock;
    const cJSON *list;
    const cJSON *item;
    const char *variant;
    const char *const *names;
    size_t n;
    unsigned i;
    unsigned k;

    if (d == NULL || kind_string(ctx, json, "variant", &variant) != 0) {
        return NULL;
    }
    for (i = 0; i < sizeof VARIANTS / sizeof VARIANTS[0]; i++) {
        if (strcmp(VARIANTS[i].name, variant) == 0) {
            d->v = &VARIANTS[i];
        }
    }
    if (d->v == NULL) {
        kind_error(ctx, "unknown \"variant\" \"%s\"", variant);
        return NULL;
    }
    bus = kind_member(ctx, json, "bus", 1);
    if (bus == NULL) {
        return NULL;
    }
    kind_push(ctx, "bus");
    if (serial_host_load(ctx, bus, BUS_EXTRA, &d->host) != 0) {
        return NULL;
    }
    if (d->host.style != SERIAL_HOST_I2C_SPI || d->host.mode != 0) {
        kind_error(ctx, "the max310x host port is \"i2c-spi\" with SPI mode 0");
        return NULL;
    }
    list = cJSON_GetObjectItemCaseSensitive(bus, "uartOffsets");
    if (list != NULL) {
        if (!cJSON_IsArray(list) || (unsigned)cJSON_GetArraySize(list) != d->v->uarts) {
            kind_error(ctx, "\"uartOffsets\" needs one number per UART");
            return NULL;
        }
        i = 0;
        cJSON_ArrayForEach(item, list) {
            if (!cJSON_IsNumber(item) || item->valuedouble != (double)(int)item->valuedouble ||
                item->valuedouble < -127 || item->valuedouble > 127) {
                kind_error(ctx, "\"uartOffsets\" entries are whole numbers from -127 to 127");
                return NULL;
            }
            d->offsets[i++] = (int)item->valuedouble;
        }
    } else if (d->v->uarts != 1) {
        kind_error(ctx, "missing \"uartOffsets\"");
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
    if (kind_string(ctx, json, "reset", &d->reset) != 0 ||
        kind_string(ctx, json, "interrupt", &d->irq) != 0) {
        return NULL;
    }
    list = kind_member(ctx, json, "channels", 1);
    if (list == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(list) || (unsigned)cJSON_GetArraySize(list) != d->v->uarts) {
        kind_error(ctx, "\"channels\" must be an array of %u channel(s) for %s", d->v->uarts,
                   d->v->name);
        return NULL;
    }
    i = 0;
    cJSON_ArrayForEach(item, list) {
        kind_push(ctx, "channels[%u]", i);
        if (kind_check_keys(ctx, item, CHANNEL_KEYS, NULL) != 0) {
            return NULL;
        }
        for (k = 0; k < CH_PIN_COUNT; k++) {
            if (kind_string(ctx, item, CHANNEL_KEYS[k], &d->channel[i][k]) != 0) {
                return NULL;
            }
        }
        kind_pop(ctx);
        i++;
    }
    if (kind_names(ctx, json, "gpio", &names, &n) != 0) {
        return NULL;
    }
    if (n != 4u * d->v->uarts) {
        kind_error(ctx, "\"gpio\" needs %u pin names (4 per UART)", 4u * d->v->uarts);
        return NULL;
    }
    for (i = 0; i < n; i++) {
        d->gpio[i] = names[i];
    }
    d->gpio_count = (unsigned)n;
    *channels = d->v->uarts;
    return d;
}

static void *mx_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t pin_count) {
    const MxDevice *d = raw;
    MxPins *p = kind_alloc(ctx, sizeof *p);
    char key[32];
    unsigned i;
    unsigned k;

    if (p == NULL || serial_host_bind(ctx, &d->host, pins, pin_count, &p->host) != 0) {
        return NULL;
    }
    if (serial_bind_pin(ctx, pins, pin_count, "reset", d->reset, &p->reset) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "interrupt", d->irq, &p->irq) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "clock.in", d->clock_in, &p->clock_in) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "clock.out", d->clock_out, &p->clock_out) != 0) {
        return NULL;
    }
    for (i = 0; i < MX_MAX_UARTS; i++) {
        for (k = 0; k < CH_PIN_COUNT; k++) {
            p->channel[i][k] = SERIAL_NO_PIN;
        }
    }
    for (i = 0; i < d->v->uarts; i++) {
        for (k = 0; k < CH_PIN_COUNT; k++) {
            snprintf(key, sizeof key, "channels[%u].%s", i, CHANNEL_KEYS[k]);
            if (serial_bind_pin(ctx, pins, pin_count, key, d->channel[i][k], &p->channel[i][k]) != 0) {
                return NULL;
            }
        }
    }
    for (i = 0; i < MX_GPIO; i++) {
        p->gpio[i] = SERIAL_NO_PIN;
    }
    for (i = 0; i < d->gpio_count; i++) {
        snprintf(key, sizeof key, "gpio[%u]", i);
        if (serial_bind_pin(ctx, pins, pin_count, key, d->gpio[i], &p->gpio[i]) != 0) {
            return NULL;
        }
    }
    return p;
}

const SerialModel MAX310X_MODEL = {
    "max310x", KEYS, TIMING, NULL,
    mx_load_device, mx_bind, NULL, mx_open_drain,
    mx_create, mx_destroy, mx_start, mx_pin_changed, mx_timer,
    mx_channels, mx_line, mx_async
};
