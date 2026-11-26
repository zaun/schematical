/*
 * The AVR USART at register level: a byte written to UDR is handed to a
 * callback, a received byte arrives through avr_usart_write_byte. A C port
 * of avr8js 0.21.1's src/peripherals/usart.ts (Copyright (c) Uri Shaked and
 * contributors, MIT; see LICENSE-avr8js). plan-62-C.
 *
 * As in avr8js: asynchronous mode only; no frame, parity or overrun error.
 */

#include <string.h>

#include "avr.h"

enum {
    UCSRA_RXC = 0x80, UCSRA_TXC = 0x40, UCSRA_UDRE = 0x20, UCSRA_U2X = 0x02, UCSRA_MPCM = 0x01,
    UCSRA_CFG_MASK = UCSRA_U2X,
    UCSRB_RXCIE = 0x80, UCSRB_TXCIE = 0x40, UCSRB_UDRIE = 0x20, UCSRB_RXEN = 0x10,
    UCSRB_TXEN = 0x08, UCSRB_UCSZ2 = 0x04,
    UCSRB_CFG_MASK = UCSRB_UCSZ2 | UCSRB_RXEN | UCSRB_TXEN,
    UCSRC_UPM1 = 0x20, UCSRC_UPM0 = 0x10, UCSRC_USBS = 0x08, UCSRC_UCSZ1 = 0x04,
    UCSRC_UCSZ0 = 0x02
};

#define HOOK_ARGS void *ctx, uint8_t value, uint8_t old_value, uint16_t addr, uint8_t mask

static void config_changed(AvrUsart *usart) {
    if (usart->on_config.fn != NULL) {
        usart->on_config.fn(usart->on_config.ctx);
    }
}

int avr_usart_rx_enabled(const AvrUsart *usart) {
    return (usart->cpu->data[usart->config.UCSRB] & UCSRB_RXEN) != 0;
}

int avr_usart_tx_enabled(const AvrUsart *usart) {
    return (usart->cpu->data[usart->config.UCSRB] & UCSRB_TXEN) != 0;
}

int avr_usart_bits(const AvrUsart *usart) {
    int ucsz = ((usart->cpu->data[usart->config.UCSRC] & (UCSRC_UCSZ1 | UCSRC_UCSZ0)) >> 1) |
               (usart->cpu->data[usart->config.UCSRB] & UCSRB_UCSZ2);

    switch (ucsz) {
        case 0: return 5;
        case 1: return 6;
        case 2: return 7;
        case 3: return 8;
        default: return 9;   /* 4..6 are reserved; 7 is 9 bits */
    }
}

int avr_usart_stop_bits(const AvrUsart *usart) {
    return (usart->cpu->data[usart->config.UCSRC] & UCSRC_USBS) ? 2 : 1;
}

int avr_usart_parity(const AvrUsart *usart) {
    uint8_t ucsrc = usart->cpu->data[usart->config.UCSRC];

    if (!(ucsrc & UCSRC_UPM1)) {
        return 0;
    }
    return (ucsrc & UCSRC_UPM0) ? 2 : 1;
}

uint32_t avr_usart_bit_cycles(const AvrUsart *usart) {
    uint32_t ubrr = ((uint32_t)usart->cpu->data[usart->config.UBRRH] << 8) |
                    usart->cpu->data[usart->config.UBRRL];
    uint32_t multiplier = (usart->cpu->data[usart->config.UCSRA] & UCSRA_U2X) ? 8 : 16;

    return (ubrr + 1) * multiplier;
}

uint32_t avr_usart_char_cycles(const AvrUsart *usart) {
    uint32_t symbols = 1 + (uint32_t)avr_usart_bits(usart) + (uint32_t)avr_usart_stop_bits(usart) +
                       (avr_usart_parity(usart) ? 1 : 0);

    return avr_usart_bit_cycles(usart) * symbols;
}

static int ucsra_write(HOOK_ARGS) {
    AvrUsart *usart = ctx;
    AvrCpu *cpu = usart->cpu;

    (void)addr;
    (void)mask;
    cpu->data[usart->config.UCSRA] = value & (UCSRA_MPCM | UCSRA_U2X);
    avr_clear_interrupt_by_flag(cpu, &usart->TXC, value);
    if ((value & UCSRA_CFG_MASK) != (old_value & UCSRA_CFG_MASK)) {
        config_changed(usart);
    }
    return 1;
}

static int ucsrb_write(HOOK_ARGS) {
    AvrUsart *usart = ctx;
    AvrCpu *cpu = usart->cpu;

    (void)addr;
    (void)mask;
    avr_update_interrupt_enable(cpu, &usart->RXC, value);
    avr_update_interrupt_enable(cpu, &usart->UDRE, value);
    avr_update_interrupt_enable(cpu, &usart->TXC, value);
    if ((value & UCSRB_RXEN) && (old_value & UCSRB_RXEN)) {
        avr_clear_interrupt(cpu, &usart->RXC, 1);
    }
    if ((value & UCSRB_TXEN) && !(old_value & UCSRB_TXEN)) {
        /* Enabling the transmitter: UDR is empty. */
        avr_set_interrupt_flag(cpu, &usart->UDRE);
    }
    cpu->data[usart->config.UCSRB] = value;
    if ((value & UCSRB_CFG_MASK) != (old_value & UCSRB_CFG_MASK)) {
        config_changed(usart);
    }
    return 1;
}

/* UCSRC, UBRRH and UBRRL: stored, and the configuration changed. */
static int config_write(HOOK_ARGS) {
    AvrUsart *usart = ctx;

    (void)old_value;
    (void)mask;
    usart->cpu->data[addr] = value;
    config_changed(usart);
    return 1;
}

static uint8_t udr_read(void *ctx, uint16_t addr) {
    AvrUsart *usart = ctx;
    int bits = avr_usart_bits(usart);
    uint8_t rx_mask = bits == 5 ? 0x1f : bits == 6 ? 0x3f : bits == 7 ? 0x7f : 0xff;
    uint8_t result = usart->rx_byte & rx_mask;

    (void)addr;
    usart->rx_byte = 0;
    avr_clear_interrupt(usart->cpu, &usart->RXC, 1);
    return result;
}

static void transmit_done(void *ctx) {
    AvrUsart *usart = ctx;

    avr_set_interrupt_flag(usart->cpu, &usart->UDRE);
    avr_set_interrupt_flag(usart->cpu, &usart->TXC);
}

static int udr_write(HOOK_ARGS) {
    AvrUsart *usart = ctx;

    (void)old_value;
    (void)addr;
    (void)mask;
    if (usart->on_transmit.fn != NULL) {
        usart->on_transmit.fn(usart->on_transmit.ctx, value);
    }
    avr_add_clock_event(usart->cpu, transmit_done, usart, avr_usart_char_cycles(usart));
    avr_clear_interrupt(usart->cpu, &usart->TXC, 1);
    avr_clear_interrupt(usart->cpu, &usart->UDRE, 1);
    return 0;   /* the core stores the byte, as after an avr8js hook that returns nothing */
}

static void receive_done(void *ctx) {
    AvrUsart *usart = ctx;

    usart->rx_busy = 0;
    avr_usart_write_byte(usart, usart->rx_pending, 1);
}

int avr_usart_write_byte(AvrUsart *usart, uint8_t value, int immediate) {
    if (usart->rx_busy || !avr_usart_rx_enabled(usart)) {
        return 0;
    }
    if (immediate) {
        usart->rx_byte = value;
        avr_set_interrupt_flag(usart->cpu, &usart->RXC);
        if (usart->on_rx_complete.fn != NULL) {
            usart->on_rx_complete.fn(usart->on_rx_complete.ctx);
        }
    } else {
        usart->rx_busy = 1;
        usart->rx_pending = value;
        avr_add_clock_event(usart->cpu, receive_done, usart, avr_usart_char_cycles(usart));
    }
    return 1;
}

void avr_usart_reset(AvrUsart *usart) {
    usart->cpu->data[usart->config.UCSRA] = UCSRA_UDRE;
    usart->cpu->data[usart->config.UCSRB] = 0;
    usart->cpu->data[usart->config.UCSRC] = UCSRC_UCSZ1 | UCSRC_UCSZ0;   /* 8 bits per character */
    usart->rx_busy = 0;
    usart->rx_byte = 0;
}

static void hook(AvrUsart *usart, uint16_t addr, AvrWriteFn fn) {
    if (addr < usart->cpu->data_size) {
        usart->cpu->write_hooks[addr].fn = fn;
        usart->cpu->write_hooks[addr].ctx = usart;
    }
}

void avr_usart_init(AvrUsart *usart, AvrCpu *cpu, const AvrUsartConfig *config, uint32_t freq_hz) {
    memset(usart, 0, sizeof(*usart));
    usart->cpu = cpu;
    usart->config = *config;
    usart->freq_hz = freq_hz;
    usart->RXC.address = config->rx_complete_interrupt;
    usart->RXC.flag_register = config->UCSRA;
    usart->RXC.flag_mask = UCSRA_RXC;
    usart->RXC.enable_register = config->UCSRB;
    usart->RXC.enable_mask = UCSRB_RXCIE;
    usart->RXC.constant = 1;
    usart->UDRE.address = config->data_register_empty_interrupt;
    usart->UDRE.flag_register = config->UCSRA;
    usart->UDRE.flag_mask = UCSRA_UDRE;
    usart->UDRE.enable_register = config->UCSRB;
    usart->UDRE.enable_mask = UCSRB_UDRIE;
    usart->TXC.address = config->tx_complete_interrupt;
    usart->TXC.flag_register = config->UCSRA;
    usart->TXC.flag_mask = UCSRA_TXC;
    usart->TXC.enable_register = config->UCSRB;
    usart->TXC.enable_mask = UCSRB_TXCIE;

    avr_usart_reset(usart);
    hook(usart, config->UCSRA, ucsra_write);
    hook(usart, config->UCSRB, ucsrb_write);
    hook(usart, config->UCSRC, config_write);
    if (config->UDR < cpu->data_size) {
        cpu->read_hooks[config->UDR].fn = udr_read;
        cpu->read_hooks[config->UDR].ctx = usart;
    }
    hook(usart, config->UDR, udr_write);
    hook(usart, config->UBRRH, config_write);
    hook(usart, config->UBRRL, config_write);
}
