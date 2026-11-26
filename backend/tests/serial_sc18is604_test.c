/*
 * serial_sc18is604_test: the SC18IS604 (I8, I9)
 * (split from serial_test.c by plan-68-G; the bench is support/serial_bench.c).
 */

#include "serial_bench.h"
#include "serial_bench_maxim.h"
#include "serial_bench_bridge.h"

/* ---- I8 / I9: the SC18IS604 -------------------------------------------------------------- */

/* The SC18IS604 with its own driver, PULLUP-3V3 parts on SCL, SDA and INT,
   the JF test target on the I2C bus (address 0x2A: A0 tied to SCL, A1 to
   SDA), and for I9 an SC16IS750 in I2C mode on the same bus. */
typedef struct {
    MB b;
    SimInstance *tgt;
    SimInstance *u2;
    SimInstance *drv2;
    SimInstance *clk2;
    const Part *p2;
    size_t pins2;
    SimTime half2;
} SpBench;

static void sp_pull(SpBench *t, SimNet *net, const char *name) {
    const Part *pu = part_library_find(library, "PULLUP-3V3");
    SimInstance *inst;

    if (pu == NULL) {
        fprintf(stderr, "no PULLUP-3V3\n");
        exit(2);
    }
    inst = sim_add_instance(t->b.sim, kind_model(pu), name, pu);
    sim_connect(t->b.sim, net, inst, 0);
}

static size_t sp_pin2(const SpBench *t, const char *name) {
    size_t i;

    for (i = 0; i < t->p2->pin_count; i++) {
        if (strcmp(t->p2->pins[i].name, name) == 0) {
            return i;
        }
    }
    fprintf(stderr, "no pin %s\n", name);
    exit(2);
}

static void sp_set2(SpBench *t, const char *name, LogicValue v) {
    sim_schedule_drive(t->b.sim, t->drv2, sp_pin2(t, name), v, DRIVE_STRONG, 0);
}

static LogicValue sp_get2(SpBench *t, const char *name) {
    return sim_read_pin(t->b.sim, t->u2, sp_pin2(t, name));
}

static SpBench *sp_open(int with_uart) {
    SpBench *t = calloc(1, sizeof *t);
    MB *b = &t->b;
    SimNet **nets;
    SimNet *scl;
    SimNet *sda;
    size_t i;

    b->part = part_library_find(library, "SFX-SC18IS604");
    if (b->part == NULL) {
        fprintf(stderr, "no SFX-SC18IS604\n");
        exit(2);
    }
    b->pins = b->part->pin_count;
    b->sim = sim_create();
    b->chip = sim_add_instance(b->sim, kind_model(b->part), "U1", b->part);
    b->drv = sim_add_instance(b->sim, &DRIVER, "TB", &b->pins);
    nets = calloc(b->pins, sizeof *nets);
    for (i = 0; i < b->pins; i++) {
        nets[i] = sim_add_net(b->sim, b->part->pins[i].name);
        sim_connect(b->sim, nets[i], b->chip, i);
        sim_connect(b->sim, nets[i], b->drv, i);
    }
    scl = nets[mb_pin(b, "SCL")];
    sda = nets[mb_pin(b, "SDA")];
    sp_pull(t, scl, "PU1");
    sp_pull(t, sda, "PU2");
    sp_pull(t, nets[mb_pin(b, "INT")], "PU3");
    t->tgt = sim_add_instance(b->sim, &TARGET, "T1", NULL);
    for (i = 0; i < TG_SCL; i++) {
        sim_connect(b->sim, sim_add_net(b->sim, "tg"), t->tgt, i);
    }
    sim_connect(b->sim, scl, t->tgt, TG_SCL);
    sim_connect(b->sim, sda, t->tgt, TG_SDA);
    sim_connect(b->sim, scl, t->tgt, TG_A0);
    sim_connect(b->sim, sda, t->tgt, TG_A1);
    if (with_uart) {
        t->p2 = part_library_find(library, "SFX-SC16IS750");
        t->pins2 = t->p2->pin_count;
        t->half2 = XT_HALF;
        t->u2 = sim_add_instance(b->sim, kind_model(t->p2), "U2", t->p2);
        t->drv2 = sim_add_instance(b->sim, &DRIVER, "TB2", &t->pins2);
        t->clk2 = sim_add_instance(b->sim, &CLOCK, "CLK2", &t->half2);
        for (i = 0; i < t->pins2; i++) {
            const char *name = t->p2->pins[i].name;
            SimNet *net;

            if (strcmp(name, "SCL/SCLK") == 0) {
                sim_connect(b->sim, scl, t->u2, i);
                continue;
            }
            if (strcmp(name, "SDA") == 0) {
                sim_connect(b->sim, sda, t->u2, i);
                continue;
            }
            net = sim_add_net(b->sim, name);
            sim_connect(b->sim, net, t->u2, i);
            if (strcmp(name, "XTAL1") == 0) {
                sim_connect(b->sim, net, t->clk2, 0);
            } else {
                sim_connect(b->sim, net, t->drv2, i);
            }
        }
    }
    free(nets);
    b->cs = "CS";
    b->sclk = "SCLK";
    b->mosi = "MOSI";
    b->miso = "MISO";
    b->scl = "SCL";
    b->sda = "SDA";
    if (!sim_start(b->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(b->sim));
        exit(2);
    }
    mb_set(b, "CS", LOGIC_1);
    mb_set(b, "SCLK", LOGIC_1);   /* mode 3: SCLK idles high */
    mb_set(b, "MOSI", LOGIC_0);
    mb_set(b, "RESET", LOGIC_1);
    if (with_uart) {
        sp_set2(t, "I2C/SPI", LOGIC_1);
        sp_set2(t, "CS/A0", LOGIC_1);
        sp_set2(t, "SI/A1", LOGIC_1);
        sp_set2(t, "RX", LOGIC_1);
        sp_set2(t, "CTS", LOGIC_1);
        sp_set2(t, "RESET", LOGIC_0);
        mb_run(b, NS(4000));
        sp_set2(t, "RESET", LOGIC_1);
    }
    mb_run(b, NS(4000));
    return t;
}

static void sp_close(SpBench *t) {
    sim_destroy(t->b.sim);
    free(t);
}

static Tgt *sp_tgt(SpBench *t) {
    return sim_instance_state(t->tgt);
}

#define SP_HALF NS(500)

/* One SPI message (mode 3, MSB first on the wire): n bytes out, MISO in. */
static void sp_msg(SpBench *t, const uint8_t *out, unsigned n, uint8_t *in, char *text) {
    MB *b = &t->b;
    unsigned i;
    unsigned bit;

    mb_set(b, "CS", LOGIC_0);
    mb_run(b, SP_HALF);
    for (i = 0; i < n; i++) {
        uint8_t v = 0;

        for (bit = 0; bit < 8; bit++) {
            LogicValue l;

            mb_set(b, "SCLK", LOGIC_0);
            mb_set(b, "MOSI", (out[i] >> (7 - bit)) & 1u ? LOGIC_1 : LOGIC_0);
            mb_run(b, SP_HALF);
            l = mb_get(b, "MISO");
            if (text != NULL) {
                text[i * 8 + bit] = "01XZ"[l];
            }
            v = (uint8_t)((v << 1) | (l == LOGIC_1 ? 1u : 0u));
            mb_set(b, "SCLK", LOGIC_1);
            mb_run(b, SP_HALF);
        }
        if (in != NULL) {
            in[i] = v;
        }
    }
    if (text != NULL) {
        text[n * 8] = '\0';
    }
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, SP_HALF);
}

static void sp_reg_w(SpBench *t, unsigned reg, unsigned v) {
    uint8_t m[3] = {0x20, (uint8_t)reg, (uint8_t)v};

    sp_msg(t, m, 3, NULL, NULL);
}

static unsigned sp_reg_r(SpBench *t, unsigned reg) {
    uint8_t m[3] = {0x21, (uint8_t)reg, 0};
    uint8_t in[3];

    sp_msg(t, m, 3, in, NULL);
    return in[2];
}

/* Waits for INT low (the transaction's end); the time waited or -1. */
static long long sp_wait_int(SpBench *t, SimTime limit) {
    return mb_wait(&t->b, "INT", LOGIC_0, limit, NS(500));
}

/* Runs a message and waits for its INT; returns I2CStat (INT released). */
static unsigned sp_do(SpBench *t, const uint8_t *m, unsigned n, SimTime limit) {
    sp_msg(t, m, n, NULL, NULL);
    if (sp_wait_int(t, limit) < 0) {
        return 0x100;
    }
    return sp_reg_r(t, 0x04);
}

/* SCL fall-to-fall spacing in the middle of a transaction. */
static long long sp_scl_period(SpBench *t, SimTime limit) {
    MB *b = &t->b;
    SimTime step = NS(50);
    SimTime spent = 0;
    SimTime falls[6];
    unsigned n = 0;
    LogicValue last = mb_get(b, "SCL");

    while (spent < limit && n < 6) {
        LogicValue now;

        mb_run(b, step);
        spent += step;
        now = mb_get(b, "SCL");
        if (last == LOGIC_1 && now == LOGIC_0) {
            falls[n++] = spent;
        }
        last = now;
    }
    return n == 6 ? (long long)(falls[5] - falls[3]) / 2 : -1;
}

static void case_i8(void) {
    SpBench *t = sp_open(0);
    MB *b = &t->b;
    uint8_t m[24];
    uint8_t in[24];
    char text[200];
    unsigned st;
    long long p;
    long long w;

    case_begin();
    mb_expect_pin(b, "MISO", LOGIC_Z, "MISO Z while CS is high");
    mb_expect_u(sp_reg_r(t, 0x00), 0x00, "IOConfig reset");
    mb_expect_u(sp_reg_r(t, 0x02), 0x19, "I2CClock reset 0x19");
    mb_expect_u(sp_reg_r(t, 0x03), 0x00, "I2CTO reset");
    mb_expect_u(sp_reg_r(t, 0x04), 0xF0, "I2CStat reset 0xF0");
    mb_expect_u(sp_reg_r(t, 0x05), 0x00, "I2CAdr reset");
    mb_expect_pin(b, "INT", LOGIC_1, "INT released after reset");
    /* 0x00 write N bytes: START, address, data, STOP; 0xF3 while busy. */
    m[0] = 0x00; m[1] = 2; m[2] = 0x54; m[3] = 0x41; m[4] = 0x42;
    sp_msg(t, m, 5, NULL, NULL);
    mb_run(b, NS(60000));
    mb_expect_u(sp_reg_r(t, 0x04), 0xF3, "I2CStat 0xF3 while the transfer runs");
    mb_expect_pin(b, "INT", LOGIC_1, "no INT while busy");
    p = sp_scl_period(t, NS(200000));
    expect_near(p, (long long)NS(13333), NS(120), "I2CClock 0x19: SCL 15 MHz / 200 = 75 kHz");
    w = sp_wait_int(t, NS(1000000));
    check(w >= 0, "INT at the end of the write");
    mb_expect_u(sp_reg_r(t, 0x04), 0xF0, "I2CStat 0xF0: done");
    mb_run(b, NS(1000));
    mb_expect_pin(b, "INT", LOGIC_1, "reading I2CStat releases INT");
    check(strcmp(sp_tgt(t)->log, "S A54 W41 W42 P ") == 0, "the target saw START, 0x54, 0x41, 0x42, STOP");
    /* 0x01 read N bytes into the buffer; 0x06 reads it out. */
    sp_tgt(t)->log[0] = '\0';
    m[0] = 0x01; m[1] = 2; m[2] = 0x55;
    st = sp_do(t, m, 3, NS(1000000));
    mb_expect_u(st, 0xF0, "read 2: I2CStat 0xF0");
    check(strcmp(sp_tgt(t)->log, "S A55 N0 KA N1 KN P ") == 0, "read: ACK the first byte, NACK the last");
    memset(m, 0, sizeof m);
    m[0] = 0x06;
    sp_msg(t, m, 3, in, text);
    check(in[1] == 0x42 && in[2] == 0x43, "0x06 returns the buffer: 0x42, 0x43");
    check(strncmp(text, "ZZZZZZZZ", 8) == 0, "MISO Z during the command byte");
    /* 0x02 read after write and 0x03 write after write: a repeated START between. */
    sp_tgt(t)->log[0] = '\0';
    m[0] = 0x02; m[1] = 1; m[2] = 1; m[3] = 0x54; m[4] = 0x10; m[5] = 0x55;
    mb_expect_u(sp_do(t, m, 6, NS(1000000)), 0xF0, "read after write: 0xF0");
    check(strcmp(sp_tgt(t)->log, "S A54 W10 Sr A55 N0 KN P ") == 0, "read after write: S W, Sr R, P");
    m[0] = 0x06; m[1] = 0;
    sp_msg(t, m, 2, in, NULL);
    mb_expect_u(in[1], 0x10, "the byte read back");
    sp_tgt(t)->log[0] = '\0';
    m[0] = 0x03; m[1] = 1; m[2] = 1; m[3] = 0x54; m[4] = 0x20; m[5] = 0x54; m[6] = 0x21;
    mb_expect_u(sp_do(t, m, 7, NS(1000000)), 0xF0, "write after write: 0xF0");
    check(strcmp(sp_tgt(t)->log, "S A54 W20 Sr A54 W21 P ") == 0, "write after write: S W, Sr W, P");
    /* Address NACK: 0xF1. */
    sp_tgt(t)->log[0] = '\0';
    m[0] = 0x00; m[1] = 1; m[2] = 0x56; m[3] = 0x00;
    mb_expect_u(sp_do(t, m, 4, NS(1000000)), 0xF1, "no target at 0x2B: I2CStat 0xF1");
    /* A byte count of 0 or more than the bytes sent: 0xF9. */
    m[0] = 0x00; m[1] = 0; m[2] = 0x54;
    mb_expect_u(sp_do(t, m, 3, NS(100000)), 0xF9, "N = 0: I2CStat 0xF9");
    m[0] = 0x00; m[1] = 3; m[2] = 0x54; m[3] = 0x01;
    mb_expect_u(sp_do(t, m, 4, NS(100000)), 0xF9, "N = 3 with one byte sent: 0xF9");
    /* I2CClock 7: 15 MHz / 56 = 268 kHz. */
    sp_reg_w(t, 0x02, 7);
    mb_expect_u(sp_reg_r(t, 0x02), 7, "I2CClock written");
    m[0] = 0x00; m[1] = 2; m[2] = 0x54; m[3] = 0x41; m[4] = 0x42;
    sp_msg(t, m, 5, NULL, NULL);
    mb_run(b, NS(45000));
    p = sp_scl_period(t, NS(100000));
    expect_near(p, (long long)NS(3733), NS(120), "I2CClock 7: SCL period 8 x 7 / 15 MHz");
    sp_wait_int(t, NS(1000000));
    sp_reg_r(t, 0x04);
    /* Arbitration: another controller holds SDA low; the SC18IS604 loses at
       its first 1 bit, waits for the bus (STOP) and starts again. */
    sp_tgt(t)->log[0] = '\0';
    m[0] = 0x00; m[1] = 1; m[2] = 0x54; m[3] = 0x41;
    sp_msg(t, m, 4, NULL, NULL);
    mb_wait(b, "SDA", LOGIC_0, NS(100000), NS(20));
    mb_set(b, "SDA", LOGIC_0);
    mb_run(b, NS(20000));
    mb_expect_pin(b, "SCL", LOGIC_1, "after losing arbitration SCL is released");
    mb_set(b, "SDA", LOGIC_Z);   /* SDA rises with SCL high: STOP */
    mb_expect_u(sp_wait_int(t, NS(1000000)) >= 0, 1, "the transfer is retried and ends");
    mb_expect_u(sp_reg_r(t, 0x04), 0xF0, "after the retry: 0xF0");
    check(strstr(sp_tgt(t)->log, "S A54 W41 P ") != NULL, "the retried transfer reached the target");
    /* Timeout (I2CTO): SCL held low; ((7 + 1) / 15 MHz) x 512 = 273 us. */
    sp_reg_w(t, 0x03, 0x01);
    mb_set(b, "SCL", LOGIC_0);
    m[0] = 0x00; m[1] = 1; m[2] = 0x54; m[3] = 0x41;
    sp_msg(t, m, 4, NULL, NULL);
    mb_run(b, NS(250000));
    mb_expect_pin(b, "INT", LOGIC_1, "no timeout at 250 us");
    w = sp_wait_int(t, NS(100000));
    check(w >= 0, "INT by 350 us");
    mb_expect_u(sp_reg_r(t, 0x04), 0xF8, "I2CStat 0xF8: timeout");
    mb_set(b, "SCL", LOGIC_Z);
    sp_reg_w(t, 0x03, 0x00);
    /* 0x18: LSB first (0x81), MSB first again (0x42). */
    m[0] = 0x18; m[1] = 0x81;
    sp_msg(t, m, 2, NULL, NULL);
    m[0] = 0x84; m[1] = 0x40; m[2] = 0x00;   /* 0x21, 0x02 sent LSB first */
    sp_msg(t, m, 3, in, NULL);
    mb_expect_u(in[2], 0xE0, "LSB first: I2CClock 0x07 comes out as 0xE0");
    m[0] = 0x18; m[1] = 0x42;
    sp_msg(t, m, 2, NULL, NULL);
    mb_expect_u(sp_reg_r(t, 0x02), 7, "MSB first again");
    /* 0xFE: the version string into the buffer, INT. */
    m[0] = 0xFE;
    sp_msg(t, m, 1, NULL, NULL);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "INT", LOGIC_0, "INT after 0xFE");
    memset(m, 0, sizeof m);
    m[0] = 0x06;
    sp_msg(t, m, 17, in, NULL);
    check(memcmp(in + 1, "SC18IS604 1.0.0", 16) == 0, "version string \"SC18IS604 1.0.0\\0\"");
    sp_reg_r(t, 0x04);
    /* GPIO: push-pull, open drain, GPIO4 input. */
    sp_reg_w(t, 0x00, 0x02);
    sp_reg_w(t, 0x01, 0x01);
    mb_run(b, NS(100));
    mb_expect_pin(b, "GPIO0", LOGIC_1, "GPIO0 push-pull high");
    sp_reg_w(t, 0x00, 0x03);
    mb_run(b, NS(100));
    mb_expect_pin(b, "GPIO0", LOGIC_Z, "GPIO0 open drain, latch 1: released");
    sp_reg_w(t, 0x01, 0x00);
    mb_run(b, NS(100));
    mb_expect_pin(b, "GPIO0", LOGIC_0, "GPIO0 open drain, latch 0: low");
    mb_set(b, "GPIO4", LOGIC_1);
    mb_set(b, "GPIO1", LOGIC_0);
    mb_set(b, "GPIO2", LOGIC_1);
    mb_set(b, "GPIO3", LOGIC_0);
    mb_expect_u(sp_reg_r(t, 0x01), 0x14, "IOState reads the pins (GPIO4 and GPIO2 high)");
    /* Deep power-down: the waking frame is ignored. */
    m[0] = 0x30; m[1] = 0x5A; m[2] = 0xA5;
    sp_msg(t, m, 3, NULL, NULL);
    sp_reg_w(t, 0x05, 0x44);
    mb_expect_u(sp_reg_r(t, 0x05), 0x00, "the frame that wakes it is ignored");
    sp_reg_w(t, 0x05, 0x44);
    mb_expect_u(sp_reg_r(t, 0x05), 0x44, "awake: I2CAdr written");
    /* RESET: a 50 ns pulse is filtered, a 200 ns one resets. */
    mb_set(b, "RESET", LOGIC_0);
    mb_run(b, NS(50));
    mb_set(b, "RESET", LOGIC_1);
    mb_run(b, NS(1000));
    mb_expect_u(sp_reg_r(t, 0x05), 0x44, "a 50 ns RESET pulse is rejected");
    mb_set(b, "RESET", LOGIC_0);
    mb_run(b, NS(200));
    mb_set(b, "RESET", LOGIC_1);
    mb_run(b, NS(1000));
    mb_expect_u(sp_reg_r(t, 0x05), 0x00, "a 200 ns RESET pulse resets");
    mb_expect_u(sp_reg_r(t, 0x02), 0x19, "I2CClock back to 0x19");
    sp_close(t);
    case_end("I8 SC18IS604: commands 0x00-0x03, 0x06, 0x18, 0x20/0x21, 0x30, 0xFE; START, STOP and "
             "repeated START at the target; ACK/NACK and 0xF0-0xF9 status codes; I2CClock rates; "
             "arbitration; timeout; GPIO; RESET filter");
}

/* I9: SPI -> SC18IS604 -> I2C (PULLUP-3V3 on SDA and SCL) -> SC16IS750 -> TX. */
static void case_i9(void) {
    SpBench *t = sp_open(1);
    MB *b = &t->b;
    uint8_t m[8];
    uint8_t in[4];
    char frame[16];
    uint16_t frames[4];
    size_t n;
    unsigned i;
    static const uint8_t SETUP[4][2] = {{0x18, 0x80}, {0x00, 0x10}, {0x08, 0x00}, {0x18, 0x03}};
    int ok = 1;

    case_begin();
    sp_reg_w(t, 0x02, 7);
    for (i = 0; i < 4; i++) {
        m[0] = 0x00; m[1] = 2; m[2] = 0x90; m[3] = SETUP[i][0]; m[4] = SETUP[i][1];
        ok &= sp_do(t, m, 5, NS(1000000)) == 0xF0;
    }
    check(ok, "LCR, DLL, DLH, LCR written over I2C: 0xF0 each");
    m[0] = 0x02; m[1] = 1; m[2] = 1; m[3] = 0x90; m[4] = 0x18; m[5] = 0x91;
    mb_expect_u(sp_do(t, m, 6, NS(1000000)), 0xF0, "read after write of LCR: 0xF0");
    m[0] = 0x06; m[1] = 0;
    sp_msg(t, m, 2, in, NULL);
    mb_expect_u(in[1], 0x03, "the SC16IS750's LCR read through the bridge");
    m[0] = 0x00; m[1] = 2; m[2] = 0x90; m[3] = 0x00; m[4] = 0x55;
    sp_msg(t, m, 5, NULL, NULL);
    /* The THR write lands at the I2C data byte; its frame follows. */
    {
        SimTime spent = 0;

        while (spent < NS(400000) && sp_get2(t, "TX") != LOGIC_0) {
            mb_run(b, NS(50));
            spent += NS(50);
        }
        mb_run(b, 16 * BIT1 / 2);
        frame[0] = "01XZ"[sp_get2(t, "TX")];
        for (i = 1; i <= 9; i++) {
            mb_run(b, 16 * BIT1);
            frame[i] = "01XZ"[sp_get2(t, "TX")];
        }
        frame[10] = '\0';
    }
    check(strcmp(frame, "0101010101") == 0, "the SC16IS750 sends 0x55 on TX");
    sp_wait_int(t, NS(1000000));
    mb_expect_u(sp_reg_r(t, 0x04), 0xF0, "the THR write: 0xF0");
    n = sim_instance_type(t->u2)->serial_sent(sim_instance_state(t->u2), 0, 0, frames, 4);
    check(n == 1 && frames[0] == 0x55, "the SC16IS750's terminal log: 0x55");
    /* The SC16IS750 NACKs the byte that sets IOControl[3]: 0xF2. */
    m[0] = 0x00; m[1] = 2; m[2] = 0x90; m[3] = 0x70; m[4] = 0x08;
    mb_expect_u(sp_do(t, m, 5, NS(1000000)), 0xF2, "SRESET byte NACKed by the target: 0xF2");
    m[0] = 0x02; m[1] = 1; m[2] = 1; m[3] = 0x90; m[4] = 0x18; m[5] = 0x91;
    sp_do(t, m, 6, NS(1000000));
    m[0] = 0x06; m[1] = 0;
    sp_msg(t, m, 2, in, NULL);
    mb_expect_u(in[1], 0x1D, "the SC16IS750 was reset (LCR 0x1D)");
    sp_close(t);
    case_end("I9 SC18IS604 -> SC16IS750: SPI commands, I2C with PULLUP-3V3 on SDA/SCL, the "
             "SC16IS750 sends 0x55; read after write; data NACK 0xF2");
}


int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_i8();
    case_i9();
    return serial_bench_finish();
}
