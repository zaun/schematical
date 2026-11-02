/*
 * ns16550 I2C/SPI-to-UART bridges (plan-12-I section 3.2): the SC16IS740,
 * SC16IS750/760 and SC16IS752/762 (NXP), the PI7C9X760/762 (Diodes, the
 * SC16IS map plus the SFR bank) and the XR20M1170 (MaxLinear, plus DLD);
 * the XR20M1280's own map is in ns16550_xr1280.c.
 *
 * The host port is serial_target.c's I2C/SPI target, with the NXP sub-address
 * byte (I2C) or command byte (SPI): register bits [6:3] (or [5:3]), channel
 * bits [2:1], SPI bit 7 = 1 read. No auto-increment: every data byte of a
 * transfer goes to the same register (the FIFO burst), except with PI7C9X
 * SFR[4] = 1. Registers 8-15: TXLVL, RXLVL, IODir, IOState, IOIntEna,
 * reserved, IOControl, EFCR; the 650-style bank at LCR = 0xBF; TCR/TLR at 6/7
 * with EFR[4] and MCR[2]; with LCR[7] = 1 (not 0xBF) only DLL, DLH, LCR (and
 * the XR20M1170's DLD) answer, other addresses read X (decisions.md).
 *
 * Sources: plan-12-data/specs/bridges.md, matrix.md Table A rows 54-62 and
 * Table B, decisions.md § bridges.md and the planner decisions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ns16550_bridge.h"

/* ---- Small helpers --------------------------------------------------------------- */

static NsBridge *br_of(const NsChannel *c) {
    return c->owner->bridge;
}

static int is_sc(const NsDevice *d) {
    return d->extended == EXT_SC16IS || d->extended == EXT_SC16IS_SFR ||
           d->extended == EXT_SC16IS_DLD;
}

static int is_xr(const NsDevice *d) {
    return d->extended == EXT_XR1280;
}

static int enhanced(const NsChannel *c) {
    return nb_known1(c->efr, 4);
}

void nb_drive(NsState *s, size_t pin, LogicValue value, SimTime delay) {
    if (!SERIAL_REAL_PIN(pin) || s->out[pin] == value) {
        return;
    }
    s->out[pin] = value;
    sim_schedule_inertial_drive(s->sim, s->instance, pin, value, DRIVE_STRONG, delay);
}

SimTime nb_cause_delay(const NsState *s, Cause cause) {
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

LogicValue nb_gpio_level(const NsState *s, unsigned bit) {
    size_t pin = s->pins->bridge->gpio[bit];

    if (pin == SERIAL_NO_PIN || bit >= s->dev->bridge->gpio_count) {
        return LOGIC_X;
    }
    return serial_level(s->sim, s->instance, pin, s->dev->bridge->gpio_pullup ? LOGIC_1 : LOGIC_X);
}

/* The levels of all 8 GPIO inputs: value and known masks. */
static void gpio_levels(const NsState *s, uint8_t *v, uint8_t *k) {
    unsigned b;

    *v = 0;
    *k = 0;
    for (b = 0; b < NB_GPIO; b++) {
        LogicValue l = nb_gpio_level(s, b);

        if (l != LOGIC_X) {
            *k |= (uint8_t)(1u << b);
            if (l == LOGIC_1) {
                *v |= (uint8_t)(1u << b);
            }
        }
    }
}

/* SC16IS: channel ch's modem pins are on (IOControl bit ch + 1). */
static LogicValue sc_modem_on(const NsState *s, unsigned ch) {
    return bit_of(s->bridge->iocontrol, ch + 1);
}

/* The GPIO bit is a modem pin now: 1, 0 or X. */
static LogicValue gpio_is_modem(const NsState *s, unsigned b) {
    const NsBridgePins *bp = s->pins->bridge;

    if (bp->gpio_ch[b] < 0) {
        return LOGIC_0;
    }
    if (is_xr(s->dev)) {
        return bool3(xr_modem_on(s));
    }
    return sc_modem_on(s, (unsigned)bp->gpio_ch[b]);
}

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
    if (d->extended < EXT_SC16IS) {
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

int ns_bridge_open_drain(const NsDevice *d, const NsPins *p, size_t pin) {
    if (pin == SERIAL_NO_PIN || p->bridge == NULL) {
        return 0;
    }
    return pin == p->bridge->irq || serial_host_open_drain(&d->bridge->host, &p->bridge->host, pin);
}

/* ---- Register reset ------------------------------------------------------------------ */

static void reset_registers(NsState *s, int power_on) {
    NsBridge *br = s->bridge;
    const NsBridgeDevice *b = s->dev->bridge;
    unsigned i;

    br->iodir = (Reg){0, 0xFF};
    br->latch = b->state_reset >= 0 ? (Reg){(uint8_t)b->state_reset, 0xFF} : (Reg){0, 0};
    br->iointena = (Reg){0, 0xFF};
    br->iocontrol = (Reg){0, 0xFF};
    br->latched = 0;
    br->latch_pending = 0;
    if (s->sim != NULL) {
        gpio_levels(s, &br->ref, &br->ref_k);
    } else {
        br->ref = 0;
        br->ref_k = 0;
    }
    for (i = 0; i < s->dev->channel_count; i++) {
        NbChannel *n = &br->ch[i];

        n->efcr = (Reg){0, 0xFF};
        n->dld = (Reg){0, 0xFF};
        n->sfren = (Reg){0, 0xFF};
        n->sfr = (Reg){0, 0xFF};
        n->cpr = (Reg){0x10, 0xFF};
        n->tidle = (Reg){0, 0xFF};
        n->trctl = (Reg){0x06, 0xFF};
        n->fc_toggle = 0;
        n->aa_on = 0;
    }
    if (is_xr(s->dev)) {
        xr_reset(s, power_on);
    }
    for (i = 0; i < s->dev->channel_count; i++) {
        ns_ext_load_divisor(&s->ch[i]);
        ns_update_format(&s->ch[i]);
    }
}

/* IOControl[3] (and XR20M1280 has none): the whole device, as the reset pin. */
static void software_reset(NsState *s) {
    unsigned i;

    ns_reset_all(s);
    reset_registers(s, 0);
    for (i = 0; i < s->dev->channel_count; i++) {
        ns_modem_refresh(s, &s->ch[i]);
    }
}

/* ---- Register decode (SC16IS family) --------------------------------------------- */

/* 0 general set (LCR[7] = 0), 1 special set (LCR[7] = 1, not 0xBF), 2 the
   enhanced set (LCR = 0xBF), -1 unknown. */
static int lcr_set(const NsChannel *c) {
    LogicValue l7 = bit_of(c->lcr, 7);

    if (l7 == LOGIC_0) {
        return 0;
    }
    if (l7 == LOGIC_X) {
        return -1;
    }
    if (((c->lcr.v ^ 0xBFu) & c->lcr.k) != 0) {
        return 1;   /* cannot be 0xBF */
    }
    return c->lcr.k == 0xFF ? 2 : -1;
}

static int sc_decode(const NsChannel *c, unsigned a, int write) {
    const NsDevice *d = c->owner->dev;
    const NbChannel *n = nb_ch(c);
    int x = write ? R_IGNORE : R_X;
    int set = lcr_set(c);

    if (a == 3) {
        return R_CORE;
    }
    if (set < 0) {
        return x;
    }
    if (set == 1) {
        if (a == 0 || a == 1) {
            return R_CORE;
        }
        if (a == 2 && d->extended == EXT_SC16IS_DLD && enhanced(c)) {
            return B_DLD;
        }
        return x;
    }
    if (set == 2) {
        if (d->extended == EXT_SC16IS_SFR) {
            LogicValue special = bit_of(n->sfr, 2);
            static const int SPECIAL[16] = {
                -1, -1, B_ASR, -1, B_CPR, B_RFD, B_TFD, -1, B_TIDLE, B_TRCTL, -1, -1, -1, -1, -1,
                B_ISCR
            };

            if (a == 13) {
                return B_SFREN;
            }
            if (SPECIAL[a] >= 0 && special != LOGIC_0) {
                if (special == LOGIC_X) {
                    return x;
                }
                if (write && (SPECIAL[a] == B_ASR || SPECIAL[a] == B_RFD || SPECIAL[a] == B_TFD)) {
                    return R_IGNORE;   /* read-only */
                }
                return SPECIAL[a];
            }
            if (a == 7) {
                if (n->sfren.k != 0xFF) {
                    return x;
                }
                if (n->sfren.v == 0x5A) {
                    return B_SFR;
                }
            }
        }
        switch (a) {
            case 2: return R_EFR;
            case 4: return R_XON1;
            case 5: return R_XON2;
            case 6: return R_XOFF1;
            case 7: return R_XOFF2;
            default: return x;
        }
    }
    /* The general set. */
    switch (a) {
        case 5:
            return write ? R_IGNORE : R_CORE;   /* LSR is read-only */
        case 6:
        case 7: {
            LogicValue tcr = and3(bool3(enhanced(c)), bit_of(c->mcr, 2));

            if (tcr == LOGIC_X) {
                return x;
            }
            if (tcr == LOGIC_1) {
                return a == 6 ? R_TCR : R_TLR;
            }
            return R_CORE;
        }
        case 8: return write ? R_IGNORE : B_TXLVL;
        case 9: return write ? R_IGNORE : B_RXLVL;
        case 10:
        case 11:
        case 12:
            if (d->bridge->gpio_count == 0) {
                return x;   /* SC16IS740: no GPIO (planner decision) */
            }
            return a == 10 ? B_IODIR : a == 11 ? B_IOSTATE : B_IOINTENA;
        case 13: return x;
        case 14:
            /* The SC16IS740 reads X here; a write still has its software reset. */
            return d->bridge->gpio_count == 0 && !write ? R_X : B_IOCONTROL;
        case 15: return B_EFCR;
        default: return R_CORE;
    }
}

int ns_bridge_decode(const NsChannel *c, unsigned address, int write) {
    if (c->owner->bridge == NULL) {
        return R_CORE;
    }
    if (is_xr(c->owner->dev)) {
        return xr_decode(c, address, write);
    }
    return sc_decode(c, address & 15u, write);
}

/* ---- GPIO (SC16IS) ----------------------------------------------------------------------- */

/* The GPIO interrupt (IOLATCH 0: an enabled input differs from its level at
   the last IOState read; IOLATCH 1: a change was latched). */
static LogicValue sc_gpio_pending(const NsState *s) {
    const NsBridge *br = s->bridge;
    LogicValue latch_mode = bit_of(br->iocontrol, 0);
    LogicValue any = LOGIC_0;
    unsigned b;

    if (s->dev->bridge->gpio_style != NB_GPIO_SC16IS) {
        return LOGIC_0;
    }
    if (latch_mode == LOGIC_1) {
        return bool3(br->latch_pending);
    }
    for (b = 0; b < s->dev->bridge->gpio_count; b++) {
        LogicValue cond;
        LogicValue level;
        LogicValue diff;

        if (s->pins->bridge->gpio[b] == SERIAL_NO_PIN) {
            continue;
        }
        cond = and3(and3(not3(bit_of(br->iodir, b)), bit_of(br->iointena, b)),
                    not3(gpio_is_modem(s, b)));
        if (cond == LOGIC_0) {
            continue;
        }
        level = nb_gpio_level(s, b);
        if (level == LOGIC_X || !((br->ref_k >> b) & 1u)) {
            diff = LOGIC_X;
        } else {
            diff = bool3((level == LOGIC_1) != (((br->ref >> b) & 1u) != 0));
        }
        any = or3(any, and3(cond, diff));
    }
    if (latch_mode == LOGIC_X) {
        return or3(any, bool3(br->latch_pending)) == LOGIC_0 ? LOGIC_0 : LOGIC_X;
    }
    return any;
}

/* A GPIO pin changed: IOLATCH = 1 latches an enabled input's new level. */
static void sc_gpio_changed(NsState *s, unsigned b) {
    NsBridge *br = s->bridge;
    LogicValue level = nb_gpio_level(s, b);

    if (bit_of(br->iocontrol, 0) != LOGIC_1 || ((br->latched >> b) & 1u)) {
        return;
    }
    if (bit_of(br->iodir, b) != LOGIC_0 || bit_of(br->iointena, b) != LOGIC_1 ||
        gpio_is_modem(s, b) != LOGIC_0) {
        return;
    }
    br->latched |= (uint8_t)(1u << b);
    br->latched_v = (uint8_t)((br->latched_v & ~(1u << b)) | (level == LOGIC_1 ? 1u << b : 0));
    br->latched_k = (uint8_t)((br->latched_k & ~(1u << b)) | (level != LOGIC_X ? 1u << b : 0));
    br->latch_pending = 1;
}

/* An IOState read, an IODir write: the GPIO interrupt clears. */
static void sc_gpio_clear(NsState *s) {
    NsBridge *br = s->bridge;

    gpio_levels(s, &br->ref, &br->ref_k);
    br->latched = 0;
    br->latch_pending = 0;
}

static void sc_iostate(const NsState *s, uint8_t *v, uint8_t *k) {
    const NsBridge *br = s->bridge;
    unsigned b;

    *v = 0;
    *k = 0;
    for (b = 0; b < NB_GPIO; b++) {
        uint8_t m = (uint8_t)(1u << b);
        LogicValue l;

        if ((br->latched & m) != 0) {
            l = (br->latched_k & m) ? ((br->latched_v & m) ? LOGIC_1 : LOGIC_0) : LOGIC_X;
        } else if (s->pins->bridge->gpio[b] == SERIAL_NO_PIN) {
            /* No pin in this package: an output reads its latch, an input X. */
            l = bit_of(br->iodir, b) == LOGIC_1 ? bit_of(br->latch, b) : LOGIC_X;
        } else {
            l = nb_gpio_level(s, b);
        }
        if (l != LOGIC_X) {
            *k |= m;
            if (l == LOGIC_1) {
                *v |= m;
            }
        }
    }
}

/* ---- Peek / write / read side effects ------------------------------------------------- */

static uint8_t count_errors(const NsChannel *c) {
    unsigned i;
    unsigned n = 0;

    for (i = 0; i < c->rx_count; i++) {
        const SerialRxChar *rc = &c->rx[(c->rx_head + i) % NS_MAX_FIFO];

        if ((rc->flags & 7u) != 0 || (rc->flags_known & 7u) != 7u) {
            n++;
        }
    }
    return (uint8_t)n;
}

/* PI7C9X ASR: [5:4] Xon/Xoff state, [1] we asked the remote to stop, [0]
   the remote stopped us. */
static uint8_t pi_asr(const NsChannel *c) {
    uint8_t v = 0;

    if ((c->efr.v & 0x0Fu) != 0) {
        if (c->tx_halted) {
            v |= 0x10;
        } else if (c->xoff_sent) {
            v |= 0x20;
        } else if (c->xon_seen) {
            v |= 0x30;
        }
    }
    if (c->xoff_sent || c->rts_off) {
        v |= 0x02;
    }
    if (c->tx_halted) {
        v |= 0x01;
    }
    return v;
}

static uint8_t pi_iscr(const NsChannel *c) {
    LogicValue src[8];
    static const unsigned BIT[7] = {1, 2, 3, 4, 5, 6, 7};   /* ls rda timeout thr modem xoff ctsrts */
    uint8_t v = 0;
    unsigned i;

    ns_sources(c, src);
    for (i = 0; i < 7; i++) {
        if (src[i] == LOGIC_1) {
            v |= (uint8_t)(1u << BIT[i]);
        }
    }
    return v;
}

void ns_bridge_peek_reg(NsChannel *c, int reg, uint8_t *v, uint8_t *k) {
    NsState *s = c->owner;
    NsBridge *br = s->bridge;
    NbChannel *n = nb_ch(c);

    *v = 0;
    *k = 0xFF;
    switch (reg) {
        case B_TXLVL: *v = (uint8_t)(s->dev->fifo - c->tx_count); break;
        case B_RXLVL: *v = (uint8_t)c->rx_count; break;
        case B_IODIR: *v = br->iodir.v; *k = br->iodir.k; break;
        case B_IOSTATE: sc_iostate(s, v, k); break;
        case B_IOINTENA: *v = br->iointena.v; *k = br->iointena.k; break;
        case B_IOCONTROL:
            *v = br->iocontrol.v & 0xF7u;   /* SRESET reads 0 (self-clearing) */
            *k = br->iocontrol.k | 0x08u;
            break;
        case B_EFCR: *v = n->efcr.v; *k = n->efcr.k; break;
        case B_DLD: *v = n->dld.v; *k = n->dld.k; break;
        case B_SFREN: *v = n->sfren.v; *k = n->sfren.k; break;
        case B_SFR: *v = n->sfr.v; *k = n->sfr.k; break;
        case B_ASR: *v = pi_asr(c); break;
        case B_CPR: *v = n->cpr.v; *k = n->cpr.k; break;
        case B_RFD:
            if (bit_of(n->sfr, 6) == LOGIC_X) {
                *k = 0;
            } else {
                *v = nb_known1(n->sfr, 6) ? count_errors(c) : (uint8_t)c->rx_count;
            }
            break;
        case B_TFD: *v = (uint8_t)c->tx_count; break;
        case B_TIDLE: *v = n->tidle.v; *k = n->tidle.k; break;
        case B_TRCTL: *v = n->trctl.v; *k = n->trctl.k; break;
        case B_ISCR: *v = pi_iscr(c); break;
        default:
            if (is_xr(s->dev)) {
                xr_peek_reg(c, reg, v, k);
            } else {
                *k = 0;
            }
            break;
    }
}

/* PI7C9X ISCR[0] CLSTATUS: the latched interrupt events clear. */
static void pi_clear_status(NsChannel *c) {
    c->thre_int = 0;
    c->thr_level_int = 0;
    c->timeout = 0;
    c->idle_ticks = 0;
    c->iir_xoff = 0;
    c->iir_special = 0;
    c->iir_ctsrts = 0;
    c->delta = 0;
    c->delta_k = 0xFF;
    c->lsr_err &= (uint8_t)~0x02;
    c->lsr_err_k |= 0x02;
}

void ns_bridge_write_reg(NsState *s, NsChannel *c, int reg, uint8_t v, uint8_t k) {
    NsBridge *br = s->bridge;
    NbChannel *n = nb_ch(c);
    unsigned i;

    switch (reg) {
        case B_IODIR:
            br->iodir = (Reg){v, k};
            sc_gpio_clear(s);   /* "writing IODir clears a pending GPIO interrupt" */
            break;
        case B_IOSTATE: br->latch = (Reg){v, k}; break;
        case B_IOINTENA: br->iointena = (Reg){v, k}; break;
        case B_IOCONTROL:
            if (!(k & 0x08u)) {
                s->unknown = 1;   /* SRESET written X: the reset is X (P2) */
                return;
            }
            if (v & 0x08u) {
                software_reset(s);
                return;
            }
            if (s->dev->bridge->gpio_count == 0) {
                return;   /* SC16IS740: only the software-reset bit */
            }
            br->iocontrol = (Reg){(uint8_t)(v & 0x07u), (uint8_t)(k | 0xF8u)};
            if (s->dev->channel_count < 2) {
                br->iocontrol.v &= 0x03u;   /* IOControl[2] reserved on one channel */
            }
            for (i = 0; i < s->dev->channel_count; i++) {
                ns_modem_refresh(s, &s->ch[i]);
            }
            if (!nb_known1(br->iocontrol, 0)) {
                br->latched = 0;
                br->latch_pending = 0;
            }
            break;
        case B_EFCR:
            if (!nb_known1(n->efcr, 0) && (v & k & 1u)) {
                n->aa_on = 0;   /* 9-bit mode on: auto-address receiver starts off */
            }
            n->efcr = (Reg){v, k};
            break;
        case B_DLD:
            n->dld = (Reg){v, k};
            ns_ext_load_divisor(c);
            ns_update_format(c);
            break;
        case B_SFREN: n->sfren = (Reg){v, k}; break;
        case B_SFR: n->sfr = (Reg){v, k}; break;
        case B_CPR:
            n->cpr = (Reg){v, k};
            c->prescale_count = 0;
            break;
        case B_TIDLE: n->tidle = (Reg){v, k}; break;   /* stored only (planner decision) */
        case B_TRCTL:
            n->trctl = (Reg){v, k};
            ns_update_format(c);
            break;
        case B_ISCR:
            if (k & v & 1u) {
                pi_clear_status(c);
            }
            break;
        default:
            if (is_xr(s->dev)) {
                xr_write_reg(s, c, reg, v, k);
            }
            break;
    }
}

int ns_bridge_read_done(NsChannel *c, unsigned address, Cause *cause) {
    NsState *s = c->owner;
    int reg;

    if (s->bridge == NULL) {
        return 0;
    }
    reg = ns_bridge_decode(c, address, 0);
    if (reg == R_CORE && is_xr(s->dev)) {
        NbChannel *n = nb_ch(c);

        if (address == 2) {
            /* An ISR read: the wake-up interrupt clears; with EMSR[7] = 1 the
               Xoff interrupt clears here (not on Xon). */
            n->wake_int = 0;
            if (c->iir_seen_k == 0xFF && (c->iir_seen & 0x3Fu) == 0x10u) {
                n->xoff_sticky = 0;
                c->iir_xoff = 0;
            }
        } else if (address == 5) {
            n->ls_now = 0;   /* EMSR[6]: an LSR read clears the immediate LSR interrupt */
        }
        return 0;
    }
    if (reg < NS_REG_BRIDGE) {
        return 0;
    }
    *cause = CAUSE_WRITE;
    if (reg == B_IOSTATE) {
        sc_gpio_clear(s);
        *cause = CAUSE_MSR_READ;
        return 1;
    }
    if (is_xr(s->dev)) {
        return xr_read_done(c, reg, cause);
    }
    return 1;
}

/* ---- The host port ------------------------------------------------------------------------ */

static void sub_decode(NsBridge *br, const NsBridgeDevice *b, uint8_t data, uint8_t known) {
    uint8_t reg_mask = b->reg_bits == 4 ? 0x78 : 0x38;

    br->sub_byte = data;
    br->sub_known = known;
    br->sub_reg = (data & reg_mask) >> 3;
    br->sub_ch = (data >> 1) & 3u;
    br->sub_x = ((known & (reg_mask | 0x06)) != (reg_mask | 0x06));
    br->have_sub = 1;
}

/* X data into every register an unknown sub-address could name (P2). */
static void write_unknown(NsState *s) {
    NsBridge *br = s->bridge;
    uint8_t reg_mask = s->dev->bridge->reg_bits == 4 ? 0x78 : 0x38;
    unsigned regs = 1u << s->dev->bridge->reg_bits;
    unsigned ch;
    unsigned r;

    for (ch = 0; ch < s->dev->channel_count; ch++) {
        for (r = 0; r < regs; r++) {
            uint8_t byte = (uint8_t)((r << 3) | (ch << 1));
            uint8_t mask = (uint8_t)(br->sub_known & (reg_mask | 0x06));

            if (((byte ^ br->sub_byte) & mask) != 0) {
                continue;
            }
            if (!s->unknown) {
                ns_host_write(s, ch, r, 0, 0);
            }
        }
    }
}

static void advance(NsState *s) {
    NsBridge *br = s->bridge;
    const NsChannel *c;

    if (br->sub_x || br->sub_ch >= s->dev->channel_count) {
        return;
    }
    c = &s->ch[br->sub_ch];
    /* PI7C9X SFR[4]: bursts auto-increment the sub-address (decisions.md). */
    if (s->dev->extended == EXT_SC16IS_SFR && nb_known1(nb_ch(c)->sfr, 4)) {
        br->sub_reg = (br->sub_reg + 1u) & ((1u << s->dev->bridge->reg_bits) - 1u);
    }
}

/* Does a write to this register reset the device (IOControl[3])? */
static int writes_reset(NsState *s, uint8_t data, uint8_t known) {
    NsBridge *br = s->bridge;

    return !is_xr(s->dev) &&
           sc_decode(&s->ch[br->sub_ch], br->sub_reg, 1) == B_IOCONTROL && (data & known & 0x08u);
}

/* A THR write with the TX FIFO full. */
static int thr_full(NsState *s) {
    NsBridge *br = s->bridge;
    NsChannel *c = &s->ch[br->sub_ch];

    return br->sub_reg == 0 && bit_of(c->lcr, 7) == LOGIC_0 && c->tx_count >= depth(c);
}

/* A data byte for the addressed register: the I2C ACK (1), NACK (0) or X. */
static LogicValue host_write(NsState *s, uint8_t data, uint8_t known) {
    NsBridge *br = s->bridge;
    const NsBridgeDevice *b = s->dev->bridge;
    LogicValue ack = LOGIC_1;

    if (s->unknown) {
        return LOGIC_X;
    }
    if (br->sub_x) {
        write_unknown(s);
        return LOGIC_X;
    }
    if (br->sub_ch >= s->dev->channel_count) {
        return LOGIC_1;   /* a reserved channel: ignored (P2) */
    }
    if (b->nack_full && thr_full(s)) {
        return LOGIC_0;   /* the byte is refused, not written */
    }
    if (b->nack_reset && writes_reset(s, data, known)) {
        ack = LOGIC_0;
    }
    ns_host_write(s, br->sub_ch, br->sub_reg, data, known);
    advance(s);
    return ack;
}

static void host_peek(NsState *s, uint8_t *data, uint8_t *known) {
    NsBridge *br = s->bridge;

    *data = 0;
    *known = 0;
    if (s->unknown || !br->have_sub || br->sub_x || br->sub_ch >= s->dev->channel_count) {
        return;
    }
    ns_host_peek(s, br->sub_ch, br->sub_reg, data, known);
}

static void host_read_done(NsState *s) {
    NsBridge *br = s->bridge;

    if (s->unknown || !br->have_sub || br->sub_x || br->sub_ch >= s->dev->channel_count) {
        return;
    }
    ns_host_read_done(s, br->sub_ch, br->sub_reg);
    advance(s);
}

/* I2C */

static void i2c_begin(void *context, int repeated) {
    NsState *s = context;

    if (!repeated) {
        s->bridge->have_sub = 0;
    }
}

static LogicValue i2c_address(void *context, uint8_t byte, uint8_t known) {
    NsState *s = context;
    int mine = serial_host_i2c_address(&s->bridge->host);
    unsigned addr = byte >> 1;

    if (mine == -1) {
        return LOGIC_0;
    }
    if (mine < 0) {
        return LOGIC_X;   /* a strap that matches no tie */
    }
    if ((known & 0xFE) != 0xFE) {
        return ((((unsigned)mine ^ addr) & (known >> 1)) == 0) ? LOGIC_X : LOGIC_0;
    }
    return bool3((unsigned)mine == addr);
}

static LogicValue i2c_received(void *context, unsigned index, uint8_t data, uint8_t known) {
    NsState *s = context;
    NsBridge *br = s->bridge;

    (void)index;
    if (!br->have_sub) {
        sub_decode(br, s->dev->bridge, data, known);
        return LOGIC_1;
    }
    return host_write(s, data, known);
}

static void i2c_next(void *context, unsigned index, uint8_t *data, uint8_t *known) {
    (void)index;
    host_peek(context, data, known);
}

static void i2c_acked(void *context, unsigned index, LogicValue ack) {
    (void)index;
    (void)ack;
    host_read_done(context);
}

/* SPI (mode 0): command byte, then data on SI (write) or SO (read). */

static void spi_begin(void *context) {
    NsState *s = context;

    s->bridge->have_sub = 0;
    s->bridge->spi_rw = -1;
}

static SerialSpiByte spi_next(void *context, unsigned index) {
    NsState *s = context;
    NsBridge *br = s->bridge;
    SerialSpiByte out = {0, 0, 0};

    if (index == 0 || br->spi_rw == 0) {
        return out;   /* SO is Z during the command byte and writes */
    }
    out.drive = 0xFF;
    if (br->spi_rw < 0) {
        return out;   /* X */
    }
    host_peek(s, &out.data, &out.known);
    return out;
}

static void spi_received(void *context, unsigned index, uint8_t data, uint8_t known) {
    NsState *s = context;
    NsBridge *br = s->bridge;

    if (index == 0) {
        sub_decode(br, s->dev->bridge, data, known);
        br->spi_rw = (known & 0x80u) ? (int)((data >> 7) & 1u) : -1;
        return;
    }
    if (br->spi_rw == 1) {
        host_read_done(s);
    } else if (br->spi_rw == 0) {
        host_write(s, data, known);
    } else if (!s->unknown) {
        br->sub_x = 1;   /* a write could have happened: X into what it could name */
        write_unknown(s);
    }
}

/* ---- Create / start / reset / pins -------------------------------------------------------- */

int ns_bridge_create(NsState *s) {
    const NsBridgeDevice *b = s->dev->bridge;
    NsBridge *br = calloc(1, sizeof *br);
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
    br->spi.begin = spi_begin;
    br->spi.next = spi_next;
    br->spi.received = spi_received;
    br->i2c.context = s;
    br->i2c.begin = i2c_begin;
    br->i2c.address = i2c_address;
    br->i2c.received = i2c_received;
    br->i2c.next = i2c_next;
    br->i2c.acked = i2c_acked;
    br->spi_rw = -1;
    br->sda_level = LOGIC_X;
    br->fast_irda = b->fast_irda_all;
    for (g = 0; g < b->fast_irda_count; g++) {
        if (strcmp(b->fast_irda_grades[g], s->grade->info.grade) == 0) {
            br->fast_irda = 1;
        }
    }
    reset_registers(s, 1);
    for (i = 0; i < s->dev->channel_count; i++) {
        NsChannel *c = &s->ch[i];

        if (b->dl_power[0] >= 0) {
            c->dll = (Reg){(uint8_t)b->dl_power[0], 0xFF};
        }
        if (b->dl_power[1] >= 0) {
            c->dlm = (Reg){(uint8_t)b->dl_power[1], 0xFF};
        }
        if (is_xr(s->dev)) {
            br->ch[i].dll_tx = br->ch[i].dll_rx = c->dll;
            br->ch[i].dlm_tx = br->ch[i].dlm_rx = c->dlm;
        }
        ns_ext_load_divisor(c);
        ns_update_format(c);
    }
    return 0;
}

void ns_bridge_destroy(NsState *s) {
    free(s->bridge);
    s->bridge = NULL;
}

void ns_bridge_start(NsState *s) {
    NsBridge *br = s->bridge;

    serial_host_start(&br->host, s->sim, s->instance);
    br->sda_level = serial_level(s->sim, s->instance, s->pins->bridge->host.i2c.sda, LOGIC_X);
    gpio_levels(s, &br->ref, &br->ref_k);
    if (is_xr(s->dev)) {
        xr_straps(s);
        xr_gpio_changed(s);
    }
}

void ns_bridge_reset(NsState *s, LogicValue active) {
    NsBridge *br = s->bridge;
    int held = br->host.held;

    if (active == LOGIC_1) {
        reset_registers(s, 0);
    }
    serial_host_hold(&br->host, s->sim, s->instance, active != LOGIC_0);
    if (held && active == LOGIC_0 && is_xr(s->dev)) {
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
    if (is_xr(s->dev)) {
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
            sc_gpio_changed(s, b);
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
            return gpio_is_modem(s, b) == LOGIC_1;
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
    if (is_xr(s->dev)) {
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
        modem = gpio_is_modem(s, b);
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
    if (is_xr(s->dev)) {
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
    if (is_xr(s->dev)) {
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
    if (is_xr(s->dev)) {
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
    if (s->bridge == NULL || !is_xr(s->dev)) {
        return 0;
    }
    return xr_levels(s, c);
}

LogicValue ns_bridge_rts(const NsChannel *c, LogicValue normal) {
    const NbChannel *n;
    LogicValue on;
    int active;

    if (is_xr(c->owner->dev)) {
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
    if (is_xr(s->dev)) {
        return nb_known1(nb_ch(c)->emsr, 4) ? LOGIC_Z : normal;   /* EMSR[4] 3-state */
    }
    if (s->dev->extended == EXT_SC16IS_SFR && nb_known1(nb_ch(c)->sfr, 3)) {
        /* SFR[3]: internal loopback RX -> TX. */
        return serial_level(s->sim, s->instance, s->pins->channel[c->index][CH_RX], LOGIC_X);
    }
    return normal;
}

int ns_bridge_irda(const NsChannel *c) {
    return c->owner->bridge != NULL && is_xr(c->owner->dev) && c->owner->bridge->enir_low;
}

int ns_bridge_ir_quarter(const NsChannel *c) {
    const NsBridge *br = c->owner->bridge;

    if (br == NULL || !br->fast_irda) {
        return 0;
    }
    if (is_xr(c->owner->dev)) {
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
    if (is_xr(c->owner->dev)) {
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

    if (c->owner->bridge == NULL) {
        return 0;
    }
    n = nb_ch(c);
    switch (c->owner->dev->extended) {
        case EXT_SC16IS_DLD: {
            unsigned mode = (n->dld.v >> 4) & 3u;

            return mode == 0 ? 16 : mode == 1 ? 8 : 4;   /* DLD[5:4]: 16x, 8x, 4x */
        }
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
    if (is_xr(d)) {
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
    if (c->owner->dev->bridge == NULL || !is_xr(c->owner->dev)) {
        return 0;
    }
    *out = xr_rx_trigger(c);
    return 1;
}

int ns_bridge_ier_mask(const NsChannel *c, uint8_t *mask) {
    if (c->owner->dev->bridge == NULL) {
        return 0;
    }
    *mask = enhanced(c) ? 0xFF : 0x0F;   /* IER[7:4] need EFR[4] */
    return 1;
}

int ns_bridge_mcr_mask(const NsChannel *c, uint8_t *mask) {
    int e;

    if (c->owner->dev->bridge == NULL) {
        return 0;
    }
    e = enhanced(c);
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
    return sc_gpio_pending(c->owner);
}

LogicValue ns_bridge_ls_source(const NsChannel *c, LogicValue normal) {
    if (is_xr(c->owner->dev)) {
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
    if (is_xr(c->owner->dev) && c->owner->bridge != NULL) {
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
    if (is_xr(c->owner->dev)) {
        xr_thr_written(c, at);
    }
}

int ns_bridge_tx_fetch(NsChannel *c) {
    return is_xr(c->owner->dev) ? xr_tx_fetch(c) : 0;
}

int ns_bridge_clock(NsState *s, NsChannel *c, int rising, LogicValue level) {
    (void)level;
    if (!is_xr(s->dev)) {
        return 0;
    }
    return xr_clock(s, c, rising);
}

int ns_bridge_sleep_check(NsState *s, NsChannel *c) {
    return is_xr(s->dev) ? xr_sleep_check(s, c) : 1;
}

void ns_bridge_sleep_changed(NsState *s, NsChannel *c) {
    if (s->bridge != NULL && is_xr(s->dev)) {
        xr_sleep_changed(s, c);
    }
}
