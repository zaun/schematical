/*
 * usbuart_test: the USB-to-UART bridges (model "usbuart") through the
 * circuit API, on the shipped parts. The terminal plays the PC: bytes sent
 * with circuit_serial_send leave on TXD, and what arrives on RXD is the
 * channel's sent log.
 */

#include <stdio.h>
#include <string.h>

#include "circuit.h"
#include "parts.h"
#include "read_file.h"

static PartLibrary *library;
static int failures;

static void check(int ok, const char *what) {
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

#define US(n) ((uint64_t)(n) * 1000000u)

typedef struct {
    Circuit *c;
    size_t chip;
} Board;

static size_t net(Board *b) {
    size_t n = 0;

    circuit_add_net(b->c, &n);
    return n;
}

static size_t part(Board *b, const char *name) {
    size_t i = 0;

    if (circuit_add_part(b->c, name, &i) != 0) {
        printf("add %s: %s\n", name, circuit_error(b->c));
    }
    return i;
}

/* A supply part on a new net; returns the net. */
static size_t rail(Board *b, const char *supply) {
    size_t n = net(b);

    circuit_connect(b->c, part(b, supply), "1", n);
    return n;
}

static void tie(Board *b, size_t p, const char *const *pins, size_t n) {
    size_t i;

    for (i = 0; pins[i] != NULL; i++) {
        circuit_connect(b->c, p, pins[i], n);
    }
}

static Board board(const char *chip) {
    Board b;

    b.c = circuit_create(library);
    b.chip = part(&b, chip);
    return b;
}

static size_t sent(Board *b, unsigned channel, uint16_t *frames, size_t cap) {
    size_t n = circuit_serial_sent_count(b->c, b->chip, channel);

    circuit_serial_sent(b->c, b->chip, channel, 0, frames, cap);
    return n;
}

static int send(Board *b, unsigned channel, const char *text) {
    return circuit_serial_send(b->c, b->chip, channel, (const uint8_t *)text, strlen(text));
}

/* XR21V1410 (QFN-16): 3 RTS, 4 CTS, 9 TX, 10 RX, 16 VCC, 1 and 13 GND. */
static const char *const V1410_VCC[] = {"16", NULL};
static const char *const V1410_GND[] = {"1", "13", NULL};

static Board v1410(size_t *tx, size_t *rx) {
    Board b = board("XR21V1410IL16-F");

    tie(&b, b.chip, V1410_VCC, rail(&b, "3V3"));
    tie(&b, b.chip, V1410_GND, rail(&b, "GND"));
    *tx = net(&b);
    *rx = net(&b);
    circuit_connect(b.c, b.chip, "9", *tx);
    circuit_connect(b.c, b.chip, "10", *rx);
    return b;
}

static void case_loopback(void) {
    uint16_t frames[4] = {0};
    Board b = board("XR21V1410IL16-F");
    size_t line;

    tie(&b, b.chip, V1410_VCC, rail(&b, "3V3"));
    tie(&b, b.chip, V1410_GND, rail(&b, "GND"));
    line = net(&b);
    circuit_connect(b.c, b.chip, "9", line);    /* TX */
    circuit_connect(b.c, b.chip, "10", line);   /* RX: looped back */
    check(circuit_serial_configure(b.c, b.chip, 0, 115200, 8, 0, 2, 0) == 0,
          "configure 115200 8N1 before start");
    check(circuit_start(b.c) == 0, "XR21V1410 starts on 3.3 V");
    check(circuit_serial_channels(b.c, b.chip) == 1, "one channel");
    check(send(&b, 0, "Hi") == 0, "send queued");
    circuit_advance(b.c, US(250));   /* 2 frames of 10 bits at 8.68 us */
    check(sent(&b, 0, frames, 4) == 2 && frames[0] == 'H' && frames[1] == 'i',
          "TX looped into RX: the terminal shows H i");
    circuit_destroy(b.c);
}

/* 0x55 at the default 9600 8N1: bit time 104.17 us. */
static void case_waveform(void) {
    size_t tx, rx;
    Board b = v1410(&tx, &rx);
    uint64_t t0;

    check(circuit_start(b.c) == 0, "start");
    circuit_advance(b.c, US(10));
    check(circuit_net_value(b.c, tx) == CIRCUIT_1, "TX idles high");
    send(&b, 0, "U");
    t0 = circuit_time(b.c);
    circuit_advance(b.c, US(52));
    check(circuit_net_value(b.c, tx) == CIRCUIT_0, "start bit low (mid-bit)");
    circuit_advance(b.c, US(104));
    check(circuit_net_value(b.c, tx) == CIRCUIT_1, "bit 0 of 0x55 is 1");
    circuit_advance(b.c, US(104));
    check(circuit_net_value(b.c, tx) == CIRCUIT_0, "bit 1 is 0");
    circuit_advance(b.c, US(104) * 8);
    check(circuit_net_value(b.c, tx) == CIRCUIT_1, "stop bit high");
    check(circuit_time(b.c) - t0 < US(1100), "one frame is about 1.04 ms at 9600");
    circuit_destroy(b.c);
}

/* RTS/CTS flow control: CTS (pin 4) high holds TX; RTS (pin 3) is asserted. */
static void case_flow(void) {
    size_t tx, rx;
    Board b = v1410(&tx, &rx);
    size_t cts = net(&b);
    size_t rts = net(&b);

    circuit_connect(b.c, b.chip, "4", cts);
    circuit_connect(b.c, b.chip, "3", rts);
    circuit_connect(b.c, part(&b, "PULLUP-3V3"), "1", cts);   /* CTS held high */
    check(circuit_serial_configure(b.c, b.chip, 0, 115200, 8, 0, 2, 1) == 0, "RTS/CTS flow on");
    check(circuit_start(b.c) == 0, "start");
    circuit_advance(b.c, US(5));
    check(circuit_net_value(b.c, rts) == CIRCUIT_0, "RTS is asserted (0) with the port open");
    send(&b, 0, "A");
    circuit_advance(b.c, US(200));
    check(circuit_net_value(b.c, tx) == CIRCUIT_1, "CTS high: nothing sent");
    check(circuit_serial_configure(b.c, b.chip, 0, 115200, 8, 0, 2, 0) == 0, "flow off while running");
    circuit_advance(b.c, US(30));
    check(circuit_net_value(b.c, tx) == CIRCUIT_0, "flow off: the queued byte starts");
    circuit_destroy(b.c);
}

/* A two-channel part: channel B looped back, channel A silent. */
static void case_two_channels(void) {
    static const char *const VCC[] = {"4", "13", "24", "31", "32", NULL};
    static const char *const GND[] = {"1", "3", "14", "23", "27", "28", NULL};
    uint16_t frames[4] = {0};
    Board b = board("XR21V1412IL32-F");
    size_t line;

    tie(&b, b.chip, VCC, rail(&b, "3V3"));
    tie(&b, b.chip, GND, rail(&b, "GND"));
    line = net(&b);
    circuit_connect(b.c, b.chip, "15", line);   /* TXB */
    circuit_connect(b.c, b.chip, "16", line);   /* RXB */
    circuit_serial_configure(b.c, b.chip, 1, 57600, 7, 2, 2, 0);   /* 7E1 */
    check(circuit_start(b.c) == 0, "XR21V1412 starts");
    check(circuit_serial_channels(b.c, b.chip) == 2, "two channels");
    send(&b, 1, "ok");
    circuit_advance(b.c, US(500));
    check(sent(&b, 1, frames, 4) == 2 && frames[0] == 'o' && frames[1] == 'k',
          "channel B 7E1 loopback: o k");
    check(sent(&b, 0, frames, 4) == 0, "channel A received nothing");
    circuit_destroy(b.c);
}

/* CY7C65213 (SSOP-28): RESETB (19) low releases TXD (1); GPIO0 (23) is the TX LED. */
static void case_reset_and_led(void) {
    Board b = board("CY7C65213-28PVXI");
    size_t vcc = rail(&b, "3V3");
    size_t gnd = rail(&b, "GND");
    size_t txd = net(&b);
    size_t led = net(&b);
    size_t rst = net(&b);

    circuit_connect(b.c, b.chip, "4", vcc);    /* VCCIO */
    circuit_connect(b.c, b.chip, "20", vcc);   /* VCC */
    circuit_connect(b.c, b.chip, "7", gnd);
    circuit_connect(b.c, b.chip, "18", gnd);
    circuit_connect(b.c, b.chip, "21", gnd);
    circuit_connect(b.c, b.chip, "1", txd);
    circuit_connect(b.c, b.chip, "23", led);
    circuit_connect(b.c, b.chip, "19", rst);
    check(circuit_start(b.c) == 0, "CY7C65213 starts on 3.3 V");
    circuit_advance(b.c, US(5));
    check(circuit_net_value(b.c, txd) == CIRCUIT_1 && circuit_net_value(b.c, led) == CIRCUIT_1,
          "RESETB open (Z, inactive): TXD idles 1, TX LED off (1)");
    send(&b, 0, "x");
    circuit_advance(b.c, US(60));
    check(circuit_net_value(b.c, led) == CIRCUIT_0, "TX LED on (0) while a frame goes out");
    circuit_destroy(b.c);

    b = board("CY7C65213-28PVXI");
    vcc = rail(&b, "3V3");
    gnd = rail(&b, "GND");
    txd = net(&b);
    circuit_connect(b.c, b.chip, "4", vcc);
    circuit_connect(b.c, b.chip, "20", vcc);
    circuit_connect(b.c, b.chip, "7", gnd);
    circuit_connect(b.c, b.chip, "19", gnd);   /* held in reset */
    circuit_connect(b.c, b.chip, "1", txd);
    check(circuit_start(b.c) == 0, "start held in reset");
    send(&b, 0, "x");
    circuit_advance(b.c, US(200));
    check(circuit_net_value(b.c, txd) == CIRCUIT_Z, "RESETB low: TXD released, nothing sent");
    circuit_destroy(b.c);
}

/* XR21B1420 (QFN-28): VCC_REG (7) on 5 V from USB, VCC (6) the 3.3 V I/O side. */
static void case_usb_supply(void) {
    Board b = board("XR21B1420IL28-F");

    circuit_connect(b.c, b.chip, "7", rail(&b, "5V"));
    circuit_connect(b.c, b.chip, "6", rail(&b, "3V3"));
    circuit_connect(b.c, b.chip, "3", rail(&b, "GND"));
    check(circuit_start(b.c) == 0, "XR21B1420: 5 V on VCC_REG and 3.3 V on VCC pass the voltage check");
    circuit_destroy(b.c);

    b = board("XR21B1420IL28-F");
    circuit_connect(b.c, b.chip, "6", rail(&b, "5V"));
    check(circuit_start(b.c) != 0 && strstr(circuit_error(b.c), "voltage") != NULL,
          "5 V on the 3.3 V VCC pin is refused");
    circuit_destroy(b.c);
}

static void case_errors(void) {
    Board b = board("SC16IS750IBS,128");

    check(circuit_serial_configure(b.c, b.chip, 0, 9600, 8, 0, 2, 0) != 0 &&
              strstr(circuit_error(b.c), "set by the part itself") != NULL,
          "a classic bridge's port is not host-set");
    circuit_destroy(b.c);
    b = board("XR21V1410IL16-F");
    check(circuit_serial_configure(b.c, b.chip, 0, 10, 8, 0, 2, 0) != 0, "baud 10 is out of range");
    check(circuit_serial_configure(b.c, b.chip, 0, 9600, 9, 0, 2, 0) != 0, "9 data bits are out of range");
    check(circuit_serial_configure(b.c, b.chip, 1, 9600, 8, 0, 2, 0) != 0, "no channel B");
    circuit_destroy(b.c);
}

static void case_facts(void) {
    const Part *p = part_library_find(library, "XR21B1424IV64-F");

    check(p != NULL && p->serial != NULL && strcmp(p->serial->model, "usbuart") == 0 &&
              strcmp(p->serial->host, "usb") == 0 && p->serial->channels == 4,
          "XR21B1424: usbuart, host usb, 4 channels");
    check(p != NULL && p->domain_count == 2, "XR21B1424: I/O and USB supply domains");
}

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL: loading parts\n");
        return 1;
    }
    case_facts();
    case_loopback();
    case_waveform();
    case_flow();
    case_two_channels();
    case_reset_and_led();
    case_usb_supply();
    case_errors();
    part_library_destroy(library);
    printf(failures ? "%d FAILED\n" : "all passed\n", failures);
    return failures != 0;
}
