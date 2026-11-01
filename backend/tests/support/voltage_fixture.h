#ifndef VOLTAGE_FIXTURE_H
#define VOLTAGE_FIXTURE_H

/*
 * Fixture parts for the voltage check (plan-07-B), loaded next to the real
 * part files (their footprints come from footprints.json):
 *
 *   VFX-HC1G00   NAND, vcc 2-6 V
 *   VFX-AUC1G00  NAND, vcc 0.8-2.7 V, inputMax 3.6 V
 *   VFX-OD1G00   open-drain NAND, vcc 2-6 V
 *   VFX-SW2      2-position switch rated 5 V
 *
 * Every gate is SOT23-5: 1 A, 2 B, 3 GND, 4 Y, 5 VCC. The switch is
 * 1 1A, 2 2A, 3 2B, 4 1B (positions 1A-1B and 2A-2B).
 */

#include <stdio.h>
#include <stdlib.h>

#include "parts.h"

#define VFX_GATE(ID, NAME, OUTPUT, VOLTAGE)                                                      \
    "{\"id\":\"" ID "\",\"function\":\"NAND\",\"description\":\"fixture\",\"drawer\":[],"         \
    "\"pinouts\":{\"p5\":[{\"number\":1,\"name\":\"A\",\"role\":\"input\"},"                       \
    "{\"number\":2,\"name\":\"B\",\"role\":\"input\"},{\"number\":3,\"name\":\"GND\",\"role\":\"ground\"}," \
    "{\"number\":4,\"name\":\"Y\",\"role\":\"output\"},{\"number\":5,\"name\":\"VCC\",\"role\":\"power\"}]}," \
    "\"gates\":[{\"op\":\"nand\",\"inputs\":[\"A\",\"B\"],\"output\":\"Y\"}]," OUTPUT              \
    "\"timing\":{\"HC\":{\"rise\":5,\"fall\":5," VOLTAGE "}},"                                     \
    "\"parts\":[{\"name\":\"" NAME "\",\"package\":\"SOT-23\",\"pinout\":\"p5\",\"footprint\":\"SOT23-5\",\"family\":\"HC\"}]}"

static const char VOLTAGE_FIXTURE_GATES[] =
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"gate\",\"devices\":["
    VFX_GATE("vfx-hc", "VFX-HC1G00", "", "\"vcc\":[2,6]") ","
    VFX_GATE("vfx-auc", "VFX-AUC1G00", "", "\"vcc\":[0.8,2.7],\"inputMax\":3.6") ","
    VFX_GATE("vfx-od", "VFX-OD1G00", "\"output\":\"open-drain\",", "\"vcc\":[2,6]")
    "]}";

static const char VOLTAGE_FIXTURE_SWITCHES[] =
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"switch\",\"devices\":["
    "{\"id\":\"vfx-sw\",\"function\":\"GDS\",\"description\":\"fixture\",\"drawer\":[],"
    "\"pinouts\":{\"p4\":[{\"number\":1,\"name\":\"1A\",\"role\":\"inout\"},"
    "{\"number\":2,\"name\":\"2A\",\"role\":\"inout\"},{\"number\":3,\"name\":\"2B\",\"role\":\"inout\"},"
    "{\"number\":4,\"name\":\"1B\",\"role\":\"inout\"}]},"
    "\"actuator\":\"Low profile\",\"contacts\":[[\"1A\",\"1B\"],[\"2A\",\"2B\"]],\"ratedVolts\":5,"
    "\"parts\":[{\"name\":\"VFX-SW2\",\"package\":\"DIP\",\"pinout\":\"p4\",\"footprint\":\"SW-DIP-4\"}]}"
    "]}";

/* Loads the fixture parts into a library that already has footprints.json.
   0, or -1 after printing the error. */
static int load_voltage_fixture(PartLibrary *library) {
    char *error = NULL;

    if (load_parts(library, VOLTAGE_FIXTURE_GATES, "voltage-fixture-gates.json", &error) != 0 ||
        load_parts(library, VOLTAGE_FIXTURE_SWITCHES, "voltage-fixture-switches.json", &error) != 0) {
        fprintf(stderr, "voltage fixture: %s\n", error != NULL ? error : "(no message)");
        free(error);
        return -1;
    }
    return 0;
}

#endif
