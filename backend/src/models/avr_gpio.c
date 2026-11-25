/*
 * AVR GPIO ports: PIN/DDR/PORT, the external interrupts and the pin-change
 * interrupt of a port. A C port of avr8js 0.21.1's src/peripherals/gpio.ts
 * (Copyright (c) Uri Shaked and contributors, MIT; see LICENSE-avr8js).
 * plan-62-B.
 */

#include <string.h>

#include "avr.h"

/* External interrupt sense control (ISCx1:0). */
enum { MODE_LOW_LEVEL, MODE_CHANGE, MODE_FALLING_EDGE, MODE_RISING_EDGE };

/* What a hook on an external interrupt register is for. */
enum { HOOK_OTHER, HOOK_MASK, HOOK_FLAG };

AvrPort *avr_cpu_port_at(AvrCpu *cpu, uint16_t port_address) {
    size_t i;

    if (port_address == 0) {
        return NULL;
    }
    for (i = 0; i < cpu->port_count; i++) {
        if (cpu->ports[i]->config.PORT == port_address) {
            return cpu->ports[i];
        }
    }
    return NULL;
}

static void write_gpio(AvrPort *port, uint8_t value, uint8_t ddr) {
    uint8_t new_value = (uint8_t)((((value & port->override_mask) | port->override_value) & ddr) |
                                  (value & ~ddr));
    uint8_t prev_value = port->last_value;

    if (new_value != prev_value || ddr != port->last_ddr) {
        port->last_value = new_value;
        port->last_ddr = ddr;
        if (port->listener.fn != NULL) {
            port->listener.fn(port->listener.ctx, new_value, prev_value);
        }
    }
}

static void toggle_interrupt(AvrPort *port, int pin, int rising_edge) {
    AvrCpu *cpu = port->cpu;
    const AvrExternalInterruptConfig *external_config = &port->config.external[pin];

    if (external_config->present) {
        AvrInterrupt *external = &port->external_ints[pin];

        if (cpu->data[external_config->EIMSK] & (1 << external_config->index)) {
            int configuration = (cpu->data[external_config->EICR] >> external_config->isc_offset) & 3;
            int generate = 0;

            external->constant = 0;
            switch (configuration) {
                case MODE_LOW_LEVEL:
                    generate = !rising_edge;
                    external->constant = 1;
                    break;
                case MODE_CHANGE:
                    generate = 1;
                    break;
                case MODE_FALLING_EDGE:
                    generate = !rising_edge;
                    break;
                case MODE_RISING_EDGE:
                    generate = rising_edge;
                    break;
            }
            if (generate) {
                avr_set_interrupt_flag(cpu, external);
            } else if (external->constant) {
                avr_clear_interrupt(cpu, external, 1);
            }
        }
    }

    if (port->config.has_pin_change && (port->config.pin_change.mask & (1 << pin))) {
        const AvrPinChangeConfig *pin_change = &port->config.pin_change;

        if (cpu->data[pin_change->PCMSK] & (1 << (pin + pin_change->offset))) {
            avr_set_interrupt_flag(cpu, &port->pcint);
        }
    }
}

static void update_pin_register(AvrPort *port, uint8_t ddr) {
    uint8_t new_pin = (uint8_t)((port->pin_value & ~ddr) | (port->last_value & ddr));
    int index;

    port->cpu->data[port->config.PIN] = new_pin;
    if (port->last_pin != new_pin) {
        for (index = 0; index < 8; index++) {
            if ((new_pin & (1 << index)) != (port->last_pin & (1 << index))) {
                int value = (new_pin & (1 << index)) != 0;

                toggle_interrupt(port, index, value);
                if (port->external_clock[index].fn != NULL) {
                    port->external_clock[index].fn(port->external_clock[index].ctx, value);
                }
            }
        }
        port->last_pin = new_pin;
    }
}

static int ddr_hook(void *ctx, uint8_t value, uint8_t old_value, uint16_t addr, uint8_t mask) {
    AvrPort *port = ctx;
    AvrCpu *cpu = port->cpu;
    uint8_t port_value = cpu->data[port->config.PORT];

    (void)old_value;
    (void)addr;
    (void)mask;
    cpu->data[port->config.DDR] = value;
    write_gpio(port, port_value, value);
    update_pin_register(port, value);
    return 1;
}

static int port_hook(void *ctx, uint8_t value, uint8_t old_value, uint16_t addr, uint8_t mask) {
    AvrPort *port = ctx;
    AvrCpu *cpu = port->cpu;
    uint8_t ddr_mask = cpu->data[port->config.DDR];

    (void)old_value;
    (void)addr;
    (void)mask;
    cpu->data[port->config.PORT] = value;
    write_gpio(port, value, ddr_mask);
    update_pin_register(port, ddr_mask);
    return 1;
}

/* Writing 1 to a PIN bit toggles the PORT bit. */
static int pin_hook(void *ctx, uint8_t value, uint8_t old_value, uint16_t addr, uint8_t mask) {
    AvrPort *port = ctx;
    AvrCpu *cpu = port->cpu;
    uint8_t old_port_value = cpu->data[port->config.PORT];
    uint8_t ddr_mask = cpu->data[port->config.DDR];
    uint8_t port_value = old_port_value ^ (value & mask);

    (void)old_value;
    (void)addr;
    cpu->data[port->config.PORT] = port_value;
    write_gpio(port, port_value, ddr_mask);
    update_pin_register(port, ddr_mask);
    return 1;
}

static void check_external_interrupts(AvrPort *port) {
    AvrCpu *cpu = port->cpu;
    int pin;

    for (pin = 0; pin < 8; pin++) {
        const AvrExternalInterruptConfig *external = &port->config.external[pin];
        int pin_value = (port->last_pin & (1 << pin)) != 0;

        if (!external->present) {
            continue;
        }
        if (!(cpu->data[external->EIMSK] & (1 << external->index)) || pin_value) {
            continue;
        }
        if (((cpu->data[external->EICR] >> external->isc_offset) & 3) == MODE_LOW_LEVEL) {
            AvrInterrupt *low = &port->low_level[pin];

            low->address = external->interrupt;
            low->flag_register = external->EIFR;
            low->flag_mask = (uint8_t)(1 << external->index);
            low->enable_register = external->EIMSK;
            low->enable_mask = (uint8_t)(1 << external->index);
            low->constant = 1;
            low->inverse_flag = 0;
            avr_queue_interrupt(cpu, low);
        }
    }
}

/* A write to EICRx, EIMSK or EIFR: every port's external interrupts follow. */
static int interrupt_hook(void *ctx, uint8_t value, uint8_t old_value, uint16_t addr, uint8_t mask) {
    const AvrPortHook *hook = ctx;
    AvrCpu *cpu = hook->port->cpu;
    size_t p;
    int i;

    (void)old_value;
    (void)addr;
    (void)mask;
    if (hook->type != HOOK_FLAG) {
        cpu->data[hook->reg] = value;
    }
    for (p = 0; p < cpu->port_count; p++) {
        AvrPort *gpio = cpu->ports[p];

        for (i = 0; i < 8; i++) {
            AvrInterrupt *external = &gpio->external_ints[i];

            if (!gpio->config.external[i].present) {
                continue;
            }
            if (hook->type == HOOK_MASK) {
                avr_update_interrupt_enable(cpu, external, value);
            }
            if (!external->constant && hook->type == HOOK_FLAG) {
                avr_clear_interrupt_by_flag(cpu, external, value);
            }
        }
        check_external_interrupts(gpio);
    }
    return 1;
}

static void attach_interrupt_hook(AvrPort *port, uint16_t reg, int type) {
    AvrPortHook *hook;

    if (reg == 0 || reg >= port->cpu->data_size ||
        port->hook_count >= sizeof port->hooks / sizeof port->hooks[0]) {
        return;
    }
    hook = &port->hooks[port->hook_count++];
    hook->port = port;
    hook->reg = reg;
    hook->type = type;
    port->cpu->write_hooks[reg].fn = interrupt_hook;
    port->cpu->write_hooks[reg].ctx = hook;
}

static int pcifr_hook(void *ctx, uint8_t value, uint8_t old_value, uint16_t addr, uint8_t mask) {
    AvrPort *port = ctx;
    AvrCpu *cpu = port->cpu;
    size_t p;

    (void)old_value;
    (void)addr;
    (void)mask;
    for (p = 0; p < cpu->port_count; p++) {
        if (cpu->ports[p]->config.has_pin_change) {
            avr_clear_interrupt_by_flag(cpu, &cpu->ports[p]->pcint, value);
        }
    }
    return 1;
}

static int pcmsk_hook(void *ctx, uint8_t value, uint8_t old_value, uint16_t addr, uint8_t mask) {
    AvrPort *port = ctx;
    AvrCpu *cpu = port->cpu;
    size_t p;

    (void)old_value;
    (void)addr;
    (void)mask;
    cpu->data[port->config.pin_change.PCMSK] = value;
    for (p = 0; p < cpu->port_count; p++) {
        if (cpu->ports[p]->config.has_pin_change) {
            avr_update_interrupt_enable(cpu, &cpu->ports[p]->pcint, value);
        }
    }
    return 1;
}

static void set_hook(AvrCpu *cpu, uint16_t addr, AvrWriteFn fn, void *ctx) {
    if (addr < cpu->data_size) {
        cpu->write_hooks[addr].fn = fn;
        cpu->write_hooks[addr].ctx = ctx;
    }
}

void avr_port_init(AvrPort *port, AvrCpu *cpu, const AvrPortConfig *config) {
    uint16_t seen_eicr[8];
    size_t seen = 0, s;
    uint16_t eimsk = 0, eifr = 0;
    int i;

    memset(port, 0, sizeof(*port));
    port->cpu = cpu;
    port->config = *config;
    port->override_mask = 0xff;
    if (cpu->port_count < AVR_MAX_PORTS) {
        cpu->ports[cpu->port_count++] = port;
    }

    set_hook(cpu, config->DDR, ddr_hook, port);
    set_hook(cpu, config->PORT, port_hook, port);
    set_hook(cpu, config->PIN, pin_hook, port);

    /* External interrupts. */
    for (i = 0; i < 8; i++) {
        const AvrExternalInterruptConfig *external = &config->external[i];

        if (!external->present) {
            continue;
        }
        port->external_ints[i].address = external->interrupt;
        port->external_ints[i].flag_register = external->EIFR;
        port->external_ints[i].flag_mask = (uint8_t)(1 << external->index);
        port->external_ints[i].enable_register = external->EIMSK;
        port->external_ints[i].enable_mask = (uint8_t)(1 << external->index);
        for (s = 0; s < seen && seen_eicr[s] != external->EICR; s++) {
        }
        if (s == seen) {
            seen_eicr[seen++] = external->EICR;
            attach_interrupt_hook(port, external->EICR, HOOK_OTHER);
        }
        if (eimsk == 0 && external->EIMSK != 0) {
            eimsk = external->EIMSK;
        }
        if (eifr == 0 && external->EIFR != 0) {
            eifr = external->EIFR;
        }
    }
    attach_interrupt_hook(port, eimsk, HOOK_MASK);
    attach_interrupt_hook(port, eifr, HOOK_FLAG);

    /* The pin-change interrupt. */
    if (config->has_pin_change) {
        const AvrPinChangeConfig *pin_change = &config->pin_change;

        port->pcint.address = pin_change->pin_change_interrupt;
        port->pcint.flag_register = pin_change->PCIFR;
        port->pcint.flag_mask = (uint8_t)(1 << pin_change->PCIE);
        port->pcint.enable_register = pin_change->PCICR;
        port->pcint.enable_mask = (uint8_t)(1 << pin_change->PCIE);
        set_hook(cpu, pin_change->PCIFR, pcifr_hook, port);
        set_hook(cpu, pin_change->PCMSK, pcmsk_hook, port);
    }
}

AvrPinState avr_port_pin_state(const AvrPort *port, int index) {
    uint8_t ddr = port->cpu->data[port->config.DDR];
    uint8_t value = port->cpu->data[port->config.PORT];
    uint8_t bit_mask = (uint8_t)(1 << index);
    AvrPinState open_state = (value & bit_mask) ? AVR_PIN_INPUT_PULLUP : AVR_PIN_INPUT;
    AvrPinState high_value = (port->open_collector & bit_mask) ? open_state : AVR_PIN_HIGH;

    if (ddr & bit_mask) {
        return (port->last_value & bit_mask) ? high_value : AVR_PIN_LOW;
    }
    return open_state;
}

void avr_port_set_pin(AvrPort *port, int index, int value) {
    uint8_t bit_mask = (uint8_t)(1 << index);

    port->pin_value &= (uint8_t)~bit_mask;
    if (value) {
        port->pin_value |= bit_mask;
    }
    update_pin_register(port, port->cpu->data[port->config.DDR]);
}

void avr_port_override_pin(AvrPort *port, int pin, AvrPinOverride mode) {
    AvrCpu *cpu = port->cpu;
    uint8_t pin_mask = (uint8_t)(1 << pin);
    uint8_t ddr_mask;

    if (mode == AVR_OVERRIDE_NONE) {
        port->override_mask |= pin_mask;
        port->override_value &= (uint8_t)~pin_mask;
    } else {
        port->override_mask &= (uint8_t)~pin_mask;
        switch (mode) {
            case AVR_OVERRIDE_ENABLE:
                port->override_value &= (uint8_t)~pin_mask;
                port->override_value |= cpu->data[port->config.PORT] & pin_mask;
                break;
            case AVR_OVERRIDE_SET:
                port->override_value |= pin_mask;
                break;
            case AVR_OVERRIDE_CLEAR:
                port->override_value &= (uint8_t)~pin_mask;
                break;
            case AVR_OVERRIDE_TOGGLE:
                port->override_value ^= pin_mask;
                break;
            case AVR_OVERRIDE_NONE:
                break;
        }
    }
    ddr_mask = cpu->data[port->config.DDR];
    write_gpio(port, cpu->data[port->config.PORT], ddr_mask);
    update_pin_register(port, ddr_mask);
}
