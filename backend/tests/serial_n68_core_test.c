/*
 * serial_n68_core_test (plan-68-N): the ns16550 core (models/ns16550.c and
 * its pieces ns16550_io.c, ns16550_kind.c) and the printer port
 * (ns16550_lpt.c) on the shipped fixture devices: reset X, the INT and DMA
 * pin rules, DLAB X, per-channel and printer-port selects, the clock pin.
 * Each case cites the spec section (planning/plan-12-data/specs/) or the
 * model rule (the header comments) it checks.
 */

#include "serial_bench_exar.h"
#include "serial_bench_n68.h"

/* A read with the bus as text, holding `pin` at `level` during the strobe. */
static void expect_read_with(Bench *b, unsigned a, const char *pin, LogicValue level,
                             const char *want, const char *what) {
    set(b, pin, level);
    run(b, NS(20));
    expect_reg(b, a, want, what);
}

/* ---- Reset X: everything unknown until a clean reset (ns16550_kind.c apply_reset) ------ */

/* MR at X: the outputs go X (the model's X rule: "MR was X: everything
   unknown until a clean reset", ns16550_int.h NsState.unknown); a clean MR
   pulse restores the reset values (PC16550D p14 reset table). */
static void case_reset_x(void) {
    Bench *b = bench_open("SFX-PC16550D", HALF_18432);

    case_begin();
    set(b, "MR", LOGIC_X);
    run(b, NS(500));
    expect_pin(b, "SOUT", LOGIC_X, "MR X");
    expect_pin(b, "RTS", LOGIC_X, "MR X");
    expect_pin(b, "INTR", LOGIC_X, "MR X");
    expect_pin(b, "TXRDY", LOGIC_X, "MR X");
    expect_reg(b, 3, "ZZZZZZZZ", "MR X: the bus is off, a read is not driven");
    pulse_reset(b);
    expect_pin(b, "SOUT", LOGIC_1, "after a clean MR");
    expect_reg(b, 5, "01100000", "LSR after a clean MR");
    case_end("C1 PC16550D: MR X makes the outputs X until a clean reset");
    bench_close(b);
}

/* The same on a quad with shared DMA pins (TL16C554 "dmaPins": "sharedAnd")
   and on an Exar part with an RST output (ST16C1450 "resetOut"). */
static void case_reset_x_shared(void) {
    Bench *b = bench_open_cfg("SFX-TL16C554", HALF_18432, &QUAD_BENCH);

    case_begin();
    set(b, "RESET", LOGIC_X);
    run(b, NS(500));
    expect_pin(b, "TXRDY", LOGIC_X, "RESET X: shared TXRDY X");
    expect_pin(b, "RXRDY", LOGIC_X, "RESET X: shared RXRDY X");
    pulse_reset(b);
    expect_pin(b, "TXRDY", LOGIC_0, "after reset: every TX empty, shared TXRDY active");
    bench_close(b);
    b = bench_open_cfg("SFX-ST16C1450", HALF_18432, &E_ONE_BENCH);
    set(b, "RESET", LOGIC_X);
    run(b, NS(500));
    expect_pin(b, "RST", LOGIC_X, "RESET X: RST X");
    expect_pin(b, "TX", LOGIC_X, "RESET X: TX X");
    pulse_reset(b);
    expect_pin(b, "RST", LOGIC_0, "after reset: RST low");
    case_end("C2 TL16C554 / ST16C1450: RESET X on shared DMA pins and RST");
    bench_close(b);
}

/* ---- INT and RXRDY pin rules (ns16550.c) ------------------------------------------------ */

/* SC16C650B INT type MCR[5] (exar-d.md § SC16C650B MCR b5): 0 push-pull,
   1 "open source" (drives only the 1); MCR[5] written X: a 1 still drives 1,
   a 0 is X (driven 0 or released). */
static void case_open_source_x(void) {
    Bench *b = bench_open_cfg("SFX-SC16C650B", HALF_18432, &NXP650_BENCH);

    case_begin();
    program(b, 1, 0x03);
    bus_write(b, 3, 0xBF);
    bus_write(b, 2, 0x10);
    bus_write(b, 3, 0x03);
    bus_write(b, 4, 0x20);
    expect_pin(b, "INT", LOGIC_Z, "MCR[5] = 1, no interrupt: released");
    n68_write_x(b, 4, 0x00, 0x20);
    expect_pin(b, "INT", LOGIC_X, "MCR[5] X, no interrupt: X");
    bus_write(b, 1, 0x02);   /* THR empty interrupt */
    run(b, NS(500));
    expect_pin(b, "INT", LOGIC_1, "MCR[5] X, interrupt: driven 1");
    case_end("C3 SC16C650B: INT with MCR[5] X");
    bench_close(b);
}

/* TL16C752C DMA mode 1 (ti-c.md, TL16C752C FIFO/DMA): RXRDY is inactive when
   the FIFO is empty "or LSR[7] flags an error". */
static void case_rxrdy_lsr7(void) {
    Bench *b = bench_open_cfg("SFX-TL16C752C", NS(50), &DUAL_CD_BENCH);
    SimTime bt;

    case_begin();
    program(b, 1, 0x1B);   /* 8E1 */
    bt = bit_time(b, 1);
    bus_write(b, 2, 0x09);   /* FIFO, DMA mode 1, trigger 8 */
    drive_bits_on(b, "RXA", "0100000000", bt);   /* 0x01 with parity 0: an error */
    set(b, "RXA", LOGIC_1);
    run(b, bt * 60);
    expect_reg(b, 5, "11100101", "LSR: PE and LSR[7] (TX idle)");
    expect_pin(b, "RXRDYA", LOGIC_1, "mode 1 with LSR[7] = 1: RXRDY inactive");
    case_end("C4 TL16C752C: mode-1 RXRDY inactive while LSR[7] is 1");
    bench_close(b);
}

/* ---- Registers (ns16550_io.c) ----------------------------------------------------------- */

/* DLAB written X: DLL/RBR and DLM/IER read X (address 0 and 1 depend on it). */
static void case_dlab_x(void) {
    Bench *b = bench_open("SFX-PC16550D", HALF_18432);

    case_begin();
    n68_write_x(b, 3, 0x03, 0x80);
    expect_reg(b, 0, "XXXXXXXX", "LCR[7] X: address 0 reads X");
    expect_reg(b, 1, "XXXXXXXX", "LCR[7] X: address 1 reads X");
    case_end("C5 PC16550D: LCR[7] X makes addresses 0 and 1 X");
    bench_close(b);
}

/* "lsrErrors": "head" (TL16C752C LSR: errors are the head byte's; LSR[7]
   while any byte in the FIFO has an error): after the good head byte is
   read, LSR[7] stays 1 for the errored byte behind it. */
static void case_lsr_head(void) {
    Bench *b = bench_open_cfg("SFX-TL16C752C", NS(50), &DUAL_CD_BENCH);
    SimTime bt;

    case_begin();
    program(b, 1, 0x1B);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0x01);
    drive_bits_on(b, "RXA", "01000000011", bt);   /* 0x01, even parity 1: good */
    drive_bits_on(b, "RXA", "01000000001", bt);   /* 0x01, parity 0: PE */
    drive_bits_on(b, "RXA", "01000000011", bt);   /* good again */
    set(b, "RXA", LOGIC_1);
    run(b, bt * 2);
    expect_reg(b, 5, "11100001", "head good, an error behind: LSR[7], no PE");
    bus_read(b, 0);
    expect_reg(b, 5, "11100101", "the errored byte at the head: PE and LSR[7]");
    bus_read(b, 0);
    expect_reg(b, 5, "01100001", "the errored byte read: LSR[7] clear");
    case_end("C6 TL16C752C: head errors, LSR[7] follows the bytes still in the FIFO");
    bench_close(b);
}

/* ---- Selects (ns16550_io.c nsc_bus_select, ns16550_kind.c is_select_pin) ------------- */

static const BenchConfig QUAD_HIGH_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"RXA=1", "RXB=1", "RXC=1", "RXD=1", "CTSA=1", "CTSB=1", "CTSC=1", "CTSD=1", "DSRA=1", "DSRB=1",
     "DSRC=1", "DSRD=1", "DCDA=1", "DCDB=1", "DCDC=1", "DCDD=1", "RIA=1", "RIB=1", "RIC=1", "RID=1",
     "INTN=0", NULL},
    "RESET", 1, "XTAL1", NULL, NULL,
    {{"CSA=1"}, {"CSB=0"}, {"CSC=0"}, {"CSD=0"}}, 4
};

/* A channel selected by an active-high select ("selectHigh", a variant of the
   TL16C554): CSA high selects channel A; a common select that is X makes
   every access X. */
static void case_select_high(void) {
    Bench *b = bench_open_cfg(n68_part("SFX-TL16C554",
                                       "{\"channels.0.select\":\"-\",\"channels.0.selectHigh\":[\"CSA\"]}"),
                              HALF_18432, &QUAD_HIGH_BENCH);

    case_begin();
    select_unit(b, 0);
    bus_write(b, 7, 0x5A);
    select_unit(b, 1);
    bus_write(b, 7, 0x3C);
    select_unit(b, 0);
    expect_reg(b, 7, "01011010", "CSA high: channel A's SCR");
    select_unit(b, 1);
    expect_reg(b, 7, "00111100", "CSB low: channel B's SCR");
    bench_close(b);
    b = bench_open_cfg(n68_part("SFX-TL16C554",
                                "{\"intPin\":\"driven\",\"intControl\":\"-\",\"bus.selectHigh\":[\"INTN\"]}"),
                       HALF_18432, &QUAD_BENCH);
    set(b, "INTN", LOGIC_1);
    run(b, NS(100));
    bus_write(b, 7, 0x66);
    expect_reg(b, 7, "01100110", "common select active: channel A");
    expect_read_with(b, 7, "INTN", LOGIC_X, "XXXXXXXX", "common select X: the read is X");
    case_end("C7 TL16C554 variants: an active-high channel select; a common select X");
    bench_close(b);
}

/* A printer-port device with one common UART select (a TL16C451 variant:
   "bus.select" instead of the channel's): the UART and the port each answer
   only their own select; a port select at X makes the access X. */
static void case_lpt_common(void) {
    Bench *b = bench_open_cfg(n68_part("SFX-TL16C451",
                                       "{\"channels.0.select\":\"-\",\"bus.select\":[\"CS0\"]}"),
                              HALF_18432, &TI451_BENCH);

    case_begin();
    select_unit(b, 0);
    bus_write(b, 7, 0x5A);
    select_unit(b, 1);
    bus_write(b, 0, 0xA5);   /* the port's data latch */
    expect_reg(b, 1, "11011111", "port status: BUSY low -> b7 1, ACK/SLCT/ERROR high, b2:0 = 1");
    select_unit(b, 0);
    expect_reg(b, 7, "01011010", "the UART's SCR kept 0x5A");
    select_unit(b, 1);
    expect_read_with(b, 1, "CS2", LOGIC_X, "XXXXXXXX", "port select X: the read is X");
    case_end("C8 TL16C451 variant: common UART select beside the printer port");
    bench_close(b);
}

/* "channelSelect" (TL16C2552 CHSEL, ti-c.md § TL16C2552): CHSEL X with CS
   active makes the access X. */
static void case_chsel_x(void) {
    Bench *b = bench_open_cfg("SFX-TL16C2552", HALF_18432, &CHSEL_BENCH);

    case_begin();
    bus_write(b, 7, 0x12);
    expect_reg(b, 7, "00010010", "CHSEL 1: channel A");
    expect_read_with(b, 7, "CHSEL", LOGIC_X, "XXXXXXXX", "CHSEL X: the read is X");
    case_end("C9 TL16C2552: CHSEL X");
    bench_close(b);
}

/* ---- The clock pin (ns16550_kind.c clock_edge) --------------------------------------- */

/* The PC16550D bench with XIN driven by the test (no bench clock). */
static const BenchConfig PC_NOCLOCK_BENCH = {
    "RDBAR", "WRBAR",
    {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"},
    {"A0", "A1", "A2"}, 3,
    {"CS0=1", "CS1=1", "CS2=0", "ADS=0", "RD=0", "WR=0", "SIN=1", "CTS=1", "DSR=1", "DCD=1",
     "RI=1", NULL},
    "MR", 1, "-", "BAUDOUT", "RCLK",
    {{NULL}}, 0
};

/* XIN X then Z: no edge either way (Z reads X). With divisor 1 BAUDOUT is
   the clock (PC16550D: BAUDOUT = XIN / divisor), so it follows XIN into X
   and stays X; a clean 0 -> 1 edge brings it back. */
static void case_clock_x(void) {
    Bench *b = bench_open_cfg("SFX-PC16550D", HALF_18432, &PC_NOCLOCK_BENCH);
    unsigned i;

    case_begin();
    for (i = 0; i < 4; i++) {
        set(b, "XIN", i & 1u ? LOGIC_1 : LOGIC_0);
        run(b, NS(271));
    }
    program(b, 1, 0x03);
    set(b, "XIN", LOGIC_1);
    run(b, NS(271));
    set(b, "XIN", LOGIC_0);
    run(b, NS(271));
    set(b, "XIN", LOGIC_1);
    run(b, NS(271));
    expect_pin(b, "BAUDOUT", LOGIC_1, "divisor 1, XIN 1: BAUDOUT 1");
    set(b, "XIN", LOGIC_X);
    run(b, NS(271));
    expect_pin(b, "BAUDOUT", LOGIC_X, "XIN X, divisor 1: BAUDOUT X");
    set(b, "XIN", LOGIC_Z);
    run(b, NS(271));
    expect_pin(b, "BAUDOUT", LOGIC_X, "XIN Z (reads X): BAUDOUT still X");
    set(b, "XIN", LOGIC_0);
    run(b, NS(271));
    set(b, "XIN", LOGIC_1);
    run(b, NS(271));
    expect_pin(b, "BAUDOUT", LOGIC_1, "clean clock again: BAUDOUT 1");
    case_end("C10 PC16550D: XIN into X and Z");
    bench_close(b);
}

int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_reset_x();
    case_reset_x_shared();
    case_open_source_x();
    case_rxrdy_lsr7();
    case_dlab_x();
    case_lsr_head();
    case_select_high();
    case_lpt_common();
    case_chsel_x();
    case_clock_x();
    return serial_bench_finish();
}
