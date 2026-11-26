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
    /* A reset from inside the chip (the watchdog). When set and `exact` is
       0 it runs in place of avr8js's bare CPU reset, so the whole chip
       restarts (avr_chip_reset). */
    AvrCallback on_reset;

    struct AvrPort *ports[12];      /* avr8js's gpioPorts, in the order they were made */
    size_t port_count;

    /* 0 (the simulator): the manual's behavior where avr8js differs from it.
       1 (the oracle tests): avr8js's behavior everywhere. */
    int exact;

    /* A clock event could not be kept (out of memory): the simulator's model
       stops the simulation after the instruction that asked for it. */
    int out_of_memory;

    /* Tests only: when not NULL, avr_instruction counts each decode branch
       it takes in coverage[1..AVR_BRANCHES]. */
    uint32_t *coverage;
};

/* ---- avr_cpu.c ----------------------------------------------------------- */

/* A CPU with flash_bytes of program memory (erased: 0xFF) and sram_bytes of
   SRAM after the 0x100 register bytes, reset. NULL when out of memory. */
AvrCpu *avr_cpu_create(uint32_t flash_bytes, uint32_t sram_bytes);
/* The same with the whole data space given: data_size bytes of registers,
   I/O and SRAM. An ATtiny25/45/85's SRAM starts at 0x60, so its data space
   is 0x60 + its SRAM (avr8js: `new CPU(flash, sram - 0xa0)`). */
AvrCpu *avr_cpu_create_data(uint32_t flash_bytes, uint32_t data_size);
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

/* ---- avr_dasm.c (plan-62-D) ------------------------------------------------ */

/* Disassembles the instruction whose first word is `word` (`next` is the
   word after it) at byte address `address` into out (cap bytes, at most 24
   are written). Returns its length in words, 1 or 2. */
unsigned avr_dasm(uint16_t word, uint16_t next, uint32_t address, char *out, size_t cap);

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
    /* Not in avr8js: told when the level the outside puts on a pin changes
       (avr_port_set_pin), whatever DDR says. The USI watches its pins so. */
    struct { AvrExternalClockFn fn; void *ctx; } pin_watch[8];
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

/* ---- avr_timer_tiny.c: src/peripherals/timer-attiny.ts ---------------------- */

/* Timer/Counter1 of the ATtiny25/45/85 (avr8js's ATtinyTimer1Config). */
typedef struct {
    uint16_t TCCR1, GTCCR, TCNT1, OCR1A, OCR1B, OCR1C, TIFR, TIMSK;
    uint8_t ovf_interrupt, comp_a_interrupt, comp_b_interrupt;   /* vector word addresses */
    uint8_t TOV1, OCF1A, OCF1B;      /* TIFR bits */
    uint8_t TOIE1, OCIE1A, OCIE1B;   /* TIMSK bits */
    uint16_t comp_port_b;            /* PORTB's address */
    uint8_t comp_pin_a, comp_pin_b;  /* OC1A, OC1B */
    uint16_t dividers[16];           /* by CS13:10 */
    /* Not in avr8js: the inverted outputs /OC1A and /OC1B (PWM mode with
       COM1x = 01), and PLLCSR. */
    uint8_t comp_pin_a_inverted, comp_pin_b_inverted;
    uint16_t PLLCSR;
} AvrTinyTimer1Config;

typedef struct {
    AvrCpu *cpu;
    AvrTinyTimer1Config config;
    int64_t last_cycle;
    int tcnt, tcnt_next, tcnt_updated;
    int ocr_a, ocr_b, ocr_c;
    int ocr_a_next, ocr_b_next;      /* not in avr8js: OCR1A/B are latched at TOP in PWM mode */
    int divider, update_divider, counting_up;
    int inverted_a, inverted_b;      /* not in avr8js: /OC1A, /OC1B are connected */
    AvrWriteHook tifr_hook, timsk_hook, gtccr_hook;   /* Timer0's, called first */
    AvrInterrupt OVF, OCFA, OCFB;
} AvrTinyTimer1;

/* After the other timers: TIFR, TIMSK and GTCCR are shared with Timer0,
   whose hooks this one chains to. */
void avr_tiny_timer1_init(AvrTinyTimer1 *timer, AvrCpu *cpu, const AvrTinyTimer1Config *config);
void avr_tiny_timer1_reset(AvrTinyTimer1 *timer);

/* ---- avr_usi.c: the Universal Serial Interface (not an avr8js port) --------- */

typedef struct {
    uint16_t USICR, USISR, USIDR, USIBR;
    uint8_t start_interrupt, overflow_interrupt;   /* vector word addresses */
    uint16_t port;                   /* the PORT address of the USI's pins */
    uint8_t data_in_pin;             /* DI, and SDA in two-wire mode */
    uint8_t data_out_pin;            /* DO */
    uint8_t clock_pin;               /* USCK, and SCL in two-wire mode */
} AvrUsiConfig;

/* What the USI does with one of its pins. */
typedef enum {
    AVR_USI_PIN_PORT,    /* nothing: the port has the pin */
    AVR_USI_PIN_LOW,
    AVR_USI_PIN_HIGH,
    AVR_USI_PIN_FLOAT    /* released, and no pull-up */
} AvrUsiPin;

typedef struct AvrUsi {
    AvrCpu *cpu;
    AvrUsiConfig config;
    AvrPort *port;
    int output_latch;                /* the bit the data output shows */
    int scl_hold;                    /* two-wire: the USI holds SCL low */
    int usiclk;                      /* USICLK as last written (it reads 0) */
    int buffer_follows;              /* the next shift also goes to USIBR */
    AvrCallback on_pins;             /* avr_usi_pin's answers may have changed */
    AvrInterrupt START, OVF;
} AvrUsi;

/* After the ports. */
void avr_usi_init(AvrUsi *usi, AvrCpu *cpu, const AvrUsiConfig *config);
void avr_usi_reset(AvrUsi *usi);
/* What the USI does with port bit `bit` now. */
AvrUsiPin avr_usi_pin(const AvrUsi *usi, int bit);

/* ---- avr_usart.c: src/peripherals/usart.ts (plan-62-C) --------------------- */

typedef struct {
    /* Vector word addresses. */
    uint8_t rx_complete_interrupt, data_register_empty_interrupt, tx_complete_interrupt;
    uint16_t UCSRA, UCSRB, UCSRC, UBRRL, UBRRH, UDR;
} AvrUsartConfig;

typedef struct {
    AvrCpu *cpu;
    AvrUsartConfig config;
    uint32_t freq_hz;
    /* A byte written to UDR leaves the chip (the frame is the listener's to time). */
    struct { void (*fn)(void *ctx, uint8_t value); void *ctx; } on_transmit;
    AvrCallback on_rx_complete;
    AvrCallback on_config;       /* the format or the baud rate changed */
    int rx_busy;
    uint8_t rx_byte;
    uint8_t rx_pending;          /* the byte a delayed avr_usart_write_byte delivers */
    AvrInterrupt RXC, UDRE, TXC;
} AvrUsart;

void avr_usart_init(AvrUsart *usart, AvrCpu *cpu, const AvrUsartConfig *config, uint32_t freq_hz);
void avr_usart_reset(AvrUsart *usart);
/* A received byte. immediate: it is in UDR now; else after one character
   time, with the receiver busy meanwhile. 0 when the receiver is busy or
   off (the byte is lost). */
int avr_usart_write_byte(AvrUsart *usart, uint8_t value, int immediate);
static inline int avr_usart_rx_busy(const AvrUsart *usart) { return usart->rx_busy; }
int avr_usart_rx_enabled(const AvrUsart *usart);
int avr_usart_tx_enabled(const AvrUsart *usart);
int avr_usart_bits(const AvrUsart *usart);        /* 5 to 9 */
int avr_usart_stop_bits(const AvrUsart *usart);   /* 1 or 2 */
int avr_usart_parity(const AvrUsart *usart);      /* 0 none, 1 even, 2 odd */
/* CPU cycles per bit: (UBRR + 1) x 16, or x 8 with U2X. Not in avr8js's
   interface; the pin-level layer (plan-62-F) times its frames with it. */
uint32_t avr_usart_bit_cycles(const AvrUsart *usart);
/* avr8js's cyclesPerChar: a whole frame. */
uint32_t avr_usart_char_cycles(const AvrUsart *usart);

/* ---- avr_spi.c: src/peripherals/spi.ts -------------------------------------- */

typedef struct {
    uint8_t spi_interrupt;       /* vector word address */
    uint16_t SPCR, SPSR, SPDR;
} AvrSpiConfig;

typedef struct AvrSpi {
    AvrCpu *cpu;
    AvrSpiConfig config;
    uint32_t freq_hz;
    /* A byte written to SPDR starts a transfer; the listener must call
       avr_spi_complete_transfer within avr_spi_transfer_cycles. NULL: the
       transfer completes by itself with 0 received. */
    struct { void (*fn)(void *ctx, uint8_t value); void *ctx; } on_byte;
    int transmission_active;
    AvrInterrupt SPI;
} AvrSpi;

void avr_spi_init(AvrSpi *spi, AvrCpu *cpu, const AvrSpiConfig *config, uint32_t freq_hz);
void avr_spi_reset(AvrSpi *spi);
void avr_spi_complete_transfer(AvrSpi *spi, uint8_t received);
int avr_spi_is_master(const AvrSpi *spi);
int avr_spi_lsb_first(const AvrSpi *spi);
int avr_spi_mode(const AvrSpi *spi);              /* 0-3: bit 0 CPOL, bit 1 CPHA */
int avr_spi_clock_divider(const AvrSpi *spi);
int avr_spi_transfer_cycles(const AvrSpi *spi);

/* ---- avr_twi.c: src/peripherals/twi.ts (master mode only, as avr8js) --------- */

typedef struct {
    uint8_t twi_interrupt;       /* vector word address */
    uint16_t TWBR, TWSR, TWAR, TWDR, TWCR, TWAMR;
} AvrTwiConfig;

/* The bus side of the master. Each call must be answered, now or later, by
   the avr_twi_complete_* of the same name. */
typedef struct {
    void (*start)(void *ctx, int repeated);
    void (*stop)(void *ctx);
    void (*connect)(void *ctx, uint8_t address, int write);
    void (*write_byte)(void *ctx, uint8_t value);
    void (*read_byte)(void *ctx, int ack);
    void *ctx;
} AvrTwiHandler;

typedef struct { uint8_t value, status, twdr; } AvrTwiRequest;

typedef struct AvrTwi {
    AvrCpu *cpu;
    AvrTwiConfig config;
    uint32_t freq_hz;
    AvrTwiHandler handler;       /* start NULL: every request is refused (no acknowledge) */
    int busy;
    AvrTwiRequest requests[8];   /* TWCR writes waiting for their clock event */
    unsigned request_head, request_count;
    AvrInterrupt TWI;
} AvrTwi;

void avr_twi_init(AvrTwi *twi, AvrCpu *cpu, const AvrTwiConfig *config, uint32_t freq_hz);
void avr_twi_reset(AvrTwi *twi);
void avr_twi_complete_start(AvrTwi *twi);
void avr_twi_complete_stop(AvrTwi *twi);
void avr_twi_complete_connect(AvrTwi *twi, int ack);
void avr_twi_complete_write(AvrTwi *twi, int ack);
void avr_twi_complete_read(AvrTwi *twi, uint8_t value);
int avr_twi_prescaler(const AvrTwi *twi);
/* CPU cycles per SCL period: 16 + 2 x TWBR x prescaler. Not in avr8js's
   interface; the pin-level layer (plan-62-F) uses it. */
uint32_t avr_twi_bit_cycles(const AvrTwi *twi);

/* ---- avr_adc.c: src/peripherals/adc.ts ---------------------------------------- */

typedef enum { AVR_ADC_AVCC, AVR_ADC_AREF, AVR_ADC_INTERNAL_1V1, AVR_ADC_INTERNAL_2V56,
               AVR_ADC_RESERVED } AvrAdcReference;
typedef enum { AVR_ADC_SINGLE_ENDED, AVR_ADC_DIFFERENTIAL, AVR_ADC_CONSTANT,
               AVR_ADC_TEMPERATURE } AvrAdcInputType;

typedef struct {
    int present;
    AvrAdcInputType type;
    uint8_t channel;             /* single ended */
    double voltage;              /* constant */
    uint8_t positive_channel, negative_channel;   /* differential */
    double gain;
} AvrAdcInput;

enum { AVR_ADC_MUX_INPUTS = 64, AVR_ADC_CHANNELS = 16 };

typedef struct {
    uint16_t ADMUX, ADCSRA, ADCSRB, ADCL, ADCH, DIDR0;
    uint8_t adc_interrupt;       /* vector word address */
    uint8_t num_channels;
    uint8_t mux_input_mask;
    AvrAdcInput mux_channels[AVR_ADC_MUX_INPUTS];
    AvrAdcReference adc_references[8];
    uint8_t reference_count;
    /* Not in avr8js: the ADMUX bit that, with REFS1, selects the 2.56 V
       reference (REFS2 of the ATtiny25/45/85: bit 4). 0 = none. */
    uint8_t refs2_mask;
} AvrAdcConfig;

typedef struct AvrAdc {
    AvrCpu *cpu;
    AvrAdcConfig config;
    double channel_volts[AVR_ADC_CHANNELS];
    double avcc, aref;
    /* A conversion starts; the listener must call avr_adc_complete_read.
       NULL: the value comes from channel_volts after avr_adc_sample_cycles. */
    struct { void (*fn)(void *ctx, const AvrAdcInput *input); void *ctx; } on_read;
    int converting;
    int conversion_cycles;
    int pending_result;
    AvrInterrupt ADC;
} AvrAdc;

void avr_adc_init(AvrAdc *adc, AvrCpu *cpu, const AvrAdcConfig *config);
void avr_adc_reset(AvrAdc *adc);
void avr_adc_complete_read(AvrAdc *adc, int value);
int avr_adc_sample_cycles(const AvrAdc *adc);

/* ---- avr_eeprom.c: src/peripherals/eeprom.ts ----------------------------------- */

typedef struct {
    uint8_t eeprom_ready_interrupt;   /* vector word address */
    uint16_t EECR, EEDR, EEARL, EEARH;
    uint32_t erase_cycles, write_cycles;
} AvrEepromConfig;

typedef struct {
    AvrCpu *cpu;
    AvrEepromConfig config;
    uint8_t *memory;             /* erased: 0xFF */
    uint32_t size;
    double write_enabled_cycles;
    double write_complete_cycles;
    AvrInterrupt EER;
} AvrEeprom;

/* 0 on success, -1 when out of memory. */
int avr_eeprom_init(AvrEeprom *eeprom, AvrCpu *cpu, uint32_t size, const AvrEepromConfig *config);
void avr_eeprom_free(AvrEeprom *eeprom);
void avr_eeprom_reset(AvrEeprom *eeprom);   /* the contents are kept */

/* ---- avr_watchdog.c: src/peripherals/watchdog.ts -------------------------------- */

typedef struct {
    uint8_t watchdog_interrupt;  /* vector word address */
    uint16_t MCUSR, WDTCSR;
} AvrWatchdogConfig;

typedef struct {
    AvrCpu *cpu;
    AvrWatchdogConfig config;
    AvrClock *clock;
    double change_enabled_cycles;
    double watchdog_timeout;
    int enabled_value;
    int scheduled;
    AvrInterrupt Watchdog;
} AvrWatchdog;

void avr_watchdog_init(AvrWatchdog *watchdog, AvrCpu *cpu, const AvrWatchdogConfig *config,
                       AvrClock *clock);
void avr_watchdog_reset(AvrWatchdog *watchdog);

#endif
