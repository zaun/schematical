/*
 * buffer_cov_test (plan-68-Q): the buffer kind's four models' loader
 * rejections (real devices of buffer.json with one edit each), a part's own
 * B-side voltage over its timing entry's, the open-drain pin hook for a port
 * no unit names, and a '623-form transceiver with one enable unknown.
 */

#include "model_pins.h"
#include "q_cov_bench.h"

#define E(...) ((const char *const[]){__VA_ARGS__, NULL})
#define B "buffer.json"

static const char NOPE[] = "no pin named \"NOPE\"";

/* "['<prefix>0', ..., '<prefix><n-1>']" in buf. */
static const char *names(char *buf, size_t size, const char *prefix, int n) {
    size_t used = 0;
    int i;

    used += (size_t)snprintf(buf + used, size - used, "[");
    for (i = 0; i < n; i++) {
        used += (size_t)snprintf(buf + used, size - used, "%s'%s%d'", i ? "," : "", prefix, i);
    }
    snprintf(buf + used, size - used, "]");
    return buf;
}

/* Two units of 40 bits (80 > 64): each {a_key: 40 names, b_key: 40 names}
   plus extra (the model's other unit keys). */
static void check_too_many_bits(const char *id, const char *a_key, const char *b_key,
                                const char *extra) {
    char a[400], b[400], unit[1200], units[2500];

    names(a, sizeof a, "P", 40);
    names(b, sizeof b, "Q", 40);
    snprintf(unit, sizeof unit, "{'%s':%s,'%s':%s%s}", a_key, a, b_key, b, extra);
    snprintf(units, sizeof units, "[%s,%s]", unit, unit);
    qc_fail(B, id, E("units", units), "at most 64 bits per part");
}

static void check_tristate_transceiver_loader(void) {
    qc_fail(B, "1G125",
            E("units.0.oe", "[{'pin':'OE','active':'low'},{'pin':'OE','active':'low'},"
                            "{'pin':'OE','active':'low'},{'pin':'OE','active':'low'},"
                            "{'pin':'OE','active':'low'}]"),
            "oe: at most 4 enables");
    qc_fail(B, "1G125", E("units.0.oe.0.active", NULL), "oe: [0]: missing \"active\"");
    qc_fail(B, "1G125", E("units.0.oe.0.pin", "'NOPE'"), NOPE);
    qc_fail(B, "1G125", E("units", NULL), "missing \"units\"");
    check_too_many_bits("74_7541", "a", "y", "");
    qc_fail(B, "74_11245", E("units.0.dir", "'DIR'"), "dir: must be {\"pin\", \"ab\"}");
    qc_fail(B, "74_11245", E("units", NULL), "missing \"units\"");
    check_too_many_bits("74_11245", "a", "b", ",'dir':{'pin':'DIR','ab':'high'}");
    qc_fail(B, "74_11245", E("units.0.a.0", "'NOPE'"), NOPE);
    qc_fail(B, "74_16623", E("units.0.oeab.pin", "'NOPE'"), NOPE);
    qc_fail(B, "74_11245", E("units.0.dir.pin", "'NOPE'"), NOPE);
    qc_fail(B, "74_11245", E("model", NULL), "missing \"model\"");
    qc_fail(B, "1G125", E("parts.0.family", "'ZZ'"), "unknown family \"ZZ\"");
}

static void check_translator_loader(void) {
    qc_fail(B, "1T45", E("domains", "[]"), "domains: must be an array of 2 domains");
    qc_fail(B, "1T45", E("domains.0", "'VCCA'"), "domains: [0]: must be {\"power\", \"pins\"}");
    qc_fail(B, "1T45", E("domains.1.x", "1"), "domains: [1]: unknown key \"x\"");
    qc_fail(B, "1T45", E("timing.AVC.vccB", "'x'"), "timing.AVC: \"vccB\" must be [min, max] in volts");
    qc_fail(B, "1T45", E("timing.AVC.inputMaxB", "'x'"), "timing.AVC: \"inputMaxB\" must be");
    qc_fail(B, "1T45", E("domains.0.power", "'NOPE'"), NOPE);
    qc_fail(B, "1T45", E("domains.1.pins.0", "'NOPE'"), NOPE);
    qc_fail(B, "1T45", E("parts.0.vccB", "'x'"), "\"vccB\" must be [min, max] in volts");
}

/* A part's "vccB" replaces its timing entry's B range (and its input max
   follows the new max); a part's "inputMaxB" alone keeps the entry's range. */
static void check_part_vcc_b(void) {
    PartLibrary *lib;
    const Part *part = qc_load(&lib, B, "1T45", E("parts.0.vccB", "[1.5,3.3]"), NULL);

    qc_expect(part != NULL && part->domain_count == 2 && part->domains[1].vcc->min_mv == 1500 &&
                  part->domains[1].vcc->max_mv == 3300 && part->domains[1].vcc->input_max_mv == 3300,
              "1T45 part vccB [1.5, 3.3]: B side 1500-3300 mV, input max 3300");
    part_library_destroy(lib);
    part = qc_load(&lib, B, "1T45", E("parts.0.inputMaxB", "4"), NULL);
    qc_expect(part != NULL && part->domain_count == 2 && part->domains[1].vcc->max_mv == 3600 &&
                  part->domains[1].vcc->input_max_mv == 4000,
              "1T45 part inputMaxB 4 (AVC vccB [1.2, 3.6]): B max 3600, input max 4000");
    part_library_destroy(lib);
}

static void check_registered_loader(void) {
    qc_fail(B, "29FCT52", E("units.0.ab.oe", NULL), "ab: missing \"oe\"");
    qc_fail(B, "29FCT52", E("units.0.ab", "'x'"), "ab: must be an object");
    qc_fail(B, "29FCT52", E("units.0.ab.store", NULL), "ab: missing \"store\"");
    qc_fail(B, "29FCT52", E("units", NULL), "missing \"units\"");
    qc_fail(B, "29FCT52", E("units.0.x", "1"), "units[0]: unknown key \"x\"");
    qc_fail(B, "29FCT52", E("units.0.a", "['A0','A1']"), "\"a\" and \"b\" must have the same number of names");
    qc_fail(B, "74_16646", E("units.0.dir", "'x'"), "dir: must be {\"pin\", \"ab\"}");
    check_too_many_bits("29FCT52", "a", "b",
                        ",'ab':{'store':'register','clk':{'pin':'CPA','edge':'rising'},'oe':{'pin':'OEA','active':'low'}}"
                        ",'ba':{'store':'register','clk':{'pin':'CPB','edge':'rising'},'oe':{'pin':'OEB','active':'low'}}");
    qc_fail(B, "29FCT52", E("units.0.ab.ce.pin", "'NOPE'"), NOPE);
    qc_fail(B, "29FCT52",
            E("units.0.ab.store", "'latch'", "units.0.ab.clk", NULL, "units.0.ab.le",
              "{'pin':'NOPE','active':'high'}"),
            NOPE);
    qc_fail(B, "29FCT52", E("units.0.ab.clk.pin", "'NOPE'"), NOPE);
    qc_fail(B, "29FCT52", E("units.0.b.7", "'NOPE'"), NOPE);
    qc_fail(B, "74_16646", E("units.1.dir.pin", "'NOPE'"), NOPE);
}

/* The open-drain hook answers for the ports and outputs the units name; a
   pin of the pinout left to no unit is not open-drain (one per model). */
static void check_open_drain_hooks(void) {
    PartLibrary *lib;
    const Part *part;

    part = qc_load(&lib, B, "74_7541",
                   E("units.0.a", "['A1','A2','A3','A4','A5','A6','A7']", "units.0.y",
                     "['Y1','Y2','Y3','Y4','Y5','Y6','Y7']"),
                   NULL);
    qc_expect(part != NULL && part_pin_open_drain(part, qc_pin(part, "Y1")) == 1 &&
                  part_pin_open_drain(part, qc_pin(part, "Y8")) == 0,
              "74x7541 with 7 bits: Y1 open-drain, Y8 not");
    part_library_destroy(lib);
    part = qc_load(&lib, B, "74_621",
                   E("units.0.a", "['A1','A2','A3','A4','A5','A6','A7']", "units.0.b",
                     "['B1','B2','B3','B4','B5','B6','B7']"),
                   NULL);
    qc_expect(part != NULL && part_pin_open_drain(part, qc_pin(part, "B1")) == 1 &&
                  part_pin_open_drain(part, qc_pin(part, "B8")) == 0,
              "74x621 with 7 bits: B1 open-drain, B8 not");
    part_library_destroy(lib);
    part = qc_load(&lib, B, "74_16646", E("output", "'open-drain'", "units.1", NULL), NULL);
    qc_expect(part != NULL && part_pin_open_drain(part, qc_pin(part, "1A1")) == 1 &&
                  part_pin_open_drain(part, qc_pin(part, "2B1")) == 0,
              "74x16646 open-drain with unit 2 removed: 1A1 open-drain, 2B1 not");
    part_library_destroy(lib);
    part = qc_load(&lib, B, "1T45", E("output", "'open-drain'"), NULL);
    qc_expect(part != NULL && part_pin_open_drain(part, qc_pin(part, "B")) == 1 &&
                  part_pin_open_drain(part, qc_pin(part, "DIR")) == 0,
              "1T45 made open-drain: B open-drain, DIR (input) not");
    part_library_destroy(lib);
}

/* SN74ABT16623: OEAB high turns A->B on, OEBA low B->A. One direction on and
   the other unknown: the on side's output is unknown (it may be fighting the
   other), and so is the unknown side's. */
static void check_one_enable_unknown(void) {
    PartLibrary *lib;
    Bench *b = qc_bench(&lib, B, "74_16623", NULL, NULL);
    int ok;

    qc_set(b, "1OEAB", L1);
    qc_set(b, "1OEBA", L1);
    qc_set(b, "1A1", L1);
    qc_step(b);
    ok = qc_is(b, "1B1", L1);
    qc_set(b, "1OEBA", LX);
    qc_step(b);
    ok &= qc_is(b, "1B1", LX);
    qc_expect(ok, "74ABT16623 A->B on, OEBA = X: 1B1 = X");
    qc_set(b, "1A1", LZ);
    qc_set(b, "1OEAB", LX);
    qc_set(b, "1OEBA", L0);
    qc_set(b, "1B1", L1);
    qc_step(b);
    qc_expect(qc_is(b, "1A1", LX), "74ABT16623 B->A on, OEAB = X: 1A1 = X");
    bench_destroy(b);
    part_library_destroy(lib);
}

int main(void) {
    check_tristate_transceiver_loader();
    check_translator_loader();
    check_part_vcc_b();
    check_registered_loader();
    check_open_drain_hooks();
    check_one_enable_unknown();
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("buffer_cov_test: all passed\n");
    return 0;
}
