#ifndef CONVERTER_H
#define CONVERTER_H

/*
 * The converter kind (plan-24-D, -E) and the rtc kind (plan-24-F): one
 * declarative engine that runs a device's digital side as
 * planning/plan-24-data/schema.md describes it — registers, a bit-pattern
 * protocol over SPI, I2C, a parallel bus or a clocked output bus,
 * conversions, ready pins, DAC registers, RTC timekeeping.
 * docs/parts-format.md § converter, § rtc.
 *
 * Pins are named in the device; every name a key uses gets a name id (an
 * index into CvDevice.names) and each pinout binds the ids to pin indices.
 */

#include <stddef.h>
#include <stdint.h>

#include "kind.h"
#include "serial_target.h"
#include "sim.h"

enum {
    CV_NONE = -1,
    CV_REG_R = -2,          /* the result code */
    CV_REG_CH = -3,         /* the channel of the conversion being output */
    CV_CAPS = 25,           /* capture letters a..y */
    CV_MAX_BITS = 4096,     /* longest pattern / input buffer */
    CV_MAX_DATA = 24        /* parallel / clocked data pins */
};

typedef struct {
    int reg;                /* register index, CV_REG_R or CV_REG_CH */
    unsigned hi, lo;
} CvField;

typedef struct {
    CvField field;
    int negate;
    uint32_t value;
} CvCond;

typedef struct {
    const CvCond *items;
    size_t count;
} CvConds;

/* A capture letter (cap 1..25 = 'a'..'y') or, with cap 0, the constant value. */
typedef struct {
    int cap;
    uint32_t value;
} CvValue;

/* ---- Patterns: one token per bit, except CVT_AT ------------------------------------ */

typedef enum { CVT_0, CVT_1, CVT_X, CVT_Z, CVT_CAP, CVT_R, CVT_CH, CVT_REF, CVT_AT } CvTokKind;

typedef struct {
    uint8_t kind;
    uint8_t cap;            /* CVT_CAP: 0..24 */
    uint8_t bit;            /* the value bit this token carries */
    int16_t reg;            /* CVT_REF: register */
} CvTok;

typedef struct {
    const CvTok *toks;
    size_t count;
    unsigned bits;          /* not counting @ */
    int has_at;
} CvPattern;

/* ---- Commands -------------------------------------------------------------------------- */

typedef enum {
    CVA_NOP, CVA_WREG, CVA_RREG, CVA_WRITE, CVA_READ, CVA_POINTER, CVA_START, CVA_STOP,
    CVA_RESET, CVA_POWERDOWN, CVA_WAKEUP, CVA_UPDATE, CVA_UPDATEALL, CVA_CLEAR, CVA_SETBITS,
    CVA_CLEARBITS
} CvAction;

typedef struct {
    CvField field;
    CvValue value;
} CvSet;

typedef struct {
    uint32_t key;
    const uint8_t *channels;
    size_t count;
} CvMapEntry;

typedef struct {
    const char *name;
    CvPattern in;
    uint8_t action;
    int has_address;
    CvValue address;
    int space;
    int reg;                /* "register", or CV_NONE */
    CvValue count;
    int count_offset;
    int has_data;
    CvValue data;
    int has_channel;
    CvValue channel;
    const CvMapEntry *map;
    size_t map_count;
    const CvSet *sets;
    size_t set_count;
    int has_reply, has_next;
    CvPattern reply, next;
    int commit_end;
    const uint8_t *then;
    size_t then_count;
    unsigned word_bits;
    CvConds when;
    int has_increment;
    CvValue increment;
} CvCommand;

/* ---- Registers -------------------------------------------------------------------------- */

typedef struct {
    const char *name;
    unsigned hi, lo;
} CvRegField;

typedef struct {
    const char *name;
    int32_t address;        /* CV_NONE: internal */
    int space;
    unsigned width;
    uint32_t reset, ro, w1c, cor;
    int result;             /* 0 none; 1 the code right-aligned; 2 into bits result_hi..lo */
    unsigned result_hi, result_lo;
    int slice;              /* resultBits */
    unsigned slice_hi, slice_lo;
    int result_channel;     /* CV_NONE: any */
    const CvRegField *fields;
    size_t field_count;
} CvRegister;

/* ---- Buses ------------------------------------------------------------------------------ */

typedef enum { CVB_SPI, CVB_I2C, CVB_I2C_SPI, CVB_PARALLEL, CVB_CLOCKED } CvBusStyle;

typedef struct {
    int pin;                /* name id, CV_NONE */
    int level;              /* the active level (rw: the read level; byteSelect: the high level) */
} CvCtl;

typedef struct {
    int pin;
    int invert;
    unsigned divide;        /* 0 or 1: follow the clock */
} CvClockOut;

typedef struct {
    int pin;
    int fall;               /* 1 the falling edge latches */
    unsigned bits;
} CvAle;

typedef struct {
    CvBusStyle style;
    /* spi */
    int cs, sclk, mosi, miso;
    int cs_level;           /* the selected level of cs (csActive) */
    unsigned mode;
    /* i2c */
    int scl, sda;
    int straps[SERIAL_I2C_STRAPS];
    unsigned strap_count;
    int table[SERIAL_I2C_TABLE];
    unsigned table_count;
    /* i2c-spi */
    int select;
    int spi_level;
    /* parallel / clocked */
    int data[CV_MAX_DATA];
    size_t data_count;
    int address[CV_MAX_DATA];
    size_t address_count;
    CvCtl pcs, rd, wr, rw, byte_sel, oe;
    int has_pcs, has_rd, has_wr, has_rw, has_byte, has_oe;
    int has_ale;
    CvAle ale;
    int clock;
    int clock_edge;         /* 1 rise, 0 fall, 2 both */
    const CvClockOut *clock_out;
    size_t clock_out_count;
} CvBus;

/* ---- Protocol ----------------------------------------------------------------------------- */

typedef enum { CVR_NONE, CVR_PATTERN, CVR_POINTER, CVR_MAP } CvReadKind;

typedef struct {
    uint32_t key;           /* address value; with byte keys 1 high / 0 low */
    CvPattern pattern;
} CvReadEntry;

typedef struct {
    CvReadKind kind;
    CvPattern pattern;
    const CvReadEntry *map;
    size_t map_count;
    int byte_keys;
} CvRead;

typedef struct {
    CvConds when;
    CvPattern pattern;      /* outs */
    CvRead read;            /* reads */
} CvAlt;

typedef struct {
    CvCond cond;
    uint8_t action;
} CvTrigger;

typedef struct {
    int stream;
    int align_end;
    CvPattern out;          /* count 0: zeros */
    int idle;               /* '0', '1', 'z', 'r' (repeat), 0 = the bus default */
    const CvCommand *commands;
    size_t command_count;
    CvRead read;
    const CvAlt *outs, *reads;
    size_t out_alt_count, read_alt_count;
    const CvTrigger *triggers;
    size_t trigger_count;
    int general_reset;
} CvProtocol;

/* ---- Conversion, ready, pins, DAC, code format ------------------------------------------- */

enum { CVS_CS_FALL = 1, CVS_CS_RISE = 2, CVS_COMMAND = 4, CVS_PIN = 8, CVS_READ = 16,
       CVS_POWER_ON = 32 };

typedef struct {
    int any;                /* "write:REG": any write to register `reg` */
    int reg;
    CvCond cond;
} CvWriteTrigger;

typedef struct {
    const CvField *fields;
    size_t field_count;
    const uint32_t *keys;   /* count * field_count */
    const uint64_t *ps;
    size_t count;
    uint64_t fixed_ps;
    uint64_t max_ps;
} CvTime;

typedef struct {
    int present;
    CvTime time;
    unsigned starts;
    const CvWriteTrigger *writes;
    size_t write_count;
    int mode;               /* 0 single, 1 continuous, 2 by a field */
    CvField mode_field;
    int mode_single;        /* field form: the value names the single state */
    uint32_t mode_value;
    int start_on_enter;
    const CvSet *busy, *done, *on_read;
    size_t busy_count, done_count, on_read_count;
    int has_channel;
    CvField channel;
} CvConversion;

enum { CVC_READ = 1, CVC_START = 2, CVC_CS_FALL = 4, CVC_PULSE = 8 };

typedef struct {
    int pin;
    int active;
    int asserts_done;
    unsigned clears;
    uint64_t pulse_ps;
    int open_drain;
    int on_miso;
    CvConds enable;
} CvReady;

typedef struct {
    int pin;
    int active;
    int edge;               /* -1 level, 1 rising, 0 falling */
} CvPinCtl;

typedef struct {
    int pin;
    CvField field;
} CvLevel;

typedef struct {
    int reg;
    uint32_t value;
} CvRegValue;

typedef struct {
    int pin;
    int level;
    const CvRegValue *values;
    size_t count;
} CvResetSelect;

enum { CVP_RESET, CVP_CONVST, CVP_START, CVP_POWERDOWN, CVP_LDAC, CVP_CLEAR, CVP_COUNT };

typedef struct {
    int has[CVP_COUNT];
    CvPinCtl ctl[CVP_COUNT];
    const CvLevel *levels;
    size_t level_count;
    const CvResetSelect *reset_select;
    size_t reset_select_count;
} CvPins;

/* A value chosen by a pin or a field (code format, clear code). */
typedef struct {
    int pin;                /* name id or CV_NONE */
    int has_field;
    CvField field;
    const uint32_t *keys;
    const uint32_t *values;
    size_t count;
    uint32_t fixed;         /* when neither pin nor field, or no key matches: values[0] */
} CvChoice;

typedef struct {
    const int *input, *output;
    size_t count;
    CvChoice clear_code;
    int ldac;               /* 0 pin, 1 command, 2 sync */
} CvDac;

enum { CVF_STRAIGHT, CVF_TWOS, CVF_OFFSET };

typedef struct {
    uint64_t t_valid, t_out, t_float, t_acc, t_df;
} CvTiming;

struct RtcClock;

typedef struct {
    int rtc;
    int dac;
    unsigned bits;
    unsigned channels;
    CvChoice code;          /* CVF_* */
    const char *const *names;   /* pin names the device's keys use */
    size_t name_count;
    const int *analog;      /* name ids */
    size_t analog_count;
    CvBus bus;
    int has_config_bus;
    CvBus config_bus;
    CvTiming timing;
    const CvRegister *regs;
    size_t reg_count;
    size_t space_count;
    CvProtocol protocol;
    CvConversion conversion;
    const CvReady *ready;
    size_t ready_count;
    CvPins pins;
    CvDac dacinfo;
    const struct RtcClock *clock;
    const PartVoltage *vcc;     /* the device's digital supply (parts may override) */
    PartConverter info;
} CvDevice;

/* Pin indices of one pinout, per name id (SIZE_MAX: the pinout has no such pin). */
typedef struct {
    const CvDevice *dev;
    const size_t *pin;
    const PartPin *pins;
    size_t pin_count;
    const uint8_t *analog;      /* per pin: 1 analog */
    const uint8_t *open_drain;  /* per pin: 1 open drain (SDA, open-drain ready pins) */
} CvBound;

/* Per-part data: the I2C address override and register reset overrides. */
typedef struct {
    int i2c_address;            /* CV_NONE: the bus table */
    int has_bus_i2c;
    int straps[SERIAL_I2C_STRAPS];
    unsigned strap_count;
    int table[SERIAL_I2C_TABLE];
    unsigned table_count;
    const CvRegValue *resets;
    size_t reset_count;
} CvPart;

/* converter_load.c */
CvDevice *cv_load_device(KindContext *ctx, const cJSON *json, int rtc);
void *cv_bind_pinout(KindContext *ctx, const void *device, const PartPin *pins, size_t pin_count);
int cv_load_part(KindContext *ctx, const void *device, const void *bound, const cJSON *json,
                 Part *out);

extern const SimComponentType CONVERTER_COMPONENT;

#endif
