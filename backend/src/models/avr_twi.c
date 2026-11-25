/*
 * The AVR TWI (I2C) master at register level: TWCR writes become start,
 * stop, address, write and read requests to a handler, which answers with
 * the avr_twi_complete_* calls. A C port of avr8js 0.21.1's
 * src/peripherals/twi.ts (Copyright (c) Uri Shaked and contributors, MIT;
 * see LICENSE-avr8js). plan-62-C.
 *
 * Master mode only, as in avr8js: no slave states, no arbitration.
 */

#include <string.h>

#include "avr.h"

enum {
    TWCR_TWINT = 0x80, TWCR_TWEA = 0x40, TWCR_TWSTA = 0x20, TWCR_TWSTO = 0x10, TWCR_TWEN = 0x04,
    TWCR_TWIE = 0x01,
    TWSR_TWS_MASK = 0xf8, TWSR_TWPS_MASK = 0x03,
    STATUS_TWI_IDLE = 0xf8,
    STATUS_START = 0x08, STATUS_REPEATED_START = 0x10,
    STATUS_SLAW_ACK = 0x18, STATUS_SLAW_NACK = 0x20,
    STATUS_DATA_SENT_ACK = 0x28, STATUS_DATA_SENT_NACK = 0x30,
    STATUS_SLAR_ACK = 0x40, STATUS_SLAR_NACK = 0x48,
    STATUS_DATA_RECEIVED_ACK = 0x50, STATUS_DATA_RECEIVED_NACK = 0x58
};

static uint8_t twi_status(const AvrTwi *twi) {
    return twi->cpu->data[twi->config.TWSR] & TWSR_TWS_MASK;
}

static void update_status(AvrTwi *twi, uint8_t value) {
    uint8_t *twsr = &twi->cpu->data[twi->config.TWSR];

    *twsr = (uint8_t)((*twsr & ~TWSR_TWS_MASK) | value);
    avr_set_interrupt_flag(twi->cpu, &twi->TWI);
}

int avr_twi_prescaler(const AvrTwi *twi) {
    switch (twi->cpu->data[twi->config.TWSR] & TWSR_TWPS_MASK) {
        case 0: return 1;
        case 1: return 4;
        case 2: return 16;
        default: return 64;
    }
}

uint32_t avr_twi_bit_cycles(const AvrTwi *twi) {
    return 16 + 2 * (uint32_t)twi->cpu->data[twi->config.TWBR] * (uint32_t)avr_twi_prescaler(twi);
}

void avr_twi_complete_start(AvrTwi *twi) {
    twi->busy = 0;
    update_status(twi, twi_status(twi) == STATUS_TWI_IDLE ? STATUS_START : STATUS_REPEATED_START);
}

void avr_twi_complete_stop(AvrTwi *twi) {
    twi->busy = 0;
    twi->cpu->data[twi->config.TWCR] &= (uint8_t)~TWCR_TWSTO;
    update_status(twi, STATUS_TWI_IDLE);
}

void avr_twi_complete_connect(AvrTwi *twi, int ack) {
    twi->busy = 0;
    if (twi->cpu->data[twi->config.TWDR] & 1) {
        update_status(twi, ack ? STATUS_SLAR_ACK : STATUS_SLAR_NACK);
    } else {
        update_status(twi, ack ? STATUS_SLAW_ACK : STATUS_SLAW_NACK);
    }
}

void avr_twi_complete_write(AvrTwi *twi, int ack) {
    twi->busy = 0;
    update_status(twi, ack ? STATUS_DATA_SENT_ACK : STATUS_DATA_SENT_NACK);
}

void avr_twi_complete_read(AvrTwi *twi, uint8_t value) {
    int ack = (twi->cpu->data[twi->config.TWCR] & TWCR_TWEA) != 0;

    twi->busy = 0;
    twi->cpu->data[twi->config.TWDR] = value;
    update_status(twi, ack ? STATUS_DATA_RECEIVED_ACK : STATUS_DATA_RECEIVED_NACK);
}

/* The clock event of a TWCR write: the oldest waiting request. */
static void request_event(void *ctx) {
    AvrTwi *twi = ctx;
    AvrTwiRequest request;
    const AvrTwiHandler *handler = &twi->handler;

    if (twi->request_count == 0) {
        return;
    }
    request = twi->requests[twi->request_head];
    twi->request_head = (twi->request_head + 1) % 8;
    twi->request_count--;

    if (request.value & TWCR_TWSTA) {
        twi->busy = 1;
        if (handler->start != NULL) {
            handler->start(handler->ctx, request.status != STATUS_TWI_IDLE);
        } else {
            avr_twi_complete_start(twi);
        }
    } else if (request.value & TWCR_TWSTO) {
        twi->busy = 1;
        if (handler->start != NULL) {
            handler->stop(handler->ctx);
        } else {
            avr_twi_complete_stop(twi);
        }
    } else if (request.status == STATUS_START || request.status == STATUS_REPEATED_START) {
        twi->busy = 1;
        if (handler->start != NULL) {
            handler->connect(handler->ctx, request.twdr >> 1, (request.twdr & 1) ? 0 : 1);
        } else {
            avr_twi_complete_connect(twi, 0);
        }
    } else if (request.status == STATUS_SLAW_ACK || request.status == STATUS_DATA_SENT_ACK) {
        twi->busy = 1;
        if (handler->start != NULL) {
            handler->write_byte(handler->ctx, request.twdr);
        } else {
            avr_twi_complete_write(twi, 0);
        }
    } else if (request.status == STATUS_SLAR_ACK || request.status == STATUS_DATA_RECEIVED_ACK) {
        twi->busy = 1;
        if (handler->start != NULL) {
            handler->read_byte(handler->ctx, (request.value & TWCR_TWEA) != 0);
        } else {
            avr_twi_complete_read(twi, 0xff);
        }
    }
}

static int twcr_write(void *ctx, uint8_t value, uint8_t old_value, uint16_t addr, uint8_t mask) {
    AvrTwi *twi = ctx;
    AvrCpu *cpu = twi->cpu;
    int clear_int = value & TWCR_TWINT;

    (void)old_value;
    (void)addr;
    (void)mask;
    cpu->data[twi->config.TWCR] = value;
    avr_clear_interrupt_by_flag(cpu, &twi->TWI, value);
    avr_update_interrupt_enable(cpu, &twi->TWI, value);
    if (clear_int && (value & TWCR_TWEN) && !twi->busy) {
        /* avr8js's closure captures the written value, the status and TWDR
           as they are now; the request runs in a clock event. Each event
           serves the oldest request: events of one cycle run newest first
           in avr8js, but a request is made at most once per instruction and
           served one cycle later, so the order is the same. */
        if (twi->request_count < 8) {
            AvrTwiRequest *request = &twi->requests[(twi->request_head + twi->request_count) % 8];

            request->value = value;
            request->status = twi_status(twi);
            request->twdr = cpu->data[twi->config.TWDR];
            twi->request_count++;
            avr_add_clock_event(cpu, request_event, twi, 0);
        }
        return 1;
    }
    return 0;
}

void avr_twi_reset(AvrTwi *twi) {
    twi->busy = 0;
    twi->request_head = 0;
    twi->request_count = 0;
    update_status(twi, STATUS_TWI_IDLE);
}

void avr_twi_init(AvrTwi *twi, AvrCpu *cpu, const AvrTwiConfig *config, uint32_t freq_hz) {
    memset(twi, 0, sizeof(*twi));
    twi->cpu = cpu;
    twi->config = *config;
    twi->freq_hz = freq_hz;
    twi->TWI.address = config->twi_interrupt;
    twi->TWI.flag_register = config->TWCR;
    twi->TWI.flag_mask = TWCR_TWINT;
    twi->TWI.enable_register = config->TWCR;
    twi->TWI.enable_mask = TWCR_TWIE;
    update_status(twi, STATUS_TWI_IDLE);
    if (config->TWCR < cpu->data_size) {
        cpu->write_hooks[config->TWCR].fn = twcr_write;
        cpu->write_hooks[config->TWCR].ctx = twi;
    }
}
