/*
 * cpu: microprocessors that run programs inside the simulation (plan-09).
 *
 * The device's "core" key picks the bus adapter: "6502" (NMOS) and "65c02"
 * (WDC W65C02S) in cpu6502.c, "z80" (Zilog Z84C00) in cpuz80.c. Each adapter wraps one of the vendored
 * floooh/chips cores (third_party/chips) and drives the pins at the
 * datasheet's times; this file holds the loader, the one SimComponentType
 * that forwards to the adapter, and what the adapters share: input sampling
 * and the instruction tracker.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

/* ---- Cores ------------------------------------------------------------- */

static const char *const CONTROLS_6502[] = {
    "clock", "rw", "sync", "irq", "nmi", "reset", "so", "rdy", "phi1Out", "phi2Out", NULL
};
static const char *const CONTROLS_65C02[] = {
    "clock", "rw", "sync", "irq", "nmi", "reset", "so", "rdy", "phi1Out", "phi2Out",
    "vectorPull", "memoryLock", "busEnable", NULL
};
static const char *const TIMING_6502[] = {
    "tcyc", "tpwh", "tpwl", "tads", "tah", "tsync", "tmds", "tdhw", "tdsr", "tpcs", "tbvd", NULL
};
static const char *const CONTROLS_Z80[] = {
    "clock", "m1", "mreq", "iorq", "rd", "wr", "rfsh", "halt", "busack",
    "wait", "int", "nmi", "reset", "busreq", NULL
};
/* PS0178 rows (plan-09-C section 4.2), in cpuz80.c's T_ order. */
static const char *const TIMING_Z80[] = {
    "tcc", "twch", "twcl", "tdcra", "tdcfmreqf", "tdcrmreqr", "tdcfmreqr", "tdcfrdf", "tdcrrdr",
    "tsdcr", "tswaitcf", "tdcrm1f", "tdcrm1r", "tdcrrfshf", "tdcrrfshr", "tdcfrdr", "tdcrrdf",
    "tsdcf", "tdcriorqf", "tdcfiorqr", "tdcfwrf", "tdcfwrr", "tdcrwrf", "tdcfhalt", "twnmi",
    "tsbusreq", "tdcrbusackf", "tdcfbusackr", "tdcrdz", "tdcrctz", "tdcraz", "tsreset", "tsint",
    "tdcfiorqf", "tdcfd", NULL
};

static const CpuCoreDesc CORES[] = {
    {"6502", CONTROLS_6502, TIMING_6502, 3, &CPU6502_OPS},
    {"65c02", CONTROLS_65C02, TIMING_6502, 3, &CPU6502_OPS},
    {"z80", CONTROLS_Z80, TIMING_Z80, 3, &CPUZ80_OPS},
};
#define CORE_COUNT (sizeof CORES / sizeof CORES[0])

/* Every key a device may carry: the shared ones, then every core's controls. */
static const char *const DEVICE_KEYS[] = {
    "core", "address", "data", "grades",
    "clock", "rw", "sync", "irq", "nmi", "reset", "so", "rdy", "phi1Out", "phi2Out",
    "vectorPull", "memoryLock", "busEnable",
    "m1", "mreq", "iorq", "rd", "wr", "rfsh", "halt", "busack", "wait", "int", "busreq",
    NULL
};
static const char *const PART_KEYS[] = {"grade", "vcc", "inputMax", NULL};
enum { FIRST_CONTROL_KEY = 4 };

static int in_list(const char *const *list, const char *name) {
    for (; *list != NULL; list++) {
        if (strcmp(*list, name) == 0) {
            return 1;
        }
    }
    return 0;
}

/* ---- Kind: devices, pinouts, parts --------------------------------------- */

static int load_grade(KindContext *ctx, const CpuCoreDesc *core, const cJSON *json,
                      CpuGrade *grade) {
    static const char *const EXTRA[] = {"note", "vcc", "inputMax", NULL};
    const cJSON *note = cJSON_GetObjectItemCaseSensitive(json, "note");
    size_t i;

    if (!cJSON_IsObject(json)) {
        return kind_error(ctx, "a grade is an object of timing values");
    }
    if (kind_check_keys(ctx, json, core->timing, EXTRA) != 0) {
        return -1;
    }
    for (i = 0; core->timing[i] != NULL; i++) {
        const cJSON *value = cJSON_GetObjectItemCaseSensitive(json, core->timing[i]);

        grade->t[i] = 0;
        if (value != NULL && kind_ns(ctx, value, core->timing[i], &grade->t[i]) != 0) {
            return -1;
        }
        if (i < core->positive && grade->t[i] == 0) {
            return kind_error(ctx, "\"%s\" must be more than 0", core->timing[i]);
        }
    }
    if (note != NULL && !cJSON_IsString(note)) {
        return kind_error(ctx, "\"note\" must be a string");
    }
    return kind_voltage(ctx, json, NULL, &grade->vcc);
}

static void *cpu_load_device(KindContext *ctx, const cJSON *json) {
    CpuDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *grades;
    const cJSON *item;
    const char *core;
    size_t i;
    size_t n = 0;

    if (device == NULL || kind_string(ctx, json, "core", &core) != 0) {
        return NULL;
    }
    for (i = 0; i < CORE_COUNT; i++) {
        if (strcmp(CORES[i].name, core) == 0) {
            device->core = &CORES[i];
        }
    }
    if (device->core == NULL) {
        kind_error(ctx, "unknown core \"%s\"", core);
        return NULL;
    }
    /* A control key of another core. */
    cJSON_ArrayForEach(item, json) {
        if (in_list(DEVICE_KEYS + FIRST_CONTROL_KEY, item->string) &&
            !in_list(device->core->controls, item->string)) {
            kind_error(ctx, "\"%s\" is not a key of core \"%s\"", item->string, core);
            return NULL;
        }
    }
    if (kind_names(ctx, json, "address", &device->address, &device->address_count) != 0) {
        return NULL;
    }
    if (device->address_count != CPU_ADDRESS_BITS) {
        kind_error(ctx, "\"address\" needs exactly %d pin names", CPU_ADDRESS_BITS);
        return NULL;
    }
    if (kind_names(ctx, json, "data", &device->data, &device->data_count) != 0) {
        return NULL;
    }
    if (device->data_count != CPU_DATA_BITS) {
        kind_error(ctx, "\"data\" needs exactly %d pin names", CPU_DATA_BITS);
        return NULL;
    }
    for (i = 0; device->core->controls[i] != NULL; i++) {
        if (kind_string(ctx, json, device->core->controls[i], &device->controls[i]) != 0) {
            return NULL;
        }
    }
    device->control_count = i;

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
        CpuGrade *grade = &device->grades[n];
        const cJSON *other;

        for (other = grades->child; other != item; other = other->next) {
            if (strcmp(other->string, item->string) == 0) {
                kind_error(ctx, "duplicate key \"%s\"", item->string);
                return NULL;
            }
        }
        kind_push(ctx, "grades.%s", item->string);
        device->grade_names[n] = kind_strdup(ctx, item->string);
        if (device->grade_names[n] == NULL || load_grade(ctx, device->core, item, grade) != 0) {
            return NULL;
        }
        kind_pop(ctx);
        grade->info.core = device->core->name;
        grade->info.grade = device->grade_names[n];
        grade->info.max_hz = 1000000000000ull / grade->t[0];
        n++;
    }
    return device;
}

static int bind_one(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *key,
                    const char *name, size_t *out) {
    kind_push(ctx, "%s", key);
    if (kind_pin(ctx, pins, pin_count, name, out) != 0) {
        return -1;
    }
    kind_pop(ctx);
    return 0;
}

static void *cpu_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                             size_t pin_count) {
    const CpuDevice *device = raw;
    CpuPins *bound = kind_alloc(ctx, sizeof(*bound));
    char key[32];
    size_t i;

    if (bound == NULL) {
        return NULL;
    }
    if (pin_count > CPU_MAX_PINS) {
        kind_error(ctx, "a processor has at most %d pins", CPU_MAX_PINS);
        return NULL;
    }
    for (i = 0; i < CPU_ADDRESS_BITS; i++) {
        snprintf(key, sizeof key, "address[%zu]", i);
        if (bind_one(ctx, pins, pin_count, key, device->address[i], &bound->address[i]) != 0) {
            return NULL;
        }
    }
    for (i = 0; i < CPU_DATA_BITS; i++) {
        snprintf(key, sizeof key, "data[%zu]", i);
        if (bind_one(ctx, pins, pin_count, key, device->data[i], &bound->data[i]) != 0) {
            return NULL;
        }
    }
    for (i = 0; i < device->control_count; i++) {
        if (bind_one(ctx, pins, pin_count, device->core->controls[i], device->controls[i],
                     &bound->control[i]) != 0) {
            return NULL;
        }
    }
    bound->device = device;
    return bound;
}

static int cpu_load_part(KindContext *ctx, const void *raw, const void *bound, const cJSON *json,
                         Part *out) {
    const CpuDevice *device = raw;
    const char *grade;
    size_t i;

    (void)bound;
    if (kind_string(ctx, json, "grade", &grade) != 0) {
        return -1;
    }
    for (i = 0; i < device->grade_count; i++) {
        if (strcmp(device->grade_names[i], grade) == 0) {
            out->cpu = &device->grades[i].info;
            kind_set_part_data(out, &device->grades[i]);
            return kind_part_voltage(ctx, json, device->grades[i].vcc, &out->vcc);
        }
    }
    return kind_error(ctx, "grade \"%s\" is not in \"grades\"", grade);
}

/* ---- The model: one type, forwarding to the part's core ------------------ */

static size_t cpu_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *cpu_create(const void *config) {
    const Part *part = config;
    const PartInternal *internal = part_internal(part);
    const CpuPins *pins = internal->bound;

    return pins->device->core->ops->create(part, pins, internal->data);
}

static const CpuCoreOps *ops_of(const void *state) {
    return ((const CpuBase *)state)->ops;
}

static void cpu_destroy(void *state) {
    if (state != NULL) {
        ops_of(state)->destroy(state);
    }
}

static void cpu_start(Sim *sim, SimInstance *instance, void *state) {
    ops_of(state)->start(sim, instance, state);
}

static void cpu_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                            LogicValue old_value, LogicValue new_value) {
    ops_of(state)->pin_changed(sim, instance, state, pin, old_value, new_value);
}

static void cpu_timer(Sim *sim, SimInstance *instance, void *state, uint32_t timer_id) {
    if (ops_of(state)->timer != NULL) {
        ops_of(state)->timer(sim, instance, state, timer_id);
    }
}

static int cpu_state(const void *state, CpuState *out) {
    memset(out, 0, sizeof(*out));
    return ops_of(state)->state(state, out);
}

static const SimComponentType CPU_MODEL = {
    "cpu", cpu_pin_count, cpu_create, cpu_destroy, cpu_start, cpu_pin_changed, cpu_timer,
    NULL, NULL, NULL, NULL, NULL, NULL, cpu_state
};

const PartKindOps CPU_KIND = {
    PART_CPU, "cpu", &CPU_MODEL, DEVICE_KEYS, PART_KEYS,
    cpu_load_device, cpu_bind_pinout, cpu_load_part
};

/* ---- Inputs ------------------------------------------------------------ */

LogicValue cpu_level(LogicValue value, LogicValue inactive) {
    switch (value) {
        case LOGIC_0:
        case LOGIC_1:
            return value;
        case LOGIC_Z:
            return inactive;
        case LOGIC_X:
            break;
    }
    return LOGIC_X;
}

LogicValue cpu_sample(const Sim *sim, const SimInstance *instance, size_t pin, SimTime changed,
                      LogicValue seen, SimTime setup, LogicValue inactive) {
    SimTime now = sim_now(sim);
    LogicValue value = sim_read_pin(sim, instance, pin);

    if (value != seen || (changed != CPU_NEVER && changed <= now && now - changed < setup)) {
        return LOGIC_X;
    }
    return cpu_level(value, inactive);
}

LogicValue cpu_sample_reset(const Sim *sim, const SimInstance *instance, size_t pin,
                            SimTime changed, LogicValue seen, SimTime setup, int in_reset) {
    LogicValue sampled = cpu_sample(sim, instance, pin, changed, seen, setup, LOGIC_1);

    if (sampled == LOGIC_X && cpu_level(sim_read_pin(sim, instance, pin), LOGIC_1) != LOGIC_X) {
        return in_reset ? LOGIC_0 : LOGIC_1;
    }
    return sampled;
}

/* ---- The instruction tracker -------------------------------------------- */

typedef struct {
    const uint8_t *bytes;
    size_t length;
    size_t used;
    char *text;
    size_t text_length;
    size_t text_cap;
} DasmIo;

static uint8_t dasm_in(void *user) {
    DasmIo *io = user;
    uint8_t value = io->used < io->length ? io->bytes[io->used] : 0;

    io->used++;
    return value;
}

static void dasm_out(char c, void *user) {
    DasmIo *io = user;

    if (io->text_length + 1 < io->text_cap) {
        io->text[io->text_length++] = c;
    }
}

void cpu_tracker_reset(CpuTracker *tracker, CpuDasm dasm) {
    memset(tracker, 0, sizeof(*tracker));
    tracker->dasm = dasm;
}

void cpu_tracker_finish(CpuTracker *tracker) {
    DasmIo io;

    if (tracker->recording && tracker->length > 0) {
        memset(&io, 0, sizeof io);
        io.bytes = tracker->bytes;
        io.length = tracker->length;
        io.text = tracker->text;
        io.text_cap = sizeof tracker->text;
        tracker->dasm(tracker->start, dasm_in, dasm_out, &io);
        tracker->text[io.text_length] = '\0';
        tracker->has = 1;
        tracker->address = tracker->start;
        tracker->last_length = io.used < sizeof tracker->last ? io.used : sizeof tracker->last;
        memcpy(tracker->last, tracker->bytes, sizeof tracker->last);
        if (tracker->last_length > tracker->length) {
            memset(tracker->last + tracker->length, 0, tracker->last_length - tracker->length);
        }
    }
    tracker->recording = 0;
}

void cpu_tracker_begin(CpuTracker *tracker, uint16_t address) {
    cpu_tracker_finish(tracker);
    tracker->recording = 1;
    tracker->start = address;
    tracker->length = 0;
    memset(tracker->bytes, 0, sizeof tracker->bytes);
}

void cpu_tracker_read(CpuTracker *tracker, uint16_t address, uint8_t data) {
    if (tracker->recording && tracker->length < sizeof tracker->bytes &&
        address == (uint16_t)(tracker->start + tracker->length)) {
        tracker->bytes[tracker->length++] = data;
    }
}

void cpu_tracker_fill(const CpuTracker *tracker, CpuState *out) {
    out->has_instruction = tracker->has;
    if (tracker->has) {
        out->instruction_address = tracker->address;
        out->instruction_length = tracker->last_length;
        memcpy(out->instruction_bytes, tracker->last, sizeof out->instruction_bytes);
        snprintf(out->instruction_text, sizeof out->instruction_text, "%s", tracker->text);
    }
}
