/*
 * icm7211_test (plan-56-C): the ICM7211 (LCD) and ICM7212 (LED) four-digit decoder/drivers
 * (datasheets/led-drivers/icm7211-icm7212.pdf), all eight variants (hexadecimal / Code B font,
 * multiplexed BCD input / microprocessor input), driven through the real bus pins.
 *
 *   ICM7211, 7211A    multiplexed input (4 data bits, 4 digit strobes): a strobe stores the nibble,
 *                     decoded, in the digit's latch; wired to the real GENERIC-LCD-4D-STATIC glass
 *   ICM7211M, 7211AM  microprocessor input (2-bit digit select, two active-low chip selects)
 *   ICM7212 family    the same inputs; 28 open-drain segment outputs (lit = 0, dark = released)
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

static LogicValue get(const Bench *b, const char *name) {
    return sim_read_pin(b->sim, b->chip, bpin(b, name));
}

static void run(Bench *b, SimTime ps) {
    if (!sim_run_until(b->sim, sim_now(b->sim) + ps)) {
        fprintf(stderr, "sim stopped: %s\n", sim_error(b->sim));
        exit(2);
    }
}

/* The glass pin of a chip pin: BP -> COM, segment "B3" -> "S3B"; NULL for every other pin. */
static int glass_name(const char *chip, char *out, size_t cap) {
    if (strcmp(chip, "BP") == 0) {
        snprintf(out, cap, "COM");
        return 1;
    }
    if (strlen(chip) == 2 && chip[0] >= 'A' && chip[0] <= 'G' && chip[1] >= '1' && chip[1] <= '4') {
        snprintf(out, cap, "S%c%c", chip[1], chip[0]);
        return 1;
    }
    return 0;
}

static Bench *bench(const char *part, const char *glass) {
    Bench *b = calloc(1, sizeof *b);
    size_t i;

    b->part = part_library_find(library, part);
    if (b->part == NULL) {
        fprintf(stderr, "no part %s\n", part);
        exit(2);
    }
    b->pins.count = b->part->pin_count;
    b->sim = sim_create();
    b->chip = sim_add_instance(b->sim, kind_model(b->part), "U1", b->part);
    b->drv = sim_add_instance(b->sim, &DRIVER, "TB", &b->pins);
    if (glass != NULL) {
        b->glass_part = part_library_find(library, glass);
        b->glass = sim_add_instance(b->sim, kind_model(b->glass_part), "G1", b->glass_part);
    }
    for (i = 0; i < b->part->pin_count; i++) {
        SimNet *net = sim_add_net(b->sim, b->part->pins[i].name);
        char g[16];

        sim_connect(b->sim, net, b->chip, i);
        sim_connect(b->sim, net, b->drv, i);
        if (glass != NULL && glass_name(b->part->pins[i].name, g, sizeof g)) {
            size_t k = pin_of(b->glass_part, g);

            if (k == SIZE_MAX) {
                fprintf(stderr, "glass has no %s\n", g);
                exit(2);
            }
            sim_connect(b->sim, net, b->glass, k);
        }
    }
    if (!sim_start(b->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(b->sim));
        exit(2);
    }
    return b;
}

static void close_bench(Bench *b) {
    sim_destroy(b->sim);
    free(b);
}

static LogicValue lv(unsigned bit) {
    return bit ? LOGIC_1 : LOGIC_0;
}

static void nibble(Bench *b, unsigned v) {
    set(b, "DIN0", lv(v & 1u));
    set(b, "DIN1", lv((v >> 1) & 1u));
    set(b, "DIN2", lv((v >> 2) & 1u));
    set(b, "DIN3", lv((v >> 3) & 1u));
}

/* ---- Inputs ---------------------------------------------------------------------------------- */

/* Multiplexed input: data valid, the strobes (bit 0 = D1 ... bit 3 = D4) high for 5 us, low again. */
static void strobe(Bench *b, unsigned mask, unsigned value) {
    static const char *const S[4] = {"STB1", "STB2", "STB3", "STB4"};
    unsigned i;

    nibble(b, value);
    run(b, 2 * US);
    for (i = 0; i < 4; i++) {
        set(b, S[i], lv((mask >> i) & 1u));
    }
    run(b, 5 * US);
    for (i = 0; i < 4; i++) {
        set(b, S[i], LOGIC_0);
    }
    run(b, 2 * US);
}

static void mux_idle(Bench *b) {
    strobe(b, 0, 0);
}

/* Microprocessor input: digit select code (DS2 DS1), data, both chip selects low, then one rising. */
static void mpu_write(Bench *b, unsigned code, unsigned value, int rise_cs2) {
    set(b, "DS1", lv(code & 1u));
    set(b, "DS2", lv((code >> 1) & 1u));
    nibble(b, value);
    set(b, "CS1", LOGIC_0);
    set(b, "CS2", LOGIC_0);
    run(b, 2 * US);
    set(b, rise_cs2 ? "CS2" : "CS1", LOGIC_1);
    run(b, 2 * US);
    set(b, rise_cs2 ? "CS1" : "CS2", LOGIC_1);
    run(b, 2 * US);
}

static void mpu_idle(Bench *b) {
    set(b, "DS1", LOGIC_0);
    set(b, "DS2", LOGIC_0);
    nibble(b, 0);
    set(b, "CS1", LOGIC_1);
    set(b, "CS2", LOGIC_1);
    run(b, 10 * US);
}

/* ---- Fonts (segments a-g = bits 0-6), from Table 1 (p.10) ------------------------------------ */

static const unsigned HEX_FONT[16] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
                                      0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};
static const unsigned CODEB_FONT[16] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
                                        0x7F, 0x6F, 0x40, 0x79, 0x76, 0x38, 0x73, 0x00};

/* The glass: 4 digits x (A..G, DP) cells; the digit's segment bits (DP never lit). */
static unsigned glass_digit(Bench *b, unsigned digit) {
    const SimComponentType *type = sim_instance_type(b->glass);
    unsigned char values[64];
    size_t n = type->lights(b->sim, b->glass, sim_instance_state(b->glass), sim_now(b->sim) - 14000 * US,
                            values, sizeof values);
    unsigned seg = 0, i;

    if (n != 32) {
        return 0xFFFF;
    }
    for (i = 0; i < 8; i++) {
        if (values[digit * 8 + i]) {
            seg |= 1u << i;
        }
    }
    return seg;
}

/* The LED chip's lit segments of a digit: pin 0 means lit; anything but Z or 0 is a failure (0xFFFF). */
static unsigned led_digit(Bench *b, unsigned digit) {
    unsigned seg = 0, i;

    for (i = 0; i < 7; i++) {
        char name[4];
        LogicValue v;

        snprintf(name, sizeof name, "%c%u", 'A' + (int)i, digit + 1);
        v = get(b, name);
        if (v == LOGIC_0) {
            seg |= 1u << i;
        } else if (v != LOGIC_Z) {
            return 0xFFFF;
        }
    }
    return seg;
}

typedef unsigned (*DigitReader)(Bench *, unsigned);

/* ---- LCD ------------------------------------------------------------------------------------- */

static int all_digits(Bench *b, DigitReader rd, const unsigned *want, const unsigned *values) {
    unsigned d, ok = 1;

    for (d = 0; d < 4; d++) {
        if (rd(b, d) != want[values[d]]) {
            ok = 0;
        }
    }
    return (int)ok;
}

/* Every nibble value on every digit through the multiplexed input: four writes of rotating values. */
static void mux_font(const char *part, const char *glass, DigitReader rd, const unsigned *font, const char *label) {
    Bench *b = bench(part, glass);
    unsigned v, ok = 1;
    char text[160];

    mux_idle(b);
    run(b, 15000 * US);
    for (v = 0; v < 16; v++) {
        unsigned values[4], d;

        for (d = 0; d < 4; d++) {
            values[d] = (v + 5 * d) & 15u;
            strobe(b, 1u << d, values[d]);
        }
        run(b, 15000 * US);
        if (!all_digits(b, rd, font, values)) {
            ok = 0;
            printf("  %s value %u: %02X %02X %02X %02X\n", label, v, rd(b, 0), rd(b, 1), rd(b, 2), rd(b, 3));
        }
    }
    snprintf(text, sizeof text, "%s: all 16 codes on all four digits through the strobes decode as Table 1", label);
    check(ok, text);
    close_bench(b);
}

static void mpu_font(const char *part, const char *glass, DigitReader rd, const unsigned *font, const char *label) {
    static const unsigned DIGIT_OF_CODE[4] = {3, 2, 1, 0};   /* DS2 DS1 = 00 -> D4, 01 -> D3, 10 -> D2, 11 -> D1 */
    Bench *b = bench(part, glass);
    unsigned v, ok = 1;
    char text[160];

    mpu_idle(b);
    run(b, 15000 * US);
    for (v = 0; v < 16; v++) {
        unsigned values[4], code;

        for (code = 0; code < 4; code++) {
            unsigned d = DIGIT_OF_CODE[code];

            values[d] = (v + 3 * code) & 15u;
            mpu_write(b, code, values[d], (int)(code & 1u));
        }
        run(b, 15000 * US);
        if (!all_digits(b, rd, font, values)) {
            ok = 0;
            printf("  %s value %u: %02X %02X %02X %02X\n", label, v, rd(b, 0), rd(b, 1), rd(b, 2), rd(b, 3));
        }
    }
    snprintf(text, sizeof text, "%s: all 16 codes on all four digits (DS2 DS1 = 00 is digit 4 ... 11 is digit 1) decode as Table 1", label);
    check(ok, text);
    close_bench(b);
}

static void mux_behaviour(const char *part, const char *glass, DigitReader rd, const unsigned *font, const char *label) {
    Bench *b = bench(part, glass);
    char text[200];
    int ok;

    mux_idle(b);
    strobe(b, 1, 1);      /* D1 = 1 */
    strobe(b, 2, 2);      /* D2 = 2 */
    strobe(b, 4, 3);      /* D3 = 3 */
    strobe(b, 8, 4);      /* D4 = 4 */
    run(b, 15000 * US);
    ok = rd(b, 0) == font[1] && rd(b, 1) == font[2] && rd(b, 2) == font[3] && rd(b, 3) == font[4];
    snprintf(text, sizeof text, "%s: D1..D4 strobes store 1, 2, 3, 4 in digits 1..4", label);
    check(ok, text);

    /* No strobe: data changes do nothing (truth table row 0000: no change) */
    nibble(b, 9);
    run(b, 10 * US);
    nibble(b, 6);
    run(b, 15000 * US);
    ok = rd(b, 0) == font[1] && rd(b, 1) == font[2] && rd(b, 2) == font[3] && rd(b, 3) == font[4];
    snprintf(text, sizeof text, "%s: with no strobe high the data inputs change nothing", label);
    check(ok, text);

    /* Transparent while the strobe is high: a data change during D2 high is stored, the other digits stay. */
    set(b, "STB2", LOGIC_1);
    run(b, 2 * US);
    nibble(b, 7);
    run(b, 2 * US);
    set(b, "STB2", LOGIC_0);
    run(b, 15000 * US);
    ok = rd(b, 0) == font[1] && rd(b, 1) == font[7] && rd(b, 2) == font[3] && rd(b, 3) == font[4];
    snprintf(text, sizeof text, "%s: a nibble that changes while D2 is high is the one stored in digit 2", label);
    check(ok, text);

    /* All four strobes: every latch */
    strobe(b, 15, 0xA);
    run(b, 15000 * US);
    ok = rd(b, 0) == font[10] && rd(b, 1) == font[10] && rd(b, 2) == font[10] && rd(b, 3) == font[10];
    snprintf(text, sizeof text, "%s: all four strobes high store the nibble in all four digits", label);
    check(ok, text);

    /* Two strobes (not in the truth table): nothing stored */
    strobe(b, 3, 5);
    run(b, 15000 * US);
    ok = rd(b, 0) == font[10] && rd(b, 1) == font[10];
    snprintf(text, sizeof text, "%s: a strobe combination the truth table does not list stores nothing", label);
    check(ok, text);
    close_bench(b);
}

static void mpu_behaviour(const char *part, const char *glass, DigitReader rd, const unsigned *font, const char *label) {
    Bench *b = bench(part, glass);
    char text[200];
    int ok;

    mpu_idle(b);
    mpu_write(b, 3, 1, 0);   /* digit 1 */
    mpu_write(b, 2, 2, 1);   /* digit 2, the other chip select rises */
    mpu_write(b, 1, 3, 0);   /* digit 3 */
    mpu_write(b, 0, 4, 1);   /* digit 4 */
    run(b, 15000 * US);
    ok = rd(b, 0) == font[1] && rd(b, 1) == font[2] && rd(b, 2) == font[3] && rd(b, 3) == font[4];
    snprintf(text, sizeof text, "%s: digit select 11, 10, 01, 00 store digits 1, 2, 3, 4 (either chip select ends the write)", label);
    check(ok, text);

    /* Nothing is stored while a chip select is high. */
    set(b, "CS1", LOGIC_1);
    set(b, "CS2", LOGIC_1);
    set(b, "DS1", LOGIC_1);
    set(b, "DS2", LOGIC_1);
    nibble(b, 9);
    run(b, 5 * US);
    set(b, "CS2", LOGIC_0);
    run(b, 5 * US);
    set(b, "CS2", LOGIC_1);
    run(b, 15000 * US);
    ok = rd(b, 0) == font[1];
    snprintf(text, sizeof text, "%s: CS2 pulsed while CS1 is high stores nothing", label);
    check(ok, text);
    set(b, "CS1", LOGIC_0);
    run(b, 5 * US);
    set(b, "CS1", LOGIC_1);
    run(b, 15000 * US);
    ok = rd(b, 0) == font[1];
    snprintf(text, sizeof text, "%s: CS1 pulsed while CS2 is high stores nothing", label);
    check(ok, text);

    /* The write happens when the select rises, not when it falls: still the old value while both are low. */
    set(b, "CS1", LOGIC_0);
    set(b, "CS2", LOGIC_0);
    run(b, 15000 * US);
    ok = rd(b, 0) == font[1];
    snprintf(text, sizeof text, "%s: nothing is stored until a chip select rises", label);
    check(ok, text);
    set(b, "CS1", LOGIC_1);
    run(b, 15000 * US);
    ok = rd(b, 0) == font[9];
    snprintf(text, sizeof text, "%s: the rising chip select stores the data seen at that time", label);
    check(ok, text);
    close_bench(b);
}

/* BP: a square wave at the datasheet's 150 Hz; an off segment follows BP, an on segment is its inverse. */
static void backplane(void) {
    Bench *b = bench("ICM7211IPL", "GENERIC-LCD-4D-STATIC");
    unsigned edges = 0, t, pair_ok = 1;
    LogicValue last = LOGIC_X;

    mux_idle(b);
    strobe(b, 1, 1);        /* digit 1 = "1": B and C on, A off */
    run(b, 10000 * US);
    for (t = 0; t < 20000; t += 50) {
        LogicValue bp = get(b, "BP"), on = get(b, "B1"), off = get(b, "A1");

        if (bp != LOGIC_0 && bp != LOGIC_1) {
            pair_ok = 0;
        }
        if (last != LOGIC_X && bp != last) {
            edges++;
        }
        last = bp;
        pair_ok &= on != bp && off == bp;
        run(b, 50 * US);
    }
    /* 20 ms at 150 Hz (6.67 ms period, an edge every 3.33 ms): 6 edges */
    check(pair_ok, "7211: BP is driven 0/1; a lit segment is the inverse of BP, a dark one equals BP");
    check(edges >= 5 && edges <= 7, "7211: BP changes level every 3.33 ms: 6 times in 20 ms (150 Hz)");
    close_bench(b);
}

static void pins(void) {
    const Part *p = part_library_find(library, "ICM7211IPL");
    const Part *q = part_library_find(library, "ICM7211AMIQ");
    const Part *l = part_library_find(library, "ICM7212IPL");
    const Part *m = part_library_find(library, "ICM7212AMIQ");
    size_t i;
    int ok;

    check(p != NULL && p->kind == PART_LCDDRIVER && l != NULL && l->kind == PART_LEDDRIVER, "kinds: 7211 lcddriver, 7212 leddriver");
#define NUM(PART, NAME, DES) (pin_of((PART), (NAME)) != SIZE_MAX && strcmp((PART)->pins[pin_of((PART), (NAME))].designator, (DES)) == 0)
    ok = NUM(p, "BP", "5") && NUM(p, "OSC", "36") && NUM(p, "STB1", "31") && NUM(p, "STB4", "34") && NUM(p, "DIN0", "27") &&
         NUM(p, "A1", "37") && NUM(p, "D1", "40") && NUM(p, "E1", "2") && NUM(p, "F4", "26") && NUM(p, "GND", "35") && NUM(p, "V+", "1");
    check(ok, "ICM7211IPL pins as p.1 / p.4 (DIP-40)");
    ok = NUM(q, "BP", "6") && NUM(q, "OSC", "40") && NUM(q, "DS1", "35") && NUM(q, "DS2", "36") && NUM(q, "CS1", "37") && NUM(q, "CS2", "38") &&
         NUM(q, "DIN0", "30") && NUM(q, "DIN3", "33") && NUM(q, "A1", "41") && NUM(q, "D1", "44") && NUM(q, "G4", "28") && NUM(q, "GND", "39") && NUM(q, "V+", "2");
    check(ok, "ICM7211AMIQ pins as p.4 (44-lead PLCC, microprocessor version)");
    ok = NUM(l, "BRT", "5") && NUM(l, "STB1", "31") && NUM(l, "GND", "35") && NUM(l, "A1", "37");
    check(ok, "ICM7212IPL pins as p.1 (BRT on pin 5)");
    ok = NUM(m, "BRT", "6") && NUM(m, "DS1", "35") && NUM(m, "CS2", "38") && NUM(m, "GND", "39") && pin_of(m, "BP") == SIZE_MAX;
    ok &= m->pins[pin_of(m, "CS1")].active_low != 0;
    check(ok, "ICM7212AMIQ pins as p.5 (BRT on pin 6, active-low chip selects)");
    ok = 0;
    for (i = 0; i < l->pin_count; i++) {
        ok += l->pins[i].role == PIN_GROUND;
    }
    check(ok == 2, "ICM7212IPL has its two GND pins (35, 36)");
    check(p->vcc != NULL && p->vcc->min_mv == 3000 && p->vcc->max_mv == 6000 && l->vcc->min_mv == 4000 && l->vcc->max_mv == 6000,
          "supply: 7211 3-6 V, 7212 4-6 V (p.2)");
    for (i = 0; i < 16; i++) {
        static const char *const NAMES[16] = {"ICM7211IPL", "ICM7211AIPL", "ICM7211MIPL", "ICM7211AMIPL", "ICM7212IPL", "ICM7212AIPL",
                                              "ICM7212MIPL", "ICM7212AMIPL", "ICM7211IQ", "ICM7211AIQ", "ICM7211MIQ", "ICM7211AMIQ",
                                              "ICM7212IQ", "ICM7212AIQ", "ICM7212MIQ", "ICM7212AMIQ"};

        if (part_library_find(library, NAMES[i]) == NULL) {
            printf("  missing %s\n", NAMES[i]);
            ok = 0;
        }
    }
    check(ok == 2, "all sixteen ordering-table names exist");
}

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL loading the parts\n");
        return 1;
    }
    pins();
    mux_font("ICM7211IPL", "GENERIC-LCD-4D-STATIC", glass_digit, HEX_FONT, "ICM7211 (hex, LCD)");
    mux_font("ICM7211AIQ", "GENERIC-LCD-4D-STATIC", glass_digit, CODEB_FONT, "ICM7211A (Code B, LCD, PLCC)");
    mpu_font("ICM7211MIPL", "GENERIC-LCD-4D-STATIC", glass_digit, HEX_FONT, "ICM7211M (hex, LCD)");
    mpu_font("ICM7211AMIQ", "GENERIC-LCD-4D-STATIC", glass_digit, CODEB_FONT, "ICM7211AM (Code B, LCD, PLCC)");
    mux_behaviour("ICM7211IPL", "GENERIC-LCD-4D-STATIC", glass_digit, HEX_FONT, "ICM7211");
    mpu_behaviour("ICM7211MIPL", "GENERIC-LCD-4D-STATIC", glass_digit, HEX_FONT, "ICM7211M");
    backplane();
    mux_font("ICM7212IPL", NULL, led_digit, HEX_FONT, "ICM7212 (hex, LED)");
    mux_font("ICM7212AIQ", NULL, led_digit, CODEB_FONT, "ICM7212A (Code B, LED, PLCC)");
    mpu_font("ICM7212MIPL", NULL, led_digit, HEX_FONT, "ICM7212M (hex, LED)");
    mpu_font("ICM7212AMIQ", NULL, led_digit, CODEB_FONT, "ICM7212AM (Code B, LED, PLCC)");
    mux_behaviour("ICM7212IPL", NULL, led_digit, HEX_FONT, "ICM7212");
    mpu_behaviour("ICM7212MIPL", NULL, led_digit, HEX_FONT, "ICM7212M");
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("icm7211_test: all passed\n");
    return 0;
}
