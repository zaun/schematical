/* models/serial_max310x_regs.c: part of the model in models/serial_max310x.c (split by plan-68-I/J). */
#include "serial_max310x_int.h"
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
    uint8_t m;

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
        case R_ISR: *v = m31x_isr_now(c); return;
        case R_LSR:
            *v = m31x_lsr_now(c);
            *k = (uint8_t)(c->lsr_k | 0xC3);
            if (c->cts == LOGIC_1) *v |= 0x80;
            if (c->cts == LOGIC_X) *k &= 0x7F;
            return;
        case R_SPCL: *v = c->spcl; return;
        case R_STS:
            m = 0x0F | ST_CLOCKREADY;
            if (var->sts_txempty) m |= ST_TXEMPTY;
            if (var->sts_sleep) m |= ST_SLEEP;
            *v = m31x_sts_now(c) & m;
            return;
        case R_MODE1:
            m = 0x1F;
            if (var->irqsel) m |= M1_IRQSEL;
            if (var->sleep) m |= M1_AUTOSLEEP | M1_FORCEDSLEEP;
            *v = m31x_rv(c, R_MODE1) & m;
            *k = (uint8_t)(c->r[R_MODE1].k | (uint8_t)~m);
            if (var->sleep) {
                *v = (uint8_t)((*v & ~M1_FORCEDSLEEP) | (c->sleeping ? M1_FORCEDSLEEP : 0));
                *k |= M1_FORCEDSLEEP;
            }
            if (var->txdisabl_empty && m31x_bit(c, R_MODE1, M1_TRNSCV) && c->tx_count == 0 &&
                !serial_tx_busy(&c->line.tx)) {
                *v |= M1_TXDIS;
                *k |= M1_TXDIS;
            }
            return;
        case R_TXLVL: *v = (uint8_t)(c->tx_count > 255 ? 255 : c->tx_count); return;
        case R_RXLVL: *v = (uint8_t)(c->rx_count > 255 ? 255 : c->rx_count); return;
        case R_GPIODATA:
            *v = (uint8_t)((c->gpi << 4) | (m31x_rv(c, R_GPIODATA) & 0x0F));
            *k = (uint8_t)((c->gpi_k << 4) | (c->r[R_GPIODATA].k & 0x0F));
            return;
        case R_GLOBAL:
            /* 0x1F is valid only on the MAX3107 (RevID) and the global variants
               (VARIANTS: last_reg 0x1E on the MAX3108). */
            *v = var->global ? m31x_global_irq(s) : (uint8_t)var->revid;
            return;
        default:
            m = m31x_writable(var, addr);
            *v = m31x_rv(c, addr) & m;
            *k = (uint8_t)(c->r[addr].k | (uint8_t)~m);
            return;
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
                unsigned trig = m31x_trigger_rx(c);

                c->lsr = (uint8_t)((c->lsr & 0x03) | (w->flags & 0x3C));
                c->lsr_k = (uint8_t)((c->lsr_k & ~0x3C) | (w->flags_k & 0x3C));
                c->rx_head = (c->rx_head + 1) % MX_FIFO;
                c->rx_count--;
                if (c->rx_count == 0 && !m31x_bit(c, R_MODE2, M2_RXEMPTYINV)) {
                    c->isr |= I_RXEMPTY;
                }
                if (c->rx_count == trig && m31x_bit(c, R_MODE2, M2_RXTRIGINV)) {
                    c->isr |= I_RXTRIG;
                }
                m31x_flow_update(c);
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
        m31x_reset_chan(t);
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

            if ((ts & TS_SYNCHEN) && (ts & 15u) == (cmd & 15u) && m31x_bit(c, R_MODE1, M1_TXDIS) &&
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
    unsigned trig = m31x_trigger_tx(c);

    if (m31x_bit(c, R_MODE2, M2_FIFORST) || c->tx_count >= MX_FIFO) {
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
            m31x_schedule(s, TM_REFCLK, 0, s->refclk_gen, m31x_refclk_half(s));
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
                    m31x_update_reference(s);
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
                m31x_update_reference(s);
            }
            c->r[R_MODE2].v = data;
            c->r[R_MODE2].k = known;
            if (data & M2_FIFORST) {
                m31x_rx_clear(c);
                m31x_tx_clear(c);
            }
            m31x_update_format(c);
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
                    m31x_update_reference(s);
                    m31x_brg_restart(c);
                }
            }
            return;
        }
        default:
            break;
    }
    c->r[addr].v = data & m31x_writable(var, addr);
    c->r[addr].k = (uint8_t)(known | (uint8_t)~m31x_writable(var, addr));
    switch (addr) {
        case R_LCR:
        case R_BRGCFG:
            m31x_update_format(c);
            if (addr == R_BRGCFG) {
                m31x_brg_restart(c);
            }
            break;
        case R_DIVLSB:
        case R_DIVMSB:
            m31x_brg_restart(c);
            break;
        case R_PLLCFG:
        case R_CLKSRC:
            m31x_update_reference(s);
            break;
        case R_FLOWLVL:
        case R_FLOWCTRL:
            m31x_flow_update(c);
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
    m31x_update_outputs(s);
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

void m31x_spi_begin(void *context) {
    MxState *s = context;

    s->spi_rw = -1;
    s->spi_x = 1;
    s->spi_addr = -1;
}

SerialSpiByte m31x_spi_next(void *context, unsigned index) {
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

void m31x_spi_bit(void *context, unsigned index, unsigned bits, uint8_t data, uint8_t known,
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
        out->data = (uint8_t)(m31x_global_irq(s) & 0x0F);
        out->known = 0x0F;
    }
}

void m31x_spi_received(void *context, unsigned index, uint8_t data, uint8_t known) {
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
LogicValue m31x_i2c_address(void *context, uint8_t byte, uint8_t known) {
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

void m31x_i2c_begin(void *context, int repeated) {
    MxState *s = context;

    if (!repeated) {
        s->i2c_have_ptr = 0;
    }
}

static int i2c_next_ptr(int ptr) {
    return ptr == 0 ? 0 : ptr + 1;
}

LogicValue m31x_i2c_received(void *context, unsigned index, uint8_t data, uint8_t known) {
    MxState *s = context;
    Chan *c;

    /* Data bytes, reads and acks reach the model only after an address byte it
       ACKed (serial_target.c i2c_falling: an ack other than 1 ignores the rest),
       and m31x_i2c_address set i2c_ch for that ACK: i2c_ch >= 0 here and in
       m31x_i2c_next and m31x_i2c_acked. */
    (void)index;
    c = &s->ch[s->i2c_ch];
    if (!s->i2c_have_ptr) {
        s->i2c_have_ptr = 1;
        if (known != 0xFF) {
            s->i2c_ptr = -1;
            return LOGIC_X;
        }
        s->i2c_ptr = reg_valid(s, data) ? data : -1;
        return m31x_bool3(s->i2c_ptr >= 0);   /* ACK only for a valid register */
    }
    if (s->i2c_ptr < 0 || !reg_valid(s, (unsigned)s->i2c_ptr)) {
        return LOGIC_0;
    }
    reg_write(s, c, (unsigned)s->i2c_ptr, data, known);
    s->i2c_ptr = i2c_next_ptr(s->i2c_ptr);
    after_access(s);
    return LOGIC_1;
}

void m31x_i2c_next(void *context, unsigned index, uint8_t *data, uint8_t *known) {
    MxState *s = context;

    (void)index;
    *data = 0;
    *known = 0;
    if (s->i2c_ptr < 0 || !reg_valid(s, (unsigned)s->i2c_ptr)) {
        return;
    }
    reg_peek(s, &s->ch[s->i2c_ch], (unsigned)s->i2c_ptr, data, known);
}

void m31x_i2c_acked(void *context, unsigned index, LogicValue ack) {
    MxState *s = context;

    (void)index;
    (void)ack;
    if (s->i2c_ptr < 0 || !reg_valid(s, (unsigned)s->i2c_ptr)) {
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
    s->xin = m31x_level(s, s->pins->clock_in, LOGIC_X);
    for (u = 0; u < s->dev->v->uarts; u++) {
        s->ch[u].cts = m31x_level(s, s->pins->channel[u][CH_CTS], LOGIC_X);
    }
    serial_host_start(&s->host, sim, instance);
    s->in_reset = 1;   /* power-on: the reset state, then the pin applies */
    m31x_apply_reset_pin(s);
    s->gpio_dirty = 1;
    m31x_update_outputs(s);
}

static void wake(Chan *c) {
    MxState *s = c->owner;

    c->sleep_idle = 0;
    if (c->sleeping && !m31x_bit(c, R_MODE1, M1_FORCEDSLEEP)) {
        c->sleeping = 0;
        m31x_update_reference(s);
        m31x_brg_restart(c);
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
        m31x_apply_reset_pin(s);
        return;
    }
    if (pin == s->pins->clock_in) {
        m31x_xin_edge(s, m31x_level(s, pin, LOGIC_X));
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
            LogicValue v = m31x_level(s, pin, LOGIC_X);

            if (v != c->cts) {
                /* CTSInt: a CTS transition (0 <-> 1; an X is no edge). */
                if (!m31x_bit(c, R_MODE2, M2_LOOP) && (v == LOGIC_0 || v == LOGIC_1) &&
                    (c->cts == LOGIC_0 || c->cts == LOGIC_1)) {
                    c->isr |= I_CTS;
                }
                c->cts = v;
                wake(c);
                m31x_update_outputs(s);
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
            m31x_ready_now(s);
        }
        return;
    }
    if (type == TM_REFCLK) {
        if (gen != (s->refclk_gen & 0xFFFFFFu) || s->ref_mode != 2 || s->period == 0 ||
            !m31x_refclk_wanted(s) || sim_now(sim) - s->last_rise > 2 * s->period) {
            return;
        }
        s->refclk = s->refclk == LOGIC_1 ? LOGIC_0 : LOGIC_1;
        m31x_update_gpio(s);
        m31x_schedule(s, TM_REFCLK, 0, s->refclk_gen, m31x_refclk_half(s));
        return;
    }
    /* TM_HALF and TM_BRG carry c->index (m31x_brg_schedule, below): always a UART. */
    if (type == TM_HALF) {
        Chan *c = &s->ch[ch];

        if (gen == (c->half_gen & 0xFFFFFFu)) {
            c->clkout = LOGIC_0;
            m31x_update_outputs(s);
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
        m31x_chan_tick(c);
        if (!c->run || c->brg_pending) {
            return;
        }
        /* The XIN clock stopped: so does the PLL (it starts again on XIN). */
        if (s->period == 0 || now - s->last_rise > 2 * s->period) {
            return;
        }
        m31x_brg_schedule(c, m31x_tick_delay(c));
        if (m31x_clk_to_rts(c)) {
            c->half_gen++;
            m31x_schedule(s, TM_HALF, ch, c->half_gen, c->brg_delay / 2);
        }
    }
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
    for (i = 0; i < sizeof m31x_VARIANTS / sizeof m31x_VARIANTS[0]; i++) {
        if (strcmp(m31x_VARIANTS[i].name, variant) == 0) {
            d->v = &m31x_VARIANTS[i];
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
    if (serial_host_load(ctx, bus, m31x_BUS_EXTRA, &d->host) != 0) {
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
        if (kind_check_keys(ctx, item, m31x_CHANNEL_KEYS, NULL) != 0) {
            return NULL;
        }
        for (k = 0; k < CH_PIN_COUNT; k++) {
            if (kind_string(ctx, item, m31x_CHANNEL_KEYS[k], &d->channel[i][k]) != 0) {
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
            snprintf(key, sizeof key, "channels[%u].%s", i, m31x_CHANNEL_KEYS[k]);
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
    "max310x", m31x_KEYS, m31x_TIMING, NULL,
    mx_load_device, mx_bind, NULL, m31x_mx_open_drain,
    m31x_mx_create, m31x_mx_destroy, mx_start, mx_pin_changed, mx_timer,
    m31x_mx_channels, m31x_mx_line, m31x_mx_async
};
