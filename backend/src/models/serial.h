#ifndef SERIAL_H
#define SERIAL_H

/*
 * The serial kind's internal interface (plan-12-C section 3.1): UARTs,
 * USARTs, ACIAs, I2C/SPI-to-UART bridges and the SPI-to-I2C bridge. One
 * PartKindOps and one SimComponentType (SERIAL_MODEL in serial.c); the
 * device's "model" key picks a SerialModel, which supplies its device keys,
 * binding and engine callbacks. Every model's state starts with SerialBase.
 */

#include "kind.h"
#include "serial_bus.h"
#include "serial_line.h"

enum {
    SERIAL_MAX_TIMING = 48,
    SERIAL_MAX_CHANNELS = 8
};

/* One grade: the model's timing values (picoseconds, in the order of
   SerialModel.timing), the facts and the grade's voltage keys. */
typedef struct {
    SimTime t[SERIAL_MAX_TIMING];
    PartSerial info;
    const PartVoltage *vcc;   /* NULL = none */
} SerialGrade;

typedef struct SerialModel SerialModel;

typedef struct {
    const SerialModel *model;
    const void *data;         /* the model's device data */
    unsigned channels;
    const char **grade_names;
    SerialGrade *grades;
    size_t grade_count;
} SerialDevice;

/* What a part hands its model: the grade and the model's part options. */
typedef struct {
    const SerialGrade *grade;
    const void *options;      /* SerialModel.load_part's result; NULL = none */
} SerialPartData;

struct SerialModel {
    const char *name;
    const char *const *keys;          /* device keys besides "model" and "grades" */
    const char *const *timing;        /* timing names (ns in JSON), NULL-terminated */
    const char *const *part_keys;     /* part keys besides grade/vcc/inputMax; may be NULL */
    /* Device keys; sets *channels (1 or more). NULL on error. */
    void *(*load_device)(KindContext *ctx, const cJSON *json, unsigned *channels);
    /* Pin names to indices for one pinout. NULL on error. */
    void *(*bind)(KindContext *ctx, const void *data, const PartPin *pins, size_t pin_count);
    /* Optional: part-level options. NULL result with ctx failed = error. */
    const void *(*load_part)(KindContext *ctx, const void *data, const cJSON *json);
    /* Optional: 1 when the pin is open-drain (voltage check). */
    int (*pin_open_drain)(const void *data, const void *pins, const void *options, size_t pin);

    void *(*create)(const Part *part, const void *data, const void *pins, const SerialGrade *grade,
                    const void *options);
    void (*destroy)(void *state);
    void (*start)(Sim *sim, SimInstance *instance, void *state);
    void (*pin_changed)(Sim *sim, SimInstance *instance, void *state, size_t pin,
                        LogicValue old_value, LogicValue new_value);
    void (*timer)(Sim *sim, SimInstance *instance, void *state, uint32_t timer_id);

    /* Terminal (plan-12-A section 3.7). */
    unsigned (*channels)(const void *state);
    SerialLine *(*line)(void *state, unsigned channel);
    /* 1 when the channel is in an asynchronous mode now (the terminal can type into it). */
    int (*async)(const void *state, unsigned channel);
};

typedef struct {
    const SerialModel *model;
} SerialBase;

/* The models (serial.c's table). */
extern const SerialModel NS16550_MODEL;

/* The pin a channel's terminal input drives (its RX pin), or SIZE_MAX. For
   circuit_serial_send's driven-net check. */
size_t serial_rx_pin(const void *state, unsigned channel);

/* ---- Shared helpers for models ------------------------------------------------ */

/* An optional pin name of an object: *out NULL when absent. */
int serial_optional_name(KindContext *ctx, const cJSON *object, const char *key, const char **out);

/* A pin index for an optional name: SERIAL_NO_PIN when name is NULL. */
int serial_bind_pin(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *key,
                    const char *name, size_t *out);

/* Pins a package may leave out: tied inside at a level, or just absent. */
typedef struct {
    const char *const *tie_names;
    const int *tie_levels;
    size_t tie_count;
    const char *const *optional;
    size_t optional_count;
} SerialAbsent;

/* As serial_bind_pin, but a name missing from this pinout is SERIAL_TIE_0/1
   when `absent` ties it, SERIAL_NO_PIN when it lists it as optional, else
   an error. */
int serial_bind_pin_absent(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *key,
                           const char *name, const SerialAbsent *absent, size_t *out);

#endif
