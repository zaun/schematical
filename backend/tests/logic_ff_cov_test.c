/*
 * logic_ff_cov_test (plan-68-Q): the gate, dff and jkff loaders' rejections
 * and the flip-flop states no other program drives (an unknown preset,
 * clear, D, J or K), on the real devices of gate.json, dff.json and
 * jkff.json with one edit each.
 */

#include "model_pins.h"
#include "q_cov_bench.h"

#define E(...) ((const char *const[]){__VA_ARGS__, NULL})

static void check_gate_loader(void) {
    static const char G[] = "gate.json";

    qc_fail(G, "74_00", E("timing.HC.note", "1"), "timing.HC: \"note\" must be a string");
    qc_fail(G, "74_00", E("timing.HC", "1"), "timing.HC: a timing entry must be an object");
    qc_fail(G, "74_00", E("timing.HC.rise", "'x'"), "\"rise\" must be a time in ns");
    qc_fail(G, "74_86", E("timing.TTL.extra", "1"), "timing.TTL: unknown key \"extra\"");
    qc_fail(G, "74_86", E("timing.TTL.otherLow.fall", "'x'"),
            "timing.TTL: otherLow: \"fall\" must be a time in ns");
    qc_fail(G, "74_86", E("timing.TTL.otherHigh.rise", "-1"),
            "timing.TTL: otherHigh: \"rise\" must be a time in ns");
    qc_fail(G, "74_00", E("gates.0.extra", "1"), "gates[0]: unknown key \"extra\"");
    qc_fail(G, "74_00", E("gates.0.op", "'nope'"), "gates[0]: unknown op \"nope\"");
    qc_fail(G, "74_00",
            E("gates.0.inputs", "['1A','1A','1A','1A','1A','1A','1A','1A','1A','1A','1A','1A','1A','1A']"),
            "gates[0]: a gate has 1 to 13 inputs");
    qc_fail(G, "74_00", E("gates", NULL), "missing \"gates\"");
    qc_fail(G, "74_00", E("gates", "[]"), "\"gates\" must be a non-empty array");
    qc_fail(G, "74_00", E("output", "'totem'"), "\"output\" must be \"push-pull\" or \"open-drain\"");
    qc_fail(G, "74_00", E("units", "'four'"), "\"units\" must be a whole number");
    qc_fail(G, "74_00", E("timing", "{}"), "\"timing\" must be a non-empty object");
    qc_fail(G, "74_00",
            E("timing", "{'HC':{'rise':9,'fall':9,'vcc':[2,6]},'HC':{'rise':9,'fall':9,'vcc':[2,6]}}"),
            "duplicate key \"HC\"");
    qc_fail(G, "74_00", E("gates.0.inputs.1", "'NOPE'"), "gates[0]: no pin named \"NOPE\"");
    qc_fail(G, "74_00", E("gates.0.output", "'NOPE'"), "gates[0]: no pin named \"NOPE\"");
    qc_fail(G, "74_00", E("parts.0.family", "'ZZZ'"), "unknown family \"ZZZ\"");
    qc_fail(G, "74_00", E("parts.0.family", NULL), "missing \"family\"");
}

/* 74HC00 (sn74hc00 Table 1): any input L -> Y = H; an unknown input with
   the other H -> Y unknown. One NAND of each family of ops is enough to see
   the inverting step; the XOR of a 74x86 with an X input is X. */
static void check_gate_x(void) {
    PartLibrary *lib;
    Bench *b = qc_bench(&lib, "gate.json", "74_86", NULL, NULL);

    qc_set(b, "1A", L1);
    qc_set(b, "1B", LX);
    qc_step(b);
    qc_expect(qc_is(b, "1Y", LX), "74x86 1A = H, 1B = X -> 1Y = X (parity unknown)");
    qc_set(b, "1B", L1);
    qc_step(b);
    qc_expect(qc_is(b, "1Y", L0), "74x86 1A = 1B = H -> 1Y = L");
    bench_destroy(b);
    part_library_destroy(lib);
}

static void check_dff_loader(void) {
    static const char D[] = "dff.json";

    qc_fail(D, "74_74", E("clockEdge", NULL), "missing \"clockEdge\"");
    qc_fail(D, "74_74", E("clockEdge", "'both'"), "\"clockEdge\" must be \"rising\" or \"falling\"");
    qc_fail(D, "74_74", E("output", "1"), "\"output\" must be \"push-pull\" or \"open-drain\"");
    qc_fail(D, "74_74", E("units", NULL), "missing \"units\"");
    qc_fail(D, "74_74", E("units", "[]"), "\"units\" must be an array of 1 to 8 units");
    qc_fail(D, "74_74", E("units.0.d", NULL), "units[0]: missing \"d\"");
    qc_fail(D, "74_74", E("timing", "{}"), "\"timing\" must be a non-empty object");
    qc_fail(D, "74_74",
            E("timing", "{'HC':{'rise':1,'fall':1,'clearWins':false},'HC':{'rise':1,'fall':1,'clearWins':false}}"),
            "duplicate key \"HC\"");
    qc_fail(D, "74_74", E("timing.HC.fall", "'x'"), "timing.HC: \"fall\" must be a time in ns");
    qc_fail(D, "74_74", E("timing.HC.note", "2"), "timing.HC: \"note\" must be a string");
    qc_fail(D, "74_74", E("timing.HC.vcc", "'5'"), "timing.HC: \"vcc\" must be [min, max] in volts");
    qc_fail(D, "74_74", E("units.0.clr", "'NOPE'"), "units[0]: no pin named \"NOPE\"");
    qc_fail(D, "74_374", E("outputEnable", "'NOPE'"), "no pin named \"NOPE\"");
    qc_fail(D, "74_74", E("parts.0.family", "'QQ'"), "unknown family \"QQ\"");
    qc_fail(D, "74_74", E("parts.0.family", "'LS'", "timing.LS", NULL), "family \"LS\" has no timing entry");
}

/* SN74HC74 function table: PRE/CLR low act at once; an unknown PRE or CLR
   leaves Q and QN unknown; a rising CLK with D unknown stores an unknown. */
static void check_dff_x(void) {
    PartLibrary *lib;
    Bench *b = qc_bench(&lib, "dff.json", "74_74", NULL, NULL);

    qc_set(b, "1PRE", L1);
    qc_set(b, "1CLR", L1);
    qc_set(b, "1D", L1);
    qc_set(b, "1CLK", L0);
    qc_step(b);
    qc_set(b, "1CLK", L1);
    qc_step(b);
    qc_expect(qc_is(b, "1Q", L1) && qc_is(b, "1QBAR", L0), "74HC74 D = H, CLK rises -> Q = H");
    qc_set(b, "1PRE", LX);
    qc_step(b);
    qc_expect(qc_is(b, "1Q", LX) && qc_is(b, "1QBAR", LX), "74HC74 PRE = X -> Q = QN = X");
    qc_set(b, "1PRE", L1);
    qc_set(b, "1CLR", L0);
    qc_step(b);
    qc_expect(qc_is(b, "1Q", L0) && qc_is(b, "1QBAR", L1), "74HC74 CLR low -> Q = L");
    qc_set(b, "1CLR", L1);
    qc_set(b, "1CLK", L0);
    qc_set(b, "1D", LX);
    qc_step(b);
    qc_set(b, "1CLK", L1);
    qc_step(b);
    qc_expect(qc_is(b, "1Q", LX) && qc_is(b, "1QBAR", LX), "74HC74 D = X, CLK rises -> Q = QN = X");
    bench_destroy(b);
    part_library_destroy(lib);
}

/* An output pin no unit names is neither open-drain (TPIC6273) nor 3-state
   (74x374): the hooks answer only for the units' Q/QN. */
static void check_dff_pin_hooks(void) {
    PartLibrary *lib;
    const Part *part = qc_load(&lib, "dff.json", "TPIC6273", E("units.0.qn", NULL), NULL);

    qc_expect(part != NULL && part_pin_open_drain(part, qc_pin(part, "DRAIN2")) == 1 &&
                  part_pin_open_drain(part, qc_pin(part, "DRAIN1")) == 0,
              "TPIC6273: DRAIN2 open-drain; DRAIN1, named by no unit, is not");
    part_library_destroy(lib);
    part = qc_load(&lib, "dff.json", "74_374", E("units.0.q", NULL), NULL);
    qc_expect(part != NULL && part_pin_tristate(part, qc_pin(part, "2Q")) == 1 &&
                  part_pin_tristate(part, qc_pin(part, "1Q")) == 0,
              "74x374: 2Q 3-state; 1Q, named by no unit, is not");
    part_library_destroy(lib);
}

static void check_jk_loader(void) {
    static const char J[] = "jkff.json";

    qc_fail(J, "74_76", E("units", NULL), "missing \"units\"");
    qc_fail(J, "74_76", E("units", "[{},{},{}]"), "\"units\" must be an array of 1 to 2 units");
    qc_fail(J, "74_76", E("units.0.x", "'1J'"), "units[0]: unknown key \"x\"");
    qc_fail(J, "74_76", E("kInverted", "'no'"), "\"kInverted\" must be true or false");
    qc_fail(J, "74_76", E("clockEdge", "'up'"), "\"clockEdge\" must be \"rising\" or \"falling\", not \"up\"");
    qc_fail(J, "74_76", E("timing", "[]"), "\"timing\" must be a non-empty object");
    qc_fail(J, "74_76", E("timing", "{'LS':{'rise':1,'fall':1},'LS':{'rise':1,'fall':1}}"),
            "duplicate key \"LS\"");
    qc_fail(J, "74_76", E("timing.LS.note", "[]"), "timing.LS: \"note\" must be a string");
    qc_fail(J, "74_76", E("timing.LS.inputMax", "7", "timing.LS.vcc", NULL),
            "timing.LS: \"inputMax\" without \"vcc\"");
    qc_fail(J, "74_76", E("units.1.k", "'NOPE'"), "units[1]: no pin named \"NOPE\"");
    qc_fail(J, "74_76", E("parts.0.family", "7"), "\"family\" must be a string");
    qc_fail(J, "74_76", E("parts.0.family", "'HC'"), "family \"HC\" has no timing entry");
}

/* SN74LS76A function table: PRE/CLR low act at once (an unknown one -> Q
   unknown); on the CLK fall J = K = H toggles (an unknown Q stays unknown),
   J = L K = L holds; with J unknown, hold (Q = L) and set differ -> unknown. */
static void check_jk_x(void) {
    PartLibrary *lib;
    Bench *b = qc_bench(&lib, "jkff.json", "74_76", NULL, NULL);

    qc_set(b, "1PRE", L1);
    qc_set(b, "1CLR", L1);
    qc_set(b, "1J", L0);
    qc_set(b, "1K", L0);
    qc_set(b, "1CLK", L1);
    qc_step(b);
    qc_set(b, "1J", LX);
    qc_set(b, "1CLK", L0);
    qc_step(b);
    qc_expect(qc_is(b, "1Q", LX) && qc_is(b, "1QBAR", LX),
              "74LS76 Q = L, J = X, K = L, CLK falls -> Q = X (hold or set)");
    qc_set(b, "1J", L1);
    qc_set(b, "1K", L1);
    qc_set(b, "1CLK", L1);
    qc_step(b);
    qc_set(b, "1CLK", L0);
    qc_step(b);
    qc_expect(qc_is(b, "1Q", LX), "74LS76 Q = X, J = K = H (toggle), CLK falls -> Q = X");
    qc_set(b, "1CLR", L0);
    qc_step(b);
    qc_expect(qc_is(b, "1Q", L0) && qc_is(b, "1QBAR", L1), "74LS76 CLR low -> Q = L");
    qc_set(b, "1CLR", LX);
    qc_step(b);
    qc_expect(qc_is(b, "1Q", LX) && qc_is(b, "1QBAR", LX), "74LS76 CLR = X -> Q = QN = X");
    bench_destroy(b);
    part_library_destroy(lib);

    /* A unit without QN (pin left to no unit) still drives its Q. */
    b = qc_bench(&lib, "jkff.json", "74_76", E("units.0.qn", NULL), NULL);
    qc_set(b, "1PRE", L0);
    qc_set(b, "1CLR", L1);
    qc_step(b);
    qc_expect(qc_is(b, "1Q", L1) && qc_is(b, "1QBAR", LZ),
              "74LS76 unit without qn: PRE low -> Q = H; QBAR not driven");
    bench_destroy(b);
    part_library_destroy(lib);
}

int main(void) {
    check_gate_loader();
    check_gate_x();
    check_dff_loader();
    check_dff_x();
    check_dff_pin_hooks();
    check_jk_loader();
    check_jk_x();
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("logic_ff_cov_test: all passed\n");
    return 0;
}
