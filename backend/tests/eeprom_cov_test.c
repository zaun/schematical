/*
 * eeprom_cov_test (plan-68-R): parallel EEPROM cases eeprom_test does not
 * drive, by the model's rules (eeprom_int.h header: loads, the tBLC window,
 * tWC, DATA# polling, software data protection; a timing violation or an
 * unknown control stores X). Fixture (tests/fixtures/model_cov/eeprom.json):
 *
 *   R68-AT28C64B  the AT28C64B (8K x 8, page 64, commands at 1555 / 0AAA,
 *                 protection off) with tOH 20 ns, tDH 10 ns and tOEH 10 ns
 *                 added to its -15 grade (the bundled one gives none)
 *
 * Memory is read through the model's memory_read hook.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "kind.h"
#include "parts.h"
#include "read_file.h"

static int failures;
static PartLibrary *library;
static const Part *part;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

#define NS 1000ull
#define US (1000ull * NS)
#define MS (1000ull * US)
#define TBLC (150 * US)
#define TWC (10 * MS)

static void set_address(Bench *b, uint32_t address) {
    unsigned i;
    char name[8];

    for (i = 0; i < 13; i++) {
        snprintf(name, sizeof name, "A%u", i);
        bench_set(b, name, (address >> i) & 1u ? LOGIC_1 : LOGIC_0);
    }
}

static void set_data(Bench *b, int drive, uint8_t value) {
    unsigned i;
    char name[8];

    for (i = 0; i < 8; i++) {
        snprintf(name, sizeof name, "I/O%u", i);
        bench_set(b, name, !drive ? LOGIC_Z : (value >> i) & 1u ? LOGIC_1 : LOGIC_0);
    }
}

static Bench *open_bench(void) {
    Bench *b = bench_create(part);

    bench_set(b, "CE#", LOGIC_0);
    bench_set(b, "OE#", LOGIC_1);
    bench_set(b, "WE#", LOGIC_1);
    bench_run(b, 1 * US);
    return b;
}

/* One load: address and data, WE# low 200 ns (tWP 100), high again, 1 us before the next. */
static void load(Bench *b, uint32_t address, uint8_t value) {
    set_address(b, address);
    set_data(b, 1, value);
    bench_run(b, 100 * NS);
    bench_set(b, "WE#", LOGIC_0);
    bench_run(b, 200 * NS);
    bench_set(b, "WE#", LOGIC_1);
    bench_run(b, 1 * US);
}

static uint8_t word(Bench *b, uint32_t address, uint8_t *known) {
    uint32_t v = 0, k = 0;

    kind_model(part)->memory_read(bench_state(b), address, 1, &v, &k);
    *known = (uint8_t)k;
    return (uint8_t)(v & k);
}

static void finish(Bench *b) {
    bench_run(b, TBLC + TWC + 1 * MS);
}

static void many_unknown_loads(void) {
    Bench *b = open_bench();
    uint8_t k1, k2;
    int i;

    for (i = 0; i < 33; i++) {          /* 33 loads with A12 unknown: more than the 32 tracked */
        set_address(b, (uint32_t)i);
        bench_set(b, "A12", LOGIC_X);
        set_data(b, 1, 0x11);
        bench_run(b, 100 * NS);
        bench_set(b, "WE#", LOGIC_0);
        bench_run(b, 200 * NS);
        bench_set(b, "WE#", LOGIC_1);
        bench_run(b, 1 * US);
    }
    finish(b);
    word(b, 0x0800, &k1);
    word(b, 0x1FFF, &k2);
    expect(k1 == 0 && k2 == 0, "33 loads at unknown addresses in one window: every word is X (0800h, 1FFFh)");
    bench_destroy(b);
}

static void partial_commands(void) {
    Bench *b = open_bench();
    uint8_t v, k;

    load(b, 0x1555, 0xAA);              /* the first step of a command ... */
    set_address(b, 0x0010);
    bench_set(b, "A3", LOGIC_X);        /* ... then a load at an unknown address */
    set_data(b, 1, 0x22);
    bench_run(b, 100 * NS);
    bench_set(b, "WE#", LOGIC_0);
    bench_run(b, 200 * NS);
    bench_set(b, "WE#", LOGIC_1);
    finish(b);
    v = word(b, 0x1555, &k);
    expect(v == 0xAA && k == 0xFF, "AA at 1555 then an unknown-address load: the AA is data (1555 = AAh)");
    word(b, 0x0018, &k);
    expect(k == 0, "... and the unknown load may have hit 0010h or 0018h: 0018h is X");
    bench_destroy(b);

    b = open_bench();
    load(b, 0x1555, 0xAA);              /* a lone first step: the window times out */
    finish(b);
    v = word(b, 0x1555, &k);
    expect(v == 0xAA && k == 0xFF, "a window of only AA at 1555 (no sequence) writes it as data");
    bench_destroy(b);
}

static void unknown_write_enable(void) {
    Bench *b = open_bench();
    uint8_t k;

    set_address(b, 0x0040);
    set_data(b, 1, 0x33);
    bench_run(b, 100 * NS);
    bench_set(b, "WE#", LOGIC_0);       /* a load starts at 0040h ... */
    bench_run(b, 100 * NS);
    set_address(b, 0x0041);
    bench_run(b, 100 * NS);
    bench_set(b, "WE#", LOGIC_X);       /* ... and WE# goes X */
    bench_run(b, 100 * NS);
    bench_set(b, "WE#", LOGIC_1);
    finish(b);
    word(b, 0x0040, &k);
    expect(k == 0, "WE# X during a load: the load's address (0040h) is X");
    word(b, 0x0041, &k);
    expect(k == 0, "... and the address now on the bus (0041h) is X");
    bench_destroy(b);
}

static void data_hold(void) {
    Bench *b = open_bench();
    uint8_t v, k;

    /* a command step whose data changes inside tDH: it is no command, and its byte is X */
    set_address(b, 0x1555);
    set_data(b, 1, 0xAA);
    bench_run(b, 100 * NS);
    bench_set(b, "WE#", LOGIC_0);
    bench_run(b, 200 * NS);
    bench_set(b, "WE#", LOGIC_1);
    bench_run(b, 5 * NS);
    set_data(b, 1, 0x00);
    bench_run(b, 1 * US);
    finish(b);
    word(b, 0x1555, &k);
    expect(k == 0, "tDH missed after the first command step: the AA is data and X (1555)");
    load(b, 0x0100, 0x5A);              /* protection never came on: a plain write still lands */
    finish(b);
    v = word(b, 0x0100, &k);
    expect(v == 0x5A && k == 0xFF, "... and a later plain write still lands (0100h = 5Ah)");
    bench_destroy(b);
}

static void poll_at_cycle_start(void) {
    Bench *b = open_bench();
    LogicValue io7;

    load(b, 0x0200, 0x80);
    set_data(b, 0, 0);
    bench_set(b, "OE#", LOGIC_0);       /* reading before the window times out */
    bench_run(b, TBLC + 10 * US);       /* the cycle has begun with OE# already low */
    io7 = bench_get(b, "I/O7");
    expect(io7 == LOGIC_0, "DATA# polling from the cycle's start: I/O7 is the complement of the last bit 7 (0)");
    bench_set(b, "OE#", LOGIC_1);
    bench_destroy(b);
}

static void address_at_load_start(void) {
    Bench *b = open_bench();
    uint8_t v, k;

    set_address(b, 0x0300);
    set_data(b, 1, 0x77);
    bench_run(b, 100 * NS);
    bench_set(b, "WE#", LOGIC_0);       /* the load starts ... */
    bench_run(b, 0);
    set_address(b, 0x0301);             /* ... and the address settles at that same instant */
    bench_run(b, 0);
    bench_run(b, 200 * NS);
    bench_set(b, "WE#", LOGIC_1);
    bench_run(b, 1 * US);
    finish(b);
    v = word(b, 0x0301, &k);
    expect(v == 0x77 && k == 0xFF, "an address that settles as the load starts: the byte lands at the final one (0301h)");
    bench_destroy(b);
}

static void output_hold(void) {
    Bench *b = open_bench();
    uint32_t values[2] = {0x0F, 0xF0};

    kind_model(part)->memory_load(bench_state(b), 0x10, values, 2);
    set_data(b, 0, 0);
    set_address(b, 0x10);
    bench_set(b, "OE#", LOGIC_0);
    bench_run(b, 1 * US);
    expect(bench_get(b, "I/O0") == LOGIC_1, "read 0010h: I/O0 is 1 (0Fh)");
    set_address(b, 0x11);
    bench_run(b, 10 * NS);
    expect(bench_get(b, "I/O0") == LOGIC_1, "10 ns after the address change (tOH 20): the old data holds");
    bench_run(b, 30 * NS);
    expect(bench_get(b, "I/O0") == LOGIC_X, "past tOH, before tACC: X");
    bench_run(b, 200 * NS);
    expect(bench_get(b, "I/O0") == LOGIC_0, "past tACC: the new data (F0h, I/O0 0)");
    bench_destroy(b);
}

int main(void) {
    char *text = read_file(GOLDEN_DIR "/../fixtures/model_cov/eeprom.json");
    char *error = NULL;

    setvbuf(stdout, NULL, _IOLBF, 0);   /* the leak check ends with _exit: keep the lines */
    library = part_library_create();
    if (library == NULL || text == NULL || load_parts_dir(library, PARTS_DIR) != 0 ||
        load_parts(library, text, "eeprom.json", &error) != 0) {
        printf("FAIL: cannot load the parts: %s\n", error != NULL ? error : "");
        return 1;
    }
    free(text);
    part = part_library_find(library, "R68-AT28C64B");
    many_unknown_loads();
    partial_commands();
    unknown_write_enable();
    data_hold();
    poll_at_cycle_start();
    address_at_load_start();
    output_hold();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("eeprom_cov_test: all passed\n");
    return 0;
}
