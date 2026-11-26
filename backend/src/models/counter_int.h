/* models/counter_int.h: what the pieces of models/counter.c share (plan-68-I/J split it). */
#ifndef MODELS_COUNTER_INT_H
#define MODELS_COUNTER_INT_H

/*
 * counter: counters and dividers (plan-10-D section 5). One kind; a device
 * names its model with "model":
 *
 *   sync     synchronous counters, with the options of the register
 *            counters (590, 592, 593, 697, 8154) and the 4522B zero latch
 *   ripple   ripple (asynchronous) counters: binary chains, the 74x90/92/390
 *            divide-by-2/5/6 sections
 *
 *   johnson, seg7, divn, rate   counter_div.c (plan-10-E)
 *
 * Counts are SeqBits (value + known mask), so an X input makes X exactly the
 * bits it could change: wherever a control is unknown, both outcomes are
 * computed and merged. Signal rules: docs/parts-format.md "Sequential kinds".
 */

#include <stdlib.h>
#include <string.h>

#include "counter.h"
#include "kind.h"
#include "model_pins.h"
#include "seq.h"

typedef struct {
    const CounterModel *model;
    void *dev;
    SeqTimingTable timing;
    CounterTiming *timings;   /* parallel to timing.entries */
} CounterDevice;
/* ---- sync ----------------------------------------------------------------- */

#define SYNC_MAX_UNITS 2
#define SYNC_MAX_STAGES 32
#define SYNC_MAX_LIST 3

enum { AT_MAX, AT_MIN, AT_TERMINAL };
enum { M_CLEAR, M_DOWN, M_LOAD, M_UP, M_HOLD };

typedef struct {
    SeqControlName c;
    int to_max;
} SyncClearName;

typedef struct {
    SeqControlName c;
    int from_register;
} SyncLoadName;

typedef struct {
    SeqControlName c;
    int at;
    SeqControlName gates[SYNC_MAX_LIST];
    size_t gate_count;
    const char *clock_low;
} SyncCarryName;

typedef struct {
    unsigned stages;
    int bcd;
    const char *code_pin;   /* codeSelect */
    int code_binary_high;
    const char *const *q;
    size_t q_count;
    SeqClockName clock;
    const char *clock_up, *clock_down;
    SeqGateName gate;
    int count;              /* 1 up, -1 down, 0 from direction/modes/dual clocks */
    SeqControlName direction;   /* active_high = "up" is high */
    SeqControlName enables[SYNC_MAX_LIST];
    size_t enable_count;
    SeqControlName any[SYNC_MAX_LIST];
    size_t any_count;
    SyncClearName clears[SYNC_MAX_LIST];
    size_t clear_count;
    SyncLoadName loads[SYNC_MAX_LIST];
    size_t load_count;
    const char *const *data;
    size_t data_count;
    int has_modes;
    const char *mode_pins[2];
    int mode_table[4];
    int mode_clear_async;
    int has_illegal;
    unsigned char illegal_up[16], illegal_down[16];
    SyncCarryName carries[SYNC_MAX_LIST];
    size_t carry_count;
    SeqControlName oe;
    int has_register;
    SeqClockName reg_clock;
    const char *const *reg_outputs;
    size_t reg_output_count;
    SeqControlName reg_oe;
    const char *reg_select;
    int reg_select_high;
    int has_input_register;
    SeqClockName ireg_clock;
    const char *const *ireg_data;
    size_t ireg_data_count;
    SeqControlName ireg_enable;
    int has_bus;
    const char *const *bus;
    size_t bus_count;
    SeqControlName bus_oe[SYNC_MAX_LIST];
    size_t bus_oe_count;
    const char *zero_pin, *zero_cascade;
} SyncUnitDef;

typedef struct {
    const char *pin;
    unsigned unit;
    unsigned byte;
} SyncSelectName;

typedef struct {
    SyncUnitDef units[SYNC_MAX_UNITS];
    size_t unit_count;
    int has_byte_bus;
    const char *const *byte_outputs;
    size_t byte_output_count;
    SyncSelectName selects[4];
    size_t select_count;
} SyncDevice;

typedef struct {
    SeqControl c;
    int to_max;
} SyncClear;

typedef struct {
    SeqControl c;
    int from_register;
} SyncLoad;

typedef struct {
    SeqControl c;
    int at;
    SeqControl gates[SYNC_MAX_LIST];
    size_t gate_count;
    size_t clock_low;
} SyncCarry;

typedef struct {
    const SyncUnitDef *def;
    size_t code_pin;
    size_t q[SYNC_MAX_STAGES];
    SeqClock clock;
    size_t clock_up, clock_down;
    SeqGate gate;
    SeqControl direction;
    SeqControl enables[SYNC_MAX_LIST], any[SYNC_MAX_LIST];
    SyncClear clears[SYNC_MAX_LIST];
    SyncLoad loads[SYNC_MAX_LIST];
    size_t data[SYNC_MAX_STAGES];
    size_t mode_pins[2];
    SyncCarry carries[SYNC_MAX_LIST];
    SeqControl oe;
    SeqClock reg_clock;
    size_t reg_outputs[SYNC_MAX_STAGES];
    SeqControl reg_oe;
    size_t reg_select;
    SeqClock ireg_clock;
    size_t ireg_data[SYNC_MAX_STAGES];
    SeqControl ireg_enable;
    size_t bus[SYNC_MAX_STAGES];
    SeqControl bus_oe[SYNC_MAX_LIST];
    size_t zero_pin, zero_cascade;
} SyncUnit;

typedef struct {
    size_t pin;
    unsigned unit;
    unsigned byte;
} SyncSelect;

typedef struct {
    const SyncDevice *dev;
    SyncUnit units[SYNC_MAX_UNITS];
    size_t byte_outputs[8];
    SyncSelect selects[4];
} SyncBound;

typedef struct {
    SeqBits count, reg, ireg;
    LogicValue last_clock, last_up, last_down, last_reg_clock, last_ireg_clock;
    LogicValue zero;
    SeqOutput q[SYNC_MAX_STAGES];
    SeqOutput carries[SYNC_MAX_LIST];
    SeqOutput reg_outputs[SYNC_MAX_STAGES];
    SeqOutput bus[SYNC_MAX_STAGES];
    SeqOutput zero_out;
} SyncUnitState;

typedef struct {
    CounterHeader h;
    const SyncBound *b;
    SyncUnitState units[SYNC_MAX_UNITS];
    SeqOutput byte_outputs[8];
} SyncState;

typedef struct {
    const Sim *sim;
    const SimInstance *inst;
    const SyncUnit *u;
    SyncUnitState *us;
    SeqBits cur;
    int edge;       /* 1, 0, -1: the effective clock had its edge */
    int dual_up;    /* dual clocks: direction of the edge (1 up, 0 down, -1 unknown) */
} SyncEval;
/* ---- ripple ------------------------------------------------------------------ */

#define RIPPLE_MAX_SECTIONS 4
#define RIPPLE_MAX_STAGES 14

enum { R_BINARY, R_DIV2, R_DIV5, R_DIV6 };

typedef struct {
    const char *const *pins;
    size_t count;
    int active_high;
    unsigned value;
} RippleCtlName;

typedef struct {
    const char *clock;
    int structure;
    unsigned stages;
    const char *q[RIPPLE_MAX_STAGES];   /* NULL = no pin */
    RippleCtlName reset, set;
    int has_reset, has_set;
} RippleSectionDef;

typedef struct {
    RippleSectionDef sections[RIPPLE_MAX_SECTIONS];
    size_t section_count;
} RippleDevice;

typedef struct {
    size_t pins[4];
    size_t count;
    int active_high;
} RippleCtl;

typedef struct {
    const RippleSectionDef *def;
    size_t clock;
    size_t q[RIPPLE_MAX_STAGES];
    RippleCtl reset, set;
} RippleSection;

typedef struct {
    RippleSection sections[RIPPLE_MAX_SECTIONS];
    size_t section_count;
} RippleBound;

typedef struct {
    CounterHeader h;
    const RippleBound *b;
    SeqBits value[RIPPLE_MAX_SECTIONS];
    LogicValue last_clock[RIPPLE_MAX_SECTIONS];
    SeqOutput q[RIPPLE_MAX_SECTIONS][RIPPLE_MAX_STAGES];
} RippleState;

uint64_t cnt_mask_of(unsigned n);
SeqBits cnt_known_bits(uint64_t value, unsigned n);
int cnt_bits_equal(SeqBits b, uint64_t value, unsigned n);
int cnt_lo(int level);
int cnt_hi(int level);
int cnt_and3(int a, int b);
int cnt_not3(int a);
int cnt_or3(int a, int b);
int cnt_bind_names(KindContext *ctx, const PartPin *pins, size_t count,
                      const char *const *names, size_t n, size_t *out);
void cnt_init_outputs(SeqOutput *outputs, const size_t *pins, size_t n);
int cnt_control_list(KindContext *ctx, const cJSON *json, const char *key, int allow_when,
                        SeqControlName *out, size_t max, size_t *count);
SeqBits cnt_step_binary(SeqBits cur, unsigned stages, int up);
extern const CounterModel cnt_SYNC_MODEL;
void *cnt_sync_load(KindContext *ctx, const cJSON *json, unsigned *units_out);
void *cnt_sync_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count);
CounterHeader *cnt_sync_create(const void *dev_raw, const void *bound);
extern const CounterModel cnt_RIPPLE_MODEL;

#endif
