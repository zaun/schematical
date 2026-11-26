/*
 * cpu_cov_test (plan-68-S): the 6502-family adapter (cpu6502.c) with an X on
 * an input of a running CPU, on cpu_test's bench (support/cpu_test_bench.c,
 * the W65C02S and MCS6502 fixtures). The rule (cpu.h, "the differential
 * rule"): an X input that could change what the CPU does makes it LOST; one
 * that cannot (an IRQ with the I flag set, SO with V already set) does not.
 */

#include "cpu_test_bench.h"

/* A booted CPU (NOPs at C000) a few cycles past reset, RUNNING. */
static Bench *running(const char *name) {
    Bench *b = boot(name, NULL, 0);

    run_cycles(b, 4);
    if (!status_is(b, CPU_RUNNING)) {
        expect(0, label(name, "booted: RUNNING"));
    }
    return b;
}

/* RES, RDY or NMI X while running: LOST. */
static void case_x_lost(const char *name) {
    Bench *b = running(name);

    set_pin(b, b->res, LOGIC_X);
    run_cycles(b, 2);
    expect(status_is(b, CPU_LOST), label(name, "RES X while running: LOST"));
    bench_close(b);

    b = running(name);
    set_pin(b, b->rdy, LOGIC_X);
    run_cycles(b, 2);
    expect(status_is(b, CPU_LOST), label(name, "RDY X while running: LOST"));
    bench_close(b);

    b = running(name);
    set_pin(b, b->nmi, LOGIC_X);
    run_cycles(b, 4);
    expect(status_is(b, CPU_LOST), label(name, "NMI X (may be an edge): LOST"));
    bench_close(b);

    /* PHI2 X: the clock outputs X, LOST. */
    b = running(name);
    set_pin(b, b->clk, LOGIC_X);
    run_to(b, sim_now(b->sim) + NS(10));
    expect(pin_value(b, b->phi2) == LOGIC_X && pin_value(b, b->phi1) == LOGIC_X &&
               status_is(b, CPU_LOST),
           label(name, "PHI2 X: PHI1O and PHI2O X, LOST"));
    bench_close(b);
}

/* IRQ X with I set (after reset): masked, no difference: still RUNNING.
   SO X with V clear: V may be set: LOST; SO X with V set: no difference. */
static void case_x_harmless(const char *name) {
    static const uint8_t SET_V[] = {0xA9, 0x40, 0x69, 0x40};   /* LDA #$40; ADC #$40: V=1 */
    Bench *b = running(name);

    set_pin(b, b->irq, LOGIC_X);
    run_cycles(b, 6);
    expect(status_is(b, CPU_RUNNING), label(name, "IRQ X with I set: RUNNING"));
    bench_close(b);

    b = running(name);
    set_pin(b, b->so, LOGIC_X);
    run_cycles(b, 4);
    expect(status_is(b, CPU_LOST), label(name, "SO X with V clear: LOST"));
    bench_close(b);

    b = boot(name, SET_V, sizeof SET_V);
    run_cycles(b, 14);
    set_pin(b, b->so, LOGIC_X);
    run_cycles(b, 6);
    expect(status_is(b, CPU_RUNNING), label(name, "SO X with V set: RUNNING"));
    bench_close(b);
}

int main(void) {
    size_t i;

    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0 || load_fixture() != 0) {
        return 2;
    }
    for (i = 0; i < sizeof CORE_NAMES / sizeof CORE_NAMES[0]; i++) {
        case_x_lost(CORE_NAMES[i]);
        case_x_harmless(CORE_NAMES[i]);
    }
    part_library_destroy(library);
    if (failures > 0) {
        printf("cpu_cov_test: %d failures\n", failures);
        return 1;
    }
    printf("cpu_cov_test: all passed\n");
    return 0;
}
