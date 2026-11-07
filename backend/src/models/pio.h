#ifndef PIO_H
#define PIO_H

/*
 * The pio kind's internal interface (plan-14-A section 4.2): parallel I/O
 * peripherals on the 65xx bus (the W65C21 PIA, the W65C22 VIA). One
 * PartKindOps and one SimComponentType (in pio.c); the device's "model" key
 * picks a PioModel, which supplies its device keys, binding and engine
 * callbacks. Every model's state starts with PioBase; every model talks to
 * the bus through bus65.h.
 */

#include "bus65.h"
#include "kind.h"

enum { PIO_MAX_TIMING = 48 };

/* One grade: the model's timing values (picoseconds, in the order of
   PioModel.timing), the facts and the grade's voltage keys. */
typedef struct {
    SimTime t[PIO_MAX_TIMING];
    PartPio info;
    const PartVoltage *vcc;   /* NULL = none */
} PioGrade;

typedef struct PioModel PioModel;

typedef struct {
    const PioModel *model;
    const void *data;         /* the model's device data */
    const char **grade_names;
    PioGrade *grades;
    size_t grade_count;
} PioDevice;

struct PioModel {
    const char *name;
    const char *const *keys;          /* device keys besides "model" and "grades" */
    const char *const *timing;        /* timing names (ns in JSON), NULL-terminated; the first is tcyc */
    unsigned positive;                /* the first `positive` timing names must be > 0 */
    /* Device keys. NULL on error. */
    void *(*load_device)(KindContext *ctx, const cJSON *json);
    /* Pin names to indices for one pinout. NULL on error. */
    void *(*bind)(KindContext *ctx, const void *data, const PartPin *pins, size_t pin_count);
    /* Optional: 1 when the pin is open-drain (voltage check). */
    int (*pin_open_drain)(const void *data, const void *pins, size_t pin);

    void *(*create)(const Part *part, const void *data, const void *pins, const PioGrade *grade);
    void (*destroy)(void *state);
    void (*start)(Sim *sim, SimInstance *instance, void *state);
    void (*pin_changed)(Sim *sim, SimInstance *instance, void *state, size_t pin,
                        LogicValue old_value, LogicValue new_value);
    void (*timer)(Sim *sim, SimInstance *instance, void *state, uint32_t timer_id);
};

typedef struct {
    const PioModel *model;
} PioBase;

/* The models (pio.c's table). */
extern const PioModel PIA6521_MODEL;

/* ---- Shared helpers: the 65xx bus and the two ports ------------------------ */

enum { PIO_PORT_BITS = 8 };

/* The pin-name keys every pio model has: "phi2", "rw", "reset", "data" (8),
   "csHigh", "csLow", "rs", "pa" (8), "pb" (8), "ca1", "ca2", "cb1", "cb2". */
typedef struct {
    const char *phi2, *rw, *reset;
    const char *const *data;
    const char *const *cs_high, *const *cs_low, *const *rs;
    size_t cs_high_count, cs_low_count, rs_count;
    const char *const *pa, *const *pb;
    const char *ca1, *ca2, *cb1, *cb2;
} PioNames;

typedef struct {
    Bus65Pins bus;
    size_t reset;
    size_t pa[PIO_PORT_BITS], pb[PIO_PORT_BITS];
    size_t ca1, ca2, cb1, cb2;
} PioPins;

/* Reads the shared keys; the lists must have exactly these counts. */
int pio_load_names(KindContext *ctx, const cJSON *json, PioNames *names, size_t cs_high_count,
                   size_t cs_low_count, size_t rs_count);

/* Binds the shared names to one pinout. */
int pio_bind_names(KindContext *ctx, const PioNames *names, const PartPin *pins, size_t pin_count,
                   PioPins *out);

/* One pin name to its index, with the key in the error path. */
int pio_bind_pin(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *key,
                 const char *name, size_t *out);

#endif
