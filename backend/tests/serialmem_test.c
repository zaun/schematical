/*
 * serialmem_test: split from serialmem_test.c by plan-68-H; the shared helpers are
 * support/serialmem_bench.c.
 */

#include "serialmem_bench.h"

/* ---- 4-Kbit EEPROM (E40) ------------------------------------------------------------------- */

static void e40_basic(unsigned mode) {
    Rig r = rig_open("FX-E40", mode);
    Rx rx;
    SPI(&r, &rx, 4, 0x03, 0x00);
    expect(rx_is(&rx, 0, 0xFF) && rx_is(&rx, 3, 0xFF), "E40 powers up erased: READ gives FF FF FF FF");
    expect(rdsr(&r) == 0x00, "E40 power-up status 00 (RDY 0, WEL 0, BP 00)");
    expect(bench_get(r.b, "SO") == LOGIC_Z, "SO is Z while CS# is high");

    /* WRITE without WREN is ignored. */
    SEND(&r, 0x02, 0x10, 0xAB);
    wait_us(&r, 5200);
    expect(mem_is(&r, 0x10, 0xFF) && rdsr(&r) == 0x00, "a WRITE without WREN stores nothing and starts no cycle");

    /* WREN, byte write, busy. */
    SEND(&r, 0x06);
    expect(rdsr(&r) == 0x02, "WREN sets WEL (status 02)");
    SEND(&r, 0x02, 0x10, 0xA5);
    expect(rdsr(&r) == 0xF3, "during the write cycle RDSR reads RDY 1, WEL 1 and the busy ones (F3)");
    SPI(&r, &rx, 1, 0x03, 0x10);
    expect(rx_z(&rx, 0), "READ is ignored during the write cycle (SO stays Z)");
    SEND(&r, 0x02, 0x11, 0x77);
    SEND(&r, 0x04);
    expect(rdsr(&r) == 0xF3, "WRITE and WRDI during the cycle are ignored");
    wait_us(&r, 4900);
    expect(rdsr(&r) == 0xF3, "still busy 4.9 ms after the write started");
    wait_us(&r, 200);
    expect(rdsr(&r) == 0x00, "the cycle ends after writeCycle (5 ms): RDY 0 and WEL 0");
    SPI(&r, &rx, 2, 0x03, 0x10);
    expect(rx_is(&rx, 0, 0xA5), "the byte is stored");
    SPI(&r, &rx, 1, 0x03, 0x11);
    expect(rx_is(&rx, 0, 0xFF), "the WRITE sent during the cycle was ignored");

    /* A8 in the opcode. */
    SEND(&r, 0x06);
    SEND(&r, 0x0A, 0xA5, 0x3C);
    wait_us(&r, 5100);
    SPI(&r, &rx, 1, 0x0B, 0xA5);
    expect(rx_is(&rx, 0, 0x3C), "opcode 0A/0B carries address bit 8: byte 1A5 written and read");
    SPI(&r, &rx, 1, 0x03, 0xA5);
    expect(rx_is(&rx, 0, 0xFF), "opcode 03 with the same address byte reads 0A5 (still erased)");

    /* Sequential read wraps at the end of the array. */
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x11);
    wait_us(&r, 5100);
    SPI(&r, &rx, 4, 0x0B, 0xFE);
    expect(rx_is(&rx, 0, 0xFF) && rx_is(&rx, 1, 0xFF) && rx_is(&rx, 2, 0x11) && rx_is(&rx, 3, 0xFF),
           "a sequential READ from 1FE wraps from 1FF to 000");
    rig_close(&r);
}

static void e40_pages_protect(void) {
    Rig r = rig_open("FX-E40", 0);
    Rx rx;
    int ok = 1;
    unsigned i;
    static const uint8_t expected[8] = {0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};

    /* Page wrap: 10 bytes from offset 6 of the page at 0x40. */
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x46, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A);
    wait_us(&r, 5100);
    for (i = 0; i < 8; i++) {
        ok &= mem_is(&r, 0x40 + i, expected[i]);
    }
    expect(ok && mem_is(&r, 0x48, 0xFF) && mem_is(&r, 0x3F, 0xFF),
           "a page write wraps inside its 8-byte page; later bytes overwrite earlier ones");
    SPI(&r, &rx, 8, 0x03, 0x40);
    ok = 1;
    for (i = 0; i < 8; i++) {
        ok &= rx_is(&rx, i, expected[i]);
    }
    expect(ok, "the page reads back over the SPI");

    /* Block protect: BP0 = 1 protects the upper quarter (180-1FF). */
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x04);
    expect(rdsr(&r) == 0xF3, "WRSR starts a write cycle too");
    wait_us(&r, 5100);
    expect(rdsr(&r) == 0x04, "BP0 is set; WEL cleared by the end of the WRSR cycle");
    SEND(&r, 0x06);
    SEND(&r, 0x0A, 0xA0, 0x55);
    wait_us(&r, 5100);
    expect(mem_is(&r, 0x1A0, 0xFF) && rdsr(&r) == 0x06,
           "a write into the protected quarter is ignored (no cycle; WEL stays 1)");
    SEND(&r, 0x0A, 0x7F, 0x66);
    wait_us(&r, 5100);
    expect(mem_is(&r, 0x17F, 0x66) && rdsr(&r) == 0x04, "a write just below the quarter is stored");
    /* BP = 2: upper half. */
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0xFF);
    wait_us(&r, 5100);
    expect(rdsr(&r) == 0x0C, "WRSR writes only BP1, BP0 (the other bits read 0): FF gives 0C");
    SEND(&r, 0x06);
    SEND(&r, 0x0A, 0x00, 0x77);
    wait_us(&r, 5100);
    expect(mem_is(&r, 0x100, 0xFF) && rdsr(&r) == 0x0E, "BP = 3 protects the whole array");
    SEND(&r, 0x01, 0x08);
    wait_us(&r, 5100);
    SEND(&r, 0x06);
    SEND(&r, 0x0A, 0x00, 0x77);
    wait_us(&r, 5100);
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0xFF, 0x88);
    wait_us(&r, 5100);
    expect(rdsr(&r) == 0x08 && mem_is(&r, 0x100, 0xFF) && mem_is(&r, 0x0FF, 0x88),
           "BP = 2 protects 100-1FF only: 0FF is written, 100 is not");
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x00);
    wait_us(&r, 5100);
    expect(rdsr(&r) == 0x00, "WRSR 00 unprotects");
    rig_close(&r);
}

static void e40_odds(void) {
    Rig r = rig_open("FX-E40", 0);
    Rx rx;
    static const uint8_t cut[] = {0x02, 0x20, 0x5A, 0x80};
    static const uint8_t wren_plus[] = {0x06, 0x80};

    /* WP# low blocks WREN, and a write interrupted by WP#. */
    bench_set(r.b, "WP#", LOGIC_0);
    bench_run(r.b, HALF);
    SEND(&r, 0x06);
    expect(rdsr(&r) == 0x00, "WREN is ignored while WP# is low (protection \"wp\")");
    bench_set(r.b, "WP#", LOGIC_1);
    bench_run(r.b, HALF);
    SEND(&r, 0x06);
    bench_set(r.b, "WP#", LOGIC_0);
    bench_run(r.b, HALF);
    SEND(&r, 0x02, 0x20, 0x5A);
    wait_us(&r, 5100);
    expect(mem_is(&r, 0x20, 0xFF) && rdsr(&r) == 0x02, "a WRITE with WP# low is dropped (WEL stays)");
    bench_set(r.b, "WP#", LOGIC_1);
    bench_run(r.b, HALF);
    /* WP# pulses low in the middle of a transaction. */
    cs_low(&r);
    byte_clock(&r, 0x02, NULL, 0);
    byte_clock(&r, 0x20, NULL, 0);
    bench_set(r.b, "WP#", LOGIC_0);
    bench_run(r.b, HALF);
    bench_set(r.b, "WP#", LOGIC_1);
    bench_run(r.b, HALF);
    byte_clock(&r, 0x5A, NULL, 0);
    cs_high(&r);
    wait_us(&r, 5100);
    expect(mem_is(&r, 0x20, 0xFF) && rdsr(&r) == 0x02,
           "WP# falling between CS# falling and rising drops the WRITE");

    /* CS# timing: a cut byte, no data, extra bits. */
    spi_bits(&r, cut, 8 + 8 + 7);
    wait_us(&r, 5100);
    expect(mem_is(&r, 0x20, 0xFF) && rdsr(&r) == 0x02,
           "CS# rising 1 bit short of the data byte drops the WRITE (no cycle, WEL stays)");
    SEND(&r, 0x02, 0x20);
    expect(rdsr(&r) == 0x02, "a WRITE with no data byte starts no cycle");
    SEND(&r, 0x04);
    expect(rdsr(&r) == 0x00, "WRDI clears WEL");
    spi_bits(&r, wren_plus, 9);
    expect(rdsr(&r) == 0x00, "WREN followed by a stray bit (CS# off a byte boundary) is ignored");
    SEND(&r, 0x0E);
    expect(rdsr(&r) == 0x02, "bit 3 of the WREN opcode is a don't care (0E)");
    SEND(&r, 0x0C);
    expect(rdsr(&r) == 0x00, "bit 3 of the WRDI opcode is a don't care (0C)");

    /* Invalid opcode. */
    SPI(&r, &rx, 2, 0x9F);
    expect(rx_z(&rx, 0) && rx_z(&rx, 1), "an unknown opcode: SO stays Z");
    SPI(&r, &rx, 1, 0x05);
    expect(rx_is(&rx, 0, 0x00), "...and nothing changed");
    /* RDSR runs on: the same byte again. */
    SPI(&r, &rx, 3, 0x05);
    expect(rx_is(&rx, 0, 0x00) && rx_is(&rx, 1, 0x00) && rx_is(&rx, 2, 0x00),
           "RDSR repeats the status byte while CS# stays low");
    rig_close(&r);
}

static void e40_hold(void) {
    Rig r = rig_open("FX-E40", 0);
    Rx rx;
    unsigned i;
    uint8_t got = 0;
    int z_in_hold = 1;

    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0xC6);
    wait_us(&r, 5100);
    cs_low(&r);
    byte_clock(&r, 0x03, NULL, 0);
    byte_clock(&r, 0x00, NULL, 0);
    for (i = 0; i < 4; i++) {
        LogicValue so = bit_clock(&r, LOGIC_0);

        got = (uint8_t)((got << 1) | (so == LOGIC_1));
    }
    bench_set(r.b, "HOLD#", LOGIC_0);
    bench_run(r.b, HALF);
    for (i = 0; i < 4; i++) {
        z_in_hold &= bit_clock(&r, LOGIC_1) == LOGIC_Z;
    }
    bench_set(r.b, "HOLD#", LOGIC_1);
    bench_run(r.b, HALF);
    for (i = 0; i < 4; i++) {
        LogicValue so = bit_clock(&r, LOGIC_0);

        got = (uint8_t)((got << 1) | (so == LOGIC_1));
    }
    cs_high(&r);
    expect(z_in_hold && got == 0xC6, "HOLD# low with SCK low pauses the read: SO Z, clocks ignored; C6 read intact");
    SPI(&r, &rx, 2, 0x03, 0x00);
    expect(rx_is(&rx, 0, 0xC6), "the chip is back in order after the hold");

    /* HOLD# low at CS# fall: held at once; an X on HOLD# during a transaction makes it X. */
    cs_low(&r);
    byte_clock(&r, 0x05, NULL, 0);
    bench_set(r.b, "HOLD#", LOGIC_X);
    bench_run(r.b, HALF);
    {
        LogicValue so = bit_clock(&r, LOGIC_0);

        expect(so == LOGIC_X, "an X on HOLD# makes the open transaction X (SO drives X)");
    }
    bench_set(r.b, "HOLD#", LOGIC_1);
    bench_run(r.b, HALF);
    cs_high(&r);
    rig_close(&r);
}

static void e40_x(void) {
    Rig r = rig_open("FX-E40", 0);
    Rx rx;
    int i;

    /* An opcode with an unknown bit: SO drives X, WEL becomes unknown. */
    cs_low(&r);
    bit_clock(&r, LOGIC_X);
    for (i = 6; i >= 0; i--) {
        bit_clock(&r, (0x06 >> i) & 1 ? LOGIC_1 : LOGIC_0);
    }
    byte_clock(&r, 0, &rx, 0);
    cs_high(&r);
    expect(rx_x(&rx, 0), "after an opcode with an unknown bit SO drives X");
    rx = rdsr_rx(&r);
    expect(rx.x[0] == 0x02 && (rx.v[0] & 0xFD) == 0 && rx.z[0] == 0,
           "...and WEL reads X (status bit 1 unknown, the rest 0)");
    SEND(&r, 0x02, 0x30, 0x55);
    wait_us(&r, 5100);
    expect(mem_unknown(&r, 0x30) && mem_is(&r, 0x31, 0xFF) && rdsr(&r) == 0x00,
           "a WRITE with WEL unknown stores X; WEL is 0 after the cycle");
    SEND(&r, 0x06);

    /* An address with an unknown bit: reads X; a write corrupts what it could name. */
    cs_low(&r);
    byte_clock(&r, 0x03, NULL, 0);
    for (i = 7; i >= 0; i--) {
        bit_clock(&r, i == 4 ? LOGIC_X : LOGIC_0);
    }
    byte_clock(&r, 0, &rx, 0);
    byte_clock(&r, 0, &rx, 1);
    cs_high(&r);
    expect(rx_x(&rx, 0) && rx_x(&rx, 1), "a READ with an unknown address bit reads X");
    cs_low(&r);
    byte_clock(&r, 0x02, NULL, 0);
    for (i = 7; i >= 0; i--) {
        bit_clock(&r, i == 4 ? LOGIC_X : (i == 5 ? LOGIC_1 : LOGIC_0));
    }
    byte_clock(&r, 0x11, NULL, 0);
    cs_high(&r);
    wait_us(&r, 5100);
    expect(mem_unknown(&r, 0x20) && mem_unknown(&r, 0x27) && mem_unknown(&r, 0x30) &&
               mem_unknown(&r, 0x37) && mem_is(&r, 0x28, 0xFF) && mem_is(&r, 0x38, 0xFF),
           "a WRITE with an unknown address bit makes the pages it could name unknown");

    /* Unknown data bit. */
    SEND(&r, 0x06);
    cs_low(&r);
    byte_clock(&r, 0x02, NULL, 0);
    byte_clock(&r, 0x50, NULL, 0);
    for (i = 7; i >= 0; i--) {
        bit_clock(&r, i == 3 ? LOGIC_X : LOGIC_1);
    }
    cs_high(&r);
    wait_us(&r, 5100);
    {
        uint8_t v, k;

        expect(mem(&r, 0x50, &v, &k) && k == 0xF7 && (v & 0xF7) == 0xF7,
               "an unknown data bit is stored as an unknown bit");
    }

    /* CS# going X mid-transaction: the write may not have happened. */
    SEND(&r, 0x06);
    cs_low(&r);
    byte_clock(&r, 0x02, NULL, 0);
    byte_clock(&r, 0x60, NULL, 0);
    byte_clock(&r, 0x3E, NULL, 0);
    bench_set(r.b, "CS#", LOGIC_X);
    bench_run(r.b, HALF);
    bench_set(r.b, "CS#", LOGIC_1);
    bench_run(r.b, HALF);
    wait_us(&r, 5100);
    expect(mem_unknown(&r, 0x60), "an X on CS# during a WRITE leaves the byte unknown");
    rig_close(&r);
}

/* ---- 32-Kbit EEPROM with WPEN (E320) -------------------------------------------------------- */

static void e320_cases(unsigned mode) {
    Rig r = rig_open("FX-E320", mode);
    Rx rx;

    SPI(&r, &rx, 2, 0x03, 0x00, 0x00);
    expect(rx_is(&rx, 0, 0xFF) && rx_is(&rx, 1, 0xFF), "E320 powers up erased");
    /* Two address bytes; the upper bits (above 4096) are ignored. */
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0xF0, 0x10, 0x42);
    expect(rdsr(&r) == 0xFF, "E320: RDSR is all ones during the write cycle (busyOnes 255)");
    wait_us(&r, 5100);
    SPI(&r, &rx, 1, 0x03, 0x00, 0x10);
    expect(rx_is(&rx, 0, 0x42), "address bits above the array are ignored (F010 = 010)");
    /* Page wrap at 32 bytes. */
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x01, 0x1E, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4);
    wait_us(&r, 5100);
    expect(mem_is(&r, 0x11E, 0xA0) && mem_is(&r, 0x11F, 0xA1) && mem_is(&r, 0x100, 0xA2) &&
               mem_is(&r, 0x101, 0xA3) && mem_is(&r, 0x102, 0xA4) && mem_is(&r, 0x120, 0xFF),
           "E320: a page write wraps inside its 32-byte page");

    /* WPEN: WRSR sets it; WP# low then blocks WRSR but not the array. */
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x80);
    wait_us(&r, 5100);
    expect(rdsr(&r) == 0x80, "E320: WRSR 80 sets WPEN");
    bench_set(r.b, "WP#", LOGIC_0);
    bench_run(r.b, HALF);
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x04);
    wait_us(&r, 5100);
    expect(rdsr(&r) == 0x82, "E320: with WPEN 1 and WP# low, WRSR is ignored (WEL stays 1)");
    SEND(&r, 0x02, 0x00, 0x05, 0x99);
    wait_us(&r, 5100);
    expect(mem_is(&r, 0x05, 0x99) && rdsr(&r) == 0x80, "E320: WP# low does not stop an array write");
    bench_set(r.b, "WP#", LOGIC_1);
    bench_run(r.b, HALF);
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x04);
    wait_us(&r, 5100);
    expect(rdsr(&r) == 0x04, "E320: with WP# high WRSR works, and clears WPEN");
    bench_set(r.b, "WP#", LOGIC_0);
    bench_run(r.b, HALF);
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x08);
    wait_us(&r, 5100);
    expect(rdsr(&r) == 0x08, "E320: with WPEN 0 WP# low does not block WRSR");
    /* BP = 2 protects the upper half (800-FFF). */
    bench_set(r.b, "WP#", LOGIC_1);
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x0A, 0x00, 0x01);
    wait_us(&r, 5100);
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x07, 0xFF, 0x02);
    wait_us(&r, 5100);
    expect(mem_is(&r, 0x7FF, 0x02) && mem_is(&r, 0xA00, 0xFF),
           "E320: BP = 2 protects 800-FFF: 7FF is written, A00 is not");
    /* An unknown WP# with WPEN 1: the WRSR is a maybe. */
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x80);
    wait_us(&r, 5100);
    bench_set(r.b, "WP#", LOGIC_X);
    bench_run(r.b, HALF);
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x00);
    wait_us(&r, 5100);
    rx = rdsr_rx(&r);
    expect((rx.x[0] & 0x8C) == 0x8C && rx.z[0] == 0,
           "E320: WRSR with WPEN 1 and WP# unknown leaves WPEN and BP unknown");
    rig_close(&r);
}

/* ---- F-RAM -------------------------------------------------------------------------------- */

static void fram_cases(unsigned mode) {
    Rig r = rig_open("FX-FR", mode);
    Rx rx;

    SPI(&r, &rx, 2, 0x03, 0x00, 0x10);
    expect(rx_x(&rx, 0) && rx_x(&rx, 1), "F-RAM powers up unknown: READ gives X");
    expect(rdsr(&r) == 0x40, "F-RAM status 40: bit 6 reads 1 (statusOnes), RDY always 0");
    SEND(&r, 0x02, 0x01, 0x00, 0x77);
    expect(mem_unknown(&r, 0x100), "a WRITE without WREN stores nothing");
    SEND(&r, 0x06);
    expect(rdsr(&r) == 0x42, "WREN sets WEL (42)");
    SEND(&r, 0x02, 0x01, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55);
    expect(rdsr(&r) == 0x40, "no write cycle: status 40 at once, WEL cleared by the WRITE");
    SPI(&r, &rx, 5, 0x03, 0x01, 0x00);
    expect(rx_is(&rx, 0, 0x11) && rx_is(&rx, 1, 0x22) && rx_is(&rx, 4, 0x55),
           "the bytes are stored at bus speed and read back");
    SEND(&r, 0x02, 0x01, 0x00, 0x99);
    expect(mem_is(&r, 0x100, 0x11), "a second WRITE needs a new WREN");
    /* No page limit: write across 256. */
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x01, 0xFE, 0xA1, 0xA2, 0xA3, 0xA4);
    expect(mem_is(&r, 0x1FE, 0xA1) && mem_is(&r, 0x1FF, 0xA2) && mem_is(&r, 0x200, 0xA3) &&
               mem_is(&r, 0x201, 0xA4),
           "an F-RAM burst has no page boundary");
    /* Wrap at the end of the array (2048). */
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x07, 0xFF, 0xB1, 0xB2);
    expect(mem_is(&r, 0x7FF, 0xB1) && mem_is(&r, 0x000, 0xB2), "the address wraps from 7FF to 000");
    SPI(&r, &rx, 2, 0x03, 0x07, 0xFF);
    expect(rx_is(&rx, 0, 0xB1) && rx_is(&rx, 1, 0xB2), "...and a READ wraps likewise");
    /* Block protect: a burst that reaches the protected block stops. */
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x04);
    expect(rdsr(&r) == 0x44, "WRSR applies at once (BP0)");
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x05, 0xFE, 0xC1, 0xC2, 0xC3, 0xC4);
    expect(mem_is(&r, 0x5FE, 0xC1) && mem_is(&r, 0x5FF, 0xC2) && mem_unknown(&r, 0x600) &&
               mem_unknown(&r, 0x601),
           "a burst reaching the protected quarter (600-7FF) stops there");
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x06, 0x10, 0xD1);
    expect(mem_unknown(&r, 0x610) && rdsr(&r) == 0x44, "a write into the protected quarter is ignored; WEL cleared");
    /* WPEN. */
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x84);
    expect(rdsr(&r) == 0xC4, "WPEN set");
    bench_set(r.b, "WP#", LOGIC_0);
    bench_run(r.b, HALF);
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x00);
    expect(rdsr(&r) == 0xC6, "WPEN with WP# low: WRSR ignored (WEL stays)");
    SEND(&r, 0x04);
    expect(rdsr(&r) == 0xC4, "WRDI clears WEL");
    bench_set(r.b, "WP#", LOGIC_1);
    bench_run(r.b, HALF);
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x00);
    expect(rdsr(&r) == 0x40, "with WP# high WRSR works");

    /* RDID: nine bytes, then SO Z. */
    SPI(&r, &rx, 11, 0x9F);
    expect(rx_is(&rx, 0, 0x7F) && rx_is(&rx, 5, 0x7F) && rx_is(&rx, 6, 0xC2) &&
               rx_is(&rx, 7, 0x25) && rx_is(&rx, 8, 0xC8) && rx_z(&rx, 9) && rx_z(&rx, 10),
           "RDID gives 7F7F7F7F7F7FC225C8, then SO Z");
    /* FSTRD: a dummy byte before the data. */
    SPI(&r, &rx, 3, 0x0B, 0x01, 0x00, 0x00);
    expect(rx_is(&rx, 0, 0x11) && rx_is(&rx, 1, 0x22), "FSTRD: address, a dummy byte, then data");
    /* SLEEP. */
    SEND(&r, 0xB9);
    SPI(&r, &rx, 1, 0x03, 0x01, 0x00);
    expect(rx_z(&rx, 0), "the transaction after SLEEP is the wake-up and is ignored");
    SPI(&r, &rx, 1, 0x03, 0x01, 0x00);
    expect(rx_z(&rx, 0), "...and so is every transaction within tREC");
    wait_us(&r, 500);
    SPI(&r, &rx, 1, 0x03, 0x01, 0x00);
    expect(rx_is(&rx, 0, 0x11), "after the wake time the chip answers again");
    /* Unknown opcodes (DPD is a flash command). */
    SPI(&r, &rx, 1, 0x9E);
    expect(rx_z(&rx, 0), "an unknown opcode is ignored");
    rig_close(&r);
}

/* ---- Facts, listing, circuit, unsimulated devices -------------------------------------------- */

static void fact_cases(void) {
    const Part *p = part_library_find(library, "FX-E40");
    const Part *f = part_library_find(library, "FX-FL");
    const Part *d = part_library_find(library, "FX-DF");
    const Part *e = part_library_find(library, "FX-PE");
    char *line;

    expect(p != NULL && p->kind == PART_SERIALMEM && p->serialmem != NULL && p->simulated == 1 &&
               strcmp(p->serialmem->type, "eeprom") == 0 && p->serialmem->size == 512 &&
               p->serialmem->address_bytes == 1 && p->serialmem->page_bytes == 8 &&
               p->serialmem->write_ps == 5000000000ULL && p->serialmem->endurance == 1000000 &&
               p->serialmem->retention_years == 100 && p->serialmem->jedec_id == NULL,
           "FX-E40 facts: eeprom, 512 bytes, 1 address byte, 8-byte page, 5 ms write, endurance, retention");
    expect(p != NULL && p->serialmem->max_clock_khz == 5000 && p->vcc != NULL && p->vcc->min_mv == 1800,
           "FX-E40: the part's maxClock (5 MHz) overrides the device's (10 MHz); vcc from the part");
    expect(f != NULL && strcmp(f->serialmem->type, "flash") == 0 && f->serialmem->size == 65536 &&
               f->serialmem->page_bytes == 256 && f->serialmem->address_bytes == 3 &&
               f->serialmem->write_ps == 2000000000ULL &&
               f->serialmem->sector_erase_ps == 45000000000ULL &&
               f->serialmem->chip_erase_ps == 2400000000000ULL &&
               strcmp(f->serialmem->jedec_id, "1F430100") == 0,
           "FX-FL facts: flash, 64 KB, 256-byte page, page program 2 ms, 4K erase 45 ms, chip erase 2.4 s, id");
    expect(d != NULL && d->simulated == 0 && strcmp(d->serialmem->type, "dataflash") == 0 &&
               d->serialmem->size == 2162688 && d->serialmem->page_bytes == 528 &&
               d->serialmem->max_clock_khz == 85000 && d->serialmem->address_bytes == 0 &&
               e != NULL && e->simulated == 0 && strcmp(e->serialmem->type, "pageeeprom") == 0,
           "the dataflash and pageeeprom fixtures load as facts with simulated 0");
    line = p != NULL ? listing_line(p) : NULL;
    expect(line != NULL && strstr(line, "|serialmem|") != NULL &&
               strstr(line, "type=eeprom size=512 addr=1 page=8 write_ps=5000000000 clock_khz=5000") != NULL,
           "the listing line names the facts");
    if (line != NULL) {
        printf("     %s\n", line);
    }
    free(line);
    line = d != NULL ? listing_line(d) : NULL;
    expect(line != NULL && strstr(line, "type=dataflash size=2162688 page=528") != NULL &&
               strlen(line) > 12 && strcmp(line + strlen(line) - 12, "|unsimulated") == 0,
           "an unsimulated part's listing line ends |unsimulated");
    free(line);
}

static void circuit_cases(void) {
    Circuit *c = circuit_create(library);
    size_t index = 0;
    size_t vcc, gnd, chip, n_vcc, n_gnd;
    uint32_t words = 0, values[4] = {0}, known[4] = {0};
    unsigned bits = 0;

    expect(circuit_add_part(c, "FX-DF", &index) == -1 &&
               strcmp(circuit_error(c), "part \"FX-DF\" is not simulated") == 0,
           "circuit_add_part refuses an unsimulated device: part \"FX-DF\" is not simulated");
    expect(circuit_add_part(c, "FX-PE", &index) == -1, "...also the page EEPROM");
    circuit_destroy(c);

    c = circuit_create(library);
    circuit_add_part(c, "5V", &vcc);
    circuit_add_part(c, "GND", &gnd);
    circuit_add_part(c, "FX-E320", &chip);
    circuit_add_net(c, &n_vcc);
    circuit_add_net(c, &n_gnd);
    circuit_connect(c, vcc, "1", n_vcc);
    circuit_connect(c, gnd, "1", n_gnd);
    circuit_connect(c, chip, "8", n_vcc);
    circuit_connect(c, chip, "4", n_gnd);
    expect(circuit_memory_size(c, chip, &words, &bits) == 1 && words == 4096 && bits == 8,
           "circuit_memory_size: 4096 words of 8 bits");
    values[0] = 0x12;
    values[1] = 0x34;
    expect(circuit_memory_load(c, chip, 0x20, values, 2) == 0, "circuit_memory_load works before start");
    expect(circuit_start(c) == 0, "the circuit starts (5 V is inside 1.8-5.5 V)");
    expect(circuit_memory_read(c, chip, 0x1F, 4, values, known) == 4 && values[1] == 0x12 &&
               values[2] == 0x34 && known[0] == 0xFF && values[0] == 0xFF && values[3] == 0xFF,
           "circuit_memory_read: the loaded bytes between erased ones (FF)");
    circuit_destroy(c);
}

/* ---- Random pin activity ------------------------------------------------------------------ */

/* Every input driven at random (0, 1, X and Z, mostly 0 and 1) must never crash the model or
   stop the simulation, whatever the chip. */
static void fuzz_cases(void) {
    static const char *const PARTS[] = {"FX-E40", "FX-E320", "FX-FR", "FX-FL", "FX-FB"};
    static const char *const INPUTS[] = {"CS#", "SCK", "SI", "WP#", "HOLD#"};
    uint32_t seed = 12345;
    size_t p;

    for (p = 0; p < sizeof PARTS / sizeof PARTS[0]; p++) {
        Bench *b = bench_create(part_library_find(library, PARTS[p]));
        int step;
        char what[80];

        for (step = 0; step < 60000; step++) {
            size_t pin;

            seed = seed * 1664525u + 1013904223u;
            pin = (seed >> 24) % 5;
            seed = seed * 1664525u + 1013904223u;
            {
                unsigned roll = (seed >> 24) % 100;
                LogicValue v = roll < 45 ? LOGIC_0 : roll < 90 ? LOGIC_1 : roll < 95 ? LOGIC_X : LOGIC_Z;

                /* CS# low most of the time, so the SPI front end sees real transactions. */
                if (pin == 0 && roll < 60) {
                    v = LOGIC_0;
                }
                bench_set(b, INPUTS[pin], v);
            }
            bench_run(b, NS(20 + (seed >> 16) % 80));
        }
        snprintf(what, sizeof what, "%s survives 60000 random pin changes", PARTS[p]);
        expect(bench_error(b) == NULL, what);
        bench_destroy(b);
    }
}

/* ---- Loader rejections ---------------------------------------------------------------------- */

static int counter;

/* A device file whose device has `keys` (with the standard pins unless they are in keys) and
   a part with `part_extra`; expects the load to fail with `want`. */
static void reject(const char *label, const char *keys, const char *part_extra, const char *want) {
    char text[6000];
    char *error = NULL;
    size_t before = part_library_count(library);
    int status;

    snprintf(text, sizeof text,
             "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"serialmem\",\"devices\":["
             "{\"id\":\"rej%d\",\"function\":\"x\",\"description\":\"d\",\"drawer\":[],%s,"
             "\"pinouts\":{\"p8\":[" PINS8 "]},\"parts\":[{\"name\":\"REJ%d\",\"package\":\"SOIC\","
             "\"pinout\":\"p8\",\"footprint\":\"SOIC-8\"%s}]}]}",
             counter, keys, counter, part_extra);
    counter++;
    status = load_parts(library, text, "reject.json", &error);
    {
        char what[400];

        snprintf(what, sizeof what, "%s: refused with \"%s\" (got: %s)", label, want,
                 status == 0 ? "loaded" : (error != NULL ? error : "(null)"));
        expect(status != 0 && error != NULL && strstr(error, want) != NULL &&
                   part_library_count(library) == before,
               what);
    }
    free(error);
}

#define EE_BASE                                                                               \
    "\"type\":\"eeprom\",\"size\":256,\"addressBytes\":1,\"pageSize\":8,\"writeCycle\":5000000," \
    PIN_KEYS
#define VCC_OK ",\"vcc\":[1.8,5.5]"

static void loader_cases(void) {
    char *error = NULL;
    char text[4000];

    /* A minimal valid pair loads. */
    snprintf(text, sizeof text,
             "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"serialmem\",\"devices\":["
             "{\"id\":\"ok1\",\"function\":\"x\",\"description\":\"d\",\"drawer\":[],%s,"
             "\"pinouts\":{\"p8\":[" PINS8 "]},\"parts\":[{\"name\":\"OK1\",\"package\":\"SOIC\","
             "\"pinout\":\"p8\",\"footprint\":\"SOIC-8\"" VCC_OK "}]}]}",
             EE_BASE);
    expect(load_parts(library, text, "ok.json", &error) == 0, "a minimal eeprom device loads (protection defaults to wpen)");
    free(error);
    error = NULL;

    reject("unknown type", "\"type\":\"rom\",\"size\":512", VCC_OK, "unknown type \"rom\"");
    reject("dataflash simulated", "\"type\":\"dataflash\",\"size\":512", VCC_OK,
           "a \"dataflash\" device has no simulation model: \"simulated\" must be false");
    reject("pageeeprom simulated", "\"simulated\":true,\"type\":\"pageeeprom\",\"size\":512", VCC_OK,
           "a \"pageeeprom\" device has no simulation model");
    reject("size 0", "\"simulated\":false,\"type\":\"dataflash\",\"size\":0", VCC_OK,
           "\"size\" must be more than 0");
    reject("size not a power of two", "\"type\":\"eeprom\",\"size\":500,\"addressBytes\":1,\"pageSize\":8,"
           "\"writeCycle\":5000000," PIN_KEYS, VCC_OK, "\"size\" must be a power of two from 4 to 16777216 bytes");
    reject("size too big", "\"type\":\"eeprom\",\"size\":33554432,\"addressBytes\":3,\"pageSize\":8,"
           "\"writeCycle\":5000000," PIN_KEYS, VCC_OK, "\"size\" must be a power of two");
    reject("eeprom without pageSize", "\"type\":\"eeprom\",\"size\":512,\"addressBytes\":1,"
           "\"writeCycle\":5000000," PIN_KEYS, VCC_OK, "missing \"pageSize\"");
    reject("eeprom without writeCycle", "\"type\":\"eeprom\",\"size\":256,\"addressBytes\":1,"
           "\"pageSize\":8," PIN_KEYS, VCC_OK, "needs \"writeCycle\"");
    reject("eeprom page above a quarter", "\"type\":\"eeprom\",\"size\":256,\"addressBytes\":1,"
           "\"pageSize\":256,\"writeCycle\":5000000," PIN_KEYS, VCC_OK, "\"pageSize\" must be a power of two, at most a quarter");
    reject("eeprom page not a power of two", "\"type\":\"eeprom\",\"size\":256,\"addressBytes\":1,"
           "\"pageSize\":12,\"writeCycle\":5000000," PIN_KEYS, VCC_OK, "\"pageSize\" must be a power of two");
    reject("no addressBytes", "\"type\":\"eeprom\",\"size\":256,\"pageSize\":8,\"writeCycle\":5000000,"
           PIN_KEYS, VCC_OK, "missing \"addressBytes\"");
    reject("addressBytes 4", "\"type\":\"eeprom\",\"size\":512,\"addressBytes\":4,\"pageSize\":8,"
           "\"writeCycle\":5000000," PIN_KEYS, VCC_OK, "\"addressBytes\" must be a whole number from 0 to 3");
    reject("size over one address byte", "\"type\":\"eeprom\",\"size\":1024,\"addressBytes\":1,"
           "\"pageSize\":8,\"writeCycle\":5000000," PIN_KEYS, VCC_OK, "\"size\" 1024 does not fit 1 address byte");
    reject("a8InOpcode with 2 address bytes", "\"type\":\"eeprom\",\"size\":512,\"addressBytes\":2,"
           "\"a8InOpcode\":true,\"pageSize\":8,\"writeCycle\":5000000," PIN_KEYS, VCC_OK,
           "\"a8InOpcode\" needs \"addressBytes\" 1");
    reject("eeprom protection sector", "\"type\":\"eeprom\",\"size\":256,\"addressBytes\":1,\"pageSize\":8,"
           "\"protection\":\"sector\",\"writeCycle\":5000000," PIN_KEYS, VCC_OK,
           "an eeprom or fram device's \"protection\" is \"wpen\" or \"wp\"");
    reject("unknown protection", "\"type\":\"eeprom\",\"size\":256,\"addressBytes\":1,\"pageSize\":8,"
           "\"protection\":\"fuse\",\"writeCycle\":5000000," PIN_KEYS, VCC_OK, "unknown protection \"fuse\"");
    reject("fram with writeCycle", "\"type\":\"fram\",\"size\":256,\"addressBytes\":1,"
           "\"writeCycle\":5000000," PIN_KEYS, VCC_OK, "a fram device has no write cycle");
    reject("fram with pageSize", "\"type\":\"fram\",\"size\":256,\"addressBytes\":1,\"pageSize\":8,"
           PIN_KEYS, VCC_OK, "a fram device has no write page");
    reject("eeprom with sleep", EE_BASE ",\"sleep\":true,\"wake\":1000", VCC_OK,
           "\"sleep\" and \"fastRead\" are for a fram device");
    reject("sleep without wake", "\"type\":\"fram\",\"size\":256,\"addressBytes\":1,\"sleep\":true,"
           PIN_KEYS, VCC_OK, "\"sleep\" needs \"wake\"");
    reject("flash timing on an eeprom", EE_BASE ",\"chipErase\":1000", VCC_OK,
           "the flash timing keys and \"protectSize\" are for a flash device");
    reject("a pin the pinout lacks", "\"type\":\"eeprom\",\"size\":256,\"addressBytes\":1,\"pageSize\":8,"
           "\"writeCycle\":5000000,\"chipSelect\":\"CS#\",\"clock\":\"SCK\",\"dataIn\":\"SI\","
           "\"dataOut\":\"NOPE\"", VCC_OK, "no pin named \"NOPE\"");
    reject("dataOut an input", "\"type\":\"eeprom\",\"size\":256,\"addressBytes\":1,\"pageSize\":8,"
           "\"writeCycle\":5000000,\"chipSelect\":\"CS#\",\"clock\":\"SCK\",\"dataIn\":\"SI\","
           "\"dataOut\":\"SCK\"", VCC_OK, "pin \"SCK\" must be an output or inout pin");
    reject("clock an output", "\"type\":\"eeprom\",\"size\":256,\"addressBytes\":1,\"pageSize\":8,"
           "\"writeCycle\":5000000,\"chipSelect\":\"CS#\",\"clock\":\"SO\",\"dataIn\":\"SI\","
           "\"dataOut\":\"SO\"", VCC_OK, "pin \"SO\" must be an input or inout pin");
    reject("missing pin keys", "\"type\":\"eeprom\",\"size\":256,\"addressBytes\":1,\"pageSize\":8,"
           "\"writeCycle\":5000000", VCC_OK, "a simulated device needs \"chipSelect\"");
    reject("reset on a simulated device", EE_BASE ",\"reset\":\"HOLD#\"", VCC_OK,
           "\"reset\" is only for a device with \"simulated\": false");
    reject("bad jedecId length", EE_BASE ",\"jedecId\":\"1F4\"", VCC_OK, "\"jedecId\" must be 1 to 16 bytes");
    reject("bad jedecId digits", EE_BASE ",\"jedecId\":\"1F4G\"", VCC_OK, "is not hex digits");
    reject("maxClock 0", EE_BASE ",\"maxClock\":0", VCC_OK, "\"maxClock\" must be a clock in MHz");
    reject("part maxClock bad", EE_BASE, VCC_OK ",\"maxClock\":-1", "\"maxClock\" must be a clock in MHz");
    reject("part without vcc", EE_BASE, "", "missing \"vcc\"");
    reject("negative time", "\"type\":\"eeprom\",\"size\":512,\"addressBytes\":1,\"pageSize\":8,"
           "\"writeCycle\":-5," PIN_KEYS, VCC_OK, "\"writeCycle\" must be a time in ns");
    reject("unknown key", EE_BASE ",\"bogus\":1", VCC_OK, "bogus");
    reject("busyOnes 256", EE_BASE ",\"busyOnes\":256", VCC_OK, "\"busyOnes\" must be a whole number from 0 to 255");
    reject("flash without protection", "\"type\":\"flash\",\"size\":65536,\"jedecId\":\"1F4301\","
           "\"pageProgram\":2000000,\"byteProgram\":8000,\"sectorErase\":45000000,\"blockErase32\":3600000,"
           "\"chipErase\":2400000000," PIN_KEYS, VCC_OK, "missing \"protection\"");
    reject("flash without jedecId", "\"type\":\"flash\",\"size\":65536,\"protection\":\"block\","
           "\"pageProgram\":2000000,\"byteProgram\":8000,\"sectorErase\":45000000,\"blockErase32\":3600000,"
           "\"chipErase\":2400000000," PIN_KEYS, VCC_OK, "a flash device needs \"jedecId\"");
    reject("flash sector without protectSize", "\"type\":\"flash\",\"size\":65536,\"protection\":\"sector\","
           "\"jedecId\":\"1F4301\",\"pageProgram\":2000000,\"byteProgram\":8000,\"sectorErase\":45000000,"
           "\"blockErase32\":3600000,\"chipErase\":2400000000," PIN_KEYS, VCC_OK, "missing \"protectSize\"");
    reject("flash without erase times", "\"type\":\"flash\",\"size\":65536,\"protection\":\"block\","
           "\"jedecId\":\"1F4301\",\"pageProgram\":2000000,\"byteProgram\":8000," PIN_KEYS, VCC_OK,
           "a flash device needs \"pageProgram\"");
    reject("flash protectSize not a power of two", "\"type\":\"flash\",\"size\":65536,\"protection\":\"sector\","
           "\"protectSize\":3000,\"jedecId\":\"1F4301\",\"pageProgram\":2000000,\"byteProgram\":8000,"
           "\"sectorErase\":45000000,\"blockErase32\":3600000,\"chipErase\":2400000000," PIN_KEYS, VCC_OK,
           "\"protectSize\" must be a power of two");
    reject("flash with 2 address bytes", "\"type\":\"flash\",\"size\":65536,\"protection\":\"block\","
           "\"addressBytes\":2,\"jedecId\":\"1F4301\",\"pageProgram\":2000000,\"byteProgram\":8000,"
           "\"sectorErase\":45000000,\"blockErase32\":3600000,\"chipErase\":2400000000," PIN_KEYS, VCC_OK,
           "a flash device takes 3 address bytes");
    reject("flash with fastRead", "\"type\":\"flash\",\"size\":65536,\"protection\":\"block\","
           "\"fastRead\":true,\"jedecId\":\"1F4301\",\"pageProgram\":2000000,\"byteProgram\":8000,"
           "\"sectorErase\":45000000,\"blockErase32\":3600000,\"chipErase\":2400000000," PIN_KEYS, VCC_OK,
           "are not flash keys");
    reject("flash protection wpen", "\"type\":\"flash\",\"size\":65536,\"protection\":\"wpen\","
           "\"jedecId\":\"1F4301\",\"pageProgram\":2000000,\"byteProgram\":8000,\"sectorErase\":45000000,"
           "\"blockErase32\":3600000,\"chipErase\":2400000000," PIN_KEYS, VCC_OK,
           "a flash device's \"protection\" is \"sector\" or \"block\"");
    /* An unsimulated device needs no pins and no timing, but still checks what it has. */
    snprintf(text, sizeof text,
             "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"serialmem\",\"devices\":["
             "{\"id\":\"ok2\",\"function\":\"x\",\"description\":\"d\",\"drawer\":[],"
             "\"simulated\":false,\"type\":\"dataflash\",\"size\":1000,"
             "\"pinouts\":{\"p8\":[" PINS8 "]},\"parts\":[{\"name\":\"OK2\",\"package\":\"SOIC\","
             "\"pinout\":\"p8\",\"footprint\":\"SOIC-8\"" VCC_OK "}]}]}");
    expect(load_parts(library, text, "ok2.json", &error) == 0,
           "an unsimulated device loads with only type and size (any size, no pin keys)");
    free(error);
    error = NULL;
    reject("unsimulated with a bad pin", "\"simulated\":false,\"type\":\"dataflash\",\"size\":1000,"
           "\"chipSelect\":\"NOPE\"", VCC_OK, "no pin named \"NOPE\"");
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 || load_fixtures() != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    fact_cases();
    e40_basic(0);
    e40_basic(3);
    e40_pages_protect();
    e40_odds();
    e40_hold();
    e40_x();
    e320_cases(0);
    e320_cases(3);
    fram_cases(0);
    fram_cases(3);
    circuit_cases();
    fuzz_cases();
    loader_cases();
    printf("%s\n", failures == 0 ? "all serialmem tests passed" : "serialmem tests FAILED");
    part_library_destroy(library);
    return failures == 0 ? 0 : 1;
}
