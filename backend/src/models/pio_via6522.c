/*
 * via6522: the WDC W65C22 versatile interface adapter (plan-14-C, D;
 * datasheets/pio/w65c22.pdf).
 *
 * 16 registers (Table 2-1): ORB/IRB, ORA/IRA (with handshake), DDRB, DDRA,
 * T1 counter and latches, T2 counter and low latch, SR, ACR, PCR, IFR, IER,
 * ORA/IRA without handshake. Writes act at the PHI2 fall (bus65); port
 * outputs follow tcpw later. Read side effects (flag clears, the CA2 read
 * strobe) act at the PHI2 fall of the read.
 *
 * Ports (section 2.1): DDR 1 drives from OR, DDR 0 floats. IRA reads the PA
 * pins (every bit); IRB reads ORB for outputs and the pins for inputs. With
 * latching (ACR bits 0, 1) a read returns the pins as they were at the last
 * active CA1/CB1 edge; once read, the register is transparent again until
 * the next edge (p.8). A pin that changed less than tpcr before the read's
 * rise reads X; a latched pin that changed less than til before the edge,
 * or within tpdh after it, latches X. With ACR bit 7, T1 drives PB7 and IRB
 * bit 7 reads its level.
 *
 * Control lines (PCR, Table 2-5): CA1/CB1 set IFR1/IFR4 on the active edge.
 * CA2/CB2: input (IFR0/IFR3 on the selected edge; cleared by an ORA/ORB
 * access unless "independent"), handshake output, pulse output, low, high.
 *
 * Timers (sections 2.5-2.10): each counter decrements at every PHI2 fall
 * but the one that loaded it. A count that wraps from 0 times out at the
 * next PHI2 rise: N + 1.5 cycles after the start write's fall. T1 one-shot
 * sets IFR6 once per start and keeps rolling; T1 free-run reloads from the
 * latch at the next fall (a period of L + 2 cycles) and toggles PB7. T2
 * one-shot as T1 one-shot; T2 pulse counting decrements once per PB6 low
 * sampled at a PHI2 rise after a high one, and sets IFR5 when it reaches 0.
 * PB6 pulses narrower than, or closer than, 2 PHI2 cycles make it X.
 *
 * Shift register (sections 2.11-2.13, ACR bits 4-2): bit 7 shifts out
 * first on CB2 and rotates into bit 0; bits shift in at bit 0. With T2
 * (001, 100, 101) CB1 toggles every N + 2 PHI2 cycles (N = the T2 low
 * latch); with PHI2 (010, 110) it is low one cycle and high the next. An
 * output bit leaves tsr1 after the PHI2 fall where CB1 falls; an input bit
 * is sampled at the PHI2 rise after CB1 rises (X if CB2 changed less than
 * tsr2 before it) and shifted in at that cycle's fall. 001, 010, 101, 110
 * stop after 8 bits and set IFR2; 100 never stops and never sets IFR2.
 * External clock (011 in on CB1 rises, 111 out on CB1 falls) shifts at the
 * PHI2 fall after the edge, sets IFR2 every 8 bits and keeps going; an edge
 * less than tpcr before that fall makes the SR and its count X. An SR
 * access restarts the count, clears IFR2 and starts the internal clock.
 *
 * IRQB is asserted while IFR & IER & 0x7F is not 0, with no delay; open
 * drain ("irqOpenDrain": W65C22N) or totem pole (W65C22S).
 *
 * RESB low clears every register but the T1 and T2 counters and latches and
 * the SR, makes every pin an input and stops the timers (they restart on
 * their start write). RESB X makes everything X. Registers power up X.
 * "busHold" (W65C22S): an input at Z reads the last level it had.
 */

#include <stdlib.h>
#include <string.h>

#include "pio.h"

enum {
    T_TCYC, T_TPWH, T_TPWL, T_TACR, T_TCAR, T_TCDR, T_THR, T_TPCR, T_TACW, T_TCAW, T_TDCW, T_THW,
    T_TCPW, T_TCA2, T_TRS1, T_TRS2, T_TWHS, T_TDS, T_TRS3, T_TRS4, T_T21, T_TIL, T_TSR1, T_TSR2,
    T_TAL, T_TPDH, T_COUNT
};

static const char *const TIMING[] = {
    "tcyc", "tpwh", "tpwl", "tacr", "tcar", "tcdr", "thr", "tpcr", "tacw", "tcaw", "tdcw", "thw",
    "tcpw", "tca2", "trs1", "trs2", "twhs", "tds", "trs3", "trs4", "t21", "til", "tsr1", "tsr2",
    "tal", "tpdh", NULL
};

static const char *const KEYS[] = {
    "phi2", "rw", "reset", "data", "csHigh", "csLow", "rs", "pa", "pb",
    "ca1", "ca2", "cb1", "cb2", "irq", "irqOpenDrain", "busHold", NULL
};

enum { MAX_PINS = 64 };

typedef struct {
    PioNames names;
    const char *irq;
    int irq_open_drain;
    int bus_hold;
} ViaDevice;

typedef struct {
    PioPins p;
    size_t irq;
    const ViaDevice *device;
} ViaPins;

enum {
    R_ORB, R_ORA, R_DDRB, R_DDRA, R_T1CL, R_T1CH, R_T1LL, R_T1LH, R_T2CL, R_T2CH, R_SR, R_ACR,
    R_PCR, R_IFR, R_IER, R_ORA_NH
};

enum {
    F_CA2 = 1 << 0, F_CA1 = 1 << 1, F_SR = 1 << 2, F_CB2 = 1 << 3, F_CB1 = 1 << 4,
    F_T2 = 1 << 5, F_T1 = 1 << 6
};

/* C2 modes from PCR bits 3-1 (CA2) / 7-5 (CB2). */
enum { C2_IN_NEG, C2_IND_NEG, C2_IN_POS, C2_IND_POS, C2_HANDSHAKE, C2_PULSE, C2_LOW, C2_HIGH,
       C2_UNKNOWN };

/* A 16-bit counter with its known bits. */
typedef struct {
    uint16_t v, k;
} Word;

/* One side's port and control lines (0 = A, 1 = B). */
typedef struct {
    uint8_t out, out_k;       /* OR */
    uint8_t ddr, ddr_k;
    uint8_t latch, latch_k;   /* IRA/IRB as latched at the C1 edge */
    int latched;              /* latch holds a value (else transparent) */
    SimTime latch_at;
    LogicValue c1, c2;        /* input levels last seen */
    LogicValue c2_out;        /* handshake/pulse output level */
    int arm;                  /* written: the write strobe acts at the next rise */
    int pulse_fall;           /* CA2 read pulse: high at the next fall */
    int pulse_rise;           /* write pulse: high at the next rise */
    int strobe_read;          /* the low came from a read handshake (CA2 only) */
    int read_or;              /* RS 1 (A) / RS 0 (B) read this cycle: clears flags at the fall */
    SimTime port_at[PIO_PORT_BITS];
    int port_seen[PIO_PORT_BITS];
} ViaSide;

typedef struct {
    PioBase base;
    Bus65 bus;
    const ViaPins *pins;
    const SimTime *t;
    Sim *sim;
    SimInstance *instance;
    LogicValue reset;
    LogicValue phi2;
    SimTime last_rise, period;  /* the measured PHI2 period */

    ViaSide side[2];
    uint8_t acr, acr_k, pcr, pcr_k;
    uint8_t ifr, ifr_k, ier, ier_k;   /* bits 0-6 */
    uint8_t sr, sr_k;
    int sr_run;                /* the internal shift clock runs */
    unsigned sr_div;           /* PHI2 falls since the last T2-rate toggle */
    int sr_count;              /* bits shifted since the access; -1 = X */
    LogicValue cb1_out;        /* the shift clock CB1 drives */
    LogicValue cb2_sr;         /* the serial output CB2 drives */
    int sr_sample_pending;     /* input: sample CB2 at the next rise */
    int sr_shift_pending;      /* input: shift the sample in at the next fall */
    LogicValue sr_sample;
    int sr_access;             /* the SR was read or written this cycle */
    int sr_ext;                /* an external CB1 edge waits for the next fall */
    SimTime sr_ext_at;
    SimTime cb2_at;
    int cb2_seen;
    uint8_t pending_clear, pending_x; /* IFR bits a read clears (or makes X) at the fall */

    Word t1c, t1l, t2c;
    uint8_t t2ll, t2ll_k;
    int t1_run, t2_run;        /* counting: 1, 0, or -1 unknown */
    int t1_loaded, t2_loaded;  /* loaded at this fall: no decrement */
    int t1_armed, t2_armed;    /* the next time-out sets the flag: 1, 0, -1 X */
    int t1_timeout;            /* wrapped: times out at the next rise */
    int t1_reload;             /* free-run: reload at the next fall */
    int t2_timeout;
    LogicValue pb7;            /* T1's PB7 output */
    LogicValue pb6_rise;       /* PB6 at the last rise (pulse counting) */
    SimTime pb6_fall_at, pb6_high_at;   /* PB6's last falling / rising edge */
    int pb6_seen_fall, pb6_seen_high;

    LogicValue held[MAX_PINS];  /* bus hold */
} Via;

/* ---- Three-valued helpers ------------------------------------------------------- */

static LogicValue bitv(uint8_t v, uint8_t k, int bit) {
    if (!(k >> bit & 1)) return LOGIC_X;
    return (v >> bit & 1) ? LOGIC_1 : LOGIC_0;
}

static void set_bit(uint8_t *v, uint8_t *k, int bit, LogicValue x) {
    uint8_t m = (uint8_t)(1u << bit);

    if (x == LOGIC_0 || x == LOGIC_1) {
        *k |= m;
        if (x == LOGIC_1) *v |= m; else *v &= (uint8_t)~m;
    } else {
        *k &= (uint8_t)~m;
        *v &= (uint8_t)~m;
    }
}

/* Sets the flags in mask (to 1, or X when x). */
static void flag_set(Via *s, uint8_t mask, int x) {
    if (x) {
        /* a flag already 1 stays 1 */
        uint8_t ones = s->ifr & s->ifr_k & mask;

        s->ifr_k &= (uint8_t)~(mask & ~ones);
        s->ifr &= s->ifr_k;
    } else {
        s->ifr |= mask;
        s->ifr_k |= mask;
    }
}

static void flag_clear(Via *s, uint8_t mask) {
    s->ifr &= (uint8_t)~mask;
    s->ifr_k |= mask;
}

/* Flags in mask that are not known 0 become X. */
static void flag_maybe_clear(Via *s, uint8_t mask) {
    uint8_t zeros = (uint8_t)(~s->ifr & s->ifr_k & mask);

    s->ifr_k &= (uint8_t)~(mask & ~zeros);
    s->ifr &= s->ifr_k;
}

/* ---- Pins -------------------------------------------------------------------------- */

static LogicValue pin_level(const Via *s, size_t pin) {
    LogicValue v = sim_read_pin(s->sim, s->instance, pin);

    if (v == LOGIC_Z) {
        return s->pins->device->bus_hold && pin < MAX_PINS ? s->held[pin] : LOGIC_X;
    }
    return v;
}

static const size_t *port_pins(const Via *s, int side) {
    return side == 0 ? s->pins->p.pa : s->pins->p.pb;
}

static size_t c1_pin(const Via *s, int side) {
    return side == 0 ? s->pins->p.ca1 : s->pins->p.cb1;
}

static size_t c2_pin(const Via *s, int side) {
    return side == 0 ? s->pins->p.ca2 : s->pins->p.cb2;
}

static int c2_mode(const Via *s, int side) {
    int shift = side == 0 ? 1 : 5;
    uint8_t m = (uint8_t)(7u << shift);

    if ((s->pcr_k & m) != m) return C2_UNKNOWN;
    return (s->pcr >> shift) & 7;
}

static void update_irq(Via *s) {
    uint8_t on = s->ifr & s->ifr_k & s->ier & s->ier_k & 0x7F;
    /* a bit could be on when neither factor is known 0 */
    uint8_t maybe = (uint8_t)((s->ifr | ~s->ifr_k) & (s->ier | ~s->ier_k) & 0x7F);
    LogicValue v;

    if (on) v = LOGIC_0;
    else if (maybe) v = LOGIC_X;
    else v = s->pins->device->irq_open_drain ? LOGIC_Z : LOGIC_1;
    sim_schedule_inertial_drive(s->sim, s->instance, s->pins->irq, v, DRIVE_STRONG, 0);
}

/* IFR bit 7 as read. */
static LogicValue irq_bit(const Via *s) {
    uint8_t on = s->ifr & s->ifr_k & s->ier & s->ier_k & 0x7F;
    uint8_t maybe = (uint8_t)((s->ifr | ~s->ifr_k) & (s->ier | ~s->ier_k) & 0x7F);

    return on ? LOGIC_1 : maybe ? LOGIC_X : LOGIC_0;
}

static LogicValue acr_bit(const Via *s, int bit) {
    return bitv(s->acr, s->acr_k, bit);
}

static void drive_ports(Via *s, int side, SimTime delay) {
    const ViaSide *p = &s->side[side];
    const size_t *pins = port_pins(s, side);
    int i;

    for (i = 0; i < PIO_PORT_BITS; i++) {
        LogicValue dir = bitv(p->ddr, p->ddr_k, i);
        LogicValue v = dir == LOGIC_1 ? bitv(p->out, p->out_k, i) : dir == LOGIC_0 ? LOGIC_Z : LOGIC_X;

        if (side == 1 && i == 7) {
            LogicValue t1 = acr_bit(s, 7);

            if (t1 == LOGIC_1) v = s->pb7;
            else if (t1 == LOGIC_X) v = LOGIC_X;
        }
        sim_schedule_inertial_drive(s->sim, s->instance, pins[i], v, DRIVE_STRONG, delay);
    }
}

static void drive_pb7(Via *s, SimTime delay) {
    if (acr_bit(s, 7) == LOGIC_1) {
        sim_schedule_inertial_drive(s->sim, s->instance, s->pins->p.pb[7], s->pb7, DRIVE_STRONG,
                                    delay);
    }
}

/* Shift register modes (ACR bits 4-2). */
enum { SR_OFF, SR_IN_T2, SR_IN_PHI2, SR_IN_EXT, SR_OUT_FREE, SR_OUT_T2, SR_OUT_PHI2, SR_OUT_EXT };

static int sr_mode(const Via *s) {
    if ((s->acr_k & 0x1C) != 0x1C) return -1;
    return (s->acr >> 2) & 7;
}

static int sr_internal(int m) {
    return m == SR_IN_T2 || m == SR_IN_PHI2 || m == SR_OUT_FREE || m == SR_OUT_T2 ||
           m == SR_OUT_PHI2;
}

static int sr_t2(int m) {
    return m == SR_IN_T2 || m == SR_OUT_FREE || m == SR_OUT_T2;
}

/* CB1: the shift clock output in the internal-clock modes, else an input. */
static void drive_cb1(Via *s, SimTime delay) {
    int m = sr_mode(s);
    LogicValue v = m < 0 ? LOGIC_X : sr_internal(m) ? s->cb1_out : LOGIC_Z;

    sim_schedule_inertial_drive(s->sim, s->instance, s->pins->p.cb1, v, DRIVE_STRONG, delay);
}

static void drive_c2(Via *s, int side, SimTime delay) {
    int mode = c2_mode(s, side);
    LogicValue v;

    if (side == 1 && sr_mode(s) != SR_OFF) {
        /* CB2 is the serial line. */
        int m = sr_mode(s);

        v = m < 0 ? LOGIC_X : m >= SR_OUT_FREE ? s->cb2_sr : LOGIC_Z;
        sim_schedule_inertial_drive(s->sim, s->instance, c2_pin(s, side), v, DRIVE_STRONG, delay);
        return;
    }
    switch (mode) {
        case C2_HANDSHAKE:
        case C2_PULSE: v = s->side[side].c2_out; break;
        case C2_LOW: v = LOGIC_0; break;
        case C2_HIGH: v = LOGIC_1; break;
        case C2_UNKNOWN: v = LOGIC_X; break;
        default: v = LOGIC_Z; break;
    }
    sim_schedule_inertial_drive(s->sim, s->instance, c2_pin(s, side), v, DRIVE_STRONG, delay);
}

static void set_c2_out(Via *s, int side, LogicValue v, SimTime delay) {
    s->side[side].c2_out = v;
    drive_c2(s, side, delay);
}

/* ---- Reset -------------------------------------------------------------------------- */

static void clear_side(ViaSide *p, int known) {
    uint8_t k = known ? 0xFF : 0;

    p->out = p->ddr = p->latch = 0;
    p->out_k = p->ddr_k = k;
    p->latch_k = 0;
    p->latched = 0;
    p->c2_out = known ? LOGIC_1 : LOGIC_X;
    p->arm = p->pulse_fall = p->pulse_rise = p->strobe_read = p->read_or = 0;
}

/* RESB low (known) or X (unknown); power-up is the unknown case with the
   timers and SR unknown too. */
static void reset_registers(Via *s, int known, int timers_too) {
    uint8_t k = known ? 0xFF : 0;
    int side;

    for (side = 0; side < 2; side++) clear_side(&s->side[side], known);
    s->acr = s->pcr = s->ifr = s->ier = 0;
    s->acr_k = s->pcr_k = k;
    s->ifr_k = s->ier_k = (uint8_t)(k & 0x7F);
    s->pending_clear = s->pending_x = 0;
    s->t1_run = s->t2_run = known ? 0 : -1;
    s->t1_loaded = s->t2_loaded = 0;
    s->t1_armed = s->t2_armed = known ? 0 : -1;
    s->t1_timeout = s->t1_reload = s->t2_timeout = 0;
    s->pb7 = known ? LOGIC_1 : LOGIC_X;
    s->sr_run = 0;
    s->sr_div = 0;
    s->sr_count = 0;
    s->cb1_out = s->cb2_sr = known ? LOGIC_1 : LOGIC_X;
    s->sr_sample_pending = s->sr_shift_pending = s->sr_access = s->sr_ext = 0;
    if (timers_too) {
        s->t1c.v = s->t1l.v = s->t2c.v = 0;
        s->t1c.k = s->t1l.k = s->t2c.k = 0;
        s->t2ll = s->t2ll_k = 0;
        s->sr = s->sr_k = 0;
    }
    for (side = 0; side < 2; side++) {
        drive_ports(s, side, 0);
        drive_c2(s, side, 0);
    }
    drive_cb1(s, 0);
    update_irq(s);
}

/* ---- Reads ------------------------------------------------------------------------- */

static int recent(const ViaSide *p, int bit, SimTime now, SimTime window) {
    return p->port_seen[bit] && now - p->port_at[bit] < window;
}

/* The pins of a port as read now (tpcr rule), one bit. */
static LogicValue port_pin_read(const Via *s, int side, int bit, SimTime now) {
    const ViaSide *p = &s->side[side];
    LogicValue v = pin_level(s, port_pins(s, side)[bit]);

    return recent(p, bit, now, s->t[T_TPCR]) ? LOGIC_X : v;
}

static void read_input(Via *s, int side, uint8_t *value, uint8_t *known) {
    ViaSide *p = &s->side[side];
    LogicValue latching = acr_bit(s, side);
    SimTime now = sim_now(s->sim);
    int i;

    *value = 0;
    *known = 0;
    for (i = 0; i < PIO_PORT_BITS; i++) {
        LogicValue v;

        if (side == 1) {
            LogicValue dir = bitv(p->ddr, p->ddr_k, i);

            if (i == 7 && acr_bit(s, 7) != LOGIC_0) {
                set_bit(value, known, i, acr_bit(s, 7) == LOGIC_1 ? s->pb7 : LOGIC_X);
                continue;
            }
            if (dir == LOGIC_1) {
                set_bit(value, known, i, bitv(p->out, p->out_k, i));
                continue;
            }
            if (dir == LOGIC_X) continue;
        }
        if (latching == LOGIC_1 && p->latched) {
            v = bitv(p->latch, p->latch_k, i);
        } else if (latching == LOGIC_X && p->latched) {
            v = port_pin_read(s, side, i, now);
            if (v != bitv(p->latch, p->latch_k, i)) v = LOGIC_X;
        } else {
            v = port_pin_read(s, side, i, now);
        }
        set_bit(value, known, i, v);
    }
}

static void word_byte(Word w, int high, uint8_t *value, uint8_t *known) {
    *value = (uint8_t)(high ? w.v >> 8 : w.v);
    *known = (uint8_t)(high ? w.k >> 8 : w.k);
}

static int via_read(void *model, unsigned reg, uint8_t *value, uint8_t *known) {
    Via *s = model;

    switch (reg) {
        case R_ORB:
            read_input(s, 1, value, known);
            s->side[1].read_or = 1;
            break;
        case R_ORA:
        case R_ORA_NH:
            read_input(s, 0, value, known);
            if (reg == R_ORA) s->side[0].read_or = 1;
            s->side[0].latched = 0;   /* transparent until the next CA1 edge */
            break;
        case R_DDRB: *value = s->side[1].ddr; *known = s->side[1].ddr_k; break;
        case R_DDRA: *value = s->side[0].ddr; *known = s->side[0].ddr_k; break;
        case R_T1CL: word_byte(s->t1c, 0, value, known); s->pending_clear |= F_T1; break;
        case R_T1CH: word_byte(s->t1c, 1, value, known); break;
        case R_T1LL: word_byte(s->t1l, 0, value, known); break;
        case R_T1LH: word_byte(s->t1l, 1, value, known); break;
        case R_T2CL: word_byte(s->t2c, 0, value, known); s->pending_clear |= F_T2; break;
        case R_T2CH: word_byte(s->t2c, 1, value, known); break;
        case R_SR:
            *value = s->sr;
            *known = s->sr_k;
            s->pending_clear |= F_SR;
            s->sr_access = 1;
            break;
        case R_ACR: *value = s->acr; *known = s->acr_k; break;
        case R_PCR: *value = s->pcr; *known = s->pcr_k; break;
        case R_IFR:
            *value = s->ifr;
            *known = s->ifr_k;
            set_bit(value, known, 7, irq_bit(s));
            break;
        case R_IER:
            *value = (uint8_t)(s->ier | 0x80);
            *known = (uint8_t)(s->ier_k | 0x80);
            break;
    }
    if (reg == R_ORB) s->side[1].latched = 0;
    return 1;
}

static void via_read_unknown(void *model) {
    Via *s = model;

    /* Any read side effect may have happened. */
    s->pending_x |= F_CA1 | F_CA2 | F_CB1 | F_CB2 | F_T1 | F_T2 | F_SR;
    if (c2_mode(s, 0) == C2_HANDSHAKE || c2_mode(s, 0) == C2_PULSE) {
        set_c2_out(s, 0, LOGIC_X, s->t[T_TCA2]);
    }
}

/* ---- Writes ------------------------------------------------------------------------ */

/* An ORA (RS 1) / ORB access: clears C1's flag and C2's unless independent. */
static uint8_t or_access_flags(const Via *s, int side) {
    int mode = c2_mode(s, side);
    uint8_t c1 = side == 0 ? F_CA1 : F_CB1;
    uint8_t c2 = side == 0 ? F_CA2 : F_CB2;

    if (mode == C2_IND_NEG || mode == C2_IND_POS) return c1;
    return (uint8_t)(c1 | c2);
}

static void enter_c2_mode(Via *s, int side, int old_mode) {
    ViaSide *p = &s->side[side];
    int mode = c2_mode(s, side);
    int out = mode == C2_HANDSHAKE || mode == C2_PULSE;

    if (out && old_mode != C2_HANDSHAKE && old_mode != C2_PULSE) {
        p->c2_out = LOGIC_1;
        p->arm = p->pulse_fall = p->pulse_rise = 0;
    } else if (!out) {
        p->arm = p->pulse_fall = p->pulse_rise = 0;
    }
    drive_c2(s, side, 0);
}

static void word_set(Word *w, int high, uint8_t value, uint8_t known) {
    if (high) {
        w->v = (uint16_t)((w->v & 0x00FF) | (value & known) << 8);
        w->k = (uint16_t)((w->k & 0x00FF) | known << 8);
    } else {
        w->v = (uint16_t)((w->v & 0xFF00) | (value & known));
        w->k = (uint16_t)((w->k & 0xFF00) | known);
    }
}

static void write_or(Via *s, int side, uint8_t value, uint8_t known, int handshake) {
    ViaSide *p = &s->side[side];

    p->out = value & known;
    p->out_k = known;
    if (handshake) {
        if (known == 0) {
            flag_maybe_clear(s, or_access_flags(s, side));
        } else {
            flag_clear(s, or_access_flags(s, side));
        }
        if (c2_mode(s, side) == C2_HANDSHAKE || c2_mode(s, side) == C2_PULSE) {
            if (known == 0) set_c2_out(s, side, LOGIC_X, 0);
            else p->arm = 1;
        }
    }
    drive_ports(s, side, s->t[T_TCPW]);
}

static void t1_start(Via *s, int known) {
    s->t1c = s->t1l;
    s->t1_run = known ? 1 : -1;
    s->t1_loaded = 1;
    s->t1_armed = known ? 1 : -1;
    s->t1_timeout = s->t1_reload = 0;
    if (known) flag_clear(s, F_T1); else flag_maybe_clear(s, F_T1);
    if (acr_bit(s, 7) != LOGIC_0) {
        s->pb7 = known ? LOGIC_0 : LOGIC_X;
        drive_pb7(s, s->t[T_TCPW]);
    }
}

static void t2_start(Via *s, int known) {
    s->t2c.v = (uint16_t)((s->t2c.v & 0xFF00) | s->t2ll);
    s->t2c.k = (uint16_t)((s->t2c.k & 0xFF00) | s->t2ll_k);
    s->t2_run = known ? 1 : -1;
    s->t2_loaded = 1;
    s->t2_armed = known ? 1 : -1;
    s->t2_timeout = 0;
    if (known) flag_clear(s, F_T2); else flag_maybe_clear(s, F_T2);
}

static void via_write(void *model, unsigned reg, uint8_t value, uint8_t known) {
    Via *s = model;
    int full = known == 0xFF;

    if (s->reset != LOGIC_1) {
        return;   /* held in reset, or X: already unknown */
    }
    switch (reg) {
        case R_ORB: write_or(s, 1, value, known, 1); break;
        case R_ORA: write_or(s, 0, value, known, 1); break;
        case R_ORA_NH: write_or(s, 0, value, known, 0); break;
        case R_DDRB:
        case R_DDRA: {
            int side = reg == R_DDRB ? 1 : 0;

            s->side[side].ddr = value & known;
            s->side[side].ddr_k = known;
            drive_ports(s, side, s->t[T_TCPW]);
            break;
        }
        case R_T1CL:
        case R_T1LL: word_set(&s->t1l, 0, value, known); break;
        case R_T1CH:
            word_set(&s->t1l, 1, value, known);
            t1_start(s, full);
            break;
        case R_T1LH:
            word_set(&s->t1l, 1, value, known);
            if (full) flag_clear(s, F_T1); else flag_maybe_clear(s, F_T1);
            break;
        case R_T2CL:
            s->t2ll = value & known;
            s->t2ll_k = known;
            break;
        case R_T2CH:
            word_set(&s->t2c, 1, value, known);
            t2_start(s, full);
            break;
        case R_SR:
            s->sr = value & known;
            s->sr_k = known;
            if (full) flag_clear(s, F_SR); else flag_maybe_clear(s, F_SR);
            s->sr_access = 1;
            break;
        case R_ACR: {
            int old_sr = sr_mode(s);

            s->acr = value & known;
            s->acr_k = known;
            if (sr_mode(s) != old_sr) {
                s->sr_run = 0;
                s->sr_div = 0;
                s->sr_count = 0;
                s->sr_sample_pending = s->sr_shift_pending = s->sr_ext = 0;
                s->cb1_out = LOGIC_1;
                if (sr_mode(s) == SR_OUT_FREE) s->sr_run = 1;
                if (sr_mode(s) == SR_OFF) flag_clear(s, F_SR);
                else if (sr_mode(s) < 0) flag_maybe_clear(s, F_SR);
                drive_cb1(s, 0);
                drive_c2(s, 1, 0);
            }
            drive_ports(s, 1, s->t[T_TCPW]);
            break;
        }
        case R_PCR: {
            int old_a = c2_mode(s, 0);
            int old_b = c2_mode(s, 1);

            s->pcr = value & known;
            s->pcr_k = known;
            enter_c2_mode(s, 0, old_a);
            enter_c2_mode(s, 1, old_b);
            break;
        }
        case R_IFR: {
            /* A 1 clears that flag; an unknown bit may. Bit 7 is ignored. */
            uint8_t ones = value & known & 0x7F;
            uint8_t x = (uint8_t)(~known & 0x7F);

            flag_clear(s, ones);
            flag_maybe_clear(s, x);
            break;
        }
        case R_IER: {
            uint8_t ones = value & known & 0x7F;
            uint8_t x = (uint8_t)(~known & 0x7F);
            LogicValue set = bitv(value, known, 7);

            if (set == LOGIC_1) {
                s->ier |= ones;
                s->ier_k |= ones;
            } else if (set == LOGIC_0) {
                s->ier &= (uint8_t)~ones;
                s->ier_k |= ones;
            } else {
                x |= ones;
            }
            s->ier_k &= (uint8_t)~x;
            s->ier &= s->ier_k;
            break;
        }
    }
    update_irq(s);
}

/* ---- Timers ------------------------------------------------------------------------- */

/* One PHI2 fall: decrement (wrap from 0 reports 1). */
static int word_dec(Word *w) {
    if (w->k != 0xFFFF) {
        w->v = 0;
        w->k = 0;
        return 0;
    }
    w->v = (uint16_t)(w->v - 1);
    return w->v == 0xFFFF;
}

static void t1_fall(Via *s) {
    if (s->t1_run == 0) {
        return;
    }
    if (s->t1_run < 0) {
        s->t1c.k = 0;
        s->t1c.v = 0;
        if (s->t1_armed != 0) flag_set(s, F_T1, 1);
        if (acr_bit(s, 7) != LOGIC_0) {
            s->pb7 = LOGIC_X;
            drive_pb7(s, 0);
        }
        return;
    }
    if (s->t1_loaded) {
        s->t1_loaded = 0;
        return;
    }
    if (s->t1_reload) {
        s->t1c = s->t1l;
        s->t1_reload = 0;
        return;
    }
    if (s->t1c.k != 0xFFFF) {
        /* unknown count: the time-out is unknown */
        if (s->t1_armed != 0) flag_set(s, F_T1, 1);
        if (acr_bit(s, 7) != LOGIC_0) {
            s->pb7 = LOGIC_X;
            drive_pb7(s, 0);
        }
        word_dec(&s->t1c);
        return;
    }
    if (word_dec(&s->t1c)) {
        s->t1_timeout = 1;
    }
}

static void t1_rise(Via *s) {
    LogicValue free_run = acr_bit(s, 6);

    if (!s->t1_timeout) {
        return;
    }
    s->t1_timeout = 0;
    if (free_run == LOGIC_1) {
        flag_set(s, F_T1, 0);
        s->t1_reload = 1;
        s->pb7 = s->pb7 == LOGIC_0 ? LOGIC_1 : s->pb7 == LOGIC_1 ? LOGIC_0 : LOGIC_X;
    } else if (free_run == LOGIC_0) {
        if (s->t1_armed == 1) {
            flag_set(s, F_T1, 0);
            s->pb7 = LOGIC_1;
        } else if (s->t1_armed < 0) {
            flag_set(s, F_T1, 1);
            s->pb7 = LOGIC_X;
        }
        s->t1_armed = 0;
    } else {
        flag_set(s, F_T1, 1);
        s->pb7 = LOGIC_X;
        s->t1_reload = 1;
    }
    drive_pb7(s, 0);
    update_irq(s);
}

static void t2_fall(Via *s) {
    LogicValue counting = acr_bit(s, 5);

    if (s->t2_run == 0 || counting == LOGIC_1) {
        s->t2_loaded = 0;
        return;
    }
    if (s->t2_run < 0 || counting == LOGIC_X) {
        s->t2c.k = 0;
        s->t2c.v = 0;
        if (s->t2_armed != 0) flag_set(s, F_T2, 1);
        return;
    }
    if (s->t2_loaded) {
        s->t2_loaded = 0;
        return;
    }
    if (s->t2c.k != 0xFFFF) {
        if (s->t2_armed != 0) flag_set(s, F_T2, 1);
        word_dec(&s->t2c);
        return;
    }
    if (word_dec(&s->t2c)) {
        s->t2_timeout = 1;
    }
}

static void t2_rise(Via *s) {
    if (acr_bit(s, 5) == LOGIC_1) {
        /* Pulse counting: a PB6 low at this rise after a high one counts. */
        LogicValue pb6 = pin_level(s, s->pins->p.pb[6]);

        if (s->t2_run == 1 && s->pb6_rise == LOGIC_1 && pb6 == LOGIC_0) {
            if (s->t2c.k != 0xFFFF) {
                if (s->t2_armed != 0) flag_set(s, F_T2, 1);
            } else {
                s->t2c.v = (uint16_t)(s->t2c.v - 1);
                if (s->t2c.v == 0 && s->t2_armed == 1) {
                    flag_set(s, F_T2, 0);
                    s->t2_armed = 0;
                }
            }
        } else if (s->t2_run == 1 && (pb6 == LOGIC_X || s->pb6_rise == LOGIC_X) &&
                   s->pb6_rise != pb6) {
            s->t2c.k = 0;
            s->t2c.v = 0;
            if (s->t2_armed != 0) flag_set(s, F_T2, 1);
        }
        s->pb6_rise = pb6;
        update_irq(s);
        return;
    }
    s->pb6_rise = pin_level(s, s->pins->p.pb[6]);
    if (!s->t2_timeout) {
        return;
    }
    s->t2_timeout = 0;
    if (sr_t2(sr_mode(s))) {
        return;   /* T2 times the shift clock: no IFR5 */
    }
    if (s->t2_armed == 1) {
        flag_set(s, F_T2, 0);
    } else if (s->t2_armed < 0) {
        flag_set(s, F_T2, 1);
    }
    s->t2_armed = 0;
    update_irq(s);
}

/* A PB6 edge in pulse-counting mode: a pulse narrower than two PHI2
   cycles (tipw), or one that starts less than two cycles after the last
   ended (tips), makes the count unknown. */
static void pb6_edge(Via *s, LogicValue v) {
    SimTime now = sim_now(s->sim);
    SimTime min = 2 * (s->period > 0 ? s->period : s->t[T_TCYC]);
    int bad = 0;

    if (v == LOGIC_0) {
        bad = s->pb6_seen_high && now - s->pb6_high_at < min;
        s->pb6_fall_at = now;
        s->pb6_seen_fall = 1;
    } else if (v == LOGIC_1) {
        bad = s->pb6_seen_fall && now - s->pb6_fall_at < min;
        s->pb6_high_at = now;
        s->pb6_seen_high = 1;
    }
    if (bad && acr_bit(s, 5) == LOGIC_1 && s->t2_run == 1) {
        s->t2c.k = 0;
        s->t2c.v = 0;
    }
}

/* ---- Shift register ------------------------------------------------------------------ */

static void sr_shift_in(Via *s, LogicValue bit) {
    s->sr = (uint8_t)(s->sr << 1);
    s->sr_k = (uint8_t)(s->sr_k << 1 | 1);
    set_bit(&s->sr, &s->sr_k, 0, bit);
}

/* Bit 7 out on CB2 (tsr1 after this fall), rotated into bit 0. */
static void sr_shift_out(Via *s) {
    LogicValue b = bitv(s->sr, s->sr_k, 7);

    s->sr = (uint8_t)(s->sr << 1);
    s->sr_k = (uint8_t)(s->sr_k << 1 | 1);
    set_bit(&s->sr, &s->sr_k, 0, b);
    s->cb2_sr = b;
    drive_c2(s, 1, s->t[T_TSR1]);
}

/* One bit done; 8 sets IFR2. Returns 1 at the 8th. */
static int sr_count_bit(Via *s) {
    if (s->sr_count < 0) {
        flag_set(s, F_SR, 1);
        return 0;
    }
    if (++s->sr_count < 8) {
        return 0;
    }
    flag_set(s, F_SR, 0);
    return 1;
}

static void sr_toggle(Via *s, int m) {
    if (s->cb1_out != LOGIC_0) {
        s->cb1_out = LOGIC_0;
        drive_cb1(s, 0);
        if (m >= SR_OUT_FREE) sr_shift_out(s);
        return;
    }
    s->cb1_out = LOGIC_1;
    drive_cb1(s, 0);
    if (m < SR_OUT_FREE) {
        s->sr_sample_pending = 1;
    } else if (m != SR_OUT_FREE && sr_count_bit(s)) {
        s->sr_run = 0;            /* 8 out: CB1 stays high, CB2 holds */
    }
}

/* Every PHI2 fall. */
static void sr_fall(Via *s) {
    int m = sr_mode(s);

    if (m == SR_OFF) {
        return;
    }
    if (m < 0) {
        s->sr = s->sr_k = 0;
        return;
    }
    if (s->sr_shift_pending) {
        s->sr_shift_pending = 0;
        sr_shift_in(s, s->sr_sample);
        if (sr_count_bit(s)) {
            s->sr_run = 0;        /* 8 in: CB1 stays high */
            return;
        }
    }
    if (m == SR_IN_EXT || m == SR_OUT_EXT) {
        if (!s->sr_ext) return;
        s->sr_ext = 0;
        if (sim_now(s->sim) - s->sr_ext_at < s->t[T_TPCR]) {
            /* An edge too close to PHI2 low: shifted or not, unknown (p.19). */
            s->sr = s->sr_k = 0;
            s->sr_count = -1;
            if (m == SR_OUT_EXT) {
                s->cb2_sr = LOGIC_X;
                drive_c2(s, 1, s->t[T_TSR1]);
            }
            flag_set(s, F_SR, 1);
            return;
        }
        if (m == SR_IN_EXT) {
            LogicValue v = pin_level(s, s->pins->p.cb2);

            sr_shift_in(s, v);
        } else {
            sr_shift_out(s);
        }
        if (sr_count_bit(s)) s->sr_count = 0;   /* keeps shifting */
        return;
    }
    if (!s->sr_run) {
        return;
    }
    if (sr_t2(m)) {
        unsigned n = s->t2ll_k == 0xFF ? s->t2ll : 0;

        if (++s->sr_div < n + 2) return;
        s->sr_div = 0;
        s->t2c.v = (uint16_t)((s->t2c.v & 0xFF00) | s->t2ll);
        s->t2c.k = (uint16_t)((s->t2c.k & 0xFF00) | s->t2ll_k);
    }
    sr_toggle(s, m);
}

/* An SR access, at its PHI2 fall: restart the count and the clock. */
static void sr_start(Via *s) {
    int m = sr_mode(s);

    s->sr_access = 0;
    if (m <= SR_OFF) {
        return;
    }
    s->sr_count = 0;
    s->sr_sample_pending = s->sr_shift_pending = 0;
    if (!sr_internal(m)) {
        return;
    }
    s->sr_run = 1;
    s->sr_div = 0;
    s->cb1_out = LOGIC_1;
    if (m == SR_IN_PHI2 || m == SR_OUT_PHI2) {
        sr_toggle(s, m);          /* CB1 low from the cycle after the access */
    } else {
        drive_cb1(s, 0);
    }
}

/* Every PHI2 rise: an input sample after a CB1 rise. */
static void sr_rise(Via *s) {
    LogicValue v;

    if (!s->sr_sample_pending) {
        return;
    }
    s->sr_sample_pending = 0;
    v = pin_level(s, s->pins->p.cb2);
    if (s->cb2_seen && sim_now(s->sim) - s->cb2_at < s->t[T_TSR2]) v = LOGIC_X;
    s->sr_sample = v;
    s->sr_shift_pending = 1;
}

/* ---- Cycle hooks -------------------------------------------------------------------- */

/* Every PHI2 fall: read side effects, the CA2 read strobe, the timers. */
static void via_cycle(void *model) {
    Via *s = model;
    int side;

    if (s->reset != LOGIC_1) {
        s->side[0].read_or = s->side[1].read_or = 0;
        s->pending_clear = s->pending_x = 0;
        s->sr_access = 0;
        return;
    }
    for (side = 0; side < 2; side++) {
        ViaSide *p = &s->side[side];

        if (p->read_or) {
            s->pending_clear |= or_access_flags(s, side);
        }
    }
    flag_clear(s, s->pending_clear);
    flag_maybe_clear(s, (uint8_t)(s->pending_x & ~s->pending_clear));

    /* CA2: the read handshake and pulse. */
    {
        ViaSide *a = &s->side[0];
        int mode = c2_mode(s, 0);

        if (mode == C2_HANDSHAKE || mode == C2_PULSE) {
            if (a->read_or) {
                set_c2_out(s, 0, LOGIC_0, s->t[T_TCA2]);
                a->strobe_read = 1;
                a->pulse_fall = mode == C2_PULSE;
            } else if (a->pulse_fall) {
                set_c2_out(s, 0, LOGIC_1, s->t[T_TRS1]);
                a->pulse_fall = 0;
            }
        }
    }
    s->side[0].read_or = s->side[1].read_or = 0;
    s->pending_clear = s->pending_x = 0;

    t1_fall(s);
    t2_fall(s);
    sr_fall(s);
    if (s->sr_access) sr_start(s);
    update_irq(s);
}

/* Every PHI2 rise: the write strobes, the time-outs, PB6 sampling. */
static void via_rise(Via *s) {
    SimTime now = sim_now(s->sim);
    int side;

    if (s->last_rise > 0 || s->period > 0) s->period = now - s->last_rise;
    s->last_rise = now;
    for (side = 0; side < 2; side++) {
        ViaSide *p = &s->side[side];
        int mode = c2_mode(s, side);

        if (mode == C2_HANDSHAKE || mode == C2_PULSE) {
            if (p->arm) {
                set_c2_out(s, side, LOGIC_0, s->t[T_TWHS]);
                p->strobe_read = 0;
                p->pulse_rise = mode == C2_PULSE;
            } else if (p->pulse_rise) {
                set_c2_out(s, side, LOGIC_1, s->t[T_TRS3]);
                p->pulse_rise = 0;
            }
        }
        p->arm = 0;
    }
    t1_rise(s);
    t2_rise(s);
    sr_rise(s);
}

static const Bus65Ops VIA_BUS = {via_read, via_write, via_cycle, via_read_unknown};

/* ---- Control line inputs ------------------------------------------------------- */

static void latch_port(Via *s, int side) {
    ViaSide *p = &s->side[side];
    SimTime now = sim_now(s->sim);
    int i;

    p->latch = p->latch_k = 0;
    for (i = 0; i < PIO_PORT_BITS; i++) {
        LogicValue v = pin_level(s, port_pins(s, side)[i]);

        if (recent(p, i, now, s->t[T_TIL])) v = LOGIC_X;
        set_bit(&p->latch, &p->latch_k, i, v);
    }
    p->latched = 1;
    p->latch_at = now;
}

static void c1_edge(Via *s, int side, LogicValue old_v, LogicValue new_v) {
    ViaSide *p = &s->side[side];
    LogicValue edge = bitv(s->pcr, s->pcr_k, side == 0 ? 0 : 4);
    uint8_t flag = side == 0 ? F_CA1 : F_CB1;
    int mode = c2_mode(s, side);

    if (old_v == LOGIC_X || new_v == LOGIC_X || edge == LOGIC_X) {
        flag_set(s, flag, 1);
        if (acr_bit(s, side) != LOGIC_0) {
            p->latch_k = 0;
            p->latched = 1;
        }
        if (mode == C2_HANDSHAKE && p->c2_out != LOGIC_1) set_c2_out(s, side, LOGIC_X, 0);
        update_irq(s);
        return;
    }
    if ((new_v == LOGIC_1) != (edge == LOGIC_1)) {
        return;
    }
    flag_set(s, flag, 0);
    if (acr_bit(s, side) == LOGIC_1) {
        latch_port(s, side);
    } else if (acr_bit(s, side) == LOGIC_X) {
        latch_port(s, side);
        p->latch_k = 0;
    }
    if (mode == C2_HANDSHAKE) {
        set_c2_out(s, side, LOGIC_1, p->strobe_read ? s->t[T_TRS2] : s->t[T_TRS4]);
    }
    update_irq(s);
}

static void c2_edge(Via *s, int side, LogicValue old_v, LogicValue new_v) {
    int mode = c2_mode(s, side);
    uint8_t flag = side == 0 ? F_CA2 : F_CB2;
    int rising;

    if (side == 1 && sr_mode(s) != SR_OFF) {
        return;   /* the serial line, not a PCR input */
    }

    if (mode == C2_UNKNOWN) {
        flag_set(s, flag, 1);
        update_irq(s);
        return;
    }
    if (mode >= C2_HANDSHAKE) {
        return;   /* an output */
    }
    rising = mode == C2_IN_POS || mode == C2_IND_POS;
    if (old_v == LOGIC_X || new_v == LOGIC_X) {
        flag_set(s, flag, 1);
    } else if ((new_v == LOGIC_1) == rising) {
        flag_set(s, flag, 0);
    } else {
        return;
    }
    update_irq(s);
}

/* ---- Engine hooks ------------------------------------------------------------------ */

static void *via_create(const Part *part, const void *data, const void *pins,
                        const PioGrade *grade) {
    Via *s = calloc(1, sizeof(*s));
    Bus65Timing timing;
    size_t i;

    (void)data;
    if (s == NULL) {
        return NULL;
    }
    s->base.model = &VIA6522_MODEL;
    s->pins = pins;
    s->t = grade->t;
    memset(&timing, 0, sizeof timing);
    timing.tacr = grade->t[T_TACR];
    timing.tcar = grade->t[T_TCAR];
    timing.tcdr = grade->t[T_TCDR];
    timing.thr = grade->t[T_THR];
    timing.tacw = grade->t[T_TACW];
    timing.tcaw = grade->t[T_TCAW];
    timing.tdcw = grade->t[T_TDCW];
    timing.thw = grade->t[T_THW];
    timing.tcyc = grade->t[T_TCYC];
    timing.tpwh = grade->t[T_TPWH];
    timing.tpwl = grade->t[T_TPWL];
    bus65_init(&s->bus, &s->pins->p.bus, &timing, &VIA_BUS, s);
    s->bus.bus_hold = s->pins->device->bus_hold;
    for (i = 0; i < MAX_PINS; i++) s->held[i] = LOGIC_X;
    (void)part;
    return s;
}

static void via_destroy(void *state) {
    free(state);
}

static LogicValue reset_value(const Via *s, LogicValue v) {
    if (v == LOGIC_Z) {
        return s->pins->device->bus_hold ? s->held[s->pins->p.reset] : LOGIC_X;
    }
    return v;
}

static void via_start(Sim *sim, SimInstance *instance, void *state) {
    Via *s = state;
    int side;

    s->sim = sim;
    s->instance = instance;
    bus65_start(&s->bus, sim, instance);
    s->phi2 = sim_read_pin(sim, instance, s->pins->p.bus.phi2);
    s->reset = reset_value(s, sim_read_pin(sim, instance, s->pins->p.reset));
    for (side = 0; side < 2; side++) {
        s->side[side].c1 = pin_level(s, c1_pin(s, side));
        s->side[side].c2 = pin_level(s, c2_pin(s, side));
    }
    s->pb6_rise = LOGIC_X;
    reset_registers(s, 0, 1);
    if (s->reset == LOGIC_0) reset_registers(s, 1, 0);
}

static int port_bit(const Via *s, int side, size_t pin) {
    const size_t *pins = port_pins(s, side);
    int i;

    for (i = 0; i < PIO_PORT_BITS; i++) {
        if (pins[i] == pin) return i;
    }
    return -1;
}

static void via_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                            LogicValue old_value, LogicValue new_value) {
    Via *s = state;
    const PioPins *p = &s->pins->p;
    int side;
    LogicValue v;

    s->sim = sim;
    s->instance = instance;
    if (pin < MAX_PINS && (new_value == LOGIC_0 || new_value == LOGIC_1)) {
        s->held[pin] = new_value;
    }
    if (pin == p->reset) {
        v = reset_value(s, new_value);
        if (v == s->reset) return;
        s->reset = v;
        if (v == LOGIC_0) reset_registers(s, 1, 0);
        else if (v == LOGIC_X) reset_registers(s, 0, 1);
        return;
    }
    if (bus65_pin_changed(&s->bus, sim, instance, pin, old_value, new_value)) {
        if (pin == p->bus.phi2) {
            LogicValue phi2 = new_value == LOGIC_Z ? LOGIC_X : new_value;

            if (s->phi2 == LOGIC_0 && phi2 == LOGIC_1 && s->reset == LOGIC_1) via_rise(s);
            s->phi2 = phi2;
        }
        return;
    }
    v = pin_level(s, pin);
    for (side = 0; side < 2; side++) {
        ViaSide *ps = &s->side[side];
        int bit = port_bit(s, side, pin);

        if (bit >= 0) {
            SimTime now = sim_now(sim);

            ps->port_at[bit] = now;
            ps->port_seen[bit] = 1;
            /* Latch hold: a change within tpdh of the latching edge. */
            if (ps->latched && now - ps->latch_at < s->t[T_TPDH]) {
                set_bit(&ps->latch, &ps->latch_k, bit, LOGIC_X);
            }
            if (side == 1 && bit == 6 && s->reset == LOGIC_1) pb6_edge(s, v);
            return;
        }
        if (pin == c1_pin(s, side)) {
            LogicValue old_v = ps->c1;

            ps->c1 = v;
            if (s->reset == LOGIC_1 && old_v != v) {
                c1_edge(s, side, old_v, v);
                if (side == 1) {
                    int m = sr_mode(s);

                    if ((m == SR_IN_EXT || m == SR_OUT_EXT) &&
                        (v == LOGIC_X || old_v == LOGIC_X)) {
                        s->sr = s->sr_k = 0;        /* an unknown clock edge */
                        s->sr_count = -1;
                    } else if ((m == SR_IN_EXT && v == LOGIC_1) ||
                               (m == SR_OUT_EXT && v == LOGIC_0)) {
                        s->sr_ext = 1;
                        s->sr_ext_at = sim_now(sim);
                    }
                }
            }
            return;
        }
        if (pin == c2_pin(s, side)) {
            LogicValue old_v = ps->c2;

            if (side == 1) {
                s->cb2_at = sim_now(sim);
                s->cb2_seen = 1;
            }

            ps->c2 = v;
            if (s->reset == LOGIC_1 && old_v != v) c2_edge(s, side, old_v, v);
            return;
        }
    }
}

static void via_timer(Sim *sim, SimInstance *instance, void *state, uint32_t timer_id) {
    Via *s = state;

    s->sim = sim;
    s->instance = instance;
    bus65_timer(&s->bus, sim, instance, timer_id);
}

/* ---- Loader --------------------------------------------------------------------- */

static void *via_load_device(KindContext *ctx, const cJSON *json) {
    ViaDevice *d = kind_alloc(ctx, sizeof(*d));

    if (d == NULL || pio_load_names(ctx, json, &d->names, 1, 1, 4) != 0 ||
        kind_string(ctx, json, "irq", &d->irq) != 0 ||
        kind_bool(ctx, json, "irqOpenDrain", &d->irq_open_drain) != 0 ||
        kind_bool(ctx, json, "busHold", &d->bus_hold) != 0) {
        return NULL;
    }
    return d;
}

static void *via_bind(KindContext *ctx, const void *data, const PartPin *pins, size_t pin_count) {
    const ViaDevice *d = data;
    ViaPins *out = kind_alloc(ctx, sizeof(*out));

    if (out == NULL) {
        return NULL;
    }
    if (pin_count > MAX_PINS) {
        kind_error(ctx, "a via6522 has at most %d pins", MAX_PINS);
        return NULL;
    }
    if (pio_bind_names(ctx, &d->names, pins, pin_count, &out->p) != 0 ||
        pio_bind_pin(ctx, pins, pin_count, "irq", d->irq, &out->irq) != 0) {
        return NULL;
    }
    out->device = d;
    return out;
}

static int via_pin_open_drain(const void *data, const void *pins, size_t pin) {
    const ViaDevice *d = data;
    const ViaPins *p = pins;

    return d->irq_open_drain && pin == p->irq;
}

const PioModel VIA6522_MODEL = {
    "via6522", KEYS, TIMING, 3,
    via_load_device, via_bind, via_pin_open_drain,
    via_create, via_destroy, via_start, via_pin_changed, via_timer
};
