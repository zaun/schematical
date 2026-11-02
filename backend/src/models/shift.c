/*
 * shift: shift registers (plan-10-G). One kind; a device names its model
 * with "model". This file holds the kind, the dispatch and the parametric
 * "shift" model: serial and parallel inputs, mode rows, a storage register
 * or latch, extra serial-output flip-flops and shared I/O pins.
 *
 * Stages, storage and the input register are SeqBits (bit k = stage k+1);
 * wherever a control is unknown both outcomes are computed and merged.
 * Signal rules: docs/parts-format.md "Sequential kinds".
 */

#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "model_pins.h"
#include "seq.h"
#include "shift.h"

/* ---- Shared pieces ------------------------------------------------------------ */

SeqBits shift_merge(SeqBits a, SeqBits b) {
    SeqBits r;

    r.known = a.known & b.known & ~(a.value ^ b.value);
    r.value = a.value & r.known;
    return r;
}

SeqBits shift_read_bits(const Sim *sim, const SimInstance *inst, const size_t *pins, size_t n) {
    SeqBits b = {0, 0};
    size_t i;

    for (i = 0; i < n; i++) {
        seq_set_bit(&b, (unsigned)i, seq_value(seq_read(sim, inst, pins[i])));
    }
    return b;
}

void shift_drive(Sim *sim, SimInstance *inst, SeqOutput *o, LogicValue v, SeqOutputStyle style,
                 const ShiftTiming *t) {
    seq_drive(sim, inst, o, v, style, t->seq);
}

static uint64_t mask_of(unsigned n) {
    return n >= 64 ? ~(uint64_t)0 : (((uint64_t)1 << n) - 1);
}

static SeqBits zero_bits(unsigned n) {
    SeqBits b = {0, mask_of(n)};

    return b;
}

static int lo(int level) {
    return level < 0 ? 0 : level;
}

static int hi(int level) {
    return level < 0 ? 1 : level;
}

static int and3(int a, int b) {
    if (a == 0 || b == 0) {
        return 0;
    }
    return a == 1 && b == 1 ? 1 : -1;
}

static int or3(int a, int b) {
    if (a == 1 || b == 1) {
        return 1;
    }
    return a == 0 && b == 0 ? 0 : -1;
}

static int not3(int a) {
    return a < 0 ? -1 : !a;
}

static int bind_names(KindContext *ctx, const PartPin *pins, size_t count,
                      const char *const *names, size_t n, size_t *out) {
    size_t i;

    for (i = 0; i < n; i++) {
        if (seq_bind(ctx, pins, count, names[i], &out[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

static void init_outputs(SeqOutput *outputs, const size_t *pins, size_t n) {
    size_t i;

    for (i = 0; i < n; i++) {
        outputs[i].pin = pins[i];
        outputs[i].scheduled = 0;
        outputs[i].driven = LOGIC_X;
    }
}

static int control_list(KindContext *ctx, const cJSON *json, const char *key, int allow_when,
                        SeqControlName *out, size_t max, size_t *count) {
    const cJSON *list = cJSON_GetObjectItemCaseSensitive(json, key);
    const cJSON *item;
    size_t n = 0;

    *count = 0;
    if (list == NULL) {
        return 0;
    }
    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) < 1 ||
        (size_t)cJSON_GetArraySize(list) > max) {
        return kind_error(ctx, "\"%s\" must be an array of 1 to %zu controls", key, max);
    }
    cJSON_ArrayForEach(item, list) {
        kind_push(ctx, "%s[%zu]", key, n);
        if (seq_control_object(ctx, item, allow_when, &out[n]) != 0) {
            return -1;
        }
        kind_pop(ctx);
        n++;
    }
    *count = n;
    return 0;
}

/* ---- The shift model ------------------------------------------------------------- */

#define SH_MAX_UNITS 2
#define SH_MAX_STAGES 64
#define SH_MAX_LIST 3
#define SH_MAX_OUTPUTS 8

enum { SERIAL_AND, SERIAL_JK, SERIAL_SELECT };
enum { ROW_HOLD, ROW_RIGHT, ROW_LEFT, ROW_LOAD };
enum { FROM_DATA, FROM_IREG, FROM_IO };

static const char *const SHIFT_KEYS[] = {"units", NULL};

typedef struct {
    unsigned stage;   /* 0-based */
    const char *pin;
    int invert;
} ShOutputName;

typedef struct {
    unsigned stages;
    SeqClockName clock;
    SeqGateName gate;
    int serial_kind;
    const char *const *serial;
    size_t serial_count;
    const char *jk_j, *jk_kbar;
    const char *sel, *sel_low, *sel_high;
    const char *left_serial;
    int has_modes;
    const char *mode_pins[2];
    int mode_table[4];
    SeqControlName enables[SH_MAX_LIST];
    size_t enable_count;
    int has_load;
    SeqControlName load;
    int load_from;
    const char *const *data;
    size_t data_count;
    SeqControlName clear;
    int clear_all;
    ShOutputName outputs[SH_MAX_OUTPUTS];
    size_t output_count;
    SeqControlName true_comp;
    int has_io;
    const char *const *io;
    size_t io_count;
    SeqControlName io_oe[SH_MAX_LIST];
    size_t io_oe_count;
    unsigned io_not_rows;   /* bit r: the io pins are not driven in mode row r */
    ShOutputName serial_outputs[2];
    size_t serial_output_count;
    int has_storage;
    SeqClockName st_clock;
    SeqControlName st_latch, st_clear, st_oe;
    const char *const *st_outputs;
    size_t st_output_count;
    SeqOutputStyle st_style;
    int st_invert;
    int has_ireg;
    SeqClockName ireg_clock;
    const char *const *ireg_data;
    size_t ireg_data_count;
    const char *clock_out;
} ShUnitDef;

typedef struct {
    ShUnitDef units[SH_MAX_UNITS];
    size_t unit_count;
} ShDevice;

typedef struct {
    const ShUnitDef *def;
    SeqClock clock;
    SeqGate gate;
    size_t serial[SH_MAX_LIST];
    size_t jk_j, jk_kbar, sel, sel_low, sel_high, left_serial;
    size_t mode_pins[2];
    SeqControl enables[SH_MAX_LIST];
    SeqControl load, clear, true_comp;
    size_t data[SH_MAX_STAGES];
    size_t outputs[SH_MAX_OUTPUTS];
    size_t io[SH_MAX_STAGES];
    SeqControl io_oe[SH_MAX_LIST];
    size_t serial_outputs[2];
    SeqClock st_clock;
    SeqControl st_latch, st_clear, st_oe;
    size_t st_outputs[SH_MAX_STAGES];
    SeqClock ireg_clock;
    size_t ireg_data[SH_MAX_STAGES];
    size_t clock_out;
} ShUnit;

typedef struct {
    ShUnit units[SH_MAX_UNITS];
    size_t unit_count;
} ShBound;

typedef struct {
    SeqBits stages, storage, ireg;
    LogicValue serial_ff[2];
    LogicValue last_clock, last_st_clock, last_ireg_clock;
    SeqOutput outputs[SH_MAX_OUTPUTS];
    SeqOutput io[SH_MAX_STAGES];
    SeqOutput serial_outputs[2];
    SeqOutput st_outputs[SH_MAX_STAGES];
    SeqOutput clock_out;
} ShUnitState;

typedef struct {
    ShiftHeader h;
    const ShBound *b;
    ShUnitState units[SH_MAX_UNITS];
} ShState;

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
        control_list(ctx, io, "outputEnables", 0, u->io_oe, SH_MAX_LIST, &u->io_oe_count) != 0) {
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
                                       "output", "invert", NULL};
    const cJSON *st = cJSON_GetObjectItemCaseSensitive(json, "storage");

    if (st == NULL) {
        return 0;
    }
    kind_push(ctx, "storage");
    if (kind_check_keys(ctx, st, KEYS, NULL) != 0) {
        return -1;
    }
    if ((cJSON_GetObjectItemCaseSensitive(st, "clock") != NULL) ==
        (cJSON_GetObjectItemCaseSensitive(st, "latch") != NULL)) {
        return kind_error(ctx, "a storage has exactly one of \"clock\" and \"latch\"");
    }
    if (seq_clock_name(ctx, st, "clock", 0, &u->st_clock) != 0 ||
        seq_control_name(ctx, st, "latch", 0, 0, &u->st_latch) != 0 ||
        seq_control_name(ctx, st, "clear", 0, 0, &u->st_clear) != 0 ||
        names_n(ctx, st, "outputs", u->stages, &u->st_outputs, &u->st_output_count, 1) != 0 ||
        seq_control_name(ctx, st, "outputEnable", 0, 0, &u->st_oe) != 0 ||
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
        control_list(ctx, json, "enables", 0, u->enables, SH_MAX_LIST, &u->enable_count) != 0 ||
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

static void *sh_load(KindContext *ctx, const cJSON *json, unsigned *units_out) {
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
        bind_names(ctx, pins, count, d->serial, d->serial_count, u->serial) != 0 ||
        seq_bind(ctx, pins, count, d->jk_j, &u->jk_j) != 0 ||
        seq_bind(ctx, pins, count, d->jk_kbar, &u->jk_kbar) != 0 ||
        seq_bind(ctx, pins, count, d->sel, &u->sel) != 0 ||
        seq_bind(ctx, pins, count, d->sel_low, &u->sel_low) != 0 ||
        seq_bind(ctx, pins, count, d->sel_high, &u->sel_high) != 0 ||
        seq_bind(ctx, pins, count, d->left_serial, &u->left_serial) != 0 ||
        seq_bind_control(ctx, pins, count, &d->load, &u->load) != 0 ||
        seq_bind_control(ctx, pins, count, &d->clear, &u->clear) != 0 ||
        seq_bind_control(ctx, pins, count, &d->true_comp, &u->true_comp) != 0 ||
        bind_names(ctx, pins, count, d->data, d->data_count, u->data) != 0 ||
        bind_names(ctx, pins, count, d->io, d->io_count, u->io) != 0 ||
        seq_bind_clock(ctx, pins, count, &d->st_clock, &u->st_clock) != 0 ||
        seq_bind_control(ctx, pins, count, &d->st_latch, &u->st_latch) != 0 ||
        seq_bind_control(ctx, pins, count, &d->st_clear, &u->st_clear) != 0 ||
        seq_bind_control(ctx, pins, count, &d->st_oe, &u->st_oe) != 0 ||
        bind_names(ctx, pins, count, d->st_outputs, d->st_output_count, u->st_outputs) != 0 ||
        seq_bind_clock(ctx, pins, count, &d->ireg_clock, &u->ireg_clock) != 0 ||
        bind_names(ctx, pins, count, d->ireg_data, d->ireg_data_count, u->ireg_data) != 0 ||
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

static void *sh_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
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

static ShiftHeader *sh_create(const void *dev, const void *bound) {
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

        us->stages = zero_bits(d->stages);
        us->storage = zero_bits(d->stages);
        us->ireg = zero_bits(d->stages);
        us->serial_ff[0] = us->serial_ff[1] = LOGIC_0;
        init_outputs(us->outputs, u->outputs, d->output_count);
        init_outputs(us->io, u->io, d->io_count);
        init_outputs(us->serial_outputs, u->serial_outputs, d->serial_output_count);
        init_outputs(us->st_outputs, u->st_outputs, d->st_output_count);
        init_outputs(&us->clock_out, &u->clock_out, 1);
    }
    return &s->h;
}

/* ---- running ---- */

typedef struct {
    const Sim *sim;
    const SimInstance *inst;
    const ShUnit *u;
    ShUnitState *us;
    SeqBits cur;
    int edge;
} ShEval;

static LogicValue effective_clock(const Sim *sim, const SimInstance *inst, const ShUnit *u) {
    LogicValue clock = sim_read_pin(sim, inst, u->clock.pin);

    if (u->gate.pin != SEQ_NO_PIN) {
        clock = seq_gated_clock(clock, sim_read_pin(sim, inst, u->gate.pin), u->gate.op);
    }
    return clock;
}

static int serial_in(const ShEval *e) {
    const ShUnit *u = e->u;
    const ShUnitDef *d = u->def;

    if (d->serial_kind == SERIAL_AND) {
        int v = 1;
        size_t i;

        for (i = 0; i < d->serial_count; i++) {
            v = and3(v, seq_read(e->sim, e->inst, u->serial[i]));
        }
        return v;
    }
    if (d->serial_kind == SERIAL_JK) {
        int q1 = seq_level(seq_bit(e->cur, 0));
        int j = seq_read(e->sim, e->inst, u->jk_j);
        int kbar = seq_read(e->sim, e->inst, u->jk_kbar);

        return or3(and3(j, not3(q1)), and3(kbar, q1));
    } else {
        int sel = seq_read(e->sim, e->inst, u->sel);
        int low = seq_read(e->sim, e->inst, u->sel_low);
        int high = seq_read(e->sim, e->inst, u->sel_high);

        return sel == 0 ? low : sel == 1 ? high : low == high ? low : -1;
    }
}

static SeqBits shift_right(const ShEval *e) {
    unsigned n = e->u->def->stages;
    SeqBits r;
    uint64_t m = mask_of(n);

    r.value = (e->cur.value << 1) & m;
    r.known = (e->cur.known << 1) & m;
    seq_set_bit(&r, 0, seq_value(serial_in(e)));
    return r;
}

static SeqBits shift_left(const ShEval *e) {
    unsigned n = e->u->def->stages;
    SeqBits r;

    r.value = e->cur.value >> 1;
    r.known = e->cur.known >> 1;
    seq_set_bit(&r, n - 1, seq_value(seq_read(e->sim, e->inst, e->u->left_serial)));
    return r;
}

static SeqBits load_source(const ShEval *e, int from) {
    const ShUnit *u = e->u;
    unsigned n = u->def->stages;

    if (from == FROM_IREG) {
        return e->us->ireg;
    }
    if (from == FROM_IO) {
        return shift_read_bits(e->sim, e->inst, u->io, n);
    }
    return shift_read_bits(e->sim, e->inst, u->data, n);
}

static unsigned mode_rows(const ShEval *e) {
    int s0 = seq_read(e->sim, e->inst, e->u->mode_pins[0]);
    int s1 = seq_read(e->sim, e->inst, e->u->mode_pins[1]);
    unsigned rows = 0;
    int a, b;

    for (a = lo(s0); a <= hi(s0); a++) {
        for (b = lo(s1); b <= hi(s1); b++) {
            rows |= 1u << (a + 2 * b);
        }
    }
    return rows;
}

static SeqBits row_result(const ShEval *e, int row) {
    const ShUnitDef *d = e->u->def;

    switch (row) {
        case ROW_RIGHT:
            return shift_right(e);
        case ROW_LEFT:
            return shift_left(e);
        case ROW_LOAD:
            return load_source(e, d->data != NULL ? FROM_DATA : FROM_IO);
        default:
            return e->cur;
    }
}

/* The mode row (or a right shift without modes). */
static SeqBits shifted(const ShEval *e) {
    const ShUnitDef *d = e->u->def;
    SeqBits r = e->cur;
    int first = 1;
    unsigned rows, k;

    if (!d->has_modes) {
        return shift_right(e);
    }
    rows = mode_rows(e);
    for (k = 0; k < 4; k++) {
        if (rows & (1u << k)) {
            SeqBits s = row_result(e, d->mode_table[k]);

            r = first ? s : shift_merge(r, s);
            first = 0;
        }
    }
    return r;
}

/* After the enables: clock clear, else clock load, else the row. */
static SeqBits clocked_inner(const ShEval *e) {
    const ShUnit *u = e->u;
    const ShUnitDef *d = u->def;
    SeqBits rest;

    if (d->has_load && d->load.clocked) {
        int l = seq_sync(e->sim, e->inst, &u->load);

        rest = l == 1 ? load_source(e, d->load_from)
             : l == 0 ? shifted(e) : shift_merge(load_source(e, d->load_from), shifted(e));
    } else {
        rest = shifted(e);
    }
    if (d->clear.pin != NULL && d->clear.clocked) {
        int c = seq_sync(e->sim, e->inst, &u->clear);

        if (c == 1) {
            return zero_bits(d->stages);
        }
        if (c < 0) {
            return shift_merge(zero_bits(d->stages), rest);
        }
    }
    return rest;
}

static SeqBits clocked(const ShEval *e) {
    const ShUnit *u = e->u;
    const ShUnitDef *d = u->def;
    int en = 1;
    size_t i;

    for (i = 0; i < d->enable_count; i++) {
        en = and3(en, seq_sync(e->sim, e->inst, &u->enables[i]));
    }
    if (en == 0) {
        return e->cur;
    }
    return en == 1 ? clocked_inner(e) : shift_merge(e->cur, clocked_inner(e));
}

static SeqBits next_stages(const ShEval *e) {
    const ShUnit *u = e->u;
    const ShUnitDef *d = u->def;
    int clear = d->clear.pin != NULL && !d->clear.clocked ? seq_async(e->sim, e->inst, &u->clear)
                                                         : 0;
    int load = d->has_load && !d->load.clocked ? seq_async(e->sim, e->inst, &u->load) : 0;
    SeqBits zero = zero_bits(d->stages);
    SeqBits r = {0, 0};
    int first = 1;
    int c, l;

    for (c = lo(clear); c <= hi(clear); c++) {
        for (l = lo(load); l <= hi(load); l++) {
            SeqBits o;

            if (c) {
                o = zero;
            } else if (l) {
                o = load_source(e, d->load_from);
            } else if (e->edge == 1) {
                o = clocked(e);
            } else {
                o = e->cur;
            }
            r = first ? o : shift_merge(r, o);
            first = 0;
        }
    }
    return r;
}

static void sh_outputs(Sim *sim, SimInstance *inst, ShState *s, size_t index) {
    const ShUnit *u = &s->b->units[index];
    const ShUnitDef *d = u->def;
    ShUnitState *us = &s->units[index];
    const ShiftTiming *t = s->h.timing;
    int tc = d->true_comp.pin != NULL ? seq_sync(sim, inst, &u->true_comp) : 1;
    size_t i;

    for (i = 0; i < d->output_count; i++) {
        int v = seq_level(seq_bit(us->stages, d->outputs[i].stage));
        int inv = d->outputs[i].invert ? 1 : 0;

        if (tc == 0) {
            inv = !inv;
        } else if (tc < 0) {
            v = -1;
        }
        shift_drive(sim, inst, &us->outputs[i], seq_value(inv ? not3(v) : v), SEQ_PUSH_PULL, t);
    }
    if (d->has_io) {
        int enable = 1;

        for (i = 0; i < d->io_oe_count; i++) {
            enable = and3(enable, seq_enable(sim, inst, &u->io_oe[i]));
        }
        if (d->has_modes && d->io_not_rows != 0) {
            ShEval e = {sim, inst, u, us, us->stages, 0};
            unsigned rows = mode_rows(&e);
            int in_not = (rows & d->io_not_rows) != 0;
            int all_not = (rows & ~d->io_not_rows) == 0;

            enable = and3(enable, all_not ? 0 : in_not ? -1 : 1);
        }
        for (i = 0; i < d->io_count; i++) {
            shift_drive(sim, inst, &us->io[i], seq_gate(seq_bit(us->stages, (unsigned)i), enable),
                        SEQ_PUSH_PULL, t);
        }
    }
    for (i = 0; i < d->serial_output_count; i++) {
        shift_drive(sim, inst, &us->serial_outputs[i], us->serial_ff[i], SEQ_PUSH_PULL, t);
    }
    if (d->has_storage) {
        int enable = seq_enable(sim, inst, &u->st_oe);

        for (i = 0; i < d->st_output_count; i++) {
            LogicValue v = seq_bit(us->storage, (unsigned)i);

            if (d->st_invert) {
                v = seq_not(v);
            }
            shift_drive(sim, inst, &us->st_outputs[i], seq_gate(v, enable), d->st_style, t);
        }
    }
    if (u->clock_out != SEQ_NO_PIN) {
        shift_drive(sim, inst, &us->clock_out,
                    seq_value(seq_read(sim, inst, u->clock.pin)), SEQ_PUSH_PULL, t);
    }
}

static void sh_eval_unit(Sim *sim, SimInstance *inst, ShState *s, size_t index, int at_start) {
    const ShUnit *u = &s->b->units[index];
    const ShUnitDef *d = u->def;
    ShUnitState *us = &s->units[index];
    ShEval e = {sim, inst, u, us, us->stages, 0};
    SeqBits before = us->stages;
    LogicValue clock = effective_clock(sim, inst, u);
    int falling = 0;
    size_t i;

    /* The input register takes its edge first (a load from it passes through). */
    if (d->has_ireg) {
        LogicValue v = sim_read_pin(sim, inst, u->ireg_clock.pin);

        if (!at_start && seq_clock_edge(&u->ireg_clock, us->last_ireg_clock, v)) {
            us->ireg = shift_read_bits(sim, inst, d->ireg_data != NULL ? u->ireg_data : u->io,
                                       d->stages);
        }
        us->last_ireg_clock = v;
    }
    if (!at_start) {
        e.edge = seq_clock_edge(&u->clock, us->last_clock, clock);
        falling = u->clock.falling ? seq_rising(us->last_clock, clock)
                                   : seq_falling(us->last_clock, clock);
    }
    us->last_clock = clock;
    us->stages = next_stages(&e);

    /* Storage: a clock edge takes the stages from before this instant's shift. */
    if (d->has_storage) {
        if (d->st_clock.pin != NULL) {
            LogicValue v = sim_read_pin(sim, inst, u->st_clock.pin);

            if (!at_start && seq_clock_edge(&u->st_clock, us->last_st_clock, v)) {
                us->storage = before;
            }
            us->last_st_clock = v;
        } else {
            int latch = seq_sync(sim, inst, &u->st_latch);

            if (latch == 1) {
                us->storage = us->stages;
            } else if (latch < 0) {
                us->storage = shift_merge(us->storage, us->stages);
            }
        }
        if (d->st_clear.pin != NULL) {
            int c = seq_async(sim, inst, &u->st_clear);

            if (c == 1) {
                us->storage = zero_bits(d->stages);
            } else if (c < 0) {
                us->storage = shift_merge(us->storage, zero_bits(d->stages));
            }
        }
    }
    /* Serial-output flip-flops take their stage on the opposite clock edge. */
    if (falling) {
        for (i = 0; i < d->serial_output_count; i++) {
            us->serial_ff[i] = seq_bit(us->stages, d->serial_outputs[i].stage);
        }
    }
    /* A clear with scope "all" also clears the storage and the serial-output flip-flops. */
    if (d->clear_all && d->clear.pin != NULL && !d->clear.clocked) {
        int c = seq_async(sim, inst, &u->clear);

        if (c == 1) {
            us->storage = zero_bits(d->stages);
            us->serial_ff[0] = us->serial_ff[1] = LOGIC_0;
        } else if (c < 0) {
            us->storage = shift_merge(us->storage, zero_bits(d->stages));
            for (i = 0; i < 2; i++) {
                if (us->serial_ff[i] != LOGIC_0) {
                    us->serial_ff[i] = LOGIC_X;
                }
            }
        }
    }
    sh_outputs(sim, inst, s, index);
}

static void sh_eval(Sim *sim, SimInstance *inst, ShState *s, int at_start) {
    size_t i;

    for (i = 0; i < s->b->unit_count; i++) {
        sh_eval_unit(sim, inst, s, i, at_start);
    }
}

static void sh_start(Sim *sim, SimInstance *inst, ShiftHeader *h) {
    ShState *s = (ShState *)h;
    size_t i;

    for (i = 0; i < s->b->unit_count; i++) {
        const ShUnit *u = &s->b->units[i];
        ShUnitState *us = &s->units[i];

        us->last_clock = effective_clock(sim, inst, u);
        if (u->st_clock.pin != SEQ_NO_PIN) {
            us->last_st_clock = sim_read_pin(sim, inst, u->st_clock.pin);
        }
        if (u->ireg_clock.pin != SEQ_NO_PIN) {
            us->last_ireg_clock = sim_read_pin(sim, inst, u->ireg_clock.pin);
        }
    }
    sh_eval(sim, inst, s, 1);
}

static void sh_changed(Sim *sim, SimInstance *inst, ShiftHeader *h, size_t pin,
                       LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    sh_eval(sim, inst, (ShState *)h, 0);
}

static int sh_open_drain(const void *dev, const void *bound, size_t pin) {
    const ShBound *b = bound;
    size_t i, k;

    (void)dev;
    for (i = 0; i < b->unit_count; i++) {
        const ShUnitDef *d = b->units[i].def;

        for (k = 0; d->has_storage && d->st_style == SEQ_OPEN_DRAIN && k < d->st_output_count;
             k++) {
            if (b->units[i].st_outputs[k] == pin) {
                return 1;
            }
        }
    }
    return 0;
}

static const ShiftModel SHIFT_MODEL = {
    "shift", SHIFT_KEYS, NULL, sh_load, sh_bind, sh_create, sh_start, sh_changed, NULL,
    sh_open_drain
};

/* ---- The kind --------------------------------------------------------------------- */

static const ShiftModel *const MODELS[] = {&SHIFT_MODEL, NULL};

static const char *const DEVICE_KEYS[] = {"model", "timing", "units", NULL};
static const char *const PART_KEYS[] = {"family", "vcc", "inputMax", NULL};

typedef struct {
    const ShiftModel *model;
    void *dev;
    SeqTimingTable timing;
    ShiftTiming *timings;
} ShiftDevice;

static void *shift_load_device(KindContext *ctx, const cJSON *json) {
    ShiftDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const char *extra_keys[SHIFT_MAX_EXTRA + 1] = {NULL};
    const char *name;
    unsigned units = 0;
    size_t i, k;

    if (dev == NULL || kind_string(ctx, json, "model", &name) != 0) {
        return NULL;
    }
    for (i = 0; MODELS[i] != NULL && strcmp(MODELS[i]->name, name) != 0; i++) {
    }
    if (MODELS[i] == NULL) {
        kind_error(ctx, "unknown model \"%s\"", name);
        return NULL;
    }
    dev->model = MODELS[i];
    if (seq_check_model_keys(ctx, json, name, dev->model->keys) != 0) {
        return NULL;
    }
    dev->dev = dev->model->load(ctx, json, &units);
    for (k = 0; dev->model->extra != NULL && dev->model->extra[k].key != NULL; k++) {
        extra_keys[k] = dev->model->extra[k].key;
    }
    if (dev->dev == NULL || seq_timing(ctx, json, extra_keys, units, &dev->timing) != 0) {
        return NULL;
    }
    dev->timings = kind_alloc(ctx, dev->timing.count * sizeof(*dev->timings));
    if (dev->timings == NULL) {
        return NULL;
    }
    for (i = 0; i < dev->timing.count; i++) {
        dev->timings[i].seq = &dev->timing.entries[i];
        for (k = 0; dev->model->extra != NULL && dev->model->extra[k].key != NULL; k++) {
            if (seq_timing_extra(ctx, &dev->timing.entries[i], dev->model->extra[k].key,
                                 dev->model->extra[k].required, &dev->timings[i].extra[k]) != 0) {
                return NULL;
            }
        }
    }
    return dev;
}

static void *shift_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                               size_t pin_count) {
    const ShiftDevice *dev = raw;

    return dev->model->bind(ctx, dev->dev, pins, pin_count);
}

static int shift_load_part(KindContext *ctx, const void *raw, const void *bound,
                           const cJSON *json, Part *out) {
    const ShiftDevice *dev = raw;
    const SeqTiming *entry;

    (void)bound;
    if (seq_part(ctx, &dev->timing, json, out, &entry) != 0) {
        return -1;
    }
    kind_set_part_data(out, &dev->timings[entry - dev->timing.entries]);
    return 0;
}

static size_t shift_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *shift_create(const void *config) {
    const PartInternal *internal = part_internal(config);
    const ShiftDevice *dev = internal->device;
    ShiftHeader *h = dev->model->create(dev->dev, internal->bound);

    if (h != NULL) {
        h->model = dev->model;
        h->timing = internal->data;
    }
    return h;
}

static void shift_destroy(void *state) {
    free(state);
}

static void shift_start(Sim *sim, SimInstance *instance, void *state) {
    ShiftHeader *h = state;

    h->model->start(sim, instance, h);
}

static void shift_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                              LogicValue old_value, LogicValue new_value) {
    ShiftHeader *h = state;

    h->model->changed(sim, instance, h, pin, old_value, new_value);
}

static void shift_timer(Sim *sim, SimInstance *instance, void *state, uint32_t id) {
    ShiftHeader *h = state;

    if (h->model->timer != NULL) {
        h->model->timer(sim, instance, h, id);
    }
}

static const SimComponentType SHIFT_SIM_MODEL = {
    .name = "shift",
    .pin_count = shift_pin_count,
    .create = shift_create,
    .destroy = shift_destroy,
    .start = shift_start,
    .pin_changed = shift_pin_changed,
    .timer = shift_timer,
};

static int shift_pin_open_drain(const Part *part, size_t pin) {
    const PartInternal *internal = part_internal(part);
    const ShiftDevice *dev = internal->device;

    return dev->model->open_drain != NULL && dev->model->open_drain(dev->dev, internal->bound, pin);
}

const PartKindOps SHIFT_KIND = {
    PART_SHIFT, "shift", &SHIFT_SIM_MODEL, DEVICE_KEYS, PART_KEYS,
    shift_load_device, shift_bind_pinout, shift_load_part, shift_pin_open_drain
};
