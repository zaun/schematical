/*
 * serial_i8251_cov_test: plan-68-P cases for the i8251 model (8251A) that
 * serial_intel_test does not reach: loader rejections, RESET at X, control
 * writes with X bits (mode, IR, ER, SBRK, RxE), a transmitter disable
 * while busy, synchronous transmit with parity, and a sync character with
 * an X bit. Facts: the header of src/models/serial_i8251.c (8251a.pdf,
 * plan-12-data/specs/classic-b.md § 8251A).
 */

#include "serial_bench.h"
#include "serial_bench_classic.h"
#include "serial_cov_classic.h"

static const char *const I8_D[8] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"};

/* TxC = RxC = 1 MHz; CLK 3.125 MHz. */
#define I8_HALF NS(500)
#define I8_CLK_HALF NS(160)
#define I8_BT1 (2 * I8_HALF)
#define I8_BT16 (16 * I8_BT1)

/* A write of v to C/D = cd; the data bits in xmask are driven X. */
static void i8_write_x(Bench *b, int cd, unsigned v, unsigned xmask) {
    unsigned i;

    set(b, "CD", cd ? LOGIC_1 : LOGIC_0);
    h_set_bus(b, I8_D, 8, 1, v);
    for (i = 0; i < 8; i++) {
        if ((xmask >> i) & 1u) {
            set(b, I8_D[i], LOGIC_X);
        }
    }
    set(b, "CS", LOGIC_0);
    run(b, NS(50));
    set(b, "WR", LOGIC_0);
    run(b, NS(300));
    set(b, "WR", LOGIC_1);
    run(b, NS(50));
    set(b, "CS", LOGIC_1);
    h_set_bus(b, I8_D, 8, 0, 0);
    run(b, NS(3000));
}

static void i8_write(Bench *b, int cd, unsigned v) {
    i8_write_x(b, cd, v, 0);
}

static void i8_expect(Bench *b, int cd, const char *want, const char *what) {
    char text[16];

    set(b, "CD", cd ? LOGIC_1 : LOGIC_0);
    set(b, "CS", LOGIC_0);
    run(b, NS(50));
    set(b, "RD", LOGIC_0);
    run(b, NS(300));
    h_get_bus(b, I8_D, 8, text);
    set(b, "RD", LOGIC_1);
    run(b, NS(50));
    set(b, "CS", LOGIC_1);
    run(b, NS(500));
    h_expect_text(text, want, what);
}

static HBench *i8_open(void) {
    static const HClockPin CLOCKS[3] = {{"TXC", 0}, {"RXC", 0}, {"CLK", 1}};
    static const SimTime HALVES[2] = {I8_HALF, I8_CLK_HALF};
    HBench *h = hb_open("SFX-8251A-dip28", CLOCKS, 3, HALVES, 2);
    Bench *b = &h->b;

    set(b, "RESET", LOGIC_1);
    set(b, "CS", LOGIC_1);
    set(b, "RD", LOGIC_1);
    set(b, "WR", LOGIC_1);
    set(b, "CD", LOGIC_0);
    set(b, "RXD", LOGIC_1);
    set(b, "CTS", LOGIC_0);
    set(b, "DSR", LOGIC_1);
    set(b, "SYNDET", LOGIC_Z);
    run(b, NS(5000));
    set(b, "RESET", LOGIC_0);
    run(b, NS(2000));
    return h;
}

/* Moves to just after a TxC/RxC falling edge. */
static void i8_align(Bench *b) {
    SimTime now = sim_now(b->sim);
    SimTime wait = (I8_BT1 - (now + I8_BT1 - NS(10)) % I8_BT1) % I8_BT1;

    if (wait > 0) {
        run(b, wait);
    }
}

/* Drives bits ('0'/'1'/'X') onto RXD, one per RxC period. */
static void i8_sync_bits(Bench *b, const char *bits) {
    size_t i;

    i8_align(b);
    for (i = 0; bits[i] != '\0'; i++) {
        set(b, "RXD", bits[i] == '1' ? LOGIC_1 : bits[i] == '0' ? LOGIC_0 : LOGIC_X);
        run(b, I8_BT1);
    }
}

/* n bits of v LSB first, appended to out. */
static void i8_bits(unsigned v, unsigned n, char *out) {
    size_t at = strlen(out);
    unsigned i;

    for (i = 0; i < n; i++) {
        out[at++] = (v >> i) & 1u ? '1' : '0';
    }
    out[at] = '\0';
}

/* ---- Loader ----------------------------------------------------------------------------- */

static void case_i8_loader(void) {
    const char *dev = "SFX-8251A";

    case_begin();
    scv_reject(dev, "{\"bus\":\"-\"}", "missing \"bus\"");
    scv_reject(dev, "{\"bus.strobe\":\"RD\"}", "unknown key \"strobe\"");
    scv_reject(dev, "{\"bus.data.7\":\"-\"}", "\"data\" needs exactly 8 pin names");
    scv_reject(dev, "{\"bus.read\":\"-\"}", "missing \"read\"");
    scv_reject(dev, "{\"clock\":7}", "\"clock\" must be a string");
    scv_reject(dev, "{\"serial.rts\":\"-\"}", "missing \"rts\"");
    scv_reject(dev, "{\"bus.reset\":\"NORESET\"}", "no pin named \"NORESET\"");
    case_end("I1 i8251 loader: bus, data, bus pins, clock, serial, pin names");
}

/* ---- RESET at X ----------------------------------------------------------------------------- */

static void case_i8_reset_x(void) {
    HBench *h = i8_open();
    Bench *b = &h->b;

    case_begin();
    i8_write(b, 1, 0x4E);
    i8_write(b, 1, 0x37);
    expect_pin(b, "DTR", LOGIC_0, "DTR low before RESET X");
    /* RESET X in the middle of a status read. */
    set(b, "CD", LOGIC_1);
    set(b, "CS", LOGIC_0);
    set(b, "RD", LOGIC_0);
    run(b, NS(300));
    set(b, "RESET", LOGIC_X);
    run(b, NS(2000));
    set(b, "RD", LOGIC_1);
    set(b, "CS", LOGIC_1);
    run(b, NS(500));
    expect_pin(b, "TXD", LOGIC_X, "RESET X: TxD X");
    expect_pin(b, "TXRDY", LOGIC_X, "RESET X: TxRDY X");
    expect_pin(b, "RXRDY", LOGIC_X, "RESET X: RxRDY X");
    expect_pin(b, "TXEMPTY", LOGIC_X, "RESET X: TxEMPTY X");
    expect_pin(b, "SYNDET", LOGIC_X, "RESET X: SYNDET X");
    expect_pin(b, "DTR", LOGIC_X, "RESET X: DTR X");
    expect_pin(b, "RTS", LOGIC_X, "RESET X: RTS X");
    /* Out of an X reset the control state is unknown: a write cannot be
       told mode from command, so it is not taken. */
    set(b, "RESET", LOGIC_0);
    run(b, NS(2000));
    i8_write(b, 1, 0x4E);
    i8_write(b, 1, 0x37);
    expect_pin(b, "DTR", LOGIC_X, "after RESET X: control writes are not taken");
    set(b, "RESET", LOGIC_1);
    run(b, NS(2000));
    set(b, "RESET", LOGIC_0);
    run(b, NS(2000));
    expect_pin(b, "DTR", LOGIC_1, "a real RESET: DTR high again");
    i8_write(b, 1, 0x4E);
    i8_write(b, 1, 0x37);
    expect_pin(b, "DTR", LOGIC_0, "a real RESET: the mode and command are taken");
    case_end("I2 8251A RESET X: every output X; after it control writes are ignored until RESET");
    hb_close(h);
}

/* ---- Control writes with X bits ----------------------------------------------------------- */

static void case_i8_x_writes(void) {
    HBench *h = i8_open();
    Bench *b = &h->b;
    char want[16];

    case_begin();
    /* A mode with X in B2 B1: the format is unknown; the next write is not
       taken as a command. */
    i8_write_x(b, 1, 0x4E, 0x01);
    i8_write(b, 1, 0x37);
    expect_pin(b, "DTR", LOGIC_1, "mode B1 X: the next control write is not a command");
    set(b, "RESET", LOGIC_1);
    run(b, NS(2000));
    set(b, "RESET", LOGIC_0);
    run(b, NS(2000));
    /* ER X: PE, OE and FE unknown. */
    i8_write(b, 1, 0x4E);
    i8_write_x(b, 1, 0x37, 0x10);
    i8_expect(b, 1, "00XXX101", "ER X: PE, OE, FE read X");
    /* SBRK X: TxD (marking) X. */
    i8_write_x(b, 1, 0x27, 0x08);
    run(b, I8_BT16);
    expect_pin(b, "TXD", LOGIC_X, "SBRK X: TxD X");
    i8_write(b, 1, 0x27);
    run(b, I8_BT16);
    expect_pin(b, "TXD", LOGIC_1, "SBRK 0: TxD marks");
    /* RxE X (async): the receiver does not run on an unknown enable. */
    i8_write_x(b, 1, 0x27, 0x04);
    h_frame_bits(0x5A, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, I8_BT16);
    run(b, I8_BT16);
    expect_pin(b, "RXRDY", LOGIC_0, "async RxE X: no character, RxRDY stays low");
    /* IR X: the state is unknown; DTR and RTS X, later writes not taken. */
    i8_write(b, 1, 0x27);
    expect_pin(b, "DTR", LOGIC_0, "command DTR");
    i8_write_x(b, 1, 0x27, 0x40);
    expect_pin(b, "DTR", LOGIC_X, "IR X: DTR X");
    expect_pin(b, "RTS", LOGIC_X, "IR X: RTS X");
    i8_write(b, 1, 0x40);
    expect_pin(b, "DTR", LOGIC_X, "IR X: a later IR is not taken either");
    case_end("I3 8251A X control bits: mode B1, ER, SBRK, RxE, IR");
    hb_close(h);
}

/* ---- A disable while busy ------------------------------------------------------------------ */

static void case_i8_disable_busy(void) {
    HBench *h = i8_open();
    Bench *b = &h->b;
    uint16_t log[4];
    size_t n;

    case_begin();
    i8_write(b, 1, 0x4E);
    i8_write(b, 1, 0x37);
    i8_write(b, 0, 'A');
    check(h_wait(b, "TXD", LOGIC_0, 2 * I8_BT16), "A starts");
    i8_write(b, 0, 'B');
    i8_write(b, 1, 0x36);   /* TxEN off while A is going out and B waits */
    run(b, 24 * I8_BT16);
    n = sent(b, log, 4);
    check(n == 2 && log[0] == 'A' && log[1] == 'B',
          "TxEN off while busy: the character written before still goes out");
    i8_write(b, 0, 'C');
    run(b, 12 * I8_BT16);
    check(sent(b, NULL, 0) == 2, "TxEN off: a character written after is held");
    case_end("I4 8251A: a transmitter disable while busy still sends what was written");
    hb_close(h);
}

/* ---- Synchronous transmit with parity ------------------------------------------------------- */

static void i8_capture(Bench *b, unsigned n, char *out) {
    unsigned i;

    for (i = 0; i < n; i++) {
        out[i] = "01XZ"[get(b, "TXD")];
        run(b, I8_BT1);
    }
    out[n] = '\0';
}

static void case_i8_sync_tx_parity(void) {
    HBench *h = i8_open();
    Bench *b = &h->b;
    char got[96];
    char want[48];

    case_begin();
    i8_write(b, 1, 0x3C);   /* sync, even parity, 8 bits, double sync */
    i8_write(b, 1, 0x16);
    i8_write(b, 1, 0x27);
    set(b, "CTS", LOGIC_1);
    i8_write(b, 1, 0x37);
    i8_write(b, 0, 0x01);
    set(b, "CTS", LOGIC_0);
    i8_capture(b, 60, got);
    want[0] = '\0';
    i8_bits(0x01, 8, want);
    strcat(want, "1");      /* one 1: the even parity bit is 1 */
    check(strstr(got, want) != NULL, "sync TX: data 0x01 with even parity 1");
    want[0] = '\0';
    i8_bits(0x16, 8, want);
    strcat(want, "1");      /* 0x16: three 1s */
    i8_bits(0x27, 8, want);
    strcat(want, "0");      /* 0x27: four 1s */
    check(strstr(got, want) != NULL, "sync TX fill: SYNC1, SYNC2 with their parity bits");
    /* Data with an X bit: that bit and the parity bit are X. */
    set(b, "CTS", LOGIC_1);
    run(b, NS(500));
    i8_write_x(b, 0, 0x00, 0x01);
    set(b, "CTS", LOGIC_0);
    i8_capture(b, 60, got);
    check(strstr(got, "X0000000X") != NULL, "sync TX: an X data bit makes the parity bit X");
    /* Odd parity, single sync. */
    i8_write(b, 1, 0x40);
    i8_write(b, 1, 0x9C);   /* sync, single, odd parity, 8 bits */
    i8_write(b, 1, 0x16);
    set(b, "CTS", LOGIC_1);
    i8_write(b, 1, 0x37);
    i8_write(b, 0, 0x03);
    set(b, "CTS", LOGIC_0);
    i8_capture(b, 40, got);
    want[0] = '\0';
    i8_bits(0x03, 8, want);
    strcat(want, "1");      /* two 1s: the odd parity bit is 1 */
    i8_bits(0x16, 8, want);
    strcat(want, "0");      /* three 1s: odd already */
    check(strstr(got, want) != NULL, "single sync, odd parity: data, then the SYNC1 fill");
    case_end("I5 8251A sync TX with parity: even, odd, fill characters, an X bit");
    hb_close(h);
}

/* ---- Sync receive with an X bit ---------------------------------------------------------- */

static void case_i8_sync_rx_x(void) {
    HBench *h = i8_open();
    Bench *b = &h->b;
    char bits[64];
    char text[16];

    case_begin();
    i8_write(b, 1, 0x9C);   /* sync, single, odd parity, 8 bits */
    i8_write(b, 1, 0x16);
    i8_write(b, 1, 0xB7);   /* EH RTS ER RxE DTR TxEN */
    strcpy(bits, "011");
    i8_bits(0x16, 8, bits);
    strcat(bits, "0");
    i8_sync_bits(b, bits);
    expect_pin(b, "SYNDET", LOGIC_1, "single sync with parity: SYNDET");
    strcpy(bits, "110X00000");   /* 0x03 with bit 3 X, parity 0 */
    i8_sync_bits(b, bits);
    expect_pin(b, "RXRDY", LOGIC_1, "the character after the sync");
    set(b, "CD", LOGIC_1);
    set(b, "CS", LOGIC_0);
    run(b, NS(50));
    set(b, "RD", LOGIC_0);
    run(b, NS(300));
    h_get_bus(b, I8_D, 8, text);
    set(b, "RD", LOGIC_1);
    set(b, "CS", LOGIC_1);
    run(b, NS(500));
    check(text[4] == 'X', "status PE X: the parity of a character with an X bit is unknown");
    i8_expect(b, 0, "0000X011", "the data with its X bit");
    /* RxE X (sync): the character is loaded, RxRDY unknown. */
    i8_write_x(b, 1, 0x37, 0x04);
    set(b, "RXD", LOGIC_1);
    run(b, 12 * I8_BT1);
    expect_pin(b, "RXRDY", LOGIC_X, "sync RxE X: RxRDY X after a character");
    case_end("I6 8251A sync RX: an X bit makes PE X; RxE X makes RxRDY X");
    hb_close(h);
}

int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_i8_loader();
    case_i8_reset_x();
    case_i8_x_writes();
    case_i8_disable_busy();
    case_i8_sync_tx_parity();
    case_i8_sync_rx_x();
    return serial_bench_finish();
}
