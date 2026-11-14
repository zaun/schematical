#ifndef MCU_FIXTURE_H
#define MCU_FIXTURE_H

/*
 * Fixture mcu part (plan-36-F), loaded next to the real part files
 * (footprint from footprints.json):
 *
 *   TESTMCU   core avr, 20 MHz device clock (the part says 10 MHz), 32 KiB
 *             flash, 2 KiB RAM, 1 KiB EEPROM, vcc 1.8-5.5 V; "simulated":
 *             false. Pinout p8: 1 VCC, 2-7 PB0-PB5 (inout), 8 GND.
 */

#include <stdio.h>
#include <stdlib.h>

#include "parts.h"

#define MFX_PIN(N, NAME, ROLE) "{\"number\":" #N ",\"name\":\"" NAME "\",\"role\":\"" ROLE "\"}"

static const char MCU_FIXTURE[] =
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"mcu\",\"devices\":["
    "{\"id\":\"mfx\",\"function\":\"8-bit MCU\",\"description\":\"fixture\",\"drawer\":[],"
    "\"simulated\":false,"
    "\"pinouts\":{\"p8\":[" MFX_PIN(1, "VCC", "power") "," MFX_PIN(2, "PB0", "inout") ","
    MFX_PIN(3, "PB1", "inout") "," MFX_PIN(4, "PB2", "inout") "," MFX_PIN(5, "PB3", "inout") ","
    MFX_PIN(6, "PB4", "inout") "," MFX_PIN(7, "PB5", "inout") "," MFX_PIN(8, "GND", "ground") "]},"
    "\"core\":\"avr\",\"clock\":20000000,\"flash\":32768,\"ram\":2048,\"eeprom\":1024,"
    "\"vcc\":[1.8,5.5],"
    "\"parts\":[{\"name\":\"TESTMCU\",\"package\":\"TSSOP\",\"pinout\":\"p8\","
    "\"footprint\":\"TSSOP-8\",\"clock\":10000000}]}"
    "]}";

#undef MFX_PIN

/* Loads the fixture mcu; prints the error and returns -1 on failure. */
static int load_mcu_fixture(PartLibrary *library) {
    char *error = NULL;

    if (load_parts(library, MCU_FIXTURE, "mcu-fixture.json", &error) != 0) {
        fprintf(stderr, "mcu fixture: %s\n", error != NULL ? error : "(none)");
        free(error);
        return -1;
    }
    return 0;
}

#endif
