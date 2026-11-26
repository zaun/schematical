/*
 * shift_special_cov_test (plan-68-Q): the ls673, tap64, bus-register, fifo
 * and serial-in models' loader rejections (fixture devices of
 * tests/fixtures/shift with one edit each), and the states no other case
 * drives: the 74LS673 storage clear unknown, and the SN74LV8153 with an
 * unknown address pin, a frame pair whose addresses differ, and RESET unknown.
 */

#include "q_cov_bench.h"

#define E(...) ((const char *const[]){__VA_ARGS__, NULL})
#define FX "fx:shift/parts.json"

static void fail(const char *id, const char *const *edits, const char *expect) {
    qc_fail(FX, id, edits, expect);
}

static void check_loaders(void) {
    static const char NOPE[] = "no pin named \"NOPE\"";

    fail("fx-74_673", E("storageOutputs.15", "'NOPE'"), NOPE);
    fail("fx-74_673", E("clock", NULL), "missing \"clock\"");
    fail("fx-74_673", E("storageOutputs", "['Y0']"), "\"storageOutputs\"");
    fail("fx-74_674", E("parallel", "['P0']"), "\"parallel\"");
    fail("fx-74_673", E("storageOutputs", NULL), "exactly one of \"storageOutputs\" and \"parallel\" is needed");
    fail("fx-74_673", E("clock", "'NOPE'"), NOPE);
    fail("fx-4517B", E("units", NULL), "missing \"units\"");
    fail("fx-4517B", E("units", "[]"), "\"units\" must be an array of 1 to 2 units");
    fail("fx-4517B", E("units.1.x", "1"), "units[1]: unknown key \"x\"");
    fail("fx-4517B", E("units.1.data", "'NOPE'"), NOPE);
    fail("fx-4034B", E("a", NULL), "missing \"a\"");
    fail("fx-4034B", E("serialIn", "'NOPE'"), NOPE);
    fail("fx-40105B", E("data", NULL), "missing \"data\"");
    fail("fx-40105B", E("words", "8"), "\"words\" must be 16");
    fail("fx-40105B", E("shiftOut", "'NOPE'"), NOPE);
    fail("fx-74_8153", E("serialOut", NULL), "missing \"serialOut\"");
    fail("fx-74_8153", E("outputSelect", "'NOPE'"), NOPE);
}

/* SN74LS673 function table (sn74ls673 p.2): STRCLR low clears Y at once,
   whatever CS; STRCLR unknown with Y all ones leaves every Y unknown. */
static void check_673_clear_unknown(void) {
    static const char *const Y[] = {"Y0", "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7",
                                    "Y8", "Y9", "Y10", "Y11", "Y12", "Y13", "Y14", "Y15"};
    PartLibrary *lib;
    Bench *b = qc_bench(&lib, FX, "fx-74_673", NULL, NULL);
    int i;
    int ok;

    qc_set(b, "CS", L0);
    qc_set(b, "STRCLR", L1);
    qc_set(b, "MODE/STRCLK", L0);
    qc_set(b, "SHCLK", L0);
    qc_set(b, "R/W", L0);
    qc_set(b, "SER/Q15", L1);
    qc_step(b);
    for (i = 0; i < 16; i++) {
        qc_pulse(b, "SHCLK");
    }
    qc_set(b, "MODE/STRCLK", L1); /* store: Y <- Q */
    qc_step(b);
    qc_set(b, "MODE/STRCLK", L0);
    qc_step(b);
    ok = qc_bus(b, Y, 16, 0xFFFF);
    qc_set(b, "STRCLR", LX);
    qc_step(b);
    qc_expect(ok && qc_bus_all(b, Y, 16, LX), "74LS673 Y all ones, STRCLR = X -> Y0-Y15 = X");
    bench_destroy(b);
    part_library_destroy(lib);
}

/* ---- SN74LV8153: frames as in shift_special_test (start 0, 1, A0-A2, nibble, stop) ---- */

static const char *const Y8[] = {"Y0", "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7"};
static const char *const A3[] = {"A0", "A1", "A2"};
#define P9600 ((SimTime)(1000000000000ull / 9600))

static void frame(Bench *b, unsigned address, unsigned nibble) {
    unsigned levels[10];
    unsigned k;

    levels[0] = 0;
    levels[1] = 1;
    for (k = 0; k < 3; k++) {
        levels[2 + k] = (address >> k) & 1u;
    }
    for (k = 0; k < 4; k++) {
        levels[5 + k] = (nibble >> k) & 1u;
    }
    levels[9] = 1;
    for (k = 0; k < 10; k++) {
        qc_set(b, "D", levels[k] ? L1 : L0);
        bench_run(b, P9600);
    }
    bench_run(b, P9600);
}

static Bench *open_8153(PartLibrary **lib, unsigned address) {
    Bench *b = qc_bench(lib, FX, "fx-74_8153", NULL, NULL);

    qc_set(b, "D", L1);
    qc_set(b, "OUTSEL", L1);
    qc_set(b, "OE", L0);
    qc_set_bus(b, A3, 3, address);
    qc_set(b, "RESET", L0);
    qc_step(b);
    qc_set(b, "RESET", L1);
    qc_step(b);
    return b;
}

/*
 * sn74lv8153 § 8.3: a byte is two frames with the device's address; a frame
 * is checked against the A pins. An unknown A pin cannot match: the frame is
 * ignored (the next two good frames still make a byte). Two frames whose
 * addresses differ (the A pins changed between them) make no byte. RESET
 * unknown leaves every Y unknown.
 */
static void check_8153_unknown(void) {
    PartLibrary *lib;
    Bench *b = open_8153(&lib, 5);
    int ok;

    qc_set(b, "A1", LX);
    frame(b, 5, 0x9); /* address pin unknown: ignored */
    qc_set(b, "A1", L0);
    frame(b, 5, 0x6);
    frame(b, 5, 0x3);
    ok = qc_bus(b, Y8, 8, 0x36);
    qc_expect(ok, "74LV8153 a frame while A1 = X is ignored; the next pair gives 0x36");

    frame(b, 5, 0x1); /* first frame, address 5 */
    qc_set_bus(b, A3, 3, 3);
    qc_step(b);
    frame(b, 3, 0x2); /* second frame, address 3: the pair is dropped */
    qc_expect(qc_bus(b, Y8, 8, 0x36), "74LV8153 a frame pair with addresses 5 then 3 is dropped: Y keeps 0x36");

    qc_set(b, "RESET", LX);
    qc_step(b);
    qc_expect(qc_bus_all(b, Y8, 8, LX), "74LV8153 RESET = X -> Y0-Y7 = X");
    bench_destroy(b);
    part_library_destroy(lib);
}

int main(void) {
    check_loaders();
    check_673_clear_unknown();
    check_8153_unknown();
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("shift_special_cov_test: all passed\n");
    return 0;
}
