/*
 * serialmem_flash_test: split from serialmem_test.c by plan-68-H; the shared helpers are
 * support/serialmem_bench.c.
 */

#include "serialmem_bench.h"

/* ---- Flash --------------------------------------------------------------------------------- */

static void flash_basic(unsigned mode) {
    Rig r = rig_open("FX-FL", mode);
    Rx rx;
    uint8_t v, k;

    SPI(&r, &rx, 6, 0x9F);
    expect(rx_is(&rx, 0, 0x1F) && rx_is(&rx, 1, 0x43) && rx_is(&rx, 2, 0x01) && rx_is(&rx, 3, 0x00) &&
               rx_z(&rx, 4) && rx_z(&rx, 5),
           "JEDEC ID 1F 43 01 00, then SO Z");
    SPI(&r, &rx, 3, 0x05);
    expect(rx_is(&rx, 0, 0x1C) && rx_is(&rx, 1, 0x00) && rx_is(&rx, 2, 0x1C),
           "status: SPRL 0, WPP 1, SWP 11 (all protected), byte 2 = 00, then byte 1 again");
    SPI(&r, &rx, 2, 0x03, 0x00, 0x00, 0x00);
    expect(rx_is(&rx, 0, 0xFF) && rx_is(&rx, 1, 0xFF), "flash powers up erased");
    SPI(&r, &rx, 2, 0x0B, 0x00, 0x00, 0x00, 0x00);
    expect(rx_is(&rx, 0, 0xFF) && rx_is(&rx, 1, 0xFF), "READ 0B: three address bytes, a dummy byte, data");

    /* Everything is protected at power-up. */
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x00, 0x10, 0xF0);
    wait_us(&r, 200);
    expect(mem_is(&r, 0x10, 0xFF) && rdsr(&r) == 0x1C,
           "a program into a protected sector is ignored; WEL is cleared");
    SEND(&r, 0x06);
    SEND(&r, 0x20, 0x00, 0x10, 0x00);
    wait_us(&r, 100000);
    expect(rdsr(&r) == 0x1C, "an erase of a protected sector is ignored");
    /* Global unprotect. */
    SEND(&r, 0x06);
    expect(rdsr(&r) == 0x1E, "WEL set (1E)");
    SEND(&r, 0x01, 0x00);
    expect(rdsr(&r) == 0x10, "WRSR 00 is a global unprotect: SWP 00, WEL cleared");

    /* Byte program: busy 8 us, ANDs with the old data. */
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x00, 0x10, 0xF0);
    expect(rdsr(&r) == 0x11, "busy during the program (RDY 1, WEL already 0)");
    SEND(&r, 0x06);
    expect(rdsr(&r) == 0x11, "WREN is ignored while busy");
    wait_us(&r, 150);
    expect(rdsr(&r) == 0x10 && mem_is(&r, 0x10, 0xF0), "the byte is stored and RDY clears after byteProgram");
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x00, 0x10, 0x3C);
    wait_us(&r, 150);
    expect(mem_is(&r, 0x10, 0x30), "a program ANDs with what is there: F0 and 3C = 30");

    /* Page program wraps inside 256 bytes and takes pageProgram. */
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x00, 0xFE, 0x01, 0x02, 0x03, 0x04, 0x05);
    wait_us(&r, 1900);
    expect(rdsr(&r) == 0x11, "page program: still busy after 1.9 ms");
    wait_us(&r, 200);
    expect(rdsr(&r) == 0x10 && mem_is(&r, 0xFE, 0x01) && mem_is(&r, 0xFF, 0x02) &&
               mem_is(&r, 0x00, 0x03) && mem_is(&r, 0x01, 0x04) && mem_is(&r, 0x02, 0x05) &&
               mem_is(&r, 0x100, 0xFF),
           "the page program stored 5 bytes with wrap inside the page");
    /* A cut byte and a missing address. */
    {
        static const uint8_t cut[] = {0x02, 0x00, 0x00, 0x20, 0xAA};

        SEND(&r, 0x06);
        spi_bits(&r, cut, 8 * 4 + 5);
        wait_us(&r, 200);
        expect(mem_is(&r, 0x20, 0xFF) && rdsr(&r) == 0x10, "CS# 3 bits short of the data byte drops the program; WEL cleared");
        SEND(&r, 0x06);
        SEND(&r, 0x02, 0x00, 0x00);
        expect(rdsr(&r) == 0x10, "a program with a cut address does nothing; WEL cleared");
    }

    /* Page erase (81): 256 bytes. */
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x01, 0x80, 0x00, 0x00);
    wait_us(&r, 2100);
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x02, 0x00, 0x00);
    wait_us(&r, 200);
    SEND(&r, 0x06);
    SEND(&r, 0x81, 0x00, 0x01, 0x23);
    expect(rdsr(&r) == 0x11, "page erase is busy");
    wait_us(&r, 6100);
    expect(rdsr(&r) == 0x10 && mem_is(&r, 0x180, 0xFF) && mem_is(&r, 0x200, 0x00) &&
               mem_is(&r, 0xFE, 0x01),
           "a page erase erases the 256 bytes of 0100-01FF only");

    /* 4-KB erase. */
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x11, 0x00, 0x00);
    wait_us(&r, 200);
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x0F, 0xFF, 0x00);
    wait_us(&r, 200);
    SEND(&r, 0x06);
    SEND(&r, 0x20, 0x00, 0x1F, 0xFF);
    wait_us(&r, 40000);
    expect(rdsr(&r) == 0x11, "4 KB erase still busy at 40 ms");
    wait_us(&r, 6000);
    expect(rdsr(&r) == 0x10 && mem_is(&r, 0x1100, 0xFF) && mem_is(&r, 0x0FFF, 0x00),
           "20: the 4-KB block 1000-1FFF is erased, 0FFF is not");

    /* 32-KB erase (52) and 64-KB erase (D8). */
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x80, 0x00, 0x00);
    wait_us(&r, 200);
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x7F, 0xFF, 0x00);
    wait_us(&r, 200);
    SEND(&r, 0x06);
    SEND(&r, 0x52, 0x00, 0x00, 0x01);
    wait_us(&r, 359000);
    expect(rdsr(&r) == 0x11, "32 KB erase is still busy at 359 ms");
    wait_us(&r, 2000);
    expect(rdsr(&r) == 0x10 && mem_is(&r, 0x7FFF, 0xFF) && mem_is(&r, 0x8000, 0x00),
           "52: the 32-KB block 0000-7FFF is erased, 8000 is not");
    SEND(&r, 0x06);
    SEND(&r, 0xD8, 0x00, 0xFF, 0xFF);
    wait_us(&r, 721000);
    expect(rdsr(&r) == 0x10 && mem_is(&r, 0x8000, 0xFF), "D8: the 64-KB block (the whole chip) is erased");

    /* Chip erase: 60 and C7. */
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x30, 0x00, 0x12);
    wait_us(&r, 200);
    SEND(&r, 0x06);
    SEND(&r, 0x60);
    wait_us(&r, 2399000);
    expect(rdsr(&r) == 0x11 && mem_is(&r, 0x3000, 0x12), "chip erase (60): busy at 2.399 s, the data is still there");
    wait_us(&r, 2000);
    expect(rdsr(&r) == 0x10 && mem_is(&r, 0x3000, 0xFF), "chip erase ends after chipErase");
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x30, 0x00, 0x12);
    wait_us(&r, 200);
    SEND(&r, 0x06);
    SEND(&r, 0xC7);
    wait_us(&r, 2402000);
    expect(mem_is(&r, 0x3000, 0xFF), "C7 is chip erase too");

    /* Erase needs WEL. */
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x30, 0x00, 0x13);
    wait_us(&r, 200);
    SEND(&r, 0x20, 0x00, 0x30, 0x00);
    wait_us(&r, 50000);
    expect(mem_is(&r, 0x3000, 0x13), "an erase without WREN does nothing");

    /* Unsupported opcodes. */
    SPI(&r, &rx, 2, 0x3B, 0x00, 0x00, 0x00);
    expect(rx_z(&rx, 0) && rx_z(&rx, 1), "dual output read (3B) is not modeled: ignored");
    SPI(&r, &rx, 1, 0x77, 0, 0, 0);
    expect(rx_z(&rx, 0), "the OTP register read (77) is not modeled: ignored");
    (void)v;
    (void)k;
    rig_close(&r);
}

static void flash_protect(void) {
    Rig r = rig_open("FX-FL", 0);
    Rx rx;

    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x00);
    expect(rdsr(&r) == 0x10, "unprotected");
    /* Protect sector 1 (4000-7FFF). */
    SEND(&r, 0x06);
    SEND(&r, 0x36, 0x00, 0x50, 0x00);
    expect(rdsr(&r) == 0x14, "36: one sector protected, SWP 01; WEL cleared");
    SPI(&r, &rx, 2, 0x3C, 0x00, 0x60, 0x00);
    expect(rx_is(&rx, 0, 0xFF) && rx_is(&rx, 1, 0xFF), "3C reads FF for a protected sector");
    SPI(&r, &rx, 2, 0x3C, 0x00, 0x00, 0x00);
    expect(rx_is(&rx, 0, 0x00) && rx_is(&rx, 1, 0x00), "3C reads 00 for an unprotected sector");
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x40, 0x00, 0x44);
    wait_us(&r, 200);
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x3F, 0xFF, 0x45);
    wait_us(&r, 200);
    expect(mem_is(&r, 0x4000, 0xFF) && mem_is(&r, 0x3FFF, 0x45),
           "a program into the protected sector is ignored, one into its neighbor is stored");
    SEND(&r, 0x06);
    SEND(&r, 0x60);
    wait_us(&r, 2500000);
    expect(mem_is(&r, 0x3FFF, 0x45), "chip erase is ignored while any sector is protected");
    SEND(&r, 0x06);
    SEND(&r, 0x39, 0x00, 0x7F, 0xFF);
    expect(rdsr(&r) == 0x10, "39: the sector is unprotected again (SWP 00)");
    SEND(&r, 0x39, 0x00, 0x00, 0x00);

    /* SPRL: lock the registers with WRSR 80. */
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x80);
    expect(rdsr(&r) == 0x90, "WRSR 80 sets SPRL (and global unprotect)");
    SEND(&r, 0x06);
    SEND(&r, 0x36, 0x00, 0x00, 0x00);
    expect(rdsr(&r) == 0x90, "36 is ignored while SPRL = 1; WEL cleared");
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x7C);
    expect(rdsr(&r) == 0x10, "with WP# high and SPRL = 1 WRSR can clear SPRL, but a global protect does not run (SWP 00)");
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x7C);
    expect(rdsr(&r) == 0x1C, "with SPRL = 0 WRSR 7C protects every sector (SWP 11)");
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x80);
    expect(rdsr(&r) == 0x90, "WRSR 80 with SPRL = 0: SPRL set and the (all-zero) global bits unprotect every sector");
    rig_close(&r);
}

static void flash_wp_lock(void) {
    Rig r = rig_open("FX-FL", 0);

    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x80);
    expect(rdsr(&r) == 0x90, "SPRL set");
    bench_set(r.b, "WP#", LOGIC_0);
    bench_run(r.b, HALF);
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x7C);
    expect(rdsr(&r) == 0x80, "WPP reads 0 with WP# low; WP# low and SPRL = 1: WRSR ignored (hard lock), WEL cleared");
    bench_set(r.b, "WP#", LOGIC_1);
    bench_run(r.b, HALF);
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x7C);
    expect(rdsr(&r) == 0x10, "WP# high: SPRL cleared by WRSR (soft lock)");
    bench_set(r.b, "WP#", LOGIC_0);
    bench_run(r.b, HALF);
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x80);
    expect(rdsr(&r) == 0x80, "WP# low and SPRL = 0: SPRL can be set (and the global unprotect runs)");
    /* A floating WP# reads as high on a part with "pullUp". */
    bench_set(r.b, "WP#", LOGIC_Z);
    bench_run(r.b, HALF);
    expect((rdsr(&r) & 0x10) == 0x10, "a floating WP# reads high (internal pull-up): WPP 1");
    rig_close(&r);
}

static void flash_misc(void) {
    Rig r = rig_open("FX-FL", 0);
    Rx rx;

    /* RSTE and the reset command. */
    SEND(&r, 0x06);
    SEND(&r, 0x31, 0x10);
    SPI(&r, &rx, 3, 0x05);
    expect(rx_is(&rx, 1, 0x10) && rx_is(&rx, 0, 0x1C), "WRSR byte 2 sets RSTE (status byte 2 = 10)");
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x00);
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x80);
    expect(rdsr(&r) == 0x90, "SPRL set, all unprotected");
    SEND(&r, 0xF0, 0xD0);
    expect(rdsr(&r) == 0x1C, "F0 D0 with RSTE 1: SPRL cleared and every sector protected again");
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x00);
    SEND(&r, 0xF0, 0xAA);
    expect(rdsr(&r) == 0x10, "F0 with a data byte other than D0 does nothing");

    /* Deep power-down. */
    SEND(&r, 0xB9);
    SPI(&r, &rx, 1, 0x05);
    expect(rx_z(&rx, 0), "in deep power-down RDSR is ignored (SO Z)");
    SPI(&r, &rx, 4, 0x9F);
    expect(rx_z(&rx, 0), "...and so is the JEDEC read");
    SEND(&r, 0xAB);
    SPI(&r, &rx, 1, 0x05);
    expect(rx_is(&rx, 0, 0x10), "AB resumes");

    /* An unknown opcode makes WEL unknown; a program with WEL unknown is a maybe. */
    {
        int i;

        cs_low(&r);
        bit_clock(&r, LOGIC_X);
        for (i = 6; i >= 0; i--) {
            bit_clock(&r, (0x06 >> i) & 1 ? LOGIC_1 : LOGIC_0);
        }
        cs_high(&r);
        rx = rdsr_rx(&r);
        expect((rx.x[0] & 0x02) == 0x02, "an opcode with an unknown bit leaves WEL unknown");
        SEND(&r, 0x02, 0x00, 0x20, 0x00, 0x66);
        wait_us(&r, 200);
        expect(mem_unknown(&r, 0x2000) && mem_is(&r, 0x2001, 0xFF),
               "a program with WEL unknown stores X in the byte it writes");
        expect(rdsr(&r) == 0x10, "...and WEL is 0 after it");
    }
    /* A program address with an unknown bit. */
    {
        int i;

        SEND(&r, 0x06);
        cs_low(&r);
        byte_clock(&r, 0x20, NULL, 0);
        byte_clock(&r, 0x00, NULL, 0);
        for (i = 7; i >= 0; i--) {
            bit_clock(&r, i == 4 ? LOGIC_X : LOGIC_0);
        }
        byte_clock(&r, 0x00, NULL, 0);
        cs_high(&r);
        wait_us(&r, 100000);
        expect(mem_unknown(&r, 0x0000) && mem_unknown(&r, 0x1000) && mem_unknown(&r, 0x1FFF),
               "an erase with an unknown address bit makes the blocks it could name unknown");
    }
    rig_close(&r);
}

static void flash_hold(void) {
    Rig r = rig_open("FX-FL", 3);
    Rx rx;
    int z = 1;
    unsigned i;
    uint8_t got = 0;

    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x00);
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x00, 0x00, 0xA9);
    wait_us(&r, 200);
    /* Mode 3, HOLD# low while SCK is high: the hold starts at the next SCK fall. */
    cs_low(&r);
    byte_clock(&r, 0x03, NULL, 0);
    byte_clock(&r, 0, NULL, 0);
    byte_clock(&r, 0, NULL, 0);
    byte_clock(&r, 0, NULL, 0);
    for (i = 0; i < 3; i++) {
        got = (uint8_t)((got << 1) | (bit_clock(&r, LOGIC_0) == LOGIC_1));
    }
    bench_set(r.b, "HOLD#", LOGIC_0);
    bench_run(r.b, HALF);
    for (i = 0; i < 3; i++) {
        z &= bit_clock(&r, LOGIC_0) == LOGIC_Z;
    }
    bench_set(r.b, "SCK", LOGIC_0);
    bench_run(r.b, HALF);
    bench_set(r.b, "HOLD#", LOGIC_1);
    bench_run(r.b, HALF);
    got = (uint8_t)((got << 1) | (bench_get(r.b, "SO") == LOGIC_1));
    bench_set(r.b, "SCK", LOGIC_1);
    bench_run(r.b, HALF);
    for (i = 0; i < 4; i++) {
        got = (uint8_t)((got << 1) | (bit_clock(&r, LOGIC_0) == LOGIC_1));
    }
    cs_high(&r);
    expect(z && got == 0xA9, "mode 3 read with a HOLD# pause in the middle of a byte keeps its place (A9)");
    SPI(&r, &rx, 1, 0x05);
    expect(rx_is(&rx, 0, 0x10), "flash is fine after the hold");
    rig_close(&r);
}

/* ---- Flash with BP0 / BPL (AT25XE512C-like) ---------------------------------------------------- */

static void flash_block(void) {
    Rig r = rig_open("FX-FB", 0);
    Rx rx;

    SPI(&r, &rx, 3, 0x9F);
    expect(rx_is(&rx, 0, 0x1F) && rx_is(&rx, 1, 0x65) && rx_is(&rx, 2, 0x01),
           "JEDEC ID 1F 65 01");
    expect(rdsr(&r) == 0x10, "block scheme power-up status 10 (WPP only): nothing is protected");
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x10, 0x00, 0x5A);
    wait_us(&r, 200);
    expect(mem_is(&r, 0x1000, 0x5A), "a program works at power-up (BP0 = 0)");
    /* WRSR takes writeStatus. */
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x04);
    expect(rdsr(&r) == 0x11, "WRSR is busy for writeStatus (20 ms)");
    wait_us(&r, 21000);
    expect(rdsr(&r) == 0x14, "BP0 set: status 14");
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x20, 0x00, 0x5A);
    wait_us(&r, 200);
    SEND(&r, 0x06);
    SEND(&r, 0x20, 0x00, 0x10, 0x00);
    wait_us(&r, 100000);
    SEND(&r, 0x06);
    SEND(&r, 0x60);
    wait_us(&r, 900000);
    expect(mem_is(&r, 0x2000, 0xFF) && mem_is(&r, 0x1000, 0x5A),
           "with BP0 program, block erase and chip erase are ignored");
    /* BPL with WP#. */
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x80);
    wait_us(&r, 21000);
    expect(rdsr(&r) == 0x90, "WRSR 80: BPL set, BP0 cleared (WP# high)");
    bench_set(r.b, "WP#", LOGIC_0);
    bench_run(r.b, HALF);
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x84);
    wait_us(&r, 21000);
    expect(rdsr(&r) == 0x80, "WP# low with BPL = 1: WRSR is ignored (hard lock)");
    bench_set(r.b, "WP#", LOGIC_1);
    bench_run(r.b, HALF);
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x04);
    wait_us(&r, 21000);
    expect(rdsr(&r) == 0x14, "WP# high: WRSR 04 sets BP0 and clears BPL");
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x00);
    wait_us(&r, 21000);
    /* D8 erases 32 KB when blockErase64 is not given; 81 and 36 are not commands. */
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x80, 0x00, 0x11);
    wait_us(&r, 200);
    SEND(&r, 0x06);
    SEND(&r, 0xD8, 0x00, 0x10, 0x00);
    wait_us(&r, 401000);
    expect(mem_is(&r, 0x1000, 0xFF) && mem_is(&r, 0x8000, 0x11), "D8 erases a 32-KB block here (no blockErase64)");
    SEND(&r, 0x06);
    SEND(&r, 0x81, 0x00, 0x80, 0x00);
    wait_us(&r, 20000);
    expect(mem_is(&r, 0x8000, 0x11), "81 (page erase) is not a command of this chip");
    SPI(&r, &rx, 1, 0x3C, 0, 0, 0);
    expect(rx_z(&rx, 0), "the sector protection commands do not exist here");
    rig_close(&r);
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 || load_fixtures() != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    flash_basic(0);
    flash_basic(3);
    flash_protect();
    flash_wp_lock();
    flash_misc();
    flash_hold();
    flash_block();
    printf("%s\n", failures == 0 ? "all serialmem tests passed" : "serialmem tests FAILED");
    part_library_destroy(library);
    return failures == 0 ? 0 : 1;
}
