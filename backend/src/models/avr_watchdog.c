/*
 * The AVR watchdog timer. A C port of avr8js 0.21.1's
 * src/peripherals/watchdog.ts (Copyright (c) Uri Shaked and contributors,
 * MIT; see LICENSE-avr8js). plan-62-C.
 */

#include <math.h>
#include <string.h>

#include "avr.h"

enum {
    MCUSR_WDRF = 0x08,
    WDTCSR_WDIF = 0x80, WDTCSR_WDIE = 0x40, WDTCSR_WDP3 = 0x20, WDTCSR_WDCE = 0x10,
    WDTCSR_WDE = 0x08, WDTCSR_WDP210 = 0x07,
    WDTCSR_PROTECT_MASK = WDTCSR_WDE | WDTCSR_WDP3 | WDTCSR_WDP210
};

/* The watchdog's own oscillator: 128 kHz. A prescaler of 2048 is 16 ms. */
#define WATCHDOG_HZ 128000.0

static void check_watchdog(void *ctx);

static int watchdog_prescaler(const AvrWatchdog *watchdog) {
    uint8_t wdtcsr = watchdog->cpu->data[watchdog->config.WDTCSR];
    int value = ((wdtcsr & WDTCSR_WDP3) >> 2) | (wdtcsr & WDTCSR_WDP210);

    return 2048 << value;
}

static void reset_watchdog(AvrWatchdog *watchdog) {
    double cycles = floor((avr_clock_frequency(watchdog->clock) / WATCHDOG_HZ) *
                          watchdog_prescaler(watchdog));

    watchdog->watchdog_timeout = (double)watchdog->cpu->cycles + cycles;
}

/* The WDR instruction. */
static void wdr(void *ctx) {
    reset_watchdog(ctx);
}

static void check_watchdog(void *ctx) {
    AvrWatchdog *watchdog = ctx;
    AvrCpu *cpu = watchdog->cpu;

    if (watchdog->enabled_value && (double)cpu->cycles >= watchdog->watchdog_timeout) {
        /* The watchdog timed out. */
        uint8_t wdtcsr = cpu->data[watchdog->config.WDTCSR];

        if (wdtcsr & WDTCSR_WDIE) {
            avr_set_interrupt_flag(cpu, &watchdog->Watchdog);
        }
        if (wdtcsr & WDTCSR_WDE) {
            if (wdtcsr & WDTCSR_WDIE) {
                cpu->data[watchdog->config.WDTCSR] &= (uint8_t)~WDTCSR_WDIE;
            } else {
                /* avr8js resets only the CPU here (pc, SP, the queues): the
                   registers and the other peripherals keep their state. A
                   real chip's watchdog reset is a full reset, so outside
                   exact mode the whole chip restarts through on_reset. */
                if (!cpu->exact && cpu->on_reset.fn != NULL) {
                    cpu->on_reset.fn(cpu->on_reset.ctx);
                } else {
                    avr_cpu_reset(cpu);
                }
                watchdog->scheduled = 0;
                cpu->data[watchdog->config.MCUSR] |= MCUSR_WDRF;
                return;
            }
        }
        reset_watchdog(watchdog);
    }
    if (watchdog->enabled_value) {
        watchdog->scheduled = 1;
        avr_add_clock_event(cpu, check_watchdog, watchdog,
                            watchdog->watchdog_timeout - (double)cpu->cycles);
    } else {
        watchdog->scheduled = 0;
    }
}

static int wdtcsr_write(void *ctx, uint8_t value, uint8_t old_value, uint16_t addr, uint8_t mask) {
    AvrWatchdog *watchdog = ctx;
    AvrCpu *cpu = watchdog->cpu;

    (void)addr;
    (void)mask;
    if ((value & WDTCSR_WDCE) && (value & WDTCSR_WDE)) {
        watchdog->change_enabled_cycles = (double)cpu->cycles + 4;
        value = value & (uint8_t)~WDTCSR_PROTECT_MASK;
    } else {
        if ((double)cpu->cycles >= watchdog->change_enabled_cycles) {
            value = (uint8_t)((value & ~WDTCSR_PROTECT_MASK) | (old_value & WDTCSR_PROTECT_MASK));
        }
        watchdog->enabled_value = (value & WDTCSR_WDE) || (value & WDTCSR_WDIE);
        cpu->data[watchdog->config.WDTCSR] = value;
    }

    if (watchdog->enabled_value) {
        reset_watchdog(watchdog);
    }
    if (watchdog->enabled_value && !watchdog->scheduled) {
        avr_add_clock_event(cpu, check_watchdog, watchdog,
                            watchdog->watchdog_timeout - (double)cpu->cycles);
    }
    avr_clear_interrupt_by_flag(cpu, &watchdog->Watchdog, value);
    return 1;
}

void avr_watchdog_reset(AvrWatchdog *watchdog) {
    watchdog->change_enabled_cycles = 0;
    watchdog->watchdog_timeout = 0;
    watchdog->enabled_value = 0;
    watchdog->scheduled = 0;
}

void avr_watchdog_init(AvrWatchdog *watchdog, AvrCpu *cpu, const AvrWatchdogConfig *config,
                       AvrClock *clock) {
    memset(watchdog, 0, sizeof(*watchdog));
    watchdog->cpu = cpu;
    watchdog->config = *config;
    watchdog->clock = clock;
    watchdog->Watchdog.address = config->watchdog_interrupt;
    watchdog->Watchdog.flag_register = config->WDTCSR;
    watchdog->Watchdog.flag_mask = WDTCSR_WDIF;
    watchdog->Watchdog.enable_register = config->WDTCSR;
    watchdog->Watchdog.enable_mask = WDTCSR_WDIE;
    cpu->watchdog_reset.fn = wdr;
    cpu->watchdog_reset.ctx = watchdog;
    if (config->WDTCSR < cpu->data_size) {
        cpu->write_hooks[config->WDTCSR].fn = wdtcsr_write;
        cpu->write_hooks[config->WDTCSR].ctx = watchdog;
    }
}
