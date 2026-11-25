/*
 * icm7218_test (plan-56-C): the ICM7218A/B (8-bit bus, MODE and WRITE: a control word then eight
 * sequential digit bytes) and the ICM7218C/D (random access: digit address DA0-DA2, data ID0-ID3
 * and ID7) 8-digit LED drivers, A and C common anode, B and D common cathode. Each bench writes
 * through the real bus pins and samples the scanned DIGIT1-8 / SEGA-SEGG / DP pins.
 *
 * Datasheets: icm7218-renesas.pdf (FN3159), icm7218-icm7228-maxim.pdf.
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

/* ---- The bench: the chip and a test driver ------------------------------------------------------ */

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
    int ca;     /* common anode: digit selected high, segment lit low */
} Bench;

static const SimTime US = 1000000;
static const SimTime NS = 1000;

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

static Bench *bench(const char *part, int ca) {
    Bench *b = calloc(1, sizeof *b);
    size_t i;

    b->part = part_library_find(library, part);
    if (b->part == NULL) {
        fprintf(stderr, "no part %s\n", part);
        exit(2);
    }
    b->ca = ca;
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

/* ---- The bus ------------------------------------------------------------------------------------ */

static const char *const ID[8] = {"ID0", "ID1", "ID2", "ID3", "ID4", "ID5", "ID6", "ID7"};

static void idle(Bench *b) {
    set(b, "WRITE", LOGIC_1);
    run(b, 1 * US);
}

/* WRITE pulse low (500 ns), data held until after the rising edge. */
static void pulse(Bench *b) {
    set(b, "WRITE", LOGIC_0);
    run(b, 500 * NS);
    set(b, "WRITE", LOGIC_1);
    run(b, 500 * NS);
}

/* ICM7218A/B: MODE high = control word on ID4-ID7, MODE low = display data on ID0-ID7. */
static void ab_bits(Bench *b, int mode, unsigned data) {
    int i;

    set(b, "MODE", mode ? LOGIC_1 : LOGIC_0);
    for (i = 0; i < 8; i++) {
        set(b, ID[i], (data >> i) & 1u ? LOGIC_1 : LOGIC_0);
    }
    run(b, 300 * NS);
}

static void ab_write(Bench *b, int mode, unsigned data) {
    ab_bits(b, mode, data);
    pulse(b);
}

/* Control word: ID7 data coming, ID6 hex (1) / Code B (0), ID5 no decode (1) / decode (0), ID4 normal (1) /
   shutdown (0); ID3-ID0 (here `junk`) are don't care. */
static void control(Bench *b, int coming, int hex, int nodecode, int run_, unsigned junk) {
    ab_write(b, 1, (coming ? 0x80u : 0) | (hex ? 0x40u : 0) | (nodecode ? 0x20u : 0) | (run_ ? 0x10u : 0) | (junk & 15u));
}

/* ICM7218C/D: digit address DA2-DA0, data ID3-ID0, ID7 = decimal point input (low lights it). */
static void cd_write(Bench *b, unsigned digit_index, unsigned value, int id7) {
    set(b, "DA0", (digit_index >> 0) & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "DA1", (digit_index >> 1) & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "DA2", (digit_index >> 2) & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "ID0", (value >> 0) & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "ID1", (value >> 1) & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "ID2", (value >> 2) & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "ID3", (value >> 3) & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "ID7", id7 ? LOGIC_1 : LOGIC_0);
    run(b, 300 * NS);
    pulse(b);
}

/* ---- Sampling the scan --------------------------------------------------------------------------- */

static const char *const DIGITS[8] = {"DIGIT1", "DIGIT2", "DIGIT3", "DIGIT4", "DIGIT5", "DIGIT6", "DIGIT7", "DIGIT8"};
static const char *const SEGS[8] = {"SEGA", "SEGB", "SEGC", "SEGD", "SEGE", "SEGF", "SEGG", "DP"};

/* The segment masks of the hex decode (bit 0 = a ... bit 6 = g) for 0-F (Figure 7 of the Maxim
   sheet: 0 1 2 3 4 5 6 7 8 9 A b C d E F). */
static const unsigned HEX[16] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};

/* The selected digit now: 1-8, 0 none, -1 several or unknown; the lit segment pins as bits 0-7. */
static int sample(Bench *b, unsigned *segments) {
    int active = 0, i;
    LogicValue on_digit = b->ca ? LOGIC_1 : LOGIC_0, off_digit = b->ca ? LOGIC_0 : LOGIC_1;
    LogicValue lit = b->ca ? LOGIC_0 : LOGIC_1, unlit = b->ca ? LOGIC_1 : LOGIC_0;

    for (i = 0; i < 8; i++) {
        LogicValue v = get(b, DIGITS[i]);

        if (v == on_digit) {
            active = active == 0 ? i + 1 : -1;
        } else if (v != off_digit) {
            active = -1;
        }
    }
    *segments = 0;
    for (i = 0; i < 8; i++) {
        LogicValue v = get(b, SEGS[i]);

        if (v == lit) {
            *segments |= 1u << i;
        } else if (v != unlit) {
            active = -1;
        }
    }
    return active;
}

/* 8.4 ms (two full frames and a bit), a sample every 100 us a quarter of the way into a 500 us slot:
   every sample has exactly one selected digit whose segments equal expect[digit], and all 8 are seen. */
static int scan_is(Bench *b, const unsigned expect[9]) {
    unsigned seen = 0;
    int ok = 1, t;

    for (t = 0; t < 84; t++) {
        unsigned seg;
        int d;

        run(b, t == 0 ? 25 * US : 100 * US);
        d = sample(b, &seg);
        if (d <= 0 || seg != expect[d]) {
            ok = 0;
        } else {
            seen |= 1u << (d - 1);
        }
    }
    return ok && seen == 0xFFu;
}

/* Every digit pin and segment pin at its inactive level (shutdown), over a few samples. */
static int scan_off(Bench *b) {
    int ok = 1, t;

    for (t = 0; t < 40; t++) {
        unsigned seg;

        ok &= sample(b, &seg) == 0 && seg == 0;
        run(b, 100 * US);
    }
    return ok;
}

/* All digits selected in turn (display on) but no segment lit. */
static int scan_blank(Bench *b) {
    unsigned zero[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

    return scan_is(b, zero);
}

/* The digit pins go active one at a time in the order D1, D7, D8, D6, D4, D3, D2, D5, 500 us each. */
static int slot_order(Bench *b) {
    static const int ORDER[8] = {1, 7, 8, 6, 4, 3, 2, 5};
    int last = 0, index = -1, order_ok = 1, step_ok = 1, steps = 0;
    SimTime changed = 0, t;

    for (t = 0; t < 8500 * US; t += 2 * US) {
        unsigned seg;
        int d = sample(b, &seg);

        if (d != last) {
            if (d <= 0) {
                order_ok = 0;
            } else {
                if (index < 0) {
                    for (index = 0; index < 7 && ORDER[index] != d; index++) {
                    }
                } else {
                    index = (index + 1) % 8;
                }
                if (d != ORDER[index]) {
                    order_ok = 0;
                }
                if (steps >= 2 && (sim_now(b->sim) - changed > 502 * US || sim_now(b->sim) - changed < 498 * US)) {
                    step_ok = 0;
                }
                steps++;
                changed = sim_now(b->sim);
            }
            last = d;
        }
        run(b, 2 * US);
    }
    return order_ok && step_ok && steps >= 16;   /* the first change is the scan already running */
}

/* ---- ICM7218A / ICM7218B ------------------------------------------------------------------------- */

/* The no-decode pattern of byte `v`: ID6 a, ID5 b, ID4 c, ID3 e, ID2 g, ID1 f, ID0 d, ID7 low = point. */
static unsigned raw_of(unsigned v) {
    return (((v >> 6) & 1u) << 0) | (((v >> 5) & 1u) << 1) | (((v >> 4) & 1u) << 2) | ((v & 1u) << 3) |
           (((v >> 3) & 1u) << 4) | (((v >> 1) & 1u) << 5) | (((v >> 2) & 1u) << 6) | ((((v >> 7) & 1u) ? 0u : 1u) << 7);
}

static void write8(Bench *b, const unsigned *bytes) {
    int i;

    for (i = 0; i < 8; i++) {
        ab_write(b, 0, bytes[i]);
    }
}

static void ab_device(const char *part, int ca, const char *tag) {
    Bench *b = bench(part, ca);
    char text[200];
    unsigned expect[9], bytes[8], zero[9] = {0};
    int i;

    idle(b);
    snprintf(text, sizeof text, "%s: a leddriver part with 28 pins", tag);
    check(b->part->kind == PART_LEDDRIVER && b->part->pin_count == 28, text);
    snprintf(text, sizeof text, "%s: after power-up the display is off (no control word yet): every digit and segment pin inactive", tag);
    check(scan_off(b), text);

    /* control word: data coming, hex, decode, running; ID3-ID0 are don't care */
    control(b, 1, 1, 0, 1, 0x05);
    snprintf(text, sizeof text, "%s: a control word with DATA COMING blanks the display (digits scan, no segment lit)", tag);
    check(scan_blank(b), text);
    snprintf(text, sizeof text, "%s: digit scan order D1 D7 D8 D6 D4 D3 D2 D5, 500 us per digit", tag);
    check(slot_order(b), text);

    /* eight bytes: hex 0..7, the point lit on digits 2 and 8 (ID7 low) */
    for (i = 0; i < 8; i++) {
        bytes[i] = (unsigned)i | ((i == 1 || i == 7) ? 0x00u : 0x80u) | 0x70u;   /* ID6-ID4 don't care when decoding */
    }
    for (i = 0; i < 7; i++) {
        ab_write(b, 0, bytes[i]);
    }
    snprintf(text, sizeof text, "%s: after seven data writes the display is still blank", tag);
    check(scan_blank(b), text);
    ab_write(b, 0, bytes[7]);
    for (i = 0; i < 8; i++) {
        expect[i + 1] = HEX[i] | ((i == 1 || i == 7) ? 0x80u : 0);
    }
    snprintf(text, sizeof text, "%s: the eighth write shows digits 1-8 = 0..7, point (ID7 low) on digits 2 and 8", tag);
    check(scan_is(b, expect), text);

    /* a ninth WRITE pulse is ignored */
    ab_write(b, 0, 0x0F);
    snprintf(text, sizeof text, "%s: WRITE pulses after the eighth are ignored until a new control word", tag);
    check(scan_is(b, expect), text);

    /* WRITE low with data on the bus latches nothing until it rises */
    control(b, 1, 1, 0, 1, 0);
    for (i = 0; i < 7; i++) {
        ab_write(b, 0, 0x80u | 0x0Au);
    }
    ab_bits(b, 0, 0x80u | 0x0Bu);
    set(b, "WRITE", LOGIC_0);
    run(b, 600 * NS);
    set(b, "WRITE", LOGIC_1);
    run(b, 500 * NS);
    for (i = 0; i < 7; i++) {
        expect[i + 1] = HEX[10];
    }
    expect[8] = HEX[11];
    snprintf(text, sizeof text, "%s: the byte on the bus when WRITE rises is the one latched; each pulse fills the next digit (A x7, b)", tag);
    check(scan_is(b, expect), text);

    /* all 16 hex codes, digits 1-8 then 9-16 in two passes */
    for (i = 0; i < 8; i++) {
        bytes[i] = 0x80u | (unsigned)i;
    }
    control(b, 1, 1, 0, 1, 0);
    write8(b, bytes);
    for (i = 0; i < 8; i++) {
        expect[i + 1] = HEX[i];
    }
    check(scan_is(b, expect), "  hex 0-7");
    for (i = 0; i < 8; i++) {
        bytes[i] = 0x80u | (unsigned)(8 + i);
    }
    control(b, 1, 1, 0, 1, 0);
    write8(b, bytes);
    for (i = 0; i < 8; i++) {
        expect[i + 1] = HEX[8 + i];
    }
    snprintf(text, sizeof text, "%s: hex decode of 0-F: 0 1 2 3 4 5 6 7 | 8 9 A b C d E F", tag);
    check(scan_is(b, expect), text);

    /* control word without data (DATA COMING low) changes nothing displayed; later data writes are ignored */
    control(b, 0, 1, 0, 1, 0);
    ab_write(b, 0, 0x80u | 0x01u);
    check(scan_is(b, expect), "  control word with DATA COMING low keeps the display; MODE low writes are then ignored");

    /* no decode: bits map straight to segments, ID7 low lights the point */
    bytes[0] = 0xC0; bytes[1] = 0xA0; bytes[2] = 0x90; bytes[3] = 0x81;
    bytes[4] = 0x88; bytes[5] = 0x82; bytes[6] = 0x84; bytes[7] = 0x00;
    control(b, 1, 1, 1, 1, 0x0A);
    write8(b, bytes);
    for (i = 0; i < 8; i++) {
        expect[i + 1] = raw_of(bytes[i]);
    }
    snprintf(text, sizeof text, "%s: no decode: ID6 a, ID5 b, ID4 c, ID0 d, ID3 e, ID1 f, ID2 g, ID7 low = point", tag);
    check(scan_is(b, expect), text);
    bytes[0] = 0xFF; bytes[1] = 0x7F; bytes[2] = 0x80; bytes[3] = 0x00;
    bytes[4] = 0xFF; bytes[5] = 0x7F; bytes[6] = 0x80; bytes[7] = 0x00;
    control(b, 1, 1, 1, 1, 0);
    write8(b, bytes);
    for (i = 0; i < 8; i++) {
        expect[i + 1] = raw_of(bytes[i]);
    }
    check(scan_is(b, expect), "  no decode: 0xFF = seven segments, point off; 0x7F = all eight; 0x80 = nothing; 0x00 = point only");

    /* the stored bytes are shown decoded or raw by the control word alone (one RAM) */
    control(b, 0, 1, 0, 1, 0);
    for (i = 0; i < 8; i++) {
        expect[i + 1] = HEX[bytes[i] & 15u] | ((bytes[i] & 0x80u) ? 0 : 0x80u);
    }
    snprintf(text, sizeof text, "%s: switching to decode with DATA COMING low re-reads the same bytes as hex codes", tag);
    check(scan_is(b, expect), text);

    /* shutdown: ID4 low; data can still be loaded; the outputs are inactive */
    control(b, 1, 1, 0, 0, 0);
    bytes[0] = 1; bytes[1] = 2; bytes[2] = 3; bytes[3] = 4; bytes[4] = 5; bytes[5] = 6; bytes[6] = 7; bytes[7] = 8;
    for (i = 0; i < 8; i++) {
        bytes[i] |= 0x80u;
    }
    write8(b, bytes);
    snprintf(text, sizeof text, "%s: shutdown (ID4 low) turns every digit and segment output off, even after a full data load", tag);
    check(scan_off(b), text);
    control(b, 0, 1, 0, 1, 0);
    for (i = 0; i < 8; i++) {
        expect[i + 1] = HEX[i + 1];
    }
    snprintf(text, sizeof text, "%s: data loaded during shutdown shows when shutdown ends", tag);
    check(scan_is(b, expect), text);

    /* a new control word with DATA COMING restarts at digit 1 */
    control(b, 1, 1, 0, 1, 0);
    ab_write(b, 0, 0x80u | 0x0Eu);
    ab_write(b, 0, 0x80u | 0x0Eu);
    ab_write(b, 0, 0x80u | 0x0Eu);
    control(b, 1, 1, 0, 1, 0);
    for (i = 0; i < 8; i++) {
        bytes[i] = 0x80u | 0x04u;
    }
    bytes[0] = 0x80u | 0x09u;
    write8(b, bytes);
    for (i = 0; i < 8; i++) {
        expect[i + 1] = HEX[4];
    }
    expect[1] = HEX[9];
    snprintf(text, sizeof text, "%s: a new control word with DATA COMING restarts the sequence at digit 1", tag);
    check(scan_is(b, expect), text);
    (void)zero;
    close_bench(b);
}

/* ---- ICM7218C / ICM7218D ------------------------------------------------------------------------- */

static void cd_device(const char *part, int ca, const char *tag) {
    Bench *b = bench(part, ca);
    char text[200];
    unsigned expect[9], zero[9] = {0, HEX[0], HEX[0], HEX[0], HEX[0], HEX[0], HEX[0], HEX[0], HEX[0]};
    int i, d;

    idle(b);
    snprintf(text, sizeof text, "%s: a leddriver part with 28 pins", tag);
    check(b->part->kind == PART_LEDDRIVER && b->part->pin_count == 28, text);
    snprintf(text, sizeof text, "%s: power-up RAM is all zero: eight 0s, no point", tag);
    check(scan_is(b, zero), text);
    snprintf(text, sizeof text, "%s: digit scan order D1 D7 D8 D6 D4 D3 D2 D5, 500 us per digit", tag);
    check(slot_order(b), text);

    /* random access: only the addressed digit changes; DA = digit - 1 */
    for (i = 0; i < 9; i++) {
        expect[i] = zero[i];
    }
    for (d = 0; d < 8; d++) {
        cd_write(b, (unsigned)d, (unsigned)(d + 1), 1);
        expect[d + 1] = HEX[d + 1];
        if (d == 6 || d == 7) {
            check(scan_is(b, expect), d == 6 ? "  digit 7 written alone: the others are untouched"
                                             : "  digits 1-8 hold 1-8 after eight random-access writes");
        }
    }
    /* the point: ID7 low lights it */
    cd_write(b, 2, 3, 0);
    expect[3] = HEX[3] | 0x80u;
    snprintf(text, sizeof text, "%s: ID7 low lights the decimal point of the addressed digit only", tag);
    check(scan_is(b, expect), text);
    cd_write(b, 2, 3, 1);
    expect[3] = HEX[3];
    check(scan_is(b, expect), "  ID7 high turns it off again");

    /* all hex codes through digit 4 */
    for (i = 0; i < 16; i++) {
        cd_write(b, 3, (unsigned)i, 1);
        expect[4] = HEX[i];
        if (!scan_is(b, expect)) {
            break;
        }
    }
    snprintf(text, sizeof text, "%s: hex decode of 0-F: 0 1 2 3 4 5 6 7 8 9 A b C d E F", tag);
    check(i == 16, text);

    /* WRITE low alone latches nothing */
    set(b, "DA0", LOGIC_0);
    set(b, "DA1", LOGIC_0);
    set(b, "DA2", LOGIC_0);
    set(b, "ID0", LOGIC_0);
    set(b, "ID1", LOGIC_0);
    set(b, "ID2", LOGIC_1);
    set(b, "ID3", LOGIC_0);
    set(b, "ID7", LOGIC_1);
    set(b, "WRITE", LOGIC_0);
    run(b, 800 * NS);
    set(b, "WRITE", LOGIC_1);
    run(b, 500 * NS);
    expect[1] = HEX[4];
    snprintf(text, sizeof text, "%s: the data on the bus when WRITE rises is stored (digit 1 = 4)", tag);
    check(scan_is(b, expect), text);
    close_bench(b);
}

/* ---- The parts ------------------------------------------------------------------------------------ */

static void parts(void) {
    static const char *const NAMES[] = {
        "ICM7218AIPI+", "ICM7218AIQI+", "ICM7218AIQI+T",
        "ICM7218BIJI", "ICM7218BIPI+", "ICM7218BIQI+",
        "ICM7218CIJI", "ICM7218CIPI+", "ICM7218CIQI+",
        "ICM7218DIJI", "ICM7218DIPI+", "ICM7218DIQI+"
    };
    size_t i;
    int ok = 1;

    for (i = 0; i < sizeof NAMES / sizeof NAMES[0]; i++) {
        const Part *p = part_library_find(library, NAMES[i]);

        ok &= p != NULL && p->kind == PART_LEDDRIVER && p->pin_count == 28;
    }
    check(ok, "all 12 ICM7218 part names are leddriver parts with 28 pins");
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL loading the parts\n");
        return 1;
    }
    parts();
    ab_device("ICM7218AIPI+", 1, "ICM7218A (common anode)");
    ab_device("ICM7218BIJI", 0, "ICM7218B (common cathode)");
    cd_device("ICM7218CIJI", 1, "ICM7218C (common anode)");
    cd_device("ICM7218DIJI", 0, "ICM7218D (common cathode)");
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("icm7218_test: all passed\n");
    return 0;
}
