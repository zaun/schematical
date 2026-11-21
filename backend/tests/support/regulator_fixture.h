#ifndef REGULATOR_FIXTURE_H
#define REGULATOR_FIXTURE_H

/*
 * Fixture parts for regulator_test (plan-42-G section 4), loaded next to the
 * real part files (footprints from footprints.json):
 *
 *   RFX-LDO33  linear, pins 1 IN, 2 GND, 3 EN, 4 PG, 5 OUT: vout 3.3, dropout 0.3, vinMax 6
 *   RFX-LDO5   linear, pins 1 IN, 2 GND, 3 OUT: vout 5, dropout 2, vinMax 35
 *   RFX-ADJ    linear, pins 1 ADJ, 2 OUT, 3 IN, adjustTo output: vref 1.25, dropout 2, vinMax 40
 *   RFX-FB     linear, pins 1 IN, 2 OUT, 3 FB, 4 GND, 5 EN (active low): vref 0.8, dropout 0.2, vinMax 6
 *   RFX-BOOST  boost, pins 1 VIN, 2 SW, 3 VOUT, 4 GND: vout 5, vinMin 0.9, vinMax 5.5
 *   RFX-DUAL   two linear, pins 1 IN1, 2 IN2, 3 OUT1, 4 OUT2, 5 EN1, 6 EN2, 7 GND: vout [1.8, 3.3],
 *              dropout 0.2, vinMax 6
 *   RFX-GEN    linear, pins 1 IN, 2 GND, 3 OUT: vout 5, dropout 2 (no vinMax)
 *   SFX-0V5    a 0.5 V supply (the library's lowest supply is 1.2 V)
 *
 * REGULATOR_DEVICE builds one device for the rejection cases.
 */

#include <stdio.h>
#include <stdlib.h>

#include "parts.h"

#define RFX_PIN(N, NAME, ROLE) "{\"number\":" #N ",\"name\":\"" NAME "\",\"role\":\"" ROLE "\"}"
#define RFX_IN(N, NAME) RFX_PIN(N, NAME, "input")
#define RFX_OUT(N, NAME) RFX_PIN(N, NAME, "output")
#define RFX_PWR(N, NAME) RFX_PIN(N, NAME, "power")
#define RFX_GND(N, NAME) RFX_PIN(N, NAME, "ground")
#define RFX_IO(N, NAME) RFX_PIN(N, NAME, "inout")
#define RFX_EL(TYPE, IN, OUT, EXTRA) \
    "{\"type\":\"" TYPE "\",\"in\":\"" IN "\",\"out\":\"" OUT "\"" EXTRA "}"
#define REGULATOR_DEVICE(ID, PINS, ELEMENTS, ANALOG, NAME, FOOTPRINT, PART_KEYS)                  \
    "{\"id\":\"" ID "\",\"function\":\"regulator\",\"description\":\"fixture\","                  \
    "\"drawer\":[],\"pinouts\":{\"p\":[" PINS "]},\"regulators\":[" ELEMENTS "],"                 \
    "\"analog\":[" ANALOG "],"                                                                    \
    "\"parts\":[{\"name\":\"" NAME "\",\"package\":\"SOT-23\",\"pinout\":\"p\","                  \
    "\"footprint\":\"" FOOTPRINT "\"" PART_KEYS "}]}"
#define REGULATOR_FILE(DEVICES)                                                                   \
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"regulator\",\"devices\":[" DEVICES "]}"

static const char REGULATOR_FIXTURE[] = REGULATOR_FILE(
    REGULATOR_DEVICE("rfx-ldo33",
                     RFX_PWR(1, "IN") "," RFX_GND(2, "GND") "," RFX_IN(3, "EN") "," RFX_OUT(4, "PG") ","
                     RFX_OUT(5, "OUT"),
                     RFX_EL("linear", "IN", "OUT", ",\"enable\":\"EN\",\"pg\":\"PG\""), "", "RFX-LDO33",
                     "PINHDR-1X05-V", ",\"vout\":3.3,\"dropout\":0.3,\"vinMax\":6") ","
    REGULATOR_DEVICE("rfx-ldo5", RFX_PWR(1, "IN") "," RFX_GND(2, "GND") "," RFX_OUT(3, "OUT"),
                     RFX_EL("linear", "IN", "OUT", ""), "", "RFX-LDO5", "PINHDR-1X03-V",
                     ",\"vout\":5,\"dropout\":2,\"vinMax\":35") ","
    REGULATOR_DEVICE("rfx-adj", RFX_IN(1, "ADJ") "," RFX_OUT(2, "OUT") "," RFX_PWR(3, "IN"),
                     RFX_EL("linear", "IN", "OUT", ",\"adjust\":\"ADJ\",\"adjustTo\":\"output\""),
                     "\"ADJ\"", "RFX-ADJ", "PINHDR-1X03-V", ",\"vref\":1.25,\"dropout\":2,\"vinMax\":40") ","
    REGULATOR_DEVICE("rfx-fb",
                     RFX_PWR(1, "IN") "," RFX_OUT(2, "OUT") "," RFX_IN(3, "FB") "," RFX_GND(4, "GND") ","
                     RFX_IN(5, "EN"),
                     RFX_EL("linear", "IN", "OUT", ",\"enable\":\"EN\",\"enableActive\":\"low\",\"adjust\":\"FB\""),
                     "\"FB\"", "RFX-FB", "PINHDR-1X05-V", ",\"vref\":0.8,\"dropout\":0.2,\"vinMax\":6") ","
    REGULATOR_DEVICE("rfx-boost",
                     RFX_PWR(1, "VIN") "," RFX_IO(2, "SW") "," RFX_OUT(3, "VOUT") "," RFX_GND(4, "GND"),
                     RFX_EL("boost", "VIN", "VOUT", ""), "\"SW\"", "RFX-BOOST", "PINHDR-1X04-V",
                     ",\"vout\":5,\"vinMin\":0.9,\"vinMax\":5.5") ","
    REGULATOR_DEVICE("rfx-dual",
                     RFX_PWR(1, "IN1") "," RFX_PWR(2, "IN2") "," RFX_OUT(3, "OUT1") "," RFX_OUT(4, "OUT2") ","
                     RFX_IN(5, "EN1") "," RFX_IN(6, "EN2") "," RFX_GND(7, "GND"),
                     RFX_EL("linear", "IN1", "OUT1", ",\"enable\":\"EN1\"") ","
                     RFX_EL("linear", "IN2", "OUT2", ",\"enable\":\"EN2\""),
                     "", "RFX-DUAL", "PINHDR-1X07-V", ",\"vout\":[1.8,3.3],\"dropout\":0.2,\"vinMax\":6") ","
    REGULATOR_DEVICE("rfx-gen", RFX_PWR(1, "IN") "," RFX_GND(2, "GND") "," RFX_OUT(3, "OUT"),
                     RFX_EL("linear", "IN", "OUT", ""), "", "RFX-GEN", "PINHDR-1X03-V",
                     ",\"vout\":5,\"dropout\":2"));

static const char SUPPLY_FIXTURE[] =
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"supply\",\"devices\":[{\"id\":\"sfx-0v5\","
    "\"function\":\"0V5\",\"description\":\"fixture\",\"drawer\":[],\"pinouts\":{\"pin\":[{\"number\":1,"
    "\"name\":\"VCC\",\"role\":\"power\"}]},\"volts\":0.5,\"parts\":[{\"name\":\"SFX-0V5\","
    "\"manufacturer\":\"Generic\",\"package\":\"\",\"pinout\":\"pin\"}]}]}";

/* Loads the fixture parts; prints the error and returns -1 on failure. */
static int load_regulator_fixture(PartLibrary *library) {
    char *error = NULL;

    if (load_parts(library, REGULATOR_FIXTURE, "regulator-fixture.json", &error) != 0 ||
        load_parts(library, SUPPLY_FIXTURE, "regulator-supply-fixture.json", &error) != 0) {
        fprintf(stderr, "regulator fixture: %s\n", error != NULL ? error : "(none)");
        free(error);
        return -1;
    }
    return 0;
}

#endif
