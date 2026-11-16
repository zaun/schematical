/*
 * generic_test (plan-46-B): the generic kind, a part that is only a symbol
 * and a footprint. Each case loads a single part file next to the bundled
 * part files (the part_file_test setup): the kind loads with or without a
 * supply range, demands "simulated": false and at least one pin, takes no
 * key of its own, lists with no facts and "|unsimulated", and a circuit
 * refuses the part.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "listing.h"
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

#define PINS3                                                                                    \
    "{\"number\":1,\"name\":\"VCC\",\"role\":\"power\"},"                                        \
    "{\"number\":2,\"name\":\"GND\",\"role\":\"ground\"},"                                       \
    "{\"number\":3,\"name\":\"OUT\",\"role\":\"output\"}"

/* One generic device: `device_keys` go on the device (each ending in a
   comma), `pins` in its one pinout. */
#define FILE_TEXT(uuid, name, device_keys, pins)                                                 \
    "{\"format\":\"schematical-part\",\"version\":1,\"uuid\":\"" uuid "\",\"kind\":\"generic\"," \
    "\"devices\":[{\"id\":\"custom\",\"function\":\"Sensor\",\"description\":\"A sensor\","      \
    "\"drawer\":[\"Custom\"]," device_keys "\"pinouts\":{\"default\":[" pins "]},"               \
    "\"parts\":[{\"name\":\"" name "\",\"manufacturer\":\"Generic\",\"package\":\"SOT-23\","     \
    "\"pinout\":\"default\",\"footprint\":\"SOT23-3\"}]}]}"

static int ends_with(const char *text, const char *tail) {
    size_t n = strlen(text), m = strlen(tail);

    return n >= m && strcmp(text + n - m, tail) == 0;
}

/* Loads `text`; expects a failure whose message contains `want`. */
static void expect_error(const char *label, const char *text, const char *want) {
    size_t parts = part_library_count(library);
    char *error = NULL;
    int status = load_part_file(library, text, "generic.schpart", &error);
    char what[512];

    snprintf(what, sizeof what, "%s: refused with \"%s\" (got: %s)", label, want,
             status == 0 ? "loaded" : (error != NULL ? error : "(null)"));
    expect(status != 0 && error != NULL && strstr(error, want) != NULL &&
               part_library_count(library) == parts,
           what);
    free(error);
}

int main(void) {
    char *error = NULL;
    const Part *part;

    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }

    /* (a) Three pins, "simulated": false, no supply range. */
    expect(load_part_file(library,
                          FILE_TEXT("B0000000-2222-4333-8444-555555555555", "GEN-A",
                                    "\"simulated\":false,", PINS3),
                          "a.schpart", &error) == 0,
           "(a) a generic part with no vcc loads");
    if (error != NULL) {
        printf("     %s\n", error);
    }
    free(error);
    error = NULL;
    part = part_library_find(library, "GEN-A");
    expect(part != NULL && part->kind == PART_GENERIC && part->simulated == 0 &&
               part->vcc == NULL && part->pin_count == 3,
           "(a) kind PART_GENERIC, simulated 0, vcc NULL, 3 pins");
    {
        char *line = part != NULL ? listing_line(part) : NULL;

        expect(line != NULL && strstr(line, "|generic|") != NULL && ends_with(line, "|unsimulated"),
               "(a) the listing line names the kind and ends |unsimulated");
        if (line != NULL) {
            printf("     %s\n", line);
        }
        free(line);
    }

    /* (b) The same with a device supply range. */
    expect(load_part_file(library,
                          FILE_TEXT("B0000001-2222-4333-8444-555555555555", "GEN-B",
                                    "\"simulated\":false,\"vcc\":[3,5.5],", PINS3),
                          "b.schpart", &error) == 0,
           "(b) a generic part with a device vcc loads");
    if (error != NULL) {
        printf("     %s\n", error);
    }
    free(error);
    error = NULL;
    part = part_library_find(library, "GEN-B");
    expect(part != NULL && part->vcc != NULL && part->vcc->min_mv == 3000 &&
               part->vcc->max_mv == 5500,
           "(b) vcc 3.0-5.5 V");

    /* (c) "simulated" missing or true. */
    expect_error("(c) no \"simulated\"",
                 FILE_TEXT("B0000002-2222-4333-8444-555555555555", "GEN-C", "", PINS3),
                 "a generic part has no simulation model: \"simulated\" must be false");
    expect_error("(c) \"simulated\": true",
                 FILE_TEXT("B0000003-2222-4333-8444-555555555555", "GEN-C",
                           "\"simulated\":true,", PINS3),
                 "a generic part has no simulation model: \"simulated\" must be false");

    /* (d) An empty pinout: the loader refuses one for every kind, before the
       kind sees it (so generic_bind_pinout's own check is a second guard). */
    expect_error("(d) empty pinout",
                 FILE_TEXT("B0000004-2222-4333-8444-555555555555", "GEN-D",
                           "\"simulated\":false,", ""),
                 "pinouts.default: a pinout must be a non-empty array of pins");

    /* (e) A key of another kind. */
    expect_error("(e) unknown device key",
                 FILE_TEXT("B0000005-2222-4333-8444-555555555555", "GEN-E",
                           "\"simulated\":false,\"gates\":[],", PINS3),
                 "\"gates\"");

    /* (f) A circuit refuses the part. */
    {
        Circuit *c = circuit_create(library);
        size_t index = 0;

        expect(circuit_add_part(c, "GEN-A", &index) == -1 &&
                   strcmp(circuit_error(c), "part \"GEN-A\" is not simulated") == 0,
               "(f) circuit_add_part refuses GEN-A: part \"GEN-A\" is not simulated");
        printf("     %s\n", circuit_error(c));
        expect(circuit_add_part(c, "5V", &index) == 0, "(f) the circuit still takes a simulated part");
        circuit_destroy(c);
    }

    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("generic_test: all passed\n");
    return 0;
}
