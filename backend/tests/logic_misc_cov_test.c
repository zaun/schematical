/*
 * logic_misc_cov_test (plan-68-Q): the register, arith, decoder and mux
 * kinds' loader rejections (real devices with one edit each), their pin
 * hooks for open-drain variants, the states no other case drives (a parity
 * bus driving odd, an adder without carry in), and the library-wide
 * "simulated" key on every sequential kind.
 */

#include "model_pins.h"
#include "q_cov_bench.h"

#define E(...) ((const char *const[]){__VA_ARGS__, NULL})

static const char NOPE[] = "no pin named \"NOPE\"";

static void check_register_loader(void) {
    static const char R[] = "register.json";

    qc_fail(R, "74_377", E("groups.0.ce", "{'pin':'G'}"), "missing \"active\"");
    qc_fail(R, "4076B", E("groups.0.ce.1.active", NULL), "[1]: missing \"active\"");
    qc_fail(R, "74_377", E("groups.0.ce", "'G'"), "must be a control or an array of controls");
    qc_fail(R, "74_377", E("groups.0.outputs.0", "'Q'"), "\"outputs\"[0]: must be an object");
    qc_fail(R, "74_377", E("groups.0.outputs.0.x", "1"), "\"outputs\"[0]: unknown key \"x\"");
    qc_fail(R, "74_377", E("groups.0", "'G'"), "groups[0]: must be an object");
    qc_fail(R, "74_377", E("groups.0.outputs", NULL), "groups[0]: missing \"outputs\"");
    qc_fail(R, "74_377", E("groups.0.outputs", "[]"), "\"outputs\" must be an array of 1 to 2 banks");
    qc_fail(R, "74_377", E("groups", NULL), "missing \"groups\"");
    qc_fail(R, "74_377", E("groups.0.ce.pin", "'NOPE'"), NOPE);
    qc_fail(R, "74_377", E("groups.0.outputs.0.q.7", "'NOPE'"), NOPE);
    qc_fail(R, "74_377", E("model", NULL), "missing \"model\"");
    /* the library itself rejects a key outside the kind's device keys */
    qc_fail(R, "74_377", E("units", "1"), "unknown key \"units\"");
    qc_fail(R, "74_377", E("parts.0.family", "'ZZ'"), "unknown family \"ZZ\"");
    qc_fail(R, "74_575", E("groups.0.clear.when", "'never'"),
            "clear: \"when\" must be \"async\" or \"clock\", not \"never\"");
}

static void check_arith_loader(void) {
    static const char A[] = "arith.json";

    qc_fail(A, "74_181", E("openDrain", "[]"), "\"openDrain\"");
    qc_fail(A, "74_283",
            E("a", "['A1','A2','A3','A4','A1','A2','A3','A4']", "b",
              "['B1','B2','B3','B4','B1','B2','B3','B4']", "sum",
              "['S1','S2','S3','S4','S1','S2','S3','S4']"),
            "more than 8 outputs");
    qc_fail(A, "74_85", E("cascadeTable.0.in", "1"), "\"in\" must be a string");
    qc_fail(A, "74_85", E("cascade.gt", "1"), "cascade: \"gt\" must be a string");
    qc_fail(A, "74_85", E("a.3", "'NOPE'"), NOPE);
    qc_fail(A, "74_85", E("outputs.lt.pin", "'NOPE'"), NOPE);
    qc_fail(A, "74_85", E("cascade.eq", "'NOPE'"), NOPE);
    qc_fail(A, "74_286", E("bus.drives", "1"), "bus: \"drives\" must be a string");
    qc_fail(A, "74_286", E("error.active", NULL), "missing \"active\"");
    qc_fail(A, "74_286", E("inputs.8", "'NOPE'"), NOPE);
    qc_fail(A, "74_286", E("pinouts.p14.5.role", "'output'"), "\"bus\": pin \"PARITY I/O\" must have role inout");
    qc_fail(A, "74_181", E("model", NULL), "missing \"model\"");
    qc_fail(A, "74_181", E("parts.0.family", "'ZZ'"), "unknown family \"ZZ\"");
    qc_fail(A, "74_283", E("b.0", "'NOPE'"), NOPE);
    qc_fail(A, "74_283", E("sum.3", "'NOPE'"), NOPE);
    qc_fail(A, "74_181", E("s.0", "'NOPE'"), NOPE);
    qc_fail(A, "74_181", E("f.3", "'NOPE'"), NOPE);
    qc_fail(A, "74_275", E("adders.0.a", "1"), "adders[0]: \"a\" must be a string");
    qc_fail(A, "74_275", E("adders.0.sum", "1"), "adders[0]: \"sum\" must be a string");
    qc_fail(A, "74_275", E("outputs", NULL), "missing \"outputs\"");
    qc_fail(A, "74_275", E("outputs.0.x", "1"), "outputs[0]: unknown key \"x\"");
    qc_fail(A, "74_275", E("enable", "{'pin':'G'}"), "missing \"active\"");
    qc_fail(A, "74_275", E("adders.0.a", "'NOPE'"), NOPE);
    qc_fail(A, "74_275", E("outputs.0.pin", "'NOPE'"), NOPE);
    qc_fail(A, "74_275", E("enable.pin", "'NOPE'"), NOPE);
    qc_fail(A, "74_280", E("output", "'tristate'"), "\"output\" must be");
}

static void check_decoder_mux_loader(void) {
    static const char D[] = "decoder.json";
    static const char M[] = "mux.json";

    qc_fail(D, "74_138", E("unselected", "1"), "\"unselected\" must be a string");
    qc_fail(D, "74_138", E("units", NULL), "missing \"units\"");
    qc_fail(D, "74_138", E("monitor", "1"), "\"monitor\" must be a string");
    qc_fail(D, "74_137", E("latchActive", NULL), "missing \"latchActive\"");
    qc_fail(D, "74_147", E("offset", "'x'"), "\"offset\" must be a whole number");
    qc_fail(D, "74_148", E("inputs.0", "'NOPE'"), NOPE);
    qc_fail(D, "4055B", E("phase", NULL), "\"phaseOut\" needs \"phase\"");
    qc_fail(D, "74_47", E("inputs.0", "'NOPE'"), NOPE);
    qc_fail(D, "74_138", E("model", NULL), "missing \"model\"");
    qc_fail(D, "74_138", E("parts.0.family", "'ZZ'"), "unknown family \"ZZ\"");
    qc_fail(D, "74_138", E("units.0.enables.2.pin", "'NOPE'"), NOPE);
    qc_fail(D, "74_138", E("monitor", "'NOPE'"), NOPE);
    qc_fail(M, "74_151", E("units", NULL), "missing \"units\"");
    qc_fail(M, "74_151", E("disabled", "1"), "\"disabled\" must be a string");
    qc_fail(M, "74_151", E("model", NULL), "missing \"model\"");
    qc_fail(M, "74_151", E("parts.0.family", "'ZZ'"), "unknown family \"ZZ\"");
    qc_fail(M, "74_354", E("oes.2.pin", "'NOPE'"), NOPE);
    qc_fail(M, "74_670", E("files", NULL), "missing \"files\"");
    qc_fail(M, "74_670", E("files.0.d.0", "'NOPE'"), NOPE);
}

/* docs/parts-format.md: "simulated" (default true) — "Any kind may set it".
   Every sequential kind checks a device's keys against its model's: the
   library's own keys must pass, "simulated" among them. */
static void check_simulated_key(void) {
    static const struct {
        const char *file, *id;
    } DEVICES[] = {
        {"fx:counter/parts.json", "fx-74_161"}, {"fx:latch/parts.json", "fx-74_373"},
        {"fx:shift/parts.json", "fx-74_164"},   {"decoder.json", "74_138"},
        {"arith.json", "74_283"},               {"mux.json", "74_151"},
        {"buffer.json", "1G125"},               {"register.json", "74_377"},
    };
    size_t i;

    for (i = 0; i < sizeof DEVICES / sizeof DEVICES[0]; i++) {
        PartLibrary *lib;
        const Part *part = qc_load(&lib, DEVICES[i].file, DEVICES[i].id, E("simulated", "true"), NULL);
        char what[160];

        snprintf(what, sizeof what, "%s with \"simulated\": true loads", DEVICES[i].id);
        qc_expect(part != NULL, what);
        part_library_destroy(lib);
    }
}

/* The open-drain / 3-state hooks of open-drain variants: an output the
   model drives answers 1, an output pin it does not drive 0. */
static void check_pin_hooks(void) {
    PartLibrary *lib;
    const Part *part;

    part = qc_load(&lib, "arith.json", "74_280", E("odd", NULL, "openDrain", "['EVEN']"), NULL);
    qc_expect(part != NULL && part_pin_open_drain(part, qc_pin(part, "EVEN")) == 1 &&
                  part_pin_open_drain(part, qc_pin(part, "ODD")) == 0,
              "74x280 without ODD, EVEN open-drain: EVEN 1, ODD (not driven) 0");
    part_library_destroy(lib);
    part = qc_load(&lib, "decoder.json", "74_148", E("output", "'open-drain'"), NULL);
    qc_expect(part != NULL && part_pin_open_drain(part, qc_pin(part, "A0")) == 1 &&
                  part_pin_open_drain(part, qc_pin(part, "GS")) == 1 &&
                  part_pin_open_drain(part, qc_pin(part, "EO")) == 1,
              "74x148 made open-drain: A0, GS, EO open-drain");
    part_library_destroy(lib);
    part = qc_load(&lib, "mux.json", "74_151", E("output", "'open-drain'"), NULL);
    qc_expect(part != NULL && part_pin_open_drain(part, qc_pin(part, "Y")) == 1 &&
                  part_pin_open_drain(part, qc_pin(part, "W")) == 1,
              "74x151 made open-drain: Y and W open-drain");
    part_library_destroy(lib);
    part = qc_load(&lib, "mux.json", "74_670", E("output", "'open-drain'", "pinouts.p16.14.role", "'output'"),
                   NULL);
    qc_expect(part != NULL && part_pin_open_drain(part, qc_pin(part, "Q1")) == 1 &&
                  part_pin_open_drain(part, qc_pin(part, "D1")) == 0 &&
                  part_pin_tristate(part, qc_pin(part, "Q1")) == 1 &&
                  part_pin_tristate(part, qc_pin(part, "D1")) == 0,
              "74x670 open-drain, D1 an output pin: Q1 open-drain and 3-state, D1 neither");
    part_library_destroy(lib);
}

/* SN74AS286: with XMIT low the PARITY I/O pin drives the parity of A-I; the
   device's "drives" picks which (even: H for an even count of ones). */
static void check_parity_drives(void) {
    static const char *const IN[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I"};
    int pass;

    for (pass = 0; pass < 2; pass++) {
        PartLibrary *lib;
        Bench *b = qc_bench(&lib, "arith.json", "74_286", pass ? E("bus.drives", "'odd'") : NULL, NULL);

        qc_set_bus(b, IN, 9, 0);
        qc_set(b, "XMIT", L0);
        qc_step(b);
        qc_expect(qc_is(b, "PARITY I/O", pass ? L0 : L1),
                  pass ? "74x286 drives odd, no ones: PARITY I/O = L"
                       : "74x286 drives even, no ones: PARITY I/O = H");
        bench_destroy(b);
        part_library_destroy(lib);
    }
}

/* 74x283 without "cin": the sum is A + B; the C0 pin, now unused, does not add. */
static void check_adder_without_carry_in(void) {
    PartLibrary *lib;
    Bench *b = qc_bench(&lib, "arith.json", "74_283", E("cin", NULL), NULL);
    static const char *const A[] = {"A1", "A2", "A3", "A4"};
    static const char *const BB[] = {"B1", "B2", "B3", "B4"};
    static const char *const S[] = {"S1", "S2", "S3", "S4"};

    qc_set_bus(b, A, 4, 1);
    qc_set_bus(b, BB, 4, 1);
    qc_set(b, "C0", L1);
    qc_step(b);
    qc_expect(qc_bus(b, S, 4, 2) && qc_is(b, "C4", L0), "74x283 without cin: 1 + 1 = 2 (C0 high ignored)");
    bench_destroy(b);
    part_library_destroy(lib);
}

int main(void) {
    check_register_loader();
    check_arith_loader();
    check_decoder_mux_loader();
    check_simulated_key();
    check_pin_hooks();
    check_parity_drives();
    check_adder_without_carry_in();
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("logic_misc_cov_test: all passed\n");
    return 0;
}
