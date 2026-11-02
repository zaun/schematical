#ifndef SERIAL_BUS_H
#define SERIAL_BUS_H

/*
 * Host bus helpers for the serial models (plan-12-C section 3.3).
 *
 * Intel style: address lines, chip selects (active-low and active-high
 * lists), an optional address strobe (ADS: address and selects pass through
 * while it is low and are latched on its rising edge), read strobes (an
 * active-low and/or an active-high pin: a read is selected AND (RDBAR low OR
 * RD high)), write strobes likewise, the 3-state data bus with the grade's
 * access and float delays, DDIS (low while the chip drives the bus) and
 * CSOUT (high while selected).
 *
 * Signal rules (plan-12-A section 3.5): a strobe or address pin at Z reads
 * as X; a chip select at Z reads inactive. An X on a strobe or select makes
 * the cycle X: a read drives X; a write stores X into the addressed register
 * (every register an X address could name). Both read strobes active at once
 * read X; both write strobes active are one write (PC16550D decisions).
 * A write stores the data on the bus when the cycle ends (the trailing edge).
 */

#include <stddef.h>
#include <stdint.h>

#include "sim.h"

enum {
    SERIAL_BUS_DATA = 8,
    SERIAL_BUS_ADDRESS = 8,
    SERIAL_BUS_SELECTS = 4
};

#define SERIAL_NO_PIN ((size_t)-1)
/* A pin a package leaves out, tied inside at a fixed level ("absentTies"). */
#define SERIAL_TIE_0 ((size_t)-2)
#define SERIAL_TIE_1 ((size_t)-3)
/* 1 for a pin that exists on the package (not absent, not tied). */
#define SERIAL_REAL_PIN(p) ((p) < SERIAL_TIE_1)

typedef struct {
    size_t data[SERIAL_BUS_DATA];
    size_t address[SERIAL_BUS_ADDRESS];   /* address[0] = A0 */
    unsigned address_count;
    size_t read;            /* active low (RDBAR); SERIAL_NO_PIN = none */
    size_t read_high;       /* active high (RD) */
    size_t write;           /* active low (WRBAR) */
    size_t write_high;      /* active high (WR) */
    size_t select[SERIAL_BUS_SELECTS];        /* active low */
    unsigned select_count;
    size_t select_high[SERIAL_BUS_SELECTS];   /* active high */
    unsigned select_high_count;
    size_t address_strobe;  /* ADS, active low */
    size_t driver_disable;  /* DDIS out */
    size_t select_out;      /* CSOUT out */
} SerialBusPins;

typedef struct {
    SimTime access;         /* read strobe to data (trvd) */
    SimTime release;        /* read end to float (thz) */
    SimTime ddis;           /* read to DDIS (trdd) */
    SimTime select_out;     /* select to CSOUT */
} SerialBusTiming;

typedef struct {
    void *context;
    /*
     * Optional: the select decode, for models with more than one unit
     * (per-channel chip selects, a channel pin). Returns 0, 1 or X and sets
     * *units, a bit per selected unit. NULL = the pins' selects, unit 0.
     * `address` is the latched address (value, known). Several units at
     * once: a read is an X cycle; a write goes to each of them (return X
     * instead when the device does not broadcast).
     */
    LogicValue (*select)(void *context, const Sim *sim, const SimInstance *instance,
                         unsigned address, unsigned address_known, uint32_t *units);
    /* The byte a read returns now. */
    void (*peek)(void *context, unsigned unit, unsigned address, uint8_t *data, uint8_t *known);
    /* A read ended: its side effects (an RBR pop, status clears). */
    void (*read_done)(void *context, unsigned unit, unsigned address);
    /* A write (known: the defined bits of data). */
    void (*write)(void *context, unsigned unit, unsigned address, uint8_t data, uint8_t known);
} SerialBusEvents;

typedef enum {
    SERIAL_CYCLE_NONE,
    SERIAL_CYCLE_ACTIVE,
    SERIAL_CYCLE_X
} SerialCycle;

typedef struct {
    const SerialBusPins *pins;
    SerialBusTiming timing;
    /* The address and select as latched by ADS (or live without it). */
    LogicValue latched_address[SERIAL_BUS_ADDRESS];
    LogicValue latched_select;
    uint32_t latched_units;
    SerialCycle read;
    SerialCycle write;
    unsigned read_unit;     /* the one unit a read reads */
    unsigned read_address;
    uint32_t write_units;
    unsigned write_address;
    unsigned write_address_known;
    LogicValue data_out[SERIAL_BUS_DATA];   /* what each data pin is set to drive */
    LogicValue ddis_out;
    LogicValue select_out;
    int enabled;            /* 0 while in reset: no cycles */
} SerialBus;

/* A pin's level as an input: 0, 1 or X; Z reads as `z_reads`. */
LogicValue serial_level(const Sim *sim, const SimInstance *instance, size_t pin, LogicValue z_reads);

void serial_bus_init(SerialBus *bus, const SerialBusPins *pins, const SerialBusTiming *timing);

/* Drives the idle outputs (data Z, DDIS high, CSOUT per select). */
void serial_bus_start(SerialBus *bus, Sim *sim, SimInstance *instance,
                      const SerialBusEvents *events);

/* Any bus pin changed: runs cycles. */
void serial_bus_update(SerialBus *bus, Sim *sim, SimInstance *instance,
                       const SerialBusEvents *events);

/* The model's state changed during a read: re-reads and drives the new byte. */
void serial_bus_refresh(SerialBus *bus, Sim *sim, SimInstance *instance,
                        const SerialBusEvents *events);

/* 1 when the pin is one of the bus's inputs. */
int serial_bus_has_pin(const SerialBus *bus, size_t pin);

#endif
