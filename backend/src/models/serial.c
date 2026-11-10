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
    /* plan-12-J */
    &MAX3100_MODEL,
    &MAX310X_MODEL,
    /* plan-12-H */
    &AY1013_MODEL,
    &I8251_MODEL,
    &I82510_MODEL,
    &ACIA6551_MODEL,
    &HS82C52_MODEL,
    &COM78C80X_MODEL,
    /* plan-12-I: the SPI-to-I2C bridge */
    &SC18IS604_MODEL,
    /* the USB-to-UART bridges */
    &USBUART_MODEL,
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
    "optionalPins", "sprPowerUp", "dmaMode1", "lsrClearOnGood", "breakMarkTicks",
    /* ns16550, plan-12-E */
    "triggerTables", "fifoCounters", "dld", "sampling", "xonAny", "powerSave", "mfPins",
    "startSample", "thrIntClear", "isrStatus", "resetOut", "divisorPowerUp", "sleepNeedsDivisor",
    "wakeIndicator", "mcr2Enhanced", "xonWriteOnly", "autoFlowLoopback", "lsrBreakHold",
    "statusWrite", "dlabHidesStatus", "fstat", "dmaHost", "chcClock", "timer", "variant5g",
    "irdaStrap",
    /* max3100, max310x (plan-12-J) */
    "shutdown", "testMode", "transceiver", "variant", "reset", "interrupt", "gpio",
    /* plan-12-H: ay1013 ("reset" above) */
    "transmitData", "receiveData", "receiveEnable", "statusEnable", "status", "control",
    "dataStrobe", "resetDav", "serialOut", "serialIn", "clocks", "resetClearsData",
    "resetClearsControl", "fiveBitStops", "cpuMode",
    /* i8251 ("bus", "clock" above) */
    "serial",
    /* hs82c52 */
    "hd6406",
    /* acia6551, plan-14-E */
    "w65c51n",
    /* com78c80x ("bus", "clock", "channels", "interrupt" above) */
    "lines",
    /* plan-12-I: the bridges (ns16550) and sc18is604 */
    "fastIrda", "dlPowerUp", "straps", "sleepPin", "i2c",
    /* usbuart ("channels", "reset" above) */
    "suspend", "suspendLow", "usbSupply",
    NULL
};
enum { FIRST_MODEL_KEY = 2 };

/* Every part key: the common ones, then the union of every model's part keys
   (plan-12-H: ay1013, acia6551; plan-12-E: ns16550 "intPin", the D variants'
   always-driven INT). */
static const char *const PART_KEYS[] = {
    "grade", "vcc", "inputMax",
    "fiveBitStops", "resetClearsData", "ctsA",
    "intPin",
    NULL
};

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

/* The host-side interface named by the device's "bus" style (PartSerial.host),
   else the model's own (usbuart: "usb"). A bus with no style, or none at all,
   is a parallel data bus. */
static const char *serial_host(const SerialModel *model, const cJSON *json) {
    static const char *const SERIAL_HOSTS[] = {"spi", "i2c", "i2c-spi"};
    const cJSON *bus = cJSON_GetObjectItemCaseSensitive(json, "bus");
    const cJSON *style = cJSON_IsObject(bus) ? cJSON_GetObjectItemCaseSensitive(bus, "style") : NULL;
    size_t i;

    if (model->host != NULL) {
        return model->host;
    }
    for (i = 0; cJSON_IsString(style) && i < sizeof(SERIAL_HOSTS) / sizeof(SERIAL_HOSTS[0]); i++) {
        if (strcmp(style->valuestring, SERIAL_HOSTS[i]) == 0) {
            return SERIAL_HOSTS[i];
        }
    }
    return "parallel";
}

/* "usbSupply": {"power": [names], "pins": [names], "vcc": [min, max]}. */
static int load_usb_supply(KindContext *ctx, const cJSON *json, SerialDevice *device) {
    static const char *const KEYS[] = {"power", "pins", "vcc", "inputMax", NULL};
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(json, "usbSupply");
    SerialUsbSupply *u;

    if (item == NULL) {
        return 0;
    }
    u = kind_alloc(ctx, sizeof(*u));
    if (u == NULL) {
        return -1;
    }
    kind_push(ctx, "usbSupply");
    if (kind_check_keys(ctx, item, KEYS, NULL) != 0 ||
        kind_names(ctx, item, "power", &u->power, &u->power_count) != 0) {
        return -1;
    }
    if (u->power_count == 0) {
        return kind_error(ctx, "\"power\" needs at least one pin name");
    }
    if (cJSON_GetObjectItemCaseSensitive(item, "pins") != NULL &&
        kind_names(ctx, item, "pins", &u->pins, &u->pin_count) != 0) {
        return -1;
    }
    if (kind_part_voltage(ctx, item, NULL, &u->vcc) != 0) {
        return -1;
    }
    kind_pop(ctx);
    device->usb_supply = u;
    return 0;
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
    if (device->data == NULL || load_usb_supply(ctx, json, device) != 0) {
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
        grade->info.host = serial_host(device->model, json);
        n++;
    }
    return device;
}

typedef struct {
    const SerialDevice *device;
    const void *pins;
    /* With usbSupply: [0] the I/O domain, [1] the USB domain. */
    size_t *power[2], *other[2];
    size_t power_count[2], other_count[2];
} SerialBound;

/* Splits a pinout into the usbSupply domain and the rest. */
static int bind_usb_supply(KindContext *ctx, const SerialUsbSupply *u, const PartPin *pins,
                           size_t pin_count, SerialBound *b) {
    unsigned char *usb = kind_alloc(ctx, pin_count ? pin_count : 1);
    size_t i, k, n;

    for (k = 0; k < 2; k++) {
        b->power[k] = kind_alloc(ctx, (pin_count ? pin_count : 1) * sizeof(size_t));
        b->other[k] = kind_alloc(ctx, (pin_count ? pin_count : 1) * sizeof(size_t));
        if (b->power[k] == NULL || b->other[k] == NULL) {
            return -1;
        }
    }
    if (usb == NULL) {
        return -1;
    }
    for (k = 0; k < u->power_count + u->pin_count; k++) {
        int is_power = k < u->power_count;
        const char *name = is_power ? u->power[k] : u->pins[k - u->power_count];
        size_t found[16];

        n = kind_pins(ctx, pins, pin_count, name, found, 16);
        if (n == 0) {
            return -1;
        }
        for (i = 0; i < n; i++) {
            if (is_power && pins[found[i]].role != PIN_POWER) {
                return kind_error(ctx, "\"usbSupply\": \"%s\" is not a power pin", name);
            }
            usb[found[i]] = 1;
        }
    }
    for (i = 0; i < pin_count; i++) {
        PinRole role = pins[i].role;

        k = usb[i];
        if (role == PIN_GROUND || role == PIN_NC || role == PIN_ABSENT) {
            continue;
        }
        if (role == PIN_POWER) {
            b->power[k][b->power_count[k]++] = i;
        } else {
            b->other[k][b->other_count[k]++] = i;
        }
    }
    if (b->power_count[0] == 0) {
        return kind_error(ctx, "\"usbSupply\": no power pin is left for the I/O supply");
    }
    return 0;
}

static void *serial_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                size_t pin_count) {
    const SerialDevice *device = raw;
    SerialBound *bound = kind_alloc(ctx, sizeof(*bound));

    if (bound == NULL) {
        return NULL;
    }
    bound->device = device;
    bound->pins = device->model->bind(ctx, device->data, pins, pin_count);
    if (bound->pins == NULL ||
        (device->usb_supply != NULL &&
         bind_usb_supply(ctx, device->usb_supply, pins, pin_count, bound) != 0)) {
        return NULL;
    }
    return bound;
}

static int serial_load_part(KindContext *ctx, const void *raw, const void *bound, const cJSON *json,
                            Part *out) {
    const SerialDevice *device = raw;
    const SerialBound *b = bound;
    SerialPartData *data;
    const cJSON *item;
    const char *grade;
    size_t i;

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
    if (kind_part_voltage(ctx, json, data->grade->vcc, &out->vcc) != 0) {
        return -1;
    }
    if (device->usb_supply != NULL) {
        PartSupplyDomain *domains = kind_alloc(ctx, 2 * sizeof(*domains));

        if (domains == NULL) {
            return -1;
        }
        for (i = 0; i < 2; i++) {
            domains[i].power_pins = b->power[i];
            domains[i].power_pin_count = b->power_count[i];
            domains[i].pins = b->other[i];
            domains[i].pin_count = b->other_count[i];
        }
        domains[0].vcc = out->vcc;
        domains[1].vcc = device->usb_supply->vcc;
        out->domains = domains;
        out->domain_count = 2;
    }
    return 0;
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

    if (channel >= model->channels(state) || !model->async(state, channel)) {
        return 0;
    }
    if (serial_line_queue(model->line(state, channel), bytes, count) != 0) {
        return 0;
    }
    if (model->queued != NULL) {
        model->queued(sim, instance, state, channel);
    }
    return 1;
}

int serial_configure(Sim *sim, SimInstance *instance, void *state, unsigned channel,
                     const SerialPortSettings *port) {
    const SerialModel *model = model_of(state);

    if (channel >= model->channels(state)) {
        return -1;
    }
    return model->configure != NULL && model->configure(sim, instance, state, channel, port);
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
