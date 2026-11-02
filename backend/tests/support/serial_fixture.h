#ifndef SERIAL_FIXTURE_H
#define SERIAL_FIXTURE_H

/*
 * Fixture serial devices (plan-12-C section 4), loaded next to the real part
 * files (their footprints come from footprints.json):
 *
 *   SFX-16550      PC16550D-shaped ns16550, plcc44 (PLCC-44), grade X
 *   SFX-16550-DIP  the same device, dip40 (PDIP-40-600)
 *   SFX-16450      the same with "fifo": 0, plcc44
 *
 * Pin names are the PC16550D's (plan-12-data/specs/ti-a.md section
 * PC16550D): D0-D7, A0-A2, CS0 CS1 (high) CS2 (low), ADS, RDBAR/RD,
 * WRBAR/WR, DDIS, MR, XIN, XOUT, RCLK, BAUDOUT, SIN, SOUT, RTS DTR OUT1
 * OUT2, CTS DSR DCD RI, INTR, TXRDY, RXRDY. Grade X: the PC16550D timing
 * (p4-5).
 */

#include <stdio.h>
#include <stdlib.h>

#include "parts.h"

#define SFX_PLCC44 \
    "\"plcc44\":[" \
    "{\"number\":1,\"name\":\"NC\",\"role\":\"nc\"},{\"number\":2,\"name\":\"D0\",\"role\":\"inout\"}," \
    "{\"number\":3,\"name\":\"D1\",\"role\":\"inout\"},{\"number\":4,\"name\":\"D2\",\"role\":\"inout\"}," \
    "{\"number\":5,\"name\":\"D3\",\"role\":\"inout\"},{\"number\":6,\"name\":\"D4\",\"role\":\"inout\"}," \
    "{\"number\":7,\"name\":\"D5\",\"role\":\"inout\"},{\"number\":8,\"name\":\"D6\",\"role\":\"inout\"}," \
    "{\"number\":9,\"name\":\"D7\",\"role\":\"inout\"},{\"number\":10,\"name\":\"RCLK\",\"role\":\"input\"}," \
    "{\"number\":11,\"name\":\"SIN\",\"role\":\"input\"},{\"number\":12,\"name\":\"NC\",\"role\":\"nc\"}," \
    "{\"number\":13,\"name\":\"SOUT\",\"role\":\"output\"},{\"number\":14,\"name\":\"CS0\",\"role\":\"input\"}," \
    "{\"number\":15,\"name\":\"CS1\",\"role\":\"input\"},{\"number\":16,\"name\":\"CS2\",\"role\":\"input\"}," \
    "{\"number\":17,\"name\":\"BAUDOUT\",\"role\":\"output\"},{\"number\":18,\"name\":\"XIN\",\"role\":\"input\"}," \
    "{\"number\":19,\"name\":\"XOUT\",\"role\":\"output\"},{\"number\":20,\"name\":\"WRBAR\",\"role\":\"input\"}," \
    "{\"number\":21,\"name\":\"WR\",\"role\":\"input\"},{\"number\":22,\"name\":\"VSS\",\"role\":\"ground\"}," \
    "{\"number\":23,\"name\":\"NC\",\"role\":\"nc\"},{\"number\":24,\"name\":\"RDBAR\",\"role\":\"input\"}," \
    "{\"number\":25,\"name\":\"RD\",\"role\":\"input\"},{\"number\":26,\"name\":\"DDIS\",\"role\":\"output\"}," \
    "{\"number\":27,\"name\":\"TXRDY\",\"role\":\"output\"},{\"number\":28,\"name\":\"ADS\",\"role\":\"input\"}," \
    "{\"number\":29,\"name\":\"A2\",\"role\":\"input\"},{\"number\":30,\"name\":\"A1\",\"role\":\"input\"}," \
    "{\"number\":31,\"name\":\"A0\",\"role\":\"input\"},{\"number\":32,\"name\":\"RXRDY\",\"role\":\"output\"}," \
    "{\"number\":33,\"name\":\"INTR\",\"role\":\"output\"},{\"number\":34,\"name\":\"NC\",\"role\":\"nc\"}," \
    "{\"number\":35,\"name\":\"OUT2\",\"role\":\"output\"},{\"number\":36,\"name\":\"RTS\",\"role\":\"output\"}," \
    "{\"number\":37,\"name\":\"DTR\",\"role\":\"output\"},{\"number\":38,\"name\":\"OUT1\",\"role\":\"output\"}," \
    "{\"number\":39,\"name\":\"MR\",\"role\":\"input\"},{\"number\":40,\"name\":\"CTS\",\"role\":\"input\"}," \
    "{\"number\":41,\"name\":\"DSR\",\"role\":\"input\"},{\"number\":42,\"name\":\"DCD\",\"role\":\"input\"}," \
    "{\"number\":43,\"name\":\"RI\",\"role\":\"input\"},{\"number\":44,\"name\":\"VDD\",\"role\":\"power\"}]"

#define SFX_DIP40 \
    "\"dip40\":[" \
    "{\"number\":1,\"name\":\"D0\",\"role\":\"inout\"},{\"number\":2,\"name\":\"D1\",\"role\":\"inout\"}," \
    "{\"number\":3,\"name\":\"D2\",\"role\":\"inout\"},{\"number\":4,\"name\":\"D3\",\"role\":\"inout\"}," \
    "{\"number\":5,\"name\":\"D4\",\"role\":\"inout\"},{\"number\":6,\"name\":\"D5\",\"role\":\"inout\"}," \
    "{\"number\":7,\"name\":\"D6\",\"role\":\"inout\"},{\"number\":8,\"name\":\"D7\",\"role\":\"inout\"}," \
    "{\"number\":9,\"name\":\"RCLK\",\"role\":\"input\"},{\"number\":10,\"name\":\"SIN\",\"role\":\"input\"}," \
    "{\"number\":11,\"name\":\"SOUT\",\"role\":\"output\"},{\"number\":12,\"name\":\"CS0\",\"role\":\"input\"}," \
    "{\"number\":13,\"name\":\"CS1\",\"role\":\"input\"},{\"number\":14,\"name\":\"CS2\",\"role\":\"input\"}," \
    "{\"number\":15,\"name\":\"BAUDOUT\",\"role\":\"output\"},{\"number\":16,\"name\":\"XIN\",\"role\":\"input\"}," \
    "{\"number\":17,\"name\":\"XOUT\",\"role\":\"output\"},{\"number\":18,\"name\":\"WRBAR\",\"role\":\"input\"}," \
    "{\"number\":19,\"name\":\"WR\",\"role\":\"input\"},{\"number\":20,\"name\":\"VSS\",\"role\":\"ground\"}," \
    "{\"number\":21,\"name\":\"RDBAR\",\"role\":\"input\"},{\"number\":22,\"name\":\"RD\",\"role\":\"input\"}," \
    "{\"number\":23,\"name\":\"DDIS\",\"role\":\"output\"},{\"number\":24,\"name\":\"TXRDY\",\"role\":\"output\"}," \
    "{\"number\":25,\"name\":\"ADS\",\"role\":\"input\"},{\"number\":26,\"name\":\"A2\",\"role\":\"input\"}," \
    "{\"number\":27,\"name\":\"A1\",\"role\":\"input\"},{\"number\":28,\"name\":\"A0\",\"role\":\"input\"}," \
    "{\"number\":29,\"name\":\"RXRDY\",\"role\":\"output\"},{\"number\":30,\"name\":\"INTR\",\"role\":\"output\"}," \
    "{\"number\":31,\"name\":\"OUT2\",\"role\":\"output\"},{\"number\":32,\"name\":\"RTS\",\"role\":\"output\"}," \
    "{\"number\":33,\"name\":\"DTR\",\"role\":\"output\"},{\"number\":34,\"name\":\"OUT1\",\"role\":\"output\"}," \
    "{\"number\":35,\"name\":\"MR\",\"role\":\"input\"},{\"number\":36,\"name\":\"CTS\",\"role\":\"input\"}," \
    "{\"number\":37,\"name\":\"DSR\",\"role\":\"input\"},{\"number\":38,\"name\":\"DCD\",\"role\":\"input\"}," \
    "{\"number\":39,\"name\":\"RI\",\"role\":\"input\"},{\"number\":40,\"name\":\"VDD\",\"role\":\"power\"}]"

/* The PC16550D's ns16550 keys (plan-12-C section 3.6) without "model" and
   "grades"; FIFO = 16 or 0, TRIGGER the rxTrigger member (empty for none). */
#define SFX_MODEL_KEYS(FIFO, TRIGGER) \
    "\"bus\":{\"style\":\"intel\",\"data\":[\"D0\",\"D1\",\"D2\",\"D3\",\"D4\",\"D5\",\"D6\",\"D7\"]," \
    "\"address\":[\"A0\",\"A1\",\"A2\"],\"read\":\"RDBAR\",\"readHigh\":\"RD\",\"write\":\"WRBAR\"," \
    "\"writeHigh\":\"WR\",\"select\":[\"CS2\"],\"selectHigh\":[\"CS0\",\"CS1\"],\"addressStrobe\":\"ADS\"," \
    "\"driverDisable\":\"DDIS\",\"reset\":\"MR\",\"resetActiveHigh\":true}," \
    "\"clock\":{\"in\":\"XIN\",\"out\":\"XOUT\",\"receive\":\"RCLK\",\"baudOut\":\"BAUDOUT\"}," \
    "\"channels\":[{\"tx\":\"SOUT\",\"rx\":\"SIN\",\"rts\":\"RTS\",\"dtr\":\"DTR\",\"cts\":\"CTS\"," \
    "\"dsr\":\"DSR\",\"dcd\":\"DCD\",\"ri\":\"RI\",\"out1\":\"OUT1\",\"out2\":\"OUT2\",\"int\":\"INTR\"," \
    "\"txrdy\":\"TXRDY\",\"rxrdy\":\"RXRDY\"}]," \
    "\"fifo\":" FIFO TRIGGER ",\"intPin\":\"driven\",\"dmaClass\":\"P\",\"timeout\":\"4ch\""

/* Grade X: the PC16550D timing (ns), p4-5. */
#define SFX_GRADES \
    "\"grades\":{\"X\":{\"trvd\":60,\"thz\":100,\"trdd\":60,\"tbld\":175,\"trint\":1000,\"thr\":175," \
    "\"tmdo\":200,\"tsim\":250,\"trim\":250,\"twxi\":195,\"twrr\":290,\"tir\":250,\"vcc\":[4.5,5.5]," \
    "\"note\":\"pc16550d.pdf p4-5\"}}"

#define SFX_KEYS(FIFO, TRIGGER) \
    "\"model\":\"ns16550\"," SFX_MODEL_KEYS(FIFO, TRIGGER) "," SFX_GRADES

#define SFX_DEVICE(ID, FIFO, TRIGGER, PARTS) \
    "{\"id\":\"" ID "\",\"function\":\"UART\",\"description\":\"fixture\",\"drawer\":[\"Interface\"]," \
    "\"pinouts\":{" SFX_PLCC44 "," SFX_DIP40 "}," SFX_KEYS(FIFO, TRIGGER) ",\"parts\":[" PARTS "]}"

static const char SERIAL_FIXTURE[] =
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"serial\",\"devices\":["
    SFX_DEVICE("sfx-16550", "16", ",\"rxTrigger\":[1,4,8,14]",
               "{\"name\":\"SFX-16550\",\"package\":\"PLCC\",\"pinout\":\"plcc44\",\"footprint\":\"PLCC-44\",\"grade\":\"X\"},"
               "{\"name\":\"SFX-16550-DIP\",\"package\":\"DIP\",\"pinout\":\"dip40\",\"footprint\":\"PDIP-40-600\",\"grade\":\"X\"}") ","
    SFX_DEVICE("sfx-16450", "0", "",
               "{\"name\":\"SFX-16450\",\"package\":\"PLCC\",\"pinout\":\"plcc44\",\"footprint\":\"PLCC-44\",\"grade\":\"X\"}")
    "]}";

/* Loads the fixture devices into a library that already has footprints.json. */
static inline int load_serial_fixture(PartLibrary *library) {
    char *error = NULL;
    int result = load_parts(library, SERIAL_FIXTURE, "serial_fixture.json", &error);

    if (result != 0) {
        fprintf(stderr, "serial fixture: %s\n", error != NULL ? error : "(no message)");
    }
    free(error);
    return result;
}

#endif
