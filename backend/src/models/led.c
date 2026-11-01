/*
 * led: a single LED. Passive: it drives nothing. It is lit while its anode
 * reads 1 and its cathode reads 0 (the old netlist simulator's rule), tracked
 * with lights.c so the app sees "on at any time during the last step".
 */

#include <stdlib.h>

#include "kind.h"
#include "lights.h"

static const char *const DEVICE_KEYS[] = {"color", "anode", "cathode", NULL};

typedef struct {
    PartLed led;
    const char *anode;
    const char *cathode;
} LedDevice;

typedef struct {
    size_t anode;
    size_t cathode;
} LedPins;

static void *led_load_device(KindContext *ctx, const cJSON *json) {
    LedDevice *device = kind_alloc(ctx, sizeof(*device));

    if (device == NULL || kind_string(ctx, json, "color", &device->led.color) != 0 ||
        kind_string(ctx, json, "anode", &device->anode) != 0 ||
        kind_string(ctx, json, "cathode", &device->cathode) != 0) {
        return NULL;
    }
    return device;
}

static void *led_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                             size_t pin_count) {
    const LedDevice *device = raw;
    LedPins *bound = kind_alloc(ctx, sizeof(*bound));

    if (bound == NULL ||
        kind_pin(ctx, pins, pin_count, device->anode, &bound->anode) != 0 ||
        kind_pin(ctx, pins, pin_count, device->cathode, &bound->cathode) != 0) {
        return NULL;
    }
    return bound;
}

static int led_load_part(KindContext *ctx, const void *device, const void *bound,
                         const cJSON *json, Part *out) {
    (void)ctx;
    (void)bound;
    (void)json;
    out->led = &((const LedDevice *)device)->led;
    return 0;
}

/* ---- Model --------------------------------------------------------------- */

typedef struct {
    LedPins pins;
    Light light;
} LedState;

static size_t led_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *led_create(const void *config) {
    LedState *state = calloc(1, sizeof(*state));

    if (state != NULL) {
        state->pins = *(const LedPins *)part_internal(config)->bound;
    }
    return state;
}

static void led_destroy(void *state) {
    free(state);
}

static void led_update(Sim *sim, SimInstance *instance, LedState *state) {
    int on = sim_read_pin(sim, instance, state->pins.anode) == LOGIC_1 &&
             sim_read_pin(sim, instance, state->pins.cathode) == LOGIC_0;

    lights_set(&state->light, 0, on, sim_now(sim));
}

static void led_start(Sim *sim, SimInstance *instance, void *state) {
    led_update(sim, instance, state);
}

static void led_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                            LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    led_update(sim, instance, state);
}

static size_t led_lights(const Sim *sim, const SimInstance *instance, const void *state,
                         SimTime since, unsigned char *out, size_t cap) {
    (void)sim;
    (void)instance;
    return lights_read(&((const LedState *)state)->light, 1, since, out, cap);
}

static size_t led_light_time(const Sim *sim, const SimInstance *instance, const void *state,
                             uint64_t *out, size_t cap) {
    (void)instance;
    return lights_time(&((const LedState *)state)->light, 1, sim_now(sim), out, cap);
}

static const SimComponentType LED_MODEL = {
    "led", led_pin_count, led_create, led_destroy, led_start, led_pin_changed, NULL,
    led_lights, NULL, NULL, NULL, led_light_time
};

const PartKindOps LED_KIND = {
    PART_LED, "led", &LED_MODEL, DEVICE_KEYS, NULL,
    led_load_device, led_bind_pinout, led_load_part
};
