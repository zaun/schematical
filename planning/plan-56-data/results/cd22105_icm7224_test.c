/*
 * cd22105_icm7224_test (plan-56-F): the CD22105 / CD22105A four-digit LCD decoder-drivers
 * (datasheets/lcd-drivers/cd22105-harris.pdf) and the ICM7224 (LCD) / ICM7225 (LED) 4-1/2 digit
 * counter/decoder/drivers (datasheets/led-drivers/icm7224-icm7225-maxim.pdf, ...-harris.pdf), all
 * driven through the real bus pins.
 *
 *   CD22105, CD22105A  microprocessor input (2-bit digit select code, 2 active-low chip selects),
 *                      hexadecimal / decimal (Code B) font; wired to the real GENERIC-LCD-4D-STATIC glass
 *   ICM7224, ICM7225   COUNT (falling edge), COUNT INHIBIT, RESET, STORE, leading zero blanking in and
 *                      out, CARRY, the half digit; the 7224 drives the glass, the 7225 sinks its LEDs
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

/* The glass pin of a chip pin: BP -> COM, segment "B3" -> "S3B"; 0 for every other pin. */
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

/* ---- Fonts (segments a-g = bits 0-6) --------------------------------------------------------- */

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

/* ---- CD22105 --------------------------------------------------------------------------------- */

static void nibble(Bench *b, unsigned v) {
    set(b, "DIN0", lv(v & 1u));
    set(b, "DIN1", lv((v >> 1) & 1u));
    set(b, "DIN2", lv((v >> 2) & 1u));
    set(b, "DIN3", lv((v >> 3) & 1u));
}

/* Digit select code (DSC2 DSC1), data, both chip selects low, then one rising. */
static void mpu_write(Bench *b, unsigned code, unsigned value, int rise_cs2) {
    set(b, "DSC1", lv(code & 1u));
    set(b, "DSC2", lv((code >> 1) & 1u));
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
    set(b, "DSC1", LOGIC_0);
    set(b, "DSC2", LOGIC_0);
    nibble(b, 0);
    set(b, "CS1", LOGIC_1);
    set(b, "CS2", LOGIC_1);
    run(b, 10 * US);
}

static int all_digits(Bench *b, DigitReader rd, const unsigned *want, const unsigned *values) {
    unsigned d, ok = 1;

    for (d = 0; d < 4; d++) {
        if (rd(b, d) != want[values[d]]) {
            ok = 0;
        }
    }
    return (int)ok;
}

static void mpu_font(const char *part, const unsigned *font, const char *label) {
    static const unsigned DIGIT_OF_CODE[4] = {3, 2, 1, 0};   /* DSC2 DSC1 = 00 -> D4, 01 -> D3, 10 -> D2, 11 -> D1 */
    Bench *b = bench(part, "GENERIC-LCD-4D-STATIC");
    unsigned v, ok = 1;
    char text[200];

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
        if (!all_digits(b, glass_digit, font, values)) {
            ok = 0;
            printf("  %s value %u: %02X %02X %02X %02X\n", label, v, glass_digit(b, 0), glass_digit(b, 1),
                   glass_digit(b, 2), glass_digit(b, 3));
        }
    }
    snprintf(text, sizeof text, "%s: all 16 codes on all four digits (DSC2 DSC1 = 00 is digit 4 ... 11 is digit 1) decode as Table I", label);
    check(ok, text);
    close_bench(b);
}

static void mpu_behaviour(const char *part, const unsigned *font, const char *label) {
    DigitReader rd = glass_digit;
    Bench *b = bench(part, "GENERIC-LCD-4D-STATIC");
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
    set(b, "DSC1", LOGIC_1);
    set(b, "DSC2", LOGIC_1);
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

    /* The write happens when the select rises, not when it falls. */
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

/* BP: a square wave at the datasheet's 125 Hz (edge every 4 ms); an off segment follows BP, an on segment is its inverse. */
static void cd_backplane(void) {
    Bench *b = bench("CD22105E", "GENERIC-LCD-4D-STATIC");
    unsigned edges = 0, t, pair_ok = 1;
    LogicValue last = LOGIC_X;

    mpu_idle(b);
    mpu_write(b, 3, 1, 0);        /* digit 1 = "1": B and C on, A off */
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
    /* 20 ms at 125 Hz (8 ms period, an edge every 4 ms): 5 edges */
    check(pair_ok, "CD22105: BP is driven 0/1; a lit segment is the inverse of BP, a dark one equals BP");
    check(edges >= 4 && edges <= 6, "CD22105: BP changes level every 4 ms: 5 times in 20 ms (125 Hz)");
    close_bench(b);
}

static void cd_pins(void) {
    const Part *p = part_library_find(library, "CD22105E");
    const Part *a = part_library_find(library, "CD22105AE");
    int ok;

    check(p != NULL && a != NULL && p->kind == PART_LCDDRIVER && a->kind == PART_LCDDRIVER, "CD22105E and CD22105AE are lcddriver parts");
#define NUM(PART, NAME, DES) (pin_of((PART), (NAME)) != SIZE_MAX && strcmp((PART)->pins[pin_of((PART), (NAME))].designator, (DES)) == 0)
    ok = NUM(p, "VDD", "1") && NUM(p, "E1", "2") && NUM(p, "G1", "3") && NUM(p, "F1", "4") && NUM(p, "BP", "5") &&
         NUM(p, "A2", "6") && NUM(p, "F2", "12") && NUM(p, "A3", "13") && NUM(p, "F3", "19") && NUM(p, "A4", "20") &&
         NUM(p, "B4", "21") && NUM(p, "G4", "25") && NUM(p, "F4", "26") && NUM(p, "DIN0", "27") && NUM(p, "DIN3", "30") &&
         NUM(p, "DSC1", "31") && NUM(p, "DSC2", "32") && NUM(p, "CS1", "33") && NUM(p, "CS2", "34") && NUM(p, "VSS", "35") &&
         NUM(p, "OSC", "36") && NUM(p, "A1", "37") && NUM(p, "B1", "38") && NUM(p, "C1", "39") && NUM(p, "D1", "40");
    check(ok, "CD22105E pins as p.1 (terminal assignment) and p.3 (block diagram)");
    ok = p->pins[pin_of(p, "CS1")].active_low != 0 && p->pins[pin_of(p, "CS2")].active_low != 0;
    check(ok, "CS1 and CS2 are active low");
    check(p->vcc != NULL && p->vcc->min_mv == 3000 && p->vcc->max_mv == 6000, "supply 3-6 V (p.2)");
}

/* ---- ICM7224 / ICM7225 ----------------------------------------------------------------------- */

typedef struct {
    Bench *b;
    int led;
} Ctr;

static void ctr_pulse(Ctr *c, unsigned n) {
    unsigned i;

    for (i = 0; i < n; i++) {
        set(c->b, "COUNT", LOGIC_0);
        run(c->b, 1 * US);
        set(c->b, "COUNT", LOGIC_1);
        run(c->b, 1 * US);
    }
}

static void ctr_reset(Ctr *c) {
    set(c->b, "RESET", LOGIC_0);
    run(c->b, 2 * US);
    set(c->b, "RESET", LOGIC_1);
    run(c->b, 2 * US);
}

/* Inputs idle: COUNT high, RESET high, STORE low (transparent), COUNT INHIBIT high; LZB IN as asked. */
static Ctr *ctr_new(const char *part, int lzb) {
    Ctr *c = calloc(1, sizeof *c);

    c->led = part[6] == '5';
    c->b = bench(part, c->led ? NULL : "GENERIC-LCD-4D-STATIC");
    set(c->b, "COUNT", LOGIC_1);
    set(c->b, "RESET", LOGIC_0);
    set(c->b, "STORE", LOGIC_0);
    set(c->b, "LZB IN", lv((unsigned)lzb));
    set(c->b, "COUNT INHIBIT", LOGIC_1);
    run(c->b, 5 * US);
    ctr_reset(c);
    return c;
}

static void ctr_free(Ctr *c) {
    close_bench(c->b);
    free(c);
}

static const char *const FONT_CHARS = "0123456789";

/* The display as text: half digit ('1' or ' '), then digits 4..1 (a digit 0-9, ' ' for blank, '?' for any other shape). */
static const char *ctr_shown(Ctr *c) {
    static char text[8];
    unsigned d, i;
    LogicValue h, bp;

    if (c->led) {
        run(c->b, 5 * US);
    } else {
        run(c->b, 15000 * US);
    }
    h = get(c->b, "1/2-DIGIT");
    bp = c->led ? LOGIC_X : get(c->b, "BP");
    if (c->led) {
        text[0] = h == LOGIC_0 ? '1' : (h == LOGIC_Z ? ' ' : '?');
    } else {
        text[0] = (h != bp && (h == LOGIC_0 || h == LOGIC_1)) ? '1' : (h == bp ? ' ' : '?');
    }
    for (d = 0; d < 4; d++) {
        unsigned seg = c->led ? led_digit(c->b, 3 - d) : glass_digit(c->b, 3 - d);
        char ch = '?';

        if (seg == 0) {
            ch = ' ';
        }
        for (i = 0; i < 10; i++) {
            if (seg == HEX_FONT[i]) {
                ch = FONT_CHARS[i];
            }
        }
        text[1 + d] = ch;
    }
    text[5] = 0;
    return text;
}

static int shows(Ctr *c, const char *want) {
    const char *got = ctr_shown(c);
    int ok = strcmp(got, want) == 0;

    if (!ok) {
        printf("  shows \"%s\", wanted \"%s\"\n", got, want);
    }
    return ok;
}

static void ctr_tests(const char *part) {
    char text[200];
    Ctr *c;
    unsigned i;
    int ok;
    const char *name = part;

    /* LZB IN low: zeros are displayed */
    c = ctr_new(part, 0);
    snprintf(text, sizeof text, "%s: RESET low gives 0000 (zeros shown with LZB IN low), half digit off", name);
    check(shows(c, " 0000"), text);
    ctr_pulse(c, 1);
    snprintf(text, sizeof text, "%s: one falling COUNT edge: 0001", name);
    check(shows(c, " 0001"), text);
    ctr_pulse(c, 8);
    check(shows(c, " 0009"), "count 9: 0009");
    ctr_pulse(c, 1);
    snprintf(text, sizeof text, "%s: the tenth count carries: 0010", name);
    check(shows(c, " 0010"), text);
    ctr_pulse(c, 90);
    check(shows(c, " 0100"), "count 100: 0100");
    ctr_pulse(c, 900);
    check(shows(c, " 1000"), "count 1000: 1000");
    ctr_pulse(c, 234);
    check(shows(c, " 1234"), "count 1234");
    ctr_pulse(c, 8766);
    snprintf(text, sizeof text, "%s: count 10000 sets the half digit and clears the four digits", name);
    check(shows(c, "10000"), text);
    ctr_pulse(c, 1);
    check(shows(c, "10001"), "count 10001");
    ctr_pulse(c, 9998);
    check(shows(c, "19999"), "count 19999 (the maximum)");
    ctr_pulse(c, 1);
    snprintf(text, sizeof text, "%s: the next count wraps to 0000 and the half digit stays set (10000)", name);
    check(shows(c, "10000"), text);
    ctr_reset(c);
    snprintf(text, sizeof text, "%s: RESET clears the four digits and the half digit", name);
    check(shows(c, " 0000"), text);
    ctr_free(c);

    /* the carry of each decade and every digit value */
    c = ctr_new(part, 0);
    ok = 1;
    for (i = 0; i < 10000; i++) {
        if (i % 1111 == 0 || i == 9999 || i % 997 == 0) {
            char want[8];

            snprintf(want, sizeof want, " %04u", i);
            if (!shows(c, want)) {
                ok = 0;
            }
        }
        ctr_pulse(c, 1);
    }
    snprintf(text, sizeof text, "%s: counting 0 to 9999 one edge at a time shows every spot value (and the next count is 10000)", name);
    check(ok && shows(c, "10000"), text);
    ctr_free(c);

    /* edges and the control inputs */
    c = ctr_new(part, 0);
    ctr_pulse(c, 5);
    set(c->b, "COUNT", LOGIC_0);
    run(c->b, 2 * US);
    snprintf(text, sizeof text, "%s: the count happens on the falling edge", name);
    check(shows(c, " 0006"), text);
    set(c->b, "COUNT", LOGIC_1);
    run(c->b, 2 * US);
    snprintf(text, sizeof text, "%s: the rising edge counts nothing", name);
    check(shows(c, " 0006"), text);
    set(c->b, "COUNT INHIBIT", LOGIC_0);
    run(c->b, 2 * US);
    ctr_pulse(c, 7);
    snprintf(text, sizeof text, "%s: COUNT INHIBIT low: COUNT edges are ignored", name);
    check(shows(c, " 0006"), text);
    set(c->b, "COUNT", LOGIC_0);
    run(c->b, 2 * US);
    set(c->b, "COUNT INHIBIT", LOGIC_1);
    run(c->b, 2 * US);
    snprintf(text, sizeof text, "%s: COUNT INHIBIT rising while COUNT is low counts nothing", name);
    check(shows(c, " 0006"), text);
    set(c->b, "COUNT", LOGIC_1);
    run(c->b, 2 * US);
    set(c->b, "COUNT INHIBIT", LOGIC_0);
    set(c->b, "COUNT", LOGIC_0);
    run(c->b, 2 * US);
    set(c->b, "COUNT INHIBIT", LOGIC_1);
    set(c->b, "COUNT", LOGIC_1);
    run(c->b, 2 * US);
    check(shows(c, " 0006"), "a COUNT edge during inhibit, then inhibit released: nothing counted");
    ctr_pulse(c, 1);
    snprintf(text, sizeof text, "%s: counting resumes when COUNT INHIBIT is high again", name);
    check(shows(c, " 0007"), text);
    set(c->b, "RESET", LOGIC_0);
    run(c->b, 2 * US);
    ctr_pulse(c, 5);
    snprintf(text, sizeof text, "%s: COUNT edges while RESET is low are ignored (counter held at 0000)", name);
    check(shows(c, " 0000"), text);
    set(c->b, "RESET", LOGIC_1);
    run(c->b, 2 * US);
    snprintf(text, sizeof text, "%s: releasing RESET does not count", name);
    check(shows(c, " 0000"), text);
    ctr_pulse(c, 3);
    check(shows(c, " 0003"), "counting after RESET");
    ctr_free(c);

    /* STORE */
    c = ctr_new(part, 0);
    ctr_pulse(c, 42);
    check(shows(c, " 0042"), "count 42");
    set(c->b, "STORE", LOGIC_1);
    run(c->b, 2 * US);
    ctr_pulse(c, 100);
    snprintf(text, sizeof text, "%s: STORE high latches the display: it still shows 0042 while the counter runs on", name);
    check(shows(c, " 0042"), text);
    set(c->b, "STORE", LOGIC_0);
    run(c->b, 2 * US);
    snprintf(text, sizeof text, "%s: STORE low again shows the counter at once: 0142", name);
    check(shows(c, " 0142"), text);
    set(c->b, "STORE", LOGIC_1);
    run(c->b, 2 * US);
    ctr_reset(c);
    snprintf(text, sizeof text, "%s: RESET with STORE high clears the counter but not the display latches", name);
    check(shows(c, " 0142"), text);
    ctr_pulse(c, 5);
    check(shows(c, " 0142"), "the display latch still holds 0142");
    set(c->b, "STORE", LOGIC_0);
    run(c->b, 2 * US);
    snprintf(text, sizeof text, "%s: STORE low shows the counter (0005, reset then 5 counts)", name);
    check(shows(c, " 0005"), text);
    set(c->b, "STORE", LOGIC_1);
    run(c->b, 2 * US);
    ctr_pulse(c, 9995);
    set(c->b, "STORE", LOGIC_0);
    run(c->b, 2 * US);
    snprintf(text, sizeof text, "%s: the half digit is latched too: STORE low shows 10000", name);
    check(shows(c, "10000"), text);
    ctr_reset(c);
    check(shows(c, " 0000"), "RESET with STORE low clears the display");
    ctr_free(c);

    /* two inputs changing in the same instant */
    c = ctr_new(part, 0);
    ctr_pulse(c, 9);
    set(c->b, "RESET", LOGIC_0);
    set(c->b, "COUNT", LOGIC_0);
    run(c->b, 2 * US);
    snprintf(text, sizeof text, "%s: RESET falling in the same instant as COUNT: reset wins, nothing counted", name);
    check(shows(c, " 0000"), text);
    set(c->b, "RESET", LOGIC_1);
    set(c->b, "COUNT", LOGIC_1);
    run(c->b, 2 * US);
    ctr_pulse(c, 2);
    set(c->b, "STORE", LOGIC_1);
    set(c->b, "LZB IN", LOGIC_1);
    run(c->b, 2 * US);
    ctr_pulse(c, 3);
    check(shows(c, " 0002"), "STORE high and LZB IN high in the same instant: STORE wins, the display holds 0002");
    set(c->b, "STORE", LOGIC_0);
    set(c->b, "COUNT INHIBIT", LOGIC_0);
    run(c->b, 2 * US);
    check(shows(c, "    5"), "STORE low and COUNT INHIBIT low in the same instant: the display shows 5 with the zeros blanked (LZB IN high)");
    ctr_free(c);

    /* leading zero blanking */
    c = ctr_new(part, 1);
    snprintf(text, sizeof text, "%s: LZB IN high and count 0000: the whole display is blank", name);
    check(shows(c, "     "), text);
    ctr_pulse(c, 7);
    snprintf(text, sizeof text, "%s: LZB IN high: leading zeros blank (7)", name);
    check(shows(c, "    7"), text);
    ctr_pulse(c, 63);
    check(shows(c, "   70"), "70: the zero after a digit is shown");
    ctr_pulse(c, 630);
    check(shows(c, "  700"), "700");
    ctr_pulse(c, 6300);
    check(shows(c, " 7000"), "7000");
    ctr_pulse(c, 3000);
    snprintf(text, sizeof text, "%s: half digit set: zeros are shown even with LZB IN high (10000)", name);
    check(shows(c, "10000"), text);
    set(c->b, "LZB IN", LOGIC_0);
    ctr_reset(c);
    ctr_pulse(c, 42);
    check(shows(c, " 0042"), "LZB IN low: 0042");
    set(c->b, "LZB IN", LOGIC_1);
    run(c->b, 2 * US);
    snprintf(text, sizeof text, "%s: LZB IN raised while counting blanks the leading zeros at once (  42)", name);
    check(shows(c, "   42"), text);
    set(c->b, "STORE", LOGIC_1);
    run(c->b, 2 * US);
    set(c->b, "LZB IN", LOGIC_0);
    run(c->b, 2 * US);
    snprintf(text, sizeof text, "%s: with STORE high LZB IN does not change the latched display", name);
    check(shows(c, "   42"), text);
    set(c->b, "STORE", LOGIC_0);
    run(c->b, 2 * US);
    check(shows(c, " 0042"), "STORE low: the new LZB state shows");
    ctr_free(c);
}

/* ICM7225: the CARRY and LZB OUT pins (ports driver); open-drain segments, 1/2-digit sink. */
static void led_outputs(void) {
    Ctr *c = ctr_new("ICM7225IPL", 1);
    unsigned i, ok = 1, high_at = 0, low_at = 0;
    LogicValue prev;

    check(get(c->b, "LZB OUT") == LOGIC_1, "ICM7225: LZB OUT is high while the display is blank (LZB IN high, count 0000)");
    ctr_pulse(c, 1);
    check(get(c->b, "LZB OUT") == LOGIC_0, "ICM7225: LZB OUT is low once a digit shows");
    ctr_reset(c);
    check(get(c->b, "LZB OUT") == LOGIC_1, "ICM7225: LZB OUT is high again after RESET");
    set(c->b, "LZB IN", LOGIC_0);
    run(c->b, 2 * US);
    check(get(c->b, "LZB OUT") == LOGIC_0, "ICM7225: LZB OUT is low with LZB IN low (zeros shown)");
    set(c->b, "LZB IN", LOGIC_1);
    run(c->b, 2 * US);
    check(get(c->b, "LZB OUT") == LOGIC_1, "ICM7225: LZB OUT follows LZB IN");
    set(c->b, "STORE", LOGIC_1);
    run(c->b, 2 * US);
    ctr_pulse(c, 3);
    check(get(c->b, "LZB OUT") == LOGIC_0, "ICM7225: LZB OUT comes from the decoder (not latched): low at count 3 though STORE is high");
    set(c->b, "STORE", LOGIC_0);
    ctr_reset(c);
    ctr_pulse(c, 9999);
    set(c->b, "RESET", LOGIC_1);
    ctr_pulse(c, 1);
    check(get(c->b, "LZB OUT") == LOGIC_0, "ICM7225: LZB OUT is low with the half digit set (count 10000)");
    ctr_free(c);

    c = ctr_new("ICM7225IPL", 0);
    prev = get(c->b, "CARRY");
    ok = prev == LOGIC_0;
    for (i = 1; i <= 10000; i++) {
        LogicValue v;

        ctr_pulse(c, 1);
        v = get(c->b, "CARRY");
        if (v == LOGIC_1 && prev == LOGIC_0 && high_at == 0) {
            high_at = i;
        }
        if (v == LOGIC_0 && prev == LOGIC_1) {
            low_at = i;
        }
        if (v != LOGIC_0 && v != LOGIC_1) {
            ok = 0;
        }
        prev = v;
    }
    check(ok && high_at == 6000 && low_at == 10000, "ICM7225: CARRY goes high at count 6000 and low at 9999 -> 10000 (p.5)");
    ctr_free(c);

    c = ctr_new("ICM7225IPL", 0);
    ctr_pulse(c, 1);
    /* a dark segment is released (Z), a lit one sinks (0): never driven high */
    ok = 1;
    for (i = 0; i < 29; i++) {
        static const char *const SEG = "ABCDEFG";
        char name[16];
        LogicValue v;

        if (i < 28) {
            snprintf(name, sizeof name, "%c%u", SEG[i % 7], i / 7 + 1);
        } else {
            snprintf(name, sizeof name, "1/2-DIGIT");
        }
        v = get(c->b, name);
        ok &= v == LOGIC_Z || v == LOGIC_0;
    }
    check(ok, "ICM7225: every segment pin is 0 (lit) or Z (dark), never driven high (open-drain sinks)");
    ctr_free(c);
}

/* ICM7224: BP at 150 Hz, the half digit and the segments against it. */
static void lcd_backplane(void) {
    Ctr *c = ctr_new("ICM7224IPL", 0);
    unsigned edges = 0, t, pair_ok = 1;
    LogicValue last = LOGIC_X;

    ctr_pulse(c, 1);         /* digit 1 = "1": B and C on, A off; half off */
    run(c->b, 10000 * US);
    for (t = 0; t < 20000; t += 50) {
        LogicValue bp = get(c->b, "BP"), on = get(c->b, "B1"), off = get(c->b, "A1"), half = get(c->b, "1/2-DIGIT");

        if (bp != LOGIC_0 && bp != LOGIC_1) {
            pair_ok = 0;
        }
        if (last != LOGIC_X && bp != last) {
            edges++;
        }
        last = bp;
        pair_ok &= on != bp && off == bp && half == bp;
        run(c->b, 50 * US);
    }
    check(pair_ok, "ICM7224: BP is driven 0/1; a lit segment is the inverse of BP, a dark one (and the half digit) equals BP");
    check(edges >= 5 && edges <= 7, "ICM7224: BP changes level every 3.33 ms: 6 times in 20 ms (150 Hz)");
    ctr_free(c);

    c = ctr_new("ICM7224IPL", 0);
    ctr_pulse(c, 10000);
    run(c->b, 15000 * US);
    {
        LogicValue bp = get(c->b, "BP"), half = get(c->b, "1/2-DIGIT");

        check(half != bp, "ICM7224: the half digit pin is the inverse of BP (lit) at count 10000");
    }
    ctr_free(c);
}

static void ctr_pins(void) {
    const Part *p = part_library_find(library, "ICM7224IPL");
    const Part *q = part_library_find(library, "ICM7224IQH");
    const Part *l = part_library_find(library, "ICM7225IPL");
    const Part *m = part_library_find(library, "ICM7225IQH");
    const Part *j = part_library_find(library, "ICM7224IJL");
    size_t i;
    int ok;

    check(p != NULL && q != NULL && j != NULL && p->kind == PART_LCDDRIVER && l != NULL && m != NULL && l->kind == PART_LEDDRIVER,
          "kinds: ICM7224 lcddriver, ICM7225 leddriver, all five ordering-table parts exist");
    ok = NUM(p, "V+", "1") && NUM(p, "E1", "2") && NUM(p, "BP", "5") && NUM(p, "A2", "6") && NUM(p, "F2", "12") &&
         NUM(p, "A4", "20") && NUM(p, "B4", "21") && NUM(p, "G4", "25") && NUM(p, "F4", "26") && NUM(p, "1/2-DIGIT", "27") &&
         NUM(p, "CARRY", "28") && NUM(p, "LZB IN", "29") && NUM(p, "LZB OUT", "30") && NUM(p, "COUNT INHIBIT", "31") &&
         NUM(p, "COUNT", "32") && NUM(p, "RESET", "33") && NUM(p, "STORE", "34") && NUM(p, "GND", "35") && NUM(p, "OSC", "36") &&
         NUM(p, "A1", "37") && NUM(p, "D1", "40");
    check(ok, "ICM7224IPL pins as p.1 (DIP-40)");
    ok = NUM(q, "V+", "2") && NUM(q, "E1", "3") && NUM(q, "BP", "6") && NUM(q, "G2", "13") && NUM(q, "F2", "14") &&
         NUM(q, "A4", "22") && NUM(q, "B4", "24") && NUM(q, "G4", "28") && NUM(q, "F4", "29") && NUM(q, "1/2-DIGIT", "30") &&
         NUM(q, "CARRY", "31") && NUM(q, "LZB IN", "32") && NUM(q, "LZB OUT", "33") && NUM(q, "COUNT INHIBIT", "35") &&
         NUM(q, "COUNT", "36") && NUM(q, "RESET", "37") && NUM(q, "STORE", "38") && NUM(q, "GND", "39") && NUM(q, "OSC", "40") &&
         NUM(q, "A1", "41") && NUM(q, "D1", "44");
    check(ok, "ICM7224IQH pins as p.8 (44-lead PLCC, pins 1, 12, 23 and 34 no connect)");
    ok = NUM(l, "BRT", "5") && NUM(l, "GND", "35") && l->pins[35].role == PIN_GROUND && NUM(m, "BRT", "6") && NUM(m, "GND", "39") &&
         m->pins[39].role == PIN_GROUND;
    check(ok, "ICM7225: BRT on pin 5 (6 on the PLCC) and a second ground on pin 36 (40)");
    ok = 1;
    for (i = 0; i < p->pin_count; i++) {
        const char *n = p->pins[i].name;

        if (strcmp(n, "STORE") == 0 || strcmp(n, "RESET") == 0 || strcmp(n, "COUNT") == 0 || strcmp(n, "COUNT INHIBIT") == 0) {
            ok &= p->pins[i].active_low != 0 && p->pins[i].role == PIN_INPUT;
        }
    }
    check(ok, "STORE, RESET, COUNT and COUNT INHIBIT are active-low inputs");
    check(p->vcc != NULL && p->vcc->min_mv == 3000 && p->vcc->max_mv == 6000 && l->vcc->min_mv == 4000 && l->vcc->max_mv == 6000,
          "supply: ICM7224 3-6 V, ICM7225 4-6 V (p.3)");
}

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL loading the parts\n");
        return 1;
    }
    cd_pins();
    mpu_font("CD22105E", HEX_FONT, "CD22105 (hexadecimal)");
    mpu_font("CD22105AE", CODEB_FONT, "CD22105A (decimal, Code B)");
    mpu_behaviour("CD22105E", HEX_FONT, "CD22105");
    mpu_behaviour("CD22105AE", CODEB_FONT, "CD22105A");
    cd_backplane();
    ctr_pins();
    ctr_tests("ICM7224IPL");
    ctr_tests("ICM7225IPL");
    lcd_backplane();
    led_outputs();
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("cd22105_icm7224_test: all passed\n");
    return 0;
}
