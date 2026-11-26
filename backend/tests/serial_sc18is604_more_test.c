/*
 * serial_sc18is604_more_test: plan-68-O coverage of the SC18IS604 SPI-to-I2C
 * bridge (models/serial_sc18is604.c): loader rejections and the command,
 * engine, reset and X states serial_sc18is604_test does not drive.
 */

#include "serial_bench.h"
#include "serial_bench_maxim.h"
#include "serial_bench_bridge.h"
#include "serial_bench_cov.h"

static void case_loader(void) {
    case_begin();
    cov_reject("SFX-SC18IS604", "{\"bus\":\"-\"}", "\"bus\"");
    cov_reject("SFX-SC18IS604", "{\"bus.mode\":0}", "the sc18is604 host port is \"spi\" mode 3");
    cov_reject("SFX-SC18IS604", "{\"i2c\":\"-\"}", "\"i2c\"");
    cov_reject("SFX-SC18IS604", "{\"i2c.int\":\"INT\"}", "unknown key \"int\"");
    cov_reject("SFX-SC18IS604", "{\"i2c.sda\":\"-\"}", "\"sda\"");
    cov_reject("SFX-SC18IS604", "{\"reset\":\"-\"}", "\"reset\"");
    cov_reject("SFX-SC18IS604", "{\"gpio\":5}", "\"gpio\" must be an array of strings");
    cov_reject("SFX-SC18IS604", "{\"gpio.4\":\"-\"}",
               "\"gpio\" needs 5 pin names (GPIO0 first; GPIO4 is an input)");
    cov_reject("SFX-SC18IS604", "{\"bus.cs\":\"NOPE\"}", "NOPE");
    cov_reject("SFX-SC18IS604", "{\"i2c.scl\":\"NOPE\"}", "NOPE");
    cov_reject("SFX-SC18IS604", "{\"gpio.2\":\"NOPE\"}", "NOPE");
    case_end("sc18is604 loader: every rejection of sp_load_device and sp_bind");
}


/* ---- The bench: the SC18IS604, the JF target at 0x2A, pull-ups, a clock stretcher --------- */

/* Holds SCL low for STRETCH after each fall it sees (a target stretching the clock). */
#define STRETCH NS(12000)

typedef struct {
    int on;
    int holding;
} Stretch;

static size_t st_pins(const void *config) {
    (void)config;
    return 1;
}

static void *st_create(const void *config) {
    (void)config;
    return calloc(1, sizeof(Stretch));
}

static void st_changed(Sim *sim, SimInstance *instance, void *state, size_t pin, LogicValue old_value,
                       LogicValue new_value) {
    Stretch *st = state;

    (void)pin;
    if (st->on && !st->holding && old_value == LOGIC_1 && new_value == LOGIC_0) {
        st->holding = 1;
        sim_schedule_drive(sim, instance, 0, LOGIC_0, DRIVE_STRONG, 0);
        sim_schedule_timer(sim, instance, 1, STRETCH);
    }
}

static void st_timer(Sim *sim, SimInstance *instance, void *state, uint32_t id) {
    Stretch *st = state;

    (void)id;
    st->holding = 0;
    sim_schedule_drive(sim, instance, 0, LOGIC_Z, DRIVE_STRONG, 0);
}

static const SimComponentType STRETCHER = {
    "stretcher", st_pins, st_create, free, NULL, st_changed, st_timer,
    NULL, NULL, NULL, NULL, NULL
};

typedef struct {
    MB b;
    SimInstance *tgt;
    SimInstance *st;
} Sp;

static Sp *sp_open(void) {
    static const char *const PULLS[] = {"SCL", "SDA", "INT", NULL};
    Sp *t = calloc(1, sizeof *t);
    MB *b = &t->b;
    SimNet **nets;
    size_t i;

    b->part = part_library_find(library, "SFX-SC18IS604");
    b->pins = b->part->pin_count;
    b->pull.count = 3;
    b->sim = sim_create();
    b->chip = sim_add_instance(b->sim, kind_model(b->part), "U1", b->part);
    b->drv = sim_add_instance(b->sim, &DRIVER, "TB", &b->pins);
    b->pu = sim_add_instance(b->sim, &PULLUP, "PU", &b->pull);
    t->tgt = sim_add_instance(b->sim, &TARGET, "T1", NULL);
    t->st = sim_add_instance(b->sim, &STRETCHER, "ST", NULL);
    nets = calloc(b->pins, sizeof *nets);
    for (i = 0; i < b->pins; i++) {
        nets[i] = sim_add_net(b->sim, b->part->pins[i].name);
        sim_connect(b->sim, nets[i], b->chip, i);
        sim_connect(b->sim, nets[i], b->drv, i);
    }
    for (i = 0; PULLS[i] != NULL; i++) {
        sim_connect(b->sim, nets[mb_pin(b, PULLS[i])], b->pu, i);
    }
    for (i = 0; i < TG_SCL; i++) {
        sim_connect(b->sim, sim_add_net(b->sim, "tg"), t->tgt, i);
    }
    sim_connect(b->sim, nets[mb_pin(b, "SCL")], t->tgt, TG_SCL);
    sim_connect(b->sim, nets[mb_pin(b, "SDA")], t->tgt, TG_SDA);
    sim_connect(b->sim, nets[mb_pin(b, "SCL")], t->tgt, TG_A0);
    sim_connect(b->sim, nets[mb_pin(b, "SDA")], t->tgt, TG_A1);
    sim_connect(b->sim, nets[mb_pin(b, "SCL")], t->st, 0);
    free(nets);
    if (!sim_start(b->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(b->sim));
        exit(2);
    }
    mb_set(b, "CS", LOGIC_1);
    mb_set(b, "SCLK", LOGIC_1);
    mb_set(b, "MOSI", LOGIC_0);
    mb_set(b, "RESET", LOGIC_1);
    mb_run(b, NS(4000));
    return t;
}

static void sp_close(Sp *t) {
    sim_destroy(t->b.sim);
    free(t);
}

static Tgt *sp_tgt(Sp *t) {
    return sim_instance_state(t->tgt);
}

#define SP_HALF NS(500)

/* One message, mode 3, from bit characters ('0' '1' 'X', spaces skipped); MISO per bit. */
static void sp_bits(Sp *t, const char *bits, char *miso) {
    MB *b = &t->b;
    size_t n = 0;

    mb_set(b, "CS", LOGIC_0);
    mb_run(b, SP_HALF);
    for (; *bits != '\0'; bits++) {
        if (*bits == ' ') {
            continue;
        }
        mb_set(b, "SCLK", LOGIC_0);
        mb_set(b, "MOSI", lv(*bits));
        mb_run(b, SP_HALF);
        if (miso != NULL) {
            miso[n] = "01XZ"[mb_get(b, "MISO")];
        }
        n++;
        mb_set(b, "SCLK", LOGIC_1);
        mb_run(b, SP_HALF);
    }
    if (miso != NULL) {
        miso[n] = '\0';
    }
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, SP_HALF);
}

/* A message of whole bytes. */
static void sp_msg(Sp *t, const uint8_t *m, unsigned n) {
    char bits[8 * 32 + 1];
    unsigned i;
    unsigned k;

    for (i = 0; i < n; i++) {
        for (k = 0; k < 8; k++) {
            bits[i * 8 + k] = (m[i] >> (7 - k)) & 1u ? '1' : '0';
        }
    }
    bits[n * 8] = '\0';
    sp_bits(t, bits, NULL);
}

static void sp_reg_w(Sp *t, unsigned reg, unsigned v) {
    uint8_t m[3] = {0x20, (uint8_t)reg, (uint8_t)v};

    sp_msg(t, m, 3);
}

/* Register read: the value as bit characters. */
static void sp_reg_text(Sp *t, unsigned reg, char *text) {
    char bits[25];
    char miso[25];
    unsigned k;

    snprintf(bits, sizeof bits, "00100001");
    for (k = 0; k < 8; k++) {
        bits[8 + k] = (reg >> (7 - k)) & 1u ? '1' : '0';
    }
    memset(bits + 16, '0', 8);
    bits[24] = '\0';
    sp_bits(t, bits, miso);
    memcpy(text, miso + 16, 8);
    text[8] = '\0';
}

static unsigned sp_reg_r(Sp *t, unsigned reg) {
    char text[9];
    unsigned v = 0;
    unsigned k;

    sp_reg_text(t, reg, text);
    for (k = 0; k < 8; k++) {
        v = (v << 1) | (text[k] == '1' ? 1u : 0u);
    }
    return v;
}

static unsigned sp_do(Sp *t, const uint8_t *m, unsigned n, SimTime limit) {
    sp_msg(t, m, n);
    if (mb_wait(&t->b, "INT", LOGIC_0, limit, NS(500)) < 0) {
        return 0x100;
    }
    return sp_reg_r(t, 0x04);
}

/* A STOP from the bench: after an X outcome the controller lets go of the bus
   mid-byte, without one, and its bus monitor still sees the bus busy. */
static void bus_stop(Sp *t) {
    mb_set(&t->b, "SCL", LOGIC_0);   /* a last SCL pulse: the target leaves its ACK slot */
    mb_run(&t->b, NS(2000));
    mb_set(&t->b, "SCL", LOGIC_Z);
    mb_run(&t->b, NS(2000));
    mb_set(&t->b, "SDA", LOGIC_0);
    mb_run(&t->b, NS(2000));
    mb_set(&t->b, "SDA", LOGIC_Z);
    mb_run(&t->b, NS(2000));
}

static void expect_text(const char *got, const char *want, const char *what) {
    char message[200];

    snprintf(message, sizeof message, "%s: %s, want %s", what, got, want);
    check(strcmp(got, want) == 0, message);
}

/* ---- Cases --------------------------------------------------------------------------------- */

static void case_unknown(void) {
    Sp *t = sp_open();
    MB *b = &t->b;
    char text[40];

    case_begin();
    /* RESET X: the whole chip is unknown: INT, the I2C lines and the GPIO outputs X. */
    sp_reg_w(t, 0x00, 0x02);
    mb_set(b, "RESET", LOGIC_X);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "INT", LOGIC_X, "RESET X: INT X");
    mb_expect_pin(b, "SCL", LOGIC_X, "RESET X: SCL X");
    mb_expect_pin(b, "GPIO0", LOGIC_X, "RESET X: GPIO0 X");
    mb_set(b, "RESET", LOGIC_1);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "SCL", LOGIC_1, "RESET high again: lines released");
    /* A register write with an unknown register number: every register X. */
    sp_bits(t, "00100000 0000000X 00000010", NULL);
    sp_reg_text(t, 0x00, text);
    expect_text(text, "XXXXXXXX", "IOConfig after a write to register 0/1");
    mb_run(b, NS(200));
    mb_expect_pin(b, "GPIO0", LOGIC_X, "IOConfig unknown: GPIO0 X");
    sp_reg_w(t, 0x00, 0x00);
    sp_reg_w(t, 0x02, 0x19);
    sp_reg_w(t, 0x03, 0x00);
    /* A command byte that may be 0x00 or 0x20: the register write may have happened. */
    sp_bits(t, "00X00000 00000101 01010101", text);
    expect_text(text + 8, "XXXXXXXXXXXXXXXX", "unknown command: MISO X after the command byte");
    sp_reg_text(t, 0x05, text);
    expect_text(text, "XXXXXXXX", "I2CAdr after a command that may have been 0x20");
    /* A command that cannot be 0x20 changes nothing. */
    sp_reg_w(t, 0x05, 0x44);
    sp_bits(t, "X0000001 00000101 01010101", NULL);
    mb_expect_u(sp_reg_r(t, 0x05), 0x44, "a command never 0x20: I2CAdr kept");
    /* I2CStat is read-only; register 7 does not exist (reads X). */
    sp_reg_w(t, 0x04, 0x12);
    mb_expect_u(sp_reg_r(t, 0x04), 0xF0, "a write to I2CStat is ignored");
    sp_reg_w(t, 0x07, 0x12);
    sp_reg_text(t, 0x07, text);
    expect_text(text, "XXXXXXXX", "register 7 reads X");
    /* IOState reads an input at X as X. */
    mb_set(b, "GPIO4", LOGIC_X);
    sp_reg_text(t, 0x01, text);
    check(text[3] == 'X', "IOState[4] X for GPIO4 at X");
    /* A byte count that is X: the outcome (I2CStat) and INT are X. */
    sp_bits(t, "00000000 0000000X 01010100 01000001", NULL);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "INT", LOGIC_X, "byte count X: INT X");
    sp_reg_text(t, 0x04, text);
    expect_text(text, "1111XXXX", "byte count X: I2CStat 0xFx");
    check(sim_instance_type(b->chip)->serial_channels(sim_instance_state(b->chip)) == 0,
          "the SC18IS604 has no UART channels");
    sp_close(t);
    case_end("SC18IS604 X: RESET X, register number X, command byte X, read-only and missing "
             "registers, GPIO input X, byte count X, no channels");
}

static void case_commands(void) {
    Sp *t = sp_open();
    MB *b = &t->b;
    uint8_t m[12];

    case_begin();
    /* Byte counts of 0 or past the message: 0xF9. */
    m[0] = 0x01; m[1] = 0; m[2] = 0x55;
    mb_expect_u(sp_do(t, m, 3, NS(100000)), 0xF9, "read N = 0: 0xF9");
    m[0] = 0x02; m[1] = 1; m[2] = 0; m[3] = 0x54; m[4] = 0x10; m[5] = 0x55;
    mb_expect_u(sp_do(t, m, 6, NS(100000)), 0xF9, "read after write, Nr = 0: 0xF9");
    m[0] = 0x03; m[1] = 1; m[2] = 2; m[3] = 0x54; m[4] = 0x20; m[5] = 0x54; m[6] = 0x21;
    mb_expect_u(sp_do(t, m, 7, NS(100000)), 0xF9, "write after write, N2 past the message: 0xF9");
    /* A command while one is running is ignored (P2). */
    sp_tgt(t)->log[0] = '\0';
    m[0] = 0x00; m[1] = 1; m[2] = 0x54; m[3] = 0x41;
    sp_msg(t, m, 4);
    m[3] = 0x77;
    sp_msg(t, m, 4);
    mb_wait(b, "INT", LOGIC_0, NS(1000000), NS(500));
    mb_expect_u(sp_reg_r(t, 0x04), 0xF0, "the first write completes");
    mb_run(b, NS(1000000));
    expect_text(sp_tgt(t)->log, "S A54 W41 P ", "only the first transfer reached the target");
    /* I2CClock = 0: the SCL generator stops; the transfer never ends (decisions.md). */
    sp_reg_w(t, 0x02, 0x00);
    sp_msg(t, m, 4);
    mb_run(b, NS(2000000));
    mb_expect_u(sp_reg_r(t, 0x04), 0xF3, "I2CClock 0: still busy (0xF3)");
    mb_expect_pin(b, "INT", LOGIC_1, "no INT");
    sp_close(t);
    case_end("SC18IS604 commands: count errors for 0x01-0x03, a command while busy, I2CClock 0");
}

static LogicValue ack_x(void *c, uint8_t byte, uint8_t known) {
    (void)c;
    (void)known;
    return (byte >> 1) == 0x2A ? LOGIC_X : LOGIC_0;
}

static LogicValue log_known(void *c, unsigned index, uint8_t data, uint8_t known) {
    (void)index;
    tg_log(c, "W%02X/", data, 0);
    tg_log(c, "%02X ", known, 0);
    return LOGIC_1;
}

static void next_half(void *c, unsigned index, uint8_t *data, uint8_t *known) {
    (void)c;
    (void)index;
    *data = 0xA5;
    *known = 0x0F;
}

static void case_bus(void) {
    Sp *t = sp_open();
    MB *b = &t->b;
    Stretch *st = sim_instance_state(t->st);
    uint8_t m[12];
    char text[40];

    case_begin();
    sp_reg_w(t, 0x02, 7);
    /* Every SCL fall stretched 12 us: the controller waits in its high, repeated
       START and STOP steps; the transfer still completes. */
    st->on = 1;
    m[0] = 0x02; m[1] = 1; m[2] = 1; m[3] = 0x54; m[4] = 0x10; m[5] = 0x55;
    mb_expect_u(sp_do(t, m, 6, NS(5000000)), 0xF0, "stretched read after write: 0xF0");
    expect_text(sp_tgt(t)->log, "S A54 W10 Sr A55 N0 KN P ", "stretched: S W, Sr R, P at the target");
    st->on = 0;
    mb_run(b, STRETCH);
    /* The target ACKs with X: I2CStat 0xFx (X). */
    sp_tgt(t)->ie.address = ack_x;
    m[0] = 0x00; m[1] = 1; m[2] = 0x54; m[3] = 0x41;
    sp_msg(t, m, 4);
    mb_wait(b, "INT", LOGIC_0, NS(1000000), NS(500));
    sp_reg_text(t, 0x04, text);
    expect_text(text, "1111XXXX", "ACK slot X: I2CStat unknown low nibble");
    sp_tgt(t)->ie.address = tg_i2c_address;
    bus_stop(t);
    /* SDA X while the controller sends a 1: the outcome is X. */
    m[0] = 0x00; m[1] = 1; m[2] = 0x54; m[3] = 0x41;
    sp_msg(t, m, 4);
    mb_wait(b, "SDA", LOGIC_0, NS(100000), NS(20));
    mb_set(b, "SDA", LOGIC_X);
    check(mb_wait(b, "INT", LOGIC_0, NS(1000000), NS(200)) >= 0, "SDA X: the transfer ends");
    mb_set(b, "SDA", LOGIC_Z);
    mb_run(b, NS(20000));
    sp_reg_text(t, 0x04, text);
    expect_text(text, "1111XXXX", "SDA X against a 1 bit: I2CStat unknown");
    bus_stop(t);
    /* A data bit X goes onto SDA as X: the target receives that bit unknown. */
    sp_tgt(t)->log[0] = '\0';
    sp_tgt(t)->ie.received = log_known;
    sp_bits(t, "00000000 00000001 01010100 0100X001", NULL);
    mb_wait(b, "INT", LOGIC_0, NS(1000000), NS(500));
    mb_expect_u(sp_reg_r(t, 0x04), 0xF0, "data bit X: still ACKed by the target (0xF0)");
    expect_text(sp_tgt(t)->log, "S A54 W41/F7 P ", "the target got 0x41 with bit 3 unknown");
    sp_tgt(t)->ie.received = tg_i2c_received;
    /* A read where the target drives X bits: the buffer keeps them unknown. */
    sp_tgt(t)->ie.next = next_half;
    m[0] = 0x01; m[1] = 1; m[2] = 0x55;
    mb_expect_u(sp_do(t, m, 3, NS(1000000)), 0xF0, "read with X bits: 0xF0");
    sp_bits(t, "00000110 00000000", text);
    expect_text(text + 8, "XXXX0101", "0x06: the high nibble X, the low 0x5");
    sp_close(t);
    case_end("SC18IS604 bus: clock stretching in the high, repeated START and STOP steps, an X "
             "ACK, SDA X against a 1, an X data bit, X bits read");
}

int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_loader();
    case_unknown();
    case_commands();
    case_bus();
    return serial_bench_finish();
}
