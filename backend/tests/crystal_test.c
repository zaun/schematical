/*
 * crystal_test (plan-44-E): the crystal kind and its clock model. The fixture
 * parts of support/crystal_fixture.h load next to the real part files: the
 * facts read back, each malformed device or part is refused, and in circuits
 * a crystal is a weak clock on its own two terminals (X1 at the level, X2 at
 * the complement), overridden by any strong driver and readable by a logic
 * gate wired to a terminal.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "crystal_fixture.h"
#include "parts.h"
#include "read_file.h"

static int failures;
static PartLibrary *library;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

static int same(const char *a, const char *b) {
    return a != NULL && b != NULL && strcmp(a, b) == 0;
}

/* ---- facts and the loader (cases a-c) ------------------------------------ */

static void fact_cases(void) {
    const Part *x = part_library_find(library, "TESTXTAL-1M");
    const Part *k = part_library_find(library, "TESTXTAL-32K");
    const Part *r = part_library_find(library, "TESTRES-8M");

    expect(x != NULL && x->kind == PART_CRYSTAL && x->crystal != NULL && x->pin_count == 2 &&
               x->vcc == NULL && x->pins[0].role == PIN_INOUT && x->pins[1].role == PIN_INOUT &&
               same(x->crystal->type, "crystal") && x->crystal->hz == 1000000 &&
               x->crystal->load_ff == 18000 && x->crystal->tolerance_ppm == 20 &&
               x->crystal->stability_ppm == 30 && x->crystal->esr_mohm == 80000 &&
               x->crystal->capacitors == 0,
           "TESTXTAL-1M: a crystal, 2 inout pins, no vcc, 1 MHz, 18 pF = 18000 fF, 20 ppm, "
           "30 ppm, 80 ohm = 80000 mohm, no built-in capacitors");
    expect(k != NULL && k->kind == PART_CRYSTAL && k->pin_count == 4 &&
               k->pins[0].role == PIN_INOUT && k->pins[1].role == PIN_NC &&
               k->pins[2].role == PIN_INOUT && k->pins[3].role == PIN_NC &&
               k->crystal != NULL && k->crystal->hz == 32768 && k->crystal->load_ff == 0 &&
               k->crystal->tolerance_ppm == 0 && k->crystal->esr_mohm == 0,
           "TESTXTAL-32K: 4 pins, pins 2 and 4 nc, 32768 Hz, no other facts");
    expect(r != NULL && r->kind == PART_CRYSTAL && r->crystal != NULL &&
               same(r->crystal->type, "resonator") && r->pin_count == 3 &&
               same(r->pins[1].name, "GND") && r->pins[1].role == PIN_NC &&
               r->crystal->capacitors == 1,
           "TESTRES-8M: a resonator, 3 pins, pin 2 GND nc, built-in capacitors");
}

/* ---- rejections (cases d-j) ---------------------------------------------- */

/* A one-file load that must be refused; `want` is part of the error. With
   `footprints`, the real part files are loaded first (footprints.json), so
   the footprint rules apply. */
static void refused_in(const char *what, const char *json, const char *want, int footprints) {
    PartLibrary *lib = part_library_create();
    char *error = NULL;
    int ok = lib != NULL && (!footprints || load_parts_dir(lib, PARTS_DIR) == 0) &&
             load_parts(lib, json, "bad-crystal.json", &error) != 0 &&
             error != NULL && strstr(error, want) != NULL;

    if (!ok) {
        printf("  error: %s\n", error != NULL ? error : "(none)");
    }
    expect(ok, what);
    free(error);
    part_library_destroy(lib);
}

static void refused(const char *what, const char *json, const char *want) {
    refused_in(what, json, want, 0);
}

#define TERM "[\"X1\",\"X2\"]"
#define BAD_PART(KEYS) CRYSTAL_FILE(CRYSTAL_DEVICE("bad", "crystal", TERM, XFX_TWO, "BAD", "RES-0402", KEYS))

static void rejection_cases(void) {
    refused("no \"hz\": refused", BAD_PART(""), "missing \"hz\"");
    refused("hz 0: refused", BAD_PART(",\"hz\":0"), "\"hz\" must be at least 1");
    refused("hz 2e9: refused", BAD_PART(",\"hz\":2000000000"),
            "\"hz\" must be a whole number from 0 to 1000000000");
    refused("type quartz: refused",
            CRYSTAL_FILE(CRYSTAL_DEVICE("bad", "quartz", TERM, XFX_TWO, "BAD", "RES-0402", ",\"hz\":1")),
            "unknown crystal type \"quartz\"");
    refused("terminals with 1 name: refused",
            CRYSTAL_FILE(CRYSTAL_DEVICE("bad", "crystal", "[\"X1\"]", XFX_TWO, "BAD", "RES-0402", ",\"hz\":1")),
            "\"terminals\" must name 2 pins");
    refused("a terminal with role input: refused",
            CRYSTAL_FILE(CRYSTAL_DEVICE("bad", "crystal", TERM,
                                        XFX_PIN(1, "X1", "input") "," XFX_IO(2, "X2"), "BAD",
                                        "RES-0402", ",\"hz\":1")),
            "crystal pin \"X1\" must be inout");
    refused("a non-terminal pin with role inout: refused",
            CRYSTAL_FILE(CRYSTAL_DEVICE("bad", "crystal", TERM,
                                        XFX_IO(1, "X1") "," XFX_IO(2, "NC") "," XFX_IO(3, "X2"),
                                        "BAD", "SOT23-3", ",\"hz\":1")),
            "crystal pin \"NC\" must be nc");
    refused("a pinout of 5 pins: refused",
            CRYSTAL_FILE(CRYSTAL_DEVICE("bad", "crystal", TERM,
                                        XFX_FOUR "," XFX_NC(5, "NC"), "BAD", "PINHDR-1X05-V",
                                        ",\"hz\":1")),
            "a crystal has 2 to 4 pins");
    refused("loadPf 0: refused", BAD_PART(",\"hz\":1,\"loadPf\":0"),
            "\"loadPf\" must be above 0 and at most 1000");
    refused("tolerancePpm 200000: refused", BAD_PART(",\"hz\":1,\"tolerancePpm\":200000"),
            "\"tolerancePpm\" must be above 0 and at most 100000");
    refused("capacitors on a crystal: refused", BAD_PART(",\"hz\":1,\"capacitors\":true"),
            "\"capacitors\" is only for a resonator");
    refused("an unlisted part key: refused", BAD_PART(",\"hz\":1,\"volts\":5"), "unknown key \"volts\"");
    refused_in("a footprint with the wrong pad count: refused",
               CRYSTAL_FILE(CRYSTAL_DEVICE("bad", "crystal", TERM, XFX_FOUR, "BAD", "RES-0402", ",\"hz\":1")),
               "has 2 pads, pinout \"p\" has 4 pins", 1);
}

/* ---- circuit cases (k-p) ------------------------------------------------- */

static size_t add(Circuit *c, const char *name) {
    size_t index = 0;

    if (circuit_add_part(c, name, &index) != 0) {
        fprintf(stderr, "add %s: %s\n", name, circuit_error(c));
        exit(1);
    }
    return index;
}

static size_t net(Circuit *c) {
    size_t index = 0;

    if (circuit_add_net(c, &index) != 0) {
        fprintf(stderr, "net: %s\n", circuit_error(c));
        exit(1);
    }
    return index;
}

static void wire(Circuit *c, size_t part, const char *pin, size_t n) {
    if (circuit_connect(c, part, pin, n) != 0) {
        fprintf(stderr, "connect %s: %s\n", pin, circuit_error(c));
        exit(1);
    }
}

static void run(Circuit *c, uint64_t ps) {
    if (circuit_advance(c, ps) == 0) {
        fprintf(stderr, "advance: %s\n", circuit_error(c) != NULL ? circuit_error(c) : "stopped");
        exit(1);
    }
}

static void clock_cases(void) {
    /* (k) TESTXTAL-1M, X1 on A, X2 on B, nothing else. */
    {
        Circuit *c = circuit_create(library);
        size_t x = add(c, "TESTXTAL-1M"), a = net(c), b = net(c);
        int ok;

        wire(c, x, "1", a);
        wire(c, x, "2", b);
        ok = circuit_start(c) == 0;
        run(c, 100000);
        ok = ok && circuit_net_value(c, a) == CIRCUIT_0 && circuit_net_value(c, b) == CIRCUIT_1;
        run(c, 500000);
        ok = ok && circuit_net_value(c, a) == CIRCUIT_1 && circuit_net_value(c, b) == CIRCUIT_0;
        run(c, 500000);
        ok = ok && circuit_net_value(c, a) == CIRCUIT_0 && circuit_net_value(c, b) == CIRCUIT_1;
        expect(ok, "TESTXTAL-1M: at 100 ns A=0 B=1, at 600 ns A=1 B=0, at 1100 ns A=0 B=1");
        circuit_destroy(c);
    }
    /* (l) TESTXTAL-32K: half period 15258789 ps. */
    {
        Circuit *c = circuit_create(library);
        size_t x = add(c, "TESTXTAL-32K"), a = net(c), b = net(c);
        int ok;

        wire(c, x, "1", a);
        wire(c, x, "3", b);
        ok = circuit_start(c) == 0;
        run(c, 15000000);
        ok = ok && circuit_net_value(c, a) == CIRCUIT_0 && circuit_net_value(c, b) == CIRCUIT_1;
        run(c, 1000000);
        ok = ok && circuit_net_value(c, a) == CIRCUIT_1;
        run(c, 15000000);
        ok = ok && circuit_net_value(c, a) == CIRCUIT_0;
        expect(ok, "TESTXTAL-32K (pads 1 and 3): half period 15258789 ps: A=0 at 15 us, 1 at 16 us, "
                   "0 at 31 us");
        circuit_destroy(c);
    }
    /* (m) a GND supply on A: A reads 0 at every time, B still toggles. */
    {
        Circuit *c = circuit_create(library);
        size_t x = add(c, "TESTXTAL-1M"), gnd = add(c, "GND"), a = net(c), b = net(c);
        int ok;

        wire(c, x, "1", a);
        wire(c, x, "2", b);
        wire(c, gnd, "1", a);
        ok = circuit_start(c) == 0;
        run(c, 100000);
        ok = ok && circuit_net_value(c, a) == CIRCUIT_0 && circuit_net_value(c, b) == CIRCUIT_1;
        run(c, 500000);
        ok = ok && circuit_net_value(c, a) == CIRCUIT_0 && circuit_net_value(c, b) == CIRCUIT_0;
        run(c, 500000);
        ok = ok && circuit_net_value(c, a) == CIRCUIT_0 && circuit_net_value(c, b) == CIRCUIT_1;
        expect(ok, "a strong GND on X1's net: A reads 0 at every time (strong beats weak), B toggles");
        circuit_destroy(c);
    }
    /* (n) X1 on the input of an SN74HC04N (5 V): the output is the complement. */
    {
        Circuit *c = circuit_create(library);
        size_t x = add(c, "TESTXTAL-1M"), g = add(c, "SN74HC04N"), vcc = add(c, "5V"),
               gnd = add(c, "GND");
        size_t hi = net(c), lo = net(c), in = net(c), out = net(c);
        uint64_t changes;
        int ok;

        wire(c, vcc, "1", hi);
        wire(c, gnd, "1", lo);
        wire(c, g, "14", hi);
        wire(c, g, "7", lo);
        wire(c, x, "1", in);
        wire(c, g, "1", in);
        wire(c, g, "2", out);
        ok = circuit_start(c) == 0;
        run(c, 700000);
        ok = ok && circuit_net_value(c, in) == CIRCUIT_1 && circuit_net_value(c, out) == CIRCUIT_0;
        run(c, 500000);
        ok = ok && circuit_net_value(c, in) == CIRCUIT_0 && circuit_net_value(c, out) == CIRCUIT_1;
        run(c, 8800000);
        changes = circuit_net_changes(c, out);
        ok = ok && changes + 1 >= 20 && changes <= 21;
        if (!ok) {
            printf("  out changed %llu times\n", (unsigned long long)changes);
        }
        expect(ok, "TESTXTAL-1M X1 on an SN74HC04N input: the output is the complement, and it "
                   "changes 20 +- 1 times in 10 us");
        circuit_destroy(c);
    }
    /* (o) TESTXTAL-FAST: 100 MHz, half period 5000 ps. */
    {
        Circuit *c = circuit_create(library);
        size_t x = add(c, "TESTXTAL-FAST"), a = net(c), b = net(c);
        uint64_t changes;
        int ok;

        wire(c, x, "1", a);
        wire(c, x, "2", b);
        ok = circuit_start(c) == 0 && circuit_advance(c, 1000000) != 0;
        changes = circuit_net_changes(c, a);
        ok = ok && circuit_error(c) == NULL && changes + 1 >= 200 && changes <= 201;
        if (!ok) {
            printf("  A changed %llu times\n", (unsigned long long)changes);
        }
        expect(ok, "TESTXTAL-FAST (100 MHz): 1 us runs without an error, A toggles 200 +- 1 times");
        circuit_destroy(c);
    }
    /* (p) neither terminal wired. */
    {
        Circuit *c = circuit_create(library);
        int ok;

        add(c, "TESTRES-8M");
        ok = circuit_start(c) == 0 && circuit_advance(c, 1000000) != 0 && circuit_error(c) == NULL;
        expect(ok, "a resonator with no terminal wired starts and advances 1 us without an error");
        circuit_destroy(c);
    }
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 ||
        load_crystal_fixture(library) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    fact_cases();
    rejection_cases();
    clock_cases();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("crystal_test: all passed\n");
    return 0;
}
