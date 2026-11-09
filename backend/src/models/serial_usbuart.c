/*
 * usbuart: USB-to-UART bridges (CP210x, CY7C652xx, MCP2200, XR21B14xx,
 * XR21V141x). The USB side is the app's terminal, playing the PC: bytes
 * typed into a channel's terminal go out its TXD pin, and characters the
 * circuit sends into RXD show in the terminal (the channel's sent log). The
 * port's baud and format are the PC's to set (circuit_serial_configure);
 * they default to 9600 8N1, no flow control.
 *
 * The bridges have their own oscillator, so the line runs on sim timers at
 * 16 ticks a bit, and only while a frame is on the wire. The receiver starts
 * ticking at a fall on RXD and stops once it is idle with RXD high; the
 * transmitter ticks while it has a frame or queued bytes.
 *
 * Pins, as the PC sees an open port with the USB link up: RTS and DTR
 * (active low) are driven 0, SUSPEND 0 and SUSPEND-bar 1. TX and RX
 * activity outputs (active low) are 0 while a frame is sent or received.
 * With RTS/CTS flow control a frame starts only while CTS reads 0. DSR,
 * DCD and RI are read by nothing. RXD and CTS at Z read 1 (the bridges'
 * pull-ups). Reset (active low, Z reads inactive): while 0 every output is
 * released and the queue dropped; while X every output is X.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serial.h"

enum { UB_MAX_CHANNELS = SERIAL_MAX_CHANNELS };

/* A channel's pins: keys in this order. */
enum { CP_TXD, CP_RXD, CP_RTS, CP_CTS, CP_DTR, CP_DSR, CP_DCD, CP_RI, CP_TXLED, CP_RXLED, CP_COUNT };
static const char *const CHANNEL_KEYS[] = {"txd", "rxd", "rts", "cts", "dtr", "dsr", "dcd", "ri",
                                           "txLed", "rxLed", NULL};

static const char *const KEYS[] = {"channels", "reset", "suspend", "suspendLow", "usbSupply", NULL};
static const char *const TIMING[] = {NULL};

typedef struct {
    const char *ch[UB_MAX_CHANNELS][CP_COUNT];
    unsigned channels;
    const char *reset, *suspend, *suspend_low;
} UbDevice;

typedef struct {
    size_t ch[UB_MAX_CHANNELS][CP_COUNT];
    size_t reset, suspend, suspend_low;
} UbPins;

/* Timer ids: (generation << 8) | (channel << 1) | kind. */
enum { TM_RX = 0, TM_TX = 1 };

typedef struct {
    SerialLine line;          /* log: received from RXD; sender queue: to send on TXD */
    SerialFormat format;
    uint32_t baud;
    int rts_cts;
    int rx_running;
    int tx_running;
} UbChannel;

typedef struct {
    SerialBase base;
    const UbDevice *dev;
    const UbPins *pins;
    Sim *sim;
    SimInstance *instance;
    LogicValue *out;
    size_t pin_count;
    LogicValue reset;         /* the reset input now: 1 running, 0 in reset, X unknown */
    uint32_t gen;             /* invalidates stale timers */
    UbChannel ch[UB_MAX_CHANNELS];
} UbState;

const SerialModel USBUART_MODEL;

/* ---- Pins ------------------------------------------------------------------------------- */

static void drive(UbState *s, size_t pin, LogicValue v) {
    if (!SERIAL_REAL_PIN(pin) || pin >= s->pin_count || s->out[pin] == v) {
        return;
    }
    s->out[pin] = v;
    sim_schedule_drive(s->sim, s->instance, pin, v, DRIVE_STRONG, 0);
}

static LogicValue input(const UbState *s, size_t pin) {
    if (!SERIAL_REAL_PIN(pin)) {
        return LOGIC_1;
    }
    return serial_level(s->sim, s->instance, pin, LOGIC_1);
}

/* The level of an output while running, or released / X per reset. */
static LogicValue gated(const UbState *s, LogicValue v) {
    return s->reset == LOGIC_1 ? v : s->reset == LOGIC_0 ? LOGIC_Z : LOGIC_X;
}

static void drive_outputs(UbState *s) {
    unsigned c;

    for (c = 0; c < s->dev->channels; c++) {
        const size_t *p = s->pins->ch[c];
        UbChannel *ch = &s->ch[c];

        drive(s, p[CP_TXD], gated(s, ch->line.tx.level));
        drive(s, p[CP_RTS], gated(s, LOGIC_0));
        drive(s, p[CP_DTR], gated(s, LOGIC_0));
        drive(s, p[CP_TXLED], gated(s, serial_tx_busy(&ch->line.tx) ? LOGIC_0 : LOGIC_1));
        drive(s, p[CP_RXLED], gated(s, serial_rx_busy(&ch->line.rx) ? LOGIC_0 : LOGIC_1));
    }
    drive(s, s->pins->suspend, gated(s, LOGIC_0));
    drive(s, s->pins->suspend_low, gated(s, LOGIC_1));
}

/* ---- The line ----------------------------------------------------------------------------- */

static SimTime tick_ps(const UbChannel *ch) {
    return (SimTime)(1000000000000.0 / ((double)ch->baud * ch->format.factor) + 0.5);
}

static uint32_t timer_id(const UbState *s, unsigned c, unsigned kind) {
    return (s->gen << 8) | (c << 1) | kind;
}

typedef struct {
    UbState *s;
    unsigned c;
} UbContext;

static int can_send(const UbState *s, unsigned c) {
    return !s->ch[c].rts_cts || input(s, s->pins->ch[c][CP_CTS]) == LOGIC_0;
}

static int tx_fetch(void *raw, uint16_t *data, uint16_t *known) {
    UbContext *ctx = raw;
    SerialSender *q = &ctx->s->ch[ctx->c].line.sender;

    if (q->count == 0 || !can_send(ctx->s, ctx->c)) {
        return 0;
    }
    *data = q->bytes[q->head];
    *known = 0xFF;
    q->head = (q->head + 1) % q->capacity;
    q->count--;
    return 1;
}

static void rx_received(void *raw, const SerialRxChar *c) {
    UbContext *ctx = raw;

    if (!(c->flags & c->flags_known & SERIAL_RX_BI)) {
        serial_line_log(&ctx->s->ch[ctx->c].line, c->data & c->known);
    }
}

/* Starts the transmitter's ticks when it has something to send. */
static void tx_wake(UbState *s, unsigned c) {
    UbChannel *ch = &s->ch[c];

    if (s->reset != LOGIC_1 || ch->tx_running || ch->line.sender.count == 0 || !can_send(s, c)) {
        return;
    }
    ch->tx_running = 1;
    sim_schedule_timer(s->sim, s->instance, timer_id(s, c, TM_TX), 0);
}

/* Starts the receiver's ticks at a fall (or X) on RXD. */
static void rx_wake(UbState *s, unsigned c) {
    UbChannel *ch = &s->ch[c];

    if (s->reset != LOGIC_1 || ch->rx_running || input(s, s->pins->ch[c][CP_RXD]) == LOGIC_1) {
        return;
    }
    ch->rx_running = 1;
    sim_schedule_timer(s->sim, s->instance, timer_id(s, c, TM_RX), tick_ps(ch));
}

static void tx_tick(UbState *s, unsigned c) {
    UbChannel *ch = &s->ch[c];
    UbContext ctx = {s, c};
    SerialTxEvents events = {&ctx, tx_fetch, NULL, NULL};

    serial_tx_tick(&ch->line.tx, &ch->format, &events);
    drive_outputs(s);
    if (serial_tx_busy(&ch->line.tx) || (ch->line.sender.count > 0 && can_send(s, c))) {
        sim_schedule_timer(s->sim, s->instance, timer_id(s, c, TM_TX), tick_ps(ch));
    } else {
        ch->tx_running = 0;
    }
}

static void rx_tick(UbState *s, unsigned c) {
    UbChannel *ch = &s->ch[c];
    UbContext ctx = {s, c};
    SerialRxEvents events = {&ctx, rx_received, NULL};
    LogicValue level = input(s, s->pins->ch[c][CP_RXD]);

    serial_rx_tick(&ch->line.rx, level, &ch->format, &events);
    drive_outputs(s);
    if (serial_rx_busy(&ch->line.rx) || level != LOGIC_1) {
        sim_schedule_timer(s->sim, s->instance, timer_id(s, c, TM_RX), tick_ps(ch));
    } else {
        ch->rx_running = 0;
    }
}

/* Drops frames in progress and stale timers; the receiver waits for a fresh fall. */
static void restart_lines(UbState *s) {
    unsigned c;

    s->gen = (s->gen + 1) & 0xFFFFFFu;
    for (c = 0; c < s->dev->channels; c++) {
        UbChannel *ch = &s->ch[c];

        serial_tx_reset(&ch->line.tx);
        serial_rx_reset(&ch->line.rx);
        ch->line.rx.last = LOGIC_1;
        ch->rx_running = 0;
        ch->tx_running = 0;
    }
}

static void wake_all(UbState *s) {
    unsigned c;

    for (c = 0; c < s->dev->channels; c++) {
        rx_wake(s, c);
        tx_wake(s, c);
    }
}

static void read_reset(UbState *s) {
    LogicValue r = SERIAL_REAL_PIN(s->pins->reset) ? input(s, s->pins->reset) : LOGIC_1;
    unsigned c;

    if (r == s->reset) {
        return;
    }
    s->reset = r;
    restart_lines(s);
    if (r != LOGIC_1) {
        for (c = 0; c < s->dev->channels; c++) {
            s->ch[c].line.sender.count = 0;
            s->ch[c].line.sender.head = 0;
        }
    }
    drive_outputs(s);
    wake_all(s);
}

/* ---- Model callbacks ------------------------------------------------------------------------ */

static void ub_start(Sim *sim, SimInstance *instance, void *raw) {
    UbState *s = raw;

    s->sim = sim;
    s->instance = instance;
    s->reset = LOGIC_Z;   /* not a level read_reset can match: always applies */
    read_reset(s);
}

static void ub_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    UbState *s = raw;
    unsigned c;

    (void)old_value;
    (void)new_value;
    s->sim = sim;
    s->instance = instance;
    if (pin == s->pins->reset) {
        read_reset(s);
        return;
    }
    for (c = 0; c < s->dev->channels; c++) {
        if (pin == s->pins->ch[c][CP_RXD]) {
            rx_wake(s, c);
        } else if (pin == s->pins->ch[c][CP_CTS]) {
            tx_wake(s, c);
        }
    }
}

static void ub_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t id) {
    UbState *s = raw;
    unsigned c = (id >> 1) & 0x7Fu;

    s->sim = sim;
    s->instance = instance;
    if ((id >> 8) != s->gen || c >= s->dev->channels || s->reset != LOGIC_1) {
        return;
    }
    if ((id & 1u) == TM_TX) {
        tx_tick(s, c);
    } else {
        rx_tick(s, c);
    }
}

static unsigned ub_channels(const void *raw) {
    return ((const UbState *)raw)->dev->channels;
}

static SerialLine *ub_line(void *raw, unsigned channel) {
    return &((UbState *)raw)->ch[channel].line;
}

static int ub_async(const void *raw, unsigned channel) {
    (void)raw;
    (void)channel;
    return 1;
}

static void ub_queued(Sim *sim, SimInstance *instance, void *raw, unsigned channel) {
    UbState *s = raw;

    s->sim = sim;
    s->instance = instance;
    tx_wake(s, channel);
}

static int ub_configure(Sim *sim, SimInstance *instance, void *raw, unsigned channel,
                        const SerialPortSettings *port) {
    UbState *s = raw;
    UbChannel *ch = &s->ch[channel];

    if (port->baud < 50 || port->baud > 12000000 || port->data_bits < 5 || port->data_bits > 8 ||
        port->parity > SERIAL_PARITY_SPACE || port->stop_halves < 2 || port->stop_halves > 4) {
        return 0;
    }
    ch->baud = port->baud;
    ch->format = serial_format_default();
    ch->format.data_bits = port->data_bits;
    ch->format.parity = (SerialParity)port->parity;
    ch->format.stop_halves = port->stop_halves;
    ch->rts_cts = port->rts_cts != 0;
    if (sim != NULL && instance != NULL && s->sim != NULL) {
        /* Running: a frame in progress is lost, as when a PC changes the port mid-frame. */
        s->sim = sim;
        s->instance = instance;
        restart_lines(s);
        drive_outputs(s);
        wake_all(s);
    }
    return 1;
}

static void *ub_create(const Part *part, const void *data, const void *pins,
                       const SerialGrade *grade, const void *options) {
    UbState *s = calloc(1, sizeof *s);
    size_t i;
    unsigned c;

    (void)grade;
    (void)options;
    if (s == NULL) {
        return NULL;
    }
    s->base.model = &USBUART_MODEL;
    s->dev = data;
    s->pins = pins;
    s->pin_count = part->pin_count;
    s->out = malloc(part->pin_count * sizeof *s->out);
    if (s->out == NULL) {
        free(s);
        return NULL;
    }
    for (i = 0; i < part->pin_count; i++) {
        s->out[i] = LOGIC_Z;
    }
    for (c = 0; c < s->dev->channels; c++) {
        UbChannel *ch = &s->ch[c];

        serial_line_init(&ch->line, SIZE_MAX);
        ch->format = serial_format_default();
        ch->baud = 9600;
        ch->line.rx.last = LOGIC_1;
    }
    return s;
}

static void ub_destroy(void *raw) {
    UbState *s = raw;
    unsigned c;

    for (c = 0; c < s->dev->channels; c++) {
        serial_line_free(&s->ch[c].line);
    }
    free(s->out);
    free(s);
}

/* ---- Loader --------------------------------------------------------------------------------- */

static void *ub_load_device(KindContext *ctx, const cJSON *json, unsigned *channels) {
    UbDevice *d = kind_alloc(ctx, sizeof *d);
    const cJSON *list;
    const cJSON *item;
    unsigned c = 0;
    unsigned k;

    if (d == NULL) {
        return NULL;
    }
    list = kind_member(ctx, json, "channels", 1);
    if (list == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) < 1 ||
        cJSON_GetArraySize(list) > UB_MAX_CHANNELS) {
        kind_error(ctx, "\"channels\" must be an array of 1 to %d objects", UB_MAX_CHANNELS);
        return NULL;
    }
    cJSON_ArrayForEach(item, list) {
        kind_push(ctx, "channels[%u]", c);
        if (kind_check_keys(ctx, item, CHANNEL_KEYS, NULL) != 0) {
            return NULL;
        }
        for (k = 0; k < CP_COUNT; k++) {
            if (serial_optional_name(ctx, item, CHANNEL_KEYS[k], &d->ch[c][k]) != 0) {
                return NULL;
            }
        }
        if (d->ch[c][CP_TXD] == NULL || d->ch[c][CP_RXD] == NULL) {
            kind_error(ctx, "missing \"%s\"", d->ch[c][CP_TXD] == NULL ? "txd" : "rxd");
            return NULL;
        }
        kind_pop(ctx);
        c++;
    }
    d->channels = c;
    if (serial_optional_name(ctx, json, "reset", &d->reset) != 0 ||
        serial_optional_name(ctx, json, "suspend", &d->suspend) != 0 ||
        serial_optional_name(ctx, json, "suspendLow", &d->suspend_low) != 0) {
        return NULL;
    }
    *channels = c;
    return d;
}

static void *ub_bind(KindContext *ctx, const void *data, const PartPin *pins, size_t pin_count) {
    const UbDevice *d = data;
    UbPins *p = kind_alloc(ctx, sizeof *p);
    char path[48];
    unsigned c;
    unsigned k;

    if (p == NULL) {
        return NULL;
    }
    for (c = 0; c < d->channels; c++) {
        for (k = 0; k < CP_COUNT; k++) {
            snprintf(path, sizeof path, "channels[%u].%s", c, CHANNEL_KEYS[k]);
            if (serial_bind_pin(ctx, pins, pin_count, path, d->ch[c][k], &p->ch[c][k]) != 0) {
                return NULL;
            }
        }
    }
    if (serial_bind_pin(ctx, pins, pin_count, "reset", d->reset, &p->reset) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "suspend", d->suspend, &p->suspend) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "suspendLow", d->suspend_low, &p->suspend_low) != 0) {
        return NULL;
    }
    return p;
}

const SerialModel USBUART_MODEL = {
    "usbuart", KEYS, TIMING, NULL,
    ub_load_device, ub_bind, NULL, NULL,
    ub_create, ub_destroy, ub_start, ub_pin_changed, ub_timer,
    ub_channels, ub_line, ub_async,
    ub_queued, ub_configure, "usb"
};
