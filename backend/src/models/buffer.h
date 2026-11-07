#ifndef BUFFER_H
#define BUFFER_H

/*
 * The buffer kind's shared pieces (plan-20-D section 2): the model table
 * and the helpers its models share. buffer.c holds the kind and the
 * tristate and transceiver models; buffer_reg.c the registered model.
 *
 * Every model keeps its run state behind a BufHeader. Ports are read with
 * buf_port: a pin the part drove until a moment ago reads X until its own
 * drive has let go, so a transceiver never reads back its own output when
 * its direction turns around (plan-20-A section 3.5).
 */

#include "kind.h"
#include "seq.h"

#define BUF_MAX_UNITS 8
#define BUF_MAX_BITS 40     /* bits per unit */
#define BUF_MAX_OE 4        /* enables that must all be asserted */
#define BUF_TOTAL_BITS 64   /* bits per part */

typedef struct BufModel BufModel;

typedef struct {
    const BufModel *model;
    const BufModel *inner;  /* translator: the transceiver or tristate model it runs */
    const SeqTiming *timing;
    SeqOutputStyle style;
    size_t pin_count;
    SimTime *own_until;     /* per pin: when the part's last drive on it lets go */
    int timer_pending;
    SimTime timer_at;
} BufHeader;

struct BufModel {
    const char *name;
    const char *const *keys;
    void *(*load)(KindContext *ctx, const cJSON *json, unsigned *units);
    void *(*bind)(KindContext *ctx, const void *dev, const PartPin *pins, size_t count);
    BufHeader *(*create)(const void *dev, const void *bound);
    /* at_start: the first evaluation (sim start). */
    void (*eval)(Sim *sim, SimInstance *inst, BufHeader *state, int at_start);
    /* 1 when pin is an output (or port) of the part; NULL = no outputs listed. */
    int (*is_output)(const void *dev, const void *bound, size_t pin);
};

/* A list of 0..BUF_MAX_OE controls that must all be asserted. */
typedef struct {
    SeqControlName names[BUF_MAX_OE];
    size_t count;
} BufEnableNames;

typedef struct {
    SeqControl controls[BUF_MAX_OE];
    size_t count;
} BufEnable;

/* Loading. "oe" may be one control object or an array of them; absent and
   not required: no controls (always enabled). */
int buf_enable_names(KindContext *ctx, const cJSON *json, const char *key, int required,
                     BufEnableNames *out);
int buf_bind_enable(KindContext *ctx, const PartPin *pins, size_t count,
                    const BufEnableNames *names, BufEnable *out);
int buf_bind_names(KindContext *ctx, const PartPin *pins, size_t count,
                   const char *const *names, size_t n, size_t *out);

/* Running. 1 all asserted, 0 one deasserted, -1 unknown (X or Z on one). */
int buf_enabled(const Sim *sim, const SimInstance *inst, const BufEnable *e);
/* A data or port pin: 0, 1 or X (Z reads X; X while the part's own drive lets go). */
LogicValue buf_port(const Sim *sim, const SimInstance *inst, const BufHeader *h, size_t pin);
/* seq_drive that also records when a released drive lets go, and asks for
   a re-evaluation then. */
void buf_drive(Sim *sim, SimInstance *inst, BufHeader *h, SeqOutput *o, LogicValue v);
LogicValue buf_invert(LogicValue v, int invert);
/* Every output starts undriven (Z), not scheduled. */
void buf_init_outputs(SeqOutput *outputs, const size_t *pins, size_t n);
/* "units": an array of 1..BUF_MAX_UNITS objects; *n its size. NULL (error set) otherwise. */
const cJSON *buf_units_array(KindContext *ctx, const cJSON *json, size_t *n);
/* A unit's "dir": {"pin": name, "ab": "high"|"low"} (ab: the level meaning A to B). */
int buf_dir_name(KindContext *ctx, const cJSON *unit, const char **pin, int *ab_high);

/* The registered model (buffer_reg.c). */
extern const BufModel BUF_REGISTERED_MODEL;

#endif
