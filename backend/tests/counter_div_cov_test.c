/*
 * counter_div_cov_test (plan-68-Q): the ripple, seg7, johnson, divn and rate
 * counter models' loader rejections (fixture devices of tests/fixtures/counter
 * and the real SN74143, one edit each), and the states no other case drives:
 * a ripple section with unknown bits clocked, a seg7 reset or latch enable
 * unknown, the SN74143 BI without RBI/RBO, a 4059 jam pin or mode unknown, a
 * rate multiplier with an unknown count.
 */

#include "q_cov_bench.h"

#define E(...) ((const char *const[]){__VA_ARGS__, NULL})
#define FX "fx:counter/parts.json"

static void fail(const char *id, const char *const *edits, const char *expect) {
    qc_fail(FX, id, edits, expect);
}

static void check_ripple_loader(void) {
    fail("fx-74_90", E("sections.0.reset.x", "1"), "sections[0]: reset: unknown key \"x\"");
    fail("fx-74_90", E("sections.0.set.value", "'x'"), "set: \"value\" must be a whole number");
    fail("fx-74_90", E("sections", NULL), "missing \"sections\"");
    fail("fx-74_90", E("units", "0"), "\"units\" must be 1 to 8");
    fail("fx-74_90", E("sections", "[]"), "\"sections\" must be an array of 1 to");
    fail("fx-74_90", E("sections.0.x", "1"), "sections[0]: unknown key \"x\"");
    fail("fx-74_93", E("sections.1.stages", "'x'"), "sections[1]: \"stages\" must be a whole number");
    fail("fx-74_93", E("sections.1.stages", "0"), "sections[1]: \"stages\" must be 1 to");
    fail("fx-74_90", E("sections.0.structure", "'div3'"),
         "\"structure\" must be \"binary\", \"div2\", \"div5\" or \"div6\", not \"div3\"");
    fail("fx-74_90", E("sections.0.stages", "1"), "\"stages\" is only for a binary section");
    fail("fx-74_90", E("sections.0.q", NULL), "sections[0]: missing \"q\"");
    fail("fx-74_90", E("sections.0.q", "['QA','QB']"), "\"q\" must have 1 entries");
    fail("fx-74_90", E("sections.0.q", "[5]"), "\"q\" entries must be pin names or null");
    fail("fx-74_90", E("sections.0.set.active", "'mid'"), "set: \"active\" must be \"high\" or \"low\"");
    fail("fx-74_90", E("sections.1.set.value", "9"), "set: \"value\" does not fit 3 stages");
    fail("fx-74_90", E("sections.0.clock", "'NOPE'"), "no pin named \"NOPE\"");
    fail("fx-74_90", E("sections.1.q.2", "'NOPE'"), "no pin named \"NOPE\"");
}

static void check_seg7_loader(void) {
    fail("fx-40110B", E("toggleEnable", "{'pin':'TE'}"), "missing \"active\"");
    fail("fx-40110B", E("toggleEnable", "[{'pin':'TE'}]"), "toggleEnable: [0]: missing \"active\"");
    fail("fx-40110B", E("clockDown", NULL), "missing \"clockDown\"");
    fail("fx-4026B", E("clock", NULL), "missing \"clock\"");
    fail("fx-4026B", E("segmentsOutput", "1"), "\"segmentsOutput\" must be a string");
    fail("fx-4026B", E("glyphs.3", "'111111'"), "glyphs[3]: must be 7 characters 0 or 1");
    fail("fx-4026B", E("glyphs.4", "'01100x1'"), "glyphs[4]: must be 7 characters 0 or 1");
    fail("fx-4026B", E("clock", "'NOPE'"), "no pin named \"NOPE\"");
    fail("fx-40110B", E("toggleEnable.pin", "'NOPE'"), "no pin named \"NOPE\"");
    qc_fail("counter.json", "SN74143", E("latchOutputs.3", "'NOPE'"), "no pin named \"NOPE\"");
    qc_fail("counter.json", "SN74143", E("pinouts.p24.5.role", "'output'"),
            "\"rbo\" pin \"BI/RBO\" must have role inout");
}

static void check_johnson_divn_rate_loader(void) {
    fail("fx-4017B", E("stages", "'x'"), "\"stages\" must be a whole number");
    fail("fx-4017B", E("stages", "3"), "\"stages\" must be 4 or 5");
    fail("fx-4017B", E("clock", NULL), "missing \"clock\"");
    fail("fx-4017B", E("outputs", "['0','1','2','3','4','5','6','7','8']"), "\"outputs\"");
    fail("fx-4018B", E("qbar", "['Q1','Q2','Q3','Q4']"), "\"qbar\"");
    fail("fx-4018B", E("jam", "['JAM1','JAM2','JAM3','JAM4']"), "\"jam\"");
    fail("fx-4017B", E("jam", "['0','1','2','3','4']"), "\"jam\" needs \"presetEnable\"");
    fail("fx-4017B", E("outputs.9", "'NOPE'"), "no pin named \"NOPE\"");
    fail("fx-4059A", E("clock", "'NOPE'"), "no pin named \"NOPE\"");
    fail("fx-4089B", E("variant", NULL), "missing \"variant\"");
    fail("fx-4089B", E("out", NULL), "missing \"out\"");
    fail("fx-4089B", E("clock", "'NOPE'"), "no pin named \"NOPE\"");
}

/*
 * Ripple sections clocked from a partly unknown state (a reset at X merges
 * the count with 0). 4040B (12-stage binary) at 3, RESET = X: Q1, Q2 unknown;
 * a clock fall adds 1: Q1-Q3 unknown, Q4 on still 0. 74x90 div-5 at 4
 * (QD = 1), R0 = X: QD unknown, QB = QC = 0; a CKB fall takes 0 -> 1 or
 * 4 -> 0: QB unknown, QC = QD = 0.
 */
static void check_ripple_unknown(void) {
    static const char *const Q12[] = {"Q1", "Q2", "Q3", "Q4", "Q5", "Q6", "Q7", "Q8", "Q9", "Q10",
                                      "Q11", "Q12"};
    PartLibrary *lib;
    Bench *b = qc_bench(&lib, FX, "fx-4040B", NULL, NULL);
    int ok;

    qc_set(b, "RESET", L1);
    qc_set(b, "CLK", L1);
    qc_step(b);
    qc_set(b, "RESET", L0);
    qc_step(b);
    qc_set(b, "CLK", L0);
    qc_step(b);
    qc_set(b, "CLK", L1);
    qc_step(b);
    qc_set(b, "CLK", L0);
    qc_step(b);
    qc_set(b, "CLK", L1);
    qc_step(b);
    qc_set(b, "CLK", L0);
    qc_step(b);
    ok = qc_bus(b, Q12, 12, 3);
    qc_set(b, "RESET", LX);
    qc_step(b);
    ok &= qc_is(b, "Q1", LX) && qc_is(b, "Q2", LX) && qc_is(b, "Q3", L0);
    qc_set(b, "RESET", L0);
    qc_set(b, "CLK", L1);
    qc_step(b);
    qc_set(b, "CLK", L0);
    qc_step(b);
    qc_expect(ok && qc_bus_all(b, Q12, 3, LX) && qc_is(b, "Q4", L0) && qc_is(b, "Q12", L0),
              "4040B at 3, RESET = X -> Q1, Q2 = X; clock -> Q1-Q3 = X, Q4-Q12 = 0");
    bench_destroy(b);
    part_library_destroy(lib);

    b = qc_bench(&lib, FX, "fx-74_90", NULL, NULL);
    qc_set(b, "R9(1)", L0);
    qc_set(b, "R9(2)", L0);
    qc_set(b, "R0(1)", L1);
    qc_set(b, "R0(2)", L1);
    qc_set(b, "CKA", L1);
    qc_set(b, "CKB", L1);
    qc_step(b);
    qc_set(b, "R0(1)", L0);
    qc_step(b);
    {
        int i;

        for (i = 0; i < 4; i++) {
            qc_set(b, "CKB", L0);
            qc_step(b);
            qc_set(b, "CKB", L1);
            qc_step(b);
        }
    }
    ok = qc_is(b, "QB", L0) && qc_is(b, "QC", L0) && qc_is(b, "QD", L1);
    qc_set(b, "R0(1)", LX);
    qc_step(b);
    ok &= qc_is(b, "QD", LX) && qc_is(b, "QB", L0) && qc_is(b, "QC", L0);
    qc_set(b, "R0(1)", L0);
    qc_set(b, "CKB", L0);
    qc_step(b);
    qc_expect(ok && qc_is(b, "QB", LX) && qc_is(b, "QC", L0) && qc_is(b, "QD", L0),
              "74x90 div-5 at 4, R0 = X -> QD = X; CKB falls (0->1 or 4->0) -> QB = X, QC = QD = 0");
    bench_destroy(b);
    part_library_destroy(lib);
}

/* CD40110B: RESET high clears; RESET unknown with a count other than 0 makes
   the count (and the shown digit) unknown. LE high holds the display; LE
   unknown while the display and the count differ makes the display unknown. */
static void check_seg7_unknown(void) {
    PartLibrary *lib;
    Bench *b = qc_bench(&lib, FX, "fx-40110B", NULL, NULL);
    int ok;
    int i;

    qc_set(b, "TE", L0);
    qc_set(b, "LE", L0);
    qc_set(b, "CLOCKDOWN", L0);
    qc_set(b, "CLOCKUP", L0);
    qc_set(b, "RESET", L1);
    qc_step(b);
    qc_set(b, "RESET", L0);
    qc_step(b);
    qc_set(b, "RESET", LX);
    qc_step(b);
    ok = qc_is(b, "g", L0) && qc_is(b, "a", L1); /* still 0: reset X at 0 */
    qc_set(b, "RESET", L0);
    for (i = 0; i < 3; i++) {
        qc_pulse(b, "CLOCKUP");
    }
    ok &= qc_is(b, "g", L1); /* 3 */
    qc_set(b, "RESET", LX);
    qc_step(b);
    qc_expect(ok && qc_is(b, "g", LX) && qc_is(b, "a", LX),
              "40110B RESET = X at 0 keeps 0; at 3 the digit becomes unknown (segments X)");
    bench_destroy(b);
    part_library_destroy(lib);

    b = qc_bench(&lib, FX, "fx-40110B", NULL, NULL);
    qc_set(b, "TE", L0);
    qc_set(b, "LE", L0);
    qc_set(b, "CLOCKDOWN", L0);
    qc_set(b, "CLOCKUP", L0);
    qc_set(b, "RESET", L1);
    qc_step(b);
    qc_set(b, "RESET", L0);
    qc_set(b, "LE", L1);
    qc_step(b);
    qc_pulse(b, "CLOCKUP");
    ok = qc_is(b, "a", L1); /* shows the held 0; the count is 1 */
    qc_set(b, "LE", LX);
    qc_step(b);
    qc_expect(ok && qc_is(b, "a", LX), "40110B LE high holds 0 while counting to 1; LE = X -> a = X");
    bench_destroy(b);
    part_library_destroy(lib);
}

/* SN74143 without RBI/RBO (BI alone): BI high blanks every segment (the
   open-drain outputs off); BI low shows the digit (0: segment a on, low). */
static void check_seg7_blank_alone(void) {
    PartLibrary *lib;
    Bench *b = qc_bench(&lib, "counter.json", "SN74143", E("rbi", NULL, "rbo", NULL), NULL);
    int ok;

    qc_set(b, "SCEI", L1);
    qc_set(b, "PCEI", L1);
    qc_set(b, "CLK", L0);
    qc_set(b, "STRB", L0);
    qc_set(b, "DP", L0);
    qc_set(b, "RBI", L1);
    qc_set(b, "BI", L0);
    qc_set(b, "CLR", L0);
    qc_step(b);
    qc_set(b, "CLR", L1);
    qc_step(b);
    ok = qc_is(b, "a", L0) && qc_is(b, "g", LZ);
    qc_set(b, "BI", L1);
    qc_step(b);
    qc_expect(ok && qc_is(b, "a", LZ) && qc_is(b, "d", LZ),
              "SN74143 (BI only) at 0: a on (low), g off; BI high -> every segment off (Z)");
    bench_destroy(b);
    part_library_destroy(lib);
}

/* CD4059A Table I: Kb = Kc = 0 is the master preset; Ka Kb Kc = 010 is not a
   listed mode, and a jam pin unknown leaves N unknown: either way the
   output is unknown after the next clock. */
static void check_divn_unknown(void) {
    static const char *const J[] = {"J1", "J2", "J3", "J4", "J5", "J6", "J7", "J8",
                                    "J9", "J10", "J11", "J12", "J13", "J14", "J15", "J16"};
    int pass;

    for (pass = 0; pass < 2; pass++) {
        PartLibrary *lib;
        Bench *b = qc_bench(&lib, FX, "fx-4059A", NULL, NULL);
        int ok;

        qc_set(b, "L", L0);
        qc_set(b, "CL", L0);
        qc_set_bus(b, J, 16, 0x0010);
        qc_set(b, "Ka", L0);
        qc_set(b, "Kb", L0);
        qc_set(b, "Kc", L0);
        qc_step(b);
        qc_pulse(b, "CL");
        ok = qc_is(b, "OUT", L0);
        if (pass == 0) {
            qc_set(b, "Ka", L1);
            qc_set(b, "Kb", L1);
            qc_set(b, "Kc", L1);
            qc_set(b, "J1", LX);
        } else {
            qc_set(b, "Kb", L1);
        }
        qc_step(b);
        qc_pulse(b, "CL");
        qc_expect(ok && qc_is(b, "OUT", LX),
                  pass == 0 ? "4059A after the preset, mode /2 with J1 = X: OUT = X"
                            : "4059A after the preset, Ka Kb Kc = 010 (not in Table I): OUT = X");
        bench_destroy(b);
        part_library_destroy(lib);
    }
}

/* CD4089B: CLEAR unknown at count 2 leaves the count unknown: 15OUT unknown,
   and OUTBAR unknown while CLOCK and STROBE are low with rate A set. A clock
   keeps it unknown (CLOCK high gates OUT off: OUTBAR = 1); CLEAR high -> 0. */
static void check_rate_unknown(void) {
    PartLibrary *lib;
    Bench *b = qc_bench(&lib, FX, "fx-4089B", NULL, NULL);
    int ok;

    qc_set(b, "INHIN", L0);
    qc_set(b, "STROBE", L0);
    qc_set(b, "CASCADE", L0);
    qc_set(b, "SET15", L0);
    qc_set(b, "A", L1);
    qc_set(b, "B", L0);
    qc_set(b, "C", L0);
    qc_set(b, "D", L0);
    qc_set(b, "CLOCK", L0);
    qc_set(b, "CLEAR", L1);
    qc_step(b);
    qc_set(b, "CLEAR", L0);
    qc_step(b);
    qc_pulse(b, "CLOCK");
    qc_pulse(b, "CLOCK");
    ok = qc_is(b, "15OUT", L0);
    qc_set(b, "CLEAR", LX);
    qc_step(b);
    ok &= qc_is(b, "15OUT", LX) && qc_is(b, "OUTBAR", LX);
    qc_set(b, "CLEAR", L0);
    qc_set(b, "CLOCK", L1);
    qc_step(b);
    ok &= qc_is(b, "15OUT", LX) && qc_is(b, "OUTBAR", L1);
    qc_set(b, "CLEAR", L1);
    qc_step(b);
    qc_expect(ok && qc_is(b, "15OUT", L0),
              "4089B CLEAR = X at 2 -> 15OUT, OUTBAR = X; clock keeps X (OUTBAR 1, CLOCK high); CLEAR -> 0");
    bench_destroy(b);
    part_library_destroy(lib);
}

int main(void) {
    check_ripple_loader();
    check_seg7_loader();
    check_johnson_divn_rate_loader();
    check_ripple_unknown();
    check_seg7_unknown();
    check_seg7_blank_alone();
    check_divn_unknown();
    check_rate_unknown();
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("counter_div_cov_test: all passed\n");
    return 0;
}
