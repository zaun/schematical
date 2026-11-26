/*
 * display_out_cov_test (plan-68-R): display_out.c paths display_out_test and
 * pcf8576_family_test do not reach, on fixtures (tests/fixtures/model_cov):
 *
 *   R68-A16       two 16-segment digits, decode "ascii16": the byte's low six
 *                 bits pick a glyph of the ICM7244/7245 font (display_out.c,
 *                 the font's comment: datasheet Figure 9), the digit pins
 *                 scanned active high
 *   R68-PCF85176  the PCF85176 with no subaddress counter (a single device)
 *                 and its mode field's value 3 (1:3) left out: display bits
 *                 past the last column are dropped (display_out.c,
 *                 disp_lcd_byte), and an unlisted mode value runs the first
 *                 listed mode (1:4)
 *
 * The SPI and I2C masters and the bench are display_out_test's and
 * pcf8576_family_test's (copied).
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

enum { HALF = 100000 };
static const SimTime US = 1000000;

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

/* Every chip pin on its own net with the test driver; weak pull-ups on SDA and SCL when present. */
static Bench *bench(const char *part) {
    Bench *b = calloc(1, sizeof *b);
    size_t i;

    b->part = part_library_find(library, part);
    if (b->part == NULL) {
        fprintf(stderr, "no part %s\n", part);
        exit(2);
    }
    b->pins.count = b->part->pin_count;
    b->pulls.count = 2;
    b->sim = sim_create();
    b->chip = sim_add_instance(b->sim, kind_model(b->part), "U1", b->part);
    b->drv = sim_add_instance(b->sim, &DRIVER, "TB", &b->pins);
    b->pu = sim_add_instance(b->sim, &PULLUP, "PU", &b->pulls);
    for (i = 0; i < b->part->pin_count; i++) {
        SimNet *net = sim_add_net(b->sim, b->part->pins[i].name);

        sim_connect(b->sim, net, b->chip, i);
        sim_connect(b->sim, net, b->drv, i);
        if (strcmp(b->part->pins[i].name, "SDA") == 0) {
            sim_connect(b->sim, net, b->pu, 0);
        } else if (strcmp(b->part->pins[i].name, "SCL") == 0) {
            sim_connect(b->sim, net, b->pu, 1);
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

/* ---- SPI (mode 0, CS active low): 16-bit write frames ---- */

static void spi_write(Bench *b, unsigned reg, unsigned value) {
    unsigned word = (reg << 8) | value, i;

    set(b, "CS", LOGIC_0);
    run(b, HALF);
    for (i = 0; i < 16; i++) {
        set(b, "DIN", (word >> (15 - i)) & 1u ? LOGIC_1 : LOGIC_0);
        run(b, HALF);
        set(b, "SCLK", LOGIC_1);
        run(b, HALF);
        set(b, "SCLK", LOGIC_0);
    }
    run(b, HALF);
    set(b, "CS", LOGIC_1);
    run(b, HALF);
}

/* The 16 segment outputs (bit 0 a1 ... bit 15 m) while DIG<digit> is the scanned one. */
static unsigned a16_digit(Bench *b, int digit) {
    static const char *const SEG[] = {"SA1", "SA2", "SB", "SC", "SD1", "SD2", "SE", "SF",
                                      "SG1", "SG2", "SH", "SI", "SJ", "SK", "SL", "SM"};
    int t;

    for (t = 0; t < 80; t++) {
        if (get(b, digit == 0 ? "DIG1" : "DIG2") == LOGIC_1 && get(b, digit == 0 ? "DIG2" : "DIG1") == LOGIC_0) {
            unsigned seg = 0, i;

            for (i = 0; i < 16; i++) {
                if (get(b, SEG[i]) == LOGIC_1) {
                    seg |= 1u << i;
                }
            }
            return seg;
        }
        run(b, 10 * US);
    }
    return 0xFFFFFFFFu;
}

static void ascii16(void) {
    Bench *b = bench("R68-A16");
    char what[128];
    unsigned d1, d2;

    set(b, "CS", LOGIC_1);
    set(b, "SCLK", LOGIC_0);
    run(b, HALF);
    spi_write(b, 1, 'A');                  /* 41h: code 01h, the font's "A" 003CFh */
    spi_write(b, 2, '0');                  /* 30h: code 30h, the font's "0" 090FFh */
    d1 = a16_digit(b, 0);
    d2 = a16_digit(b, 1);
    snprintf(what, sizeof what, "ascii16: 'A' lights 03CFh (a1 a2 b c f e g1 g2), '0' lights 90FFh (read %04X %04X)",
             d1, d2);
    check(d1 == 0x03CF && d2 == 0x90FF, what);
    spi_write(b, 1, 'a');                  /* 61h: six bits 21h, the font's "!" 10800h (i and the point) */
    spi_write(b, 2, ' ');
    d1 = a16_digit(b, 0);
    d2 = a16_digit(b, 1);
    check(d1 == 0x0800 && d2 == 0x0000,
          "ascii16: 'a' (61h) is code 21h, '!': segment i (no point pin on 16 segments); a space is blank");
    close_bench(b);
}

/* ---- I2C (address 0x38) ---- */

static void line(Bench *b, const char *pin, int level) {
    set(b, pin, level ? LOGIC_Z : LOGIC_0);
}

static void i2c_bit(Bench *b, int out) {
    line(b, "SDA", out);
    run(b, HALF);
    line(b, "SCL", 1);
    run(b, HALF);
    line(b, "SCL", 0);
    run(b, HALF);
}

static void i2c_byte(Bench *b, unsigned v) {
    int i;

    for (i = 7; i >= 0; i--) {
        i2c_bit(b, (v >> i) & 1u);
    }
    i2c_bit(b, 1);
}

static void xfer(Bench *b, const unsigned *bytes, size_t n) {
    size_t i;

    line(b, "SDA", 1);
    line(b, "SCL", 1);
    run(b, HALF);
    line(b, "SDA", 0);
    run(b, HALF);
    line(b, "SCL", 0);
    run(b, HALF);
    i2c_byte(b, 0x38u << 1);
    for (i = 0; i < n; i++) {
        i2c_byte(b, bytes[i]);
    }
    line(b, "SDA", 0);
    run(b, HALF);
    line(b, "SCL", 1);
    run(b, HALF);
    line(b, "SDA", 1);
    run(b, HALF);
}

/* Mode-set (E = 1, mode m), load-data-pointer p, then the data bytes. */
static void load(Bench *b, unsigned m, unsigned p, const unsigned *data, size_t n) {
    unsigned bytes[16];
    size_t i;

    bytes[0] = 0x80u | 0x40u | 0x08u | m;
    bytes[1] = p;
    for (i = 0; i < n; i++) {
        bytes[2 + i] = data[i];
    }
    xfer(b, bytes, 2 + n);
}

/* The display RAM's 20 bytes, from the register window (the RAM registers in address order). */
static int ram(Bench *b, uint8_t out[20]) {
    ConverterRegister regs[64];
    size_t n = kind_model(b->part)->registers(sim_instance_state(b->chip), regs, 64), i, k = 0;

    for (i = 0; i < n && k < 20; i++) {
        if (regs[i].address >= 0 && regs[i].address < 20 && regs[i].width == 8) {
            out[regs[i].address] = (uint8_t)regs[i].value;
            k++;
        }
    }
    return k == 20;
}

static void single_device(void) {
    Bench *b = bench("R68-PCF85176");
    static const unsigned FF[1] = {0xFF};
    uint8_t r[20] = {0};
    char what[128];
    int i, others = 0;

    set(b, "SA0", LOGIC_0);
    run(b, HALF);
    load(b, 0, 39, FF, 1);                 /* 1:4, the last column: 4 bits fit, 4 fall past the end */
    check(ram(b, r), "R68-PCF85176: the 20 RAM bytes read from the register window");
    for (i = 0; i < 19; i++) {
        others |= r[i];
    }
    snprintf(what, sizeof what, "1:4 at column 39 with no subaddress: column 39's 4 bits set, nothing wraps to "
             "column 0 (byte 19 %02X, bytes 0-18 %s)", r[19], others ? "touched" : "0");
    check(r[19] != 0 && (r[19] == 0x0F || r[19] == 0xF0) && others == 0, what);
    close_bench(b);

    b = bench("R68-PCF85176");
    set(b, "SA0", LOGIC_0);
    run(b, HALF);
    load(b, 3, 0, FF, 1);                  /* mode value 3 is not listed: the first mode, 1:4 */
    ram(b, r);
    snprintf(what, sizeof what, "an unlisted mode value runs 1:4: FFh fills columns 0 and 1 (byte 0 %02X, byte 1 %02X)",
             r[0], r[1]);
    check(r[0] == 0xFF && r[1] == 0x00, what);
    close_bench(b);
}

static int load_file(const char *name) {
    char path[512];
    char *text, *error = NULL;
    int rc;

    snprintf(path, sizeof path, "%s/../fixtures/model_cov/%s", GOLDEN_DIR, name);
    text = read_file(path);
    rc = text != NULL ? load_parts(library, text, name, &error) : -1;
    if (rc != 0) {
        printf("FAIL loading %s: %s\n", name, error != NULL ? error : "(no file)");
    }
    free(error);
    free(text);
    return rc;
}

int main(void) {
    setvbuf(stdout, NULL, _IOLBF, 0);   /* the leak check ends with _exit: keep the lines */
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0 || load_file("leddriver.json") != 0 ||
        load_file("lcddriver.json") != 0) {
        return 1;
    }
    ascii16();
    single_device();
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("display_out_cov_test: all passed\n");
    return 0;
}
