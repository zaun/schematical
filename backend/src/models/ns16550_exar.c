/*
 * ns16550 Exar/MaxLinear extensions (plan-12-E): the ST16C/XR16C/XR16L/
 * XR16M/XR16V devices. Every ns_ext_* hook of ns16550_ext.c hands a device
 * with Exar keys (dev->x) to its ns_x_* twin here, and the core calls the
 * other ns_x_* hooks directly. Styles ("extended"):
 *
 *   absent    16550 core plus the Exar keys below (ST16C450/550/2550/554,
 *             XR16C2550, XR16V554)
 *   "ier5"    ST16C1450/1550: IER[5] special mode (ISR[5:4] status, MCR[2] to
 *             the RST pin, MCR[7] power-down, FCR[5:4] TX trigger)
 *   "exar650" the LCR = 0xBF bank (EFR, Xon1/2, Xoff1/2) with Exar's DLD, AFR
 *             and DREV/DVID (XR16L580/2551/2552, XR16M25xx/26xx,
 *             XR16V25xx/26xx, XR16V564/654)
 *   "exar854" the XR16C854/864 bank: FC/TRG, FCTR, EFR, Xon/Xoff; FLVL/EMSR
 *   "m752"    XR16M752: the 752 bank (TCR, TLR, FIFO Rdy) plus DLD
 *   "5g"      XR16V798/XR16L784: the flat A7..A0 map with the global block
 *
 * Sources: plan-12-data/specs/exar-a.md, exar-b.md, exar-c.md, exar-d.md,
 * with decisions.md and plan-12-E-ns16550-exar.md (Corrections).
 *
 * Exar TX trigger levels count the bytes in the TX FIFO: the THR interrupt
 * comes when the count falls below the level, or when the FIFO empties if it
 * never reached the level since the last reload. The core works in free
 * spaces, so ns_x_tx_threshold returns depth - level + 1 (count < level is
 * free >= depth - level + 1).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ns16550_int.h"

/* ---- Small helpers -------------------------------------------------------------- */

static const NsExarDevice *xd(const NsChannel *c) {
    return c->owner->dev->x;
}

static NsExarStyle style_of(const NsChannel *c) {
    return c->owner->dev->x->style;
}

static int kb(Reg r, unsigned bit) {
    return bit_of(r, bit) == LOGIC_1;
}

static int lcr_bf(const NsChannel *c) {
    return c->lcr.k == 0xFF && c->lcr.v == 0xBF;
}

static uint8_t word_mask(const NsChannel *c) {
    return (uint8_t)((1u << (5 + (c->lcr.v & 3u))) - 1u);
}

static int has_bank(const NsChannel *c) {
    NsExarStyle st = style_of(c);

    return st == XS_650 || st == XS_854 || st == XS_M752 || st == XS_5G;
}

/* The enhanced-bit gate: IER[5] (ier5) or EFR[4]. */
static int enhanced(const NsChannel *c) {
    switch (style_of(c)) {
        case XS_IER5: return kb(c->ier, 5);
        case XS_650:
        case XS_854:
        case XS_M752:
        case XS_5G: return kb(c->efr, 4);
        default: return 0;
    }
}

static int loopback(const NsChannel *c) {
    return kb(c->mcr, 4);
}

static unsigned tx_free(const NsChannel *c) {
    return depth(c) - c->tx_count;
}

static int mode1(const NsChannel *c) {
    return c->fifo_on && c->dma_mode;
}

/* The active-low RXRDY/TXRDY levels (core rules for classes N, B, T, P). */
static LogicValue rxrdy_pin(const NsChannel *c) {
    if (!mode1(c)) {
        return bool3(c->rx_count == 0);
    }
    if (c->owner->dev->dma_class == 'T' && c->lsr7) {
        return LOGIC_1;
    }
    return bool3(!c->rxrdy1);
}

static LogicValue txrdy_pin(const NsChannel *c) {
    unsigned threshold;

    if (!mode1(c)) {
        return bool3(c->tx_count != 0);
    }
    switch (c->owner->dev->dma_class) {
        case 'N':
        case 'B':
            return bool3(tx_free(c) == 0);
        case 'T':
            threshold = ns_ext_tx_threshold(c);
            return bool3(tx_free(c) < (threshold != 0 ? threshold : 1));
        default:
            return bool3(!c->txrdy1);
    }
}

static void set_format_rate(NsChannel *c) {
    c->format.factor = ns_x_factor(c);
    c->format.sample = ns_x_start_sample(c, c->format.factor);
}

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
static int opt_choice(KindContext *ctx, const cJSON *json, const char *key,
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
    static const NsExarStyle STYLES[] = {XS_NONE, XS_NONE, XS_NONE, XS_NONE, XS_NONE, XS_NONE,
                                         XS_IER5, XS_650, XS_854, XS_M752, XS_5G};
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
    x->style = STYLES[d->extended];
    if ((d->address_count == 8) != (x->style == XS_5G)) {
        return kind_error(ctx, "\"bus\".\"address\" has 8 pin names (A0-A7) with \"5g\", else 3");
    }
    if (d->extended != EXT_NONE && d->extended < EXT_IER5) {
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

        if (opt_choice(ctx, json, "fifoCounters", COUNTERS, &choice) != 0) {
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

        if (opt_choice(ctx, json, "sampling", SAMPLING, &choice) != 0) {
            return -1;
        }
        if ((choice == 0) != (x->style == XS_5G)) {
            return kind_error(ctx, "\"sampling\": \"8xmode\" goes with \"5g\" (and only it)");
        }
        x->dld_sampling = x->dld && x->style != XS_5G;
    }
    {
        static const char *const XON_ANY[] = {"mcr5", NULL};

        if (opt_choice(ctx, json, "xonAny", XON_ANY, &choice) != 0) {
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

        if (opt_choice(ctx, json, "thrIntClear", THR, &choice) != 0) {
            return -1;
        }
        x->thr_ier1 = choice == 1;
        if (opt_choice(ctx, json, "isrStatus", ISR, &choice) != 0) {
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
            opt_choice(ctx, t, "style", T_STYLES, &choice) != 0) {
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

        if (opt_choice(ctx, json, "variant5g", VARIANTS, &choice) != 0) {
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

/* ---- Binding and part options ------------------------------------------------------ */

int ns_x_bind(KindContext *ctx, const NsDevice *d, const PartPin *pins, size_t pin_count,
              NsPins *p, const SerialAbsent *absent) {
    const NsExarDevice *x = d->x;
    NsExarPins *xp = kind_alloc(ctx, sizeof *xp);
    char key[32];
    unsigned i;

    if (xp == NULL) {
        return -1;
    }
#define BIND(k, name, slot) \
    if (serial_bind_pin_absent(ctx, pins, pin_count, k, name, absent, slot) != 0) return -1
    BIND("bus.modePin", x->mode_pin, &xp->mode);
    BIND("bus.rw", x->rw, &xp->rw);
    BIND("bus.strobe", x->strobe, &xp->strobe);
    BIND("bus.channelBits", x->chan_bit[0], &xp->chan_bit[0]);
    BIND("bus.channelBits", x->chan_bit[1], &xp->chan_bit[1]);
    BIND("bus.irq", x->irq, &xp->irq);
    BIND("interrupt", x->interrupt, &xp->interrupt);
    BIND("resetOut", x->reset_out, &xp->reset_out);
    BIND("powerSave.pin", x->pwrsave, &xp->pwrsave);
    BIND("fstat", x->fstat, &xp->fstat);
    BIND("dmaHost.aen", x->aen, &xp->aen);
    BIND("dmaHost.tc", x->tc, &xp->tc);
    BIND("chcClock", x->chc_clock, &xp->chc_clock);
    BIND("irdaStrap", x->enir, &xp->enir);
    BIND("timer.clock", x->tmrck, &xp->tmrck);
    for (i = 0; i < 4; i++) {
        BIND("dmaHost.dack", x->dack[i], &xp->dack[i]);
    }
    for (i = 0; i < 8; i++) {
        snprintf(key, sizeof key, "channels[%u].bclk", i);
        BIND(key, x->bclk[i], &xp->bclk[i]);
        snprintf(key, sizeof key, "channels[%u].irtx", i);
        BIND(key, x->irtx[i], &xp->irtx[i]);
    }
#undef BIND
    /* With no strobe named, Motorola's CS is the first chip select. */
    if (xp->strobe == SERIAL_NO_PIN && x->motorola_bus) {
        xp->strobe = d->select_count > 0 ? p->bus.select[0]
                   : d->chan_select_count[0] > 0 ? p->chan_select[0][0] : SERIAL_NO_PIN;
    }
    p->xp = xp;
    return 0;
}

/* Part key "intPin": the D variants (INTSEL bonded high) drive INT always. */
const void *ns_x_load_part(KindContext *ctx, const void *data, const cJSON *json) {
    static const char *const INT_PINS[] = {
        "driven", "out2-gated", "intn", "intsel", "mcr5-open-source", NULL
    };
    NsExarPart *part;
    int choice;

    (void)data;
    if (opt_choice(ctx, json, "intPin", INT_PINS, &choice) != 0 || choice < 0) {
        return NULL;
    }
    if (choice == INT_INTN || choice == INT_INTSEL || choice == INT_MCR5_OPEN_SOURCE) {
        kind_error(ctx, "part \"intPin\" must be \"driven\" or \"out2-gated\"");
        return NULL;
    }
    part = kind_alloc(ctx, sizeof *part);
    if (part != NULL) {
        part->int_pin = choice;
    }
    return part;
}

int ns_x_open_drain(const void *data, const void *pins, const void *options, size_t pin) {
    const NsDevice *d = data;
    const NsPins *p = pins;

    (void)options;
    return d->x != NULL && p->xp != NULL && p->xp->interrupt == pin && pin != SERIAL_NO_PIN;
}

/* ---- Reset ------------------------------------------------------------------------- */

static void sample_straps(NsState *s, NsChannel *c) {
    const NsExarDevice *x = s->dev->x;

    if (s->sim == NULL) {
        return;
    }
    /* CLKSEL: GND = ÷4, VCC = ÷1 (absent = ÷1); MCR[7] still reads 0 (P1). */
    if (s->dev->prescaler == PRESCALE_CLKSEL) {
        LogicValue clksel = serial_level(s->sim, s->instance, s->pins->clock_select, LOGIC_1);

        c->xc.prescale4 = clksel == LOGIC_0;
    }
    /* 5G ENIR: MCR[6] (IrDA) preset. */
    if (x->enir != NULL) {
        LogicValue enir = serial_level(s->sim, s->instance, s->pins->xp->enir, LOGIC_X);

        c->mcr.v = (uint8_t)((c->mcr.v & ~0x40u) | (enir == LOGIC_1 ? 0x40u : 0));
        c->mcr.k = (uint8_t)((c->mcr.k & ~0x40u) | (enir == LOGIC_X ? 0 : 0x40u));
    }
}

static void reset_globals(NsState *s) {
    NsExarState *g = &s->xs;

    g->mode8x = 0;
    g->sleep = 0;
    g->regb = 0;
    g->timer_reload = 0;
    g->timer_count = 0;
    g->timer_running = 0;
    g->timer_oneshot = 0;
    g->timer_ext = 0;
    g->timer_int_en = 0;
    g->timer_int = LOGIC_0;
    g->timercntl = 0;
}

void ns_x_reset(NsState *s, NsChannel *c, int power_on) {
    const NsDevice *d = s->dev;
    const NsExarDevice *x = d->x;
    NsExarChannel *xc = &c->xc;

    c->efr = (Reg){0, 0xFF};
    c->tcr = (Reg){d->tcr_reset, 0xFF};
    c->tlr = (Reg){0, 0xFF};
    c->afr = (Reg){d->afr_reset, 0xFF};
    /* Xon/Xoff: reset to 0 (Exar reset tables), the XR16M752's at power-up only. */
    if (x->style != XS_M752 || power_on) {
        Reg *xon[4] = {&c->xon1, &c->xon2, &c->xoff1, &c->xoff2};
        unsigned i;

        for (i = 0; i < 4; i++) {
            *xon[i] = x->style == XS_M752 && d->xon_power[i] < 0 ? (Reg){0, 0}
                    : (Reg){(uint8_t)(d->xon_power[i] >= 0 ? d->xon_power[i] : 0), 0xFF};
        }
    }
    if (d->mcr_reset >= 0) {
        c->mcr = (Reg){(uint8_t)d->mcr_reset, 0xFF};
    }
    if (power_on && x->divisor_power_up >= 0) {
        c->dll = (Reg){(uint8_t)x->divisor_power_up, 0xFF};
        c->dlm = (Reg){0, 0xFF};
    }
    /* XR16V798: RST# and the soft reset set the divisor to 1 (Table 19). */
    if (x->style == XS_5G && x->v798) {
        c->dll = (Reg){1, 0xFF};
        c->dlm = (Reg){0, 0xFF};
    }
    c->tx_trigger = 0;
    c->thr_level_int = 0;
    c->tx_halted = 0;
    c->xoff_sent = 0;
    c->flow_out_count = 0;
    c->seq_wait = 0;
    c->iir_xoff = 0;
    c->iir_special = 0;
    c->iir_ctsrts = 0;
    c->rts_off = 0;
    c->rts_pin_last = LOGIC_X;
    c->cts_last = LOGIC_X;
    c->asleep = 0;
    c->low_power = 0;
    c->ir_low = 0;
    c->ir_last = LOGIC_X;
    c->rs485_on = 0;
    c->rs485_lead = 0;
    c->rs485_hold = 0;
    c->prescale_count = 0;
    xc->dld = (Reg){0, 0xFF};
    xc->fctr = (Reg){0, 0xFF};
    xc->emsr = (Reg){0, 0xFF};
    xc->msrw = (Reg){0, 0xFF};
    xc->xchar = (Reg){0, 0xFF};
    xc->trg_rx = 0;
    xc->trg_tx = 0;
    xc->trg_rx_bad = x->style == XS_854;   /* TRG = 0: X when table D is chosen */
    xc->trg_tx_bad = x->style == XS_854;
    xc->prescale4 = 0;
    xc->filled = 0;
    xc->xoff_delay = 0;
    xc->wake_int = 0;
    xc->ls_new = 0;
    xc->powered_down = 0;
    xc->rs485 = 0;
    xc->rs485_hold = 0;
    xc->imm_armed = 0;
    xc->imm_valid = 0;
    xc->flvl_tx_next = 0;
    xc->rx_allow = 0;
    xc->drq_tx = 0;
    xc->drq_rx = 0;
    xc->tc_tx = 0;
    xc->tc_rx = 0;
    xc->dsr_last = LOGIC_X;
    sample_straps(s, c);
    if (c->index == 0 && !s->xs.soft) {
        reset_globals(s);
        s->xs.isolated = 0;
    }
    ns_x_load_divisor(c);
}

/* ---- Register decode --------------------------------------------------------------- */

typedef enum {
    R_CORE, R_X, R_EFR, R_XON1, R_XON2, R_XOFF1, R_XOFF2, R_DLD, R_AFR, R_DREV, R_DVID, R_TCR,
    R_TLR, R_FIFORDY, R_FCTR, R_FC, R_TRG, R_FLVL, R_EMSR, R_TXCNT, R_TXTRG, R_RXCNT, R_RXTRG,
    R_XCHAR, R_MSRW, R_THR_IMM
} XReg;

/* The 0xBF bank could be selected: LCR's known bits all agree with 0xBF. */
static int bf_possible(const NsChannel *c) {
    return ((c->lcr.v ^ 0xBFu) & c->lcr.k) == 0;
}

static int dl_zero(const NsChannel *c) {
    return c->dll.k == 0xFF && c->dlm.k == 0xFF && c->dll.v == 0 && c->dlm.v == 0;
}

static XReg decode_bf(const NsChannel *c, unsigned a, int write) {
    const NsExarDevice *x = xd(c);

    switch (a) {
        case 0:
            if (x->style == XS_854) {
                return write ? R_TRG : R_FC;
            }
            return R_X;   /* decisions.md: 000/001 at 0xBF: X, writes ignored (P2) */
        case 1:
            return x->style == XS_854 ? R_FCTR : R_X;
        case 2:
            return R_EFR;
        case 3:
            return R_CORE;
        default:
            if (!write && x->xon_write_only) {
                return R_CORE;   /* XR16L580: the reads follow the normal bank */
            }
            return (XReg)(R_XON1 + (a - 4));
    }
}

/* 650 / 854 / m752 styles and the plain style, by LCR, EFR[4] and MCR. */
static XReg decode_bank(const NsChannel *c, unsigned a, int write) {
    const NsDevice *d = c->owner->dev;
    const NsExarDevice *x = d->x;
    LogicValue lcr7 = bit_of(c->lcr, 7);
    LogicValue efr4 = bit_of(c->efr, 4);

    if (has_bank(c) && lcr_bf(c)) {
        return decode_bf(c, a, write);
    }
    if (has_bank(c) && c->lcr.k != 0xFF && bf_possible(c) && a != 3) {
        return R_X;   /* could be the 0xBF bank or not */
    }
    if (lcr7 == LOGIC_X && a != 3) {
        /* The core reads X for DLL/DLM vs RBR/IER; the rest is X where LCR[7]
           changes the register. */
        if (a <= 1) {
            return R_CORE;
        }
        if ((a == 2 && (d->afr == AFR_EXAR || (x->dld && has_bank(c)))) || d->dlab_hides_upper ||
            ((a == 5 || a == 6) && x->dlab_hides_status && !write)) {
            return R_X;
        }
        lcr7 = LOGIC_0;
    }
    if (lcr7 == LOGIC_1) {
        switch (a) {
            case 0:
                return !write && d->drev >= 0 && dl_zero(c) ? R_DREV : R_CORE;
            case 1:
                return !write && d->dvid >= 0 && dl_zero(c) ? R_DVID : R_CORE;
            case 2:
                if (x->dld && has_bank(c)) {
                    if (efr4 == LOGIC_X) {
                        return R_X;
                    }
                    if (efr4 == LOGIC_1) {
                        return R_DLD;
                    }
                }
                if (d->afr == AFR_EXAR) {
                    return R_AFR;
                }
                return d->dlab_hides_upper ? R_X : R_CORE;
            case 3:
                return R_CORE;
            default:
                if (d->dlab_hides_upper) {
                    return R_X;
                }
                if (!write && (a == 5 || a == 6) && x->dlab_hides_status) {
                    return R_X;
                }
                break;
        }
    }
    if (write && (a == 5 || a == 6) && x->status_read_only) {
        return R_X;
    }
    if (x->style == XS_854 && a == 7 && lcr7 == LOGIC_0) {
        LogicValue swap = bit_of(c->xc.fctr, 6);

        if (swap == LOGIC_X) {
            return R_X;
        }
        if (swap == LOGIC_1) {
            return write ? R_EMSR : R_FLVL;
        }
    }
    if (x->style == XS_M752 && (a == 6 || a == 7)) {
        LogicValue tcr = and3(efr4, bit_of(c->mcr, 6));
        LogicValue rdy = and3(bit_of(c->mcr, 2), not3(bit_of(c->mcr, 4)));

        if (a == 7 && !write) {
            /* FIFO Rdy wins over SPR and TLR (Table 12). */
            if (rdy == LOGIC_1) {
                return R_FIFORDY;
            }
            if (rdy == LOGIC_X) {
                return R_X;
            }
        }
        if (tcr == LOGIC_X) {
            return R_X;
        }
        if (tcr == LOGIC_1) {
            return a == 6 ? R_TCR : R_TLR;
        }
    }
    return R_CORE;
}

/* The 5G channel map (A3..A0); no 0xBF bank. */
static XReg decode_5g(const NsChannel *c, unsigned address, int write) {
    const NsExarDevice *x = xd(c);
    unsigned a = address & 0x0Fu;
    LogicValue lcr7 = bit_of(c->lcr, 7);

    switch (a) {
        case 0:
            if (write && lcr7 == LOGIC_0 && c->xc.imm_armed) {
                return R_THR_IMM;
            }
            return R_CORE;
        case 2:
            if (x->dld) {
                if (lcr7 == LOGIC_X) {
                    return R_X;
                }
                if (lcr7 == LOGIC_1) {
                    return R_DLD;
                }
            }
            return R_CORE;
        case 5:
            return write ? R_X : R_CORE;   /* LSR read-only (decisions.md) */
        case 6:
            return write ? R_MSRW : R_CORE;
        case 8: return R_FCTR;
        case 9: return R_EFR;
        case 10: return write ? R_TXTRG : R_TXCNT;
        case 11: return write ? R_RXTRG : R_RXCNT;
        case 12: return write ? R_XOFF1 : R_XCHAR;
        case 13: return write ? R_XOFF2 : R_X;
        case 14: return write ? R_XON1 : R_X;
        case 15: return write ? R_XON2 : R_X;
        default:
            return R_CORE;
    }
}

static XReg decode(const NsChannel *c, unsigned address, int write) {
    switch (style_of(c)) {
        case XS_5G: return decode_5g(c, address, write);
        default: return decode_bank(c, address & 7u, write);
    }
}

/* ---- Register values ----------------------------------------------------------------- */

/* M752 FIFO Rdy: 1 = ready = NOT the TXRDYx/RXRDYx pin (decisions.md). */
static uint8_t fifo_ready(const NsState *s) {
    uint8_t v = 0;
    unsigned i;

    for (i = 0; i < s->dev->channel_count && i < 4; i++) {
        if (txrdy_pin(&s->ch[i]) == LOGIC_0) {
            v |= (uint8_t)(1u << i);
        }
        if (rxrdy_pin(&s->ch[i]) == LOGIC_0) {
            v |= (uint8_t)(0x10u << i);
        }
    }
    return v;
}

int ns_x_peek(NsChannel *c, unsigned address, uint8_t *v, uint8_t *k) {
    const NsDevice *d = c->owner->dev;
    NsExarChannel *xc = &c->xc;

    switch (decode(c, address, 0)) {
        case R_CORE:
        case R_THR_IMM:
            return 0;
        case R_EFR: *v = c->efr.v; *k = c->efr.k; return 1;
        case R_XON1: *v = c->xon1.v; *k = c->xon1.k; return 1;
        case R_XON2: *v = c->xon2.v; *k = c->xon2.k; return 1;
        case R_XOFF1: *v = c->xoff1.v; *k = c->xoff1.k; return 1;
        case R_XOFF2: *v = c->xoff2.v; *k = c->xoff2.k; return 1;
        case R_DLD:
            *v = xc->dld.v;
            *k = xc->dld.k;
            if (style_of(c) == XS_5G) {
                *v &= 0x0F;   /* b7-4 read 0 */
                *k |= 0xF0;
            }
            return 1;
        case R_AFR: *v = c->afr.v & 0x07; *k = (uint8_t)(c->afr.k | 0xF8); return 1;
        case R_DREV: *v = (uint8_t)d->drev; *k = 0xFF; return 1;
        case R_DVID: *v = (uint8_t)d->dvid; *k = 0xFF; return 1;
        case R_TCR: *v = c->tcr.v; *k = c->tcr.k; return 1;
        case R_TLR: *v = c->tlr.v; *k = c->tlr.k; return 1;
        case R_FIFORDY: *v = fifo_ready(c->owner); *k = 0xFF; return 1;
        case R_FCTR: *v = xc->fctr.v; *k = xc->fctr.k; return 1;
        case R_FC:
            /* FC: RX count (FCTR[7] = 0) or TX count. */
            if (bit_of(xc->fctr, 7) == LOGIC_X) {
                *v = 0; *k = 0;
            } else {
                *v = (uint8_t)(kb(xc->fctr, 7) ? c->tx_count : c->rx_count);
                *k = 0xFF;
            }
            return 1;
        case R_FLVL: {
            unsigned mode = xc->emsr.v & 3u;
            int tx = mode == 1 || (mode == 3 && xc->flvl_tx_next);

            if ((xc->emsr.k & 3u) != 3u) {
                *v = 0; *k = 0;
            } else {
                *v = (uint8_t)(tx ? c->tx_count : c->rx_count);
                *k = 0xFF;
            }
            return 1;
        }
        case R_TXCNT: *v = (uint8_t)c->tx_count; *k = 0xFF; return 1;
        case R_RXCNT: *v = (uint8_t)c->rx_count; *k = 0xFF; return 1;
        case R_XCHAR: *v = xc->xchar.v; *k = xc->xchar.k; return 1;
        case R_X:
        case R_TRG:
        case R_EMSR:
        case R_TXTRG:
        case R_RXTRG:
        case R_MSRW:
            *v = 0; *k = 0;
            return 1;
    }
    return 0;
}

/* A read of one of the registers above ended. */
void ns_x_read_done(NsChannel *c, unsigned address) {
    NsExarChannel *xc = &c->xc;

    switch (decode(c, address, 0)) {
        case R_FLVL:
            if ((xc->emsr.v & 3u) == 3u) {
                xc->flvl_tx_next = !xc->flvl_tx_next;   /* alternate RX, TX, RX, ... */
            }
            break;
        case R_XCHAR:
            xc->xchar.v = 0;   /* each bit clears once read */
            break;
        default:
            break;
    }
}

static void queue_flow(NsChannel *c, uint8_t ch) {
    if (c->flow_out_count < sizeof c->flow_out) {
        c->flow_out[c->flow_out_count++] = ch;
    }
}

/* Xon (1) or Xoff (0) for a TX mode EFR[3:2]; 5G XCHAR b3/b2 note it. */
static void queue_flow_pair(NsChannel *c, unsigned txmode, int xon) {
    Reg first = xon ? c->xon1 : c->xoff1;
    Reg second = xon ? c->xon2 : c->xoff2;

    switch (txmode) {
        case 2: queue_flow(c, first.v); break;
        case 1: queue_flow(c, second.v); break;
        case 3: queue_flow(c, first.v); queue_flow(c, second.v); break;
        default: return;
    }
    if (style_of(c) == XS_5G && xd(c)->v798) {
        c->xc.xchar.v = (uint8_t)((c->xc.xchar.v & ~0x0Cu) | (xon ? 0x08u : 0x04u));
    }
}

int ns_x_write(NsState *s, NsChannel *c, unsigned address, uint8_t v, uint8_t k, Cause *cause) {
    const NsDevice *d = s->dev;
    const NsExarDevice *x = d->x;
    NsExarChannel *xc = &c->xc;
    unsigned i;

    *cause = CAUSE_WRITE;
    switch (decode(c, address, 1)) {
        case R_CORE:
            return 0;
        case R_X:
        case R_FIFORDY:
        case R_DREV:
        case R_DVID:
        case R_FC:
        case R_FLVL:
        case R_TXCNT:
        case R_RXCNT:
        case R_XCHAR:
            return 1;
        case R_EFR: {
            unsigned old_tx = (c->efr.v >> 2) & 3u;

            c->efr = (Reg){v, k};
            if (c->xoff_sent && ((v >> 2) & 3u) != old_tx) {
                queue_flow_pair(c, old_tx, 1);   /* the owed Xon still goes out */
                c->xoff_sent = 0;
            }
            if ((v & 0x0Fu) == 0) {
                xc->xchar.v = 0;          /* 5G: cleared when flow control is off */
                xc->xoff_delay = 0;
                c->tx_halted = 0;
            }
            ns_x_levels(s, c);
            return 1;
        }
        case R_XON1: c->xon1 = (Reg){v, k}; return 1;
        case R_XON2: c->xon2 = (Reg){v, k}; return 1;
        case R_XOFF1: c->xoff1 = (Reg){v, k}; return 1;
        case R_XOFF2: c->xoff2 = (Reg){v, k}; return 1;
        case R_DLD:
            xc->dld = (Reg){v, k};
            if (x->style == XS_5G) {
                xc->dld.v &= 0x0F;
                xc->dld.k |= 0xF0;
            }
            ns_x_load_divisor(c);
            set_format_rate(c);
            return 1;
        case R_AFR:
            c->afr = (Reg){(uint8_t)(v & 0x07), (uint8_t)(k | 0xF8)};
            if (!x->afr0_per_channel && (k & 1u)) {
                /* One concurrent-write bit, set or cleared from either channel. */
                s->conc = v & 1u;
                for (i = 0; i < d->channel_count; i++) {
                    s->ch[i].afr.v = (uint8_t)((s->ch[i].afr.v & ~1u) | (v & 1u));
                }
            }
            *cause = CAUSE_MCR_WRITE;   /* the MF pins follow */
            return 1;
        case R_TCR: c->tcr = (Reg){v, k}; ns_x_levels(s, c); return 1;
        case R_TLR: c->tlr = (Reg){v, k}; ns_x_levels(s, c); return 1;
        case R_FCTR:
            xc->fctr = (Reg){v, k};
            ns_x_levels(s, c);
            return 1;
        case R_TRG:
            /* TRG: table D's RX (FCTR[7] = 0) or TX level, 1-128 (decisions.md). */
            if (bit_of(xc->fctr, 7) == LOGIC_1) {
                xc->trg_tx = v;
                xc->trg_tx_bad = k != 0xFF || v == 0 || v > 128;
            } else if (bit_of(xc->fctr, 7) == LOGIC_0) {
                xc->trg_rx = v;
                xc->trg_rx_bad = k != 0xFF || v == 0 || v > 128;
            } else {
                xc->trg_rx_bad = 1;
                xc->trg_tx_bad = 1;
            }
            ns_x_levels(s, c);
            return 1;
        case R_EMSR:
            xc->emsr = (Reg){v, k};
            xc->flvl_tx_next = 0;   /* the first FLVL read after an EMSR write is RX */
            *cause = CAUSE_MCR_WRITE;
            return 1;
        case R_TXTRG:
            xc->trg_tx = v > 64 ? 64 : v;
            xc->trg_tx_bad = k != 0xFF;
            return 1;
        case R_RXTRG:
            xc->trg_rx = v > 64 ? 64 : v;
            xc->trg_rx_bad = k != 0xFF;
            ns_x_levels(s, c);
            return 1;
        case R_MSRW: {
            /* 5G MSR write: [7:4] RS-485 delay, [3] TX disable, [2] RX disable
               (XR16V798; the XR16L784 has [7:4] only); EFR[4] enables it. */
            uint8_t mask = x->v798 ? 0xFC : 0xF0;
            int rx_was = kb(xc->msrw, 2);

            if (!enhanced(c)) {
                return 1;
            }
            xc->msrw = (Reg){(uint8_t)((xc->msrw.v & ~mask) | (v & mask)),
                             (uint8_t)((xc->msrw.k & ~mask) | (k & mask))};
            if (!rx_was && kb(xc->msrw, 2)) {
                /* RX disable: the character in progress finishes; idle, one more comes in. */
                xc->rx_allow = serial_rx_busy(&c->line.rx) || c->pending_count > 0 ? 0 : 1;
            }
            return 1;
        }
        case R_THR_IMM:
            /* Send-immediate: this byte goes out ahead of the FIFO. */
            xc->imm_armed = 0;
            xc->imm_valid = 1;
            xc->imm_data = v;
            xc->imm_known = k;
            c->mcr.v &= (uint8_t)~0x08u;   /* MCR[3] self-clears */
            ns_x_wake(c);
            *cause = CAUSE_THR_WRITE;
            return 1;
    }
    return 0;
}

/* ---- Write masks, FCR, MCR ---------------------------------------------------------- */

uint8_t ns_x_ier_mask(const NsChannel *c) {
    switch (style_of(c)) {
        case XS_IER5: return 0x2F;   /* IER[5] special mode; [7:6], [4] reserved */
        case XS_650:
        case XS_854:
        case XS_M752: return enhanced(c) ? 0xFF : 0x0F;
        case XS_5G: return enhanced(c) ? 0xEF : 0x0F;   /* IER[4] reserved (SLEEP register) */
        default: return 0x0F;
    }
}

uint8_t ns_x_mcr_mask(const NsChannel *c) {
    const NsExarDevice *x = xd(c);

    switch (x->style) {
        case XS_IER5:
            /* MCR[7] (power-down) only with IER[5] = 1. */
            return kb(c->ier, 5) ? 0x9F : 0x1F;
        case XS_650:
        case XS_854:
        case XS_M752:
            if (enhanced(c)) {
                return 0xFF;
            }
            return x->mcr2_enhanced ? 0x1B : 0x1F;
        case XS_5G:
            return enhanced(c) ? 0xFF : 0x13;   /* MCR[7:5], [3:2] are enhanced bits */
        default:
            return c->owner->dev->prescaler != PRESCALE_NONE ? 0x9F : 0x1F;
    }
}

void ns_x_mcr_written(NsChannel *c, uint8_t mask) {
    const NsExarDevice *x = xd(c);

    if (x->style == XS_IER5) {
        if (mask & 0x80u) {
            c->xc.powered_down = kb(c->mcr, 7);
            if (!c->xc.powered_down) {
                c->asleep = 0;
            }
        }
        return;
    }
    /* MCR[7] written: the prescaler follows it (an override of CLKSEL). */
    if ((mask & 0x80u) && c->owner->dev->prescaler != PRESCALE_NONE) {
        c->xc.prescale4 = kb(c->mcr, 7);
    }
    /* 5G send-immediate: MCR[3] = 1 arms it (not in loopback, nor with TX disabled). */
    if (x->style == XS_5G && x->v798 && (mask & 0x08u) && kb(c->mcr, 3) && !loopback(c) &&
        !kb(c->xc.msrw, 3)) {
        c->xc.imm_armed = 1;
    }
}

void ns_x_fcr(NsChannel *c, uint8_t v, uint8_t k) {
    const NsDevice *d = c->owner->dev;

    /* FCR[5:4]: the TX trigger, written with the enhanced gate (IER[5] or EFR[4]);
       the 854 and 5G tables take it as written. */
    if ((k & 0x30u) != 0x30u) {
        return;
    }
    switch (style_of(c)) {
        case XS_IER5:
        case XS_650:
        case XS_M752:
            if (d->has_tx_trigger && enhanced(c)) {
                c->tx_trigger = (v >> 4) & 3u;
            }
            break;
        case XS_854:
        case XS_5G:
            if (enhanced(c)) {
                c->tx_trigger = (v >> 4) & 3u;
            }
            break;
        default:
            break;
    }
}

/* ---- Trigger levels ---------------------------------------------------------------- */

/* The 854 (FCTR[5:4]) / 5G (FCTR[7:6]) table: 0-2 = A-C, 3 = D; -1 = X. */
static int table_of(const NsChannel *c) {
    unsigned shift = style_of(c) == XS_5G ? 6 : 4;
    uint8_t bits = (uint8_t)(3u << shift);

    if ((c->xc.fctr.k & bits) != bits) {
        return -1;
    }
    return (c->xc.fctr.v >> shift) & 3;
}

/* 1 when table D's RX level is X (TRG out of range). */
static int rx_trigger_x(const NsChannel *c) {
    if (style_of(c) != XS_854 && style_of(c) != XS_5G) {
        return 0;
    }
    return table_of(c) < 0 || (table_of(c) == 3 && c->xc.trg_rx_bad);
}

unsigned ns_x_rx_trigger(const NsChannel *c, unsigned fcr_level) {
    const NsDevice *d = c->owner->dev;
    int t;

    switch (style_of(c)) {
        case XS_854:
        case XS_5G:
            t = table_of(c);
            if (t < 0) {
                return fcr_level;   /* X: the sources hook makes the level X */
            }
            if (t < 3) {
                return xd(c)->rx_table[t][c->trigger & 3u];
            }
            return c->xc.trg_rx != 0 ? c->xc.trg_rx : 1;
        case XS_M752: {
            unsigned tlr = (c->tlr.v >> 4) & 0x0Fu;

            return d->tlr != 0 && tlr != 0 ? tlr * d->tlr : fcr_level;
        }
        default:
            return fcr_level;
    }
}

/* The Exar TX trigger in bytes (0 = none: the interrupt is "empty"). */
static unsigned tx_level_bytes(const NsChannel *c) {
    const NsDevice *d = c->owner->dev;
    int t;

    switch (style_of(c)) {
        case XS_IER5:
            return d->has_tx_trigger && kb(c->ier, 5) ? d->tx_trigger[c->tx_trigger & 3u] : 0;
        case XS_650:
            return d->has_tx_trigger ? d->tx_trigger[c->tx_trigger & 3u] : 0;
        case XS_854:
        case XS_5G:
            t = table_of(c);
            if (t < 0) {
                return 1;
            }
            if (t < 3) {
                return xd(c)->tx_table[t][c->tx_trigger & 3u];
            }
            return c->xc.trg_tx != 0 && !c->xc.trg_tx_bad ? c->xc.trg_tx : 1;
        default:
            return 0;
    }
}

unsigned ns_x_tx_threshold(const NsChannel *c) {
    const NsDevice *d = c->owner->dev;
    unsigned level;
    unsigned deep = depth(c);

    if (!c->fifo_on) {
        return 0;
    }
    if (style_of(c) == XS_M752) {
        /* TI style: free spaces (TLR × 4, else FCR[5:4]). */
        unsigned tlr = c->tlr.v & 0x0Fu;

        if (d->tlr != 0 && tlr != 0) {
            return tlr * d->tlr;
        }
        return d->has_tx_trigger ? d->tx_trigger[c->tx_trigger & 3u] : 0;
    }
    level = tx_level_bytes(c);
    if (level == 0) {
        return 0;
    }
    if (level > deep) {
        level = deep;
    }
    return deep - level + 1;   /* count < level  <=>  free >= depth - level + 1 */
}

/* The FIFO emptied (ns_set_thre): the THR interrupt too, unless the FIFO
   reached the trigger since the last reload (the 5G interrupts twice). */
int ns_x_thre_at_empty(NsChannel *c) {
    int fire;

    if (ns_x_tx_threshold(c) == 0) {
        return 0;
    }
    fire = !c->xc.filled || style_of(c) == XS_5G;
    c->xc.filled = 0;
    return fire;
}

/* ---- Flow control: receive side ------------------------------------------------------ */

static void resume_tx(NsChannel *c) {
    c->tx_halted = 0;
    c->iir_xoff = 0;
}

int ns_x_rx_char(NsState *s, NsChannel *c, const SerialRxChar *rc) {
    const NsDevice *d = s->dev;
    const NsExarDevice *x = d->x;
    NsExarChannel *xc = &c->xc;
    uint8_t mask = word_mask(c);
    uint8_t data;
    unsigned rxmode = c->efr.v & 3u;
    unsigned txmode = (c->efr.v >> 2) & 3u;
    int sequential;
    int xon = 0;
    int xoff = 0;

    if (xc->rx_allow > 0) {
        xc->rx_allow--;   /* 5G RX disable: the one character still let in */
    }
    if (!has_bank(c)) {
        return 1;
    }
    /* The special-character interrupt clears with the next character
       (XR16M752: only by an ISR read). */
    if (x->style != XS_M752) {
        c->iir_special = 0;
    }
    if ((rc->known & mask) != mask ||
        (rc->flags & (SERIAL_RX_PE | SERIAL_RX_FE | SERIAL_RX_BI)) != 0 ||
        (rc->flags_known & 7u) != 7u) {
        c->seq_wait = 0;
        return 1;   /* errored or unknown characters are data */
    }
    data = (uint8_t)(rc->data & mask);

    /* The second of a sequential pair. */
    if (c->seq_wait != 0) {
        int waiting = c->seq_wait;
        SerialRxChar held = c->seq_held;

        c->seq_wait = 0;
        if (waiting == 1 && data == (c->xon2.v & mask)) {
            resume_tx(c);
            xc->xchar.v |= 0x02;
            return 0;
        }
        if (waiting == 2 && data == (c->xoff2.v & mask)) {
            c->tx_halted = 1;
            c->iir_xoff = 1;
            xc->xchar.v |= 0x01;
            return 0;
        }
        ns_store_char(c, &held);   /* not a pair: the first goes to the FIFO */
    }

    /* EFR[3:0] (exarOr): 1011/0111 compare either character; 1111 and 0011 the pairs. */
    sequential = rxmode == 3 && (txmode == 3 || txmode == 0 || d->sw_flow == SW_NXP_AND);
    switch (rxmode) {
        case 2:
            xon = data == (c->xon1.v & mask);
            xoff = data == (c->xoff1.v & mask);
            break;
        case 1:
            xon = data == (c->xon2.v & mask);
            xoff = data == (c->xoff2.v & mask);
            break;
        case 3:
            if (sequential) {
                if (data == (c->xoff1.v & mask)) {
                    c->seq_wait = 2;
                    c->seq_held = *rc;
                    return 0;
                }
                if (data == (c->xon1.v & mask)) {
                    c->seq_wait = 1;
                    c->seq_held = *rc;
                    return 0;
                }
            } else {
                xon = data == (c->xon1.v & mask) || data == (c->xon2.v & mask);
                xoff = data == (c->xoff1.v & mask) || data == (c->xoff2.v & mask);
            }
            break;
        default:
            break;
    }
    if (xoff) {
        c->tx_halted = 1;
        c->iir_xoff = 1;
        xc->xchar.v |= 0x01;
        /* EFR[1:0] = 01 with EFR[5]: Xoff2 is a flow character, not stored, and
           raises the special-character interrupt too. */
        if (d->special_char && kb(c->efr, 5) && rxmode == 1) {
            c->iir_special = 1;
        }
        return 0;
    }
    if (xon) {
        resume_tx(c);
        xc->xchar.v |= 0x02;
        return 0;
    }
    /* Xon-any (MCR[5]): any other character resumes; it is stored. */
    if (c->tx_halted && x->xon_any && kb(c->mcr, 5)) {
        resume_tx(c);
    }
    /* Special character (EFR[5]): Xoff2 is stored and flags ISR[4]. */
    if (d->special_char && kb(c->efr, 5) && data == (c->xoff2.v & mask)) {
        c->iir_special = 1;
    }
    return 1;
}

/* ---- Flow control: levels (auto-RTS/DTR, Xoff/Xon sending) ------------------------- */

/* Table D hysteresis (EMSR[5:4]:FCTR[1:0] on the 854, FCTR[3:0] on the 5G). */
static unsigned hysteresis(const NsChannel *c) {
    static const unsigned TABLE[16] = {0, 4, 6, 8, 8, 16, 24, 32, 40, 44, 48, 52, 12, 20, 28, 36};
    unsigned i = style_of(c) == XS_5G ? (c->xc.fctr.v & 0x0Fu)
                                      : ((c->xc.emsr.v >> 2) & 0x0Cu) | (c->xc.fctr.v & 3u);

    return TABLE[i];
}

/* RTS off/on levels and Xoff/Xon send levels for the current trigger. */
static void flow_levels(const NsChannel *c, unsigned *rts_off, unsigned *rts_on, unsigned *xoff_at,
                        unsigned *xon_at) {
    const NsDevice *d = c->owner->dev;
    const NsExarDevice *x = d->x;
    unsigned i = c->trigger & 3u;
    unsigned m = d->tlr != 0 ? d->tlr : 4;

    *rts_off = *rts_on = *xoff_at = *xon_at = 0;
    switch (x->style) {
        case XS_M752:
            *rts_off = *xoff_at = (c->tcr.v & 0x0Fu) * m;
            *rts_on = *xon_at = ((c->tcr.v >> 4) & 0x0Fu) * m;
            break;
        case XS_854:
        case XS_5G: {
            int t = table_of(c);

            if (t < 0) {
                t = 0;
            }
            if (t < 3) {
                const unsigned *tab = x->rx_table[t];

                *xoff_at = tab[i];
                *xon_at = i > 0 ? tab[i - 1] : 0;
                *rts_off = i < 3 ? tab[i + 1] : tab[3];
                *rts_on = *xon_at;
            } else {
                unsigned trg = ns_rx_level(c);
                unsigned h = hysteresis(c);

                *xoff_at = trg;
                *rts_off = trg + h;
                /* On again below trigger - hysteresis (decisions.md: 0 = at RXTRG). */
                *rts_on = *xon_at = trg > h ? trg - h - 1 : 0;
            }
            break;
        }
        default:
            if (d->auto_flow == FLOW_TABLE) {
                *rts_off = d->rts_table[i][0];
                *rts_on = d->rts_table[i][1];
            }
            if (d->has_xoff_table) {
                *xoff_at = d->xoff_table[i][0];
                *xon_at = d->xoff_table[i][1];
            }
            break;
    }
}

static int flow_allowed(const NsChannel *c) {
    return !loopback(c) || xd(c)->flow_loopback;
}

static int auto_rts_on(const NsChannel *c) {
    return c->owner->dev->auto_flow != FLOW_NONE && has_bank(c) && kb(c->efr, 6) &&
           flow_allowed(c);
}

static int auto_cts_on(const NsChannel *c) {
    return c->owner->dev->auto_flow != FLOW_NONE && has_bank(c) && kb(c->efr, 7) &&
           flow_allowed(c);
}

/* 5G: MCR[2] = 1 moves auto flow to DTR/DSR. */
static int flow_on_dtr(const NsChannel *c) {
    return c->owner->dev->auto_flow == FLOW_DTR_DSR && kb(c->mcr, 2);
}

static unsigned char_ticks(const NsChannel *c) {
    return serial_format_char_bits(&c->format) * c->format.factor;
}

/* XR16C864 DMA requests (decisions.md): TXDRQ while the TX FIFO is below its
   trigger, until TC or the FIFO fills to it; RXDRQ at the RX trigger or a
   timeout, until TC or the FIFO empties. */
static void dma_requests(NsChannel *c) {
    NsExarChannel *xc = &c->xc;
    unsigned level = tx_level_bytes(c);

    if (level == 0) {
        level = 1;
    }
    if (c->tx_count >= level) {
        xc->drq_tx = 0;
        xc->tc_tx = 0;
    } else if (!xc->tc_tx) {
        xc->drq_tx = 1;
    }
    if (c->rx_count == 0) {
        xc->drq_rx = 0;
        xc->tc_rx = 0;
    } else if (!xc->tc_rx && (c->rx_count >= (unsigned)ns_rx_level(c) || c->timeout)) {
        xc->drq_rx = 1;
    }
}

void ns_x_levels(NsState *s, NsChannel *c) {
    const NsExarDevice *x = s->dev->x;
    unsigned level = c->rx_count;
    unsigned txmode = (c->efr.v >> 2) & 3u;
    unsigned rts_off;
    unsigned rts_on;
    unsigned xoff_at;
    unsigned xon_at;
    int was_off = c->rts_off;

    flow_levels(c, &rts_off, &rts_on, &xoff_at, &xon_at);
    /* Auto-RTS (or DTR): inactive at the off level, active again at the on level. */
    if (auto_rts_on(c)) {
        if (rts_off != 0 && level >= rts_off) {
            c->rts_off = 1;
        } else if (level <= rts_on) {
            c->rts_off = 0;
        }
    } else {
        c->rts_off = 0;
    }
    if (!was_off && c->rts_off && kb(c->ier, 6) && kb(c->mcr, flow_on_dtr(c) ? 0 : 1)) {
        c->iir_ctsrts = 1;   /* the RTS (DTR) pin went low -> high */
    }
    /* Xoff / Xon sending: Xoff two character times after the trigger is
       crossed (XR16M752: at the TCR level), Xon at the level below. */
    if (has_bank(c) && txmode != 0 && c->fifo_on && xoff_at != 0) {
        if (!c->xoff_sent && c->xc.xoff_delay == 0 && level >= xoff_at) {
            if (x->style == XS_M752) {
                queue_flow_pair(c, txmode, 0);
                c->xoff_sent = 1;
            } else {
                c->xc.xoff_delay = 2 * char_ticks(c);
            }
        } else if (c->xc.xoff_delay != 0 && level <= xon_at) {
            c->xc.xoff_delay = 0;   /* drained before the Xoff went out */
        } else if (c->xoff_sent && level <= xon_at) {
            queue_flow_pair(c, txmode, 1);
            c->xoff_sent = 0;
        }
    }
    if (x->dma_host) {
        dma_requests(c);
    }
    ns_x_sleep_check(s, c);
}

/* RS-485 direction: the pin, the turnaround delay in bit times. */
static int rs485_enabled(const NsChannel *c) {
    switch (c->owner->dev->rs485) {
        case RS485_FCTR_RTS:
        case RS485_FCTR_MCR2: return kb(c->xc.fctr, 5);
        case RS485_DLD6: return kb(c->xc.dld, 6);
        case RS485_FCTR3_OP1: return kb(c->xc.fctr, 3);
        default: return 0;
    }
}

static unsigned rs485_delay_bits(const NsChannel *c) {
    switch (c->owner->dev->rs485) {
        case RS485_FCTR_RTS:
        case RS485_FCTR_MCR2: return (c->xc.msrw.v >> 4) & 0x0Fu;   /* 0-15 bit times */
        case RS485_DLD6: return 1;   /* released one bit after the last stop bit */
        default: return 0;
    }
}

/* One baud tick of the channel (after the transmitter's). */
void ns_x_tick(NsState *s, NsChannel *c) {
    NsExarChannel *xc = &c->xc;

    (void)s;
    if (xc->xoff_delay != 0 && --xc->xoff_delay == 0) {
        queue_flow_pair(c, (c->efr.v >> 2) & 3u, 0);
        c->xoff_sent = 1;
    }
    if (xc->rs485 && !serial_tx_busy(&c->line.tx) && c->tx_count == 0 && c->flow_out_count == 0 &&
        !xc->imm_valid) {
        if (xc->rs485_hold == 0) {
            xc->rs485_hold = rs485_delay_bits(c) * c->format.factor + 1;
        }
        if (--xc->rs485_hold == 0) {
            xc->rs485 = 0;
        }
    }
}

/* ---- Flow control: transmit side ----------------------------------------------------- */

int ns_x_tx_flow(NsChannel *c, uint16_t *data) {
    if (c->flow_out_count > 0) {
        *data = c->flow_out[0];
        memmove(c->flow_out, c->flow_out + 1, --c->flow_out_count);
        return 1;
    }
    /* 5G send-immediate: after the flow characters, ahead of the FIFO. */
    if (c->xc.imm_valid && !kb(c->xc.msrw, 3)) {
        *data = c->xc.imm_data;
        c->xc.imm_valid = 0;
        return 1;
    }
    return 0;
}

int ns_x_tx_blocked(NsState *s, NsChannel *c) {
    if (c->tx_halted) {
        return 1;   /* Xoff received */
    }
    if (style_of(c) == XS_5G && kb(c->xc.msrw, 3)) {
        return 1;   /* TX disable: the TSR halts, the FIFO keeps its data */
    }
    if (auto_cts_on(c)) {
        int dsr = flow_on_dtr(c);
        LogicValue cts;

        if (loopback(c)) {
            /* XR16L580: auto flow works in loopback (RTS -> CTS inside). */
            cts = and3(bit_of(c->mcr, dsr ? 0 : 1), bool3(!c->rts_off));
        } else {
            cts = not3(serial_level(s->sim, s->instance,
                                    s->pins->channel[c->index][dsr ? CH_DSR : CH_CTS], LOGIC_X));
        }
        if (cts != LOGIC_1) {
            return 1;
        }
    }
    return 0;
}

/* ---- Interrupts -------------------------------------------------------------------- */

LogicValue ns_x_xoff_source(const NsChannel *c) {
    if (!has_bank(c)) {
        return LOGIC_0;
    }
    return and3(bool3(c->iir_xoff || c->iir_special), bit_of(c->ier, 5));
}

LogicValue ns_x_ctsrts_source(const NsChannel *c) {
    if (!has_bank(c)) {
        return LOGIC_0;
    }
    return and3(bool3(c->iir_ctsrts), or3(bit_of(c->ier, 6), bit_of(c->ier, 7)));
}

/* The TX interrupt waits for the TSR (RS-485 modes, XR16C854 FCTR[3]). */
static int thre_waits_tsr(const NsChannel *c) {
    switch (style_of(c)) {
        case XS_854: return kb(c->xc.fctr, 3);
        case XS_5G: return kb(c->xc.fctr, 5);
        case XS_M752: return kb(c->xc.dld, 6);
        default: return 0;
    }
}

void ns_x_sources(const NsChannel *c, LogicValue *ls, LogicValue *rda, LogicValue *thre,
                  LogicValue *wake) {
    if (thre_waits_tsr(c) && serial_tx_busy(&c->line.tx)) {
        *thre = LOGIC_0;
    }
    /* 5G: an errored byte interrupts as it comes in, not only at the top. */
    if (c->xc.ls_new) {
        *ls = or3(*ls, bit_of(c->ier, 2));
    }
    /* Table D with TRG out of range: whether the trigger is reached is X. */
    if (rx_trigger_x(c) && c->rx_count > 0) {
        *rda = and3(LOGIC_X, bit_of(c->ier, 0));
    }
    *wake = bool3(c->xc.wake_int);
}

/* ST16C1450/1550 ISR[5:4] with IER[5] = 1: TXRDY in [4], RXRDY in [5]. */
void ns_x_iir(const NsChannel *c, uint8_t *v, uint8_t *k) {
    int tx;
    int rx;

    (void)k;
    if (style_of(c) != XS_IER5 || bit_of(c->ier, 5) != LOGIC_1) {
        return;
    }
    tx = txrdy_pin(c) == LOGIC_0;
    rx = rxrdy_pin(c) == LOGIC_0;
    if (xd(c)->isr_inverted) {
        tx = !tx;
        rx = !rx;
    }
    *v = (uint8_t)((*v & ~0x30u) | (tx ? 0x10u : 0) | (rx ? 0x20u : 0));
}

void ns_x_iir_read(NsChannel *c, uint8_t iir) {
    const NsDevice *d = c->owner->dev;
    const NsExarDevice *x = d->x;

    if ((iir & 0x3F) == 0x10) {
        c->iir_special = 0;
        if (x->style != XS_M752) {
            c->iir_xoff = 0;   /* Exar: an ISR read clears the Xoff interrupt (TX stays halted) */
        }
    }
    if ((iir & 0x3F) == 0x20 &&
        (d->ctsrts_clear == CLEAR_IIR || d->ctsrts_clear == CLEAR_IIR_OR_MSR)) {
        c->iir_ctsrts = 0;
    }
    /* ST16C1450/1550: an ISR read alone does not clear TXRDY (only a THR write,
       or IER[1] = 0 after the read). */
    if ((iir & 0x0F) == 0x02 && x->thr_ier1 && kb(c->ier, 1)) {
        c->thre_int = 1;
    }
    /* The wake-up indicator clears with an ISR read (XR16L784: an INT0 read). */
    if ((iir & 0x3F) == 0x01 && !(x->style == XS_5G && !x->v798)) {
        c->xc.wake_int = 0;
    }
}

void ns_x_status_read(NsChannel *c, int msr) {
    const NsDevice *d = c->owner->dev;

    if (!msr && d->special_clear == CLEAR_LSR) {
        c->iir_special = 0;
    }
    if (msr && (d->ctsrts_clear == CLEAR_MSR || d->ctsrts_clear == CLEAR_IIR_OR_MSR)) {
        c->iir_ctsrts = 0;
    }
}

void ns_x_error_in(NsChannel *c) {
    if (style_of(c) == XS_5G) {
        c->xc.ls_new = 1;
    }
}

/* XR16L784: the timeout interrupt clears when the FIFO empties, not on a read. */
int ns_x_timeout_holds(const NsChannel *c) {
    return style_of(c) == XS_5G && !xd(c)->v798 && c->rx_count > 0;
}

void ns_x_lsr_read(NsChannel *c) {
    c->xc.ls_new = 0;
    /* LSR[4] stays while RX is still low after the break (Exar text). */
    if (xd(c)->break_hold && c->line.rx.phase == SERIAL_RX_WAIT_MARK) {
        c->lsr_err |= 0x10;
        c->lsr_err_k |= 0x10;
    }
}

/* ---- Pins ----------------------------------------------------------------------------- */

LogicValue ns_x_rts(const NsChannel *c, LogicValue normal) {
    const NsDevice *d = c->owner->dev;

    /* RS-485 on RTS: high while transmitting (XR16M752 DLD[6]; 5G FCTR[5],
       on DTR instead when the XR16V798's MCR[2] = 1). */
    if (rs485_enabled(c) && (d->rs485 == RS485_DLD6 || d->rs485 == RS485_FCTR_RTS ||
                             (d->rs485 == RS485_FCTR_MCR2 && !kb(c->mcr, 2)))) {
        return bool3(c->xc.rs485);
    }
    if (auto_rts_on(c) && !flow_on_dtr(c) && c->rts_off) {
        return LOGIC_1;
    }
    return normal;
}

LogicValue ns_x_dtr(const NsChannel *c, LogicValue normal) {
    const NsDevice *d = c->owner->dev;

    if (rs485_enabled(c) && d->rs485 == RS485_FCTR_MCR2 && kb(c->mcr, 2)) {
        return bool3(c->xc.rs485);
    }
    if (auto_rts_on(c) && flow_on_dtr(c) && c->rts_off) {
        return LOGIC_1;
    }
    return normal;
}

/* XR16C864 FCTR[3]: OP1 is the RS-485 direction, low while transmitting. */
LogicValue ns_x_out1(const NsChannel *c, LogicValue normal) {
    if (c->owner->dev->rs485 == RS485_FCTR3_OP1 && rs485_enabled(c)) {
        return bool3(!c->xc.rs485);
    }
    return normal;
}

static int irda_on(const NsChannel *c) {
    switch (c->owner->dev->irda) {
        case IRDA_MCR6:
        case IRDA_MCR6_EXAR: return kb(c->mcr, 6);
        case IRDA_DLD7: return kb(c->xc.dld, 7);
        default: return 0;
    }
}

/* SIR: a 0 bit is a 3/16-bit pulse from the 7th to the 10th 16x clock; idle low. */
static LogicValue ir_encode(const NsChannel *c, LogicValue normal) {
    unsigned f;
    unsigned at;

    if (normal == LOGIC_1) {
        return LOGIC_0;
    }
    if (normal == LOGIC_X) {
        return LOGIC_X;
    }
    if (!serial_tx_busy(&c->line.tx)) {
        return LOGIC_0;
    }
    f = c->line.tx.format.factor;
    at = c->line.tx.count;
    return bool3(at >= 7 * f / 16 && at < 10 * f / 16);
}

LogicValue ns_x_tx_pin(const NsChannel *c, LogicValue normal) {
    return irda_on(c) ? ir_encode(c, normal) : normal;
}

/* RX: the IrDA polarity (MCR[2] on the XR16L580/M/V parts, FCTR[2] on the 854,
   FCTR[4] on the 5G), echo suppression (5G IrDA) and RX disable (XR16V798). */
static int ir_inverted(const NsChannel *c) {
    switch (style_of(c)) {
        case XS_854: return kb(c->xc.fctr, 2);
        case XS_5G: return kb(c->xc.fctr, 4);
        default: return xd(c)->irda_polarity && kb(c->mcr, 2);
    }
}

LogicValue ns_x_rx_decode(NsChannel *c, LogicValue pin) {
    LogicValue level;

    if (style_of(c) == XS_5G) {
        if (kb(c->xc.msrw, 2) && !serial_rx_busy(&c->line.rx) && c->xc.rx_allow == 0) {
            return LOGIC_1;   /* RX disabled */
        }
        if (irda_on(c) && serial_tx_busy(&c->line.tx)) {
            return LOGIC_1;   /* the receiver is off while IrDA transmits */
        }
    }
    if (!irda_on(c)) {
        return pin;
    }
    if (ir_inverted(c)) {
        pin = not3(pin);
    }
    /* A pulse's falling edge is a 0 for one bit. */
    if (c->ir_last == LOGIC_1 && pin == LOGIC_0) {
        c->ir_low = c->format.factor;
    }
    c->ir_last = pin;
    level = c->ir_low > 0 ? LOGIC_0 : LOGIC_1;
    if (c->ir_low > 0) {
        c->ir_low--;
    }
    return level;
}

void ns_x_pins(NsState *s, NsChannel *c, Cause cause) {
    const NsExarDevice *x = s->dev->x;
    const NsExarPins *xp = s->pins->xp;
    size_t op = s->pins->op[c->index];
    size_t mf = s->pins->mf[c->index];
    size_t irtx = xp->irtx[c->index];
    LogicValue loop = bit_of(c->mcr, 4);
    LogicValue op_level = or3(not3(bit_of(c->mcr, 3)), loop);
    SimTime delay = cause == CAUSE_MCR_WRITE ? t_of(s, T_MDO) : 0;

    if (s->unknown) {
        ns_drive(s, op, LOGIC_X, 0);
        ns_drive(s, mf, LOGIC_X, 0);
        ns_drive(s, irtx, LOGIC_X, 0);
        return;
    }
    if (s->in_reset) {
        op_level = LOGIC_1;
    }
    ns_drive(s, op, op_level, delay);
    /* MF: AFR[2:1] picks OP2, BAUDOUT (driven on clock edges by the core),
       RXRDY or the reserved code. */
    if (mf != SERIAL_NO_PIN) {
        NsMfFunction f = x->has_mf ? x->mf[(c->afr.v >> 1) & 3u] : MF_OP2;

        switch (f) {
            case MF_OP2: ns_drive(s, mf, op_level, delay); break;
            case MF_BAUDOUT: break;
            case MF_RXRDY: ns_drive(s, mf, s->in_reset ? LOGIC_1 : rxrdy_pin(c), 0); break;
            default: ns_drive(s, mf, LOGIC_X, 0); break;
        }
    }
    /* IRTX (XR16C854/864 QFP100): always IrDA-encoded, idle low. */
    if (irtx != SERIAL_NO_PIN) {
        LogicValue tx = c->tsr;

        if (kb(c->lcr, 6)) {
            tx = LOGIC_0;
        }
        ns_drive(s, irtx, s->in_reset ? LOGIC_0 : ir_encode(c, tx), 0);
    }
}

/* XR16C864 TXDRQ/RXDRQ (active high) on the TXRDY/RXRDY pins by EMSR[2]/[3]. */
void ns_x_dma_pins(const NsChannel *c, LogicValue *rxrdy, LogicValue *txrdy) {
    if (!xd(c)->dma_host || c->owner->in_reset) {
        return;
    }
    /* The requests follow the FIFO levels as the pins are driven (every bus
       access and tick ends here). */
    dma_requests((NsChannel *)c);
    /* Class X: with the requests off the pins are TXRDY/RXRDY, mode-1 TXRDY
       low while a slot is free (decisions.md). */
    if (mode1(c) && !kb(c->xc.emsr, 2)) {
        *txrdy = bool3(tx_free(c) == 0);
    }
    if (kb(c->xc.emsr, 3)) {
        *rxrdy = bool3(c->xc.drq_rx);
    }
    if (kb(c->xc.emsr, 2)) {
        *txrdy = bool3(c->xc.drq_tx);
    }
}

/* Open drain: 0 while any level is 1, X while one is X, else released. */
static LogicValue open_drain(LogicValue any) {
    return any == LOGIC_1 ? LOGIC_0 : any == LOGIC_X ? LOGIC_X : LOGIC_Z;
}

static LogicValue all_channels(const NsState *s) {
    LogicValue any = LOGIC_0;
    unsigned i;

    for (i = 0; i < s->dev->channel_count; i++) {
        any = or3(any, ns_int_level(&s->ch[i]));
    }
    return any;
}

static LogicValue timer_level(const NsState *s) {
    return s->xs.timer_int_en ? s->xs.timer_int : LOGIC_0;
}

int ns_x_int_pin(const NsState *s, const NsChannel *c, LogicValue level, LogicValue *out) {
    const NsExarDevice *x = s->dev->x;
    size_t pin = s->pins->channel[c->index][CH_INT];
    int mode;

    if (x != NULL && x->motorola_bus && s->xs.motorola != 0) {
        if (s->xs.motorola < 0) {
            *out = LOGIC_X;
        } else if (pin == s->pins->xp->irq) {
            /* Motorola: one open-drain IRQ for every channel, not gated by MCR[3]. */
            *out = s->in_reset ? LOGIC_Z : open_drain(all_channels(s));
        } else {
            *out = x->irq_others_low ? LOGIC_0 : LOGIC_Z;
        }
        return 1;
    }
    if (s->xs.part == NULL || s->xs.part->int_pin < 0) {
        return 0;
    }
    /* The part's "intPin" (D variants: always driven). */
    mode = s->xs.part->int_pin;
    if (mode == INT_DRIVEN) {
        *out = level;
    } else {
        LogicValue out2 = bit_of(c->mcr, 3);

        *out = out2 == LOGIC_1 ? level : out2 == LOGIC_0 ? LOGIC_Z : LOGIC_X;
    }
    return 1;
}

static SimTime cause_delay(const NsState *s, Cause cause) {
    switch (cause) {
        case CAUSE_RBR_READ:
        case CAUSE_LSR_READ: return t_of(s, T_RINT);
        case CAUSE_IIR_READ: return t_of(s, T_IR);
        case CAUSE_MSR_READ: return t_of(s, T_RIM);
        case CAUSE_THR_WRITE: return t_of(s, T_HR);
        case CAUSE_MCR_WRITE: return t_of(s, T_MDO);
        case CAUSE_MODEM_IN: return t_of(s, T_SIM);
        default: return 0;
    }
}

/* Shared DMA pins (XR16V554/564/654, XR16C854/864): the AND of the channels'
   active-low levels, refreshed on every channel update (receive events too). */
static void shared_dma(NsState *s, Cause cause) {
    LogicValue rx = LOGIC_1;
    LogicValue tx = LOGIC_1;
    unsigned i;

    if (s->dev->dma_pins != DMA_PINS_SHARED_AND) {
        return;
    }
    if (s->unknown) {
        rx = tx = LOGIC_X;
    } else if (!s->in_reset) {
        for (i = 0; i < s->dev->channel_count; i++) {
            rx = and3(rx, rxrdy_pin(&s->ch[i]));
            tx = and3(tx, txrdy_pin(&s->ch[i]));
        }
    }
    ns_drive(s, s->pins->dma_rxrdy, rx, cause == CAUSE_RBR_READ ? t_of(s, T_WRR) : 0);
    ns_drive(s, s->pins->dma_txrdy, tx, cause == CAUSE_THR_WRITE ? t_of(s, T_WXI) : 0);
}

/* Device-level pins: the Motorola IRQ (all channels), the 5G INT#, RST. */
void ns_x_device_pins(NsState *s, Cause cause) {
    const NsExarDevice *x = s->dev->x;
    const NsExarPins *xp = s->pins->xp;
    SimTime delay = cause_delay(s, cause);

    shared_dma(s, cause);
    if (x->motorola_bus && s->xs.motorola == 1 && !s->unknown) {
        ns_drive(s, xp->irq, s->in_reset ? LOGIC_Z : open_drain(all_channels(s)), delay);
    }
    if (xp->interrupt != SERIAL_NO_PIN) {
        LogicValue level = or3(all_channels(s), timer_level(s));

        ns_drive(s, xp->interrupt,
                 s->unknown ? LOGIC_X : s->in_reset ? LOGIC_Z : open_drain(level), delay);
    }
    /* ST16C1450/1550 RST = RESET OR (IER[5] AND MCR[2]); loopback cuts MCR[2] off. */
    if (xp->reset_out != SERIAL_NO_PIN) {
        const NsChannel *c = &s->ch[0];
        LogicValue reset = serial_level(s->sim, s->instance, s->pins->reset,
                                        s->dev->reset_active_high ? LOGIC_0 : LOGIC_1);
        LogicValue soft = and3(and3(bit_of(c->ier, 5), bit_of(c->mcr, 2)), not3(bit_of(c->mcr, 4)));

        if (!s->dev->reset_active_high) {
            reset = not3(reset);
        }
        ns_drive(s, xp->reset_out, s->unknown ? LOGIC_X : or3(reset, s->in_reset ? LOGIC_0 : soft),
                 delay);
    }
}

/* ---- Clocks ------------------------------------------------------------------------- */

int ns_x_prescale(NsState *s, NsChannel *c, int rising) {
    (void)s;
    if (c->asleep || c->xc.powered_down) {
        return 0;   /* sleep / power-down: the baud clock stops */
    }
    if (c->xc.prescale4) {
        if (!rising) {
            return 0;
        }
        if (++c->prescale_count < 4) {
            return 0;
        }
        c->prescale_count = 0;
    }
    return 1;
}

void ns_x_load_divisor(NsChannel *c) {
    serial_baud_load(&c->baud, (uint32_t)c->dll.v | ((uint32_t)c->dlm.v << 8),
                     c->dll.k == 0xFF && c->dlm.k == 0xFF);
    /* DLD[3:0]: the fraction n/16. */
    if (xd(c)->dld && (c->xc.dld.k & 0x0Fu) == 0x0Fu) {
        c->baud.frac = c->xc.dld.v & 0x0Fu;
        c->baud.frac_den = 16;
    }
}

unsigned ns_x_factor(const NsChannel *c) {
    const NsExarDevice *x = xd(c);

    if (x->dld_sampling && (c->xc.dld.k & 0x30u) == 0x30u) {
        unsigned rate = (c->xc.dld.v >> 4) & 3u;

        return rate == 0 ? 16 : rate == 1 ? 8 : 4;   /* DLD[5:4]: 16x, 8x, 4x */
    }
    if (x->style == XS_5G && ((c->owner->xs.mode8x >> c->index) & 1u)) {
        return 8;   /* 8XMODE */
    }
    return 16;
}

/* "startSample": 7.5 checks the start bit half a clock earlier: tick 7 of 16. */
unsigned ns_x_start_sample(const NsChannel *c, unsigned factor) {
    return xd(c)->start_75 ? factor * 15 / 32 : 0;
}

int ns_x_own_clock(const NsState *s, unsigned channel) {
    return s->dev->x->chc_clock != NULL && channel == 2 &&
           s->pins->xp->chc_clock != SERIAL_NO_PIN;
}

/* BCLK (XR16C864): the channel's 16x clock. */
void ns_x_clock_out(NsState *s, NsChannel *c, LogicValue baudout) {
    size_t pin = s->pins->xp->bclk[c->index];

    if (!SERIAL_REAL_PIN(pin) || s->out[pin] == baudout) {
        return;
    }
    s->out[pin] = baudout;
    sim_schedule_drive(s->sim, s->instance, pin, baudout, DRIVE_STRONG, t_of(s, T_BLD));
}

/* An edge of a channel's own clock pin (CHCCLK): the core's per-channel step. */
static void own_clock_edge(NsState *s, NsChannel *c, LogicValue level) {
    LogicValue last = c->xc.clock_level;
    int rising = last == LOGIC_0 && level == LOGIC_1;
    int tick = 0;

    c->xc.clock_level = level;
    if (level == last || (!rising && !(last == LOGIC_1 && level == LOGIC_0))) {
        return;
    }
    if (ns_x_prescale(s, c, rising) || (!rising && !c->asleep && !c->xc.prescale4)) {
        tick = serial_baud_edge(&c->baud, rising, level);
    }
    ns_x_clock_out(s, c, s->unknown || c->asleep ? LOGIC_X : c->baud.baudout);
    if (tick) {
        ns_tx_tick(s, c);
        if (s->pins->receive == SERIAL_NO_PIN) {
            ns_rx_tick(s, c);
        }
    }
}

/* ---- The 5G timer ------------------------------------------------------------------ */

static void timer_step(NsState *s) {
    NsExarState *g = &s->xs;

    if (!g->timer_running || s->in_reset || s->unknown) {
        return;
    }
    if (g->timer_reload < 2) {
        g->timer_int = LOGIC_X;   /* N = 0 or 1 (decisions.md) */
        return;
    }
    if (g->timer_count > 0 && --g->timer_count == 0) {
        g->timer_int = LOGIC_1;
        if (g->timer_oneshot) {
            g->timer_running = 0;
        } else {
            g->timer_count = g->timer_reload;
        }
        ns_update_outputs(s, CAUSE_SERIAL);
    }
}

static int all_asleep(const NsState *s) {
    unsigned i;

    for (i = 0; i < s->dev->channel_count; i++) {
        if (!s->ch[i].asleep) {
            return 0;
        }
    }
    return 1;
}

void ns_x_clock_edge(NsState *s, int rising) {
    /* The oscillator stops while every channel sleeps. */
    if (rising && s->dev->x->tmrck != NULL && !s->xs.timer_ext && !all_asleep(s)) {
        timer_step(s);
    }
}

/* ---- Sleep, wake, PowerSave ------------------------------------------------------- */

static int sleep_enabled(const NsChannel *c) {
    switch (c->owner->dev->sleep) {
        case SLEEP_IER4_EXAR:
        case SLEEP_IER4_EXAR_ALL: return kb(c->ier, 4);
        case SLEEP_GLOBAL: return (c->owner->xs.sleep >> c->index) & 1u;
        default: return 0;
    }
}

/* Nothing pending, no modem delta, RX idle high, nothing to send. */
static int channel_idle(const NsState *s, const NsChannel *c) {
    uint8_t iir;
    uint8_t iir_k;
    size_t rx = s->pins->channel[c->index][CH_RX];

    if (serial_rx_busy(&c->line.rx) || c->pending_count > 0 || c->rx_count > 0 ||
        c->tx_count > 0 || serial_tx_busy(&c->line.tx) || c->flow_out_count > 0 ||
        c->xc.imm_valid || loopback(c) || (c->delta & 0x0F) != 0 || c->xc.wake_int ||
        c->xc.xoff_delay != 0) {
        return 0;
    }
    if (serial_level(s->sim, s->instance, rx, LOGIC_1) != LOGIC_1) {
        return 0;
    }
    if (xd(c)->sleep_divisor && (!c->baud.known || c->baud.divisor == 0)) {
        return 0;
    }
    ns_iir_now(c, &iir, &iir_k);
    return iir_k == 0xFF && (iir & 1u);
}

static void update_isolation(NsState *s) {
    const NsExarPins *xp = s->pins->xp;
    int was = s->xs.isolated;

    s->xs.isolated = xp->pwrsave != SERIAL_NO_PIN && all_asleep(s) &&
                     serial_level(s->sim, s->instance, xp->pwrsave, LOGIC_X) == LOGIC_1;
    if (was != s->xs.isolated && s->sim != NULL) {
        serial_bus_update(&s->bus, s->sim, s->instance, &s->events);
    }
}

void ns_x_sleep_check(NsState *s, NsChannel *c) {
    const NsDevice *d = s->dev;
    unsigned i;

    if (d->x->style == XS_IER5) {
        c->asleep = c->xc.powered_down;   /* MCR[7] power-down */
        return;
    }
    if (d->sleep < SLEEP_IER4_EXAR || s->sim == NULL || s->in_reset || s->unknown) {
        return;
    }
    if (d->sleep == SLEEP_GLOBAL) {
        /* 5G: each channel with its SLEEP bit sleeps when idle. */
        if (!c->asleep && sleep_enabled(c) && channel_idle(s, c)) {
            c->asleep = 1;
        }
    } else {
        /* Exar IER[4]: the chip sleeps when every channel asks and is idle
           (one channel on the single parts). */
        for (i = 0; i < d->channel_count; i++) {
            if (!sleep_enabled(&s->ch[i]) || !channel_idle(s, &s->ch[i])) {
                return;
            }
        }
        for (i = 0; i < d->channel_count; i++) {
            s->ch[i].asleep = 1;
        }
    }
    update_isolation(s);
}

static void wake_all(NsChannel *by) {
    NsState *s = by->owner;
    int was_asleep = 0;
    unsigned i;

    for (i = 0; i < s->dev->channel_count; i++) {
        was_asleep |= s->ch[i].asleep;
        s->ch[i].asleep = 0;
    }
    /* The wake-up indicator: INT asserts, ISR reads "none" (0x01). */
    if (was_asleep && (s->dev->x->wake_indicator || s->dev->sleep == SLEEP_GLOBAL)) {
        by->xc.wake_int = 1;
    }
    if (was_asleep) {
        update_isolation(s);
    }
}

void ns_x_wake(NsChannel *c) {
    NsState *s = c->owner;
    const NsDevice *d = s->dev;
    unsigned threshold = ns_x_tx_threshold(c);

    /* A THR write (bytes waiting): the trigger bookkeeping, RS-485 direction. */
    if (c->tx_count > 0 || c->xc.imm_valid) {
        if (threshold != 0 && tx_free(c) < threshold) {
            c->xc.filled = 1;
        }
        if (rs485_enabled(c)) {
            c->xc.rs485 = 1;   /* before the start bit */
            c->xc.rs485_hold = 0;
        }
    }
    if (d->x->style == XS_IER5 || d->sleep < SLEEP_IER4_EXAR) {
        return;
    }
    /* Wake on a THR write or an RX start bit (RX low). */
    if (c->tx_count > 0 || c->xc.imm_valid ||
        serial_level(s->sim, s->instance, s->pins->channel[c->index][CH_RX], LOGIC_1) == LOGIC_0) {
        wake_all(c);
    }
}

int ns_x_cts_changed(NsChannel *c, LogicValue cts_active) {
    NsState *s = c->owner;
    int auto_cts = auto_cts_on(c);

    if (flow_on_dtr(c)) {
        LogicValue dsr = not3(serial_level(s->sim, s->instance,
                                           s->pins->channel[c->index][CH_DSR], LOGIC_X));

        if (auto_cts && c->xc.dsr_last == LOGIC_1 && dsr == LOGIC_0 && kb(c->ier, 7)) {
            c->iir_ctsrts = 1;   /* DSR pin low -> high */
        }
        c->xc.dsr_last = dsr;
    } else if (auto_cts && c->cts_last == LOGIC_1 && cts_active == LOGIC_0 && kb(c->ier, 7)) {
        c->iir_ctsrts = 1;   /* CTS pin low -> high */
    }
    c->cts_last = cts_active;
    if (s->dev->sleep >= SLEEP_IER4_EXAR) {
        wake_all(c);   /* a modem input change wakes (also from PowerSave) */
    }
    return 1;
}

/* ---- Bus: select decode and the extra units ------------------------------------------ */

/* Units past the channels: the 5G global block, then the reserved / FSTAT
   unit, then one DMA unit per channel (XR16C864). */
static unsigned unit_global(const NsState *s) { return s->dev->channel_count; }
static unsigned unit_other(const NsState *s) { return s->dev->channel_count + 1; }
static unsigned unit_dma(const NsState *s, unsigned ch) { return s->dev->channel_count + 2 + ch; }

int ns_x_needs_select(const NsDevice *d) {
    const NsExarDevice *x = d->x;

    return x->motorola_bus || x->pwrsave != NULL || x->style == XS_5G || x->fstat != NULL ||
           x->dma_host;
}

static LogicValue low_active(const NsState *s, size_t pin) {
    return not3(serial_level(s->sim, s->instance, pin, LOGIC_1));
}

/* The device's own chip selects (5G CS; the single-CS Motorola parts). */
static LogicValue bus_cs(const NsState *s) {
    const SerialBusPins *bp = &s->pins->bus;
    LogicValue all = LOGIC_1;
    unsigned i;

    for (i = 0; i < bp->select_count; i++) {
        all = and3(all, low_active(s, bp->select[i]));
    }
    for (i = 0; i < bp->select_high_count; i++) {
        all = and3(all, serial_level(s->sim, s->instance, bp->select_high[i], LOGIC_0));
    }
    return all;
}

static int select_5g(NsState *s, unsigned address, unsigned known, uint32_t *units,
                     LogicValue *result) {
    const NsExarDevice *x = s->dev->x;
    LogicValue cs = bus_cs(s);
    unsigned cc = s->dev->channel_count;
    unsigned ch_bits = x->v798 ? 0x70u : 0x30u;

    if (cs != LOGIC_1) {
        *result = cs;
        return 1;
    }
    *result = LOGIC_1;
    if (!(known & 0x80u)) {
        *result = LOGIC_X;
        return 1;
    }
    if (address & 0x80u) {
        *units = (address & 0x70u) == 0 ? 1u << unit_global(s) : 1u << unit_other(s);
        return 1;
    }
    if (!x->v798 && (known & 0x40u) && (address & 0x40u)) {
        *units = 1u << unit_other(s);   /* XR16L784: 0x40-0x7F reserved */
        return 1;
    }
    /* REGB[0]: a channel-register write goes to every channel. */
    if (s->xs.regb & 1u) {
        *units = (1u << cc) - 1u;
        return 1;
    }
    if ((known & ch_bits) != ch_bits || (!x->v798 && !(known & 0x40u))) {
        *result = LOGIC_X;
        return 1;
    }
    *units = 1u << ((address & ch_bits) >> 4);
    return 1;
}

int ns_x_select(NsState *s, unsigned address, unsigned address_known, uint32_t *units,
                LogicValue *result) {
    const NsExarDevice *x = s->dev->x;
    const NsExarPins *xp = s->pins->xp;
    unsigned cc = s->dev->channel_count;
    unsigned i;

    /* PowerSave: the bus inputs are isolated. */
    if (s->xs.isolated) {
        *result = LOGIC_0;
        return 1;
    }
    /* XR16C864 DMA cycle: DACKx low and AEN high; CS and A2..A0 are ignored. */
    if (x->dma_host) {
        LogicValue aen = serial_level(s->sim, s->instance, xp->aen, LOGIC_0);

        for (i = 0; i < cc; i++) {
            LogicValue dack = low_active(s, xp->dack[i]);
            LogicValue cycle = and3(aen, dack);

            if (cycle == LOGIC_X) {
                *result = LOGIC_X;
                return 1;
            }
            if (cycle == LOGIC_1) {
                *units = 1u << unit_dma(s, i);
                *result = LOGIC_1;
                return 1;
            }
        }
    }
    /* FSTAT (FSRS low): any address. */
    if (xp->fstat != SERIAL_NO_PIN) {
        LogicValue fsrs = low_active(s, xp->fstat);

        if (fsrs != LOGIC_0) {
            *units = 1u << unit_other(s);
            *result = fsrs;
            return 1;
        }
    }
    if (x->style == XS_5G) {
        return select_5g(s, address, address_known, units, result);
    }
    if (x->motorola_bus && s->xs.motorola != 0) {
        LogicValue cs = s->xs.motorola < 0 ? LOGIC_X : low_active(s, xp->strobe);
        unsigned ch = 0;

        *result = cs;
        if (cs != LOGIC_1) {
            return 1;
        }
        /* A3 (and A4) on the Intel select pins pick the channel. */
        for (i = 0; i < x->chan_bits; i++) {
            LogicValue bit = serial_level(s->sim, s->instance, xp->chan_bit[i], LOGIC_X);

            if (bit == LOGIC_X) {
                *result = LOGIC_X;
                return 1;
            }
            ch |= (bit == LOGIC_1 ? 1u : 0u) << i;
        }
        *units = ch < cc ? 1u << ch : 1u << unit_other(s);
        return 1;
    }
    if (!s->dev->per_channel_select && s->dev->channel_pin == NULL && s->dev->lpt == NULL) {
        *result = bus_cs(s);   /* one channel, the bus's own selects */
        return 1;
    }
    return 0;
}

/* The 5G per-channel 3-bit interrupt code for INT1-INT3. */
static int int_code(const NsState *s, const NsChannel *c) {
    uint8_t iir;
    uint8_t k;

    ns_iir_now(c, &iir, &k);
    if (k != 0xFF) {
        return -1;
    }
    if (iir & 1u) {
        if (c->index == 0 && timer_level(s) == LOGIC_1) {
            return 7;
        }
        return c->index == 0 && timer_level(s) == LOGIC_X ? -1 : 0;
    }
    switch (iir & 0x3Eu) {
        case 0x06:
        case 0x04: return 1;
        case 0x0C: return 2;
        case 0x02: return 3;
        default: return 4;
    }
}

static void global_peek(NsState *s, unsigned address, uint8_t *v, uint8_t *k) {
    const NsDevice *d = s->dev;
    const NsExarDevice *x = d->x;
    NsExarState *g = &s->xs;
    unsigned cc = d->channel_count;
    unsigned i;

    *v = 0;
    *k = 0xFF;
    switch (address & 0x0Fu) {
        case 0x0:
            for (i = 0; i < cc; i++) {
                LogicValue level = ns_int_level(&s->ch[i]);

                if (i == 0) {
                    level = or3(level, timer_level(s));
                }
                if (level == LOGIC_1) {
                    *v |= (uint8_t)(1u << i);
                } else if (level == LOGIC_X) {
                    *k &= (uint8_t)~(1u << i);
                }
            }
            break;
        case 0x1:
        case 0x2:
        case 0x3: {
            uint32_t value = 0;
            uint32_t known = 0xFFFFFFu;
            unsigned shift = ((address & 0x0Fu) - 1u) * 8u;

            for (i = 0; i < cc; i++) {
                int code = int_code(s, &s->ch[i]);

                if (code < 0) {
                    known &= ~(7u << (3 * i));
                } else {
                    value |= (uint32_t)code << (3 * i);
                }
            }
            *v = (uint8_t)(value >> shift);
            *k = (uint8_t)(known >> shift);
            break;
        }
        case 0x4:
            if (x->timer_commands) {
                /* XR16V798: 01 while the timer interrupt is pending. */
                *v = g->timer_int == LOGIC_1 ? 1 : 0;
                *k = g->timer_int == LOGIC_X ? 0xFE : 0xFF;
            } else {
                *v = g->timercntl & 0x0Fu;
            }
            break;
        case 0x6: *v = (uint8_t)g->timer_reload; break;
        case 0x7: *v = (uint8_t)(g->timer_reload >> 8); break;
        case 0x8: *v = g->mode8x; break;
        case 0xB: *v = g->sleep; break;
        case 0xC: *v = d->drev >= 0 ? (uint8_t)d->drev : 0; break;
        case 0xD: *v = d->dvid >= 0 ? (uint8_t)d->dvid : 0; break;
        case 0xE: *v = g->regb; break;
        case 0xF: *k = 0; break;   /* not listed: X */
        default: break;            /* TIMER, REGA, RESET read 00 */
    }
}

static void global_read_done(NsState *s, unsigned address) {
    const NsExarDevice *x = s->dev->x;
    unsigned i;

    switch (address & 0x0Fu) {
        case 0x0:
            /* XR16L784: an INT0 read clears the wake-up indicator. */
            if (!x->v798) {
                for (i = 0; i < s->dev->channel_count; i++) {
                    s->ch[i].xc.wake_int = 0;
                }
            }
            break;
        case 0x4:
            if (s->xs.timer_int != LOGIC_X) {
                s->xs.timer_int = LOGIC_0;   /* a TIMERCNTL read clears the timer interrupt */
            }
            break;
        default:
            return;
    }
    ns_update_outputs(s, CAUSE_IIR_READ);
}

static void timer_start(NsExarState *g) {
    g->timer_running = 1;
    g->timer_count = g->timer_reload;
}

static void global_write(NsState *s, unsigned address, uint8_t v, uint8_t k) {
    const NsDevice *d = s->dev;
    const NsExarDevice *x = d->x;
    NsExarState *g = &s->xs;
    uint8_t chans = (uint8_t)((1u << d->channel_count) - 1u);
    unsigned i;

    if (k != 0xFF) {
        v = 0;   /* an X write: the global registers take nothing (P2) */
    }
    switch (address & 0x0Fu) {
        case 0x4:
            if (x->timer_commands) {
                switch (v & 0x0Fu) {
                    case 0x1: g->timer_int_en = 1; break;
                    case 0x2: g->timer_int_en = 0; break;
                    case 0x3: g->timer_oneshot = 1; break;
                    case 0x4: g->timer_oneshot = 0; break;
                    case 0x5: g->timer_ext = 0; break;
                    case 0x6: g->timer_ext = 1; break;
                    case 0x9: timer_start(g); break;
                    case 0xA: g->timer_running = 0; break;
                    case 0xB:
                        g->timer_running = 0;
                        g->timer_count = g->timer_reload;
                        g->timer_int = LOGIC_0;
                        break;
                    default: break;   /* 0000, 0111, 1000, 1100-1111 reserved */
                }
            } else {
                /* XR16L784: b0 INT enable, b1 start/stop, b2 one-shot, b3 TMRCK. */
                if ((v & 2u) && !g->timer_running) {
                    timer_start(g);
                }
                g->timer_running = (v & 2u) != 0;
                g->timer_int_en = v & 1u;
                g->timer_oneshot = (v & 4u) != 0;
                g->timer_ext = (v & 8u) != 0;
                g->timercntl = v & 0x0Fu;
            }
            break;
        case 0x6: g->timer_reload = (uint16_t)((g->timer_reload & 0xFF00u) | v); break;
        case 0x7: g->timer_reload = (uint16_t)((g->timer_reload & 0x00FFu) | (v << 8)); break;
        case 0x8:
            g->mode8x = v & chans;
            for (i = 0; i < d->channel_count; i++) {
                set_format_rate(&s->ch[i]);
            }
            break;
        case 0xA:
            /* RESET: soft-reset the channels whose bits are 1 (self-clearing). */
            g->soft = 1;
            for (i = 0; i < d->channel_count; i++) {
                if ((v >> i) & 1u) {
                    ns_reset_channel(s, &s->ch[i]);
                    ns_x_load_divisor(&s->ch[i]);
                    set_format_rate(&s->ch[i]);
                }
            }
            g->soft = 0;
            break;
        case 0xB:
            g->sleep = v & chans;
            for (i = 0; i < d->channel_count; i++) {
                if (!((g->sleep >> i) & 1u)) {
                    s->ch[i].asleep = 0;
                }
                ns_x_sleep_check(s, &s->ch[i]);
            }
            break;
        case 0xE: g->regb = v & 1u; break;
        default: break;   /* read-only or reserved */
    }
    ns_update_outputs(s, CAUSE_WRITE);
}

/* FSTAT: b3-0 = NOT TXRDYD..A, b7-4 = RXRDYD..A pins. */
static uint8_t fstat(const NsState *s) {
    uint8_t v = 0;
    unsigned i;

    for (i = 0; i < s->dev->channel_count && i < 4; i++) {
        if (txrdy_pin(&s->ch[i]) == LOGIC_0) {
            v |= (uint8_t)(1u << i);
        }
        if (rxrdy_pin(&s->ch[i]) == LOGIC_1) {
            v |= (uint8_t)(0x10u << i);
        }
    }
    return v;
}

/* A DMA cycle reads the RX FIFO / writes the TX FIFO whatever LCR[7] is. */
static Reg dma_begin(NsChannel *c) {
    Reg saved = c->lcr;

    c->lcr.v &= 0x7F;
    c->lcr.k |= 0x80;
    return saved;
}

int ns_x_unit_peek(NsState *s, unsigned unit, unsigned address, uint8_t *v, uint8_t *k) {
    const NsExarDevice *x = s->dev->x;

    if (unit < s->dev->channel_count) {
        return 0;
    }
    *v = 0;
    *k = 0;
    if (x->style == XS_5G && unit == unit_global(s)) {
        global_peek(s, address, v, k);
    } else if (unit == unit_other(s) && x->fstat != NULL) {
        *v = fstat(s);
        *k = 0xFF;
    } else if (x->dma_host && unit >= unit_dma(s, 0)) {
        NsChannel *c = &s->ch[unit - unit_dma(s, 0)];
        Reg saved = dma_begin(c);

        ns_reg_peek(c, 0, v, k);
        c->lcr = saved;
    }
    return 1;
}

int ns_x_unit_read_done(NsState *s, unsigned unit, unsigned address) {
    const NsExarDevice *x = s->dev->x;

    if (unit < s->dev->channel_count) {
        return 0;
    }
    if (x->style == XS_5G && unit == unit_global(s)) {
        global_read_done(s, address);
    } else if (x->dma_host && unit >= unit_dma(s, 0)) {
        NsChannel *c = &s->ch[unit - unit_dma(s, 0)];
        Reg saved = dma_begin(c);
        Cause cause = ns_reg_read_done(c, 0);

        c->lcr = saved;
        ns_x_levels(s, c);
        ns_update_channel(s, c, cause);
    }
    return 1;
}

int ns_x_unit_write(NsState *s, unsigned unit, unsigned address, uint8_t v, uint8_t k) {
    const NsExarDevice *x = s->dev->x;

    if (unit < s->dev->channel_count) {
        return 0;
    }
    if (x->style == XS_5G && unit == unit_global(s)) {
        global_write(s, address, v, k);
    } else if (x->dma_host && unit >= unit_dma(s, 0)) {
        NsChannel *c = &s->ch[unit - unit_dma(s, 0)];
        Reg saved = dma_begin(c);
        Cause cause = ns_reg_write(s, c, 0, v, k);

        c->lcr = saved;
        ns_x_levels(s, c);
        ns_update_channel(s, c, cause);
    }
    return 1;   /* FSTAT and reserved addresses take no writes */
}

/* AFR[0] concurrent write: one shared bit, or (XR16L2552) the addressed channel's. */
int ns_x_conc(const NsState *s, unsigned unit) {
    if (s->dev->x->afr0_per_channel) {
        return unit < s->dev->channel_count && kb(s->ch[unit].afr, 0);
    }
    return s->conc;
}

/* Motorola mode: the reset pin is active low. */
int ns_x_reset_active_high(const NsState *s) {
    if (s->dev->x->motorola_bus && s->xs.motorola == 1) {
        return 0;
    }
    return s->dev->reset_active_high;
}

/* ---- Pin changes, start -------------------------------------------------------------- */

static void read_mode(NsState *s) {
    const NsExarPins *xp = s->pins->xp;
    LogicValue mode;

    if (!s->dev->x->motorola_bus) {
        return;
    }
    /* 16/68: high = Intel (16), low = Motorola (68). */
    mode = serial_level(s->sim, s->instance, xp->mode, LOGIC_X);
    s->xs.motorola = mode == LOGIC_1 ? 0 : mode == LOGIC_0 ? 1 : -1;
    s->bus.motorola = s->xs.motorola;
    s->bus.rw = xp->rw;
}

int ns_x_is_select_pin(const NsState *s, size_t pin) {
    const NsExarPins *xp = s->pins->xp;
    unsigned i;

    if (pin == SERIAL_NO_PIN) {
        return 0;
    }
    if (pin == xp->mode || pin == xp->strobe || pin == xp->chan_bit[0] || pin == xp->chan_bit[1] ||
        pin == xp->pwrsave || pin == xp->fstat || pin == xp->aen) {
        return 1;
    }
    for (i = 0; i < 4; i++) {
        if (pin == xp->dack[i]) {
            return 1;
        }
    }
    return 0;
}

/* 1 when the bus mode changed (the core re-applies the reset level). */
int ns_x_pin_changed(NsState *s, size_t pin) {
    const NsExarPins *xp = s->pins->xp;
    int before = s->xs.motorola;
    unsigned i;

    if (pin == SERIAL_NO_PIN) {
        return 0;
    }
    if (pin == xp->mode) {
        read_mode(s);
        return s->xs.motorola != before;
    }
    if (pin == xp->pwrsave) {
        update_isolation(s);
    }
    if (pin == xp->tmrck) {
        LogicValue level = serial_level(s->sim, s->instance, pin, LOGIC_X);

        if (s->xs.tmrck_level == LOGIC_0 && level == LOGIC_1 && s->xs.timer_ext) {
            timer_step(s);
        }
        s->xs.tmrck_level = level;
    }
    if (pin == xp->chc_clock) {
        own_clock_edge(s, &s->ch[2], serial_level(s->sim, s->instance, pin, LOGIC_X));
    }
    if (pin == xp->tc) {
        LogicValue level = serial_level(s->sim, s->instance, pin, LOGIC_0);

        /* TC pulse: ends the requests of the channel whose DACK is low. */
        if (s->xs.tc_level == LOGIC_0 && level == LOGIC_1) {
            for (i = 0; i < s->dev->channel_count; i++) {
                if (low_active(s, xp->dack[i]) == LOGIC_1) {
                    NsExarChannel *xc = &s->ch[i].xc;

                    if (xc->drq_tx) {
                        xc->drq_tx = 0;
                        xc->tc_tx = 1;
                    }
                    if (xc->drq_rx) {
                        xc->drq_rx = 0;
                        xc->tc_rx = 1;
                    }
                }
            }
            ns_update_outputs(s, CAUSE_SERIAL);
        }
        s->xs.tc_level = level;
    }
    return 0;
}

void ns_x_start(NsState *s) {
    unsigned i;

    read_mode(s);
    s->xs.tmrck_level = serial_level(s->sim, s->instance, s->pins->xp->tmrck, LOGIC_X);
    s->xs.tc_level = serial_level(s->sim, s->instance, s->pins->xp->tc, LOGIC_0);
    for (i = 0; i < s->dev->channel_count; i++) {
        s->ch[i].xc.clock_level = serial_level(s->sim, s->instance, s->pins->xp->chc_clock,
                                               LOGIC_X);
        sample_straps(s, &s->ch[i]);   /* CLKSEL and ENIR at power-up */
    }
}
