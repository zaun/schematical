/*
 * acia6551: the CDP65C51 / CDP65C51A ACIA (plan-12-H section 3.4; facts in
 * plan-12-data/specs/classic-b.md section CDP65C51, decisions.md section
 * CDP65C51 and the planner decision on the TDRE interrupt).
 *
 * Bus (6500 style): selected = CS0 high and CS1 low; RS1..RS0 and R/W pick
 * the register (Table I). A selected cycle acts while PHI2 is high: a read
 * drives the data tcdr after PHI2 rises and floats thr after it falls; a
 * write is taken on PHI2's falling edge; read side effects (RDRF clear,
 * status-read clears) happen as PHI2 falls.
 *
 * Clocks: XTLI is counted; the control register's SBR picks a divisor of it
 * (Table II, 1.8432 MHz gives the printed rates) or, with SBR = 0000, XTLI
 * is itself the 16x clock. The transmitter always uses it; the receiver
 * uses it with RCS = 1 (RXC then outputs the 16x clock) or counts RXC (an
 * external 16x clock) with RCS = 0.
 *
 * Interrupts (IRQ open drain, active low): RDRF (IRD = 0, DCD low),
 * DCD/DSR changes, and TDRE as a level while TIC = 01 (planner decision:
 * it recurs while idle, Fig 9); each source is enabled by command bit 0 when
 * it arises; a status read clears bit 7 and the latched sources.
 * CTS: the CDP65C51 marks TXD at once while CTS is high (the character is
 * lost, TDRE reads 0, its interrupts go on); the CDP65C51A (part key
 * "ctsA") finishes the character and holds the next with no TDRE
 * interrupt until CTS is low.
 *
 * "w65c51n": true (plan-14-E; datasheets/controller-uart/w65c51n.pdf): the
 * WDC W65C51N. TDRE reads 1 always (p.10: the TSR is loaded when the TDR is
 * written), so a TIC = 01 interrupt never clears; a TDR write while a frame
 * is going out cuts it off at the next bit boundary, where the new byte's
 * start bit begins (the cut-off character is not logged); there is no
 * parity (p.14: command bits 5-7 are ignored, PE reads 0).
 */

#include <stdlib.h>
#include <string.h>

#include "serial_util.h"

enum { T_CDR, T_HR, T_DD, T_DLY, T_IRQ };
static const char *const TIMING[] = {"tcdr", "thr", "tdd", "tdly", "tirq", NULL};

static const char *const KEYS[] = {"bus", "clock", "serial", "w65c51n", NULL};
static const char *const PART_KEYS[] = {"ctsA", NULL};

enum { B_SELECT, B_SELECT_HIGH, B_PHI2, B_RW, B_RESET, B_COUNT };
static const char *const BUS_NAMES[] = {"select", "selectHigh", "phi2", "readWrite", "reset"};
static const char *const BUS_KEYS[] = {
    "data", "registerSelect", "select", "selectHigh", "phi2", "readWrite", "reset", NULL
};

enum { K_IN, K_OUT, K_RECEIVE, K_COUNT };
static const char *const CLOCK_KEYS[] = {"in", "out", "receive", NULL};

enum { P_TX, P_RX, P_RTS, P_CTS, P_DTR, P_DSR, P_DCD, P_IRQ, P_COUNT };
static const char *const SERIAL_KEYS[] = {"tx", "rx", "rts", "cts", "dtr", "dsr", "dcd", "irq",
                                          NULL};

/* Table II: the baud divisors of XTLI (SBR 0001..1111); 0000 = XTLI / 16. */
static const uint32_t DIVISOR[16] = {
    16, 36864, 24576, 16768, 13696, 12288, 6144, 3072, 1536, 1024, 768, 512, 384, 256, 192, 96
};

typedef struct {
    const char *data[8], *rs[2];
    const char *bus[B_COUNT];
    const char *clock[K_COUNT];
    const char *pin[P_COUNT];
    int w65c51n;
} AcDevice;

typedef struct {
    size_t data[8], rs[2];
    size_t bus[B_COUNT];
    size_t clock[K_COUNT];
    size_t pin[P_COUNT];
} AcPins;

typedef struct {
    int cts_a;
} AcOptions;

typedef struct {
    SerialBase base;
    const AcDevice *dev;
    const AcPins *pins;
    const SerialGrade *grade;
    int cts_a;
    Sim *sim;
    SimInstance *instance;
    SuOutputs o;
    SerialLine line;
    SerialBaud baud;
    SerialFormat format;

    int in_reset;
    int unknown;
    SuReg command, control;
    SuReg rdr, tdr;
    LogicValue tdre, rdrf, pe, fe, ovr;
    int committed;                /* TDR written while DTR (command b0) was 1 */
    LogicValue tsr;
    int break_end;                /* ticks of stop bit after a break */
    int break_on;
    /* Interrupts. */
    int int_rx, int_modem;
    /* DCD / DSR status bits and their freeze. */
    LogicValue dcd_bit, dsr_bit;
    int frozen;
    /* Echo. */
    LogicValue echo[8];
    unsigned echo_at;
    int echo_mark;                /* overrun in echo: mark until a start after an RDR read */
    int echo_wait_read;
    /* Bus. */
    LogicValue phi2, xtli, rxc;
    int cycle;                    /* 0 none, 1 read, 2 write, 3 X */
    unsigned cycle_rs, cycle_rs_known;
    int resync;
    int cut;                      /* W65C51N: a TDR write cuts the frame in progress */
} AcState;

static SimTime t_of(const AcState *s, unsigned i) {
    return s->grade->t[i];
}

/* ---- Format and clocks ------------------------------------------------------------------- */

static unsigned word_length(const AcState *s) {
    return 8u - ((s->control.v >> 5) & 3u);
}

static void update_format(AcState *s) {
    SerialFormat f = serial_format_default();
    unsigned wl = word_length(s);
    int parity = s->dev->w65c51n ? 0 : (s->command.v >> 5) & 1u;
    unsigned pmc = (s->command.v >> 6) & 3u;

    f.data_bits = wl;
    f.parity = !parity ? SERIAL_PARITY_NONE
             : pmc == 0 ? SERIAL_PARITY_ODD : pmc == 1 ? SERIAL_PARITY_EVEN
             : pmc == 2 ? SERIAL_PARITY_MARK : SERIAL_PARITY_SPACE;
    if (s->control.v & 0x80u) {
        /* 2 stops, except 1.5 for 5 bits without parity and 1 for 8 bits with it. */
        f.stop_halves = wl == 5 && !parity ? 3 : wl == 8 && parity ? 2 : 4;
    } else {
        f.stop_halves = 2;
    }
    f.format_x = (s->control.k & 0xE0u) != 0xE0u ||
                 (!s->dev->w65c51n && (s->command.k & 0xE0u) != 0xE0u);
    s->format = f;
}

static void load_baud(AcState *s) {
    unsigned sbr = s->control.v & 15u;
    uint32_t d = sbr == 0 ? 1 : DIVISOR[sbr] / 16;

    serial_baud_load(&s->baud, d, (s->control.k & 15u) == 15u);
}

static int rcs_internal(const AcState *s) {
    return (s->control.v >> 4) & 1u;
}

static LogicValue dtr_bit(const AcState *s) {
    return su_bit(s->command, 0);
}

static unsigned tic(const AcState *s) {
    return (s->command.v >> 2) & 3u;
}

static int echo_mode(const AcState *s) {
    return (s->command.v & 0x10u) && tic(s) == 0;
}

static LogicValue cts_high(const AcState *s) {
    return serial_level(s->sim, s->instance, s->pins->pin[P_CTS], LOGIC_X);
}

/* ---- Interrupts and outputs ----------------------------------------------------------------- */

static LogicValue tdre_int(const AcState *s) {
    LogicValue on = su_and(dtr_bit(s), su_bool(tic(s) == 1 && (s->command.k & 0x0Cu) == 0x0Cu));

    if (s->dev->w65c51n) {
        return on;   /* TDRE is always 1 */
    }
    if (s->cts_a) {
        on = su_and(on, su_not(cts_high(s)));
    }
    return su_and(on, s->tdre);
}

static LogicValue irq_level(const AcState *s) {
    return su_or(su_bool(s->int_rx || s->int_modem), tdre_int(s));
}

static void status_of(const AcState *s, uint8_t *v, uint8_t *k) {
    LogicValue bits[8];
    LogicValue tdre = s->tdre;
    unsigned i;

    /* CDP65C51: while CTS is high the status does not show TDR empty. */
    if (!s->cts_a) {
        tdre = su_and(tdre, su_not(cts_high(s)));
    }
    if (s->dev->w65c51n) {
        tdre = LOGIC_1;   /* p.10: always 1 */
    }
    bits[0] = s->dev->w65c51n ? LOGIC_0 : s->pe;
    bits[1] = s->fe;
    bits[2] = s->ovr;
    bits[3] = s->rdrf;
    bits[4] = tdre;
    bits[5] = s->dcd_bit;
    bits[6] = s->dsr_bit;
    bits[7] = irq_level(s);
    *v = 0;
    *k = 0;
    for (i = 0; i < 8; i++) {
        if (bits[i] == LOGIC_1) *v |= (uint8_t)(1u << i);
        if (bits[i] != LOGIC_X) *k |= (uint8_t)(1u << i);
    }
}

static void reg_peek(const AcState *s, unsigned rs, uint8_t *v, uint8_t *k) {
    switch (rs & 3u) {
        case 0: *v = s->rdr.v; *k = s->rdr.k; break;
        case 1: status_of(s, v, k); break;
        case 2: *v = s->command.v; *k = s->command.k; break;
        default: *v = s->control.v; *k = s->control.k; break;
    }
}

static LogicValue txd_level(const AcState *s) {
    LogicValue t = s->tsr;

    if (s->break_on) {
        t = LOGIC_0;
    }
    if (echo_mode(s)) {
        t = s->echo_mark ? LOGIC_1 : s->echo[s->echo_at];
    }
    /* CDP65C51: TXD marks at once while CTS is high. */
    if (!s->cts_a) {
        LogicValue c = cts_high(s);

        if (c == LOGIC_1) {
            t = LOGIC_1;
        } else if (c == LOGIC_X && t != LOGIC_1) {
            t = LOGIC_X;
        }
    }
    return t;
}

static void drive_data(AcState *s) {
    uint8_t v = 0;
    uint8_t k = 0;
    int release = s->cycle != 1 && s->cycle != 3;

    if (s->cycle == 1 && s->cycle_rs_known == 3u) {
        reg_peek(s, s->cycle_rs, &v, &k);
    }
    su_drive_bus(&s->o, s->sim, s->instance, s->pins->data, 8, v, k, release,
                 release ? t_of(s, T_HR) : t_of(s, T_CDR));
}

static void update_outputs(AcState *s, SimTime delay) {
    const size_t *p = s->pins->pin;
    LogicValue rts;

    if (s->unknown) {
        su_drive(&s->o, s->sim, s->instance, p[P_TX], LOGIC_X, 0);
        su_drive(&s->o, s->sim, s->instance, p[P_RTS], LOGIC_X, 0);
        su_drive(&s->o, s->sim, s->instance, p[P_DTR], LOGIC_X, 0);
        su_drive(&s->o, s->sim, s->instance, p[P_IRQ], LOGIC_X, 0);
        su_drive(&s->o, s->sim, s->instance, s->pins->clock[K_RECEIVE], LOGIC_X, 0);
        return;
    }
    /* RTS: high with TIC = 00 unless echo; DTR: low with command b0 = 1. */
    rts = (s->command.k & 0x1Cu) != 0x1Cu ? LOGIC_X : su_bool(tic(s) == 0 && !(s->command.v & 0x10u));
    su_drive(&s->o, s->sim, s->instance, p[P_RTS], s->in_reset ? LOGIC_1 : rts, t_of(s, T_DLY));
    su_drive(&s->o, s->sim, s->instance, p[P_DTR], s->in_reset ? LOGIC_1 : su_not(dtr_bit(s)),
             t_of(s, T_DLY));
    su_drive(&s->o, s->sim, s->instance, p[P_TX], s->in_reset ? LOGIC_1 : txd_level(s), t_of(s, T_DD));
    su_drive_od(&s->o, s->sim, s->instance, p[P_IRQ], s->in_reset ? LOGIC_1 : su_not(irq_level(s)),
                delay);
    /* RXC: the 16x clock out with RCS = 1, else an input. */
    if (rcs_internal(s) && (s->control.k & 0x10u)) {
        su_drive_clock(&s->o, s->sim, s->instance, s->pins->clock[K_RECEIVE], s->baud.baudout, 0);
    } else if (!(s->control.k & 0x10u)) {
        su_drive(&s->o, s->sim, s->instance, s->pins->clock[K_RECEIVE], LOGIC_X, 0);
    } else {
        su_drive(&s->o, s->sim, s->instance, s->pins->clock[K_RECEIVE], LOGIC_Z, 0);
    }
    drive_data(s);
}

/* ---- Modem inputs ---------------------------------------------------------------------------- */

static void modem_sample(AcState *s, int may_interrupt) {
    LogicValue dcd = serial_level(s->sim, s->instance, s->pins->pin[P_DCD], LOGIC_X);
    LogicValue dsr = serial_level(s->sim, s->instance, s->pins->pin[P_DSR], LOGIC_X);

    if (s->frozen) {
        return;   /* held until the status register is read (p5) */
    }
    if (dcd != s->dcd_bit || dsr != s->dsr_bit) {
        s->dcd_bit = dcd;
        s->dsr_bit = dsr;
        if (may_interrupt && dtr_bit(s) == LOGIC_1) {
            s->int_modem = 1;
            s->frozen = 1;
        }
    }
}

/* ---- Reset -------------------------------------------------------------------------------- */

static void reset_state(AcState *s) {
    s->command = su_reg(0, 0xFF);
    s->control = su_reg(0, 0xFF);
    s->tdre = LOGIC_1;
    s->rdrf = s->pe = s->fe = s->ovr = LOGIC_0;
    s->committed = 0;
    s->tsr = LOGIC_1;
    s->break_end = 0;
    s->break_on = 0;
    s->cut = 0;
    s->int_rx = s->int_modem = 0;
    s->frozen = 0;
    s->echo_mark = 0;
    s->echo_wait_read = 0;
    {
        unsigned i;

        for (i = 0; i < 8; i++) {
            s->echo[i] = LOGIC_1;
        }
    }
    serial_tx_reset(&s->line.tx);
    serial_rx_reset(&s->line.rx);
    update_format(s);
    load_baud(s);
}

static void apply_reset(AcState *s) {
    LogicValue r = su_not(serial_level(s->sim, s->instance, s->pins->bus[B_RESET], LOGIC_1));

    if (r == LOGIC_X) {
        s->unknown = 1;
        s->in_reset = 0;
    } else if (r == LOGIC_1) {
        s->unknown = 0;
        s->in_reset = 1;
        reset_state(s);
        /* Status DCD / DSR follow the pins. */
        s->dcd_bit = serial_level(s->sim, s->instance, s->pins->pin[P_DCD], LOGIC_X);
        s->dsr_bit = serial_level(s->sim, s->instance, s->pins->pin[P_DSR], LOGIC_X);
        serial_line_sender_reset(&s->line, s->sim, s->instance);
    } else {
        if (s->unknown) {
            reset_state(s);
            s->command = su_reg(0, 0);
            s->control = su_reg(0, 0);
            s->tdre = s->rdrf = s->pe = s->fe = s->ovr = LOGIC_X;
            s->tsr = LOGIC_X;
            update_format(s);
            load_baud(s);
        }
        s->unknown = 0;
        s->in_reset = 0;
    }
    update_outputs(s, 0);
}

/* ---- Registers -------------------------------------------------------------------------------- */

static void write_reg(AcState *s, unsigned rs, uint8_t v, uint8_t k) {
    switch (rs & 3u) {
        case 0:
            s->tdr = su_reg(v, k);
            s->tdre = LOGIC_0;
            s->committed = dtr_bit(s) == LOGIC_1;
            if (s->dev->w65c51n && serial_tx_busy(&s->line.tx)) {
                s->cut = 1;   /* loads the TSR at once */
            }
            break;
        case 1:
            /* Programmed reset (p14): OVR cleared, command b4..b0 = 0;
               DCD/DSR interrupts off (an IRQ they caused is released). */
            s->ovr = LOGIC_0;
            s->command.v &= 0xE0u;
            s->command.k |= 0x1Fu;
            s->int_modem = 0;
            s->frozen = 0;
            update_format(s);
            break;
        case 2: {
            unsigned old_tic = tic(s);

            s->command = su_reg(v, k);
            if (old_tic == 3 && tic(s) != 3 && s->break_on) {
                s->break_on = 0;
                s->break_end = 16;   /* a stop bit, then normal transmission */
            }
            update_format(s);
            break;
        }
        default:
            s->control = su_reg(v, k);
            update_format(s);
            load_baud(s);
            break;
    }
}

static void read_done(AcState *s, unsigned rs) {
    if ((rs & 3u) == 0) {
        s->rdrf = LOGIC_0;
        s->int_rx = 0;
        if (s->dev->w65c51n) {
            /* p.10: overrun, framing and parity "are automatically cleared
               after a read of the Receiver Data Register". */
            s->ovr = s->fe = s->pe = LOGIC_0;
        }
        if (s->echo_wait_read) {
            s->echo_wait_read = 0;   /* the mark ends at the next start bit */
        }
    } else if ((rs & 3u) == 1) {
        LogicValue dcd = serial_level(s->sim, s->instance, s->pins->pin[P_DCD], LOGIC_X);
        LogicValue dsr = serial_level(s->sim, s->instance, s->pins->pin[P_DSR], LOGIC_X);

        s->int_rx = 0;
        s->int_modem = 0;
        s->frozen = 0;
        /* Levels changed meanwhile: another interrupt at once (p5). */
        if (dcd != s->dcd_bit || dsr != s->dsr_bit) {
            modem_sample(s, 1);
        }
        if (!s->frozen) {
            s->dcd_bit = dcd;
            s->dsr_bit = dsr;
        }
    }
}

/* ---- Bus ------------------------------------------------------------------------------------- */

static void bus_update(AcState *s) {
    LogicValue cs0 = serial_level(s->sim, s->instance, s->pins->bus[B_SELECT_HIGH], LOGIC_0);
    LogicValue cs1 = serial_level(s->sim, s->instance, s->pins->bus[B_SELECT], LOGIC_1);
    LogicValue phi2 = serial_level(s->sim, s->instance, s->pins->bus[B_PHI2], LOGIC_X);
    LogicValue rw = serial_level(s->sim, s->instance, s->pins->bus[B_RW], LOGIC_X);
    LogicValue sel = su_and(cs0, su_not(cs1));
    unsigned rs;
    unsigned rs_known;
    int falling = s->phi2 == LOGIC_1 && phi2 != LOGIC_1;
    int cycle = 0;

    su_read_bus(s->sim, s->instance, s->pins->rs, 2, &rs, &rs_known);
    if (s->in_reset || s->unknown) {
        s->cycle = 0;
        s->phi2 = phi2;
        drive_data(s);
        return;
    }
    if (sel != LOGIC_0 && phi2 != LOGIC_0) {
        if (sel == LOGIC_1 && phi2 == LOGIC_1 && rw == LOGIC_1) {
            cycle = 1;
        } else if (sel == LOGIC_1 && phi2 == LOGIC_1 && rw == LOGIC_0) {
            cycle = 2;
        } else {
            cycle = 3;
        }
    }
    if (falling && s->cycle != 0) {
        /* PHI2 falls: the cycle ends. */
        if (s->cycle == 2 || s->cycle == 3) {
            unsigned v;
            unsigned k;
            unsigned a;

            su_read_bus(s->sim, s->instance, s->pins->data, 8, &v, &k);
            if (s->cycle == 2 && s->cycle_rs_known == 3u) {
                write_reg(s, s->cycle_rs, (uint8_t)v, (uint8_t)k);
            } else {
                /* X cycle or address: X into every register it could name
                   (a programmed reset has no data to make X). */
                for (a = 0; a < 4; a++) {
                    if ((a & s->cycle_rs_known) == (s->cycle_rs & s->cycle_rs_known) && a != 1) {
                        write_reg(s, a, 0, 0);
                    }
                }
            }
        } else if (s->cycle == 1 && s->cycle_rs_known == 3u) {
            read_done(s, s->cycle_rs);
        }
        cycle = 0;
    }
    s->phi2 = phi2;
    s->cycle = cycle;
    if (cycle != 0) {
        s->cycle_rs = rs;
        s->cycle_rs_known = rs_known;
    }
    update_outputs(s, t_of(s, T_IRQ));
}

/* ---- Transmitter --------------------------------------------------------------------------------- */

static int tx_fetch(void *context, uint16_t *data, uint16_t *known) {
    AcState *s = context;

    if (s->tdre != LOGIC_0 || s->break_end > 0) {
        return 0;
    }
    if (dtr_bit(s) != LOGIC_1 && !s->committed) {
        return 0;
    }
    if (tic(s) == 3) {
        return 0;   /* break: no new characters */
    }
    if (s->cts_a && cts_high(s) != LOGIC_0) {
        return 0;   /* CDP65C51A: held while CTS is high */
    }
    *data = s->tdr.v;
    *known = s->tdr.k;
    s->tdre = LOGIC_1;   /* "at the beginning of the start bit" */
    s->committed = 0;
    return 1;
}

static void tx_done(void *context, uint16_t data, uint16_t known) {
    AcState *s = context;

    (void)known;
    serial_line_log(&s->line, data);
}

/* 1 when this tick ends the bit the transmitter is sending. */
static int tx_bit_ends(const SerialTx *tx) {
    switch (tx->phase) {
        case SERIAL_TX_START:
        case SERIAL_TX_DATA:
        case SERIAL_TX_PARITY:
            return tx->count + 1 >= tx->format.factor;
        case SERIAL_TX_STOP:
            return (tx->stop_ticks + 1) % tx->format.factor == 0;
        default:
            return 1;
    }
}

static void tx_tick(AcState *s) {
    SerialTxEvents events = {s, tx_fetch, NULL, tx_done};

    if (s->in_reset || s->unknown) {
        return;
    }
    if (s->cut && (!serial_tx_busy(&s->line.tx) || tx_bit_ends(&s->line.tx))) {
        /* W65C51N: the new byte's start bit begins at this bit boundary. A
           frame cut in its stop bits had sent all its data: it is logged. */
        if (s->line.tx.phase == SERIAL_TX_STOP) {
            uint16_t mask = (uint16_t)((1u << s->line.tx.format.data_bits) - 1);

            serial_line_log(&s->line, s->line.tx.data & mask);
        }
        s->cut = 0;
        serial_tx_reset(&s->line.tx);
    }
    if (s->break_end > 0) {
        s->break_end--;
        s->tsr = LOGIC_1;
    } else {
        s->tsr = serial_tx_tick(&s->line.tx, &s->format, &events);
    }
    /* Break: after the TDR and the shift register are empty (p13). */
    s->break_on = tic(s) == 3 && s->tdre == LOGIC_1 && !serial_tx_busy(&s->line.tx);
    update_outputs(s, t_of(s, T_IRQ));
}

/* ---- Receiver ---------------------------------------------------------------------------------- */

static void on_received(void *context, const SerialRxChar *c) {
    AcState *s = context;
    unsigned wl = word_length(s);
    uint8_t mask = (uint8_t)((1u << wl) - 1);
    unsigned pmc = (s->command.v >> 6) & 3u;
    LogicValue pe = (c->flags_known & SERIAL_RX_PE) ? su_bool(c->flags & SERIAL_RX_PE) : LOGIC_X;
    LogicValue fe = (c->flags_known & SERIAL_RX_FE) ? su_bool(c->flags & SERIAL_RX_FE) : LOGIC_X;
    LogicValue dcd = serial_level(s->sim, s->instance, s->pins->pin[P_DCD], LOGIC_X);

    if (s->format.parity == SERIAL_PARITY_MARK || s->format.parity == SERIAL_PARITY_SPACE ||
        pmc >= 2 || s->dev->w65c51n) {
        pe = LOGIC_0;   /* mark/space: the check is disabled */
    }
    if (fe == LOGIC_1 && !(c->flags & SERIAL_RX_BI)) {
        s->resync = 1;
    }
    if (s->rdrf == LOGIC_1) {
        /* Overrun: the new character is not transferred (p9). The W65C51N
           transfers it and flags the overrun (w65c51n.pdf p.7). */
        s->ovr = LOGIC_1;
        if (echo_mode(s)) {
            s->echo_mark = 1;
            s->echo_wait_read = 1;
        }
        if (!s->dev->w65c51n) {
            return;
        }
    }
    s->rdr = su_reg((uint8_t)(c->data & mask), s->format.format_x ? 0 : (uint8_t)((c->known & mask) | ~mask));
    s->rdrf = LOGIC_1;
    s->pe = pe;
    s->fe = fe;
    /* RX interrupt: IRD = 0, enabled, DCD low (DCD high disables it). */
    if (dtr_bit(s) == LOGIC_1 && !(s->command.v & 0x02u) && dcd == LOGIC_0) {
        s->int_rx = 1;
    }
}

static void rx_tick(AcState *s) {
    SerialRxEvents events = {s, on_received, NULL};
    LogicValue pin = serial_level(s->sim, s->instance, s->pins->pin[P_RX], LOGIC_X);
    LogicValue level;

    if (s->in_reset || s->unknown) {
        return;
    }
    level = serial_line_rx_input(&s->line, s->sim, s->instance, pin, &s->format);
    /* Echo: TXD repeats RXD half a bit later (8 ticks). */
    s->echo[s->echo_at] = level;
    s->echo_at = (s->echo_at + 1) % 8u;
    if (s->echo_mark && !s->echo_wait_read && level == LOGIC_0) {
        s->echo_mark = 0;
    }
    /* Disabled (command b0 = 0): a character in progress completes. */
    if (dtr_bit(s) == LOGIC_1 || serial_rx_busy(&s->line.rx)) {
        s->resync = 0;
        serial_rx_tick(&s->line.rx, level, &s->format, &events);
        if (s->resync) {
            su_rx_no_resync(&s->line.rx, level);
        }
    } else {
        s->line.rx.last = level;
    }
    update_outputs(s, t_of(s, T_IRQ));
}

/* ---- Engine callbacks ---------------------------------------------------------------------------- */

static void xtli_edge(AcState *s, LogicValue level) {
    int rising = s->xtli == LOGIC_0 && level == LOGIC_1;
    int falling = s->xtli == LOGIC_1 && level == LOGIC_0;
    int tick;

    s->xtli = level;
    if (!rising && !falling) {
        return;
    }
    tick = serial_baud_edge(&s->baud, rising, level);
    if (tick) {
        tx_tick(s);
        if (rcs_internal(s)) {
            rx_tick(s);
        }
    } else if (rcs_internal(s)) {
        update_outputs(s, 0);   /* RXC follows the generator */
    }
}

static void ac_start(Sim *sim, SimInstance *instance, void *raw) {
    AcState *s = raw;

    s->sim = sim;
    s->instance = instance;
    s->xtli = serial_level(sim, instance, s->pins->clock[K_IN], LOGIC_X);
    s->rxc = serial_level(sim, instance, s->pins->clock[K_RECEIVE], LOGIC_X);
    s->phi2 = serial_level(sim, instance, s->pins->bus[B_PHI2], LOGIC_X);
    apply_reset(s);
}

static void ac_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    AcState *s = raw;
    const AcPins *p = s->pins;

    (void)old_value;
    (void)new_value;
    s->sim = sim;
    s->instance = instance;
    if (pin == p->bus[B_RESET]) {
        apply_reset(s);
        return;
    }
    if (pin == p->clock[K_IN]) {
        xtli_edge(s, serial_level(sim, instance, pin, LOGIC_X));
        return;
    }
    if (pin == p->clock[K_RECEIVE] && !rcs_internal(s)) {
        LogicValue v = serial_level(sim, instance, pin, LOGIC_X);
        int rising = s->rxc == LOGIC_0 && v == LOGIC_1;

        s->rxc = v;
        if (rising) {
            rx_tick(s);
        }
        return;
    }
    if (pin == p->pin[P_DCD] || pin == p->pin[P_DSR]) {
        if (!s->in_reset && !s->unknown) {
            modem_sample(s, 1);
        } else {
            s->dcd_bit = serial_level(sim, instance, p->pin[P_DCD], LOGIC_X);
            s->dsr_bit = serial_level(sim, instance, p->pin[P_DSR], LOGIC_X);
        }
        update_outputs(s, t_of(s, T_IRQ));
        return;
    }
    if (pin == p->pin[P_CTS]) {
        update_outputs(s, t_of(s, T_IRQ));
        return;
    }
    bus_update(s);
}

/* ---- Terminal ---------------------------------------------------------------------------------- */

static unsigned ac_channels(const void *raw) {
    (void)raw;
    return 1;
}

static SerialLine *ac_line(void *raw, unsigned channel) {
    (void)channel;
    return &((AcState *)raw)->line;
}

static int ac_async(const void *raw, unsigned channel) {
    (void)raw;
    (void)channel;
    return 1;
}

/* ---- Create / destroy / loader ------------------------------------------------------------------- */

static void *ac_create(const Part *part, const void *data, const void *pins,
                       const SerialGrade *grade, const void *options) {
    AcState *s = calloc(1, sizeof *s);
    const AcOptions *opt = options;
    size_t i;

    if (s == NULL) {
        return NULL;
    }
    s->base.model = &ACIA6551_MODEL;
    s->dev = data;
    s->pins = pins;
    s->grade = grade;
    s->cts_a = opt != NULL && opt->cts_a;
    s->o.count = part->pin_count;
    s->o.out = malloc(part->pin_count * sizeof *s->o.out);
    if (s->o.out == NULL) {
        free(s);
        return NULL;
    }
    for (i = 0; i < part->pin_count; i++) {
        s->o.out[i] = LOGIC_Z;
    }
    serial_line_init(&s->line, s->pins->pin[P_RX]);
    s->rdr = su_reg(0, 0);
    s->tdr = su_reg(0, 0);
    s->dcd_bit = s->dsr_bit = LOGIC_X;
    reset_state(s);
    s->xtli = s->rxc = s->phi2 = LOGIC_X;
    return s;
}

static void ac_destroy(void *raw) {
    AcState *s = raw;

    serial_line_free(&s->line);
    free(s->o.out);
    free(s);
}

static void *ac_load_device(KindContext *ctx, const cJSON *json, unsigned *channels) {
    AcDevice *d = kind_alloc(ctx, sizeof *d);
    const cJSON *bus;
    unsigned i;

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
        su_load_names(ctx, bus, "registerSelect", 1, d->rs, 2, 2, NULL) != 0) {
        return NULL;
    }
    for (i = 0; i < B_COUNT; i++) {
        if (kind_string(ctx, bus, BUS_NAMES[i], &d->bus[i]) != 0) {
            return NULL;
        }
    }
    kind_pop(ctx);
    if (su_load_pin_object(ctx, json, "clock", 1, CLOCK_KEYS, d->clock, 0x7u) != 0 ||
        su_load_pin_object(ctx, json, "serial", 1, SERIAL_KEYS, d->pin, 0xFFu) != 0) {
        return NULL;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "w65c51n") != NULL &&
        kind_bool(ctx, json, "w65c51n", &d->w65c51n) != 0) {
        return NULL;
    }
    *channels = 1;
    return d;
}

static void *ac_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t pin_count) {
    const AcDevice *d = raw;
    AcPins *p = kind_alloc(ctx, sizeof *p);

    if (p == NULL ||
        su_bind_names(ctx, pins, pin_count, "bus.data", d->data, p->data, 8) != 0 ||
        su_bind_names(ctx, pins, pin_count, "bus.registerSelect", d->rs, p->rs, 2) != 0 ||
        su_bind_names(ctx, pins, pin_count, "bus", d->bus, p->bus, B_COUNT) != 0 ||
        su_bind_object(ctx, pins, pin_count, "clock", CLOCK_KEYS, d->clock, p->clock) != 0 ||
        su_bind_object(ctx, pins, pin_count, "serial", SERIAL_KEYS, d->pin, p->pin) != 0) {
        return NULL;
    }
    return p;
}

static const void *ac_load_part(KindContext *ctx, const void *data, const cJSON *json) {
    AcOptions *o;

    (void)data;
    if (cJSON_GetObjectItemCaseSensitive(json, "ctsA") == NULL) {
        return NULL;
    }
    o = kind_alloc(ctx, sizeof *o);
    if (o == NULL || kind_bool(ctx, json, "ctsA", &o->cts_a) != 0) {
        return NULL;
    }
    return o;
}

static int ac_pin_open_drain(const void *data, const void *pins, const void *options, size_t pin) {
    (void)data;
    (void)options;
    return pin == ((const AcPins *)pins)->pin[P_IRQ];
}

const SerialModel ACIA6551_MODEL = {
    "acia6551", KEYS, TIMING, PART_KEYS,
    ac_load_device, ac_bind, ac_load_part, ac_pin_open_drain,
    ac_create, ac_destroy, ac_start, ac_pin_changed, NULL,
    ac_channels, ac_line, ac_async
};
