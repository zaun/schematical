/*
 * model_pins_cov_test (plan-68-R): the pin-fact calls of model_pins.h (and
 * timer_section_pins, part_switch_state_contacts) on what their comments
 * promise: 0 for NULL, for a part of another kind and past the last index;
 * the real answer for a part of the kind. Parts are the bundled ones, plus
 * one fixture resistor whose pair names a pin group on its "b" end:
 *
 *   R68-RBUS   pins 1 "C", 2 "2", 3 "C"; one resistor {a: "2", b: "C"}
 *   R68-GEN    a generic part (symbol and footprint only), 3 pins
 *
 * And the generic kind's engine model (generic.c: passive, no state; the
 * Part itself is what create returns).
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "model_pins.h"
#include "parts.h"
#include "read_file.h"
#include "timer.h"

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
        exit(1);
    }
    return part;
}

static const char RBUS[] =
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"resistor\",\"devices\":["
    "{\"id\":\"r68-rbus\",\"function\":\"1 k\",\"description\":\"fixture\",\"drawer\":[],"
    "\"pinouts\":{\"p\":[{\"number\":1,\"name\":\"C\",\"role\":\"inout\"},"
    "{\"number\":2,\"name\":\"2\",\"role\":\"inout\"},{\"number\":3,\"name\":\"C\",\"role\":\"inout\"}]},"
    "\"resistors\":[{\"a\":\"2\",\"b\":\"C\"}],"
    "\"parts\":[{\"name\":\"R68-RBUS\",\"package\":\"SOT-23\",\"pinout\":\"p\",\"footprint\":\"SOT23-3\","
    "\"ohms\":1000}]}]}";

static const char GEN[] =
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"generic\",\"devices\":["
    "{\"id\":\"r68-gen\",\"function\":\"Sensor\",\"description\":\"fixture\",\"drawer\":[],"
    "\"simulated\":false,\"pinouts\":{\"p\":[{\"number\":1,\"name\":\"VCC\",\"role\":\"power\"},"
    "{\"number\":2,\"name\":\"GND\",\"role\":\"ground\"},{\"number\":3,\"name\":\"OUT\",\"role\":\"output\"}]},"
    "\"parts\":[{\"name\":\"R68-GEN\",\"package\":\"SOT-23\",\"pinout\":\"p\",\"footprint\":\"SOT23-3\"}]}]}";

/* Every call on NULL and on a part of another kind (`other`) answers 0. */
static void wrong_parts(const Part *other, const char *label) {
    size_t a = 99, b = 99, c = 99;
    uint32_t mask = 0;
    TimerPins tp;
    char what[160];
    int zero;

    zero = !led_pins(other, &a, &b) && !nixie_link(other, 0, &a, &b) && !nixie_anode_min_mv(other, 0) &&
           !dekatron_link(other, 0, &a, &b) && !dekatron_anode_min_mv(other, 0) &&
           !charlcd_backlight_pins(other, 0, &a, &b) && !diode_pair(other, 0, &a, &b) &&
           !resistor_pair(other, 0, &a, &b) && !transistor_element(other, 0, NULL, &a, &b, &c) &&
           !transistor_pin_limit_mv(other, 0) &&
           !regulator_element(other, 0, NULL, &a, &b, &c, NULL, NULL, NULL, NULL) &&
           !regulator_pin_in(other, 0, 0) && !regulator_pin_out(other, 0, 0) &&
           !regulator_input_max_mv(other) &&
           !powerswitch_element(other, 0, &a, &b, &c, NULL, NULL, NULL, NULL) &&
           !powerswitch_pin_in(other, 0, 0) && !powerswitch_pin_out(other, 0, 0) &&
           !powerswitch_input_max_mv(other) && !inductor_fixed_contact(other, 0, &a, &b) &&
           !fuse_fixed_contact(other, 0, &a, &b) && !relay_fixed_contact(other, 0, &a, &b) &&
           !part_fixed_contact(other, 0, &a, &b) && !switch_contact_pins(other, 1, &a, &b) &&
           !timer_section_pins(other, 0, &tp) && !part_switch_state_contacts(other, 1, &mask);
    snprintf(what, sizeof what, "every pin-fact call answers 0 for %s, and writes nothing", label);
    expect(zero && a == 99 && b == 99 && c == 99 && mask == 0, what);
}

int main(void) {
    char *error = NULL;
    size_t a = 0, b = 0, c = 0;
    TimerPins tp;

    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 ||
        load_parts(library, RBUS, "rbus.json", &error) != 0 ||
        load_parts(library, GEN, "gen.json", &error) != 0) {
        printf("FAIL: cannot load the part library: %s\n", error != NULL ? error : "");
        free(error);
        return 1;
    }

    wrong_parts(NULL, "NULL");
    /* A header is a kind none of the calls answers for. */
    wrong_parts(find("GENERIC-HEADER-1X01"), "a header (GENERIC-HEADER-1X01)");

    /* An inductor's and a fuse's contacts answer for their own kind only:
       each is 0 for the other, though both bind the same terminals shape. */
    {
        const Part *ind = find("1227AS-H-1R0N=P2");
        const Part *fuse = find("0001.1004");

        expect(inductor_fixed_contact(ind, 0, &a, &b) == 1 && a == 0 && b == 1 &&
                   part_fixed_contact(ind, 0, &a, &b) == 1,
               "an inductor's contact 0 is its terminals, pins 0 and 1");
        expect(fuse_fixed_contact(ind, 0, &a, &b) == 0 && inductor_fixed_contact(fuse, 0, &a, &b) == 0,
               "fuse_fixed_contact(inductor) and inductor_fixed_contact(fuse) are 0 (another kind)");
        expect(fuse_fixed_contact(fuse, 0, &a, &b) == 1 && fuse_fixed_contact(fuse, 1, &a, &b) == 0,
               "a two-pin fuse has one contact, its terminals");
    }

    /* A resistor pair whose b end names two pins: the link goes to the
       name's first pin; the same-named pins are then tied both ways. */
    {
        const Part *r = find("R68-RBUS");
        size_t a1 = 9, b1 = 9, a2 = 9, b2 = 9;
        int ok = resistor_pair(r, 0, &a, &b) && a == 1 && b == 0 && resistor_pair(r, 1, &a1, &b1) &&
                 resistor_pair(r, 2, &a2, &b2) && !resistor_pair(r, 3, &c, &c);

        expect(ok && a1 == 0 && b1 == 2 && a2 == 2 && b2 == 0,
               "R68-RBUS: link 2-C to the first C (pin 0), then ties C0-C2 and C2-C0, then 0");
    }

    /* A BJT's base is never refused (0); an NC pin carries no rating (0);
       a channel pin has the part's Vceo; a pin past the end is 0. */
    {
        const Part *q = find("DMMT3906-7-F");

        expect(transistor_pin_limit_mv(q, 4) == 0, "DMMT3906-7-F: the base (pin 5) has no limit");
        expect(transistor_pin_limit_mv(q, 1) == 0, "DMMT3906-7-F: the NC pin (pin 2) has no limit");
        expect(transistor_pin_limit_mv(q, 0) == q->transistor->vmax_mv && q->transistor->vmax_mv > 0,
               "DMMT3906-7-F: an emitter has the part's Vceo");
        expect(transistor_pin_limit_mv(q, q->pin_count) == 0, "DMMT3906-7-F: a pin past the end is 0");
    }

    /* A switch position is 1-based: 0 and one past the last are refused. */
    {
        const Part *sw = find("TE-GDS0204");

        expect(switch_contact_pins(sw, 1, &a, &b) == 1 && switch_contact_pins(sw, 2, &a, &b) == 1,
               "TE-GDS0204: positions 1 and 2 have contacts");
        expect(switch_contact_pins(sw, 0, &a, &b) == 0 && switch_contact_pins(sw, 3, &a, &b) == 0,
               "TE-GDS0204: position 0 and position 3 have none");
    }

    /* A 555 has one section: section 1 does not exist. */
    {
        const Part *t = find("ICM7555ESA+");

        expect(timer_section_pins(t, 0, &tp) == 1 && tp.trig == 1 && tp.out == 2,
               "ICM7555ESA+: section 0 has TRIG on pin 2 and OUT on pin 3");
        expect(timer_section_pins(t, 1, &tp) == 0, "ICM7555ESA+: no section 1");
    }

    /* The generic model: its pin count is the part's, and create hands back the Part. */
    {
        const Part *g = find("R68-GEN");
        const SimComponentType *type = kind_model(g);

        expect(type != NULL && type->pin_count(g) == 3, "R68-GEN: the generic model has the part's 3 pins");
        expect(type != NULL && type->create(g) == (const void *)g && type->destroy == NULL,
               "R68-GEN: create returns the Part itself (no state to free)");
    }

    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("model_pins_cov_test: all passed\n");
    return 0;
}
