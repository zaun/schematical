/* circuit_bench: shared by the programs split from circuit_check.c (plan-68-H). */
#include "circuit_bench.h"


int failures;
PartLibrary *library;

void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

size_t part(Circuit *c, const char *name) {
    size_t index = (size_t)-1;

    if (circuit_add_part(c, name, &index) != 0) {
        printf("  add %s: %s\n", name, circuit_error(c));
    }
    return index;
}

size_t net(Circuit *c) {
    size_t index = (size_t)-1;

    circuit_add_net(c, &index);
    return index;
}

int light(Circuit *c, size_t index) {
    unsigned char on = 0;

    return circuit_part_lights(c, index, &on, 1) == 1 ? on : -1;
}

/* VCC on the anode (pin 2), GND on the cathode (pin 1): lit after one step. */
void check_lit_led(void) {
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
void check_header_passes_nets(void) {
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
void check_dark_led(void) {
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
void check_short(void) {
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
void check_pulls(void) {
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
const char *pin_named(const char *part_name, const char *name) {
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

/* A ball-pinout LED from an inline part file: "B2" (anode) on VCC, "A1" on GND lights it. */
/* Error text matches exactly (plan-04-B section 4.1). */
void expect_error(Circuit *c, int result, const char *message, const char *what) {
    const char *error = circuit_error(c);
    int ok = result == -1 && error != NULL && strcmp(error, message) == 0;

    if (!ok) {
        printf("  got %d \"%s\", wanted \"%s\"\n", result, error != NULL ? error : "(none)", message);
    }
    expect(ok, what);
}
