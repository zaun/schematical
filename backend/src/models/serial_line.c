/*
 * The serial line engine (serial_line.h; plan-12-C section 3.2).
 */

#include "serial_line.h"

#include <stdlib.h>
#include <string.h>

SerialFormat serial_format_default(void) {
    SerialFormat format;

    memset(&format, 0, sizeof format);
    format.data_bits = 8;
    format.parity = SERIAL_PARITY_NONE;
    format.stop_halves = 2;
    format.factor = 16;
    return format;
}

unsigned serial_format_char_bits(const SerialFormat *format) {
    return 1 + format->data_bits + (format->parity != SERIAL_PARITY_NONE) +
           (format->stop_halves + 1) / 2;
}

static unsigned sample_tick(const SerialFormat *format) {
    return format->sample != 0 ? format->sample : (format->factor + 1) / 2;
}

/* ---- Baud generator -------------------------------------------------------- */

void serial_baud_load(SerialBaud *baud, uint32_t divisor, int known) {
    baud->divisor = divisor;
    baud->known = known;
    baud->count = 0;
    baud->frac = 0;
    baud->frac_acc = 0;
    baud->extra = 0;
    baud->baudout = known && divisor != 0 ? (baud->low2 && divisor >= 2 ? LOGIC_0 : LOGIC_1)
                                          : LOGIC_X;
}

int serial_baud_edge(SerialBaud *baud, int rising, LogicValue clock_level) {
    if (!baud->known || baud->divisor == 0) {
        baud->baudout = LOGIC_X;
        return 0;
    }
    if (baud->divisor == 1 && baud->frac == 0) {
        baud->baudout = clock_level;
        return rising;
    }
    if (!rising) {
        return 0;
    }
    baud->count++;
    if (baud->count >= baud->divisor + (baud->extra ? 1u : 0u)) {
        baud->count = 0;
        /* Fractional divisor: spread frac / frac_den extra edges over the ticks. */
        baud->extra = 0;
        if (baud->frac != 0 && baud->frac_den != 0) {
            baud->frac_acc += baud->frac;
            if (baud->frac_acc >= baud->frac_den) {
                baud->frac_acc -= baud->frac_den;
                baud->extra = 1;
            }
        }
    }
    if (baud->low2 && baud->divisor >= 2) {
        /* TI/Intersil: low for 2 clocks (1 when N = 2), high for the rest. */
        baud->baudout = baud->count < (baud->divisor == 2 ? 1u : 2u) ? LOGIC_0 : LOGIC_1;
    } else {
        baud->baudout = baud->count < (baud->divisor + 1) / 2 ? LOGIC_1 : LOGIC_0;
    }
    return baud->count == 0;
}

/* ---- Parity ---------------------------------------------------------------- */

/* The parity bit for data under `parity`: 0, 1 or X (unknown data). */
static LogicValue parity_bit(SerialParity parity, uint16_t data, uint16_t known, unsigned bits) {
    uint16_t mask = (uint16_t)((1u << bits) - 1);
    unsigned ones = 0;
    unsigned i;

    switch (parity) {
        case SERIAL_PARITY_MARK: return LOGIC_1;
        case SERIAL_PARITY_SPACE: return LOGIC_0;
        case SERIAL_PARITY_NONE: return LOGIC_1;
        case SERIAL_PARITY_ODD:
        case SERIAL_PARITY_EVEN:
            break;
    }
    if ((known & mask) != mask) {
        return LOGIC_X;
    }
    for (i = 0; i < bits; i++) {
        ones += (data >> i) & 1u;
    }
    /* Odd: the parity bit makes the count of ones odd. */
    if (parity == SERIAL_PARITY_ODD) {
        return ones % 2 == 0 ? LOGIC_1 : LOGIC_0;
    }
    return ones % 2 == 0 ? LOGIC_0 : LOGIC_1;
}

/* ---- Transmitter ----------------------------------------------------------- */

void serial_tx_reset(SerialTx *tx) {
    memset(tx, 0, sizeof *tx);
    tx->phase = SERIAL_TX_IDLE;
    tx->level = LOGIC_1;
}

int serial_tx_busy(const SerialTx *tx) {
    return tx->phase != SERIAL_TX_IDLE;
}

static LogicValue tx_bit_level(const SerialTx *tx) {
    if (tx->format.format_x) {
        return LOGIC_X;
    }
    switch (tx->phase) {
        case SERIAL_TX_IDLE: return LOGIC_1;
        case SERIAL_TX_START: return LOGIC_0;
        case SERIAL_TX_DATA:
            if (!((tx->known >> tx->bit) & 1u)) {
                return LOGIC_X;
            }
            return (tx->data >> tx->bit) & 1u ? LOGIC_1 : LOGIC_0;
        case SERIAL_TX_PARITY:
            return parity_bit(tx->format.parity, tx->data, tx->known, tx->format.data_bits);
        case SERIAL_TX_STOP: return LOGIC_1;
    }
    return LOGIC_X;
}

static int tx_begin(SerialTx *tx, const SerialFormat *format, const SerialTxEvents *events) {
    uint16_t data = 0;
    uint16_t known = 0;

    if (events->fetch == NULL || !events->fetch(events->context, &data, &known)) {
        return 0;
    }
    tx->format = *format;
    tx->data = data;
    tx->known = known;
    tx->phase = SERIAL_TX_START;
    tx->bit = 0;
    tx->count = 0;
    tx->stop_ticks = 0;
    return 1;
}

static unsigned stop_total(const SerialFormat *format) {
    return format->stop_halves * format->factor / 2;
}

static void tx_enter_stop(SerialTx *tx, const SerialTxEvents *events) {
    tx->phase = SERIAL_TX_STOP;
    tx->stop_ticks = 0;
    /* One stop bit: the last stop bit starts now. */
    if (stop_total(&tx->format) <= tx->format.factor && events->last_stop != NULL) {
        events->last_stop(events->context);
    }
}

LogicValue serial_tx_tick(SerialTx *tx, const SerialFormat *format, const SerialTxEvents *events) {
    switch (tx->phase) {
        case SERIAL_TX_IDLE:
            tx_begin(tx, format, events);
            break;
        case SERIAL_TX_START:
            if (++tx->count >= tx->format.factor) {
                tx->count = 0;
                tx->phase = SERIAL_TX_DATA;
                tx->bit = 0;
            }
            break;
        case SERIAL_TX_DATA:
            if (++tx->count >= tx->format.factor) {
                tx->count = 0;
                if (++tx->bit >= tx->format.data_bits) {
                    if (tx->format.parity != SERIAL_PARITY_NONE) {
                        tx->phase = SERIAL_TX_PARITY;
                    } else {
                        tx_enter_stop(tx, events);
                    }
                }
            }
            break;
        case SERIAL_TX_PARITY:
            if (++tx->count >= tx->format.factor) {
                tx->count = 0;
                tx_enter_stop(tx, events);
            }
            break;
        case SERIAL_TX_STOP: {
            unsigned total = stop_total(&tx->format);

            tx->stop_ticks++;
            if (total > tx->format.factor && tx->stop_ticks == total - tx->format.factor &&
                events->last_stop != NULL) {
                events->last_stop(events->context);
            }
            if (tx->stop_ticks >= total) {
                uint16_t data = tx->data;
                uint16_t known = tx->known;
                unsigned bits = tx->format.data_bits;
                uint16_t mask = (uint16_t)((1u << bits) - 1);

                tx->phase = SERIAL_TX_IDLE;
                if (events->done != NULL) {
                    events->done(events->context, data & mask, known & mask);
                }
                /* Back to back: the next start bit begins at this tick. */
                tx_begin(tx, format, events);
            }
            break;
        }
    }
    tx->level = tx_bit_level(tx);
    return tx->level;
}

/* ---- Receiver -------------------------------------------------------------- */

void serial_rx_reset(SerialRx *rx) {
    uint64_t ticks = rx->ticks;

    memset(rx, 0, sizeof *rx);
    rx->phase = SERIAL_RX_IDLE;
    rx->last = LOGIC_X;
    rx->history[0] = rx->history[1] = rx->history[2] = LOGIC_X;
    rx->ticks = ticks;
}

int serial_rx_busy(const SerialRx *rx) {
    return rx->phase != SERIAL_RX_IDLE && rx->phase != SERIAL_RX_WAIT_MARK;
}

static LogicValue majority(const LogicValue *h) {
    unsigned ones = 0;
    unsigned zeros = 0;
    unsigned i;

    for (i = 0; i < 3; i++) {
        ones += h[i] == LOGIC_1;
        zeros += h[i] == LOGIC_0;
    }
    if (ones >= 2) {
        return LOGIC_1;
    }
    if (zeros >= 2) {
        return LOGIC_0;
    }
    return LOGIC_X;
}

/* 1 when this tick is a data/parity/stop sample point; the sampled level in *out. */
static int rx_sample(SerialRx *rx, LogicValue level, LogicValue *out) {
    unsigned at = rx->format.factor + (rx->format.majority ? 1u : 0u);

    if (++rx->count < at) {
        return 0;
    }
    rx->count = rx->format.majority ? 1u : 0u;
    *out = rx->format.majority ? majority(rx->history) : level;
    return 1;
}

static void rx_begin_data(SerialRx *rx, const SerialRxEvents *events) {
    rx->phase = SERIAL_RX_DATA;
    rx->bit = 0;
    rx->count = 0;
    rx->data = 0;
    rx->known = 0;
    rx->parity = LOGIC_1;
    rx->all_zero = 1;
    rx->zero_known = 1;
    if (events->started != NULL) {
        events->started(events->context);
    }
}

static void note_zero(SerialRx *rx, LogicValue bit) {
    if (bit == LOGIC_1) {
        rx->all_zero = 0;
    } else if (bit == LOGIC_X) {
        rx->zero_known = 0;
    }
}

static void rx_deliver(SerialRx *rx, LogicValue stop, LogicValue stop2,
                       const SerialRxEvents *events, int *is_break, int *framing) {
    SerialRxChar c;
    unsigned bits = rx->format.data_bits;
    uint16_t mask = (uint16_t)((1u << bits) - 1);
    LogicValue expected;

    memset(&c, 0, sizeof c);
    c.data = rx->data & mask;
    c.known = rx->known & mask;
    c.flags_known = SERIAL_RX_PE | SERIAL_RX_FE | SERIAL_RX_BI;

    /* Framing: the (first, or both) stop bit(s) sampled 0. */
    if (stop == LOGIC_0 || stop2 == LOGIC_0) {
        c.flags |= SERIAL_RX_FE;
    } else if (stop == LOGIC_X || stop2 == LOGIC_X) {
        c.flags_known &= (uint8_t)~SERIAL_RX_FE;
    }
    /* Parity. */
    if (rx->format.parity != SERIAL_PARITY_NONE) {
        expected = parity_bit(rx->format.parity, rx->data, rx->known, bits);
        if (expected == LOGIC_X || rx->parity == LOGIC_X) {
            c.flags_known &= (uint8_t)~SERIAL_RX_PE;
        } else if (expected != rx->parity) {
            c.flags |= SERIAL_RX_PE;
        }
    }
    /* Break: everything from the start bit to the stop bit was 0. */
    note_zero(rx, stop);
    if (rx->all_zero && rx->zero_known) {
        c.flags |= SERIAL_RX_BI;
    } else if (rx->all_zero) {
        c.flags_known &= (uint8_t)~SERIAL_RX_BI;
    }
    *is_break = rx->all_zero && rx->zero_known;
    *framing = stop == LOGIC_0;
    if (events->received != NULL) {
        events->received(events->context, &c);
    }
}

void serial_rx_tick(SerialRx *rx, LogicValue level, const SerialFormat *format,
                    const SerialRxEvents *events) {
    LogicValue bit;
    int is_break = 0;
    int framing = 0;

    rx->ticks++;
    rx->history[2] = rx->history[1];
    rx->history[1] = rx->history[0];
    rx->history[0] = level;

    switch (rx->phase) {
        case SERIAL_RX_IDLE:
            if (rx->last == LOGIC_1 && level == LOGIC_0) {
                rx->phase = SERIAL_RX_START;
                rx->count = 0;
                rx->format = *format;
            }
            break;
        case SERIAL_RX_START:
            if (++rx->count >= sample_tick(&rx->format)) {
                if (level == LOGIC_0) {
                    rx_begin_data(rx, events);
                } else {
                    rx->phase = SERIAL_RX_IDLE;   /* a false start */
                }
            }
            break;
        case SERIAL_RX_DATA:
            if (rx_sample(rx, level, &bit)) {
                if (bit == LOGIC_1) {
                    rx->data |= (uint16_t)(1u << rx->bit);
                }
                if (bit != LOGIC_X) {
                    rx->known |= (uint16_t)(1u << rx->bit);
                }
                note_zero(rx, bit);
                if (++rx->bit >= rx->format.data_bits) {
                    rx->phase = rx->format.parity != SERIAL_PARITY_NONE ? SERIAL_RX_PARITY
                                                                         : SERIAL_RX_STOP;
                }
            }
            break;
        case SERIAL_RX_PARITY:
            if (rx_sample(rx, level, &bit)) {
                rx->parity = bit;
                note_zero(rx, bit);
                rx->phase = SERIAL_RX_STOP;
            }
            break;
        case SERIAL_RX_STOP:
            if (rx_sample(rx, level, &bit)) {
                if (rx->format.check_both_stops && rx->format.stop_halves == 4) {
                    rx->stop1 = bit;   /* held until the second stop */
                    rx->phase = SERIAL_RX_STOP2;
                    break;
                }
                rx_deliver(rx, bit, LOGIC_1, events, &is_break, &framing);
                if (is_break) {
                    rx->phase = SERIAL_RX_WAIT_MARK;
                } else if (framing) {
                    /* PC16550D p16: the framing error is taken as the next
                       start bit, sampled twice, then the data comes in. */
                    rx_begin_data(rx, events);
                } else {
                    rx->phase = SERIAL_RX_IDLE;
                }
            }
            break;
        case SERIAL_RX_STOP2:
            if (rx_sample(rx, level, &bit)) {
                rx_deliver(rx, rx->stop1, bit, events, &is_break, &framing);
                if (is_break) {
                    rx->phase = SERIAL_RX_WAIT_MARK;
                } else if (bit == LOGIC_0) {
                    rx_begin_data(rx, events);
                } else {
                    rx->phase = SERIAL_RX_IDLE;
                }
            }
            break;
        case SERIAL_RX_WAIT_MARK:
            if (level == LOGIC_1) {
                rx->phase = SERIAL_RX_IDLE;
            }
            break;
    }
    rx->last = level;
}

/* ---- Channel: log and sender -------------------------------------------------- */

void serial_line_init(SerialLine *line, size_t rx_pin) {
    memset(line, 0, sizeof *line);
    serial_tx_reset(&line->tx);
    serial_rx_reset(&line->rx);
    line->sender.level = LOGIC_Z;
    line->rx_pin = rx_pin;
}

void serial_line_free(SerialLine *line) {
    free(line->log);
    free(line->sender.bytes);
    line->log = NULL;
    line->sender.bytes = NULL;
}

void serial_line_log(SerialLine *line, uint16_t data) {
    if (line->log_count == line->log_capacity) {
        size_t capacity = line->log_capacity == 0 ? 64 : line->log_capacity * 2;
        uint16_t *bigger = realloc(line->log, capacity * sizeof *bigger);

        if (bigger == NULL) {
            return;
        }
        line->log = bigger;
        line->log_capacity = capacity;
    }
    line->log[line->log_count++] = data;
}

size_t serial_line_sent(const SerialLine *line, size_t start, uint16_t *frames, size_t cap) {
    size_t n = 0;

    while (start + n < line->log_count && n < cap) {
        frames[n] = line->log[start + n];
        n++;
    }
    return n;
}

int serial_line_queue(SerialLine *line, const uint8_t *bytes, size_t count) {
    SerialSender *s = &line->sender;
    size_t i;

    if (s->count + count > s->capacity) {
        size_t capacity = s->capacity == 0 ? 64 : s->capacity;
        uint8_t *bigger;

        while (capacity < s->count + count) {
            capacity *= 2;
        }
        bigger = malloc(capacity);
        if (bigger == NULL) {
            return -1;
        }
        for (i = 0; i < s->count; i++) {
            bigger[i] = s->bytes[(s->head + i) % s->capacity];
        }
        free(s->bytes);
        s->bytes = bigger;
        s->capacity = capacity;
        s->head = 0;
    }
    for (i = 0; i < count; i++) {
        s->bytes[(s->head + s->count) % s->capacity] = bytes[i];
        s->count++;
    }
    return 0;
}

static void sender_drive(SerialLine *line, Sim *sim, SimInstance *instance, LogicValue level) {
    if (line->sender.level != level) {
        line->sender.level = level;
        if (line->rx_pin != SIZE_MAX) {
            sim_schedule_drive(sim, instance, line->rx_pin, level, DRIVE_STRONG, 0);
        }
    }
}

void serial_line_sender_reset(SerialLine *line, Sim *sim, SimInstance *instance) {
    line->sender.count = 0;
    line->sender.head = 0;
    if (line->sender.active) {
        line->sender.active = 0;
        sender_drive(line, sim, instance, LOGIC_Z);
    }
}

/* Sender bit numbering: 0 = idle mark before the first frame, 1 = start,
   2..1+n = data, then the parity bit, then the stop period (one "bit" of
   stop_halves * factor / 2 ticks). */
static unsigned sender_stop_bit(const SerialSender *s) {
    return 2 + s->format.data_bits + (s->format.parity != SERIAL_PARITY_NONE);
}

static LogicValue sender_bit(const SerialSender *s) {
    unsigned n = s->format.data_bits;

    if (s->bit == 0) {
        return LOGIC_1;
    }
    if (s->bit == 1) {
        return LOGIC_0;
    }
    if (s->bit < 2 + n) {
        return (s->data >> (s->bit - 2)) & 1u ? LOGIC_1 : LOGIC_0;
    }
    if (s->bit < sender_stop_bit(s)) {
        return parity_bit(s->format.parity, s->data, 0xFFFF, n);
    }
    return LOGIC_1;
}

static unsigned sender_bit_ticks(const SerialSender *s) {
    return s->bit == sender_stop_bit(s) ? s->format.stop_halves * s->format.factor / 2
                                        : s->format.factor;
}

static void sender_next_byte(SerialSender *s, const SerialFormat *format) {
    s->format = *format;
    s->format.format_x = 0;
    s->data = (uint16_t)(s->bytes[s->head] & ((1u << format->data_bits) - 1));
    s->head = (s->head + 1) % s->capacity;
    s->count--;
    s->bit = 1;
    s->count_ticks = sender_bit_ticks(s);
}

LogicValue serial_line_rx_input(SerialLine *line, Sim *sim, SimInstance *instance,
                                 LogicValue pin_level, const SerialFormat *format) {
    SerialSender *s = &line->sender;

    if (!s->active) {
        if (s->count == 0 || serial_rx_busy(&line->rx)) {
            return pin_level;
        }
        /* One idle bit of mark first, so a receiver on an undriven line sees
           the 1 -> 0 of the start bit. */
        s->active = 1;
        s->format = *format;
        s->format.format_x = 0;
        s->bit = 0;
        s->count_ticks = s->format.factor;
    }
    /* count_ticks: ticks left in the current bit, this one included. */
    if (s->count_ticks == 0) {
        if (s->bit == 0) {
            sender_next_byte(s, format);
        } else if (s->bit < sender_stop_bit(s)) {
            s->bit++;
            s->count_ticks = sender_bit_ticks(s);
        } else if (s->count > 0) {
            sender_next_byte(s, format);   /* back to back */
        } else {
            s->active = 0;
            sender_drive(line, sim, instance, LOGIC_Z);
            return pin_level;
        }
    }
    s->count_ticks--;
    sender_drive(line, sim, instance, sender_bit(s));
    return s->level;
}
