#include "serial_max310x_int.h"
#include "logic_alloc.h"
const char *const m31x_TIMING[] = {"tdo", "tft", "tready", NULL};

const char *const m31x_KEYS[] = {
    "variant", "bus", "reset", "clock", "channels", "interrupt", "gpio", NULL
};
const char *const m31x_BUS_EXTRA[] = {"uartOffsets", NULL};
const char *const m31x_CHANNEL_KEYS[] = {"tx", "rx", "rts", "cts", NULL};

const Variant m31x_VARIANTS[] = {
    {"max3107", 1, 0xA1, 0, -1, 0, 1, 1, 0, 1, 0, 1, 0x08, 0, 0, 0, 0, 0, 0, 0, 0, 0x1F},
    {"max3108", 1, -1, 0, -1, 0, 1, 1, 1, 1, 1, 0, 0x18, 0, 0, 1, 0, 0, 0, 1, 1, 0x1E},
    {"max3109", 2, -1, 1, 0xC2, 1, 0, 1, 1, 1, 1, 0, 0x18, 0, 0, 1, 1, 1, 1, 1, 1, 0x25},
    {"max14830", 4, -1, 1, 0xB4, 2, 1, 0, 0, 0, 0, 0, 0x08, 1, 1, 1, 1, 1, 0, 0, 1, 0x25},
};

/* Writable bits per register (others read 0). */
static const uint8_t WRITABLE[0x20] = {
    0xFF, 0xFF, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x6F, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF,
    0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0xFF, 0x3F, 0xFF, 0xFF, 0x9E, 0x00
};

/* ---- Small helpers ------------------------------------------------------------------------ */

static SimTime t_of(const MxState *s, unsigned i) {
    return s->grade->t[i];
}

LogicValue m31x_not3(LogicValue a) {
    return a == LOGIC_0 ? LOGIC_1 : a == LOGIC_1 ? LOGIC_0 : LOGIC_X;
}

LogicValue m31x_bool3(int b) {
    return b ? LOGIC_1 : LOGIC_0;
}

uint8_t m31x_rv(const Chan *c, unsigned reg) {
    return c->r[reg].v;
}

int m31x_bit(const Chan *c, unsigned reg, uint8_t mask) {
    return (c->r[reg].v & c->r[reg].k & mask) != 0;
}

LogicValue m31x_level(const MxState *s, size_t pin, LogicValue z_reads) {
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

void m31x_schedule(MxState *s, unsigned type, unsigned ch, uint32_t gen, SimTime delay) {
    sim_schedule_timer(s->sim, s->instance, ((gen & 0xFFFFFFu) << 8) | (type << 4) | ch, delay);
}

Chan *m31x_clock_chan(MxState *s) {
    return &s->ch[0];
}

/* CLKtoRTS: UART0's CLKSource where the clock registers are shared. */
int m31x_clk_to_rts(const Chan *c) {
    const Chan *k = c->owner->dev->v->shared_clock ? &c->owner->ch[0] : c;

    return (k->r[R_CLKSRC].v & k->r[R_CLKSRC].k & CK_CLKTORTS) != 0;
}

/* The bits a register write stores (others read 0). */
uint8_t m31x_writable(const Variant *v, unsigned addr) {
    switch (addr) {
        case R_STSEN:
            return (uint8_t)(0x2F | (v->sts_sleep ? 0x40 : 0) | (v->sts_txempty ? 0x80 : 0));
        case R_BRGCFG:
            return (uint8_t)(0x3F | (v->clk_disable ? 0x40 : 0));
        default:
            return addr < 0x20 ? WRITABLE[addr] : 0xFF;
    }
}

unsigned m31x_trigger_rx(const Chan *c) {
    return (m31x_rv(c, R_FIFOTRG) >> 4) * 8u;
}

unsigned m31x_trigger_tx(const Chan *c) {
    return (m31x_rv(c, R_FIFOTRG) & 15u) * 8u;
}

/* ---- Interrupts --------------------------------------------------------------------------- */

uint8_t m31x_sts_now(const Chan *c) {
    const MxState *s = c->owner;
    const Variant *v = s->dev->v;
    uint8_t b = c->sts;

    if (v->sts_sleep && c->sleeping) {
        b |= ST_SLEEP;
    }
    if (s->ref_mode != 0 && (m31x_rv(&s->ch[0], R_CLKSRC) & CK_CRYSTAL) &&
        (s->ref_mode == 1 ? s->rises > 0 : s->period > 0) && !c->sleeping) {
        b |= ST_CLOCKREADY;
    }
    return b;
}

uint8_t m31x_lsr_now(const Chan *c) {
    return c->lsr & 0x3F;
}

uint8_t m31x_isr_now(const Chan *c) {
    uint8_t b = c->isr & 0xF8;

    if (m31x_sts_now(c) & m31x_rv(c, R_STSEN)) {
        b |= I_STS;
    }
    if (c->spcl & m31x_rv(c, R_SPCLEN)) {
        b |= I_SPCL;
    }
    if (m31x_lsr_now(c) & m31x_rv(c, R_LSRINTEN)) {
        b |= I_LSR;
    }
    return b;
}

/* The UART has an enabled interrupt pending. */
int m31x_pending(const Chan *c) {
    return (m31x_isr_now(c) & m31x_rv(c, R_IRQEN)) != 0;
}

uint8_t m31x_global_irq(const MxState *s) {
    uint8_t b = 0;
    unsigned u;

    for (u = 0; u < s->dev->v->uarts; u++) {
        if (!m31x_pending(&s->ch[u])) {
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

        if ((!v->irqsel || m31x_bit(c, R_MODE1, M1_IRQSEL)) && m31x_pending(c)) {
            return LOGIC_0;
        }
    }
    return LOGIC_Z;
}

/* ---- Formats and the baud generator -------------------------------------------------------- */

void m31x_update_format(Chan *c) {
    const Variant *v = c->owner->dev->v;
    uint8_t lcr = m31x_rv(c, R_LCR);
    uint8_t brg = m31x_rv(c, R_BRGCFG);
    SerialFormat f = serial_format_default();
    unsigned len = 5 + (lcr & 3u);

    c->factor = (brg & 0x20) ? 4 : (brg & 0x10) ? 8 : 16;
    f.factor = c->factor;
    f.data_bits = len;
    f.stop_halves = (lcr & 4u) ? (len == 5 ? 3 : 4) : 2;
    f.majority = c->factor == 16 || (c->factor == 8 && v->noise_2x);
    if (m31x_bit(c, R_MODE2, M2_MULTIDROP)) {
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

uint32_t m31x_divisor(const Chan *c) {
    return (uint32_t)m31x_rv(c, R_DIVLSB) | ((uint32_t)m31x_rv(c, R_DIVMSB) << 8);
}

static int divisor_known(const Chan *c) {
    return c->r[R_DIVLSB].k == 0xFF && c->r[R_DIVMSB].k == 0xFF && (c->r[R_BRGCFG].k & 0x7F) == 0x7F;
}

/* The next tick's length in fREF cycles (DIV, plus one when the fraction
   carries). */
uint32_t m31x_next_target(Chan *c) {
    uint32_t t = m31x_divisor(c);

    c->brg_acc += m31x_rv(c, R_BRGCFG) & 15u;
    if (c->brg_acc >= 16) {
        c->brg_acc -= 16;
        t++;
    }
    return t;
}

int m31x_chan_clock_on(const Chan *c) {
    const MxState *s = c->owner;

    return s->ref_mode != 0 && !s->in_reset && !s->unknown && !c->sleeping && !c->in_rst &&
           !(s->dev->v->clk_disable && m31x_bit(c, R_BRGCFG, 0x40)) && divisor_known(c) &&
           m31x_divisor(c) != 0;
}

SimTime m31x_tick_delay(Chan *c) {
    MxState *s = c->owner;
    uint64_t num = (uint64_t)s->period * s->prediv * m31x_next_target(c);
    uint64_t d = num / s->mult;

    c->brg_rem += num % s->mult;
    if (c->brg_rem >= s->mult) {
        c->brg_rem -= s->mult;
        d++;
    }
    return (SimTime)d;
}

void m31x_brg_schedule(Chan *c, SimTime delay) {
    MxState *s = c->owner;

    c->brg_gen++;
    c->brg_pending = 1;
    c->brg_delay = delay;
    c->brg_due = sim_now(s->sim) + delay;
    m31x_schedule(s, TM_BRG, c->index, c->brg_gen, delay);
}

/* The generator's configuration changed: restart it. */
void m31x_brg_restart(Chan *c) {
    MxState *s = c->owner;
    int on = m31x_chan_clock_on(c);

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
    c->brg_target = m31x_next_target(c);
    if (s->ref_mode == 2 && s->period > 0 && sim_now(s->sim) - s->last_rise <= 2 * s->period) {
        m31x_brg_schedule(c, m31x_tick_delay(c));
    }
}

/* ---- Outputs -------------------------------------------------------------------------------- */

/* The internal RTS (before RTSHiZ, inversion and loopback's pin rule).
   Never asked for in reset: m31x_update_outputs drives RTS high itself
   then, and m31x_cts_internal is only reached from tx_fetch, which runs on
   a baud tick, and the generator is stopped in reset (m31x_chan_clock_on). */
static LogicValue rts_internal(const Chan *c) {
    if (m31x_clk_to_rts(c)) {
        return c->clkout;
    }
    if (m31x_bit(c, R_MODE1, M1_TRNSCV)) {
        return m31x_bool3(c->dir != DIR_IDLE);
    }
    if (m31x_bit(c, R_FLOWCTRL, FC_AUTORTS)) {
        return m31x_bool3(c->rts_flow);
    }
    return (c->r[R_LCR].k & LCR_RTS) ? m31x_bool3(m31x_rv(c, R_LCR) & LCR_RTS) : LOGIC_X;
}

LogicValue m31x_cts_internal(const Chan *c) {
    if (m31x_bit(c, R_MODE2, M2_LOOP)) {
        return rts_internal(c);
    }
    return c->cts;
}

/* The TX signal as it leaves the chip (or loops back), before 3-state. */
LogicValue m31x_tx_signal(const Chan *c) {
    LogicValue v = c->tx_level;
    const MxState *s = c->owner;

    if (s->in_reset) {
        return LOGIC_1;
    }
    if (m31x_bit(c, R_LCR, LCR_BREAK)) {
        v = LOGIC_0;
    }
    if (m31x_bit(c, R_IRDA, IR_EN) && c->factor == 16) {
        unsigned width = m31x_bit(c, R_IRDA, IR_MIR) ? 4 : 3;

        if (v != LOGIC_1 && c->line.tx.count >= width) {
            v = LOGIC_1;
        }
    }
    if (m31x_bit(c, R_IRDA, IR_TXINV)) {
        v = m31x_not3(v);
    }
    return v;
}

void m31x_update_gpio(MxState *s) {
    unsigned i;

    for (i = 0; i < s->dev->gpio_count; i++) {
        const Chan *c = &s->ch[i / 4];
        unsigned b = i % 4;
        uint8_t cfg = m31x_rv(c, R_GPIOCFG);
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
                               ? m31x_bool3((m31x_rv(c, R_GPIODATA) >> b) & 1u)
                               : LOGIC_X;

            v = ((cfg >> (4 + b)) & 1u) ? (d == LOGIC_1 ? LOGIC_Z : d) : d;
        } else {
            v = LOGIC_Z;
        }
        drive(s, s->pins->gpio[i], v, 0);
    }
}

void m31x_update_outputs(MxState *s) {
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
        tx = m31x_tx_signal(c);
        if (!s->in_reset && (m31x_bit(c, R_MODE1, M1_TXHIZ) || m31x_bit(c, R_MODE2, M2_LOOP))) {
            tx = LOGIC_Z;
        }
        drive(s, pin[CH_TX], tx, 0);
        if (s->in_reset) {
            rts = LOGIC_1;
        } else if (m31x_bit(c, R_MODE2, M2_LOOP)) {
            rts = (c->r[R_LCR].k & LCR_RTS) ? m31x_bool3(m31x_rv(c, R_LCR) & LCR_RTS) : LOGIC_X;
        } else {
            rts = rts_internal(c);
        }
        if (!s->in_reset && s->dev->v->rts_invert && m31x_bit(c, R_IRDA, IR_RTSINV)) {
            rts = m31x_not3(rts);
        }
        if (!s->in_reset && m31x_bit(c, R_MODE1, M1_RTSHIZ)) {
            rts = LOGIC_Z;
        }
        if (m31x_clk_to_rts(c)) {
            drive_clock(s, pin[CH_RTS], rts);
        } else {
            drive(s, pin[CH_RTS], rts, 0);
        }
    }
    m31x_update_gpio(s);
}

/* ---- Reset ----------------------------------------------------------------------------------------------- */

void m31x_reset_chan(Chan *c) {
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
    m31x_update_format(c);
}

void m31x_ready_now(MxState *s) {
    s->ready = 1;
    m31x_update_outputs(s);
}

void m31x_apply_reset_pin(MxState *s) {
    LogicValue v = m31x_level(s, s->pins->reset, LOGIC_1);
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
            m31x_reset_chan(&s->ch[u]);
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
            m31x_schedule(s, TM_READY, 0, s->ready_gen, t_of(s, T_READY));
        }
        serial_host_hold(&s->host, s->sim, s->instance, 0);
    }
    m31x_update_reference(s);
    m31x_update_outputs(s);
}

/* ---- Terminal ------------------------------------------------------------------------------------------------- */

unsigned m31x_mx_channels(const void *raw) {
    return ((const MxState *)raw)->dev->v->uarts;
}

SerialLine *m31x_mx_line(void *raw, unsigned channel) {
    return &((MxState *)raw)->ch[channel].line;
}

int m31x_mx_async(const void *raw, unsigned channel) {
    const Chan *c = &((const MxState *)raw)->ch[channel];

    /* IrDA pulses and inverted RX are not the terminal's NRZ waveform. */
    return !m31x_bit(c, R_IRDA, IR_EN) && !m31x_bit(c, R_IRDA, IR_RXINV);
}

/* ---- Create / destroy ------------------------------------------------------------------------------------------- */

extern const SerialModel MAX310X_MODEL;

void *m31x_mx_create(const Part *part, const void *data, const void *pins,
                       const SerialGrade *grade, const void *options) {
    MxState *s = logic_calloc(1, sizeof *s);
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
    s->out = logic_malloc(part->pin_count * sizeof *s->out);
    if (s->out == NULL) {
        logic_free(s);
        return NULL;
    }
    for (i = 0; i < part->pin_count; i++) {
        s->out[i] = LOGIC_Z;
    }
    serial_host_init(&s->host, &s->dev->host, &s->pins->host);
    s->host.spi.t_out = grade->t[T_DO];
    s->host.spi.t_float = grade->t[T_FT];
    s->spi_events.context = s;
    s->spi_events.begin = m31x_spi_begin;
    s->spi_events.next = m31x_spi_next;
    s->spi_events.bit = m31x_spi_bit;
    s->spi_events.received = m31x_spi_received;
    s->i2c_events.context = s;
    s->i2c_events.begin = m31x_i2c_begin;
    s->i2c_events.address = m31x_i2c_address;
    s->i2c_events.received = m31x_i2c_received;
    s->i2c_events.next = m31x_i2c_next;
    s->i2c_events.acked = m31x_i2c_acked;
    s->i2c_ch = -1;
    s->i2c_ptr = -1;
    s->xin = LOGIC_X;
    s->refclk = LOGIC_0;
    for (u = 0; u < s->dev->v->uarts; u++) {
        Chan *c = &s->ch[u];

        c->owner = s;
        c->index = u;
        serial_line_init(&c->line, s->pins->channel[u][CH_RX]);
        m31x_reset_chan(c);
        c->cts = LOGIC_X;
    }
    s->in_reset = 1;
    return s;
}

void m31x_mx_destroy(void *raw) {
    MxState *s = raw;
    unsigned u;

    for (u = 0; u < s->dev->v->uarts; u++) {
        serial_line_free(&s->ch[u].line);
    }
    logic_free(s->out);
    logic_free(s);
}

int m31x_mx_open_drain(const void *data, const void *pins, const void *options, size_t pin) {
    const MxDevice *d = data;
    const MxPins *p = pins;

    /* pin is a part pin index (part_pin_open_drain via serial_pin_open_drain),
       never SERIAL_NO_PIN; IRQ is a required pin. */
    (void)options;
    return pin == p->irq || serial_host_open_drain(&d->host, &p->host, pin);
}

