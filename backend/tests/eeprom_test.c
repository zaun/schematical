/*
 * eeprom_test: the eeprom model against the datasheet rules (plan-04-A
 * section 5.4): erased power-up and the read window, byte and page loads,
 * the tBLC window and tWC write cycle with DATA# polling and the toggle bit,
 * timing violations storing X, mixed pages, software data protection, an
 * unknown WE#, and the memory hooks.
 *
 * Cases 1-9 run on an inline fixture (the AT28C256 device: grades 15 and
 * 15F, one 28-pin pinout); the rest on parts from eeprom.json.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "parts.h"
#include "read_file.h"
#include "sim.h"

#define NS(value) ((SimTime)((value) * 1000ull))
#define US(value) (NS(value) * 1000ull)
#define MS(value) (US(value) * 1000ull)

/* AT28C256 timing, grade 15. */
#define TBLC US(150)
#define TWC MS(10)

static int failures = 0;
static PartLibrary *library;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

/* ---- The fixture --------------------------------------------------------- */

#define PIN(N, NAME, ROLE) "{'number':" #N ",'name':'" NAME "','role':'" ROLE "'}"
#define ADDR(N, NAME) "{'number':" #N ",'name':'" NAME "','role':'input','group':'address'}"

static const char FIXTURE[] =
    "{'format':'schematical-parts','version':1,'kind':'eeprom','devices':["
    "{'id':'FIXTURE-AT28C256','function':'32 KB','description':'32K x 8 paged parallel EEPROM',"
    "'drawer':['Memory','EEPROM'],'note':'eeprom_test fixture: the AT28C256 device',"
    "'pinouts':{'p28':["
    ADDR(1, "A14") "," ADDR(2, "A12") "," ADDR(3, "A7") "," ADDR(4, "A6") "," ADDR(5, "A5") ","
    ADDR(6, "A4") "," ADDR(7, "A3") "," ADDR(8, "A2") "," ADDR(9, "A1") "," ADDR(10, "A0") ","
    PIN(11, "I/O0", "inout") "," PIN(12, "I/O1", "inout") "," PIN(13, "I/O2", "inout") ","
    PIN(14, "GND", "ground") "," PIN(15, "I/O3", "inout") "," PIN(16, "I/O4", "inout") ","
    PIN(17, "I/O5", "inout") "," PIN(18, "I/O6", "inout") "," PIN(19, "I/O7", "inout") ","
    PIN(20, "CE#", "input") "," ADDR(21, "A10") "," PIN(22, "OE#", "input") "," ADDR(23, "A11") ","
    ADDR(24, "A9") "," ADDR(25, "A8") "," ADDR(26, "A13") "," PIN(27, "WE#", "input") ","
    PIN(28, "VCC", "power") "]},"
    "'address':['A0','A1','A2','A3','A4','A5','A6','A7','A8','A9','A10','A11','A12','A13','A14'],"
    "'data':['I/O0','I/O1','I/O2','I/O3','I/O4','I/O5','I/O6','I/O7'],"
    "'chipEnable':'CE#','outputEnable':'OE#','writeEnable':'WE#',"
    "'pageSize':64,'commandAddresses':['5555','2AAA'],'protection':'off',"
    "'grades':{"
    "'15':{'tacc':150,'tce':150,'toe':70,'tdf':50,'tah':50,'twp':100,'tds':50,'twph':50,"
    "'tblc':150000,'twc':10000000},"
    "'15F':{'tacc':150,'tce':150,'toe':70,'tdf':50,'tah':50,'twp':100,'tds':50,'twph':50,"
    "'tblc':150000,'twc':3000000}},"
    "'parts':["
    "{'name':'FIXTURE-15','package':'PDIP','pinout':'p28','footprint':'PDIP-28-600','grade':'15'},"
    "{'name':'FIXTURE-15F','package':'PDIP','pinout':'p28','footprint':'PDIP-28-600','grade':'15F'}"
    "]}]}";

static int load_fixture(void) {
    char *text = malloc(sizeof FIXTURE);
    char *error = NULL;
    size_t i;
    int result;

    for (i = 0; i < sizeof FIXTURE; i++) {
        text[i] = FIXTURE[i] == '\'' ? '"' : FIXTURE[i];
    }
    result = load_parts(library, text, "eeprom_fixture.json", &error);
    if (result != 0) {
        fprintf(stderr, "fixture: %s\n", error != NULL ? error : "(no message)");
    }
    free(error);
    free(text);
    return result;
}

/* ---- Test bench: one driver pin wired to every EEPROM pin ---------------- */

static size_t bench_pin_count(const void *config) {
    return *(const size_t *)config;
}

static const SimComponentType BENCH_TYPE = {
    "bench", bench_pin_count, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

typedef struct {
    Sim *sim;
    SimInstance *chip;
    SimInstance *bench;
    const Part *part;
    unsigned address_bits;
    size_t pins;
} Bench;

static size_t pin_named(const Bench *bench, const char *name) {
    size_t i;

    for (i = 0; i < bench->part->pin_count; i++) {
        if (strcmp(bench->part->pins[i].name, name) == 0) {
            return i;
        }
    }
    fprintf(stderr, "no pin %s on %s\n", name, bench->part->name);
    exit(2);
}

static void set_pin(Bench *bench, const char *name, LogicValue value) {
    sim_schedule_drive(bench->sim, bench->bench, pin_named(bench, name), value, DRIVE_STRONG, 0);
}

static void run_to(Bench *bench, SimTime time) {
    if (!sim_run_until(bench->sim, time)) {
        fprintf(stderr, "sim stopped: %s\n", sim_error(bench->sim));
    }
}

static void set_address(Bench *bench, uint32_t address) {
    unsigned i;
    char name[16];

    for (i = 0; i < bench->address_bits; i++) {
        snprintf(name, sizeof(name), "A%u", i);
        set_pin(bench, name, (address >> i) & 1 ? LOGIC_1 : LOGIC_0);
    }
}

/* Drives the data bus, or releases it (Z) when drive is 0. */
static void set_data(Bench *bench, int drive, uint8_t value) {
    unsigned i;
    char name[16];

    for (i = 0; i < 8; i++) {
        snprintf(name, sizeof(name), "I/O%u", i);
        set_pin(bench, name, !drive ? LOGIC_Z : (value >> i) & 1 ? LOGIC_1 : LOGIC_0);
    }
}

/* The data bus, high bit first: 0, 1, X or Z per bit. */
static const char *bus(Bench *bench) {
    static char text[9];
    unsigned i;
    char name[16];

    for (i = 0; i < 8; i++) {
        snprintf(name, sizeof(name), "I/O%u", i);
        text[7 - i] = "01XZ"[sim_read_pin(bench->sim, bench->chip, pin_named(bench, name))];
    }
    text[8] = '\0';
    return text;
}

/* Opens a bench at power-up with CE#, OE#, WE# high and the bus released. */
static int bench_open(Bench *bench, const char *name) {
    size_t i;

    memset(bench, 0, sizeof(*bench));
    bench->part = part_library_find(library, name);
    if (bench->part == NULL || bench->part->kind != PART_EEPROM) {
        fprintf(stderr, "no eeprom part %s\n", name);
        exit(2);
    }
    bench->pins = bench->part->pin_count;
    for (i = 0; i < 24; i++) {
        bench->address_bits = (bench->part->eeprom->words >> i) == 1 ? (unsigned)i : bench->address_bits;
    }
    bench->sim = sim_create();
    bench->chip = sim_add_instance(bench->sim, kind_model(bench->part), "U1", bench->part);
    bench->bench = sim_add_instance(bench->sim, &BENCH_TYPE, "TB", &bench->pins);
    for (i = 0; i < bench->pins; i++) {
        SimNet *net = sim_add_net(bench->sim, "n");

        sim_connect(bench->sim, net, bench->chip, i);
        sim_connect(bench->sim, net, bench->bench, i);
    }
    if (!sim_start(bench->sim)) {
        return 0;
    }
    set_pin(bench, "CE#", LOGIC_1);
    set_pin(bench, "OE#", LOGIC_1);
    set_pin(bench, "WE#", LOGIC_1);
    set_address(bench, 0);
    set_data(bench, 0, 0);
    run_to(bench, NS(100));
    return 1;
}

static void bench_close(Bench *bench) {
    sim_destroy(bench->sim);
}

static void word_at(Bench *bench, uint32_t address, uint32_t *value, uint32_t *known) {
    const SimComponentType *type = sim_instance_type(bench->chip);

    if (type->memory_read(sim_instance_state(bench->chip), address, 1, value, known) != 1) {
        *value = 0;
        *known = 0;
    }
}

static int word_is(Bench *bench, uint32_t address, uint8_t expected) {
    uint32_t value;
    uint32_t known;

    word_at(bench, address, &value, &known);
    return known == 0xFF && value == expected;
}

static int word_unknown(Bench *bench, uint32_t address) {
    uint32_t value;
    uint32_t known;

    word_at(bench, address, &value, &known);
    return known == 0;
}

/*
 * A WE#-controlled load with CE# already low and OE# high: the address at t,
 * WE# low at t + 10 ns for `pulse`, the data `setup` before WE# rises.
 * Returns the time the load ends (WE# rising).
 */
static SimTime load_timed(Bench *bench, SimTime t, uint32_t address, uint8_t value, SimTime pulse,
                          SimTime setup) {
    SimTime end = t + NS(10) + pulse;

    run_to(bench, t);
    set_address(bench, address);
    run_to(bench, t + NS(10));
    set_pin(bench, "WE#", LOGIC_0);
    run_to(bench, end - setup);
    set_data(bench, 1, value);
    run_to(bench, end);
    set_pin(bench, "WE#", LOGIC_1);
    run_to(bench, end + NS(20));
    set_data(bench, 0, 0);
    return end;
}

/* A load that meets every time (tWP 120 ns, tDS 80 ns). Takes 300 ns. */
static SimTime load_byte(Bench *bench, SimTime t, uint32_t address, uint8_t value) {
    return load_timed(bench, t, address, value, NS(120), NS(80));
}

/* A read pulse: OE# low at t, the bus sampled `after` later, OE# high. */
static const char *read_at(Bench *bench, SimTime t, SimTime after) {
    static char text[9];

    run_to(bench, t);
    set_pin(bench, "OE#", LOGIC_0);
    run_to(bench, t + after);
    memcpy(text, bus(bench), sizeof text);
    set_pin(bench, "OE#", LOGIC_1);
    run_to(bench, t + after + NS(100));
    return text;
}

/* Loads the three-byte enable prefix from t; returns the last end. */
static SimTime load_enable(Bench *bench, SimTime t, uint32_t a, uint32_t b) {
    load_byte(bench, t, a, 0xAA);
    load_byte(bench, t + NS(300), b, 0x55);
    return load_byte(bench, t + NS(600), a, 0xA0);
}

static SimTime load_disable(Bench *bench, SimTime t, uint32_t a, uint32_t b) {
    static const uint8_t DATA[6] = {0xAA, 0x55, 0x80, 0xAA, 0x55, 0x20};
    SimTime end = t;
    int i;

    for (i = 0; i < 6; i++) {
        end = load_byte(bench, t + NS(300) * (SimTime)i, (i == 1 || i == 4) ? b : a, DATA[i]);
    }
    return end;
}

/* ---- 1. Power-up and the read window ------------------------------------- */

static void check_power_up(void) {
    Bench b;
    uint32_t value[4];
    uint32_t known[4];
    const SimComponentType *type;
    int all = 1;
    uint32_t a;
    SimTime t;

    expect(bench_open(&b, "FIXTURE-15"), "1: fixture part starts");
    type = sim_instance_type(b.chip);
    for (a = 0; a < 32768; a += 4) {
        size_t i;

        type->memory_read(sim_instance_state(b.chip), a, 4, value, known);
        for (i = 0; i < 4; i++) {
            all &= value[i] == 0xFF && known[i] == 0xFF;
        }
    }
    expect(all, "1: power-up contents are erased (every byte FF, known)");
    expect(strcmp(bus(&b), "ZZZZZZZZ") == 0, "1: outputs are high-Z before CE#/OE#");

    t = NS(1000);
    run_to(&b, t);
    set_address(&b, 0);
    set_pin(&b, "CE#", LOGIC_0);
    set_pin(&b, "OE#", LOGIC_0);
    run_to(&b, t + 1);
    expect(strcmp(bus(&b), "XXXXXXXX") == 0, "1: outputs drive X as soon as the read starts");
    run_to(&b, t + NS(150) - 1);
    expect(strcmp(bus(&b), "XXXXXXXX") == 0, "1: data is not valid before max(tACC, tCE, tOE)");
    run_to(&b, t + NS(150));
    expect(strcmp(bus(&b), "11111111") == 0, "1: FF is valid at max(tACC, tCE, tOE) = 150 ns");

    t = NS(2000);
    run_to(&b, t);
    set_pin(&b, "OE#", LOGIC_1);
    run_to(&b, t + NS(50) - 1);
    expect(strcmp(bus(&b), "XXXXXXXX") == 0, "1: outputs are X until tDF after OE# rises");
    run_to(&b, t + NS(50));
    expect(strcmp(bus(&b), "ZZZZZZZZ") == 0, "1: outputs are high-Z at tDF");
    bench_close(&b);
}

/* ---- 2. Byte write, the write cycle, DATA# polling and the toggle bit ---- */

static void check_byte_write(void) {
    Bench b;
    SimTime end;
    SimTime start;
    const char *r1;
    const char *r2;
    const char *r3;
    char s1[9], s2[9], s3[9];

    bench_open(&b, "FIXTURE-15");
    set_pin(&b, "CE#", LOGIC_0);
    end = load_byte(&b, NS(1000), 0x1234, 0x5A);
    start = end + TBLC;

    expect(word_is(&b, 0x1234, 0xFF), "2: memory unchanged right after the load");
    /* A read before tBLC runs out shows the stored (old) byte. */
    set_address(&b, 0x1234);
    r1 = read_at(&b, end + US(10), NS(200));
    expect(strcmp(r1, "11111111") == 0, "2: a read during the load window returns the old byte");

    r1 = read_at(&b, start + US(100), NS(100));
    memcpy(s1, r1, 9);
    r2 = read_at(&b, start + US(200), NS(100));
    memcpy(s2, r2, 9);
    r3 = read_at(&b, start + US(300), NS(100));
    memcpy(s3, r3, 9);
    printf("     polls: %s %s %s\n", s1, s2, s3);
    expect(strcmp(s1, "10XXXXXX") == 0, "2: 1st poll: I/O7 = not bit 7 (1), toggle 0, I/O0-5 X");
    expect(strcmp(s2, "11XXXXXX") == 0, "2: 2nd poll: toggle 1");
    expect(strcmp(s3, "10XXXXXX") == 0, "2: 3rd poll: toggle 0");

    run_to(&b, start + TWC - 1);
    expect(word_is(&b, 0x1234, 0xFF), "2: memory unchanged until tBLC + tWC after the load");
    run_to(&b, start + TWC);
    expect(word_is(&b, 0x1234, 0x5A), "2: 5A committed at tBLC + tWC");
    r1 = read_at(&b, start + TWC + US(1), NS(200));
    expect(strcmp(r1, "01011010") == 0, "2: a read after the cycle returns 5A");
    bench_close(&b);
}

/* ---- 3. Page write --------------------------------------------------------- */

static void check_page_write(void) {
    Bench b;
    SimTime end = 0;
    SimTime t = NS(1000);
    int before = 1;
    int after = 1;
    unsigned i;

    bench_open(&b, "FIXTURE-15");
    set_pin(&b, "CE#", LOGIC_0);
    for (i = 0; i < 64; i++) {
        end = load_byte(&b, t, 0x0400 + i, (uint8_t)(i ^ 0x3C));
        t += NS(300);
    }
    run_to(&b, end + TBLC + TWC - 1);
    for (i = 0; i < 64; i++) {
        before &= word_is(&b, 0x0400 + i, 0xFF);
    }
    run_to(&b, end + TBLC + TWC);
    for (i = 0; i < 64; i++) {
        after &= word_is(&b, 0x0400 + i, (uint8_t)(i ^ 0x3C));
    }
    expect(before, "3: no byte of the page is written before the cycle ends");
    expect(after, "3: all 64 bytes commit together at the end of one cycle");
    expect(word_is(&b, 0x03FF, 0xFF) && word_is(&b, 0x0440, 0xFF), "3: bytes outside the page unchanged");
    bench_close(&b);
}

/* ---- 4. A gap longer than tBLC ------------------------------------------- */

static void check_gap(void) {
    Bench b;
    SimTime end;

    bench_open(&b, "FIXTURE-15");
    set_pin(&b, "CE#", LOGIC_0);
    end = load_byte(&b, NS(1000), 0x2000, 0x12);
    load_byte(&b, end + TBLC + US(1), 0x2001, 0x34);
    run_to(&b, end + TBLC + TWC + TBLC + TWC + MS(1));
    expect(word_is(&b, 0x2000, 0x12), "4: the first byte is written");
    expect(word_is(&b, 0x2001, 0xFF), "4: a byte started after tBLC (cycle running) is ignored");
    bench_close(&b);
}

/* ---- 5. A load during the write cycle ------------------------------------ */

static void check_busy_load(void) {
    Bench b;
    SimTime end;

    bench_open(&b, "FIXTURE-15");
    set_pin(&b, "CE#", LOGIC_0);
    end = load_byte(&b, NS(1000), 0x3000, 0x21);
    load_byte(&b, end + TBLC + MS(5), 0x3001, 0x43);
    run_to(&b, end + TBLC + TWC + MS(5) + TBLC + TWC);
    expect(word_is(&b, 0x3000, 0x21), "5: the byte before the cycle is written");
    expect(word_is(&b, 0x3001, 0xFF), "5: a load during tWC is ignored");
    bench_close(&b);
}

/* ---- 6. Timing violations -------------------------------------------------- */

static void check_violations(void) {
    Bench b;
    SimTime end;

    bench_open(&b, "FIXTURE-15");
    set_pin(&b, "CE#", LOGIC_0);
    end = load_timed(&b, NS(1000), 0x0010, 0x77, NS(100) - NS(1), NS(80));
    run_to(&b, end + TBLC + TWC);
    expect(word_unknown(&b, 0x0010), "6: a pulse of tWP - 1 ns stores X");

    end = load_timed(&b, end + TBLC + TWC + US(1), 0x0011, 0x77, NS(120), NS(49));
    run_to(&b, end + TBLC + TWC);
    expect(word_unknown(&b, 0x0011), "6: data set up 1 ns short of tDS stores X");

    end = load_timed(&b, end + TBLC + TWC + US(1), 0x0012, 0x77, NS(100), NS(50));
    run_to(&b, end + TBLC + TWC);
    expect(word_is(&b, 0x0012, 0x77), "6: exactly tWP and tDS store the data");
    bench_close(&b);
}

/* ---- 7. A load outside the window's page --------------------------------- */

static void check_mixed_page(void) {
    Bench b;
    SimTime end;

    bench_open(&b, "FIXTURE-15");
    set_pin(&b, "CE#", LOGIC_0);
    load_byte(&b, NS(1000), 0x0500, 0x01);
    load_byte(&b, NS(1300), 0x0501, 0x02);
    end = load_byte(&b, NS(1600), 0x0540, 0x03);
    run_to(&b, end + TBLC + TWC);
    expect(word_unknown(&b, 0x0500) && word_unknown(&b, 0x0501),
           "7: a page change makes every loaded byte of the first page X");
    expect(word_is(&b, 0x0540, 0xFF), "7: the other page is not written");
    expect(word_is(&b, 0x0502, 0xFF), "7: unloaded bytes of the first page unchanged");
    bench_close(&b);
}

/* ---- 8. Software data protection ----------------------------------------- */

static void check_sdp(void) {
    Bench b;
    SimTime end;
    SimTime start;
    const char *poll;

    bench_open(&b, "FIXTURE-15");
    set_pin(&b, "CE#", LOGIC_0);

    /* Enable prefix + one byte. */
    load_enable(&b, NS(1000), 0x5555, 0x2AAA);
    end = load_byte(&b, NS(1900), 0x0600, 0x11);
    run_to(&b, end + TBLC + TWC);
    expect(word_is(&b, 0x0600, 0x11), "8: the byte after the enable prefix is written");
    expect(word_is(&b, 0x5555, 0xFF) && word_is(&b, 0x2AAA, 0xFF), "8: command bytes are never data");

    /* Protected now: a plain byte runs a cycle but writes nothing. */
    end = load_byte(&b, end + TBLC + TWC + US(1), 0x0601, 0x22);
    start = end + TBLC;
    set_address(&b, 0x0601);
    poll = read_at(&b, start + US(50), NS(100));
    expect(strcmp(poll, "10XXXXXX") == 0, "8: an unprotected load still runs a cycle (polling seen)");
    run_to(&b, start + TWC);
    expect(word_is(&b, 0x0601, 0xFF), "8: ... and writes nothing");
    poll = read_at(&b, start + TWC + US(1), NS(200));
    expect(strcmp(poll, "11111111") == 0, "8: the cycle ends at tWC");

    /* Prefix again + byte: written, protection stays on. */
    load_enable(&b, start + TWC + US(10), 0x5555, 0x2AAA);
    end = load_byte(&b, start + TWC + US(10) + NS(900), 0x0602, 0x33);
    run_to(&b, end + TBLC + TWC);
    expect(word_is(&b, 0x0602, 0x33), "8: a protected write (prefix + byte) is written");

    /* Disable, then plain bytes write again. */
    end = load_disable(&b, end + TBLC + TWC + US(1), 0x5555, 0x2AAA);
    run_to(&b, end + TBLC + TWC);
    end = load_byte(&b, end + TBLC + TWC + US(1), 0x0603, 0x44);
    run_to(&b, end + TBLC + TWC);
    expect(word_is(&b, 0x0603, 0x44), "8: after the disable sequence plain bytes write again");
    expect(word_is(&b, 0x5555, 0xFF), "8: the disable sequence wrote no data");

    /* A sequence that diverges: its loads are data. */
    load_byte(&b, end + TBLC + TWC + US(1), 0x5555, 0xAA);
    end = load_byte(&b, end + TBLC + TWC + US(1) + NS(300), 0x5556, 0x66);
    run_to(&b, end + TBLC + TWC);
    expect(word_is(&b, 0x5555, 0xAA) && word_is(&b, 0x5556, 0x66),
           "8: loads that match a sequence and then diverge are data");
    bench_close(&b);
}

/* ---- 9. Unknown WE# -------------------------------------------------------- */

static void check_unknown_we(void) {
    Bench b;

    bench_open(&b, "FIXTURE-15");
    set_pin(&b, "CE#", LOGIC_0);
    set_address(&b, 0x0700);
    run_to(&b, NS(1000));
    set_pin(&b, "WE#", LOGIC_X);
    run_to(&b, NS(1100));
    set_pin(&b, "WE#", LOGIC_1);
    run_to(&b, NS(1100) + TBLC + TWC);
    expect(word_unknown(&b, 0x0700), "9: X on WE# with a known address makes that byte X");
    expect(word_is(&b, 0x0701, 0xFF), "9: ... and only that byte");
    bench_close(&b);
}

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0 || load_fixture() != 0) {
        return 1;
    }
    check_power_up();
    check_byte_write();
    check_page_write();
    check_gap();
    check_busy_load();
    check_violations();
    check_mixed_page();
    check_sdp();
    check_unknown_we();
    part_library_destroy(library);
    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
