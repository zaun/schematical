/*
 * serial_exar_test: the Exar / MaxLinear options E1-E15
 * (split from serial_test.c by plan-68-G; the bench is support/serial_bench.c).
 */

#include "serial_bench.h"
#include "serial_bench_d.h"
#include "serial_bench_exar.h"

/* ---- E1 ST16C550, E2 ST16C450: the Exar 16550/16450 baselines ------------------------- */

/* Drives RX low for 7.5 clocks, starting 20 ns before a clock edge, then high:
   1 when the receiver took it as a start bit (it then reads 0xFF). */
static int e_half_start(Bench *b) {
    run(b, b->period * 32);                /* mark first: the receiver needs a 1 before the edge */
    e_before_rise(b, NS(20));
    set(b, "RX", LOGIC_0);
    run(b, b->period * 15 / 2);
    set(b, "RX", LOGIC_1);
    run(b, b->period * 16 * 12);
    return (bus_read(b, 5) & 1u) != 0;
}

static void case_e1(void) {
    Bench *b = bench_open_cfg("SFX-ST16C550", HALF_18432, &E_ST450_BENCH);
    SimTime bt;
    unsigned i;
    char text[9];

    case_begin();
    bus_write(b, 7, 0x12);
    pulse_reset(b);
    expect_reg(b, 7, "11111111", "SPR resets to 0xFF (Table 4)");
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    /* Start bit checked 7.5 clocks in: a 7.5-clock low pulse is a start bit. */
    check(e_half_start(b), "a 7.5-clock start pulse is a start bit (startSample 7.5)");
    bus_read(b, 0);
    /* DMA class N: mode-1 TXRDY low again as soon as one slot frees. */
    bus_write(b, 2, 0x09);                 /* FIFO, DMA mode 1 */
    for (i = 0; i < 17; i++) {
        bus_write(b, 0, 0x30 + i);
    }
    run(b, NS(2000));
    expect_pin(b, "TXRDY", LOGIC_1, "TX FIFO full: TXRDY high");
    run(b, bt * 11);
    expect_pin(b, "TXRDY", LOGIC_0, "class N: one slot free, TXRDY low (P would wait for empty)");
    run(b, bt * 10 * 18);
    /* Timeout 4 characters + 12 bit times. */
    bus_write(b, 2, 0x47);                 /* trigger 4: one byte is below it */
    bus_write(b, 1, 0x01);
    drive_named(b, "RX", 0x41, bt);
    run(b, bt * 45);
    bus_read_text(b, 2, text);
    check(strcmp(text, "11000001") == 0, "45 bit times after the byte: no timeout yet (4ch would be)");
    run(b, bt * 8);
    expect_reg(b, 2, "11001100", "4 characters + 12 bit times: timeout (4w12)");
    expect_pin(b, "INT", LOGIC_1, "INT driven high (no OUT2 gating)");
    bus_read(b, 0);
    run(b, NS(500));
    expect_pin(b, "INT", LOGIC_0, "INT driven low");
    case_end("E1 ST16C550: SPR 0xFF, start at 7.5, DMA class N, 4w12 timeout, driven INT");
    bench_close(b);
}

static void case_e2(void) {
    Bench *b = bench_open_cfg("SFX-ST16C450", HALF_18432, &E_ST450_BENCH);

    case_begin();
    expect_reg(b, 7, "11111111", "SPR 0xFF after reset");
    expect_reg(b, 2, "00000001", "16450: IIR[7:6] = 00");
    bus_write(b, 2, 0xC1);
    expect_reg(b, 2, "00000001", "no FCR: IIR stays 0x01");
    expect_pin(b, "CSOUT", LOGIC_1, "selected: CSOUT high");
    set(b, "CS2", LOGIC_1);
    run(b, NS(200));
    expect_pin(b, "CSOUT", LOGIC_0, "deselected: CSOUT low");
    set(b, "CS2", LOGIC_0);
    /* The active-high IOR strobe reads too. */
    bus_write(b, 7, 0x5A);
    set(b, "IORBAR", LOGIC_1);
    set_address(b, 7);
    set(b, "IOR", LOGIC_1);
    run(b, NS(150));
    expect_pin(b, "DDIS", LOGIC_0, "a read in progress: DDIS low");
    check(get(b, "D1") == LOGIC_1 && get(b, "D0") == LOGIC_0, "IOR (active high) reads SPR 0x5A");
    set(b, "IOR", LOGIC_0);
    run(b, NS(200));
    expect_pin(b, "DDIS", LOGIC_1, "read over: DDIS high");
    program(b, 1, 0x03);
    check(e_half_start(b), "a 7.5-clock start pulse is a start bit");
    expect_reg(b, 0, "11111111", "and reads 0xFF");
    case_end("E2 ST16C450: 16450 Exar (SPR 0xFF, CSOUT, DDIS, IOR/IORBAR, start at 7.5)");
    bench_close(b);
}

/* ---- E3 ST16C1450, E4 ST16C1550: IER[5] special mode -------------------------------- */

static void case_e3(void) {
    Bench *b = bench_open_cfg("SFX-ST16C1450", HALF_18432, &E_ONE_BENCH);
    SimTime bt;

    case_begin();
    expect_pin(b, "INT", LOGIC_Z, "INT 3-state after reset (MCR[3] = 0)");
    expect_pin(b, "RST", LOGIC_0, "RST follows RESET (low)");
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 4, 0x84);                 /* MCR[7] without IER[5]: not written */
    expect_reg(b, 4, "00000100", "MCR[7] needs IER[5]");
    expect_pin(b, "RST", LOGIC_0, "MCR[2] without IER[5]: RST low");
    bus_write(b, 1, 0x20);                 /* IER[5] special mode */
    expect_reg(b, 1, "00100000", "IER[5] writable");
    run(b, NS(200));
    expect_pin(b, "RST", LOGIC_1, "IER[5] and MCR[2]: RST high (soft reset out)");
    expect_reg(b, 2, "00010001", "ISR[4] = TXRDY (THR empty), active high");
    drive_named(b, "RX", 0x41, bt);
    run(b, bt);
    expect_reg(b, 2, "00110001", "data: ISR[5] = RXRDY (IER[0] = 0: no interrupt)");
    bus_read(b, 0);
    /* THR interrupt: an ISR read does not clear it. */
    bus_write(b, 4, 0x08);                 /* INT enabled, RST low again */
    bus_write(b, 1, 0x22);
    run(b, NS(300));
    expect_pin(b, "INT", LOGIC_1, "THR empty: INT high");
    expect_reg(b, 2, "00010010", "ISR: TX ready");
    expect_reg(b, 2, "00010010", "still TX ready after a read (ier1Rewrite)");
    bus_write(b, 1, 0x20);
    expect_reg(b, 2, "00010001", "IER[1] = 0: cleared");
    /* Power-down (MCR[7] with IER[5]): the baud clock stops. */
    bus_write(b, 4, 0x88);
    expect_reg(b, 4, "10001000", "MCR[7] = 1 with IER[5]");
    bus_write(b, 1, 0x22);
    expect_reg(b, 2, "00010010", "IER[1] again with the THR empty: TX ready");
    bus_write(b, 0, 0x5A);
    expect_reg(b, 2, "00000001", "a THR write clears it (THR full: ISR[4] = 0)");
    check(!wait_level(b, "TX", LOGIC_0, bt * 12), "powered down: nothing is sent");
    bus_write(b, 4, 0x08);
    check(wait_level(b, "TX", LOGIC_0, bt * 4), "MCR[7] = 0: the byte goes out");
    /* Loopback cuts MCR[2] from RST. */
    run(b, bt * 12);
    bus_write(b, 4, 0x14);
    run(b, NS(200));
    expect_pin(b, "RST", LOGIC_0, "loopback: RST = RESET only");
    case_end("E3 ST16C1450: IER[5] mode (ISR[5:4], RST, MCR[7] power-down), ier1Rewrite THR rule");
    bench_close(b);
}

static void case_e4(void) {
    Bench *b = bench_open_cfg("SFX-ST16C1550", HALF_18432, &E_ONE_BENCH);
    SimTime bt;
    SimTime ct;
    unsigned i;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    ct = bt * 10;
    bus_write(b, 2, 0x21);                 /* FIFO; FCR[5:4] without IER[5]: ignored */
    bus_write(b, 1, 0x22);
    expect_reg(b, 2, "11100010", "IER[5]: ISR[5:4] inverted (TX empty: [4] = 0; RX empty: [5] = 1)");
    bus_write(b, 2, 0x21);                 /* TX trigger 8, written with IER[5] */
    bus_write(b, 4, 0x08);
    for (i = 0; i < 14; i++) {
        bus_write(b, 0, 0x40 + i);
    }
    run(b, ct * 11 / 2);
    expect_pin(b, "INT", LOGIC_0, "TX trigger 8: 8 bytes still in the FIFO, no interrupt");
    run(b, ct * 3 / 2);
    expect_pin(b, "INT", LOGIC_1, "the count fell below 8: TX interrupt (Exar counts bytes)");
    expect_reg(b, 2, "11110010", "ISR: TX ready; FIFO not empty [4] = 1, RX empty [5] = 1");
    run(b, ct * 8);
    /* Not filled to the trigger: the interrupt waits for empty. */
    bus_write(b, 1, 0x20);
    bus_write(b, 1, 0x22);
    bus_read(b, 2);
    bus_write(b, 0, 0x31);
    bus_write(b, 0, 0x32);
    bus_write(b, 0, 0x33);
    run(b, ct * 3 / 2);
    expect_pin(b, "INT", LOGIC_0, "3 bytes (trigger 8 never reached): no interrupt yet");
    run(b, ct * 2);
    expect_pin(b, "INT", LOGIC_1, "the FIFO emptied: TX interrupt");
    /* MCR[7] is power-down (decisions.md), not a prescaler. */
    bus_write(b, 4, 0x88);
    bus_write(b, 0, 0x5A);
    check(!wait_level(b, "TX", LOGIC_0, bt * 12), "MCR[7]: powered down");
    bus_write(b, 4, 0x08);
    check(wait_level(b, "TX", LOGIC_0, bt * 4), "and back");
    case_end("E4 ST16C1550: IER[5] TX trigger (bytes), inverted ISR[5:4], MCR[7] power-down");
    bench_close(b);
}

/* ---- E5 ST16C2550, E7 XR16C2550: dual, per-channel CS ------------------------------- */

static void case_e5_e7(const char *part, const char *name) {
    Bench *b = bench_open_cfg(part, HALF_18432, &DUAL_CD_BENCH);
    SimTime bt;
    char text[9];

    case_begin();
    expect_pin(b, "INTA", LOGIC_Z, "INT 3-state until MCR[3]");
    expect_pin(b, "OP2A", LOGIC_1, "OP2 = NOT MCR[3]");
    /* Both CS low: a write reaches both channels; a read is X. */
    set(b, "CSB", LOGIC_0);
    run(b, NS(100));
    bus_write(b, 7, 0x3C);
    bus_read_text(b, 7, text);
    check(strcmp(text, "XXXXXXXX") == 0, "a read with both CS low: X (contention)");
    select_unit(b, 1);
    expect_reg(b, 7, "00111100", "channel B got the broadcast write");
    select_unit(b, 0);
    expect_reg(b, 7, "00111100", "channel A too");
    bus_write(b, 4, 0x08);
    run(b, NS(300));
    expect_pin(b, "OP2A", LOGIC_0, "MCR[3] = 1: OP2A low");
    expect_pin(b, "INTA", LOGIC_0, "MCR[3] = 1: INTA driven");
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    /* Timeout above RX data (Exar ISR table). */
    bus_write(b, 2, 0x07);                 /* FIFO, trigger 1 */
    bus_write(b, 1, 0x01);
    drive_named(b, "RXA", 0x41, bt);
    expect_reg(b, 2, "11000100", "RX data at trigger 1");
    run(b, bt * 60);
    expect_reg(b, 2, "11001100", "RX data and timeout both pending: the timeout shows");
    bus_read(b, 0);
    /* Break: LSR[4] holds while RX stays low. */
    set(b, "RXA", LOGIC_0);
    run(b, bt * 14);
    check((bus_read(b, 5) & 0x10) != 0, "break: LSR[4]");
    check((bus_read(b, 5) & 0x10) != 0, "RX still low: LSR[4] holds after the read");
    set(b, "RXA", LOGIC_1);
    run(b, bt * 2);
    bus_read(b, 5);
    check((bus_read(b, 5) & 0x10) == 0, "RX back to mark: an LSR read clears LSR[4]");
    case_end(name);
    bench_close(b);
}

/* ---- E6 ST16C554: quad; the D variant's INT ----------------------------------------- */

static void case_e6(void) {
    Bench *b = bench_open_cfg("SFX-ST16C554", HALF_18432, &QUAD_C_BENCH);
    Bench *d;
    unsigned u;

    case_begin();
    for (u = 0; u < 4; u++) {
        select_unit(b, u);
        bus_write(b, 7, 0x10 + u);
    }
    for (u = 0; u < 4; u++) {
        char want[9];
        unsigned i;

        for (i = 0; i < 8; i++) {
            want[7 - i] = ((0x10 + u) >> i) & 1u ? '1' : '0';
        }
        want[8] = '\0';
        select_unit(b, u);
        expect_reg(b, 7, want, "each channel's own SPR");
    }
    expect_pin(b, "INTC", LOGIC_Z, "ST16C554CQ64 (INTSEL to GND): INT 3-state with MCR[3] = 0");
    d = bench_open_cfg("SFX-ST16C554-lqfp64-driven", HALF_18432, &QUAD_C_BENCH);
    expect_pin(d, "INTC", LOGIC_0, "ST16C554DCQ64 (part intPin driven): INT driven low");
    bus_write(d, 1, 0x02);
    run(d, NS(300));
    expect_pin(d, "INTA", LOGIC_1, "the D part: INT high without MCR[3]");
    bench_close(d);
    case_end("E6 ST16C554: four channels; part key intPin: CQ64 3-state, DCQ64 driven");
    bench_close(b);
}

/* ---- E8 XR16L580: exar650 bank, auto flow in loopback ------------------------------- */

static void case_e8(void) {
    Bench *b = bench_open_cfg("SFX-XR16L580-tqfp48", HALF_18432, &E_L580_BENCH);
    SimTime bt;
    unsigned i;

    case_begin();
    expect_reg(b, 7, "11111111", "SPR 0xFF");
    bus_write(b, 3, 0x80);
    expect_reg(b, 0, "00000001", "DLL 0x01 at power-up");
    bus_write(b, 0, 0x00);
    expect_reg(b, 0, "00000001", "DLL = DLM = 0: DREV 0x01");
    expect_reg(b, 1, "00000001", "DVID 0x01");
    bus_write(b, 0, 0x01);
    expect_reg(b, 0, "00000001", "the DLL write still landed");
    bus_write(b, 3, 0xBF);
    bus_write(b, 2, 0x10);
    expect_reg(b, 2, "00010000", "LCR = 0xBF: EFR");
    bus_write(b, 4, 0x11);                 /* Xon1 (write-only) */
    expect_reg(b, 0, "XXXXXXXX", "000 at 0xBF: X (decisions.md)");
    bus_write(b, 3, 0x03);
    bus_write(b, 4, 0x00);
    bus_write(b, 3, 0xBF);
    expect_reg(b, 4, "00000000", "Xon1 is write-only: the read is MCR");
    bus_write(b, 2, 0x00);
    bus_write(b, 3, 0x03);
    bus_write(b, 4, 0x04);
    expect_reg(b, 4, "00000000", "EFR[4] = 0: MCR[2] (with MCR[7:5]) not written");
    bt = bit_time(b, 1);
    /* Auto RTS/CTS in loopback: trigger 1 -> RTS off at 4. */
    e_efr(b, 0xD0);
    bus_write(b, 2, 0x07);
    bus_write(b, 4, 0x12);                 /* loopback, RTS */
    for (i = 0; i < 7; i++) {
        bus_write(b, 0, 0x41 + i);
    }
    run(b, bt * 10 * 9);
    check((bus_read(b, 5) & 0x20) == 0, "RX FIFO at 4: RTS -> CTS off, the rest held back");
    for (i = 0; i < 4; i++) {
        bus_read(b, 0);
    }
    run(b, bt * 10 * 5);
    check((bus_read(b, 5) & 0x60) == 0x60, "emptied: RTS on, the last bytes went out");
    case_end("E8 XR16L580: exar650 bank, DREV/DVID 01/01, write-only Xon, auto flow in loopback");
    bench_close(b);
}

/* ---- E9 XR16L2551, E12 XR16M2551, E14 XR16M2651: Motorola, PowerSave ---------------- */

/* Motorola mode on a 2551-style dual: CS on CSA, A3 on CSB, R/W on IOW,
   RESET active low, one open-drain IRQ on INTA, INTB driven low. */




































/* PowerSave: with PWRSAVE high and the chip asleep the bus is isolated; an RX
   start bit wakes it. `indicator`: the wake-up interrupt (ISR 0x01, INT high). */





































static void case_e9(void) {
    Bench *b = bench_open_cfg("SFX-XR16L2551", HALF_18432, &E_DUAL_MOTO_BENCH);

    case_begin();
    bus_write(b, 3, 0x80);
    bus_write(b, 0, 0x00);
    bus_write(b, 1, 0x00);
    expect_reg(b, 1, "00000010", "DVID 0x02");
    bus_write(b, 3, 0x03);
    e_motorola_dual(b);
    pulse_reset(b);
    select_unit(b, 0);
    expect_reg(b, 7, "11111111", "back in Intel mode: CSA selects channel A");
    e_powersave(b, 0);
    case_end("E9 XR16L2551: Intel and Motorola (A3 channel, IRQ, RESET low), PowerSave isolation");
    bench_close(b);
}

/* ---- E10 XR16L2552: AFR[0] per channel, DREV/DVID, MF ------------------------------- */

static void case_e10(void) {
    Bench *b = bench_open_cfg("SFX-XR16L2552-tqfp48", HALF_18432, &CHSEL_BENCH);

    case_begin();
    expect_pin(b, "INTA", LOGIC_0, "INT always driven (no MCR[3] gating)");
    bus_write(b, 3, 0x80);
    bus_write(b, 0, 0x00);
    bus_write(b, 1, 0x00);
    expect_reg(b, 0, "00000001", "DREV 0x01");
    expect_reg(b, 1, "00000010", "DVID 0x02");
    bus_write(b, 2, 0x01);                 /* channel A AFR[0] = 1 */
    expect_reg(b, 2, "00000001", "AFR at 2 with LCR[7] = 1");
    bus_write(b, 3, 0x03);
    select_unit(b, 1);
    bus_write(b, 3, 0x03);
    select_unit(b, 0);
    bus_write(b, 7, 0x44);                 /* addressed to A, whose AFR[0] = 1: both */
    select_unit(b, 1);
    expect_reg(b, 7, "01000100", "A's AFR[0] = 1: the write reached B too");
    bus_write(b, 7, 0x22);                 /* addressed to B (AFR[0] = 0): B only */
    select_unit(b, 0);
    expect_reg(b, 7, "01000100", "B's AFR[0] = 0: A kept its value (per-channel bit)");
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x00);
    bus_write(b, 3, 0x03);
    /* MF: AFR[2:1] = 00 OP2 (= NOT MCR[3]), 10 RXRDY. */
    expect_pin(b, "MFA", LOGIC_1, "MF = OP2, MCR[3] = 0: high");
    bus_write(b, 4, 0x08);
    run(b, NS(300));
    expect_pin(b, "MFA", LOGIC_0, "MCR[3] = 1: low");
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x04);
    bus_write(b, 3, 0x03);
    run(b, NS(300));
    expect_pin(b, "MFA", LOGIC_1, "MF = RXRDY: no data, high");
    case_end("E10 XR16L2552: AFR[0] broadcast per channel, DREV/DVID 01/02, MF pin");
    bench_close(b);
}

/* ---- E11 XR16M2550, E13 XR16M2650: exar650 with DLD -------------------------------- */

static void case_e11(void) {
    Bench *b = bench_open_cfg("SFX-XR16M2550-tqfp48", HALF_18432, &DUAL_CD_BENCH);
    SimTime t;
    unsigned i;

    case_begin();
    bus_write(b, 3, 0x80);
    expect_reg(b, 0, "00000001", "DLL 0x01 at power-up");
    bus_write(b, 0, 0x00);
    expect_reg(b, 1, "00000010", "DVID 0x02");
    bus_write(b, 0, 0x02);
    expect_reg(b, 2, "00000001", "EFR[4] = 0: address 2 is the ISR (decisions.md)");
    bus_write(b, 3, 0x03);
    e_efr(b, 0x10);
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x18);                 /* DLD: 8x sampling, fraction 8/16 */
    expect_reg(b, 2, "00011000", "EFR[4] = 1: DLD");
    bus_write(b, 3, 0x03);
    bus_write(b, 0, 0xFF);
    t = e_low_time(b, "TXA", NS(200000), NS(50));
    /* (2 + 8/16) x 8 clocks = 20 clocks per bit. */
    check(t >= b->period * 19 && t <= b->period * 21, "DLD: bit = (2 + 8/16) x 8 clocks");
    run(b, NS(400000));
    bus_write(b, 3, 0x80);
    bus_write(b, 0, 0x01);
    bus_write(b, 2, 0x00);
    bus_write(b, 3, 0x03);
    /* Auto RTS (EFR[6]): trigger 1 -> RTS off at 4, on at 0. */
    e_efr(b, 0x50);
    bus_write(b, 2, 0x07);
    bus_write(b, 4, 0x02);
    run(b, NS(300));
    expect_pin(b, "RTSA", LOGIC_0, "auto RTS: RTS active");
    for (i = 0; i < 3; i++) {
        drive_named(b, "RXA", 0x31, bit_time(b, 1));
    }
    expect_pin(b, "RTSA", LOGIC_0, "3 bytes: still active");
    drive_named(b, "RXA", 0x32, bit_time(b, 1));
    expect_pin(b, "RTSA", LOGIC_1, "4 bytes (next level above 1): RTS off");
    for (i = 0; i < 4; i++) {
        bus_read(b, 0);
    }
    run(b, NS(300));
    expect_pin(b, "RTSA", LOGIC_0, "empty: RTS on");
    case_end("E11 XR16M2550: exar650 + DLD (fraction, 8x), DVID 0x02, auto-RTS table");
    bench_close(b);
}

static void case_e13(void) {
    Bench *b = bench_open_cfg("SFX-XR16M2650-tqfp48", HALF_18432, &DUAL_CD_BENCH);
    SimTime bt;
    unsigned i;

    case_begin();
    bus_write(b, 3, 0x80);
    bus_write(b, 0, 0x00);
    expect_reg(b, 1, "00000110", "DVID 0x06");
    bus_write(b, 0, 0x01);
    bus_write(b, 3, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0x01);                 /* FIFO, RX trigger 00 = 8 */
    bus_write(b, 1, 0x01);
    for (i = 0; i < 7; i++) {
        drive_named(b, "RXA", 0x41, bt);
    }
    expect_reg(b, 2, "11000001", "7 bytes: below the default trigger 8");
    drive_named(b, "RXA", 0x41, bt);
    expect_reg(b, 2, "11000100", "8 bytes: RX trigger (32-byte table)");
    for (i = 0; i < 24; i++) {
        drive_named(b, "RXA", 0x42, bt);
    }
    check((bus_read(b, 5) & 0x02) == 0, "32 bytes fit (no overrun)");
    drive_named(b, "RXA", 0x43, bt);
    check((bus_read(b, 5) & 0x02) != 0, "the 33rd overruns");
    /* Xoff1 (EFR[3:2] = 10) two character times after the RX FIFO reaches 8. */
    bus_write(b, 2, 0x07);
    bus_write(b, 3, 0xBF);
    bus_write(b, 6, 0x13);
    bus_write(b, 4, 0x11);
    bus_write(b, 2, 0x18);
    bus_write(b, 3, 0x03);
    for (i = 0; i < 8; i++) {
        drive_named(b, "RXA", 0x50, bt);
    }
    check(!wait_level(b, "TXA", LOGIC_0, bt * 10), "within 2 characters of the trigger: no Xoff");
    check(wait_level(b, "TXA", LOGIC_0, bt * 14), "two character times on: Xoff goes out");
    run(b, bt * 12);
    for (i = 0; i < 8; i++) {
        bus_read(b, 0);
    }
    run(b, bt * 14);
    {
        uint16_t frames[8];
        const SimComponentType *type = sim_instance_type(b->chip);
        size_t n = type->serial_sent(sim_instance_state(b->chip), 0, 0, frames, 8);

        check(n >= 2 && frames[n - 2] == 0x13 && frames[n - 1] == 0x11,
              "Xoff1 (0x13) sent, and Xon1 (0x11) once the FIFO drained to 0");
    }
    case_end("E13 XR16M2650: 32-byte FIFOs, RX trigger 8 by default, DVID 0x06, Xoff 2 chars late");
    bench_close(b);
}

static void case_e12_e14(const char *part, const char *name) {
    Bench *b = bench_open_cfg(part, HALF_18432, &E_DUAL_CLKSEL_BENCH);
    SimTime t;

    case_begin();
    /* CLKSEL low at reset: ÷4, MCR still reads 0 (decisions.md P1). */
    set(b, "CLKSEL", LOGIC_0);
    pulse_reset(b);
    expect_reg(b, 4, "00000000", "CLKSEL low: MCR reads 0x00");
    program(b, 1, 0x03);
    bus_write(b, 0, 0xFF);
    t = e_low_time(b, "TXA", NS(400000), NS(100));
    check(t >= b->period * 63 && t <= b->period * 65, "CLKSEL low: ÷4 (64 clocks per bit)");
    run(b, NS(800000));
    e_efr(b, 0x10);
    bus_write(b, 4, 0x00);                 /* MCR[7] = 0 written: overrides CLKSEL */
    bus_write(b, 0, 0xFF);
    t = e_low_time(b, "TXA", NS(400000), NS(50));
    check(t >= b->period * 15 && t <= b->period * 17, "an MCR[7] write overrides: ÷1");
    run(b, NS(200000));
    set(b, "CLKSEL", LOGIC_1);
    pulse_reset(b);
    e_motorola_dual(b);
    pulse_reset(b);
    e_powersave(b, 1);
    case_end(name);
    bench_close(b);
}

/* ---- E15 XR16M752: m752 (TCR/TLR, FIFO Rdy, DLD[7]/[6]), Motorola IRQ ---------------- */

static void case_e15(void) {
    Bench *b = bench_open_cfg("SFX-XR16M752-tqfp48-68", HALF_18432, &E_DUAL_68_BENCH);
    SimTime bt;
    SimTime t;

    case_begin();
    expect_reg(b, 3, "00011101", "LCR resets to 0x1D");
    bus_write(b, 3, 0x80);
    bus_write(b, 0, 0x00);
    expect_reg(b, 1, "00000000", "no DREV/DVID: DLL = DLM = 0 reads DLM");
    bus_write(b, 0, 0x01);
    bus_write(b, 3, 0x03);
    bt = bit_time(b, 1);
    e_efr(b, 0x10);
    bus_write(b, 4, 0x40);                 /* MCR[6]: TCR/TLR */
    expect_reg(b, 6, "00001111", "TCR resets to 0x0F");
    bus_write(b, 7, 0x21);                 /* TLR: RX 2 x 4, TX 1 x 4 */
    expect_reg(b, 7, "00100001", "TLR");
    bus_write(b, 4, 0x44);                 /* MCR[2] too: FIFO Rdy wins at 7 */
    bus_write(b, 2, 0x01);
    expect_reg(b, 7, "00000011", "FIFO Rdy over TLR: both TX ready, no RX");
    bus_write(b, 4, 0x00);
    /* DLD[7] IrDA, DLD[6] RS-485 on RTS. */
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x80);
    bus_write(b, 3, 0x03);
    run(b, NS(300));
    expect_pin(b, "TXA", LOGIC_0, "DLD[7]: IrDA, TX idles low");
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x40);
    bus_write(b, 3, 0x03);
    run(b, NS(300));
    expect_pin(b, "RTSA", LOGIC_0, "DLD[6] RS-485: RTS low while idle");
    bus_write(b, 0, 0x55);
    run(b, bt * 3);
    expect_pin(b, "RTSA", LOGIC_1, "transmitting: RTS high");
    t = sim_now(b->sim);
    wait_level(b, "RTSA", LOGIC_0, bt * 20);
    t = sim_now(b->sim) - t;
    check(t >= bt * 7 && t <= bt * 9, "RTS drops a bit after the last stop bit");
    /* Motorola (XR68M752): IRQ not gated by MCR[3] (planner decision). */
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x00);
    bus_write(b, 3, 0x03);
    set(b, "RESET", LOGIC_1);
    set(b, "16/68", LOGIC_0);
    set(b, "CSA", LOGIC_1);
    set(b, "CSB", LOGIC_0);
    run(b, NS(500));
    m_write(b, "CSA", 4, 0x00);
    m_write(b, "CSA", 1, 0x02);
    run(b, NS(300));
    expect_pin(b, "INTA", LOGIC_0, "Motorola IRQ low with MCR[3] = 0");
    expect_pin(b, "INTB", LOGIC_Z, "INTB not used in Motorola mode");
    case_end("E15 XR16M752: m752 TCR/TLR, FIFO Rdy, DLD[7] IrDA, DLD[6] RS-485, Motorola IRQ");
    bench_close(b);
}


int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_e1();
    case_e2();
    case_e3();
    case_e4();
    case_e5_e7("SFX-ST16C2550", "E5 ST16C2550: dual, per-channel CS (broadcast write, X read), "
                                "OP2, timeout over RX data, break hold");
    case_e6();
    case_e5_e7("SFX-XR16C2550", "E7 XR16C2550: the ST16C2550 keys, the same checks");
    case_e8();
    case_e9();
    case_e10();
    case_e11();
    case_e12_e14("SFX-XR16M2551-tqfp48-2551",
                 "E12 XR16M2551: CLKSEL ÷4 and the MCR[7] override, Motorola, PowerSave");
    case_e13();
    case_e12_e14("SFX-XR16M2651-tqfp48-2551",
                 "E14 XR16M2651: the same with 32-byte FIFOs (CLKSEL, Motorola, PowerSave)");
    case_e15();
    return serial_bench_finish();
}
