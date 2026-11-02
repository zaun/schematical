#ifndef SHIFT_H
#define SHIFT_H

/*
 * The shift kind's model interface (plan-10-G section 3), internal to the
 * models: shift.c holds the kind, the dispatch and the parametric shift
 * model; shift_special.c (plan-10-H) adds the special ones. Same shape as
 * the counter kind's (counter.h).
 */

#include "kind.h"
#include "seq.h"

#define SHIFT_MAX_EXTRA 4

typedef struct {
    const char *key;
    int required;
} ShiftExtraKey;

/* One timing entry with its extra keys, in the order the model lists them. */
typedef struct {
    const SeqTiming *seq;
    SimTime extra[SHIFT_MAX_EXTRA];
} ShiftTiming;

typedef struct ShiftModel ShiftModel;

/* Every model's run state starts with this. */
typedef struct {
    const ShiftModel *model;
    const ShiftTiming *timing;
} ShiftHeader;

struct ShiftModel {
    const char *name;
    const char *const *keys;
    const ShiftExtraKey *extra;            /* {NULL} terminated; may be NULL */
    void *(*load)(KindContext *ctx, const cJSON *json, unsigned *units);
    void *(*bind)(KindContext *ctx, const void *dev, const PartPin *pins, size_t count);
    ShiftHeader *(*create)(const void *dev, const void *bound);
    void (*start)(Sim *sim, SimInstance *inst, ShiftHeader *state);
    void (*changed)(Sim *sim, SimInstance *inst, ShiftHeader *state, size_t pin,
                    LogicValue old_value, LogicValue new_value);
    void (*timer)(Sim *sim, SimInstance *inst, ShiftHeader *state, uint32_t id);   /* may be NULL */
    int (*open_drain)(const void *dev, const void *bound, size_t pin);              /* may be NULL */
};

/* The models of shift_special.c (plan-10-H); shift.c lists them in its table. */
extern const ShiftModel LS673_MODEL;
extern const ShiftModel TAP64_MODEL;
extern const ShiftModel BUSREG_MODEL;
extern const ShiftModel FIFO_MODEL;
extern const ShiftModel SERIN_MODEL;

/* ---- Shared pieces ----------------------------------------------------------- */

SeqBits shift_merge(SeqBits a, SeqBits b);
SeqBits shift_read_bits(const Sim *sim, const SimInstance *inst, const size_t *pins, size_t n);
void shift_drive(Sim *sim, SimInstance *inst, SeqOutput *o, LogicValue v, SeqOutputStyle style,
                 const ShiftTiming *t);

#endif
