/*
 * tlc6983_cov_test (plan-68-R): the TLC6983's CCSI frames that tlc6983_test
 * does not send (datasheets/led-drivers/tlc6983.pdf; the wire helpers are
 * tlc6983_test's): FC10 / FC11 written and read back (AA0A / AA0B, read
 * AA6A / AA6B), the chip index read (AA70) after CHIP_INDEX (AA10), a read
 * of an ID the device does not have (no answer), a frame whose command word
 * has a wrong check bit (dropped), and unknown levels on the wire: an X on
 * SIN is sampled as a high (SIN idles high), and an SCLK that was unknown
 * and settles is an edge (one cannot be ruled out). And the scan: fewer
 * scan lines (FC0 SCAN_NUM, at VSYNC) while a later line is on restart the
 * scan at LINE0.
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

static void fc10_fc11(void) {
    Bench *b = bench();
    char what[96];
    uint64_t v;

    check(rd(b, 0xAA6A) == 0 && rd(b, 0xAA6B) == 0, "FC10 and FC11 reset to 0");
    send1(b, 0xAA0A, 0x15);
    send1(b, 0xAA0B, 0xFFFFFFFFFFEAull);
    v = rd(b, 0xAA6A);
    snprintf(what, sizeof what, "FC10 written 15h reads back 15h (read %012llX)", (unsigned long long)v);
    check(v == 0x15, what);
    v = rd(b, 0xAA6B);
    snprintf(what, sizeof what, "FC11 keeps only its 5 bits: written ..FFEAh reads 0Ah (read %012llX)",
             (unsigned long long)v);
    check(v == 0x0A, what);
    close_bench(b);
}

static void index_and_unknown_read(void) {
    Bench *b = bench();
    uint64_t v = 0;
    unsigned echo = 0;

    send(b, 0xAA10, NULL, 0);
    check(read_reg(b, 0xAA70, &v, &echo) && echo == 0xAA70 && v == 0,
          "after CHIP_INDEX the one chip on the chain reads index 0 (AA70)");
    check(!read_reg(b, 0xAA65, &v, &echo), "a read of AA65 (no such register) gets no answer on SOUT");
    close_bench(b);
}

static void bad_command_check(void) {
    Bench *b = bench();

    tick(b, 0);
    word(b, 0xAA03, 1);                    /* the command word's check bit is wrong */
    word(b, 0x0001, 0);
    word(b, 0x0203, 0);
    word(b, 0x0405, 0);
    end(b);
    check(((rd(b, 0xAA63) >> 8) & 0xFFFFFFu) == 0x7F7F7Fu, "a frame whose command word has a wrong check bit "
          "is dropped: FC3 keeps its reset CC values");
    close_bench(b);
}

/* SIN unknown at one sample: the bit reads high. */
static void sin_unknown(void) {
    Bench *b = bench();
    int i;
    uint64_t v;

    tick(b, 0);
    word(b, 0xAA03, 0);
    word(b, 0x003B, 0);
    word(b, 0x54A9, 0);
    /* R word 7F00h with its MSB sent as X: read as 1, so FF00h */
    set(b, "SIN", LOGIC_X);
    run(b, 20 * NS);
    b->sclk = b->sclk == LOGIC_0 ? LOGIC_1 : LOGIC_0;
    set(b, "SCLK", b->sclk);
    run(b, 20 * NS);
    for (i = 14; i >= 0; i--) {
        tick(b, (0x7F00 >> i) & 1);
    }
    tick(b, 1);                            /* check bit: NOT the last bit (0) */
    end(b);
    v = rd(b, 0xAA63);
    check(v == 0x003B54A9FF00ull, "an X on SIN is sampled as 1: R word 7F00h with an X MSB writes FF00h");
    close_bench(b);
}

/* SCLK unknown, then back at its old level: that is an edge, so the START
   on SIN is taken there and the frame that follows is written. */
static void sclk_unknown(void) {
    Bench *b = bench();
    uint64_t v;

    set(b, "SIN", LOGIC_0);
    run(b, 20 * NS);
    set(b, "SCLK", LOGIC_X);
    run(b, 20 * NS);
    set(b, "SCLK", b->sclk);               /* back to the level it had */
    run(b, 20 * NS);
    word(b, 0xAA03, 0);
    word(b, 0x003B, 0);
    word(b, 0x54A9, 0);
    word(b, 0xFF00, 0);
    end(b);
    v = rd(b, 0xAA63);
    check(v == 0x003B54A9FF00ull, "SCLK X then its old level is an edge: the START is taken and FC3 is written");
    close_bench(b);
}

/* SCAN_NUM from 15 (16 lines) to 1 (2 lines) while LINE10 is on: the scan goes back to LINE0. */
static void fewer_lines(void) {
    Bench *b = bench();
    int t, on10 = 0, ok = 1;

    for (t = 0; t < 4000 && !on10; t++) {
        run(b, 1 * US);
        on10 = get(b, "LINE10") == LOGIC_0;
    }
    check(on10, "the 16-line scan reaches LINE10");
    send1(b, 0xAA00, 0x100070010107ull);   /* FC0 with SCAN_NUM = 1 */
    send(b, 0xAAF0, NULL, 0);              /* VSYNC: FC0 applies */
    for (t = 0; t < 200; t++) {
        run(b, 1 * US);
        if (get(b, "LINE10") == LOGIC_0 || get(b, "LINE2") == LOGIC_0) {
            ok = 0;
        }
    }
    check(ok, "after SCAN_NUM 1 at VSYNC only LINE0 and LINE1 are scanned (LINE10 off at once, LINE2 never on)");
    close_bench(b);
}

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL loading the parts\n");
        return 1;
    }
    fc10_fc11();
    index_and_unknown_read();
    bad_command_check();
    sin_unknown();
    sclk_unknown();
    fewer_lines();
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("tlc6983_cov_test: all passed\n");
    return 0;
}
