/*
 * serial_acia_test: the CDP65C51 and W65C51N (H6, plan-14-E)
 * (split from serial_test.c by plan-68-G; the bench is support/serial_bench.c).
 */

#include "serial_bench.h"
#include "serial_bench_classic.h"

/* ---- H6: CDP65C51 (acia6551) ----------------------------------------------------------- */

static const char *const AC_D[8] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"};

/* XTLI 1.8432 MHz: 542.534 ns. */


/* One PHI2 cycle on register rs: write v, or read (returns the bus as text). */
static void ac_cycle(Bench *b, unsigned rs, int write, unsigned v, char *text) {
    set(b, "RS0", rs & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "RS1", rs & 2u ? LOGIC_1 : LOGIC_0);
    set(b, "RW", write ? LOGIC_0 : LOGIC_1);
    set(b, "CS0", LOGIC_1);
    set(b, "CS1", LOGIC_0);
    if (write) {
        h_set_bus(b, AC_D, 8, 1, v);
    }
    run(b, NS(100));
    set(b, "PHI2", LOGIC_1);
    run(b, NS(400));
    if (!write && text != NULL) {
        h_get_bus(b, AC_D, 8, text);
    }
    set(b, "PHI2", LOGIC_0);
    run(b, NS(100));
    set(b, "CS0", LOGIC_0);
    set(b, "RW", LOGIC_1);
    h_set_bus(b, AC_D, 8, 0, 0);
    run(b, NS(400));
}

static void ac_expect(Bench *b, unsigned rs, const char *want, const char *what) {
    char text[16];

    ac_cycle(b, rs, 0, 0, text);
    h_expect_text(text, want, what);
}

/* The period between two RXC rising edges (the 16x clock out). */
static SimTime ac_rxc_period(Bench *b, SimTime limit) {
    SimTime t0 = 0;
    SimTime spent = 0;
    LogicValue last = get(b, "RXC");
    int seen = 0;
    SimTime step = NS(20);

    while (spent < limit) {
        LogicValue now;

        run(b, step);
        spent += step;
        now = get(b, "RXC");
        if (last == LOGIC_0 && now == LOGIC_1) {
            if (seen) {
                return sim_now(b->sim) - t0;
            }
            seen = 1;
            t0 = sim_now(b->sim);
        }
        last = now;
    }
    return 0;
}

static void case_h6_acia6551(void) {
    static const HClockPin CLOCKS[1] = {{"XTLI", 0}};
    static const uint32_t DIV[16] = {
        0, 36864, 24576, 16768, 13696, 12288, 6144, 3072, 1536, 1024, 768, 512, 384, 256, 192, 96
    };
    SimTime half = AC_XHALF;
    HBench *h = hb_open("SFX-CDP65C51-so28", CLOCKS, 1, &half, 1);
    Bench *b = &h->b;
    const SimTime xper = 2 * AC_XHALF;
    const SimTime bt19200 = 96 * xper;
    char got[24];
    char want[24];
    unsigned code;

    case_begin();
    set(b, "RES", LOGIC_0);
    set(b, "CS0", LOGIC_0);
    set(b, "CS1", LOGIC_1);
    set(b, "PHI2", LOGIC_0);
    set(b, "RW", LOGIC_1);
    set(b, "RS0", LOGIC_0);
    set(b, "RS1", LOGIC_0);
    set(b, "RXD", LOGIC_1);
    set(b, "CTS", LOGIC_0);
    set(b, "DCD", LOGIC_0);
    set(b, "DSR", LOGIC_0);
    run(b, NS(2000));
    set(b, "RES", LOGIC_1);
    run(b, NS(1000));
    ac_expect(b, 1, "00010000", "status after RES (TDRE; DCD, DSR low)");
    ac_expect(b, 2, "00000000", "command after RES");
    ac_expect(b, 3, "00000000", "control after RES");
    expect_pin(b, "DTR", LOGIC_1, "DTR high after RES");
    expect_pin(b, "RTS", LOGIC_1, "RTS high after RES");
    expect_pin(b, "IRQ", LOGIC_Z, "IRQ released after RES");

    /* The internal baud table: RXC (RCS = 1) is the 16x clock. */
    for (code = 1; code < 16; code++) {
        SimTime want_period = (SimTime)(DIV[code] / 16) * xper;
        SimTime got_period;
        char message[96];

        ac_cycle(b, 3, 1, 0x10 | code, NULL);
        got_period = ac_rxc_period(b, 3 * want_period + NS(10000));
        snprintf(message, sizeof message, "SBR %u: 16x period %llu ps, want %llu", code,
                 (unsigned long long)got_period, (unsigned long long)want_period);
        check(got_period + NS(40) >= want_period && got_period <= want_period + NS(40), message);
    }

    /* 19200 8N1, TIC = 10 (RTS low, no TX interrupt), IRD = 1. */
    ac_cycle(b, 3, 1, 0x1F, NULL);
    ac_cycle(b, 2, 1, 0x0B, NULL);
    expect_pin(b, "DTR", LOGIC_0, "command b0: DTR low");
    expect_pin(b, "RTS", LOGIC_0, "TIC 10: RTS low");
    expect_pin(b, "IRQ", LOGIC_Z, "TIC 10: no TDRE interrupt");
    ac_cycle(b, 0, 1, 'A', NULL);
    check(h_catch(b, "TXD", bt19200, 9, 4 * bt19200, got), "19200 frame");
    h_frame_bits('A', 8, 0, 1, want);
    h_expect_text(got, want, "19200 8N1 A");
    run(b, bt19200);
    h_expect_sent(b, 0, 'A', "A logged");

    /* TDRE interrupt as a level (planner decision): TIC = 01. */
    ac_cycle(b, 2, 1, 0x05, NULL);
    run(b, NS(1000));
    expect_pin(b, "IRQ", LOGIC_0, "TIC 01: IRQ while TDRE");
    ac_expect(b, 1, "10010000", "status: IRQ, TDRE");
    expect_pin(b, "IRQ", LOGIC_0, "TDRE interrupt recurs after the status read (level)");
    ac_cycle(b, 2, 1, 0x0B, NULL);
    run(b, NS(1000));
    expect_pin(b, "IRQ", LOGIC_Z, "TIC 10 releases it");

    /* Receive with the RX interrupt (IRD = 0); PE, FE, overrun. */
    ac_cycle(b, 2, 1, 0x09, NULL);
    h_frame_bits(0x3C, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt19200);
    run(b, bt19200);
    expect_pin(b, "IRQ", LOGIC_0, "RDRF interrupt");
    ac_expect(b, 1, "10011000", "status: IRQ, TDRE, RDRF");
    expect_pin(b, "IRQ", LOGIC_Z, "the status read releases IRQ");
    ac_expect(b, 0, "00111100", "RDR");
    ac_expect(b, 1, "00010000", "RDRF cleared by the RDR read");
    h_drive_bits(b, "RXD", want, bt19200);
    run(b, bt19200);
    h_frame_bits(0x55, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt19200);
    run(b, bt19200);
    ac_expect(b, 1, "10011100", "status: IRQ (the second byte), overrun (the third is not taken)");
    ac_expect(b, 0, "00111100", "RDR keeps the last valid character");
    /* Programmed reset: OVR cleared, command b4..b0 = 0. */
    ac_cycle(b, 1, 1, 0x00, NULL);
    ac_expect(b, 1, "00010000", "programmed reset clears OVR");
    ac_expect(b, 2, "00000000", "programmed reset: command b4..b0 = 0");
    expect_pin(b, "DTR", LOGIC_1, "programmed reset: DTR high");
    /* Even parity with a wrong parity bit; a framing error. */
    ac_cycle(b, 2, 1, 0x6B, NULL);   /* even, parity on, TIC 10, IRD, DTR */
    h_frame_bits(0x07, 8, 'o', 1, want);
    h_drive_bits(b, "RXD", want, bt19200);
    run(b, bt19200);
    ac_expect(b, 1, "00011001", "status: RDRF, PE");
    ac_cycle(b, 0, 0, 0, NULL);
    h_frame_bits(0x07, 8, 'e', 0, want);
    strcat(want, "0");
    h_drive_bits(b, "RXD", want, bt19200);
    run(b, 2 * bt19200);
    ac_expect(b, 1, "00011010", "status: RDRF, FE");
    ac_cycle(b, 0, 0, 0, NULL);
    ac_cycle(b, 2, 1, 0x0B, NULL);

    /* DCD / DSR: an interrupt on a change; the bits hold until read. */
    ac_cycle(b, 2, 1, 0x09, NULL);
    set(b, "DSR", LOGIC_1);
    run(b, NS(500));
    expect_pin(b, "IRQ", LOGIC_0, "DSR change interrupts");
    set(b, "DSR", LOGIC_0);
    run(b, NS(500));
    ac_expect(b, 1, "11010010", "status: IRQ, DSR held at the changed level");
    expect_pin(b, "IRQ", LOGIC_0, "the change back interrupts again at the read");
    ac_expect(b, 1, "10010010", "status: DSR now low (FE of the last word)");
    ac_expect(b, 1, "00010010", "and released");
    ac_cycle(b, 2, 1, 0x0B, NULL);

    /* Echo: TXD repeats RXD half a bit later (REM, TIC 00). */
    ac_cycle(b, 2, 1, 0x13, NULL);
    expect_pin(b, "RTS", LOGIC_0, "echo: RTS low");
    set(b, "RXD", LOGIC_0);
    run(b, bt19200 / 4);
    expect_pin(b, "TXD", LOGIC_1, "echo: not yet (quarter bit)");
    run(b, bt19200 / 2);
    expect_pin(b, "TXD", LOGIC_0, "echo: after half a bit");
    set(b, "RXD", LOGIC_1);
    run(b, 12 * bt19200);
    ac_cycle(b, 0, 0, 0, NULL);
    ac_cycle(b, 2, 1, 0x0B, NULL);

    /* CDP65C51 CTS: TXD marks at once; TDRE does not show in the status. */
    ac_cycle(b, 0, 1, 0x00, NULL);
    check(h_catch(b, "TXD", bt19200, 0, 2 * bt19200, got), "a zero byte starts");
    set(b, "CTS", LOGIC_1);
    run(b, NS(2000));
    expect_pin(b, "TXD", LOGIC_1, "CTS high: TXD marks at once");
    ac_expect(b, 1, "00000000", "CTS high: TDRE not shown");
    set(b, "CTS", LOGIC_0);
    run(b, 12 * bt19200);

    /* Break (TIC 11): low once both registers are empty. */
    ac_cycle(b, 2, 1, 0x0F, NULL);
    run(b, bt19200);
    expect_pin(b, "TXD", LOGIC_0, "TIC 11: break");
    ac_cycle(b, 2, 1, 0x0B, NULL);
    run(b, bt19200 / 2);
    expect_pin(b, "TXD", LOGIC_1, "a stop bit after the break");

    /* SBR 0000: XTLI is the 16x clock. */
    ac_cycle(b, 3, 1, 0x10, NULL);
    ac_cycle(b, 0, 1, 0x81, NULL);
    check(h_catch(b, "TXD", 16 * xper, 9, 40 * xper, got), "16x external frame");
    h_frame_bits(0x81, 8, 0, 1, want);
    h_expect_text(got, want, "SBR 0000: 16 XTLI periods a bit");
    run(b, 32 * xper);

    /* Terminal at 19200. */
    ac_cycle(b, 3, 1, 0x1F, NULL);
    set(b, "RXD", LOGIC_Z);
    run(b, NS(100));
    check(h_send(b, 0, "OK") == 1, "terminal send accepted");
    run(b, 11 * bt19200);
    ac_expect(b, 0, "01001111", "terminal O");
    run(b, 11 * bt19200);
    ac_expect(b, 0, "01001011", "terminal K");
    case_end("H6 CDP65C51: baud table at 1.8432 MHz, TDRE level IRQ, RX/errors, programmed reset, DCD/DSR, echo, CTS, break, terminal");
    hb_close(h);
}

/* ---- plan-14-E: W65C51N (acia6551 with "w65c51n") ------------------------------------------ */

/* One PHI2 cycle on the W65C51N's register rs (CS1B, RWB): write v, or read
   (the bus as text). */
static void wn_cycle(Bench *b, unsigned rs, int write, unsigned v, char *text) {
    set(b, "RS0", rs & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "RS1", rs & 2u ? LOGIC_1 : LOGIC_0);
    set(b, "RWB", write ? LOGIC_0 : LOGIC_1);
    set(b, "CS0", LOGIC_1);
    set(b, "CS1B", LOGIC_0);
    if (write) {
        h_set_bus(b, AC_D, 8, 1, v);
    }
    run(b, NS(100));
    set(b, "PHI2", LOGIC_1);
    run(b, NS(400));
    if (!write && text != NULL) {
        h_get_bus(b, AC_D, 8, text);
    }
    set(b, "PHI2", LOGIC_0);
    run(b, NS(100));
    set(b, "CS0", LOGIC_0);
    set(b, "RWB", LOGIC_1);
    h_set_bus(b, AC_D, 8, 0, 0);
    run(b, NS(400));
}

static void wn_expect(Bench *b, unsigned rs, const char *want, const char *what) {
    char text[16];

    wn_cycle(b, rs, 0, 0, text);
    h_expect_text(text, want, what);
}

/* A W65C51N6TPG-14 on 1.8432 MHz XTLI, reset, then 19200 8N1 with DTR and
   TIC = 10 (RTS low, no TX interrupt), IRD = 1. */
static HBench *wn_open(void) {
    static const HClockPin CLOCKS[1] = {{"XTLI", 0}};
    SimTime half = AC_XHALF;
    HBench *h = hb_open("W65C51N6TPG-14", CLOCKS, 1, &half, 1);
    Bench *b = &h->b;

    set(b, "RESB", LOGIC_0);
    set(b, "CS0", LOGIC_0);
    set(b, "CS1B", LOGIC_1);
    set(b, "PHI2", LOGIC_0);
    set(b, "RWB", LOGIC_1);
    set(b, "RS0", LOGIC_0);
    set(b, "RS1", LOGIC_0);
    set(b, "RxD", LOGIC_1);
    set(b, "CTSB", LOGIC_0);
    set(b, "DCDB", LOGIC_0);
    set(b, "DSRB", LOGIC_0);
    run(b, NS(2000));
    set(b, "RESB", LOGIC_1);
    run(b, NS(1000));
    return h;
}

static void wn_19200(Bench *b) {
    wn_cycle(b, 3, 1, 0x1F, NULL);   /* 1 stop, 8 bits, baud generator, 19200 */
    wn_cycle(b, 2, 1, 0x0B, NULL);   /* TIC 10, IRD, DTR */
}

#define WN_BT (96 * 2 * AC_XHALF)    /* 19200: 52.08 us */

static void case_e1_w65c51n_tdre(void) {
    HBench *h = wn_open();
    Bench *b = &h->b;

    case_begin();
    wn_expect(b, 1, "00010000", "status after RESB: TDRE");
    wn_19200(b);
    wn_cycle(b, 0, 1, 'A', NULL);
    wn_expect(b, 1, "00010000", "TDRE 1 right after the TDR write");
    check(h_wait(b, "TxD", LOGIC_0, 2 * WN_BT), "the frame starts");
    run(b, 3 * WN_BT);
    wn_expect(b, 1, "00010000", "TDRE 1 during the frame");
    run(b, 8 * WN_BT);
    wn_expect(b, 1, "00010000", "TDRE 1 after the frame");
    h_expect_sent(b, 0, 'A', "A logged");
    case_end("E1 W65C51N: TDRE reads 1 after reset, during and after a frame");
    hb_close(h);
}

static void case_e2_w65c51n_cut(void) {
    HBench *h = wn_open();
    Bench *b = &h->b;
    const SimComponentType *type;
    SimTime t0;
    char got[16];
    char want[16];
    unsigned i;

    case_begin();
    wn_19200(b);
    wn_cycle(b, 0, 1, 'A', NULL);
    check(h_wait(b, "TxD", LOGIC_0, 2 * WN_BT), "A's start bit");
    t0 = sim_now(b->sim);
    /* 'A' = 0x41: bit period 7 (data bit 6) is a 1. Write 'B' in period 6;
       at the boundary B's start bit (0) begins instead. */
    run(b, 6 * WN_BT + WN_BT / 2);
    wn_cycle(b, 0, 1, 'B', NULL);
    run(b, t0 + 7 * WN_BT + WN_BT / 2 - sim_now(b->sim));
    got[0] = "01XZ"[get(b, "TxD")];
    for (i = 1; i <= 9; i++) {
        run(b, WN_BT);
        got[i] = "01XZ"[get(b, "TxD")];
    }
    got[10] = '\0';
    h_frame_bits('B', 8, 0, 1, want);
    h_expect_text(got, want, "B starts at the next bit boundary");
    run(b, 2 * WN_BT);
    type = sim_instance_type(b->chip);
    check(type->serial_sent(sim_instance_state(b->chip), 0, 0, NULL, 0) == 1,
          "one frame logged (A was cut off)");
    h_expect_sent(b, 0, 'B', "the log holds B");
    case_end("E2 W65C51N: a TDR write mid-frame cuts it off at the next bit boundary");
    hb_close(h);
}

static void case_e3_w65c51n_no_parity(void) {
    HBench *h = wn_open();
    Bench *b = &h->b;
    char got[24];
    char want[24];

    case_begin();
    wn_cycle(b, 3, 1, 0x1F, NULL);
    wn_cycle(b, 2, 1, 0x2B, NULL);   /* PME set (ignored), TIC 10, IRD, DTR */
    wn_cycle(b, 0, 1, 'C', NULL);
    check(h_catch(b, "TxD", WN_BT, 10, 4 * WN_BT, got), "frame");
    h_frame_bits('C', 8, 0, 1, want);
    strcat(want, "1");               /* then mark: no parity bit */
    h_expect_text(got, want, "10-bit frame, no parity bit");
    /* Receive an 8N1 frame whose 10th bit would be a bad parity bit. */
    h_frame_bits(0x07, 8, 0, 1, want);
    h_drive_bits(b, "RxD", want, WN_BT);
    run(b, WN_BT);
    wn_expect(b, 1, "00011000", "status: TDRE, RDRF; PE 0");
    wn_expect(b, 0, "00000111", "RDR");
    case_end("E3 W65C51N: no parity: 10-bit frames, PE reads 0");
    hb_close(h);
}

static void case_e4_w65c51n_tic01(void) {
    HBench *h = wn_open();
    Bench *b = &h->b;

    case_begin();
    wn_cycle(b, 3, 1, 0x1F, NULL);
    expect_pin(b, "IRQB", LOGIC_Z, "IRQB released");
    wn_cycle(b, 2, 1, 0x05, NULL);   /* TIC 01 ("do not use"), DTR */
    run(b, NS(500));
    expect_pin(b, "IRQB", LOGIC_0, "TIC 01: IRQB low at once (TDRE is always 1)");
    wn_expect(b, 1, "10010000", "status read: bit 7 set again at once");
    run(b, NS(500));
    expect_pin(b, "IRQB", LOGIC_0, "still low after the status read");
    wn_cycle(b, 2, 1, 0x0B, NULL);
    run(b, NS(1000));
    expect_pin(b, "IRQB", LOGIC_Z, "TIC 10 releases it");
    case_end("E4 W65C51N: TIC 01 interrupts at once and again after every status read");
    hb_close(h);
}

static void case_e5_w65c51n_cts(void) {
    HBench *h = wn_open();
    Bench *b = &h->b;
    unsigned i;
    int marking = 1;

    case_begin();
    wn_19200(b);
    wn_cycle(b, 0, 1, 0x00, NULL);   /* all data bits 0 */
    check(h_wait(b, "TxD", LOGIC_0, 2 * WN_BT), "the frame starts");
    run(b, 3 * WN_BT);
    expect_pin(b, "TxD", LOGIC_0, "a 0 data bit");
    set(b, "CTSB", LOGIC_1);
    run(b, NS(1000));
    expect_pin(b, "TxD", LOGIC_1, "CTSB high: TxD marks at once");
    for (i = 0; i < 8; i++) {
        run(b, WN_BT);
        marking &= get(b, "TxD") == LOGIC_1;
    }
    check(marking, "TxD stays marking while CTSB is high");
    case_end("E5 W65C51N: CTSB high stops the transmitter at once");
    hb_close(h);
}

/* The time from a falling edge on pin to its next rising edge (50 ns steps). */
static SimTime wn_low_time(Bench *b, const char *pin, SimTime limit) {
    SimTime t0;

    if (!h_wait(b, pin, LOGIC_0, limit)) return 0;
    t0 = sim_now(b->sim);
    if (!h_wait(b, pin, LOGIC_1, limit)) return 0;
    return sim_now(b->sim) - t0;
}

static void case_e6_w65c51n_baud(void) {
    HBench *h = wn_open();
    Bench *b = &h->b;
    SimTime t;
    char message[96];

    case_begin();
    wn_19200(b);
    wn_cycle(b, 0, 1, 0x55, NULL);   /* start 0, then 1 0 1 0 ...: one-bit lows */
    t = wn_low_time(b, "TxD", 4 * WN_BT);
    snprintf(message, sizeof message, "0x1F: bit %llu ps, want 52083 ns",
             (unsigned long long)t);
    check(t >= NS(52083) - NS(100) && t <= NS(52083) + NS(100), message);
    run(b, 12 * WN_BT);
    wn_cycle(b, 3, 1, 0x10, NULL);   /* SBR 0000: XTLI / 16 */
    wn_cycle(b, 0, 1, 0x55, NULL);
    t = wn_low_time(b, "TxD", NS(200000));
    snprintf(message, sizeof message, "0x10: bit %llu ps, want 8681 ns (115200)",
             (unsigned long long)t);
    check(t >= NS(8681) - NS(100) && t <= NS(8681) + NS(100), message);
    case_end("E6 W65C51N: 19200 (0x1F) and XTLI/16 = 115200 (0x10) bit times");
    hb_close(h);
}

static void case_e7_w65c51n_terminal(void) {
    HBench *h = wn_open();
    Bench *b = &h->b;

    case_begin();
    wn_19200(b);
    set(b, "RxD", LOGIC_Z);
    run(b, NS(100));
    check(h_send(b, 0, "OK") == 1, "terminal send accepted");
    run(b, 11 * WN_BT);
    wn_expect(b, 0, "01001111", "terminal O");
    run(b, 11 * WN_BT);
    wn_expect(b, 0, "01001011", "terminal K");
    wn_cycle(b, 0, 1, 'A', NULL);
    run(b, 12 * WN_BT);
    h_expect_sent(b, 0, 'A', "a sent A is in the log");
    case_end("E7 W65C51N: the terminal types OK into RxD; a sent A is logged");
    hb_close(h);
}

static void case_e8_w65c51n_overrun(void) {
    HBench *h = wn_open();
    Bench *b = &h->b;
    char want[24];

    case_begin();
    wn_19200(b);
    run(b, WN_BT);                   /* the receiver sees mark first */
    h_frame_bits('H', 8, 0, 1, want);
    h_drive_bits(b, "RxD", want, WN_BT);
    run(b, WN_BT);
    h_frame_bits('I', 8, 0, 1, want);
    h_drive_bits(b, "RxD", want, WN_BT);
    run(b, WN_BT);
    wn_expect(b, 1, "00011100", "status: TDRE, RDRF, overrun");
    wn_expect(b, 0, "01001001", "RDR holds the second byte (I)");
    wn_expect(b, 1, "00010000", "the RDR read cleared RDRF and the overrun");
    case_end("E8 W65C51N: an overrun transfers the new byte; an RDR read clears OVR");
    hb_close(h);
}



int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_h6_acia6551();
    case_e1_w65c51n_tdre();
    case_e2_w65c51n_cut();
    case_e3_w65c51n_no_parity();
    case_e4_w65c51n_tic01();
    case_e5_w65c51n_cts();
    case_e6_w65c51n_baud();
    case_e7_w65c51n_terminal();
    case_e8_w65c51n_overrun();
    return serial_bench_finish();
}
