/*
 * pio_via_test: split from pio_test.c by plan-68-H; the shared helpers are
 * support/pio_bench.c.
 */

#include "pio_bench.h"

/* ---- C: via6522 (the shipped W65C22N and W65C22S) ----------------------------- */

static const Part *via_part(int cmos) {
    return shipped_part(cmos ? "W65C22S6TPG-14" : "W65C22N6TPG-14");
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

/* The voltage check's open-drain answer for the shipped parts. */
static int pin_index(const Part *part, const char *name) {
    size_t i;

    for (i = 0; i < part->pin_count; i++) {
        if (strcmp(part->pins[i].name, name) == 0) return (int)i;
    }
    return -1;
}

static void open_drain_pins(void) {
    int before = failures;
    const Part *n = shipped_part("W65C22N6TPG-14");
    const Part *s = shipped_part("W65C22S6TPLG-14");
    const Part *pia = shipped_part("W65C21N6TPLG-14");

    CHECK(part_pin_open_drain(n, (size_t)pin_index(n, "IRQB")) == 1);
    CHECK(part_pin_open_drain(s, (size_t)pin_index(s, "IRQB")) == 0);
    CHECK(part_pin_open_drain(pia, (size_t)pin_index(pia, "IRQAB")) == 1);
    CHECK(part_pin_open_drain(pia, (size_t)pin_index(pia, "IRQBB")) == 1);
    CHECK(part_pin_open_drain(n, (size_t)pin_index(n, "PA0")) == 0);
    printf("%s open-drain IRQ pins\n", failures > before ? "FAIL" : "ok");
}

int main(void) {
    Part part = test_part();

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
    open_drain_pins();
    if (shipped_library != NULL) part_library_destroy(shipped_library);
    if (failures > 0) {
        printf("%d failures\n", failures);
        return 1;
    }
    return 0;
}
