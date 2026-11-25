#ifndef OPAMP_FIXTURE_H
#define OPAMP_FIXTURE_H

/*
 * Fixture parts for opamp_test (plan-48-F section 5), loaded next to the
 * real part files (footprints from footprints.json):
 *
 *   TESTOPAMP2   general, dual-8 pinout, 1 MHz, 0.5 V/us, 7 mV offset, vcc 3-32 V (PDIP-8)
 *   TESTOPAMP2A  the same with a 3 mV offset
 *   TESTOPAMP1   cmos, single-5a pinout, rail-to-rail in and out, vcc 1.8-5.5 V (SOT23-5)
 *
 * OPAMP_DEVICE builds one device for the rejection cases.
 */

#include <stdio.h>
#include <stdlib.h>

#include "parts.h"

#define OFX_PIN(N, NAME, ROLE) "{\"number\":" #N ",\"name\":\"" NAME "\",\"role\":\"" ROLE "\"}"

/* The dual-8 pinout of plan-48-B section 3.1. */
#define OFX_DUAL8                                                                                 \
    OFX_PIN(1, "OUT1", "output") "," OFX_PIN(2, "IN1-", "input") "," OFX_PIN(3, "IN1+", "input")  \
    "," OFX_PIN(4, "V-", "ground") "," OFX_PIN(5, "IN2+", "input") "," OFX_PIN(6, "IN2-", "input") \
    "," OFX_PIN(7, "OUT2", "output") "," OFX_PIN(8, "V+", "power")

/* The single-5a pinout. */
#define OFX_SINGLE5A                                                                              \
    OFX_PIN(1, "OUT", "output") "," OFX_PIN(2, "V-", "ground") "," OFX_PIN(3, "IN+", "input")     \
    "," OFX_PIN(4, "IN-", "input") "," OFX_PIN(5, "V+", "power")

#define OFX_AMP1 "{\"plus\":\"IN1+\",\"minus\":\"IN1-\",\"output\":\"OUT1\"}"
#define OFX_AMP2 "{\"plus\":\"IN2+\",\"minus\":\"IN2-\",\"output\":\"OUT2\"}"
#define OFX_AMP "{\"plus\":\"IN+\",\"minus\":\"IN-\",\"output\":\"OUT\"}"

/* TYPE, AMPS (the array text), PINS, KEYS (more device keys with a leading comma), PARTS (the array text). */
#define OPAMP_DEVICE(ID, TYPE, AMPS, PINS, KEYS, PARTS)                                           \
    "{\"id\":\"" ID "\",\"function\":\"fixture\",\"description\":\"fixture\",\"drawer\":[],"    \
    "\"pinouts\":{\"p\":[" PINS "]},\"type\":\"" TYPE "\",\"amplifiers\":[" AMPS "]" KEYS ","     \
    "\"parts\":[" PARTS "]}"
#define OPAMP_FILE(DEVICES)                                                                       \
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"opamp\",\"devices\":[" DEVICES "]}"
#define OFX_PART(NAME, FOOTPRINT, KEYS)                                                           \
    "{\"name\":\"" NAME "\",\"package\":\"DIP\",\"pinout\":\"p\",\"footprint\":\"" FOOTPRINT "\"" KEYS "}"

static const char OPAMP_FIXTURE[] = OPAMP_FILE(
    OPAMP_DEVICE("ofx-dual", "general", OFX_AMP1 "," OFX_AMP2, OFX_DUAL8,
                 ",\"gbwHz\":1000000,\"slewVus\":0.5,\"vosMv\":7,\"vcc\":[3,32]",
                 OFX_PART("TESTOPAMP2", "PDIP-8", "") "," OFX_PART("TESTOPAMP2A", "PDIP-8", ",\"vosMv\":3")) ","
    OPAMP_DEVICE("ofx-single", "cmos", OFX_AMP, OFX_SINGLE5A,
                 ",\"railIn\":true,\"railOut\":true,\"vcc\":[1.8,5.5]", OFX_PART("TESTOPAMP1", "SOT23-5", "")));

/* Loads the fixture parts; prints the error and returns -1 on failure. */
static int load_opamp_fixture(PartLibrary *library) {
    char *error = NULL;

    if (load_parts(library, OPAMP_FIXTURE, "opamp-fixture.json", &error) != 0) {
        fprintf(stderr, "opamp fixture: %s\n", error != NULL ? error : "(none)");
        free(error);
        return -1;
    }
    return 0;
}

#endif
