#ifndef SEQ_H
#define SEQ_H

/*
 * Helpers shared by the sequential kinds (latch, counter, shift; plan-10-B
 * section 4.2): timing entries, control objects, level reading with the
 * signal rules of docs/parts-format.md "Sequential kinds", edges, bit
 * vectors with a known mask, and change-suppressed inertial outputs.
 *
 * Load-time helpers return 0, or -1 with kind_error set.
 */

#include <stddef.h>
#include <stdint.h>

#include "kind.h"

#define SEQ_NO_PIN ((size_t)-1)

/* A control at load time: {"pin": name, "active": "high"|"low"[, "when": ...]}. */
typedef struct {
    const char *pin;   /* NULL = no control */
    int active_high;
    int clocked;       /* "when": "clock" (1) or "async" (0) */
} SeqControlName;

/* A bound control; pin SEQ_NO_PIN = none. */
typedef struct {
    size_t pin;
    int active_high;
    int clocked;
} SeqControl;

/* One timing entry. json is the entry's object, valid only while loading. */
typedef struct {
    const char *family;
    SimTime rise, fall;
    const cJSON *json;
    const PartVoltage *vcc;
    PartLogic logic;
} SeqTiming;

typedef struct {
    SeqTiming *entries;
    size_t count;
} SeqTimingTable;

/* Up to 64 bits; bit i is known when (known >> i) & 1. */
typedef struct {
    uint64_t value, known;
} SeqBits;

typedef enum { SEQ_PUSH_PULL, SEQ_OPEN_DRAIN } SeqOutputStyle;

/* One output pin's drive state. */
typedef struct {
    size_t pin;
    int scheduled;
    LogicValue driven;
} SeqOutput;

/* ---- Loading ------------------------------------------------------------ */

/*
 * Every key of device_json that is not a library device key (id function
 * description drawer note pinouts parts), "model" or "timing" must be in
 * model_keys (NULL-terminated); else "<key>" is not a key of model "<model>".
 */
int seq_check_model_keys(KindContext *ctx, const cJSON *device_json, const char *model_name,
                         const char *const *model_keys);

/*
 * Parses "timing": family -> {rise, fall[, note, vcc, inputMax, extra_keys...]}.
 * Each entry's logic is {family, units}.
 */
int seq_timing(KindContext *ctx, const cJSON *device_json, const char *const *extra_keys,
               unsigned units, SeqTimingTable *table);

/* An extra timing key in ns, as ps. Absent and not required: *ps unchanged, 0. */
int seq_timing_extra(KindContext *ctx, const SeqTiming *entry, const char *key, int required,
                     SimTime *ps);

/* Reads the part's "family", finds its entry, sets out->logic and out->vcc. */
int seq_part(KindContext *ctx, const SeqTimingTable *table, const cJSON *part_json, Part *out,
             const SeqTiming **entry);

/* "high" -> 1, "low" -> 0, for the key's string value. */
int seq_active(KindContext *ctx, const cJSON *json, const char *key, int *active_high);

/*
 * A control object at json[key]. Absent and not required: name->pin = NULL.
 * allow_when: "when" is allowed (and required); otherwise it is an error.
 */
int seq_control_name(KindContext *ctx, const cJSON *json, const char *key, int required,
                     int allow_when, SeqControlName *name);

/* An optional pin name: absent -> NULL. */
int seq_optional_name(KindContext *ctx, const cJSON *json, const char *key, const char **out);

/* An array of min..max pin names. */
int seq_names(KindContext *ctx, const cJSON *json, const char *key, size_t min, size_t max,
              const char *const **names, size_t *count);

/* "output": "push-pull" (default) or "open-drain". */
int seq_output_style(KindContext *ctx, const cJSON *json, SeqOutputStyle *style);

/* An optional bool: absent -> fallback. */
int seq_optional_bool(KindContext *ctx, const cJSON *json, const char *key, int fallback,
                      int *out);

/* Name -> pin index (kind_pin); NULL -> SEQ_NO_PIN. */
int seq_bind(KindContext *ctx, const PartPin *pins, size_t count, const char *name,
             size_t *index);
int seq_bind_control(KindContext *ctx, const PartPin *pins, size_t count,
                     const SeqControlName *name, SeqControl *control);

/* ---- Running ------------------------------------------------------------ */

/* 1, 0, or -1 (X or Z). */
int seq_level(LogicValue v);

/* Asynchronous control: asserted 1 / 0 / -1; Z reads inactive; no pin -> 0. */
int seq_async(const Sim *sim, const SimInstance *inst, const SeqControl *c);

/* Synchronous control or enable: asserted 1 / 0 / -1; Z reads X; no pin -> 0. */
int seq_sync(const Sim *sim, const SimInstance *inst, const SeqControl *c);

/* Output enable: 1 enabled / 0 disabled / -1 unknown (X or Z); no pin -> 1. */
int seq_enable(const Sim *sim, const SimInstance *inst, const SeqControl *c);

/* A pin's level: 1, 0, or -1 (X or Z); SEQ_NO_PIN -> -1. */
int seq_read(const Sim *sim, const SimInstance *inst, size_t pin);

/* Verilog posedge / negedge: any move into 1 (0) from another value. */
int seq_rising(LogicValue old_value, LogicValue new_value);
int seq_falling(LogicValue old_value, LogicValue new_value);

/* 0, 1 or X. */
LogicValue seq_bit(SeqBits b, unsigned i);
/* Sets bit i to v (0, 1, else unknown). */
void seq_set_bit(SeqBits *b, unsigned i, LogicValue v);
LogicValue seq_not(LogicValue v);
/* v from a level (1, 0, -1). */
LogicValue seq_value(int level);

/* enable 1 -> v, 0 -> Z, -1 -> X. */
LogicValue seq_gate(LogicValue v, int enable);

/*
 * Drives o: open drain turns 1 (and Z) into Z; skipped when o already has
 * that value scheduled; else an inertial strong drive, fall delay for 0,
 * rise for 1 or Z, the longer for X. No pin -> nothing.
 */
void seq_drive(Sim *sim, SimInstance *inst, SeqOutput *o, LogicValue v, SeqOutputStyle style,
               const SeqTiming *t);

#endif
