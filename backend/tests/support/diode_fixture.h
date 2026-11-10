#ifndef DIODE_FIXTURE_H
#define DIODE_FIXTURE_H

/*
 * Fixture diode parts (plan-29-H section 6), loaded next to the real part
 * files (footprints from footprints.json):
 *
 *   DFX-D1   single diode: 1 K, 2 A
 *   DFX-D2   common-cathode pair: 1 A1, 2 A2, 3 K
 *   DFX-D3   single diode with a cathode tab: 1 K, 2 A, 3 K (tab)
 */

#include <stdio.h>
#include <stdlib.h>

#include "parts.h"

#define DFX_PIN(N, NAME) "{\"number\":" #N ",\"name\":\"" NAME "\",\"role\":\"inout\"}"
#define DFX_DEVICE(ID, PINS, DIODES, NAME, FOOTPRINT)                                         \
    "{\"id\":\"" ID "\",\"function\":\"Diode\",\"description\":\"fixture\",\"drawer\":[],"   \
    "\"pinouts\":{\"p\":[" PINS "]},\"type\":\"rectifier\",\"diodes\":[" DIODES "],"          \
    "\"parts\":[{\"name\":\"" NAME "\",\"package\":\"SMA\",\"pinout\":\"p\","                 \
    "\"footprint\":\"" FOOTPRINT "\",\"vr\":100,\"io\":1}]}"

static const char DIODE_FIXTURE[] =
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"diode\",\"devices\":["
    DFX_DEVICE("dfx-d1", DFX_PIN(1, "K") "," DFX_PIN(2, "A"),
               "{\"anode\":\"A\",\"cathode\":\"K\"}", "DFX-D1", "RES-0402") ","
    DFX_DEVICE("dfx-d2", DFX_PIN(1, "A1") "," DFX_PIN(2, "A2") "," DFX_PIN(3, "K"),
               "{\"anode\":\"A1\",\"cathode\":\"K\"},{\"anode\":\"A2\",\"cathode\":\"K\"}",
               "DFX-D2", "PINHDR-1X03-V") ","
    DFX_DEVICE("dfx-d3", DFX_PIN(1, "K") "," DFX_PIN(2, "A") "," DFX_PIN(3, "K"),
               "{\"anode\":\"A\",\"cathode\":\"K\"}", "DFX-D3", "PINHDR-1X03-V")
    "]}";

#undef DFX_PIN
#undef DFX_DEVICE

/* Loads the fixture diodes; prints the error and returns -1 on failure. */
static int load_diode_fixture(PartLibrary *library) {
    char *error = NULL;

    if (load_parts(library, DIODE_FIXTURE, "diode-fixture.json", &error) != 0) {
        fprintf(stderr, "diode fixture: %s\n", error != NULL ? error : "(none)");
        free(error);
        return -1;
    }
    return 0;
}

#endif
