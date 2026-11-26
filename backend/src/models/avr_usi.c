/*
 * The Universal Serial Interface of the ATtiny25/45/85: a shift register
 * (USIDR), a 4-bit counter and a start detector, from which firmware makes
 * SPI (three-wire mode) and I2C (two-wire mode).
 *
 * This is NOT a port of avr8js's src/peripherals/usi.ts. That file reads
 * the clock source and the wire mode from the wrong bits of USICR
 * (`value & ((USICS1 | USICS0) >> 2)` is bits 1:0, USICLK and USITC), takes
 * its start and stop conditions from the port's output register instead of
 * the pins, and never samples on a clock edge. This one is written from
 * the datasheet (Atmel 2586, chapter 15) and works on pins: it watches the
 * levels the outside puts on its pins (AvrPort.pin_watch) and tells its
 * owner what it drives (avr_usi_pin, on_pins). It has no oracle; the
 * engine tests cover it (avr_tiny_test).
 *
 * Three-wire mode (USIWM = 01): DO shows the shift register's top bit when
 * its DDR bit is set; DI is sampled into the bottom bit. USCK is the port
 * pin.
 *
 * Two-wire mode (USIWM = 10, 11): SDA and SCL are open-drain and have no
 * pull-up. With its DDR bit set, SDA is low when the top bit or its PORT
 * bit is 0, and SCL is low when its PORT bit is 0 or the USI holds it: after
 * a start condition until USISIF is cleared, and in mode 11 also after a
 * counter overflow until USIOIF is cleared. The hold begins once SCL is low.
 *
 * Clock (USICS1:0, USICLK):
 *   00, USICLK written 1   one shift and one count per write
 *   01                     Timer0's compare match: not modeled, no clock
 *   10                     shift on the clock pin's rising edge, the output
 *                          changes on its falling edge
 *   11                     the other way round
 * With 10 and 11 the counter counts both edges of the pin, or, when USICLK
 * was written 1, each write of 1 to USITC. USITC toggles the clock pin's
 * PORT bit.
 */

#include <string.h>

#include "avr.h"

/* USICR bits. */
enum { USITC = 0x01, USICLK = 0x02, USICS0 = 0x04, USICS1 = 0x08, USIWM0 = 0x10, USIWM1 = 0x20,
       USIOIE = 0x40, USISIE = 0x80 };
/* USISR bits. */
enum { USICNT = 0x0f, USIDC = 0x10, USIPF = 0x20, USIOIF = 0x40, USISIF = 0x80 };

static int wire_mode(const AvrUsi *usi) {
    return (usi->cpu->data[usi->config.USICR] >> 4) & 3;
}

static int pin_level(const AvrUsi *usi, int bit) {
    return usi->port != NULL && (usi->port->pin_value & (1 << bit)) != 0;
}

static void pins_changed(AvrUsi *usi) {
    if (usi->on_pins.fn != NULL) {
        usi->on_pins.fn(usi->on_pins.ctx);
    }
}

/* The two-wire clock hold: wanted while a start or (mode 11) an overflow
   flag is set, and taken once SCL is low. */
static void update_hold(AvrUsi *usi) {
    uint8_t usisr = usi->cpu->data[usi->config.USISR];
    int mode = wire_mode(usi);
    int wanted = mode >= 2 && ((usisr & USISIF) || (mode == 3 && (usisr & USIOIF)));
    int hold = usi->scl_hold;

    if (!wanted) {
        hold = 0;
    } else if (!pin_level(usi, usi->config.clock_pin)) {
        hold = 1;
    }
    if (hold != usi->scl_hold) {
        usi->scl_hold = hold;
        pins_changed(usi);
    }
}

static void set_latch(AvrUsi *usi) {
    int bit = (usi->cpu->data[usi->config.USIDR] & 0x80) != 0;

    if (bit != usi->output_latch) {
        usi->output_latch = bit;
        pins_changed(usi);
    }
}

static void shift(AvrUsi *usi) {
    AvrCpu *cpu = usi->cpu;

    cpu->data[usi->config.USIDR] = (uint8_t)((cpu->data[usi->config.USIDR] << 1) |
                                             pin_level(usi, usi->config.data_in_pin));
    if (usi->buffer_follows) {
        /* The counter overflowed in the same instruction that made this
           clock edge (USITC): the buffer takes the byte with this bit. */
        cpu->data[usi->config.USIBR] = cpu->data[usi->config.USIDR];
        usi->buffer_follows = 0;
    }
}

/* One count. 1 when the counter overflowed. */
static int count(AvrUsi *usi) {
    AvrCpu *cpu = usi->cpu;
    uint8_t usisr = cpu->data[usi->config.USISR];
    uint8_t counter = (uint8_t)((usisr + 1) & USICNT);

    cpu->data[usi->config.USISR] = (uint8_t)((usisr & ~USICNT) | counter);
    if (counter != 0) {
        return 0;
    }
    cpu->data[usi->config.USIBR] = cpu->data[usi->config.USIDR];
    avr_set_interrupt_flag(cpu, &usi->OVF);
    update_hold(usi);
    return 1;
}

/* The level on the clock pin changed. */
static void clock_edge(void *ctx, int level) {
    AvrUsi *usi = ctx;
    uint8_t usicr = usi->cpu->data[usi->config.USICR];

    update_hold(usi);
    if (!(usicr & USICS1)) {
        return;
    }
    if (level == !(usicr & USICS0)) {
        shift(usi);
    } else {
        usi->buffer_follows = 0;
        set_latch(usi);
    }
    if (!usi->usiclk) {
        count(usi);
    }
}

/* The level on the data pin changed: in two-wire mode, with SCL high, a
   fall is a start condition and a rise is a stop condition. */
static void data_edge(void *ctx, int level) {
    AvrUsi *usi = ctx;
    AvrCpu *cpu = usi->cpu;

    if (wire_mode(usi) < 2 || !pin_level(usi, usi->config.clock_pin)) {
        return;
    }
    if (level) {
        cpu->data[usi->config.USISR] |= USIPF;
    } else {
        avr_set_interrupt_flag(cpu, &usi->START);
        update_hold(usi);
    }
}

static int usidr_write(void *ctx, uint8_t value, uint8_t old_value, uint16_t addr, uint8_t mask) {
    AvrUsi *usi = ctx;

    (void)old_value;
    (void)mask;
    usi->cpu->data[addr] = value;
    set_latch(usi);
    return 1;
}

static int usisr_write(void *ctx, uint8_t value, uint8_t old_value, uint16_t addr, uint8_t mask) {
    AvrUsi *usi = ctx;
    AvrCpu *cpu = usi->cpu;
    const uint8_t flags = USISIF | USIOIF | USIPF;   /* a 1 written clears the flag */

    (void)old_value;
    (void)mask;
    cpu->data[addr] = (uint8_t)((cpu->data[addr] & flags & ~value) | (value & USICNT));
    avr_clear_interrupt_by_flag(cpu, &usi->START, value);
    avr_clear_interrupt_by_flag(cpu, &usi->OVF, value);
    update_hold(usi);
    return 1;
}

/* USIDC: in two-wire mode, 1 while the shift register's top bit is 1 and
   the SDA pin is low (someone else holds it). */
static uint8_t usisr_read(void *ctx, uint16_t addr) {
    const AvrUsi *usi = ctx;
    uint8_t value = usi->cpu->data[addr];

    if (wire_mode(usi) >= 2 && usi->output_latch && !pin_level(usi, usi->config.data_in_pin)) {
        value |= USIDC;
    }
    return value;
}

static int usicr_write(void *ctx, uint8_t value, uint8_t old_value, uint16_t addr, uint8_t mask) {
    AvrUsi *usi = ctx;
    AvrCpu *cpu = usi->cpu;

    (void)old_value;
    (void)mask;
    cpu->data[addr] = (uint8_t)(value & ~(USICLK | USITC));   /* both read 0 */
    usi->usiclk = (value & USICLK) != 0;
    usi->buffer_follows = 0;
    avr_update_interrupt_enable(cpu, &usi->START, value);
    avr_update_interrupt_enable(cpu, &usi->OVF, value);
    if (!(value & (USICS1 | USICS0)) && (value & USICLK)) {
        /* The software clock strobe. */
        shift(usi);
        set_latch(usi);
        count(usi);
    }
    if (value & USITC) {
        uint16_t port = usi->config.port;

        if ((value & USICS1) && (value & USICLK)) {
            /* The pin's edge, and with it the shift, arrives after this
               instruction: an overflow now must not miss that bit. */
            usi->buffer_follows = count(usi);
        }
        avr_write_data(cpu, port, (uint8_t)(cpu->data[port] ^ (1 << usi->config.clock_pin)), 0xff);
    }
    update_hold(usi);
    pins_changed(usi);
    return 1;
}

AvrUsiPin avr_usi_pin(const AvrUsi *usi, int bit) {
    const AvrCpu *cpu = usi->cpu;
    int mode = wire_mode(usi);
    int output, port_bit;

    if (mode == 0 || usi->port == NULL) {
        return AVR_USI_PIN_PORT;
    }
    output = (cpu->data[usi->port->config.DDR] & (1 << bit)) != 0;
    port_bit = (cpu->data[usi->port->config.PORT] & (1 << bit)) != 0;
    if (mode == 1) {
        if (bit == usi->config.data_out_pin && output) {
            return usi->output_latch ? AVR_USI_PIN_HIGH : AVR_USI_PIN_LOW;
        }
        return AVR_USI_PIN_PORT;
    }
    if (bit == usi->config.data_in_pin) {
        return output && (!usi->output_latch || !port_bit) ? AVR_USI_PIN_LOW : AVR_USI_PIN_FLOAT;
    }
    if (bit == usi->config.clock_pin) {
        return output && (!port_bit || usi->scl_hold) ? AVR_USI_PIN_LOW : AVR_USI_PIN_FLOAT;
    }
    return AVR_USI_PIN_PORT;
}

static void set_interrupt(AvrInterrupt *interrupt, uint8_t address, uint16_t flag_register,
                          uint8_t flag_mask, uint16_t enable_register, uint8_t enable_mask) {
    memset(interrupt, 0, sizeof(*interrupt));
    interrupt->address = address;
    interrupt->flag_register = flag_register;
    interrupt->flag_mask = flag_mask;
    interrupt->enable_register = enable_register;
    interrupt->enable_mask = enable_mask;
    /* The flags stay set, and the interrupt pending, until the firmware
       writes 1 to them (datasheet 15.5.3). */
    interrupt->constant = 1;
}

void avr_usi_init(AvrUsi *usi, AvrCpu *cpu, const AvrUsiConfig *config) {
    memset(usi, 0, sizeof(*usi));
    usi->cpu = cpu;
    usi->config = *config;
    usi->port = avr_cpu_port_at(cpu, config->port);
    set_interrupt(&usi->START, config->start_interrupt, config->USISR, USISIF, config->USICR,
                  USISIE);
    set_interrupt(&usi->OVF, config->overflow_interrupt, config->USISR, USIOIF, config->USICR,
                  USIOIE);
    if (config->USIBR >= cpu->data_size || usi->port == NULL) {
        return;
    }
    cpu->write_hooks[config->USICR].fn = usicr_write;
    cpu->write_hooks[config->USICR].ctx = usi;
    cpu->write_hooks[config->USISR].fn = usisr_write;
    cpu->write_hooks[config->USISR].ctx = usi;
    cpu->read_hooks[config->USISR].fn = usisr_read;
    cpu->read_hooks[config->USISR].ctx = usi;
    cpu->write_hooks[config->USIDR].fn = usidr_write;
    cpu->write_hooks[config->USIDR].ctx = usi;
    usi->port->pin_watch[config->clock_pin].fn = clock_edge;
    usi->port->pin_watch[config->clock_pin].ctx = usi;
    usi->port->pin_watch[config->data_in_pin].fn = data_edge;
    usi->port->pin_watch[config->data_in_pin].ctx = usi;
}

void avr_usi_reset(AvrUsi *usi) {
    usi->output_latch = 0;
    usi->scl_hold = 0;
    usi->usiclk = 0;
    usi->buffer_follows = 0;
}
