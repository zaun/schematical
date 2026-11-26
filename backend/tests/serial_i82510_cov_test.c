/*
 * serial_i82510_cov_test: the 82510 paths serial_intel_test does not reach
 * (plan-68-P): the clock straps (no /2, crystal sleep), RESET X, the
 * multifunction pins (TA, TB, ICLK, OUT1, SCLK), break, the automatic
 * handshake, the bank 1-3 writes (MCR, RCM, TCM, FMD, ACR1, RIE, MIE), 1x
 * clocks from SCLK, BRGs clocked by SCLK, EBCDIC, software parity and
 * parity errors received, framing errors, uLAN addresses and FIFO locking,
 * echo suppression, the start-bit check and the DPLL; and the loader's
 * rejections. Expected values: plan-12-data/specs/classic-b.md § 82510
 * (82510.pdf).
 */

#include "serial_bench.h"
#include "serial_bench_classic.h"

#include "cJSON.h"

static const char *const I5_D[8] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"};

/* CLK 18.432 MHz (external clock, /2: system clock 9.216 MHz). */
#define I5_HALF ((SimTime)27127)
#define I5_SYS (4 * I5_HALF)
/* Wake-up path: system clock / 2 / 5 = 16x: 160 system clocks a bit. */
#define I5_BT (160 * I5_SYS)
#define SCLK_HALF NS(2000)
#define SCLK_BT (2 * SCLK_HALF)

static void i5_addr(Bench *b, unsigned a) {
    set(b, "A0", a & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "A1", a & 2u ? LOGIC_1 : LOGIC_0);
    set(b, "A2", a & 4u ? LOGIC_1 : LOGIC_0);
}

static void i5_write(Bench *b, unsigned a, unsigned v) {
    i5_addr(b, a);
    h_set_bus(b, I5_D, 8, 1, v);
    set(b, "CS", LOGIC_0);
    run(b, NS(20));
    set(b, "WR", LOGIC_0);
    run(b, NS(300));
    set(b, "WR", LOGIC_1);
    run(b, NS(20));
    set(b, "CS", LOGIC_1);
    h_set_bus(b, I5_D, 8, 0, 0);
    run(b, NS(300));
}

static unsigned i5_read(Bench *b, unsigned a, char *text) {
    unsigned v;

    i5_addr(b, a);
    set(b, "CS", LOGIC_0);
    run(b, NS(20));
    set(b, "RD", LOGIC_0);
    run(b, NS(320));
    v = h_get_bus(b, I5_D, 8, text);
    set(b, "RD", LOGIC_1);
    run(b, NS(20));
    set(b, "CS", LOGIC_1);
    run(b, NS(300));
    return v;
}

static void i5_expect(Bench *b, unsigned a, const char *want, const char *what) {
    char text[16];

    i5_read(b, a, text);
    h_expect_text(text, want, what);
}

/* Selects a bank, then writes register a of it. */
static void i5_set(Bench *b, unsigned bank, unsigned a, unsigned v) {
    i5_write(b, 2, bank << 5);
    i5_write(b, a, v);
}

/* Selects a bank, then reads register a of it as text. */
static void i5_check(Bench *b, unsigned bank, unsigned a, const char *want, const char *what) {
    i5_write(b, 2, bank << 5);
    i5_expect(b, a, want, what);
}

/* The 82510 with RTS and DTR at `rts`/`dtr` as RESET falls (the straps; Z:
   the weak pull-ups), CLK running; `sclk`: RI/SCLK gets a free-running clock. */
static HBench *i5_open_strap(LogicValue rts, LogicValue dtr, int sclk) {
    static const HClockPin CLOCKS[2] = {{"CLK/X1", 0}, {"RI/SCLK", 1}};
    SimTime halves[2] = {I5_HALF, SCLK_HALF};
    HBench *h = hb_open("SFX-82510-plcc28", CLOCKS, sclk ? 2 : 1, halves, sclk ? 2 : 1);
    Bench *b = &h->b;

    set(b, "RESET", LOGIC_1);
    set(b, "CS", LOGIC_1);
    set(b, "RD", LOGIC_1);
    set(b, "WR", LOGIC_1);
    set(b, "RXD", LOGIC_1);
    set(b, "CTS", LOGIC_1);
    if (!sclk) {
        set(b, "RI/SCLK", LOGIC_1);
    }
    set(b, "DSR/TA/OUT0", LOGIC_1);
    set(b, "DCD/ICLK/OUT1", LOGIC_1);
    i5_addr(b, 0);
    run(b, NS(1000));   /* in reset: RTS and DTR are inputs (weak pull-ups) */
    set(b, "RTS", rts);
    set(b, "DTR/TB", dtr);
    run(b, NS(2000));
    /* RESET falls with the straps still driven; they are released at once
       (open collector from RESET: hold 0..Tcy-20, p38). */
    set(b, "RESET", LOGIC_0);
    set(b, "RTS", LOGIC_Z);
    set(b, "DTR/TB", LOGIC_Z);
    run(b, NS(1000));
    return h;
}

/* Drives a received frame (8 data bits, no parity, `stop`) at the wake-up rate. */
static void i5_rx(Bench *b, unsigned v, unsigned n, int stop) {
    char bits[16];

    h_frame_bits(v, n, 0, stop ? 1 : 0, bits);
    if (!stop) {
        strcat(bits, "0");
    }
    h_drive_bits(b, "RXD", bits, I5_BT);
    run(b, I5_BT);
}

/* Sees both levels on `pin` while the clocks run. */
static int toggles(Bench *b, const char *pin) {
    int seen0 = 0;
    int seen1 = 0;
    unsigned i;

    for (i = 0; i < 60; i++) {
        run(b, I5_SYS / 3);
        seen0 |= get(b, pin) == LOGIC_0;
        seen1 |= get(b, pin) == LOGIC_1;
    }
    return seen0 && seen1;
}

/* ---- Straps and RESET ---------------------------------------------------------------------- */

static void case_straps(void) {
    HBench *h = i5_open_strap(LOGIC_Z, LOGIC_0, 0);
    Bench *b = &h->b;
    char got[32];
    char want[32];

    case_begin();
    /* DTR low as RESET falls (external clock): no /2, system clock = CLK (p8). */
    i5_set(b, 0, 3, 0x03);
    i5_write(b, 0, 'A');
    check(h_catch(b, "TXD", I5_BT / 2, 9, 4 * I5_BT, got), "no /2: frame");
    h_frame_bits('A', 8, 0, 1, want);
    h_expect_text(got, want, "DTR strap low: the bit time halves (system clock = CLK)");
    run(b, I5_BT);
    /* RESET X: the device is unknown; back to 0 without a reset: registers unknown. */
    set(b, "RESET", LOGIC_X);
    run(b, NS(500));
    expect_pin(b, "TXD", LOGIC_X, "RESET X: TXD X");
    expect_pin(b, "INT", LOGIC_X, "RESET X: INT X");
    /* A read cut short by RESET going X releases the data bus. */
    i5_addr(b, 3);
    set(b, "CS", LOGIC_0);
    set(b, "RD", LOGIC_0);
    run(b, NS(400));
    h_expect_bus(b, I5_D, 8, "ZZZZZZZZ", "RESET X: no read cycle (bus disabled)");
    set(b, "RD", LOGIC_1);
    set(b, "CS", LOGIC_1);
    run(b, NS(300));
    set(b, "RESET", LOGIC_0);
    run(b, NS(500));
    i5_expect(b, 3, "XXXXXXXX", "RESET X then 0: LCR unknown (never reset)");
    case_end("82510 straps: DTR low disables /2; RESET X makes the device and its registers unknown");
    hb_close(h);

    h = i5_open_strap(LOGIC_0, LOGIC_Z, 0);
    b = &h->b;
    case_begin();
    /* RTS low as RESET falls: crystal mode; power down there is sleep (p13). */
    i5_set(b, 1, 7, 0x02);
    i5_expect(b, 3, "XXXXXXXX", "sleep: registers read invalid");
    i5_write(b, 2, 0x40);    /* a BANK write in sleep is ignored */
    /* The registers come back unknown (the model keeps value 0 for them,
       PMD 00H: the multifunction pins as outputs), so the bench lets go. */
    set(b, "DSR/TA/OUT0", LOGIC_Z);
    set(b, "DCD/ICLK/OUT1", LOGIC_Z);
    i5_expect(b, 4, "XXXXXXXX", "reading FLR leaves sleep (its data invalid)");
    i5_check(b, 2, 1, "XXXXXXXX", "after sleep: FMD lost (registers not preserved)");
    i5_check(b, 2, 4, "XXXXXXXX", "after sleep: IMD lost");
    case_end("82510 crystal strap: sleep loses the registers; FLR read exits");
    hb_close(h);
}

/* ---- Pins, break, handshake, command registers ----------------------------------------------- */

static void case_pins(void) {
    HBench *h = i5_open_strap(LOGIC_Z, LOGIC_Z, 0);
    Bench *b = &h->b;
    char got[32];

    case_begin();
    i5_check(b, 1, 0, "XXXXXXXX", "bank 1 RXD with the RX FIFO empty: X");
    i5_check(b, 1, 1, "XXXXXXXX", "bank 1 RXF with the RX FIFO empty: X");
    /* PMD 00H: DTR/TB is TB, DSR/TA/OUT0 is TA, DCD/ICLK/OUT1 is ICLK (Table 1). */
    set(b, "DSR/TA/OUT0", LOGIC_Z);
    set(b, "DCD/ICLK/OUT1", LOGIC_Z);
    i5_set(b, 3, 4, 0x00);
    check(toggles(b, "DTR/TB"), "PMD DTF = 0: TB (BRGB output) on DTR/TB");
    check(toggles(b, "DSR/TA/OUT0"), "PMD DTAD = DTAF = 0: TA (BRGA output) on DSR/TA/OUT0");
    check(toggles(b, "DCD/ICLK/OUT1"), "PMD DIOD = DIOF = 0: ICLK on DCD/ICLK/OUT1");
    /* PMD 40H: DCD/ICLK/OUT1 is OUT1 (MCR bit 2, 1 = pin low), here through bank 1's MCR. */
    i5_set(b, 3, 4, 0x40);
    i5_set(b, 1, 4, 0x04);
    run(b, NS(300));
    expect_pin(b, "DCD/ICLK/OUT1", LOGIC_0, "OUT1 from MCR bit 2 (bank 1 A4 writes MCR)");
    i5_set(b, 0, 4, 0x14);
    run(b, NS(300));
    expect_pin(b, "DCD/ICLK/OUT1", LOGIC_1, "local loopback: OUT1 pin inactive");
    i5_set(b, 0, 4, 0x00);
    run(b, NS(300));
    expect_pin(b, "DCD/ICLK/OUT1", LOGIC_1, "MCR bit 2 = 0: OUT1 high");
    i5_set(b, 3, 4, 0xFC);
    set(b, "DSR/TA/OUT0", LOGIC_1);
    set(b, "DCD/ICLK/OUT1", LOGIC_1);
    /* LCR SBK: TXD low regardless; cleared, TXD mark. */
    i5_set(b, 0, 3, 0x43);
    run(b, NS(400));
    expect_pin(b, "TXD", LOGIC_0, "SBK: TXD low");
    i5_write(b, 3, 0x03);
    run(b, NS(400));
    expect_pin(b, "TXD", LOGIC_1, "SBK cleared: TXD mark");
    /* Automatic handshake (TMD TM = 11): RTS only while enabled with data. */
    set(b, "CTS", LOGIC_0);
    i5_set(b, 2, 3, 0x18);
    run(b, NS(500));
    expect_pin(b, "RTS", LOGIC_1, "automatic: RTS inactive with nothing to send");
    i5_set(b, 0, 0, 0x55);
    run(b, I5_BT);
    expect_pin(b, "RTS", LOGIC_0, "automatic: RTS active while sending");
    run(b, 12 * I5_BT);
    expect_pin(b, "RTS", LOGIC_1, "automatic: RTS inactive once FIFO and shift register are empty");
    i5_set(b, 2, 3, 0x00);
    /* TCM: TxDI holds the FIFO; FTF flushes it; TxEN sends; FTM aborts a character. */
    i5_set(b, 1, 6, 0x01);
    i5_write(b, 0, 0x11);
    i5_write(b, 0, 0x22);
    check(!h_catch(b, "TXD", I5_BT, 9, 3 * I5_BT, got), "TCM TxDI: nothing sent");
    i5_expect(b, 4, "00000010", "FLR: two in the TX FIFO");
    i5_write(b, 6, 0x04);
    i5_expect(b, 4, "00000000", "TCM FTF: TX FIFO flushed");
    i5_write(b, 0, 0x33);
    i5_write(b, 6, 0x02);
    check(h_catch(b, "TXD", I5_BT, 9, 3 * I5_BT, got), "TCM TxEN: the character goes");
    h_expect_text(got, "0110011001", "TxEN: 0x33");
    run(b, 2 * I5_BT);
    i5_write(b, 0, 0x00);
    check(h_wait(b, "TXD", LOGIC_0, 3 * I5_BT), "0x00 starts");
    run(b, 2 * I5_BT);
    i5_write(b, 6, 0x08);
    run(b, NS(400));
    expect_pin(b, "TXD", LOGIC_1, "TCM FTM: the TX machine is reset (TXD mark)");
    check(!h_wait(b, "TXD", LOGIC_0, 8 * I5_BT), "FTM: the rest of the character is not sent");
    /* RCM (the RX FIFO is one deep at wake-up). */
    i5_set(b, 1, 5, 0x40);
    i5_rx(b, 0x41, 8, 1);
    i5_expect(b, 4, "00000000", "RCM RxDI: nothing received");
    i5_write(b, 5, 0x80);
    i5_rx(b, 0x42, 8, 1);
    i5_expect(b, 4, "00010000", "RCM RxE: received");
    i5_write(b, 5, 0x10);
    i5_expect(b, 4, "00000000", "RCM FRF: RX FIFO flushed");
    i5_write(b, 5, 0x08);
    i5_rx(b, 0x43, 8, 0);
    i5_expect(b, 4, "00000000", "RCM LRF: the character is discarded");
    i5_expect(b, 5, "00001000", "LRF: its status (FE) is still reported");
    i5_write(b, 5, 0x04);
    i5_rx(b, 0x44, 8, 1);
    i5_expect(b, 4, "00010000", "RCM ORF: received again");
    i5_expect(b, 0, "01000100", "ORF: the character");
    i5_write(b, 5, 0x40);
    i5_write(b, 5, 0x08);
    i5_write(b, 5, 0x20);
    i5_rx(b, 0x45, 8, 1);
    i5_expect(b, 4, "00010000", "RCM FRM: reception enabled and the FIFO unlocked");
    i5_read(b, 0, NULL);
    /* Banks 2 and 3: the writable bits. */
    i5_set(b, 2, 1, 0xFF);
    i5_expect(b, 1, "00110011", "FMD keeps bits 5:4, 1:0");
    i5_write(b, 1, 0x00);
    i5_write(b, 5, 0x5A);
    i5_expect(b, 5, "01011010", "ACR1");
    i5_write(b, 6, 0xFF);
    i5_expect(b, 6, "11111110", "RIE bit 0 reserved");
    i5_write(b, 6, 0x1E);
    i5_write(b, 0, 0xFF);
    i5_expect(b, 0, "XXXXXXXX", "bank 2 A0 write ignored: still no register");
    i5_set(b, 3, 5, 0xFF);
    i5_expect(b, 5, "00001111", "MIE keeps bits 3:0");
    i5_write(b, 7, 0xFF);
    i5_expect(b, 7, "XXXXXXXX", "bank 3 A7 write ignored: still no register");
    case_end("82510 pins: TA/TB/ICLK/OUT1, SBK, automatic RTS, TCM, RCM, bank 2/3 writes");
    hb_close(h);
}

/* ---- SCLK: 1x clocks and BRG sources ----------------------------------------------------------- */

static void sclk_align(Bench *b) {
    while (get(b, "RI/SCLK") != LOGIC_1) run(b, NS(50));
    while (get(b, "RI/SCLK") != LOGIC_0) run(b, NS(50));
}

static void case_sclk(void) {
    HBench *h = i5_open_strap(LOGIC_Z, LOGIC_Z, 1);
    Bench *b = &h->b;
    char got[32];
    char want[32];
    char bits[16];
    SimTime t0;
    SimTime d;
    int seen_x = 0;
    unsigned i;

    case_begin();
    i5_set(b, 3, 4, 0xF4);              /* PMD RRF = 0: the pin is SCLK */
    i5_write(b, 0, 0x20);               /* CLCF TxCM: 1x TX clock from SCLK */
    /* 1x stops (Table 13, 1x column): 000 not defined. */
    i5_set(b, 0, 3, 0x03);
    i5_write(b, 0, 0x0F);
    for (i = 0; i < 40; i++) {
        run(b, SCLK_BT / 2);
        seen_x |= get(b, "TXD") == LOGIC_X;
    }
    check(seen_x, "1x, stop code 000 (not defined): the frame is X");
    run(b, 12 * SCLK_BT);
    i5_set(b, 2, 3, 0x01);              /* TMD SBL1: code 010 = 1 stop in 1x */
    i5_set(b, 0, 0, 0x55);
    check(h_catch(b, "TXD", SCLK_BT, 9, 4 * SCLK_BT, got), "1x frame");
    h_expect_text(got, "0101010101", "1x TX from SCLK, 1 stop: 0x55");
    run(b, 4 * SCLK_BT);
    /* Code 111 (SBL2 SBL1 = 11, SBL0 = 1): 2 stops in 1x: start to start 11 bits. */
    i5_set(b, 2, 3, 0x03);
    i5_set(b, 0, 3, 0x07);
    i5_write(b, 0, 0xFF);
    i5_write(b, 0, 0xFF);
    check(h_wait(b, "TXD", LOGIC_0, 4 * SCLK_BT), "first start");
    t0 = sim_now(b->sim);
    run(b, 9 * SCLK_BT + SCLK_BT / 2);
    check(h_wait(b, "TXD", LOGIC_0, 3 * SCLK_BT), "second start");
    d = sim_now(b->sim) - t0;
    check(d + NS(200) >= 11 * SCLK_BT && d <= 11 * SCLK_BT + NS(200), "1x code 111: two stop bits");
    run(b, 12 * SCLK_BT);
    i5_set(b, 2, 3, 0x00);
    i5_set(b, 0, 3, 0x03);
    /* 1x RX: RXD sampled on SCLK rising (CLCF RxCM). */
    i5_set(b, 3, 0, 0x80);
    sclk_align(b);
    h_frame_bits(0x3C, 8, 0, 1, bits);
    h_drive_bits(b, "RXD", bits, SCLK_BT);
    run(b, 2 * SCLK_BT);
    i5_check(b, 0, 0, "00111100", "1x RX from SCLK: 0x3C");
    /* BRGA from SCLK (BACF BACS), TX 16x from BRGA: SCLK / 2 / 16. */
    i5_set(b, 3, 1, 0x44);
    i5_write(b, 0, 0x10);
    i5_set(b, 0, 0, 'A');
    check(h_catch(b, "TXD", 32 * SCLK_BT, 9, 40 * 32 * SCLK_BT, got), "BRGA from SCLK frame");
    h_frame_bits('A', 8, 0, 1, want);
    h_expect_text(got, want, "BRGA from SCLK: 32 SCLK periods a bit");
    run(b, 2 * 32 * SCLK_BT);
    /* BRGB from SCLK (BBCS = 01), TX from BRGB: SCLK / 5 / 16. */
    i5_set(b, 3, 3, 0x44);
    i5_write(b, 0, 0x00);
    i5_set(b, 0, 0, 'B');
    check(h_catch(b, "TXD", 80 * SCLK_BT, 9, 40 * 80 * SCLK_BT, got), "BRGB from SCLK frame");
    h_frame_bits('B', 8, 0, 1, want);
    h_expect_text(got, want, "BRGB from SCLK: 80 SCLK periods a bit");
    case_end("82510 SCLK: 1x TX and RX, 1x stop codes, BRGA and BRGB clocked by SCLK");
    hb_close(h);
}

/* ---- Receiver ---------------------------------------------------------------------------------- */

static void drive_frame_at(Bench *b, unsigned v, SimTime bt) {
    char bits[16];

    h_frame_bits(v, 8, 0, 1, bits);
    h_drive_bits(b, "RXD", bits, bt);
    run(b, 2 * I5_BT);
}

static void case_receiver(void) {
    HBench *h = i5_open_strap(LOGIC_Z, LOGIC_Z, 0);
    Bench *b = &h->b;
    char got[32];
    char want[32];
    char text[16];
    unsigned v;

    case_begin();
    i5_set(b, 0, 3, 0x03);
    /* EBCDIC control characters (RMD µCM = 11): 00H-3FH. */
    i5_set(b, 2, 7, 0xC0);
    i5_rx(b, 0x25, 8, 1);
    i5_check(b, 1, 1, "01001000", "EBCDIC 25H: RXF ROK, ACR");
    i5_expect(b, 5, "10000001", "EBCDIC 25H: RST CRF");
    i5_read(b, 0, NULL);
    i5_rx(b, 0x41, 8, 1);
    i5_expect(b, 1, "01000000", "EBCDIC: 41H is not a control character");
    i5_read(b, 0, NULL);
    i5_set(b, 2, 7, 0x00);
    /* Even parity (LCR 1BH): a wrong parity bit gives RPE and PE. */
    i5_set(b, 0, 3, 0x1B);
    i5_rx(b, 0x001, 9, 1);              /* 0x01 with parity 0: wrong for even */
    i5_check(b, 1, 1, "00010000", "parity error: RXF RPE, not ROK");
    i5_expect(b, 5, "00000101", "parity error: RST PE");
    i5_read(b, 0, NULL);
    /* Software parity (TMD SPF, LCR PM0 only): RPE is the received bit, ROK ignores it. */
    i5_set(b, 2, 3, 0x04);
    i5_set(b, 0, 3, 0x0B);
    i5_rx(b, 0x101, 9, 1);
    i5_check(b, 1, 1, "01010000", "software parity, bit 1: RXF ROK, RPE = 1");
    i5_read(b, 0, NULL);
    i5_rx(b, 0x001, 9, 1);
    i5_expect(b, 1, "01000000", "software parity, bit 0: RXF ROK, RPE = 0");
    i5_read(b, 0, NULL);
    i5_set(b, 2, 3, 0x00);
    i5_set(b, 0, 3, 0x03);
    /* A framing error (not a break): RFE and FE; the line held low after it
       is not a new start bit (a fresh 1 -> 0 is needed). */
    h_frame_bits(0x41, 8, 0, 0, want);
    strcat(want, "00");
    h_drive_bits(b, "RXD", want, I5_BT);
    run(b, 3 * I5_BT);
    i5_check(b, 1, 1, "00000010", "framing error: RXF RFE");
    i5_expect(b, 5, "00001001", "framing error: RST FE");
    i5_read(b, 0, NULL);
    run(b, 12 * I5_BT);
    i5_expect(b, 4, "00000000", "no character from the low after the framing error");
    /* Start check (RMD SSM, 7/16 window): a start bit with a mark glitch in
       its middle passes the majority vote but not "all samples 0". */
    {
        static const unsigned RMD[2] = {0x18, 0x10};
        static const char *const FLR[2] = {"00000000", "00010000"};
        static const char *const WHAT[2] = {"SSM = 1: a glitched start bit is a false start",
                                            "SSM = 0: the majority accepts it"};
        unsigned k;

        for (k = 0; k < 2; k++) {
            i5_set(b, 2, 7, RMD[k]);
            i5_write(b, 2, 0x20);
            set(b, "RXD", LOGIC_0);
            run(b, I5_BT * 8 / 16);
            set(b, "RXD", LOGIC_1);
            run(b, I5_BT * 2 / 16);
            set(b, "RXD", LOGIC_0);
            run(b, I5_BT * 6 / 16);
            set(b, "RXD", LOGIC_1);
            run(b, 12 * I5_BT);
            i5_expect(b, 4, FLR[k], WHAT[k]);
            i5_read(b, 0, NULL);
        }
    }
    /* The DPLL: a sender 7% slow is followed (edges nudge the bit counter);
       with RMD DPD the same frame is mis-sampled. */
    i5_set(b, 2, 7, 0x00);
    drive_frame_at(b, 0x55, I5_BT * 107 / 100);
    i5_check(b, 1, 1, "01000000", "DPLL on: 7% slow 55H received OK");
    i5_expect(b, 0, "01010101", "DPLL on: the character");
    i5_set(b, 2, 7, 0x20);
    drive_frame_at(b, 0x55, I5_BT * 107 / 100);
    i5_write(b, 2, 0x20);
    v = i5_read(b, 1, text);
    check((v & 0x40u) == 0 || i5_read(b, 0, NULL) != 0x55, "DPLL off (DPD): 7% slow 55H is not received OK");
    i5_read(b, 0, NULL);
    i5_set(b, 2, 7, 0x00);
    (void)got;
    case_end("82510 receiver: EBCDIC, parity and software parity, framing error, SSM, DPLL");
    hb_close(h);
}

/* ---- uLAN, FIFO lock, echo ---------------------------------------------------------------------- */

static void case_ulan(void) {
    HBench *h = i5_open_strap(LOGIC_Z, LOGIC_Z, 0);
    Bench *b = &h->b;
    char got[32];
    char want[32];

    case_begin();
    i5_set(b, 0, 3, 0x03);
    i5_set(b, 2, 4, 0x0A);              /* IMD: automatic ack, uLAN, 4-deep RX FIFO */
    /* TX: the address marker (TXF bit 7) is the bit after the 8 data bits. */
    i5_set(b, 1, 1, 0x80);
    i5_write(b, 0, 0x12);
    check(h_catch(b, "TXD", I5_BT, 10, 4 * I5_BT, got), "uLAN address frame");
    h_frame_bits(0x112, 9, 0, 1, want);
    h_expect_text(got, want, "uLAN TX: address marker 1 after D7");
    run(b, 2 * I5_BT);
    i5_write(b, 1, 0x00);
    i5_write(b, 0, 0x34);
    check(h_catch(b, "TXD", I5_BT, 10, 4 * I5_BT, got), "uLAN data frame");
    h_frame_bits(0x034, 9, 0, 1, want);
    h_expect_text(got, want, "uLAN TX: address marker 0 for data");
    run(b, 2 * I5_BT);
    /* RX, manual mode: an address character matching ACR0. */
    i5_set(b, 0, 7, 0x12);
    i5_rx(b, 0x112, 9, 1);
    i5_check(b, 1, 1, "01001000", "uLAN address: RXF ROK, ACR");
    i5_expect(b, 5, "11000001", "uLAN address = ACR0: RST CRF, PCRF");
    i5_expect(b, 0, "00010010", "uLAN address character in the FIFO (8 bits)");
    i5_rx(b, 0x055, 9, 1);
    i5_expect(b, 1, "01000000", "uLAN data character: no ACR");
    i5_read(b, 0, NULL);
    /* Automatic (RMD 40H): a mismatching address locks the FIFO, a match opens it. */
    i5_set(b, 2, 7, 0x40);
    i5_rx(b, 0x113, 9, 1);
    i5_rx(b, 0x066, 9, 1);
    i5_check(b, 1, 4, "00000000", "automatic: mismatch locks; neither character stored");
    i5_expect(b, 5, "10000000", "automatic: the mismatching address still sets CRF");
    i5_rx(b, 0x112, 9, 1);
    i5_rx(b, 0x077, 9, 1);
    i5_expect(b, 4, "00100000", "automatic: the matching address and its data stored");
    i5_write(b, 5, 0x10);
    /* Semi-automatic (RMD 80H): any address character opens a locked FIFO. */
    i5_set(b, 2, 7, 0x80);
    i5_set(b, 1, 5, 0x08);
    i5_rx(b, 0x199, 9, 1);
    i5_expect(b, 4, "00010000", "semi-automatic: any address opens the FIFO");
    i5_write(b, 5, 0x10);
    i5_set(b, 2, 7, 0x00);
    i5_write(b, 4, 0x0C);
    case_end("82510 uLAN: address marker sent, address recognition, automatic and semi-automatic locking");
    hb_close(h);
}

static void case_echo(void) {
    HBench *h = i5_open_strap(LOGIC_Z, LOGIC_Z, 0);
    Bench *b = &h->b;
    char got[32];
    char want[32];

    case_begin();
    i5_set(b, 0, 3, 0x03);
    i5_set(b, 2, 4, 0x09);              /* IMD LEM with MCR LC = 0: automatic echo */
    i5_write(b, 3, 0x80);               /* TMD EED */
    i5_rx(b, 0x41, 8, 0);
    check(!h_catch(b, "TXD", I5_BT, 9, 3 * I5_BT, got), "EED: a character with an error is not echoed");
    i5_write(b, 3, 0x40);               /* TMD CED, ASCII control set */
    i5_write(b, 7, 0x80);
    h_frame_bits(0x0D, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, I5_BT);
    check(!h_catch(b, "TXD", I5_BT, 9, 3 * I5_BT, got), "CED: a control character is not echoed");
    h_frame_bits('A', 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, I5_BT);
    check(h_catch(b, "TXD", I5_BT, 9, 4 * I5_BT, got), "CED: an ordinary character is echoed");
    h_expect_text(got, want, "echo of A");
    case_end("82510 echo: EED and CED suppress errored and control characters");
    hb_close(h);
}

/* ---- Loader rejections ------------------------------------------------------------------------- */

static int cov_counter;

static int apply_edit(cJSON *root, const char *path, const cJSON *value) {
    char buf[160];
    char *key;
    char *dot;
    cJSON *at = root;
    int del = cJSON_IsString(value) && strcmp(value->valuestring, "-") == 0;

    snprintf(buf, sizeof buf, "%s", path);
    key = buf;
    while ((dot = strchr(key, '.')) != NULL) {
        *dot = '\0';
        at = cJSON_GetObjectItemCaseSensitive(at, key);
        if (at == NULL) {
            return -1;
        }
        key = dot + 1;
    }
    if (del) {
        cJSON_DeleteItemFromObjectCaseSensitive(at, key);
        return 0;
    }
    if (cJSON_GetObjectItemCaseSensitive(at, key) != NULL) {
        return cJSON_ReplaceItemInObjectCaseSensitive(at, key, cJSON_Duplicate(value, 1)) ? 0 : -1;
    }
    cJSON_AddItemToObject(at, key, cJSON_Duplicate(value, 1));
    return 0;
}

/* Loads an edited copy of the 82510 fixture device; expects a refusal containing `want`. */
static void reject(const char *edits, const char *want) {
    cJSON *file = cJSON_Parse(SERIAL_FIXTURE_CLASSIC);
    cJSON *ed = cJSON_Parse(edits);
    cJSON *dev = NULL;
    cJSON *item;
    cJSON *out;
    char name[64];
    char what[600];
    char *json;
    char *error = NULL;
    int status;

    cJSON_ArrayForEach(item, cJSON_GetObjectItemCaseSensitive(file, "devices")) {
        if (strcmp(cJSON_GetObjectItemCaseSensitive(item, "id")->valuestring, "SFX-82510") == 0) {
            dev = cJSON_Duplicate(item, 1);
        }
    }
    if (dev == NULL || ed == NULL) {
        fprintf(stderr, "reject: no device or bad edits %s\n", edits);
        exit(2);
    }
    cJSON_ArrayForEach(item, ed) {
        if (apply_edit(dev, item->string, item) != 0) {
            fprintf(stderr, "reject: cannot apply %s\n", item->string);
            exit(2);
        }
    }
    snprintf(name, sizeof name, "SFX-82510-P%d", cov_counter);
    cJSON_ReplaceItemInObjectCaseSensitive(dev, "id", cJSON_CreateString(name));
    cJSON_ArrayForEach(item, cJSON_GetObjectItemCaseSensitive(dev, "parts")) {
        snprintf(name, sizeof name, "SFX-82510-P%d-plcc28", cov_counter);
        cJSON_ReplaceItemInObjectCaseSensitive(item, "name", cJSON_CreateString(name));
    }
    cov_counter++;
    out = cJSON_CreateObject();
    cJSON_AddStringToObject(out, "format", "schematical-parts");
    cJSON_AddNumberToObject(out, "version", 1);
    cJSON_AddStringToObject(out, "kind", "serial");
    cJSON_AddItemToArray(cJSON_AddArrayToObject(out, "devices"), dev);
    json = cJSON_PrintUnformatted(out);
    status = load_parts(library, json, "i82510_cov.json", &error);
    snprintf(what, sizeof what, "82510 %s: refused with \"%s\" (got: %s)", edits, want,
             status == 0 ? "loaded" : (error != NULL ? error : "(null)"));
    check(status != 0 && error != NULL && strstr(error, want) != NULL, what);
    free(error);
    cJSON_free(json);
    cJSON_Delete(out);
    cJSON_Delete(ed);
    cJSON_Delete(file);
}

static void case_loader(void) {
    case_begin();
    reject("{\"bus\":\"-\"}", "\"bus\"");
    reject("{\"bus.bogus\":\"X\"}", "bogus");
    reject("{\"bus.data\":[\"D0\"]}", "\"data\" needs exactly 8 pin names");
    reject("{\"bus.address\":[\"A0\"]}", "\"address\" needs exactly 3 pin names");
    reject("{\"bus.read\":\"-\"}", "\"read\"");
    reject("{\"clock\":\"-\"}", "\"clock\"");
    reject("{\"serial.tx\":\"-\"}", "\"tx\"");
    reject("{\"serial.bogus\":\"TXD\"}", "bogus");
    reject("{\"bus.read\":\"NOPE\"}", "NOPE");
    reject("{\"serial.dcd\":\"NOPE\"}", "NOPE");
    case_end("82510 loader: every rejection");
}

int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_straps();
    case_pins();
    case_sclk();
    case_receiver();
    case_ulan();
    case_echo();
    case_loader();
    return serial_bench_finish();
}
