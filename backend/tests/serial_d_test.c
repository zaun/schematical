/*
 * serial_d_test: the TI parts, D1-D9
 * (split from serial_test.c by plan-68-G; the bench is support/serial_bench.c).
 */

#include "serial_bench.h"
#include "serial_bench_d.h"

/* ---- D1 TL16C550C: auto flow (AFE) ------------------------------------------------------- */

static void case_d1(void) {
    Bench *b = bench_open_cfg("SFX-TL16C550C", HALF_18432, &TI550_BENCH);
    SimTime bt;
    unsigned i;
    char text[9];

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0x41);          /* FIFO, trigger 4 */
    bus_write(b, 4, 0x22);          /* AFE + RTS: auto-RTS and auto-CTS */
    run(b, NS(500));
    expect_pin(b, "RTS", LOGIC_0, "auto-RTS asserts RTS while below the trigger");
    for (i = 0; i < 3; i++) {
        drive_byte(b, 0x41 + i, bt, 1);
    }
    run(b, bt);
    expect_pin(b, "RTS", LOGIC_0, "3 bytes, trigger 4: RTS still active");
    drive_byte(b, 0x44, bt, 1);
    run(b, bt);
    expect_pin(b, "RTS", LOGIC_1, "the 4th byte reaches the trigger: RTS inactive");
    for (i = 0; i < 3; i++) {
        bus_read(b, 0);
    }
    expect_pin(b, "RTS", LOGIC_1, "RTS stays inactive until the FIFO is empty");
    bus_read(b, 0);
    run(b, bt);
    expect_pin(b, "RTS", LOGIC_0, "emptied: RTS active again");
    /* Auto-CTS: CTS inactive holds the transmitter; no ΔCTS meanwhile. */
    bus_read(b, 6);
    set(b, "CTS", LOGIC_1);
    run(b, NS(500));
    bus_write(b, 0, 0x55);
    check(!wait_start(b, bt * 12), "CTS inactive: nothing is sent");
    set(b, "CTS", LOGIC_0);
    check(wait_start(b, bt * 30), "CTS active: the byte goes out");
    bus_read_text(b, 6, text);
    check(text[7] == '0', "no ΔCTS while auto-CTS is on (decisions.md)");
    case_end("D1 TL16C550C afe: auto-RTS at the trigger, auto-CTS holds TX, no ΔCTS");
    bench_close(b);
}

/* ---- D2 TL16C550D: 9-RCLK timeout; BGA without modem inputs ------------------------------- */

static void case_d2(void) {
    Bench *b = bench_open_cfg("SFX-TL16C550D", HALF_18432, &TI550_BENCH);
    SimTime bt;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0x41);
    bus_write(b, 1, 0x01);
    drive_byte(b, 0x61, bt, 1);
    /* The byte lands 3 RCLKs after its stop sample (mid stop bit), and the
       timeout comes 4 characters (40 bits) + 9 RCLKs later: 39.9 bit times
       after drive_byte returns (it ends half a bit after the stop sample). */
    run(b, bt * 39);
    expect_pin(b, "INTRPT", LOGIC_0, "39 bit times after the frame: no timeout yet");
    run(b, bt * 2);
    expect_pin(b, "INTRPT", LOGIC_1, "timeout after 4 characters + 9 RCLKs");
    bench_close(b);
    /* The ZQS (bga24) package has no DSR, RI, DCD: MSR[5:7] read X, deltas 0. */
    check(part_exists("SFX-TL16C550D-bga24"), "the bga24 package fixture (device data)");
    if (part_exists("SFX-TL16C550D-bga24")) {
        b = bench_open_cfg("SFX-TL16C550D-bga24", HALF_18432, &TI550_BGA_BENCH);
        expect_reg(b, 6, "XXX00000", "bga24: MSR DSR/RI/DCD X, CTS inactive, deltas 0");
        program(b, 1, 0x03);
        bus_write(b, 4, 0x10);             /* loopback: the internal RCLK = BAUDOUT */
        bus_write(b, 0, 0x6B);
        run(b, bit_time(b, 1) * 12);
        expect_reg(b, 0, "01101011", "bga24: the receiver runs on the internal BAUDOUT");
        bench_close(b);
    }
    case_end("D2 TL16C550D: timeout delay 9 RCLKs; bga24 without modem inputs");
}

/* ---- D3 TL16C750: 16/64-byte FIFO, rxTrigger64, sleep -------------------------------------- */

static void case_d3(void) {
    Bench *b = bench_open_cfg("SFX-TL16C750", HALF_18432, &TI550_BENCH);
    SimTime bt;
    unsigned i;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0x41);                 /* 16-byte, trigger 4 */
    expect_reg(b, 2, "11000001", "16-byte mode: IIR[5] = 0");
    bus_write(b, 3, 0x83);
    bus_write(b, 2, 0x61);                 /* FCR[5] with LCR[7] = 1: 64-byte, trigger 16 */
    bus_write(b, 3, 0x03);
    expect_reg(b, 2, "11100001", "64-byte mode: IIR[5] = 1");
    bus_write(b, 1, 0x01);
    for (i = 0; i < 15; i++) {
        drive_byte(b, 0x30 + i, bt, 1);
    }
    run(b, bt);
    expect_pin(b, "INTRPT", LOGIC_0, "64-byte mode trigger 01 = 16: 15 bytes no interrupt");
    drive_byte(b, 0x3F, bt, 1);
    run(b, bt);
    expect_pin(b, "INTRPT", LOGIC_1, "16 bytes: interrupt (rxTrigger64)");
    for (i = 0; i < 16; i++) {
        bus_read(b, 0);
    }
    for (i = 0; i < 17; i++) {
        drive_byte(b, 0x40 + i, bt, 1);
    }
    run(b, bt);
    expect_reg(b, 5, "01100001", "64-byte FIFO: 17 bytes, no overrun");
    for (i = 0; i < 17; i++) {
        bus_read(b, 0);
    }
    /* Sleep (IER[4]): idle, the baud clock stops (BAUDOUT X); a THR write wakes it. */
    bus_write(b, 1, 0x10);
    run(b, bt * 2);
    expect_pin(b, "BAUDOUT", LOGIC_X, "asleep: BAUDOUT X (decisions.md)");
    bus_write(b, 0, 0x5A);
    check(wait_start(b, bt * 4), "a THR write wakes it and the byte goes out");
    case_end("D3 TL16C750: 16/64-byte FIFO by FCR[5], rxTrigger64, IER[4] sleep");
    bench_close(b);
}

/* ---- D4 TL16C750E: LCR 0x1D, DLF, TLR x8 --------------------------------------------------- */

static void case_d4(void) {
    Bench *b = bench_open_cfg("SFX-TL16C750E", NS(50), &TI750E_BENCH);
    SimTime t0;
    SimTime t1;
    char message[96];

    case_begin();
    expect_reg(b, 3, "00011101", "LCR resets to 0x1D");
    /* EFR[4] on, then DLL = 1 with DLF = 0x20 (1 + 32/64 = 1.5), 8N1. */
    bus_write(b, 3, 0xBF);
    bus_write(b, 2, 0x10);
    bus_write(b, 3, 0x83);
    bus_write(b, 0, 0x01);
    bus_write(b, 1, 0x00);
    bus_write(b, 7, 0x20);
    expect_reg(b, 7, "00100000", "DLF reads back");
    bus_write(b, 3, 0x03);
    bus_write(b, 0, 0x00);   /* 0x00: start + 8 zeros, then the stop's rising edge */
    check(wait_level(b, "TX", LOGIC_0, NS(100000)), "the byte starts");
    t0 = sim_now(b->sim);
    while (get(b, "TX") == LOGIC_0) {
        run(b, NS(10));
    }
    t1 = sim_now(b->sim);
    /* 9 bits of 1.5 x 16 clocks (100 ns) = 21.6 us. */
    snprintf(message, sizeof message, "9 bits with DLF 32/64: %llu ns, want about 21600",
             (unsigned long long)((t1 - t0) / 1000));
    check(t1 - t0 > NS(21000) && t1 - t0 < NS(22200), message);
    /* TLR x 8: RX nibble 1 -> trigger 8. */
    bus_write(b, 4, 0x40);                 /* MCR[6]: TCR/TLR */
    bus_write(b, 7, 0x10);
    expect_reg(b, 7, "00010000", "TLR reads back at 7 with MCR[6]");
    /* MODE low: IOW-only (E's "intel-motorola" style). CS is the strobe and
       IOW at CS's fall picks write (low) or read (high); IOR is tied high and
       RESET stays active high (p32). */
    set(b, "CS", LOGIC_1);
    set(b, "IOR", LOGIC_1);
    set(b, "MODE", LOGIC_0);
    run(b, NS(500));
    {
        static const unsigned addr_val[2][2] = {{4, 0x00}, {7, 0x5A}};   /* MCR off, SCR */
        unsigned i;

        for (i = 0; i < 2; i++) {
            set_address(b, addr_val[i][0]);
            set_data(b, 1, addr_val[i][1]);
            set(b, "IOW", LOGIC_0);
            run(b, NS(80));
            set(b, "CS", LOGIC_0);
            run(b, NS(120));
            set(b, "CS", LOGIC_1);
            run(b, NS(100));
            set_data(b, 0, 0);
            set(b, "IOW", LOGIC_1);
            run(b, NS(160));
        }
    }
    {
        char text[9];
        unsigned i;

        set_address(b, 7);
        set(b, "IOW", LOGIC_1);
        run(b, NS(80));
        set(b, "CS", LOGIC_0);
        run(b, NS(150));
        for (i = 0; i < 8; i++) {
            text[7 - i] = "01XZ"[get(b, b->cfg->data[i])];
        }
        text[8] = '\0';
        set(b, "CS", LOGIC_1);
        run(b, NS(160));
        snprintf(message, sizeof message, "IOW-only mode: SCR written and read by CS: %s", text);
        check(strcmp(text, "01011010") == 0, message);
    }
    expect_pin(b, "D0", LOGIC_Z, "IOW-only mode: the bus floats after the CS pulse");
    case_end("D4 TL16C750E: LCR 0x1D, DLF fraction n/64, TLR access, IOW-only MODE");
    bench_close(b);
}

/* ---- D5 TL16C754B: 752 bank, TCR auto-RTS, Xon/Xoff, INTSEL, CLKSEL --------------------- */

static void case_d5(void) {
    Bench *b = bench_open_cfg("SFX-TL16C754B", NS(50), &QUAD_CD_BENCH);
    SimTime bt;
    unsigned i;

    case_begin();
    expect_reg(b, 4, "00000000", "CLKSEL = 1: MCR[7] = 0 after reset");
    expect_pin(b, "INTA", LOGIC_Z, "INTSEL = 0, MCR[3] = 0: INTA 3-state");
    set(b, "INTSEL", LOGIC_1);
    run(b, NS(500));
    expect_pin(b, "INTA", LOGIC_0, "INTSEL = 1: INTA driven");
    bus_write(b, 3, 0xBF);
    bus_write(b, 2, 0x50);                 /* EFR: enhanced + auto-RTS */
    bus_write(b, 4, 0x01);                 /* Xon1 = 0x01 (bank address 4) */
    bus_write(b, 6, 0x13);                 /* Xoff1 = 0x13 */
    bus_write(b, 3, 0x03);
    bus_write(b, 4, 0x42);                 /* MCR[6] (TCR/TLR) + RTS */
    bus_write(b, 6, 0x12);                 /* TCR: halt 2 x 4 = 8, resume 1 x 4 = 4 */
    bus_write(b, 4, 0x02);
    bus_write(b, 3, 0x83);
    bus_write(b, 0, 0x01);
    bus_write(b, 1, 0x00);
    bus_write(b, 3, 0x03);
    bus_write(b, 2, 0x01);
    bt = bit_time(b, 1);
    run(b, NS(500));
    expect_pin(b, "RTSA", LOGIC_0, "auto-RTS active below the halt level");
    for (i = 0; i < 8; i++) {
        drive_named(b, "RXA", 0x30 + i, bt);
    }
    run(b, bt * 2);
    expect_pin(b, "RTSA", LOGIC_1, "8 bytes reach TCR halt 8: RTS inactive");
    for (i = 0; i < 4; i++) {
        bus_read(b, 0);
    }
    run(b, bt);
    expect_pin(b, "RTSA", LOGIC_0, "down to TCR resume 4: RTS active again");
    for (i = 0; i < 4; i++) {
        bus_read(b, 0);
    }
    /* Xoff1 received halts TX (EFR[1:0] = 10); Xon1 resumes. */
    bus_write(b, 3, 0xBF);
    bus_write(b, 2, 0x12);
    bus_write(b, 3, 0x03);
    drive_named(b, "RXA", 0x13, bt);
    run(b, bt);
    bus_write(b, 0, 0x77);
    check(!wait_level(b, "TXA", LOGIC_0, bt * 12), "after Xoff1 nothing is sent");
    expect_reg(b, 5, "00000000", "the Xoff is not stored (DR 0); the byte waits (THRE, TEMT 0)");
    drive_named(b, "RXA", 0x01, bt);
    check(wait_level(b, "TXA", LOGIC_0, bt * 12), "Xon1 resumes the transmitter");
    case_end("D5 TL16C754B: TCR auto-RTS, Xon/Xoff, INTSEL, CLKSEL, 752 bank");
    bench_close(b);
}

/* ---- D6 TL16C754C: trigger table [1,4,56,60]; Xon power-up values ------------------------ */

static void case_d6(void) {
    Bench *b = bench_open_cfg("SFX-TL16C754C", NS(50), &QUAD_C_BENCH);
    SimTime bt;

    case_begin();
    bus_write(b, 3, 0xBF);
    expect_reg(b, 4, "11111111", "Xon1 powers up 0xFF");
    expect_reg(b, 5, "11101111", "Xon2 powers up 0xEF");
    expect_reg(b, 7, "11111111", "Xoff2 powers up 0xFF");
    bus_write(b, 3, 0x83);
    bus_write(b, 0, 0x01);
    bus_write(b, 1, 0x00);
    bus_write(b, 3, 0x03);
    expect_reg(b, 7, "11111111", "SPR powers up 0xFF");
    bus_write(b, 2, 0x41);                 /* trigger 01 = 4 */
    bus_write(b, 1, 0x01);
    bus_write(b, 4, 0x08);                 /* MCR[3]: INTA enabled */
    bt = bit_time(b, 1);
    drive_named(b, "RXA", 1, bt);
    drive_named(b, "RXA", 2, bt);
    drive_named(b, "RXA", 3, bt);
    run(b, bt);
    expect_pin(b, "INTA", LOGIC_0, "3 bytes, trigger 4: no interrupt");
    drive_named(b, "RXA", 4, bt);
    run(b, bt);
    expect_pin(b, "INTA", LOGIC_1, "4 bytes: RX data interrupt");
    case_end("D6 TL16C754C: rxTrigger [1,4,56,60]; Xon/SPR power-up values");
    bench_close(b);
}

/* ---- D7 TL16C752C: AFR CONC, IrDA pulses, RS-485 DTR ---------------------------------------- */

static void case_d7(void) {
    Bench *b = bench_open_cfg("SFX-TL16C752C", NS(50), &DUAL_CD_BENCH);
    SimTime bt;
    SimTime t0;
    SimTime t1;
    char message[96];

    case_begin();
    /* CONC: a write to channel A reaches B. */
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x01);                 /* LCR[7:5] = 100: AFR, CONC */
    bus_write(b, 3, 0x80);
    bus_write(b, 0, 0x01);
    bus_write(b, 1, 0x00);
    bus_write(b, 3, 0x03);
    bus_write(b, 7, 0x5A);
    select_unit(b, 1);
    expect_reg(b, 7, "01011010", "CONC: channel B got the SPR write");
    expect_reg(b, 3, "00000011", "and the LCR write");
    select_unit(b, 0);
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x00);
    bus_write(b, 3, 0x03);
    bt = bit_time(b, 1);
    /* IrDA (AFR[1]): a 0 bit is a 3/16 pulse; idle low. */
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x12);                 /* RCVEN + IREN */
    bus_write(b, 3, 0x03);
    expect_pin(b, "TXA", LOGIC_0, "IrDA: TX idles low");
    bus_write(b, 0, 0xFE);                 /* start 0, bit0 0 */
    check(wait_level(b, "TXA", LOGIC_1, bt * 4), "a light pulse for the start bit");
    t0 = sim_now(b->sim);
    while (get(b, "TXA") == LOGIC_1) {
        run(b, NS(10));
    }
    t1 = sim_now(b->sim);
    snprintf(message, sizeof message, "pulse %llu ns, want 3/16 bit (%llu)",
             (unsigned long long)((t1 - t0) / 1000), (unsigned long long)(bt * 3 / 16 / 1000));
    check(t1 - t0 + NS(250) >= bt * 3 / 16 && t1 - t0 <= bt * 3 / 16 + NS(250), message);
    run(b, bt * 12);
    /* RS-485 (AFR[2]) with DLY 3: DTR high while sending and 3 bits after. */
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x74);                 /* DLY 3, RCVEN, 485EN */
    bus_write(b, 3, 0x03);
    expect_pin(b, "DTRA", LOGIC_0, "RS-485 idle: DTR low");
    bus_write(b, 0, 0x00);
    check(wait_level(b, "DTRA", LOGIC_1, bt * 2), "DTR rises before the start bit");
    check(wait_level(b, "TXA", LOGIC_0, bt * 3), "then the start bit");
    t0 = sim_now(b->sim);
    check(wait_level(b, "DTRA", LOGIC_0, bt * 20), "DTR falls after the frame");
    t1 = sim_now(b->sim);
    snprintf(message, sizeof message, "DTR low %llu bit times after the start, want 10 + 3",
             (unsigned long long)((t1 - t0) / bt));
    check(t1 - t0 >= bt * 12 && t1 - t0 <= bt * 14, message);
    case_end("D7 TL16C752C: AFR CONC, IrDA 3/16 pulses, RS-485 DTR with DLY 3");
    bench_close(b);
}

/* ---- D8 TL16C554: four channels, shared AND DMA pins, INTN --------------------------------- */

static void case_d8_d9(const char *part, const char *name) {
    Bench *b = bench_open_cfg(part, HALF_18432, &QUAD_BENCH);
    unsigned u;
    char text[9];

    case_begin();
    for (u = 0; u < 4; u++) {
        select_unit(b, u);
        bus_write(b, 7, 0x10 + u);
    }
    for (u = 0; u < 4; u++) {
        select_unit(b, u);
        bus_read_text(b, 7, text);
        check(strtol(text, NULL, 2) == (long)(0x10 + u), "each channel's own SPR");
    }
    expect_pin(b, "TXRDY", LOGIC_0, "shared TXRDY: low (all empty)");
    select_unit(b, 2);
    bus_write(b, 0, 0x55);
    expect_pin(b, "TXRDY", LOGIC_0, "one channel loaded: the AND stays low (others empty)");
    /* INTN low: MCR[3] gates; INTN high: always driven. */
    expect_pin(b, "INTA", LOGIC_Z, "INTN low, MCR[3] = 0: INTA 3-state");
    set(b, "INTN", LOGIC_1);
    run(b, NS(500));
    expect_pin(b, "INTA", LOGIC_0, "INTN high: INTA driven");
    /* A byte received with no bus access moves the shared RXRDY (it follows
       the channels, not only the cycles): channel 0 in loopback, divisor 1. */
    select_unit(b, 0);
    bus_write(b, 3, 0x80);
    bus_write(b, 0, 0x01);
    bus_write(b, 1, 0x00);
    bus_write(b, 3, 0x03);
    bus_write(b, 4, 0x10);
    expect_pin(b, "RXRDY", LOGIC_1, "shared RXRDY: high, nothing received");
    bus_write(b, 0, 0x41);
    run(b, NS(150000));   /* one 8N1 frame at divisor 1: 160 clocks of 542.5 ns = 87 us */
    expect_pin(b, "RXRDY", LOGIC_0, "shared RXRDY: low once channel 0 holds a byte");
    case_end(name);
    bench_close(b);
}


int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_d1();
    case_d2();
    case_d3();
    case_d4();
    case_d5();
    case_d6();
    case_d7();
    case_d8_d9("SFX-TL16C554", "D8 TL16C554: four channels, shared AND TXRDY, INTN both levels");
    case_d8_d9("SFX-TL16C554A", "D9 TL16C554A: the same on the AFE part");
    return serial_bench_finish();
}
