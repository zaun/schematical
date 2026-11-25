/*
 * timer (plan-48-E): 555-type timers, one section (555) or two (556). A
 * section is a latch Q with OUT = Q (strong) and DISCH = strong 0 while Q is
 * 0, released (Z) while Q is 1: the open-collector discharge transistor in
 * phase with the output. What sets Q depends on the section's mode, which
 * circuit.c chooses at start from the resistors and capacitors wired to it
 * (timer_rc.c) and hands over with timer_model_set:
 *
 *   latch     no capacitor on the timing pins: TRIG low sets Q, TRIG high with
 *             THRES high resets it, RESET low clears it (the default; the mode
 *             trace_check sees, a part alone)
 *   astable   Q is 1 for first_ps, then 0 for low_ps and 1 for high_ps, ...;
 *             TRIG and THRES are not read; RESET low holds Q at 0 and the
 *             cycle restarts when it is released
 *   one-shot  TRIG low while idle makes Q 1 for high_ps; Q stays 1 after the
 *             pulse while TRIG is still low
 *   stopped   Q is 0 always
 *
 * Every output change follows its cause by tpd, except the ends of an
 * astable period and of a one-shot pulse, which fall exactly on the times
 * the analysis computed. Unconnected pins read Z; an unnamed RESET reads as
 * inactive.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "timer.h"

static const char *const DEVICE_KEYS[] = {"process", "timers", "maxHz", "tpd", "vcc", "inputMax", NULL};
static const char *const PART_KEYS[] = {"vcc", "inputMax", NULL};
static const char *const SECTION_KEYS[] = {"trigger", "threshold", "discharge", "output", "reset",
                                           "control", NULL};
static const char *const PROCESSES[] = {"bipolar", "cmos", NULL};

typedef struct {
    const char *trigger, *threshold, *discharge, *output, *reset, *control;  /* reset, control: NULL = none */
} TimerSectionNames;

typedef struct {
    const char *process;
    TimerSectionNames sections[2];
    size_t section_count;
    uint64_t max_hz, tpd_ps;
    const PartVoltage *vcc;
} TimerDevice;

/* Bound to a pinout: the sections' pins, as indices into Part.pins. */
typedef struct {
    size_t section_count;
    TimerPins pins[2];
} TimerBound;

static void *timer_load_device(KindContext *ctx, const cJSON *json) {
    TimerDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *item, *sections, *section;
    const char *text;
    size_t i, k;

    if (device == NULL || kind_string(ctx, json, "process", &text) != 0) {
        return NULL;
    }
    for (i = 0; PROCESSES[i] != NULL; i++) {
        if (strcmp(PROCESSES[i], text) == 0) {
            device->process = PROCESSES[i];
        }
    }
    if (device->process == NULL) {
        kind_error(ctx, "unknown timer process \"%s\"", text);
        return NULL;
    }
    sections = kind_member(ctx, json, "timers", 1);
    if (sections == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(sections) || cJSON_GetArraySize(sections) < 1 || cJSON_GetArraySize(sections) > 2) {
        kind_error(ctx, "\"timers\" must have 1 or 2 entries");
        return NULL;
    }
    device->section_count = (size_t)cJSON_GetArraySize(sections);
    k = 0;
    cJSON_ArrayForEach(section, sections) {
        TimerSectionNames *names = &device->sections[k];

        kind_push(ctx, "timers[%zu]", k);
        if (!cJSON_IsObject(section) || kind_check_keys(ctx, section, SECTION_KEYS, NULL) != 0 ||
            kind_string(ctx, section, "trigger", &names->trigger) != 0 ||
            kind_string(ctx, section, "threshold", &names->threshold) != 0 ||
            kind_string(ctx, section, "discharge", &names->discharge) != 0 ||
            kind_string(ctx, section, "output", &names->output) != 0) {
            if (!kind_failed(ctx)) {
                kind_error(ctx, "a timer section must be an object");
            }
            kind_pop(ctx);
            return NULL;
        }
        item = kind_member(ctx, section, "reset", 0);
        if (item != NULL && kind_string(ctx, section, "reset", &names->reset) != 0) {
            kind_pop(ctx);
            return NULL;
        }
        item = kind_member(ctx, section, "control", 0);
        if (item != NULL && kind_string(ctx, section, "control", &names->control) != 0) {
            kind_pop(ctx);
            return NULL;
        }
        kind_pop(ctx);
        k++;
    }
    item = kind_member(ctx, json, "maxHz", 1);
    if (item == NULL) {
        return NULL;
    }
    if (!cJSON_IsNumber(item) || !(item->valuedouble >= 1) || item->valuedouble > 100000000.0 ||
        item->valuedouble != (double)(uint64_t)item->valuedouble) {
        kind_error(ctx, "\"maxHz\" must be a whole number from 1 to 100000000");
        return NULL;
    }
    device->max_hz = (uint64_t)item->valuedouble;
    item = kind_member(ctx, json, "tpd", 1);
    if (item == NULL) {
        return NULL;
    }
    if (cJSON_IsNumber(item) && !(item->valuedouble > 0)) {
        kind_error(ctx, "\"tpd\" must be above 0");
        return NULL;
    }
    if (kind_ns(ctx, item, "tpd", &device->tpd_ps) != 0) {
        return NULL;
    }
    if (device->tpd_ps == 0) {
        kind_error(ctx, "\"tpd\" must be above 0");
        return NULL;
    }
    if (kind_voltage(ctx, json, NULL, &device->vcc) != 0) {
        return NULL;
    }
    return device;
}

/* A named pin with the role it needs; `seen` marks the pins sections use. */
static int bind_name(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *name,
                     PinRole role, const char *role_text, unsigned char *seen, size_t *out) {
    if (kind_pin(ctx, pins, pin_count, name, out) != 0) {
        return -1;
    }
    if (seen[*out]) {
        return kind_error(ctx, "timer pin \"%s\" is named twice", name);
    }
    seen[*out] = 1;
    if (pins[*out].role != role) {
        return kind_error(ctx, "timer pin \"%s\" must be %s", name, role_text);
    }
    return 0;
}

static void *timer_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins, size_t pin_count) {
    const TimerDevice *device = raw;
    TimerBound *bound = kind_alloc(ctx, sizeof(*bound));
    unsigned char *seen = kind_alloc(ctx, pin_count);
    size_t i, k, power = SIZE_MAX, ground = SIZE_MAX, powers = 0, grounds = 0;

    if (bound == NULL || seen == NULL) {
        return NULL;
    }
    bound->section_count = device->section_count;
    for (i = 0; i < pin_count; i++) {
        if (pins[i].role == PIN_POWER) {
            power = i;
            powers++;
        } else if (pins[i].role == PIN_GROUND) {
            ground = i;
            grounds++;
        }
    }
    if (powers != 1 || grounds != 1) {
        kind_error(ctx, "a timer has one power pin and one ground pin");
        return NULL;
    }
    for (k = 0; k < device->section_count; k++) {
        const TimerSectionNames *names = &device->sections[k];
        TimerPins *p = &bound->pins[k];

        p->vcc = power;
        p->gnd = ground;
        p->reset = SIZE_MAX;
        p->cont = SIZE_MAX;
        if (bind_name(ctx, pins, pin_count, names->trigger, PIN_INPUT, "an input", seen, &p->trig) != 0 ||
            bind_name(ctx, pins, pin_count, names->threshold, PIN_INPUT, "an input", seen, &p->thres) != 0 ||
            bind_name(ctx, pins, pin_count, names->discharge, PIN_OUTPUT, "an output", seen, &p->disch) != 0 ||
            bind_name(ctx, pins, pin_count, names->output, PIN_OUTPUT, "an output", seen, &p->out) != 0) {
            return NULL;
        }
        if (names->reset != NULL &&
            bind_name(ctx, pins, pin_count, names->reset, PIN_INPUT, "an input", seen, &p->reset) != 0) {
            return NULL;
        }
        if (names->control != NULL &&
            bind_name(ctx, pins, pin_count, names->control, PIN_INPUT, "an input", seen, &p->cont) != 0) {
            return NULL;
        }
    }
    return bound;
}

static int timer_load_part(KindContext *ctx, const void *raw, const void *bound, const cJSON *json,
                           Part *out) {
    const TimerDevice *device = raw;
    PartTimer *info = kind_alloc(ctx, sizeof(*info));

    (void)bound;
    if (info == NULL) {
        return -1;
    }
    info->timer_count = device->section_count;
    info->process = device->process;
    info->max_hz = device->max_hz;
    info->tpd_ps = device->tpd_ps;
    out->timer = info;
    return kind_part_voltage(ctx, json, device->vcc, &out->vcc);
}

static int timer_pin_open_drain(const Part *part, size_t pin) {
    const TimerBound *bound = part_internal(part)->bound;
    size_t k;

    for (k = 0; k < bound->section_count; k++) {
        if (bound->pins[k].disch == pin) {
            return 1;
        }
    }
    return 0;
}

static int timer_pin_analog(const Part *part, size_t pin) {
    const TimerBound *bound = part_internal(part)->bound;
    size_t k;

    for (k = 0; k < bound->section_count; k++) {
        if (bound->pins[k].cont == pin) {
            return 1;
        }
    }
    return 0;
}

int timer_section_pins(const Part *part, unsigned section, TimerPins *out) {
    const TimerBound *bound;

    if (part == NULL || part->kind != PART_TIMER || section >= part->timer->timer_count) {
        return 0;
    }
    bound = part_internal(part)->bound;
    *out = bound->pins[section];
    return 1;
}

/* ---- Model --------------------------------------------------------------- */

typedef struct {
    TimerPins pins;
    TimerState timing;
    LogicValue q;
    uint32_t epoch;       /* a timer event of an older epoch is stale */
    int started;          /* begin_section has run */
    int active;           /* astable: a cycle is running */
    int high;             /* astable: the high phase is running */
    int pulsing;          /* one-shot: a pulse is running */
    int held;             /* one-shot: the pulse ended with TRIG still low; Q stays 1 */
} Section;

typedef struct {
    const Part *part;
    uint64_t tpd_ps;
    size_t section_count;
    Section sections[2];
} TimerModel;

static size_t timer_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *timer_create(const void *config) {
    const Part *part = config;
    const TimerBound *bound = part_internal(part)->bound;
    TimerModel *model = calloc(1, sizeof(*model));
    size_t k;

    if (model == NULL) {
        return NULL;
    }
    model->part = part;
    model->tpd_ps = part->timer->tpd_ps;
    model->section_count = bound->section_count;
    for (k = 0; k < model->section_count; k++) {
        model->sections[k].pins = bound->pins[k];
        model->sections[k].timing.mode = TIMER_LATCH;
        model->sections[k].timing.reason = TIMER_OK;
    }
    return model;
}

static void timer_destroy(void *state) {
    free(state);
}

void timer_model_set(void *state, unsigned section, const TimerState *timing) {
    TimerModel *model = state;

    if (model != NULL && section < model->section_count) {
        model->sections[section].timing = *timing;
    }
}

int timer_model_get(const void *state, unsigned section, TimerState *out) {
    const TimerModel *model = state;

    if (model == NULL || section >= model->section_count) {
        return 0;
    }
    *out = model->sections[section].timing;
    return 1;
}

static LogicValue read_pin(const Sim *sim, const SimInstance *instance, size_t pin) {
    return pin == SIZE_MAX ? LOGIC_1 : sim_read_pin(sim, instance, pin);
}

/* Q -> OUT and DISCH, `delay` from now. */
static void drive_q(Sim *sim, SimInstance *instance, const Section *section, SimTime delay) {
    LogicValue out = section->q == LOGIC_1 ? LOGIC_1 : section->q == LOGIC_0 ? LOGIC_0 : LOGIC_X;
    LogicValue disch = section->q == LOGIC_1 ? LOGIC_Z : section->q == LOGIC_0 ? LOGIC_0 : LOGIC_X;

    sim_schedule_inertial_drive(sim, instance, section->pins.out, out, DRIVE_STRONG, delay);
    sim_schedule_inertial_drive(sim, instance, section->pins.disch, disch, DRIVE_STRONG, delay);
}

static uint32_t timer_id(const Section *section, unsigned index) {
    return (section->epoch << 1) | (index & 1u);
}

/* Drops the pending period or pulse end and everything that depends on it. */
static void cancel(Section *section) {
    section->epoch++;
    section->active = 0;
    section->high = 0;
    section->pulsing = 0;
    section->held = 0;
}

static void set_q(Sim *sim, SimInstance *instance, Section *section, LogicValue q, SimTime delay) {
    section->q = q;
    drive_q(sim, instance, section, delay);
}

/* ---- Latch ----------------------------------------------------------------- */

static void evaluate_latch(Sim *sim, SimInstance *instance, TimerModel *model, Section *section) {
    LogicValue reset = read_pin(sim, instance, section->pins.reset);
    LogicValue trig = sim_read_pin(sim, instance, section->pins.trig);
    LogicValue thres = sim_read_pin(sim, instance, section->pins.thres);
    LogicValue next = section->q;

    if (reset == LOGIC_0) {
        next = LOGIC_0;
    } else if (reset == LOGIC_X) {
        next = LOGIC_X;
    } else if (trig == LOGIC_0) {
        next = LOGIC_1;
    } else if (trig == LOGIC_1) {
        if (thres == LOGIC_1) {
            next = LOGIC_0;
        } else if (thres != LOGIC_0 && section->q == LOGIC_1) {
            next = LOGIC_X;
        }
    } else if (!(thres == LOGIC_1 && section->q == LOGIC_0)) {
        next = LOGIC_X;
    }
    if (next != section->q) {
        set_q(sim, instance, section, next, model->tpd_ps);
    }
}

/* ---- Astable --------------------------------------------------------------- */

/* Q goes high `delay` from now and the high phase lasts `width`. */
static void start_high(Sim *sim, SimInstance *instance, Section *section, unsigned index, SimTime delay,
                       SimTime width) {
    section->epoch++;
    section->high = 1;
    set_q(sim, instance, section, LOGIC_1, delay);
    sim_schedule_timer(sim, instance, timer_id(section, index), delay + width);
}

static void evaluate_astable(Sim *sim, SimInstance *instance, TimerModel *model, Section *section,
                             unsigned index, int starting) {
    LogicValue reset = read_pin(sim, instance, section->pins.reset);

    if (reset == LOGIC_0 || reset == LOGIC_X) {
        LogicValue q = reset == LOGIC_0 ? LOGIC_0 : LOGIC_X;

        cancel(section);
        if (section->q != q || starting) {
            set_q(sim, instance, section, q, starting ? 0 : model->tpd_ps);
        }
        return;
    }
    if (!section->active) {
        /* a new cycle: at start at once, after a RESET release tpd later */
        section->active = 1;
        start_high(sim, instance, section, index, starting ? 0 : model->tpd_ps, section->timing.first_ps);
    }
}

static void timer_astable_end(Sim *sim, SimInstance *instance, Section *section, unsigned index) {
    if (section->high) {
        section->high = 0;
        section->q = LOGIC_0;
        drive_q(sim, instance, section, 0);
        sim_schedule_timer(sim, instance, timer_id(section, index), section->timing.low_ps);
    } else {
        section->high = 1;
        section->q = LOGIC_1;
        drive_q(sim, instance, section, 0);
        sim_schedule_timer(sim, instance, timer_id(section, index), section->timing.high_ps);
    }
}

/* ---- One-shot -------------------------------------------------------------- */

static void evaluate_one_shot(Sim *sim, SimInstance *instance, TimerModel *model, Section *section,
                              unsigned index, int starting) {
    LogicValue reset = read_pin(sim, instance, section->pins.reset);
    LogicValue trig = sim_read_pin(sim, instance, section->pins.trig);

    if (reset == LOGIC_0 || reset == LOGIC_X) {
        LogicValue q = reset == LOGIC_0 ? LOGIC_0 : LOGIC_X;

        cancel(section);
        if (section->q != q || starting) {
            set_q(sim, instance, section, q, starting ? 0 : model->tpd_ps);
        }
        return;
    }
    if (section->pulsing) {
        return;
    }
    if (section->held) {
        if (trig == LOGIC_1) {
            section->held = 0;
            set_q(sim, instance, section, LOGIC_0, model->tpd_ps);
        }
        return;
    }
    if (section->q == LOGIC_X && trig == LOGIC_1) {
        set_q(sim, instance, section, LOGIC_0, model->tpd_ps);
    }
    if (trig == LOGIC_0 && section->q != LOGIC_1) {
        section->epoch++;
        section->pulsing = 1;
        set_q(sim, instance, section, LOGIC_1, starting ? 0 : model->tpd_ps);
        sim_schedule_timer(sim, instance, timer_id(section, index),
                           (starting ? 0 : model->tpd_ps) + section->timing.high_ps);
    }
}

static void timer_one_shot_end(Sim *sim, SimInstance *instance, Section *section) {
    section->pulsing = 0;
    if (sim_read_pin(sim, instance, section->pins.trig) == LOGIC_1) {
        set_q(sim, instance, section, LOGIC_0, 0);
    } else {
        section->held = 1;
    }
}

/* ---- Engine callbacks ------------------------------------------------------- */

static void evaluate(Sim *sim, SimInstance *instance, TimerModel *model, unsigned index, int starting) {
    Section *section = &model->sections[index];

    switch (section->timing.mode) {
    case TIMER_LATCH:
        evaluate_latch(sim, instance, model, section);
        break;
    case TIMER_ASTABLE:
        evaluate_astable(sim, instance, model, section, index, starting);
        break;
    case TIMER_ONE_SHOT:
        evaluate_one_shot(sim, instance, model, section, index, starting);
        break;
    case TIMER_STOPPED:
        break;
    }
}

/* The first evaluation of a section, as an engine event just after the time-0 drives of
   the other parts have settled (a pin reads Z until then). */
#define START_ID 0x80000000u

static void begin_section(Sim *sim, SimInstance *instance, TimerModel *model, unsigned k) {
    Section *section = &model->sections[k];

    section->epoch = 0;
    section->active = section->high = section->pulsing = section->held = 0;
    section->q = LOGIC_0;
    if (section->timing.mode == TIMER_LATCH) {
        /* a latch's first value comes from its inputs; until then OUT is low */
        evaluate_latch(sim, instance, model, section);
        if (section->q == LOGIC_0) {
            drive_q(sim, instance, section, 0);
        }
    } else if (section->timing.mode == TIMER_STOPPED) {
        drive_q(sim, instance, section, 0);
    } else {
        evaluate(sim, instance, model, k, 1);
        if (section->q == LOGIC_0 && !section->pulsing) {
            drive_q(sim, instance, section, 0);   /* idle: OUT low */
        }
    }
}

static void timer_start(Sim *sim, SimInstance *instance, void *raw) {
    TimerModel *model = raw;
    unsigned k;

    for (k = 0; k < model->section_count; k++) {
        model->sections[k].started = 0;
        sim_schedule_timer(sim, instance, START_ID | k, 0);
    }
}

static void timer_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin, LogicValue old_value,
                              LogicValue new_value) {
    TimerModel *model = raw;
    unsigned k;

    (void)old_value;
    (void)new_value;
    for (k = 0; k < model->section_count; k++) {
        const TimerPins *p = &model->sections[k].pins;

        if (!model->sections[k].started) {
            continue;
        }
        if (pin == p->reset || pin == p->trig || (pin == p->thres && model->sections[k].timing.mode == TIMER_LATCH)) {
            evaluate(sim, instance, model, k, 0);
        }
    }
}

static void timer_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t id) {
    TimerModel *model = raw;
    unsigned index = id & 1u;
    Section *section;

    if (id & START_ID) {
        index = id & 0x7fffffffu;
        if (index < model->section_count) {
            model->sections[index].started = 1;
            begin_section(sim, instance, model, index);
        }
        return;
    }
    if (index >= model->section_count) {
        return;
    }
    section = &model->sections[index];
    if ((id >> 1) != section->epoch) {
        return;   /* a RESET or a newer start dropped it */
    }
    if (section->timing.mode == TIMER_ASTABLE) {
        timer_astable_end(sim, instance, section, index);
    } else if (section->timing.mode == TIMER_ONE_SHOT) {
        timer_one_shot_end(sim, instance, section);
    }
}

static const SimComponentType TIMER_MODEL = {
    .name = "timer",
    .pin_count = timer_pin_count,
    .create = timer_create,
    .destroy = timer_destroy,
    .start = timer_start,
    .pin_changed = timer_pin_changed,
    .timer = timer_timer,
};

const PartKindOps TIMER_KIND = {
    PART_TIMER, "timer", &TIMER_MODEL, DEVICE_KEYS, PART_KEYS,
    timer_load_device, timer_bind_pinout, timer_load_part, timer_pin_open_drain, timer_pin_analog
};
