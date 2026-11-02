/*
 * cpu6502: the 6502-family bus adapter (plan-09-B sections 4.4-4.5), for
 * "core": "6502" (NMOS, vendored m6502.h) and "65c02" (WDC W65C02S,
 * vendored w65c02.h).
 *
 * The core is ticked once per PHI2 fall (the commit): it takes the data
 * sampled then and returns the next bus cycle, which the adapter drives at
 * the datasheet's times: address and R/W X at fall + tAH, valid at
 * fall + tADS; SYNC valid at fall + tSYNC; write data X at the rise and
 * valid at rise + tMDS, Z at fall + tDHW.
 *
 * The CPU starts LOST (every output X) and only a reset (RES low for at
 * least 2 cycles) leaves it. It goes LOST again on a clock that is X or too
 * fast, an X on RES or RDY, or an unknown input that changes its state: an
 * input sampled less than its setup time after it changed counts as X, and
 * a tick with X inputs runs twice (X bits 0, then 1); different results
 * make the CPU lost (the differential rule).
 *
 * RDY low holds a read cycle (and, on the 65C02, a write cycle) without a
 * tick. SO falling sets V. The 65C02 adds VPB, MLB, BE (bus Z after tBVD)
 * and pulls RDY low while WAI waits; STP stops it until reset. The NMOS JAM
 * opcodes hang it (JAMMED).
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

#include "../third_party/chips/m6502.h"
#include "../third_party/chips/m6502dasm.h"
#include "../third_party/chips/w65c02.h"
#include "../third_party/chips/w65c02dasm.h"

/* Control order (cpu.c CONTROLS_6502 / CONTROLS_65C02). */
enum { K_CLOCK, K_RW, K_SYNC, K_IRQ, K_NMI, K_RESET, K_SO, K_RDY, K_PHI1, K_PHI2, K_VPB, K_MLB,
       K_BE, K_COUNT };
/* Timing order (cpu.c TIMING_6502). */
enum { T_CYC, T_PWH, T_PWL, T_ADS, T_AH, T_SYNC, T_MDS, T_DHW, T_DSR, T_PCS, T_BVD };
/* The bus pins BE floats: A0-A15, D0-D7, R/W. */
enum { BUS_DATA = 16, BUS_RW = 24, BUS_COUNT = 25 };

/* The two cores share the pin layout (M6502_* == W65C02_*). */
#define PIN_DATA 0xFF0000ull
#define PIN_RW M6502_RW
#define PIN_SYNC M6502_SYNC
#define PIN_IRQ M6502_IRQ
#define PIN_NMI M6502_NMI
#define PIN_RDY M6502_RDY
#define PIN_RES M6502_RES

typedef union {
    m6502_t m6502;
    w65c02_t w65c02;
} Core;

typedef struct {
    CpuBase base;
    int wdc;                       /* 65c02 */
    const SimTime *t;
    size_t bus_pin[BUS_COUNT];
    size_t ctl[K_COUNT];
    Core core;
    uint64_t pins;                 /* the current bus cycle (the last tick's result) */
    CpuStatus status;
    uint64_t cycles;
    unsigned reset_cycles;
    LogicValue clock;
    SimTime last_rise, last_fall;  /* CPU_NEVER: none since the clock was last known */
    SimTime changed[CPU_MAX_PINS];
    LogicValue seen[CPU_MAX_PINS];  /* the value pin_changed last reported */
    int so_fell;
    LogicValue be;                 /* BE level; 1 on the NMOS chip */
    LogicValue bus[BUS_COUNT];     /* what each bus pin carries once its drives land */
    LogicValue sync_out, vpb_out, mlb_out;
    unsigned mlb_left;             /* cycles of a read-modify-write still to come */
    int rdy_driven;                /* WAI pulls RDY low */
    int jammed;
    CpuTracker tracker;
} Cpu65;

/* ---- The core ------------------------------------------------------------ */

static uint64_t core_init(const Cpu65 *s, Core *c) {
    memset(c, 0, sizeof(*c));
    if (s->wdc) {
        return w65c02_init(&c->w65c02, &(w65c02_desc_t){0});
    }
    return m6502_init(&c->m6502, &(m6502_desc_t){0});
}

static uint64_t core_tick(const Cpu65 *s, Core *c, uint64_t pins) {
    return s->wdc ? w65c02_tick(&c->w65c02, pins) : m6502_tick(&c->m6502, pins);
}

static unsigned core_opcode(const Cpu65 *s, const Core *c) {
    return (unsigned)((s->wdc ? c->w65c02.IR : c->m6502.IR) >> 3) & 0xFF;
}

static uint8_t *core_p(const Cpu65 *s, Core *c) {
    return s->wdc ? &c->w65c02.P : &c->m6502.P;
}

static int core_frozen(const Cpu65 *s, const Core *c) {
    return s->wdc && (c->w65c02.wait_flag || c->w65c02.stop_flag);
}

/* Equal state, ignoring the data and IRQ bits of the stored pins. */
static int same_core(const Cpu65 *s, const Core *a, const Core *b) {
    const uint64_t mask = ~(PIN_DATA | PIN_IRQ);

    if (s->wdc) {
        const w65c02_t *x = &a->w65c02;
        const w65c02_t *y = &b->w65c02;

        return x->IR == y->IR && x->PC == y->PC && x->AD == y->AD && x->A == y->A &&
               x->X == y->X && x->Y == y->Y && x->S == y->S && x->P == y->P &&
               (x->PINS & mask) == (y->PINS & mask) && x->irq_pip == y->irq_pip &&
               x->nmi_pip == y->nmi_pip && x->brk_flags == y->brk_flags &&
               x->wait_flag == y->wait_flag && x->stop_flag == y->stop_flag;
    } else {
        const m6502_t *x = &a->m6502;
        const m6502_t *y = &b->m6502;

        return x->IR == y->IR && x->PC == y->PC && x->AD == y->AD && x->A == y->A &&
               x->X == y->X && x->Y == y->Y && x->S == y->S && x->P == y->P &&
               (x->PINS & mask) == (y->PINS & mask) && x->irq_pip == y->irq_pip &&
               x->nmi_pip == y->nmi_pip && x->brk_flags == y->brk_flags &&
               x->bcd_enabled == y->bcd_enabled;
    }
}

static int is_jam(unsigned opcode) {
    static const uint8_t JAM[] = {0x02, 0x12, 0x22, 0x32, 0x42, 0x52, 0x62, 0x72, 0x92, 0xB2,
                                  0xD2, 0xF2};
    size_t i;

    for (i = 0; i < sizeof JAM; i++) {
        if (JAM[i] == opcode) {
            return 1;
        }
    }
    return 0;
}

/* The W65C02S read-modify-write opcodes (plan-09-B section 4.5). */
static int is_rmw(unsigned opcode) {
    switch (opcode) {
        case 0x06: case 0x0E: case 0x16: case 0x1E:   /* ASL */
        case 0x26: case 0x2E: case 0x36: case 0x3E:   /* ROL */
        case 0x46: case 0x4E: case 0x56: case 0x5E:   /* LSR */
        case 0x66: case 0x6E: case 0x76: case 0x7E:   /* ROR */
        case 0xC6: case 0xCE: case 0xD6: case 0xDE:   /* DEC */
        case 0xE6: case 0xEE: case 0xF6: case 0xFE:   /* INC */
        case 0x04: case 0x0C: case 0x14: case 0x1C:   /* TSB, TRB */
            return 1;
    }
    return (opcode & 0x0F) == 0x07;                   /* RMB, SMB */
}

/* ---- Drives -------------------------------------------------------------- */

static LogicValue bit_value(uint64_t pins, uint64_t bit) {
    return (pins & bit) ? LOGIC_1 : LOGIC_0;
}

/* A bus pin, gated by BE: the value is remembered, and driven only while BE
   is high. */
static void bus_put(Sim *sim, SimInstance *instance, Cpu65 *s, size_t index, LogicValue value,
                    SimTime delay, int inertial) {
    s->bus[index] = value;
    if (s->be != LOGIC_1) {
        return;
    }
    if (inertial) {
        sim_schedule_inertial_drive(sim, instance, s->bus_pin[index], value, DRIVE_STRONG, delay);
    } else {
        sim_schedule_drive(sim, instance, s->bus_pin[index], value, DRIVE_STRONG, delay);
    }
}

static void out_inertial(Sim *sim, SimInstance *instance, Cpu65 *s, int key, LogicValue *out,
                         LogicValue value, SimTime delay) {
    *out = value;
    sim_schedule_inertial_drive(sim, instance, s->ctl[key], value, DRIVE_STRONG, delay);
}

static void release_rdy(Sim *sim, SimInstance *instance, Cpu65 *s) {
    if (s->rdy_driven) {
        sim_schedule_drive(sim, instance, s->ctl[K_RDY], LOGIC_Z, DRIVE_STRONG, 0);
        s->rdy_driven = 0;
    }
}

/* Lost: every bus output X now (Z while BE is low), until a reset. */
static void enter_lost(Sim *sim, SimInstance *instance, Cpu65 *s) {
    size_t i;

    if (s->status == CPU_LOST) {
        return;
    }
    s->status = CPU_LOST;
    s->mlb_left = 0;
    for (i = 0; i < BUS_COUNT; i++) {
        bus_put(sim, instance, s, i, LOGIC_X, 0, 1);
    }
    out_inertial(sim, instance, s, K_SYNC, &s->sync_out, LOGIC_X, 0);
    if (s->wdc) {
        out_inertial(sim, instance, s, K_VPB, &s->vpb_out, LOGIC_X, 0);
        out_inertial(sim, instance, s, K_MLB, &s->mlb_out, LOGIC_X, 0);
    }
    release_rdy(sim, instance, s);
}

/* RES held: address X, data Z, R/W 1, SYNC 0, VPB 1, MLB 1 from tAH. */
static void enter_reset(Sim *sim, SimInstance *instance, Cpu65 *s) {
    size_t i;
    SimTime tah = s->t[T_AH];

    s->status = CPU_RESET;
    s->reset_cycles = 0;
    s->mlb_left = 0;
    for (i = 0; i < BUS_DATA; i++) {
        bus_put(sim, instance, s, i, LOGIC_X, tah, 1);
    }
    for (i = BUS_DATA; i < BUS_RW; i++) {
        bus_put(sim, instance, s, i, LOGIC_Z, tah, 1);
    }
    bus_put(sim, instance, s, BUS_RW, LOGIC_1, tah, 1);
    out_inertial(sim, instance, s, K_SYNC, &s->sync_out, LOGIC_0, tah);
    if (s->wdc) {
        out_inertial(sim, instance, s, K_VPB, &s->vpb_out, LOGIC_1, tah);
        out_inertial(sim, instance, s, K_MLB, &s->mlb_out, LOGIC_1, tah);
    }
    release_rdy(sim, instance, s);
}

/* An output that changes: X at tAH, the value at `valid` (transport). */
static void out_change(Sim *sim, SimInstance *instance, Cpu65 *s, int key, LogicValue *out,
                       LogicValue value, SimTime valid) {
    if (*out != value) {
        sim_schedule_drive(sim, instance, s->ctl[key], LOGIC_X, DRIVE_STRONG, s->t[T_AH]);
        sim_schedule_drive(sim, instance, s->ctl[key], value, DRIVE_STRONG, valid);
        *out = value;
    }
}

/* A 65C02 strobe (VPB, MLB): the value at tADS. */
static void out_strobe(Sim *sim, SimInstance *instance, Cpu65 *s, int key, LogicValue *out,
                       LogicValue value) {
    if (*out != value) {
        sim_schedule_drive(sim, instance, s->ctl[key], value, DRIVE_STRONG, s->t[T_ADS]);
        *out = value;
    }
}

/* Drives the bus cycle in s->pins, from a commit. */
static void drive_cycle(Sim *sim, SimInstance *instance, Cpu65 *s) {
    uint64_t pins = s->pins;
    size_t i;

    for (i = 0; i < BUS_DATA; i++) {
        LogicValue v = bit_value(pins, 1ull << i);

        if (s->bus[i] != v) {
            bus_put(sim, instance, s, i, LOGIC_X, s->t[T_AH], 0);
            bus_put(sim, instance, s, i, v, s->t[T_ADS], 0);
        }
    }
    if (s->bus[BUS_RW] != bit_value(pins, PIN_RW)) {
        bus_put(sim, instance, s, BUS_RW, LOGIC_X, s->t[T_AH], 0);
        bus_put(sim, instance, s, BUS_RW, bit_value(pins, PIN_RW), s->t[T_ADS], 0);
    }
    out_change(sim, instance, s, K_SYNC, &s->sync_out, bit_value(pins, PIN_SYNC), s->t[T_SYNC]);
}

/* VPB and MLB for the cycle in s->pins (65C02). */
static void drive_strobes(Sim *sim, SimInstance *instance, Cpu65 *s) {
    uint64_t pins = s->pins;
    uint16_t addr = M6502_GET_ADDR(pins);
    int read = (pins & PIN_RW) != 0;
    unsigned opcode = core_opcode(s, &s->core);
    LogicValue mlb = LOGIC_1;

    out_strobe(sim, instance, s, K_VPB, &s->vpb_out,
               read && addr >= 0xFFFA && opcode == 0x00 ? LOGIC_0 : LOGIC_1);
    if (s->mlb_left > 0) {
        s->mlb_left--;
        mlb = LOGIC_0;
    } else if (read && is_rmw(opcode)) {
        Core peek = s->core;
        uint64_t p1 = core_tick(s, &peek, pins);
        uint64_t p2 = core_tick(s, &peek, p1);

        if ((p1 & PIN_RW) && M6502_GET_ADDR(p1) == addr && !(p2 & PIN_RW) &&
            M6502_GET_ADDR(p2) == addr) {
            s->mlb_left = 2;
            mlb = LOGIC_0;
        }
    }
    out_strobe(sim, instance, s, K_MLB, &s->mlb_out, mlb);
}

/* ---- The commit (PHI2 fall) ---------------------------------------------- */

static LogicValue sample(const Sim *sim, const SimInstance *instance, const Cpu65 *s, int key,
                         SimTime setup) {
    size_t pin = s->ctl[key];

    return cpu_sample(sim, instance, pin, s->changed[pin], s->seen[pin], setup, LOGIC_1);
}

/* After a tick: status, the tracker, and WAI's RDY drive. */
static void after_tick(Sim *sim, SimInstance *instance, Cpu65 *s, int was_frozen) {
    int frozen = core_frozen(s, &s->core);

    if (s->wdc) {
        s->status = s->core.w65c02.stop_flag ? CPU_STOPPED
                  : s->core.w65c02.wait_flag ? CPU_WAITING : CPU_RUNNING;
        if (s->core.w65c02.wait_flag && !s->rdy_driven) {
            sim_schedule_drive(sim, instance, s->ctl[K_RDY], LOGIC_0, DRIVE_STRONG, 0);
            s->rdy_driven = 1;
        } else if (!s->core.w65c02.wait_flag) {
            release_rdy(sim, instance, s);
        }
    } else if (is_jam(core_opcode(s, &s->core))) {
        s->status = CPU_JAMMED;
        if (!s->jammed) {
            cpu_tracker_finish(&s->tracker);
            s->jammed = 1;
        }
    } else {
        s->status = CPU_RUNNING;
    }
    if ((s->pins & PIN_SYNC) && !(was_frozen && frozen) && s->status != CPU_JAMMED) {
        cpu_tracker_begin(&s->tracker, M6502_GET_ADDR(s->pins));
    }
}

/* RES released after a reset: a fresh core, ticked into its reset sequence. */
static void start_core(Sim *sim, SimInstance *instance, Cpu65 *s) {
    uint64_t pins = core_init(s, &s->core);

    s->pins = core_tick(s, &s->core, pins);
    s->cycles = 0;
    s->so_fell = 0;
    s->mlb_left = 0;
    s->jammed = 0;
    cpu_tracker_reset(&s->tracker, s->wdc ? w65c02dasm_op : m6502dasm_op);
    drive_cycle(sim, instance, s);
    if (s->wdc) {
        drive_strobes(sim, instance, s);
    }
    after_tick(sim, instance, s, 0);
}

static void commit(Sim *sim, SimInstance *instance, Cpu65 *s) {
    int read = (s->pins & PIN_RW) != 0;
    int active = s->status != CPU_LOST && s->status != CPU_RESET;
    LogicValue res = cpu_sample_reset(sim, instance, s->ctl[K_RESET], s->changed[s->ctl[K_RESET]],
                                      s->seen[s->ctl[K_RESET]], s->t[T_PCS],
                                      s->status == CPU_RESET);
    LogicValue level;
    uint64_t in;
    uint64_t xmask = 0;
    uint8_t data = 0;
    int was_frozen;
    size_t i;

    /* 1. Write release. */
    if (active && !read) {
        for (i = BUS_DATA; i < BUS_RW; i++) {
            bus_put(sim, instance, s, i, LOGIC_Z, s->t[T_DHW], 0);
        }
    }
    /* 2. Reset. */
    if (res == LOGIC_X) {
        enter_lost(sim, instance, s);
        return;
    }
    if (res == LOGIC_0) {
        if (s->status != CPU_RESET) {
            enter_reset(sim, instance, s);
        }
        s->reset_cycles++;
        return;
    }
    if (s->status == CPU_RESET) {
        if (s->reset_cycles >= 2) {
            start_core(sim, instance, s);
        } else {
            enter_lost(sim, instance, s);
        }
        return;
    }
    /* 3. Only a reset leaves LOST. */
    if (s->status == CPU_LOST) {
        return;
    }
    /* 4. RDY (not while WAI pulls it low itself). */
    if (s->status != CPU_WAITING) {
        level = sample(sim, instance, s, K_RDY, s->t[T_PCS]);
        if (level == LOGIC_X) {
            enter_lost(sim, instance, s);
            return;
        }
        if (level == LOGIC_0 && (read || s->wdc)) {
            s->cycles++;
            return;
        }
    }
    /* 5. SO: a fall since the last commit sets V. */
    level = sample(sim, instance, s, K_SO, s->t[T_PCS]);
    if (s->so_fell) {
        *core_p(s, &s->core) |= M6502_VF;
        s->so_fell = 0;
    }
    if (level == LOGIC_X && !(*core_p(s, &s->core) & M6502_VF)) {
        enter_lost(sim, instance, s);
        return;
    }
    /* 6. The tick, with the differential rule. */
    in = s->pins & ~(PIN_DATA | PIN_IRQ | PIN_NMI | PIN_RDY | PIN_RES);
    if (read) {
        for (i = 0; i < CPU_DATA_BITS; i++) {
            size_t pin = s->bus_pin[BUS_DATA + i];

            level = cpu_sample(sim, instance, pin, s->changed[pin], s->seen[pin], s->t[T_DSR],
                               LOGIC_X);
            if (level == LOGIC_X) {
                xmask |= 1ull << (16 + i);
            } else if (level == LOGIC_1) {
                data |= (uint8_t)(1u << i);
            }
        }
        in |= (uint64_t)data << 16;
        cpu_tracker_read(&s->tracker, M6502_GET_ADDR(s->pins), data);
    }
    level = sample(sim, instance, s, K_IRQ, s->t[T_PCS]);
    if (level == LOGIC_X) {
        xmask |= PIN_IRQ;
    } else if (level == LOGIC_0) {
        in |= PIN_IRQ;
    }
    level = sample(sim, instance, s, K_NMI, s->t[T_PCS]);
    if (level == LOGIC_X) {
        xmask |= PIN_NMI;
    } else if (level == LOGIC_0) {
        in |= PIN_NMI;
    }
    was_frozen = core_frozen(s, &s->core);
    if (xmask == 0) {
        s->pins = core_tick(s, &s->core, in);
    } else {
        Core a = s->core;
        Core b = s->core;
        uint64_t pa = core_tick(s, &a, in);
        uint64_t pb = core_tick(s, &b, in | xmask);
        uint64_t mask = ~(PIN_IRQ | PIN_NMI);

        if (pa & PIN_RW) {
            mask &= ~PIN_DATA;   /* a read cycle passes the input data through */
        }
        if (!same_core(s, &a, &b) || (pa & mask) != (pb & mask)) {
            enter_lost(sim, instance, s);
            return;
        }
        s->core = a;
        s->pins = pa;
    }
    s->cycles++;
    /* 7. Drive the next cycle; 8. status. */
    drive_cycle(sim, instance, s);
    if (s->wdc) {
        drive_strobes(sim, instance, s);
    }
    after_tick(sim, instance, s, was_frozen);
}

/* ---- Pins ------------------------------------------------------------------ */

static void clock_rise(Sim *sim, SimInstance *instance, Cpu65 *s, SimTime now) {
    size_t i;

    if (s->last_fall != CPU_NEVER && now - s->last_fall < s->t[T_PWL]) {
        enter_lost(sim, instance, s);
    }
    s->last_rise = now;
    if (s->status != CPU_LOST && s->status != CPU_RESET && !(s->pins & PIN_RW)) {
        uint8_t data = M6502_GET_DATA(s->pins);

        for (i = 0; i < CPU_DATA_BITS; i++) {
            bus_put(sim, instance, s, BUS_DATA + i, LOGIC_X, 0, 0);
            bus_put(sim, instance, s, BUS_DATA + i, (data >> i) & 1 ? LOGIC_1 : LOGIC_0,
                    s->t[T_MDS], 0);
        }
    }
}

static void clock_fall(Sim *sim, SimInstance *instance, Cpu65 *s, SimTime now) {
    int ok = 1;

    if (s->last_rise != CPU_NEVER && now - s->last_rise < s->t[T_PWH]) {
        ok = 0;
    }
    if (s->last_fall != CPU_NEVER && now - s->last_fall < s->t[T_CYC]) {
        ok = 0;
    }
    s->last_fall = now;
    if (!ok) {
        enter_lost(sim, instance, s);
        return;
    }
    commit(sim, instance, s);
}

/* BE (65C02): the bus floats, or returns, tBVD after a change. */
static void bus_enable(Sim *sim, SimInstance *instance, Cpu65 *s, LogicValue level) {
    size_t i;

    if (level == s->be) {
        return;
    }
    s->be = level;
    for (i = 0; i < BUS_COUNT; i++) {
        LogicValue v = level == LOGIC_0 ? LOGIC_Z : level == LOGIC_1 ? s->bus[i] : LOGIC_X;

        sim_schedule_inertial_drive(sim, instance, s->bus_pin[i], v, DRIVE_STRONG, s->t[T_BVD]);
    }
}

static void c65_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                            LogicValue old_value, LogicValue new_value) {
    Cpu65 *s = raw;
    SimTime now = sim_now(sim);

    if (pin < CPU_MAX_PINS) {
        s->changed[pin] = now;
        s->seen[pin] = new_value;
    }
    if (pin == s->ctl[K_CLOCK]) {
        LogicValue level = new_value == LOGIC_0 || new_value == LOGIC_1 ? new_value : LOGIC_X;

        sim_schedule_drive(sim, instance, s->ctl[K_PHI2], level, DRIVE_STRONG, 0);
        sim_schedule_drive(sim, instance, s->ctl[K_PHI1],
                           level == LOGIC_X ? LOGIC_X : level == LOGIC_1 ? LOGIC_0 : LOGIC_1,
                           DRIVE_STRONG, 0);
        if (level == LOGIC_X) {
            s->clock = LOGIC_X;
            s->last_rise = CPU_NEVER;
            s->last_fall = CPU_NEVER;
            enter_lost(sim, instance, s);
        } else if (level != s->clock) {
            s->clock = level;
            if (level == LOGIC_1) {
                clock_rise(sim, instance, s, now);
            } else {
                clock_fall(sim, instance, s, now);
            }
        }
    } else if (pin == s->ctl[K_SO]) {
        if (cpu_level(new_value, LOGIC_1) == LOGIC_0 && cpu_level(old_value, LOGIC_1) != LOGIC_0) {
            s->so_fell = 1;
        }
    } else if (s->wdc && pin == s->ctl[K_BE]) {
        bus_enable(sim, instance, s, cpu_level(new_value, LOGIC_1));
    }
}

/* ---- Model ------------------------------------------------------------------ */

static void *c65_create(const Part *part, const CpuPins *pins, const CpuGrade *grade) {
    Cpu65 *s = calloc(1, sizeof(*s));
    size_t i;

    if (s == NULL) {
        return NULL;
    }
    s->base.ops = &CPU6502_OPS;
    s->wdc = strcmp(part->cpu->core, "65c02") == 0;
    s->t = grade->t;
    for (i = 0; i < CPU_ADDRESS_BITS; i++) {
        s->bus_pin[i] = pins->address[i];
    }
    for (i = 0; i < CPU_DATA_BITS; i++) {
        s->bus_pin[BUS_DATA + i] = pins->data[i];
    }
    s->bus_pin[BUS_RW] = pins->control[K_RW];
    for (i = 0; i < pins->device->control_count && i < K_COUNT; i++) {
        s->ctl[i] = pins->control[i];
    }
    s->status = CPU_LOST;
    s->clock = LOGIC_X;
    s->last_rise = CPU_NEVER;
    s->last_fall = CPU_NEVER;
    s->be = LOGIC_1;
    for (i = 0; i < CPU_MAX_PINS; i++) {
        s->changed[i] = CPU_NEVER;
        s->seen[i] = LOGIC_Z;   /* a net with no drive yet */
    }
    cpu_tracker_reset(&s->tracker, s->wdc ? w65c02dasm_op : m6502dasm_op);
    return s;
}

static void c65_destroy(void *state) {
    free(state);
}

/* Power-up: LOST, every bus output X. */
static void c65_start(Sim *sim, SimInstance *instance, void *raw) {
    Cpu65 *s = raw;
    size_t i;

    for (i = 0; i < BUS_COUNT; i++) {
        bus_put(sim, instance, s, i, LOGIC_X, 0, 0);
    }
    s->sync_out = LOGIC_X;
    sim_schedule_drive(sim, instance, s->ctl[K_SYNC], LOGIC_X, DRIVE_STRONG, 0);
    if (s->wdc) {
        s->vpb_out = LOGIC_X;
        s->mlb_out = LOGIC_X;
        sim_schedule_drive(sim, instance, s->ctl[K_VPB], LOGIC_X, DRIVE_STRONG, 0);
        sim_schedule_drive(sim, instance, s->ctl[K_MLB], LOGIC_X, DRIVE_STRONG, 0);
    }
}

static int c65_state(const void *raw, CpuState *out) {
    const Cpu65 *s = raw;

    out->status = s->status;
    out->cycles = s->cycles;
    if (s->status != CPU_LOST && s->status != CPU_RESET) {
        const Core *c = &s->core;
        uint8_t a = s->wdc ? c->w65c02.A : c->m6502.A;
        uint8_t x = s->wdc ? c->w65c02.X : c->m6502.X;
        uint8_t y = s->wdc ? c->w65c02.Y : c->m6502.Y;
        uint8_t sp = s->wdc ? c->w65c02.S : c->m6502.S;
        uint8_t p = s->wdc ? c->w65c02.P : c->m6502.P;
        uint16_t pc = s->wdc ? c->w65c02.PC : c->m6502.PC;

        out->registers[0] = (CpuRegister){"A", a, 8};
        out->registers[1] = (CpuRegister){"X", x, 8};
        out->registers[2] = (CpuRegister){"Y", y, 8};
        out->registers[3] = (CpuRegister){"S", sp, 8};
        out->registers[4] = (CpuRegister){"P", p, 8};
        out->registers[5] = (CpuRegister){"PC", pc, 16};
        out->register_count = 6;
    }
    cpu_tracker_fill(&s->tracker, out);
    return 1;
}

const CpuCoreOps CPU6502_OPS = {
    c65_create, c65_destroy, c65_start, c65_pin_changed, NULL, c65_state
};
