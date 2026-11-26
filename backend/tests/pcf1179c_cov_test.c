/*
 * pcf1179c_cov_test (plan-68-R): PCF1179C clock cases pcf1179c_test does not
 * drive (datasheets/lcd-drivers/pcf1179c.pdf; display_clock.c header), on its
 * bench (copied) wired to the GENERIC-LCD-CLOCK-1:2 glass:
 *
 *   a reset (both buttons released together) restarts the 1 Hz count: the
 *   tick the old count had pending does not step the new one;
 *   both buttons held is the segment test, in which nothing steps: S1's
 *   repeats while S2 is also down do not count (DATA low: no reset after);
 *   a face whose hour-tens digit has fewer glyphs than the hour needs shows
 *   that digit blank (fixture R68-PCF1179C, tests/fixtures/model_cov: the
 *   PCF1179C with hour-tens glyphs blank and 1 only).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "parts.h"
#include "read_file.h"
#include "sim.h"

static int failures;
static PartLibrary *library;

static void check(int ok, const char *what) {
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

typedef struct {
    size_t count;
} PinCount;

static size_t drv_pins(const void *config) {
    return ((const PinCount *)config)->count;
}

static void *drv_create(const void *config) {
    PinCount *c = malloc(sizeof *c);

    *c = *(const PinCount *)config;
    return c;
}

static const SimComponentType DRIVER = {"tb", drv_pins, drv_create, free, NULL, NULL, NULL,
                                        NULL, NULL, NULL, NULL, NULL};

typedef struct {
    Sim *sim;
    SimInstance *chip, *glass, *drv;
    const Part *part, *glass_part;
    PinCount pins;
} Bench;

static const SimTime US = 1000000;
static const SimTime MS = 1000000000ull;
static const SimTime SEC = 1000000000000ull;

static size_t pin_of(const Part *part, const char *name) {
    size_t i;

    for (i = 0; i < part->pin_count; i++) {
        if (strcmp(part->pins[i].name, name) == 0) {
            return i;
        }
    }
    return SIZE_MAX;
}

static size_t bpin(const Bench *b, const char *name) {
    size_t i = pin_of(b->part, name);

    if (i == SIZE_MAX) {
        fprintf(stderr, "no pin %s\n", name);
        exit(2);
    }
    return i;
}

static void set(Bench *b, const char *name, LogicValue v) {
    sim_schedule_drive(b->sim, b->drv, bpin(b, name), v, DRIVE_STRONG, 0);
}

static void release(Bench *b, const char *name) {
    set(b, name, LOGIC_Z);
}

static LogicValue get(const Bench *b, const char *name) {
    return sim_read_pin(b->sim, b->chip, bpin(b, name));
}

static void run(Bench *b, SimTime ps) {
    if (!sim_run_until(b->sim, sim_now(b->sim) + ps)) {
        fprintf(stderr, "sim stopped: %s\n", sim_error(b->sim));
        exit(2);
    }
}

/* The chip's pins wired by name to the glass's (BP1, BP2 and the 13 segment pins), the rest to the testbench. */
static Bench *bench(const char *part, int mode_high, int mode_open) {
    Bench *b = calloc(1, sizeof *b);
    size_t i;

    b->part = part_library_find(library, part);
    b->glass_part = part_library_find(library, "GENERIC-LCD-CLOCK-1:2");
    if (b->part == NULL || b->glass_part == NULL) {
        fprintf(stderr, "missing part\n");
        exit(2);
    }
    b->pins.count = b->part->pin_count;
    b->sim = sim_create();
    b->chip = sim_add_instance(b->sim, kind_model(b->part), "U1", b->part);
    b->drv = sim_add_instance(b->sim, &DRIVER, "TB", &b->pins);
    b->glass = sim_add_instance(b->sim, kind_model(b->glass_part), "G1", b->glass_part);
    for (i = 0; i < b->part->pin_count; i++) {
        SimNet *net = sim_add_net(b->sim, b->part->pins[i].name);
        size_t k = pin_of(b->glass_part, b->part->pins[i].name);

        sim_connect(b->sim, net, b->chip, i);
        sim_connect(b->sim, net, b->drv, i);
        if (k != SIZE_MAX) {
            sim_connect(b->sim, net, b->glass, k);
        }
    }
    /* straps first: MODE, ENABLE high, TS and FLASH low */
    if (!mode_open) {
        set(b, "MODE", mode_high ? LOGIC_1 : LOGIC_0);
    }
    set(b, "ENABLE", LOGIC_1);
    set(b, "TS", LOGIC_1);
    set(b, "FLASH", LOGIC_0);
    if (!sim_start(b->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(b->sim));
        exit(2);
    }
    run(b, 1 * US);
    return b;
}

static void close_bench(Bench *b) {
    sim_destroy(b->sim);
    free(b);
}

/* ---- Reading the glass: cells in file order -------------------------------------------------- */

enum { D1 = 0, D2 = 6, D3 = 13, D4 = 20, COLON = 27, AM = 29, PM = 30, CELLS = 31 };

static unsigned char lit[CELLS + 8];

/* Reads the glass 20 ms from now: the frame is 15.625 ms, so the last 16 ms hold both slots of one
   stable face (a change lands within a step of a tick, never in the window). */
static int read_glass(Bench *b) {
    const SimComponentType *type = sim_instance_type(b->glass);
    size_t n;

    run(b, 20 * MS);
    n = type->lights(b->sim, b->glass, sim_instance_state(b->glass), sim_now(b->sim) - 16000 * US,
                            lit, sizeof lit);

    return n == CELLS;
}

/* Segments a-g (bit 0-6) of a digit; digit 1 has no f (its cells are a b c d e g). */
static unsigned seg_bits(int digit) {
    unsigned s = 0, j;

    if (digit == 1) {
        static const unsigned pos[6] = {0, 1, 2, 3, 4, 6};

        for (j = 0; j < 6; j++) {
            s |= (unsigned)(lit[D1 + j] != 0) << pos[j];
        }
        return s;
    }
    for (j = 0; j < 7; j++) {
        s |= (unsigned)(lit[(digit == 2 ? D2 : digit == 3 ? D3 : D4) + j] != 0) << j;
    }
    return s;
}

static const unsigned FONT[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

/* The time the glass shows, h * 100 + m; -1 when a digit is no number. The hour-tens digit is blank, 1
   (b, c) or 2 (a b d e g). */
static int shown(Bench *b) {
    unsigned h1, h2, m1, m2;
    int tens;

    if (!read_glass(b)) {
        return -2;
    }
    h1 = seg_bits(1);
    h2 = seg_bits(2);
    m1 = seg_bits(3);
    m2 = seg_bits(4);
    tens = h1 == 0 ? 0 : h1 == 0x06 ? 1 : h1 == 0x5B ? 2 : -1;
    {
        int a = -1, c = -1, d = -1, i;

        for (i = 0; i < 10; i++) {
            a = h2 == FONT[i] ? i : a;
            c = m1 == FONT[i] ? i : c;
            d = m2 == FONT[i] ? i : d;
        }
        if (tens < 0 || a < 0 || c < 0 || d < 0 || c > 5) {
            return -1;
        }
        return (tens * 10 + a) * 100 + c * 10 + d;
    }
}

static int colon_on(void) {
    return lit[COLON] && lit[COLON + 1];
}

static int all_lit(void) {
    size_t i;

    for (i = 0; i < CELLS; i++) {
        if (!lit[i]) {
            return 0;
        }
    }
    return 1;
}

/* n presses of a button (a press is 100 ms low, 10 ms released). */
static void press(Bench *b, const char *pin, int n) {
    int i;

    for (i = 0; i < n; i++) {
        set(b, pin, LOGIC_0);
        run(b, 100 * MS);
        release(b, pin);
        run(b, 10 * MS);
    }
}

/* ---- Tests ------------------------------------------------------------------------------------ */

static void reset_drops_the_old_tick(void) {
    Bench *b = bench("PCF1179CT,118", 0, 0);

    set(b, "FLASH", LOGIC_1);              /* the colon shows the seconds' parity: on in an odd second */
    run(b, 300 * MS);
    set(b, "S1", LOGIC_0);
    run(b, 50 * MS);
    set(b, "S2", LOGIC_0);
    run(b, 200 * MS);                      /* both down: the segment test */
    release(b, "S1");
    release(b, "S2");                      /* at ~0.55 s: a reset, a new 1 s count from here */
    run(b, 600 * MS);                      /* ~1.15 s: the old count's tick (1.0 s) is past */
    read_glass(b);
    check(!colon_on(), "after a reset the old pending tick does not count: second 0 at 1.15 s (colon off)");
    run(b, 700 * MS);                      /* ~1.85 s: the new count's first tick (1.55 s) is past */
    read_glass(b);
    check(colon_on(), "... the new count's own tick does: second 1 at 1.85 s (colon on)");
    close_bench(b);
}

static void test_mode_does_not_step(void) {
    Bench *b = bench("PCF1179CT,118", 0, 0);
    char what[96];
    int t;

    set(b, "DATA", LOGIC_0);               /* no reset when both are released */
    set(b, "S1", LOGIC_0);
    run(b, 100 * MS);                      /* S1 debounced: hours + 1, its first repeat due at ~1.07 s */
    set(b, "S2", LOGIC_0);
    run(b, 3 * SEC);                       /* both held through S1's repeats */
    release(b, "S1");
    release(b, "S2");
    run(b, 100 * MS);
    t = shown(b);
    snprintf(what, sizeof what, "both held 3 s: S1's repeats did not step, still 1:00 (shows %d)", t);
    check(t == 100, what);
    close_bench(b);
}

static void fewer_glyphs(void) {
    Bench *b = bench("R68-PCF1179C", 0, 0);
    char what[96];
    int t;

    press(b, "S1", 20);
    t = shown(b);
    snprintf(what, sizeof what, "hour-tens glyphs blank and 1 only: 20:00 shows a blank tens digit, 0:00 (shows %d)", t);
    check(t == 0, what);
    press(b, "S1", 1);
    t = shown(b);
    check(t == 100, "... and 21:00 shows 1:00 (units 1)");
    close_bench(b);
}

int main(void) {
    char *text = read_file(GOLDEN_DIR "/../fixtures/model_cov/pcf1179c.json");
    char *error = NULL;

    setvbuf(stdout, NULL, _IOLBF, 0);   /* the leak check ends with _exit: keep the lines */
    library = part_library_create();
    if (text == NULL || load_parts_dir(library, PARTS_DIR) != 0 ||
        load_parts(library, text, "pcf1179c.json", &error) != 0) {
        printf("FAIL loading the parts: %s\n", error != NULL ? error : "");
        return 1;
    }
    free(text);
    reset_drops_the_old_tick();
    test_mode_does_not_step();
    fewer_glyphs();
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("pcf1179c_cov_test: all passed\n");
    return 0;
}
