/*
 * serialmem_cov_test (plan-68-R): SPI memory cases serialmem_test and
 * serialmem_flash_test do not drive, on the serialmem bench and its fixture
 * devices (tests/support/serialmem_bench.h), by the model's rules
 * (serialmem.c: an X anywhere in a transaction makes what it did unknown;
 * an unknown address hits every cell it may name), plus two fixtures here:
 *
 *   FX-F16   a 16-KB block-protect flash with no "writeStatus" (WRSR takes
 *            no time), a 32-KB erase larger than the array, and its
 *            "jedecId" in lowercase hex ("1f4a0b")
 *   FX-ENW   a 32-Kbit "wp" EEPROM whose device names no WP# or HOLD# pin
 *            (an unnamed control reads inactive)
 */

#include "serialmem_bench.h"

#define F16_KEYS                                                                              \
    "\"type\":\"flash\",\"size\":16384,\"protection\":\"block\",\"jedecId\":\"1f4a0b\","     \
    "\"pageProgram\":2000000,\"byteProgram\":100000,\"sectorErase\":50000000,"                \
    "\"blockErase32\":400000000,\"chipErase\":800000000,\"pullUp\":true,"                    \
    "\"outputValid\":20,\"outputDisable\":30," PIN_KEYS
#define ENW_KEYS                                                                              \
    "\"type\":\"eeprom\",\"size\":4096,\"addressBytes\":2,\"pageSize\":32,"                  \
    "\"protection\":\"wp\",\"writeCycle\":5000000,\"busyOnes\":255,"                         \
    "\"outputValid\":20,\"outputDisable\":30,\"chipSelect\":\"CS#\",\"clock\":\"SCK\","     \
    "\"dataIn\":\"SI\",\"dataOut\":\"SO\""

/* A transaction of n bytes with bit `xbit` (7 = MSB) of byte `xbyte` sent as X; nrx bytes read. */
static void spi_x(Rig *r, const uint8_t *tx, size_t n, size_t xbyte, int xbit, size_t nrx, Rx *rx) {
    size_t i;
    int bit;

    if (rx != NULL) {
        memset(rx, 0, sizeof *rx);
    }
    cs_low(r);
    for (i = 0; i < n; i++) {
        for (bit = 7; bit >= 0; bit--) {
            LogicValue v = (tx[i] >> bit) & 1 ? LOGIC_1 : LOGIC_0;

            bit_clock(r, i == xbyte && bit == xbit ? LOGIC_X : v);
        }
    }
    for (i = 0; i < nrx; i++) {
        byte_clock(r, 0, rx, i);
    }
    cs_high(r);
}

/* A transaction of n bytes with CS# X for half a clock after byte `after` (then low again): an X
   on the transaction itself (serial_target.c), its address and data still known. */
static void spi_glitch(Rig *r, const uint8_t *tx, size_t n, size_t after) {
    size_t i;

    cs_low(r);
    for (i = 0; i < n; i++) {
        byte_clock(r, tx[i], NULL, 0);
        if (i == after) {
            bench_set(r->b, "CS#", LOGIC_X);
            bench_run(r->b, HALF);
            bench_set(r->b, "CS#", LOGIC_0);
            bench_run(r->b, HALF);
        }
    }
    cs_high(r);
}

static int status_x(Rig *r, uint8_t bits) {
    Rx rx = rdsr_rx(r);

    return (rx.x[0] & bits) == bits;
}

/* ---- EEPROM and F-RAM ---- */

static void fram_cases(void) {
    Rig r = rig_open("FX-FR", 0);
    static const uint8_t wr_x_addr[] = {0x02, 0x00, 0x10, 0x55, 0x66};
    static const uint8_t wr_x_data[] = {0x02, 0x00, 0x20, 0xAA, 0xBB};   /* CS# X after the address */
    uint32_t v[8], k[8];
    size_t n;

    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x10, 0x01, 0x02, 0x03);   /* 10h-12h = 01 02 03 */
    SEND(&r, 0x06);
    spi_x(&r, wr_x_addr, sizeof wr_x_addr, 2, 0, 0, NULL);   /* address bit 0 X: 10h or 11h */
    expect(mem_unknown(&r, 0x10) && mem_unknown(&r, 0x11),
           "F-RAM write at an unknown address (10h or 11h): both cells X");
    expect(mem_is(&r, 0x12, 0x03), "... and the write stopped there: 12h keeps 03h");
    SEND(&r, 0x06);
    spi_glitch(&r, wr_x_data, sizeof wr_x_data, 2);
    expect(mem_unknown(&r, 0x20) && mem_unknown(&r, 0x21),
           "F-RAM write in a transaction CS# made X: every byte it wrote is X");
    n = kind_model(r.part)->memory_read(bench_state(r.b), 2046, 8, v, k);
    expect(n == 2, "memory_read from 2046 of a 2048-byte array copies 2 words");
    rig_close(&r);
}

static void eeprom_cases(void) {
    Rig r = rig_open("FX-E320", 0);
    static const uint8_t wren_x[] = {0x06};
    uint8_t s;

    spi_glitch(&r, wren_x, 1, 0);          /* WREN, then CS# X */
    expect(status_x(&r, 0x02), "EEPROM: WREN in a transaction CS# made X: WEL X");
    SEND(&r, 0x04);
    SEND(&r, 0x06);
    SEND(&r, 0x01);                        /* WRSR without its data byte */
    s = rdsr(&r);
    expect((s & 0x03) == 0x02, "EEPROM: WRSR with no data byte starts nothing (not busy, WEL still 1)");
    SEND(&r, 0x02, 0x00);                  /* WRITE with one of its two address bytes */
    s = rdsr(&r);
    expect((s & 0x03) == 0x02, "EEPROM: WRITE cut inside its address starts nothing (not busy, WEL still 1)");

    /* WPEN set, then a WRSR during which WP# fell: hardware protected, WPEN stays */
    SEND(&r, 0x01, 0x80);
    wait_us(&r, 6000);
    expect((rdsr(&r) & 0x80) == 0x80, "EEPROM: WPEN set");
    SEND(&r, 0x06);
    cs_low(&r);
    byte_clock(&r, 0x01, NULL, 0);
    bench_set(r.b, "WP#", LOGIC_0);
    bench_run(r.b, HALF);
    bench_set(r.b, "WP#", LOGIC_1);
    byte_clock(&r, 0x00, NULL, 0);
    cs_high(&r);
    wait_us(&r, 6000);
    expect((rdsr(&r) & 0x80) == 0x80, "EEPROM: WP# low during the WRSR (WPEN 1) blocks it: WPEN stays 1");
    rig_close(&r);

    r = rig_open("FX-E40", 0);
    bench_set(r.b, "WP#", LOGIC_X);
    SEND(&r, 0x06);
    expect(status_x(&r, 0x02), "EEPROM (\"wp\"): WREN with WP# X: WEL X");
    rig_close(&r);

    r = rig_open("FX-ENW", 0);
    bench_set(r.b, "WP#", LOGIC_0);        /* a pin the device does not name */
    SEND(&r, 0x06);
    expect((rdsr(&r) & 0x02) == 0x02, "EEPROM with no WP# named: WREN sets WEL (the pin at 0 is no WP#)");
    rig_close(&r);
}

/* ---- flash ---- */

static void flash_sector_cases(void) {
    Rig r = rig_open("FX-FL", 0);
    static const uint8_t wrsr_x[] = {0x01, 0x00};
    static const uint8_t protect_x[] = {0x36, 0x00, 0x40, 0x00};
    static const uint8_t erase_x[] = {0x20, 0x00, 0x10, 0x00};
    uint8_t v, k;

    SEND(&r, 0x06);
    expect((rdsr(&r) & 0x02) == 0x02, "flash: WREN sets WEL");
    SEND(&r, 0x04);
    expect((rdsr(&r) & 0x02) == 0x00, "flash: WRDI clears WEL");

    SEND(&r, 0x06);
    SEND(&r, 0x36, 0x00);                  /* protect, cut after one address byte */
    expect((rdsr(&r) & 0x0E) == 0x0C, "flash: a protect without its address clears WEL, sectors still all protected");

    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x00);                  /* global unprotect */
    expect((rdsr(&r) & 0x0C) == 0x00, "flash: WRSR 00h unprotects every sector");
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x10, 0x00, 0x0F);
    wait_us(&r, 200);
    expect(mem_is(&r, 0x1000, 0x0F), "flash: 1000h programmed to 0Fh");
    SEND(&r, 0x06);
    SEND(&r, 0x20, 0x00);                  /* 4-KB erase without its address */
    wait_us(&r, 50000);
    expect(mem_is(&r, 0x1000, 0x0F), "flash: an erase cut inside its address erases nothing");
    SEND(&r, 0x06);
    spi_glitch(&r, erase_x, sizeof erase_x, 3);          /* CS# X after the address */
    wait_us(&r, 50000);
    mem(&r, 0x1000, &v, &k);
    expect(k == 0x0F && v == 0x0F && mem_is(&r, 0x1FFF, 0xFF),
           "flash: an X erase may or may not have run: 0Fh keeps its 1 bits known, its 0 bits X; FFh stays");

    SEND(&r, 0x06);
    spi_x(&r, protect_x, sizeof protect_x, 2, 0, 0, NULL);   /* address bit unknown */
    expect(status_x(&r, 0x0C), "flash: a protect at an unknown address: every sector X (SWP X)");

    SEND(&r, 0x06);
    spi_x(&r, wrsr_x, 2, 1, 7, 0, NULL);  /* SPRL bit X */
    expect(status_x(&r, 0x8C), "flash (sector): WRSR with an X bit: SPRL and every sector X");
    rig_close(&r);

    /* WRSR2 with WEL 0 is ignored: RSTE stays 0, so a RESET does nothing */
    r = rig_open("FX-FL", 0);
    SEND(&r, 0x31, 0x10);
    SEND(&r, 0x06);
    SEND(&r, 0xF0, 0xD0);
    expect((rdsr(&r) & 0x02) == 0x02, "flash: WRSR2 without WREN is ignored (RSTE 0): RESET leaves WEL 1");
    SEND(&r, 0x31, 0x10);                  /* with WEL 1 it takes: RSTE 1 */
    SEND(&r, 0x06);
    SEND(&r, 0xF0, 0xD0);
    expect((rdsr(&r) & 0x02) == 0x00, "flash: WRSR2 with WEL 1 sets RSTE: RESET clears WEL");
    rig_close(&r);
}

static void flash_block_cases(void) {
    Rig r = rig_open("FX-FB", 0);
    static const uint8_t wrsr_x[] = {0x01, 0x04};

    SEND(&r, 0x01, 0x84);                  /* without WREN */
    expect((rdsr(&r) & 0x84) == 0x00, "flash (block): WRSR with WEL 0 changes nothing (BPL, BP0 0)");
    SEND(&r, 0x06);
    SEND(&r, 0x01);                        /* without its data byte */
    expect((rdsr(&r) & 0x03) == 0x00, "flash (block): WRSR with no data byte clears WEL, starts nothing");
    SEND(&r, 0x06);
    spi_glitch(&r, wrsr_x, 2, 1);
    wait_us(&r, 21000);
    expect(status_x(&r, 0x84), "flash (block): WRSR in a transaction CS# made X: after tW, BPL and BP0 X");
    bench_set(r.b, "WP#", LOGIC_X);
    expect(status_x(&r, 0x10), "flash: WP# X reads as an X status bit 4");
    rig_close(&r);

    r = rig_open("FX-F16", 0);
    {
        Rx rx;

        SPI(&r, &rx, 3, 0x9F);
        expect(rx_is(&rx, 0, 0x1F) && rx_is(&rx, 1, 0x4A) && rx_is(&rx, 2, 0x0B),
               "FX-F16: RDID returns the lowercase \"1f4a0b\" as 1F 4A 0B");
    }
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x04);                  /* no "writeStatus": applied at once */
    expect((rdsr(&r) & 0x05) == 0x04, "flash with no writeStatus: WRSR takes no time (BP0 1, not busy)");
    SEND(&r, 0x06);
    SEND(&r, 0x01, 0x00);
    SEND(&r, 0x06);
    SEND(&r, 0x02, 0x00, 0x3F, 0xFF, 0x00);
    wait_us(&r, 200);
    expect(mem_is(&r, 0x3FFF, 0x00), "FX-F16: 3FFFh programmed to 00h");
    SEND(&r, 0x06);
    SEND(&r, 0x52, 0x00, 0x00, 0x00);      /* 32-KB erase on a 16-KB array */
    wait_us(&r, 410000);
    expect(mem_is(&r, 0x3FFF, 0xFF), "FX-F16: a 32-KB erase erases the whole 16-KB array (3FFFh FFh)");
    rig_close(&r);
}

/* "simulated": false: the model is passive, no memory */
static void passive_hooks(void) {
    Rig r = rig_open("FX-DF", 0);
    const SimComponentType *type = kind_model(r.part);
    uint32_t words = 7, values[2] = {1, 2}, k[2];
    unsigned bits = 7;

    type->memory_load(bench_state(r.b), 0, values, 2);
    expect(type->memory_size(bench_state(r.b), &words, &bits) == 0, "a passive serialmem has no memory size");
    expect(type->memory_read(bench_state(r.b), 0, 2, values, k) == 0, "... reads no words, and took no load");
    rig_close(&r);
}

int main(void) {
    static const char *const MORE[] = {
        FILE_TEXT("f16", F16_KEYS, "FX-F16", ""),
        FILE_TEXT("enw", ENW_KEYS, "FX-ENW", "")
    };
    size_t i;

    setvbuf(stdout, NULL, _IOLBF, 0);   /* the leak check ends with _exit: keep the lines */
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 || load_fixtures() != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    for (i = 0; i < 2; i++) {
        char *error = NULL;

        if (load_parts(library, MORE[i], "more.json", &error) != 0) {
            printf("FAIL: fixture %zu: %s\n", i, error != NULL ? error : "");
            free(error);
            return 1;
        }
    }
    fram_cases();
    eeprom_cases();
    flash_sector_cases();
    flash_block_cases();
    passive_hooks();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("serialmem_cov_test: all passed\n");
    return 0;
}
