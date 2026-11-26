/*
 * shift_cov_test (plan-68-Q): the shift kind's and the shift model's loader
 * rejections (fixture devices of tests/fixtures/shift with one edit each),
 * and the states no other case drives: a clocked clear unknown (74x323 MR),
 * a storage clear unknown (74x594 RCLR), a clear-all unknown (TPIC6C596),
 * and the analog high-voltage outputs of the HV5812.
 */

#include "model_pins.h"
#include "q_cov_bench.h"

#define E(...) ((const char *const[]){__VA_ARGS__, NULL})
#define FX "fx:shift/parts.json"

static void fail(const char *id, const char *const *edits, const char *expect) {
    qc_fail(FX, id, edits, expect);
}

static void check_kind_loader(void) {
    fail("fx-74_164", E("model", NULL), "missing \"model\"");
    fail("fx-74_164", E("model", "'twist'"), "unknown model \"twist\"");
    fail("fx-74_164", E("words", "16"), "\"words\" is not a key of model \"shift\"");
    fail("fx-40105B", E("timing.HC.fallThrough", "'x'"), "timing.HC: \"fallThrough\" must be a time in ns");
    fail("fx-40105B", E("timing.HC.fallThrough", NULL), "timing.HC: missing \"fallThrough\"");
    fail("fx-74_164", E("parts.0.family", "'ZZ'"), "unknown family \"ZZ\"");
    fail("fx-74_164", E("units.0.serial.1", "'NOPE'"), "no pin named \"NOPE\"");
    fail("fx-74_598", E("units.0.enables", "[]"), "\"enables\" must be an array of 1 to");
    fail("fx-74_598", E("units.0.enables.0.active", NULL), "enables[0]: missing \"active\"");
}

static void check_unit_loader(void) {
    fail("fx-74_164", E("units.0.outputs", "[]"), "\"outputs\" must be an array of 1 to");
    fail("fx-74_164", E("units.0.outputs.0.x", "1"), "outputs[0]: unknown key \"x\"");
    fail("fx-74_164", E("units.0.outputs.0.stage", "0"), "outputs[0]: \"stage\" must be 1 to 8");
    fail("fx-74_194", E("units.0.modes.table.0", "'spin'"), "modes: table[0]: unknown mode \"spin\"");
    fail("fx-74_194", E("units.0.modes.x", "1"), "modes: unknown key \"x\"");
    fail("fx-74_195", E("units.0.serialJK.x", "1"), "serialJK: unknown key \"x\"");
    fail("fx-4031B", E("units.0.serialSelect.x", "1"), "serialSelect: unknown key \"x\"");
    fail("fx-74_165", E("units.0.load.x", "1"), "load: unknown key \"x\"");
    fail("fx-74_165", E("units.0.load.when", "'never'"),
         "load: \"when\" must be \"async\" or \"clock\", not \"never\"");
    fail("fx-74_165", E("units.0.load.from", "'bus'"),
         "load: \"from\" must be \"data\", \"inputRegister\" or \"io\", not \"bus\"");
    fail("fx-74_165", E("units.0.load.from", "'io'"), "a load from \"io\" needs \"io\"");
    fail("fx-74_299", E("units.0.io.x", "1"), "io: unknown key \"x\"");
    fail("fx-74_299", E("units.0.io.notInModes", "'load'"), "\"notInModes\" must be an array of mode names");
    fail("fx-74_299", E("units.0.io.notInModes", "['jump']"), "\"notInModes\" must be an array of mode names");
    fail("fx-74_595", E("units.0.storage.x", "1"), "storage: unknown key \"x\"");
    fail("fx-74_595", E("units.0.storage.outputEnable", "{'pin':'OE'}"), "storage: outputEnable: missing \"active\"");
    fail("fx-74_597", E("units.0.inputRegister.x", "1"), "inputRegister: unknown key \"x\"");
    fail("fx-74_164", E("units.0.x", "1"), "units[0]: unknown key \"x\"");
    fail("fx-TPIC6C595", E("units.0.clearScope", "'some'"), "\"clearScope\" must be \"shift\" or \"all\"");
    fail("fx-74_165", E("units.0.data", NULL), "a load from \"data\" needs \"data\"");
    fail("fx-74_597", E("units.0.inputRegister", NULL), "a load from \"inputRegister\" needs \"inputRegister\"");
    fail("fx-74_194", E("units.0.data", NULL), "a \"load\" mode row needs \"data\" or \"io\"");
    fail("fx-74_598", E("units.0.io", NULL), "\"inputRegister\" needs \"data\" or \"io\"");
    fail("fx-74_164", E("units", NULL), "missing \"units\"");
    fail("fx-74_164", E("units", "[]"), "\"units\" must be an array of 1 to");
}

static void check_bind(void) {
    static const char NOPE[] = "no pin named \"NOPE\"";

    fail("fx-74_164", E("units.0.clock.pin", "'NOPE'"), NOPE);
    fail("fx-74_194", E("units.0.modes.pins.1", "'NOPE'"), NOPE);
    fail("fx-74_598", E("units.0.enables.0.pin", "'NOPE'"), NOPE);
    fail("fx-74_299", E("units.0.io.outputEnables.1.pin", "'NOPE'"), NOPE);
    fail("fx-74_164", E("units.0.outputs.7.pin", "'NOPE'"), NOPE);
    fail("fx-4094B", E("units.0.serialOutputs.0.pin", "'NOPE'"), NOPE);
}

/* A load from the I/O pins ("from": "io") is a valid unit (74x299 with a
   load control on its I/O pins) and loads. */
static void check_load_from_io(void) {
    PartLibrary *lib;
    const Part *part = qc_load(&lib, FX, "fx-74_299",
                               E("units.0.load", "{'pin':'OE1','active':'low','when':'clock','from':'io'}"),
                               NULL);

    qc_expect(part != NULL && part_library_count(lib) > 0, "74x299 with a load from \"io\" loads");
    part_library_destroy(lib);
}

/* 74x323 (synchronous MR): MR unknown at a clock that shifts a 1 into
   stage 1: Q0 = X (cleared or shifted), Q7 = 0 (0 either way). */
static void check_clocked_clear_unknown(void) {
    PartLibrary *lib;
    Bench *b = qc_bench(&lib, FX, "fx-74_323", NULL, NULL);
    int ok;

    qc_set(b, "OE1", L1);
    qc_set(b, "OE2", L1);
    qc_set(b, "CP", L0);
    qc_set(b, "S0", L1);
    qc_set(b, "S1", L0);
    qc_set(b, "DS0", L1);
    qc_set(b, "DS7", L0);
    qc_set(b, "MR", L0);
    qc_step(b);
    qc_pulse(b, "CP");
    ok = qc_is(b, "Q0", L0) && qc_is(b, "Q7", L0);
    qc_set(b, "MR", LX);
    qc_step(b);
    qc_pulse(b, "CP");
    qc_expect(ok && qc_is(b, "Q0", LX) && qc_is(b, "Q7", L0),
              "74x323 MR = X at a shift-right clock with DS0 = 1: Q0 = X, Q7 = 0");
    bench_destroy(b);
    part_library_destroy(lib);
}

static const char *const QS[] = {"QA", "QB", "QC", "QD", "QE", "QF", "QG", "QH"};

/* 74x594: RCLR unknown with the storage at FF: every QA-QH unknown. */
static void check_storage_clear_unknown(void) {
    PartLibrary *lib;
    Bench *b = qc_bench(&lib, FX, "fx-74_594", NULL, NULL);
    int i;
    int ok;

    qc_set(b, "SRCLR", L1);
    qc_set(b, "RCLR", L1);
    qc_set(b, "RCLK", L0);
    qc_set(b, "SRCLK", L0);
    qc_set(b, "SER", L1);
    qc_step(b);
    for (i = 0; i < 8; i++) {
        qc_pulse(b, "SRCLK");
    }
    qc_pulse(b, "RCLK");
    ok = qc_bus(b, QS, 8, 0xFF);
    qc_set(b, "RCLR", LX);
    qc_step(b);
    qc_expect(ok && qc_bus_all(b, QS, 8, LX), "74x594 storage FF, RCLR = X -> QA-QH = X");
    bench_destroy(b);
    part_library_destroy(lib);
}

/* TPIC6C596 (clear scope all): CLR unknown with ones everywhere: the
   storage (DRAIN) and the serial-output flip-flop (SEROUT) become unknown. */
static void check_clear_all_unknown(void) {
    PartLibrary *lib;
    Bench *b = qc_bench(&lib, FX, "fx-TPIC6C596", NULL, NULL);
    int i;
    int ok;

    qc_set(b, "CLR", L1);
    qc_set(b, "G", L0);
    qc_set(b, "RCK", L0);
    qc_set(b, "SRCK", L0);
    qc_set(b, "SERIN", L1);
    qc_step(b);
    for (i = 0; i < 8; i++) {
        qc_pulse(b, "SRCK");
    }
    qc_pulse(b, "RCK");
    ok = qc_is(b, "SEROUT", L1) && qc_is(b, "DRAIN0", L0);
    qc_set(b, "CLR", LX);
    qc_step(b);
    qc_expect(ok && qc_is(b, "SEROUT", LX) && qc_is(b, "DRAIN0", LX) && qc_is(b, "DRAIN7", LX),
              "TPIC6C596 all ones, CLR = X -> SEROUT, DRAIN0-7 = X");
    bench_destroy(b);
    part_library_destroy(lib);
}

/* HV5812 (hv5812 p.1): VPP is the high-voltage supply of the HVOUT
   drivers, so both are analog to the voltage check; DATA OUT is a logic
   output. Open-drain storage outputs would not carry VPP. */
static void check_analog_supply(void) {
    PartLibrary *lib;
    const Part *part = qc_load(&lib, "shift.json", "HV5812", NULL, NULL);

    qc_expect(part != NULL && part_pin_analog(part, qc_pin(part, "VPP")) == 1 &&
                  part_pin_analog(part, qc_pin(part, "HVOUT1")) == 1 &&
                  part_pin_analog(part, qc_pin(part, "DATA OUT")) == 0,
              "HV5812: VPP and HVOUT1 analog, DATA OUT not");
    part_library_destroy(lib);
    part = qc_load(&lib, "shift.json", "HV5812", E("units.0.storage.output", "'open-drain'"), NULL);
    qc_expect(part != NULL && part_pin_analog(part, qc_pin(part, "VPP")) == 1 &&
                  part_pin_analog(part, qc_pin(part, "HVOUT1")) == 0,
              "HV5812 with open-drain storage outputs: VPP analog, HVOUT1 not");
    part_library_destroy(lib);
}

int main(void) {
    check_kind_loader();
    check_unit_loader();
    check_bind();
    check_load_from_io();
    check_clocked_clear_unknown();
    check_storage_clear_unknown();
    check_clear_all_unknown();
    check_analog_supply();
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("shift_cov_test: all passed\n");
    return 0;
}
