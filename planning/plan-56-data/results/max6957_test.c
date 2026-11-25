/*
 * max6957_test (plan-56-D): the MAX6957 28-port (AAX 36-pin SSOP, ATL 40-pin TQFN) and 20-port (AAI 28-pin
 * SSOP, ANI 28-pin DIP) SPI LED driver / GPIO expander. Every check goes through the real SPI pins (16-bit
 * frames, MSB first, mode 0) and looks at the port pins, at a real LED wired to a port, and at the DOUT
 * bits the next frame shifts out. The external world is a test driver on the same nets.
 *
 * Datasheet: max6957.pdf (Maxim 19-2429 Rev 5).
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

static const SimComponentType DRIVER = {"tb", drv_pins, drv_create, free, NULL, NULL, NULL,
                                        NULL, NULL, NULL, NULL, NULL};

typedef struct {
    Sim *sim;
    SimInstance *chip, *drv, *led, *vcc;
    const Part *part;
    PinCount pins;
} Bench;

static const SimTime US = 1000000;
static const SimTime HALF = 100000;

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

static const Part *find(const char *name) {
    const Part *p = part_library_find(library, name);

    if (p == NULL) {
        fprintf(stderr, "no part %s\n", name);
        exit(2);
    }
    return p;
}

/* `led_port`: a GENERIC-LED with its anode on a 5 V supply and its cathode on that port pin (or NULL). */
static Bench *bench(const char *part, const char *led_port) {
    Bench *b = calloc(1, sizeof *b);
    SimNet **nets;
    size_t i;

    b->part = find(part);
    b->pins.count = b->part->pin_count;
    b->sim = sim_create();
    b->chip = sim_add_instance(b->sim, kind_model(b->part), "U1", b->part);
    b->drv = sim_add_instance(b->sim, &DRIVER, "TB", &b->pins);
    nets = calloc(b->part->pin_count, sizeof *nets);
    for (i = 0; i < b->part->pin_count; i++) {
        nets[i] = sim_add_net(b->sim, b->part->pins[i].name);
        sim_connect(b->sim, nets[i], b->chip, i);
        sim_connect(b->sim, nets[i], b->drv, i);
    }
    if (led_port != NULL) {
        const Part *led = find("GENERIC-LED"), *five = find("5V");
        SimNet *rail = sim_add_net(b->sim, "5V");

        b->led = sim_add_instance(b->sim, kind_model(led), "D1", led);
        b->vcc = sim_add_instance(b->sim, kind_model(five), "PWR1", five);
        sim_connect(b->sim, rail, b->vcc, 0);
        sim_connect(b->sim, rail, b->led, pin_of(led, "A"));
        sim_connect(b->sim, nets[bpin(b, led_port)], b->led, pin_of(led, "K"));
    }
    free(nets);
    if (!sim_start(b->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(b->sim));
        exit(2);
    }
    set(b, "CS", LOGIC_1);
    set(b, "SCLK", LOGIC_0);
    run(b, HALF);
    return b;
}

static void close_bench(Bench *b) {
    sim_destroy(b->sim);
    free(b);
}

/* One 16-bit frame (CS low, MSB first, DIN set up before SCLK rises); returns the 16 bits DOUT showed,
   each read just before its rising edge. `bits` other than 16: only the last `bits` of `word` are sent. */
static unsigned frame(Bench *b, unsigned long word, unsigned bits) {
    unsigned i, out = 0;

    set(b, "CS", LOGIC_0);
    run(b, HALF);
    for (i = 0; i < bits; i++) {
        set(b, "DIN", (word >> (bits - 1 - i)) & 1u ? LOGIC_1 : LOGIC_0);
        run(b, HALF);
        out = (out << 1) | (get(b, "DOUT") == LOGIC_1 ? 1u : 0u);
        set(b, "SCLK", LOGIC_1);
        run(b, HALF);
        set(b, "SCLK", LOGIC_0);
    }
    run(b, HALF);
    set(b, "CS", LOGIC_1);
    run(b, HALF);
    return out;
}

static void wr(Bench *b, unsigned addr, unsigned data) {
    frame(b, ((unsigned long)addr << 8) | data, 16);
}

/* A read: the command, then a no-op whose frame shifts the result out. Returns the 16 bits (command byte,
   register byte) of that second frame. */
static unsigned rd16(Bench *b, unsigned addr) {
    frame(b, 0x8000u | (addr << 8), 16);
    return frame(b, 0, 16);
}

static unsigned rd(Bench *b, unsigned addr) {
    return rd16(b, addr) & 0xFFu;
}

static unsigned reg(Bench *b, const char *name) {
    const SimComponentType *type = sim_instance_type(b->chip);
    ConverterRegister regs[128];
    size_t n = type->registers(sim_instance_state(b->chip), regs, 128), i;

    for (i = 0; i < n && i < 128; i++) {
        if (strcmp(regs[i].name, name) == 0) {
            return regs[i].value;
        }
    }
    fprintf(stderr, "no register %s\n", name);
    exit(2);
}

static void port_pin(char *out, int p) {
    sprintf(out, "P%d", p);
}

static LogicValue port(Bench *b, int p) {
    char name[8];

    port_pin(name, p);
    return get(b, name);
}

static void drive_port(Bench *b, int p, LogicValue v) {
    char name[8];

    port_pin(name, p);
    set(b, name, v);
}

/* ---- Pinouts --------------------------------------------------------------------------------------- */

typedef struct {
    int number;
    const char *name;
} PinRow;

static int pinout_is(const Part *part, const PinRow *rows, size_t n) {
    size_t i;

    if (part->pin_count != n) {
        return 0;
    }
    for (i = 0; i < n; i++) {
        if (part->pins[i].name == NULL || strcmp(part->pins[i].name, rows[i].name) != 0) {
            return 0;
        }
    }
    return 1;
}

static void pinouts(void) {
    /* p.20: 36-pin SSOP */
    static const char *const SSOP36[36] = {"ISET", "GND", "GND", "DOUT", "P8", "P12", "P9", "P13", "P10", "P14", "P11",
                                           "P15", "P16", "P17", "P18", "P19", "P20", "P21", "P22", "P23", "P24", "P25",
                                           "P26", "P27", "P28", "P7", "P29", "P6", "P30", "P5", "P31", "P4", "SCLK",
                                           "DIN", "CS", "V+"};
    /* p.20: 40-pin TQFN (N.C. 11, 20, 31) */
    static const char *const TQFN40[40] = {"P8", "P12", "P9", "P13", "P10", "P14", "P11", "P15", "P16", "P17", "NC",
                                           "P18", "P19", "P20", "P21", "P22", "P23", "P24", "P25", "NC", "P26", "P27",
                                           "P28", "P7", "P29", "P6", "P30", "P5", "P31", "P4", "NC", "SCLK", "DIN",
                                           "CS", "V+", "ISET", "GND", "GND", "GND", "DOUT"};
    const Part *p;
    size_t i;
    int ok = 1;

    p = find("MAX6957AAX+");
    ok = p->pin_count == 36;
    for (i = 0; ok && i < 36; i++) {
        ok = strcmp(p->pins[i].name, SSOP36[i]) == 0;
    }
    check(ok, "MAX6957AAX+: the 36-pin SSOP pinout is the one on p.20");
    p = find("MAX6957ATL+");
    ok = p->pin_count == 40;
    for (i = 0; ok && i < 40; i++) {
        ok = strcmp(p->pins[i].name, TQFN40[i]) == 0;
    }
    check(ok, "MAX6957ATL+: the 40-pin TQFN pinout is the one on p.20 (exposed pad is not a pin)");
    p = find("MAX6957AAI+");
    ok = p->pin_count == 28 && strcmp(p->pins[0].name, "ISET") == 0 && strcmp(p->pins[3].name, "DOUT") == 0 &&
         strcmp(p->pins[4].name, "P12") == 0 && strcmp(p->pins[23].name, "P31") == 0 && strcmp(p->pins[24].name, "SCLK") == 0 &&
         strcmp(p->pins[25].name, "DIN") == 0 && strcmp(p->pins[26].name, "CS") == 0 && strcmp(p->pins[27].name, "V+") == 0;
    check(ok && pin_of(p, "P4") == SIZE_MAX && pin_of(p, "P11") == SIZE_MAX, "MAX6957AAI+: the 28-pin pinout has ports P12-P31 only (p.1)");
    p = find("MAX6957ANI+");
    check(p->pin_count == 28 && pin_of(p, "P12") == 4, "MAX6957ANI+: the 28-pin DIP has the same pinout");
}

/* ---- One device ------------------------------------------------------------------------------------ */

/* Port p's configuration register address (Table 1) and the shift of its two bits. */
static unsigned cfg_addr(int p) {
    return 0x09u + (unsigned)(p / 4) - 1u;
}

static void set_mode(Bench *b, int p, unsigned mode, unsigned others) {
    /* all four ports of p's register at `others` (a 2-bit mode), port p at `mode` */
    unsigned v = 0, k;

    for (k = 0; k < 4; k++) {
        v |= (k == (unsigned)(p % 4) ? mode : others) << (2 * k);
    }
    wr(b, cfg_addr(p), v);
}

static void device(const char *part, int first_port) {
    Bench *b = bench(part, "P20");
    char text[240];
    int p, ok, q;
    unsigned v, w;
    const int top = 31;

    snprintf(text, sizeof text, "%s: a leddriver part", part);
    check(b->part->kind == PART_LEDDRIVER, text);

    /* power-up (Table 4): shutdown, every port an input without pullup, registers at their reset values */
    run(b, 100 * US);
    ok = 1;
    for (p = first_port; p <= top; p++) {
        ok &= port(b, p) == LOGIC_Z;
    }
    snprintf(text, sizeof text, "%s: power-up: all ports are inputs (released)", part);
    check(ok, text);
    check(get(b, "DOUT") == LOGIC_Z, "DOUT is released while CS is high (a deviation from the sheet: it never floats there)");
    snprintf(text, sizeof text, "%s: power-up registers (Table 4): port config 0xAA, configuration 0 (shutdown), global current 0, mask 0, ports 0", part);
    check(reg(b, "PC4") == 0xAA && reg(b, "PC12") == 0xAA && reg(b, "PC28") == 0xAA && reg(b, "CONFIG") == 0 &&
              reg(b, "GLOBAL") == 0 && reg(b, "MASK") == 0 && reg(b, "TEST") == 0 && reg(b, "PORT") == 0 &&
              reg(b, "CUR054") == 0 && reg(b, "CUR1FE") == 0,
          text);

    /* port configuration of P12-P15: LED driver, GPIO output, input, input with pullup (Table 2) */
    wr(b, 0x0B, 0xE4);
    wr(b, 0x2C, 0x01);                          /* P12 on */
    wr(b, 0x2D, 0x01);                          /* P13 high */
    run(b, 10 * US);
    snprintf(text, sizeof text, "%s: in shutdown every port stays released, even a pulled-up input and a GPIO output at 1", part);
    check(port(b, 12) == LOGIC_Z && port(b, 13) == LOGIC_Z && port(b, 14) == LOGIC_Z && port(b, 15) == LOGIC_Z, text);
    wr(b, 0x04, 0x01);                          /* normal operation */
    run(b, 10 * US);
    snprintf(text, sizeof text, "%s: LED driver port with bit 1 sinks (pin low)", part);
    check(port(b, 12) == LOGIC_0, text);
    snprintf(text, sizeof text, "%s: GPIO output with bit 1 drives high", part);
    check(port(b, 13) == LOGIC_1, text);
    snprintf(text, sizeof text, "%s: input without pullup: released", part);
    check(port(b, 14) == LOGIC_Z, text);
    snprintf(text, sizeof text, "%s: input with pullup: a floating pin is pulled high", part);
    check(port(b, 15) == LOGIC_1, text);
    wr(b, 0x2C, 0x00);
    wr(b, 0x2D, 0x00);
    snprintf(text, sizeof text, "%s: LED port with bit 0 is released (off), GPIO output with bit 0 drives low", part);
    check(port(b, 12) == LOGIC_Z && port(b, 13) == LOGIC_0, text);

    /* the port register read-back: output / LED: the stored bit, input: the pin */
    wr(b, 0x2C, 0x01);
    wr(b, 0x2D, 0x01);
    v = rd16(b, 0x2C);
    snprintf(text, sizeof text, "%s: read port 12: the frame shifts out the command byte 0xAC, then 0x01", part);
    check(v == 0xAC01u, text);
    check(rd(b, 0x2D) == 1, "read port 13 (GPIO output high) = 1");
    check(rd(b, 0x2E) == 0, "read port 14 (input, floating) = 0");
    check(rd(b, 0x2F) == 1, "read port 15 (input with pullup, floating) = 1");
    drive_port(b, 14, LOGIC_1);
    drive_port(b, 15, LOGIC_0);
    run(b, 10 * US);
    check(rd(b, 0x2E) == 1, "read port 14 (input) follows the pin: 1 when the pin is driven high");
    check(rd(b, 0x2F) == 0, "read port 15 (input with pullup) = 0 when the pin is driven low");
    check(port(b, 15) == LOGIC_0 && port(b, 14) == LOGIC_1, "an external driver wins over the weak pullup");
    drive_port(b, 14, LOGIC_Z);
    drive_port(b, 15, LOGIC_Z);
    run(b, 10 * US);
    check(rd(b, 0x2C) == 1 && rd(b, 0x2D) == 1, "read ports 12, 13 return their stored bits");

    /* eight ports at a time: P12-P19 as GPIO outputs, written and read through 0x4C / 0xCC */
    wr(b, 0x0B, 0x55);
    wr(b, 0x0C, 0x55);
    wr(b, 0x4C, 0xA5);
    ok = port(b, 12) == LOGIC_1 && port(b, 13) == LOGIC_0 && port(b, 14) == LOGIC_1 && port(b, 15) == LOGIC_0 &&
         port(b, 16) == LOGIC_0 && port(b, 17) == LOGIC_1 && port(b, 18) == LOGIC_0 && port(b, 19) == LOGIC_1;
    snprintf(text, sizeof text, "%s: write ports 12-19 (0x4C) with 0xA5: D0 is port 12, D7 port 19", part);
    check(ok, text);
    check(rd(b, 0x4C) == 0xA5, "read ports 12-19 (0xCC) returns 0xA5");
    wr(b, 0x4D, 0x0F);                             /* ports 13-20: 13..16 high, 17..20 low */
    check(port(b, 12) == LOGIC_1 && port(b, 13) == LOGIC_1 && port(b, 16) == LOGIC_1 && port(b, 17) == LOGIC_0 &&
              port(b, 19) == LOGIC_0,
          "write ports 13-20 (0x4D) 0x0F: P12 untouched, P13-P16 high, P17-P19 low");
    check(rd(b, 0x4D) == 0x0F, "read ports 13-20 returns 0x0F");
    wr(b, 0x0E, 0x55);
    wr(b, 0x0F, 0x55);
    wr(b, 0x58, 0xC3);
    ok = port(b, 24) == LOGIC_1 && port(b, 25) == LOGIC_1 && port(b, 26) == LOGIC_0 && port(b, 29) == LOGIC_0 &&
         port(b, 30) == LOGIC_1 && port(b, 31) == LOGIC_1;
    snprintf(text, sizeof text, "%s: write ports 24-31 (0x58) with 0xC3", part);
    check(ok, text);
    wr(b, 0x5C, 0xF5);                             /* ports 28-31: D0-D3 = 0101, D7-D4 do not exist */
    check(port(b, 28) == LOGIC_1 && port(b, 29) == LOGIC_0 && port(b, 30) == LOGIC_1 && port(b, 31) == LOGIC_0 &&
              port(b, 27) == LOGIC_0,
          "write ports 28-31 (0x5C) 0xF5: D3-D0 used");
    check(rd(b, 0x5C) == 0x05, "read ports 28-31 (0xDC) returns 0x05: D7-D4 read 0");
    wr(b, 0x5F, 0x01);
    check(port(b, 31) == LOGIC_1 && rd(b, 0x5F) == 1, "write and read port 31 only through 0x5F");
    wr(b, 0x5A, 0xFF);                             /* ports 26-31 */
    check(port(b, 26) == LOGIC_1 && port(b, 31) == LOGIC_1 && rd(b, 0x5A) == 0x3F, "ports 26-31 (0x5A): six bits, read back 0x3F");
    wr(b, 0x59, 0x00);
    check(port(b, 25) == LOGIC_0 && port(b, 31) == LOGIC_0 && rd(b, 0x59) == 0, "ports 25-31 (0x59): seven bits");
    wr(b, 0x3F, 0xFE);
    check(port(b, 31) == LOGIC_0, "single-port write takes D0 only: 0xFE writes 0");
    wr(b, 0x3F, 0x03);
    check(port(b, 31) == LOGIC_1, "single-port write of 0x03 sets the port (D0 = 1)");
    if (first_port == 4) {
        wr(b, 0x0A, 0x55);
        wr(b, 0x09, 0x55);
        wr(b, 0x44, 0x81);                         /* ports 4-11 */
        check(port(b, 4) == LOGIC_1 && port(b, 11) == LOGIC_1 && port(b, 7) == LOGIC_0 && rd(b, 0x44) == 0x81,
              "write ports 4-11 (0x44) 0x81: P4 and P11 high");
        wr(b, 0x40, 0x0A);                         /* ports 4-7 */
        check(port(b, 5) == LOGIC_1 && port(b, 4) == LOGIC_0 && port(b, 7) == LOGIC_1 && port(b, 8) == LOGIC_0 &&
                  rd(b, 0x40) == 0x0A,
              "write ports 4-7 (0x40) 0x0A: only P4-P7 change");
        wr(b, 0x43, 0x7F);
        check(rd(b, 0x43) == 0x7F && port(b, 10) == LOGIC_1, "ports 4-10 (0x43): seven bits");
    } else {
        wr(b, 0x44, 0x81);
        wr(b, 0x25, 0x01);
        check(rd(b, 0x44) == 0 && rd(b, 0x25) == 0, "20-port device: the registers of P4-P11 take writes but no pin exists, read-back 0");
        check(reg(b, "PORT") != 0, "20-port device: the data bits of P4-P11 are stored");
    }

    /* P20 is on a real LED (anode on 5 V): configure it as an LED driver */
    set_mode(b, 20, 0, 2);
    wr(b, 0x34, 0x00);
    {
        SimTime since = sim_now(b->sim);
        const SimComponentType *type = sim_instance_type(b->led);
        unsigned char values[4];

        run(b, 50 * US);
        type->lights(b->sim, b->led, sim_instance_state(b->led), since, values, sizeof values);
        check(values[0] == 0, "LED on P20: register bit 0, the LED is off");
        wr(b, 0x34, 0x01);
        since = sim_now(b->sim);
        run(b, 50 * US);
        type->lights(b->sim, b->led, sim_instance_state(b->led), since, values, sizeof values);
        check(values[0] == 1, "LED on P20: register bit 1, the sink turns the LED on");
    }

    /* the registers read back through the bus */
    wr(b, 0x02, 0xFF);
    check(rd(b, 0x02) == 0x0F && reg(b, "GLOBAL") == 0x0F, "global current register keeps D3-D0 (reads 0x0F)");
    wr(b, 0x04, 0xFF);
    check(rd(b, 0x04) == 0xC1 && reg(b, "CONFIG") == 0xC1, "configuration register: M (D7), I (D6) and S (D0) are stored, D5-D1 read 0");
    wr(b, 0x04, 0x01);
    wr(b, 0x06, 0xFF);
    check(rd(b, 0x06) == 0x7F, "transition detect mask register: D6-D0 stored, D7 reads 0");
    wr(b, 0x06, 0x00);
    wr(b, 0x07, 0xFF);
    check(rd(b, 0x07) == 0x01, "display test register: only D0 is stored");
    wr(b, 0x07, 0x00);
    ok = 1;
    for (w = 0x12; w <= 0x1F; w++) {
        wr(b, w, 0xA0u | (w - 0x12));
    }
    for (w = 0x12; w <= 0x1F; w++) {
        ok &= rd(b, w) == (0xA0u | (w - 0x12));
    }
    check(ok, "the 14 segment current registers 0x12-0x1F store and read back both nibbles");
    check(rd(b, 0x0F) == 0x55 && rd(b, 0x0B) == 0x55, "port configuration registers read back");
    check(rd(b, 0x00) == 0 && rd(b, 0x01) == 0 && rd(b, 0x08) == 0 && rd(b, 0x20) == 0 && rd(b, 0x23) == 0,
          "the no-op, unused addresses and virtual ports 0-3 read 0");
    wr(b, 0x20, 0xFF);
    wr(b, 0x01, 0xFF);
    check(reg(b, "CONFIG") == 0x01 && reg(b, "GLOBAL") == 0x0F, "writes to virtual ports and unused addresses change nothing");

    /* read pipelining: every frame shifts out what the frame before it asked for (p.10) */
    wr(b, 0x2D, 0x01);
    frame(b, 0x8000u | (0x2Du << 8), 16);                  /* ask for port 13 */
    v = frame(b, 0x8000u | (0x02u << 8), 16);              /* ask for 0x02 while receiving port 13 */
    w = frame(b, 0x0000, 16);
    check(v == 0xAD01u && w == 0x820Fu, "back-to-back reads: the data of each read arrives in the following frame, after the command byte");
    wr(b, 0x2D, 0x00);
    frame(b, (0xA5ul << 16) | 0x2D01ul, 24);
    check(port(b, 13) == LOGIC_1, "a frame of 24 bits is decoded from its last 16");
    frame(b, 0x2D00ul >> 8, 8);
    check(port(b, 13) == LOGIC_1, "a frame of fewer than 16 bits changes nothing");
    v = frame(b, 0x0000, 16);
    check(v == 0, "no read pending: DOUT shifts out zeros");
    wr(b, 0x2D, 0x00);
    v = frame(b, 0x0000, 16);
    check(v == 0x2D00u, "the frame after a write shifts out that write's command and data bytes (daisy-chain copy)");

    /* shutdown: ports forced to inputs, settings kept; the display test overrides it for LED drivers */
    wr(b, 0x0B, 0xE4);
    wr(b, 0x2C, 0x01);
    wr(b, 0x2D, 0x01);
    wr(b, 0x04, 0x00);
    snprintf(text, sizeof text, "%s: shutdown releases the LED and GPIO ports and keeps their data", part);
    check(port(b, 12) == LOGIC_Z && port(b, 13) == LOGIC_Z && port(b, 15) == LOGIC_Z && rd(b, 0x2C) == 1, text);
    wr(b, 0x07, 0x01);
    snprintf(text, sizeof text, "%s: display test in shutdown: LED ports sink, the GPIO output stays released", part);
    check(port(b, 12) == LOGIC_0 && port(b, 13) == LOGIC_Z, text);
    wr(b, 0x2C, 0x00);
    check(port(b, 12) == LOGIC_0, "display test sinks an LED port whose register bit is 0");
    wr(b, 0x04, 0x01);
    check(port(b, 12) == LOGIC_0 && port(b, 13) == LOGIC_1 && port(b, 15) == LOGIC_1,
          "display test with normal operation: LED port sinks, the GPIO output and the pullup are unaffected");
    wr(b, 0x07, 0x00);
    check(port(b, 12) == LOGIC_Z && port(b, 13) == LOGIC_1, "display test off: the LED port follows its bit again (released)");

    /* each port's own configuration field and data bit drive only its own pin */
    ok = 1;
    for (p = first_port; p <= top; p++) {
        for (q = 0; q < 7; q++) {
            wr(b, 0x09u + (unsigned)q, 0xAA);
        }
        set_mode(b, p, 1, 2);                     /* P p a GPIO output, its neighbours inputs */
        wr(b, 0x20u + (unsigned)p, 0x01);
        run(b, 5 * US);
        for (q = first_port; q <= top; q++) {
            LogicValue lv = port(b, q);

            if (q == p ? lv != LOGIC_1 : lv != LOGIC_Z) {
                ok = 0;
                printf("     port %d output: P%d reads %d\n", p, q, (int)lv);
            }
        }
        set_mode(b, p, 0, 2);                     /* an LED driver: low with the bit set */
        if (port(b, p) != LOGIC_0) {
            ok = 0;
            printf("     port %d as LED: reads %d\n", p, (int)port(b, p));
        }
        wr(b, 0x20u + (unsigned)p, 0x00);
        if (port(b, p) != LOGIC_Z) {
            ok = 0;
        }
    }
    snprintf(text, sizeof text, "%s: each port's own two configuration bits and data bit drive its own pin (P%d-P31)", part, first_port);
    check(ok, text);
    close_bench(b);
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL loading the parts\n");
        return 1;
    }
    pinouts();
    device("MAX6957AAX+", 4);
    device("MAX6957ATL+T", 4);
    device("MAX6957AAI+", 12);
    device("MAX6957ANI+", 12);
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("max6957_test: all passed\n");
    return 0;
}
