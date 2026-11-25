/*
 * pcf8576_family_test (plan-56-E): the NXP 40 x 4 I2C LCD drivers PCF8576D / PCF85176 / PCA85176 / PCA85276
 * (lcddriver.json, devices PCF85176, PCA85176, PCA85276) through their real I2C bus, wired to the real
 * GENERIC-LCD-* glass:
 *
 *   - the pinouts (TSSOP56 and TQFP64: BP0-BP3, S0-S39, the n.c. pins; PCA85276 pin 52 = T1, no A2)
 *   - the I2C address 0111 00 SA0 (0x38 / 0x39), no acknowledge for any other address
 *   - power-on: display disabled (every output released), RAM cleared
 *   - the command bytes with the C continuation bit: mode-set, load-data-pointer, device-select, bank-select,
 *     blink-select (stored only), several commands in one transfer
 *   - display data placed per mode (static 1 row, 1:2 2 rows, 1:3 3 rows, 1:4 4 rows per column, most
 *     significant bit first, pointer step 8 / 4 / 3 / 2), the 7-segment fills of Figure 13, Table 6 and the
 *     rewrite of Table 7
 *   - the subaddress counter against the hardware subaddress pins, pointer overflow, bits that fall into the
 *     next device
 *   - the input and output RAM banks in static and 1:2 (no effect in 1:3 and 1:4)
 *   - the display-enable bit and the waveforms: the backplane of each slot driven, the paired backplanes
 *     (static all four, 1:2 BP0 = BP2 and BP1 = BP3, 1:3 BP3 = BP1), the frame time of each device
 *   - the glass lights equal the RAM bits (column = segment pin, row = backplane)
 *
 * NOT modeled (and so not tested): LCD bias and VLCD, blinking (blink-select is only stored), the CLK / SYNC
 * cascade pins and OSC, the missing acknowledge of a display byte addressed to another subaddress, and the
 * refusal of a read address byte.
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

/* ---- The bench: the chip, a test driver, pull-ups, an optional glass ----------------------------- */

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
    const char *chip;
    const char *glass;
} Wire;

typedef struct {
    Sim *sim;
    SimInstance *chip, *glass, *drv, *pu;
    const Part *part, *glass_part;
    PinCount pins, pulls;
} Bench;

static const SimTime US = 1000000;
enum { HALF = 100000 };

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

/* Chip pin names that share a net with a glass pin: `wires` (NULL-terminated by a NULL chip). The
   glass is NULL for a bare chip. Weak pull-ups on SDA and SCL. */
static Bench *bench(const char *part, const char *glass, const Wire *wires) {
    static const char *const PULLS[] = {"SDA", "SCL"};
    Bench *b = calloc(1, sizeof *b);
    char (*nodes)[32] = calloc(256, sizeof *nodes);
    SimNet **nets = calloc(256, sizeof *nets);
    size_t i, k, n = 0;

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
    if (glass != NULL) {
        b->glass_part = part_library_find(library, glass);
        if (b->glass_part == NULL) {
            fprintf(stderr, "no glass %s\n", glass);
            exit(2);
        }
        b->glass = sim_add_instance(b->sim, kind_model(b->glass_part), "G1", b->glass_part);
    }
    for (i = 0; i < b->part->pin_count; i++) {
        char node[32];
        SimNet *net = NULL;
        const char *gname = NULL;

        for (k = 0; wires != NULL && wires[k].chip != NULL; k++) {
            if (strcmp(wires[k].chip, b->part->pins[i].name) == 0) {
                gname = wires[k].glass;
            }
        }
        if (gname != NULL) {
            snprintf(node, sizeof node, "G.%s", gname);
        } else {
            snprintf(node, sizeof node, "%s.%zu", b->part->pins[i].name, i);
        }
        for (k = 0; k < n; k++) {
            if (strcmp(nodes[k], node) == 0) {
                net = nets[k];
            }
        }
        if (net == NULL) {
            net = sim_add_net(b->sim, node);
            snprintf(nodes[n], sizeof nodes[n], "%s", node);
            nets[n++] = net;
            if (gname != NULL) {
                size_t g = pin_of(b->glass_part, gname);

                if (g == SIZE_MAX) {
                    fprintf(stderr, "no glass pin %s\n", gname);
                    exit(2);
                }
                sim_connect(b->sim, net, b->glass, g);
            }
        }
        sim_connect(b->sim, net, b->chip, i);
        sim_connect(b->sim, net, b->drv, i);
        for (k = 0; k < 2; k++) {
            if (strcmp(PULLS[k], b->part->pins[i].name) == 0) {
                sim_connect(b->sim, net, b->pu, k);
            }
        }
    }
    free(nodes);
    free(nets);
    if (!sim_start(b->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(b->sim));
        exit(2);
    }
    set(b, "SA0", LOGIC_0);
    run(b, HALF);
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

/* One write transfer to the 7-bit address `addr`; returns whether the address byte was acknowledged. */
static int xfer(Bench *b, unsigned addr, const unsigned *bytes, size_t n) {
    size_t i;
    int ack;

    i2c_start(b);
    ack = i2c_byte(b, addr << 1);
    for (i = 0; ack && i < n; i++) {
        i2c_byte(b, bytes[i]);
    }
    i2c_stop(b);
    return ack;
}

/* Command bytes (a continuation bit C in bit 7). */
static unsigned MODE(int c, int e, int bias, int m) {
    return (unsigned)(c << 7) | 0x40u | (unsigned)(e << 3) | (unsigned)(bias << 2) | (unsigned)m;
}
static unsigned PTR(int c, int p) {
    return (unsigned)(c << 7) | (unsigned)p;
}
static unsigned DEVSEL(int c, int a) {
    return (unsigned)(c << 7) | 0x60u | (unsigned)a;
}
static unsigned BANK(int c, int i, int o) {
    return (unsigned)(c << 7) | 0x78u | (unsigned)(i << 1) | (unsigned)o;
}
static unsigned BLINK(int c, int ab, int bf) {
    return (unsigned)(c << 7) | 0x70u | (unsigned)(ab << 2) | (unsigned)bf;
}

enum { M14 = 0, MSTATIC = 1, M12 = 2, M13 = 3 };

/* Commands "mode-set (E = 1), load-data-pointer P" and then the data bytes, in one transfer. */
static void load(Bench *b, int mode, int ptr, const unsigned *data, size_t n) {
    unsigned bytes[64];
    size_t i;

    bytes[0] = MODE(1, 1, 0, mode);
    bytes[1] = PTR(0, ptr);
    for (i = 0; i < n; i++) {
        bytes[2 + i] = data[i];
    }
    xfer(b, 0x38, bytes, 2 + n);
}

/* ---- The reference RAM (written from the datasheet's description, independent of the model) ---------- */

typedef struct {
    unsigned char bit[40][4];
} Ram;

/* A byte arrives in a mode of `rows` backplanes with the data pointer at `ptr`: bit j (7 first) is bit
   number ptr * rows + j of the column-major stream: column stream / rows, row stream % rows. */
static void ref_byte(Ram *r, int rows, int ptr, int row0, unsigned byte) {
    int j;

    for (j = 0; j < 8; j++) {
        int s = ptr * rows + j, col = s / rows, row = s % rows + row0;

        if (col < 40 && row < 4) {
            r->bit[col][row] = (byte >> (7 - j)) & 1u;
        }
    }
}

static int rows_of(int mode) {
    return mode == MSTATIC ? 1 : mode == M12 ? 2 : mode == M13 ? 3 : 4;
}

static int step_of(int mode) {
    return mode == MSTATIC ? 8 : mode == M12 ? 4 : mode == M13 ? 3 : 2;
}

/* ---- The glass ----------------------------------------------------------------------------------- */

typedef struct {
    const char *name;
    int per_digit_pins;
    int bps;
    int cells;
} Glass;

static const Glass G14 = {"GENERIC-LCD-8D-1:4", 2, 4, 64};
static const Glass G13 = {"GENERIC-LCD-8D-1:3", 3, 3, 64};
static const Glass G12 = {"GENERIC-LCD-4D-1:2", 4, 2, 32};
static const Glass GST = {"GENERIC-LCD-4D-STATIC", 8, 1, 32};

/* The wires of chip S0.. to the glass's segment pins in pinout order, and the backplanes. */
static Wire *wires_for(const Glass *g, const Part *glass) {
    Wire *w = calloc(80, sizeof *w);
    size_t i, n = 0;
    int seg = 0;
    static char names[80][8];
    static char gnames[80][8];

    for (i = 0; i < glass->pin_count; i++) {
        const char *name = glass->pins[i].name;

        if (glass->pins[i].role == PIN_NC || strncmp(name, "BP", 2) == 0 || strcmp(name, "COM") == 0) {
            continue;
        }
        snprintf(names[n], 8, "S%d", seg++);
        snprintf(gnames[n], 8, "%s", name);
        w[n].chip = names[n];
        w[n].glass = gnames[n];
        n++;
    }
    {
        static const char *const BP4[4][2] = {{"BP0", "BP1"}, {"BP1", "BP2"}, {"BP2", "BP3"}, {"BP3", "BP4"}};
        static const char *const BP3[4][2] = {{"BP0", "BP1"}, {"BP1", "BP2"}, {"BP2", "BP3"}, {"BP3", "BP2"}};
        static const char *const BP2[4][2] = {{"BP0", "BP1"}, {"BP1", "BP2"}, {"BP2", "BP1"}, {"BP3", "BP2"}};
        static const char *const BP1[4][2] = {{"BP0", "COM"}, {"BP1", "COM"}, {"BP2", "COM"}, {"BP3", "COM"}};
        const char *(*t)[2] = g->bps == 4 ? (const char *(*)[2])BP4 : g->bps == 3 ? (const char *(*)[2])BP3 :
                              g->bps == 2 ? (const char *(*)[2])BP2 : (const char *(*)[2])BP1;
        int k;

        for (k = 0; k < 4; k++) {
            w[n].chip = t[k][0];
            w[n].glass = t[k][1];
            n++;
        }
    }
    return w;
}

static Bench *glass_bench(const char *part, const Glass *g) {
    Bench *b;
    Wire *w;
    const Part *gp = part_library_find(library, g->name);

    if (gp == NULL) {
        fprintf(stderr, "no glass %s\n", g->name);
        exit(2);
    }
    w = wires_for(g, gp);
    b = bench(part, g->name, w);
    free(w);
    return b;
}

static size_t glass_lights(Bench *b, SimTime since, unsigned char *out, size_t cap) {
    const SimComponentType *type = sim_instance_type(b->glass);

    return type->lights(b->sim, b->glass, sim_instance_state(b->glass), since, out, cap);
}

/* The lights over the last two frames equal the reference RAM: glass cell n is segment pin
   per_digit_pins * (n / 8) + (n % 8) / bps, backplane (n % 8) % bps. */
static int lights_match(Bench *b, const Glass *g, const Ram *ram, unsigned frame_us, char *diff, size_t cap) {
    unsigned char lit[128];
    size_t n = glass_lights(b, sim_now(b->sim) - (SimTime)frame_us * 2 * US, lit, sizeof lit);
    int c, ok = n == (size_t)g->cells;

    diff[0] = '\0';
    for (c = 0; ok && c < g->cells; c++) {
        int pin = g->per_digit_pins * (c / 8) + (c % 8) / g->bps, bp = (c % 8) % g->bps;

        if ((lit[c] != 0) != (ram->bit[pin][bp] != 0)) {
            snprintf(diff, cap, "cell %d (pin %d bp %d): lit %d want %d", c, pin, bp, lit[c], ram->bit[pin][bp]);
            ok = 0;
        }
    }
    return ok;
}

/* ---- Pinouts and address ------------------------------------------------------------------------- */

static const char *pin_name(const Part *p, unsigned number) {
    size_t i;

    for (i = 0; i < p->pin_count; i++) {
        if (p->pins[i].designator != NULL && (unsigned)atoi(p->pins[i].designator) == number) {
            return p->pins[i].name;
        }
    }
    return "?";
}

static int pin_is(const Part *p, unsigned number, const char *name) {
    return strcmp(pin_name(p, number), name) == 0;
}

static void pinouts(void) {
    const Part *p = part_library_find(library, "PCF8576DT/2,518");
    const Part *h = part_library_find(library, "PCA85176H/Q900/1,5");
    unsigned i;
    int ok = 1;

    check(p != NULL && h != NULL, "the parts load");
    if (p == NULL || h == NULL) {
        exit(2);
    }
    check(p->kind == PART_LCDDRIVER && p->pin_count == 56, "PCF8576DT/2,518: lcddriver, 56 pins");
    ok = pin_is(p, 1, "BP2") && pin_is(p, 2, "BP1") && pin_is(p, 3, "BP3") && pin_is(p, 56, "BP0");
    check(ok, "TSSOP56: BP2 1, BP1 2, BP3 3, BP0 56 (pcf8576d.pdf p.4)");
    ok = 1;
    for (i = 0; i < 40; i++) {
        char want[8];

        snprintf(want, sizeof want, "S%u", i);
        ok &= pin_is(p, 4 + i, want);
    }
    check(ok, "TSSOP56: S0-S39 on pins 4-43");
    ok = pin_is(p, 44, "SDA") && pin_is(p, 45, "SCL") && pin_is(p, 46, "SYNC") && pin_is(p, 47, "CLK") &&
         pin_is(p, 48, "VDD") && pin_is(p, 49, "OSC") && pin_is(p, 50, "A0") && pin_is(p, 51, "A1") &&
         pin_is(p, 52, "A2") && pin_is(p, 53, "SA0") && pin_is(p, 54, "VSS") && pin_is(p, 55, "VLCD");
    check(ok, "TSSOP56: SDA 44, SCL 45, SYNC 46, CLK 47, VDD 48, OSC 49, A0-A2 50-52, SA0 53, VSS 54, VLCD 55");
    check(h != NULL && h->pin_count == 64, "PCA85176H: 64 pins");
    ok = pin_is(h, 10, "SDA") && pin_is(h, 11, "SCL") && pin_is(h, 12, "SYNC") && pin_is(h, 13, "CLK") &&
         pin_is(h, 14, "VDD") && pin_is(h, 15, "OSC") && pin_is(h, 16, "A0") && pin_is(h, 17, "A1") &&
         pin_is(h, 18, "A2") && pin_is(h, 19, "SA0") && pin_is(h, 20, "VSS") && pin_is(h, 21, "VLCD") &&
         pin_is(h, 25, "BP0") && pin_is(h, 26, "BP2") && pin_is(h, 27, "BP1") && pin_is(h, 28, "BP3");
    check(ok, "TQFP64: SDA 10, SCL 11, SYNC 12, CLK 13, VDD 14, OSC 15, A0-A2 16-18, SA0 19, VSS 20, VLCD 21, BP0 25, BP2 26, BP1 27, BP3 28");
    ok = pin_is(h, 29, "S0") && pin_is(h, 32, "S3") && pin_is(h, 34, "S4") && pin_is(h, 47, "S17") &&
         pin_is(h, 49, "S18") && pin_is(h, 64, "S33") && pin_is(h, 2, "S34") && pin_is(h, 7, "S39");
    check(ok, "TQFP64: S0-S3 29-32, S4-S17 34-47, S18-S33 49-64, S34-S39 2-7 (pcf85176.pdf p.5)");
    ok = 1;
    {
        static const unsigned NCS[] = {1, 8, 9, 22, 23, 24, 33, 48};

        for (i = 0; i < 8; i++) {
            size_t k;

            ok &= pin_is(h, NCS[i], "NC");
            for (k = 0; k < h->pin_count; k++) {
                if (h->pins[k].designator != NULL && (unsigned)atoi(h->pins[k].designator) == NCS[i]) {
                    ok &= h->pins[k].role == PIN_NC;
                }
            }
        }
    }
    check(ok, "TQFP64: pins 1, 8, 9, 22-24, 33, 48 are n.c.");
}

static void address(void) {
    Bench *b = glass_bench("PCF8576DT/2,518", &G14);
    unsigned none[1] = {0};
    int ok0, ok1, ok2;

    run(b, 10 * US);
    ok0 = xfer(b, 0x38, none, 0);
    ok1 = xfer(b, 0x39, none, 0);
    ok2 = xfer(b, 0x3A, none, 0);
    check(ok0 && !ok1 && !ok2, "SA0 low: 0111000 acknowledged, 0111001 and 0111010 not");
    set(b, "SA0", LOGIC_1);
    run(b, 10 * US);
    ok0 = xfer(b, 0x38, none, 0);
    ok1 = xfer(b, 0x39, none, 0);
    check(!ok0 && ok1, "SA0 high: 0111001 acknowledged, 0111000 not");
    close_bench(b);
}

/* ---- Waveforms --------------------------------------------------------------------------------- */

/* Over `frames` frames sampled every 20 us: per backplane, whether it was driven low, driven high, and
   how many samples it was driven. Also whether two slots (different signal groups) were driven at once. */
typedef struct {
    unsigned low[4], high[4], driven[4];
    unsigned samples;
    unsigned groups_seen;
    int mixed;
} Wave;

static void sample_waves(Bench *b, unsigned us, Wave *w) {
    static const char *const BPS[4] = {"BP0", "BP1", "BP2", "BP3"};
    unsigned t;

    memset(w, 0, sizeof *w);
    for (t = 0; t < us; t += 20) {
        int i;
        LogicValue v[4];

        for (i = 0; i < 4; i++) {
            v[i] = get(b, BPS[i]);
            if (v[i] == LOGIC_0 || v[i] == LOGIC_1) {
                w->driven[i]++;
                if (v[i] == LOGIC_0) {
                    w->low[i]++;
                } else {
                    w->high[i]++;
                }
            }
        }
        w->samples++;
        run(b, 20 * US);
    }
}

/* ---- The chip ------------------------------------------------------------------------------------ */

static void poweron(const char *part, unsigned frame_us) {
    Bench *b = glass_bench(part, &G14);
    Ram ram;
    char diff[160];
    int ok = 1, i;
    static const char *const OUT[] = {"BP0", "BP1", "BP2", "BP3", "S0", "S1", "S17", "S39"};
    char text[200];

    memset(&ram, 0, sizeof ram);
    run(b, (SimTime)frame_us * 3 * US);
    for (i = 0; i < 8; i++) {
        ok &= get(b, OUT[i]) == LOGIC_Z;
    }
    snprintf(text, sizeof text, "%s: power-on: the display is disabled (E = 0), every backplane and segment output is released", part);
    check(ok, text);
    snprintf(text, sizeof text, "%s: power-on: the glass is dark", part);
    check(lights_match(b, &G14, &ram, frame_us, diff, sizeof diff), text);
    close_bench(b);
}

/* The whole mode and data suite on one part. `frame_us` is the half-period of the model's frame:
   Tfr / 2 (the datasheet's frame time is a full AC period). */
static void suite(const char *part, unsigned frame_us) {
    char text[240], diff[160];
    Ram ram;
    Bench *b;

    /* ---- 1:4 multiplex: Figure 13's 7-segment fill, bytes a c b DP f e g d. */
    b = glass_bench(part, &G14);
    memset(&ram, 0, sizeof ram);
    run(b, 10 * US);
    {
        /* digit "1" at columns 0-1 (b and c: 0x60), "8" at columns 2-3 (0xFF), DP of digit 3 at column 5 row 3 */
        unsigned data[3] = {0x60, 0xFF, 0x10};
        int k;

        load(b, M14, 0, data, 3);
        for (k = 0; k < 3; k++) {
            ref_byte(&ram, 4, k * 2, 0, data[k]);
        }
    }
    run(b, (SimTime)frame_us * 4 * US);
    snprintf(text, sizeof text, "%s: 1:4: three bytes at pointer 0 (step 2) light the cells of the RAM columns 0-5", part);
    check(lights_match(b, &G14, &ram, frame_us, diff, sizeof diff), text);
    if (diff[0]) {
        printf("  %s\n", diff);
    }
    check(ram.bit[0][1] == 1 && ram.bit[0][2] == 1 && ram.bit[0][0] == 0 && ram.bit[2][0] == 1 && ram.bit[5][3] == 0 &&
              ram.bit[4][3] == 1,
          "1:4: the reference itself: byte 0x60 puts bits in rows 1 and 2 of column 0 (a c b DP = 0 1 1 0)");
    {
        /* a byte at an explicit pointer: P = 10 */
        unsigned data[2] = {0x81, 0x42};

        load(b, M14, 10, data, 2);
        ref_byte(&ram, 4, 10, 0, data[0]);
        ref_byte(&ram, 4, 12, 0, data[1]);
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: 1:4: load-data-pointer 10 places the next bytes at columns 10-13 and keeps the rest", part);
        check(lights_match(b, &G14, &ram, frame_us, diff, sizeof diff), text);
        if (diff[0]) {
            printf("  %s\n", diff);
        }
    }
    {
        /* all RAM ones (20 bytes of 0xFF from pointer 0) */
        unsigned data[20];
        int k;

        for (k = 0; k < 20; k++) {
            data[k] = 0xFF;
            ref_byte(&ram, 4, k * 2, 0, 0xFF);
        }
        load(b, M14, 0, data, 20);
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: 1:4: 20 bytes of 0xFF fill the 40 x 4 RAM: every cell lights", part);
        check(lights_match(b, &G14, &ram, frame_us, diff, sizeof diff), text);
    }
    /* ---- the display-enable bit: E = 0 releases every output, the RAM stays */
    {
        unsigned off[2] = {MODE(0, 0, 0, M14), 0};
        unsigned on[2] = {MODE(0, 1, 0, M14), 0};
        Ram zero;

        memset(&zero, 0, sizeof zero);
        xfer(b, 0x38, off, 1);
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: mode-set with E = 0 releases BP0-BP3 and the segment outputs and darkens the glass", part);
        check(get(b, "BP0") == LOGIC_Z && get(b, "BP3") == LOGIC_Z && get(b, "S0") == LOGIC_Z && get(b, "S39") == LOGIC_Z &&
                  lights_match(b, &G14, &zero, frame_us, diff, sizeof diff),
              text);
        xfer(b, 0x38, on, 1);
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: mode-set with E = 1 brings the RAM back", part);
        check(lights_match(b, &G14, &ram, frame_us, diff, sizeof diff), text);
    }
    /* ---- the waveforms in 1:4: one backplane at a time, each a quarter of the frame */
    {
        Wave w;
        int i;
        unsigned span = frame_us * 4;
        unsigned slot_samples = frame_us / 4 / 20;
        int ok = 1;

        sample_waves(b, span, &w);
        for (i = 0; i < 4; i++) {
            ok &= w.low[i] > 0 && w.high[i] > 0;
            ok &= w.driven[i] + 4 >= slot_samples * 4 && w.driven[i] <= slot_samples * 4 + 8;
        }
        snprintf(text, sizeof text, "%s: 1:4: each backplane is driven low and high, for a quarter of the frame (%u us slots)", part, frame_us / 4);
        check(ok, text);
        if (!ok) {
            printf("  samples %u: driven %u %u %u %u, want about %u\n", w.samples, w.driven[0], w.driven[1], w.driven[2],
                   w.driven[3], slot_samples * 4);
        }
    }
    /* ---- blink-select and the C bit: five commands chained in one transfer, then a data byte */
    {
        unsigned bytes[8];
        Ram r2 = ram;

        bytes[0] = BLINK(1, 1, 2);
        bytes[1] = BANK(1, 0, 0);
        bytes[2] = DEVSEL(1, 0);
        bytes[3] = MODE(1, 1, 1, M14);
        bytes[4] = PTR(0, 4);
        bytes[5] = 0xA5;
        xfer(b, 0x38, bytes, 6);
        ref_byte(&r2, 4, 4, 0, 0xA5);
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: blink-select, bank-select, device-select, mode-set (bias 1/2) and load-data-pointer chained by the C bit, then a data byte", part);
        check(lights_match(b, &G14, &r2, frame_us, diff, sizeof diff), text);
        if (diff[0]) {
            printf("  %s\n", diff);
        }
        ram = r2;
    }
    /* ---- a mode change keeps the RAM: static shows row 0 of every column on all four backplanes */
    {
        unsigned st[1] = {MODE(0, 1, 0, MSTATIC)};
        Ram view;
        int c, k;

        memset(&view, 0, sizeof view);
        for (c = 0; c < 40; c++) {
            for (k = 0; k < 4; k++) {
                view.bit[c][k] = ram.bit[c][0];
            }
        }
        xfer(b, 0x38, st, 1);
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: mode-set to static keeps the RAM and shows its row 0 (the four backplanes carry one signal)", part);
        check(lights_match(b, &G14, &view, frame_us, diff, sizeof diff), text);
        if (diff[0]) {
            printf("  %s\n", diff);
        }
        st[0] = MODE(0, 1, 0, M14);
        xfer(b, 0x38, st, 1);
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: mode-set back to 1:4 shows all four rows again", part);
        check(lights_match(b, &G14, &ram, frame_us, diff, sizeof diff), text);
    }
    close_bench(b);

    /* ---- 1:3 multiplex: Table 6 (3 bytes fill 8 columns, bit 3 of the third address untouched), Table 7 */
    b = glass_bench(part, &G13);
    memset(&ram, 0, sizeof ram);
    run(b, 10 * US);
    {
        unsigned data[3] = {0xA5, 0x3C, 0xC3};
        int k;

        load(b, M13, 0, data, 3);
        for (k = 0; k < 3; k++) {
            ref_byte(&ram, 3, k * 3, 0, data[k]);
        }
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: 1:3: three bytes fill rows 0-2 of columns 0-7, pointer step 3 (Figure 13, Table 6)", part);
        check(lights_match(b, &G13, &ram, frame_us, diff, sizeof diff), text);
        if (diff[0]) {
            printf("  %s\n", diff);
        }
    }
    check(ram.bit[0][0] == 1 && ram.bit[0][1] == 0 && ram.bit[0][2] == 1 && ram.bit[1][0] == 0 && ram.bit[1][1] == 0 &&
              ram.bit[1][2] == 1 && ram.bit[2][0] == 0 && ram.bit[2][1] == 1 && ram.bit[2][2] == 0,
          "1:3: the reference: byte a (a7 a6 a5 / a4 a3 a2 / a1 a0) = 0xA5 fills column 0 rows 0-2 = a7 a6 a5, column 1 = a4 a3 a2, column 2 rows 0-1 = a1 a0");
    {
        /* Table 7: a second byte written at the address of bit a1 (column 2) overwrites a1 and a0 with b7 and b6 */
        unsigned data[1] = {0xC0};

        load(b, M13, 2, data, 1);
        ref_byte(&ram, 3, 2, 0, data[0]);
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: 1:3: a byte written at pointer 2 overwrites bits a1, a0 of column 2 with b7, b6 (Table 7)", part);
        check(lights_match(b, &G13, &ram, frame_us, diff, sizeof diff), text);
        if (diff[0]) {
            printf("  %s\n", diff);
        }
    }
    {
        /* bank-select has no effect in 1:3 (and 1:4): the input bank bit does not move the data */
        unsigned bytes[4] = {BANK(1, 1, 1), MODE(1, 1, 0, M13), PTR(0, 9), 0xFF};

        xfer(b, 0x38, bytes, 4);
        ref_byte(&ram, 3, 9, 0, 0xFF);
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: 1:3: bank-select I = O = 1 has no effect (data in rows 0-2, rows 0-2 displayed)", part);
        check(lights_match(b, &G13, &ram, frame_us, diff, sizeof diff), text);
    }
    {
        /* Waveform: BP3 carries the same signal as BP1 and BP0, BP1 (and BP3), BP2 take a third of the frame */
        Wave w;
        unsigned slot = frame_us / 3 / 20;
        int ok;
        int agree = 1;
        unsigned t;

        for (t = 0; t < frame_us * 3; t += 20) {
            agree &= get(b, "BP3") == get(b, "BP1");
            run(b, 20 * US);
        }
        sample_waves(b, frame_us * 4, &w);
        ok = w.low[0] > 0 && w.high[0] > 0 && w.low[2] > 0 && w.high[2] > 0 && w.driven[0] + 4 >= slot * 4 &&
             w.driven[0] <= slot * 4 + 8 && w.driven[1] + 4 >= slot * 4 && w.driven[1] <= slot * 4 + 8 &&
             w.driven[3] == w.driven[1];
        snprintf(text, sizeof text, "%s: 1:3: BP3 carries the same signal as BP1 (paired), each backplane slot is a third of the frame (%u us)", part, frame_us / 3);
        check(ok && agree, text);
        if (!(ok && agree)) {
            printf("  driven %u %u %u %u, want about %u\n", w.driven[0], w.driven[1], w.driven[2], w.driven[3], slot * 4);
        }
    }
    close_bench(b);

    /* ---- 1:2 multiplex: 4 columns x 2 rows per byte, step 4, banks (rows 2-3) */
    b = glass_bench(part, &G12);
    memset(&ram, 0, sizeof ram);
    run(b, 10 * US);
    {
        unsigned data[4] = {0xA5, 0x3C, 0x0F, 0xF0};
        int k;

        load(b, M12, 0, data, 4);
        for (k = 0; k < 4; k++) {
            ref_byte(&ram, 2, k * 4, 0, data[k]);
        }
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: 1:2: four bytes fill rows 0-1 of columns 0-15, pointer step 4", part);
        check(lights_match(b, &G12, &ram, frame_us, diff, sizeof diff), text);
        if (diff[0]) {
            printf("  %s\n", diff);
        }
    }
    {
        /* input bank 1 (rows 2-3): the data go to rows 2-3 and the displayed rows 0-1 do not change */
        unsigned bytes[4] = {BANK(1, 1, 0), MODE(1, 1, 0, M12), PTR(0, 0), 0xFF};
        Ram shown = ram;

        xfer(b, 0x38, bytes, 4);
        ref_byte(&ram, 2, 0, 2, 0xFF);
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: 1:2: bank-select I = 1, O = 0: new data lands in rows 2-3 and the shown rows 0-1 stay", part);
        check(lights_match(b, &G12, &shown, frame_us, diff, sizeof diff), text);
        {
            /* output bank 1: rows 2-3 are shown (the row-2 bits sit in backplane slot 0) */
            unsigned sw[2] = {BANK(0, 1, 1), 0};
            Ram view;
            int c;

            memset(&view, 0, sizeof view);
            for (c = 0; c < 40; c++) {
                view.bit[c][0] = ram.bit[c][2];
                view.bit[c][1] = ram.bit[c][3];
            }
            xfer(b, 0x38, sw, 1);
            run(b, (SimTime)frame_us * 4 * US);
            snprintf(text, sizeof text, "%s: 1:2: bank-select O = 1 displays RAM rows 2-3 on BP0, BP1", part);
            check(lights_match(b, &G12, &view, frame_us, diff, sizeof diff), text);
            {
                /* input bank back to 0 while O = 1: new data go to rows 0-1 and are not shown yet */
                unsigned back[4] = {BANK(1, 0, 1), PTR(0, 0), 0x00, 0x00};

                xfer(b, 0x38, back, 4);
                ref_byte(&ram, 2, 0, 0, 0x00);
                ref_byte(&ram, 2, 4, 0, 0x00);
                run(b, (SimTime)frame_us * 4 * US);
                snprintf(text, sizeof text, "%s: 1:2: I = 0, O = 1: data written to rows 0-1 are not shown, rows 2-3 still are", part);
                check(lights_match(b, &G12, &view, frame_us, diff, sizeof diff), text);
            }
        }
    }
    {
        /* Waveform in 1:2: BP2 = BP0 and BP3 = BP1, half the frame each */
        Wave w;
        int agree = 1;
        unsigned t, slot = frame_us / 2 / 20;
        unsigned sw[1] = {BANK(0, 0, 0)};

        xfer(b, 0x38, sw, 1);
        for (t = 0; t < frame_us * 3; t += 20) {
            agree &= get(b, "BP2") == get(b, "BP0") && get(b, "BP3") == get(b, "BP1");
            run(b, 20 * US);
        }
        sample_waves(b, frame_us * 4, &w);
        snprintf(text, sizeof text, "%s: 1:2: BP2 = BP0 and BP3 = BP1, each pair driven half the frame (%u us)", part, frame_us / 2);
        check(agree && w.driven[0] + 4 >= slot * 4 && w.driven[0] <= slot * 4 + 8 && w.driven[1] + 4 >= slot * 4 &&
                  w.driven[1] <= slot * 4 + 8 && w.low[0] > 0 && w.high[1] > 0,
              text);
    }
    close_bench(b);

    /* ---- static: one row, step 8, banks (row 2), all four backplanes the same */
    b = glass_bench(part, &GST);
    memset(&ram, 0, sizeof ram);
    run(b, 10 * US);
    {
        unsigned data[4] = {0xA5, 0x3C, 0x81, 0x7E};
        int k;

        load(b, MSTATIC, 0, data, 4);
        for (k = 0; k < 4; k++) {
            ref_byte(&ram, 1, k * 8, 0, data[k]);
        }
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: static: four bytes fill row 0 of columns 0-31, pointer step 8, MSB first", part);
        check(lights_match(b, &GST, &ram, frame_us, diff, sizeof diff), text);
        if (diff[0]) {
            printf("  %s\n", diff);
        }
    }
    {
        unsigned bytes[4] = {BANK(1, 1, 0), MODE(1, 1, 0, MSTATIC), PTR(0, 0), 0xFF};
        Ram shown = ram;

        xfer(b, 0x38, bytes, 4);
        ref_byte(&ram, 1, 0, 2, 0xFF);
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: static: I = 1 stores the bits in row 2 and the shown row 0 stays", part);
        check(lights_match(b, &GST, &shown, frame_us, diff, sizeof diff), text);
        {
            unsigned sw[1] = {BANK(0, 1, 1)};
            Ram view;
            int c;

            memset(&view, 0, sizeof view);
            for (c = 0; c < 40; c++) {
                view.bit[c][0] = ram.bit[c][2];
            }
            xfer(b, 0x38, sw, 1);
            run(b, (SimTime)frame_us * 4 * US);
            snprintf(text, sizeof text, "%s: static: O = 1 displays row 2", part);
            check(lights_match(b, &GST, &view, frame_us, diff, sizeof diff), text);
        }
    }
    {
        /* all four backplanes carry the same signal and alternate each frame */
        Wave w;
        unsigned t;
        int same = 1;

        for (t = 0; t < frame_us * 4; t += 20) {
            LogicValue a = get(b, "BP0");

            same &= (a == LOGIC_0 || a == LOGIC_1) && get(b, "BP1") == a && get(b, "BP2") == a && get(b, "BP3") == a;
            run(b, 20 * US);
        }
        sample_waves(b, frame_us * 4, &w);
        snprintf(text, sizeof text, "%s: static: BP0-BP3 carry one signal, driven the whole time, low and high on alternate frames", part);
        check(same && w.low[0] > 0 && w.high[0] > 0 && w.driven[0] == w.samples, text);
    }
    /* a mode change keeps the RAM: back to static after the 1:4 etc. is the same RAM */
    close_bench(b);

    /* ---- subaddress counter and hardware subaddress, pointer overflow and spill (1:4) */
    {
        /* hardware subaddress 1 (A0 high): data are stored only while the counter is 1 */
        Ram zero, expect;
        unsigned bytes[6];

        b = glass_bench(part, &G14);
        memset(&ram, 0, sizeof ram);
        memset(&zero, 0, sizeof zero);
        set(b, "A0", LOGIC_1);
        run(b, 10 * US);
        bytes[0] = MODE(1, 1, 0, M14);
        bytes[1] = DEVSEL(1, 0);
        bytes[2] = PTR(0, 0);
        bytes[3] = 0xFF;
        bytes[4] = 0xFF;
        xfer(b, 0x38, bytes, 5);
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: device-select 0 with hardware subaddress 1 (A0 high): the data are not stored", part);
        check(lights_match(b, &G14, &zero, frame_us, diff, sizeof diff), text);
        bytes[1] = DEVSEL(1, 1);
        xfer(b, 0x38, bytes, 5);
        ref_byte(&ram, 4, 0, 0, 0xFF);
        ref_byte(&ram, 4, 2, 0, 0xFF);
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: device-select 1 matches hardware subaddress 1: the data are stored", part);
        check(lights_match(b, &G14, &ram, frame_us, diff, sizeof diff), text);
        /* counter 0: the bytes at pointers 36 and 38 are skipped (the pointer still steps by 2 each); the next
           would be at pointer 40: it wraps to 0 and the counter to 1, which matches: stored at columns 0-1 */
        bytes[0] = MODE(1, 1, 0, M14);
        bytes[1] = DEVSEL(1, 0);
        bytes[2] = PTR(0, 36);
        bytes[3] = 0x11;
        bytes[4] = 0x22;
        bytes[5] = 0x33;
        xfer(b, 0x38, bytes, 6);
        ref_byte(&ram, 4, 0, 0, 0x33);
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: skipped bytes still step the pointer; past column 39 it wraps to 0 and the counter increments", part);
        check(lights_match(b, &G14, &ram, frame_us, diff, sizeof diff), text);
        if (diff[0]) {
            printf("  %s\n", diff);
        }
        /* a byte at pointer 39 with counter 0: column 39 is the counter-0 device's, column 40 falls into the
           next device (counter 1: this one): its bits land in column 0 */
        bytes[1] = DEVSEL(1, 0);
        bytes[2] = PTR(0, 39);
        bytes[3] = 0x0B;
        xfer(b, 0x38, bytes, 4);
        expect = ram;
        expect.bit[0][0] = 1;
        expect.bit[0][1] = 0;
        expect.bit[0][2] = 1;
        expect.bit[0][3] = 1;
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: a byte at pointer 39 (counter 0): its second nibble 1011 falls into column 0 of the next device (subaddress 1, this one)", part);
        check(lights_match(b, &G14, &expect, frame_us, diff, sizeof diff), text);
        if (diff[0]) {
            printf("  %s\n", diff);
        }
        close_bench(b);
    }
    {
        /* a single device (hardware subaddress 0, counter 0): the bits past column 39 are discarded */
        Ram expect;
        unsigned bytes[4] = {MODE(1, 1, 0, M14), DEVSEL(1, 0), PTR(0, 39), 0x0B};

        b = glass_bench(part, &G14);
        memset(&expect, 0, sizeof expect);
        run(b, 10 * US);
        xfer(b, 0x38, bytes, 4);
        ref_byte(&expect, 4, 39, 0, 0x00);
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: single device: a byte at pointer 39 stores its first nibble (0000) in column 39 and discards the rest", part);
        check(lights_match(b, &G14, &expect, frame_us, diff, sizeof diff), text);
        bytes[3] = 0xF1;
        xfer(b, 0x38, bytes, 4);
        expect.bit[39][0] = 1;
        expect.bit[39][1] = 1;
        expect.bit[39][2] = 1;
        expect.bit[39][3] = 1;
        run(b, (SimTime)frame_us * 4 * US);
        snprintf(text, sizeof text, "%s: single device: 0xF1 at pointer 39 sets column 39 rows 0-3, column 0 stays clear", part);
        check(lights_match(b, &G14, &expect, frame_us, diff, sizeof diff), text);
        close_bench(b);
    }
}

/* The frame time: the slot of BP0 starts once per model frame, which is half the datasheet's frame
   period Tfr = 24 / fclk (a full AC cycle). */
static void frame_time(const char *part, unsigned tfr_us) {
    Bench *b = glass_bench(part, &G14);
    unsigned data[1] = {0xFF};
    unsigned t, starts = 0, first = 0, last = 0, period = 0;
    LogicValue prev = LOGIC_Z;
    char text[200];

    run(b, 10 * US);
    load(b, M14, 0, data, 1);
    run(b, (SimTime)tfr_us * 2 * US);
    for (t = 0; t < tfr_us * 8; t += 5) {
        LogicValue v = get(b, "BP0");

        if (prev == LOGIC_Z && v != LOGIC_Z) {
            if (starts++ == 0) {
                first = t;
            }
            last = t;
        }
        prev = v;
        run(b, 5 * US);
    }
    if (starts > 1) {
        period = (last - first) / (starts - 1);
    }
    snprintf(text, sizeof text, "%s: BP0's slot recurs every %u us (half of Tfr = %u us), measured %u us", part, tfr_us / 2, tfr_us, period);
    check(starts >= 14 && period + 10 >= tfr_us / 2 && period <= tfr_us / 2 + 10, text);
    close_bench(b);
}

/* ---- The loader: the run-time mode, "slots", "outputBank" and "load" of the lcd driver, and the
   lcdload action ---------------------------------------------------------------------------------- */

/* A 2-backplane lcd driver (modes static and 1:2 chosen by M.M, mode value 0 = static, 1 = 1:2). `commands`
   is the protocol's command list, `display` the display section (single quotes for double quotes). */

#define LD_MODE "{'field':'M.M','values':{'0':'static','1':'1:2'}}"
#define LD_SLOTS "'slots':{'static':[['BP0','BP1']],'1:2':[['BP0'],['BP1']]}"
#define LD_LOAD "'load':{'pointer':'P','step':{'static':8,'1:2':4}}"
#define LD_CMD "{'name':'W','in':'1xxxxxxx','action':'lcdload'}"

static char *quoted(const char *text) {
    char *copy = malloc(strlen(text) + 1);
    size_t i;

    for (i = 0; text[i] != '\0'; i++) {
        copy[i] = text[i] == '\'' ? '"' : text[i];
    }
    copy[i] = '\0';
    return copy;
}

static void loader_case(const char *id, const char *commands, const char *display, const char *want, const char *what) {
    char text[6000];
    char *copy, *error = NULL;
    size_t before = part_library_count(library);
    int result;

    snprintf(text, sizeof text,
             "{'format':'schematical-parts','version':1,'kind':'lcddriver','devices':[{'id':'%s','function':'x',"
             "'description':'d','drawer':['Displays & Indicators','LCD drivers'],"
             "'pinouts':{'p':[{'number':1,'name':'SDA','role':'inout'},{'number':2,'name':'SCL','role':'input'},"
             "{'number':3,'name':'VCC','role':'power'},{'number':4,'name':'GND','role':'ground'},"
             "{'number':5,'name':'BP0','role':'output'},{'number':6,'name':'BP1','role':'output'},"
             "{'number':7,'name':'S0','role':'output'},{'number':8,'name':'S1','role':'output'}]},"
             "'vcc':[2.5,5.5],'bus':{'style':'i2c','scl':'SCL','sda':'SDA','address':[],'table':[56]},"
             "'registers':[{'name':'M','width':2,'fields':[{'name':'M','bits':[1,0]}]},{'name':'P','width':6},"
             "{'name':'B','width':1},{'name':'S','width':2},{'name':'H','width':2}],"
             "'protocol':{'frame':'stream','commands':[%s]},'display':%s,"
             "'parts':[{'name':'%s-X','package':'SOIC','pinout':'p','footprint':'SOIC-8'}]}]}",
             id, commands, display, id);
    copy = quoted(text);
    result = load_parts(library, copy, "fixture.json", &error);
    if (want == NULL) {
        if (result != 0) {
            printf("  got: %s\n", error != NULL ? error : "?");
        }
        check(result == 0 && part_library_count(library) == before + 1, what);
    } else {
        if (result == 0 || error == NULL || strstr(error, want) == NULL) {
            printf("  got:  %s\n  want: %s\n", error != NULL ? error : "(loaded)", want);
        }
        check(result != 0 && error != NULL && strstr(error, want) != NULL && part_library_count(library) == before, what);
    }
    free(error);
    free(copy);
}

#define LD_BASE "{'driver':'lcd','mode':" LD_MODE ",'backplanes':['BP0','BP1']," LD_SLOTS ",'segmentPins':['S0','S1']," \
                "'ram':{'bytes':1,'address':0},'frameUs':1000"

static void loader(void) {
    loader_case("LDOK", LD_CMD, LD_BASE "," LD_LOAD "}", NULL, "a run-time mode with slots and a load section loads");
    loader_case("LD1", LD_CMD, "{'driver':'lcd','mode':" LD_MODE ",'backplanes':['BP0','BP1'],'segmentPins':['S0','S1'],"
                "'ram':{'bytes':1,'address':0},'frameUs':1000}", "needs \"slots\"", "a mode field needs \"slots\"");
    loader_case("LD2", LD_CMD, "{'driver':'lcd','mode':" LD_MODE ",'backplanes':['BP0','BP1'],'slots':{'static':[['BP0','BP9']],"
                "'1:2':[['BP0'],['BP1']]},'segmentPins':['S0','S1'],'ram':{'bytes':1,'address':0},'frameUs':1000}",
                "is not one of \"backplanes\"", "a slot names a pin that is not a backplane");
    loader_case("LD3", LD_CMD, "{'driver':'lcd','mode':" LD_MODE ",'backplanes':['BP0','BP1'],'slots':{'static':[['BP0'],['BP1']],"
                "'1:2':[['BP0'],['BP1']]},'segmentPins':['S0','S1'],'ram':{'bytes':1,'address':0},'frameUs':1000}",
                "is an array of 1 slots", "a mode's slot count must match the mode (static has 1)");
    loader_case("LD4", LD_CMD, "{'driver':'lcd','mode':{'field':'M.M','values':{'0':'1:4'}},'backplanes':['BP0','BP1'],"
                "'slots':{'1:4':[['BP0'],['BP1'],['BP0'],['BP1']]},'segmentPins':['S0','S1'],'ram':{'bytes':1,'address':0},"
                "'frameUs':1000}", "needs 4 backplane pins", "a mode needs as many backplane pins as slots");
    loader_case("LD5", LD_CMD, LD_BASE "," LD_LOAD ",'ramOrder':'backplane'}", "needs \"ramOrder\" \"segment\"",
                "a run-time mode needs segment-major RAM");
    loader_case("LD6", LD_CMD, "{'driver':'lcd','mode':'1:2','backplanes':['BP0','BP1'],'segmentPins':['S0','S1'],"
                "'ram':{'bytes':1,'address':0},'frameUs':1000," LD_LOAD "}", "need \"mode\" as a field",
                "\"load\" needs a mode field");
    loader_case("LD7", LD_CMD, LD_BASE ",'load':{'pointer':'P','step':{'static':8}}}", "no entry for the mode with 2 slots",
                "\"step\" must name every mode");
    loader_case("LD8", LD_CMD, LD_BASE ",'load':{'pointer':'P','step':{'static':8,'1:2':0}}}", "1 to 64",
                "\"step\" is 1 to 64");
    loader_case("LD9", LD_CMD, LD_BASE ",'load':{'pointer':'P','step':{'static':8,'1:2':4},'subaddress':'S'}}",
                "go together", "\"subaddress\" needs \"hardware\"");
    loader_case("LD10", LD_CMD, LD_BASE ",'load':{'pointer':'P','step':{'static':8,'1:2':4},'subaddress':'S','hardware':'H',"
                "'subaddresses':1}}", "2 to 256", "\"subaddresses\" is 2 to 256");
    loader_case("LD11", LD_CMD, LD_BASE ",'load':{'pointer':'NOPE','step':{'static':8,'1:2':4}}}", "NOPE",
                "\"pointer\" names a register");
    loader_case("LD12", LD_CMD, LD_BASE "}", "\"lcdload\" needs an lcd \"display\" with a \"load\" section",
                "lcdload needs a load section");
    loader_case("LD13", LD_CMD, "{'driver':'lcd','mode':'1:2','backplanes':['BP0','BP1'],'segmentPins':['S0','S1'],"
                "'ram':{'bytes':1,'address':0},'frameUs':1000,'outputBank':'B'}", "need \"mode\" as a field",
                "\"outputBank\" needs a mode field");
    loader_case("LD14", LD_CMD, LD_BASE ",'outputBank':'B'," LD_LOAD "}", NULL, "\"outputBank\" with a field loads");
}

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL loading the parts\n");
        return 1;
    }
    pinouts();
    loader();
    address();
    poweron("PCF8576DT/2,518", 6487);
    suite("PCF8576DT/2,518", 6487);
    poweron("PCA85176H/Q900/1,5", 6487);
    suite("PCF85176T/1Y", 6487);
    suite("PCA85176H/Q900/1,5", 6487);
    frame_time("PCF8576DT/2,518", 12974);
    frame_time("PCA85176H/Q900/1,5", 12974);
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("pcf8576_family_test: all passed\n");
    return 0;
}
