/*
 * The AVR's serial peripherals on their pins (plan-62-F).
 *
 * avr8js (and so the C core, avr_usart.c, avr_spi.c, avr_twi.c) models the
 * USART, the SPI and the TWI as byte callbacks: a byte leaves, a byte
 * arrives, no pin moves. This file is the layer between those callbacks and
 * the engine's nets. It is ours, not a port, and avr8js cannot check it;
 * avr_serial_test runs it against library parts that speak the same buses.
 *
 * Timing is in engine time: one bit is the peripheral's bit time in CPU
 * cycles (avr_usart_bit_cycles, avr_spi_clock_divider, avr_twi_bit_cycles)
 * times the CPU cycle's length now (mcu_avr_cycle_ps). Every edge is a timer
 * of the adapter, so a reset cancels a transfer.
 *
 * USART. While TXEN is set TXD is the transmitter's: 1 when idle, and each
 * byte written to UDR goes out as a frame (start bit, 5 to 9 data bits LSB
 * first, the parity bit, 1 or 2 stop bits). While RXEN is set a 1-to-0
 * change on RXD starts a frame: the pin is sampled in the middle of every
 * bit and the byte delivered after a stop bit that reads 1. As in avr8js
 * there is no frame, parity or overrun error: a frame with a bad stop bit is
 * dropped and a received parity bit is not checked.
 *
 * SPI. Master only (with MSTR clear a transfer completes with 0 and no pin
 * moves). A byte written to SPDR is clocked out on MOSI and SCK and in from
 * MISO in the mode and bit order of SPCR. As on the chip, SCK and MOSI are
 * driven only where the user set their DDR bits, and SS is an ordinary port
 * pin.
 *
 * TWI. Master only. While TWEN is set SDA and SCL are open-drain lines: the
 * master pulls a line low or releases it (the bus needs pull-ups: resistors,
 * or the port's own, which a released pin still applies). A bit is four
 * quarters: SCL low for two, with SDA changing between them, and SCL high
 * for two, so SDA never moves next to an SCL edge (a change of SDA while
 * SCL is high is a start or a stop). After releasing SCL the master waits
 * until it reads 1, so a target may stretch the clock. No arbitration.
 */

#include <string.h>

#include "mcu.h"

/* ---- USART --------------------------------------------------------------- */

static void usart_start_frame(McuUsartLine *line);

static AvrUsart *usart_of(McuUsartLine *line) {
    return &line->mcu->chip->usarts[line->index];
}

/* TXD when no frame is going out: the transmitter's 1, or the port's pin. */
static void usart_idle(McuUsartLine *line) {
    mcu_avr_set_override(line->mcu, line->txd,
                         avr_usart_tx_enabled(usart_of(line)) ? MCU_OVERRIDE_HIGH : MCU_OVERRIDE_NONE);
}

static void usart_put(McuUsartLine *line, int level) {
    mcu_avr_set_override(line->mcu, line->txd, level ? MCU_OVERRIDE_HIGH : MCU_OVERRIDE_LOW);
}

static void usart_start_frame(McuUsartLine *line) {
    AvrUsart *usart = usart_of(line);
    uint16_t data = line->tx_queue[line->tx_head];
    int bits = avr_usart_bits(usart);
    int parity = avr_usart_parity(usart);
    int ones = 0, i;
    unsigned n = 0;

    line->tx_head = (line->tx_head + 1) % MCU_TX_QUEUE;
    line->tx_count--;
    line->tx_levels[n++] = 0;                       /* the start bit */
    for (i = 0; i < bits; i++) {
        int bit = (data >> i) & 1;

        line->tx_levels[n++] = (uint8_t)bit;
        ones += bit;
    }
    if (parity != 0) {
        /* Even parity makes the count of ones even; odd makes it odd. */
        line->tx_levels[n++] = (uint8_t)((ones & 1) ^ (parity == 2));
    }
    for (i = 0; i < avr_usart_stop_bits(usart); i++) {
        line->tx_levels[n++] = 1;
    }
    line->tx_length = n;
    line->tx_position = 0;
    line->tx_bit_ps = (SimTime)avr_usart_bit_cycles(usart) * mcu_avr_cycle_ps(line->mcu);
    line->tx_active = 1;
    usart_put(line, 0);
    mcu_avr_schedule(line->mcu, MCU_TIMER_USART_TX + line->index, line->tx_bit_ps);
}

/* The end of a bit of the frame going out. A frame is going out whenever
   this timer is pending: only a serial reset ends one early, and every
   reset starts a new timer generation (mcu_avr.c), so the timer is dropped. */
static void usart_tx_timer(McuUsartLine *line) {
    if (++line->tx_position < line->tx_length) {
        usart_put(line, line->tx_levels[line->tx_position]);
        mcu_avr_schedule(line->mcu, MCU_TIMER_USART_TX + line->index, line->tx_bit_ps);
        return;
    }
    line->tx_active = 0;
    if (line->tx_count > 0 && avr_usart_tx_enabled(usart_of(line))) {
        usart_start_frame(line);
    } else {
        line->tx_count = 0;
        usart_idle(line);
    }
}

/* A byte was written to UDR. */
static void usart_transmit(void *ctx, uint8_t value) {
    McuUsartLine *line = ctx;
    AvrUsart *usart = usart_of(line);

    if (line->mcu->state != MCU_RUNNING || !avr_usart_tx_enabled(usart) ||
        line->tx_count == MCU_TX_QUEUE) {
        return;
    }
    line->tx_queue[(line->tx_head + line->tx_count) % MCU_TX_QUEUE] =
        (uint16_t)(value | ((usart->cpu->data[usart->config.UCSRB] & 1) << 8));   /* TXB8 */
    line->tx_count++;
    if (!line->tx_active) {
        usart_start_frame(line);
    }
}

/* UCSRnA/B/C or UBRR changed: TXEN may have. A frame going out finishes first. */
static void usart_config(void *ctx) {
    McuUsartLine *line = ctx;

    if (line->mcu->state == MCU_RUNNING && !line->tx_active) {
        usart_idle(line);
    }
}

static void usart_pin(McuUsartLine *line, LogicValue old_value, LogicValue new_value) {
    AvrUsart *usart = usart_of(line);

    if (line->rx_active || !avr_usart_rx_enabled(usart) || old_value != LOGIC_1 ||
        new_value != LOGIC_0) {
        return;
    }
    line->rx_active = 1;
    line->rx_position = 0;
    line->rx_shift = 0;
    line->rx_data_bits = (unsigned)avr_usart_bits(usart);
    line->rx_frame_bits = 1 + line->rx_data_bits + (avr_usart_parity(usart) ? 1 : 0) + 1;
    line->rx_bit_ps = (SimTime)avr_usart_bit_cycles(usart) * mcu_avr_cycle_ps(line->mcu);
    mcu_avr_schedule(line->mcu, MCU_TIMER_USART_RX + line->index, line->rx_bit_ps / 2);
}

/* The middle of a bit of the frame coming in. rx_active is set while this
   timer is pending (as for the transmitter: a reset drops the timer). */
static void usart_rx_timer(McuUsartLine *line) {
    int level;

    level = mcu_avr_pin_high(line->mcu, line->rxd);
    if (line->rx_position == 0) {
        if (level) {
            line->rx_active = 0;   /* a glitch, not a start bit */
            return;
        }
    } else if (line->rx_position <= line->rx_data_bits) {
        line->rx_shift |= (uint16_t)(level << (line->rx_position - 1));
    } else if (line->rx_position == line->rx_frame_bits - 1) {
        line->rx_active = 0;
        if (level) {
            /* A good stop bit. avr8js's receiver holds 8 bits. */
            avr_usart_write_byte(usart_of(line), (uint8_t)line->rx_shift, 1);
        }
        return;
    }
    line->rx_position++;
    mcu_avr_schedule(line->mcu, MCU_TIMER_USART_RX + line->index, line->rx_bit_ps);
}

/* ---- SPI ------------------------------------------------------------------- */

enum { SPCR_SPE = 0x40, SPCR_MSTR = 0x10, SPCR_CPOL = 0x08 };

static AvrSpi *spi_of(McuSpiLine *line) {
    return &line->mcu->chip->spis[line->index];
}

/* As on the chip, the master drives SCK and MOSI only where their DDR bit
   is set. */
static void spi_put(McuSpiLine *line, size_t pin, int level) {
    if (mcu_avr_pin_is_output(line->mcu, pin)) {
        mcu_avr_set_override(line->mcu, pin, level ? MCU_OVERRIDE_HIGH : MCU_OVERRIDE_LOW);
    } else {
        mcu_avr_set_override(line->mcu, pin, MCU_OVERRIDE_NONE);
    }
}

static int spi_out_bit(const McuSpiLine *line, unsigned index) {
    return (line->out >> (line->lsb_first ? index : 7 - index)) & 1;
}

static void spi_sample(McuSpiLine *line, unsigned index) {
    if (mcu_avr_pin_high(line->mcu, line->miso)) {
        line->in |= (uint8_t)(1 << (line->lsb_first ? index : 7 - index));
    }
}

/* With MSTR clear nothing is modeled: the transfer ends with 0. */
static void spi_slave_complete(void *ctx) {
    McuSpiLine *line = ctx;

    avr_spi_complete_transfer(spi_of(line), 0);
}

/* A byte was written to SPDR with the SPI enabled. */
static void spi_byte(void *ctx, uint8_t value) {
    McuSpiLine *line = ctx;
    AvrSpi *spi = spi_of(line);
    int mode = avr_spi_mode(spi);

    if (!avr_spi_is_master(spi) || line->mcu->state != MCU_RUNNING) {
        avr_add_clock_event(spi->cpu, spi_slave_complete, line, avr_spi_transfer_cycles(spi));
        return;
    }
    line->active = 1;
    line->out = value;
    line->in = 0;
    line->edge = 0;
    line->cpol = mode & 1;
    line->cpha = (mode >> 1) & 1;
    line->lsb_first = avr_spi_lsb_first(spi);
    line->half_ps = (SimTime)avr_spi_clock_divider(spi) * mcu_avr_cycle_ps(line->mcu) / 2;
    spi_put(line, line->sck, line->cpol);
    if (!line->cpha) {
        spi_put(line, line->mosi, spi_out_bit(line, 0));
    }
    mcu_avr_schedule(line->mcu, MCU_TIMER_SPI + line->index, line->half_ps);
}

/* A clock edge: 16 per byte, the odd ones leading. */
static void spi_timer(McuSpiLine *line) {
    unsigned index;
    int leading;

    if (!line->active) {
        return;
    }
    line->edge++;
    index = (line->edge - 1) / 2;
    leading = line->edge & 1;
    spi_put(line, line->sck, leading ? !line->cpol : line->cpol);
    if (leading != line->cpha) {
        /* The sampling edge: leading with CPHA 0, trailing with CPHA 1. */
        spi_sample(line, index);
    } else if (line->cpha) {
        spi_put(line, line->mosi, spi_out_bit(line, index));        /* set up on the leading edge */
    } else if (index < 7) {
        spi_put(line, line->mosi, spi_out_bit(line, index + 1));    /* ... on the trailing edge */
    }
    if (line->edge == 16) {
        line->active = 0;
        avr_spi_complete_transfer(spi_of(line), line->in);
        return;
    }
    mcu_avr_schedule(line->mcu, MCU_TIMER_SPI + line->index, line->half_ps);
}

/* A write to SPCR, after the peripheral's own hook: SCK idles at CPOL while
   the master is enabled, and the pins go back to the port when it is not.
   Only running firmware writes SPCR (a reset clears it without the hook). */
static int spi_spcr_write(void *ctx, uint8_t value, uint8_t old_value, uint16_t addr, uint8_t mask) {
    McuSpiLine *line = ctx;
    int handled = line->spcr_hook.fn != NULL
        ? line->spcr_hook.fn(line->spcr_hook.ctx, value, old_value, addr, mask) : 0;

    if ((value & SPCR_SPE) && (value & SPCR_MSTR)) {
        if (!line->active) {
            spi_put(line, line->sck, (value & SPCR_CPOL) != 0);
        }
    } else {
        line->active = 0;
        mcu_avr_set_override(line->mcu, line->sck, MCU_OVERRIDE_NONE);
        mcu_avr_set_override(line->mcu, line->mosi, MCU_OVERRIDE_NONE);
    }
    return handled;
}

/* ---- TWI -------------------------------------------------------------------- */

enum { TWCR_TWEN = 0x04 };

/* The steps of a bus operation. */
enum {
    T_SDA,        /* arg 1: release SDA, 0: pull it low */
    T_SCL,        /* the same for SCL */
    T_WAIT,       /* arg quarters of a bit */
    T_WAIT_SCL,   /* until SCL reads 1 (a target may hold it low) */
    T_SAMPLE_BIT, /* SDA into the byte being read, MSB first */
    T_SAMPLE_ACK, /* SDA low is an acknowledge */
    T_DONE        /* arg: which avr_twi_complete_* */
};
enum { DONE_START, DONE_STOP, DONE_CONNECT, DONE_WRITE, DONE_READ };

static AvrTwi *twi_of(McuTwiLine *line) {
    return &line->mcu->chip->twis[line->index];
}

static void twi_add(McuTwiLine *line, uint8_t kind, uint8_t arg) {
    if (line->count < MCU_TWI_STEPS) {
        line->steps[line->count].kind = kind;
        line->steps[line->count].arg = arg;
        line->count++;
    }
}

/* One bit with SCL low on entry and on exit: a quarter, SDA, a quarter, SCL
   high for two quarters (sampling SDA at their end when asked), SCL low. */
static void twi_add_bit(McuTwiLine *line, int sda, int sample) {
    twi_add(line, T_WAIT, 1);
    twi_add(line, T_SDA, (uint8_t)sda);
    twi_add(line, T_WAIT, 1);
    twi_add(line, T_SCL, 1);
    twi_add(line, T_WAIT_SCL, 0);
    twi_add(line, T_WAIT, 2);
    if (sample >= 0) {
        twi_add(line, (uint8_t)sample, 0);
    }
    twi_add(line, T_SCL, 0);
}

static void twi_run(McuTwiLine *line) {
    AvrTwi *twi = twi_of(line);
    McuAvr *mcu = line->mcu;

    while (line->position < line->count) {
        McuTwiStep step = line->steps[line->position++];

        switch (step.kind) {
            case T_SDA:
                mcu_avr_set_override(mcu, line->sda, step.arg ? MCU_OVERRIDE_RELEASE : MCU_OVERRIDE_LOW);
                break;
            case T_SCL:
                mcu_avr_set_override(mcu, line->scl, step.arg ? MCU_OVERRIDE_RELEASE : MCU_OVERRIDE_LOW);
                break;
            case T_WAIT:
                mcu_avr_schedule(mcu, MCU_TIMER_TWI + line->index, line->quarter_ps * step.arg);
                return;
            case T_WAIT_SCL:
                /* The release takes effect in the engine after this call; the
                   pin change that brings SCL to 1 resumes the operation. */
                line->waiting_scl = 1;
                return;
            case T_SAMPLE_BIT:
                line->shift = (uint8_t)((line->shift << 1) | mcu_avr_pin_high(mcu, line->sda));
                break;
            case T_SAMPLE_ACK:
                line->ack = sim_read_pin(mcu->sim, mcu->instance, line->sda) == LOGIC_0;
                break;
            case T_DONE:
                line->count = 0;
                line->position = 0;
                switch (step.arg) {
                    case DONE_START: avr_twi_complete_start(twi); break;
                    case DONE_STOP: avr_twi_complete_stop(twi); break;
                    case DONE_CONNECT: avr_twi_complete_connect(twi, line->ack); break;
                    case DONE_WRITE: avr_twi_complete_write(twi, line->ack); break;
                    case DONE_READ: avr_twi_complete_read(twi, line->shift); break;
                }
                return;
        }
    }
}

/* Starts the operation whose steps were just added. */
static void twi_begin(McuTwiLine *line) {
    line->position = 0;
    line->waiting_scl = 0;
    line->quarter_ps = (SimTime)avr_twi_bit_cycles(twi_of(line)) * mcu_avr_cycle_ps(line->mcu) / 4;
    twi_run(line);
}

static void twi_start(void *ctx, int repeated) {
    McuTwiLine *line = ctx;

    line->count = 0;
    if (repeated) {
        /* SCL is low: let SDA go high, then SCL, before the start itself. */
        twi_add(line, T_WAIT, 1);
        twi_add(line, T_SDA, 1);
        twi_add(line, T_WAIT, 1);
        twi_add(line, T_SCL, 1);
        twi_add(line, T_WAIT_SCL, 0);
        twi_add(line, T_WAIT, 2);
    }
    twi_add(line, T_SDA, 0);      /* SDA falls while SCL is high */
    twi_add(line, T_WAIT, 2);
    twi_add(line, T_SCL, 0);
    twi_add(line, T_DONE, DONE_START);
    twi_begin(line);
}

static void twi_stop(void *ctx) {
    McuTwiLine *line = ctx;

    line->count = 0;
    twi_add(line, T_WAIT, 1);
    twi_add(line, T_SDA, 0);
    twi_add(line, T_WAIT, 1);
    twi_add(line, T_SCL, 1);
    twi_add(line, T_WAIT_SCL, 0);
    twi_add(line, T_WAIT, 2);
    twi_add(line, T_SDA, 1);      /* SDA rises while SCL is high */
    twi_add(line, T_WAIT, 2);
    twi_add(line, T_DONE, DONE_STOP);
    twi_begin(line);
}

/* Eight bits out, MSB first, then the target's acknowledge. */
static void twi_send(McuTwiLine *line, uint8_t value, uint8_t done) {
    int bit;

    line->count = 0;
    for (bit = 7; bit >= 0; bit--) {
        twi_add_bit(line, (value >> bit) & 1, -1);
    }
    twi_add_bit(line, 1, T_SAMPLE_ACK);
    twi_add(line, T_DONE, done);
    twi_begin(line);
}

static void twi_connect(void *ctx, uint8_t address, int write) {
    twi_send(ctx, (uint8_t)((address << 1) | (write ? 0 : 1)), DONE_CONNECT);
}

static void twi_write(void *ctx, uint8_t value) {
    twi_send(ctx, value, DONE_WRITE);
}

/* Eight bits in, then the master's acknowledge (or none). */
static void twi_read(void *ctx, int ack) {
    McuTwiLine *line = ctx;
    int bit;

    line->count = 0;
    line->shift = 0;
    for (bit = 0; bit < 8; bit++) {
        twi_add_bit(line, 1, T_SAMPLE_BIT);
    }
    twi_add_bit(line, ack ? 0 : 1, -1);
    twi_add(line, T_WAIT, 1);
    twi_add(line, T_SDA, 1);
    twi_add(line, T_DONE, DONE_READ);
    twi_begin(line);
}

/* A write to TWCR, after the peripheral's own hook: TWEN gives the two pins
   to the TWI or back to the port. */
static int twi_twcr_write(void *ctx, uint8_t value, uint8_t old_value, uint16_t addr, uint8_t mask) {
    McuTwiLine *line = ctx;
    int handled = line->twcr_hook.fn != NULL
        ? line->twcr_hook.fn(line->twcr_hook.ctx, value, old_value, addr, mask) : 0;
    int enabled = (value & TWCR_TWEN) != 0;

    if (line->mcu->state != MCU_RUNNING || enabled == line->enabled) {
        return handled;
    }
    line->enabled = enabled;
    if (enabled) {
        mcu_avr_set_override(line->mcu, line->sda, MCU_OVERRIDE_RELEASE);
        mcu_avr_set_override(line->mcu, line->scl, MCU_OVERRIDE_RELEASE);
    } else {
        /* An operation in progress is abandoned with the bus. */
        if (line->count > 0) {
            line->count = 0;
            line->position = 0;
            line->waiting_scl = 0;
            twi_of(line)->busy = 0;
        }
        mcu_avr_set_override(line->mcu, line->sda, MCU_OVERRIDE_NONE);
        mcu_avr_set_override(line->mcu, line->scl, MCU_OVERRIDE_NONE);
    }
    return handled;
}

/* ---- The adapter's calls ------------------------------------------------------ */

void mcu_avr_serial_init(McuAvr *mcu) {
    const AvrLayout *layout = mcu->pins->layout;
    AvrChip *chip = mcu->chip;
    AvrCpu *cpu = chip->cpu;
    size_t i;

    for (i = 0; i < chip->usart_count; i++) {
        McuUsartLine *line = &mcu->usarts[i];

        line->mcu = mcu;
        line->index = (unsigned)i;
        line->rxd = mcu_avr_pin(mcu, layout->usart_pins[i].rxd);
        line->txd = mcu_avr_pin(mcu, layout->usart_pins[i].txd);
        chip->usarts[i].on_transmit.fn = usart_transmit;
        chip->usarts[i].on_transmit.ctx = line;
        chip->usarts[i].on_config.fn = usart_config;
        chip->usarts[i].on_config.ctx = line;
    }
    for (i = 0; i < chip->spi_count; i++) {
        McuSpiLine *line = &mcu->spis[i];
        uint16_t spcr = chip->spis[i].config.SPCR;

        line->mcu = mcu;
        line->index = (unsigned)i;
        line->mosi = mcu_avr_pin(mcu, layout->spi_pins[i].mosi);
        line->miso = mcu_avr_pin(mcu, layout->spi_pins[i].miso);
        line->sck = mcu_avr_pin(mcu, layout->spi_pins[i].sck);
        chip->spis[i].on_byte.fn = spi_byte;
        chip->spis[i].on_byte.ctx = line;
        if (spcr < cpu->data_size) {
            line->spcr_hook = cpu->write_hooks[spcr];
            cpu->write_hooks[spcr].fn = spi_spcr_write;
            cpu->write_hooks[spcr].ctx = line;
        }
    }
    for (i = 0; i < chip->twi_count; i++) {
        McuTwiLine *line = &mcu->twis[i];
        uint16_t twcr = chip->twis[i].config.TWCR;

        line->mcu = mcu;
        line->index = (unsigned)i;
        line->sda = mcu_avr_pin(mcu, layout->twi_pins[i].sda);
        line->scl = mcu_avr_pin(mcu, layout->twi_pins[i].scl);
        chip->twis[i].handler.start = twi_start;
        chip->twis[i].handler.stop = twi_stop;
        chip->twis[i].handler.connect = twi_connect;
        chip->twis[i].handler.write_byte = twi_write;
        chip->twis[i].handler.read_byte = twi_read;
        chip->twis[i].handler.ctx = line;
        if (twcr < cpu->data_size) {
            line->twcr_hook = cpu->write_hooks[twcr];
            cpu->write_hooks[twcr].fn = twi_twcr_write;
            cpu->write_hooks[twcr].ctx = line;
        }
    }
}

void mcu_avr_serial_reset(McuAvr *mcu) {
    size_t i;

    for (i = 0; i < AVR_LAYOUT_SERIALS; i++) {
        McuUsartLine *usart = &mcu->usarts[i];
        McuSpiLine *spi = &mcu->spis[i];
        McuTwiLine *twi = &mcu->twis[i];

        usart->tx_head = 0;
        usart->tx_count = 0;
        usart->tx_active = 0;
        usart->rx_active = 0;
        spi->active = 0;
        twi->enabled = 0;
        twi->count = 0;
        twi->position = 0;
        twi->waiting_scl = 0;
    }
}

void mcu_avr_serial_pin(McuAvr *mcu, size_t pin, LogicValue old_value, LogicValue new_value) {
    size_t i;

    for (i = 0; i < mcu->chip->usart_count; i++) {
        if (pin == mcu->usarts[i].rxd) {
            usart_pin(&mcu->usarts[i], old_value, new_value);
        }
    }
    for (i = 0; i < mcu->chip->twi_count; i++) {
        McuTwiLine *line = &mcu->twis[i];

        if (pin == line->scl && line->waiting_scl && new_value == LOGIC_1) {
            line->waiting_scl = 0;
            twi_run(line);
        }
    }
}

/* kind is one this file scheduled: MCU_TIMER_* + a line index below
   AVR_LAYOUT_SERIALS. */
void mcu_avr_serial_timer(McuAvr *mcu, unsigned kind) {
    unsigned index = kind & 0x0f;

    switch (kind & 0xf0) {
        case MCU_TIMER_USART_TX: usart_tx_timer(&mcu->usarts[index]); break;
        case MCU_TIMER_USART_RX: usart_rx_timer(&mcu->usarts[index]); break;
        case MCU_TIMER_SPI: spi_timer(&mcu->spis[index]); break;
        case MCU_TIMER_TWI: twi_run(&mcu->twis[index]); break;
    }
}
