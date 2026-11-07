/*
 * pia6521: the WDC W65C21 peripheral interface adapter (plan-14-B;
 * datasheets/pio/w65c21.pdf).
 *
 * Two sides (A, B), each with a control register (CR), a data direction
 * register (DDR) and an output register (OR). RS1 picks the side; RS0 = 1
 * is CR, RS0 = 0 is OR when CR bit 2 = 1 and DDR when it is 0 (Table 2).
 * CR bits 0-5 are written; bits 6 (IRQx2) and 7 (IRQx1) are the flags,
 * set by CA2/CB2 and CA1/CB1 edges and cleared by a read of OR.
 *
 * Ports (p.16): port A reads its pins; an output 1 is its resistive
 * pull-ups (weak), an output 0 drives strong, and an input is pulled up
 * (weak 1). Port B outputs drive strong and read back from ORB; its inputs
 * float. Outputs change tcpw after the write's PHI2 fall. A pin that
 * changed less than tpcr before a read's PHI2 rise reads X.
 *
 * Control lines (Table 3, p.18): CA1/CB1 set bit 7 on the active edge (CR
 * bit 1); CA2/CB2 as inputs (CR bit 5 = 0) set bit 6 on the edge CR bit 4
 * picks. A pulse that ends within pwi of its active edge sets the flag X.
 * CA2/CB2 as outputs: handshake (CA2 low after a read of ORA, high on the
 * next active CA1 edge; CB2 low at the PHI2 rise after a write of ORB,
 * high on the next active CB1 edge), pulse (low for one cycle), or the
 * level of CR bit 3. Entering handshake or pulse mode sets the line high.
 *
 * IRQx (open drain) is low while (bit 7 and bit 0) or (bit 6 and bit 3 and
 * not bit 5): trs3 after the edge that sets a flag, released tir after the
 * PHI2 fall of the read that clears it.
 *
 * RESB low clears every register; RESB X makes them X; RESB Z reads high.
 * Registers power up X.
 */

#include <stdlib.h>
#include <string.h>

#include "pio.h"

enum {
    T_TCYC, T_TC, T_TACR, T_TCAR, T_TPCR, T_TCDR, T_THR, T_TACW, T_TCAW, T_TDCW, T_THW, T_TCPW,
    T_TCA2, T_TCA2H, T_TRS2, T_TCB2, T_TCB2H, T_TDC, T_PWI, T_TRS3, T_TIR, T_COUNT
};

static const char *const TIMING[] = {
    "tcyc", "tc", "tacr", "tcar", "tpcr", "tcdr", "thr", "tacw", "tcaw", "tdcw", "thw", "tcpw",
    "tca2", "tca2h", "trs2", "tcb2", "tcb2h", "tdc", "pwi", "trs3", "tir", NULL
};

static const char *const KEYS[] = {
    "phi2", "rw", "reset", "data", "csHigh", "csLow", "rs", "pa", "pb",
    "ca1", "ca2", "cb1", "cb2", "irqa", "irqb", NULL
};

typedef struct {
    PioNames names;
    const char *irq[2];
} PiaDevice;

typedef struct {
    PioPins p;
    size_t irq[2];
} PiaPins;

/* C2 modes from CR bits 5-3. */
enum { MODE_UNKNOWN = -1, MODE_INPUT = 0, MODE_HANDSHAKE = 4, MODE_PULSE = 5, MODE_LOW = 6,
       MODE_HIGH = 7 };

typedef struct {
    uint8_t cr, cr_k;          /* bits 6-7 are the flags; _k: known bits */
    uint8_t ddr, ddr_k;
    uint8_t out, out_k;        /* OR */
    LogicValue c1, c2;         /* the input levels last seen */
    /* The narrow-pulse check per flag (0: C1/bit 7, 1: C2/bit 6). */
    int open[2];
    SimTime edge_at[2];
    LogicValue prior[2];
    LogicValue c2_out;         /* CA2/CB2 in handshake and pulse modes */
    int pulse;                 /* a pulse is low: ends at the next fall (A) / rise (B) */
    int arm;                   /* B: ORB written; CB2 acts at the next rise */
    int read_or;               /* OR read this cycle */
    int read_unknown;          /* maybe read this cycle */
    SimTime port_at[PIO_PORT_BITS];
    int port_seen[PIO_PORT_BITS];
} PiaSide;

typedef struct {
    PioBase base;
    Bus65 bus;
    const PiaPins *pins;
    const SimTime *t;
    Sim *sim;
    SimInstance *instance;
    PiaSide side[2];
    LogicValue reset;
    LogicValue phi2;
} Pia;

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

static LogicValue and3(LogicValue a, LogicValue b) {
    if (a == LOGIC_0 || b == LOGIC_0) return LOGIC_0;
    if (a == LOGIC_1 && b == LOGIC_1) return LOGIC_1;
    return LOGIC_X;
}

static LogicValue or3(LogicValue a, LogicValue b) {
    if (a == LOGIC_1 || b == LOGIC_1) return LOGIC_1;
    if (a == LOGIC_0 && b == LOGIC_0) return LOGIC_0;
    return LOGIC_X;
}

static LogicValue not3(LogicValue a) {
    return a == LOGIC_0 ? LOGIC_1 : a == LOGIC_1 ? LOGIC_0 : LOGIC_X;
}

static LogicValue level(LogicValue v) {
    return v == LOGIC_Z ? LOGIC_X : v;
}

/* ---- Outputs ----------------------------------------------------------------------- */

static size_t c2_pin(const Pia *s, int side) {
    return side == 0 ? s->pins->p.ca2 : s->pins->p.cb2;
}

static int c2_mode(const PiaSide *p) {
    if ((p->cr_k & 0x38) != 0x38) {
        /* bit 5 known 0 is an input whatever bits 4-3 are */
        if ((p->cr_k & 0x20) && !(p->cr & 0x20)) return MODE_INPUT;
        return MODE_UNKNOWN;
    }
    if (!(p->cr & 0x20)) return MODE_INPUT;
    return 4 + ((p->cr >> 3) & 3);
}

static void drive_irq(Pia *s, int side, SimTime delay) {
    PiaSide *p = &s->side[side];
    LogicValue t1 = and3(bitv(p->cr, p->cr_k, 7), bitv(p->cr, p->cr_k, 0));
    LogicValue t2 = and3(and3(bitv(p->cr, p->cr_k, 6), bitv(p->cr, p->cr_k, 3)),
                         not3(bitv(p->cr, p->cr_k, 5)));
    LogicValue on = or3(t1, t2);
    LogicValue v = on == LOGIC_1 ? LOGIC_0 : on == LOGIC_0 ? LOGIC_Z : LOGIC_X;

    sim_schedule_inertial_drive(s->sim, s->instance, s->pins->irq[side], v, DRIVE_STRONG, delay);
}

static void drive_ports(Pia *s, int side, SimTime delay) {
    const PiaSide *p = &s->side[side];
    const size_t *pins = side == 0 ? s->pins->p.pa : s->pins->p.pb;
    int i;

    for (i = 0; i < PIO_PORT_BITS; i++) {
        LogicValue dir = bitv(p->ddr, p->ddr_k, i);
        LogicValue v = LOGIC_X;
        DriveStrength strength = DRIVE_STRONG;

        if (dir == LOGIC_1) {
            v = bitv(p->out, p->out_k, i);
            if (side == 0 && v == LOGIC_1) strength = DRIVE_WEAK;   /* resistive pull-ups */
        } else if (dir == LOGIC_0) {
            v = side == 0 ? LOGIC_1 : LOGIC_Z;
            strength = side == 0 ? DRIVE_WEAK : DRIVE_STRONG;
        }
        sim_schedule_inertial_drive(s->sim, s->instance, pins[i], v, strength, delay);
    }
}

static void drive_c2(Pia *s, int side, SimTime delay) {
    const PiaSide *p = &s->side[side];
    int mode = c2_mode(p);
    LogicValue v = LOGIC_X;
    DriveStrength strength = DRIVE_STRONG;

    if (mode == MODE_INPUT) {
        v = side == 0 ? LOGIC_1 : LOGIC_Z;
        strength = side == 0 ? DRIVE_WEAK : DRIVE_STRONG;
    } else if (mode == MODE_LOW || mode == MODE_HIGH) {
        v = mode == MODE_HIGH ? LOGIC_1 : LOGIC_0;
    } else if (mode == MODE_HANDSHAKE || mode == MODE_PULSE) {
        v = p->c2_out;
    }
    sim_schedule_inertial_drive(s->sim, s->instance, c2_pin(s, side), v, strength, delay);
}

static void set_c2_out(Pia *s, int side, LogicValue v, SimTime delay) {
    s->side[side].c2_out = v;
    drive_c2(s, side, delay);
}

/* ---- Reset ----------------------------------------------------------------------- */

static void all_registers(Pia *s, int known) {
    int side;

    for (side = 0; side < 2; side++) {
        PiaSide *p = &s->side[side];

        p->cr = p->ddr = p->out = 0;
        p->cr_k = p->ddr_k = p->out_k = known ? 0xFF : 0;
        p->open[0] = p->open[1] = 0;
        p->c2_out = known ? LOGIC_1 : LOGIC_X;
        p->pulse = p->arm = p->read_or = p->read_unknown = 0;
        drive_ports(s, side, 0);
        drive_c2(s, side, 0);
        drive_irq(s, side, 0);
    }
}

/* ---- Bus callbacks ----------------------------------------------------------------- */

static int recent(const Pia *s, const PiaSide *p, int bit, SimTime now) {
    return p->port_seen[bit] && now - p->port_at[bit] < s->t[T_TPCR];
}

static void read_port(Pia *s, int side, uint8_t *value, uint8_t *known) {
    const PiaSide *p = &s->side[side];
    const size_t *pins = side == 0 ? s->pins->p.pa : s->pins->p.pb;
    SimTime now = sim_now(s->sim);
    int i;

    *value = 0;
    *known = 0;
    for (i = 0; i < PIO_PORT_BITS; i++) {
        LogicValue v;

        if (side == 1) {
            LogicValue dir = bitv(p->ddr, p->ddr_k, i);

            if (dir == LOGIC_1) {
                set_bit(value, known, i, bitv(p->out, p->out_k, i));
                continue;
            }
            if (dir == LOGIC_X) {
                continue;
            }
        }
        v = sim_read_pin(s->sim, s->instance, pins[i]);
        if (v == LOGIC_Z) v = side == 0 ? LOGIC_1 : LOGIC_X;
        if (recent(s, p, i, now)) v = LOGIC_X;
        set_bit(value, known, i, v);
    }
}

static int pia_read(void *model, unsigned reg, uint8_t *value, uint8_t *known) {
    Pia *s = model;
    int side = (int)(reg >> 1 & 1);
    PiaSide *p = &s->side[side];
    LogicValue select;

    if (reg & 1) {
        *value = p->cr;
        *known = p->cr_k;
        return 1;
    }
    select = bitv(p->cr, p->cr_k, 2);
    if (select == LOGIC_X) {
        *value = 0;
        *known = 0;
        p->read_unknown = 1;
    } else if (select == LOGIC_0) {
        *value = p->ddr;
        *known = p->ddr_k;
    } else {
        read_port(s, side, value, known);
        if (s->reset == LOGIC_1) p->read_or = 1;
    }
    return 1;
}

static void pia_read_unknown(void *model) {
    Pia *s = model;
    int side;

    for (side = 0; side < 2; side++) {
        PiaSide *p = &s->side[side];

        /* An OR read is possible unless CR bit 2 is known 0. */
        if (!((p->cr_k & 4) && !(p->cr & 4))) p->read_unknown = 1;
    }
}

/* Writes value/known into a register, keeping a known bit only where it
   was known and equal (the target is one of several). */
static void merge(uint8_t *v, uint8_t *k, uint8_t value, uint8_t known) {
    *k = (uint8_t)(*k & known & ~(*v ^ value));
    *v &= *k;
}

static void enter_mode(Pia *s, int side, int old_mode) {
    PiaSide *p = &s->side[side];
    int mode = c2_mode(p);

    if ((mode == MODE_HANDSHAKE || mode == MODE_PULSE) &&
        old_mode != MODE_HANDSHAKE && old_mode != MODE_PULSE) {
        p->c2_out = LOGIC_1;
        p->pulse = p->arm = 0;
    } else if (mode != MODE_HANDSHAKE && mode != MODE_PULSE) {
        p->pulse = p->arm = 0;
        if (mode == MODE_UNKNOWN) p->c2_out = LOGIC_X;
    }
    /* CR bit 6 cannot be set while bit 5 = 1. */
    if (bitv(p->cr, p->cr_k, 5) == LOGIC_1) {
        set_bit(&p->cr, &p->cr_k, 6, LOGIC_0);
    } else if (bitv(p->cr, p->cr_k, 5) == LOGIC_X && bitv(p->cr, p->cr_k, 6) != LOGIC_0) {
        set_bit(&p->cr, &p->cr_k, 6, LOGIC_X);
    }
    drive_c2(s, side, 0);
}

static void pia_write(void *model, unsigned reg, uint8_t value, uint8_t known) {
    Pia *s = model;
    int side = (int)(reg >> 1 & 1);
    PiaSide *p = &s->side[side];
    LogicValue select;

    if (s->reset != LOGIC_1) {
        return;   /* held in reset (or X: already unknown) */
    }
    if (reg & 1) {
        int old_mode = c2_mode(p);

        p->cr = (uint8_t)((p->cr & 0xC0) | (value & 0x3F));
        p->cr_k = (uint8_t)((p->cr_k & 0xC0) | (known & 0x3F));
        p->cr &= (uint8_t)(p->cr_k | 0xC0);
        enter_mode(s, side, old_mode);
        drive_irq(s, side, 0);
        return;
    }
    select = bitv(p->cr, p->cr_k, 2);
    if (select == LOGIC_X || known == 0) {
        /* Either register (an unknown write or PHI2 X: both). */
        merge(&p->ddr, &p->ddr_k, value, known);
        merge(&p->out, &p->out_k, value, known);
        if (side == 1 && (c2_mode(p) == MODE_HANDSHAKE || c2_mode(p) == MODE_PULSE)) {
            set_c2_out(s, side, LOGIC_X, 0);
        }
    } else if (select == LOGIC_0) {
        p->ddr = value & known;
        p->ddr_k = known;
    } else {
        p->out = value & known;
        p->out_k = known;
        if (side == 1) p->arm = 1;
    }
    drive_ports(s, side, s->t[T_TCPW]);
}

/* Every PHI2 fall: read side effects (flag clears, the CA2 strobe). */
static void pia_cycle(void *model) {
    Pia *s = model;
    int side;

    for (side = 0; side < 2; side++) {
        PiaSide *p = &s->side[side];
        int mode = c2_mode(p);

        if (p->read_or || p->read_unknown) {
            LogicValue v = p->read_or ? LOGIC_0 : LOGIC_X;

            if (p->read_unknown && !p->read_or) {
                if (bitv(p->cr, p->cr_k, 6) != LOGIC_0) set_bit(&p->cr, &p->cr_k, 6, v);
                if (bitv(p->cr, p->cr_k, 7) != LOGIC_0) set_bit(&p->cr, &p->cr_k, 7, v);
            } else {
                set_bit(&p->cr, &p->cr_k, 6, v);
                set_bit(&p->cr, &p->cr_k, 7, v);
            }
            p->open[0] = p->open[1] = 0;
            drive_irq(s, side, s->t[T_TIR]);
        }
        if (side == 0 && (mode == MODE_HANDSHAKE || mode == MODE_PULSE)) {
            if (p->read_or || p->read_unknown) {
                set_c2_out(s, side, p->read_or ? LOGIC_0 : LOGIC_X, s->t[T_TCA2]);
                p->pulse = mode == MODE_PULSE;
            } else if (p->pulse) {
                set_c2_out(s, side, LOGIC_1, s->t[T_TCA2H]);
                p->pulse = 0;
            }
        }
        p->read_or = p->read_unknown = 0;
    }
}

/* Every PHI2 rise: the CB2 strobe after a write of ORB. */
static void pia_rise(Pia *s) {
    PiaSide *p = &s->side[1];
    int mode = c2_mode(p);

    if (mode == MODE_HANDSHAKE || mode == MODE_PULSE) {
        if (p->arm) {
            set_c2_out(s, 1, LOGIC_0, s->t[T_TCB2]);
            p->pulse = mode == MODE_PULSE;
        } else if (p->pulse) {
            set_c2_out(s, 1, LOGIC_1, s->t[T_TCB2H]);
            p->pulse = 0;
        }
    }
    p->arm = 0;
}

static const Bus65Ops PIA_BUS = {pia_read, pia_write, pia_cycle, pia_read_unknown};

/* ---- Control line inputs ------------------------------------------------------- */

/* An edge on C1 (which = 0, flag bit 7, edge bit 1) or a C2 input (which =
   1, flag bit 6, edge bit 4). */
static void control_edge(Pia *s, int side, int which, LogicValue old_v, LogicValue new_v) {
    PiaSide *p = &s->side[side];
    int flag = which == 0 ? 7 : 6;
    LogicValue edge_sel = bitv(p->cr, p->cr_k, which == 0 ? 1 : 4);
    SimTime now = sim_now(s->sim);
    LogicValue was = bitv(p->cr, p->cr_k, flag);

    if (old_v == LOGIC_X || new_v == LOGIC_X || edge_sel == LOGIC_X) {
        if (was != LOGIC_1) {
            set_bit(&p->cr, &p->cr_k, flag, LOGIC_X);
            drive_irq(s, side, s->t[T_TRS3]);
        }
        if (which == 0 && c2_mode(p) == MODE_HANDSHAKE && p->c2_out != LOGIC_1) {
            set_c2_out(s, side, LOGIC_X, s->t[T_TRS2]);
        }
        p->open[which] = 0;
        return;
    }
    if ((new_v == LOGIC_1) == (edge_sel == LOGIC_1)) {
        /* The active edge. */
        p->prior[which] = was;
        p->open[which] = 1;
        p->edge_at[which] = now;
        set_bit(&p->cr, &p->cr_k, flag, LOGIC_1);
        drive_irq(s, side, s->t[T_TRS3]);
        if (which == 0 && c2_mode(p) == MODE_HANDSHAKE) {
            set_c2_out(s, side, LOGIC_1, s->t[T_TRS2]);
        }
    } else if (p->open[which]) {
        /* The pulse ends: narrower than pwi, the chip may have missed it. */
        if (now - p->edge_at[which] < s->t[T_PWI] && p->prior[which] != LOGIC_1) {
            set_bit(&p->cr, &p->cr_k, flag, LOGIC_X);
            drive_irq(s, side, 0);
        }
        p->open[which] = 0;
    }
}

/* ---- Engine hooks ----------------------------------------------------------------- */

static void *pia_create(const Part *part, const void *data, const void *pins,
                        const PioGrade *grade) {
    Pia *s = calloc(1, sizeof(*s));
    Bus65Timing timing;

    (void)part;
    (void)data;
    if (s == NULL) {
        return NULL;
    }
    s->base.model = &PIA6521_MODEL;
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
    timing.tpwh = grade->t[T_TC];
    timing.tpwl = grade->t[T_TC];
    bus65_init(&s->bus, &s->pins->p.bus, &timing, &PIA_BUS, s);
    return s;
}

static void pia_destroy(void *state) {
    free(state);
}

static LogicValue reset_level(const Pia *s) {
    LogicValue v = sim_read_pin(s->sim, s->instance, s->pins->p.reset);

    return v == LOGIC_Z ? LOGIC_1 : v;
}

static void pia_start(Sim *sim, SimInstance *instance, void *state) {
    Pia *s = state;
    int side;

    s->sim = sim;
    s->instance = instance;
    bus65_start(&s->bus, sim, instance);
    s->phi2 = sim_read_pin(sim, instance, s->pins->p.bus.phi2);
    s->reset = reset_level(s);
    for (side = 0; side < 2; side++) {
        s->side[side].c1 = level(sim_read_pin(sim, instance, side == 0 ? s->pins->p.ca1
                                                                        : s->pins->p.cb1));
        s->side[side].c2 = level(sim_read_pin(sim, instance, c2_pin(s, side)));
    }
    all_registers(s, s->reset == LOGIC_0);
}

static int port_bit(const Pia *s, int side, size_t pin) {
    const size_t *pins = side == 0 ? s->pins->p.pa : s->pins->p.pb;
    int i;

    for (i = 0; i < PIO_PORT_BITS; i++) {
        if (pins[i] == pin) return i;
    }
    return -1;
}

static void pia_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                            LogicValue old_value, LogicValue new_value) {
    Pia *s = state;
    const PioPins *p = &s->pins->p;
    int side;

    s->sim = sim;
    s->instance = instance;
    if (pin == p->reset) {
        LogicValue v = new_value == LOGIC_Z ? LOGIC_1 : new_value;

        s->reset = v;
        if (v == LOGIC_0) all_registers(s, 1);
        else if (v == LOGIC_X) all_registers(s, 0);
        return;
    }
    if (bus65_pin_changed(&s->bus, sim, instance, pin, old_value, new_value)) {
        LogicValue phi2 = new_value == LOGIC_Z ? LOGIC_X : new_value;

        if (pin == p->bus.phi2) {
            if (s->phi2 == LOGIC_0 && phi2 == LOGIC_1 && s->reset == LOGIC_1) pia_rise(s);
            s->phi2 = phi2;
        }
        return;
    }
    for (side = 0; side < 2; side++) {
        PiaSide *ps = &s->side[side];
        int bit = port_bit(s, side, pin);
        LogicValue v = level(new_value);

        if (bit >= 0) {
            ps->port_at[bit] = sim_now(sim);
            ps->port_seen[bit] = 1;
            return;
        }
        if (pin == (side == 0 ? p->ca1 : p->cb1)) {
            LogicValue old_v = ps->c1;

            ps->c1 = v;
            if (s->reset == LOGIC_1 && old_v != v) control_edge(s, side, 0, old_v, v);
            return;
        }
        if (pin == c2_pin(s, side)) {
            LogicValue old_v = ps->c2;
            int mode = c2_mode(ps);

            /* Port A's CA2 input is pulled up: Z reads 1. */
            if (side == 0 && new_value == LOGIC_Z) v = LOGIC_1;
            ps->c2 = v;
            if (s->reset != LOGIC_1 || old_v == v) return;
            if (mode == MODE_INPUT) {
                control_edge(s, side, 1, old_v, v);
            } else if (mode == MODE_UNKNOWN && bitv(ps->cr, ps->cr_k, 6) != LOGIC_1) {
                set_bit(&ps->cr, &ps->cr_k, 6, LOGIC_X);
                drive_irq(s, side, s->t[T_TRS3]);
            }
            return;
        }
    }
}

static void pia_timer(Sim *sim, SimInstance *instance, void *state, uint32_t timer_id) {
    Pia *s = state;

    s->sim = sim;
    s->instance = instance;
    bus65_timer(&s->bus, sim, instance, timer_id);
}

/* ---- Loader --------------------------------------------------------------------- */

static void *pia_load_device(KindContext *ctx, const cJSON *json) {
    PiaDevice *d = kind_alloc(ctx, sizeof(*d));

    if (d == NULL || pio_load_names(ctx, json, &d->names, 2, 1, 2) != 0 ||
        kind_string(ctx, json, "irqa", &d->irq[0]) != 0 ||
        kind_string(ctx, json, "irqb", &d->irq[1]) != 0) {
        return NULL;
    }
    return d;
}

static void *pia_bind(KindContext *ctx, const void *data, const PartPin *pins, size_t pin_count) {
    const PiaDevice *d = data;
    PiaPins *out = kind_alloc(ctx, sizeof(*out));

    if (out == NULL || pio_bind_names(ctx, &d->names, pins, pin_count, &out->p) != 0 ||
        pio_bind_pin(ctx, pins, pin_count, "irqa", d->irq[0], &out->irq[0]) != 0 ||
        pio_bind_pin(ctx, pins, pin_count, "irqb", d->irq[1], &out->irq[1]) != 0) {
        return NULL;
    }
    return out;
}

static int pia_pin_open_drain(const void *data, const void *pins, size_t pin) {
    const PiaPins *p = pins;

    (void)data;
    return pin == p->irq[0] || pin == p->irq[1];
}

const PioModel PIA6521_MODEL = {
    "pia6521", KEYS, TIMING, 2,
    pia_load_device, pia_bind, pia_pin_open_drain,
    pia_create, pia_destroy, pia_start, pia_pin_changed, pia_timer
};
