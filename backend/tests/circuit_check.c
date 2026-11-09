/*
 * circuit_check: the circuit API (circuit.h) with parts from the JSON part
 * files. Every scenario the old netlist-level tests covered
 * (netlist_check.c, switch_check.c check_netlist, sram_check.c
 * check_netlist_inspect), plus the new error cases.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "parts.h"
#include "read_file.h"
#include "voltage_fixture.h"
#include "serial_fixture.h"
#include "serial_fixture_classic.h"

static int failures;
static PartLibrary *library;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

static size_t part(Circuit *c, const char *name) {
    size_t index = (size_t)-1;

    if (circuit_add_part(c, name, &index) != 0) {
        printf("  add %s: %s\n", name, circuit_error(c));
    }
    return index;
}

static size_t net(Circuit *c) {
    size_t index = (size_t)-1;

    circuit_add_net(c, &index);
    return index;
}

static int light(Circuit *c, size_t index) {
    unsigned char on = 0;

    return circuit_part_lights(c, index, &on, 1) == 1 ? on : -1;
}

/* VCC on the anode (pin 2), GND on the cathode (pin 1): lit after one step. */
static void check_lit_led(void) {
    Circuit *c = circuit_create(library);
    size_t vcc = part(c, "5V");
    size_t gnd = part(c, "GND");
    size_t led = part(c, "GENERIC-LED");
    size_t n0 = net(c);
    size_t n1 = net(c);

    circuit_connect(c, vcc, "1", n0);
    circuit_connect(c, gnd, "1", n1);
    circuit_connect(c, led, "2", n0);
    circuit_connect(c, led, "1", n1);
    expect(circuit_start(c) == 0 && circuit_advance(c, 1000) == 1, "LED circuit runs");
    expect(light(c, led) == 1, "LED with anode high and cathode low is lit");
    circuit_destroy(c);
}

/* plan-23-A: a header is passive terminals. 5V reaches the LED's anode
   through header pin 1's net and GND its cathode through pin 40's; the
   other 38 pins are left open. */
static void check_header_passes_nets(void) {
    Circuit *c = circuit_create(library);
    size_t vcc = part(c, "5V");
    size_t gnd = part(c, "GND");
    size_t header = part(c, "GENERIC-HEADER-2X20");
    size_t led = part(c, "GENERIC-LED");
    size_t supply_net = net(c);
    size_t ground_net = net(c);

    expect(header != (size_t)-1, "GENERIC-HEADER-2X20 is added");
    circuit_connect(c, vcc, "1", supply_net);
    circuit_connect(c, header, "1", supply_net);
    circuit_connect(c, led, "2", supply_net);
    circuit_connect(c, gnd, "1", ground_net);
    circuit_connect(c, header, "40", ground_net);
    circuit_connect(c, led, "1", ground_net);
    expect(circuit_start(c) == 0 && circuit_advance(c, 1000) == 1, "header circuit runs");
    expect(light(c, led) == 1, "LED wired from header pin 1 (A) to pin 40 (K) is lit");
    circuit_destroy(c);
}

/* The anode is left unconnected, so it reads Z and the LED stays dark. */
static void check_dark_led(void) {
    Circuit *c = circuit_create(library);
    size_t vcc = part(c, "5V");
    size_t gnd = part(c, "GND");
    size_t led = part(c, "GENERIC-LED");
    size_t n0 = net(c);
    size_t n1 = net(c);

    circuit_connect(c, vcc, "1", n0);
    circuit_connect(c, gnd, "1", n1);
    circuit_connect(c, led, "1", n1);
    circuit_start(c);
    circuit_advance(c, 1000);
    expect(light(c, led) == 0, "LED with an unconnected anode is dark");
    circuit_destroy(c);
}

/* VCC and GND both drive net 0. */
static void check_short(void) {
    Circuit *c = circuit_create(library);
    size_t vcc = part(c, "5V");
    size_t gnd = part(c, "GND");
    size_t n0 = net(c);
    size_t shorted = 99;

    circuit_connect(c, vcc, "1", n0);
    circuit_connect(c, gnd, "1", n0);
    circuit_start(c);
    expect(circuit_advance(c, 1000) == 0, "a short stops the run");
    expect(circuit_error(c) != NULL && strncmp(circuit_error(c), "smoke:", 6) == 0,
           "a short is a smoke error");
    expect(circuit_shorted_net(c, &shorted) == 1 && shorted == 0, "the shorted net is reported");
    expect(circuit_advance(c, 1000) == 0, "advance stays stopped");
    circuit_destroy(c);
}

/* plan-12-A: a pull part drives weakly; a strong driver wins without a short. */
static void check_pulls(void) {
    Circuit *c = circuit_create(library);
    size_t pu = part(c, "PULLUP-5V");
    size_t n0 = net(c);
    size_t shorted = 99;

    circuit_connect(c, pu, "1", n0);
    expect(circuit_start(c) == 0 && circuit_advance(c, 1000) == 1, "pull-up circuit runs");
    expect(circuit_net_value(c, n0) == CIRCUIT_1, "a net with only a pull-up reads 1");
    circuit_destroy(c);

    c = circuit_create(library);
    pu = part(c, "PULLUP-5V");
    {
        size_t gnd = part(c, "GND");

        n0 = net(c);
        circuit_connect(c, pu, "1", n0);
        circuit_connect(c, gnd, "1", n0);
    }
    expect(circuit_start(c) == 0 && circuit_advance(c, 1000) == 1,
           "pull-up against GND runs without smoke");
    expect(circuit_net_value(c, n0) == CIRCUIT_0, "a strong 0 beats a pull-up");
    expect(circuit_shorted_net(c, &shorted) == 0, "a pull-up against GND is no short");
    circuit_destroy(c);

    c = circuit_create(library);
    pu = part(c, "PULLUP-3V3");
    {
        size_t pd = part(c, "PULLDOWN");

        n0 = net(c);
        circuit_connect(c, pu, "1", n0);
        circuit_connect(c, pd, "1", n0);
    }
    circuit_start(c);
    circuit_advance(c, 1000);
    expect(circuit_net_value(c, n0) == CIRCUIT_X, "a pull-up and a pull-down read X");
    circuit_destroy(c);
}

/* ---- plan-12-C: the serial terminal API --------------------------------- */

/* The designator of the pin named `name` on a library part. */
static const char *pin_named(const char *part_name, const char *name) {
    const Part *p = part_library_find(library, part_name);
    size_t i;

    for (i = 0; p != NULL && i < p->pin_count; i++) {
        if (strcmp(p->pins[i].name, name) == 0) {
            return p->pins[i].designator;
        }
    }
    printf("  %s has no pin %s\n", part_name, name);
    return "?";
}

/* SFX-16550 driven by two DIP switches with pull-ups: every switch position
   pulls its line to GND when on. Positions: TE-GDS0804 1-8 = D0-D7,
   TE-GDS0604 1-3 = A0-A2, 4 = WRBAR, 5 = RDBAR. */
typedef struct {
    Circuit *c;
    size_t uart, s8, s6;
    size_t data[8];
} SerialRig;

static void rig_line(SerialRig *r, size_t sw, unsigned position, size_t net_index, size_t gnd_net) {
    const char *name = sw == r->s8 ? "TE-GDS0804" : "TE-GDS0604";
    char a[4];
    char b[4];
    size_t pu = part(r->c, "PULLUP-5V");

    snprintf(a, sizeof a, "%uA", position);
    snprintf(b, sizeof b, "%uB", position);
    circuit_connect(r->c, pu, "1", net_index);
    circuit_connect(r->c, sw, pin_named(name, a), net_index);
    circuit_connect(r->c, sw, pin_named(name, b), gnd_net);
}

static void rig_uart(SerialRig *r, const char *pin, size_t net_index) {
    circuit_connect(r->c, r->uart, pin_named("SFX-16550", pin), net_index);
}

static void rig_build(SerialRig *r) {
    static const char *const DATA[8] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"};
    static const char *const CONTROL[5] = {"A0", "A1", "A2", "WRBAR", "RDBAR"};
    size_t v;
    size_t g;
    size_t clk;
    size_t baud;
    size_t sin;
    size_t osc;
    unsigned i;

    r->c = circuit_create(library);
    r->uart = part(r->c, "SFX-16550");
    circuit_connect(r->c, part(r->c, "5V"), "1", v = net(r->c));
    circuit_connect(r->c, part(r->c, "GND"), "1", g = net(r->c));
    r->s8 = part(r->c, "TE-GDS0804");
    r->s6 = part(r->c, "TE-GDS0604");
    osc = part(r->c, "ECS-100A-018");
    clk = net(r->c);
    baud = net(r->c);
    sin = net(r->c);
    circuit_connect(r->c, osc, "14", v);
    circuit_connect(r->c, osc, "7", g);
    circuit_connect(r->c, osc, "8", clk);
    rig_uart(r, "VDD", v);
    rig_uart(r, "VSS", g);
    rig_uart(r, "XIN", clk);
    rig_uart(r, "BAUDOUT", baud);
    rig_uart(r, "RCLK", baud);
    rig_uart(r, "CS0", v);
    rig_uart(r, "CS1", v);
    rig_uart(r, "CTS", v);
    rig_uart(r, "DSR", v);
    rig_uart(r, "DCD", v);
    rig_uart(r, "RI", v);
    rig_uart(r, "CS2", g);
    rig_uart(r, "ADS", g);
    rig_uart(r, "RD", g);
    rig_uart(r, "WR", g);
    rig_uart(r, "MR", g);
    rig_uart(r, "SIN", sin);
    circuit_connect(r->c, part(r->c, "PULLUP-5V"), "1", sin);
    for (i = 0; i < 8; i++) {
        r->data[i] = net(r->c);
        rig_uart(r, DATA[i], r->data[i]);
        rig_line(r, r->s8, i + 1, r->data[i], g);
    }
    for (i = 0; i < 5; i++) {
        size_t n = net(r->c);

        rig_uart(r, CONTROL[i], n);
        rig_line(r, r->s6, i + 1, n, g);
    }
}

static void rig_address(SerialRig *r, unsigned a) {
    unsigned i;

    for (i = 0; i < 3; i++) {
        circuit_set_switch(r->c, r->s6, i + 1, !((a >> i) & 1u));
    }
}

static void rig_write(SerialRig *r, unsigned a, unsigned data) {
    unsigned i;

    rig_address(r, a);
    for (i = 0; i < 8; i++) {
        circuit_set_switch(r->c, r->s8, i + 1, !((data >> i) & 1u));
    }
    circuit_advance(r->c, 100000);
    circuit_set_switch(r->c, r->s6, 4, 1);
    circuit_advance(r->c, 200000);
    circuit_set_switch(r->c, r->s6, 4, 0);
    circuit_advance(r->c, 100000);
    for (i = 0; i < 8; i++) {
        circuit_set_switch(r->c, r->s8, i + 1, 0);
    }
    circuit_advance(r->c, 100000);
}

/* A read: the data nets during the strobe; -1 when a bit is not 0 or 1. */
static int rig_read(SerialRig *r, unsigned a) {
    int value = 0;
    unsigned i;

    rig_address(r, a);
    circuit_advance(r->c, 100000);
    circuit_set_switch(r->c, r->s6, 5, 1);
    circuit_advance(r->c, 200000);
    for (i = 0; i < 8; i++) {
        CircuitValue bit = circuit_net_value(r->c, r->data[i]);

        if (bit == CIRCUIT_1) {
            value |= 1 << i;
        } else if (bit != CIRCUIT_0) {
            value = -1;
            break;
        }
    }
    circuit_set_switch(r->c, r->s6, 5, 0);
    circuit_advance(r->c, 200000);
    return value;
}

static void check_serial_terminal(void) {
    SerialRig r;
    uint16_t frames[4] = {0};
    const uint8_t ok[2] = {'O', 'K'};
    const uint64_t bit = 16 * 542535;   /* 16 clocks of 1.8432 MHz */
    const size_t gnd = 2;               /* UART 0, 5V 1, GND 2 */
    int first;
    int second;

    rig_build(&r);
    expect(circuit_serial_channels(r.c, r.uart) == 1, "a UART has 1 serial channel");
    expect(circuit_serial_channels(r.c, gnd) == 0, "a supply has no serial channel");
    expect(circuit_serial_send(r.c, r.uart, 0, ok, 2) == -1 &&
               strcmp(circuit_error(r.c), "the circuit is not running") == 0,
           "send before start is refused");
    expect(circuit_start(r.c) == 0, "UART rig starts");
    circuit_advance(r.c, 1000000);
    rig_write(&r, 3, 0x80);
    rig_write(&r, 0, 0x01);
    rig_write(&r, 1, 0x00);
    rig_write(&r, 3, 0x03);
    rig_write(&r, 2, 0x01);
    rig_write(&r, 0, 'H');
    rig_write(&r, 0, 'i');
    circuit_advance(r.c, 24 * bit);
    expect(circuit_serial_sent_count(r.c, r.uart, 0) == 2 &&
               circuit_serial_sent(r.c, r.uart, 0, 0, frames, 4) == 2 && frames[0] == 0x48 &&
               frames[1] == 0x69,
           "8N1 at 115200 sends H and i: 0x48 0x69");
    expect(circuit_serial_sent(r.c, r.uart, 0, 1, frames, 4) == 1 && frames[0] == 0x69,
           "circuit_serial_sent from frame 1");
    expect(circuit_serial_send(r.c, r.uart, 0, ok, 2) == 0,
           "typing OK is accepted (only a pull-up on RX)");
    circuit_advance(r.c, 24 * bit);
    first = rig_read(&r, 0);
    second = rig_read(&r, 0);
    expect(first == 0x4F && second == 0x4B, "the RBR reads 0x4F, then 0x4B");
    if (first != 0x4F || second != 0x4B) {
        printf("  read %d %d\n", first, second);
    }
    expect(circuit_serial_send(r.c, r.uart, 1, ok, 2) == -1 &&
               strcmp(circuit_error(r.c), "channel 1 of part 0 does not exist") == 0,
           "send to a channel out of range is refused");
    expect(circuit_serial_send(r.c, gnd, 0, ok, 2) == -1 &&
               strcmp(circuit_error(r.c), "part 2 is not a serial part") == 0,
           "send to a part that is not serial is refused");
    circuit_destroy(r.c);

    /* Another part's output on the RX net. */
    {
        Circuit *c = circuit_create(library);
        size_t u = part(c, "SFX-16550");
        size_t osc = part(c, "ECS-100A-018");
        size_t n = net(c);

        circuit_connect(c, u, pin_named("SFX-16550", "SIN"), n);
        circuit_connect(c, osc, "8", n);
        circuit_start(c);
        expect(circuit_serial_send(c, u, 0, ok, 2) == -1 &&
                   strcmp(circuit_error(c), "the RX pin of channel 0 of part 0 is driven by part 1") == 0,
               "send onto an RX net another part drives is refused");
        circuit_destroy(c);
    }
}

/* ---- plan-12-H: the terminal on an 8251A in synchronous mode -------------- */

/* SFX-8251A-dip28 on two DIP switches with pull-ups: TE-GDS0804 1-8 =
   D0-D7, TE-GDS0604 1 = C/D, 2 = WR; CS, RESET low, RD high. */
static void sync_write(SerialRig *r, int cd, unsigned data) {
    unsigned i;

    for (i = 0; i < 8; i++) {
        circuit_set_switch(r->c, r->s8, i + 1, !((data >> i) & 1u));
    }
    circuit_set_switch(r->c, r->s6, 1, !cd);
    circuit_advance(r->c, 100000);
    circuit_set_switch(r->c, r->s6, 2, 1);
    circuit_advance(r->c, 300000);
    circuit_set_switch(r->c, r->s6, 2, 0);
    circuit_advance(r->c, 3000000);
}

static void check_serial_sync_8251(void) {
    static const char *const DATA[8] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"};
    SerialRig r;
    const uint8_t ok[2] = {'O', 'K'};
    size_t v;
    size_t g;
    size_t rxd;
    unsigned i;

    memset(&r, 0, sizeof r);
    r.c = circuit_create(library);
    r.uart = part(r.c, "SFX-8251A-dip28");
    circuit_connect(r.c, part(r.c, "5V"), "1", v = net(r.c));
    circuit_connect(r.c, part(r.c, "GND"), "1", g = net(r.c));
    r.s8 = part(r.c, "TE-GDS0804");
    r.s6 = part(r.c, "TE-GDS0604");
    circuit_connect(r.c, r.uart, pin_named("SFX-8251A-dip28", "VCC"), v);
    circuit_connect(r.c, r.uart, pin_named("SFX-8251A-dip28", "GND"), g);
    circuit_connect(r.c, r.uart, pin_named("SFX-8251A-dip28", "CS"), g);
    circuit_connect(r.c, r.uart, pin_named("SFX-8251A-dip28", "RESET"), g);
    circuit_connect(r.c, r.uart, pin_named("SFX-8251A-dip28", "RD"), v);
    circuit_connect(r.c, r.uart, pin_named("SFX-8251A-dip28", "CTS"), g);
    circuit_connect(r.c, r.uart, pin_named("SFX-8251A-dip28", "DSR"), v);
    rxd = net(r.c);
    circuit_connect(r.c, r.uart, pin_named("SFX-8251A-dip28", "RXD"), rxd);
    circuit_connect(r.c, part(r.c, "PULLUP-5V"), "1", rxd);
    for (i = 0; i < 8; i++) {
        size_t n = net(r.c);

        circuit_connect(r.c, r.uart, pin_named("SFX-8251A-dip28", DATA[i]), n);
        rig_line(&r, r.s8, i + 1, n, g);
    }
    for (i = 0; i < 2; i++) {
        size_t n = net(r.c);

        circuit_connect(r.c, r.uart, pin_named("SFX-8251A-dip28", i == 0 ? "CD" : "WR"), n);
        rig_line(&r, r.s6, i + 1, n, g);
    }
    expect(circuit_start(r.c) == 0, "8251A rig starts");
    circuit_advance(r.c, 1000000);
    /* Mode 0x00: synchronous, 5 bits, double sync. */
    sync_write(&r, 1, 0x00);
    expect(circuit_serial_send(r.c, r.uart, 0, ok, 2) == -1 &&
               strcmp(circuit_error(r.c), "channel 0 of part 0 is not asynchronous") == 0,
           "send to an 8251A in synchronous mode: channel 0 of part 0 is not asynchronous");
    /* SYNC1, SYNC2, then IR and an asynchronous mode: accepted. */
    sync_write(&r, 1, 0x16);
    sync_write(&r, 1, 0x16);
    sync_write(&r, 1, 0x40);
    sync_write(&r, 1, 0x4E);
    sync_write(&r, 1, 0x37);
    expect(circuit_serial_send(r.c, r.uart, 0, ok, 2) == 0,
           "after IR and an async mode the same 8251A takes the terminal");
    circuit_destroy(r.c);
}

static void check_unknown_part(void) {
    Circuit *c = circuit_create(library);
    size_t index;

    expect(circuit_add_part(c, "NOPE", &index) == -1 && circuit_error(c) != NULL &&
               strstr(circuit_error(c), "unknown part") != NULL,
           "unknown part name fails with a message");
    circuit_destroy(c);
}

/* SiT1534 (32.768 kHz) output on an LED anode, cathode to GND. */
static void check_oscillator_blinks(void) {
    Circuit *c = circuit_create(library);
    const Part *osc_part = part_library_find(library, "OSC-SIT1534-2012");
    size_t osc = part(c, "OSC-SIT1534-2012");
    size_t gnd = part(c, "GND");
    size_t led = part(c, "GENERIC-LED");
    size_t n0 = net(c);
    size_t n1 = net(c);
    const char *out = NULL;
    size_t i;
    int last = -1;
    int transitions = 0;

    for (i = 0; osc_part != NULL && i < osc_part->pin_count; i++) {
        if (osc_part->pins[i].role == PIN_OUTPUT) {
            out = osc_part->pins[i].designator;
            break;
        }
    }
    circuit_connect(c, osc, out, n0);
    circuit_connect(c, gnd, "1", n1);
    circuit_connect(c, led, "2", n0);
    circuit_connect(c, led, "1", n1);
    circuit_start(c);
    /* About 1 ms of simulated time, in 1 us steps. */
    for (i = 0; i < 1000; i++) {
        int lit;

        if (circuit_advance(c, 1000000) != 1) {
            break;
        }
        lit = light(c, led);
        if (last != -1 && lit != last) {
            transitions++;
        }
        last = lit;
    }
    expect(transitions >= 2, "LED blinks while the oscillator runs");
    circuit_destroy(c);
}

/*
 * VCC -> switch position 2 -> net 1 -> LED anode; LED cathode -> GND.
 * Position 2 of a 4-position switch is pins 2 and 7.
 */
static void check_switch(void) {
    Circuit *c = circuit_create(library);
    size_t vcc = part(c, "5V");
    size_t sw = part(c, "TE-GDS0404");
    size_t led = part(c, "GENERIC-LED");
    size_t gnd = part(c, "GND");
    size_t n0 = net(c);
    size_t n1 = net(c);
    size_t n2 = net(c);
    Circuit *small;

    circuit_connect(c, vcc, "1", n0);
    circuit_connect(c, sw, "2", n0);
    circuit_connect(c, sw, "7", n1);
    circuit_connect(c, led, "2", n1);
    circuit_connect(c, led, "1", n2);
    circuit_connect(c, gnd, "1", n2);
    expect(circuit_set_switch(c, sw, 2, 1) == 0, "a switch position turns ON before start");
    expect(circuit_start(c) == 0 && circuit_advance(c, 1000) == 1, "switch circuit runs");
    expect(circuit_net_value(c, n1) == CIRCUIT_1 && light(c, led) == 1,
           "a switch that starts ON passes VCC and lights the LED");

    expect(circuit_set_switch(c, sw, 2, 0) == 0, "the switch turns OFF while running");
    expect(circuit_advance(c, 1000) == 1, "runs after turning OFF");
    expect(circuit_net_value(c, n1) == CIRCUIT_Z, "turning the switch OFF lets the net float");
    /* The LED was on at the start of that step (the contact opens at its first instant). */
    expect(light(c, led) == 1, "the LED counts as lit for the step it went out in");
    expect(circuit_advance(c, 1000) == 1 && light(c, led) == 0, "the next step it is dark");

    expect(circuit_set_switch(c, sw, 2, 1) == 0, "the switch turns back ON");
    expect(circuit_advance(c, 1000) == 1 && circuit_net_value(c, n1) == CIRCUIT_1 &&
               light(c, led) == 1,
           "turning it back ON lights the LED");

    expect(circuit_set_switch(c, led, 1, 1) == -1 && strstr(circuit_error(c), "is not a switch") != NULL,
           "an LED is not a switch");
    expect(circuit_set_switch(c, sw, 9, 1) == -1, "position 9 is out of range");
    circuit_destroy(c);

    small = circuit_create(library);
    sw = part(small, "TE-GDS0204");
    expect(circuit_set_switch(small, sw, 3, 1) == -1, "a missing position is rejected");
    circuit_destroy(small);
}

/* AS6C62256 DIP: CE# (20) and WE# (27) to VCC, VSS (14) to GND, VCC (28) to VCC. */
static void check_memory(void) {
    Circuit *c = circuit_create(library);
    size_t vcc = part(c, "5V");
    size_t gnd = part(c, "GND");
    size_t ram = part(c, "AS6C62256-55PCN");
    size_t n0 = net(c);
    size_t n1 = net(c);
    uint32_t values[4] = {1, 1, 1, 1};
    uint32_t known[4] = {1, 1, 1, 1};
    uint32_t words = 0;
    unsigned bits = 0;

    circuit_connect(c, vcc, "1", n0);
    circuit_connect(c, gnd, "1", n1);
    circuit_connect(c, ram, "20", n0);
    circuit_connect(c, ram, "27", n0);
    circuit_connect(c, ram, "14", n1);
    circuit_connect(c, ram, "28", n0);
    expect(circuit_start(c) == 0 && circuit_advance(c, 1000000) == 1, "circuit with an SRAM runs");
    expect(circuit_memory_size(c, ram, &words, &bits) && words == 32768 && bits == 8,
           "circuit_memory_size reports 32768 x 8");
    expect(!circuit_memory_size(c, vcc, &words, &bits), "a supply is not a memory");
    expect(circuit_memory_read(c, ram, 32766, 4, values, known) == 2,
           "memory_read clips at the end of memory");
    expect(known[0] == 0 && known[1] == 0, "unwritten words are X");
    expect(circuit_memory_read(c, ram, 32768, 1, values, known) == 0,
           "reading past the end reads nothing");
    expect(circuit_part_lights(c, ram, NULL, 0) == 0, "an SRAM has no lights");
    circuit_destroy(c);
}

/* A ball-pinout LED from an inline part file: "B2" (anode) on VCC, "A1" on GND lights it. */
/* Error text matches exactly (plan-04-B section 4.1). */
static void expect_error(Circuit *c, int result, const char *message, const char *what) {
    const char *error = circuit_error(c);
    int ok = result == -1 && error != NULL && strcmp(error, message) == 0;

    if (!ok) {
        printf("  got %d \"%s\", wanted \"%s\"\n", result, error != NULL ? error : "(none)", message);
    }
    expect(ok, what);
}

/* circuit_memory_load: before start only, EEPROMs only, in range, values that fit. */
static void check_memory_load(void) {
    Circuit *c = circuit_create(library);
    size_t rom = part(c, "AT28C256-15PU");
    size_t ram = part(c, "AS6C62256-55PCN");
    static const uint32_t TWO[2] = {0x12, 0x34};
    static const uint32_t THREE[3] = {1, 2, 3};
    static const uint32_t WIDE[1] = {0x100};
    uint32_t values[2] = {0, 0};
    uint32_t known[2] = {0, 0};

    expect(circuit_memory_load(c, rom, 0x7FFE, TWO, 2) == 0, "memory load: two words at 0x7FFE");
    circuit_memory_read(c, rom, 0x7FFE, 2, values, known);
    expect(values[0] == 0x12 && values[1] == 0x34 && known[0] == 0xFF && known[1] == 0xFF,
           "memory load: read back 12 34, every bit known");
    circuit_memory_read(c, rom, 0, 1, values, known);
    expect(values[0] == 0xFF && known[0] == 0xFF, "memory load: word 0 still erased (FF)");
    expect(circuit_memory_load(c, rom, 5, TWO, 0) == 0, "memory load: count 0 succeeds");

    expect_error(c, circuit_memory_load(c, 99, 0, TWO, 1), "no part 99", "memory load: part 99");
    expect_error(c, circuit_memory_load(c, ram, 0, TWO, 1),
                 "part 1 (AS6C62256-55PCN) has no loadable memory", "memory load: an SRAM is not loadable");
    expect_error(c, circuit_memory_load(c, rom, 0x7FFE, THREE, 3),
                 "memory load of 3 words at 32766 is past the end of AT28C256-15PU (32768 words)",
                 "memory load: past the end");
    expect_error(c, circuit_memory_load(c, rom, 0x10, WIDE, 1), "word 16: 0x100 does not fit 8 bits",
                 "memory load: a value wider than the word");

    expect(circuit_start(c) == 0, "memory load: circuit starts");
    circuit_memory_read(c, rom, 0x7FFE, 2, values, known);
    expect(values[0] == 0x12 && values[1] == 0x34 && known[0] == 0xFF,
           "memory load: the loaded words survive circuit_start");
    expect_error(c, circuit_memory_load(c, rom, 0, TWO, 1), "circuit already started",
                 "memory load: refused after start");
    circuit_destroy(c);
}

static void check_ball_led(void) {
    /* Every part names a footprint; this one is a 2 x 2 grid holding A1 and B2. */
    static const char *const footprints =
        "{\"format\":\"schematical-footprints\",\"version\":1,\"footprints\":["
        "{\"id\":\"BALL-LED-2\",\"description\":\"d\",\"style\":\"grid\",\"pins\":2,"
        "\"rows\":2,\"columns\":2,\"pitch\":0.5,\"balls\":[\"A1\",\"B2\"],"
        "\"body\":{\"width\":1,\"height\":1},\"pad\":{\"width\":0.2,\"height\":0.2,\"shape\":\"round\"}}]}";
    static const char *const text =
        "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"led\",\"devices\":["
        "{\"id\":\"led-ball\",\"function\":\"LED\",\"description\":\"d\",\"drawer\":[],"
        "\"pinouts\":{\"bga\":[{\"ball\":\"A1\",\"name\":\"K\",\"role\":\"input\"},"
        "{\"ball\":\"B2\",\"name\":\"A\",\"role\":\"input\"}]},"
        "\"colors\":[\"Red\"],\"anode\":\"A\",\"cathode\":\"K\","
        "\"parts\":[{\"name\":\"LED-BALL-BGA\",\"package\":\"BGA\",\"pinout\":\"bga\","
        "\"footprint\":\"BALL-LED-2\"}]}]}";
    char *error = NULL;
    Circuit *c;
    size_t vcc, gnd, led, n0, n1;

    expect(load_footprints(library, footprints, "ball-footprints.json", &error) == 0,
           "a ball LED footprint loads");
    free(error);
    error = NULL;
    expect(load_parts(library, text, "ball.json", &error) == 0, "a ball-pinout LED loads");
    free(error);
    c = circuit_create(library);
    vcc = part(c, "5V");
    gnd = part(c, "GND");
    led = part(c, "LED-BALL-BGA");
    n0 = net(c);
    n1 = net(c);
    expect(circuit_connect(c, led, "C3", n0) == -1 && circuit_error(c) != NULL &&
               strstr(circuit_error(c), "has no pin C3") != NULL,
           "an unknown ball is rejected");
    expect(circuit_connect(c, vcc, "1", n0) == 0 && circuit_connect(c, gnd, "1", n1) == 0 &&
               circuit_connect(c, led, "B2", n0) == 0 && circuit_connect(c, led, "A1", n1) == 0,
           "a ball LED connects by ball name");
    circuit_start(c);
    circuit_advance(c, 1000000);
    expect(light(c, led) == 1, "a ball LED with VCC on B2 and GND on A1 is lit");
    circuit_destroy(c);
}

static void check_building_errors(void) {
    Circuit *c = circuit_create(library);
    size_t display = part(c, "LTC-4727JR");
    size_t n0 = net(c);
    size_t index;

    expect(circuit_connect(c, display, "10", n0) == -1 && circuit_error(c) != NULL &&
               strstr(circuit_error(c), "pin 10 of LTC-4727JR has no lead") != NULL,
           "connecting an absent pin is rejected");
    expect(circuit_connect(c, display, "99", n0) == -1, "a pin number past the end is rejected");
    expect(circuit_connect(c, display, "1", 5) == -1, "a missing net is rejected");
    expect(circuit_connect(c, display, NULL, n0) == -1 &&
               strstr(circuit_error(c), "no pin given") != NULL,
           "a NULL pin is rejected");
    expect(circuit_start(c) == 0, "the circuit starts");
    expect(circuit_add_part(c, "5V", &index) == -1 &&
               strstr(circuit_error(c), "circuit already started") != NULL,
           "adding a part after start is rejected");
    expect(circuit_add_net(c, &index) == -1, "adding a net after start is rejected");
    expect(circuit_connect(c, display, "1", n0) == -1, "connecting after start is rejected");
    circuit_destroy(c);
}

/*
 * A 1 MHz oscillator lighting an LED for 10 us: the clock net changes 21
 * times (Z -> 0 at t = 0, then every 500 ns) and the LED is on for 10 half
 * periods, 5 000 000 ps. The oscillator is a 1.8 V-option part (1.7-1.9 V),
 * so its supply is 1V8 (5 V would fail the voltage check).
 */
static void check_changes_and_light_time(void) {
    Circuit *c = circuit_create(library);
    size_t vcc = part(c, "1V8");
    size_t gnd = part(c, "GND");
    size_t osc = part(c, "LMK6CE001000DDLER");
    size_t led = part(c, "GENERIC-LED");
    size_t vcc_net = net(c);
    size_t gnd_net = net(c);
    size_t clock = net(c);
    uint64_t on_ps[2] = { 0, 0 };

    circuit_connect(c, vcc, "1", vcc_net);
    circuit_connect(c, gnd, "1", gnd_net);
    circuit_connect(c, osc, "4", vcc_net);
    circuit_connect(c, osc, "2", gnd_net);
    circuit_connect(c, osc, "3", clock);
    circuit_connect(c, led, "2", clock);
    circuit_connect(c, led, "1", gnd_net);
    expect(circuit_start(c) == 0 && circuit_advance(c, 10000000) == 1,
           "oscillator and LED run 10 us");
    expect(circuit_net_changes(c, clock) == 21, "1 MHz clock net changed 21 times in 10 us");
    expect(circuit_part_light_time(c, led, on_ps, 2) == 1, "LED has one light-time value");
    expect(on_ps[0] == 5000000, "LED on for 5 000 000 ps of 10 us");
    expect(circuit_part_light_time(c, vcc, on_ps, 2) == 0, "VCC has no lights");
    expect(circuit_net_changes(c, 99) == 0, "net changes for a bad index is 0");
    circuit_destroy(c);
}

/* 5V on the 0.8-2.7 V fixture part's VCC: refused at start, with the text
   and the problem; 3V3 on the 2-6 V fixture part starts and runs. */
static void check_voltage_start(void) {
    Circuit *c = circuit_create(library);
    size_t s = part(c, "5V");
    size_t auc = part(c, "VFX-AUC1G00");
    size_t n0 = net(c);
    VoltageProblem problem;

    circuit_connect(c, s, "1", n0);
    circuit_connect(c, auc, "5", n0);
    expect(circuit_start(c) == -1, "5V on a 0.8-2.7 V part's VCC is refused at start");
    expect(circuit_error(c) != NULL &&
               strcmp(circuit_error(c),
                      "voltage: 5 V on pin 5 (VCC) of VFX-AUC1G00; it needs 0.8-2.7 V") == 0,
           "the start refusal names the pin and the range");
    expect(circuit_voltage_problem(c, &problem) == 1 && problem.kind == VOLTAGE_POWER_RANGE &&
               problem.part == auc && problem.net == n0 && problem.mv == 5000,
           "circuit_voltage_problem returns the POWER_RANGE problem");
    expect(circuit_advance(c, 1000) == 0, "a refused circuit does not run");
    circuit_destroy(c);

    c = circuit_create(library);
    s = part(c, "3V3");
    {
        size_t hc = part(c, "VFX-HC1G00");
        size_t gnd = part(c, "GND");
        size_t n1;

        n0 = net(c);
        n1 = net(c);
        circuit_connect(c, s, "1", n0);
        circuit_connect(c, hc, "5", n0);
        circuit_connect(c, hc, "1", n0);
        circuit_connect(c, hc, "2", n0);
        circuit_connect(c, gnd, "1", n1);
        circuit_connect(c, hc, "3", n1);
        expect(circuit_start(c) == 0 && circuit_advance(c, 1000) == 1,
               "3V3 on a 2-6 V part starts and runs");
        expect(circuit_voltage_problem(c, &problem) == 0, "a running circuit has no voltage problem");
    }
    circuit_destroy(c);
}

/* A running circuit: a switch closes and joins a 3V3 net to a 5V net. */
static void check_voltage_switch(void) {
    Circuit *c = circuit_create(library);
    size_t low = part(c, "3V3");
    size_t high = part(c, "5V");
    size_t sw = part(c, "VFX-SW2");
    size_t a = net(c);
    size_t b = net(c);
    VoltageProblem problem;

    circuit_connect(c, low, "1", a);
    circuit_connect(c, high, "1", b);
    circuit_connect(c, sw, "1", a);   /* 1A */
    circuit_connect(c, sw, "4", b);   /* 1B */
    expect(circuit_start(c) == 0 && circuit_advance(c, 1000) == 1,
           "3V3 and 5V on two nets run while the switch is open");
    expect(circuit_set_switch(c, sw, 1, 1) == 0, "the contact closes");
    expect(circuit_advance(c, 1000) == 0, "closing 3V3 onto 5V stops the run");
    expect(circuit_error(c) != NULL && strncmp(circuit_error(c), "voltage:", 8) == 0 &&
               strcmp(circuit_error(c), "voltage: 3.3 V and 5 V supplies on one net (3V3, 5V)") == 0,
           "the stop is a voltage error");
    expect(circuit_voltage_problem(c, &problem) == 1 && problem.kind == VOLTAGE_SUPPLY_CLASH,
           "circuit_voltage_problem returns the CLASH");
    expect(circuit_advance(c, 1000) == 0, "advance stays stopped");
    circuit_destroy(c);

    /* Closed before start: the start check sees it. */
    c = circuit_create(library);
    low = part(c, "3V3");
    high = part(c, "5V");
    sw = part(c, "VFX-SW2");
    a = net(c);
    b = net(c);
    circuit_connect(c, low, "1", a);
    circuit_connect(c, high, "1", b);
    circuit_connect(c, sw, "1", a);
    circuit_connect(c, sw, "4", b);
    circuit_set_switch(c, sw, 1, 1);
    expect(circuit_start(c) == -1, "a contact closed before start is checked at start");
    circuit_set_switch(c, sw, 1, 0);
    expect(circuit_start(c) == 0 && circuit_voltage_problem(c, NULL) == 0,
           "opened again, the circuit starts");
    circuit_destroy(c);
}

/* ---- plan-24-G: an ADC read by a CPU ---------------------------------------- */

/* A W65C02S board (io_board's decoding: SRAM 0000-3FFF, VIA 6000, EEPROM
   8000-FFFF) with an ADS1115IDGSR on the VIA's port B: SDA = PB0, SCL = PB1,
   each pulled up, ADDR grounded (address 0x48). The program bit-bangs I2C
   (open drain by DDRB, ORB = 0), writes CONFIG = 0x85A3 (OS = 1 starts a
   single-shot conversion, MUX AIN0-AIN1, DR 250 SPS), polls CONFIG until OS
   reads 1, then reads CONVERSION. SRAM results: 0200 write ACKs ORed
   (0 = all acknowledged), 0201 the first poll's CONFIG MSB, 0202 polls that
   saw OS = 0, 0203 the last CONFIG MSB, 0204/0205 CONVERSION, 0206 read
   ACKs ORed, 0207 = AA when done. */
static const uint8_t ADC_PROGRAM[] = {
    /* RESET: */
    0x78,                /* 8000 SEI */
    0xD8,                /* 8001 CLD */
    0xA2, 0xFF,          /* 8002 LDX #$FF */
    0x9A,                /* 8004 TXS */
    0xA9, 0x00,          /* 8005 LDA #$00 */
    0x8D, 0x00, 0x60,    /* 8007 STA ORB */
    0x8D, 0x02, 0x60,    /* 800A STA DDRB */
    0x85, 0x13,          /* 800D STA $13 */
    0x85, 0x14,          /* 800F STA $14 */
    0x8D, 0x01, 0x02,    /* 8011 STA $0201 */
    0x20, 0xF3, 0x80,    /* 8014 JSR START */
    0xA9, 0x90,          /* 8017 LDA #$90 */
    0x85, 0x10,          /* 8019 STA $10 */
    0x20, 0x2D, 0x81,    /* 801B JSR SEND */
    0xA5, 0x12,          /* 801E LDA $12 */
    0x05, 0x13,          /* 8020 ORA $13 */
    0x85, 0x13,          /* 8022 STA $13 */
    0xA9, 0x01,          /* 8024 LDA #$01 */
    0x85, 0x10,          /* 8026 STA $10 */
    0x20, 0x2D, 0x81,    /* 8028 JSR SEND */
    0xA5, 0x12,          /* 802B LDA $12 */
    0x05, 0x13,          /* 802D ORA $13 */
    0x85, 0x13,          /* 802F STA $13 */
    0xA9, 0x85,          /* 8031 LDA #$85 */
    0x85, 0x10,          /* 8033 STA $10 */
    0x20, 0x2D, 0x81,    /* 8035 JSR SEND */
    0xA5, 0x12,          /* 8038 LDA $12 */
    0x05, 0x13,          /* 803A ORA $13 */
    0x85, 0x13,          /* 803C STA $13 */
    0xA9, 0xA3,          /* 803E LDA #$A3 */
    0x85, 0x10,          /* 8040 STA $10 */
    0x20, 0x2D, 0x81,    /* 8042 JSR SEND */
    0xA5, 0x12,          /* 8045 LDA $12 */
    0x05, 0x13,          /* 8047 ORA $13 */
    0x85, 0x13,          /* 8049 STA $13 */
    0x20, 0x14, 0x81,    /* 804B JSR STOP */
    0xA5, 0x13,          /* 804E LDA $13 */
    0x8D, 0x00, 0x02,    /* 8050 STA $0200 */
    0xA9, 0x00,          /* 8053 LDA #$00 */
    0x85, 0x13,          /* 8055 STA $13 */
    /* POLL: */
    0x20, 0xF3, 0x80,    /* 8057 JSR START */
    0xA9, 0x91,          /* 805A LDA #$91 */
    0x85, 0x10,          /* 805C STA $10 */
    0x20, 0x2D, 0x81,    /* 805E JSR SEND */
    0xA5, 0x12,          /* 8061 LDA $12 */
    0x05, 0x13,          /* 8063 ORA $13 */
    0x85, 0x13,          /* 8065 STA $13 */
    0xA9, 0x00,          /* 8067 LDA #$00 */
    0x85, 0x12,          /* 8069 STA $12 */
    0x20, 0x79, 0x81,    /* 806B JSR RECV */
    0xA5, 0x10,          /* 806E LDA $10 */
    0x8D, 0x03, 0x02,    /* 8070 STA $0203 */
    0xA9, 0x01,          /* 8073 LDA #$01 */
    0x85, 0x12,          /* 8075 STA $12 */
    0x20, 0x79, 0x81,    /* 8077 JSR RECV */
    0xA5, 0x10,          /* 807A LDA $10 */
    0x8D, 0x08, 0x02,    /* 807C STA $0208 */
    0x20, 0x14, 0x81,    /* 807F JSR STOP */
    0xA5, 0x14,          /* 8082 LDA $14 */
    0xD0, 0x06,          /* 8084 BNE NOTFIRST */
    0xAD, 0x03, 0x02,    /* 8086 LDA $0203 */
    0x8D, 0x01, 0x02,    /* 8089 STA $0201 */
    /* NOTFIRST: */
    0xAD, 0x03, 0x02,    /* 808C LDA $0203 */
    0x30, 0x05,          /* 808F BMI DONE */
    0xE6, 0x14,          /* 8091 INC $14 */
    0x4C, 0x57, 0x80,    /* 8093 JMP POLL */
    /* DONE: */
    0xA5, 0x14,          /* 8096 LDA $14 */
    0x8D, 0x02, 0x02,    /* 8098 STA $0202 */
    0x20, 0xF3, 0x80,    /* 809B JSR START */
    0xA9, 0x90,          /* 809E LDA #$90 */
    0x85, 0x10,          /* 80A0 STA $10 */
    0x20, 0x2D, 0x81,    /* 80A2 JSR SEND */
    0xA5, 0x12,          /* 80A5 LDA $12 */
    0x05, 0x13,          /* 80A7 ORA $13 */
    0x85, 0x13,          /* 80A9 STA $13 */
    0xA9, 0x00,          /* 80AB LDA #$00 */
    0x85, 0x10,          /* 80AD STA $10 */
    0x20, 0x2D, 0x81,    /* 80AF JSR SEND */
    0xA5, 0x12,          /* 80B2 LDA $12 */
    0x05, 0x13,          /* 80B4 ORA $13 */
    0x85, 0x13,          /* 80B6 STA $13 */
    0x20, 0x14, 0x81,    /* 80B8 JSR STOP */
    0x20, 0xF3, 0x80,    /* 80BB JSR START */
    0xA9, 0x91,          /* 80BE LDA #$91 */
    0x85, 0x10,          /* 80C0 STA $10 */
    0x20, 0x2D, 0x81,    /* 80C2 JSR SEND */
    0xA5, 0x12,          /* 80C5 LDA $12 */
    0x05, 0x13,          /* 80C7 ORA $13 */
    0x85, 0x13,          /* 80C9 STA $13 */
    0xA9, 0x00,          /* 80CB LDA #$00 */
    0x85, 0x12,          /* 80CD STA $12 */
    0x20, 0x79, 0x81,    /* 80CF JSR RECV */
    0xA5, 0x10,          /* 80D2 LDA $10 */
    0x8D, 0x04, 0x02,    /* 80D4 STA $0204 */
    0xA9, 0x01,          /* 80D7 LDA #$01 */
    0x85, 0x12,          /* 80D9 STA $12 */
    0x20, 0x79, 0x81,    /* 80DB JSR RECV */
    0xA5, 0x10,          /* 80DE LDA $10 */
    0x8D, 0x05, 0x02,    /* 80E0 STA $0205 */
    0x20, 0x14, 0x81,    /* 80E3 JSR STOP */
    0xA5, 0x13,          /* 80E6 LDA $13 */
    0x8D, 0x06, 0x02,    /* 80E8 STA $0206 */
    0xA9, 0xAA,          /* 80EB LDA #$AA */
    0x8D, 0x07, 0x02,    /* 80ED STA $0207 */
    /* HALT: */
    0x4C, 0xF0, 0x80,    /* 80F0 JMP HALT */
    /* START: */
    0xAD, 0x02, 0x60,    /* 80F3 LDA DDRB */
    0x29, 0xFE,          /* 80F6 AND #$FE */
    0x8D, 0x02, 0x60,    /* 80F8 STA DDRB */
    0xAD, 0x02, 0x60,    /* 80FB LDA DDRB */
    0x29, 0xFD,          /* 80FE AND #$FD */
    0x8D, 0x02, 0x60,    /* 8100 STA DDRB */
    0xAD, 0x02, 0x60,    /* 8103 LDA DDRB */
    0x09, 0x01,          /* 8106 ORA #$01 */
    0x8D, 0x02, 0x60,    /* 8108 STA DDRB */
    0xAD, 0x02, 0x60,    /* 810B LDA DDRB */
    0x09, 0x02,          /* 810E ORA #$02 */
    0x8D, 0x02, 0x60,    /* 8110 STA DDRB */
    0x60,                /* 8113 RTS */
    /* STOP: */
    0xAD, 0x02, 0x60,    /* 8114 LDA DDRB */
    0x09, 0x01,          /* 8117 ORA #$01 */
    0x8D, 0x02, 0x60,    /* 8119 STA DDRB */
    0xAD, 0x02, 0x60,    /* 811C LDA DDRB */
    0x29, 0xFD,          /* 811F AND #$FD */
    0x8D, 0x02, 0x60,    /* 8121 STA DDRB */
    0xAD, 0x02, 0x60,    /* 8124 LDA DDRB */
    0x29, 0xFE,          /* 8127 AND #$FE */
    0x8D, 0x02, 0x60,    /* 8129 STA DDRB */
    0x60,                /* 812C RTS */
    /* SEND: */
    0xA2, 0x08,          /* 812D LDX #$08 */
    /* SBIT: */
    0x06, 0x10,          /* 812F ASL $10 */
    0xB0, 0x0B,          /* 8131 BCS SONE */
    0xAD, 0x02, 0x60,    /* 8133 LDA DDRB */
    0x09, 0x01,          /* 8136 ORA #$01 */
    0x8D, 0x02, 0x60,    /* 8138 STA DDRB */
    0x4C, 0x46, 0x81,    /* 813B JMP SCLK */
    /* SONE: */
    0xAD, 0x02, 0x60,    /* 813E LDA DDRB */
    0x29, 0xFE,          /* 8141 AND #$FE */
    0x8D, 0x02, 0x60,    /* 8143 STA DDRB */
    /* SCLK: */
    0xAD, 0x02, 0x60,    /* 8146 LDA DDRB */
    0x29, 0xFD,          /* 8149 AND #$FD */
    0x8D, 0x02, 0x60,    /* 814B STA DDRB */
    0xAD, 0x02, 0x60,    /* 814E LDA DDRB */
    0x09, 0x02,          /* 8151 ORA #$02 */
    0x8D, 0x02, 0x60,    /* 8153 STA DDRB */
    0xCA,                /* 8156 DEX */
    0xD0, 0xD6,          /* 8157 BNE SBIT */
    0xAD, 0x02, 0x60,    /* 8159 LDA DDRB */
    0x29, 0xFE,          /* 815C AND #$FE */
    0x8D, 0x02, 0x60,    /* 815E STA DDRB */
    0xAD, 0x02, 0x60,    /* 8161 LDA DDRB */
    0x29, 0xFD,          /* 8164 AND #$FD */
    0x8D, 0x02, 0x60,    /* 8166 STA DDRB */
    0xAD, 0x00, 0x60,    /* 8169 LDA ORB */
    0x29, 0x01,          /* 816C AND #$01 */
    0x85, 0x12,          /* 816E STA $12 */
    0xAD, 0x02, 0x60,    /* 8170 LDA DDRB */
    0x09, 0x02,          /* 8173 ORA #$02 */
    0x8D, 0x02, 0x60,    /* 8175 STA DDRB */
    0x60,                /* 8178 RTS */
    /* RECV: */
    0xAD, 0x02, 0x60,    /* 8179 LDA DDRB */
    0x29, 0xFE,          /* 817C AND #$FE */
    0x8D, 0x02, 0x60,    /* 817E STA DDRB */
    0xA9, 0x00,          /* 8181 LDA #$00 */
    0x85, 0x10,          /* 8183 STA $10 */
    0xA2, 0x08,          /* 8185 LDX #$08 */
    /* RBIT: */
    0xAD, 0x02, 0x60,    /* 8187 LDA DDRB */
    0x29, 0xFD,          /* 818A AND #$FD */
    0x8D, 0x02, 0x60,    /* 818C STA DDRB */
    0xAD, 0x00, 0x60,    /* 818F LDA ORB */
    0x4A,                /* 8192 LSR */
    0x26, 0x10,          /* 8193 ROL $10 */
    0xAD, 0x02, 0x60,    /* 8195 LDA DDRB */
    0x09, 0x02,          /* 8198 ORA #$02 */
    0x8D, 0x02, 0x60,    /* 819A STA DDRB */
    0xCA,                /* 819D DEX */
    0xD0, 0xE7,          /* 819E BNE RBIT */
    0xA5, 0x12,          /* 81A0 LDA $12 */
    0xD0, 0x0B,          /* 81A2 BNE RNACK */
    0xAD, 0x02, 0x60,    /* 81A4 LDA DDRB */
    0x09, 0x01,          /* 81A7 ORA #$01 */
    0x8D, 0x02, 0x60,    /* 81A9 STA DDRB */
    0x4C, 0xB7, 0x81,    /* 81AC JMP RCLK */
    /* RNACK: */
    0xAD, 0x02, 0x60,    /* 81AF LDA DDRB */
    0x29, 0xFE,          /* 81B2 AND #$FE */
    0x8D, 0x02, 0x60,    /* 81B4 STA DDRB */
    /* RCLK: */
    0xAD, 0x02, 0x60,    /* 81B7 LDA DDRB */
    0x29, 0xFD,          /* 81BA AND #$FD */
    0x8D, 0x02, 0x60,    /* 81BC STA DDRB */
    0xAD, 0x02, 0x60,    /* 81BF LDA DDRB */
    0x09, 0x02,          /* 81C2 ORA #$02 */
    0x8D, 0x02, 0x60,    /* 81C4 STA DDRB */
    0xAD, 0x02, 0x60,    /* 81C7 LDA DDRB */
    0x29, 0xFE,          /* 81CA AND #$FE */
    0x8D, 0x02, 0x60,    /* 81CC STA DDRB */
    0x60,                /* 81CF RTS */
};

typedef struct {
    Circuit *c;
    size_t v, g;
    size_t nets[64];
    char names[64][8];
    size_t net_count;
} Board;

static size_t board_net(Board *b, const char *name) {
    size_t i;

    for (i = 0; i < b->net_count; i++) {
        if (strcmp(b->names[i], name) == 0) {
            return b->nets[i];
        }
    }
    snprintf(b->names[b->net_count], sizeof b->names[0], "%s", name);
    b->nets[b->net_count] = net(b->c);
    return b->nets[b->net_count++];
}

/* A chip with its power and ground pins on the supplies. */
static size_t board_chip(Board *b, const char *name) {
    const Part *p = part_library_find(library, name);
    size_t index = part(b->c, name);
    size_t i;

    for (i = 0; p != NULL && i < p->pin_count; i++) {
        if (p->pins[i].role == PIN_POWER || p->pins[i].role == PIN_GROUND) {
            circuit_connect(b->c, index, p->pins[i].designator,
                            p->pins[i].role == PIN_POWER ? b->v : b->g);
        }
    }
    return index;
}

static void board_wire(Board *b, size_t chip, const char *part_name, const char *pin,
                       const char *net_name) {
    size_t n = strcmp(net_name, "ONE") == 0 ? b->v
             : strcmp(net_name, "ZERO") == 0 ? b->g
                                             : board_net(b, net_name);

    if (circuit_connect(b->c, chip, pin_named(part_name, pin), n) != 0) {
        printf("  connect %s %s: %s\n", part_name, pin, circuit_error(b->c));
    }
}

static void board_bus(Board *b, size_t chip, const char *part_name, const char *prefix,
                      const char *net_prefix, int n) {
    char pin[16];
    char name[8];
    int i;

    for (i = 0; i < n; i++) {
        snprintf(pin, sizeof pin, "%s%d", prefix, i);
        snprintf(name, sizeof name, "%s%d", net_prefix, i);
        board_wire(b, chip, part_name, pin, name);
    }
}

static void board_pullup(Board *b, const char *net_name) {
    circuit_connect(b->c, part(b->c, "PULLUP-5V"), "1", board_net(b, net_name));
}

static void check_adc_from_cpu(void) {
    static const char *const GATES[10][3] = {
        {"A15", "A15", "G1"}, {"A14", "A14", "G2"}, {"G1", "G2", "G3"}, {"G1", "A14", "G4"},
        {"A13", "A13", "G5"}, {"A12", "A12", "G6"}, {"G5", "G6", "G7"}, {"G7", "G7", "G8"},
        {"RW", "RW", "G9"}, {"CLK", "G9", "G10"},
    };
    static uint32_t words[32768];
    static Board board;
    Board *b = &board;
    size_t cpu, rom, ram, via, adc, osc, sw, nand[3];
    uint32_t values[8];
    uint32_t known[8];
    CpuState state;
    size_t i;
    int step;

    memset(b, 0, sizeof *b);
    b->c = circuit_create(library);
    circuit_connect(b->c, part(b->c, "5V"), "1", b->v = net(b->c));
    circuit_connect(b->c, part(b->c, "GND"), "1", b->g = net(b->c));
    cpu = board_chip(b, "W65C02S6TPG-14");
    rom = board_chip(b, "AT28C256-15PU");
    ram = board_chip(b, "AS6C62256-55PCN");
    via = board_chip(b, "W65C22S6TPG-14");
    adc = board_chip(b, "ADS1115IDGSR");
    osc = board_chip(b, "ECS-100A-010");
    for (i = 0; i < 3; i++) {
        nand[i] = board_chip(b, "SN74HC00N");
    }
    sw = part(b->c, "TE-GDS0604");

    /* The clock (1 MHz) and the reset: a switch to GND on a pulled-up RES. */
    board_wire(b, osc, "ECS-100A-010", "OUTPUT", "CLK");
    board_pullup(b, "RES");
    circuit_connect(b->c, sw, pin_named("TE-GDS0604", "1A"), board_net(b, "RES"));
    circuit_connect(b->c, sw, pin_named("TE-GDS0604", "1B"), b->g);

    board_bus(b, cpu, "W65C02S6TPG-14", "A", "A", 16);
    board_bus(b, cpu, "W65C02S6TPG-14", "D", "D", 8);
    board_wire(b, cpu, "W65C02S6TPG-14", "PHI2", "CLK");
    board_wire(b, cpu, "W65C02S6TPG-14", "RESB", "RES");
    board_wire(b, cpu, "W65C02S6TPG-14", "RWB", "RW");
    board_wire(b, cpu, "W65C02S6TPG-14", "IRQB", "IRQ");
    board_wire(b, cpu, "W65C02S6TPG-14", "NMIB", "ONE");
    board_wire(b, cpu, "W65C02S6TPG-14", "RDY", "ONE");
    board_wire(b, cpu, "W65C02S6TPG-14", "SOB", "ONE");
    board_wire(b, cpu, "W65C02S6TPG-14", "BE", "ONE");
    for (i = 0; i < 10; i++) {
        char pin[4];
        const char *const name = "SN74HC00N";

        snprintf(pin, sizeof pin, "%zuA", i % 4 + 1);
        board_wire(b, nand[i / 4], name, pin, GATES[i][0]);
        snprintf(pin, sizeof pin, "%zuB", i % 4 + 1);
        board_wire(b, nand[i / 4], name, pin, GATES[i][1]);
        snprintf(pin, sizeof pin, "%zuY", i % 4 + 1);
        board_wire(b, nand[i / 4], name, pin, GATES[i][2]);
    }
    board_bus(b, rom, "AT28C256-15PU", "A", "A", 15);
    board_bus(b, rom, "AT28C256-15PU", "I/O", "D", 8);
    board_wire(b, rom, "AT28C256-15PU", "CE#", "G1");
    board_wire(b, rom, "AT28C256-15PU", "OE#", "G9");
    board_wire(b, rom, "AT28C256-15PU", "WE#", "ONE");
    board_bus(b, ram, "AS6C62256-55PCN", "A", "A", 15);
    board_bus(b, ram, "AS6C62256-55PCN", "DQ", "D", 8);
    board_wire(b, ram, "AS6C62256-55PCN", "CE#", "G3");
    board_wire(b, ram, "AS6C62256-55PCN", "OE#", "G9");
    board_wire(b, ram, "AS6C62256-55PCN", "WE#", "G10");
    board_bus(b, via, "W65C22S6TPG-14", "D", "D", 8);
    board_bus(b, via, "W65C22S6TPG-14", "RS", "A", 4);
    board_wire(b, via, "W65C22S6TPG-14", "PHI2", "CLK");
    board_wire(b, via, "W65C22S6TPG-14", "RESB", "RES");
    board_wire(b, via, "W65C22S6TPG-14", "RWB", "RW");
    board_wire(b, via, "W65C22S6TPG-14", "CS1", "A13");
    board_wire(b, via, "W65C22S6TPG-14", "CS2B", "G4");
    board_wire(b, via, "W65C22S6TPG-14", "IRQB", "IRQ");
    board_wire(b, via, "W65C22S6TPG-14", "PB0", "SDA");
    board_wire(b, via, "W65C22S6TPG-14", "PB1", "SCL");
    /* PB2-PB7 are read with PB0: pulled low so the byte is known (a pull,
       not GND: before reset the VIA drives its ports X). */
    for (i = 2; i < 8; i++) {
        char pin[4];

        snprintf(pin, sizeof pin, "PB%zu", i);
        board_wire(b, via, "W65C22S6TPG-14", pin, "PBX");
    }
    circuit_connect(b->c, part(b->c, "PULLDOWN"), "1", board_net(b, "PBX"));

    /* The ADC on the bus, with the pull-ups. */
    board_wire(b, adc, "ADS1115IDGSR", "SDA", "SDA");
    board_wire(b, adc, "ADS1115IDGSR", "SCL", "SCL");
    board_wire(b, adc, "ADS1115IDGSR", "ADDR", "ZERO");
    board_wire(b, adc, "ADS1115IDGSR", "ALERT/RDY", "ALERT");
    board_pullup(b, "SDA");
    board_pullup(b, "SCL");
    board_pullup(b, "ALERT");

    for (i = 0; i < 32768; i++) {
        words[i] = 0xFF;
    }
    for (i = 0; i < sizeof ADC_PROGRAM; i++) {
        words[i] = ADC_PROGRAM[i];
    }
    words[0x7FFC] = 0x00; /* RESET vector: 8000 */
    words[0x7FFD] = 0x80;
    expect(circuit_memory_load(b->c, rom, 0, words, 32768) == 0, "adc board: the program loads");
    circuit_set_switch(b->c, sw, 1, 1);
    expect(circuit_start(b->c) == 0, "adc board: the circuit starts");
    circuit_advance(b->c, 10000000); /* RES low for 10 cycles */
    circuit_set_switch(b->c, sw, 1, 0);
    for (step = 0; step < 40; step++) {
        if (!circuit_advance(b->c, 500000000)) {
            printf("  stopped: %s\n", circuit_error(b->c) ? circuit_error(b->c) : "?");
            break;
        }
        if (circuit_memory_read(b->c, ram, 0x0207, 1, values, known) == 1 && known[0] == 0xFF &&
            values[0] == 0xAA) {
            break;
        }
    }
    expect(circuit_cpu_state(b->c, cpu, &state) == 0 && state.status == CPU_RUNNING,
           "adc board: the CPU runs");
    printf("  %llu cycles, at %04X %s\n", (unsigned long long)state.cycles,
           (unsigned)state.instruction_address, state.instruction_text);
    expect(circuit_memory_read(b->c, ram, 0x0200, 8, values, known) == 8,
           "adc board: the results read back");
    for (i = 0; i < 8; i++) {
        if (known[i] != 0xFF) {
            values[i] = 0x100;
        }
    }
    printf("  results: %02X %02X %02X %02X %02X %02X %02X %02X\n", values[0], values[1],
           values[2], values[3], values[4], values[5], values[6], values[7]);
    expect(values[7] == 0xAA, "adc board: the program finishes");
    expect(values[0] == 0x00 && values[6] == 0x00, "adc board: the ADS1115 acknowledges every byte");
    expect(values[1] == 0x05, "adc board: the first poll reads CONFIG 05xx (OS = 0, converting)");
    expect(values[2] >= 1, "adc board: the program polls while OS = 0");
    expect(values[3] == 0x85, "adc board: OS reads 1 when the conversion is done (CONFIG 85xx)");
    expect(values[4] == 0x00 && values[5] == 0x00, "adc board: CONVERSION reads 0x0000 (0 V)");

    /* plan-24-I: the register window's snapshot of the ADC. */
    {
        ConverterRegister regs[8];
        int n = circuit_converter_registers(b->c, adc, regs, 8);

        expect(n == 4 && strcmp(regs[0].name, "CONVERSION") == 0 && regs[0].address == 0 &&
                   regs[0].width == 16 && regs[0].value == 0,
               "registers: 4, CONVERSION at 0, 16 bits, 0x0000");
        expect(n == 4 && strcmp(regs[1].name, "CONFIG") == 0 && regs[1].address == 1 &&
                   regs[1].value == 0x85A3,
               "registers: CONFIG holds the written 0x85A3 (OS = 1: done)");
        if (n == 4) {
            printf("  registers: %s=%04X %s=%04X %s=%04X %s=%04X\n", regs[0].name, regs[0].value,
                   regs[1].name, regs[1].value, regs[2].name, regs[2].value, regs[3].name,
                   regs[3].value);
        }
        expect(n == 4 && strcmp(regs[2].name, "LO_THRESH") == 0 && regs[2].value == 0x8000 &&
                   regs[3].value == 0x7FFF,
               "registers: the thresholds keep their reset values");
        expect(circuit_converter_registers(b->c, adc, NULL, 0) == 4 &&
                   circuit_converter_registers(b->c, adc, regs, 1) == 4,
               "registers: the count with no room, or room for one");
        expect_error(b->c, circuit_converter_registers(b->c, cpu, regs, 8), "part 2 has no registers",
                     "registers: a CPU has none");
        expect_error(b->c, circuit_converter_registers(b->c, 999, regs, 8), "no part 999",
                     "registers: no part 999");
        expect_error(b->c, circuit_rtc_time(b->c, adc, &(RtcTime){0}), "part 6 is not an RTC",
                     "rtc time: an ADC is not an RTC");
    }
    circuit_destroy(b->c);
}

/* plan-24-I: an RTC's clock follows simulated time from 2000-01-01
   00:00:00 (a Saturday); its registers show the counters in BCD. */
static void check_rtc_time(void) {
    Board board;
    Board *b = &board;
    size_t rtc;
    RtcTime t;
    ConverterRegister regs[16];
    int n;

    memset(b, 0, sizeof *b);
    b->c = circuit_create(library);
    circuit_connect(b->c, part(b->c, "3V3"), "1", b->v = net(b->c));
    circuit_connect(b->c, part(b->c, "GND"), "1", b->g = net(b->c));
    rtc = board_chip(b, "BQ32000DR");
    board_wire(b, rtc, "BQ32000DR", "SDA", "SDA");
    board_wire(b, rtc, "BQ32000DR", "SCL", "SCL");
    board_wire(b, rtc, "BQ32000DR", "IRQ", "IRQ");
    circuit_connect(b->c, part(b->c, "PULLUP-3V3"), "1", board_net(b, "SDA"));
    circuit_connect(b->c, part(b->c, "PULLUP-3V3"), "1", board_net(b, "SCL"));
    circuit_connect(b->c, part(b->c, "PULLUP-3V3"), "1", board_net(b, "IRQ"));
    expect_error(b->c, circuit_rtc_time(b->c, rtc, &t), "the circuit is not running",
                 "rtc time: refused before start");
    expect(circuit_start(b->c) == 0, "rtc: the circuit starts");
    expect(circuit_rtc_time(b->c, rtc, &t) == 0 && t.year == 2000 && t.month == 1 && t.day == 1 &&
               t.weekday == 6 && t.hour == 0 && t.minute == 0 && t.second == 0 && t.running,
           "rtc time: 2000-01-01 00:00:00, a Saturday, running");
    circuit_advance(b->c, 2500000000000ull);
    expect(circuit_rtc_time(b->c, rtc, &t) == 0 && t.second == 2 && t.minute == 0,
           "rtc time: 2 s later at 2.5 s");
    circuit_advance(b->c, 60000000000000ull);
    expect(circuit_rtc_time(b->c, rtc, &t) == 0 && t.second == 2 && t.minute == 1,
           "rtc time: a minute later 00:01:02");
    printf("  rtc %04d-%02d-%02d %02d:%02d:%02d weekday %d\n", t.year, t.month, t.day, t.hour,
           t.minute, t.second, t.weekday);
    n = circuit_converter_registers(b->c, rtc, regs, 16);
    expect(n >= 2 && regs[0].address == 0 && (regs[0].value & 0x7F) == 0x02 && regs[1].address == 1 &&
               (regs[1].value & 0x7F) == 0x01,
           "rtc registers: SECONDS 02, MINUTES 01 (BCD)");
    if (n >= 2) {
        printf("  rtc registers: %d, %s=%02X %s=%02X\n", n, regs[0].name, regs[0].value, regs[1].name,
               regs[1].value);
    }
    circuit_destroy(b->c);
}

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0 || load_voltage_fixture(library) != 0 ||
        load_serial_fixture(library) != 0 || load_serial_fixture_classic(library) != 0) {
        return 1;
    }
    check_lit_led();
    check_header_passes_nets();
    check_dark_led();
    check_short();
    check_pulls();
    check_serial_terminal();
    check_serial_sync_8251();
    check_unknown_part();
    check_oscillator_blinks();
    check_switch();
    check_memory();
    check_memory_load();
    check_building_errors();
    check_ball_led();
    check_changes_and_light_time();
    check_voltage_start();
    check_voltage_switch();
    check_adc_from_cpu();
    check_rtc_time();
    part_library_destroy(library);
    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
