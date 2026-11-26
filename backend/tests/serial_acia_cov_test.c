/*
 * serial_acia_cov_test: plan-68-P cases for the acia6551 model (CDP65C51,
 * CDP65C51A, W65C51N) that serial_acia_test does not reach: loader
 * rejections, 2-stop / 1.5-stop formats, the CDP65C51A CTS hold, CTS and
 * RES at X, echo with an overrun, break with a character waiting, and a
 * W65C51N TDR write in the stop bit. Facts: the header of
 * src/models/serial_acia6551.c (cdp65c51.pdf, w65c51n.pdf).
 */

#include "serial_bench.h"
#include "serial_bench_classic.h"
#include "serial_cov_classic.h"

static const char *const AC_D[8] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"};
static const HClockPin AC_CLOCKS[1] = {{"XTLI", 0}};

#define AC_BT (96 * 2 * AC_XHALF)   /* 19200 at 1.8432 MHz */

/* One PHI2 cycle on register rs: write v, or read (the bus as text). The
   names differ between the CDP65C51 (CS1, RW) and the W65C51N (CS1B, RWB). */
static const char *ac_cs1 = "CS1";
static const char *ac_rw = "RW";

static void ac_cycle(Bench *b, unsigned rs, int write, unsigned v, char *text) {
    set(b, "RS0", rs & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "RS1", rs & 2u ? LOGIC_1 : LOGIC_0);
    set(b, ac_rw, write ? LOGIC_0 : LOGIC_1);
    set(b, "CS0", LOGIC_1);
    set(b, ac_cs1, LOGIC_0);
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
    set(b, ac_rw, LOGIC_1);
    h_set_bus(b, AC_D, 8, 0, 0);
    run(b, NS(400));
}

static void ac_expect(Bench *b, unsigned rs, const char *want, const char *what) {
    char text[16];

    ac_cycle(b, rs, 0, 0, text);
    h_expect_text(text, want, what);
}

/* A CDP65C51-shaped part (the fixture or a variant), reset, CTS/DCD/DSR low. */
static HBench *ac_open(const char *part) {
    SimTime half = AC_XHALF;
    HBench *h = hb_open(part, AC_CLOCKS, 1, &half, 1);
    Bench *b = &h->b;

    ac_cs1 = "CS1";
    ac_rw = "RW";
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
    return h;
}

/* The time from now to the next falling edge of pin (0 = none in limit). */
static SimTime ac_next_fall(Bench *b, const char *pin, SimTime limit) {
    SimTime t0 = sim_now(b->sim);

    if (!h_wait(b, pin, LOGIC_1, limit) || !h_wait(b, pin, LOGIC_0, limit)) {
        return 0;
    }
    return sim_now(b->sim) - t0;
}

static int ac_near(SimTime got, SimTime want, SimTime tol) {
    return got + tol >= want && got <= want + tol;
}

/* ---- Loader ---------------------------------------------------------------------------- */

static void case_ac_loader(void) {
    const char *dev = "SFX-CDP65C51";

    case_begin();
    scv_reject(dev, "{\"bus\":\"-\"}", "missing \"bus\"");
    scv_reject(dev, "{\"bus.strobe\":\"PHI2\"}", "unknown key \"strobe\"");
    scv_reject(dev, "{\"bus.data.7\":\"-\"}", "\"data\" needs exactly 8 pin names");
    scv_reject(dev, "{\"bus.registerSelect\":\"-\"}", "missing \"registerSelect\"");
    scv_reject(dev, "{\"bus.phi2\":5}", "\"phi2\" must be a string");
    scv_reject(dev, "{\"clock\":\"-\"}", "missing \"clock\"");
    scv_reject(dev, "{\"serial.irq\":\"-\"}", "missing \"irq\"");
    scv_reject(dev, "{\"w65c51n\":\"yes\"}", "\"w65c51n\" must be true or false");
    scv_reject(dev, "{\"bus.phi2\":\"PHI9\"}", "no pin named \"PHI9\"");
    scv_reject(dev, "{\"parts.0.ctsA\":1}", "\"ctsA\" must be true or false");
    case_end("AC1 acia6551 loader: bus, data, registerSelect, strings, clock, serial, w65c51n, pins, ctsA");
}

/* ---- Formats: CR bit 7 (stop bits) ---------------------------------------------------- */

static void case_ac_stops(void) {
    HBench *h = ac_open("SFX-CDP65C51-so28");
    Bench *b = &h->b;
    SimTime t0;
    SimTime t;
    char message[120];

    case_begin();
    /* 8 bits, no parity, CR7 = 1: 2 stop bits. Two 0xFF bytes back to back:
       start bit to start bit is 1 + 8 + 2 bit times. */
    ac_cycle(b, 3, 1, 0x9F, NULL);
    ac_cycle(b, 2, 1, 0x0B, NULL);
    ac_cycle(b, 0, 1, 0xFF, NULL);
    check(h_wait(b, "TXD", LOGIC_0, 2 * AC_BT), "8N2: the first start bit");
    t0 = sim_now(b->sim);
    ac_cycle(b, 0, 1, 0xFF, NULL);
    t = ac_next_fall(b, "TXD", 14 * AC_BT) > 0 ? sim_now(b->sim) - t0 : 0;
    snprintf(message, sizeof message, "8N2: start to start %llu ps, want 11 bits",
             (unsigned long long)t);
    check(ac_near(t, 11 * AC_BT, AC_BT / 16), message);
    run(b, 12 * AC_BT);
    /* 5 bits, no parity, CR7 = 1: 1.5 stop bits (7.5 bit times a frame). */
    ac_cycle(b, 3, 1, 0xFF, NULL);
    ac_cycle(b, 0, 1, 0x1F, NULL);
    check(h_wait(b, "TXD", LOGIC_0, 2 * AC_BT), "5N1.5: the first start bit");
    t0 = sim_now(b->sim);
    ac_cycle(b, 0, 1, 0x1F, NULL);
    t = ac_next_fall(b, "TXD", 10 * AC_BT) > 0 ? sim_now(b->sim) - t0 : 0;
    snprintf(message, sizeof message, "5N1.5: start to start %llu ps, want 7.5 bits",
             (unsigned long long)t);
    check(ac_near(t, 15 * AC_BT / 2, AC_BT / 16), message);
    run(b, 10 * AC_BT);
    h_expect_sent(b, 0, 0x1F, "5-bit frames logged");
    case_end("AC2 CDP65C51: CR7 = 1 gives 2 stops (8N) and 1.5 stops (5N)");
    hb_close(h);
}

/* ---- CDP65C51A (ctsA): CTS high holds the next character, no TDRE IRQ ----------------- */

static void case_ac_cts_a(void) {
    HBench *h = ac_open(scv_part("SFX-CDP65C51", "{\"parts.0.ctsA\":true}"));
    Bench *b = &h->b;
    char got[16];
    char want[16];

    case_begin();
    ac_cycle(b, 3, 1, 0x1F, NULL);
    set(b, "CTS", LOGIC_1);
    run(b, NS(500));
    ac_cycle(b, 2, 1, 0x07, NULL);   /* TIC 01, IRD, DTR */
    run(b, NS(1000));
    expect_pin(b, "IRQ", LOGIC_Z, "ctsA: CTS high, no TDRE interrupt");
    set(b, "CTS", LOGIC_0);
    run(b, NS(1000));
    expect_pin(b, "IRQ", LOGIC_0, "ctsA: CTS low, the TDRE interrupt");
    set(b, "CTS", LOGIC_1);
    run(b, NS(500));
    ac_cycle(b, 0, 1, 'A', NULL);
    check(!h_wait(b, "TXD", LOGIC_0, 3 * AC_BT), "ctsA: CTS high holds the character");
    check(sent(b, NULL, 0) == 0, "ctsA: nothing sent while CTS is high");
    set(b, "CTS", LOGIC_0);
    check(h_catch(b, "TXD", AC_BT, 9, 3 * AC_BT, got), "ctsA: CTS low lets it go");
    h_frame_bits('A', 8, 0, 1, want);
    h_expect_text(got, want, "ctsA: the held A");
    case_end("AC3 CDP65C51A: CTS high holds the next character and its TDRE interrupt");
    hb_close(h);
}

/* ---- X inputs: CTS and RES --------------------------------------------------------------- */

static void case_ac_x(void) {
    HBench *h = ac_open("SFX-CDP65C51-so28");
    Bench *b = &h->b;

    case_begin();
    ac_cycle(b, 3, 1, 0x1F, NULL);
    ac_cycle(b, 2, 1, 0x0B, NULL);
    /* CTS at X: a 0 on TXD could be the mark CTS high forces: X. */
    ac_cycle(b, 0, 1, 0x00, NULL);
    check(h_wait(b, "TXD", LOGIC_0, 2 * AC_BT), "a zero byte starts");
    set(b, "CTS", LOGIC_X);
    run(b, NS(1000));
    expect_pin(b, "TXD", LOGIC_X, "CTS X during a 0 bit: TXD X");
    set(b, "CTS", LOGIC_0);
    run(b, 12 * AC_BT);
    expect_pin(b, "TXD", LOGIC_1, "CTS low again: TXD marks after the frame");
    /* CTS X while TXD marks: TXD stays 1. */
    set(b, "CTS", LOGIC_X);
    run(b, NS(1000));
    expect_pin(b, "TXD", LOGIC_1, "CTS X while marking: TXD 1");
    set(b, "CTS", LOGIC_0);
    /* RES at X: every output unknown. */
    set(b, "RES", LOGIC_X);
    run(b, NS(1000));
    expect_pin(b, "TXD", LOGIC_X, "RES X: TXD X");
    expect_pin(b, "RTS", LOGIC_X, "RES X: RTS X");
    expect_pin(b, "DTR", LOGIC_X, "RES X: DTR X");
    expect_pin(b, "IRQ", LOGIC_X, "RES X: IRQ X");
    expect_pin(b, "RXC", LOGIC_X, "RES X: RXC X");
    /* Released from X without a reset: the registers are unknown. */
    set(b, "RES", LOGIC_1);
    run(b, NS(1000));
    ac_expect(b, 2, "XXXXXXXX", "after RES X: command unknown");
    ac_expect(b, 3, "XXXXXXXX", "after RES X: control unknown");
    set(b, "RES", LOGIC_0);
    run(b, NS(1000));
    set(b, "RES", LOGIC_1);
    run(b, NS(1000));
    ac_expect(b, 2, "00000000", "a real reset clears the command again");
    case_end("AC4 CDP65C51: CTS X makes a 0 bit X; RES X makes every output X, registers X after");
    hb_close(h);
}

/* ---- Echo with an overrun --------------------------------------------------------------- */

static void case_ac_echo_overrun(void) {
    HBench *h = ac_open("SFX-CDP65C51-so28");
    Bench *b = &h->b;
    char want[16];
    int marked = 1;
    unsigned i;

    case_begin();
    ac_cycle(b, 3, 1, 0x1F, NULL);
    ac_cycle(b, 2, 1, 0x13, NULL);   /* REM, TIC 00, IRD, DTR */
    run(b, AC_BT);
    h_frame_bits(0x41, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, AC_BT);
    run(b, AC_BT);
    h_drive_bits(b, "RXD", want, AC_BT);   /* RDR not read: an overrun */
    run(b, AC_BT);
    ac_expect(b, 1, "00011100", "status: TDRE, RDRF, overrun");
    /* After an overrun in echo mode TXD marks; a new start bit before the RDR
       is read is not echoed. */
    set(b, "RXD", LOGIC_0);
    for (i = 0; i < 8; i++) {
        run(b, AC_BT / 4);
        marked &= get(b, "TXD") == LOGIC_1;
    }
    check(marked, "echo after an overrun: TXD marks");
    set(b, "RXD", LOGIC_1);
    run(b, 12 * AC_BT);
    ac_expect(b, 0, "01000001", "the RDR read");
    /* The mark ends at the next start bit after the read. */
    set(b, "RXD", LOGIC_0);
    run(b, AC_BT);
    expect_pin(b, "TXD", LOGIC_0, "echo resumes at the start bit after the RDR read");
    set(b, "RXD", LOGIC_1);
    run(b, 12 * AC_BT);
    case_end("AC5 CDP65C51 echo: an overrun marks TXD until a start bit after the RDR read");
    hb_close(h);
}

/* ---- Break with a character waiting ----------------------------------------------------- */

static void case_ac_break_pending(void) {
    HBench *h = ac_open("SFX-CDP65C51-so28");
    Bench *b = &h->b;
    char got[16];
    char want[16];

    case_begin();
    ac_cycle(b, 3, 1, 0x1F, NULL);
    ac_cycle(b, 2, 1, 0x0F, NULL);   /* TIC 11: break */
    run(b, AC_BT);
    expect_pin(b, "TXD", LOGIC_0, "TIC 11: break");
    ac_cycle(b, 0, 1, 'Z', NULL);
    run(b, 4 * AC_BT);
    check(sent(b, NULL, 0) == 0, "TIC 11: the written character is not sent");
    ac_expect(b, 1, "00000000", "TIC 11: TDRE stays 0 (the character waits)");
    ac_cycle(b, 2, 1, 0x0B, NULL);   /* TIC 10: normal transmission */
    check(h_catch(b, "TXD", AC_BT, 9, 4 * AC_BT, got), "TIC 10: the waiting character goes");
    h_frame_bits('Z', 8, 0, 1, want);
    h_expect_text(got, want, "the character held during the break");
    run(b, 2 * AC_BT);
    h_expect_sent(b, 0, 'Z', "Z logged");
    case_end("AC6 CDP65C51: TIC 11 sends no new character; it goes once TIC leaves 11");
    hb_close(h);
}

/* ---- W65C51N: a TDR write during the stop bit ------------------------------------------- */

static void case_ac_w65_stop_cut(void) {
    SimTime half = AC_XHALF;
    HBench *h = hb_open("W65C51N6TPG-14", AC_CLOCKS, 1, &half, 1);
    Bench *b = &h->b;
    uint16_t log[4];
    SimTime t0;
    SimTime t;
    char message[120];

    case_begin();
    ac_cs1 = "CS1B";
    ac_rw = "RWB";
    set(b, "RESB", LOGIC_0);
    set(b, "CS0", LOGIC_0);
    set(b, "CS1B", LOGIC_1);
    set(b, "PHI2", LOGIC_0);
    set(b, "RWB", LOGIC_1);
    set(b, "RxD", LOGIC_1);
    set(b, "CTSB", LOGIC_0);
    set(b, "DCDB", LOGIC_0);
    set(b, "DSRB", LOGIC_0);
    run(b, NS(2000));
    set(b, "RESB", LOGIC_1);
    run(b, NS(1000));
    ac_cycle(b, 3, 1, 0x1F, NULL);
    ac_cycle(b, 2, 1, 0x0B, NULL);
    ac_cycle(b, 0, 1, 'A', NULL);
    check(h_wait(b, "TxD", LOGIC_0, 2 * AC_BT), "A's start bit");
    t0 = sim_now(b->sim);
    /* Bit period 9 is A's stop bit: write B in it. */
    run(b, 9 * AC_BT + AC_BT / 4);
    ac_cycle(b, 0, 1, 'B', NULL);
    check(h_wait(b, "TxD", LOGIC_0, 2 * AC_BT), "B's start bit");
    t = sim_now(b->sim) - t0;
    snprintf(message, sizeof message, "B starts at the stop bit's end: %llu ps, want 10 bits",
             (unsigned long long)t);
    check(ac_near(t, 10 * AC_BT, AC_BT / 8), message);
    run(b, 11 * AC_BT);
    check(sent(b, log, 4) == 2 && log[0] == 'A' && log[1] == 'B',
          "a frame cut in its stop bit had sent its data: A and B logged");
    case_end("AC7 W65C51N: a TDR write in the stop bit starts the next frame; A is logged");
    hb_close(h);
}

int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_ac_loader();
    case_ac_stops();
    case_ac_cts_a();
    case_ac_x();
    case_ac_echo_overrun();
    case_ac_break_pending();
    case_ac_w65_stop_cut();
    return serial_bench_finish();
}
