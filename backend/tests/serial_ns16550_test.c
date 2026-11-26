/*
 * serial_ns16550_test: the ns16550 core on the fixture devices (cases 1-12), the terminal outcome, the host port
 * (split from serial_test.c by plan-68-G; the bench is support/serial_bench.c).
 */

#include "serial_bench.h"

/* ---- 1. Reset values ------------------------------------------------------------------ */

static void case_reset(void) {
    Bench *b = bench_open(c_part, HALF_18432);

    case_begin();
    expect_reg(b, 1, "00000000", "IER after MR");
    expect_reg(b, 2, "00000001", "IIR after MR");
    expect_reg(b, 3, "00000000", "LCR after MR");
    expect_reg(b, 4, "00000000", "MCR after MR");
    expect_reg(b, 5, "01100000", "LSR after MR");
    expect_reg(b, 6, "00000000", "MSR after MR (inputs inactive)");
    expect_reg(b, 7, "00000000", "SCR after MR (decisions: 0x00)");
    expect_reg(b, 0, "XXXXXXXX", "RBR at power-on (not reset)");
    bus_write(b, 3, 0x80);
    expect_reg(b, 0, "XXXXXXXX", "DLL at power-on");
    expect_reg(b, 1, "XXXXXXXX", "DLM at power-on");
    bus_write(b, 3, 0x00);
    expect_pin(b, "SOUT", LOGIC_1, "after MR");
    expect_pin(b, "INTR", LOGIC_0, "after MR");
    expect_pin(b, "RTS", LOGIC_1, "after MR");
    expect_pin(b, "DTR", LOGIC_1, "after MR");
    expect_pin(b, "OUT1", LOGIC_1, "after MR");
    expect_pin(b, "OUT2", LOGIC_1, "after MR");
    expect_pin(b, "RXRDY", LOGIC_1, "after MR");
    expect_pin(b, "TXRDY", LOGIC_0, "after MR");
    expect_pin(b, "DDIS", LOGIC_1, "no read");
    expect_pin(b, "BAUDOUT", LOGIC_X, "divisor X");
    expect_pin(b, "D0", LOGIC_Z, "bus released");
    /* Modem inputs: CTS low -> MSR[4] = 1 with DCTS. */
    set(b, "CTS", LOGIC_0);
    run(b, NS(500));
    expect_reg(b, 6, "00010001", "MSR after CTS goes active");
    /* MR resets the registers again but leaves SCR's power-on rule and DLL. */
    bus_write(b, 7, 0x5A);
    bus_write(b, 1, 0x0F);
    set(b, "MR", LOGIC_1);
    run(b, NS(6000));
    set(b, "MR", LOGIC_0);
    run(b, NS(100));
    expect_reg(b, 1, "00000000", "IER after a second MR");
    expect_reg(b, 6, "00010000", "MSR after MR: deltas cleared, CTS still active");
    /* A write cut short by MR is not stored: an X-data write to LSR (factory
       test bits) in progress when MR rises leaves LSR at its reset value. */
    set_address(b, 5);
    set_data(b, 0, 0);
    run(b, NS(40));
    set(b, b->cfg->write, LOGIC_0);
    run(b, NS(120));
    set(b, "MR", LOGIC_1);
    run(b, NS(6000));
    set(b, b->cfg->write, LOGIC_1);
    run(b, NS(100));
    set(b, "MR", LOGIC_0);
    run(b, NS(100));
    expect_reg(b, 5, "01100000", "LSR after a write cut short by MR");
    case_end("1 reset values and outputs after MR");
    bench_close(b);
}

/* ---- 2. Divisor and BAUDOUT ------------------------------------------------------------ */

/* Time between two BAUDOUT rising edges. */
static SimTime baud_period(Bench *b) {
    SimTime step = NS(10);
    SimTime t0 = 0;
    LogicValue last = get(b, "BAUDOUT");
    int edges = 0;
    SimTime spent = 0;

    while (edges < 2 && spent < NS(200000)) {
        LogicValue now;

        run(b, step);
        spent += step;
        now = get(b, "BAUDOUT");
        if (last == LOGIC_0 && now == LOGIC_1) {
            if (edges == 0) {
                t0 = sim_now(b->sim);
            }
            edges++;
            if (edges == 2) {
                return sim_now(b->sim) - t0;
            }
        }
        last = now;
    }
    return 0;
}

static void case_divisor(void) {
    Bench *b = bench_open(c_part, NS(50));   /* 10 MHz clock */
    SimTime p;
    char message[96];

    case_begin();
    bus_write(b, 3, 0x80);
    bus_write(b, 0, 0x03);
    bus_write(b, 1, 0x00);
    expect_reg(b, 0, "00000011", "DLL reads back");
    expect_reg(b, 1, "00000000", "DLM reads back");
    p = baud_period(b);
    snprintf(message, sizeof message, "BAUDOUT period %llu ps, want 3 x 100 ns",
             (unsigned long long)p);
    check(p == NS(300), message);
    bus_write(b, 0, 0x0A);
    p = baud_period(b);
    snprintf(message, sizeof message, "BAUDOUT period %llu ps, want 10 x 100 ns",
             (unsigned long long)p);
    check(p == NS(1000), message);
    bus_write(b, 0, 0x01);
    p = baud_period(b);
    snprintf(message, sizeof message, "divisor 1: BAUDOUT period %llu ps, want the clock's",
             (unsigned long long)p);
    check(p == NS(100), message);
    bus_write(b, 0, 0x00);
    run(b, NS(500));
    expect_pin(b, "BAUDOUT", LOGIC_X, "divisor 0");
    case_end("2 divisor latch, BAUDOUT period, divisor 0");
    bench_close(b);
}

/* ---- 3. Transmit frames ------------------------------------------------------------------ */

static void tx_one(Bench *b, unsigned lcr, unsigned byte, unsigned bits, const char *want,
                   const char *what) {
    char got[24];
    char message[128];

    bus_write(b, 3, lcr);
    bus_write(b, 0, byte);
    check(wait_start(b, NS(20000)), what);
    sample_frame(b, 1, bits, got);
    snprintf(message, sizeof message, "%s: SOUT %s, want %s", what, got, want);
    check(strcmp(got, want) == 0, message);
    run(b, bit_time(b, 1) * 4);
}

static void case_tx(void) {
    Bench *b = bench_open(c_part, HALF_18432);
    uint16_t frames[8];
    SimTime t0;
    SimTime t1;
    char message[96];

    case_begin();
    program(b, 1, 0x03);
    /* 8N1 'H' = 0x48: start, 0 0 0 1 0 0 1 0, stop. */
    tx_one(b, 0x03, 0x48, 9, "0000100101", "8N1 0x48");
    /* 7E2 0x35 = 0110101: data LSB first 1010110, parity even (4 ones) 0, stops 1 1. */
    tx_one(b, 0x1E, 0x35, 10, "01010110011", "7E2 0x35");
    /* 5O1.5 0x13 = 10011: data 11001, three ones: odd parity 0, stop. */
    tx_one(b, 0x0C, 0x13, 7, "01100101", "5O1.5 0x13");
    /* Stick parity: PEN, EPS, stick -> parity 0; EPS 0 -> parity 1. */
    tx_one(b, 0x3B, 0xFF, 10, "01111111101", "8, stick parity 0");
    tx_one(b, 0x2B, 0x00, 10, "00000000011", "8, stick parity 1");
    check(sent(b, frames, 8) == 5 && frames[0] == 0x48 && frames[1] == 0x35 &&
              frames[2] == 0x13 && frames[3] == 0xFF && frames[4] == 0x00,
          "the sent log holds the data bits");
    /* Bit time = 16 x divisor clocks: the start bit's length. */
    bus_write(b, 3, 0x03);
    bus_write(b, 0, 0x01);
    check(wait_start(b, NS(20000)), "start of 0x01");
    t0 = sim_now(b->sim);
    while (get(b, "SOUT") == LOGIC_0) {
        run(b, NS(10));
    }
    t1 = sim_now(b->sim);
    snprintf(message, sizeof message, "start bit %llu ps, want 16 clocks (%llu)",
             (unsigned long long)(t1 - t0), (unsigned long long)bit_time(b, 1));
    check(t1 - t0 >= bit_time(b, 1) - b->period / 4 && t1 - t0 <= bit_time(b, 1) + NS(200),
          message);
    /* 1.5 stop bits = 24 clocks: back-to-back 5O1.5 frames start 16 + 5 x 16
       + 16 + 24 = 136 clocks apart. */
    run(b, bit_time(b, 1) * 12);
    bus_write(b, 2, 0x01);
    bus_write(b, 3, 0x0C);
    bus_write(b, 0, 0x00);
    bus_write(b, 0, 0x00);
    /* Parity rises (data 0, odd parity 1) frame to frame, measured to 10 ns. */
    check(wait_start(b, NS(20000)), "first 5O1.5 start");
    while (get(b, "SOUT") == LOGIC_0) {
        run(b, NS(10));
    }
    t0 = sim_now(b->sim);
    while (get(b, "SOUT") != LOGIC_0) {
        run(b, NS(10));
    }
    while (get(b, "SOUT") == LOGIC_0) {
        run(b, NS(10));
    }
    t1 = sim_now(b->sim);
    snprintf(message, sizeof message, "5O1.5 frame to frame %llu ps, want 136 clocks (%llu)",
             (unsigned long long)(t1 - t0), (unsigned long long)(136 * b->period));
    check(t1 - t0 + NS(20) >= 136 * b->period && t1 - t0 <= 136 * b->period + NS(20), message);
    case_end("3 transmit 8N1, 7E2, 5O1.5, stick parity; bit time; sent log");
    bench_close(b);
}

/* ---- 4. Receive ------------------------------------------------------------------------------ */

static void case_rx(void) {
    Bench *b = bench_open(c_part, HALF_18432);
    SimTime bt;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    drive_byte(b, 0x5A, bt, 1);
    run(b, bt * 2);
    expect_reg(b, 5, "01100001", "LSR: DR after a frame");
    expect_reg(b, 0, "01011010", "RBR = 0x5A");
    expect_reg(b, 5, "01100000", "LSR: DR cleared by the RBR read");

    /* A false start: low for a quarter bit. */
    set(b, "SIN", LOGIC_0);
    run(b, bt / 4);
    set(b, "SIN", LOGIC_1);
    run(b, bt * 12);
    expect_reg(b, 5, "01100000", "a short low pulse is no start bit");

    /* Framing error, then the resync: the bad stop bit is the next start. */
    drive_bits(b, "0101010100", bt);          /* 0x55 with stop 0 */
    drive_bits(b, "11000000", bt);            /* the next char's data: 0x03 ... */
    drive_bits(b, "11", bt);                  /* ... its stop */
    set(b, "SIN", LOGIC_1);
    run(b, bt * 2);
    expect_reg(b, 5, "01101011", "LSR: FE, and OE (the resync char overran the first), DR");
    expect_reg(b, 0, "00000011", "the resync character: the 8 bits after the bad stop");

    /* Parity error: 8E1 with odd parity on the line. */
    bus_write(b, 3, 0x1B);
    drive_bits(b, "0100000001", bt);          /* 0x01, parity 1 (even wants 1? one 1 -> 1) */
    drive_bits(b, "1", bt);
    run(b, bt * 2);
    expect_reg(b, 5, "01100001", "even parity with the right parity bit: no PE");
    bus_read(b, 0);
    drive_bits(b, "0100000000", bt);          /* parity 0: wrong */
    drive_bits(b, "1", bt);
    run(b, bt * 2);
    expect_reg(b, 5, "01100101", "a wrong parity bit sets PE");
    bus_read(b, 0);

    /* Break: low for two character times: BI with FE, one 0x00. */
    bus_write(b, 3, 0x03);
    set(b, "SIN", LOGIC_0);
    run(b, bt * 22);
    set(b, "SIN", LOGIC_1);
    run(b, bt * 3);
    expect_reg(b, 5, "01111001", "LSR: BI and FE after a break");
    expect_reg(b, 0, "00000000", "the break character is 0x00");
    expect_reg(b, 5, "01100000", "only one character for the break");
    case_end("4 receive: data, false start, framing error and resync, parity, break");
    bench_close(b);
}

/* ---- 5. FIFO ---------------------------------------------------------------------------------- */

static void case_fifo(void) {
    Bench *b = bench_open(c_part, HALF_18432);
    SimTime bt;
    unsigned i;
    char text[24];

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0xC1);   /* FIFO on, trigger 14 */
    bus_write(b, 1, 0x01);   /* ERBFI */
    expect_reg(b, 2, "11000001", "IIR: FIFOs enabled, nothing pending");
    for (i = 0; i < 13; i++) {
        drive_byte(b, 0x30 + i, bt, 1);
    }
    run(b, bt);
    expect_pin(b, "INTR", LOGIC_0, "13 bytes, trigger 14");
    drive_byte(b, 0x3D, bt, 1);
    run(b, bt);
    expect_pin(b, "INTR", LOGIC_1, "14 bytes reach trigger 14");
    expect_reg(b, 2, "11000100", "IIR: RX data available");
    drive_byte(b, 0x3E, bt, 1);
    drive_byte(b, 0x3F, bt, 1);
    run(b, bt);
    expect_reg(b, 5, "01100001", "16 bytes: no overrun");
    drive_byte(b, 0x40, bt, 1);
    run(b, bt);
    expect_reg(b, 5, "01100011", "the 17th byte overruns");
    for (i = 0; i < 16; i++) {
        bus_read_text(b, 0, text);
        if (i == 0) {
            check(strcmp(text, "00110000") == 0, "the first byte is first out");
        }
        if (i == 15) {
            check(strcmp(text, "00111111") == 0, "the 16th byte is last; the 17th was lost");
        }
    }
    expect_reg(b, 5, "01100000", "empty after 16 reads");

    /* Trigger levels 1, 4, 8. */
    {
        static const unsigned FCR[3] = {0x01, 0x41, 0x81};
        static const unsigned LEVEL[3] = {1, 4, 8};
        unsigned t;

        for (t = 0; t < 3; t++) {
            bus_write(b, 2, FCR[t] | 0x02);
            for (i = 0; i + 1 < LEVEL[t]; i++) {
                drive_byte(b, 0x41, bt, 1);
            }
            run(b, bt);
            check(get(b, "INTR") == LOGIC_0, "below the trigger level: no interrupt");
            drive_byte(b, 0x42, bt, 1);
            run(b, bt);
            check(get(b, "INTR") == LOGIC_1, "at the trigger level: interrupt");
        }
    }

    /* LSR[7]: an error character in the FIFO; FCR[1] clears the RX FIFO. */
    bus_write(b, 2, 0xC3);
    drive_byte(b, 0x11, bt, 1);
    drive_byte(b, 0x22, bt, 0);   /* framing error */
    set(b, "SIN", LOGIC_1);
    run(b, bt * 12);
    expect_reg(b, 5, "11100001", "LSR[7]: an error further down the FIFO");
    expect_reg(b, 5, "11100001", "LSR[7] stays: the error is not at the top yet");
    bus_read(b, 0);
    expect_reg(b, 5, "11101001", "the error char at the top shows FE; LSR[7] while it is in the FIFO");
    expect_reg(b, 5, "01100001", "that LSR read cleared FE and LSR[7] (no later errors)");
    bus_write(b, 2, 0xC3);
    expect_reg(b, 5, "01100000", "FCR[1] cleared the RX FIFO");
    case_end("5 FIFO: trigger levels 1/4/8/14, overrun on the 17th, LSR[7], FCR[1]");
    bench_close(b);
}

/* ---- 6. Timeout ----------------------------------------------------------------------------- */

static void case_timeout(void) {
    Bench *b = bench_open(c_part, HALF_18432);
    SimTime bt;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0x41);   /* FIFO, trigger 4 */
    bus_write(b, 1, 0x01);
    drive_byte(b, 0x61, bt, 1);
    run(b, bt * 30);
    expect_pin(b, "INTR", LOGIC_0, "3 character times: no timeout yet");
    run(b, bt * 12);
    expect_pin(b, "INTR", LOGIC_1, "after 4 character times: timeout");
    expect_reg(b, 2, "11001100", "IIR = C (timeout)");
    expect_reg(b, 0, "01100001", "the byte");
    expect_reg(b, 2, "11000001", "the RBR read cleared the timeout");
    run(b, NS(1000));
    expect_pin(b, "INTR", LOGIC_0, "INTR cleared (trint after the read)");
    case_end("6 timeout after 4 character times, cleared by an RBR read");
    bench_close(b);
}

/* ---- 7. Interrupt priority ------------------------------------------------------------------------ */

static void case_priority(void) {
    Bench *b = bench_open(c_part, HALF_18432);
    SimTime bt;
    SimTime t;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    drive_byte(b, 0x77, bt, 0);   /* framing error: line status + data */
    set(b, "SIN", LOGIC_1);
    run(b, bt * 3);
    set(b, "CTS", LOGIC_0);       /* modem */
    run(b, NS(500));
    bus_write(b, 1, 0x0F);        /* all enabled; THRE already empty -> THRE source */
    expect_reg(b, 2, "00000110", "line status first");
    bus_read(b, 5);
    expect_reg(b, 2, "00000100", "then RX data");
    bus_read(b, 0);
    expect_reg(b, 2, "00000010", "then THRE");
    expect_reg(b, 2, "00000000", "the IIR read that reported THRE cleared it: modem");
    bus_read(b, 6);
    expect_reg(b, 2, "00000001", "nothing after the MSR read");
    expect_pin(b, "INTR", LOGIC_0, "nothing pending");

    /* A THR write clears THRE; it comes back when the byte moves to the shifter. */
    bus_write(b, 1, 0x00);
    bus_write(b, 1, 0x02);
    expect_reg(b, 2, "00000010", "enabling ETBEI with THRE = 1 raises THRE");
    bus_write(b, 0, 0x55);
    expect_reg(b, 5, "00100000", "the byte moved to the shifter at once: THRE again, TEMT 0");
    run(b, bt * 12);

    /* Transmitter interrupt delay (FIFO mode): the first after FCR[0] is
       immediate; a lone byte later delays THRE to its last stop bit. */
    bus_write(b, 2, 0x01);
    bus_write(b, 0, 0x11);
    run(b, bt / 2);
    check((bus_read(b, 5) & 0x20) != 0, "first THRE after FCR[0] is immediate");
    run(b, bt * 12);
    bus_read(b, 2);
    bus_write(b, 0, 0x22);
    t = sim_now(b->sim);
    run(b, bt * 2);
    check((bus_read(b, 5) & 0x20) == 0, "a lone byte: THRE delayed while it is sent");
    while ((bus_read(b, 5) & 0x20) == 0 && sim_now(b->sim) - t < bt * 12) {
    }
    t = sim_now(b->sim) - t;
    check(t > bt * 8 && t < bt * 10, "THRE comes at the last stop bit (9 bit times in)");
    run(b, bt * 4);
    /* Two bytes at once: THRE as soon as the FIFO empties. */
    bus_write(b, 0, 0x33);
    bus_write(b, 0, 0x44);
    run(b, bt * 11);
    check((bus_read(b, 5) & 0x20) != 0, "two bytes at once: THRE when the FIFO empties");
    case_end("7 interrupt priority, IIR codes, THRE clears, transmitter interrupt delay");
    bench_close(b);
}

/* ---- 8. Modem status -------------------------------------------------------------------------------- */

static void case_modem(void) {
    Bench *b = bench_open(c_part, HALF_18432);

    case_begin();
    bus_write(b, 1, 0x08);
    set(b, "DSR", LOGIC_0);
    set(b, "DCD", LOGIC_0);
    run(b, NS(500));
    expect_pin(b, "INTR", LOGIC_1, "modem interrupt");
    expect_reg(b, 2, "00000000", "IIR: modem status");
    expect_reg(b, 6, "10101010", "MSR: DSR, DCD active, DDSR, DDCD");
    expect_reg(b, 6, "10100000", "deltas cleared by the read");
    expect_pin(b, "INTR", LOGIC_0, "interrupt cleared by the MSR read");
    set(b, "RI", LOGIC_0);
    run(b, NS(500));
    expect_reg(b, 6, "11100000", "RI active: no TERI on the leading edge");
    set(b, "RI", LOGIC_1);
    run(b, NS(500));
    expect_reg(b, 6, "10100100", "TERI on the trailing edge of RI");
    expect_reg(b, 6, "10100000", "TERI cleared");
    /* MCR drives the outputs inverted. */
    bus_write(b, 4, 0x0F);
    run(b, NS(300));
    expect_pin(b, "DTR", LOGIC_0, "MCR[0]");
    expect_pin(b, "RTS", LOGIC_0, "MCR[1]");
    expect_pin(b, "OUT1", LOGIC_0, "MCR[2]");
    expect_pin(b, "OUT2", LOGIC_0, "MCR[3]");
    case_end("8 modem: delta bits, TERI on the trailing RI edge, MSR read clears, interrupt");
    bench_close(b);
}

/* ---- 9. Loopback ------------------------------------------------------------------------------------- */

static void case_loopback(void) {
    Bench *b = bench_open(c_part, HALF_18432);
    SimTime bt;
    SimTime spent = 0;
    int stayed = 1;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 4, 0x1F);   /* loop + all modem outputs */
    run(b, NS(300));
    expect_reg(b, 6, "11111011", "MSR mirrors MCR (DTR->DSR, RTS->CTS, OUT1->RI, OUT2->DCD)");
    expect_pin(b, "RTS", LOGIC_1, "modem outputs inactive in loopback");
    expect_pin(b, "OUT2", LOGIC_1, "modem outputs inactive in loopback");
    bus_write(b, 0, 0xA5);
    while (spent < bt * 12) {
        stayed &= get(b, "SOUT") == LOGIC_1;
        run(b, NS(500));
        spent += NS(500);
    }
    check(stayed, "SOUT stays 1 in loopback");
    expect_reg(b, 0, "10100101", "the sent byte arrives in RBR");
    case_end("9 loopback: SOUT 1, TX to RX, MSR from MCR");
    bench_close(b);
}

/* ---- 10. DMA modes ---------------------------------------------------------------------------------- */

static void case_dma(void) {
    Bench *b = bench_open(c_part, HALF_18432);
    SimTime bt;
    unsigned i;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    /* Mode 0. */
    expect_pin(b, "TXRDY", LOGIC_0, "mode 0: TXRDY active when empty");
    expect_pin(b, "RXRDY", LOGIC_1, "mode 0: RXRDY inactive when empty");
    drive_byte(b, 0x01, bt, 1);
    run(b, bt);
    expect_pin(b, "RXRDY", LOGIC_0, "mode 0: RXRDY active with one character");
    bus_read(b, 0);
    run(b, NS(300));
    expect_pin(b, "RXRDY", LOGIC_1, "mode 0: RXRDY inactive after the read (twrr)");
    /* Mode 1: FIFO, DMA mode, trigger 4. */
    bus_write(b, 2, 0x49);
    drive_byte(b, 0x02, bt, 1);
    run(b, bt);
    expect_pin(b, "RXRDY", LOGIC_1, "mode 1: one character is not enough");
    for (i = 0; i < 3; i++) {
        drive_byte(b, 0x03, bt, 1);
    }
    run(b, bt);
    expect_pin(b, "RXRDY", LOGIC_0, "mode 1: RXRDY at the trigger level");
    for (i = 0; i < 3; i++) {
        bus_read(b, 0);
    }
    expect_pin(b, "RXRDY", LOGIC_0, "mode 1: stays active until empty");
    bus_read(b, 0);
    run(b, NS(300));
    expect_pin(b, "RXRDY", LOGIC_1, "mode 1: inactive when the FIFO is empty");
    for (i = 0; i < 16; i++) {
        bus_write(b, 0, 0x30 + i);
    }
    expect_pin(b, "TXRDY", LOGIC_0, "mode 1: TXRDY active until the FIFO is full");
    bus_write(b, 0, 0x50);
    expect_pin(b, "TXRDY", LOGIC_1, "mode 1: TXRDY inactive when full");
    case_end("10 DMA modes 0 and 1 on TXRDY and RXRDY");
    bench_close(b);
}

/* ---- 11. Bus timing and strobes ----------------------------------------------------------------------- */

static void case_bus(void) {
    Bench *b = bench_open(c_part, HALF_18432);
    char text[9];

    case_begin();
    bus_write(b, 7, 0x96);
    set_address(b, 7);
    run(b, NS(50));
    set(b, "RDBAR", LOGIC_0);
    run(b, NS(59));
    expect_pin(b, "D1", LOGIC_Z, "59 ns into the read: not yet (trvd 60)");
    expect_pin(b, "DDIS", LOGIC_1, "59 ns: DDIS still high (trdd 60)");
    run(b, NS(2));
    expect_pin(b, "D1", LOGIC_1, "61 ns: data");
    expect_pin(b, "DDIS", LOGIC_0, "61 ns: DDIS low");
    set(b, "RDBAR", LOGIC_1);
    run(b, NS(99));
    expect_pin(b, "D1", LOGIC_1, "99 ns after the read: still driven (thz 100)");
    run(b, NS(2));
    expect_pin(b, "D1", LOGIC_Z, "101 ns: floating");

    /* ADS: the address latched on its rising edge is held while it is high. */
    set_address(b, 7);
    run(b, NS(50));
    set(b, "ADS", LOGIC_1);
    run(b, NS(50));
    set_address(b, 3);
    run(b, NS(50));
    set(b, "RDBAR", LOGIC_0);
    run(b, NS(150));
    check(get(b, "D1") == LOGIC_1 && get(b, "D2") == LOGIC_1 && get(b, "D0") == LOGIC_0,
          "ADS high: the latched address (SCR) is read");
    set(b, "RDBAR", LOGIC_1);
    run(b, NS(150));
    set(b, "ADS", LOGIC_0);
    run(b, NS(50));

    /* X on RD during a read drives X. */
    set_address(b, 7);
    set(b, "RD", LOGIC_X);
    run(b, NS(150));
    expect_pin(b, "D1", LOGIC_X, "X read strobe: X data");
    set(b, "RD", LOGIC_0);
    run(b, NS(200));
    /* RDBAR low and RD high together: X (decisions.md). */
    set(b, "RDBAR", LOGIC_0);
    set(b, "RD", LOGIC_1);
    run(b, NS(150));
    expect_pin(b, "D1", LOGIC_X, "both read strobes active: X data");
    set(b, "RDBAR", LOGIC_1);
    set(b, "RD", LOGIC_0);
    run(b, NS(200));
    /* WR high alone is a write. */
    set_address(b, 7);
    set_data(b, 1, 0x3C);
    run(b, NS(40));
    set(b, "WR", LOGIC_1);
    run(b, NS(120));
    set(b, "WR", LOGIC_0);
    run(b, NS(40));
    set_data(b, 0, 0);
    run(b, NS(100));
    bus_read_text(b, 7, text);
    check(strcmp(text, "00111100") == 0, "the active-high write strobe writes");
    case_end("11 bus: trvd, thz, DDIS, ADS latch, X and double read strobes, WR");
    bench_close(b);
}

/* ---- 12. 16450 ---------------------------------------------------------------------------------------- */

static void case_16450(void) {
    Bench *b = bench_open("SFX-16450", HALF_18432);
    SimTime bt;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0xC1);
    expect_reg(b, 2, "00000001", "16450: FCR write ignored, IIR[7:6] = 00");
    bus_write(b, 1, 0x01);
    drive_byte(b, 0x31, bt, 1);
    drive_byte(b, 0x32, bt, 1);
    run(b, bt);
    expect_reg(b, 5, "01100011", "16450: the second byte overruns (no FIFO)");
    expect_reg(b, 0, "00110010", "16450: the RBR holds the last byte");
    drive_byte(b, 0x33, bt, 1);
    run(b, bt * 60);
    expect_reg(b, 2, "00000100", "16450: data available, never a timeout");
    case_end("12 16450: no FCR, no FIFO, no timeout");
    bench_close(b);
}

/* ---- Outcome: the terminal ---------------------------------------------------------------------------- */

static void case_terminal(void) {
    Bench *b = bench_open(c_part, HALF_18432);
    uint16_t frames[4] = {0};
    SimTime bt;

    case_begin();
    program(b, 1, 0x03);   /* 8N1, divisor 1: 115200 baud on 1.8432 MHz */
    bus_write(b, 2, 0x01);
    bt = bit_time(b, 1);
    bus_write(b, 0, 'H');
    bus_write(b, 0, 'i');
    run(b, bt * 24);
    check(sent(b, frames, 4) == 2 && frames[0] == 0x48 && frames[1] == 0x69,
          "sent log: 0x48 0x69");
    set(b, "SIN", LOGIC_Z);   /* nothing else drives RX */
    run(b, NS(100));
    check(sb_send(b, "OK") == 1, "send queued");
    run(b, bt * 24);
    expect_reg(b, 0, "01001111", "RBR reads 0x4F");
    expect_reg(b, 0, "01001011", "then 0x4B");
    case_end("C outcome: 8N1 at 115200 sends H i; typed OK arrives as 0x4F 0x4B");
    bench_close(b);
}
/* D24: C's cases on the shipped PC16550D device object. */
static void case_d24(void) {
    int before = failures;

    c_part = "SFX-PC16550D";
    printf("-- C's cases on SFX-PC16550D:\n");
    case_reset();
    case_divisor();
    case_tx();
    case_rx();
    case_fifo();
    case_timeout();
    case_priority();
    case_modem();
    case_loopback();
    case_dma();
    case_bus();
    case_terminal();
    c_part = "SFX-16550";
    case_begin();
    case_failures = failures - before;
    case_end("D24 PC16550D: the C cases on the shipped device object");
}
/* PartSerial.host: the host-side interface the drawer shows, from "bus". */
static void case_host(void) {
    static const struct { const char *part, *host; } CASES[] = {
        {"SC16IS750IBS,128", "i2c-spi"}, {"MAX3100CEE+", "spi"}, {"SC18IS604PWJ", "spi"},
        {"TL16C550CFN", "parallel"}, {"AY-5-1013A", "parallel"},
    };
    size_t i;

    for (i = 0; i < sizeof(CASES) / sizeof(CASES[0]); i++) {
        const Part *part = part_library_find(library, CASES[i].part);

        check(part != NULL && part->serial != NULL && part->serial->host != NULL &&
              strcmp(part->serial->host, CASES[i].host) == 0, CASES[i].part);
    }
}

int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_reset();
    case_divisor();
    case_tx();
    case_rx();
    case_fifo();
    case_timeout();
    case_priority();
    case_modem();
    case_loopback();
    case_dma();
    case_bus();
    case_16450();
    case_terminal();
    case_d24();
    case_host();
    return serial_bench_finish();
}
