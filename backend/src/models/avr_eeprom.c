/*
 * The AVR's on-chip EEPROM. A C port of avr8js 0.21.1's
 * src/peripherals/eeprom.ts with its EEPROMMemoryBackend (Copyright (c) Uri
 * Shaked and contributors, MIT; see LICENSE-avr8js). plan-62-C.
 */

#include <stdlib.h>
#include <string.h>

#include "avr.h"

enum {
    EERE = 1 << 0, EEPE = 1 << 1, EEMPE = 1 << 2, EERIE = 1 << 3, EEPM0 = 1 << 4, EEPM1 = 1 << 5,
    EECR_WRITE_MASK = EEPE | EEMPE | EERIE | EEPM0 | EEPM1
};

static void clear_eempe(void *ctx) {
    AvrEeprom *eeprom = ctx;

    eeprom->cpu->data[eeprom->config.EECR] &= (uint8_t)~EEMPE;
}

static void write_done(void *ctx) {
    AvrEeprom *eeprom = ctx;

    avr_set_interrupt_flag(eeprom->cpu, &eeprom->EER);
}

static int eecr_write(void *ctx, uint8_t eecr, uint8_t old_value, uint16_t hook_addr, uint8_t mask) {
    AvrEeprom *eeprom = ctx;
    AvrCpu *cpu = eeprom->cpu;
    const AvrEepromConfig *config = &eeprom->config;
    uint32_t addr = ((uint32_t)cpu->data[config->EEARH] << 8) | cpu->data[config->EEARL];

    (void)old_value;
    (void)hook_addr;
    (void)mask;
    cpu->data[config->EECR] = (uint8_t)((cpu->data[config->EECR] & ~EECR_WRITE_MASK) |
                                        (eecr & EECR_WRITE_MASK));
    avr_update_interrupt_enable(cpu, &eeprom->EER, eecr);

    if (eecr & EERE) {
        avr_clear_interrupt(cpu, &eeprom->EER, 1);
    }

    if (eecr & EEMPE) {
        /* Setting EEPE within four clock cycles writes; the bit then clears. */
        eeprom->write_enabled_cycles = (double)cpu->cycles + 4;
        avr_add_clock_event(cpu, clear_eempe, eeprom, 4);
    }

    /* Read. */
    if (eecr & EERE) {
        /* An address past the end reads `undefined` in avr8js, stored as 0. */
        cpu->data[config->EEDR] = addr < eeprom->size ? eeprom->memory[addr] : 0;
        /* The CPU is halted for four cycles before the next instruction. */
        cpu->cycles += 4;
        return 1;
    }

    /* Write. */
    if (eecr & EEPE) {
        uint8_t eedr;

        /* If EEMPE is zero, setting EEPE has no effect. */
        if ((double)cpu->cycles >= eeprom->write_enabled_cycles) {
            cpu->data[config->EECR] &= (uint8_t)~EEPE;
            return 1;
        }
        /* A write is in progress. */
        if ((double)cpu->cycles < eeprom->write_complete_cycles) {
            return 1;
        }
        eedr = cpu->data[config->EEDR];
        eeprom->write_complete_cycles = (double)cpu->cycles;
        if (!(eecr & EEPM1)) {   /* erase */
            if (addr < eeprom->size) {
                eeprom->memory[addr] = 0xff;
            }
            eeprom->write_complete_cycles += config->erase_cycles;
        }
        if (!(eecr & EEPM0)) {   /* write: bits only go from 1 to 0 */
            if (addr < eeprom->size) {
                eeprom->memory[addr] &= eedr;
            }
            eeprom->write_complete_cycles += config->write_cycles;
        }
        cpu->data[config->EECR] |= EEPE;
        avr_add_clock_event(cpu, write_done, eeprom,
                            eeprom->write_complete_cycles - (double)cpu->cycles);
        /* The CPU is halted for two cycles before the next instruction. */
        cpu->cycles += 2;
    }
    return 1;
}

void avr_eeprom_reset(AvrEeprom *eeprom) {
    eeprom->write_enabled_cycles = 0;
    eeprom->write_complete_cycles = 0;
}

int avr_eeprom_init(AvrEeprom *eeprom, AvrCpu *cpu, uint32_t size, const AvrEepromConfig *config) {
    memset(eeprom, 0, sizeof(*eeprom));
    eeprom->cpu = cpu;
    eeprom->config = *config;
    eeprom->size = size;
    eeprom->memory = malloc(size > 0 ? size : 1);
    if (eeprom->memory == NULL) {
        return -1;
    }
    memset(eeprom->memory, 0xff, size);
    eeprom->EER.address = config->eeprom_ready_interrupt;
    eeprom->EER.flag_register = config->EECR;
    eeprom->EER.flag_mask = EEPE;
    eeprom->EER.enable_register = config->EECR;
    eeprom->EER.enable_mask = EERIE;
    eeprom->EER.constant = 1;
    eeprom->EER.inverse_flag = 1;
    if (config->EECR < cpu->data_size) {
        cpu->write_hooks[config->EECR].fn = eecr_write;
        cpu->write_hooks[config->EECR].ctx = eeprom;
    }
    return 0;
}

void avr_eeprom_free(AvrEeprom *eeprom) {
    free(eeprom->memory);
    eeprom->memory = NULL;
}
