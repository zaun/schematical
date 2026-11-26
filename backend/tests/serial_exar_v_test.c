/*
 * serial_exar_v_test: the Exar / MaxLinear options E16-E28
 * (split from serial_test.c by plan-68-G; the bench is support/serial_bench.c).
 */

#include "serial_bench.h"
#include "serial_bench_d.h"
#include "serial_bench_exar.h"

/* ---- E16-E21 XR16V2550/2551/2552/2650/2651/2652 ------------------------------------- */

static void e_dvid(Bench *b, const char *want) {
    bus_write(b, 3, 0x80);
    bus_write(b, 0, 0x00);
    bus_write(b, 1, 0x00);
    expect_reg(b, 1, want, "DVID");
    bus_write(b, 0, 0x01);
    bus_write(b, 3, 0x03);
}

/* IrDA with the MCR[2] RX polarity: idle-high, low pulses decode with MCR[2] = 1. */
static void e_irda_polarity(Bench *b, const char *tx, const char *rx) {
    SimTime bt = bit_time(b, 1);
    unsigned i;

    e_efr(b, 0x10);
    bus_write(b, 4, 0x44);                 /* IrDA, MCR[2]: active-high idle RX */
    run(b, NS(300));
    expect_pin(b, tx, LOGIC_0, "IrDA: TX idles low");
    /* 0x41 as inverted pulses: each 0 bit a low pulse of 3/16 bit, idle high. */
    for (i = 0; i < 10; i++) {
        int bit = i == 0 ? 0 : i == 9 ? 1 : (0x41 >> (i - 1)) & 1;

        set(b, rx, LOGIC_1);
        run(b, bt * 7 / 16);
        set(b, rx, bit ? LOGIC_1 : LOGIC_0);
        run(b, bt * 3 / 16);
        set(b, rx, LOGIC_1);
        run(b, bt * 6 / 16);
    }
    run(b, bt * 2);
    expect_reg(b, 0, "01000001", "inverted IrDA pulses decode with MCR[2] = 1");
    bus_write(b, 4, 0x00);
    e_efr(b, 0x00);
}

static void case_e16(void) {
    Bench *b = bench_open_cfg("SFX-XR16V2550", HALF_18432, &E_DUAL_QFN_BENCH);

    case_begin();
    expect_reg(b, 7, "11111111", "SPR 0xFF");
    e_dvid(b, "00000010");
    program(b, 1, 0x03);
    e_irda_polarity(b, "TXA", "RXA");
    case_end("E16 XR16V2550: exar650 + DLD, DVID 0x02, IrDA with the MCR[2] RX polarity");
    bench_close(b);
}

static void case_e19(void) {
    Bench *b = bench_open_cfg("SFX-XR16V2650-tqfp48", HALF_18432, &DUAL_CD_BENCH);
    unsigned i;

    case_begin();
    e_dvid(b, "00000110");
    program(b, 1, 0x03);
    /* TX trigger 00 = 16 (32-byte table): the interrupt as the count drops below 16. */
    e_efr(b, 0x10);
    bus_write(b, 2, 0x01);
    bus_write(b, 4, 0x08);
    bus_write(b, 1, 0x02);
    bus_read(b, 2);
    for (i = 0; i < 20; i++) {
        bus_write(b, 0, 0x30 + i);
    }
    run(b, bit_time(b, 1) * 10 * 2);
    expect_pin(b, "INTA", LOGIC_0, "17 bytes left: no TX interrupt");
    run(b, bit_time(b, 1) * 10 * 3);
    expect_pin(b, "INTA", LOGIC_1, "below 16: TX interrupt");
    case_end("E19 XR16V2650: 32-byte tables (TX trigger 16 by default), DVID 0x06");
    bench_close(b);
}

static void case_e17_e20(const char *part, const BenchConfig *cfg, const char *dvid,
                         const char *name) {
    Bench *b = bench_open_cfg(part, HALF_18432, cfg);

    case_begin();
    e_dvid(b, dvid);
    e_motorola_dual(b);
    pulse_reset(b);
    e_powersave(b, 1);
    case_end(name);
    bench_close(b);
}

static void case_e18_e21(const char *part, const char *dvid, const char *name) {
    Bench *b = bench_open_cfg(part, HALF_18432, &CHSEL_BENCH);
    SimTime t;

    case_begin();
    expect_pin(b, "INTA", LOGIC_0, "INT always driven");
    e_dvid(b, dvid);
    program(b, 1, 0x03);
    /* EFR[4] = 0: AFR at 2; EFR[4] = 1: DLD there. */
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x02);                 /* AFR[2:1] = 01: MF = BAUDOUT */
    expect_reg(b, 2, "00000010", "EFR[4] = 0: AFR");
    bus_write(b, 3, 0x03);
    run(b, NS(2000));
    t = sim_now(b->sim);
    wait_level(b, "MFA", LOGIC_0, NS(5000));
    wait_level(b, "MFA", LOGIC_1, NS(5000));
    check(sim_now(b->sim) - t < NS(3000), "AFR[2:1] = 01: MF is BAUDOUT (toggling)");
    e_efr(b, 0x10);
    bus_write(b, 3, 0x80);
    expect_reg(b, 2, "00000000", "EFR[4] = 1: DLD at 2");
    bus_write(b, 3, 0x03);
    e_efr(b, 0x00);
    /* Shared AFR[0]: set from channel B, a write to A reaches both. */
    select_unit(b, 1);
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x01);
    bus_write(b, 3, 0x03);
    select_unit(b, 0);
    bus_write(b, 7, 0x99);
    select_unit(b, 1);
    expect_reg(b, 7, "10011001", "AFR[0] (one bit for both): concurrent write");
    case_end(name);
    bench_close(b);
}

/* ---- E22 XR16V554, E23 XR16V564, E24 XR16V654: the quads ----------------------------- */

static void case_e22(void) {
    Bench *b = bench_open_cfg("SFX-XR16V554", HALF_18432, &E_QUAD_V_BENCH);
    SimTime bt;
    unsigned i;

    case_begin();
    bus_write(b, 3, 0x80);
    expect_reg(b, 0, "00000001", "DLL 0x01 at power-up");
    expect_reg(b, 5, "XXXXXXXX", "LSR with LCR[7] = 1: X (decisions.md)");
    bus_write(b, 3, 0x03);
    expect_pin(b, "INTA", LOGIC_Z, "INTSEL low, MCR[3] = 0: INT 3-state");
    set(b, "INTSEL", LOGIC_1);
    run(b, NS(300));
    expect_pin(b, "INTA", LOGIC_0, "INTSEL high: INT driven");
    set(b, "INTSEL", LOGIC_0);
    bt = bit_time(b, 1);
    for (i = 0; i < 4; i++) {
        program_unit(b, i, 1, 0x03);
        bus_write(b, 2, 0x09);             /* FIFO, DMA mode 1 */
    }
    select_unit(b, 2);
    bus_write(b, 2, 0x89);                 /* channel C: RX trigger 8 */
    for (i = 0; i < 7; i++) {
        drive_named(b, "RXC", 0x41, bt);
    }
    expect_pin(b, "RXRDY", LOGIC_1, "7 bytes, trigger 8: shared RXRDY inactive");
    drive_named(b, "RXC", 0x41, bt);
    expect_pin(b, "RXRDY", LOGIC_0, "8 bytes: channel C pulls the shared RXRDY (AND) low");
    /* Motorola: CS on CSA, A4:A3 on CSC:CSB. */
    set(b, "RESET", LOGIC_1);
    set(b, "16/68", LOGIC_0);
    set(b, "CSA", LOGIC_1);
    set(b, "CSB", LOGIC_1);
    set(b, "CSC", LOGIC_1);
    run(b, NS(500));
    m_write(b, "CSA", 7, 0xD3);            /* A4:A3 = 11: channel D */
    set(b, "16/68", LOGIC_1);
    set(b, "RESET", LOGIC_0);
    run(b, NS(500));
    select_unit(b, 3);
    expect_reg(b, 7, "11010011", "Motorola A4:A3 = 11 wrote channel D");
    case_end("E22 XR16V554: quad, INTSEL, shared AND RXRDY, LSR X with DLAB, Motorola A4:A3");
    bench_close(b);
}

/* Sleep needs IER[4] on all four channels. */
static void e_sleep_all(Bench *b, unsigned channels) {
    unsigned i;

    for (i = 0; i < channels; i++) {
        select_unit(b, i);
        e_efr(b, 0x10);
        bus_write(b, 1, 0x10);
    }
    run(b, NS(20000));
}

static void case_e23_e24(const char *part, const char *d_part, unsigned fifo, const char *name) {
    Bench *b = bench_open_cfg(part, HALF_18432, &E_QUAD_CLKSEL_BENCH);
    Bench *d = bench_open_cfg(d_part, HALF_18432, &E_QUAD_CLKSEL_BENCH);
    SimTime bt;
    SimTime t;
    unsigned i;

    case_begin();
    expect_pin(b, "INTA", LOGIC_Z, "IV part: INT 3-state");
    expect_pin(d, "INTA", LOGIC_0, "DIV part (intPin driven): INT driven");
    bench_close(d);
    bus_write(b, 3, 0x83);
    expect_reg(b, 7, "XXXXXXXX", "100-111 with LCR[7] = 1: X (decisions.md)");
    bus_write(b, 3, 0x03);
    /* CLKSEL low: ÷4 after reset. */
    set(b, "CLKSEL", LOGIC_0);
    pulse_reset(b);
    program(b, 1, 0x03);
    bus_write(b, 0, 0xFF);
    t = e_low_time(b, "TXA", NS(400000), NS(100));
    check(t >= b->period * 63 && t <= b->period * 65, "CLKSEL low: ÷4");
    run(b, NS(800000));
    set(b, "CLKSEL", LOGIC_1);
    pulse_reset(b);
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0x01);
    for (i = 0; i < fifo; i++) {
        drive_named(b, "RXA", 0x20 + (i & 0x3F), bt);
    }
    check((bus_read(b, 5) & 0x02) == 0, "the FIFO holds its size");
    drive_named(b, "RXA", 0x41, bt);
    check((bus_read(b, 5) & 0x02) != 0, "one more overruns");
    /* LSR writes: stored on the XR16V564 (OE, factory-test style), ignored on the V654. */
    bus_write(b, 5, 0x02);
    if (fifo == 64) {
        check((bus_read(b, 5) & 0x02) == 0, "XR16V654: LSR is read-only (statusWrite false)");
    } else {
        check((bus_read(b, 5) & 0x02) != 0, "XR16V564: an LSR write sets OE (writable)");
    }
    /* Sleep: IER[4] on three channels is not enough; on all four the clock stops. */
    pulse_reset(b);
    program(b, 1, 0x03);
    e_sleep_all(b, 3);
    select_unit(b, 0);
    bus_write(b, 0, 0x55);
    check(wait_level(b, "TXA", LOGIC_0, bit_time(b, 1) * 4), "IER[4] on three channels: a byte goes out");
    run(b, bit_time(b, 1) * 12);
    e_sleep_all(b, 4);
    select_unit(b, 3);
    bus_write(b, 0, 0x55);                 /* a THR write wakes the chip */
    check(wait_level(b, "TXD", LOGIC_0, bit_time(b, 1) * 4), "a THR write wakes it");
    case_end(name);
    bench_close(b);
}

/* ---- E25 XR16C854, E26 XR16C864 ------------------------------------------------------ */

static void case_e25(void) {
    Bench *b = bench_open_cfg("SFX-XR16C854-qfp100", HALF_18432, &E_C854_BENCH);
    Bench *d = bench_open_cfg("SFX-XR16C854-lqfp64-driven", HALF_18432, &QUAD_C_BENCH);
    SimTime bt;
    unsigned i;
    char text[9];

    case_begin();
    expect_pin(d, "INTA", LOGIC_0, "XR16C854DIV (part intPin driven): INT driven");
    bench_close(d);
    e_dvid(b, "00010100");
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    /* Table B (FCTR[5:4] = 01): RX trigger 00 = 8. */
    bus_write(b, 3, 0xBF);
    bus_write(b, 1, 0x10);                 /* FCTR: table B */
    expect_reg(b, 1, "00010000", "FCTR at 001 in the 0xBF bank");
    bus_write(b, 3, 0x03);
    bus_write(b, 2, 0x01);
    bus_write(b, 1, 0x01);
    for (i = 0; i < 7; i++) {
        drive_named(b, "RXA", 0x41, bt);
    }
    expect_reg(b, 2, "11000001", "table B, 7 bytes: below 8");
    drive_named(b, "RXA", 0x41, bt);
    expect_reg(b, 2, "11000100", "table B: RX trigger 8");
    /* FC: the RX count at 000 in the 0xBF bank. */
    bus_write(b, 3, 0xBF);
    expect_reg(b, 0, "00001000", "FC: 8 bytes");
    /* Table D: TRG 3 (FCTR[7] = 0: RX). */
    bus_write(b, 1, 0x30);
    bus_write(b, 0, 0x03);
    bus_write(b, 3, 0x03);
    bus_write(b, 2, 0x07);                 /* empty the FIFOs */
    for (i = 0; i < 2; i++) {
        drive_named(b, "RXA", 0x42, bt);
    }
    expect_reg(b, 2, "11000001", "TRG 3: 2 bytes below");
    drive_named(b, "RXA", 0x42, bt);
    expect_reg(b, 2, "11000100", "TRG 3: the third byte triggers");
    /* TRG 0 is out of range: the trigger is X (decisions.md). */
    bus_write(b, 3, 0xBF);
    bus_write(b, 0, 0x00);
    bus_write(b, 3, 0x03);
    bus_read_text(b, 2, text);
    check(strcmp(text, "XXXXXXXX") == 0, "TRG 0: whether the trigger is reached is X");
    /* FLVL/EMSR swap (FCTR[6]): EMSR[1:0] = 11 alternates RX and TX counts. */
    bus_write(b, 3, 0xBF);
    bus_write(b, 1, 0x40);
    bus_write(b, 3, 0x03);
    bus_write(b, 7, 0x03);
    expect_reg(b, 7, "00000011", "FLVL first read: RX count 3");
    expect_reg(b, 7, "00000000", "second read: TX count 0");
    expect_reg(b, 7, "00000011", "third: RX again");
    bus_write(b, 3, 0xBF);
    bus_write(b, 1, 0x00);
    bus_write(b, 3, 0x03);
    bus_write(b, 2, 0x07);
    /* Exar Xon/Xoff: EFR = 1011 compares Xon1 OR Xon2 (exarOr, P9). */
    bus_write(b, 3, 0xBF);
    bus_write(b, 4, 0x11);                 /* Xon1 */
    bus_write(b, 5, 0x12);                 /* Xon2 */
    bus_write(b, 6, 0x13);                 /* Xoff1 */
    bus_write(b, 7, 0x14);                 /* Xoff2 */
    bus_write(b, 2, 0x1B);
    bus_write(b, 3, 0x03);
    bus_write(b, 1, 0x20);                 /* IER[5]: Xoff interrupt */
    drive_named(b, "RXA", 0x14, bt);       /* Xoff2 alone halts (either character) */
    expect_reg(b, 2, "11010000", "Xoff2 alone: Xoff interrupt (ISR 0x10)");
    bus_write(b, 0, 0x55);
    check(!wait_level(b, "TXA", LOGIC_0, bt * 12), "halted by Xoff2");
    drive_named(b, "RXA", 0x12, bt);       /* Xon2 alone resumes */
    check(wait_level(b, "TXA", LOGIC_0, bt * 12), "Xon2 alone resumes");
    check((bus_read(b, 5) & 0x01) == 0, "flow characters are not stored");
    run(b, bt * 12);
    /* Xon-any (MCR[5]): any character resumes, and is stored. */
    drive_named(b, "RXA", 0x13, bt);       /* Xoff1 */
    bus_write(b, 4, 0x20);
    bus_write(b, 0, 0x56);
    check(!wait_level(b, "TXA", LOGIC_0, bt * 12), "halted by Xoff1");
    drive_named(b, "RXA", 0x41, bt);
    check(wait_level(b, "TXA", LOGIC_0, bt * 12), "Xon-any: 0x41 resumes the transmitter");
    expect_reg(b, 0, "01000001", "and 0x41 is stored");
    bus_write(b, 4, 0x00);
    /* FSTAT (FSRS low, any address): TX ready bits. */
    set(b, "FSRS", LOGIC_0);
    select_unit(b, 4);
    run(b, bt * 12);
    expect_reg(b, 3, "11111111", "FSTAT (any address): TX ready (NOT TXRDY) x4, RXRDY pins high x4");
    set(b, "FSRS", LOGIC_1);
    /* IRTX: always IrDA-encoded (MCR[6] = 0): idle low, a pulse per 0 bit. */
    program_unit(b, 1, 1, 0x03);
    expect_pin(b, "IRTXB", LOGIC_0, "IRTX idles low");
    bus_write(b, 0, 0x00);
    check(wait_level(b, "IRTXB", LOGIC_1, bt * 2), "IRTX pulses for the start bit");
    check(wait_level(b, "IRTXB", LOGIC_0, bt), "a 3/16-bit pulse");
    run(b, bt * 12);
    /* Channel C's clock is the CHCCLK pin (held low: no ticks). */
    program_unit(b, 2, 1, 0x03);
    bus_write(b, 0, 0x55);
    check(!wait_level(b, "TXC", LOGIC_0, bt * 3), "CHCCLK still: channel C does not send");
    for (i = 0; i < 40 && get(b, "TXC") != LOGIC_0; i++) {
        set(b, "CHCCLK", LOGIC_1);
        run(b, NS(300));
        set(b, "CHCCLK", LOGIC_0);
        run(b, NS(300));
    }
    check(get(b, "TXC") == LOGIC_0, "CHCCLK edges clock channel C");
    case_end("E25 XR16C854: tables A-D, TRG (0 = X), FC, FLVL/EMSR, Exar-or Xon/Xoff, FSTAT, "
             "IRTX, CHCCLK, DIV");
    bench_close(b);
}

static void case_e26(void) {
    Bench *b = bench_open_cfg("SFX-XR16C864", HALF_18432, &E_C864_BENCH);
    SimTime bt;
    unsigned i;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    expect_pin(b, "OP1A", LOGIC_1, "OP1 = NOT MCR[2]: high");
    /* FCTR[3]: OP1 is the RS-485 direction, low while transmitting. */
    bus_write(b, 3, 0xBF);
    bus_write(b, 1, 0x08);
    bus_write(b, 3, 0x03);
    run(b, NS(300));
    expect_pin(b, "OP1A", LOGIC_1, "RS-485 idle: OP1 high (receive)");
    bus_write(b, 0, 0x5A);
    run(b, bt * 3);
    expect_pin(b, "OP1A", LOGIC_0, "transmitting: OP1 low");
    run(b, bt * 9);
    expect_pin(b, "OP1A", LOGIC_1, "after the last stop bit: OP1 high again (no delay)");
    bus_write(b, 3, 0xBF);
    bus_write(b, 1, 0x00);
    bus_write(b, 3, 0x03);
    /* BCLK: the 16x clock (divisor 1: the clock itself). */
    wait_level(b, "BCLKA", LOGIC_0, NS(2000));
    check(wait_level(b, "BCLKA", LOGIC_1, NS(2000)), "BCLK toggles (16x clock)");
    /* DMA requests: EMSR[3]: RXRDY becomes RXDRQ (active high). */
    bus_write(b, 3, 0xBF);
    bus_write(b, 1, 0x40);                 /* FCTR[6]: EMSR at 7 */
    bus_write(b, 3, 0x03);
    bus_write(b, 2, 0x41);                 /* FIFO, RX trigger 4 */
    bus_write(b, 7, 0x0C);                 /* EMSR[3:2]: both DRQs */
    run(b, NS(300));
    expect_pin(b, "RXRDYA", LOGIC_0, "RXDRQ: no request");
    expect_pin(b, "TXRDYA", LOGIC_1, "TXDRQ: the TX FIFO is below its trigger");
    for (i = 0; i < 4; i++) {
        drive_named(b, "RXA", 0x61 + i, bt);
    }
    expect_pin(b, "RXRDYA", LOGIC_1, "RX trigger 4: RXDRQ high");
    /* A DMA read: DACKA low and AEN high read the RX FIFO, CS and address ignored. */
    select_unit(b, 4);
    set(b, "DACKA", LOGIC_0);
    set(b, "AEN", LOGIC_1);
    run(b, NS(100));
    expect_reg(b, 5, "01100001", "DMA read: the RX FIFO (0x61), whatever the address");
    /* TC ends the request of the channel whose DACK is low. */
    set(b, "TC", LOGIC_1);
    run(b, NS(100));
    set(b, "TC", LOGIC_0);
    run(b, NS(300));
    expect_pin(b, "RXRDYA", LOGIC_0, "TC: RXDRQ ends");
    expect_pin(b, "TXRDYA", LOGIC_0, "TC: TXDRQ ends too");
    set(b, "DACKA", LOGIC_1);
    set(b, "AEN", LOGIC_0);
    select_unit(b, 0);
    expect_reg(b, 0, "01100010", "the next RX byte (0x62) by a normal read");
    case_end("E26 XR16C864: 128-byte FIFO, FCTR[3] RS-485 on OP1, BCLK, DRQ/DACK/AEN/TC");
    bench_close(b);
}

/* ---- E27 XR16V798, E28 XR16L784: the 5G flat map --------------------------------------- */

static void case_e27(void) {
    Bench *b = bench_open_cfg("SFX-XR16V798", HALF_18432, &E_V798_BENCH);
    SimTime bt;
    SimTime t;
    unsigned i;

    case_begin();
    expect_reg(b, 0x8D, "01001000", "global DVID 0x48");
    expect_reg(b, 0x8C, "00000001", "global DREV 0x01");
    expect_pin(b, "INT", LOGIC_Z, "INT# open drain, released");
    /* Channel 5 at 0x50: its registers, with FCTR/EFR at 8/9. */
    bus_write(b, 0x57, 0x5E);
    expect_reg(b, 0x57, "01011110", "channel 5 SPR at 0x57");
    expect_reg(b, 0x37, "11111111", "channel 3's SPR untouched (0xFF)");
    bus_write(b, 0x53, 0x80);
    expect_reg(b, 0x50, "00000001", "DLL 0x01 after RST#");
    bus_write(b, 0x53, 0x03);
    bt = bit_time(b, 1);
    /* TXCNT / RXCNT. */
    bus_write(b, 0x52, 0x01);
    for (i = 0; i < 3; i++) {
        drive_named(b, "RX5", 0x41 + i, bt);
    }
    expect_reg(b, 0x5B, "00000011", "RXCNT 3");
    /* INT0 / INT1-3: channel 5 code 001 (RX data) at bits 17:15. */
    bus_write(b, 0x51, 0x01);
    run(b, NS(300));
    expect_pin(b, "INT", LOGIC_0, "channel 5's interrupt pulls INT# low");
    expect_reg(b, 0x80, "00100000", "INT0: channel 5 pending");
    expect_reg(b, 0x82, "10000000", "INT2 bit 7 = channel 5 code bit 0");
    expect_reg(b, 0x83, "00000000", "INT3: channel 5 code bits 2:1 = 00");
    bus_write(b, 0x51, 0x00);
    bus_write(b, 0x52, 0x07);
    /* REGB: a write to every channel. */
    bus_write(b, 0x8E, 0x01);
    bus_write(b, 0x07, 0x77);
    bus_write(b, 0x8E, 0x00);
    expect_reg(b, 0x67, "01110111", "REGB: channel 6 got the write");
    expect_reg(b, 0x17, "01110111", "and channel 1");
    /* RESET register: soft reset channel 6 only. */
    bus_write(b, 0x8A, 0x40);
    expect_reg(b, 0x67, "11111111", "RESET bit 6: channel 6 back to SPR 0xFF");
    expect_reg(b, 0x17, "01110111", "channel 1 kept its value");
    /* 8XMODE: channel 0 at 8x. */
    bus_write(b, 0x88, 0x01);
    bus_write(b, 0x03, 0x03);
    bus_write(b, 0x00, 0xFF);
    t = e_low_time(b, "TX0", NS(100000), NS(50));
    check(t >= b->period * 7 && t <= b->period * 9, "8XMODE: 8 clocks per bit");
    run(b, NS(100000));
    bus_write(b, 0x88, 0x00);
    /* The timer: one-shot, 100 oscillator clocks, interrupt on INT#. */
    bus_write(b, 0x86, 100);
    bus_write(b, 0x87, 0);
    bus_write(b, 0x84, 0x05);              /* clock = oscillator */
    bus_write(b, 0x84, 0x03);              /* one-shot */
    bus_write(b, 0x84, 0x01);              /* interrupt on */
    bus_write(b, 0x84, 0x09);              /* start */
    run(b, b->period * 90);
    expect_pin(b, "INT", LOGIC_Z, "90 clocks: no timer interrupt yet");
    run(b, b->period * 15);
    expect_pin(b, "INT", LOGIC_0, "100 clocks: the timer interrupt");
    expect_reg(b, 0x81, "00000111", "INT1: channel 0 code 111 (timer)");
    expect_reg(b, 0x84, "00000001", "TIMERCNTL reads 01 (pending), and the read clears it");
    run(b, NS(300));
    expect_pin(b, "INT", LOGIC_Z, "INT# released");
    /* Send-immediate (MCR[3]): the next THR byte goes out ahead of the FIFO. */
    bus_write(b, 0x09, 0x10);              /* EFR[4] */
    bus_write(b, 0x02, 0x01);
    for (i = 0; i < 4; i++) {
        bus_write(b, 0x00, 0x11 + i);
    }
    bus_write(b, 0x04, 0x08);
    bus_write(b, 0x00, 0xA5);
    run(b, bt * 10 * 6);
    {
        uint16_t frames[64];
        size_t n = sent(b, frames, 64);

        check(n >= 5 && frames[n - 5] == 0x11 && frames[n - 4] == 0xA5 && frames[n - 3] == 0x12 &&
              frames[n - 1] == 0x14, "send-immediate: 0x11, then 0xA5 ahead of 0x12-0x14");
    }
    expect_reg(b, 0x04, "00000000", "MCR[3] self-clears");
    /* TX disable (MSR write [3]). */
    bus_write(b, 0x06, 0x08);
    bus_write(b, 0x00, 0x55);
    check(!wait_level(b, "TX0", LOGIC_0, bt * 12), "TX disabled: the byte waits");
    bus_write(b, 0x06, 0x00);
    check(wait_level(b, "TX0", LOGIC_0, bt * 4), "enabled again: it goes");
    run(b, bt * 12);
    /* Auto flow on DTR/DSR (MCR[2] = 1): DSR inactive holds the transmitter. */
    bus_write(b, 0x09, 0x90);              /* EFR[7] auto CTS/DSR, EFR[4] */
    bus_write(b, 0x04, 0x04);
    bus_write(b, 0x00, 0x66);
    check(!wait_level(b, "TX0", LOGIC_0, bt * 12), "MCR[2] = 1: DSR high (inactive) holds TX");
    set(b, "DSR0", LOGIC_0);
    check(wait_level(b, "TX0", LOGIC_0, bt * 4), "DSR active: TX goes (CTS ignored)");
    run(b, bt * 12);
    bus_write(b, 0x04, 0x00);
    bus_write(b, 0x09, 0x10);
    /* Auto RS-485 on RTS (MCR[2] = 0) with a 2-bit turnaround (MSR write [7:4]). */
    bus_write(b, 0x06, 0x20);              /* MSR write: delay 2 */
    bus_write(b, 0x08, 0x20);              /* FCTR[5] */
    run(b, NS(300));
    expect_pin(b, "RTS0", LOGIC_0, "RS-485 idle: RTS low");
    bus_write(b, 0x00, 0x33);
    run(b, bt * 2);
    expect_pin(b, "RTS0", LOGIC_1, "transmitting: RTS high");
    t = sim_now(b->sim);
    wait_level(b, "RTS0", LOGIC_0, bt * 20);
    t = sim_now(b->sim) - t;
    check(t >= bt * 9 && t <= bt * 11, "low 2 bit times after the last stop bit");
    /* SLEEP register: a sleeping channel wakes on a THR write. */
    bus_write(b, 0x08, 0x00);
    bus_read(b, 0x06);                     /* clear the MSR deltas (DSR0 moved) */
    bus_write(b, 0x8B, 0x01);
    run(b, bt * 2);
    bus_write(b, 0x00, 0x44);
    check(wait_level(b, "TX0", LOGIC_0, bt * 4), "asleep: a THR write wakes channel 0");
    run(b, bt * 12);
    expect_pin(b, "INT", LOGIC_0, "the wake-up indicator asserts INT#");
    expect_reg(b, 0x02, "11000001", "ISR reads none (the indicator), and the read clears it");
    bus_write(b, 0x8B, 0x00);
    run(b, NS(300));
    expect_pin(b, "INT", LOGIC_Z, "released");
    case_end("E27 XR16V798: 5G map, global block (INT0-3, DVID, REGB, RESET, 8XMODE, SLEEP), "
             "timer, send-immediate, TX disable, DTR/DSR flow, RS-485 delay");
    bench_close(b);
}

static void case_e28(void) {
    Bench *b = bench_open_cfg("SFX-XR16L784", HALF_18432, &E_L784_BENCH);
    SimTime bt;
    SimTime t;
    unsigned i;

    case_begin();
    expect_reg(b, 0x8D, "00100100", "global DVID 0x24");
    expect_reg(b, 0x47, "XXXXXXXX", "0x40-0x7F reserved: X");
    bus_write(b, 0x33, 0x80);
    expect_reg(b, 0x30, "XXXXXXXX", "DLL X (not reset)");
    bus_write(b, 0x30, 0x01);
    bus_write(b, 0x31, 0x00);
    bus_write(b, 0x33, 0x03);
    bt = bit_time(b, 1);
    /* RS-485 on RTS only (planner decision), even with MCR[2] = 1. */
    bus_write(b, 0x39, 0x10);
    bus_write(b, 0x34, 0x04);              /* MCR[2] = 1 (DTR/DSR flow select) */
    bus_write(b, 0x36, 0x10);              /* delay 1 */
    bus_write(b, 0x38, 0x20);              /* FCTR[5] */
    run(b, NS(300));
    expect_pin(b, "RTS3", LOGIC_0, "RS-485 idle: RTS low");
    bus_write(b, 0x30, 0x5A);
    run(b, bt * 2);
    expect_pin(b, "RTS3", LOGIC_1, "transmitting: RTS high (not DTR)");
    expect_pin(b, "DTR3", LOGIC_1, "DTR untouched (MCR[0] = 0: inactive)");
    t = sim_now(b->sim);
    wait_level(b, "RTS3", LOGIC_0, bt * 20);
    t = sim_now(b->sim) - t;
    check(t >= bt * 8 && t <= bt * 10, "low 1 bit time after the last stop bit");
    /* Timer: bit-mapped TIMERCNTL (re-triggerable, interrupt, start). */
    bus_write(b, 0x86, 50);
    bus_write(b, 0x87, 0);
    bus_write(b, 0x84, 0x03);
    expect_reg(b, 0x84, "00000011", "TIMERCNTL reads its bits");
    run(b, b->period * 55);
    expect_pin(b, "INT", LOGIC_0, "the timer interrupt");
    bus_read(b, 0x84);
    run(b, NS(300));
    expect_pin(b, "INT", LOGIC_Z, "a TIMERCNTL read clears it");
    run(b, b->period * 50);
    expect_pin(b, "INT", LOGIC_0, "re-triggerable: again N clocks later");
    bus_write(b, 0x84, 0x00);
    bus_read(b, 0x84);
    /* Timer on TMRCK: one-shot, N = 3 pin edges. */
    bus_write(b, 0x86, 3);
    bus_write(b, 0x84, 0x0F);              /* TMRCK, one-shot, start, interrupt */
    for (i = 0; i < 2; i++) {
        set(b, "TMRCK", LOGIC_1);
        run(b, NS(500));
        set(b, "TMRCK", LOGIC_0);
        run(b, NS(500));
    }
    expect_pin(b, "INT", LOGIC_Z, "2 TMRCK edges: not yet");
    set(b, "TMRCK", LOGIC_1);
    run(b, NS(500));
    expect_pin(b, "INT", LOGIC_0, "the third TMRCK edge: timer interrupt");
    bus_read(b, 0x84);
    bus_write(b, 0x84, 0x00);
    /* The timeout interrupt clears when the FIFO empties, not on a read. */
    bus_write(b, 0x03, 0x80);
    bus_write(b, 0x00, 0x01);
    bus_write(b, 0x01, 0x00);
    bus_write(b, 0x03, 0x03);
    bus_write(b, 0x02, 0x41);
    bus_write(b, 0x01, 0x01);
    drive_named(b, "RX0", 0x31, bt);
    drive_named(b, "RX0", 0x32, bt);
    run(b, bt * 60);
    expect_reg(b, 0x02, "11001100", "timeout");
    bus_read(b, 0x00);
    expect_reg(b, 0x02, "11001100", "one byte left: the timeout stays (XR16L784)");
    bus_read(b, 0x00);
    expect_reg(b, 0x02, "11000001", "empty: cleared");
    bus_write(b, 0x01, 0x00);
    /* Sleep: the wake-up indicator clears with an INT0 read, not the ISR. */
    bus_write(b, 0x8B, 0x01);
    run(b, bt * 2);
    bus_write(b, 0x00, 0x44);
    run(b, bt * 12);
    expect_pin(b, "INT", LOGIC_0, "woken: the wake-up indicator");
    bus_read(b, 0x02);
    run(b, NS(300));
    expect_pin(b, "INT", LOGIC_0, "an ISR read leaves it (XR16L784)");
    bus_read(b, 0x80);
    run(b, NS(300));
    expect_pin(b, "INT", LOGIC_Z, "an INT0 read clears it");
    bus_write(b, 0x8B, 0x00);
    case_end("E28 XR16L784: 5G map with 4 channels, reserved 0x40, RS-485 on RTS, bit-mapped "
             "timer (TMRCK), timeout and wake-up clear rules");
    bench_close(b);
}


int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_e16();
    case_e17_e20("SFX-XR16V2551-tqfp48-2551", &E_DUAL_MOTO_BENCH, "00000010",
                 "E17 XR16V2551: DVID 0x02, Motorola, PowerSave with the wake-up indicator");
    case_e18_e21("SFX-XR16V2552", "00000010",
                 "E18 XR16V2552: MF (BAUDOUT by AFR[2:1]), AFR vs DLD by EFR[4], shared AFR[0]");
    case_e19();
    case_e17_e20("SFX-XR16V2651-tqfp48-2651", &E_DUAL_CLKSEL_BENCH, "00000110",
                 "E20 XR16V2651: DVID 0x06, Motorola, PowerSave with the wake-up indicator");
    case_e18_e21("SFX-XR16V2652", "00000110",
                 "E21 XR16V2652: the 2552 wrapper with 32-byte FIFOs (MF, AFR/DLD, AFR[0])");
    case_e22();
    case_e23_e24("SFX-XR16V564", "SFX-XR16V564-lqfp64-driven", 32,
                 "E23 XR16V564: 32-byte FIFO, CLKSEL ÷4, DIV part, 100-111 X with DLAB, sleep");
    case_e23_e24("SFX-XR16V654", "SFX-XR16V654-lqfp64-driven", 64,
                 "E24 XR16V654: 64-byte FIFO, CLKSEL ÷4, DIV part, read-only LSR, sleep");
    case_e25();
    case_e26();
    case_e27();
    case_e28();
    return serial_bench_finish();
}
