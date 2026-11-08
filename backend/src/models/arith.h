#ifndef ARITH_H
#define ARITH_H

/*
 * arith: comparators, adders, ALUs, parity generators and Wallace trees
 * (plan-22). arith.c holds the kind and the models `compare` and `parity`;
 * arith_alu.c the models `adder`, `alu181` and `wallace` (plan-22-C).
 * Signal rules: docs/parts-format.md "arith".
 */

#include <stddef.h>
#include <stdint.h>

#include "kind.h"
#include "seq.h"

/* Unknown input bits enumerated before every output goes X (plan-22-A 3.3). */
#define ARITH_MAX_UNKNOWN 8
#define ARITH_MAX_OUTPUTS 8

typedef struct ArithModel ArithModel;

/* Every model's run state starts with this. */
typedef struct {
    const ArithModel *model;
    const SeqTiming *timing;
} ArithHeader;

struct ArithModel {
    const char *name;
    const char *const *keys;
    void *(*load)(KindContext *ctx, const cJSON *json);
    void *(*bind)(KindContext *ctx, const void *dev, const PartPin *pins, size_t count);
    ArithHeader *(*create)(const void *bound);
    /* pin: the pin that changed (SEQ_NO_PIN at start). */
    void (*eval)(Sim *sim, SimInstance *inst, ArithHeader *state, size_t pin);
};

/* The models of arith_alu.c (plan-22-C). */
extern const ArithModel ARITH_ADDER, ARITH_ALU181, ARITH_WALLACE;

/*
 * Output pins: the device's "output" style, or open drain for a pin named in
 * "openDrain". Bound per pinout; outputs[] is filled by the models in the
 * order they drive.
 */
typedef struct {
    size_t pins[ARITH_MAX_OUTPUTS];
    SeqOutputStyle style[ARITH_MAX_OUTPUTS];
    size_t count;
} ArithOutputs;

typedef struct {
    SeqOutput o[ARITH_MAX_OUTPUTS];
} ArithDrive;

/* Device-wide output style keys: "output" and "openDrain" (names, loaded). */
typedef struct {
    SeqOutputStyle style;
    const char *const *open_drain;
    size_t open_drain_count;
} ArithStyle;

/*
 * A combinational function of up to 32 input bits giving up to 32 output
 * bits (the pin levels the model drives).
 */
typedef uint32_t (*ArithFn)(const void *bound, uint32_t inputs);

/*
 * The outputs for the input_count input bits whose known bits are `known`: every completion of
 * the unknown bits is evaluated (at most ARITH_MAX_UNKNOWN of them, else
 * every output is unknown); an output bit is known when every completion
 * gives the same value. *out_known gets the known mask (limited to `outputs`
 * bits).
 */
uint32_t arith_eval_unknown(ArithFn fn, const void *bound, uint32_t inputs, uint32_t known,
                            unsigned input_count, unsigned outputs, uint32_t *out_known);

/* Loads "output" and "openDrain". */
int arith_load_style(KindContext *ctx, const cJSON *json, ArithStyle *style);

/* Adds one output pin by name (NULL: none) with the device's style. */
int arith_bind_output(KindContext *ctx, const PartPin *pins, size_t count, const ArithStyle *style,
                      const char *name, ArithOutputs *out);

void arith_init_drive(const ArithOutputs *outs, ArithDrive *drive);

/* Drives output i with v. */
void arith_drive(Sim *sim, SimInstance *inst, const ArithOutputs *outs, ArithDrive *drive,
                 size_t i, LogicValue v, const SeqTiming *t);

/* Reads the pins into bits first.. of *value / *known (Z and X unknown). */
void arith_read_bits(const Sim *sim, const SimInstance *inst, const size_t *pins, size_t n,
                     unsigned first, uint32_t *value, uint32_t *known);

/* Pin names -> indices; n names. */
int arith_bind_names(KindContext *ctx, const PartPin *pins, size_t count,
                     const char *const *names, size_t n, size_t *out);

/* Output bit i of (value, known) as a LogicValue. */
LogicValue arith_bit(uint32_t value, uint32_t known, unsigned i);

#endif
