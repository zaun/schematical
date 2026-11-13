#ifndef RESISTOR_FIXTURE_H
#define RESISTOR_FIXTURE_H

/*
 * Fixture parts for resistor_test (plan-34-E section 6), loaded next to the
 * real part files (footprints from footprints.json):
 *
 *   RFX-R1     two-pin resistor 1, 2: 4.7 kOhm, 1 %, 0.1 W
 *   RFX-RA4    4-element isolated array, 8 pins: R1 1-8, R2 2-7, R3 3-6, R4 4-5
 *   RFX-RB3    bussed array, 5 pins: C (pins 1 and 5, one node) to 2, 3, 4
 *   RFX-LV1G04 inverter, vcc 3-3.6 V, inputMax 3.6 V: 1 A, 2 GND, 3 Y, 4 VCC
 *
 * RESISTOR_DEVICE builds one resistor device for the rejection cases.
 */

#include <stdio.h>
#include <stdlib.h>

#include "parts.h"

#define RFX_PIN(N, NAME, ROLE) "{\"number\":" #N ",\"name\":\"" NAME "\",\"role\":\"" ROLE "\"}"
#define RFX_IO(N, NAME) RFX_PIN(N, NAME, "inout")
#define RFX_PAIR(A, B) "{\"a\":\"" A "\",\"b\":\"" B "\"}"
#define RESISTOR_DEVICE(ID, PINS, RESISTORS, NAME, FOOTPRINT, PART_KEYS)                         \
    "{\"id\":\"" ID "\",\"function\":\"1 k\\u03a9 \\u2013 5 k\\u03a9\",\"description\":\"fixture\"," \
    "\"drawer\":[],\"pinouts\":{\"p\":[" PINS "]}," RESISTORS                                     \
    "\"parts\":[{\"name\":\"" NAME "\",\"package\":\"0603\",\"pinout\":\"p\","                    \
    "\"footprint\":\"" FOOTPRINT "\"" PART_KEYS "}]}"
#define RESISTOR_FILE(DEVICES)                                                                    \
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"resistor\",\"devices\":[" DEVICES "]}"

static const char RESISTOR_FIXTURE[] = RESISTOR_FILE(
    RESISTOR_DEVICE("rfx-r1", RFX_IO(1, "1") "," RFX_IO(2, "2"),
                    "\"resistors\":[" RFX_PAIR("1", "2") "],", "RFX-R1", "RES-0603",
                    ",\"ohms\":4700,\"tolerance\":1,\"power\":0.1") ","
    RESISTOR_DEVICE("rfx-ra4",
                    RFX_IO(1, "1") "," RFX_IO(2, "2") "," RFX_IO(3, "3") "," RFX_IO(4, "4") ","
                    RFX_IO(5, "5") "," RFX_IO(6, "6") "," RFX_IO(7, "7") "," RFX_IO(8, "8"),
                    "\"resistors\":[" RFX_PAIR("1", "8") "," RFX_PAIR("2", "7") ","
                    RFX_PAIR("3", "6") "," RFX_PAIR("4", "5") "],",
                    "RFX-RA4", "SOIC-8", ",\"ohms\":10000,\"tolerance\":5,\"power\":0.0625") ","
    RESISTOR_DEVICE("rfx-rb3",
                    RFX_IO(1, "C") "," RFX_IO(2, "2") "," RFX_IO(3, "3") "," RFX_IO(4, "4") ","
                    RFX_IO(5, "C"),
                    "\"resistors\":[" RFX_PAIR("C", "2") "," RFX_PAIR("C", "3") ","
                    RFX_PAIR("C", "4") "],",
                    "RFX-RB3", "PINHDR-1X05-V", ",\"ohms\":0"));

static const char RESISTOR_FIXTURE_GATE[] =
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"gate\",\"devices\":["
    "{\"id\":\"rfx-lv\",\"function\":\"Inverter\",\"description\":\"fixture\",\"drawer\":[],"
    "\"pinouts\":{\"p4\":[" RFX_PIN(1, "A", "input") "," RFX_PIN(2, "GND", "ground") ","
    RFX_PIN(3, "Y", "output") "," RFX_PIN(4, "VCC", "power") "]},"
    "\"gates\":[{\"op\":\"not\",\"inputs\":[\"A\"],\"output\":\"Y\"}],"
    "\"timing\":{\"LVC\":{\"rise\":4,\"fall\":4,\"vcc\":[3,3.6],\"inputMax\":3.6}},"
    "\"parts\":[{\"name\":\"RFX-LV1G04\",\"package\":\"SOT-23\",\"pinout\":\"p4\","
    "\"footprint\":\"PINHDR-1X04-V\",\"family\":\"LVC\"}]}"
    "]}";

/* Loads the fixture parts; prints the error and returns -1 on failure. */
static int load_resistor_fixture(PartLibrary *library) {
    const char *files[] = {RESISTOR_FIXTURE, RESISTOR_FIXTURE_GATE};
    size_t i;

    for (i = 0; i < 2; i++) {
        char *error = NULL;

        if (load_parts(library, files[i], "resistor-fixture.json", &error) != 0) {
            fprintf(stderr, "resistor fixture: %s\n", error != NULL ? error : "(none)");
            free(error);
            return -1;
        }
    }
    return 0;
}

#endif
