/*
 * mcu_test (plan-36-F): the mcu kind. The fixture device file
 * (support/mcu_fixture.h) loads next to the real part files; the PartMcu
 * facts read back, with the part's clock over the device's; the listing
 * line ends "|unsimulated"; and a circuit refuses the part: an mcu has no
 * simulation model.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "listing.h"
#include "mcu_fixture.h"
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

static void fact_cases(void) {
    const Part *m = part_library_find(library, "TESTMCU");

    expect(m != NULL && m->kind == PART_MCU && m->mcu != NULL && m->pin_count == 8,
           "TESTMCU loads as an mcu with 8 pins");
    expect(m != NULL && m->mcu != NULL && strcmp(m->mcu->core, "avr") == 0 &&
               m->mcu->max_hz == 10000000ULL && m->mcu->flash == 32768 && m->mcu->ram == 2048 &&
               m->mcu->eeprom == 1024 && m->mcu->psram == 0,
           "TESTMCU: core avr, clock 10000000 (the part's, over the device's 20 MHz), flash 32768, "
           "ram 2048, eeprom 1024, no psram");
    expect(m != NULL && m->vcc != NULL && m->vcc->min_mv == 1800 && m->vcc->max_mv == 5500,
           "TESTMCU: vcc 1.8-5.5 V");
    expect(m != NULL && m->simulated == 0, "TESTMCU: simulated 0");
}

static void listing_cases(void) {
    const Part *m = part_library_find(library, "TESTMCU");
    char *line = m != NULL ? listing_line(m) : NULL;

    expect(line != NULL && strstr(line, "|mcu|") != NULL &&
               strstr(line, "core=avr clock=10000000 flash=32768 ram=2048 eeprom=1024") != NULL &&
               ends_with(line, "|unsimulated"),
           "the listing line names the facts and ends |unsimulated");
    if (line != NULL) {
        printf("     %s\n", line);
    }
    free(line);
}

static void refusal_cases(void) {
    Circuit *c = circuit_create(library);
    size_t index = 0;

    expect(circuit_add_part(c, "TESTMCU", &index) == -1 &&
               strcmp(circuit_error(c), "part \"TESTMCU\" is not simulated") == 0,
           "circuit_add_part refuses TESTMCU: part \"TESTMCU\" is not simulated");
    printf("     %s\n", circuit_error(c));
    expect(circuit_add_part(c, "5V", &index) == 0, "the circuit still takes a simulated part");
    circuit_destroy(c);
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 || load_mcu_fixture(library) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    fact_cases();
    listing_cases();
    refusal_cases();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("mcu_test: all passed\n");
    return 0;
}
