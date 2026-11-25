#ifndef RELAY_FIXTURE_H
#define RELAY_FIXTURE_H

/*
 * Fixture parts for relay_test (plan-47-J section 6), loaded next to the real
 * part files (footprints from footprints.json). Pins in the order of their
 * numbers:
 *
 *   TESTRLY-SPDT  electromechanical, coil A1 A2, one pole COM NO NC; 5 V, 125 ohm, 10 A, 30 V,
 *                 operate 10 ms, release 5 ms: A1 A2 COM NO NC
 *   TESTRLY-DPDT  two poles: A1 A2 COM1 NO1 NC1 COM2 NO2 NC2
 *   TESTRLY-POL   as SPDT, the coil polarized
 *   TESTRLY-L1    latching, one coil: A1 A2 COM NO NC
 *   TESTRLY-L2    latching, two coils (S1 S2 set, R1 R2 reset): S1 S2 R1 R2 COM NO NC
 *   TESTRLY-L2C   latching, two coils with a common pin: S C R COM NO NC
 *   TESTRLY-NO    one pole with a normally-open contact only: A1 A2 COM NO
 *   TESTRLY-TIE   SPDT with two pins named COM: A1 A2 COM NO NC COM
 *   TESTSSR       solid-state, input A K, one pole D1-D2 (common, no): A K D1 D2
 *   TESTSSR2      solid-state, two channels: A1 K1 A2 K2 D1A D1B D2A D2B
 *
 * RELAY_DEVICE builds one device for the rejection cases.
 */

#include <stdio.h>
#include <stdlib.h>

#include "parts.h"

#define RFX_PIN(N, NAME, ROLE) "{\"number\":" #N ",\"name\":\"" NAME "\",\"role\":\"" ROLE "\"}"
#define RFX_IN(N, NAME) RFX_PIN(N, NAME, "input")
#define RFX_IO(N, NAME) RFX_PIN(N, NAME, "inout")
#define RFX_NC(N, NAME) RFX_PIN(N, NAME, "nc")
#define RELAY_DEVICE(ID, TYPE, EXTRA, PINS, NAME, FOOTPRINT, PART_KEYS)                          \
    "{\"id\":\"" ID "\",\"function\":\"fixture\",\"description\":\"fixture\",\"drawer\":[],"    \
    "\"pinouts\":{\"p\":[" PINS "]},\"type\":\"" TYPE "\"" EXTRA ","                            \
    "\"parts\":[{\"name\":\"" NAME "\",\"package\":\"TH\",\"pinout\":\"p\","                    \
    "\"footprint\":\"" FOOTPRINT "\"" PART_KEYS "}]}"
#define RELAY_FILE(DEVICES)                                                                      \
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"relay\",\"devices\":[" DEVICES "]}"

#define RFX_COIL(A, B) "{\"a\":\"" A "\",\"b\":\"" B "\"}"
#define RFX_COIL_POL(A, B) "{\"a\":\"" A "\",\"b\":\"" B "\",\"polarized\":true}"
#define RFX_POLE(C, NO, NC) "{\"common\":\"" C "\",\"no\":\"" NO "\",\"nc\":\"" NC "\"}"
#define RFX_POLE_COIL(C, NO, NC, N) "{\"common\":\"" C "\",\"no\":\"" NO "\",\"nc\":\"" NC "\",\"coil\":" #N "}"

#define RFX_SPDT_PINS RFX_IN(1, "A1") "," RFX_IN(2, "A2") "," RFX_IO(3, "COM") "," RFX_IO(4, "NO") "," RFX_IO(5, "NC")
#define RFX_SPDT_KEYS ",\"coilVolts\":5,\"coilOhms\":125,\"contactAmps\":10,\"contactVolts\":30,\"operateMs\":10,\"releaseMs\":5"
#define RFX_SPDT_EXTRA ",\"coils\":[" RFX_COIL("A1", "A2") "],\"contacts\":[" RFX_POLE("COM", "NO", "NC") "]"
#define RFX_PLAIN_KEYS ",\"coilVolts\":5,\"contactAmps\":1"

static const char RELAY_FIXTURE[] = RELAY_FILE(
    RELAY_DEVICE("rfx-spdt", "electromechanical", RFX_SPDT_EXTRA, RFX_SPDT_PINS, "TESTRLY-SPDT",
                 "PINHDR-1X05-V", RFX_SPDT_KEYS) ","
    RELAY_DEVICE("rfx-dpdt", "electromechanical",
                 ",\"coils\":[" RFX_COIL("A1", "A2") "],\"contacts\":[" RFX_POLE("COM1", "NO1", "NC1") ","
                 RFX_POLE("COM2", "NO2", "NC2") "]",
                 RFX_IN(1, "A1") "," RFX_IN(2, "A2") "," RFX_IO(3, "COM1") "," RFX_IO(4, "NO1") ","
                 RFX_IO(5, "NC1") "," RFX_IO(6, "COM2") "," RFX_IO(7, "NO2") "," RFX_IO(8, "NC2"),
                 "TESTRLY-DPDT", "PINHDR-1X08-V", RFX_PLAIN_KEYS) ","
    RELAY_DEVICE("rfx-pol", "electromechanical",
                 ",\"coils\":[" RFX_COIL_POL("A1", "A2") "],\"contacts\":[" RFX_POLE("COM", "NO", "NC") "]",
                 RFX_SPDT_PINS, "TESTRLY-POL", "PINHDR-1X05-V", RFX_PLAIN_KEYS) ","
    RELAY_DEVICE("rfx-l1", "electromechanical",
                 ",\"latching\":true,\"coils\":[" RFX_COIL("A1", "A2") "],\"contacts\":[" RFX_POLE("COM", "NO", "NC") "]",
                 RFX_SPDT_PINS, "TESTRLY-L1", "PINHDR-1X05-V", RFX_PLAIN_KEYS) ","
    RELAY_DEVICE("rfx-l2", "electromechanical",
                 ",\"latching\":true,\"coils\":[" RFX_COIL("S1", "S2") "," RFX_COIL("R1", "R2")
                 "],\"contacts\":[" RFX_POLE("COM", "NO", "NC") "]",
                 RFX_IN(1, "S1") "," RFX_IN(2, "S2") "," RFX_IN(3, "R1") "," RFX_IN(4, "R2") ","
                 RFX_IO(5, "COM") "," RFX_IO(6, "NO") "," RFX_IO(7, "NC"),
                 "TESTRLY-L2", "PINHDR-1X07-V", RFX_PLAIN_KEYS) ","
    RELAY_DEVICE("rfx-l2c", "electromechanical",
                 ",\"latching\":true,\"coils\":[" RFX_COIL("S", "C") "," RFX_COIL("R", "C")
                 "],\"contacts\":[" RFX_POLE("COM", "NO", "NC") "]",
                 RFX_IN(1, "S") "," RFX_IN(2, "C") "," RFX_IN(3, "R") "," RFX_IO(4, "COM") ","
                 RFX_IO(5, "NO") "," RFX_IO(6, "NC"),
                 "TESTRLY-L2C", "PINHDR-1X06-V", RFX_PLAIN_KEYS) ","
    RELAY_DEVICE("rfx-no", "electromechanical",
                 ",\"coils\":[" RFX_COIL("A1", "A2") "],\"contacts\":[{\"common\":\"COM\",\"no\":\"NO\"}]",
                 RFX_IN(1, "A1") "," RFX_IN(2, "A2") "," RFX_IO(3, "COM") "," RFX_IO(4, "NO"),
                 "TESTRLY-NO", "PINHDR-1X04-V", RFX_PLAIN_KEYS) ","
    RELAY_DEVICE("rfx-tie", "electromechanical", RFX_SPDT_EXTRA,
                 RFX_IN(1, "A1") "," RFX_IN(2, "A2") "," RFX_IO(3, "COM") "," RFX_IO(4, "NO") ","
                 RFX_IO(5, "NC") "," RFX_IO(6, "COM"),
                 "TESTRLY-TIE", "PINHDR-1X06-V", RFX_PLAIN_KEYS) ","
    RELAY_DEVICE("rfx-ssr", "solid-state",
                 ",\"coils\":[" RFX_COIL("A", "K") "],\"contacts\":[{\"common\":\"D1\",\"no\":\"D2\"}]",
                 RFX_IN(1, "A") "," RFX_IN(2, "K") "," RFX_IO(3, "D1") "," RFX_IO(4, "D2"),
                 "TESTSSR", "PINHDR-1X04-V", ",\"contactAmps\":0.12,\"contactVolts\":60,\"onOhms\":25") ","
    RELAY_DEVICE("rfx-ssr2", "solid-state",
                 ",\"coils\":[" RFX_COIL("A1", "K1") "," RFX_COIL("A2", "K2") "],\"contacts\":["
                 "{\"common\":\"D1A\",\"no\":\"D1B\",\"coil\":1},{\"common\":\"D2A\",\"no\":\"D2B\",\"coil\":2}]",
                 RFX_IN(1, "A1") "," RFX_IN(2, "K1") "," RFX_IN(3, "A2") "," RFX_IN(4, "K2") ","
                 RFX_IO(5, "D1A") "," RFX_IO(6, "D1B") "," RFX_IO(7, "D2A") "," RFX_IO(8, "D2B"),
                 "TESTSSR2", "PINHDR-1X08-V", ",\"contactAmps\":0.5"));

/* Loads the fixture parts; prints the error and returns -1 on failure. */
static int load_relay_fixture(PartLibrary *library) {
    char *error = NULL;

    if (load_parts(library, RELAY_FIXTURE, "relay-fixture.json", &error) != 0) {
        fprintf(stderr, "relay fixture: %s\n", error != NULL ? error : "(none)");
        free(error);
        return -1;
    }
    return 0;
}

#endif
