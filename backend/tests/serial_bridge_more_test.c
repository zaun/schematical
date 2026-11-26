/*
 * serial_bridge_more_test: plan-68-O coverage of the SC16IS / PI7C9X / XR20M1170
 * bridges (models/ns16550_bridge*.c): loader rejections and the register,
 * GPIO and host-port states serial_bridge_test does not drive. The XR20M1280
 * cases are in serial_xr1280_more_test.c.
 */

#include "serial_bench.h"
#include "serial_bench_maxim.h"
#include "serial_bench_bridge.h"
#include "serial_bench_cov.h"

/* ---- Loader rejections (ns_bridge_load, ns_bridge_bind) ------------------------------- */

static void case_loader(void) {
    case_begin();
    cov_reject("SFX-SC16IS750", "{\"bus.spi.mode\":1}", "the bridges' SPI port is mode 0");
    cov_reject("SFX-SC16IS750", "{\"bus.spi.mode\":4}", "\"mode\" must be a whole number from 0 to 3");
    cov_reject("SFX-SC16IS750", "{\"bus.reset\":5}", "\"reset\"");
    cov_reject("SFX-SC16IS750", "{\"bus.resetActiveHigh\":3}", "\"resetActiveHigh\" must be true or false");
    cov_reject("SFX-SC16IS750", "{\"bus.registerBits\":9}", "\"registerBits\" must be a whole number from 0 to 4");
    cov_reject("SFX-SC16IS750", "{\"bus.registerBits\":2}", "\"registerBits\" is 3 or 4");
    cov_reject("SFX-SC16IS750", "{\"bus.nack\":5}", "\"nack\" must be an array of strings");
    cov_reject("SFX-SC16IS750", "{\"bus.nack\":[\"late\"]}", "\"nack\" entries are \"full\" and \"reset\"");
    cov_reject("SFX-SC16IS750", "{\"bus.sdaTimeout\":-1}", "\"sdaTimeout\" must be a time in ns");
    cov_reject("SFX-SC16IS750", "{\"interrupt\":\"-\"}", "\"interrupt\"");
    cov_reject("SFX-SC16IS750", "{\"gpio.mode\":1}", "unknown key \"mode\"");
    cov_reject("SFX-SC16IS750", "{\"gpio.pins\":[]}", "\"pins\" has 1 to 8 names (GPIO0 first)");
    cov_reject("SFX-SC16IS750", "{\"gpio.registers\":\"pca\"}", "\"registers\" is \"sc16is\" or \"xr1280\"");
    cov_reject("SFX-SC16IS750", "{\"gpio.pullUp\":2}", "\"pullUp\" must be true or false");
    cov_reject("SFX-SC16IS750", "{\"gpio.stateReset\":256}", "\"stateReset\" must be a whole number from 0 to 255");
    cov_reject("SFX-SC16IS750", "{\"fastIrda\":7}", "\"fastIrda\" must be an array of strings");
    cov_reject("SFX-SC16IS750", "{\"dlPowerUp\":1}", "\"dlPowerUp\" is [DLL, DLM], a byte or null each");
    cov_reject("SFX-SC16IS750", "{\"dlPowerUp\":[1,256]}", "\"dlPowerUp\" is [DLL, DLM], a byte or null each");
    cov_reject("SFX-SC16IS750", "{\"dlPowerUp\":[1.5,0]}", "\"dlPowerUp\" is [DLL, DLM], a byte or null each");
    cov_reject("SFX-XR20M1280", "{\"straps.en\":\"EN485\"}", "unknown key \"en\"");
    cov_reject("SFX-XR20M1280", "{\"straps.enir\":4}", "\"enir\"");
    cov_reject("SFX-XR20M1280", "{\"sleepPin\":4}", "\"sleepPin\"");
    cov_reject("SFX-SC16IS750", "{\"extended\":\"650\"}", "an I2C/SPI bridge needs \"extended\"");
    cov_reject("SFX-XR20M1280", "{\"bus.registerBits\":4}", "\"xr1280\" goes with \"registerBits\": 3");
    cov_reject("SFX-SC16IS750", "{\"gpio.registers\":\"xr1280\"}", "\"gpio\" \"registers\" must match \"extended\"");
    cov_reject("SFX-SC16IS750", "{\"sleepPin\":\"GPIO0\"}", "\"straps\" and \"sleepPin\" are XR20M1280 keys");
    cov_reject("SFX-SC16IS752", "{\"channels.2\":{\"tx\":\"TXA\",\"rx\":\"RXA\"}}", "a bridge has 1 or 2 channels");
    cov_reject("SFX-SC16IS750", "{\"interrupt\":\"NOPE\"}", "NOPE");
    cov_reject("SFX-SC16IS750", "{\"gpio.pins.0\":\"NOPE\"}", "NOPE");
    cov_reject("SFX-XR20M1280", "{\"sleepPin\":\"NOPE\"}", "NOPE");
    case_end("bridge loader: every rejection of ns_bridge_load and ns_bridge_bind");
}

static void enhanced_on(Br *r, unsigned ch) {
    br_w(r, ch, 3, 0xBF);
    br_w(r, ch, 2, 0x10);
    br_w(r, ch, 3, 0x03);
}

/* ---- Register decode ------------------------------------------------------------------ */

static void case_decode(void) {
    Br r = br_open("SFX-SC16IS750", 1, 0, NULL);

    case_begin();
    /* LCR 1011111X: maybe 0xBF, maybe not: every address but LCR is X. */
    br_w(&r, 0, 7, 0x5A);
    wx(&r, 0, 3, "1011111X");
    br_expect_text(&r, 0, 3, "1011111X", "LCR keeps the unknown bit");
    br_expect_text(&r, 0, 7, "XXXXXXXX", "LCR maybe 0xBF: SPR or XOFF2 unknown -> X");
    wx(&r, 0, 3, "X0000011");
    br_expect_text(&r, 0, 7, "XXXXXXXX", "LCR[7] X: SPR or DLH-set X");
    /* LCR[7] = 1, not 0xBF: only DLL, DLH and LCR answer. */
    br_w(&r, 0, 3, 0x80);
    br_expect_text(&r, 0, 4, "XXXXXXXX", "LCR 0x80: MCR's address reads X (decisions.md)");
    br_expect(&r, 0, 1, 0x00, "LCR 0x80: DLH answers");
    /* LCR = 0xBF: XON1, XON2, XOFF1 at 4-6; TXLVL's address reads X. */
    br_w(&r, 0, 3, 0xBF);
    br_w(&r, 0, 4, 0x11);
    br_w(&r, 0, 5, 0x12);
    br_w(&r, 0, 6, 0x13);
    br_expect(&r, 0, 4, 0x11, "XON1");
    br_expect(&r, 0, 5, 0x12, "XON2");
    br_expect(&r, 0, 6, 0x13, "XOFF1");
    br_expect_text(&r, 0, 9, "XXXXXXXX", "LCR 0xBF: RXLVL's address reads X");
    br_w(&r, 0, 2, 0x10);
    br_w(&r, 0, 3, 0x03);
    /* EFR[4] = 1 and MCR[2] X: TCR or MSR, unknown. */
    wx(&r, 0, 4, "00000X00");
    br_expect_text(&r, 0, 6, "XXXXXXXX", "MCR[2] X: address 6 (TCR or MSR) reads X");
    br_w(&r, 0, 4, 0x00);
    /* IOIntEna, IOControl, EFCR read back; IOControl[2] is reserved on one channel. */
    br_w(&r, 0, 12, 0x5A);
    br_expect(&r, 0, 12, 0x5A, "IOIntEna read back");
    br_w(&r, 0, 14, 0x07);
    br_expect(&r, 0, 14, 0x03, "IOControl[2] reserved on the SC16IS750 (one channel): reads 0");
    br_w(&r, 0, 14, 0x00);
    br_w(&r, 0, 15, 0x06);
    br_expect(&r, 0, 15, 0x06, "EFCR read back");
    br_w(&r, 0, 15, 0x00);
    /* Re-enabling IER[1] with the THR empty raises no interrupt (p27 note 1). */
    br_w(&r, 0, 1, 0x02);
    br_expect(&r, 0, 2, 0x01, "SC16IS750: IER[1] with THR empty: no THR interrupt");
    mb_close(r.b);
    /* The XR20M1170 does raise one ("If the THR is empty when this bit is enabled"). */
    r = br_open("SFX-XR20M1170", 1, 1, NULL);
    br_w(&r, 0, 1, 0x02);
    br_expect(&r, 0, 2, 0x02, "XR20M1170: IER[1] with THR empty: THR interrupt");
    mb_close(r.b);
    case_end("SC16IS decode: LCR maybe 0xBF, LCR[7] special set, Xon/Xoff at 0xBF, TCR unknown; "
             "IOIntEna/IOControl/EFCR, IER[1] THR rule SC16IS vs XR20M1170");
}

/* ---- PI7C9X special registers ----------------------------------------------------------- */

static void pi_sfr(Br *r, const char *sfr) {
    br_w(r, 0, 3, 0xBF);
    br_w(r, 0, 13, 0x5A);
    wx(r, 0, 7, sfr);
}

static void case_pi_sfr(void) {
    Br r = br_open("SFX-PI7C9X760", 1, 0, NULL);
    MB *b = r.b;

    case_begin();
    pi_sfr(&r, "00000100");
    br_expect(&r, 0, 7, 0x04, "SFR read back");
    br_w(&r, 0, 5, 0x33);
    br_expect(&r, 0, 5, 0x00, "RFD is read-only: the write is ignored (count 0)");
    pi_sfr(&r, "00000000");
    br_expect(&r, 0, 5, 0x00, "and XON2 (address 5 with SFR[2] = 0) was not written");
    pi_sfr(&r, "00000X00");
    br_expect_text(&r, 0, 2, "XXXXXXXX", "SFR[2] X: ASR or EFR, unknown");
    pi_sfr(&r, "0X000100");
    br_expect_text(&r, 0, 5, "XXXXXXXX", "SFR[6] X: RFD or RLS, unknown");
    wx(&r, 0, 13, "0101101X");
    br_expect_text(&r, 0, 7, "XXXXXXXX", "SFREN maybe 0x5A: SFR or XOFF2, unknown");
    /* ISCR: the THR interrupt (TRCTL[2] = 1 by reset) shows as bit 4; CLSTATUS clears it. */
    br_w(&r, 0, 13, 0x00);
    br_w(&r, 0, 3, 0x03);
    br_program(&r, 0, 1);
    br_w(&r, 0, 1, 0x02);
    br_w(&r, 0, 0, 0x55);
    mb_run(b, 14 * BIT1);
    mb_expect_pin(b, "IRQ", LOGIC_0, "THR interrupt after the byte went out: IRQ low");
    pi_sfr(&r, "00000100");
    br_expect(&r, 0, 15, 0x10, "ISCR[4]: THR interrupt active");
    br_w(&r, 0, 15, 0x01);
    br_expect(&r, 0, 15, 0x00, "ISCR[0] CLSTATUS: the status clears");
    br_w(&r, 0, 3, 0x03);
    mb_run(b, NS(2000));
    mb_expect_pin(b, "IRQ", LOGIC_1, "IRQ released by CLSTATUS");
    mb_close(b);
    case_end("PI7C9X: SFR read back, RFD read-only, SFR[2]/SFR[6]/SFREN unknown, ISCR and CLSTATUS");
}

/* ASR: software flow control state (bridges.md § PI7C9X760: 5:4 01 Xoff received,
   11 Xon received; bit 0 the transmitter stopped by the remote; bit 1 we sent
   Xoff or RTS). */
static void case_pi_asr(void) {
    Br r = br_open("SFX-PI7C9X760", 1, 0, NULL);
    MB *b = r.b;
    SimTime bt = BIT1;   /* CPR and TRCTL reset: 16 XIN periods per bit */

    case_begin();
    br_program(&r, 0, 1);
    br_w(&r, 0, 3, 0xBF);
    br_w(&r, 0, 4, 0x11);   /* XON1 */
    br_w(&r, 0, 6, 0x13);   /* XOFF1 */
    br_w(&r, 0, 2, 0x1A);   /* EFR: enhanced, TX Xon1/Xoff1, RX compare Xon1/Xoff1 */
    br_w(&r, 0, 3, 0x03);
    mb_drive_byte(b, "RX", 0x13, bt, 1);
    mb_run(b, 2 * bt);
    pi_sfr(&r, "00000100");
    br_expect(&r, 0, 2, 0x11, "Xoff received: ASR 01 in [5:4], transmitter stopped (bit 0)");
    br_w(&r, 0, 3, 0x03);
    mb_drive_byte(b, "RX", 0x11, bt, 1);
    mb_run(b, 2 * bt);
    pi_sfr(&r, "00000100");
    br_expect(&r, 0, 2, 0x30, "Xon received: ASR 11 in [5:4]");
    /* TX Xoff1 at TCR's halt level (4): ASR 10 in [5:4] (TX off) and bit 1. */
    pi_sfr(&r, "00000000");
    br_w(&r, 0, 13, 0x00);
    br_w(&r, 0, 3, 0x03);
    br_w(&r, 0, 2, 0x01);
    br_w(&r, 0, 4, 0x04);
    br_w(&r, 0, 6, 0x01);
    br_w(&r, 0, 4, 0x00);
    mb_drive_byte(b, "RX", 0x41, bt, 1);
    mb_drive_byte(b, "RX", 0x42, bt, 1);
    mb_drive_byte(b, "RX", 0x43, bt, 1);
    mb_drive_byte(b, "RX", 0x44, bt, 1);
    mb_run(b, 14 * bt);
    pi_sfr(&r, "00000100");
    br_expect(&r, 0, 2, 0x22, "Xoff sent: ASR 10 in [5:4], remote stopped (bit 1)");
    mb_close(b);
    r = br_open("SFX-PI7C9X760", 1, 0, NULL);
    b = r.b;
    br_program(&r, 0, 1);
    /* Auto RTS: RTS off once the RX FIFO reaches TCR's halt level. */
    pi_sfr(&r, "00000000");
    br_w(&r, 0, 13, 0x00);
    br_w(&r, 0, 2, 0x50);   /* EFR: auto RTS, enhanced */
    br_w(&r, 0, 3, 0x03);
    br_w(&r, 0, 2, 0x01);   /* FIFO on */
    br_w(&r, 0, 4, 0x04);   /* MCR[2]: TCR/TLR */
    br_w(&r, 0, 6, 0x01);   /* TCR: halt at 4, resume at 0 */
    br_w(&r, 0, 4, 0x02);   /* RTS on */
    mb_drive_byte(b, "RX", 0x41, bt, 1);
    mb_drive_byte(b, "RX", 0x42, bt, 1);
    mb_drive_byte(b, "RX", 0x43, bt, 1);
    mb_drive_byte(b, "RX", 0x44, bt, 1);
    mb_run(b, 2 * bt);
    mb_expect_pin(b, "RTS", LOGIC_1, "auto RTS: RTS high at the halt level");
    pi_sfr(&r, "00000100");
    br_expect(&r, 0, 2, 0x02, "ASR[1]: we stopped the remote (RTS)");
    mb_close(b);
    case_end("PI7C9X ASR: Xoff received, Xon received, RTS halt");
}

/* ---- GPIO and X ------------------------------------------------------------------------- */

static void case_gpio_x(void) {
    Br r = br_open("SFX-SC16IS752", 1, 0, NULL);
    MB *b = r.b;

    case_begin();
    /* An enabled input at X: the GPIO interrupt is unknown. */
    br_w(&r, 0, 12, 0x01);
    br_r(&r, 0, 11, NULL);
    mb_set(b, "GPIO0/DSRB", LOGIC_X);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_X, "GPIO0 X: IRQ X");
    mb_set(b, "GPIO0/DSRB", LOGIC_Z);
    mb_run(b, NS(1000));
    br_r(&r, 0, 11, NULL);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_1, "back to the read level: no interrupt");
    /* IOLATCH X: nothing pending reads 0, a change makes it X. */
    wx(&r, 0, 14, "0000000X");
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_1, "IOLATCH X, no change: IRQ released");
    mb_set(b, "GPIO0/DSRB", LOGIC_0);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_X, "IOLATCH X and a change: IRQ X");
    br_w(&r, 0, 14, 0x00);
    mb_set(b, "GPIO0/DSRB", LOGIC_Z);
    mb_run(b, NS(1000));
    br_r(&r, 0, 11, NULL);
    /* IOLATCH 1: a pin not enabled in IOIntEna is not latched. */
    br_w(&r, 0, 14, 0x01);
    br_w(&r, 0, 12, 0x00);
    mb_set(b, "GPIO1/DTRB", LOGIC_0);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_1, "IOLATCH 1, IOIntEna 0: no latch, no interrupt");
    mb_expect_u(br_r(&r, 0, 11, NULL) & 0x02u, 0x00, "IOState shows the live level (0)");
    mb_set(b, "GPIO1/DTRB", LOGIC_Z);
    br_w(&r, 0, 14, 0x00);
    /* IOControl[1] X: GPIO5 is DTR A or a GPIO input: X where DTR would drive. */
    br_w(&r, 0, 4, 0x01);
    wx(&r, 0, 14, "000000X0");
    mb_run(b, NS(1000));
    mb_expect_pin(b, "GPIO5/DTRA", LOGIC_X, "IOControl[1] X: DTRA X");
    mb_expect_pin(b, "GPIO4/DSRA", LOGIC_Z, "DSRA (an input either way): Z");
    br_w(&r, 0, 14, 0x00);
    /* IOControl[3] (SRESET) X: the whole device is unknown. */
    br_w(&r, 0, 10, 0x08);
    wx(&r, 0, 14, "0000X000");
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_X, "SRESET X: IRQ X");
    mb_expect_pin(b, "GPIO3/RIB", LOGIC_X, "SRESET X: GPIO outputs X");
    br_expect_text(&r, 0, 7, "XXXXXXXX", "SRESET X: registers read X");
    br_reset(&r);
    br_expect(&r, 0, 3, 0x1D, "the RESET pin clears it");
    mb_close(b);
    /* A package without the GPIO pins: outputs read their latch, inputs X. */
    r = br_open("SFX-XR20M1170-tssop16", 1, 1, NULL);
    br_w(&r, 0, 10, 0x01);
    br_w(&r, 0, 11, 0x01);
    br_expect_text(&r, 0, 11, "XXXXXXX1", "tssop16: GPIO0 output reads its latch, inputs X");
    mb_close(r.b);
    case_end("SC16IS GPIO: input X, IOLATCH X, a disabled pin under IOLATCH, IOControl[1] X, "
             "SRESET X, no GPIO pins in the package");
}

/* ---- The host port in X ------------------------------------------------------------------- */

static void case_host_x(void) {
    Br r = br_open("SFX-SC16IS750", 0, 0, NULL);
    MB *b = r.b;
    char text[40];

    case_begin();
    /* A sub-address with an X register bit: both registers it could name get X. */
    br_w(&r, 0, 7, 0x5A);
    i2c_start(b);
    i2c_write(b, r.addr << 1);
    check(i2c_bits(b, "0X111000") == LOGIC_0, "sub-address byte ACKed");
    check(i2c_bits(b, "00010010") == LOGIC_X, "data with an unknown target register: ACK X");
    i2c_stop(b);
    br_expect_text(&r, 0, 7, "XXXXXXXX", "SPR (register 7) X");
    br_expect_text(&r, 0, 15, "XXXXXXXX", "EFCR (register 15) X");
    br_w(&r, 0, 15, 0x00);
    /* Address byte with an X bit: X if it could match, NACK if another bit differs. */
    i2c_start(b);
    check(i2c_bits(b, "1001X000") == LOGIC_X, "0x48 with bit 3 X: ACK X");
    i2c_stop(b);
    i2c_start(b);
    check(i2c_bits(b, "0001X000") == LOGIC_1, "bit 7 wrong and bit 3 X: NACK");
    i2c_stop(b);
    /* The A0 strap X: the address is unknown. */
    mb_set(b, r.a0, LOGIC_X);
    mb_run(b, NS(2000));
    i2c_start(b);
    check(i2c_write(b, r.addr << 1) == LOGIC_X, "A0 X: ACK X");
    i2c_stop(b);
    mb_set(b, r.a0, LOGIC_1);
    mb_run(b, NS(2000));
    /* Device unknown (SRESET X): data bytes ACK X. */
    i2c_start(b);
    i2c_write(b, r.addr << 1);
    i2c_write(b, br_sub(14, 0));
    i2c_bits(b, "0000X000");
    i2c_stop(b);
    check(br_w(&r, 0, 7, 0x11) == LOGIC_X, "device unknown: data ACK X");
    mb_close(b);
    /* A table entry of null: no address for that strap combination. */
    r = br_open(cov_part("SFX-SC16IS750", "{\"bus.i2c.table.5\":null}"), 0, 0, NULL);
    i2c_start(r.b);
    check(i2c_write(r.b, 0x48 << 1) == LOGIC_1, "A1, A0 high with a null entry: NACK");
    i2c_stop(r.b);
    mb_close(r.b);
    /* SPI: the R/W bit X: a read drives X; data bytes may have been written. */
    r = br_open("SFX-SC16IS750", 1, 0, NULL);
    br_w(&r, 0, 7, 0x5A);
    spx(r.b, "X0111000 00000000", text);
    check(strcmp(text + 8, "XXXXXXXX") == 0, "R/W X: SO drives X for the data byte");
    br_expect_text(&r, 0, 7, "XXXXXXXX", "and SPR, which it may have written, is X");
    mb_close(r.b);
    case_end("host port X: sub-address X, address byte X, strap X, device X, null table entry, "
             "SPI R/W X");
}

/* ---- Transmit paths -------------------------------------------------------------------- */

static void case_pi_tx(void) {
    Br r = br_open("SFX-PI7C9X760", 1, 0, NULL);
    MB *b = r.b;
    SimTime bt = BIT1 * 20 / 16;
    uint16_t frames[4];

    case_begin();
    br_program(&r, 0, 1);
    /* MCR[3:2] need EFR[4] (bridges.md § PI7C9X760). */
    br_w(&r, 0, 4, 0x0C);
    br_expect(&r, 0, 4, 0x00, "MCR[3:2] without EFR[4]: not written");
    enhanced_on(&r, 0);
    br_w(&r, 0, 4, 0x0C);
    br_expect(&r, 0, 4, 0x0C, "with EFR[4]: written");
    br_w(&r, 0, 4, 0x00);
    /* SFR[1] auto DSR/DTR: DTR follows the RTS rule (active), TX waits for DSR. */
    br_w(&r, 0, 14, 0x02);   /* GPIO7..4 are the modem pins */
    mb_set(b, "GPIO4/DSR", LOGIC_1);
    pi_sfr(&r, "00000010");
    br_w(&r, 0, 3, 0x03);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "GPIO5/DTR", LOGIC_0, "auto DTR: DTR active (low) with MCR[0] = 0");
    br_w(&r, 0, 0, 0x5A);
    mb_run(b, 14 * bt);
    check(mb_sent(b, 0, frames, 4) == 0, "auto DSR: DSR inactive, nothing sent");
    /* SFR[0]: transmit regardless of flow control. */
    pi_sfr(&r, "00000011");
    br_w(&r, 0, 3, 0x03);
    mb_run(b, 14 * bt);
    check(mb_sent(b, 0, frames, 4) == 1 && frames[0] == 0x5A, "SFR[0]: the byte goes out");
    /* SFR[3]: internal loopback RX -> TX. */
    pi_sfr(&r, "00001000");
    br_w(&r, 0, 3, 0x03);
    mb_set(b, "RX", LOGIC_0);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "TX", LOGIC_0, "SFR[3]: TX follows RX (low)");
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "TX", LOGIC_1, "SFR[3]: TX follows RX (high)");
    /* CPR M = 0 is undefined: the generator stops (P2), nothing is sent. */
    pi_sfr(&r, "00000100");
    br_w(&r, 0, 4, 0x00);
    br_w(&r, 0, 7, 0x00);
    br_w(&r, 0, 3, 0x03);
    br_w(&r, 0, 0, 0x33);
    mb_run(b, 20 * bt);
    check(mb_sent(b, 0, frames, 4) == 1, "CPR M = 0: no frame");
    mb_close(b);
    case_end("PI7C9X transmit: MCR mask, auto DTR/DSR, SFR[0] force, SFR[3] loopback, CPR M = 0");
}

static void case_rts_irda(void) {
    Br r = br_open("SFX-SC16IS750", 1, 0, NULL);
    MB *b = r.b;
    long long w;

    case_begin();
    /* EFCR[4] or [5] X: RTS X. */
    wx(&r, 0, 15, "000X0000");
    mb_run(b, NS(1000));
    mb_expect_pin(b, "RTS", LOGIC_X, "RTSCON X: RTS X");
    wx(&r, 0, 15, "00X10000");
    mb_run(b, NS(1000));
    mb_expect_pin(b, "RTS", LOGIC_X, "RTSINVER X: RTS X");
    br_w(&r, 0, 15, 0x00);
    /* TLR = 1: the TX trigger is 4 spaces (TLR[3:0] x 4). */
    br_program(&r, 0, 1);
    enhanced_on(&r, 0);
    br_w(&r, 0, 2, 0x01);
    br_w(&r, 0, 4, 0x04);
    br_w(&r, 0, 7, 0x01);
    br_w(&r, 0, 4, 0x00);
    br_w(&r, 0, 15, 0x04);
    {
        unsigned i;

        for (i = 0; i < 62; i++) {
            br_w(&r, 0, 0, i);
        }
    }
    br_w(&r, 0, 1, 0x02);
    br_expect(&r, 0, 8, 2, "TXLVL 2 (TX held by EFCR[2])");
    br_w(&r, 0, 15, 0x00);
    check(mb_wait(b, "IRQ", LOGIC_0, NS(200000), NS(200)) >= 0, "the THR interrupt comes as TX drains");
    br_expect(&r, 0, 8, 4, "at 4 spaces (TLR = 1 -> 4), not the FCR trigger of 8");
    br_w(&r, 0, 2, 0x07);
    br_w(&r, 0, 1, 0x00);
    mb_run(b, 70 * 10 * BIT1);
    /* IrDA (MCR[6]): the SC16IS750 grade has no fast mode: 3/16 pulses even with EFCR[7]. */
    br_w(&r, 0, 4, 0x40);
    br_w(&r, 0, 15, 0x80);
    br_w(&r, 0, 0, 0x00);
    mb_wait(b, "TX", LOGIC_1, NS(100000), NS(20));
    w = br_width(b, "TX", LOGIC_1, NS(2000));
    expect_near(w, BIT1 * 3 / 16, NS(60), "SC16IS750 IrDA pulse: 3/16 bit");
    mb_close(b);
    /* The SC16IS760 grade: EFCR[7] = 1 gives 1/4-bit pulses. */
    r = br_open("SFX-SC16IS750-hvqfn24-760", 1, 0, NULL);
    b = r.b;
    br_program(&r, 0, 1);
    enhanced_on(&r, 0);
    br_w(&r, 0, 4, 0x40);
    br_w(&r, 0, 15, 0x80);
    br_w(&r, 0, 0, 0x00);
    mb_wait(b, "TX", LOGIC_1, NS(100000), NS(20));
    w = br_width(b, "TX", LOGIC_1, NS(2000));
    expect_near(w, BIT1 / 4, NS(60), "SC16IS760 IrDA, EFCR[7] = 1: 1/4-bit pulse");
    mb_close(b);
    /* XR20M1170 auto RS-485 with the FIFO off: the THR interrupt waits for the TSR. */
    r = br_open("SFX-XR20M1170", 1, 1, NULL);
    b = r.b;
    br_program(&r, 0, 1);
    br_w(&r, 0, 15, 0x10);
    br_w(&r, 0, 1, 0x02);
    br_r(&r, 0, 2, NULL);
    br_w(&r, 0, 0, 0x55);
    mb_run(b, 3 * BIT1);
    br_expect(&r, 0, 2, 0x01, "RS-485, FIFO off: no THR interrupt while the byte shifts out");
    mb_run(b, 10 * BIT1);
    br_expect(&r, 0, 2, 0x02, "THR interrupt once the transmitter is empty");
    mb_close(b);
    case_end("EFCR RTS X, TLR trigger, IrDA 3/16 vs the 760's 1/4 pulse, XR20M1170 RS-485 THR");
}

int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_loader();
    case_decode();
    case_pi_sfr();
    case_pi_asr();
    case_gpio_x();
    case_host_x();
    case_pi_tx();
    case_rts_irda();
    return serial_bench_finish();
}
