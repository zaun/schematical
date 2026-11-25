/*
 * pld_test: the pld kind (programmable logic devices). The shipped parts
 * read back their facts (macrocells, the part's speed grade, the supply
 * range with a commercial part's own); each rule of the loader refuses what
 * it should (single part files next to the bundled part files); the listing
 * ends "|unsimulated"; and a circuit refuses the part.
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

static int ends_with(const char *text, const char *tail) {
    size_t n = strlen(text), m = strlen(tail);

    return n >= m && strcmp(text + n - m, tail) == 0;
}

#define PINS3                                                                                    \
    "{\"number\":1,\"name\":\"VCC\",\"role\":\"power\"},"                                        \
    "{\"number\":2,\"name\":\"GND\",\"role\":\"ground\"},"                                       \
    "{\"number\":3,\"name\":\"I\",\"role\":\"input\"}"

#define PINS_NO_GROUND                                                                           \
    "{\"number\":1,\"name\":\"VCC\",\"role\":\"power\"},"                                        \
    "{\"number\":2,\"name\":\"I\",\"role\":\"input\"}"

#define PINS_NO_POWER                                                                            \
    "{\"number\":1,\"name\":\"GND\",\"role\":\"ground\"},"                                       \
    "{\"number\":2,\"name\":\"I\",\"role\":\"input\"}"

/* One pld device: `device_keys` go on the device (each ending in a comma),
   `part_keys` on its one part, `pins` in its one pinout. */
#define FILE_TEXT(uuid, name, device_keys, part_keys, pins)                                      \
    "{\"format\":\"schematical-part\",\"version\":1,\"uuid\":\"" uuid "\",\"kind\":\"pld\","     \
    "\"devices\":[{\"id\":\"custom\",\"function\":\"Test\",\"description\":\"A PLD\","           \
    "\"drawer\":[\"Custom\"]," device_keys "\"pinouts\":{\"default\":[" pins "]},"               \
    "\"parts\":[{\"name\":\"" name "\",\"manufacturer\":\"Generic\",\"package\":\"SOT-23\","     \
    "\"pinout\":\"default\",\"footprint\":\"SOT23-3\"" part_keys "}]}]}"

#define GOOD_DEVICE "\"simulated\":false,\"macrocells\":4,\"vcc\":[4.5,5.5],"

/* Loads `text`; expects a failure whose message contains `want`. */
static void expect_error(const char *label, const char *text, const char *want) {
    size_t parts = part_library_count(library);
    char *error = NULL;
    int status = load_part_file(library, text, "pld.schpart", &error);
    char what[512];

    snprintf(what, sizeof what, "%s: refused with \"%s\" (got: %s)", label, want,
             status == 0 ? "loaded" : (error != NULL ? error : "(null)"));
    expect(status != 0 && error != NULL && strstr(error, want) != NULL &&
               part_library_count(library) == parts,
           what);
    free(error);
}

static void shipped_cases(void) {
    const Part *a = part_library_find(library, "ATF22V10C-10JU");
    const Part *c = part_library_find(library, "ATF22V10C-5JX");
    const Part *g = part_library_find(library, "ATF750C-15GM/883");
    const Part *s = part_library_find(library, "ATF16V8B-15SU");
    char *line = a != NULL ? listing_line(a) : NULL;

    expect(a != NULL && a->kind == PART_PLD && a->pld != NULL && a->pin_count == 28 &&
               a->simulated == 0,
           "ATF22V10C-10JU: a pld with 28 pins, not simulated");
    expect(a != NULL && a->pld != NULL && a->pld->macrocells == 10 && a->pld->tpd_ps == 10000,
           "ATF22V10C-10JU: 10 macrocells, tpd 10 ns");
    expect(a != NULL && a->vcc != NULL && a->vcc->min_mv == 4500 && a->vcc->max_mv == 5500,
           "ATF22V10C-10JU: the device's vcc 4.5-5.5 V");
    expect(c != NULL && c->pld != NULL && c->pld->tpd_ps == 5000 && c->vcc != NULL &&
               c->vcc->min_mv == 4750 && c->vcc->max_mv == 5250,
           "ATF22V10C-5JX: tpd 5 ns, its own commercial vcc 4.75-5.25 V");
    expect(g != NULL && g->pld != NULL && g->pld->macrocells == 10 && (g->flags & PART_MILITARY) &&
               (g->flags & PART_CERAMIC),
           "ATF750C-15GM/883: military and ceramic");
    expect(s != NULL && s->pld != NULL && s->pld->macrocells == 8 && s->pld->tpd_ps == 15000 &&
               s->pin_count == 20,
           "ATF16V8B-15SU: 8 macrocells, tpd 15 ns, 20 pins");
    expect(line != NULL && strstr(line, "|pld|") != NULL &&
               strstr(line, "macrocells=10 tpd=10000") != NULL && ends_with(line, "|unsimulated"),
           "the listing line names the facts and ends |unsimulated");
    free(line);
}

static void loader_cases(void) {
    char *error = NULL;
    const Part *part;

    expect(load_part_file(library,
                          FILE_TEXT("C0000000-2222-4333-8444-555555555555", "PLD-A", GOOD_DEVICE,
                                    ",\"tpd\":7.5", PINS3),
                          "a.schpart", &error) == 0,
           "(a) a pld with macrocells, vcc and tpd loads");
    if (error != NULL) {
        printf("     %s\n", error);
    }
    free(error);
    part = part_library_find(library, "PLD-A");
    expect(part != NULL && part->pld != NULL && part->pld->macrocells == 4 &&
               part->pld->tpd_ps == 7500,
           "(a) 4 macrocells, tpd 7.5 ns = 7500 ps");

    expect_error("(b) no \"simulated\"",
                 FILE_TEXT("C0000001-2222-4333-8444-555555555555", "PLD-B",
                           "\"macrocells\":4,\"vcc\":[4.5,5.5],", ",\"tpd\":10", PINS3),
                 "a pld has no simulation model: \"simulated\" must be false");
    expect_error("(c) no macrocells",
                 FILE_TEXT("C0000002-2222-4333-8444-555555555555", "PLD-C",
                           "\"simulated\":false,\"vcc\":[4.5,5.5],", ",\"tpd\":10", PINS3),
                 "macrocells");
    expect_error("(d) zero macrocells",
                 FILE_TEXT("C0000003-2222-4333-8444-555555555555", "PLD-D",
                           "\"simulated\":false,\"macrocells\":0,\"vcc\":[4.5,5.5],",
                           ",\"tpd\":10", PINS3),
                 "\"macrocells\" must be above 0");
    expect_error("(e) no tpd",
                 FILE_TEXT("C0000004-2222-4333-8444-555555555555", "PLD-E", GOOD_DEVICE, "", PINS3),
                 "tpd");
    expect_error("(f) zero tpd",
                 FILE_TEXT("C0000005-2222-4333-8444-555555555555", "PLD-F", GOOD_DEVICE,
                           ",\"tpd\":0", PINS3),
                 "\"tpd\" must be above 0");
    expect_error("(g) no vcc anywhere",
                 FILE_TEXT("C0000006-2222-4333-8444-555555555555", "PLD-G",
                           "\"simulated\":false,\"macrocells\":4,", ",\"tpd\":10", PINS3),
                 "vcc");
    expect_error("(h) no ground pin",
                 FILE_TEXT("C0000007-2222-4333-8444-555555555555", "PLD-H", GOOD_DEVICE,
                           ",\"tpd\":10", PINS_NO_GROUND),
                 "a pld pinout needs a ground pin");
    expect_error("(i) no power pin",
                 FILE_TEXT("C0000008-2222-4333-8444-555555555555", "PLD-I", GOOD_DEVICE,
                           ",\"tpd\":10", PINS_NO_POWER),
                 "a pld pinout needs a power pin");
    expect_error("(j) a key of another kind",
                 FILE_TEXT("C0000009-2222-4333-8444-555555555555", "PLD-J",
                           GOOD_DEVICE "\"gates\":[],", ",\"tpd\":10", PINS3),
                 "\"gates\"");
}

static void refusal_cases(void) {
    Circuit *c = circuit_create(library);
    size_t index = 0;

    expect(circuit_add_part(c, "ATF22V10C-10JU", &index) == -1 &&
               strcmp(circuit_error(c), "part \"ATF22V10C-10JU\" is not simulated") == 0,
           "circuit_add_part refuses ATF22V10C-10JU: part \"ATF22V10C-10JU\" is not simulated");
    expect(circuit_add_part(c, "5V", &index) == 0, "the circuit still takes a simulated part");
    circuit_destroy(c);
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    shipped_cases();
    loader_cases();
    refusal_cases();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("pld_test: all passed\n");
    return 0;
}
