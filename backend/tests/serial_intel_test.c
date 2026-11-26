/*
 * serial_intel_test: the 8251A and 82510 (H4, H5)
 * (split from serial_test.c by plan-68-G; the bench is support/serial_bench.c).
 */

#include "serial_bench.h"
#include "serial_bench_classic.h"

/* ---- H4: 8251A (i8251) --------------------------------------------------------------- */

static const char *const I8_D[8] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"};

/* TxC = RxC = 1 MHz; CLK 3.125 MHz. */
#define I8_HALF NS(500)
#define I8_CLK_HALF NS(160)

static void i8_write(Bench *b, int cd, unsigned v) {
    set(b, "CD", cd ? LOGIC_1 : LOGIC_0);
    h_set_bus(b, I8_D, 8, 1, v);
    set(b, "CS", LOGIC_0);
    run(b, NS(50));
    set(b, "WR", LOGIC_0);
    run(b, NS(300));
    set(b, "WR", LOGIC_1);
    run(b, NS(50));
    set(b, "CS", LOGIC_1);
    h_set_bus(b, I8_D, 8, 0, 0);
    run(b, NS(3000));   /* recovery between writes */
}

static unsigned i8_read(Bench *b, int cd, char *text) {
    unsigned v;

    set(b, "CD", cd ? LOGIC_1 : LOGIC_0);
    set(b, "CS", LOGIC_0);
    run(b, NS(50));
    set(b, "RD", LOGIC_0);
    run(b, NS(300));
    v = h_get_bus(b, I8_D, 8, text);
    set(b, "RD", LOGIC_1);
    run(b, NS(50));
    set(b, "CS", LOGIC_1);
    run(b, NS(500));   /* past tclr */
    return v;
}

static void i8_expect(Bench *b, int cd, const char *want, const char *what) {
    char text[16];

    i8_read(b, cd, text);
    h_expect_text(text, want, what);
}

static HBench *i8_open(void) {
    static const HClockPin CLOCKS[3] = {{"TXC", 0}, {"RXC", 0}, {"CLK", 1}};
    static const SimTime HALVES[2] = {I8_HALF, I8_CLK_HALF};
    HBench *h = hb_open("SFX-8251A-dip28", CLOCKS, 3, HALVES, 2);
    Bench *b = &h->b;

    set(b, "RESET", LOGIC_1);
    set(b, "CS", LOGIC_1);
    set(b, "RD", LOGIC_1);
    set(b, "WR", LOGIC_1);
    set(b, "CD", LOGIC_0);
    set(b, "RXD", LOGIC_1);
    set(b, "CTS", LOGIC_0);
    set(b, "DSR", LOGIC_1);
    set(b, "SYNDET", LOGIC_Z);
    run(b, NS(5000));
    set(b, "RESET", LOGIC_0);
    run(b, NS(2000));
    return h;
}

/* Moves to just after a TxC/RxC falling edge (bits change there; RxC
   rising samples mid-bit). */
static void i8_align(Bench *b) {
    SimTime period = 2 * I8_HALF;
    SimTime now = sim_now(b->sim);
    SimTime wait = (period - (now + period - NS(10)) % period) % period;

    if (wait > 0) {
        run(b, wait);
    }
}

/* Waits to the next character boundary of n-bit characters after a stream
   that ended on one at time t_b. */
static void i8_to_boundary(Bench *b, SimTime t_b, unsigned n) {
    SimTime span = (SimTime)n * 2 * I8_HALF;
    SimTime gone = (sim_now(b->sim) - t_b) % span;

    if (gone != 0) {
        run(b, span - gone);
    }
}

/* Drives bits ('0'/'1') onto RXD, one per RxC period. */
static void i8_sync_bits(Bench *b, const char *bits) {
    size_t i;

    i8_align(b);
    for (i = 0; bits[i] != '\0'; i++) {
        set(b, "RXD", bits[i] == '1' ? LOGIC_1 : LOGIC_0);
        run(b, 2 * I8_HALF);
    }
}

/* The 8 bits of v LSB first. */
static void i8_byte_bits(unsigned v, unsigned n, char *out) {
    unsigned i;

    for (i = 0; i < n; i++) {
        out[i] = (v >> i) & 1u ? '1' : '0';
    }
    out[n] = '\0';
}

static void case_h4_i8251(void) {
    HBench *h = i8_open();
    Bench *b = &h->b;
    const SimTime bt16 = 16 * 2 * I8_HALF;
    const SimTime bt64 = 64 * 2 * I8_HALF;
    const SimTime bt1 = 2 * I8_HALF;
    char got[64];
    char want[64];
    unsigned i;
    SimTime t_b;

    case_begin();
    /* After RESET: idle, mode format next. */
    expect_pin(b, "TXD", LOGIC_1, "TxD marking after RESET");
    expect_pin(b, "TXRDY", LOGIC_0, "TxRDY low (TxEN 0)");
    expect_pin(b, "RXRDY", LOGIC_0, "RxRDY low");
    expect_pin(b, "TXEMPTY", LOGIC_1, "TxEMPTY high");
    expect_pin(b, "SYNDET", LOGIC_0, "SYNDET output low");
    expect_pin(b, "DTR", LOGIC_1, "DTR high");
    expect_pin(b, "RTS", LOGIC_1, "RTS high");
    i8_expect(b, 1, "00000101", "status after RESET (TxRDY, TxEMPTY)");

    /* Async 16x 8N1, then the command: TxEN DTR RxE ER RTS. */
    i8_write(b, 1, 0x4E);
    i8_write(b, 1, 0x37);
    expect_pin(b, "DTR", LOGIC_0, "DTR command bit");
    expect_pin(b, "RTS", LOGIC_0, "RTS command bit");
    expect_pin(b, "TXRDY", LOGIC_1, "TxRDY pin: empty, CTS low, TxEN");
    check(!h_send(b, 0, "") || 1, "");
    i8_write(b, 0, 'H');
    check(h_catch(b, "TXD", bt16, 9, 4 * bt16, got), "16x frame");
    h_frame_bits('H', 8, 0, 1, want);
    h_expect_text(got, want, "16x 8N1 H on TxD");
    expect_pin(b, "TXEMPTY", LOGIC_0, "TxEMPTY low while sending");
    run(b, bt16);
    expect_pin(b, "TXEMPTY", LOGIC_1, "TxEMPTY high when done");
    h_expect_sent(b, 0, 'H', "H logged");
    /* Receive 16x. */
    h_frame_bits(0x5A, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt16);
    run(b, bt16);
    expect_pin(b, "RXRDY", LOGIC_1, "RxRDY after a character");
    i8_expect(b, 1, "00000111", "status: RxRDY");
    i8_expect(b, 0, "01011010", "data 0x5A");
    expect_pin(b, "RXRDY", LOGIC_0, "the data read resets RxRDY");
    /* Overrun, then framing error; ER clears both. */
    h_drive_bits(b, "RXD", want, bt16);
    run(b, bt16);
    h_drive_bits(b, "RXD", want, bt16);
    run(b, bt16);
    i8_expect(b, 1, "00010111", "status: OE");
    h_frame_bits(0x0F, 8, 0, 0, want);
    strcat(want, "0");
    h_drive_bits(b, "RXD", want, bt16);
    run(b, 2 * bt16);
    i8_expect(b, 1, "00110111", "status: OE and FE");
    i8_write(b, 1, 0x37);
    i8_expect(b, 1, "00000111", "ER resets OE and FE");
    i8_read(b, 0, NULL);
    /* CTS high: the TxRDY pin goes low, the status bit stays. */
    set(b, "CTS", LOGIC_1);
    run(b, NS(500));
    expect_pin(b, "TXRDY", LOGIC_0, "TxRDY pin needs CTS low");
    i8_expect(b, 1, "00000101", "TxRDY status bit is the buffer");
    set(b, "CTS", LOGIC_0);
    /* DSR low: status bit 7. */
    set(b, "DSR", LOGIC_0);
    run(b, NS(500));
    i8_expect(b, 1, "10000101", "DSR status bit");
    set(b, "DSR", LOGIC_1);
    /* SBRK: TxD low. */
    i8_write(b, 1, 0x3F);
    run(b, bt16);
    expect_pin(b, "TXD", LOGIC_0, "SBRK forces TxD low");
    i8_write(b, 1, 0x37);
    run(b, bt16);
    expect_pin(b, "TXD", LOGIC_1, "SBRK off");
    /* TxEN off: TxRDY pin low and nothing is sent. */
    i8_write(b, 1, 0x36);
    expect_pin(b, "TXRDY", LOGIC_0, "TxEN off: TxRDY pin low");
    i8_write(b, 0, 0x11);
    check(!h_catch(b, "TXD", bt16, 9, 4 * bt16, got), "TxEN off: nothing sent");
    expect_pin(b, "TXEMPTY", LOGIC_1, "TxEMPTY stays high while disabled");
    i8_write(b, 1, 0x37);
    check(h_catch(b, "TXD", bt16, 9, 4 * bt16, got), "TxEN on: the waiting byte goes");
    run(b, 2 * bt16);
    /* RxE off: RxRDY held reset. */
    i8_write(b, 1, 0x33);
    h_frame_bits(0x22, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt16);
    run(b, bt16);
    expect_pin(b, "RXRDY", LOGIC_0, "RxE off: RxRDY held reset");
    i8_write(b, 1, 0x37);
    /* Break detect: RxD low for two characters. */
    set(b, "RXD", LOGIC_0);
    run(b, 22 * bt16);
    expect_pin(b, "SYNDET", LOGIC_1, "BRKDET after two characters of low");
    i8_expect(b, 1, "01100111", "status: BRKDET (with the break's FE)");
    set(b, "RXD", LOGIC_1);
    run(b, bt16);
    expect_pin(b, "SYNDET", LOGIC_0, "BRKDET reset by RxD high");
    i8_write(b, 1, 0x37);
    i8_read(b, 0, NULL);

    /* Parity (7E2) and the parity error. */
    i8_write(b, 1, 0x40);
    i8_write(b, 1, 0xFA);   /* 2 stops, even, parity on, 7 bits, 16x */
    i8_write(b, 1, 0x37);
    i8_write(b, 0, 0x43);
    check(h_catch(b, "TXD", bt16, 10, 4 * bt16, got), "7E2 frame");
    h_frame_bits(0x43, 7, 'e', 2, want);
    h_expect_text(got, want, "7E2 0x43");
    run(b, 2 * bt16);
    h_frame_bits(0x43, 7, 'o', 1, want);
    h_drive_bits(b, "RXD", want, bt16);
    run(b, bt16);
    i8_expect(b, 1, "00001111", "status: PE");
    i8_expect(b, 0, "01000011", "7-bit data, unused bit 0");

    /* 64x. */
    i8_write(b, 1, 0x40);
    i8_write(b, 1, 0x4F);
    i8_write(b, 1, 0x37);
    i8_write(b, 0, 0x96);
    check(h_catch(b, "TXD", bt64, 9, 4 * bt64, got), "64x frame");
    h_frame_bits(0x96, 8, 0, 1, want);
    h_expect_text(got, want, "64x 8N1");
    run(b, bt64);
    h_drive_bits(b, "RXD", want, bt64);
    run(b, bt64);
    i8_expect(b, 0, "10010110", "64x receive");

    /* 1x: one bit per TxC / RxC. */
    i8_write(b, 1, 0x40);
    i8_write(b, 1, 0x4D);
    i8_write(b, 1, 0x37);
    set(b, "CTS", LOGIC_1);   /* hold the byte until the catch is ready */
    run(b, NS(500));
    i8_write(b, 0, 0x69);
    set(b, "CTS", LOGIC_0);
    check(h_catch(b, "TXD", bt1, 9, 16 * bt1, got), "1x frame");
    h_frame_bits(0x69, 8, 0, 1, want);
    h_expect_text(got, want, "1x 8N1");
    run(b, 4 * bt1);
    i8_align(b);
    h_drive_bits(b, "RXD", want, bt1);
    run(b, 4 * bt1);
    i8_expect(b, 0, "01101001", "1x receive");

    /* Terminal (async 16x). */
    i8_write(b, 1, 0x40);
    i8_write(b, 1, 0x4E);
    i8_write(b, 1, 0x37);
    set(b, "RXD", LOGIC_Z);
    run(b, NS(100));
    check(h_send(b, 0, "OK") == 1, "terminal send accepted (async)");
    h_wait(b, "RXRDY", LOGIC_1, 12 * bt16);
    i8_expect(b, 0, "01001111", "terminal O");
    h_wait(b, "RXRDY", LOGIC_1, 12 * bt16);
    i8_expect(b, 0, "01001011", "terminal K");
    i8_write(b, 0, 'A');
    run(b, 12 * bt16);
    h_expect_sent(b, 0, 'A', "transmitted A logged");
    set(b, "RXD", LOGIC_1);

    /* IR: back to the mode format; sync, 8 bits, single SYNC 0x16. */
    i8_write(b, 1, 0x40);
    expect_pin(b, "DTR", LOGIC_1, "IR: DTR inactive");
    i8_write(b, 1, 0x8C);
    i8_write(b, 1, 0x16);
    check(h_send(b, 0, "x") == 0, "terminal refused in sync mode");
    i8_write(b, 1, 0xB7);   /* EH RTS ER RxE DTR TxEN */
    i8_expect(b, 0, "11111111", "EH sets the receive buffer to ones");
    expect_pin(b, "SYNDET", LOGIC_0, "hunting");
    i8_byte_bits(0x16, 8, want);
    strcpy(got, "1101");
    strcat(got, want);
    i8_byte_bits(0x41, 8, want);
    strcat(got, want);
    i8_sync_bits(b, got);
    t_b = sim_now(b->sim);
    expect_pin(b, "SYNDET", LOGIC_1, "SYNDET after SYNC1 (bit-by-bit hunt)");
    expect_pin(b, "RXRDY", LOGIC_1, "the character after the sync");
    i8_expect(b, 1, "01000111", "status: SYNDET, RxRDY (TxEMPTY)");
    expect_pin(b, "SYNDET", LOGIC_0, "a status read resets SYNDET");
    i8_expect(b, 0, "01000001", "sync data 0x41");
    /* Out of hunt: a sync character at a boundary sets SYNDET again. */
    i8_to_boundary(b, t_b, 8);
    i8_byte_bits(0x16, 8, want);
    i8_sync_bits(b, want);
    expect_pin(b, "SYNDET", LOGIC_1, "SYNDET at a later SYNC1 boundary");
    i8_read(b, 1, NULL);
    i8_read(b, 0, NULL);

    /* Sync TX: a data character, then SYNC fill with TxEMPTY high. */
    set(b, "CTS", LOGIC_1);
    run(b, NS(500));
    i8_write(b, 0, 0x55);
    set(b, "CTS", LOGIC_0);
    for (i = 0; i < 40; i++) {
        got[i] = "01XZ"[get(b, "TXD")];
        run(b, 2 * I8_HALF);
    }
    got[40] = '\0';
    i8_byte_bits(0x55, 8, want);
    check(strstr(got, want) != NULL, "sync TX: the data character on TxD");
    i8_byte_bits(0x16, 8, want);
    {
        char two[24];

        strcpy(two, want);
        strcat(two, want);
        check(strstr(got, two) != NULL, "sync TX: SYNC fill when the CPU is late");
    }
    expect_pin(b, "TXEMPTY", LOGIC_1, "TxEMPTY high during the fill");

    /* Double sync with parity: SYNC1 then SYNC2 contiguous. */
    i8_write(b, 1, 0x40);
    i8_write(b, 1, 0x3C);   /* sync, even parity on, 8 bits, double */
    i8_write(b, 1, 0x16);
    i8_write(b, 1, 0x27);
    i8_write(b, 1, 0xB7);
    {
        char p1[16];
        char p2[16];

        i8_byte_bits(0x16, 8, p1);
        strcat(p1, "1");   /* 0x16 has three ones: even parity bit 1 */
        i8_byte_bits(0x27, 8, p2);
        strcat(p2, "0");   /* 0x27 has four ones */
        strcpy(got, "011");
        strcat(got, p1);
        strcat(got, "0000");
        i8_sync_bits(b, got);
        expect_pin(b, "SYNDET", LOGIC_0, "SYNC1 alone is not sync (double)");
        strcpy(got, p1);
        strcat(got, p2);
        i8_sync_bits(b, got);
        expect_pin(b, "SYNDET", LOGIC_1, "SYNC1 then SYNC2: sync");
        i8_byte_bits(0x31, 8, p1);
        strcat(p1, "0");   /* wrong: 0x31 has three ones, even parity bit is 1 */
        i8_sync_bits(b, p1);
        i8_expect(b, 1, "01001111", "status: SYNDET, PE, RxRDY");
        i8_expect(b, 0, "00110001", "data after the sync pair");
    }

    /* External sync: SYNDET is an input; its rising edge ends the hunt. */
    i8_write(b, 1, 0x40);
    i8_write(b, 1, 0xCC);   /* sync, external, single, 8 bits */
    i8_write(b, 1, 0x11);
    i8_write(b, 1, 0xB7);
    expect_pin(b, "SYNDET", LOGIC_Z, "external sync: SYNDET is an input");
    i8_align(b);
    set(b, "SYNDET", LOGIC_1);   /* assembly starts at the next RxC rising edge */
    i8_byte_bits(0xC3, 8, want);
    i8_sync_bits(b, want);
    set(b, "SYNDET", LOGIC_0);
    expect_pin(b, "RXRDY", LOGIC_1, "external sync: assembling after SYNDET");
    i8_expect(b, 0, "11000011", "external sync data");
    set(b, "SYNDET", LOGIC_Z);
    case_end("H4 8251A: async 1x/16x/64x, every command bit, errors, break; sync hunt, SYNDET, fill, double sync, external sync; terminal");
    hb_close(h);
}

/* ---- H5: 82510 (i82510) ----------------------------------------------------------------- */

static const char *const I5_D[8] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"};

/* CLK 18.432 MHz (external clock, /2: system clock 9.216 MHz). */
#define I5_HALF ((SimTime)27127)
#define I5_SYS (4 * I5_HALF)

static void i5_addr(Bench *b, unsigned a) {
    set(b, "A0", a & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "A1", a & 2u ? LOGIC_1 : LOGIC_0);
    set(b, "A2", a & 4u ? LOGIC_1 : LOGIC_0);
}

static void i5_write(Bench *b, unsigned a, unsigned v) {
    i5_addr(b, a);
    h_set_bus(b, I5_D, 8, 1, v);
    set(b, "CS", LOGIC_0);
    run(b, NS(20));
    set(b, "WR", LOGIC_0);
    run(b, NS(300));
    set(b, "WR", LOGIC_1);
    run(b, NS(20));
    set(b, "CS", LOGIC_1);
    h_set_bus(b, I5_D, 8, 0, 0);
    run(b, NS(300));
}

static unsigned i5_read(Bench *b, unsigned a, char *text) {
    unsigned v;

    i5_addr(b, a);
    set(b, "CS", LOGIC_0);
    run(b, NS(20));
    set(b, "RD", LOGIC_0);
    run(b, NS(320));
    v = h_get_bus(b, I5_D, 8, text);
    set(b, "RD", LOGIC_1);
    run(b, NS(20));
    set(b, "CS", LOGIC_1);
    run(b, NS(300));
    return v;
}

static void i5_expect(Bench *b, unsigned a, const char *want, const char *what) {
    char text[16];

    i5_read(b, a, text);
    h_expect_text(text, want, what);
}

static void i5_bank(Bench *b, unsigned bank) {
    i5_write(b, 2, bank << 5);
}

static HBench *i5_open(void) {
    static const HClockPin CLOCKS[1] = {{"CLK/X1", 0}};
    SimTime half = I5_HALF;
    HBench *h = hb_open("SFX-82510-plcc28", CLOCKS, 1, &half, 1);
    Bench *b = &h->b;

    set(b, "RESET", LOGIC_1);
    set(b, "CS", LOGIC_1);
    set(b, "RD", LOGIC_1);
    set(b, "WR", LOGIC_1);
    set(b, "RXD", LOGIC_1);
    set(b, "CTS", LOGIC_1);
    set(b, "RI/SCLK", LOGIC_1);
    set(b, "DSR/TA/OUT0", LOGIC_1);
    set(b, "DCD/ICLK/OUT1", LOGIC_1);
    i5_addr(b, 0);
    run(b, NS(3000));
    set(b, "RESET", LOGIC_0);
    run(b, NS(1000));
    return h;
}

static void case_h5_i82510(void) {
    HBench *h = i5_open();
    Bench *b = &h->b;
    /* Wake-up path: 9.216 MHz / 2 / 5 = 921.6 kHz 16x: 57600 baud. */
    const SimTime bt = 160 * I5_SYS;
    char got[32];
    char want[32];
    char text[16];
    unsigned i;

    case_begin();
    /* Every bank's wake-up values (Table 4). */
    i5_expect(b, 1, "00000000", "bank 0 GER");
    i5_expect(b, 2, "00000001", "GIR/BANK");
    i5_expect(b, 3, "00000000", "LCR");
    i5_expect(b, 4, "00000000", "MCR");
    i5_expect(b, 5, "01100000", "LSR");
    i5_expect(b, 6, "00000000", "MSR (inputs inactive)");
    i5_expect(b, 7, "00000000", "ACR0");
    i5_write(b, 3, 0x80);
    i5_expect(b, 0, "00000010", "BAL");
    i5_expect(b, 1, "00000000", "BAH");
    i5_write(b, 3, 0x00);
    i5_bank(b, 1);
    i5_expect(b, 2, "00100001", "GIR with bank 1");
    i5_expect(b, 3, "00110000", "TMST");
    i5_expect(b, 4, "00000000", "FLR");
    i5_expect(b, 5, "00000000", "RST");
    i5_expect(b, 7, "00010010", "GSR (TX idle, TX FIFO at threshold)");
    i5_bank(b, 2);
    i5_expect(b, 0, "XXXXXXXX", "bank 2 A0 is not a register: X");
    i5_expect(b, 1, "00000000", "FMD");
    i5_expect(b, 3, "00000000", "TMD");
    i5_expect(b, 4, "00001100", "IMD");
    i5_expect(b, 5, "00000000", "ACR1");
    i5_expect(b, 6, "00011110", "RIE");
    i5_expect(b, 7, "00000000", "RMD");
    i5_bank(b, 3);
    i5_expect(b, 0, "00000000", "CLCF");
    i5_expect(b, 1, "00000100", "BACF");
    i5_expect(b, 3, "10000100", "BBCF");
    i5_expect(b, 4, "11111100", "PMD");
    i5_expect(b, 5, "00001111", "MIE");
    i5_expect(b, 6, "00000000", "TMIE");
    i5_expect(b, 7, "XXXXXXXX", "bank 3 A7 is not a register: X");
    i5_bank(b, 0);
    i5_write(b, 3, 0x80);   /* DLAB (bank 0 LCR) */
    i5_bank(b, 3);
    i5_expect(b, 0, "00000101", "BBL");
    i5_expect(b, 1, "00000000", "BBH");
    i5_bank(b, 0);
    i5_write(b, 3, 0x00);

    /* 8N1 through the 8250 path (BRGA / 2 -> BRGB / 5). */
    i5_bank(b, 0);
    i5_write(b, 3, 0x03);
    i5_write(b, 0, 'A');
    check(h_catch(b, "TXD", bt, 9, 4 * bt, got), "frame");
    h_frame_bits('A', 8, 0, 1, want);
    h_expect_text(got, want, "57600 8N1 through the cascaded BRGs");
    run(b, bt);
    h_expect_sent(b, 0, 'A', "A logged");
    /* TX from BRGA directly (CLCF TxCS = 1): 9.216 MHz / 2 / 16. */
    i5_bank(b, 3);
    i5_write(b, 0, 0x10);
    i5_bank(b, 0);
    i5_write(b, 0, 0x5A);
    check(h_catch(b, "TXD", 32 * I5_SYS, 9, 40 * 32 * I5_SYS, got), "BRGA frame");
    h_frame_bits(0x5A, 8, 0, 1, want);
    h_expect_text(got, want, "TX from BRGA: 32 system clocks a bit");
    run(b, 64 * I5_SYS);
    i5_bank(b, 3);
    i5_write(b, 0, 0x00);
    /* BRGB divisor 10 through DLAB: half the rate. */
    i5_bank(b, 0);
    i5_write(b, 3, 0x83);
    i5_bank(b, 3);
    i5_write(b, 0, 0x0A);
    i5_bank(b, 0);
    i5_write(b, 3, 0x03);
    i5_write(b, 0, 0xC3);
    check(h_catch(b, "TXD", 2 * bt, 9, 8 * bt, got), "BBL 10 frame");
    h_frame_bits(0xC3, 8, 0, 1, want);
    h_expect_text(got, want, "BBL = 10: 28800");
    run(b, 2 * bt);
    i5_write(b, 3, 0x83);
    i5_bank(b, 3);
    i5_write(b, 0, 0x05);
    i5_bank(b, 0);
    i5_write(b, 3, 0x03);

    /* Receive (RX FIFO 1 deep at wake-up): RFIR, RXF, RXD. */
    h_frame_bits(0x3C, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt);
    run(b, bt);
    i5_expect(b, 5, "01100001", "LSR: RFIR");
    i5_bank(b, 1);
    i5_expect(b, 1, "01000000", "RXF: ROK");
    i5_expect(b, 4, "00010000", "FLR: one in the RX FIFO");
    i5_expect(b, 0, "00111100", "RXD");
    i5_expect(b, 4, "00000000", "FLR after the read");
    /* Four-deep FIFO (IMD.RFD = 0); a fifth character: the oldest goes, OE. */
    i5_bank(b, 2);
    i5_write(b, 4, 0x08);
    for (i = 0; i < 5; i++) {
        h_frame_bits(0x30 + i, 8, 0, 1, want);
        h_drive_bits(b, "RXD", want, bt);
    }
    run(b, bt);
    i5_bank(b, 1);
    i5_expect(b, 4, "01000000", "FLR: four in the RX FIFO");
    i5_expect(b, 5, "00000011", "RST: OE, RFIR");
    i5_expect(b, 0, "00110001", "the oldest (0x30) was dropped");
    for (i = 0; i < 3; i++) {
        i5_read(b, 0, NULL);
    }

    /* Interrupts: the RX FIFO block (GER RFIE). */
    i5_bank(b, 0);
    i5_write(b, 1, 0x01);
    h_frame_bits(0x41, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt);
    run(b, bt);
    expect_pin(b, "INT", LOGIC_1, "RX FIFO interrupt");
    i5_expect(b, 2, "00000100", "GIR: RX FIFO (010), pending");
    i5_read(b, 0, NULL);
    run(b, NS(500));
    expect_pin(b, "INT", LOGIC_0, "reading the character services it");
    /* The TX FIFO block and the hold state. */
    i5_write(b, 1, 0x02);
    run(b, NS(500));
    expect_pin(b, "INT", LOGIC_1, "TX FIFO interrupt (0 <= threshold 0)");
    i5_expect(b, 2, "00000010", "GIR: TX FIFO (001)");
    run(b, NS(500));
    expect_pin(b, "INT", LOGIC_0, "the GIR read enters the TX FIFO hold state");
    i5_write(b, 0, 0x11);
    run(b, 12 * bt);
    expect_pin(b, "INT", LOGIC_1, "a write leaves the hold state: the next TX FIFO interrupt");
    i5_write(b, 1, 0x00);
    /* Priorities: timer over modem. A timer (BRGA in timer mode, count 20). */
    i5_bank(b, 3);
    i5_write(b, 0, 0x50);   /* CLCF: TX and RX from BRGB... BRGA becomes the timer */
    i5_write(b, 0, 0x00);
    i5_write(b, 1, 0x00);   /* BACF: BRGA in timer mode */
    i5_write(b, 3, 0x04);   /* BBCF: BRGB from the system clock, BRG mode */
    i5_write(b, 6, 0x01);   /* TMIE TAIE */
    i5_bank(b, 0);
    i5_write(b, 3, 0x83);
    i5_write(b, 0, 20);     /* BAL: the count */
    i5_write(b, 3, 0x03);
    i5_write(b, 1, 0x28);   /* GER: TIE, MIE */
    set(b, "CTS", LOGIC_0);
    run(b, NS(500));
    i5_expect(b, 2, "00000000", "GIR: modem (000) pending");
    i5_bank(b, 1);
    i5_write(b, 3, 0x31);   /* TMCR: gates open, STA */
    run(b, 30 * I5_SYS);
    i5_expect(b, 2, "00101010", "GIR: timer (101) over modem");
    i5_expect(b, 3, "00110001", "TMST: TAEx");
    i5_expect(b, 3, "00110000", "TMST read clears TAEx");
    i5_expect(b, 6, "00010001", "MSR: CTS active, DCTS");
    i5_expect(b, 2, "00100001", "nothing pending");
    i5_bank(b, 3);
    i5_write(b, 1, 0x04);   /* BACF back to BRG mode */
    i5_write(b, 3, 0x84);   /* BBCF: BRGB from BRGA again */
    i5_bank(b, 0);
    i5_write(b, 3, 0x80);
    i5_write(b, 0, 0x02);
    i5_write(b, 3, 0x03);
    i5_write(b, 1, 0x00);

    /* Control characters: ASCII set (RMD 80H) and ACR0. */
    i5_bank(b, 2);
    i5_write(b, 7, 0x80);
    i5_bank(b, 0);
    i5_write(b, 7, 'Z');
    h_frame_bits(0x0D, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt);
    run(b, bt);
    i5_bank(b, 1);
    i5_expect(b, 1, "01001000", "RXF: ROK, ACR (a control character)");
    i5_expect(b, 5, "10000001", "RST: CRF");
    i5_read(b, 0, NULL);
    h_frame_bits('Z', 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt);
    run(b, bt);
    i5_expect(b, 5, "01000001", "RST: PCRF (ACR0)");
    i5_read(b, 0, NULL);
    /* Break: BkD with FE, a null character with BKF; BkT on the mark. The
       null matches ACR1 = 00 (PCRF, as the p41 diagrams' RST 5CH). */
    i5_bank(b, 2);
    i5_write(b, 7, 0x00);
    i5_bank(b, 1);
    set(b, "RXD", LOGIC_0);
    run(b, 14 * bt);
    i5_expect(b, 1, "00001110", "RXF: ACR (ACR1 match), BKF, RFE");
    i5_expect(b, 5, "01011001", "RST: PCRF, BkD, FE");
    set(b, "RXD", LOGIC_1);
    run(b, bt);
    i5_expect(b, 5, "00100001", "RST: BkT");
    i5_read(b, 0, NULL);
    i5_bank(b, 0);

    /* Quarter-bit stops: SBL 010 = 3/4; two characters back to back. */
    i5_bank(b, 2);
    i5_write(b, 3, 0x01);   /* TMD SBL1 = 1 */
    i5_bank(b, 0);
    i5_write(b, 3, 0x03);   /* SBL0 = 0: code 010 */
    i5_write(b, 0, 0xFF);
    i5_write(b, 0, 0xFF);
    {
        SimTime t0;
        SimTime spent = 0;

        h_wait(b, "TXD", LOGIC_0, 4 * bt);
        t0 = sim_now(b->sim);
        run(b, 9 * bt + bt / 4);
        while (get(b, "TXD") != LOGIC_0 && spent < 2 * bt) {
            run(b, NS(100));
            spent += NS(100);
        }
        {
            SimTime d = sim_now(b->sim) - t0;
            SimTime want_d = 9 * bt + 3 * bt / 4;
            char message[96];

            snprintf(message, sizeof message, "3/4 stop: start to start %llu ps, want %llu",
                     (unsigned long long)d, (unsigned long long)want_d);
            check(d + NS(400) >= want_d && d <= want_d + NS(400), message);
        }
    }
    run(b, 12 * bt);
    i5_bank(b, 2);
    i5_write(b, 3, 0x20);   /* NBCL: 9-bit characters */
    i5_bank(b, 1);
    i5_write(b, 1, 0x20);   /* TXF D8 */
    i5_write(b, 0, 0x55);
    check(h_catch(b, "TXD", bt, 10, 4 * bt, got), "9-bit frame");
    h_expect_text(got, "01010101011", "9-bit character: D8 from TXF");
    run(b, 2 * bt);
    /* Software parity: LCR PM0 with TMD SPF; the bit from TXF.SP. */
    i5_bank(b, 2);
    i5_write(b, 3, 0x04);
    i5_bank(b, 0);
    i5_write(b, 3, 0x0B);
    i5_bank(b, 1);
    i5_write(b, 1, 0x40);
    i5_write(b, 0, 0x01);
    check(h_catch(b, "TXD", bt, 10, 4 * bt, got), "software parity frame");
    h_expect_text(got, "01000000011", "software parity bit 1 from TXF.SP");
    run(b, 2 * bt);
    i5_bank(b, 2);
    i5_write(b, 3, 0x00);
    i5_bank(b, 0);
    i5_write(b, 3, 0x03);

    /* Semi-automatic handshake: RTS while enabled, TX only with CTS. */
    i5_bank(b, 2);
    i5_write(b, 3, 0x10);
    run(b, NS(500));
    expect_pin(b, "RTS", LOGIC_0, "semi-automatic: RTS asserted while enabled");
    set(b, "CTS", LOGIC_1);
    run(b, NS(500));
    i5_bank(b, 0);
    i5_write(b, 0, 0x66);
    check(!h_catch(b, "TXD", bt, 9, 3 * bt, got), "semi-automatic: CTS inactive holds TX");
    set(b, "CTS", LOGIC_0);
    check(h_catch(b, "TXD", bt, 9, 3 * bt, got), "CTS active: sent");
    run(b, 2 * bt);
    i5_bank(b, 2);
    i5_write(b, 3, 0x00);

    /* Local loopback: TXD high, the receiver hears the transmitter. */
    i5_bank(b, 0);
    i5_write(b, 4, 0x10);
    i5_write(b, 0, 0xA5);
    run(b, bt);
    expect_pin(b, "TXD", LOGIC_1, "local loopback: TXD held mark");
    run(b, 11 * bt);
    i5_expect(b, 0, "10100101", "local loopback: received");
    /* Remote loopback (LEM): RXD straight to TXD. */
    i5_bank(b, 2);
    i5_write(b, 4, 0x09);
    set(b, "RXD", LOGIC_0);
    run(b, NS(300));
    expect_pin(b, "TXD", LOGIC_0, "remote loopback: TXD follows RXD");
    set(b, "RXD", LOGIC_1);
    run(b, NS(300));
    /* Automatic echo (LEM, LC = 0): a received character is sent back. */
    i5_bank(b, 0);
    i5_write(b, 4, 0x00);
    h_frame_bits(0x4E, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt);
    check(h_catch(b, "TXD", bt, 9, 4 * bt, got), "echo frame");
    h_expect_text(got, want, "automatic echo");
    run(b, 2 * bt);
    i5_bank(b, 2);
    i5_write(b, 4, 0x08);
    i5_bank(b, 0);

    /* PMD: DSR/TA/OUT0 as OUT0 (MCR bit 5). */
    set(b, "DSR/TA/OUT0", LOGIC_Z);
    i5_bank(b, 3);
    i5_write(b, 4, 0xDC);   /* DTAD = 0 (output), DTAF = 1 (OUT0) */
    i5_bank(b, 0);
    i5_write(b, 4, 0x20);
    run(b, NS(300));
    expect_pin(b, "DSR/TA/OUT0", LOGIC_0, "OUT0 from MCR bit 5");
    i5_write(b, 4, 0x00);
    run(b, NS(300));
    expect_pin(b, "DSR/TA/OUT0", LOGIC_1, "OUT0 inactive");
    i5_bank(b, 3);
    i5_write(b, 4, 0xFC);
    i5_bank(b, 0);
    set(b, "DSR/TA/OUT0", LOGIC_1);

    /* Manual acknowledge (IMD.IAM = 0): INT holds until ICM.INTA. */
    i5_bank(b, 2);
    i5_write(b, 4, 0x00);
    i5_bank(b, 0);
    i5_write(b, 1, 0x01);
    h_frame_bits(0x21, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt);
    run(b, bt);
    i5_read(b, 0, NULL);
    run(b, NS(500));
    expect_pin(b, "INT", LOGIC_1, "manual acknowledge: INT holds after the service");
    i5_bank(b, 1);
    i5_write(b, 7, 0x08);
    run(b, NS(1000));
    expect_pin(b, "INT", LOGIC_0, "ICM.INTA releases it");
    /* Power down (external clock: idle); reading FLR exits. */
    i5_write(b, 7, 0x02);
    i5_bank(b, 1);
    i5_write(b, 0, 0x77);
    check(!h_catch(b, "TXD", bt, 9, 3 * bt, got), "power down: the transmitter stops");
    i5_expect(b, 4, "XXXXXXXX", "FLR read leaves power down (data invalid)");
    check(h_catch(b, "TXD", bt, 9, 3 * bt, got), "running again");
    run(b, 2 * bt);
    /* ICM.SRST: the wake-up values again. */
    i5_write(b, 7, 0x10);
    i5_expect(b, 2, "00000001", "SRST: bank 0, nothing pending");
    i5_bank(b, 2);
    i5_expect(b, 4, "00001100", "SRST: IMD wake-up value");
    i5_bank(b, 0);

    /* Terminal. */
    i5_write(b, 3, 0x03);
    set(b, "RXD", LOGIC_Z);
    run(b, NS(100));
    check(h_send(b, 0, "OK") == 1, "terminal send accepted");
    run(b, 11 * bt);
    i5_expect(b, 0, "01001111", "terminal O");
    run(b, 11 * bt);
    i5_expect(b, 0, "01001011", "terminal K");
    (void)text;
    case_end("H5 82510: every bank's registers, both BRGs and a timer, FIFOs, interrupt blocks and priority, hold state, control characters, break, stops, 9-bit, soft parity, handshake, loopbacks, echo, PMD, ack modes, power down, terminal");
    hb_close(h);
}


int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_h4_i8251();
    case_h5_i82510();
    return serial_bench_finish();
}
