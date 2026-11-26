/* models/ns16550_kind.c: part of the model in models/ns16550.c (split by plan-68-I/J). */
#include "ns16550_core_int.h"
/* ---- Engine callbacks ----------------------------------------------------------------------------- */

static LogicValue reset_level(const NsState *s) {
    int high = s->dev->x != NULL ? ns_x_reset_active_high(s) : s->dev->reset_active_high;   /* plan-12-E */
    LogicValue inactive = high ? LOGIC_0 : LOGIC_1;
    LogicValue v = serial_level(s->sim, s->instance, s->pins->reset, inactive);

    if (v == LOGIC_X) {
        return LOGIC_X;
    }
    return bool3(v == (high ? LOGIC_1 : LOGIC_0));
}

/* "iowOnly": MODE high = IOR/IOW cycles; low = CS is the strobe and the
   write pin picks the direction (serial_bus.c's Motorola cycles). */
static void read_iow_mode(NsState *s) {
    LogicValue mode;

    if (!SERIAL_REAL_PIN(s->pins->iow_mode)) {
        return;
    }
    mode = serial_level(s->sim, s->instance, s->pins->iow_mode, LOGIC_X);
    s->bus.motorola = mode == LOGIC_1 ? 0 : mode == LOGIC_0 ? 1 : -1;
    s->bus.rw = s->pins->iow_rw;
}

static void apply_reset(NsState *s) {
    LogicValue active = reset_level(s);

    if (active == LOGIC_X) {
        s->unknown = 1;
        s->in_reset = 0;
        s->bus.enabled = 0;
    } else if (active == LOGIC_1) {
        s->unknown = 0;
        s->in_reset = 1;
        s->bus.enabled = 0;
        nsc_reset_all(s);
    } else if (s->in_reset) {
        s->in_reset = 0;
        s->bus.enabled = 1;
    } else if (!s->unknown) {
        s->bus.enabled = 1;
    }
    if (s->bridge != NULL) {
        s->bus.enabled = 0;   /* plan-12-I: no parallel bus; the host port follows reset */
        ns_bridge_reset(s, active);
    }
    serial_bus_update(&s->bus, s->sim, s->instance, &s->events);
    nsc_update_outputs(s, CAUSE_SERIAL);
}

static void clock_edge(NsState *s, LogicValue level) {
    int rising = s->clock_level == LOGIC_0 && level == LOGIC_1;
    unsigned i;

    if (level == s->clock_level) {
        return;
    }
    if (!rising && !(s->clock_level == LOGIC_1 && level == LOGIC_0)) {
        /* Into or out of X: no edge; BAUDOUT follows a divisor-1 clock. */
        s->clock_level = level;
        for (i = 0; i < s->dev->channel_count; i++) {
            if (s->ch[i].baud.known && s->ch[i].baud.divisor == 1) {
                s->ch[i].baud.baudout = level == LOGIC_Z ? LOGIC_X : level;
            }
        }
        nsc_drive_baudout(s, s->ch[0].baud.baudout, t_of(s, T_BLD));
        return;
    }
    s->clock_level = level;
    if (rising && s->dev->bus_sync && s->bus_dirty) {
        s->bus_dirty = 0;
        serial_bus_update(&s->bus, s->sim, s->instance, &s->events);
    }
    if (s->dev->x != NULL) {
        ns_x_clock_edge(s, rising);   /* plan-12-E: the 5G timer */
    }
    for (i = 0; i < s->dev->channel_count; i++) {
        NsChannel *c = &s->ch[i];
        int tick = 0;
        LogicValue baudout;

        if (s->dev->x != NULL && ns_x_own_clock(s, i)) {
            continue;   /* plan-12-E: CHCCLK clocks this channel */
        }

        if (s->bridge != NULL && ns_bridge_clock(s, c, rising, level)) {
            continue;   /* plan-12-I: XR20M1280 independent TX/RX generators */
        }
        if (ns_ext_prescale(s, c, rising) || (!rising && !c->asleep &&
                                              !(s->dev->prescaler != PRESCALE_NONE &&
                                                bit_of(c->mcr, 7) == LOGIC_1))) {
            tick = serial_baud_edge(&c->baud, rising, level);
        }
        /* Asleep (or the 750's low-power mode) the baud clock stops: BAUDOUT X. */
        baudout = s->unknown || c->asleep ? LOGIC_X : c->baud.baudout;
        if (i == 0) {
            nsc_drive_baudout(s, baudout, t_of(s, T_BLD));
        }
        if (SERIAL_REAL_PIN(s->pins->mf[i]) && ((c->afr.v >> 1) & 3u) == 1 &&
            s->out[s->pins->mf[i]] != baudout) {
            s->out[s->pins->mf[i]] = baudout;
            sim_schedule_drive(s->sim, s->instance, s->pins->mf[i], s->tri ? LOGIC_Z : baudout,
                               DRIVE_STRONG, t_of(s, T_BLD));
        }
        if (s->dev->x != NULL) {
            ns_x_clock_out(s, c, baudout);   /* plan-12-E: BCLK pins */
        }
        if (tick) {
            nsc_tx_tick(s, c);
            if (s->pins->receive == SERIAL_NO_PIN) {
                nsc_rx_tick(s, c);
            }
        }
    }
}

static void modem_changed(NsState *s, NsChannel *c) {
    LogicValue now[4];

    if (s->in_reset || s->unknown) {
        nsc_modem_now(s, c, now);
        memcpy(c->msr, now, sizeof now);
        return;
    }
    nsc_modem_now(s, c, now);
    nsc_set_modem(c, now, ns_ext_cts_changed(c, now[M_CTS]) ? 0x0F : 0x0E);
    nsc_update_channel_outputs(s, c, CAUSE_MODEM_IN);
    serial_bus_refresh(&s->bus, s->sim, s->instance, &s->events);
}

static void ns_start(Sim *sim, SimInstance *instance, void *raw) {
    NsState *s = raw;
    unsigned i;

    s->sim = sim;
    s->instance = instance;
    for (i = 0; i < s->dev->channel_count; i++) {
        LogicValue now[4];

        nsc_modem_now(s, &s->ch[i], now);
        memcpy(s->ch[i].msr, now, sizeof now);
    }
    s->clock_level = serial_level(sim, instance, s->pins->clock_in, LOGIC_X);
    s->receive_level = serial_level(sim, instance, s->pins->receive, LOGIC_X);
    nsc_drive_baudout(s, s->ch[0].baud.baudout, 0);
    s->tri = serial_level(sim, instance, s->pins->tri, LOGIC_0) == LOGIC_1;
    if (s->dev->x != NULL) {
        ns_x_start(s);   /* plan-12-E: bus mode, own clocks */
    }
    read_iow_mode(s);
    serial_bus_start(&s->bus, sim, instance, &s->events);
    if (s->bridge != NULL) {
        ns_bridge_start(s);   /* plan-12-I */
    }
    apply_reset(s);
}

/* `pin` is a real pin (the one caller is ns_pin_changed). */
static int is_select_pin(const NsState *s, size_t pin) {
    unsigned i;
    unsigned k;

    if (pin == s->pins->channel_pin || (s->pins->lpt != NULL && pin == s->pins->lpt->select)) {
        return 1;
    }
    if (s->dev->x != NULL && ns_x_is_select_pin(s, pin)) {
        return 1;   /* plan-12-E */
    }
    for (i = 0; i < s->dev->channel_count; i++) {
        for (k = 0; k < s->dev->chan_select_count[i]; k++) {
            if (s->pins->chan_select[i][k] == pin) {
                return 1;
            }
        }
        for (k = 0; k < s->dev->chan_select_high_count[i]; k++) {
            if (s->pins->chan_select_high[i][k] == pin) {
                return 1;
            }
        }
    }
    return 0;
}

static void ns_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    NsState *s = raw;
    unsigned i;
    unsigned j;

    (void)old_value;
    (void)new_value;
    s->sim = sim;
    s->instance = instance;
    if (pin == s->pins->reset) {
        apply_reset(s);
        return;
    }
    if (pin == s->pins->iow_mode && SERIAL_REAL_PIN(pin)) {
        read_iow_mode(s);
        serial_bus_update(&s->bus, s->sim, s->instance, &s->events);
    }
    if (s->dev->x != NULL && ns_x_pin_changed(s, pin)) {
        apply_reset(s);   /* plan-12-E: the bus mode changed the reset polarity */
    }
    if (s->bridge != NULL) {
        ns_bridge_pin(s, pin);   /* plan-12-I: host port, GPIO, straps */
    }
    if (pin == s->pins->clock_in) {
        clock_edge(s, serial_level(sim, instance, pin, LOGIC_X));
    }
    if (pin == s->pins->receive) {
        LogicValue level = serial_level(sim, instance, pin, LOGIC_X);
        int rising = s->receive_level == LOGIC_0 && level == LOGIC_1;

        s->receive_level = level;
        if (rising) {
            for (i = 0; i < s->dev->channel_count; i++) {
                nsc_rx_tick(s, &s->ch[i]);
            }
        }
    }
    if (serial_bus_has_pin(&s->bus, pin) || is_select_pin(s, pin)) {
        if (s->dev->bus_sync) {
            s->bus_dirty = 1;   /* sampled at the next XIN rising edge */
            serial_bus_capture(&s->bus, sim, instance);
        } else {
            serial_bus_update(&s->bus, sim, instance, &s->events);
        }
    }
    if (pin == s->pins->int_control) {
        nsc_update_outputs(s, CAUSE_SERIAL);
    }
    if (pin == s->pins->tri) {
        LogicValue tri = serial_level(sim, instance, pin, LOGIC_0);
        size_t p;

        s->tri = tri == LOGIC_1;
        for (p = 0; p < s->pin_count; p++) {
            if (s->out[p] != LOGIC_Z || s->tri) {
                sim_schedule_drive(sim, instance, p, s->tri ? LOGIC_Z : s->out[p], DRIVE_STRONG, 0);
            }
        }
    }
    if (ns_lpt_has_pin(s, pin)) {
        ns_lpt_pins(s);
    }
    for (i = 0; i < s->dev->channel_count; i++) {
        if (pin == s->pins->channel[i][CH_RX]) {
            ns_ext_wake(&s->ch[i]);   /* sleep wakes on RX activity */
        }
    }
    for (i = 0; i < s->dev->channel_count; i++) {
        for (j = 0; j < 4; j++) {
            if (pin == s->pins->channel[i][MODEM_PIN[j]]) {
                modem_changed(s, &s->ch[i]);
            }
        }
    }
}

/* plan-12-I: the bridges' timers (the I2C SDA-low timeout). */
static void ns_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t timer_id) {
    NsState *s = raw;

    s->sim = sim;
    s->instance = instance;
    if (s->bridge != NULL) {
        ns_bridge_timer(s, timer_id);
    }
}

static void *ns_load_device(KindContext *ctx, const cJSON *json, unsigned *channels) {
    static const char *const INT_PINS[] = {
        "driven", "out2-gated", "intn", "intsel", "mcr5-open-source", NULL
    };
    static const char *const DMA_CLASSES[] = {"P", "N", "T", "X", "B", NULL};
    static const char *const DMA_PINS[] = {"perChannel", "sharedAnd", "sharedWireOr", NULL};
    static const char *const BROADCASTS[] = {"none", "multi-cs", "afr-conc",
                                              "afr0-perChannel" /* plan-12-E */, NULL};
    static const char *const TIMEOUTS[] = {"4ch", "4w12", NULL};
    static const char *const PRIORITIES[] = {"standard", "nxpEqual",
                                              "exarTimeoutFirst" /* plan-12-E */, NULL};
    static const char *const LSR7[] = {"noErrors", "lsrRead", NULL};
    static const char *const STOPS[] = {"first", "both", NULL};
    NsDevice *d = kind_alloc(ctx, sizeof *d);
    const cJSON *fifo;
    unsigned max_fifo;
    int choice;
    int value;
    int host;

    if (d == NULL) {
        return NULL;
    }
    host = nsc_load_bus(ctx, json, d);
    if (host < 0 || nsc_load_clock(ctx, json, d) != 0 || nsc_load_channels(ctx, json, d) != 0) {
        return NULL;
    }
    /* "fifo": 0, 16, 32, 64, 128, or "750" (16 or 64 by FCR[5]). */
    fifo = kind_member(ctx, json, "fifo", 1);
    if (fifo == NULL) {
        return NULL;
    }
    if (cJSON_IsString(fifo) && strcmp(fifo->valuestring, "750") == 0) {
        d->fifo = 16;
        d->fifo750 = 1;
    } else if (cJSON_IsNumber(fifo) && (fifo->valuedouble == 0 || fifo->valuedouble == 16 ||
                                         fifo->valuedouble == 32 || fifo->valuedouble == 64 ||
                                         fifo->valuedouble == 128)) {
        d->fifo = (unsigned)fifo->valuedouble;
    } else {
        kind_error(ctx, "\"fifo\" must be 0, 16, 32, 64, 128 or \"750\"");
        return NULL;
    }
    max_fifo = d->fifo750 ? 64 : d->fifo;
    if (nsc_load_levels(ctx, json, "rxTrigger", d->fifo, d->rx_trigger) != 0 ||
        nsc_load_levels(ctx, json, "rxTrigger64", 64, d->rx_trigger64) != 0 ||
        nsc_load_levels(ctx, json, "txTrigger", max_fifo, d->tx_trigger) != 0) {
        return NULL;
    }
    d->has_tx_trigger = d->tx_trigger[0] != 0;
    if (d->rx_trigger64[0] != 0 && !d->fifo750) {
        kind_error(ctx, "\"rxTrigger64\" needs \"fifo\": \"750\"");
        return NULL;
    }
    if (nsc_load_byte(ctx, json, "tlr", 0, &value) != 0) {
        return NULL;
    }
    if (value != 0 && value != 4 && value != 8) {
        kind_error(ctx, "\"tlr\" must be 4 or 8");
        return NULL;
    }
    d->tlr = (unsigned)value;

    if (nsc_load_choice(ctx, json, "intPin", INT_PINS, INT_DRIVEN, &choice) != 0) {
        return NULL;
    }
    d->int_pin = (NsIntPin)choice;
    if (serial_optional_name(ctx, json, "intControl", &d->int_control) != 0) {
        return NULL;
    }
    if ((d->int_pin == INT_INTN || d->int_pin == INT_INTSEL) != (d->int_control != NULL)) {
        kind_error(ctx, "\"intControl\" names the INTN/INTSEL pin, for \"intn\" and \"intsel\" only");
        return NULL;
    }
    if (nsc_load_choice(ctx, json, "dmaClass", DMA_CLASSES, 0, &choice) != 0) {
        return NULL;
    }
    d->dma_class = "PNTXB"[choice];
    if (nsc_load_choice(ctx, json, "dmaPins", DMA_PINS, DMA_PINS_NAMED, &choice) != 0) {
        return NULL;
    }
    d->dma_pins = (NsDmaPins)choice;
    if (cJSON_GetObjectItemCaseSensitive(json, "dma") != NULL) {
        static const char *const DMA_KEYS[] = {"txrdy", "rxrdy", NULL};
        const cJSON *dma = cJSON_GetObjectItemCaseSensitive(json, "dma");

        kind_push(ctx, "dma");
        if (kind_check_keys(ctx, dma, DMA_KEYS, NULL) != 0 ||
            serial_optional_name(ctx, dma, "txrdy", &d->dma_txrdy) != 0 ||
            serial_optional_name(ctx, dma, "rxrdy", &d->dma_rxrdy) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    if ((d->dma_pins != DMA_PINS_NAMED) != (d->dma_txrdy != NULL || d->dma_rxrdy != NULL)) {
        kind_error(ctx, "shared \"dmaPins\" and the \"dma\" pins go together");
        return NULL;
    }
    if (nsc_load_choice(ctx, json, "broadcastWrite", BROADCASTS, BROADCAST_NONE, &choice) != 0) {
        return NULL;
    }
    d->broadcast = (NsBroadcast)choice;
    if (cJSON_GetObjectItemCaseSensitive(json, "channelSelect") != NULL) {
        static const char *const CS_KEYS[] = {"pin", "aIsHigh", NULL};
        const cJSON *cs = cJSON_GetObjectItemCaseSensitive(json, "channelSelect");

        kind_push(ctx, "channelSelect");
        if (kind_check_keys(ctx, cs, CS_KEYS, NULL) != 0 ||
            kind_string(ctx, cs, "pin", &d->channel_pin) != 0 ||
            kind_bool(ctx, cs, "aIsHigh", &d->a_is_high) != 0) {
            return NULL;
        }
        kind_pop(ctx);
        if (d->channel_count != 2 || d->per_channel_select) {
            kind_error(ctx, "\"channelSelect\" is for 2 channels sharing one chip select");
            return NULL;
        }
    }
    if (d->channel_count > 1 && !d->per_channel_select && d->channel_pin == NULL && !host &&
        d->address_count != 8) {   /* plan-12-E: the 5G map decodes A6..A4 */
        kind_error(ctx, "more than one channel needs per-channel selects or \"channelSelect\"");
        return NULL;
    }
    if (nsc_load_byte(ctx, json, "lcrReset", 0, &value) != 0) {
        return NULL;
    }
    d->lcr_reset = (uint8_t)value;
    if (nsc_load_byte(ctx, json, "sprReset", -1, &d->spr_reset) != 0 ||
        nsc_load_byte(ctx, json, "tcrReset", 0, &value) != 0) {
        return NULL;
    }
    d->tcr_reset = (uint8_t)value;
    if (nsc_load_choice(ctx, json, "timeout", TIMEOUTS, 0, &choice) != 0) {
        return NULL;
    }
    d->timeout_4w12 = choice == 1;
    if (nsc_load_choice(ctx, json, "iirPriority", PRIORITIES, IIR_STANDARD, &choice) != 0) {
        return NULL;
    }
    d->iir_priority = (NsIirPriority)choice;
    if (nsc_load_choice(ctx, json, "lsr7Clear", LSR7, 0, &choice) != 0) {
        return NULL;
    }
    d->lsr7_on_read = choice == 1;
    if (nsc_load_choice(ctx, json, "stopCheck", STOPS, 0, &choice) != 0) {
        return NULL;
    }
    d->stop_both = choice == 1;
    if (nsc_load_optional_bool(ctx, json, "statusReadDefer", &d->status_defer) != 0 ||
        nsc_load_optional_bool(ctx, json, "busSync", &d->bus_sync) != 0 ||
        nsc_load_optional_bool(ctx, json, "majorityVote", &d->majority) != 0) {
        return NULL;
    }
    /* plan-12-E's Exar keys are for the parallel-bus devices; a bridge (an
       I2C/SPI host port, plan-12-I) has its own "interrupt" and no A2..A0. */
    if (nsc_load_phase2(ctx, json, d) != 0 || (!host && ns_x_load(ctx, json, d) != 0)) {
        return NULL;
    }
    /* plan-12-I: the serial host port and the bridge keys. */
    if (host) {
        if (ns_bridge_load(ctx, json, cJSON_GetObjectItemCaseSensitive(json, "bus"), d) != 0) {
            return NULL;
        }
    } else {
        unsigned i;

        for (i = 0; nsc_BRIDGE_KEYS[i] != NULL; i++) {
            if (cJSON_GetObjectItemCaseSensitive(json, nsc_BRIDGE_KEYS[i]) != NULL) {
                kind_error(ctx, "\"%s\" needs an I2C/SPI \"bus\"", nsc_BRIDGE_KEYS[i]);
                return NULL;
            }
        }
        if (d->extended >= EXT_SC16IS && d->extended <= EXT_XR1280) {
            kind_error(ctx, "the bridge \"extended\" styles need an I2C/SPI \"bus\"");
            return NULL;
        }
    }
    *channels = d->channel_count;
    return d;
}

/* The device being bound: its "absentTies" / "optionalPins" (binding is one
   device at a time). */
static const SerialAbsent *binding_absent;

static int bind_pin(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *key,
                    const char *name, size_t *out) {
    return serial_bind_pin_absent(ctx, pins, pin_count, key, name, binding_absent, out);
}

static void *ns_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t pin_count) {
    const NsDevice *d = raw;
    NsPins *p = kind_alloc(ctx, sizeof *p);
    char key[32];
    unsigned i;
    unsigned k;

    if (p == NULL) {
        return NULL;
    }
    binding_absent = &d->absent;
    for (i = 0; i < SERIAL_BUS_DATA; i++) {
        snprintf(key, sizeof key, "bus.data[%u]", i);
        if (bind_pin(ctx, pins, pin_count, key, d->data[i], &p->bus.data[i]) != 0) {
            return NULL;
        }
    }
    for (i = 0; i < d->address_count; i++) {
        snprintf(key, sizeof key, "bus.address[%u]", i);
        if (bind_pin(ctx, pins, pin_count, key, d->address[i], &p->bus.address[i]) != 0) {
            return NULL;
        }
    }
    p->bus.address_count = d->address_count;
    for (i = 0; i < d->select_count; i++) {
        if (bind_pin(ctx, pins, pin_count, "bus.select", d->select[i], &p->bus.select[i]) != 0) {
            return NULL;
        }
    }
    p->bus.select_count = d->select_count;
    for (i = 0; i < d->select_high_count; i++) {
        if (bind_pin(ctx, pins, pin_count, "bus.selectHigh", d->select_high[i],
                            &p->bus.select_high[i]) != 0) {
            return NULL;
        }
    }
    p->bus.select_high_count = d->select_high_count;
    if (bind_pin(ctx, pins, pin_count, "bus.read", d->read, &p->bus.read) != 0 ||
        bind_pin(ctx, pins, pin_count, "bus.readHigh", d->read_high, &p->bus.read_high) != 0 ||
        bind_pin(ctx, pins, pin_count, "bus.write", d->write, &p->bus.write) != 0 ||
        bind_pin(ctx, pins, pin_count, "bus.writeHigh", d->write_high, &p->bus.write_high) != 0 ||
        bind_pin(ctx, pins, pin_count, "bus.addressStrobe", d->address_strobe,
                        &p->bus.address_strobe) != 0 ||
        bind_pin(ctx, pins, pin_count, "bus.driverDisable", d->driver_disable,
                        &p->bus.driver_disable) != 0 ||
        bind_pin(ctx, pins, pin_count, "bus.selectOut", d->select_out, &p->bus.select_out) != 0 ||
        bind_pin(ctx, pins, pin_count, "bus.reset", d->reset, &p->reset) != 0 ||
        bind_pin(ctx, pins, pin_count, "bus.iowOnly", d->iow_mode, &p->iow_mode) != 0 ||
        bind_pin(ctx, pins, pin_count, "bus.iowOnly", d->iow_rw, &p->iow_rw) != 0 ||
        bind_pin(ctx, pins, pin_count, "clock.in", d->clock_in, &p->clock_in) != 0 ||
        bind_pin(ctx, pins, pin_count, "clock.out", d->clock_out, &p->clock_out) != 0 ||
        bind_pin(ctx, pins, pin_count, "clock.receive", d->receive, &p->receive) != 0 ||
        bind_pin(ctx, pins, pin_count, "clock.baudOut", d->baudout, &p->baudout) != 0) {
        return NULL;
    }
    for (i = 0; i < SERIAL_MAX_CHANNELS; i++) {
        for (k = 0; k < CH_PIN_COUNT; k++) {
            p->channel[i][k] = SERIAL_NO_PIN;
        }
    }
    for (i = 0; i < d->channel_count; i++) {
        for (k = 0; k < CH_PIN_COUNT; k++) {
            snprintf(key, sizeof key, "channels[%u].%s", i, nsc_CHANNEL_KEYS[k]);
            if (bind_pin(ctx, pins, pin_count, key, d->channel[i][k], &p->channel[i][k]) != 0) {
                return NULL;
            }
        }
        for (k = 0; k < d->chan_select_count[i]; k++) {
            snprintf(key, sizeof key, "channels[%u].select", i);
            if (bind_pin(ctx, pins, pin_count, key, d->chan_select[i][k],
                                &p->chan_select[i][k]) != 0) {
                return NULL;
            }
        }
        for (k = 0; k < d->chan_select_high_count[i]; k++) {
            snprintf(key, sizeof key, "channels[%u].selectHigh", i);
            if (bind_pin(ctx, pins, pin_count, key, d->chan_select_high[i][k],
                                &p->chan_select_high[i][k]) != 0) {
                return NULL;
            }
        }
    }
    if (bind_pin(ctx, pins, pin_count, "channelSelect.pin", d->channel_pin,
                        &p->channel_pin) != 0 ||
        bind_pin(ctx, pins, pin_count, "dma.txrdy", d->dma_txrdy, &p->dma_txrdy) != 0 ||
        bind_pin(ctx, pins, pin_count, "dma.rxrdy", d->dma_rxrdy, &p->dma_rxrdy) != 0 ||
        bind_pin(ctx, pins, pin_count, "intControl", d->int_control, &p->int_control) != 0 ||
        bind_pin(ctx, pins, pin_count, "clock.select", d->clock_select, &p->clock_select) != 0 ||
        bind_pin(ctx, pins, pin_count, "tristate", d->tri, &p->tri) != 0) {
        return NULL;
    }
    for (i = 0; i < SERIAL_MAX_CHANNELS; i++) {
        p->op[i] = SERIAL_NO_PIN;
        p->mf[i] = SERIAL_NO_PIN;
    }
    for (i = 0; i < d->channel_count; i++) {
        snprintf(key, sizeof key, "channels[%u].op", i);
        if (bind_pin(ctx, pins, pin_count, key, d->op[i], &p->op[i]) != 0) {
            return NULL;
        }
        snprintf(key, sizeof key, "channels[%u].mf", i);
        if (bind_pin(ctx, pins, pin_count, key, d->mf[i], &p->mf[i]) != 0) {
            return NULL;
        }
    }
    if (d->lpt != NULL) {
        NsLptPins *lp = kind_alloc(ctx, sizeof *lp);
        const NsLptDevice *ld = d->lpt;
        const char *const names[] = {ld->select, ld->strobe, ld->auto_feed, ld->init, ld->select_in,
                                     ld->ack, ld->busy, ld->paper_end, ld->slct, ld->error, ld->irq,
                                     ld->input_enable, ld->pemd, ld->enirq};
        size_t *const slots[] = {&lp->select, &lp->strobe, &lp->auto_feed, &lp->init, &lp->select_in,
                                 &lp->ack, &lp->busy, &lp->paper_end, &lp->slct, &lp->error, &lp->irq,
                                 &lp->input_enable, &lp->pemd, &lp->enirq};

        if (lp == NULL) {
            return NULL;
        }
        for (i = 0; i < 8; i++) {
            snprintf(key, sizeof key, "printerPort.data[%u]", i);
            if (bind_pin(ctx, pins, pin_count, key, ld->data[i], &lp->data[i]) != 0) {
                return NULL;
            }
        }
        for (i = 0; i < sizeof names / sizeof names[0]; i++) {
            if (bind_pin(ctx, pins, pin_count, "printerPort", names[i], slots[i]) != 0) {
                return NULL;
            }
        }
        p->lpt = lp;
    }
    if (d->bridge != NULL && ns_bridge_bind(ctx, d, pins, pin_count, p) != 0) {
        return NULL;   /* plan-12-I */
    }
    if (d->x != NULL && ns_x_bind(ctx, d, pins, pin_count, p, &d->absent) != 0) {
        return NULL;   /* plan-12-E */
    }
    return p;
}

static int ns_open_drain(const void *data, const void *pins, const void *options, size_t pin) {
    const NsDevice *d = data;

    if (d->bridge != NULL) {
        return ns_bridge_open_drain(d, pins, pin);   /* plan-12-I */
    }
    return ns_x_open_drain(data, pins, options, pin);   /* plan-12-E */
}

/* With a "tristate" pin (the TL16C552's TRI), every output floats while it is high. */
static int ns_tristate(const void *data, const void *pins, const void *options, size_t pin) {
    const NsPins *p = pins;

    (void)data;
    (void)options;
    (void)pin;
    return p->tri != SERIAL_NO_PIN;
}

static const char *const PART_KEYS[] = {"intPin" /* plan-12-E */, NULL};

const SerialModel NS16550_MODEL = {
    "ns16550", nsc_KEYS, nsc_TIMING, PART_KEYS,
    ns_load_device, ns_bind, ns_x_load_part, ns_open_drain,
    nsc_ns_create, nsc_ns_destroy, ns_start, ns_pin_changed, ns_timer,
    nsc_ns_channels, nsc_ns_line, nsc_ns_async,
    NULL, NULL, NULL, ns_tristate
};
