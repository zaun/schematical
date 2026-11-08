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
 *   VFX-OD74     open-drain D flip-flop, vcc 2-6 V
 *   VFX-PP74     push-pull D flip-flop, vcc 2-6 V
 *   VFX-T45      dual-supply translator (buffer, plan-20-F):
 *                A side {A, DIR} from VCCA 1.65-3.6 V, B side {B} from VCCB 1.65-5.5 V
 *
 * Every gate is SOT23-5: 1 A, 2 B, 3 GND, 4 Y, 5 VCC. The switch is
 * 1 1A, 2 2A, 3 2B, 4 1B (positions 1A-1B and 2A-2B). Each flip-flop is
 * SOT23-5: 1 D, 2 CLK, 3 GND, 4 Q, 5 VCC. The translator is SOT23-6:
 * 1 VCCA, 2 GND, 3 A, 4 B, 5 DIR, 6 VCCB.
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

#define VFX_DFF(ID, NAME, OUTPUT)                                                                \
    "{\"id\":\"" ID "\",\"function\":\"D flip-flop\",\"description\":\"fixture\",\"drawer\":[]," \
    "\"pinouts\":{\"p5\":[{\"number\":1,\"name\":\"D\",\"role\":\"input\"},"                     \
    "{\"number\":2,\"name\":\"CLK\",\"role\":\"input\"},{\"number\":3,\"name\":\"GND\",\"role\":\"ground\"}," \
    "{\"number\":4,\"name\":\"Q\",\"role\":\"output\"},{\"number\":5,\"name\":\"VCC\",\"role\":\"power\"}]}," \
    "\"units\":[{\"d\":\"D\",\"clk\":\"CLK\",\"q\":\"Q\"}],\"presetActiveHigh\":false,"            \
    "\"clearActiveHigh\":false,\"clockEdge\":\"rising\",\"output\":\"" OUTPUT "\","                 \
    "\"timing\":{\"HC\":{\"rise\":5,\"fall\":5,\"clearWins\":false,\"vcc\":[2,6]}},"                 \
    "\"parts\":[{\"name\":\"" NAME "\",\"package\":\"SOT-23\",\"pinout\":\"p5\",\"footprint\":\"SOT23-5\",\"family\":\"HC\"}]}"

static const char VOLTAGE_FIXTURE_DFFS[] =
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"dff\",\"devices\":["
    VFX_DFF("vfx-od74", "VFX-OD74", "open-drain") ","
    VFX_DFF("vfx-pp74", "VFX-PP74", "push-pull")
    "]}";

static const char VOLTAGE_FIXTURE_TRANSLATORS[] =
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"buffer\",\"devices\":["
    "{\"id\":\"vfx-t45\",\"function\":\"Level shifter\",\"description\":\"fixture\",\"drawer\":[],"
    "\"pinouts\":{\"p6\":[{\"number\":1,\"name\":\"VCCA\",\"role\":\"power\"},"
    "{\"number\":2,\"name\":\"GND\",\"role\":\"ground\"},{\"number\":3,\"name\":\"A\",\"role\":\"inout\"},"
    "{\"number\":4,\"name\":\"B\",\"role\":\"inout\"},{\"number\":5,\"name\":\"DIR\",\"role\":\"input\"},"
    "{\"number\":6,\"name\":\"VCCB\",\"role\":\"power\"}]},\"model\":\"translator\","
    "\"units\":[{\"a\":[\"A\"],\"b\":[\"B\"],\"dir\":{\"pin\":\"DIR\",\"ab\":\"high\"}}],"
    "\"domains\":[{\"power\":\"VCCA\",\"pins\":[\"A\",\"DIR\"]},{\"power\":\"VCCB\",\"pins\":[\"B\"]}],"
    "\"timing\":{\"LVC\":{\"rise\":4,\"fall\":4,\"vcc\":[1.65,3.6],\"vccB\":[1.65,5.5]}},"
    "\"parts\":[{\"name\":\"VFX-T45\",\"package\":\"SOT-23\",\"pinout\":\"p6\",\"footprint\":\"SOT23-6\",\"family\":\"LVC\"}]}"
    "]}";

/* plan-24-D: a converter whose AVDD and AIN are analog pins (the voltage check
   skips them); DVDD is its digital supply. */
static const char VOLTAGE_FIXTURE_CONVERTERS[] =
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"converter\",\"devices\":["
    "{\"id\":\"vfx-adc\",\"function\":\"ADC\",\"description\":\"fixture\",\"drawer\":[],"
    "\"pinouts\":{\"p6\":[{\"number\":1,\"name\":\"DVDD\",\"role\":\"power\"},"
    "{\"number\":2,\"name\":\"AVDD\",\"role\":\"power\"},{\"number\":3,\"name\":\"GND\",\"role\":\"ground\"},"
    "{\"number\":4,\"name\":\"AIN\",\"role\":\"input\"},{\"number\":5,\"name\":\"SCLK\",\"role\":\"input\"},"
    "{\"number\":6,\"name\":\"SDO\",\"role\":\"output\"}]},"
    "\"type\":\"adc\",\"bits\":12,\"channels\":1,\"codeFormat\":\"straight\",\"vcc\":[2.7,3.6],"
    "\"analog\":[\"AVDD\",\"AIN\"],\"bus\":{\"style\":\"spi\",\"sclk\":\"SCLK\",\"miso\":\"SDO\"},"
    "\"protocol\":{\"frame\":\"fixed\",\"out\":\"R{12}\"},\"conversion\":{\"time\":1000,\"start\":[\"power-on\"]},"
    "\"parts\":[{\"name\":\"VFX-ADC\",\"package\":\"SOT-23\",\"pinout\":\"p6\",\"footprint\":\"SOT23-6\"}]}"
    "]}";

/* Loads the fixture parts into a library that already has footprints.json.
   0, or -1 after printing the error. */
static int load_voltage_fixture(PartLibrary *library) {
    char *error = NULL;

    if (load_parts(library, VOLTAGE_FIXTURE_GATES, "voltage-fixture-gates.json", &error) != 0 ||
        load_parts(library, VOLTAGE_FIXTURE_SWITCHES, "voltage-fixture-switches.json", &error) != 0 ||
        load_parts(library, VOLTAGE_FIXTURE_DFFS, "voltage-fixture-dffs.json", &error) != 0 ||
        load_parts(library, VOLTAGE_FIXTURE_TRANSLATORS, "voltage-fixture-translators.json",
                   &error) != 0 ||
        load_parts(library, VOLTAGE_FIXTURE_CONVERTERS, "voltage-fixture-converters.json",
                   &error) != 0) {
        fprintf(stderr, "voltage fixture: %s\n", error != NULL ? error : "(no message)");
        free(error);
        return -1;
    }
    return 0;
}

#endif
