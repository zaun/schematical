#ifndef TRANSISTOR_FIXTURE_H
#define TRANSISTOR_FIXTURE_H

/*
 * Fixture parts for transistor_test (plan-40-F section 6), loaded next to
 * the real part files (footprints from footprints.json):
 *
 *   TFX-Q1   npn, pins 1 E, 2 B, 3 C: vceo 40, ic 0.2, pd 0.625, hfe 100
 *   TFX-Q2   pnp, pins 1 E, 2 B, 3 C: vceo 5 (a base on 12 V is still never refused)
 *   TFX-M1   nmos, pins 1 G, 2 D, 3 S: vds 60, id 0.3, rdson 5, vgsth 2.1, vgs 20, logicLevel false
 *   TFX-M2   pmos, pins 1 G, 2 D, 3 S: vds 20, logicLevel true
 *   TFX-M5   nmos, pins 1 G, 2 S, 3 D: vds 5, vgs 8 (the rating cases)
 *   TFX-DT   nmos with a tab, pins 1 G, 2 D, 3 S, 4 D: the tab and the drain lead tied
 *   TFX-PB   pre-biased npn, pins 1 E, 2 B, 3 C: r1 10000, r2 47000
 *   TFX-ARR  complementary pair, 6 pins: 1 E1, 2 B1, 3 C2, 4 E2, 5 B2, 6 C1 (npn E1 B1 C1, pnp E2 B2 C2)
 *   TFX-GEN  generic npn, no ratings (never refused)
 *
 * TRANSISTOR_DEVICE builds one transistor device for the rejection cases.
 */

#include <stdio.h>
#include <stdlib.h>

#include "parts.h"

#define TFX_PIN(N, NAME, ROLE) "{\"number\":" #N ",\"name\":\"" NAME "\",\"role\":\"" ROLE "\"}"
#define TFX_IO(N, NAME) TFX_PIN(N, NAME, "inout")
#define TFX_IN(N, NAME) TFX_PIN(N, NAME, "input")
#define TFX_BJT(TYPE, B, C, E) \
    "{\"type\":\"" TYPE "\",\"base\":\"" B "\",\"collector\":\"" C "\",\"emitter\":\"" E "\"}"
#define TFX_FET(TYPE, G, D, S) \
    "{\"type\":\"" TYPE "\",\"gate\":\"" G "\",\"drain\":\"" D "\",\"source\":\"" S "\"}"
#define TRANSISTOR_DEVICE(ID, PINS, ELEMENTS, NAME, FOOTPRINT, PART_KEYS)                         \
    "{\"id\":\"" ID "\",\"function\":\"transistor\",\"description\":\"fixture\","                 \
    "\"drawer\":[],\"pinouts\":{\"p\":[" PINS "]},\"elements\":[" ELEMENTS "],"                   \
    "\"parts\":[{\"name\":\"" NAME "\",\"package\":\"TO-92\",\"pinout\":\"p\","                   \
    "\"footprint\":\"" FOOTPRINT "\"" PART_KEYS "}]}"
#define TRANSISTOR_FILE(DEVICES)                                                                  \
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"transistor\",\"devices\":[" DEVICES "]}"

#define TFX_EBC TFX_IO(1, "E") "," TFX_IN(2, "B") "," TFX_IO(3, "C")
#define TFX_GDS TFX_IN(1, "G") "," TFX_IO(2, "D") "," TFX_IO(3, "S")

static const char TRANSISTOR_FIXTURE[] = TRANSISTOR_FILE(
    TRANSISTOR_DEVICE("tfx-q1", TFX_EBC, TFX_BJT("npn", "B", "C", "E"), "TFX-Q1", "PINHDR-1X03-V",
                      ",\"vceo\":40,\"ic\":0.2,\"pd\":0.625,\"hfe\":100") ","
    TRANSISTOR_DEVICE("tfx-q2", TFX_EBC, TFX_BJT("pnp", "B", "C", "E"), "TFX-Q2", "PINHDR-1X03-V",
                      ",\"vceo\":5,\"ic\":0.2") ","
    TRANSISTOR_DEVICE("tfx-m1", TFX_GDS, TFX_FET("nmos", "G", "D", "S"), "TFX-M1", "PINHDR-1X03-V",
                      ",\"vds\":60,\"id\":0.3,\"rdson\":5,\"vgsth\":2.1,\"vgs\":20,\"logicLevel\":false") ","
    TRANSISTOR_DEVICE("tfx-m2", TFX_GDS, TFX_FET("pmos", "G", "D", "S"), "TFX-M2", "PINHDR-1X03-V",
                      ",\"vds\":20,\"logicLevel\":true") ","
    TRANSISTOR_DEVICE("tfx-m5", TFX_IN(1, "G") "," TFX_IO(2, "S") "," TFX_IO(3, "D"),
                      TFX_FET("nmos", "G", "D", "S"), "TFX-M5", "PINHDR-1X03-V", ",\"vds\":5,\"vgs\":8") ","
    TRANSISTOR_DEVICE("tfx-dt", TFX_GDS "," TFX_IO(4, "D"), TFX_FET("nmos", "G", "D", "S"), "TFX-DT",
                      "PINHDR-1X04-V", ",\"vds\":30") ","
    TRANSISTOR_DEVICE("tfx-pb", TFX_EBC, TFX_BJT("npn", "B", "C", "E"), "TFX-PB", "PINHDR-1X03-V",
                      ",\"vceo\":50,\"r1\":10000,\"r2\":47000") ","
    TRANSISTOR_DEVICE("tfx-arr",
                      TFX_IO(1, "E1") "," TFX_IN(2, "B1") "," TFX_IO(3, "C2") "," TFX_IO(4, "E2") ","
                      TFX_IN(5, "B2") "," TFX_IO(6, "C1"),
                      TFX_BJT("npn", "B1", "C1", "E1") "," TFX_BJT("pnp", "B2", "C2", "E2"), "TFX-ARR",
                      "PINHDR-1X06-V", ",\"vceo\":40") ","
    TRANSISTOR_DEVICE("tfx-gen", TFX_EBC, TFX_BJT("npn", "B", "C", "E"), "TFX-GEN", "PINHDR-1X03-V", ""));

/* Loads the fixture parts; prints the error and returns -1 on failure. */
static int load_transistor_fixture(PartLibrary *library) {
    char *error = NULL;

    if (load_parts(library, TRANSISTOR_FIXTURE, "transistor-fixture.json", &error) != 0) {
        fprintf(stderr, "transistor fixture: %s\n", error != NULL ? error : "(none)");
        free(error);
        return -1;
    }
    return 0;
}

#endif
