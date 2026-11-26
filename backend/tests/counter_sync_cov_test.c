/*
 * counter_sync_cov_test (plan-68-Q): the counter kind's and the sync model's
 * loader rejections (one per message, each the fixture device of
 * tests/fixtures/counter with one edit), the shared timing loader (seq.c),
 * and the sync counter states no other case drives: an unknown BCD digit,
 * an unknown code select, an unknown count enable, a "hold" mode row, an
 * unknown input-register enable and two clocks rising at once.
 */

#include "q_cov_bench.h"

#define E(...) ((const char *const[]){__VA_ARGS__, NULL})
#define FX "fx:counter/parts.json"

static void fail(const char *id, const char *const *edits, const char *expect) {
    qc_fail(FX, id, edits, expect);
}

/* counter.c (the kind) and seq.c (the timing table, the family). */
static void check_kind_loader(void) {
    fail("fx-74_161", E("model", NULL), "missing \"model\"");
    fail("fx-74_161", E("model", "'gray'"), "unknown model \"gray\"");
    fail("fx-74_161", E("stages", "5"), "\"stages\" is not a key of model \"sync\"");
    fail("fx-74_161", E("units.0.q.0", "'NOPE'"), "no pin named \"NOPE\"");
    fail("fx-74_161", E("units.0.enables", "[]"), "\"enables\" must be an array of 1 to 3 controls");
    fail("fx-74_161", E("units.0.enables.0.active", NULL), "enables[0]: missing \"active\"");
    fail("fx-74_90", E("timing.HC.stage", NULL), "timing.HC: missing \"stage\"");
    fail("fx-74_90", E("timing.HC.stage", "'x'"), "timing.HC: \"stage\" must be a time in ns");
    fail("fx-74_161", E("parts.0.family", "'NOPE'"), "unknown family \"NOPE\"");
    fail("fx-74_161", E("parts.0.family", "'LS'"), "family \"LS\" has no timing entry");
    fail("fx-74_161", E("timing", NULL), "missing \"timing\"");
    fail("fx-74_161", E("timing", "{}"), "\"timing\" must be a non-empty object");
    fail("fx-74_161", E("timing", "{'HC':{'rise':1,'fall':1},'HC':{'rise':1,'fall':1}}"),
         "duplicate key \"HC\"");
    fail("fx-74_161", E("timing.HC.note", "false"), "timing.HC: \"note\" must be a string");
    fail("fx-74_161", E("timing.HC.vcc", "[6,2]"), "timing.HC: \"vcc\": min must be");
    fail("fx-4518B", E("units.0.clockGate.x", "1"), "clockGate: unknown key \"x\"");
    fail("fx-4518B", E("units.0.clockGate.op", "'xor'"),
         "\"op\" must be \"and-not\", \"or\" or \"or-not\", not \"xor\"");
}

/* sync: every rejection of the unit, its lists and the byte bus. */
static void check_sync_loader(void) {
    fail("fx-74_161", E("units.0.clears", "[]"), "\"clears\" must be an array of 1 to 3 objects");
    fail("fx-74_161", E("units.0.clears.0.active", NULL), "clears[0]: missing \"active\"");
    fail("fx-74_161", E("units.0.clears.0.when", "'never'"),
         "clears[0]: \"when\" must be \"async\" or \"clock\", not \"never\"");
    fail("fx-74_161", E("units.0.clears.0.to", "'one'"),
         "clears[0]: \"to\" must be \"zero\" or \"max\", not \"one\"");
    fail("fx-74_161", E("units.0.carries", "[]"), "\"carries\" must be an array of 1 to 3 objects");
    fail("fx-74_161", E("units.0.carries.0.x", "1"), "carries[0]: unknown key \"x\"");
    fail("fx-74_161", E("units.0.carries.0.at", "'mid'"),
         "carries[0]: \"at\" must be \"max\", \"min\" or \"terminal\", not \"mid\"");
    fail("fx-74_867", E("units.0.modes.x", "1"), "modes: unknown key \"x\"");
    fail("fx-74_867", E("units.0.modes.table.1", "'jump'"), "modes: table[1]: unknown mode \"jump\"");
    fail("fx-74_867", E("units.0.modes.clear", "'never'"),
         "modes: \"clear\" must be \"async\" or \"clock\", not \"never\"");
    fail("fx-74_190", E("units.0.illegal.x", "[]"), "illegal: unknown key \"x\"");
    fail("fx-74_190", E("units.0.illegal.up", NULL), "illegal: missing \"up\"");
    fail("fx-74_190", E("units.0.illegal.up", "[1]"), "illegal: \"up\" must be 16 numbers 0 to 15");
    fail("fx-74_190", E("units.0.illegal.down.12", "16"), "illegal: \"down\" must be 16 numbers 0 to 15");
    fail("fx-74_190", E("units.0.illegal.up.0", "5"), "illegal: up[0] must be 1 (the normal sequence)");
    fail("fx-74_590", E("units.0.register.x", "1"), "register: unknown key \"x\"");
    fail("fx-74_697", E("units.0.register.select.x", "1"), "register: select: unknown key \"x\"");
    fail("fx-74_592", E("units.0.inputRegister.x", "1"), "inputRegister: unknown key \"x\"");
    fail("fx-74_593", E("units.0.bus.x", "1"), "bus: unknown key \"x\"");
    fail("fx-74_161", E("units.0.x", "1"), "units[0]: unknown key \"x\"");
    fail("fx-4029B", E("units.0.code", "'bcd'"), "\"codeSelect\" and \"code\" cannot both be given");
    fail("fx-4029B", E("units.0.stages", "8"), "a \"codeSelect\" unit has 4 stages");
    fail("fx-4029B", E("units.0.codeSelect.x", "1"), "codeSelect: unknown key \"x\"");
    fail("fx-74_161", E("units.0.code", NULL), "units[0]: missing \"code\"");
    fail("fx-74_161", E("units.0.code", "'gray'"), "\"code\" must be \"binary\" or \"bcd\", not \"gray\"");
    fail("fx-74_161", E("units.0.q", "['QA','QB','QC']"), "\"q\"");
    fail("fx-74_193", E("units.0.clock", "{'pin':'UP','edge':'rising'}"),
         "\"clock\" and \"clockUp\"/\"clockDown\" cannot both be given");
    fail("fx-74_193", E("units.0.clockDown", NULL), "units[0]: missing \"clockDown\"");
    fail("fx-74_161", E("units.0.clock", NULL), "units[0]: missing \"clock\"");
    fail("fx-74_161", E("units.0.count", "'sideways'"), "\"count\" must be \"up\" or \"down\"");
    fail("fx-74_169", E("units.0.direction.x", "1"), "direction: unknown key \"x\"");
    fail("fx-74_593", E("units.0.enablesAny", "[]"), "\"enablesAny\" must be an array of 1 to 3 controls");
    fail("fx-74_161", E("units.0.loads", "[]"), "\"loads\" must be an array of 1 to 3 objects");
    fail("fx-74_161", E("units.0.data", NULL), "a load from \"data\" needs \"data\"");
    fail("fx-74_161", E("units.0.loads.0.from", "'register'"),
         "a load from \"register\" needs \"inputRegister\"");
    fail("fx-74_867", E("units.0.data", NULL), "\"modes\" needs \"data\"");
    fail("fx-74_593", E("units.0.bus", NULL), "\"inputRegister\" needs \"data\" or a \"bus\"");
    fail("fx-74_161", E("units.0.count", NULL), "the count direction needs");
    fail("fx-4522B", E("units.0.zeroLatch.x", "1"), "zeroLatch: unknown key \"x\"");
    fail("fx-4522B", E("units.0.loads", NULL), "\"zeroLatch\" needs a load (its preset enable)");
    fail("fx-74_161", E("units", NULL), "missing \"units\"");
    fail("fx-74_161", E("units", "[]"), "\"units\" must be an array of 1 to 2 units");
    fail("fx-74_8154", E("byteBus.x", "1"), "byteBus: unknown key \"x\"");
    fail("fx-74_8154", E("byteBus.selects", "[]"), "byteBus: \"selects\" must be an array of 1 to 4 objects");
    fail("fx-74_8154", E("byteBus.selects.0.x", "1"), "selects[0]: unknown key \"x\"");
    fail("fx-74_8154", E("byteBus.selects.0.byte", "3"), "unit 0 has no register byte 3");
}

/* sync: a name the pinout does not have, in each list the binder walks. */
static void check_sync_bind(void) {
    static const char NOPE[] = "no pin named \"NOPE\"";

    fail("fx-74_867", E("units.0.modes.pins.1", "'NOPE'"), NOPE);
    fail("fx-74_161", E("units.0.enables.1.pin", "'NOPE'"), NOPE);
    fail("fx-74_593", E("units.0.enablesAny.1.pin", "'NOPE'"), NOPE);
    fail("fx-74_161", E("units.0.clears.0.pin", "'NOPE'"), NOPE);
    fail("fx-74_161", E("units.0.loads.0.pin", "'NOPE'"), NOPE);
    fail("fx-74_161", E("units.0.carries.0.pin", "'NOPE'"), NOPE);
    fail("fx-74_161", E("units.0.carries.0.gates.0.pin", "'NOPE'"), NOPE);
    fail("fx-74_593", E("units.0.bus.outputEnables.1.pin", "'NOPE'"), NOPE);
    fail("fx-74_8154", E("byteBus.outputs.7", "'NOPE'"), NOPE);
    fail("fx-74_8154", E("byteBus.selects.0.pin", "'NOPE'"), NOPE);
}

static const char *const Q4[] = {"QA", "QB", "QC", "QD"};
static const char *const Q8[] = {"QA", "QB", "QC", "QD", "QE", "QF", "QG", "QH"};
static const char *const D8[] = {"A", "B", "C", "D", "E", "F", "G", "H"};

/*
 * BCD (plan-10-D § 5: each digit steps when every lower digit is at its
 * terminal): the 74x867 set to two BCD digits, loaded with an unknown low
 * digit and 5 in the high digit. Counting up, the low digit stays unknown and
 * the high digit is 5 or 6: QE, QF unknown, QG = 1, QH = 0.
 */
static void check_bcd_unknown_digit(void) {
    PartLibrary *lib;
    Bench *b = qc_bench(&lib, FX, "fx-74_867", E("units.0.code", "'bcd'"), NULL);
    int ok;

    qc_set(b, "ENP", L0);
    qc_set(b, "ENT", L0);
    qc_set(b, "CLK", L0);
    qc_set(b, "S0", L0);
    qc_set(b, "S1", L1); /* load */
    qc_set_bus(b, D8, 8, 0x50);
    qc_set(b, "A", LX);
    qc_step(b);
    qc_pulse(b, "CLK");
    qc_set(b, "S0", L1); /* up */
    qc_step(b);
    qc_pulse(b, "CLK");
    ok = qc_bus_all(b, Q8, 4, LX) && qc_is(b, "QE", LX) && qc_is(b, "QF", LX) && qc_is(b, "QG", L1) &&
         qc_is(b, "QH", L0);
    qc_expect(ok, "BCD x2 at ?5 (low digit unknown) counts up: low digit X, high digit 5 or 6");
    bench_destroy(b);
    part_library_destroy(lib);
}

/* 4029B: BINARY/DECADE unknown at count 9 counting up: binary gives 10
   (1010), decade 0 (0000): Q1 = 0, Q2 = X, Q3 = 0, Q4 = X. */
static void check_code_select_unknown(void) {
    static const char *const JAM[] = {"JAM1", "JAM2", "JAM3", "JAM4"};
    PartLibrary *lib;
    Bench *b = qc_bench(&lib, FX, "fx-4029B", NULL, NULL);

    qc_set(b, "CLOCK", L0);
    qc_set(b, "CARRYIN", L0);
    qc_set(b, "UP/DOWN", L1);
    qc_set(b, "BINARY/DECADE", L0);
    qc_set_bus(b, JAM, 4, 9);
    qc_set(b, "PE", L1);
    qc_step(b);
    qc_set(b, "PE", L0);
    qc_set(b, "BINARY/DECADE", LX);
    qc_step(b);
    qc_pulse(b, "CLOCK");
    qc_expect(qc_is(b, "Q1", L0) && qc_is(b, "Q2", LX) && qc_is(b, "Q3", L0) && qc_is(b, "Q4", LX),
              "4029B at 9, BINARY/DECADE = X, clock up: 10 or 0 -> Q1 = 0, Q2 = X, Q3 = 0, Q4 = X");
    bench_destroy(b);
    part_library_destroy(lib);
}

/* 74x161: ENP unknown at count 0: the counter holds or steps to 1 -> QA = X,
   QB..QD = 0. A "hold" mode row (the 74x867 table with row S1 S0 = 10 set to
   hold) keeps the count on the clock. */
static void check_unknown_enable_and_hold(void) {
    PartLibrary *lib;
    Bench *b = qc_bench(&lib, FX, "fx-74_161", NULL, NULL);

    qc_set(b, "CLR", L1);
    qc_set(b, "LOAD", L1);
    qc_set(b, "ENT", L1);
    qc_set(b, "ENP", LX);
    qc_set(b, "CLK", L0);
    qc_step(b);
    qc_pulse(b, "CLK");
    qc_expect(qc_is(b, "QA", LX) && qc_is(b, "QB", L0) && qc_is(b, "QC", L0) && qc_is(b, "QD", L0),
              "74x161 at 0, ENP = X, clock: QA = X (0 or 1), QB-QD = 0");
    bench_destroy(b);
    part_library_destroy(lib);

    b = qc_bench(&lib, FX, "fx-74_867", E("units.0.modes.table.2", "'hold'"), NULL);
    qc_set(b, "ENP", L0);
    qc_set(b, "ENT", L0);
    qc_set(b, "CLK", L0);
    qc_set(b, "S0", L1);
    qc_set(b, "S1", L1); /* up */
    qc_step(b);
    qc_pulse(b, "CLK");
    qc_pulse(b, "CLK");
    qc_set(b, "S0", L0); /* S1 S0 = 10: hold */
    qc_step(b);
    qc_pulse(b, "CLK");
    qc_pulse(b, "CLK");
    qc_expect(qc_bus(b, Q8, 8, 2), "74x867 with a hold row: two counts, then two held clocks -> 2");
    bench_destroy(b);
    part_library_destroy(lib);
}

/* 74x593: RCKEN unknown when RCK rises: the input register keeps or takes
   the bus, so the bits that differ (low nibble 0 vs 1) are unknown; a
   CLOAD copies them to the counter, seen on the bus with G high. */
static void check_input_register_enable_unknown(void) {
    static const char *const BUS[] = {"A/QA", "B/QB", "C/QC", "D/QD", "E/QE", "F/QF", "G/QG", "H/QH"};
    PartLibrary *lib;
    Bench *b = qc_bench(&lib, FX, "fx-74_593", NULL, NULL);
    unsigned i;
    int ok = 1;

    qc_set(b, "CCLR", L1);
    qc_set(b, "CLOAD", L1);
    qc_set(b, "CCK", L0);
    qc_set(b, "CCKEN", L0);
    qc_set(b, "CCKENBAR", L1);
    qc_set(b, "G", L0);
    qc_set(b, "GBAR", L1);
    qc_set(b, "RCK", L0);
    qc_set(b, "RCKEN", LX);
    qc_set_bus(b, BUS, 8, 0x0F);
    qc_step(b);
    qc_set(b, "RCK", L1);
    qc_step(b);
    qc_set(b, "CLOAD", L0);
    qc_step(b);
    qc_set(b, "CLOAD", L1);
    for (i = 0; i < 8; i++) {
        qc_set(b, BUS[i], LZ);
    }
    qc_set(b, "G", L1);
    qc_set(b, "GBAR", L0);
    qc_step(b);
    for (i = 0; i < 8; i++) {
        ok &= qc_is(b, BUS[i], i < 4 ? LX : L0);
    }
    qc_expect(ok, "74x593 RCK rises with RCKEN = X on bus 0x0F; CLOAD -> count low nibble X, high 0");
    bench_destroy(b);
    part_library_destroy(lib);
}

/* 74x193 (sn74ls193 function table: one clock rises while the other is
   high): UP and DOWN wired together rise at once from count 0, so the
   count may be 0, 1 or 15 -> every Q unknown. */
static void check_both_clocks(void) {
    static const char *const JOIN[] = {"UP", "DOWN", NULL};
    PartLibrary *lib;
    Bench *b = qc_bench_joined(&lib, FX, "fx-74_193", NULL, JOIN);

    qc_set(b, "LOAD", L1);
    qc_set(b, "CLR", L1);
    qc_set(b, "UP", L0);
    qc_step(b);
    qc_set(b, "CLR", L0);
    qc_step(b);
    qc_set(b, "UP", L1);
    qc_step(b);
    qc_expect(qc_bus_all(b, Q4, 4, LX), "74x193 UP and DOWN rise together at 0: QA-QD = X");
    bench_destroy(b);
    part_library_destroy(lib);
}

int main(void) {
    check_kind_loader();
    check_sync_loader();
    check_sync_bind();
    check_bcd_unknown_digit();
    check_code_select_unknown();
    check_unknown_enable_and_hold();
    check_input_register_enable_unknown();
    check_both_clocks();
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("counter_sync_cov_test: all passed\n");
    return 0;
}
