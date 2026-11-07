/*
 * pio: parallel I/O peripherals on the 65xx bus (plan-14): the W65C21 PIA
 * and the W65C22 VIA.
 *
 * The device's "model" key picks a PioModel (pio.h); this file holds the
 * loader (model dispatch, grades) and the one SimComponentType that forwards
 * every hook to the part's model. The bus itself is bus65.c.
 */

#include "pio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The model table (NULL-terminated). */
static const PioModel *const MODELS[] = {
    &PIA6521_MODEL,   /* plan-14-B */
    NULL
};

/* Every key a device may carry: "model", "grades", then the union of every
   model's keys (each model's own list is checked after "model"). */
static const char *const DEVICE_KEYS[] = {
    "model", "grades",
    /* the shared 65xx keys (pio_load_names) */
    "phi2", "rw", "reset", "data", "csHigh", "csLow", "rs", "pa", "pb",
    "ca1", "ca2", "cb1", "cb2",
    /* pia6521 */
    "irqa", "irqb",
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

static int load_list(KindContext *ctx, const cJSON *json, const char *key, size_t count,
                     const char *const **out) {
    size_t n;

    if (kind_names(ctx, json, key, out, &n) != 0) {
        return -1;
    }
    if (n != count) {
        return kind_error(ctx, "\"%s\" needs exactly %zu pin names", key, count);
    }
    return 0;
}

int pio_load_names(KindContext *ctx, const cJSON *json, PioNames *names, size_t cs_high_count,
                   size_t cs_low_count, size_t rs_count) {
    names->cs_high_count = cs_high_count;
    names->cs_low_count = cs_low_count;
    names->rs_count = rs_count;
    if (kind_string(ctx, json, "phi2", &names->phi2) != 0 ||
        kind_string(ctx, json, "rw", &names->rw) != 0 ||
        kind_string(ctx, json, "reset", &names->reset) != 0 ||
        load_list(ctx, json, "data", BUS65_DATA, &names->data) != 0 ||
        load_list(ctx, json, "csHigh", cs_high_count, &names->cs_high) != 0 ||
        load_list(ctx, json, "csLow", cs_low_count, &names->cs_low) != 0 ||
        load_list(ctx, json, "rs", rs_count, &names->rs) != 0 ||
        load_list(ctx, json, "pa", PIO_PORT_BITS, &names->pa) != 0 ||
        load_list(ctx, json, "pb", PIO_PORT_BITS, &names->pb) != 0 ||
        kind_string(ctx, json, "ca1", &names->ca1) != 0 ||
        kind_string(ctx, json, "ca2", &names->ca2) != 0 ||
        kind_string(ctx, json, "cb1", &names->cb1) != 0 ||
        kind_string(ctx, json, "cb2", &names->cb2) != 0) {
        return -1;
    }
    return 0;
}

int pio_bind_pin(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *key,
                 const char *name, size_t *out) {
    kind_push(ctx, "%s", key);
    if (kind_pin(ctx, pins, pin_count, name, out) != 0) {
        return -1;
    }
    kind_pop(ctx);
    return 0;
}

static int bind_list(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *key,
                     const char *const *names, size_t count, size_t *out) {
    char path[32];
    size_t i;

    for (i = 0; i < count; i++) {
        snprintf(path, sizeof path, "%s[%zu]", key, i);
        if (pio_bind_pin(ctx, pins, pin_count, path, names[i], &out[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

int pio_bind_names(KindContext *ctx, const PioNames *names, const PartPin *pins, size_t pin_count,
                   PioPins *out) {
    Bus65Pins *bus = &out->bus;

    bus->cs_high_count = names->cs_high_count;
    bus->cs_low_count = names->cs_low_count;
    bus->rs_count = names->rs_count;
    if (pio_bind_pin(ctx, pins, pin_count, "phi2", names->phi2, &bus->phi2) != 0 ||
        pio_bind_pin(ctx, pins, pin_count, "rw", names->rw, &bus->rwb) != 0 ||
        pio_bind_pin(ctx, pins, pin_count, "reset", names->reset, &out->reset) != 0 ||
        bind_list(ctx, pins, pin_count, "data", names->data, BUS65_DATA, bus->data) != 0 ||
        bind_list(ctx, pins, pin_count, "csHigh", names->cs_high, names->cs_high_count,
                  bus->cs_high) != 0 ||
        bind_list(ctx, pins, pin_count, "csLow", names->cs_low, names->cs_low_count,
                  bus->cs_low) != 0 ||
        bind_list(ctx, pins, pin_count, "rs", names->rs, names->rs_count, bus->rs) != 0 ||
        bind_list(ctx, pins, pin_count, "pa", names->pa, PIO_PORT_BITS, out->pa) != 0 ||
        bind_list(ctx, pins, pin_count, "pb", names->pb, PIO_PORT_BITS, out->pb) != 0 ||
        pio_bind_pin(ctx, pins, pin_count, "ca1", names->ca1, &out->ca1) != 0 ||
        pio_bind_pin(ctx, pins, pin_count, "ca2", names->ca2, &out->ca2) != 0 ||
        pio_bind_pin(ctx, pins, pin_count, "cb1", names->cb1, &out->cb1) != 0 ||
        pio_bind_pin(ctx, pins, pin_count, "cb2", names->cb2, &out->cb2) != 0) {
        return -1;
    }
    return 0;
}

/* ---- Kind: devices, pinouts, parts ------------------------------------------------ */

static int load_grade(KindContext *ctx, const PioModel *model, const cJSON *json, PioGrade *grade) {
    static const char *const EXTRA[] = {"note", "vcc", "inputMax", NULL};
    const cJSON *note = cJSON_GetObjectItemCaseSensitive(json, "note");
    size_t i;

    if (!cJSON_IsObject(json)) {
        return kind_error(ctx, "a grade is an object of timing values");
    }
    if (kind_check_keys(ctx, json, model->timing, EXTRA) != 0) {
        return -1;
    }
    for (i = 0; model->timing[i] != NULL && i < PIO_MAX_TIMING; i++) {
        const cJSON *value = cJSON_GetObjectItemCaseSensitive(json, model->timing[i]);

        grade->t[i] = 0;
        if (value != NULL && kind_ns(ctx, value, model->timing[i], &grade->t[i]) != 0) {
            return -1;
        }
        if (i < model->positive && grade->t[i] == 0) {
            return kind_error(ctx, "\"%s\" must be more than 0", model->timing[i]);
        }
    }
    if (note != NULL && !cJSON_IsString(note)) {
        return kind_error(ctx, "\"note\" must be a string");
    }
    return kind_voltage(ctx, json, NULL, &grade->vcc);
}

static void *pio_load_device(KindContext *ctx, const cJSON *json) {
    PioDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *grades;
    const cJSON *item;
    const char *name;
    size_t i;
    size_t n = 0;

    if (device == NULL) {
        return NULL;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "model") == NULL) {
        kind_error(ctx, "\"model\" is required");
        return NULL;
    }
    if (kind_string(ctx, json, "model", &name) != 0) {
        return NULL;
    }
    for (i = 0; MODELS[i] != NULL; i++) {
        if (strcmp(MODELS[i]->name, name) == 0) {
            device->model = MODELS[i];
        }
    }
    if (device->model == NULL) {
        kind_error(ctx, "unknown pio model \"%s\"", name);
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
    device->data = device->model->load_device(ctx, json);
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
        PioGrade *grade = &device->grades[n];
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
        grade->info.grade = device->grade_names[n];
        grade->info.max_hz = grade->t[0] > 0 ? 1000000000000ull / grade->t[0] : 0;
        n++;
    }
    return device;
}

typedef struct {
    const PioDevice *device;
    const void *pins;
} PioBound;

static void *pio_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                             size_t pin_count) {
    const PioDevice *device = raw;
    PioBound *bound = kind_alloc(ctx, sizeof(*bound));

    if (bound == NULL) {
        return NULL;
    }
    bound->device = device;
    bound->pins = device->model->bind(ctx, device->data, pins, pin_count);
    return bound->pins != NULL ? bound : NULL;
}

static int pio_load_part(KindContext *ctx, const void *raw, const void *bound, const cJSON *json,
                         Part *out) {
    const PioDevice *device = raw;
    const char *grade;
    size_t i;

    (void)bound;
    if (kind_string(ctx, json, "grade", &grade) != 0) {
        return -1;
    }
    for (i = 0; i < device->grade_count; i++) {
        if (strcmp(device->grade_names[i], grade) == 0) {
            out->pio = &device->grades[i].info;
            kind_set_part_data(out, &device->grades[i]);
            return kind_part_voltage(ctx, json, device->grades[i].vcc, &out->vcc);
        }
    }
    return kind_error(ctx, "grade \"%s\" is not in \"grades\"", grade);
}

static int pio_pin_open_drain(const Part *part, size_t pin) {
    const PartInternal *internal = part_internal(part);
    const PioBound *bound = internal->bound;

    return bound->device->model->pin_open_drain != NULL &&
           bound->device->model->pin_open_drain(bound->device->data, bound->pins, pin);
}

/* ---- The model: one type, forwarding to the part's model ---------------------- */

static size_t pio_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *pio_create(const void *config) {
    const Part *part = config;
    const PartInternal *internal = part_internal(part);
    const PioBound *bound = internal->bound;

    return bound->device->model->create(part, bound->device->data, bound->pins, internal->data);
}

static const PioModel *model_of(const void *state) {
    return ((const PioBase *)state)->model;
}

static void pio_destroy(void *state) {
    if (state != NULL) {
        model_of(state)->destroy(state);
    }
}

static void pio_start(Sim *sim, SimInstance *instance, void *state) {
    model_of(state)->start(sim, instance, state);
}

static void pio_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                            LogicValue old_value, LogicValue new_value) {
    model_of(state)->pin_changed(sim, instance, state, pin, old_value, new_value);
}

static void pio_timer(Sim *sim, SimInstance *instance, void *state, uint32_t timer_id) {
    if (model_of(state)->timer != NULL) {
        model_of(state)->timer(sim, instance, state, timer_id);
    }
}

static const SimComponentType PIO_COMPONENT = {
    "pio", pio_pin_count, pio_create, pio_destroy, pio_start, pio_pin_changed, pio_timer
};

const PartKindOps PIO_KIND = {
    PART_PIO, "pio", &PIO_COMPONENT, DEVICE_KEYS, PART_KEYS,
    pio_load_device, pio_bind_pinout, pio_load_part, pio_pin_open_drain
};
