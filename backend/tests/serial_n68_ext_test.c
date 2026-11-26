/*
 * serial_n68_ext_test (plan-68-N): the ns16550 enhanced features
 * (models/ns16550_ext.c) on the shipped fixture devices — the 650/752/NS-EFR
 * register banks, software flow control (sending and receiving Xon/Xoff),
 * auto-RTS/CTS, IrDA, RS-485, the prescaler, DLF and sleep. Each case cites
 * the spec section (planning/plan-12-data/specs/) its expectations come from.
 */

#include "serial_bench_exar.h"
#include "serial_bench_n68.h"

/* Writes one register of the LCR = 0xBF bank, then sets LCR back to `lcr`. */
static void bank_write(Bench *b, unsigned a, unsigned v, unsigned lcr) {
    bus_write(b, 3, 0xBF);
    bus_write(b, a, v);
    bus_write(b, 3, lcr);
}

/* ---- SC16C650B (exar-d.md § SC16C650B) ---------------------------------------------------- */

/* Registers at LCR = 0xBF: EFR, Xon1/2, Xoff1/2 read back (Table 3 p9). */
static void case_650_bank(void) {
    Bench *b = bench_open_cfg("SFX-SC16C650B", HALF_18432, &NXP650_BENCH);

    case_begin();
    bus_write(b, 3, 0xBF);
    bus_write(b, 2, 0x1A);
    bus_write(b, 6, 0x93);
    bus_write(b, 7, 0x94);
    expect_reg(b, 2, "00011010", "EFR reads back at LCR = 0xBF");
    expect_reg(b, 6, "10010011", "Xoff1 reads back");
    expect_reg(b, 7, "10010100", "Xoff2 reads back");
    bus_write(b, 3, 0x03);
    expect_reg(b, 3, "00000011", "LCR 0x03 after the bank");
    case_end("X1 SC16C650B: EFR and Xoff1/2 read back at LCR = 0xBF");
    bench_close(b);
}

/* Auto Xoff/Xon (p10-11, Table 4 p10): trigger 8 sends Xoff at 8 received
   characters and Xon at 0; EFR[3:2] picks Xon1/Xoff1 (10), Xon2/Xoff2 (01)
   or both (11). */
static void flow_650(unsigned efr_tx, unsigned *want, unsigned n_want, unsigned *want_on,
                     unsigned n_on, const char *name) {
    Bench *b = bench_open_cfg("SFX-SC16C650B", HALF_18432, &NXP650_BENCH);
    SimTime bt;
    char what[160];
    unsigned i;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 3, 0xBF);
    bus_write(b, 4, 0x11);   /* Xon1 */
    bus_write(b, 5, 0x12);   /* Xon2 */
    bus_write(b, 6, 0x13);   /* Xoff1 */
    bus_write(b, 7, 0x14);   /* Xoff2 */
    bus_write(b, 2, 0x10 | efr_tx);
    bus_write(b, 3, 0x03);
    bus_write(b, 2, 0x01);   /* FIFO on, RX trigger 8 */
    for (i = 0; i < 7; i++) {
        drive_named(b, "RX", 0x40 + i, bt);
    }
    run(b, bt * 12);
    check(!n68_sent_has(b, 0, 0, want[0]), "7 received: no Xoff yet");
    drive_named(b, "RX", 0x47, bt);
    run(b, bt * 24);
    for (i = 0; i < n_want; i++) {
        snprintf(what, sizeof what, "%s: Xoff 0x%02X sent at 8 received", name, want[i]);
        check(n68_sent_has(b, 0, 0, want[i]), what);
    }
    for (i = 0; i < 8; i++) {
        bus_read(b, 0);
    }
    run(b, bt * 24);
    for (i = 0; i < n_on; i++) {
        snprintf(what, sizeof what, "%s: Xon 0x%02X sent with the FIFO empty", name, want_on[i]);
        check(n68_sent_has(b, 0, 0, want_on[i]), what);
    }
    snprintf(what, sizeof what, "X2 SC16C650B: auto Xoff/Xon sending, %s", name);
    case_end(what);
    bench_close(b);
}

static void case_650_flow(void) {
    unsigned off1[] = {0x13};
    unsigned on1[] = {0x11};
    unsigned off2[] = {0x14};
    unsigned on2[] = {0x12};
    unsigned off12[] = {0x13, 0x14};
    unsigned on12[] = {0x11, 0x12};

    flow_650(0x08, off1, 1, on1, 1, "EFR[3:2] = 10 (Xon1/Xoff1)");
    flow_650(0x04, off2, 1, on2, 1, "EFR[3:2] = 01 (Xon2/Xoff2)");
    flow_650(0x0C, off12, 2, on12, 2, "EFR[3:2] = 11 (both)");
}

/* An Xoff went out; EFR's TX mode then changes: the owed Xon of the old mode
   still goes out (flow chars resume the far end, p10-11). */
static void case_650_owed_xon(void) {
    Bench *b = bench_open_cfg("SFX-SC16C650B", HALF_18432, &NXP650_BENCH);
    SimTime bt;
    size_t before;
    unsigned i;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 3, 0xBF);
    bus_write(b, 4, 0x11);
    bus_write(b, 6, 0x13);
    bus_write(b, 2, 0x18);   /* enhanced, TX Xon1/Xoff1 */
    bus_write(b, 3, 0x03);
    bus_write(b, 2, 0x01);
    for (i = 0; i < 8; i++) {
        drive_named(b, "RX", 0x30 + i, bt);
    }
    run(b, bt * 24);
    check(n68_sent_has(b, 0, 0, 0x13), "Xoff1 sent at 8 received");
    before = n68_sent_count(b, 0);
    bank_write(b, 2, 0x10, 0x03);   /* software flow off */
    run(b, bt * 24);
    check(n68_sent_has(b, 0, before, 0x11), "flow turned off: the owed Xon1 goes out");
    case_end("X3 SC16C650B: EFR change after an Xoff sends the owed Xon");
    bench_close(b);
}

/* Special character (EFR[5], p20/p37): a received char equal to Xoff2 sets
   ISR[4] (IER[5]) and is stored; the SC16C650B needs EFR[3:0] = 0000. */
static void case_650_special(void) {
    Bench *b = bench_open_cfg("SFX-SC16C650B", HALF_18432, &NXP650_BENCH);
    SimTime bt;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 3, 0xBF);
    bus_write(b, 7, 0x2A);   /* Xoff2 */
    bus_write(b, 2, 0x30);   /* enhanced, special char, no software flow */
    bus_write(b, 3, 0x03);
    bus_write(b, 2, 0x01);
    bus_write(b, 1, 0x20);   /* IER[5] */
    drive_named(b, "RX", 0x2A, bt);
    run(b, bt * 2);
    expect_reg(b, 2, "11010000", "Xoff2 received: ISR = special character (0x10)");
    expect_reg(b, 0, "00101010", "the special character is stored");
    case_end("X4 SC16C650B: special character with EFR[3:0] = 0000");
    bench_close(b);
}

/* Auto RTS (EFR[6], Table 4 p10): RTS goes high at the trigger; IER[6]
   then raises the CTS/RTS interrupt (ISR 0x20, p21) for RTS low -> high. */
static void case_650_rts_int(void) {
    Bench *b = bench_open_cfg("SFX-SC16C650B", HALF_18432, &NXP650_BENCH);
    SimTime bt;
    unsigned i;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bank_write(b, 2, 0x50, 0x03);   /* enhanced, auto RTS */
    bus_write(b, 2, 0x01);
    bus_write(b, 4, 0x02);   /* MCR[1]: RTS on */
    bus_write(b, 1, 0x40);   /* IER[6] */
    expect_pin(b, "RTS", LOGIC_0, "RTS active before the trigger");
    for (i = 0; i < 8; i++) {
        drive_named(b, "RX", 0x50 + i, bt);
    }
    run(b, bt);
    expect_pin(b, "RTS", LOGIC_1, "8 received: auto RTS goes high");
    expect_reg(b, 2, "11100000", "ISR = CTS/RTS change (0x20)");
    case_end("X5 SC16C650B: auto RTS raises the RTS interrupt");
    bench_close(b);
}

/* ---- NS16C2552 (ti-c.md § NS16C2552/NS16C2752) ------------------------------------------- */

/* Opens the NS16C2552 on channel 1, 8N1 divisor 1, Xon1/Xon2/Xoff1/Xoff2 =
   0x11/0x12/0x13/0x14, EFR = `efr`, FIFO on (trigger 1). */
static Bench *ns_open(unsigned efr, SimTime *bt) {
    Bench *b = bench_open_cfg("SFX-NS16C2552", HALF_18432, &NS_BENCH);

    program(b, 1, 0x03);
    *bt = bit_time(b, 1);
    bus_write(b, 3, 0xBF);
    bus_write(b, 4, 0x11);
    bus_write(b, 5, 0x12);
    bus_write(b, 6, 0x13);
    bus_write(b, 7, 0x14);
    bus_write(b, 2, efr);
    bus_write(b, 3, 0x03);
    bus_write(b, 2, 0x01);
    return b;
}

/* A THR byte goes out within a few frames (1), or is held (0). */
static int tx_goes(Bench *b, unsigned v, SimTime bt) {
    size_t before = n68_sent_count(b, 0);

    bus_write(b, 0, v);
    run(b, bt * 24);
    return n68_sent_count(b, 0) > before && n68_sent_has(b, 0, before, v);
}

/* EFR[3:0] = 0011 (Table 20 p20): RX compares Xon1 & Xon2, Xoff1 & Xoff2 as
   sequential pairs; a first char not followed by its pair is stored (Reset
   notes p22); flow chars are not stored. */
static void case_ns_pairs(void) {
    SimTime bt;
    Bench *b = ns_open(0x13, &bt);

    case_begin();
    drive_named(b, "SIN1", 0x13, bt);
    drive_named(b, "SIN1", 0x14, bt);
    run(b, bt);
    expect_reg(b, 5, "01100000", "Xoff1 Xoff2: nothing stored");
    check(!tx_goes(b, 0x41, bt), "Xoff1 Xoff2: the transmitter halts");
    drive_named(b, "SIN1", 0x11, bt);
    drive_named(b, "SIN1", 0x55, bt);
    run(b, bt);
    expect_reg(b, 0, "00010001", "Xon1 then 0x55: Xon1 is stored");
    expect_reg(b, 0, "01010101", "then 0x55");
    check(n68_sent_count(b, 0) == 0, "Xon1 alone does not resume");
    drive_named(b, "SIN1", 0x11, bt);
    drive_named(b, "SIN1", 0x12, bt);
    run(b, bt * 24);
    check(n68_sent_has(b, 0, 0, 0x41), "Xon1 Xon2: the held byte goes out");
    expect_reg(b, 5, "01100000", "Xon1 Xon2: nothing stored");
    case_end("X6 NS16C2552: EFR 0011, sequential Xon/Xoff pairs");
    bench_close(b);
}

/* EFR[3:0] = 1011 (Table 20): RX compares Xon1 or Xon2, Xoff1 or Xoff2. */
static void case_ns_either(void) {
    SimTime bt;
    Bench *b = ns_open(0x1B, &bt);

    case_begin();
    drive_named(b, "SIN1", 0x14, bt);
    run(b, bt);
    check(!tx_goes(b, 0x42, bt), "Xoff2 alone halts (1011)");
    drive_named(b, "SIN1", 0x11, bt);
    run(b, bt * 24);
    check(n68_sent_has(b, 0, 0, 0x42), "Xon1 alone resumes (1011)");
    expect_reg(b, 5, "01100000", "flow chars not stored");
    case_end("X7 NS16C2552: EFR 1011, Xon1 or Xon2 / Xoff1 or Xoff2");
    bench_close(b);
}

/* EFR[1:0] = 01 compares Xon2/Xoff2; with EFR[5] Xoff2 is not stored and
   raises the Xoff/special interrupt (Table 20; Behavior "Special character"). */
static void case_ns_mode01(void) {
    SimTime bt;
    Bench *b = ns_open(0x31, &bt);

    case_begin();
    bus_write(b, 1, 0x20);
    drive_named(b, "SIN1", 0x14, bt);
    run(b, bt);
    expect_reg(b, 2, "11010000", "Xoff2 in mode 01 with EFR[5]: IIR = Xoff/special");
    expect_reg(b, 5, "01100000", "Xoff2 is not stored");
    check(!tx_goes(b, 0x43, bt), "Xoff2 halts");
    drive_named(b, "SIN1", 0x12, bt);
    run(b, bt * 24);
    check(n68_sent_has(b, 0, 0, 0x43), "Xon2 resumes");
    case_end("X8 NS16C2552: EFR 0x31, Xoff2 is flow and special");
    bench_close(b);
}

/* EFR[5] special character (Table 8, EFR bit 5): Xoff2 is stored and flags
   IIR[4]; the interrupt clears on the next received character. */
static void case_ns_special_next(void) {
    SimTime bt;
    Bench *b = ns_open(0x30, &bt);

    case_begin();
    bus_write(b, 1, 0x20);
    drive_named(b, "SIN1", 0x14, bt);
    run(b, bt);
    expect_reg(b, 2, "11010000", "Xoff2 with EFR[5]: IIR = special character");
    drive_named(b, "SIN1", 0x41, bt);
    run(b, bt);
    expect_reg(b, 2, "11000001", "the next character cleared it");
    expect_reg(b, 0, "00010100", "Xoff2 stored");
    expect_reg(b, 0, "01000001", "then 0x41");
    case_end("X8b NS16C2552: special character cleared by the next character");
    bench_close(b);
}

/* Xon-any (MCR[5], Behavior): any received character resumes TX and is stored. */
static void case_ns_xon_any(void) {
    SimTime bt;
    Bench *b = ns_open(0x12, &bt);

    case_begin();
    bus_write(b, 4, 0x20);
    drive_named(b, "SIN1", 0x13, bt);
    run(b, bt);
    check(!tx_goes(b, 0x44, bt), "Xoff1 halts");
    drive_named(b, "SIN1", 0x61, bt);
    run(b, bt * 24);
    check(n68_sent_has(b, 0, 0, 0x44), "0x61 with Xon-any resumes");
    expect_reg(b, 0, "01100001", "and 0x61 is stored");
    case_end("X9 NS16C2552: Xon-any resumes on any character");
    bench_close(b);
}

/* RTS interrupt (IER[6], Table 27: trigger 1 deasserts RTS at 2) cleared by
   an IIR read; CTS interrupt (IER[7], auto-CTS, CTS low -> high) - both IIR
   0x20, "reading IIR or MSR" (Tables 7-8). */
static void case_ns_ctsrts(void) {
    SimTime bt;
    Bench *b = ns_open(0x50, &bt);

    case_begin();
    bus_write(b, 4, 0x02);
    bus_write(b, 1, 0x40);
    drive_named(b, "SIN1", 0x01, bt);
    drive_named(b, "SIN1", 0x02, bt);
    run(b, bt);
    expect_pin(b, "RTS1", LOGIC_1, "2 received: auto RTS off");
    expect_reg(b, 2, "11100000", "IIR = RTS/CTS change");
    expect_reg(b, 2, "11000001", "an IIR read cleared it");
    bus_read(b, 0);
    bus_read(b, 0);
    bank_write(b, 2, 0x90, 0x03);   /* auto CTS */
    bus_write(b, 1, 0x80);
    set(b, "CTS1", LOGIC_0);
    run(b, bt);
    bus_read(b, 6);
    set(b, "CTS1", LOGIC_1);
    run(b, bt);
    expect_reg(b, 2, "11100000", "CTS low -> high with IER[7]: IIR = RTS/CTS change");
    case_end("X10 NS16C2552: RTS and CTS interrupts, IIR read clears");
    bench_close(b);
}

/* AFR (LCR[7] = 1, A = 010): b7:3 reserved 0; MF select AFR[2:1] = 01
   BAUDOUT, 11 forced 1 (MF selection, PLCC note). DREV at 0 with DLL = DLM = 0. */
static void case_ns_afr(void) {
    SimTime bt;
    Bench *b = ns_open(0x00, &bt);
    int saw0 = 0;
    int saw1 = 0;
    unsigned i;

    case_begin();
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0xFE);
    expect_reg(b, 2, "00000110", "AFR reads b2:1, b7:3 read 0");
    bus_write(b, 2, 0x02);   /* MF = BAUDOUT */
    bus_write(b, 3, 0x03);
    for (i = 0; i < 40; i++) {
        run(b, NS(70));
        saw0 |= get(b, "MF1") == LOGIC_0;
        saw1 |= get(b, "MF1") == LOGIC_1;
    }
    check(saw0 && saw1, "AFR[2:1] = 01: MF1 is the 16x clock");
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x06);   /* MF forced 1 */
    bus_write(b, 3, 0x03);
    run(b, NS(500));
    expect_pin(b, "MF1", LOGIC_1, "AFR[2:1] = 11: MF forced 1");
    case_end("X11 NS16C2552: AFR read mask, MF = BAUDOUT and forced 1");
    bench_close(b);
}

/* Sleep (IER[4] with EFR[4], pp29-30) is entered only with a divisor other
   than 0: with divisor 0 the channel stays awake, so once loopback (which
   keeps it awake, p30) is on and the divisor is loaded, the 16x clock on MF
   runs; with loopback off it sleeps and the clock stops (BAUDOUT X). */
static void case_ns_sleep_divisor(void) {
    SimTime bt;
    Bench *b = ns_open(0x10, &bt);
    int saw0 = 0;
    int saw1 = 0;
    unsigned i;

    case_begin();
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x02);   /* MF = BAUDOUT */
    bus_write(b, 0, 0x00);
    bus_write(b, 3, 0x03);
    bus_write(b, 1, 0x10);   /* IER[4] with divisor 0 */
    bus_write(b, 4, 0x10);   /* loopback */
    bus_write(b, 3, 0x80);
    bus_write(b, 0, 0x01);
    bus_write(b, 3, 0x03);
    for (i = 0; i < 40; i++) {
        run(b, NS(70));
        saw0 |= get(b, "MF1") == LOGIC_0;
        saw1 |= get(b, "MF1") == LOGIC_1;
    }
    check(saw0 && saw1, "divisor 0 with IER[4]: stayed awake, MF1 runs once loaded");
    bus_write(b, 4, 0x00);
    run(b, NS(2000));
    expect_pin(b, "MF1", LOGIC_X, "divisor 1, idle, no loopback: asleep, BAUDOUT stops");
    case_end("X12 NS16C2552: sleep waits for a divisor other than 0");
    bench_close(b);
}

/* idRegs "dvid" (docs/parts-format.md): DVID reads at A = 1 with LCR[7] = 1 and
   DLL = DLM = 0 (a variant of the NS16C2552 with a DVID). */
static void case_dvid(void) {
    Bench *b = bench_open_cfg(n68_part("SFX-NS16C2552", "{\"idRegs\":{\"drev\":49,\"dvid\":90}}"),
                              HALF_18432, &NS_BENCH);

    case_begin();
    bus_write(b, 3, 0x80);
    bus_write(b, 0, 0x00);
    bus_write(b, 1, 0x00);
    expect_reg(b, 0, "00110001", "DREV 0x31");
    expect_reg(b, 1, "01011010", "DVID 0x5A");
    case_end("X13 idRegs dvid: DVID at DLM with the divisor 0");
    bench_close(b);
}

/* ---- Helpers for pins and X writes ------------------------------------------------------- */

/* EFR[4] = 1 through the LCR = 0xBF bank, then LCR = `lcr`. */
static void enhance(Bench *b, unsigned efr, unsigned lcr) {
    bank_write(b, 2, efr, lcr);
}

/* ---- SC16C752B register map corners (exar-d.md § SC16C752B, Table 9 notes) -------------- */

/* At LCR = 0xBF with EFR[4] and MCR[6] both Xoff1/2 and TCR/TLR qualify: read
   X, writes ignored (decisions.md). With LCR[7] = 1 (not 0xBF) the TCR needs
   LCR[7] = 0 on this part (Table 9 note: "presumably also requires
   LCR[7] = 0"): X. MCR[6] written X makes the TCR/TLR condition X. */
static void case_752b_map(void) {
    Bench *b = bench_open_cfg("SFX-SC16C752B", NS(50), &DUAL_SYNC_BENCH);

    case_begin();
    bus_write(b, 3, 0xBF);
    bus_write(b, 6, 0x13);
    bus_write(b, 7, 0x14);
    bus_write(b, 2, 0x10);
    bus_write(b, 3, 0x03);
    bus_write(b, 4, 0x40);   /* MCR[6]: TCR/TLR */
    bus_write(b, 6, 0x21);   /* TCR */
    bus_write(b, 3, 0xBF);
    expect_reg(b, 6, "XXXXXXXX", "LCR 0xBF with the TCR condition: 110 reads X");
    bus_write(b, 6, 0x55);   /* ignored */
    bus_write(b, 3, 0x83);
    expect_reg(b, 6, "XXXXXXXX", "LCR[7] = 1: the TCR needs LCR[7] = 0 here, X");
    bus_write(b, 3, 0x03);
    expect_reg(b, 6, "00100001", "TCR kept 0x21");
    bus_write(b, 4, 0x00);
    bus_write(b, 3, 0xBF);
    expect_reg(b, 6, "00010011", "Xoff1 kept 0x13 (the write was ignored)");
    bus_write(b, 3, 0x03);
    n68_write_x(b, 4, 0x00, 0x40);   /* MCR[6] X */
    expect_reg(b, 6, "XXXXXXXX", "MCR[6] X: 110 reads X");
    bus_write(b, 4, 0x00);
    bus_write(b, 7, 0x5A);           /* SPR */
    n68_write_x(b, 4, 0x00, 0x40);
    bus_write(b, 7, 0x77);           /* TLR or SPR: ignored */
    bus_write(b, 4, 0x00);
    expect_reg(b, 7, "01011010", "SPR unchanged by the ignored write");
    case_end("X14 SC16C752B: Xoff/TCR overlap, TCR with LCR[7] = 1, MCR[6] X");
    bench_close(b);
}

/* FIFO Rdy (MCR[2] = 1, MCR[4] = 0; b4 RX FIFO A status = RXRDY, i.e. the
   trigger reached): 8 characters at trigger 8 set b4. */
static void case_752b_fifordy(void) {
    Bench *b = bench_open_cfg("SFX-SC16C752B", NS(50), &DUAL_SYNC_BENCH);
    SimTime bt;
    unsigned i;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0x01);   /* FIFO on, trigger 8 */
    bus_write(b, 4, 0x04);
    expect_reg(b, 7, "00000011", "FIFO Rdy: both TX empty, no RX");
    for (i = 0; i < 8; i++) {
        drive_named(b, "RXA", 0x30 + i, bt);
    }
    run(b, bt);
    expect_reg(b, 7, "00010011", "8 received at trigger 8: RX FIFO A ready");
    case_end("X15 SC16C752B: FIFO Rdy RX status");
    bench_close(b);
}

/* Special character cleared by an LSR read on this part ("specialClear":
   "lsr"; ISR clear rules p21 of the family). */
static void case_752b_special_lsr(void) {
    Bench *b = bench_open_cfg("SFX-SC16C752B", NS(50), &DUAL_SYNC_BENCH);
    SimTime bt;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 3, 0xBF);
    bus_write(b, 7, 0x2A);
    bus_write(b, 2, 0x30);
    bus_write(b, 3, 0x03);
    bus_write(b, 2, 0x01);
    bus_write(b, 1, 0x20);
    drive_named(b, "RXA", 0x2A, bt);
    run(b, bt);
    expect_reg(b, 2, "11010000", "special character: IIR 0x10");
    expect_reg(b, 2, "11010000", "an IIR read does not clear it here");
    bus_read(b, 5);
    expect_reg(b, 2, "11000001", "the LSR read cleared it");
    case_end("X16 SC16C752B: the special character clears on an LSR read");
    bench_close(b);
}

/* ---- TL16C750E (ti-b.md § TL16C750E) ----------------------------------------------------- */

/* At 111 both DLF and TLR can be enabled (Register map, "Two priority cases
   are not stated"): reads X. DLF[7] = 1: 8x sampling, so a bit is 8 clocks. */
static void case_750e_dlf(void) {
    Bench *b = bench_open_cfg("SFX-TL16C750E", NS(50), &TI750E_BENCH);
    SimTime len;
    char what[96];

    case_begin();
    program(b, 1, 0x03);
    enhance(b, 0x10, 0x03);
    bus_write(b, 4, 0x40);
    bus_write(b, 3, 0x83);
    expect_reg(b, 7, "XXXXXXXX", "DLF and TLR both enabled: 111 reads X");
    bus_write(b, 3, 0x03);
    bus_write(b, 4, 0x00);
    bus_write(b, 3, 0x83);
    bus_write(b, 7, 0x80);   /* DLF[7]: 8x */
    bus_write(b, 3, 0x03);
    bus_write(b, 0, 0x00);
    len = n68_low_len(b, "TX", NS(20000));
    snprintf(what, sizeof what, "DLF[7] = 1: start+8 zero bits %llu ps, want 9 x 8 clocks",
             (unsigned long long)len);
    check(len >= NS(9 * 800) - NS(100) && len <= NS(9 * 800) + NS(200), what);
    case_end("X17 TL16C750E: DLF/TLR overlap reads X; DLF[7] 8x sampling");
    bench_close(b);
}

/* RS-485 (AFR[2], pp44-45): DTR goes high before the first start bit and
   stays high while data is pending; RCVEN = 0 (AFR[4]) ignores RX while the
   channel transmits (Table 23 text). */
static void case_750e_rs485(void) {
    Bench *b = bench_open_cfg("SFX-TL16C750E", NS(50), &TI750E_BENCH);
    SimTime bt;
    unsigned i;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0x01);
    bus_write(b, 3, 0x83);
    bus_write(b, 2, 0x04);   /* AFR: 485EN, RCVEN = 0 */
    bus_write(b, 3, 0x03);
    expect_pin(b, "DTR", LOGIC_0, "RS-485 idle: DTR low");
    for (i = 0; i < 4; i++) {
        bus_write(b, 0, 0x55);
    }
    check(wait_level(b, "DTR", LOGIC_1, bt * 2), "data pending: DTR goes high");
    expect_pin(b, "TX", LOGIC_1, "DTR leads the start bit");
    run(b, bt * 3);
    expect_pin(b, "DTR", LOGIC_1, "DTR high while sending");
    for (i = 0; i < 9; i++) {
        set(b, "RX", i == 0 || (i & 1) ? LOGIC_0 : LOGIC_1);
        run(b, bt);
    }
    set(b, "RX", LOGIC_1);
    run(b, bt * 50);
    expect_reg(b, 5, "01100000", "RCVEN = 0: the frame received while sending is not stored");
    expect_pin(b, "DTR", LOGIC_0, "all sent: DTR low again");
    case_end("X18 TL16C750E: RS-485 DTR, RCVEN = 0 while transmitting");
    bench_close(b);
}

/* IrDA (AFR[1], pp46-49): TX is low when no data is sent, also with break
   (LCR[6]) set and the transmitter idle; an X break bit makes TX X. */
static void case_750e_irda_pin(void) {
    Bench *b = bench_open_cfg("SFX-TL16C750E", NS(50), &TI750E_BENCH);

    case_begin();
    program(b, 1, 0x03);
    bus_write(b, 3, 0x83);
    bus_write(b, 2, 0x12);   /* AFR: IREN, RCVEN */
    bus_write(b, 3, 0x43);   /* break */
    run(b, NS(2000));
    expect_pin(b, "TX", LOGIC_0, "IrDA, break, idle: no light");
    n68_write_x(b, 3, 0x03, 0x40);
    run(b, NS(500));
    expect_pin(b, "TX", LOGIC_X, "IrDA, break bit X: TX X");
    case_end("X19 TL16C750E: IrDA TX with break and with an X break bit");
    bench_close(b);
}

/* ---- TL16C752C (ti-c.md § TL16C752C) ----------------------------------------------------- */

/* TCR at 110 with EFR[4] = 1 and MCR[6] = 1, whatever LCR[7] (Registers);
   Xoff sent when the RX FIFO reaches the TCR halt level (x4); MCR[7] ÷4
   (Baud): a bit is 4 x 16 clocks. */
static void case_752c_tcr(void) {
    Bench *b = bench_open_cfg("SFX-TL16C752C", NS(50), &DUAL_CD_BENCH);
    SimTime bt;
    SimTime len;
    char what[96];
    unsigned i;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 3, 0xBF);
    bus_write(b, 4, 0x11);
    bus_write(b, 6, 0x13);
    bus_write(b, 2, 0x18);   /* enhanced, TX Xon1/Xoff1 */
    bus_write(b, 3, 0x03);
    bus_write(b, 2, 0x01);
    bus_write(b, 4, 0x40);
    bus_write(b, 6, 0x01);   /* TCR: halt 4, resume 0 */
    bus_write(b, 3, 0x83);
    expect_reg(b, 6, "00000001", "TCR reads at 110 with LCR[7] = 1");
    bus_write(b, 3, 0x03);
    bus_write(b, 4, 0x00);
    for (i = 0; i < 3; i++) {
        drive_named(b, "RXA", 0x20 + i, bt);
    }
    run(b, bt * 12);
    check(!n68_sent_has(b, 0, 0, 0x13), "3 received, halt 4: no Xoff");
    drive_named(b, "RXA", 0x23, bt);
    run(b, bt * 24);
    check(n68_sent_has(b, 0, 0, 0x13), "4 received: Xoff1 sent");
    for (i = 0; i < 4; i++) {
        bus_read(b, 0);
    }
    run(b, bt * 24);
    check(n68_sent_has(b, 0, 0, 0x11), "emptied to resume 0: Xon1 sent");
    bus_write(b, 4, 0x80);   /* MCR[7]: ÷4 */
    bus_write(b, 0, 0x00);
    len = n68_low_len(b, "TXA", NS(200000));
    snprintf(what, sizeof what, "MCR[7] ÷4: 9 zero bits %llu ps, want 9 x 64 clocks",
             (unsigned long long)len);
    check(len >= NS(9 * 6400) - NS(400) && len <= NS(9 * 6400) + NS(800), what);
    case_end("X20 TL16C752C: TCR with LCR[7] = 1, TCR Xoff/Xon, ÷4 prescaler");
    bench_close(b);
}

/* ---- Sleep conditions (NS16C2552 pp29-30) ---------------------------------------------- */

/* A pending Xoff interrupt keeps the channel awake (sleep only with no
   interrupt pending, pp29-30); an IIR read clears the Xoff interrupt but
   not the halt (Table 8); once idle the channel sleeps and the 16x clock on
   MF1 stops. */
static void case_ns_sleep_pending(void) {
    SimTime bt;
    Bench *b = ns_open(0x12, &bt);

    case_begin();
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x02);
    bus_write(b, 3, 0x03);
    bus_write(b, 1, 0x20);
    drive_named(b, "SIN1", 0x13, bt);
    run(b, bt);
    bus_write(b, 1, 0x30);   /* sleep enable with the Xoff interrupt pending */
    check(n68_toggles(b, "MF1", 40, NS(70)), "Xoff interrupt pending: awake");
    expect_reg(b, 2, "11010000", "IIR = Xoff");
    expect_reg(b, 2, "11000001", "the IIR read cleared the Xoff interrupt (Table 8)");
    check(!tx_goes(b, 0x45, bt), "the transmitter stays halted");
    drive_named(b, "SIN1", 0x11, bt);
    run(b, bt * 24);
    check(n68_sent_has(b, 0, 0, 0x45), "Xon1 resumes");
    run(b, bt * 2);
    expect_pin(b, "MF1", LOGIC_X, "nothing pending: asleep");
    case_end("X21 NS16C2552: a pending interrupt keeps the channel awake");
    bench_close(b);
}

/* "sleep": "ier4-all" (docs/parts-format.md): a channel sleeps only when
   every channel has IER[4] set (a variant of the NS16C2552). */
static void case_sleep_all(void) {
    Bench *b = bench_open_cfg(n68_part("SFX-NS16C2552", "{\"sleep\":\"ier4-all\"}"), HALF_18432,
                              &NS_BENCH);
    unsigned u;

    case_begin();
    for (u = 0; u < 2; u++) {
        program_unit(b, u, 1, 0x03);
        enhance(b, 0x10, 0x03);
        bus_write(b, 3, 0x80);
        bus_write(b, 2, 0x02);
        bus_write(b, 3, 0x03);
    }
    select_unit(b, 0);
    bus_write(b, 1, 0x10);
    check(n68_toggles(b, "MF1", 40, NS(70)), "only channel 1 has IER[4]: awake");
    select_unit(b, 1);
    bus_write(b, 1, 0x10);
    select_unit(b, 0);
    bus_write(b, 1, 0x10);
    run(b, NS(2000));
    expect_pin(b, "MF1", LOGIC_X, "both channels have IER[4]: channel 1 asleep");
    case_end("X22 sleep ier4-all: every channel's IER[4]");
    bench_close(b);
}

/* ---- TLR levels and AFE trigger 14 -------------------------------------------------------- */

/* TLR (EFR[4] = 1, MCR[6] = 1, 111): b7:4 RX trigger x4, b3:0 TX trigger
   (spaces) x4; 0 = the FCR level (SC16C752B, Table 9; exar-d.md TLR). */
static void case_752b_tlr(void) {
    Bench *b = bench_open_cfg("SFX-SC16C752B", NS(50), &DUAL_SYNC_BENCH);
    SimTime bt;
    unsigned i;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    enhance(b, 0x10, 0x03);
    bus_write(b, 2, 0x01);
    bus_write(b, 4, 0x40);
    bus_write(b, 7, 0x1F);   /* RX trigger 4, TX trigger 60 spaces */
    bus_write(b, 4, 0x00);
    bus_write(b, 1, 0x01);
    for (i = 0; i < 3; i++) {
        drive_named(b, "RXA", 0x30 + i, bt);
    }
    run(b, bt);
    expect_reg(b, 2, "11000001", "3 received, TLR RX trigger 4: no RX interrupt");
    drive_named(b, "RXA", 0x33, bt);
    run(b, bt);
    expect_reg(b, 2, "11000100", "4 received: RX data interrupt");
    bus_write(b, 4, 0x04);   /* FIFO Rdy */
    for (i = 0; i < 8; i++) {
        bus_write(b, 0, 0x41);
    }
    expect_reg(b, 7, "00010010", "8 in the TX FIFO, 56 spaces < 60: TX FIFO A not ready");
    case_end("X23 SC16C752B: TLR RX and TX trigger levels");
    bench_close(b);
}

/* Auto-RTS at trigger 14 (ti-a.md TL16C550C Autoflow): RTS goes high at the
   first data bit of the 16th character and low again once a byte is free
   with nothing coming in. */
static void case_550c_rts14(void) {
    Bench *b = bench_open_cfg("SFX-TL16C550C", HALF_18432, &TI550_BENCH);
    SimTime bt;
    unsigned i;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0xC1);   /* FIFO, trigger 14 */
    bus_write(b, 4, 0x22);   /* AFE + RTS */
    for (i = 0; i < 15; i++) {
        drive_byte(b, 0x30 + i, bt, 1);
    }
    run(b, bt);
    expect_pin(b, "RTS", LOGIC_0, "15 received: RTS still low");
    drive_bits(b, "00", bt);   /* start and data bit 0 of the 16th */
    run(b, bt / 2);
    expect_pin(b, "RTS", LOGIC_1, "the 16th coming in: RTS high");
    drive_bits(b, "0000000", bt);
    set(b, "SIN", LOGIC_1);
    run(b, bt * 2);
    bus_read(b, 0);
    run(b, bt);
    expect_pin(b, "RTS", LOGIC_0, "a byte free, nothing coming: RTS low");
    case_end("X24 TL16C550C: auto-RTS at trigger 14");
    bench_close(b);
}

/* Auto-CTS in loopback (ti-a.md TL16C550C Autoflow and Loopback: CTS is
   MCR[1] inside): with RTS (MCR[1]) off the transmitter waits; on, the byte
   goes round the loop. */
static void case_550c_cts_loop(void) {
    Bench *b = bench_open_cfg("SFX-TL16C550C", HALF_18432, &TI550_BENCH);
    SimTime bt;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0x01);
    bus_write(b, 4, 0x30);   /* AFE, loopback, RTS off */
    bus_write(b, 0, 0x5A);
    run(b, bt * 24);
    expect_reg(b, 5, "00000000", "loop CTS (MCR[1]) off: the byte waits");
    bus_write(b, 4, 0x32);
    run(b, bt * 24);
    expect_reg(b, 5, "01100001", "MCR[1] on: sent and looped back");
    expect_reg(b, 0, "01011010", "the byte");
    case_end("X25 TL16C550C: auto-CTS from MCR[1] in loopback");
    bench_close(b);
}

int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_650_bank();
    case_650_flow();
    case_650_owed_xon();
    case_650_special();
    case_650_rts_int();
    case_ns_pairs();
    case_ns_either();
    case_ns_mode01();
    case_ns_special_next();
    case_ns_xon_any();
    case_ns_ctsrts();
    case_ns_afr();
    case_ns_sleep_divisor();
    case_dvid();
    case_752b_map();
    case_752b_fifordy();
    case_752b_special_lsr();
    case_750e_dlf();
    case_750e_rs485();
    case_750e_irda_pin();
    case_752c_tcr();
    case_ns_sleep_pending();
    case_sleep_all();
    case_752b_tlr();
    case_550c_rts14();
    case_550c_cts_loop();
    return serial_bench_finish();
}
