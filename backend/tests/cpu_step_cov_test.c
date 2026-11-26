/*
 * cpu_step_cov_test (plan-68-S): the Z80 (cpuz80.c) and 65816 (cpu65816.c)
 * adapters in the states cpu_z80_test and cpu_65816_test do not drive: X on
 * INT#, NMI#, WAIT#, RESET#, BUSREQ#, the clock, RESB or ABORTB, unknown
 * memory or I/O data, WAIT# in a fetch, and an instruction with internal
 * T-states. The shipped Z84C0010PEG and W65C816S6TPG-14 run on the generic
 * pin bench (support/bench.c), the test stepping the clock at 1 MHz and
 * serving memory and I/O from arrays between edges.
 *
 * The rule (cpu.h, the differential rule): an X input that could change
 * what the CPU does makes it LOST; one that cannot (INT# with interrupts
 * disabled) leaves it running. Z80 timing (Z84C00 product spec): WAIT# low
 * at the T2 fall inserts wait states for as long as it stays low; LD SP,HL
 * takes 6 T-states.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "cpu_state.h"
#include "kind.h"
#include "parts.h"
#include "read_file.h"

#define NS 1000ull
#define HALF (500 * NS)

static int failures;
static PartLibrary *library;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "ok" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

static const char *const DATA[8] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"};

static CpuStatus status_of(Bench *b, const Part *part) {
    CpuState state;

    memset(&state, 0, sizeof state);
    if (!kind_model(part)->cpu_state(bench_state(b), &state)) {
        return CPU_LOST;
    }
    return state.status;
}

/* ---- Z80 -------------------------------------------------------------------------- */

typedef struct {
    Bench *b;
    const Part *part;
    uint8_t mem[65536], known[65536];
    uint8_t io;
    int io_known;
    int clk;
    unsigned m1_falls;
    int m1_was;
    unsigned rises;
} Z80Rig;

static int zaddress(Z80Rig *z) {
    char name[8];
    int a = 0;
    int i;

    for (i = 0; i < 16; i++) {
        LogicValue v;

        snprintf(name, sizeof name, "A%d", i);
        v = bench_get(z->b, name);
        if (v != LOGIC_0 && v != LOGIC_1) return -1;
        if (v == LOGIC_1) a |= 1 << i;
    }
    return a;
}

static void zdata(Z80Rig *z, int value, int known) {
    int i;

    for (i = 0; i < 8; i++) {
        bench_set(z->b, DATA[i], value < 0 ? LOGIC_Z : !known ? LOGIC_X
                                 : (value >> i & 1) ? LOGIC_1 : LOGIC_0);
    }
}

/* 200 ns after an edge: serve a read (memory or I/O), else float the bus. */
static void zserve(Z80Rig *z) {
    int rd = bench_get(z->b, "RD#") == LOGIC_0;
    int mreq = bench_get(z->b, "MREQ#") == LOGIC_0;
    int iorq = bench_get(z->b, "IORQ#") == LOGIC_0;
    int a = zaddress(z);

    if (rd && mreq && a >= 0) {
        zdata(z, z->mem[a], z->known[a]);
    } else if (rd && iorq) {
        zdata(z, z->io, z->io_known);
    } else {
        zdata(z, -1, 0);
    }
}

/* One clock edge (rise when level 1) and the half period after it. */
static void zedge(Z80Rig *z, int level) {
    LogicValue m1;

    z->clk = level;
    bench_set(z->b, "CLK", level ? LOGIC_1 : LOGIC_0);
    bench_run(z->b, 200 * NS);
    zserve(z);
    m1 = bench_get(z->b, "M1#");
    if (m1 == LOGIC_0 && z->m1_was != LOGIC_0) z->m1_falls++;
    z->m1_was = m1;
    if (level) z->rises++;
    bench_run(z->b, HALF - 200 * NS);
}

static void zclocks(Z80Rig *z, unsigned n) {
    while (n-- > 0) {
        zedge(z, 1);
        zedge(z, 0);
    }
}

/* A Z80 with every input inactive, reset for 4 clocks; memory all NOP. */
static Z80Rig *zopen(const uint8_t *program, size_t count) {
    Z80Rig *z = calloc(1, sizeof *z);

    z->part = part_library_find(library, "Z84C0010PEG");
    z->b = bench_create(z->part);
    memset(z->known, 1, sizeof z->known);
    if (program != NULL) memcpy(z->mem, program, count);
    z->io_known = 1;
    bench_set(z->b, "INT#", LOGIC_1);
    bench_set(z->b, "NMI#", LOGIC_1);
    bench_set(z->b, "WAIT#", LOGIC_1);
    bench_set(z->b, "BUSREQ#", LOGIC_1);
    bench_set(z->b, "RESET#", LOGIC_0);
    zdata(z, -1, 0);
    zclocks(z, 4);
    bench_set(z->b, "RESET#", LOGIC_1);
    zclocks(z, 2);
    return z;
}

static void zclose(Z80Rig *z) {
    bench_destroy(z->b);
    free(z);
}

static int zrunning(Z80Rig *z) {
    return status_of(z->b, z->part) == CPU_RUNNING;
}

static int zlost(Z80Rig *z) {
    return status_of(z->b, z->part) == CPU_LOST;
}

static void z80_int_x(void) {
    static const uint8_t EI[] = {0xFB};
    Z80Rig *z = zopen(NULL, 0);

    zclocks(z, 8);
    bench_set(z->b, "INT#", LOGIC_X);
    zclocks(z, 12);
    expect(zrunning(z), "Z80: INT# X with interrupts disabled (after reset): RUNNING");
    zclose(z);

    z = zopen(EI, sizeof EI);
    zclocks(z, 12);
    bench_set(z->b, "INT#", LOGIC_X);
    zclocks(z, 12);
    expect(zlost(z), "Z80: INT# X after EI: LOST");
    zclose(z);
}

/* NMI# X set just before each rise of an instruction in turn: wherever it
   lands, LOST. */
static void z80_nmi_x(const uint8_t *program, size_t count, unsigned skip, const char *what) {
    unsigned k;
    int all = 1;

    for (k = 0; k < 12; k++) {
        Z80Rig *z = zopen(program, count);

        zclocks(z, skip + k);
        bench_set(z->b, "NMI#", LOGIC_X);
        zclocks(z, 6);
        all &= zlost(z);
        zclose(z);
    }
    expect(all, what);
}

static void z80_wait_in_fetch(void) {
    Z80Rig *z = zopen(NULL, 0);
    unsigned falls;

    zclocks(z, 8);
    falls = z->m1_falls;
    /* WAIT# low from a T1: M1# stays low while it is low (Tw states). */
    while (bench_get(z->b, "M1#") != LOGIC_0) zclocks(z, 1);
    bench_set(z->b, "WAIT#", LOGIC_0);
    zclocks(z, 10);
    expect(bench_get(z->b, "M1#") == LOGIC_0 && zrunning(z),
           "Z80: WAIT# low in a fetch: M1# held low (wait states), RUNNING");
    bench_set(z->b, "WAIT#", LOGIC_1);
    falls = z->m1_falls;
    zclocks(z, 8);
    expect(z->m1_falls == falls + 2 && zrunning(z),
           "Z80: WAIT# high again: the NOPs go on, a fetch every 4 clocks");
    zclose(z);
}

/* LD SP,HL: 6 T-states, the last two internal (no machine cycle). */
static void z80_internal_states(void) {
    Z80Rig *z = zopen(NULL, 0);
    unsigned falls;

    memset(z->mem, 0xF9, sizeof z->mem);
    zclocks(z, 8);
    falls = z->m1_falls;
    zclocks(z, 24);
    expect(z->m1_falls == falls + 4 && zrunning(z), "Z80: LD SP,HL every 6 clocks (4 + 2 internal)");
    zclose(z);
}

static void z80_unknown_data(void) {
    static const uint8_t LD[] = {0x3A, 0x00, 0x80};         /* LD A,($8000) */
    static const uint8_t IN[] = {0xDB, 0x10};               /* IN A,($10) */
    static const uint8_t OUT[] = {0xD3, 0x10};              /* OUT ($10),A */
    Z80Rig *z = zopen(LD, sizeof LD);

    z->known[0x8000] = 0;
    zclocks(z, 16);
    expect(zlost(z), "Z80: LD A,($8000) reading X: LOST");
    zclose(z);

    z = zopen(IN, sizeof IN);
    z->io_known = 0;
    zclocks(z, 16);
    expect(zlost(z), "Z80: IN A,($10) reading X: LOST");
    zclose(z);

    /* WAIT# X through the I/O cycle: maybe a wait state: LOST. */
    z = zopen(OUT, sizeof OUT);
    zclocks(z, 7);
    bench_set(z->b, "WAIT#", LOGIC_X);
    zclocks(z, 6);
    expect(zlost(z), "Z80: WAIT# X in OUT's I/O cycle: LOST");
    zclose(z);
}

static void z80_control_x(void) {
    Z80Rig *z = zopen(NULL, 0);

    zclocks(z, 6);
    bench_set(z->b, "RESET#", LOGIC_X);
    zclocks(z, 2);
    expect(zlost(z), "Z80: RESET# X: LOST");
    zclose(z);

    z = zopen(NULL, 0);
    zclocks(z, 6);
    bench_set(z->b, "BUSREQ#", LOGIC_X);
    zclocks(z, 2);
    expect(zlost(z), "Z80: BUSREQ# X: LOST");
    zclose(z);

    z = zopen(NULL, 0);
    zclocks(z, 6);
    bench_set(z->b, "CLK", LOGIC_X);
    bench_run(z->b, 100 * NS);
    expect(zlost(z) && bench_get(z->b, "M1#") == LOGIC_X && bench_get(z->b, "MREQ#") == LOGIC_X,
           "Z80: CLK X: LOST, the outputs X");
    zclose(z);
}

/* ---- 65816 ------------------------------------------------------------------------ */

typedef struct {
    Bench *b;
    const Part *part;
} Rig816;

/* One PHI2 cycle: low 500 ns, high 500 ns. The bench drives NOP (EA) on a
   read from rise + 100 ns to fall + 20 ns (the data hold), and floats the
   bus otherwise (the bank address is out while PHI2 is low). */
static void c816_cycle(Rig816 *r) {
    int i;

    bench_set(r->b, "PHI2", LOGIC_0);
    bench_run(r->b, 20 * NS);
    for (i = 0; i < 8; i++) bench_set(r->b, DATA[i], LOGIC_Z);
    bench_run(r->b, HALF - 20 * NS);
    bench_set(r->b, "PHI2", LOGIC_1);
    bench_run(r->b, 100 * NS);
    if (bench_get(r->b, "RWB") == LOGIC_1) {
        for (i = 0; i < 8; i++) bench_set(r->b, DATA[i], (0xEA >> i & 1) ? LOGIC_1 : LOGIC_0);
    }
    bench_run(r->b, HALF - 100 * NS);
}

static void c816_cycles(Rig816 *r, unsigned n) {
    while (n-- > 0) c816_cycle(r);
}

/* Every byte reads EA: the reset vector is EAEA and the program NOPs. */
static Rig816 *c816_open(void) {
    Rig816 *r = calloc(1, sizeof *r);
    static const char *const HIGH[] = {"IRQB", "NMIB", "RDY", "ABORTB", "BE"};
    size_t i;

    r->part = part_library_find(library, "W65C816S6TPG-14");
    r->b = bench_create(r->part);
    for (i = 0; i < sizeof HIGH / sizeof HIGH[0]; i++) bench_set(r->b, HIGH[i], LOGIC_1);
    bench_set(r->b, "RESB", LOGIC_0);
    c816_cycles(r, 4);
    bench_run(r->b, 100 * NS);
    bench_set(r->b, "RESB", LOGIC_1);
    c816_cycles(r, 12);
    return r;
}

static void c816_close(Rig816 *r) {
    bench_destroy(r->b);
    free(r);
}

static void c816_x(const char *pin, const char *what) {
    Rig816 *r = c816_open();

    expect(status_of(r->b, r->part) == CPU_RUNNING, "65816: running NOPs after reset");
    bench_set(r->b, pin, LOGIC_X);
    c816_cycles(r, 3);
    expect(status_of(r->b, r->part) == CPU_LOST, what);
    c816_close(r);
}

static void c816_clock_x(void) {
    Rig816 *r = c816_open();

    bench_set(r->b, "PHI2", LOGIC_X);
    bench_run(r->b, 100 * NS);
    expect(status_of(r->b, r->part) == CPU_LOST, "65816: PHI2 X: LOST");
    c816_close(r);
}

int main(void) {
    static const uint8_t EI_NOP[] = {0xFB};
    static const uint8_t IN_LOOP[] = {0xFB, 0xDB, 0x10, 0xC3, 0x01, 0x00};   /* EI; IN A,($10); JP 1 */

    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        return 2;
    }
    z80_int_x();
    z80_nmi_x(EI_NOP, sizeof EI_NOP, 6, "Z80: NMI# X at any rise of a NOP: LOST");
    z80_nmi_x(IN_LOOP, sizeof IN_LOOP, 10, "Z80: NMI# X at any rise of IN A,(n): LOST");
    z80_wait_in_fetch();
    z80_internal_states();
    z80_unknown_data();
    z80_control_x();
    c816_x("RESB", "65816: RESB X while running: LOST");
    c816_x("ABORTB", "65816: ABORTB X while running: LOST");
    c816_clock_x();
    part_library_destroy(library);
    if (failures > 0) {
        printf("cpu_step_cov_test: %d failures\n", failures);
        return 1;
    }
    printf("cpu_step_cov_test: all passed\n");
    return 0;
}
