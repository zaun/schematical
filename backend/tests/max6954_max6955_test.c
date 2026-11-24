/*
 * max6954_max6955_test (plan-56-D): the MAX6954 (4-wire SPI) and MAX6955 (I2C) LED display drivers with I/O
 * expander (leddriver.json). The two chips have the same registers (Table 6 / Table 7), 32 digit registers in
 * two planes P0 and P1, five GPIO ports and the same pin-multiplexed O0-O18 outputs; they differ in the bus:
 * the MAX6954's 16-bit SPI frame (R/W, 7-bit address, data byte; a read is answered in the next frame) and the
 * MAX6955's 16 I2C addresses (Table 5) with the command byte and its auto-increment.
 *
 * Every check goes through the real bus pins. The MAX6955's address straps, power-up values (Table 16), command
 * byte, auto-increment, planes, both-planes writes (0x60-0x6F), read-back and GPIO ports are checked on all
 * three of its parts; the MAX6954's frame, DOUT echo and read-back, the global clear (R), the key mask
 * registers and the GPIO ports on both of its parts. The pin names of every package and the footprint pads are
 * checked against the pin configurations (max6954.pdf p.37, max6955.pdf p.39) and the package drawings.
 *
 * Datasheets: datasheets/led-drivers/max6954.pdf (Maxim 19-2460 Rev 5), max6955.pdf (19-2548 Rev 4).
 *
 * NOT modeled (and so not tested): the O0-O18 digit and segment outputs (one display driver per device and the
 * digits driver has nine shared pins and one pin map; the O pins stay released), the fonts, blink and its BLINK
 * and OSC_OUT pins, intensity, key scanning (the Key_A..D registers read 0 and the port bits D7:D5 do not turn
 * P0-P3 into key inputs or P4 into the IRQ output), the configuration register's T bit and the P phase bit; for
 * the MAX6955 also R (global clear), which an I2C data byte cannot reach in this engine, and the key masks.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "parts.h"
#include "read_file.h"
#include "sim.h"
#include "converter_state.h"

static int failures;
static PartLibrary *library;

static void check(int ok, const char *what) {
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

/* ---- The bench: the chip, a test driver on every pin and weak pull-ups on the open-drain lines --------- */

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

static const Part *find(const char *name) {
    const Part *p = part_library_find(library, name);

    if (p == NULL) {
        fprintf(stderr, "no part %s\n", name);
        exit(2);
    }
    return p;
}

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

    b->part = find(part);
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

/* A register of the chip by name (the register window's view). */
static unsigned reg(Bench *b, const char *name) {
    const SimComponentType *type = sim_instance_type(b->chip);
    ConverterRegister regs[160];
    size_t n = type->registers(sim_instance_state(b->chip), regs, 160), i;

    for (i = 0; i < n && i < 160; i++) {
        if (strcmp(regs[i].name, name) == 0) {
            return regs[i].value;
        }
    }
    fprintf(stderr, "no register %s\n", name);
    exit(2);
}

/* ---- Pin lists and footprints ----------------------------------------------------------------------- */

/* SSOP: 1 P0, 2 P1, 3-6 the bus, 7-15 O0-O8, 16 GND, 17 ISET, 18 GND, 19 V+, 20 OSC, 21 V+, 22-31 O9-O18,
   32 BLINK, 33 OSC_OUT, 34 P2, 35 P3, 36 P4/IRQ (max6954.pdf p.37, max6955.pdf p.39) */
static const char *const SSOP_TAIL[] = {"O0", "O1", "O2", "O3", "O4", "O5", "O6", "O7", "O8", "GND", "ISET", "GND",
                                        "V+", "OSC", "V+", "O9", "O10", "O11", "O12", "O13", "O14", "O15", "O16",
                                        "O17", "O18", "BLINK", "OSC_OUT", "P2", "P3", "P4/IRQ"};
/* TQFN: 1 bus pin, 2-10 O0-O8, 11 N.C., 12 GND, 13 GND, 14 ISET, 15 GND, 16 V+, 17 OSC, 18 V+, 19 V+, 20 N.C.,
   21-30 O9-O18, 31 BLINK, 32 OSC_OUT, 33 P2, 34 P3, 35 P4/IRQ, 36 P0, 37 P1, 38-40 the bus */
static const char *const TQFN_MID[] = {"O0", "O1", "O2", "O3", "O4", "O5", "O6", "O7", "O8", "NC", "GND", "GND",
                                       "ISET", "GND", "V+", "OSC", "V+", "V+", "NC", "O9", "O10", "O11", "O12",
                                       "O13", "O14", "O15", "O16", "O17", "O18", "BLINK", "OSC_OUT", "P2", "P3",
                                       "P4/IRQ", "P0", "P1"};

/* `bus`: the four bus pin names, in SSOP pin order 3-6 (SPI: CS DOUT CLK DIN; I2C: AD0 SDA SCL AD1); the TQFN
   has the one on pin 1 (the SSOP's pin 6) first and pins 3-5 (the SSOP's 3, 4, 5) last. */
static void pinouts(const char *ssop, const char *tqfn, const char *const bus[4], const char *what) {
    const Part *s = find(ssop), *t = find(tqfn);
    char text[200];
    size_t i;
    int ok = s->kind == PART_LEDDRIVER && s->pin_count == 36 && t->pin_count == 40;

    ok = ok && strcmp(s->pins[0].name, "P0") == 0 && strcmp(s->pins[1].name, "P1") == 0;
    for (i = 0; ok && i < 4; i++) {
        ok = strcmp(s->pins[2 + i].name, bus[i]) == 0;
    }
    for (i = 0; ok && i < sizeof SSOP_TAIL / sizeof *SSOP_TAIL; i++) {
        ok = strcmp(s->pins[6 + i].name, SSOP_TAIL[i]) == 0;
    }
    snprintf(text, sizeof text, "%s: the 36-pin SSOP pin names follow the pin configuration", what);
    check(ok, text);
    ok = strcmp(t->pins[0].name, bus[3]) == 0;
    for (i = 0; ok && i < sizeof TQFN_MID / sizeof *TQFN_MID; i++) {
        ok = strcmp(t->pins[1 + i].name, TQFN_MID[i]) == 0;
    }
    for (i = 0; ok && i < 3; i++) {
        ok = strcmp(t->pins[37 + i].name, bus[i]) == 0;
    }
    snprintf(text, sizeof text, "%s: the 40-pin TQFN pin names follow the pin configuration (P2 is pin 33, P3 pin 34)", what);
    check(ok, text);
}

/* Every pin of the part has a pad of its number in the part's footprint; `pads` pads in all. */
static void footprint_pads(const char *name, const char *id, size_t pads) {
    const Part *p = find(name);
    char text[200];
    size_t i, k, unnumbered = 0;
    int ok = p->footprint != NULL && strcmp(p->footprint->id, id) == 0 && p->footprint->pad_count == pads;

    for (i = 0; ok && i < p->pin_count; i++) {
        char number[16];
        int found = 0;

        snprintf(number, sizeof number, "%zu", i + 1);
        for (k = 0; k < p->footprint->pad_count; k++) {
            const char *d = p->footprint->pads[k].designator;

            if (d != NULL && strcmp(d, number) == 0) {
                found = 1;
            }
        }
        ok = found;
    }
    for (k = 0; ok && k < p->footprint->pad_count; k++) {
        if (p->footprint->pads[k].designator == NULL || strcmp(p->footprint->pads[k].designator, "NULL") == 0) {
            unnumbered++;
        }
    }
    snprintf(text, sizeof text, "%s: footprint %s has %zu pads and a pad for every pin (%zu unnumbered)", name, id, pads, unnumbered);
    check(ok && unnumbered == pads - p->pin_count, text);
}

/* ---- The GPIO ports (the same on both chips): open-drain outputs / inputs --------------------------------- */

/* `wr1(b, reg, v)` and `rd1(b, reg)`: one register through the chip's own bus. */
typedef struct {
    void (*wr1)(Bench *, unsigned, unsigned);
    unsigned (*rd1)(Bench *, unsigned);
} Access;

static void gpio(Bench *b, const Access *a) {
    a->wr1(b, 0x06, 0x1F);
    set(b, "P1", LOGIC_0);
    set(b, "P0", LOGIC_1);
    run(b, 10 * US);
    check((a->rd1(b, 0x05) & 0x03u) == 0x01, "GPIO read: P0 high, P1 low from the pins (all ports are inputs)");
    set(b, "P1", LOGIC_1);
    run(b, 10 * US);
    check((a->rd1(b, 0x05) & 0x03u) == 0x03, "GPIO read follows the pins");
    set(b, "P0", LOGIC_Z);
    set(b, "P1", LOGIC_Z);
    a->wr1(b, 0x05, 0x00);
    a->wr1(b, 0x06, 0x1B); /* P2 an output, the others inputs */
    run(b, 10 * US);
    check(get(b, "P2") == LOGIC_0, "port P2 as an output with data 0: the pin is driven low");
    a->wr1(b, 0x05, 0x04);
    run(b, 10 * US);
    check(get(b, "P2") == LOGIC_1 && (a->rd1(b, 0x05) & 0x04u) == 0x04, "data 1: the open-drain output is released (pulled high), the read shows the pin");
    a->wr1(b, 0x05, 0x00);
    run(b, 10 * US);
    check(get(b, "P2") == LOGIC_0 && (a->rd1(b, 0x05) & 0x04u) == 0x00, "back to 0: low, and the read shows the pin level");
    a->wr1(b, 0x06, 0x17); /* P3 an output with nothing pulling it up */
    a->wr1(b, 0x05, 0x08);
    run(b, 50 * US);
    check(get(b, "P3") == LOGIC_Z, "open-drain output with data 1 and no pull-up: the pin floats (not driven high or low)");
    a->wr1(b, 0x05, 0x00);
    run(b, 10 * US);
    check(get(b, "P3") == LOGIC_0, "data 0 drives P3 low");
    a->wr1(b, 0x05, 0x08);
    run(b, 10 * US);
    check(get(b, "P3") == LOGIC_Z, "and data 1 releases it again (the latch is not changed by the pin level)");
    a->wr1(b, 0x06, 0x0F); /* P4/IRQ an output */
    a->wr1(b, 0x05, 0x10);
    run(b, 10 * US);
    check(get(b, "P4/IRQ") == LOGIC_1 && (a->rd1(b, 0x05) & 0x10u) == 0x10, "P4 as an output: data 1 releases it, the read bit D4 follows");
    a->wr1(b, 0x05, 0x00);
    run(b, 10 * US);
    check(get(b, "P4/IRQ") == LOGIC_0, "P4 data 0 drives it low");
}

/* The O0-O18 pins are not driven by this model: they stay released. */
static void led_pins_released(Bench *b) {
    char name[8];
    int i, ok = 1;

    for (i = 0; i < 19; i++) {
        snprintf(name, sizeof name, "O%d", i);
        ok &= get(b, name) == LOGIC_Z;
    }
    check(ok && get(b, "BLINK") == LOGIC_Z && get(b, "OSC_OUT") == LOGIC_Z,
          "O0-O18, BLINK and OSC_OUT are not driven (released); see the NOT modeled list");
}

/* ---- MAX6955: I2C ------------------------------------------------------------------------------------ */

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

static unsigned addr7 = 0x60;

static int wr(Bench *b, unsigned r, const unsigned *v, size_t n) {
    size_t i;
    int ack;

    i2c_start(b);
    ack = i2c_byte(b, addr7 << 1);
    i2c_byte(b, r);
    for (i = 0; i < n; i++) {
        i2c_byte(b, v[i]);
    }
    i2c_stop(b);
    return ack;
}

static void wr1(Bench *b, unsigned r, unsigned v) {
    wr(b, r, &v, 1);
}

/* Command byte then a repeated START and the read (Figures 7 and 9). */
static void rd(Bench *b, unsigned r, unsigned *v, size_t n) {
    size_t i;

    i2c_start(b);
    i2c_byte(b, addr7 << 1);
    i2c_byte(b, r);
    i2c_stop(b);
    i2c_start(b);
    i2c_byte(b, (addr7 << 1) | 1u);
    for (i = 0; i < n; i++) {
        v[i] = i2c_in(b, i + 1 < n);
    }
    i2c_stop(b);
}

static unsigned rd1(Bench *b, unsigned r) {
    unsigned v = 0;

    rd(b, r, &v, 1);
    return v;
}

static const Access I2C = {wr1, rd1};

typedef struct {
    const char *name;
    int level; /* 0 GND, 1 V+, 2 SDA, 3 SCL */
} Tie;

/* Table 5: the 16 addresses. */
static void address_table(const char *part) {
    static const char *const pulls[] = {"SDA", "SCL", NULL};
    static const Tie TIES[4] = {{"GND", 0}, {"V+", 1}, {"SDA", 2}, {"SCL", 3}};
    char text[200];
    int ok = 1, a1, a0;

    for (a1 = 0; a1 < 4; a1++) {
        for (a0 = 0; a0 < 4; a0++) {
            Bench *b = bench(part, pulls);
            unsigned expect = 0x60u + 4u * (unsigned)a1 + (unsigned)a0;
            int i;

            for (i = 0; i < 2; i++) {
                const char *pin = i == 0 ? "AD0" : "AD1";
                int lvl = i == 0 ? TIES[a0].level : TIES[a1].level;

                if (lvl == 0) {
                    set(b, pin, LOGIC_0);
                } else if (lvl == 1) {
                    set(b, pin, LOGIC_1);
                } else if (lvl == 2) {
                    b->tie_sda[i] = pin;
                    set(b, pin, LOGIC_1);
                } else {
                    b->tie_scl[i] = pin;
                    set(b, pin, LOGIC_1);
                }
            }
            run(b, 10 * US);
            addr7 = expect;
            ok &= wr(b, 0x01, (const unsigned[]){0x00}, 1) == 1;
            addr7 = expect ^ 1u;
            ok &= wr(b, 0x01, (const unsigned[]){0x00}, 1) == 0;
            close_bench(b);
        }
    }
    addr7 = 0x60;
    snprintf(text, sizeof text, "%s Table 5: AD1/AD0 on GND, V+, SDA, SCL give 0x60 + 4 * AD1 + AD0 (GND 0, V+ 1, SDA 2, SCL 3), a neighbour address is not acknowledged", part);
    check(ok, text);
}

static void chip_i2c(const char *part) {
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
    check(rd1(b, 0x0C) == 0xA5, "digit type register (0x0C) stores its byte (CC7..CC0, Table 13); the model reads it back, the chip returns Key_A pressed");
    wr1(b, 0x0C, 0x00);
    wr1(b, 0x08, 0xFF);
    wr1(b, 0x09, 0xFF);
    check(rd1(b, 0x08) == 0x00 && rd1(b, 0x09) == 0x00,
          "0x08-0x0B: a write sets the key mask (not stored), a read is the debounced key register: 0");
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

    gpio(b, &I2C);
    led_pins_released(b);
    close_bench(b);
}

/* ---- MAX6954: SPI ------------------------------------------------------------------------------------ */

/* One frame (CS low, MSB first, DIN set up before CLK rises); returns the bits DOUT showed, each read just
   before its rising edge. `bits` other than 16: only the last `bits` of `word` are sent. */
static unsigned long frame(Bench *b, unsigned long word, unsigned bits) {
    unsigned i;
    unsigned long out = 0;

    set(b, "CS", LOGIC_0);
    run(b, HALF);
    for (i = 0; i < bits; i++) {
        set(b, "DIN", (word >> (bits - 1 - i)) & 1u ? LOGIC_1 : LOGIC_0);
        run(b, HALF);
        out = (out << 1) | (get(b, "DOUT") == LOGIC_1 ? 1u : 0u);
        set(b, "CLK", LOGIC_1);
        run(b, HALF);
        set(b, "CLK", LOGIC_0);
    }
    run(b, HALF);
    set(b, "CS", LOGIC_1);
    run(b, HALF);
    return out;
}

static void swr(Bench *b, unsigned addr, unsigned data) {
    frame(b, ((unsigned long)addr << 8) | data, 16);
}

/* A read: the command (D15 = 1), then a no-op whose frame shifts the answer out (p.8). Returns the 16 bits
   (the command byte echoed, the register byte) of that second frame. */
static unsigned srd16(Bench *b, unsigned addr) {
    frame(b, 0x8000u | (addr << 8), 16);
    return (unsigned)frame(b, 0, 16);
}

static unsigned srd1(Bench *b, unsigned addr) {
    return srd16(b, addr) & 0xFFu;
}

static const Access SPI = {swr, srd1};

static void chip_spi(const char *part) {
    static const char *const pulls[] = {"P2", "P4/IRQ", NULL};
    Bench *b = bench(part, pulls);
    unsigned v[16];
    int i, ok;

    set(b, "CS", LOGIC_1);
    set(b, "CLK", LOGIC_0);
    run(b, 10 * US);
    printf("-- %s\n", part);

    check(get(b, "DOUT") == LOGIC_Z, "DOUT is released while CS is high (the chip never floats it: a deviation)");
    /* Table 15 power-up */
    check(srd1(b, 0x01) == 0xFF && srd1(b, 0x02) == 0x00 && srd1(b, 0x03) == 0x07 && srd1(b, 0x04) == 0x00 &&
              srd1(b, 0x05) == 0x14 && srd1(b, 0x06) == 0x1F && srd1(b, 0x07) == 0x00 && srd1(b, 0x0C) == 0x00,
          "power-up (Table 15, p.24): decode 0xFF, intensity 0, scan limit 7, configuration 0 (shutdown), GPIO read 0x14 (the pulled-up P2 and P4 pins; the latch is 0), ports all inputs 0x1F, test 0, 0x0C reads Key_A pressed 0");
    ok = 1;
    for (i = 0; i < 16; i++) {
        ok &= srd1(b, 0x20u + (unsigned)i) == (i < 8 ? 0x20u : 0x00u);
        ok &= srd1(b, 0x40u + (unsigned)i) == (i < 8 ? 0x20u : 0x00u);
    }
    check(ok, "power-up: both planes hold 0x20 (blank) in digits 0-7 and 0x00 in digits 0a-7a (0x20-0x2F, 0x40-0x4F)");
    ok = 1;
    for (i = 0; i < 8; i++) {
        ok &= srd1(b, 0x10u + (unsigned)i) == 0;
    }
    check(ok && srd1(b, 0x08) == 0 && srd1(b, 0x09) == 0 && srd1(b, 0x0A) == 0 && srd1(b, 0x0B) == 0 &&
              srd1(b, 0x0D) == 0 && srd1(b, 0x0E) == 0 && srd1(b, 0x0F) == 0,
          "power-up: the intensity registers and the key registers read 0 (no keys pressed, no key scan)");

    /* the frame (Table 5) and the read-back through the next frame */
    check(srd16(b, 0x01) == 0x81FFu, "a read answers in the next frame: the command byte 0x81 echoed, then the register (0xFF)");
    swr(b, 0x01, 0x5A);
    check(frame(b, 0, 16) == 0x015Au, "after a write the next frame shifts the written frame out again (the daisy-chain copy, p.7)");
    swr(b, 0x02, 0xFF);
    swr(b, 0x03, 0xFF);
    check(srd1(b, 0x01) == 0x5A && srd1(b, 0x02) == 0x0F && srd1(b, 0x03) == 0x07,
          "decode mode stores a byte; global intensity 4 bits and scan limit 3 bits (unused bits read 0, Table 6 note)");
    swr(b, 0x04, 0x4D);
    check(srd1(b, 0x04) == 0x4D, "configuration: I, E, B and S are stored (Tables 16-23)");
    swr(b, 0x04, 0x01);
    check(srd1(b, 0x04) == 0x01, "configuration S = 1: normal operation");
    swr(b, 0x07, 0xFF);
    check(srd1(b, 0x07) == 0x01, "display test register stores D0 only");
    swr(b, 0x07, 0x00);
    swr(b, 0x0C, 0xA5);
    check(reg(b, "DTYPE") == 0xA5 && srd1(b, 0x0C) == 0x00, "digit type (0x0C) is written but cannot be read: a read is Key_A pressed (p.10, Table 6)");
    swr(b, 0x0C, 0x00);
    swr(b, 0x08, 0xFF);
    swr(b, 0x0B, 0x81);
    check(reg(b, "KEYA_MASK") == 0xFF && reg(b, "KEYD_MASK") == 0x81 && reg(b, "KEYB_MASK") == 0 && srd1(b, 0x08) == 0 && srd1(b, 0x0B) == 0,
          "0x08-0x0B: a write stores the key mask, a read is the debounced key register (0, Table 6)");
    swr(b, 0x0D, 0xFF);
    check(srd1(b, 0x0D) == 0x00, "the key pressed registers are read-only (do not write)");
    for (i = 0; i < 8; i++) {
        swr(b, 0x10u + (unsigned)i, 0x11u * (unsigned)i + 1u);
    }
    check(srd1(b, 0x10) == 0x01 && srd1(b, 0x11) == 0x12 && srd1(b, 0x14) == 0x45 && srd1(b, 0x17) == 0x78,
          "intensity registers 0x10-0x17 store bytes (Table 27)");

    /* digit planes */
    for (i = 0; i < 8; i++) {
        swr(b, 0x20u + (unsigned)i, 0x41u + (unsigned)i);
    }
    check(srd1(b, 0x20) == 0x41 && srd1(b, 0x27) == 0x48 && srd1(b, 0x28) == 0x00 && srd1(b, 0x29) == 0x00,
          "plane P0: digits 0-7 at 0x20-0x27, 0a and 1a unchanged");
    check(srd1(b, 0x40) == 0x20 && srd1(b, 0x47) == 0x20, "plane P1 is a separate RAM: still 0x20");
    for (i = 0; i < 8; i++) {
        swr(b, 0x40u + (unsigned)i, 0x80u | (0x30u + (unsigned)i));
    }
    check(srd1(b, 0x43) == 0xB3 && srd1(b, 0x23) == 0x44, "plane P1 write 0x40-0x47 (decimal point bit D7 kept), plane P0 untouched");
    swr(b, 0x28, 0x5A);
    swr(b, 0x29, 0xA5);
    check(srd1(b, 0x28) == 0x5A && srd1(b, 0x29) == 0xA5 && srd1(b, 0x48) == 0x00,
          "digits 0a and 1a (7-segment only) at 0x28 and 0x29 in plane P0");
    swr(b, 0x6A, 0x3C);
    check(srd1(b, 0x2A) == 0x3C && srd1(b, 0x4A) == 0x3C && srd1(b, 0x6A) == 0x00,
          "0x6A writes digit 2a in both planes P0 and P1 with the same data and reads as 0x00 (Table 6)");
    swr(b, 0x63, 0x99);
    swr(b, 0x60, 0x31);
    swr(b, 0x67, 0x37);
    check(srd1(b, 0x23) == 0x99 && srd1(b, 0x43) == 0x99 && srd1(b, 0x20) == 0x31 && srd1(b, 0x40) == 0x31 &&
              srd1(b, 0x27) == 0x37 && srd1(b, 0x47) == 0x37,
          "0x60, 0x63 and 0x67 write digits 0, 3 and 7 in both planes");
    swr(b, 0x4F, 0x12);
    check(srd1(b, 0x4F) == 0x12 && srd1(b, 0x2F) == 0x00, "digit 7a plane P1 at 0x4F");
    swr(b, 0x31, 0xEE);
    swr(b, 0x7F, 0xEE);
    check(srd1(b, 0x31) == 0x00 && srd1(b, 0x7F) == 0x00 && srd1(b, 0x21) == 0x42, "an address the table does not list changes nothing and reads 0");

    /* the frame: the last 16 bits count, a shorter frame does nothing (p.7, Figure 5) */
    frame(b, 0xAB217Eul, 24); /* last 16 bits: 0x217E: digit 1 = 0x7E */
    check(srd1(b, 0x21) == 0x7E, "a 24-bit frame stores its last 16 bits (the earlier bits are shifted past)");
    frame(b, 0x2155, 8); /* only the last 8 bits (0x55) clocked in: no 16-bit word */
    check(srd1(b, 0x21) == 0x7E, "a frame shorter than 16 bits changes nothing");

    /* R: global clear of both planes (Table 22) */
    swr(b, 0x04, 0x01);
    swr(b, 0x04, 0x21);
    ok = 1;
    for (i = 0; i < 16; i++) {
        ok &= srd1(b, 0x20u + (unsigned)i) == 0 && srd1(b, 0x40u + (unsigned)i) == 0;
    }
    check(ok && srd1(b, 0x04) == 0x01, "configuration R = 1 clears the digit data of both planes (to 0x00) and is not stored");
    check(srd1(b, 0x01) == 0x5A && srd1(b, 0x10) == 0x01, "R does not touch the control and intensity registers");

    gpio(b, &SPI);
    led_pins_released(b);
    close_bench(b);
}

int main(void) {
    static const char *const BUS54[4] = {"CS", "DOUT", "CLK", "DIN"};
    static const char *const BUS55[4] = {"AD0", "SDA", "SCL", "AD1"};
    static const char *const SSOP55[] = {"MAX6955AAX+", "MAX6955AAX+T"};
    size_t k;

    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL loading the parts\n");
        return 1;
    }
    pinouts("MAX6954AAX+", "MAX6954ATL+", BUS54, "MAX6954");
    pinouts("MAX6955AAX+", "MAX6955ATL+", BUS55, "MAX6955");
    check(strcmp(find("MAX6955AAX+T")->pins[3].name, "SDA") == 0 && find("MAX6955AAX+T")->pin_count == 36,
          "MAX6955AAX+T (tape and reel) has the 36-pin SSOP pinout");
    footprint_pads("MAX6954AAX+", "SSOP-36-A36", 36);
    footprint_pads("MAX6954ATL+", "TQFN-40-T4066", 41);
    for (k = 0; k < 2; k++) {
        footprint_pads(SSOP55[k], "SSOP-36-A36", 36);
    }
    footprint_pads("MAX6955ATL+", "TQFN-40-T4066", 41);
    address_table("MAX6955AAX+");
    chip_i2c("MAX6955AAX+");
    chip_i2c("MAX6955AAX+T");
    chip_i2c("MAX6955ATL+");
    chip_spi("MAX6954AAX+");
    chip_spi("MAX6954ATL+");
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("max6954_max6955_test: all passed\n");
    return 0;
}
