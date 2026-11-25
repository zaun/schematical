#ifndef POWERSWITCH_FIXTURE_H
#define POWERSWITCH_FIXTURE_H

/*
 * Fixture parts for powerswitch_test (plan-55-A section 4), loaded next to the
 * real part files (footprints from footprints.json):
 *
 *   PFX-LOAD   load, pins 1 VIN, 2 GND, 3 ON (active high), 4 VOUT: vinMin 1, vinMax 5.5
 *   PFX-ILIM   limited, pins 1 IN, 2 GND, 3 EN (high), 4 OUT, 5 FLG (fault): vinMax 6, ilimit 0.5
 *   PFX-LOW    limited, pins 1 IN, 2 GND, 3 EN (low), 4 OUT, 5 FLG (fault): vinMax 6
 *   PFX-DIODE  ideal-diode, pins 1 VIN, 2 GND, 3 CE (low, against the input), 4 ST (low while
 *              disabled), 5 VOUT: vinMin 1.5, vinMax 5.5
 *   PFX-DUAL   ideal-diode, two: pins 1 VIN1, 2 VIN2, 3 GND, 4 ON (low, logic, shared), 5 ST (analog),
 *              6 VOUT (shared): vinMax 5.5
 *   PFX-QUAD   limited, four: pins 1 IN1 (OUT1, OUT2), 2 IN2 (OUT3, OUT4), 3 GND, 4-7 OUT1-OUT4,
 *              8-11 EN1-EN4 (high), 12-15 OC1-OC4 (fault): vinMax 6
 *   PFX-FREE   load, one with no enable: pins 1 IN, 2 GND, 3 OUT; no part keys
 *
 * POWERSWITCH_DEVICE builds one device for the rejection cases.
 */

#include <stdio.h>
#include <stdlib.h>

#include "parts.h"

#define PFX_PIN(N, NAME, ROLE) "{\"number\":" #N ",\"name\":\"" NAME "\",\"role\":\"" ROLE "\"}"
#define PFX_IN(N, NAME) PFX_PIN(N, NAME, "input")
#define PFX_OUT(N, NAME) PFX_PIN(N, NAME, "output")
#define PFX_PWR(N, NAME) PFX_PIN(N, NAME, "power")
#define PFX_GND(N, NAME) PFX_PIN(N, NAME, "ground")
#define PFX_SW(IN, OUT, EXTRA) "{\"in\":\"" IN "\",\"out\":\"" OUT "\"" EXTRA "}"
#define POWERSWITCH_DEVICE(ID, TYPE, PINS, SWITCHES, ANALOG, NAME, FOOTPRINT, PART_KEYS)          \
    "{\"id\":\"" ID "\",\"function\":\"power switch\",\"description\":\"fixture\","                \
    "\"drawer\":[],\"pinouts\":{\"p\":[" PINS "]},\"type\":\"" TYPE "\",\"switches\":["            \
    SWITCHES "],\"analog\":[" ANALOG "],"                                                         \
    "\"parts\":[{\"name\":\"" NAME "\",\"package\":\"SOT-23\",\"pinout\":\"p\","                  \
    "\"footprint\":\"" FOOTPRINT "\"" PART_KEYS "}]}"
#define POWERSWITCH_FILE(DEVICES)                                                                 \
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"powerswitch\",\"devices\":[" DEVICES "]}"

static const char POWERSWITCH_FIXTURE[] = POWERSWITCH_FILE(
    POWERSWITCH_DEVICE("pfx-load", "load",
                       PFX_PWR(1, "VIN") "," PFX_GND(2, "GND") "," PFX_IN(3, "ON") "," PFX_OUT(4, "VOUT"),
                       PFX_SW("VIN", "VOUT", ",\"enable\":\"ON\""), "", "PFX-LOAD", "PINHDR-1X04-V",
                       ",\"vinMin\":1,\"vinMax\":5.5") ","
    POWERSWITCH_DEVICE("pfx-ilim", "limited",
                       PFX_PWR(1, "IN") "," PFX_GND(2, "GND") "," PFX_IN(3, "EN") "," PFX_OUT(4, "OUT") ","
                       PFX_OUT(5, "FLG"),
                       PFX_SW("IN", "OUT", ",\"enable\":\"EN\",\"flag\":\"FLG\",\"flagWhen\":\"fault\""), "",
                       "PFX-ILIM", "PINHDR-1X05-V", ",\"vinMax\":6,\"ilimit\":0.5") ","
    POWERSWITCH_DEVICE("pfx-low", "limited",
                       PFX_PWR(1, "IN") "," PFX_GND(2, "GND") "," PFX_IN(3, "EN") "," PFX_OUT(4, "OUT") ","
                       PFX_OUT(5, "FLG"),
                       PFX_SW("IN", "OUT", ",\"enable\":\"EN\",\"enableActive\":\"low\",\"flag\":\"FLG\""), "",
                       "PFX-LOW", "PINHDR-1X05-V", ",\"vinMax\":6") ","
    POWERSWITCH_DEVICE("pfx-diode", "ideal-diode",
                       PFX_PWR(1, "VIN") "," PFX_GND(2, "GND") "," PFX_IN(3, "CE") "," PFX_OUT(4, "ST") ","
                       PFX_OUT(5, "VOUT"),
                       PFX_SW("VIN", "VOUT",
                              ",\"enable\":\"CE\",\"enableActive\":\"low\",\"enableRef\":\"input\","
                              "\"flag\":\"ST\",\"flagWhen\":\"disabled\""),
                       "", "PFX-DIODE", "PINHDR-1X05-V", ",\"vinMin\":1.5,\"vinMax\":5.5") ","
    POWERSWITCH_DEVICE("pfx-dual", "ideal-diode",
                       PFX_PWR(1, "VIN1") "," PFX_PWR(2, "VIN2") "," PFX_GND(3, "GND") "," PFX_IN(4, "ON") ","
                       PFX_OUT(5, "ST") "," PFX_OUT(6, "VOUT"),
                       PFX_SW("VIN1", "VOUT", ",\"enable\":\"ON\",\"enableActive\":\"low\"") ","
                       PFX_SW("VIN2", "VOUT", ",\"enable\":\"ON\",\"enableActive\":\"low\""),
                       "\"ST\"", "PFX-DUAL", "PINHDR-1X06-V", ",\"vinMax\":5.5") ","
    POWERSWITCH_DEVICE("pfx-quad", "limited",
                       PFX_PWR(1, "IN1") "," PFX_PWR(2, "IN2") "," PFX_GND(3, "GND") "," PFX_OUT(4, "OUT1") ","
                       PFX_OUT(5, "OUT2") "," PFX_OUT(6, "OUT3") "," PFX_OUT(7, "OUT4") "," PFX_IN(8, "EN1") ","
                       PFX_IN(9, "EN2") "," PFX_IN(10, "EN3") "," PFX_IN(11, "EN4") "," PFX_OUT(12, "OC1") ","
                       PFX_OUT(13, "OC2") "," PFX_OUT(14, "OC3") "," PFX_OUT(15, "OC4"),
                       PFX_SW("IN1", "OUT1", ",\"enable\":\"EN1\",\"flag\":\"OC1\"") ","
                       PFX_SW("IN1", "OUT2", ",\"enable\":\"EN2\",\"flag\":\"OC2\"") ","
                       PFX_SW("IN2", "OUT3", ",\"enable\":\"EN3\",\"flag\":\"OC3\"") ","
                       PFX_SW("IN2", "OUT4", ",\"enable\":\"EN4\",\"flag\":\"OC4\""),
                       "", "PFX-QUAD", "PINHDR-1X15-V", ",\"vinMax\":6") ","
    POWERSWITCH_DEVICE("pfx-free", "load",
                       PFX_PWR(1, "IN") "," PFX_GND(2, "GND") "," PFX_OUT(3, "OUT"),
                       PFX_SW("IN", "OUT", ""), "", "PFX-FREE", "PINHDR-1X03-V", ""));

/* Loads the fixture parts (the 0.5 V supply SFX-0V5 and the RFX- regulators come from
   regulator_fixture.h, loaded first); prints the error and returns -1 on failure. */
static int load_powerswitch_fixture(PartLibrary *library) {
    char *error = NULL;

    if (load_parts(library, POWERSWITCH_FIXTURE, "powerswitch-fixture.json", &error) != 0) {
        fprintf(stderr, "powerswitch fixture: %s\n", error != NULL ? error : "(none)");
        free(error);
        return -1;
    }
    return 0;
}

#endif
