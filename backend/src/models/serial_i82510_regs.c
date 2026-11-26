/* models/serial_i82510_regs.c: part of the model in models/serial_i82510.c (split by plan-68-I/J). */
#include "serial_i82510_int.h"
#include "logic_alloc.h"
/* ---- Registers -------------------------------------------------------------------------- */

static int dlab(const I5State *s) {
    return (i82_rv(s, R_LCR) >> 7) & 1u;
}

static void reset_regs(I5State *s) {
    unsigned i;

    for (i = 0; i < R_COUNT; i++) {
        s->r[i] = su_reg(i82_RESET_VALUE[i], 0xFF);
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
    i82_brg_load(s, 0);
    i82_brg_load(s, 1);
    i82_update_format(s);
}

static void reg_peek(I5State *s, unsigned a, uint8_t *v, uint8_t *k) {
    unsigned b = s->bank;

    *k = 0xFF;
    *v = 0;
    if (a == 2) {
        *v = i82_gir_of(s);
        return;
    }
    switch (b) {
        case 0:
            switch (a) {
                case 0:
                    if (dlab(s)) { *v = i82_rv(s, R_BAL); *k = s->r[R_BAL].k; }
                    else if (s->rx_count > 0) { *v = (uint8_t)s->rx[0].data; *k = (uint8_t)s->rx[0].known; }
                    else { *k = 0; }
                    return;
                case 1:
                    if (dlab(s)) { *v = i82_rv(s, R_BAH); *k = s->r[R_BAH].k; }
                    else { *v = i82_rv(s, R_GER) & 0x3Fu; *k = s->r[R_GER].k; }
                    return;
                case 3: *v = i82_rv(s, R_LCR); *k = s->r[R_LCR].k; return;
                case 4: *v = i82_rv(s, R_MCR) & 0x3Fu; *k = s->r[R_MCR].k; return;
                case 5: *v = i82_lsr_of(s); return;
                case 6: *v = i82_msr_of(s, k); return;
                default: *v = i82_rv(s, R_ACR0); *k = s->r[R_ACR0].k; return;
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
                case 5: *v = (uint8_t)((s->rst & 0xFEu) | (i82_rfir(s) ? 1 : 0)); return;
                case 6: *v = i82_msr_of(s, k); return;
                default: *v = i82_gsr_of(s); return;
            }
        case 2:
            switch (a) {
                case 1: *v = i82_rv(s, R_FMD); *k = s->r[R_FMD].k; return;
                case 3: *v = i82_rv(s, R_TMD); *k = s->r[R_TMD].k; return;
                case 4: *v = i82_rv(s, R_IMD) & 0x0Fu; *k = s->r[R_IMD].k; return;
                case 5: *v = i82_rv(s, R_ACR1); *k = s->r[R_ACR1].k; return;
                case 6: *v = i82_rv(s, R_RIE); *k = s->r[R_RIE].k; return;
                case 7: *v = i82_rv(s, R_RMD); *k = s->r[R_RMD].k; return;
                default: *k = 0; return;   /* bank 2 A0: not in Table 9 */
            }
        default:
            switch (a) {
                case 0:
                    if (dlab(s)) { *v = i82_rv(s, R_BBL); *k = s->r[R_BBL].k; }
                    else { *v = i82_rv(s, R_CLCF); *k = s->r[R_CLCF].k; }
                    return;
                case 1:
                    if (dlab(s)) { *v = i82_rv(s, R_BBH); *k = s->r[R_BBH].k; }
                    else { *v = i82_rv(s, R_BACF); *k = s->r[R_BACF].k; }
                    return;
                case 3: *v = i82_rv(s, R_BBCF); *k = s->r[R_BBCF].k; return;
                case 4: *v = i82_rv(s, R_PMD); *k = s->r[R_PMD].k; return;
                case 5: *v = i82_rv(s, R_MIE); *k = s->r[R_MIE].k; return;
                case 6: *v = i82_rv(s, R_TMIE); *k = s->r[R_TMIE].k; return;
                default: *k = 0; return;   /* bank 3 A7: not in Table 9 */
            }
    }
}

static void rx_pop(I5State *s) {
    if (s->rx_count > 0) {
        memmove(&s->rx[0], &s->rx[1], (s->rx_count - 1) * sizeof s->rx[0]);
        s->rx_count--;
        i82_serviced(s);
    }
}

static void read_done(I5State *s, unsigned a) {
    unsigned b = s->bank;

    if (a == 2) {
        /* A GIR read reporting the TX FIFO enters the hold state (p29). */
        if (i82_gir_block(s) == 1) {
            s->tx_hold_state = 1;
            i82_serviced(s);
        }
        return;
    }
    if ((b == 0 && a == 0 && !dlab(s)) || (b == 1 && a == 0)) {
        rx_pop(s);
    } else if ((b == 0 || b == 1) && a == 5) {
        /* LSR / RST read: RST 7:1 and LSR 4:1 clear. */
        s->rst = 0;
        i82_serviced(s);
    } else if ((b == 0 || b == 1) && a == 6) {
        s->msr_delta = 0;
        i82_serviced(s);
    } else if (b == 1 && a == 3) {
        s->tmst &= (uint8_t)~3u;
        i82_serviced(s);
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
        s->tx[s->tx_count].data = (uint16_t)(v | ((i82_rv(s, R_TXF) & 0x20u) ? 0x100u : 0));
        s->tx[s->tx_count].known = (uint16_t)(k | 0x100u);
        s->tx[s->tx_count].txf = i82_rv(s, R_TXF);
        s->tx_count++;
    }   /* a fifth character is dropped, no error (p9) */
    s->tx_hold_state = 0;
    i82_serviced(s);
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
                    if (dlab(s)) { s->r[R_BAL] = su_reg(v, k); i82_brg_load(s, 0); }
                    else { push_txd(s, v, k); }
                    return;
                case 1:
                    if (dlab(s)) { s->r[R_BAH] = su_reg(v, k); i82_brg_load(s, 0); }
                    else { s->r[R_GER] = su_reg(v & 0x3Fu, k); s->tx_hold_state &= (v >> 1) & 1u; }
                    return;
                case 3: s->r[R_LCR] = su_reg(v, k); i82_update_format(s); return;
                case 4: s->r[R_MCR] = su_reg(v & 0x3Fu, k); return;
                case 5:
                    /* Writing 0 clears LSR bits (bit 0: the RX FIFO interrupt). */
                    s->rst &= (uint8_t)(v | 0xE1u);
                    if (!(v & 1u)) s->rfir_ack = 1;
                    i82_serviced(s);
                    return;
                case 6:
                    s->msr_delta &= v;
                    i82_serviced(s);
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
                    if (v & 1u) { s->brg[0].timer_run = !i82_brg_mode(s, 0); s->brg[0].timer_count = i82_divisor_of(s, 0); }
                    if (v & 2u) { s->brg[1].timer_run = !i82_brg_mode(s, 1); s->brg[1].timer_count = i82_divisor_of(s, 1); }
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
                case 3: s->r[R_TMD] = su_reg(v, k); i82_update_format(s); return;
                case 4: s->r[R_IMD] = su_reg(v & 0x0Fu, k); i82_update_format(s); return;
                case 5: s->r[R_ACR1] = su_reg(v, k); return;
                case 6: s->r[R_RIE] = su_reg(v & 0xFEu, k); return;
                case 7: s->r[R_RMD] = su_reg(v & 0xF8u, k); return;
                default: return;
            }
        default:
            switch (a) {
                case 0:
                    if (dlab(s)) { s->r[R_BBL] = su_reg(v, k); i82_brg_load(s, 1); }
                    else { s->r[R_CLCF] = su_reg(v & 0xF0u, k); i82_update_format(s); }
                    return;
                case 1:
                    if (dlab(s)) { s->r[R_BBH] = su_reg(v, k); i82_brg_load(s, 1); }
                    else { s->r[R_BACF] = su_reg(v & 0x44u, k); i82_brg_load(s, 0); }
                    return;
                case 3: s->r[R_BBCF] = su_reg(v & 0xC4u, k); i82_brg_load(s, 1); return;
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
        i82_update_format(s);
    }
    read_done(s, address & 7u);
    i82_track_modem(s);
    i82_update_outputs(s);
}

static void bus_write(void *context, unsigned unit, unsigned address, uint8_t data, uint8_t known) {
    I5State *s = context;

    (void)unit;
    if (s->unknown || (s->pd_sleep && !(s->bank == 1 && (address & 7u) == 7u))) {
        return;
    }
    write_reg(s, address & 7u, data, known);
    i82_refresh(s);
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
            i82_update_format(s);
        }
        s->unknown = 0;
        s->in_reset = 0;
        s->bus.enabled = 1;
    }
    serial_bus_update(&s->bus, s->sim, s->instance, &s->events);
    i82_refresh(s);
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
            i82_sysclk_edge(s, s->sysclk == LOGIC_1 ? LOGIC_0 : LOGIC_1);
        }
    } else if (rising || falling) {
        i82_sysclk_edge(s, level);
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
    i82_track_modem(s);
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
        i82_sclk_edge(s, serial_level(sim, instance, pin, LOGIC_X));
    }
    if (serial_bus_has_pin(&s->bus, pin)) {
        serial_bus_update(&s->bus, sim, instance, &s->events);
    }
    if (pin == p[P_CTS] || pin == p[P_DSR] || pin == p[P_DCD] || pin == p[P_RI] || pin == p[P_RX]) {
        i82_refresh(s);
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
    I5State *s = logic_calloc(1, sizeof *s);
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
    s->o.out = logic_malloc(part->pin_count * sizeof *s->o.out);
    if (s->o.out == NULL) {
        logic_free(s);
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
    logic_free(s->o.out);
    logic_free(s);
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
    if (kind_check_keys(ctx, bus, i82_BUS_KEYS, NULL) != 0 ||
        su_load_names(ctx, bus, "data", 1, d->data, 8, 8, NULL) != 0 ||
        su_load_names(ctx, bus, "address", 1, d->addr, 3, 3, NULL) != 0 ||
        kind_string(ctx, bus, "read", &d->read) != 0 ||
        kind_string(ctx, bus, "write", &d->write) != 0 ||
        kind_string(ctx, bus, "select", &d->select) != 0 ||
        kind_string(ctx, bus, "reset", &d->reset) != 0) {
        return NULL;
    }
    kind_pop(ctx);
    if (su_load_pin_object(ctx, json, "clock", 1, i82_CLOCK_KEYS, d->clock, 0x3u) != 0 ||
        su_load_pin_object(ctx, json, "serial", 1, i82_SERIAL_KEYS, d->pin, 0x1FFu) != 0) {
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
        su_bind_object(ctx, pins, pin_count, "clock", i82_CLOCK_KEYS, d->clock, p->clock) != 0 ||
        su_bind_object(ctx, pins, pin_count, "serial", i82_SERIAL_KEYS, d->pin, p->pin) != 0) {
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
    "i82510", i82_KEYS, i82_TIMING, NULL,
    i5_load_device, i5_bind, NULL, NULL,
    i5_create, i5_destroy, i5_start, i5_pin_changed, NULL,
    i5_channels, i5_line, i5_async
};
