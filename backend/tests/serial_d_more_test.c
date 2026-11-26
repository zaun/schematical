/*
 * serial_d_more_test: the TI and other 16550 parts D10-D24, and the shipped-device comparison
 * (split from serial_test.c by plan-68-G; the bench is support/serial_bench.c).
 */

#include "serial_bench.h"
#include "serial_bench_d.h"

/* ---- D10/D11 TL16C552(A): two channels + printer port --------------------------------------- */

static void case_d10_d11(const char *part, const char *name) {
    Bench *b = bench_open_cfg(part, HALF_18432, &TI552_BENCH);
    char text[9];

    case_begin();
    expect_reg(b, 5, "01100000", "RESET is active low: released, LSR 0x60");
    select_unit(b, 2);                     /* the printer port */
    bus_write(b, 0, 0xA5);
    expect_pin(b, "PD0", LOGIC_1, "PEMD low: PD drives the latch");
    expect_pin(b, "PD1", LOGIC_0, "PD1");
    bus_write(b, 2, 0x01);                 /* STB asserted */
    expect_pin(b, "STB", LOGIC_0, "control bit 0 asserts STB low");
    bus_read_text(b, 1, text);
    check(text[0] == '1' && text[5] == '1' && text[6] == '1' && text[7] == '1',
          "status: BSY = NOT BUSY, PRINT = 1, b1:0 = 1");
    bus_write(b, 2, 0x10);                 /* INT2 EN, ENIRQ low: INT2 = ACK */
    expect_pin(b, "INT2", LOGIC_1, "INT2 follows ACK (high)");
    set(b, "ACK", LOGIC_0);
    run(b, NS(300));
    expect_pin(b, "INT2", LOGIC_0, "INT2 follows ACK (low)");
    set(b, "ACK", LOGIC_1);
    /* TRI floats every output. */
    set(b, "TRI", LOGIC_1);
    run(b, NS(300));
    expect_pin(b, "SOUT0", LOGIC_Z, "TRI high: SOUT0 floats");
    expect_pin(b, "PD0", LOGIC_Z, "TRI high: PD floats");
    set(b, "TRI", LOGIC_0);
    run(b, NS(300));
    expect_pin(b, "SOUT0", LOGIC_1, "TRI low: SOUT0 driven again");
    case_end(name);
    bench_close(b);
}

/* ---- D12/D13 TL16C451/452: 16450 channels, printer port, single CLK, OUT2 gating -------------- */

static void case_d12_d13(const char *part, const BenchConfig *cfg, const char *name) {
    Bench *b = bench_open_cfg(part, HALF_18432, cfg);
    SimTime bt;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    expect_reg(b, 2, "00000001", "16450 channel: IIR 01");
    bus_write(b, 1, 0x02);                 /* ETBEI: THRE interrupt */
    expect_pin(b, "INT0", LOGIC_Z, "MCR[3] = 0: INT0 3-state");
    bus_write(b, 4, 0x08);
    run(b, NS(500));
    expect_pin(b, "INT0", LOGIC_1, "MCR[3] = 1: INT0 driven high (THRE)");
    bus_write(b, 0, 0x41);
    check(wait_level(b, "SOUT0", LOGIC_0, bt * 3), "the single CLK clocks the transmitter");
    /* Printer port: LPTOE low drives PD; high makes them inputs. */
    select_unit(b, cfg->units - 1);
    bus_write(b, 0, 0x3C);
    expect_pin(b, "PD2", LOGIC_1, "LPTOE low: PD drives the latch");
    set(b, "LPTOE", LOGIC_1);
    run(b, NS(100));
    expect_pin(b, "PD2", LOGIC_Z, "LPTOE high: the PD pins float");
    set(b, "PD2", LOGIC_0);
    run(b, NS(300));
    check((bus_read(b, 0) & 0x04) == 0, "LPTOE high: the data register reads the PD pins");
    case_end(name);
    bench_close(b);
}

/* ---- D14 TL16C450, D20 82C50A ----------------------------------------------------------------- */

static void case_d14(void) {
    Bench *b = bench_open_cfg("SFX-TL16C450", NS(50), &TI450_BENCH);
    SimTime t0;
    LogicValue last;

    case_begin();
    expect_pin(b, "INTRPT", LOGIC_0, "INTRPT driven low after MR (no OUT2 gating)");
    expect_pin(b, "CSOUT", LOGIC_1, "CSOUT high while selected");
    set(b, "CS0", LOGIC_0);
    run(b, NS(200));
    expect_pin(b, "CSOUT", LOGIC_0, "CSOUT low when not selected");
    set(b, "CS0", LOGIC_1);
    bus_write(b, 3, 0x80);
    bus_write(b, 0, 0x05);
    bus_write(b, 1, 0x00);
    /* BAUDOUT low for 2 clocks, high for 3 (divisor 5). */
    last = get(b, "BAUDOUT");
    while (!(last == LOGIC_1 && get(b, "BAUDOUT") == LOGIC_0)) {
        last = get(b, "BAUDOUT");
        run(b, NS(5));
    }
    t0 = sim_now(b->sim);
    while (get(b, "BAUDOUT") == LOGIC_0) {
        run(b, NS(5));
    }
    check(sim_now(b->sim) - t0 >= NS(195) && sim_now(b->sim) - t0 <= NS(210),
          "BAUDOUT low for 2 XTAL1 cycles");
    expect_reg(b, 2, "00000001", "16450: no FIFO bits");
    case_end("D14 TL16C450: driven INTRPT, CSOUT, BAUDOUT low for 2 clocks, no FIFO");
    bench_close(b);
}

static void case_d20(void) {
    Bench *b = bench_open_cfg("SFX-82C50A", HALF_18432, &TI450_BENCH);
    SimTime bt;
    char bits[16];

    case_begin();
    program(b, 1, 0x07);                   /* 8 bits, 2 stops */
    bt = bit_time(b, 1);
    run(b, bt * 2);                        /* the receiver sees the idle line first */
    /* Second stop bit 0: a framing error on the 82C50A (both stops checked). */
    strcpy(bits, "0100000001");
    drive_bits(b, bits, bt);
    set(b, "SIN", LOGIC_0);
    run(b, bt);
    set(b, "SIN", LOGIC_1);
    run(b, bt * 3);
    expect_reg(b, 5, "01101001", "82C50A: a 0 second stop bit is a framing error");
    bus_read(b, 0);
    expect_pin(b, "INTRPT", LOGIC_0, "INTRPT driven (no OUT2 gating)");
    case_end("D20 82C50A: both stop bits checked, deferred status, driven INTRPT");
    bench_close(b);
}

/* ---- D15 TL16C550B: DMA class B ---------------------------------------------------------------- */

static void case_d15(void) {
    Bench *b = bench_open_cfg("SFX-TL16C550B", HALF_18432, &TI550_BENCH);
    unsigned i;
    SimTime bt;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0x09);                 /* FIFO + DMA mode 1 */
    /* The first byte moves straight to the shifter: 17 writes fill the FIFO. */
    for (i = 0; i < 17; i++) {
        bus_write(b, 0, 0x30 + i);
    }
    run(b, NS(300));
    expect_pin(b, "TXRDY", LOGIC_1, "mode 1: TXRDY high when the FIFO is full");
    check(wait_level(b, "TXRDY", LOGIC_0, bt * 12), "TXRDY low again once one slot frees");
    case_end("D15 TL16C550B: DMA class B (TXRDY back as soon as a slot is free)");
    bench_close(b);
}

/* ---- D16 TL16C2550: per-channel chip selects --------------------------------------------------- */

static void case_d16(void) {
    Bench *b = bench_open_cfg("SFX-TL16C2550", HALF_18432, &DUAL_CD_BENCH);
    char text[9];

    case_begin();
    bus_write(b, 7, 0xA1);
    select_unit(b, 1);
    bus_write(b, 7, 0xB2);
    bus_read_text(b, 7, text);
    check(strcmp(text, "10110010") == 0, "channel B's SPR");
    select_unit(b, 0);
    bus_read_text(b, 7, text);
    check(strcmp(text, "10100001") == 0, "channel A's SPR");
    /* Both CS low: no broadcast on this part: a read is X. */
    set(b, "CSA", LOGIC_0);
    set(b, "CSB", LOGIC_0);
    run(b, NS(100));
    bus_read_text(b, 7, text);
    check(strcmp(text, "XXXXXXXX") == 0, "both CS low: the read is X (P2)");
    case_end("D16 TL16C2550: per-channel chip selects");
    bench_close(b);
}

/* ---- D17 TL16C2552, D18 NS16C2552, D19 NS16C2752: channel pin, AFR, MF -------------------------- */

static void case_d17(void) {
    Bench *b = bench_open_cfg("SFX-TL16C2552", HALF_18432, &CHSEL_BENCH);
    char text[9];

    case_begin();
    expect_reg(b, 4, "00001000", "MCR resets to 0x08 (decisions.md): INT driven");
    bus_write(b, 7, 0x11);
    select_unit(b, 1);
    bus_write(b, 7, 0x22);
    select_unit(b, 0);
    bus_read_text(b, 7, text);
    check(strcmp(text, "00010001") == 0, "CHSEL = 1 is channel A");
    /* AFR (DLAB = 1, address 2): MF select 10 = RXRDY on MFA. */
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x04);
    bus_write(b, 3, 0x03);
    run(b, NS(300));
    expect_pin(b, "MFA", LOGIC_1, "MFA = RXRDY (inactive, empty)");
    case_end("D17 TL16C2552: channelSelect, AFR MF select, MCR reset 0x08");
    bench_close(b);
}

static void case_d18_d19(const char *part, const char *name, unsigned trigger_index,
                         unsigned rts_off, unsigned drev) {
    Bench *b = bench_open_cfg(part, HALF_18432, &NS_BENCH);
    SimTime bt;
    unsigned i;
    char text[9];

    case_begin();
    expect_reg(b, 7, "11111111", "SCR resets to 0xFF");
    bus_write(b, 3, 0x80);
    bus_write(b, 0, 0x00);
    bus_write(b, 1, 0x00);
    bus_read_text(b, 0, text);
    check(strtol(text, NULL, 2) == (long)drev, "DREV reads with DLL = DLM = 0");
    bus_write(b, 0, 0x01);
    bus_write(b, 3, 0xBF);
    bus_write(b, 2, 0x50);                 /* EFR: enhanced + auto-RTS */
    bus_write(b, 3, 0x03);
    bus_write(b, 4, 0x02);                 /* MCR[1]: needed for auto-RTS */
    bus_write(b, 2, 0x01 | (trigger_index << 6));
    bt = bit_time(b, 1);
    run(b, NS(300));
    expect_pin(b, "RTS1", LOGIC_0, "auto-RTS active");
    for (i = 0; i + 1 < rts_off; i++) {
        drive_named(b, "SIN1", 0x40, bt);
    }
    run(b, bt);
    expect_pin(b, "RTS1", LOGIC_0, "one below the table's off level: still active");
    drive_named(b, "SIN1", 0x41, bt);
    run(b, bt);
    expect_pin(b, "RTS1", LOGIC_1, "the table's off level: RTS inactive");
    /* lsrClearOnGood: a framing error, the byte read out (the LSR not read),
       then a good byte loading clears FE without an LSR read. */
    bus_write(b, 2, 0x06);                 /* clear the FIFOs, then FIFOs off */
    bus_write(b, 2, 0x00);
    bus_read(b, 5);
    drive_bits_on(b, "SIN1", "0110000000", bt);   /* 0x03, stop 0 */
    set(b, "SIN1", LOGIC_1);
    run(b, bt * 3);
    bus_read(b, 0);
    drive_named(b, "SIN1", 0x55, bt);
    run(b, bt);
    check((bus_read(b, 5) & 0x1E) == 0, "a good byte loading cleared FE (NS rule)");
    case_end(name);
    bench_close(b);
}

/* ---- D21 COM92C451 ------------------------------------------------------------------------------- */

static void case_d21(void) {
    Bench *b = bench_open_cfg("SFX-COM92C451", NS(50), &COM92_BENCH);
    SimTime t0;
    SimTime t1;

    case_begin();
    bus_write(b, 3, 0x80);
    bus_write(b, 0, 0x00);
    bus_write(b, 1, 0x00);
    bus_write(b, 3, 0x03);
    bus_write(b, 0, 0x00);
    check(wait_start(b, NS(100000)), "divisor 0 still clocks (as 3)");
    t0 = sim_now(b->sim);
    while (get(b, "SOUT") == LOGIC_0) {
        run(b, NS(10));
    }
    t1 = sim_now(b->sim);
    check(t1 - t0 > NS(43000) && t1 - t0 < NS(43500), "9 bits x 16 x 3 clocks of 100 ns = 43.2 us");
    /* PIA: control bit 4 enables IRQ0; ACK falling latches it; a status read clears. */
    select_unit(b, 1);
    expect_pin(b, "IRQ0", LOGIC_Z, "IRQ0 3-state while disabled");
    bus_write(b, 2, 0x10);
    expect_pin(b, "IRQ0", LOGIC_0, "enabled, no ACK yet");
    set(b, "ACK", LOGIC_0);
    run(b, NS(300));
    set(b, "ACK", LOGIC_1);
    run(b, NS(300));
    expect_pin(b, "IRQ0", LOGIC_1, "ACK falling latches IRQ0");
    bus_read(b, 1);
    run(b, NS(300));
    expect_pin(b, "IRQ0", LOGIC_0, "a status read clears IRQ0");
    expect_pin(b, "INIT", LOGIC_0, "control bit 2 = 0: INIT asserted low (reset state)");
    case_end("D21 COM92C451: divisor 0 = 3, PIA IRQ0, INIT after reset");
    bench_close(b);
}

/* ---- D22 SC16C650B, D23 SC16C752B ---------------------------------------------------------------- */

static void case_d22(void) {
    Bench *b = bench_open_cfg("SFX-SC16C650B", HALF_18432, &NXP650_BENCH);
    SimTime bt;
    unsigned i;

    case_begin();
    expect_reg(b, 7, "11111111", "SPR powers up 0xFF");
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 3, 0xBF);
    bus_write(b, 2, 0x10);                 /* EFR[4] */
    bus_write(b, 3, 0x03);
    bus_write(b, 4, 0x20);                 /* MCR[5]: INT open source */
    bus_write(b, 1, 0x01);
    run(b, NS(300));
    expect_pin(b, "INT", LOGIC_Z, "open source, no interrupt: INT floats");
    bus_write(b, 2, 0x01);                 /* FIFO, trigger 8 */
    for (i = 0; i < 8; i++) {
        drive_named(b, "RX", 0x30, bt);
    }
    run(b, bt);
    expect_pin(b, "INT", LOGIC_1, "trigger 8: INT driven high");
    /* LSR[7] clears on any LSR read (lsr7Clear: lsrRead). */
    drive_bits_on(b, "RX", "0100011000", bt);   /* 0x31 with stop 0 */
    set(b, "RX", LOGIC_1);
    run(b, bt * 3);
    check((bus_read(b, 5) & 0x80) != 0, "an errored byte: LSR[7]");
    check((bus_read(b, 5) & 0x80) == 0, "the LSR read cleared LSR[7]");
    /* DLAB hides the upper registers. */
    bus_write(b, 3, 0x83);
    expect_reg(b, 7, "XXXXXXXX", "LCR[7] = 1: SPR not reachable (X)");
    bus_write(b, 3, 0x03);
    case_end("D22 SC16C650B: 650 bank, open-source INT, LSR[7] on read, DLAB hides");
    bench_close(b);
}

static void case_d23(void) {
    Bench *b = bench_open_cfg("SFX-SC16C752B", NS(50), &DUAL_SYNC_BENCH);
    SimTime bt;

    case_begin();
    expect_reg(b, 3, "00011101", "LCR resets to 0x1D");
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 3, 0xBF);
    bus_write(b, 2, 0x10);
    bus_write(b, 3, 0x03);
    bus_write(b, 4, 0x44);                 /* MCR[6] TCR/TLR, MCR[2] FIFO Rdy */
    expect_reg(b, 7, "XXXXXXXX", "TLR and FIFO Rdy both qualify: X (decisions.md)");
    bus_write(b, 4, 0x04);
    bus_write(b, 2, 0x01);
    expect_reg(b, 7, "00000011", "FIFO Rdy: both TX FIFOs ready, RX empty");
    /* IrDA (MCR[6] is also TLR access; use MCR = 0x40 only with EFR[4]). */
    bus_write(b, 4, 0x40);
    run(b, NS(300));
    expect_pin(b, "TXA", LOGIC_0, "IrDA on: TX idles low");
    bus_write(b, 4, 0x00);
    (void)bt;
    case_end("D23 SC16C752B: LCR 0x1D, 752 decode X rule, FIFO Rdy, IrDA");
    bench_close(b);
}

/* ---- D24 PC16550D: C's cases on the shipped device ------------------------------------------------ */

static void case_shipped(void) {
    char path[512];
    char *ns_text = read_fixture("ns16550.json");
    char *bridge_text = read_fixture("bridges.json");
    char *shipped_text;
    char message[160];

    case_begin();
    snprintf(path, sizeof path, "%s/serial.json", PARTS_DIR);
    shipped_text = read_file(path);
    check(ns_text != NULL && bridge_text != NULL, "the fixture files");
    check(shipped_text != NULL, "Parts/serial.json (shipped by plan-12-F onwards)");
    if (ns_text != NULL && bridge_text != NULL && shipped_text != NULL) {
        cJSON *shipped = cJSON_Parse(shipped_text);
        int ns = compare_shipped(ns_text, shipped, "ns16550");
        int bridges = compare_shipped(bridge_text, shipped, "bridges");
        int classic = compare_shipped(SERIAL_FIXTURE_CLASSIC, shipped, "classic");
        int maxim = compare_shipped(SERIAL_FIXTURE_MAXIM, shipped, "maxim");

        printf("   compared: %d ns16550, %d bridges, %d classic, %d maxim\n", ns, bridges,
               classic, maxim);
        snprintf(message, sizeof message, "%d ns16550 fixture devices found in serial.json", ns);
        check(ns > 0, message);
        cJSON_Delete(shipped);
    }
    free(ns_text);
    free(bridge_text);
    free(shipped_text);
    case_end("shipped: the fixture devices are the shipped device objects");
}

int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_d10_d11("SFX-TL16C552", "D10 TL16C552: printer port (PEMD, STB, status, INT2), TRI");
    case_d10_d11("SFX-TL16C552A", "D11 TL16C552A: the same");
    case_d12_d13("SFX-TL16C451", &TI451_BENCH,
                 "D12 TL16C451: 16450 channel, single CLK, OUT2-gated INT, printer port");
    case_d12_d13("SFX-TL16C452", &TI45X_BENCH,
                 "D13 TL16C452: two 16450 channels, printer port with LPTOE");
    case_d14();
    case_d15();
    case_d16();
    case_d17();
    case_d18_d19("SFX-NS16C2552", "D18 NS16C2552: channel pin, DREV 0x31, RTS table (trigger 4: off at 8)",
                 1, 8, 0x31);
    case_d18_d19("SFX-NS16C2752", "D19 NS16C2752: DREV 0x21, RTS table (trigger 16: off at 56)",
                 1, 56, 0x21);
    case_d20();
    case_d21();
    case_d22();
    case_d23();
    case_shipped();
    return serial_bench_finish();
}
