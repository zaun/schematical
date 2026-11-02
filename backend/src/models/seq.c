/*
 * seq: helpers shared by the sequential kinds (seq.h; plan-10-B section 4.2).
 */

#include <string.h>

#include "seq.h"

static const char *const LIBRARY_DEVICE_KEYS[] = {
    "id", "function", "description", "drawer", "note", "pinouts", "parts", "model", "timing", NULL
};

static int in_list(const char *const *list, const char *key) {
    size_t i;

    for (i = 0; list != NULL && list[i] != NULL; i++) {
        if (strcmp(list[i], key) == 0) {
            return 1;
        }
    }
    return 0;
}

int seq_check_model_keys(KindContext *ctx, const cJSON *device_json, const char *model_name,
                         const char *const *model_keys) {
    const cJSON *item;

    for (item = device_json->child; item != NULL; item = item->next) {
        if (!in_list(LIBRARY_DEVICE_KEYS, item->string) && !in_list(model_keys, item->string)) {
            return kind_error(ctx, "\"%s\" is not a key of model \"%s\"", item->string, model_name);
        }
    }
    return 0;
}

int seq_timing(KindContext *ctx, const cJSON *device_json, const char *const *extra_keys,
               unsigned units, SeqTimingTable *table) {
    static const char *const BASE_KEYS[] = {"rise", "fall", "note", "vcc", "inputMax", NULL};
    const cJSON *timing = kind_member(ctx, device_json, "timing", 1);
    const cJSON *item;
    size_t n = 0;

    if (timing == NULL) {
        return -1;
    }
    if (!cJSON_IsObject(timing) || timing->child == NULL) {
        return kind_error(ctx, "\"timing\" must be a non-empty object");
    }
    table->count = (size_t)cJSON_GetArraySize(timing);
    table->entries = kind_alloc(ctx, table->count * sizeof(*table->entries));
    if (table->entries == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(item, timing) {
        SeqTiming *entry = &table->entries[n];
        const cJSON *note = cJSON_GetObjectItemCaseSensitive(item, "note");
        const cJSON *other;

        for (other = timing->child; other != item; other = other->next) {
            if (strcmp(other->string, item->string) == 0) {
                return kind_error(ctx, "duplicate key \"%s\"", item->string);
            }
        }
        kind_push(ctx, "timing.%s", item->string);
        entry->family = kind_logic_family(ctx, item->string);
        if (entry->family == NULL || kind_check_keys(ctx, item, BASE_KEYS, extra_keys) != 0 ||
            kind_ns(ctx, cJSON_GetObjectItemCaseSensitive(item, "rise"), "rise", &entry->rise) != 0 ||
            kind_ns(ctx, cJSON_GetObjectItemCaseSensitive(item, "fall"), "fall", &entry->fall) != 0) {
            return -1;
        }
        if (note != NULL && !cJSON_IsString(note)) {
            return kind_error(ctx, "\"note\" must be a string");
        }
        if (kind_voltage(ctx, item, NULL, &entry->vcc) != 0) {
            return -1;
        }
        entry->json = item;
        entry->logic.family = entry->family;
        entry->logic.units = units;
        kind_pop(ctx);
        n++;
    }
    return 0;
}

int seq_timing_extra(KindContext *ctx, const SeqTiming *entry, const char *key, int required,
                     SimTime *ps) {
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(entry->json, key);
    int rc;

    if (item == NULL) {
        return required ? kind_error(ctx, "timing.%s: missing \"%s\"", entry->family, key) : 0;
    }
    kind_push(ctx, "timing.%s", entry->family);
    rc = kind_ns(ctx, item, key, ps);
    if (rc == 0) {
        kind_pop(ctx);
    }
    return rc;
}

int seq_part(KindContext *ctx, const SeqTimingTable *table, const cJSON *part_json, Part *out,
             const SeqTiming **entry) {
    const char *family;
    size_t i;

    if (kind_string(ctx, part_json, "family", &family) != 0 ||
        kind_logic_family(ctx, family) == NULL) {
        return -1;
    }
    for (i = 0; i < table->count; i++) {
        if (strcmp(table->entries[i].family, family) == 0) {
            *entry = &table->entries[i];
            out->logic = &table->entries[i].logic;
            return kind_part_voltage(ctx, part_json, table->entries[i].vcc, &out->vcc);
        }
    }
    return kind_error(ctx, "family \"%s\" has no timing entry", family);
}

int seq_active(KindContext *ctx, const cJSON *json, const char *key, int *active_high) {
    const char *text;

    if (kind_string(ctx, json, key, &text) != 0) {
        return -1;
    }
    if (strcmp(text, "high") == 0) {
        *active_high = 1;
    } else if (strcmp(text, "low") == 0) {
        *active_high = 0;
    } else {
        return kind_error(ctx, "\"%s\" must be \"high\" or \"low\", not \"%s\"", key, text);
    }
    return 0;
}

int seq_control_object(KindContext *ctx, const cJSON *item, int allow_when,
                       SeqControlName *name) {
    static const char *const KEYS[] = {"pin", "active", NULL};
    static const char *const WHEN_KEYS[] = {"pin", "active", "when", NULL};

    name->pin = NULL;
    name->active_high = 0;
    name->clocked = 0;
    if (kind_check_keys(ctx, item, allow_when ? WHEN_KEYS : KEYS, NULL) != 0 ||
        kind_string(ctx, item, "pin", &name->pin) != 0 ||
        seq_active(ctx, item, "active", &name->active_high) != 0) {
        return -1;
    }
    if (allow_when) {
        const char *when;

        if (kind_string(ctx, item, "when", &when) != 0) {
            return -1;
        }
        if (strcmp(when, "clock") == 0) {
            name->clocked = 1;
        } else if (strcmp(when, "async") != 0) {
            return kind_error(ctx, "\"when\" must be \"async\" or \"clock\", not \"%s\"", when);
        }
    }
    return 0;
}

int seq_control_name(KindContext *ctx, const cJSON *json, const char *key, int required,
                     int allow_when, SeqControlName *name) {
    const cJSON *item = kind_member(ctx, json, key, required);

    name->pin = NULL;
    name->active_high = 0;
    name->clocked = 0;
    if (item == NULL) {
        return required ? -1 : 0;
    }
    kind_push(ctx, "%s", key);
    if (seq_control_object(ctx, item, allow_when, name) != 0) {
        return -1;
    }
    kind_pop(ctx);
    return 0;
}

int seq_optional_name(KindContext *ctx, const cJSON *json, const char *key, const char **out) {
    *out = NULL;
    if (cJSON_GetObjectItemCaseSensitive(json, key) == NULL) {
        return 0;
    }
    return kind_string(ctx, json, key, out);
}

int seq_names(KindContext *ctx, const cJSON *json, const char *key, size_t min, size_t max,
              const char *const **names, size_t *count) {
    if (kind_names(ctx, json, key, names, count) != 0) {
        return -1;
    }
    if (*count < min || *count > max) {
        if (min == max) {
            return kind_error(ctx, "\"%s\" must have %zu names", key, min);
        }
        return kind_error(ctx, "\"%s\" must have %zu to %zu names", key, min, max);
    }
    return 0;
}

int seq_output_style(KindContext *ctx, const cJSON *json, SeqOutputStyle *style) {
    const cJSON *output = cJSON_GetObjectItemCaseSensitive(json, "output");

    *style = SEQ_PUSH_PULL;
    if (output == NULL) {
        return 0;
    }
    if (!cJSON_IsString(output) || (strcmp(output->valuestring, "push-pull") != 0 &&
                                    strcmp(output->valuestring, "open-drain") != 0)) {
        return kind_error(ctx, "\"output\" must be \"push-pull\" or \"open-drain\"");
    }
    if (strcmp(output->valuestring, "open-drain") == 0) {
        *style = SEQ_OPEN_DRAIN;
    }
    return 0;
}

int seq_optional_bool(KindContext *ctx, const cJSON *json, const char *key, int fallback,
                      int *out) {
    if (cJSON_GetObjectItemCaseSensitive(json, key) == NULL) {
        *out = fallback;
        return 0;
    }
    return kind_bool(ctx, json, key, out);
}

int seq_bind(KindContext *ctx, const PartPin *pins, size_t count, const char *name,
             size_t *index) {
    *index = SEQ_NO_PIN;
    return name == NULL ? 0 : kind_pin(ctx, pins, count, name, index);
}

int seq_bind_control(KindContext *ctx, const PartPin *pins, size_t count,
                     const SeqControlName *name, SeqControl *control) {
    control->active_high = name->active_high;
    control->clocked = name->clocked;
    return seq_bind(ctx, pins, count, name->pin, &control->pin);
}

int seq_clock_name(KindContext *ctx, const cJSON *json, const char *key, int required,
                   SeqClockName *name) {
    static const char *const KEYS[] = {"pin", "edge", NULL};
    const cJSON *item = kind_member(ctx, json, key, required);
    const char *edge;

    name->pin = NULL;
    name->falling = 0;
    if (item == NULL) {
        return required ? -1 : 0;
    }
    kind_push(ctx, "%s", key);
    if (kind_check_keys(ctx, item, KEYS, NULL) != 0 ||
        kind_string(ctx, item, "pin", &name->pin) != 0 ||
        kind_string(ctx, item, "edge", &edge) != 0) {
        return -1;
    }
    if (strcmp(edge, "falling") == 0) {
        name->falling = 1;
    } else if (strcmp(edge, "rising") != 0) {
        return kind_error(ctx, "\"edge\" must be \"rising\" or \"falling\", not \"%s\"", edge);
    }
    kind_pop(ctx);
    return 0;
}

int seq_bind_clock(KindContext *ctx, const PartPin *pins, size_t count, const SeqClockName *name,
                   SeqClock *clock) {
    clock->falling = name->falling;
    return seq_bind(ctx, pins, count, name->pin, &clock->pin);
}

int seq_gate_name(KindContext *ctx, const cJSON *json, const char *key, SeqGateName *name) {
    static const char *const KEYS[] = {"pin", "op", NULL};
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);
    const char *op;

    name->pin = NULL;
    name->op = SEQ_GATE_AND_NOT;
    if (item == NULL) {
        return 0;
    }
    kind_push(ctx, "%s", key);
    if (kind_check_keys(ctx, item, KEYS, NULL) != 0 ||
        kind_string(ctx, item, "pin", &name->pin) != 0 ||
        kind_string(ctx, item, "op", &op) != 0) {
        return -1;
    }
    if (strcmp(op, "and-not") == 0) {
        name->op = SEQ_GATE_AND_NOT;
    } else if (strcmp(op, "or") == 0) {
        name->op = SEQ_GATE_OR;
    } else if (strcmp(op, "or-not") == 0) {
        name->op = SEQ_GATE_OR_NOT;
    } else {
        return kind_error(ctx, "\"op\" must be \"and-not\", \"or\" or \"or-not\", not \"%s\"", op);
    }
    kind_pop(ctx);
    return 0;
}

int seq_bind_gate(KindContext *ctx, const PartPin *pins, size_t count, const SeqGateName *name,
                  SeqGate *gate) {
    gate->op = name->op;
    return seq_bind(ctx, pins, count, name->pin, &gate->pin);
}

/* ---- Running ------------------------------------------------------------- */

LogicValue seq_gated_clock(LogicValue clock, LogicValue gate, SeqGateOp op) {
    int c = seq_level(clock);
    int g = seq_level(gate);

    if (op == SEQ_GATE_AND_NOT) {
        if (c == 0 || g == 1) {
            return LOGIC_0;
        }
        return c == 1 && g == 0 ? LOGIC_1 : LOGIC_X;
    }
    if (op == SEQ_GATE_OR_NOT) {
        g = g < 0 ? -1 : !g;
    }
    if (c == 1 || g == 1) {
        return LOGIC_1;
    }
    return c == 0 && g == 0 ? LOGIC_0 : LOGIC_X;
}

int seq_clock_edge(const SeqClock *clock, LogicValue old_value, LogicValue new_value) {
    return clock->falling ? seq_falling(old_value, new_value) : seq_rising(old_value, new_value);
}

int seq_level(LogicValue v) {
    return v == LOGIC_1 ? 1 : v == LOGIC_0 ? 0 : -1;
}

int seq_read(const Sim *sim, const SimInstance *inst, size_t pin) {
    return pin == SEQ_NO_PIN ? -1 : seq_level(sim_read_pin(sim, inst, pin));
}

int seq_async(const Sim *sim, const SimInstance *inst, const SeqControl *c) {
    LogicValue v;

    if (c->pin == SEQ_NO_PIN) {
        return 0;
    }
    v = sim_read_pin(sim, inst, c->pin);
    if (v == LOGIC_Z) {
        return 0;
    }
    if (v == LOGIC_X) {
        return -1;
    }
    return (v == LOGIC_1) == (c->active_high != 0);
}

int seq_sync(const Sim *sim, const SimInstance *inst, const SeqControl *c) {
    int level;

    if (c->pin == SEQ_NO_PIN) {
        return 0;
    }
    level = seq_level(sim_read_pin(sim, inst, c->pin));
    if (level < 0) {
        return -1;
    }
    return level == (c->active_high != 0);
}

int seq_enable(const Sim *sim, const SimInstance *inst, const SeqControl *c) {
    if (c->pin == SEQ_NO_PIN) {
        return 1;
    }
    return seq_sync(sim, inst, c);
}

int seq_rising(LogicValue old_value, LogicValue new_value) {
    return old_value != LOGIC_1 && new_value == LOGIC_1;
}

int seq_falling(LogicValue old_value, LogicValue new_value) {
    return old_value != LOGIC_0 && new_value == LOGIC_0;
}

LogicValue seq_bit(SeqBits b, unsigned i) {
    if (!((b.known >> i) & 1u)) {
        return LOGIC_X;
    }
    return (b.value >> i) & 1u ? LOGIC_1 : LOGIC_0;
}

void seq_set_bit(SeqBits *b, unsigned i, LogicValue v) {
    uint64_t mask = (uint64_t)1 << i;

    if (v == LOGIC_0 || v == LOGIC_1) {
        b->known |= mask;
        if (v == LOGIC_1) {
            b->value |= mask;
        } else {
            b->value &= ~mask;
        }
    } else {
        b->known &= ~mask;
        b->value &= ~mask;
    }
}

LogicValue seq_not(LogicValue v) {
    return v == LOGIC_0 ? LOGIC_1 : v == LOGIC_1 ? LOGIC_0 : LOGIC_X;
}

LogicValue seq_value(int level) {
    return level == 1 ? LOGIC_1 : level == 0 ? LOGIC_0 : LOGIC_X;
}

LogicValue seq_gate(LogicValue v, int enable) {
    if (enable == 1) {
        return v;
    }
    return enable == 0 ? LOGIC_Z : LOGIC_X;
}

void seq_drive(Sim *sim, SimInstance *inst, SeqOutput *o, LogicValue v, SeqOutputStyle style,
               const SeqTiming *t) {
    SimTime delay;

    if (o->pin == SEQ_NO_PIN) {
        return;
    }
    if (style == SEQ_OPEN_DRAIN && v == LOGIC_1) {
        v = LOGIC_Z;
    }
    if (o->scheduled && o->driven == v) {
        return;
    }
    o->scheduled = 1;
    o->driven = v;
    if (v == LOGIC_0) {
        delay = t->fall;
    } else if (v == LOGIC_1 || v == LOGIC_Z) {
        delay = t->rise;
    } else {
        delay = t->rise > t->fall ? t->rise : t->fall;
    }
    sim_schedule_inertial_drive(sim, inst, o->pin, v, DRIVE_STRONG, delay);
}
