#ifndef SERIAL_UTIL_H
#define SERIAL_UTIL_H

/*
 * Small helpers shared by the plan-12-H serial models (serial_ay1013.c,
 * serial_i8251.c, serial_i82510.c, serial_acia6551.c, serial_hs82c52.c,
 * serial_com78c80x.c): three-valued logic, (value, known) registers, output
 * drives that change only on a new value, and loader helpers for pin-name
 * objects and lists.
 */

#include <stdio.h>
#include <string.h>

#include "serial.h"

/* A register: value and known bits. */
typedef struct {
    uint8_t v;
    uint8_t k;
} SuReg;

static inline SuReg su_reg(uint8_t v, uint8_t k) {
    SuReg r;

    r.v = v;
    r.k = k;
    return r;
}

static inline LogicValue su_bit(SuReg r, unsigned bit) {
    if (!((r.k >> bit) & 1u)) {
        return LOGIC_X;
    }
    return (r.v >> bit) & 1u ? LOGIC_1 : LOGIC_0;
}

static inline LogicValue su_not(LogicValue a) {
    return a == LOGIC_0 ? LOGIC_1 : a == LOGIC_1 ? LOGIC_0 : LOGIC_X;
}

static inline LogicValue su_and(LogicValue a, LogicValue b) {
    if (a == LOGIC_0 || b == LOGIC_0) {
        return LOGIC_0;
    }
    return a == LOGIC_1 && b == LOGIC_1 ? LOGIC_1 : LOGIC_X;
}

static inline LogicValue su_or(LogicValue a, LogicValue b) {
    if (a == LOGIC_1 || b == LOGIC_1) {
        return LOGIC_1;
    }
    return a == LOGIC_0 && b == LOGIC_0 ? LOGIC_0 : LOGIC_X;
}

static inline LogicValue su_bool(int b) {
    return b ? LOGIC_1 : LOGIC_0;
}

/* Merges two candidate levels when a selector is X: equal stays, else X. */
static inline LogicValue su_mux(LogicValue sel, LogicValue if1, LogicValue if0) {
    if (sel == LOGIC_1) {
        return if1;
    }
    if (sel == LOGIC_0) {
        return if0;
    }
    return if1 == if0 ? if1 : LOGIC_X;
}

/* Bit n of a (value, known) pair as a level. */
static inline LogicValue su_vk(unsigned v, unsigned k, unsigned bit) {
    if (!((k >> bit) & 1u)) {
        return LOGIC_X;
    }
    return (v >> bit) & 1u ? LOGIC_1 : LOGIC_0;
}

/* Output pins: the level each pin was last set to drive. */
typedef struct {
    LogicValue *out;
    size_t count;
} SuOutputs;

/* Drives a pin strong after `delay`, inertially, when the value changes. */
static inline void su_drive(SuOutputs *o, Sim *sim, SimInstance *instance, size_t pin,
                            LogicValue value, SimTime delay) {
    if (pin == SERIAL_NO_PIN || pin >= o->count || o->out[pin] == value) {
        return;
    }
    o->out[pin] = value;
    sim_schedule_inertial_drive(sim, instance, pin, value, DRIVE_STRONG, delay);
}

/* A clock-like output: a plain propagation delay (no pulse filtering). */
static inline void su_drive_clock(SuOutputs *o, Sim *sim, SimInstance *instance, size_t pin,
                                  LogicValue value, SimTime delay) {
    if (pin == SERIAL_NO_PIN || pin >= o->count || o->out[pin] == value) {
        return;
    }
    o->out[pin] = value;
    sim_schedule_drive(sim, instance, pin, value, DRIVE_STRONG, delay);
}

/* An open-drain output: 0 strong, 1 released (Z), X as X. */
static inline void su_drive_od(SuOutputs *o, Sim *sim, SimInstance *instance, size_t pin,
                               LogicValue active_low_level, SimTime delay) {
    LogicValue v = active_low_level == LOGIC_1 ? LOGIC_Z : active_low_level;

    su_drive(o, sim, instance, pin, v, delay);
}

/* A weak drive (straps sampled during reset). */
static inline void su_drive_weak(SuOutputs *o, Sim *sim, SimInstance *instance, size_t pin,
                                 LogicValue value) {
    if (pin == SERIAL_NO_PIN || pin >= o->count) {
        return;
    }
    o->out[pin] = LOGIC_Z;   /* the next strong drive always goes out */
    sim_schedule_inertial_drive(sim, instance, pin, value, DRIVE_WEAK, 0);
}

/* Reads n pins as a (value, known) pair, LSB first; Z reads X. */
static inline void su_read_bus(const Sim *sim, const SimInstance *instance, const size_t *pins,
                               unsigned n, unsigned *value, unsigned *known) {
    unsigned i;

    *value = 0;
    *known = 0;
    for (i = 0; i < n; i++) {
        LogicValue v = serial_level(sim, instance, pins[i], LOGIC_X);

        if (v == LOGIC_1) {
            *value |= 1u << i;
        }
        if (v != LOGIC_X) {
            *known |= 1u << i;
        }
    }
}

/* Drives n pins from a (value, known) pair, or releases them (Z). */
static inline void su_drive_bus(SuOutputs *o, Sim *sim, SimInstance *instance, const size_t *pins,
                                unsigned n, unsigned value, unsigned known, int release,
                                SimTime delay) {
    unsigned i;

    for (i = 0; i < n; i++) {
        LogicValue v = release ? LOGIC_Z : su_vk(value, known, i);

        su_drive(o, sim, instance, pins[i], v, delay);
    }
}

/* ---- Loader helpers ---------------------------------------------------------------------- */

/* An array of exactly n pin names (n = 0: 1 to cap names); *count set. */
static inline int su_load_names(KindContext *ctx, const cJSON *object, const char *key,
                                int required, const char **out, unsigned cap, unsigned exact,
                                unsigned *count) {
    const char *const *names;
    size_t n;
    size_t i;

    if (count != NULL) {
        *count = 0;
    }
    if (cJSON_GetObjectItemCaseSensitive(object, key) == NULL) {
        return required ? kind_error(ctx, "missing \"%s\"", key) : 0;
    }
    if (kind_names(ctx, object, key, &names, &n) != 0) {
        return -1;
    }
    if (exact != 0 && n != exact) {
        return kind_error(ctx, "\"%s\" needs exactly %u pin names", key, exact);
    }
    if (n == 0 || n > cap) {
        return kind_error(ctx, "\"%s\" needs 1 to %u pin names", key, cap);
    }
    for (i = 0; i < n; i++) {
        out[i] = names[i];
    }
    if (count != NULL) {
        *count = (unsigned)n;
    }
    return 0;
}

/*
 * An object of pin names: keys[i] -> out[i]. Keys whose bit is set in
 * `required` must be present. Unknown keys are rejected. A missing object is
 * an error when `object_required`, else every name stays NULL.
 */
static inline int su_load_pin_object(KindContext *ctx, const cJSON *json, const char *key,
                                     int object_required, const char *const *keys,
                                     const char **out, unsigned required) {
    const cJSON *object = kind_member(ctx, json, key, object_required);
    unsigned i;

    for (i = 0; keys[i] != NULL; i++) {
        out[i] = NULL;
    }
    if (object == NULL) {
        return object_required ? -1 : 0;
    }
    kind_push(ctx, "%s", key);
    if (!cJSON_IsObject(object)) {
        return kind_error(ctx, "must be an object of pin names");
    }
    if (kind_check_keys(ctx, object, keys, NULL) != 0) {
        return -1;
    }
    for (i = 0; keys[i] != NULL; i++) {
        if (serial_optional_name(ctx, object, keys[i], &out[i]) != 0) {
            return -1;
        }
        if (((required >> i) & 1u) && out[i] == NULL) {
            return kind_error(ctx, "missing \"%s\"", keys[i]);
        }
    }
    kind_pop(ctx);
    return 0;
}

/* Binds n optional names (NULL = no pin); key names the error path. */
static inline int su_bind_names(KindContext *ctx, const PartPin *pins, size_t pin_count,
                                const char *key, const char *const *names, size_t *out,
                                unsigned n) {
    char path[64];
    unsigned i;

    for (i = 0; i < n; i++) {
        snprintf(path, sizeof path, "%s[%u]", key, i);
        if (serial_bind_pin(ctx, pins, pin_count, path, names[i], &out[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

/* Binds an object loaded by su_load_pin_object. */
static inline int su_bind_object(KindContext *ctx, const PartPin *pins, size_t pin_count,
                                 const char *key, const char *const *keys,
                                 const char *const *names, size_t *out) {
    char path[64];
    unsigned i;

    for (i = 0; keys[i] != NULL; i++) {
        snprintf(path, sizeof path, "%s.%s", key, keys[i]);
        if (serial_bind_pin(ctx, pins, pin_count, path, names[i], &out[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

/*
 * The receiver after a tick (every H model): the engine takes a framing
 * error as the next start bit (the PC16550D resync). These devices look for
 * a fresh 1 -> 0 instead, so after a framing error the receiver goes idle
 * with the current level as its last one.
 */
static inline void su_rx_no_resync(SerialRx *rx, LogicValue level) {
    serial_rx_reset(rx);
    rx->last = level;
}

/* The 16x-style ticks per bit of a 1x clock: the engine needs a start check
   between the edge and the first data sample, so 1x runs at factor 2 with
   two engine ticks per clock edge. */
enum { SU_ONE_X_FACTOR = 2 };

#endif
