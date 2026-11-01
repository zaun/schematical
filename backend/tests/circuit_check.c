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
    size_t vcc = part(c, "VCC");
    size_t gnd = part(c, "GND");
    size_t led = part(c, "LED-RED-0603");
    size_t n0 = net(c);
    size_t n1 = net(c);

    circuit_connect(c, vcc, 1, n0);
    circuit_connect(c, gnd, 1, n1);
    circuit_connect(c, led, 2, n0);
    circuit_connect(c, led, 1, n1);
    expect(circuit_start(c) == 0 && circuit_advance(c, 1000) == 1, "LED circuit runs");
    expect(light(c, led) == 1, "LED with anode high and cathode low is lit");
    circuit_destroy(c);
}

/* The anode is left unconnected, so it reads Z and the LED stays dark. */
static void check_dark_led(void) {
    Circuit *c = circuit_create(library);
    size_t vcc = part(c, "VCC");
    size_t gnd = part(c, "GND");
    size_t led = part(c, "LED-GREEN-0603");
    size_t n0 = net(c);
    size_t n1 = net(c);

    circuit_connect(c, vcc, 1, n0);
    circuit_connect(c, gnd, 1, n1);
    circuit_connect(c, led, 1, n1);
    circuit_start(c);
    circuit_advance(c, 1000);
    expect(light(c, led) == 0, "LED with an unconnected anode is dark");
    circuit_destroy(c);
}

/* VCC and GND both drive net 0. */
static void check_short(void) {
    Circuit *c = circuit_create(library);
    size_t vcc = part(c, "VCC");
    size_t gnd = part(c, "GND");
    size_t n0 = net(c);
    size_t shorted = 99;

    circuit_connect(c, vcc, 1, n0);
    circuit_connect(c, gnd, 1, n0);
    circuit_start(c);
    expect(circuit_advance(c, 1000) == 0, "a short stops the run");
    expect(circuit_error(c) != NULL && strncmp(circuit_error(c), "smoke:", 6) == 0,
           "a short is a smoke error");
    expect(circuit_shorted_net(c, &shorted) == 1 && shorted == 0, "the shorted net is reported");
    expect(circuit_advance(c, 1000) == 0, "advance stays stopped");
    circuit_destroy(c);
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
    size_t led = part(c, "LED-RED-0603");
    size_t n0 = net(c);
    size_t n1 = net(c);
    unsigned out = 0;
    size_t i;
    int last = -1;
    int transitions = 0;

    for (i = 0; osc_part != NULL && i < osc_part->pin_count; i++) {
        if (osc_part->pins[i].role == PIN_OUTPUT) {
            out = osc_part->pins[i].number;
            break;
        }
    }
    circuit_connect(c, osc, out, n0);
    circuit_connect(c, gnd, 1, n1);
    circuit_connect(c, led, 2, n0);
    circuit_connect(c, led, 1, n1);
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
    size_t vcc = part(c, "VCC");
    size_t sw = part(c, "TE-GDS0404");
    size_t led = part(c, "LED-RED-0603");
    size_t gnd = part(c, "GND");
    size_t n0 = net(c);
    size_t n1 = net(c);
    size_t n2 = net(c);
    Circuit *small;

    circuit_connect(c, vcc, 1, n0);
    circuit_connect(c, sw, 2, n0);
    circuit_connect(c, sw, 7, n1);
    circuit_connect(c, led, 2, n1);
    circuit_connect(c, led, 1, n2);
    circuit_connect(c, gnd, 1, n2);
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
    size_t vcc = part(c, "VCC");
    size_t gnd = part(c, "GND");
    size_t ram = part(c, "AS6C62256-55PCN");
    size_t n0 = net(c);
    size_t n1 = net(c);
    uint32_t values[4] = {1, 1, 1, 1};
    uint32_t known[4] = {1, 1, 1, 1};
    uint32_t words = 0;
    unsigned bits = 0;

    circuit_connect(c, vcc, 1, n0);
    circuit_connect(c, gnd, 1, n1);
    circuit_connect(c, ram, 20, n0);
    circuit_connect(c, ram, 27, n0);
    circuit_connect(c, ram, 14, n1);
    circuit_connect(c, ram, 28, n0);
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

static void check_building_errors(void) {
    Circuit *c = circuit_create(library);
    size_t display = part(c, "LTC-4727JR");
    size_t n0 = net(c);
    size_t index;

    expect(circuit_connect(c, display, 10, n0) == -1 && circuit_error(c) != NULL &&
               strstr(circuit_error(c), "pin 10 of LTC-4727JR has no lead") != NULL,
           "connecting an absent pin is rejected");
    expect(circuit_connect(c, display, 99, n0) == -1, "a pin number past the end is rejected");
    expect(circuit_connect(c, display, 1, 5) == -1, "a missing net is rejected");
    expect(circuit_start(c) == 0, "the circuit starts");
    expect(circuit_add_part(c, "VCC", &index) == -1 &&
               strstr(circuit_error(c), "circuit already started") != NULL,
           "adding a part after start is rejected");
    expect(circuit_add_net(c, &index) == -1, "adding a net after start is rejected");
    expect(circuit_connect(c, display, 1, n0) == -1, "connecting after start is rejected");
    circuit_destroy(c);
}

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        return 1;
    }
    check_lit_led();
    check_dark_led();
    check_short();
    check_unknown_part();
    check_oscillator_blinks();
    check_switch();
    check_memory();
    check_building_errors();
    part_library_destroy(library);
    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
