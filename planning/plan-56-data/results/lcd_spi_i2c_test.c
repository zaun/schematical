/*
 * lcd_spi_i2c_test (plan-56-E): the I2C / SPI LCD segment drivers PCF8553, PCA8553 (NXP 40 x 4, I2C or SPI),
 * BU91795 (ROHM 12 x 4, I2C) and BU9796AFS / BU9796AMUV (ROHM 20 x 4 and 12 x 4, I2C), each through its
 * real bus, wired to the bundled GENERIC-LCD glass: the lit cells equal the display RAM bits.
 *
 * Datasheets: datasheets/lcd-drivers/pcf8553.pdf, pca8553.pdf, bu91795muf-m-e.pdf, bu9796axxx-e.pdf
 * (page numbers are PDF pages). Covered: the pinouts, the I2C address (PCF/PCA 0 1 1 1 0 A1 A0 for the four
 * strap combinations, ROHM 0111 110), the SPI port of the PCF/PCA (IFS = 1, 3-wire SDIO read-back), every
 * register and command the models implement (Software_reset, Device_ctrl, Display_ctrl_1 and 2 with their
 * unused bits; ICSET, DISCTL, ADSET, MODESET, BLKCTL, APCTL with C = 1 and C = 0), RAM addressing and
 * auto-increment, display enable / power-down, the reset pin, the RAM layout against 1:4, 1:3, 1:2 and
 * static glass, and the backplane waveform (one backplane driven at a time, low and high).
 *
 * Not modeled (and so not tested): the multiplex mode field (the `lcd` driver always scans four backplanes:
 * a 1:3, 1:2 or static glass wired to COM0.. shows the RAM bits of its backplanes), the frame frequency,
 * blinking, bias, inversion, BOOST, the clock pins and the oscillator select, all-pixel-on (BU chips), the
 * RAM address wrap 13h -> 00h of the BU chips, RAM data that follows a non-ADSET command with C = 0 (dropped),
 * and floating A0 / A1 straps (the engine has no tie for an open pin: the sheet says internal pull-downs).
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

/* ---- The bench: the chip, an optional glass, a test driver on every chip pin, weak pull-ups --------- */

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
    SimInstance *chip, *glass, *drv, *pu;
    const Part *part, *glass_part;
    PinCount pins, pulls;
    const char *sda, *scl;
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

/* Chip pin that glass pin `name` (the glass's own pin index `seg` among its segment pins) is wired to:
   BP1..BP4 and COM to COM0..COM3, the segment pins in order to SEG0, SEG1, ... NULL for NC. */
static const char *glass_to_chip(const Bench *b, const char *name, int *seg, char *buf, size_t cap) {
    if (strcmp(name, "NC") == 0) {
        return NULL;
    }
    if (strcmp(name, "COM") == 0) {
        return "COM0";
    }
    if (strncmp(name, "BP", 2) == 0) {
        snprintf(buf, cap, "COM%d", name[2] - '1');
        return buf;
    }
    snprintf(buf, cap, "SEG%d", (*seg)++);
    return pin_of(b->part, buf) == SIZE_MAX ? NULL : buf;
}

/* `pulls`: NULL-terminated pin names with a weak pull-up. `glass` (or NULL): a glass part wired to the
   chip's COM and SEG pins. */
static Bench *bench(const char *part, const char *glass, const char *const *pulls, const char *sda,
                    const char *scl) {
    Bench *b = calloc(1, sizeof *b);
    SimNet **nets;
    size_t i, k;

    b->part = part_library_find(library, part);
    if (b->part == NULL) {
        fprintf(stderr, "no part %s\n", part);
        exit(2);
    }
    b->sda = sda;
    b->scl = scl;
    b->pins.count = b->part->pin_count;
    for (k = 0; pulls != NULL && pulls[k] != NULL; k++) {
        b->pulls.count++;
    }
    b->sim = sim_create();
    b->chip = sim_add_instance(b->sim, kind_model(b->part), "U1", b->part);
    b->drv = sim_add_instance(b->sim, &DRIVER, "TB", &b->pins);
    b->pu = sim_add_instance(b->sim, &PULLUP, "PU", &b->pulls);
    nets = calloc(b->part->pin_count, sizeof *nets);
    for (i = 0; i < b->part->pin_count; i++) {
        nets[i] = sim_add_net(b->sim, b->part->pins[i].name);
        sim_connect(b->sim, nets[i], b->chip, i);
        sim_connect(b->sim, nets[i], b->drv, i);
        for (k = 0; pulls != NULL && pulls[k] != NULL; k++) {
            if (strcmp(pulls[k], b->part->pins[i].name) == 0) {
                sim_connect(b->sim, nets[i], b->pu, k);
            }
        }
    }
    if (glass != NULL) {
        int seg = 0;

        b->glass_part = part_library_find(library, glass);
        if (b->glass_part == NULL) {
            fprintf(stderr, "no glass %s\n", glass);
            exit(2);
        }
        b->glass = sim_add_instance(b->sim, kind_model(b->glass_part), "G1", b->glass_part);
        for (i = 0; i < b->glass_part->pin_count; i++) {
            char buf[16];
            const char *chip = glass_to_chip(b, b->glass_part->pins[i].name, &seg, buf, sizeof buf);

            if (chip != NULL) {
                sim_connect(b->sim, nets[bpin(b, chip)], b->glass, i);
            }
        }
    }
    free(nets);
    return b;
}

static void start(Bench *b) {
    if (!sim_start(b->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(b->sim));
        exit(2);
    }
}

static void close_bench(Bench *b) {
    sim_destroy(b->sim);
    free(b);
}

/* A register of the chip by name (internal ones too), -1 when there is none. */
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

/* ---- I2C ------------------------------------------------------------------------------------------ */

static void line(Bench *b, const char *pin, int level) {
    set(b, pin, level ? LOGIC_Z : LOGIC_0);
}

static LogicValue i2c_bit(Bench *b, int out) {
    LogicValue v;

    line(b, b->sda, out);
    run(b, HALF);
    line(b, b->scl, 1);
    run(b, HALF);
    v = get(b, b->sda);
    line(b, b->scl, 0);
    run(b, HALF);
    return v;
}

static void i2c_start(Bench *b) {
    line(b, b->sda, 1);
    line(b, b->scl, 1);
    run(b, HALF);
    line(b, b->sda, 0);
    run(b, HALF);
    line(b, b->scl, 0);
    run(b, HALF);
}

static void i2c_stop(Bench *b) {
    line(b, b->sda, 0);
    run(b, HALF);
    line(b, b->scl, 1);
    run(b, HALF);
    line(b, b->sda, 1);
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

/* START, 7-bit address + write, the bytes, STOP. Returns whether the address was acknowledged. */
static int i2c_write(Bench *b, unsigned addr, const unsigned *v, size_t n) {
    size_t i;
    int ack;

    i2c_start(b);
    ack = i2c_byte(b, addr << 1);
    for (i = 0; ack && i < n; i++) {
        i2c_byte(b, v[i]);
    }
    i2c_stop(b);
    return ack;
}

/* PCF8553: pointer write, then a read of n bytes from it. */
static void pcf_i2c_read(Bench *b, unsigned addr, unsigned ap, unsigned *out, size_t n) {
    size_t i;

    i2c_start(b);
    i2c_byte(b, addr << 1);
    i2c_byte(b, ap);
    i2c_stop(b);
    i2c_start(b);
    i2c_byte(b, (addr << 1) | 1u);
    for (i = 0; i < n; i++) {
        out[i] = i2c_in(b, i + 1 < n);
    }
    i2c_stop(b);
}

/* ---- SPI (PCF8553: CE = SDA/CE, SCL, 3-wire SDIO, mode 0) -------------------------------------------- */

static void spi_idle(Bench *b) {
    set(b, "SDA/CE", LOGIC_1);
    set(b, "SCL", LOGIC_0);
    set(b, "SDIO", LOGIC_Z);
    run(b, HALF);
}

static void spi_out(Bench *b, unsigned v) {
    int i;

    for (i = 7; i >= 0; i--) {
        set(b, "SDIO", ((v >> i) & 1u) ? LOGIC_1 : LOGIC_0);
        run(b, HALF);
        set(b, "SCL", LOGIC_1);
        run(b, HALF);
        set(b, "SCL", LOGIC_0);
        run(b, HALF);
    }
}

/* The read address byte: SDIO is released after the last rising edge, before the chip drives it (p.38,
   tt(SDI-SDO) = 0 ns). */
static void spi_out_release(Bench *b, unsigned v) {
    int i;

    for (i = 7; i >= 1; i--) {
        set(b, "SDIO", ((v >> i) & 1u) ? LOGIC_1 : LOGIC_0);
        run(b, HALF);
        set(b, "SCL", LOGIC_1);
        run(b, HALF);
        set(b, "SCL", LOGIC_0);
        run(b, HALF);
    }
    set(b, "SDIO", (v & 1u) ? LOGIC_1 : LOGIC_0);
    run(b, HALF);
    set(b, "SCL", LOGIC_1);
    run(b, HALF);
    set(b, "SDIO", LOGIC_Z);
    run(b, HALF);
    set(b, "SCL", LOGIC_0);
    run(b, HALF);
}

static unsigned spi_in(Bench *b) {
    unsigned v = 0;
    int i;

    set(b, "SDIO", LOGIC_Z);
    for (i = 0; i < 8; i++) {
        run(b, HALF);
        set(b, "SCL", LOGIC_1);
        run(b, HALF);
        v = (v << 1) | (get(b, "SDIO") == LOGIC_1 ? 1u : 0u);
        set(b, "SCL", LOGIC_0);
    }
    return v;
}

/* CE low, R/W = 0 | AP, the data bytes, CE high. */
static void spi_write(Bench *b, unsigned ap, const unsigned *v, size_t n) {
    size_t i;

    set(b, "SDA/CE", LOGIC_0);
    run(b, HALF);
    spi_out(b, ap);
    for (i = 0; i < n; i++) {
        spi_out(b, v[i]);
    }
    set(b, "SDIO", LOGIC_Z);
    run(b, HALF);
    set(b, "SDA/CE", LOGIC_1);
    run(b, HALF);
}

static void spi_read(Bench *b, unsigned ap, unsigned *out, size_t n) {
    size_t i;

    set(b, "SDA/CE", LOGIC_0);
    run(b, HALF);
    spi_out_release(b, 0x80u | ap);
    for (i = 0; i < n; i++) {
        out[i] = spi_in(b);
    }
    run(b, HALF);
    set(b, "SDA/CE", LOGIC_1);
    run(b, HALF);
}

/* ---- The glass ----------------------------------------------------------------------------------- */

static size_t glass_lights(Bench *b, SimTime since, char *out, size_t cap) {
    const SimComponentType *type = sim_instance_type(b->glass);
    unsigned char values[200];
    size_t n = type->lights(b->sim, b->glass, sim_instance_state(b->glass), since, values, sizeof values);
    size_t i;

    for (i = 0; i < n && i + 1 < cap; i++) {
        out[i] = values[i] ? '1' : '0';
    }
    out[i] = '\0';
    return n;
}

/* The display RAM as the chip shows it: ram[segment][backplane]. */
typedef struct {
    unsigned char bit[40][4];
    int segments;
} Picture;

static void expect_lights(const Picture *p, const char *glass, char *want) {
    size_t cells, i;

    if (strcmp(glass, "GENERIC-LCD-8D-1:4") == 0) {
        cells = 64;
    } else if (strcmp(glass, "GENERIC-LCD-8D-1:3") == 0) {
        cells = 64;
    } else if (strcmp(glass, "GENERIC-LCD-4D-1:2") == 0) {
        cells = 32;
    } else {
        cells = 32;
    }
    for (i = 0; i < cells; i++) {
        size_t g, k;

        if (strcmp(glass, "GENERIC-LCD-8D-1:4") == 0) {
            g = i / 4;
            k = i % 4;
        } else if (strcmp(glass, "GENERIC-LCD-8D-1:3") == 0) {
            g = 3 * (i / 8) + (i % 8) / 3;
            k = (i % 8) % 3;
        } else if (strcmp(glass, "GENERIC-LCD-4D-1:2") == 0) {
            g = i / 2;
            k = i % 2;
        } else {
            g = i;
            k = 0;
        }
        want[i] = (g < (size_t)p->segments && p->bit[g][k]) ? '1' : '0';
    }
    want[cells] = '\0';
}

/* Does the glass show `p` over the last two frames? Prints both strings when not. */
static int shows(Bench *b, const Picture *p, const char *glass, unsigned frame_us) {
    char lights[200], want[200];
    SimTime window = (SimTime)frame_us * 2 * US;

    run(b, (SimTime)frame_us * 4 * US);
    glass_lights(b, sim_now(b->sim) - window, lights, sizeof lights);
    expect_lights(p, glass, want);
    if (strcmp(lights, want) != 0) {
        printf("  lit  %s\n  want %s\n", lights, want);
        return 0;
    }
    return 1;
}

static int dark(Bench *b, const char *glass, unsigned frame_us) {
    char lights[200];
    SimTime window = (SimTime)frame_us * 2 * US;

    (void)glass;
    run(b, (SimTime)frame_us * 4 * US);
    glass_lights(b, sim_now(b->sim) - window, lights, sizeof lights);
    return strchr(lights, '1') == NULL;
}

/* Over `us` microseconds sampled every `step`: which backplanes were driven low and high, and was more
   than one ever driven at once. */
static void waveform(Bench *b, size_t count, unsigned us, unsigned step, unsigned *low, unsigned *high,
                     int *overlap) {
    unsigned t;

    *low = *high = 0;
    *overlap = 0;
    for (t = 0; t < us; t += step) {
        unsigned driven = 0;
        size_t i;

        for (i = 0; i < count; i++) {
            char name[8];
            LogicValue v;

            snprintf(name, sizeof name, "COM%zu", i);
            v = get(b, name);
            if (v == LOGIC_0 || v == LOGIC_1) {
                driven++;
                if (v == LOGIC_0) {
                    *low |= 1u << i;
                } else {
                    *high |= 1u << i;
                }
            }
        }
        if (driven > 1) {
            *overlap = 1;
        }
        run(b, step * US);
    }
}

static int all_released(Bench *b, int segs, int bps) {
    int i, ok = 1;

    for (i = 0; i < segs; i++) {
        char name[12];

        snprintf(name, sizeof name, "SEG%d", i);
        ok &= get(b, name) == LOGIC_Z;
    }
    for (i = 0; i < bps; i++) {
        char name[12];

        snprintf(name, sizeof name, "COM%d", i);
        ok &= get(b, name) == LOGIC_Z;
    }
    return ok;
}

/* ---- PCF8553 / PCA8553 -------------------------------------------------------------------------------- */

enum { PCF_FRAME = 15625 };

static void pcf_picture(Picture *p, const unsigned *ram) {
    int g, k;

    memset(p, 0, sizeof *p);
    p->segments = 40;
    for (g = 0; g < 40; g++) {
        for (k = 0; k < 4; k++) {
            p->bit[g][k] = (unsigned char)((ram[5 * k + g / 8] >> (g % 8)) & 1u);
        }
    }
}

static const char *const TSSOP56_NAMES[56] = {
    "SEG34", "SEG35", "SEG36", "SEG37", "SEG38", "SEG39", "COM0", "COM1", "COM2", "COM3", "VLCD", "VDD", "IFS", "VSS",
    "RST", "SDIO", "A0", "CLK", "SCL", "SDA/CE", "PORE", "A1", "SEG0", "SEG1", "SEG2", "SEG3", "SEG4", "SEG5",
    "SEG6", "SEG7", "SEG8", "SEG9", "SEG10", "SEG11", "SEG12", "SEG13", "SEG14", "SEG15", "SEG16", "SEG17", "SEG18",
    "SEG19", "SEG20", "SEG21", "SEG22", "SEG23", "SEG24", "SEG25", "SEG26", "SEG27", "SEG28", "SEG29", "SEG30",
    "SEG31", "SEG32", "SEG33"};

/* The pin names of Figure 2 (pcf8553 p.4, pca8553 p.4) and the supply and bus roles of Table 3 (p.5). */
static void pcf_pinout(const char *chip) {
    const Part *p = part_library_find(library, chip);
    size_t i;
    int ok = p != NULL && p->pin_count == 56;

    for (i = 0; ok && i < 56; i++) {
        ok = strcmp(p->pins[i].name, TSSOP56_NAMES[i]) == 0;
    }
    check(ok, "TSSOP56 pin names follow Figure 2");
    check(ok && p->pins[10].role == PIN_POWER && p->pins[11].role == PIN_POWER && p->pins[13].role == PIN_GROUND &&
              p->pins[15].role == PIN_INOUT && p->pins[19].role == PIN_INOUT && p->pins[6].role == PIN_OUTPUT &&
              p->pins[22].role == PIN_OUTPUT && p->pins[14].role == PIN_INPUT,
          "VLCD and VDD are supplies, SDIO and SDA/CE bidirectional, COM and SEG outputs, RST an input");
}

static void pcf_i2c(const char *chip, int pca) {
    static const char *const PULLS[] = {"SDA/CE", "SCL", NULL};
    static const unsigned ADDR[4] = {0x38, 0x39, 0x3A, 0x3B};
    Bench *b;
    unsigned ram[20], r[24], w[32];
    Picture pic;
    char text[200];
    unsigned dev_mask = pca ? 0x1Fu : 0x0Fu;
    unsigned low, high;
    int overlap, i, a0, a1;

    snprintf(text, sizeof text, "%s", chip);
    b = bench(chip, "GENERIC-LCD-8D-1:4", PULLS, "SDA/CE", "SCL");
    start(b);
    set(b, "IFS", LOGIC_0);
    set(b, "A0", LOGIC_0);
    set(b, "A1", LOGIC_0);
    run(b, 10 * US);
    /* power-on: registers at reset values, display off (p.10) */
    pcf_i2c_read(b, 0x38, 0, r, 24);
    {
        int ok = r[0] == 0 && r[1] == 0x04 && r[2] == 0 && r[3] == 0;

        for (i = 4; i < 24; i++) {
            ok &= r[i] == 0;
        }
        snprintf(text, sizeof text, "%s I2C: reset values read back: Device_ctrl 04h (64 Hz, oscillator on), the rest 0", chip);
        check(ok, text);
    }
    check(all_released(b, 40, 4) && dark(b, "GENERIC-LCD-8D-1:4", PCF_FRAME),
          "power-down after reset: every backplane and segment pin released, the glass dark (DE = 0)");
    /* writes to RAM while the display is disabled show nothing */
    for (i = 0; i < 20; i++) {
        ram[i] = (unsigned)((i * 37 + 11) & 0xFF);
    }
    w[0] = 0x04;
    memcpy(w + 1, ram, sizeof ram);
    check(i2c_write(b, 0x38, w, 21) != 0, "I2C: the pointer and 20 RAM bytes are acknowledged");
    check(dark(b, "GENERIC-LCD-8D-1:4", PCF_FRAME), "RAM written with DE = 0: the glass stays dark");
    pcf_i2c_read(b, 0x38, 4, r, 20);
    check(memcmp(r, ram, sizeof ram) == 0, "I2C: the 20 RAM bytes read back from 04h with auto-increment");
    /* Display_ctrl_1: DE = 1 */
    w[0] = 0x02;
    w[1] = 0x01;
    i2c_write(b, 0x38, w, 2);
    pcf_picture(&pic, ram);
    check(shows(b, &pic, "GENERIC-LCD-8D-1:4", PCF_FRAME),
          "DE = 1: the 1:4 glass shows the RAM: COM0 04h-08h ... COM3 13h-17h, SEG0 the LSB of the first byte of a row");
    waveform(b, 4, PCF_FRAME * 3u, 100, &low, &high, &overlap);
    check(low == 15 && high == 15 && !overlap, "every backplane is driven low and high over two frames, one at a time");
    /* a second picture replaces the first; single byte write */
    w[0] = 0x09;
    w[1] = 0xFF;
    w[2] = 0x00;
    i2c_write(b, 0x38, w, 3);
    ram[5] = 0xFF;
    ram[6] = 0x00;
    pcf_picture(&pic, ram);
    check(shows(b, &pic, "GENERIC-LCD-8D-1:4", PCF_FRAME), "a two-byte write at 09h changes COM1 SEG0-15 only");
    /* register bits: unused bits read 0 */
    w[0] = 0x01;
    w[1] = 0xFF;
    w[2] = 0xFF;
    w[3] = 0xFF;
    i2c_write(b, 0x38, w, 4);
    pcf_i2c_read(b, 0x38, 1, r, 3);
    snprintf(text, sizeof text, "%s: bits labeled 0 in Device_ctrl, Display_ctrl_1 and 2 read back 0 (%02X %02X %02X)", chip,
             dev_mask | (0u << 5), 0x1Fu, 0x07u);
    check(r[0] == dev_mask && r[1] == 0x1F && r[2] == 0x07, text);
    check(dark(b, "GENERIC-LCD-8D-1:4", PCF_FRAME) == 0, "DE stayed 1 across the register writes");
    /* DE = 0 */
    w[0] = 0x02;
    w[1] = 0x1E;
    i2c_write(b, 0x38, w, 2);
    check(all_released(b, 40, 4) && dark(b, "GENERIC-LCD-8D-1:4", PCF_FRAME),
          "DE = 0: every pin released and the glass dark; MUX, B and BOOST are only stored");
    w[0] = 0x02;
    w[1] = 0x01;
    i2c_write(b, 0x38, w, 2);
    check(shows(b, &pic, "GENERIC-LCD-8D-1:4", PCF_FRAME), "DE = 1 again: the picture is back (RAM kept)");
    /* the pointer stops at 17h: the extra bytes are ignored */
    w[0] = 0x16;
    w[1] = 0xA5;
    w[2] = 0x5A;
    w[3] = 0x11;
    w[4] = 0x22;
    i2c_write(b, 0x38, w, 5);
    pcf_i2c_read(b, 0x38, 0x16, r, 2);
    check(r[0] == 0xA5 && r[1] == 0x5A, "writes past 17h are ignored (the pointer stops at 18h)");
    ram[18] = 0xA5;
    ram[19] = 0x5A;
    /* a pointer outside 00h-17h: the transfer is ignored but acknowledged */
    w[0] = 0x1C;
    w[1] = 0xEE;
    check(i2c_write(b, 0x38, w, 2) != 0, "a pointer of 1Ch is acknowledged");
    pcf_i2c_read(b, 0x38, 0x04, r, 20);
    check(memcmp(r, ram, 20) == 0, "... and changes nothing");
    /* software reset: only 0010 1100 resets */
    w[0] = 0x00;
    w[1] = 0x55;
    i2c_write(b, 0x38, w, 2);
    pcf_i2c_read(b, 0x38, 0x01, r, 1);
    check(r[0] == dev_mask, "Software_reset with a value other than 2Ch does nothing");
    w[0] = 0x00;
    w[1] = 0x2C;
    i2c_write(b, 0x38, w, 2);
    pcf_i2c_read(b, 0x38, 0, r, 24);
    {
        int ok = r[1] == 0x04 && r[2] == 0 && r[3] == 0;

        for (i = 4; i < 24; i++) {
            ok &= r[i] == 0;
        }
        check(ok && all_released(b, 40, 4), "Software_reset 2Ch: registers and RAM back to reset, display off");
    }
    /* the hardware reset pin */
    w[0] = 0x04;
    w[1] = 0x81;
    w[2] = 0x42;
    i2c_write(b, 0x38, w, 3);
    w[0] = 0x02;
    w[1] = 0x01;
    i2c_write(b, 0x38, w, 2);
    set(b, "RST", LOGIC_0);
    run(b, 20 * US);
    set(b, "RST", LOGIC_Z);
    run(b, 20 * US);
    pcf_i2c_read(b, 0x38, 0x02, r, 3);
    check(r[0] == 0 && all_released(b, 40, 4), "RST low: Display_ctrl_1 back to 0 (display disabled), outputs released");
    pcf_i2c_read(b, 0x38, 0x04, r, 2);
    check(r[0] == 0 && r[1] == 0, "RST low clears the display RAM");
    close_bench(b);

    /* the four slave addresses 0 1 1 1 0 A1 A0 (Table 12, p.28) */
    for (a1 = 0; a1 < 2; a1++) {
        for (a0 = 0; a0 < 2; a0++) {
            unsigned want = ADDR[a1 * 2 + a0];
            int ok = 1;

            b = bench(chip, NULL, PULLS, "SDA/CE", "SCL");
            start(b);
            set(b, "IFS", LOGIC_0);
            set(b, "A0", a0 ? LOGIC_1 : LOGIC_0);
            set(b, "A1", a1 ? LOGIC_1 : LOGIC_0);
            run(b, 10 * US);
            w[0] = 0x00;
            ok &= i2c_write(b, want, w, 1) != 0;
            for (i = 0; i < 4; i++) {
                if (ADDR[i] != want) {
                    ok &= i2c_write(b, ADDR[i], w, 1) == 0;
                }
            }
            ok &= i2c_write(b, 0x3C, w, 1) == 0 && i2c_write(b, 0x30, w, 1) == 0;
            snprintf(text, sizeof text, "%s: A1 = %d, A0 = %d answers 0x%02X and no other address", chip, a1, a0, want);
            check(ok, text);
            close_bench(b);
        }
    }
}

/* One glass wired to COM0.. and SEG0..: the lit cells equal the RAM bits of its backplanes, whatever the
   MUX field says. */
static void pcf_glass(const char *chip, const char *glass, unsigned mux, int bps) {
    static const char *const PULLS[] = {"SDA/CE", "SCL", NULL};
    Bench *b = bench(chip, glass, PULLS, "SDA/CE", "SCL");
    unsigned ram[20], w[24];
    Picture pic;
    char text[200];
    int i;
    unsigned low, high;
    int overlap;

    start(b);
    set(b, "IFS", LOGIC_0);
    set(b, "A0", LOGIC_0);
    set(b, "A1", LOGIC_0);
    run(b, 10 * US);
    for (i = 0; i < 20; i++) {
        ram[i] = (unsigned)((i * 73 + 29) & 0xFF);
    }
    w[0] = 0x02;
    w[1] = (mux << 2) | 1u;
    i2c_write(b, 0x38, w, 2);
    w[0] = 0x04;
    memcpy(w + 1, ram, sizeof ram);
    i2c_write(b, 0x38, w, 21);
    pcf_picture(&pic, ram);
    {
        /* backplanes the glass does not use read as 0 in the expected picture */
        int g, k;

        for (g = 0; g < 40; g++) {
            for (k = bps; k < 4; k++) {
                pic.bit[g][k] = 0;
            }
        }
    }
    snprintf(text, sizeof text, "%s: MUX = %u, %s wired to COM0-COM%d: the lit cells equal the RAM bits of those backplanes", chip,
             mux, glass, bps - 1);
    check(shows(b, &pic, glass, PCF_FRAME), text);
    waveform(b, 4, PCF_FRAME * 3u, 100, &low, &high, &overlap);
    snprintf(text, sizeof text, "%s: the backplanes are driven one at a time, low and high", chip);
    check(low == 15 && high == 15 && !overlap, text);
    close_bench(b);
}

static void pcf_spi(const char *chip) {
    Bench *b = bench(chip, "GENERIC-LCD-8D-1:4", NULL, "SDA/CE", "SCL");
    unsigned ram[20], r[24], w[24], single;
    Picture pic;
    char text[200];
    int i, ok;

    start(b);
    set(b, "IFS", LOGIC_1);
    set(b, "SDA/CE", LOGIC_1);
    set(b, "SCL", LOGIC_0);
    run(b, 10 * US);
    spi_idle(b);
    spi_read(b, 0, r, 24);
    ok = r[0] == 0 && r[1] == 0x04 && r[2] == 0 && r[3] == 0;
    for (i = 4; i < 24; i++) {
        ok &= r[i] == 0;
    }
    snprintf(text, sizeof text, "%s SPI: IFS = 1 selects SPI; the registers read back through SDIO (reset values)", chip);
    check(ok, text);
    for (i = 0; i < 20; i++) {
        ram[i] = (unsigned)((i * 59 + 3) & 0xFF);
    }
    memcpy(w, ram, sizeof ram);
    spi_write(b, 0x04, w, 20);
    single = 0x01;
    spi_write(b, 0x02, &single, 1);
    pcf_picture(&pic, ram);
    check(shows(b, &pic, "GENERIC-LCD-8D-1:4", PCF_FRAME), "SPI: 20 RAM bytes from AP = 04h and DE = 1: the glass shows the RAM");
    spi_read(b, 0x04, r, 20);
    check(memcmp(r, ram, 20) == 0, "SPI: the RAM reads back from AP = 04h with auto-increment");
    spi_read(b, 0x09, r, 3);
    check(r[0] == ram[5] && r[1] == ram[6] && r[2] == ram[7], "SPI: a read starting at AP = 09h");
    /* R/W = 0, a 2-byte write at 0x03: Display_ctrl_2 and the first RAM byte */
    w[0] = 0x07;
    w[1] = 0xC3;
    spi_write(b, 0x03, w, 2);
    spi_read(b, 0x03, r, 2);
    ram[0] = 0xC3;
    check(r[0] == 0x07 && r[1] == 0xC3, "SPI: auto-increment from Display_ctrl_2 (03h) into the RAM (04h)");
    pcf_picture(&pic, ram);
    check(shows(b, &pic, "GENERIC-LCD-8D-1:4", PCF_FRAME), "SPI: the glass follows");
    /* software reset by SPI */
    single = 0x2C;
    spi_write(b, 0x00, &single, 1);
    spi_read(b, 0x00, r, 24);
    ok = r[1] == 0x04 && r[2] == 0 && r[3] == 0;
    for (i = 4; i < 24; i++) {
        ok &= r[i] == 0;
    }
    check(ok && all_released(b, 40, 4), "SPI: Software_reset 2Ch resets registers and RAM");
    /* an access with bits 6-5 not 00, or AP above 17h, changes nothing */
    single = 0x01;
    spi_write(b, 0x02, &single, 1);
    single = 0x77;
    spi_write(b, 0x20 | 0x04, &single, 1);
    spi_write(b, 0x1F, &single, 1);
    spi_read(b, 0x04, r, 1);
    check(r[0] == 0, "SPI: an address byte with bits 6-5 = 01 or AP = 1Fh writes nothing");
    close_bench(b);
    /* in SPI mode the I2C port is off */
    {
        static const char *const PULLS[] = {"SDA/CE", "SCL", NULL};

        b = bench(chip, NULL, PULLS, "SDA/CE", "SCL");
        start(b);
        set(b, "IFS", LOGIC_1);
        run(b, 10 * US);
        w[0] = 0x00;
        snprintf(text, sizeof text, "%s: IFS = 1: the I2C address 0x38 is not acknowledged", chip);
        check(i2c_write(b, 0x38, w, 1) == 0, text);
        close_bench(b);
    }
}

/* ---- ROHM BU91795 / BU9796A ----------------------------------------------------------------------------- */

enum { BU_FRAME = 12500 };

typedef struct {
    const char *part;
    int is91795;        /* 1: ADSET 4 bits at 04h-0Fh; 0: 5 bits at 00h-13h */
    int segments;       /* output pins SEG0.. */
    int first;          /* DDRAM address of SEG0 */
    int pins;
} BuChip;

/* The DDRAM as nibbles by address 0..0x13. */
typedef struct {
    unsigned char nib[0x14];
} Ddram;

static void bu_picture(const BuChip *c, const Ddram *d, Picture *p) {
    int g, k;

    memset(p, 0, sizeof *p);
    p->segments = c->segments;
    for (g = 0; g < c->segments; g++) {
        for (k = 0; k < 4; k++) {
            p->bit[g][k] = (unsigned char)((d->nib[c->first + g] >> (3 - k)) & 1u);
        }
    }
}

/* Writes data bytes at the DDRAM address: ADSET with C = 0, then the bytes; updates the model nibbles. */
static void bu_ram(Bench *b, Ddram *d, unsigned addr, const unsigned *bytes, size_t n) {
    unsigned w[40];
    size_t i;
    unsigned a = addr;

    w[0] = addr;   /* ADSET, C = 0 */
    for (i = 0; i < n; i++) {
        w[1 + i] = bytes[i];
        if (a < 0x14) {
            d->nib[a] = (unsigned char)(bytes[i] >> 4);
        }
        a++;
        if (a < 0x14) {
            d->nib[a] = (unsigned char)(bytes[i] & 0xFu);
        }
        a++;
    }
    i2c_write(b, 0x3E, w, n + 1);
}

static const char *const BU24_NAMES[24] = {"OSCIN", "SCL", "SDA", "SEG0", "SEG1", "SEG2", "SEG3", "SEG4", "SEG5", "SEG6",
                                           "SEG7", "SEG8", "SEG9", "SEG10", "SEG11", "COM0", "COM1", "COM2", "COM3", "VLCD",
                                           "VDD", "VSS", "TEST1", "TEST2"};

static void bu_pinout(const BuChip *c) {
    const Part *p = part_library_find(library, c->part);
    size_t i;
    int ok = p != NULL && p->pin_count == (size_t)c->pins;

    if (c->pins == 24) {
        for (i = 0; ok && i < 24; i++) {
            ok = strcmp(p->pins[i].name, BU24_NAMES[i]) == 0;
        }
    } else {
        /* BU9796AFS: SEG2..SEG19 on 1-18, COM0-3 19-22, VLCD 23, VDD 24, VSS 25, TEST1 26, TEST2 27, OSCIN 28,
           SCL 29, SDA 30, SEG0 31, SEG1 32 (p.2) */
        static const char *const N[32] = {"SEG2", "SEG3", "SEG4", "SEG5", "SEG6", "SEG7", "SEG8", "SEG9", "SEG10", "SEG11",
                                          "SEG12", "SEG13", "SEG14", "SEG15", "SEG16", "SEG17", "SEG18", "SEG19", "COM0",
                                          "COM1", "COM2", "COM3", "VLCD", "VDD", "VSS", "TEST1", "TEST2", "OSCIN", "SCL",
                                          "SDA", "SEG0", "SEG1"};

        for (i = 0; ok && i < 32; i++) {
            ok = strcmp(p->pins[i].name, N[i]) == 0;
        }
    }
    {
        char text[160];

        snprintf(text, sizeof text, "%s: pin names follow the pin configuration figure", c->part);
        check(ok, text);
    }
}

static int bu_ack(Bench *b, unsigned addr) {
    unsigned w[1] = {0xC0};

    return i2c_write(b, addr, w, 1);
}

static void bu_chip(const BuChip *c) {
    static const char *const PULLS[] = {"SDA", "SCL", NULL};
    Bench *b;
    Ddram d;
    Picture pic;
    char text[220];
    unsigned bytes[24], w[32];
    int i;
    unsigned low, high;
    int overlap;
    const char *glass = "GENERIC-LCD-8D-1:4";
    unsigned lo = c->is91795 ? 4u : 0u;     /* first ADSET address of SEG0 */

    bu_pinout(c);
    b = bench(c->part, glass, PULLS, "SDA", "SCL");
    memset(&d, 0, sizeof d);
    start(b);
    run(b, 10 * US);
    check(bu_ack(b, 0x3E) && !bu_ack(b, 0x3D) && !bu_ack(b, 0x3C) && !bu_ack(b, 0x3F),
          "slave address 0111 1100 (write) is acknowledged, its neighbours are not");
    snprintf(text, sizeof text, "%s: power-on: display off, every pin released", c->part);
    check(reg_value(b, "MODE") == 0 && all_released(b, c->segments, 4), text);
    /* the datasheet's start sequence: software reset, blink off, DISCTL, ICSET external clock, ADSET, data */
    w[0] = 0xEA;
    w[1] = 0xF0;
    w[2] = 0xA4;
    w[3] = 0xE9;
    check(i2c_write(b, 0x3E, w, 4) != 0, "ICSET reset, BLKCTL, DISCTL, ICSET are acknowledged in one transfer");
    check(reg_value(b, "ICSET") == 1 && reg_value(b, "BLKCTL") == 0 && reg_value(b, "DISCTL") == 0x04,
          "ICSET P0 = 1 (external clock), BLKCTL off, DISCTL 80 Hz + frame inversion + power save 1 are stored");
    /* RAM written while the display is off */
    for (i = 0; i < 6; i++) {
        bytes[i] = (unsigned)((i * 53 + 0x1D) & 0xFF);
    }
    bu_ram(b, &d, lo, bytes, 6);
    check(dark(b, glass, BU_FRAME), "RAM written with the display off: the glass stays dark");
    /* MODESET display on, as its own transfer */
    w[0] = 0xC8;
    i2c_write(b, 0x3E, w, 1);
    bu_picture(c, &d, &pic);
    snprintf(text, sizeof text, "%s: MODESET display on: the 1:4 glass shows SEG0-SEG%d: bit 3 = COM0 ... bit 0 = COM3 of each nibble", c->part, 5);
    check(shows(b, &pic, glass, BU_FRAME), text);
    waveform(b, 4, BU_FRAME * 3u, 50, &low, &high, &overlap);
    check(low == 15 && high == 15 && !overlap, "every backplane is driven low and high over two frames, one at a time");
    /* the rest of the RAM in one go, a full picture */
    for (i = 0; i < 12; i++) {
        bytes[i] = (unsigned)((i * 91 + 0x6B) & 0xFF);
    }
    bu_ram(b, &d, lo, bytes, (size_t)(c->segments + 1) / 2);
    bu_picture(c, &d, &pic);
    check(shows(b, &pic, glass, BU_FRAME), "a full picture: every segment nibble lands on its segment pin");
    /* commands and data in one transfer: MODESET (C = 1), then ADSET (C = 0) and data */
    for (i = 0; i < 4; i++) {
        bytes[i] = (unsigned)(0xA0 + i * 0x11);
    }
    w[0] = 0xC8;
    w[1] = lo + 2;
    memcpy(w + 2, bytes, 2 * sizeof w[0]);
    i2c_write(b, 0x3E, w, 4);
    d.nib[lo + 2] = (unsigned char)(bytes[0] >> 4);
    d.nib[lo + 3] = (unsigned char)(bytes[0] & 0xF);
    d.nib[lo + 4] = (unsigned char)(bytes[1] >> 4);
    d.nib[lo + 5] = (unsigned char)(bytes[1] & 0xF);
    bu_picture(c, &d, &pic);
    check(shows(b, &pic, glass, BU_FRAME), "MODESET (C = 1) then ADSET (C = 0) and two data bytes in one transfer: RAM from the address set");
    /* ADSET with C = 1 sets the address, a later ADSET with C = 0 takes the last address */
    w[0] = 0x80u | (lo + 6);
    w[1] = lo + 8;
    bytes[0] = 0x5A;
    w[2] = bytes[0];
    i2c_write(b, 0x3E, w, 3);
    d.nib[lo + 8] = 5;
    d.nib[lo + 9] = 0xA;
    bu_picture(c, &d, &pic);
    check(shows(b, &pic, glass, BU_FRAME), "ADSET (C = 1) then ADSET (C = 0): the last address wins, data from there");
    /* the display off command keeps the RAM */
    w[0] = 0xC0;
    i2c_write(b, 0x3E, w, 1);
    check(dark(b, glass, BU_FRAME) && all_released(b, c->segments, 4), "MODESET display off: the glass is dark, every pin released");
    w[0] = 0xC8;
    i2c_write(b, 0x3E, w, 1);
    check(shows(b, &pic, glass, BU_FRAME), "display on again: the RAM was kept");
    /* APCTL: all pixel off blanks the glass; all pixel on is not modeled */
    w[0] = 0xFD;
    i2c_write(b, 0x3E, w, 1);
    check(reg_value(b, "APCTL") == 1 && dark(b, glass, BU_FRAME), "APCTL P0 = 1 (all pixels off): the glass is dark");
    w[0] = 0xFF;
    i2c_write(b, 0x3E, w, 1);
    check(reg_value(b, "APCTL") == 3 && dark(b, glass, BU_FRAME), "APCTL P1 = P0 = 1: APOFF wins");
    w[0] = 0xFC;
    i2c_write(b, 0x3E, w, 1);
    check(shows(b, &pic, glass, BU_FRAME), "APCTL normal: the RAM is shown again");
    w[0] = 0xFE;
    i2c_write(b, 0x3E, w, 1);
    check(reg_value(b, "APCTL") == 2, "APCTL P1 = 1 (all pixels on) is stored (the driver cannot light every cell)");
    w[0] = 0xFC;
    i2c_write(b, 0x3E, w, 1);
    /* the command registers */
    w[0] = 0xBD;   /* DISCTL: P4P3 = 11, P2 = 1, P1P0 = 01 */
    i2c_write(b, 0x3E, w, 1);
    check(reg_value(b, "DISCTL") == 0x1D, "DISCTL P4-P0 = 11101 is stored");
    w[0] = 0xE8;
    i2c_write(b, 0x3E, w, 1);
    check(reg_value(b, "ICSET") == 0, "ICSET P0 = 0: internal oscillator");
    w[0] = 0xF3;
    i2c_write(b, 0x3E, w, 1);
    check(reg_value(b, "BLKCTL") == 3, "BLKCTL P1 P0 = 11 is stored");
    if (!c->is91795) {
        w[0] = 0xCC;   /* MODESET: display on, 1/2 bias */
        i2c_write(b, 0x3E, w, 1);
        check(reg_value(b, "MODE") == 3, "MODESET P3 = 1, P2 = 1: display on, 1/2 bias");
        w[0] = 0xC8;
        i2c_write(b, 0x3E, w, 1);
        check(reg_value(b, "MODE") == 1, "MODESET P2 = 0: 1/3 bias");
    }
    /* ADSET out of range: address 00000 */
    memset(&d, 0, sizeof d);
    memset(bytes, 0, sizeof bytes);
    bu_ram(b, &d, lo, bytes, (size_t)(c->segments + 1) / 2);
    w[0] = 0xEA;
    i2c_write(b, 0x3E, w, 1);
    w[0] = 0xC8;
    i2c_write(b, 0x3E, w, 1);
    bu_picture(c, &d, &pic);
    {
        long a, bad = c->is91795 ? 0x03 : 0x14;
        unsigned three[3] = {0xAB, 0xCD, 0x12};

        w[0] = (unsigned)bad;
        memcpy(w + 1, three, sizeof three);
        i2c_write(b, 0x3E, w, 4);
        a = reg_value(b, "DD00");
        snprintf(text, sizeof text, "%s: ADSET %02lX is out of range: the data starts at address 00h", c->part, bad);
        check(a == 0xA && reg_value(b, "DD01") == 0xB && reg_value(b, "DD02") == 0xC && reg_value(b, "DD03") == 0xD &&
                  reg_value(b, "DD04") == 0x1 && reg_value(b, "DD05") == 0x2, text);
        if (c->is91795) {
            memset(&d, 0, sizeof d);
            d.nib[4] = 1;
            d.nib[5] = 2;
        } else {
            memset(&d, 0, sizeof d);
            d.nib[0] = 0xA;
            d.nib[1] = 0xB;
            d.nib[2] = 0xC;
            d.nib[3] = 0xD;
            d.nib[4] = 1;
            d.nib[5] = 2;
        }
        bu_picture(c, &d, &pic);
        check(shows(b, &pic, glass, BU_FRAME), "... and the glass shows the nibbles that landed on segment addresses");
    }
    {
        unsigned one[1] = {0x34};

        w[0] = c->is91795 ? 0x0F : 0x12;
        w[1] = one[0];
        i2c_write(b, 0x3E, w, 2);
        if (c->is91795) {
            check(reg_value(b, "DD0F") == 3 && reg_value(b, "DD10") == 4, "BU91795: ADSET 0Fh (SEG11) continues into the unused 10h");
        } else {
            check(reg_value(b, "DD12") == 3 && reg_value(b, "DD13") == 4, "BU9796A: ADSET 12h (SEG18) continues into 13h (SEG19)");
        }
    }
    /* software reset keeps the RAM, clears the display on bit and the address */
    w[0] = 0xEA;
    i2c_write(b, 0x3E, w, 1);
    check(reg_value(b, "MODE") == 0 && reg_value(b, "DISCTL") == 2 && reg_value(b, "BLKCTL") == 0 && reg_value(b, "APCTL") == 0 &&
              reg_value(b, "ICSET") == 0 && reg_value(b, "DD04") == 1,
          "ICSET P1 = 1: display off, registers at reset (DISCTL 80 Hz, line inversion, normal), DDRAM data kept");
    /* ICSET software reset with C = 0: data follows at address 00h */
    {
        unsigned two[2] = {0x9C, 0x7E};

        w[0] = 0x6A;     /* C = 0, 1101, *, P1 = 1, P0 = 0 */
        memcpy(w + 1, two, sizeof two);
        i2c_write(b, 0x3E, w, 3);
        check(reg_value(b, "DD00") == 9 && reg_value(b, "DD01") == 0xC && reg_value(b, "DD02") == 7 && reg_value(b, "DD03") == 0xE,
              "ICSET reset with C = 0: the data bytes that follow are written from address 00h");
    }
    /* a non-ADSET command with C = 0: the RAM data after it is dropped (documented limit) */
    {
        unsigned one[1] = {0xFF};

        w[0] = 0x48;     /* MODESET P3 = 1, C = 0 */
        w[1] = one[0];
        i2c_write(b, 0x3E, w, 2);
        check(reg_value(b, "MODE") == 1 && reg_value(b, "DD04") == 1 && reg_value(b, "DD05") == 2,
              "MODESET with C = 0 switches the display on; the data byte after it does not reach the RAM (engine limit)");
    }
    close_bench(b);
}

/* The BU glass cases with other glass: 4D-1:2 wired to COM0-1, static to COM0. */
static void bu_glass(const BuChip *c, const char *glass, int bps) {
    static const char *const PULLS[] = {"SDA", "SCL", NULL};
    Bench *b = bench(c->part, glass, PULLS, "SDA", "SCL");
    Ddram d;
    Picture pic;
    unsigned bytes[12], w[2];
    char text[200];
    int i, g, k;

    start(b);
    run(b, 10 * US);
    memset(&d, 0, sizeof d);
    for (i = 0; i < 12; i++) {
        bytes[i] = (unsigned)((i * 37 + 0x2D) & 0xFF);
    }
    w[0] = 0xC8;
    i2c_write(b, 0x3E, w, 1);
    bu_ram(b, &d, c->is91795 ? 4u : 0u, bytes, (size_t)(c->segments + 1) / 2);
    bu_picture(c, &d, &pic);
    for (g = 0; g < 40; g++) {
        for (k = bps; k < 4; k++) {
            pic.bit[g][k] = 0;
        }
    }
    snprintf(text, sizeof text, "%s: %s wired to COM0-COM%d shows the RAM bits of its backplanes", c->part, glass, bps - 1);
    check(shows(b, &pic, glass, BU_FRAME), text);
    close_bench(b);
}

/* ---- Main -------------------------------------------------------------------------------------------------- */

int main(void) {
    static const BuChip BU91795 = {"BU91795MUF-ME2", 1, 12, 4, 24};
    static const BuChip BU9796AFS = {"BU9796AFS-E2", 0, 20, 0, 32};
    static const BuChip BU9796AMUV = {"BU9796AMUV-E2", 0, 12, 0, 24};
    static const BuChip BU9796AMUVZ = {"BU9796AMUV-ZE2", 0, 12, 0, 24};
    static const char *const PCFS[] = {"PCF8553DTT/AY", "PCF8553DTT/AJ", "PCA8553DTT/AY", "PCA8553DTT/AJ"};
    size_t i;

    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL loading the parts\n");
        return 1;
    }
    for (i = 0; i < 4; i++) {
        check(part_library_find(library, PCFS[i]) != NULL && part_library_find(library, PCFS[i])->kind == PART_LCDDRIVER,
              "the PCF8553 / PCA8553 parts load as lcddriver parts");
    }
    for (i = 0; i < 4; i += 2) {
        pcf_pinout(PCFS[i]);
        pcf_i2c(PCFS[i], PCFS[i][1] == 'C' && PCFS[i][2] == 'A');
    }
    pcf_glass("PCF8553DTT/AY", "GENERIC-LCD-8D-1:3", 1, 3);
    pcf_glass("PCF8553DTT/AY", "GENERIC-LCD-4D-1:2", 2, 2);
    pcf_glass("PCF8553DTT/AY", "GENERIC-LCD-4D-STATIC", 3, 1);
    pcf_glass("PCA8553DTT/AY", "GENERIC-LCD-8D-1:3", 1, 3);
    pcf_spi("PCF8553DTT/AY");
    pcf_spi("PCA8553DTT/AY");
    bu_chip(&BU91795);
    bu_chip(&BU9796AFS);
    bu_chip(&BU9796AMUV);
    bu_chip(&BU9796AMUVZ);
    bu_glass(&BU91795, "GENERIC-LCD-4D-1:2", 2);
    bu_glass(&BU9796AFS, "GENERIC-LCD-8D-1:3", 3);
    bu_glass(&BU9796AMUV, "GENERIC-LCD-4D-STATIC", 1);
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("lcd_spi_i2c_test: all passed\n");
    return 0;
}
