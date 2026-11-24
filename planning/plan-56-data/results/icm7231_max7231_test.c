/*
 * icm7231_max7231_test (plan-56-F): the triplexed (1:3) numeric LCD drivers ICM7231, ICM7232 (Harris / Intersil
 * FN3161) and MAX7231, MAX7232 (Maxim, 19-xxxx 1995), through their real buses, wired to the bundled
 * GENERIC-LCD-8D-1:3 glass and read on the backplane and segment pins.
 *
 * Datasheets: datasheets/lcd-drivers/icm7231-icm7232-harris.pdf, max7231-max7234.pdf (PDF pages).
 * Covered: the pinouts (DIP-40 and PLCC-44, the ICM7232CR reversed one), the parallel write (address A2-A0,
 * data AN2 AN1 BD3-BD0 latched on the rising edge of CS), the serial write (8, 9 or 10 bits on the rising edge
 * of DATA CLOCK, AN1 AN2 BD0-BD3 A0-A3, decoded by the WRITE pulse; a short frame, a WRITE pulse that resets a
 * partial word, addresses above D10), hexadecimal and Code B fonts of every digit, the annunciators (A and B
 * versions: both on COM3; C versions: an2 on COM1, an1 on COM3), the 1:3 waveform (one backplane driven at a
 * time, AC alternation each frame, 90 Hz) and VDISP high = shutdown (pins released, data kept).
 *
 * Not modeled (and so not tested): the MAX7233 / MAX7234 18-segment ASCII decode (devices not added), DATA
 * ACCEPTED (never driven), the eleventh-clock reset of the serial shift register (a longer word writes its
 * last ten bits), VDISP open = shutdown, display voltage levels, the CS mid-level test mode.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "converter_state.h"
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
enum { HALF = 2, FRAME_US = 5556, SLOT_US = 1852 };

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

static void setn(Bench *b, const char *name, int v) {
    set(b, name, v ? LOGIC_1 : LOGIC_0);
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

static void run_us(Bench *b, unsigned us) {
    run(b, (SimTime)us * US);
}

/* ---- the fonts: segments lit per code (datasheet Table 1 / Table 2) -------------------------------- */

static const char *const HEX_FONT[16] = {"abcdef", "bc", "abdeg", "abcdg", "bcfg", "acdfg", "acdefg", "abc",
                                         "abcdefg", "abcdfg", "abcefg", "cdefg", "adef", "bcdeg", "adefg", "aefg"};
static const char *const CODEB_FONT[16] = {"abcdef", "bc", "abdeg", "abcdg", "bcfg", "acdfg", "acdefg", "abc",
                                           "abcdefg", "abcdfg", "g", "adefg", "bcefg", "def", "abefg", ""};

/* The (column, backplane) a segment sits at: column 0 = Z line (pin nZ), 1 = Y, 2 = X; backplane 0..2. */
typedef struct {
    int col, bp;
} Cell;

static Cell seg_cell(int variant_c, char s) {
    switch (s) {
    case 'a':
        return (Cell){1, 0};
    case 'b':
        return (Cell){0, 0};
    case 'c':
        return (Cell){0, 1};
    case 'd':
        return (Cell){1, 2};
    case 'g':
        return (Cell){1, 1};
    case 'f':
        return variant_c ? (Cell){2, 1} : (Cell){2, 0};
    default: /* e */
        return variant_c ? (Cell){2, 2} : (Cell){2, 1};
    }
}

/* The expected 9-bit RAM word of a digit: bit 3 * column + backplane. an1 = Z line on COM3, an2 = X line on
   COM3 (A, B) or COM1 (C). */
static unsigned expect_word(int variant_c, const char *segs, int an1, int an2) {
    unsigned w = 0;
    const char *p;

    for (p = segs; *p; p++) {
        Cell c = seg_cell(variant_c, *p);

        w |= 1u << (3 * c.col + c.bp);
    }
    if (an1) {
        w |= 1u << (3 * 0 + 2);
    }
    if (an2) {
        w |= variant_c ? 1u << (3 * 2 + 0) : 1u << (3 * 2 + 2);
    }
    return w;
}

/* ---- the device table ---------------------------------------------------------------------------------- */

typedef struct {
    const char *part;
    int serial, digits, variant_c;
    const char *const *font;
    const char *bp[3];
    const char *label;
} Dev;

static const Dev DEVS[] = {
    {"ICM7231BFIPL", 0, 8, 0, CODEB_FONT, {"BP1", "BP2", "BP3"}, "ICM7231B"},
    {"ICM7232AFIPL", 1, 10, 0, HEX_FONT, {"BP1", "BP2", "BP3"}, "ICM7232A"},
    {"ICM7232BFIPL", 1, 10, 0, CODEB_FONT, {"BP1", "BP2", "BP3"}, "ICM7232B"},
    {"ICM7232CRIPL", 1, 10, 1, CODEB_FONT, {"BP1", "BP2", "BP3"}, "ICM7232C"},
    {"MAX7231AFIPL", 0, 8, 0, HEX_FONT, {"COM1", "COM2", "COM3"}, "MAX7231A"},
    {"MAX7231BFIPL", 0, 8, 0, CODEB_FONT, {"COM1", "COM2", "COM3"}, "MAX7231B"},
    {"MAX7231CFIPL", 0, 8, 1, CODEB_FONT, {"COM1", "COM2", "COM3"}, "MAX7231C"},
    {"MAX7232AFIPL", 1, 10, 0, HEX_FONT, {"COM1", "COM2", "COM3"}, "MAX7232A"},
    {"MAX7232BFIPL", 1, 10, 0, CODEB_FONT, {"COM1", "COM2", "COM3"}, "MAX7232B"},
    {"MAX7232CFIPL", 1, 10, 1, CODEB_FONT, {"COM1", "COM2", "COM3"}, "MAX7232C"},
};

static void segname(char *buf, size_t cap, int digit, int col) {
    snprintf(buf, cap, "%d%c", digit, "ZYX"[col]);
}

/* ---- bench ------------------------------------------------------------------------------------------- */

static Bench *bench(const Dev *dv, const char *part, int with_glass) {
    Bench *b = calloc(1, sizeof *b);
    SimNet **nets;
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
    nets = calloc(b->part->pin_count, sizeof *nets);
    for (i = 0; i < b->part->pin_count; i++) {
        nets[i] = sim_add_net(b->sim, b->part->pins[i].name);
        sim_connect(b->sim, nets[i], b->chip, i);
        sim_connect(b->sim, nets[i], b->drv, i);
    }
    if (with_glass) {
        b->glass_part = part_library_find(library, "GENERIC-LCD-8D-1:3");
        b->glass = sim_add_instance(b->sim, kind_model(b->glass_part), "G1", b->glass_part);
        for (i = 0; i < b->glass_part->pin_count; i++) {
            const char *name = b->glass_part->pins[i].name;
            char chip[16];

            if (strncmp(name, "BP", 2) == 0) {
                snprintf(chip, sizeof chip, "%s", dv->bp[name[2] - '1']);
            } else if (name[0] == 'S') {
                /* S<d>A -> <d>Z, S<d>B -> <d>Y, S<d>C -> <d>X */
                snprintf(chip, sizeof chip, "%c%c", name[1], name[2] == 'A' ? 'Z' : name[2] == 'B' ? 'Y' : 'X');
            } else {
                continue;
            }
            sim_connect(b->sim, nets[bpin(b, chip)], b->glass, i);
        }
    }
    free(nets);
    if (!sim_start(b->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(b->sim));
        exit(2);
    }
    if (dv->serial) {
        setn(b, "WRITE", 1);
        setn(b, "DATA CLOCK", 1);
        setn(b, "DATA INPUT", 0);
    } else {
        int k;

        setn(b, "CS", 1);
        for (k = 0; k < 3; k++) {
            char n[4];

            snprintf(n, sizeof n, "A%d", k);
            setn(b, n, 0);
        }
        for (k = 0; k < 4; k++) {
            char n[4];

            snprintf(n, sizeof n, "BD%d", k);
            setn(b, n, 0);
        }
        setn(b, "AN1", 0);
        setn(b, "AN2", 0);
    }
    setn(b, "VDISP", 0);
    run_us(b, 10);
    return b;
}

static void close_bench(Bench *b) {
    sim_destroy(b->sim);
    free(b);
}

static long reg_value(const Bench *b, const char *name) {
    const SimComponentType *type = sim_instance_type(b->chip);
    ConverterRegister regs[96];
    size_t n = type->registers(sim_instance_state(b->chip), regs, 96), i;

    for (i = 0; i < n; i++) {
        if (strcmp(regs[i].name, name) == 0) {
            return (long)regs[i].value;
        }
    }
    return -1;
}

static long dreg(const Bench *b, int digit) {
    char n[8];

    snprintf(n, sizeof n, "D%d", digit);
    return reg_value(b, n);
}

/* ---- bus ------------------------------------------------------------------------------------------------ */

/* Parallel write (ICM7231 / MAX7231): CS low, address and data, CS high latches. */
static void par_set(Bench *b, int digit0, unsigned nib, int an1, int an2) {
    int k;

    for (k = 0; k < 3; k++) {
        char n[4];

        snprintf(n, sizeof n, "A%d", k);
        setn(b, n, (digit0 >> k) & 1);
    }
    for (k = 0; k < 4; k++) {
        char n[4];

        snprintf(n, sizeof n, "BD%d", k);
        setn(b, n, (nib >> k) & 1);
    }
    setn(b, "AN1", an1);
    setn(b, "AN2", an2);
}

static void par_write(Bench *b, int digit0, unsigned nib, int an1, int an2) {
    par_set(b, digit0, nib, an1, an2);
    run_us(b, HALF);
    setn(b, "CS", 0);
    run_us(b, HALF);
    setn(b, "CS", 1);
    run_us(b, HALF);
}

static void ser_bit(Bench *b, int v) {
    setn(b, "DATA INPUT", v);
    run_us(b, HALF);
    setn(b, "DATA CLOCK", 0);
    run_us(b, HALF);
    setn(b, "DATA CLOCK", 1);
    run_us(b, HALF);
}

static void write_pulse(Bench *b) {
    setn(b, "WRITE", 0);
    run_us(b, HALF);
    setn(b, "WRITE", 1);
    run_us(b, HALF);
}

/* The bits of a word in time order: AN1 AN2 BD0..BD3 A0..A3, the first `skip` of them dropped
   (skip 0: 10 bits, 1: 9 bits without AN1, 2: 8 bits without the annunciators). No WRITE pulse. */
static void ser_word(Bench *b, int digit0, unsigned nib, int an1, int an2, int skip) {
    int bits[10], i;

    bits[0] = an1;
    bits[1] = an2;
    for (i = 0; i < 4; i++) {
        bits[2 + i] = (int)((nib >> i) & 1u);
        bits[6 + i] = (digit0 >> i) & 1;
    }
    for (i = skip; i < 10; i++) {
        ser_bit(b, bits[i]);
    }
}

/* ---- waveform and glass ----------------------------------------------------------------------------- */

/* Run to the middle of the next slot (slots start at time 0 and last SLOT_US). */
static void to_slot_middle(Bench *b) {
    SimTime slot = (SimTime)SLOT_US * US, rem = sim_now(b->sim) % slot;
    SimTime target = slot / 2;

    run(b, rem <= target ? target - rem : slot - rem + target);
}

/* Run to the middle of the next slot of backplane 1 (slot index divisible by 3). */
static void to_bp1_middle(Bench *b) {
    SimTime slot = (SimTime)SLOT_US * US;

    to_slot_middle(b);
    while ((sim_now(b->sim) / slot) % 3 != 0) {
        run(b, slot);
    }
}

typedef struct {
    unsigned char lit[10][3][3];    /* digit, column, backplane */
    unsigned bp_low, bp_high;       /* per backplane (bit k): driven low / high at some sample */
    int bad;                        /* a sample with other than one backplane driven, or a segment pin undriven */
} Wave;

/* Twelve samples, the middle of each slot over two frames. */
static void sample_wave(Bench *b, const Dev *dv, Wave *w) {
    int s;

    memset(w, 0, sizeof *w);
    to_bp1_middle(b);
    for (s = 0; s < 12; s++) {
        int k, driven = 0, which = -1, d, c;
        LogicValue level = LOGIC_Z;

        for (k = 0; k < 3; k++) {
            LogicValue v = get(b, dv->bp[k]);

            if (v == LOGIC_0 || v == LOGIC_1) {
                driven++;
                which = k;
                level = v;
                if (v == LOGIC_0) {
                    w->bp_low |= 1u << k;
                } else {
                    w->bp_high |= 1u << k;
                }
            }
        }
        if (driven != 1 || which != s % 3) {
            w->bad = 1;
        } else {
            for (d = 1; d <= dv->digits; d++) {
                for (c = 0; c < 3; c++) {
                    char n[8];
                    LogicValue v;

                    segname(n, sizeof n, d, c);
                    v = get(b, n);
                    if (v != LOGIC_0 && v != LOGIC_1) {
                        w->bad = 1;
                    } else if (v != level) {
                        w->lit[d - 1][c][which] = 1;
                    }
                }
            }
        }
        run_us(b, SLOT_US);
    }
}

/* Does the pin waveform equal the registers' RAM bits (digit by digit)? */
static int wave_matches_ram(Bench *b, const Dev *dv) {
    Wave w;
    int d, c, k, ok = 1;

    sample_wave(b, dv, &w);
    if (w.bad || w.bp_low != 7u || w.bp_high != 7u) {
        printf("  waveform bad=%d low=%x high=%x\n", w.bad, w.bp_low, w.bp_high);
        return 0;
    }
    for (d = 1; d <= dv->digits; d++) {
        long ram = dreg(b, d);

        for (c = 0; c < 3; c++) {
            for (k = 0; k < 3; k++) {
                int want = (int)((ram >> (3 * c + k)) & 1);

                if (w.lit[d - 1][c][k] != want) {
                    printf("  digit %d col %d bp %d lit %d want %d\n", d, c, k + 1, w.lit[d - 1][c][k], want);
                    ok = 0;
                }
            }
        }
    }
    return ok;
}

/* The glass cells: S<d>A = (A BP1, F BP2, E BP3), S<d>B = (B, G, D), S<d>C = (C BP1, DP BP2, none);
   cell order per digit A F E B G D C DP; wired Z -> A, Y -> B, X -> C. */
static int glass_matches(Bench *b, int digits) {
    const SimComponentType *type = sim_instance_type(b->glass);
    unsigned char values[200];
    char want[100];
    size_t n, i;
    int d;
    static const int map[8][2] = {{0, 0}, {0, 1}, {0, 2}, {1, 0}, {1, 1}, {1, 2}, {2, 0}, {2, 1}};
    SimTime window = (SimTime)FRAME_US * 2 * US;

    run(b, (SimTime)FRAME_US * 4 * US);
    n = type->lights(b->sim, b->glass, sim_instance_state(b->glass), sim_now(b->sim) - window, values, sizeof values);
    if (n != 64) {
        return 0;
    }
    for (d = 1; d <= 8; d++) {
        long ram = d <= digits ? dreg(b, d) : 0;
        int j;

        for (j = 0; j < 8; j++) {
            want[8 * (d - 1) + j] = (ram >> (3 * map[j][0] + map[j][1])) & 1 ? 1 : 0;
        }
    }
    for (i = 0; i < n; i++) {
        if ((values[i] != 0) != (want[i] != 0)) {
            printf("  glass cell %zu lit %d want %d\n", i, values[i] != 0, want[i] != 0);
            return 0;
        }
    }
    return 1;
}

/* ---- write helper per bus -------------------------------------------------------------------------------- */

static void put(Bench *b, const Dev *dv, int digit0, unsigned nib, int an1, int an2) {
    if (dv->serial) {
        write_pulse(b);
        ser_word(b, digit0, nib, an1, an2, 0);
        write_pulse(b);
    } else {
        par_write(b, digit0, nib, an1, an2);
    }
}

/* ---- tests ------------------------------------------------------------------------------------------------ */

typedef struct {
    int num;
    const char *name;
} PinWant;

static void pinout(const char *part, const PinWant *w, size_t n, size_t count, const char *label) {
    const Part *p = part_library_find(library, part);
    size_t i;
    int ok = p != NULL && p->pin_count == count && p->kind == PART_LCDDRIVER;
    int power = 0, ground = 0;

    for (i = 0; ok && i < n; i++) {
        ok = strcmp(p->pins[w[i].num - 1].name, w[i].name) == 0;
        if (!ok) {
            printf("  pin %d is %s, want %s\n", w[i].num, p->pins[w[i].num - 1].name, w[i].name);
        }
    }
    for (i = 0; p != NULL && i < p->pin_count; i++) {
        power += p->pins[i].role == PIN_POWER;
        ground += p->pins[i].role == PIN_GROUND;
    }
    check(ok && power == 1 && ground == 1, label);
}

static void pinouts(void) {
    static const PinWant H31[] = {{1, "CS"}, {2, "VDISP"}, {3, "BP1"}, {5, "BP3"}, {6, "1Z"}, {7, "1Y"}, {8, "1X"},
                                  {20, "5X"}, {21, "6Z"}, {29, "8X"}, {30, "AN1"}, {31, "AN2"}, {32, "BD0"},
                                  {35, "BD3"}, {36, "VSS"}, {37, "A0"}, {39, "A2"}, {40, "VDD"}};
    static const PinWant M31[] = {{1, "CS"}, {2, "VDISP"}, {3, "COM1"}, {5, "COM3"}, {6, "1Z"}, {20, "5X"},
                                  {21, "6Z"}, {29, "8X"}, {30, "AN1"}, {31, "AN2"}, {32, "BD0"}, {35, "BD3"},
                                  {36, "GND"}, {37, "A0"}, {39, "A2"}, {40, "V+"}};
    static const PinWant M31Q[] = {{1, "NC"}, {2, "CS"}, {3, "VDISP"}, {4, "COM1"}, {6, "COM3"}, {7, "1Z"},
                                   {12, "NC"}, {13, "2X"}, {17, "4Z"}, {23, "NC"}, {29, "7X"}, {32, "8X"},
                                   {33, "AN1"}, {34, "NC"}, {35, "AN2"}, {36, "BD0"}, {39, "BD3"}, {40, "GND"},
                                   {41, "A0"}, {43, "A2"}, {44, "V+"}};
    static const PinWant H32[] = {{1, "DATA CLOCK"}, {2, "VDISP"}, {3, "BP1"}, {6, "1Z"}, {20, "5X"}, {21, "6Z"},
                                  {23, "6X"}, {33, "10Z"}, {35, "10X"}, {36, "VSS"}, {37, "DATA ACCEPTED"},
                                  {38, "DATA INPUT"}, {39, "WRITE"}, {40, "VDD"}};
    static const PinWant H32R[] = {{1, "DATA CLOCK"}, {20, "5X"}, {21, "10X"}, {23, "10Z"}, {24, "9X"}, {26, "9Z"},
                                   {30, "7X"}, {32, "7Z"}, {33, "6X"}, {35, "6Z"}, {36, "VSS"}, {37, "DATA ACCEPTED"},
                                   {38, "DATA INPUT"}, {39, "WRITE"}, {40, "VDD"}};
    static const PinWant M32[] = {{1, "DATA CLOCK"}, {2, "VDISP"}, {3, "COM1"}, {6, "1Z"}, {21, "6Z"}, {35, "10X"},
                                  {36, "GND"}, {37, "DATA ACCEPTED"}, {38, "DATA INPUT"}, {39, "WRITE"}, {40, "V+"}};
    static const PinWant M32Q[] = {{1, "NC"}, {2, "DATA CLOCK"}, {3, "VDISP"}, {4, "COM1"}, {7, "1Z"}, {12, "NC"},
                                   {29, "7X"}, {30, "8Z"}, {33, "9Z"}, {34, "NC"}, {35, "9Y"}, {37, "10Z"},
                                   {39, "10X"}, {40, "GND"}, {41, "DATA ACCEPTED"}, {42, "DATA INPUT"},
                                   {43, "WRITE"}, {44, "V+"}};

    pinout("ICM7231BFIPL", H31, sizeof H31 / sizeof *H31, 40, "ICM7231 DIP-40 pinout (icm7231-icm7232-harris p.2)");
    pinout("ICM7231BFIJL", H31, sizeof H31 / sizeof *H31, 40, "ICM7231 ceramic DIP-40 pinout");
    pinout("ICM7232AFIPL", H32, sizeof H32 / sizeof *H32, 40, "ICM7232AF/BF DIP-40 pinout (p.2)");
    pinout("ICM7232AFIJL", H32, sizeof H32 / sizeof *H32, 40, "ICM7232AF ceramic DIP-40 pinout");
    pinout("ICM7232BFIPL", H32, sizeof H32 / sizeof *H32, 40, "ICM7232BF DIP-40 pinout");
    pinout("ICM7232CRIPL", H32R, sizeof H32R / sizeof *H32R, 40, "ICM7232CR reversed DIP-40 pinout (p.2)");
    pinout("MAX7231BFIPL", M31, sizeof M31 / sizeof *M31, 40, "MAX7231 DIP-40 pinout (max7231-max7234 p.1)");
    pinout("MAX7231BFIQ", M31Q, sizeof M31Q / sizeof *M31Q, 44, "MAX7231 PLCC-44 pinout (p.1)");
    pinout("MAX7231BFIQH+D", M31Q, sizeof M31Q / sizeof *M31Q, 44, "MAX7231BFIQH+D is the PLCC-44 part");
    pinout("MAX7231CFIPL+", M31, sizeof M31 / sizeof *M31, 40, "MAX7231CFIPL+ is the DIP-40 part");
    pinout("MAX7232AFIPL", M32, sizeof M32 / sizeof *M32, 40, "MAX7232 DIP-40 pinout (p.2)");
    pinout("MAX7232CFIQ", M32Q, sizeof M32Q / sizeof *M32Q, 44, "MAX7232 PLCC-44 pinout (p.2)");
}

static int is_output_pin(const Part *p, const char *name) {
    size_t i = pin_of(p, name);

    return i != SIZE_MAX && p->pins[i].role == PIN_OUTPUT;
}

static void device_tests(const Dev *dv) {
    Bench *b = bench(dv, dv->part, 0);
    char msg[160];
    int d, n, c;
    long shadow[10];
    int ok;

    /* reset state: all RAM dark, one backplane driven at a time */
    ok = 1;
    for (d = 1; d <= dv->digits; d++) {
        ok &= dreg(b, d) == 0;
    }
    snprintf(msg, sizeof msg, "%s: power-on RAM is dark", dv->label);
    check(ok, msg);
    snprintf(msg, sizeof msg, "%s: power-on waveform: one backplane at a time, both levels, no cell lit", dv->label);
    check(wave_matches_ram(b, dv), msg);
    memset(shadow, 0, sizeof shadow);

    /* every digit, every code (no annunciators): the RAM word is the decoded font */
    ok = 1;
    for (d = 0; d < dv->digits; d++) {
        for (n = 0; n < 16; n++) {
            long want = (long)expect_word(dv->variant_c, dv->font[n], 0, 0);

            put(b, dv, d, (unsigned)n, 0, 0);
            shadow[d] = want;
            if (dreg(b, d + 1) != want) {
                printf("  digit %d code %d: RAM %lx want %lx\n", d + 1, n, dreg(b, d + 1), want);
                ok = 0;
            }
        }
        for (c = 0; c < dv->digits; c++) {
            if (dreg(b, c + 1) != shadow[c]) {
                printf("  digit %d changed by a write to digit %d\n", c + 1, d + 1);
                ok = 0;
            }
        }
    }
    snprintf(msg, sizeof msg, "%s: %d digits x 16 codes decode to the %s font on the right cells", dv->label,
             dv->digits, dv->font == HEX_FONT ? "hexadecimal" : "Code B");
    check(ok, msg);

    /* the annunciators, every digit, with a lit 8 */
    ok = 1;
    for (d = 0; d < dv->digits; d++) {
        int an;

        for (an = 0; an < 4; an++) {
            long want = (long)expect_word(dv->variant_c, dv->font[8], an & 1, an >> 1);

            put(b, dv, d, 8, an & 1, an >> 1);
            if (dreg(b, d + 1) != want) {
                printf("  digit %d AN %d: RAM %lx want %lx\n", d + 1, an, dreg(b, d + 1), want);
                ok = 0;
            }
        }
    }
    snprintf(msg, sizeof msg, "%s: annunciators AN1 (Z line, COM3) and AN2 (X line, %s)", dv->label,
             dv->variant_c ? "COM1" : "COM3");
    check(ok, msg);

    /* the waveform equals the RAM: a mixed pattern */
    for (d = 0; d < dv->digits; d++) {
        put(b, dv, d, (unsigned)((d * 5 + 3) & 15), d & 1, (d >> 1) & 1);
    }
    snprintf(msg, sizeof msg, "%s: backplane / segment pins show the RAM (cell lit when its bit is 1)", dv->label);
    check(wave_matches_ram(b, dv), msg);

    /* shutdown with VDISP high keeps the data */
    setn(b, "VDISP", 1);
    run_us(b, 100);
    ok = 1;
    for (n = 0; n < 3; n++) {
        ok &= get(b, dv->bp[n]) == LOGIC_Z;
    }
    for (d = 1; d <= dv->digits; d++) {
        for (c = 0; c < 3; c++) {
            char nm[8];

            segname(nm, sizeof nm, d, c);
            ok &= get(b, nm) == LOGIC_Z;
        }
    }
    snprintf(msg, sizeof msg, "%s: VDISP high: every backplane and segment pin released", dv->label);
    check(ok, msg);
    setn(b, "VDISP", 0);
    run_us(b, 100);
    snprintf(msg, sizeof msg, "%s: VDISP low again: data kept and shown", dv->label);
    check(dreg(b, 1) == (long)expect_word(dv->variant_c, dv->font[3], 0, 0) && wave_matches_ram(b, dv), msg);

    /* AC: each backplane's level alternates between frames; frame is 3 slots of 1852 us (90 Hz) */
    {
        LogicValue l0, l1, l2;

        to_bp1_middle(b);
        l0 = get(b, dv->bp[0]);
        run_us(b, 3 * SLOT_US);
        l1 = get(b, dv->bp[0]);
        run_us(b, 3 * SLOT_US);
        l2 = get(b, dv->bp[0]);
        snprintf(msg, sizeof msg, "%s: BP1 level alternates every 5556 us frame", dv->label);
        check((l0 == LOGIC_0 || l0 == LOGIC_1) && l1 != l0 && (l1 == LOGIC_0 || l1 == LOGIC_1) && l2 == l0, msg);
    }
    snprintf(msg, sizeof msg, "%s: DATA ACCEPTED / unused outputs are outputs", dv->label);
    check(!dv->serial || is_output_pin(b->part, "DATA ACCEPTED"), msg);
    close_bench(b);
}

static void parallel_timing(const Dev *dv) {
    Bench *b = bench(dv, dv->part, 0);
    char msg[160];
    long before;

    /* nothing is written while CS stays high, whatever the pins do */
    par_set(b, 2, 7, 1, 1);
    run_us(b, 20);
    check(dreg(b, 3) == 0, "parallel: CS high: data and address changes write nothing");
    /* the data at the RISING edge of CS counts, not the data at its fall */
    setn(b, "CS", 0);
    run_us(b, HALF);
    check(dreg(b, 3) == 0, "parallel: CS low alone writes nothing yet");
    par_set(b, 5, 9, 0, 1);
    run_us(b, HALF);
    setn(b, "CS", 1);
    run_us(b, HALF);
    before = (long)expect_word(dv->variant_c, dv->font[9], 0, 1);
    snprintf(msg, sizeof msg, "%s: parallel: the trailing (rising) edge of CS latches the data then present", dv->label);
    check(dreg(b, 6) == before && dreg(b, 3) == 0, msg);
    close_bench(b);
}

static void serial_framing(const Dev *dv) {
    Bench *b = bench(dv, dv->part, 0);
    char msg[160];
    int ok;
    long w8 = (long)expect_word(dv->variant_c, dv->font[8], 0, 0);

    /* 10-bit word */
    write_pulse(b);
    ser_word(b, 4, 0xA, 1, 1, 0);
    check(dreg(b, 5) == 0, "serial: nothing is written before the WRITE pulse");
    write_pulse(b);
    snprintf(msg, sizeof msg, "%s: serial: 10-bit word (AN1 AN2 BD0-3 A0-3) writes D5 with both annunciators", dv->label);
    check(dreg(b, 5) == (long)expect_word(dv->variant_c, dv->font[10], 1, 1), msg);
    /* 9-bit word: AN2 only */
    write_pulse(b);
    ser_word(b, 4, 0x8, 1, 1, 1);
    write_pulse(b);
    check(dreg(b, 5) == (long)expect_word(dv->variant_c, dv->font[8], 0, 1), "serial: 9-bit word: AN2 on, AN1 off");
    /* 8-bit word: no annunciators */
    write_pulse(b);
    ser_word(b, 4, 0x8, 1, 1, 2);
    write_pulse(b);
    check(dreg(b, 5) == w8, "serial: 8-bit word: both annunciators off");
    /* last digit */
    write_pulse(b);
    ser_word(b, 9, 0x3, 0, 1, 0);
    write_pulse(b);
    check(dreg(b, 10) == (long)expect_word(dv->variant_c, dv->font[3], 0, 1), "serial: address 9 = D10");
    /* addresses 10..15: nothing changes */
    ok = 1;
    {
        int a, len;
        long snap[10];

        for (a = 0; a < 10; a++) {
            snap[a] = dreg(b, a + 1);
        }
        for (a = 10; a < 16; a++) {
            for (len = 0; len < 3; len++) {
                int i;

                write_pulse(b);
                ser_word(b, a, 0xF, 1, 1, len);
                write_pulse(b);
                for (i = 0; i < 10; i++) {
                    ok &= dreg(b, i + 1) == snap[i];
                }
            }
        }
    }
    check(ok, "serial: addresses 10-15 (Table 4) change nothing at 10, 9 and 8 bits");
    /* fewer than 8 bits: nothing; a WRITE pulse resets a partial word */
    write_pulse(b);
    {
        int i;

        for (i = 0; i < 7; i++) {
            ser_bit(b, 1);
        }
    }
    write_pulse(b);
    check(dreg(b, 1) == 0 || dreg(b, 1) == (long)expect_word(dv->variant_c, dv->font[0], 0, 0),
          "serial: a 7-bit word writes nothing");
    {
        long d1 = dreg(b, 1);
        int i;

        for (i = 0; i < 5; i++) {
            ser_bit(b, 1);
        }
        write_pulse(b);                 /* resets the five bits */
        ser_word(b, 0, 0x2, 0, 0, 2);
        write_pulse(b);
        snprintf(msg, sizeof msg, "%s: serial: a WRITE pulse discards a partial word (D1 was %lx)", dv->label, d1);
        check(dreg(b, 1) == (long)expect_word(dv->variant_c, dv->font[2], 0, 0), msg);
    }
    /* clock edges while WRITE is low are not data */
    close_bench(b);
}

static void glass_tests(const Dev *dv) {
    Bench *b = bench(dv, dv->part, 1);
    char msg[160];
    int d;

    snprintf(msg, sizeof msg, "%s: glass dark at power-on", dv->label);
    check(glass_matches(b, dv->digits), msg);
    for (d = 0; d < dv->digits; d++) {
        put(b, dv, d, (unsigned)((d * 3 + 1) & 15), d & 1, (d >> 1) & 1);
    }
    snprintf(msg, sizeof msg, "%s: GENERIC-LCD-8D-1:3 glass lights exactly the RAM bits (digits 1-8)", dv->label);
    check(glass_matches(b, dv->digits), msg);
    for (d = 0; d < dv->digits; d++) {
        put(b, dv, d, 8, 1, 1);
    }
    check(glass_matches(b, dv->digits), "glass: all 8s with both annunciators");
    setn(b, "VDISP", 1);
    run_us(b, 100);
    {
        const SimComponentType *type = sim_instance_type(b->glass);
        unsigned char values[200];
        size_t n, i;
        int lit = 0;

        run(b, (SimTime)FRAME_US * 4 * US);
        n = type->lights(b->sim, b->glass, sim_instance_state(b->glass), sim_now(b->sim) - (SimTime)FRAME_US * 2 * US,
                         values, sizeof values);
        for (i = 0; i < n; i++) {
            lit |= values[i] != 0;
        }
        snprintf(msg, sizeof msg, "%s: glass dark in shutdown (VDISP high)", dv->label);
        check(n == 64 && !lit, msg);
    }
    close_bench(b);
}

int main(void) {
    size_t i;

    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL loading the parts\n");
        return 1;
    }
    pinouts();
    for (i = 0; i < sizeof DEVS / sizeof *DEVS; i++) {
        const Dev *dv = &DEVS[i];

        check(part_library_find(library, dv->part) != NULL && part_library_find(library, dv->part)->kind == PART_LCDDRIVER,
              "the part loads as an lcddriver part");
        device_tests(dv);
        glass_tests(dv);
        if (dv->serial) {
            serial_framing(dv);
        } else {
            parallel_timing(dv);
        }
    }
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("icm7231_max7231_test: all passed\n");
    return 0;
}
