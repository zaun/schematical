/*
 * serial: bus-controller ICs that move bytes over a serial line (plan-12):
 * UARTs, USARTs, ACIAs and the I2C/SPI bridges.
 *
 * The device's "model" key picks a SerialModel (serial.h); this file holds
 * the loader (model dispatch, grades), the one SimComponentType that forwards
 * every hook to the part's model, and the terminal hooks (sent log, typing
 * into a channel's RX pin), which go through the model's SerialLine.
 */

#include "serial.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const SerialModel *const MODELS[] = {
    &NS16550_MODEL,
};
#define MODEL_COUNT (sizeof MODELS / sizeof MODELS[0])

/* Every key a device may carry: "model", "grades", then the union of every
   model's keys (each model's own list is checked after "model"). */
static const char *const DEVICE_KEYS[] = {
    "model", "grades",
    /* ns16550 */
    "bus", "clock", "channels", "fifo", "rxTrigger", "intPin", "dmaClass", "timeout",
    "channelSelect", "broadcastWrite", "rxTrigger64", "txTrigger", "tlr", "dmaPins", "dma",
    "intControl", "lcrReset", "sprReset", "tcrReset", "iirPriority", "lsr7Clear", "stopCheck",
    "statusReadDefer", "busSync", "majorityVote",
    "extended", "afr", "afrReset", "dlf", "autoFlow", "rtsTable", "xoffTable", "swFlow",
    "specialChar", "specialClear", "specialNeedsNoFlow", "ctsRtsClear", "sleep", "irda", "rs485",
    "prescaler", "lsrErrors", "mcrReset", "xonReset", "tcrNeedsDlab0", "dlabHidesUpper",
    "idRegs", "timeoutDelay", "divisorZero", "printerPort", "tristate", "absentTies",
    "optionalPins", "sprPowerUp", "dmaMode1",
    NULL
};
enum { FIRST_MODEL_KEY = 2 };

static const char *const PART_KEYS[] = {"grade", "vcc", "inputMax", NULL};

static int in_list(const char *const *list, const char *name) {
    if (list == NULL) {
        return 0;
    }
    for (; *list != NULL; list++) {
        if (strcmp(*list, name) == 0) {
            return 1;
        }
    }
    return 0;
}

/* ---- Shared helpers ------------------------------------------------------------ */

int serial_optional_name(KindContext *ctx, const cJSON *object, const char *key, const char **out) {
    *out = NULL;
    if (cJSON_GetObjectItemCaseSensitive(object, key) == NULL) {
        return 0;
    }
    return kind_string(ctx, object, key, out);
}

int serial_bind_pin(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *key,
                    const char *name, size_t *out) {
    *out = SERIAL_NO_PIN;
    if (name == NULL) {
        return 0;
    }
    kind_push(ctx, "%s", key);
    if (kind_pin(ctx, pins, pin_count, name, out) != 0) {
        return -1;
    }
    kind_pop(ctx);
    return 0;
}

int serial_bind_pin_absent(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *key,
                           const char *name, const SerialAbsent *absent, size_t *out) {
    size_t i;
    int found = 0;

    *out = SERIAL_NO_PIN;
    if (name == NULL) {
        return 0;
    }
    for (i = 0; i < pin_count; i++) {
        found |= strcmp(pins[i].name, name) == 0;
    }
    if (!found && absent != NULL) {
        for (i = 0; i < absent->tie_count; i++) {
            if (strcmp(absent->tie_names[i], name) == 0) {
                *out = absent->tie_levels[i] ? SERIAL_TIE_1 : SERIAL_TIE_0;
                return 0;
            }
        }
        for (i = 0; i < absent->optional_count; i++) {
            if (strcmp(absent->optional[i], name) == 0) {
                return 0;
            }
        }
    }
    return serial_bind_pin(ctx, pins, pin_count, key, name, out);
}

/* ---- Kind: devices, pinouts, parts ------------------------------------------------ */

static int load_grade(KindContext *ctx, const SerialModel *model, const cJSON *json,
                      SerialGrade *grade) {
    static const char *const EXTRA[] = {"note", "vcc", "inputMax", NULL};
    const cJSON *note = cJSON_GetObjectItemCaseSensitive(json, "note");
    size_t i;

    if (!cJSON_IsObject(json)) {
        return kind_error(ctx, "a grade is an object of timing values");
    }
    if (kind_check_keys(ctx, json, model->timing, EXTRA) != 0) {
        return -1;
    }
    for (i = 0; model->timing[i] != NULL && i < SERIAL_MAX_TIMING; i++) {
        const cJSON *value = cJSON_GetObjectItemCaseSensitive(json, model->timing[i]);

        grade->t[i] = 0;
        if (value != NULL && kind_ns(ctx, value, model->timing[i], &grade->t[i]) != 0) {
            return -1;
        }
    }
    if (note != NULL && !cJSON_IsString(note)) {
        return kind_error(ctx, "\"note\" must be a string");
    }
    return kind_voltage(ctx, json, NULL, &grade->vcc);
}

static void *serial_load_device(KindContext *ctx, const cJSON *json) {
    SerialDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *grades;
    const cJSON *item;
    const char *name;
    size_t i;
    size_t n = 0;

    if (device == NULL || kind_string(ctx, json, "model", &name) != 0) {
        return NULL;
    }
    for (i = 0; i < MODEL_COUNT; i++) {
        if (strcmp(MODELS[i]->name, name) == 0) {
            device->model = MODELS[i];
        }
    }
    if (device->model == NULL) {
        kind_error(ctx, "unknown model \"%s\"", name);
        return NULL;
    }
    /* A key of another model. */
    cJSON_ArrayForEach(item, json) {
        if (in_list(DEVICE_KEYS + FIRST_MODEL_KEY, item->string) &&
            !in_list(device->model->keys, item->string)) {
            kind_error(ctx, "\"%s\" is not a key of model \"%s\"", item->string, name);
            return NULL;
        }
    }
    device->channels = 1;
    device->data = device->model->load_device(ctx, json, &device->channels);
    if (device->data == NULL) {
        return NULL;
    }

    grades = kind_member(ctx, json, "grades", 1);
    if (grades == NULL) {
        return NULL;
    }
    if (!cJSON_IsObject(grades) || grades->child == NULL) {
        kind_error(ctx, "\"grades\" must be a non-empty object");
        return NULL;
    }
    device->grade_count = (size_t)cJSON_GetArraySize(grades);
    device->grade_names = kind_alloc(ctx, device->grade_count * sizeof(*device->grade_names));
    device->grades = kind_alloc(ctx, device->grade_count * sizeof(*device->grades));
    if (device->grade_names == NULL || device->grades == NULL) {
        return NULL;
    }
    cJSON_ArrayForEach(item, grades) {
        SerialGrade *grade = &device->grades[n];
        const cJSON *other;

        for (other = grades->child; other != item; other = other->next) {
            if (strcmp(other->string, item->string) == 0) {
                kind_error(ctx, "duplicate key \"%s\"", item->string);
                return NULL;
            }
        }
        kind_push(ctx, "grades.%s", item->string);
        device->grade_names[n] = kind_strdup(ctx, item->string);
        if (device->grade_names[n] == NULL || load_grade(ctx, device->model, item, grade) != 0) {
            return NULL;
        }
        kind_pop(ctx);
        grade->info.model = device->model->name;
        grade->info.channels = device->channels;
        grade->info.grade = device->grade_names[n];
        n++;
    }
    return device;
}

typedef struct {
    const SerialDevice *device;
    const void *pins;
} SerialBound;

static void *serial_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                size_t pin_count) {
    const SerialDevice *device = raw;
    SerialBound *bound = kind_alloc(ctx, sizeof(*bound));

    if (bound == NULL) {
        return NULL;
    }
    bound->device = device;
    bound->pins = device->model->bind(ctx, device->data, pins, pin_count);
    return bound->pins != NULL ? bound : NULL;
}

static int serial_load_part(KindContext *ctx, const void *raw, const void *bound, const cJSON *json,
                            Part *out) {
    const SerialDevice *device = raw;
    SerialPartData *data;
    const cJSON *item;
    const char *grade;
    size_t i;

    (void)bound;
    /* A part key of another model. */
    cJSON_ArrayForEach(item, json) {
        int other = 0;

        for (i = 0; i < MODEL_COUNT; i++) {
            other |= MODELS[i] != device->model && in_list(MODELS[i]->part_keys, item->string);
        }
        if (other && !in_list(device->model->part_keys, item->string)) {
            return kind_error(ctx, "\"%s\" is not a part key of model \"%s\"", item->string,
                              device->model->name);
        }
    }
    if (kind_string(ctx, json, "grade", &grade) != 0) {
        return -1;
    }
    data = kind_alloc(ctx, sizeof(*data));
    if (data == NULL) {
        return -1;
    }
    for (i = 0; i < device->grade_count; i++) {
        if (strcmp(device->grade_names[i], grade) == 0) {
            data->grade = &device->grades[i];
        }
    }
    if (data->grade == NULL) {
        return kind_error(ctx, "grade \"%s\" is not in \"grades\"", grade);
    }
    if (device->model->load_part != NULL) {
        data->options = device->model->load_part(ctx, device->data, json);
        if (kind_failed(ctx)) {
            return -1;
        }
    }
    out->serial = &data->grade->info;
    kind_set_part_data(out, data);
    return kind_part_voltage(ctx, json, data->grade->vcc, &out->vcc);
}

static int serial_pin_open_drain(const Part *part, size_t pin) {
    const PartInternal *internal = part_internal(part);
    const SerialBound *bound = internal->bound;
    const SerialPartData *data = internal->data;

    return bound->device->model->pin_open_drain != NULL &&
           bound->device->model->pin_open_drain(bound->device->data, bound->pins, data->options,
                                                pin);
}

/* ---- The model: one type, forwarding to the part's model ---------------------- */

static size_t serial_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *serial_create(const void *config) {
    const Part *part = config;
    const PartInternal *internal = part_internal(part);
    const SerialBound *bound = internal->bound;
    const SerialPartData *data = internal->data;

    return bound->device->model->create(part, bound->device->data, bound->pins, data->grade,
                                        data->options);
}

static const SerialModel *model_of(const void *state) {
    return ((const SerialBase *)state)->model;
}

static void serial_destroy(void *state) {
    if (state != NULL) {
        model_of(state)->destroy(state);
    }
}

static void serial_start(Sim *sim, SimInstance *instance, void *state) {
    model_of(state)->start(sim, instance, state);
}

static void serial_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                               LogicValue old_value, LogicValue new_value) {
    model_of(state)->pin_changed(sim, instance, state, pin, old_value, new_value);
}

static void serial_timer(Sim *sim, SimInstance *instance, void *state, uint32_t timer_id) {
    if (model_of(state)->timer != NULL) {
        model_of(state)->timer(sim, instance, state, timer_id);
    }
}

static size_t serial_channels(const void *state) {
    return model_of(state)->channels(state);
}

static size_t serial_sent(const void *state, unsigned channel, size_t start, uint16_t *frames,
                          size_t cap) {
    const SerialModel *model = model_of(state);
    const SerialLine *line;

    if (channel >= model->channels(state)) {
        return 0;
    }
    line = model->line((void *)state, channel);
    if (frames == NULL || cap == 0) {
        return line->log_count;
    }
    serial_line_sent(line, start, frames, cap);
    return line->log_count;
}

static int serial_send(Sim *sim, SimInstance *instance, void *state, unsigned channel,
                       const uint8_t *bytes, size_t count) {
    const SerialModel *model = model_of(state);

    (void)sim;
    (void)instance;
    if (channel >= model->channels(state) || !model->async(state, channel)) {
        return 0;
    }
    return serial_line_queue(model->line(state, channel), bytes, count) == 0;
}

size_t serial_rx_pin(const void *state, unsigned channel) {
    const SerialModel *model = model_of(state);

    if (channel >= model->channels(state)) {
        return SIZE_MAX;
    }
    return model->line((void *)state, channel)->rx_pin;
}

static const SimComponentType SERIAL_COMPONENT = {
    "serial", serial_pin_count, serial_create, serial_destroy, serial_start, serial_pin_changed,
    serial_timer, NULL, NULL, NULL, NULL, NULL, NULL, NULL /* cpu_state */, NULL /* lcd_state */,
    serial_channels, serial_sent, serial_send
};

const PartKindOps SERIAL_KIND = {
    PART_SERIAL, "serial", &SERIAL_COMPONENT, DEVICE_KEYS, PART_KEYS,
    serial_load_device, serial_bind_pinout, serial_load_part, serial_pin_open_drain
};
