#include "shift_int.h"

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

SeqBits shf_zero_bits(unsigned n) {
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

int shf_bind_names(KindContext *ctx, const PartPin *pins, size_t count,
                      const char *const *names, size_t n, size_t *out) {
    size_t i;

    for (i = 0; i < n; i++) {
        if (seq_bind(ctx, pins, count, names[i], &out[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

void shf_init_outputs(SeqOutput *outputs, const size_t *pins, size_t n) {
    size_t i;

    for (i = 0; i < n; i++) {
        outputs[i].pin = pins[i];
        outputs[i].scheduled = 0;
        outputs[i].driven = LOGIC_X;
    }
}

int shf_control_list(KindContext *ctx, const cJSON *json, const char *key, int allow_when,
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

static const char *const SHIFT_KEYS[] = {"units", NULL};

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
            return shf_zero_bits(d->stages);
        }
        if (c < 0) {
            return shift_merge(shf_zero_bits(d->stages), rest);
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
    SeqBits zero = shf_zero_bits(d->stages);
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
        /* Blanking reads every bit as 0, all-on as 1, then the polarity control inverts. */
        int blank = d->st_blank.pin != NULL ? seq_sync(sim, inst, &u->st_blank) : 0;
        int all_on = d->st_all_on.pin != NULL ? seq_sync(sim, inst, &u->st_all_on) : 0;
        int flip = d->st_polarity.pin != NULL ? seq_sync(sim, inst, &u->st_polarity) : 0;

        for (i = 0; i < d->st_output_count; i++) {
            LogicValue v = seq_bit(us->storage, (unsigned)i);

            if (blank != 0 || all_on != 0 || flip != 0) {
                int level = or3(and3(seq_level(v), not3(blank)), all_on);

                v = seq_value(flip < 0 ? -1 : flip ? not3(level) : level);
            }
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
        } else if (d->st_latch.pin == NULL) {
            us->storage = us->stages;   /* no clock, no latch: the outputs follow the stages */
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
                us->storage = shf_zero_bits(d->stages);
            } else if (c < 0) {
                us->storage = shift_merge(us->storage, shf_zero_bits(d->stages));
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
            us->storage = shf_zero_bits(d->stages);
            us->serial_ff[0] = us->serial_ff[1] = LOGIC_0;
        } else if (c < 0) {
            us->storage = shift_merge(us->storage, shf_zero_bits(d->stages));
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

/* The storage's own supply pin (VPP, VBB) and the push-pull outputs it feeds: they
   carry the high voltage, not VCC, so the voltage check skips them. */
static int sh_analog(const void *dev, const void *bound, size_t pin) {
    const ShBound *b = bound;
    size_t i, k;

    (void)dev;
    for (i = 0; i < b->unit_count; i++) {
        const ShUnitDef *d = b->units[i].def;

        if (!d->has_storage || d->st_supply == NULL) {
            continue;
        }
        if (b->units[i].st_supply == pin) {
            return 1;
        }
        for (k = 0; d->st_style != SEQ_OPEN_DRAIN && k < d->st_output_count; k++) {
            if (b->units[i].st_outputs[k] == pin) {
                return 1;
            }
        }
    }
    return 0;
}

/* A unit's storage outputs when its storage has an "outputEnable". */
static int sh_tristate(const void *dev, const void *bound, size_t pin) {
    const ShBound *b = bound;
    size_t i, k;

    (void)dev;
    for (i = 0; i < b->unit_count; i++) {
        const ShUnitDef *d = b->units[i].def;

        for (k = 0; d->has_storage && b->units[i].st_oe.pin != SEQ_NO_PIN && k < d->st_output_count; k++) {
            if (b->units[i].st_outputs[k] == pin) {
                return 1;
            }
        }
    }
    return 0;
}

static const ShiftModel SHIFT_MODEL = {
    "shift", SHIFT_KEYS, NULL, shf_sh_load, shf_sh_bind, shf_sh_create, sh_start, sh_changed, NULL,
    sh_open_drain, sh_analog, sh_tristate
};

/* ---- The kind --------------------------------------------------------------------- */

static const ShiftModel *const MODELS[] = {
    &SHIFT_MODEL, &LS673_MODEL, &TAP64_MODEL, &BUSREG_MODEL, &FIFO_MODEL, &SERIN_MODEL, NULL
};

/* "model", "timing" and every model's keys (the library checks these first). */
static const char *const DEVICE_KEYS[] = {
    "model", "timing", "units",
    /* ls673 */
    "clock", "chipSelect", "readWrite", "mode", "serialIo", "storageOutputs", "storageClear",
    "parallel",
    /* bus-register */
    "a", "b", "aEnable", "direction", "parallelSerial", "asyncSync", "serialIn",
    /* fifo */
    "data", "outputs", "shiftIn", "shiftOut", "inputReady", "outputReady", "reset",
    "outputEnable", "words", "unloadNeedsEnable", "loadWhileHigh",
    /* serial-in */
    "address", "outputSelect", "serialOut",
    NULL
};
static const char *const PART_KEYS[] = {"family", "vcc", "inputMax", NULL};

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

static int shift_pin_analog(const Part *part, size_t pin) {
    const PartInternal *internal = part_internal(part);
    const ShiftDevice *dev = internal->device;

    return dev->model->analog != NULL && dev->model->analog(dev->dev, internal->bound, pin);
}

static int shift_pin_tristate(const Part *part, size_t pin) {
    const PartInternal *internal = part_internal(part);
    const ShiftDevice *dev = internal->device;

    return dev->model->tristate != NULL && dev->model->tristate(dev->dev, internal->bound, pin);
}

const PartKindOps SHIFT_KIND = {
    PART_SHIFT, "shift", &SHIFT_SIM_MODEL, DEVICE_KEYS, PART_KEYS,
    shift_load_device, shift_bind_pinout, shift_load_part, shift_pin_open_drain, shift_pin_analog,
    shift_pin_tristate
};
