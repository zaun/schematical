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
    size_t led = part(c, "LED-RED-0603");
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

/* The anode is left unconnected, so it reads Z and the LED stays dark. */
static void check_dark_led(void) {
    Circuit *c = circuit_create(library);
    size_t vcc = part(c, "5V");
    size_t gnd = part(c, "GND");
    size_t led = part(c, "LED-GREEN-0603");
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
    size_t led = part(c, "LED-RED-0603");
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
        "\"color\":\"Red\",\"anode\":\"A\",\"cathode\":\"K\","
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
    size_t led = part(c, "LED-RED-0603");
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

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0 || load_voltage_fixture(library) != 0 ||
        load_serial_fixture(library) != 0) {
        return 1;
    }
    check_lit_led();
    check_dark_led();
    check_short();
    check_pulls();
    check_serial_terminal();
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
    part_library_destroy(library);
    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
