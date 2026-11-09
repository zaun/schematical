#ifndef SERIAL_FIXTURE_MAXIM_H
#define SERIAL_FIXTURE_MAXIM_H

/*
 * plan-12-J fixtures: the six Maxim device objects as they will ship in
 * serial.json (keys, every pinout their parts use, grades from
 * plan-12-data/specs/maxim.md), plan-12-D section 3.3. Only two things
 * differ: the id ("SFX-" + the device id, so they load next to the real
 * devices) and one fixture part per pinout, SFX-<id>-<pinout>, on an existing
 * footprint of the same pin count (J's own footprints land in Phase 2).
 * The MAX3108's 25-ball WLP has none, so its fixture part uses SFX-WLP-25,
 * a copy of footprints.md's WLP-25 object under a fixture id.
 *
 * Generated from the spec pin tables; pins are the datasheets' names.
 */

#include <stdio.h>
#include <stdlib.h>

#include "parts.h"

#define SFX_MAX3100 \
    "{\"id\":\"SFX-MAX3100\",\"function\":\"Bridge UART\",\"description\":\"SPI/MICROWIRE-compatible UART" \
    " in QSOP-16\",\"drawer\":[\"Interface\"],\"note\":\"max3100.pdf p.1-24; plan-12-data/specs/maxim" \
    ".md § MAX3100\",\"pinouts\":{\"dip14\":[{\"number\":1,\"name\":\"DIN\",\"role\":\"input\"},{\"nu" \
    "mber\":2,\"name\":\"DOUT\",\"role\":\"output\"},{\"number\":3,\"name\":\"SCLK\",\"role\":\"input" \
    "\"},{\"number\":4,\"name\":\"CS\",\"role\":\"input\"},{\"number\":5,\"name\":\"IRQ\",\"role\":\"" \
    "output\"},{\"number\":6,\"name\":\"SHDN\",\"role\":\"input\"},{\"number\":7,\"name\":\"GND\",\"r" \
    "ole\":\"ground\"},{\"number\":8,\"name\":\"X2\",\"role\":\"output\",\"group\":\"crystal\",\"side" \
    "\":\"in\"},{\"number\":9,\"name\":\"X1\",\"role\":\"input\",\"group\":\"crystal\"},{\"number\":1" \
    "0,\"name\":\"CTS\",\"role\":\"input\"},{\"number\":11,\"name\":\"RTS\",\"role\":\"output\"},{\"n" \
    "umber\":12,\"name\":\"RX\",\"role\":\"input\"},{\"number\":13,\"name\":\"TX\",\"role\":\"output" \
    "\"},{\"number\":14,\"name\":\"VCC\",\"role\":\"power\"}],\"qsop16\":[{\"number\":1,\"name\":\"DI" \
    "N\",\"role\":\"input\"},{\"number\":2,\"name\":\"DOUT\",\"role\":\"output\"},{\"number\":3,\"nam" \
    "e\":\"SCLK\",\"role\":\"input\"},{\"number\":4,\"name\":\"CS\",\"role\":\"input\"},{\"number\":5" \
    ",\"name\":\"NC\",\"role\":\"nc\"},{\"number\":6,\"name\":\"IRQ\",\"role\":\"output\"},{\"number" \
    "\":7,\"name\":\"SHDN\",\"role\":\"input\"},{\"number\":8,\"name\":\"GND\",\"role\":\"ground\"},{" \
    "\"number\":9,\"name\":\"X2\",\"role\":\"output\",\"group\":\"crystal\",\"side\":\"in\"},{\"numbe" \
    "r\":10,\"name\":\"X1\",\"role\":\"input\",\"group\":\"crystal\"},{\"number\":11,\"name\":\"CTS\"" \
    ",\"role\":\"input\"},{\"number\":12,\"name\":\"NC\",\"role\":\"nc\"},{\"number\":13,\"name\":\"R" \
    "TS\",\"role\":\"output\"},{\"number\":14,\"name\":\"RX\",\"role\":\"input\"},{\"number\":15,\"na" \
    "me\":\"TX\",\"role\":\"output\"},{\"number\":16,\"name\":\"VCC\",\"role\":\"power\"}],\"tqfn24-4" \
    "x4\":[{\"number\":1,\"name\":\"NC\",\"role\":\"nc\"},{\"number\":2,\"name\":\"DOUT\",\"role\":\"" \
    "output\"},{\"number\":3,\"name\":\"SCLK\",\"role\":\"input\"},{\"number\":4,\"name\":\"CS\",\"ro" \
    "le\":\"input\"},{\"number\":5,\"name\":\"IRQ\",\"role\":\"output\"},{\"number\":6,\"name\":\"NC" \
    "\",\"role\":\"nc\"},{\"number\":7,\"name\":\"NC\",\"role\":\"nc\"},{\"number\":8,\"name\":\"SHDN" \
    "\",\"role\":\"input\"},{\"number\":9,\"name\":\"GND\",\"role\":\"ground\"},{\"number\":10,\"name" \
    "\":\"X2\",\"role\":\"output\",\"group\":\"crystal\",\"side\":\"in\"},{\"number\":11,\"name\":\"X" \
    "1\",\"role\":\"input\",\"group\":\"crystal\"},{\"number\":12,\"name\":\"NC\",\"role\":\"nc\"},{" \
    "\"number\":13,\"name\":\"NC\",\"role\":\"nc\"},{\"number\":14,\"name\":\"NC\",\"role\":\"nc\"},{" \
    "\"number\":15,\"name\":\"CTS\",\"role\":\"input\"},{\"number\":16,\"name\":\"RTS\",\"role\":\"ou" \
    "tput\"},{\"number\":17,\"name\":\"RX\",\"role\":\"input\"},{\"number\":18,\"name\":\"NC\",\"role" \
    "\":\"nc\"},{\"number\":19,\"name\":\"NC\",\"role\":\"nc\"},{\"number\":20,\"name\":\"NC\",\"role" \
    "\":\"nc\"},{\"number\":21,\"name\":\"TX\",\"role\":\"output\"},{\"number\":22,\"name\":\"VCC\"," \
    "\"role\":\"power\"},{\"number\":23,\"name\":\"DIN\",\"role\":\"input\"},{\"number\":24,\"name\":" \
    "\"NC\",\"role\":\"nc\"}]},\"model\":\"max3100\",\"bus\":{\"style\":\"spi\",\"cs\":\"CS\",\"sclk" \
    "\":\"SCLK\",\"mosi\":\"DIN\",\"miso\":\"DOUT\",\"mode\":0},\"clock\":{\"in\":\"X1\",\"out\":\"X2" \
    "\"},\"channels\":[{\"tx\":\"TX\",\"rx\":\"RX\",\"rts\":\"RTS\",\"cts\":\"CTS\",\"int\":\"IRQ\"}]" \
    ",\"shutdown\":\"SHDN\",\"testMode\":\"loopback\",\"grades\":{\"C\":{\"tdv\":100,\"ttr\":100,\"td" \
    "o\":100,\"vcc\":[2.7,5.5],\"note\":\"max3100.pdf p.2-3, 0 to +70 C\"},\"E\":{\"tdv\":100,\"ttr\"" \
    ":100,\"tdo\":100,\"vcc\":[2.7,5.5],\"note\":\"max3100.pdf p.2-3, -40 to +85 C\"}},\"parts\":[{\"" \
    "name\":\"SFX-MAX3100-dip14\",\"package\":\"DIP\",\"pinout\":\"dip14\",\"footprint\":\"PDIP-14\"," \
    "\"grade\":\"E\"},{\"name\":\"SFX-MAX3100-qsop16\",\"package\":\"QSOP\",\"pinout\":\"qsop16\",\"f" \
    "ootprint\":\"SSOP-16\",\"grade\":\"E\"},{\"name\":\"SFX-MAX3100-tqfn24-4x4\",\"package\":\"TQFN" \
    "\",\"pinout\":\"tqfn24-4x4\",\"footprint\":\"SSOP-24\",\"grade\":\"E\"}]}"

#define SFX_MAX3140 \
    "{\"id\":\"SFX-MAX3140\",\"function\":\"Bridge UART\",\"description\":\"SPI/MICROWIRE-compatible UART" \
    " with integrated true fail-safe RS-485/RS-422 transceivers\",\"drawer\":[\"Interface\"],\"note\"" \
    ":\"max3140.pdf p.1-36; plan-12-data/specs/maxim.md § MAX3140\",\"pinouts\":{\"qsop28\":[{\"numbe" \
    "r\":1,\"name\":\"X2\",\"role\":\"output\",\"group\":\"crystal\",\"side\":\"in\"},{\"number\":2," \
    "\"name\":\"X1\",\"role\":\"input\",\"group\":\"crystal\"},{\"number\":3,\"name\":\"CTS\",\"role" \
    "\":\"input\"},{\"number\":4,\"name\":\"RTS\",\"role\":\"output\"},{\"number\":5,\"name\":\"RX\"," \
    "\"role\":\"input\"},{\"number\":6,\"name\":\"TX\",\"role\":\"output\"},{\"number\":7,\"name\":\"" \
    "H/F\",\"role\":\"input\"},{\"number\":8,\"name\":\"GND\",\"role\":\"ground\"},{\"number\":9,\"na" \
    "me\":\"RO\",\"role\":\"output\"},{\"number\":10,\"name\":\"RE\",\"role\":\"input\"},{\"number\":" \
    "11,\"name\":\"DE\",\"role\":\"input\"},{\"number\":12,\"name\":\"DI\",\"role\":\"input\"},{\"num" \
    "ber\":13,\"name\":\"SRL\",\"role\":\"input\"},{\"number\":14,\"name\":\"NC\",\"role\":\"nc\"},{" \
    "\"number\":15,\"name\":\"TXP\",\"role\":\"input\"},{\"number\":16,\"name\":\"Y\",\"role\":\"inou" \
    "t\"},{\"number\":17,\"name\":\"NC\",\"role\":\"nc\"},{\"number\":18,\"name\":\"Z\",\"role\":\"in" \
    "out\"},{\"number\":19,\"name\":\"B\",\"role\":\"input\"},{\"number\":20,\"name\":\"A\",\"role\":" \
    "\"input\"},{\"number\":21,\"name\":\"RXP\",\"role\":\"input\"},{\"number\":22,\"name\":\"VCC\"," \
    "\"role\":\"power\"},{\"number\":23,\"name\":\"DIN\",\"role\":\"input\"},{\"number\":24,\"name\":" \
    "\"DOUT\",\"role\":\"output\"},{\"number\":25,\"name\":\"SCLK\",\"role\":\"input\"},{\"number\":2" \
    "6,\"name\":\"CS\",\"role\":\"input\"},{\"number\":27,\"name\":\"IRQ\",\"role\":\"output\"},{\"nu" \
    "mber\":28,\"name\":\"SHDN\",\"role\":\"input\"}]},\"model\":\"max3100\",\"bus\":{\"style\":\"spi" \
    "\",\"cs\":\"CS\",\"sclk\":\"SCLK\",\"mosi\":\"DIN\",\"miso\":\"DOUT\",\"mode\":0},\"clock\":{\"i" \
    "n\":\"X1\",\"out\":\"X2\"},\"channels\":[{\"tx\":\"TX\",\"rx\":\"RX\",\"rts\":\"RTS\",\"cts\":\"" \
    "CTS\",\"int\":\"IRQ\"}],\"shutdown\":\"SHDN\",\"testMode\":\"txLow\",\"transceiver\":{\"y\":\"Y" \
    "\",\"z\":\"Z\",\"a\":\"A\",\"b\":\"B\",\"de\":\"DE\",\"re\":\"RE\",\"ro\":\"RO\",\"di\":\"DI\"," \
    "\"hf\":\"H/F\",\"srl\":\"SRL\",\"txp\":\"TXP\",\"rxp\":\"RXP\"},\"grades\":{\"C\":{\"tdv\":100," \
    "\"ttr\":100,\"tdo\":100,\"tdp115\":2030,\"tdp500\":720,\"tdp10m\":34,\"tdz115\":3500,\"tdz500\":" \
    "2500,\"tdz10m\":150,\"tdd\":100,\"trp\":127,\"trp10m\":106,\"trz\":20,\"tshdn\":200,\"tdzs115\":" \
    "6000,\"tdzs500\":4500,\"tdzs10m\":250,\"trzs\":3500,\"vcc\":[4.75,5.25],\"note\":\"max3140.pdf p" \
    ".2-7, 0 to +70 C; transceiver typical values\"},\"E\":{\"tdv\":100,\"ttr\":100,\"tdo\":100,\"tdp" \
    "115\":2030,\"tdp500\":720,\"tdp10m\":34,\"tdz115\":3500,\"tdz500\":2500,\"tdz10m\":150,\"tdd\":1" \
    "00,\"trp\":127,\"trp10m\":106,\"trz\":20,\"tshdn\":200,\"tdzs115\":6000,\"tdzs500\":4500,\"tdzs1" \
    "0m\":250,\"trzs\":3500,\"vcc\":[4.75,5.25],\"note\":\"max3140.pdf p.2-7, -40 to +85 C; transceiv" \
    "er typical values\"}},\"parts\":[{\"name\":\"SFX-MAX3140-qsop28\",\"package\":\"QSOP\",\"pinout" \
    "\":\"qsop28\",\"footprint\":\"SOIC-28W\",\"grade\":\"E\"}]}"

#define SFX_MAX3107 \
    "{\"id\":\"SFX-MAX3107\",\"function\":\"Bridge UART\",\"description\":\"SPI/I2C UART with 128-word FI" \
    "FOs\",\"drawer\":[\"Interface\"],\"note\":\"max3107.pdf p.1-52; plan-12-data/specs/maxim.md § MA" \
    "X3107\",\"pinouts\":{\"ssop24\":[{\"number\":1,\"name\":\"XIN\",\"role\":\"input\",\"group\":\"c" \
    "rystal\"},{\"number\":2,\"name\":\"AGND\",\"role\":\"ground\"},{\"number\":3,\"name\":\"VA\",\"r" \
    "ole\":\"power\"},{\"number\":4,\"name\":\"V18\",\"role\":\"power\"},{\"number\":5,\"name\":\"I2C" \
    "/SPI\",\"role\":\"input\"},{\"number\":6,\"name\":\"LDOEN\",\"role\":\"input\"},{\"number\":7,\"" \
    "name\":\"DOUT/SDA\",\"role\":\"inout\"},{\"number\":8,\"name\":\"SCLK/SCL\",\"role\":\"input\"}," \
    "{\"number\":9,\"name\":\"CS/A0\",\"role\":\"input\"},{\"number\":10,\"name\":\"DIN/A1\",\"role\"" \
    ":\"input\"},{\"number\":11,\"name\":\"IRQ\",\"role\":\"output\"},{\"number\":12,\"name\":\"RST\"" \
    ",\"role\":\"input\"},{\"number\":13,\"name\":\"VL\",\"role\":\"power\"},{\"number\":14,\"name\":" \
    "\"DGND\",\"role\":\"ground\"},{\"number\":15,\"name\":\"GPIO0\",\"role\":\"inout\"},{\"number\":" \
    "16,\"name\":\"GPIO1\",\"role\":\"inout\"},{\"number\":17,\"name\":\"GPIO2\",\"role\":\"inout\"}," \
    "{\"number\":18,\"name\":\"GPIO3\",\"role\":\"inout\"},{\"number\":19,\"name\":\"CTS\",\"role\":" \
    "\"input\"},{\"number\":20,\"name\":\"RTS/CLKOUT\",\"role\":\"output\"},{\"number\":21,\"name\":" \
    "\"RX\",\"role\":\"input\"},{\"number\":22,\"name\":\"TX\",\"role\":\"output\"},{\"number\":23,\"" \
    "name\":\"VEXT\",\"role\":\"power\"},{\"number\":24,\"name\":\"XOUT\",\"role\":\"output\",\"group" \
    "\":\"crystal\",\"side\":\"in\"}],\"tqfn24-3.5\":[{\"number\":1,\"name\":\"V18\",\"role\":\"power" \
    "\"},{\"number\":2,\"name\":\"I2C/SPI\",\"role\":\"input\"},{\"number\":3,\"name\":\"LDOEN\",\"ro" \
    "le\":\"input\"},{\"number\":4,\"name\":\"DOUT/SDA\",\"role\":\"inout\"},{\"number\":5,\"name\":" \
    "\"SCLK/SCL\",\"role\":\"input\"},{\"number\":6,\"name\":\"CS/A0\",\"role\":\"input\"},{\"number" \
    "\":7,\"name\":\"DIN/A1\",\"role\":\"input\"},{\"number\":8,\"name\":\"IRQ\",\"role\":\"output\"}" \
    ",{\"number\":9,\"name\":\"RST\",\"role\":\"input\"},{\"number\":10,\"name\":\"VL\",\"role\":\"po" \
    "wer\"},{\"number\":11,\"name\":\"DGND\",\"role\":\"ground\"},{\"number\":12,\"name\":\"GPIO0\"," \
    "\"role\":\"inout\"},{\"number\":13,\"name\":\"GPIO1\",\"role\":\"inout\"},{\"number\":14,\"name" \
    "\":\"GPIO2\",\"role\":\"inout\"},{\"number\":15,\"name\":\"GPIO3\",\"role\":\"inout\"},{\"number" \
    "\":16,\"name\":\"CTS\",\"role\":\"input\"},{\"number\":17,\"name\":\"RTS/CLKOUT\",\"role\":\"out" \
    "put\"},{\"number\":18,\"name\":\"RX\",\"role\":\"input\"},{\"number\":19,\"name\":\"TX\",\"role" \
    "\":\"output\"},{\"number\":20,\"name\":\"VEXT\",\"role\":\"power\"},{\"number\":21,\"name\":\"XO" \
    "UT\",\"role\":\"output\",\"group\":\"crystal\",\"side\":\"in\"},{\"number\":22,\"name\":\"XIN\"," \
    "\"role\":\"input\",\"group\":\"crystal\"},{\"number\":23,\"name\":\"AGND\",\"role\":\"ground\"}," \
    "{\"number\":24,\"name\":\"VA\",\"role\":\"power\"}]},\"model\":\"max310x\",\"variant\":\"max3107" \
    "\",\"bus\":{\"style\":\"i2c-spi\",\"selectPin\":\"I2C/SPI\",\"spiLevel\":1,\"selectSampled\":\"l" \
    "ive\",\"spi\":{\"cs\":\"CS/A0\",\"sclk\":\"SCLK/SCL\",\"mosi\":\"DIN/A1\",\"miso\":\"DOUT/SDA\"," \
    "\"mode\":0},\"i2c\":{\"scl\":\"SCLK/SCL\",\"sda\":\"DOUT/SDA\",\"address\":[\"CS/A0\",\"DIN/A1\"" \
    "],\"table\":[44,45,null,null,46,47,null,null,null,null,null,null,null,null,null,null]}},\"reset" \
    "\":\"RST\",\"clock\":{\"in\":\"XIN\",\"out\":\"XOUT\"},\"channels\":[{\"tx\":\"TX\",\"rx\":\"RX" \
    "\",\"rts\":\"RTS/CLKOUT\",\"cts\":\"CTS\"}],\"interrupt\":\"IRQ\",\"gpio\":[\"GPIO0\",\"GPIO1\"," \
    "\"GPIO2\",\"GPIO3\"],\"grades\":{\"E\":{\"tdo\":20,\"tft\":10,\"vcc\":[2.35,3.6],\"note\":\"max3" \
    "107.pdf p.6-9, -40 to +85 C; VA (VL and VEXT 1.71-3.6 V)\"}},\"parts\":[{\"name\":\"SFX-MAX3107-" \
    "ssop24\",\"package\":\"SSOP\",\"pinout\":\"ssop24\",\"footprint\":\"SSOP-24\",\"grade\":\"E\"},{" \
    "\"name\":\"SFX-MAX3107-tqfn24-3.5\",\"package\":\"TQFN\",\"pinout\":\"tqfn24-3.5\",\"footprint\"" \
    ":\"TSSOP-24\",\"grade\":\"E\"}]}"

#define SFX_MAX3108 \
    "{\"id\":\"SFX-MAX3108\",\"function\":\"Bridge UART\",\"description\":\"SPI/I2C UART with 128-word FI" \
    "FOs in WLP\",\"drawer\":[\"Interface\"],\"note\":\"max3108.pdf p.1-56; plan-12-data/specs/maxim." \
    "md § MAX3108\",\"pinouts\":{\"wlp25\":[{\"ball\":\"A1\",\"name\":\"VCC\",\"role\":\"power\"},{\"" \
    "ball\":\"A2\",\"name\":\"MISO/SDA\",\"role\":\"inout\"},{\"ball\":\"A3\",\"name\":\"CS/A0\",\"ro" \
    "le\":\"input\"},{\"ball\":\"A4\",\"name\":\"IRQ\",\"role\":\"output\"},{\"ball\":\"A5\",\"name\"" \
    ":\"VL\",\"role\":\"power\"},{\"ball\":\"B1\",\"name\":\"V18\",\"role\":\"power\"},{\"ball\":\"B2" \
    "\",\"name\":\"RST\",\"role\":\"input\"},{\"ball\":\"B3\",\"name\":\"SCLK/SCL\",\"role\":\"input" \
    "\"},{\"ball\":\"B4\",\"name\":\"MOSI/A1\",\"role\":\"input\"},{\"ball\":\"B5\",\"name\":\"DGND\"" \
    ",\"role\":\"ground\"},{\"ball\":\"C1\",\"name\":\"AGND\",\"role\":\"ground\"},{\"ball\":\"C2\"," \
    "\"name\":\"LDOEN\",\"role\":\"input\"},{\"ball\":\"C3\",\"name\":\"NC\",\"role\":\"nc\"},{\"ball" \
    "\":\"C4\",\"name\":\"GPIO0\",\"role\":\"inout\"},{\"ball\":\"C5\",\"name\":\"GPIO1\",\"role\":\"" \
    "inout\"},{\"ball\":\"D1\",\"name\":\"XIN\",\"role\":\"input\",\"group\":\"crystal\"},{\"ball\":" \
    "\"D2\",\"name\":\"XOUT\",\"role\":\"output\",\"group\":\"crystal\",\"side\":\"in\"},{\"ball\":\"" \
    "D3\",\"name\":\"SPI/I2C\",\"role\":\"input\"},{\"ball\":\"D4\",\"name\":\"GPIO3\",\"role\":\"ino" \
    "ut\"},{\"ball\":\"D5\",\"name\":\"CTS\",\"role\":\"input\"},{\"ball\":\"E1\",\"name\":\"VEXT\"," \
    "\"role\":\"power\"},{\"ball\":\"E2\",\"name\":\"GPIO2\",\"role\":\"inout\"},{\"ball\":\"E3\",\"n" \
    "ame\":\"RTS\",\"role\":\"output\"},{\"ball\":\"E4\",\"name\":\"TX\",\"role\":\"output\"},{\"ball" \
    "\":\"E5\",\"name\":\"RX\",\"role\":\"input\"}]},\"model\":\"max310x\",\"variant\":\"max3108\",\"" \
    "bus\":{\"style\":\"i2c-spi\",\"selectPin\":\"SPI/I2C\",\"spiLevel\":1,\"selectSampled\":\"live\"" \
    ",\"spi\":{\"cs\":\"CS/A0\",\"sclk\":\"SCLK/SCL\",\"mosi\":\"MOSI/A1\",\"miso\":\"MISO/SDA\",\"mo" \
    "de\":0},\"i2c\":{\"scl\":\"SCLK/SCL\",\"sda\":\"MISO/SDA\",\"address\":[\"CS/A0\",\"MOSI/A1\"]," \
    "\"table\":[108,97,98,99,100,101,102,103,104,105,106,107,96,109,110,111]}},\"reset\":\"RST\",\"cl" \
    "ock\":{\"in\":\"XIN\",\"out\":\"XOUT\"},\"channels\":[{\"tx\":\"TX\",\"rx\":\"RX\",\"rts\":\"RTS" \
    "\",\"cts\":\"CTS\"}],\"interrupt\":\"IRQ\",\"gpio\":[\"GPIO0\",\"GPIO1\",\"GPIO2\",\"GPIO3\"],\"" \
    "grades\":{\"E\":{\"tdo\":20,\"tft\":10,\"vcc\":[1.71,3.6],\"note\":\"max3108.pdf p.6-10, -40 to " \
    "+85 C (2.35 V min with the PLL)\"}},\"parts\":[{\"name\":\"SFX-MAX3108-wlp25\",\"package\":\"WLP" \
    "\",\"pinout\":\"wlp25\",\"footprint\":\"SFX-WLP-25\",\"grade\":\"E\"}]}"

#define SFX_MAX3109 \
    "{\"id\":\"SFX-MAX3109\",\"function\":\"Bridge UART\",\"description\":\"Dual serial UART with 128-wor" \
    "d FIFOs\",\"drawer\":[\"Interface\"],\"note\":\"max3109.pdf p.1-66; plan-12-data/specs/maxim.md " \
    "§ MAX3109\",\"pinouts\":{\"tqfn32\":[{\"number\":1,\"name\":\"RST\",\"role\":\"input\"},{\"numbe" \
    "r\":2,\"name\":\"MISO/SDA\",\"role\":\"inout\"},{\"number\":3,\"name\":\"SCLK/SCL\",\"role\":\"i" \
    "nput\"},{\"number\":4,\"name\":\"GPIO7\",\"role\":\"inout\"},{\"number\":5,\"name\":\"CS/A0\",\"" \
    "role\":\"input\"},{\"number\":6,\"name\":\"MOSI/A1\",\"role\":\"input\"},{\"number\":7,\"name\":" \
    "\"IRQ\",\"role\":\"output\"},{\"number\":8,\"name\":\"VL\",\"role\":\"power\"},{\"number\":9,\"n" \
    "ame\":\"SPI/I2C\",\"role\":\"input\"},{\"number\":10,\"name\":\"DGND\",\"role\":\"ground\"},{\"n" \
    "umber\":11,\"name\":\"GPIO0\",\"role\":\"inout\"},{\"number\":12,\"name\":\"GPIO4\",\"role\":\"i" \
    "nout\"},{\"number\":13,\"name\":\"GPIO1\",\"role\":\"inout\"},{\"number\":14,\"name\":\"GPIO5\"," \
    "\"role\":\"inout\"},{\"number\":15,\"name\":\"CTS0\",\"role\":\"input\"},{\"number\":16,\"name\"" \
    ":\"CTS1\",\"role\":\"input\"},{\"number\":17,\"name\":\"TX1\",\"role\":\"output\"},{\"number\":1" \
    "8,\"name\":\"TX0\",\"role\":\"output\"},{\"number\":19,\"name\":\"RX0\",\"role\":\"input\"},{\"n" \
    "umber\":20,\"name\":\"RX1\",\"role\":\"input\"},{\"number\":21,\"name\":\"RTS0\",\"role\":\"outp" \
    "ut\"},{\"number\":22,\"name\":\"RTS1\",\"role\":\"output\"},{\"number\":23,\"name\":\"GPIO2\",\"" \
    "role\":\"inout\"},{\"number\":24,\"name\":\"GPIO3\",\"role\":\"inout\"},{\"number\":25,\"name\":" \
    "\"VEXT\",\"role\":\"power\"},{\"number\":26,\"name\":\"XIN\",\"role\":\"input\",\"group\":\"crys" \
    "tal\"},{\"number\":27,\"name\":\"XOUT\",\"role\":\"output\",\"group\":\"crystal\",\"side\":\"in" \
    "\"},{\"number\":28,\"name\":\"GPIO6\",\"role\":\"inout\"},{\"number\":29,\"name\":\"AGND\",\"rol" \
    "e\":\"ground\"},{\"number\":30,\"name\":\"LDOEN\",\"role\":\"input\"},{\"number\":31,\"name\":\"" \
    "V18\",\"role\":\"power\"},{\"number\":32,\"name\":\"VCC\",\"role\":\"power\"}]},\"model\":\"max3" \
    "10x\",\"variant\":\"max3109\",\"bus\":{\"style\":\"i2c-spi\",\"selectPin\":\"SPI/I2C\",\"spiLeve" \
    "l\":1,\"selectSampled\":\"live\",\"spi\":{\"cs\":\"CS/A0\",\"sclk\":\"SCLK/SCL\",\"mosi\":\"MOSI" \
    "/A1\",\"miso\":\"MISO/SDA\",\"mode\":0},\"i2c\":{\"scl\":\"SCLK/SCL\",\"sda\":\"MISO/SDA\",\"add" \
    "ress\":[\"CS/A0\",\"MOSI/A1\"],\"table\":[108,97,98,99,100,101,102,103,104,105,106,107,96,109,11" \
    "0,111]},\"uartOffsets\":[0,-16]},\"reset\":\"RST\",\"clock\":{\"in\":\"XIN\",\"out\":\"XOUT\"}," \
    "\"channels\":[{\"tx\":\"TX0\",\"rx\":\"RX0\",\"rts\":\"RTS0\",\"cts\":\"CTS0\"},{\"tx\":\"TX1\"," \
    "\"rx\":\"RX1\",\"rts\":\"RTS1\",\"cts\":\"CTS1\"}],\"interrupt\":\"IRQ\",\"gpio\":[\"GPIO0\",\"G" \
    "PIO1\",\"GPIO2\",\"GPIO3\",\"GPIO4\",\"GPIO5\",\"GPIO6\",\"GPIO7\"],\"grades\":{\"E\":{\"tdo\":2" \
    "0,\"tft\":10,\"tready\":200000,\"vcc\":[1.71,3.6],\"note\":\"max3109.pdf p.7-11, p.62 (ready abo" \
    "ut 200 us after reset), -40 to +85 C (2.35 V min with the PLL)\"}},\"parts\":[{\"name\":\"SFX-MA" \
    "X3109-tqfn32\",\"package\":\"TQFN\",\"pinout\":\"tqfn32\",\"footprint\":\"PLCC-32\",\"grade\":\"" \
    "E\"}]}"

#define SFX_MAX14830 \
    "{\"id\":\"SFX-MAX14830\",\"function\":\"Bridge UART\",\"description\":\"Quad serial UART with 128-wo" \
    "rd FIFOs\",\"drawer\":[\"Interface\"],\"note\":\"max14830.pdf p.1-68; plan-12-data/specs/maxim.m" \
    "d § MAX14830\",\"pinouts\":{\"tqfn48\":[{\"number\":1,\"name\":\"SPI/I2C\",\"role\":\"input\"},{" \
    "\"number\":2,\"name\":\"LDOEN\",\"role\":\"input\"},{\"number\":3,\"name\":\"MISO/SDA\",\"role\"" \
    ":\"inout\"},{\"number\":4,\"name\":\"SCLK/SCL\",\"role\":\"input\"},{\"number\":5,\"name\":\"CS/" \
    "A0\",\"role\":\"input\"},{\"number\":6,\"name\":\"MOSI/A1\",\"role\":\"input\"},{\"number\":7,\"" \
    "name\":\"IRQ\",\"role\":\"output\"},{\"number\":8,\"name\":\"RST\",\"role\":\"input\"},{\"number" \
    "\":9,\"name\":\"VL\",\"role\":\"power\"},{\"number\":10,\"name\":\"DGND\",\"role\":\"ground\"},{" \
    "\"number\":11,\"name\":\"GPIO0\",\"role\":\"inout\"},{\"number\":12,\"name\":\"GPIO1\",\"role\":" \
    "\"inout\"},{\"number\":13,\"name\":\"GPIO2\",\"role\":\"inout\"},{\"number\":14,\"name\":\"GPIO3" \
    "\",\"role\":\"inout\"},{\"number\":15,\"name\":\"RTS0\",\"role\":\"output\"},{\"number\":16,\"na" \
    "me\":\"CTS0\",\"role\":\"input\"},{\"number\":17,\"name\":\"RX0\",\"role\":\"input\"},{\"number" \
    "\":18,\"name\":\"TX0\",\"role\":\"output\"},{\"number\":19,\"name\":\"GPIO4\",\"role\":\"inout\"" \
    "},{\"number\":20,\"name\":\"GPIO5\",\"role\":\"inout\"},{\"number\":21,\"name\":\"GPIO6\",\"role" \
    "\":\"inout\"},{\"number\":22,\"name\":\"GPIO7\",\"role\":\"inout\"},{\"number\":23,\"name\":\"RT" \
    "S1\",\"role\":\"output\"},{\"number\":24,\"name\":\"CTS1\",\"role\":\"input\"},{\"number\":25,\"" \
    "name\":\"RX1\",\"role\":\"input\"},{\"number\":26,\"name\":\"TX1\",\"role\":\"output\"},{\"numbe" \
    "r\":27,\"name\":\"GPIO8\",\"role\":\"inout\"},{\"number\":28,\"name\":\"GPIO9\",\"role\":\"inout" \
    "\"},{\"number\":29,\"name\":\"GPIO10\",\"role\":\"inout\"},{\"number\":30,\"name\":\"GPIO11\",\"" \
    "role\":\"inout\"},{\"number\":31,\"name\":\"RTS2\",\"role\":\"output\"},{\"number\":32,\"name\":" \
    "\"CTS2\",\"role\":\"input\"},{\"number\":33,\"name\":\"RX2\",\"role\":\"input\"},{\"number\":34," \
    "\"name\":\"TX2\",\"role\":\"output\"},{\"number\":35,\"name\":\"GPIO12\",\"role\":\"inout\"},{\"" \
    "number\":36,\"name\":\"GPIO13\",\"role\":\"inout\"},{\"number\":37,\"name\":\"GPIO14\",\"role\":" \
    "\"inout\"},{\"number\":38,\"name\":\"GPIO15\",\"role\":\"inout\"},{\"number\":39,\"name\":\"RTS3" \
    "\",\"role\":\"output\"},{\"number\":40,\"name\":\"CTS3\",\"role\":\"input\"},{\"number\":41,\"na" \
    "me\":\"RX3\",\"role\":\"input\"},{\"number\":42,\"name\":\"TX3\",\"role\":\"output\"},{\"number" \
    "\":43,\"name\":\"VEXT\",\"role\":\"power\"},{\"number\":44,\"name\":\"XOUT\",\"role\":\"output\"" \
    ",\"group\":\"crystal\",\"side\":\"in\"},{\"number\":45,\"name\":\"XIN\",\"role\":\"input\",\"gro" \
    "up\":\"crystal\"},{\"number\":46,\"name\":\"AGND\",\"role\":\"ground\"},{\"number\":47,\"name\":" \
    "\"VA\",\"role\":\"power\"},{\"number\":48,\"name\":\"V18\",\"role\":\"power\"}]},\"model\":\"max" \
    "310x\",\"variant\":\"max14830\",\"bus\":{\"style\":\"i2c-spi\",\"selectPin\":\"SPI/I2C\",\"spiLe" \
    "vel\":1,\"selectSampled\":\"live\",\"spi\":{\"cs\":\"CS/A0\",\"sclk\":\"SCLK/SCL\",\"mosi\":\"MO" \
    "SI/A1\",\"miso\":\"MISO/SDA\",\"mode\":0},\"i2c\":{\"scl\":\"SCLK/SCL\",\"sda\":\"MISO/SDA\",\"a" \
    "ddress\":[\"CS/A0\",\"MOSI/A1\"],\"table\":[108,97,98,99,100,101,102,103,104,105,106,107,96,109," \
    "110,111]},\"uartOffsets\":[0,-16,-64,-80]},\"reset\":\"RST\",\"clock\":{\"in\":\"XIN\",\"out\":" \
    "\"XOUT\"},\"channels\":[{\"tx\":\"TX0\",\"rx\":\"RX0\",\"rts\":\"RTS0\",\"cts\":\"CTS0\"},{\"tx" \
    "\":\"TX1\",\"rx\":\"RX1\",\"rts\":\"RTS1\",\"cts\":\"CTS1\"},{\"tx\":\"TX2\",\"rx\":\"RX2\",\"rt" \
    "s\":\"RTS2\",\"cts\":\"CTS2\"},{\"tx\":\"TX3\",\"rx\":\"RX3\",\"rts\":\"RTS3\",\"cts\":\"CTS3\"}" \
    "],\"interrupt\":\"IRQ\",\"gpio\":[\"GPIO0\",\"GPIO1\",\"GPIO2\",\"GPIO3\",\"GPIO4\",\"GPIO5\",\"" \
    "GPIO6\",\"GPIO7\",\"GPIO8\",\"GPIO9\",\"GPIO10\",\"GPIO11\",\"GPIO12\",\"GPIO13\",\"GPIO14\",\"G" \
    "PIO15\"],\"grades\":{\"E\":{\"tdo\":20,\"tft\":10,\"vcc\":[2.35,3.6],\"note\":\"max14830.pdf p.8" \
    "-12, -40 to +85 C; VA (VL and VEXT 1.71-3.6 V)\"}},\"parts\":[{\"name\":\"SFX-MAX14830-tqfn48\"," \
    "\"package\":\"TQFN\",\"pinout\":\"tqfn48\",\"footprint\":\"TSSOP-48\",\"grade\":\"E\"}]}"

#define SFX_MAXIM_FOOTPRINTS \
    "{\"format\":\"schematical-footprints\",\"version\":1,\"footprints\":[{\"id\":\"SFX-WLP-25\",\"de" \
    "scription\":\"25-bump WLP (Maxim W252B2+1), 5 x 5 grid, 2.07 x 2.07 mm, 0.4 mm pitch\",\"note\":" \
    "\"fixture copy of footprints.md WLP-25 (lands in plan-12-J Phase 2)\",\"style\":\"grid\",\"pins" \
    "\":25,\"body\":{\"width\":2.07,\"height\":2.07},\"rows\":5,\"columns\":5,\"pitch\":0.4,\"pad\":{" \
    "\"width\":0.2,\"height\":0.2,\"shape\":\"round\"},\"balls\":[\"A1\",\"A2\",\"A3\",\"A4\",\"A5\"," \
    "\"B1\",\"B2\",\"B3\",\"B4\",\"B5\",\"C1\",\"C2\",\"C3\",\"C4\",\"C5\",\"D1\",\"D2\",\"D3\",\"D4" \
    "\",\"D5\",\"E1\",\"E2\",\"E3\",\"E4\",\"E5\"]}]}"

static const char SERIAL_FIXTURE_MAXIM[] =
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"serial\",\"devices\":["
    SFX_MAX3100 "," SFX_MAX3140 "," SFX_MAX3107 "," SFX_MAX3108 "," SFX_MAX3109 "," SFX_MAX14830
    "]}";

/* Loads the fixture footprint and devices into a library that already has
   footprints.json. */
static inline int load_serial_fixture_maxim(PartLibrary *library) {
    static const char footprints[] = SFX_MAXIM_FOOTPRINTS;
    char *error = NULL;
    int result = load_footprints(library, footprints, "serial_fixture_maxim_footprints.json", &error);

    if (result == 0) {
        free(error);
        error = NULL;
        result = load_parts(library, SERIAL_FIXTURE_MAXIM, "serial_fixture_maxim.json", &error);
    }
    if (result != 0) {
        fprintf(stderr, "maxim fixture: %s\n", error != NULL ? error : "(no message)");
    }
    free(error);
    return result;
}

#endif
