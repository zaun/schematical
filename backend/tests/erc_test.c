/*
 * erc_test: erc_check (include/erc.h, plan-63-D) against what KiCad 10.0.8's
 * `kicad-cli sch erc` reported (erc_pair_table.h: every pair of the twelve pin
 * types on one net, and each type alone; planning/plan-63-data/probe_nets.txt:
 * three-pin nets and No Connect flags), plus attachments, Power Flags, rail pins
 * and edits of the pin map.
 */

#include <stdio.h>
#include <string.h>

#include "erc.h"
#include "erc_pair_table.h"

static int failures;

static void expect(int ok, const char *what) {
    if (!ok) {
        printf("FAIL: %s\n", what);
        failures++;
    }
}

#define MAX_PROBLEMS 64

typedef struct {
    ErcProblem problems[MAX_PROBLEMS];
    size_t count;
    unsigned rules;   /* a bit per ErcProblemKind other than PIN_TO_PIN */
    int level;        /* the highest PIN_TO_PIN level */
    size_t of_kind[5];
} Result;

static Result run(const ErcPin *pins, size_t count, size_t nets, const ErcNet *net_flags,
                  const uint8_t map[ERC_PIN_TYPE_COUNT][ERC_PIN_TYPE_COUNT]) {
    Result r;
    size_t i;

    memset(&r, 0, sizeof r);
    r.count = erc_check(pins, count, nets, net_flags, map, r.problems, MAX_PROBLEMS);
    for (i = 0; i < r.count && i < MAX_PROBLEMS; i++) {
        r.of_kind[r.problems[i].kind]++;
        if (r.problems[i].kind == ERC_PIN_TO_PIN) {
            if (r.problems[i].level > r.level) {
                r.level = r.problems[i].level;
            }
        } else {
            r.rules |= R(r.problems[i].kind);
        }
    }
    return r;
}

static ErcPin pin(size_t net, ErcPinType type) {
    ErcPin p;

    memset(&p, 0, sizeof p);
    p.net = net;
    p.type = type;
    return p;
}

/* Every pair and every lone pin of the table, against KiCad's reports. */
static void check_table(void) {
    size_t i, checked = 0;

    for (i = 0; i < sizeof ERC_PAIRS / sizeof ERC_PAIRS[0]; i++) {
        const ErcPairRow *row = &ERC_PAIRS[i];
        ErcPin pins[2];
        Result r;
        char what[160];
        size_t n = 0;

        if (row->b == ERC_PIN_TYPE_COUNT) {
            pins[n++] = pin(SIZE_MAX, row->a);
        } else {
            pins[n++] = pin(0, row->a);
            pins[n++] = pin(0, row->b);
        }
        r = run(pins, n, 1, NULL, ERC_DEFAULT_PIN_MAP);
        snprintf(what, sizeof what, "%s + %s: rules %x level %d, KiCad %x level %d", erc_pin_type_name(row->a),
                 row->b == ERC_PIN_TYPE_COUNT ? "(alone)" : erc_pin_type_name(row->b), r.rules, r.level,
                 row->rules, row->level);
        expect(r.rules == row->rules && r.level == row->level, what);
        checked++;
    }
    expect(checked == 90, "78 pairs and 12 lone pins");
}

static void check_map(void) {
    int a, b;

    for (a = 0; a < ERC_PIN_TYPE_COUNT; a++) {
        for (b = 0; b < ERC_PIN_TYPE_COUNT; b++) {
            expect(ERC_DEFAULT_PIN_MAP[a][b] == ERC_DEFAULT_PIN_MAP[b][a], "the default pin map is symmetric");
            expect(ERC_DEFAULT_PIN_MAP[a][b] <= 2, "a cell is 0, 1 or 2");
        }
    }
}

/* probe_nets.txt: counts on nets of three pins. */
static void check_three_pins(void) {
    ErcPin outs[3] = {pin(0, ERC_OUTPUT), pin(0, ERC_OUTPUT), pin(0, ERC_OUTPUT)};
    ErcPin two_out_in[3] = {pin(0, ERC_OUTPUT), pin(0, ERC_OUTPUT), pin(0, ERC_INPUT)};
    ErcPin power_outs[3] = {pin(0, ERC_POWER_OUT), pin(0, ERC_POWER_OUT), pin(0, ERC_POWER_OUT)};
    ErcPin inputs[3] = {pin(0, ERC_INPUT), pin(0, ERC_INPUT), pin(0, ERC_INPUT)};
    ErcPin power_ins_input[3] = {pin(0, ERC_POWER_IN), pin(0, ERC_POWER_IN), pin(0, ERC_INPUT)};
    ErcPin power_ins[3] = {pin(0, ERC_POWER_IN), pin(0, ERC_POWER_IN), pin(0, ERC_POWER_IN)};
    Result r;

    r = run(outs, 3, 1, NULL, ERC_DEFAULT_PIN_MAP);
    expect(r.count == 2 && r.of_kind[ERC_PIN_TO_PIN] == 2, "three outputs: two pin_to_pin");
    expect(r.problems[0].pin == 1 && r.problems[0].other == 0 && r.problems[1].pin == 2 &&
               r.problems[1].other == 0, "each later output against the first");
    r = run(two_out_in, 3, 1, NULL, ERC_DEFAULT_PIN_MAP);
    expect(r.count == 1 && r.of_kind[ERC_PIN_TO_PIN] == 1, "two outputs and an input: one");
    r = run(power_outs, 3, 1, NULL, ERC_DEFAULT_PIN_MAP);
    expect(r.count == 2 && r.level == 2, "three power_out: two errors");
    r = run(inputs, 3, 1, NULL, ERC_DEFAULT_PIN_MAP);
    expect(r.count == 1 && r.of_kind[ERC_PIN_NOT_DRIVEN] == 1 && r.problems[0].pin == 0,
           "three inputs: one pin_not_driven, on the first");
    r = run(power_ins_input, 3, 1, NULL, ERC_DEFAULT_PIN_MAP);
    expect(r.count == 1 && r.of_kind[ERC_POWER_PIN_NOT_DRIVEN] == 1 && r.problems[0].pin == 0,
           "two power_in and an input: one power_pin_not_driven");
    r = run(power_ins, 3, 1, NULL, ERC_DEFAULT_PIN_MAP);
    expect(r.count == 1 && r.of_kind[ERC_POWER_PIN_NOT_DRIVEN] == 1, "three power_in: one");
}

/* probe_nets.txt: what a No Connect flag exempts. */
static void check_no_connect_marks(void) {
    ErcPin one[1];
    ErcPin two[2];
    Result r;

    one[0] = pin(SIZE_MAX, ERC_INPUT);
    one[0].no_connect_mark = 1;
    r = run(one, 1, 0, NULL, ERC_DEFAULT_PIN_MAP);
    expect(r.count == 0, "an unwired input with a flag reports nothing");
    one[0] = pin(SIZE_MAX, ERC_POWER_IN);
    one[0].no_connect_mark = 1;
    r = run(one, 1, 0, NULL, ERC_DEFAULT_PIN_MAP);
    expect(r.count == 0, "an unwired power_in with a flag reports nothing");
    one[0] = pin(0, ERC_OUTPUT);
    one[0].no_connect_mark = 1;
    r = run(one, 1, 1, NULL, ERC_DEFAULT_PIN_MAP);
    expect(r.count == 0, "a pin wired to its own flag alone reports nothing");

    two[0] = pin(0, ERC_INPUT);
    two[0].no_connect_mark = 1;
    two[1] = pin(0, ERC_OUTPUT);
    r = run(two, 2, 1, NULL, ERC_DEFAULT_PIN_MAP);
    expect(r.count == 1 && r.of_kind[ERC_NO_CONNECT_CONNECTED] == 1 && r.problems[0].pin == 0,
           "a flag on a pin wired to another: no_connect_connected");
    two[0] = pin(0, ERC_INPUT);
    two[0].no_connect_mark = 1;
    two[1] = pin(0, ERC_INPUT);
    r = run(two, 2, 1, NULL, ERC_DEFAULT_PIN_MAP);
    expect(r.count == 1 && r.of_kind[ERC_NO_CONNECT_CONNECTED] == 1,
           "the other input of a flagged pair is not reported as not driven");
}

static void check_attachments_and_flags(void) {
    ErcPin one[1];
    ErcPin two[2];
    ErcNet nets[1];
    Result r;

    /* A lone connected pin: a label or a bus port on its net connects it. */
    one[0] = pin(0, ERC_PASSIVE);
    r = run(one, 1, 1, NULL, ERC_DEFAULT_PIN_MAP);
    expect(r.of_kind[ERC_PIN_NOT_CONNECTED] == 1, "a lone wired pin is not connected");
    nets[0].has_attachment = 1;
    r = run(one, 1, 1, nets, ERC_DEFAULT_PIN_MAP);
    expect(r.count == 0, "a label on its net connects it");
    one[0] = pin(0, ERC_INPUT);
    r = run(one, 1, 1, nets, ERC_DEFAULT_PIN_MAP);
    expect(r.count == 1 && r.of_kind[ERC_PIN_NOT_DRIVEN] == 1, "a lone input with a label is still not driven");

    /* A Power Flag is a power_out pin: it drives a power_in net, and conflicts with an output. */
    two[0] = pin(0, ERC_POWER_IN);
    two[1] = pin(0, ERC_POWER_OUT);
    r = run(two, 2, 1, NULL, ERC_DEFAULT_PIN_MAP);
    expect(r.count == 0, "a Power Flag drives a power_in pin");
    two[0] = pin(0, ERC_OUTPUT);
    r = run(two, 2, 1, NULL, ERC_DEFAULT_PIN_MAP);
    expect(r.count == 1 && r.of_kind[ERC_PIN_TO_PIN] == 1 && r.level == 2, "power_out + output is an error");
    /* A flag alone is a pin that is not connected. */
    one[0] = pin(SIZE_MAX, ERC_POWER_OUT);
    r = run(one, 1, 0, NULL, ERC_DEFAULT_PIN_MAP);
    expect(r.count == 1 && r.of_kind[ERC_PIN_NOT_CONNECTED] == 1, "an unwired Power Flag is not connected");

    /* A rail symbol (power_in, drives_power) drives its own net: the IC's power pin and an input tied to it. */
    two[0] = pin(0, ERC_POWER_IN);
    two[1] = pin(0, ERC_POWER_IN);
    two[1].drives_power = 1;
    r = run(two, 2, 1, NULL, ERC_DEFAULT_PIN_MAP);
    expect(r.count == 0, "an IC's power pin on a rail symbol is driven");
    two[0] = pin(0, ERC_INPUT);
    r = run(two, 2, 1, NULL, ERC_DEFAULT_PIN_MAP);
    expect(r.count == 0, "an input tied to a rail symbol is driven");
    /* Without the drive, the same net reports it. */
    two[1].drives_power = 0;
    r = run(two, 2, 1, NULL, ERC_DEFAULT_PIN_MAP);
    expect(r.of_kind[ERC_POWER_PIN_NOT_DRIVEN] == 1, "power_in without a driver is not driven");
}

static void check_pin_map_edits(void) {
    uint8_t map[ERC_PIN_TYPE_COUNT][ERC_PIN_TYPE_COUNT];
    ErcPin two[2] = {pin(0, ERC_OUTPUT), pin(0, ERC_OUTPUT)};
    Result r;

    memcpy(map, ERC_DEFAULT_PIN_MAP, sizeof map);
    r = run(two, 2, 1, NULL, map);
    expect(r.level == 2, "output + output is an error by default");
    map[ERC_OUTPUT][ERC_OUTPUT] = 0;
    r = run(two, 2, 1, NULL, map);
    expect(r.count == 0, "a cell set to 0 reports nothing");
    map[ERC_OUTPUT][ERC_OUTPUT] = 1;
    r = run(two, 2, 1, NULL, map);
    expect(r.count == 1 && r.level == 1, "a cell set to 1 reports a warning");
}

static void check_order_and_cap(void) {
    ErcPin pins[5] = {pin(1, ERC_PASSIVE), pin(SIZE_MAX, ERC_PASSIVE), pin(0, ERC_PASSIVE), pin(1, ERC_OUTPUT),
                      pin(0, ERC_OUTPUT)};
    ErcProblem out[1];
    Result r;
    size_t total;

    r = run(pins, 5, 2, NULL, ERC_DEFAULT_PIN_MAP);
    expect(r.count == 1 && r.of_kind[ERC_PIN_TO_PIN] == 0 && r.of_kind[ERC_PIN_NOT_CONNECTED] == 1,
           "one unwired pin is not connected; the wired nets have two pins each");
    expect(r.problems[0].pin == 1 && r.problems[0].net == SIZE_MAX, "the unwired pin sorts last (the only problem)");
    total = erc_check(pins, 5, 2, NULL, ERC_DEFAULT_PIN_MAP, out, 1);
    expect(total == r.count && out[0].pin == r.problems[0].pin, "cap limits what is written, not what is counted");
    expect(erc_check(pins, 5, 2, NULL, ERC_DEFAULT_PIN_MAP, NULL, 0) == r.count, "NULL out counts");
    expect(erc_check(NULL, 0, 0, NULL, ERC_DEFAULT_PIN_MAP, NULL, 0) == 0, "no pins, no problems");
}

int main(void) {
    check_table();
    check_map();
    check_three_pins();
    check_no_connect_marks();
    check_attachments_and_flags();
    check_pin_map_edits();
    check_order_and_cap();
    if (failures > 0) {
        printf("%d failures\n", failures);
        return 1;
    }
    printf("erc_test: 0 failures\n");
    return 0;
}
