#ifndef KIND_H
#define KIND_H

/*
 * Internal interface between the part library and the kinds (plan-01-A
 * section 3.5). One PartKindOps per kind; kinds.c lists them in KINDS[].
 *
 * Loading a device runs, in order: load_device (the device's kind keys),
 * bind_pinout once per pinout (resolve pin names to indices in that
 * pinout), then load_part per part (part keys; fills the kind's info
 * pointer on the Part). Everything a kind allocates comes from the
 * KindContext arena, which the library frees if the file fails to load.
 *
 * The engine model of every part gets the `const Part *` as its config.
 * Models reach their data through part_internal(part).
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "cJSON.h"
#include "parts.h"
#include "sim.h"

typedef struct KindContext KindContext;

typedef struct PartKindOps {
    PartKind kind;
    const char *name;                       /* the file's "kind" */
    const SimComponentType *model;          /* engine callbacks; config = const Part * */
    const char *const *device_keys;         /* kind keys allowed on a device, NULL-terminated */
    const char *const *part_keys;           /* kind keys allowed on a part, NULL-terminated */
    void *(*load_device)(KindContext *ctx, const cJSON *device);
    void *(*bind_pinout)(KindContext *ctx, const void *device,
                         const PartPin *pins, size_t pin_count);
    int (*load_part)(KindContext *ctx, const void *device, const void *bound,
                     const cJSON *part, Part *out);
} PartKindOps;

/* What Part.internal points to. */
typedef struct {
    const PartKindOps *ops;
    const void *device;   /* from load_device */
    const void *bound;    /* from bind_pinout, for the part's pinout */
    const void *data;     /* optional, set by load_part */
} PartInternal;

static inline const PartInternal *part_internal(const Part *part) {
    return (const PartInternal *)part->internal;
}

/* The engine model of a part. */
static inline const SimComponentType *kind_model(const Part *part) {
    return part_internal(part)->ops->model;
}

/* Lets load_part attach per-part data for the model. */
static inline void kind_set_part_data(Part *out, const void *data) {
    ((PartInternal *)out->internal)->data = data;
}

/* ---- The kind table (kinds.c) ------------------------------------------- */

extern const PartKindOps *const KINDS[];
extern const size_t KIND_COUNT;

const PartKindOps *kind_find(const char *name);

/* ---- KindContext helpers (kind_util.c) ---------------------------------- */

/* Zeroed memory from the file's arena; NULL (with an error set) when out of memory. */
void *kind_alloc(KindContext *ctx, size_t size);
char *kind_strdup(KindContext *ctx, const char *text);

/*
 * Records an error, prefixed with the source and current JSON path, e.g.
 * "gate.json: devices[3] (74_00): parts[12] (SN74HC00N): unknown family". Only
 * the first error is kept. Always returns -1.
 */
int kind_error(KindContext *ctx, const char *format, ...);
int kind_failed(const KindContext *ctx);

/* Path segments for messages, e.g. kind_push(ctx, "gates[%zu]", i). */
void kind_push(KindContext *ctx, const char *format, ...);
void kind_pop(KindContext *ctx);

/*
 * Rejects keys of an object that are in neither list (both NULL-terminated,
 * either may be NULL; both NULL = any key) and keys that appear twice.
 */
int kind_check_keys(KindContext *ctx, const cJSON *object, const char *const *common,
                    const char *const *extra);

/* The member, or NULL; an error when required and missing. */
const cJSON *kind_member(KindContext *ctx, const cJSON *object, const char *key, int required);

/* Typed members. Required unless stated; each returns 0 or -1 (error set). */
int kind_string(KindContext *ctx, const cJSON *object, const char *key, const char **out);
int kind_uint(KindContext *ctx, const cJSON *object, const char *key, uint64_t max,
              uint64_t *out);
/* A JSON number (any value; callers check the range). */
int kind_number(KindContext *ctx, const cJSON *object, const char *key, double *out);
int kind_bool(KindContext *ctx, const cJSON *object, const char *key, int *out);
/* An array of strings; *out is arena memory. */
int kind_names(KindContext *ctx, const cJSON *object, const char *key,
               const char *const **out, size_t *count);

/* A time in nanoseconds (a JSON number), as picoseconds: llround(ns * 1000). */
int kind_ns(KindContext *ctx, const cJSON *value, const char *what, uint64_t *ps);

/* ---- Voltages (volts in JSON, millivolts in C; docs/parts-format.md) ---- */

/* A number of volts from 0 to max_v, as millivolts: llround(v * 1000). */
int kind_volts(KindContext *ctx, const cJSON *object, const char *key, double max_v,
               uint32_t *mv);

/* A [min, max] pair of volts, 0 < min, max <= max_v; min < max when strict,
   else min <= max. */
int kind_volt_range(KindContext *ctx, const cJSON *object, const char *key, double max_v,
                    int strict, uint32_t *min_mv, uint32_t *max_mv);

/*
 * Reads optional "vcc" and "inputMax" from json (plan-07-A section 4.2). On
 * return *out is the resolved voltage (base merged with json's keys), base
 * itself when json has neither key, or NULL when neither has a range. json's
 * "vcc" replaces base's range and its "inputMax" replaces base's; an input
 * max given at neither level follows the range's max. -1 on error.
 */
int kind_voltage(KindContext *ctx, const cJSON *json, const PartVoltage *base,
                 const PartVoltage **out);

/* The one pin with this name; an error when there is none or several. */
int kind_pin(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *name,
             size_t *index);

/* Every pin with this name, in pin order; 0 (error set) when there is none. */
size_t kind_pins(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *name,
                 size_t *out, size_t cap);

/* JEDEC JEP95 ball name: 1-2 row letters (no I, O, Q, S, X, Z), then a
   column of 1-99 with no leading zero, e.g. "A1", "AB12". */
int kind_is_ball_name(const char *text);

/* Index of a JEP95 row letter (A = 0, B = 1, ... skipping I O Q S X Z), or -1. */
int kind_ball_row(char c);

/* The library's spelling of a logic family name, or NULL (error set). */
const char *kind_logic_family(KindContext *ctx, const char *text);

#endif
