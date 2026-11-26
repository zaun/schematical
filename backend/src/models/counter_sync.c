/* models/counter_sync.c: part of the model in models/counter.c (split by plan-68-I/J). */
#include "counter_int.h"

static const char *const SYNC_KEYS[] = {"units", "byteBus", NULL};

/* ---- sync: counting --------------------------------------------------------- */

/* The all-max value: every bit (binary) or every digit 9 (bcd). */
static uint64_t max_value(unsigned stages, int bcd) {
    uint64_t v = 0;
    unsigned d;

    if (!bcd) {
        return cnt_mask_of(stages);
    }
    for (d = 0; d < stages / 4; d++) {
        v |= (uint64_t)9 << (4 * d);
    }
    return v;
}

/* Binary +1 / -1 with three-valued ripple carry. */
SeqBits cnt_step_binary(SeqBits cur, unsigned stages, int up) {
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
        carry = up ? cnt_and3(b, carry) : cnt_and3(cnt_not3(b), carry);
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

    for (k = cnt_lo(bcd); k <= cnt_hi(bcd); k++) {
        SeqBits s = k ? step_bcd(d, e->cur, d->stages, up) : cnt_step_binary(e->cur, d->stages, up);

        r = k == cnt_lo(bcd) ? s : counter_merge(r, s);
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
        all = cnt_and3(all, seq_sync(e->sim, e->inst, &e->u->enables[i]));
    }
    for (i = 0; i < d->any_count; i++) {
        any = cnt_or3(any, seq_sync(e->sim, e->inst, &e->u->any[i]));
    }
    return cnt_and3(all, any);
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

    for (a = cnt_lo(s0); a <= cnt_hi(s0); a++) {
        for (b = cnt_lo(s1); b <= cnt_hi(s1); b++) {
            rows |= 1u << (a + 2 * b);
        }
    }
    return rows;
}

static SeqBits mode_row_result(const SyncEval *e, int action) {
    const SyncUnitDef *d = e->u->def;

    switch (action) {
        case M_CLEAR:
            return d->mode_clear_async ? e->cur : cnt_known_bits(0, d->stages);
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
        r = i < n_clears ? cnt_known_bits(e->u->clears[i].to_max ? max_value(d->stages, d->bcd) : 0,
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
            r = cnt_known_bits(0, d->stages);
        } else {
            const SeqControl *c =
                i < n_clears ? &e->u->clears[i].c : &e->u->loads[i - n_clears].c;

            if (c->clocked) {
                continue;
            }
            level = seq_async(e->sim, e->inst, c);
            r = i < n_clears
                    ? cnt_known_bits(e->u->clears[i].to_max ? max_value(d->stages, d->bcd) : 0,
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
    int at_max = -2, at_min = cnt_bits_equal(v, 0, d->stages);
    int at, k;
    size_t i;

    for (k = cnt_lo(bcd); k <= cnt_hi(bcd); k++) {
        int m = cnt_bits_equal(v, max_value(d->stages, k), d->stages);

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
        at = cnt_and3(at, seq_sync(e->sim, e->inst, &c->gates[i]));
    }
    if (c->clock_low != SEQ_NO_PIN) {
        at = cnt_and3(at, cnt_not3(seq_read(e->sim, e->inst, c->clock_low)));
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
        int pin_level = u->carries[i].c.active_high ? level : cnt_not3(level);

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
            enable = cnt_and3(enable, seq_enable(sim, inst, &u->bus_oe[i]));
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
    int is_zero = cnt_bits_equal(us->count, 0, u->def->stages);
    int load = seq_async(e->sim, e->inst, &u->loads[0].c);
    int ck = seq_level(effective_clock(e->sim, e->inst, u));
    int set = cnt_and3(is_zero, seq_read(e->sim, e->inst, u->zero_cascade));
    int reset = cnt_or3(cnt_and3(load, cnt_not3(ck)), cnt_and3(cnt_not3(is_zero), cnt_not3(load)));
    LogicValue z = LOGIC_X;
    int have = 0;
    int r, sv;

    for (r = cnt_lo(reset); r <= cnt_hi(reset); r++) {
        for (sv = cnt_lo(set); sv <= cnt_hi(set); sv++) {
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

/* A unit's q pins when it has an "outputEnable", its register's outputs when the register has one. */
static int sync_tristate(const void *bound, size_t pin) {
    const SyncBound *b = bound;
    size_t i, k;

    for (i = 0; i < b->dev->unit_count; i++) {
        const SyncUnit *u = &b->units[i];
        const SyncUnitDef *d = u->def;

        for (k = 0; u->oe.pin != SEQ_NO_PIN && k < d->q_count; k++) {
            if (u->q[k] == pin) {
                return 1;
            }
        }
        for (k = 0; d->has_register && u->reg_oe.pin != SEQ_NO_PIN && k < d->reg_output_count; k++) {
            if (u->reg_outputs[k] == pin) {
                return 1;
            }
        }
    }
    return 0;
}

const CounterModel cnt_SYNC_MODEL = {
    "sync", SYNC_KEYS, NULL, cnt_sync_load, cnt_sync_bind, cnt_sync_create, NULL, sync_start, sync_changed,
    NULL, NULL, sync_tristate
};

