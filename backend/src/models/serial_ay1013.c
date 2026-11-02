/*
 * ay1013: the pin-parallel UARTs (plan-12-H section 3.1): AY-5-1013A and
 * AY-3-1015D, COM2017, COM1863/COM8018, HD-6402 and the CDP1854A, whose
 * mode 1 is a CDP1800 bus UART on the same pins (`"cpuMode"`).
 * Facts: plan-12-data/specs/classic-a.md, decisions.md section classic-a.
 *
 * Mode 0 (every device): no register bus. The control word (NB1, NB2, TSB,
 * NP, EPS) is a level latch, transparent while the control strobe is high,
 * held when it falls. DS low loads TD into the transmitter holding register
 * (taken on DS's rising edge); RDE low drives RD, SWE low drives the status
 * pins (PE FE OR DAV TBMT; EOC and SO are always driven); RDAV low clears
 * DAV. XR high resets. TCP and RCP are 16x clocks (32x on COM8018 while
 * HIACC is high); the transmitter shifts on TCP's falling edge and the
 * receiver counts RCP's falling edges (decisions.md: TCP falling edge;
 * start-bit centre at clock 8).
 *
 * Mode 1 (CDP1854A, MODE high): selected = CS1 high, CS2 low, CS3 high.
 * RSEL and RD/WR pick THR / RHR / control / status (Table III); a write is
 * taken on TPB's falling edge, a read drives the R BUS for the whole
 * selected read, and TPB in a read does its side effects (DA reset,
 * interrupt clears, Table I). INT, DA and THRE are active low; FE and PE/OE
 * are always driven (decisions.md); RTS, CTS, PSI and ES per p4-p7.
 *
 * Signal rules (plan-12-A section 3.5): power-on is the reset state;
 * registers the reset does not clear (the control word, the receive holding
 * register on most devices) power on X. Inputs at Z read X, except the reset
 * pin and the mode-1 chip selects (inactive).
 */

#include <stdlib.h>
#include <string.h>

#include "serial_util.h"

enum { T_PD, T_DS, T_EN, T_DIS, T_DAV, T_RDAV, T_ACC, T_INT };
static const char *const TIMING[] = {
    "tpd", "tds", "ten", "tdis", "tdav", "trdav", "tacc", "tint", NULL
};

static const char *const KEYS[] = {
    "transmitData", "receiveData", "receiveEnable", "statusEnable", "status", "control",
    "dataStrobe", "resetDav", "serialOut", "serialIn", "clocks", "reset", "resetClearsData",
    "resetClearsControl", "fiveBitStops", "cpuMode", NULL
};
static const char *const PART_KEYS[] = {"fiveBitStops", "resetClearsData", NULL};

enum { ST_DAV, ST_TBMT, ST_EOC, ST_OR, ST_FE, ST_PE, ST_COUNT };
static const char *const STATUS_KEYS[] = {"dav", "tbmt", "eoc", "or", "fe", "pe", NULL};

enum { CPU_MODE, CPU_CS1, CPU_CS2, CPU_CS3, CPU_RSEL, CPU_RW, CPU_TPB, CPU_CTS, CPU_PSI, CPU_ES,
       CPU_COUNT };
static const char *const CPU_KEYS[] = {
    "modePin", "select1", "select2", "select3", "registerSelect", "readWrite", "tpb", "cts",
    "psi", "es", NULL
};

/* Control word bits (mode 0 latch order). */
enum { C_NB1, C_NB2, C_TSB, C_NP, C_EPS, C_COUNT };

/* ---- Device and pins -------------------------------------------------------------- */

typedef struct {
    const char *td[8], *rd[8];
    const char *rde, *swe, *ds, *rdav, *so, *si, *tcp, *rcp, *hiacc, *reset;
    const char *status[ST_COUNT];
    const char *strobe, *control[C_COUNT];
    const char *cpu[CPU_COUNT];
    int has_cpu;
    int reset_clears_data;
    int reset_clears_control;
    unsigned five_stop_halves;    /* 3 (1.5 stops) or 4 (2) */
} AyDevice;

typedef struct {
    size_t td[8], rd[8];
    size_t rde, swe, ds, rdav, so, si, tcp, rcp, hiacc, reset, strobe;
    size_t status[ST_COUNT];
    size_t control[C_COUNT];
    size_t cpu[CPU_COUNT];
} AyPins;

typedef struct {
    int five_stop_halves;         /* 0 = the device's */
    int reset_clears_data;        /* -1 = the device's */
} AyOptions;

/* ---- State -------------------------------------------------------------------------- */

typedef struct {
    SerialBase base;
    const AyDevice *dev;
    const AyPins *pins;
    const SerialGrade *grade;
    Sim *sim;
    SimInstance *instance;
    SuOutputs o;
    SerialLine line;
    SerialFormat format;
    unsigned five_halves;
    int clears_data;

    LogicValue mode;              /* 0, 1 (CDP1854A mode 1) or X */
    int in_reset;
    int unknown;                  /* the reset pin is X */
    LogicValue tcp_level, rcp_level, ds_level, strobe_level, psi_level, cts_level, tpb_level;

    SuReg control;                /* mode 0: C_* bits; mode 1: the control register */
    SuReg thr;
    LogicValue thr_full;          /* mode 0: not TBMT */
    int ds_hold;                  /* DS is low: TBMT shows 0, the byte is taken on DS rising */
    SuReg rhr;
    LogicValue dav, overrun, fe, pe;
    int dav_wait;                 /* DAV is due (timer) */
    int dav_wait_rdav;            /* COM1863: DAV waits for RDAR high */
    int eoc_pulse;                /* EOC high for one TCP between back-to-back frames */
    LogicValue tsr;               /* the transmitter's output */
    int resync;

    /* Mode 1. */
    LogicValue thre, tsre, psi, da_m1, oe_m1, fe_m1, pe_m1;
    int thre_due;                 /* THRE sets one TCLOCK after the shift register loads */
    int int_da, int_thre, int_done, int_psi, int_cts;
    int break_hold;
    int frame_sent;
    int read_cycle;               /* 0 none, 1 read, 2 read X */
    int write_cycle;              /* 0 none, 1 write, 2 write X */
    unsigned cycle_rsel;          /* 0, 1, 2 = X */
} AyState;

static SimTime t_of(const AyState *s, unsigned i) {
    return s->grade->t[i];
}

static int hiacc_on(const AyState *s) {
    if (s->sim == NULL) {
        return 0;
    }
    return serial_level(s->sim, s->instance, s->pins->hiacc, LOGIC_0) == LOGIC_1;
}

/* ---- Format ------------------------------------------------------------------------ */

static void update_format(AyState *s) {
    SerialFormat f = serial_format_default();
    SuReg c = s->control;
    unsigned nb;
    LogicValue np, eps, tsb;
    unsigned known;

    if (s->mode == LOGIC_1) {
        /* Table IV: 4 WLS2, 3 WLS1, 2 SBS, 1 EPE, 0 PI. */
        nb = (c.v >> 3) & 3u;
        known = ((c.k >> 3) & 3u) == 3u;
        tsb = su_bit(c, 2);
        eps = su_bit(c, 1);
        np = su_bit(c, 0);
    } else {
        nb = (c.v & 1u) | ((c.v >> C_NB2) & 1u) << 1;
        known = (c.k & 3u) == 3u;
        tsb = su_bit(c, C_TSB);
        eps = su_bit(c, C_EPS);
        np = su_bit(c, C_NP);
    }
    f.data_bits = 5 + nb;
    f.stop_halves = tsb == LOGIC_1 ? (f.data_bits == 5 ? s->five_halves : 4) : 2;
    f.parity = np == LOGIC_1 ? SERIAL_PARITY_NONE
             : eps == LOGIC_1 ? SERIAL_PARITY_EVEN : SERIAL_PARITY_ODD;
    f.factor = hiacc_on(s) ? 32 : 16;
    f.format_x = !known || tsb == LOGIC_X || np == LOGIC_X ||
                 (np == LOGIC_0 && eps == LOGIC_X);
    s->format = f;
}

/* ---- Outputs ------------------------------------------------------------------------ */

static void drive_all_x(AyState *s) {
    unsigned i;

    for (i = 0; i < 8; i++) {
        su_drive(&s->o, s->sim, s->instance, s->pins->rd[i], LOGIC_X, 0);
    }
    for (i = 0; i < ST_COUNT; i++) {
        su_drive(&s->o, s->sim, s->instance, s->pins->status[i], LOGIC_X, 0);
    }
    su_drive(&s->o, s->sim, s->instance, s->pins->so, LOGIC_X, 0);
}

static LogicValue rd_enable_level(const AyState *s) {
    return serial_level(s->sim, s->instance, s->pins->rde, LOGIC_X);
}

static void update_mode0(AyState *s, SimTime status_delay) {
    const AyPins *p = s->pins;
    LogicValue swe = serial_level(s->sim, s->instance, p->swe, LOGIC_X);
    LogicValue rde = rd_enable_level(s);
    LogicValue value[ST_COUNT];
    LogicValue eoc;
    unsigned i;

    value[ST_DAV] = s->dav;
    value[ST_TBMT] = s->ds_hold ? LOGIC_0 : su_not(s->thr_full);
    value[ST_OR] = s->overrun;
    value[ST_FE] = s->fe;
    value[ST_PE] = s->pe;
    eoc = su_bool(s->eoc_pulse || !serial_tx_busy(&s->line.tx));
    value[ST_EOC] = s->in_reset ? LOGIC_1 : eoc;

    for (i = 0; i < ST_COUNT; i++) {
        LogicValue v = value[i];
        SimTime delay = status_delay;

        if (i != ST_EOC) {
            if (swe == LOGIC_1) {
                v = LOGIC_Z;
                delay = t_of(s, T_DIS);
            } else if (swe == LOGIC_X) {
                v = LOGIC_X;
            } else if (p->status[i] != SERIAL_NO_PIN && s->o.out[p->status[i]] == LOGIC_Z) {
                delay = t_of(s, T_EN);
            }
        }
        su_drive(&s->o, s->sim, s->instance, p->status[i], v, delay);
    }
    for (i = 0; i < 8; i++) {
        LogicValue v = su_bit(s->rhr, i);
        SimTime delay = 0;

        if (rde == LOGIC_1) {
            v = LOGIC_Z;
            delay = t_of(s, T_DIS);
        } else if (rde == LOGIC_X) {
            v = LOGIC_X;
        } else if (p->rd[i] != SERIAL_NO_PIN && s->o.out[p->rd[i]] == LOGIC_Z) {
            delay = t_of(s, T_EN);
        }
        su_drive(&s->o, s->sim, s->instance, p->rd[i], v, delay);
    }
    su_drive(&s->o, s->sim, s->instance, p->so, s->in_reset ? LOGIC_1 : s->tsr, t_of(s, T_PD));
}

/* Mode 1: the status register (Table II). */
static void m1_status(const AyState *s, uint8_t *v, uint8_t *k) {
    LogicValue es = su_not(serial_level(s->sim, s->instance, s->pins->cpu[CPU_ES], LOGIC_X));
    LogicValue bits[8];
    unsigned i;

    bits[7] = s->thre;
    bits[6] = s->tsre;
    bits[5] = s->psi;
    bits[4] = es;
    bits[3] = s->fe_m1;
    bits[2] = s->pe_m1;
    bits[1] = s->oe_m1;
    bits[0] = s->da_m1;
    *v = 0;
    *k = 0;
    for (i = 0; i < 8; i++) {
        if (bits[i] == LOGIC_1) *v |= (uint8_t)(1u << i);
        if (bits[i] != LOGIC_X) *k |= (uint8_t)(1u << i);
    }
}

static LogicValue m1_int(const AyState *s) {
    LogicValue ie = su_bit(s->control, 5);
    int any = s->int_da || s->int_thre || s->int_done || s->int_psi || s->int_cts;

    return su_and(ie, su_bool(any));
}

static LogicValue m1_rts(const AyState *s) {
    LogicValue tr = su_bit(s->control, 7);
    int busy = s->thr_full == LOGIC_1 || serial_tx_busy(&s->line.tx);

    if (busy) {
        return LOGIC_0;
    }
    return su_not(tr);
}

static LogicValue m1_sdo(const AyState *s) {
    if (s->in_reset) {
        return LOGIC_1;
    }
    if (s->break_hold) {
        return LOGIC_0;
    }
    if (su_bit(s->control, 6) == LOGIC_X) {
        return s->tsr == LOGIC_0 ? LOGIC_0 : LOGIC_X;
    }
    return s->tsr;
}

static void update_mode1(AyState *s, SimTime delay) {
    const AyPins *p = s->pins;
    unsigned i;

    su_drive(&s->o, s->sim, s->instance, p->status[ST_PE], su_not(m1_int(s)), delay);
    su_drive(&s->o, s->sim, s->instance, p->status[ST_FE], s->fe_m1, delay);
    su_drive(&s->o, s->sim, s->instance, p->status[ST_OR], su_or(s->pe_m1, s->oe_m1), delay);
    su_drive(&s->o, s->sim, s->instance, p->status[ST_DAV], su_not(s->da_m1), delay);
    su_drive(&s->o, s->sim, s->instance, p->status[ST_TBMT], su_not(s->thre), delay);
    su_drive(&s->o, s->sim, s->instance, p->status[ST_EOC], m1_rts(s), delay);
    su_drive(&s->o, s->sim, s->instance, p->so, m1_sdo(s), t_of(s, T_PD));
    /* R BUS: driven for the whole selected read. */
    for (i = 0; i < 8; i++) {
        LogicValue v = LOGIC_Z;
        SimTime d = t_of(s, T_DIS);

        if (s->read_cycle == 2) {
            v = LOGIC_X;
            d = t_of(s, T_ACC);
        } else if (s->read_cycle == 1) {
            uint8_t rv;
            uint8_t rk;

            if (s->cycle_rsel == 1) {
                m1_status(s, &rv, &rk);
            } else {
                rv = s->rhr.v;
                rk = s->rhr.k;
            }
            v = su_vk(rv, rk, i);
            d = t_of(s, T_ACC);
        }
        su_drive(&s->o, s->sim, s->instance, p->rd[i], v, d);
    }
}

static void update_outputs(AyState *s, SimTime delay) {
    if (s->unknown || s->mode == LOGIC_X) {
        drive_all_x(s);
        return;
    }
    if (s->mode == LOGIC_1) {
        update_mode1(s, delay);
    } else {
        update_mode0(s, delay);
    }
}

/* ---- Reset ----------------------------------------------------------------------------- */

static void reset_state(AyState *s) {
    s->thr_full = LOGIC_0;
    s->ds_hold = 0;
    s->dav = s->overrun = s->fe = s->pe = LOGIC_0;
    s->dav_wait = 0;
    s->dav_wait_rdav = 0;
    s->eoc_pulse = 0;
    s->tsr = LOGIC_1;
    s->resync = 0;
    if (s->clears_data) {
        s->rhr = su_reg(0, 0xFF);
    }
    if (s->dev->reset_clears_control) {
        s->control = su_reg(0, 0xFF);
    }
    /* Mode 1 (CLEAR): status register, interrupts, break. */
    s->thre = LOGIC_1;
    s->tsre = LOGIC_1;
    s->psi = s->da_m1 = s->oe_m1 = s->fe_m1 = s->pe_m1 = LOGIC_0;
    s->thre_due = 0;
    s->int_da = s->int_thre = s->int_done = s->int_psi = s->int_cts = 0;
    s->break_hold = 0;
    serial_tx_reset(&s->line.tx);
    serial_rx_reset(&s->line.rx);
    update_format(s);
}

static LogicValue reset_active(const AyState *s) {
    /* Mode 0: XR / MR high resets; mode 1: CLEAR low. Z reads inactive. */
    if (s->mode == LOGIC_1) {
        LogicValue v = serial_level(s->sim, s->instance, s->pins->reset, LOGIC_1);

        return su_not(v);
    }
    return serial_level(s->sim, s->instance, s->pins->reset, LOGIC_0);
}

static void sample_strobe(AyState *s);

static void apply_reset(AyState *s) {
    LogicValue r = reset_active(s);

    if (r == LOGIC_X) {
        s->unknown = 1;
        s->in_reset = 0;
    } else if (r == LOGIC_1) {
        s->unknown = 0;
        s->in_reset = 1;
        reset_state(s);
        serial_line_sender_reset(&s->line, s->sim, s->instance);
    } else {
        s->in_reset = 0;
        if (s->unknown) {
            /* Out of an X reset: everything the reset sets is unknown. */
            s->unknown = 0;
            s->thr_full = s->dav = s->overrun = s->fe = s->pe = LOGIC_X;
            s->rhr = su_reg(0, 0);
            s->control = su_reg(0, 0);
            s->tsr = LOGIC_X;
            s->thre = s->tsre = s->psi = s->da_m1 = s->oe_m1 = s->fe_m1 = s->pe_m1 = LOGIC_X;
            update_format(s);
        }
    }
    sample_strobe(s);
    update_outputs(s, 0);
}

/* ---- Mode 0: control latch, data strobe, RDAV --------------------------------------------- */

static void sample_strobe(AyState *s) {
    LogicValue strobe;
    unsigned i;

    if (s->mode != LOGIC_0 || s->unknown) {
        return;
    }
    strobe = serial_level(s->sim, s->instance, s->pins->strobe, LOGIC_X);
    s->strobe_level = strobe;
    if (strobe == LOGIC_0) {
        return;
    }
    /* Transparent while high; X strobe: the word may change, so X where the
       pins differ from the held word. */
    for (i = 0; i < C_COUNT; i++) {
        LogicValue pin = serial_level(s->sim, s->instance, s->pins->control[i], LOGIC_X);
        LogicValue held = su_bit(s->control, i);
        LogicValue now = strobe == LOGIC_1 ? pin : (pin == held ? pin : LOGIC_X);
        uint8_t bit = (uint8_t)(1u << i);

        s->control.v = (uint8_t)((s->control.v & ~bit) | (now == LOGIC_1 ? bit : 0));
        s->control.k = (uint8_t)((s->control.k & ~bit) | (now != LOGIC_X ? bit : 0));
    }
    update_format(s);
}

static void load_thr(AyState *s, int x_strobe) {
    unsigned v;
    unsigned k;

    su_read_bus(s->sim, s->instance, s->pins->td, 8, &v, &k);
    if (x_strobe) {
        k = 0;
    }
    s->thr = su_reg((uint8_t)v, (uint8_t)k);
    s->thr_full = x_strobe ? LOGIC_X : LOGIC_1;
}

static void data_strobe_changed(AyState *s) {
    LogicValue ds = serial_level(s->sim, s->instance, s->pins->ds, LOGIC_X);
    LogicValue was = s->ds_level;

    s->ds_level = ds;
    if (s->in_reset || s->unknown || s->mode != LOGIC_0) {
        return;
    }
    if (ds == LOGIC_0 && was != LOGIC_0) {
        /* TBMT falls tds after the strobe (Fig.18, drawn from DS). */
        s->ds_hold = 1;
        update_outputs(s, t_of(s, T_DS));
    } else if (was == LOGIC_0 && ds == LOGIC_1) {
        s->ds_hold = 0;
        load_thr(s, 0);      /* data taken on the trailing edge */
        update_outputs(s, t_of(s, T_DS));
    } else if (ds == LOGIC_X || (was == LOGIC_X && ds == LOGIC_0)) {
        s->ds_hold = ds == LOGIC_0;
        load_thr(s, 1);
        update_outputs(s, 0);
    } else if (was == LOGIC_X && ds == LOGIC_1) {
        load_thr(s, 1);      /* an X strobe ended: the cycle was X */
        update_outputs(s, 0);
    }
}

static void rdav_changed(AyState *s) {
    LogicValue r = serial_level(s->sim, s->instance, s->pins->rdav, LOGIC_X);

    if (s->in_reset || s->unknown || s->mode != LOGIC_0) {
        return;
    }
    if (r == LOGIC_0) {
        s->dav = LOGIC_0;
    } else if (r == LOGIC_X && s->dav != LOGIC_0) {
        s->dav = LOGIC_X;
    } else if (r == LOGIC_1 && s->dav_wait_rdav) {
        /* COM1863: DAV rises once RDAR is high again (p7). */
        s->dav_wait_rdav = 0;
        s->dav = LOGIC_1;
    }
    update_outputs(s, t_of(s, T_RDAV));
}

/* ---- Transmitter --------------------------------------------------------------------- */

static int tx_fetch(void *context, uint16_t *data, uint16_t *known) {
    AyState *s = context;

    if (s->thr_full == LOGIC_0) {
        return 0;
    }
    if (s->mode == LOGIC_1 && s->cts_level != LOGIC_0) {
        return 0;   /* CTS high inhibits the transfer (p4) */
    }
    *data = s->thr.v;
    *known = s->thr_full == LOGIC_1 ? s->thr.k : 0;
    s->thr_full = LOGIC_0;
    if (s->mode == LOGIC_1) {
        s->thre_due = 1;
        s->tsre = LOGIC_0;
        s->frame_sent = 0;
    }
    return 1;
}

static void tx_done(void *context, uint16_t data, uint16_t known) {
    AyState *s = context;

    (void)known;
    serial_line_log(&s->line, data);
    s->eoc_pulse = 1;
    s->frame_sent = 1;
}

static void tx_tick(AyState *s) {
    SerialTxEvents events = {s, tx_fetch, NULL, tx_done};
    int busy_before;

    if (s->in_reset || s->unknown || s->mode == LOGIC_X) {
        return;
    }
    if (s->mode == LOGIC_1 && s->cts_level == LOGIC_1) {
        return;   /* CTS high also stops the shifting (p4) */
    }
    s->eoc_pulse = 0;
    if (s->thre_due) {
        /* One TCLOCK after the load: THRE and its interrupt (p6). */
        s->thre_due = 0;
        s->thre = LOGIC_1;
        if (su_bit(s->control, 7) == LOGIC_1) {
            s->int_thre = 1;
        }
    }
    busy_before = serial_tx_busy(&s->line.tx);
    s->tsr = serial_tx_tick(&s->line.tx, &s->format, &events);
    if (s->mode == LOGIC_1) {
        if (s->frame_sent && s->break_hold && su_bit(s->control, 6) == LOGIC_0) {
            s->break_hold = 0;   /* a word was transmitted */
        }
        if (busy_before && !serial_tx_busy(&s->line.tx)) {
            s->tsre = LOGIC_1;
            if (s->thre == LOGIC_1 && s->thr_full == LOGIC_0) {
                s->int_done = 1;   /* THRE . TSRE */
            }
        }
        s->frame_sent = 0;
    }
    update_outputs(s, t_of(s, T_PD));
}

/* ---- Receiver ------------------------------------------------------------------------- */

static void on_received(void *context, const SerialRxChar *c) {
    AyState *s = context;
    uint8_t mask = (uint8_t)((1u << s->format.data_bits) - 1);
    LogicValue pe = (c->flags_known & SERIAL_RX_PE) ? su_bool(c->flags & SERIAL_RX_PE) : LOGIC_X;
    LogicValue fe = (c->flags_known & SERIAL_RX_FE) ? su_bool(c->flags & SERIAL_RX_FE) : LOGIC_X;

    if (s->format.parity == SERIAL_PARITY_NONE) {
        pe = LOGIC_0;   /* NP: PE unconditionally 0 */
    }
    if (fe == LOGIC_1 && !(c->flags & SERIAL_RX_BI)) {
        s->resync = 1;
    }
    /* Unused high bits read 0 (decisions.md: unused RD bits 0). */
    s->rhr = su_reg((uint8_t)(c->data & mask), (uint8_t)((c->known & mask) | ~mask));
    if (s->format.format_x) {
        s->rhr.k = 0;
    }
    if (s->mode == LOGIC_1) {
        s->oe_m1 = s->da_m1 == LOGIC_0 ? LOGIC_0 : s->da_m1 == LOGIC_1 ? LOGIC_1 : LOGIC_X;
        s->pe_m1 = pe;
        s->fe_m1 = fe;
        s->da_m1 = LOGIC_1;
        s->int_da = 1;
        return;
    }
    if (s->dev->hiacc != NULL) {
        /* COM1863/COM8018 (p5, p8): OR is reset only if RDA = 0 and RDAR = 1;
           in HIACC mode RDA is notched low; RDA rises when RDAR is high. */
        LogicValue rdar = serial_level(s->sim, s->instance, s->pins->rdav, LOGIC_X);

        s->overrun = su_or(s->dav, su_not(rdar));
        if (hiacc_on(s)) {
            s->dav = LOGIC_0;
        }
    } else {
        s->overrun = s->dav;
    }
    s->pe = pe;
    s->fe = fe;
    s->dav_wait = 1;
    sim_schedule_timer(s->sim, s->instance, 1, t_of(s, T_DAV));
}

static void rx_tick(AyState *s) {
    SerialRxEvents events = {s, on_received, NULL};
    LogicValue pin;
    LogicValue level;

    if (s->in_reset || s->unknown || s->mode == LOGIC_X) {
        return;
    }
    pin = serial_level(s->sim, s->instance, s->pins->si, LOGIC_X);
    level = serial_line_rx_input(&s->line, s->sim, s->instance, pin, &s->format);
    s->resync = 0;
    serial_rx_tick(&s->line.rx, level, &s->format, &events);
    if (s->resync) {
        su_rx_no_resync(&s->line.rx, level);
    }
    update_outputs(s, 0);
}

static void dav_timer(AyState *s) {
    LogicValue rdav;

    if (!s->dav_wait || s->in_reset || s->unknown || s->mode != LOGIC_0) {
        return;
    }
    s->dav_wait = 0;
    rdav = serial_level(s->sim, s->instance, s->pins->rdav, LOGIC_X);
    if (rdav == LOGIC_1) {
        s->dav = LOGIC_1;
    } else if (rdav == LOGIC_X) {
        s->dav = LOGIC_X;
    } else if (s->dev->hiacc != NULL) {
        s->dav_wait_rdav = 1;   /* COM1863: RDA waits for RDAR high */
    } else {
        s->dav = LOGIC_0;       /* AY: RDAV low holds the DAV flip-flop reset */
    }
    update_outputs(s, t_of(s, T_PD));
}

/* ---- Mode 1: the CDP1800 bus ---------------------------------------------------------- */

static LogicValue m1_selected(const AyState *s) {
    LogicValue cs1 = serial_level(s->sim, s->instance, s->pins->cpu[CPU_CS1], LOGIC_0);
    LogicValue cs2 = serial_level(s->sim, s->instance, s->pins->cpu[CPU_CS2], LOGIC_1);
    LogicValue cs3 = serial_level(s->sim, s->instance, s->pins->cpu[CPU_CS3], LOGIC_0);

    return su_and(su_and(cs1, su_not(cs2)), cs3);
}

static void m1_write(AyState *s, unsigned rsel, uint8_t v, uint8_t k) {
    if (rsel == 0 || rsel == 2) {
        s->thr = su_reg(v, rsel == 2 ? 0 : k);
        s->thr_full = rsel == 2 ? LOGIC_X : LOGIC_1;
        s->thre = rsel == 2 ? LOGIC_X : LOGIC_0;
    }
    if (rsel == 1 || rsel == 2) {
        LogicValue tr = rsel == 2 ? LOGIC_X : su_vk(v, k, 7);

        if (tr == LOGIC_1) {
            /* TR = 1 inhibits changing the other control flip-flops. */
            s->control.v |= 0x80;
            s->control.k |= 0x80;
            if (s->thre == LOGIC_1) {
                s->int_thre = 1;   /* the initial THRE interrupt */
            }
        } else if (tr == LOGIC_0) {
            s->control = su_reg(v, k);
        } else {
            s->control = su_reg(0, 0);
        }
        if (su_bit(s->control, 6) == LOGIC_1) {
            s->break_hold = 1;
        }
        update_format(s);
    }
}

static void m1_bus(AyState *s, int tpb_rise, int tpb_fall) {
    LogicValue sel = m1_selected(s);
    LogicValue rw = serial_level(s->sim, s->instance, s->pins->cpu[CPU_RW], LOGIC_X);
    LogicValue rsel = serial_level(s->sim, s->instance, s->pins->cpu[CPU_RSEL], LOGIC_X);
    unsigned rs = rsel == LOGIC_X ? 2u : rsel == LOGIC_1 ? 1u : 0u;
    int read = 0;

    if (sel == LOGIC_1 && rw == LOGIC_1) {
        read = rs == 2 ? 2 : 1;
    } else if (sel == LOGIC_X || (sel == LOGIC_1 && rw == LOGIC_X)) {
        read = 2;
    }
    s->read_cycle = read;
    s->cycle_rsel = rs;

    /* TPB during a read: the side effects (Table I). */
    if (read == 1 && tpb_rise) {
        if (rs == 0) {
            s->da_m1 = LOGIC_0;
            s->int_da = 0;
        } else {
            s->int_thre = 0;
            s->int_done = 0;
            s->int_cts = 0;
        }
    }
    if (read == 1 && tpb_fall && rs == 1) {
        s->int_psi = 0;
        s->psi = LOGIC_0;
    }
    /* Write: the overlap of TPB, the selects and RD/WR = 0, taken on TPB's
       trailing edge. */
    if (tpb_rise && sel != LOGIC_0 && rw != LOGIC_1) {
        s->write_cycle = sel == LOGIC_1 && rw == LOGIC_0 ? 1 : 2;
        if (s->write_cycle == 1 && rs == 0) {
            s->int_thre = 0;   /* write of a character: TPB leading edge */
            s->int_done = 0;
        }
    }
    if (tpb_fall && s->write_cycle != 0) {
        unsigned v;
        unsigned k;

        su_read_bus(s->sim, s->instance, s->pins->td, 8, &v, &k);
        if (s->write_cycle == 2 || sel != LOGIC_1 || rw != LOGIC_0) {
            /* X cycle: X into every register the address could name. */
            m1_write(s, rs, 0, 0);
        } else {
            m1_write(s, rs, (uint8_t)v, (uint8_t)k);
        }
        s->write_cycle = 0;
    }
    update_outputs(s, t_of(s, T_INT));
}

static void m1_peripheral(AyState *s, size_t pin) {
    if (pin == s->pins->cpu[CPU_PSI]) {
        LogicValue v = serial_level(s->sim, s->instance, pin, LOGIC_X);

        if (s->psi_level == LOGIC_1 && v == LOGIC_0) {
            s->psi = LOGIC_1;
            s->int_psi = 1;
        }
        s->psi_level = v;
    }
    if (pin == s->pins->cpu[CPU_CTS]) {
        LogicValue v = serial_level(s->sim, s->instance, pin, LOGIC_X);

        if (s->cts_level == LOGIC_0 && v == LOGIC_1) {
            if (s->thre == LOGIC_1 && s->tsre == LOGIC_1) {
                s->int_cts = 1;
            }
            if (su_bit(s->control, 6) == LOGIC_0) {
                s->break_hold = 0;   /* CTS high ends a held break */
            }
        }
        s->cts_level = v;
    }
    update_outputs(s, t_of(s, T_INT));
}

/* ---- Engine callbacks ------------------------------------------------------------------ */

static void read_mode(AyState *s) {
    LogicValue m = s->pins->cpu[CPU_MODE] == SERIAL_NO_PIN
        ? LOGIC_0 : serial_level(s->sim, s->instance, s->pins->cpu[CPU_MODE], LOGIC_X);

    if (m != s->mode) {
        s->mode = m;
        s->control = s->dev->reset_clears_control ? su_reg(0, 0xFF) : su_reg(0, 0);
        update_format(s);
    }
}

static void ay_start(Sim *sim, SimInstance *instance, void *raw) {
    AyState *s = raw;
    const AyPins *p;

    s->sim = sim;
    s->instance = instance;
    p = s->pins;
    s->mode = LOGIC_Z;   /* forces read_mode to take the pin */
    read_mode(s);
    s->tcp_level = serial_level(sim, instance, p->tcp, LOGIC_X);
    s->rcp_level = serial_level(sim, instance, p->rcp, LOGIC_X);
    s->ds_level = serial_level(sim, instance, p->ds, LOGIC_X);
    s->psi_level = serial_level(sim, instance, p->cpu[CPU_PSI], LOGIC_X);
    s->cts_level = serial_level(sim, instance, p->cpu[CPU_CTS], LOGIC_X);
    s->tpb_level = serial_level(sim, instance, p->cpu[CPU_TPB], LOGIC_X);
    update_format(s);
    apply_reset(s);
}

static void ay_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    AyState *s = raw;
    const AyPins *p = s->pins;
    unsigned i;

    (void)old_value;
    (void)new_value;
    s->sim = sim;
    s->instance = instance;
    if (pin == p->cpu[CPU_MODE]) {
        read_mode(s);
        apply_reset(s);
        return;
    }
    if (pin == p->reset) {
        apply_reset(s);
        return;
    }
    if (pin == p->hiacc) {
        update_format(s);
    }
    if (pin == p->tcp) {
        LogicValue v = serial_level(sim, instance, pin, LOGIC_X);
        int falling = s->tcp_level == LOGIC_1 && v == LOGIC_0;

        s->tcp_level = v;
        if (falling) {
            tx_tick(s);
        }
    }
    if (pin == p->rcp) {
        LogicValue v = serial_level(sim, instance, pin, LOGIC_X);
        int falling = s->rcp_level == LOGIC_1 && v == LOGIC_0;

        s->rcp_level = v;
        if (falling) {
            rx_tick(s);
        }
    }
    if (s->mode == LOGIC_1) {
        if (pin == p->cpu[CPU_TPB]) {
            LogicValue v = serial_level(sim, instance, pin, LOGIC_X);
            int rise = s->tpb_level != LOGIC_1 && v == LOGIC_1;
            int fall = s->tpb_level == LOGIC_1 && v != LOGIC_1;

            s->tpb_level = v;
            if (!s->in_reset && !s->unknown) {
                m1_bus(s, rise, fall);
            }
            return;
        }
        if (pin == p->cpu[CPU_CS1] || pin == p->cpu[CPU_CS2] || pin == p->cpu[CPU_CS3] ||
            pin == p->cpu[CPU_RSEL] || pin == p->cpu[CPU_RW]) {
            if (!s->in_reset && !s->unknown) {
                m1_bus(s, 0, 0);
            }
            return;
        }
        if (pin == p->cpu[CPU_PSI] || pin == p->cpu[CPU_CTS]) {
            if (!s->in_reset && !s->unknown) {
                m1_peripheral(s, pin);
            } else {
                s->psi_level = serial_level(sim, instance, p->cpu[CPU_PSI], LOGIC_X);
                s->cts_level = serial_level(sim, instance, p->cpu[CPU_CTS], LOGIC_X);
            }
            return;
        }
        if (pin == p->cpu[CPU_ES]) {
            update_outputs(s, t_of(s, T_INT));
        }
        return;
    }
    /* Mode 0. */
    s->psi_level = serial_level(sim, instance, p->cpu[CPU_PSI], LOGIC_X);
    s->cts_level = serial_level(sim, instance, p->cpu[CPU_CTS], LOGIC_X);
    s->tpb_level = serial_level(sim, instance, p->cpu[CPU_TPB], LOGIC_X);
    if (pin == p->strobe) {
        sample_strobe(s);
    }
    for (i = 0; i < C_COUNT; i++) {
        if (pin == p->control[i]) {
            sample_strobe(s);
        }
    }
    if (pin == p->ds) {
        data_strobe_changed(s);
    }
    if (pin == p->rdav) {
        rdav_changed(s);
    }
    if (pin == p->rde || pin == p->swe) {
        update_outputs(s, 0);
    }
}

static void ay_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t id) {
    AyState *s = raw;

    s->sim = sim;
    s->instance = instance;
    if (id == 1) {
        dav_timer(s);
    }
}

/* ---- Terminal -------------------------------------------------------------------------------- */

static unsigned ay_channels(const void *raw) {
    (void)raw;
    return 1;
}

static SerialLine *ay_line(void *raw, unsigned channel) {
    (void)channel;
    return &((AyState *)raw)->line;
}

static int ay_async(const void *raw, unsigned channel) {
    (void)raw;
    (void)channel;
    return 1;
}

/* ---- Create / destroy ------------------------------------------------------------------------- */

static void *ay_create(const Part *part, const void *data, const void *pins,
                       const SerialGrade *grade, const void *options) {
    AyState *s = calloc(1, sizeof *s);
    const AyOptions *opt = options;
    size_t i;

    if (s == NULL) {
        return NULL;
    }
    s->base.model = &AY1013_MODEL;
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
    s->five_halves = opt != NULL && opt->five_stop_halves != 0 ? (unsigned)opt->five_stop_halves
                                                               : s->dev->five_stop_halves;
    s->clears_data = opt != NULL && opt->reset_clears_data >= 0 ? opt->reset_clears_data
                                                                : s->dev->reset_clears_data;
    serial_line_init(&s->line, s->pins->si);
    /* Power-on: the reset state; the control word and (unless the reset
       clears it) the receive holding register are X. */
    s->control = su_reg(0, 0);
    s->rhr = su_reg(0, 0);
    s->mode = LOGIC_0;
    reset_state(s);
    s->tcp_level = s->rcp_level = s->ds_level = s->strobe_level = LOGIC_X;
    s->psi_level = s->cts_level = s->tpb_level = LOGIC_X;
    return s;
}

static void ay_destroy(void *raw) {
    AyState *s = raw;

    serial_line_free(&s->line);
    free(s->o.out);
    free(s);
}

/* ---- Loader ---------------------------------------------------------------------------------- */

static int load_stops(KindContext *ctx, const cJSON *json, unsigned *halves) {
    double v;

    if (kind_number(ctx, json, "fiveBitStops", &v) != 0) {
        return -1;
    }
    if (v != 1.5 && v != 2) {
        return kind_error(ctx, "\"fiveBitStops\" must be 1.5 or 2");
    }
    *halves = v == 1.5 ? 3u : 4u;
    return 0;
}

static int load_control(KindContext *ctx, const cJSON *json, AyDevice *d) {
    static const char *const CONTROL_KEYS[] = {
        "strobe", "noParity", "stops", "wordLength", "evenParity", NULL
    };
    const cJSON *c = kind_member(ctx, json, "control", 1);
    const char *wl[2];

    if (c == NULL) {
        return -1;
    }
    kind_push(ctx, "control");
    if (kind_check_keys(ctx, c, CONTROL_KEYS, NULL) != 0 ||
        kind_string(ctx, c, "strobe", &d->strobe) != 0 ||
        kind_string(ctx, c, "noParity", &d->control[C_NP]) != 0 ||
        kind_string(ctx, c, "stops", &d->control[C_TSB]) != 0 ||
        kind_string(ctx, c, "evenParity", &d->control[C_EPS]) != 0 ||
        su_load_names(ctx, c, "wordLength", 1, wl, 2, 2, NULL) != 0) {
        return -1;
    }
    d->control[C_NB1] = wl[0];
    d->control[C_NB2] = wl[1];
    kind_pop(ctx);
    return 0;
}

static int load_clocks(KindContext *ctx, const cJSON *json, AyDevice *d) {
    static const char *const CLOCK_KEYS[] = {"transmit", "receive", "hiacc", NULL};
    const char *names[3];

    if (su_load_pin_object(ctx, json, "clocks", 1, CLOCK_KEYS, names, 3u) != 0) {
        return -1;
    }
    d->tcp = names[0];
    d->rcp = names[1];
    d->hiacc = names[2];
    return 0;
}

static void *ay_load_device(KindContext *ctx, const cJSON *json, unsigned *channels) {
    AyDevice *d = kind_alloc(ctx, sizeof *d);

    if (d == NULL ||
        su_load_names(ctx, json, "transmitData", 1, d->td, 8, 8, NULL) != 0 ||
        su_load_names(ctx, json, "receiveData", 1, d->rd, 8, 8, NULL) != 0 ||
        kind_string(ctx, json, "receiveEnable", &d->rde) != 0 ||
        kind_string(ctx, json, "statusEnable", &d->swe) != 0 ||
        su_load_pin_object(ctx, json, "status", 1, STATUS_KEYS, d->status, 0x3Fu) != 0 ||
        load_control(ctx, json, d) != 0 ||
        kind_string(ctx, json, "dataStrobe", &d->ds) != 0 ||
        kind_string(ctx, json, "resetDav", &d->rdav) != 0 ||
        kind_string(ctx, json, "serialOut", &d->so) != 0 ||
        kind_string(ctx, json, "serialIn", &d->si) != 0 ||
        load_clocks(ctx, json, d) != 0 ||
        kind_string(ctx, json, "reset", &d->reset) != 0 ||
        kind_bool(ctx, json, "resetClearsData", &d->reset_clears_data) != 0 ||
        load_stops(ctx, json, &d->five_stop_halves) != 0) {
        return NULL;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "resetClearsControl") != NULL &&
        kind_bool(ctx, json, "resetClearsControl", &d->reset_clears_control) != 0) {
        return NULL;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "cpuMode") != NULL) {
        if (su_load_pin_object(ctx, json, "cpuMode", 1, CPU_KEYS, d->cpu, 0x3FFu) != 0) {
            return NULL;
        }
        d->has_cpu = 1;
    }
    *channels = 1;
    return d;
}

static void *ay_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t pin_count) {
    const AyDevice *d = raw;
    AyPins *p = kind_alloc(ctx, sizeof *p);
    unsigned i;

    if (p == NULL ||
        su_bind_names(ctx, pins, pin_count, "transmitData", d->td, p->td, 8) != 0 ||
        su_bind_names(ctx, pins, pin_count, "receiveData", d->rd, p->rd, 8) != 0 ||
        su_bind_object(ctx, pins, pin_count, "status", STATUS_KEYS, d->status, p->status) != 0 ||
        su_bind_names(ctx, pins, pin_count, "control", d->control, p->control, C_COUNT) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "control.strobe", d->strobe, &p->strobe) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "receiveEnable", d->rde, &p->rde) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "statusEnable", d->swe, &p->swe) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "dataStrobe", d->ds, &p->ds) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "resetDav", d->rdav, &p->rdav) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "serialOut", d->so, &p->so) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "serialIn", d->si, &p->si) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "clocks.transmit", d->tcp, &p->tcp) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "clocks.receive", d->rcp, &p->rcp) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "clocks.hiacc", d->hiacc, &p->hiacc) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "reset", d->reset, &p->reset) != 0) {
        return NULL;
    }
    for (i = 0; i < CPU_COUNT; i++) {
        p->cpu[i] = SERIAL_NO_PIN;
    }
    if (d->has_cpu &&
        su_bind_object(ctx, pins, pin_count, "cpuMode", CPU_KEYS, d->cpu, p->cpu) != 0) {
        return NULL;
    }
    return p;
}

static const void *ay_load_part(KindContext *ctx, const void *data, const cJSON *json) {
    AyOptions *o;

    (void)data;
    if (cJSON_GetObjectItemCaseSensitive(json, "fiveBitStops") == NULL &&
        cJSON_GetObjectItemCaseSensitive(json, "resetClearsData") == NULL) {
        return NULL;
    }
    o = kind_alloc(ctx, sizeof *o);
    if (o == NULL) {
        return NULL;
    }
    o->reset_clears_data = -1;
    if (cJSON_GetObjectItemCaseSensitive(json, "fiveBitStops") != NULL) {
        unsigned halves;

        if (load_stops(ctx, json, &halves) != 0) {
            return NULL;
        }
        o->five_stop_halves = (int)halves;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "resetClearsData") != NULL &&
        kind_bool(ctx, json, "resetClearsData", &o->reset_clears_data) != 0) {
        return NULL;
    }
    return o;
}

const SerialModel AY1013_MODEL = {
    "ay1013", KEYS, TIMING, PART_KEYS,
    ay_load_device, ay_bind, ay_load_part, NULL,
    ay_create, ay_destroy, ay_start, ay_pin_changed, ay_timer,
    ay_channels, ay_line, ay_async
};
