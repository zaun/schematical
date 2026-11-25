/*
 * The AVR SPI at register level: a byte written to SPDR is handed to a
 * callback, which completes the transfer with the byte received. A C port
 * of avr8js 0.21.1's src/peripherals/spi.ts (Copyright (c) Uri Shaked and
 * contributors, MIT; see LICENSE-avr8js). plan-62-C.
 */

#include <string.h>

#include "avr.h"

enum {
    SPCR_SPIE = 0x80, SPCR_SPE = 0x40, SPCR_DORD = 0x20, SPCR_MSTR = 0x10, SPCR_CPOL = 0x08,
    SPCR_CPHA = 0x04, SPCR_SPR_MASK = 0x03,
    SPSR_SPIF = 0x80, SPSR_WCOL = 0x40, SPSR_SPI2X = 0x01
};

#define HOOK_ARGS void *ctx, uint8_t value, uint8_t old_value, uint16_t addr, uint8_t mask
#define HOOK_UNUSED (void)old_value; (void)addr; (void)mask

int avr_spi_is_master(const AvrSpi *spi) {
    return (spi->cpu->data[spi->config.SPCR] & SPCR_MSTR) != 0;
}

int avr_spi_lsb_first(const AvrSpi *spi) {
    return (spi->cpu->data[spi->config.SPCR] & SPCR_DORD) != 0;
}

int avr_spi_mode(const AvrSpi *spi) {
    uint8_t spcr = spi->cpu->data[spi->config.SPCR];

    return ((spcr & SPCR_CPHA) ? 2 : 0) | ((spcr & SPCR_CPOL) ? 1 : 0);
}

int avr_spi_clock_divider(const AvrSpi *spi) {
    int base = (spi->cpu->data[spi->config.SPSR] & SPSR_SPI2X) ? 2 : 4;

    switch (spi->cpu->data[spi->config.SPCR] & SPCR_SPR_MASK) {
        case 0: return base;
        case 1: return base * 4;
        case 2: return base * 16;
        default: return base * 32;
    }
}

int avr_spi_transfer_cycles(const AvrSpi *spi) {
    return avr_spi_clock_divider(spi) * 8;
}

void avr_spi_complete_transfer(AvrSpi *spi, uint8_t received) {
    spi->cpu->data[spi->config.SPDR] = received;
    avr_set_interrupt_flag(spi->cpu, &spi->SPI);
    spi->transmission_active = 0;
}

/* avr8js's default onByte: nothing is connected, 0 comes back. */
static void default_complete(void *ctx) {
    avr_spi_complete_transfer(ctx, 0);
}

static int spdr_write(HOOK_ARGS) {
    AvrSpi *spi = ctx;
    AvrCpu *cpu = spi->cpu;

    HOOK_UNUSED;
    if (!(cpu->data[spi->config.SPCR] & SPCR_SPE)) {
        return 0;   /* SPI not enabled: an ordinary store */
    }
    if (spi->transmission_active) {
        cpu->data[spi->config.SPSR] |= SPSR_WCOL;   /* write collision */
        return 1;
    }
    cpu->data[spi->config.SPSR] &= (uint8_t)~SPSR_WCOL;
    avr_clear_interrupt(cpu, &spi->SPI, 1);
    spi->transmission_active = 1;
    if (spi->on_byte.fn != NULL) {
        spi->on_byte.fn(spi->on_byte.ctx, value);
    } else {
        avr_add_clock_event(cpu, default_complete, spi, avr_spi_transfer_cycles(spi));
    }
    return 1;
}

static int spcr_write(HOOK_ARGS) {
    AvrSpi *spi = ctx;

    HOOK_UNUSED;
    avr_update_interrupt_enable(spi->cpu, &spi->SPI, value);
    return 0;
}

static int spsr_write(HOOK_ARGS) {
    AvrSpi *spi = ctx;

    HOOK_UNUSED;
    spi->cpu->data[spi->config.SPSR] = value;
    avr_clear_interrupt_by_flag(spi->cpu, &spi->SPI, value);
    return 0;
}

void avr_spi_reset(AvrSpi *spi) {
    spi->transmission_active = 0;
}

void avr_spi_init(AvrSpi *spi, AvrCpu *cpu, const AvrSpiConfig *config, uint32_t freq_hz) {
    memset(spi, 0, sizeof(*spi));
    spi->cpu = cpu;
    spi->config = *config;
    spi->freq_hz = freq_hz;
    spi->SPI.address = config->spi_interrupt;
    spi->SPI.flag_register = config->SPSR;
    spi->SPI.flag_mask = SPSR_SPIF;
    spi->SPI.enable_register = config->SPCR;
    spi->SPI.enable_mask = SPCR_SPIE;
    if (config->SPDR < cpu->data_size && config->SPCR < cpu->data_size &&
        config->SPSR < cpu->data_size) {
        cpu->write_hooks[config->SPDR].fn = spdr_write;
        cpu->write_hooks[config->SPDR].ctx = spi;
        cpu->write_hooks[config->SPCR].fn = spcr_write;
        cpu->write_hooks[config->SPCR].ctx = spi;
        cpu->write_hooks[config->SPSR].fn = spsr_write;
        cpu->write_hooks[config->SPSR].ctx = spi;
    }
}
