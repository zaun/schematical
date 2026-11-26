/* models/shift_load.c: part of the model in models/shift.c (split by plan-68-I/J). */
#include "shift_int.h"
/* ---- loading ---- */

static int names_n(KindContext *ctx, const cJSON *json, const char *key, size_t n,
                   const char *const **out, size_t *count, int required) {
    *out = NULL;
    *count = 0;
    if (cJSON_GetObjectItemCaseSensitive(json, key) == NULL) {
        return required ? kind_error(ctx, "missing \"%s\"", key) : 0;
    }
    return seq_names(ctx, json, key, n, n, out, count);
}

static int load_outputs(KindContext *ctx, const cJSON *json, const char *key, unsigned stages,
                        int with_invert, ShOutputName *out, size_t max, size_t *count) {
    static const char *const KEYS[] = {"stage", "pin", "invert", NULL};
    static const char *const KEYS_NO_INVERT[] = {"stage", "pin", NULL};
    const cJSON *list = cJSON_GetObjectItemCaseSensitive(json, key);
    const cJSON *item;
    size_t n = 0;

    *count = 0;
    if (list == NULL) {
        return 0;
    }
    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) < 1 ||
        (size_t)cJSON_GetArraySize(list) > max) {
        return kind_error(ctx, "\"%s\" must be an array of 1 to %zu objects", key, max);
    }
    cJSON_ArrayForEach(item, list) {
        uint64_t stage;

        kind_push(ctx, "%s[%zu]", key, n);
        if (kind_check_keys(ctx, item, with_invert ? KEYS : KEYS_NO_INVERT, NULL) != 0 ||
            kind_uint(ctx, item, "stage", stages, &stage) != 0 ||
            kind_string(ctx, item, "pin", &out[n].pin) != 0 ||
            (with_invert && seq_optional_bool(ctx, item, "invert", 0, &out[n].invert) != 0)) {
            return -1;
        }
        if (stage < 1) {
            return kind_error(ctx, "\"stage\" must be 1 to %u", stages);
        }
        out[n].stage = (unsigned)stage - 1;
        kind_pop(ctx);
        n++;
    }
    *count = n;
    return 0;
}

static int row_index(const char *name) {
    static const char *const ROWS[] = {"hold", "right", "left", "load"};
    int i;

    for (i = 0; i < 4; i++) {
        if (strcmp(ROWS[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

static int load_serial(KindContext *ctx, const cJSON *json, ShUnitDef *u) {
    int given = (cJSON_GetObjectItemCaseSensitive(json, "serial") != NULL) +
                (cJSON_GetObjectItemCaseSensitive(json, "serialJK") != NULL) +
                (cJSON_GetObjectItemCaseSensitive(json, "serialSelect") != NULL);
    const cJSON *item;

    if (given != 1) {
        return kind_error(ctx, "exactly one of \"serial\", \"serialJK\" and \"serialSelect\" is "
                               "needed");
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "serial") != NULL) {
        u->serial_kind = SERIAL_AND;
        return seq_names(ctx, json, "serial", 1, SH_MAX_LIST, &u->serial, &u->serial_count);
    }
    if ((item = cJSON_GetObjectItemCaseSensitive(json, "serialJK")) != NULL) {
        static const char *const KEYS[] = {"j", "kbar", NULL};

        u->serial_kind = SERIAL_JK;
        kind_push(ctx, "serialJK");
        if (kind_check_keys(ctx, item, KEYS, NULL) != 0 ||
            kind_string(ctx, item, "j", &u->jk_j) != 0 ||
            kind_string(ctx, item, "kbar", &u->jk_kbar) != 0) {
            return -1;
        }
        kind_pop(ctx);
        return 0;
    }
    item = cJSON_GetObjectItemCaseSensitive(json, "serialSelect");
    {
        static const char *const KEYS[] = {"select", "low", "high", NULL};

        u->serial_kind = SERIAL_SELECT;
        kind_push(ctx, "serialSelect");
        if (kind_check_keys(ctx, item, KEYS, NULL) != 0 ||
            kind_string(ctx, item, "select", &u->sel) != 0 ||
            kind_string(ctx, item, "low", &u->sel_low) != 0 ||
            kind_string(ctx, item, "high", &u->sel_high) != 0) {
            return -1;
        }
        kind_pop(ctx);
    }
    return 0;
}

static int load_modes(KindContext *ctx, const cJSON *json, ShUnitDef *u) {
    static const char *const KEYS[] = {"pins", "table", NULL};
    const cJSON *modes = cJSON_GetObjectItemCaseSensitive(json, "modes");
    const char *const *pins;
    const char *const *table;
    size_t n, i;

    if (modes == NULL) {
        return 0;
    }
    kind_push(ctx, "modes");
    if (kind_check_keys(ctx, modes, KEYS, NULL) != 0 ||
        seq_names(ctx, modes, "pins", 2, 2, &pins, &n) != 0 ||
        seq_names(ctx, modes, "table", 4, 4, &table, &n) != 0) {
        return -1;
    }
    for (i = 0; i < 4; i++) {
        u->mode_table[i] = row_index(table[i]);
        if (u->mode_table[i] < 0) {
            return kind_error(ctx, "table[%zu]: unknown mode \"%s\"", i, table[i]);
        }
    }
    u->mode_pins[0] = pins[0];
    u->mode_pins[1] = pins[1];
    kind_pop(ctx);
    u->has_modes = 1;
    return 0;
}

static int load_load(KindContext *ctx, const cJSON *json, ShUnitDef *u) {
    static const char *const KEYS[] = {"pin", "active", "when", "from", NULL};
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(json, "load");
    const char *when, *from;

    if (item == NULL) {
        return 0;
    }
    kind_push(ctx, "load");
    if (kind_check_keys(ctx, item, KEYS, NULL) != 0 ||
        kind_string(ctx, item, "pin", &u->load.pin) != 0 ||
        seq_active(ctx, item, "active", &u->load.active_high) != 0 ||
        kind_string(ctx, item, "when", &when) != 0 || kind_string(ctx, item, "from", &from) != 0) {
        return -1;
    }
    if (strcmp(when, "clock") == 0) {
        u->load.clocked = 1;
    } else if (strcmp(when, "async") != 0) {
        return kind_error(ctx, "\"when\" must be \"async\" or \"clock\", not \"%s\"", when);
    }
    if (strcmp(from, "data") == 0) {
        u->load_from = FROM_DATA;
    } else if (strcmp(from, "inputRegister") == 0) {
        u->load_from = FROM_IREG;
    } else if (strcmp(from, "io") == 0) {
        u->load_from = FROM_IO;
    } else {
        return kind_error(ctx, "\"from\" must be \"data\", \"inputRegister\" or \"io\", not \"%s\"",
                          from);
    }
    kind_pop(ctx);
    u->has_load = 1;
    return 0;
}

static int load_io(KindContext *ctx, const cJSON *json, ShUnitDef *u) {
    static const char *const KEYS[] = {"pins", "outputEnables", "notInModes", NULL};
    const cJSON *io = cJSON_GetObjectItemCaseSensitive(json, "io");
    const cJSON *rows;
    const cJSON *item;

    if (io == NULL) {
        return 0;
    }
    kind_push(ctx, "io");
    if (kind_check_keys(ctx, io, KEYS, NULL) != 0 ||
        names_n(ctx, io, "pins", u->stages, &u->io, &u->io_count, 1) != 0 ||
        shf_control_list(ctx, io, "outputEnables", 0, u->io_oe, SH_MAX_LIST, &u->io_oe_count) != 0) {
        return -1;
    }
    rows = cJSON_GetObjectItemCaseSensitive(io, "notInModes");
    if (rows != NULL) {
        if (!cJSON_IsArray(rows)) {
            return kind_error(ctx, "\"notInModes\" must be an array of mode names");
        }
        cJSON_ArrayForEach(item, rows) {
            int r = cJSON_IsString(item) ? row_index(item->valuestring) : -1;
            int k;

            if (r < 0) {
                return kind_error(ctx, "\"notInModes\" must be an array of mode names");
            }
            for (k = 0; k < 4; k++) {
                if (u->has_modes && u->mode_table[k] == r) {
                    u->io_not_rows |= 1u << k;
                }
            }
        }
    }
    kind_pop(ctx);
    u->has_io = 1;
    return 0;
}

static int load_storage(KindContext *ctx, const cJSON *json, ShUnitDef *u) {
    static const char *const KEYS[] = {"clock", "latch", "clear", "outputs", "outputEnable",
                                       "output", "invert", "blank", "allOn", "polarity", "supply",
                                       NULL};
    const cJSON *st = cJSON_GetObjectItemCaseSensitive(json, "storage");

    if (st == NULL) {
        return 0;
    }
    kind_push(ctx, "storage");
    if (kind_check_keys(ctx, st, KEYS, NULL) != 0) {
        return -1;
    }
    if (cJSON_GetObjectItemCaseSensitive(st, "clock") != NULL &&
        cJSON_GetObjectItemCaseSensitive(st, "latch") != NULL) {
        return kind_error(ctx, "a storage has at most one of \"clock\" and \"latch\"");
    }
    if (seq_clock_name(ctx, st, "clock", 0, &u->st_clock) != 0 ||
        seq_control_name(ctx, st, "latch", 0, 0, &u->st_latch) != 0 ||
        seq_control_name(ctx, st, "clear", 0, 0, &u->st_clear) != 0 ||
        names_n(ctx, st, "outputs", u->stages, &u->st_outputs, &u->st_output_count, 1) != 0 ||
        seq_control_name(ctx, st, "outputEnable", 0, 0, &u->st_oe) != 0 ||
        seq_control_name(ctx, st, "blank", 0, 0, &u->st_blank) != 0 ||
        seq_control_name(ctx, st, "allOn", 0, 0, &u->st_all_on) != 0 ||
        seq_control_name(ctx, st, "polarity", 0, 0, &u->st_polarity) != 0 ||
        seq_optional_name(ctx, st, "supply", &u->st_supply) != 0 ||
        seq_output_style(ctx, st, &u->st_style) != 0 ||
        seq_optional_bool(ctx, st, "invert", 0, &u->st_invert) != 0) {
        return -1;
    }
    kind_pop(ctx);
    u->has_storage = 1;
    return 0;
}

static int load_ireg(KindContext *ctx, const cJSON *json, ShUnitDef *u) {
    static const char *const KEYS[] = {"clock", "data", NULL};
    const cJSON *r = cJSON_GetObjectItemCaseSensitive(json, "inputRegister");

    if (r == NULL) {
        return 0;
    }
    kind_push(ctx, "inputRegister");
    if (kind_check_keys(ctx, r, KEYS, NULL) != 0 ||
        seq_clock_name(ctx, r, "clock", 1, &u->ireg_clock) != 0 ||
        names_n(ctx, r, "data", u->stages, &u->ireg_data, &u->ireg_data_count, 0) != 0) {
        return -1;
    }
    kind_pop(ctx);
    u->has_ireg = 1;
    return 0;
}

static int sh_load_unit(KindContext *ctx, const cJSON *json, ShUnitDef *u) {
    static const char *const UNIT_KEYS[] = {
        "stages", "clock", "clockGate", "serial", "serialJK", "serialSelect", "leftSerial",
        "modes", "enables", "load", "data", "clear", "clearScope", "outputs", "trueComplement",
        "io", "serialOutputs", "storage", "inputRegister", "clockOut", NULL
    };
    uint64_t stages;
    const cJSON *item;

    if (kind_check_keys(ctx, json, UNIT_KEYS, NULL) != 0 ||
        kind_uint(ctx, json, "stages", 1000, &stages) != 0) {
        return -1;
    }
    if (stages < 1 || stages > SH_MAX_STAGES) {
        return kind_error(ctx, "\"stages\" must be 1 to %d", SH_MAX_STAGES);
    }
    u->stages = (unsigned)stages;
    if (seq_clock_name(ctx, json, "clock", 1, &u->clock) != 0 ||
        seq_gate_name(ctx, json, "clockGate", &u->gate) != 0 || load_serial(ctx, json, u) != 0 ||
        seq_optional_name(ctx, json, "leftSerial", &u->left_serial) != 0 ||
        load_modes(ctx, json, u) != 0 ||
        shf_control_list(ctx, json, "enables", 0, u->enables, SH_MAX_LIST, &u->enable_count) != 0 ||
        load_load(ctx, json, u) != 0 ||
        names_n(ctx, json, "data", u->stages, &u->data, &u->data_count, 0) != 0 ||
        seq_control_name(ctx, json, "clear", 0, 1, &u->clear) != 0 ||
        load_outputs(ctx, json, "outputs", u->stages, 1, u->outputs, SH_MAX_OUTPUTS,
                     &u->output_count) != 0 ||
        seq_control_name(ctx, json, "trueComplement", 0, 0, &u->true_comp) != 0 ||
        load_io(ctx, json, u) != 0 ||
        load_outputs(ctx, json, "serialOutputs", u->stages, 0, u->serial_outputs, 2,
                     &u->serial_output_count) != 0 ||
        load_storage(ctx, json, u) != 0 || load_ireg(ctx, json, u) != 0 ||
        seq_optional_name(ctx, json, "clockOut", &u->clock_out) != 0) {
        return -1;
    }
    item = cJSON_GetObjectItemCaseSensitive(json, "clearScope");
    if (item != NULL) {
        if (!cJSON_IsString(item) ||
            (strcmp(item->valuestring, "shift") != 0 && strcmp(item->valuestring, "all") != 0)) {
            return kind_error(ctx, "\"clearScope\" must be \"shift\" or \"all\"");
        }
        u->clear_all = strcmp(item->valuestring, "all") == 0;
    }
    if (u->has_load && u->load_from == FROM_DATA && u->data == NULL) {
        return kind_error(ctx, "a load from \"data\" needs \"data\"");
    }
    if (u->has_load && u->load_from == FROM_IREG && !u->has_ireg) {
        return kind_error(ctx, "a load from \"inputRegister\" needs \"inputRegister\"");
    }
    if (u->has_load && u->load_from == FROM_IO && !u->has_io) {
        return kind_error(ctx, "a load from \"io\" needs \"io\"");
    }
    if (u->has_modes && u->data == NULL && !u->has_io) {
        int k;

        for (k = 0; k < 4; k++) {
            if (u->mode_table[k] == ROW_LOAD) {
                return kind_error(ctx, "a \"load\" mode row needs \"data\" or \"io\"");
            }
        }
    }
    if (u->has_ireg && u->ireg_data == NULL && !u->has_io) {
        return kind_error(ctx, "\"inputRegister\" needs \"data\" or \"io\"");
    }
    return 0;
}

void *shf_sh_load(KindContext *ctx, const cJSON *json, unsigned *units_out) {
    ShDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const cJSON *units = kind_member(ctx, json, "units", 1);
    const cJSON *item;
    size_t n = 0;

    if (dev == NULL || units == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(units) || cJSON_GetArraySize(units) < 1 ||
        cJSON_GetArraySize(units) > SH_MAX_UNITS) {
        kind_error(ctx, "\"units\" must be an array of 1 to %d units", SH_MAX_UNITS);
        return NULL;
    }
    cJSON_ArrayForEach(item, units) {
        kind_push(ctx, "units[%zu]", n);
        if (sh_load_unit(ctx, item, &dev->units[n]) != 0) {
            return NULL;
        }
        kind_pop(ctx);
        n++;
    }
    dev->unit_count = n;
    *units_out = (unsigned)n;
    return dev;
}

static int bind_unit(KindContext *ctx, const ShUnitDef *d, ShUnit *u, const PartPin *pins,
                     size_t count) {
    size_t i;

    u->def = d;
    if (seq_bind_clock(ctx, pins, count, &d->clock, &u->clock) != 0 ||
        seq_bind_gate(ctx, pins, count, &d->gate, &u->gate) != 0 ||
        shf_bind_names(ctx, pins, count, d->serial, d->serial_count, u->serial) != 0 ||
        seq_bind(ctx, pins, count, d->jk_j, &u->jk_j) != 0 ||
        seq_bind(ctx, pins, count, d->jk_kbar, &u->jk_kbar) != 0 ||
        seq_bind(ctx, pins, count, d->sel, &u->sel) != 0 ||
        seq_bind(ctx, pins, count, d->sel_low, &u->sel_low) != 0 ||
        seq_bind(ctx, pins, count, d->sel_high, &u->sel_high) != 0 ||
        seq_bind(ctx, pins, count, d->left_serial, &u->left_serial) != 0 ||
        seq_bind_control(ctx, pins, count, &d->load, &u->load) != 0 ||
        seq_bind_control(ctx, pins, count, &d->clear, &u->clear) != 0 ||
        seq_bind_control(ctx, pins, count, &d->true_comp, &u->true_comp) != 0 ||
        shf_bind_names(ctx, pins, count, d->data, d->data_count, u->data) != 0 ||
        shf_bind_names(ctx, pins, count, d->io, d->io_count, u->io) != 0 ||
        seq_bind_clock(ctx, pins, count, &d->st_clock, &u->st_clock) != 0 ||
        seq_bind_control(ctx, pins, count, &d->st_latch, &u->st_latch) != 0 ||
        seq_bind_control(ctx, pins, count, &d->st_clear, &u->st_clear) != 0 ||
        seq_bind_control(ctx, pins, count, &d->st_oe, &u->st_oe) != 0 ||
        seq_bind_control(ctx, pins, count, &d->st_blank, &u->st_blank) != 0 ||
        seq_bind_control(ctx, pins, count, &d->st_all_on, &u->st_all_on) != 0 ||
        seq_bind_control(ctx, pins, count, &d->st_polarity, &u->st_polarity) != 0 ||
        seq_bind(ctx, pins, count, d->st_supply, &u->st_supply) != 0 ||
        shf_bind_names(ctx, pins, count, d->st_outputs, d->st_output_count, u->st_outputs) != 0 ||
        seq_bind_clock(ctx, pins, count, &d->ireg_clock, &u->ireg_clock) != 0 ||
        shf_bind_names(ctx, pins, count, d->ireg_data, d->ireg_data_count, u->ireg_data) != 0 ||
        seq_bind(ctx, pins, count, d->clock_out, &u->clock_out) != 0) {
        return -1;
    }
    for (i = 0; i < 2; i++) {
        if (seq_bind(ctx, pins, count, d->has_modes ? d->mode_pins[i] : NULL, &u->mode_pins[i]) !=
            0) {
            return -1;
        }
    }
    for (i = 0; i < d->enable_count; i++) {
        if (seq_bind_control(ctx, pins, count, &d->enables[i], &u->enables[i]) != 0) {
            return -1;
        }
    }
    for (i = 0; i < d->io_oe_count; i++) {
        if (seq_bind_control(ctx, pins, count, &d->io_oe[i], &u->io_oe[i]) != 0) {
            return -1;
        }
    }
    for (i = 0; i < d->output_count; i++) {
        if (seq_bind(ctx, pins, count, d->outputs[i].pin, &u->outputs[i]) != 0) {
            return -1;
        }
    }
    for (i = 0; i < d->serial_output_count; i++) {
        if (seq_bind(ctx, pins, count, d->serial_outputs[i].pin, &u->serial_outputs[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

void *shf_sh_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const ShDevice *dev = raw;
    ShBound *b = kind_alloc(ctx, sizeof(*b));
    size_t i;

    if (b == NULL) {
        return NULL;
    }
    for (i = 0; i < dev->unit_count; i++) {
        kind_push(ctx, "units[%zu]", i);
        if (bind_unit(ctx, &dev->units[i], &b->units[i], pins, count) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    b->unit_count = dev->unit_count;
    return b;
}

ShiftHeader *shf_sh_create(const void *dev, const void *bound) {
    const ShBound *b = bound;
    ShState *s = calloc(1, sizeof(*s));
    size_t i;

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    for (i = 0; i < b->unit_count; i++) {
        const ShUnit *u = &b->units[i];
        const ShUnitDef *d = u->def;
        ShUnitState *us = &s->units[i];

        us->stages = shf_zero_bits(d->stages);
        us->storage = shf_zero_bits(d->stages);
        us->ireg = shf_zero_bits(d->stages);
        us->serial_ff[0] = us->serial_ff[1] = LOGIC_0;
        shf_init_outputs(us->outputs, u->outputs, d->output_count);
        shf_init_outputs(us->io, u->io, d->io_count);
        shf_init_outputs(us->serial_outputs, u->serial_outputs, d->serial_output_count);
        shf_init_outputs(us->st_outputs, u->st_outputs, d->st_output_count);
        shf_init_outputs(&us->clock_out, &u->clock_out, 1);
    }
    return &s->h;
}

