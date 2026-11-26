/*
 * serial_maxim_test: the SPI/I2C target front ends (JF) and the MAX3100 / MAX3140 (J1, J2)
 * (split from serial_test.c by plan-68-G; the bench is support/serial_bench.c).
 */

#include "serial_bench.h"
#include "serial_bench_maxim.h"

static void case_jf_spi(void) {
    TgBench *t = tg_open();
    MB *b = &t->b;
    char text[40];
    uint32_t in;

    case_begin();
    mb_expect_pin(b, "MISO", LOGIC_Z, "MISO while CS is high");
    in = mb_spi(b, 0x3C5A, 16, text, 1);
    mb_expect_u(in, 0xA5A4, "MISO shifts byte 0 then byte 1 (next(index))");
    mb_expect_pin(b, "MISO", LOGIC_1, "CS still low: MISO shows bit 7 of byte 2 (0xA7)");
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, NS(100));
    mb_expect_pin(b, "MISO", LOGIC_Z, "MISO 3-state again after CS rises (t_float)");
    check(strcmp(tg_state(t)->log, "B R3C R5A E16 ") == 0, "SPI log: begin, two bytes, end after 16 bits");
    /* An X on CS mid-transaction: MISO drives X, the rest of the bits arrive X. */
    tg_state(t)->log[0] = '\0';
    mb_set(b, "CS", LOGIC_0);
    mb_run(b, NS(200));
    mb_set(b, "CS", LOGIC_X);
    mb_run(b, NS(100));
    mb_expect_pin(b, "MISO", LOGIC_X, "CS X: MISO drives X");
    mb_spi(b, 0xFF, 8, NULL, 0);
    check(strcmp(tg_state(t)->log, "B R00? E8 ") == 0, "CS X: the byte arrives unknown");
    case_end("JF1 SPI target: MISO Z while CS is high, bytes both ways, X on CS");
    sim_destroy(b->sim);
    free(t);
}

static void case_jf_i2c(void) {
    TgBench *t = tg_open();
    MB *b = &t->b;
    char text[16];
    LogicValue ack;
    unsigned v;

    case_begin();
    i2c_start(b);
    ack = i2c_write(b, 0x2A << 1);
    check(ack == LOGIC_0, "address 0x2A write: ACK");
    check(tg_state(t)->ties[0] == SERIAL_TIE_SCL && tg_state(t)->ties[1] == SERIAL_TIE_SDA,
          "straps: A0 tied to SCL, A1 tied to SDA");
    ack = i2c_write(b, 0x41);
    check(ack == LOGIC_0, "data byte: ACK");
    mb_expect_pin(b, "SDA", LOGIC_1, "SDA released after the ACK slot");
    i2c_restart(b);
    ack = i2c_write(b, (0x2A << 1) | 1);
    check(ack == LOGIC_0, "repeated START, address read: ACK");
    v = i2c_read(b, 1, text);
    mb_expect_u(v, 0x41, "first read byte (shifted on SCL falling edges)");
    v = i2c_read(b, 0, text);
    mb_expect_u(v, 0x42, "second read byte, then NACK");
    i2c_stop(b);
    check(strcmp(tg_state(t)->log, "S A54 W41 Sr A55 N0 KA N1 KN P ") == 0,
          "I2C log: S, address, write, Sr, read 2 with ACK then NACK, P");
    /* A wrong address: NACK, and the target ignores the rest until STOP. */
    tg_state(t)->log[0] = '\0';
    i2c_start(b);
    ack = i2c_write(b, 0x2B << 1);
    check(ack == LOGIC_1, "wrong address: NACK (SDA stays high)");
    ack = i2c_write(b, 0x00);
    check(ack == LOGIC_1, "not addressed: no ACK for data");
    i2c_stop(b);
    check(strcmp(tg_state(t)->log, "S A56 ") == 0, "wrong address: no write, no end event");
    case_end("JF2 I2C target: START, repeated START, STOP, ACK/NACK, reads, straps on SCL/SDA");
    sim_destroy(b->sim);
    free(t);
}

/* ---- J1 MAX3100 ------------------------------------------------------------------------------- */

#define M3_HALF NS(271)   /* X1 = 1.8432 MHz */

static MB *m3_open(const char *part) {
    MB *b = mb_open(part, "X1", M3_HALF, NULL, NULL);

    b->cs = "CS";
    b->sclk = "SCLK";
    b->mosi = "DIN";
    b->miso = "DOUT";
    mb_set(b, "CS", LOGIC_1);
    mb_set(b, "SCLK", LOGIC_0);
    mb_set(b, "DIN", LOGIC_0);
    mb_set(b, "SHDN", LOGIC_1);
    mb_set(b, "RX", LOGIC_1);
    mb_set(b, "CTS", LOGIC_1);
    mb_run(b, NS(2000));
    return b;
}

static unsigned m3_word(MB *b, unsigned din) {
    return mb_spi(b, din, 16, NULL, 0);
}

/* The length of data bit 0 of a 0x01 frame (high between the start bit and
   bit 1), measured edge to edge. */













static void m3_register_cases(MB *b, const char *what, int *ok_out) {
    SimTime bt = 16u * 2 * M3_HALF;   /* BRD 1 */
    char frame[16];
    char message[160];
    uint16_t log[8];
    unsigned w;

    (void)ok_out;
    mb_expect_pin(b, "DOUT", LOGIC_Z, "DOUT while CS is high");
    mb_expect_pin(b, "IRQ", LOGIC_Z, "IRQ released after power-on");
    mb_expect_pin(b, "RTS", LOGIC_1, "RTS pin high after power-on");
    mb_expect_pin(b, "TX", LOGIC_1, "TX idles high");
    w = m3_word(b, 0x4000);
    mb_expect_u(w, 0x4000, "Read Config at power-on: R = 0, T = 1, config 0");
    /* Write Config: RM, 8N1, B = 0000 (115200 on 1.8432 MHz). */
    w = m3_word(b, 0xC000 | 0x0400);
    mb_expect_u(w, 0x4000, "Write Config returns R T and zeros");
    w = m3_word(b, 0x4000);
    mb_expect_u(w, 0x4400, "Read Config returns the last configuration (RM)");
    /* Write Data 'A' with RTS = 1 (pin low). */
    m3_word(b, 0x8000 | 0x0200 | 'A');
    mb_expect_pin(b, "RTS", LOGIC_0, "RTS bit 1 drives the pin low");
    check(mb_wait(b, "TX", LOGIC_0, bt * 4, NS(20)) >= 0, "TX start bit");
    mb_frame(b, "TX", bt, 9, frame);
    snprintf(message, sizeof message, "%s: TX frame of 'A' (0x41 LSB first, stop): %s", what, frame);
    check(strcmp(frame, "0100000101") == 0, message);
    mb_run(b, bt * 2);
    check(mb_sent(b, 0, log, 8) >= 1 && log[0] == 'A', "sent log holds 'A'");
    /* Receive: one frame on RX, Read Data pops it; IRQ by RM. */
    mb_drive_byte(b, "RX", 0x5A, bt, 1);
    mb_run(b, bt);
    mb_expect_pin(b, "IRQ", LOGIC_0, "R with RM: IRQ asserted (open drain low)");
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0xC0FF, 0xC05A, "Read Data: R = 1, T = 1, D = 0x5A");
    mb_expect_pin(b, "IRQ", LOGIC_Z, "IRQ released once the word is read");
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x80FF, 0x0000, "Read Data on an empty FIFO: R = 0, D = 0");
}

static void case_j1(void) {
    MB *b = m3_open("SFX-MAX3100-qsop16");
    SimTime bt = 16u * 2 * M3_HALF;
    char frame[16];
    unsigned w;
    unsigned i;
    long long len;
    int ok = 1;

    case_begin();
    m3_register_cases(b, "MAX3100", &ok);
    /* Baud table: B = 1010 is BRD 12, B = 1000 is BRD 3. */
    m3_word(b, 0xC000 | 0x000A);
    m3_word(b, 0x8000 | 0x01);
    len = m3_start_bit(b, "TX", bt * 30);
    check(within(len, (long long)(16u * 12 * 2 * M3_HALF), (long long)(2 * M3_HALF)),
          "B = 1010: bit time 16 x 12 X1 periods");
    mb_run(b, bt * 12 * 10);
    m3_word(b, 0xC000 | 0x0008);
    m3_word(b, 0x8000 | 0x01);
    len = m3_start_bit(b, "TX", bt * 8);
    check(within(len, (long long)(16u * 3 * 2 * M3_HALF), (long long)(2 * M3_HALF)),
          "B = 1000: bit time 16 x 3 X1 periods");
    mb_run(b, bt * 3 * 10);
    /* FIFO: 8 words in, read in order; a 9th clears it (RA/FE set). */
    m3_word(b, 0xC000);
    for (i = 0; i < 8; i++) {
        mb_drive_byte(b, "RX", 0x30 + i, bt, 1);
    }
    mb_run(b, bt);
    for (i = 0; i < 8; i++) {
        w = m3_word(b, 0x0000);
        mb_expect_u(w & 0x80FF, 0x8030 + i, "FIFO word in order with R = 1");
    }
    mb_expect_u(m3_word(b, 0x0000) & 0x8000, 0, "FIFO empty after 8 reads");
    for (i = 0; i < 9; i++) {
        mb_drive_byte(b, "RX", 0x40 + i, bt, 1);
    }
    mb_run(b, bt);
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x84FF, 0x0400, "9th word: the FIFO is cleared, RA/FE = 1, R = 0");
    /* Framing error sets RA/FE; the next good frame clears it. */
    m3_word(b, 0xC000);
    mb_drive_byte(b, "RX", 0x55, bt, 0);
    mb_run(b, bt * 2);
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x0400, 0x0400, "a 0 stop bit sets RA/FE");
    mb_drive_byte(b, "RX", 0x66, bt, 1);
    mb_run(b, bt);
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x0400, 0, "the next good frame clears RA/FE");
    /* IRQ masks: RAM (bit 8) with a framing error. */
    m3_word(b, 0xC000 | 0x0100);
    mb_expect_pin(b, "IRQ", LOGIC_Z, "RAM with RA/FE = 0: no IRQ");
    mb_drive_byte(b, "RX", 0x00, bt, 0);
    mb_run(b, bt * 2);
    mb_expect_pin(b, "IRQ", LOGIC_0, "RAM: a framing error asserts IRQ");
    /* T bit and its latch: TM (bit 11). */
    m3_word(b, 0xC000 | 0x0800);
    mb_expect_pin(b, "IRQ", LOGIC_0, "TM: T = 1 after Write Config asserts IRQ");
    mb_set(b, "CS", LOGIC_0);
    mb_run(b, SPI_HALF);
    for (i = 0; i < 16; i++) {
        mb_set(b, "DIN", LOGIC_0);
        mb_run(b, SPI_HALF);
        mb_set(b, "SCLK", LOGIC_1);
        mb_run(b, SPI_HALF);
        mb_set(b, "SCLK", LOGIC_0);
    }
    mb_run(b, SPI_HALF);
    mb_expect_pin(b, "IRQ", LOGIC_0, "T latch: still set after the 16th clock (CS low)");
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, NS(200));
    mb_expect_pin(b, "IRQ", LOGIC_Z, "T latch cleared on CS rising of Read Data (planner)");
    m3_word(b, 0x8000 | 0x11);
    m3_word(b, 0x8000 | 0x22);
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x4000, 0, "T = 0 while the buffer holds the second word");
    mb_run(b, bt * 22);
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x4000, 0x4000, "T = 1 once both have gone");
    mb_expect_pin(b, "IRQ", LOGIC_Z, "the Read Data cleared the latch again");
    /* Test mode: digital loopback and the 16x clock on RTS while CS is low. */
    m3_word(b, 0xC000);
    m3_word(b, 0x4001);
    m3_word(b, 0x8000 | 0x5C);
    mb_set(b, "CS", LOGIC_0);
    {
        int seen0 = 0;
        int seen1 = 0;

        for (i = 0; i < 8; i++) {
            mb_run(b, NS(130));
            seen0 |= mb_get(b, "RTS") == LOGIC_0;
            seen1 |= mb_get(b, "RTS") == LOGIC_1;
        }
        check(seen0 && seen1, "test mode: RTS shows the 16x clock while CS is low");
    }
    mb_run(b, bt * 12);
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, NS(500));
    mb_expect_pin(b, "RTS", LOGIC_1, "test mode: RTS back to its bit with CS high");
    m3_word(b, 0x4000);   /* TEST = 0 leaves test mode */
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x80FF, 0x805C, "test mode: TX looped back into the receiver");
    /* 9-bit mode: PE = 1, Pt sent as the 9th bit, Pr received. */
    m3_word(b, 0xC000 | 0x0020 | 0x0200);
    m3_word(b, 0x8000 | 0x0100 | 0x12);
    check(mb_wait(b, "TX", LOGIC_0, bt * 4, NS(20)) >= 0, "9-bit: start bit");
    mb_frame(b, "TX", bt, 10, frame);
    check(strcmp(frame, "00100100011") == 0, "9-bit: D0-D7 of 0x12, Pt = 1, stop");
    mb_drive_bits(b, "RX", "0100100001", bt, LOGIC_1);   /* 0x12, 9th bit 0, stop */
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, bt * 2);
    mb_expect_pin(b, "IRQ", LOGIC_Z, "PM: no IRQ for Pr = 0");
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x01FF, 0x0012, "9-bit: Pr = 0, D = 0x12");
    mb_drive_bits(b, "RX", "1010010011", bt, LOGIC_1);   /* 0x25, 9th bit 1, stop */
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, bt * 2);
    mb_expect_pin(b, "IRQ", LOGIC_0, "PM: Pr = 1 asserts IRQ (9-bit address)");
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x01FF, 0x0125, "9-bit: Pr = 1, D = 0x25");
    /* Software shutdown: SHDNo after the transmitter is idle; RA on RX activity. */
    m3_word(b, 0xC000 | 0x1000 | 0x0100);
    w = m3_word(b, 0x4000);
    mb_expect_u(w & 0x1000, 0x1000, "SHDNi: SHDNo = 1");
    mb_set(b, "RX", LOGIC_0);
    mb_run(b, NS(1000));
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, NS(1000));
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x0400, 0x0400, "shutdown: an RX transition sets RA");
    mb_expect_pin(b, "IRQ", LOGIC_0, "shutdown: RA with RAM asserts IRQ (wake-up)");
    m3_word(b, 0x8000 | 0x77);
    mb_run(b, bt * 3);
    mb_expect_pin(b, "TX", LOGIC_1, "shutdown: no transmission (clock stopped)");
    m3_word(b, 0xC000);
    w = m3_word(b, 0x4000);
    mb_expect_u(w & 0x1400, 0, "Write Config SHDNi = 0: running again, RA cleared");
    /* Hardware shutdown aborts a frame. */
    m3_word(b, 0x8000 | 0x00);
    check(mb_wait(b, "TX", LOGIC_0, bt * 4, NS(20)) >= 0, "frame starts");
    mb_run(b, bt * 2);
    mb_set(b, "SHDN", LOGIC_0);
    mb_run(b, NS(100));
    mb_expect_pin(b, "TX", LOGIC_1, "SHDN low aborts the frame (TX high)");
    w = m3_word(b, 0x4000);
    mb_expect_u(w & 0x5000, 0x5000, "SHDN low: SHDNo = 1, T = 1");
    mb_set(b, "SHDN", LOGIC_1);
    mb_run(b, NS(100));
    w = m3_word(b, 0x4000);
    mb_expect_u(w & 0x1000, 0, "SHDN high: SHDNo = 0");
    case_end("J1 MAX3100: commands, baud table, FIFO, IRQ masks, T latch, loopback, 9-bit, shutdown");
    mb_close(b);
}

/* ---- J2 MAX3140 ---------------------------------------------------------------------------------- */

static void case_j2(void) {
    MB *b = m3_open("SFX-MAX3140-qsop28");
    SimTime bt = 16u * 2 * M3_HALF;
    unsigned w;
    int ok = 1;

    case_begin();
    m3_register_cases(b, "MAX3140", &ok);
    /* Driver: DE = 1, Y = DI, Z = not DI; TXP inverts; DE = 0 -> Z. SRL open (115k). */
    mb_set(b, "DI", LOGIC_1);
    mb_set(b, "DE", LOGIC_1);
    mb_set(b, "RE", LOGIC_1);
    mb_run(b, NS(4000));
    mb_expect_pin(b, "Y", LOGIC_1, "DE = 1, DI = 1");
    mb_expect_pin(b, "Z", LOGIC_0, "DE = 1, DI = 1");
    mb_set(b, "DI", LOGIC_0);
    mb_run(b, NS(2100));
    mb_expect_pin(b, "Y", LOGIC_0, "DI = 0 after tdp115");
    mb_expect_pin(b, "Z", LOGIC_1, "DI = 0 after tdp115");
    mb_set(b, "TXP", LOGIC_1);
    mb_run(b, NS(2100));
    mb_expect_pin(b, "Y", LOGIC_1, "TXP = 1 inverts the driver");
    mb_set(b, "TXP", LOGIC_0);
    mb_set(b, "DE", LOGIC_0);
    mb_run(b, NS(2500));
    mb_expect_pin(b, "Y", LOGIC_Z, "DE = 0: Y high impedance");
    mb_expect_pin(b, "Z", LOGIC_Z, "DE = 0: Z high impedance");
    /* Receiver: RE = 0, H/F open (pull-down: full duplex). */
    mb_set(b, "RE", LOGIC_0);
    mb_set(b, "A", LOGIC_1);
    mb_set(b, "B", LOGIC_0);
    mb_run(b, NS(4000));
    mb_expect_pin(b, "RO", LOGIC_1, "A = 1, B = 0 -> RO 1");
    mb_set(b, "A", LOGIC_0);
    mb_set(b, "B", LOGIC_1);
    mb_run(b, NS(300));
    mb_expect_pin(b, "RO", LOGIC_0, "A = 0, B = 1 -> RO 0");
    mb_set(b, "B", LOGIC_0);
    mb_run(b, NS(300));
    mb_expect_pin(b, "RO", LOGIC_1, "A = B (shorted) -> RO 1 (true fail-safe)");
    mb_set(b, "A", LOGIC_Z);
    mb_set(b, "B", LOGIC_Z);
    mb_run(b, NS(300));
    mb_expect_pin(b, "RO", LOGIC_1, "A, B open -> RO 1 (true fail-safe)");
    mb_set(b, "A", LOGIC_X);
    mb_set(b, "B", LOGIC_0);
    mb_run(b, NS(300));
    mb_expect_pin(b, "RO", LOGIC_X, "A = X -> RO X");
    mb_set(b, "A", LOGIC_0);
    mb_set(b, "B", LOGIC_1);
    mb_set(b, "RXP", LOGIC_1);
    mb_run(b, NS(300));
    mb_expect_pin(b, "RO", LOGIC_1, "RXP = 1 inverts the receiver");
    mb_set(b, "RXP", LOGIC_0);
    mb_set(b, "RE", LOGIC_1);
    mb_run(b, NS(300));
    mb_expect_pin(b, "RO", LOGIC_Z, "RE = 1: RO high impedance");
    /* Half duplex: the receiver reads Y/Z (its own driver: echo). */
    mb_set(b, "H/F", LOGIC_1);
    mb_set(b, "RE", LOGIC_0);
    mb_set(b, "DE", LOGIC_1);
    mb_set(b, "DI", LOGIC_0);
    mb_run(b, NS(8000));
    mb_expect_pin(b, "RO", LOGIC_0, "H/F = 1: RO follows Y/Z (driver 0)");
    mb_set(b, "DI", LOGIC_1);
    mb_run(b, NS(2500));
    mb_expect_pin(b, "RO", LOGIC_1, "H/F = 1: RO follows Y/Z (driver 1)");
    mb_set(b, "H/F", LOGIC_0);
    /* Transceiver shutdown: RE = 1, DE = 0 held; enabling is then slower. */
    mb_set(b, "RE", LOGIC_1);
    mb_set(b, "DE", LOGIC_0);
    mb_run(b, NS(5000));
    mb_set(b, "DE", LOGIC_1);
    mb_run(b, NS(4000));
    mb_expect_pin(b, "Y", LOGIC_Z, "from shutdown: not yet enabled after tdz115");
    mb_run(b, NS(2500));
    mb_expect_pin(b, "Y", LOGIC_1, "from shutdown: enabled after tdzs115");
    mb_set(b, "DE", LOGIC_0);
    mb_set(b, "DI", LOGIC_Z);
    mb_set(b, "A", LOGIC_Z);
    mb_set(b, "B", LOGIC_Z);
    /* Test mode: TX held low while CS is low (planner decision). */
    m3_word(b, 0x4001);
    mb_set(b, "CS", LOGIC_0);
    mb_run(b, NS(200));
    mb_expect_pin(b, "TX", LOGIC_0, "MAX3140 test mode: TX low while CS is low");
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, NS(200));
    mb_expect_pin(b, "TX", LOGIC_1, "MAX3140 test mode: TX released with CS high");
    m3_word(b, 0x4000);
    /* Terminal: drives A (true) and B (complement); the UART receives it. */
    mb_run(b, NS(1000));
    check(mb_send(b, 0, "K") == 1, "terminal send queued");
    {
        int ab = 0;
        unsigned i;

        for (i = 0; i < 40; i++) {
            mb_run(b, bt / 2);
            if (mb_get(b, "A") == LOGIC_0 && mb_get(b, "B") == LOGIC_1) {
                ab = 1;
            }
        }
        check(ab, "terminal: A low with B high during a 0 bit");
    }
    mb_expect_pin(b, "A", LOGIC_Z, "terminal: A released after the frame");
    mb_expect_pin(b, "B", LOGIC_Z, "terminal: B released after the frame");
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x80FF, 0x8000 | 'K', "terminal: Read Data gets 'K'");
    case_end("J2 MAX3140: J1 registers, Y/Z true/complement and Z, A/B fail-safe receive, test mode TX low");
    mb_close(b);
}


int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_jf_spi();
    case_jf_i2c();
    case_j1();
    case_j2();
    return serial_bench_finish();
}
