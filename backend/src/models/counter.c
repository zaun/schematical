/*
 * counter: counters and dividers (plan-10-D section 5). One kind; a device
 * names its model with "model":
 *
 *   sync     synchronous counters, with the options of the register
 *            counters (590, 592, 593, 697, 8154) and the 4522B zero latch
 *   ripple   ripple (asynchronous) counters: binary chains, the 74x90/92/390
 *            divide-by-2/5/6 sections
 *
 *   johnson, seg7, divn, rate   counter_div.c (plan-10-E)
 *
 * Counts are SeqBits (value + known mask), so an X input makes X exactly the
 * bits it could change: wherever a control is unknown, both outcomes are
 * computed and merged. Signal rules: docs/parts-format.md "Sequential kinds".
 */

#include <stdlib.h>
#include <string.h>

#include "counter.h"
#include "kind.h"
#include "model_pins.h"
#include "seq.h"

/* ---- Shared pieces --------------------------------------------------------- */

SeqBits counter_merge(SeqBits a, SeqBits b) {
    SeqBits r;

    r.known = a.known & b.known & ~(a.value ^ b.value);
    r.value = a.value & r.known;
    return r;
}

SeqBits counter_read_bits(const Sim *sim, const SimInstance *inst, const size_t *pins,
                          size_t count) {
    SeqBits b = {0, 0};
    size_t i;

    for (i = 0; i < count; i++) {
        seq_set_bit(&b, (unsigned)i, seq_value(seq_read(sim, inst, pins[i])));
    }
    return b;
}

void counter_drive(Sim *sim, SimInstance *inst, SeqOutput *o, LogicValue v,
                   const CounterTiming *t, SimTime extra_ps) {
    SeqTiming shifted = *t->seq;

    shifted.rise += extra_ps;
    shifted.fall += extra_ps;
    seq_drive(sim, inst, o, v, SEQ_PUSH_PULL, &shifted);
}

static uint64_t mask_of(unsigned n) {
    return n >= 64 ? ~(uint64_t)0 : (((uint64_t)1 << n) - 1);
}

static SeqBits known_bits(uint64_t value, unsigned n) {
    SeqBits b;

    b.known = mask_of(n);
    b.value = value & b.known;
    return b;
}

/* 1 equal, 0 differs in a known bit, -1 unknown; over n bits. */
static int bits_equal(SeqBits b, uint64_t value, unsigned n) {
    uint64_t m = mask_of(n);

    if (((b.value ^ value) & b.known & m) != 0) {
        return 0;
    }
    return (b.known & m) == m ? 1 : -1;
}

static int lo(int level) {
    return level < 0 ? 0 : level;
}

static int hi(int level) {
    return level < 0 ? 1 : level;
}

/* Three-valued AND / NOT. */
static int and3(int a, int b) {
    if (a == 0 || b == 0) {
        return 0;
    }
    return a == 1 && b == 1 ? 1 : -1;
}

static int not3(int a) {
    return a < 0 ? -1 : !a;
}

static int or3(int a, int b) {
    if (a == 1 || b == 1) {
        return 1;
    }
    return a == 0 && b == 0 ? 0 : -1;
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

/* An optional array of control objects, at most max. */
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

/* ---- sync ----------------------------------------------------------------- */

#define SYNC_MAX_UNITS 2
#define SYNC_MAX_STAGES 32
#define SYNC_MAX_LIST 3

enum { AT_MAX, AT_MIN, AT_TERMINAL };
enum { M_CLEAR, M_DOWN, M_LOAD, M_UP, M_HOLD };

static const char *const SYNC_KEYS[] = {"units", "byteBus", NULL};

typedef struct {
    SeqControlName c;
    int to_max;
} SyncClearName;

typedef struct {
    SeqControlName c;
    int from_register;
} SyncLoadName;

typedef struct {
    SeqControlName c;
    int at;
    SeqControlName gates[SYNC_MAX_LIST];
    size_t gate_count;
    const char *clock_low;
} SyncCarryName;

typedef struct {
    unsigned stages;
    int bcd;
    const char *code_pin;   /* codeSelect */
    int code_binary_high;
    const char *const *q;
    size_t q_count;
    SeqClockName clock;
    const char *clock_up, *clock_down;
    SeqGateName gate;
    int count;              /* 1 up, -1 down, 0 from direction/modes/dual clocks */
    SeqControlName direction;   /* active_high = "up" is high */
    SeqControlName enables[SYNC_MAX_LIST];
    size_t enable_count;
    SeqControlName any[SYNC_MAX_LIST];
    size_t any_count;
    SyncClearName clears[SYNC_MAX_LIST];
    size_t clear_count;
    SyncLoadName loads[SYNC_MAX_LIST];
    size_t load_count;
    const char *const *data;
    size_t data_count;
    int has_modes;
    const char *mode_pins[2];
    int mode_table[4];
    int mode_clear_async;
    int has_illegal;
    unsigned char illegal_up[16], illegal_down[16];
    SyncCarryName carries[SYNC_MAX_LIST];
    size_t carry_count;
    SeqControlName oe;
    int has_register;
    SeqClockName reg_clock;
    const char *const *reg_outputs;
    size_t reg_output_count;
    SeqControlName reg_oe;
    const char *reg_select;
    int reg_select_high;
    int has_input_register;
    SeqClockName ireg_clock;
    const char *const *ireg_data;
    size_t ireg_data_count;
    SeqControlName ireg_enable;
    int has_bus;
    const char *const *bus;
    size_t bus_count;
    SeqControlName bus_oe[SYNC_MAX_LIST];
    size_t bus_oe_count;
    const char *zero_pin, *zero_cascade;
} SyncUnitDef;

typedef struct {
    const char *pin;
    unsigned unit;
    unsigned byte;
} SyncSelectName;

typedef struct {
    SyncUnitDef units[SYNC_MAX_UNITS];
    size_t unit_count;
    int has_byte_bus;
    const char *const *byte_outputs;
    size_t byte_output_count;
    SyncSelectName selects[4];
    size_t select_count;
} SyncDevice;

typedef struct {
    SeqControl c;
    int to_max;
} SyncClear;

typedef struct {
    SeqControl c;
    int from_register;
} SyncLoad;

typedef struct {
    SeqControl c;
    int at;
    SeqControl gates[SYNC_MAX_LIST];
    size_t gate_count;
    size_t clock_low;
} SyncCarry;

typedef struct {
    const SyncUnitDef *def;
    size_t code_pin;
    size_t q[SYNC_MAX_STAGES];
    SeqClock clock;
    size_t clock_up, clock_down;
    SeqGate gate;
    SeqControl direction;
    SeqControl enables[SYNC_MAX_LIST], any[SYNC_MAX_LIST];
    SyncClear clears[SYNC_MAX_LIST];
    SyncLoad loads[SYNC_MAX_LIST];
    size_t data[SYNC_MAX_STAGES];
    size_t mode_pins[2];
    SyncCarry carries[SYNC_MAX_LIST];
    SeqControl oe;
    SeqClock reg_clock;
    size_t reg_outputs[SYNC_MAX_STAGES];
    SeqControl reg_oe;
    size_t reg_select;
    SeqClock ireg_clock;
    size_t ireg_data[SYNC_MAX_STAGES];
    SeqControl ireg_enable;
    size_t bus[SYNC_MAX_STAGES];
    SeqControl bus_oe[SYNC_MAX_LIST];
    size_t zero_pin, zero_cascade;
} SyncUnit;

typedef struct {
    size_t pin;
    unsigned unit;
    unsigned byte;
} SyncSelect;

typedef struct {
    const SyncDevice *dev;
    SyncUnit units[SYNC_MAX_UNITS];
    size_t byte_outputs[8];
    SyncSelect selects[4];
} SyncBound;

typedef struct {
    SeqBits count, reg, ireg;
    LogicValue last_clock, last_up, last_down, last_reg_clock, last_ireg_clock;
    LogicValue zero;
    SeqOutput q[SYNC_MAX_STAGES];
    SeqOutput carries[SYNC_MAX_LIST];
    SeqOutput reg_outputs[SYNC_MAX_STAGES];
    SeqOutput bus[SYNC_MAX_STAGES];
    SeqOutput zero_out;
} SyncUnitState;

typedef struct {
    CounterHeader h;
    const SyncBound *b;
    SyncUnitState units[SYNC_MAX_UNITS];
    SeqOutput byte_outputs[8];
} SyncState;

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
            control_list(ctx, item, "gates", 0, c->gates, SYNC_MAX_LIST, &c->gate_count) != 0 ||
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
        control_list(ctx, bus, "outputEnables", 0, u->bus_oe, SYNC_MAX_LIST, &u->bus_oe_count) !=
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
    if (control_list(ctx, json, "enables", 0, u->enables, SYNC_MAX_LIST, &u->enable_count) != 0 ||
        control_list(ctx, json, "enablesAny", 0, u->any, SYNC_MAX_LIST, &u->any_count) != 0) {
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

static void *sync_load(KindContext *ctx, const cJSON *json, unsigned *units_out) {
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
        bind_names(ctx, pins, count, d->q, d->q_count, u->q) != 0 ||
        seq_bind_clock(ctx, pins, count, &d->clock, &u->clock) != 0 ||
        seq_bind(ctx, pins, count, d->clock_up, &u->clock_up) != 0 ||
        seq_bind(ctx, pins, count, d->clock_down, &u->clock_down) != 0 ||
        seq_bind_gate(ctx, pins, count, &d->gate, &u->gate) != 0 ||
        bind_ctl(ctx, pins, count, &d->direction, &u->direction) != 0 ||
        bind_names(ctx, pins, count, d->data, d->data_count, u->data) != 0 ||
        bind_ctl(ctx, pins, count, &d->oe, &u->oe) != 0 ||
        seq_bind_clock(ctx, pins, count, &d->reg_clock, &u->reg_clock) != 0 ||
        bind_names(ctx, pins, count, d->reg_outputs, d->reg_output_count, u->reg_outputs) != 0 ||
        bind_ctl(ctx, pins, count, &d->reg_oe, &u->reg_oe) != 0 ||
        seq_bind(ctx, pins, count, d->reg_select, &u->reg_select) != 0 ||
        seq_bind_clock(ctx, pins, count, &d->ireg_clock, &u->ireg_clock) != 0 ||
        bind_names(ctx, pins, count, d->ireg_data, d->ireg_data_count, u->ireg_data) != 0 ||
        bind_ctl(ctx, pins, count, &d->ireg_enable, &u->ireg_enable) != 0 ||
        bind_names(ctx, pins, count, d->bus, d->bus_count, u->bus) != 0 ||
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

static void *sync_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
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
        if (bind_names(ctx, pins, count, dev->byte_outputs, 8, b->byte_outputs) != 0) {
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

static CounterHeader *sync_create(const void *dev_raw, const void *bound) {
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

        us->count = known_bits(0, d->stages);
        us->reg = known_bits(0, d->stages);
        us->ireg = known_bits(0, d->stages);
        us->zero = LOGIC_0;
        init_outputs(us->q, u->q, d->q_count);
        init_outputs(us->reg_outputs, u->reg_outputs, d->reg_output_count);
        init_outputs(us->bus, u->bus, d->bus_count);
        for (j = 0; j < d->carry_count; j++) {
            init_outputs(&us->carries[j], &u->carries[j].c.pin, 1);
        }
        init_outputs(&us->zero_out, &u->zero_pin, 1);
    }
    init_outputs(s->byte_outputs, b->byte_outputs, b->dev->has_byte_bus ? 8 : 0);
    return &s->h;
}

/* ---- sync: counting --------------------------------------------------------- */

/* The all-max value: every bit (binary) or every digit 9 (bcd). */
static uint64_t max_value(unsigned stages, int bcd) {
    uint64_t v = 0;
    unsigned d;

    if (!bcd) {
        return mask_of(stages);
    }
    for (d = 0; d < stages / 4; d++) {
        v |= (uint64_t)9 << (4 * d);
    }
    return v;
}

/* Binary +1 / -1 with three-valued ripple carry. */
static SeqBits step_binary(SeqBits cur, unsigned stages, int up) {
    SeqBits r = {0, 0};
    int carry = 1;
    unsigned i;

    for (i = 0; i < stages; i++) {
        int b = seq_level(seq_bit(cur, i));
        int sum;

        if (carry == 0) {
            sum = b;
        } else if (b < 0 || carry < 0) {
            sum = -1;
        } else {
            sum = b ^ 1;
        }
        seq_set_bit(&r, i, seq_value(sum));
        carry = up ? and3(b, carry) : and3(not3(b), carry);
    }
    return r;
}

static unsigned digit_next(const SyncUnitDef *d, unsigned v, int up) {
    if (v < 10) {
        return up ? (v + 1) % 10 : (v + 9) % 10;
    }
    if (d->has_illegal) {
        return up ? d->illegal_up[v] : d->illegal_down[v];
    }
    return up ? (v + 1) & 15 : (v - 1) & 15;
}

/* BCD: each digit steps when every lower digit is at its terminal (9 up, 0 down). */
static SeqBits step_bcd(const SyncUnitDef *d, SeqBits cur, unsigned stages, int up) {
    SeqBits r = cur;
    int carry = 1;
    unsigned digit;

    for (digit = 0; digit < stages / 4 && carry != 0; digit++) {
        unsigned shift = 4 * digit;
        int known = ((cur.known >> shift) & 15u) == 15u;
        unsigned v = (unsigned)((cur.value >> shift) & 15u);
        uint64_t clear = ~((uint64_t)15 << shift);
        int terminal;

        if (!known) {
            r.known &= clear;
            r.value &= clear;
            carry = -1;
            continue;
        }
        terminal = up ? v == 9 : v == 0;
        if (carry == 1) {
            unsigned n = digit_next(d, v, up);

            r.value = (r.value & clear) | ((uint64_t)n << shift);
        } else {
            /* Unknown carry in: the digit may or may not step. */
            unsigned n = digit_next(d, v, up);
            SeqBits a = {(uint64_t)v << shift, (uint64_t)15 << shift};
            SeqBits b = {(uint64_t)n << shift, (uint64_t)15 << shift};
            SeqBits m = counter_merge(a, b);

            r.known = (r.known & clear) | m.known;
            r.value = (r.value & clear) | m.value;
        }
        carry = terminal ? carry : 0;
    }
    return r;
}

typedef struct {
    const Sim *sim;
    const SimInstance *inst;
    const SyncUnit *u;
    SyncUnitState *us;
    SeqBits cur;
    int edge;       /* 1, 0, -1: the effective clock had its edge */
    int dual_up;    /* dual clocks: direction of the edge (1 up, 0 down, -1 unknown) */
} SyncEval;

/* Code of the counter at this edge: 1 bcd, 0 binary, -1 unknown. */
static int unit_bcd(const SyncEval *e) {
    const SyncUnitDef *d = e->u->def;

    if (d->code_pin != NULL) {
        int level = seq_read(e->sim, e->inst, e->u->code_pin);

        if (level < 0) {
            return -1;
        }
        return (level == 1) != (d->code_binary_high != 0);
    }
    return d->bcd;
}

static SeqBits step(const SyncEval *e, int up) {
    const SyncUnitDef *d = e->u->def;
    int bcd = unit_bcd(e);
    SeqBits r = {0, 0};
    int k;

    for (k = lo(bcd); k <= hi(bcd); k++) {
        SeqBits s = k ? step_bcd(d, e->cur, d->stages, up) : step_binary(e->cur, d->stages, up);

        r = k == lo(bcd) ? s : counter_merge(r, s);
    }
    return r;
}

static SeqBits step_dir(const SyncEval *e, int up_level) {
    if (up_level == 1) {
        return step(e, 1);
    }
    if (up_level == 0) {
        return step(e, 0);
    }
    return counter_merge(step(e, 1), step(e, 0));
}

static int enables_level(const SyncEval *e) {
    const SyncUnitDef *d = e->u->def;
    int all = 1;
    int any = d->any_count > 0 ? 0 : 1;
    size_t i;

    for (i = 0; i < d->enable_count; i++) {
        all = and3(all, seq_sync(e->sim, e->inst, &e->u->enables[i]));
    }
    for (i = 0; i < d->any_count; i++) {
        any = or3(any, seq_sync(e->sim, e->inst, &e->u->any[i]));
    }
    return and3(all, any);
}

/* The unit's count direction when not given by modes: 1 up, 0 down, -1 unknown. */
static int direction_level(const SyncEval *e) {
    const SyncUnitDef *d = e->u->def;

    if (d->clock_up != NULL) {
        return e->dual_up;
    }
    if (d->direction.pin != NULL) {
        return seq_sync(e->sim, e->inst, &e->u->direction);
    }
    return d->count > 0;
}

static SeqBits load_value(const SyncEval *e, int from_register) {
    if (from_register) {
        return e->us->ireg;
    }
    return counter_read_bits(e->sim, e->inst, e->u->data, e->u->def->stages);
}

static SeqBits counted(const SyncEval *e, int up_level) {
    int en = enables_level(e);

    if (en == 0) {
        return e->cur;
    }
    if (en == 1) {
        return step_dir(e, up_level);
    }
    return counter_merge(e->cur, step_dir(e, up_level));
}

/* The mode rows the S pins may select (bit r set = row r possible). */
static unsigned mode_rows(const SyncEval *e) {
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

static SeqBits mode_row_result(const SyncEval *e, int action) {
    const SyncUnitDef *d = e->u->def;

    switch (action) {
        case M_CLEAR:
            return d->mode_clear_async ? e->cur : known_bits(0, d->stages);
        case M_LOAD:
            return load_value(e, 0);
        case M_UP:
            return counted(e, 1);
        case M_DOWN:
            return counted(e, 0);
        default:
            return e->cur;
    }
}

/* The clocked step, after clock clears and loads (index i into them). */
static SeqBits clocked(const SyncEval *e, size_t i) {
    const SyncUnitDef *d = e->u->def;
    size_t n_clears = d->clear_count;
    size_t n = n_clears + d->load_count;

    for (; i < n; i++) {
        const SeqControl *c = i < n_clears ? &e->u->clears[i].c : &e->u->loads[i - n_clears].c;
        int level;
        SeqBits r;

        if (!c->clocked) {
            continue;
        }
        level = seq_sync(e->sim, e->inst, c);
        if (level == 0) {
            continue;
        }
        r = i < n_clears ? known_bits(e->u->clears[i].to_max ? max_value(d->stages, d->bcd) : 0,
                                      d->stages)
                         : load_value(e, e->u->loads[i - n_clears].from_register);
        return level == 1 ? r : counter_merge(r, clocked(e, i + 1));
    }
    if (d->has_modes) {
        unsigned rows = mode_rows(e);
        SeqBits r = {0, 0};
        int first = 1;
        unsigned row;

        for (row = 0; row < 4; row++) {
            if (rows & (1u << row)) {
                SeqBits s = mode_row_result(e, d->mode_table[row]);

                r = first ? s : counter_merge(r, s);
                first = 0;
            }
        }
        return r;
    }
    return counted(e, direction_level(e));
}

static SeqBits after_clock(const SyncEval *e) {
    SeqBits r;

    if (e->edge == 0) {
        return e->cur;
    }
    r = clocked(e, 0);
    return e->edge == 1 ? r : counter_merge(e->cur, r);
}

/* The modes' asynchronous clear: 1 / 0 / -1. */
static int mode_async_clear(const SyncEval *e) {
    const SyncUnitDef *d = e->u->def;
    unsigned rows;
    int any = 0, all = 1;
    unsigned row;

    if (!d->has_modes || !d->mode_clear_async) {
        return 0;
    }
    rows = mode_rows(e);
    for (row = 0; row < 4; row++) {
        if (rows & (1u << row)) {
            if (d->mode_table[row] == M_CLEAR) {
                any = 1;
            } else {
                all = 0;
            }
        }
    }
    return !any ? 0 : all ? 1 : -1;
}

/* Asynchronous controls in order (clears, loads, the modes clear), then the clock. */
static SeqBits async_chain(const SyncEval *e, size_t i) {
    const SyncUnitDef *d = e->u->def;
    size_t n_clears = d->clear_count;
    size_t n = n_clears + d->load_count;

    for (; i <= n; i++) {
        int level;
        SeqBits r;

        if (i == n) {
            level = mode_async_clear(e);
            r = known_bits(0, d->stages);
        } else {
            const SeqControl *c =
                i < n_clears ? &e->u->clears[i].c : &e->u->loads[i - n_clears].c;

            if (c->clocked) {
                continue;
            }
            level = seq_async(e->sim, e->inst, c);
            r = i < n_clears
                    ? known_bits(e->u->clears[i].to_max ? max_value(d->stages, d->bcd) : 0,
                                 d->stages)
                    : load_value(e, e->u->loads[i - n_clears].from_register);
        }
        if (level == 0) {
            continue;
        }
        return level == 1 ? r : counter_merge(r, async_chain(e, i + 1));
    }
    return after_clock(e);
}

/* ---- sync: outputs ---------------------------------------------------------- */

static int carry_level(const SyncEval *e, const SyncCarry *c) {
    const SyncUnitDef *d = e->u->def;
    SeqBits v = e->us->count;
    int bcd = unit_bcd(e);
    int at_max = -2, at_min = bits_equal(v, 0, d->stages);
    int at, k;
    size_t i;

    for (k = lo(bcd); k <= hi(bcd); k++) {
        int m = bits_equal(v, max_value(d->stages, k), d->stages);

        at_max = at_max == -2 ? m : at_max == m ? m : -1;
    }
    if (c->at == AT_MAX) {
        at = at_max;
    } else if (c->at == AT_MIN) {
        at = at_min;
    } else if (d->has_modes) {
        unsigned rows = mode_rows(e);
        unsigned row;
        int first = 1;

        at = 0;
        for (row = 0; row < 4; row++) {
            int r;

            if (!(rows & (1u << row))) {
                continue;
            }
            r = d->mode_table[row] == M_UP ? at_max : d->mode_table[row] == M_DOWN ? at_min : 0;
            at = first ? r : at == r ? r : -1;
            first = 0;
        }
    } else {
        int up = direction_level(e);

        at = up == 1 ? at_max : up == 0 ? at_min : at_max == at_min ? at_max : -1;
    }
    for (i = 0; i < c->gate_count; i++) {
        at = and3(at, seq_sync(e->sim, e->inst, &c->gates[i]));
    }
    if (c->clock_low != SEQ_NO_PIN) {
        at = and3(at, not3(seq_read(e->sim, e->inst, c->clock_low)));
    }
    return at;
}

static void drive_bits(Sim *sim, SimInstance *inst, SeqOutput *outs, size_t n, SeqBits v,
                       int enable, const CounterTiming *t) {
    size_t i;

    for (i = 0; i < n; i++) {
        counter_drive(sim, inst, &outs[i], seq_gate(seq_bit(v, (unsigned)i), enable), t, 0);
    }
}

static void sync_outputs(Sim *sim, SimInstance *inst, SyncState *s, size_t index) {
    const SyncUnit *u = &s->b->units[index];
    const SyncUnitDef *d = u->def;
    SyncUnitState *us = &s->units[index];
    const CounterTiming *t = s->h.timing;
    SyncEval e = {sim, inst, u, us, us->count, 0, -1};
    size_t i;

    drive_bits(sim, inst, us->q, d->q_count, us->count, seq_enable(sim, inst, &u->oe), t);
    for (i = 0; i < d->carry_count; i++) {
        int level = carry_level(&e, &u->carries[i]);
        int pin_level = u->carries[i].c.active_high ? level : not3(level);

        counter_drive(sim, inst, &us->carries[i], seq_value(pin_level), t, 0);
    }
    if (d->has_register && d->reg_output_count > 0) {
        SeqBits shown = us->reg;

        if (u->reg_select != SEQ_NO_PIN) {
            int level = seq_read(sim, inst, u->reg_select);
            int reg = level < 0 ? -1 : (level == 1) == (d->reg_select_high != 0);

            shown = reg == 1 ? us->reg : reg == 0 ? us->count : counter_merge(us->reg, us->count);
        }
        drive_bits(sim, inst, us->reg_outputs, d->reg_output_count, shown,
                   seq_enable(sim, inst, &u->reg_oe), t);
    }
    if (d->has_bus) {
        int enable = 1;

        for (i = 0; i < d->bus_oe_count; i++) {
            enable = and3(enable, seq_enable(sim, inst, &u->bus_oe[i]));
        }
        drive_bits(sim, inst, us->bus, d->bus_count, us->count, enable, t);
    }
    if (d->zero_pin != NULL) {
        counter_drive(sim, inst, &us->zero_out, us->zero, t, 0);
    }
}

static void byte_bus_outputs(Sim *sim, SimInstance *inst, SyncState *s) {
    const SyncBound *b = s->b;
    const SyncDevice *dev = b->dev;
    SeqBits shown = {0, 0};
    int enable = 0;
    size_t i;

    if (!dev->has_byte_bus) {
        return;
    }
    for (i = 0; i < dev->select_count; i++) {
        int level = seq_read(sim, inst, b->selects[i].pin);

        if (level == 1) {
            continue;
        }
        if (level < 0) {
            enable = -1;
            break;
        }
        shown.value = s->units[b->selects[i].unit].reg.value >> (8 * b->selects[i].byte);
        shown.known = s->units[b->selects[i].unit].reg.known >> (8 * b->selects[i].byte);
        enable = 1;
        break;
    }
    drive_bits(sim, inst, s->byte_outputs, 8, shown, enable, s->h.timing);
}

/* ---- sync: evaluation ------------------------------------------------------- */

static LogicValue effective_clock(const Sim *sim, const SimInstance *inst, const SyncUnit *u) {
    LogicValue clock = sim_read_pin(sim, inst, u->clock.pin);

    if (u->gate.pin != SEQ_NO_PIN) {
        clock = seq_gated_clock(clock, sim_read_pin(sim, inst, u->gate.pin), u->gate.op);
    }
    return clock;
}

static void zero_latch(const SyncEval *e) {
    const SyncUnit *u = e->u;
    SyncUnitState *us = e->us;
    int is_zero = bits_equal(us->count, 0, u->def->stages);
    int load = seq_async(e->sim, e->inst, &u->loads[0].c);
    int ck = seq_level(effective_clock(e->sim, e->inst, u));
    int set = and3(is_zero, seq_read(e->sim, e->inst, u->zero_cascade));
    int reset = or3(and3(load, not3(ck)), and3(not3(is_zero), not3(load)));
    LogicValue z = LOGIC_X;
    int have = 0;
    int r, sv;

    for (r = lo(reset); r <= hi(reset); r++) {
        for (sv = lo(set); sv <= hi(set); sv++) {
            LogicValue v = r ? LOGIC_0 : sv ? LOGIC_1 : us->zero;

            z = !have ? v : z == v ? v : LOGIC_X;
            have = 1;
        }
    }
    us->zero = z;
}

static void sync_eval_unit(Sim *sim, SimInstance *inst, SyncState *s, size_t index, int at_start) {
    const SyncUnit *u = &s->b->units[index];
    const SyncUnitDef *d = u->def;
    SyncUnitState *us = &s->units[index];
    SyncEval e = {sim, inst, u, us, us->count, 0, -1};
    SeqBits before = us->count;

    /* Input register, then the output register (from the count before this edge). */
    if (d->has_input_register) {
        LogicValue v = sim_read_pin(sim, inst, u->ireg_clock.pin);

        if (!at_start && seq_clock_edge(&u->ireg_clock, us->last_ireg_clock, v)) {
            int en = d->ireg_enable.pin != NULL ? seq_sync(sim, inst, &u->ireg_enable) : 1;
            SeqBits in = counter_read_bits(sim, inst, d->ireg_data != NULL ? u->ireg_data : u->bus,
                                           d->stages);

            if (en == 1) {
                us->ireg = in;
            } else if (en < 0) {
                us->ireg = counter_merge(us->ireg, in);
            }
        }
        us->last_ireg_clock = v;
    }
    if (d->has_register) {
        LogicValue v = sim_read_pin(sim, inst, u->reg_clock.pin);

        if (!at_start && seq_clock_edge(&u->reg_clock, us->last_reg_clock, v)) {
            us->reg = before;
        }
        us->last_reg_clock = v;
    }

    /* The clock edge. */
    if (d->clock_up != NULL) {
        LogicValue up = sim_read_pin(sim, inst, u->clock_up);
        LogicValue down = sim_read_pin(sim, inst, u->clock_down);
        int up_edge = seq_rising(us->last_up, up) ? seq_level(down) : 0;
        int down_edge = seq_rising(us->last_down, down) ? seq_level(up) : 0;

        if (up_edge != 0 && down_edge != 0) {
            e.edge = -1;
            e.dual_up = -1;
        } else if (up_edge != 0) {
            e.edge = up_edge;
            e.dual_up = 1;
        } else if (down_edge != 0) {
            e.edge = down_edge;
            e.dual_up = 0;
        }
        us->last_up = up;
        us->last_down = down;
    } else {
        LogicValue clock = effective_clock(sim, inst, u);

        e.edge = seq_clock_edge(&u->clock, us->last_clock, clock);
        us->last_clock = clock;
    }
    if (at_start) {
        e.edge = 0;
    }
    us->count = async_chain(&e, 0);
    if (d->zero_pin != NULL) {
        e.cur = us->count;
        zero_latch(&e);
    }
    sync_outputs(sim, inst, s, index);
}

static void sync_eval(Sim *sim, SimInstance *inst, SyncState *s, int at_start) {
    size_t i;

    for (i = 0; i < s->b->dev->unit_count; i++) {
        sync_eval_unit(sim, inst, s, i, at_start);
    }
    byte_bus_outputs(sim, inst, s);
}

static void sync_start(Sim *sim, SimInstance *inst, CounterHeader *h) {
    SyncState *s = (SyncState *)h;
    size_t i;

    for (i = 0; i < s->b->dev->unit_count; i++) {
        const SyncUnit *u = &s->b->units[i];
        SyncUnitState *us = &s->units[i];

        us->last_clock = u->def->clock_up != NULL ? LOGIC_X : effective_clock(sim, inst, u);
        us->last_up = u->clock_up != SEQ_NO_PIN ? sim_read_pin(sim, inst, u->clock_up) : LOGIC_X;
        us->last_down =
            u->clock_down != SEQ_NO_PIN ? sim_read_pin(sim, inst, u->clock_down) : LOGIC_X;
    }
    sync_eval(sim, inst, s, 1);
}

static void sync_changed(Sim *sim, SimInstance *inst, CounterHeader *h, size_t pin,
                         LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    sync_eval(sim, inst, (SyncState *)h, 0);
}

static const CounterModel SYNC_MODEL = {
    "sync", SYNC_KEYS, NULL, sync_load, sync_bind, sync_create, NULL, sync_start, sync_changed,
    NULL
};

/* ---- ripple ------------------------------------------------------------------ */

#define RIPPLE_MAX_SECTIONS 4
#define RIPPLE_MAX_STAGES 14

enum { R_BINARY, R_DIV2, R_DIV5, R_DIV6 };

static const char *const RIPPLE_KEYS[] = {"units", "sections", NULL};
static const CounterExtraKey RIPPLE_EXTRA[] = {{"stage", 1}, {NULL, 0}};

typedef struct {
    const char *const *pins;
    size_t count;
    int active_high;
    unsigned value;
} RippleCtlName;

typedef struct {
    const char *clock;
    int structure;
    unsigned stages;
    const char *q[RIPPLE_MAX_STAGES];   /* NULL = no pin */
    RippleCtlName reset, set;
    int has_reset, has_set;
} RippleSectionDef;

typedef struct {
    RippleSectionDef sections[RIPPLE_MAX_SECTIONS];
    size_t section_count;
} RippleDevice;

typedef struct {
    size_t pins[4];
    size_t count;
    int active_high;
} RippleCtl;

typedef struct {
    const RippleSectionDef *def;
    size_t clock;
    size_t q[RIPPLE_MAX_STAGES];
    RippleCtl reset, set;
} RippleSection;

typedef struct {
    RippleSection sections[RIPPLE_MAX_SECTIONS];
    size_t section_count;
} RippleBound;

typedef struct {
    CounterHeader h;
    const RippleBound *b;
    SeqBits value[RIPPLE_MAX_SECTIONS];
    LogicValue last_clock[RIPPLE_MAX_SECTIONS];
    SeqOutput q[RIPPLE_MAX_SECTIONS][RIPPLE_MAX_STAGES];
} RippleState;

static int ripple_ctl(KindContext *ctx, const cJSON *json, const char *key, int with_value,
                      RippleCtlName *out, int *present) {
    static const char *const KEYS[] = {"pins", "active", NULL};
    static const char *const VALUE_KEYS[] = {"pins", "active", "value", NULL};
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);
    uint64_t value;

    *present = 0;
    if (item == NULL) {
        return 0;
    }
    kind_push(ctx, "%s", key);
    if (kind_check_keys(ctx, item, with_value ? VALUE_KEYS : KEYS, NULL) != 0 ||
        seq_names(ctx, item, "pins", 1, 4, &out->pins, &out->count) != 0 ||
        seq_active(ctx, item, "active", &out->active_high) != 0) {
        return -1;
    }
    if (with_value) {
        if (kind_uint(ctx, item, "value", 63, &value) != 0) {
            return -1;
        }
        out->value = (unsigned)value;
    }
    kind_pop(ctx);
    *present = 1;
    return 0;
}

static void *ripple_load(KindContext *ctx, const cJSON *json, unsigned *units_out) {
    static const char *const SECTION_KEYS[] = {"clock", "structure", "stages", "q", "reset", "set",
                                               NULL};
    RippleDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const cJSON *sections = kind_member(ctx, json, "sections", 1);
    const cJSON *item;
    uint64_t units;
    size_t n = 0;

    if (dev == NULL || sections == NULL || kind_uint(ctx, json, "units", 8, &units) != 0) {
        return NULL;
    }
    if (units < 1) {
        kind_error(ctx, "\"units\" must be 1 to 8");
        return NULL;
    }
    if (!cJSON_IsArray(sections) || cJSON_GetArraySize(sections) < 1 ||
        cJSON_GetArraySize(sections) > RIPPLE_MAX_SECTIONS) {
        kind_error(ctx, "\"sections\" must be an array of 1 to %d sections", RIPPLE_MAX_SECTIONS);
        return NULL;
    }
    cJSON_ArrayForEach(item, sections) {
        RippleSectionDef *sd = &dev->sections[n];
        const char *structure;
        const cJSON *q;
        const cJSON *qi;
        unsigned k = 0;

        kind_push(ctx, "sections[%zu]", n);
        if (kind_check_keys(ctx, item, SECTION_KEYS, NULL) != 0 ||
            kind_string(ctx, item, "clock", &sd->clock) != 0 ||
            kind_string(ctx, item, "structure", &structure) != 0) {
            return NULL;
        }
        if (strcmp(structure, "binary") == 0) {
            uint64_t stages;

            sd->structure = R_BINARY;
            if (kind_uint(ctx, item, "stages", RIPPLE_MAX_STAGES, &stages) != 0) {
                return NULL;
            }
            if (stages < 1) {
                kind_error(ctx, "\"stages\" must be 1 to %d", RIPPLE_MAX_STAGES);
                return NULL;
            }
            sd->stages = (unsigned)stages;
        } else {
            if (strcmp(structure, "div2") == 0) {
                sd->structure = R_DIV2;
                sd->stages = 1;
            } else if (strcmp(structure, "div5") == 0) {
                sd->structure = R_DIV5;
                sd->stages = 3;
            } else if (strcmp(structure, "div6") == 0) {
                sd->structure = R_DIV6;
                sd->stages = 3;
            } else {
                kind_error(ctx, "\"structure\" must be \"binary\", \"div2\", \"div5\" or "
                                "\"div6\", not \"%s\"", structure);
                return NULL;
            }
            if (cJSON_GetObjectItemCaseSensitive(item, "stages") != NULL) {
                kind_error(ctx, "\"stages\" is only for a binary section");
                return NULL;
            }
        }
        q = kind_member(ctx, item, "q", 1);
        if (q == NULL) {
            return NULL;
        }
        if (!cJSON_IsArray(q) || (unsigned)cJSON_GetArraySize(q) != sd->stages) {
            kind_error(ctx, "\"q\" must have %u entries (a pin name or null per stage)",
                       sd->stages);
            return NULL;
        }
        cJSON_ArrayForEach(qi, q) {
            if (cJSON_IsNull(qi)) {
                sd->q[k] = NULL;
            } else if (cJSON_IsString(qi)) {
                sd->q[k] = kind_strdup(ctx, qi->valuestring);
            } else {
                kind_error(ctx, "\"q\" entries must be pin names or null");
                return NULL;
            }
            k++;
        }
        if (ripple_ctl(ctx, item, "reset", 0, &sd->reset, &sd->has_reset) != 0 ||
            ripple_ctl(ctx, item, "set", 1, &sd->set, &sd->has_set) != 0) {
            return NULL;
        }
        if (sd->has_set && sd->set.value >= (1u << sd->stages)) {
            kind_error(ctx, "set: \"value\" does not fit %u stages", sd->stages);
            return NULL;
        }
        kind_pop(ctx);
        n++;
    }
    dev->section_count = n;
    *units_out = (unsigned)units;
    return dev;
}

static int ripple_bind_ctl(KindContext *ctx, const PartPin *pins, size_t count,
                           const RippleCtlName *n, RippleCtl *c) {
    c->count = n->count;
    c->active_high = n->active_high;
    return bind_names(ctx, pins, count, n->pins, n->count, c->pins);
}

static void *ripple_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const RippleDevice *dev = raw;
    RippleBound *b = kind_alloc(ctx, sizeof(*b));
    size_t i;

    if (b == NULL) {
        return NULL;
    }
    for (i = 0; i < dev->section_count; i++) {
        const RippleSectionDef *sd = &dev->sections[i];
        RippleSection *s = &b->sections[i];
        unsigned k;

        kind_push(ctx, "sections[%zu]", i);
        s->def = sd;
        if (seq_bind(ctx, pins, count, sd->clock, &s->clock) != 0 ||
            (sd->has_reset && ripple_bind_ctl(ctx, pins, count, &sd->reset, &s->reset) != 0) ||
            (sd->has_set && ripple_bind_ctl(ctx, pins, count, &sd->set, &s->set) != 0)) {
            return NULL;
        }
        for (k = 0; k < sd->stages; k++) {
            if (seq_bind(ctx, pins, count, sd->q[k], &s->q[k]) != 0) {
                return NULL;
            }
        }
        kind_pop(ctx);
    }
    b->section_count = dev->section_count;
    return b;
}

static CounterHeader *ripple_create(const void *dev, const void *bound) {
    const RippleBound *b = bound;
    RippleState *s = calloc(1, sizeof(*s));
    size_t i;

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    for (i = 0; i < b->section_count; i++) {
        s->value[i] = known_bits(0, b->sections[i].def->stages);
        init_outputs(s->q[i], b->sections[i].q, b->sections[i].def->stages);
    }
    return &s->h;
}

/* A reset/set: asserted (1) when every pin is at its active level; Z reads inactive. */
static int ripple_ctl_level(const Sim *sim, const SimInstance *inst, const RippleCtl *c) {
    int all = 1;
    size_t i;

    for (i = 0; i < c->count; i++) {
        SeqControl one = {c->pins[i], c->active_high, 0};

        all = and3(all, seq_async(sim, inst, &one));
    }
    return all;
}

/* The next state of a section on its clock's falling edge (all bits known). */
static unsigned ripple_next(int structure, unsigned v, unsigned stages) {
    switch (structure) {
        case R_DIV2:
            return v ^ 1u;
        case R_DIV5: {
            /* QB = bit 0, QC = bit 1, QD = bit 2; 0-4, then the JK structure's
               illegal states (counter-b.md § 74_390): 5 -> 2, 6 -> 2, 7 -> 0. */
            static const unsigned char NEXT[8] = {1, 2, 3, 4, 0, 2, 2, 0};

            return NEXT[v & 7u];
        }
        case R_DIV6: {
            /* QB, QC a divide-by-3 (00 -> 01 -> 10 -> 00; 11 -> 00); QD (bit 2)
               toggles when QC falls (counter-b.md § 74_92). */
            unsigned low = v & 3u;
            unsigned next_low = low == 0 ? 1u : low == 1 ? 2u : 0u;
            unsigned qd = (v >> 2) & 1u;

            if ((low & 2u) && !(next_low & 2u)) {
                qd ^= 1u;
            }
            return next_low | (qd << 2);
        }
        default:
            return (v + 1) & (unsigned)mask_of(stages);
    }
}

/* Ripple depth of a stage: binary k; div5 QC 1; div6 QD 1; else 0. */
static unsigned ripple_depth(int structure, unsigned k) {
    switch (structure) {
        case R_BINARY:
            return k;
        case R_DIV5:
            return k == 1;
        case R_DIV6:
            return k == 2;
        default:
            return 0;
    }
}

static SeqBits ripple_clocked(const RippleSectionDef *sd, SeqBits v) {
    unsigned n = 1u << sd->stages;
    SeqBits r = {0, 0};
    int first = 1;
    unsigned x;

    if ((v.known & mask_of(sd->stages)) == mask_of(sd->stages)) {
        return known_bits(ripple_next(sd->structure, (unsigned)v.value, sd->stages), sd->stages);
    }
    if (sd->stages > 4) {
        /* Binary: three-valued +1. */
        return step_binary(v, sd->stages, 1);
    }
    for (x = 0; x < n; x++) {
        if (((x ^ v.value) & v.known) != 0) {
            continue;
        }
        r = first ? known_bits(ripple_next(sd->structure, x, sd->stages), sd->stages)
                  : counter_merge(r, known_bits(ripple_next(sd->structure, x, sd->stages),
                                                sd->stages));
        first = 0;
    }
    return r;
}

static void ripple_eval(Sim *sim, SimInstance *inst, RippleState *s, int at_start) {
    const RippleBound *b = s->b;
    const CounterTiming *t = s->h.timing;
    size_t i;

    for (i = 0; i < b->section_count; i++) {
        const RippleSection *sec = &b->sections[i];
        const RippleSectionDef *sd = sec->def;
        LogicValue clock = sim_read_pin(sim, inst, sec->clock);
        int edge = !at_start && seq_falling(s->last_clock[i], clock);
        int set = sd->has_set ? ripple_ctl_level(sim, inst, &sec->set) : 0;
        int reset = sd->has_reset ? ripple_ctl_level(sim, inst, &sec->reset) : 0;
        SeqBits v = s->value[i];
        SeqBits next = edge ? ripple_clocked(sd, v) : v;
        SeqBits r = {0, 0};
        int first = 1;
        int a, c;
        unsigned k;

        s->last_clock[i] = clock;
        for (a = lo(set); a <= hi(set); a++) {
            for (c = lo(reset); c <= hi(reset); c++) {
                SeqBits o = a ? known_bits(sd->set.value, sd->stages)
                              : c ? known_bits(0, sd->stages) : next;

                r = first ? o : counter_merge(r, o);
                first = 0;
            }
        }
        s->value[i] = r;
        for (k = 0; k < sd->stages; k++) {
            counter_drive(sim, inst, &s->q[i][k], seq_bit(r, k), t,
                          t->extra[0] * ripple_depth(sd->structure, k));
        }
    }
}

static void ripple_start(Sim *sim, SimInstance *inst, CounterHeader *h) {
    RippleState *s = (RippleState *)h;
    size_t i;

    for (i = 0; i < s->b->section_count; i++) {
        s->last_clock[i] = sim_read_pin(sim, inst, s->b->sections[i].clock);
    }
    ripple_eval(sim, inst, s, 1);
}

static void ripple_changed(Sim *sim, SimInstance *inst, CounterHeader *h, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    ripple_eval(sim, inst, (RippleState *)h, 0);
}

static const CounterModel RIPPLE_MODEL = {
    "ripple", RIPPLE_KEYS, RIPPLE_EXTRA, ripple_load, ripple_bind, ripple_create, NULL,
    ripple_start, ripple_changed, NULL
};

/* ---- The kind ------------------------------------------------------------------ */

static const CounterModel *const MODELS[] = {
    &SYNC_MODEL, &RIPPLE_MODEL, &JOHNSON_MODEL, &SEG7_MODEL, &DIVN_MODEL, &RATE_MODEL, NULL
};

/* "model", "timing" and every model's keys (the library checks these first). */
static const char *const DEVICE_KEYS[] = {
    "model", "timing", "units", "byteBus", "sections",
    /* johnson */
    "stages", "clock", "clockGate", "reset", "outputs", "carry", "data", "presetEnable", "jam",
    "qbar",
    /* seg7 */
    "clockUp", "clockDown", "segments", "glyphs", "carryPulse", "borrowPulse", "displayEnable",
    "ungatedC", "lampTest", "rippleBlank", "toggleEnable", "latchEnable",
    /* divn */
    "modes", "output",
    /* rate */
    "variant", "rate", "enable", "strobe", "cascade", "clear", "set", "out", "outBar", "terminal",
    "enableOut",
    NULL
};
static const char *const PART_KEYS[] = {"family", "vcc", "inputMax", NULL};

typedef struct {
    const CounterModel *model;
    void *dev;
    SeqTimingTable timing;
    CounterTiming *timings;   /* parallel to timing.entries */
} CounterDevice;

static void *counter_load_device(KindContext *ctx, const cJSON *json) {
    CounterDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const char *extra_keys[COUNTER_MAX_EXTRA + 1] = {NULL};
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

static void *counter_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                 size_t pin_count) {
    const CounterDevice *dev = raw;

    return dev->model->bind(ctx, dev->dev, pins, pin_count);
}

static int counter_load_part(KindContext *ctx, const void *raw, const void *bound,
                             const cJSON *json, Part *out) {
    const CounterDevice *dev = raw;
    const SeqTiming *entry;

    (void)bound;
    if (seq_part(ctx, &dev->timing, json, out, &entry) != 0) {
        return -1;
    }
    kind_set_part_data(out, &dev->timings[entry - dev->timing.entries]);
    return 0;
}

/* ---- Engine callbacks ------------------------------------------------------------ */

static size_t counter_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *counter_create(const void *config) {
    const PartInternal *internal = part_internal(config);
    const CounterDevice *dev = internal->device;
    CounterHeader *h = dev->model->create(dev->dev, internal->bound);

    if (h != NULL) {
        h->model = dev->model;
        h->timing = internal->data;
    }
    return h;
}

static void counter_destroy(void *state) {
    CounterHeader *h = state;

    if (h != NULL && h->model->destroy != NULL) {
        h->model->destroy(h);
    } else {
        free(state);
    }
}

static void counter_start(Sim *sim, SimInstance *instance, void *state) {
    CounterHeader *h = state;

    h->model->start(sim, instance, h);
}

static void counter_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                                LogicValue old_value, LogicValue new_value) {
    CounterHeader *h = state;

    h->model->changed(sim, instance, h, pin, old_value, new_value);
}

static void counter_timer(Sim *sim, SimInstance *instance, void *state, uint32_t id) {
    CounterHeader *h = state;

    if (h->model->timer != NULL) {
        h->model->timer(sim, instance, h, id);
    }
}

static const SimComponentType COUNTER_SIM_MODEL = {
    .name = "counter",
    .pin_count = counter_pin_count,
    .create = counter_create,
    .destroy = counter_destroy,
    .start = counter_start,
    .pin_changed = counter_pin_changed,
    .timer = counter_timer,
};

/* No counter has open-drain outputs. */
static int counter_pin_open_drain(const Part *part, size_t pin) {
    (void)part;
    (void)pin;
    return 0;
}

const PartKindOps COUNTER_KIND = {
    PART_COUNTER, "counter", &COUNTER_SIM_MODEL, DEVICE_KEYS, PART_KEYS,
    counter_load_device, counter_bind_pinout, counter_load_part, counter_pin_open_drain
};
