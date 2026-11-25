/*
 * max6955_test (plan-56-D): the MAX6955 I2C LED display driver with I/O expander (leddriver.json) through
 * its real I2C bus: the 16 slave addresses of Table 5 (AD1, AD0 tied to GND, V+, SDA or SCL), the command
 * byte and its auto-increment, the registers of Table 7 with their power-up values (Table 16), the digit
 * planes P0 and P1 (0x20-0x2F, 0x40-0x4F, 0x60-0x6F writes both), read-back, and the five GPIO ports
 * (input, open-drain output).
 *
 * Datasheet: datasheets/led-drivers/max6955.pdf. NOT modeled (and so not tested): the O0-O18 digit and
 * segment outputs (the digit-type register's pin multiplexing and the 14- and 16-segment fonts need a
 * new display driver, see the result file's engineNeeds), blink, intensity, key scanning (the Key_A..D
 * registers read 0 and port bits D7:D5 of the port configuration register do not turn P0-P3 into key
 * inputs), the configuration register's R (global clear) and T bits, the P phase bit, the reserved
 * 0x7F pointer lock.
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
    const char *tie_sda[2], *tie_scl[2]; /* address pins wired to SDA / SCL: they follow the master's level */
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
    const char *const *tie = strcmp(pin, "SDA") == 0 ? b->tie_sda : strcmp(pin, "SCL") == 0 ? b->tie_scl : NULL;
    int i;

    set(b, pin, level ? LOGIC_Z : LOGIC_0);
    for (i = 0; tie != NULL && i < 2; i++) {
        if (tie[i] != NULL) {
            set(b, tie[i], level ? LOGIC_1 : LOGIC_0);
        }
    }
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

static unsigned addr7 = 0x60;

static int wr(Bench *b, unsigned reg, const unsigned *v, size_t n) {
    size_t i;
    int ack;

    i2c_start(b);
    ack = i2c_byte(b, addr7 << 1);
    i2c_byte(b, reg);
    for (i = 0; i < n; i++) {
        i2c_byte(b, v[i]);
    }
    i2c_stop(b);
    return ack;
}

static void wr1(Bench *b, unsigned reg, unsigned v) {
    wr(b, reg, &v, 1);
}

/* Command byte then a repeated START and the read (Figure 7 + 9). */
static void rd(Bench *b, unsigned reg, unsigned *v, size_t n) {
    size_t i;

    i2c_start(b);
    i2c_byte(b, addr7 << 1);
    i2c_byte(b, reg);
    i2c_stop(b);
    i2c_start(b);
    i2c_byte(b, (addr7 << 1) | 1u);
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

/* ---- Table 5: the 16 addresses -------------------------------------------------------------- */

typedef struct {
    const char *name;
    int level; /* 0 GND, 1 V+, 2 SDA, 3 SCL */
} Tie;

static void address_table(const char *part) {
    static const char *const pulls[] = {"SDA", "SCL", NULL};
    static const Tie TIES[4] = {{"GND", 0}, {"V+", 1}, {"SDA", 2}, {"SCL", 3}};
    int ok = 1, a1, a0;

    for (a1 = 0; a1 < 4; a1++) {
        for (a0 = 0; a0 < 4; a0++) {
            Bench *b = bench(part, pulls);
            unsigned expect = 0x60u + 4u * (unsigned)a1 + (unsigned)a0;
            Bench *c = b;
            int i;

            for (i = 0; i < 2; i++) {
                const char *pin = i == 0 ? "AD0" : "AD1";
                int lvl = i == 0 ? TIES[a0].level : TIES[a1].level;

                if (lvl == 0) {
                    set(c, pin, LOGIC_0);
                } else if (lvl == 1) {
                    set(c, pin, LOGIC_1);
                } else if (lvl == 2) {
                    c->tie_sda[i] = pin;
                    set(c, pin, LOGIC_1);
                } else {
                    c->tie_scl[i] = pin;
                    set(c, pin, LOGIC_1);
                }
            }
            run(c, 10 * US);
            addr7 = expect;
            ok &= wr(c, 0x01, (const unsigned[]){0x00}, 1) == 1;
            addr7 = expect ^ 1u;
            ok &= wr(c, 0x01, (const unsigned[]){0x00}, 1) == 0;
            close_bench(c);
        }
    }
    addr7 = 0x60;
    check(ok, "Table 5: AD1/AD0 on GND, V+, SDA, SCL give 0x60 + 4 * AD1 + AD0 (GND 0, V+ 1, SDA 2, SCL 3), a neighbour address is not acknowledged");
}

/* ---- The chip bench -------------------------------------------------------------------------- */

static void chip(const char *part) {
    static const char *const pulls[] = {"SDA", "SCL", "P2", "P4/IRQ", NULL};
    Bench *b = bench(part, pulls);
    unsigned v[16];
    int i, ok;

    set(b, "AD0", LOGIC_0);
    set(b, "AD1", LOGIC_0);
    run(b, 10 * US);
    printf("-- %s\n", part);

    /* Table 16 power-up */
    check(rd1(b, 0x01) == 0xFF && rd1(b, 0x02) == 0x00 && rd1(b, 0x03) == 0x07 && rd1(b, 0x04) == 0x00 &&
              rd1(b, 0x05) == 0x14 && rd1(b, 0x06) == 0x1F && rd1(b, 0x07) == 0x00 && rd1(b, 0x0C) == 0x00,
          "power-up (Table 16, p.25): decode 0xFF, intensity 0, scan limit 7, configuration 0 (shutdown), GPIO read 0x14 (the pulled-up P2 and P4 pins; the latch is 0), ports all inputs 0x1F, test 0, digit type 0");
    rd(b, 0x20, v, 16);
    ok = 1;
    for (i = 0; i < 16; i++) {
        ok &= v[i] == (i < 8 ? 0x20u : 0x00u);
    }
    check(ok, "power-up: plane P0 digits 0-7 hold 0x20 (blank) and digits 0a-7a 0x00");
    rd(b, 0x40, v, 16);
    ok = 1;
    for (i = 0; i < 16; i++) {
        ok &= v[i] == (i < 8 ? 0x20u : 0x00u);
    }
    check(ok, "power-up: plane P1 the same");
    rd(b, 0x10, v, 8);
    ok = 1;
    for (i = 0; i < 8; i++) {
        ok &= v[i] == 0;
    }
    check(ok && rd1(b, 0x08) == 0 && rd1(b, 0x09) == 0 && rd1(b, 0x0A) == 0 && rd1(b, 0x0B) == 0 &&
              rd1(b, 0x0D) == 0 && rd1(b, 0x0E) == 0 && rd1(b, 0x0F) == 0,
          "power-up: the intensity registers and the key registers read 0 (no keys pressed, no key scan)");

    /* control registers */
    wr1(b, 0x01, 0x5A);
    wr1(b, 0x02, 0xFF);
    wr1(b, 0x03, 0xFF);
    check(rd1(b, 0x01) == 0x5A && rd1(b, 0x02) == 0x0F && rd1(b, 0x03) == 0x07,
          "decode mode stores a byte; global intensity 4 bits and scan limit 3 bits (unused bits read 0, Table 7 note)");
    wr1(b, 0x04, 0xFF);
    check(rd1(b, 0x04) == 0x4D, "configuration: I, E, B and S are stored; P (read only), R and T (transient) and D1 read 0 (Tables 17-25)");
    wr1(b, 0x04, 0x01);
    check(rd1(b, 0x04) == 0x01, "configuration S = 1: normal operation");
    wr1(b, 0x07, 0xFF);
    check(rd1(b, 0x07) == 0x01, "display test register stores D0 only");
    wr1(b, 0x07, 0x00);
    wr1(b, 0x0C, 0xA5);
    check(rd1(b, 0x0C) == 0xA5, "digit type register (0x0C) stores its byte (CC7..CC0, Table 13)");
    wr1(b, 0x0C, 0x00);
    wr1(b, 0x08, 0xFF);
    wr1(b, 0x09, 0xFF);
    check(rd1(b, 0x08) == 0x00 && rd1(b, 0x09) == 0x00,
          "0x08-0x0B: a write sets the key mask (not observable), a read is the debounced key register: 0");
    wr1(b, 0x0D, 0xFF);
    check(rd1(b, 0x0D) == 0x00, "the key pressed registers are read-only");
    for (i = 0; i < 8; i++) {
        v[i] = 0x11u * (unsigned)i + 1u;
    }
    wr(b, 0x10, v, 8);
    rd(b, 0x10, v, 8);
    check(v[0] == 0x01 && v[1] == 0x12 && v[7] == 0x78, "intensity registers 0x10-0x17 store bytes (auto-increment)");

    /* digit planes */
    for (i = 0; i < 8; i++) {
        v[i] = 0x41u + (unsigned)i;
    }
    wr(b, 0x20, v, 8);
    rd(b, 0x20, v, 10);
    check(v[0] == 0x41 && v[7] == 0x48 && v[8] == 0x00 && v[9] == 0x00,
          "plane P0: eight bytes written from 0x20 land in digits 0-7 (auto-increment), 0a and 1a unchanged");
    check(rd1(b, 0x40) == 0x20 && rd1(b, 0x47) == 0x20, "plane P1 is a separate RAM: still 0x20");
    for (i = 0; i < 8; i++) {
        v[i] = 0x80u | (0x30u + (unsigned)i);
    }
    wr(b, 0x40, v, 8);
    check(rd1(b, 0x43) == 0xB3 && rd1(b, 0x23) == 0x44, "plane P1 write 0x40-0x47 (decimal point bit D7 kept), plane P0 untouched");
    wr(b, 0x28, (const unsigned[]){0x5A, 0xA5}, 2);
    check(rd1(b, 0x28) == 0x5A && rd1(b, 0x29) == 0xA5 && rd1(b, 0x48) == 0x00,
          "digits 0a and 1a (7-segment only) at 0x28 and 0x29 in plane P0");
    wr1(b, 0x6A, 0x3C);
    check(rd1(b, 0x2A) == 0x3C && rd1(b, 0x4A) == 0x3C, "0x6A writes digit 2a in both planes P0 and P1 with the same data (Table 7)");
    wr1(b, 0x63, 0x99);
    check(rd1(b, 0x23) == 0x99 && rd1(b, 0x43) == 0x99, "0x63 writes digit 3 in both planes");
    wr1(b, 0x60, 0x31);
    wr1(b, 0x67, 0x37);
    check(rd1(b, 0x20) == 0x31 && rd1(b, 0x40) == 0x31 && rd1(b, 0x27) == 0x37 && rd1(b, 0x47) == 0x37,
          "0x60 and 0x67 write digits 0 and 7 in both planes");
    wr1(b, 0xA1, 0x77);
    check(rd1(b, 0x21) == 0x77, "bit 7 of the command byte is a don't care (0xA1 writes 0x21)");
    wr1(b, 0x4F, 0x12);
    check(rd1(b, 0x4F) == 0x12 && rd1(b, 0x2F) == 0x00, "digit 7a plane P1 at 0x4F");

    /* GPIO */
    wr1(b, 0x06, 0x1F);
    set(b, "P1", LOGIC_0);
    set(b, "P0", LOGIC_1);
    run(b, 10 * US);
    check((rd1(b, 0x05) & 0x03u) == 0x01, "GPIO read: P0 high, P1 low from the pins (all ports are inputs)");
    set(b, "P1", LOGIC_1);
    run(b, 10 * US);
    check((rd1(b, 0x05) & 0x03u) == 0x03, "GPIO read follows the pins");
    set(b, "P0", LOGIC_Z);
    set(b, "P1", LOGIC_Z);
    wr1(b, 0x05, 0x00);
    wr1(b, 0x06, 0x1B); /* P2 an output, the others inputs */
    run(b, 10 * US);
    check(get(b, "P2") == LOGIC_0, "port P2 as an output with data 0: the pin is driven low");
    wr1(b, 0x05, 0x04);
    run(b, 10 * US);
    check(get(b, "P2") == LOGIC_1 && (rd1(b, 0x05) & 0x04u) == 0x04, "data 1: the open-drain output is released (pulled high), the read shows the pin");
    wr1(b, 0x05, 0x00);
    run(b, 10 * US);
    check(get(b, "P2") == LOGIC_0 && (rd1(b, 0x05) & 0x04u) == 0x00, "back to 0: low, and the read shows the pin level");
    wr1(b, 0x06, 0x17); /* P3 an output with nothing pulling it up */
    wr1(b, 0x05, 0x08);
    run(b, 50 * US);
    check(get(b, "P3") == LOGIC_Z, "open-drain output with data 1 and no pull-up: the pin floats (not driven high or low)");
    wr1(b, 0x05, 0x00);
    run(b, 10 * US);
    check(get(b, "P3") == LOGIC_0, "data 0 drives P3 low");
    wr1(b, 0x05, 0x08);
    run(b, 10 * US);
    check(get(b, "P3") == LOGIC_Z, "and data 1 releases it again (the latch is not changed by the pin level)");
    wr1(b, 0x06, 0x0F); /* P4/IRQ an output */
    wr1(b, 0x05, 0x10);
    run(b, 10 * US);
    check(get(b, "P4/IRQ") == LOGIC_1 && (rd1(b, 0x05) & 0x10u) == 0x10, "P4 as an output: data 1 releases it, the read bit D4 follows");
    wr1(b, 0x05, 0x00);
    run(b, 10 * US);
    check(get(b, "P4/IRQ") == LOGIC_0, "P4 data 0 drives it low");
    close_bench(b);
}

static void pinouts(void) {
    const Part *s = part_library_find(library, "MAX6955AAX+"), *t = part_library_find(library, "MAX6955ATL+");
    /* SSOP: 1 P0, 2 P1, 3 AD0, 4 SDA, 5 SCL, 6 AD1, 7-15 O0-O8, 16 GND, 17 ISET, 18 GND, 19 V+, 20 OSC, 21 V+,
       22-31 O9-O18, 32 BLINK, 33 OSC_OUT, 34 P2, 35 P3, 36 P4/IRQ (p.39) */
    static const char *const SN[36] = {"P0", "P1", "AD0", "SDA", "SCL", "AD1", "O0", "O1", "O2", "O3", "O4", "O5",
                                       "O6", "O7", "O8", "GND", "ISET", "GND", "V+", "OSC", "V+", "O9", "O10",
                                       "O11", "O12", "O13", "O14", "O15", "O16", "O17", "O18", "BLINK", "OSC_OUT",
                                       "P2", "P3", "P4/IRQ"};
    /* TQFN: 1 AD1, 2-10 O0-O8, 11 N.C., 12 GND, 13 GND, 14 ISET, 15 GND, 16 V+, 17 OSC, 18 V+, 19 V+, 20 N.C.,
       21-30 O9-O18, 31 BLINK, 32 OSC_OUT, 33 P3, 34 P2, 35 P4/IRQ, 36 P0, 37 P1, 38 AD0, 39 SDA, 40 SCL */
    static const char *const TN[40] = {"AD1", "O0", "O1", "O2", "O3", "O4", "O5", "O6", "O7", "O8", "NC", "GND",
                                       "GND", "ISET", "GND", "V+", "OSC", "V+", "V+", "NC", "O9", "O10", "O11",
                                       "O12", "O13", "O14", "O15", "O16", "O17", "O18", "BLINK", "OSC_OUT", "P3",
                                       "P2", "P4/IRQ", "P0", "P1", "AD0", "SDA", "SCL"};
    size_t i;
    int ok = s != NULL && t != NULL && s->kind == PART_LEDDRIVER && s->pin_count == 36 && t->pin_count == 40;

    for (i = 0; ok && i < 36; i++) {
        ok = strcmp(s->pins[i].name, SN[i]) == 0;
    }
    for (i = 0; ok && i < 40; i++) {
        ok = strcmp(t->pins[i].name, TN[i]) == 0;
    }
    check(ok, "36-pin SSOP and 40-pin TQFN pin names follow the Pin Description and Pin Configurations (p.6, p.39)");
}

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL loading the parts\n");
        return 1;
    }
    pinouts();
    address_table("MAX6955AAX+");
    chip("MAX6955AAX+");
    chip("MAX6955ATL+");
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("max6955_test: all passed\n");
    return 0;
}
