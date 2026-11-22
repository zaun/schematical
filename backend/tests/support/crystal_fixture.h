#ifndef CRYSTAL_FIXTURE_H
#define CRYSTAL_FIXTURE_H

/*
 * Fixture parts for crystal_test (plan-44-E section 6), loaded next to the
 * real part files (footprints from footprints.json):
 *
 *   TESTXTAL-1M    crystal, 1 MHz, 18 pF, 20 ppm, 30 ppm, 80 ohm: 1 X1, 2 X2 (RES-0402)
 *   TESTXTAL-32K   crystal, 32768 Hz, four pads: 1 X1, 2 NC, 3 X2, 4 NC (OSC-3215-4)
 *   TESTXTAL-FAST  crystal, 100 MHz: 1 X1, 2 X2 (RES-0402)
 *   TESTRES-8M     ceramic resonator, 8 MHz, built-in capacitors: 1 X1, 2 GND, 3 X2 (SOT23-3)
 *
 * CRYSTAL_DEVICE builds one device for the rejection cases.
 */

#include <stdio.h>
#include <stdlib.h>

#include "parts.h"

#define XFX_PIN(N, NAME, ROLE) "{\"number\":" #N ",\"name\":\"" NAME "\",\"role\":\"" ROLE "\"}"
#define XFX_IO(N, NAME) XFX_PIN(N, NAME, "inout")
#define XFX_NC(N, NAME) XFX_PIN(N, NAME, "nc")
#define CRYSTAL_DEVICE(ID, TYPE, TERMINALS, PINS, NAME, FOOTPRINT, PART_KEYS)                    \
    "{\"id\":\"" ID "\",\"function\":\"fixture\",\"description\":\"fixture\",\"drawer\":[],"    \
    "\"pinouts\":{\"p\":[" PINS "]},\"type\":\"" TYPE "\",\"terminals\":" TERMINALS ","         \
    "\"parts\":[{\"name\":\"" NAME "\",\"package\":\"SMD\",\"pinout\":\"p\","                   \
    "\"footprint\":\"" FOOTPRINT "\"" PART_KEYS "}]}"
#define CRYSTAL_FILE(DEVICES)                                                                    \
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"crystal\",\"devices\":[" DEVICES "]}"

#define XFX_TWO XFX_IO(1, "X1") "," XFX_IO(2, "X2")
#define XFX_FOUR XFX_IO(1, "X1") "," XFX_NC(2, "NC") "," XFX_IO(3, "X2") "," XFX_NC(4, "NC")
#define XFX_THREE XFX_IO(1, "X1") "," XFX_NC(2, "GND") "," XFX_IO(3, "X2")

static const char CRYSTAL_FIXTURE[] = CRYSTAL_FILE(
    CRYSTAL_DEVICE("xfx-1m", "crystal", "[\"X1\",\"X2\"]", XFX_TWO, "TESTXTAL-1M", "RES-0402",
                   ",\"hz\":1000000,\"loadPf\":18,\"tolerancePpm\":20,\"stabilityPpm\":30,"
                   "\"esrOhms\":80") ","
    CRYSTAL_DEVICE("xfx-32k", "crystal", "[\"X1\",\"X2\"]", XFX_FOUR, "TESTXTAL-32K", "OSC-3215-4",
                   ",\"hz\":32768") ","
    CRYSTAL_DEVICE("xfx-fast", "crystal", "[\"X1\",\"X2\"]", XFX_TWO, "TESTXTAL-FAST", "RES-0402",
                   ",\"hz\":100000000") ","
    CRYSTAL_DEVICE("xfx-8m", "resonator", "[\"X1\",\"X2\"]", XFX_THREE, "TESTRES-8M", "SOT23-3",
                   ",\"hz\":8000000,\"capacitors\":true"));

/* Loads the fixture parts; prints the error and returns -1 on failure. */
static int load_crystal_fixture(PartLibrary *library) {
    char *error = NULL;

    if (load_parts(library, CRYSTAL_FIXTURE, "crystal-fixture.json", &error) != 0) {
        fprintf(stderr, "crystal fixture: %s\n", error != NULL ? error : "(none)");
        free(error);
        return -1;
    }
    return 0;
}

#endif
