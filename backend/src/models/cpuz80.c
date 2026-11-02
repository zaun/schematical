/*
 * cpuz80: the Z80 bus adapter (plan-09-C), for "core": "z80" (Zilog Z84C00,
 * vendored z80.h).
 *
 * The core advances one T-state per tick, but puts its requests where it
 * finds convenient (memory on T2, I/O read on TW*, ...). The adapter is a
 * commit scheduler around it:
 *
 * - At each CLK rise with no machine cycle (MC) in progress it peeks ahead
 *   on a copy of the core (up to 3 ticks) for the next bus request. The
 *   request's type and tick offset say whether this rise is the T1 of an
 *   opcode fetch, memory read/write, I/O read/write or interrupt
 *   acknowledge, or an internal T-state.
 * - Each core tick of the MC is committed at the clock edge where the chip
 *   samples that tick's inputs: WAIT at a fall, data at T3 rise (fetch,
 *   int-ack) or T3 fall (memory and I/O reads), INT, NMI, BUSREQ and RESET
 *   at every rise.
 * - The pins follow the PS0178 waveforms of the MC, from the CLK edges plus
 *   each row's delay, independent of where the core put its request.
 *
 * The core has no RESET, BUSREQ or BUSACK; the adapter provides them, puts
 * PC on the bus during the int-ack (the core leaves the refresh address),
 * and drives HALT# at T4 fall from a one-tick peek. Lost and the
 * differential rule are plan-09-B's (cpu6502.c).
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

#include "../third_party/chips/z80.h"
#include "../third_party/chips/z80dasm.h"

/* Control order (cpu.c CONTROLS_Z80). */
enum { K_CLOCK, K_M1, K_MREQ, K_IORQ, K_RD, K_WR, K_RFSH, K_HALT, K_BUSACK, K_WAIT, K_INT,
       K_NMI, K_RESET, K_BUSREQ, K_COUNT };
/* Timing order (cpu.c TIMING_Z80). */
enum { T_CC, T_WCH, T_WCL, T_DCRA, T_DCFMREQF, T_DCRMREQR, T_DCFMREQR, T_DCFRDF, T_DCRRDR, T_SDCR,
       T_SWAITCF, T_DCRM1F, T_DCRM1R, T_DCRRFSHF, T_DCRRFSHR, T_DCFRDR, T_DCRRDF, T_SDCF,
       T_DCRIORQF, T_DCFIORQR, T_DCFWRF, T_DCFWRR, T_DCRWRF, T_DCFHALT, T_WNMI, T_SBUSREQ,
       T_DCRBUSACKF, T_DCFBUSACKR, T_DCRDZ, T_DCRCTZ, T_DCRAZ, T_SRESET, T_SINT, T_DCFIORQF,
       T_DCFD };

#define PIN_DATA 0xFF0000ull
#define REQUEST (Z80_MREQ | Z80_IORQ)

typedef enum { MC_NONE, MC_FETCH, MC_MREAD, MC_MWRITE, MC_IOREAD, MC_IOWRITE, MC_INTACK } Mc;

typedef struct {
    CpuBase base;
    const SimTime *t;
    size_t addr_pin[CPU_ADDRESS_BITS];
    size_t data_pin[CPU_DATA_BITS];
    size_t ctl[K_COUNT];
    z80_t core;
    uint64_t pins;                 /* the last tick's result, passed back in */
    CpuStatus status;
    uint64_t cycles;
    unsigned reset_count;
    LogicValue clock;
    SimTime last_rise, last_fall;
    SimTime changed[CPU_MAX_PINS];
    LogicValue seen[CPU_MAX_PINS];
    /* The latest samples. */
    LogicValue int_level, nmi_level, wait_level, busreq_last;
    int nmi_pending;               /* a fall of NMI# not yet passed to the core */
    int nmi_short;                 /* an NMI# pulse shorter than tWNMI */
    SimTime nmi_fell;
    /* The machine cycle. */
    Mc mc;
    int phase;
    int first_wait;                /* the T2 fall of a memory write: WR# low there */
    uint16_t mc_addr;
    uint8_t mc_data;
    int release_data;              /* a write's data goes Z at the next T1 rise */
    int halt_release;              /* HALT# goes high at the next fall */
    int released;                  /* bus released: BUSACK# low */
    int resume;                    /* BUSREQ# back high: BUSACK# high at the next fall */
    LogicValue addr_out[CPU_ADDRESS_BITS];
    LogicValue data_out[CPU_DATA_BITS];
    LogicValue out[K_COUNT];
    CpuTracker tracker;
} Z80;

/* ---- Drives -------------------------------------------------------------- */

static void drive(Sim *sim, SimInstance *instance, size_t pin, LogicValue value, SimTime delay) {
    sim_schedule_drive(sim, instance, pin, value, DRIVE_STRONG, delay);
}

static void set_out(Sim *sim, SimInstance *instance, Z80 *s, int key, LogicValue value,
                    SimTime delay) {
    if (s->out[key] != value) {
        drive(sim, instance, s->ctl[key], value, delay);
        s->out[key] = value;
    }
}

/* A <- value: each changing bit X now, then the value at tDCRA. */
static void set_address(Sim *sim, SimInstance *instance, Z80 *s, uint16_t value) {
    size_t i;

    for (i = 0; i < CPU_ADDRESS_BITS; i++) {
        LogicValue v = (value >> i) & 1 ? LOGIC_1 : LOGIC_0;

        if (s->addr_out[i] != v) {
            drive(sim, instance, s->addr_pin[i], LOGIC_X, 0);
            drive(sim, instance, s->addr_pin[i], v, s->t[T_DCRA]);
            s->addr_out[i] = v;
        }
    }
}

/* D <- value: X now, the value at tDCFD. */
static void set_data(Sim *sim, SimInstance *instance, Z80 *s, uint8_t value) {
    size_t i;

    for (i = 0; i < CPU_DATA_BITS; i++) {
        LogicValue v = (value >> i) & 1 ? LOGIC_1 : LOGIC_0;

        drive(sim, instance, s->data_pin[i], LOGIC_X, 0);
        drive(sim, instance, s->data_pin[i], v, s->t[T_DCFD]);
        s->data_out[i] = v;
    }
}

static void float_data(Sim *sim, SimInstance *instance, Z80 *s, SimTime delay) {
    size_t i;

    for (i = 0; i < CPU_DATA_BITS; i++) {
        if (s->data_out[i] != LOGIC_Z) {
            drive(sim, instance, s->data_pin[i], LOGIC_Z, delay);
            s->data_out[i] = LOGIC_Z;
        }
    }
}

static void float_address(Sim *sim, SimInstance *instance, Z80 *s, SimTime delay) {
    size_t i;

    for (i = 0; i < CPU_ADDRESS_BITS; i++) {
        if (s->addr_out[i] != LOGIC_Z) {
            drive(sim, instance, s->addr_pin[i], LOGIC_Z, delay);
            s->addr_out[i] = LOGIC_Z;
        }
    }
}

/* Every output to one value now, cancelling pending drives (lost: X). */
static void all_outputs(Sim *sim, SimInstance *instance, Z80 *s, LogicValue value) {
    static const int OUTPUTS[] = {K_M1, K_MREQ, K_IORQ, K_RD, K_WR, K_RFSH, K_HALT, K_BUSACK};
    size_t i;

    for (i = 0; i < CPU_ADDRESS_BITS; i++) {
        sim_schedule_inertial_drive(sim, instance, s->addr_pin[i], value, DRIVE_STRONG, 0);
        s->addr_out[i] = value;
    }
    for (i = 0; i < CPU_DATA_BITS; i++) {
        sim_schedule_inertial_drive(sim, instance, s->data_pin[i], value, DRIVE_STRONG, 0);
        s->data_out[i] = value;
    }
    for (i = 0; i < sizeof OUTPUTS / sizeof OUTPUTS[0]; i++) {
        sim_schedule_inertial_drive(sim, instance, s->ctl[OUTPUTS[i]], value, DRIVE_STRONG, 0);
        s->out[OUTPUTS[i]] = value;
    }
}

static void enter_lost(Sim *sim, SimInstance *instance, Z80 *s) {
    if (s->status == CPU_LOST) {
        return;
    }
    s->status = CPU_LOST;
    s->mc = MC_NONE;
    s->released = 0;
    s->resume = 0;
    s->halt_release = 0;
    s->release_data = 0;
    all_outputs(sim, instance, s, LOGIC_X);
}

/* RESET# low: address and data Z, every control output high. */
static void enter_reset(Sim *sim, SimInstance *instance, Z80 *s) {
    static const int OUTPUTS[] = {K_M1, K_MREQ, K_IORQ, K_RD, K_WR, K_RFSH, K_HALT, K_BUSACK};
    size_t i;

    s->status = CPU_RESET;
    s->reset_count = 0;
    s->mc = MC_NONE;
    s->released = 0;
    s->resume = 0;
    s->halt_release = 0;
    s->release_data = 0;
    for (i = 0; i < CPU_ADDRESS_BITS; i++) {
        sim_schedule_inertial_drive(sim, instance, s->addr_pin[i], LOGIC_Z, DRIVE_STRONG, 0);
        s->addr_out[i] = LOGIC_Z;
    }
    for (i = 0; i < CPU_DATA_BITS; i++) {
        sim_schedule_inertial_drive(sim, instance, s->data_pin[i], LOGIC_Z, DRIVE_STRONG, 0);
        s->data_out[i] = LOGIC_Z;
    }
    for (i = 0; i < sizeof OUTPUTS / sizeof OUTPUTS[0]; i++) {
        sim_schedule_inertial_drive(sim, instance, s->ctl[OUTPUTS[i]], LOGIC_1, DRIVE_STRONG, 0);
        s->out[OUTPUTS[i]] = LOGIC_1;
    }
}

/* ---- The core ------------------------------------------------------------ */

/* Equal state, ignoring the stored pins' D0-D7, INT and WAIT, and the INT
   bit of int_bits while interrupts are disabled (it is only read at a fetch
   with IFF1 set, and every tick rewrites it). */
static int same_core(const z80_t *a, const z80_t *b) {
    const uint64_t mask = ~(PIN_DATA | Z80_INT | Z80_WAIT);
    uint64_t ia = a->int_bits;
    uint64_t ib = b->int_bits;

    if (!a->iff1 && !b->iff1) {
        ia &= ~Z80_INT;
        ib &= ~Z80_INT;
    }
    return a->step == b->step && a->addr == b->addr && a->dlatch == b->dlatch &&
           a->opcode == b->opcode && a->hlx_idx == b->hlx_idx &&
           a->prefix_active == b->prefix_active && (a->pins & mask) == (b->pins & mask) &&
           ia == ib && a->pc == b->pc && a->af == b->af && a->bc == b->bc && a->de == b->de &&
           a->hl == b->hl && a->ix == b->ix && a->iy == b->iy && a->wz == b->wz &&
           a->sp == b->sp && a->ir == b->ir && a->af2 == b->af2 && a->bc2 == b->bc2 &&
           a->de2 == b->de2 && a->hl2 == b->hl2 && a->im == b->im && a->iff1 == b->iff1 &&
           a->iff2 == b->iff2;
}

/* The tick's inputs: the last result with INT and NMI from the latest
   rise, WAIT and data as given. xmask collects the X bits. */
static uint64_t inputs(const Z80 *s, LogicValue wait, int with_data, uint8_t data, uint8_t xdata,
                       uint64_t *xmask) {
    uint64_t in = s->pins & ~(PIN_DATA | Z80_INT | Z80_NMI | Z80_WAIT);

    *xmask = 0;
    if (s->int_level == LOGIC_X) {
        *xmask |= Z80_INT;
    } else if (s->int_level == LOGIC_0) {
        in |= Z80_INT;
    }
    if (s->nmi_level == LOGIC_X) {
        *xmask |= Z80_NMI;
    } else if (s->nmi_level == LOGIC_0) {
        in |= Z80_NMI;
    }
    if (wait == LOGIC_X) {
        *xmask |= Z80_WAIT;
    } else if (wait == LOGIC_0) {
        in |= Z80_WAIT;
    }
    if (with_data) {
        in |= (uint64_t)data << 16;
        *xmask |= (uint64_t)xdata << 16;
    }
    return in;
}

static void status_after_tick(Z80 *s) {
    s->status = (s->pins & Z80_HALT) ? CPU_HALTED : CPU_RUNNING;
}

/* Commits one tick (the differential rule for X inputs). Returns 0 when
   the CPU went lost. *waited: the tick re-ran its step for WAIT. */
static int commit(Sim *sim, SimInstance *instance, Z80 *s, LogicValue wait, int with_data,
                  uint8_t data, uint8_t xdata, int *waited) {
    uint64_t xmask;
    uint64_t in = inputs(s, wait, with_data, data, xdata, &xmask);
    uint16_t step = s->core.step;

    if (xmask == 0) {
        s->pins = z80_tick(&s->core, in);
    } else {
        z80_t a = s->core;
        z80_t b = s->core;
        uint64_t pa = z80_tick(&a, in);
        uint64_t pb = z80_tick(&b, in | xmask);
        uint64_t mask = ~(Z80_INT | Z80_NMI | Z80_WAIT);

        if (!(pa & Z80_WR)) {
            mask &= ~PIN_DATA;   /* not a write: the data bits are the input's */
        }
        if (!same_core(&a, &b) || (pa & mask) != (pb & mask)) {
            enter_lost(sim, instance, s);
            return 0;
        }
        s->core = a;
        s->pins = pa;
    }
    if (waited != NULL) {
        *waited = wait == LOGIC_0 && s->core.step == step;
    }
    if (!(s->pins & Z80_HALT) && s->out[K_HALT] == LOGIC_0) {
        s->halt_release = 1;
    }
    status_after_tick(s);
    return 1;
}

/* The data bus sampled now (setup `setup`): value and X bits. */
static void sample_data(const Sim *sim, const SimInstance *instance, const Z80 *s, SimTime setup,
                        uint8_t *data, uint8_t *xdata) {
    size_t i;

    *data = 0;
    *xdata = 0;
    for (i = 0; i < CPU_DATA_BITS; i++) {
        size_t pin = s->data_pin[i];
        LogicValue v = cpu_sample(sim, instance, pin, s->changed[pin], s->seen[pin], setup, LOGIC_X);

        if (v == LOGIC_X) {
            *xdata |= (uint8_t)(1u << i);
        } else if (v == LOGIC_1) {
            *data |= (uint8_t)(1u << i);
        }
    }
}

static LogicValue sample(const Sim *sim, const SimInstance *instance, const Z80 *s, int key,
                         SimTime setup) {
    size_t pin = s->ctl[key];

    return cpu_sample(sim, instance, pin, s->changed[pin], s->seen[pin], setup, LOGIC_1);
}

/* ---- Machine cycles ------------------------------------------------------ */

/* The refresh half of a fetch (or int-ack) at T3 rise: strobes up, A <- IR,
   RFSH# low. */
static void refresh(Sim *sim, SimInstance *instance, Z80 *s) {
    set_out(sim, instance, s, K_MREQ, LOGIC_1, s->t[T_DCRMREQR]);
    set_out(sim, instance, s, K_RD, LOGIC_1, s->t[T_DCRRDR]);
    set_out(sim, instance, s, K_M1, LOGIC_1, s->t[T_DCRM1R]);
    set_out(sim, instance, s, K_IORQ, LOGIC_1, s->t[T_DCRM1R]);
    set_address(sim, instance, s, Z80_GET_ADDR(s->pins));
    set_out(sim, instance, s, K_RFSH, LOGIC_0, s->t[T_DCRRFSHF]);
}

/* Looks ahead from this rise for the next request: its pins and offset k
   (3 = none within 3 ticks). */
static int peek(const Z80 *s, uint64_t *request) {
    z80_t copy = s->core;
    uint64_t xmask;
    uint64_t in = inputs(s, LOGIC_1, 0, 0, 0, &xmask);
    int k;

    for (k = 0; k < 3; k++) {
        uint64_t out = z80_tick(&copy, in);

        if (out & REQUEST) {
            *request = out;
            return k;
        }
        in = (out & ~(Z80_INT | Z80_NMI | Z80_WAIT)) | (in & (Z80_INT | Z80_NMI));
    }
    *request = 0;
    return 3;
}

static Mc classify(uint64_t request, int k) {
    if ((request & (Z80_M1 | Z80_MREQ | Z80_RD)) == (Z80_M1 | Z80_MREQ | Z80_RD)) {
        return k == 0 ? MC_FETCH : MC_NONE;
    }
    if ((request & (Z80_M1 | Z80_IORQ)) == (Z80_M1 | Z80_IORQ)) {
        return k == 2 ? MC_INTACK : MC_NONE;
    }
    if ((request & (Z80_MREQ | Z80_RD)) == (Z80_MREQ | Z80_RD)) {
        return k == 1 ? MC_MREAD : MC_NONE;
    }
    if ((request & (Z80_MREQ | Z80_WR)) == (Z80_MREQ | Z80_WR)) {
        return k == 1 ? MC_MWRITE : MC_NONE;
    }
    if ((request & (Z80_IORQ | Z80_WR)) == (Z80_IORQ | Z80_WR)) {
        return k == 1 ? MC_IOWRITE : MC_NONE;
    }
    if ((request & (Z80_IORQ | Z80_RD)) == (Z80_IORQ | Z80_RD)) {
        return k == 2 ? MC_IOREAD : MC_NONE;
    }
    return MC_NONE;
}

/* A rise with no MC in progress: start one, or an internal T-state. */
static void mc_start(Sim *sim, SimInstance *instance, Z80 *s) {
    uint64_t request;
    int k = peek(s, &request);
    Mc mc = classify(request, k);

    if (s->out[K_RFSH] == LOGIC_0) {
        set_out(sim, instance, s, K_RFSH, LOGIC_1, s->t[T_DCRRFSHR]);
    }
    if (s->release_data) {
        float_data(sim, instance, s, s->t[T_DCRDZ]);
        s->release_data = 0;
    }
    if (mc != MC_NONE && s->busreq_last == LOGIC_0) {
        /* BUSREQ# was low at the last T-state: release the bus. */
        s->released = 1;
        s->status = CPU_BUS_RELEASED;
        set_out(sim, instance, s, K_BUSACK, LOGIC_0, s->t[T_DCRBUSACKF]);
        float_address(sim, instance, s, s->t[T_DCRAZ]);
        float_data(sim, instance, s, s->t[T_DCRDZ]);
        set_out(sim, instance, s, K_MREQ, LOGIC_Z, s->t[T_DCRCTZ]);
        set_out(sim, instance, s, K_IORQ, LOGIC_Z, s->t[T_DCRCTZ]);
        set_out(sim, instance, s, K_RD, LOGIC_Z, s->t[T_DCRCTZ]);
        set_out(sim, instance, s, K_WR, LOGIC_Z, s->t[T_DCRCTZ]);
        return;
    }
    if (!commit(sim, instance, s, LOGIC_1, 0, 0, 0, NULL)) {
        return;
    }
    s->mc = mc;
    s->phase = 1;
    s->first_wait = 1;
    switch (mc) {
        case MC_NONE:
            return;   /* an internal T-state */
        case MC_FETCH:
            s->mc_addr = Z80_GET_ADDR(s->pins);
            if (!s->core.prefix_active) {
                cpu_tracker_begin(&s->tracker, s->mc_addr);
            }
            break;
        case MC_INTACK:
            s->mc_addr = s->core.pc;   /* the chip puts PC on the bus */
            break;
        default:
            s->mc_addr = Z80_GET_ADDR(request);
            s->mc_data = Z80_GET_DATA(request);
            break;
    }
    set_address(sim, instance, s, s->mc_addr);
    if (mc == MC_FETCH || mc == MC_INTACK) {
        set_out(sim, instance, s, K_M1, LOGIC_0, s->t[T_DCRM1F]);
    }
}

static void mc_done(Z80 *s) {
    s->mc = MC_NONE;
    s->phase = 0;
}

/* HALT# at T4 fall: low when the next tick halts. */
static void halt_check(Sim *sim, SimInstance *instance, Z80 *s) {
    z80_t copy = s->core;
    uint64_t xmask;
    uint64_t out = z80_tick(&copy, inputs(s, LOGIC_1, 0, 0, 0, &xmask));

    if ((out & Z80_HALT) && s->out[K_HALT] == LOGIC_1) {
        set_out(sim, instance, s, K_HALT, LOGIC_0, s->t[T_DCFHALT]);
    }
}

static void mc_rise(Sim *sim, SimInstance *instance, Z80 *s) {
    uint8_t data, xdata;
    int waited;

    switch (s->mc) {
        case MC_FETCH:
        case MC_INTACK:
            if (s->mc == MC_INTACK && (s->phase == 2 || s->phase == 4)) {
                /* T2: tick 2; TW*1: tick 3 (M1|IORQ). */
                if (commit(sim, instance, s, LOGIC_1, 0, 0, 0, NULL)) {
                    s->phase++;
                }
                return;
            }
            if ((s->mc == MC_FETCH && s->phase == 4) || (s->mc == MC_INTACK && s->phase == 8)) {
                /* T3 (or Tw): tick 2 / 4 with WAIT and the data, then the refresh tick. */
                sample_data(sim, instance, s, s->t[T_SDCR], &data, &xdata);
                if (!commit(sim, instance, s, s->wait_level, 1, data, xdata, &waited)) {
                    return;
                }
                if (waited) {
                    s->phase--;
                    return;
                }
                if (s->mc == MC_FETCH) {
                    cpu_tracker_read(&s->tracker, s->mc_addr, data);
                }
                if (!commit(sim, instance, s, LOGIC_1, 0, 0, 0, NULL)) {
                    return;
                }
                refresh(sim, instance, s);
                s->phase++;
                return;
            }
            if ((s->mc == MC_FETCH && s->phase == 6) || (s->mc == MC_INTACK && s->phase == 10)) {
                if (commit(sim, instance, s, LOGIC_1, 0, 0, 0, NULL)) {   /* T4 */
                    s->phase++;
                }
                return;
            }
            s->phase++;
            return;
        case MC_MREAD:
            s->phase++;   /* T2, Tw, T3 rise: nothing */
            return;
        case MC_MWRITE:
            if (s->phase == 4) {
                if (commit(sim, instance, s, LOGIC_1, 0, 0, 0, NULL)) {   /* T3 */
                    s->phase++;
                }
                return;
            }
            s->phase++;
            return;
        case MC_IOWRITE:
        case MC_IOREAD:
            if (s->phase == 2) {
                /* T2: tick 2; IORQ# and RD#/WR# low. */
                if (!commit(sim, instance, s, LOGIC_1, 0, 0, 0, NULL)) {
                    return;
                }
                set_out(sim, instance, s, K_IORQ, LOGIC_0, s->t[T_DCRIORQF]);
                if (s->mc == MC_IOWRITE) {
                    set_out(sim, instance, s, K_WR, LOGIC_0, s->t[T_DCRWRF]);
                } else {
                    set_out(sim, instance, s, K_RD, LOGIC_0, s->t[T_DCRRDF]);
                }
                s->phase++;
                return;
            }
            if (s->mc == MC_IOWRITE && s->phase == 6) {
                if (commit(sim, instance, s, LOGIC_1, 0, 0, 0, NULL)) {   /* T3: tick 4 */
                    s->phase++;
                }
                return;
            }
            s->phase++;
            return;
        case MC_NONE:
            return;
    }
}

static void mc_fall(Sim *sim, SimInstance *instance, Z80 *s) {
    uint8_t data, xdata;
    int waited;

    switch (s->mc) {
        case MC_FETCH:
            switch (s->phase) {
                case 1:   /* T1 */
                    set_out(sim, instance, s, K_MREQ, LOGIC_0, s->t[T_DCFMREQF]);
                    set_out(sim, instance, s, K_RD, LOGIC_0, s->t[T_DCFRDF]);
                    break;
                case 3:   /* T2, Tw: WAIT for the commit at the next rise */
                    s->wait_level = sample(sim, instance, s, K_WAIT, s->t[T_SWAITCF]);
                    break;
                case 5:   /* T3: the refresh strobe */
                    set_out(sim, instance, s, K_MREQ, LOGIC_0, s->t[T_DCFMREQF]);
                    break;
                case 7:   /* T4 */
                    set_out(sim, instance, s, K_MREQ, LOGIC_1, s->t[T_DCFMREQR]);
                    halt_check(sim, instance, s);
                    mc_done(s);
                    return;
            }
            s->phase++;
            return;
        case MC_INTACK:
            switch (s->phase) {
                case 5:   /* TW*1 */
                    set_out(sim, instance, s, K_IORQ, LOGIC_0, s->t[T_DCFIORQF]);
                    break;
                case 7:   /* TW*2, Tw */
                    s->wait_level = sample(sim, instance, s, K_WAIT, s->t[T_SWAITCF]);
                    break;
                case 9:   /* T3 */
                    set_out(sim, instance, s, K_MREQ, LOGIC_0, s->t[T_DCFMREQF]);
                    break;
                case 11:  /* T4 */
                    set_out(sim, instance, s, K_MREQ, LOGIC_1, s->t[T_DCFMREQR]);
                    mc_done(s);
                    return;
            }
            s->phase++;
            return;
        case MC_MREAD:
        case MC_MWRITE:
            if (s->phase == 1) {   /* T1 */
                set_out(sim, instance, s, K_MREQ, LOGIC_0, s->t[T_DCFMREQF]);
                if (s->mc == MC_MREAD) {
                    set_out(sim, instance, s, K_RD, LOGIC_0, s->t[T_DCFRDF]);
                } else {
                    set_data(sim, instance, s, s->mc_data);
                }
                s->phase++;
                return;
            }
            if (s->phase == 3) {   /* T2, Tw: tick 2 with WAIT */
                if (s->mc == MC_MWRITE && s->first_wait) {
                    set_out(sim, instance, s, K_WR, LOGIC_0, s->t[T_DCFWRF]);
                }
                s->first_wait = 0;
                s->wait_level = sample(sim, instance, s, K_WAIT, s->t[T_SWAITCF]);
                if (!commit(sim, instance, s, s->wait_level, 0, 0, 0, &waited)) {
                    return;
                }
                s->phase = waited ? 2 : 4;
                return;
            }
            if (s->phase == 5) {   /* T3 */
                if (s->mc == MC_MREAD) {
                    sample_data(sim, instance, s, s->t[T_SDCF], &data, &xdata);
                    if (!commit(sim, instance, s, LOGIC_1, 1, data, xdata, NULL)) {
                        return;
                    }
                    cpu_tracker_read(&s->tracker, s->mc_addr, data);
                    set_out(sim, instance, s, K_RD, LOGIC_1, s->t[T_DCFRDR]);
                } else {
                    set_out(sim, instance, s, K_WR, LOGIC_1, s->t[T_DCFWRR]);
                    s->release_data = 1;
                }
                set_out(sim, instance, s, K_MREQ, LOGIC_1, s->t[T_DCFMREQR]);
                mc_done(s);
                return;
            }
            s->phase++;
            return;
        case MC_IOWRITE:
        case MC_IOREAD:
            if (s->phase == 1) {   /* T1 */
                if (s->mc == MC_IOWRITE) {
                    set_data(sim, instance, s, s->mc_data);
                }
                s->phase++;
                return;
            }
            if (s->phase == 5) {   /* TW*, Tw: tick 3 with WAIT */
                s->wait_level = sample(sim, instance, s, K_WAIT, s->t[T_SWAITCF]);
                if (!commit(sim, instance, s, s->wait_level, 0, 0, 0, &waited)) {
                    return;
                }
                s->phase = waited ? 4 : 6;
                return;
            }
            if (s->phase == 7) {   /* T3 */
                if (s->mc == MC_IOREAD) {
                    sample_data(sim, instance, s, s->t[T_SDCF], &data, &xdata);
                    if (!commit(sim, instance, s, LOGIC_1, 1, data, xdata, NULL)) {
                        return;
                    }
                    set_out(sim, instance, s, K_RD, LOGIC_1, s->t[T_DCFRDR]);
                } else {
                    set_out(sim, instance, s, K_WR, LOGIC_1, s->t[T_DCFWRR]);
                    s->release_data = 1;
                }
                set_out(sim, instance, s, K_IORQ, LOGIC_1, s->t[T_DCFIORQR]);
                mc_done(s);
                return;
            }
            s->phase++;
            return;
        case MC_NONE:
            return;
    }
}

/* ---- Clock edges ------------------------------------------------------------ */

/* NMI#: a fall is kept until the next rise passes it on; a pulse shorter
   than tWNMI reads X. */
static LogicValue nmi_sample(const Sim *sim, const SimInstance *instance, Z80 *s) {
    LogicValue level = sample(sim, instance, s, K_NMI, 0);
    LogicValue result = level;

    if (s->nmi_short) {
        result = LOGIC_X;
    } else if (s->nmi_pending && level != LOGIC_X) {
        result = LOGIC_0;
    }
    s->nmi_short = 0;
    s->nmi_pending = 0;
    return result;
}

static void clock_rise(Sim *sim, SimInstance *instance, Z80 *s) {
    LogicValue reset = cpu_sample_reset(sim, instance, s->ctl[K_RESET], s->changed[s->ctl[K_RESET]],
                                        s->seen[s->ctl[K_RESET]], s->t[T_SRESET],
                                        s->status == CPU_RESET);
    LogicValue busreq;

    s->int_level = sample(sim, instance, s, K_INT, s->t[T_SINT]);
    s->nmi_level = nmi_sample(sim, instance, s);
    busreq = sample(sim, instance, s, K_BUSREQ, s->t[T_SBUSREQ]);
    if (reset == LOGIC_X) {
        enter_lost(sim, instance, s);
        return;
    }
    if (reset == LOGIC_0) {
        if (s->status != CPU_RESET) {
            enter_reset(sim, instance, s);
        }
        s->reset_count++;
        return;
    }
    if (s->status == CPU_RESET) {
        if (s->reset_count < 3) {
            enter_lost(sim, instance, s);
            return;
        }
        s->pins = z80_reset(&s->core);
        s->status = CPU_RUNNING;
        s->cycles = 0;
        s->busreq_last = LOGIC_1;
        cpu_tracker_reset(&s->tracker, z80dasm_op);
    }
    if (s->status == CPU_LOST) {
        return;
    }
    s->cycles++;
    if (busreq == LOGIC_X) {
        enter_lost(sim, instance, s);
        return;
    }
    if (s->released) {
        if (s->resume) {
            /* BUSACK# went high at the last fall: drive the bus again. */
            s->released = 0;
            s->resume = 0;
            set_out(sim, instance, s, K_MREQ, LOGIC_1, s->t[T_DCRCTZ]);
            set_out(sim, instance, s, K_IORQ, LOGIC_1, s->t[T_DCRCTZ]);
            set_out(sim, instance, s, K_RD, LOGIC_1, s->t[T_DCRCTZ]);
            set_out(sim, instance, s, K_WR, LOGIC_1, s->t[T_DCRCTZ]);
            s->busreq_last = busreq;
            status_after_tick(s);
            mc_start(sim, instance, s);
            return;
        }
        if (busreq == LOGIC_1) {
            s->resume = 1;
        }
        s->busreq_last = busreq;
        return;
    }
    if (s->mc != MC_NONE) {
        mc_rise(sim, instance, s);
    } else {
        mc_start(sim, instance, s);
    }
    s->busreq_last = busreq;
}

static void clock_fall(Sim *sim, SimInstance *instance, Z80 *s) {
    if (s->status == CPU_LOST || s->status == CPU_RESET) {
        return;
    }
    if (s->halt_release) {
        set_out(sim, instance, s, K_HALT, LOGIC_1, s->t[T_DCFHALT]);
        s->halt_release = 0;
    }
    if (s->released) {
        if (s->resume) {
            set_out(sim, instance, s, K_BUSACK, LOGIC_1, s->t[T_DCFBUSACKR]);
        }
        return;
    }
    mc_fall(sim, instance, s);
}

static void z80_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                            LogicValue old_value, LogicValue new_value) {
    Z80 *s = raw;
    SimTime now = sim_now(sim);

    (void)old_value;
    if (pin < CPU_MAX_PINS) {
        s->changed[pin] = now;
        s->seen[pin] = new_value;
    }
    if (pin == s->ctl[K_CLOCK]) {
        LogicValue level = new_value == LOGIC_0 || new_value == LOGIC_1 ? new_value : LOGIC_X;

        if (level == LOGIC_X) {
            s->clock = LOGIC_X;
            s->last_rise = CPU_NEVER;
            s->last_fall = CPU_NEVER;
            enter_lost(sim, instance, s);
            return;
        }
        if (level == s->clock) {
            return;
        }
        s->clock = level;
        if (level == LOGIC_1) {
            int ok = (s->last_fall == CPU_NEVER || now - s->last_fall >= s->t[T_WCL]) &&
                     (s->last_rise == CPU_NEVER || now - s->last_rise >= s->t[T_CC]);

            s->last_rise = now;
            if (!ok) {
                enter_lost(sim, instance, s);
            }
            clock_rise(sim, instance, s);
        } else {
            int ok = s->last_rise == CPU_NEVER || now - s->last_rise >= s->t[T_WCH];

            s->last_fall = now;
            if (!ok) {
                enter_lost(sim, instance, s);
                return;
            }
            clock_fall(sim, instance, s);
        }
    } else if (pin == s->ctl[K_NMI]) {
        LogicValue level = cpu_level(new_value, LOGIC_1);

        if (level == LOGIC_0) {
            s->nmi_pending = 1;
            s->nmi_fell = now;
        } else if (level == LOGIC_1 && s->nmi_pending && now - s->nmi_fell < s->t[T_WNMI]) {
            s->nmi_short = 1;
        }
    }
}

/* ---- Model --------------------------------------------------------------- */

static void *z80_create(const Part *part, const CpuPins *pins, const CpuGrade *grade) {
    Z80 *s = calloc(1, sizeof(*s));
    size_t i;

    (void)part;
    if (s == NULL) {
        return NULL;
    }
    s->base.ops = &CPUZ80_OPS;
    s->t = grade->t;
    for (i = 0; i < CPU_ADDRESS_BITS; i++) {
        s->addr_pin[i] = pins->address[i];
    }
    for (i = 0; i < CPU_DATA_BITS; i++) {
        s->data_pin[i] = pins->data[i];
    }
    for (i = 0; i < K_COUNT; i++) {
        s->ctl[i] = pins->control[i];
    }
    s->status = CPU_LOST;
    s->clock = LOGIC_X;
    s->last_rise = CPU_NEVER;
    s->last_fall = CPU_NEVER;
    s->int_level = LOGIC_1;
    s->nmi_level = LOGIC_1;
    s->busreq_last = LOGIC_1;
    for (i = 0; i < CPU_MAX_PINS; i++) {
        s->changed[i] = CPU_NEVER;
        s->seen[i] = LOGIC_Z;
    }
    cpu_tracker_reset(&s->tracker, z80dasm_op);
    return s;
}

static void z80_destroy(void *state) {
    free(state);
}

/* Power-up: LOST, every output X. */
static void z80_start(Sim *sim, SimInstance *instance, void *raw) {
    Z80 *s = raw;

    s->status = CPU_RESET;   /* so all_outputs runs through enter_lost */
    enter_lost(sim, instance, s);
}

static int z80_state(const void *raw, CpuState *out) {
    const Z80 *s = raw;
    const z80_t *c = &s->core;

    out->status = s->status;
    out->cycles = s->cycles;
    if (s->status != CPU_LOST && s->status != CPU_RESET) {
        const CpuRegister regs[] = {
            {"AF", c->af, 16}, {"BC", c->bc, 16}, {"DE", c->de, 16}, {"HL", c->hl, 16},
            {"AF'", c->af2, 16}, {"BC'", c->bc2, 16}, {"DE'", c->de2, 16}, {"HL'", c->hl2, 16},
            {"IX", c->ix, 16}, {"IY", c->iy, 16}, {"SP", c->sp, 16}, {"PC", c->pc, 16},
            {"I", c->i, 8}, {"R", c->r, 8}, {"WZ", c->wz, 16}, {"IM", c->im, 2},
            {"IFF1", c->iff1, 1}, {"IFF2", c->iff2, 1},
        };

        memcpy(out->registers, regs, sizeof regs);
        out->register_count = sizeof regs / sizeof regs[0];
    }
    cpu_tracker_fill(&s->tracker, out);
    return 1;
}

const CpuCoreOps CPUZ80_OPS = {
    z80_create, z80_destroy, z80_start, z80_pin_changed, NULL, z80_state
};
