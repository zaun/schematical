#ifndef COUNTER_H
#define COUNTER_H

/*
 * The counter kind's model interface (plan-10-D section 3), internal to the
 * models: counter.c holds the kind, the dispatch and the sync and ripple
 * models; counter_div.c (plan-10-E) adds the rest. A device names its
 * model with "model"; the model's loader reads the device keys, its binder
 * the pins, and its engine callbacks run the part.
 */

#include "kind.h"
#include "seq.h"

#define COUNTER_MAX_EXTRA 4

/* A model's extra timing key (ns), e.g. ripple's "stage". */
typedef struct {
    const char *key;
    int required;
} CounterExtraKey;

/* One timing entry with its extra keys, in the order the model lists them. */
typedef struct {
    const SeqTiming *seq;
    SimTime extra[COUNTER_MAX_EXTRA];
} CounterTiming;

typedef struct CounterModel CounterModel;

/* Every model's run state starts with this. */
typedef struct {
    const CounterModel *model;
    const CounterTiming *timing;
} CounterHeader;

struct CounterModel {
    const char *name;
    const char *const *keys;               /* device keys besides model and timing */
    const CounterExtraKey *extra;          /* extra timing keys, {NULL} terminated; may be NULL */
    void *(*load)(KindContext *ctx, const cJSON *json, unsigned *units);
    void *(*bind)(KindContext *ctx, const void *dev, const PartPin *pins, size_t count);
    CounterHeader *(*create)(const void *dev, const void *bound);
    void (*destroy)(CounterHeader *state);   /* NULL = free() */
    void (*start)(Sim *sim, SimInstance *inst, CounterHeader *state);
    void (*changed)(Sim *sim, SimInstance *inst, CounterHeader *state, size_t pin,
                    LogicValue old_value, LogicValue new_value);
    void (*timer)(Sim *sim, SimInstance *inst, CounterHeader *state, uint32_t id);   /* may be NULL */
};

/* The models of counter_div.c (plan-10-E); counter.c lists them in its table. */
extern const CounterModel JOHNSON_MODEL;
extern const CounterModel SEG7_MODEL;
extern const CounterModel DIVN_MODEL;
extern const CounterModel RATE_MODEL;

/* ---- Shared pieces of the counter models --------------------------------- */

/* Bits merge: a bit stays known only where a and b agree. */
SeqBits counter_merge(SeqBits a, SeqBits b);
/* The pins' levels as bits (X or Z -> unknown); count <= 64. */
SeqBits counter_read_bits(const Sim *sim, const SimInstance *inst, const size_t *pins,
                          size_t count);
/* Drives o with the entry's delays plus extra_ps. */
void counter_drive(Sim *sim, SimInstance *inst, SeqOutput *o, LogicValue v,
                   const CounterTiming *t, SimTime extra_ps);

#endif
