/*
 * The AVR system clock prescaler (CLKPR). A C port of avr8js 0.21.1's
 * src/peripherals/clock.ts (Copyright (c) Uri Shaked and contributors, MIT;
 * see LICENSE-avr8js). plan-62-B.
 */

#include "avr.h"

enum { CLKPCE = 128 };

/* By CLKPS3:0. Values 9-15 are "reserved" in the datasheet; avr8js's authors
   measured them on an ATmega328P with a scope. */
static const int PRESCALERS[16] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 2, 4, 8, 16, 32, 64, 128};

static int clkpr_hook(void *ctx, uint8_t clkpr, uint8_t old_value, uint16_t addr, uint8_t mask) {
    AvrClock *clock = ctx;
    AvrCpu *cpu = clock->cpu;
    double cycles = (double)cpu->cycles;

    (void)old_value;
    (void)addr;
    (void)mask;
    if ((clock->clock_enabled_cycles == 0 || clock->clock_enabled_cycles < cycles) &&
        clkpr == CLKPCE) {
        clock->clock_enabled_cycles = cycles + 4;
    } else if (clock->clock_enabled_cycles != 0 && clock->clock_enabled_cycles >= cycles) {
        int index = clkpr & 0xf;
        int old_prescaler = clock->prescaler_value;

        clock->clock_enabled_cycles = 0;
        clock->prescaler_value = PRESCALERS[index];
        cpu->data[clock->config.CLKPR] = (uint8_t)index;
        if (old_prescaler != clock->prescaler_value) {
            clock->cycles_delta = (cycles + clock->cycles_delta) *
                                      ((double)old_prescaler / clock->prescaler_value) -
                                  cycles;
        }
    }
    return 1;
}

void avr_clock_reset(AvrClock *clock) {
    clock->clock_enabled_cycles = 0;
    clock->prescaler_value = PRESCALERS[clock->reset_index & 0xf];
    clock->cycles_delta = 0;
    clock->cpu->data[clock->config.CLKPR] = clock->reset_index & 0xf;
}

void avr_clock_init(AvrClock *clock, AvrCpu *cpu, uint32_t base_hz, const AvrClockConfig *config,
                    uint8_t reset_index) {
    clock->cpu = cpu;
    clock->base_hz = base_hz;
    clock->config = *config;
    clock->reset_index = reset_index;
    avr_clock_reset(clock);
    if (config->CLKPR < cpu->data_size) {
        cpu->write_hooks[config->CLKPR].fn = clkpr_hook;
        cpu->write_hooks[config->CLKPR].ctx = clock;
    }
}
