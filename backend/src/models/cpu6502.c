/*
 * cpu6502: the 6502-family bus adapter (plan-09-B sections 4.4-4.5).
 * Phase 1: every output X and status LOST; the adapter lands in phase 2.
 */

#include <stdlib.h>

#include "cpu.h"

typedef struct {
    CpuBase base;
    const Part *part;
} Cpu65;

static void *c65_create(const Part *part, const CpuPins *pins, const CpuGrade *grade) {
    Cpu65 *state = calloc(1, sizeof(*state));

    (void)pins;
    (void)grade;
    if (state != NULL) {
        state->base.ops = &CPU6502_OPS;
        state->part = part;
    }
    return state;
}

static void c65_destroy(void *state) {
    free(state);
}

static void c65_start(Sim *sim, SimInstance *instance, void *raw) {
    Cpu65 *state = raw;
    size_t i;

    for (i = 0; i < state->part->pin_count; i++) {
        PinRole role = state->part->pins[i].role;

        if (role == PIN_OUTPUT || role == PIN_INOUT) {
            sim_schedule_drive(sim, instance, i, LOGIC_X, DRIVE_STRONG, 0);
        }
    }
}

static void c65_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                            LogicValue old_value, LogicValue new_value) {
    (void)sim;
    (void)instance;
    (void)state;
    (void)pin;
    (void)old_value;
    (void)new_value;
}

static int c65_state(const void *state, CpuState *out) {
    (void)state;
    out->status = CPU_LOST;
    return 1;
}

const CpuCoreOps CPU6502_OPS = {
    c65_create, c65_destroy, c65_start, c65_pin_changed, NULL, c65_state
};
