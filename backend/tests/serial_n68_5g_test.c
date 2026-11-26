/*
 * serial_n68_5g_test (plan-68-N): the Exar 5G parts of the ns16550 model
 * (models/ns16550_exar*.c) on the shipped XR16V798 fixture: the channel map
 * (DLD, XCHAR, TXCNT/RXCNT, TXTRG/RXTRG, the MSR write), the global block
 * (INT0-3, the timer, 8XMODE, SLEEP, REGB), the A7/channel selects, auto
 * flow and RS-485 on DTR/DSR, IrDA echo suppression. Expectations from
 * planning/plan-12-data/specs/exar-d.md § XR16V798 (Tables 8-19).
 */

#include "serial_bench_exar.h"
#include "serial_bench_n68.h"

/* ---- XR16V798 (exar-d.md § XR16V798: the 5G map, global block, timer) ------------------ */

/* Channel 0 of the XR16V798 at 8N1, divisor 1 (the RST# value), FIFO on. */
static Bench *g_open(SimTime *bt) {
    Bench *b = bench_open_cfg("SFX-XR16V798", HALF_18432, &E_V798_BENCH);

    bus_write(b, 0x03, 0x03);
    bus_write(b, 0x09, 0x10);   /* EFR[4] */
    bus_write(b, 0x02, 0x01);
    *bt = bit_time(b, 1);
    return b;
}

/* DLD (LCR[7] = 1, 0010): b3-0 fraction, b7-4 read 0; with LCR[7] X the
   register at 0010 (DLD or ISR) is X. */
static void case_5g_dld(void) {
    SimTime bt;
    Bench *b = g_open(&bt);

    case_begin();
    bus_write(b, 0x03, 0x83);
    bus_write(b, 0x02, 0xFF);
    expect_reg(b, 0x02, "00001111", "DLD: b7-4 read 0");
    bus_write(b, 0x02, 0x00);
    n68_write_x(b, 0x03, 0x03, 0x80);
    expect_reg(b, 0x02, "XXXXXXXX", "LCR[7] X: 0010 is DLD or ISR, X");
    bus_write(b, 0x03, 0x03);
    case_end("G1 XR16V798: DLD read mask, LCR[7] X");
    bench_close(b);
}

/* XCHAR (R, bits self-clear on read): b0 Xoff received, b1 Xon received,
   b2 Xoff sent, b3 Xon sent. Xoff is sent 2 char times after the RX FIFO
   crosses the trigger (table A trigger 1), Xon below it. A TX mode turned
   off before the Xoff goes out sends nothing. */
static void case_5g_xchar(void) {
    SimTime bt;
    Bench *b = g_open(&bt);

    case_begin();
    bus_write(b, 0x0C, 0x13);   /* XOFF1 */
    bus_write(b, 0x0E, 0x11);   /* XON1 */
    bus_write(b, 0x09, 0x1A);   /* TX Xon1/Xoff1, RX compare Xon1/Xoff1 */
    drive_named(b, "RX0", 0x13, bt);
    run(b, bt);
    expect_reg(b, 0x0C, "00000001", "Xoff received: XCHAR b0");
    expect_reg(b, 0x0C, "00000000", "the read cleared it");
    drive_named(b, "RX0", 0x11, bt);
    run(b, bt);
    expect_reg(b, 0x0C, "00000010", "Xon received: XCHAR b1");
    drive_named(b, "RX0", 0x41, bt);
    run(b, bt * 30);
    check(n68_sent_has(b, 0, 0, 0x13), "1 received: Xoff1 sent");
    expect_reg(b, 0x0C, "00000100", "Xoff sent: XCHAR b2");
    bus_read(b, 0x00);
    run(b, bt * 30);
    check(n68_sent_has(b, 0, 0, 0x11), "emptied: Xon1 sent");
    expect_reg(b, 0x0C, "00001000", "Xon sent: XCHAR b3");
    drive_named(b, "RX0", 0x42, bt);
    bus_write(b, 0x09, 0x12);   /* TX mode off before the Xoff goes out (RX mode kept) */
    run(b, bt * 30);
    expect_reg(b, 0x0C, "00000000", "TX mode off in the delay: no Xoff sent");
    case_end("G2 XR16V798: XCHAR flow status bits, Xoff delay cancelled");
    bench_close(b);
}

/* MSR write b2 RX disable: idle, one more full character is received, then
   the receiver stops. */
static void case_5g_rx_disable(void) {
    SimTime bt;
    Bench *b = g_open(&bt);

    case_begin();
    bus_write(b, 0x06, 0x04);
    drive_named(b, "RX0", 0x31, bt);
    drive_named(b, "RX0", 0x32, bt);
    run(b, bt);
    expect_reg(b, 0x0B, "00000001", "RX disabled while idle: one character, then none");
    expect_reg(b, 0x00, "00110001", "the one let in");
    case_end("G3 XR16V798: RX disable lets one character in");
    bench_close(b);
}

/* IER b2: an errored character interrupts as it comes in (only LSR[7] shows
   while it is not at the top) and again when it reaches the top. */
static void case_5g_ls_now(void) {
    SimTime bt;
    Bench *b = g_open(&bt);

    case_begin();
    bus_write(b, 0x01, 0x04);
    drive_named(b, "RX0", 0x41, bt);
    drive_bits_on(b, "RX0", "0100001000", bt);   /* 0x42, stop bit 0 */
    set(b, "RX0", LOGIC_1);
    run(b, bt * 2);
    expect_reg(b, 0x02, "11000110", "an errored byte behind a good one: LS interrupt now");
    expect_reg(b, 0x05, "11100001", "LSR: only LSR[7] (the head is good)");
    expect_reg(b, 0x02, "11000001", "the LSR read cleared it");
    bus_read(b, 0x00);
    expect_reg(b, 0x02, "11000110", "the errored byte at the top: LS again");
    case_end("G4 XR16V798: the line-status interrupt as an errored byte arrives");
    bench_close(b);
}

/* The timer (Table 10 p24): N = 0x0002-0xFFFF valid (N < 2: the interrupt is X,
   decisions.md); commands 0010 interrupt off, 0100 re-triggerable, 0110 TMRCK
   clock, 1010 stop, 1011 reset; reserved codes do nothing. */
static void case_5g_timer(void) {
    SimTime bt;
    Bench *b = g_open(&bt);

    case_begin();
    bus_write(b, 0x86, 20);
    bus_write(b, 0x87, 0);
    expect_reg(b, 0x86, "00010100", "TIMERLSB reads back");
    expect_reg(b, 0x87, "00000000", "TIMERMSB reads back");
    bus_write(b, 0x84, 0x05);   /* oscillator */
    bus_write(b, 0x84, 0x04);   /* re-triggerable */
    bus_write(b, 0x84, 0x01);   /* interrupt on */
    bus_write(b, 0x84, 0x09);   /* start */
    run(b, b->period * 24);
    expect_pin(b, "INT", LOGIC_0, "20 clocks: timer interrupt");
    expect_reg(b, 0x84, "00000001", "TIMERCNTL 01; the read clears it");
    run(b, NS(300));
    expect_pin(b, "INT", LOGIC_Z, "cleared");
    run(b, b->period * 22);
    expect_pin(b, "INT", LOGIC_0, "re-triggerable: again 20 clocks later");
    bus_write(b, 0x84, 0x02);   /* interrupt off */
    bus_read(b, 0x84);
    run(b, b->period * 22);
    expect_pin(b, "INT", LOGIC_Z, "interrupt off: INT# released");
    expect_reg(b, 0x84, "00000001", "the time-out still shows in TIMERCNTL");
    bus_write(b, 0x84, 0x0A);   /* stop */
    bus_read(b, 0x84);
    run(b, b->period * 44);
    expect_reg(b, 0x84, "00000000", "stopped: no time-out");
    bus_write(b, 0x84, 0x06);   /* TMRCK */
    bus_write(b, 0x84, 0x01);
    bus_write(b, 0x84, 0x09);
    run(b, b->period * 44);
    expect_reg(b, 0x84, "00000000", "TMRCK clock, TMRCK still: no time-out");
    bus_write(b, 0x84, 0x00);   /* reserved */
    bus_write(b, 0x84, 0x0B);   /* reset: stopped, count reloaded */
    bus_write(b, 0x84, 0x05);
    run(b, b->period * 44);
    expect_reg(b, 0x84, "00000000", "reset stops the timer");
    bus_write(b, 0x86, 1);      /* N = 1: invalid */
    bus_write(b, 0x84, 0x09);
    run(b, b->period * 4);
    expect_reg(b, 0x80, "0000000X", "N = 1: the timer interrupt (INT0 b0) is X");
    expect_pin(b, "INT", LOGIC_X, "N = 1: INT# X");
    case_end("G5 XR16V798: timer commands and N < 2");
    bench_close(b);
}

/* The global block (Table 8): 8XMODE, SLEEP, REGB read back; 0x8F is not
   listed (X); INT0 is read-only; an X write takes nothing (P2). INT1-3 hold
   3-bit codes (Table 9): 001 RX line status, 010 timeout, 011 TX, 100 MSR. */
static void case_5g_globals(void) {
    SimTime bt;
    Bench *b = g_open(&bt);

    case_begin();
    bus_write(b, 0x88, 0x05);
    expect_reg(b, 0x88, "00000101", "8XMODE reads back");
    bus_write(b, 0x88, 0x00);
    bus_write(b, 0x8B, 0x80);
    expect_reg(b, 0x8B, "10000000", "SLEEP reads back");
    bus_write(b, 0x8B, 0x00);
    bus_write(b, 0x8E, 0x01);
    expect_reg(b, 0x8E, "00000001", "REGB reads back");
    bus_write(b, 0x8E, 0x00);
    expect_reg(b, 0x8F, "XXXXXXXX", "0x8F not listed: X");
    bus_write(b, 0x80, 0xFF);
    expect_reg(b, 0x80, "00000000", "INT0 is read-only");
    n68_write_x(b, 0x88, 0x01, 0x02);
    expect_reg(b, 0x88, "00000000", "an X write takes nothing");
    bus_write(b, 0x01, 0x02);   /* THR empty interrupt */
    run(b, NS(300));
    expect_reg(b, 0x81, "00000011", "TX ready: code 011");
    bus_write(b, 0x01, 0x08);
    set(b, "CTS0", LOGIC_0);
    run(b, NS(500));
    expect_reg(b, 0x81, "00000100", "MSR: code 100");
    bus_read(b, 0x06);
    bus_write(b, 0x01, 0x04);
    drive_bits_on(b, "RX0", "0100001000", bt);
    set(b, "RX0", LOGIC_1);
    run(b, bt * 2);
    expect_reg(b, 0x81, "00000001", "RX line status: code 001");
    bus_read(b, 0x05);
    bus_read(b, 0x00);
    bus_read(b, 0x05);
    bus_write(b, 0x02, 0x41);   /* trigger 4 */
    bus_write(b, 0x01, 0x01);
    drive_named(b, "RX0", 0x41, bt);
    run(b, bt * 60);
    expect_reg(b, 0x81, "00000010", "RX timeout (1 byte, trigger 4): code 010");
    bus_read(b, 0x00);
    n68_write_x(b, 0x01, 0x00, 0x02);   /* IER[1] X with THR empty */
    run(b, NS(300));
    expect_reg(b, 0x80, "0000000X", "IER X: INT0 for channel 0 is X");
    expect_reg(b, 0x81, "00000XXX", "and its INT1 code X");
    case_end("G6 XR16V798: global registers, INT0-3 codes");
    bench_close(b);
}

/* Selects: A7 X (global block or a channel) and A6..A4 X (which channel) make
   the access X. */
static void case_5g_select_x(void) {
    SimTime bt;
    Bench *b = g_open(&bt);

    case_begin();
    bus_write(b, 0x07, 0x5A);
    set(b, "A7", LOGIC_X);
    set_address(b, 0x07);
    run(b, NS(40));
    expect_reg(b, 0x07, "01011010", "A7 0: channel 0 SPR");
    set_address(b, 0x07);
    set(b, "A7", LOGIC_X);
    run(b, NS(40));
    set(b, b->cfg->read, LOGIC_0);
    run(b, NS(150));
    expect_pin(b, "D0", LOGIC_X, "A7 X: the read is X");
    set(b, b->cfg->read, LOGIC_1);
    run(b, NS(200));
    set_address(b, 0x07);
    set(b, "A5", LOGIC_X);
    run(b, NS(40));
    set(b, b->cfg->read, LOGIC_0);
    run(b, NS(150));
    expect_pin(b, "D0", LOGIC_X, "A5 X: which channel is X");
    set(b, b->cfg->read, LOGIC_1);
    run(b, NS(200));
    case_end("G7 XR16V798: A7 X and channel bits X");
    bench_close(b);
}

/* Auto flow on DTR/DSR (MCR[2] = 1): the CTS/DSR interrupt (IER b7) on DSR
   low -> high. IrDA (MCR[6]): the receiver is off while the transmitter
   sends (echo suppression). */
static void case_5g_dsr_irda(void) {
    SimTime bt;
    Bench *b = g_open(&bt);
    unsigned i;

    case_begin();
    bus_write(b, 0x09, 0x90);
    bus_write(b, 0x04, 0x04);
    bus_write(b, 0x01, 0x80);
    set(b, "DSR0", LOGIC_0);
    run(b, bt);
    bus_read(b, 0x06);
    set(b, "DSR0", LOGIC_1);
    run(b, bt);
    expect_reg(b, 0x02, "11100000", "DSR low -> high: ISR = CTS/DSR change");
    bus_read(b, 0x06);
    bus_write(b, 0x01, 0x00);
    bus_write(b, 0x09, 0x10);
    bus_write(b, 0x04, 0x40);   /* IrDA */
    set(b, "RX0", LOGIC_0);
    for (i = 0; i < 4; i++) {
        bus_write(b, 0x00, 0x00);
    }
    run(b, bt * 2);
    for (i = 0; i < 10; i++) {   /* light pulses that would be a frame of zeros */
        set(b, "RX0", LOGIC_1);
        run(b, bt * 3 / 16);
        set(b, "RX0", LOGIC_0);
        run(b, bt - bt * 3 / 16);
    }
    run(b, bt * 40);
    expect_reg(b, 0x0B, "00000000", "IrDA: nothing received while sending");
    case_end("G8 XR16V798: DSR interrupt, IrDA echo suppression");
    bench_close(b);
}

/* ---- More 5G registers and pins (exar-d.md § XR16V798) ----------------------------------- */

/* TXCNT (R) / TXTRG, RXTRG (W, table D, FCTR[7:6] = 11); 1101 and 1111 read
   as reserved (X); the MSR write needs EFR[4] (shaded bits, p42). */
static void case_5g_counts(void) {
    SimTime bt;
    Bench *b = g_open(&bt);
    unsigned i;

    case_begin();
    bus_write(b, 0x08, 0xC0);   /* FCTR: table D */
    bus_write(b, 0x0B, 3);      /* RXTRG 3 */
    bus_write(b, 0x0A, 2);      /* TXTRG 2 */
    bus_write(b, 0x01, 0x01);
    drive_named(b, "RX0", 0x31, bt);
    drive_named(b, "RX0", 0x32, bt);
    run(b, bt);
    expect_reg(b, 0x02, "11000001", "2 received, RXTRG 3: no RX data interrupt");
    drive_named(b, "RX0", 0x33, bt);
    run(b, bt);
    expect_reg(b, 0x02, "11000100", "3 received: RX data interrupt");
    for (i = 0; i < 5; i++) {
        bus_write(b, 0x00, 0x41);
    }
    expect_reg(b, 0x0A, "00000100", "TXCNT 4 (5 written, the first in the TSR)");
    expect_reg(b, 0x0D, "XXXXXXXX", "1101 reads reserved: X");
    expect_reg(b, 0x0F, "XXXXXXXX", "1111 reads reserved: X");
    run(b, bt * 60);
    bus_write(b, 0x09, 0x00);   /* EFR[4] off */
    bus_write(b, 0x06, 0x08);   /* TX disable: ignored */
    bus_write(b, 0x00, 0x55);
    check(wait_level(b, "TX0", LOGIC_0, bt * 4), "the MSR write without EFR[4] is ignored: TX goes");
    case_end("G9 XR16V798: TXCNT, TXTRG/RXTRG, reserved reads, MSR write gate");
    bench_close(b);
}

/* MCR[2] = 1 moves auto flow and RS-485 to DTR/DSR: auto DTR goes high at
   table A's next level (trigger 1: 4); RS-485 (FCTR[5]) drives DTR high
   while sending. */
static void case_5g_dtr(void) {
    SimTime bt;
    Bench *b = g_open(&bt);
    unsigned i;

    case_begin();
    bus_write(b, 0x09, 0x50);   /* EFR[4], auto RTS/DTR */
    bus_write(b, 0x04, 0x05);   /* DTR on, MCR[2] */
    for (i = 0; i < 4; i++) {
        drive_named(b, "RX0", 0x30 + i, bt);
    }
    run(b, bt);
    expect_pin(b, "DTR0", LOGIC_1, "4 received: auto DTR high");
    for (i = 0; i < 4; i++) {
        bus_read(b, 0x00);
    }
    bus_write(b, 0x09, 0x10);
    bus_write(b, 0x08, 0x20);   /* FCTR[5]: RS-485 */
    run(b, NS(500));
    expect_pin(b, "DTR0", LOGIC_0, "RS-485 on DTR, idle: low");
    bus_write(b, 0x00, 0x33);
    run(b, bt * 2);
    expect_pin(b, "DTR0", LOGIC_1, "sending: DTR high");
    case_end("G10 XR16V798: auto DTR and RS-485 on DTR (MCR[2] = 1)");
    bench_close(b);
}

/* Global reads of TIMER, REGA, RESET: 00 (Table 8). */
static void case_5g_reads(void) {
    SimTime bt;
    Bench *b = g_open(&bt);

    case_begin();
    expect_reg(b, 0x85, "00000000", "TIMER reads 00");
    expect_reg(b, 0x89, "00000000", "REGA reads 00");
    expect_reg(b, 0x8A, "00000000", "RESET reads 00");
    case_end("G11 XR16V798: TIMER, REGA, RESET read 00");
    bench_close(b);
}

/* A 5G part with an active-high chip select (a variant: "bus.selectHigh"):
   CS high selects. */
static const BenchConfig V798_HIGH_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"},
    {"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7"}, 8,
    {"CS=1", "16/68=1", "TMRCK=0", "ENIR=0", "RX0=1", "RX1=1", "RX2=1", "RX3=1", "RX4=1", "RX5=1",
     "RX6=1", "RX7=1", "CTS0=1", "CTS1=1", "CTS2=1", "CTS3=1", "CTS4=1", "CTS5=1", "CTS6=1",
     "CTS7=1", "DSR0=1", "DSR1=1", "DSR2=1", "DSR3=1", NULL},
    "RST", 0, "XTAL1", NULL, NULL, {{NULL}}, 0
};

static void case_5g_select_high(void) {
    Bench *b = bench_open_cfg(n68_part("SFX-XR16V798", "{\"bus.select\":\"-\",\"bus.selectHigh\":[\"CS\"]}"),
                              HALF_18432, &V798_HIGH_BENCH);

    case_begin();
    bus_write(b, 0x07, 0x3C);
    expect_reg(b, 0x07, "00111100", "CS high: channel 0 SPR");
    set(b, "CS", LOGIC_0);
    run(b, NS(50));
    expect_reg(b, 0x07, "ZZZZZZZZ", "CS low: not selected");
    case_end("G12 XR16V798 variant: an active-high chip select");
    bench_close(b);
}

int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_5g_dld();
    case_5g_xchar();
    case_5g_rx_disable();
    case_5g_ls_now();
    case_5g_timer();
    case_5g_globals();
    case_5g_select_x();
    case_5g_dsr_irda();
    case_5g_counts();
    case_5g_dtr();
    case_5g_reads();
    case_5g_select_high();
    return serial_bench_finish();
}
