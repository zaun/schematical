/*
 * capacitor_test (plan-33-E): the capacitor kind. The fixture device file
 * (support/capacitor_fixture.h: one tantalum and one ceramic device) loads
 * next to the real part files; the PartCapacitor facts read back; a
 * capacitor between two driven nets drives nothing, so after 1 us both nets
 * still carry their drivers' values.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "capacitor_fixture.h"
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

static int same(const char *a, const char *b) {
    return a != NULL && b != NULL && strcmp(a, b) == 0;
}

static void fact_cases(void) {
    const Part *ta = part_library_find(library, "TESTCAP");
    const Part *c = part_library_find(library, "TESTCAP-C");
    const Part *n = part_library_find(library, "TESTCAP-N");

    expect(ta != NULL && ta->kind == PART_CAPACITOR && ta->capacitor != NULL &&
               ta->vcc == NULL && ta->pin_count == 2,
           "TESTCAP loads as a capacitor with 2 pins and no vcc");
    expect(ta != NULL && ta->capacitor != NULL && same(ta->capacitor->type, "tantalum") &&
               ta->capacitor->polarized == 1 && ta->capacitor->capacitance_ff == 10000000000ULL &&
               ta->capacitor->rated_mv == 6300 && ta->capacitor->dielectric == NULL &&
               same(ta->capacitor->tolerance, "\xc2\xb1" "10%"),
           "TESTCAP: tantalum, polarized, 1e-5 F = 10000000000 fF, 6.3 V, no dielectric, +-10%");
    expect(ta != NULL && same(ta->pins[0].name, "+") && same(ta->pins[1].name, "-") &&
               ta->pins[0].role == PIN_INPUT && ta->pins[1].role == PIN_INPUT,
           "TESTCAP: pins + and -, both input");
    expect(c != NULL && c->capacitor != NULL && same(c->capacitor->type, "ceramic") &&
               c->capacitor->polarized == 0 && c->capacitor->capacitance_ff == 100000000ULL &&
               c->capacitor->rated_mv == 50000 && same(c->capacitor->dielectric, "X7R") &&
               same(c->capacitor->tolerance, "\xc2\xb1" "10%"),
           "TESTCAP-C: ceramic, not polarized, 1e-7 F = 100000000 fF, 50 V, X7R, +-10%");
    expect(n != NULL && n->capacitor != NULL && n->capacitor->rated_mv == 0 &&
               n->capacitor->dielectric == NULL && n->capacitor->tolerance == NULL,
           "TESTCAP-N: no rating, dielectric or tolerance");
}

static size_t add(Circuit *c, const char *name) {
    size_t index = 0;

    if (circuit_add_part(c, name, &index) != 0) {
        fprintf(stderr, "add %s: %s\n", name, circuit_error(c));
        exit(1);
    }
    return index;
}

static size_t net(Circuit *c) {
    size_t index = 0;

    if (circuit_add_net(c, &index) != 0) {
        fprintf(stderr, "net: %s\n", circuit_error(c));
        exit(1);
    }
    return index;
}

static void wire(Circuit *c, size_t part, const char *pin, size_t n) {
    if (circuit_connect(c, part, pin, n) != 0) {
        fprintf(stderr, "connect %s: %s\n", pin, circuit_error(c));
        exit(1);
    }
}

static void sim_cases(void) {
    const char *names[] = {"TESTCAP-C", "TESTCAP"};
    size_t k;

    for (k = 0; k < 2; k++) {
        Circuit *c = circuit_create(library);
        size_t cap = add(c, names[k]), vcc = add(c, "5V"), gnd = add(c, "GND");
        size_t hi = net(c), lo = net(c);
        char what[160];

        wire(c, vcc, "1", hi);
        wire(c, gnd, "1", lo);
        wire(c, cap, "1", hi);
        wire(c, cap, "2", lo);
        snprintf(what, sizeof what,
                 "%s between 5V and GND: after 1 us the nets are still 1 and 0", names[k]);
        expect(circuit_start(c) == 0 && circuit_advance(c, 1000000) != 0 &&
                   circuit_net_value(c, hi) == CIRCUIT_1 && circuit_net_value(c, lo) == CIRCUIT_0,
               what);
        circuit_destroy(c);
    }
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 ||
        load_capacitor_fixture(library) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    fact_cases();
    sim_cases();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("capacitor_test: all passed\n");
    return 0;
}
