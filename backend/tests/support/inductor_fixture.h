#ifndef INDUCTOR_FIXTURE_H
#define INDUCTOR_FIXTURE_H

/*
 * Fixture parts for inductor_test (plan-47-I section 6), loaded next to the
 * real part files (footprints from footprints.json):
 *
 *   TESTIND-10U    10 uH, +-20%, 1.2 A, Isat 1.5 A, 0.15 ohm, shielded: 1, 2 (RES-0603)
 *   TESTIND-1N     1 nH, no other fact: 1, 2 (RES-0603)
 *   TESTIND-4PAD   2 uH, four pads, two per terminal: 1=1, 2=1, 3=2, 4=2 (OSC-3215-4)
 *
 * IND_DEVICE builds one device for the rejection cases.
 */

#include <stdio.h>
#include <stdlib.h>

#include "parts.h"

#define IFX_PIN(N, NAME, ROLE) "{\"number\":" #N ",\"name\":\"" NAME "\",\"role\":\"" ROLE "\"}"
#define IFX_IO(N, NAME) IFX_PIN(N, NAME, "inout")
#define IND_DEVICE(ID, TERMINALS, PINS, NAME, FOOTPRINT, PART_KEYS)                              \
    "{\"id\":\"" ID "\",\"function\":\"fixture\",\"description\":\"fixture\",\"drawer\":[],"    \
    "\"pinouts\":{\"p\":[" PINS "]},\"terminals\":" TERMINALS ","                               \
    "\"parts\":[{\"name\":\"" NAME "\",\"package\":\"SMD\",\"pinout\":\"p\","                   \
    "\"footprint\":\"" FOOTPRINT "\"" PART_KEYS "}]}"
#define IND_FILE(DEVICES)                                                                        \
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"inductor\",\"devices\":[" DEVICES "]}"

#define IFX_TWO IFX_IO(1, "1") "," IFX_IO(2, "2")
#define IFX_FOUR IFX_IO(1, "1") "," IFX_IO(2, "1") "," IFX_IO(3, "2") "," IFX_IO(4, "2")

static const char INDUCTOR_FIXTURE[] = IND_FILE(
    IND_DEVICE("ifx-10u", "[\"1\",\"2\"]", IFX_TWO, "TESTIND-10U", "RES-0603",
               ",\"henries\":0.00001,\"tolerance\":\"+-20%\",\"amps\":1.2,\"isat\":1.5,"
               "\"dcr\":0.15,\"shielding\":\"shielded\"") ","
    IND_DEVICE("ifx-1n", "[\"1\",\"2\"]", IFX_TWO, "TESTIND-1N", "RES-0603",
               ",\"henries\":0.000000001") ","
    IND_DEVICE("ifx-4pad", "[\"1\",\"2\"]", IFX_FOUR, "TESTIND-4PAD", "OSC-3215-4",
               ",\"henries\":0.000002"));

/* Loads the fixture parts; prints the error and returns -1 on failure. */
static int load_inductor_fixture(PartLibrary *library) {
    char *error = NULL;

    if (load_parts(library, INDUCTOR_FIXTURE, "inductor-fixture.json", &error) != 0) {
        fprintf(stderr, "inductor fixture: %s\n", error != NULL ? error : "(none)");
        free(error);
        return -1;
    }
    return 0;
}

#endif
