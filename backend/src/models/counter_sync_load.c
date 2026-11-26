/* models/counter_sync_load.c: part of the model in models/counter.c (split by plan-68-I/J). */
#include "counter_int.h"
/* ---- sync: loading -------------------------------------------------------- */

static int sync_names_n(KindContext *ctx, const cJSON *json, const char *key, size_t n,
                        const char *const **out, size_t *count, int required) {
    if (cJSON_GetObjectItemCaseSensitive(json, key) == NULL) {
        *out = NULL;
        *count = 0;
        return required ? kind_error(ctx, "missing \"%s\"", key) : 0;
    }
    return seq_names(ctx, json, key, n, n, out, count);
}

static int load_when_controls(KindContext *ctx, const cJSON *json, const char *key,
                              SeqControlName *names, size_t *count, const char *extra_key,
                              const char *a, const char *b, int *flags) {
    const cJSON *list = cJSON_GetObjectItemCaseSensitive(json, key);
    const cJSON *item;
    size_t n = 0;

    *count = 0;
    if (list == NULL) {
        return 0;
    }
    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) < 1 ||
        cJSON_GetArraySize(list) > SYNC_MAX_LIST) {
        return kind_error(ctx, "\"%s\" must be an array of 1 to %d objects", key, SYNC_MAX_LIST);
    }
    cJSON_ArrayForEach(item, list) {
        static const char *const KEYS_CLEAR[] = {"pin", "active", "when", "to", NULL};
        static const char *const KEYS_LOAD[] = {"pin", "active", "when", "from", NULL};
        const char *when;
        const char *value;

        kind_push(ctx, "%s[%zu]", key, n);
        if (kind_check_keys(ctx, item, strcmp(extra_key, "to") == 0 ? KEYS_CLEAR : KEYS_LOAD,
                            NULL) != 0 ||
            kind_string(ctx, item, "pin", &names[n].pin) != 0 ||
            seq_active(ctx, item, "active", &names[n].active_high) != 0 ||
            kind_string(ctx, item, "when", &when) != 0 ||
            kind_string(ctx, item, extra_key, &value) != 0) {
            return -1;
        }
        if (strcmp(when, "clock") == 0) {
            names[n].clocked = 1;
        } else if (strcmp(when, "async") == 0) {
            names[n].clocked = 0;
        } else {
            return kind_error(ctx, "\"when\" must be \"async\" or \"clock\", not \"%s\"", when);
        }
        if (strcmp(value, a) == 0) {
            flags[n] = 0;
        } else if (strcmp(value, b) == 0) {
            flags[n] = 1;
        } else {
            return kind_error(ctx, "\"%s\" must be \"%s\" or \"%s\", not \"%s\"", extra_key, a, b,
                              value);
        }
        kind_pop(ctx);
        n++;
    }
    *count = n;
    return 0;
}

static int load_carries(KindContext *ctx, const cJSON *json, SyncUnitDef *u) {
    static const char *const KEYS[] = {"pin", "active", "at", "gates", "clockLow", NULL};
    const cJSON *list = cJSON_GetObjectItemCaseSensitive(json, "carries");
    const cJSON *item;
    size_t n = 0;

    if (list == NULL) {
        return 0;
    }
    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) < 1 ||
        cJSON_GetArraySize(list) > SYNC_MAX_LIST) {
        return kind_error(ctx, "\"carries\" must be an array of 1 to %d objects", SYNC_MAX_LIST);
    }
    cJSON_ArrayForEach(item, list) {
        SyncCarryName *c = &u->carries[n];
        const char *at;

        kind_push(ctx, "carries[%zu]", n);
        if (kind_check_keys(ctx, item, KEYS, NULL) != 0 ||
            kind_string(ctx, item, "pin", &c->c.pin) != 0 ||
            seq_active(ctx, item, "active", &c->c.active_high) != 0 ||
            kind_string(ctx, item, "at", &at) != 0 ||
            cnt_control_list(ctx, item, "gates", 0, c->gates, SYNC_MAX_LIST, &c->gate_count) != 0 ||
            seq_optional_name(ctx, item, "clockLow", &c->clock_low) != 0) {
            return -1;
        }
        if (strcmp(at, "max") == 0) {
            c->at = AT_MAX;
        } else if (strcmp(at, "min") == 0) {
            c->at = AT_MIN;
        } else if (strcmp(at, "terminal") == 0) {
            c->at = AT_TERMINAL;
        } else {
            return kind_error(ctx, "\"at\" must be \"max\", \"min\" or \"terminal\", not \"%s\"", at);
        }
        kind_pop(ctx);
        n++;
    }
    u->carry_count = n;
    return 0;
}

static int load_modes(KindContext *ctx, const cJSON *json, SyncUnitDef *u) {
    static const char *const KEYS[] = {"pins", "table", "clear", NULL};
    static const char *const ROWS[] = {"clear", "down", "load", "up", "hold"};
    const cJSON *modes = cJSON_GetObjectItemCaseSensitive(json, "modes");
    const char *const *pins;
    const char *const *table;
    const char *clear;
    size_t n, i, j;

    if (modes == NULL) {
        return 0;
    }
    kind_push(ctx, "modes");
    if (kind_check_keys(ctx, modes, KEYS, NULL) != 0 ||
        seq_names(ctx, modes, "pins", 2, 2, &pins, &n) != 0 ||
        seq_names(ctx, modes, "table", 4, 4, &table, &n) != 0 ||
        kind_string(ctx, modes, "clear", &clear) != 0) {
        return -1;
    }
    u->mode_pins[0] = pins[0];
    u->mode_pins[1] = pins[1];
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 5 && strcmp(ROWS[j], table[i]) != 0; j++) {
        }
        if (j == 5) {
            return kind_error(ctx, "table[%zu]: unknown mode \"%s\"", i, table[i]);
        }
        u->mode_table[i] = (int)j;
    }
    if (strcmp(clear, "async") == 0) {
        u->mode_clear_async = 1;
    } else if (strcmp(clear, "clock") != 0) {
        return kind_error(ctx, "\"clear\" must be \"async\" or \"clock\", not \"%s\"", clear);
    }
    kind_pop(ctx);
    u->has_modes = 1;
    return 0;
}

static int load_illegal(KindContext *ctx, const cJSON *json, SyncUnitDef *u) {
    static const char *const KEYS[] = {"up", "down", NULL};
    const cJSON *ill = cJSON_GetObjectItemCaseSensitive(json, "illegal");
    int dir;

    if (ill == NULL) {
        return 0;
    }
    if (!u->bcd) {
        return kind_error(ctx, "\"illegal\" is only for a bcd unit");
    }
    kind_push(ctx, "illegal");
    if (kind_check_keys(ctx, ill, KEYS, NULL) != 0) {
        return -1;
    }
    for (dir = 0; dir < 2; dir++) {
        const char *key = dir == 0 ? "up" : "down";
        const cJSON *arr = kind_member(ctx, ill, key, 1);
        const cJSON *item;
        unsigned char *out = dir == 0 ? u->illegal_up : u->illegal_down;
        int i = 0;

        if (arr == NULL) {
            return -1;
        }
        if (!cJSON_IsArray(arr) || cJSON_GetArraySize(arr) != 16) {
            return kind_error(ctx, "\"%s\" must be 16 numbers 0 to 15", key);
        }
        cJSON_ArrayForEach(item, arr) {
            int normal = dir == 0 ? (i + 1) % 10 : (i + 9) % 10;

            if (!cJSON_IsNumber(item) || item->valuedouble < 0 || item->valuedouble > 15 ||
                item->valuedouble != (int)item->valuedouble) {
                return kind_error(ctx, "\"%s\" must be 16 numbers 0 to 15", key);
            }
            out[i] = (unsigned char)item->valueint;
            if (i < 10 && out[i] != normal) {
                return kind_error(ctx, "%s[%d] must be %d (the normal sequence)", key, i, normal);
            }
            i++;
        }
    }
    kind_pop(ctx);
    u->has_illegal = 1;
    return 0;
}

static int load_register(KindContext *ctx, const cJSON *json, SyncUnitDef *u) {
    static const char *const KEYS[] = {"clock", "outputs", "outputEnable", "select", NULL};
    static const char *const SELECT_KEYS[] = {"pin", "register", NULL};
    const cJSON *reg = cJSON_GetObjectItemCaseSensitive(json, "register");
    const cJSON *select;

    if (reg == NULL) {
        return 0;
    }
    kind_push(ctx, "register");
    if (kind_check_keys(ctx, reg, KEYS, NULL) != 0 ||
        seq_clock_name(ctx, reg, "clock", 1, &u->reg_clock) != 0 ||
        sync_names_n(ctx, reg, "outputs", u->stages, &u->reg_outputs, &u->reg_output_count, 0) !=
            0 ||
        seq_control_name(ctx, reg, "outputEnable", 0, 0, &u->reg_oe) != 0) {
        return -1;
    }
    select = cJSON_GetObjectItemCaseSensitive(reg, "select");
    if (select != NULL) {
        kind_push(ctx, "select");
        if (kind_check_keys(ctx, select, SELECT_KEYS, NULL) != 0 ||
            kind_string(ctx, select, "pin", &u->reg_select) != 0 ||
            seq_active(ctx, select, "register", &u->reg_select_high) != 0) {
            return -1;
        }
        kind_pop(ctx);
    }
    kind_pop(ctx);
    u->has_register = 1;
    return 0;
}

static int load_input_register(KindContext *ctx, const cJSON *json, SyncUnitDef *u) {
    static const char *const KEYS[] = {"clock", "data", "enable", NULL};
    const cJSON *reg = cJSON_GetObjectItemCaseSensitive(json, "inputRegister");

    if (reg == NULL) {
        return 0;
    }
    kind_push(ctx, "inputRegister");
    if (kind_check_keys(ctx, reg, KEYS, NULL) != 0 ||
        seq_clock_name(ctx, reg, "clock", 1, &u->ireg_clock) != 0 ||
        sync_names_n(ctx, reg, "data", u->stages, &u->ireg_data, &u->ireg_data_count, 0) != 0 ||
        seq_control_name(ctx, reg, "enable", 0, 0, &u->ireg_enable) != 0) {
        return -1;
    }
    kind_pop(ctx);
    u->has_input_register = 1;
    return 0;
}

static int load_bus(KindContext *ctx, const cJSON *json, SyncUnitDef *u) {
    static const char *const KEYS[] = {"pins", "outputEnables", NULL};
    const cJSON *bus = cJSON_GetObjectItemCaseSensitive(json, "bus");

    if (bus == NULL) {
        return 0;
    }
    kind_push(ctx, "bus");
    if (kind_check_keys(ctx, bus, KEYS, NULL) != 0 ||
        sync_names_n(ctx, bus, "pins", u->stages, &u->bus, &u->bus_count, 1) != 0 ||
        cnt_control_list(ctx, bus, "outputEnables", 0, u->bus_oe, SYNC_MAX_LIST, &u->bus_oe_count) !=
            0) {
        return -1;
    }
    kind_pop(ctx);
    u->has_bus = 1;
    return 0;
}

static int sync_load_unit(KindContext *ctx, const cJSON *json, SyncUnitDef *u) {
    static const char *const UNIT_KEYS[] = {
        "stages", "code", "q", "clock", "clockUp", "clockDown", "clockGate", "count", "direction",
        "enables", "enablesAny", "clears", "loads", "data", "modes", "illegal", "carries",
        "outputEnable", "register", "inputRegister", "bus", "zeroLatch", "codeSelect", NULL
    };
    uint64_t stages;
    const cJSON *item;
    int flags[SYNC_MAX_LIST];
    size_t i;

    if (kind_check_keys(ctx, json, UNIT_KEYS, NULL) != 0 ||
        kind_uint(ctx, json, "stages", SYNC_MAX_STAGES, &stages) != 0) {
        return -1;
    }
    if (stages < 1) {
        return kind_error(ctx, "\"stages\" must be 1 to %d", SYNC_MAX_STAGES);
    }
    u->stages = (unsigned)stages;
    item = cJSON_GetObjectItemCaseSensitive(json, "codeSelect");
    if (item != NULL) {
        static const char *const KEYS[] = {"pin", "binary", NULL};

        if (cJSON_GetObjectItemCaseSensitive(json, "code") != NULL) {
            return kind_error(ctx, "\"codeSelect\" and \"code\" cannot both be given");
        }
        if (u->stages != 4) {
            return kind_error(ctx, "a \"codeSelect\" unit has 4 stages");
        }
        kind_push(ctx, "codeSelect");
        if (kind_check_keys(ctx, item, KEYS, NULL) != 0 ||
            kind_string(ctx, item, "pin", &u->code_pin) != 0 ||
            seq_active(ctx, item, "binary", &u->code_binary_high) != 0) {
            return -1;
        }
        kind_pop(ctx);
    } else {
        const char *code;

        if (kind_string(ctx, json, "code", &code) != 0) {
            return -1;
        }
        if (strcmp(code, "bcd") == 0) {
            u->bcd = 1;
            if (u->stages % 4 != 0) {
                return kind_error(ctx, "a bcd unit's \"stages\" must be a multiple of 4");
            }
        } else if (strcmp(code, "binary") != 0) {
            return kind_error(ctx, "\"code\" must be \"binary\" or \"bcd\", not \"%s\"", code);
        }
    }
    if (sync_names_n(ctx, json, "q", u->stages, &u->q, &u->q_count, 0) != 0) {
        return -1;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "clockUp") != NULL) {
        if (cJSON_GetObjectItemCaseSensitive(json, "clock") != NULL) {
            return kind_error(ctx, "\"clock\" and \"clockUp\"/\"clockDown\" cannot both be given");
        }
        if (kind_string(ctx, json, "clockUp", &u->clock_up) != 0 ||
            kind_string(ctx, json, "clockDown", &u->clock_down) != 0) {
            return -1;
        }
    } else if (seq_clock_name(ctx, json, "clock", 1, &u->clock) != 0) {
        return -1;
    }
    if (seq_gate_name(ctx, json, "clockGate", &u->gate) != 0) {
        return -1;
    }
    item = cJSON_GetObjectItemCaseSensitive(json, "count");
    if (item != NULL) {
        if (!cJSON_IsString(item) ||
            (strcmp(item->valuestring, "up") != 0 && strcmp(item->valuestring, "down") != 0)) {
            return kind_error(ctx, "\"count\" must be \"up\" or \"down\"");
        }
        u->count = strcmp(item->valuestring, "up") == 0 ? 1 : -1;
    }
    item = cJSON_GetObjectItemCaseSensitive(json, "direction");
    if (item != NULL) {
        static const char *const KEYS[] = {"pin", "up", NULL};

        kind_push(ctx, "direction");
        if (kind_check_keys(ctx, item, KEYS, NULL) != 0 ||
            kind_string(ctx, item, "pin", &u->direction.pin) != 0 ||
            seq_active(ctx, item, "up", &u->direction.active_high) != 0) {
            return -1;
        }
        kind_pop(ctx);
    }
    if (cnt_control_list(ctx, json, "enables", 0, u->enables, SYNC_MAX_LIST, &u->enable_count) != 0 ||
        cnt_control_list(ctx, json, "enablesAny", 0, u->any, SYNC_MAX_LIST, &u->any_count) != 0) {
        return -1;
    }
    {
        SeqControlName names[SYNC_MAX_LIST];
        size_t n;

        if (load_when_controls(ctx, json, "clears", names, &n, "to", "zero", "max", flags) != 0) {
            return -1;
        }
        for (i = 0; i < n; i++) {
            u->clears[i].c = names[i];
            u->clears[i].to_max = flags[i];
        }
        u->clear_count = n;
        if (load_when_controls(ctx, json, "loads", names, &n, "from", "data", "register",
                               flags) != 0) {
            return -1;
        }
        for (i = 0; i < n; i++) {
            u->loads[i].c = names[i];
            u->loads[i].from_register = flags[i];
        }
        u->load_count = n;
    }
    if (sync_names_n(ctx, json, "data", u->stages, &u->data, &u->data_count, 0) != 0 ||
        load_modes(ctx, json, u) != 0 || load_illegal(ctx, json, u) != 0 ||
        load_carries(ctx, json, u) != 0 ||
        seq_control_name(ctx, json, "outputEnable", 0, 0, &u->oe) != 0 ||
        load_register(ctx, json, u) != 0 || load_input_register(ctx, json, u) != 0 ||
        load_bus(ctx, json, u) != 0) {
        return -1;
    }
    for (i = 0; i < u->load_count; i++) {
        if (!u->loads[i].from_register && u->data == NULL) {
            return kind_error(ctx, "a load from \"data\" needs \"data\"");
        }
        if (u->loads[i].from_register && !u->has_input_register) {
            return kind_error(ctx, "a load from \"register\" needs \"inputRegister\"");
        }
    }
    if (u->has_modes && u->data == NULL) {
        return kind_error(ctx, "\"modes\" needs \"data\"");
    }
    if (u->has_input_register && u->ireg_data == NULL && !u->has_bus) {
        return kind_error(ctx, "\"inputRegister\" needs \"data\" or a \"bus\"");
    }
    if (u->count == 0 && u->direction.pin == NULL && !u->has_modes && u->clock_up == NULL) {
        return kind_error(ctx, "the count direction needs \"count\", \"direction\", \"modes\" or "
                               "\"clockUp\"/\"clockDown\"");
    }
    item = cJSON_GetObjectItemCaseSensitive(json, "zeroLatch");
    if (item != NULL) {
        static const char *const KEYS[] = {"pin", "cascade", NULL};

        kind_push(ctx, "zeroLatch");
        if (kind_check_keys(ctx, item, KEYS, NULL) != 0 ||
            kind_string(ctx, item, "pin", &u->zero_pin) != 0 ||
            kind_string(ctx, item, "cascade", &u->zero_cascade) != 0) {
            return -1;
        }
        kind_pop(ctx);
        if (u->load_count == 0) {
            return kind_error(ctx, "\"zeroLatch\" needs a load (its preset enable)");
        }
    }
    return 0;
}

void *cnt_sync_load(KindContext *ctx, const cJSON *json, unsigned *units_out) {
    SyncDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const cJSON *units = kind_member(ctx, json, "units", 1);
    const cJSON *item;
    size_t n = 0;

    if (dev == NULL || units == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(units) || cJSON_GetArraySize(units) < 1 ||
        cJSON_GetArraySize(units) > SYNC_MAX_UNITS) {
        kind_error(ctx, "\"units\" must be an array of 1 to %d units", SYNC_MAX_UNITS);
        return NULL;
    }
    cJSON_ArrayForEach(item, units) {
        kind_push(ctx, "units[%zu]", n);
        if (sync_load_unit(ctx, item, &dev->units[n]) != 0) {
            return NULL;
        }
        kind_pop(ctx);
        n++;
    }
    dev->unit_count = n;
    item = cJSON_GetObjectItemCaseSensitive(json, "byteBus");
    if (item != NULL) {
        static const char *const KEYS[] = {"outputs", "selects", NULL};
        static const char *const SEL_KEYS[] = {"pin", "unit", "byte", NULL};
        const cJSON *selects;
        const cJSON *sel;
        size_t k = 0;

        kind_push(ctx, "byteBus");
        if (kind_check_keys(ctx, item, KEYS, NULL) != 0 ||
            seq_names(ctx, item, "outputs", 8, 8, &dev->byte_outputs, &dev->byte_output_count) !=
                0) {
            return NULL;
        }
        selects = kind_member(ctx, item, "selects", 1);
        if (selects == NULL || !cJSON_IsArray(selects) || cJSON_GetArraySize(selects) < 1 ||
            cJSON_GetArraySize(selects) > 4) {
            kind_error(ctx, "\"selects\" must be an array of 1 to 4 objects");
            return NULL;
        }
        cJSON_ArrayForEach(sel, selects) {
            uint64_t unit, byte;

            kind_push(ctx, "selects[%zu]", k);
            if (kind_check_keys(ctx, sel, SEL_KEYS, NULL) != 0 ||
                kind_string(ctx, sel, "pin", &dev->selects[k].pin) != 0 ||
                kind_uint(ctx, sel, "unit", n - 1, &unit) != 0 ||
                kind_uint(ctx, sel, "byte", 3, &byte) != 0) {
                return NULL;
            }
            if (!dev->units[unit].has_register || dev->units[unit].stages < (byte + 1) * 8) {
                kind_error(ctx, "unit %llu has no register byte %llu", (unsigned long long)unit,
                           (unsigned long long)byte);
                return NULL;
            }
            dev->selects[k].unit = (unsigned)unit;
            dev->selects[k].byte = (unsigned)byte;
            kind_pop(ctx);
            k++;
        }
        dev->select_count = k;
        dev->has_byte_bus = 1;
        kind_pop(ctx);
    }
    *units_out = (unsigned)n;
    return dev;
}

static int bind_ctl(KindContext *ctx, const PartPin *pins, size_t count, const SeqControlName *n,
                    SeqControl *c) {
    return seq_bind_control(ctx, pins, count, n, c);
}

static int sync_bind_unit(KindContext *ctx, const SyncUnitDef *d, SyncUnit *u,
                          const PartPin *pins, size_t count) {
    size_t i;

    u->def = d;
    if (seq_bind(ctx, pins, count, d->code_pin, &u->code_pin) != 0 ||
        cnt_bind_names(ctx, pins, count, d->q, d->q_count, u->q) != 0 ||
        seq_bind_clock(ctx, pins, count, &d->clock, &u->clock) != 0 ||
        seq_bind(ctx, pins, count, d->clock_up, &u->clock_up) != 0 ||
        seq_bind(ctx, pins, count, d->clock_down, &u->clock_down) != 0 ||
        seq_bind_gate(ctx, pins, count, &d->gate, &u->gate) != 0 ||
        bind_ctl(ctx, pins, count, &d->direction, &u->direction) != 0 ||
        cnt_bind_names(ctx, pins, count, d->data, d->data_count, u->data) != 0 ||
        bind_ctl(ctx, pins, count, &d->oe, &u->oe) != 0 ||
        seq_bind_clock(ctx, pins, count, &d->reg_clock, &u->reg_clock) != 0 ||
        cnt_bind_names(ctx, pins, count, d->reg_outputs, d->reg_output_count, u->reg_outputs) != 0 ||
        bind_ctl(ctx, pins, count, &d->reg_oe, &u->reg_oe) != 0 ||
        seq_bind(ctx, pins, count, d->reg_select, &u->reg_select) != 0 ||
        seq_bind_clock(ctx, pins, count, &d->ireg_clock, &u->ireg_clock) != 0 ||
        cnt_bind_names(ctx, pins, count, d->ireg_data, d->ireg_data_count, u->ireg_data) != 0 ||
        bind_ctl(ctx, pins, count, &d->ireg_enable, &u->ireg_enable) != 0 ||
        cnt_bind_names(ctx, pins, count, d->bus, d->bus_count, u->bus) != 0 ||
        seq_bind(ctx, pins, count, d->zero_pin, &u->zero_pin) != 0 ||
        seq_bind(ctx, pins, count, d->zero_cascade, &u->zero_cascade) != 0) {
        return -1;
    }
    for (i = 0; i < 2; i++) {
        if (seq_bind(ctx, pins, count, d->has_modes ? d->mode_pins[i] : NULL, &u->mode_pins[i]) !=
            0) {
            return -1;
        }
    }
    for (i = 0; i < d->enable_count; i++) {
        if (bind_ctl(ctx, pins, count, &d->enables[i], &u->enables[i]) != 0) {
            return -1;
        }
    }
    for (i = 0; i < d->any_count; i++) {
        if (bind_ctl(ctx, pins, count, &d->any[i], &u->any[i]) != 0) {
            return -1;
        }
    }
    for (i = 0; i < d->clear_count; i++) {
        u->clears[i].to_max = d->clears[i].to_max;
        if (bind_ctl(ctx, pins, count, &d->clears[i].c, &u->clears[i].c) != 0) {
            return -1;
        }
    }
    for (i = 0; i < d->load_count; i++) {
        u->loads[i].from_register = d->loads[i].from_register;
        if (bind_ctl(ctx, pins, count, &d->loads[i].c, &u->loads[i].c) != 0) {
            return -1;
        }
    }
    for (i = 0; i < d->carry_count; i++) {
        const SyncCarryName *cn = &d->carries[i];
        SyncCarry *c = &u->carries[i];
        size_t j;

        c->at = cn->at;
        c->gate_count = cn->gate_count;
        if (bind_ctl(ctx, pins, count, &cn->c, &c->c) != 0 ||
            seq_bind(ctx, pins, count, cn->clock_low, &c->clock_low) != 0) {
            return -1;
        }
        for (j = 0; j < cn->gate_count; j++) {
            if (bind_ctl(ctx, pins, count, &cn->gates[j], &c->gates[j]) != 0) {
                return -1;
            }
        }
    }
    for (i = 0; i < d->bus_oe_count; i++) {
        if (bind_ctl(ctx, pins, count, &d->bus_oe[i], &u->bus_oe[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

void *cnt_sync_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const SyncDevice *dev = raw;
    SyncBound *b = kind_alloc(ctx, sizeof(*b));
    size_t i;

    if (b == NULL) {
        return NULL;
    }
    b->dev = dev;
    for (i = 0; i < dev->unit_count; i++) {
        kind_push(ctx, "units[%zu]", i);
        if (sync_bind_unit(ctx, &dev->units[i], &b->units[i], pins, count) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    if (dev->has_byte_bus) {
        kind_push(ctx, "byteBus");
        if (cnt_bind_names(ctx, pins, count, dev->byte_outputs, 8, b->byte_outputs) != 0) {
            return NULL;
        }
        for (i = 0; i < dev->select_count; i++) {
            b->selects[i].unit = dev->selects[i].unit;
            b->selects[i].byte = dev->selects[i].byte;
            if (seq_bind(ctx, pins, count, dev->selects[i].pin, &b->selects[i].pin) != 0) {
                return NULL;
            }
        }
        kind_pop(ctx);
    }
    return b;
}

CounterHeader *cnt_sync_create(const void *dev_raw, const void *bound) {
    const SyncBound *b = bound;
    SyncState *s = calloc(1, sizeof(*s));
    size_t i;

    (void)dev_raw;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    for (i = 0; i < b->dev->unit_count; i++) {
        const SyncUnit *u = &b->units[i];
        const SyncUnitDef *d = u->def;
        SyncUnitState *us = &s->units[i];
        size_t j;

        us->count = cnt_known_bits(0, d->stages);
        us->reg = cnt_known_bits(0, d->stages);
        us->ireg = cnt_known_bits(0, d->stages);
        us->zero = LOGIC_0;
        cnt_init_outputs(us->q, u->q, d->q_count);
        cnt_init_outputs(us->reg_outputs, u->reg_outputs, d->reg_output_count);
        cnt_init_outputs(us->bus, u->bus, d->bus_count);
        for (j = 0; j < d->carry_count; j++) {
            cnt_init_outputs(&us->carries[j], &u->carries[j].c.pin, 1);
        }
        cnt_init_outputs(&us->zero_out, &u->zero_pin, 1);
    }
    cnt_init_outputs(s->byte_outputs, b->byte_outputs, b->dev->has_byte_bus ? 8 : 0);
    return &s->h;
}

