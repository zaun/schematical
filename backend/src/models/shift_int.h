/* models/shift_int.h: what the pieces of models/shift.c share (plan-68-I/J split it). */
#ifndef MODELS_SHIFT_INT_H
#define MODELS_SHIFT_INT_H

/*
 * shift: shift registers (plan-10-G). One kind; a device names its model
 * with "model". This file holds the kind, the dispatch and the parametric
 * "shift" model: serial and parallel inputs, mode rows, a storage register
 * or latch, extra serial-output flip-flops and shared I/O pins. The special
 * models (ls673, tap64, bus-register, fifo, serial-in) are in
 * shift_special.c (plan-10-H).
 *
 * Stages, storage and the input register are SeqBits (bit k = stage k+1);
 * wherever a control is unknown both outcomes are computed and merged.
 * Signal rules: docs/parts-format.md "Sequential kinds".
 */

#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "model_pins.h"
#include "seq.h"
#include "shift.h"

/* ---- The shift model ------------------------------------------------------------- */

#define SH_MAX_UNITS 2
#define SH_MAX_STAGES 64
#define SH_MAX_LIST 3
#define SH_MAX_OUTPUTS 8

enum { SERIAL_AND, SERIAL_JK, SERIAL_SELECT };
enum { ROW_HOLD, ROW_RIGHT, ROW_LEFT, ROW_LOAD };
enum { FROM_DATA, FROM_IREG, FROM_IO };

typedef struct {
    unsigned stage;   /* 0-based */
    const char *pin;
    int invert;
} ShOutputName;

typedef struct {
    unsigned stages;
    SeqClockName clock;
    SeqGateName gate;
    int serial_kind;
    const char *const *serial;
    size_t serial_count;
    const char *jk_j, *jk_kbar;
    const char *sel, *sel_low, *sel_high;
    const char *left_serial;
    int has_modes;
    const char *mode_pins[2];
    int mode_table[4];
    SeqControlName enables[SH_MAX_LIST];
    size_t enable_count;
    int has_load;
    SeqControlName load;
    int load_from;
    const char *const *data;
    size_t data_count;
    SeqControlName clear;
    int clear_all;
    ShOutputName outputs[SH_MAX_OUTPUTS];
    size_t output_count;
    SeqControlName true_comp;
    int has_io;
    const char *const *io;
    size_t io_count;
    SeqControlName io_oe[SH_MAX_LIST];
    size_t io_oe_count;
    unsigned io_not_rows;   /* bit r: the io pins are not driven in mode row r */
    ShOutputName serial_outputs[2];
    size_t serial_output_count;
    int has_storage;
    SeqClockName st_clock;
    SeqControlName st_latch, st_clear, st_oe, st_blank, st_all_on, st_polarity;
    const char *st_supply;   /* the outputs' own supply pin (VPP, VBB), or NULL */
    const char *const *st_outputs;
    size_t st_output_count;
    SeqOutputStyle st_style;
    int st_invert;
    int has_ireg;
    SeqClockName ireg_clock;
    const char *const *ireg_data;
    size_t ireg_data_count;
    const char *clock_out;
} ShUnitDef;

typedef struct {
    ShUnitDef units[SH_MAX_UNITS];
    size_t unit_count;
} ShDevice;

typedef struct {
    const ShUnitDef *def;
    SeqClock clock;
    SeqGate gate;
    size_t serial[SH_MAX_LIST];
    size_t jk_j, jk_kbar, sel, sel_low, sel_high, left_serial;
    size_t mode_pins[2];
    SeqControl enables[SH_MAX_LIST];
    SeqControl load, clear, true_comp;
    size_t data[SH_MAX_STAGES];
    size_t outputs[SH_MAX_OUTPUTS];
    size_t io[SH_MAX_STAGES];
    SeqControl io_oe[SH_MAX_LIST];
    size_t serial_outputs[2];
    SeqClock st_clock;
    SeqControl st_latch, st_clear, st_oe, st_blank, st_all_on, st_polarity;
    size_t st_supply;
    size_t st_outputs[SH_MAX_STAGES];
    SeqClock ireg_clock;
    size_t ireg_data[SH_MAX_STAGES];
    size_t clock_out;
} ShUnit;

typedef struct {
    ShUnit units[SH_MAX_UNITS];
    size_t unit_count;
} ShBound;

typedef struct {
    SeqBits stages, storage, ireg;
    LogicValue serial_ff[2];
    LogicValue last_clock, last_st_clock, last_ireg_clock;
    SeqOutput outputs[SH_MAX_OUTPUTS];
    SeqOutput io[SH_MAX_STAGES];
    SeqOutput serial_outputs[2];
    SeqOutput st_outputs[SH_MAX_STAGES];
    SeqOutput clock_out;
} ShUnitState;

typedef struct {
    ShiftHeader h;
    const ShBound *b;
    ShUnitState units[SH_MAX_UNITS];
} ShState;

/* ---- running ---- */

typedef struct {
    const Sim *sim;
    const SimInstance *inst;
    const ShUnit *u;
    ShUnitState *us;
    SeqBits cur;
    int edge;
} ShEval;

typedef struct {
    const ShiftModel *model;
    void *dev;
    SeqTimingTable timing;
    ShiftTiming *timings;
} ShiftDevice;

SeqBits shf_zero_bits(unsigned n);
int shf_bind_names(KindContext *ctx, const PartPin *pins, size_t count,
                      const char *const *names, size_t n, size_t *out);
void shf_init_outputs(SeqOutput *outputs, const size_t *pins, size_t n);
int shf_control_list(KindContext *ctx, const cJSON *json, const char *key, int allow_when,
                        SeqControlName *out, size_t max, size_t *count);
void *shf_sh_load(KindContext *ctx, const cJSON *json, unsigned *units_out);
void *shf_sh_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count);
ShiftHeader *shf_sh_create(const void *dev, const void *bound);

#endif
