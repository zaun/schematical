/*
 * pcf1179c_test (plan-56-F): the PCF1179C 4-digit duplex LCD car clock (datasheets/lcd-drivers/pcf1179c.pdf), a
 * bus-less lcddriver with the `clock` display driver, wired pin for pin to the GENERIC-LCD-CLOCK-1:2 glass:
 * power-on 1:00 AM (MODE high) / 0:00 (MODE low), the 1 Hz counters, the colon (FLASH), AM/PM, the S1 / S2
 * buttons (debounce, first repeat after 1 s, 4 a second, ENABLE), S2 zeroing the seconds, both buttons =
 * every segment on and a reset on release (not while DATA is low), TS low (counters stopped, 64 x faster
 * buttons), and the duplex waveforms on the real pins.
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

/* ---- Tests ------------------------------------------------------------------------------------ */

static void pins_and_power_on(void) {
    const Part *p = part_library_find(library, "PCF1179CT,118");
    Bench *b;
    char what[96];
    int t;

    check(p != NULL && p->pin_count == 28, "PCF1179CT,118 has 28 pins (SO28)");
    check(p != NULL && pin_of(p, "BP1") != SIZE_MAX && pin_of(p, "AM/PM") != SIZE_MAX &&
              strcmp(p->pins[pin_of(p, "BP1")].designator, "28") == 0 && strcmp(p->pins[pin_of(p, "VDD")].designator, "10") == 0,
          "pins as p.4 Table 4 (BP1 28, VDD 10)");
    check(p != NULL && p->vcc != NULL && p->vcc->min_mv == 3000 && p->vcc->max_mv == 6000, "supply 3-6 V (p.11)");
    b = bench("PCF1179CT,118", 1, 0);
    t = shown(b);
    snprintf(what, sizeof what, "MODE high: power-on 1:00 AM (shows %d)", t);
    check(t == 100 && lit[AM] && !lit[PM], what);
    close_bench(b);
    b = bench("PCF1179CT,118", 0, 0);
    t = shown(b);
    snprintf(what, sizeof what, "MODE low: power-on 0:00, 24-hour, no AM/PM (shows %d)", t);
    check(t == 0 && !lit[AM] && !lit[PM], what);
    check(colon_on(), "FLASH low: the colon is static (on)");
    close_bench(b);
    b = bench("PCF1179CT,118", 0, 1);
    t = shown(b);
    snprintf(what, sizeof what, "MODE open: 24-hour, 0:00 (shows %d)", t);
    check(t == 0, what);
    close_bench(b);
}

static void counting(void) {
    Bench *b = bench("PCF1179CT,118", 0, 0);
    char what[96];
    int t;

    run(b, 60 * SEC);
    t = shown(b);
    snprintf(what, sizeof what, "60 s later: 0:01 (shows %d)", t);
    check(t == 1, what);
    run(b, 59 * 60 * SEC);
    t = shown(b);
    snprintf(what, sizeof what, "1 h later: 1:00 (shows %d)", t);
    check(t == 100, what);
    close_bench(b);

    b = bench("PCF1179CT,118", 1, 0);
    run(b, 11 * 3600 * SEC);
    t = shown(b);
    snprintf(what, sizeof what, "12-hour: after 11 h, 12:00 PM (shows %d, AM %d PM %d)", t, lit[AM], lit[PM]);
    check(t == 1200 && !lit[AM] && lit[PM], what);
    run(b, 1 * 3600 * SEC);
    t = shown(b);
    snprintf(what, sizeof what, "12-hour: after 12 h, 1:00 PM (shows %d)", t);
    check(t == 100 && lit[PM], what);
    run(b, 11 * 3600 * SEC);
    t = shown(b);
    snprintf(what, sizeof what, "12-hour: after 23 h, 12:00 AM (shows %d)", t);
    check(t == 1200 && lit[AM] && !lit[PM], what);
    close_bench(b);

    b = bench("PCF1179CT,118", 0, 0);
    run(b, 20 * 3600 * SEC);
    t = shown(b);
    snprintf(what, sizeof what, "24-hour: after 20 h, 20:00 (hour tens 2 shape; shows %d)", t);
    check(t == 2000, what);
    run(b, 3 * 3600 * SEC + 59 * 60 * SEC + 60 * SEC);
    t = shown(b);
    snprintf(what, sizeof what, "24-hour: after 24 h, back to 0:00 (shows %d)", t);
    check(t == 0, what);
    close_bench(b);
}

static void colon(void) {
    Bench *b = bench("PCF1179CT,118", 0, 0);

    set(b, "FLASH", LOGIC_1);
    run(b, 500 * MS);
    read_glass(b);
    check(!colon_on(), "FLASH high: the colon is off in an even second");
    run(b, 1 * SEC);
    read_glass(b);
    check(colon_on(), "FLASH high: the colon is on in an odd second (1 Hz)");
    run(b, 1 * SEC);
    read_glass(b);
    check(!colon_on(), "FLASH high: off again the second after");
    close_bench(b);
}

static void buttons(void) {
    Bench *b = bench("PCF1179CT,118", 0, 0);
    char what[96];
    int t;

    run(b, 30 * SEC);                          /* 0:00:30 */
    set(b, "S2", LOGIC_0);
    run(b, 40 * MS);
    t = shown(b);
    check(t == 0, "S2 low for less than the debounce changes nothing");
    run(b, 40 * MS);
    t = shown(b);
    snprintf(what, sizeof what, "S2 low past the debounce: minutes + 1 (shows %d)", t);
    check(t == 1, what);
    run(b, 900 * MS);
    t = shown(b);
    check(t == 1, "S2 held 1 s: no auto-increment yet");
    run(b, 1200 * MS);                         /* 2.22 s after the press */
    t = shown(b);
    snprintf(what, sizeof what, "S2 held 2.2 s: 1 + 1 (after 1 s) + 4 a second (shows %d)", t);
    check(t == 6, what);
    release(b, "S2");
    run(b, 5 * SEC);
    t = shown(b);
    check(t == 6, "S2 released: the minutes stay");
    run(b, 30 * SEC);
    t = shown(b);
    snprintf(what, sizeof what, "S2 zeroed the seconds: 35 s after the press the minute is still 6 (shows %d)", t);
    check(t == 6, what);
    run(b, 30 * SEC);
    t = shown(b);
    snprintf(what, sizeof what, "... and 65 s after, 7 (shows %d)", t);
    check(t == 7, what);
    close_bench(b);

    b = bench("PCF1179CT,118", 0, 0);
    set(b, "S1", LOGIC_0);
    run(b, 100 * MS);
    t = shown(b);
    snprintf(what, sizeof what, "S1 low past the debounce: hours + 1 (shows %d)", t);
    check(t == 100, what);
    release(b, "S1");
    run(b, 100 * MS);
    set(b, "ENABLE", LOGIC_0);
    run(b, 10 * MS);
    set(b, "S1", LOGIC_0);
    run(b, 3 * SEC);
    t = shown(b);
    check(t == 100, "ENABLE low: S1 does nothing");
    release(b, "S1");
    close_bench(b);

    /* minute overflow does not carry into the hours */
    b = bench("PCF1179CT,118", 0, 0);
    set(b, "S2", LOGIC_0);
    run(b, 1 * SEC + 70 * MS);
    run(b, 15 * SEC);                          /* 60 steps through 59 -> 0 */
    release(b, "S2");
    t = shown(b);
    snprintf(what, sizeof what, "minute 59 -> 0 by S2 does not carry into the hours (shows %d)", t);
    check(t >= 0 && t < 100, what);
    close_bench(b);

    /* hours walk 1..12, PM flips passing 11 -> 12 */
    b = bench("PCF1179CT,118", 1, 0);
    {
        int i;

        for (i = 0; i < 11; i++) {
            set(b, "S1", LOGIC_0);
            run(b, 100 * MS);
            release(b, "S1");
            run(b, 10 * MS);
        }
    }
    t = shown(b);
    snprintf(what, sizeof what, "12-hour: 11 presses of S1 from 1:00 AM reach 12:00 PM (shows %d)", t);
    check(t == 1200 && lit[PM] && !lit[AM], what);
    close_bench(b);
}

static void segment_test_and_reset(void) {
    Bench *b = bench("PCF1179CT,118", 0, 0);
    char what[96];
    int t;

    run(b, 5 * 60 * SEC);                      /* 0:05 */
    set(b, "S1", LOGIC_0);
    set(b, "S2", LOGIC_0);
    run(b, 200 * MS);
    read_glass(b);
    check(all_lit(), "S1 and S2 both low: every segment is on");
    release(b, "S1");
    release(b, "S2");
    run(b, 200 * MS);
    t = shown(b);
    snprintf(what, sizeof what, "releasing both resets the clock to 0:00 (shows %d)", t);
    check(t == 0, what);
    close_bench(b);

    b = bench("PCF1179CT,118", 1, 0);
    run(b, 5 * 60 * SEC);
    set(b, "S1", LOGIC_0);
    set(b, "S2", LOGIC_0);
    run(b, 200 * MS);
    release(b, "S1");
    release(b, "S2");
    run(b, 200 * MS);
    t = shown(b);
    snprintf(what, sizeof what, "12-hour: the reset returns to 1:00 AM (shows %d)", t);
    check(t == 100 && lit[AM], what);
    close_bench(b);

    b = bench("PCF1179CT,118", 0, 0);
    run(b, 5 * 60 * SEC);
    set(b, "DATA", LOGIC_0);
    set(b, "S1", LOGIC_0);
    set(b, "S2", LOGIC_0);
    run(b, 200 * MS);
    read_glass(b);
    check(all_lit(), "DATA low: both buttons still light every segment");
    release(b, "S1");
    release(b, "S2");
    run(b, 200 * MS);
    t = shown(b);
    snprintf(what, sizeof what, "DATA low: no reset on release (shows %d)", t);
    check(t == 5, what);
    close_bench(b);

    b = bench("PCF1179CT,118", 0, 1);
    run(b, 60 * SEC);
    set(b, "S1", LOGIC_0);
    set(b, "S2", LOGIC_0);
    run(b, 200 * MS);
    release(b, "S1");
    release(b, "S2");
    run(b, 200 * MS);
    t = shown(b);
    snprintf(what, sizeof what, "MODE open: a reset flips 24 -> 12 hour, 1:00 AM (shows %d)", t);
    check(t == 100 && lit[AM] && !lit[PM], what);
    close_bench(b);
}

static void test_speed(void) {
    Bench *b = bench("PCF1179CT,118", 0, 0);
    char what[96];
    int t;

    set(b, "TS", LOGIC_0);
    run(b, 300 * SEC);
    t = shown(b);
    snprintf(what, sizeof what, "TS low: the counters are stopped (300 s later shows %d)", t);
    check(t == 0, what);
    set(b, "S2", LOGIC_0);
    run(b, 3 * MS);
    release(b, "S2");
    t = shown(b);
    snprintf(what, sizeof what, "TS low: the debounce is 64 x faster (1 ms; shows %d)", t);
    check(t == 1, what);
    set(b, "S2", LOGIC_0);
    run(b, 100 * MS);
    release(b, "S2");
    t = shown(b);
    snprintf(what, sizeof what, "TS low: first repeat after 1/64 s, then 64 x 4 a second (shows %d)", t);
    check(t > 10, what);
    close_bench(b);
}

static void waveforms(void) {
    Bench *b = bench("PCF1179CT,118", 0, 0);
    int bp1_driven = 0, bp2_driven = 0, both_driven = 0, i;

    for (i = 0; i < 64; i++) {
        LogicValue a = get(b, "BP1"), c = get(b, "BP2");

        bp1_driven += a == LOGIC_0 || a == LOGIC_1;
        bp2_driven += c == LOGIC_0 || c == LOGIC_1;
        both_driven += (a == LOGIC_0 || a == LOGIC_1) && (c == LOGIC_0 || c == LOGIC_1);
        run(b, 1 * MS);
    }
    check(bp1_driven > 0 && bp2_driven > 0 && both_driven == 0,
          "duplex drive: one backplane driven at a time, the other released");
    close_bench(b);
}

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL loading the parts\n");
        return 1;
    }
    pins_and_power_on();
    counting();
    colon();
    buttons();
    segment_test_and_reset();
    test_speed();
    waveforms();
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("pcf1179c_test: all passed\n");
    return 0;
}
