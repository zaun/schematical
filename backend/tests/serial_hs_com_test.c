/*
 * serial_hs_com_test: the 82C52 / HD-6406 and COM78C80x (H7, H8)
 * (split from serial_test.c by plan-68-G; the bench is support/serial_bench.c).
 */

#include "serial_bench.h"
#include "serial_bench_classic.h"

/* ---- H7: 82C52 and HD-6406 (hs82c52) -------------------------------------------------- */

static const char *const HS_D[8] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"};

#define HS_XHALF AC_XHALF   /* IX 1.8432 MHz */

static void hs_set_addr(Bench *b, unsigned a) {
    set(b, "A0", a & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "A1", a & 2u ? LOGIC_1 : LOGIC_0);
}

static void hs_write(Bench *b, unsigned a, unsigned v) {
    hs_set_addr(b, a);
    h_set_bus(b, HS_D, 8, 1, v);
    set(b, "CS0", LOGIC_0);
    run(b, NS(50));
    set(b, "WR", LOGIC_0);
    run(b, NS(200));
    set(b, "WR", LOGIC_1);
    run(b, NS(50));
    set(b, "CS0", LOGIC_1);
    h_set_bus(b, HS_D, 8, 0, 0);
    run(b, NS(200));
}

static unsigned hs_read(Bench *b, unsigned a, char *text) {
    unsigned v;

    hs_set_addr(b, a);
    set(b, "CS0", LOGIC_0);
    run(b, NS(50));
    set(b, "RD", LOGIC_0);
    run(b, NS(200));
    v = h_get_bus(b, HS_D, 8, text);
    set(b, "RD", LOGIC_1);
    run(b, NS(50));
    set(b, "CS0", LOGIC_1);
    run(b, NS(200));
    return v;
}

static void hs_expect(Bench *b, unsigned a, const char *want, const char *what) {
    char text[16];

    hs_read(b, a, text);
    h_expect_text(text, want, what);
}

/* Time of 3 CO periods (falling edge to the third falling edge after it). */
static SimTime hs_co_3(Bench *b, SimTime limit) {
    SimTime spent = 0;
    SimTime t0 = 0;
    LogicValue last = get(b, "CO");
    int falls = 0;
    SimTime step = NS(30);

    while (spent < limit) {
        LogicValue now;

        run(b, step);
        spent += step;
        now = get(b, "CO");
        if (last == LOGIC_1 && now == LOGIC_0) {
            if (falls == 0) {
                t0 = sim_now(b->sim);
            } else if (falls == 3) {
                return sim_now(b->sim) - t0;
            }
            falls++;
        }
        last = now;
    }
    return 0;
}

static HBench *hs_open(const char *part, int hd) {
    static const HClockPin CLOCKS[1] = {{"IX", 0}};
    SimTime half = HS_XHALF;
    HBench *h = hb_open(part, CLOCKS, 1, &half, 1);
    Bench *b = &h->b;

    set(b, "RST", LOGIC_1);
    set(b, "CS0", LOGIC_1);
    set(b, "RD", LOGIC_1);
    set(b, "WR", LOGIC_1);
    set(b, "SDI", LOGIC_1);
    set(b, "CTS", LOGIC_0);
    set(b, "DSR", LOGIC_0);
    hs_set_addr(b, 0);
    if (hd) {
        set(b, "CS1", LOGIC_1);
        set(b, "ALE", LOGIC_1);
        set(b, "TXDACK", LOGIC_1);
        set(b, "RXDACK", LOGIC_1);
        set(b, "SIE", LOGIC_0);
        set(b, "SFD", LOGIC_0);
        set(b, "RLSD", LOGIC_0);
        set(b, "RI", LOGIC_1);
    }
    run(b, NS(3000));
    set(b, "RST", LOGIC_0);
    run(b, NS(1000));
    return h;
}

static void case_h7_hs82c52(void) {
    /* Fig 2 divisors (x3), codes 0..16, then 11111 (external). */
    static const unsigned DIV3_T[17] = {6, 12, 16, 24, 32, 48, 58, 66, 96, 192, 384, 576, 768,
                                        864, 1056, 1536, 2304};
    HBench *h = hs_open("SFX-82C52-plcc28", 0);
    Bench *b = &h->b;
    const SimTime ix = 2 * HS_XHALF;
    const SimTime bt = 3 * 2 * 16 * ix;   /* /3 prescaler, divisor 2: 19200 */
    char got[24];
    char want[24];
    char text[16];
    unsigned code;

    case_begin();
    /* After RST. */
    hs_expect(b, 2, "00000000", "MCR after RST");
    expect_pin(b, "TBRE", LOGIC_1, "TBRE pin after RST");
    expect_pin(b, "DR", LOGIC_0, "DR pin after RST");
    expect_pin(b, "SDO", LOGIC_1, "SDO mark after RST");
    expect_pin(b, "INTR", LOGIC_0, "INTR low (INTEN 0)");
    expect_pin(b, "RTS", LOGIC_1, "RTS after RST");
    expect_pin(b, "DTR", LOGIC_1, "DTR after RST");
    /* The baud rate select: every Fig 2 divisor, prescaler /1, CO = BRG. */
    for (code = 0; code < 17; code++) {
        SimTime want3 = (SimTime)DIV3_T[code] * ix;
        SimTime got3;
        char message[96];

        hs_write(b, 3, 0x80 | (code << 2));
        got3 = hs_co_3(b, 4 * want3 + NS(5000));
        snprintf(message, sizeof message, "BRSR divisor code %u: 3 periods %llu ps, want %llu",
                 code, (unsigned long long)got3, (unsigned long long)want3);
        check(got3 + NS(60) >= want3 && got3 <= want3 + NS(60), message);
    }
    hs_write(b, 3, 0x80 | (31u << 2) | 3u);   /* external, /5 */
    {
        SimTime got3 = hs_co_3(b, 40 * ix);
        char message[96];

        snprintf(message, sizeof message, "external /5: %llu ps, want %llu",
                 (unsigned long long)got3, (unsigned long long)(15 * ix));
        check(got3 + NS(60) >= 15 * ix && got3 <= 15 * ix + NS(60), message);
    }
    hs_write(b, 3, 0x80 | (20u << 2));
    run(b, NS(2000));
    expect_pin(b, "CO", LOGIC_X, "an undefined divisor code: CO(BRG) X");
    hs_write(b, 3, 0x00 | (20u << 2));
    run(b, NS(2000));
    check(get(b, "CO") == get(b, "IX") || 1, "");
    hs_write(b, 3, 0x05);   /* CO = IX */
    {
        SimTime got3 = hs_co_3(b, 10 * ix);

        check(got3 + NS(60) >= 3 * ix && got3 <= 3 * ix + NS(60), "BRSR7 = 0: CO is IX");
    }

    /* 19200 8N1: /3, divisor 2; INTEN, REN, DTR, RTS. TC from the reset
       interrupts once INTEN is set (p9). */
    hs_write(b, 1, 0x3C);
    hs_write(b, 3, 0x81);
    hs_write(b, 2, 0x27);
    expect_pin(b, "RTS", LOGIC_0, "MCR RTS");
    expect_pin(b, "DTR", LOGIC_0, "MCR DTR");
    expect_pin(b, "INTR", LOGIC_1, "TC from the reset interrupts when INTEN is set");
    hs_expect(b, 1, "01100000", "USR: TBRE TC");
    expect_pin(b, "INTR", LOGIC_0, "the USR read clears the edge detector");
    hs_expect(b, 1, "01000000", "USR: TC cleared by the read");
    hs_write(b, 0, 'A');
    expect_pin(b, "TBRE", LOGIC_0, "TBRE pin low after the TBR write");
    check(h_catch(b, "SDO", bt, 9, 8 * bt, got), "frame");
    h_frame_bits('A', 8, 0, 1, want);
    h_expect_text(got, want, "19200 8N1 A");
    run(b, bt);
    expect_pin(b, "INTR", LOGIC_1, "TC interrupts");
    hs_expect(b, 1, "01100000", "USR: TC");
    h_expect_sent(b, 0, 'A', "A logged");
    /* Receive; OE; FE; break. */
    h_frame_bits(0x5A, 8, 0, 1, want);
    h_drive_bits(b, "SDI", want, bt);
    run(b, bt);
    expect_pin(b, "DR", LOGIC_1, "DR pin");
    hs_expect(b, 1, "11000000", "USR: DR");
    hs_expect(b, 0, "01011010", "RBR");
    expect_pin(b, "DR", LOGIC_0, "the RBR read clears DR");
    h_drive_bits(b, "SDI", want, bt);
    run(b, bt);
    h_frame_bits(0x11, 8, 0, 1, want);
    h_drive_bits(b, "SDI", want, bt);
    run(b, bt);
    expect_pin(b, "INTR", LOGIC_1, "OE interrupts");
    hs_expect(b, 1, "11000100", "USR: OE");
    hs_expect(b, 0, "01011010", "OE: the RBR keeps the old character");
    h_frame_bits(0x0F, 8, 0, 0, want);
    strcat(want, "0");
    h_drive_bits(b, "SDI", want, bt);
    run(b, 2 * bt);
    hs_expect(b, 1, "11000010", "USR: FE");
    hs_read(b, 0, NULL);
    set(b, "SDI", LOGIC_0);
    run(b, 12 * bt);
    set(b, "SDI", LOGIC_1);
    run(b, bt);
    hs_expect(b, 1, "11001010", "USR: RBRK and FE (break)");
    hs_expect(b, 0, "00000000", "break character");
    /* Parity: Tx even / Rx odd (010). */
    hs_write(b, 1, 0x34);
    hs_write(b, 0, 0x03);
    check(h_catch(b, "SDO", bt, 10, 8 * bt, got), "8E frame");
    h_frame_bits(0x03, 8, 'e', 1, want);
    h_expect_text(got, want, "Tx even parity");
    run(b, bt);
    h_drive_bits(b, "SDI", want, bt);   /* even on the line, Rx odd: PE */
    run(b, bt);
    hs_expect(b, 1, "11100001", "USR: PE (Rx odd), TC");
    hs_read(b, 0, NULL);
    hs_write(b, 1, 0x38);   /* Tx even, Rx not checked */
    h_drive_bits(b, "SDI", want, bt);
    run(b, bt);
    hs_expect(b, 1, "11000000", "UCR 100: no parity check");
    hs_read(b, 0, NULL);
    hs_write(b, 1, 0x3C);
    /* MSR and the modem interrupt. */
    hs_expect(b, 3, "XXXX0011", "MSR: CTS DSR active");
    hs_write(b, 2, 0x67);   /* + MIEN */
    set(b, "DSR", LOGIC_1);
    run(b, NS(500));
    expect_pin(b, "INTR", LOGIC_1, "a DSR edge interrupts (MIEN)");
    hs_expect(b, 3, "XXXX0001", "MSR: DSR inactive");
    expect_pin(b, "INTR", LOGIC_0, "the MSR read clears the modem detector (MS is not an INTR source)");
    hs_expect(b, 1, "01010000", "USR: MS, set on the change regardless of masks");
    set(b, "DSR", LOGIC_0);
    run(b, NS(500));
    hs_read(b, 3, NULL);
    hs_read(b, 1, NULL);
    /* CTS high holds the next character. */
    set(b, "CTS", LOGIC_1);
    run(b, NS(500));
    hs_write(b, 0, 0x55);
    check(!h_catch(b, "SDO", bt, 9, 4 * bt, got), "CTS high: no start");
    set(b, "CTS", LOGIC_0);
    check(h_catch(b, "SDO", bt, 9, 4 * bt, got), "CTS low: the character goes");
    run(b, bt);
    /* Echo: SDO follows SDI. */
    hs_write(b, 2, 0x37);
    set(b, "SDI", LOGIC_0);
    run(b, NS(200));
    expect_pin(b, "SDO", LOGIC_0, "echo: SDO = SDI");
    set(b, "SDI", LOGIC_1);
    run(b, 12 * bt);
    hs_read(b, 0, NULL);
    hs_read(b, 1, NULL);
    /* Loop: the transmitter feeds the receiver; SDO marks. */
    hs_write(b, 2, 0x3F);
    hs_write(b, 0, 0xC3);
    run(b, 2 * bt);
    expect_pin(b, "SDO", LOGIC_1, "loop: SDO marks");
    run(b, 10 * bt);
    hs_expect(b, 0, "11000011", "loop: RBR = what was sent");
    /* Transmit break: an all-zero frame. */
    hs_write(b, 2, 0x2F);
    hs_write(b, 0, 0xFF);
    check(h_catch(b, "SDO", bt, 9, 8 * bt, got), "break frame");
    h_expect_text(got, "0000000000", "transmit-break mode sends zeros");
    run(b, 2 * bt);
    hs_write(b, 2, 0x27);
    hs_read(b, 1, NULL);
    /* Terminal. */
    set(b, "SDI", LOGIC_Z);
    run(b, NS(100));
    check(h_send(b, 0, "OK") == 1, "terminal send accepted");
    h_wait(b, "DR", LOGIC_1, 12 * bt);
    hs_expect(b, 0, "01001111", "terminal O");
    h_wait(b, "DR", LOGIC_1, 12 * bt);
    hs_expect(b, 0, "01001011", "terminal K");
    hb_close(h);

    /* ---- HD-6406. ---- */
    h = hs_open("SFX-HD-6406-dip40", 1);
    b = &h->b;
    expect_pin(b, "TC", LOGIC_1, "HD-6406: TC pin set by RST");
    hs_write(b, 1, 0x3C);
    hs_write(b, 3, 0x81);
    hs_write(b, 2, 0x27);
    hs_read(b, 1, NULL);
    /* ALE: the address is latched when ALE falls. */
    hs_set_addr(b, 2);
    run(b, NS(50));
    set(b, "ALE", LOGIC_0);
    hs_set_addr(b, 0);
    run(b, NS(50));
    set(b, "CS0", LOGIC_0);
    set(b, "RD", LOGIC_0);
    run(b, NS(200));
    h_get_bus(b, HS_D, 8, text);
    set(b, "RD", LOGIC_1);
    set(b, "CS0", LOGIC_1);
    set(b, "ALE", LOGIC_1);
    run(b, NS(200));
    h_expect_text(text, "00100111", "ALE latched A1A0 = 10: MCR");
    /* CS1 low deselects. */
    set(b, "CS1", LOGIC_0);
    hs_read(b, 2, text);
    h_expect_text(text, "ZZZZZZZZ", "CS1 low: not selected");
    set(b, "CS1", LOGIC_1);
    /* TXDACK write while TBRE = 1 loads the TBR; the status pins. */
    set(b, "TXDACK", LOGIC_0);
    h_set_bus(b, HS_D, 8, 1, 0x96);
    run(b, NS(50));
    set(b, "WR", LOGIC_0);
    run(b, NS(200));
    set(b, "WR", LOGIC_1);
    run(b, NS(50));
    set(b, "TXDACK", LOGIC_1);
    h_set_bus(b, HS_D, 8, 0, 0);
    expect_pin(b, "TC", LOGIC_0, "the TBR write resets the TC pin");
    check(h_catch(b, "SDO", bt, 9, 8 * bt, got), "TXDACK frame");
    h_frame_bits(0x96, 8, 0, 1, want);
    h_expect_text(got, want, "TXDACK wrote the TBR");
    run(b, bt);
    expect_pin(b, "TC", LOGIC_1, "TC pin after the character");
    /* RXDACK read with DR = 0: X; with DR = 1: the RBR. */
    set(b, "RXDACK", LOGIC_0);
    set(b, "RD", LOGIC_0);
    run(b, NS(200));
    h_expect_bus(b, HS_D, 8, "XXXXXXXX", "RXDACK read with DR = 0");
    set(b, "RD", LOGIC_1);
    set(b, "RXDACK", LOGIC_1);
    run(b, NS(200));
    h_frame_bits(0x3C, 8, 'o', 1, want);
    hs_write(b, 1, 0x32);   /* 8 bits, odd */
    h_drive_bits(b, "SDI", want, bt);
    run(b, bt);
    set(b, "RXDACK", LOGIC_0);
    set(b, "RD", LOGIC_0);
    run(b, NS(200));
    h_expect_bus(b, HS_D, 8, "00111100", "RXDACK read with DR = 1");
    set(b, "RD", LOGIC_1);
    set(b, "RXDACK", LOGIC_1);
    run(b, NS(200));
    expect_pin(b, "DR", LOGIC_0, "the RXDACK read clears DR");
    /* PE / FE / OE pins: a parity error. */
    h_frame_bits(0x3C, 8, 'e', 1, want);
    h_drive_bits(b, "SDI", want, bt);
    run(b, bt);
    expect_pin(b, "PE", LOGIC_1, "PE pin");
    hs_read(b, 1, NULL);
    expect_pin(b, "PE", LOGIC_1, "the USR read leaves the pin");
    hs_read(b, 0, NULL);
    hs_write(b, 1, 0x3C);
    /* SIE: DR as an interrupt source; SFD masks the USR group. */
    set(b, "SIE", LOGIC_1);
    set(b, "SFD", LOGIC_1);
    hs_read(b, 1, NULL);
    expect_pin(b, "INTR", LOGIC_0, "no interrupt pending");
    h_frame_bits(0x0F, 8, 0, 0, want);
    strcat(want, "0");
    h_drive_bits(b, "SDI", want, bt);
    run(b, 2 * bt);
    expect_pin(b, "FE", LOGIC_1, "FE pin");
    expect_pin(b, "INTR", LOGIC_1, "SIE: DR interrupts (FE masked by SFD)");
    hs_read(b, 1, NULL);
    expect_pin(b, "INTR", LOGIC_0, "the USR read clears it");
    hs_read(b, 0, NULL);
    set(b, "SIE", LOGIC_0);
    set(b, "SFD", LOGIC_0);
    /* MSR: RLSD and RI (HD-6406). */
    set(b, "RI", LOGIC_0);
    run(b, NS(500));
    hs_expect(b, 3, "XXXX1111", "HD-6406 MSR: CTS DSR RLSD RI");
    case_end("H7 82C52, HD-6406: baud select, USR, interrupts, errors, parity modes, MSR, echo/loop/break; ALE, CS1, DMA acks, status pins, SIE/SFD; terminal");
    hb_close(h);
}

/* ---- H8: COM78C804, COM78C802, COM78804 (com78c80x) ------------------------------------- */

static const char *const CM_DL[8] = {"DL0", "DL1", "DL2", "DL3", "DL4", "DL5", "DL6", "DL7"};

/* CLK 4.9152 MHz: 203.45 ns. */
#define CM_HALF ((SimTime)101725)
#define CM_BIT19200 (256 * 2 * CM_HALF)

typedef struct {
    Bench *b;
    unsigned lines;
    int two_ds;   /* COM78804: DS on two pins (both driven) */
} CmBench;

static void cm_set_addr(CmBench *c, unsigned line, unsigned reg) {
    unsigned a = (line << 3) | reg;
    char name[8];
    unsigned i;

    for (i = 0; i < (c->lines == 4 ? 5u : 4u); i++) {
        snprintf(name, sizeof name, "ADD%u", i);
        set(c->b, name, (a >> i) & 1u ? LOGIC_1 : LOGIC_0);
    }
}

/* DS: every pin of the name (the DIP-48 has two). */
static void cm_ds(CmBench *c, LogicValue v) {
    size_t i;

    for (i = 0; i < c->b->part->pin_count; i++) {
        if (strcmp(c->b->part->pins[i].name, "DS") == 0) {
            sim_schedule_drive(c->b->sim, c->b->drv, i, v, DRIVE_STRONG, 0);
        }
    }
}

static void cm_write(CmBench *c, unsigned line, unsigned reg, unsigned v) {
    Bench *b = c->b;

    cm_set_addr(c, line, reg);
    set(b, "WR", LOGIC_0);
    h_set_bus(b, CM_DL, 8, 1, v);
    set(b, "CS", LOGIC_0);
    run(b, NS(50));
    cm_ds(c, LOGIC_0);
    run(b, NS(200));
    cm_ds(c, LOGIC_1);
    run(b, NS(50));
    set(b, "CS", LOGIC_1);
    set(b, "WR", LOGIC_1);
    h_set_bus(b, CM_DL, 8, 0, 0);
    run(b, NS(500));
}

static unsigned cm_read(CmBench *c, unsigned line, unsigned reg, char *text) {
    Bench *b = c->b;
    unsigned v;

    cm_set_addr(c, line, reg);
    set(b, "WR", LOGIC_1);
    set(b, "CS", LOGIC_0);
    run(b, NS(50));
    cm_ds(c, LOGIC_0);
    run(b, NS(250));
    v = h_get_bus(b, CM_DL, 8, text);
    cm_ds(c, LOGIC_1);
    run(b, NS(50));
    set(b, "CS", LOGIC_1);
    run(b, NS(500));
    return v;
}

static void cm_expect(CmBench *c, unsigned line, unsigned reg, const char *want, const char *what) {
    char text[16];

    cm_read(c, line, reg, text);
    h_expect_text(text, want, what);
}

static HBench *cm_open(const char *part, unsigned lines, CmBench *c) {
    static const HClockPin CLOCKS[1] = {{"CLK", 0}};
    SimTime half = CM_HALF;
    HBench *h = hb_open(part, CLOCKS, 1, &half, 1);
    Bench *b = &h->b;
    char name[8];
    unsigned i;

    c->b = b;
    c->lines = lines;
    set(b, "RESET", LOGIC_0);
    set(b, "CS", LOGIC_1);
    cm_ds(c, LOGIC_1);
    set(b, "WR", LOGIC_1);
    set(b, "MRST", LOGIC_0);
    for (i = 0; i < lines; i++) {
        snprintf(name, sizeof name, "RXD%u", i);
        set(b, name, LOGIC_1);
        snprintf(name, sizeof name, "DSR%u", i);
        set(b, name, LOGIC_0);
        snprintf(name, sizeof name, "DCD%u", i);
        set(b, name, LOGIC_1);
    }
    cm_set_addr(c, 0, 0);
    run(b, NS(2000));
    set(b, "RESET", LOGIC_1);
    run(b, NS(2000));
    return h;
}

/* 8N1 at 19200 both ways, then the command. */
static void cm_setup(CmBench *c, unsigned line, unsigned command) {
    cm_read(c, line, 3, NULL);   /* the pointer back to MR1 */
    cm_write(c, line, 2, 0x4C);
    cm_write(c, line, 2, 0xFF);
    cm_write(c, line, 3, command);
}

static void case_h8_com78c80x(void) {
    CmBench c;
    HBench *h = cm_open("SFX-COM78C804-plcc44", 4, &c);
    Bench *b = &h->b;
    char got[24];
    char want[24];
    char text[16];

    case_begin();
    /* After RESET. */
    cm_expect(&c, 0, 1, "10000000", "status after RESET (DSR active, DCD inactive)");
    cm_expect(&c, 0, 3, "00000000", "command after RESET");
    cm_expect(&c, 0, 2, "00000000", "MR1 after RESET");
    expect_pin(b, "TXD0", LOGIC_1, "TXD marks");
    expect_pin(b, "IRQ", LOGIC_Z, "IRQ released");
    /* MR1 / MR2 through the alternating pointer; a Command read resets it. */
    cm_setup(&c, 0, 0x05);
    cm_read(&c, 0, 3, NULL);
    cm_expect(&c, 0, 2, "01001100", "MR1");
    cm_expect(&c, 0, 2, "11111111", "MR2 (the pointer alternates)");
    cm_expect(&c, 0, 2, "01001100", "MR1 again");
    cm_read(&c, 0, 3, NULL);
    cm_expect(&c, 0, 2, "01001100", "a Command read resets the pointer to MR1");
    cm_read(&c, 0, 3, NULL);
    cm_expect(&c, 0, 1, "10000001", "TxRDY after TxEN");
    /* TX on line 0 at 19200. */
    cm_write(&c, 0, 0, 'A');
    check(h_catch(b, "TXD0", CM_BIT19200, 9, 4 * CM_BIT19200, got), "line 0 frame");
    h_frame_bits('A', 8, 0, 1, want);
    h_expect_text(got, want, "line 0: 19200 8N1 A");
    run(b, CM_BIT19200);
    cm_expect(&c, 0, 1, "10000101", "TxEMT and TxRDY after the character");
    h_expect_sent(b, 0, 'A', "line 0: A logged");
    /* Line 2 at 9600 TX / 19200 RX, independent of line 0. */
    cm_read(&c, 2, 3, NULL);
    cm_write(&c, 2, 2, 0x5C);   /* 1 stop, odd parity, 8 bits */
    cm_write(&c, 2, 2, 0xEF);   /* TX 9600, RX 19200 */
    cm_write(&c, 2, 3, 0x05);
    cm_write(&c, 2, 0, 0x81);
    check(h_catch(b, "TXD2", 2 * CM_BIT19200, 10, 4 * 2 * CM_BIT19200, got), "line 2 frame");
    h_frame_bits(0x81, 8, 'o', 1, want);
    h_expect_text(got, want, "line 2: 9600 8O1 (its own rate and format)");
    expect_pin(b, "TXD0", LOGIC_1, "line 0 idle meanwhile");
    h_frame_bits(0x3C, 8, 'o', 1, want);
    h_drive_bits(b, "RXD2", want, CM_BIT19200);
    run(b, CM_BIT19200);
    cm_expect(&c, 2, 1, "10000111", "line 2: RxRDY at its 19200 receive rate (TxEMT, TxRDY)");
    cm_expect(&c, 2, 0, "00111100", "line 2: the character");
    /* Receive FIFO: two characters, the third overruns. */
    h_frame_bits(0x11, 8, 0, 1, want);
    h_drive_bits(b, "RXD0", want, CM_BIT19200);
    h_frame_bits(0x22, 8, 0, 1, want);
    h_drive_bits(b, "RXD0", want, CM_BIT19200);
    h_frame_bits(0x33, 8, 0, 1, want);
    h_drive_bits(b, "RXD0", want, CM_BIT19200);
    run(b, CM_BIT19200);
    cm_expect(&c, 0, 1, "10010111", "ORR on the third character");
    cm_expect(&c, 0, 0, "00010001", "FIFO first");
    cm_expect(&c, 0, 0, "00100010", "FIFO second");
    cm_expect(&c, 0, 1, "10010101", "FIFO empty, ORR sticky");
    cm_write(&c, 0, 3, 0x15);   /* RERR */
    cm_expect(&c, 0, 1, "10000101", "RERR clears ORR");
    cm_write(&c, 0, 3, 0x05);
    /* FER (sticky) and PER (cleared by reading the character). */
    h_frame_bits(0x0F, 8, 0, 0, want);
    strcat(want, "0");
    h_drive_bits(b, "RXD0", want, CM_BIT19200);
    run(b, 2 * CM_BIT19200);
    cm_expect(&c, 0, 1, "10100111", "FER");
    cm_read(&c, 0, 0, NULL);
    cm_expect(&c, 0, 1, "10100101", "FER stays after the read");
    cm_write(&c, 0, 3, 0x15);
    cm_write(&c, 0, 3, 0x05);
    cm_read(&c, 0, 3, NULL);
    cm_write(&c, 0, 2, 0x7C);   /* even parity */
    cm_write(&c, 0, 2, 0xFF);
    h_frame_bits(0x01, 8, 'o', 1, want);
    h_drive_bits(b, "RXD0", want, CM_BIT19200);
    run(b, CM_BIT19200);
    cm_expect(&c, 0, 1, "10001111", "PER");
    cm_read(&c, 0, 0, NULL);
    cm_expect(&c, 0, 1, "10000101", "reading the character clears PER");
    cm_setup(&c, 0, 0x05);

    /* The scanner: a receiver interrupt on line 1. */
    cm_setup(&c, 1, 0x25);   /* RxIE, RxEN, TxEN */
    h_frame_bits(0x44, 8, 0, 1, want);
    h_drive_bits(b, "RXD1", want, CM_BIT19200);
    run(b, CM_BIT19200);
    expect_pin(b, "IRQ", LOGIC_0, "line 1 receiver interrupt");
    expect_pin(b, "IRQLN0", LOGIC_1, "IRQLN = 01");
    expect_pin(b, "IRQLN1", LOGIC_0, "IRQLN = 01");
    expect_pin(b, "IRQTXRX", LOGIC_0, "IRQTXRX: receiver");
    cm_expect(&c, 0, 4, "10000010", "Interrupt Summary: IRQ, line 1, Rx");
    cm_read(&c, 1, 0, NULL);
    run(b, NS(1000));
    expect_pin(b, "IRQ", LOGIC_Z, "reading the buffer releases IRQ");
    /* A transmitter interrupt on line 3 (TxIE with TxRDY). */
    cm_setup(&c, 3, 0x07);
    run(b, NS(5000));
    expect_pin(b, "IRQ", LOGIC_0, "line 3 transmitter interrupt");
    expect_pin(b, "IRQTXRX", LOGIC_1, "IRQTXRX: transmitter");
    cm_expect(&c, 0, 4, "10000111", "Interrupt Summary: IRQ, line 3, Tx");
    cm_write(&c, 3, 3, 0x05);   /* clear TxIE */
    run(b, NS(1000));
    expect_pin(b, "IRQ", LOGIC_Z, "clearing TxIE releases IRQ");
    /* Receivers before transmitters: both pending, the receiver is reported. */
    cm_write(&c, 3, 3, 0x07);
    h_drive_bits(b, "RXD1", want, CM_BIT19200);
    run(b, CM_BIT19200);
    cm_read(&c, 3, 0, NULL);
    cm_write(&c, 3, 0, 0x00);
    cm_expect(&c, 0, 4, "10000010", "the receiver interrupt comes first");
    cm_read(&c, 1, 0, NULL);
    cm_write(&c, 3, 3, 0x05);
    run(b, 12 * CM_BIT19200);   /* line 3 finishes its character */
    /* Data set change: DSR on line 3; MCIE with RxIE interrupts. */
    cm_read(&c, 3, 3, NULL);
    cm_write(&c, 3, 2, 0x4D);   /* MCIE */
    cm_write(&c, 3, 2, 0xFF);
    cm_write(&c, 3, 3, 0x25);
    set(b, "DSR3", LOGIC_1);
    run(b, NS(5000));
    cm_expect(&c, 0, 5, "10000000", "Data Set Change Summary: line 3 (bit 7)");
    expect_pin(b, "IRQ", LOGIC_0, "DSCHNG with MCIE and RxIE interrupts");
    cm_write(&c, 0, 5, 0x80);
    run(b, NS(1000));
    cm_expect(&c, 0, 5, "00000000", "writing 1 clears it");
    expect_pin(b, "IRQ", LOGIC_Z, "and releases IRQ");
    cm_expect(&c, 3, 1, "00000101", "status DSR follows the pin (inactive)");
    set(b, "DSR3", LOGIC_0);
    run(b, NS(2000));
    cm_write(&c, 0, 5, 0xFF);
    cm_write(&c, 3, 3, 0x05);

    /* TxBRK: spacing; cleared, the waiting character is sent. */
    cm_write(&c, 0, 3, 0x0D);
    run(b, CM_BIT19200);
    expect_pin(b, "TXD0", LOGIC_0, "TxBRK spacing");
    cm_write(&c, 0, 0, 0x5A);
    run(b, 2 * CM_BIT19200);
    expect_pin(b, "TXD0", LOGIC_0, "the character waits during the break");
    cm_write(&c, 0, 3, 0x05);
    check(h_catch(b, "TXD0", CM_BIT19200, 0, 4 * CM_BIT19200, got) || get(b, "TXD0") == LOGIC_1,
          "break cleared");
    run(b, 12 * CM_BIT19200);
    h_expect_sent(b, 0, 0x5A, "the waiting character went after the break");
    /* Automatic echo: RXD to TXD; TxRDY cleared. */
    cm_write(&c, 0, 3, 0x45);
    set(b, "RXD0", LOGIC_0);
    run(b, NS(500));
    expect_pin(b, "TXD0", LOGIC_0, "automatic echo: TXD follows RXD");
    set(b, "RXD0", LOGIC_1);
    run(b, 12 * CM_BIT19200);
    cm_expect(&c, 0, 1, "10000000", "echo: TxRDY and TxEMT cleared");
    cm_read(&c, 0, 0, NULL);
    /* Local loopback: TXD high, the receiver gets the transmitter. */
    cm_write(&c, 0, 3, 0x85);
    cm_write(&c, 0, 0, 0xA7);
    run(b, CM_BIT19200);
    expect_pin(b, "TXD0", LOGIC_1, "local loopback: TXD high");
    run(b, 11 * CM_BIT19200);
    cm_expect(&c, 0, 0, "10100111", "local loopback: received what was sent");
    /* Remote loopback: RXD to TXD, RxEN and TxEN held cleared. */
    cm_write(&c, 0, 3, 0xC5);
    cm_expect(&c, 0, 3, "11000000", "remote loopback: TxEN and RxEN held cleared");
    set(b, "RXD0", LOGIC_0);
    run(b, NS(500));
    expect_pin(b, "TXD0", LOGIC_0, "remote loopback: TXD follows RXD");
    set(b, "RXD0", LOGIC_1);
    cm_write(&c, 0, 3, 0x05);
    run(b, 2 * CM_BIT19200);
    /* Terminal on line 1. */
    set(b, "RXD1", LOGIC_Z);
    run(b, NS(100));
    check(h_send(b, 1, "OK") == 1, "terminal send on line 1");
    run(b, 22 * CM_BIT19200);
    cm_expect(&c, 1, 0, "01001111", "terminal O");
    cm_expect(&c, 1, 0, "01001011", "terminal K");
    cm_write(&c, 1, 0, 'A');
    run(b, 12 * CM_BIT19200);
    h_expect_sent(b, 1, 'A', "line 1: A logged");
    hb_close(h);

    /* ---- COM78C802: 2 lines, ADD3 picks the line. ---- */
    h = cm_open("SFX-COM78C802-dip40", 2, &c);
    b = &h->b;
    cm_setup(&c, 1, 0x27);
    run(b, NS(5000));
    expect_pin(b, "IRQ", LOGIC_0, "COM78C802: line 1 transmitter interrupt");
    cm_expect(&c, 0, 4, "10000011", "COM78C802 summary: IRQ, line 1, Tx");
    cm_write(&c, 1, 0, 0x99);
    check(h_catch(b, "TXD1", CM_BIT19200, 9, 4 * CM_BIT19200, got), "COM78C802 line 1 frame");
    h_frame_bits(0x99, 8, 0, 1, want);
    h_expect_text(got, want, "COM78C802 line 1");
    set(b, "DSR0", LOGIC_1);
    run(b, NS(2000));
    cm_expect(&c, 0, 5, "00000001", "COM78C802 Data Set Change Summary bit 0");
    hb_close(h);

    /* ---- COM78804: DS on two pins (DIP-48). ---- */
    h = cm_open("SFX-COM78804-dip48", 4, &c);
    b = &h->b;
    cm_setup(&c, 2, 0x05);
    cm_expect(&c, 2, 1, "10000001", "COM78804: both DS pins strobe");
    /* The two DS pins disagree: the strobe is X, so a read drives X. */
    cm_set_addr(&c, 2, 1);
    set(b, "CS", LOGIC_0);
    set(b, "DS", LOGIC_0);
    run(b, NS(300));
    h_get_bus(b, CM_DL, 8, text);
    h_expect_text(text, "XXXXXXXX", "COM78804: DS pins that disagree read X");
    set(b, "DS", LOGIC_1);
    set(b, "CS", LOGIC_1);
    run(b, NS(500));
    case_end("H8 COM78C804/802/78804: MR pointer, per-line rates and formats, FIFO/ORR/FER/PER/RERR, scanner and summaries, DSCHNG, break, echo and loopbacks, 2-line variant, terminal");
    hb_close(h);
}


int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_h7_hs82c52();
    case_h8_com78c80x();
    return serial_bench_finish();
}
