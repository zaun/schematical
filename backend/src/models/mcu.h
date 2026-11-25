#ifndef MCU_H
#define MCU_H

/*
 * The mcu kind's internal interface (plan-62-E, F): what the loader (mcu.c)
 * builds for a device with a "model", and the engine adapter for AVR models
 * that mcu.c's engine model forwards to: mcu_avr.c (the chip on the engine:
 * clock, reset, port pins, ADC, inspection) and mcu_avr_serial.c (the
 * USART, SPI and TWI on their pins).
 */

#include "avr_chip.h"
#include "cpu_state.h"
#include "kind.h"

#define MCU_NO_PIN ((size_t)-1)

typedef struct {
    PartMcu facts;
    const PartVoltage *vcc;
    const AvrLayout *layout;   /* NULL: no "model", facts only */
} McuDevice;

/* A model device's pins in one pinout, by name. */
typedef struct {
    const AvrLayout *layout;
    size_t port[AVR_LAYOUT_PORTS][8];   /* [layout port][bit]: the pin "P<letter><bit>" */
    size_t adc6, adc7;                  /* pins named ADC6 and ADC7 */
} McuAvrPins;

typedef struct McuAvr McuAvr;

/* ---- mcu_avr.c: the engine hooks -------------------------------------------- */

McuAvr *mcu_avr_create(const Part *part, const McuAvrPins *pins);
void mcu_avr_destroy(McuAvr *mcu);
void mcu_avr_start(McuAvr *mcu, Sim *sim, SimInstance *instance);
void mcu_avr_pin_changed(McuAvr *mcu, size_t pin, LogicValue old_value, LogicValue new_value);
void mcu_avr_timer(McuAvr *mcu, uint32_t timer_id);
int mcu_avr_memory_size(const McuAvr *mcu, uint32_t *words, unsigned *bits);
size_t mcu_avr_memory_read(const McuAvr *mcu, uint32_t start, size_t count, uint32_t *values,
                           uint32_t *known);
void mcu_avr_memory_load(McuAvr *mcu, uint32_t start, const uint32_t *values, size_t count);
int mcu_avr_cpu_state(const McuAvr *mcu, CpuState *out);

/* ---- The adapter's state (shared by mcu_avr.c and mcu_avr_serial.c) ---------- */

typedef enum { MCU_STARTUP, MCU_IN_RESET, MCU_RUNNING } McuState;

/* What a part pin is to the chip. */
typedef enum { MCU_PIN_OTHER, MCU_PIN_PORT, MCU_PIN_RESET, MCU_PIN_ADC_ONLY } McuPinKind;

typedef struct {
    McuPinKind kind;
    uint8_t port;        /* layout port index */
    uint8_t bit;
    int8_t adc_channel;  /* -1: not an ADC input */
} McuPin;

/* What the chip last put on a pin. */
typedef struct {
    uint8_t value;       /* LogicValue */
    uint8_t strength;    /* DriveStrength */
} McuDrive;

/* A serial peripheral takes a port pin: it drives it low or high, or
   releases it (an open-drain line at 1: the pin is an input, with its
   pull-up when the PORT bit is set). NONE: the port has the pin. */
typedef enum { MCU_OVERRIDE_NONE, MCU_OVERRIDE_LOW, MCU_OVERRIDE_HIGH,
               MCU_OVERRIDE_RELEASE } McuOverride;

/* Timer kinds: the low byte of a timer id (the rest is the generation). A
   serial timer's kind is its base plus the peripheral's index. */
enum {
    MCU_TIMER_STARTUP = 1,
    MCU_TIMER_STEP = 2,
    MCU_TIMER_USART_TX = 0x10,
    MCU_TIMER_USART_RX = 0x20,
    MCU_TIMER_SPI = 0x30,
    MCU_TIMER_TWI = 0x40
};

enum { MCU_TX_QUEUE = 8, MCU_FRAME_BITS = 14, MCU_TWI_STEPS = 96 };

/* A USART on its pins (mcu_avr_serial.c). */
typedef struct {
    McuAvr *mcu;
    unsigned index;
    size_t rxd, txd;                 /* part pins, or MCU_NO_PIN */
    uint16_t tx_queue[MCU_TX_QUEUE]; /* bytes waiting (bit 8: TXB8) */
    unsigned tx_head, tx_count;
    int tx_active;
    uint8_t tx_levels[MCU_FRAME_BITS];
    unsigned tx_length, tx_position;
    SimTime tx_bit_ps;
    int rx_active;
    unsigned rx_position, rx_data_bits, rx_frame_bits;
    uint16_t rx_shift;
    SimTime rx_bit_ps;
} McuUsartLine;

/* An SPI master on its pins. */
typedef struct {
    McuAvr *mcu;
    unsigned index;
    size_t mosi, miso, sck;
    AvrWriteHook spcr_hook;          /* the peripheral's own SPCR hook, called first */
    int active;
    uint8_t out, in;
    unsigned edge;                   /* 1..16: the clock edges of a byte */
    int cpol, cpha, lsb_first;
    SimTime half_ps;
} McuSpiLine;

typedef struct { uint8_t kind, arg; } McuTwiStep;

/* A TWI master on its two open-drain lines. */
typedef struct {
    McuAvr *mcu;
    unsigned index;
    size_t sda, scl;
    AvrWriteHook twcr_hook;          /* the peripheral's own TWCR hook, called first */
    int enabled;
    McuTwiStep steps[MCU_TWI_STEPS]; /* the bus operation in progress */
    unsigned count, position;
    int waiting_scl;                 /* released SCL, waiting for it to read 1 */
    uint8_t shift;
    int ack;
    SimTime quarter_ps;
} McuTwiLine;

typedef struct {
    McuAvr *mcu;
    size_t port;
} McuPortListener;

struct McuAvr {
    const Part *part;
    const McuAvrPins *pins;
    AvrChip *chip;
    Sim *sim;
    SimInstance *instance;

    McuPin *pin_map;             /* part->pin_count entries */
    McuDrive *driven;            /* part->pin_count entries */
    uint8_t *override;           /* part->pin_count entries: McuOverride */
    McuPortListener listeners[AVR_LAYOUT_PORTS];
    size_t xtal1, xtal2, reset_pin;   /* part pins, or MCU_NO_PIN */

    McuState state;
    uint32_t generation;         /* stale timers carry an older one */

    /* The clock. */
    int external_clock;
    uint64_t startup_edges;
    SimTime last_edge;           /* the last rising edge of XTAL1 */
    SimTime period_ps;           /* between the last two */
    int edge_wait;               /* external: edges until the next CPU cycle */
    uint64_t clock_cycles;       /* external: CPU cycles since the run began */
    uint64_t due;                /* external: the cycle the next instruction starts at */
    int chip_was_reset;          /* the chip reset itself during the last step */

    /* The last instruction, for the processor window. */
    int has_instruction;
    uint32_t instruction_pc;
    uint16_t instruction_words[2];
    unsigned instruction_length;

    /* The serial peripherals on their pins. */
    McuUsartLine usarts[AVR_LAYOUT_SERIALS];
    McuSpiLine spis[AVR_LAYOUT_SERIALS];
    McuTwiLine twis[AVR_LAYOUT_SERIALS];
};

/* ---- mcu_avr.c, for mcu_avr_serial.c ---------------------------------------- */

/* The part pin of a port pin by name, or MCU_NO_PIN. */
size_t mcu_avr_pin(const McuAvr *mcu, AvrPinName name);
/* A serial peripheral takes or returns a pin; the pin's drive follows at once. */
void mcu_avr_set_override(McuAvr *mcu, size_t pin, McuOverride override);
/* 1 when the pin's DDR bit is set. */
int mcu_avr_pin_is_output(const McuAvr *mcu, size_t pin);
/* 1 when the pin's net reads 1 (0 for 0, X, Z and no pin). */
int mcu_avr_pin_high(const McuAvr *mcu, size_t pin);
/* One CPU cycle in picoseconds, now. */
SimTime mcu_avr_cycle_ps(const McuAvr *mcu);
/* A timer of this kind (MCU_TIMER_*) after `delay`; a reset cancels it. */
void mcu_avr_schedule(McuAvr *mcu, unsigned kind, SimTime delay);

/* ---- mcu_avr_serial.c --------------------------------------------------------- */

/* Once, after the chip is made: finds the pins and installs the callbacks. */
void mcu_avr_serial_init(McuAvr *mcu);
/* Every transfer in progress is forgotten (the chip was reset). The caller
   clears the overrides. */
void mcu_avr_serial_reset(McuAvr *mcu);
/* A port pin changed while the chip runs. */
void mcu_avr_serial_pin(McuAvr *mcu, size_t pin, LogicValue old_value, LogicValue new_value);
/* A serial timer fired. */
void mcu_avr_serial_timer(McuAvr *mcu, unsigned kind);

#endif
