/*
 * serial_ay1013_cov_test: plan-68-P cases for the ay1013 model (AY-5-1013A,
 * CDP1854A) that serial_ay1013_test does not reach: loader rejections, the
 * reset pin at X, DS / RDAV / the control strobe at X, RDAV held low or X
 * through a reception, a reset before DAV is due, and the CDP1854A mode-1
 * bus with X selects and an X CTS. Facts: the header of
 * src/models/serial_ay1013.c (plan-12-data/specs/classic-a.md).
 */

#include "serial_bench.h"
#include "serial_bench_classic.h"
#include "serial_cov_classic.h"

static const char *const AY_DB[8] = {"DB1", "DB2", "DB3", "DB4", "DB5", "DB6", "DB7", "DB8"};
static const char *const AY_RD[8] = {"RD1", "RD2", "RD3", "RD4", "RD5", "RD6", "RD7", "RD8"};
static const char *const AY_STATUS[5] = {"PE", "FE", "OR", "DAV", "TBMT"};

/* 1 MHz on TCP and RCP: a 16 us bit. */
#define AY_HALF NS(500)
#define AY_BIT NS(16000)

/* The AY-5-1013A, reset, control word 8N1 latched, RDE and SWE low. */
static HBench *ay_open(void) {
    static const HClockPin CLOCKS[2] = {{"TCP", 0}, {"RCP", 0}};
    SimTime half = AY_HALF;
    HBench *h = hb_open("SFX-AY-5-1013A-dip40", CLOCKS, 2, &half, 1);
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
    run(b, NS(100));
    set(b, "CS", LOGIC_1);
    run(b, NS(400));
    set(b, "CS", LOGIC_0);
    run(b, NS(2000));
    return h;
}

/* Drives an 8N1 frame of v onto SI and waits two bit times. */
static void ay_rx(Bench *b, unsigned v) {
    char bits[24];

    h_frame_bits(v, 8, 0, 1, bits);
    h_drive_bits(b, "SI", bits, AY_BIT);
    run(b, 2 * AY_BIT);
}

/* ---- Loader ----------------------------------------------------------------------------- */

static void case_ay_loader(void) {
    const char *ay = "SFX-AY-5-1013A";
    const char *cdp = "SFX-CDP1854A";

    case_begin();
    scv_reject(ay, "{\"fiveBitStops\":\"two\"}", "\"fiveBitStops\" must be a number");
    scv_reject(ay, "{\"fiveBitStops\":3}", "\"fiveBitStops\" must be 1.5 or 2");
    scv_reject(ay, "{\"control\":\"-\"}", "missing \"control\"");
    scv_reject(ay, "{\"control.latch\":\"CS\"}", "unknown key \"latch\"");
    scv_reject(ay, "{\"clocks.transmit\":\"-\"}", "missing \"transmit\"");
    scv_reject(ay, "{\"transmitData.7\":\"-\"}", "\"transmitData\" needs exactly 8 pin names");
    scv_reject(ay, "{\"resetClearsControl\":\"no\"}", "\"resetClearsControl\" must be true or false");
    scv_reject(cdp, "{\"cpuMode.tpb\":\"-\"}", "missing \"tpb\"");
    scv_reject(ay, "{\"serialOut\":\"SOX\"}", "no pin named \"SOX\"");
    scv_reject(cdp, "{\"cpuMode.tpb\":\"TPBX\"}", "no pin named \"TPBX\"");
    scv_reject(ay, "{\"parts.0.fiveBitStops\":3}", "\"fiveBitStops\" must be 1.5 or 2");
    scv_reject(ay, "{\"parts.0.resetClearsData\":\"no\"}", "\"resetClearsData\" must be true or false");
    case_end("Y1 ay1013 loader: fiveBitStops, control, clocks, data, resetClearsControl, cpuMode, pins, part keys");
}

/* ---- The reset pin at X ------------------------------------------------------------------ */

static void case_ay_reset_x(void) {
    HBench *h = ay_open();
    Bench *b = &h->b;

    case_begin();
    h_expect_bus(b, AY_STATUS, 5, "10000", "status after XR");
    set(b, "XR", LOGIC_X);
    run(b, NS(2000));
    h_expect_bus(b, AY_STATUS, 5, "XXXXX", "XR X: the status pins X");
    h_expect_bus(b, AY_RD, 8, "XXXXXXXX", "XR X: RD X");
    expect_pin(b, "SO", LOGIC_X, "XR X: SO X");
    /* Out of an X reset: everything the reset sets is unknown. */
    set(b, "XR", LOGIC_0);
    run(b, NS(2000));
    /* The holding register may have been full: it goes to the shift register
       at the next TCP edge (TBMT high again), unknown on SO. */
    h_expect_bus(b, AY_STATUS, 5, "1XXXX", "after XR X: DAV OR FE PE unknown, TBMT after the transfer");
    expect_pin(b, "SO", LOGIC_X, "after XR X: SO unknown");
    set(b, "XR", LOGIC_1);
    run(b, NS(2000));
    set(b, "XR", LOGIC_0);
    run(b, NS(2000));
    h_expect_bus(b, AY_STATUS, 5, "10000", "a real XR: the reset state again");
    case_end("Y2 AY-5-1013A XR X: every output X, the reset flags unknown after it");
    hb_close(h);
}

/* ---- DS and the control strobe at X ------------------------------------------------------- */

static void case_ay_strobes_x(void) {
    HBench *h = ay_open();
    Bench *b = &h->b;
    char got[24];

    case_begin();
    /* DS 1 -> X: the load may have happened: TBMT X. */
    h_set_bus(b, AY_DB, 8, 1, 0x55);
    set(b, "DS", LOGIC_X);
    run(b, NS(50));
    expect_pin(b, "TBMT", LOGIC_X, "DS X: TBMT X (before the next TCP edge)");
    run(b, NS(950));
    /* X -> 1 ends an X cycle: the character is X. */
    set(b, "DS", LOGIC_1);
    run(b, NS(1000));
    check(h_catch(b, "SO", AY_BIT, 9, 4 * AY_BIT, got), "the X character starts");
    check(strncmp(got, "0XXXXXXXX1", 10) == 0, "X DS cycle: the data bits go out X");
    run(b, 2 * AY_BIT);
    /* X -> 0: still X; the byte is not taken until DS rises. */
    set(b, "DS", LOGIC_X);
    run(b, NS(500));
    set(b, "DS", LOGIC_0);
    run(b, NS(500));
    expect_pin(b, "TBMT", LOGIC_0, "DS low after X: TBMT low (DS held)");
    set(b, "DS", LOGIC_1);
    run(b, 14 * AY_BIT);
    expect_pin(b, "TBMT", LOGIC_1, "the character went: TBMT high");
    /* The control strobe at X with NP changing: the parity setting is X; a
       received character's data is unknown. */
    set(b, "NP", LOGIC_0);
    set(b, "CS", LOGIC_X);
    run(b, NS(500));
    set(b, "CS", LOGIC_0);
    set(b, "NP", LOGIC_1);
    run(b, NS(500));
    ay_rx(b, 0x3C);
    h_expect_bus(b, AY_RD, 8, "XXXXXXXX", "control word X: the received data is X");
    case_end("Y3 AY-5-1013A: DS X makes TBMT and the character X; CS X makes the format X");
    hb_close(h);
}

/* ---- RDAV: X, held low, held X ------------------------------------------------------------- */

static void case_ay_rdav(void) {
    HBench *h = ay_open();
    Bench *b = &h->b;

    case_begin();
    ay_rx(b, 0x41);
    expect_pin(b, "DAV", LOGIC_1, "DAV after a character");
    set(b, "RDAV", LOGIC_X);
    run(b, NS(1000));
    expect_pin(b, "DAV", LOGIC_X, "RDAV X while DAV is set: DAV X");
    set(b, "RDAV", LOGIC_0);
    run(b, NS(1000));
    expect_pin(b, "DAV", LOGIC_0, "RDAV low resets DAV");
    /* RDAV held low through a reception: the DAV flip-flop stays reset. */
    ay_rx(b, 0x42);
    expect_pin(b, "DAV", LOGIC_0, "RDAV held low: DAV stays low");
    h_expect_bus(b, AY_RD, 8, "01000010", "the character is still transferred");
    /* RDAV held X through a reception: DAV X. */
    set(b, "RDAV", LOGIC_X);
    run(b, NS(1000));
    ay_rx(b, 0x43);
    expect_pin(b, "DAV", LOGIC_X, "RDAV held X: DAV X when it is due");
    set(b, "RDAV", LOGIC_1);
    run(b, NS(1000));
    case_end("Y4 AY-5-1013A RDAV: X clears to X, held low keeps DAV reset, held X makes it X");
    hb_close(h);
}

/* ---- A reset before DAV is due --------------------------------------------------------------- */

static void case_ay_reset_before_dav(void) {
    HBench *h = ay_open();
    Bench *b = &h->b;
    char bits[24];
    SimTime spent = 0;

    case_begin();
    ay_rx(b, 0x11);
    expect_pin(b, "DAV", LOGIC_1, "the first character: DAV");
    /* The second one overruns: OR rises when it is transferred, DAV is due
       tdav (3 us) later. XR in between. */
    h_frame_bits(0x22, 8, 0, 0, bits);
    h_drive_bits(b, "SI", bits, AY_BIT);
    while (get(b, "OR") != LOGIC_1 && spent < AY_BIT) {
        run(b, NS(100));
        spent += NS(100);
    }
    check(get(b, "OR") == LOGIC_1, "the overrun shows on OR");
    set(b, "XR", LOGIC_1);
    run(b, NS(500));
    set(b, "XR", LOGIC_0);
    run(b, NS(5000));
    h_expect_bus(b, AY_STATUS, 5, "10000", "XR before DAV was due: DAV stays low");
    case_end("Y5 AY-5-1013A: a reset inside tdav cancels the DAV that was due");
    hb_close(h);
}

/* ---- CDP1854A mode 1: X selects, X CTS ------------------------------------------------------ */

static const char *const CDP_TBUS[8] = {"TBUS0", "TBUS1", "TBUS2", "TBUS3", "TBUS4", "TBUS5", "TBUS6", "TBUS7"};
static const char *const CDP_RBUS[8] = {"RBUS0", "RBUS1", "RBUS2", "RBUS3", "RBUS4", "RBUS5", "RBUS6", "RBUS7"};

static void cdp_cycle(Bench *b, int rsel, int write, unsigned v) {
    set(b, "SFD/RSEL", rsel ? LOGIC_1 : LOGIC_0);
    set(b, "CRL/RD_WR", write ? LOGIC_0 : LOGIC_1);
    if (write) {
        h_set_bus(b, CDP_TBUS, 8, 1, v);
    }
    set(b, "THRL/CS1", LOGIC_1);
    run(b, NS(300));
    set(b, "DAR/TPB", LOGIC_1);
    run(b, NS(200));
    set(b, "DAR/TPB", LOGIC_0);
    run(b, NS(200));
    set(b, "THRL/CS1", LOGIC_0);
    set(b, "CRL/RD_WR", LOGIC_1);
    run(b, NS(300));
}

static void case_cdp_mode1_x(void) {
    static const HClockPin CLOCKS[2] = {{"TCLOCK", 0}, {"RCLOCK", 0}};
    SimTime half = AY_HALF;
    HBench *h = hb_open("SFX-CDP1854A-dip40", CLOCKS, 2, &half, 1);
    Bench *b = &h->b;
    char got[24];

    case_begin();
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
    cdp_cycle(b, 1, 1, 0x19);   /* 8 bits, 1 stop, PI */
    /* A selected read with RSEL X: which register is unknown, R BUS X. */
    set(b, "SFD/RSEL", LOGIC_X);
    set(b, "THRL/CS1", LOGIC_1);
    run(b, NS(400));
    h_expect_bus(b, CDP_RBUS, 8, "XXXXXXXX", "read with RSEL X: R BUS X");
    set(b, "THRL/CS1", LOGIC_0);
    set(b, "SFD/RSEL", LOGIC_0);
    run(b, NS(400));
    expect_pin(b, "RBUS0", LOGIC_Z, "released after the read");
    /* CS1 X during a read: maybe selected, R BUS X. */
    set(b, "THRL/CS1", LOGIC_X);
    run(b, NS(400));
    h_expect_bus(b, CDP_RBUS, 8, "XXXXXXXX", "read with CS1 X: R BUS X");
    set(b, "THRL/CS1", LOGIC_0);
    run(b, NS(400));
    /* CTS X: the transfer to the shift register waits (CTS must be low). */
    set(b, "EPE/CTS", LOGIC_X);
    run(b, NS(500));
    cdp_cycle(b, 0, 1, 0x55);
    check(!h_wait(b, "SDO", LOGIC_0, 4 * AY_BIT), "CTS X: no transfer");
    check(sent(b, NULL, 0) == 0, "CTS X: nothing sent");
    set(b, "EPE/CTS", LOGIC_0);
    check(h_catch(b, "SDO", AY_BIT, 9, 4 * AY_BIT, got), "CTS low: the character goes");
    check(strcmp(got, "0101010101") == 0, "CTS low: 0x55 8N1");
    case_end("Y6 CDP1854A mode 1: RSEL or CS1 X reads X; CTS X holds the transfer");
    hb_close(h);
}

int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_ay_loader();
    case_ay_reset_x();
    case_ay_strobes_x();
    case_ay_rdav();
    case_ay_reset_before_dav();
    case_cdp_mode1_x();
    return serial_bench_finish();
}
