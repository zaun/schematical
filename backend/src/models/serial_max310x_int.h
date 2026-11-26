/* models/serial_max310x_int.h: what the pieces of models/serial_max310x.c share (plan-68-I/J split it). */
#ifndef MODELS_SERIAL_MAX310X_INT_H
#define MODELS_SERIAL_MAX310X_INT_H

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

typedef enum { CH_TX, CH_RX, CH_RTS, CH_CTS, CH_PIN_COUNT } ChannelPin;

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

extern const char *const m31x_TIMING[4];
extern const char *const m31x_KEYS[8];
extern const char *const m31x_BUS_EXTRA[2];
extern const char *const m31x_CHANNEL_KEYS[5];
extern const Variant m31x_VARIANTS[4];
LogicValue m31x_not3(LogicValue a);
LogicValue m31x_bool3(int b);
uint8_t m31x_rv(const Chan *c, unsigned reg);
int m31x_bit(const Chan *c, unsigned reg, uint8_t mask);
LogicValue m31x_level(const MxState *s, size_t pin, LogicValue z_reads);
void m31x_schedule(MxState *s, unsigned type, unsigned ch, uint32_t gen, SimTime delay);
Chan *m31x_clock_chan(MxState *s);
int m31x_clk_to_rts(const Chan *c);
uint8_t m31x_writable(const Variant *v, unsigned addr);
unsigned m31x_trigger_rx(const Chan *c);
unsigned m31x_trigger_tx(const Chan *c);
uint8_t m31x_sts_now(const Chan *c);
uint8_t m31x_lsr_now(const Chan *c);
uint8_t m31x_isr_now(const Chan *c);
int m31x_pending(const Chan *c);
uint8_t m31x_global_irq(const MxState *s);
void m31x_update_format(Chan *c);
uint32_t m31x_divisor(const Chan *c);
uint32_t m31x_next_target(Chan *c);
int m31x_chan_clock_on(const Chan *c);
SimTime m31x_tick_delay(Chan *c);
void m31x_brg_schedule(Chan *c, SimTime delay);
void m31x_brg_restart(Chan *c);
LogicValue m31x_cts_internal(const Chan *c);
LogicValue m31x_tx_signal(const Chan *c);
void m31x_update_gpio(MxState *s);
void m31x_update_outputs(MxState *s);
void m31x_reset_chan(Chan *c);
void m31x_ready_now(MxState *s);
void m31x_apply_reset_pin(MxState *s);
unsigned m31x_mx_channels(const void *raw);
SerialLine *m31x_mx_line(void *raw, unsigned channel);
int m31x_mx_async(const void *raw, unsigned channel);
void *m31x_mx_create(const Part *part, const void *data, const void *pins,
                       const SerialGrade *grade, const void *options);
void m31x_mx_destroy(void *raw);
int m31x_mx_open_drain(const void *data, const void *pins, const void *options, size_t pin);
void m31x_rx_clear(Chan *c);
void m31x_tx_clear(Chan *c);
void m31x_flow_update(Chan *c);
void m31x_chan_tick(Chan *c);
void m31x_update_reference(MxState *s);
int m31x_refclk_wanted(const MxState *s);
SimTime m31x_refclk_half(MxState *s);
void m31x_xin_edge(MxState *s, LogicValue v);
void m31x_spi_begin(void *context);
SerialSpiByte m31x_spi_next(void *context, unsigned index);
void m31x_spi_bit(void *context, unsigned index, unsigned bits, uint8_t data, uint8_t known,
                    SerialSpiByte *out);
void m31x_spi_received(void *context, unsigned index, uint8_t data, uint8_t known);
LogicValue m31x_i2c_address(void *context, uint8_t byte, uint8_t known);
void m31x_i2c_begin(void *context, int repeated);
LogicValue m31x_i2c_received(void *context, unsigned index, uint8_t data, uint8_t known);
void m31x_i2c_next(void *context, unsigned index, uint8_t *data, uint8_t *known);
void m31x_i2c_acked(void *context, unsigned index, LogicValue ack);

#endif
