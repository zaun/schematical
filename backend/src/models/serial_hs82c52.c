/*
 * hs82c52: the Harris/Intersil 82C52 serial controller and the HD-6406
 * PACI, which is the 82C52 core with more pins (plan-12-H section 3.5; facts
 * in plan-12-data/specs/classic-b.md sections 82C52 and HD-6406,
 * decisions.md sections 82C52 and HD-6406).
 *
 * Bus: Intel RD/WR with CS0 low (and CS1 high on the HD-6406); A1..A0 pick
 * TBR/RBR, UCR/USR, MCR, BRSR/MSR (Table 1). The HD-6406's ALE latches
 * A1..A0 (transparent while high, held when it falls); RXDACK low reads the
 * RBR and TXDACK low writes the TBR whatever the address (decisions.md: an
 * RXDACK read with DR = 0 drives X, a TXDACK write with TBRE = 0 is
 * ignored). serial_bus.c runs the cycles; the "unit" it hands back is the
 * register (0-3), 4 for an RXDACK read and 5 for a TXDACK write.
 *
 * Baud: IX is counted through the BRSR prescaler (1, 3, 4, 5) and divisor
 * (Fig 2, with 16/3, 32/3 and 58/3 spread over three periods; 11111 =
 * external, divide by 1; decisions.md: HD-6406 00111 = 22, 01101 = 288; the
 * unlisted codes stop the generator and CO reads X when it shows the BRG).
 * The 16x tick is the falling edge of CO(BRG); TX and RX share it.
 *
 * USR: DR and TBRE mirror the pins; PE FE OE RBRK MS TC are latched and a
 * USR read clears them (decisions.md); a condition during the read is
 * applied at its end (p9 NOTE). INTR = INTEN and (a rising edge of
 * RBRK|TC|OE|FE|PE, or MIEN and an edge on a modem input; HD-6406 also SIE
 * and a rising edge of DR|TBRE, and SFD masking the USR group).
 */

#include <stdlib.h>
#include <string.h>

#include "serial_util.h"

enum { T_RLDV, T_RHDZ, T_CO, T_BRG, T_SDO, T_INTR, T_FLAG };
static const char *const TIMING[] = {"trldv", "trhdz", "tco", "tbrg", "tsdo", "tintr", "tflag", NULL};

static const char *const KEYS[] = {"bus", "clock", "serial", "hd6406", NULL};

static const char *const BUS_KEYS[] = {
    "data", "address", "read", "write", "select", "selectHigh", "addressLatch", "reset", NULL
};

enum { K_IN, K_OUT, K_CO, K_COUNT };
static const char *const CLOCK_KEYS[] = {"in", "out", "clockOut", NULL};

enum { P_TX, P_RX, P_CTS, P_DSR, P_DTR, P_RTS, P_INT, P_TBRE, P_DR, P_COUNT };
static const char *const SERIAL_KEYS[] = {"tx", "rx", "cts", "dsr", "dtr", "rts", "int", "tbre", "dr",
                                          NULL};

enum { X_RLSD, X_RI, X_TC, X_OE, X_FE, X_PE, X_SIE, X_SFD, X_TXDACK, X_RXDACK, X_COUNT };
static const char *const HD_KEYS[] = {"rlsd", "ri", "tc", "oe", "fe", "pe", "sie", "sfd", "txdack",
                                      "rxdack", NULL};

/* BRSR D6..D2: the divisor in thirds (0 = undefined: stopped). */
static const unsigned DIV3[32] = {
    6, 12, 16, 24, 32, 48, 58, 66, 96, 192, 384, 576, 768, 864, 1056, 1536, 2304,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3
};
static const unsigned PRESCALE[4] = {1, 3, 4, 5};

enum { U_PE = 1, U_FE = 2, U_OE = 4, U_RBRK = 8, U_MS = 16, U_TC = 32, U_LATCHED = 0x3F };

typedef struct {
    const char *data[8], *addr[2];
    const char *read, *write, *select, *select_high, *ale, *reset;
    const char *clock[K_COUNT];
    const char *pin[P_COUNT];
    const char *hd[X_COUNT];
    int is_hd;
} HsDevice;

typedef struct {
    SerialBusPins bus;
    size_t addr[2], select, select_high, ale, reset;
    size_t clock[K_COUNT];
    size_t pin[P_COUNT];
    size_t hd[X_COUNT];
} HsPins;

typedef struct {
    SerialBase base;
    const HsDevice *dev;
    const HsPins *pins;
    const SerialGrade *grade;
    Sim *sim;
    SimInstance *instance;
    SuOutputs o;
    SerialBus bus;
    SerialBusEvents events;
    SerialLine line;
    SerialFormat txf, rxf;

    int in_reset;
    int unknown;
    SuReg ucr, brsr, mcr;
    SuReg tbr, rbr;
    int tbre;                     /* the TBR is empty */
    int tbr_age;                  /* 16x ticks since the TBR was loaded with the TR idle */
    int staged;                   /* the TBR moved to the TR; it starts on the next tick */
    uint16_t stage_v, stage_k;
    int dr;
    uint8_t usr, usr_k;           /* the latched USR bits (U_*) */
    int usr_reading;
    uint8_t usr_pending, usr_at_read;
    int usr_edge;                 /* the USR-group edge detector */
    int sie_edge;                 /* HD-6406: DR | TBRE edge detector */
    int modem_edge;
    uint8_t last_usr;             /* the USR group bits the detector last saw */
    int last_dr, last_tbre;
    LogicValue modem_level[4];    /* CTS DSR RLSD RI pins */
    /* HD-6406 status pins. */
    LogicValue tc_pin, oe_pin, fe_pin, pe_pin;
    LogicValue tsr;
    /* Receiver pipeline: the character reaches the RBR 3 ticks after its sample. */
    struct { SerialRxChar c; unsigned ticks; } pending[4];
    unsigned pending_count;
    int resync;
    /* Baud generator. */
    unsigned pre_count, acc;
    LogicValue ix, co_brg;
    LogicValue a_latch[2];
} HsState;

static SimTime t_of(const HsState *s, unsigned i) {
    return s->grade->t[i];
}

/* ---- Format and BRG ----------------------------------------------------------------------- */

static unsigned mode_of(const HsState *s) {
    return (s->mcr.v >> 3) & 3u;
}

static void update_format(HsState *s) {
    SerialFormat t = serial_format_default();
    SerialFormat r;
    unsigned par = (s->ucr.v >> 1) & 7u;
    static const SerialParity TXP[8] = {
        SERIAL_PARITY_EVEN, SERIAL_PARITY_ODD, SERIAL_PARITY_EVEN, SERIAL_PARITY_ODD,
        SERIAL_PARITY_EVEN, SERIAL_PARITY_ODD, SERIAL_PARITY_NONE, SERIAL_PARITY_NONE
    };
    static const SerialParity RXP[8] = {
        SERIAL_PARITY_EVEN, SERIAL_PARITY_ODD, SERIAL_PARITY_ODD, SERIAL_PARITY_EVEN,
        SERIAL_PARITY_EVEN, SERIAL_PARITY_ODD, SERIAL_PARITY_NONE, SERIAL_PARITY_NONE
    };

    t.data_bits = 5 + ((s->ucr.v >> 4) & 3u);
    t.parity = TXP[par];
    t.stop_halves = (s->ucr.v & 1u) ? (t.data_bits == 5 ? 3 : 4) : 2;
    t.format_x = (s->ucr.k & 0x3Fu) != 0x3Fu;
    r = t;
    r.parity = RXP[par];   /* codes 100 / 101: the bit is there, not checked */
    r.stop_halves = (s->ucr.v & 1u) && t.data_bits != 5 ? 4 : 2;
    s->txf = t;
    s->rxf = r;
}

static int rx_checks_parity(const HsState *s) {
    unsigned par = (s->ucr.v >> 1) & 7u;

    return par < 4;
}

/* The divisor in thirds; 0 = stopped (undefined code or X). */
static unsigned div3(const HsState *s) {
    if ((s->brsr.k & 0x7Fu) != 0x7Fu) {
        return 0;
    }
    return DIV3[(s->brsr.v >> 2) & 31u];
}

static void brg_reset(HsState *s) {
    s->pre_count = 0;
    s->acc = 0;
    s->co_brg = LOGIC_1;
}

/* ---- Outputs ----------------------------------------------------------------------------- */

static LogicValue modem_active(const HsState *s, unsigned i) {
    return su_not(s->modem_level[i]);
}

/* The USR bits that feed the interrupt edge detector. */
static uint8_t usr_group(const HsState *s) {
    uint8_t mask = U_PE | U_FE | U_OE | U_RBRK | U_TC;
    LogicValue sfd = s->dev->is_hd ? serial_level(s->sim, s->instance, s->pins->hd[X_SFD], LOGIC_X)
                                   : LOGIC_0;

    if (sfd == LOGIC_1) {
        mask = 0;   /* SFD masks PE OE FE TC (and RBRK, decisions.md) */
    }
    return (uint8_t)(s->usr & mask);
}

static LogicValue intr_level(const HsState *s) {
    LogicValue inten = su_bit(s->mcr, 2);
    LogicValue mien = su_bit(s->mcr, 6);
    LogicValue any = su_or(su_bool(s->usr_edge), su_and(mien, su_bool(s->modem_edge)));

    if (s->dev->is_hd) {
        LogicValue sie = serial_level(s->sim, s->instance, s->pins->hd[X_SIE], LOGIC_X);

        any = su_or(any, su_and(sie, su_bool(s->sie_edge)));
    }
    return su_and(inten, any);
}

static LogicValue sdo_level(const HsState *s) {
    unsigned m = mode_of(s);

    if ((s->mcr.k & 0x18u) != 0x18u) {
        return LOGIC_X;
    }
    switch (m) {
        case 1: return serial_tx_busy(&s->line.tx) ? LOGIC_0 : LOGIC_1;   /* transmit break */
        case 2: return serial_level(s->sim, s->instance, s->pins->pin[P_RX], LOGIC_X);   /* echo */
        case 3: return LOGIC_1;                                              /* loop: mark */
        default: return s->tsr;
    }
}

static LogicValue co_level(const HsState *s) {
    LogicValue sel = su_bit(s->brsr, 7);

    if (sel == LOGIC_0) {
        return s->ix;
    }
    if (div3(s) == 0) {
        return LOGIC_X;
    }
    if (div3(s) == 3 && (s->brsr.v & 3u) == 0) {
        return s->ix;   /* external, divide by 1: the 16x clock is IX */
    }
    return su_mux(sel, s->co_brg, s->ix);
}

static void update_outputs(HsState *s, SimTime delay) {
    const size_t *p = s->pins->pin;
    const size_t *x = s->pins->hd;

    if (s->unknown) {
        static const unsigned OUTS[] = {P_TX, P_DTR, P_RTS, P_INT, P_TBRE, P_DR};
        unsigned i;

        for (i = 0; i < sizeof OUTS / sizeof OUTS[0]; i++) {
            su_drive(&s->o, s->sim, s->instance, p[OUTS[i]], LOGIC_X, 0);
        }
        su_drive(&s->o, s->sim, s->instance, s->pins->clock[K_CO], LOGIC_X, 0);
        if (s->dev->is_hd) {
            su_drive(&s->o, s->sim, s->instance, x[X_TC], LOGIC_X, 0);
            su_drive(&s->o, s->sim, s->instance, x[X_OE], LOGIC_X, 0);
            su_drive(&s->o, s->sim, s->instance, x[X_FE], LOGIC_X, 0);
            su_drive(&s->o, s->sim, s->instance, x[X_PE], LOGIC_X, 0);
        }
        return;
    }
    su_drive(&s->o, s->sim, s->instance, p[P_TX], s->in_reset ? LOGIC_1 : sdo_level(s), t_of(s, T_SDO));
    su_drive(&s->o, s->sim, s->instance, p[P_RTS], su_not(su_bit(s->mcr, 0)), t_of(s, T_FLAG));
    su_drive(&s->o, s->sim, s->instance, p[P_DTR], su_not(su_bit(s->mcr, 1)), t_of(s, T_FLAG));
    su_drive(&s->o, s->sim, s->instance, p[P_INT], s->in_reset ? LOGIC_0 : intr_level(s),
             delay != 0 ? delay : t_of(s, T_INTR));
    su_drive(&s->o, s->sim, s->instance, p[P_TBRE], su_bool(s->tbre), t_of(s, T_FLAG));
    su_drive(&s->o, s->sim, s->instance, p[P_DR], su_bool(s->dr), t_of(s, T_FLAG));
    su_drive_clock(&s->o, s->sim, s->instance, s->pins->clock[K_CO], co_level(s),
                   su_bit(s->brsr, 7) == LOGIC_1 ? t_of(s, T_BRG) : t_of(s, T_CO));
    if (s->dev->is_hd) {
        su_drive(&s->o, s->sim, s->instance, x[X_TC], s->tc_pin, t_of(s, T_FLAG));
        su_drive(&s->o, s->sim, s->instance, x[X_OE], s->oe_pin, t_of(s, T_FLAG));
        su_drive(&s->o, s->sim, s->instance, x[X_FE], s->fe_pin, t_of(s, T_FLAG));
        su_drive(&s->o, s->sim, s->instance, x[X_PE], s->pe_pin, t_of(s, T_FLAG));
    }
}

/* Edge detectors: a USR group bit, DR or TBRE going to 1 (each bit's own
   rising edge; DR and TBRE are levels, so an edge of their OR would miss DR
   while TBRE is high). */
static void track_edges(HsState *s) {
    uint8_t group = usr_group(s);

    if (group & ~s->last_usr) {
        s->usr_edge = 1;
    }
    if ((s->dr && !s->last_dr) || (s->tbre && !s->last_tbre)) {
        s->sie_edge = 1;
    }
    s->last_usr = group;
    s->last_dr = s->dr;
    s->last_tbre = s->tbre;
}

static void refresh(HsState *s, SimTime delay) {
    track_edges(s);
    update_outputs(s, delay);
    serial_bus_refresh(&s->bus, s->sim, s->instance, &s->events);
}

/* Sets latched USR bits (a USR read in progress defers them, p9 NOTE). */
static void usr_set(HsState *s, uint8_t bits) {
    if (s->usr_reading) {
        s->usr_pending |= bits;
        return;
    }
    s->usr |= bits;
}

/* ---- Reset -------------------------------------------------------------------------------- */

static void reset_state(HsState *s) {
    s->brsr.v &= 0x7Fu;
    s->brsr.k |= 0x80u;
    s->mcr = su_reg(0, 0xFF);
    s->tbre = 1;
    s->tbr_age = 0;
    s->staged = 0;
    s->dr = 0;
    s->usr = U_TC;
    s->usr_k = 0xFF;
    s->usr_reading = 0;
    s->usr_pending = 0;
    /* TC is set by the reset: the edge detector sees it (p9). */
    s->usr_edge = 1;
    s->last_usr = U_TC;
    s->sie_edge = 0;
    s->last_dr = 0;
    s->last_tbre = 1;
    s->modem_edge = 0;
    s->tc_pin = LOGIC_1;
    s->oe_pin = s->fe_pin = s->pe_pin = LOGIC_0;
    s->tsr = LOGIC_1;
    s->pending_count = 0;
    brg_reset(s);
    serial_tx_reset(&s->line.tx);
    serial_rx_reset(&s->line.rx);
    update_format(s);
}

static void apply_reset(HsState *s) {
    LogicValue r = serial_level(s->sim, s->instance, s->pins->reset, LOGIC_0);

    if (r == LOGIC_X) {
        s->unknown = 1;
        s->in_reset = 0;
        s->bus.enabled = 0;
    } else if (r == LOGIC_1) {
        s->unknown = 0;
        s->in_reset = 1;
        s->bus.enabled = 0;
        reset_state(s);
        serial_line_sender_reset(&s->line, s->sim, s->instance);
    } else {
        if (s->unknown) {
            reset_state(s);
            s->mcr = su_reg(0, 0);
            s->brsr = su_reg(0, 0);
            update_format(s);
        }
        s->unknown = 0;
        s->in_reset = 0;
        s->bus.enabled = 1;
    }
    serial_bus_update(&s->bus, s->sim, s->instance, &s->events);
    refresh(s, 0);
}

/* ---- Registers ------------------------------------------------------------------------------ */

static void msr_of(const HsState *s, uint8_t *v, uint8_t *k) {
    unsigned n = s->dev->is_hd ? 4u : 2u;
    unsigned i;

    *v = 0;
    *k = s->dev->is_hd ? 0x0Fu : 0x0Fu;
    for (i = 0; i < n; i++) {
        LogicValue a = modem_active(s, i);

        if (a == LOGIC_1) *v |= (uint8_t)(1u << i);
        if (a == LOGIC_X) *k &= (uint8_t)~(1u << i);
    }
    /* D7..D4 undefined: X (P2). */
}

static void bus_peek(void *context, unsigned unit, unsigned address, uint8_t *data, uint8_t *known) {
    HsState *s = context;

    (void)address;
    *data = 0;
    *known = 0;
    if (s->unknown) {
        return;
    }
    switch (unit) {
        case 0:
            *data = s->rbr.v;
            *known = s->rbr.k;
            break;
        case 1:
            if (!s->usr_reading) {
                s->usr_reading = 1;
                s->usr_pending = 0;
                s->usr_at_read = s->usr;
            }
            *data = (uint8_t)((s->usr & U_LATCHED) | (s->tbre ? 0x40 : 0) | (s->dr ? 0x80 : 0));
            *known = (uint8_t)(s->usr_k | 0xC0);
            break;
        case 2:
            *data = s->mcr.v;
            *known = s->mcr.k;
            break;
        case 3:
            msr_of(s, data, known);
            break;
        case 4:
            /* RXDACK read: the RBR while DR; X otherwise (decisions.md). */
            if (s->dr) {
                *data = s->rbr.v;
                *known = s->rbr.k;
            }
            break;
        default:
            break;   /* TXDACK with RD: X */
    }
}

static void bus_read_done(void *context, unsigned unit, unsigned address) {
    HsState *s = context;
    SimTime delay = t_of(s, T_INTR);

    (void)address;
    if (s->unknown) {
        return;
    }
    if (unit == 0 || unit == 4) {
        if (unit == 0 || s->dr) {
            s->dr = 0;
        }
    } else if (unit == 1) {
        uint8_t again = (uint8_t)(s->usr_pending & ~s->usr_at_read);

        /* The read clears the latched bits and the edge detectors; a
           condition during the read: set now, unless it was already set
           (then it stays cleared). */
        s->usr = again;
        s->usr_reading = 0;
        s->usr_pending = 0;
        s->usr_edge = 0;
        s->sie_edge = 0;
        s->last_usr = 0;   /* bits set again at the read's end are new edges */
    } else if (unit == 3) {
        s->modem_edge = 0;
    }
    /* No bus refresh here: the cycle is still marked active, and a peek
       would start a new USR read. */
    track_edges(s);
    update_outputs(s, delay);
}

static void write_tbr(HsState *s, uint8_t data, uint8_t known) {
    s->tbr = su_reg(data, known);
    s->tbre = 0;
    s->tbr_age = 0;
    s->tc_pin = LOGIC_0;   /* HD-6406 TC pin: reset by a TBR write */
}

static void bus_write(void *context, unsigned unit, unsigned address, uint8_t data, uint8_t known) {
    HsState *s = context;

    (void)address;
    if (s->unknown) {
        return;
    }
    switch (unit) {
        case 0:
            write_tbr(s, data, known);
            break;
        case 1:
            s->ucr = su_reg(data, known);
            update_format(s);
            break;
        case 2:
            s->mcr = su_reg(data, known);
            break;
        case 3:
            s->brsr = su_reg(data, known);
            brg_reset(s);
            break;
        case 5:
            if (s->tbre) {
                write_tbr(s, data, known);   /* TXDACK with TBRE = 0: ignored */
            }
            break;
        default:
            break;
    }
    refresh(s, t_of(s, T_FLAG));
}

/* The select decode: CS0 low (and CS1 high), or a DMA acknowledge. */
static LogicValue bus_select(void *context, const Sim *sim, const SimInstance *instance,
                             unsigned address, unsigned address_known, unsigned *unit) {
    HsState *s = context;
    LogicValue cs0 = serial_level(sim, instance, s->pins->select, LOGIC_1);
    LogicValue cs1 = serial_level(sim, instance, s->pins->select_high, LOGIC_1);
    LogicValue sel = su_and(su_not(cs0), cs1);
    LogicValue rxdack = su_not(serial_level(sim, instance, s->pins->hd[X_RXDACK], LOGIC_1));
    LogicValue txdack = su_not(serial_level(sim, instance, s->pins->hd[X_TXDACK], LOGIC_1));
    LogicValue dack = su_or(rxdack, txdack);

    (void)address;
    (void)address_known;
    *unit = 0;
    if (dack != LOGIC_0) {
        if (dack == LOGIC_X || sel != LOGIC_0 || (rxdack == LOGIC_1 && txdack == LOGIC_1)) {
            return LOGIC_X;
        }
        *unit = rxdack == LOGIC_1 ? 4u : 5u;
        return LOGIC_1;
    }
    if (sel == LOGIC_1) {
        if (s->a_latch[0] == LOGIC_X || s->a_latch[1] == LOGIC_X) {
            return LOGIC_X;
        }
        *unit = (s->a_latch[0] == LOGIC_1 ? 1u : 0u) | (s->a_latch[1] == LOGIC_1 ? 2u : 0u);
    }
    return sel;
}

static void track_address(HsState *s) {
    LogicValue ale = s->pins->ale == SERIAL_NO_PIN
        ? LOGIC_1 : serial_level(s->sim, s->instance, s->pins->ale, LOGIC_X);
    unsigned i;

    for (i = 0; i < 2; i++) {
        LogicValue a = serial_level(s->sim, s->instance, s->pins->addr[i], LOGIC_X);

        if (ale == LOGIC_1) {
            s->a_latch[i] = a;
        } else if (ale == LOGIC_X && a != s->a_latch[i]) {
            s->a_latch[i] = LOGIC_X;
        }
    }
}

/* ---- Transmitter ----------------------------------------------------------------------------- */

static int tx_fetch(void *context, uint16_t *data, uint16_t *known) {
    HsState *s = context;
    unsigned m = mode_of(s);

    if (m == 2) {
        return 0;   /* echo: the normal transmitter is off */
    }
    if (s->staged) {
        s->staged = 0;
        *data = s->stage_v;
        *known = s->stage_k;
        return 1;
    }
    /* Back to back (the TR was busy): the next character goes at once. */
    if (!s->tbre && s->tbr_age < 0 &&
        (m == 3 || serial_level(s->sim, s->instance, s->pins->pin[P_CTS], LOGIC_X) == LOGIC_0)) {
        *data = s->tbr.v;
        *known = s->tbr.k;
        s->tbre = 1;
        return 1;
    }
    return 0;
}

static void tx_done(void *context, uint16_t data, uint16_t known) {
    HsState *s = context;

    (void)known;
    serial_line_log(&s->line, data);
}

static void tx_tick(HsState *s) {
    SerialTxEvents events = {s, tx_fetch, NULL, tx_done};
    int busy = serial_tx_busy(&s->line.tx);
    int busy_now;
    LogicValue cts = serial_level(s->sim, s->instance, s->pins->pin[P_CTS], LOGIC_X);

    s->tsr = serial_tx_tick(&s->line.tx, &s->txf, &events);
    busy_now = serial_tx_busy(&s->line.tx);
    if (busy && !busy_now && s->tbre && !s->staged) {
        usr_set(s, U_TC);
        s->tc_pin = LOGIC_1;
    }
    /* TR idle: TBRE high at the 4th tick after the write (or after CTS goes
       low), the start bit at the 5th (p14 notes 2, 4). */
    if (!s->tbre && !busy_now && !s->staged && (cts == LOGIC_0 || mode_of(s) == 3) &&
        mode_of(s) != 2) {
        if (++s->tbr_age >= 4) {
            s->staged = 1;
            s->stage_v = s->tbr.v;
            s->stage_k = s->tbr.k;
            s->tbre = 1;
        }
    } else if (!s->tbre && busy_now) {
        s->tbr_age = -1;   /* waiting behind the TR: it follows back to back */
    }
}

/* ---- Receiver ------------------------------------------------------------------------------- */

static void on_received(void *context, const SerialRxChar *c) {
    HsState *s = context;

    if ((c->flags_known & SERIAL_RX_FE) && (c->flags & SERIAL_RX_FE) && !(c->flags & SERIAL_RX_BI)) {
        s->resync = 1;
    }
    if (s->pending_count < 4) {
        s->pending[s->pending_count].c = *c;
        s->pending[s->pending_count].ticks = 3;   /* the 11th tick of the stop bit */
        s->pending_count++;
    }
}

static void deliver(HsState *s, const SerialRxChar *c) {
    unsigned n = s->rxf.data_bits;
    uint8_t mask = (uint8_t)((1u << n) - 1);
    uint8_t bits = 0;
    int pe = rx_checks_parity(s) && (c->flags & SERIAL_RX_PE);
    int fe = (c->flags & SERIAL_RX_FE) != 0;
    int brk = (c->flags & SERIAL_RX_BI) != 0;

    if (s->dr) {
        /* Overrun: the character in the receiver register is lost (p8). */
        usr_set(s, U_OE);
        s->oe_pin = LOGIC_1;
        return;
    }
    s->rbr = su_reg((uint8_t)(c->data & mask), s->rxf.format_x ? 0 : (uint8_t)((c->known & mask) | ~mask));
    s->dr = 1;
    if (pe) bits |= U_PE;
    if (fe) bits |= U_FE;
    if (brk) bits |= U_RBRK;
    usr_set(s, bits);
    s->oe_pin = LOGIC_0;
    s->fe_pin = su_bool(fe);
    s->pe_pin = su_bool(pe);
}

static void rx_tick(HsState *s) {
    SerialRxEvents events = {s, on_received, NULL};
    LogicValue pin = serial_level(s->sim, s->instance, s->pins->pin[P_RX], LOGIC_X);
    LogicValue level;
    LogicValue ren = su_bit(s->mcr, 5);
    unsigned i;

    for (i = 0; i < s->pending_count;) {
        if (--s->pending[i].ticks == 0) {
            deliver(s, &s->pending[i].c);
            memmove(&s->pending[i], &s->pending[i + 1], (s->pending_count - i - 1) * sizeof s->pending[0]);
            s->pending_count--;
        } else {
            i++;
        }
    }
    level = serial_line_rx_input(&s->line, s->sim, s->instance, pin, &s->rxf);
    if (mode_of(s) == 3) {
        level = s->tsr;   /* loop: the transmitter feeds the receiver */
    }
    if (ren == LOGIC_0) {
        level = LOGIC_1;   /* REN gates off the input */
    } else if (ren == LOGIC_X) {
        level = level == LOGIC_1 ? LOGIC_1 : LOGIC_X;
    }
    s->resync = 0;
    serial_rx_tick(&s->line.rx, level, &s->rxf, &events);
    if (s->resync) {
        su_rx_no_resync(&s->line.rx, level);
    }
}

static void baud_tick(HsState *s) {
    if (s->in_reset || s->unknown) {
        return;
    }
    tx_tick(s);
    rx_tick(s);
    refresh(s, 0);
}

static void ix_edge(HsState *s, LogicValue level) {
    int rising = s->ix == LOGIC_0 && level == LOGIC_1;
    unsigned d3 = div3(s);

    s->ix = level;
    if (rising && !s->in_reset && d3 != 0) {
        unsigned p = PRESCALE[s->brsr.v & 3u];

        if (++s->pre_count >= p) {
            s->pre_count = 0;
            s->acc += 3;
            if (s->acc >= d3) {
                s->acc -= d3;
                s->co_brg = LOGIC_0;   /* the falling edge of CO(BRG) */
                baud_tick(s);
            } else if (s->acc * 2 >= d3) {
                s->co_brg = LOGIC_1;
            }
        } else if (d3 == 3 && s->pre_count >= (p + 1) / 2) {
            s->co_brg = LOGIC_1;   /* external: the prescaler's own duty cycle */
        }
    }
    update_outputs(s, 0);
}

/* ---- Modem inputs ---------------------------------------------------------------------------- */

static size_t modem_pin(const HsState *s, unsigned i) {
    switch (i) {
        case 0: return s->pins->pin[P_CTS];
        case 1: return s->pins->pin[P_DSR];
        case 2: return s->pins->hd[X_RLSD];
        default: return s->pins->hd[X_RI];
    }
}

static void modem_changed(HsState *s) {
    unsigned i;

    for (i = 0; i < 4; i++) {
        LogicValue v = serial_level(s->sim, s->instance, modem_pin(s, i), LOGIC_X);

        if (modem_pin(s, i) == SERIAL_NO_PIN) {
            continue;
        }
        if (v != s->modem_level[i]) {
            if (!s->in_reset && !s->unknown) {
                s->modem_edge = 1;
                usr_set(s, U_MS);   /* regardless of the masks (p14 note 5C) */
            }
            s->modem_level[i] = v;
        }
    }
    refresh(s, 0);
}

/* ---- Engine callbacks -------------------------------------------------------------------------- */

static void hs_start(Sim *sim, SimInstance *instance, void *raw) {
    HsState *s = raw;
    unsigned i;

    s->sim = sim;
    s->instance = instance;
    s->ix = serial_level(sim, instance, s->pins->clock[K_IN], LOGIC_X);
    for (i = 0; i < 4; i++) {
        s->modem_level[i] = serial_level(sim, instance, modem_pin(s, i), LOGIC_X);
    }
    track_address(s);
    serial_bus_start(&s->bus, sim, instance, &s->events);
    apply_reset(s);
}

static int is_bus_pin(const HsState *s, size_t pin) {
    const size_t *x = s->pins->hd;

    return serial_bus_has_pin(&s->bus, pin) || pin == s->pins->addr[0] || pin == s->pins->addr[1] ||
           pin == s->pins->ale || pin == s->pins->select || pin == s->pins->select_high ||
           (s->dev->is_hd && (pin == x[X_RXDACK] || pin == x[X_TXDACK]));
}

static void hs_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    HsState *s = raw;
    unsigned i;

    (void)old_value;
    (void)new_value;
    s->sim = sim;
    s->instance = instance;
    if (pin == s->pins->reset) {
        apply_reset(s);
        return;
    }
    if (pin == s->pins->clock[K_IN]) {
        ix_edge(s, serial_level(sim, instance, pin, LOGIC_X));
        return;
    }
    if (is_bus_pin(s, pin)) {
        track_address(s);
        serial_bus_update(&s->bus, sim, instance, &s->events);
        refresh(s, 0);
    }
    for (i = 0; i < 4; i++) {
        if (pin == modem_pin(s, i)) {
            modem_changed(s);
        }
    }
    if (pin == s->pins->pin[P_RX] && mode_of(s) == 2) {
        update_outputs(s, 0);   /* echo follows SDI */
    }
    if (s->dev->is_hd && (pin == s->pins->hd[X_SIE] || pin == s->pins->hd[X_SFD])) {
        refresh(s, 0);
    }
}

/* ---- Terminal ------------------------------------------------------------------------------------ */

static unsigned hs_channels(const void *raw) {
    (void)raw;
    return 1;
}

static SerialLine *hs_line(void *raw, unsigned channel) {
    (void)channel;
    return &((HsState *)raw)->line;
}

static int hs_async(const void *raw, unsigned channel) {
    (void)raw;
    (void)channel;
    return 1;
}

/* ---- Create / destroy / loader -------------------------------------------------------------------- */

static void *hs_create(const Part *part, const void *data, const void *pins,
                       const SerialGrade *grade, const void *options) {
    HsState *s = calloc(1, sizeof *s);
    size_t i;

    (void)options;
    if (s == NULL) {
        return NULL;
    }
    s->base.model = &HS82C52_MODEL;
    s->dev = data;
    s->pins = pins;
    s->grade = grade;
    s->o.count = part->pin_count;
    s->o.out = malloc(part->pin_count * sizeof *s->o.out);
    if (s->o.out == NULL) {
        free(s);
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
    s->events.select = bus_select;
    s->events.peek = bus_peek;
    s->events.read_done = bus_read_done;
    s->events.write = bus_write;
    serial_line_init(&s->line, s->pins->pin[P_RX]);
    /* Power-on: UCR and BRSR (except D7) are not reset: X. */
    s->ucr = su_reg(0, 0);
    s->brsr = su_reg(0, 0);
    s->rbr = su_reg(0, 0);
    s->tbr = su_reg(0, 0);
    reset_state(s);
    s->ix = LOGIC_X;
    s->a_latch[0] = s->a_latch[1] = LOGIC_X;
    for (i = 0; i < 4; i++) {
        s->modem_level[i] = LOGIC_X;
    }
    return s;
}

static void hs_destroy(void *raw) {
    HsState *s = raw;

    serial_line_free(&s->line);
    free(s->o.out);
    free(s);
}

static void *hs_load_device(KindContext *ctx, const cJSON *json, unsigned *channels) {
    HsDevice *d = kind_alloc(ctx, sizeof *d);
    const cJSON *bus;

    if (d == NULL) {
        return NULL;
    }
    bus = kind_member(ctx, json, "bus", 1);
    if (bus == NULL) {
        return NULL;
    }
    kind_push(ctx, "bus");
    if (kind_check_keys(ctx, bus, BUS_KEYS, NULL) != 0 ||
        su_load_names(ctx, bus, "data", 1, d->data, 8, 8, NULL) != 0 ||
        su_load_names(ctx, bus, "address", 1, d->addr, 2, 2, NULL) != 0 ||
        kind_string(ctx, bus, "read", &d->read) != 0 ||
        kind_string(ctx, bus, "write", &d->write) != 0 ||
        kind_string(ctx, bus, "select", &d->select) != 0 ||
        kind_string(ctx, bus, "reset", &d->reset) != 0 ||
        serial_optional_name(ctx, bus, "selectHigh", &d->select_high) != 0 ||
        serial_optional_name(ctx, bus, "addressLatch", &d->ale) != 0) {
        return NULL;
    }
    kind_pop(ctx);
    if (su_load_pin_object(ctx, json, "clock", 1, CLOCK_KEYS, d->clock, 0x7u) != 0 ||
        su_load_pin_object(ctx, json, "serial", 1, SERIAL_KEYS, d->pin, 0x1FFu) != 0) {
        return NULL;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "hd6406") != NULL) {
        if (su_load_pin_object(ctx, json, "hd6406", 1, HD_KEYS, d->hd, 0x3FFu) != 0) {
            return NULL;
        }
        d->is_hd = 1;
    }
    *channels = 1;
    return d;
}

static void *hs_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t pin_count) {
    const HsDevice *d = raw;
    HsPins *p = kind_alloc(ctx, sizeof *p);
    unsigned i;

    if (p == NULL ||
        su_bind_names(ctx, pins, pin_count, "bus.data", d->data, p->bus.data, 8) != 0 ||
        su_bind_names(ctx, pins, pin_count, "bus.address", d->addr, p->addr, 2) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.read", d->read, &p->bus.read) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.write", d->write, &p->bus.write) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.select", d->select, &p->select) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.selectHigh", d->select_high, &p->select_high) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.addressLatch", d->ale, &p->ale) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.reset", d->reset, &p->reset) != 0 ||
        su_bind_object(ctx, pins, pin_count, "clock", CLOCK_KEYS, d->clock, p->clock) != 0 ||
        su_bind_object(ctx, pins, pin_count, "serial", SERIAL_KEYS, d->pin, p->pin) != 0) {
        return NULL;
    }
    for (i = 0; i < X_COUNT; i++) {
        p->hd[i] = SERIAL_NO_PIN;
    }
    if (d->is_hd && su_bind_object(ctx, pins, pin_count, "hd6406", HD_KEYS, d->hd, p->hd) != 0) {
        return NULL;
    }
    /* serial_bus.c: no address lines (the unit is the register), CS0 as the
       select for its pin list; bus_select decodes. */
    p->bus.address_count = 0;
    p->bus.select[0] = p->select;
    p->bus.select_count = 1;
    p->bus.select_high_count = 0;
    p->bus.read_high = SERIAL_NO_PIN;
    p->bus.write_high = SERIAL_NO_PIN;
    p->bus.address_strobe = SERIAL_NO_PIN;
    p->bus.driver_disable = SERIAL_NO_PIN;
    p->bus.select_out = SERIAL_NO_PIN;
    return p;
}

const SerialModel HS82C52_MODEL = {
    "hs82c52", KEYS, TIMING, NULL,
    hs_load_device, hs_bind, NULL, NULL,
    hs_create, hs_destroy, hs_start, hs_pin_changed, NULL,
    hs_channels, hs_line, hs_async
};
