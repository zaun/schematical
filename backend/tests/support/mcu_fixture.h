#ifndef MCU_FIXTURE_H
#define MCU_FIXTURE_H

/*
 * Fixture mcu part (plan-36-F), loaded next to the real part files
 * (footprint from footprints.json):
 *
 *   TESTMCU   core avr, 20 MHz device clock (the part says 10 MHz), 32 KiB
 *             flash, 2 KiB RAM, 1 KiB EEPROM, vcc 1.8-5.5 V; "simulated":
 *             false. Pinout p8: 1 VCC, 2-7 PB0-PB5 (inout), 8 GND.
 *
 * and, simulated (plan-62-E), with the p28 pinout of mcu.json's atmega328p
 * and the footprint of ATMEGA328P-PU:
 *
 *   TESTAVR     model atmega328, 32 KiB flash, 2 KiB RAM, 1 KiB EEPROM
 *   TESTAVR48   model atmega328, 4 KiB flash (one-word vectors), 512 bytes
 *               of RAM, 256 of EEPROM
 */

#include <stdio.h>
#include <stdlib.h>

#include "parts.h"

#define MFX_PIN(N, NAME, ROLE) "{\"number\":" #N ",\"name\":\"" NAME "\",\"role\":\"" ROLE "\"}"

/* mcu.json's atmega328p pinout p28 (PDIP-28). */
#define MFX_P28 \
    MFX_PIN(1, "PC6", "inout") "," MFX_PIN(2, "PD0", "inout") "," MFX_PIN(3, "PD1", "inout") "," \
    MFX_PIN(4, "PD2", "inout") "," MFX_PIN(5, "PD3", "inout") "," MFX_PIN(6, "PD4", "inout") "," \
    MFX_PIN(7, "VCC", "power") "," MFX_PIN(8, "GND", "ground") "," MFX_PIN(9, "PB6", "inout") "," \
    MFX_PIN(10, "PB7", "inout") "," MFX_PIN(11, "PD5", "inout") "," MFX_PIN(12, "PD6", "inout") "," \
    MFX_PIN(13, "PD7", "inout") "," MFX_PIN(14, "PB0", "inout") "," MFX_PIN(15, "PB1", "inout") "," \
    MFX_PIN(16, "PB2", "inout") "," MFX_PIN(17, "PB3", "inout") "," MFX_PIN(18, "PB4", "inout") "," \
    MFX_PIN(19, "PB5", "inout") "," MFX_PIN(20, "AVCC", "power") "," MFX_PIN(21, "AREF", "input") "," \
    MFX_PIN(22, "GND", "ground") "," MFX_PIN(23, "PC0", "inout") "," MFX_PIN(24, "PC1", "inout") "," \
    MFX_PIN(25, "PC2", "inout") "," MFX_PIN(26, "PC3", "inout") "," MFX_PIN(27, "PC4", "inout") "," \
    MFX_PIN(28, "PC5", "inout")

static const char MCU_FIXTURE[] =
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"mcu\",\"devices\":["
    "{\"id\":\"mfx\",\"function\":\"8-bit MCU\",\"description\":\"fixture\",\"drawer\":[],"
    "\"simulated\":false,"
    "\"pinouts\":{\"p8\":[" MFX_PIN(1, "VCC", "power") "," MFX_PIN(2, "PB0", "inout") ","
    MFX_PIN(3, "PB1", "inout") "," MFX_PIN(4, "PB2", "inout") "," MFX_PIN(5, "PB3", "inout") ","
    MFX_PIN(6, "PB4", "inout") "," MFX_PIN(7, "PB5", "inout") "," MFX_PIN(8, "GND", "ground") "]},"
    "\"core\":\"avr\",\"clock\":20000000,\"flash\":32768,\"ram\":2048,\"eeprom\":1024,"
    "\"vcc\":[1.8,5.5],"
    "\"parts\":[{\"name\":\"TESTMCU\",\"package\":\"TSSOP\",\"pinout\":\"p8\","
    "\"footprint\":\"TSSOP-8\",\"clock\":10000000}]},"
    "{\"id\":\"mfx328\",\"function\":\"8-bit AVR\",\"description\":\"fixture\",\"drawer\":[],"
    "\"model\":\"atmega328\","
    "\"pinouts\":{\"p28\":[" MFX_P28 "]},"
    "\"core\":\"avr\",\"clock\":20000000,\"flash\":32768,\"ram\":2048,\"eeprom\":1024,"
    "\"vcc\":[1.8,5.5],"
    "\"parts\":[{\"name\":\"TESTAVR\",\"package\":\"DIP\",\"pinout\":\"p28\","
    "\"footprint\":\"PDIP-28-MC-28P3\"}]},"
    "{\"id\":\"mfx48\",\"function\":\"8-bit AVR\",\"description\":\"fixture\",\"drawer\":[],"
    "\"model\":\"atmega328\","
    "\"pinouts\":{\"p28\":[" MFX_P28 "]},"
    "\"core\":\"avr\",\"clock\":20000000,\"flash\":4096,\"ram\":512,\"eeprom\":256,"
    "\"vcc\":[1.8,5.5],"
    "\"parts\":[{\"name\":\"TESTAVR48\",\"package\":\"DIP\",\"pinout\":\"p28\","
    "\"footprint\":\"PDIP-28-MC-28P3\"}]}"
    "]}";

#undef MFX_P28

#undef MFX_PIN

/* Loads the fixture mcu; prints the error and returns -1 on failure. */
static int load_mcu_fixture(PartLibrary *library) {
    char *error = NULL;

    if (load_parts(library, MCU_FIXTURE, "mcu-fixture.json", &error) != 0) {
        fprintf(stderr, "mcu fixture: %s\n", error != NULL ? error : "(none)");
        free(error);
        return -1;
    }
    return 0;
}

#endif
