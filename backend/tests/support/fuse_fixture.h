#ifndef FUSE_FIXTURE_H
#define FUSE_FIXTURE_H

/*
 * Fixture parts for fuse_test (plan-47-I section 6), loaded next to the real
 * part files (footprints from footprints.json):
 *
 *   TESTFUSE-1A  fuse, 1 A, fast, 250 V AC, 125 V DC, 35 A breaking: 1, 2 (RES-0603)
 *   TESTPTC      PTC, hold 0.5 A, trip 1 A, 6 V, 40 A maximum: 1, 2 (RES-0603)
 *
 * FUSE_DEVICE builds one device for the rejection cases.
 */

#include <stdio.h>
#include <stdlib.h>

#include "parts.h"

#define FFX_PIN(N, NAME, ROLE) "{\"number\":" #N ",\"name\":\"" NAME "\",\"role\":\"" ROLE "\"}"
#define FFX_IO(N, NAME) FFX_PIN(N, NAME, "inout")
#define FUSE_DEVICE(ID, TYPE, TERMINALS, PINS, NAME, FOOTPRINT, PART_KEYS)                       \
    "{\"id\":\"" ID "\",\"function\":\"fixture\",\"description\":\"fixture\",\"drawer\":[],"    \
    "\"pinouts\":{\"p\":[" PINS "]},\"type\":\"" TYPE "\",\"terminals\":" TERMINALS ","         \
    "\"parts\":[{\"name\":\"" NAME "\",\"package\":\"SMD\",\"pinout\":\"p\","                   \
    "\"footprint\":\"" FOOTPRINT "\"" PART_KEYS "}]}"
#define FUSE_FILE(DEVICES)                                                                       \
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"fuse\",\"devices\":[" DEVICES "]}"

#define FFX_TWO FFX_IO(1, "1") "," FFX_IO(2, "2")

static const char FUSE_FIXTURE[] = FUSE_FILE(
    FUSE_DEVICE("ffx-fuse", "fuse", "[\"1\",\"2\"]", FFX_TWO, "TESTFUSE-1A", "RES-0603",
                ",\"amps\":1,\"speed\":\"fast\",\"voltsAc\":250,\"voltsDc\":125,\"breaking\":35") ","
    FUSE_DEVICE("ffx-ptc", "ptc", "[\"1\",\"2\"]", FFX_TWO, "TESTPTC", "RES-0603",
                ",\"hold\":0.5,\"trip\":1,\"volts\":6,\"imax\":40"));

/* Loads the fixture parts; prints the error and returns -1 on failure. */
static int load_fuse_fixture(PartLibrary *library) {
    char *error = NULL;

    if (load_parts(library, FUSE_FIXTURE, "fuse-fixture.json", &error) != 0) {
        fprintf(stderr, "fuse fixture: %s\n", error != NULL ? error : "(none)");
        free(error);
        return -1;
    }
    return 0;
}

#endif
