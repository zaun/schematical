/*
 * max6950_test (plan-56-D): the MAX6950 (5 digits) and MAX6951 (8 digits) SPI common-cathode LED drivers.
 * Each bench wires one LSHD-7503 single-digit common-cathode display per digit exactly as Table 1 of the
 * datasheet does (digit k's two cathode pins on DIG/SEGk, its segments dp g f e d c b a on the other eight
 * pins in column order), writes the registers through the real SPI pins (16-bit frames, MSB first,
 * mode 0) and reads what the displays light. The chip's own pins are sampled for the digit scan.
 *
 * Datasheet: max6950-max6951.pdf (Maxim 19-2227 Rev 2).
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

/* ---- The bench ----------------------------------------------------------------------------------- */

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
    SimInstance *chip, *drv, *disp[8];
    const Part *part;
    PinCount pins;
    int digits;
} Bench;

static const SimTime US = 1000000;
static const SimTime MS = 1000000000;
static const SimTime HALF = 100000;

static const char *const SHARED[9] = {"DIG0/SEG0", "DIG1/SEG1", "DIG2/SEG2", "DIG3/SEG3", "DIG4/SEG4",
                                      "DIG5/SEG5", "DIG6/SEG6", "DIG7/SEG7", "SEG8"};
static const char *const SHARED_6950[9] = {"DIG0/SEG0", "DIG1/SEG1", "DIG2/SEG2", "DIG3/SEG3", "DIG4/SEG4",
                                           "SEG5", "SEG6", "SEG7", "SEG8"};
/* The segment each data line carries: dp g f e d c b a (Table 1), as an index a=0 .. g=6, dp=7. */
static const int LINE_SEG[8] = {7, 6, 5, 4, 3, 2, 1, 0};
static const char *const SEGNAME[8] = {"A", "B", "C", "D", "E", "F", "G", "DP"};

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

static const char *shared_name(const Bench *b, int q) {
    return b->digits == 8 ? SHARED[q] : SHARED_6950[q];
}

static Bench *bench(const char *part, int digits) {
    Bench *b = calloc(1, sizeof *b);
    const Part *disp = part_library_find(library, "LSHD-7503");
    SimNet **nets;
    size_t i, k;
    int d;

    b->part = part_library_find(library, part);
    if (b->part == NULL || disp == NULL) {
        fprintf(stderr, "no part %s\n", part);
        exit(2);
    }
    b->digits = digits;
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
    for (d = 0; d < digits; d++) {
        char ref[8];
        int q, j = 0;

        snprintf(ref, sizeof ref, "DS%d", d + 1);
        b->disp[d] = sim_add_instance(b->sim, kind_model(disp), ref, disp);
        for (k = 0; k < disp->pin_count; k++) {
            if (strcmp(disp->pins[k].name, "CC") == 0) {
                sim_connect(b->sim, nets[bpin(b, shared_name(b, d))], b->disp[d], k);
            }
        }
        for (q = 0; q < 9; q++) {
            size_t dp;

            if (q == d) {
                continue;
            }
            dp = pin_of(disp, SEGNAME[LINE_SEG[j++]]);
            sim_connect(b->sim, nets[bpin(b, shared_name(b, q))], b->disp[d], dp);
        }
    }
    free(nets);
    if (!sim_start(b->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(b->sim));
        exit(2);
    }
    set(b, "CS", LOGIC_1);
    set(b, "CLK", LOGIC_0);
    run(b, HALF);
    return b;
}

static void close_bench(Bench *b) {
    sim_destroy(b->sim);
    free(b);
}

/* ---- SPI: CS low, `bits` bits of `word` MSB first on DIN with CLK rising, CS high -------------- */

static void frame(Bench *b, unsigned long word, unsigned bits) {
    unsigned i;

    set(b, "CS", LOGIC_0);
    run(b, HALF);
    for (i = 0; i < bits; i++) {
        set(b, "DIN", (word >> (bits - 1 - i)) & 1u ? LOGIC_1 : LOGIC_0);
        run(b, HALF);
        set(b, "CLK", LOGIC_1);
        run(b, HALF);
        set(b, "CLK", LOGIC_0);
    }
    run(b, HALF);
    set(b, "CS", LOGIC_1);
    run(b, HALF);
}

static void wr(Bench *b, unsigned addr, unsigned data) {
    frame(b, ((unsigned long)addr << 8) | data, 16);
}

/* CONFIG: S normal, B fast blink, E blink enable, T sync, R clear (Table 5). */
static void config(Bench *b, int s, int bl, int e, int t, int r) {
    wr(b, 0x04, (unsigned)((r ? 0x20 : 0) | (t ? 0x10 : 0) | (e ? 0x08 : 0) | (bl ? 0x04 : 0) | (s ? 1 : 0)));
}

/* ---- What the displays show --------------------------------------------------------------------- */

/* The segments of display d lit at any time since `since`: bit 0 a ... bit 6 g, bit 7 dp. */
static unsigned lit_since(Bench *b, int d, SimTime since) {
    const SimComponentType *type = sim_instance_type(b->disp[d]);
    unsigned char values[32];
    unsigned out = 0;
    size_t n = type->lights(b->sim, b->disp[d], sim_instance_state(b->disp[d]), since, values, sizeof values), i;

    for (i = 0; i < n && i < 8; i++) {
        if (values[i]) {
            out |= 1u << i;
        }
    }
    return out;
}

/* Two full 8-digit frames (3.2 ms) after the last write: what each display shows. */
static void shows(Bench *b, unsigned *out) {
    SimTime since = sim_now(b->sim);
    int d;

    run(b, 4 * MS);
    for (d = 0; d < b->digits; d++) {
        out[d] = lit_since(b, d, since);
    }
}

static int shows_are(Bench *b, const unsigned *expect) {
    unsigned got[8];
    int d, ok = 1;

    shows(b, got);
    for (d = 0; d < b->digits; d++) {
        if (got[d] != expect[d]) {
            printf("     digit %d shows %02X, expected %02X\n", d, got[d], expect[d]);
            ok = 0;
        }
    }
    return ok;
}

/* Hexadecimal font a-g as bits 0-6 (Table 15, p.14). */
static const unsigned HEX[16] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
                                 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};

/* The segments (a-g bit 0-6, dp bit 7) a raw byte lights (Table 16, p.15: D7 dp, D6 a, D5 b, D4 c, D3 d, D2 e, D1 f, D0 g). */
static unsigned raw_of(unsigned v) {
    return (((v >> 6) & 1u) << 0) | (((v >> 5) & 1u) << 1) | (((v >> 4) & 1u) << 2) | (((v >> 3) & 1u) << 3) |
           (((v >> 2) & 1u) << 4) | (((v >> 1) & 1u) << 5) | ((v & 1u) << 6) | (((v >> 7) & 1u) << 7);
}

/* The selected (low) digit pin now: its index, -1 none, -2 several. Every other shared pin must be 1 or Z. */
static int selected(Bench *b) {
    int q, sel = -1;

    for (q = 0; q < 9; q++) {
        LogicValue v = get(b, shared_name(b, q));

        if (v == LOGIC_0) {
            sel = sel == -1 ? q : -2;
        } else if (v != LOGIC_1 && v != LOGIC_Z) {
            return -3;
        }
    }
    return sel;
}

static unsigned registers_of(Bench *b, const char *name, int *found) {
    const SimComponentType *type = sim_instance_type(b->chip);
    ConverterRegister regs[64];
    size_t n = type->registers(sim_instance_state(b->chip), regs, 64), i;

    *found = 0;
    for (i = 0; i < n && i < 64; i++) {
        if (strcmp(regs[i].name, name) == 0) {
            *found = 1;
            return regs[i].value;
        }
    }
    return 0;
}

static unsigned reg(Bench *b, const char *name) {
    int found;
    unsigned v = registers_of(b, name, &found);

    if (!found) {
        fprintf(stderr, "no register %s\n", name);
        exit(2);
    }
    return v;
}

/* ---- One device --------------------------------------------------------------------------------- */

static void device(const char *part, int n) {
    Bench *b = bench(part, n);
    char text[200];
    unsigned expect[8], zero[8] = {0};
    int d, k, ok, seen[9], q, order_ok, prev, steps;
    SimTime t0, base;

    snprintf(text, sizeof text, "%s: a leddriver part with 16 pins", part);
    check(b->part->kind == PART_LEDDRIVER && b->part->pin_count == 16, text);

    /* power-up: shutdown, every driver high impedance, registers at their Table 4 values */
    run(b, 3 * MS);
    ok = 1;
    for (q = 0; q < 9; q++) {
        ok &= get(b, shared_name(b, q)) == LOGIC_Z;
    }
    snprintf(text, sizeof text, "%s: power-up is shutdown: all nine digit/segment pins high impedance", part);
    check(ok, text);
    snprintf(text, sizeof text, "%s: power-up registers: decode 0, scan limit 4 (five digits), configuration 0, test 0, digits 0", part);
    check(reg(b, "DECODE") == 0 && reg(b, "SCAN") == 4 && reg(b, "CONFIG") == 0 && reg(b, "TEST") == 0 &&
              reg(b, "INTENSITY") == 0 && reg(b, "D0P0") == 0 && reg(b, "D3P1") == 0,
          text);

    /* data written during shutdown is kept and shows when the chip is switched on (p.7) */
    for (d = 0; d < n; d++) {
        wr(b, 0x60 + (unsigned)d, 0x80u | (unsigned)(1 << (d % 7)));     /* both planes: dp + one segment */
    }
    for (d = 0; d < n; d++) {
        expect[d] = 0;
    }
    snprintf(text, sizeof text, "%s: digit data written in shutdown is not displayed", part);
    check(shows_are(b, expect), text);
    config(b, 1, 0, 0, 0, 0);
    wr(b, 0x03, 7);
    for (d = 0; d < n; d++) {
        expect[d] = raw_of(0x80u | (unsigned)(1 << (d % 7)));
    }
    snprintf(text, sizeof text, "%s: switched on with all digits scanned: each digit shows its own raw byte (Table 16 order, point on D7)", part);
    check(shows_are(b, expect), text);

    /* the scan: digit pin k low in slot k, 200 us per digit, the other pins carry that digit's segments */
    order_ok = 1;
    prev = -9;
    steps = 0;
    for (k = 0; k < 9; k++) {
        seen[k] = 0;
    }
    t0 = 0;
    for (k = 0; k < 400; k++) {
        int s = selected(b);

        if (s < 0) {
            order_ok = 0;
        } else {
            seen[s]++;
            if (s != prev) {
                if (prev >= 0 && s != (prev + 1) % n) {
                    order_ok = 0;
                }
                if (prev >= 0 && t0 != 0 && sim_now(b->sim) - t0 > 205 * US) {
                    order_ok = 0;
                }
                steps++;
                t0 = sim_now(b->sim);
            }
            prev = s;
        }
        run(b, 20 * US);
    }
    for (k = 0; k < n; k++) {
        order_ok &= seen[k] > 0;
    }
    snprintf(text, sizeof text, "%s: the scan selects digit pin 0, 1, ... %d in turn, 200 us each", part, n - 1);
    check(order_ok && steps >= 2 * n, text);

    /* Table 1: while digit k is selected the pins that are not its cathode are high only for its lit segments */
    {
        int slot_ok = 1;

        for (k = 0; k < 300; k++) {
            int s = selected(b);

            if (s >= 0 && s < n) {
                unsigned want = raw_of(0x80u | (unsigned)(1 << (s % 7))), got_s = 0;
                int j = 0;

                for (q = 0; q < 9; q++) {
                    if (q == s) {
                        continue;
                    }
                    if (get(b, shared_name(b, q)) == LOGIC_1) {
                        got_s |= 1u << LINE_SEG[j];
                    } else if (get(b, shared_name(b, q)) != LOGIC_Z) {
                        slot_ok = 0;
                    }
                    j++;
                }
                if (got_s != want) {
                    slot_ok = 0;
                }
            }
            run(b, 17 * US);
        }
        snprintf(text, sizeof text, "%s: in digit k's slot the data pins are 1 for its lit segments and released for the rest", part);
        check(slot_ok, text);
    }

    /* each raw bit alone, on digit 0: D0 g, D1 f, D2 e, D3 d, D4 c, D5 b, D6 a, D7 dp */
    ok = 1;
    for (k = 0; k < 8; k++) {
        wr(b, 0x60, 1u << k);
        for (d = 0; d < n; d++) {
            expect[d] = d == 0 ? raw_of(1u << k) : raw_of(0x80u | (unsigned)(1 << (d % 7)));
        }
        ok &= shows_are(b, expect);
    }
    snprintf(text, sizeof text, "%s: no-decode: data bit D7..D0 lights dp a b c d e f g (Table 16)", part);
    check(ok, text);

    /* hexadecimal decode, per digit */
    wr(b, 0x01, 0x04);     /* digit 2 only */
    ok = 1;
    for (k = 0; k < 16; k++) {
        wr(b, 0x62, (unsigned)k | 0x70u);                 /* bits 6-4 are ignored by the decoder */
        for (d = 0; d < n; d++) {
            expect[d] = d == 0 ? raw_of(1u << 7) : d == 2 ? HEX[k] : raw_of(0x80u | (unsigned)(1 << (d % 7)));
        }
        ok &= shows_are(b, expect);
    }
    snprintf(text, sizeof text, "%s: hexadecimal decode of digit 2 (0-9 A b C d E F, D6-D4 ignored); the other digits stay raw", part);
    check(ok, text);
    wr(b, 0x62, 0x80u | 0x06u);                            /* decoded 6 with the point */
    for (d = 0; d < n; d++) {
        expect[d] = d == 0 ? raw_of(1u << 7) : d == 2 ? (HEX[6] | 0x80u) : raw_of(0x80u | (unsigned)(1 << (d % 7)));
    }
    snprintf(text, sizeof text, "%s: the decimal point (D7 = 1) is independent of the decoder", part);
    check(shows_are(b, expect), text);
    wr(b, 0x01, (1u << n) - 1u);
    for (d = 0; d < n; d++) {
        wr(b, 0x60 + (unsigned)d, (unsigned)(d + 8));
    }
    for (d = 0; d < n; d++) {
        expect[d] = HEX[d + 8];
    }
    snprintf(text, sizeof text, "%s: decode mode 0xFF: every digit decoded", part);
    check(shows_are(b, expect), text);

    /* scan limit */
    for (d = 0; d < n; d++) {
        wr(b, 0x60 + (unsigned)d, 0x08u | (unsigned)d);
    }
    ok = 1;
    for (k = 0; k < 8; k++) {
        wr(b, 0x03, (unsigned)k);
        for (d = 0; d < n; d++) {
            expect[d] = d <= k ? HEX[d] : 0;
        }
        for (d = 0; d < n; d++) {
            wr(b, 0x60 + (unsigned)d, (unsigned)d);
        }
        ok &= shows_are(b, expect);
    }
    snprintf(text, sizeof text, "%s: scan limit N shows digits 0..N only (N = 0..7)", part);
    check(ok, text);
    wr(b, 0x03, 0);
    ok = 1;
    for (k = 0; k < 200; k++) {
        ok &= selected(b) <= 0;
        run(b, 23 * US);
    }
    snprintf(text, sizeof text, "%s: with scan limit 0 only digit pin 0 ever goes low", part);
    check(ok, text);

    /* display test: every segment of every digit, over the scan limit, the decode and shutdown */
    wr(b, 0x07, 1);
    for (d = 0; d < n; d++) {
        expect[d] = 0xFF;
    }
    snprintf(text, sizeof text, "%s: display test lights every segment of every digit (eight scanned despite scan limit 0)", part);
    check(shows_are(b, expect), text);
    config(b, 0, 0, 0, 0, 0);
    snprintf(text, sizeof text, "%s: display test overrides shutdown", part);
    check(shows_are(b, expect), text);
    wr(b, 0x07, 0);
    for (d = 0; d < n; d++) {
        expect[d] = 0;
    }
    snprintf(text, sizeof text, "%s: test off in shutdown: dark again, pins released", part);
    check(shows_are(b, expect), text);
    ok = 1;
    for (q = 0; q < 9; q++) {
        ok &= get(b, shared_name(b, q)) == LOGIC_Z;
    }
    check(ok, "shutdown releases every pin");
    config(b, 1, 0, 0, 0, 0);
    wr(b, 0x03, 7);

    /* the clear bit */
    wr(b, 0x01, 0);
    for (d = 0; d < n; d++) {
        wr(b, 0x60 + (unsigned)d, 0xFF);
    }
    for (d = 0; d < n; d++) {
        expect[d] = 0xFF;
    }
    check(shows_are(b, expect), "all-ones raw bytes light every segment");
    config(b, 1, 0, 0, 0, 1);
    for (d = 0; d < n; d++) {
        expect[d] = 0;
    }
    snprintf(text, sizeof text, "%s: configuration R = 1 clears the digit data of both planes (raw digits go dark)", part);
    check(shows_are(b, expect) && reg(b, "D0P0") == 0 && reg(b, "D2P1") == 0 && reg(b, "CONFIG") == 1, text);
    wr(b, 0x01, 0xFF);
    for (d = 0; d < n; d++) {
        expect[d] = HEX[0];
    }
    snprintf(text, sizeof text, "%s: cleared digits with decode enabled show 0", part);
    check(shows_are(b, expect), text);
    wr(b, 0x01, 0);

    /* more or fewer than 16 bits: the last 16 are used (p.6) */
    wr(b, 0x60, 0);
    frame(b, (0xA5ul << 16) | 0x6045ul, 24);               /* 8 junk bits then 0x60, 0x45: digit 0 both planes = 0x45 */
    for (d = 0; d < n; d++) {
        expect[d] = d == 0 ? raw_of(0x45) : 0;
    }
    snprintf(text, sizeof text, "%s: with more than 16 bits clocked only the last 16 are taken", part);
    check(shows_are(b, expect), text);
    frame(b, 0x61ul, 8);                                   /* 8 bits only: not a 16-bit word */
    check(shows_are(b, expect), "a frame shorter than 16 bits changes nothing");

    /* ignored addresses: no-op, reserved 0x06, unlisted, D15 set */
    wr(b, 0x00, 0xFF);
    wr(b, 0x06, 0xFF);
    wr(b, 0x05, 0xFF);
    wr(b, 0x28, 0xFF);
    wr(b, 0x68, 0xFF);
    frame(b, 0x8160ul | 0x0000, 16);
    snprintf(text, sizeof text, "%s: no-op, the reserved 0x06 and unlisted addresses change nothing", part);
    check(shows_are(b, expect) && reg(b, "DECODE") == 0 && reg(b, "TEST") == 0 && reg(b, "CONFIG") == 1, text);

    /* intensity is stored and nothing else */
    wr(b, 0x02, 0xA7);
    snprintf(text, sizeof text, "%s: the intensity register keeps D3-D0 and does not change the display", part);
    check(reg(b, "INTENSITY") == 7 && shows_are(b, expect), text);

    /* plane writes and blink */
    config(b, 1, 0, 0, 0, 0);
    wr(b, 0x60, 0x00);
    wr(b, 0x20, 0x01);     /* digit 0 P0: g */
    wr(b, 0x41, 0x40);     /* digit 1 P1: a */
    for (d = 0; d < n; d++) {
        expect[d] = d == 0 ? raw_of(0x01) : 0;
    }
    snprintf(text, sizeof text, "%s: blink off: the digit shows plane P0 (a P1-only write is invisible)", part);
    check(shows_are(b, expect) && reg(b, "D0P0") == 1 && reg(b, "D0P1") == 0 && reg(b, "D1P1") == 0x40 && reg(b, "D1P0") == 0, text);
    wr(b, 0x40, 0x40);     /* digit 0 P1: a */
    wr(b, 0x21, 0x01);     /* digit 1 P0: g */
    config(b, 1, 0, 1, 0, 0);
    /* the blink phase counts from time 0: P0 in 0-1 s, P1 in 1-2 s, ... (half period 1 s, Table 7) */
    ok = 1;
    base = (sim_now(b->sim) / (2000 * MS) + 1) * 2000 * MS;
    for (k = 0; k < 4; k++) {
        SimTime start = base + (SimTime)(k * 1000 + 100) * MS;
        unsigned want0 = k % 2 == 0 ? raw_of(0x01) : raw_of(0x40), want1 = k % 2 == 0 ? raw_of(0x01) : raw_of(0x40);

        run(b, start - sim_now(b->sim));
        t0 = sim_now(b->sim);
        run(b, 800 * MS);
        if (lit_since(b, 0, t0) != want0 || lit_since(b, 1, t0) != want1) {
            ok = 0;
            printf("     second %d: digit 0 %02X digit 1 %02X\n", k, lit_since(b, 0, t0), lit_since(b, 1, t0));
        }
    }
    snprintf(text, sizeof text, "%s: slow blink (E = 1, B = 0): P0 for 1 s, P1 for 1 s", part);
    check(ok, text);
    config(b, 1, 1, 1, 0, 0);
    ok = 1;
    base = (sim_now(b->sim) / (1000 * MS) + 1) * 1000 * MS;
    for (k = 0; k < 6; k++) {
        SimTime start = base + (SimTime)(k * 500 + 50) * MS;
        unsigned want = (((base / MS + (SimTime)k * 500) / 500) % 2) == 0 ? raw_of(0x01) : raw_of(0x40);

        run(b, start - sim_now(b->sim));
        t0 = sim_now(b->sim);
        run(b, 400 * MS);
        if (lit_since(b, 0, t0) != want) {
            ok = 0;
            printf("     fast half %d: digit 0 %02X\n", k, lit_since(b, 0, t0));
        }
    }
    snprintf(text, sizeof text, "%s: fast blink (B = 1): half period 0.5 s", part);
    check(ok, text);
    config(b, 1, 1, 0, 0, 0);
    t0 = sim_now(b->sim);
    run(b, 2500 * MS);
    snprintf(text, sizeof text, "%s: blink disabled again (E = 0): plane P0 only", part);
    check(lit_since(b, 0, t0) == raw_of(0x01) && lit_since(b, 1, t0) == raw_of(0x01), text);
    (void)zero;
    close_bench(b);
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL loading the parts\n");
        return 1;
    }
    device("MAX6951CEE+", 8);
    device("MAX6950CEE+", 5);
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("max6950_test: all passed\n");
    return 0;
}
