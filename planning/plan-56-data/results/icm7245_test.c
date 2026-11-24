/*
 * icm7245_test (plan-56-C): the ICM7244 and ICM7245 8-character 16-segment LED display decoder drivers
 * (datasheets/led-drivers/icm7244-renesas.pdf, icm7245-renesas.pdf), through the real write port:
 * random access (A2 A1 A0 = character), sequential access (SEN, internal counter, CLR), the
 * 64-character font of Figure 9 on the segment pins, and the character scan (CHAR pins active
 * low, open drain, 320 us per character).
 * Needs the `ascii16` decode and 17 segment pins (see engineNeeds in the result file).
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
    SimInstance *chip, *drv;
    const Part *part;
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

static Bench *bench(const char *part) {
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
    for (i = 0; i < b->part->pin_count; i++) {
        SimNet *net = sim_add_net(b->sim, b->part->pins[i].name);

        sim_connect(b->sim, net, b->chip, i);
        sim_connect(b->sim, net, b->drv, i);
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

static void data(Bench *b, unsigned v) {
    static const char *const D[6] = {"D0", "D1", "D2", "D3", "D4", "D5"};
    unsigned i;

    for (i = 0; i < 6; i++) {
        set(b, D[i], lv((v >> i) & 1u));
    }
}

/* ---- The write port: CS high, WR low for 1 us (data and address valid), WR high ------------------ */

static int entries;

static void idle(Bench *b, int sequential) {
    set(b, "CS", LOGIC_1);
    set(b, "WR", LOGIC_1);
    set(b, "MODE", lv((unsigned)sequential));
    set(b, "OSC/OFF", LOGIC_1);
    set(b, "A0/SEN", LOGIC_0);
    set(b, "A1/CLR", LOGIC_1);
    entries = 0;
    set(b, "A2/DISP FULL", LOGIC_0);
    data(b, 0);
    run(b, 10 * US);
}

static void write_ra(Bench *b, unsigned address, unsigned code) {
    set(b, "A0/SEN", lv(address & 1u));
    set(b, "A1/CLR", lv((address >> 1) & 1u));
    set(b, "A2/DISP FULL", lv((address >> 2) & 1u));
    data(b, code);
    run(b, 1 * US);
    set(b, "WR", LOGIC_0);
    run(b, 1 * US);
    set(b, "WR", LOGIC_1);
    run(b, 1 * US);
}

/* Sequential access: SEN (A0) high and CLR (A1) high. A2 is the chip's DISP FULL output; the model does not
   drive it yet (engineNeeds), and an address pin that reads Z makes the whole address unknown, so the bench
   drives it with the level the chip would: high once eight entries are in. */
static void write_sa(Bench *b, unsigned code, int sen, int clr) {
    set(b, "A0/SEN", lv((unsigned)sen));
    set(b, "A1/CLR", lv((unsigned)clr));
    set(b, "A2/DISP FULL", lv(entries >= 8));
    data(b, code);
    run(b, 1 * US);
    set(b, "WR", LOGIC_0);
    run(b, 1 * US);
    set(b, "WR", LOGIC_1);
    run(b, 1 * US);
    set(b, "A1/CLR", LOGIC_1);
    run(b, 1 * US);
    if (!clr) {
        entries = 0;
    } else if (sen && entries < 8) {
        entries++;
    }
    set(b, "A2/DISP FULL", lv(entries >= 8));
}

static const char *const FONT[64] = {
    "a1 a2 b d1 d2 e f g2 i",   /* 0x00 @ */
    "a1 a2 b c e f g1 g2",   /* 0x01 A */
    "a1 a2 b c d1 d2 g2 i l",   /* 0x02 B */
    "a1 a2 d1 d2 e f",   /* 0x03 C */
    "a1 a2 b c d1 d2 i l",   /* 0x04 D */
    "a1 a2 d1 d2 e f g1 g2",   /* 0x05 E */
    "a1 a2 e f g1",   /* 0x06 F */
    "a1 a2 c d1 d2 e f g2",   /* 0x07 G */
    "b c e f g1 g2",   /* 0x08 H */
    "i l",   /* 0x09 I */
    "b c d1 d2 e",   /* 0x0A J */
    "e f g1 j k",   /* 0x0B K */
    "d1 d2 e f",   /* 0x0C L */
    "b c e f h j",   /* 0x0D M */
    "b c e f h k",   /* 0x0E N */
    "a1 a2 b c d1 d2 e f",   /* 0x0F O */
    "a1 a2 b e f g1 g2",   /* 0x10 P */
    "a1 a2 b c d1 d2 e f k",   /* 0x11 Q */
    "a1 a2 b e f g1 g2 k",   /* 0x12 R */
    "a1 a2 c d1 d2 f g1 g2",   /* 0x13 S */
    "a1 a2 i l",   /* 0x14 T */
    "b c d1 d2 e f",   /* 0x15 U */
    "e f j m",   /* 0x16 V */
    "b c e f k m",   /* 0x17 W */
    "h j k m",   /* 0x18 X */
    "h j l",   /* 0x19 Y */
    "a1 a2 d1 d2 j m",   /* 0x1A Z */
    "a2 d1 i l",   /* 0x1B [ */
    "h k",   /* 0x1C backslash */
    "a1 d2 i l",   /* 0x1D ] */
    "a2 b j m",   /* 0x1E ^ */
    "d1 d2",   /* 0x1F _ */
    "",   /* 0x20   */
    "i DP",   /* 0x21 ! */
    "b f",   /* 0x22 " */
    "a2 d1 d2 g1 g2 i l",   /* 0x23 # */
    "a1 a2 c d1 d2 f g1 g2 i l",   /* 0x24 $ */
    "a1 c d1 f g1 g2 i j l m",   /* 0x25 % */
    "a2 c d1 d2 i j l m",   /* 0x26 & */
    "i",   /* 0x27 ' */
    "j k",   /* 0x28 ( */
    "h m",   /* 0x29 ) */
    "g1 g2 h i j k l m",   /* 0x2A star */
    "g1 g2 i l",   /* 0x2B + */
    "m",   /* 0x2C , */
    "g1 g2",   /* 0x2D - */
    "DP",   /* 0x2E . */
    "j m",   /* 0x2F / */
    "a1 a2 b c d1 d2 e f j m",   /* 0x30 0 */
    "i l",   /* 0x31 1 */
    "a1 a2 b d1 d2 e g1 g2",   /* 0x32 2 */
    "a1 a2 b c d1 d2 g2",   /* 0x33 3 */
    "b c f g1 g2",   /* 0x34 4 */
    "a1 a2 d1 d2 f g1 k",   /* 0x35 5 */
    "a1 a2 c d1 d2 e f g1 g2",   /* 0x36 6 */
    "a1 a2 b c",   /* 0x37 7 */
    "a1 a2 b c d1 d2 e f g1 g2",   /* 0x38 8 */
    "a1 a2 b c d1 d2 f g1 g2",   /* 0x39 9 */
    "DP",   /* 0x3A : */
    "m",   /* 0x3B ; */
    "d1 d2 m",   /* 0x3C < */
    "d1 d2 g1 g2",   /* 0x3D = */
    "d1 d2 k",   /* 0x3E > */
    "a1 a2 b g2 l",   /* 0x3F ? */
};

/* ---- Reading the scan -------------------------------------------------------------------------- */

static const char *const SEGS[17] = {"SEG a1", "SEG a2", "SEG b", "SEG c", "SEG d1", "SEG d2", "SEG e", "SEG f",
                                     "SEG g1", "SEG g2", "SEG h", "SEG i", "SEG j", "SEG k", "SEG l", "SEG m", "DP"};
static const char *const SEGSHORT[17] = {"a1", "a2", "b", "c", "d1", "d2", "e", "f", "g1", "g2", "h", "i", "j", "k", "l", "m", "DP"};

/* The segment mask the pins show now (bit i = pin SEGS[i] high); 0x1FFFF0 flags a pin that is not 0 or 1. */
static unsigned lit_now(Bench *b) {
    unsigned seg = 0, i;

    for (i = 0; i < 17; i++) {
        LogicValue v = get(b, SEGS[i]);

        if (v == LOGIC_1) {
            seg |= 1u << i;
        } else if (v != LOGIC_0) {
            return 0xFFFFFFu;
        }
    }
    return seg;
}

static unsigned mask_of(const char *names) {
    unsigned m = 0, i;
    char copy[96], *tok, *save = NULL;

    snprintf(copy, sizeof copy, "%s", names);
    for (tok = strtok_r(copy, " ", &save); tok != NULL; tok = strtok_r(NULL, " ", &save)) {
        for (i = 0; i < 17; i++) {
            if (strcmp(tok, SEGSHORT[i]) == 0) {
                m |= 1u << i;
            }
        }
    }
    return m;
}

/* The active character (0-7), -1 none, -2 several; an inactive CHAR pin is released (open drain). */
static int active_char(Bench *b) {
    int active = -1, i;

    for (i = 0; i < 8; i++) {
        char name[8];
        LogicValue v;

        snprintf(name, sizeof name, "CHAR%d", i + 1);
        v = get(b, name);
        if (v == LOGIC_0) {
            active = active == -1 ? i : -2;
        } else if (v != LOGIC_Z) {
            return -3;
        }
    }
    return active;
}

/* Over 3.2 ms (about 1.25 scan cycles), sample every 20 us: the segments seen while character k is
   active equal want[k]. Returns 1 if every character was seen and always matched. */
static int scan_is(Bench *b, const unsigned *want) {
    unsigned seen = 0;
    int ok = 1, t;

    for (t = 0; t < 160; t++) {
        int c = active_char(b);

        if (c >= 0) {
            seen |= 1u << c;
            if (lit_now(b) != want[c]) {
                ok = 0;
                if (getenv("ICM_DEBUG")) {
                    printf("  char %d: pins %05X want %05X\n", c + 1, lit_now(b), want[c]);
                }
            }
        } else if (c != -1) {
            ok = 0;
        }
        run(b, 20 * US);
    }
    return ok && seen == 0xFFu;
}

static void blank_want(unsigned *w) {
    int i;

    for (i = 0; i < 8; i++) {
        w[i] = 0;
    }
}

static void tests(const char *part, const char *label) {
    Bench *b = bench(part);
    unsigned want[8], code, ok;
    char text[200];
    int last, t, order_ok, i, seen_one = 0;
    SimTime changed = 0, now;

    /* ---- random access ---- */
    idle(b, 0);
    blank_want(want);
    snprintf(text, sizeof text, "%s: after power-up every character is blank (the memory reads 100000)", label);
    check(scan_is(b, want), text);
    write_ra(b, 0, 'H' & 0x3F);
    write_ra(b, 1, 'I' & 0x3F);
    write_ra(b, 7, '7' & 0x3F);
    write_ra(b, 4, '!' & 0x3F);
    want[0] = mask_of(FONT['H' & 0x3F]);
    want[1] = mask_of(FONT['I' & 0x3F]);
    want[7] = mask_of(FONT['7' & 0x3F]);
    want[4] = mask_of(FONT['!' & 0x3F]);
    snprintf(text, sizeof text, "%s: random access writes land at A2 A1 A0 (0, 1, 7, 4), one character at a time", label);
    check(scan_is(b, want), text);
    check((want[4] >> 16) == 1, "'!' lights the decimal point pin DP (the font's dot)");

    /* every one of the 64 codes, on character 3 */
    ok = 1;
    for (code = 0; code < 64; code++) {
        unsigned mask;

        write_ra(b, 2, code);
        mask = mask_of(FONT[code]);
        for (t = 0; t < 300; t++) {
            if (active_char(b) == 2) {
                if (lit_now(b) != mask) {
                    ok = 0;
                    printf("  code %02X: pins %05X want %05X\n", code, lit_now(b), mask);
                }
                break;
            }
            run(b, 10 * US);
        }
        if (t == 300) {
            ok = 0;
        }
    }
    snprintf(text, sizeof text, "%s: all 64 codes light the segments of Figure 9", label);
    check(ok, text);

    /* the scan: character k active for 320 us, in order 1..8, repeating every 2.56 ms */
    last = -1;
    order_ok = 1;
    changed = 0;
    now = sim_now(b->sim);
    for (t = 0; t < 600; t++) {
        int c = active_char(b);

        if (c != last) {
            if (c >= 0 && last >= 0 && c != (last + 1) % 8) {
                order_ok = 0;
            }
            if (c >= 0 && last >= 0 && changed != 0 && seen_one) {
                SimTime len = sim_now(b->sim) - changed;

                if (len < 300 * US || len > 340 * US) {
                    order_ok = 0;
                }
            }
            if (c >= 0) {
                seen_one = changed != 0;
                changed = sim_now(b->sim);
            }
            last = c;
        }
        run(b, 10 * US);
    }
    (void)now;
    snprintf(text, sizeof text, "%s: the scan visits characters 1..8 in order, 320 us each", label);
    check(order_ok, text);
    check(get(b, "CHAR1") == LOGIC_Z || get(b, "CHAR1") == LOGIC_0, "CHAR pins are open drain (0 or released, never 1)");
    close_bench(b);

    /* ---- sequential access ---- */
    b = bench(part);
    idle(b, 1);
    for (i = 0; i < 8; i++) {
        write_sa(b, (unsigned)('A' + i) & 0x3F, 1, 1);
    }
    for (i = 0; i < 8; i++) {
        want[i] = mask_of(FONT[('A' + i) & 0x3F]);
    }
    snprintf(text, sizeof text, "%s: sequential access stores entries 1..8 in characters 1..8", label);
    check(scan_is(b, want), text);
    write_sa(b, 'Z' & 0x3F, 1, 1);
    snprintf(text, sizeof text, "%s: a ninth entry is ignored (the counter is full)", label);
    check(scan_is(b, want), text);
    write_sa(b, 'Z' & 0x3F, 1, 0);          /* A1/CLR low: memory to blank, counter to 0 */
    blank_want(want);
    snprintf(text, sizeof text, "%s: A1/CLR low during a write clears the memory", label);
    check(scan_is(b, want), text);
    write_sa(b, '1' & 0x3F, 1, 1);
    write_sa(b, '2' & 0x3F, 0, 1);          /* SEN low: no action */
    write_sa(b, '3' & 0x3F, 1, 1);
    want[0] = mask_of(FONT['1' & 0x3F]);
    want[1] = mask_of(FONT['3' & 0x3F]);
    snprintf(text, sizeof text, "%s: after CLR the first entry is character 1; a write with SEN low is ignored", label);
    check(scan_is(b, want), text);
    close_bench(b);

    /* ---- mode: MODE low is random access even after sequential writes */
    b = bench(part);
    idle(b, 1);
    write_sa(b, 'A' & 0x3F, 1, 1);
    set(b, "MODE", LOGIC_0);
    run(b, 5 * US);
    write_ra(b, 5, 'B' & 0x3F);
    blank_want(want);
    want[0] = mask_of(FONT['A' & 0x3F]);
    want[5] = mask_of(FONT['B' & 0x3F]);
    snprintf(text, sizeof text, "%s: MODE low switches to random access, the earlier entry stays", label);
    check(scan_is(b, want), text);
    close_bench(b);
}

static void pins(void) {
    const Part *p = part_library_find(library, "ICM7245AIM44Z");
    const Part *q = part_library_find(library, "ICM7244BIM44Z");
    int ok;

#define NUM(PART, NAME, DES) (pin_of((PART), (NAME)) != SIZE_MAX && strcmp((PART)->pins[pin_of((PART), (NAME))].designator, (DES)) == 0)
    check(p != NULL && q != NULL && p->kind == PART_LEDDRIVER, "ICM7245AIM44Z and ICM7244BIM44Z exist as leddriver parts");
    ok = NUM(p, "SEG d1", "1") && NUM(p, "D0", "4") && NUM(p, "D5", "9") && NUM(p, "CS", "10") && NUM(p, "WR", "13") &&
         NUM(p, "CHAR8", "14") && NUM(p, "VSS", "18") && NUM(p, "CHAR4", "19") && NUM(p, "CHAR1", "24") && NUM(p, "OSC/OFF", "25") &&
         NUM(p, "A2/DISP FULL", "26") && NUM(p, "A1/CLR", "27") && NUM(p, "A0/SEN", "28") && NUM(p, "MODE", "29") &&
         NUM(p, "SEG j", "30") && NUM(p, "DP", "32") && NUM(p, "SEG d2", "33") && NUM(p, "SEG l", "38") && NUM(p, "VDD", "39") &&
         NUM(p, "SEG m", "40") && NUM(p, "SEG e", "41") && NUM(p, "SEG g1", "42") && NUM(p, "SEG k", "43") && NUM(p, "SEG c", "44");
    check(ok, "ICM7245 pins as p.2 (44-lead MQFP)");
    check(p->vcc != NULL && p->vcc->min_mv == 3000 && p->vcc->max_mv == 3600 && q->vcc->min_mv == 4750 && q->vcc->max_mv == 5250,
          "supply: 7245 3.0-3.6 V (p.4), 7244 4.75-5.25 V");
}

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL loading the parts\n");
        return 1;
    }
    pins();
    tests("ICM7245AIM44Z", "ICM7245");
    tests("ICM7244BIM44Z", "ICM7244");
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("icm7245_test: all passed\n");
    return 0;
}
