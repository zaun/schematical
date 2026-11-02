#ifndef SERIAL_TARGET_H
#define SERIAL_TARGET_H

/*
 * SPI target and I2C target host front ends for the serial models
 * (plan-12-I section 3.1; landed with plan-12-J, which needs them first).
 * Byte level: the front end does the pin protocol and hands the model whole
 * bytes through callbacks; the model owns its command and sub-address
 * layout (the Maxim command byte, the NXP sub-address byte, the SC18IS604
 * command set).
 *
 *   SerialSpi    CS, SCLK, MOSI, MISO; SPI modes 0-3 (CPOL, CPHA). MISO is
 *                3-state while CS is high and drives only the bits the model
 *                marks to drive (others Z) while CS is low.
 *   SerialI2c    SCL, SDA: START, repeated START, STOP, the address byte
 *                (the model says ACK or NACK), bytes in with ACK/NACK from
 *                the model, bytes out shifted on SCL falling edges, the
 *                controller's ACK/NACK. SDA is open drain (0 strong or Z).
 *                No clock stretching: SCL is only an input.
 *   SerialHost   one of the two, or both on shared pins with a select pin
 *                ("i2c-spi"), loaded from a device's "bus" object.
 *
 * Signal rules (plan-12-A section 3.5): SCLK, MOSI, SCL and SDA at Z read X;
 * CS at Z reads inactive (1). An X on CS or SCLK during a transaction makes
 * the rest of it X: bits in arrive unknown and MISO drives X where it would
 * drive. An X on SCL, or on SDA while SCL is 1, ends an I2C transaction
 * (the model hears end(0)) and the target waits for the next START.
 *
 * Address straps (I2C): a strap pin may be tied low, high, to SCL or to SDA
 * (the SC16IS7xx and MAX3108 address tables). The front end tells them
 * apart from the strap's level in three bus states: idle (SCL 1, SDA 1), at
 * the START (SCL 1, SDA 0) and at the first SCL fall after it (SCL 0,
 * SDA 0): low 000, high 111, SCL 110, SDA 100. The strap is decoded by the
 * time the address byte is complete.
 */

#include <stddef.h>
#include <stdint.h>

#include "kind.h"
#include "serial_bus.h"
#include "sim.h"

enum {
    SERIAL_I2C_STRAPS = 4
};

/* Strap ties (the digits of an I2C address table index). */
enum {
    SERIAL_TIE_LOW = 0,
    SERIAL_TIE_HIGH = 1,
    SERIAL_TIE_SCL = 2,
    SERIAL_TIE_SDA = 3
};

/* ---- SPI target ------------------------------------------------------------- */

typedef struct {
    size_t cs;              /* active low */
    size_t sclk;
    size_t mosi;
    size_t miso;
    unsigned mode;          /* 0-3: CPOL = mode >> 1, CPHA = mode & 1 */
} SerialSpiPins;

/* A byte to shift out MSB first: bit n of drive = 1 drives bit n of data
   (X where known is 0), 0 leaves MISO Z for that bit. */
typedef struct {
    uint8_t data;
    uint8_t known;
    uint8_t drive;
} SerialSpiByte;

typedef struct {
    void *context;
    /* CS fell: a transaction begins. May be NULL. */
    void (*begin)(void *context);
    /* The byte to shift out as byte `index` of this transaction (0 first).
       Asked just before its first bit goes out. */
    SerialSpiByte (*next)(void *context, unsigned index);
    /* Optional: a bit came in. `bits` = bits of byte `index` so far (1-8),
       `data`/`known` = those bits, right-aligned. The model may change the
       bits of *out that have not gone out yet (a reply that depends on the
       command's first bits). */
    void (*bit)(void *context, unsigned index, unsigned bits, uint8_t data, uint8_t known,
                SerialSpiByte *out);
    /* A whole byte came in (the 8th sample edge). */
    void (*received)(void *context, unsigned index, uint8_t data, uint8_t known);
    /* Optional: a shift edge, `bits` = bits sampled so far in the transaction. */
    void (*shift)(void *context, unsigned bits);
    /* CS rose: the transaction ends; `bits` = bits sampled in it. May be NULL. */
    void (*end)(void *context, unsigned bits);
} SerialSpiEvents;

typedef struct {
    const SerialSpiPins *pins;
    SimTime t_valid;        /* CS fall to the first MISO bit (CPHA 0) */
    SimTime t_out;          /* shift edge to the next MISO bit */
    SimTime t_float;        /* CS rise to MISO Z */
    LogicValue sclk;        /* SCLK's last known level (0 or 1) */
    LogicValue cs;          /* CS as last seen: 0 selected, 1 idle, X */
    int active;             /* a transaction is open */
    int x;                  /* this transaction is X */
    unsigned bits;          /* bits sampled */
    uint8_t in;
    uint8_t in_known;
    SerialSpiByte out;
    int out_bit;            /* the bit of `out` on MISO now (7..0); -1 none */
    int fetched;            /* `out` holds byte bits / 8 */
    LogicValue miso;        /* what MISO is set to drive */
    int enabled;
} SerialSpi;

/* Timing starts at 0; the model sets t_valid, t_out and t_float. */
void serial_spi_init(SerialSpi *spi, const SerialSpiPins *pins);
/* Reads the pins' levels; MISO Z. */
void serial_spi_start(SerialSpi *spi, Sim *sim, SimInstance *instance);
/* Any pin changed (other pins are ignored). */
void serial_spi_pin(SerialSpi *spi, Sim *sim, SimInstance *instance, size_t pin,
                    const SerialSpiEvents *events);
/* Enables or disables the target (reset, the other front end selected). A
   disabled target drops its transaction (no end event) and releases MISO;
   once enabled it waits for the next CS fall. */
void serial_spi_enable(SerialSpi *spi, Sim *sim, SimInstance *instance, int enabled);

/* ---- I2C target --------------------------------------------------------------- */

typedef struct {
    size_t scl;
    size_t sda;
    size_t strap[SERIAL_I2C_STRAPS];   /* address strap pins, A0 first */
    unsigned strap_count;
} SerialI2cPins;

typedef struct {
    void *context;
    /* A START (repeated = 0) or a repeated START while this target was
       addressed (repeated = 1). May be NULL. */
    void (*begin)(void *context, int repeated);
    /* The address byte (7-bit address << 1 | R/W) with its known bits:
       LOGIC_1 = ACK (this target), LOGIC_0 = NACK (not this target; the
       front end ignores the bus until the next START), LOGIC_X = undefined. */
    LogicValue (*address)(void *context, uint8_t byte, uint8_t known);
    /* A byte written by the controller (index 0 = first after the address):
       LOGIC_1 ACK, LOGIC_0 NACK, LOGIC_X. */
    LogicValue (*received)(void *context, unsigned index, uint8_t data, uint8_t known);
    /* The byte to send as byte `index` of a read (at the SCL fall that
       starts it). */
    void (*next)(void *context, unsigned index, uint8_t *data, uint8_t *known);
    /* The controller answered byte `index` of a read: LOGIC_1 ACK (SDA 0),
       LOGIC_0 NACK, LOGIC_X. The read's side effects belong here. May be NULL. */
    void (*acked)(void *context, unsigned index, LogicValue ack);
    /* The transaction this target was addressed in ended: stop = 1 at a STOP,
       0 when an X on the bus abandoned it. May be NULL. */
    void (*end)(void *context, int stop);
} SerialI2cEvents;

typedef enum {
    SERIAL_I2C_IDLE,
    SERIAL_I2C_ADDRESS,
    SERIAL_I2C_WRITE,
    SERIAL_I2C_ACK_OUT,     /* this target's ACK slot */
    SERIAL_I2C_READ,
    SERIAL_I2C_ACK_IN,      /* the controller's ACK slot */
    SERIAL_I2C_IGNORE       /* not addressed: wait for START or STOP */
} SerialI2cPhase;

typedef struct {
    const SerialI2cPins *pins;
    SimTime t_out;          /* SCL fall to SDA */
    SerialI2cPhase phase;
    LogicValue scl;
    LogicValue sda;
    unsigned bits;
    uint8_t in;
    uint8_t in_known;
    uint8_t out;
    uint8_t out_known;
    unsigned index;
    int read;               /* the address byte's R/W */
    int selected;           /* addressed in this transaction (an end event is owed) */
    LogicValue ack;         /* the ACK decided for the slot coming */
    LogicValue sda_out;     /* what SDA is set to drive (0, X or Z) */
    int after_start;        /* the first SCL fall after a START is still to come */
    LogicValue strap_idle[SERIAL_I2C_STRAPS];
    LogicValue strap_start[SERIAL_I2C_STRAPS];
    LogicValue strap_fall[SERIAL_I2C_STRAPS];
    int enabled;
} SerialI2c;

/* t_out starts at 0; the model may set it. */
void serial_i2c_init(SerialI2c *i2c, const SerialI2cPins *pins);
void serial_i2c_start(SerialI2c *i2c, Sim *sim, SimInstance *instance);
void serial_i2c_pin(SerialI2c *i2c, Sim *sim, SimInstance *instance, size_t pin,
                    const SerialI2cEvents *events);
void serial_i2c_enable(SerialI2c *i2c, Sim *sim, SimInstance *instance, int enabled);
/* The tie of strap i (SERIAL_TIE_*), or -1 when unknown (X, or levels that
   match no tie). Valid from the address byte of a transaction on. */
int serial_i2c_strap(const SerialI2c *i2c, unsigned i);

/* ---- Loaded front ends ("bus" object) ------------------------------------------ */

typedef enum {
    SERIAL_HOST_SPI,
    SERIAL_HOST_I2C,
    SERIAL_HOST_I2C_SPI     /* both, picked by a select pin */
} SerialHostStyle;

enum { SERIAL_I2C_TABLE = 64 };

typedef struct {
    SerialHostStyle style;
    /* SPI */
    const char *cs, *sclk, *mosi, *miso;
    unsigned mode;
    /* I2C */
    const char *scl, *sda;
    const char *strap[SERIAL_I2C_STRAPS];
    unsigned strap_count;
    int table[SERIAL_I2C_TABLE];       /* 7-bit address per strap combination; -1 none */
    unsigned table_count;              /* 4 ^ strap_count */
    /* i2c-spi */
    const char *select;
    int spi_level;                     /* the select level that picks SPI */
    int select_live;                   /* 1: follow the pin; 0: sampled at reset */
} SerialHostDevice;

typedef struct {
    SerialSpiPins spi;
    SerialI2cPins i2c;
    size_t select;
} SerialHostPins;

typedef enum {
    SERIAL_HOST_NONE,       /* neither (select X, or in reset) */
    SERIAL_HOST_ON_SPI,
    SERIAL_HOST_ON_I2C
} SerialHostMode;

typedef struct {
    const SerialHostDevice *dev;
    const SerialHostPins *pins;
    SerialSpi spi;
    SerialI2c i2c;
    SerialHostMode mode;
    int held;               /* in reset: nothing active */
    LogicValue sampled;     /* the select level sampled at reset (select_live 0) */
} SerialHost;

/*
 * Loads a "bus" object: {"style": "spi", "cs", "sclk", "mosi", "miso",
 * "mode"}; {"style": "i2c", "scl", "sda", "address": [strap names, A0
 * first], "table": [7-bit address or null per strap combination]};
 * {"style": "i2c-spi", "selectPin", "spiLevel" (0 or 1), "selectSampled"
 * ("live" or "reset"), "spi": {...}, "i2c": {...}}. `extra` = the model's
 * own keys in the bus object (may be NULL). Table index: the sum over straps
 * of tie(A_k) * 4^k (SERIAL_TIE_*).
 */
int serial_host_load(KindContext *ctx, const cJSON *bus, const char *const *extra,
                     SerialHostDevice *out);
int serial_host_bind(KindContext *ctx, const SerialHostDevice *dev, const PartPin *pins,
                     size_t pin_count, SerialHostPins *out);
/* 1 when the pin is one of the front end's (or a strap or the select). */
int serial_host_has_pin(const SerialHostDevice *dev, const SerialHostPins *pins, size_t pin);
/* 1 when the pin is open drain in some mode (SDA; SCL as the bus line). */
int serial_host_open_drain(const SerialHostDevice *dev, const SerialHostPins *pins, size_t pin);

void serial_host_init(SerialHost *host, const SerialHostDevice *dev, const SerialHostPins *pins);
void serial_host_start(SerialHost *host, Sim *sim, SimInstance *instance);
void serial_host_pin(SerialHost *host, Sim *sim, SimInstance *instance, size_t pin,
                     const SerialSpiEvents *spi, const SerialI2cEvents *i2c);
/* Reset held (1) or released (0): a release samples a "reset" select. */
void serial_host_hold(SerialHost *host, Sim *sim, SimInstance *instance, int held);
/* The table's 7-bit address for the straps now: >= 0, -1 none, -2 unknown. */
int serial_host_i2c_address(const SerialHost *host);

#endif
