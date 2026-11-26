/*
 * serial_xr1280_more_test: plan-68-O coverage of the XR20M1280 bridge
 * (models/ns16550_xr1280.c and its hooks in ns16550_bridge*.c): the 3-bit map
 * in its unknown states, the baud generators, auto RS-485, flow control,
 * EMSR, 9-bit mode, sleep and the GPIO set. Sources: the model's header
 * comment and plan-12-data/specs/bridges.md § XR20M1280.
 */

#include "serial_bench.h"
#include "serial_bench_maxim.h"
#include "serial_bench_bridge.h"
#include "serial_bench_cov.h"

#define XR_PART "SFX-XR20M1280-qfn32"

static void xw(Br *r, unsigned reg, unsigned v) {
    br_w(r, 0, reg, v);
}

static Br xr_open(void) {
    Br r = br_open(XR_PART, 1, 1, NULL);

    xw(&r, 3, 0xBF);
    xw(&r, 2, 0x10);   /* EFR[4]: the enhanced bits */
    xw(&r, 3, 0x80);
    xw(&r, 0, 0x01);
    xw(&r, 1, 0x00);
    xw(&r, 3, 0x03);
    return r;
}

/* LCR 0xBF, then back to 8N1. */
static void bf(Br *r) {
    xw(r, 3, 0xBF);
}

static void n81(Br *r) {
    xw(r, 3, 0x03);
}

/* ---- Decode and registers --------------------------------------------------------------- */

static void case_registers(void) {
    Br r = xr_open();
    MB *b = r.b;
    char text[12];

    case_begin();
    /* LCR maybe 0xBF: everything but LCR reads X. */
    wx(&r, 0, 3, "1011111X");
    br_expect_text(&r, 0, 7, "XXXXXXXX", "LCR 1011111X: SPR's address reads X");
    wx(&r, 0, 3, "X0000011");
    br_expect_text(&r, 0, 7, "XXXXXXXX", "LCR[7] X: SPR or the divisor set: X");
    n81(&r);
    /* MSR read at 6 (SFR is write-only there). */
    br_r(&r, 0, 6, text);
    check(text[3] == '0', "MSR[4]: CTS# high is CTS inactive (0)");
    /* SFR[0] X: the GPIO or Xon/Xoff bank at 0xBF, GPIOLVL or SPR at 7: X. */
    wx(&r, 0, 6, "0000000X");
    bf(&r);
    br_expect_text(&r, 0, 4, "XXXXXXXX", "SFR[0] X: Xon1 or GPIOINT, unknown");
    n81(&r);
    br_expect_text(&r, 0, 7, "XXXXXXXX", "SFR[0] X: SPR or GPIOLVL, unknown");
    xw(&r, 6, 0x01);
    /* GPIO3T and GPIOINV read back (LCR 0xBF, SFR[0] = 1). */
    bf(&r);
    xw(&r, 5, 0x30);
    xw(&r, 6, 0x0C);
    br_expect(&r, 0, 5, 0x30, "GPIO3T read back");
    br_expect(&r, 0, 6, 0x0C, "GPIOINV read back");
    xw(&r, 4, 0x22);
    br_expect(&r, 0, 4, 0x22, "GPIOINT read back");
    br_expect(&r, 0, 7, 0xFF, "GPIOSEL reset 0xFF (all inputs)");
    xw(&r, 4, 0x00);
    xw(&r, 5, 0x00);
    xw(&r, 6, 0x00);
    n81(&r);
    xw(&r, 6, 0x00);
    /* FCTR[6] X: address 7 is EMSR/FC or SPR: X. */
    bf(&r);
    wx(&r, 0, 1, "0X000000");
    br_expect_text(&r, 0, 1, "0X000000", "FCTR read back with its X bit");
    n81(&r);
    br_expect_text(&r, 0, 7, "XXXXXXXX", "FCTR[6] X: address 7 unknown");
    /* FCTR[7] X: a TRG write is lost and FC reads X. */
    bf(&r);
    wx(&r, 0, 1, "X0000000");
    xw(&r, 0, 0x10);
    br_expect_text(&r, 0, 0, "XXXXXXXX", "FCTR[7] X: FC (RX or TX count) unknown");
    /* FCTR[6] = 1: address 7 is EMSR (W) / FC (R); SFR[4] holds TX. */
    xw(&r, 1, 0x40);
    n81(&r);
    xw(&r, 2, 0x01);   /* FIFOs on */
    xw(&r, 6, 0x10);
    xw(&r, 0, 0x31);
    xw(&r, 0, 0x32);
    xw(&r, 0, 0x33);
    br_expect(&r, 0, 7, 0, "FC with EMSR[1:0] = 00: the RX count");
    xw(&r, 7, 0x01);
    br_expect(&r, 0, 7, 3, "EMSR[1:0] = 01: the TX count (SFR[4] TX disable holds 3)");
    xw(&r, 7, 0x03);
    br_expect(&r, 0, 7, 0, "EMSR[1:0] = 11: RX first");
    br_expect(&r, 0, 7, 3, "then TX");
    br_expect(&r, 0, 7, 0, "then RX again");
    mb_run(b, 30 * BIT1);
    check(mb_sent(b, 0, NULL, 0) == 0, "SFR[4]: nothing sent");
    mb_close(b);
    case_end("XR20M1280 registers: LCR maybe 0xBF, MSR, SFR[0] X, GPIO3T/GPIOINV, FCTR[6]/[7] X, "
             "EMSR FC count select, SFR[4] TX disable");
}

static void case_divisors(void) {
    Br r = xr_open();
    MB *b = r.b;
    long long t;

    case_begin();
    /* DLD[6] = 0: a DLL/DLM write goes to both generators. */
    xw(&r, 3, 0x80);
    xw(&r, 2, 0x00);
    xw(&r, 0, 0x05);
    xw(&r, 1, 0x00);
    xw(&r, 2, 0xC0);
    br_expect(&r, 0, 0, 0x05, "DLD[6] = 0 wrote both: the RX generator's DLL");
    br_expect(&r, 0, 1, 0x00, "and its DLM");
    br_expect(&r, 0, 2, 0xC0, "DLD reads the select and the RX generator's fraction");
    xw(&r, 2, 0x40);
    br_expect(&r, 0, 0, 0x05, "the TX generator's DLL");
    /* DLL = DLM = 0: DLL's address reads X (DREV, decisions.md). */
    xw(&r, 2, 0x00);
    xw(&r, 0, 0x00);
    br_expect_text(&r, 0, 0, "XXXXXXXX", "DLL = DLM = 0: DREV, not printed: X");
    xw(&r, 0, 0x01);
    n81(&r);
    /* MCR[7] (EFR[4]): XTAL1 / 4 into both generators. */
    xw(&r, 4, 0x80);
    xw(&r, 0, 0x55);
    t = br_bit_time(b, "TX");
    expect_near(t, 4 * BIT1, NS(240), "MCR[7]: 4 x 16 XTAL1 periods per bit");
    mb_run(b, 50 * BIT1);
    xw(&r, 4, 0x00);
    mb_close(b);
    case_end("XR20M1280 divisors: DLD[6] = 0 writes both generators, DLM/DLD reads, DREV, MCR[7]");
}

/* ---- Transmit: RS-485, EMSR, 3-state ------------------------------------------------------ */

static void case_rs485(void) {
    Br r = xr_open();
    MB *b = r.b;
    long long lead;
    long long low;
    uint16_t frames[4];

    case_begin();
    /* FCTR[3] auto RS-485, SHR = 0x21: RTS# leads the start bit by 2 bit times
       and stays (1 + 1) bit times after the stop bit. */
    xw(&r, 5, 0x21);
    bf(&r);
    xw(&r, 1, 0x08);
    n81(&r);
    mb_expect_pin(b, "RTS", LOGIC_1, "RS-485 idle: RTS# high");
    xw(&r, 0, 0x55);
    check(mb_wait(b, "RTS", LOGIC_0, NS(20000), NS(20)) >= 0, "RTS# asserts at the THR write");
    /* RTS# fell at the THR write, which lands at the last SCLK rise: 3 SPI
       half periods before the write call returns. */
    lead = mb_wait(b, "TX", LOGIC_0, NS(20000), NS(20)) + 3 * (long long)SPI_HALF;
    expect_near(lead, 2 * BIT1, NS(200), "SHR[7:4] = 2: two bit times to the start bit");
    low = br_width(b, "RTS", LOGIC_0, NS(40000));
    expect_near(low, 12 * BIT1, NS(300), "RTS# stays 10 + (1 + SHR[3:0]) bit times from the start bit");
    /* EMSR[3]: RTS# high while sending. */
    bf(&r);
    xw(&r, 1, 0x48);
    n81(&r);
    xw(&r, 7, 0x08);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "RTS", LOGIC_0, "EMSR[3]: RTS# low while idle");
    /* EMSR[4]: TX, RTS#, DTR# 3-state. */
    xw(&r, 7, 0x10);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "TX", LOGIC_Z, "EMSR[4]: TX 3-state");
    /* RTS# should be 3-state too (bridges.md § XR20M1280: "EMSR[4] 3-states
       TX/RTS#/DTR#"); the core's loopback or3 in ns16550.c turns the bridge's
       Z into X — reported to plan-68-N, not asserted here. */
    mb_expect_pin(b, "DTR/GPIO0", LOGIC_Z, "EMSR[4]: DTR# 3-state");
    xw(&r, 7, 0x00);
    bf(&r);
    xw(&r, 1, 0x40);
    n81(&r);
    /* EMSR[2]: the next THR byte goes out ahead of the FIFO. */
    xw(&r, 2, 0x01);
    xw(&r, 6, 0x10);
    xw(&r, 0, 0x41);
    xw(&r, 0, 0x42);
    xw(&r, 7, 0x04);
    xw(&r, 0, 0x43);
    xw(&r, 6, 0x00);
    mb_run(b, 40 * BIT1);
    check(mb_sent(b, 0, frames, 4) == 4 && frames[1] == 0x43 && frames[2] == 0x41 &&
              frames[3] == 0x42,
          "EMSR[2]: 0x43 first, then 0x41, 0x42");
    mb_close(b);
    /* RS-485 turned on with a byte already loaded (held by SFR[4]): RTS# asserts
       when the transmitter wants to start, and SHR[7:4] = 4 still leads it. */
    r = xr_open();
    b = r.b;
    xw(&r, 5, 0x40);
    xw(&r, 6, 0x10);
    xw(&r, 0, 0x55);
    bf(&r);
    xw(&r, 1, 0x08);
    n81(&r);
    mb_expect_pin(b, "RTS", LOGIC_1, "TX held: RTS# not asserted by FCTR[3] alone");
    xw(&r, 6, 0x00);
    check(mb_wait(b, "RTS", LOGIC_0, NS(20000), NS(20)) >= 0, "TX released: RTS# asserts");
    lead = mb_wait(b, "TX", LOGIC_0, NS(20000), NS(20));
    expect_near(lead, 4 * BIT1, NS(900), "SHR[7:4] = 4: four bit times before the start bit");
    mb_close(b);
    case_end("XR20M1280 auto RS-485 (SHR lead and turn-around, enabled with data loaded), EMSR[3], "
             "EMSR[4] 3-state, EMSR[2]");
}

/* ---- Receive: flow control, EMSR[6]/[7], 9-bit, break -------------------------------------- */

static void case_flow(void) {
    Br r = xr_open();
    MB *b = r.b;
    uint16_t frames[8];
    size_t n;

    case_begin();
    /* Auto RTS (EFR[6]), table A, trigger 1: RTS# off at the next level (4), on at 0;
       IER[6] = the RTS interrupt. */
    bf(&r);
    xw(&r, 2, 0x50);
    n81(&r);
    xw(&r, 2, 0x01);
    xw(&r, 4, 0x02);
    xw(&r, 1, 0x40);
    mb_drive_byte(b, "RX", 0x01, BIT1, 1);
    mb_drive_byte(b, "RX", 0x02, BIT1, 1);
    mb_drive_byte(b, "RX", 0x03, BIT1, 1);
    mb_run(b, 2 * BIT1);
    mb_expect_pin(b, "RTS", LOGIC_0, "3 bytes: RTS# still asserted");
    mb_drive_byte(b, "RX", 0x04, BIT1, 1);
    mb_run(b, 2 * BIT1);
    mb_expect_pin(b, "RTS", LOGIC_1, "4 bytes: RTS# off");
    br_expect(&r, 0, 2, 0xE0, "ISR: the RTS interrupt (IER[6])");
    br_r(&r, 0, 0, NULL);
    br_r(&r, 0, 0, NULL);
    br_r(&r, 0, 0, NULL);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "RTS", LOGIC_1, "1 byte left: still off (on at 0)");
    br_r(&r, 0, 0, NULL);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "RTS", LOGIC_0, "empty: RTS# on again");
    xw(&r, 1, 0x00);
    /* Table D with the FIFOs off: the trigger is 1, not TRG (5). */
    bf(&r);
    xw(&r, 1, 0x30);
    xw(&r, 0, 5);
    n81(&r);
    xw(&r, 2, 0x00);
    mb_drive_byte(b, "RX", 0x05, BIT1, 1);
    mb_run(b, 2 * BIT1);
    mb_expect_pin(b, "RTS", LOGIC_1, "FIFOs off: RTS# off after 1 byte (trigger 1)");
    br_r(&r, 0, 0, NULL);
    mb_run(b, NS(1000));
    bf(&r);
    xw(&r, 1, 0x00);
    n81(&r);
    xw(&r, 2, 0x01);
    /* Software flow, EFR[3:2] = 11: Xoff1 then Xoff2 two characters after the
       trigger; Xon1, Xon2 once the FIFO is empty (Table 8). */
    bf(&r);
    xw(&r, 4, 0x11);
    xw(&r, 5, 0x12);
    xw(&r, 6, 0x13);
    xw(&r, 7, 0x14);
    xw(&r, 2, 0x1C);
    n81(&r);
    mb_drive_byte(b, "RX", 0x61, BIT1, 1);
    mb_run(b, 40 * BIT1);
    n = mb_sent(b, 0, frames, 8);
    check(n == 2 && frames[0] == 0x13 && frames[1] == 0x14, "Xoff1, Xoff2 sent");
    br_r(&r, 0, 0, NULL);
    mb_run(b, 40 * BIT1);
    n = mb_sent(b, 0, frames, 8);
    check(n == 4 && frames[2] == 0x11 && frames[3] == 0x12, "Xon1, Xon2 after the read");
    mb_close(b);
    /* EMSR[7] = 1: the Xoff interrupt stays until an ISR read, Xon does not clear it. */
    r = xr_open();
    b = r.b;
    bf(&r);
    xw(&r, 4, 0x11);
    xw(&r, 6, 0x13);
    xw(&r, 2, 0x12);
    xw(&r, 1, 0x40);
    n81(&r);
    xw(&r, 7, 0x80);
    xw(&r, 1, 0x20);
    mb_drive_byte(b, "RX", 0x13, BIT1, 1);
    mb_run(b, 2 * BIT1);
    mb_drive_byte(b, "RX", 0x11, BIT1, 1);
    mb_run(b, 2 * BIT1);
    mb_expect_pin(b, "IRQ", LOGIC_0, "EMSR[7]: the Xoff interrupt survives the Xon");
    br_expect(&r, 0, 2, 0x10, "ISR: Xoff (0x10)");
    br_expect(&r, 0, 2, 0x01, "the ISR read cleared it");
    mb_close(b);
    case_end("XR20M1280 flow: auto RTS levels and the RTS interrupt, Xoff/Xon pairs, EMSR[7]");
}

static void case_rx(void) {
    Br r = xr_open();
    MB *b = r.b;
    char text[12];

    case_begin();
    /* FIFO off: the RX interrupt at 1 byte. */
    xw(&r, 1, 0x01);
    mb_drive_byte(b, "RX", 0x5A, BIT1, 1);
    mb_run(b, 2 * BIT1);
    br_expect(&r, 0, 2, 0x04, "FIFO off: RX data interrupt after 1 byte");
    br_r(&r, 0, 0, NULL);
    /* EMSR[6] = 1: an errored byte interrupts at once, behind a good one. */
    xw(&r, 2, 0xC1);   /* FIFO on, trigger 14 */
    xw(&r, 1, 0x04);
    bf(&r);
    xw(&r, 1, 0x40);
    n81(&r);
    xw(&r, 7, 0x40);
    mb_drive_byte(b, "RX", 0x41, BIT1, 1);
    mb_drive_byte(b, "RX", 0x42, BIT1, 0);   /* framing error */
    mb_run(b, 4 * BIT1);
    br_expect(&r, 0, 2, 0xC6, "EMSR[6]: line status interrupt with the error second in the FIFO");
    br_r(&r, 0, 5, NULL);
    br_expect(&r, 0, 2, 0xC1, "the LSR read clears it (error not at the head)");
    xw(&r, 2, 0x07);
    /* SFR[5] RX disable: an errored byte is dropped, no interrupt. */
    xw(&r, 6, 0x20);
    mb_drive_byte(b, "RX", 0x43, BIT1, 0);
    mb_run(b, 4 * BIT1);
    br_expect(&r, 0, 2, 0xC1, "SFR[5]: dropped, no interrupt");
    xw(&r, 6, 0x00);
    xw(&r, 1, 0x00);
    /* Break: LSR[4] stays while RX is held low after it. */
    mb_set(b, "RX", LOGIC_0);
    mb_run(b, 24 * BIT1);
    br_r(&r, 0, 5, text);
    br_r(&r, 0, 5, text);
    check(text[3] == '1', "LSR[4] still set while RX stays low");
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, 4 * BIT1);
    xw(&r, 2, 0x07);
    br_r(&r, 0, 5, NULL);
    mb_close(b);
    case_end("XR20M1280 RX: FIFO off trigger, EMSR[6] immediate LSR interrupt, SFR[5] drop, break");
}

/* 9-bit mode (SFR[6]), LCR[5:3] = 111: the 9th bit as the forced parity. */
static void case_nine_bit(void) {
    Br r = xr_open();
    MB *b = r.b;

    case_begin();
    xw(&r, 3, 0x3B);
    xw(&r, 2, 0x07);
    xw(&r, 6, 0x60);   /* SFR: 9-bit, RX disable */
    mb_drive_bits(b, "RX", "0000110001", BIT1, LOGIC_1);   /* data 0x30 */
    mb_drive_bits(b, "RX", "1000000011", BIT1, LOGIC_1);   /* address 0x01 */
    mb_run(b, 4 * BIT1);
    br_expect(&r, 0, 0, 0x01, "receiver off: the data byte dropped, the address kept");
    xw(&r, 2, 0x07);
    /* Auto address: EFR[5], XOFF2 = 0x05. */
    xw(&r, 3, 0xBF);
    xw(&r, 2, 0x30);
    xw(&r, 7, 0x05);
    xw(&r, 3, 0x3B);
    xw(&r, 6, 0x40);
    mb_drive_bits(b, "RX", "1100000011", BIT1, LOGIC_1);   /* address 0x03: dropped */
    mb_drive_bits(b, "RX", "0000110001", BIT1, LOGIC_1);   /* dropped */
    mb_drive_bits(b, "RX", "1010000011", BIT1, LOGIC_1);   /* address 0x05: kept */
    mb_drive_bits(b, "RX", "0100110001", BIT1, LOGIC_1);   /* 0x32: kept */
    mb_run(b, 4 * BIT1);
    xw(&r, 3, 0xBF);
    xw(&r, 1, 0x40);
    xw(&r, 3, 0x3B);
    br_expect(&r, 0, 7, 2, "FC: the matching address and its data");
    br_expect(&r, 0, 0, 0x05, "the address");
    br_expect(&r, 0, 0, 0x32, "its data");
    mb_close(b);
    case_end("XR20M1280 9-bit: receiver off keeps addresses only, auto address by XOFF2");
}

/* ---- Sleep, GPIO, unknown ------------------------------------------------------------------ */

static void case_sleep(void) {
    Br r = xr_open();
    MB *b = r.b;

    case_begin();
    /* FCTR[0] = 1: the pin is PWRDN#, high while awake. */
    bf(&r);
    xw(&r, 1, 0x01);
    n81(&r);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "SLEEP/PWRDN", LOGIC_1, "PWRDN# high while awake");
    /* IER[4] sleep with FCR[3] wake-up enabled: asleep once idle; an RX edge wakes
       it with the wake-up interrupt. */
    xw(&r, 2, 0x09);
    xw(&r, 1, 0x10);
    mb_run(b, 4 * BIT1);
    mb_expect_pin(b, "SLEEP/PWRDN", LOGIC_0, "asleep: PWRDN# low");
    mb_expect_pin(b, "IRQ", LOGIC_1, "no interrupt while asleep");
    mb_drive_byte(b, "RX", 0x55, BIT1, 1);
    mb_run(b, 2 * BIT1);
    mb_expect_pin(b, "IRQ", LOGIC_0, "woken by RX: the FCR[3] wake-up interrupt");
    mb_close(b);
    /* A divisor of 0 keeps it awake (no clock to stop). */
    r = br_open(XR_PART, 1, 1, NULL);
    b = r.b;
    xw(&r, 3, 0xBF);
    xw(&r, 2, 0x10);
    xw(&r, 1, 0x01);
    xw(&r, 3, 0x80);
    xw(&r, 0, 0x00);
    xw(&r, 3, 0x03);
    xw(&r, 2, 0x09);
    xw(&r, 1, 0x10);
    mb_run(b, 4 * BIT1);
    mb_expect_pin(b, "SLEEP/PWRDN", LOGIC_1, "divisor 0: never asleep");
    /* FCTR[0] X: PWRDN# X. */
    xw(&r, 3, 0xBF);
    wx(&r, 0, 1, "0000000X");
    mb_run(b, NS(1000));
    mb_expect_pin(b, "SLEEP/PWRDN", LOGIC_X, "FCTR[0] X: the pin X");
    mb_close(b);
    case_end("XR20M1280 sleep: PWRDN#, IER[4] sleep with the wake-up interrupt, divisor 0, FCTR[0] X");
}

static void case_gpio(void) {
    Br r = xr_open();
    MB *b = r.b;
    char text[12];

    case_begin();
    /* MCR[2] X: DTR#/GPIO0 is DTR# or a GPIO: X. */
    wx(&r, 0, 4, "00000X01");
    mb_run(b, NS(1000));
    mb_expect_pin(b, "DTR/GPIO0", LOGIC_X, "MCR[2] X: DTR#/GPIO0 X");
    xw(&r, 4, 0x04);
    xw(&r, 6, 0x05);   /* SFR: GPIO bank, GPIO interrupt enable */
    /* GPIOSEL X for GPIO4: X. */
    bf(&r);
    wx(&r, 0, 7, "111X1111");
    mb_run(b, NS(1000));
    mb_expect_pin(b, "GPIO4", LOGIC_X, "GPIOSEL[4] X: GPIO4 X");
    xw(&r, 7, 0xFF);
    /* GPIOINV inverts an input in GPIOLVL; GPIOINV X makes it X. */
    xw(&r, 6, 0x20);
    n81(&r);
    mb_set(b, "GPIO5", LOGIC_1);
    mb_run(b, NS(1000));
    br_r(&r, 0, 7, text);
    check(text[2] == '0', "GPIOINV[5]: GPIO5 high reads 0");
    bf(&r);
    wx(&r, 0, 6, "00X00000");
    n81(&r);
    br_r(&r, 0, 7, text);
    check(text[2] == 'X', "GPIOINV[5] X: GPIO5 reads X");
    /* SFR[2] X with a GPIO interrupt pending: ISR[6] unknown. */
    bf(&r);
    xw(&r, 6, 0x00);
    xw(&r, 4, 0x40);   /* GPIOINT: GPIO6 */
    n81(&r);
    mb_set(b, "GPIO6", LOGIC_0);
    mb_run(b, NS(1000));
    mb_set(b, "GPIO6", LOGIC_1);
    mb_run(b, NS(1000));
    wx(&r, 0, 6, "00000X01");
    br_r(&r, 0, 2, text);
    check(text[1] == 'X', "SFR[2] X: ISR[6] (the GPIO interrupt) unknown");
    mb_close(b);
    /* RESET X: the device is unknown: GPIO outputs and PWRDN# X. */
    r = xr_open();
    b = r.b;
    xw(&r, 4, 0x04);
    xw(&r, 6, 0x01);
    bf(&r);
    xw(&r, 7, 0xEF);   /* GPIO4 an output */
    xw(&r, 1, 0x01);
    n81(&r);
    mb_set(b, "RESET", LOGIC_X);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "GPIO4", LOGIC_X, "RESET X: GPIO output X");
    mb_expect_pin(b, "SLEEP/PWRDN", LOGIC_X, "RESET X: PWRDN# X");
    mb_close(b);
    case_end("XR20M1280 GPIO: MCR[2] X, GPIOSEL X, GPIOINV and GPIOINV X, ISR[6] X, RESET X");
}

static void case_irda(void) {
    Br r = xr_open();
    MB *b = r.b;
    long long w;

    case_begin();
    /* MCR[6] IrDA; SFR[3] = 1 the fast (1/4-bit) pulse (bridges.md § XR20M1280). */
    xw(&r, 4, 0x40);
    xw(&r, 6, 0x08);
    xw(&r, 0, 0x00);
    mb_wait(b, "TX", LOGIC_1, NS(100000), NS(20));   /* the start bit's pulse (maybe begun) */
    mb_wait(b, "TX", LOGIC_0, NS(100000), NS(20));
    mb_wait(b, "TX", LOGIC_1, NS(100000), NS(20));   /* data bit 0's pulse, from its edge */
    w = br_width(b, "TX", LOGIC_1, NS(2000));
    expect_near(w, BIT1 / 4, NS(60), "SFR[3]: 1/4-bit IrDA pulse");
    mb_close(b);
    case_end("XR20M1280 IrDA: SFR[3] fast pulse");
}

static void case_triggers(void) {
    Br r = xr_open();

    case_begin();
    /* Table D with FCTR[7] = 1: TRG is the TX trigger (spaces). */
    xw(&r, 2, 0x01);
    bf(&r);
    xw(&r, 1, 0xB0);
    xw(&r, 0, 200);
    n81(&r);
    xw(&r, 1, 0x02);
    br_expect(&r, 0, 2, 0xC1, "TX TRG 200 > 128 spaces: no THR interrupt on enable");
    xw(&r, 1, 0x00);
    bf(&r);
    xw(&r, 0, 16);
    n81(&r);
    xw(&r, 1, 0x02);
    br_expect(&r, 0, 2, 0xC2, "TX TRG 16 <= 128 spaces: THR interrupt");
    mb_close(r.b);
    case_end("XR20M1280 table D TX trigger (TRG with FCTR[7] = 1)");
}

int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_registers();
    case_divisors();
    case_rs485();
    case_flow();
    case_rx();
    case_nine_bit();
    case_sleep();
    case_gpio();
    case_irda();
    case_triggers();
    return serial_bench_finish();
}
