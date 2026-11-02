#ifndef SERIAL_LINE_H
#define SERIAL_LINE_H

/*
 * The serial line engine every serial model uses (plan-12-A section 3.3,
 * plan-12-C section 3.2). Pin-agnostic: the model feeds it clock edges and
 * ticks and reads back levels.
 *
 *   SerialBaud    counts clock edges; every `divisor` rising edges is one
 *                 tick (16x, 8x, 4x ... of the bit rate); BAUDOUT level.
 *   SerialTx      one tick at a time: idle mark, start, 5-9 data bits LSB
 *                 first, parity, 1 / 1.5 / 2 stops. Asks the model for the
 *                 next character when idle.
 *   SerialRx      one tick at a time with the line level: start detection on
 *                 1 -> 0, start validated at the sample tick, data / parity /
 *                 stop sampled mid-bit, framing / parity / break detection,
 *                 the PC16550D resync after a framing error.
 *   SerialLine    one channel: a Tx, an Rx, the sent-frame log (terminal),
 *                 and the terminal's send queue, which drives the channel's RX
 *                 pin as a real waveform (plan-12-A section 3.7).
 *
 * Levels are LogicValue (0, 1, X). Data is (data, known) pairs: bit n of
 * known is 1 when bit n of data is defined.
 */

#include <stddef.h>
#include <stdint.h>

#include "sim.h"

typedef enum {
    SERIAL_PARITY_NONE,
    SERIAL_PARITY_ODD,
    SERIAL_PARITY_EVEN,
    SERIAL_PARITY_MARK,    /* stick 1 */
    SERIAL_PARITY_SPACE    /* stick 0 */
} SerialParity;

typedef struct {
    unsigned data_bits;      /* 5..9 */
    SerialParity parity;
    unsigned stop_halves;    /* 2 = 1 stop, 3 = 1.5, 4 = 2 */
    unsigned factor;         /* ticks per bit: 16 (default), 8, 4, 1, 32, 64 */
    unsigned sample;         /* ticks after the start is seen to the start-bit check; 0 = factor / 2 */
    int majority;            /* data bits: majority of the samples at sample - 1, sample, sample + 1 */
    int check_both_stops;    /* with 2 stop bits, a 0 in either is a framing error (82C50A) */
    int format_x;            /* 1: the format itself is unknown; every bit sent is X */
    unsigned mark_ticks;     /* after a break: ticks of mark before a new start (0 = 1) */
} SerialFormat;

/* The default 8N1 at 16x. */
SerialFormat serial_format_default(void);

/* Bits in a whole character (start + data + parity + stops, a half stop
   rounded up), for timeouts and break detection. */
unsigned serial_format_char_bits(const SerialFormat *format);

/* ---- Baud generator ------------------------------------------------------ */

typedef struct {
    uint32_t divisor;        /* 1..65535; 0 stops the generator */
    int known;               /* 0: the divisor is X (BAUDOUT X, no ticks) */
    uint32_t count;          /* rising edges since the last tick */
    LogicValue baudout;      /* the BAUDOUT level after the last edge */
    uint32_t frac;           /* fractional divisor: frac / frac_den extra per tick */
    uint32_t frac_den;
    uint32_t frac_acc;
    int extra;               /* this period is one edge longer */
    int low2;                /* BAUDOUT low for 2 edges (1 when N = 2), high for the rest */
} SerialBaud;

/* Loads a new divisor; the counter restarts (PC16550D: "writing either
   divisor latch immediately loads the baud counter"). */
void serial_baud_load(SerialBaud *baud, uint32_t divisor, int known);

/*
 * One clock edge (rising = 1 for 0 -> 1, 0 for 1 -> 0). Returns 1 when this
 * edge makes a tick. BAUDOUT: with divisor 1 it follows the clock; else it is
 * high for the first half of each divisor period (ceil(N / 2) edges) and low
 * for the rest; X when the divisor is 0 or X.
 */
int serial_baud_edge(SerialBaud *baud, int rising, LogicValue clock_level);

/* ---- Transmitter ----------------------------------------------------------- */

typedef enum {
    SERIAL_TX_IDLE,
    SERIAL_TX_START,
    SERIAL_TX_DATA,
    SERIAL_TX_PARITY,
    SERIAL_TX_STOP
} SerialTxPhase;

typedef struct {
    SerialTxPhase phase;
    unsigned bit;            /* data bit index, or stop half count */
    unsigned count;          /* ticks into the current bit */
    unsigned stop_ticks;     /* ticks of stop sent */
    uint16_t data;
    uint16_t known;
    SerialFormat format;     /* latched at the start bit */
    LogicValue level;        /* the output after the last tick */
} SerialTx;

/* Asked when the transmitter is idle at a tick: 1 and the character to send,
   or 0 for none. */
typedef int (*SerialTxFetch)(void *context, uint16_t *data, uint16_t *known);

typedef struct {
    void *context;
    SerialTxFetch fetch;
    /* The frame's last stop bit starts (the PC16550D's transmitter-interrupt
       delay ends here). May be NULL. */
    void (*last_stop)(void *context);
    /* A frame finished: its data bits. May be NULL. */
    void (*done)(void *context, uint16_t data, uint16_t known);
} SerialTxEvents;

void serial_tx_reset(SerialTx *tx);
/* One tick. `format` is the current format (latched at each start bit).
   Returns the line level for the next tick period. */
LogicValue serial_tx_tick(SerialTx *tx, const SerialFormat *format, const SerialTxEvents *events);
int serial_tx_busy(const SerialTx *tx);

/* ---- Receiver -------------------------------------------------------------- */

enum {
    SERIAL_RX_PE = 1,        /* parity error */
    SERIAL_RX_FE = 2,        /* framing error (stop bit 0) */
    SERIAL_RX_BI = 4         /* break: start, data, parity and stop all 0 */
};

typedef struct {
    uint16_t data;
    uint16_t known;
    uint8_t flags;           /* SERIAL_RX_* */
    uint8_t flags_known;     /* SERIAL_RX_* bits whose value is defined */
} SerialRxChar;

typedef enum {
    SERIAL_RX_IDLE,
    SERIAL_RX_START,
    SERIAL_RX_DATA,
    SERIAL_RX_PARITY,
    SERIAL_RX_STOP,
    SERIAL_RX_STOP2,
    SERIAL_RX_WAIT_MARK      /* after a break: wait for the line to go back to 1 */
} SerialRxPhase;

typedef struct {
    SerialRxPhase phase;
    unsigned bit;
    unsigned count;
    LogicValue last;         /* the level at the previous tick */
    LogicValue history[3];   /* the last 3 levels, newest first (majority vote) */
    uint16_t data;
    uint16_t known;
    LogicValue parity;
    LogicValue stop1;        /* the first stop bit, while the second is checked */
    int all_zero;
    int zero_known;
    SerialFormat format;     /* latched at the start bit */
    uint64_t ticks;          /* ticks seen; the timeout's clock */
    unsigned marks;          /* ticks of mark seen while waiting after a break */
} SerialRx;

typedef struct {
    void *context;
    /* A character is complete (at the first stop bit's sample). */
    void (*received)(void *context, const SerialRxChar *c);
    /* A start bit was validated (receiver activity, for timeouts). May be NULL. */
    void (*started)(void *context);
} SerialRxEvents;

void serial_rx_reset(SerialRx *rx);
/* One tick with the line level now. */
void serial_rx_tick(SerialRx *rx, LogicValue level, const SerialFormat *format,
                    const SerialRxEvents *events);
int serial_rx_busy(const SerialRx *rx);

/* ---- A channel: log and terminal sender ------------------------------------ */

typedef struct {
    uint8_t *bytes;
    size_t head;
    size_t count;
    size_t capacity;
    int active;              /* sending a frame */
    uint16_t data;
    unsigned bit;            /* 0 start, 1..n data, then parity, then stops */
    unsigned count_ticks;    /* ticks left in the current bit */
    SerialFormat format;
    LogicValue level;
    int driven;              /* the RX pin is being driven */
} SerialSender;

typedef struct {
    SerialTx tx;
    SerialRx rx;
    SerialSender sender;
    uint16_t *log;
    size_t log_count;
    size_t log_capacity;
    size_t rx_pin;           /* the pin the sender drives; SIZE_MAX = none */
} SerialLine;

void serial_line_init(SerialLine *line, size_t rx_pin);
void serial_line_free(SerialLine *line);

/* Appends a sent frame's data bits to the log (unknown bits as 0). */
void serial_line_log(SerialLine *line, uint16_t data);
size_t serial_line_sent(const SerialLine *line, size_t start, uint16_t *frames, size_t cap);

/* Queues bytes for the sender. 0 on success, -1 when out of memory. */
int serial_line_queue(SerialLine *line, const uint8_t *bytes, size_t count);

/*
 * The receiver's input at a receive tick: the sender's level while it sends,
 * else `pin_level`. Advances the sender one tick first: when idle with bytes
 * queued and the receiver idle, it starts a frame in `format`; it drives the
 * RX pin strong while sending and releases it (Z) after the last stop bit.
 */
LogicValue serial_line_rx_input(SerialLine *line, Sim *sim, SimInstance *instance,
                                 LogicValue pin_level, const SerialFormat *format);

/* Drops the sender's queue and releases the pin (reset). */
void serial_line_sender_reset(SerialLine *line, Sim *sim, SimInstance *instance);

#endif
