/* models/ns16550_bridge_kind.c: part of the model in models/ns16550_bridge.c (split by plan-68-I/J). */
#include "ns16550_bridge_int.h"
#include "logic_alloc.h"
/* ---- Loader ------------------------------------------------------------------------ */

static int load_optional_name(KindContext *ctx, const cJSON *json, const char *key,
                              const char **out) {
    return serial_optional_name(ctx, json, key, out);
}

int ns_bridge_load(KindContext *ctx, const cJSON *json, const cJSON *bus, NsDevice *d) {
    static const char *const BUS_EXTRA[] = {"reset", "resetActiveHigh", "registerBits", "nack",
                                            "sdaTimeout", NULL};
    NsBridgeDevice *b = kind_alloc(ctx, sizeof *b);
    const cJSON *item;
    uint64_t value;

    if (b == NULL) {
        return -1;
    }
    kind_push(ctx, "bus");
    if (serial_host_load(ctx, bus, BUS_EXTRA, &b->host) != 0) {
        return -1;
    }
    if (b->host.style != SERIAL_HOST_I2C && b->host.mode != 0) {
        return kind_error(ctx, "the bridges' SPI port is mode 0");
    }
    if (serial_optional_name(ctx, bus, "reset", &d->reset) != 0) {
        return -1;
    }
    d->reset_active_high = 1;
    if (cJSON_GetObjectItemCaseSensitive(bus, "resetActiveHigh") != NULL &&
        kind_bool(ctx, bus, "resetActiveHigh", &d->reset_active_high) != 0) {
        return -1;
    }
    b->reg_bits = 4;
    if (cJSON_GetObjectItemCaseSensitive(bus, "registerBits") != NULL) {
        if (kind_uint(ctx, bus, "registerBits", 4, &value) != 0) {
            return -1;
        }
        if (value != 3 && value != 4) {
            return kind_error(ctx, "\"registerBits\" is 3 or 4");
        }
        b->reg_bits = (unsigned)value;
    }
    item = cJSON_GetObjectItemCaseSensitive(bus, "nack");
    if (item != NULL) {
        const char *const *names;
        size_t n;
        size_t i;

        if (kind_names(ctx, bus, "nack", &names, &n) != 0) {
            return -1;
        }
        for (i = 0; i < n; i++) {
            if (strcmp(names[i], "full") == 0) {
                b->nack_full = 1;
            } else if (strcmp(names[i], "reset") == 0) {
                b->nack_reset = 1;
            } else {
                return kind_error(ctx, "\"nack\" entries are \"full\" and \"reset\"");
            }
        }
    }
    item = cJSON_GetObjectItemCaseSensitive(bus, "sdaTimeout");
    if (item != NULL && kind_ns(ctx, item, "sdaTimeout", &b->sda_timeout) != 0) {
        return -1;
    }
    kind_pop(ctx);

    if (kind_string(ctx, json, "interrupt", &b->irq) != 0) {
        return -1;
    }
    b->state_reset = -1;
    item = cJSON_GetObjectItemCaseSensitive(json, "gpio");
    if (item != NULL) {
        static const char *const GPIO_KEYS[] = {"pins", "registers", "pullUp", "stateReset", NULL};
        const char *const *names;
        const char *style;
        size_t n;
        size_t i;

        kind_push(ctx, "gpio");
        if (kind_check_keys(ctx, item, GPIO_KEYS, NULL) != 0 ||
            kind_names(ctx, item, "pins", &names, &n) != 0 ||
            kind_string(ctx, item, "registers", &style) != 0) {
            return -1;
        }
        if (n < 1 || n > NB_GPIO) {
            return kind_error(ctx, "\"pins\" has 1 to 8 names (GPIO0 first)");
        }
        for (i = 0; i < n; i++) {
            b->gpio[i] = names[i];
        }
        b->gpio_count = (unsigned)n;
        if (strcmp(style, "sc16is") == 0) {
            b->gpio_style = NB_GPIO_SC16IS;
        } else if (strcmp(style, "xr1280") == 0) {
            b->gpio_style = NB_GPIO_XR1280;
        } else {
            return kind_error(ctx, "\"registers\" is \"sc16is\" or \"xr1280\"");
        }
        if (cJSON_GetObjectItemCaseSensitive(item, "pullUp") != NULL &&
            kind_bool(ctx, item, "pullUp", &b->gpio_pullup) != 0) {
            return -1;
        }
        if (cJSON_GetObjectItemCaseSensitive(item, "stateReset") != NULL) {
            if (kind_uint(ctx, item, "stateReset", 255, &value) != 0) {
                return -1;
            }
            b->state_reset = (int)value;
        }
        kind_pop(ctx);
    }
    item = cJSON_GetObjectItemCaseSensitive(json, "fastIrda");
    if (item != NULL) {
        if (cJSON_IsBool(item)) {
            b->fast_irda_all = cJSON_IsTrue(item);
        } else if (kind_names(ctx, json, "fastIrda", &b->fast_irda_grades, &b->fast_irda_count) != 0) {
            return -1;
        }
    }
    b->dl_power[0] = b->dl_power[1] = -1;
    item = cJSON_GetObjectItemCaseSensitive(json, "dlPowerUp");
    if (item != NULL) {
        unsigned i;

        if (!cJSON_IsArray(item) || cJSON_GetArraySize(item) != 2) {
            return kind_error(ctx, "\"dlPowerUp\" is [DLL, DLM], a byte or null each");
        }
        for (i = 0; i < 2; i++) {
            const cJSON *v = cJSON_GetArrayItem(item, (int)i);

            if (cJSON_IsNull(v)) {
                continue;
            }
            if (!cJSON_IsNumber(v) || v->valuedouble < 0 || v->valuedouble > 255 ||
                v->valuedouble != (double)(int)v->valuedouble) {
                return kind_error(ctx, "\"dlPowerUp\" is [DLL, DLM], a byte or null each");
            }
            b->dl_power[i] = (int)v->valuedouble;
        }
    }
    item = cJSON_GetObjectItemCaseSensitive(json, "straps");
    if (item != NULL) {
        static const char *const STRAP_KEYS[] = {"en485", "enir", NULL};

        kind_push(ctx, "straps");
        if (kind_check_keys(ctx, item, STRAP_KEYS, NULL) != 0 ||
            load_optional_name(ctx, item, "en485", &b->en485) != 0 ||
            load_optional_name(ctx, item, "enir", &b->enir) != 0) {
            return -1;
        }
        kind_pop(ctx);
    }
    if (load_optional_name(ctx, json, "sleepPin", &b->sleep_pin) != 0) {
        return -1;
    }
    if (d->extended < EXT_SC16IS || d->extended > EXT_XR1280) {
        return kind_error(ctx, "an I2C/SPI bridge needs \"extended\": \"sc16is\", \"sc16is-sfr\", "
                               "\"sc16is-dld\" or \"xr1280\"");
    }
    if ((d->extended == EXT_XR1280) != (b->reg_bits == 3)) {
        return kind_error(ctx, "\"xr1280\" goes with \"registerBits\": 3");
    }
    if ((b->gpio_style == NB_GPIO_XR1280) != (d->extended == EXT_XR1280 && b->gpio_count > 0)) {
        return kind_error(ctx, "\"gpio\" \"registers\" must match \"extended\"");
    }
    if ((b->en485 != NULL || b->enir != NULL || b->sleep_pin != NULL) && d->extended != EXT_XR1280) {
        return kind_error(ctx, "\"straps\" and \"sleepPin\" are XR20M1280 keys");
    }
    if (d->channel_count > NB_CHANNELS) {
        return kind_error(ctx, "a bridge has 1 or 2 channels");
    }
    d->bridge = b;
    return 0;
}

int ns_bridge_bind(KindContext *ctx, const NsDevice *d, const PartPin *pins, size_t pin_count,
                   NsPins *p) {
    const NsBridgeDevice *b = d->bridge;
    NsBridgePins *bp = kind_alloc(ctx, sizeof *bp);
    char key[32];
    unsigned i;
    unsigned ch;

    if (bp == NULL || serial_host_bind(ctx, &b->host, pins, pin_count, &bp->host) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "interrupt", b->irq, &bp->irq) != 0 ||
        serial_bind_pin_absent(ctx, pins, pin_count, "straps.en485", b->en485, &d->absent,
                               &bp->en485) != 0 ||
        serial_bind_pin_absent(ctx, pins, pin_count, "straps.enir", b->enir, &d->absent,
                               &bp->enir) != 0 ||
        serial_bind_pin_absent(ctx, pins, pin_count, "sleepPin", b->sleep_pin, &d->absent,
                               &bp->sleep_pin) != 0) {
        return -1;
    }
    for (i = 0; i < NB_GPIO; i++) {
        bp->gpio[i] = SERIAL_NO_PIN;
        bp->gpio_ch[i] = -1;
        bp->gpio_role[i] = -1;
    }
    for (i = 0; i < b->gpio_count; i++) {
        snprintf(key, sizeof key, "gpio.pins[%u]", i);
        if (serial_bind_pin_absent(ctx, pins, pin_count, key, b->gpio[i], &d->absent,
                                   &bp->gpio[i]) != 0) {
            return -1;
        }
        if (!SERIAL_REAL_PIN(bp->gpio[i])) {
            bp->gpio[i] = SERIAL_NO_PIN;
            continue;
        }
        for (ch = 0; ch < d->channel_count; ch++) {
            static const ChannelPin ROLES[4] = {CH_DSR, CH_DTR, CH_DCD, CH_RI};
            unsigned r;

            for (r = 0; r < 4; r++) {
                if (p->channel[ch][ROLES[r]] == bp->gpio[i]) {
                    bp->gpio_ch[i] = (int)ch;
                    bp->gpio_role[i] = (int)ROLES[r];
                }
            }
        }
    }
    p->bridge = bp;
    return 0;
}

/* Called by ns_open_drain for a device with a bridge, with a pin of the part. */
int ns_bridge_open_drain(const NsDevice *d, const NsPins *p, size_t pin) {
    return pin == p->bridge->irq || serial_host_open_drain(&d->bridge->host, &p->bridge->host, pin);
}

/* ---- Create / start / reset / pins -------------------------------------------------------- */

int ns_bridge_create(NsState *s) {
    const NsBridgeDevice *b = s->dev->bridge;
    NsBridge *br = logic_calloc(1, sizeof *br);
    unsigned i;
    size_t g;

    if (br == NULL) {
        return -1;
    }
    s->bridge = br;
    serial_host_init(&br->host, &b->host, &s->pins->bridge->host);
    br->host.spi.t_out = t_of(s, T_DO);
    br->host.spi.t_float = t_of(s, T_TR);
    br->host.i2c.t_out = t_of(s, T_VD);
    br->spi.context = s;
    br->spi.begin = brs_spi_begin;
    br->spi.next = brs_spi_next;
    br->spi.received = brs_spi_received;
    br->i2c.context = s;
    br->i2c.begin = brs_i2c_begin;
    br->i2c.address = brs_i2c_address;
    br->i2c.received = brs_i2c_received;
    br->i2c.next = brs_i2c_next;
    br->i2c.acked = brs_i2c_acked;
    br->spi_rw = -1;
    br->sda_level = LOGIC_X;
    br->fast_irda = b->fast_irda_all;
    for (g = 0; g < b->fast_irda_count; g++) {
        if (strcmp(b->fast_irda_grades[g], s->grade->info.grade) == 0) {
            br->fast_irda = 1;
        }
    }
    brs_reset_registers(s, 1);
    for (i = 0; i < s->dev->channel_count; i++) {
        NsChannel *c = &s->ch[i];

        if (b->dl_power[0] >= 0) {
            c->dll = (Reg){(uint8_t)b->dl_power[0], 0xFF};
        }
        if (b->dl_power[1] >= 0) {
            c->dlm = (Reg){(uint8_t)b->dl_power[1], 0xFF};
        }
        if (brs_is_xr(s->dev)) {
            br->ch[i].dll_tx = br->ch[i].dll_rx = c->dll;
            br->ch[i].dlm_tx = br->ch[i].dlm_rx = c->dlm;
        }
        ns_ext_load_divisor(c);
        ns_update_format(c);
    }
    return 0;
}

void ns_bridge_destroy(NsState *s) {
    logic_free(s->bridge);
    s->bridge = NULL;
}

void ns_bridge_start(NsState *s) {
    NsBridge *br = s->bridge;

    serial_host_start(&br->host, s->sim, s->instance);
    br->sda_level = serial_level(s->sim, s->instance, s->pins->bridge->host.i2c.sda, LOGIC_X);
    brs_gpio_levels(s, &br->ref, &br->ref_k);
    if (brs_is_xr(s->dev)) {
        xr_straps(s);
        xr_gpio_changed(s);
    }
}

void ns_bridge_reset(NsState *s, LogicValue active) {
    NsBridge *br = s->bridge;
    int held = br->host.held;

    if (active == LOGIC_1) {
        brs_reset_registers(s, 0);
    }
    serial_host_hold(&br->host, s->sim, s->instance, active != LOGIC_0);
    if (held && active == LOGIC_0 && brs_is_xr(s->dev)) {
        xr_straps(s);   /* EN485#, ENIR# sampled as reset ends */
    }
}

void ns_bridge_timer(NsState *s, uint32_t id) {
    NsBridge *br = s->bridge;

    /* "resets the serial bus interface if SDA is held LOW for a minimum of
       25 ms" (SC16IS750 p42 note 2). */
    if (id == br->sda_gen && br->sda_level == LOGIC_0 && br->host.mode == SERIAL_HOST_ON_I2C) {
        serial_i2c_enable(&br->host.i2c, s->sim, s->instance, 0);
        serial_i2c_enable(&br->host.i2c, s->sim, s->instance, 1);
    }
}

void ns_bridge_pin(NsState *s, size_t pin) {
    NsBridge *br = s->bridge;
    const NsBridgePins *bp = s->pins->bridge;
    unsigned b;

    if (serial_host_has_pin(&s->dev->bridge->host, &bp->host, pin)) {
        serial_host_pin(&br->host, s->sim, s->instance, pin, &br->spi, &br->i2c);
        if (pin == bp->host.i2c.sda) {
            LogicValue sda = serial_level(s->sim, s->instance, pin, LOGIC_X);

            if (sda != br->sda_level) {
                br->sda_level = sda;
                br->sda_gen++;
                if (sda == LOGIC_0 && s->dev->bridge->sda_timeout != 0) {
                    sim_schedule_timer(s->sim, s->instance, br->sda_gen,
                                       s->dev->bridge->sda_timeout);
                }
            }
        }
    }
    if (s->in_reset || s->unknown) {
        return;
    }
    if (brs_is_xr(s->dev)) {
        if (pin == bp->sleep_pin) {
            xr_sleep_pin(s);
        }
        for (b = 0; b < NB_GPIO; b++) {
            if (pin == bp->gpio[b]) {
                xr_gpio_changed(s);
                ns_bridge_outputs(s, CAUSE_MODEM_IN);
                break;
            }
        }
        return;
    }
    for (b = 0; b < NB_GPIO; b++) {
        if (pin == bp->gpio[b]) {
            brs_sc_gpio_changed(s, b);
            ns_bridge_outputs(s, CAUSE_MODEM_IN);
        }
    }
}

int ns_bridge_owns(const NsState *s, size_t pin) {
    const NsBridgePins *bp = s->pins->bridge;
    unsigned b;

    for (b = 0; b < NB_GPIO; b++) {
        if (bp->gpio[b] == pin) {
            return 1;
        }
    }
    return 0;
}

int ns_bridge_modem_input(const NsState *s, unsigned ch, size_t pin, LogicValue *z_reads) {
    const NsBridgePins *bp = s->pins->bridge;
    unsigned b;

    (void)ch;
    for (b = 0; b < NB_GPIO; b++) {
        if (bp->gpio[b] == pin) {
            if (s->dev->bridge->gpio_pullup) {
                *z_reads = LOGIC_1;   /* the GPIO pins' active pull-up */
            }
            return brs_gpio_is_modem(s, b) == LOGIC_1;
        }
    }
    return 1;
}

/* The IRQ level of every source: 1 pending, 0 none, X. */
static LogicValue irq_pending(const NsState *s) {
    LogicValue any = LOGIC_0;
    unsigned i;

    for (i = 0; i < s->dev->channel_count; i++) {
        uint8_t v;
        uint8_t k;

        ns_iir_now(&s->ch[i], &v, &k);
        any = or3(any, (k & 1u) ? bool3(!(v & 1u)) : LOGIC_X);
    }
    if (brs_is_xr(s->dev)) {
        any = or3(any, xr_irq_extra(s));
    }
    return any;
}

/* SC16IS GPIO pins: outputs per IODir and IOState; modem pins per IOControl. */
static void sc_gpio_outputs(NsState *s) {
    NsBridge *br = s->bridge;
    const NsBridgePins *bp = s->pins->bridge;
    SimTime delay = t_of(s, T_GPIO);
    unsigned b;

    for (b = 0; b < NB_GPIO; b++) {
        size_t pin = bp->gpio[b];
        LogicValue modem;
        LogicValue level;

        if (pin == SERIAL_NO_PIN) {
            continue;
        }
        if (s->unknown) {
            nb_drive(s, pin, LOGIC_X, 0);
            continue;
        }
        if (s->in_reset) {
            nb_drive(s, pin, LOGIC_Z, 0);   /* GPIOs are inputs after reset */
            continue;
        }
        modem = brs_gpio_is_modem(s, b);
        if (modem == LOGIC_1 || modem == LOGIC_X) {
            level = LOGIC_Z;
            if (bp->gpio_role[b] == CH_DTR) {
                const NsChannel *c = &s->ch[bp->gpio_ch[b]];

                level = not3(bit_of(c->mcr, 0));
                if (s->dev->extended == EXT_SC16IS_SFR && nb_known1(nb_ch(c)->sfr, 1)) {
                    level = bool3(c->rts_off);   /* PI7C9X auto DTR follows the RTS rule */
                }
                level = or3(level, bit_of(c->mcr, 4));   /* loopback: inactive */
            }
            if (modem == LOGIC_X && level != LOGIC_Z) {
                level = LOGIC_X;
            }
        } else {
            LogicValue dir = bit_of(br->iodir, b);

            level = dir == LOGIC_1 ? bit_of(br->latch, b) : dir == LOGIC_0 ? LOGIC_Z : LOGIC_X;
        }
        nb_drive(s, pin, level, delay);
    }
}

void ns_bridge_outputs(NsState *s, Cause cause) {
    const NsBridgePins *bp = s->pins->bridge;
    LogicValue irq;

    if (s->unknown) {
        irq = LOGIC_X;
    } else if (s->in_reset) {
        irq = LOGIC_Z;
    } else {
        LogicValue p = irq_pending(s);

        irq = p == LOGIC_1 ? LOGIC_0 : p == LOGIC_0 ? LOGIC_Z : LOGIC_X;
    }
    nb_drive(s, bp->irq, irq, nb_cause_delay(s, cause));
    if (brs_is_xr(s->dev)) {
        xr_gpio_outputs(s);
    } else {
        sc_gpio_outputs(s);
    }
}

/* ---- Hooks into the core and ext ------------------------------------------------------------ */

int ns_bridge_rx_char(NsState *s, NsChannel *c, const SerialRxChar *rc) {
    NbChannel *n;
    int address;
    int rx_off;

    if (s->bridge == NULL) {
        return -1;
    }
    if (brs_is_xr(s->dev)) {
        return xr_rx_char(s, c, rc);
    }
    n = nb_ch(c);
    rx_off = nb_known1(n->efcr, 1);
    if (!nb_known1(n->efcr, 0)) {
        return rx_off ? 0 : -1;   /* RXDISABLE: nothing is received */
    }
    /* 9-bit (multidrop) mode: the parity bit is the address flag; with
       LCR[5:3] = 111 an address byte shows as a parity error. */
    address = (rc->flags_known & SERIAL_RX_PE) && (rc->flags & SERIAL_RX_PE);
    if (nb_known1(c->efr, 5)) {
        /* Auto address: XOFF2 is this station's address. */
        if (address) {
            uint8_t mask = (uint8_t)((1u << (5 + (c->lcr.v & 3u))) - 1u);

            n->aa_on = (rc->known & mask) == mask && (rc->data & mask) == (c->xoff2.v & mask);
            return n->aa_on;
        }
        return n->aa_on ? 1 : 0;
    }
    if (address) {
        return 1;   /* addresses always reach the FIFO */
    }
    return rx_off ? 0 : -1;
}

int ns_bridge_tx_blocked(NsState *s, NsChannel *c) {
    NbChannel *n;

    if (s->bridge == NULL) {
        return -1;
    }
    if (brs_is_xr(s->dev)) {
        return xr_tx_blocked(s, c);
    }
    n = nb_ch(c);
    if (nb_known1(n->efcr, 2)) {
        return 1;   /* TXDISABLE: the FIFO fills, nothing starts */
    }
    if (s->dev->extended == EXT_SC16IS_SFR) {
        if (nb_known1(n->sfr, 0)) {
            return 0;   /* SFR[0]: transmit regardless of flow control */
        }
        if (nb_known1(n->sfr, 1) && c->msr[M_DSR] != LOGIC_1) {
            return 1;   /* auto DSR */
        }
    }
    return -1;
}

int ns_bridge_levels(NsState *s, NsChannel *c) {
    if (s->bridge == NULL || !brs_is_xr(s->dev)) {
        return 0;
    }
    return xr_levels(s, c);
}

LogicValue ns_bridge_rts(const NsChannel *c, LogicValue normal) {
    const NbChannel *n;
    LogicValue on;
    int active;

    if (brs_is_xr(c->owner->dev)) {
        return xr_rts(c, normal);
    }
    n = nb_ch(c);
    on = bit_of(n->efcr, 4);
    if (on == LOGIC_0) {
        return normal;
    }
    /* RTSCON: RTS low from the THR write until the last bit is sent. */
    active = c->tx_count > 0 || serial_tx_busy(&c->line.tx) || c->flow_out_count > 0;
    if (on == LOGIC_X) {
        return LOGIC_X;
    }
    if (bit_of(n->efcr, 5) == LOGIC_X) {
        return LOGIC_X;
    }
    return bool3(active == nb_known1(n->efcr, 5));   /* RTSINVER: high while sending */
}

LogicValue ns_bridge_tx_pin(const NsChannel *c, LogicValue normal, int irda) {
    const NsState *s = c->owner;

    (void)irda;
    if (brs_is_xr(s->dev)) {
        return nb_known1(nb_ch(c)->emsr, 4) ? LOGIC_Z : normal;   /* EMSR[4] 3-state */
    }
    if (s->dev->extended == EXT_SC16IS_SFR && nb_known1(nb_ch(c)->sfr, 3)) {
        /* SFR[3]: internal loopback RX -> TX. */
        return serial_level(s->sim, s->instance, s->pins->channel[c->index][CH_RX], LOGIC_X);
    }
    return normal;
}

int ns_bridge_irda(const NsChannel *c) {
    return c->owner->bridge != NULL && brs_is_xr(c->owner->dev) && c->owner->bridge->enir_low;
}

int ns_bridge_ir_quarter(const NsChannel *c) {
    const NsBridge *br = c->owner->bridge;

    if (br == NULL || !br->fast_irda) {
        return 0;
    }
    if (brs_is_xr(c->owner->dev)) {
        return nb_known1(nb_ch(c)->xsfr, 3);
    }
    return nb_known1(nb_ch(c)->efcr, 7);
}

int ns_bridge_prescale(NsChannel *c, int rising) {
    const NbChannel *n;
    unsigned m;
    unsigned total;

    if (c->owner->bridge == NULL || c->owner->dev->extended != EXT_SC16IS_SFR) {
        return -1;
    }
    n = nb_ch(c);
    /* PI7C9X: XIN / 2^(M - 1) / (4 with MCR[7]); M = 0 is undefined (P2):
       the generator stops. */
    m = (n->cpr.v >> 4) & 0x0Fu;
    if ((n->cpr.k & 0xF0u) != 0xF0u || m == 0 || bit_of(c->mcr, 7) == LOGIC_X) {
        return 0;
    }
    total = (1u << (m - 1)) * (nb_known1(c->mcr, 7) ? 4u : 1u);
    if (total == 1) {
        return 1;
    }
    if (!rising) {
        return 0;
    }
    if (++c->prescale_count < total) {
        return 0;
    }
    c->prescale_count = 0;
    return 1;
}

int ns_bridge_load_divisor(NsChannel *c) {
    const NbChannel *n;

    if (c->owner->bridge == NULL) {
        return 0;
    }
    if (brs_is_xr(c->owner->dev)) {
        xr_load_divisors(c);
        return 1;
    }
    if (c->owner->dev->extended != EXT_SC16IS_DLD) {
        return 0;
    }
    n = nb_ch(c);
    serial_baud_load(&c->baud, (uint32_t)c->dll.v | ((uint32_t)c->dlm.v << 8),
                     c->dll.k == 0xFF && c->dlm.k == 0xFF);
    c->baud.frac = (n->dld.k & 0x0Fu) == 0x0Fu ? n->dld.v & 0x0Fu : 0;
    c->baud.frac_den = 16;
    return 1;
}

int ns_bridge_factor(const NsChannel *c) {
    const NbChannel *n;
    unsigned mode;

    if (c->owner->bridge == NULL) {
        return 0;
    }
    n = nb_ch(c);
    mode = (n->dld.v >> 4) & 3u;
    switch (c->owner->dev->extended) {
        case EXT_SC16IS_DLD:
            return mode == 0 ? 16 : mode == 1 ? 8 : 4;   /* DLD[5:4]: 16x, 8x, 4x */
        case EXT_SC16IS_SFR:
            /* Sample rate 16 - SCR + N (TRCTL[7:4], CPR[3:0]). */
            return (int)(16u - ((n->trctl.v >> 4) & 0x0Fu) + (n->cpr.v & 0x0Fu));
        case EXT_XR1280:
            return (int)n->tx_factor;
        default:
            return 0;
    }
}

int ns_bridge_tx_threshold(const NsChannel *c, unsigned *out) {
    const NsDevice *d = c->owner->dev;
    unsigned tlr = c->tlr.v & 0x0Fu;

    if (d->bridge == NULL) {
        return 0;
    }
    if (brs_is_xr(d)) {
        *out = xr_tx_threshold(c);
        return 1;
    }
    /* SC16IS: the TX trigger applies in FIFO mode with or without EFR[4]
       (FCR[5:4] reset 00 = 8 spaces; EFR[4] only gates writing them). */
    if (!c->fifo_on) {
        *out = 0;
    } else if (tlr != 0) {
        *out = tlr * 4u;
    } else {
        *out = d->tx_trigger[c->tx_trigger & 3u];
    }
    return 1;
}

int ns_bridge_rx_trigger(const NsChannel *c, unsigned *out) {
    if (c->owner->dev->bridge == NULL || !brs_is_xr(c->owner->dev)) {
        return 0;
    }
    *out = xr_rx_trigger(c);
    return 1;
}

int ns_bridge_ier_mask(const NsChannel *c, uint8_t *mask) {
    if (c->owner->dev->bridge == NULL) {
        return 0;
    }
    *mask = brs_enhanced(c) ? 0xFF : 0x0F;   /* IER[7:4] need EFR[4] */
    return 1;
}

int ns_bridge_mcr_mask(const NsChannel *c, uint8_t *mask) {
    int e;

    if (c->owner->dev->bridge == NULL) {
        return 0;
    }
    e = brs_enhanced(c);
    switch (c->owner->dev->extended) {
        case EXT_SC16IS: *mask = e ? 0xF7 : 0x13; break;     /* MCR[3] reserved */
        case EXT_SC16IS_SFR: *mask = e ? 0xFF : 0x13; break; /* MCR[3:2] need EFR[4] */
        case EXT_SC16IS_DLD: *mask = e ? 0xFF : 0x1B; break; /* OP2 always */
        default: *mask = e ? 0xFF : 0x1F; break;             /* XR20M1280: MCR[2] GPIO select */
    }
    return 1;
}

int ns_bridge_thr_on_enable(const NsChannel *c) {
    /* SC16IS7xx p27 note 1: re-enabling IER[1] gives no new interrupt (the
       PI7C9X clone too); the XR20M1170/1280 raise one when the THR is empty. */
    switch (c->owner->dev->extended) {
        case EXT_SC16IS:
        case EXT_SC16IS_SFR: return 0;
        default: return 1;
    }
}

LogicValue ns_bridge_gpio_source(const NsChannel *c) {
    if (c->index != 0 || c->owner->bridge == NULL) {
        return LOGIC_0;   /* channel A's IIR only (planner decision) */
    }
    return brs_sc_gpio_pending(c->owner);
}

LogicValue ns_bridge_ls_source(const NsChannel *c, LogicValue normal) {
    if (brs_is_xr(c->owner->dev)) {
        const NbChannel *n = nb_ch(c);

        /* EMSR[6] = 1: at once when an errored byte comes in. */
        if (nb_known1(n->emsr, 6) && n->ls_now) {
            return or3(normal, bit_of(c->ier, 2));
        }
    }
    return normal;
}

LogicValue ns_bridge_thr_source(const NsChannel *c, LogicValue normal) {
    const NbChannel *n = nb_ch(c);

    switch (c->owner->dev->extended) {
        case EXT_SC16IS_SFR:
            return and3(normal, bit_of(n->trctl, 2));   /* TRCTL[2] TX empty interrupt enable */
        case EXT_SC16IS_DLD:
            /* Auto RS-485, no FIFO: the TX interrupt waits for the TSR. */
            if (nb_known1(n->efcr, 4) && !c->fifo_on) {
                return and3(normal, bool3(!serial_tx_busy(&c->line.tx)));
            }
            return normal;
        case EXT_XR1280:
            if (nb_known1(n->fctr, 3)) {
                return and3(normal, bool3(c->tx_count == 0 && !serial_tx_busy(&c->line.tx)));
            }
            return normal;
        default:
            return normal;
    }
}

int ns_bridge_timeout_on(const NsChannel *c) {
    if (c->owner->dev->extended == EXT_SC16IS_SFR) {
        return nb_known1(nb_ch(c)->trctl, 1);   /* TRCTL[1] RX timeout enable */
    }
    return 1;
}

void ns_bridge_iir_bits(const NsChannel *c, uint8_t *v, uint8_t *k) {
    if (brs_is_xr(c->owner->dev) && c->owner->bridge != NULL) {
        /* ISR[6]: differs from ISR[7] while a GPIO interrupt is pending. */
        LogicValue g = xr_gpio_int(c->owner);

        if (g == LOGIC_X) {
            *k &= (uint8_t)~0x40u;
        } else if (g == LOGIC_1) {
            *v ^= 0x40u;
        }
    }
}

void ns_bridge_thr_written(NsChannel *c, unsigned at) {
    if (brs_is_xr(c->owner->dev)) {
        xr_thr_written(c, at);
    }
}

int ns_bridge_tx_fetch(NsChannel *c) {
    return brs_is_xr(c->owner->dev) ? xr_tx_fetch(c) : 0;
}

int ns_bridge_clock(NsState *s, NsChannel *c, int rising, LogicValue level) {
    (void)level;
    if (!brs_is_xr(s->dev)) {
        return 0;
    }
    return xr_clock(s, c, rising);
}

int ns_bridge_sleep_check(NsState *s, NsChannel *c) {
    return brs_is_xr(s->dev) ? xr_sleep_check(s, c) : 1;
}

void ns_bridge_sleep_changed(NsState *s, NsChannel *c) {
    if (s->bridge != NULL && brs_is_xr(s->dev)) {
        xr_sleep_changed(s, c);
    }
}
