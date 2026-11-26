/*
 * serial_max310x_test: the MAX310x family (J3-J6)
 * (split from serial_test.c by plan-68-G; the bench is support/serial_bench.c).
 */

#include "serial_bench.h"
#include "serial_bench_maxim.h"

/* ---- J3-J6 the max310x family ----------------------------------------------------------------- */

#define MX_HALF NS(271)   /* XIN = 1.8432 MHz */

static const char *const MX_PULLS[] = {"SCLK/SCL", "DOUT/SDA", NULL};
static const char *const MX_PULLS_B[] = {"SCLK/SCL", "MISO/SDA", NULL};

/* A max310x part in SPI mode (spi = 1) or I2C mode, its pins idle. */
static MB *mx_open(const char *part, int spi, SimTime half, const char *const *ties) {
    int b_names = strstr(part, "MAX3107") == NULL;   /* MISO/SDA, MOSI/A1, SPI/I2C */
    MB *b;
    const char *select;

    b = mb_open(part, "XIN", half, ties, spi ? NULL : (b_names ? MX_PULLS_B : MX_PULLS));
    select = b_names ? "SPI/I2C" : "I2C/SPI";
    b->cs = "CS/A0";
    b->sclk = "SCLK/SCL";
    b->mosi = b_names ? "MOSI/A1" : "DIN/A1";
    b->miso = b_names ? "MISO/SDA" : "DOUT/SDA";
    b->scl = "SCLK/SCL";
    b->sda = b->miso;
    mb_set(b, select, spi ? LOGIC_1 : LOGIC_0);
    mb_set(b, "RST", LOGIC_1);
    if (spi) {
        mb_set(b, "CS/A0", LOGIC_1);
        mb_set(b, "SCLK/SCL", LOGIC_0);
        mb_set(b, b->mosi, LOGIC_0);
    }
    mb_run(b, NS(2000));
    return b;
}

static void mx_w(MB *b, unsigned cmd, unsigned value) {
    mb_spi(b, ((0x80u | cmd) << 8) | value, 16, NULL, 0);
}

static unsigned mx_r(MB *b, unsigned cmd) {
    return mb_spi(b, cmd << 8, 16, NULL, 0) & 0xFF;
}

/* Clock on (external clock: ClockEn + PLLBypass), 8N1, DIV = 1. */
static void mx_setup(MB *b, unsigned u_bits, unsigned clksrc) {
    mx_w(b, 0x1E, clksrc);
    mx_w(b, u_bits | 0x0B, 0x03);
    mx_w(b, u_bits | 0x1C, 0x01);
    mx_w(b, u_bits | 0x1D, 0x00);
}

static void case_j3(void) {
    MB *b = mx_open("SFX-MAX3107-ssop24", 1, MX_HALF, NULL);
    SimTime bt = 16u * 2 * MX_HALF;
    uint16_t log[8];
    char text[40];
    char frame[16];
    unsigned v;
    unsigned i;
    long long t;

    case_begin();
    mb_expect_pin(b, "IRQ", LOGIC_Z, "IRQSel = 0: IRQ released once the device is ready");
    mb_set(b, "CTS", LOGIC_0);
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, NS(1000));
    /* Command byte: R + address; MISO Z during it, the data after. */
    v = mb_spi(b, 0x1F00, 16, text, 0) & 0xFF;
    mb_expect_u(v, 0xA1, "RevID (0x1F) reads 0xA1");
    check(strncmp(text, "ZZZZZZZZ", 8) == 0, "MISO Z during the command byte");
    mb_expect_u(mx_r(b, 0x0B), 0x05, "LCR resets to 0x05 (6 bits, 2 stops)");
    mb_expect_u(mx_r(b, 0x10), 0xFF, "FIFOTrgLvl resets to 0xFF");
    mb_expect_u(mx_r(b, 0x1A), 0x01, "PLLConfig resets to 0x01");
    mb_expect_u(mx_r(b, 0x1C), 0x01, "DIVLSB resets to 0x01");
    mb_expect_u(mx_r(b, 0x1E), 0x08, "CLKSource resets to 0x08 (ClockEn = 0)");
    mb_expect_u(mx_r(b, 0x02), 0x60, "ISR resets to 0x60");
    mb_expect_u(mx_r(b, 0x02), 0x00, "ISR is clear-on-read");
    /* Write: MISO stays Z. */
    v = mb_spi(b, 0x8B03, 16, text, 0);
    check(strcmp(text, "ZZZZZZZZZZZZZZZZ") == 0, "MISO Z for a whole write");
    /* Burst write to THR: the FIFO fills (no clock yet: ClockEn = 0). */
    mb_spi(b, ((uint32_t)0x80 << 24) | ((uint32_t)'a' << 16) | ((uint32_t)'b' << 8) | 'c', 32, NULL, 0);
    mb_expect_u(mx_r(b, 0x11), 3, "TxFIFOLvl after a 3-byte burst to THR");
    /* Burst auto-increment from 0x14 (XON1..XOFF2), and read back. */
    mb_spi(b, ((uint32_t)0x94 << 24) | 0x112233u, 32, NULL, 1);
    mb_spi(b, 0x44, 8, NULL, 0);
    v = mb_spi(b, (uint32_t)0x14 << 24, 32, NULL, 1);
    mb_expect_u(v & 0xFFFFFF, 0x112233, "burst read XON1, XON2, XOFF1");
    v = mb_spi(b, 0, 8, NULL, 0);
    mb_expect_u(v, 0x44, "burst read XOFF2");
    /* The clock on: the three bytes go out. */
    mx_setup(b, 0, 0x18);
    mb_run(b, bt * 34);
    check(mb_sent(b, 0, log, 8) == 3 && log[0] == 'a' && log[1] == 'b' && log[2] == 'c',
          "THR burst sent 'a' 'b' 'c'");
    mb_expect_u(mx_r(b, 0x11), 0, "TxFIFOLvl 0 after sending");
    /* RX FIFO level and a burst read of RHR (stays at 0x00). */
    for (i = 0; i < 5; i++) {
        mb_drive_byte(b, "RX", 0x61 + i, bt, 1);
    }
    mb_run(b, bt);
    mb_expect_u(mx_r(b, 0x12), 5, "RxFIFOLvl after 5 frames");
    v = mb_spi(b, 0x00000000, 32, NULL, 1);
    mb_expect_u(v & 0xFFFFFF, 0x616263, "burst read of RHR stays on the FIFO");
    v = mb_spi(b, 0, 16, NULL, 0);
    mb_expect_u(v, 0x6465, "burst read of RHR (2)");
    mb_expect_u(mx_r(b, 0x12), 0, "RxFIFOLvl 0 after reading");
    /* Errors: a framing error shows in LSR after its word is read. */
    mb_drive_byte(b, "RX", 0x70, bt, 0);
    mb_run(b, bt * 2);
    mb_expect_u(mx_r(b, 0x00), 0x70, "word with a framing error");
    mb_expect_u(mx_r(b, 0x04) & 0x0C, 0x08, "LSR FrameErr for the word just read");
    /* Auto RTS: Halt 8, Resume 8 (FlowLvl 0x11): RTS high above 8 words. */
    mx_w(b, 0x0F, 0x11);
    mx_w(b, 0x13, 0x01);
    mb_expect_pin(b, "RTS/CLKOUT", LOGIC_0, "AutoRTS: RTS asserted (low) with the FIFO empty");
    for (i = 0; i < 9; i++) {
        mb_drive_byte(b, "RX", 0x30 + i, bt, 1);
    }
    mb_run(b, bt);
    mb_expect_pin(b, "RTS/CLKOUT", LOGIC_1, "AutoRTS: 9 words > Halt: RTS deasserted");
    mx_r(b, 0x00);
    mb_expect_pin(b, "RTS/CLKOUT", LOGIC_1, "AutoRTS: 8 words is not below Resume");
    mx_r(b, 0x00);
    mb_expect_pin(b, "RTS/CLKOUT", LOGIC_0, "AutoRTS: 7 words < Resume: RTS asserted");
    mx_w(b, 0x0A, 0x02);   /* FIFORst */
    mx_w(b, 0x0A, 0x00);
    /* Auto CTS: transmit only while CTS is low. */
    mx_w(b, 0x13, 0x02);
    mb_set(b, "CTS", LOGIC_1);
    mb_run(b, NS(500));
    mx_w(b, 0x00, 0x5A);
    t = mb_wait(b, "TX", LOGIC_0, bt * 3, NS(50));
    check(t < 0, "AutoCTS: CTS high holds the transmitter");
    mb_set(b, "CTS", LOGIC_0);
    t = mb_wait(b, "TX", LOGIC_0, bt * 3, NS(50));
    check(t >= 0, "AutoCTS: CTS low lets it send");
    mb_run(b, bt * 12);
    mb_expect_u(mx_r(b, 0x02) & 0x80, 0x80, "CTS transitions set ISR[7] CTSInt");
    /* XON/XOFF transmit control: FlowCtrl SwFlowEn + SwFlow[1:0] = 10. */
    mx_w(b, 0x13, 0x28);
    mx_w(b, 0x14, 0x11);
    mx_w(b, 0x16, 0x13);
    mb_drive_byte(b, "RX", 0x13, bt, 1);
    mb_run(b, bt);
    mx_w(b, 0x00, 0x21);
    t = mb_wait(b, "TX", LOGIC_0, bt * 3, NS(50));
    check(t < 0, "XOFF1 received: the transmitter holds");
    mb_drive_byte(b, "RX", 0x11, bt, 1);
    t = mb_wait(b, "TX", LOGIC_0, bt * 3, NS(50));
    check(t >= 0, "XON1 received: it sends again");
    mb_run(b, bt * 12);
    mb_expect_u(mx_r(b, 0x12), 0, "XON/XOFF characters are not stored");
    /* Special characters: MODE2 SpecialChr, SwFlow[1:0] = 11 (all four). */
    mx_w(b, 0x13, 0x30);
    mx_w(b, 0x0A, 0x10);
    mx_w(b, 0x05, 0x0F);
    mx_w(b, 0x01, 0x02);
    mb_drive_byte(b, "RX", 0x13, bt, 1);
    mb_run(b, bt);
    mb_expect_u(mx_r(b, 0x06), 0x04, "SpclCharInt XOFF1Int");
    mb_expect_u(mx_r(b, 0x00), 0x13, "the special character is stored");
    mb_expect_u(mx_r(b, 0x06), 0x00, "SpclCharInt is clear-on-read");
    mx_w(b, 0x0A, 0x00);
    mx_w(b, 0x13, 0x00);
    mx_w(b, 0x01, 0x00);
    /* IrDA SIR: a 0 bit is a 3/16-bit low pulse; RX idles low, high pulses are 0s.
       Switching RX's idle level makes junk: flush it with FIFORst. */
    mx_w(b, 0x0E, 0x03);
    mb_set(b, "RX", LOGIC_0);
    mb_run(b, bt * 24);
    mx_w(b, 0x0A, 0x02);
    mx_w(b, 0x0A, 0x00);
    mb_expect_u(mx_r(b, 0x12), 0, "IrDA: RX idling low receives nothing");
    mx_w(b, 0x00, 0x00);
    /* The second pulse (data bit 0) is measured edge to edge. */
    t = mb_wait(b, "TX", LOGIC_0, bt * 3, NS(10));
    t = t < 0 ? t : mb_wait(b, "TX", LOGIC_1, bt, NS(10));
    t = t < 0 ? t : mb_wait(b, "TX", LOGIC_0, bt, NS(10));
    t = t < 0 ? t : mb_wait(b, "TX", LOGIC_1, bt, NS(10));
    {
        char message[120];

        snprintf(message, sizeof message, "IrDA SIR: pulse %lld ps, 3/16 of a bit is %llu", t,
                 (unsigned long long)(3 * 2 * MX_HALF));
        check(within(t, (long long)(3 * 2 * MX_HALF), NS(20)), message);
    }
    mb_run(b, bt * 12);
    {
        /* 0x0F: start 0, 1111 0000, stop 1 -> pulses for the 0s. */
        static const char bits[] = "01111000011";
        for (i = 0; bits[i] != '\0'; i++) {
            if (bits[i] == '0') {
                mb_set(b, "RX", LOGIC_1);
                mb_run(b, 3 * 2 * MX_HALF);
                mb_set(b, "RX", LOGIC_0);
                mb_run(b, bt - 3 * 2 * MX_HALF);
            } else {
                mb_run(b, bt);
            }
        }
    }
    mb_run(b, bt);
    mb_expect_u(mx_r(b, 0x00), 0x0F, "IrDA SIR receive: 0x0F");
    mx_w(b, 0x0E, 0x00);
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, bt * 24);
    mx_w(b, 0x0A, 0x02);
    mx_w(b, 0x0A, 0x00);
    /* RxTimeOut = 2 character times. */
    mx_w(b, 0x0C, 0x02);
    mb_drive_byte(b, "RX", 0x3C, bt, 1);
    mb_run(b, bt * 12);
    mb_expect_u(mx_r(b, 0x04) & 0x01, 0, "RTimeout: not yet after 1 character time");
    mb_run(b, bt * 12);
    mb_expect_u(mx_r(b, 0x04) & 0x01, 0x01, "RTimeout after 2 character times");
    mb_expect_u(mx_r(b, 0x00), 0x3C, "the waiting word");
    mb_expect_u(mx_r(b, 0x04) & 0x01, 0, "reading the word clears RTimeout");
    mx_w(b, 0x0C, 0x00);
    /* Noise: one of the three samples of a bit disagrees (one 16x tick). */
    {
        SimTime tick = 2 * MX_HALF;

        mb_set(b, "RX", LOGIC_0);
        mb_run(b, bt * 4 + bt / 2 - tick / 2);
        mb_set(b, "RX", LOGIC_1);
        mb_run(b, tick);
        mb_set(b, "RX", LOGIC_0);
        mb_run(b, bt * 5 - bt / 2 - tick / 2);
        mb_set(b, "RX", LOGIC_1);
        mb_run(b, bt * 2);
    }
    mb_expect_u(mx_r(b, 0x00), 0x00, "a glitched 0 bit still reads 0 (majority)");
    mb_expect_u(mx_r(b, 0x04) & 0x20, 0x20, "LSR RxNoise for that word");
    /* RS-485 direction control: Setup 2, Hold 3 bit times on RTS. */
    mx_w(b, 0x0D, 0x23);
    mx_w(b, 0x09, 0x10);
    mb_expect_pin(b, "RTS/CLKOUT", LOGIC_0, "auto direction: RTS low while idle");
    mx_w(b, 0x00, 0x01);
    t = mb_wait(b, "RTS/CLKOUT", LOGIC_1, bt * 2, NS(10));
    check(t >= 0, "auto direction: RTS high once the FIFO has data");
    t = mb_wait(b, "TX", LOGIC_0, bt * 4, NS(10));
    check(within(t, (long long)(2 * bt), (long long)(2 * 2 * MX_HALF)),
          "auto direction: the start bit Setup = 2 bit times after RTS");
    t = mb_wait(b, "RTS/CLKOUT", LOGIC_0, bt * 16, NS(10));
    check(within(t, (long long)(13 * bt), (long long)(2 * 2 * MX_HALF)),
          "auto direction: RTS low Hold = 3 bit times after the stop bit");
    mx_w(b, 0x09, 0x00);
    mx_w(b, 0x0D, 0x00);
    /* Multidrop with address filtering: XOFF2 = 0x42; LCR forced parity is the 9th bit. */
    mx_w(b, 0x17, 0x42);
    mx_w(b, 0x0B, 0x2B);
    mx_w(b, 0x0A, 0x50);
    mx_w(b, 0x05, 0x20);
    mb_drive_bits(b, "RX", "1000001011", bt, LOGIC_1);   /* address 0x41 */
    mb_drive_bits(b, "RX", "1000100001", bt, LOGIC_1);   /* data 0x11 */
    mb_drive_bits(b, "RX", "0100001011", bt, LOGIC_1);   /* address 0x42 (ours) */
    mb_drive_bits(b, "RX", "0100010001", bt, LOGIC_1);   /* data 0x22 */
    mb_run(b, bt);
    mb_expect_u(mx_r(b, 0x12), 1, "multidrop: only data after our address is stored");
    mb_expect_u(mx_r(b, 0x00), 0x22, "multidrop: the data word");
    mb_expect_u(mx_r(b, 0x04) & 0x04, 0, "multidrop: LSR[2] = its 9th bit (0)");
    mb_expect_u(mx_r(b, 0x06) & 0x20, 0x20, "multidrop: MultiDropInt");
    mx_w(b, 0x00, 0x42);   /* EvenParity = 0: the 9th bit is 1 */
    check(mb_wait(b, "TX", LOGIC_0, bt * 3, NS(20)) >= 0, "multidrop: frame");
    mb_frame(b, "TX", bt, 10, frame);
    check(strcmp(frame, "00100001011") == 0, "multidrop: 0x42 with the 9th bit 1, stop");
    mb_run(b, bt * 2);
    mx_w(b, 0x0A, 0x00);
    mx_w(b, 0x0B, 0x03);
    mx_w(b, 0x05, 0x00);
    /* GPIOs: GPIO0 push-pull out, GPIO1 open-drain out, GPIO2 input with an interrupt. */
    mx_w(b, 0x18, 0x23);
    mx_w(b, 0x19, 0x03);
    mb_expect_pin(b, "GPIO0", LOGIC_1, "GPIO0 push-pull 1");
    mb_expect_pin(b, "GPIO1", LOGIC_Z, "GPIO1 open drain 1 is released");
    mx_w(b, 0x19, 0x00);
    mb_expect_pin(b, "GPIO1", LOGIC_0, "GPIO1 open drain 0");
    mx_r(b, 0x08);
    mx_w(b, 0x07, 0x04);
    mx_w(b, 0x01, 0x04);
    mb_set(b, "GPIO2", LOGIC_1);
    mb_run(b, NS(2000));
    mb_expect_u(mx_r(b, 0x19) & 0x40, 0x40, "GPIOData input bit 6 = GPIO2");
    mb_expect_pin(b, "IRQ", LOGIC_Z, "IRQSel = 0: no IRQ yet");
    mx_w(b, 0x09, 0x80);
    mb_expect_pin(b, "IRQ", LOGIC_0, "GPI2Int through STSInt and ISR[2] with IRQSel");
    mb_expect_u(mx_r(b, 0x08) & 0x04, 0x04, "STSInt GPI2Int");
    mb_expect_pin(b, "IRQ", LOGIC_Z, "STSInt read clears it");
    mx_w(b, 0x01, 0x00);
    mx_w(b, 0x07, 0x00);
    mx_w(b, 0x18, 0x00);
    /* Forced sleep stops the UART; waking sends what waited. */
    mx_w(b, 0x09, 0x20);
    mb_expect_u(mx_r(b, 0x09) & 0x20, 0x20, "ForcedSleep reads back");
    mb_expect_u(mx_r(b, 0x08) & 0x40, 0x40, "STSInt SleepInt while asleep");
    mx_w(b, 0x00, 0x01);
    check(mb_wait(b, "TX", LOGIC_0, bt * 3, NS(50)) < 0, "asleep: nothing is sent");
    mx_w(b, 0x09, 0x00);
    check(mb_wait(b, "TX", LOGIC_0, bt * 3, NS(50)) >= 0, "awake: the word goes out");
    mb_run(b, bt * 12);
    /* CLKtoRTS: the 16x clock (DIV = 1: XIN) on RTS/CLKOUT. */
    mx_w(b, 0x1E, 0x98);
    {
        int seen0 = 0;
        int seen1 = 0;

        for (i = 0; i < 8; i++) {
            mb_run(b, NS(130));
            seen0 |= mb_get(b, "RTS/CLKOUT") == LOGIC_0;
            seen1 |= mb_get(b, "RTS/CLKOUT") == LOGIC_1;
        }
        check(seen0 && seen1, "CLKtoRTS: RTS/CLKOUT toggles");
    }
    mx_w(b, 0x1E, 0x18);
    /* A write command with X address bits: every register it could name
       holds X (plan-12-A section 3.5). */
    mb_set(b, "CS/A0", LOGIC_0);
    mb_run(b, SPI_HALF);
    for (i = 0; i < 16; i++) {
        mb_set(b, b->mosi, i == 0 ? LOGIC_1 : i < 8 ? LOGIC_X : LOGIC_0);
        mb_run(b, SPI_HALF);
        mb_set(b, "SCLK/SCL", LOGIC_1);
        mb_run(b, SPI_HALF);
        mb_set(b, "SCLK/SCL", LOGIC_0);
    }
    mb_run(b, SPI_HALF);
    mb_set(b, "CS/A0", LOGIC_1);
    mb_run(b, SPI_HALF);
    mb_spi(b, 0x0B00, 16, text, 0);
    check(strcmp(text + 8, "XXXXXXXX") == 0, "X command: LCR holds X");
    mb_close(b);

    /* I2C: A1 = VL (DIN/A1 high), A0 = DGND: 7-bit address 0x2E. */
    b = mx_open("SFX-MAX3107-tqfn24-3.5", 0, MX_HALF, NULL);
    mb_set(b, "DIN/A1", LOGIC_1);
    mb_set(b, "CS/A0", LOGIC_0);
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, NS(2000));
    i2c_start(b);
    check(i2c_write(b, 0x2C << 1) == LOGIC_1, "I2C: 0x2C is not this strap (NACK)");
    i2c_stop(b);
    i2c_start(b);
    check(i2c_write(b, 0x2E << 1) == LOGIC_0, "I2C: 0x2E (A1 = VL, A0 = DGND) ACKs");
    check(i2c_write(b, 0x20) == LOGIC_1, "I2C: register 0x20 is not valid (NACK)");
    i2c_stop(b);
    i2c_start(b);
    i2c_write(b, 0x2E << 1);
    check(i2c_write(b, 0x14) == LOGIC_0, "I2C: register address 0x14 ACKs");
    i2c_write(b, 0x5A);
    i2c_write(b, 0xA5);
    i2c_restart(b);
    check(i2c_write(b, (0x2E << 1) | 1) == LOGIC_0, "I2C: read address after Sr");
    i2c_read(b, 0, NULL);
    i2c_stop(b);
    i2c_start(b);
    i2c_write(b, 0x2E << 1);
    i2c_write(b, 0x14);
    i2c_restart(b);
    i2c_write(b, (0x2E << 1) | 1);
    v = i2c_read(b, 1, NULL);
    mb_expect_u(v, 0x5A, "I2C burst write then read: XON1");
    v = i2c_read(b, 0, NULL);
    mb_expect_u(v, 0xA5, "I2C auto-increment: XON2");
    i2c_stop(b);
    i2c_start(b);
    i2c_write(b, 0x2E << 1);
    i2c_write(b, 0x1F);
    i2c_restart(b);
    i2c_write(b, (0x2E << 1) | 1);
    v = i2c_read(b, 0, NULL);
    i2c_stop(b);
    mb_expect_u(v, 0xA1, "I2C: RevID");
    case_end("J3 MAX3107: SPI and I2C command bytes, bursts, FIFO levels, flow control, special characters, IrDA");
    mb_close(b);
}

/* Start-bit length of a 0x01 sent on `tx` (ps). */
static long long mx_bit_time(MB *b, const char *tx, SimTime limit) {
    return m3_start_bit(b, tx, limit);
}

static void case_j4(void) {
    /* XIN = 3.6864 MHz nominal: 136 ns halves (period 272 ns). */
    SimTime half = NS(136);
    SimTime p = 2 * half;
    MB *b = mx_open("SFX-MAX3107-ssop24", 1, half, NULL);
    long long t;
    char message[160];

    case_begin();
    mb_set(b, "RX", LOGIC_1);
    mb_set(b, "CTS", LOGIC_0);
    /* PLL bypass (counted): DIV = 2: bit = 16 x 2 x XIN. */
    mx_w(b, 0x1E, 0x18);
    mx_w(b, 0x0B, 0x03);
    mx_w(b, 0x1C, 0x02);
    mx_w(b, 0x00, 0x01);
    t = mx_bit_time(b, "TX", 200 * p);
    snprintf(message, sizeof message, "bypass: bit %lld ps, want %llu", t, (unsigned long long)(32 * p));
    check(within(t, (long long)(32 * p), (long long)p), message);
    mb_run(b, 32 * p * 10);
    /* Fractional: DIV 2 + 8/16: nine low bits of 0x00 last 9 x 16 x 2.5 periods. */
    mx_w(b, 0x1B, 0x08);
    mx_w(b, 0x00, 0x00);
    if (mb_wait(b, "TX", LOGIC_0, 200 * p, NS(10)) >= 0) {
        t = mb_wait(b, "TX", LOGIC_1, 600 * p, NS(10));
        snprintf(message, sizeof message, "fraction 2.5: 9 bits %lld ps, want %llu", t,
                 (unsigned long long)(9 * 40 * p));
        check(within(t, (long long)(9 * 40 * p), (long long)p), message);
    } else {
        check(0, "fraction: no frame");
    }
    mb_run(b, 40 * p * 4);
    mx_w(b, 0x1B, 0x00);
    /* 2x mode: 8 ticks per bit. */
    mx_w(b, 0x1B, 0x10);
    mx_w(b, 0x00, 0x01);
    t = mx_bit_time(b, "TX", 200 * p);
    check(within(t, (long long)(16 * p), (long long)p), "2x mode: bit = 8 x DIV XIN periods");
    mb_run(b, 16 * p * 12);
    mx_w(b, 0x1B, 0x00);
    /* PLL x6 with PreDiv 6: fREF = XIN (measured period). */
    mx_w(b, 0x1A, 0x06);
    mx_w(b, 0x1E, 0x14);
    mb_run(b, 4 * p);
    mx_w(b, 0x00, 0x01);
    t = mx_bit_time(b, "TX", 200 * p);
    snprintf(message, sizeof message, "PLL x6 / 6: bit %lld ps, want %llu", t, (unsigned long long)(32 * p));
    check(within(t, (long long)(32 * p), (long long)p), message);
    mb_run(b, 32 * p * 10);
    /* PLL x6 with PreDiv 3: fREF = 2 x XIN: half the bit time. */
    mx_w(b, 0x1A, 0x03);
    mb_run(b, 4 * p);
    mx_w(b, 0x00, 0x01);
    t = mx_bit_time(b, "TX", 200 * p);
    snprintf(message, sizeof message, "PLL x6 / 3: bit %lld ps, want %llu", t, (unsigned long long)(16 * p));
    check(within(t, (long long)(16 * p), (long long)p), message);
    mb_run(b, 16 * p * 10);
    /* PLL x48 / 48: fREF = XIN again. */
    mx_w(b, 0x1A, 0x40 | 48);
    mb_run(b, 4 * p);
    mx_w(b, 0x00, 0x01);
    t = mx_bit_time(b, "TX", 200 * p);
    check(within(t, (long long)(32 * p), (long long)p), "PLL x48 / 48: bit = 32 XIN periods");
    mb_run(b, 32 * p * 10);
    /* PLLEn = 0 without bypass: no reference, nothing is sent. */
    mx_w(b, 0x1E, 0x10);
    mx_w(b, 0x00, 0x01);
    check(mb_wait(b, "TX", LOGIC_0, 100 * p, NS(20)) < 0, "PLLBypass = 0, PLLEn = 0: no clock");
    case_end("J4 MAX3107 clocking: bypass counts XIN, PLL x6 and x48 measure it; fraction, 2x");
    mb_close(b);
}

static void case_j5(void) {
    MB *b = mx_open("SFX-MAX3109-tqfn32", 1, MX_HALF, NULL);
    SimTime bt = 16u * 2 * MX_HALF;
    uint16_t log[4];
    char text[40];
    unsigned v;

    case_begin();
    mb_set(b, "RX0", LOGIC_1);
    mb_set(b, "RX1", LOGIC_1);
    mb_set(b, "CTS0", LOGIC_0);
    mb_set(b, "CTS1", LOGIC_0);
    mb_expect_pin(b, "IRQ", LOGIC_Z, "MAX3109: IRQ released (no IRQSel)");
    /* U selects the UART: LCR 0x03 on UART0, 0x1B on UART1. */
    mx_w(b, 0x0B, 0x03);
    mx_w(b, 0x20 | 0x0B, 0x1B);
    mb_expect_u(mx_r(b, 0x0B), 0x03, "UART0 LCR");
    mb_expect_u(mx_r(b, 0x20 | 0x0B), 0x1B, "UART1 LCR");
    mx_w(b, 0x14, 0x77);
    mx_w(b, 0x20 | 0x14, 0x88);
    mb_expect_u(mx_r(b, 0x14), 0x77, "UART0 XON1");
    mb_expect_u(mx_r(b, 0x20 | 0x14), 0x88, "UART1 XON1");
    /* Bit 6 must be 0: such a command reads X. */
    mb_spi(b, 0x4B00, 16, text, 0);
    check(strcmp(text + 8, "XXXXXXXX") == 0, "command bit 6 = 1 reads X");
    /* GlobalIRQ and the IRQ bits on clocks 5-8 of a read command. */
    mb_expect_u(mx_r(b, 0x1F), 0x03, "GlobalIRQ 0x03: nothing pending");
    mx_w(b, 0x20 | 0x01, 0x40);   /* UART1 IRQEn RxEmptyIEn: ISR[6] is set */
    mb_expect_pin(b, "IRQ", LOGIC_0, "UART1 interrupt: IRQ low");
    mb_expect_u(mx_r(b, 0x1F), 0x01, "GlobalIRQ: IRQ1 = 0 (pending)");
    mb_spi(b, 0x00, 8, text, 0);
    check(strcmp(text, "ZZZZ0001") == 0, "fast read: Z, then 0 0 IRQ1 IRQ0 on clocks 5-8");
    mx_r(b, 0x20 | 0x02);   /* UART1 ISR read clears it */
    mb_expect_pin(b, "IRQ", LOGIC_Z, "ISR read: IRQ released");
    mb_expect_u(mx_r(b, 0x1F), 0x03, "GlobalIRQ back to 0x03");
    mx_w(b, 0x20 | 0x01, 0x00);
    /* Extended map: GloblComnd 0xCE, RevID at 0x25 (0xC2, decisions.md). */
    mx_w(b, 0x1F, 0xCE);
    mb_expect_u(mx_r(b, 0x05), 0xC2, "extended: RevID 0xC2");
    mx_w(b, 0x1F, 0xCD);
    mb_expect_u(mx_r(b, 0x05), 0x00, "extended off: 0x05 is LSRIntEn again");
    /* PLLConfig only through UART0. */
    mx_w(b, 0x20 | 0x1A, 0x22);
    mb_expect_u(mx_r(b, 0x1A), 0x01, "PLLConfig written through UART1 is ignored");
    mb_spi(b, (0x20u | 0x1A) << 8, 16, text, 0);
    check(strcmp(text + 8, "XXXXXXXX") == 0, "PLLConfig read through UART1 is X");
    /* Both UARTs send with their own format (UART1: 8 bits, even parity). */
    mx_setup(b, 0, 0x18);
    mx_w(b, 0x20 | 0x1C, 0x01);
    mx_w(b, 0x00, 0x31);
    mx_w(b, 0x20 | 0x00, 0x32);
    mb_run(b, bt * 14);
    check(mb_sent(b, 0, log, 4) == 1 && log[0] == 0x31, "UART0 sent 0x31 on TX0");
    check(mb_sent(b, 1, log, 4) == 1 && log[0] == 0x32, "UART1 sent 0x32 on TX1");
    /* Transmit trigger: UART1 SynchEn, TrigSel 3; GloblComnd 0xE3 starts it. */
    mx_w(b, 0x1F, 0xCE);
    mx_w(b, 0x20 | 0x00, 0x10 | 0x03);   /* UART1 TxSynch */
    mx_w(b, 0x1F, 0xCD);
    v = mx_r(b, 0x20 | 0x09);
    mb_expect_u(v & 0x02, 0x02, "SynchEn forces TxDisabl");
    mx_w(b, 0x20 | 0x00, 0x44);
    check(mb_wait(b, "TX1", LOGIC_0, bt * 3, NS(50)) < 0, "triggered UART waits");
    mx_w(b, 0x1F, 0xE2);
    check(mb_wait(b, "TX1", LOGIC_0, bt * 2, NS(50)) < 0, "another trigger (0xE2) does not start it");
    mx_w(b, 0x1F, 0xE3);
    check(mb_wait(b, "TX1", LOGIC_0, bt * 2, NS(50)) >= 0, "GloblComnd 0xE3 starts UART1");
    mb_run(b, bt * 12);
    check(mb_sent(b, 1, log, 4) == 2 && log[1] == 0x44, "UART1 sent 0x44");
    /* TIMER: 50 % clock on GPIO1 at UARTClk / (1024 x Timer); CLKtoGPIO: XIN on GPIO0. */
    mx_w(b, 0x1F, 0xCE);
    mx_w(b, 0x03, 0x01);   /* UART0 TIMER1 = 1 */
    mx_w(b, 0x04, 0x80);   /* TIMER2: TmrToGPIO */
    mx_w(b, 0x00, 0x80);   /* TxSynch: CLKtoGPIO */
    mx_w(b, 0x1F, 0xCD);
    {
        long long t0 = mb_wait(b, "GPIO1", LOGIC_1, 2048 * 2 * MX_HALF, NS(50));
        long long t1 = t0 < 0 ? -1 : mb_wait(b, "GPIO1", LOGIC_0, 1024 * 2 * MX_HALF, NS(50));
        int seen0 = 0;
        int seen1 = 0;
        unsigned k;

        check(within(t1, (long long)(512 * 2 * MX_HALF), NS(100)),
              "TIMER = 1: GPIO1 half period = 512 UART clocks");
        for (k = 0; k < 8; k++) {
            mb_run(b, NS(130));
            seen0 |= mb_get(b, "GPIO0") == LOGIC_0;
            seen1 |= mb_get(b, "GPIO0") == LOGIC_1;
        }
        check(seen0 && seen1, "CLKtoGPIO: the reference clock on GPIO0");
    }
    mx_w(b, 0x1F, 0xCE);
    mx_w(b, 0x04, 0x00);
    mx_w(b, 0x00, 0x00);
    mx_w(b, 0x1F, 0xCD);
    /* Receive per channel. */
    mb_drive_byte(b, "RX1", 0x55, bt, 1);
    mb_run(b, bt);
    mb_expect_u(mx_r(b, 0x20 | 0x12), 1, "UART1 RxFIFOLvl");
    mb_expect_u(mx_r(b, 0x12), 0, "UART0 RxFIFOLvl");
    mb_expect_u(mx_r(b, 0x20 | 0x00), 0x55, "UART1 RHR");
    case_end("J5 MAX3109: U bit, per-channel registers, GlobalIRQ, fast read, extended map, triggers");
    mb_close(b);
}

static void case_j6(void) {
    MB *b = mx_open("SFX-MAX14830-tqfn48", 1, MX_HALF, NULL);
    SimTime bt = 16u * 2 * MX_HALF;
    static const char *const MAX3108_TIES[] = {"MOSI/A1", "SCLK/SCL", "CS/A0", "MISO/SDA", NULL};
    char text[40];
    uint16_t log[4];
    unsigned u;
    unsigned v;

    case_begin();
    for (u = 0; u < 4; u++) {
        char name[8];

        snprintf(name, sizeof name, "RX%u", u);
        mb_set(b, name, LOGIC_1);
        snprintf(name, sizeof name, "CTS%u", u);
        mb_set(b, name, LOGIC_0);
    }
    mb_run(b, NS(500));
    for (u = 0; u < 4; u++) {
        mx_w(b, (u << 5) | 0x14, 0x10 + u);
    }
    for (u = 0; u < 4; u++) {
        mb_expect_u(mx_r(b, (u << 5) | 0x14), 0x10 + u, "U1:U0 select the UART (XON1)");
    }
    mb_expect_u(mx_r(b, 0x1F), 0x0F, "GlobalIRQ 0x0F");
    mx_w(b, 0x1F, 0xCE);
    mb_expect_u(mx_r(b, 0x05), 0xB4, "RevID 0xB4");
    mx_w(b, 0x1F, 0xCD);
    mx_w(b, (2u << 5) | 0x01, 0x40);
    mb_spi(b, 0x00, 8, text, 0);
    check(strcmp(text, "ZZZZ1011") == 0, "read command: IRQ3..IRQ0 on clocks 5-8 (UART2 pending)");
    mb_expect_pin(b, "IRQ", LOGIC_Z, "IRQSel = 0: a pending UART does not drive IRQ");
    mx_w(b, (2u << 5) | 0x09, 0x80);   /* IRQSel on UART2 */
    mb_expect_pin(b, "IRQ", LOGIC_0, "UART2 pending with IRQSel");
    mx_r(b, (2u << 5) | 0x02);
    mb_expect_pin(b, "IRQ", LOGIC_Z, "ISR read releases IRQ");
    mx_w(b, (2u << 5) | 0x01, 0x00);
    mb_expect_u(mx_r(b, 0x1E), 0x08, "CLKSource resets to 0x08");
    mx_setup(b, 0, 0x18);
    for (u = 1; u < 4; u++) {
        mx_w(b, (u << 5) | 0x0B, 0x03);
        mx_w(b, (u << 5) | 0x1C, 0x01);
    }
    for (u = 0; u < 4; u++) {
        mx_w(b, (u << 5) | 0x00, 0x41 + u);
    }
    mb_run(b, bt * 14);
    for (u = 0; u < 4; u++) {
        check(mb_sent(b, u, log, 4) == 1 && log[0] == 0x41 + u, "each UART sends on its own TX");
    }
    mb_close(b);

    /* MAX14830 I2C: UART2 answers at the table address - 0x40. */
    b = mx_open("SFX-MAX14830-tqfn48", 0, MX_HALF, NULL);
    mb_set(b, "MOSI/A1", LOGIC_0);
    mb_set(b, "CS/A0", LOGIC_0);
    mb_run(b, NS(2000));
    i2c_start(b);
    check(i2c_write(b, (0x6C - 0x40) << 1) == LOGIC_0, "MAX14830 I2C: UART2 at 0x2C (0x58 8-bit)");
    i2c_write(b, 0x14);
    i2c_write(b, 0x99);
    i2c_stop(b);
    i2c_start(b);
    i2c_write(b, 0x6C << 1);
    i2c_write(b, 0x14);
    i2c_restart(b);
    i2c_write(b, (0x6C << 1) | 1);
    v = i2c_read(b, 0, NULL);
    i2c_stop(b);
    mb_expect_u(v, 0x00, "UART0 XON1 untouched");
    i2c_start(b);
    i2c_write(b, (0x6C - 0x40) << 1);
    i2c_write(b, 0x14);
    i2c_restart(b);
    i2c_write(b, ((0x6C - 0x40) << 1) | 1);
    v = i2c_read(b, 0, NULL);
    i2c_stop(b);
    mb_expect_u(v, 0x99, "UART2 XON1 through its own address");
    mb_close(b);

    /* MAX3108 WLP: the ball pinout; I2C with A1 on SCL, A0 on SDA (0xD6: 0x6B). */
    b = mx_open("SFX-MAX3108-wlp25", 0, MX_HALF, MAX3108_TIES);
    check(strcmp(b->part->pins[mb_pin(b, "SPI/I2C")].designator, "D3") == 0, "WLP: SPI/I2C on ball D3");
    check(strcmp(b->part->pins[mb_pin(b, "TX")].designator, "E4") == 0, "WLP: TX on ball E4");
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, NS(2000));
    i2c_start(b);
    check(i2c_write(b, 0x6C << 1) == LOGIC_1, "MAX3108: 0x6C is the DGND/DGND address (NACK)");
    i2c_stop(b);
    i2c_start(b);
    check(i2c_write(b, 0x6B << 1) == LOGIC_0, "MAX3108: A1 = SCL, A0 = SDA answers at 0x6B");
    check(i2c_write(b, 0x1F) == LOGIC_1, "MAX3108: no RevID (0x1F not valid: NACK)");
    i2c_stop(b);
    i2c_start(b);
    i2c_write(b, 0x6B << 1);
    i2c_write(b, 0x1C);
    i2c_restart(b);
    i2c_write(b, (0x6B << 1) | 1);
    v = i2c_read(b, 1, NULL);
    mb_expect_u(v, 0x01, "MAX3108 DIVLSB 0x01");
    v = i2c_read(b, 0, NULL);
    mb_expect_u(v, 0x00, "MAX3108 DIVMSB (auto-increment)");
    i2c_stop(b);
    i2c_start(b);
    i2c_write(b, 0x6B << 1);
    i2c_write(b, 0x1E);
    i2c_restart(b);
    i2c_write(b, (0x6B << 1) | 1);
    v = i2c_read(b, 0, NULL);
    i2c_stop(b);
    mb_expect_u(v, 0x18, "MAX3108 CLKSource resets to 0x18");
    case_end("J6 MAX14830 U1:U0 channels and I2C offsets; MAX3108 WLP balls and its address table");
    mb_close(b);
}


























int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_j3();
    case_j4();
    case_j5();
    case_j6();
    return serial_bench_finish();
}
