#ifndef CAPACITOR_FIXTURE_H
#define CAPACITOR_FIXTURE_H

/*
 * Fixture capacitor parts (plan-33-E), loaded next to the real part files
 * (footprints from footprints.json):
 *
 *   TESTCAP    tantalum, 10 uF, rated 6.3 V: 1 +, 2 -
 *   TESTCAP-C  ceramic, 100 nF, rated 50 V, X7R, +-10%: 1, 2
 *   TESTCAP-N  ceramic, 100 nF, no rating: 1, 2
 */

#include <stdio.h>
#include <stdlib.h>

#include "parts.h"

#define CFX_PIN(N, NAME) "{\"number\":" #N ",\"name\":\"" NAME "\",\"role\":\"input\"}"

static const char CAPACITOR_FIXTURE[] =
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"capacitor\",\"devices\":["
    "{\"id\":\"cfx-ta\",\"function\":\"10 uF\",\"description\":\"fixture\",\"drawer\":[],"
    "\"pinouts\":{\"polar\":[" CFX_PIN(1, "+") "," CFX_PIN(2, "-") "]},"
    "\"type\":\"tantalum\",\"farads\":1e-5,\"positive\":\"+\",\"negative\":\"-\","
    "\"parts\":[{\"name\":\"TESTCAP\",\"package\":\"0402\",\"pinout\":\"polar\","
    "\"footprint\":\"RES-0402\",\"volts\":6.3,\"tolerance\":\"\xc2\xb1" "10%\"}]},"
    "{\"id\":\"cfx-c\",\"function\":\"100 nF\",\"description\":\"fixture\",\"drawer\":[],"
    "\"pinouts\":{\"two-pin\":[" CFX_PIN(1, "1") "," CFX_PIN(2, "2") "]},"
    "\"type\":\"ceramic\",\"farads\":1e-7,"
    "\"parts\":[{\"name\":\"TESTCAP-C\",\"package\":\"0402\",\"pinout\":\"two-pin\","
    "\"footprint\":\"RES-0402\",\"volts\":50,\"dielectric\":\"X7R\","
    "\"tolerance\":\"\xc2\xb1" "10%\"},"
    "{\"name\":\"TESTCAP-N\",\"package\":\"0402\",\"pinout\":\"two-pin\","
    "\"footprint\":\"RES-0402\"}]}"
    "]}";

#undef CFX_PIN

/* Loads the fixture capacitors; prints the error and returns -1 on failure. */
static int load_capacitor_fixture(PartLibrary *library) {
    char *error = NULL;

    if (load_parts(library, CAPACITOR_FIXTURE, "capacitor-fixture.json", &error) != 0) {
        fprintf(stderr, "capacitor fixture: %s\n", error != NULL ? error : "(none)");
        free(error);
        return -1;
    }
    return 0;
}

#endif
