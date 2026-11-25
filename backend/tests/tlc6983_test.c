/*
 * tlc6983_test (plan-56-D): the TLC6983 48 x 16 RGB matrix LED driver (datasheets/led-drivers/tlc6983.pdf)
 * through its real pins: the CCSI frame (START, 17-bit words with the check bit, 18 high END), the register
 * reset values and read-back, FC0 / FC1 applying at VSYNC, WRTGS writes through the line and channel counters
 * into the two banks, the bank swap on VSYNC, Soft_Reset, and the line scan with the R / G / B outputs.
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
    LogicValue sclk;
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

static Bench *bench(void) {
    Bench *b = calloc(1, sizeof *b);
    size_t i;

    b->part = part_library_find(library, "TLC6983ZXLR");
    if (b->part == NULL) {
        fprintf(stderr, "no TLC6983ZXLR\n");
        exit(2);
    }
    b->pins.count = b->part->pin_count;
    b->sim = sim_create();
    b->chip = sim_add_instance(b->sim, kind_model(b->part), "U1", b->part);
    b->drv = sim_add_instance(b->sim, &DRIVER, "TB", &b->pins);
    for (i = 0; i < b->part->pin_count; i++) {
        SimNet *net = sim_add_net(b->sim, b->part->pins[i].designator);

        sim_connect(b->sim, net, b->chip, i);
        sim_connect(b->sim, net, b->drv, i);
    }
    b->sclk = LOGIC_0;
    set(b, "SCLK", LOGIC_0);
    set(b, "SIN", LOGIC_1);
    if (!sim_start(b->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(b->sim));
        exit(2);
    }
    run(b, 1 * US);
    return b;
}

static void close_bench(Bench *b) {
    sim_destroy(b->sim);
    free(b);
}

/* ---- The wire --------------------------------------------------------------------------------- */

/* One bit: SIN set, then an SCLK edge (either); the chip samples SIN at the edge and shifts SOUT. Returns
   SOUT after the edge. */
static int tick(Bench *b, int bit) {
    set(b, "SIN", bit ? LOGIC_1 : LOGIC_0);
    run(b, 20 * NS);
    b->sclk = b->sclk == LOGIC_0 ? LOGIC_1 : LOGIC_0;
    set(b, "SCLK", b->sclk);
    run(b, 20 * NS);
    return get(b, "SOUT") == LOGIC_1;
}

static void word(Bench *b, unsigned w, int bad_check) {
    int i;

    for (i = 15; i >= 0; i--) {
        tick(b, (w >> i) & 1);
    }
    tick(b, (((w & 1u) == 0) ? 1 : 0) ^ bad_check);
}

static void end(Bench *b) {
    int i;

    for (i = 0; i < 18; i++) {
        tick(b, 1);
    }
}

/* A frame: START, the command, `units` 48-bit units (B word first), END. */
static void send(Bench *b, unsigned id, const uint64_t *units, int n) {
    int i;

    tick(b, 0);
    word(b, id, 0);
    for (i = 0; i < n; i++) {
        word(b, (unsigned)(units[i] >> 32) & 0xFFFFu, 0);
        word(b, (unsigned)(units[i] >> 16) & 0xFFFFu, 0);
        word(b, (unsigned)units[i] & 0xFFFFu, 0);
    }
    end(b);
}

static void send1(Bench *b, unsigned id, uint64_t v) {
    send(b, id, &v, 1);
}

static void send_bad(Bench *b, unsigned id, uint64_t v) {
    tick(b, 0);
    word(b, id, 0);
    word(b, (unsigned)(v >> 32) & 0xFFFFu, 0);
    word(b, (unsigned)(v >> 16) & 0xFFFFu, 1);     /* a wrong check bit */
    word(b, (unsigned)v & 0xFFFFu, 0);
    end(b);
}

/* A read: the command (head only), then idle clocks while SOUT answers: START, head, 3 words. */
static int read_reg(Bench *b, unsigned id, uint64_t *value, unsigned *echo) {
    uint64_t v = 0;
    unsigned head = 0, i;
    int sout;

    tick(b, 0);
    word(b, id, 0);
    end(b);
    /* the chip answers at the edges that follow */
    sout = tick(b, 1);
    if (sout != 0) {
        return 0;                                  /* no START */
    }
    for (i = 0; i < 17; i++) {
        sout = tick(b, 1);
        if (i < 16) {
            head = (head << 1) | (unsigned)sout;
        }
    }
    for (i = 0; i < 51; i++) {
        sout = tick(b, 1);
        if (i % 17 != 16) {
            v = (v << 1) | (uint64_t)sout;
        }
    }
    *value = v;
    *echo = head;
    return 1;
}

static uint64_t rd(Bench *b, unsigned id) {
    uint64_t v = 0xDEAD;
    unsigned echo = 0;

    if (!read_reg(b, id, &v, &echo) || echo != id) {
        return 0xBAD0BAD0BAD0ull;
    }
    return v;
}

/* ---- Tests ------------------------------------------------------------------------------------ */

static void pins(void) {
    const Part *p = part_library_find(library, "TLC6983ZXLR");
    size_t i, nc = 0, gnd = 0;
    int ok;

    check(p != NULL && p->pin_count == 96, "TLC6983ZXLR has 96 balls");
#define BALL(NAME, DES) (pin_of(p, NAME) != SIZE_MAX && strcmp(p->pins[pin_of(p, NAME)].designator, DES) == 0)
    ok = p != NULL && BALL("R0", "B5") && BALL("G0", "B4") && BALL("B0", "B3") && BALL("R15", "E8") &&
         BALL("G15", "F8") && BALL("B15", "G8") && BALL("LINE0", "D1") && BALL("LINE15", "C11") && BALL("IREF", "J1") &&
         BALL("VCC", "F1") && BALL("SCLK", "B9") && BALL("SIN", "B8") && BALL("SOUT", "B7");
    check(ok, "balls as p.3-4 (R0 B5, G0 B4, B0 B3, LINE0 D1, IREF J1, VCC F1, SCLK B9, SIN B8, SOUT B7)");
    for (i = 0; p != NULL && i < p->pin_count; i++) {
        nc += p->pins[i].role == PIN_NC;
        gnd += strcmp(p->pins[i].name, "GND") == 0;
    }
    check(nc == 2 && gnd == 19, "2 no-connect balls and 19 ground balls");
    check(p != NULL && p->vcc != NULL && p->vcc->min_mv == 2500 && p->vcc->max_mv == 5500, "supply 2.5-5.5 V (p.5)");
}

static void registers(void) {
    Bench *b = bench();
    uint64_t v;
    char what[96];

    v = rd(b, 0xAA60);
    snprintf(what, sizeof what, "FC0 reads its reset value 1000 700F 0107h (p.33; read %012llX)", (unsigned long long)v);
    check(v == 0x1000700F0107ull, what);
    v = rd(b, 0xAA61);
    snprintf(what, sizeof what, "FC1 reset: LINE_SWT 0111b, LG_STEP 01001b x3 (read %012llX)", (unsigned long long)v);
    check(v == ((7ull << 37) | (9ull << 20) | (9ull << 15) | (9ull << 10)), what);
    v = rd(b, 0xAA63);
    check(((v >> 8) & 0xFFFFFFu) == 0x7F7F7Fu, "FC3 reset: CC_R, CC_G and CC_B are 7Fh");
    check(((v >> 32) & 7u) == 3, "FC3 reset: BC = 011b");
    check(rd(b, 0xAA6C) == 0 && rd(b, 0xAA6F) == 0, "FC12 and FC15 (LED open / short flags) read 0");

    /* Table 9-4's example values */
    send1(b, 0xAA00, 0x1000583F0107ull);
    send1(b, 0xAA01, 0x2AE00094A631ull);
    send1(b, 0xAA03, 0x003B54A9FF00ull);
    v = rd(b, 0xAA63);
    snprintf(what, sizeof what, "FC3 written 003B 54A9 FF00h reads back (read %012llX)", (unsigned long long)v);
    check(v == 0x003B54A9FF00ull, what);
    v = rd(b, 0xAA60);
    check(v == 0x1000700F0107ull, "FC0 written, but still the old value before VSYNC (p.50)");
    send(b, 0xAAF0, NULL, 0);
    v = rd(b, 0xAA60);
    snprintf(what, sizeof what, "after VSYNC FC0 is 1000 583F 0107h (read %012llX)", (unsigned long long)v);
    check(v == 0x1000583F0107ull, what);
    v = rd(b, 0xAA61);
    check(v == 0x2AE00094A631ull, "after VSYNC FC1 is 2AE0 0094 A631h");

    send_bad(b, 0xAA03, 0xFFFFFFFFFFFFull);
    v = rd(b, 0xAA63);
    check(v == 0x003B54A9FF00ull, "a frame with a wrong check bit changes nothing");

    send(b, 0xAA80, NULL, 0);
    v = rd(b, 0xAA63);
    snprintf(what, sizeof what, "Soft_Reset restores FC3 (read %012llX)", (unsigned long long)v);
    check(((v >> 8) & 0xFFFFFFu) == 0x7F7F7Fu && ((v >> 32) & 7u) == 3, what);
    close_bench(b);
}

/* Waits (in 1 us steps, up to 3 frames) until LINE n is low; 1 when it is. */
static int wait_line(Bench *b, int n) {
    char name[16];
    int i;

    snprintf(name, sizeof name, "LINE%d", n);
    /* a slot is computed when it starts: wait for the line to be released first, so the
       slot read is a new one (the current one may predate a write or a VSYNC) */
    for (i = 0; i < 700 && get(b, name) == LOGIC_0; i++) {
        run(b, 1 * US);
    }
    for (i = 0; i < 700; i++) {
        if (get(b, name) == LOGIC_0) {
            return 1;
        }
        run(b, 1 * US);
    }
    return 0;
}

static int lit(Bench *b, char colour, int pixel) {
    char name[8];

    snprintf(name, sizeof name, "%c%d", colour, pixel);
    return get(b, name) == LOGIC_1;
}

static int others_off(Bench *b, int line) {
    int l, ok = 1;
    char name[16];

    for (l = 0; l < 16; l++) {
        snprintf(name, sizeof name, "LINE%d", l);
        if (l != line) {
            ok &= get(b, name) == LOGIC_Z;
        }
    }
    return ok;
}

static void display(void) {
    Bench *b = bench();
    int i, found;
    char what[96];

    /* two lines (SCAN_NUM = 1: FC0 applies at the VSYNC); every unit is its own WRTGS frame */
    send1(b, 0xAA00, (0x1000700F0107ull & ~(0x1Full << 16)) | (1ull << 16));
    send(b, 0xAAF0, NULL, 0);
    for (i = 0; i < 32; i++) {
        uint64_t u = 0;

        if (i == 3) {
            u = 0x1;                                    /* line 0, pixel 3: R = 1 */
        } else if (i == 7) {
            u = 0x8000ull << 16;                        /* line 0, pixel 7: G = 8000h */
        } else if (i == 16 + 5) {
            u = 0xFFFFull << 32;                        /* line 1, pixel 5: B = FFFFh */
        } else if (i == 16 + 9) {
            u = 0x0001ull | (0x0001ull << 16) | (0x0001ull << 32);   /* line 1, pixel 9: white */
        }
        send1(b, 0xAA30, u);
    }
    found = wait_line(b, 0);
    check(found, "before the VSYNC the written bank is not shown: line 0 still scans");
    check(!lit(b, 'R', 3) && !lit(b, 'G', 7), "before the VSYNC: no pixel is lit (the other bank is displayed)");
    send(b, 0xAAF0, NULL, 0);
    found = wait_line(b, 0);
    check(found, "after the VSYNC line 0 is pulled low");
    snprintf(what, sizeof what, "line 0: R3 and G7 lit (R3 %d G7 %d B5 %d)", lit(b, 'R', 3), lit(b, 'G', 7), lit(b, 'B', 5));
    check(lit(b, 'R', 3) && lit(b, 'G', 7) && !lit(b, 'B', 5) && !lit(b, 'R', 7), what);
    check(others_off(b, 0), "line 0 low: the other 15 lines are released");
    found = wait_line(b, 1);
    snprintf(what, sizeof what, "line 1: B5 and white pixel 9 lit (B5 %d R9 %d G9 %d B9 %d)", lit(b, 'B', 5), lit(b, 'R', 9),
             lit(b, 'G', 9), lit(b, 'B', 9));
    check(found && lit(b, 'B', 5) && lit(b, 'R', 9) && lit(b, 'G', 9) && lit(b, 'B', 9) && !lit(b, 'R', 3), what);
    check(others_off(b, 1), "line 1 low: the other 15 lines are released");
    {
        int ever = 0, k;

        for (k = 0; k < 200; k++) {
            char name[16];
            int l;

            for (l = 2; l < 16; l++) {
                snprintf(name, sizeof name, "LINE%d", l);
                ever |= get(b, name) == LOGIC_0;
            }
            run(b, 1 * US);
        }
        check(!ever, "SCAN_NUM = 1 scans two lines: LINE2-LINE15 are never low");
    }

    /* the next frame goes into the other bank and shows after the next VSYNC */
    for (i = 0; i < 32; i++) {
        send1(b, 0xAA30, i == 0 ? 0x2ull : 0);          /* line 0, pixel 0: R = 2 */
    }
    wait_line(b, 0);
    check(!lit(b, 'R', 0) && lit(b, 'R', 3), "the next frame is written to the other bank: the shown frame stays");
    send(b, 0xAAF0, NULL, 0);
    wait_line(b, 0);
    check(lit(b, 'R', 0) && !lit(b, 'R', 3), "the next VSYNC swaps the banks: the new frame shows");
    close_bench(b);
}

static void counters(void) {
    Bench *b = bench();
    int i;
    char name[16];

    /* default 16 lines: line 15, pixel 15 is the 256th unit */
    send(b, 0xAAF0, NULL, 0);                            /* bank 1 receives */
    for (i = 0; i < 256; i++) {
        send1(b, 0xAA30, i == 255 ? 0x1ull << 32 : 0);
    }
    send(b, 0xAAF0, NULL, 0);
    wait_line(b, 15);
    snprintf(name, sizeof name, "B15");
    check(lit(b, 'B', 15) && !lit(b, 'B', 14) && !lit(b, 'R', 15), "256 units: the last (line 15, pixel 15) lands on line 15 pixel 15");
    wait_line(b, 0);
    check(!lit(b, 'B', 15), "... and nowhere else (line 0 shows nothing)");
    close_bench(b);
}

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL loading the parts\n");
        return 1;
    }
    pins();
    registers();
    display();
    counters();
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("tlc6983_test: all passed\n");
    return 0;
}
