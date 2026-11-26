/*
 * serial_bridge_test: the SC16IS / PI7C9X / XR20M bridges (I1-I7)
 * (split from serial_test.c by plan-68-G; the bench is support/serial_bench.c).
 */

#include "serial_bench.h"
#include "serial_bench_maxim.h"
#include "serial_bench_bridge.h"

/* ---- I1 SC16IS750 over I2C ------------------------------------------------------------- */

static void case_i1(void) {
    Br r;
    MB *b;
    char frame[16];
    uint16_t frames[8];
    size_t n;

    case_begin();
    address_table("SFX-SC16IS750", 0);
    r = br_open("SFX-SC16IS750", 0, 0, NULL);
    b = r.b;
    br_expect(&r, 0, 3, 0x1D, "LCR after reset (lcrReset 0x1D)");
    br_expect(&r, 0, 8, 0x40, "TXLVL after reset");
    br_expect(&r, 0, 9, 0x00, "RXLVL after reset");
    br_expect(&r, 0, 2, 0x01, "IIR after reset");
    br_expect(&r, 0, 5, 0x60, "LSR after reset");
    mb_expect_pin(b, "IRQ", LOGIC_1, "IRQ released (pulled up)");
    /* LCR, DLL, THR: a TX frame. */
    br_program(&r, 0, 16);
    check(br_w(&r, 0, 0, 0x55) == LOGIC_0, "THR write ACKed");
    br_frame(b, "TX", 16 * BIT1, frame, NS(200000));
    check(strcmp(frame, "0101010101") == 0, "TX frame of 0x55 at divisor 16 (start, LSB first, stop)");
    mb_run(b, NS(400000));
    n = mb_sent(b, 0, frames, 8);
    check(n == 1 && frames[0] == 0x55, "one frame sent: 0x55");
    /* RX: a frame in, RXLVL, the RX interrupt, RHR. */
    br_w(&r, 0, 1, 0x01);
    mb_drive_byte(b, "RX", 0xA5, 16 * BIT1, 1);
    mb_run(b, 16 * BIT1);
    br_expect(&r, 0, 9, 0x01, "RXLVL after a frame");
    mb_expect_pin(b, "IRQ", LOGIC_0, "IRQ low: RX data");
    br_expect(&r, 0, 2, 0x04, "IIR: RHR data (FIFO off)");
    br_expect(&r, 0, 0, 0xA5, "RHR");
    mb_run(b, NS(2000));
    mb_expect_pin(b, "IRQ", LOGIC_1, "IRQ released after the RHR read");
    /* A wrong address: NACK. */
    i2c_start(b);
    check(i2c_write(b, 0x49 << 1) == LOGIC_1, "address 0x49 (not this chip's 0x48): NACK");
    i2c_stop(b);
    /* SRESET: the byte is NACKed and the chip resets (SPR is not reset). */
    br_w(&r, 0, 7, 0x5A);
    check(br_w(&r, 0, 14, 0x08) == LOGIC_1, "IOControl[3] (SRESET) write: NACK");
    br_expect(&r, 0, 3, 0x1D, "LCR back at 0x1D after the software reset");
    br_expect(&r, 0, 7, 0x5A, "SPR kept by the software reset");
    br_expect(&r, 0, 1, 0x00, "IER reset");
    mb_close(b);
    /* The 25 ms SDA-low bus timeout: the target holds SDA low (bit 7 of a
       read) while the controller stops clocking; it lets go after 25 ms. */
    r = br_open("SFX-SC16IS750", 0, 0, NULL);
    b = r.b;
    br_w(&r, 0, 7, 0x00);
    i2c_start(b);
    i2c_write(b, r.addr << 1);
    i2c_write(b, br_sub(7, 0));
    i2c_restart(b);
    i2c_write(b, (r.addr << 1) | 1u);
    mb_run(b, NS(2000));
    mb_expect_pin(b, "SDA", LOGIC_0, "the target drives bit 7 (0) of the read");
    mb_run(b, (SimTime)24000000000ull);
    mb_expect_pin(b, "SDA", LOGIC_0, "still held at 24 ms");
    mb_run(b, (SimTime)2000000000ull);
    mb_expect_pin(b, "SDA", LOGIC_1, "released after 25 ms (bus interface reset)");
    i2c_stop(b);
    br_expect(&r, 0, 3, 0x1D, "the port answers again");
    mb_close(b);
    case_end("I1 SC16IS750 I2C: 16-entry address table, LCR/DLL/THR -> TX frame, RX -> RHR, IRQ, "
             "NACK for a wrong address and for SRESET, 25 ms SDA timeout");
}

/* ---- I2 SC16IS750 over SPI ------------------------------------------------------------- */

static void case_i2(void) {
    Br r = br_open("SFX-SC16IS750", 1, 0, NULL);
    MB *b = r.b;
    char text[80];
    uint8_t out[8];
    uint8_t in[8];
    uint16_t frames[8];
    size_t n;

    case_begin();
    mb_expect_pin(b, "SO", LOGIC_Z, "SO 3-state while CS is high");
    mb_spi(b, (0x80u | br_sub(3, 0)) << 8, 16, text, 0);
    check(strcmp(text, "ZZZZZZZZ00011101") == 0, "LCR read: SO Z during the command byte, then 0x1D");
    mb_expect_pin(b, "SO", LOGIC_Z, "SO 3-state again after CS rises");
    /* Command byte: bit 7 R/W, [6:3] register, [2:1] channel. */
    br_w(&r, 0, 7, 0x5A);
    br_expect(&r, 0, 7, 0x5A, "SPR written and read back");
    br_w(&r, 1, 7, 0x33);
    br_expect(&r, 0, 7, 0x5A, "a write with channel bits 01 (reserved on one channel) is ignored");
    br_expect_text(&r, 1, 7, "XXXXXXXX", "a read with channel bits 01 drives X");
    mb_spi(b, (br_sub(7, 0) | 0x01u) << 8 | 0x44u, 16, NULL, 0);
    br_expect(&r, 0, 7, 0x44, "command bit 0 is don't care");
    /* Burst FIFO write in one CS frame (TX held by EFCR[2]). */
    br_program(&r, 0, 1);
    br_w(&r, 0, 2, 0x07);
    br_w(&r, 0, 15, 0x04);
    out[0] = (uint8_t)br_sub(0, 0);
    out[1] = 0x11;
    out[2] = 0x22;
    out[3] = 0x33;
    out[4] = 0x44;
    br_spi_frame(b, out, 5, NULL, NULL);
    br_expect(&r, 0, 8, 60, "TXLVL after a 4-byte burst (TXDISABLE holds them)");
    mb_run(b, 30 * BIT1);
    n = mb_sent(b, 0, frames, 8);
    check(n == 0, "EFCR[2] TXDISABLE: nothing sent");
    br_w(&r, 0, 15, 0x00);
    mb_run(b, 50 * BIT1);
    n = mb_sent(b, 0, frames, 8);
    check(n == 4 && frames[0] == 0x11 && frames[1] == 0x22 && frames[2] == 0x33 && frames[3] == 0x44,
          "the burst goes out in order once TX is enabled");
    /* Burst FIFO read in one CS frame. */
    mb_drive_byte(b, "RX", 0x61, BIT1, 1);
    mb_drive_byte(b, "RX", 0x62, BIT1, 1);
    mb_drive_byte(b, "RX", 0x63, BIT1, 1);
    mb_run(b, 4 * BIT1);
    br_expect(&r, 0, 9, 3, "RXLVL 3");
    out[0] = (uint8_t)(0x80u | br_sub(0, 0));
    out[1] = out[2] = out[3] = 0;
    br_spi_frame(b, out, 4, in, text);
    check(in[1] == 0x61 && in[2] == 0x62 && in[3] == 0x63, "one CS frame reads RHR three times in order");
    br_expect(&r, 0, 9, 0, "RXLVL 0 after the burst read");
    /* RXDISABLE: frames are dropped. */
    br_w(&r, 0, 15, 0x02);
    mb_drive_byte(b, "RX", 0x70, BIT1, 1);
    mb_run(b, 4 * BIT1);
    br_expect(&r, 0, 9, 0, "EFCR[1] RXDISABLE: nothing received");
    br_w(&r, 0, 15, 0x00);
    /* RS-485: RTSCON drives RTS low while sending, RTSINVER flips it. */
    mb_expect_pin(b, "RTS", LOGIC_1, "RTS inactive (MCR[1] = 0)");
    br_w(&r, 0, 15, 0x10);
    br_w(&r, 0, 0, 0x7E);
    mb_run(b, NS(400));
    mb_expect_pin(b, "RTS", LOGIC_0, "EFCR[4]: RTS low from the THR write");
    mb_run(b, 12 * BIT1);
    mb_expect_pin(b, "RTS", LOGIC_1, "EFCR[4]: RTS high after the last bit");
    br_w(&r, 0, 15, 0x30);
    mb_expect_pin(b, "RTS", LOGIC_0, "EFCR[5] RTSINVER: RTS low while idle");
    br_w(&r, 0, 0, 0x7E);
    mb_run(b, NS(400));
    mb_expect_pin(b, "RTS", LOGIC_1, "EFCR[5]: RTS high while sending");
    mb_run(b, 12 * BIT1);
    br_w(&r, 0, 15, 0x00);
    /* 9-bit multidrop: with LCR[5:3] = 111 an address byte (9th bit 1)
       arrives with PE; data bytes wait for EFCR[1] = 0. */
    br_w(&r, 0, 3, 0x3B);
    br_w(&r, 0, 2, 0x07);
    br_w(&r, 0, 15, 0x03);
    mb_drive_bits(b, "RX", "0000110001", BIT1, LOGIC_1);   /* data 0x30, 9th bit 0 */
    mb_drive_bits(b, "RX", "1000000011", BIT1, LOGIC_1);   /* address 0x01, 9th bit 1 */
    mb_run(b, 4 * BIT1);
    br_expect(&r, 0, 9, 1, "9-bit normal mode, receiver off: only the address byte is kept");
    br_expect(&r, 0, 5, 0xE5, "LSR: data, PE (the address flag), FIFO error");
    br_expect(&r, 0, 0, 0x01, "RHR: the address");
    /* Auto address (EFR[5] = 1, Xoff2 = this address). */
    br_w(&r, 0, 3, 0xBF);
    br_w(&r, 0, 2, 0x30);
    br_w(&r, 0, 7, 0x05);
    br_w(&r, 0, 3, 0x3B);
    br_w(&r, 0, 15, 0x01);
    mb_drive_bits(b, "RX", "0000110001", BIT1, LOGIC_1);   /* data before any address: dropped */
    mb_drive_bits(b, "RX", "1100000011", BIT1, LOGIC_1);   /* address 0x03: not ours, dropped */
    mb_drive_bits(b, "RX", "0000110001", BIT1, LOGIC_1);   /* dropped */
    mb_drive_bits(b, "RX", "1010000011", BIT1, LOGIC_1);   /* address 0x05: ours, kept */
    mb_drive_bits(b, "RX", "0100110001", BIT1, LOGIC_1);   /* data 0x32: kept */
    mb_drive_bits(b, "RX", "1100000011", BIT1, LOGIC_1);   /* address 0x03: receiver off, dropped */
    mb_drive_bits(b, "RX", "0000110001", BIT1, LOGIC_1);   /* dropped */
    mb_run(b, 4 * BIT1);
    br_expect(&r, 0, 9, 2, "auto address: our address and the data after it");
    br_expect(&r, 0, 0, 0x05, "first: the matching address");
    br_expect(&r, 0, 0, 0x32, "then its data");
    mb_close(b);
    case_end("I2 SC16IS750 SPI: command byte R/W, register, channel bits; SO 3-state; burst FIFO "
             "write and read; EFCR TX/RX disable, RS-485 RTSCON/RTSINVER, 9-bit multidrop and auto address");
}

/* ---- I3 SC16IS752 ------------------------------------------------------------------------ */

static void case_i3(void) {
    Br r = br_open("SFX-SC16IS752", 1, 0, NULL);
    MB *b = r.b;
    uint16_t frames[4];

    case_begin();
    br_w(&r, 1, 7, 0x42);
    br_w(&r, 0, 7, 0x24);
    br_expect(&r, 1, 7, 0x42, "channel B SPR");
    br_expect(&r, 0, 7, 0x24, "channel A SPR");
    br_program(&r, 1, 1);
    br_expect(&r, 1, 3, 0x03, "channel B LCR");
    br_expect(&r, 0, 3, 0x1D, "channel A LCR untouched");
    br_w(&r, 1, 0, 0x5A);
    mb_run(b, 12 * BIT1);
    check(mb_sent(b, 1, frames, 4) == 1 && frames[0] == 0x5A, "channel B sends on TXB");
    check(mb_sent(b, 0, frames, 4) == 0, "channel A sent nothing");
    /* GPIO input change: the interrupt shows in channel A's IIR only. */
    br_w(&r, 0, 12, 0x01);
    br_r(&r, 0, 11, NULL);
    mb_expect_pin(b, "IRQ", LOGIC_1, "no GPIO interrupt yet");
    mb_set(b, "GPIO0/DSRB", LOGIC_0);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_0, "GPIO0 changed: IRQ low");
    br_expect(&r, 0, 2, 0x30, "channel A IIR: I/O pins (0x30)");
    br_expect(&r, 1, 2, 0x01, "channel B IIR: none (planner decision)");
    mb_expect_u(br_r(&r, 0, 11, NULL) & 0x0Fu, 0x0E, "IOState: GPIO0 low, GPIO1-3 pulled up");
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_1, "the IOState read clears it");
    /* IOLATCH = 0: a pulse back to the old level before the read clears it. */
    mb_set(b, "GPIO0/DSRB", LOGIC_1);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_0, "GPIO0 high: IRQ low");
    mb_set(b, "GPIO0/DSRB", LOGIC_0);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_1, "IOLATCH 0: back to the read level, the interrupt clears");
    /* IOLATCH = 1: the change is latched until IOState is read. */
    br_w(&r, 0, 14, 0x01);
    mb_set(b, "GPIO0/DSRB", LOGIC_1);
    mb_run(b, NS(1000));
    mb_set(b, "GPIO0/DSRB", LOGIC_0);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_0, "IOLATCH 1: still pending after the pin returns");
    mb_expect_u(br_r(&r, 0, 11, NULL) & 0x01u, 0x01, "IOState shows the latched 1");
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_1, "the IOState read clears the latch");
    /* Writing IODir clears a pending GPIO interrupt. */
    mb_set(b, "GPIO0/DSRB", LOGIC_1);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_0, "pending again");
    br_w(&r, 0, 10, 0x00);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_1, "an IODir write clears it");
    br_w(&r, 0, 14, 0x00);
    /* GPIO outputs. */
    br_w(&r, 0, 10, 0x08);
    br_w(&r, 0, 11, 0x08);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "GPIO3/RIB", LOGIC_1, "GPIO3 output high");
    br_w(&r, 0, 11, 0x00);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "GPIO3/RIB", LOGIC_0, "GPIO3 output low");
    br_w(&r, 0, 10, 0x00);
    /* IOControl[1]: GPIO7..4 are channel A's modem pins; IODir has no effect on them. */
    br_w(&r, 0, 10, 0xF0);
    br_w(&r, 0, 11, 0x00);
    br_w(&r, 0, 14, 0x02);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "GPIO5/DTRA", LOGIC_1, "DTRA inactive (MCR[0] = 0)");
    br_w(&r, 0, 4, 0x01);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "GPIO5/DTRA", LOGIC_0, "MCR[0] = 1: DTRA low");
    mb_expect_pin(b, "GPIO1/DTRB", LOGIC_Z, "channel B's DTR is still a GPIO input");
    br_r(&r, 0, 6, NULL);
    mb_set(b, "GPIO4/DSRA", LOGIC_0);
    mb_run(b, NS(1000));
    mb_expect_u(br_r(&r, 0, 6, NULL) & 0x22u, 0x22, "MSR A: DSR active and delta-DSR");
    br_w(&r, 0, 14, 0x00);
    mb_close(b);
    case_end("I3 SC16IS752: two channels by sub-address, GPIO change interrupt in channel A's IIR, "
             "IOLATCH, IODir clear, GPIO outputs, IOControl modem pins");
}

/* ---- I4 SC16IS740 ------------------------------------------------------------------------- */

static void case_i4(void) {
    Br r = br_open("SFX-SC16IS740", 0, 0, NULL);
    MB *b = r.b;

    case_begin();
    br_expect_text(&r, 0, 10, "XXXXXXXX", "IODir reads X");
    br_expect_text(&r, 0, 11, "XXXXXXXX", "IOState reads X");
    br_expect_text(&r, 0, 12, "XXXXXXXX", "IOIntEna reads X");
    br_expect_text(&r, 0, 13, "XXXXXXXX", "reserved 0x0D reads X");
    br_expect_text(&r, 0, 14, "XXXXXXXX", "IOControl reads X");
    br_expect(&r, 0, 8, 0x40, "TXLVL is there");
    br_expect_text(&r, 0, 6, "XXX00000", "MSR: CD, RI, DSR have no pins (X), CTS high reads 0");
    br_w(&r, 0, 3, 0x03);
    check(br_w(&r, 0, 14, 0x08) == LOGIC_1, "IOControl[3]: the software reset still works (NACK)");
    br_expect(&r, 0, 3, 0x1D, "LCR after the software reset");
    mb_close(b);
    case_end("I4 SC16IS740: no GPIO (IODir, IOState, IOIntEna, IOControl read X), software reset");
}

/* ---- I5 PI7C9X760 / PI7C9X762 ------------------------------------------------------------- */

static void pi_sfr_on(Br *r, unsigned ch, unsigned sfr) {
    br_w(r, ch, 3, 0xBF);
    br_w(r, ch, 13, 0x5A);
    br_w(r, ch, 7, sfr);
}

static void case_i5(void) {
    Br r = br_open("SFX-PI7C9X760", 0, 0, NULL);
    MB *b = r.b;
    LogicValue acks[66];
    long long t;
    unsigned i;
    int ok;

    case_begin();
    br_expect(&r, 0, 7, 0xFF, "SPR power-up 0xFF");
    br_expect_text(&r, 0, 11, "XXXXXXXX", "IOState: inputs with no pull-up read X");
    br_w(&r, 0, 10, 0xFF);
    br_expect(&r, 0, 11, 0xFF, "IOState as outputs: the latch's reset value 0xFF");
    br_w(&r, 0, 10, 0x00);
    pi_sfr_on(&r, 0, 0x04);
    br_expect(&r, 0, 13, 0x5A, "SFREN");
    br_expect(&r, 0, 4, 0x10, "CPR reset 0x10");
    br_expect(&r, 0, 9, 0x06, "TRCTL reset 0x06");
    br_expect(&r, 0, 2, 0x00, "ASR");
    br_w(&r, 0, 8, 0x37);
    br_expect(&r, 0, 8, 0x37, "TIDLE reads back what was written");
    br_w(&r, 0, 13, 0x00);
    br_expect(&r, 0, 7, 0x00, "SFREN != 0x5A: address 7 is XOFF2 again (reset 0x00)");
    /* CPR M = 2: XIN / 2 into the generator. */
    br_w(&r, 0, 13, 0x5A);
    br_w(&r, 0, 4, 0x20);
    br_program(&r, 0, 1);
    br_w(&r, 0, 0, 0x55);
    t = br_bit_time(b, "TX");
    expect_near(t, 2 * BIT1, NS(60), "CPR M = 2: a bit is 2 x 16 XIN periods");
    mb_run(b, 12 * BIT1 * 2);
    /* CPR N = 4: sample rate 16 - SCR + N = 20. */
    pi_sfr_on(&r, 0, 0x04);
    br_w(&r, 0, 4, 0x14);
    br_w(&r, 0, 9, 0x07);   /* TRCTL[0] TX idle insertion on, with TIDLE: no effect */
    br_w(&r, 0, 3, 0x03);
    br_w(&r, 0, 0, 0x55);
    t = br_bit_time(b, "TX");
    expect_near(t, BIT1 * 20 / 16, NS(60), "CPR N = 4: 20 XIN periods per bit");
    mb_run(b, 30 * BIT1);
    /* TIDLE with TRCTL[0] on: stored only; back-to-back bytes stay back to back. */
    br_w(&r, 0, 2, 0x01);
    br_w(&r, 0, 15, 0x04);
    br_w(&r, 0, 0, 0x55);
    br_w(&r, 0, 0, 0x55);
    br_w(&r, 0, 15, 0x00);
    mb_wait(b, "TX", LOGIC_0, NS(100000), NS(20));
    mb_run(b, BIT1 * 20 / 16 * 19 / 2);
    mb_expect_pin(b, "TX", LOGIC_1, "mid stop bit of the first frame");
    mb_run(b, BIT1 * 20 / 16);
    mb_expect_pin(b, "TX", LOGIC_0, "TIDLE stored only: the next start bit follows at once");
    mb_run(b, 24 * BIT1);
    /* NACK on a THR write with the TX FIFO full. */
    br_w(&r, 0, 2, 0x07);
    br_w(&r, 0, 15, 0x04);
    i2c_start(b);
    i2c_write(b, r.addr << 1);
    i2c_write(b, br_sub(0, 0));
    for (i = 0; i < 65; i++) {
        acks[i] = i2c_write(b, i);
    }
    i2c_stop(b);
    ok = 1;
    for (i = 0; i < 64; i++) {
        ok &= acks[i] == LOGIC_0;
    }
    check(ok, "64 THR bytes: ACK");
    check(acks[64] == LOGIC_1, "the 65th with the FIFO full: NACK");
    br_expect(&r, 0, 8, 0, "TXLVL 0");
    pi_sfr_on(&r, 0, 0x04);
    br_expect(&r, 0, 6, 64, "TFD 64");
    br_w(&r, 0, 3, 0x03);
    br_w(&r, 0, 2, 0x07);
    br_w(&r, 0, 15, 0x00);
    /* RFD / RLS. */
    br_program(&r, 0, 1);
    mb_drive_byte(b, "RX", 0x41, BIT1 * 20 / 16, 1);
    mb_drive_byte(b, "RX", 0x42, BIT1 * 20 / 16, 0);   /* framing error */
    mb_run(b, 4 * BIT1);
    pi_sfr_on(&r, 0, 0x04);
    br_expect(&r, 0, 5, 3, "RFD: 3 bytes (the framing error's resync adds 0xFF)");
    br_w(&r, 0, 7, 0x44);
    br_expect(&r, 0, 5, 1, "RLS (SFR[6] = 1): 1 byte with an error");
    /* SFR[4]: bursts auto-increment the sub-address. */
    br_w(&r, 0, 7, 0x10);
    br_w(&r, 0, 3, 0x03);
    i2c_start(b);
    i2c_write(b, r.addr << 1);
    i2c_write(b, br_sub(7, 0));
    i2c_write(b, 0x99);   /* SPR */
    i2c_write(b, 0x00);   /* TXLVL (read-only) */
    i2c_stop(b);
    br_expect(&r, 0, 7, 0x99, "burst write: SPR");
    i2c_start(b);
    i2c_write(b, r.addr << 1);
    i2c_write(b, br_sub(3, 0));
    i2c_restart(b);
    i2c_write(b, (r.addr << 1) | 1u);
    mb_expect_u(i2c_read(b, 1, NULL), 0x03, "burst read: LCR");
    mb_expect_u(i2c_read(b, 0, NULL), 0x00, "then MCR (auto-increment)");
    i2c_stop(b);
    mb_close(b);
    /* PI7C9X762: the SFR bank per channel; GPIO registers shared. */
    r = br_open("SFX-PI7C9X762", 1, 0, NULL);
    b = r.b;
    pi_sfr_on(&r, 1, 0x04);
    br_w(&r, 1, 8, 0x21);
    br_expect(&r, 1, 8, 0x21, "channel B TIDLE");
    br_w(&r, 0, 3, 0xBF);
    br_expect(&r, 0, 7, 0x00, "channel A: address 7 is still XOFF2 (its SFREN is 0)");
    br_w(&r, 0, 3, 0x03);
    br_w(&r, 1, 3, 0x03);
    br_w(&r, 1, 10, 0x0F);
    br_expect(&r, 0, 10, 0x0F, "IODir is shared by both channels");
    mb_close(b);
    case_end("I5 PI7C9X760, PI7C9X762: SFR bank (SFREN 0x5A, SFR, ASR, CPR M/N, TRCTL, RFD/RLS, TFD), "
             "TIDLE stored only, NACK on a full TX FIFO, SFR[4] bursts, per-channel bank");
}

/* ---- I6 XR20M1170 ------------------------------------------------------------------------ */

static void case_i6(void) {
    Br r;
    MB *b;
    long long t;
    LogicValue acks[66];
    unsigned i;

    case_begin();
    address_table("SFX-XR20M1170", 1);
    r = br_open("SFX-XR20M1170", 0, 1, NULL);
    b = r.b;
    br_expect(&r, 0, 3, 0x1D, "LCR reset 0x1D");
    br_expect(&r, 0, 7, 0xFF, "SPR power-up 0xFF");
    br_w(&r, 0, 3, 0xBF);
    br_w(&r, 0, 2, 0x10);
    br_w(&r, 0, 3, 0x03);
    br_w(&r, 0, 4, 0x04);
    br_expect(&r, 0, 6, 0x0F, "TCR reset 0x0F (EFR[4] = 1, MCR[2] = 1)");
    br_w(&r, 0, 4, 0x00);
    /* DLD: divisor 1 + 8/16. */
    br_w(&r, 0, 3, 0x80);
    br_expect(&r, 0, 0, 0x01, "DLL power-up 0x01");
    br_w(&r, 0, 2, 0x08);
    br_expect(&r, 0, 2, 0x08, "DLD at 2 with LCR[7] = 1 and EFR[4] = 1");
    br_w(&r, 0, 3, 0x03);
    br_w(&r, 0, 0, 0x55);
    t = br_bit_time(b, "TX");
    expect_near(t, BIT1 * 3 / 2, NS(120), "DLD 8/16: 1.5 x 16 XTAL1 periods per bit");
    mb_run(b, 20 * BIT1);
    br_w(&r, 0, 3, 0x80);
    br_w(&r, 0, 2, 0x18);
    br_w(&r, 0, 3, 0x03);
    br_w(&r, 0, 0, 0x55);
    t = br_bit_time(b, "TX");
    expect_near(t, BIT1 * 3 / 4, NS(120), "DLD[5:4] = 01: 8x sampling, 1.5 x 8 periods per bit");
    mb_run(b, 20 * BIT1);
    /* NACK with the TX FIFO full. */
    br_w(&r, 0, 2, 0x07);
    br_w(&r, 0, 15, 0x04);
    i2c_start(b);
    i2c_write(b, r.addr << 1);
    i2c_write(b, br_sub(0, 0));
    for (i = 0; i < 65; i++) {
        acks[i] = i2c_write(b, i);
    }
    i2c_stop(b);
    check(acks[63] == LOGIC_0 && acks[64] == LOGIC_1, "XR20M1170: NACK for the 65th THR byte");
    mb_close(b);
    /* The 16-pin package: no GPIO or modem pins. */
    r = br_open("SFX-XR20M1170-tssop16", 1, 1, NULL);
    br_expect(&r, 0, 3, 0x1D, "tssop16: the register map answers over SPI");
    mb_close(r.b);
    case_end("I6 XR20M1170: 8-address table (A1 = SCL as VCC, SDA as GND), DLD fraction and 8x, "
             "TCR 0x0F, NACK on a full TX FIFO, tssop16");
}

/* ---- I7 XR20M1280 ------------------------------------------------------------------------ */

static unsigned xr_sub(unsigned reg) {
    return (reg & 7u) << 3;
}

static void xr_w(Br *r, unsigned reg, unsigned v) {
    br_w(r, 0, reg, v);   /* reg < 8: bits [5:3] as [6:3] with bit 6 = 0 */
}

static void case_i7(void) {
    Br r = br_open("SFX-XR20M1280-qfn32", 1, 1, NULL);
    MB *b = r.b;
    char text[20];
    long long t;
    unsigned v;

    case_begin();
    mb_spi(b, (0x80u | xr_sub(7)) << 8, 16, text, 0);
    check(strcmp(text + 8, "11111111") == 0, "SPR reset 0xFF at sub-address bits [5:3] = 111");
    br_expect(&r, 0, 3, 0x00, "LCR reset 0x00");
    br_expect(&r, 0, 5, 0x60, "LSR at 5");
    xr_w(&r, 7, 0x5A);
    br_expect(&r, 0, 7, 0x5A, "SPR written");
    mb_spi(b, (0x80u | 0x40u | xr_sub(7)) << 8, 16, text, 0);
    check(strcmp(text + 8, "01011010") == 0, "SPI bit 6 is not an address bit");
    /* Independent TX and RX generators (DLD[7:6]). */
    xr_w(&r, 3, 0xBF);
    xr_w(&r, 2, 0x10);
    xr_w(&r, 3, 0x80);
    xr_w(&r, 2, 0x40);   /* independent, TX selected */
    xr_w(&r, 0, 0x01);
    xr_w(&r, 1, 0x00);
    xr_w(&r, 2, 0xC0);   /* RX selected */
    xr_w(&r, 0, 0x02);
    xr_w(&r, 1, 0x00);
    br_expect(&r, 0, 0, 0x02, "DLL (RX generator selected)");
    xr_w(&r, 2, 0x40);
    br_expect(&r, 0, 0, 0x01, "DLL (TX generator selected)");
    xr_w(&r, 3, 0x03);
    xr_w(&r, 0, 0x55);
    t = br_bit_time(b, "TX");
    expect_near(t, BIT1, NS(60), "TX at divisor 1");
    mb_run(b, 12 * BIT1);
    mb_drive_byte(b, "RX", 0xA5, 2 * BIT1, 1);
    mb_run(b, 4 * BIT1);
    br_expect(&r, 0, 5, 0x61, "RX at divisor 2: a good frame");
    br_expect(&r, 0, 0, 0xA5, "RHR 0xA5");
    mb_drive_byte(b, "RX", 0xA5, BIT1, 1);
    mb_run(b, 20 * BIT1);
    v = br_r(&r, 0, 0, NULL);
    check(v != 0xA5, "a frame at the TX rate does not decode on the RX generator");
    br_r(&r, 0, 5, NULL);
    /* FC (LCR = 0xBF, address 0): the RX FIFO count; TRG writes. */
    xr_w(&r, 2, 0x01);
    mb_drive_byte(b, "RX", 0x31, 2 * BIT1, 1);
    mb_drive_byte(b, "RX", 0x32, 2 * BIT1, 1);
    mb_run(b, 4 * BIT1);
    xr_w(&r, 3, 0xBF);
    br_expect(&r, 0, 0, 2, "FC: 2 bytes in the RX FIFO");
    xr_w(&r, 1, 0x30);   /* FCTR: table D */
    xr_w(&r, 0, 0x02);   /* TRG (RX): 2 */
    xr_w(&r, 3, 0x03);
    xr_w(&r, 1, 0x01);
    br_expect(&r, 0, 2, 0xC4, "table D, TRG 2: RX data at 2 bytes");
    br_r(&r, 0, 0, NULL);
    br_expect(&r, 0, 2, 0xC1, "below the trigger after one read (no timeout yet)");
    br_r(&r, 0, 0, NULL);
    xr_w(&r, 1, 0x00);
    /* GPIO: MCR[2] = 1 turns DTR#/GPIO0 into a GPIO; SFR[0] maps the GPIO set. */
    xr_w(&r, 4, 0x01);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "DTR/GPIO0", LOGIC_0, "MCR[2] = 0: DTR# low (MCR[0] = 1)");
    xr_w(&r, 4, 0x04);
    xr_w(&r, 6, 0x05);   /* SFR: GPIO access, GPIO interrupt enable */
    xr_w(&r, 3, 0xBF);
    xr_w(&r, 7, 0xFE);   /* GPIOSEL: GPIO0 an output */
    xr_w(&r, 4, 0x02);   /* GPIOINT: GPIO1 */
    xr_w(&r, 3, 0x03);
    xr_w(&r, 7, 0x01);   /* GPIOLVL */
    mb_run(b, NS(1000));
    mb_expect_pin(b, "DTR/GPIO0", LOGIC_1, "GPIO0 output high (GPIOLVL)");
    xr_w(&r, 7, 0x00);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "DTR/GPIO0", LOGIC_0, "GPIO0 output low");
    mb_set(b, "DSR/GPIO1", LOGIC_0);
    mb_run(b, NS(1000));
    br_r(&r, 0, 7, NULL);
    mb_set(b, "DSR/GPIO1", LOGIC_1);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_0, "GPIO1 low -> high: IRQ low");
    br_expect(&r, 0, 2, 0x81, "ISR[6] differs from ISR[7] (FIFOs on): the GPIO interrupt");
    mb_expect_u(br_r(&r, 0, 7, NULL) & 0x02u, 0x02, "GPIOLVL reads GPIO1 high");
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_1, "the GPIOLVL read clears it");
    mb_close(b);
    case_end("I7 XR20M1280: 3-bit sub-address map, LCR 0x00, SPR 0xFF, independent TX/RX baud "
             "generators, FC, FCTR table D with TRG, GPIO set with ISR[6]");
}


int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_i1();
    case_i2();
    case_i3();
    case_i4();
    case_i5();
    case_i6();
    case_i7();
    return serial_bench_finish();
}
