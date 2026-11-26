/*
 * latch_cov_test (plan-68-Q): the latch kind's five models' loader
 * rejections (fixture devices of tests/fixtures/latch with one edit each),
 * an unknown S input of an S-R latch, and the open-drain / 3-state pin hooks
 * for an output pin no unit names.
 */

#include "model_pins.h"
#include "q_cov_bench.h"

#define E(...) ((const char *const[]){__VA_ARGS__, NULL})
#define FX "fx:latch/parts.json"

static const char NOPE[] = "no pin named \"NOPE\"";

static void fail(const char *id, const char *const *edits, const char *expect) {
    qc_fail(FX, id, edits, expect);
}

static void check_transparent_loader(void) {
    fail("fx-74_373", E("model", NULL), "missing \"model\"");
    fail("fx-74_373", E("parts.0.family", "'ZZ'"), "unknown family \"ZZ\"");
    fail("fx-74_373", E("units", NULL), "missing \"units\"");
    fail("fx-74_373", E("units", "[]"), "\"units\" must be an array of 1 to 40 units");
    fail("fx-74_373", E("units.0.x", "1"), "units[0]: unknown key \"x\"");
    fail("fx-74_373", E("units.0.q", NULL), "units[0]: a unit needs \"q\" or \"qn\"");
    fail("fx-4042B", E("polarity", "1"), "\"polarity\" must be a string");
    fail("fx-74_373", E("enableActive", NULL), "missing \"enableActive\"");
    fail("fx-74_373", E("outputEnableActive", NULL), "missing \"outputEnableActive\"");
    fail("fx-74_373", E("units.0.d", "'NOPE'"), NOPE);
    fail("fx-4042B", E("polarity", "'NOPE'"), NOPE);
}

static void check_other_loaders(void) {
    fail("fx-74_259", E("address.0", "'NOPE'"), NOPE);
    fail("fx-74_259", E("data", NULL), "missing \"data\"");
    fail("fx-74_259", E("outputs", "['Q0','Q1','Q2','Q3']"), "\"outputs\" must have 8 names");
    fail("fx-74_259", E("outputs.7", "'NOPE'"), NOPE);
    fail("fx-74_279", E("units", NULL), "missing \"units\"");
    fail("fx-74_279", E("units", "[]"), "\"units\" must be an array of 1 to");
    fail("fx-74_279", E("units.0.x", "1"), "units[0]: unknown key \"x\"");
    fail("fx-74_279", E("both", NULL), "missing \"both\"");
    fail("fx-74_279", E("both", "'none'"), "\"both\" must be \"set\" or \"reset\", not \"none\"");
    fail("fx-74_279", E("units.0.s.1", "'NOPE'"), NOPE);
    fail("fx-74_279", E("units.0.r", "'NOPE'"), NOPE);
    fail("fx-4043B", E("outputEnable.pin", "'NOPE'"), NOPE);
    fail("fx-74_4511", E("inputs", NULL), "missing \"inputs\"");
    fail("fx-74_4511", E("glyphs.2", "'11'"), "glyphs[2]: must be 7 characters 0 or 1");
    fail("fx-74_4511", E("glyphs.3", "'111100x'"), "glyphs[3]: must be 7 characters 0 or 1");
    fail("fx-74_4511", E("segments.6", "'NOPE'"), NOPE);
    fail("fx-74_16260", E("latchEnables", NULL), "missing \"latchEnables\"");
    fail("fx-74_16260", E("latchEnables.x", "'A1'"), "latchEnables: unknown key \"x\"");
    fail("fx-74_16260", E("latchEnables.b1", "1"), "latchEnables: \"b1\" must be a string");
    fail("fx-74_16260", E("a", NULL), "missing \"a\"");
    fail("fx-74_16260", E("select", "'NOPE'"), NOPE);
}

/* SN74LS279 function table: S low sets, R low resets; an unknown S with the
   latch at 0 may set it -> Q unknown; R low then resets it to 0. */
static void check_sr_unknown_set(void) {
    PartLibrary *lib;
    Bench *b = qc_bench(&lib, FX, "fx-74_279", NULL, NULL);
    int ok;

    qc_set(b, "1S1", L1);
    qc_set(b, "1S2", L1);
    qc_set(b, "1R", L1);
    qc_step(b);
    ok = qc_is(b, "1Q", L0);
    qc_set(b, "1S1", LX);
    qc_step(b);
    ok &= qc_is(b, "1Q", LX);
    qc_set(b, "1S1", L1);
    qc_set(b, "1R", L0);
    qc_step(b);
    qc_expect(ok && qc_is(b, "1Q", L0), "74LS279 at 0, 1S1 = X -> 1Q = X; 1R low -> 0");
    bench_destroy(b);
    part_library_destroy(lib);
}

/* The pin hooks answer for the units' outputs only: an output pin left to no
   unit is neither open-drain (a 74x373 made open-drain) nor 3-state (4043B). */
static void check_pin_hooks(void) {
    PartLibrary *lib;
    const Part *part = qc_load(&lib, FX, "fx-74_373", E("output", "'open-drain'", "units.7", NULL), NULL);

    qc_expect(part != NULL && part_pin_open_drain(part, qc_pin(part, "1Q")) == 1 &&
                  part_pin_open_drain(part, qc_pin(part, "8Q")) == 0,
              "74x373 open-drain without unit 8: 1Q open-drain, 8Q not");
    part_library_destroy(lib);
    part = qc_load(&lib, FX, "fx-74_373", NULL, NULL);
    qc_expect(part != NULL && part_pin_open_drain(part, qc_pin(part, "1Q")) == 0,
              "74x373 (push-pull): 1Q not open-drain");
    part_library_destroy(lib);
    part = qc_load(&lib, FX, "fx-4043B", E("units.3", NULL), NULL);
    qc_expect(part != NULL && part_pin_tristate(part, qc_pin(part, "Q1")) == 1 &&
                  part_pin_tristate(part, qc_pin(part, "Q4")) == 0,
              "4043B without unit 4: Q1 3-state, Q4 not");
    part_library_destroy(lib);
}

int main(void) {
    check_transparent_loader();
    check_other_loaders();
    check_sr_unknown_set();
    check_pin_hooks();
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("latch_cov_test: all passed\n");
    return 0;
}
