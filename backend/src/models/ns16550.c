#include "ns16550_core_int.h"
#include "logic_alloc.h"

const char *const nsc_TIMING[] = {
    "trvd", "thz", "trdd", "tbld", "trint", "thr", "tmdo", "tsim", "trim", "twxi", "twrr", "tir",
    "tcso",
    /* plan-12-I: SPI SCLK to SO, CS to SO float, I2C SCL to SDA valid, GPIO output */
    "tdo", "ttr", "tvd", "tgpio", NULL
};
const char *const nsc_KEYS[] = {
    "bus", "clock", "channels", "fifo", "rxTrigger", "intPin", "dmaClass", "timeout",
    /* plan-12-D */
    "channelSelect", "broadcastWrite", "rxTrigger64", "txTrigger", "tlr", "dmaPins", "dma",
    "intControl", "lcrReset", "sprReset", "tcrReset", "iirPriority", "lsr7Clear", "stopCheck",
    "statusReadDefer", "busSync", "majorityVote",
    "extended", "afr", "afrReset", "dlf", "autoFlow", "rtsTable", "xoffTable", "swFlow",
    "specialChar", "specialClear", "specialNeedsNoFlow", "ctsRtsClear", "sleep", "irda", "rs485",
    "prescaler", "lsrErrors", "mcrReset", "xonReset", "tcrNeedsDlab0", "dlabHidesUpper",
    "idRegs", "timeoutDelay", "divisorZero", "printerPort", "tristate", "absentTies",
    "optionalPins", "sprPowerUp", "dmaMode1", "lsrClearOnGood", "breakMarkTicks",
    /* plan-12-I: the I2C/SPI bridges */
    "interrupt", "gpio", "fastIrda", "dlPowerUp", "straps", "sleepPin",
    /* plan-12-E (ns16550_exar.c) */
    "triggerTables", "fifoCounters", "dld", "sampling", "xonAny", "powerSave", "mfPins",
    "startSample", "thrIntClear", "isrStatus", "resetOut", "divisorPowerUp", "sleepNeedsDivisor",
    "wakeIndicator", "mcr2Enhanced", "xonWriteOnly", "autoFlowLoopback", "lsrBreakHold",
    "statusWrite", "dlabHidesStatus", "fstat", "dmaHost", "chcClock", "timer",
    "variant5g", "irdaStrap",
    NULL
};
/* The plan-12-I keys, which need a serial host "bus". */
const char *const nsc_BRIDGE_KEYS[] = {
    /* not "interrupt": plan-12-E's 5G devices use it too (an object there) */
    "gpio", "fastIrda", "dlPowerUp", "straps", "sleepPin", NULL
};
const char *const nsc_CHANNEL_KEYS[] = {
    "tx", "rx", "rts", "dtr", "cts", "dsr", "dcd", "ri", "out1", "out2", "int", "txrdy", "rxrdy",
    NULL
};
/* Keys of a channel object besides its pins. */
const char *const nsc_CHANNEL_EXTRA_KEYS[] = {"select", "selectHigh", "op", "mf",
                                                 "bclk", "irtx" /* plan-12-E */, NULL};


const ChannelPin MODEM_PIN[4] = {CH_CTS, CH_DSR, CH_RI, CH_DCD};

void ns_drive(NsState *s, size_t pin, LogicValue value, SimTime delay) {
    if (!SERIAL_REAL_PIN(pin) || s->out[pin] == value) {
        return;
    }
    if (s->bridge != NULL && ns_bridge_owns(s, pin)) {
        return;   /* plan-12-I: a GPIO/modem pin the bridge drives */
    }
    s->out[pin] = value;
    /* TRI high (TL16C552): every output floats. */
    sim_schedule_inertial_drive(s->sim, s->instance, pin, s->tri ? LOGIC_Z : value, DRIVE_STRONG,
                                delay);
}

/* BAUDOUT: a clock, so a plain propagation delay (an inertial drive would
   swallow pulses shorter than tbld). */
void nsc_drive_baudout(NsState *s, LogicValue value, SimTime delay) {
    size_t pin = s->pins->baudout;

    if (!SERIAL_REAL_PIN(pin) || s->out[pin] == value) {
        return;
    }
    s->out[pin] = value;
    sim_schedule_drive(s->sim, s->instance, pin, value, DRIVE_STRONG, delay);
}

int nsc_loopback(const NsChannel *c) {
    return c->mcr.v & 0x10;
}

void nsc_update_format(NsChannel *c) {
    uint8_t lcr = c->lcr.v;
    SerialFormat f = serial_format_default();

    f.data_bits = 5 + (lcr & 3u);
    f.stop_halves = lcr & 4u ? (f.data_bits == 5 ? 3 : 4) : 2;
    if (!(lcr & 8u)) {
        f.parity = SERIAL_PARITY_NONE;
    } else if (lcr & 0x20u) {
        f.parity = lcr & 0x10u ? SERIAL_PARITY_SPACE : SERIAL_PARITY_MARK;
    } else {
        f.parity = lcr & 0x10u ? SERIAL_PARITY_EVEN : SERIAL_PARITY_ODD;
    }
    f.format_x = (c->lcr.k & 0x3Fu) != 0x3Fu;
    f.majority = c->owner->dev->majority;
    f.check_both_stops = c->owner->dev->stop_both;
    f.mark_ticks = c->owner->dev->break_mark_ticks;
    f.factor = ns_ext_factor(c);
    if (c->owner->dev->x != NULL) {
        f.sample = ns_x_start_sample(c, f.factor);   /* plan-12-E */
    }
    c->format = f;
}

void nsc_load_divisor(NsChannel *c) {
    c->baud.low2 = c->owner->dev->baudout_low2;
    ns_ext_load_divisor(c);
    /* COM92C451: divisor 0 divides by 3. */
    if (c->baud.known && c->baud.divisor == 0 && c->owner->dev->divisor_zero != 0) {
        c->baud.divisor = c->owner->dev->divisor_zero;
        c->baud.baudout = LOGIC_1;
    }
}

/* ---- FIFOs ---------------------------------------------------------------------------- */

int nsc_char_has_error(const SerialRxChar *c) {
    return (c->flags & (SERIAL_RX_PE | SERIAL_RX_FE | SERIAL_RX_BI)) != 0 ||
           (c->flags_known & (SERIAL_RX_PE | SERIAL_RX_FE | SERIAL_RX_BI)) !=
               (SERIAL_RX_PE | SERIAL_RX_FE | SERIAL_RX_BI);
}

/* LSR[4:2] bits of a character: PE -> 2, FE -> 3, BI -> 4. */
void nsc_char_lsr_bits(const SerialRxChar *rc, uint8_t *v, uint8_t *k) {
    *v = 0;
    *k = 0x1C;
    if (rc->flags & SERIAL_RX_PE) *v |= 0x04;
    if (rc->flags & SERIAL_RX_FE) *v |= 0x08;
    if (rc->flags & SERIAL_RX_BI) *v |= 0x10;
    if (!(rc->flags_known & SERIAL_RX_PE)) *k &= (uint8_t)~0x04;
    if (!(rc->flags_known & SERIAL_RX_FE)) *k &= (uint8_t)~0x08;
    if (!(rc->flags_known & SERIAL_RX_BI)) *k &= (uint8_t)~0x10;
}

/* ORs a character's error bits into LSR[4:2] (it reached the top). */
void nsc_show_errors(NsChannel *c, const SerialRxChar *rc) {
    uint8_t v;
    uint8_t k;

    uint8_t ones;
    uint8_t unknown;

    nsc_char_lsr_bits(rc, &v, &k);
    ones = v & k;
    unknown = (uint8_t)(0x1C & ~k);
    /* A known 1 sets the bit; an unknown one makes it X unless already 1. */
    c->lsr_err |= ones;
    c->lsr_err_k |= ones;
    c->lsr_err_k &= (uint8_t)~(unknown & ~(c->lsr_err & c->lsr_err_k));
}

void ns_clear_rx(NsChannel *c) {
    c->rx_head = 0;
    c->rx_count = 0;
    c->timeout = 0;
    c->idle_ticks = 0;
    c->rxrdy1 = 0;
}

void ns_set_thre(NsChannel *c) {
    c->thre = 1;
    c->thre_wait = 0;
    /* With a TX trigger the THR interrupt comes at the trigger, not at empty. */
    if (ns_ext_tx_threshold(c) == 0 ||
        (c->owner->dev->x != NULL && ns_x_thre_at_empty(c))) {   /* plan-12-E */
        c->thre_int = 1;
    }
    c->two_seen = 0;
    c->immediate = 0;
}

void ns_clear_tx(NsChannel *c) {
    c->tx_head = 0;
    c->tx_count = 0;
    c->flow_out_count = 0;
    if (!c->thre) {
        ns_set_thre(c);
    }
}

/* The RX trigger level in force: FCR (or the 750's 64-byte table), unless
   TLR overrides it; 1 with the FIFOs off. */
int ns_rx_level(const NsChannel *c) {
    if (!c->fifo_on) {
        return 1;
    }
    return (int)ns_ext_rx_trigger(c, rx_trigger_level(c));
}

/* Free TX FIFO spaces. */
unsigned nsc_tx_free(const NsChannel *c) {
    return depth(c) - c->tx_count;
}

static LogicValue any_bits(uint8_t v, uint8_t k, uint8_t mask) {
    if (v & k & mask) {
        return LOGIC_1;
    }
    return (k & mask) == mask ? LOGIC_0 : LOGIC_X;
}

static Sources sources(const NsChannel *c) {
    Sources r;
    LogicValue rda;

    const NsDevice *d = c->owner->dev;
    LogicValue thre = bool3(c->thre_int);

    rda = bool3(c->rx_count >= (unsigned)ns_rx_level(c) && c->rx_count > 0);
    if (d->lsr_head) {
        /* 752 style: OE (until an LSR read), or an errored byte anywhere in the FIFO. */
        r.ls = and3(or3(any_bits(c->lsr_err, c->lsr_err_k, 0x02), bool3(c->lsr7)),
                    bit_of(c->ier, 2));
    } else {
        r.ls = and3(any_bits(c->lsr_err, c->lsr_err_k, 0x1E), bit_of(c->ier, 2));
    }
    r.rda = and3(rda, bit_of(c->ier, 0));
    r.timeout = and3(bool3(c->fifo_on && c->timeout), bit_of(c->ier, 0));
    /* RS-485 with 485LG = 0 (TI AFR): TX interrupts wait for the TSR. */
    if (d->rs485 == RS485_AFR_DTR && bit_of(c->afr, 2) == LOGIC_1 && bit_of(c->afr, 3) != LOGIC_1 &&
        serial_tx_busy(&c->line.tx)) {
        thre = LOGIC_0;
    }
    r.thre = and3(thre, bit_of(c->ier, 1));
    r.modem = and3(any_bits(c->delta, c->delta_k, 0x0F), bit_of(c->ier, 3));
    r.xoff = ns_ext_xoff_source(c);
    r.ctsrts = ns_ext_ctsrts_source(c);
    r.gpio = LOGIC_0;
    r.wake = LOGIC_0;
    if (d->x != NULL) {
        ns_x_sources(c, &r.ls, &r.rda, &r.thre, &r.wake);   /* plan-12-E */
    }
    if (d->bridge != NULL) {
        /* plan-12-I: the GPIO source; the bridges' line-status and THR rules. */
        r.gpio = ns_bridge_gpio_source(c);
        r.ls = ns_bridge_ls_source(c, r.ls);
        r.thre = ns_bridge_thr_source(c, r.thre);
    }
    return r;
}

void ns_sources(const NsChannel *c, LogicValue out[8]) {
    Sources r = sources(c);

    out[0] = r.ls;
    out[1] = r.rda;
    out[2] = r.timeout;
    out[3] = r.thre;
    out[4] = r.modem;
    out[5] = r.xoff;
    out[6] = r.ctsrts;
    out[7] = r.gpio;
}

/* IIR: the highest pending source. Unknown when an X decides it. */
void nsc_iir_of(const NsChannel *c, uint8_t *v, uint8_t *k) {
    Sources s = sources(c);
    /* Standard: RX data (trigger reached) above the timeout; nxpEqual: one
       level, shown as the timeout while one is pending. */
    int timeout_first = c->owner->dev->iir_priority == IIR_NXP_EQUAL ||
                        c->owner->dev->iir_priority == IIR_EXAR_TIMEOUT_FIRST;   /* plan-12-E */
    /* plan-12-I: the SC16IS7xx GPIO source (0x30) between modem and Xoff. */
    const LogicValue order[8] = {s.ls, timeout_first ? s.timeout : s.rda,
                                 timeout_first ? s.rda : s.timeout, s.thre, s.modem, s.gpio,
                                 s.xoff, s.ctsrts};
    const uint8_t ID[8] = {0x06, timeout_first ? 0x0C : 0x04, timeout_first ? 0x04 : 0x0C,
                           0x02, 0x00, 0x30, 0x10, 0x20};
    unsigned i;

    *v = 0x01;
    *k = 0xFF;
    for (i = 0; i < 8; i++) {
        if (order[i] == LOGIC_X) {
            *k = 0;
            break;
        }
        if (order[i] == LOGIC_1) {
            *v = ID[i];
            break;
        }
    }
    if (c->fifo_on) {
        *v |= 0xC0;
        if (c->owner->dev->fifo750 && c->fifo64) {
            *v |= 0x20;   /* TL16C750: IIR[5] in 64-byte mode */
        }
    }
    if (c->fifo_x) {
        *k &= (uint8_t)~0xC0;
    }
    if (c->owner->dev->x != NULL) {
        ns_x_iir(c, v, k);   /* plan-12-E: ST16C1450/1550 ISR[5:4] */
    }
    if (c->owner->dev->bridge != NULL) {
        ns_bridge_iir_bits(c, v, k);   /* plan-12-I: XR20M1280 ISR[6] */
    }
}

static LogicValue int_level(const NsChannel *c) {
    Sources s = sources(c);

    return or3(or3(or3(s.ls, s.rda), or3(s.timeout, s.thre)),
               or3(or3(s.modem, or3(s.gpio, s.wake)), or3(s.xoff, s.ctsrts)));
}

/* plan-12-E: the interrupt level, for ns16550_exar.c. */
LogicValue ns_int_level(const NsChannel *c) {
    return int_level(c);
}

void ns_iir_now(const NsChannel *c, uint8_t *v, uint8_t *k) {
    nsc_iir_of(c, v, k);
}

/* ---- Outputs ----------------------------------------------------------------------------- */

/* RXRDY (active low) for one channel. Mode 0: low with a character.
   Mode 1: the latch (trigger or timeout; cleared when empty); class T also
   goes inactive while LSR[7] is 1. */
static LogicValue rxrdy_level(const NsChannel *c, int mode1) {
    if (!mode1) {
        return bool3(c->rx_count == 0);
    }
    if (c->owner->dev->dma_class == 'T' && c->lsr7) {
        return LOGIC_1;
    }
    return bool3(!c->rxrdy1);
}

/* TXRDY (active low). Mode 0: low when the THR/FIFO is empty. Mode 1 by
   class: P low when empty, high when full (a latch); N and B low while a
   slot is free; T low while the free spaces reach the TX trigger. */
static LogicValue txrdy_level(const NsChannel *c, int mode1) {
    unsigned free_slots = depth(c) - c->tx_count;

    if (!mode1) {
        return bool3(c->tx_count != 0);
    }
    switch (c->owner->dev->dma_class) {
        case 'N':
        case 'B':
            return bool3(free_slots == 0);
        case 'T': {
            unsigned level = ns_ext_tx_threshold(c);

            return bool3(free_slots < (level != 0 ? level : 1));
        }
        default:
            return bool3(!c->txrdy1);
    }
}

/* The INT pin for an interrupt level, per "intPin": driven; OUT2-gated
   (3-state unless MCR[3] = 1, the register bit, loopback or not); INTN
   (pin high: driven, low or open: OUT2-gated); INTSEL (enabled by the pin OR
   MCR[3]); MCR[5] open source (SC16C650B: MCR[5] = 1 drives only the 1). */
static LogicValue int_pin_level(const NsState *s, const NsChannel *c, LogicValue level) {
    LogicValue out2 = bit_of(c->mcr, 3);
    LogicValue enable;
    LogicValue x_out;

    if ((s->dev->x != NULL || s->xs.part != NULL) && ns_x_int_pin(s, c, level, &x_out)) {
        return x_out;   /* plan-12-E: Motorola IRQ, 5G INT, part "intPin" */
    }
    switch (s->dev->int_pin) {
        case INT_DRIVEN:
            return level;
        case INT_OUT2_GATED:
            enable = out2;
            break;
        case INT_INTN:
            enable = or3(serial_level(s->sim, s->instance, s->pins->int_control, LOGIC_0), out2);
            break;
        case INT_INTSEL:
            enable = or3(serial_level(s->sim, s->instance, s->pins->int_control, LOGIC_X), out2);
            break;
        case INT_MCR5_OPEN_SOURCE: {
            LogicValue open = bit_of(c->mcr, 5);

            if (open == LOGIC_0) {
                return level;
            }
            if (open == LOGIC_1) {
                return level == LOGIC_1 ? LOGIC_1 : level == LOGIC_0 ? LOGIC_Z : LOGIC_X;
            }
            return level == LOGIC_1 ? LOGIC_1 : LOGIC_X;
        }
        default:
            return level;
    }
    if (enable == LOGIC_1) {
        return level;
    }
    return enable == LOGIC_0 ? LOGIC_Z : LOGIC_X;
}

static void update_channel_pins(NsState *s, NsChannel *c, Cause cause);

void nsc_update_channel_outputs(NsState *s, NsChannel *c, Cause cause) {
    update_channel_pins(s, c, cause);
    /* The shared DMA pins follow every channel, not only bus cycles (a byte
       received or sent with no access moves them too). */
    nsc_update_shared_dma(s, cause);
    if (s->bridge != NULL) {
        ns_bridge_outputs(s, cause);   /* plan-12-I: IRQ, GPIO and modem pins */
    }
}

static void update_channel_pins(NsState *s, NsChannel *c, Cause cause) {
    const size_t *pin = s->pins->channel[c->index];
    SimTime int_delay = 0;
    SimTime modem_delay = 0;
    SimTime txrdy_delay = 0;
    SimTime rxrdy_delay = 0;
    LogicValue loop = bit_of(c->mcr, 4);
    LogicValue sout;
    LogicValue intr;
    LogicValue rxrdy;
    LogicValue txrdy;
    int mode1 = c->fifo_on && c->dma_mode;
    unsigned i;

    switch (cause) {
        case CAUSE_RBR_READ: int_delay = t_of(s, T_RINT); rxrdy_delay = t_of(s, T_WRR); break;
        case CAUSE_LSR_READ: int_delay = t_of(s, T_RINT); break;
        case CAUSE_IIR_READ: int_delay = t_of(s, T_IR); break;
        case CAUSE_MSR_READ: int_delay = t_of(s, T_RIM); break;
        case CAUSE_THR_WRITE: int_delay = t_of(s, T_HR); txrdy_delay = t_of(s, T_WXI); break;
        case CAUSE_MCR_WRITE: modem_delay = t_of(s, T_MDO); int_delay = t_of(s, T_MDO); break;
        case CAUSE_MODEM_IN: int_delay = t_of(s, T_SIM); break;
        case CAUSE_SERIAL:
        case CAUSE_WRITE:
            break;
    }
    ns_ext_pins(s, c, cause);

    if (s->unknown) {
        for (i = 0; i < CH_PIN_COUNT; i++) {
            if (i != CH_RX && i != CH_CTS && i != CH_DSR && i != CH_DCD && i != CH_RI) {
                ns_drive(s, pin[i], LOGIC_X, 0);
            }
        }
        if (s->dev->x != NULL) {
            ns_x_device_pins(s, cause);   /* plan-12-E */
        }
        return;
    }
    if (s->in_reset) {
        ns_drive(s, pin[CH_TX], LOGIC_1, 0);
        ns_drive(s, pin[CH_RTS], LOGIC_1, 0);
        ns_drive(s, pin[CH_DTR], LOGIC_1, 0);
        ns_drive(s, pin[CH_OUT1], LOGIC_1, 0);
        ns_drive(s, pin[CH_OUT2], LOGIC_1, 0);
        ns_drive(s, pin[CH_INT], int_pin_level(s, c, LOGIC_0), 0);
        /* decisions.md (TL16C554, P3): RXRDY and TXRDY inactive during reset. */
        ns_drive(s, pin[CH_RXRDY], LOGIC_1, 0);
        ns_drive(s, pin[CH_TXRDY], LOGIC_1, 0);
        if (s->dev->x != NULL) {
            ns_x_device_pins(s, cause);   /* plan-12-E */
        }
        return;
    }

    /* SOUT: mark in loopback; 0 while break is set; else the transmitter. */
    sout = c->tsr;
    {
        LogicValue brk = bit_of(c->lcr, 6);

        if (brk == LOGIC_1) {
            sout = LOGIC_0;
        } else if (brk == LOGIC_X) {
            sout = sout == LOGIC_0 ? LOGIC_0 : LOGIC_X;
        }
    }
    sout = ns_ext_tx_pin(c, sout);   /* IrDA encodes after break */
    if (loop == LOGIC_1) {
        sout = LOGIC_1;
    } else if (loop == LOGIC_X && sout != LOGIC_1) {
        sout = LOGIC_X;
    }
    ns_drive(s, pin[CH_TX], sout, 0);

    /* Modem outputs: MCR bits inverted; forced high (inactive) in loopback.
       Auto-RTS drives RTS; RS-485 drives DTR. */
    {
        static const ChannelPin OUT_PIN[4] = {CH_DTR, CH_RTS, CH_OUT1, CH_OUT2};

        for (i = 0; i < 4; i++) {
            LogicValue level = not3(bit_of(c->mcr, i));

            if (i == 0) {
                level = ns_ext_dtr(c, level);
            } else if (i == 1) {
                level = ns_ext_rts(c, level);
            } else if (i == 2 && s->dev->x != NULL) {
                level = ns_x_out1(c, level);   /* plan-12-E: XR16C864 OP1 RS-485 */
            }
            level = or3(level, loop);
            ns_drive(s, pin[OUT_PIN[i]], level, modem_delay);
        }
    }

    /* INT: active high; driven, gated or open per "intPin". */
    intr = int_pin_level(s, c, int_level(c));
    ns_drive(s, pin[CH_INT], intr, int_delay);

    /* RXRDY / TXRDY (active low). */
    rxrdy = rxrdy_level(c, mode1);
    txrdy = txrdy_level(c, mode1);
    if (s->dev->x != NULL) {
        ns_x_dma_pins(c, &rxrdy, &txrdy);   /* plan-12-E: XR16C864 DRQ */
    }
    ns_drive(s, pin[CH_RXRDY], rxrdy, rxrdy_delay);
    ns_drive(s, pin[CH_TXRDY], txrdy, txrdy_delay);
    if (s->dev->x != NULL) {
        ns_x_device_pins(s, cause);   /* plan-12-E: IRQ/INT#, RST, FSTAT */
    }
}

/* Shared DMA pins (TL16C554: the channels' active-low signals ANDed;
   TL16C754B: wire-OR, open drain). */
void nsc_update_shared_dma(NsState *s, Cause cause) {
    const NsDevice *d = s->dev;
    LogicValue rx = LOGIC_1;
    LogicValue tx = LOGIC_1;
    SimTime rx_delay = cause == CAUSE_RBR_READ ? t_of(s, T_WRR) : 0;
    SimTime tx_delay = cause == CAUSE_THR_WRITE ? t_of(s, T_WXI) : 0;
    unsigned i;

    if (d->dma_pins == DMA_PINS_NAMED) {
        return;
    }
    if (s->unknown) {
        rx = tx = LOGIC_X;
    } else if (!s->in_reset) {
        for (i = 0; i < d->channel_count; i++) {
            const NsChannel *c = &s->ch[i];
            int mode1 = c->fifo_on && c->dma_mode;

            rx = and3(rx, rxrdy_level(c, mode1));
            tx = and3(tx, txrdy_level(c, mode1));
        }
    }
    if (d->dma_pins == DMA_PINS_SHARED_WIRE_OR) {
        rx = rx == LOGIC_1 ? LOGIC_Z : rx;
        tx = tx == LOGIC_1 ? LOGIC_Z : tx;
    }
    ns_drive(s, s->pins->dma_rxrdy, rx, rx_delay);
    ns_drive(s, s->pins->dma_txrdy, tx, tx_delay);
}

void nsc_update_outputs(NsState *s, Cause cause) {
    unsigned i;

    for (i = 0; i < s->dev->channel_count; i++) {
        nsc_update_channel_outputs(s, &s->ch[i], cause);
    }
    nsc_update_shared_dma(s, cause);
    ns_lpt_pins(s);
    serial_bus_refresh(&s->bus, s->sim, s->instance, &s->events);
}

void ns_update_outputs(NsState *s, Cause cause) {
    nsc_update_outputs(s, cause);
}

void ns_update_channel(NsState *s, NsChannel *c, Cause cause) {
    nsc_update_channel_outputs(s, c, cause);
}

void nsc_update_dma_latches(NsChannel *c) {
    if (c->rx_count == 0) {
        c->rxrdy1 = 0;
    } else if (c->rx_count >= (c->owner->dev->x != NULL ? (unsigned)ns_rx_level(c)
                                                        : rx_trigger_level(c)) ||   /* plan-12-E */
               c->timeout) {
        c->rxrdy1 = 1;
    }
    if (c->tx_count == 0) {
        c->txrdy1 = 1;
    } else if (c->tx_count >= depth(c)) {
        c->txrdy1 = 0;
    }
}

/* ---- Modem status --------------------------------------------------------------------------- */

/* Sets the status bits to `now`, recording deltas (TERI: RI 1 -> 0). */
void nsc_set_modem(NsChannel *c, const LogicValue now[4], uint8_t record) {
    unsigned i;

    for (i = 0; i < 4; i++) {
        LogicValue before = c->msr[i];
        uint8_t bit = (uint8_t)(1u << i);
        int changed;
        int unknown;

        if (before == now[i]) {
            continue;
        }
        if (i == M_RI) {
            changed = before == LOGIC_1 && now[i] == LOGIC_0;
            unknown = !changed && (before == LOGIC_X ? now[i] != LOGIC_1 : before == LOGIC_1);
        } else {
            changed = before != LOGIC_X && now[i] != LOGIC_X;
            unknown = !changed;
        }
        if (!(record & bit)) {
            c->msr[i] = now[i];
            continue;
        }
        if (changed) {
            c->delta |= bit;
            c->delta_k |= bit;
        } else if (unknown && !((c->delta & c->delta_k) & bit)) {
            c->delta_k &= (uint8_t)~bit;
        }
        c->msr[i] = now[i];
    }
}

/* The status the modem bits should have now: the pins, or MCR in loopback. */
void nsc_modem_now(const NsState *s, const NsChannel *c, LogicValue now[4]) {
    LogicValue loop = bit_of(c->mcr, 4);
    unsigned i;

    for (i = 0; i < 4; i++) {
        size_t pin = s->pins->channel[c->index][MODEM_PIN[i]];
        /* Absent pins read X (decisions.md); present ones are active low.
           plan-12-I: a bridge's modem pin that is a GPIO now reads as absent. */
        LogicValue z_reads = LOGIC_X;
        LogicValue from_pin = pin == SERIAL_NO_PIN ||
                                      (s->bridge != NULL &&
                                       !ns_bridge_modem_input(s, c->index, pin, &z_reads))
                                  ? LOGIC_X
                                  : not3(serial_level(s->sim, s->instance, pin, z_reads));
        /* Loopback: RTS -> CTS, DTR -> DSR, OUT1 -> RI, OUT2 -> DCD. */
        static const unsigned FROM_MCR[4] = {1, 0, 2, 3};
        LogicValue from_mcr = bit_of(c->mcr, FROM_MCR[i]);

        if (loop == LOGIC_1) {
            now[i] = from_mcr;
        } else if (loop == LOGIC_0) {
            now[i] = from_pin;
        } else {
            now[i] = from_pin == from_mcr ? from_pin : LOGIC_X;
        }
    }
}

/* ---- Reset -------------------------------------------------------------------------------- */

static void reset_channel(NsState *s, NsChannel *c, int power_on) {
    LogicValue now[4];

    c->ier = (Reg){0, 0xFF};
    c->lcr = (Reg){s->dev->lcr_reset, 0xFF};
    c->mcr = (Reg){0, 0xFF};
    if (s->dev->spr_reset >= 0) {
        c->scr = (Reg){(uint8_t)s->dev->spr_reset, 0xFF};
    }
    c->fifo_on = 0;
    c->fifo_x = 0;
    c->fifo64 = 0;
    c->reading_status = 0;
    c->dma_mode = 0;
    c->trigger = 0;
    ns_clear_rx(c);
    c->pending_count = 0;
    c->lsr_err = 0;
    c->lsr_err_k = 0xFF;
    c->lsr7 = 0;
    c->tx_head = 0;
    c->tx_count = 0;
    c->thre = 1;
    c->thre_wait = 0;
    c->thre_int = 0;
    c->two_seen = 0;
    c->immediate = 0;
    c->delta = 0;
    c->delta_k = 0xFF;
    c->txrdy1 = 1;
    serial_tx_reset(&c->line.tx);
    serial_rx_reset(&c->line.rx);
    c->tsr = LOGIC_1;
    ns_ext_reset(s, c, power_on);
    nsc_update_format(c);
    if (s->sim != NULL) {
        nsc_modem_now(s, c, now);
        memcpy(c->msr, now, sizeof now);
    }
}

void nsc_reset_all(NsState *s) {
    unsigned i;

    for (i = 0; i < s->dev->channel_count; i++) {
        reset_channel(s, &s->ch[i], 0);
    }
    ns_lpt_reset(s);
}

void ns_reset_all(NsState *s) {
    nsc_reset_all(s);
}

void ns_modem_refresh(NsState *s, NsChannel *c) {
    LogicValue now[4];

    nsc_modem_now(s, c, now);
    memcpy(c->msr, now, sizeof now);
}

/* ---- plan-12-E: core entry points for ns16550_exar.c ------------------------------------------ */

void ns_reg_peek(NsChannel *c, unsigned address, uint8_t *v, uint8_t *k) {
    nsc_reg_peek(c, address, v, k);
}

Cause ns_reg_read_done(NsChannel *c, unsigned address) {
    return nsc_reg_read_done(c, address);
}

Cause ns_reg_write(NsState *s, NsChannel *c, unsigned address, uint8_t v, uint8_t k) {
    return nsc_reg_write(s, c, address, v, k);
}

void ns_reset_channel(NsState *s, NsChannel *c) {
    reset_channel(s, c, 0);
}

/* ---- Terminal ---------------------------------------------------------------------------------- */

unsigned nsc_ns_channels(const void *raw) {
    return ((const NsState *)raw)->dev->channel_count;
}

SerialLine *nsc_ns_line(void *raw, unsigned channel) {
    return &((NsState *)raw)->ch[channel].line;
}

int nsc_ns_async(const void *raw, unsigned channel) {
    (void)raw;
    (void)channel;
    return 1;
}

/* ---- Create / destroy ------------------------------------------------------------------------------ */

static int d_needs_select(const NsDevice *d) {
    return d->channel_pin != NULL || d->per_channel_select || d->lpt != NULL ||
           (d->x != NULL && ns_x_needs_select(d));   /* plan-12-E */
}

void *nsc_ns_create(const Part *part, const void *data, const void *pins,
                       const SerialGrade *grade, const void *options) {
    NsState *s = logic_calloc(1, sizeof *s);
    unsigned i;

    if (s == NULL) {
        return NULL;
    }
    s->xs.part = options;   /* plan-12-E: the part's "intPin" */
    s->base.model = &NS16550_MODEL;
    s->part = part;
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
    if (s->dev->lpt != NULL) {
        s->lpt = ns_lpt_create(s);
        if (s->lpt == NULL) {
            logic_free(s->out);
            logic_free(s);
            return NULL;
        }
    }
    {
        SerialBusTiming timing = {grade->t[T_RVD], grade->t[T_HZ], grade->t[T_RDD], 0};

        serial_bus_init(&s->bus, &s->pins->bus, &timing);
    }
    s->events.context = s;
    s->events.select = d_needs_select(s->dev) ? nsc_bus_select : NULL;
    s->events.peek = nsc_bus_peek;
    s->events.read_done = nsc_bus_read_done;
    s->events.write = nsc_bus_write;
    for (i = 0; i < s->dev->channel_count; i++) {
        NsChannel *c = &s->ch[i];

        c->owner = s;
        c->index = i;
        serial_line_init(&c->line, s->pins->channel[i][CH_RX]);
        /* Power-on: MR's state; DLL, DLM and RBR are not reset, so X. */
        c->dll = (Reg){0, 0};
        c->dlm = (Reg){0, 0};
        c->rbr = (Reg){0, 0};
        c->scr = s->dev->spr_power_up >= 0 ? (Reg){(uint8_t)s->dev->spr_power_up, 0xFF} : (Reg){0, 0};
        c->msr[0] = c->msr[1] = c->msr[2] = c->msr[3] = LOGIC_X;
        reset_channel(s, c, 1);
        nsc_load_divisor(c);
    }
    s->clock_level = LOGIC_X;
    s->receive_level = LOGIC_X;
    if (s->dev->bridge != NULL && ns_bridge_create(s) != 0) {
        ns_lpt_destroy(s->lpt);
        logic_free(s->out);
        logic_free(s);
        return NULL;
    }
    return s;
}

void nsc_ns_destroy(void *raw) {
    NsState *s = raw;
    unsigned i;

    for (i = 0; i < s->dev->channel_count; i++) {
        serial_line_free(&s->ch[i].line);
    }
    ns_lpt_destroy(s->lpt);
    if (s->bridge != NULL) {
        ns_bridge_destroy(s);
    }
    logic_free(s->out);
    logic_free(s);
}

