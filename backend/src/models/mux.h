#ifndef MUX_H
#define MUX_H

/*
 * mux: the multiplexer kind's model table and the helpers its models share
 * (plan-21-B section 3). mux.c holds the kind and model `select`;
 * mux_stored.c the models with storage (plan-21-C).
 */

#include <stddef.h>

#include "kind.h"
#include "seq.h"

#define MUX_MAX_UNITS 6
#define MUX_MAX_SELECT 4
#define MUX_MAX_DATA 16

typedef struct MuxModel MuxModel;

/* Every model's run state starts with this. */
typedef struct {
    const MuxModel *model;
    const SeqTiming *timing;
} MuxHeader;

struct MuxModel {
    const char *name;
    const char *const *keys;
    void *(*load)(KindContext *ctx, const cJSON *json, unsigned *units);
    void *(*bind)(KindContext *ctx, const void *dev, const PartPin *pins, size_t count);
    MuxHeader *(*create)(const void *bound);
    /* pin: the pin that changed (SEQ_NO_PIN at start), with its old and new value. */
    void (*eval)(Sim *sim, SimInstance *inst, MuxHeader *state, size_t pin,
                 LogicValue old_value, LogicValue new_value);
    int (*open_drain)(const void *bound, size_t pin);
};

/* The models of mux_stored.c (plan-21-C). */
extern const MuxModel MUX_STORED, MUX_REGFILE;

/* ---- One selector unit (select, and the selector inside stored) ---------- */

typedef struct {
    const char *const *data;
    size_t data_count;
    const char *const *select;
    size_t select_count;
    const char *y, *w;
    SeqControlName strobe, oe;
} MuxUnitNames;

typedef struct {
    size_t data[MUX_MAX_DATA];
    size_t data_count;
    size_t select[MUX_MAX_SELECT];
    size_t select_count;
    size_t y, w;
    SeqControl strobe, oe;
} MuxUnit;

/* The unit's two outputs' drive state. */
typedef struct {
    SeqOutput y, w;
} MuxUnitOut;

/* Shared device keys of a selector: "units", "disabled", "invert", "output". */
typedef struct {
    MuxUnitNames units[MUX_MAX_UNITS];
    size_t unit_count;
    int disabled_z, invert;
    SeqOutputStyle style;
} MuxSelectNames;

typedef struct {
    MuxUnit units[MUX_MAX_UNITS];
    size_t unit_count;
    int disabled_z, invert;
    SeqOutputStyle style;
} MuxSelectBound;

/* Loads "units", "disabled", "invert", "output"; *units = the unit count. */
int mux_load_select(KindContext *ctx, const cJSON *json, MuxSelectNames *names, unsigned *units);
int mux_bind_select(KindContext *ctx, const MuxSelectNames *names, const PartPin *pins,
                    size_t count, MuxSelectBound *bound);
void mux_init_select_out(const MuxSelectBound *b, MuxUnitOut *out);

/*
 * The value data[code] for the select levels sel (1, 0, -1): an unknown
 * select bit gives X unless every data value it chooses between is equal
 * and known.
 */
LogicValue mux_choose(const LogicValue *data, const int *sel, size_t select_count);

/*
 * Drives unit u's Y and W from the chosen value v (before "invert"): OE
 * deasserted (the unit's oe, or shared_oe 0: the device's common output
 * enables) -> Z, X OE -> X; else strobe deasserted -> the disabled state
 * (Y low and W high, or Z), X strobe -> X; else Y = v (inverted per
 * invert), W = NOT Y.
 */
void mux_drive_unit(Sim *sim, SimInstance *inst, const MuxSelectBound *b, size_t u,
                    MuxUnitOut *out, LogicValue v, int shared_oe, const SeqTiming *t);
int mux_select_open_drain(const MuxSelectBound *b, size_t pin);

/* ---- Small helpers -------------------------------------------------------- */

int mux_bind_names(KindContext *ctx, const PartPin *pins, size_t count,
                   const char *const *names, size_t n, size_t *out);
void mux_init_outputs(SeqOutput *outputs, const size_t *pins, size_t n);
int mux_pin_in(size_t pin, const size_t *pins, size_t n);

#endif
