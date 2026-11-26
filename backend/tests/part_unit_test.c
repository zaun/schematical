/*
 * part_unit_count / part_unit_at (plan-67-A): a gate part's units are its
 * gates, with the shape, the input pins, one or two outputs each marked
 * inverted or not, and the device's Schmitt and open-drain marks. Two gates on
 * the same inputs are one unit with two outputs. Every other part has none.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parts.h"
#include "read_file.h"

static int failures;

#define CHECK(cond, ...)                                     \
    do {                                                     \
        if (!(cond)) {                                       \
            fprintf(stderr, "FAIL %s:%d: ", __FILE__, __LINE__); \
            fprintf(stderr, __VA_ARGS__);                    \
            fputc('\n', stderr);                             \
            failures++;                                      \
        }                                                    \
    } while (0)

static const Part *find(const PartLibrary *library, const char *name) {
    const Part *part = part_library_find(library, name);

    CHECK(part != NULL, "no part %s", name);
    return part;
}

/* The engine pin index of the pin named `name`, or -1. */
static int pin_named(const Part *part, const char *name) {
    size_t i;

    for (i = 0; i < part->pin_count; i++) {
        if (strcmp(part->pins[i].name, name) == 0) {
            return (int)i;
        }
    }
    return -1;
}

/* A gate file with one 5-pin device (pins A, B, GND, Y, VCC) and the given
   device keys after "pinouts". */
#define FILE_WITH(name, keys)                                                                    \
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"gate\",\"devices\":["            \
    "{\"id\":\"" name "\",\"function\":\"NAND\",\"description\":\"Test gate\","                  \
    "\"drawer\":[\"Logic\",\"Gates\"],"                                                          \
    "\"pinouts\":{\"p5\":[{\"number\":1,\"name\":\"A\",\"role\":\"input\"},"                     \
    "{\"number\":2,\"name\":\"B\",\"role\":\"input\"},{\"number\":3,\"name\":\"GND\",\"role\":\"ground\"}," \
    "{\"number\":4,\"name\":\"Y\",\"role\":\"output\"},{\"number\":5,\"name\":\"VCC\",\"role\":\"power\"}]}," \
    keys ","                                                                                     \
    "\"timing\":{\"HC\":{\"rise\":9,\"fall\":9,\"vcc\":[2,6]}},"                                 \
    "\"parts\":[{\"name\":\"" name "-PART\",\"manufacturer\":\"Generic\",\"package\":\"SOT-23\"," \
    "\"pinout\":\"p5\",\"footprint\":\"SOT23-5\",\"family\":\"HC\"}]}]}"

/* Loading `json` must fail with a message containing `expected`. */
static void expect_error(PartLibrary *library, const char *json, const char *expected) {
    char *error = NULL;
    int result = load_parts(library, json, "test.json", &error);

    CHECK(result != 0, "expected a load error containing: %s", expected);
    CHECK(error != NULL && strstr(error, expected) != NULL, "error was: %s (wanted: %s)",
          error != NULL ? error : "(none)", expected);
    free(error);
}

static void test_quad_nand(const PartLibrary *library) {
    const Part *part = find(library, "SN74HC00N");
    size_t i;

    if (part == NULL) {
        return;
    }
    CHECK(part_unit_count(part) == 4, "SN74HC00N has %zu units", part_unit_count(part));
    for (i = 0; i < part_unit_count(part); i++) {
        const PartUnit *unit = part_unit_at(part, i);

        CHECK(unit->shape == UNIT_AND, "unit %zu shape %d", i, (int)unit->shape);
        CHECK(unit->input_count == 2, "unit %zu has %zu inputs", i, unit->input_count);
        CHECK(unit->output_count == 1 && unit->outputs[0].inverted == 1, "unit %zu output", i);
        CHECK(unit->schmitt == 0 && unit->open_drain == 0, "unit %zu marks", i);
    }
    {
        const PartUnit *unit = part_unit_at(part, 0);

        CHECK((int)unit->inputs[0] == pin_named(part, "1A"), "unit A input 0");
        CHECK((int)unit->inputs[1] == pin_named(part, "1B"), "unit A input 1");
        CHECK((int)unit->outputs[0].pin == pin_named(part, "1Y"), "unit A output");
    }
    CHECK(part_unit_at(part, 4) == NULL, "part_unit_at past the end");
}

static void test_two_outputs(const PartLibrary *library) {
    const Part *part = find(library, "CD4068BE");
    const PartUnit *unit;

    if (part == NULL) {
        return;
    }
    CHECK(part_unit_count(part) == 1, "CD4068BE has %zu units", part_unit_count(part));
    unit = part_unit_at(part, 0);
    CHECK(unit->shape == UNIT_AND && unit->input_count == 8, "CD4068BE shape / inputs");
    CHECK(unit->output_count == 2, "CD4068BE has %zu outputs", unit->output_count);
    CHECK((int)unit->outputs[0].pin == pin_named(part, "K") && unit->outputs[0].inverted == 0,
          "CD4068BE true output is K, first");
    CHECK((int)unit->outputs[1].pin == pin_named(part, "J") && unit->outputs[1].inverted == 1,
          "CD4068BE inverted output is J, second");

    part = find(library, "CD4041UBE");
    if (part == NULL) {
        return;
    }
    CHECK(part_unit_count(part) == 4, "CD4041UBE has %zu units", part_unit_count(part));
    unit = part_unit_at(part, 0);
    CHECK(unit->shape == UNIT_BUFFER && unit->input_count == 1 && unit->output_count == 2,
          "CD4041UBE unit A");
    CHECK(unit->outputs[0].inverted == 0 && unit->outputs[1].inverted == 1,
          "CD4041UBE true output first");
}

static void test_wide_and_marks(const PartLibrary *library) {
    const Part *part = find(library, "SN74ALS133N");

    if (part != NULL) {
        CHECK(part_unit_count(part) == 1 && part_unit_at(part, 0)->input_count == 13,
              "SN74ALS133N is one 13-input unit");
    }
    part = find(library, "SN74HC266N");
    if (part != NULL) {
        const PartUnit *unit = part_unit_at(part, 0);

        CHECK(unit != NULL && unit->shape == UNIT_XOR && unit->outputs[0].inverted == 1,
              "SN74HC266N is XNOR");
        CHECK(unit != NULL && unit->open_drain == 1, "SN74HC266N is open drain");
    }
    part = find(library, "SN74HC14N");
    if (part != NULL) {
        size_t i;

        CHECK(part_unit_count(part) == 6, "SN74HC14N has %zu units", part_unit_count(part));
        for (i = 0; i < part_unit_count(part); i++) {
            CHECK(part_unit_at(part, i)->schmitt == 1, "SN74HC14N unit %zu is Schmitt", i);
            CHECK(part_unit_at(part, i)->shape == UNIT_BUFFER &&
                      part_unit_at(part, i)->outputs[0].inverted == 1,
                  "SN74HC14N unit %zu is an inverter", i);
        }
    }
    part = find(library, "SN74HC74N");
    if (part != NULL) {
        CHECK(part_unit_count(part) == 0 && part_unit_at(part, 0) == NULL,
              "a flip-flop has no units");
    }
    CHECK(part_unit_count(NULL) == 0 && part_unit_at(NULL, 0) == NULL, "NULL part");
}

/* Every gate part: as many units as its drawer count, no pin in two units,
   inputs are inputs and outputs are outputs. */
static void test_every_gate_part(const PartLibrary *library) {
    size_t count = part_library_count(library);
    size_t gates = 0;
    size_t i;

    for (i = 0; i < count; i++) {
        const Part *part = part_library_at(library, i);
        unsigned char *seen;
        size_t u;
        size_t k;

        if (part->kind != PART_GATE) {
            continue;
        }
        gates++;
        CHECK(part_unit_count(part) == part->logic->units, "%s: %zu units, logic says %u",
              part->name, part_unit_count(part), part->logic->units);
        seen = calloc(part->pin_count, 1);
        for (u = 0; u < part_unit_count(part); u++) {
            const PartUnit *unit = part_unit_at(part, u);

            CHECK(unit->output_count == 1 || unit->output_count == 2, "%s unit %zu outputs",
                  part->name, u);
            for (k = 0; k < unit->input_count; k++) {
                size_t pin = unit->inputs[k];

                CHECK(pin < part->pin_count && part->pins[pin].role == PIN_INPUT,
                      "%s unit %zu input %zu is not an input pin", part->name, u, k);
                if (pin < part->pin_count) {
                    CHECK(seen[pin] == 0, "%s pin %zu is in two units", part->name, pin);
                    seen[pin] = 1;
                }
            }
            for (k = 0; k < unit->output_count; k++) {
                size_t pin = unit->outputs[k].pin;

                CHECK(pin < part->pin_count && part->pins[pin].role == PIN_OUTPUT,
                      "%s unit %zu output %zu is not an output pin", part->name, u, k);
                if (pin < part->pin_count) {
                    CHECK(seen[pin] == 0, "%s pin %zu is in two units", part->name, pin);
                    seen[pin] = 1;
                }
            }
        }
        free(seen);
    }
    CHECK(gates > 3000, "only %zu gate parts were checked", gates);
}

static void test_load_errors(PartLibrary *library) {
    expect_error(library,
                 FILE_WITH("UNIT-THREE",
                           "\"gates\":[{\"op\":\"nand\",\"inputs\":[\"A\",\"B\"],\"output\":\"Y\"},"
                           "{\"op\":\"and\",\"inputs\":[\"A\",\"B\"],\"output\":\"Y\"},"
                           "{\"op\":\"nand\",\"inputs\":[\"A\",\"B\"],\"output\":\"Y\"}]"),
                 "more than two gates share these inputs");
    expect_error(library,
                 FILE_WITH("UNIT-SAME",
                           "\"gates\":[{\"op\":\"nand\",\"inputs\":[\"A\",\"B\"],\"output\":\"Y\"},"
                           "{\"op\":\"nand\",\"inputs\":[\"A\",\"B\"],\"output\":\"Y\"}]"),
                 "gates on the same inputs must be a true and an inverted output");
    expect_error(library,
                 FILE_WITH("UNIT-INPUT",
                           "\"gates\":[{\"op\":\"nand\",\"inputs\":[\"A\",\"B\"],\"output\":\"Y\"}],"
                           "\"input\":\"slow\""),
                 "\"input\" must be \"standard\" or \"schmitt\"");
}

int main(void) {
    PartLibrary *library = part_library_create();

    if (load_parts_dir(library, PARTS_DIR) != 0) {
        return 1;
    }
    test_quad_nand(library);
    test_two_outputs(library);
    test_wide_and_marks(library);
    test_every_gate_part(library);
    test_load_errors(library);
    part_library_destroy(library);
    if (failures != 0) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    printf("part_unit_test: ok\n");
    return 0;
}
