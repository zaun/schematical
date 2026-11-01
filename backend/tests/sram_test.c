/*
 * sram_test: the SRAM model against its datasheet timing, on the part
 * library (port of the old sram_check.c bench: power-on X, write and read
 * cycles, the read output window, write timing violations storing X, byte
 * lanes). Parts come from sram.json; pins resolve through Part.pins; memory
 * is read through the model's memory_read hook. The catalog and size-label
 * checks are covered by listing_check; the netlist inspect check moved to
 * circuit_check.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "parts.h"
#include "read_file.h"
#include "sim.h"

#define NS(value) ((SimTime)((value) * 1000))

static int failures = 0;
static PartLibrary *library;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

/* ---- Test bench: one driver pin wired to every SRAM pin ------------------ */

static size_t bench_pin_count(const void *config) {
    return *(const size_t *)config;
}

static const SimComponentType BENCH_TYPE = {
    "bench", bench_pin_count, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

typedef struct {
    Sim *sim;
    SimInstance *sram;
    SimInstance *bench;
    const Part *part;
    const char *data_prefix; /* "DQ" or "I/O" */
    unsigned data_base;      /* number of data bit 0 in the pin names */
    unsigned data_bits;
} Bench;

/* 0-based pin index of a named pin. */
static size_t pin_named(const Bench *bench, const char *name) {
    size_t i;

    for (i = 0; i < bench->part->pin_count; i++) {
        if (strcmp(bench->part->pins[i].name, name) == 0) {
            return bench->part->pins[i].number - 1;
        }
    }
    fprintf(stderr, "no pin %s on %s\n", name, bench->part->name);
    exit(2);
}

static int bench_open(Bench *bench, const char *part, const char *data_prefix,
                      unsigned data_base, unsigned data_bits) {
    static size_t pins;
    size_t i;

    memset(bench, 0, sizeof(*bench));
    bench->part = part_library_find(library, part);
    if (bench->part == NULL || bench->part->kind != PART_SRAM) {
        return 0;
    }
    pins = bench->part->pin_count;
    bench->data_prefix = data_prefix;
    bench->data_base = data_base;
    bench->data_bits = data_bits;
    bench->sim = sim_create();
    bench->sram = sim_add_instance(bench->sim, kind_model(bench->part), "U1", bench->part);
    bench->bench = sim_add_instance(bench->sim, &BENCH_TYPE, "TB", &pins);
    for (i = 0; i < pins; i++) {
        SimNet *net = sim_add_net(bench->sim, "n");

        sim_connect(bench->sim, net, bench->sram, i);
        sim_connect(bench->sim, net, bench->bench, i);
    }
    return sim_start(bench->sim);
}

static void bench_close(Bench *bench) {
    sim_destroy(bench->sim);
}

static void set_pin(Bench *bench, const char *name, LogicValue value) {
    sim_schedule_drive(bench->sim, bench->bench, pin_named(bench, name), value, DRIVE_STRONG, 0);
}

static void set_address(Bench *bench, unsigned bits, unsigned long address) {
    unsigned i;
    char name[16];

    for (i = 0; i < bits; i++) {
        snprintf(name, sizeof(name), "A%u", i);
        set_pin(bench, name, (address >> i) & 1 ? LOGIC_1 : LOGIC_0);
    }
}

static void data_name(const Bench *bench, unsigned bit, char *name, size_t size) {
    snprintf(name, size, "%s%u", bench->data_prefix, bit + bench->data_base);
}

/* Drives the data bits in mask; the others are released (Z). */
static void set_data(Bench *bench, uint32_t mask, uint32_t value) {
    unsigned i;
    char name[16];

    for (i = 0; i < bench->data_bits; i++) {
        data_name(bench, i, name, sizeof(name));
        set_pin(bench, name, !(mask >> i & 1) ? LOGIC_Z : (value >> i) & 1 ? LOGIC_1 : LOGIC_0);
    }
}

static void release_data(Bench *bench) {
    set_data(bench, 0, 0);
}

static void run_to(Bench *bench, SimTime time) {
    if (!sim_run_until(bench->sim, time)) {
        fprintf(stderr, "sim stopped: %s\n", sim_error(bench->sim));
    }
}

/* The data bus as one character per bit, high bit first: 0, 1, X or Z. */
static const char *bus(Bench *bench) {
    static char text[40];
    unsigned i;
    char name[16];

    for (i = 0; i < bench->data_bits; i++) {
        LogicValue v;

        data_name(bench, i, name, sizeof(name));
        v = sim_read_pin(bench->sim, bench->sram, pin_named(bench, name));
        text[bench->data_bits - 1 - i] = "01XZ"[v];
    }
    text[bench->data_bits] = '\0';
    return text;
}

static int bench_memory_size(Bench *bench, uint32_t *words, unsigned *bits) {
    const SimComponentType *type = sim_instance_type(bench->sram);

    return type->memory_size(sim_instance_state(bench->sram), words, bits);
}

static void word_at(Bench *bench, uint32_t address, uint32_t *value, uint32_t *known) {
    const SimComponentType *type = sim_instance_type(bench->sram);

    if (type->memory_read(sim_instance_state(bench->sram), address, 1, value, known) != 1) {
        *value = 0;
        *known = 0;
    }
}

/* ---- AS6C62256 (32K x 8, 55 ns) ------------------------------------------ */

/*
 * A WE#-controlled write: address at start, WE# low 5 ns later (so the
 * address is valid for tAW = 50 ns before a 45 ns pulse ends), data at
 * data_at, WE# high after `pulse`.
 */
static void write_cycle(Bench *bench, SimTime start, uint32_t address, uint32_t value,
                        SimTime data_at, SimTime pulse) {
    run_to(bench, start);
    set_address(bench, 15, address);
    start += NS(5);
    run_to(bench, start);
    set_pin(bench, "WE#", LOGIC_0);
    run_to(bench, data_at);
    set_data(bench, 0xFF, value);
    run_to(bench, start + pulse);
    set_pin(bench, "WE#", LOGIC_1);
    run_to(bench, start + pulse + NS(10));
    release_data(bench);
}

static void check_as6c62256(void) {
    Bench b;
    uint32_t value;
    uint32_t known;
    uint32_t count;
    unsigned bits;
    SimTime t;

    expect(bench_open(&b, "AS6C62256-55PCN", "DQ", 0, 8), "AS6C62256-55PCN resolves and starts");

    expect(bench_memory_size(&b, &count, &bits) && count == 32768 && bits == 8,
           "AS6C62256 is 32768 words of 8 bits");
    word_at(&b, 0, &value, &known);
    expect(known == 0, "power-on contents are X");

    /* Deselected, outputs off. */
    set_pin(&b, "CE#", LOGIC_1);
    set_pin(&b, "OE#", LOGIC_1);
    set_pin(&b, "WE#", LOGIC_1);
    set_address(&b, 15, 0);
    run_to(&b, NS(100));
    expect(strcmp(bus(&b), "ZZZZZZZZ") == 0, "deselected outputs are high-Z");

    /* A good write: tWP 45, tDW 25, address set up for tAW. */
    set_pin(&b, "CE#", LOGIC_0);
    write_cycle(&b, NS(200), 0x1234, 0xA5, NS(205), NS(45));
    word_at(&b, 0x1234, &value, &known);
    expect(known == 0xFF && value == 0xA5, "write at minimum tWP stores the data");

    /* tWP 1 ps short. */
    write_cycle(&b, NS(400), 0x0042, 0x3C, NS(405), NS(45) - 1);
    word_at(&b, 0x0042, &value, &known);
    expect(known == 0, "write pulse shorter than tWP stores X");

    /* Address set at the same time as WE# falls: tAW (50) > tWP (45) is missed. */
    run_to(&b, NS(500));
    set_address(&b, 15, 0x0043);
    set_data(&b, 0xFF, 0x99);
    set_pin(&b, "WE#", LOGIC_0);
    run_to(&b, NS(545));
    set_pin(&b, "WE#", LOGIC_1);
    run_to(&b, NS(560));
    release_data(&b);
    word_at(&b, 0x0043, &value, &known);
    expect(known == 0, "address valid for less than tAW stores X");

    /* tDW violated on bit 0 only: bits 1..7 set early, bit 0 late. */
    run_to(&b, NS(590));
    set_address(&b, 15, 0x0100);
    run_to(&b, NS(600));
    set_data(&b, 0xFF, 0xF0);
    set_pin(&b, "WE#", LOGIC_0);
    run_to(&b, NS(600) + NS(45) - NS(24));
    set_data(&b, 0xFF, 0xF1);
    run_to(&b, NS(600) + NS(45));
    set_pin(&b, "WE#", LOGIC_1);
    run_to(&b, NS(700));
    release_data(&b);
    word_at(&b, 0x0100, &value, &known);
    expect(known == 0xFE && (value & 0xFE) == 0xF0, "data setup shorter than tDW stores X in that bit");

    /* Data and WE# changing at the same instant: tDH = 0, the old data is stored. */
    run_to(&b, NS(800));
    set_address(&b, 15, 0x0101);
    set_data(&b, 0xFF, 0x5A);
    set_pin(&b, "WE#", LOGIC_0);
    run_to(&b, NS(860));
    set_pin(&b, "WE#", LOGIC_1);
    set_data(&b, 0xFF, 0x00);
    run_to(&b, NS(900));
    release_data(&b);
    word_at(&b, 0x0101, &value, &known);
    expect(known == 0xFF && value == 0x5A, "data changing with the end of write (tDH = 0) keeps the old data");

    /* Address moving inside a write corrupts both locations. */
    write_cycle(&b, NS(1000), 0x0200, 0x11, NS(1005), NS(50));
    write_cycle(&b, NS(1100), 0x0201, 0x22, NS(1105), NS(50));
    {
        uint32_t k0, k1;

        word_at(&b, 0x0200, &value, &k0);
        word_at(&b, 0x0201, &value, &k1);
        expect(k0 == 0xFF && k1 == 0xFF, "two good writes before the address-change test");
    }
    run_to(&b, NS(1200));
    set_address(&b, 15, 0x0200);
    set_data(&b, 0xFF, 0x77);
    set_pin(&b, "WE#", LOGIC_0);
    run_to(&b, NS(1220));
    set_address(&b, 15, 0x0201);
    run_to(&b, NS(1300));
    set_pin(&b, "WE#", LOGIC_1);
    run_to(&b, NS(1310));
    release_data(&b);
    {
        uint32_t k0, k1;

        word_at(&b, 0x0200, &value, &k0);
        word_at(&b, 0x0201, &value, &k1);
        expect(k0 == 0 && k1 == 0, "address change during a write stores X at both addresses");
    }

    /* Read window: OE# low at t with the address already stable. */
    run_to(&b, NS(2000));
    set_address(&b, 15, 0x1234);
    run_to(&b, NS(2200));
    t = NS(2200);
    set_pin(&b, "OE#", LOGIC_0);
    run_to(&b, t + NS(5) - 1);
    expect(strcmp(bus(&b), "ZZZZZZZZ") == 0, "outputs stay high-Z until tOLZ");
    run_to(&b, t + NS(5));
    expect(strcmp(bus(&b), "XXXXXXXX") == 0, "outputs drive X from tOLZ until tOE");
    run_to(&b, t + NS(30) - 1);
    expect(strcmp(bus(&b), "XXXXXXXX") == 0, "data is not valid before tOE");
    run_to(&b, t + NS(30));
    expect(strcmp(bus(&b), "10100101") == 0, "data is valid at tOE");

    /* Address change: old data for tOH, X, new data at tAA. */
    t = NS(2400);
    run_to(&b, t);
    set_address(&b, 15, 0x0101);
    run_to(&b, t + NS(10) - 1);
    expect(strcmp(bus(&b), "10100101") == 0, "old data holds for tOH after an address change");
    run_to(&b, t + NS(10));
    expect(strcmp(bus(&b), "XXXXXXXX") == 0, "data is X after tOH");
    run_to(&b, t + NS(55) - 1);
    expect(strcmp(bus(&b), "XXXXXXXX") == 0, "new data is not valid before tAA");
    run_to(&b, t + NS(55));
    expect(strcmp(bus(&b), "01011010") == 0, "new data is valid at tAA");

    /* Reading a location never written. */
    t = NS(2600);
    run_to(&b, t);
    set_address(&b, 15, 0x7FFF);
    run_to(&b, t + NS(60));
    expect(strcmp(bus(&b), "XXXXXXXX") == 0, "reading unwritten memory gives X");

    /* OE# high: X until tOHZ, then high-Z. */
    set_address(&b, 15, 0x1234);
    run_to(&b, NS(2800));
    t = NS(2800);
    set_pin(&b, "OE#", LOGIC_1);
    run_to(&b, t + 1);
    expect(strcmp(bus(&b), "XXXXXXXX") == 0, "outputs are X while turning off");
    run_to(&b, t + NS(20));
    expect(strcmp(bus(&b), "ZZZZZZZZ") == 0, "outputs are high-Z at tOHZ");

    /* Write with OE# low: the chip lets go of the bus tWHZ after WE# falls. */
    run_to(&b, NS(3000));
    set_pin(&b, "OE#", LOGIC_0);
    run_to(&b, NS(3100));
    expect(strcmp(bus(&b), "10100101") == 0, "reading again with OE# low");
    t = NS(3200);
    run_to(&b, t);
    set_pin(&b, "WE#", LOGIC_0);
    run_to(&b, t + NS(20));
    expect(strcmp(bus(&b), "ZZZZZZZZ") == 0, "WE# low turns the outputs off at tWHZ");
    set_data(&b, 0xFF, 0x0F);
    run_to(&b, t + NS(45));
    set_pin(&b, "WE#", LOGIC_1);
    run_to(&b, t + NS(46));
    release_data(&b);
    run_to(&b, t + NS(100));
    word_at(&b, 0x1234, &value, &known);
    expect(known == 0xFF && value == 0x0F, "WE#-controlled write with OE# low stores the data");
    expect(strcmp(bus(&b), "00001111") == 0, "outputs return with the new data after the write");

    /* CE# floating (Z) reads as deselected. */
    set_pin(&b, "CE#", LOGIC_Z);
    run_to(&b, NS(3500));
    expect(strcmp(bus(&b), "ZZZZZZZZ") == 0, "an undriven CE# deselects the chip");

    /* X on WE# while selected: the addressed word becomes X. */
    set_pin(&b, "CE#", LOGIC_0);
    run_to(&b, NS(3600));
    set_pin(&b, "WE#", LOGIC_X);
    run_to(&b, NS(3700));
    set_pin(&b, "WE#", LOGIC_1);
    run_to(&b, NS(3800));
    word_at(&b, 0x1234, &value, &known);
    expect(known == 0, "an unknown WE# stores X at the addressed word");

    bench_close(&b);
}

/* ---- AS7C34098A (256K x 16, byte lanes, tWP1/tWP2) ----------------------- */

static void check_as7c34098a(void) {
    Bench b;
    uint32_t value;
    uint32_t known;

    expect(bench_open(&b, "AS7C34098A-10JCN", "I/O", 1, 16), "AS7C34098A-10JCN resolves and starts");

    set_pin(&b, "CE#", LOGIC_0);
    set_pin(&b, "OE#", LOGIC_1);
    set_pin(&b, "WE#", LOGIC_1);
    set_pin(&b, "LB#", LOGIC_1);
    set_pin(&b, "UB#", LOGIC_1);
    set_address(&b, 18, 0);
    run_to(&b, NS(100));

    /* Lower byte only. */
    set_address(&b, 18, 0x2ABCD);
    set_data(&b, 0xFFFF, 0xBEEF);
    set_pin(&b, "LB#", LOGIC_0);
    set_pin(&b, "WE#", LOGIC_0);
    run_to(&b, NS(110));
    set_pin(&b, "WE#", LOGIC_1);
    set_pin(&b, "LB#", LOGIC_1);
    run_to(&b, NS(120));
    word_at(&b, 0x2ABCD, &value, &known);
    expect(known == 0x00FF && (value & 0xFF) == 0xEF, "LB# alone writes only the low byte");

    /* OE# high: tWP1 = 7 ns is enough. */
    run_to(&b, NS(200));
    set_address(&b, 18, 0x00010);
    set_pin(&b, "LB#", LOGIC_0);
    set_pin(&b, "UB#", LOGIC_0);
    set_pin(&b, "WE#", LOGIC_0);
    run_to(&b, NS(207));
    set_pin(&b, "WE#", LOGIC_1);
    run_to(&b, NS(220));
    word_at(&b, 0x00010, &value, &known);
    expect(known == 0xFFFF && value == 0xBEEF, "7 ns write pulse with OE# high meets tWP1");

    /* OE# low: the same 7 ns pulse misses tWP2 = 10 ns. */
    release_data(&b);
    set_pin(&b, "OE#", LOGIC_0);
    run_to(&b, NS(300));
    set_address(&b, 18, 0x00011);
    set_pin(&b, "WE#", LOGIC_0);
    run_to(&b, NS(305));
    set_data(&b, 0xFFFF, 0x1234);
    run_to(&b, NS(307));
    set_pin(&b, "WE#", LOGIC_1);
    run_to(&b, NS(320));
    word_at(&b, 0x00011, &value, &known);
    expect(known == 0, "7 ns write pulse with OE# low misses tWP2 and stores X");

    /* Byte-enable read: UB# high leaves the upper byte high-Z. */
    release_data(&b);
    set_pin(&b, "UB#", LOGIC_1);
    set_address(&b, 18, 0x00010);
    run_to(&b, NS(400));
    expect(strcmp(bus(&b), "ZZZZZZZZ11101111") == 0, "UB# high reads only the low byte");

    bench_close(&b);
}

/* The hooks report the size; past the end reads nothing. */
static void check_memory_hooks(void) {
    Bench b;
    const SimComponentType *type;
    uint32_t words = 0;
    unsigned bits = 0;
    uint32_t values[4] = {1, 1, 1, 1};
    uint32_t known[4] = {1, 1, 1, 1};

    expect(bench_open(&b, "AS6C62256-55PCN", "DQ", 0, 8), "AS6C62256-55PCN resolves and starts");
    type = sim_instance_type(b.sram);
    expect(type->memory_size(sim_instance_state(b.sram), &words, &bits) && words == 32768 &&
               bits == 8,
           "memory_size reports 32768 x 8");
    expect(type->memory_read(sim_instance_state(b.sram), 32766, 4, values, known) == 2,
           "memory_read clips at the end of memory");
    expect(known[0] == 0 && known[1] == 0, "power-on words are X");
    expect(type->memory_read(sim_instance_state(b.sram), 32768, 1, values, known) == 0,
           "memory_read past the end reads nothing");
    bench_close(&b);
}

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        return 1;
    }
    check_as6c62256();
    check_as7c34098a();
    check_memory_hooks();
    part_library_destroy(library);
    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
