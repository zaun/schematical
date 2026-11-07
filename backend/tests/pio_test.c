/*
 * pio_test: the pio kind's models and the 65xx bus helper (plan-14).
 *
 * A1-A8 drive bus65 through a test-only component (16 registers in an
 * array) on the shared bench, with the W65C22S 14 MHz timing (tacr 10, tcar
 * 10, tcdr 20, thr 10, tacw 10, tcaw 10, tdcw 10, thw 10 ns; tcyc 70, tpwh
 * 35, tpwl 35; w65c22.pdf p.41).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "bus65.h"
#include "kind.h"

static int failures;

#define CHECK(cond)                                                        \
    do {                                                                   \
        if (!(cond)) {                                                     \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);         \
            failures++;                                                    \
        }                                                                  \
    } while (0)

#define NS 1000ull
#define HALF (500 * NS)   /* 1 MHz PHI2 */

/* ---- The test component ----------------------------------------------------- */

enum {
    P_PHI2, P_RWB, P_CS1, P_CS2B, P_RS0, P_RS1, P_RS2, P_RS3,
    P_D0, P_D1, P_D2, P_D3, P_D4, P_D5, P_D6, P_D7, P_COUNT
};

static const PartPin TEST_PINS[P_COUNT] = {
    {"1", "PHI2", PIN_INPUT, NULL}, {"2", "RWB", PIN_INPUT, NULL},
    {"3", "CS1", PIN_INPUT, NULL}, {"4", "CS2B", PIN_INPUT, NULL},
    {"5", "RS0", PIN_INPUT, NULL}, {"6", "RS1", PIN_INPUT, NULL},
    {"7", "RS2", PIN_INPUT, NULL}, {"8", "RS3", PIN_INPUT, NULL},
    {"9", "D0", PIN_INOUT, "data"}, {"10", "D1", PIN_INOUT, "data"},
    {"11", "D2", PIN_INOUT, "data"}, {"12", "D3", PIN_INOUT, "data"},
    {"13", "D4", PIN_INOUT, "data"}, {"14", "D5", PIN_INOUT, "data"},
    {"15", "D6", PIN_INOUT, "data"}, {"16", "D7", PIN_INOUT, "data"},
};

static const Bus65Pins TEST_BUS_PINS = {
    P_PHI2, P_RWB, {P_D0, P_D1, P_D2, P_D3, P_D4, P_D5, P_D6, P_D7},
    {P_CS1}, 1, {P_CS2B}, 1, {P_RS0, P_RS1, P_RS2, P_RS3}, 4
};

static const Bus65Timing TEST_TIMING = {
    10 * NS, 10 * NS, 20 * NS, 10 * NS, 10 * NS, 10 * NS, 10 * NS, 10 * NS,
    70 * NS, 35 * NS, 35 * NS
};

typedef struct {
    Bus65 bus;
    uint8_t regs[16], known[16];
    unsigned reads, writes, cycles;
} TestRegs;

static int t_read(void *model, unsigned reg, uint8_t *value, uint8_t *known) {
    TestRegs *t = model;

    t->reads++;
    *value = t->regs[reg];
    *known = t->known[reg];
    return 1;
}

static void t_write(void *model, unsigned reg, uint8_t value, uint8_t known) {
    TestRegs *t = model;

    t->writes++;
    t->regs[reg] = value & known;
    t->known[reg] = known;
}

static void t_cycle(void *model) {
    ((TestRegs *)model)->cycles++;
}

static const Bus65Ops TEST_OPS = {t_read, t_write, t_cycle};

static size_t t_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *t_create(const void *config) {
    TestRegs *t = calloc(1, sizeof(*t));

    (void)config;
    bus65_init(&t->bus, &TEST_BUS_PINS, &TEST_TIMING, &TEST_OPS, t);
    return t;
}

static void t_destroy(void *state) {
    free(state);
}

static void t_start(Sim *sim, SimInstance *instance, void *state) {
    bus65_start(&((TestRegs *)state)->bus, sim, instance);
}

static void t_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                          LogicValue old_value, LogicValue new_value) {
    bus65_pin_changed(&((TestRegs *)state)->bus, sim, instance, pin, old_value, new_value);
}

static void t_timer(Sim *sim, SimInstance *instance, void *state, uint32_t timer_id) {
    bus65_timer(&((TestRegs *)state)->bus, sim, instance, timer_id);
}

static const SimComponentType TEST_TYPE = {
    "bus65-test", t_pin_count, t_create, t_destroy, t_start, t_pin_changed, t_timer
};

static const PartKindOps TEST_KIND = {PART_PIO, "bus65-test", &TEST_TYPE};
static const PartInternal TEST_INTERNAL = {&TEST_KIND, NULL, NULL, NULL};

static Part test_part(void) {
    Part part;

    memset(&part, 0, sizeof part);
    part.kind = PART_PIO;
    part.name = "BUS65";
    part.pins = TEST_PINS;
    part.pin_count = P_COUNT;
    part.internal = &TEST_INTERNAL;
    return part;
}

/* ---- Bench helpers ----------------------------------------------------------------- */

static const char *const DATA_NAMES[8] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"};
static const char *const RS_NAMES[4] = {"RS0", "RS1", "RS2", "RS3"};

static LogicValue bit_value(unsigned v, int i) {
    return (v >> i & 1) ? LOGIC_1 : LOGIC_0;
}

static void set_rs(Bench *b, unsigned reg) {
    int i;

    for (i = 0; i < 4; i++) bench_set(b, RS_NAMES[i], bit_value(reg, i));
}

static void set_data(Bench *b, unsigned v) {
    int i;

    for (i = 0; i < 8; i++) bench_set(b, DATA_NAMES[i], bit_value(v, i));
}

static void release_data(Bench *b) {
    int i;

    for (i = 0; i < 8; i++) bench_set(b, DATA_NAMES[i], LOGIC_Z);
}

/* 'v' when every data pin is 0/1 (*value set), 'X' when all are X, 'Z' when all Z, else '?'. */
static char data_state(Bench *b, unsigned *value) {
    int x = 0, z = 0, k = 0;
    int i;

    *value = 0;
    for (i = 0; i < 8; i++) {
        LogicValue v = bench_get(b, DATA_NAMES[i]);

        if (v == LOGIC_X) x++;
        else if (v == LOGIC_Z) z++;
        else {
            k++;
            if (v == LOGIC_1) *value |= 1u << i;
        }
    }
    return k == 8 ? 'v' : x == 8 ? 'X' : z == 8 ? 'Z' : '?';
}

static TestRegs *regs_of(Bench *b) {
    return bench_state(b);
}

/* A bench with PHI2 low, selected (CS1 1, CS2B 0), RWB 1, RS 0, run 1 us. */
static Bench *new_bench(const Part *part) {
    Bench *b = bench_create(part);

    bench_set(b, "PHI2", LOGIC_0);
    bench_set(b, "CS1", LOGIC_1);
    bench_set(b, "CS2B", LOGIC_0);
    bench_set(b, "RWB", LOGIC_1);
    set_rs(b, 0);
    bench_run(b, 2 * HALF);
    return b;
}

/* One full PHI2 cycle from PHI2 low: controls set now, the low phase, the
   high phase (write data from 100 ns into it), PHI2 falls; data released
   30 ns after the fall. Ends 30 ns after the fall. */
static void bus_write(Bench *b, unsigned reg, unsigned value) {
    bench_set(b, "RWB", LOGIC_0);
    set_rs(b, reg);
    bench_run(b, HALF);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, 100 * NS);
    set_data(b, value);
    bench_run(b, HALF - 100 * NS);
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 30 * NS);
    release_data(b);
    bench_set(b, "RWB", LOGIC_1);
    bench_run(b, 1);
}

/* ---- A: bus65 ------------------------------------------------------------------- */

static void a1_read(const Part *part) {
    Bench *b = new_bench(part);
    int before = failures;
    unsigned v;
    char s;

    bus_write(b, 3, 0x5A);
    CHECK(regs_of(b)->regs[3] == 0x5A && regs_of(b)->known[3] == 0xFF);
    set_rs(b, 3);
    bench_run(b, HALF);
    CHECK(data_state(b, &v) == 'Z');
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, 1);
    CHECK(data_state(b, &v) == 'X');
    bench_run(b, 20 * NS - 2);
    CHECK(data_state(b, &v) == 'X');
    bench_run(b, 1);                 /* rise + 20 ns */
    s = data_state(b, &v);
    CHECK(s == 'v' && v == 0x5A);
    bench_run(b, HALF - 20 * NS);
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 10 * NS - 1);
    s = data_state(b, &v);
    CHECK(s == 'v' && v == 0x5A);
    bench_run(b, 1);                 /* fall + 10 ns */
    CHECK(data_state(b, &v) == 'Z');
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s A1 read\n", failures > before ? "FAIL" : "ok");
}

static void a2_write(const Part *part) {
    Bench *b = new_bench(part);
    int before = failures;

    /* Valid 15 ns before the fall: stored. */
    bench_set(b, "RWB", LOGIC_0);
    set_rs(b, 5);
    bench_run(b, HALF);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, HALF - 15 * NS);
    set_data(b, 0xC3);
    bench_run(b, 15 * NS);
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 30 * NS);
    CHECK(regs_of(b)->regs[5] == 0xC3 && regs_of(b)->known[5] == 0xFF);

    /* 0x0F early, then 0x3C 5 ns before the fall: the changed bits (0x33) unknown. */
    set_rs(b, 6);
    bench_run(b, HALF - 30 * NS);
    bench_set(b, "PHI2", LOGIC_1);
    set_data(b, 0x0F);
    bench_run(b, HALF - 5 * NS);
    set_data(b, 0x3C);
    bench_run(b, 5 * NS);
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 30 * NS);
    CHECK(regs_of(b)->known[6] == (uint8_t)~0x33);
    CHECK(regs_of(b)->regs[6] == (0x3C & ~0x33));

    /* Hold: D7 changes 5 ns after the fall -> bit 7 unknown. */
    set_rs(b, 7);
    bench_run(b, HALF - 30 * NS);
    bench_set(b, "PHI2", LOGIC_1);
    set_data(b, 0xFF);
    bench_run(b, HALF);
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 5 * NS);
    bench_set(b, "D7", LOGIC_0);
    bench_run(b, 30 * NS);
    CHECK(regs_of(b)->known[7] == 0x7F && regs_of(b)->regs[7] == 0x7F);
    /* 15 ns after the fall: past thw, kept. */
    set_rs(b, 8);
    bench_run(b, HALF - 35 * NS);
    bench_set(b, "PHI2", LOGIC_1);
    set_data(b, 0xFF);
    bench_run(b, HALF);
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 15 * NS);
    release_data(b);
    bench_run(b, 30 * NS);
    CHECK(regs_of(b)->known[8] == 0xFF && regs_of(b)->regs[8] == 0xFF);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s A2 write\n", failures > before ? "FAIL" : "ok");
}

static void a3_setup(const Part *part) {
    Bench *b = new_bench(part);
    int before = failures;
    unsigned reads;
    unsigned v;

    bus_write(b, 2, 0x11);
    set_rs(b, 2);
    bench_run(b, HALF - 5 * NS);
    set_rs(b, 3);                    /* RS changes 5 ns before the rise */
    bench_run(b, 5 * NS);
    reads = regs_of(b)->reads;
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, 30 * NS);
    CHECK(data_state(b, &v) == 'X');
    CHECK(regs_of(b)->reads == reads);
    bench_run(b, HALF - 30 * NS);
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 10 * NS);
    CHECK(data_state(b, &v) == 'Z');
    /* 15 ns before the rise: a known read. */
    set_rs(b, 2);
    bench_run(b, HALF - 25 * NS);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, 30 * NS);
    CHECK(data_state(b, &v) == 'v' && v == 0x11);
    CHECK(regs_of(b)->reads == reads + 1);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s A3 setup\n", failures > before ? "FAIL" : "ok");
}

static void a4_hold(const Part *part) {
    Bench *b = new_bench(part);
    int before = failures;
    unsigned v;

    bus_write(b, 4, 0x77);
    set_rs(b, 4);
    bench_run(b, HALF);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, 5 * NS);
    bench_set(b, "CS1", LOGIC_0);    /* 5 ns after the rise */
    bench_run(b, 20 * NS);
    CHECK(data_state(b, &v) == 'X');
    bench_run(b, HALF - 25 * NS);
    CHECK(data_state(b, &v) == 'X');
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 10 * NS - 1);
    CHECK(data_state(b, &v) == 'X');
    bench_run(b, 1);
    CHECK(data_state(b, &v) == 'Z');
    /* A change 15 ns after the rise is past tcar: the read stands. */
    bench_set(b, "CS1", LOGIC_1);
    bench_run(b, HALF);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, 15 * NS);
    bench_set(b, "CS1", LOGIC_0);
    bench_run(b, 10 * NS);
    CHECK(data_state(b, &v) == 'v' && v == 0x77);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s A4 hold\n", failures > before ? "FAIL" : "ok");
}

static void a5_not_selected(const Part *part) {
    Bench *b = new_bench(part);
    int before = failures;
    unsigned reads, writes;
    unsigned v;

    bench_set(b, "CS2B", LOGIC_1);
    bench_run(b, 1);
    reads = regs_of(b)->reads;
    writes = regs_of(b)->writes;
    bus_write(b, 1, 0x99);
    set_rs(b, 1);
    bench_run(b, HALF);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, 1);
    CHECK(data_state(b, &v) == 'Z');
    bench_run(b, 100 * NS);
    CHECK(data_state(b, &v) == 'Z');
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 50 * NS);
    CHECK(data_state(b, &v) == 'Z');
    CHECK(regs_of(b)->reads == reads && regs_of(b)->writes == writes);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s A5 not selected\n", failures > before ? "FAIL" : "ok");
}

static void a6_phi2_x(const Part *part) {
    Bench *b = new_bench(part);
    int before = failures;
    unsigned v;
    int r;
    int all = 1;

    bus_write(b, 9, 0x42);
    CHECK(regs_of(b)->known[9] == 0xFF);
    bench_set(b, "PHI2", LOGIC_X);
    bench_run(b, 1);
    for (r = 0; r < 16; r++) all &= regs_of(b)->known[r] == 0;
    CHECK(all);
    CHECK(data_state(b, &v) == 'X');
    bench_run(b, HALF);
    CHECK(data_state(b, &v) == 'X');
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 1);
    CHECK(data_state(b, &v) == 'Z');
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s A6 PHI2 X\n", failures > before ? "FAIL" : "ok");
}

static void a7_cycle_hook(const Part *part) {
    Bench *b = new_bench(part);
    int before = failures;
    unsigned cycles = regs_of(b)->cycles;
    int i;

    bus_write(b, 0, 0x01);           /* selected */
    CHECK(regs_of(b)->cycles == cycles + 1);
    bench_set(b, "CS1", LOGIC_0);
    for (i = 0; i < 3; i++) {        /* not selected */
        bench_run(b, HALF);
        bench_set(b, "PHI2", LOGIC_1);
        bench_run(b, HALF);
        bench_set(b, "PHI2", LOGIC_0);
        bench_run(b, 1);
    }
    CHECK(regs_of(b)->cycles == cycles + 4);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s A7 cycle hook\n", failures > before ? "FAIL" : "ok");
}

/* Added in A: PHI2 phases shorter than tpwh/tpwl, or rises closer than
   tcyc, make every register unknown. */
static void a8_widths(const Part *part) {
    Bench *b = new_bench(part);
    int before = failures;
    int r;
    int all = 1;

    bus_write(b, 3, 0x33);
    CHECK(regs_of(b)->known[3] == 0xFF);
    bench_run(b, HALF);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, 20 * NS);           /* high 20 ns < tpwh 35 */
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 1);
    for (r = 0; r < 16; r++) all &= regs_of(b)->known[r] == 0;
    CHECK(all);
    /* A legal cycle afterwards works. */
    bench_run(b, HALF);
    bus_write(b, 3, 0x34);
    CHECK(regs_of(b)->regs[3] == 0x34 && regs_of(b)->known[3] == 0xFF);
    /* Low 20 ns < tpwl. */
    bench_run(b, HALF);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, HALF);
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 20 * NS);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, 1);
    CHECK(regs_of(b)->known[3] == 0);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s A8 PHI2 widths\n", failures > before ? "FAIL" : "ok");
}

int main(void) {
    Part part = test_part();

    a1_read(&part);
    a2_write(&part);
    a3_setup(&part);
    a4_hold(&part);
    a5_not_selected(&part);
    a6_phi2_x(&part);
    a7_cycle_hook(&part);
    a8_widths(&part);
    if (failures > 0) {
        printf("%d failures\n", failures);
        return 1;
    }
    return 0;
}
