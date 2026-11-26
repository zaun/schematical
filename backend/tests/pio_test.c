/*
 * pio_test: split from pio_test.c by plan-68-H; the shared helpers are
 * support/pio_bench.c.
 */

#include "pio_bench.h"

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

static const Part *pia_part(void) {
    return shipped_part("W65C21N6TPG-14");
}

enum { PIA_ORA = 0, PIA_CRA = 1, PIA_ORB = 2, PIA_CRB = 3 };

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
    if (shipped_library != NULL) part_library_destroy(shipped_library);
    if (failures > 0) {
        printf("%d failures\n", failures);
        return 1;
    }
    return 0;
}
