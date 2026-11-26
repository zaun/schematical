/* models/ns16550_exar_load.c: part of the model in models/ns16550_exar.c (split by plan-68-I/J). */
#include "ns16550_exar_int.h"
/* ---- Loader --------------------------------------------------------------------- */

static NsExarDevice *xdev(KindContext *ctx, NsDevice *d) {
    if (d->x == NULL) {
        d->x = kind_alloc(ctx, sizeof(NsExarDevice));
    }
    return (NsExarDevice *)d->x;
}

static int opt_bool(KindContext *ctx, const cJSON *json, const char *key, int *out) {
    if (cJSON_GetObjectItemCaseSensitive(json, key) == NULL) {
        return 0;
    }
    return kind_bool(ctx, json, key, out);
}

/* An optional string that must be one of `values`; *out its index or -1. */
int exs_opt_choice(KindContext *ctx, const cJSON *json, const char *key,
                      const char *const *values, int *out) {
    const char *text;
    int i;

    *out = -1;
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

int ns_x_load_bus(KindContext *ctx, const cJSON *bus, NsDevice *d, const char *style) {
    NsExarDevice *x;
    const cJSON *bits;
    const char *others = NULL;

    if (strcmp(style, "intel-motorola") != 0) {
        return kind_error(ctx, "unknown bus style \"%s\"", style);
    }
    x = xdev(ctx, d);
    if (x == NULL) {
        return -1;
    }
    x->motorola_bus = 1;
    if (kind_string(ctx, bus, "modePin", &x->mode_pin) != 0 ||
        kind_string(ctx, bus, "rw", &x->rw) != 0 ||
        serial_optional_name(ctx, bus, "strobe", &x->strobe) != 0 ||
        serial_optional_name(ctx, bus, "irq", &x->irq) != 0 ||
        serial_optional_name(ctx, bus, "irqOthers", &others) != 0) {
        return -1;
    }
    if (others != NULL && strcmp(others, "low") != 0 && strcmp(others, "z") != 0) {
        return kind_error(ctx, "\"irqOthers\" must be \"low\" or \"z\"");
    }
    x->irq_others_low = others != NULL && strcmp(others, "low") == 0;
    bits = cJSON_GetObjectItemCaseSensitive(bus, "channelBits");
    if (bits != NULL) {
        const char *const *names;
        size_t n;
        size_t i;

        if (kind_names(ctx, bus, "channelBits", &names, &n) != 0) {
            return -1;
        }
        if (n > 2) {
            return kind_error(ctx, "\"channelBits\" has at most 2 pin names (A3, A4)");
        }
        for (i = 0; i < n; i++) {
            x->chan_bit[i] = names[i];
        }
        x->chan_bits = (unsigned)n;
    }
    return 0;
}

int ns_x_load_channel(KindContext *ctx, const cJSON *item, NsDevice *d, unsigned n) {
    const char *bclk = NULL;
    const char *irtx = NULL;
    NsExarDevice *x;

    if (serial_optional_name(ctx, item, "bclk", &bclk) != 0 ||
        serial_optional_name(ctx, item, "irtx", &irtx) != 0) {
        return -1;
    }
    if (bclk == NULL && irtx == NULL) {
        return 0;
    }
    x = xdev(ctx, d);
    if (x == NULL) {
        return -1;
    }
    x->bclk[n] = bclk;
    x->irtx[n] = irtx;
    return 0;
}

/* [[4 levels] x 3, "trg"]: tables A-C, then D (programmable). */
static int load_table_set(KindContext *ctx, const cJSON *obj, const char *key, unsigned max,
                          unsigned out[3][4]) {
    const cJSON *list = cJSON_GetObjectItemCaseSensitive(obj, key);
    const cJSON *row;
    unsigned i = 0;

    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) != 4) {
        return kind_error(ctx, "\"triggerTables\".\"%s\" must be 3 tables of 4 levels and \"trg\"",
                          key);
    }
    cJSON_ArrayForEach(row, list) {
        const cJSON *item;
        unsigned k = 0;

        if (i == 3) {
            if (!cJSON_IsString(row) || strcmp(row->valuestring, "trg") != 0) {
                return kind_error(ctx, "table D of \"%s\" must be \"trg\"", key);
            }
            break;
        }
        if (!cJSON_IsArray(row) || cJSON_GetArraySize(row) != 4) {
            return kind_error(ctx, "\"triggerTables\".\"%s\" tables are 4 levels", key);
        }
        cJSON_ArrayForEach(item, row) {
            if (!cJSON_IsNumber(item) || item->valuedouble < 1 || item->valuedouble > max ||
                item->valuedouble != (double)(unsigned)item->valuedouble) {
                return kind_error(ctx, "trigger levels are whole numbers from 1 to the FIFO size");
            }
            out[i][k++] = (unsigned)item->valuedouble;
        }
        i++;
    }
    return 0;
}

static int load_mf(KindContext *ctx, const cJSON *json, NsExarDevice *x) {
    static const char *const FUNCTIONS[] = {"op2", "baudout", "rxrdy", "x", NULL};
    const cJSON *list = cJSON_GetObjectItemCaseSensitive(json, "mfPins");
    const cJSON *item;
    unsigned i = 0;

    if (list == NULL) {
        return 0;
    }
    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) != 4) {
        return kind_error(ctx, "\"mfPins\" must be the 4 functions of AFR[2:1] = 00..11");
    }
    cJSON_ArrayForEach(item, list) {
        int f;

        if (!cJSON_IsString(item)) {
            return kind_error(ctx, "\"mfPins\" entries are \"op2\", \"baudout\", \"rxrdy\" or \"x\"");
        }
        for (f = 0; FUNCTIONS[f] != NULL && strcmp(FUNCTIONS[f], item->valuestring) != 0; f++) {
        }
        if (FUNCTIONS[f] == NULL) {
            return kind_error(ctx, "\"mfPins\" entries are \"op2\", \"baudout\", \"rxrdy\" or \"x\"");
        }
        x->mf[i++] = (NsMfFunction)f;
    }
    x->has_mf = 1;
    return 0;
}

/* Every Exar key, and whether the device needs the Exar code at all. */
int ns_x_load(KindContext *ctx, const cJSON *json, NsDevice *d) {
    static const char *const E_KEYS[] = {
        "triggerTables", "fifoCounters", "dld", "sampling", "xonAny", "powerSave", "mfPins",
        "startSample", "thrIntClear", "isrStatus", "resetOut", "divisorPowerUp",
        "sleepNeedsDivisor", "wakeIndicator", "mcr2Enhanced", "xonWriteOnly", "autoFlowLoopback",
        "lsrBreakHold", "statusWrite", "dlabHidesStatus", "fstat", "dmaHost", "chcClock",
        "interrupt", "timer", "variant5g", "irdaStrap", NULL
    };
    NsExarDevice *x;
    int need = d->x != NULL || d->extended >= EXT_IER5 || d->afr == AFR_EXAR ||
               d->auto_flow >= FLOW_EXAR_TABLE || d->sw_flow == SW_EXAR_OR ||
               d->sleep >= SLEEP_IER4_EXAR || d->irda >= IRDA_MCR6_EXAR ||
               d->rs485 >= RS485_FCTR_RTS || d->prescaler == PRESCALE_CLKSEL ||
               d->broadcast == BROADCAST_AFR0_PER_CHANNEL || d->address_count != 3;
    int choice;
    unsigned i;

    for (i = 0; E_KEYS[i] != NULL; i++) {
        need |= cJSON_GetObjectItemCaseSensitive(json, E_KEYS[i]) != NULL;
    }
    if (!need) {
        return 0;
    }
    x = xdev(ctx, d);
    if (x == NULL) {
        return -1;
    }
    /* By name: the NsExtended values are not contiguous (plan-12-I's bridge
       styles sit between D's and these). */
    switch (d->extended) {
        case EXT_IER5: x->style = XS_IER5; break;
        case EXT_EXAR650: x->style = XS_650; break;
        case EXT_EXAR854: x->style = XS_854; break;
        case EXT_M752: x->style = XS_M752; break;
        case EXT_5G: x->style = XS_5G; break;
        default: x->style = XS_NONE; break;
    }
    if ((d->address_count == 8) != (x->style == XS_5G)) {
        return kind_error(ctx, "\"bus\".\"address\" has 8 pin names (A0-A7) with \"5g\", else 3");
    }
    if (d->extended != EXT_NONE && d->extended < EXT_IER5) {   /* D's and I's styles */
        return kind_error(ctx, "Exar keys go with \"extended\": absent, \"ier5\", \"exar650\", "
                               "\"exar854\", \"m752\" or \"5g\"");
    }

    /* "triggerTables": {"rx": [A, B, C, "trg"], "tx": [...]} (exar854, 5g). */
    if (cJSON_GetObjectItemCaseSensitive(json, "triggerTables") != NULL) {
        static const char *const TT_KEYS[] = {"rx", "tx", NULL};
        const cJSON *tt = cJSON_GetObjectItemCaseSensitive(json, "triggerTables");

        kind_push(ctx, "triggerTables");
        if (kind_check_keys(ctx, tt, TT_KEYS, NULL) != 0 ||
            load_table_set(ctx, tt, "rx", d->fifo, x->rx_table) != 0 ||
            load_table_set(ctx, tt, "tx", d->fifo, x->tx_table) != 0) {
            return -1;
        }
        kind_pop(ctx);
        x->has_tables = 1;
    }
    if ((x->style == XS_854 || x->style == XS_5G) != x->has_tables) {
        return kind_error(ctx, "\"triggerTables\" goes with \"exar854\" and \"5g\" (and only they)");
    }
    {
        static const char *const COUNTERS[] = {"fcFlvl", NULL};

        if (exs_opt_choice(ctx, json, "fifoCounters", COUNTERS, &choice) != 0) {
            return -1;
        }
        x->fc_flvl = choice == 0;
        if (x->fc_flvl && x->style != XS_854) {
            return kind_error(ctx, "\"fifoCounters\": \"fcFlvl\" is the exar854 FC/FLVL pair");
        }
    }
    if (opt_bool(ctx, json, "dld", &x->dld) != 0) {
        return -1;
    }
    if (x->dld && x->style != XS_650 && x->style != XS_M752 && x->style != XS_5G) {
        return kind_error(ctx, "\"dld\" goes with \"exar650\", \"m752\" or \"5g\"");
    }
    {
        static const char *const SAMPLING[] = {"8xmode", NULL};

        if (exs_opt_choice(ctx, json, "sampling", SAMPLING, &choice) != 0) {
            return -1;
        }
        if ((choice == 0) != (x->style == XS_5G)) {
            return kind_error(ctx, "\"sampling\": \"8xmode\" goes with \"5g\" (and only it)");
        }
        x->dld_sampling = x->dld && x->style != XS_5G;
    }
    {
        static const char *const XON_ANY[] = {"mcr5", NULL};

        if (exs_opt_choice(ctx, json, "xonAny", XON_ANY, &choice) != 0) {
            return -1;
        }
        x->xon_any = choice == 0;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "powerSave") != NULL) {
        static const char *const PS_KEYS[] = {"pin", NULL};
        const cJSON *ps = cJSON_GetObjectItemCaseSensitive(json, "powerSave");

        kind_push(ctx, "powerSave");
        if (kind_check_keys(ctx, ps, PS_KEYS, NULL) != 0 ||
            kind_string(ctx, ps, "pin", &x->pwrsave) != 0) {
            return -1;
        }
        kind_pop(ctx);
    }
    if (load_mf(ctx, json, x) != 0) {
        return -1;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "startSample") != NULL) {
        const cJSON *ss = cJSON_GetObjectItemCaseSensitive(json, "startSample");

        if (!cJSON_IsNumber(ss) || (ss->valuedouble != 7.5 && ss->valuedouble != 8)) {
            return kind_error(ctx, "\"startSample\" must be 7.5 or 8");
        }
        x->start_75 = ss->valuedouble == 7.5;
    }
    {
        static const char *const THR[] = {"standard", "ier1Rewrite", NULL};
        static const char *const ISR[] = {"high", "inverted", NULL};

        if (exs_opt_choice(ctx, json, "thrIntClear", THR, &choice) != 0) {
            return -1;
        }
        x->thr_ier1 = choice == 1;
        if (exs_opt_choice(ctx, json, "isrStatus", ISR, &choice) != 0) {
            return -1;
        }
        x->isr_inverted = choice == 1;
        if (choice >= 0 && x->style != XS_IER5) {
            return kind_error(ctx, "\"isrStatus\" goes with \"ier5\"");
        }
    }
    if (serial_optional_name(ctx, json, "resetOut", &x->reset_out) != 0) {
        return -1;
    }
    x->divisor_power_up = -1;
    if (cJSON_GetObjectItemCaseSensitive(json, "divisorPowerUp") != NULL) {
        uint64_t v;

        if (kind_uint(ctx, json, "divisorPowerUp", 255, &v) != 0) {
            return -1;
        }
        x->divisor_power_up = (int)v;
    }
    {
        int status_write = 1;

        if (opt_bool(ctx, json, "sleepNeedsDivisor", &x->sleep_divisor) != 0 ||
            opt_bool(ctx, json, "wakeIndicator", &x->wake_indicator) != 0 ||
            opt_bool(ctx, json, "mcr2Enhanced", &x->mcr2_enhanced) != 0 ||
            opt_bool(ctx, json, "xonWriteOnly", &x->xon_write_only) != 0 ||
            opt_bool(ctx, json, "autoFlowLoopback", &x->flow_loopback) != 0 ||
            opt_bool(ctx, json, "lsrBreakHold", &x->break_hold) != 0 ||
            opt_bool(ctx, json, "statusWrite", &status_write) != 0 ||
            opt_bool(ctx, json, "dlabHidesStatus", &x->dlab_hides_status) != 0) {
            return -1;
        }
        x->status_read_only = !status_write;
    }
    if (serial_optional_name(ctx, json, "fstat", &x->fstat) != 0 ||
        serial_optional_name(ctx, json, "chcClock", &x->chc_clock) != 0 ||
        serial_optional_name(ctx, json, "interrupt", &x->interrupt) != 0) {
        return -1;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "dmaHost") != NULL) {
        static const char *const DH_KEYS[] = {"dack", "aen", "tc", NULL};
        const cJSON *dh = cJSON_GetObjectItemCaseSensitive(json, "dmaHost");
        const char *const *names;
        size_t n;

        kind_push(ctx, "dmaHost");
        if (kind_check_keys(ctx, dh, DH_KEYS, NULL) != 0 ||
            kind_names(ctx, dh, "dack", &names, &n) != 0 ||
            kind_string(ctx, dh, "aen", &x->aen) != 0 ||
            kind_string(ctx, dh, "tc", &x->tc) != 0) {
            return -1;
        }
        if (n != d->channel_count || n > 4) {
            return kind_error(ctx, "\"dack\" names one DACK pin per channel");
        }
        for (i = 0; i < n; i++) {
            x->dack[i] = names[i];
        }
        kind_pop(ctx);
        x->dma_host = 1;
        if (d->dma_class != 'X') {
            return kind_error(ctx, "\"dmaHost\" goes with \"dmaClass\": \"X\"");
        }
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "timer") != NULL) {
        static const char *const T_KEYS[] = {"clock", "style", NULL};
        static const char *const T_STYLES[] = {"commands", "bits", NULL};
        const cJSON *t = cJSON_GetObjectItemCaseSensitive(json, "timer");

        kind_push(ctx, "timer");
        if (kind_check_keys(ctx, t, T_KEYS, NULL) != 0 ||
            kind_string(ctx, t, "clock", &x->tmrck) != 0 ||
            exs_opt_choice(ctx, t, "style", T_STYLES, &choice) != 0) {
            return -1;
        }
        if (choice < 0) {
            return kind_error(ctx, "missing \"style\"");
        }
        x->timer_commands = choice == 0;
        kind_pop(ctx);
    }
    {
        static const char *const VARIANTS[] = {"xr16v798", "xr16l784", NULL};

        if (exs_opt_choice(ctx, json, "variant5g", VARIANTS, &choice) != 0) {
            return -1;
        }
        x->v798 = choice == 0;
        if ((choice >= 0) != (x->style == XS_5G) ||
            (x->style == XS_5G && (x->interrupt == NULL || x->tmrck == NULL))) {
            return kind_error(ctx, "\"5g\" needs \"variant5g\", \"interrupt\" and \"timer\"");
        }
    }
    if (d->irda == IRDA_MCR6_EXAR) {
        x->irda_polarity = x->style != XS_854 && x->style != XS_5G;
    }
    /* "irdaStrap": the 5G ENIR pin, sampled into MCR[6] at power-up and reset. */
    if (serial_optional_name(ctx, json, "irdaStrap", &x->enir) != 0) {
        return -1;
    }
    if ((d->irda == IRDA_DLD7 || d->rs485 == RS485_DLD6) && !(x->dld && x->style == XS_M752)) {
        return kind_error(ctx, "\"dld7\" / \"dld6\" go with \"m752\" and \"dld\"");
    }
    if (d->afr == AFR_EXAR && x->style != XS_650) {
        return kind_error(ctx, "\"afr\": \"exar\" goes with \"exar650\"");
    }
    x->afr0_per_channel = d->broadcast == BROADCAST_AFR0_PER_CHANNEL;
    if (x->afr0_per_channel && d->afr != AFR_EXAR) {
        return kind_error(ctx, "\"afr0-perChannel\" needs \"afr\": \"exar\"");
    }
    return 0;
}

