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
#include "read_file.h"
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
    unsigned reads, writes, cycles, unknown_reads;
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

static void t_read_unknown(void *model) {
    ((TestRegs *)model)->unknown_reads++;
}

static const Bus65Ops TEST_OPS = {t_read, t_write, t_cycle, t_read_unknown};

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

    for (i = 0; i < 4 && bench_has_pin(b, RS_NAMES[i]); i++) {
        bench_set(b, RS_NAMES[i], bit_value(reg, i));
    }
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
    CHECK(regs_of(b)->unknown_reads == 1);
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
    CHECK(regs_of(b)->unknown_reads == 1);
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

/* ---- B: pia6521 (the W65C21N device) ------------------------------------------- */

/* The W65C21N device of pio.json (p40 pinout, one part). */
static const char W65C21N_INLINE[] =
    "{\"id\":\"W65C21N\",\"function\":\"PIA\",\"description\":\"Peripheral interface adapter, NM"
    "OS-compatible (WDC W65C21N)\",\"drawer\":[\"Interface\"],\"note\":\"W65C21 (datasheets/p"
    "io/w65c21.pdf): pinouts p.3, AC timing p.8, timing figures p.9-10, registers Tab"
    "les 1-2 p.11, port buffers p.11 and p.16, control lines p.14-15 and Table 3 p.18"
    ", ordering p.20. The N and S dies differ only in port buffer currents (not model"
    "ed).\",\"model\":\"pia6521\",\"phi2\":\"PHI2\",\"rw\":\"RWB\",\"reset\":\"RESB\",\"data\":[\"D0\",\"D1"
    "\",\"D2\",\"D3\",\"D4\",\"D5\",\"D6\",\"D7\"],\"csHigh\":[\"CS0\",\"CS1\"],\"csLow\":[\"CS2B\"],\"rs\":[\""
    "RS0\",\"RS1\"],\"pa\":[\"PA0\",\"PA1\",\"PA2\",\"PA3\",\"PA4\",\"PA5\",\"PA6\",\"PA7\"],\"pb\":[\"PB0\",\""
    "PB1\",\"PB2\",\"PB3\",\"PB4\",\"PB5\",\"PB6\",\"PB7\"],\"ca1\":\"CA1\",\"ca2\":\"CA2\",\"cb1\":\"CB1\",\"c"
    "b2\":\"CB2\",\"irqa\":\"IRQAB\",\"irqb\":\"IRQBB\",\"grades\":{\"14\":{\"tcyc\":70,\"tc\":35,\"tacr\""
    ":8,\"tcar\":0,\"tpcr\":10,\"tcdr\":20,\"thr\":5,\"tacw\":8,\"tcaw\":0,\"tdcw\":10,\"thw\":5,\"tcp"
    "w\":20,\"tca2\":20,\"tca2h\":20,\"trs2\":25,\"tcb2\":70,\"tcb2h\":20,\"tdc\":5,\"pwi\":70,\"trs3"
    "\":20,\"tir\":25,\"vcc\":[4.5,5.5],\"note\":\"AC timing p.8, the one 14 MHz @ 5 V table "
    "(both dies). pwi is printed in the Max column; it is a minimum. VDD 5 V +/- 10% "
    "(p.2).\"}},\"pinouts\":{\"p40\":[{\"number\":1,\"name\":\"VSS\",\"role\":\"ground\"},{\"number\":"
    "2,\"name\":\"PA0\",\"role\":\"inout\",\"group\":\"port A\"},{\"number\":3,\"name\":\"PA1\",\"role\":"
    "\"inout\",\"group\":\"port A\"},{\"number\":4,\"name\":\"PA2\",\"role\":\"inout\",\"group\":\"port "
    "A\"},{\"number\":5,\"name\":\"PA3\",\"role\":\"inout\",\"group\":\"port A\"},{\"number\":6,\"name\""
    ":\"PA4\",\"role\":\"inout\",\"group\":\"port A\"},{\"number\":7,\"name\":\"PA5\",\"role\":\"inout\","
    "\"group\":\"port A\"},{\"number\":8,\"name\":\"PA6\",\"role\":\"inout\",\"group\":\"port A\"},{\"nu"
    "mber\":9,\"name\":\"PA7\",\"role\":\"inout\",\"group\":\"port A\"},{\"number\":10,\"name\":\"PB0\","
    "\"role\":\"inout\",\"group\":\"port B\"},{\"number\":11,\"name\":\"PB1\",\"role\":\"inout\",\"group"
    "\":\"port B\"},{\"number\":12,\"name\":\"PB2\",\"role\":\"inout\",\"group\":\"port B\"},{\"number\""
    ":13,\"name\":\"PB3\",\"role\":\"inout\",\"group\":\"port B\"},{\"number\":14,\"name\":\"PB4\",\"rol"
    "e\":\"inout\",\"group\":\"port B\"},{\"number\":15,\"name\":\"PB5\",\"role\":\"inout\",\"group\":\"p"
    "ort B\"},{\"number\":16,\"name\":\"PB6\",\"role\":\"inout\",\"group\":\"port B\"},{\"number\":17,"
    "\"name\":\"PB7\",\"role\":\"inout\",\"group\":\"port B\"},{\"number\":18,\"name\":\"CB1\",\"role\":\""
    "input\"},{\"number\":19,\"name\":\"CB2\",\"role\":\"inout\"},{\"number\":20,\"name\":\"VDD\",\"rol"
    "e\":\"power\"},{\"number\":21,\"name\":\"RWB\",\"role\":\"input\"},{\"number\":22,\"name\":\"CS0\","
    "\"role\":\"input\"},{\"number\":23,\"name\":\"CS2B\",\"role\":\"input\"},{\"number\":24,\"name\":\""
    "CS1\",\"role\":\"input\"},{\"number\":25,\"name\":\"PHI2\",\"role\":\"input\"},{\"number\":26,\"na"
    "me\":\"D7\",\"role\":\"inout\",\"group\":\"data\"},{\"number\":27,\"name\":\"D6\",\"role\":\"inout\","
    "\"group\":\"data\"},{\"number\":28,\"name\":\"D5\",\"role\":\"inout\",\"group\":\"data\"},{\"number"
    "\":29,\"name\":\"D4\",\"role\":\"inout\",\"group\":\"data\"},{\"number\":30,\"name\":\"D3\",\"role\":"
    "\"inout\",\"group\":\"data\"},{\"number\":31,\"name\":\"D2\",\"role\":\"inout\",\"group\":\"data\"},"
    "{\"number\":32,\"name\":\"D1\",\"role\":\"inout\",\"group\":\"data\"},{\"number\":33,\"name\":\"D0\""
    ",\"role\":\"inout\",\"group\":\"data\"},{\"number\":34,\"name\":\"RESB\",\"role\":\"input\"},{\"num"
    "ber\":35,\"name\":\"RS1\",\"role\":\"input\"},{\"number\":36,\"name\":\"RS0\",\"role\":\"input\"},{"
    "\"number\":37,\"name\":\"IRQBB\",\"role\":\"output\"},{\"number\":38,\"name\":\"IRQAB\",\"role\":\""
    "output\"},{\"number\":39,\"name\":\"CA2\",\"role\":\"inout\"},{\"number\":40,\"name\":\"CA1\",\"ro"
    "le\":\"input\"}]},\"parts\":[{\"name\":\"W65C21N6TPG-14\",\"package\":\"DIP\",\"pinout\":\"p40\","
    "\"footprint\":\"PDIP-40-600\",\"grade\":\"14\"}]}";

static PartLibrary *inline_library(const char *device) {
    PartLibrary *library = part_library_create();
    char path[1024];
    char *footprints;
    char *text;
    char *error = NULL;
    size_t n;

    snprintf(path, sizeof path, "%s/footprints.json", PARTS_DIR);
    footprints = read_file(path);
    if (footprints == NULL || load_footprints(library, footprints, "footprints.json", &error) != 0) {
        fprintf(stderr, "footprints: %s\n", error != NULL ? error : "unreadable");
        exit(2);
    }
    free(footprints);
    n = strlen(device) + 128;
    text = malloc(n);
    snprintf(text, n, "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"pio\",\"devices\":[%s]}",
             device);
    if (load_parts(library, text, "inline.json", &error) != 0) {
        fprintf(stderr, "inline device: %s\n", error);
        exit(2);
    }
    free(text);
    return library;
}

static PartLibrary *pia_library;

static const Part *pia_part(void) {
    const Part *part;

    if (pia_library == NULL) {
        pia_library = inline_library(W65C21N_INLINE);
    }
    part = part_library_find(pia_library, "W65C21N6TPG-14");
    if (part == NULL) {
        fprintf(stderr, "no W65C21N6TPG-14\n");
        exit(2);
    }
    return part;
}

enum { PIA_ORA = 0, PIA_CRA = 1, PIA_ORB = 2, PIA_CRB = 3 };

static const char *const PA_NAMES[8] = {"PA0", "PA1", "PA2", "PA3", "PA4", "PA5", "PA6", "PA7"};
static const char *const PB_NAMES[8] = {"PB0", "PB1", "PB2", "PB3", "PB4", "PB5", "PB6", "PB7"};

/* A W65C21N on the bench, reset for 2 cycles, then RESB high; CA1/CB1 low. */
static Bench *pia_bench(void) {
    Bench *b = bench_create(pia_part());

    bench_set(b, "RESB", LOGIC_0);
    bench_set(b, "CA1", LOGIC_0);
    bench_set(b, "CB1", LOGIC_0);
    bench_bus65_begin(b);
    bench_bus65_idle(b, 2);
    bench_set(b, "RESB", LOGIC_1);
    bench_bus65_idle(b, 1);
    return b;
}

/* The byte on 8 pins; *x gets the bits that are not 0/1. */
static unsigned pins_value(Bench *b, const char *const *names, unsigned *x) {
    unsigned v = 0;
    int i;

    *x = 0;
    for (i = 0; i < 8; i++) {
        LogicValue p = bench_get(b, names[i]);

        if (p == LOGIC_1) v |= 1u << i;
        else if (p != LOGIC_0) *x |= 1u << i;
    }
    return v;
}

/* From a fall: an unselected cycle begins (data released, RWB 1, CS2B
   high at +30 ns); returns at +30 ns. */
static void idle_begin(Bench *b) {
    bench_run(b, 30 * NS);
    release_data(b);
    bench_set(b, "RWB", LOGIC_1);
    bench_set(b, "CS2B", LOGIC_1);
}

/* `since_fall` ns into an unselected cycle: PHI2 rises at +500 ns, falls at
   +1000 ns; returns at the fall. */
static void idle_end(Bench *b, SimTime since_fall) {
    bench_run(b, HALF - since_fall);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, HALF);
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 0);
}

/* From a fall: an unselected cycle up to its PHI2 rise, where it returns. */
static void idle_to_rise(Bench *b) {
    idle_begin(b);
    bench_run(b, 470 * NS);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, 0);
}

/* `elapsed` ns after a rise: the rest of the high phase, then PHI2 falls. */
static void finish_high(Bench *b, SimTime elapsed) {
    bench_run(b, HALF - elapsed);
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 0);
}

static void b1_reset(void) {
    Bench *b = pia_bench();
    int before = failures;
    uint8_t x;
    unsigned px;

    CHECK(bench_bus65_read(b, PIA_CRA, &x) == 0 && x == 0);
    CHECK(bench_bus65_read(b, PIA_CRB, &x) == 0 && x == 0);
    CHECK(bench_bus65_read(b, PIA_ORA, &x) == 0 && x == 0);   /* DDRA */
    CHECK(bench_bus65_read(b, PIA_ORB, &x) == 0 && x == 0);   /* DDRB */
    /* Port A inputs pulled up (weak: a driver wins), port B floats, IRQs Z. */
    CHECK(pins_value(b, PA_NAMES, &px) == 0xFF && px == 0);
    bench_set(b, "PA0", LOGIC_0);
    bench_run(b, NS);
    CHECK(bench_get(b, "PA0") == LOGIC_0);
    bench_set(b, "PA0", LOGIC_Z);
    bench_run(b, NS);
    CHECK(bench_get(b, "PB0") == LOGIC_Z && bench_get(b, "PB7") == LOGIC_Z);
    CHECK(bench_get(b, "IRQAB") == LOGIC_Z && bench_get(b, "IRQBB") == LOGIC_Z);
    /* ORA and ORB hold 0: make the ports outputs and read them. */
    bench_bus65_idle(b, 1);
    bench_bus65_write(b, PIA_ORA, 0xFF);
    bench_bus65_write(b, PIA_ORB, 0xFF);
    bench_bus65_write(b, PIA_CRA, 0x04);
    bench_bus65_write(b, PIA_CRB, 0x04);
    CHECK(bench_bus65_read(b, PIA_ORA, &x) == 0x00 && x == 0);
    CHECK(bench_bus65_read(b, PIA_ORB, &x) == 0x00 && x == 0);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s B1 reset\n", failures > before ? "FAIL" : "ok");
}

static void b2_select(void) {
    Bench *b = pia_bench();
    int before = failures;
    uint8_t x;
    int i;

    bench_bus65_write(b, PIA_ORA, 0x0F);                      /* DDRA */
    CHECK(bench_bus65_read(b, PIA_ORA, &x) == 0x0F && x == 0);
    bench_bus65_write(b, PIA_CRA, 0x04);
    /* ORA: the pins, low nibble driven 0 (ORA = 0), high nibble pulled up. */
    CHECK(bench_bus65_read(b, PIA_ORA, &x) == 0xF0 && x == 0);
    bench_bus65_write(b, PIA_ORB, 0x3C);                      /* DDRB */
    CHECK(bench_bus65_read(b, PIA_ORB, &x) == 0x3C && x == 0);
    bench_bus65_write(b, PIA_CRB, 0x04);
    bench_bus65_write(b, PIA_ORB, 0xFF);                      /* ORB */
    for (i = 0; i < 8; i++) bench_set(b, PB_NAMES[i], (0x3C >> i & 1) ? LOGIC_Z : LOGIC_0);
    CHECK(bench_bus65_read(b, PIA_ORB, &x) == 0x3C && x == 0);
    bench_bus65_write(b, PIA_CRB, 0x00);
    CHECK(bench_bus65_read(b, PIA_ORB, &x) == 0x3C && x == 0); /* DDRB kept */
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s B2 DDR/OR select\n", failures > before ? "FAIL" : "ok");
}

static void b3_port_a(void) {
    Bench *b = pia_bench();
    int before = failures;
    uint8_t x;
    unsigned px;

    bench_bus65_write(b, PIA_ORA, 0xFF);                      /* DDRA */
    bench_bus65_write(b, PIA_CRA, 0x04);
    bench_run(b, 30 * NS);
    CHECK(pins_value(b, PA_NAMES, &px) == 0x00 && px == 0);
    bench_bus65_write(b, PIA_ORA, 0xA5);
    bench_run(b, 20 * NS - 1);
    CHECK(pins_value(b, PA_NAMES, &px) == 0x00 && px == 0);
    bench_run(b, 1);                 /* fall + 20 ns */
    CHECK(pins_value(b, PA_NAMES, &px) == 0xA5 && px == 0);
    /* Port A reads its pins: a driver holding PA2 low wins over the pull-up. */
    bench_set(b, "PA2", LOGIC_0);
    CHECK(bench_bus65_read(b, PIA_ORA, &x) == (0xA5 & ~4) && x == 0);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s B3 port A out\n", failures > before ? "FAIL" : "ok");
}

static void b4_port_b(void) {
    Bench *b = pia_bench();
    int before = failures;
    uint8_t x;
    int i;

    bench_bus65_write(b, PIA_ORB, 0x0F);                      /* DDRB */
    bench_bus65_write(b, PIA_CRB, 0x04);
    bench_bus65_write(b, PIA_ORB, 0x5A);
    for (i = 4; i < 8; i++) bench_set(b, PB_NAMES[i], (0x9 >> (i - 4) & 1) ? LOGIC_1 : LOGIC_0);
    CHECK(bench_bus65_read(b, PIA_ORB, &x) == 0x9A && x == 0);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s B4 port B mixed\n", failures > before ? "FAIL" : "ok");
}

static void b5_ca1_irq(void) {
    Bench *b = pia_bench();
    int before = failures;
    uint8_t x;

    bench_bus65_write(b, PIA_CRA, 0x07);                      /* rising, enabled, ORA */
    idle_begin(b);
    bench_run(b, 70 * NS);
    bench_set(b, "CA1", LOGIC_1);
    bench_run(b, 20 * NS - 1);
    CHECK(bench_get(b, "IRQAB") == LOGIC_Z);
    bench_run(b, 1);                 /* the edge + 20 ns */
    CHECK(bench_get(b, "IRQAB") == LOGIC_0);
    idle_end(b, 120 * NS);
    CHECK(bench_bus65_read(b, PIA_CRA, &x) == 0x87 && x == 0);
    bench_bus65_read(b, PIA_ORA, NULL);
    bench_run(b, 25 * NS - 1);
    CHECK(bench_get(b, "IRQAB") == LOGIC_0);
    bench_run(b, 1);                 /* fall + 25 ns */
    CHECK(bench_get(b, "IRQAB") == LOGIC_Z);
    CHECK(bench_bus65_read(b, PIA_CRA, &x) == 0x07 && x == 0);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s B5 CA1 edge + IRQ\n", failures > before ? "FAIL" : "ok");
}

static void b6_narrow(void) {
    Bench *b = pia_bench();
    int before = failures;
    uint8_t x;

    bench_bus65_write(b, PIA_CRA, 0x06);                      /* rising, not enabled */
    idle_begin(b);
    bench_run(b, 70 * NS);
    bench_set(b, "CA1", LOGIC_1);
    bench_run(b, 50 * NS);
    bench_set(b, "CA1", LOGIC_0);
    idle_end(b, 150 * NS);
    CHECK(bench_bus65_read(b, PIA_CRA, &x) == 0x06 && x == 0x80);
    /* 100 ns wide: seen. */
    bench_bus65_read(b, PIA_ORA, NULL);                       /* clears the flags */
    idle_begin(b);
    bench_run(b, 70 * NS);
    bench_set(b, "CA1", LOGIC_1);
    bench_run(b, 100 * NS);
    bench_set(b, "CA1", LOGIC_0);
    idle_end(b, 200 * NS);
    CHECK(bench_bus65_read(b, PIA_CRA, &x) == 0x86 && x == 0);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s B6 CA1 narrow pulse\n", failures > before ? "FAIL" : "ok");
}

static void b7_handshake(void) {
    Bench *b = pia_bench();
    int before = failures;

    bench_set(b, "CA1", LOGIC_1);    /* an inactive (rising) edge for CRA bit 1 = 0 */
    bench_bus65_write(b, PIA_CRA, 0x24);                      /* handshake on read */
    bench_run(b, NS);
    CHECK(bench_get(b, "CA2") == LOGIC_1);
    bench_bus65_read(b, PIA_ORA, NULL);
    bench_run(b, 20 * NS - 1);
    CHECK(bench_get(b, "CA2") == LOGIC_1);
    bench_run(b, 1);                 /* fall + 20 ns */
    CHECK(bench_get(b, "CA2") == LOGIC_0);
    bench_run(b, 100 * NS);
    bench_set(b, "CA1", LOGIC_0);    /* the active (falling) edge */
    bench_run(b, 25 * NS - 1);
    CHECK(bench_get(b, "CA2") == LOGIC_0);
    bench_run(b, 1);
    CHECK(bench_get(b, "CA2") == LOGIC_1);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s B7 CA2 handshake\n", failures > before ? "FAIL" : "ok");
}

static void b8_cb2_pulse(void) {
    Bench *b = pia_bench();
    int before = failures;

    bench_bus65_write(b, PIA_CRB, 0x2C);                      /* pulse on write, ORB */
    bench_run(b, NS);
    CHECK(bench_get(b, "CB2") == LOGIC_1);
    bench_bus65_write(b, PIA_ORB, 0x12);
    CHECK(bench_get(b, "CB2") == LOGIC_1);
    idle_to_rise(b);
    bench_run(b, 70 * NS - 1);
    CHECK(bench_get(b, "CB2") == LOGIC_1);
    bench_run(b, 1);                 /* rise + 70 ns */
    CHECK(bench_get(b, "CB2") == LOGIC_0);
    finish_high(b, 70 * NS);
    idle_to_rise(b);
    bench_run(b, 20 * NS - 1);
    CHECK(bench_get(b, "CB2") == LOGIC_0);
    bench_run(b, 1);                 /* the following rise + 20 ns */
    CHECK(bench_get(b, "CB2") == LOGIC_1);
    finish_high(b, 20 * NS);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s B8 CB2 pulse\n", failures > before ? "FAIL" : "ok");
}

static void b9_manual(void) {
    Bench *b = pia_bench();
    int before = failures;

    bench_bus65_write(b, PIA_CRA, 0x34);
    bench_run(b, NS);
    CHECK(bench_get(b, "CA2") == LOGIC_0);
    bench_bus65_idle(b, 1);
    bench_bus65_write(b, PIA_CRA, 0x3C);
    bench_run(b, NS);
    CHECK(bench_get(b, "CA2") == LOGIC_1);
    bench_bus65_idle(b, 1);
    bench_bus65_write(b, PIA_CRB, 0x34);
    bench_run(b, NS);
    CHECK(bench_get(b, "CB2") == LOGIC_0);
    bench_bus65_idle(b, 1);
    bench_bus65_write(b, PIA_CRB, 0x3C);
    bench_run(b, NS);
    CHECK(bench_get(b, "CB2") == LOGIC_1);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s B9 manual outputs\n", failures > before ? "FAIL" : "ok");
}

static void b10_inputs(void) {
    Bench *b = pia_bench();
    int before = failures;
    uint8_t x;
    uint8_t value = 0;
    int i;

    CHECK(bench_get(b, "CA2") == LOGIC_1);                    /* input, pulled up */
    bench_bus65_write(b, PIA_CRA, 0x08);                      /* falling, enabled */
    idle_begin(b);
    bench_run(b, 70 * NS);
    bench_set(b, "CA2", LOGIC_0);
    bench_run(b, 20 * NS);
    CHECK(bench_get(b, "IRQAB") == LOGIC_0);
    bench_run(b, 100 * NS);
    bench_set(b, "CA2", LOGIC_Z);    /* released: the pull-up */
    idle_end(b, 220 * NS);
    CHECK(bench_bus65_read(b, PIA_CRA, &x) == 0x48 && x == 0);
    bench_bus65_write(b, PIA_CRA, 0x00);                      /* disabled: flag kept */
    bench_run(b, NS);
    CHECK(bench_get(b, "IRQAB") == LOGIC_Z);
    CHECK(bench_bus65_read(b, PIA_CRA, &x) == 0x40 && x == 0);
    /* Peripheral setup: PB0 changes 5 ns before the read's rise -> X. */
    bench_bus65_write(b, PIA_CRB, 0x04);                      /* DDRB = 0: inputs */
    for (i = 0; i < 8; i++) bench_set(b, PB_NAMES[i], LOGIC_0);
    bench_bus65_idle(b, 1);
    bench_run(b, 30 * NS);
    release_data(b);
    bench_set(b, "RWB", LOGIC_1);
    set_rs(b, PIA_ORB);
    bench_set(b, "CS2B", LOGIC_0);
    bench_run(b, 465 * NS);
    bench_set(b, "PB0", LOGIC_1);
    bench_run(b, 5 * NS);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, 400 * NS);
    x = 0;
    for (i = 0; i < 8; i++) {
        LogicValue v = bench_get(b, DATA_NAMES[i]);

        if (v == LOGIC_1) value |= (uint8_t)(1u << i);
        else if (v != LOGIC_0) x |= (uint8_t)(1u << i);
    }
    CHECK(x == 0x01 && value == 0x00);
    finish_high(b, 400 * NS);
    /* 15 ns before: read. */
    CHECK(bench_bus65_read(b, PIA_ORB, &x) == 0x01 && x == 0);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s B10 inputs as interrupts\n", failures > before ? "FAIL" : "ok");
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
    b1_reset();
    b2_select();
    b3_port_a();
    b4_port_b();
    b5_ca1_irq();
    b6_narrow();
    b7_handshake();
    b8_cb2_pulse();
    b9_manual();
    b10_inputs();
    if (pia_library != NULL) part_library_destroy(pia_library);
    if (failures > 0) {
        printf("%d failures\n", failures);
        return 1;
    }
    return 0;
}
