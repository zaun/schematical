/*
 * serial_target_more_test: plan-68-O coverage of the SPI/I2C target front ends
 * (models/serial_target.c): the "bus" loader's rejections, then the front ends
 * driven directly on the JF test target (support/serial_bench_maxim.c) in the
 * states serial_maxim_test's JF cases do not reach.
 */

#include "serial_bench.h"
#include "serial_bench_maxim.h"
#include "serial_bench_bridge.h"
#include "serial_bench_cov.h"

/* The SC16IS750's bus as I2C only (the same pins and table). */
#define I2C_ONLY                                                                               \
    "{\"bus\":{\"style\":\"i2c\",\"scl\":\"SCL/SCLK\",\"sda\":\"SDA\",\"address\":[\"CS/A0\","   \
    "\"SI/A1\"],\"table\":[77,76,78,79,73,72,74,75,81,80,82,83,85,84,86,87],\"reset\":\"RESET\"," \
    "\"resetActiveHigh\":false}"

static void case_loader(void) {
    case_begin();
    /* serial_host_load */
    cov_reject("SFX-SC18IS604", "{\"bus\":5}", "\"bus\" must be an object");
    cov_reject("SFX-SC18IS604", "{\"bus.style\":\"-\"}", "\"style\"");
    cov_reject("SFX-SC18IS604", "{\"bus.style\":\"uart\"}", "unknown bus style \"uart\"");
    cov_reject("SFX-SC18IS604", "{\"bus.speed\":1}", "unknown key \"speed\"");
    cov_reject("SFX-SC18IS604", "{\"bus.miso\":7}", "\"miso\" must be a string");
    cov_reject("SFX-SC18IS604", "{\"bus.mode\":\"-\"}", "the sc18is604 host port is \"spi\" mode 3");
    cov_reject("SFX-SC18IS604", "{\"bus.mode\":\"x\"}", "\"mode\" must be a whole number from 0 to 3");
    /* load_i2c, on the SC16IS750 with an I2C-only bus */
    cov_reject("SFX-SC16IS750", I2C_ONLY ",\"bus.sda\":\"-\"}", "\"sda\"");
    cov_reject("SFX-SC16IS750", I2C_ONLY ",\"bus.address\":3}", "\"address\" must be an array of strings");
    cov_reject("SFX-SC16IS750", I2C_ONLY ",\"bus.address\":[\"A\",\"B\",\"C\",\"D\"]}",
               "\"address\" has at most 3 pin names");
    cov_reject("SFX-SC16IS750", I2C_ONLY ",\"bus.table\":\"-\"}", "\"table\"");
    cov_reject("SFX-SC16IS750", I2C_ONLY ",\"bus.table\":[72]}",
               "\"table\" needs 16 entries (4 per address pin tie)");
    cov_reject("SFX-SC16IS750", I2C_ONLY ",\"bus.table.3\":128}",
               "\"table\" entries are 7-bit addresses (0-127) or null");
    cov_reject("SFX-SC16IS750", I2C_ONLY ",\"bus.table.3\":\"x\"}",
               "\"table\" entries are 7-bit addresses (0-127) or null");
    cov_reject("SFX-SC16IS750", I2C_ONLY ",\"bus.mode\":0}", "unknown key \"mode\"");
    /* i2c-spi */
    cov_reject("SFX-SC16IS750", "{\"bus.selectPin\":1}", "\"selectPin\" must be a string");
    cov_reject("SFX-SC16IS750", "{\"bus.spiLevel\":2}", "\"spiLevel\" must be a whole number from 0 to 1");
    cov_reject("SFX-SC16IS750", "{\"bus.selectSampled\":1}", "\"selectSampled\" must be a string");
    cov_reject("SFX-SC16IS750", "{\"bus.selectSampled\":\"never\"}",
               "\"selectSampled\" is \"live\" or \"reset\"");
    cov_reject("SFX-SC16IS750", "{\"bus.spi\":\"-\"}", "\"spi\"");
    cov_reject("SFX-SC16IS750", "{\"bus.spi.scl\":\"X\"}", "unknown key \"scl\"");
    cov_reject("SFX-SC16IS750", "{\"bus.i2c.cs\":\"X\"}", "unknown key \"cs\"");
    /* serial_host_bind */
    cov_reject("SFX-SC16IS750", "{\"bus.spi.miso\":\"NOPE\"}", "NOPE");
    cov_reject("SFX-SC16IS750", "{\"bus.i2c.sda\":\"NOPE\"}", "NOPE");
    cov_reject("SFX-SC16IS750", "{\"bus.i2c.address.1\":\"NOPE\"}", "NOPE");
    cov_reject("SFX-SC16IS750", "{\"bus.selectPin\":\"NOPE\"}", "NOPE");
    case_end("bus loader: serial_host_load and serial_host_bind rejections (spi, i2c, i2c-spi)");
}

/* ---- The test target on its own nets (straps A0/A1 driven by the bench) ---------------- */

static TgBench *tb_open(void) {
    static const char *const NAMES[TG_PINS] = {"CS", "SCLK", "MOSI", "MISO", "SCL", "SDA", "A0", "A1"};
    TgBench *t = calloc(1, sizeof *t);
    MB *b = &t->b;
    SimNet *nets[TG_PINS];
    size_t i;

    for (i = 0; i < TG_PINS; i++) {
        t->pin_table[i].name = NAMES[i];
    }
    t->part.pins = t->pin_table;
    t->part.pin_count = TG_PINS;
    b->part = &t->part;
    b->pins = TG_PINS;
    b->pull.count = 2;
    b->sim = sim_create();
    b->chip = sim_add_instance(b->sim, &TARGET, "T1", NULL);
    b->drv = sim_add_instance(b->sim, &DRIVER, "TB", &b->pins);
    b->pu = sim_add_instance(b->sim, &PULLUP, "PU", &b->pull);
    for (i = 0; i < TG_PINS; i++) {
        nets[i] = sim_add_net(b->sim, NAMES[i]);
        sim_connect(b->sim, nets[i], b->chip, i);
        sim_connect(b->sim, nets[i], b->drv, i);
    }
    sim_connect(b->sim, nets[TG_SCL], b->pu, 0);
    sim_connect(b->sim, nets[TG_SDA], b->pu, 1);
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
    mb_set(b, "SCLK", LOGIC_0);
    mb_set(b, "MOSI", LOGIC_0);
    mb_set(b, "A0", LOGIC_1);
    mb_set(b, "A1", LOGIC_0);
    mb_run(b, NS(1000));
    return t;
}

static void tb_close(TgBench *t) {
    sim_destroy(t->b.sim);
    free(t);
}

/* Logging callbacks the JF target does not have. */
static void log_bit(void *c, unsigned index, unsigned bits, uint8_t data, uint8_t known,
                    SerialSpiByte *out) {
    (void)index;
    (void)data;
    (void)known;
    (void)out;
    if (bits == 8) {
        tg_log(c, "b8 ", 0, 0);
    }
}

static void log_shift(void *c, unsigned bits) {
    tg_log(c, "s%u ", bits, 0);
}

static SerialSpiByte half_known(void *c, unsigned index) {
    SerialSpiByte b = {0xA5, 0x7F, 0xFF};   /* bit 7 unknown */

    tg_log(c, "N%u ", index, 0);
    return b;
}

static LogicValue ack_any(void *c, uint8_t byte, uint8_t known) {
    (void)known;
    tg_log(c, "A%02X ", byte, 0);
    return LOGIC_1;
}

/* Drives one SPI bit (mode `mode`): MOSI, the leading then the trailing edge;
   MISO as read before the sample edge. */
static LogicValue spi_bit(MB *b, unsigned mode, LogicValue mosi) {
    LogicValue idle = (mode >> 1) ? LOGIC_1 : LOGIC_0;
    LogicValue active = idle == LOGIC_1 ? LOGIC_0 : LOGIC_1;
    LogicValue miso;

    mb_set(b, "MOSI", mosi);
    if (mode & 1u) {
        mb_set(b, "SCLK", active);   /* shift edge */
        mb_run(b, SPI_HALF);
        miso = mb_get(b, "MISO");
        mb_set(b, "SCLK", idle);     /* sample edge */
        mb_run(b, SPI_HALF);
        return miso;
    }
    mb_run(b, SPI_HALF);
    miso = mb_get(b, "MISO");
    mb_set(b, "SCLK", active);
    mb_run(b, SPI_HALF);
    mb_set(b, "SCLK", idle);
    return miso;
}

static unsigned spi_byte(MB *b, unsigned mode, unsigned out, char *text) {
    unsigned v = 0;
    int i;

    for (i = 7; i >= 0; i--) {
        LogicValue l = spi_bit(b, mode, (out >> i) & 1u ? LOGIC_1 : LOGIC_0);

        if (text != NULL) {
            text[7 - i] = "01XZ"[l];
            text[8] = '\0';
        }
        v = (v << 1) | (l == LOGIC_1 ? 1u : 0u);
    }
    return v;
}

/* An I2C bit with any SDA level (Z = released, i.e. 1 on the pull-up). */
static LogicValue i2c_bit_v(MB *b, LogicValue sda) {
    LogicValue v;

    mb_set(b, "SDA", sda);
    mb_run(b, I2C_Q);
    mb_set(b, "SCL", LOGIC_Z);
    mb_run(b, I2C_Q);
    v = mb_get(b, "SDA");
    mb_run(b, I2C_Q);
    mb_set(b, "SCL", LOGIC_0);
    mb_run(b, I2C_Q);
    return v;
}

/* ---- SPI front end -------------------------------------------------------------------- */

static void case_spi_modes(void) {
    TgBench *t = tb_open();
    MB *b = &t->b;
    Tgt *g = tg_state(t);
    char text[16];
    unsigned v;

    case_begin();
    /* Mode 1 (CPHA 1): nothing is fetched at the CS fall; the first edge
       (rising, SCLK idles low) shifts bit 7 out, the falling edge samples. */
    g->sp.mode = 1;
    g->se.bit = log_bit;
    g->se.shift = log_shift;
    mb_set(b, "CS", LOGIC_0);
    mb_run(b, SPI_HALF);
    mb_expect_pin(b, "MISO", LOGIC_Z, "mode 1: MISO Z after the CS fall (no bit before the first edge)");
    v = spi_byte(b, 1, 0x3C, text);
    mb_expect_u(v, 0xA5, "mode 1: byte 0 shifted out on the leading edges");
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, SPI_HALF);
    check(strstr(g->log, "R3C ") != NULL, "mode 1: 0x3C sampled on the trailing edges");
    check(strstr(g->log, "s0 ") != NULL && strstr(g->log, "s7 ") != NULL,
          "mode 1: the shift event at each leading edge (0 and 7 bits sampled so far)");
    check(strstr(g->log, "b8 ") != NULL, "mode 1: the bit event at the 8th bit");
    /* Mode 3 (CPOL 1, CPHA 1): SCLK idles high; falling shifts, rising samples. */
    g->log[0] = '\0';
    g->sp.mode = 3;
    g->se.bit = NULL;
    g->se.shift = NULL;
    mb_set(b, "SCLK", LOGIC_1);
    mb_run(b, SPI_HALF);
    mb_set(b, "CS", LOGIC_0);
    mb_run(b, SPI_HALF);
    v = spi_byte(b, 3, 0x81, NULL);
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, SPI_HALF);
    mb_expect_u(v, 0xA5, "mode 3: byte 0 out");
    check(strcmp(g->log, "B R81 E8 ") == 0, "mode 3: 0x81 in, 8 bits");
    /* Mode 0 with SCLK high at the CS fall: the first (trailing) edge comes
       before any bit and shifts nothing; byte 0 is fetched once. */
    g->log[0] = '\0';
    g->sp.mode = 0;
    g->se.next = half_known;
    mb_set(b, "CS", LOGIC_0);
    mb_run(b, SPI_HALF);
    mb_expect_pin(b, "MISO", LOGIC_X, "byte 0 bit 7 unknown (known 0x7F): MISO X");
    mb_set(b, "SCLK", LOGIC_0);
    mb_run(b, SPI_HALF);
    mb_expect_pin(b, "MISO", LOGIC_X, "the early trailing edge leaves bit 7 on MISO");
    spi_byte(b, 0, 0x00, text);
    check(strcmp(text, "X0100101") == 0, "byte 0 = 0xA5 with bit 7 X");
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, SPI_HALF);
    check(strcmp(g->log, "B N0 R00 E8 ") == 0, "next(0) asked once (at the CS fall), not again at the early edge");
    /* No callbacks: next NULL leaves MISO Z; no begin, received or end events. */
    g->log[0] = '\0';
    g->se.next = NULL;
    g->se.begin = NULL;
    g->se.received = NULL;
    g->se.end = NULL;
    mb_set(b, "CS", LOGIC_0);
    mb_run(b, SPI_HALF);
    spi_byte(b, 0, 0x55, text);
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, SPI_HALF);
    check(strcmp(text, "ZZZZZZZZ") == 0, "next NULL: nothing driven");
    check(g->log[0] == '\0', "begin, received, end NULL: no events");
    tb_close(t);
    case_end("SPI target: modes 1 and 3, bit and shift events, an early trailing edge in mode 0, "
             "unknown MISO bits, NULL callbacks");
}

static void case_spi_x_hold(void) {
    TgBench *t = tb_open();
    MB *b = &t->b;
    Tgt *g = tg_state(t);
    char text[16];
    size_t cs = TG_CS;

    case_begin();
    /* SCLK X mid-transaction: MISO X and the rest of the bits unknown. */
    mb_set(b, "CS", LOGIC_0);
    mb_run(b, SPI_HALF);
    spi_bit(b, 0, LOGIC_1);
    mb_set(b, "SCLK", LOGIC_X);
    mb_run(b, SPI_HALF);
    mb_expect_pin(b, "MISO", LOGIC_X, "SCLK X: MISO drives X");
    mb_set(b, "SCLK", LOGIC_0);
    mb_run(b, SPI_HALF);
    spi_byte(b, 0, 0xFF, NULL);
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, SPI_HALF);
    check(strcmp(g->log, "B R80? E9 ") == 0, "after SCLK X: bit 7 (1) known, the 7 bits after it unknown");
    /* SCLK X while idle: nothing happens. */
    g->log[0] = '\0';
    mb_set(b, "SCLK", LOGIC_X);
    mb_run(b, SPI_HALF);
    mb_set(b, "SCLK", LOGIC_0);
    mb_run(b, SPI_HALF);
    mb_expect_pin(b, "MISO", LOGIC_Z, "SCLK X with CS high: MISO stays Z");
    check(g->log[0] == '\0', "no event");
    /* serial_spi_hold outside a transaction: no hold. */
    serial_spi_hold(&g->spi, b->sim, b->chip, 1, 0);
    check(g->spi.held == 0, "a hold with CS high does nothing");
    /* A repeated hold state is ignored; release drives the bit again. */
    mb_set(b, "CS", LOGIC_0);
    mb_run(b, SPI_HALF);
    serial_spi_hold(&g->spi, b->sim, b->chip, 1, 0);
    serial_spi_hold(&g->spi, b->sim, b->chip, 1, 0);
    mb_run(b, SPI_HALF);
    mb_expect_pin(b, "MISO", LOGIC_Z, "held: MISO Z");
    serial_spi_hold(&g->spi, b->sim, b->chip, 0, 0);
    mb_run(b, SPI_HALF);
    mb_expect_pin(b, "MISO", LOGIC_1, "released: bit 7 of 0xA5 again");
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, SPI_HALF);
    /* A disabled target ignores its pins. */
    g->log[0] = '\0';
    serial_spi_enable(&g->spi, b->sim, b->chip, 0);
    mb_set(b, "CS", LOGIC_0);
    mb_run(b, SPI_HALF);
    spi_byte(b, 0, 0x12, text);
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, SPI_HALF);
    check(g->log[0] == '\0' && strcmp(text, "ZZZZZZZZ") == 0, "disabled: no events, MISO Z");
    serial_spi_enable(&g->spi, b->sim, b->chip, 1);
    /* An unbound CS (SERIAL_NO_PIN) and a call with SERIAL_NO_PIN: ignored,
       not taken as a CS change. */
    g->sp.cs = SERIAL_NO_PIN;
    serial_spi_pin(&g->spi, b->sim, b->chip, SERIAL_NO_PIN, &g->se);
    check(g->log[0] == '\0' && !g->spi.active, "SERIAL_NO_PIN: no transaction");
    g->sp.cs = cs;
    /* No MISO pin: the transaction runs, nothing is driven. */
    g->sp.miso = SERIAL_NO_PIN;
    mb_set(b, "CS", LOGIC_0);
    mb_run(b, SPI_HALF);
    spi_byte(b, 0, 0x34, text);
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, SPI_HALF);
    check(strcmp(text, "ZZZZZZZZ") == 0 && strstr(g->log, "R34 ") != NULL,
          "no MISO pin: 0x34 received, MISO never driven");
    g->sp.miso = TG_MISO;
    tb_close(t);
    case_end("SPI target: SCLK X during and outside a transaction, hold outside a transaction "
             "and repeated, disabled target, SERIAL_NO_PIN, no MISO pin");
}

/* ---- I2C front end -------------------------------------------------------------------- */

static void case_i2c_x(void) {
    TgBench *t = tb_open();
    MB *b = &t->b;
    Tgt *g = tg_state(t);
    LogicValue ack;
    unsigned i;

    case_begin();
    check(serial_i2c_strap(&g->i2c, 2) == -1, "strap index past strap_count: -1");
    /* A0 X: its level matches no tie. */
    mb_set(b, "A0", LOGIC_X);
    mb_run(b, I2C_Q);
    i2c_start(b);
    i2c_write(b, 0x2A << 1);
    i2c_stop(b);
    check(g->ties[0] == -1, "A0 X: strap unknown (-1)");
    mb_set(b, "A0", LOGIC_1);
    mb_run(b, I2C_Q);
    /* SCL X after the address ACK: the transaction ends (end(0)), SDA released. */
    g->log[0] = '\0';
    i2c_start(b);
    check(i2c_write(b, 0x2A << 1) == LOGIC_0, "address ACK");
    mb_set(b, "SCL", LOGIC_X);
    mb_run(b, I2C_Q);
    mb_set(b, "SCL", LOGIC_0);
    mb_run(b, I2C_Q);
    ack = i2c_write(b, 0x41);
    check(ack == LOGIC_1, "after SCL X the target ignores the bus: no ACK");
    i2c_stop(b);
    check(strcmp(g->log, "S A54 Px ") == 0, "SCL X: end(0), no write");
    /* SDA X while SCL is high: the same. */
    g->log[0] = '\0';
    i2c_start(b);
    i2c_write(b, 0x2A << 1);
    mb_set(b, "SCL", LOGIC_Z);
    mb_run(b, I2C_Q);
    mb_set(b, "SDA", LOGIC_X);
    mb_run(b, I2C_Q);
    mb_set(b, "SDA", LOGIC_0);
    mb_set(b, "SCL", LOGIC_0);
    mb_run(b, I2C_Q);
    i2c_stop(b);
    check(strcmp(g->log, "S A54 Px ") == 0, "SDA X with SCL high: end(0)");
    /* X on SCL with the bus idle: no event. */
    g->log[0] = '\0';
    mb_set(b, "SCL", LOGIC_X);
    mb_run(b, I2C_Q);
    mb_set(b, "SCL", LOGIC_Z);
    mb_run(b, I2C_Q);
    check(g->log[0] == '\0', "SCL X while idle: nothing");
    /* The model ACKs an address whose R/W bit is X: the ACK slot drives X. */
    g->ie.address = ack_any;
    i2c_start(b);
    for (i = 0; i < 7; i++) {
        i2c_bit_v(b, (0x2Au >> (6 - i)) & 1u ? LOGIC_Z : LOGIC_0);
    }
    i2c_bit_v(b, LOGIC_X);
    ack = i2c_bit_v(b, LOGIC_Z);
    check(ack == LOGIC_X, "addressed with R/W unknown: the ACK slot is X");
    i2c_stop(b);
    g->ie.address = tg_i2c_address;
    /* A disabled target, and SERIAL_NO_PIN: ignored. */
    g->log[0] = '\0';
    serial_i2c_enable(&g->i2c, b->sim, b->chip, 0);
    i2c_start(b);
    ack = i2c_write(b, 0x2A << 1);
    i2c_stop(b);
    check(ack == LOGIC_1 && g->log[0] == '\0', "disabled: no ACK, no event");
    serial_i2c_enable(&g->i2c, b->sim, b->chip, 1);
    g->ip.sda = SERIAL_NO_PIN;
    serial_i2c_pin(&g->i2c, b->sim, b->chip, SERIAL_NO_PIN, &g->ie);
    g->ip.sda = TG_SDA;
    check(g->log[0] == '\0' && g->i2c.phase == SERIAL_I2C_IDLE, "SERIAL_NO_PIN: no START seen");
    tb_close(t);
    case_end("I2C target: strap index range, strap levels that match no tie or are X, X on SCL "
             "or SDA (end(0)), the R/W bit X, disabled target, SERIAL_NO_PIN");
}

/* ---- The host port (serial_host_*) ---------------------------------------------------------- */

static void case_host(void) {
    const char *part = cov_part("SFX-SC16IS750", I2C_ONLY "}");
    Br r = br_open(part, 0, 0, NULL);
    SerialHostDevice dev;
    SerialHostPins pins;
    SerialHost host;

    case_begin();
    /* "style": "i2c": the I2C target is on from reset; no SPI select pin. */
    br_expect(&r, 0, 3, 0x1D, "I2C-only bus: LCR 0x1D at 0x48 (A1, A0 high)");
    br_w(&r, 0, 7, 0x5A);
    br_expect(&r, 0, 7, 0x5A, "I2C-only bus: SPR written and read");
    /* The straps are pins of the front end of their own (not SPI pins here):
       a change while the bus is idle is sampled. */
    mb_set(r.b, "CS/A0", LOGIC_0);
    mb_run(r.b, NS(2000));
    r.addr = nxp_address(1, 0);
    br_expect(&r, 0, 7, 0x5A, "A0 now low: the chip answers at 0x49");
    i2c_start(r.b);
    check(i2c_write(r.b, 0x48 << 1) == LOGIC_1, "and no longer at 0x48");
    i2c_stop(r.b);
    mb_close(r.b);
    /* API: SERIAL_NO_PIN is none of the front end's pins, even where a pin
       (here the select) is unbound. */
    memset(&dev, 0, sizeof dev);
    dev.style = SERIAL_HOST_SPI;
    memset(&pins, 0, sizeof pins);
    pins.spi.cs = pins.spi.sclk = pins.spi.mosi = pins.spi.miso = 0;
    pins.i2c.scl = pins.i2c.sda = 1;
    pins.select = SERIAL_NO_PIN;
    check(serial_host_has_pin(&dev, &pins, SERIAL_NO_PIN) == 0, "has_pin(SERIAL_NO_PIN) = 0");
    /* serial_host_pin(SERIAL_NO_PIN) does nothing, not even with a live select
       that is unbound (which would read X and turn both front ends off). */
    dev.style = SERIAL_HOST_I2C_SPI;
    dev.select_live = 1;
    serial_host_init(&host, &dev, &pins);
    host.mode = SERIAL_HOST_ON_SPI;
    serial_host_pin(&host, NULL, NULL, SERIAL_NO_PIN, NULL, NULL);
    check(host.mode == SERIAL_HOST_ON_SPI, "host_pin(SERIAL_NO_PIN): the mode stays");
    case_end("host port: an I2C-only bus, straps that are not SPI pins, SERIAL_NO_PIN calls");
}

int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_loader();
    case_spi_modes();
    case_spi_x_hold();
    case_i2c_x();
    case_host();
    return serial_bench_finish();
}
