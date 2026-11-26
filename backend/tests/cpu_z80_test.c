/*
 * cpu_z80_test: split from cpu_test.c by plan-68-H; the shared helpers are
 * support/cpu_test_bench.c.
 */

#include "cpu_test_bench.h"

/* ---- The Z80 bench (plan-09-C section 5.1) ---------------------------------- */

/*
 * A second bench for the Z80: it drives CLK (1 MHz, 50 %) and the inputs,
 * and reacts to the strobes: on MREQ#+RD# (not refresh) it drives memory
 * from RD# fall + 100 ns to RD# rise + 20 ns; on MREQ#+WR# it stores at WR#
 * rise; I/O the same on IORQ# (reads return io_in); an int-ack (M1#+IORQ#)
 * gets `vector`. Every machine cycle with a strobe is logged with its T1
 * (the index of its first CLK rise) and the strobe's fall and rise times.
 */

enum { ZT_RISE, ZT_FALL, ZT_SERVE, ZT_RELEASE, ZT_POKE };

static size_t zbench_pins(const void *config) {
    return ((const ZBench *)config)->part->pin_count;
}

static LogicValue zpin(const ZBench *b, size_t pin) {
    return sim_read_pin(b->sim, b->tb, pin);
}

static void zdrive(ZBench *b, size_t pin, LogicValue value, SimTime delay) {
    sim_schedule_drive(b->sim, b->tb, pin, value, DRIVE_STRONG, delay);
}

static int zaddress(const ZBench *b) {
    int value = 0;
    int i;

    for (i = 0; i < 16; i++) {
        LogicValue v = zpin(b, b->a[i]);

        if (v != LOGIC_0 && v != LOGIC_1) {
            return -1;
        }
        value |= (v == LOGIC_1) << i;
    }
    return value;
}

/* The data bus: value, -1 X or mixed, -2 all Z. */
static int zdata(const ZBench *b) {
    int value = 0;
    int z = 0;
    int i;

    for (i = 0; i < 8; i++) {
        LogicValue v = zpin(b, b->d[i]);

        z += v == LOGIC_Z;
        if (v == LOGIC_1) {
            value |= 1 << i;
        } else if (v != LOGIC_0) {
            value = -1000;
        }
    }
    return z == 8 ? -2 : value < 0 ? -1 : value;
}

static void zdrive_data(ZBench *b, int value) {
    int i;

    for (i = 0; i < 8; i++) {
        zdrive(b, b->d[i], value < 0 ? LOGIC_X : (value >> i) & 1 ? LOGIC_1 : LOGIC_0, 0);
    }
}

static long zopen(ZBench *b, char type, int addr, size_t back) {
    ZCycle *c;

    if (b->log_count >= ZLOG_MAX) {
        return -1;
    }
    c = &b->log[b->log_count];
    memset(c, 0, sizeof(*c));
    c->type = type;
    c->addr = addr;
    c->data = -1;
    c->t1 = b->rises - 1 - back;
    c->start = sim_now(b->sim);
    c->m1_fall = b->m1_fell;
    return (long)b->log_count++;
}

static void zserve(ZBench *b, int value) {
    b->serve_value = value;
    sim_schedule_timer(b->sim, b->tb, ZT_SERVE, NS(100));
}

static void zbench_start(Sim *sim, SimInstance *instance, void *state) {
    ZBench *b = state;

    (void)instance;
    zdrive(b, b->clk, LOGIC_0, 0);
    sim_schedule_timer(sim, b->tb, ZT_RISE, b->half);
    b->next_rise = b->half;
}

static void zbench_timer(Sim *sim, SimInstance *instance, void *state, uint32_t id) {
    ZBench *b = state;
    SimTime high;
    int i;

    (void)instance;
    switch (id) {
        case ZT_RISE:
            zdrive(b, b->clk, LOGIC_1, 0);
            if (b->rises < RISES_MAX) {
                b->rise_at[b->rises] = sim_now(sim);
            }
            b->rises++;
            high = b->high_once != 0 ? b->high_once : b->half;
            b->high_once = 0;
            sim_schedule_timer(sim, b->tb, ZT_FALL, high);
            b->next_rise = sim_now(sim) + high + b->half;
            break;
        case ZT_FALL:
            zdrive(b, b->clk, LOGIC_0, 0);
            sim_schedule_timer(sim, b->tb, ZT_RISE, b->half);
            break;
        case ZT_SERVE:
            zdrive_data(b, b->serve_value);
            b->serving = 1;
            break;
        case ZT_RELEASE:
            for (i = 0; i < 8; i++) {
                zdrive(b, b->d[i], LOGIC_Z, 0);
            }
            b->serving = 0;
            break;
        case ZT_POKE:
            if (b->serving) {
                zdrive_data(b, b->poke_value);
            }
            break;
    }
}

static void zbench_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                               LogicValue old_value, LogicValue new_value) {
    ZBench *b = state;
    SimTime now = sim_now(sim);
    int i;
    long e;

    (void)instance;
    for (i = 0; i < 8; i++) {
        if (pin == b->d[i]) {
            b->data_changed = now;
        }
    }
    if (pin == b->m1 && new_value == LOGIC_0) {
        b->m1_fell = now;
    }
    if (pin == b->halt && new_value == LOGIC_0) {
        b->halt_fell = now;
    }
    if (pin == b->rd && new_value == LOGIC_0 && old_value == LOGIC_1) {
        int addr = zaddress(b);

        if (zpin(b, b->mreq) == LOGIC_0 && zpin(b, b->rfsh) == LOGIC_1) {
            e = zopen(b, zpin(b, b->m1) == LOGIC_0 ? 'F' : 'R', addr, 0);
            if (e >= 0) {
                b->log[e].data = addr >= 0 && b->known[addr] ? b->mem[addr] : -1;
                zserve(b, b->log[e].data);
                b->open_read = e;
            }
        } else if (zpin(b, b->iorq) == LOGIC_0) {
            e = zopen(b, 'I', addr, 1);
            if (e >= 0) {
                b->log[e].data = b->io_in;
                zserve(b, b->io_in);
                b->open_read = e;
            }
        }
    } else if (pin == b->rd && new_value == LOGIC_1 && b->open_read >= 0) {
        b->log[b->open_read].end = now;
        b->open_read = -1;
        sim_schedule_timer(sim, b->tb, ZT_RELEASE, NS(20));
    } else if (pin == b->wr && new_value == LOGIC_0 && old_value == LOGIC_1) {
        int addr = zaddress(b);

        if (zpin(b, b->mreq) == LOGIC_0) {
            b->open_write = zopen(b, 'W', addr, 1);   /* WR# falls in T2 */
        } else if (zpin(b, b->iorq) == LOGIC_0) {
            b->open_write = zopen(b, 'O', addr, 1);
        }
    } else if (pin == b->wr && new_value == LOGIC_1 && b->open_write >= 0) {
        ZCycle *c = &b->log[b->open_write];

        c->end = now;
        c->data = zdata(b);
        c->data_at = b->data_changed;
        if (c->type == 'W' && c->addr >= 0) {
            b->mem[c->addr] = (uint8_t)(c->data >= 0 ? c->data : 0);
            b->known[c->addr] = c->data >= 0;
        }
        b->open_write = -1;
    } else if (pin == b->iorq && new_value == LOGIC_0 && zpin(b, b->m1) == LOGIC_0) {
        e = zopen(b, 'A', zaddress(b), 2);
        if (e >= 0) {
            b->log[e].data = b->vector;
            zserve(b, b->vector);
            b->open_read = e;
        }
    } else if (pin == b->iorq && new_value == LOGIC_1 && b->open_read >= 0 &&
               b->log[b->open_read].type == 'A') {
        b->log[b->open_read].end = now;
        b->open_read = -1;
        sim_schedule_timer(sim, b->tb, ZT_RELEASE, NS(20));
    }
}

static const SimComponentType ZBENCH_TYPE = {
    "zbench", zbench_pins, bench_create, bench_destroy, zbench_start, zbench_pin_changed,
    zbench_timer, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static void zset(ZBench *b, size_t pin, LogicValue value) {
    zdrive(b, pin, value, 0);
}

static void zrun_to(ZBench *b, SimTime time) {
    if (!sim_run_until(b->sim, time)) {
        fprintf(stderr, "sim stopped: %s\n", sim_error(b->sim));
        exit(2);
    }
}

/* Runs n more CLK rises, stopping 200 ns after the last. */
static void zrun_rises(ZBench *b, unsigned n) {
    unsigned i;

    for (i = 0; i < n; i++) {
        zrun_to(b, b->next_rise + NS(200));
    }
}

static ZBench *zbench_open(void) {
    ZBench *b = calloc(1, sizeof(*b));
    const Part *part = part_library_find(library, Z84C00);
    size_t i;
    char pin[8];

    if (b == NULL || part == NULL) {
        fprintf(stderr, "no part %s\n", Z84C00);
        exit(2);
    }
    b->rise_at = calloc(RISES_MAX, sizeof(*b->rise_at));
    b->part = part;
    for (i = 0; i < 16; i++) {
        snprintf(pin, sizeof pin, "A%zu", i);
        b->a[i] = pin_named(part, pin);
    }
    for (i = 0; i < 8; i++) {
        snprintf(pin, sizeof pin, "D%zu", i);
        b->d[i] = pin_named(part, pin);
    }
    b->clk = pin_named(part, "CLK");
    b->m1 = pin_named(part, "M1#");
    b->mreq = pin_named(part, "MREQ#");
    b->iorq = pin_named(part, "IORQ#");
    b->rd = pin_named(part, "RD#");
    b->wr = pin_named(part, "WR#");
    b->rfsh = pin_named(part, "RFSH#");
    b->halt = pin_named(part, "HALT#");
    b->busack = pin_named(part, "BUSACK#");
    b->wait = pin_named(part, "WAIT#");
    b->intr = pin_named(part, "INT#");
    b->nmi = pin_named(part, "NMI#");
    b->reset = pin_named(part, "RESET#");
    b->busreq = pin_named(part, "BUSREQ#");
    b->half = NS(500);
    b->open_read = -1;
    b->open_write = -1;
    b->sim = sim_create();
    b->cpu = sim_add_instance(b->sim, kind_model(part), "U1", part);
    b->tb = sim_add_instance(b->sim, &ZBENCH_TYPE, "TB", b);
    for (i = 0; i < part->pin_count; i++) {
        SimNet *net = sim_add_net(b->sim, "n");

        sim_connect(b->sim, net, b->cpu, i);
        sim_connect(b->sim, net, b->tb, i);
    }
    if (!sim_start(b->sim)) {
        fprintf(stderr, "sim_start: %s\n", sim_error(b->sim));
        exit(2);
    }
    zset(b, b->intr, LOGIC_1);
    zset(b, b->nmi, LOGIC_1);
    zset(b, b->wait, LOGIC_1);
    zset(b, b->busreq, LOGIC_1);
    zset(b, b->reset, LOGIC_1);
    return b;
}

static void zbench_close(ZBench *b) {
    sim_destroy(b->sim);
    free(b->rise_at);
    free(b);
}

static void zload(ZBench *b, uint16_t address, const uint8_t *bytes, size_t count) {
    size_t i;

    for (i = 0; i < count; i++) {
        b->mem[(uint16_t)(address + i)] = bytes[i];
        b->known[(uint16_t)(address + i)] = 1;
    }
}

/* RESET# low for 4 clocks; returns 200 ns after T1 rise of the first fetch. */
static void zreset(ZBench *b) {
    zset(b, b->reset, LOGIC_0);
    zrun_rises(b, 4);
    zset(b, b->reset, LOGIC_1);
    b->first = b->rises;
    zrun_rises(b, 1);
}

/* A bench with NOPs at 0000-00FF, 0038, 0066 and 0500, the program at
   0000, and a reset done. */
static ZBench *zboot(const uint8_t *program, size_t count) {
    ZBench *b = zbench_open();
    size_t i;

    for (i = 0; i < 0x100; i++) {
        b->mem[i] = 0x00;
        b->known[i] = 1;
    }
    zload(b, 0x0000, program, count);
    zreset(b);
    return b;
}

static int zstatus_is(const ZBench *b, CpuStatus status) {
    CpuState state;
    const SimComponentType *type = sim_instance_type(b->cpu);

    return type->cpu_state(sim_instance_state(b->cpu), &state) && state.status == status;
}

static void zstate(const ZBench *b, CpuState *state) {
    const SimComponentType *type = sim_instance_type(b->cpu);

    type->cpu_state(sim_instance_state(b->cpu), state);
}

static uint32_t zreg(const CpuState *state, const char *name) {
    size_t i;

    for (i = 0; i < state->register_count; i++) {
        if (strcmp(state->registers[i].name, name) == 0) {
            return state->registers[i].value;
        }
    }
    return 0xEEEE;
}

static long zfind(const ZBench *b, char type, int addr, size_t from) {
    size_t i;

    for (i = from; i < b->log_count; i++) {
        if (b->log[i].type == type && (addr < 0 || b->log[i].addr == addr)) {
            return (long)i;
        }
    }
    return -1;
}

/* T-states of log entry i: up to the next entry's T1. */
static size_t zlength(const ZBench *b, long i) {
    return i >= 0 && (size_t)i + 1 < b->log_count ? b->log[i + 1].t1 - b->log[i].t1 : 0;
}

static const SimTime *ztiming(const ZBench *b) {
    return ((const CpuGrade *)part_internal(b->part)->data)->t;
}

/* Grade-10 times by name (cpuz80.c order). */
enum { ZT_CC, ZT_WCH, ZT_WCL, ZT_DCRA, ZT_DCFMREQF, ZT_DCRMREQR, ZT_DCFMREQR, ZT_DCFRDF,
       ZT_DCRRDR, ZT_SDCR, ZT_SWAITCF, ZT_DCRM1F, ZT_DCRM1R, ZT_DCRRFSHF, ZT_DCRRFSHR, ZT_DCFRDR,
       ZT_DCRRDF, ZT_SDCF, ZT_DCRIORQF, ZT_DCFIORQR, ZT_DCFWRF, ZT_DCFWRR, ZT_DCRWRF,
       ZT_DCFHALT, ZT_WNMI, ZT_SBUSREQ, ZT_DCRBUSACKF, ZT_DCFBUSACKR, ZT_DCRDZ, ZT_DCRCTZ,
       ZT_DCRAZ, ZT_SRESET, ZT_SINT, ZT_DCFIORQF, ZT_DCFD };

/* The time of CLK rise index i (past or future, 1 MHz). */
static SimTime zrise(const ZBench *b, size_t i) {
    return b->rise_at[b->first] + (SimTime)(i - b->first) * NS(1000);
}

/* ---- 5.1 Z80 cases ------------------------------------------------------- */

/* 1. Power-up lost. */
static void zcase_power_up(void) {
    ZBench *b = zbench_open();

    zrun_rises(b, 5);
    expect(zaddress(b) == -1 && zpin(b, b->m1) == LOGIC_X && zpin(b, b->mreq) == LOGIC_X &&
               zpin(b, b->rd) == LOGIC_X && zstatus_is(b, CPU_LOST),
           "Z80 1: power-up outputs X, status LOST");
    zbench_close(b);
}

/* 2. Reset: the first MC is a fetch at 0000. */
static void zcase_reset(void) {
    ZBench *b = zboot(NULL, 0);

    zrun_rises(b, 4);
    expect(b->log_count > 0 && b->log[0].type == 'F' && b->log[0].addr == 0x0000 &&
               b->log[0].t1 == b->first && zstatus_is(b, CPU_RUNNING),
           "Z80 2: after reset the first MC is a fetch at 0000; RUNNING");
    zbench_close(b);
}

/* 3. A 2-clock reset is too short. */
static void zcase_short_reset(void) {
    ZBench *b = zbench_open();

    zset(b, b->reset, LOGIC_0);
    zrun_rises(b, 2);
    zset(b, b->reset, LOGIC_1);
    zrun_rises(b, 3);
    expect(zstatus_is(b, CPU_LOST), "Z80 3: RESET# low for 2 clocks: LOST");
    zbench_close(b);
}

/* 3b. RESET# released inside tSRESET before a rise: the first fetch starts
   one T-state later; the CPU runs. */
static void zcase_reset_setup(void) {
    ZBench *b = zbench_open();
    size_t i;

    for (i = 0; i < 0x100; i++) {
        b->known[i] = 1;
    }
    zset(b, b->reset, LOGIC_0);
    zrun_rises(b, 4);
    zrun_to(b, b->next_rise - ztiming(b)[ZT_SRESET] + NS(1));
    zset(b, b->reset, LOGIC_1);
    b->first = b->rises + 1;   /* the rise after next is T1 */
    zrun_rises(b, 8);
    expect(b->log_count > 0 && b->log[0].type == 'F' && b->log[0].addr == 0x0000 &&
               b->log[0].t1 == b->first && zstatus_is(b, CPU_RUNNING),
           "Z80 3: RESET# released inside tSRESET: the first fetch one T later, CPU runs");
    zbench_close(b);
}

/* 4. The first fetch's waveform. */
static void zcase_fetch(void) {
    ZBench *b = zbench_open();
    const SimTime *t = ztiming(b);
    SimTime r, f;
    int a_before, a_after;
    LogicValue m1_before, m1_after, mreq_t1, rd_t1, rfsh_t3, rd_t3, mreq_t3, m1_t3;
    LogicValue mreq_rf, mreq_t4;
    int a_t3;
    size_t i;

    for (i = 0; i < 0x100; i++) {
        b->known[i] = 1;
    }
    zset(b, b->reset, LOGIC_0);
    zrun_rises(b, 4);
    zset(b, b->reset, LOGIC_1);
    b->first = b->rises;
    r = b->next_rise;          /* T1 rise */
    f = r + NS(500);           /* T1 fall */
    zrun_to(b, r + t[ZT_DCRA] - NS(1));
    a_before = zaddress(b);
    m1_before = zpin(b, b->m1);
    zrun_to(b, r + t[ZT_DCRA] + NS(1));
    a_after = zaddress(b);
    m1_after = zpin(b, b->m1);
    zrun_to(b, f + t[ZT_DCFRDF] + NS(1));
    mreq_t1 = zpin(b, b->mreq);
    rd_t1 = zpin(b, b->rd);
    zrun_to(b, r + NS(2000) + t[ZT_DCRRFSHF] + NS(1));   /* T3 rise + the longest delay */
    rfsh_t3 = zpin(b, b->rfsh);
    rd_t3 = zpin(b, b->rd);
    mreq_t3 = zpin(b, b->mreq);
    m1_t3 = zpin(b, b->m1);
    a_t3 = zaddress(b);
    zrun_to(b, r + NS(2500) + t[ZT_DCFMREQF] + NS(1));   /* T3 fall */
    mreq_rf = zpin(b, b->mreq);
    zrun_to(b, r + NS(3500) + t[ZT_DCFMREQR] + NS(1));   /* T4 fall */
    mreq_t4 = zpin(b, b->mreq);
    expect(a_before == -1 && a_after == 0x0000, "Z80 4: A valid at T1 rise + tDCRA");
    expect(m1_after == LOGIC_0 && (m1_before == LOGIC_1 || r + t[ZT_DCRM1F] < r + t[ZT_DCRA] - NS(1)),
           "Z80 4: M1# low at T1 rise + tDCRM1F");
    expect(mreq_t1 == LOGIC_0 && rd_t1 == LOGIC_0 && b->log_count > 0 &&
               b->log[0].start == f + t[ZT_DCFRDF],
           "Z80 4: MREQ# and RD# low after T1 fall (RD# at + tDCFRDF)");
    expect(rd_t3 == LOGIC_1 && mreq_t3 == LOGIC_1 && m1_t3 == LOGIC_1 && rfsh_t3 == LOGIC_0 &&
               a_t3 == 0x0000 && b->log[0].end == r + NS(2000) + t[ZT_DCRRDR],
           "Z80 4: T3 rise: RD#, MREQ#, M1# high, A = I:R = 0000, RFSH# low");
    expect(mreq_rf == LOGIC_0 && mreq_t4 == LOGIC_1, "Z80 4: refresh MREQ# low at T3 fall, high at T4 fall");
    zbench_close(b);
}

/* 5. Memory read and write. */
static void zcase_memory(void) {
    static const uint8_t PROGRAM[] = {0x3A, 0x00, 0x80, 0x32, 0x01, 0x80};
    ZBench *b = zbench_open();
    const SimTime *t = ztiming(b);
    CpuState state;
    long r, w;
    size_t i;

    for (i = 0; i < 0x100; i++) {
        b->known[i] = 1;
    }
    zload(b, 0x0000, PROGRAM, sizeof PROGRAM);
    zload(b, 0x8000, (const uint8_t[]){0xAB}, 1);
    zreset(b);
    zrun_rises(b, 30);
    zstate(b, &state);
    r = zfind(b, 'R', 0x8000, 0);
    w = zfind(b, 'W', 0x8001, 0);
    expect(r >= 0 && zlength(b, r) == 3 && b->log[r].start == zrise(b, b->log[r].t1) + NS(500) + t[ZT_DCFRDF] &&
               b->log[r].end == zrise(b, b->log[r].t1 + 2) + NS(500) + t[ZT_DCFRDR] &&
               (zreg(&state, "AF") >> 8) == 0xAB,
           "Z80 5: LD A,(8000h): a 3-T read, RD# from T1 fall to T3 fall, A = AB");
    expect(w >= 0 && zlength(b, w) == 3 &&
               b->log[w].data_at == zrise(b, b->log[w].t1) + NS(500) + t[ZT_DCFD] &&
               b->log[w].start == zrise(b, b->log[w].t1 + 1) + NS(500) + t[ZT_DCFWRF] &&
               b->log[w].end == zrise(b, b->log[w].t1 + 2) + NS(500) + t[ZT_DCFWRR] &&
               b->known[0x8001] && b->mem[0x8001] == 0xAB,
           "Z80 5: LD (8001h),A: D valid at T1 fall + tDCFD, WR# T2 fall to T3 fall, stored AB");
    zbench_close(b);
}

/* 6. I/O write and read. */
static void zcase_io(void) {
    static const uint8_t PROGRAM[] = {0x3E, 0x12, 0xD3, 0x10, 0xDB, 0x20};
    ZBench *b = zboot(PROGRAM, sizeof PROGRAM);
    const SimTime *t = ztiming(b);
    CpuState state;
    long o, in;

    b->io_in = 0x5A;
    zrun_rises(b, 30);
    zstate(b, &state);
    o = zfind(b, 'O', -1, 0);
    in = zfind(b, 'I', -1, 0);
    expect(o >= 0 && b->log[o].addr == 0x1210 && b->log[o].data == 0x12 && zlength(b, o) == 4 &&
               b->log[o].start == zrise(b, b->log[o].t1 + 1) + t[ZT_DCRWRF],
           "Z80 6: OUT (10h),A: a 4-T write of 12 to port 1210, WR# low from T2 rise");
    expect(in >= 0 && b->log[in].addr == 0x1220 && zlength(b, in) == 4 &&
               b->log[in].start == zrise(b, b->log[in].t1 + 1) + t[ZT_DCRRDF] &&
               (zreg(&state, "AF") >> 8) == 0x5A,
           "Z80 6: IN A,(20h): a 4-T read of port 1220, A = 5A");
    zbench_close(b);
}

/* 7. WAIT on a memory read; WAIT X at T2 fall. */
static void zcase_wait(void) {
    static const uint8_t PROGRAM[] = {0x3A, 0x00, 0x80};
    ZBench *b = zboot(PROGRAM, sizeof PROGRAM);
    long r;

    zload(b, 0x8000, (const uint8_t[]){0xAB}, 1);
    /* MCs: fetch T 0-3, read 0001 T 4-6, read 0002 T 7-9, read 8000 from T 10. */
    zrun_to(b, zrise(b, b->first + 11) + NS(200));
    zset(b, b->wait, LOGIC_0);
    zrun_to(b, zrise(b, b->first + 13) + NS(200));
    zset(b, b->wait, LOGIC_1);
    zrun_rises(b, 8);
    r = zfind(b, 'R', 0x8000, 0);
    expect(r >= 0 && b->log[r].t1 == b->first + 10 && zlength(b, r) == 5 &&
               b->log[r].end == zrise(b, b->log[r].t1 + 4) + NS(500) + ztiming(b)[ZT_DCFRDR],
           "Z80 7: WAIT# low across T2 fall for 2 clocks: a 5-T read, RD# low through Tw");
    zbench_close(b);

    b = zboot(PROGRAM, sizeof PROGRAM);
    zrun_to(b, zrise(b, b->first + 11) + NS(200));
    zset(b, b->wait, LOGIC_X);
    zrun_to(b, zrise(b, b->first + 12) + NS(200));
    expect(zstatus_is(b, CPU_LOST), "Z80 7: WAIT# X at the T2 fall that checks it: LOST");
    zbench_close(b);
}

/* 8. The opcode byte changes just inside / outside tSDCR before T3 rise. */
static void zcase_setup(void) {
    int pass;

    for (pass = 0; pass < 2; pass++) {
        ZBench *b = zboot(NULL, 0);
        SimTime t3 = zrise(b, b->first + 4 + 2);   /* the second fetch (0001): T3 rise */
        SimTime tsdcr = ztiming(b)[ZT_SDCR];
        SimTime poke = pass == 0 ? t3 - tsdcr + NS(1) : t3 - tsdcr - NS(1);

        zrun_to(b, t3 - NS(300));
        b->poke_value = 0x76;
        sim_schedule_timer(b->sim, b->tb, ZT_POKE, poke - sim_now(b->sim));
        zrun_rises(b, 8);
        if (pass == 0) {
            expect(zstatus_is(b, CPU_LOST), "Z80 8: opcode change tSDCR - 1 ns before T3 rise: LOST");
        } else {
            expect(zstatus_is(b, CPU_HALTED) && zpin(b, b->halt) == LOGIC_0,
                   "Z80 8: opcode change tSDCR + 1 ns before T3 rise: HALT executes");
        }
        zbench_close(b);
    }
}

/* 9. IM 1: an int-ack of 6 T, two pushes, the fetch at 0038. */
static void zcase_im1(void) {
    static const uint8_t PROGRAM[] = {0xED, 0x56, 0xFB};
    ZBench *b = zboot(PROGRAM, sizeof PROGRAM);
    const SimTime *t = ztiming(b);
    long a;

    zrun_rises(b, 12);
    zset(b, b->intr, LOGIC_0);
    zrun_rises(b, 30);
    a = zfind(b, 'A', -1, 0);
    expect(a >= 0 && b->log[a].m1_fall == zrise(b, b->log[a].t1) + t[ZT_DCRM1F] &&
               b->log[a].start == zrise(b, b->log[a].t1 + 2) + NS(500) + t[ZT_DCFIORQF] &&
               b->log[a].end == zrise(b, b->log[a].t1 + 4) + t[ZT_DCRM1R],
           "Z80 9: int-ack: M1# low from T1 rise, IORQ# from TW*1 fall, both high at T3 rise (6 T)");
    expect(a >= 0 && b->log[a].addr >= 0x0003 && b->log[a].addr <= 0x0010,
           "Z80 9: the int-ack puts PC on the address bus");
    expect(a >= 0 && (size_t)a + 3 < b->log_count && b->log[a + 1].type == 'W' &&
               b->log[a + 1].t1 == b->log[a].t1 + 7 && b->log[a + 2].type == 'W' &&
               b->log[a + 3].type == 'F' && b->log[a + 3].addr == 0x0038 &&
               b->log[a + 1].data == (b->log[a].addr >> 8) && b->log[a + 2].data == (b->log[a].addr & 0xFF),
           "Z80 9: then PC pushed (two writes) and a fetch at 0038");
    zbench_close(b);
}

/* 10. IM 2 (I = 12, vector 34) and IM 0 (FF on the bus: RST 38h). */
static void zcase_im2_im0(void) {
    static const uint8_t IM2[] = {0x3E, 0x12, 0xED, 0x47, 0xED, 0x5E, 0xFB};
    static const uint8_t IM0[] = {0xED, 0x46, 0xFB};
    ZBench *b = zboot(IM2, sizeof IM2);
    long a;

    zload(b, 0x1234, (const uint8_t[]){0x00, 0x05}, 2);
    zload(b, 0x0500, (const uint8_t[]){0x00, 0x00, 0x00}, 3);
    b->vector = 0x34;
    zrun_rises(b, 30);
    zset(b, b->intr, LOGIC_0);
    zrun_rises(b, 30);
    a = zfind(b, 'A', -1, 0);
    expect(a >= 0 && (size_t)a + 5 < b->log_count && b->log[a + 3].type == 'R' &&
               b->log[a + 3].addr == 0x1234 && b->log[a + 4].addr == 0x1235 &&
               b->log[a + 5].type == 'F' && b->log[a + 5].addr == 0x0500,
           "Z80 10: IM 2: reads at 1234/1235, then a fetch at 0500");
    zbench_close(b);

    b = zboot(IM0, sizeof IM0);
    b->vector = 0xFF;
    zrun_rises(b, 20);
    zset(b, b->intr, LOGIC_0);
    zrun_rises(b, 30);
    a = zfind(b, 'A', -1, 0);
    expect(a >= 0 && (size_t)a + 3 < b->log_count && b->log[a + 1].type == 'W' &&
               b->log[a + 2].type == 'W' && b->log[a + 3].type == 'F' && b->log[a + 3].addr == 0x0038,
           "Z80 10: IM 0 with FF on the bus: RST 38h (two pushes, a fetch at 0038)");
    zbench_close(b);
}

/* 11. NMI; a pulse shorter than tWNMI. */
static void zcase_nmi(void) {
    ZBench *b = zboot(NULL, 0);
    long f;

    zrun_rises(b, 10);
    zset(b, b->nmi, LOGIC_0);
    zrun_rises(b, 30);
    f = zfind(b, 'F', 0x0066, 0);
    expect(f >= 3 && b->log[f - 1].type == 'W' && b->log[f - 2].type == 'W' &&
               b->log[f - 3].type == 'F' && zlength(b, f - 3) == 5,
           "Z80 11: NMI# 1->0: a 5-T fetch, two pushes, a fetch at 0066");
    expect(zfind(b, 'F', 0x0066, (size_t)f + 1) == -1, "Z80 11: NMI# held low does not re-trigger");
    zbench_close(b);

    b = zboot(NULL, 0);
    zrun_to(b, zrise(b, b->first + 10) + NS(300));
    zset(b, b->nmi, LOGIC_0);
    zrun_to(b, sim_now(b->sim) + NS(30));
    zset(b, b->nmi, LOGIC_1);
    zrun_rises(b, 3);
    expect(zstatus_is(b, CPU_LOST), "Z80 11: an NMI# pulse shorter than tWNMI: LOST");
    zbench_close(b);
}

/* 12. HALT: HALT# low at T4 fall + tDCFHALT; refresh goes on; INT exits. */
static void zcase_halt(void) {
    static const uint8_t PROGRAM[] = {0xED, 0x56, 0xFB, 0x76};
    ZBench *b = zboot(PROGRAM, sizeof PROGRAM);
    CpuState before, after;
    long h, a;

    zrun_rises(b, 20);
    h = zfind(b, 'F', 0x0003, 0);
    zstate(b, &before);
    zrun_rises(b, 20);
    zstate(b, &after);
    expect(h >= 0 && b->halt_fell == zrise(b, b->log[h].t1 + 3) + NS(500) + ztiming(b)[ZT_DCFHALT] &&
               after.status == CPU_HALTED,
           "Z80 12: HALT# low at the HALT fetch's T4 fall + tDCFHALT");
    expect((zreg(&after, "R") & 0x7F) != (zreg(&before, "R") & 0x7F) &&
               zfind(b, 'F', 0x0003, (size_t)h + 1) >= 0,
           "Z80 12: the halted M1 cycles keep refreshing (R advances)");
    zset(b, b->intr, LOGIC_0);
    zrun_rises(b, 12);
    a = zfind(b, 'A', -1, 0);
    expect(zpin(b, b->halt) == LOGIC_1 && a >= 0, "Z80 12: INT# with EI exits: HALT# high, an int-ack");
    zbench_close(b);
}

static int zfloats(const ZBench *b) {
    int ok = zpin(b, b->mreq) == LOGIC_Z && zpin(b, b->iorq) == LOGIC_Z &&
             zpin(b, b->rd) == LOGIC_Z && zpin(b, b->wr) == LOGIC_Z;
    size_t i;

    for (i = 0; i < 16; i++) {
        ok = ok && zpin(b, b->a[i]) == LOGIC_Z;
    }
    for (i = 0; i < 8; i++) {
        ok = ok && zpin(b, b->d[i]) == LOGIC_Z;
    }
    return ok;
}

/* 13. BUSREQ. */
static void zcase_busreq(void) {
    ZBench *b = zboot(NULL, 0);
    size_t count;
    int floated, released, idle;

    zrun_rises(b, 9);
    zset(b, b->busreq, LOGIC_0);
    zrun_rises(b, 6);
    floated = zfloats(b);
    released = zpin(b, b->busack) == LOGIC_0 && zstatus_is(b, CPU_BUS_RELEASED);
    count = b->log_count;
    zrun_rises(b, 5);
    idle = b->log_count == count;
    expect(floated && released, "Z80 13: BUSREQ# low: BUSACK# low, bus Z, status BUS_RELEASED");
    expect(idle, "Z80 13: no MC while the bus is released");
    zset(b, b->busreq, LOGIC_1);
    zrun_rises(b, 1);
    zrun_to(b, sim_now(b->sim) + NS(400));   /* past the next fall */
    expect(zpin(b, b->busack) == LOGIC_1, "Z80 13: released: BUSACK# high at the next fall");
    zrun_rises(b, 5);
    expect(b->log_count > count && zstatus_is(b, CPU_RUNNING), "Z80 13: then the pending MC runs");
    zbench_close(b);
}

/* 14. A high phase shorter than tWCH. */
static void zcase_clock(void) {
    ZBench *b = zboot(NULL, 0);

    zrun_rises(b, 4);
    b->high_once = ztiming(b)[ZT_WCH] - NS(5);
    zrun_rises(b, 3);
    expect(zstatus_is(b, CPU_LOST), "Z80 14: a high phase below tWCH: LOST");
    zbench_close(b);
}

/* 15. The program of the sub-plan's outcome. */
static void zcase_program(void) {
    static const uint8_t PROGRAM[] = {0x3E, 0x12, 0x01, 0x56, 0x34, 0x32, 0x00, 0x80,
                                      0xD3, 0x10, 0x76};
    static const size_t LENGTHS[] = {4, 3, 4, 3, 3, 4, 3, 3, 3, 4, 3, 4, 4};
    ZBench *b = zboot(PROGRAM, sizeof PROGRAM);
    CpuState before, state;
    long w, o;
    size_t i;
    int ok = 1;

    while (zfind(b, 'F', 0x000A, 0) < 0) {
        zrun_rises(b, 1);
    }
    zstate(b, &before);
    zrun_rises(b, 12);
    zstate(b, &state);
    for (i = 0; i < sizeof LENGTHS / sizeof LENGTHS[0]; i++) {
        ok = ok && zlength(b, (long)i) == LENGTHS[i];
    }
    w = zfind(b, 'W', 0x8000, 0);
    o = zfind(b, 'O', 0x1210, 0);
    expect(ok, "Z80 15: MCs of 4,3 / 4,3,3 / 4,3,3,3 / 4,3,4 / 4 T-states");
    expect(w >= 0 && b->log[w].data == 0x12 && o >= 0 && b->log[o].data == 0x12,
           "Z80 15: the write of 12 to 8000 and the I/O write of 12 to port 1210");
    expect(zreg(&state, "BC") == 0x3456 && state.status == CPU_HALTED && zpin(b, b->halt) == LOGIC_0 &&
               strcmp(state.instruction_text, "HALT") == 0,
           "Z80 15: BC = 3456, status HALTED, last instruction HALT");
    expect(before.has_instruction && strcmp(before.instruction_text, "OUT (10h),A") == 0 &&
               before.instruction_address == 0x0008,
           "Z80 15: the instruction before it reads back as OUT (10h),A");
    zbench_close(b);
}

int main(void) {
    size_t i;

    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0 || load_fixture() != 0) {
        return 2;
    }
    for (i = 0; i < sizeof CORE_NAMES / sizeof CORE_NAMES[0]; i++) {
    }
    zcase_power_up();
    zcase_reset();
    zcase_short_reset();
    zcase_reset_setup();
    zcase_fetch();
    zcase_memory();
    zcase_io();
    zcase_wait();
    zcase_setup();
    zcase_im1();
    zcase_im2_im0();
    zcase_nmi();
    zcase_halt();
    zcase_busreq();
    zcase_clock();
    zcase_program();

    part_library_destroy(library);
    if (failures > 0) {
        printf("cpu_test: %d failed\n", failures);
        return 1;
    }
    printf("cpu_test: all passed\n");
    return 0;
}
