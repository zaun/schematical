/*
 * cpu65816: the WDC W65C816S bus adapter (plan-14-H), for "core": "65816"
 * on the vendored jsmooch core (third_party/jsmooch, README-jsmooch.md).
 *
 * The core is ticked once per PHI2 fall (the commit): it takes the data
 * sampled then and presents the next bus cycle, which the adapter drives at
 * the datasheet's times (Table 4-2, Figure 4-1):
 *
 *   A0-A15, RWB, VDA, VPA, VPB, MLB  X at fall + tAH, valid at + tADS
 *   D0-D7 (PHI2 low)                  the bank address: X at fall + tDHW,
 *                                     valid at + tBAS
 *   D0-D7 (PHI2 high)                 read: Z at rise + tBH; write: X at
 *                                     rise + tBH, data at + tMDS, held to the
 *                                     next fall + tDHW
 *   E                                 at fall + tEH when it changes
 *   MX                                M from rise + tEH, X from fall + tEH
 *
 * Read data is sampled at the fall (setup tDSR). The CPU starts LOST (every
 * output X) and only a reset (RESB low for at least 2 cycles) leaves it; it
 * goes LOST again on a clock that is X or too fast, an X on RESB, RDY or
 * ABORTB, or an unknown input that changes its state (the differential
 * rule: a tick with X inputs runs twice, X bits 0 then 1; different results
 * make it lost).
 *
 * RDY low holds the cycle (D0-D7 stay in their data-transfer state through
 * PHI2 low, section 7.6); WAI pulls RDY low until an interrupt; STP stops the
 * CPU until reset. ABORTB low at a fall marks the instruction in progress:
 * when its last cycle ends (the next opcode fetch) the registers go back to
 * the snapshot taken at its opcode fetch and the core runs the abort
 * sequence. BE low floats A0-A15, D0-D7 and RWB after tBVD.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

#include "../third_party/jsmooch/wdc65816.h"
#include "../third_party/jsmooch/wdc65816_disassembler.h"

/* Control order (cpu.c CONTROLS_65816). */
enum { K_CLOCK, K_RW, K_IRQ, K_NMI, K_RESET, K_RDY, K_ABORT, K_VDA, K_VPA, K_VPB, K_MLB, K_E,
       K_MX, K_BE, K_COUNT };
/* Timing order (cpu.c TIMING_65816). */
enum { T_CYC, T_PWH, T_PWL, T_ADS, T_AH, T_BAS, T_BH, T_DSR, T_MDS, T_DHW, T_PCS, T_ES, T_EH,
       T_BVD };
/* The bus pins BE floats: A0-A15, D0-D7, RWB. */
enum { BUS_DATA = 16, BUS_RW = 24, BUS_COUNT = 25 };

typedef struct {
    CpuBase base;
    const SimTime *t;
    size_t bus_pin[BUS_COUNT];
    size_t ctl[K_COUNT];
    struct WDC65816 core;
    u64 clock_count;
    CpuStatus status;
    uint64_t cycles;
    unsigned reset_cycles;
    LogicValue clock;
    SimTime last_rise, last_fall;  /* CPU_NEVER: none since the clock was last known */
    SimTime changed[CPU_MAX_PINS];
    LogicValue seen[CPU_MAX_PINS];
    LogicValue be;
    LogicValue bus[BUS_COUNT];     /* what each bus pin carries once its drives land */
    LogicValue vda_out, vpa_out, vpb_out, mlb_out, e_out;
    int held;                      /* RDY held the cycle at the last fall */
    int rdy_driven;                /* WAI pulls RDY low */
    int abort_marked;              /* ABORTB seen during the instruction in progress */
    struct WDC65816_regs snapshot; /* the registers at that instruction's opcode fetch */
    CpuTracker tracker;
} Cpu816;

/* ---- The disassembler (tracker hook) ------------------------------------- */

typedef struct {
    uint32_t pc;
    const uint8_t *bytes;
    size_t count;
} DasmBytes;

static u32 dasm_read(void *ptr, u32 addr) {
    const DasmBytes *b = ptr;
    uint32_t offset = (addr - b->pc) & 0xFFFFu;

    return offset < b->count ? b->bytes[offset] : 0;
}

static int is_hex(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

/* jsmooch's text, uppercased, without its "[xxxxxx]" effective-address
   suffix. `user` holds the instruction's E, M and X (bits 0-2). */
static size_t dasm_816(uint32_t pc, const uint8_t *bytes, size_t count, void *user, char *out,
                       size_t cap) {
    unsigned flags = (unsigned)(uintptr_t)user;
    DasmBytes b = {pc, bytes, count};
    struct jsm_debug_read_trace rt;
    struct WDC65816_regs regs;
    struct jsm_string text;
    size_t n;
    size_t i;
    u32 length;

    memset(&rt, 0, sizeof rt);
    memset(&regs, 0, sizeof regs);
    rt.ptr = &b;
    rt.read_trace = dasm_read;
    jsm_string_init(&text, 64);
    length = WDC65816_disassemble(pc & 0xFFFFFFu, &regs, flags & 1u, (flags >> 1) & 1u,
                                  (flags >> 2) & 1u, &rt, &text, NULL);
    n = text.ptr != NULL ? strlen(text.ptr) : 0;
    if (n >= 8 && text.ptr[n - 8] == '[' && text.ptr[n - 1] == ']') {
        int hex = 1;

        for (i = n - 7; i < n - 1; i++) hex &= is_hex(text.ptr[i]);
        if (hex) {
            n -= 8;
            while (n > 0 && text.ptr[n - 1] == ' ') n--;
        }
    }
    for (i = 0; i < n && i + 1 < cap; i++) {
        char c = text.ptr[i];

        out[i] = c >= 'a' && c <= 'z' ? (char)(c - 'a' + 'A') : c;
    }
    if (cap > 0) out[i] = '\0';
    jsm_string_delete(&text);
    return length > 0 ? length : 1;
}

/* ---- The core ------------------------------------------------------------ */

static int core_fetch(const struct WDC65816 *c) {
    return c->regs.TCU == 0 && c->pins.VDA && c->pins.VPA;
}

/* Equal state. IRQ_pending and interrupt_pending only mirror the IRQB input,
   set again at every tick; what the level did shows in the rest (an
   interrupt taken, a WAI ended). */
static int same_core(const struct WDC65816 *a, const struct WDC65816 *b) {
    const struct WDC65816_regs *x = &a->regs;
    const struct WDC65816_regs *y = &b->regs;
    const struct WDC65816_pins *p = &a->pins;
    const struct WDC65816_pins *q = &b->pins;

    return x->IR == y->IR && x->TCU == y->TCU && x->TR == y->TR && x->TA == y->TA &&
           x->skipped_cycle == y->skipped_cycle && x->STP == y->STP && x->WAI == y->WAI &&
           x->C == y->C && x->D == y->D && x->X == y->X && x->Y == y->Y && x->P.v == y->P.v &&
           x->PBR == y->PBR && x->PC == y->PC && x->S == y->S && x->DBR == y->DBR &&
           x->E == y->E && x->NMI_pending == y->NMI_pending && x->NMI_old == y->NMI_old &&
           x->ABORT_pending == y->ABORT_pending &&
           p->Addr == q->Addr && p->BA == q->BA && p->RW == q->RW && p->VDA == q->VDA &&
           p->VPA == q->VPA && p->VPB == q->VPB && p->ML == q->ML && p->E == q->E &&
           p->M == q->M && p->X == q->X && (!p->RW || (p->D & 0xFF) == (q->D & 0xFF)) &&
           a->trace.ins_PC == b->trace.ins_PC;
}

/* ---- Drives -------------------------------------------------------------- */

static LogicValue bit_of(unsigned value, unsigned bit) {
    return (value >> bit) & 1u ? LOGIC_1 : LOGIC_0;
}

/* A bus pin, gated by BE: the value is remembered, and driven only while BE
   is high. */
static void bus_put(Sim *sim, SimInstance *instance, Cpu816 *s, size_t index, LogicValue value,
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

static void out_now(Sim *sim, SimInstance *instance, Cpu816 *s, int key, LogicValue *out,
                    LogicValue value, SimTime delay) {
    if (out != NULL) *out = value;
    sim_schedule_inertial_drive(sim, instance, s->ctl[key], value, DRIVE_STRONG, delay);
}

/* An output that changes: X at tAH, the value at tADS (transport). */
static void out_change(Sim *sim, SimInstance *instance, Cpu816 *s, int key, LogicValue *out,
                       LogicValue value) {
    if (*out != value) {
        sim_schedule_drive(sim, instance, s->ctl[key], LOGIC_X, DRIVE_STRONG, s->t[T_AH]);
        sim_schedule_drive(sim, instance, s->ctl[key], value, DRIVE_STRONG, s->t[T_ADS]);
        *out = value;
    }
}

static void release_rdy(Sim *sim, SimInstance *instance, Cpu816 *s) {
    if (s->rdy_driven) {
        sim_schedule_drive(sim, instance, s->ctl[K_RDY], LOGIC_Z, DRIVE_STRONG, 0);
        s->rdy_driven = 0;
    }
}

/* Lost: every output X now (bus Z while BE is low), until a reset. */
static void enter_lost(Sim *sim, SimInstance *instance, Cpu816 *s) {
    size_t i;

    if (s->status == CPU_LOST) {
        return;
    }
    s->status = CPU_LOST;
    s->held = 0;
    s->abort_marked = 0;
    for (i = 0; i < BUS_COUNT; i++) {
        bus_put(sim, instance, s, i, LOGIC_X, 0, 1);
    }
    out_now(sim, instance, s, K_VDA, &s->vda_out, LOGIC_X, 0);
    out_now(sim, instance, s, K_VPA, &s->vpa_out, LOGIC_X, 0);
    out_now(sim, instance, s, K_VPB, &s->vpb_out, LOGIC_X, 0);
    out_now(sim, instance, s, K_MLB, &s->mlb_out, LOGIC_X, 0);
    out_now(sim, instance, s, K_E, &s->e_out, LOGIC_X, 0);
    out_now(sim, instance, s, K_MX, NULL, LOGIC_X, 0);
    release_rdy(sim, instance, s);
}

/* RESB held: address X, data Z, RWB 1, VDA = VPA = 0, VPB = MLB = 1, E = 1,
   MX = 1 from tAH. */
static void enter_reset(Sim *sim, SimInstance *instance, Cpu816 *s) {
    SimTime tah = s->t[T_AH];
    size_t i;

    s->status = CPU_RESET;
    s->reset_cycles = 0;
    s->held = 0;
    s->abort_marked = 0;
    for (i = 0; i < BUS_DATA; i++) {
        bus_put(sim, instance, s, i, LOGIC_X, tah, 1);
    }
    for (i = BUS_DATA; i < BUS_RW; i++) {
        bus_put(sim, instance, s, i, LOGIC_Z, tah, 1);
    }
    bus_put(sim, instance, s, BUS_RW, LOGIC_1, tah, 1);
    out_now(sim, instance, s, K_VDA, &s->vda_out, LOGIC_0, tah);
    out_now(sim, instance, s, K_VPA, &s->vpa_out, LOGIC_0, tah);
    out_now(sim, instance, s, K_VPB, &s->vpb_out, LOGIC_1, tah);
    out_now(sim, instance, s, K_MLB, &s->mlb_out, LOGIC_1, tah);
    out_now(sim, instance, s, K_E, &s->e_out, LOGIC_1, tah);
    out_now(sim, instance, s, K_MX, NULL, LOGIC_1, tah);
    release_rdy(sim, instance, s);
}

/* Drives the bus cycle the core presents, from a commit (a PHI2 fall). */
static void drive_cycle(Sim *sim, SimInstance *instance, Cpu816 *s) {
    const struct WDC65816_pins *p = &s->core.pins;
    unsigned addr = p->Addr & 0xFFFFu;
    LogicValue rwb = p->RW ? LOGIC_0 : LOGIC_1;
    LogicValue e = p->E ? LOGIC_1 : LOGIC_0;
    size_t i;

    for (i = 0; i < BUS_DATA; i++) {
        LogicValue v = bit_of(addr, (unsigned)i);

        if (s->bus[i] != v) {
            bus_put(sim, instance, s, i, LOGIC_X, s->t[T_AH], 0);
            bus_put(sim, instance, s, i, v, s->t[T_ADS], 0);
        }
    }
    if (s->bus[BUS_RW] != rwb) {
        bus_put(sim, instance, s, BUS_RW, LOGIC_X, s->t[T_AH], 0);
        bus_put(sim, instance, s, BUS_RW, rwb, s->t[T_ADS], 0);
    }
    /* PHI2 low: the bank address on D0-D7. */
    for (i = 0; i < 8; i++) {
        bus_put(sim, instance, s, BUS_DATA + i, LOGIC_X, s->t[T_DHW], 0);
        bus_put(sim, instance, s, BUS_DATA + i, bit_of(p->BA, (unsigned)i), s->t[T_BAS], 0);
    }
    out_change(sim, instance, s, K_VDA, &s->vda_out, p->VDA ? LOGIC_1 : LOGIC_0);
    out_change(sim, instance, s, K_VPA, &s->vpa_out, p->VPA ? LOGIC_1 : LOGIC_0);
    out_change(sim, instance, s, K_VPB, &s->vpb_out, p->VPB ? LOGIC_0 : LOGIC_1);
    out_change(sim, instance, s, K_MLB, &s->mlb_out, p->ML ? LOGIC_0 : LOGIC_1);
    if (s->e_out != e) {
        out_now(sim, instance, s, K_E, &s->e_out, e, s->t[T_EH]);
    }
    /* MX shows X while PHI2 is low. */
    out_now(sim, instance, s, K_MX, NULL, p->X ? LOGIC_1 : LOGIC_0, s->t[T_EH]);
}

/* ---- The commit (PHI2 fall) ---------------------------------------------- */

static LogicValue sample(const Sim *sim, const SimInstance *instance, const Cpu816 *s, int key,
                         SimTime setup) {
    size_t pin = s->ctl[key];

    return cpu_sample(sim, instance, pin, s->changed[pin], s->seen[pin], setup, LOGIC_1);
}

static void set_inputs(struct WDC65816 *c, int irq, int nmi) {
    WDC65816_set_IRQ_level(c, (u32)irq);
    WDC65816_set_NMI_level(c, (u32)nmi);
}

/* After a tick: status, WAI's RDY drive, the tracker. */
static void after_tick(Sim *sim, SimInstance *instance, Cpu816 *s) {
    const struct WDC65816 *c = &s->core;

    s->status = c->regs.STP ? CPU_STOPPED : c->regs.WAI ? CPU_WAITING : CPU_RUNNING;
    if (c->regs.WAI && !c->regs.STP && !s->rdy_driven) {
        sim_schedule_drive(sim, instance, s->ctl[K_RDY], LOGIC_0, DRIVE_STRONG, 0);
        s->rdy_driven = 1;
    } else if (!c->regs.WAI) {
        release_rdy(sim, instance, s);
    }
    if (core_fetch(c) && !c->regs.STP) {
        unsigned flags = (unsigned)c->pins.E | (unsigned)c->pins.M << 1 | (unsigned)c->pins.X << 2;

        cpu_tracker_begin(&s->tracker, ((uint32_t)c->pins.BA << 16) | (c->pins.Addr & 0xFFFFu),
                          (void *)(uintptr_t)flags);
    }
}

/* RESB released after a reset: a fresh core in its reset sequence. */
static void start_core(Sim *sim, SimInstance *instance, Cpu816 *s) {
    WDC65816_delete(&s->core);
    WDC65816_init(&s->core, &s->clock_count);
    s->core.regs.E = 1;
    s->core.regs.P.M = s->core.regs.P.X = 1;
    s->core.regs.STP = 0;
    s->core.pins.E = 1;
    s->core.pins.M = s->core.pins.X = 1;
    WDC65816_reset(&s->core);
    WDC65816_cycle(&s->core);
    s->cycles = 0;
    s->held = 0;
    s->abort_marked = 0;
    cpu_tracker_reset(&s->tracker, dasm_816);
    drive_cycle(sim, instance, s);
    after_tick(sim, instance, s);
}

static void commit(Sim *sim, SimInstance *instance, Cpu816 *s) {
    struct WDC65816_pins *p = &s->core.pins;
    int read = !p->RW;
    LogicValue res = cpu_sample_reset(sim, instance, s->ctl[K_RESET], s->changed[s->ctl[K_RESET]],
                                      s->seen[s->ctl[K_RESET]], s->t[T_PCS],
                                      s->status == CPU_RESET);
    LogicValue level;
    LogicValue irq;
    LogicValue nmi;
    uint8_t data = 0;
    uint8_t xdata = 0;
    size_t i;

    /* 1. Reset. */
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
    /* 2. Only a reset leaves LOST. */
    if (s->status == CPU_LOST) {
        return;
    }
    /* 3. RDY (not while WAI pulls it low itself): low holds the cycle. */
    if (s->status != CPU_WAITING) {
        level = sample(sim, instance, s, K_RDY, s->t[T_PCS]);
        if (level == LOGIC_X) {
            enter_lost(sim, instance, s);
            return;
        }
        if (level == LOGIC_0) {
            s->held = 1;
            s->cycles++;
            /* MX still alternates. */
            out_now(sim, instance, s, K_MX, NULL, p->X ? LOGIC_1 : LOGIC_0, s->t[T_EH]);
            return;
        }
    }
    s->held = 0;
    /* 4. ABORTB: marks the instruction in progress (and ends a WAI). */
    level = sample(sim, instance, s, K_ABORT, s->t[T_PCS]);
    if (level == LOGIC_X) {
        enter_lost(sim, instance, s);
        return;
    }
    if (level == LOGIC_0) {
        s->abort_marked = 1;
        s->core.regs.WAI = 0;
    }
    /* 5. The read data. */
    if (read) {
        for (i = 0; i < CPU_DATA_BITS; i++) {
            size_t pin = s->bus_pin[BUS_DATA + i];

            level = cpu_sample(sim, instance, pin, s->changed[pin], s->seen[pin], s->t[T_DSR],
                               LOGIC_X);
            if (level == LOGIC_X) {
                xdata |= (uint8_t)(1u << i);
            } else if (level == LOGIC_1) {
                data |= (uint8_t)(1u << i);
            }
        }
        if (p->VPA) {
            cpu_tracker_read(&s->tracker, ((uint32_t)p->BA << 16) | (p->Addr & 0xFFFFu), data);
        }
        p->D = data;
    }
    /* 6. An opcode fetch ends the instruction before it: an abort restores
       its registers; otherwise this is the next instruction's snapshot. */
    if (core_fetch(&s->core) && !s->core.regs.STP) {
        if (s->abort_marked) {
            struct WDC65816_regs now = s->core.regs;

            s->core.regs = s->snapshot;
            s->core.regs.TCU = now.TCU;
            s->core.regs.NMI_pending = now.NMI_pending;
            s->core.regs.IRQ_pending = now.IRQ_pending;
            s->core.regs.interrupt_pending = now.interrupt_pending;
            s->core.regs.NMI_old = now.NMI_old;
            s->core.regs.ABORT_pending = 1;
            s->abort_marked = 0;
        } else {
            s->snapshot = s->core.regs;
        }
    }
    /* 7. The tick, with the differential rule. */
    irq = sample(sim, instance, s, K_IRQ, s->t[T_PCS]);
    nmi = sample(sim, instance, s, K_NMI, s->t[T_PCS]);
    if (xdata == 0 && irq != LOGIC_X && nmi != LOGIC_X) {
        set_inputs(&s->core, irq == LOGIC_0, nmi == LOGIC_0);
        WDC65816_cycle(&s->core);
    } else {
        struct WDC65816 a = s->core;
        struct WDC65816 b = s->core;

        a.pins.D = data;
        b.pins.D = data | xdata;
        set_inputs(&a, irq == LOGIC_0, nmi == LOGIC_0);
        set_inputs(&b, irq != LOGIC_1, nmi != LOGIC_1);
        WDC65816_cycle(&a);
        WDC65816_cycle(&b);
        if (!same_core(&a, &b)) {
            enter_lost(sim, instance, s);
            return;
        }
        s->core = a;
    }
    s->cycles++;
    /* 8. Drive the next cycle; status. */
    drive_cycle(sim, instance, s);
    after_tick(sim, instance, s);
}

/* ---- Pins ------------------------------------------------------------------ */

static void clock_rise(Sim *sim, SimInstance *instance, Cpu816 *s, SimTime now) {
    const struct WDC65816_pins *p = &s->core.pins;
    size_t i;

    if (s->last_fall != CPU_NEVER && now - s->last_fall < s->t[T_PWL]) {
        enter_lost(sim, instance, s);
    }
    s->last_rise = now;
    if (s->status == CPU_LOST || s->status == CPU_RESET) {
        return;
    }
    /* MX shows M while PHI2 is high. */
    out_now(sim, instance, s, K_MX, NULL, p->M ? LOGIC_1 : LOGIC_0, s->t[T_EH]);
    if (s->held) {
        return;   /* RDY: the data pins stay in their transfer state */
    }
    for (i = 0; i < 8; i++) {
        if (p->RW) {
            bus_put(sim, instance, s, BUS_DATA + i, LOGIC_X, s->t[T_BH], 0);
            bus_put(sim, instance, s, BUS_DATA + i, bit_of(p->D & 0xFFu, (unsigned)i), s->t[T_MDS],
                    0);
        } else {
            bus_put(sim, instance, s, BUS_DATA + i, LOGIC_Z, s->t[T_BH], 0);
        }
    }
}

static void clock_fall(Sim *sim, SimInstance *instance, Cpu816 *s, SimTime now) {
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

/* BE: A0-A15, D0-D7 and RWB float, or return, tBVD after a change. */
static void bus_enable(Sim *sim, SimInstance *instance, Cpu816 *s, LogicValue level) {
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

static void c816_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                             LogicValue old_value, LogicValue new_value) {
    Cpu816 *s = raw;
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
        } else if (level != s->clock) {
            s->clock = level;
            if (level == LOGIC_1) {
                clock_rise(sim, instance, s, now);
            } else {
                clock_fall(sim, instance, s, now);
            }
        }
    } else if (pin == s->ctl[K_BE]) {
        bus_enable(sim, instance, s, cpu_level(new_value, LOGIC_1));
    }
}

/* ---- Model ------------------------------------------------------------------ */

static void *c816_create(const Part *part, const CpuPins *pins, const CpuGrade *grade) {
    Cpu816 *s = calloc(1, sizeof(*s));
    size_t i;

    (void)part;
    if (s == NULL) {
        return NULL;
    }
    s->base.ops = &CPU65816_OPS;
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
    WDC65816_init(&s->core, &s->clock_count);
    s->status = CPU_LOST;
    s->clock = LOGIC_X;
    s->last_rise = CPU_NEVER;
    s->last_fall = CPU_NEVER;
    s->be = LOGIC_1;
    for (i = 0; i < CPU_MAX_PINS; i++) {
        s->changed[i] = CPU_NEVER;
        s->seen[i] = LOGIC_Z;
    }
    cpu_tracker_reset(&s->tracker, dasm_816);
    return s;
}

static void c816_destroy(void *state) {
    Cpu816 *s = state;

    WDC65816_delete(&s->core);
    free(s);
}

/* Power-up: LOST, every output X. */
static void c816_start(Sim *sim, SimInstance *instance, void *raw) {
    Cpu816 *s = raw;
    size_t i;

    for (i = 0; i < BUS_COUNT; i++) {
        bus_put(sim, instance, s, i, LOGIC_X, 0, 0);
    }
    s->vda_out = s->vpa_out = s->vpb_out = s->mlb_out = s->e_out = LOGIC_X;
    sim_schedule_drive(sim, instance, s->ctl[K_VDA], LOGIC_X, DRIVE_STRONG, 0);
    sim_schedule_drive(sim, instance, s->ctl[K_VPA], LOGIC_X, DRIVE_STRONG, 0);
    sim_schedule_drive(sim, instance, s->ctl[K_VPB], LOGIC_X, DRIVE_STRONG, 0);
    sim_schedule_drive(sim, instance, s->ctl[K_MLB], LOGIC_X, DRIVE_STRONG, 0);
    sim_schedule_drive(sim, instance, s->ctl[K_E], LOGIC_X, DRIVE_STRONG, 0);
    sim_schedule_drive(sim, instance, s->ctl[K_MX], LOGIC_X, DRIVE_STRONG, 0);
}

static int c816_state(const void *raw, CpuState *out) {
    const Cpu816 *s = raw;

    out->status = s->status;
    out->cycles = s->cycles;
    if (s->status != CPU_LOST && s->status != CPU_RESET) {
        const struct WDC65816_regs *r = &s->core.regs;

        out->registers[0] = (CpuRegister){"C", r->C, 16};
        out->registers[1] = (CpuRegister){"X", r->X & 0xFFFFu, 16};
        out->registers[2] = (CpuRegister){"Y", r->Y & 0xFFFFu, 16};
        out->registers[3] = (CpuRegister){"S", r->S & 0xFFFFu, 16};
        out->registers[4] = (CpuRegister){"D", r->D & 0xFFFFu, 16};
        out->registers[5] = (CpuRegister){"DBR", r->DBR & 0xFFu, 8};
        out->registers[6] = (CpuRegister){"PBR", r->PBR & 0xFFu, 8};
        out->registers[7] = (CpuRegister){"PC", r->PC & 0xFFFFu, 16};
        out->registers[8] = (CpuRegister){"P", r->P.v, 8};
        out->registers[9] = (CpuRegister){"E", r->E & 1u, 1};
        out->register_count = 10;
    }
    cpu_tracker_fill(&s->tracker, out);
    return 1;
}

const CpuCoreOps CPU65816_OPS = {
    c816_create, c816_destroy, c816_start, c816_pin_changed, NULL, c816_state
};
