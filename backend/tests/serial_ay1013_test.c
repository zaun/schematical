/*
 * serial_ay1013_test: the AY-5-1013A family (H1-H3)
 * (split from serial_test.c by plan-68-G; the bench is support/serial_bench.c).
 */

#include "serial_bench.h"
#include "serial_bench_classic.h"

/* ---- H1: AY-5-1013A (ay1013, pin-parallel) ---------------------------------------- */

static const char *const AY_DB[8] = {"DB1", "DB2", "DB3", "DB4", "DB5", "DB6", "DB7", "DB8"};
static const char *const AY_RD[8] = {"RD1", "RD2", "RD3", "RD4", "RD5", "RD6", "RD7", "RD8"};
static const char *const AY_STATUS[5] = {"PE", "FE", "OR", "DAV", "TBMT"};

/* 1 MHz on TCP and RCP: a 16 us bit. */
#define AY_HALF NS(500)
#define AY_BIT NS(16000)

static HBench *ay_open(const char *part) {
    static const HClockPin CLOCKS[2] = {{"TCP", 0}, {"RCP", 0}};
    SimTime half = AY_HALF;
    HBench *h = hb_open(part, CLOCKS, 2, &half, 1);
    Bench *b = &h->b;

    set(b, "XR", LOGIC_1);
    set(b, "RDE", LOGIC_0);
    set(b, "SWE", LOGIC_0);
    set(b, "RDAV", LOGIC_1);
    set(b, "DS", LOGIC_1);
    set(b, "SI", LOGIC_1);
    set(b, "CS", LOGIC_0);
    set(b, "NP", LOGIC_1);
    set(b, "TSB", LOGIC_0);
    set(b, "NB1", LOGIC_1);
    set(b, "NB2", LOGIC_1);
    set(b, "EPS", LOGIC_0);
    h_set_bus(b, AY_DB, 8, 1, 0);
    run(b, NS(2000));
    set(b, "XR", LOGIC_0);
    run(b, NS(2000));
    return h;
}

/* Loads the control word: nb data bits, np, eps, tsb (CS pulse). */
static void ay_control(Bench *b, unsigned nb, int np, int eps, int tsb) {
    set(b, "NB1", (nb - 5) & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "NB2", (nb - 5) & 2u ? LOGIC_1 : LOGIC_0);
    set(b, "NP", np ? LOGIC_1 : LOGIC_0);
    set(b, "EPS", eps ? LOGIC_1 : LOGIC_0);
    set(b, "TSB", tsb ? LOGIC_1 : LOGIC_0);
    run(b, NS(100));
    set(b, "CS", LOGIC_1);
    run(b, NS(400));
    set(b, "CS", LOGIC_0);
    run(b, NS(100));
}

static void ay_strobe(Bench *b, unsigned v) {
    h_set_bus(b, AY_DB, 8, 1, v);
    run(b, NS(100));
    set(b, "DS", LOGIC_0);
    run(b, NS(300));
    set(b, "DS", LOGIC_1);
    run(b, NS(100));
}

/* Sends v and checks the frame and the time from the start bit to EOC (in
   half-bit units, so 1.5 stops can be told from 2). */
static void ay_tx_check(Bench *b, unsigned v, unsigned nb, char parity, unsigned stop_halves,
                        const char *what) {
    char want[24];
    char got[24];
    SimTime t0;
    SimTime spent = 0;
    unsigned bits = 1 + nb + (parity != 0);
    unsigned halves;
    char message[160];

    ay_strobe(b, v);
    if (!h_catch(b, "SO", AY_BIT, bits - 1, 4 * AY_BIT, got)) {
        check(0, what);
        return;
    }
    h_frame_bits(v, nb, parity, 0, want);
    t0 = sim_now(b->sim) - (SimTime)(bits - 1) * AY_BIT - AY_BIT / 2;
    snprintf(message, sizeof message, "%s frame", what);
    h_expect_text(got, want, message);
    while (get(b, "EOC") != LOGIC_1 && spent < 4 * AY_BIT) {
        run(b, NS(250));
        spent += NS(250);
    }
    halves = (unsigned)((sim_now(b->sim) - t0 + AY_BIT / 4) / (AY_BIT / 2));
    snprintf(message, sizeof message, "%s: EOC after %u half bits, want %u", what, halves,
             2 * bits + stop_halves);
    check(halves == 2 * bits + stop_halves, message);
}

static void ay_rx(Bench *b, unsigned v, unsigned nb, char parity, int stop) {
    char bits[24];

    h_frame_bits(v, nb, parity, 0, bits);
    strcat(bits, stop ? "1" : "0");
    h_drive_bits(b, "SI", bits, AY_BIT);
    run(b, AY_BIT * 2);
}

static void case_h1_ay1013(void) {
    HBench *h = ay_open("SFX-AY-5-1013A-dip40");
    Bench *b = &h->b;
    static const char PARITY[3] = {0, 'o', 'e'};
    unsigned nb;
    unsigned p;
    unsigned tsb;
    char text[64];

    case_begin();
    /* After XR: TBMT, EOC, SO high; DAV, OR, FE, PE low; RHR not cleared (X). */
    h_expect_bus(b, AY_STATUS, 5, "10000", "status after XR (TBMT DAV OR FE PE)");
    expect_pin(b, "EOC", LOGIC_1, "after XR");
    expect_pin(b, "SO", LOGIC_1, "after XR");
    h_expect_bus(b, AY_RD, 8, "XXXXXXXX", "RHR at power-on (XR does not clear it)");

    /* Every word length x parity x stop combination (5-bit TSB = 2 stops). */
    for (nb = 5; nb <= 8; nb++) {
        for (p = 0; p < 3; p++) {
            for (tsb = 0; tsb < 2; tsb++) {
                char what[64];

                snprintf(what, sizeof what, "TX %u%c%u", nb, p == 0 ? 'N' : p == 1 ? 'O' : 'E',
                         tsb ? 2u : 1u);
                ay_control(b, nb, p == 0, p == 2, (int)tsb);
                ay_tx_check(b, 0xA5 & ((1u << nb) - 1), nb, PARITY[p], tsb ? 4u : 2u, what);
            }
        }
    }
    /* The control latch: CS low holds the word while the pins change. */
    ay_control(b, 8, 1, 0, 0);
    set(b, "NB1", LOGIC_0);
    set(b, "NB2", LOGIC_0);
    set(b, "NP", LOGIC_0);
    run(b, NS(200));
    ay_tx_check(b, 0x81, 8, 0, 2, "latched 8N1 with the pins at 5O");
    /* CS high: transparent. */
    set(b, "CS", LOGIC_1);
    run(b, NS(200));
    ay_tx_check(b, 0x15, 5, 'o', 2, "CS high: the pins act directly (5O1)");
    set(b, "CS", LOGIC_0);
    ay_control(b, 8, 1, 0, 0);

    /* TBMT: low after DS, high when the shift register takes the byte; a
       second byte waits (double buffer). */
    ay_strobe(b, 0x31);
    run(b, AY_BIT / 4);
    ay_strobe(b, 0x32);
    run(b, NS(600));
    expect_pin(b, "TBMT", LOGIC_0, "second byte waiting");
    expect_pin(b, "EOC", LOGIC_0, "sending");
    run(b, AY_BIT * 11);
    expect_pin(b, "TBMT", LOGIC_1, "second byte taken after the first frame");
    run(b, AY_BIT * 11);
    expect_pin(b, "EOC", LOGIC_1, "both sent");

    /* Receive: DAV after the stop sample, RD the data; RDAV clears DAV. */
    ay_rx(b, 0x5A, 8, 0, 1);
    run(b, NS(4000));
    h_expect_bus(b, AY_STATUS, 5, "11000", "after one byte (TBMT DAV OR FE PE)");
    h_expect_bus(b, AY_RD, 8, "01011010", "RD after 0x5A");
    set(b, "RDAV", LOGIC_0);
    run(b, NS(600));
    expect_pin(b, "DAV", LOGIC_0, "RDAV low clears DAV");
    set(b, "RDAV", LOGIC_1);
    run(b, NS(600));
    /* Overrun: a second byte while DAV is 1. */
    ay_rx(b, 0x11, 8, 0, 1);
    run(b, NS(4000));
    ay_rx(b, 0x22, 8, 0, 1);
    run(b, NS(4000));
    h_expect_bus(b, AY_STATUS, 5, "11100", "OR on the second byte without RDAV");
    h_expect_bus(b, AY_RD, 8, "00100010", "the newer byte replaces the older");
    set(b, "RDAV", LOGIC_0);
    run(b, NS(600));
    set(b, "RDAV", LOGIC_1);
    /* Framing error: stop bit 0. */
    ay_rx(b, 0x33, 8, 0, 0);
    run(b, NS(4000));
    h_expect_bus(b, AY_STATUS, 5, "11010", "FE (stop bit 0); OR cleared");
    set(b, "RDAV", LOGIC_0);
    run(b, NS(600));
    set(b, "RDAV", LOGIC_1);
    /* Parity error: 7E with odd parity on the line. */
    ay_control(b, 7, 0, 1, 0);
    ay_rx(b, 0x41, 7, 'o', 1);
    run(b, NS(4000));
    h_expect_bus(b, AY_STATUS, 5, "11001", "PE on a wrong parity bit");
    h_expect_bus(b, AY_RD, 8, "01000001", "7-bit data, RD8 = 0");
    set(b, "RDAV", LOGIC_0);
    run(b, NS(600));
    set(b, "RDAV", LOGIC_1);
    ay_control(b, 8, 1, 0, 0);

    /* RDE / SWE 3-state; EOC and SO stay driven. */
    set(b, "RDE", LOGIC_1);
    set(b, "SWE", LOGIC_1);
    run(b, NS(1000));
    h_expect_bus(b, AY_RD, 8, "ZZZZZZZZ", "RDE high");
    h_expect_bus(b, AY_STATUS, 5, "ZZZZZ", "SWE high");
    expect_pin(b, "EOC", LOGIC_1, "EOC is not 3-state");
    set(b, "RDE", LOGIC_0);
    set(b, "SWE", LOGIC_0);
    run(b, NS(1000));

    /* XR: status reset, RHR kept (AY-5-1013A). */
    ay_rx(b, 0x66, 8, 0, 1);
    run(b, NS(4000));
    set(b, "XR", LOGIC_1);
    run(b, NS(1000));
    set(b, "XR", LOGIC_0);
    run(b, NS(1000));
    h_expect_bus(b, AY_STATUS, 5, "10000", "status after XR");
    h_expect_bus(b, AY_RD, 8, "01100110", "XR keeps the RHR on the AY-5-1013A");

    /* Terminal: OK typed into SI arrives on RD; a transmitted A is logged. */
    set(b, "SI", LOGIC_Z);   /* the terminal drives SI */
    run(b, NS(100));
    check(h_send(b, 0, "OK") == 1, "terminal send accepted");
    h_wait(b, "DAV", LOGIC_1, 20 * AY_BIT);
    h_get_bus(b, AY_RD, 8, text);
    h_expect_text(text, "01001111", "terminal O on RD");
    set(b, "RDAV", LOGIC_0);
    run(b, NS(600));
    set(b, "RDAV", LOGIC_1);
    h_wait(b, "DAV", LOGIC_1, 20 * AY_BIT);
    h_get_bus(b, AY_RD, 8, text);
    h_expect_text(text, "01001011", "terminal K on RD");
    ay_strobe(b, 'A');
    run(b, 12 * AY_BIT);
    h_expect_sent(b, 0, 'A', "transmitted A in the sent log");
    hb_close(h);

    /* The AY-3-1015D variant: 1.5 stops with 5 bits; XR clears the RHR. */
    h = ay_open("SFX-AY-5-1013A-dip40n");
    b = &h->b;
    h_expect_bus(b, AY_RD, 8, "00000000", "AY-3-1015D: XR clears the RHR");
    ay_control(b, 5, 1, 0, 1);
    ay_tx_check(b, 0x15, 5, 0, 3, "AY-3-1015D 5N1.5");
    ay_control(b, 8, 1, 0, 0);
    ay_rx(b, 0x77, 8, 0, 1);
    run(b, NS(2000));
    h_expect_bus(b, AY_RD, 8, "01110111", "AY-3-1015D receives");
    set(b, "XR", LOGIC_1);
    run(b, NS(1000));
    set(b, "XR", LOGIC_0);
    run(b, NS(1000));
    h_expect_bus(b, AY_RD, 8, "00000000", "AY-3-1015D: XR clears the RHR");
    case_end("H1 AY-5-1013A: control latch, every TX format, DAV/RDAV, OR/FE/PE, RDE/SWE, reset, terminal");
    hb_close(h);
}

/* The other pin-parallel devices on the same machine (COM2017, HD-6402):
   their pin names, 5-bit 1.5 stops, receive, the reset rule, terminal. */
typedef struct {
    const char *part;
    const char *td[8], *rd[8];
    const char *ds, *rdav, *so, *si, *xr, *strobe, *np, *tsb, *nb1, *nb2, *eps, *rde, *swe, *dav, *eoc;
    const char *tcp, *rcp;
    int clears;
} AyPins;

static void case_h1_pin_parallel(void) {
    static const AyPins DEV[2] = {
        {"SFX-COM2017-dip40",
         {"TD1", "TD2", "TD3", "TD4", "TD5", "TD6", "TD7", "TD8"},
         {"RD1", "RD2", "RD3", "RD4", "RD5", "RD6", "RD7", "RD8"},
         "TDS", "RDAR", "TSO", "RSI", "MR", "CS", "NPB", "NSB", "NDB1", "NDB2", "POE", "RDE", "SWE",
         "RDA", "TEOC", "TCP", "RCP", 0},
        {"SFX-HD-6402-dip40",
         {"TBR1", "TBR2", "TBR3", "TBR4", "TBR5", "TBR6", "TBR7", "TBR8"},
         {"RBR1", "RBR2", "RBR3", "RBR4", "RBR5", "RBR6", "RBR7", "RBR8"},
         "TBRL", "DRR", "TRO", "RRI", "MR", "CRL", "PI", "SBS", "CLS1", "CLS2", "EPE", "RRD", "SFD",
         "DR", "TRE", "TRC", "RRC", 0},
    };
    unsigned d;

    case_begin();
    for (d = 0; d < 2; d++) {
        const AyPins *p = &DEV[d];
        HClockPin clocks[2] = {{p->tcp, 0}, {p->rcp, 0}};
        SimTime half = AY_HALF;
        HBench *h = hb_open(p->part, clocks, 2, &half, 1);
        Bench *b = &h->b;
        char got[24];
        char want[24];
        char text[16];
        char what[96];
        SimTime t0;
        unsigned halves;

        set(b, p->xr, LOGIC_1);
        set(b, p->rde, LOGIC_0);
        set(b, p->swe, LOGIC_0);
        set(b, p->rdav, LOGIC_1);
        set(b, p->ds, LOGIC_1);
        set(b, p->si, LOGIC_1);
        set(b, p->np, LOGIC_1);
        set(b, p->tsb, LOGIC_1);
        set(b, p->nb1, LOGIC_0);
        set(b, p->nb2, LOGIC_0);
        set(b, p->eps, LOGIC_0);
        set(b, p->strobe, LOGIC_1);
        h_set_bus(b, p->td, 8, 1, 0x15);
        run(b, NS(2000));
        set(b, p->xr, LOGIC_0);
        set(b, p->strobe, LOGIC_0);   /* latch 5N, 2 stops selected */
        run(b, NS(2000));
        h_get_bus(b, p->rd, 8, text);
        snprintf(what, sizeof what, "%s: RD after the reset (not cleared: X)", p->part);
        h_expect_text(text, "XXXXXXXX", what);
        /* 5 bits with the stop select: 1.5 stops. */
        set(b, p->ds, LOGIC_0);
        run(b, NS(300));
        set(b, p->ds, LOGIC_1);
        check(h_catch(b, p->so, AY_BIT, 5, 4 * AY_BIT, got), p->part);
        h_frame_bits(0x15, 5, 0, 0, want);
        snprintf(what, sizeof what, "%s: 5N frame", p->part);
        h_expect_text(got, want, what);
        t0 = sim_now(b->sim) - 5 * AY_BIT - AY_BIT / 2;
        h_wait(b, p->eoc, LOGIC_1, 4 * AY_BIT);
        halves = (unsigned)((sim_now(b->sim) - t0 + AY_BIT / 4) / (AY_BIT / 2));
        snprintf(what, sizeof what, "%s: 5-bit stop select gives 1.5 stops (%u half bits)", p->part,
                 halves);
        check(halves == 15, what);
        /* 8N1, receive, terminal. */
        set(b, p->nb1, LOGIC_1);
        set(b, p->nb2, LOGIC_1);
        set(b, p->tsb, LOGIC_0);
        set(b, p->strobe, LOGIC_1);
        run(b, NS(300));
        set(b, p->strobe, LOGIC_0);
        h_frame_bits(0xA6, 8, 0, 1, want);
        h_drive_bits(b, p->si, want, AY_BIT);
        run(b, AY_BIT);
        expect_pin(b, p->dav, LOGIC_1, p->part);
        h_get_bus(b, p->rd, 8, text);
        snprintf(what, sizeof what, "%s: received 0xA6", p->part);
        h_expect_text(text, "10100110", what);
        set(b, p->rdav, LOGIC_0);
        run(b, NS(600));
        set(b, p->rdav, LOGIC_1);
        set(b, p->si, LOGIC_Z);
        run(b, NS(100));
        check(h_send(b, 0, "OK") == 1, p->part);
        h_wait(b, p->dav, LOGIC_1, 12 * AY_BIT);
        h_get_bus(b, p->rd, 8, text);
        snprintf(what, sizeof what, "%s: terminal O", p->part);
        h_expect_text(text, "01001111", what);
        h_set_bus(b, p->td, 8, 1, 'A');
        set(b, p->ds, LOGIC_0);
        run(b, NS(300));
        set(b, p->ds, LOGIC_1);
        run(b, 12 * AY_BIT);
        h_expect_sent(b, 0, 'A', p->part);
        hb_close(h);
    }
    case_end("H1 (COM2017, HD-6402): pin names, 5-bit 1.5 stops, receive, reset keeps RD, terminal");
}

/* ---- H2: COM1863 / COM8018 (ay1013 with HIACC) -------------------------------------- */

static const char *const COM_TD[8] = {"TD1", "TD2", "TD3", "TD4", "TD5", "TD6", "TD7", "TD8"};
static const char *const COM_RD[8] = {"RD1", "RD2", "RD3", "RD4", "RD5", "RD6", "RD7", "RD8"};

static void com_control(Bench *b, unsigned nb, int npb, int poe, int nsb) {
    set(b, "NDB1", (nb - 5) & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "NDB2", (nb - 5) & 2u ? LOGIC_1 : LOGIC_0);
    set(b, "NPB", npb ? LOGIC_1 : LOGIC_0);
    set(b, "POE", poe ? LOGIC_1 : LOGIC_0);
    set(b, "NSB", nsb ? LOGIC_1 : LOGIC_0);
    run(b, NS(100));
    set(b, "CS", LOGIC_1);
    run(b, NS(300));
    set(b, "CS", LOGIC_0);
    run(b, NS(100));
}

static void com_strobe(Bench *b, unsigned v) {
    h_set_bus(b, COM_TD, 8, 1, v);
    run(b, NS(100));
    set(b, "TDS", LOGIC_0);
    run(b, NS(300));
    set(b, "TDS", LOGIC_1);
    run(b, NS(100));
}

static void case_h2_com1863(void) {
    static const HClockPin CLOCKS[2] = {{"TCP", 0}, {"RCP", 0}};
    SimTime half = AY_HALF;
    HBench *h = hb_open("SFX-COM1863-dip40", CLOCKS, 2, &half, 1);
    Bench *b = &h->b;
    const SimTime bit32 = 2 * AY_BIT;
    char got[24];
    char want[24];
    char text[16];
    SimTime t0;

    case_begin();
    set(b, "MR", LOGIC_1);
    set(b, "RDE", LOGIC_0);
    set(b, "SWE", LOGIC_0);
    set(b, "RDAR", LOGIC_1);
    set(b, "TDS", LOGIC_1);
    set(b, "RSI", LOGIC_1);
    set(b, "CS", LOGIC_0);
    set(b, "HIACC", LOGIC_0);
    h_set_bus(b, COM_TD, 8, 1, 0);
    run(b, NS(2000));
    set(b, "MR", LOGIC_0);
    run(b, NS(2000));
    h_expect_bus(b, COM_RD, 8, "00000000", "MR clears RD1-RD8 (COM1863)");

    /* 16x with HIACC low. */
    com_control(b, 8, 1, 0, 0);
    com_strobe(b, 0x96);
    check(h_catch(b, "TSO", AY_BIT, 9, 4 * AY_BIT, got), "16x frame starts");
    h_frame_bits(0x96, 8, 0, 1, want);
    h_expect_text(got, want, "HIACC low: 16 clocks a bit");
    run(b, 2 * AY_BIT);

    /* HIACC high: 32x clocks. */
    set(b, "HIACC", LOGIC_1);
    run(b, NS(200));
    com_strobe(b, 0x96);
    check(h_catch(b, "TSO", bit32, 9, 4 * bit32, got), "32x frame starts");
    h_expect_text(got, want, "HIACC high: 32 clocks a bit");
    run(b, 2 * bit32);

    /* 5 data bits with NSB = 1: 1.5 stop bits (planner decision). */
    com_control(b, 5, 1, 0, 1);
    com_strobe(b, 0x15);
    check(h_catch(b, "TSO", bit32, 5, 4 * bit32, got), "5-bit frame starts");
    t0 = sim_now(b->sim) - 5 * bit32 - bit32 / 2;
    h_wait(b, "TEOC", LOGIC_1, 4 * bit32);
    {
        unsigned halves = (unsigned)((sim_now(b->sim) - t0 + bit32 / 4) / (bit32 / 2));
        char message[96];

        snprintf(message, sizeof message, "5-bit NSB=1: TEOC after %u half bits, want 15", halves);
        check(halves == 15, message);
    }
    com_control(b, 8, 1, 0, 0);

    /* 32x receive; RDAR held low over the transfer: ROR set, RDA waits for
       RDAR high (p7). */
    h_frame_bits(0x3C, 8, 0, 1, want);
    set(b, "RDAR", LOGIC_0);
    h_drive_bits(b, "RSI", want, bit32);
    run(b, bit32);
    expect_pin(b, "RDA", LOGIC_0, "RDA held low while RDAR is low");
    h_expect_bus(b, COM_RD, 8, "00111100", "32x data");
    expect_pin(b, "ROR", LOGIC_1, "ROR: RDAR was low at the transfer");
    set(b, "RDAR", LOGIC_1);
    run(b, NS(500));
    expect_pin(b, "RDA", LOGIC_1, "RDA rises once RDAR is high");
    set(b, "RDAR", LOGIC_0);
    run(b, NS(500));
    set(b, "RDAR", LOGIC_1);
    run(b, NS(500));
    h_drive_bits(b, "RSI", want, bit32);
    run(b, bit32);
    expect_pin(b, "ROR", LOGIC_0, "ROR reset: RDA low and RDAR high at the transfer");

    /* Terminal at 32x. */
    set(b, "RDAR", LOGIC_0);
    run(b, NS(500));
    set(b, "RDAR", LOGIC_1);
    set(b, "RSI", LOGIC_Z);
    run(b, NS(500));
    check(h_send(b, 0, "OK") == 1, "terminal send accepted");
    h_wait(b, "RDA", LOGIC_1, 12 * bit32);
    h_get_bus(b, COM_RD, 8, text);
    h_expect_text(text, "01001111", "terminal O");
    set(b, "RDAR", LOGIC_0);
    run(b, NS(500));
    set(b, "RDAR", LOGIC_1);
    h_wait(b, "RDA", LOGIC_1, 12 * bit32);
    h_get_bus(b, COM_RD, 8, text);
    h_expect_text(text, "01001011", "terminal K");
    com_strobe(b, 'A');
    run(b, 12 * bit32);
    h_expect_sent(b, 0, 'A', "transmitted A logged");
    case_end("H2 COM1863/COM8018: HIACC 32x clocks, 5-bit 1.5 stops, RDA/ROR rule, terminal");
    hb_close(h);
}

/* ---- H3: CDP1854A (ay1013 with cpuMode) ---------------------------------------------- */

static const char *const CDP_TBUS[8] = {"TBUS0", "TBUS1", "TBUS2", "TBUS3", "TBUS4", "TBUS5", "TBUS6", "TBUS7"};
static const char *const CDP_RBUS[8] = {"RBUS0", "RBUS1", "RBUS2", "RBUS3", "RBUS4", "RBUS5", "RBUS6", "RBUS7"};

/* Mode 1: one CPU cycle; rsel, write (1) or read (0); returns the R BUS
   during the read as text. */
static void cdp_cycle(Bench *b, int rsel, int write, unsigned v, char *text) {
    set(b, "SFD/RSEL", rsel ? LOGIC_1 : LOGIC_0);
    set(b, "CRL/RD_WR", write ? LOGIC_0 : LOGIC_1);
    if (write) {
        h_set_bus(b, CDP_TBUS, 8, 1, v);
    }
    set(b, "THRL/CS1", LOGIC_1);
    run(b, NS(300));
    if (!write && text != NULL) {
        h_get_bus(b, CDP_RBUS, 8, text);
    }
    set(b, "DAR/TPB", LOGIC_1);
    run(b, NS(200));
    set(b, "DAR/TPB", LOGIC_0);
    run(b, NS(200));
    set(b, "THRL/CS1", LOGIC_0);
    set(b, "CRL/RD_WR", LOGIC_1);
    run(b, NS(300));
}

static void case_h3_cdp1854a(void) {
    static const HClockPin CLOCKS[2] = {{"TCLOCK", 0}, {"RCLOCK", 0}};
    SimTime half = AY_HALF;
    HBench *h;
    Bench *b;
    char got[24];
    char want[24];
    char text[16];

    case_begin();
    /* ---- Mode 0: the AY-5-1013 machine. ---- */
    h = hb_open("SFX-CDP1854A-dip40", CLOCKS, 2, &half, 1);
    b = &h->b;
    set(b, "MODE", LOGIC_0);
    set(b, "MR/CLEAR", LOGIC_1);
    set(b, "RRD/CS2", LOGIC_0);
    set(b, "SFD/RSEL", LOGIC_0);
    set(b, "DAR/TPB", LOGIC_1);
    set(b, "THRL/CS1", LOGIC_1);
    set(b, "SDI", LOGIC_1);
    set(b, "CRL/RD_WR", LOGIC_0);
    set(b, "PI/CS3", LOGIC_1);
    set(b, "SBS", LOGIC_0);
    set(b, "WLS1/ES", LOGIC_1);
    set(b, "WLS2/PSI", LOGIC_1);
    set(b, "EPE/CTS", LOGIC_0);
    h_set_bus(b, CDP_TBUS, 8, 1, 0);
    run(b, NS(2000));
    set(b, "MR/CLEAR", LOGIC_0);
    run(b, NS(2000));
    h_expect_bus(b, CDP_RBUS, 8, "00000000", "mode 0: MR clears the RHR");
    expect_pin(b, "THRE", LOGIC_1, "mode 0: THRE after MR");
    expect_pin(b, "TSRE/RTS", LOGIC_1, "mode 0: TSRE after MR");
    /* MR cleared the control register: 5 bits, odd parity, 1 stop. */
    h_set_bus(b, CDP_TBUS, 8, 1, 0x1B);
    set(b, "THRL/CS1", LOGIC_0);
    run(b, NS(300));
    set(b, "THRL/CS1", LOGIC_1);
    check(h_catch(b, "SDO", AY_BIT, 7, 4 * AY_BIT, got), "mode 0 frame");
    h_frame_bits(0x1B, 5, 'o', 1, want);
    h_expect_text(got, want, "mode 0 after MR: 5O1 (control register cleared)");
    run(b, 2 * AY_BIT);
    /* CRL pulse: 8 bits, no parity (PI high), SBS 1 -> 2 stops. */
    set(b, "SBS", LOGIC_1);
    set(b, "CRL/RD_WR", LOGIC_1);
    run(b, NS(300));
    set(b, "CRL/RD_WR", LOGIC_0);
    h_set_bus(b, CDP_TBUS, 8, 1, 0xC3);
    set(b, "THRL/CS1", LOGIC_0);
    run(b, NS(300));
    set(b, "THRL/CS1", LOGIC_1);
    check(h_catch(b, "SDO", AY_BIT, 10, 4 * AY_BIT, got), "mode 0 8N2 frame");
    h_frame_bits(0xC3, 8, 0, 2, want);
    h_expect_text(got, want, "mode 0: 8N2 from the latched control word");
    run(b, 2 * AY_BIT);
    /* Receive: DA high, RBUS data; DAR low resets DA. */
    h_frame_bits(0x5A, 8, 0, 1, want);
    h_drive_bits(b, "SDI", want, AY_BIT);
    run(b, AY_BIT);
    expect_pin(b, "DA", LOGIC_1, "mode 0: DA");
    h_expect_bus(b, CDP_RBUS, 8, "01011010", "mode 0: R BUS");
    set(b, "DAR/TPB", LOGIC_0);
    run(b, NS(400));
    set(b, "DAR/TPB", LOGIC_1);
    run(b, NS(400));
    expect_pin(b, "DA", LOGIC_0, "mode 0: DAR resets DA");
    set(b, "SFD/RSEL", LOGIC_1);
    set(b, "RRD/CS2", LOGIC_1);
    run(b, NS(500));
    expect_pin(b, "DA", LOGIC_Z, "mode 0: SFD high floats the status");
    expect_pin(b, "RBUS0", LOGIC_Z, "mode 0: RRD high floats the R BUS");
    expect_pin(b, "TSRE/RTS", LOGIC_1, "mode 0: TSRE is not 3-state");
    hb_close(h);

    /* ---- Mode 1: the CDP1800 bus. ---- */
    h = hb_open("SFX-CDP1854A-dip40", CLOCKS, 2, &half, 1);
    b = &h->b;
    set(b, "MODE", LOGIC_1);
    set(b, "MR/CLEAR", LOGIC_0);
    set(b, "THRL/CS1", LOGIC_0);
    set(b, "RRD/CS2", LOGIC_0);
    set(b, "PI/CS3", LOGIC_1);
    set(b, "DAR/TPB", LOGIC_0);
    set(b, "CRL/RD_WR", LOGIC_1);
    set(b, "SFD/RSEL", LOGIC_0);
    set(b, "SDI", LOGIC_1);
    set(b, "EPE/CTS", LOGIC_0);
    set(b, "WLS2/PSI", LOGIC_1);
    set(b, "WLS1/ES", LOGIC_1);
    set(b, "SBS", LOGIC_0);
    h_set_bus(b, CDP_TBUS, 8, 1, 0);
    run(b, NS(2000));
    set(b, "MR/CLEAR", LOGIC_1);
    run(b, NS(2000));
    cdp_cycle(b, 1, 0, 0, text);
    h_expect_text(text, "11000000", "mode 1: status after CLEAR (THRE TSRE)");
    expect_pin(b, "PE/INT", LOGIC_1, "mode 1: INT inactive (IE 0)");
    expect_pin(b, "TSRE/RTS", LOGIC_1, "mode 1: RTS high (TR 0, empty)");
    /* Format: IE, 8 bits, 1 stop, PI. Then TR: RTS low, the initial THRE interrupt. */
    cdp_cycle(b, 1, 1, 0x39, NULL);
    expect_pin(b, "PE/INT", LOGIC_1, "mode 1: no interrupt before TR");
    cdp_cycle(b, 1, 1, 0x80, NULL);
    expect_pin(b, "TSRE/RTS", LOGIC_0, "mode 1: TR sets RTS low");
    expect_pin(b, "PE/INT", LOGIC_0, "mode 1: the initial THRE interrupt");
    cdp_cycle(b, 1, 0, 0, text);
    h_expect_text(text, "11000000", "mode 1: status");
    expect_pin(b, "PE/INT", LOGIC_1, "mode 1: a status read clears the THRE interrupt");
    /* A character: THRE low, then the shift register takes it; THRE pin
       low (active) and INT after one TCLOCK. */
    cdp_cycle(b, 0, 1, 'A', NULL);
    check(h_catch(b, "SDO", AY_BIT, 9, 4 * AY_BIT, got), "mode 1 frame");
    h_frame_bits('A', 8, 0, 1, want);
    h_expect_text(got, want, "mode 1: 8N1 A on SDO");
    expect_pin(b, "THRE", LOGIC_0, "mode 1: THRE pin active (low)");
    expect_pin(b, "PE/INT", LOGIC_0, "mode 1: THRE interrupt");
    run(b, 2 * AY_BIT);
    h_expect_sent(b, 0, 'A', "mode 1: A logged");
    cdp_cycle(b, 1, 0, 0, text);
    expect_pin(b, "PE/INT", LOGIC_1, "mode 1: status read clears THRE and THRE.TSRE");
    /* Receive: DA pin low, INT; an RHR read with TPB resets DA. */
    h_frame_bits(0xA5, 8, 0, 1, want);
    h_drive_bits(b, "SDI", want, AY_BIT);
    run(b, AY_BIT);
    expect_pin(b, "DA", LOGIC_0, "mode 1: DA pin active (low)");
    expect_pin(b, "PE/INT", LOGIC_0, "mode 1: DA interrupt");
    cdp_cycle(b, 0, 0, 0, text);
    h_expect_text(text, "10100101", "mode 1: RHR read");
    expect_pin(b, "DA", LOGIC_1, "mode 1: the read resets DA");
    expect_pin(b, "PE/INT", LOGIC_1, "mode 1: and its interrupt");
    expect_pin(b, "RBUS0", LOGIC_Z, "mode 1: R BUS released after the read");
    /* Overrun: two characters without a read; PE/OE pin high. */
    h_drive_bits(b, "SDI", want, AY_BIT);
    run(b, AY_BIT);
    h_drive_bits(b, "SDI", want, AY_BIT);
    run(b, AY_BIT);
    expect_pin(b, "OE/PE_OE", LOGIC_1, "mode 1: PE/OE pin on overrun");
    cdp_cycle(b, 1, 0, 0, text);
    h_expect_text(text, "11000011", "mode 1: status with OE and DA");
    cdp_cycle(b, 0, 0, 0, text);
    /* PSI falling: status bit 5 and INT; ES low: status bit 4. */
    set(b, "WLS2/PSI", LOGIC_0);
    set(b, "WLS1/ES", LOGIC_0);
    run(b, NS(500));
    expect_pin(b, "PE/INT", LOGIC_0, "mode 1: PSI interrupt");
    cdp_cycle(b, 1, 0, 0, text);
    h_expect_text(text, "11110010", "mode 1: status with PSI and ES (OE kept)");
    expect_pin(b, "PE/INT", LOGIC_1, "mode 1: the status read clears PSI");
    set(b, "WLS2/PSI", LOGIC_1);
    set(b, "WLS1/ES", LOGIC_1);
    /* BREAK holds SDO low. */
    cdp_cycle(b, 1, 1, 0x79, NULL);
    run(b, AY_BIT);
    expect_pin(b, "SDO", LOGIC_0, "mode 1: BREAK holds SDO low");
    cdp_cycle(b, 1, 1, 0x39, NULL);
    run(b, AY_BIT);
    expect_pin(b, "SDO", LOGIC_0, "mode 1: SDO stays low after BREAK is reset");
    cdp_cycle(b, 0, 1, 0x00, NULL);
    run(b, 12 * AY_BIT);
    expect_pin(b, "SDO", LOGIC_1, "mode 1: a transmitted word ends the break");
    /* CTS high stops the transmitter. */
    set(b, "EPE/CTS", LOGIC_1);
    run(b, NS(500));
    cdp_cycle(b, 0, 1, 0x55, NULL);
    run(b, 4 * AY_BIT);
    expect_pin(b, "SDO", LOGIC_1, "mode 1: CTS high inhibits the transfer");
    set(b, "EPE/CTS", LOGIC_0);
    check(h_catch(b, "SDO", AY_BIT, 9, 4 * AY_BIT, got), "mode 1: sent once CTS is low");
    run(b, 2 * AY_BIT);
    /* Terminal. */
    set(b, "SDI", LOGIC_Z);
    run(b, NS(100));
    check(h_send(b, 0, "OK") == 1, "mode 1: terminal send accepted");
    h_wait(b, "DA", LOGIC_0, 12 * AY_BIT);
    cdp_cycle(b, 0, 0, 0, text);
    h_expect_text(text, "01001111", "mode 1: terminal O");
    h_wait(b, "DA", LOGIC_0, 12 * AY_BIT);
    cdp_cycle(b, 0, 0, 0, text);
    h_expect_text(text, "01001011", "mode 1: terminal K");
    case_end("H3 CDP1854A: mode 0 as H1; mode 1 registers, TR/RTS, INT sources, break, CTS, terminal");
    hb_close(h);
}


int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_h1_ay1013();
    case_h1_pin_parallel();
    case_h2_com1863();
    case_h3_cdp1854a();
    return serial_bench_finish();
}
