#ifndef SERIALMEM_BENCH_H
#define SERIALMEM_BENCH_H

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

#define SPI(R, RX, NRX, ...)                                                                  \
    do {                                                                                      \
        static const uint8_t tx_[] = {__VA_ARGS__};                                           \
        spi((R), tx_, sizeof tx_, (NRX), (RX));                                               \
    } while (0)
#define SEND(R, ...) SPI((R), NULL, 0, __VA_ARGS__)

extern int failures;
extern PartLibrary *library;
void expect(int ok, const char *what);
int load_fixtures(void);

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
Rig rig_open(const char *name, unsigned mode);
void rig_close(Rig *r);
void cs_low(Rig *r);
void cs_high(Rig *r);
LogicValue bit_clock(Rig *r, LogicValue si);
void byte_clock(Rig *r, uint8_t out, Rx *rx, size_t index);
void spi(Rig *r, const uint8_t *tx, size_t ntx, size_t nrx, Rx *rx);
void spi_bits(Rig *r, const uint8_t *tx, unsigned bits);
int rx_is(const Rx *rx, size_t i, uint8_t value);
int rx_z(const Rx *rx, size_t i);
int rx_x(const Rx *rx, size_t i);
void wait_us(Rig *r, double us);
uint8_t rdsr(Rig *r);
Rx rdsr_rx(Rig *r);
int mem(Rig *r, uint32_t address, uint8_t *value, uint8_t *known);
int mem_is(Rig *r, uint32_t address, uint8_t want);
int mem_unknown(Rig *r, uint32_t address);

#endif
