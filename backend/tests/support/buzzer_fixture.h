#ifndef BUZZER_FIXTURE_H
#define BUZZER_FIXTURE_H

/*
 * Fixture parts for buzzer_test (plan-47-J section 6), loaded next to the
 * real part files (footprints from footprints.json):
 *
 *   TESTBUZ-IND  indicator, magnetic, 2300 Hz, 5 V (3-7 V), 30 mA, 85 dB: 1 +, 2 - (PINHDR-1X02-V)
 *   TESTBUZ-PZ   transducer, piezo, 4000 Hz: 1 +, 2 -, 3 F (nc) (PINHDR-1X03-V)
 *
 * BUZZER_DEVICE builds one device for the rejection cases.
 */

#include <stdio.h>
#include <stdlib.h>

#include "parts.h"

#define BFX_PIN(N, NAME, ROLE) "{\"number\":" #N ",\"name\":\"" NAME "\",\"role\":\"" ROLE "\"}"
#define BFX_IN(N, NAME) BFX_PIN(N, NAME, "input")
#define BUZZER_DEVICE(ID, DRIVER, TECH, PINS, NAME, FOOTPRINT, PART_KEYS)                        \
    "{\"id\":\"" ID "\",\"function\":\"fixture\",\"description\":\"fixture\",\"drawer\":[],"    \
    "\"pinouts\":{\"p\":[" PINS "]},\"driver\":\"" DRIVER "\",\"technology\":\"" TECH "\","     \
    "\"positive\":\"+\",\"negative\":\"-\","                                                    \
    "\"parts\":[{\"name\":\"" NAME "\",\"package\":\"TH\",\"pinout\":\"p\","                    \
    "\"footprint\":\"" FOOTPRINT "\"" PART_KEYS "}]}"
#define BUZZER_FILE(DEVICES)                                                                     \
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"buzzer\",\"devices\":[" DEVICES "]}"

#define BFX_TWO BFX_IN(1, "+") "," BFX_IN(2, "-")
#define BFX_THREE BFX_TWO "," BFX_PIN(3, "F", "nc")

static const char BUZZER_FIXTURE[] = BUZZER_FILE(
    BUZZER_DEVICE("bfx-ind", "indicator", "magnetic", BFX_TWO, "TESTBUZ-IND", "PINHDR-1X02-V",
                  ",\"hz\":2300,\"volts\":5,\"voltsMin\":3,\"voltsMax\":7,\"ma\":30,\"db\":85") ","
    BUZZER_DEVICE("bfx-pz", "transducer", "piezo", BFX_THREE, "TESTBUZ-PZ", "PINHDR-1X03-V",
                  ",\"hz\":4000"));

/* Loads the fixture parts; prints the error and returns -1 on failure. */
static int load_buzzer_fixture(PartLibrary *library) {
    char *error = NULL;

    if (load_parts(library, BUZZER_FIXTURE, "buzzer-fixture.json", &error) != 0) {
        fprintf(stderr, "buzzer fixture: %s\n", error != NULL ? error : "(none)");
        free(error);
        return -1;
    }
    return 0;
}

#endif
