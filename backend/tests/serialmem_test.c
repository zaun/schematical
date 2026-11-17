/*
 * serialmem_test: the serialmem kind (SPI EEPROM, F-RAM and flash models in
 * backend/src/models/serialmem.c). Fixture devices written inline (so the
 * test does not depend on shipped parts) are loaded next to the real part
 * files and driven through a bench with a bit-level SPI master in mode 0
 * and mode 3: every opcode of every model, page wrap, block protect, WEL,
 * busy behavior, CS# timing, HOLD#, power-up contents, X handling, the
 * "simulated": false devices, and the loader's rejections.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "circuit.h"
#include "kind.h"
#include "listing.h"
#include "parts.h"
#include "read_file.h"

#define NS(v) ((SimTime)((v) * 1000ull))
#define HALF NS(100)

static int failures;
static PartLibrary *library;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

/* ---- Fixture devices --------------------------------------------------------------------- */

#define PINS8                                                                                \
    "{\"number\":1,\"name\":\"CS#\",\"role\":\"input\",\"activeLow\":true,\"group\":\"spi\"}," \
    "{\"number\":2,\"name\":\"SO\",\"role\":\"output\",\"group\":\"spi\"},"                     \
    "{\"number\":3,\"name\":\"WP#\",\"role\":\"input\",\"activeLow\":true,"                    \
    "\"group\":\"control\"},"                                                                   \
    "{\"number\":4,\"name\":\"GND\",\"role\":\"ground\"},"                                      \
    "{\"number\":5,\"name\":\"SI\",\"role\":\"input\",\"group\":\"spi\"},"                      \
    "{\"number\":6,\"name\":\"SCK\",\"role\":\"input\",\"group\":\"spi\"},"                     \
    "{\"number\":7,\"name\":\"HOLD#\",\"role\":\"input\",\"activeLow\":true,"                  \
    "\"group\":\"control\"},"                                                                   \
    "{\"number\":8,\"name\":\"VCC\",\"role\":\"power\"}"

#define PIN_KEYS                                                                             \
    "\"chipSelect\":\"CS#\",\"clock\":\"SCK\",\"dataIn\":\"SI\",\"dataOut\":\"SO\","         \
    "\"writeProtect\":\"WP#\",\"hold\":\"HOLD#\""

/* One device file: `keys` are the device's keys (no trailing comma), `part_extra` extra part keys. */
#define FILE_TEXT(ID, KEYS, PART, PART_EXTRA)                                                 \
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"serialmem\",\"devices\":["    \
    "{\"id\":\"" ID "\",\"function\":\"fixture\",\"description\":\"fixture\",\"drawer\":[],"  \
    KEYS ",\"pinouts\":{\"p8\":[" PINS8 "]},\"parts\":[{\"name\":\"" PART                     \
    "\",\"package\":\"SOIC\",\"pinout\":\"p8\",\"footprint\":\"SOIC-8\","                    \
    "\"vcc\":[1.8,5.5]" PART_EXTRA "}]}]}"

/* 4-Kbit EEPROM: A8 in the opcode, WP# blocks every write. */
#define E40_KEYS                                                                              \
    "\"type\":\"eeprom\",\"size\":512,\"addressBytes\":1,\"a8InOpcode\":true,"              \
    "\"pageSize\":8,\"protection\":\"wp\",\"writeCycle\":5000000,\"busyOnes\":240,"          \
    "\"outputValid\":20,\"outputDisable\":30,\"maxClock\":10,\"endurance\":1000000,"         \
    "\"retention\":100," PIN_KEYS
/* 32-Kbit EEPROM: two address bytes, WPEN, busy reads all ones. */
#define E320_KEYS                                                                             \
    "\"type\":\"eeprom\",\"size\":4096,\"addressBytes\":2,\"pageSize\":32,"                  \
    "\"protection\":\"wpen\",\"writeCycle\":5000000,\"busyOnes\":255,"                       \
    "\"outputValid\":20,\"outputDisable\":30," PIN_KEYS
/* 16-Kbit F-RAM with RDID, FSTRD and SLEEP. */
#define FR_KEYS                                                                               \
    "\"type\":\"fram\",\"size\":2048,\"addressBytes\":2,\"protection\":\"wpen\","            \
    "\"fastRead\":true,\"sleep\":true,\"wake\":450000,\"statusOnes\":64,"                    \
    "\"jedecId\":\"7F7F7F7F7F7FC225C8\",\"outputValid\":20,\"outputDisable\":30," PIN_KEYS
/* 64-KB flash with four 16-KB sector protection registers. */
#define FL_KEYS                                                                               \
    "\"type\":\"flash\",\"size\":65536,\"protection\":\"sector\",\"protectSize\":16384,"     \
    "\"jedecId\":\"1F430100\",\"pageProgram\":2000000,\"byteProgram\":100000,"                 \
    "\"pageErase\":6000000,\"sectorErase\":45000000,\"blockErase32\":360000000,"             \
    "\"blockErase64\":720000000,\"chipErase\":2400000000,\"pullUp\":true,"                   \
    "\"outputValid\":20,\"outputDisable\":30," PIN_KEYS
/* 64-KB flash with BP0 and BPL (no 64-KB erase, no page erase). */
#define FB_KEYS                                                                               \
    "\"type\":\"flash\",\"size\":65536,\"protection\":\"block\",\"jedecId\":\"1F6501\","     \
    "\"pageProgram\":2000000,\"byteProgram\":100000,\"sectorErase\":50000000,"                \
    "\"blockErase32\":400000000,\"chipErase\":800000000,\"writeStatus\":20000000,"           \
    "\"pullUp\":true,\"outputValid\":20,\"outputDisable\":30," PIN_KEYS
/* Facts only. */
#define DF_KEYS                                                                               \
    "\"simulated\":false,\"type\":\"dataflash\",\"size\":2162688,\"pageSize\":528,"          \
    "\"maxClock\":85,\"chipSelect\":\"CS#\",\"clock\":\"SCK\",\"dataIn\":\"SI\","            \
    "\"dataOut\":\"SO\",\"reset\":\"HOLD#\""
#define PE_KEYS                                                                               \
    "\"simulated\":false,\"type\":\"pageeeprom\",\"size\":4194304,\"pageSize\":256"

static int load_fixtures(void) {
    static const char *const FILES[] = {
        FILE_TEXT("e40", E40_KEYS, "FX-E40", ",\"maxClock\":5"),
        FILE_TEXT("e320", E320_KEYS, "FX-E320", ""),
        FILE_TEXT("fr", FR_KEYS, "FX-FR", ""),
        FILE_TEXT("fl", FL_KEYS, "FX-FL", ""),
        FILE_TEXT("fb", FB_KEYS, "FX-FB", ""),
        FILE_TEXT("df", DF_KEYS, "FX-DF", ""),
        FILE_TEXT("pe", PE_KEYS, "FX-PE", "")
    };
    size_t i;

    for (i = 0; i < sizeof FILES / sizeof FILES[0]; i++) {
        char *error = NULL;

        if (load_parts(library, FILES[i], "serialmem-fixture.json", &error) != 0) {
            fprintf(stderr, "fixture %zu: %s\n", i, error != NULL ? error : "(none)");
            free(error);
            return -1;
        }
    }
    return 0;
}

/* ---- SPI master -------------------------------------------------------------------------- */

typedef struct {
    Bench *b;
    const Part *part;
    unsigned mode;
} Rig;

typedef struct {
    uint8_t v[48];   /* the bits read as 1 */
    uint8_t z[48];   /* the bits read as Z */
    uint8_t x[48];   /* the bits read as X */
} Rx;

static Rig rig_open(const char *name, unsigned mode) {
    Rig r;

    r.part = part_library_find(library, name);
    r.b = bench_create(r.part);
    r.mode = mode;
    bench_set(r.b, "CS#", LOGIC_1);
    bench_set(r.b, "WP#", LOGIC_1);
    bench_set(r.b, "HOLD#", LOGIC_1);
    bench_set(r.b, "SI", LOGIC_0);
    bench_set(r.b, "SCK", mode == 3 ? LOGIC_1 : LOGIC_0);
    bench_run(r.b, NS(1000));
    return r;
}

static void rig_close(Rig *r) {
    bench_destroy(r->b);
}

static void cs_low(Rig *r) {
    bench_set(r->b, "CS#", LOGIC_0);
    bench_run(r->b, HALF);
}

static void cs_high(Rig *r) {
    if (r->mode == 0) {
        bench_run(r->b, HALF);   /* the last SCK fall settles first */
    }
    bench_set(r->b, "CS#", LOGIC_1);
    bench_run(r->b, HALF);
}

/* One SCK cycle with SI = si; the level SO has before the rising edge. */
static LogicValue bit_clock(Rig *r, LogicValue si) {
    LogicValue so;

    if (r->mode == 3) {
        bench_set(r->b, "SCK", LOGIC_0);
    }
    bench_set(r->b, "SI", si);
    bench_run(r->b, HALF);
    so = bench_get(r->b, "SO");
    bench_set(r->b, "SCK", LOGIC_1);
    bench_run(r->b, HALF);
    if (r->mode == 0) {
        bench_set(r->b, "SCK", LOGIC_0);
    }
    return so;
}

static void byte_clock(Rig *r, uint8_t out, Rx *rx, size_t index) {
    int bit;

    for (bit = 7; bit >= 0; bit--) {
        LogicValue so = bit_clock(r, (out >> bit) & 1 ? LOGIC_1 : LOGIC_0);

        if (rx != NULL) {
            if (so == LOGIC_1) {
                rx->v[index] |= (uint8_t)(1u << bit);
            } else if (so == LOGIC_Z) {
                rx->z[index] |= (uint8_t)(1u << bit);
            } else if (so == LOGIC_X) {
                rx->x[index] |= (uint8_t)(1u << bit);
            }
        }
    }
}

/* One transaction: ntx bytes out, then nrx bytes read (zeros out). */
static void spi(Rig *r, const uint8_t *tx, size_t ntx, size_t nrx, Rx *rx) {
    size_t i;

    if (rx != NULL) {
        memset(rx, 0, sizeof *rx);
    }
    cs_low(r);
    for (i = 0; i < ntx; i++) {
        byte_clock(r, tx[i], NULL, 0);
    }
    for (i = 0; i < nrx; i++) {
        byte_clock(r, 0, rx, i);
    }
    cs_high(r);
}

/* A transaction cut after `bits` bits (the bytes of tx, MSB first). */
static void spi_bits(Rig *r, const uint8_t *tx, unsigned bits) {
    unsigned i;

    cs_low(r);
    for (i = 0; i < bits; i++) {
        bit_clock(r, (tx[i / 8] >> (7 - i % 8)) & 1 ? LOGIC_1 : LOGIC_0);
    }
    cs_high(r);
}

#define SPI(R, RX, NRX, ...)                                                                  \
    do {                                                                                      \
        static const uint8_t tx_[] = {__VA_ARGS__};                                           \
        spi((R), tx_, sizeof tx_, (NRX), (RX));                                               \
    } while (0)
#define SEND(R, ...) SPI((R), NULL, 0, __VA_ARGS__)

static int rx_is(const Rx *rx, size_t i, uint8_t value) {
    return rx->v[i] == value && rx->z[i] == 0 && rx->x[i] == 0;
}
static int rx_z(const Rx *rx, size_t i) {
    return rx->z[i] == 0xFF;
}
static int rx_x(const Rx *rx, size_t i) {
    return rx->x[i] == 0xFF;
}

static void wait_us(Rig *r, double us) {
    bench_run(r->b, (SimTime)(us * 1e6));
}

/* The first RDSR byte (opcode 05). */
static uint8_t rdsr(Rig *r) {
    Rx rx;

    SPI(r, &rx, 1, 0x05);
    return rx.v[0];
}

static Rx rdsr_rx(Rig *r) {
    Rx rx;

    SPI(r, &rx, 1, 0x05);
    return rx;
}

/* The memory model's contents, without the SPI. */
static int mem(Rig *r, uint32_t address, uint8_t *value, uint8_t *known) {
    const SimComponentType *type = kind_model(r->part);
    uint32_t v = 0, k = 0;

    if (type->memory_read(bench_state(r->b), address, 1, &v, &k) != 1) {
        return 0;
    }
    *value = (uint8_t)v;
    *known = (uint8_t)k;
    return 1;
}

static int mem_is(Rig *r, uint32_t address, uint8_t want) {
    uint8_t v, k;

    return mem(r, address, &v, &k) && k == 0xFF && v == want;
}

static int mem_unknown(Rig *r, uint32_t address) {
    uint8_t v, k;

    return mem(r, address, &v, &k) && k == 0;
}

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
    flash_basic(0);
    flash_basic(3);
    flash_protect();
    flash_wp_lock();
    flash_misc();
    flash_hold();
    flash_block();
    circuit_cases();
    fuzz_cases();
    loader_cases();
    printf("%s\n", failures == 0 ? "all serialmem tests passed" : "serialmem tests FAILED");
    part_library_destroy(library);
    return failures == 0 ? 0 : 1;
}
