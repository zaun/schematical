/*
 * The AVR CPU's data structures: memory, hooks, the interrupt queue and
 * clock events. A C port of avr8js 0.21.1's src/cpu/cpu.ts and
 * src/cpu/interrupt.ts (Copyright (c) Uri Shaked and contributors, MIT; see
 * LICENSE-avr8js). plan-62-A.
 */

#include <stdlib.h>
#include <string.h>

#include "avr.h"

AvrCpu *avr_cpu_create(uint32_t flash_bytes, uint32_t sram_bytes) {
    AvrCpu *cpu = calloc(1, sizeof(*cpu));

    if (cpu == NULL) {
        return NULL;
    }
    cpu->data_size = sram_bytes + AVR_REGISTER_SPACE;
    cpu->flash_bytes = flash_bytes;
    cpu->flash_words = flash_bytes / 2;
    cpu->pc22 = flash_bytes > 0x20000;
    cpu->data = calloc(cpu->data_size, 1);
    cpu->flash = malloc(flash_bytes > 0 ? flash_bytes : 1);
    cpu->read_hooks = calloc(cpu->data_size, sizeof(*cpu->read_hooks));
    cpu->write_hooks = calloc(cpu->data_size, sizeof(*cpu->write_hooks));
    if (cpu->data == NULL || cpu->flash == NULL || cpu->read_hooks == NULL ||
        cpu->write_hooks == NULL) {
        avr_cpu_destroy(cpu);
        return NULL;
    }
    memset(cpu->flash, 0xff, flash_bytes);
    avr_cpu_reset(cpu);
    return cpu;
}

static void free_list(AvrClockEvent *event) {
    while (event != NULL) {
        AvrClockEvent *next = event->next;

        free(event);
        event = next;
    }
}

/* 1 when `event` is one of the nodes of `list`. */
static int in_list(const AvrClockEvent *list, const AvrClockEvent *event) {
    for (; list != NULL; list = list->next) {
        if (list == event) {
            return 1;
        }
    }
    return 0;
}

static int in_pool(const AvrCpu *cpu, const AvrClockEvent *event) {
    size_t i;

    for (i = 0; i < cpu->event_pool_count; i++) {
        if (cpu->event_pool[i] == event) {
            return 1;
        }
    }
    return 0;
}

void avr_cpu_destroy(AvrCpu *cpu) {
    AvrClockEvent *event;
    size_t i;

    if (cpu == NULL) {
        return;
    }
    /* A node may sit in the pool and in a list at once (see spare_event);
       free each node once. */
    for (i = 0; i < cpu->event_pool_count; i++) {
        event = cpu->event_pool[i];
        if (event != NULL && !in_list(cpu->next_event, event) &&
            !in_list(cpu->spare_events, event)) {
            size_t j;

            for (j = i + 1; j < cpu->event_pool_count; j++) {
                if (cpu->event_pool[j] == event) {
                    cpu->event_pool[j] = NULL;
                }
            }
            free(event);
        }
    }
    free_list(cpu->next_event);
    free_list(cpu->spare_events);
    free(cpu->data);
    free(cpu->flash);
    free(cpu->read_hooks);
    free(cpu->write_hooks);
    free(cpu);
}

/*
 * A node avr8js drops (its pool of 10 is full, or reset forgot the list) is
 * garbage there. Here it goes to spare_events, where the next new node
 * comes from, so a long run does not grow memory. A node that is still
 * reachable (queued, pooled, or the one whose callback is running) is left
 * alone: avr8js can hold one node in two places, and the port keeps exactly
 * its behavior.
 */
static void spare_event(AvrCpu *cpu, AvrClockEvent *event) {
    if (event == cpu->running_event || in_list(cpu->next_event, event) || in_pool(cpu, event) ||
        in_list(cpu->spare_events, event)) {
        return;
    }
    event->next = cpu->spare_events;
    cpu->spare_events = event;
}

void avr_cpu_reset(AvrCpu *cpu) {
    AvrClockEvent *event = cpu->next_event;

    avr_set16(cpu, AVR_SP, cpu->data_size - 1);
    cpu->pc = 0;
    memset(cpu->pending, 0, sizeof(cpu->pending));
    cpu->next_interrupt = -1;
    cpu->next_event = NULL;
    if (cpu->running_event != NULL) {
        /* avr8js: tick() then sets nextClockEvent to the running event's
           next, which brings the rest of the old list back. Keep it. */
        cpu->reset_in_event = 1;
        return;
    }
    while (event != NULL) {
        AvrClockEvent *next = event->next;

        spare_event(cpu, event);
        event = next;
    }
}

uint8_t avr_read_data(AvrCpu *cpu, int32_t addr) {
    /* Past the end avr8js reads `undefined`, which stores as 0. */
    if (addr < 0 || (uint32_t)addr >= cpu->data_size) {
        return 0;
    }
    if (addr >= 32 && cpu->read_hooks[addr].fn != NULL) {
        return cpu->read_hooks[addr].fn(cpu->read_hooks[addr].ctx, (uint16_t)addr);
    }
    return cpu->data[addr];
}

void avr_write_data(AvrCpu *cpu, int32_t addr, uint8_t value, uint8_t mask) {
    /* Past the end a typed array drops the store. */
    if (addr < 0 || (uint32_t)addr >= cpu->data_size) {
        return;
    }
    if (cpu->write_hooks[addr].fn != NULL &&
        cpu->write_hooks[addr].fn(cpu->write_hooks[addr].ctx, value, cpu->data[addr],
                                  (uint16_t)addr, mask)) {
        return;
    }
    cpu->data[addr] = value;
}

/* ---- Interrupts ---------------------------------------------------------- */

void avr_set_interrupt_flag(AvrCpu *cpu, const AvrInterrupt *interrupt) {
    if (interrupt->inverse_flag) {
        cpu->data[interrupt->flag_register] &= (uint8_t)~interrupt->flag_mask;
    } else {
        cpu->data[interrupt->flag_register] |= interrupt->flag_mask;
    }
    if (cpu->data[interrupt->enable_register] & interrupt->enable_mask) {
        avr_queue_interrupt(cpu, interrupt);
    }
}

void avr_update_interrupt_enable(AvrCpu *cpu, const AvrInterrupt *interrupt,
                                 uint8_t register_value) {
    if (register_value & interrupt->enable_mask) {
        int bit_set = (cpu->data[interrupt->flag_register] & interrupt->flag_mask) != 0;

        if (interrupt->inverse_flag ? !bit_set : bit_set) {
            avr_queue_interrupt(cpu, interrupt);
        }
    } else {
        avr_clear_interrupt(cpu, interrupt, 0);
    }
}

void avr_queue_interrupt(AvrCpu *cpu, const AvrInterrupt *interrupt) {
    int address = interrupt->address;

    cpu->pending[address] = interrupt;
    if (cpu->next_interrupt == -1 || cpu->next_interrupt > address) {
        cpu->next_interrupt = address;
    }
    if (address > cpu->max_interrupt) {
        cpu->max_interrupt = address;
    }
}

void avr_clear_interrupt(AvrCpu *cpu, const AvrInterrupt *interrupt, int clear_flag) {
    int address = interrupt->address;
    int i;

    if (clear_flag) {
        cpu->data[interrupt->flag_register] &= (uint8_t)~interrupt->flag_mask;
    }
    if (cpu->pending[address] == NULL) {
        return;
    }
    cpu->pending[address] = NULL;
    if (cpu->next_interrupt == address) {
        cpu->next_interrupt = -1;
        for (i = address + 1; i <= cpu->max_interrupt; i++) {
            if (cpu->pending[i] != NULL) {
                cpu->next_interrupt = i;
                break;
            }
        }
    }
}

void avr_clear_interrupt_by_flag(AvrCpu *cpu, const AvrInterrupt *interrupt,
                                 uint8_t register_value) {
    if (register_value & interrupt->flag_mask) {
        cpu->data[interrupt->flag_register] &= (uint8_t)~interrupt->flag_mask;
        avr_clear_interrupt(cpu, interrupt, 1);
    }
}

void avr_interrupt(AvrCpu *cpu, uint8_t addr) {
    uint16_t sp = avr_get16(cpu, AVR_SP);

    /* Raw stores, as avr8js's cpu.data[sp] = …: no hooks, dropped past the end. */
    if (sp < cpu->data_size) {
        cpu->data[sp] = (uint8_t)cpu->pc;
    }
    if ((uint32_t)(sp - 1) < cpu->data_size && sp >= 1) {
        cpu->data[sp - 1] = (uint8_t)(cpu->pc >> 8);
    }
    if (cpu->pc22 && sp >= 2 && (uint32_t)(sp - 2) < cpu->data_size) {
        cpu->data[sp - 2] = (uint8_t)(cpu->pc >> 16);
    }
    avr_set16(cpu, AVR_SP, (uint32_t)(sp - (cpu->pc22 ? 3 : 2)));
    cpu->data[AVR_SREG] &= 0x7f;   /* clear the global interrupt flag */
    /* avr8js adds 2 cycles here. The ATmega328P datasheet ("Interrupt
       Response Time") says the response takes four clock cycles:
       the program counter is pushed and the vector's jump is fetched. We
       follow the datasheet. `exact` keeps avr8js's 2 so the oracle tests
       compare whole traces. */
    cpu->cycles += cpu->exact ? 2 : 4;
    cpu->pc = addr;
}

/* ---- Clock events -------------------------------------------------------- */

void avr_add_clock_event(AvrCpu *cpu, AvrEventFn fn, void *ctx, double cycles) {
    AvrClockEvent *entry;
    AvrClockEvent *event;
    AvrClockEvent *last = NULL;

    cycles = (double)cpu->cycles + (cycles > 1 ? cycles : 1);
    if (cpu->event_pool_count > 0) {
        entry = cpu->event_pool[--cpu->event_pool_count];
    } else if (cpu->spare_events != NULL) {
        entry = cpu->spare_events;
        cpu->spare_events = entry->next;
        entry->next = NULL;
    } else {
        entry = calloc(1, sizeof(*entry));
        if (entry == NULL) {
            return;
        }
    }
    entry->cycles = cycles;
    entry->fn = fn;
    entry->ctx = ctx;
    event = cpu->next_event;
    while (event != NULL && event->cycles < cycles) {
        last = event;
        event = event->next;
    }
    if (last != NULL) {
        last->next = entry;
        entry->next = event;
    } else {
        cpu->next_event = entry;
        entry->next = event;
    }
}

int avr_update_clock_event(AvrCpu *cpu, AvrEventFn fn, void *ctx, double cycles) {
    if (avr_clear_clock_event(cpu, fn, ctx)) {
        avr_add_clock_event(cpu, fn, ctx, cycles);
        return 1;
    }
    return 0;
}

int avr_clear_clock_event(AvrCpu *cpu, AvrEventFn fn, void *ctx) {
    AvrClockEvent *event = cpu->next_event;
    AvrClockEvent *last = NULL;

    while (event != NULL) {
        if (event->fn == fn && event->ctx == ctx) {
            if (last != NULL) {
                last->next = event->next;
            } else {
                cpu->next_event = event->next;
            }
            if (cpu->event_pool_count < AVR_EVENT_POOL) {
                cpu->event_pool[cpu->event_pool_count++] = event;
            } else {
                spare_event(cpu, event);
            }
            return 1;
        }
        last = event;
        event = event->next;
    }
    return 0;
}

void avr_cpu_tick(AvrCpu *cpu) {
    AvrClockEvent *event = cpu->next_event;
    int next;

    if (event != NULL && event->cycles <= (double)cpu->cycles) {
        AvrClockEvent *lost = NULL;

        cpu->running_event = event;
        cpu->reset_in_event = 0;
        event->fn(event->ctx);
        cpu->running_event = NULL;
        if (cpu->reset_in_event) {
            /* Events added after the reset are dropped by the assignment
               below, as in avr8js. */
            lost = cpu->next_event;
            cpu->reset_in_event = 0;
        }
        cpu->next_event = event->next;
        while (lost != NULL) {
            AvrClockEvent *after = lost->next;

            spare_event(cpu, lost);
            lost = after;
        }
        if (cpu->event_pool_count < AVR_EVENT_POOL) {
            cpu->event_pool[cpu->event_pool_count++] = event;
        } else {
            spare_event(cpu, event);
        }
    }

    next = cpu->next_interrupt;
    if ((cpu->data[AVR_SREG] & 0x80) && next >= 0) {
        const AvrInterrupt *interrupt = cpu->pending[next];

        avr_interrupt(cpu, interrupt->address);
        if (!interrupt->constant) {
            avr_clear_interrupt(cpu, interrupt, 1);
        }
    }
}
