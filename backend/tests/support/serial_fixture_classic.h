#ifndef SERIAL_FIXTURE_CLASSIC_H
#define SERIAL_FIXTURE_CLASSIC_H

/*
 * plan-12-H fixtures: the 13 classic-UART devices (models ay1013, i8251,
 * i82510, acia6551, hs82c52, com78c80x) as their serial.json objects will
 * ship (plan-12-D section 3.3): every pinout their parts use, the grades
 * with the spec's timing, the model keys. The fixture copies differ only in
 * the id ("SFX-" + the device id, so they load next to the real devices) and
 * the parts: one part per pinout, SFX-<id>-<pinout> (AY-5-1013A also has
 * the AY-3-1015D variant on "dip40n").
 *
 * Footprints: the real footprint where footprints.json has it. Not there yet
 * (plan-12-H Phase 2 adds them): PLCC-28 (82510, 82C52 plcc28) uses LCCC-28,
 * CDIP-40-600 uses PDIP-40-600, PDIP-48-600 (COM78804) uses SSOP-48, each
 * with the same pin count.
 *
 * Generated from the plan-12-H device list (classic-a.md, classic-b.md);
 * edit both together.
 */

#include <stdio.h>
#include <stdlib.h>

#include "parts.h"

#define SFX_AY_5_1013A \
    "{\"id\":\"SFX-AY-5-1013A\",\"function\":\"UART\",\"description\":\"UAR/T: universal asynchronous receiver/t" \
    "ransmitter\",\"drawer\":[\"Interface\"],\"note\":\"ay-5-1013.pdf p1-13; plan-12-data/specs/classic-a.md " \
    "§ AY-5-1013A\",\"pinouts\":{\"dip40\":[{\"number\":1,\"name\":\"VCC\",\"role\":\"power\"},{\"number\":2,\"name\":\"V" \
    "GG\",\"role\":\"power\"},{\"number\":3,\"name\":\"GND\",\"role\":\"ground\"},{\"number\":4,\"name\":\"RDE\",\"role\":\"i" \
    "nput\"},{\"number\":5,\"name\":\"RD8\",\"role\":\"output\"},{\"number\":6,\"name\":\"RD7\",\"role\":\"output\"},{\"num" \
    "ber\":7,\"name\":\"RD6\",\"role\":\"output\"},{\"number\":8,\"name\":\"RD5\",\"role\":\"output\"},{\"number\":9,\"name" \
    "\":\"RD4\",\"role\":\"output\"},{\"number\":10,\"name\":\"RD3\",\"role\":\"output\"},{\"number\":11,\"name\":\"RD2\",\"r" \
    "ole\":\"output\"},{\"number\":12,\"name\":\"RD1\",\"role\":\"output\"},{\"number\":13,\"name\":\"PE\",\"role\":\"outpu" \
    "t\"},{\"number\":14,\"name\":\"FE\",\"role\":\"output\"},{\"number\":15,\"name\":\"OR\",\"role\":\"output\"},{\"number" \
    "\":16,\"name\":\"SWE\",\"role\":\"input\"},{\"number\":17,\"name\":\"RCP\",\"role\":\"input\"},{\"number\":18,\"name\":" \
    "\"RDAV\",\"role\":\"input\"},{\"number\":19,\"name\":\"DAV\",\"role\":\"output\"},{\"number\":20,\"name\":\"SI\",\"role" \
    "\":\"input\"},{\"number\":21,\"name\":\"XR\",\"role\":\"input\"},{\"number\":22,\"name\":\"TBMT\",\"role\":\"output\"}," \
    "{\"number\":23,\"name\":\"DS\",\"role\":\"input\"},{\"number\":24,\"name\":\"EOC\",\"role\":\"output\"},{\"number\":25" \
    ",\"name\":\"SO\",\"role\":\"output\"},{\"number\":26,\"name\":\"DB1\",\"role\":\"input\"},{\"number\":27,\"name\":\"DB2" \
    "\",\"role\":\"input\"},{\"number\":28,\"name\":\"DB3\",\"role\":\"input\"},{\"number\":29,\"name\":\"DB4\",\"role\":\"in" \
    "put\"},{\"number\":30,\"name\":\"DB5\",\"role\":\"input\"},{\"number\":31,\"name\":\"DB6\",\"role\":\"input\"},{\"numb" \
    "er\":32,\"name\":\"DB7\",\"role\":\"input\"},{\"number\":33,\"name\":\"DB8\",\"role\":\"input\"},{\"number\":34,\"name" \
    "\":\"CS\",\"role\":\"input\"},{\"number\":35,\"name\":\"NP\",\"role\":\"input\"},{\"number\":36,\"name\":\"TSB\",\"role\"" \
    ":\"input\"},{\"number\":37,\"name\":\"NB2\",\"role\":\"input\"},{\"number\":38,\"name\":\"NB1\",\"role\":\"input\"},{\"" \
    "number\":39,\"name\":\"EPS\",\"role\":\"input\"},{\"number\":40,\"name\":\"TCP\",\"role\":\"input\"}],\"dip40n\":[{\"n" \
    "umber\":1,\"name\":\"VCC\",\"role\":\"power\"},{\"number\":2,\"name\":\"NC\",\"role\":\"nc\"},{\"number\":3,\"name\":\"G" \
    "ND\",\"role\":\"ground\"},{\"number\":4,\"name\":\"RDE\",\"role\":\"input\"},{\"number\":5,\"name\":\"RD8\",\"role\":\"o" \
    "utput\"},{\"number\":6,\"name\":\"RD7\",\"role\":\"output\"},{\"number\":7,\"name\":\"RD6\",\"role\":\"output\"},{\"nu" \
    "mber\":8,\"name\":\"RD5\",\"role\":\"output\"},{\"number\":9,\"name\":\"RD4\",\"role\":\"output\"},{\"number\":10,\"na" \
    "me\":\"RD3\",\"role\":\"output\"},{\"number\":11,\"name\":\"RD2\",\"role\":\"output\"},{\"number\":12,\"name\":\"RD1\"," \
    "\"role\":\"output\"},{\"number\":13,\"name\":\"PE\",\"role\":\"output\"},{\"number\":14,\"name\":\"FE\",\"role\":\"outp" \
    "ut\"},{\"number\":15,\"name\":\"OR\",\"role\":\"output\"},{\"number\":16,\"name\":\"SWE\",\"role\":\"input\"},{\"numbe" \
    "r\":17,\"name\":\"RCP\",\"role\":\"input\"},{\"number\":18,\"name\":\"RDAV\",\"role\":\"input\"},{\"number\":19,\"name" \
    "\":\"DAV\",\"role\":\"output\"},{\"number\":20,\"name\":\"SI\",\"role\":\"input\"},{\"number\":21,\"name\":\"XR\",\"role" \
    "\":\"input\"},{\"number\":22,\"name\":\"TBMT\",\"role\":\"output\"},{\"number\":23,\"name\":\"DS\",\"role\":\"input\"}," \
    "{\"number\":24,\"name\":\"EOC\",\"role\":\"output\"},{\"number\":25,\"name\":\"SO\",\"role\":\"output\"},{\"number\":2" \
    "6,\"name\":\"DB1\",\"role\":\"input\"},{\"number\":27,\"name\":\"DB2\",\"role\":\"input\"},{\"number\":28,\"name\":\"DB" \
    "3\",\"role\":\"input\"},{\"number\":29,\"name\":\"DB4\",\"role\":\"input\"},{\"number\":30,\"name\":\"DB5\",\"role\":\"i" \
    "nput\"},{\"number\":31,\"name\":\"DB6\",\"role\":\"input\"},{\"number\":32,\"name\":\"DB7\",\"role\":\"input\"},{\"num" \
    "ber\":33,\"name\":\"DB8\",\"role\":\"input\"},{\"number\":34,\"name\":\"CS\",\"role\":\"input\"},{\"number\":35,\"name" \
    "\":\"NP\",\"role\":\"input\"},{\"number\":36,\"name\":\"TSB\",\"role\":\"input\"},{\"number\":37,\"name\":\"NB2\",\"role" \
    "\":\"input\"},{\"number\":38,\"name\":\"NB1\",\"role\":\"input\"},{\"number\":39,\"name\":\"EPS\",\"role\":\"input\"},{" \
    "\"number\":40,\"name\":\"TCP\",\"role\":\"input\"}]},\"model\":\"ay1013\",\"transmitData\":[\"DB1\",\"DB2\",\"DB3\",\"D" \
    "B4\",\"DB5\",\"DB6\",\"DB7\",\"DB8\"],\"receiveData\":[\"RD1\",\"RD2\",\"RD3\",\"RD4\",\"RD5\",\"RD6\",\"RD7\",\"RD8\"],\"re" \
    "ceiveEnable\":\"RDE\",\"statusEnable\":\"SWE\",\"status\":{\"dav\":\"DAV\",\"tbmt\":\"TBMT\",\"eoc\":\"EOC\",\"or\":\"OR" \
    "\",\"fe\":\"FE\",\"pe\":\"PE\"},\"control\":{\"strobe\":\"CS\",\"noParity\":\"NP\",\"stops\":\"TSB\",\"wordLength\":[\"NB1" \
    "\",\"NB2\"],\"evenParity\":\"EPS\"},\"dataStrobe\":\"DS\",\"resetDav\":\"RDAV\",\"serialOut\":\"SO\",\"serialIn\":\"SI" \
    "\",\"clocks\":{\"transmit\":\"TCP\",\"receive\":\"RCP\"},\"reset\":\"XR\",\"resetClearsData\":false,\"fiveBitStops" \
    "\":2,\"grades\":{\"A\":{\"tpd\":500,\"tds\":500,\"ten\":500,\"tdis\":500,\"tdav\":3000,\"trdav\":500,\"vcc\":[4.75," \
    "5.25],\"note\":\"ay-5-1013.pdf p6-p9 (AY-5-1013A; VGG -12 V on pin 2)\"},\"D\":{\"tpd\":500,\"tds\":1000,\"" \
    "ten\":500,\"tdis\":500,\"tdav\":0,\"trdav\":500,\"vcc\":[4.75,5.25],\"note\":\"ay-5-1013.pdf p10-p13 (AY-3-1" \
    "015D; DAV 1 RCP after the transfer)\"}},\"parts\":[{\"name\":\"SFX-AY-5-1013A-dip40\",\"package\":\"DIP\",\"" \
    "pinout\":\"dip40\",\"footprint\":\"PDIP-40-600\",\"grade\":\"A\"},{\"name\":\"SFX-AY-5-1013A-dip40n\",\"package\"" \
    ":\"DIP\",\"pinout\":\"dip40n\",\"footprint\":\"PDIP-40-600\",\"grade\":\"D\",\"fiveBitStops\":1.5,\"resetClearsDa" \
    "ta\":true}]}"

#define SFX_COM2017 \
    "{\"id\":\"SFX-COM2017\",\"function\":\"UART\",\"description\":\"Universal asynchronous receiver/transmitter" \
    "\",\"drawer\":[\"Interface\"],\"note\":\"com2017.pdf p1-8; plan-12-data/specs/classic-a.md § COM2017\",\"p" \
    "inouts\":{\"dip40\":[{\"number\":1,\"name\":\"VCC\",\"role\":\"power\"},{\"number\":2,\"name\":\"VDD\",\"role\":\"powe" \
    "r\"},{\"number\":3,\"name\":\"GND\",\"role\":\"ground\"},{\"number\":4,\"name\":\"RDE\",\"role\":\"input\"},{\"number\"" \
    ":5,\"name\":\"RD8\",\"role\":\"output\"},{\"number\":6,\"name\":\"RD7\",\"role\":\"output\"},{\"number\":7,\"name\":\"R" \
    "D6\",\"role\":\"output\"},{\"number\":8,\"name\":\"RD5\",\"role\":\"output\"},{\"number\":9,\"name\":\"RD4\",\"role\":\"" \
    "output\"},{\"number\":10,\"name\":\"RD3\",\"role\":\"output\"},{\"number\":11,\"name\":\"RD2\",\"role\":\"output\"},{" \
    "\"number\":12,\"name\":\"RD1\",\"role\":\"output\"},{\"number\":13,\"name\":\"RPE\",\"role\":\"output\"},{\"number\":1" \
    "4,\"name\":\"RFE\",\"role\":\"output\"},{\"number\":15,\"name\":\"ROR\",\"role\":\"output\"},{\"number\":16,\"name\":\"" \
    "SWE\",\"role\":\"input\"},{\"number\":17,\"name\":\"RCP\",\"role\":\"input\"},{\"number\":18,\"name\":\"RDAR\",\"role\"" \
    ":\"input\"},{\"number\":19,\"name\":\"RDA\",\"role\":\"output\"},{\"number\":20,\"name\":\"RSI\",\"role\":\"input\"},{" \
    "\"number\":21,\"name\":\"MR\",\"role\":\"input\"},{\"number\":22,\"name\":\"TBMT\",\"role\":\"output\"},{\"number\":23" \
    ",\"name\":\"TDS\",\"role\":\"input\"},{\"number\":24,\"name\":\"TEOC\",\"role\":\"output\"},{\"number\":25,\"name\":\"T" \
    "SO\",\"role\":\"output\"},{\"number\":26,\"name\":\"TD1\",\"role\":\"input\"},{\"number\":27,\"name\":\"TD2\",\"role\":" \
    "\"input\"},{\"number\":28,\"name\":\"TD3\",\"role\":\"input\"},{\"number\":29,\"name\":\"TD4\",\"role\":\"input\"},{\"n" \
    "umber\":30,\"name\":\"TD5\",\"role\":\"input\"},{\"number\":31,\"name\":\"TD6\",\"role\":\"input\"},{\"number\":32,\"n" \
    "ame\":\"TD7\",\"role\":\"input\"},{\"number\":33,\"name\":\"TD8\",\"role\":\"input\"},{\"number\":34,\"name\":\"CS\",\"r" \
    "ole\":\"input\"},{\"number\":35,\"name\":\"NPB\",\"role\":\"input\"},{\"number\":36,\"name\":\"NSB\",\"role\":\"input\"" \
    "},{\"number\":37,\"name\":\"NDB2\",\"role\":\"input\"},{\"number\":38,\"name\":\"NDB1\",\"role\":\"input\"},{\"number" \
    "\":39,\"name\":\"POE\",\"role\":\"input\"},{\"number\":40,\"name\":\"TCP\",\"role\":\"input\"}]},\"model\":\"ay1013\",\"" \
    "transmitData\":[\"TD1\",\"TD2\",\"TD3\",\"TD4\",\"TD5\",\"TD6\",\"TD7\",\"TD8\"],\"receiveData\":[\"RD1\",\"RD2\",\"RD3\"" \
    ",\"RD4\",\"RD5\",\"RD6\",\"RD7\",\"RD8\"],\"receiveEnable\":\"RDE\",\"statusEnable\":\"SWE\",\"status\":{\"dav\":\"RDA\"" \
    ",\"tbmt\":\"TBMT\",\"eoc\":\"TEOC\",\"or\":\"ROR\",\"fe\":\"RFE\",\"pe\":\"RPE\"},\"control\":{\"strobe\":\"CS\",\"noParity" \
    "\":\"NPB\",\"stops\":\"NSB\",\"wordLength\":[\"NDB1\",\"NDB2\"],\"evenParity\":\"POE\"},\"dataStrobe\":\"TDS\",\"reset" \
    "Dav\":\"RDAR\",\"serialOut\":\"TSO\",\"serialIn\":\"RSI\",\"clocks\":{\"transmit\":\"TCP\",\"receive\":\"RCP\"},\"rese" \
    "t\":\"MR\",\"resetClearsData\":false,\"fiveBitStops\":1.5,\"grades\":{\"X\":{\"tpd\":350,\"tds\":400,\"ten\":350," \
    "\"tdis\":350,\"tdav\":0,\"trdav\":300,\"vcc\":[4.75,5.25],\"note\":\"com2017.pdf p6-p7 (VDD -12 V on pin 2)" \
    "\"}},\"parts\":[{\"name\":\"SFX-COM2017-dip40\",\"package\":\"DIP\",\"pinout\":\"dip40\",\"footprint\":\"PDIP-40-6" \
    "00\",\"grade\":\"X\"}]}"

#define SFX_COM1863 \
    "{\"id\":\"SFX-COM1863\",\"function\":\"UART\",\"description\":\"Universal asynchronous receiver/transmitter" \
    "\",\"drawer\":[\"Interface\"],\"note\":\"com1863.pdf p1-8; plan-12-data/specs/classic-a.md § COM1863\",\"p" \
    "inouts\":{\"dip40\":[{\"number\":1,\"name\":\"VDD\",\"role\":\"power\"},{\"number\":2,\"name\":\"HIACC\",\"role\":\"in" \
    "put\"},{\"number\":3,\"name\":\"GND\",\"role\":\"ground\"},{\"number\":4,\"name\":\"RDE\",\"role\":\"input\"},{\"numbe" \
    "r\":5,\"name\":\"RD8\",\"role\":\"output\"},{\"number\":6,\"name\":\"RD7\",\"role\":\"output\"},{\"number\":7,\"name\":" \
    "\"RD6\",\"role\":\"output\"},{\"number\":8,\"name\":\"RD5\",\"role\":\"output\"},{\"number\":9,\"name\":\"RD4\",\"role\"" \
    ":\"output\"},{\"number\":10,\"name\":\"RD3\",\"role\":\"output\"},{\"number\":11,\"name\":\"RD2\",\"role\":\"output\"}" \
    ",{\"number\":12,\"name\":\"RD1\",\"role\":\"output\"},{\"number\":13,\"name\":\"RPE\",\"role\":\"output\"},{\"number\"" \
    ":14,\"name\":\"RFE\",\"role\":\"output\"},{\"number\":15,\"name\":\"ROR\",\"role\":\"output\"},{\"number\":16,\"name\"" \
    ":\"SWE\",\"role\":\"input\"},{\"number\":17,\"name\":\"RCP\",\"role\":\"input\"},{\"number\":18,\"name\":\"RDAR\",\"rol" \
    "e\":\"input\"},{\"number\":19,\"name\":\"RDA\",\"role\":\"output\"},{\"number\":20,\"name\":\"RSI\",\"role\":\"input\"}" \
    ",{\"number\":21,\"name\":\"MR\",\"role\":\"input\"},{\"number\":22,\"name\":\"TBMT\",\"role\":\"output\"},{\"number\":" \
    "23,\"name\":\"TDS\",\"role\":\"input\"},{\"number\":24,\"name\":\"TEOC\",\"role\":\"output\"},{\"number\":25,\"name\":" \
    "\"TSO\",\"role\":\"output\"},{\"number\":26,\"name\":\"TD1\",\"role\":\"input\"},{\"number\":27,\"name\":\"TD2\",\"role" \
    "\":\"input\"},{\"number\":28,\"name\":\"TD3\",\"role\":\"input\"},{\"number\":29,\"name\":\"TD4\",\"role\":\"input\"},{" \
    "\"number\":30,\"name\":\"TD5\",\"role\":\"input\"},{\"number\":31,\"name\":\"TD6\",\"role\":\"input\"},{\"number\":32," \
    "\"name\":\"TD7\",\"role\":\"input\"},{\"number\":33,\"name\":\"TD8\",\"role\":\"input\"},{\"number\":34,\"name\":\"CS\"," \
    "\"role\":\"input\"},{\"number\":35,\"name\":\"NPB\",\"role\":\"input\"},{\"number\":36,\"name\":\"NSB\",\"role\":\"inpu" \
    "t\"},{\"number\":37,\"name\":\"NDB2\",\"role\":\"input\"},{\"number\":38,\"name\":\"NDB1\",\"role\":\"input\"},{\"numb" \
    "er\":39,\"name\":\"POE\",\"role\":\"input\"},{\"number\":40,\"name\":\"TCP\",\"role\":\"input\"}]},\"model\":\"ay1013\"" \
    ",\"transmitData\":[\"TD1\",\"TD2\",\"TD3\",\"TD4\",\"TD5\",\"TD6\",\"TD7\",\"TD8\"],\"receiveData\":[\"RD1\",\"RD2\",\"RD" \
    "3\",\"RD4\",\"RD5\",\"RD6\",\"RD7\",\"RD8\"],\"receiveEnable\":\"RDE\",\"statusEnable\":\"SWE\",\"status\":{\"dav\":\"RD" \
    "A\",\"tbmt\":\"TBMT\",\"eoc\":\"TEOC\",\"or\":\"ROR\",\"fe\":\"RFE\",\"pe\":\"RPE\"},\"control\":{\"strobe\":\"CS\",\"noPari" \
    "ty\":\"NPB\",\"stops\":\"NSB\",\"wordLength\":[\"NDB1\",\"NDB2\"],\"evenParity\":\"POE\"},\"dataStrobe\":\"TDS\",\"res" \
    "etDav\":\"RDAR\",\"serialOut\":\"TSO\",\"serialIn\":\"RSI\",\"clocks\":{\"transmit\":\"TCP\",\"receive\":\"RCP\",\"hia" \
    "cc\":\"HIACC\"},\"reset\":\"MR\",\"resetClearsData\":true,\"fiveBitStops\":1.5,\"grades\":{\"X\":{\"tpd\":500,\"td" \
    "s\":400,\"ten\":250,\"tdis\":250,\"tdav\":0,\"trdav\":300,\"vcc\":[4.75,5.25],\"note\":\"com1863.pdf p6-p7\"}}," \
    "\"parts\":[{\"name\":\"SFX-COM1863-dip40\",\"package\":\"DIP\",\"pinout\":\"dip40\",\"footprint\":\"PDIP-40-600\"," \
    "\"grade\":\"X\"}]}"

#define SFX_HD_6402 \
    "{\"id\":\"SFX-HD-6402\",\"function\":\"UART\",\"description\":\"CMOS universal asynchronous receiver transm" \
    "itter (UART)\",\"drawer\":[\"Interface\"],\"note\":\"hd-6402.pdf p1-9; plan-12-data/specs/classic-a.md §" \
    " HD-6402\",\"pinouts\":{\"dip40\":[{\"number\":1,\"name\":\"VCC\",\"role\":\"power\"},{\"number\":2,\"name\":\"NC\",\"" \
    "role\":\"nc\"},{\"number\":3,\"name\":\"GND\",\"role\":\"ground\"},{\"number\":4,\"name\":\"RRD\",\"role\":\"input\"},{" \
    "\"number\":5,\"name\":\"RBR8\",\"role\":\"output\"},{\"number\":6,\"name\":\"RBR7\",\"role\":\"output\"},{\"number\":7" \
    ",\"name\":\"RBR6\",\"role\":\"output\"},{\"number\":8,\"name\":\"RBR5\",\"role\":\"output\"},{\"number\":9,\"name\":\"R" \
    "BR4\",\"role\":\"output\"},{\"number\":10,\"name\":\"RBR3\",\"role\":\"output\"},{\"number\":11,\"name\":\"RBR2\",\"ro" \
    "le\":\"output\"},{\"number\":12,\"name\":\"RBR1\",\"role\":\"output\"},{\"number\":13,\"name\":\"PE\",\"role\":\"outpu" \
    "t\"},{\"number\":14,\"name\":\"FE\",\"role\":\"output\"},{\"number\":15,\"name\":\"OE\",\"role\":\"output\"},{\"number" \
    "\":16,\"name\":\"SFD\",\"role\":\"input\"},{\"number\":17,\"name\":\"RRC\",\"role\":\"input\"},{\"number\":18,\"name\":" \
    "\"DRR\",\"role\":\"input\"},{\"number\":19,\"name\":\"DR\",\"role\":\"output\"},{\"number\":20,\"name\":\"RRI\",\"role\"" \
    ":\"input\"},{\"number\":21,\"name\":\"MR\",\"role\":\"input\"},{\"number\":22,\"name\":\"TBRE\",\"role\":\"output\"},{" \
    "\"number\":23,\"name\":\"TBRL\",\"role\":\"input\"},{\"number\":24,\"name\":\"TRE\",\"role\":\"output\"},{\"number\":2" \
    "5,\"name\":\"TRO\",\"role\":\"output\"},{\"number\":26,\"name\":\"TBR1\",\"role\":\"input\"},{\"number\":27,\"name\":\"" \
    "TBR2\",\"role\":\"input\"},{\"number\":28,\"name\":\"TBR3\",\"role\":\"input\"},{\"number\":29,\"name\":\"TBR4\",\"rol" \
    "e\":\"input\"},{\"number\":30,\"name\":\"TBR5\",\"role\":\"input\"},{\"number\":31,\"name\":\"TBR6\",\"role\":\"input\"" \
    "},{\"number\":32,\"name\":\"TBR7\",\"role\":\"input\"},{\"number\":33,\"name\":\"TBR8\",\"role\":\"input\"},{\"number" \
    "\":34,\"name\":\"CRL\",\"role\":\"input\"},{\"number\":35,\"name\":\"PI\",\"role\":\"input\"},{\"number\":36,\"name\":\"" \
    "SBS\",\"role\":\"input\"},{\"number\":37,\"name\":\"CLS2\",\"role\":\"input\"},{\"number\":38,\"name\":\"CLS1\",\"role" \
    "\":\"input\"},{\"number\":39,\"name\":\"EPE\",\"role\":\"input\"},{\"number\":40,\"name\":\"TRC\",\"role\":\"input\"}]}" \
    ",\"model\":\"ay1013\",\"transmitData\":[\"TBR1\",\"TBR2\",\"TBR3\",\"TBR4\",\"TBR5\",\"TBR6\",\"TBR7\",\"TBR8\"],\"rece" \
    "iveData\":[\"RBR1\",\"RBR2\",\"RBR3\",\"RBR4\",\"RBR5\",\"RBR6\",\"RBR7\",\"RBR8\"],\"receiveEnable\":\"RRD\",\"status" \
    "Enable\":\"SFD\",\"status\":{\"dav\":\"DR\",\"tbmt\":\"TBRE\",\"eoc\":\"TRE\",\"or\":\"OE\",\"fe\":\"FE\",\"pe\":\"PE\"},\"con" \
    "trol\":{\"strobe\":\"CRL\",\"noParity\":\"PI\",\"stops\":\"SBS\",\"wordLength\":[\"CLS1\",\"CLS2\"],\"evenParity\":\"E" \
    "PE\"},\"dataStrobe\":\"TBRL\",\"resetDav\":\"DRR\",\"serialOut\":\"TRO\",\"serialIn\":\"RRI\",\"clocks\":{\"transmit" \
    "\":\"TRC\",\"receive\":\"RRC\"},\"reset\":\"MR\",\"resetClearsData\":false,\"fiveBitStops\":1.5,\"grades\":{\"R\":{" \
    "\"tpd\":0,\"tds\":0,\"ten\":160,\"tdis\":160,\"tdav\":0,\"trdav\":0,\"vcc\":[4.5,5.5],\"note\":\"hd-6402.pdf p7 (" \
    "HD-6402R column)\"}},\"parts\":[{\"name\":\"SFX-HD-6402-dip40\",\"package\":\"DIP\",\"pinout\":\"dip40\",\"footp" \
    "rint\":\"PDIP-40-600\",\"grade\":\"R\"}]}"

#define SFX_CDP1854A \
    "{\"id\":\"SFX-CDP1854A\",\"function\":\"UART\",\"description\":\"Programmable universal asynchronous receiv" \
    "er/transmitter (UART)\",\"drawer\":[\"Interface\"],\"note\":\"cdp1854a.pdf p1-17; plan-12-data/specs/cla" \
    "ssic-a.md § CDP1854A\",\"pinouts\":{\"dip40\":[{\"number\":1,\"name\":\"VDD\",\"role\":\"power\"},{\"number\":2,\"" \
    "name\":\"MODE\",\"role\":\"input\"},{\"number\":3,\"name\":\"VSS\",\"role\":\"ground\"},{\"number\":4,\"name\":\"RRD/C" \
    "S2\",\"role\":\"input\"},{\"number\":5,\"name\":\"RBUS7\",\"role\":\"output\"},{\"number\":6,\"name\":\"RBUS6\",\"role" \
    "\":\"output\"},{\"number\":7,\"name\":\"RBUS5\",\"role\":\"output\"},{\"number\":8,\"name\":\"RBUS4\",\"role\":\"outpu" \
    "t\"},{\"number\":9,\"name\":\"RBUS3\",\"role\":\"output\"},{\"number\":10,\"name\":\"RBUS2\",\"role\":\"output\"},{\"n" \
    "umber\":11,\"name\":\"RBUS1\",\"role\":\"output\"},{\"number\":12,\"name\":\"RBUS0\",\"role\":\"output\"},{\"number\"" \
    ":13,\"name\":\"PE/INT\",\"role\":\"output\"},{\"number\":14,\"name\":\"FE\",\"role\":\"output\"},{\"number\":15,\"nam" \
    "e\":\"OE/PE_OE\",\"role\":\"output\"},{\"number\":16,\"name\":\"SFD/RSEL\",\"role\":\"input\"},{\"number\":17,\"name" \
    "\":\"RCLOCK\",\"role\":\"input\"},{\"number\":18,\"name\":\"DAR/TPB\",\"role\":\"input\"},{\"number\":19,\"name\":\"DA" \
    "\",\"role\":\"output\"},{\"number\":20,\"name\":\"SDI\",\"role\":\"input\"},{\"number\":21,\"name\":\"MR/CLEAR\",\"rol" \
    "e\":\"input\"},{\"number\":22,\"name\":\"THRE\",\"role\":\"output\"},{\"number\":23,\"name\":\"THRL/CS1\",\"role\":\"i" \
    "nput\"},{\"number\":24,\"name\":\"TSRE/RTS\",\"role\":\"output\"},{\"number\":25,\"name\":\"SDO\",\"role\":\"output\"" \
    "},{\"number\":26,\"name\":\"TBUS0\",\"role\":\"input\"},{\"number\":27,\"name\":\"TBUS1\",\"role\":\"input\"},{\"numb" \
    "er\":28,\"name\":\"TBUS2\",\"role\":\"input\"},{\"number\":29,\"name\":\"TBUS3\",\"role\":\"input\"},{\"number\":30,\"" \
    "name\":\"TBUS4\",\"role\":\"input\"},{\"number\":31,\"name\":\"TBUS5\",\"role\":\"input\"},{\"number\":32,\"name\":\"T" \
    "BUS6\",\"role\":\"input\"},{\"number\":33,\"name\":\"TBUS7\",\"role\":\"input\"},{\"number\":34,\"name\":\"CRL/RD_WR" \
    "\",\"role\":\"input\"},{\"number\":35,\"name\":\"PI/CS3\",\"role\":\"input\"},{\"number\":36,\"name\":\"SBS\",\"role\":" \
    "\"input\"},{\"number\":37,\"name\":\"WLS2/PSI\",\"role\":\"input\"},{\"number\":38,\"name\":\"WLS1/ES\",\"role\":\"in" \
    "put\"},{\"number\":39,\"name\":\"EPE/CTS\",\"role\":\"input\"},{\"number\":40,\"name\":\"TCLOCK\",\"role\":\"input\"}" \
    "]},\"model\":\"ay1013\",\"transmitData\":[\"TBUS0\",\"TBUS1\",\"TBUS2\",\"TBUS3\",\"TBUS4\",\"TBUS5\",\"TBUS6\",\"TBU" \
    "S7\"],\"receiveData\":[\"RBUS0\",\"RBUS1\",\"RBUS2\",\"RBUS3\",\"RBUS4\",\"RBUS5\",\"RBUS6\",\"RBUS7\"],\"receiveEna" \
    "ble\":\"RRD/CS2\",\"statusEnable\":\"SFD/RSEL\",\"status\":{\"dav\":\"DA\",\"tbmt\":\"THRE\",\"eoc\":\"TSRE/RTS\",\"or" \
    "\":\"OE/PE_OE\",\"fe\":\"FE\",\"pe\":\"PE/INT\"},\"control\":{\"strobe\":\"CRL/RD_WR\",\"noParity\":\"PI/CS3\",\"stops" \
    "\":\"SBS\",\"wordLength\":[\"WLS1/ES\",\"WLS2/PSI\"],\"evenParity\":\"EPE/CTS\"},\"dataStrobe\":\"THRL/CS1\",\"res" \
    "etDav\":\"DAR/TPB\",\"serialOut\":\"SDO\",\"serialIn\":\"SDI\",\"clocks\":{\"transmit\":\"TCLOCK\",\"receive\":\"RCL" \
    "OCK\"},\"reset\":\"MR/CLEAR\",\"resetClearsData\":true,\"fiveBitStops\":1.5,\"resetClearsControl\":true,\"cp" \
    "uMode\":{\"modePin\":\"MODE\",\"select1\":\"THRL/CS1\",\"select2\":\"RRD/CS2\",\"select3\":\"PI/CS3\",\"registerSe" \
    "lect\":\"SFD/RSEL\",\"readWrite\":\"CRL/RD_WR\",\"tpb\":\"DAR/TPB\",\"cts\":\"EPE/CTS\",\"psi\":\"WLS2/PSI\",\"es\":\"" \
    "WLS1/ES\"},\"grades\":{\"5V\":{\"tpd\":300,\"tds\":200,\"ten\":75,\"tdis\":200,\"tdav\":225,\"trdav\":150,\"tacc\":" \
    "200,\"tint\":220,\"vcc\":[4.0,10.5],\"note\":\"cdp1854a.pdf p8-p16 (5 V typical column)\"}},\"parts\":[{\"n" \
    "ame\":\"SFX-CDP1854A-dip40\",\"package\":\"DIP\",\"pinout\":\"dip40\",\"footprint\":\"PDIP-40-600\",\"grade\":\"5V" \
    "\"}]}"

#define SFX_8251A \
    "{\"id\":\"SFX-8251A\",\"function\":\"UART\",\"description\":\"Programmable communication interface\",\"drawer" \
    "\":[\"Interface\"],\"note\":\"8251a.pdf p2-26; plan-12-data/specs/classic-b.md § 8251A\",\"pinouts\":{\"di" \
    "p28\":[{\"number\":1,\"name\":\"D2\",\"role\":\"inout\"},{\"number\":2,\"name\":\"D3\",\"role\":\"inout\"},{\"number\":" \
    "3,\"name\":\"RXD\",\"role\":\"input\"},{\"number\":4,\"name\":\"GND\",\"role\":\"ground\"},{\"number\":5,\"name\":\"D4\"" \
    ",\"role\":\"inout\"},{\"number\":6,\"name\":\"D5\",\"role\":\"inout\"},{\"number\":7,\"name\":\"D6\",\"role\":\"inout\"}" \
    ",{\"number\":8,\"name\":\"D7\",\"role\":\"inout\"},{\"number\":9,\"name\":\"TXC\",\"role\":\"input\"},{\"number\":10,\"" \
    "name\":\"WR\",\"role\":\"input\"},{\"number\":11,\"name\":\"CS\",\"role\":\"input\"},{\"number\":12,\"name\":\"CD\",\"ro" \
    "le\":\"input\"},{\"number\":13,\"name\":\"RD\",\"role\":\"input\"},{\"number\":14,\"name\":\"RXRDY\",\"role\":\"output" \
    "\"},{\"number\":15,\"name\":\"TXRDY\",\"role\":\"output\"},{\"number\":16,\"name\":\"SYNDET\",\"role\":\"inout\"},{\"n" \
    "umber\":17,\"name\":\"CTS\",\"role\":\"input\"},{\"number\":18,\"name\":\"TXEMPTY\",\"role\":\"output\"},{\"number\":" \
    "19,\"name\":\"TXD\",\"role\":\"output\"},{\"number\":20,\"name\":\"CLK\",\"role\":\"input\"},{\"number\":21,\"name\":\"" \
    "RESET\",\"role\":\"input\"},{\"number\":22,\"name\":\"DSR\",\"role\":\"input\"},{\"number\":23,\"name\":\"RTS\",\"role" \
    "\":\"output\"},{\"number\":24,\"name\":\"DTR\",\"role\":\"output\"},{\"number\":25,\"name\":\"RXC\",\"role\":\"input\"}" \
    ",{\"number\":26,\"name\":\"VCC\",\"role\":\"power\"},{\"number\":27,\"name\":\"D0\",\"role\":\"inout\"},{\"number\":28" \
    ",\"name\":\"D1\",\"role\":\"inout\"}]},\"model\":\"i8251\",\"bus\":{\"data\":[\"D0\",\"D1\",\"D2\",\"D3\",\"D4\",\"D5\",\"D6\"" \
    ",\"D7\"],\"controlData\":\"CD\",\"read\":\"RD\",\"write\":\"WR\",\"select\":\"CS\",\"reset\":\"RESET\"},\"clock\":\"CLK\"," \
    "\"serial\":{\"tx\":\"TXD\",\"rx\":\"RXD\",\"txClock\":\"TXC\",\"rxClock\":\"RXC\",\"txrdy\":\"TXRDY\",\"rxrdy\":\"RXRDY\"," \
    "\"txempty\":\"TXEMPTY\",\"syndet\":\"SYNDET\",\"dsr\":\"DSR\",\"dtr\":\"DTR\",\"cts\":\"CTS\",\"rts\":\"RTS\"},\"grades\":" \
    "{\"C\":{\"trd\":200,\"tdf\":100,\"tclr\":400,\"tdtx\":1000,\"twc\":2560,\"vcc\":[4.5,5.5],\"note\":\"8251a.pdf p1" \
    "8-19 (twc = 8 tCY at tCY 320 ns)\"}},\"parts\":[{\"name\":\"SFX-8251A-dip28\",\"package\":\"DIP\",\"pinout\":" \
    "\"dip28\",\"footprint\":\"CDIP-28-600\",\"grade\":\"C\"}]}"

static const char SERIAL_FIXTURE_CLASSIC[] =
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"serial\",\"devices\":["
    SFX_AY_5_1013A ","
    SFX_COM2017 ","
    SFX_COM1863 ","
    SFX_HD_6402 ","
    SFX_CDP1854A ","
    SFX_8251A
    "]}";

/* Loads the classic fixture devices into a library that has footprints.json. */
static inline int load_serial_fixture_classic(PartLibrary *library) {
    char *error = NULL;
    int result = load_parts(library, SERIAL_FIXTURE_CLASSIC, "serial_fixture_classic.json", &error);

    if (result != 0) {
        fprintf(stderr, "serial classic fixture: %s\n", error != NULL ? error : "(no message)");
    }
    free(error);
    return result;
}

#endif
