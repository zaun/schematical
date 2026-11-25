/*
 * erc_pins_test: erc_pin_type / erc_pin_drives_power (include/erc.h,
 * plan-63-B) on named real parts: the role and kind rules, the passive kinds,
 * supplies and pulls, regulator outputs, open-drain outputs, and the kinds
 * whose outputs are 3-state through an output enable.
 */

#include <stdio.h>
#include <string.h>

#include "erc.h"
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

static const Part *find(const char *name) {
    const Part *part = part_library_find(library, name);

    if (part == NULL) {
        printf("FAIL: no part %s\n", name);
        failures++;
    }
    return part;
}

/* Every pin of the part with role `role` has type `want`; at least one does. */
static void role_is(const char *name, PinRole role, ErcPinType want) {
    const Part *part = find(name);
    size_t i, seen = 0, wrong = 0;
    char what[160];

    if (part == NULL) {
        return;
    }
    for (i = 0; i < part->pin_count; i++) {
        ErcPinType type;

        if (part->pins[i].role != role) {
            continue;
        }
        seen++;
        if (!erc_pin_type(part, i, &type) || type != want) {
            wrong++;
        }
    }
    snprintf(what, sizeof what, "%s: role %d pins are %s (%zu pins, %zu wrong)", name, (int)role,
             erc_pin_type_name(want), seen, wrong);
    expect(seen > 0 && wrong == 0, what);
}

/* Every pin that has a type has type `want` (an nc pin is no_connect). */
static void every_pin_is(const char *name, ErcPinType want) {
    const Part *part = find(name);
    size_t i, wrong = 0;
    char what[160];

    if (part == NULL) {
        return;
    }
    for (i = 0; i < part->pin_count; i++) {
        ErcPinType type;

        if (part->pins[i].role == PIN_NC || part->pins[i].role == PIN_ABSENT) {
            continue;
        }
        if (!erc_pin_type(part, i, &type) || type != want) {
            wrong++;
        }
    }
    snprintf(what, sizeof what, "%s: every pin is %s (%zu wrong)", name, erc_pin_type_name(want), wrong);
    expect(wrong == 0, what);
}

/* The pin named `pin_name` has type `want` and drives_power `drives`. */
static void named_pin(const char *name, const char *pin_name, ErcPinType want, int drives) {
    const Part *part = find(name);
    size_t i;
    char what[160];

    if (part == NULL) {
        return;
    }
    for (i = 0; i < part->pin_count; i++) {
        ErcPinType type;

        if (strcmp(part->pins[i].name, pin_name) != 0) {
            continue;
        }
        snprintf(what, sizeof what, "%s.%s is %s, drives power %d", name, pin_name, erc_pin_type_name(want),
                 drives);
        expect(erc_pin_type(part, i, &type) && type == want && erc_pin_drives_power(part, i) == drives, what);
        return;
    }
    snprintf(what, sizeof what, "%s has a pin %s", name, pin_name);
    expect(0, what);
}

static void check_names(void) {
    expect(strcmp(erc_pin_type_name(ERC_INPUT), "input") == 0, "name input");
    expect(strcmp(erc_pin_type_name(ERC_TRI_STATE), "tri_state") == 0, "name tri_state");
    expect(strcmp(erc_pin_type_name(ERC_POWER_IN), "power_in") == 0, "name power_in");
    expect(strcmp(erc_pin_type_name(ERC_OPEN_COLLECTOR), "open_collector") == 0, "name open_collector");
    expect(strcmp(erc_pin_type_name(ERC_NO_CONNECT), "no_connect") == 0, "name no_connect");
    expect(ERC_PIN_TYPE_COUNT == 12, "twelve types");
}

static void check_plain(void) {
    role_is("SN74HC00N", PIN_INPUT, ERC_INPUT);
    role_is("SN74HC00N", PIN_OUTPUT, ERC_OUTPUT);
    role_is("SN74HC00N", PIN_POWER, ERC_POWER_IN);
    role_is("SN74HC00N", PIN_GROUND, ERC_POWER_IN);
    role_is("SN74LS07N", PIN_OUTPUT, ERC_OPEN_COLLECTOR);
    role_is("SN74HC74N", PIN_OUTPUT, ERC_OUTPUT);
    role_is("SN74HC138N", PIN_OUTPUT, ERC_OUTPUT);
    role_is("SN74HC163N", PIN_OUTPUT, ERC_OUTPUT);
    role_is("ATMEGA328P-PU", PIN_INOUT, ERC_BIDIRECTIONAL);
    role_is("ATMEGA328P-PU", PIN_POWER, ERC_POWER_IN);
    role_is("ATMEGA328P-PU", PIN_GROUND, ERC_POWER_IN);
}

/* Buffer (3-state), latch and dff parts with an output enable: every output
   is tri_state, every input an input (plan-63-B phase 2). */
static void check_tristate(void) {
    static const char *const NAMES[] = {
        "SN74HC244N", "SN74HC240N", "SN74HC541N", "SN74HC125N", "SN74HC373N", "SN74HC574N", "SN74LS374N",
    };
    size_t i;

    for (i = 0; i < sizeof NAMES / sizeof NAMES[0]; i++) {
        role_is(NAMES[i], PIN_OUTPUT, ERC_TRI_STATE);
        role_is(NAMES[i], PIN_INPUT, ERC_INPUT);
        role_is(NAMES[i], PIN_POWER, ERC_POWER_IN);
    }
}

/* register, shift and counter parts with an output enable (plan-63-C phase 1). */
static void check_sequential_tristate(void) {
    static const char *const STORAGE[] = {"QA", "QB", "QC", "QD", "QE", "QF", "QG", "QH"};
    size_t i;

    for (i = 0; i < sizeof STORAGE / sizeof STORAGE[0]; i++) {
        named_pin("SN74HC595N", STORAGE[i], ERC_TRI_STATE, 0);
    }
    named_pin("SN74HC595N", "QH'", ERC_OUTPUT, 0);
    role_is("74ACT11374DWR", PIN_OUTPUT, ERC_TRI_STATE);
    /* The counter's Q pins are gated by OE; its carries (RCO, CCO) are not. */
    for (i = 0; i < 4; i++) {
        named_pin("SN74ALS561AN", STORAGE[i], ERC_TRI_STATE, 0);
    }
    named_pin("SN74ALS561AN", "RCO", ERC_OUTPUT, 0);
    named_pin("SN74ALS561AN", "CCO", ERC_OUTPUT, 0);
}

static void check_passive_kinds(void) {
    every_pin_is("GENERIC-LED", ERC_PASSIVE);
    every_pin_is("GENERIC-RESISTOR", ERC_PASSIVE);
    every_pin_is("GENERIC-HEADER-1X02", ERC_PASSIVE);
}

static void check_supplies(void) {
    named_pin("5V", "VCC", ERC_POWER_IN, 1);
    named_pin("GND", "GND", ERC_POWER_IN, 1);
    named_pin("PULLUP-5V", "PU", ERC_PASSIVE, 0);
    named_pin("PULLDOWN", "PD", ERC_PASSIVE, 0);
    /* An IC's own power pin drives nothing. */
    named_pin("SN74HC00N", "VCC", ERC_POWER_IN, 0);
}

static void check_regulator(void) {
    named_pin("LM7805MPX/NOPB", "OUTPUT", ERC_POWER_OUT, 0);
    named_pin("LM7805MPX/NOPB", "INPUT", ERC_POWER_IN, 0);
}

static void check_edges(void) {
    const Part *part = find("SN74HC00N");
    ErcPinType type = ERC_FREE;

    if (part == NULL) {
        return;
    }
    expect(!erc_pin_type(part, part->pin_count, &type) && type == ERC_FREE, "a pin past the end has no type");
    expect(!erc_pin_type(NULL, 0, &type), "no part, no type");
    expect(erc_pin_drives_power(part, part->pin_count) == 0, "no drive past the end");
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    check_names();
    check_plain();
    check_tristate();
    check_sequential_tristate();
    check_passive_kinds();
    check_supplies();
    check_regulator();
    check_edges();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("erc_pins_test: all passed\n");
    return 0;
}
