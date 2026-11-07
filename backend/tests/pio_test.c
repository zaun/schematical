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

/* ---- B: pia6521 (the shipped W65C21N) ------------------------------------------- */

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

/* The shipped part library (schematical/schematical/Parts). */
static PartLibrary *shipped_library;

static const Part *shipped_part(const char *name) {
    const Part *part;

    if (shipped_library == NULL) {
        shipped_library = part_library_create();
        if (load_parts_dir(shipped_library, PARTS_DIR) != 0) {
            exit(2);
        }
    }
    part = part_library_find(shipped_library, name);
    if (part == NULL) {
        fprintf(stderr, "no part %s\n", name);
        exit(2);
    }
    return part;
}

static const Part *pia_part(void) {
    return shipped_part("W65C21N6TPG-14");
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

/* ---- C: via6522 (inline W65C22N and W65C22S devices) ----------------------------- */

/* The W65C22N and W65C22S devices (p40 pinout, one part each). */
static const char W65C22_INLINE[] =
    "{\"id\":\"W65C22N\",\"function\":\"VIA\",\"description\":\"Versatile interface adapter, NMO"
    "S-compatible (WDC W65C22N)\",\"drawer\":[\"Interface\"],\"note\":\"W65C22 (datasheets/pi"
    "o/w65c22.pdf): register map Table 2-1 p.7, ports and handshake p.7-12, timers p."
    "13-19, shift register p.19-24, interrupts p.24-26, pins p.27-35, AC Tables 4-3/4"
    "-4 p.40-43, caveats p.48, ordering p.50. IRQB is open drain (p.24); no bus holdi"
    "ng.\",\"model\":\"via6522\",\"phi2\":\"PHI2\",\"rw\":\"RWB\",\"reset\":\"RESB\",\"data\":[\"D0\",\"D1\""
    ",\"D2\",\"D3\",\"D4\",\"D5\",\"D6\",\"D7\"],\"csHigh\":[\"CS1\"],\"csLow\":[\"CS2B\"],\"rs\":[\"RS0\",\"R"
    "S1\",\"RS2\",\"RS3\"],\"pa\":[\"PA0\",\"PA1\",\"PA2\",\"PA3\",\"PA4\",\"PA5\",\"PA6\",\"PA7\"],\"pb\":[\"P"
    "B0\",\"PB1\",\"PB2\",\"PB3\",\"PB4\",\"PB5\",\"PB6\",\"PB7\"],\"ca1\":\"CA1\",\"ca2\":\"CA2\",\"cb1\":\"CB"
    "1\",\"cb2\":\"CB2\",\"irq\":\"IRQB\",\"irqOpenDrain\":true,\"busHold\":false,\"grades\":{\"14\":{"
    "\"tcyc\":70,\"tpwh\":35,\"tpwl\":35,\"tacr\":10,\"tcar\":10,\"tcdr\":20,\"thr\":10,\"tpcr\":10,\""
    "tacw\":10,\"tcaw\":10,\"tdcw\":10,\"thw\":10,\"tcpw\":250,\"tca2\":75,\"trs1\":75,\"trs2\":100,"
    "\"twhs\":75,\"tds\":100,\"trs3\":150,\"trs4\":88,\"t21\":88,\"til\":62,\"tsr1\":35,\"tsr2\":105,"
    "\"tal\":88,\"tpdh\":10,\"vcc\":[4.75,5.25],\"note\":\"Tables 4-3a and 4-4a, 5.0 V +/- 5% "
    "(14 MHz). tRS1 is printed in us; it is ns. VDD Table 4-2a.\"}},\"pinouts\":{\"p40\":["
    "{\"number\":1,\"name\":\"VSS\",\"role\":\"ground\"},{\"number\":2,\"name\":\"PA0\",\"role\":\"inout"
    "\",\"group\":\"port A\"},{\"number\":3,\"name\":\"PA1\",\"role\":\"inout\",\"group\":\"port A\"},{\""
    "number\":4,\"name\":\"PA2\",\"role\":\"inout\",\"group\":\"port A\"},{\"number\":5,\"name\":\"PA3\""
    ",\"role\":\"inout\",\"group\":\"port A\"},{\"number\":6,\"name\":\"PA4\",\"role\":\"inout\",\"group"
    "\":\"port A\"},{\"number\":7,\"name\":\"PA5\",\"role\":\"inout\",\"group\":\"port A\"},{\"number\":"
    "8,\"name\":\"PA6\",\"role\":\"inout\",\"group\":\"port A\"},{\"number\":9,\"name\":\"PA7\",\"role\":"
    "\"inout\",\"group\":\"port A\"},{\"number\":10,\"name\":\"PB0\",\"role\":\"inout\",\"group\":\"port"
    " B\"},{\"number\":11,\"name\":\"PB1\",\"role\":\"inout\",\"group\":\"port B\"},{\"number\":12,\"na"
    "me\":\"PB2\",\"role\":\"inout\",\"group\":\"port B\"},{\"number\":13,\"name\":\"PB3\",\"role\":\"ino"
    "ut\",\"group\":\"port B\"},{\"number\":14,\"name\":\"PB4\",\"role\":\"inout\",\"group\":\"port B\"}"
    ",{\"number\":15,\"name\":\"PB5\",\"role\":\"inout\",\"group\":\"port B\"},{\"number\":16,\"name\":"
    "\"PB6\",\"role\":\"inout\",\"group\":\"port B\"},{\"number\":17,\"name\":\"PB7\",\"role\":\"inout\","
    "\"group\":\"port B\"},{\"number\":18,\"name\":\"CB1\",\"role\":\"inout\"},{\"number\":19,\"name\":"
    "\"CB2\",\"role\":\"inout\"},{\"number\":20,\"name\":\"VDD\",\"role\":\"power\"},{\"number\":21,\"na"
    "me\":\"IRQB\",\"role\":\"output\"},{\"number\":22,\"name\":\"RWB\",\"role\":\"input\"},{\"number\":"
    "23,\"name\":\"CS2B\",\"role\":\"input\"},{\"number\":24,\"name\":\"CS1\",\"role\":\"input\"},{\"num"
    "ber\":25,\"name\":\"PHI2\",\"role\":\"input\"},{\"number\":26,\"name\":\"D7\",\"role\":\"inout\",\"g"
    "roup\":\"data\"},{\"number\":27,\"name\":\"D6\",\"role\":\"inout\",\"group\":\"data\"},{\"number\":"
    "28,\"name\":\"D5\",\"role\":\"inout\",\"group\":\"data\"},{\"number\":29,\"name\":\"D4\",\"role\":\"i"
    "nout\",\"group\":\"data\"},{\"number\":30,\"name\":\"D3\",\"role\":\"inout\",\"group\":\"data\"},{\""
    "number\":31,\"name\":\"D2\",\"role\":\"inout\",\"group\":\"data\"},{\"number\":32,\"name\":\"D1\",\""
    "role\":\"inout\",\"group\":\"data\"},{\"number\":33,\"name\":\"D0\",\"role\":\"inout\",\"group\":\"d"
    "ata\"},{\"number\":34,\"name\":\"RESB\",\"role\":\"input\"},{\"number\":35,\"name\":\"RS3\",\"role"
    "\":\"input\"},{\"number\":36,\"name\":\"RS2\",\"role\":\"input\"},{\"number\":37,\"name\":\"RS1\",\""
    "role\":\"input\"},{\"number\":38,\"name\":\"RS0\",\"role\":\"input\"},{\"number\":39,\"name\":\"CA"
    "2\",\"role\":\"inout\"},{\"number\":40,\"name\":\"CA1\",\"role\":\"input\"}]},\"parts\":[{\"name\":"
    "\"W65C22N6TPG-14\",\"package\":\"DIP\",\"pinout\":\"p40\",\"footprint\":\"PDIP-40-600\",\"grade"
    "\":\"14\"}]},{\"id\":\"W65C22S\",\"function\":\"VIA\",\"description\":\"Versatile interface ad"
    "apter, CMOS (WDC W65C22S)\",\"drawer\":[\"Interface\"],\"note\":\"W65C22 (datasheets/pio"
    "/w65c22.pdf): register map Table 2-1 p.7, ports and handshake p.7-12, timers p.1"
    "3-19, shift register p.19-24, interrupts p.24-26, pins p.27-35, AC Tables 4-3/4-"
    "4 p.40-43, caveats p.48, ordering p.50. IRQB is a totem-pole output (p.24); ever"
    "y pin but PHI2 has a bus holding device (p.48).\",\"model\":\"via6522\",\"phi2\":\"PHI2\""
    ",\"rw\":\"RWB\",\"reset\":\"RESB\",\"data\":[\"D0\",\"D1\",\"D2\",\"D3\",\"D4\",\"D5\",\"D6\",\"D7\"],\"csH"
    "igh\":[\"CS1\"],\"csLow\":[\"CS2B\"],\"rs\":[\"RS0\",\"RS1\",\"RS2\",\"RS3\"],\"pa\":[\"PA0\",\"PA1\",\""
    "PA2\",\"PA3\",\"PA4\",\"PA5\",\"PA6\",\"PA7\"],\"pb\":[\"PB0\",\"PB1\",\"PB2\",\"PB3\",\"PB4\",\"PB5\",\"P"
    "B6\",\"PB7\"],\"ca1\":\"CA1\",\"ca2\":\"CA2\",\"cb1\":\"CB1\",\"cb2\":\"CB2\",\"irq\":\"IRQB\",\"irqOpen"
    "Drain\":false,\"busHold\":true,\"grades\":{\"14\":{\"tcyc\":70,\"tpwh\":35,\"tpwl\":35,\"tacr\""
    ":10,\"tcar\":10,\"tcdr\":20,\"thr\":10,\"tpcr\":10,\"tacw\":10,\"tcaw\":10,\"tdcw\":10,\"thw\":1"
    "0,\"tcpw\":30,\"tca2\":75,\"trs1\":75,\"trs2\":100,\"twhs\":75,\"tds\":100,\"trs3\":150,\"trs4\""
    ":88,\"t21\":88,\"til\":62,\"tsr1\":35,\"tsr2\":105,\"tal\":88,\"tpdh\":10,\"vcc\":[1.71,5.5],\""
    "note\":\"Tables 4-3b and 4-4b, the 5.0 V +/- 5% (14 MHz) column. tRS1 is printed i"
    "n us; it is ns. VDD 1.71-5.5 V across the voltage columns (Table 4-2b).\"}},\"pino"
    "uts\":{\"p40\":[{\"number\":1,\"name\":\"VSS\",\"role\":\"ground\"},{\"number\":2,\"name\":\"PA0\","
    "\"role\":\"inout\",\"group\":\"port A\"},{\"number\":3,\"name\":\"PA1\",\"role\":\"inout\",\"group\""
    ":\"port A\"},{\"number\":4,\"name\":\"PA2\",\"role\":\"inout\",\"group\":\"port A\"},{\"number\":5"
    ",\"name\":\"PA3\",\"role\":\"inout\",\"group\":\"port A\"},{\"number\":6,\"name\":\"PA4\",\"role\":\""
    "inout\",\"group\":\"port A\"},{\"number\":7,\"name\":\"PA5\",\"role\":\"inout\",\"group\":\"port A"
    "\"},{\"number\":8,\"name\":\"PA6\",\"role\":\"inout\",\"group\":\"port A\"},{\"number\":9,\"name\":"
    "\"PA7\",\"role\":\"inout\",\"group\":\"port A\"},{\"number\":10,\"name\":\"PB0\",\"role\":\"inout\","
    "\"group\":\"port B\"},{\"number\":11,\"name\":\"PB1\",\"role\":\"inout\",\"group\":\"port B\"},{\"n"
    "umber\":12,\"name\":\"PB2\",\"role\":\"inout\",\"group\":\"port B\"},{\"number\":13,\"name\":\"PB3"
    "\",\"role\":\"inout\",\"group\":\"port B\"},{\"number\":14,\"name\":\"PB4\",\"role\":\"inout\",\"gro"
    "up\":\"port B\"},{\"number\":15,\"name\":\"PB5\",\"role\":\"inout\",\"group\":\"port B\"},{\"numbe"
    "r\":16,\"name\":\"PB6\",\"role\":\"inout\",\"group\":\"port B\"},{\"number\":17,\"name\":\"PB7\",\"r"
    "ole\":\"inout\",\"group\":\"port B\"},{\"number\":18,\"name\":\"CB1\",\"role\":\"inout\"},{\"numbe"
    "r\":19,\"name\":\"CB2\",\"role\":\"inout\"},{\"number\":20,\"name\":\"VDD\",\"role\":\"power\"},{\"n"
    "umber\":21,\"name\":\"IRQB\",\"role\":\"output\"},{\"number\":22,\"name\":\"RWB\",\"role\":\"input"
    "\"},{\"number\":23,\"name\":\"CS2B\",\"role\":\"input\"},{\"number\":24,\"name\":\"CS1\",\"role\":\""
    "input\"},{\"number\":25,\"name\":\"PHI2\",\"role\":\"input\"},{\"number\":26,\"name\":\"D7\",\"rol"
    "e\":\"inout\",\"group\":\"data\"},{\"number\":27,\"name\":\"D6\",\"role\":\"inout\",\"group\":\"data"
    "\"},{\"number\":28,\"name\":\"D5\",\"role\":\"inout\",\"group\":\"data\"},{\"number\":29,\"name\":\""
    "D4\",\"role\":\"inout\",\"group\":\"data\"},{\"number\":30,\"name\":\"D3\",\"role\":\"inout\",\"grou"
    "p\":\"data\"},{\"number\":31,\"name\":\"D2\",\"role\":\"inout\",\"group\":\"data\"},{\"number\":32,"
    "\"name\":\"D1\",\"role\":\"inout\",\"group\":\"data\"},{\"number\":33,\"name\":\"D0\",\"role\":\"inou"
    "t\",\"group\":\"data\"},{\"number\":34,\"name\":\"RESB\",\"role\":\"input\"},{\"number\":35,\"name"
    "\":\"RS3\",\"role\":\"input\"},{\"number\":36,\"name\":\"RS2\",\"role\":\"input\"},{\"number\":37,\""
    "name\":\"RS1\",\"role\":\"input\"},{\"number\":38,\"name\":\"RS0\",\"role\":\"input\"},{\"number\":"
    "39,\"name\":\"CA2\",\"role\":\"inout\"},{\"number\":40,\"name\":\"CA1\",\"role\":\"input\"}]},\"par"
    "ts\":[{\"name\":\"W65C22S6TPG-14\",\"package\":\"DIP\",\"pinout\":\"p40\",\"footprint\":\"PDIP-4"
    "0-600\",\"grade\":\"14\"}]}";

static PartLibrary *via_library;

static const Part *via_part(int cmos) {
    const char *name = cmos ? "W65C22S6TPG-14" : "W65C22N6TPG-14";
    const Part *part;

    if (via_library == NULL) {
        via_library = inline_library(W65C22_INLINE);
    }
    part = part_library_find(via_library, name);
    if (part == NULL) {
        fprintf(stderr, "no part %s\n", name);
        exit(2);
    }
    return part;
}

enum {
    V_ORB, V_ORA, V_DDRB, V_DDRA, V_T1CL, V_T1CH, V_T1LL, V_T1LH, V_T2CL, V_T2CH, V_SR, V_ACR,
    V_PCR, V_IFR, V_IER, V_ORA_NH
};

/* A VIA on the bench, reset for 2 cycles, RESB high; CA1/CB1 low, CA2/CB2 high. */
static Bench *via_bench(int cmos) {
    Bench *b = bench_create(via_part(cmos));

    bench_set(b, "RESB", LOGIC_0);
    bench_set(b, "CA1", LOGIC_0);
    bench_set(b, "CB1", LOGIC_0);
    bench_set(b, "CA2", LOGIC_1);
    bench_set(b, "CB2", LOGIC_1);
    bench_bus65_begin(b);
    bench_bus65_idle(b, 2);
    bench_set(b, "RESB", LOGIC_1);
    bench_bus65_idle(b, 1);
    return b;
}

/* From a fall: a selected read of reg up to its PHI2 rise, where it returns. */
static void read_to_rise(Bench *b, unsigned reg) {
    bench_run(b, 30 * NS);
    release_data(b);
    bench_set(b, "RWB", LOGIC_1);
    set_rs(b, reg);
    bench_set(b, "CS2B", LOGIC_0);
    bench_run(b, 470 * NS);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, 0);
}

/* `elapsed` ns after the rise of a read: the byte at rise + 400 ns, then the fall. */
static uint8_t read_finish(Bench *b, SimTime elapsed, uint8_t *unknown) {
    uint8_t value = 0;
    uint8_t x = 0;
    int i;

    bench_run(b, 400 * NS - elapsed);
    for (i = 0; i < 8; i++) {
        LogicValue v = bench_get(b, DATA_NAMES[i]);

        if (v == LOGIC_1) value |= (uint8_t)(1u << i);
        else if (v != LOGIC_0) x |= (uint8_t)(1u << i);
    }
    finish_high(b, 400 * NS);
    if (unknown != NULL) *unknown = x;
    return value;
}

/* An unselected cycle; returns the pin's level 1 ns after the PHI2 rise. */
static LogicValue idle_sample(Bench *b, const char *pin) {
    LogicValue v;

    idle_to_rise(b);
    bench_run(b, NS);
    v = bench_get(b, pin);
    finish_high(b, NS);
    return v;
}

static void c1_reset(void) {
    Bench *b = via_bench(0);
    int before = failures;
    uint8_t x;

    bench_set(b, "CA2", LOGIC_Z);
    bench_bus65_write(b, V_T1LL, 0x34);
    bench_bus65_write(b, V_T1LH, 0x12);
    bench_bus65_write(b, V_DDRA, 0xFF);
    bench_bus65_write(b, V_DDRB, 0xFF);
    bench_bus65_write(b, V_ACR, 0x40);
    bench_bus65_write(b, V_PCR, 0x0C);                          /* CA2 low output */
    bench_bus65_write(b, V_IER, 0x82);
    bench_run(b, 50 * NS);
    CHECK(bench_get(b, "PA0") == LOGIC_0 && bench_get(b, "CA2") == LOGIC_0);
    bench_set(b, "RESB", LOGIC_0);
    bench_bus65_idle(b, 2);
    bench_set(b, "RESB", LOGIC_1);
    bench_bus65_idle(b, 1);
    CHECK(bench_bus65_read(b, V_DDRA, &x) == 0 && x == 0);
    CHECK(bench_bus65_read(b, V_DDRB, &x) == 0 && x == 0);
    CHECK(bench_bus65_read(b, V_ACR, &x) == 0 && x == 0);
    CHECK(bench_bus65_read(b, V_PCR, &x) == 0 && x == 0);
    CHECK(bench_bus65_read(b, V_IFR, &x) == 0 && x == 0);
    CHECK(bench_bus65_read(b, V_IER, &x) == 0x80 && x == 0);
    CHECK(bench_bus65_read(b, V_T1LL, &x) == 0x34 && x == 0);   /* latches kept */
    CHECK(bench_bus65_read(b, V_T1LH, &x) == 0x12 && x == 0);
    CHECK(bench_get(b, "PA0") == LOGIC_Z && bench_get(b, "PB0") == LOGIC_Z);
    CHECK(bench_get(b, "CA2") == LOGIC_Z);                      /* an input again */
    CHECK(bench_get(b, "IRQB") == LOGIC_Z);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s C1 reset\n", failures > before ? "FAIL" : "ok");
}

static void c2_ports(void) {
    Bench *b = via_bench(1);
    int before = failures;
    unsigned px;
    uint8_t x;
    int i;

    bench_bus65_write(b, V_DDRA, 0xFF);
    bench_bus65_write(b, V_ORA, 0x5A);
    bench_run(b, 30 * NS - 1);
    CHECK(pins_value(b, PA_NAMES, &px) == 0x00 && px == 0);
    bench_run(b, 1);                 /* fall + tcpw 30 ns */
    CHECK(pins_value(b, PA_NAMES, &px) == 0x5A && px == 0);
    bench_bus65_write(b, V_DDRB, 0x0F);
    bench_bus65_write(b, V_ORB, 0xA5);
    for (i = 4; i < 8; i++) bench_set(b, PB_NAMES[i], (0x6 >> (i - 4) & 1) ? LOGIC_1 : LOGIC_0);
    CHECK(bench_bus65_read(b, V_ORB, &x) == 0x65 && x == 0);
    CHECK(bench_bus65_read(b, V_ORA, &x) == 0x5A && x == 0);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s C2 ports\n", failures > before ? "FAIL" : "ok");
}

static void set_port(Bench *b, const char *const *names, unsigned value) {
    int i;

    for (i = 0; i < 8; i++) bench_set(b, names[i], (value >> i & 1) ? LOGIC_1 : LOGIC_0);
}

static void c3_latching(void) {
    Bench *b = via_bench(0);
    int before = failures;
    uint8_t x;

    bench_bus65_write(b, V_ACR, 0x03);
    bench_bus65_write(b, V_PCR, 0x11);                          /* CA1, CB1 rising */
    set_port(b, PA_NAMES, 0x3C);
    set_port(b, PB_NAMES, 0xC3);
    idle_begin(b);
    bench_run(b, 100 * NS);
    bench_set(b, "CA1", LOGIC_1);
    bench_set(b, "CB1", LOGIC_1);
    bench_run(b, 50 * NS);
    set_port(b, PA_NAMES, 0x00);
    set_port(b, PB_NAMES, 0x00);
    idle_end(b, 180 * NS);
    CHECK(bench_bus65_read(b, V_ORA, &x) == 0x3C && x == 0);    /* latched */
    CHECK(bench_bus65_read(b, V_ORA, &x) == 0x00 && x == 0);    /* read: transparent */
    CHECK(bench_bus65_read(b, V_ORB, &x) == 0xC3 && x == 0);
    /* PA0 changes 20 ns before the edge (til 62): latched X. */
    bench_set(b, "CA1", LOGIC_0);
    idle_begin(b);
    bench_run(b, 100 * NS);
    bench_set(b, "PA0", LOGIC_1);
    bench_run(b, 20 * NS);
    bench_set(b, "CA1", LOGIC_1);
    idle_end(b, 150 * NS);
    bench_bus65_read(b, V_ORA, &x);
    CHECK(x == 0x01);
    /* PA1 changes 5 ns after the edge (tpdh 10): latched X. */
    bench_set(b, "CA1", LOGIC_0);
    idle_begin(b);
    bench_run(b, 100 * NS);
    bench_set(b, "CA1", LOGIC_1);
    bench_run(b, 5 * NS);
    bench_set(b, "PA1", LOGIC_1);
    idle_end(b, 135 * NS);
    bench_bus65_read(b, V_ORA, &x);
    CHECK(x == 0x02);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s C3 latching\n", failures > before ? "FAIL" : "ok");
}

/* An active (falling) edge on a control input that idles high. */
static void pulse_low(Bench *b, const char *pin) {
    idle_begin(b);
    bench_run(b, 100 * NS);
    bench_set(b, pin, LOGIC_0);
    bench_run(b, 100 * NS);
    bench_set(b, pin, LOGIC_1);
    idle_end(b, 230 * NS);
}

static void c4_rs1_vs_rsf(void) {
    Bench *b = via_bench(0);
    int before = failures;
    uint8_t x;

    bench_set(b, "CA1", LOGIC_1);
    bench_bus65_idle(b, 1);
    pulse_low(b, "CA1");                                        /* PCR 0: falling edges */
    pulse_low(b, "CA2");
    CHECK(bench_bus65_read(b, V_IFR, &x) == 0x03 && x == 0);
    bench_bus65_read(b, V_ORA_NH, NULL);
    bench_bus65_write(b, V_ORA_NH, 0x00);
    CHECK(bench_bus65_read(b, V_IFR, &x) == 0x03 && x == 0);
    bench_bus65_read(b, V_ORA, NULL);
    CHECK(bench_bus65_read(b, V_IFR, &x) == 0x00 && x == 0);
    pulse_low(b, "CA1");
    pulse_low(b, "CA2");
    bench_bus65_write(b, V_ORA, 0x00);                           /* a write clears too */
    CHECK(bench_bus65_read(b, V_IFR, &x) == 0x00 && x == 0);
    /* CA2 handshake: RS F leaves CA2 alone, RS 1 strobes it. */
    bench_set(b, "CA2", LOGIC_Z);
    bench_bus65_write(b, V_PCR, 0x08);
    bench_bus65_read(b, V_ORA_NH, NULL);
    bench_run(b, 100 * NS);
    CHECK(bench_get(b, "CA2") == LOGIC_1);
    bench_bus65_read(b, V_ORA, NULL);
    bench_run(b, 100 * NS);
    CHECK(bench_get(b, "CA2") == LOGIC_0);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s C4 RS 1 vs RS F\n", failures > before ? "FAIL" : "ok");
}

static void c5_read_handshake(void) {
    Bench *b = via_bench(0);
    int before = failures;

    bench_set(b, "CA2", LOGIC_Z);
    bench_set(b, "CA1", LOGIC_1);
    bench_bus65_write(b, V_PCR, 0x08);                          /* CA2 handshake, CA1 falling */
    bench_run(b, NS);
    CHECK(bench_get(b, "CA2") == LOGIC_1);
    bench_bus65_read(b, V_ORA, NULL);
    bench_run(b, 75 * NS - 1);
    CHECK(bench_get(b, "CA2") == LOGIC_1);
    bench_run(b, 1);                 /* fall + tca2 75 ns */
    CHECK(bench_get(b, "CA2") == LOGIC_0);
    bench_run(b, 100 * NS);
    bench_set(b, "CA1", LOGIC_0);
    bench_run(b, 100 * NS - 1);
    CHECK(bench_get(b, "CA2") == LOGIC_0);
    bench_run(b, 1);                 /* edge + trs2 100 ns */
    CHECK(bench_get(b, "CA2") == LOGIC_1);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s C5 CA2 read handshake\n", failures > before ? "FAIL" : "ok");
}

static void c6_write_handshake(void) {
    Bench *b = via_bench(1);
    int before = failures;

    bench_set(b, "CB2", LOGIC_Z);
    bench_set(b, "CB1", LOGIC_1);
    bench_bus65_write(b, V_PCR, 0x80);                          /* CB2 handshake, CB1 falling */
    bench_run(b, NS);
    CHECK(bench_get(b, "CB2") == LOGIC_1);
    bench_bus65_write(b, V_ORB, 0x11);
    CHECK(bench_get(b, "CB2") == LOGIC_1);
    idle_to_rise(b);
    bench_run(b, 75 * NS - 1);
    CHECK(bench_get(b, "CB2") == LOGIC_1);
    bench_run(b, 1);                 /* the next rise + twhs 75 ns */
    CHECK(bench_get(b, "CB2") == LOGIC_0);
    finish_high(b, 75 * NS);
    idle_begin(b);
    bench_run(b, 100 * NS);
    bench_set(b, "CB1", LOGIC_0);
    bench_run(b, 88 * NS - 1);
    CHECK(bench_get(b, "CB2") == LOGIC_0);
    bench_run(b, 1);                 /* edge + trs4 88 ns */
    CHECK(bench_get(b, "CB2") == LOGIC_1);
    idle_end(b, 218 * NS);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s C6 CB2 write handshake\n", failures > before ? "FAIL" : "ok");
}

static void c7_pulses(void) {
    Bench *b = via_bench(0);
    int before = failures;

    bench_set(b, "CA2", LOGIC_Z);
    bench_set(b, "CB2", LOGIC_Z);
    bench_bus65_write(b, V_PCR, 0xAA);                          /* CA2 and CB2 pulse */
    bench_run(b, NS);
    CHECK(bench_get(b, "CA2") == LOGIC_1 && bench_get(b, "CB2") == LOGIC_1);
    /* CA2 read pulse: low at fall + tca2, high at the next fall + trs1. */
    bench_bus65_read(b, V_ORA, NULL);
    idle_begin(b);
    bench_run(b, 45 * NS);
    CHECK(bench_get(b, "CA2") == LOGIC_0);
    idle_end(b, 75 * NS);
    bench_run(b, 75 * NS - 1);
    CHECK(bench_get(b, "CA2") == LOGIC_0);
    bench_run(b, 1);
    CHECK(bench_get(b, "CA2") == LOGIC_1);
    idle_end(b, 75 * NS);
    /* CA2 and CB2 write pulse: low at the next rise + twhs, high at the rise
       after + trs3. */
    bench_bus65_write(b, V_ORB, 0x00);
    idle_to_rise(b);
    bench_run(b, 75 * NS);
    CHECK(bench_get(b, "CB2") == LOGIC_0);
    finish_high(b, 75 * NS);
    idle_to_rise(b);
    bench_run(b, 150 * NS - 1);
    CHECK(bench_get(b, "CB2") == LOGIC_0);
    bench_run(b, 1);
    CHECK(bench_get(b, "CB2") == LOGIC_1);
    finish_high(b, 150 * NS);
    bench_bus65_write(b, V_ORA, 0x00);
    idle_to_rise(b);
    bench_run(b, 75 * NS);
    CHECK(bench_get(b, "CA2") == LOGIC_0);
    finish_high(b, 75 * NS);
    idle_to_rise(b);
    bench_run(b, 150 * NS);
    CHECK(bench_get(b, "CA2") == LOGIC_1);
    finish_high(b, 150 * NS);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s C7 pulse outputs\n", failures > before ? "FAIL" : "ok");
}

static void c8_independent(void) {
    Bench *b = via_bench(1);
    int before = failures;
    uint8_t x;

    bench_set(b, "CA1", LOGIC_1);
    bench_bus65_write(b, V_PCR, 0x02);                          /* CA2 independent, falling */
    pulse_low(b, "CA2");
    pulse_low(b, "CA1");
    CHECK(bench_bus65_read(b, V_IFR, &x) == 0x03 && x == 0);
    bench_bus65_read(b, V_ORA, NULL);
    CHECK(bench_bus65_read(b, V_IFR, &x) == 0x01 && x == 0);    /* IFR1 cleared, IFR0 kept */
    bench_bus65_write(b, V_IFR, 0x01);
    CHECK(bench_bus65_read(b, V_IFR, &x) == 0x00 && x == 0);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s C8 independent input\n", failures > before ? "FAIL" : "ok");
}

/* A T1 or T2 start: the low byte, then the start write (returns at its fall). */
static void timer_start(Bench *b, int t2, unsigned count) {
    bench_bus65_write(b, t2 ? V_T2CL : V_T1CL, count & 0xFF);
    bench_bus65_write(b, t2 ? V_T2CH : V_T1CH, count >> 8);
}

static void c9_t1_one_shot(void) {
    Bench *b = via_bench(0);
    int before = failures;
    static const uint8_t WANT[7] = {5, 4, 3, 2, 1, 0, 0xFF};
    uint8_t x;
    int j;

    bench_bus65_write(b, V_IER, 0xC0);
    timer_start(b, 0, 5);
    for (j = 1; j <= 6; j++) {
        CHECK(bench_bus65_read(b, V_T1CL, &x) == WANT[j - 1] && x == 0);
        CHECK(bench_get(b, "IRQB") == LOGIC_Z);
    }
    /* Cycle 7: IFR6 at its rise, 6.5 us after the write's fall. */
    read_to_rise(b, V_T1CH);
    CHECK(bench_get(b, "IRQB") == LOGIC_0);
    CHECK(read_finish(b, 0, &x) == 0xFF && x == 0);
    CHECK(bench_bus65_read(b, V_T1CL, &x) == 0xFE && x == 0);   /* cycle 8; clears IFR6 */
    CHECK(bench_get(b, "IRQB") == LOGIC_Z);
    /* No second IFR6: past another wrap of the counter. */
    bench_bus65_idle(b, 65536 + 10);
    CHECK(bench_bus65_read(b, V_IFR, &x) == 0x00 && x == 0);
    CHECK(bench_get(b, "IRQB") == LOGIC_Z);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s C9 T1 one-shot\n", failures > before ? "FAIL" : "ok");
}

static void c10_t1_free_run(void) {
    Bench *b = via_bench(1);
    int before = failures;
    static LogicValue level[300];
    int toggles[8];
    int n = 0;
    int j;

    bench_bus65_write(b, V_ACR, 0xC0);
    timer_start(b, 0, 3);                                       /* N = L = 3 */
    bench_run(b, 30 * NS);
    CHECK(bench_get(b, "PB7") == LOGIC_0);                       /* low at the write */
    /* Cycles 1..: PB7 sampled after each rise. Cycle 7 writes the high
       latch (RS 7 = 1: L = 0x0103); the reload after it is the first to use
       it. */
    level[0] = LOGIC_0;
    for (j = 1; j < 290; j++) {
        if (j == 7) {
            bench_bus65_write(b, V_T1LH, 1);
            level[j] = level[j - 1];
            continue;
        }
        level[j] = idle_sample(b, "PB7");
    }
    for (j = 1; j < 290 && n < 8; j++) {
        if (level[j] != level[j - 1]) toggles[n++] = j;
    }
    /* Time-outs at the rises of cycles N + 2 = 5, then every L + 2: 10
       (L = 3), then 10 + 0x105 = 271 (L = 0x103). */
    CHECK(n == 3 && toggles[0] == 5 && toggles[1] == 10 && toggles[2] == 271);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s C10 T1 free-run PB7\n", failures > before ? "FAIL" : "ok");
}

static void c11_t2_one_shot(void) {
    Bench *b = via_bench(0);
    int before = failures;
    uint8_t x;
    int j;

    bench_bus65_write(b, V_IER, 0xA0);
    timer_start(b, 1, 4);
    for (j = 1; j <= 5; j++) {
        CHECK(bench_bus65_read(b, V_T2CH, &x) == 0 && x == 0);
        CHECK(bench_get(b, "IRQB") == LOGIC_Z);
    }
    read_to_rise(b, V_T2CH);                                     /* cycle 6 = N + 2 */
    CHECK(bench_get(b, "IRQB") == LOGIC_0);
    CHECK(read_finish(b, 0, &x) == 0xFF && x == 0);
    CHECK(bench_bus65_read(b, V_T2CL, &x) == 0xFE && x == 0);   /* rolls on; clears IFR5 */
    CHECK(bench_bus65_read(b, V_T2CL, &x) == 0xFD && x == 0);
    CHECK(bench_get(b, "IRQB") == LOGIC_Z);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s C11 T2 one-shot\n", failures > before ? "FAIL" : "ok");
}

/* A PB6 pulse: low for `low` cycles, then high for `high` cycles; PB6 moves
   100 ns into each cycle's low phase. */
static void pb6_pulse(Bench *b, int low, int high) {
    idle_begin(b);
    bench_run(b, 70 * NS);
    bench_set(b, "PB6", LOGIC_0);
    idle_end(b, 100 * NS);
    if (low > 1) bench_bus65_idle(b, (unsigned)(low - 1));
    idle_begin(b);
    bench_run(b, 70 * NS);
    bench_set(b, "PB6", LOGIC_1);
    idle_end(b, 100 * NS);
    if (high > 1) bench_bus65_idle(b, (unsigned)(high - 1));
}

static void c12_t2_pulse_count(void) {
    Bench *b = via_bench(1);
    int before = failures;
    uint8_t x;

    bench_set(b, "PB6", LOGIC_1);
    bench_bus65_write(b, V_ACR, 0x20);
    bench_bus65_write(b, V_IER, 0xA0);
    timer_start(b, 1, 3);
    bench_bus65_idle(b, 4);
    CHECK(bench_bus65_read(b, V_T2CL, &x) == 3 && x == 0);      /* no PHI2 counting */
    pb6_pulse(b, 2, 2);
    pb6_pulse(b, 2, 2);
    CHECK(bench_bus65_read(b, V_T2CL, &x) == 1 && x == 0);
    CHECK(bench_get(b, "IRQB") == LOGIC_1);
    pb6_pulse(b, 2, 2);
    CHECK(bench_get(b, "IRQB") == LOGIC_0);                      /* IFR5 after the third */
    CHECK(bench_bus65_read(b, V_IFR, &x) == 0xA0 && x == 0);
    CHECK(bench_bus65_read(b, V_T2CL, &x) == 0 && x == 0);       /* clears IFR5 */
    pb6_pulse(b, 2, 2);
    CHECK(bench_bus65_read(b, V_T2CL, &x) == 0xFF && x == 0);    /* keeps counting, */
    CHECK(bench_bus65_read(b, V_IFR, &x) == 0x00 && x == 0);     /* no second IFR5 */
    /* A 1-cycle-wide pulse: the count is unknown. */
    timer_start(b, 1, 3);
    bench_bus65_idle(b, 2);
    pb6_pulse(b, 1, 2);
    bench_bus65_read(b, V_T2CL, &x);
    CHECK(x == 0xFF);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s C12 T2 pulse count\n", failures > before ? "FAIL" : "ok");
}

static void c13_interrupts(int cmos) {
    Bench *b = via_bench(cmos);
    int before = failures;
    uint8_t x;

    bench_set(b, "CA1", LOGIC_1);
    bench_bus65_idle(b, 1);
    CHECK(bench_get(b, "IRQB") == (cmos ? LOGIC_1 : LOGIC_Z));
    bench_bus65_write(b, V_IER, 0x82);                          /* set bit 1 */
    CHECK(bench_bus65_read(b, V_IER, &x) == 0x82 && x == 0);
    bench_bus65_write(b, V_IER, 0x90);                          /* set bit 4 */
    bench_bus65_write(b, V_IER, 0x10);                          /* clear bit 4 */
    CHECK(bench_bus65_read(b, V_IER, &x) == 0x82 && x == 0);
    pulse_low(b, "CA1");
    CHECK(bench_get(b, "IRQB") == LOGIC_0);
    CHECK(bench_bus65_read(b, V_IFR, &x) == 0x82 && x == 0);
    bench_bus65_write(b, V_IER, 0x02);                          /* disable: released */
    CHECK(bench_get(b, "IRQB") == (cmos ? LOGIC_1 : LOGIC_Z));
    CHECK(bench_bus65_read(b, V_IFR, &x) == 0x02 && x == 0);    /* flag kept, bit 7 off */
    bench_bus65_write(b, V_IER, 0x82);
    CHECK(bench_get(b, "IRQB") == LOGIC_0);
    bench_bus65_write(b, V_IFR, 0x82);                          /* clear by IFR write */
    CHECK(bench_get(b, "IRQB") == (cmos ? LOGIC_1 : LOGIC_Z));
    CHECK(bench_bus65_read(b, V_IFR, &x) == 0x00 && x == 0);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s C13 IER/IFR/IRQB (%s)\n", failures > before ? "FAIL" : "ok", cmos ? "S" : "N");
}

static void c14_bus_hold(void) {
    int cmos;
    int before = failures;

    for (cmos = 0; cmos < 2; cmos++) {
        Bench *b = via_bench(cmos);
        uint8_t x;

        bench_set(b, "PA0", LOGIC_1);
        bench_bus65_idle(b, 1);
        bench_set(b, "PA0", LOGIC_Z);
        bench_bus65_idle(b, 1);
        bench_bus65_read(b, V_ORA_NH, &x);
        CHECK(cmos ? (x & 1) == 0 : (x & 1) == 1);
        if (cmos) CHECK(bench_bus65_read(b, V_ORA_NH, NULL) & 1);
        CHECK(bench_error(b) == NULL);
        bench_destroy(b);
    }
    printf("%s C14 bus hold\n", failures > before ? "FAIL" : "ok");
}

/* ---- D: the via6522 shift register ------------------------------------------------- */

/* One unselected PHI2 cycle; returns the pin just after its fall. */
static LogicValue cycle_then(Bench *b, const char *pin) {
    bench_bus65_idle(b, 1);
    return bench_get(b, pin);
}

static void d1_disabled(void) {
    Bench *b = via_bench(1);   /* bus hold: released CB1/CB2 keep their level */
    int before = failures;
    uint8_t x;
    int j;

    bench_set(b, "CB1", LOGIC_Z);
    bench_set(b, "CB2", LOGIC_Z);
    bench_bus65_write(b, V_SR, 0x5A);
    CHECK(bench_bus65_read(b, V_SR, &x) == 0x5A && x == 0);
    for (j = 0; j < 20; j++) {
        CHECK(cycle_then(b, "CB1") == LOGIC_Z && bench_get(b, "CB2") == LOGIC_Z);
    }
    CHECK(bench_bus65_read(b, V_IFR, &x) == 0x00 && x == 0);
    CHECK(bench_bus65_read(b, V_SR, &x) == 0x5A && x == 0);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s D1 SR disabled (000)\n", failures > before ? "FAIL" : "ok");
}

/* Shift-in with an internal clock: each time CB1 falls the bench puts the
   next bit (MSB first) on CB2. Returns the cycles (after the starting SR
   read's fall = cycle 0) at which CB1 changed, in *toggles. */
static int shift_in_internal(Bench *b, uint8_t pattern, int cycles, int *toggles, int cap) {
    LogicValue last;
    int n = 0;
    int bit = 7;
    int j;

    bench_bus65_read(b, V_SR, NULL);                            /* the access starts it */
    last = bench_get(b, "CB1");
    if (last == LOGIC_0) {
        if (n < cap) toggles[n++] = 0;
        bench_set(b, "CB2", (pattern >> bit-- & 1) ? LOGIC_1 : LOGIC_0);
    }
    for (j = 1; j <= cycles; j++) {
        LogicValue v = cycle_then(b, "CB1");

        if (v != last && n < cap) toggles[n++] = j;
        if (v == LOGIC_0 && last == LOGIC_1 && bit >= 0) {
            bench_set(b, "CB2", (pattern >> bit-- & 1) ? LOGIC_1 : LOGIC_0);
        }
        last = v;
    }
    return n;
}

static void d2_in_t2(void) {
    Bench *b = via_bench(0);
    int before = failures;
    int toggles[40];
    int n;
    int j;
    int even = 1;
    uint8_t x;

    bench_set(b, "CB1", LOGIC_Z);
    bench_bus65_write(b, V_T2CL, 2);                            /* N = 2: every 4 cycles */
    bench_bus65_write(b, V_ACR, 0x04);
    n = shift_in_internal(b, 0xA5, 70, toggles, 40);
    for (j = 0; j < n; j++) even &= toggles[j] == 4 * (j + 1);
    CHECK(n == 16 && even);                                     /* 8 pulses, then high */
    CHECK(bench_get(b, "CB1") == LOGIC_1);
    CHECK((bench_bus65_read(b, V_IFR, &x) & 0x04) && x == 0);
    CHECK(bench_bus65_read(b, V_SR, &x) == 0xA5 && x == 0);
    CHECK((bench_bus65_read(b, V_IFR, &x) & 0x04) == 0);        /* the SR read cleared it */
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s D2 SR in, T2 (001)\n", failures > before ? "FAIL" : "ok");
}

static void d3_in_phi2(void) {
    Bench *b = via_bench(1);
    int before = failures;
    int toggles[40];
    int n;
    int j;
    int ok = 1;
    uint8_t x;

    bench_set(b, "CB1", LOGIC_Z);
    bench_bus65_write(b, V_ACR, 0x08);
    n = shift_in_internal(b, 0x3C, 24, toggles, 40);
    for (j = 0; j < n; j++) ok &= toggles[j] == j;              /* low 1 cycle, high 1 */
    CHECK(n == 16 && ok);
    CHECK(bench_get(b, "CB1") == LOGIC_1);
    CHECK((bench_bus65_read(b, V_IFR, &x) & 0x04) && x == 0);
    CHECK(bench_bus65_read(b, V_SR, &x) == 0x3C && x == 0);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s D3 SR in, PHI2 (010)\n", failures > before ? "FAIL" : "ok");
}

static void d4_in_external(void) {
    Bench *b = via_bench(0);
    int before = failures;
    uint8_t x;
    int bit;

    bench_bus65_write(b, V_ACR, 0x0C);
    bench_bus65_read(b, V_SR, NULL);                            /* count from 0 */
    for (bit = 7; bit >= 0; bit--) {
        idle_begin(b);
        bench_run(b, 70 * NS);
        bench_set(b, "CB2", (0x81 >> bit & 1) ? LOGIC_1 : LOGIC_0);
        bench_run(b, 100 * NS);
        bench_set(b, "CB1", LOGIC_1);                           /* shifts at this cycle's fall */
        idle_end(b, 200 * NS);
        bench_set(b, "CB1", LOGIC_0);
        if (bit == 1) CHECK((bench_bus65_read(b, V_IFR, &x) & 0x04) == 0);
    }
    CHECK((bench_bus65_read(b, V_IFR, &x) & 0x04) && x == 0);   /* after 8 */
    CHECK(bench_bus65_read(b, V_SR, &x) == 0x81 && x == 0);
    /* A CB1 rise 5 ns before a PHI2 fall: shifted or not, the SR is X. */
    idle_to_rise(b);
    bench_run(b, 495 * NS);
    bench_set(b, "CB1", LOGIC_1);
    bench_run(b, 5 * NS);
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 0);
    bench_set(b, "CB1", LOGIC_0);
    bench_bus65_read(b, V_SR, &x);
    CHECK(x == 0xFF);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s D4 SR in, external (011)\n", failures > before ? "FAIL" : "ok");
}

/* Shift-out with an internal clock: CB2 read at each CB1 rise, MSB first,
   for the first bits_wanted rises; *rises_after counts the rises after. */
static unsigned shift_out_internal(Bench *b, int bits_wanted, int cycles, int *bits,
                                   int *rises_after) {
    LogicValue last = bench_get(b, "CB1");
    unsigned value = 0;
    int n = 0;
    int j;

    *rises_after = 0;
    for (j = 1; j <= cycles; j++) {
        LogicValue v = cycle_then(b, "CB1");

        if (v == LOGIC_1 && last == LOGIC_0) {
            if (n < bits_wanted) {
                value = value << 1 | (bench_get(b, "CB2") == LOGIC_1);
                n++;
            } else {
                (*rises_after)++;
            }
        }
        last = v;
    }
    *bits = n;
    return value;
}

static void d5_out_free(void) {
    Bench *b = via_bench(0);
    int before = failures;
    int bits;
    int after;
    unsigned v;
    uint8_t x;

    bench_set(b, "CB1", LOGIC_Z);
    bench_set(b, "CB2", LOGIC_Z);
    bench_bus65_write(b, V_T2CL, 1);                            /* toggles every 3 cycles */
    bench_bus65_write(b, V_ACR, 0x10);
    bench_bus65_write(b, V_SR, 0x96);
    v = shift_out_internal(b, 24, 24 * 6 + 12, &bits, &after);
    CHECK(bits == 24 && v == 0x969696u);
    CHECK(after == 2);                                          /* free running: keeps going */
    CHECK(!(bench_bus65_read(b, V_IFR, &x) & 0x04) && !(x & 0x04));  /* never IFR2 */
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s D5 SR out, free-running T2 (100)\n", failures > before ? "FAIL" : "ok");
}

static void d6_out_t2(void) {
    Bench *b = via_bench(1);
    int before = failures;
    LogicValue cb2;
    LogicValue last;
    unsigned value = 0;
    int n = 0;
    int j;
    int falls = 0;
    int timing = 1;
    uint8_t x;

    bench_set(b, "CB1", LOGIC_Z);
    bench_set(b, "CB2", LOGIC_Z);
    bench_bus65_write(b, V_T2CL, 2);
    bench_bus65_write(b, V_ACR, 0x14);
    bench_bus65_write(b, V_SR, 0xC3);
    last = bench_get(b, "CB1");
    for (j = 1; j <= 80; j++) {
        LogicValue v = cycle_then(b, "CB1");

        if (v == LOGIC_0 && last == LOGIC_1) {
            /* The bit leaves tsr1 = 35 ns after this fall. */
            int bit = 0xC3 >> (7 - falls) & 1;

            cb2 = bench_get(b, "CB2");
            bench_run(b, 34 * NS);
            if (falls > 0 && ((0xC3 >> (8 - falls)) & 1) != bit) {
                timing &= bench_get(b, "CB2") == cb2;           /* not yet */
            }
            bench_run(b, NS);
            timing &= bench_get(b, "CB2") == (bit ? LOGIC_1 : LOGIC_0);
            falls++;
        }
        if (v == LOGIC_1 && last == LOGIC_0) {
            value = value << 1 | (bench_get(b, "CB2") == LOGIC_1);
            n++;
        }
        last = v;
    }
    CHECK(n == 8 && value == 0xC3 && falls == 8 && timing);
    CHECK(bench_get(b, "CB1") == LOGIC_1 && bench_get(b, "CB2") == LOGIC_1); /* holds bit 0 */
    CHECK((bench_bus65_read(b, V_IFR, &x) & 0x04) && x == 0);
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s D6 SR out, T2 (101)\n", failures > before ? "FAIL" : "ok");
}

static void d7_out_phi2(void) {
    Bench *b = via_bench(0);
    int before = failures;
    int bits;
    int after;
    unsigned v;
    uint8_t x;

    bench_set(b, "CB1", LOGIC_Z);
    bench_set(b, "CB2", LOGIC_Z);
    bench_bus65_write(b, V_ACR, 0x18);
    bench_bus65_write(b, V_SR, 0x5A);
    v = shift_out_internal(b, 8, 30, &bits, &after);
    CHECK(bits == 8 && v == 0x5A && after == 0);
    CHECK((bench_bus65_read(b, V_IFR, &x) & 0x04) && !(x & 0x04));   /* IFR2 after 8 */
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s D7 SR out, PHI2 (110)\n", failures > before ? "FAIL" : "ok");
}

static void d8_out_external(void) {
    Bench *b = via_bench(1);
    int before = failures;
    unsigned value = 0;
    uint8_t x;
    int k;

    bench_set(b, "CB1", LOGIC_1);
    bench_set(b, "CB2", LOGIC_Z);
    bench_bus65_write(b, V_ACR, 0x1C);
    bench_bus65_write(b, V_SR, 0x0F);
    for (k = 0; k < 16; k++) {
        idle_begin(b);
        bench_run(b, 70 * NS);
        bench_set(b, "CB1", LOGIC_0);                           /* shifts at this cycle's fall */
        idle_end(b, 100 * NS);
        bench_run(b, 35 * NS);
        value = value << 1 | (bench_get(b, "CB2") == LOGIC_1);
        bench_set(b, "CB1", LOGIC_1);
        if (k == 7) {
            CHECK((bench_bus65_read(b, V_IFR, &x) & 0x04) && x == 0);
            bench_bus65_write(b, V_IFR, 0x04);
        }
    }
    CHECK(value == 0x0F0F);
    CHECK((bench_bus65_read(b, V_IFR, &x) & 0x04) && x == 0);   /* again after 8 more */
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
    printf("%s D8 SR out, external (111)\n", failures > before ? "FAIL" : "ok");
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
    c1_reset();
    c2_ports();
    c3_latching();
    c4_rs1_vs_rsf();
    c5_read_handshake();
    c6_write_handshake();
    c7_pulses();
    c8_independent();
    c9_t1_one_shot();
    c10_t1_free_run();
    c11_t2_one_shot();
    c12_t2_pulse_count();
    c13_interrupts(0);
    c13_interrupts(1);
    c14_bus_hold();
    d1_disabled();
    d2_in_t2();
    d3_in_phi2();
    d4_in_external();
    d5_out_free();
    d6_out_t2();
    d7_out_phi2();
    d8_out_external();
    if (via_library != NULL) part_library_destroy(via_library);
    if (shipped_library != NULL) part_library_destroy(shipped_library);
    if (failures > 0) {
        printf("%d failures\n", failures);
        return 1;
    }
    return 0;
}
