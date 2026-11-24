/*
 * as1115_test (plan-56-D): the AS1115 8-digit LED driver (leddriver.json) through its real
 * I2C bus: slave address 0000000, command byte + data bytes with auto-increment, the
 * decode-enable register (Code-B decode per digit, no-decode raw segment bits), shutdown,
 * display test, register read-back, and the scanned digit / segment pins in both packages.
 *
 * Datasheet: datasheets/led-drivers/as1115.pdf (page numbers are PDF pages). Not modeled (and so not
 * tested): HEX decode (the model decodes Code-B only), the scan-limit register (all 8 digits always
 * scan), intensity, blink, key scan (KEYA / KEYB read 0xFF), open/short LED detection, the feature
 * register's reset-on-shutdown rule and reg_res bit, I2C self addressing.
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

/* ---- The bench: the chip, a test driver on every pin and weak pull-ups on the bus ---------- */

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

static void pull_start(Sim *sim, SimInstance *instance, void *state) {
    size_t i;

    for (i = 0; i < ((PinCount *)state)->count; i++) {
        sim_schedule_drive(sim, instance, i, LOGIC_1, DRIVE_WEAK, 0);
    }
}

static const SimComponentType DRIVER = {"tb", drv_pins, drv_create, free, NULL, NULL, NULL,
                                        NULL, NULL, NULL, NULL, NULL};
static const SimComponentType PULLUP = {"pu", drv_pins, drv_create, free, pull_start, NULL, NULL,
                                        NULL, NULL, NULL, NULL, NULL};

typedef struct {
    Sim *sim;
    SimInstance *chip, *drv, *pu;
    const Part *part;
    PinCount pins, pulls;
} Bench;

static const SimTime US = 1000000;
enum { HALF = 100000 };

static size_t bpin(const Bench *b, const char *name) {
    size_t i;

    for (i = 0; i < b->part->pin_count; i++) {
        if (strcmp(b->part->pins[i].name, name) == 0) {
            return i;
        }
    }
    fprintf(stderr, "no pin %s\n", name);
    exit(2);
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

/* `pulls`: NULL-terminated pin names that get a weak pull-up (the bus lines, open-drain outputs). */
static Bench *bench(const char *part, const char *const *pulls) {
    Bench *b = calloc(1, sizeof *b);
    size_t i, k;

    b->part = part_library_find(library, part);
    if (b->part == NULL) {
        fprintf(stderr, "no part %s\n", part);
        exit(2);
    }
    b->pins.count = b->part->pin_count;
    for (k = 0; pulls != NULL && pulls[k] != NULL; k++) {
        b->pulls.count++;
    }
    b->sim = sim_create();
    b->chip = sim_add_instance(b->sim, kind_model(b->part), "U1", b->part);
    b->drv = sim_add_instance(b->sim, &DRIVER, "TB", &b->pins);
    b->pu = sim_add_instance(b->sim, &PULLUP, "PU", &b->pulls);
    for (i = 0; i < b->part->pin_count; i++) {
        SimNet *net = sim_add_net(b->sim, b->part->pins[i].name);

        sim_connect(b->sim, net, b->chip, i);
        sim_connect(b->sim, net, b->drv, i);
        for (k = 0; pulls != NULL && pulls[k] != NULL; k++) {
            if (strcmp(pulls[k], b->part->pins[i].name) == 0) {
                sim_connect(b->sim, net, b->pu, k);
            }
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

/* ---- I2C ------------------------------------------------------------------------------------ */

static void line(Bench *b, const char *pin, int level) {
    set(b, pin, level ? LOGIC_Z : LOGIC_0);
}

static LogicValue i2c_bit(Bench *b, int out) {
    LogicValue v;

    line(b, "SDA", out);
    run(b, HALF);
    line(b, "SCL", 1);
    run(b, HALF);
    v = get(b, "SDA");
    line(b, "SCL", 0);
    run(b, HALF);
    return v;
}

static void i2c_start(Bench *b) {
    line(b, "SDA", 1);
    line(b, "SCL", 1);
    run(b, HALF);
    line(b, "SDA", 0);
    run(b, HALF);
    line(b, "SCL", 0);
    run(b, HALF);
}

static void i2c_stop(Bench *b) {
    line(b, "SDA", 0);
    run(b, HALF);
    line(b, "SCL", 1);
    run(b, HALF);
    line(b, "SDA", 1);
    run(b, HALF);
}

/* Sends a byte; 1 when the chip acknowledged. */
static int i2c_byte(Bench *b, unsigned v) {
    int i;

    for (i = 7; i >= 0; i--) {
        i2c_bit(b, (v >> i) & 1u);
    }
    return i2c_bit(b, 1) == LOGIC_0;
}

static unsigned i2c_in(Bench *b, int ack) {
    unsigned v = 0;
    int i;

    for (i = 0; i < 8; i++) {
        v = (v << 1) | (i2c_bit(b, 1) == LOGIC_1 ? 1u : 0u);
    }
    i2c_bit(b, !ack);
    return v;
}

/* ---- The chip -------------------------------------------------------------------------------- */

enum { ADDR = 0x00 };

static void wr(Bench *b, unsigned reg, const unsigned *v, size_t n) {
    size_t i;

    i2c_start(b);
    i2c_byte(b, ADDR << 1);
    i2c_byte(b, reg);
    for (i = 0; i < n; i++) {
        i2c_byte(b, v[i]);
    }
    i2c_stop(b);
}

static void wr1(Bench *b, unsigned reg, unsigned v) {
    wr(b, reg, &v, 1);
}

static void rd(Bench *b, unsigned reg, unsigned *v, size_t n) {
    size_t i;

    i2c_start(b);
    i2c_byte(b, ADDR << 1);
    i2c_byte(b, reg);
    i2c_stop(b);
    i2c_start(b);
    i2c_byte(b, (ADDR << 1) | 1u);
    for (i = 0; i < n; i++) {
        v[i] = i2c_in(b, i + 1 < n);
    }
    i2c_stop(b);
}

static unsigned rd1(Bench *b, unsigned reg) {
    unsigned v = 0;

    rd(b, reg, &v, 1);
    return v;
}

/* ---- The scan -------------------------------------------------------------------------------- */

static const char *const DIGS[8] = {"DIG0", "DIG1", "DIG2", "DIG3", "DIG4", "DIG5", "DIG6", "DIG7"};
static const char *const SEGS[8] = {"SEGA", "SEGB", "SEGC", "SEGD", "SEGE", "SEGF", "SEGG", "SEGDP"};

/* The selected digit (0-7; -1 none, -2 several) and the lit segments as bits 0-7 (A-G, DP). */
static int sample(Bench *b, unsigned *segments) {
    int active = -1, i;

    for (i = 0; i < 8; i++) {
        if (get(b, DIGS[i]) == LOGIC_0) {
            active = active == -1 ? i : -2;
        }
    }
    *segments = 0;
    for (i = 0; i < 8; i++) {
        if (get(b, SEGS[i]) == LOGIC_1) {
            *segments |= 1u << i;
        }
    }
    return active;
}

/* A little over two frames (8 slots of 179 us), sampled every 20 us: exactly one digit selected in
   every sample, its lit segments equal expect[digit], and every digit seen. */
static int display_is(Bench *b, const unsigned *expect) {
    unsigned seen = 0;
    int ok = 1, t;

    for (t = 0; t < 150; t++) {
        unsigned seg;
        int d = sample(b, &seg);

        if (d < 0 || seg != expect[d]) {
            ok = 0;
        } else {
            seen |= 1u << d;
        }
        run(b, 20 * US);
    }
    return ok && seen == 0xFFu;
}

/* Every digit pin deselected (high) and no segment lit. */
static int display_off(Bench *b) {
    int ok = 1, t;

    for (t = 0; t < 60; t++) {
        unsigned seg;

        ok &= sample(b, &seg) == -1 && seg == 0;
        run(b, 20 * US);
    }
    return ok;
}

/* Segment letters to the pin mask. */
static unsigned lit(const char *letters) {
    unsigned m = 0;

    for (; *letters != '\0'; letters++) {
        m |= *letters == 'P' ? 0x80u : 1u << (*letters - 'a');
    }
    return m;
}

/* The no-decode byte (D7 DP, D6 A ... D0 G) as a pin mask. */
static unsigned raw(unsigned v) {
    unsigned m = 0;
    int i;

    for (i = 0; i < 7; i++) {
        if (v & (1u << (6 - i))) {
            m |= 1u << i;
        }
    }
    return m | (v & 0x80u);
}

/* Table 9 Code-B font, characters 0-15. */
static const char *const CODEB[16] = {"abcdef", "bc", "abdeg", "abcdg", "bcfg", "acdfg", "acdefg", "abc",
                                      "abcdefg", "abcdfg", "g", "adefg", "bcefg", "def", "abefg", ""};

static void chip(const char *part) {
    static const char *const pulls[] = {"SDA", "SCL", "IRQ", NULL};
    Bench *b = bench(part, pulls);
    unsigned v[16], e[8];
    int i;

    run(b, 10 * US);
    printf("-- %s\n", part);
    check(display_off(b), "power-up: shutdown, the display is blank (digit pins high, segments low)");
    check(rd1(b, 0x0C) == 0x00 && rd1(b, 0x09) == 0x00 && rd1(b, 0x0A) == 0x00 && rd1(b, 0x0B) == 0x00 &&
              rd1(b, 0x0E) == 0x00 && rd1(b, 0x0F) == 0x00,
          "power-up register values: shutdown, decode, intensity, scan limit, feature, test all 0 (p.14, p.18)");
    rd(b, 0x01, v, 8);
    check(v[0] == 0 && v[3] == 0 && v[7] == 0, "power-up: the eight digit registers are 0 (blank)");
    check(rd1(b, 0x1C) == 0xFF && rd1(b, 0x1D) == 0xFF,
          "no key shorted: KEYA and KEYB read 0xFF (open = 1, p.17)");

    /* address: 0000000 only */
    i2c_start(b);
    check(i2c_byte(b, 0x01 << 1) == 0, "address 0000001 is not acknowledged (the factory address is 0000000, p.12)");
    i2c_stop(b);
    i2c_start(b);
    check(i2c_byte(b, ADDR << 1) == 1, "address 0000000 is acknowledged");
    i2c_stop(b);

    /* Code-B decode of all 16 codes on 8 digits at a time */
    wr1(b, 0x09, 0xFF);
    wr1(b, 0x0C, 0x01);
    check(rd1(b, 0x09) == 0xFF && rd1(b, 0x0C) == 0x01, "decode and shutdown registers read back what was written");
    for (i = 0; i < 8; i++) {
        v[i] = (unsigned)i;
    }
    wr(b, 0x01, v, 8);
    for (i = 0; i < 8; i++) {
        e[i] = lit(CODEB[i]);
    }
    check(display_is(b, e), "normal operation: digits 0-7 show Code-B 0-7 (command 0x01..0x08 auto-incrementing writes)");
    for (i = 0; i < 8; i++) {
        v[i] = (unsigned)i + 8;
        e[i] = lit(CODEB[i + 8]);
    }
    wr(b, 0x01, v, 8);
    check(display_is(b, e), "Code-B 8, 9, '-', E, H, L, P, blank (Table 9, p.16)");
    for (i = 0; i < 8; i++) {
        v[i] = 0x80u | (0x70u & (unsigned)(i << 4)) | (unsigned)i;
        e[i] = lit(CODEB[i]) | 0x80u;
    }
    wr(b, 0x01, v, 8);
    check(display_is(b, e), "decoded: D7 lights the decimal point, D6:D4 are ignored (p.13)");

    /* per-digit decode enable and no-decode raw bytes */
    wr1(b, 0x09, 0x25);
    for (i = 0; i < 8; i++) {
        v[i] = 0xA0u | (unsigned)i;
    }
    wr(b, 0x01, v, 8);
    for (i = 0; i < 8; i++) {
        e[i] = (0x25u >> i) & 1u ? lit(CODEB[i]) | 0x80u : raw(0xA0u | (unsigned)i);
    }
    check(display_is(b, e), "decode enable 0x25: digits 0, 2, 5 decoded, the others raw (Table 8, p.13)");
    wr1(b, 0x09, 0x00);
    for (i = 0; i < 8; i++) {
        v[i] = 1u << i;
        e[i] = raw(1u << i);
    }
    wr(b, 0x01, v, 8);
    check(display_is(b, e), "no decode: D6 A, D5 B, D4 C, D3 D, D2 E, D1 F, D0 G, D7 DP (Table 11, p.15)");
    check(raw(0x40) == lit("a") && raw(0x01) == lit("g") && raw(0x80) == lit("P"), "(the raw map used above)");
    wr1(b, 0x04, 0xFF);
    e[3] = 0xFF;
    for (i = 0; i < 8; i++) {
        e[i] = i == 3 ? 0xFFu : raw(1u << i);
    }
    check(display_is(b, e), "digit 3 written alone with 0xFF (command 0x04): all eight segments of digit 3, the others unchanged");
    check(rd1(b, 0x04) == 0xFF && rd1(b, 0x03) == 0x04 && rd1(b, 0x05) == 0x10, "digit registers read back");

    /* shutdown and display test */
    wr1(b, 0x0C, 0x00);
    check(display_off(b), "shutdown (0x0C = 0): blank, digit registers keep their data");
    check(rd1(b, 0x04) == 0xFF, "data retained in shutdown (p.14)");
    wr1(b, 0x0F, 0x01);
    for (i = 0; i < 8; i++) {
        e[i] = 0xFF;
    }
    check(display_is(b, e), "display test: every segment of every digit, also in shutdown (p.16)");
    wr1(b, 0x0F, 0x00);
    check(display_off(b), "display test off: back to the shutdown blank");
    wr1(b, 0x0C, 0x81);
    for (i = 0; i < 8; i++) {
        e[i] = i == 3 ? 0xFFu : raw(1u << i);
    }
    check(display_is(b, e), "shutdown register 0x81: normal operation, feature register unchanged (Table 7, p.13)");

    /* registers */
    wr1(b, 0x0A, 0xFF);
    wr1(b, 0x0B, 0xFF);
    check(rd1(b, 0x0A) == 0x0F && rd1(b, 0x0B) == 0x07, "intensity keeps 4 bits, scan limit 3 bits (the unused bits read 0)");
    wr1(b, 0x0E, 0xFF);
    check(rd1(b, 0x0E) == 0xF7, "feature register: D3 is not used and reads 0, the other bits are stored (Table 20)");
    wr1(b, 0x0E, 0x00);
    wr1(b, 0x10, 0xA5);
    wr1(b, 0x13, 0x5A);
    check(rd1(b, 0x10) == 0xA5 && rd1(b, 0x13) == 0x5A, "per-digit intensity registers 0x10-0x13 store a byte");
    wr1(b, 0x1C, 0x00);
    wr1(b, 0x14, 0xFF);
    check(rd1(b, 0x1C) == 0xFF && rd1(b, 0x14) == 0x00, "key-scan and diagnostic registers are read-only");
    wr1(b, 0x0F, 0xFF);
    check(rd1(b, 0x0F) == 0x07, "test register: only D2:D0 are writable");
    wr1(b, 0x0F, 0x00);
    wr1(b, 0x2D, 0x01);
    check(rd1(b, 0x2D) == 0x01, "self-addressing register (0x2D) stores D0");
    rd(b, 0x08, v, 4);
    check(v[0] == 0x80 && v[1] == 0x00 && v[2] == 0x0F && v[3] == 0x07,
          "read auto-increment across 0x08, 0x09 (decode), 0x0A, 0x0B");
    close_bench(b);
}

static void pinouts(void) {
    const Part *q = part_library_find(library, "AS1115-BSST");
    size_t i;
    int ok = q != NULL && q->kind == PART_LEDDRIVER && q->pin_count == 24;

    /* QSOP-24: 1 SDA, 2-5 DIG0-3, 6 GND, 7-10 DIG4-7, 11 KEYA, 12 KEYB, 13 ISET, 14 SCL, 15 SEGA,
       16 SEGF, 17 SEGB, 18 SEGG, 19 VDD, 20 SEGC, 21 SEGE, 22 SEGDP, 23 SEGD, 24 IRQ (Figure 2, p.4) */
    static const char *const QN[24] = {"SDA", "DIG0", "DIG1", "DIG2", "DIG3", "GND", "DIG4", "DIG5", "DIG6",
                                       "DIG7", "KEYA", "KEYB", "ISET", "SCL", "SEGA", "SEGF", "SEGB", "SEGG",
                                       "VDD", "SEGC", "SEGE", "SEGDP", "SEGD", "IRQ"};
    for (i = 0; ok && i < 24; i++) {
        ok = strcmp(q->pins[i].name, QN[i]) == 0;
    }
    check(ok, "QSOP-24 pin names follow Figure 2 (p.4)");
}

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL loading the parts\n");
        return 1;
    }
    pinouts();
    chip("AS1115-BSST");
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("as1115_test: all passed\n");
    return 0;
}
