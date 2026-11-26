/*
 * sram_cov_test (plan-68-R): SRAM write cases sram_test does not drive, on
 * the model's rules (sram.c header: a write takes data and address at its
 * end; a violation stores X; an unknown control or address makes whatever it
 * may have hit X). Fixture parts (tests/fixtures/model_cov/sram.json), loaded
 * next to the bundled parts:
 *
 *   R68-SRAM-TWR  the AS6C6264-55 with a write recovery tWR = 10 ns and a
 *                 data hold tDH = 5 ns (the bundled grade gives neither)
 *   R68-SRAM-1BE  the AS6C2016-55 (16 data bits) with one byte enable, LB#
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

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

#define NS 1000ull

static const Part *part;

static Bench *open_bench(const char *name) {
    Bench *b;

    part = part_library_find(library, name);
    if (part == NULL) {
        printf("FAIL: no part %s\n", name);
        exit(1);
    }
    b = bench_create(part);
    bench_set(b, "CE#", LOGIC_0);
    if (bench_has_pin(b, "CE2")) {
        bench_set(b, "CE2", LOGIC_1);
    }
    bench_set(b, "OE#", LOGIC_1);
    bench_set(b, "WE#", LOGIC_1);
    bench_run(b, 100 * NS);
    return b;
}

static void set_address(Bench *b, unsigned address) {
    unsigned i;
    char name[8];

    for (i = 0; i < 24; i++) {
        snprintf(name, sizeof name, "A%u", i);
        if (bench_has_pin(b, name)) {
            bench_set(b, name, (address >> i) & 1u ? LOGIC_1 : LOGIC_0);
        }
    }
}

static void set_data(Bench *b, unsigned bits, unsigned value) {
    unsigned i;
    char name[8];

    for (i = 0; i < bits; i++) {
        snprintf(name, sizeof name, "DQ%u", i);
        bench_set(b, name, (value >> i) & 1u ? LOGIC_1 : LOGIC_0);
    }
}

/* Address and data set, WE# low 50 ns (tWP 45, tDW 25 met), WE# high; returns at the rise. */
static void write_word(Bench *b, unsigned bits, unsigned address, unsigned value) {
    set_address(b, address);
    set_data(b, bits, value);
    bench_run(b, 10 * NS);
    bench_set(b, "WE#", LOGIC_0);
    bench_run(b, 50 * NS);
    bench_set(b, "WE#", LOGIC_1);
}

/* The stored word: its value, and its known bits in *known. */
static unsigned word(Bench *b, unsigned address, unsigned *known) {
    uint32_t value = 0, bits = 0;

    kind_model(part)->memory_read(bench_state(b), address, 1, &value, &bits);
    *known = bits;
    return value & bits;
}

static void unknown_address(void) {
    Bench *b = open_bench("R68-SRAM-TWR");
    unsigned k0, k1, k2;
    unsigned v2;

    write_word(b, 8, 0, 0x11);
    bench_run(b, 100 * NS);
    write_word(b, 8, 1, 0x22);
    bench_run(b, 100 * NS);
    write_word(b, 8, 2, 0x33);
    bench_run(b, 100 * NS);
    /* A0 unknown at the end of a write to 0 or 1: both may have been hit */
    set_address(b, 0);
    set_data(b, 8, 0x44);
    bench_set(b, "A0", LOGIC_X);
    bench_run(b, 10 * NS);
    bench_set(b, "WE#", LOGIC_0);
    bench_run(b, 50 * NS);
    bench_set(b, "WE#", LOGIC_1);
    bench_run(b, 100 * NS);
    word(b, 0, &k0);
    word(b, 1, &k1);
    v2 = word(b, 2, &k2);
    expect(k0 == 0 && k1 == 0, "a write with A0 unknown makes words 0 and 1 (both it may hit) X");
    expect(v2 == 0x33 && k2 == 0xFF, "... and leaves word 2 (A1 known 1) as written, 33h");
    bench_destroy(b);
}

static void write_recovery(void) {
    Bench *b = open_bench("R68-SRAM-TWR");
    unsigned k5, k6, v7, k7;

    write_word(b, 8, 6, 0x66);
    bench_run(b, 100 * NS);
    write_word(b, 8, 5, 0x55);
    bench_run(b, 2 * NS);                  /* the address moves 2 ns after WE# rose: inside tWR */
    set_address(b, 6);
    bench_run(b, 100 * NS);
    word(b, 5, &k5);
    word(b, 6, &k6);
    expect(k5 == 0, "tWR: the address changed 2 ns after the write ended (tWR 10 ns): word 5 is X");
    expect(k6 == 0, "tWR: ... and the word it moved to, 6, is X");
    write_word(b, 8, 7, 0x77);
    bench_run(b, 20 * NS);                 /* past tWR */
    set_address(b, 6);
    bench_run(b, 100 * NS);
    v7 = word(b, 7, &k7);
    expect(v7 == 0x77 && k7 == 0xFF, "tWR met: an address change 20 ns after the write keeps word 7 (77h)");
    bench_destroy(b);
}

static void data_hold(void) {
    Bench *b = open_bench("R68-SRAM-TWR");
    unsigned v, k;

    write_word(b, 8, 3, 0x0F);
    bench_run(b, 2 * NS);                  /* DQ0 moves 2 ns after WE# rose: inside tDH */
    bench_set(b, "DQ0", LOGIC_0);
    bench_run(b, 100 * NS);
    v = word(b, 3, &k);
    expect(k == 0xFE && v == 0x0E, "tDH: DQ0 changed 2 ns after the write (tDH 5 ns): bit 0 X, the rest 0Eh");
    write_word(b, 8, 4, 0x0F);
    bench_run(b, 20 * NS);
    bench_set(b, "DQ0", LOGIC_0);
    bench_run(b, 100 * NS);
    v = word(b, 4, &k);
    expect(k == 0xFF && v == 0x0F, "tDH met: DQ0 changed 20 ns after the write keeps 0Fh");
    bench_destroy(b);
}

static void write_enable_unknown(void) {
    Bench *b = open_bench("R68-SRAM-TWR");
    unsigned v, k;

    write_word(b, 8, 9, 0x99);
    bench_run(b, 100 * NS);
    set_address(b, 9);
    set_data(b, 8, 0x5A);
    bench_run(b, 10 * NS);
    bench_set(b, "WE#", LOGIC_0);
    bench_run(b, 50 * NS);
    bench_set(b, "WE#", LOGIC_X);          /* the write's end is unknown */
    bench_run(b, 50 * NS);
    word(b, 9, &k);
    expect(k == 0, "WE# going from low to X in a write makes the word X");
    bench_set(b, "WE#", LOGIC_1);
    bench_run(b, 100 * NS);
    write_word(b, 8, 10, 0xA5);
    bench_run(b, 100 * NS);
    v = word(b, 10, &k);
    expect(v == 0xA5 && k == 0xFF, "the next clean write (word 10, A5h) is stored");
    bench_destroy(b);
}

static void one_byte_enable(void) {
    Bench *b = open_bench("R68-SRAM-1BE");
    unsigned v, k;

    bench_set(b, "LB#", LOGIC_0);
    write_word(b, 16, 2, 0xABCD);
    bench_run(b, 100 * NS);
    v = word(b, 2, &k);
    expect(v == 0xABCD && k == 0xFFFF, "one byte enable: LB# low writes all 16 bits (one lane), ABCDh");
    bench_set(b, "LB#", LOGIC_1);
    write_word(b, 16, 2, 0x1234);
    bench_run(b, 100 * NS);
    v = word(b, 2, &k);
    expect(v == 0xABCD && k == 0xFFFF, "LB# high: the write is blocked for all 16 bits");
    bench_destroy(b);
}

int main(void) {
    char *text = read_file(GOLDEN_DIR "/../fixtures/model_cov/sram.json");
    char *error = NULL;

    setvbuf(stdout, NULL, _IOLBF, 0);   /* the leak check ends with _exit: keep the lines */
    library = part_library_create();
    if (library == NULL || text == NULL || load_parts_dir(library, PARTS_DIR) != 0 ||
        load_parts(library, text, "sram.json", &error) != 0) {
        printf("FAIL: cannot load the parts: %s\n", error != NULL ? error : "");
        return 1;
    }
    free(text);
    unknown_address();
    write_recovery();
    data_hold();
    write_enable_unknown();
    one_byte_enable();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("sram_cov_test: all passed\n");
    return 0;
}
