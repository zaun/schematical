/* models/serial_max310x_data.c: part of the model in models/serial_max310x.c (split by plan-68-I/J). */
#include "serial_max310x_int.h"
/* ---- FIFOs and the receive path ------------------------------------------------------------------- */

void m31x_rx_clear(Chan *c) {
    if (c->rx_count != 0 && !m31x_bit(c, R_MODE2, M2_RXEMPTYINV)) {
        c->isr |= I_RXEMPTY;
    }
    c->rx_head = 0;
    c->rx_count = 0;
    c->idle_ticks = 0;
}

void m31x_tx_clear(Chan *c) {
    if (c->tx_count != 0) {
        c->isr |= I_TXEMPTY;
    }
    c->tx_head = 0;
    c->tx_count = 0;
    c->isr &= (uint8_t)~I_TXTRIG;
}

/* Auto RTS and XON/XOFF sending follow the RX level (MAX3108 p22 wording). */
void m31x_flow_update(Chan *c) {
    unsigned halt = (m31x_rv(c, R_FLOWLVL) & 15u) * 8u;
    unsigned resume = (m31x_rv(c, R_FLOWLVL) >> 4) * 8u;
    unsigned sel = m31x_rv(c, R_FLOWCTRL) >> 6;   /* SwFlow[3:2] */

    if (c->rx_count > halt) {
        c->rts_flow = 1;
    } else if (c->rx_count < resume) {
        c->rts_flow = 0;
    }
    if (!m31x_bit(c, R_FLOWCTRL, FC_SWFLOWEN) || sel == 0) {
        return;
    }
    if (c->rx_count > halt && !c->xoff_sent) {
        c->xoff_sent = 1;
        c->ctl_n = 0;
        if (sel & 2u) c->ctl[c->ctl_n++] = m31x_rv(c, R_XOFF1);
        if (sel & 1u) c->ctl[c->ctl_n++] = m31x_rv(c, R_XOFF2);
    } else if (c->rx_count < resume && c->xoff_sent) {
        c->xoff_sent = 0;
        c->ctl_n = 0;
        if (sel & 2u) c->ctl[c->ctl_n++] = m31x_rv(c, R_XON1);
        if (sel & 1u) c->ctl[c->ctl_n++] = m31x_rv(c, R_XON2);
    }
}

static void rx_push(Chan *c, const RxWord *w) {
    unsigned trig = m31x_trigger_rx(c);

    if (m31x_bit(c, R_MODE2, M2_FIFORST)) {
        return;
    }
    if (c->rx_count >= MX_FIFO) {
        c->lsr |= L_OVERRUN;   /* the new word is lost */
        return;
    }
    c->rx[(c->rx_head + c->rx_count) % MX_FIFO] = *w;
    c->rx_count++;
    if (c->rx_count == 1 && m31x_bit(c, R_MODE2, M2_RXEMPTYINV)) {
        c->isr |= I_RXEMPTY;
    }
    if (c->rx_count == trig && !m31x_bit(c, R_MODE2, M2_RXTRIGINV)) {
        c->isr |= I_RXTRIG;
    }
    m31x_flow_update(c);
}

/* The multidrop / special-character address: XOFF2, or XOFF2[7:4] with the
   GPIO inputs (GPIAddr). */
static uint8_t own_address(const Chan *c) {
    if (m31x_bit(c, R_FLOWCTRL, FC_GPIADDR)) {
        return (uint8_t)((m31x_rv(c, R_XOFF2) & 0xF0) | (c->gpi & 0x0F));
    }
    return m31x_rv(c, R_XOFF2);
}

static uint8_t xoff2_compare(const Chan *c) {
    return own_address(c);
}

/* A received word through flow control, special characters and multidrop. */
static void rx_word(Chan *c, RxWord w) {
    const Variant *v = c->owner->dev->v;
    unsigned len = 5 + (m31x_rv(c, R_LCR) & 3u);
    uint8_t byte = (uint8_t)(w.data & ((1u << len) - 1));
    int known = (w.known & ((1u << len) - 1)) == ((1u << len) - 1);
    unsigned sel = (m31x_rv(c, R_FLOWCTRL) >> 4) & 3u;   /* SwFlow[1:0] */
    int flow = m31x_bit(c, R_FLOWCTRL, FC_SWFLOWEN);
    int special = m31x_bit(c, R_MODE2, M2_SPECIAL);

    if (w.flags & L_BREAK) {
        c->spcl |= S_BREAK;
        rx_push(c, &w);
        return;
    }
    if (m31x_bit(c, R_MODE2, M2_MULTIDROP) && (c->rxf.data_bits == len + 1)) {
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

            if (c->held_kind == 1 && byte == m31x_rv(c, R_XON2)) {
                c->tx_halted = 0;
                done = 1;
            } else if (c->held_kind == 2 && byte == m31x_rv(c, R_XOFF2)) {
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
            if (byte == m31x_rv(c, R_XON1) || byte == m31x_rv(c, R_XOFF1)) {
                c->held_valid = 1;
                c->held = w;
                c->held_kind = byte == m31x_rv(c, R_XON1) ? 1 : 2;
                return;
            }
        } else if (fsel == 2) {
            if (byte == m31x_rv(c, R_XON1)) { c->tx_halted = 0; return; }
            if (byte == m31x_rv(c, R_XOFF1)) { c->tx_halted = 1; return; }
        } else if (fsel == 1) {
            if (byte == m31x_rv(c, R_XON2)) { c->tx_halted = 0; return; }
            if (byte == m31x_rv(c, R_XOFF2)) { c->tx_halted = 1; return; }
        }
    }
    if (special) {
        unsigned ssel = flow ? 1u : sel;   /* with flow on: XON2/XOFF2 are special */

        if ((ssel & 2u) && byte == m31x_rv(c, R_XON1)) c->spcl |= S_XON1;
        if ((ssel & 2u) && byte == m31x_rv(c, R_XOFF1)) c->spcl |= S_XOFF1;
        if ((ssel & 1u) && byte == m31x_rv(c, R_XON2)) c->spcl |= S_XON2;
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
    if (c->echo_ticks > 0 || (m31x_bit(c, R_MODE2, M2_ECHO) && serial_tx_busy(&c->line.tx))) {
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
    LogicValue pin = m31x_level(s, s->pins->channel[c->index][CH_RX], LOGIC_1);
    LogicValue in = serial_line_rx_input(&c->line, s->sim, s->instance, pin, &c->rxf);
    SerialRxPhase phase = c->line.rx.phase;
    unsigned bitno = c->line.rx.bit;

    if (m31x_bit(c, R_MODE2, M2_LOOP)) {
        in = m31x_tx_signal(c);
    }
    if (!c->line.sender.active || m31x_bit(c, R_MODE2, M2_LOOP)) {
        if (m31x_bit(c, R_IRDA, IR_RXINV)) {
            in = m31x_not3(in);
        }
        if (m31x_bit(c, R_IRDA, IR_EN) && c->factor == 16) {
            in = irda_decode(c, in);
        }
    }
    if (m31x_bit(c, R_MODE1, M1_RXDIS) && !serial_rx_busy(&c->line.rx)) {
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
    unsigned len = 5 + (m31x_rv(c, R_LCR) & 3u);
    unsigned before;

    if (c->in_rst || m31x_bit(c, R_MODE2, M2_FIFORST)) {
        return 0;
    }
    if (m31x_bit(c, R_MODE1, M1_TXDIS) && !c->sync_go) {
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
    if (m31x_bit(c, R_FLOWCTRL, FC_AUTOCTS) && m31x_cts_internal(c) != LOGIC_0) {
        return 0;
    }
    if (m31x_bit(c, R_MODE1, M1_TRNSCV)) {
        if (c->dir == DIR_IDLE) {
            unsigned setup = m31x_rv(c, R_HDPLX) >> 4;

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
    if (before >= m31x_trigger_tx(c) && c->tx_count < m31x_trigger_tx(c)) {
        c->isr &= (uint8_t)~I_TXTRIG;
    }
ninth:
    if (m31x_bit(c, R_MODE2, M2_MULTIDROP) && c->txf.data_bits == len + 1) {
        /* The 9th bit: forced high with EvenParity = 0, low with 1 (p19). */
        if (m31x_bit(c, R_LCR, LCR_FORCE)) {
            if (!m31x_bit(c, R_LCR, LCR_EVEN)) {
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
        if (m31x_bit(c, R_MODE1, M1_TRNSCV) && c->dir == DIR_SEND) {
            unsigned hold = m31x_rv(c, R_HDPLX) & 15u;

            c->dir = hold > 0 ? DIR_HOLD : DIR_IDLE;
            c->dir_count = hold * c->factor;
        }
        if (m31x_bit(c, R_MODE2, M2_ECHO)) {
            c->echo_ticks = (m31x_rv(c, R_HDPLX) & 15u) * c->factor;
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
    return c->rx_count == 0 && c->tx_count == 0 && !m31x_pending(c) && !serial_tx_busy(&c->line.tx) &&
           !serial_rx_busy(&c->line.rx);
}

void m31x_chan_tick(Chan *c) {
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
    if (c->rx_count > 0 && m31x_rv(c, R_RXTIMEOUT) != 0 && !(c->lsr & L_TIMEOUT)) {
        uint64_t limit = (uint64_t)m31x_rv(c, R_RXTIMEOUT) * serial_format_char_bits(&c->rxf) * c->factor;

        if (++c->idle_ticks >= limit) {
            c->lsr |= L_TIMEOUT;
        }
    }
    if (s->dev->v->sleep && m31x_bit(c, R_MODE1, M1_AUTOSLEEP)) {
        if (sleep_quiet(c)) {
            if (++c->sleep_idle >= 65536ull * serial_format_char_bits(&c->rxf) * c->factor) {
                c->sleeping = 1;
                c->sleep_idle = 0;
                m31x_brg_restart(c);
                m31x_update_reference(s);
            }
        } else {
            c->sleep_idle = 0;
        }
    }
    m31x_update_outputs(s);
}

/* ---- The reference clock ------------------------------------------------------------------------------- */

void m31x_update_reference(MxState *s) {
    Chan *c0 = m31x_clock_chan(s);
    uint8_t ck = m31x_rv(c0, R_CLKSRC);
    uint8_t pll = m31x_rv(c0, R_PLLCFG);
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
            m31x_brg_restart(&s->ch[u]);
        }
    } else {
        for (u = 0; u < s->dev->v->uarts; u++) {
            if (s->ch[u].run != m31x_chan_clock_on(&s->ch[u])) {
                m31x_brg_restart(&s->ch[u]);
            }
        }
    }
}

int m31x_refclk_wanted(const MxState *s) {
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
SimTime m31x_refclk_half(MxState *s) {
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
        LogicValue v = m31x_level(s, s->pins->gpio[i], LOGIC_0);
        uint8_t m = (uint8_t)(1u << b);
        int input = !((m31x_rv(c, R_GPIOCFG) >> b) & 1u);
        uint8_t old_v = c->gpi & m;
        uint8_t old_k = c->gpi_k & m;

        if (c->sleeping) {
            continue;
        }
        c->gpi = (uint8_t)((c->gpi & ~m) | (v == LOGIC_1 ? m : 0));
        c->gpi_k = (uint8_t)((c->gpi_k & ~m) | (v != LOGIC_X ? m : 0));
        if (input && (old_v != (c->gpi & m) || old_k != (c->gpi_k & m))) {
            if (!s->dev->v->gpi_gate || (m31x_rv(c, R_STSEN) & m)) {
                c->sts |= m;
            }
        }
    }
}

static void brg_edge(Chan *c, int rising, LogicValue level_now) {
    if (!c->run) {
        return;
    }
    if (c->brg_target <= 1 && m31x_clk_to_rts(c)) {
        c->clkout = level_now;   /* divisor 1: the clock passes through */
    }
    if (!rising) {
        return;
    }
    if (++c->brg_count >= c->brg_target) {
        c->brg_count = 0;
        c->brg_target = m31x_next_target(c);
        c->clkout = LOGIC_1;
        m31x_chan_tick(c);
    } else if (c->brg_count >= (c->brg_target + 1) / 2) {
        c->clkout = LOGIC_0;
    }
}

void m31x_xin_edge(MxState *s, LogicValue v) {
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
                    m31x_brg_schedule(c, m31x_tick_delay(c));
                } else if (old != 0 && old != p) {
                    /* The clock changed: rescale what is left of this tick. */
                    SimTime left = c->brg_due > now ? c->brg_due - now : 0;

                    m31x_brg_schedule(c, (SimTime)((uint64_t)left * p / old));
                }
            }
            if (m31x_refclk_wanted(s) && old == 0) {
                m31x_schedule(s, TM_REFCLK, 0, s->refclk_gen, m31x_refclk_half(s));
            }
        }
    }
    if (s->ref_mode == 1) {
        for (u = 0; u < s->dev->v->uarts; u++) {
            brg_edge(&s->ch[u], rising, v);
        }
    }
    m31x_update_outputs(s);
}

