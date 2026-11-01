/*
 * oscillator: a clock source (ported from backend/osc.c). The primary output
 * toggles every half period, starting at 0; a second output is its
 * complement. A LOW on the enable pin puts every output in high impedance;
 * NC or HIGH keeps the clock running. The half period is rounded to whole
 * picoseconds.
 */

#include <stdlib.h>
#include <string.h>

#include "kind.h"

static const char *const DEVICE_KEYS[] = {"family", "minHz", "maxHz", "outputs", NULL};
static const char *const PART_KEYS[] = {"defaultHz", "enablePin", NULL};

typedef struct {
    const char *family;
    uint64_t min_hz;
    uint64_t max_hz;
    const char *const *outputs;
    size_t output_count;
} OscDevice;

typedef struct {
    size_t out_pin;
    int comp_pin;   /* -1 when there is none */
} OscPins;

static void *osc_load_device(KindContext *ctx, const cJSON *json) {
    OscDevice *device = kind_alloc(ctx, sizeof(*device));

    if (device == NULL || kind_string(ctx, json, "family", &device->family) != 0 ||
        kind_uint(ctx, json, "minHz", UINT64_MAX, &device->min_hz) != 0 ||
        kind_uint(ctx, json, "maxHz", UINT64_MAX, &device->max_hz) != 0 ||
        kind_names(ctx, json, "outputs", &device->outputs, &device->output_count) != 0) {
        return NULL;
    }
    if (device->min_hz == 0 || device->min_hz > device->max_hz) {
        kind_error(ctx, "need 0 < minHz <= maxHz");
        return NULL;
    }
    if (device->output_count < 1 || device->output_count > 2) {
        kind_error(ctx, "\"outputs\" is [primary] or [primary, complement]");
        return NULL;
    }
    return device;
}

static void *osc_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                             size_t pin_count) {
    const OscDevice *device = raw;
    OscPins *bound = kind_alloc(ctx, sizeof(*bound));
    size_t comp;

    if (bound == NULL || kind_pin(ctx, pins, pin_count, device->outputs[0], &bound->out_pin) != 0) {
        return NULL;
    }
    bound->comp_pin = -1;
    if (device->output_count == 2) {
        if (kind_pin(ctx, pins, pin_count, device->outputs[1], &comp) != 0) {
            return NULL;
        }
        bound->comp_pin = (int)comp;
    }
    return bound;
}

static int osc_load_part(KindContext *ctx, const void *raw, const void *bound,
                         const cJSON *json, Part *out) {
    const OscDevice *device = raw;
    PartOscillator *info = kind_alloc(ctx, sizeof(*info));

    (void)bound;
    if (info == NULL || kind_uint(ctx, json, "defaultHz", UINT64_MAX, &info->default_hz) != 0) {
        return -1;
    }
    if (info->default_hz < device->min_hz || info->default_hz > device->max_hz) {
        return kind_error(ctx, "defaultHz %llu is outside minHz..maxHz",
                          (unsigned long long)info->default_hz);
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "enablePin") != NULL) {
        uint64_t number;

        if (kind_uint(ctx, json, "enablePin", out->pin_count, &number) != 0) {
            return -1;
        }
        if (number == 0) {
            return kind_error(ctx, "\"enablePin\" is a pin number from 1");
        }
        info->enable_pin = (unsigned)number;
    }
    info->family = device->family;
    info->min_hz = device->min_hz;
    info->max_hz = device->max_hz;
    out->oscillator = info;
    return 0;
}

/* ---- Model --------------------------------------------------------------- */

typedef struct {
    size_t out_pin;
    int comp_pin;
    int enable_pin;   /* pin index, -1 when there is none */
    SimTime half_period;
    LogicValue level;
} OscState;

static size_t osc_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *osc_create(const void *config) {
    const Part *part = config;
    const OscPins *pins = part_internal(part)->bound;
    uint64_t hz = part->oscillator->default_hz;
    OscState *state;

    if (hz == 0) {
        return NULL;
    }
    state = malloc(sizeof(*state));
    if (state == NULL) {
        return NULL;
    }
    state->half_period = (1000000000000ULL + hz) / (2 * hz);
    state->out_pin = pins->out_pin;
    state->comp_pin = pins->comp_pin;
    state->enable_pin = part->oscillator->enable_pin > 0 ? (int)part->oscillator->enable_pin - 1 : -1;
    state->level = LOGIC_0;
    return state;
}

static void osc_destroy(void *state) {
    free(state);
}

static void drive_outputs(Sim *sim, SimInstance *instance, const OscState *state) {
    int enabled = state->enable_pin < 0 ||
                  sim_read_pin(sim, instance, (size_t)state->enable_pin) != LOGIC_0;
    LogicValue primary = enabled ? state->level : LOGIC_Z;
    LogicValue complement = !enabled ? LOGIC_Z
                          : state->level == LOGIC_0 ? LOGIC_1 : LOGIC_0;

    sim_schedule_drive(sim, instance, state->out_pin, primary, DRIVE_STRONG, 0);
    if (state->comp_pin >= 0) {
        sim_schedule_drive(sim, instance, (size_t)state->comp_pin, complement, DRIVE_STRONG, 0);
    }
}

static void osc_start(Sim *sim, SimInstance *instance, void *raw) {
    OscState *state = raw;

    state->level = LOGIC_0;
    drive_outputs(sim, instance, state);
    sim_schedule_timer(sim, instance, 0, state->half_period);
}

static void osc_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t timer_id) {
    OscState *state = raw;

    (void)timer_id;
    state->level = state->level == LOGIC_0 ? LOGIC_1 : LOGIC_0;
    drive_outputs(sim, instance, state);
    sim_schedule_timer(sim, instance, 0, state->half_period);
}

static void osc_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                            LogicValue old_value, LogicValue new_value) {
    const OscState *state = raw;

    (void)old_value;
    (void)new_value;
    if (state->enable_pin >= 0 && pin == (size_t)state->enable_pin) {
        drive_outputs(sim, instance, state);
    }
}

static const SimComponentType OSC_MODEL = {
    "oscillator", osc_pin_count, osc_create, osc_destroy, osc_start, osc_pin_changed, osc_timer,
    NULL, NULL, NULL, NULL
};

const PartKindOps OSCILLATOR_KIND = {
    PART_OSCILLATOR, "oscillator", &OSC_MODEL, DEVICE_KEYS, PART_KEYS,
    osc_load_device, osc_bind_pinout, osc_load_part
};
