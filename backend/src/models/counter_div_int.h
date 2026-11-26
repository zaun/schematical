/* models/counter_div_int.h: what the pieces of models/counter_div.c share (plan-68-I/J split it). */
#ifndef MODELS_COUNTER_DIV_INT_H
#define MODELS_COUNTER_DIV_INT_H

/*
 * counter_div: the counter kind's dividing models (plan-10-E section 4).
 *
 *   johnson  Johnson (twisted-ring) counters with decoded outputs and the
 *            anti-lock gate D3 = Q2.(Q1 + Q3): 4017B, 4022B, 74HC4017, 4018B
 *   seg7     decade counters with 7-segment outputs: 4026B, 4033B, 40110B
 *   divn     programmable divide-by-N (master preset, extra count): 4059A
 *   rate     rate multipliers: 4089B, 4527B, 74x97
 *
 * counter.c holds the kind and lists these models (counter.h).
 */

#include <stdlib.h>
#include <string.h>

#include "counter.h"
#include "kind.h"
#include "seq.h"

/* ---- johnson ---------------------------------------------------------------- */

#define JOHNSON_MAX 5

typedef struct {
    unsigned stages;
    const char *clock;
    SeqGateName gate;
    SeqControlName reset, preset;
    const char *const *outputs;
    size_t output_count;
    const char *carry, *data;
    const char *const *jam;
    size_t jam_count;
    const char *const *qbar;
    size_t qbar_count;
} JohnsonDevice;

typedef struct {
    const JohnsonDevice *dev;
    size_t clock;
    SeqGate gate;
    SeqControl reset, preset;
    size_t outputs[2 * JOHNSON_MAX];
    size_t carry, data;
    size_t jam[JOHNSON_MAX];
    size_t qbar[JOHNSON_MAX];
} JohnsonBound;

typedef struct {
    CounterHeader h;
    const JohnsonBound *b;
    int q[JOHNSON_MAX];          /* stage levels 1 / 0 / -1 */
    LogicValue last_clock;
    SeqOutput outputs[2 * JOHNSON_MAX];
    SeqOutput carry;
    SeqOutput qbar[JOHNSON_MAX];
} JohnsonState;

typedef struct {
    const char *clock, *latch, *output;
    const char *const *jam;
    size_t jam_count;
    const char *const *modes;
    size_t mode_count;
} DivnDevice;

typedef struct {
    size_t clock, latch, output;
    size_t jam[16];
    size_t modes[3];
} DivnBound;

typedef struct {
    CounterHeader h;
    const DivnBound *b;
    long count;        /* C; -1 unknown */
    int preset;        /* the extra count after a master preset is pending */
    int pulse;         /* 1 / 0 / -1 for the current clock period */
    int latched;       /* latch enable has caught a pulse */
    LogicValue last_clock;
    SeqOutput out;
} DivnState;

enum { RATE_BINARY4, RATE_BCD, RATE_BINARY6 };

typedef struct {
    int variant;
    unsigned bits;       /* rate pins */
    int max;             /* 15, 9, 63 */
    const char *clock;
    const char *const *rate;
    size_t rate_count;
    SeqControlName enable, strobe, cascade, clear, set;
    const char *out, *out_bar, *terminal, *enable_out;
} RateDevice;

typedef struct {
    const RateDevice *dev;
    size_t clock;
    size_t rate[6];
    SeqControl enable, strobe, cascade, clear, set;
    size_t out, out_bar, terminal, enable_out;
} RateBound;

typedef struct {
    CounterHeader h;
    const RateBound *b;
    int count;   /* -1 unknown */
    LogicValue last_clock;
    SeqOutput out, out_bar, terminal, enable_out;
} RateState;

#define SEG7_MAX_TOGGLES 4

enum { SEG7_TIMER_CARRY = 1, SEG7_TIMER_BORROW = 2 };

typedef struct {
    const char *clock, *clock_up, *clock_down;
    SeqGateName gate;
    SeqControlName reset, lamp_test, toggle[SEG7_MAX_TOGGLES], latch;
    size_t toggle_count;
    const char *const *segments;
    size_t segment_count;
    unsigned char glyphs[10];
    const char *carry, *carry_pulse, *borrow_pulse;
    const char *de_in, *de_out, *ungated_c, *rb_in, *rb_out;
    /* plan-56-F (SN74143): all optional */
    int segments_high;                 /* segmentsActive; 1 = a lit segment is a high pin */
    SeqOutputStyle segments_style;     /* segmentsOutput */
    const char *max_pin;
    SeqControlName max_enable;
    const char *const *latch_outputs;  /* 4 names */
    SeqControlName blank;
    const char *rbi, *rbo, *dp_in, *dp_out;
} Seg7Device;

typedef struct {
    const Seg7Device *dev;
    size_t clock, clock_up, clock_down;
    SeqGate gate;
    SeqControl reset, lamp_test, toggle[SEG7_MAX_TOGGLES], latch;
    size_t segments[7];
    size_t carry, carry_pulse, borrow_pulse, de_in, de_out, ungated_c, rb_in, rb_out;
    size_t max_pin, latch_outputs[4], rbi, rbo, dp_in, dp_out;
    SeqControl max_enable, blank;
} Seg7Bound;

typedef struct {
    CounterHeader h;
    const Seg7Bound *b;
    int count;         /* 0-9, -1 unknown */
    int shown;         /* the count the segments show (latchEnable) */
    LogicValue last_clock, last_up, last_down;
    uint32_t carry_gen, borrow_gen;
    SeqOutput segments[7], carry, carry_pulse, borrow_pulse, de_out, ungated_c, rb_out;
    SeqOutput max_out, latch_out[4], rbo, dp_out;
} Seg7State;

int cdv_and3(int a, int b);
int cdv_or3(int a, int b);
int cdv_not3(int a);
int cdv_bind_names(KindContext *ctx, const PartPin *pins, size_t count,
                      const char *const *names, size_t n, size_t *out);
void cdv_init_outputs(SeqOutput *outputs, const size_t *pins, size_t n);
void cdv_drive(Sim *sim, SimInstance *inst, SeqOutput *o, int level, const CounterTiming *t);
int cdv_pin_pair(KindContext *ctx, const cJSON *json, const char *key, const char **in,
                    const char **out);

#endif
