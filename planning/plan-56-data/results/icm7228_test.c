/*
 * icm7228_test (plan-56-C): the ICM7228A/B/C/D 8-digit LED display decoder drivers
 * (leddriver.json) through their real microprocessor bus: parallel data pins, MODE,
 * digit address pins and the WRITE strobe (data latch on the rising edge), checked on
 * the scanned digit and segment pins.
 *
 *   A (ICM7228AIBIZ) common anode, control register + sequential / single-digit update
 *   B (ICM7228BIBIZ) common cathode, same protocol, opposite output polarity
 *   C (ICM7228CIBIZ) common anode, digit address + data, no control register
 *   D (ICM7228DIPI)  common cathode, the C protocol
 *
 * Datasheet: datasheets/led-drivers/icm7228-intersil-2015.pdf (A, B, C) and
 * icm7228-intersil-2002.pdf (D). Not modeled (and so not tested): Code B, the RAM
 * banks A/B, the three-level pin 9 of C/D, interdigit blanking.
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

/* ---- The bench: the chip and a test driver on every pin ------------------------------------ */

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

static const SimTime NS = 1000;
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

/* ---- The scan ----------------------------------------------------------------------------- */

static const char *const DIGITS[8] = {"DIGIT 1", "DIGIT 2", "DIGIT 3", "DIGIT 4",
                                      "DIGIT 5", "DIGIT 6", "DIGIT 7", "DIGIT 8"};
static const char *const SEGS[8] = {"SEG a", "SEG b", "SEG c", "SEG d", "SEG e", "SEG f", "SEG g", "DP"};

/* Common anode (A, C): a digit is selected high and a segment is lit low; common cathode (B, D)
   the opposite. */
typedef struct {
    Bench *b;
    int cathode;
} Chip;

/* The selected digit (1-8; 0 none, -1 several) and the lit segments as bits 0-7 (a-g, DP). */
static int sample(const Chip *c, unsigned *segments) {
    LogicValue sel = c->cathode ? LOGIC_0 : LOGIC_1, lit = c->cathode ? LOGIC_1 : LOGIC_0;
    int active = 0, i;

    for (i = 0; i < 8; i++) {
        if (get(c->b, DIGITS[i]) == sel) {
            active = active == 0 ? i + 1 : -1;
        }
    }
    *segments = 0;
    for (i = 0; i < 8; i++) {
        if (get(c->b, SEGS[i]) == lit) {
            *segments |= 1u << i;
        }
    }
    return active;
}

/* Runs a little over two frames (8 slots of 320 us each), sampling every 20 us: in every sample
   exactly one digit is selected, the lit segments equal expect[digit - 1], and every digit is
   seen. */
static int display_is(const Chip *c, const unsigned *expect) {
    unsigned seen = 0;
    int ok = 1, t;

    for (t = 0; t < 140; t++) {
        unsigned seg;
        int d = sample(c, &seg);

        if (d < 1 || seg != expect[d - 1]) {
            ok = 0;
        } else {
            seen |= 1u << (d - 1);
        }
        run(c->b, 20 * US);
    }
    return ok && seen == 0xFFu;
}

/* Every digit pin deselected and no segment lit (shutdown). */
static int display_off(const Chip *c) {
    int ok = 1, t;

    for (t = 0; t < 40; t++) {
        unsigned seg;

        ok &= sample(c, &seg) == 0 && seg == 0;
        run(c->b, 20 * US);
    }
    return ok;
}

/* The multiplex sequence: the selected digit changes every 320 us, in the order D2 D5 D1 D7 D8 D6 D4 D3
   (2015 p.14). */
static void scan_order(const Chip *c) {
    static const int ORDER[8] = {2, 5, 1, 7, 8, 6, 4, 3};
    int last = 0, count = 0, order_ok = 1, step_ok = 1, have = 0, start = -1, t, i;
    SimTime changed = 0;

    for (t = 0; t < 400; t++) {
        unsigned seg;
        int d = sample(c, &seg);

        if (d != last && d > 0) {
            if (last > 0) {
                int next = -1;

                for (i = 0; i < 8; i++) {
                    if (ORDER[i] == last) {
                        next = ORDER[(i + 1) % 8];
                    }
                }
                order_ok &= d == next;
                if (have && sim_now(c->b->sim) - changed != 320 * US) {
                    step_ok = 0;
                }
                changed = sim_now(c->b->sim);
                have = 1;
                count++;
            } else {
                start = d;
            }
            last = d;
        }
        run(c->b, 10 * US);
    }
    check(start > 0 && order_ok && count >= 8, "scan: the digits are selected in the order D2 D5 D1 D7 D8 D6 D4 D3");
    check(step_ok, "scan: each digit is selected for 320 us");
}

/* ---- The bus --------------------------------------------------------------------------------- */

static const char *const AB_DATA[8] = {"ID0", "ID1", "ID2", "ID3", "ID4 (SHUTDOWN)", "ID5 (DECODE)",
                                       "ID6 (HEXA/CODE B)", "ID7 (DATA COMING)"};

static void ab_put(Bench *b, unsigned v) {
    int i;

    for (i = 0; i < 8; i++) {
        set(b, AB_DATA[i], (v >> i) & 1u ? LOGIC_1 : LOGIC_0);
    }
}

/* One write cycle: MODE and the data settle 300 ns before WRITE falls, WRITE is low 200 ns and
   rises (the latching edge) while they are still valid. */
static void ab_write(Bench *b, int mode, unsigned v) {
    set(b, "MODE", mode ? LOGIC_1 : LOGIC_0);
    ab_put(b, v);
    run(b, 300 * NS);
    set(b, "WRITE", LOGIC_0);
    run(b, 200 * NS);
    set(b, "WRITE", LOGIC_1);
    run(b, 300 * NS);
}

enum { DC = 0x80, HEXA = 0x40, NODEC = 0x20, RUN = 0x10, BANKA = 0x08 };

/* A control word: DATA COMING (ID7), hex (ID6 high), no decode (ID5 high), run (ID4 high), bank A
   (ID3), digit address in ID2-0. */
static void ab_control(Bench *b, unsigned bits, unsigned digit) {
    ab_write(b, 1, bits | (digit & 7u));
}

static void ab_start(Bench *b) {
    set(b, "WRITE", LOGIC_1);
    set(b, "MODE", LOGIC_0);
    ab_put(b, 0);
    run(b, 1 * US);
}

static const char *const CD_DATA[5] = {"ID0", "ID1", "ID2", "ID3", "ID7 (INPUT DP)"};
static const char *const CD_ADDR[3] = {"DA0", "DA1", "DA2"};

/* Digit 1-8 (address 0-7), the data nibble and the point input ID7 (low lights the point). */
static void cd_write(Bench *b, unsigned digit, unsigned nibble, int id7) {
    int i;

    for (i = 0; i < 4; i++) {
        set(b, CD_DATA[i], (nibble >> i) & 1u ? LOGIC_1 : LOGIC_0);
    }
    set(b, CD_DATA[4], id7 ? LOGIC_1 : LOGIC_0);
    for (i = 0; i < 3; i++) {
        set(b, CD_ADDR[i], ((digit - 1) >> i) & 1u ? LOGIC_1 : LOGIC_0);
    }
    run(b, 300 * NS);
    set(b, "WRITE", LOGIC_0);
    run(b, 200 * NS);
    set(b, "WRITE", LOGIC_1);
    run(b, 300 * NS);
}

static void cd_start(Bench *b) {
    set(b, "WRITE", LOGIC_1);
    run(b, 1 * US);
}

/* ---- Fonts and raw bytes --------------------------------------------------------------------- */

/* Hexadecimal font (Table 5), segments a-g = bits 0-6. */
static const unsigned HEX[16] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
                                 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};

/* A No Decode byte (Table 6): ID6 a, ID5 b, ID4 c, ID0 d, ID3 e, ID1 f, ID2 g, and the point is lit
   by ID7 LOW. Returns the lit segments as bits 0-7 (a-g, DP). */
static unsigned raw_lit(unsigned id) {
    unsigned lit = 0;

    lit |= ((id >> 6) & 1u) << 0;
    lit |= ((id >> 5) & 1u) << 1;
    lit |= ((id >> 4) & 1u) << 2;
    lit |= ((id >> 0) & 1u) << 3;
    lit |= ((id >> 3) & 1u) << 4;
    lit |= ((id >> 1) & 1u) << 5;
    lit |= ((id >> 2) & 1u) << 6;
    lit |= ((~id >> 7) & 1u) << 7;
    return lit;
}

static unsigned hex_lit(unsigned id) {
    return HEX[id & 0xFu] | (((~id >> 7) & 1u) << 7);
}

static void fill(unsigned *expect, unsigned v) {
    int i;

    for (i = 0; i < 8; i++) {
        expect[i] = v;
    }
}

/* ---- ICM7228A / B -------------------------------------------------------------------------- */

static void ab_chip(const char *part, int cathode) {
    Bench *b = bench(part);
    Chip c = {b, cathode};
    unsigned expect[8], none[8];
    unsigned raw[8] = {0x80, 0x00, 0x7F, 0xFF, 0x41, 0x2A, 0x08, 0x95};
    unsigned dec[8] = {0x05, 0x8A, 0x0F, 0x01, 0x93, 0x04, 0x7F, 0x08};
    char what[160];
    unsigned v;
    int i;

    ab_start(b);
    fill(none, 0);
    snprintf(what, sizeof what, "%s: a leddriver part on a parallel bus", part);
    check(b->part->kind == PART_LEDDRIVER, what);

    /* power-up: no control word yet, the display is off */
    check(display_off(&c), "reset: no control word written, every digit and segment pin is inactive");

    /* sequential update, No Decode: display blanked at the control word, shown after the 8th write */
    ab_control(b, DC | HEXA | NODEC | RUN | BANKA, 0);
    check(display_is(&c, none), "sequential: a control word with DATA COMING blanks the display (digits scan, nothing lit)");
    scan_order(&c);
    for (i = 0; i < 3; i++) {
        ab_write(b, 0, raw[i]);
    }
    check(display_is(&c, none), "sequential: still blank after 3 of 8 writes");
    for (i = 3; i < 8; i++) {
        ab_write(b, 0, raw[i]);
    }
    for (i = 0; i < 8; i++) {
        expect[i] = raw_lit(raw[i]);
    }
    check(display_is(&c, expect), "sequential: the 8th write unblanks; No Decode bytes (a = ID6 ... d = ID0, point lit by ID7 low) show in digits 1-8");
    ab_write(b, 0, 0x7F);
    ab_write(b, 0, 0x7F);
    check(display_is(&c, expect), "sequential: further writes after the 8th are ignored");
    check(raw_lit(0x80) == 0 && raw_lit(0x00) == 0x80, "No Decode: hex 80 lights nothing (the blank byte of p.14), ID7 low lights only the point");

    /* sequential update, Decode */
    ab_control(b, DC | HEXA | RUN | BANKA, 0);
    for (i = 0; i < 8; i++) {
        ab_write(b, 0, dec[i]);
    }
    for (i = 0; i < 8; i++) {
        expect[i] = hex_lit(dec[i]);
    }
    check(display_is(&c, expect), "sequential decode: hexadecimal digits 5 A F 1 3 4 F 8, points on ID7 low");

    /* every hex code in digit 8 by single-digit update */
    for (v = 0; v < 16; v++) {
        ab_control(b, HEXA | RUN | BANKA, 7);
        ab_write(b, 0, 0x80 | v);
        fill(expect, 0);
        for (i = 0; i < 8; i++) {
            expect[i] = hex_lit(dec[i]);
        }
        expect[7] = HEX[v];
        if (!display_is(&c, expect)) {
            break;
        }
    }
    check(v == 16, "single digit: all 16 hexadecimal characters of Table 5 on digit 8 (7 abc, 6 and 9 with bars), other digits unchanged");

    /* single-digit update: address mapping and mixed Decode / No Decode */
    ab_control(b, DC | HEXA | RUN | BANKA, 0);
    for (i = 0; i < 8; i++) {
        ab_write(b, 0, 0x8E);       /* E */
    }
    fill(expect, HEX[14]);
    check(display_is(&c, expect), "decode: all eight digits show E");
    for (i = 0; i < 8; i++) {
        ab_control(b, HEXA | RUN | BANKA, (unsigned)i);
        ab_write(b, 0, 0x80 | (unsigned)i);
    }
    for (i = 0; i < 8; i++) {
        expect[i] = HEX[i];
    }
    check(display_is(&c, expect), "single digit: ID2-0 = 000 ... 111 address digits 1 ... 8 (Table 4)");
    ab_control(b, HEXA | NODEC | RUN | BANKA, 2);       /* digit 3 No Decode, others stay decoded */
    ab_write(b, 0, 0x7F);
    expect[2] = raw_lit(0x7F);
    check(display_is(&c, expect), "single digit: digit 3 written with No Decode shows raw segments (0x7F: all seven segments, and the point lit by ID7 low) while the others stay decoded");
    ab_control(b, HEXA | RUN | BANKA, 4);               /* digit 5 decoded after */
    ab_write(b, 0, 0x03);
    expect[4] = hex_lit(0x03);
    check(display_is(&c, expect), "single digit: a decoded digit written after it (3 with the point)");
    ab_control(b, HEXA | NODEC | RUN | BANKA | 0, 0);   /* control only (no data): the digits keep their format */
    check(display_is(&c, expect), "control word without a RAM update changes no digit's format or data");
    ab_control(b, HEXA | RUN | BANKA, 0);               /* decoded, address digit 1 */
    ab_write(b, 0, 0x91);
    ab_write(b, 0, 0x95);                               /* a second write goes to the same digit */
    expect[0] = hex_lit(0x95);
    check(display_is(&c, expect), "single digit: a write after a control word with DATA COMING low goes to the addressed digit (and again to the same digit)");

    /* a control word with DATA COMING restarts the sequence even in the middle of one */
    ab_control(b, DC | HEXA | NODEC | RUN | BANKA, 0);
    ab_write(b, 0, 0x7F);
    ab_write(b, 0, 0x7F);
    ab_control(b, DC | HEXA | NODEC | RUN | BANKA, 0);
    for (i = 0; i < 8; i++) {
        ab_write(b, 0, 0x80 | (unsigned)(1u << (i % 7)));
    }
    for (i = 0; i < 8; i++) {
        expect[i] = raw_lit(0x80 | (unsigned)(1u << (i % 7)));
    }
    check(display_is(&c, expect), "sequential: a new control word with DATA COMING restarts at digit 1 (the data of the unfinished one is overwritten)");

    /* shutdown: ID4 low; data can still be entered; the display returns with the RAM */
    ab_control(b, HEXA | NODEC | BANKA, 0);
    check(display_off(&c), "shutdown: a control word with ID4 low leaves every digit pin and segment pin inactive");
    ab_control(b, HEXA | NODEC | BANKA, 3);             /* still shut down, address digit 4 */
    ab_write(b, 0, 0xFF);
    check(display_off(&c), "shutdown: writing RAM in shutdown keeps the display off");
    ab_control(b, HEXA | NODEC | RUN | BANKA, 7);
    expect[3] = raw_lit(0xFF);
    check(display_is(&c, expect), "shutdown: leaving it shows the RAM, including the byte written while it was off");

    /* blank by No Decode and hex 80 on every digit (p.14) */
    ab_control(b, DC | HEXA | NODEC | RUN | BANKA, 0);
    for (i = 0; i < 8; i++) {
        ab_write(b, 0, 0x80);
    }
    check(display_is(&c, none), "No Decode with hex 80 in every digit shows nothing");
    close_bench(b);
}

/* ---- ICM7228C / D -------------------------------------------------------------------------- */

static void cd_chip(const char *part, int cathode) {
    Bench *b = bench(part);
    Chip c = {b, cathode};
    unsigned expect[8];
    char what[160];
    unsigned v;
    int i;

    cd_start(b);
    snprintf(what, sizeof what, "%s: a leddriver part on a parallel bus", part);
    check(b->part->kind == PART_LEDDRIVER, what);
    {
        unsigned seg;

        check(sample(&c, &seg) == 2, "scan: slot 1 (from power-up) selects digit 2");
    }
    for (i = 1; i <= 8; i++) {
        cd_write(b, (unsigned)i, (unsigned)(i - 1), 1);
        expect[i - 1] = HEX[i - 1];
    }
    check(display_is(&c, expect), "digit address: DA2-0 = 000 ... 111 write digits 1 ... 8, each from ID3-ID0 (digits 0-7 shown)");
    scan_order(&c);
    for (v = 0; v < 16; v++) {
        cd_write(b, 8, v, 1);
        expect[7] = HEX[v];
        if (!display_is(&c, expect)) {
            break;
        }
    }
    check(v == 16, "all 16 hexadecimal characters of Table 5 on digit 8, the other digits unchanged");
    cd_write(b, 3, 8, 0);
    expect[2] = HEX[8] | 0x80u;
    check(display_is(&c, expect), "ID7 low lights the decimal point of the addressed digit only");
    cd_write(b, 3, 8, 1);
    expect[2] = HEX[8];
    check(display_is(&c, expect), "ID7 high leaves the point off");
    /* the data is written on the rising edge of WRITE: nothing happens while it is low */
    set(b, "DA2", LOGIC_0);
    set(b, "DA1", LOGIC_0);
    set(b, "DA0", LOGIC_0);
    set(b, "ID0", LOGIC_1);
    set(b, "ID1", LOGIC_1);
    set(b, "ID2", LOGIC_1);
    set(b, "ID3", LOGIC_1);
    set(b, "ID7 (INPUT DP)", LOGIC_1);
    run(b, 300 * NS);
    set(b, "WRITE", LOGIC_0);
    run(b, 500 * US);
    expect[0] = HEX[0];
    check(display_is(&c, expect), "WRITE low alone writes nothing");
    set(b, "WRITE", LOGIC_1);
    run(b, 300 * NS);
    expect[0] = HEX[15];
    check(display_is(&c, expect), "the rising edge of WRITE latches the data (F in digit 1)");
    close_bench(b);
}

/* ---- Pins and parts ------------------------------------------------------------------------- */

static void pinouts(void) {
    static const char *const LETTERS = "ABCD";
    int k;

    for (k = 0; k < 4; k++) {
        char name[32], what[160];
        const Part *p;

        snprintf(name, sizeof name, "ICM7228%cIPI%s", LETTERS[k], k < 3 ? "Z" : "");
        if (k == 3) {
            snprintf(name, sizeof name, "ICM7228DIPI");
        }
        p = part_library_find(library, name);
        if (p == NULL) {
            snprintf(what, sizeof what, "%s: part found", name);
            check(0, what);
            continue;
        }
        snprintf(what, sizeof what, "%s: 28 pins, VDD 19, VSS 28, WRITE 8, %s", name, k < 2 ? "MODE 9" : "pin 9 HEXA/CODE B/SHUTDOWN");
        check(p->pin_count == 28 && strcmp(p->pins[18].name, "VDD") == 0 && strcmp(p->pins[27].name, "VSS") == 0 &&
                  strcmp(p->pins[7].name, "WRITE") == 0 &&
                  strcmp(p->pins[8].name, k < 2 ? "MODE" : "HEXA/CODE B/SHUTDOWN") == 0, what);
        snprintf(what, sizeof what, "%s: pin 1 is %s, pin 15 is %s", name, (k & 1) ? "DIGIT 4" : "SEG c", (k & 1) ? "DP" : "DIGIT 1");
        check(strcmp(p->pins[0].name, (k & 1) ? "DIGIT 4" : "SEG c") == 0 &&
                  strcmp(p->pins[14].name, (k & 1) ? "DP" : "DIGIT 1") == 0, what);
    }
}

int main(void) {
    char *error = NULL;

    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL loading the parts\n");
        return 1;
    }
    free(error);
    pinouts();
    ab_chip("ICM7228AIBIZ", 0);
    ab_chip("ICM7228BIBIZ", 1);
    cd_chip("ICM7228CIBIZ", 0);
    cd_chip("ICM7228DIPI", 1);
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("icm7228_test: all passed\n");
    return 0;
}
