/*
 * serial_n68_exar_test (plan-68-N): the Exar options of the ns16550 model
 * (models/ns16550_exar*.c) on the shipped fixture devices — software flow
 * control on the 650-style bank, the CTS/RTS interrupts, sleep, the MF and
 * IrDA pins, the 854/864 and 5G register maps and selects, the 5G timer.
 * Each case cites the spec section (planning/plan-12-data/specs/exar-*.md)
 * its expectations come from.
 */

#include "serial_bench_exar.h"
#include "serial_bench_n68.h"

/* ---- XR16L2552 software flow (exar-c.md / exar-d.md: the ST16C650A-style
   EFR bank, Table "EFR[3:0]"; Exar prints "or" for 1011/0111) ------------- */

/* Channel A of the XR16L2552, 8N1 divisor 1, Xon1/Xon2/Xoff1/Xoff2 =
   0x11/0x12/0x13/0x14, EFR = `efr`, FIFO on (trigger 1). */
static Bench *xa_open(const char *part, unsigned efr, SimTime *bt) {
    Bench *b = bench_open_cfg(part, HALF_18432, &CHSEL_BENCH);

    select_unit(b, 0);
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

static int tx_goes(Bench *b, unsigned v, SimTime bt) {
    size_t before = n68_sent_count(b, 0);

    bus_write(b, 0, v);
    run(b, bt * 24);
    return n68_sent_has(b, 0, before, v);
}

/* EFR 0011: sequential pairs; Xon1 not followed by Xon2 is stored. */
static void case_pairs(void) {
    SimTime bt;
    Bench *b = xa_open("SFX-XR16L2552", 0x13, &bt);

    case_begin();
    drive_named(b, "RXA", 0x13, bt);
    drive_named(b, "RXA", 0x14, bt);
    run(b, bt);
    expect_reg(b, 5, "01100000", "Xoff1 Xoff2: nothing stored");
    check(!tx_goes(b, 0x41, bt), "Xoff1 Xoff2 halts");
    drive_named(b, "RXA", 0x11, bt);
    drive_named(b, "RXA", 0x55, bt);
    run(b, bt);
    expect_reg(b, 0, "00010001", "Xon1 then 0x55: Xon1 stored");
    expect_reg(b, 0, "01010101", "then 0x55");
    drive_named(b, "RXA", 0x11, bt);
    drive_named(b, "RXA", 0x12, bt);
    run(b, bt * 24);
    check(n68_sent_has(b, 0, 0, 0x41), "Xon1 Xon2 resumes");
    case_end("E1 XR16L2552: EFR 0011, sequential pairs");
    bench_close(b);
}

/* EFR 1011 (either character), 0010 (Xon1/Xoff1), 0001 (Xon2/Xoff2). */
static void case_modes(void) {
    static const struct { unsigned efr, xoff, xon; const char *name; } MODES[] = {
        {0x1B, 0x14, 0x11, "1011: Xoff2 halts, Xon1 resumes"},
        {0x12, 0x13, 0x11, "0010: Xoff1 / Xon1"},
        {0x11, 0x14, 0x12, "0001: Xoff2 / Xon2"},
    };
    unsigned i;

    case_begin();
    for (i = 0; i < 3; i++) {
        SimTime bt;
        Bench *b = xa_open("SFX-XR16L2552", MODES[i].efr, &bt);
        char what[96];

        drive_named(b, "RXA", MODES[i].xoff, bt);
        run(b, bt);
        snprintf(what, sizeof what, "%s: halted", MODES[i].name);
        check(!tx_goes(b, 0x42, bt), what);
        drive_named(b, "RXA", MODES[i].xon, bt);
        run(b, bt * 24);
        snprintf(what, sizeof what, "%s: resumed", MODES[i].name);
        check(n68_sent_has(b, 0, 0, 0x42), what);
        expect_reg(b, 5, "01100000", "flow characters are not stored");
        bench_close(b);
    }
    case_end("E2 XR16L2552: EFR RX compare modes 1011, 0010, 0001");
}

/* EFR[1:0] = 01 with EFR[5]: Xoff2 is a flow character (not stored) and
   raises the special-character interrupt too (exar-d.md p37 text). A
   received flow character with a framing error is data (stored, no halt). */
static void case_special_and_errors(void) {
    SimTime bt;
    Bench *b = xa_open("SFX-XR16L2552", 0x31, &bt);

    case_begin();
    bus_write(b, 1, 0x20);
    drive_named(b, "RXA", 0x14, bt);
    run(b, bt);
    expect_reg(b, 2, "11010000", "Xoff2 in mode 01 with EFR[5]: ISR = Xoff/special");
    expect_reg(b, 5, "01100000", "Xoff2 not stored");
    drive_named(b, "RXA", 0x12, bt);
    run(b, bt);
    bus_write(b, 1, 0x00);
    drive_bits_on(b, "RXA", "0001010000", bt);   /* Xoff2 (0x14) with stop bit 0 */
    set(b, "RXA", LOGIC_1);
    run(b, bt * 2);
    expect_reg(b, 5, "11101001", "an errored Xoff2: stored with FE");
    expect_reg(b, 0, "00010100", "the errored Xoff2 is data");
    check(tx_goes(b, 0x43, bt), "an errored Xoff2 does not halt");
    case_end("E3 XR16L2552: Xoff2 as flow and special; an errored flow char is data");
    bench_close(b);
}

/* Auto Xoff sending (exar-d.md XR16V564 Behavior: "Xoff sent 2 char times after
   the RX FIFO crosses the trigger"; xoffTable trigger 1: Xoff at 1, Xon at 0);
   EFR's TX mode changed after an Xoff: the owed Xon goes out. */
static void case_send_xoff(void) {
    SimTime bt;
    Bench *b = xa_open("SFX-XR16L2552", 0x18, &bt);
    size_t before;

    case_begin();
    drive_named(b, "RXA", 0x61, bt);
    run(b, bt * 30);
    check(n68_sent_has(b, 0, 0, 0x13), "1 received at trigger 1: Xoff1 sent");
    before = n68_sent_count(b, 0);
    bus_write(b, 3, 0xBF);
    bus_write(b, 2, 0x14);   /* TX mode 01 now */
    bus_write(b, 3, 0x03);
    run(b, bt * 30);
    check(n68_sent_has(b, 0, before, 0x11), "TX mode changed: the owed Xon1 goes out");
    case_end("E4 XR16L2552: Xoff sending and the owed Xon");
    bench_close(b);
}

/* Auto RTS interrupt (IER[6], RTS low -> high with MCR[1]) and auto CTS
   interrupt (IER[7], CTS low -> high); "ctsRtsClear": "msr" (MSR read). */
static void case_ctsrts(void) {
    SimTime bt;
    Bench *b = xa_open("SFX-XR16L2552", 0x50, &bt);
    unsigned i;

    case_begin();
    bus_write(b, 4, 0x02);
    bus_write(b, 1, 0x40);
    for (i = 0; i < 4; i++) {
        drive_named(b, "RXA", 0x30 + i, bt);
    }
    run(b, bt);
    expect_pin(b, "RTSA", LOGIC_1, "rtsTable trigger 1: RTS off at 4");
    expect_reg(b, 2, "11100000", "ISR = CTS/RTS change");
    bus_read(b, 6);
    expect_reg(b, 2, "11000001", "cleared by the MSR read");
    for (i = 0; i < 4; i++) {
        bus_read(b, 0);
    }
    bus_write(b, 3, 0xBF);
    bus_write(b, 2, 0x90);
    bus_write(b, 3, 0x03);
    bus_write(b, 1, 0x80);
    set(b, "CTSA", LOGIC_0);
    run(b, bt);
    bus_read(b, 6);
    set(b, "CTSA", LOGIC_1);
    run(b, bt);
    expect_reg(b, 2, "11100000", "CTS low -> high with IER[7]: ISR = CTS/RTS");
    case_end("E5 XR16L2552: RTS and CTS interrupts");
    bench_close(b);
}

/* "ctsRtsClear": "iir" and "specialClear": "lsr" on an Exar part (variants):
   an ISR read clears the CTS/RTS interrupt; an LSR read clears the special
   character interrupt. */
static void case_clear_variants(void) {
    SimTime bt;
    Bench *b = xa_open(n68_part("SFX-XR16L2552", "{\"ctsRtsClear\":\"iir\",\"specialClear\":\"lsr\"}"),
                       0x70, &bt);
    unsigned i;

    case_begin();
    bus_write(b, 3, 0xBF);
    bus_write(b, 7, 0x2A);
    bus_write(b, 3, 0x03);
    bus_write(b, 4, 0x02);
    bus_write(b, 1, 0x40);
    for (i = 0; i < 4; i++) {
        drive_named(b, "RXA", 0x30 + i, bt);
    }
    run(b, bt);
    expect_reg(b, 2, "11100000", "ISR = CTS/RTS change");
    expect_reg(b, 2, "11000001", "the ISR read cleared it (iir)");
    for (i = 0; i < 4; i++) {
        bus_read(b, 0);
    }
    bus_write(b, 1, 0x20);
    drive_named(b, "RXA", 0x2A, bt);
    run(b, bt);
    expect_reg(b, 5, "01100001", "the special character stored");
    expect_reg(b, 2, "11000001", "the LSR read cleared the special interrupt (lsr)");
    case_end("E6 XR16L2552 variant: ctsRtsClear iir, specialClear lsr");
    bench_close(b);
}

/* Sleep (IER[4], all channels; "sleepNeedsDivisor": the XR16L2552 sleeps only
   with a divisor loaded): with divisor 0 it stays awake, so with loopback on
   (never idle) the 16x clock on MFA (mfPins: AFR[2:1] = 01 BAUDOUT) runs once
   the divisor is loaded; loopback off, both channels sleep, MFA stops (X).
   AFR[2:1] = 11 ("x"): MF X. */
static void case_sleep_divisor(void) {
    Bench *b = bench_open_cfg("SFX-XR16L2552", HALF_18432, &CHSEL_BENCH);
    unsigned u;

    case_begin();
    for (u = 0; u < 2; u++) {
        select_unit(b, u);
        bus_write(b, 3, 0xBF);
        bus_write(b, 2, 0x10);
        bus_write(b, 3, 0x80);
        bus_write(b, 0, 0x00);
        bus_write(b, 1, 0x00);
        bus_write(b, 2, 0x02);   /* AFR: MF = BAUDOUT */
        bus_write(b, 3, 0x03);
    }
    for (u = 0; u < 2; u++) {
        select_unit(b, u);
        bus_read(b, 6);          /* no modem deltas */
        bus_write(b, 1, 0x10);   /* IER[4] with divisor 0 */
    }
    select_unit(b, 0);
    bus_write(b, 4, 0x10);
    for (u = 0; u < 2; u++) {
        select_unit(b, u);
        bus_write(b, 3, 0x80);
        bus_write(b, 0, 0x01);
        bus_write(b, 3, 0x03);
    }
    check(n68_toggles(b, "MFA", 40, NS(70)), "divisor 0 when sleep was asked: awake; MFA runs");
    select_unit(b, 0);
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x06);   /* AFR[2:1] = 11 */
    bus_write(b, 3, 0x03);
    run(b, NS(500));
    expect_pin(b, "MFA", LOGIC_X, "AFR[2:1] = 11 (reserved): MF X");
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x02);
    bus_write(b, 3, 0x03);
    check(n68_toggles(b, "MFA", 40, NS(70)), "AFR[2:1] = 01 again: MFA runs");
    bus_write(b, 4, 0x00);
    run(b, NS(3000));
    expect_pin(b, "MFA", LOGIC_X, "idle, IER[4] on both: asleep, MFA stops");
    case_end("E7 XR16L2552: sleep needs a divisor; MF BAUDOUT and reserved");
    bench_close(b);
}

/* ---- XR16L2552 / XR16M2550 register map corners (exar-d.md XR16V564 Registers: the
   0xBF bank, DLL/DLM/DLD at LCR[7] = 1) ------------------------------------------------- */

/* The bank's Xon/Xoff registers read back at LCR = 0xBF; an LCR written with
   X bits that could make 0xBF makes the bank addresses X; LCR[7] X: 000/001
   read X (DLL or RBR) and 010 (AFR or ISR) is X; EFR 1100 sends both Xoff1
   and Xoff2. */
static void case_l2552_map(void) {
    SimTime bt;
    Bench *b = xa_open("SFX-XR16L2552", 0x1C, &bt);

    case_begin();
    bus_write(b, 3, 0xBF);
    expect_reg(b, 4, "00010001", "Xon1 reads back");
    expect_reg(b, 5, "00010010", "Xon2 reads back");
    expect_reg(b, 6, "00010011", "Xoff1 reads back");
    expect_reg(b, 7, "00010100", "Xoff2 reads back");
    n68_write_x(b, 3, 0xBF, 0x01);
    expect_reg(b, 4, "XXXXXXXX", "LCR 0xBF with b0 X: 100 is Xon1 or MCR, X");
    n68_write_x(b, 3, 0x03, 0x80);
    expect_reg(b, 0, "XXXXXXXX", "LCR[7] X: 000 reads X");
    expect_reg(b, 2, "XXXXXXXX", "LCR[7] X: 010 (AFR or ISR) reads X");
    bus_write(b, 3, 0x03);
    drive_named(b, "RXA", 0x61, bt);
    run(b, bt * 40);
    check(n68_sent_has(b, 0, 0, 0x13) && n68_sent_has(b, 0, 0, 0x14), "EFR[3:2] = 11: Xoff1 and Xoff2 sent");
    case_end("H1 XR16L2552: bank reads, LCR X, EFR 11xx Xoff pair");
    bench_close(b);
}

/* "mcrReset" (docs/parts-format.md) on an Exar part (a variant): MCR reads
   the value after reset. A part "intPin": "out2-gated" (a variant): INT
   3-state while MCR[3] = 0, driven with MCR[3] = 1, X with MCR[3] X. */
static void case_l2552_variants(void) {
    Bench *b = bench_open_cfg(n68_part("SFX-XR16L2552", "{\"mcrReset\":8}"), HALF_18432, &CHSEL_BENCH);

    case_begin();
    select_unit(b, 0);
    expect_reg(b, 4, "00001000", "mcrReset 8: MCR after reset");
    bench_close(b);
    b = bench_open_cfg(n68_part("SFX-XR16L2552", "{\"parts.0.intPin\":\"out2-gated\"}"), HALF_18432,
                       &CHSEL_BENCH);
    select_unit(b, 0);
    program(b, 1, 0x03);
    bus_write(b, 1, 0x02);
    run(b, NS(500));
    expect_pin(b, "INTA", LOGIC_Z, "out2-gated, MCR[3] = 0: INT 3-state");
    bus_write(b, 4, 0x08);
    run(b, NS(500));
    expect_pin(b, "INTA", LOGIC_1, "MCR[3] = 1: INT driven (THR empty)");
    n68_write_x(b, 4, 0x00, 0x08);
    run(b, NS(500));
    expect_pin(b, "INTA", LOGIC_X, "MCR[3] X: INT X");
    case_end("H2 XR16L2552 variants: mcrReset; part intPin out2-gated");
    bench_close(b);
}

/* XR16M2550 DLD (LCR[7] = 1, A = 010, with EFR[4] = 1): EFR[4] written X makes
   010 X. An FCR write with FCR[5:4] X leaves the TX trigger as it was. */
static void case_m2550(void) {
    Bench *b = bench_open_cfg("SFX-XR16M2550-tqfp48", HALF_18432, &DUAL_CD_BENCH);

    case_begin();
    program(b, 1, 0x03);
    bus_write(b, 3, 0xBF);
    n68_write_x(b, 2, 0x00, 0x10);
    bus_write(b, 3, 0x83);
    expect_reg(b, 2, "XXXXXXXX", "EFR[4] X: 010 (DLD or AFR/ISR) is X");
    bus_write(b, 3, 0xBF);
    bus_write(b, 2, 0x10);
    bus_write(b, 3, 0x03);
    bus_write(b, 2, 0x31);   /* TX trigger 11 */
    n68_write_x(b, 2, 0x01, 0x30);
    bus_write(b, 1, 0x02);
    {
        unsigned i;

        for (i = 0; i < 16; i++) {
            bus_write(b, 0, 0x41);
        }
    }
    run(b, bit_time(b, 1) * 10 * 2);
    expect_reg(b, 2, "11000010", "TX trigger kept: the THR interrupt before the FIFO empties");
    expect_reg(b, 5, "00000000", "the FIFO is not empty yet");
    case_end("H3 XR16M2550: EFR[4] X at DLD; FCR[5:4] X ignored");
    bench_close(b);
}

/* ---- XR16C854 / XR16C864 (exar-d.md § XR16C854: FCTR, TRG, FC/FLVL, EMSR) ---------- */

static void c854_bank(Bench *b, unsigned a, unsigned v) {
    bus_write(b, 3, 0xBF);
    bus_write(b, a, v);
    bus_write(b, 3, 0x03);
}

/* FCTR[6] X: 111 (SPR or FLVL/EMSR) is X; FC (0xBF, 000) with FCTR[7] X is X;
   FLVL with EMSR[1:0] X is X; TRG written with FCTR[7] = 1 is table D's TX
   level (the THR interrupt below it); TRG with FCTR[7] X makes table D X. */
static void case_854_regs(void) {
    Bench *b = bench_open_cfg("SFX-XR16C854-qfp100", HALF_18432, &E_C854_BENCH);
    SimTime bt;
    unsigned i;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    c854_bank(b, 2, 0x10);   /* EFR[4] */
    bus_write(b, 3, 0xBF);
    n68_write_x(b, 1, 0x00, 0x40);   /* FCTR[6] X */
    bus_write(b, 3, 0x03);
    expect_reg(b, 7, "XXXXXXXX", "FCTR[6] X: 111 is X");
    bus_write(b, 3, 0xBF);
    n68_write_x(b, 1, 0x00, 0x80);   /* FCTR[7] X */
    expect_reg(b, 0, "XXXXXXXX", "FCTR[7] X: FC is X");
    bus_write(b, 1, 0x40);           /* FCTR[6]: FLVL/EMSR at 111 */
    bus_write(b, 3, 0x03);
    n68_write_x(b, 7, 0x00, 0x03);   /* EMSR[1:0] X */
    expect_reg(b, 7, "XXXXXXXX", "EMSR[1:0] X: FLVL is X");
    bus_write(b, 3, 0xBF);
    bus_write(b, 1, 0xB0);           /* FCTR: TX TRG select, table D */
    bus_write(b, 0, 8);              /* TRG: TX level 8 */
    bus_write(b, 3, 0x03);
    bus_write(b, 2, 0x01);
    bus_write(b, 1, 0x02);
    for (i = 0; i < 20; i++) {
        bus_write(b, 0, 0x41);
    }
    expect_reg(b, 2, "11000001", "20 in the FIFO, TX level 8: no THR interrupt");
    run(b, bt * 10 * 14);
    expect_reg(b, 2, "11000010", "below 8: the THR interrupt");
    expect_reg(b, 5, "00000000", "the FIFO is not empty");
    run(b, bt * 10 * 10);
    bus_write(b, 3, 0xBF);
    n68_write_x(b, 1, 0x30, 0x80);   /* table D, FCTR[7] X */
    bus_write(b, 0, 4);              /* TRG: RX or TX unknown */
    bus_write(b, 3, 0x03);
    bus_write(b, 1, 0x01);
    drive_named(b, "RXA", 0x31, bt);
    run(b, bt);
    expect_reg(b, 2, "XXXXXXXX", "table D RX level X: the RX data interrupt is X");
    case_end("H4 XR16C854: FCTR[6]/[7] X, FLVL X, TRG TX level, TRG X");
    bench_close(b);
}

/* FCTR[5:4] written X: the trigger table is X, so the RX trigger is X (the
   RX data interrupt X) and the TX level falls back to 1. */
static void case_854_table_x(void) {
    Bench *b = bench_open_cfg("SFX-XR16C854-qfp100", HALF_18432, &E_C854_BENCH);
    SimTime bt;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    c854_bank(b, 2, 0x18);   /* EFR[4], TX Xon1/Xoff1 */
    bus_write(b, 3, 0xBF);
    bus_write(b, 6, 0x13);
    n68_write_x(b, 1, 0x00, 0x30);
    bus_write(b, 3, 0x03);
    bus_write(b, 2, 0x01);
    bus_write(b, 1, 0x01);
    drive_named(b, "RXA", 0x31, bt);
    run(b, bt);
    expect_reg(b, 2, "XXXXXXXX", "table X: the RX data interrupt is X");
    run(b, bt * 30);
    check(n68_sent_has(b, 0, 0, 0x13), "table X: Xoff at table A's level (1)");
    case_end("H5 XR16C854: trigger table X");
    bench_close(b);
}

/* IrDA (MCR[6]) with FCTR[2] = 1: active-low IR pulses on RX decode (RX
   inversion option); IRTX (QFP100) is IrDA-encoded: with break every bit
   is a 0, a pulse per bit; RESET X makes IRTX X. */
static void case_854_irda(void) {
    Bench *b = bench_open_cfg("SFX-XR16C854-qfp100", HALF_18432, &E_C854_BENCH);
    SimTime bt;
    unsigned i;
    unsigned pulses = 0;
    LogicValue last = LOGIC_0;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    c854_bank(b, 2, 0x10);
    bus_write(b, 3, 0xBF);
    bus_write(b, 1, 0x04);   /* FCTR[2]: RX inverted */
    bus_write(b, 3, 0x03);
    bus_write(b, 4, 0x40);
    set(b, "RXA", LOGIC_1);
    run(b, bt * 2);
    for (i = 0; i < 10; i++) {   /* start 0, data 0x00, stop 1: low pulses on the 0 bits */
        if (i < 9) {
            set(b, "RXA", LOGIC_0);
            run(b, bt * 3 / 16);
            set(b, "RXA", LOGIC_1);
            run(b, bt - bt * 3 / 16);
        } else {
            run(b, bt);
        }
    }
    run(b, bt * 2);
    expect_reg(b, 0, "00000000", "inverted IR pulses decode as 0x00");
    expect_reg(b, 5, "01100000", "one byte, read");
    bus_write(b, 4, 0x00);
    bus_write(b, 3, 0x43);   /* break */
    bus_write(b, 0, 0xFF);
    for (i = 0; i < 2000; i++) {
        LogicValue v;

        run(b, bt / 100);
        v = get(b, "IRTXA");
        pulses += last == LOGIC_0 && v == LOGIC_1;
        last = v;
    }
    check(pulses >= 9, "break while sending 0xFF: an IR pulse for every bit");
    bus_write(b, 3, 0x03);
    run(b, bt * 12);
    set(b, "RESET", LOGIC_X);
    run(b, NS(500));
    expect_pin(b, "IRTXA", LOGIC_X, "RESET X: IRTX X");
    expect_pin(b, "TXRDY", LOGIC_X, "RESET X: the shared TXRDY X");
    case_end("H6 XR16C854: inverted IrDA RX, IRTX with break, RESET X");
    bench_close(b);
}

/* XR16C864 class X (decisions.md): mode-1 TXRDY low while a TX slot is free;
   a DMA cycle with AEN X is X. */
static void case_864(void) {
    Bench *b = bench_open_cfg("SFX-XR16C864", HALF_18432, &E_C864_BENCH);
    unsigned i;

    case_begin();
    program(b, 1, 0x03);
    bus_write(b, 2, 0x09);   /* FIFO, DMA mode 1 */
    expect_pin(b, "TXRDYA", LOGIC_0, "mode 1, room: TXRDY low");
    for (i = 0; i < 129; i++) {
        bus_write(b, 0, 0x41);
    }
    expect_pin(b, "TXRDYA", LOGIC_1, "the 128-byte FIFO full: TXRDY high");
    set(b, "DACKA", LOGIC_0);
    set(b, "AEN", LOGIC_X);
    run(b, NS(40));
    set(b, b->cfg->read, LOGIC_0);
    run(b, NS(150));
    expect_pin(b, "D0", LOGIC_X, "DACK low with AEN X: the access is X");
    set(b, b->cfg->read, LOGIC_1);
    run(b, NS(200));
    case_end("H7 XR16C864: mode-1 TXRDY, a DMA cycle with AEN X");
    bench_close(b);
}

/* ---- XR16M752 (exar-d.md § XR16M752: TCR/TLR, FIFO Rdy) ---------------------------- */

/* TCR (EFR[4], MCR[6], 110): Xoff sent at the TCR halt level (x4) and Xon at
   resume; MCR[2] X (FIFO Rdy or SPR) makes 111 X; MCR[6] X makes 110 X. */
static void case_m752(void) {
    Bench *b = bench_open_cfg("SFX-XR16M752", HALF_18432, &DUAL_CD_BENCH);
    SimTime bt;
    unsigned i;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 3, 0xBF);
    bus_write(b, 4, 0x11);
    bus_write(b, 6, 0x13);
    bus_write(b, 2, 0x18);
    bus_write(b, 3, 0x03);
    bus_write(b, 2, 0x01);
    bus_write(b, 4, 0x40);
    bus_write(b, 6, 0x01);   /* TCR: halt 4, resume 0 */
    expect_reg(b, 6, "00000001", "TCR reads back");
    bus_write(b, 4, 0x00);
    for (i = 0; i < 4; i++) {
        drive_named(b, "RXA", 0x30 + i, bt);
    }
    run(b, bt * 24);
    check(n68_sent_has(b, 0, 0, 0x13), "4 received, halt 4: Xoff1 sent");
    for (i = 0; i < 4; i++) {
        bus_read(b, 0);
    }
    run(b, bt * 24);
    check(n68_sent_has(b, 0, 0, 0x11), "resume 0: Xon1 sent");
    n68_write_x(b, 4, 0x00, 0x04);
    expect_reg(b, 7, "XXXXXXXX", "MCR[2] X: 111 (FIFO Rdy or SPR) is X");
    n68_write_x(b, 4, 0x00, 0x40);
    expect_reg(b, 6, "XXXXXXXX", "MCR[6] X: 110 (TCR or MSR) is X");
    bus_write(b, 4, 0x00);
    case_end("H8 XR16M752: TCR Xoff/Xon levels, MCR[2]/[6] X");
    bench_close(b);
}

/* ---- Odds and ends ----------------------------------------------------------------------- */

/* XR16C864 DMA write cycle (DACK low, AEN high, IOW): the byte goes to the
   channel's TX FIFO whatever LCR[7] is (decisions.md). */
static void case_864_dma_write(void) {
    Bench *b = bench_open_cfg("SFX-XR16C864", HALF_18432, &E_C864_BENCH);

    case_begin();
    program(b, 1, 0x03);
    bus_write(b, 3, 0x83);
    set(b, "DACKA", LOGIC_0);
    set(b, "AEN", LOGIC_1);
    run(b, NS(40));
    set_data(b, 1, 0x5A);
    run(b, NS(40));
    set(b, "IOW", LOGIC_0);
    run(b, NS(120));
    set(b, "IOW", LOGIC_1);
    run(b, NS(40));
    set_data(b, 0, 0);
    set(b, "DACKA", LOGIC_1);
    set(b, "AEN", LOGIC_0);
    run(b, NS(200));
    bus_write(b, 3, 0x03);
    run(b, bit_time(b, 1) * 12);
    check(n68_sent_has(b, 0, 0, 0x5A), "the DMA write with LCR[7] = 1 was sent");
    case_end("H9 XR16C864: a DMA write cycle");
    bench_close(b);
}

/* ST16C1450 RST output with an active-low RESET (a variant): RST = RESET
   asserted OR the software reset. XR16L2552 IrDA (MCR[6]) with an X break
   bit: TX X. XR16M752 FIFO Rdy b4: channel A has data (RXRDY active). */
static void case_misc_pins(void) {
    Bench *b = bench_open_cfg(n68_part("SFX-ST16C1450", "{\"bus.resetActiveHigh\":false}"),
                              HALF_18432, &E_ONE_BENCH);
    SimTime bt;

    case_begin();
    set(b, "RESET", LOGIC_1);
    run(b, NS(500));
    expect_pin(b, "RST", LOGIC_0, "RESET high (inactive): RST low");
    set(b, "RESET", LOGIC_0);
    run(b, NS(500));
    expect_pin(b, "RST", LOGIC_1, "RESET low (active): RST high");
    set(b, "RESET", LOGIC_1);
    run(b, NS(500));
    bench_close(b);
    b = xa_open("SFX-XR16L2552", 0x10, &bt);
    bus_write(b, 4, 0x40);
    n68_write_x(b, 3, 0x03, 0x40);
    run(b, NS(500));
    expect_pin(b, "TXA", LOGIC_X, "IrDA, break bit X: TX X");
    bench_close(b);
    b = bench_open_cfg("SFX-XR16M752", HALF_18432, &DUAL_CD_BENCH);
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    drive_named(b, "RXA", 0x41, bt);
    run(b, bt);
    bus_write(b, 4, 0x04);
    expect_reg(b, 7, "00010011", "FIFO Rdy: RX A has data, both TX empty");
    case_end("H10 ST16C1450 variant RST, XR16L2552 IrDA break X, XR16M752 FIFO Rdy RX");
    bench_close(b);
}

/* Motorola mode (16/68 low, XR16L2551): the channel bit (A3 on CSB) X makes
   the access X. */
static void case_moto_x(void) {
    Bench *b = bench_open_cfg("SFX-XR16L2551", HALF_18432, &E_DUAL_MOTO_BENCH);
    char text[9];

    case_begin();
    set(b, "RESET", LOGIC_1);   /* Motorola: RESET is active low */
    set(b, "16/68", LOGIC_0);
    set(b, "CSA", LOGIC_1);
    set(b, "CSB", LOGIC_0);
    run(b, NS(500));
    m_write(b, "CSA", 7, 0xA5);
    m_expect(b, "CSA", 7, "10100101", "Motorola, A3 = 0: channel A's SPR");
    set(b, "CSB", LOGIC_X);
    run(b, NS(50));
    m_read_text(b, "CSA", 7, text);
    check(strcmp(text, "XXXXXXXX") == 0, "Motorola, channel bit X: the read is X");
    case_end("H11 XR16L2551 Motorola: channel bit X");
    bench_close(b);
}

/* An 854-style table D TX level above the FIFO size (a variant with a 64-byte
   FIFO, TRG written 100): the level is the whole FIFO, so the THR
   interrupt waits for the FIFO to empty (the trigger is "below 64"). */
static void case_854_trg_big(void) {
    Bench *b = bench_open_cfg(n68_part("SFX-XR16C854", "{\"fifo\":64,\"rxTrigger\":[1,4,8,14],"
                                       "\"triggerTables.rx.2\":[8,16,56,60],\"triggerTables.tx.2\":[8,16,32,56]}"),
                              HALF_18432, &QUAD_C_BENCH);
    unsigned i;

    case_begin();
    program(b, 1, 0x03);
    c854_bank(b, 2, 0x10);
    bus_write(b, 3, 0xBF);
    bus_write(b, 1, 0xB0);   /* TX TRG, table D */
    bus_write(b, 0, 100);
    bus_write(b, 3, 0x03);
    bus_write(b, 2, 0x01);
    for (i = 0; i < 64; i++) {
        bus_write(b, 0, 0x41);
    }
    bus_write(b, 1, 0x02);
    run(b, bit_time(b, 1) * 10 * 2);
    expect_reg(b, 2, "11000010", "TX level 100 > 64: below the FIFO size, the THR interrupt");
    case_end("H12 XR16C854 variant: TX level above the FIFO size");
    bench_close(b);
}

int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_pairs();
    case_modes();
    case_special_and_errors();
    case_send_xoff();
    case_ctsrts();
    case_clear_variants();
    case_sleep_divisor();
    case_l2552_map();
    case_l2552_variants();
    case_m2550();
    case_854_regs();
    case_854_table_x();
    case_854_irda();
    case_864();
    case_m752();
    case_864_dma_write();
    case_misc_pins();
    case_moto_x();
    case_854_trg_big();
    return serial_bench_finish();
}
