/* models/ns16550_load.c: part of the model in models/ns16550.c (split by plan-68-I/J). */
#include "ns16550_core_int.h"
/* ---- Loader ---------------------------------------------------------------------------------------- */

static int load_name_list(KindContext *ctx, const cJSON *object, const char *key, int required,
                          const char **out, unsigned cap, unsigned *count, unsigned exact) {
    const char *const *names;
    size_t n;
    size_t i;

    *count = 0;
    if (cJSON_GetObjectItemCaseSensitive(object, key) == NULL) {
        if (required) {
            return kind_error(ctx, "missing \"%s\"", key);
        }
        return 0;
    }
    if (kind_names(ctx, object, key, &names, &n) != 0) {
        return -1;
    }
    if ((exact != 0 && n != exact) || n > cap) {
        if (exact != 0) {
            return kind_error(ctx, "\"%s\" needs exactly %u pin names", key, exact);
        }
        return kind_error(ctx, "\"%s\" has at most %u pin names", key, cap);
    }
    for (i = 0; i < n; i++) {
        out[i] = names[i];
    }
    *count = (unsigned)n;
    return 0;
}

/* 1 = the bus is a serial host port (plan-12-I, loaded by ns_bridge_load). */
int nsc_load_bus(KindContext *ctx, const cJSON *json, NsDevice *d) {
    static const char *const BUS_KEYS[] = {
        "style", "data", "address", "read", "readHigh", "write", "writeHigh", "select",
        "selectHigh", "addressStrobe", "driverDisable", "selectOut", "reset", "resetActiveHigh",
        "iowOnly",
        /* plan-12-E: "intel-motorola" */
        "modePin", "rw", "strobe", "channelBits", "irq", "irqOthers",
        NULL
    };
    const cJSON *bus = kind_member(ctx, json, "bus", 1);
    const char *style;
    unsigned n;

    if (bus == NULL) {
        return -1;
    }
    kind_push(ctx, "bus");
    if (kind_string(ctx, bus, "style", &style) != 0) {
        return -1;
    }
    if ((strcmp(style, "intel") == 0 || strcmp(style, "intel-motorola") == 0) &&
        kind_check_keys(ctx, bus, BUS_KEYS, NULL) != 0) {
        return -1;
    }
    if (strcmp(style, "spi") == 0 || strcmp(style, "i2c") == 0 || strcmp(style, "i2c-spi") == 0) {
        kind_pop(ctx);
        return 1;   /* plan-12-I */
    }
    if (strcmp(style, "intel") != 0 && ns_x_load_bus(ctx, bus, d, style) != 0) {   /* plan-12-E */
        return -1;
    }
    if (load_name_list(ctx, bus, "data", 1, d->data, SERIAL_BUS_DATA, &n, SERIAL_BUS_DATA) != 0 ||
        load_name_list(ctx, bus, "address", 1, d->address, SERIAL_BUS_ADDRESS, &d->address_count,
                       0) != 0 ||
        load_name_list(ctx, bus, "select", 0, d->select, SERIAL_BUS_SELECTS, &d->select_count, 0) != 0 ||
        load_name_list(ctx, bus, "selectHigh", 0, d->select_high, SERIAL_BUS_SELECTS,
                       &d->select_high_count, 0) != 0 ||
        serial_optional_name(ctx, bus, "read", &d->read) != 0 ||
        serial_optional_name(ctx, bus, "readHigh", &d->read_high) != 0 ||
        serial_optional_name(ctx, bus, "write", &d->write) != 0 ||
        serial_optional_name(ctx, bus, "writeHigh", &d->write_high) != 0 ||
        serial_optional_name(ctx, bus, "addressStrobe", &d->address_strobe) != 0 ||
        serial_optional_name(ctx, bus, "driverDisable", &d->driver_disable) != 0 ||
        serial_optional_name(ctx, bus, "selectOut", &d->select_out) != 0 ||
        serial_optional_name(ctx, bus, "reset", &d->reset) != 0) {
        return -1;
    }
    if (d->address_count != 3 && d->address_count != 8) {
        return kind_error(ctx, "\"address\" needs 3 pin names (8 for \"5g\")");   /* plan-12-E */
    }
    if (d->read == NULL && d->read_high == NULL) {
        return kind_error(ctx, "a bus needs \"read\" or \"readHigh\"");
    }
    if (d->write == NULL && d->write_high == NULL) {
        return kind_error(ctx, "a bus needs \"write\" or \"writeHigh\"");
    }
    d->reset_active_high = 1;
    if (cJSON_GetObjectItemCaseSensitive(bus, "resetActiveHigh") != NULL &&
        kind_bool(ctx, bus, "resetActiveHigh", &d->reset_active_high) != 0) {
        return -1;
    }
    if (cJSON_GetObjectItemCaseSensitive(bus, "iowOnly") != NULL) {
        static const char *const IOW_KEYS[] = {"modePin", "rw", NULL};
        const cJSON *iow = cJSON_GetObjectItemCaseSensitive(bus, "iowOnly");

        kind_push(ctx, "iowOnly");
        if (kind_check_keys(ctx, iow, IOW_KEYS, NULL) != 0 ||
            kind_string(ctx, iow, "modePin", &d->iow_mode) != 0 ||
            kind_string(ctx, iow, "rw", &d->iow_rw) != 0) {
            return -1;
        }
        kind_pop(ctx);
    }
    kind_pop(ctx);
    return 0;
}

int nsc_load_clock(KindContext *ctx, const cJSON *json, NsDevice *d) {
    static const char *const CLOCK_KEYS[] = {"in", "out", "receive", "baudOut", "single", "select",
                                             "baudOutShape", NULL};
    const cJSON *clock = kind_member(ctx, json, "clock", 1);
    const char *single = NULL;

    if (clock == NULL) {
        return -1;
    }
    kind_push(ctx, "clock");
    if (kind_check_keys(ctx, clock, CLOCK_KEYS, NULL) != 0 ||
        serial_optional_name(ctx, clock, "single", &single) != 0) {
        return -1;
    }
    /* "single": one clock input for both directions (no RCLK, no BAUDOUT). */
    if (single != NULL) {
        if (cJSON_GetObjectItemCaseSensitive(clock, "in") != NULL ||
            cJSON_GetObjectItemCaseSensitive(clock, "receive") != NULL) {
            return kind_error(ctx, "\"single\" replaces \"in\" and \"receive\"");
        }
        d->clock_in = single;
    } else if (kind_string(ctx, clock, "in", &d->clock_in) != 0) {
        return -1;
    }
    if (cJSON_GetObjectItemCaseSensitive(clock, "baudOutShape") != NULL) {
        const char *shape;

        if (kind_string(ctx, clock, "baudOutShape", &shape) != 0) {
            return -1;
        }
        if (strcmp(shape, "low2") != 0 && strcmp(shape, "half") != 0) {
            return kind_error(ctx, "\"baudOutShape\" must be \"half\" or \"low2\"");
        }
        d->baudout_low2 = strcmp(shape, "low2") == 0;
    }
    if (serial_optional_name(ctx, clock, "select", &d->clock_select) != 0 ||
        serial_optional_name(ctx, clock, "out", &d->clock_out) != 0 ||
        serial_optional_name(ctx, clock, "receive", &d->receive) != 0 ||
        serial_optional_name(ctx, clock, "baudOut", &d->baudout) != 0) {
        return -1;
    }
    kind_pop(ctx);
    return 0;
}

int nsc_load_channels(KindContext *ctx, const cJSON *json, NsDevice *d) {
    const cJSON *list = kind_member(ctx, json, "channels", 1);
    const cJSON *item;
    unsigned n = 0;

    if (list == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) < 1 ||
        cJSON_GetArraySize(list) > SERIAL_MAX_CHANNELS) {
        return kind_error(ctx, "\"channels\" must be an array of 1 to %d channels",
                          SERIAL_MAX_CHANNELS);
    }
    cJSON_ArrayForEach(item, list) {
        unsigned k;

        kind_push(ctx, "channels[%u]", n);
        if (kind_check_keys(ctx, item, nsc_CHANNEL_KEYS, nsc_CHANNEL_EXTRA_KEYS) != 0) {
            return -1;
        }
        for (k = 0; k < CH_PIN_COUNT; k++) {
            if (serial_optional_name(ctx, item, nsc_CHANNEL_KEYS[k], &d->channel[n][k]) != 0) {
                return -1;
            }
        }
        if (d->channel[n][CH_TX] == NULL || d->channel[n][CH_RX] == NULL) {
            return kind_error(ctx, "a channel needs \"tx\" and \"rx\"");
        }
        if (serial_optional_name(ctx, item, "op", &d->op[n]) != 0 ||
            serial_optional_name(ctx, item, "mf", &d->mf[n]) != 0 ||
            ns_x_load_channel(ctx, item, d, n) != 0) {   /* plan-12-E: "bclk", "irtx" */
            return -1;
        }
        if (load_name_list(ctx, item, "select", 0, d->chan_select[n], SERIAL_BUS_SELECTS,
                           &d->chan_select_count[n], 0) != 0 ||
            load_name_list(ctx, item, "selectHigh", 0, d->chan_select_high[n], SERIAL_BUS_SELECTS,
                           &d->chan_select_high_count[n], 0) != 0) {
            return -1;
        }
        if (d->chan_select_count[n] + d->chan_select_high_count[n] > 0) {
            d->per_channel_select = 1;
        }
        kind_pop(ctx);
        n++;
    }
    d->channel_count = n;
    if (d->per_channel_select) {
        for (n = 0; n < d->channel_count; n++) {
            if (d->chan_select_count[n] + d->chan_select_high_count[n] == 0) {
                return kind_error(ctx, "with per-channel selects every channel needs one");
            }
        }
    }
    return 0;
}

/* An optional string member that must be one of `values`; *out = its index,
   or `fallback` when absent. */
int nsc_load_choice(KindContext *ctx, const cJSON *json, const char *key,
                       const char *const *values, int fallback, int *out) {
    const char *text;
    int i;

    *out = fallback;
    if (cJSON_GetObjectItemCaseSensitive(json, key) == NULL) {
        return 0;
    }
    if (kind_string(ctx, json, key, &text) != 0) {
        return -1;
    }
    for (i = 0; values[i] != NULL; i++) {
        if (strcmp(values[i], text) == 0) {
            *out = i;
            return 0;
        }
    }
    return kind_error(ctx, "unknown \"%s\" \"%s\"", key, text);
}

int nsc_load_optional_bool(KindContext *ctx, const cJSON *json, const char *key, int *out) {
    *out = 0;
    if (cJSON_GetObjectItemCaseSensitive(json, key) == NULL) {
        return 0;
    }
    return kind_bool(ctx, json, key, out);
}

/* 4 whole numbers from 1 to max. */
int nsc_load_levels(KindContext *ctx, const cJSON *json, const char *key, unsigned max,
                       unsigned out[4]) {
    const cJSON *list = cJSON_GetObjectItemCaseSensitive(json, key);
    const cJSON *item;
    unsigned i = 0;

    if (list == NULL) {
        return 0;
    }
    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) != 4) {
        return kind_error(ctx, "\"%s\" must be 4 numbers", key);
    }
    cJSON_ArrayForEach(item, list) {
        if (!cJSON_IsNumber(item) || item->valuedouble < 1 || item->valuedouble > max ||
            item->valuedouble != (double)(unsigned)item->valuedouble) {
            return kind_error(ctx, "\"%s\" values must be whole numbers from 1 to the FIFO size", key);
        }
        out[i++] = (unsigned)item->valuedouble;
    }
    return 0;
}

int nsc_load_byte(KindContext *ctx, const cJSON *json, const char *key, int fallback, int *out) {
    uint64_t v;

    *out = fallback;
    if (cJSON_GetObjectItemCaseSensitive(json, key) == NULL) {
        return 0;
    }
    if (kind_uint(ctx, json, key, 255, &v) != 0) {
        return -1;
    }
    *out = (int)v;
    return 0;
}

/* [[off, on] x 4]: a per-trigger table of two levels. */
static int load_pairs(KindContext *ctx, const cJSON *json, const char *key, uint8_t out[4][2],
                      int *present) {
    const cJSON *list = cJSON_GetObjectItemCaseSensitive(json, key);
    const cJSON *row;
    unsigned i = 0;

    *present = 0;
    if (list == NULL) {
        return 0;
    }
    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) != 4) {
        return kind_error(ctx, "\"%s\" must be 4 [level, level] pairs", key);
    }
    cJSON_ArrayForEach(row, list) {
        const cJSON *a = cJSON_GetArrayItem(row, 0);
        const cJSON *b = cJSON_GetArrayItem(row, 1);

        if (!cJSON_IsArray(row) || cJSON_GetArraySize(row) != 2 || !cJSON_IsNumber(a) ||
            !cJSON_IsNumber(b) || a->valuedouble < 0 || a->valuedouble > 128 ||
            b->valuedouble < 0 || b->valuedouble > 128) {
            return kind_error(ctx, "\"%s\" must be 4 [level, level] pairs of 0 to 128", key);
        }
        out[i][0] = (uint8_t)a->valuedouble;
        out[i][1] = (uint8_t)b->valuedouble;
        i++;
    }
    *present = 1;
    return 0;
}

static int load_printer_port(KindContext *ctx, const cJSON *json, NsDevice *d) {
    static const char *const LPT_KEYS[] = {
        "style", "data", "select", "strobe", "autoFeed", "init", "selectIn", "ack", "busy",
        "paperEnd", "slct", "error", "irq", "inputEnable", "pemd", "enirq", NULL
    };
    static const char *const STYLES[] = {"ti451", "ti552", "com92c451", NULL};
    const cJSON *port = cJSON_GetObjectItemCaseSensitive(json, "printerPort");
    NsLptDevice *l;
    unsigned n;
    int style;

    if (port == NULL) {
        return 0;
    }
    l = kind_alloc(ctx, sizeof *l);
    if (l == NULL) {
        return -1;
    }
    kind_push(ctx, "printerPort");
    if (kind_check_keys(ctx, port, LPT_KEYS, NULL) != 0 ||
        nsc_load_choice(ctx, port, "style", STYLES, -1, &style) != 0) {
        return -1;
    }
    if (style < 0) {
        return kind_error(ctx, "missing \"style\"");
    }
    l->style = (NsLptStyle)style;
    if (load_name_list(ctx, port, "data", 1, l->data, 8, &n, 8) != 0 ||
        kind_string(ctx, port, "select", &l->select) != 0 ||
        kind_string(ctx, port, "strobe", &l->strobe) != 0 ||
        kind_string(ctx, port, "autoFeed", &l->auto_feed) != 0 ||
        kind_string(ctx, port, "init", &l->init) != 0 ||
        kind_string(ctx, port, "selectIn", &l->select_in) != 0 ||
        kind_string(ctx, port, "ack", &l->ack) != 0 ||
        kind_string(ctx, port, "busy", &l->busy) != 0 ||
        kind_string(ctx, port, "paperEnd", &l->paper_end) != 0 ||
        kind_string(ctx, port, "slct", &l->slct) != 0 ||
        kind_string(ctx, port, "error", &l->error) != 0 ||
        kind_string(ctx, port, "irq", &l->irq) != 0 ||
        serial_optional_name(ctx, port, "inputEnable", &l->input_enable) != 0 ||
        serial_optional_name(ctx, port, "pemd", &l->pemd) != 0 ||
        serial_optional_name(ctx, port, "enirq", &l->enirq) != 0) {
        return -1;
    }
    if ((l->style == LPT_TI451) != (l->input_enable != NULL) ||
        (l->style == LPT_TI552) != (l->pemd != NULL && l->enirq != NULL)) {
        return kind_error(ctx, "\"inputEnable\" is for ti451; \"pemd\" and \"enirq\" for ti552");
    }
    kind_pop(ctx);
    d->lpt = l;
    return 0;
}

/* "absentTies": {pin: 0 or 1} for pins a package leaves out and ties
   inside; "optionalPins": [pin] that a package may simply leave out. */
static int load_absent(KindContext *ctx, const cJSON *json, NsDevice *d) {
    const cJSON *ties = cJSON_GetObjectItemCaseSensitive(json, "absentTies");
    const cJSON *item;
    size_t n = 0;

    if (ties != NULL) {
        char **names;
        int *levels;

        if (!cJSON_IsObject(ties)) {
            return kind_error(ctx, "\"absentTies\" must be an object of pin: 0 or 1");
        }
        names = kind_alloc(ctx, ((size_t)cJSON_GetArraySize(ties) + 1) * sizeof *names);
        levels = kind_alloc(ctx, ((size_t)cJSON_GetArraySize(ties) + 1) * sizeof *levels);
        if (names == NULL || levels == NULL) {
            return -1;
        }
        cJSON_ArrayForEach(item, ties) {
            if (!cJSON_IsNumber(item) || (item->valuedouble != 0 && item->valuedouble != 1)) {
                return kind_error(ctx, "\"absentTies\" must be an object of pin: 0 or 1");
            }
            names[n] = kind_strdup(ctx, item->string);
            levels[n] = item->valuedouble == 1;
            n++;
        }
        d->absent.tie_names = (const char *const *)names;
        d->absent.tie_levels = levels;
        d->absent.tie_count = n;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "optionalPins") != NULL) {
        if (kind_names(ctx, json, "optionalPins", &d->absent.optional, &d->absent.optional_count) != 0) {
            return -1;
        }
    }
    return 0;
}

/* The plan-12-D Phase 2 keys. */
int nsc_load_phase2(KindContext *ctx, const cJSON *json, NsDevice *d) {
    static const char *const EXTENDED[] = {"none", "afe", "750", "650", "752", "nsEfr",
                                           /* plan-12-I */
                                           "sc16is", "sc16is-sfr", "sc16is-dld", "xr1280",
                                           /* plan-12-E */ "ier5", "exar650", "exar854",
                                           "m752", "5g", NULL};
    static const char *const AFRS[] = {"none", "ti752", "ti2552", "ns", "exar" /* plan-12-E */, NULL};
    static const char *const FLOWS[] = {"none", "afe", "tcr", "efrTable",
                                         "exarTable", "dtrDsr" /* plan-12-E */, NULL};
    static const char *const SWFLOWS[] = {"none", "nxpAnd", "tiAnd", "exarOr" /* plan-12-E */, NULL};
    static const char *const CLEARS[] = {"iir", "lsr", "msr", "iirOrMsr", NULL};
    static const char *const SLEEPS[] = {"none", "ier4", "ier4-all",
                                          /* plan-12-E */ "ier4-exar", "ier4-exar-all", "global",
                                          NULL};
    static const char *const IRDAS[] = {"none", "mcr6", "afr1", "mcr6-exar", "dld7" /* plan-12-E */,
                                         NULL};
    static const char *const RS485S[] = {"none", "afr-dtr",
                                          /* plan-12-E */ "fctr-rts", "fctr-mcr2", "dld6",
                                          "fctr3-op1", NULL};
    static const char *const PRESCALERS[] = {"none", "mcr7", "mcr7-clksel", "clksel" /* plan-12-E */,
                                              NULL};
    static const char *const LSR_ERRORS[] = {"sticky", "head", NULL};
    int choice;
    int value;
    int present;

    if (nsc_load_choice(ctx, json, "extended", EXTENDED, EXT_NONE, &choice) != 0) {
        return -1;
    }
    d->extended = (NsExtended)choice;
    if (nsc_load_choice(ctx, json, "afr", AFRS, AFR_NONE, &choice) != 0) {
        return -1;
    }
    d->afr = (NsAfr)choice;
    if (nsc_load_byte(ctx, json, "afrReset", 0, &value) != 0) {
        return -1;
    }
    d->afr_reset = (uint8_t)value;
    if (nsc_load_optional_bool(ctx, json, "dlf", &d->dlf) != 0 ||
        nsc_load_choice(ctx, json, "autoFlow", FLOWS, FLOW_NONE, &choice) != 0) {
        return -1;
    }
    d->auto_flow = (NsAutoFlow)choice;
    if (load_pairs(ctx, json, "rtsTable", d->rts_table, &present) != 0) {
        return -1;
    }
    if ((d->auto_flow == FLOW_TABLE) != present) {
        return kind_error(ctx, "\"rtsTable\" goes with \"autoFlow\": \"efrTable\"");
    }
    if (load_pairs(ctx, json, "xoffTable", d->xoff_table, &d->has_xoff_table) != 0 ||
        nsc_load_choice(ctx, json, "swFlow", SWFLOWS, SW_NONE, &choice) != 0) {
        return -1;
    }
    d->sw_flow = (NsSwFlow)choice;
    if (nsc_load_optional_bool(ctx, json, "specialChar", &d->special_char) != 0 ||
        nsc_load_optional_bool(ctx, json, "specialNeedsNoFlow", &d->special_needs_no_flow) != 0 ||
        nsc_load_choice(ctx, json, "specialClear", CLEARS, CLEAR_IIR, &choice) != 0) {
        return -1;
    }
    d->special_clear = (NsClear)choice;
    if (nsc_load_choice(ctx, json, "ctsRtsClear", CLEARS, CLEAR_IIR, &choice) != 0) {
        return -1;
    }
    d->ctsrts_clear = (NsClear)choice;
    if (nsc_load_choice(ctx, json, "sleep", SLEEPS, SLEEP_NONE, &choice) != 0) {
        return -1;
    }
    d->sleep = (NsSleep)choice;
    if (nsc_load_choice(ctx, json, "irda", IRDAS, IRDA_NONE, &choice) != 0) {
        return -1;
    }
    d->irda = (NsIrda)choice;
    if (nsc_load_choice(ctx, json, "rs485", RS485S, RS485_NONE, &choice) != 0) {
        return -1;
    }
    d->rs485 = (NsRs485)choice;
    if (nsc_load_choice(ctx, json, "prescaler", PRESCALERS, PRESCALE_NONE, &choice) != 0) {
        return -1;
    }
    d->prescaler = (NsPrescaler)choice;
    if ((d->prescaler == PRESCALE_MCR7_CLKSEL || d->prescaler == PRESCALE_CLKSEL) !=   /* plan-12-E */
        (d->clock_select != NULL)) {
        return kind_error(ctx, "\"mcr7-clksel\" and \"clksel\" need \"clock\".\"select\" (and only they)");
    }
    if (nsc_load_choice(ctx, json, "lsrErrors", LSR_ERRORS, 0, &choice) != 0) {
        return -1;
    }
    d->lsr_head = choice == 1;
    if (nsc_load_byte(ctx, json, "mcrReset", -1, &d->mcr_reset) != 0 ||
        nsc_load_optional_bool(ctx, json, "tcrNeedsDlab0", &d->tcr_needs_dlab0) != 0 ||
        nsc_load_optional_bool(ctx, json, "dlabHidesUpper", &d->dlab_hides_upper) != 0) {
        return -1;
    }
    /* Power-up Xon1, Xon2, Xoff1, Xoff2 (absent: X). */
    d->xon_power[0] = d->xon_power[1] = d->xon_power[2] = d->xon_power[3] = -1;
    if (cJSON_GetObjectItemCaseSensitive(json, "xonReset") != NULL) {
        const cJSON *list = cJSON_GetObjectItemCaseSensitive(json, "xonReset");
        const cJSON *item;
        unsigned i = 0;

        if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) != 4) {
            return kind_error(ctx, "\"xonReset\" must be 4 bytes (Xon1, Xon2, Xoff1, Xoff2)");
        }
        cJSON_ArrayForEach(item, list) {
            if (!cJSON_IsNumber(item) || item->valuedouble < 0 || item->valuedouble > 255) {
                return kind_error(ctx, "\"xonReset\" must be 4 bytes (Xon1, Xon2, Xoff1, Xoff2)");
            }
            d->xon_power[i++] = (int)item->valuedouble;
        }
    }
    d->drev = -1;
    d->dvid = -1;
    if (cJSON_GetObjectItemCaseSensitive(json, "idRegs") != NULL) {
        static const char *const ID_KEYS[] = {"drev", "dvid", NULL};
        const cJSON *ids = cJSON_GetObjectItemCaseSensitive(json, "idRegs");

        kind_push(ctx, "idRegs");
        if (kind_check_keys(ctx, ids, ID_KEYS, NULL) != 0 ||
            nsc_load_byte(ctx, ids, "drev", -1, &d->drev) != 0 ||
            nsc_load_byte(ctx, ids, "dvid", -1, &d->dvid) != 0) {
            return -1;
        }
        kind_pop(ctx);
    }
    if (nsc_load_byte(ctx, json, "timeoutDelay", 8, &value) != 0) {
        return -1;
    }
    d->timeout_extra = (unsigned)value;
    if (nsc_load_byte(ctx, json, "divisorZero", 0, &value) != 0) {
        return -1;
    }
    d->divisor_zero = (unsigned)value;
    if (serial_optional_name(ctx, json, "tristate", &d->tri) != 0 ||
        load_printer_port(ctx, json, d) != 0) {
        return -1;
    }
    if (load_absent(ctx, json, d) != 0 ||
        nsc_load_byte(ctx, json, "sprPowerUp", -1, &d->spr_power_up) != 0 ||
        nsc_load_optional_bool(ctx, json, "lsrClearOnGood", &d->lsr_clear_on_good) != 0 ||
        nsc_load_byte(ctx, json, "breakMarkTicks", 0, &value) != 0) {
        return -1;
    }
    d->break_mark_ticks = (unsigned)value;
    if (cJSON_GetObjectItemCaseSensitive(json, "dmaMode1") != NULL) {
        int mode1;

        if (kind_bool(ctx, json, "dmaMode1", &mode1) != 0) {
            return -1;
        }
        d->no_dma_mode1 = !mode1;
    }
    if (d->lpt != NULL && d->channel_pin != NULL) {
        return kind_error(ctx, "a printer port needs per-channel or common selects");
    }
    return 0;
}

