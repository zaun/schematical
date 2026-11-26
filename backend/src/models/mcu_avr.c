/*
 * The engine adapter for AVR microcontrollers (plan-62-E): an AvrChip
 * (avr_chip.h, the C port of avr8js) on the event engine.
 *
 * The clock pins and the reset pin are the layout's (avr_chip.h): XTAL1
 * PB6, XTAL2 PB7 and RESET PC6 on the ATmegas; XTAL1 PB3, XTAL2 PB4 and
 * RESET PB5 on the ATtiny25/45/85.
 *
 * Startup. For AVR_STARTUP_PS after the circuit starts the chip is in reset:
 * every port pin Z. It counts rising edges on XTAL1.
 *
 * Clock, chosen once at the end of startup (a real chip chooses with fuses,
 * which do not exist here):
 *   - 2 or more rising edges on XTAL1 during startup: an external clock. One
 *     CPU cycle per rising edge of XTAL1 (per `prescaler` edges, CLKPR).
 *     XTAL1 and XTAL2 are clock pins: never driven, and their PIN bits read
 *     0. CLKPR resets to 0.
 *   - otherwise the internal 8 MHz oscillator divided by 8 (CLKPR resets to
 *     3, the CKDIV8 fuse): a CPU cycle is 125 ns x prescaler, 1 us until the
 *     firmware writes CLKPR. XTAL1 and XTAL2 are port pins.
 *
 * Reset. The reset pin is not a port pin: the chip pulls it up (weak 1);
 * while it reads 0 or X the chip is in reset, every port pin Z; when it
 * reads 1 again the chip restarts. A watchdog reset restarts it the same
 * way.
 *
 * Pins. An output drives strong 0 or 1; an input is Z, or weak 1 with its
 * pull-up. An input reads 1 for a 1 on its net and 0 for 0, X and Z. A pin
 * write lands at the time of the instruction that made it.
 *
 * ADC. The engine has no analog levels: an ADC input is 5 V when its net
 * reads 1 and 0 V otherwise, against AVCC = AREF = 5 V.
 *
 * Serial pins. The USART, SPI and TWI put their bytes on pins in
 * mcu_avr_serial.c, which takes a pin from its port with an override here.
 * The USI (ATtiny25/45/85) works on pins by itself (avr_usi.c): it is asked
 * what it does with each of its pins whenever one is updated, and it sees
 * the level of every pin through its port.
 */

#include <stdlib.h>
#include <string.h>

#include "mcu.h"
#include "logic_alloc.h"

#define AVR_STARTUP_PS 1000000000ull    /* 1 ms */
#define AVR_RC_CYCLE_PS 125000ull       /* the internal oscillator: 8 MHz */
#define AVR_RC_HZ 8000000u

#define TIMER_ID(KIND, GENERATION) ((uint32_t)(KIND) | ((uint32_t)(GENERATION) << 8))

/* ---- Pins ------------------------------------------------------------------ */

static void drive(McuAvr *mcu, size_t pin, LogicValue value, DriveStrength strength) {
    if (pin == MCU_NO_PIN || mcu->sim == NULL) {
        return;
    }
    if (mcu->driven[pin].value == (uint8_t)value && mcu->driven[pin].strength == (uint8_t)strength) {
        return;
    }
    mcu->driven[pin].value = (uint8_t)value;
    mcu->driven[pin].strength = (uint8_t)strength;
    sim_schedule_drive(mcu->sim, mcu->instance, pin, value, strength, 0);
}

/* 1 when the chip owns the pin as a port pin now (not the reset pin, not a
   clock pin of an external clock). */
static int is_port_pin(const McuAvr *mcu, size_t pin) {
    if (mcu->pin_map[pin].kind != MCU_PIN_PORT) {
        return 0;
    }
    return !(mcu->external_clock && (pin == mcu->xtal1 || pin == mcu->xtal2));
}

/* Puts on a port pin what the chip does with it now: a serial peripheral's
   override if it has the pin, else the port's state. */
static void update_pin(McuAvr *mcu, size_t pin) {
    const McuPin *map = &mcu->pin_map[pin];
    const AvrPort *port = &mcu->chip->ports[map->port];

    if (!is_port_pin(mcu, pin)) {
        return;
    }
    if (mcu->state != MCU_RUNNING) {
        drive(mcu, pin, LOGIC_Z, DRIVE_STRONG);
        return;
    }
    switch ((McuOverride)mcu->override[pin]) {
        case MCU_OVERRIDE_LOW:
            drive(mcu, pin, LOGIC_0, DRIVE_STRONG);
            return;
        case MCU_OVERRIDE_HIGH:
            drive(mcu, pin, LOGIC_1, DRIVE_STRONG);
            return;
        case MCU_OVERRIDE_RELEASE:
            if (mcu->chip->cpu->data[port->config.PORT] & (1 << map->bit)) {
                drive(mcu, pin, LOGIC_1, DRIVE_WEAK);   /* the pull-up */
            } else {
                drive(mcu, pin, LOGIC_Z, DRIVE_STRONG);
            }
            return;
        case MCU_OVERRIDE_NONE:
            break;
    }
    if (mcu->chip->layout.has_usi && port == mcu->chip->usi.port) {
        switch (avr_usi_pin(&mcu->chip->usi, map->bit)) {
            case AVR_USI_PIN_LOW:
                drive(mcu, pin, LOGIC_0, DRIVE_STRONG);
                return;
            case AVR_USI_PIN_HIGH:
                drive(mcu, pin, LOGIC_1, DRIVE_STRONG);
                return;
            case AVR_USI_PIN_FLOAT:
                drive(mcu, pin, LOGIC_Z, DRIVE_STRONG);
                return;
            case AVR_USI_PIN_PORT:
                break;
        }
    }
    switch (avr_port_pin_state(port, map->bit)) {
        case AVR_PIN_LOW: drive(mcu, pin, LOGIC_0, DRIVE_STRONG); break;
        case AVR_PIN_HIGH: drive(mcu, pin, LOGIC_1, DRIVE_STRONG); break;
        case AVR_PIN_INPUT: drive(mcu, pin, LOGIC_Z, DRIVE_STRONG); break;
        case AVR_PIN_INPUT_PULLUP: drive(mcu, pin, LOGIC_1, DRIVE_WEAK); break;
    }
}

/* Puts a port's state on its pins. */
static void update_port_pins(McuAvr *mcu, size_t port) {
    int bit;

    for (bit = 0; bit < 8; bit++) {
        size_t pin = mcu->pins->port[port][bit];

        if (pin != MCU_NO_PIN) {
            update_pin(mcu, pin);
        }
    }
}

size_t mcu_avr_pin(const McuAvr *mcu, AvrPinName name) {
    const AvrLayout *layout = mcu->pins->layout;
    size_t port;

    for (port = 0; port < layout->port_count; port++) {
        if (layout->ports[port].letter == name.port && name.bit < 8) {
            return mcu->pins->port[port][name.bit];
        }
    }
    return MCU_NO_PIN;
}

void mcu_avr_set_override(McuAvr *mcu, size_t pin, McuOverride override) {
    if (pin == MCU_NO_PIN || mcu->override[pin] == (uint8_t)override) {
        return;
    }
    mcu->override[pin] = (uint8_t)override;
    update_pin(mcu, pin);
}

int mcu_avr_pin_is_output(const McuAvr *mcu, size_t pin) {
    const McuPin *map;

    if (pin == MCU_NO_PIN) {
        return 0;
    }
    map = &mcu->pin_map[pin];
    return (mcu->chip->cpu->data[mcu->chip->ports[map->port].config.DDR] & (1 << map->bit)) != 0;
}

int mcu_avr_pin_high(const McuAvr *mcu, size_t pin) {
    return pin != MCU_NO_PIN && sim_read_pin(mcu->sim, mcu->instance, pin) == LOGIC_1;
}

static void update_all_pins(McuAvr *mcu) {
    size_t port;

    for (port = 0; port < mcu->chip->port_count; port++) {
        update_port_pins(mcu, port);
    }
}

/* The USI changed what it does with its pins. */
static void usi_pins(void *ctx) {
    McuAvr *mcu = ctx;
    const AvrUsi *usi = &mcu->chip->usi;
    size_t port;

    for (port = 0; port < mcu->chip->port_count; port++) {
        if (&mcu->chip->ports[port] == usi->port) {
            update_port_pins(mcu, port);
        }
    }
}

static void port_listener(void *ctx, uint8_t value, uint8_t old_value) {
    McuPortListener *listener = ctx;

    (void)value;
    (void)old_value;
    update_port_pins(listener->mcu, listener->port);
}

/* A pin's level into the chip: its port's PIN bit and its ADC channel. */
static void read_pin(McuAvr *mcu, size_t pin, LogicValue value) {
    const McuPin *map = &mcu->pin_map[pin];

    if (map->adc_channel >= 0) {
        mcu->chip->adc.channel_volts[map->adc_channel] = value == LOGIC_1 ? 5.0 : 0.0;
    }
    if (is_port_pin(mcu, pin)) {
        avr_port_set_pin(&mcu->chip->ports[map->port], map->bit, value == LOGIC_1);
    }
}

static void read_all_pins(McuAvr *mcu) {
    size_t pin;

    for (pin = 0; pin < mcu->part->pin_count; pin++) {
        if (mcu->pin_map[pin].kind == MCU_PIN_PORT || mcu->pin_map[pin].kind == MCU_PIN_ADC_ONLY) {
            read_pin(mcu, pin, sim_read_pin(mcu->sim, mcu->instance, pin));
        }
    }
}

/* ---- Running ------------------------------------------------------------------ */

SimTime mcu_avr_cycle_ps(const McuAvr *mcu) {
    SimTime base = mcu->external_clock ? mcu->period_ps : AVR_RC_CYCLE_PS;

    return base * (SimTime)avr_clock_prescaler(&mcu->chip->clock);
}

void mcu_avr_schedule(McuAvr *mcu, unsigned kind, SimTime delay) {
    sim_schedule_timer(mcu->sim, mcu->instance, TIMER_ID(kind, mcu->generation), delay);
}

/* One instruction. Returns the CPU cycles it took. */
static uint64_t step(McuAvr *mcu) {
    AvrCpu *cpu = mcu->chip->cpu;
    uint64_t before = cpu->cycles;
    uint32_t pc = cpu->pc;
    uint16_t word = avr_flash_word(cpu, pc);
    uint16_t next = avr_flash_word(cpu, (int64_t)pc + 1);
    char text[32];

    mcu->chip_was_reset = 0;
    avr_chip_step(mcu->chip);
    if (cpu->out_of_memory) {
        sim_fail(mcu->sim, "out of memory");
    }
    mcu->has_instruction = 1;
    mcu->instruction_pc = pc;
    mcu->instruction_words[0] = word;
    mcu->instruction_words[1] = next;
    mcu->instruction_length = avr_dasm(word, next, pc * 2, text, sizeof text);
    if (mcu->chip_was_reset || cpu->cycles <= before) {
        /* The watchdog reset the chip: its cycle counter started over. */
        return 1;
    }
    return cpu->cycles - before;
}

static void schedule_step(McuAvr *mcu, uint64_t cycles) {
    mcu_avr_schedule(mcu, MCU_TIMER_STEP, cycles * mcu_avr_cycle_ps(mcu));
}

/* The chip leaves reset: the first instruction runs one cycle from now. */
/* The serial peripherals let go of everything. */
static void forget_serial(McuAvr *mcu) {
    memset(mcu->override, MCU_OVERRIDE_NONE, mcu->part->pin_count);
    mcu_avr_serial_reset(mcu);
}

static void begin_run(McuAvr *mcu) {
    mcu->generation++;
    mcu->state = MCU_RUNNING;
    mcu->has_instruction = 0;
    forget_serial(mcu);
    avr_chip_reset(mcu->chip);
    read_all_pins(mcu);
    update_all_pins(mcu);
    if (mcu->external_clock) {
        mcu->edge_wait = avr_clock_prescaler(&mcu->chip->clock);
        mcu->due = mcu->clock_cycles + 1;
    } else {
        schedule_step(mcu, 1);
    }
}

static void enter_reset(McuAvr *mcu) {
    mcu->generation++;
    mcu->state = MCU_IN_RESET;
    forget_serial(mcu);
    avr_chip_reset(mcu->chip);
    update_all_pins(mcu);
}

/* 1 while the reset pin holds the chip in reset: it reads 0 or X. With no
   reset pin, or nothing on it but the chip's own pull-up, it reads 1. */
static int reset_held(const McuAvr *mcu) {
    LogicValue value;

    if (mcu->reset_pin == MCU_NO_PIN) {
        return 0;
    }
    value = sim_read_pin(mcu->sim, mcu->instance, mcu->reset_pin);
    return value == LOGIC_0 || value == LOGIC_X;
}

/* The watchdog reset the chip (avr_chip_reset already ran, inside a step). */
static void chip_reset_itself(void *ctx) {
    McuAvr *mcu = ctx;

    mcu->chip_was_reset = 1;
    /* Serial timers of before the reset are stale: a new generation. The
       step timer is rescheduled by its own handler, with the new one. */
    mcu->generation++;
    forget_serial(mcu);
    read_all_pins(mcu);
    update_all_pins(mcu);
    if (mcu->external_clock) {
        mcu->edge_wait = avr_clock_prescaler(&mcu->chip->clock);
    }
}

/* The end of startup: the clock is chosen, and the chip runs or stays in reset. */
static void end_startup(McuAvr *mcu) {
    uint32_t hz = AVR_RC_HZ;

    mcu->external_clock = mcu->xtal1 != MCU_NO_PIN && mcu->startup_edges >= 2 && mcu->period_ps > 0;
    if (mcu->external_clock) {
        hz = (uint32_t)((1000000000000ull + mcu->period_ps / 2) / mcu->period_ps);
    }
    mcu->chip->clock.reset_index = mcu->external_clock ? 0 : 3;
    avr_chip_set_base_hz(mcu->chip, hz);
    mcu->clock_cycles = 0;
    if (reset_held(mcu)) {
        enter_reset(mcu);
    } else {
        begin_run(mcu);
    }
}

/* An external clock's rising edge. */
static void clock_edge(McuAvr *mcu) {
    if (mcu->state != MCU_RUNNING) {
        return;
    }
    if (--mcu->edge_wait > 0) {
        return;
    }
    mcu->clock_cycles++;
    if (mcu->clock_cycles >= mcu->due) {
        mcu->due = mcu->clock_cycles + step(mcu);
    }
    mcu->edge_wait = avr_clock_prescaler(&mcu->chip->clock);
}

/* ---- Engine hooks ------------------------------------------------------------- */

void mcu_avr_start(McuAvr *mcu, Sim *sim, SimInstance *instance) {
    size_t pin;

    mcu->sim = sim;
    mcu->instance = instance;
    mcu->generation++;
    mcu->state = MCU_STARTUP;
    mcu->external_clock = 0;
    mcu->startup_edges = 0;
    mcu->period_ps = 0;
    mcu->last_edge = 0;
    for (pin = 0; pin < mcu->part->pin_count; pin++) {
        mcu->driven[pin].value = LOGIC_Z;
        mcu->driven[pin].strength = DRIVE_STRONG;
    }
    /* The reset pin's pull-up. */
    drive(mcu, mcu->reset_pin, LOGIC_1, DRIVE_WEAK);
    sim_schedule_timer(sim, instance, TIMER_ID(MCU_TIMER_STARTUP, mcu->generation), AVR_STARTUP_PS);
}

void mcu_avr_pin_changed(McuAvr *mcu, size_t pin, LogicValue old_value, LogicValue new_value) {
    if (pin == mcu->xtal1) {
        int rising = old_value == LOGIC_0 && new_value == LOGIC_1;

        if (mcu->state == MCU_STARTUP) {
            if (rising) {
                SimTime now = sim_now(mcu->sim);

                if (mcu->startup_edges > 0) {
                    mcu->period_ps = now - mcu->last_edge;
                }
                mcu->last_edge = now;
                mcu->startup_edges++;
            }
            return;
        }
        if (mcu->external_clock) {
            if (rising) {
                SimTime now = sim_now(mcu->sim);

                if (now > mcu->last_edge) {
                    mcu->period_ps = now - mcu->last_edge;
                }
                mcu->last_edge = now;
                clock_edge(mcu);
            }
            return;
        }
    }
    if (pin == mcu->reset_pin) {
        if (mcu->state == MCU_STARTUP) {
            return;
        }
        if (reset_held(mcu)) {
            if (mcu->state == MCU_RUNNING) {
                enter_reset(mcu);
            }
        } else if (mcu->state == MCU_IN_RESET) {
            begin_run(mcu);
        }
        return;
    }
    if (mcu->pin_map[pin].kind == MCU_PIN_PORT || mcu->pin_map[pin].kind == MCU_PIN_ADC_ONLY) {
        if (mcu->state == MCU_RUNNING) {
            read_pin(mcu, pin, new_value);
            mcu_avr_serial_pin(mcu, pin, old_value, new_value);
        }
    }
}

void mcu_avr_timer(McuAvr *mcu, uint32_t timer_id) {
    if ((timer_id >> 8) != (mcu->generation & 0xffffff)) {
        return;   /* from before a reset */
    }
    switch (timer_id & 0xff) {
        case MCU_TIMER_STARTUP:
            end_startup(mcu);
            break;
        case MCU_TIMER_STEP:
            if (mcu->state == MCU_RUNNING && !mcu->external_clock) {
                uint64_t cycles = step(mcu);

                /* After the step: a watchdog reset inside it starts a new
                   generation, and this timer must belong to it. */
                schedule_step(mcu, cycles);
            }
            break;
        default:
            if (mcu->state == MCU_RUNNING) {
                mcu_avr_serial_timer(mcu, timer_id & 0xff);
            }
            break;
    }
}

/* ---- Inspection ------------------------------------------------------------------ */

int mcu_avr_memory_size(const McuAvr *mcu, uint32_t *words, unsigned *bits) {
    *words = mcu->chip->cpu->flash_bytes;
    *bits = 8;
    return 1;
}

size_t mcu_avr_memory_read(const McuAvr *mcu, uint32_t start, size_t count, uint32_t *values,
                           uint32_t *known) {
    const AvrCpu *cpu = mcu->chip->cpu;
    size_t i;

    if (start >= cpu->flash_bytes) {
        return 0;
    }
    if (count > cpu->flash_bytes - start) {
        count = cpu->flash_bytes - start;
    }
    for (i = 0; i < count; i++) {
        if (values != NULL) {
            values[i] = cpu->flash[start + i];
        }
        if (known != NULL) {
            known[i] = 0xff;
        }
    }
    return count;
}

void mcu_avr_memory_load(McuAvr *mcu, uint32_t start, const uint32_t *values, size_t count) {
    size_t i;

    for (i = 0; i < count; i++) {
        mcu->chip->cpu->flash[start + i] = (uint8_t)values[i];
    }
}

int mcu_avr_cpu_state(const McuAvr *mcu, CpuState *out) {
    static const char *const R[32] = {
        "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9", "R10", "R11", "R12", "R13",
        "R14", "R15", "R16", "R17", "R18", "R19", "R20", "R21", "R22", "R23", "R24", "R25", "R26",
        "R27", "R28", "R29", "R30", "R31"};
    const AvrCpu *cpu = mcu->chip->cpu;
    size_t i, n = 0;

    memset(out, 0, sizeof(*out));
    if (mcu->state != MCU_RUNNING) {
        out->status = CPU_RESET;
        return 1;
    }
    out->status = CPU_RUNNING;
    out->cycles = cpu->cycles;
    out->registers[n++] = (CpuRegister){"PC", cpu->pc * 2, 16};
    out->registers[n++] = (CpuRegister){"SP", avr_get16(cpu, AVR_SP), 16};
    out->registers[n++] = (CpuRegister){"SREG", cpu->data[AVR_SREG], 8};
    for (i = 0; i < 32; i++) {
        out->registers[n++] = (CpuRegister){R[i], cpu->data[i], 8};
    }
    out->register_count = n;
    if (mcu->has_instruction) {
        unsigned word;

        out->has_instruction = 1;
        out->instruction_address = mcu->instruction_pc * 2;
        out->instruction_length = mcu->instruction_length * 2;
        for (word = 0; word < mcu->instruction_length; word++) {
            out->instruction_bytes[word * 2] = (uint8_t)mcu->instruction_words[word];
            out->instruction_bytes[word * 2 + 1] = (uint8_t)(mcu->instruction_words[word] >> 8);
        }
        avr_dasm(mcu->instruction_words[0], mcu->instruction_words[1], mcu->instruction_pc * 2,
                 out->instruction_text, sizeof out->instruction_text);
    }
    return 1;
}

/* ---- Creation ---------------------------------------------------------------------- */

McuAvr *mcu_avr_create(const Part *part, const McuAvrPins *pins) {
    McuAvr *mcu = logic_calloc(1, sizeof(*mcu));
    const AvrLayout *layout = pins->layout;
    size_t port, pin;
    int bit, channel;

    if (mcu == NULL) {
        return NULL;
    }
    mcu->part = part;
    mcu->pins = pins;
    mcu->chip = avr_chip_create(layout, part->mcu->flash, part->mcu->ram, part->mcu->eeprom,
                                AVR_RC_HZ, 3);
    mcu->pin_map = logic_calloc(part->pin_count > 0 ? part->pin_count : 1, sizeof(*mcu->pin_map));
    mcu->driven = logic_calloc(part->pin_count > 0 ? part->pin_count : 1, sizeof(*mcu->driven));
    mcu->override = logic_calloc(part->pin_count > 0 ? part->pin_count : 1, 1);
    if (mcu->chip == NULL || mcu->pin_map == NULL || mcu->driven == NULL || mcu->override == NULL) {
        mcu_avr_destroy(mcu);
        return NULL;
    }
    mcu->xtal1 = MCU_NO_PIN;
    mcu->xtal2 = MCU_NO_PIN;
    mcu->reset_pin = MCU_NO_PIN;
    for (pin = 0; pin < part->pin_count; pin++) {
        mcu->pin_map[pin].adc_channel = -1;
    }
    for (port = 0; port < layout->port_count; port++) {
        char letter = layout->ports[port].letter;

        for (bit = 0; bit < 8; bit++) {
            pin = pins->port[port][bit];
            if (pin == MCU_NO_PIN) {
                continue;
            }
            mcu->pin_map[pin].kind = MCU_PIN_PORT;
            mcu->pin_map[pin].port = (uint8_t)port;
            mcu->pin_map[pin].bit = (uint8_t)bit;
            /* The layout's clock pins, reset pin and ADC inputs. */
            if (letter == layout->xtal1.port && bit == layout->xtal1.bit) {
                mcu->xtal1 = pin;
            } else if (letter == layout->xtal2.port && bit == layout->xtal2.bit) {
                mcu->xtal2 = pin;
            } else if (letter == layout->reset.port && bit == layout->reset.bit) {
                mcu->reset_pin = pin;
                mcu->pin_map[pin].kind = MCU_PIN_RESET;
            }
            for (channel = 0; channel < AVR_LAYOUT_ADC_PINS; channel++) {
                if (letter == layout->adc_pins[channel].port && bit == layout->adc_pins[channel].bit &&
                    mcu->pin_map[pin].kind == MCU_PIN_PORT) {
                    mcu->pin_map[pin].adc_channel = (int8_t)channel;
                }
            }
        }
        mcu->listeners[port].mcu = mcu;
        mcu->listeners[port].port = port;
        mcu->chip->ports[port].listener.fn = port_listener;
        mcu->chip->ports[port].listener.ctx = &mcu->listeners[port];
    }
    if (pins->adc6 != MCU_NO_PIN) {
        mcu->pin_map[pins->adc6].kind = MCU_PIN_ADC_ONLY;
        mcu->pin_map[pins->adc6].adc_channel = 6;
    }
    if (pins->adc7 != MCU_NO_PIN) {
        mcu->pin_map[pins->adc7].kind = MCU_PIN_ADC_ONLY;
        mcu->pin_map[pins->adc7].adc_channel = 7;
    }
    mcu->chip->after_reset.fn = chip_reset_itself;
    mcu->chip->after_reset.ctx = mcu;
    if (mcu->chip->layout.has_usi) {
        mcu->chip->usi.on_pins.fn = usi_pins;
        mcu->chip->usi.on_pins.ctx = mcu;
    }
    mcu->state = MCU_STARTUP;
    mcu_avr_serial_init(mcu);
    return mcu;
}

void mcu_avr_destroy(McuAvr *mcu) {
    if (mcu == NULL) {
        return;
    }
    avr_chip_destroy(mcu->chip);
    logic_free(mcu->pin_map);
    logic_free(mcu->driven);
    logic_free(mcu->override);
    logic_free(mcu);
}
