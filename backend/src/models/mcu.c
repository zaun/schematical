/*
 * mcu: a microcontroller (plan-36-F, plan-62-E). Every device has facts:
 * core, clock, memory sizes and supply range, for the catalog, the BOM and
 * the voltage check.
 *
 * A device with a "model" is simulated: the model names a register layout
 * of the AVR core (avr_chip.h: the ATmega48/88/168/328 and the
 * ATtiny25/45/85), and mcu_avr.c runs it on the engine. A
 * device without one has no model of its core, must say "simulated": false,
 * and circuit_add_part refuses its parts; its engine model is passive (the
 * header.c shape), which is what trace_check's bare engine harness, the one
 * place that instantiates it, gets.
 */

#include <string.h>

#include "kind.h"
#include "mcu.h"

static const char *const DEVICE_KEYS[] = {"core", "clock", "flash", "ram", "eeprom", "vcc",
                                          "inputMax", "model", NULL};
static const char *const PART_KEYS[] = {"vcc", "inputMax", "clock", "flash", "psram", NULL};

static const char *const CORES[] = {"avr", "avrrc", "avrxt", "avrxm", "cortex-m0plus",
                                    "xtensa-lx6", "xtensa-lx7", "riscv", NULL};

/* An optional byte count (at most 4294967295) into *out; absent leaves it. */
static int mcu_bytes(KindContext *ctx, const cJSON *json, const char *key, uint32_t *out) {
    uint64_t value;

    if (cJSON_GetObjectItemCaseSensitive(json, key) == NULL) {
        return 0;
    }
    if (kind_uint(ctx, json, key, UINT32_MAX, &value) != 0) {
        return -1;
    }
    *out = (uint32_t)value;
    return 0;
}

/* An optional clock (Hz, above 0) into *out; absent leaves it. */
static int mcu_clock(KindContext *ctx, const cJSON *json, uint64_t *out) {
    uint64_t value;

    if (cJSON_GetObjectItemCaseSensitive(json, "clock") == NULL) {
        return 0;
    }
    if (kind_uint(ctx, json, "clock", UINT64_MAX, &value) != 0) {
        return -1;
    }
    if (value == 0) {
        return kind_error(ctx, "\"clock\" must be above 0");
    }
    *out = value;
    return 0;
}

static void *mcu_load_device(KindContext *ctx, const cJSON *json) {
    McuDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *simulated = cJSON_GetObjectItemCaseSensitive(json, "simulated");
    const char *core;
    const char *model = NULL;
    uint64_t value;
    size_t i;

    if (device == NULL) {
        return NULL;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "model") != NULL) {
        if (kind_string(ctx, json, "model", &model) != 0) {
            return NULL;
        }
        if (cJSON_IsFalse(simulated)) {
            kind_error(ctx, "an mcu with a \"model\" is simulated: remove \"simulated\": false");
            return NULL;
        }
    } else if (!cJSON_IsFalse(simulated)) {
        kind_error(ctx, "an mcu has no simulation model: \"simulated\" must be false");
        return NULL;
    }
    if (kind_string(ctx, json, "core", &core) != 0) {
        return NULL;
    }
    for (i = 0; CORES[i] != NULL && strcmp(CORES[i], core) != 0; i++) {
    }
    if (CORES[i] == NULL) {
        kind_error(ctx, "unknown mcu core \"%s\"", core);
        return NULL;
    }
    device->facts.core = CORES[i];
    if (model != NULL) {
        if (strcmp(core, "avr") != 0) {
            kind_error(ctx, "\"model\" needs core \"avr\"");
            return NULL;
        }
        device->layout = avr_layout_find(model);
        if (device->layout == NULL) {
            kind_error(ctx, "unknown mcu model \"%s\"", model);
            return NULL;
        }
        device->facts.model = device->layout->name;
    }
    if (kind_member(ctx, json, "clock", 1) == NULL || mcu_clock(ctx, json, &device->facts.max_hz) != 0 ||
        kind_uint(ctx, json, "flash", UINT32_MAX, &value) != 0) {
        return NULL;
    }
    device->facts.flash = (uint32_t)value;
    if (kind_uint(ctx, json, "ram", UINT32_MAX, &value) != 0) {
        return NULL;
    }
    device->facts.ram = (uint32_t)value;
    if (mcu_bytes(ctx, json, "eeprom", &device->facts.eeprom) != 0 ||
        kind_voltage(ctx, json, NULL, &device->vcc) != 0) {
        return NULL;
    }
    if (device->layout != NULL && (device->facts.flash < 512 || device->facts.flash % 2 != 0)) {
        kind_error(ctx, "an mcu with a \"model\" needs \"flash\" of 512 bytes or more, an even number");
        return NULL;
    }
    return device;
}

/* A model device's pins by name: P<letter><bit> for the layout's ports,
   ADC6 and ADC7. Any other name takes no part in the simulation. */
static void *mcu_bind_model(KindContext *ctx, const McuDevice *device, const PartPin *pins,
                            size_t pin_count) {
    const AvrLayout *layout = device->layout;
    McuAvrPins *bound = kind_alloc(ctx, sizeof(*bound));
    size_t i, p;
    int bit;

    if (bound == NULL) {
        return NULL;
    }
    bound->layout = layout;
    bound->adc6 = MCU_NO_PIN;
    bound->adc7 = MCU_NO_PIN;
    for (p = 0; p < AVR_LAYOUT_PORTS; p++) {
        for (bit = 0; bit < 8; bit++) {
            bound->port[p][bit] = MCU_NO_PIN;
        }
    }
    for (i = 0; i < pin_count; i++) {
        const char *name = pins[i].name;

        if (name == NULL) {
            continue;
        }
        if (strcmp(name, "ADC6") == 0) {
            bound->adc6 = i;
        } else if (strcmp(name, "ADC7") == 0) {
            bound->adc7 = i;
        } else if (name[0] == 'P' && name[1] >= 'A' && name[1] <= 'L' && name[2] >= '0' &&
                   name[2] <= '7' && name[3] == '\0') {
            for (p = 0; p < layout->port_count && layout->ports[p].letter != name[1]; p++) {
            }
            if (p == layout->port_count) {
                kind_error(ctx, "mcu model \"%s\" has no pin \"%s\"", layout->name, name);
                return NULL;
            }
            bound->port[p][name[2] - '0'] = i;
        }
    }
    return bound;
}

static void *mcu_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                             size_t pin_count) {
    const McuDevice *device = raw;
    int power = 0, ground = 0;
    size_t i;

    for (i = 0; i < pin_count; i++) {
        power |= pins[i].role == PIN_POWER;
        ground |= pins[i].role == PIN_GROUND;
    }
    if (!power) {
        kind_error(ctx, "an mcu pinout needs a power pin");
        return NULL;
    }
    if (!ground) {
        kind_error(ctx, "an mcu pinout needs a ground pin");
        return NULL;
    }
    if (device->layout != NULL) {
        return mcu_bind_model(ctx, device, pins, pin_count);
    }
    return (void *)raw;
}

static int mcu_load_part(KindContext *ctx, const void *raw, const void *bound,
                         const cJSON *json, Part *out) {
    const McuDevice *device = raw;
    PartMcu *info = kind_alloc(ctx, sizeof(*info));

    (void)bound;
    if (info == NULL) {
        return -1;
    }
    *info = device->facts;
    if (mcu_clock(ctx, json, &info->max_hz) != 0 || mcu_bytes(ctx, json, "flash", &info->flash) != 0 ||
        mcu_bytes(ctx, json, "psram", &info->psram) != 0) {
        return -1;
    }
    if (device->layout != NULL && info->flash != device->facts.flash) {
        return kind_error(ctx, "a part of an mcu with a \"model\" cannot override \"flash\"");
    }
    out->mcu = info;
    return kind_part_voltage(ctx, json, device->vcc, &out->vcc);
}

/* ---- The engine model ------------------------------------------------------- */

/* The state of a part with no model: nothing. Any non-NULL pointer tells the
   engine creation worked. */
static const char PASSIVE = 0;

static McuAvr *model_of(const void *state) {
    return state == (const void *)&PASSIVE ? NULL : (McuAvr *)state;
}

static size_t mcu_pin_count(const void *config) { return ((const Part *)config)->pin_count; }

static void *mcu_create(const void *config) {
    const Part *part = config;
    const McuDevice *device = part_internal(part)->device;

    if (device->layout == NULL) {
        return (void *)&PASSIVE;
    }
    return mcu_avr_create(part, part_internal(part)->bound);
}

static void mcu_destroy(void *state) {
    if (model_of(state) != NULL) {
        mcu_avr_destroy(model_of(state));
    }
}

static void mcu_start(Sim *sim, SimInstance *instance, void *state) {
    if (model_of(state) != NULL) {
        mcu_avr_start(model_of(state), sim, instance);
    }
}

static void mcu_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                            LogicValue old_value, LogicValue new_value) {
    (void)sim;
    (void)instance;
    if (model_of(state) != NULL) {
        mcu_avr_pin_changed(model_of(state), pin, old_value, new_value);
    }
}

static void mcu_timer(Sim *sim, SimInstance *instance, void *state, uint32_t timer_id) {
    (void)sim;
    (void)instance;
    if (model_of(state) != NULL) {
        mcu_avr_timer(model_of(state), timer_id);
    }
}

static int mcu_memory_size(const void *state, uint32_t *words, unsigned *bits) {
    return model_of(state) != NULL ? mcu_avr_memory_size(model_of(state), words, bits) : 0;
}

static size_t mcu_memory_read(const void *state, uint32_t start, size_t count, uint32_t *values,
                              uint32_t *known) {
    return model_of(state) != NULL ? mcu_avr_memory_read(model_of(state), start, count, values, known)
                                   : 0;
}

static void mcu_memory_load(void *state, uint32_t start, const uint32_t *values, size_t count) {
    if (model_of(state) != NULL) {
        mcu_avr_memory_load(model_of(state), start, values, count);
    }
}

static int mcu_cpu_state(const void *state, struct CpuState *out) {
    return model_of(state) != NULL ? mcu_avr_cpu_state(model_of(state), out) : 0;
}

static const SimComponentType MCU_MODEL = {
    .name = "mcu",
    .pin_count = mcu_pin_count,
    .create = mcu_create,
    .destroy = mcu_destroy,
    .start = mcu_start,
    .pin_changed = mcu_pin_changed,
    .timer = mcu_timer,
    .memory_size = mcu_memory_size,
    .memory_read = mcu_memory_read,
    .memory_load = mcu_memory_load,
    .cpu_state = mcu_cpu_state,
};

const PartKindOps MCU_KIND = {
    PART_MCU, "mcu", &MCU_MODEL, DEVICE_KEYS, PART_KEYS,
    mcu_load_device, mcu_bind_pinout, mcu_load_part
};
