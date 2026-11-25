/*
 * is31fl3707_test (plan-56-D): the IS31FL3707 (Lumissil) I2C 8x8 / 7x9 matrix LED driver. Each bench
 * talks over the real SDA / SCL pins (pull-ups on both) and samples the scanned SW1-SW8 rows and the
 * CS1-CS7 / SW9/CS8 columns on simulated time.
 *
 * Covered: the three orderable parts and their pins, the AD address strap (GND, VCC, SCL, SDA), every
 * register of the map (PWM 03h-8Fh incl. the gaps between dots, scaling 92h-9Fh, A0h configuration,
 * A1h, B0h-B6h, the read-only open/short status, the CFh reset), the auto-increment write and read,
 * the scan of the rows (29.93 us per row: tSCAN 29 + tNOL1 0.6 + tNOL2 0.33, 8 rows), software shutdown
 * (SSD), hardware shutdown (SDB: columns blanked only) with the registers still reachable.
 *
 * Not covered (not modeled, see planning/plan-56-data/specs/is31fl3707-tlc6983.md): which dots are
 * lit (PWM nonzero: engineNeeds), the SWS row count, the 7x9 mode where pin 18 is SW9, PWM, current
 * scaling, open/short detection, spread spectrum.
 *
 * Datasheet: is31fl3707.pdf (Lumissil Rev. A, 09/17/2026).
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

/* ---- The bench: the chip, a test driver and pull-ups ---------------------------------------------- */

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
#define PERIOD (29930000ull)    /* one row: 29 us + 0.6 us + 0.33 us */

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

static void run_to(Bench *b, SimTime t) {
    if (t > sim_now(b->sim)) {
        run(b, t - sim_now(b->sim));
    }
}

/* `tie`: "GND", "VCC", "SCL" or "SDA": where the AD pin goes. SCL and SDA put the chip's AD pin on
   that line's net; GND and VCC drive it. SDB is held high. */
static Bench *bench(const char *part, const char *tie) {
    static const char *const PULLS[] = {"SDA", "SCL"};
    Bench *b = calloc(1, sizeof *b);
    SimNet *nets[64];
    size_t i, k;

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
        nets[i] = sim_add_net(b->sim, b->part->pins[i].name);
    }
    for (i = 0; i < b->part->pin_count; i++) {
        SimNet *net = nets[i];
        const char *name = b->part->pins[i].name;

        if (strcmp(name, "AD") == 0 && (strcmp(tie, "SCL") == 0 || strcmp(tie, "SDA") == 0)) {
            sim_connect(b->sim, nets[bpin(b, tie)], b->chip, i);   /* AD on the SCL / SDA net, no driver */
            continue;
        }
        sim_connect(b->sim, net, b->chip, i);
        sim_connect(b->sim, net, b->drv, i);
        for (k = 0; k < 2; k++) {
            if (strcmp(PULLS[k], name) == 0) {
                sim_connect(b->sim, net, b->pu, k);
            }
        }
    }
    if (!sim_start(b->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(b->sim));
        exit(2);
    }
    set(b, "VCC", LOGIC_1);
    set(b, "SDB", LOGIC_1);
    if (strcmp(tie, "GND") == 0) {
        set(b, "AD", LOGIC_0);
    } else if (strcmp(tie, "VCC") == 0) {
        set(b, "AD", LOGIC_1);
    }
    run(b, 1 * US);
    return b;
}

static void close_bench(Bench *b) {
    sim_destroy(b->sim);
    free(b);
}

/* ---- I2C --------------------------------------------------------------------------------------- */

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

static unsigned chip_addr = 0x68;

/* A write transaction: address, register, n data bytes (auto-increment). */
static void wr(Bench *b, unsigned reg, const unsigned *v, size_t n) {
    size_t i;

    i2c_start(b);
    i2c_byte(b, chip_addr << 1);
    i2c_byte(b, reg);
    for (i = 0; i < n; i++) {
        i2c_byte(b, v[i]);
    }
    i2c_stop(b);
}

static void wr1(Bench *b, unsigned reg, unsigned v) {
    wr(b, reg, &v, 1);
}

/* A read transaction: register pointer write, restart, n data bytes. */
static void rd(Bench *b, unsigned reg, unsigned *v, size_t n) {
    size_t i;

    i2c_start(b);
    i2c_byte(b, chip_addr << 1);
    i2c_byte(b, reg);
    i2c_start(b);
    i2c_byte(b, (chip_addr << 1) | 1u);
    for (i = 0; i < n; i++) {
        v[i] = i2c_in(b, i + 1 < n);
    }
    i2c_stop(b);
}

static unsigned rd1(Bench *b, unsigned reg) {
    unsigned v = 0xFFFF;

    rd(b, reg, &v, 1);
    return v;
}

/* Does the chip acknowledge this 8-bit address byte? */
static int acks(Bench *b, unsigned address_byte) {
    int ack;

    i2c_start(b);
    ack = i2c_byte(b, address_byte);
    if (ack && (address_byte & 1u)) {
        i2c_in(b, 0);   /* a read: take one byte and NAK it */
    }
    i2c_stop(b);
    return ack;
}

/* ---- The scan ------------------------------------------------------------------------------------ */

static const char *const SW[8] = {"SW1", "SW2", "SW3", "SW4", "SW5", "SW6", "SW7", "SW8"};
static const char *const CS[8] = {"CS1", "CS2", "CS3", "CS4", "CS5", "CS6", "CS7", "SW9/CS8"};

/* Bit r set = row SW(r+1) is high. */
static unsigned rows(const Bench *b) {
    unsigned m = 0;
    int r;

    for (r = 0; r < 8; r++) {
        if (get(b, SW[r]) == LOGIC_1) {
            m |= 1u << r;
        }
    }
    return m;
}

/* Bit c set = column CS(c+1) is low (a sink on, a lit dot). */
static unsigned lows(const Bench *b) {
    unsigned m = 0;
    int c;

    for (c = 0; c < 8; c++) {
        if (get(b, CS[c]) == LOGIC_0) {
            m |= 1u << c;
        }
    }
    return m;
}

/* Over `slots` consecutive row slots (sampled mid-slot) exactly row (slot % 8) is high, every column high. */
static int scanning(Bench *b, int slots) {
    int ok = 1, i;
    SimTime t = (sim_now(b->sim) / PERIOD + 1) * PERIOD + PERIOD / 2;

    for (i = 0; i < slots; i++) {
        unsigned want = 1u << ((t / PERIOD) % 8u);

        run_to(b, t);
        ok &= rows(b) == want && lows(b) == 0;
        t += PERIOD;
    }
    return ok;
}

static int dark(Bench *b) {
    int ok = 1, i;

    for (i = 0; i < 20; i++) {
        run(b, 11 * US);
        ok &= rows(b) == 0 && lows(b) == 0;
    }
    return ok;
}

/* ---- Tests -------------------------------------------------------------------------------------- */

static void parts(void) {
    static const char *const NAMES[3] = {"IS31FL3707-GRLS4-TR", "IS31FL3707-SALS4-TR", "IS31FL3707-SALS4"};
    static const char *const FOOT[3] = {"SOIC-24W", "QSOP-24", "QSOP-24"};
    int i;

    for (i = 0; i < 3; i++) {
        const Part *p = part_library_find(library, NAMES[i]);
        char what[96];

        snprintf(what, sizeof what, "%s: a leddriver part, 24 pins, footprint %s", NAMES[i], FOOT[i]);
        check(p != NULL && p->kind == PART_LEDDRIVER && p->pin_count == 24 &&
                  p->footprint != NULL && strcmp(p->footprint->id, FOOT[i]) == 0,
              what);
    }
    {
        Bench *b = bench("IS31FL3707-SALS4", "GND");

        check(strcmp(b->part->pins[17].name, "SW9/CS8") == 0 && strcmp(b->part->pins[0].name, "SW2") == 0 &&
                  strcmp(b->part->pins[1].name, "SW1") == 0 && strcmp(b->part->pins[23].name, "SW3") == 0,
              "pinout: 1 SW2, 2 SW1, 18 SW9/CS8, 24 SW3 (p.3)");
        close_bench(b);
    }
}

static void straps(void) {
    static const struct {
        const char *tie;
        unsigned address;
    } T[4] = {{"GND", 0x68}, {"SCL", 0x69}, {"SDA", 0x6A}, {"VCC", 0x6B}};
    int i, k;

    for (i = 0; i < 4; i++) {
        Bench *b = bench("IS31FL3707-GRLS4-TR", T[i].tie);
        char what[96];
        int others_nak = 1;

        snprintf(what, sizeof what, "AD at %s: acknowledges write 0x%02X and read 0x%02X", T[i].tie,
                 T[i].address << 1, (T[i].address << 1) | 1u);
        check(acks(b, T[i].address << 1) && acks(b, (T[i].address << 1) | 1u), what);
        for (k = 0; k < 4; k++) {
            if (k != i) {
                others_nak &= !acks(b, T[k].address << 1);
            }
        }
        others_nak &= !acks(b, 0x38u << 1);
        snprintf(what, sizeof what, "AD at %s: the other three straps' addresses and 0x38 are not acknowledged",
                 T[i].tie);
        check(others_nak, what);
        close_bench(b);
    }
}

static void registers(void) {
    Bench *b = bench("IS31FL3707-GRLS4-TR", "GND");
    unsigned v[32], r[32];
    unsigned a;
    int ok;

    chip_addr = 0x68;
    /* reset values (Table 2) */
    check(rd1(b, 0xA0) == 0x10, "reset: CONFIG (A0h) = 0001 0000 (8 rows, software shutdown)");
    check(rd1(b, 0xA1) == 0 && rd1(b, 0xB0) == 0 && rd1(b, 0xB1) == 0 && rd1(b, 0xB3) == 0 &&
              rd1(b, 0xB6) == 0,
          "reset: A1h, B0h, B1h, B3h, B6h = 0");
    check(rd1(b, 0xB2) == 0x01, "reset: PWM frequency (B2h) = 0000 0001 (34 kHz)");
    ok = 1;
    for (a = 0x03; a <= 0x8F; a++) {
        ok &= rd1(b, a) == 0;
    }
    check(ok, "reset: PWM 03h-8Fh read 0");
    ok = 1;
    for (a = 0x92; a <= 0x9F; a++) {
        ok &= rd1(b, a) == 0;
    }
    check(ok, "reset: scaling 92h-9Fh read 0");

    /* single writes to the corners of Figure 9 / Figure 10 */
    {
        static const unsigned PWM[] = {0x03, 0x04, 0x07, 0x0E, 0x0F, 0x10, 0x13, 0x20, 0x30, 0x40, 0x50,
                                       0x60, 0x70, 0x80, 0x83, 0x87, 0x8E, 0x8F};
        size_t i;

        ok = 1;
        for (i = 0; i < sizeof PWM / sizeof PWM[0]; i++) {
            wr1(b, PWM[i], 0x80u + (unsigned)i);
        }
        for (i = 0; i < sizeof PWM / sizeof PWM[0]; i++) {
            ok &= rd1(b, PWM[i]) == 0x80u + (unsigned)i;
        }
        check(ok, "PWM: 18 dots (SW1 CS1/CS2/CS5/CS6/CS7/CS8, SW2 CS1, SW3..SW8 CS8, SW9 CS1/CS5/CS6/CS7) write and read back");
    }
    wr1(b, 0x03, 0xB5);
    check(rd1(b, 0x03) == 0xB5 && rd1(b, 0x04) == 0x81, "PWM 03h = B5h, its neighbour 04h unchanged");
    wr1(b, 0x03, 0x00);
    check(rd1(b, 0x03) == 0, "PWM 03h written back to 0");
    ok = 1;
    for (a = 0x92; a <= 0x96; a++) {
        wr1(b, a, 0xA0u + a - 0x92u);
    }
    wr1(b, 0x9D, 0xB0);
    wr1(b, 0x9E, 0xB1);
    wr1(b, 0x9F, 0xB2);
    for (a = 0x92; a <= 0x96; a++) {
        ok &= rd1(b, a) == 0xA0u + a - 0x92u;
    }
    ok &= rd1(b, 0x9D) == 0xB0 && rd1(b, 0x9E) == 0xB1 && rd1(b, 0x9F) == 0xB2;
    check(ok, "scaling: CS1-CS5 at 92h-96h, CS6-CS8 at 9Dh-9Fh write and read back");

    /* the gaps are reserved: writes dropped, reads 0 */
    wr1(b, 0x08, 0xFF);
    wr1(b, 0x0D, 0xFF);
    wr1(b, 0x11, 0xFF);
    wr1(b, 0x90, 0xFF);
    wr1(b, 0x97, 0xFF);
    wr1(b, 0x9C, 0xFF);
    check(rd1(b, 0x08) == 0 && rd1(b, 0x0D) == 0 && rd1(b, 0x11) == 0 && rd1(b, 0x90) == 0 &&
              rd1(b, 0x97) == 0 && rd1(b, 0x9C) == 0,
          "addresses between dots (08h-0Dh, 11h-12h, 90h-91h, 97h-9Ch): writes dropped, read 0");

    /* the other registers */
    wr1(b, 0xA1, 0x40);
    wr1(b, 0xB0, 0xFA);
    wr1(b, 0xB1, 0x1F);
    wr1(b, 0xB2, 0x08);
    wr1(b, 0xB3, 0x08);
    wr1(b, 0xB6, 0xB7);
    check(rd1(b, 0xA1) == 0x40 && rd1(b, 0xB0) == 0xFA && rd1(b, 0xB1) == 0x1F && rd1(b, 0xB2) == 0x08 &&
              rd1(b, 0xB3) == 0x08 && rd1(b, 0xB6) == 0xB7,
          "A1h GCC, B0h pull-down/up, B1h spread spectrum, B2h PWM frequency, B3h open/short select, B6h write and read back");
    wr1(b, 0xA0, 0x57);
    check(rd1(b, 0xA0) == 0x57, "CONFIG A0h: SWS 5, OSDE 3, SSD 1 write and read back");
    wr1(b, 0xB4, 0xFF);
    wr1(b, 0xB5, 0xFF);
    check(rd1(b, 0xB4) == 0 && rd1(b, 0xB5) == 0, "open/short status B4h, B5h are read only (read 0, writes dropped)");

    /* auto-increment (Figure 7): 05h, 06h, 07h, the unused 08h-0Dh, then 0Eh, 0Fh, 10h */
    for (a = 0; a < 12; a++) {
        v[a] = 0x21u + a;
    }
    wr(b, 0x05, v, 12);
    rd(b, 0x05, r, 3);
    check(r[0] == 0x21 && r[1] == 0x22 && r[2] == 0x23, "auto-increment write from 05h: dots 05h, 06h, 07h hold bytes 0-2");
    rd(b, 0x0E, r, 3);
    check(r[0] == 0x2A && r[1] == 0x2B && r[2] == 0x2C,
          "auto-increment write: the pointer steps through the 6 unused addresses (08h-0Dh), 0Eh-10h hold bytes 9-11");
    check(rd1(b, 0x08) == 0 && rd1(b, 0x0D) == 0, "auto-increment write: the unused addresses stay 0");
    {
        unsigned seq[4];

        rd(b, 0x0E, seq, 4);
        check(seq[0] == 0x2A && seq[1] == 0x2B && seq[2] == 0x2C && seq[3] == 0,
              "auto-increment read from 0Eh: 0Eh, 0Fh, 10h, then unused 11h reads 0");
    }
    v[0] = 0x71;
    v[1] = 0x72;
    v[2] = 0x73;
    v[3] = 0x74;
    wr(b, 0x10, v, 4);
    check(rd1(b, 0x10) == 0x71 && rd1(b, 0x13) == 0x74 && rd1(b, 0x11) == 0 && rd1(b, 0x12) == 0,
          "auto-increment write 10h to 13h crosses SW1 CS8 -> unused 11h/12h -> SW2 CS1");

    /* reset register CFh: AEh resets everything, anything else does nothing */
    wr1(b, 0xCF, 0x55);
    check(rd1(b, 0xA0) == 0x57 && rd1(b, 0x05) == 0x21, "CFh = 55h: no reset");
    wr1(b, 0xCF, 0xAE);
    ok = rd1(b, 0xA0) == 0x10 && rd1(b, 0xA1) == 0 && rd1(b, 0xB0) == 0 && rd1(b, 0xB1) == 0 &&
         rd1(b, 0xB2) == 0x01 && rd1(b, 0xB3) == 0 && rd1(b, 0xB6) == 0 && rd1(b, 0xCF) == 0;
    check(ok, "CFh = AEh: CONFIG back to 10h, B2h to 01h, A1h, B0h, B1h, B3h, B6h to 0");
    ok = 1;
    for (a = 0x03; a <= 0x8F; a++) {
        ok &= rd1(b, a) == 0;
    }
    for (a = 0x92; a <= 0x9F; a++) {
        ok &= rd1(b, a) == 0;
    }
    check(ok, "CFh = AEh: every PWM and scaling register back to 0");
    close_bench(b);
}

static void scan(void) {
    Bench *b = bench("IS31FL3707-SALS4", "GND");
    int i;

    chip_addr = 0x68;
    /* power-on: software shutdown (SSD 0): no row, no column */
    check(dark(b), "power-on (SSD = 0): every row low and every column high (matrix blanked)");
    wr1(b, 0xA0, 0x11);     /* SWS 0001 (8 rows), SSD 1 */
    check(scanning(b, 24), "SSD = 1: SW1-SW8 rise one at a time in order, three rounds, every column high (no dot lit)");

    /* the row period: 29.93 us */
    {
        SimTime now = sim_now(b->sim);
        SimTime next = (now / PERIOD + 1) * PERIOD;
        unsigned k = (unsigned)((next / PERIOD) % 8u);

        run_to(b, next - 1 * US);
        i = rows(b) == 1u << ((k + 7u) % 8u);
        run_to(b, next + 1 * US);
        check(i && rows(b) == 1u << k, "row period 29.93 us: the next row rises 29.93 us after the last, not before");
    }
    /* the PWM data of a dot does not move the scan */
    wr1(b, 0x03, 0xFF);
    check(scanning(b, 8), "a PWM write does not disturb the row sequence");

    /* software shutdown again */
    wr1(b, 0xA0, 0x10);
    run(b, 5 * US);
    check(dark(b), "SSD back to 0: scan stops, every row low and every column high, registers kept");
    check(rd1(b, 0x03) == 0xFF, "software shutdown: the registers keep their data (p.18) and can be read");
    wr1(b, 0xA0, 0x11);
    check(scanning(b, 8), "SSD = 1 again: scan resumes");

    /* hardware shutdown: SDB low */
    set(b, "SDB", LOGIC_0);
    run(b, 5 * US);
    {
        int ok = 1;

        for (i = 0; i < 20; i++) {
            run(b, 11 * US);
            ok &= lows(b) == 0;
        }
        /* The real chip also stops the SW rows; the model can only blank the columns (engineNeeds). */
        check(ok, "SDB low (SSD = 1): every column stays high, no sink on (the rows keep scanning in the model)");
    }
    wr1(b, 0xA1, 0x3C);
    check(rd1(b, 0xA1) == 0x3C && rd1(b, 0x03) == 0xFF,
          "SDB low: the registers can still be written and read (p.16)");
    set(b, "SDB", LOGIC_1);
    run(b, 20 * US);
    check(scanning(b, 8) && rd1(b, 0xA1) == 0x3C, "SDB high: scan resumes, registers kept");

    /* software reset stops the scan (SSD back to 0) */
    wr1(b, 0xCF, 0xAE);
    run(b, 5 * US);
    check(dark(b), "reset register AEh: CONFIG back to SSD = 0, scan stops");
    close_bench(b);
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL loading the parts\n");
        return 1;
    }
    parts();
    straps();
    registers();
    scan();
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("is31fl3707_test: all passed\n");
    return 0;
}
