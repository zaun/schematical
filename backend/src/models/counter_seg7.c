/* models/counter_seg7.c: part of the model in models/counter_div.c (split by plan-68-I/J). */
#include "counter_div_int.h"
#include "logic_alloc.h"
/* ---- seg7 -------------------------------------------------------------------- */

static const char *const SEG7_KEYS[] = {
    "clock", "clockGate", "clockUp", "clockDown", "reset", "segments", "glyphs", "carry",
    "carryPulse", "borrowPulse", "displayEnable", "ungatedC", "lampTest", "rippleBlank",
    "toggleEnable", "latchEnable", "segmentsActive", "segmentsOutput", "maxCount", "latchOutputs",
    "blank", "rbi", "rbo", "decimalPoint", NULL
};
/* extra[0], [1]: the carry and borrow pulse widths; extra[2], [3]: the maxCount pin's own rise and
   fall delays (ns; 0 = the entry's rise and fall), plan-56-F. */
static const CounterExtraKey SEG7_EXTRA[] = {
    {"carryPulse", 0}, {"borrowPulse", 0}, {"maxRise", 0}, {"maxFall", 0}, {NULL, 0}
};

/* toggleEnable: one control object or an array of them. */
static int seg7_toggles(KindContext *ctx, const cJSON *json, Seg7Device *dev) {
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(json, "toggleEnable");
    const cJSON *e;

    dev->toggle_count = 0;
    if (item == NULL) {
        return 0;
    }
    if (!cJSON_IsArray(item)) {
        if (seq_control_name(ctx, json, "toggleEnable", 0, 0, &dev->toggle[0]) != 0) {
            return -1;
        }
        dev->toggle_count = 1;
        return 0;
    }
    kind_push(ctx, "toggleEnable");
    if (cJSON_GetArraySize(item) < 1 || cJSON_GetArraySize(item) > SEG7_MAX_TOGGLES) {
        return kind_error(ctx, "must have 1 to %d controls", SEG7_MAX_TOGGLES);
    }
    cJSON_ArrayForEach(e, item) {
        kind_push(ctx, "[%zu]", dev->toggle_count);
        if (seq_control_object(ctx, e, 0, &dev->toggle[dev->toggle_count]) != 0) {
            return -1;
        }
        kind_pop(ctx);
        dev->toggle_count++;
    }
    kind_pop(ctx);
    return 0;
}

/* maxCount: {"pin", "enable": control}. */
static int seg7_max_count(KindContext *ctx, const cJSON *json, Seg7Device *dev) {
    static const char *const KEYS[] = {"pin", "enable", NULL};
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(json, "maxCount");

    dev->max_pin = NULL;
    dev->max_enable.pin = NULL;
    if (item == NULL) {
        return 0;
    }
    kind_push(ctx, "maxCount");
    if (kind_check_keys(ctx, item, KEYS, NULL) != 0 || kind_string(ctx, item, "pin", &dev->max_pin) != 0 ||
        seq_control_name(ctx, item, "enable", 1, 0, &dev->max_enable) != 0) {
        return -1;
    }
    kind_pop(ctx);
    return 0;
}

static void *seg7_load(KindContext *ctx, const cJSON *json, unsigned *units) {
    Seg7Device *dev = kind_alloc(ctx, sizeof(*dev));
    const char *const *glyphs;
    size_t glyph_count;
    size_t i, j;

    if (dev == NULL) {
        return NULL;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "clockUp") != NULL) {
        if (kind_string(ctx, json, "clockUp", &dev->clock_up) != 0 ||
            kind_string(ctx, json, "clockDown", &dev->clock_down) != 0) {
            return NULL;
        }
    } else if (kind_string(ctx, json, "clock", &dev->clock) != 0) {
        return NULL;
    }
    if (seq_gate_name(ctx, json, "clockGate", &dev->gate) != 0 ||
        seq_control_name(ctx, json, "reset", 1, 0, &dev->reset) != 0 ||
        seq_control_name(ctx, json, "lampTest", 0, 0, &dev->lamp_test) != 0 ||
        seg7_toggles(ctx, json, dev) != 0 ||
        seq_control_name(ctx, json, "latchEnable", 0, 0, &dev->latch) != 0 ||
        seq_names(ctx, json, "segments", 7, 7, &dev->segments, &dev->segment_count) != 0 ||
        seq_names(ctx, json, "glyphs", 10, 10, &glyphs, &glyph_count) != 0 ||
        seq_optional_name(ctx, json, "carry", &dev->carry) != 0 ||
        seq_optional_name(ctx, json, "carryPulse", &dev->carry_pulse) != 0 ||
        seq_optional_name(ctx, json, "borrowPulse", &dev->borrow_pulse) != 0 ||
        seq_optional_name(ctx, json, "ungatedC", &dev->ungated_c) != 0 ||
        cdv_pin_pair(ctx, json, "displayEnable", &dev->de_in, &dev->de_out) != 0 ||
        cdv_pin_pair(ctx, json, "rippleBlank", &dev->rb_in, &dev->rb_out) != 0 ||
        seg7_max_count(ctx, json, dev) != 0 ||
        seq_control_name(ctx, json, "blank", 0, 0, &dev->blank) != 0 ||
        seq_optional_name(ctx, json, "rbi", &dev->rbi) != 0 ||
        seq_optional_name(ctx, json, "rbo", &dev->rbo) != 0 ||
        cdv_pin_pair(ctx, json, "decimalPoint", &dev->dp_in, &dev->dp_out) != 0) {
        return NULL;
    }
    dev->segments_high = 1;
    dev->segments_style = SEQ_PUSH_PULL;
    if (cJSON_GetObjectItemCaseSensitive(json, "segmentsActive") != NULL &&
        seq_active(ctx, json, "segmentsActive", &dev->segments_high) != 0) {
        return NULL;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "segmentsOutput") != NULL) {
        const char *style;

        if (kind_string(ctx, json, "segmentsOutput", &style) != 0) {
            return NULL;
        }
        if (strcmp(style, "open-drain") == 0) {
            dev->segments_style = SEQ_OPEN_DRAIN;
        } else if (strcmp(style, "push-pull") != 0) {
            kind_error(ctx, "\"segmentsOutput\" must be \"push-pull\" or \"open-drain\"");
            return NULL;
        }
    }
    if (dev->segments_style == SEQ_OPEN_DRAIN && dev->segments_high) {
        kind_error(ctx, "open-drain \"segmentsOutput\" needs \"segmentsActive\" low");
        return NULL;
    }
    if ((dev->rbi != NULL) != (dev->rbo != NULL)) {
        kind_error(ctx, "\"rbi\" and \"rbo\" go together");
        return NULL;
    }
    if (dev->rbi != NULL && dev->rb_in != NULL) {
        kind_error(ctx, "\"rbi\"/\"rbo\" and \"rippleBlank\" are alternatives");
        return NULL;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "latchOutputs") != NULL &&
        seq_names(ctx, json, "latchOutputs", 4, 4, &dev->latch_outputs, &glyph_count) != 0) {
        return NULL;
    }
    for (i = 0; i < 10; i++) {
        if (strlen(glyphs[i]) != 7) {
            kind_error(ctx, "glyphs[%zu]: must be 7 characters 0 or 1 (segments a to g)", i);
            return NULL;
        }
        for (j = 0; j < 7; j++) {
            if (glyphs[i][j] != '0' && glyphs[i][j] != '1') {
                kind_error(ctx, "glyphs[%zu]: must be 7 characters 0 or 1 (segments a to g)", i);
                return NULL;
            }
            if (glyphs[i][j] == '1') {
                dev->glyphs[i] |= (unsigned char)(1u << j);
            }
        }
    }
    *units = 1;
    return dev;
}

static void *seg7_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const Seg7Device *d = raw;
    Seg7Bound *b = kind_alloc(ctx, sizeof(*b));
    size_t i;

    if (b == NULL || seq_bind(ctx, pins, count, d->clock, &b->clock) != 0 ||
        seq_bind(ctx, pins, count, d->clock_up, &b->clock_up) != 0 ||
        seq_bind(ctx, pins, count, d->clock_down, &b->clock_down) != 0 ||
        seq_bind_gate(ctx, pins, count, &d->gate, &b->gate) != 0 ||
        seq_bind_control(ctx, pins, count, &d->reset, &b->reset) != 0 ||
        seq_bind_control(ctx, pins, count, &d->lamp_test, &b->lamp_test) != 0 ||
        seq_bind_control(ctx, pins, count, &d->latch, &b->latch) != 0 ||
        cdv_bind_names(ctx, pins, count, d->segments, 7, b->segments) != 0 ||
        seq_bind(ctx, pins, count, d->carry, &b->carry) != 0 ||
        seq_bind(ctx, pins, count, d->carry_pulse, &b->carry_pulse) != 0 ||
        seq_bind(ctx, pins, count, d->borrow_pulse, &b->borrow_pulse) != 0 ||
        seq_bind(ctx, pins, count, d->de_in, &b->de_in) != 0 ||
        seq_bind(ctx, pins, count, d->de_out, &b->de_out) != 0 ||
        seq_bind(ctx, pins, count, d->ungated_c, &b->ungated_c) != 0 ||
        seq_bind(ctx, pins, count, d->rb_in, &b->rb_in) != 0 ||
        seq_bind(ctx, pins, count, d->rb_out, &b->rb_out) != 0 ||
        seq_bind(ctx, pins, count, d->max_pin, &b->max_pin) != 0 ||
        seq_bind_control(ctx, pins, count, &d->max_enable, &b->max_enable) != 0 ||
        seq_bind_control(ctx, pins, count, &d->blank, &b->blank) != 0 ||
        seq_bind(ctx, pins, count, d->rbi, &b->rbi) != 0 ||
        seq_bind(ctx, pins, count, d->rbo, &b->rbo) != 0 ||
        seq_bind(ctx, pins, count, d->dp_in, &b->dp_in) != 0 ||
        seq_bind(ctx, pins, count, d->dp_out, &b->dp_out) != 0) {
        return NULL;
    }
    for (i = 0; i < d->toggle_count; i++) {
        if (seq_bind_control(ctx, pins, count, &d->toggle[i], &b->toggle[i]) != 0) {
            return NULL;
        }
    }
    if (d->latch_outputs != NULL && cdv_bind_names(ctx, pins, count, d->latch_outputs, 4, b->latch_outputs) != 0) {
        return NULL;
    }
    if (b->rbo != SEQ_NO_PIN && pins[b->rbo].role != PIN_INOUT) {
        kind_error(ctx, "\"rbo\" pin \"%s\" must have role inout", d->rbo);
        return NULL;
    }
    b->dev = d;
    return b;
}

static CounterHeader *seg7_create(const void *dev, const void *bound) {
    const Seg7Bound *b = bound;
    Seg7State *s = logic_calloc(1, sizeof(*s));

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    cdv_init_outputs(s->segments, b->segments, 7);
    cdv_init_outputs(&s->carry, &b->carry, 1);
    cdv_init_outputs(&s->carry_pulse, &b->carry_pulse, 1);
    cdv_init_outputs(&s->borrow_pulse, &b->borrow_pulse, 1);
    cdv_init_outputs(&s->de_out, &b->de_out, 1);
    cdv_init_outputs(&s->ungated_c, &b->ungated_c, 1);
    cdv_init_outputs(&s->rb_out, &b->rb_out, 1);
    cdv_init_outputs(&s->max_out, &b->max_pin, 1);
    cdv_init_outputs(s->latch_out, b->latch_outputs, b->dev->latch_outputs != NULL ? 4 : 0);
    cdv_init_outputs(&s->rbo, &b->rbo, 1);
    cdv_init_outputs(&s->dp_out, &b->dp_out, 1);
    return &s->h;
}

static LogicValue seg7_clock(const Sim *sim, const SimInstance *inst, const Seg7Bound *b) {
    LogicValue clock = sim_read_pin(sim, inst, b->clock);

    if (b->gate.pin != SEQ_NO_PIN) {
        clock = seq_gated_clock(clock, sim_read_pin(sim, inst, b->gate.pin), b->gate.op);
    }
    return clock;
}

/* Segment j of a count (count -1 -> -1 unless every digit agrees). */
static int glyph_level(const Seg7Device *d, int count, unsigned j) {
    int have = 0, v = -1;
    int c;

    if (count >= 0) {
        return (d->glyphs[count] >> j) & 1u;
    }
    for (c = 0; c < 10; c++) {
        int o = (d->glyphs[c] >> j) & 1u;

        v = !have ? o : v == o ? v : -1;
        have = 1;
    }
    return v;
}

/* A one-shot low pulse on o: low now (fall delay), high again width_ps later. */
static void seg7_pulse(Sim *sim, SimInstance *inst, Seg7State *s, SeqOutput *o, uint32_t kind,
                       uint32_t *gen, SimTime width_ps) {
    if (o->pin == SEQ_NO_PIN) {
        return;
    }
    counter_drive(sim, inst, o, LOGIC_0, s->h.timing, 0);
    (*gen)++;
    sim_schedule_timer(sim, inst, (kind << 24) | (*gen & 0xFFFFFFu),
                       s->h.timing->seq->fall + width_ps);
}

/* A control that is optional: no pin -> never asserted. */
static int seg7_control(const Sim *sim, const SimInstance *inst, const SeqControl *c) {
    return c->pin == SEQ_NO_PIN ? 0 : seq_sync(sim, inst, c);
}

static void seg7_outputs(Sim *sim, SimInstance *inst, Seg7State *s) {
    const Seg7Bound *b = s->b;
    const Seg7Device *d = b->dev;
    const CounterTiming *t = s->h.timing;
    int lamp = seq_sync(sim, inst, &b->lamp_test);
    int de = b->de_in != SEQ_NO_PIN ? seq_read(sim, inst, b->de_in) : 1;
    int is_zero = s->shown < 0 ? -1 : s->shown == 0;
    int rbi = b->rb_in != SEQ_NO_PIN ? seq_read(sim, inst, b->rb_in) : 1;
    int blank = cdv_or3(cdv_not3(de), cdv_and3(is_zero, cdv_not3(rbi)));
    int bi = 0, ripple = 0, hard = 0, dp = 0;
    unsigned j;

    if (b->lamp_test.pin == SEQ_NO_PIN) {
        lamp = 0;
    }
    if (b->rbi != SEQ_NO_PIN) {
        /* SN74143: BI high blanks and pulls RBO low; RBI low with the latched 0 blanks and pulls it
           low; an external low on RBO (Z reads high) blanks. The RBO pin drives only the first two
           terms, so it never holds itself low. */
        LogicValue pin = sim_read_pin(sim, inst, b->rbo);
        int external = pin == LOGIC_0 ? 1 : pin == LOGIC_X ? -1 : 0;

        bi = seg7_control(sim, inst, &b->blank);
        ripple = cdv_and3(is_zero, cdv_not3(seq_read(sim, inst, b->rbi)));
        blank = cdv_or3(blank, ripple);
        hard = cdv_or3(bi, external);
    } else if (b->blank.pin != SEQ_NO_PIN) {
        bi = seg7_control(sim, inst, &b->blank);
        hard = bi;
    }
    for (j = 0; j < 7; j++) {
        int seg = cdv_and3(cdv_not3(hard), cdv_or3(lamp, cdv_and3(cdv_not3(blank), glyph_level(d, s->shown, j))));

        seq_drive(sim, inst, &s->segments[j], seq_value(d->segments_high ? seg : cdv_not3(seg)),
                  d->segments_style, t->seq);
    }
    cdv_drive(sim, inst, &s->carry, s->count < 0 ? -1 : s->count <= 4, t);
    cdv_drive(sim, inst, &s->de_out, de, t);
    cdv_drive(sim, inst, &s->ungated_c, glyph_level(d, s->count, 2), t);
    cdv_drive(sim, inst, &s->rb_out, cdv_not3(cdv_and3(s->count < 0 ? -1 : s->count == 0, cdv_not3(rbi))), t);
    if (b->rbo != SEQ_NO_PIN) {
        seq_drive(sim, inst, &s->rbo, seq_value(cdv_not3(cdv_or3(bi, ripple))), SEQ_OPEN_DRAIN, t->seq);
    }
    if (b->dp_out != SEQ_NO_PIN) {
        dp = cdv_and3(seq_read(sim, inst, b->dp_in), cdv_not3(cdv_or3(hard, blank)));
        seq_drive(sim, inst, &s->dp_out, seq_value(d->segments_high ? dp : cdv_not3(dp)),
                  d->segments_style, t->seq);
    }
    if (d->latch_outputs != NULL) {
        for (j = 0; j < 4; j++) {
            cdv_drive(sim, inst, &s->latch_out[j], s->shown < 0 ? -1 : (s->shown >> j) & 1, t);
        }
    }
    if (b->max_pin != SEQ_NO_PIN) {
        int low = cdv_and3(s->count < 0 ? -1 : s->count == 9, seq_sync(sim, inst, &b->max_enable));
        SeqTiming own = *t->seq;

        if (t->extra[2] != 0 || t->extra[3] != 0) {
            own.rise = t->extra[2];
            own.fall = t->extra[3];
        }
        seq_drive(sim, inst, &s->max_out, seq_value(cdv_not3(low)), SEQ_PUSH_PULL, &own);
    }
}

/* 1 when every toggleEnable control is asserted, 0 when one is not, else -1; none -> 1. */
static int seg7_toggle(const Sim *sim, const SimInstance *inst, const Seg7Bound *b) {
    int all = 1;
    size_t i;

    for (i = 0; i < b->dev->toggle_count; i++) {
        all = cdv_and3(all, seq_sync(sim, inst, &b->toggle[i]));
    }
    return all;
}

static int seg7_open_drain(const void *bound, size_t pin) {
    const Seg7Bound *b = bound;
    size_t i;

    if (b->rbo != SEQ_NO_PIN && pin == b->rbo) {
        return 1;
    }
    if (b->dev->segments_style != SEQ_OPEN_DRAIN) {
        return 0;
    }
    for (i = 0; i < 7; i++) {
        if (pin == b->segments[i]) {
            return 1;
        }
    }
    return b->dp_out != SEQ_NO_PIN && pin == b->dp_out;
}

static void seg7_eval(Sim *sim, SimInstance *inst, Seg7State *s, int at_start) {
    const Seg7Bound *b = s->b;
    const Seg7Device *d = b->dev;
    int reset = seq_async(sim, inst, &b->reset);
    int toggle = seg7_toggle(sim, inst, b);
    int step = 0;   /* +1 up, -1 down, 0 none; 2 = unknown */
    int before = s->count;
    int latch;

    if (d->clock_up != NULL) {
        LogicValue up = sim_read_pin(sim, inst, b->clock_up);
        LogicValue down = sim_read_pin(sim, inst, b->clock_down);

        if (!at_start && seq_rising(s->last_up, up)) {
            step += 1;
        }
        if (!at_start && seq_rising(s->last_down, down)) {
            step -= 1;
        }
        s->last_up = up;
        s->last_down = down;
    } else {
        LogicValue clock = seg7_clock(sim, inst, b);

        if (!at_start && seq_rising(s->last_clock, clock)) {
            step = 1;
        }
        s->last_clock = clock;
    }
    if (step != 0 && s->count >= 0) {
        int stepped = (s->count + 10 + step) % 10;

        if (toggle == 1) {
            s->count = stepped;
        } else if (toggle < 0) {
            s->count = -1;
        }
    }
    if (reset == 1) {
        s->count = 0;
    } else if (reset < 0 && s->count != 0) {
        s->count = -1;
    }
    latch = b->latch.pin != SEQ_NO_PIN ? seq_sync(sim, inst, &b->latch) : 0;
    if (latch == 0) {
        s->shown = s->count;
    } else if (latch < 0 && s->shown != s->count) {
        s->shown = -1;
    }
    if (!at_start && reset != 1 && before >= 0 && s->count >= 0) {
        if (before == 9 && s->count == 0 && step > 0) {
            seg7_pulse(sim, inst, s, &s->carry_pulse, SEG7_TIMER_CARRY, &s->carry_gen,
                       s->h.timing->extra[0]);
        } else if (before == 0 && s->count == 9 && step < 0) {
            seg7_pulse(sim, inst, s, &s->borrow_pulse, SEG7_TIMER_BORROW, &s->borrow_gen,
                       s->h.timing->extra[1]);
        }
    }
    if (at_start) {
        counter_drive(sim, inst, &s->carry_pulse, LOGIC_1, s->h.timing, 0);
        counter_drive(sim, inst, &s->borrow_pulse, LOGIC_1, s->h.timing, 0);
    }
    seg7_outputs(sim, inst, s);
}

static void seg7_start(Sim *sim, SimInstance *inst, CounterHeader *h) {
    Seg7State *s = (Seg7State *)h;
    const Seg7Bound *b = s->b;

    s->count = 0;
    s->shown = 0;
    if (b->dev->clock_up != NULL) {
        s->last_up = sim_read_pin(sim, inst, b->clock_up);
        s->last_down = sim_read_pin(sim, inst, b->clock_down);
    } else {
        s->last_clock = seg7_clock(sim, inst, b);
    }
    seg7_eval(sim, inst, s, 1);
}

static void seg7_changed(Sim *sim, SimInstance *inst, CounterHeader *h, size_t pin,
                         LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    seg7_eval(sim, inst, (Seg7State *)h, 0);
}

static void seg7_timer(Sim *sim, SimInstance *inst, CounterHeader *h, uint32_t id) {
    Seg7State *s = (Seg7State *)h;
    SeqTiming now = *s->h.timing->seq;
    CounterTiming immediate = {&now, {0}};
    uint32_t kind = id >> 24;
    uint32_t gen = id & 0xFFFFFFu;

    now.rise = 0;
    now.fall = 0;
    if (kind == SEG7_TIMER_CARRY && gen == (s->carry_gen & 0xFFFFFFu)) {
        counter_drive(sim, inst, &s->carry_pulse, LOGIC_1, &immediate, 0);
    } else if (kind == SEG7_TIMER_BORROW && gen == (s->borrow_gen & 0xFFFFFFu)) {
        counter_drive(sim, inst, &s->borrow_pulse, LOGIC_1, &immediate, 0);
    }
}

const CounterModel SEG7_MODEL = {
    "seg7", SEG7_KEYS, SEG7_EXTRA, seg7_load, seg7_bind, seg7_create, NULL, seg7_start,
    seg7_changed, seg7_timer, seg7_open_drain
};

