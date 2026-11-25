#ifndef AVR_H
#define AVR_H

/*
 * The AVR core (plan-62): a C port of avr8js 0.21.1 by Uri Shaked and
 * contributors (MIT, see LICENSE-avr8js in this folder). This header
 * declares the CPU (avr_cpu.c ports avr8js's src/cpu/cpu.ts and
 * interrupt.ts, avr_instruction.c ports src/cpu/instruction.ts).
 *
 * The port keeps avr8js's structure and names so the two can be read side
 * by side, and the tests compare them state for state (avr_core_test,
 * avr_periph_test, with avr8js run under Node as the oracle).
 *
 * Where avr8js disagrees with Microchip's AVR Instruction Set Manual the C
 * code follows the manual; each such place has a comment. `exact` (below)
 * switches those places back to avr8js's behavior for the oracle tests.
 */

#include <stddef.h>
#include <stdint.h>

typedef struct AvrCpu AvrCpu;

/* A read hook: the value a read of data address `addr` gives. */
typedef uint8_t (*AvrReadFn)(void *ctx, uint16_t addr);
/* A write hook. `old_value` is the byte in memory, `mask` the bits the
   instruction addresses (SBI and CBI name one bit; 0xff otherwise). Returns
   1 when it handled the write: the core then stores nothing. */
typedef int (*AvrWriteFn)(void *ctx, uint8_t value, uint8_t old_value, uint16_t addr,
                          uint8_t mask);
typedef void (*AvrEventFn)(void *ctx);

typedef struct { AvrReadFn fn; void *ctx; } AvrReadHook;
typedef struct { AvrWriteFn fn; void *ctx; } AvrWriteHook;
typedef struct { AvrEventFn fn; void *ctx; } AvrCallback;

/* avr8js's AVRInterruptConfig. `address` is the vector's word address. */
typedef struct AvrInterrupt {
    uint8_t address;
    uint16_t enable_register;
    uint8_t enable_mask;
    uint16_t flag_register;
    uint8_t flag_mask;
    int constant;       /* stays pending after it is taken (a level interrupt) */
    int inverse_flag;   /* the flag is set when its bit is 0 */
} AvrInterrupt;

enum {
    AVR_MAX_INTERRUPTS = 128,   /* avr8js's MAX_INTERRUPTS */
    AVR_REGISTER_SPACE = 0x100, /* registers and I/O below the SRAM */
    AVR_SP = 93,                /* SPL; SPH is 94 */
    AVR_SREG = 95,
    AVR_EVENT_POOL = 10,        /* avr8js keeps at most 10 spare event nodes */
    AVR_BRANCHES = 99           /* decode branches of avr_instruction */
};

typedef struct AvrClockEvent {
    double cycles;              /* a JavaScript number in avr8js: may be fractional */
    AvrEventFn fn;
    void *ctx;
    struct AvrClockEvent *next;
} AvrClockEvent;

struct AvrCpu {
    uint8_t *data;              /* registers 0-31, I/O from 32, SRAM from 0x100 */
    uint32_t data_size;
    uint8_t *flash;             /* program memory, bytes; a word is little-endian */
    uint32_t flash_bytes;
    uint32_t flash_words;
    int pc22;                   /* the program counter has 22 bits (flash over 128 KB) */

    uint32_t pc;                /* word index */
    uint64_t cycles;

    AvrReadHook *read_hooks;    /* data_size entries */
    AvrWriteHook *write_hooks;

    const AvrInterrupt *pending[AVR_MAX_INTERRUPTS];
    int next_interrupt;         /* lowest pending address, or -1 */
    int max_interrupt;

    AvrClockEvent *next_event;
    AvrClockEvent *event_pool[AVR_EVENT_POOL];
    size_t event_pool_count;
    AvrClockEvent *running_event;   /* the event whose callback is running, or NULL */
    int reset_in_event;             /* avr_cpu_reset ran inside that callback */
    AvrClockEvent *spare_events;    /* nodes avr8js would have left to the garbage collector */

    AvrCallback watchdog_reset;     /* the WDR instruction */

    struct AvrPort *ports[12];      /* avr8js's gpioPorts, in the order they were made */
    size_t port_count;

    /* 0 (the simulator): the manual's behavior where avr8js differs from it.
       1 (the oracle tests): avr8js's behavior everywhere. */
    int exact;

    /* Tests only: when not NULL, avr_instruction counts each decode branch
       it takes in coverage[1..AVR_BRANCHES]. */
    uint32_t *coverage;
};

/* ---- avr_cpu.c ----------------------------------------------------------- */

/* A CPU with flash_bytes of program memory (erased: 0xFF) and sram_bytes of
   SRAM after the 0x100 register bytes, reset. NULL when out of memory. */
AvrCpu *avr_cpu_create(uint32_t flash_bytes, uint32_t sram_bytes);
void avr_cpu_destroy(AvrCpu *cpu);
/* avr8js's CPU.reset(): SP to the top of memory, pc 0, no pending
   interrupt, no clock event. Memory is not cleared. */
void avr_cpu_reset(AvrCpu *cpu);

uint8_t avr_read_data(AvrCpu *cpu, int32_t addr);
void avr_write_data(AvrCpu *cpu, int32_t addr, uint8_t value, uint8_t mask);

static inline uint16_t avr_get16(const AvrCpu *cpu, uint32_t addr) {
    return (uint16_t)(cpu->data[addr] | (cpu->data[addr + 1] << 8));
}

static inline void avr_set16(AvrCpu *cpu, uint32_t addr, uint32_t value) {
    cpu->data[addr] = (uint8_t)value;
    cpu->data[addr + 1] = (uint8_t)(value >> 8);
}

static inline uint16_t avr_flash_word(const AvrCpu *cpu, int64_t index) {
    if (index < 0 || index >= cpu->flash_words) {
        return 0;
    }
    return (uint16_t)(cpu->flash[index * 2] | (cpu->flash[index * 2 + 1] << 8));
}

void avr_set_interrupt_flag(AvrCpu *cpu, const AvrInterrupt *interrupt);
void avr_update_interrupt_enable(AvrCpu *cpu, const AvrInterrupt *interrupt, uint8_t register_value);
void avr_queue_interrupt(AvrCpu *cpu, const AvrInterrupt *interrupt);
void avr_clear_interrupt(AvrCpu *cpu, const AvrInterrupt *interrupt, int clear_flag);
void avr_clear_interrupt_by_flag(AvrCpu *cpu, const AvrInterrupt *interrupt, uint8_t register_value);

/* Runs fn(ctx) once the cycle counter reaches now + max(1, cycles). An
   event's identity is its (fn, ctx) pair. */
void avr_add_clock_event(AvrCpu *cpu, AvrEventFn fn, void *ctx, double cycles);
/* Moves a queued event; 0 when it was not queued (nothing is added). */
int avr_update_clock_event(AvrCpu *cpu, AvrEventFn fn, void *ctx, double cycles);
/* Removes a queued event; 0 when it was not queued. */
int avr_clear_clock_event(AvrCpu *cpu, AvrEventFn fn, void *ctx);

/* avr8js's CPU.tick(): at most one due clock event, then the lowest pending
   interrupt when interrupts are enabled. */
void avr_cpu_tick(AvrCpu *cpu);
/* Takes the interrupt at word address `addr`: pushes pc, clears I. */
void avr_interrupt(AvrCpu *cpu, uint8_t addr);

/* ---- avr_instruction.c --------------------------------------------------- */

/* Executes the instruction at pc. */
void avr_instruction(AvrCpu *cpu);

/* ---- avr_gpio.c: avr8js's src/peripherals/gpio.ts (plan-62-B) ------------- */

typedef struct AvrPort AvrPort;

/* An external interrupt pin (INT0, INT1). `present` 0 = the pin has none. */
typedef struct {
    int present;
    uint16_t EICR;       /* the register with the pin's ISCx1:0 bits */
    uint16_t EIMSK;
    uint16_t EIFR;
    uint8_t isc_offset;  /* of ISCx0 in EICR */
    uint8_t index;       /* bit in EIMSK / EIFR */
    uint8_t interrupt;   /* vector word address */
} AvrExternalInterruptConfig;

typedef struct {
    uint8_t PCIE;        /* bit in PCICR / PCIFR */
    uint16_t PCICR;
    uint16_t PCIFR;
    uint16_t PCMSK;
    uint8_t pin_change_interrupt;   /* vector word address */
    uint8_t mask;
    uint8_t offset;
} AvrPinChangeConfig;

typedef struct {
    uint16_t PIN, DDR, PORT;
    int has_pin_change;
    AvrPinChangeConfig pin_change;
    AvrExternalInterruptConfig external[8];
} AvrPortConfig;

typedef enum { AVR_PIN_LOW, AVR_PIN_HIGH, AVR_PIN_INPUT, AVR_PIN_INPUT_PULLUP } AvrPinState;

/* How a timer's compare output takes a pin (avr8js's PinOverrideMode). */
typedef enum {
    AVR_OVERRIDE_NONE,
    AVR_OVERRIDE_ENABLE,
    AVR_OVERRIDE_SET,
    AVR_OVERRIDE_CLEAR,
    AVR_OVERRIDE_TOGGLE
} AvrPinOverride;

typedef void (*AvrPortListenerFn)(void *ctx, uint8_t value, uint8_t old_value);
typedef void (*AvrExternalClockFn)(void *ctx, int pin_value);

typedef struct { AvrPort *port; uint16_t reg; int type; } AvrPortHook;

struct AvrPort {
    AvrCpu *cpu;
    AvrPortConfig config;
    AvrInterrupt external_ints[8];   /* valid where config.external[i].present */
    AvrInterrupt low_level[8];       /* the low-level interrupt queued while the pin is low */
    AvrInterrupt pcint;              /* valid when config.has_pin_change */
    struct { AvrPortListenerFn fn; void *ctx; } listener;
    struct { AvrExternalClockFn fn; void *ctx; } external_clock[8];
    AvrPortHook hooks[10];
    size_t hook_count;
    uint8_t pin_value;
    uint8_t override_mask;
    uint8_t override_value;
    uint8_t last_value;
    uint8_t last_ddr;
    uint8_t last_pin;
    uint8_t open_collector;
};

enum { AVR_MAX_PORTS = 12 };

/* Installs the port on the CPU (its hooks, and the CPU's port list). */
void avr_port_init(AvrPort *port, AvrCpu *cpu, const AvrPortConfig *config);
/* The outside world drives pin `index` (0-7): what PINx reads on an input. */
void avr_port_set_pin(AvrPort *port, int index, int value);
/* What the chip does with the pin. */
AvrPinState avr_port_pin_state(const AvrPort *port, int index);
/* A timer's compare output takes, drives or releases a pin. */
void avr_port_override_pin(AvrPort *port, int pin, AvrPinOverride mode);
/* The port whose PORT register is at `port_address`, or NULL. */
AvrPort *avr_cpu_port_at(AvrCpu *cpu, uint16_t port_address);

/* ---- avr_clock.c: src/peripherals/clock.ts -------------------------------- */

typedef struct { uint16_t CLKPR; } AvrClockConfig;

typedef struct {
    AvrCpu *cpu;
    uint32_t base_hz;
    AvrClockConfig config;
    uint8_t reset_index;           /* CLKPR after reset (not in avr8js: always 0 there) */
    double clock_enabled_cycles;
    int prescaler_value;
    double cycles_delta;
} AvrClock;

void avr_clock_init(AvrClock *clock, AvrCpu *cpu, uint32_t base_hz, const AvrClockConfig *config,
                    uint8_t reset_index);
/* CLKPR and the prescaler back to the reset value. */
void avr_clock_reset(AvrClock *clock);
static inline int avr_clock_prescaler(const AvrClock *clock) { return clock->prescaler_value; }
/* avr8js's AVRClock.frequency: the CPU's clock in Hz. */
static inline double avr_clock_frequency(const AvrClock *clock) {
    return (double)clock->base_hz / clock->prescaler_value;
}
double avr_clock_time_nanos(const AvrClock *clock);

/* ---- avr_timer.c: src/peripherals/timer.ts -------------------------------- */

typedef struct {
    int bits;                 /* 8 or 16 */
    uint16_t dividers[8];     /* by CS2:0; 0 = stopped or external clock */
    /* Vector word addresses (0 = not available). */
    uint8_t capture_interrupt, comp_a_interrupt, comp_b_interrupt, comp_c_interrupt, ovf_interrupt;
    /* Register addresses (0 = not available). */
    uint16_t TIFR, OCRA, OCRB, OCRC, ICR, TCNT, TCCRA, TCCRB, TCCRC, TIMSK;
    /* TIFR bits, then TIMSK bits. */
    uint8_t TOV, OCFA, OCFB, OCFC;
    uint8_t TOIE, OCIEA, OCIEB, OCIEC;
    /* Compare output pins: the port's PORT address and the bit. */
    uint16_t comp_port_a; uint8_t comp_pin_a;
    uint16_t comp_port_b; uint8_t comp_pin_b;
    uint16_t comp_port_c; uint8_t comp_pin_c;
    /* The external clock pin (port 0 = none). */
    uint16_t external_clock_port; uint8_t external_clock_pin;
} AvrTimerConfig;

typedef struct {
    AvrCpu *cpu;
    AvrTimerConfig config;
    int max;                  /* 0xff or 0xffff */
    double last_cycle;
    int ocr_a, next_ocr_a, ocr_b, next_ocr_b, ocr_c, next_ocr_c;
    int has_ocr_c;
    int ocr_update_mode, tov_update_mode;
    int icr;
    int timer_mode, top_value;
    int tcnt, tcnt_next;
    int comp_a, comp_b, comp_c;
    int tcnt_updated, update_divider, counting_up;
    int divider;
    AvrPort *external_clock_port;
    int external_clock_rising_edge;
    uint8_t high_byte_temp;
    AvrInterrupt OVF, OCFA, OCFB, OCFC;
} AvrTimer;

void avr_timer_init(AvrTimer *timer, AvrCpu *cpu, const AvrTimerConfig *config);
void avr_timer_reset(AvrTimer *timer);

#endif
