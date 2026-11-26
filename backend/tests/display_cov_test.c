/*
 * display_cov_test (plan-68-R): the light_time hook (sim.h: each light's
 * cumulative on-time since start, at sim_now) of the nixie, dekatron,
 * LCD-glass, 7-segment and relay models, on bundled parts in a bench
 * (tests/support/bench.h):
 *
 *   IN-14          a numeral glows while the anode reads 1 and its cathode 0
 *   GC10B          the glow starts on cathode 0 when the anode goes high
 *   LCD-S401C39TR  a static cell is dark while its segment equals COM
 *   LTC-4727JR     common cathode: an LED is on while its segment pin is 1
 *                  and its digit's common 0
 *   ACT512         one light per channel, on while its armature is operated
 *                  (1 ms after the coil is driven: relay.c header)
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

#define US 1000000ull

static Bench *bench_of(const char *name, const Part **part) {
    *part = part_library_find(library, name);
    if (*part == NULL) {
        printf("FAIL: no part %s\n", name);
        exit(1);
    }
    return bench_create(*part);
}

/* The model's light_time at the bench's now; the hook ignores the instance. */
static size_t times(Bench *b, const Part *part, uint64_t *out, size_t cap) {
    const SimComponentType *type = kind_model(part);

    if (type->light_time == NULL) {
        return (size_t)-1;
    }
    return type->light_time(bench_sim(b), NULL, bench_state(b), out, cap);
}

static void nixie(void) {
    const Part *part;
    Bench *b = bench_of("IN-14", &part);
    uint64_t t[16] = {0};
    size_t n;

    bench_set(b, "A", LOGIC_0);
    bench_set(b, "K0", LOGIC_0);
    bench_set(b, "K1", LOGIC_1);
    bench_run(b, 10 * US);
    bench_set(b, "A", LOGIC_1);
    bench_run(b, 30 * US);
    bench_set(b, "A", LOGIC_0);
    bench_run(b, 5 * US);
    n = times(b, part, t, 16);
    /* the cathodes in the device's order: K0 "0" first, then K1 "1" */
    expect(n == 12, "IN-14: light_time reports its 12 glyphs");
    expect(t[0] >= 29 * US && t[0] <= 31 * US, "IN-14: \"0\" was on for the 30 us the anode was high");
    expect(t[1] == 0, "IN-14: \"1\" (cathode high) never glowed");
    expect(times(b, part, t, 1) == 12 && t[0] >= 29 * US, "IN-14: cap 1 writes only the first glyph");
    bench_destroy(b);
}

static void dekatron(void) {
    const Part *part;
    Bench *b = bench_of("GC10B", &part);
    uint64_t t[10] = {0};
    size_t n, p;
    uint64_t others = 0;

    bench_set(b, "A", LOGIC_0);
    bench_set(b, "G1", LOGIC_1);
    bench_set(b, "G2", LOGIC_1);
    bench_run(b, 10 * US);
    bench_set(b, "A", LOGIC_1);
    bench_run(b, 20 * US);
    n = times(b, part, t, 10);
    for (p = 1; p < 10; p++) {
        others += t[p];
    }
    expect(n == 10, "GC10B: light_time reports its 10 positions");
    expect(t[0] >= 19 * US && t[0] <= 21 * US && others == 0,
           "GC10B: the glow sat on position 0 for the 20 us since the anode rose; no other position lit");
    bench_destroy(b);
}

static void lcd(void) {
    const Part *part;
    Bench *b = bench_of("LCD-S401C39TR", &part);
    uint64_t t[64] = {0};
    size_t n;

    bench_set(b, "COM", LOGIC_0);
    bench_set(b, "1A", LOGIC_0);
    bench_set(b, "1B", LOGIC_1);
    bench_run(b, 40 * US);
    bench_set(b, "COM", LOGIC_1);
    bench_run(b, 10 * US);
    n = times(b, part, t, 64);
    /* cells in the device's order: digit 1 A, then digit 1 B */
    expect(n == part->display->led_count && n >= 2, "LCD-S401C39TR: light_time reports every cell");
    expect(t[0] >= 9 * US && t[0] <= 11 * US,
           "LCD-S401C39TR: 1A was on only the 10 us COM differed from it");
    expect(t[1] >= 39 * US && t[1] <= 41 * US, "LCD-S401C39TR: 1B was on the 40 us COM was 0");
    bench_destroy(b);
}

static void seven_segment(void) {
    const Part *part;
    Bench *b = bench_of("LTC-4727JR", &part);
    uint64_t t[40] = {0};
    size_t n;

    bench_set(b, "CC1", LOGIC_1);
    bench_set(b, "CC2", LOGIC_1);
    bench_set(b, "CC3", LOGIC_1);
    bench_set(b, "CC4", LOGIC_1);
    bench_set(b, "A/L1", LOGIC_1);
    bench_set(b, "B/L2", LOGIC_0);
    bench_run(b, 10 * US);
    bench_set(b, "CC1", LOGIC_0);           /* digit 1 on: its A lights */
    bench_run(b, 25 * US);
    n = times(b, part, t, 40);
    /* the LEDs in the device's order: digit 1 A, then digit 1 B */
    expect(n == part->display->led_count && n >= 2, "LTC-4727JR: light_time reports every LED");
    expect(t[0] >= 24 * US && t[0] <= 26 * US, "LTC-4727JR: digit 1 A was on the 25 us CC1 was low");
    expect(t[1] == 0, "LTC-4727JR: digit 1 B (its segment pin low) never lit");
    bench_destroy(b);
}

static void relay(void) {
    const Part *part;
    Bench *b = bench_of("ACT512", &part);
    uint64_t t[4] = {0};
    size_t n;

    bench_set(b, "A1", LOGIC_0);
    bench_set(b, "A2", LOGIC_0);
    bench_set(b, "A3", LOGIC_0);
    bench_set(b, "A4", LOGIC_0);
    bench_run(b, 2000 * US);
    bench_set(b, "A1", LOGIC_1);            /* coil 1 driven forward */
    bench_run(b, 5000 * US);
    n = times(b, part, t, 4);
    expect(n == 2, "ACT512: light_time reports its 2 channels");
    expect(t[0] >= 3990 * US && t[0] <= 4010 * US,
           "ACT512: channel 1 was operated 4 ms (5 ms driven less the 1 ms armature delay)");
    expect(t[1] == 0, "ACT512: channel 2 (coil undriven) never operated");
    expect(times(b, part, t, 1) == 2, "ACT512: cap 1 still counts both channels");
    bench_destroy(b);
}

int main(void) {
    setvbuf(stdout, NULL, _IOLBF, 0);   /* the leak check ends with _exit: keep the lines */
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    nixie();
    dekatron();
    lcd();
    seven_segment();
    relay();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("display_cov_test: all passed\n");
    return 0;
}
