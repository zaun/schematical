/* pio_bench: shared by the programs split from pio_test.c (plan-68-H). */
#include "pio_bench.h"


int failures;

const PartPin TEST_PINS[P_COUNT] = {
    {"1", "PHI2", PIN_INPUT, NULL}, {"2", "RWB", PIN_INPUT, NULL},
    {"3", "CS1", PIN_INPUT, NULL}, {"4", "CS2B", PIN_INPUT, NULL},
    {"5", "RS0", PIN_INPUT, NULL}, {"6", "RS1", PIN_INPUT, NULL},
    {"7", "RS2", PIN_INPUT, NULL}, {"8", "RS3", PIN_INPUT, NULL},
    {"9", "D0", PIN_INOUT, "data"}, {"10", "D1", PIN_INOUT, "data"},
    {"11", "D2", PIN_INOUT, "data"}, {"12", "D3", PIN_INOUT, "data"},
    {"13", "D4", PIN_INOUT, "data"}, {"14", "D5", PIN_INOUT, "data"},
    {"15", "D6", PIN_INOUT, "data"}, {"16", "D7", PIN_INOUT, "data"},
};

const Bus65Pins TEST_BUS_PINS = {
    P_PHI2, P_RWB, {P_D0, P_D1, P_D2, P_D3, P_D4, P_D5, P_D6, P_D7},
    {P_CS1}, 1, {P_CS2B}, 1, {P_RS0, P_RS1, P_RS2, P_RS3}, 4
};

const Bus65Timing TEST_TIMING = {
    10 * NS, 10 * NS, 20 * NS, 10 * NS, 10 * NS, 10 * NS, 10 * NS, 10 * NS,
    70 * NS, 35 * NS, 35 * NS
};

int t_read(void *model, unsigned reg, uint8_t *value, uint8_t *known) {
    TestRegs *t = model;

    t->reads++;
    *value = t->regs[reg];
    *known = t->known[reg];
    return 1;
}

void t_write(void *model, unsigned reg, uint8_t value, uint8_t known) {
    TestRegs *t = model;

    t->writes++;
    t->regs[reg] = value & known;
    t->known[reg] = known;
}

void t_cycle(void *model) {
    ((TestRegs *)model)->cycles++;
}

void t_read_unknown(void *model) {
    ((TestRegs *)model)->unknown_reads++;
}

const Bus65Ops TEST_OPS = {t_read, t_write, t_cycle, t_read_unknown};

size_t t_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

void *t_create(const void *config) {
    TestRegs *t = calloc(1, sizeof(*t));

    (void)config;
    bus65_init(&t->bus, &TEST_BUS_PINS, &TEST_TIMING, &TEST_OPS, t);
    return t;
}

void t_destroy(void *state) {
    free(state);
}

void t_start(Sim *sim, SimInstance *instance, void *state) {
    bus65_start(&((TestRegs *)state)->bus, sim, instance);
}

void t_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                          LogicValue old_value, LogicValue new_value) {
    bus65_pin_changed(&((TestRegs *)state)->bus, sim, instance, pin, old_value, new_value);
}

void t_timer(Sim *sim, SimInstance *instance, void *state, uint32_t timer_id) {
    bus65_timer(&((TestRegs *)state)->bus, sim, instance, timer_id);
}

const SimComponentType TEST_TYPE = {
    "bus65-test", t_pin_count, t_create, t_destroy, t_start, t_pin_changed, t_timer
};

const PartKindOps TEST_KIND = {PART_PIO, "bus65-test", &TEST_TYPE};
const PartInternal TEST_INTERNAL = {&TEST_KIND, NULL, NULL, NULL};

Part test_part(void) {
    Part part;

    memset(&part, 0, sizeof part);
    part.kind = PART_PIO;
    part.name = "BUS65";
    part.pins = TEST_PINS;
    part.pin_count = P_COUNT;
    part.internal = &TEST_INTERNAL;
    return part;
}

/* ---- Bench helpers ----------------------------------------------------------------- */

const char *const DATA_NAMES[8] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"};
const char *const RS_NAMES[4] = {"RS0", "RS1", "RS2", "RS3"};

LogicValue bit_value(unsigned v, int i) {
    return (v >> i & 1) ? LOGIC_1 : LOGIC_0;
}

void set_rs(Bench *b, unsigned reg) {
    int i;

    for (i = 0; i < 4 && bench_has_pin(b, RS_NAMES[i]); i++) {
        bench_set(b, RS_NAMES[i], bit_value(reg, i));
    }
}

void set_data(Bench *b, unsigned v) {
    int i;

    for (i = 0; i < 8; i++) bench_set(b, DATA_NAMES[i], bit_value(v, i));
}

void release_data(Bench *b) {
    int i;

    for (i = 0; i < 8; i++) bench_set(b, DATA_NAMES[i], LOGIC_Z);
}

/* 'v' when every data pin is 0/1 (*value set), 'X' when all are X, 'Z' when all Z, else '?'. */
char data_state(Bench *b, unsigned *value) {
    int x = 0, z = 0, k = 0;
    int i;

    *value = 0;
    for (i = 0; i < 8; i++) {
        LogicValue v = bench_get(b, DATA_NAMES[i]);

        if (v == LOGIC_X) x++;
        else if (v == LOGIC_Z) z++;
        else {
            k++;
            if (v == LOGIC_1) *value |= 1u << i;
        }
    }
    return k == 8 ? 'v' : x == 8 ? 'X' : z == 8 ? 'Z' : '?';
}

TestRegs *regs_of(Bench *b) {
    return bench_state(b);
}

/* A bench with PHI2 low, selected (CS1 1, CS2B 0), RWB 1, RS 0, run 1 us. */
Bench *new_bench(const Part *part) {
    Bench *b = bench_create(part);

    bench_set(b, "PHI2", LOGIC_0);
    bench_set(b, "CS1", LOGIC_1);
    bench_set(b, "CS2B", LOGIC_0);
    bench_set(b, "RWB", LOGIC_1);
    set_rs(b, 0);
    bench_run(b, 2 * HALF);
    return b;
}

/* One full PHI2 cycle from PHI2 low: controls set now, the low phase, the
   high phase (write data from 100 ns into it), PHI2 falls; data released
   30 ns after the fall. Ends 30 ns after the fall. */
void bus_write(Bench *b, unsigned reg, unsigned value) {
    bench_set(b, "RWB", LOGIC_0);
    set_rs(b, reg);
    bench_run(b, HALF);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, 100 * NS);
    set_data(b, value);
    bench_run(b, HALF - 100 * NS);
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 30 * NS);
    release_data(b);
    bench_set(b, "RWB", LOGIC_1);
    bench_run(b, 1);
}


/* ---- B: pia6521 (the shipped W65C21N) ------------------------------------------- */

/* The shipped part library (schematical/schematical/Parts). */
PartLibrary *shipped_library;

const Part *shipped_part(const char *name) {
    const Part *part;

    if (shipped_library == NULL) {
        shipped_library = part_library_create();
        if (load_parts_dir(shipped_library, PARTS_DIR) != 0) {
            exit(2);
        }
    }
    part = part_library_find(shipped_library, name);
    if (part == NULL) {
        fprintf(stderr, "no part %s\n", name);
        exit(2);
    }
    return part;
}

const char *const PA_NAMES[8] = {"PA0", "PA1", "PA2", "PA3", "PA4", "PA5", "PA6", "PA7"};
const char *const PB_NAMES[8] = {"PB0", "PB1", "PB2", "PB3", "PB4", "PB5", "PB6", "PB7"};

/* The byte on 8 pins; *x gets the bits that are not 0/1. */
unsigned pins_value(Bench *b, const char *const *names, unsigned *x) {
    unsigned v = 0;
    int i;

    *x = 0;
    for (i = 0; i < 8; i++) {
        LogicValue p = bench_get(b, names[i]);

        if (p == LOGIC_1) v |= 1u << i;
        else if (p != LOGIC_0) *x |= 1u << i;
    }
    return v;
}

/* From a fall: an unselected cycle begins (data released, RWB 1, CS2B
   high at +30 ns); returns at +30 ns. */
void idle_begin(Bench *b) {
    bench_run(b, 30 * NS);
    release_data(b);
    bench_set(b, "RWB", LOGIC_1);
    bench_set(b, "CS2B", LOGIC_1);
}

/* `since_fall` ns into an unselected cycle: PHI2 rises at +500 ns, falls at
   +1000 ns; returns at the fall. */
void idle_end(Bench *b, SimTime since_fall) {
    bench_run(b, HALF - since_fall);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, HALF);
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 0);
}

/* From a fall: an unselected cycle up to its PHI2 rise, where it returns. */
void idle_to_rise(Bench *b) {
    idle_begin(b);
    bench_run(b, 470 * NS);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, 0);
}

/* `elapsed` ns after a rise: the rest of the high phase, then PHI2 falls. */
void finish_high(Bench *b, SimTime elapsed) {
    bench_run(b, HALF - elapsed);
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 0);
}
