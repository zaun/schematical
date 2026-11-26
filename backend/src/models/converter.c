#include "converter_core_int.h"

/* ---- Small helpers ----------------------------------------------------------------------- */

static uint32_t mask_of(unsigned bits) {
    return bits >= 32 ? 0xFFFFFFFFu : ((1u << bits) - 1u);
}

size_t cv_pin(const CvState *s, int name) {
    return name < 0 ? SIZE_MAX : s->b->pin[name];
}

LogicValue cv_level(const CvState *s, size_t pin, LogicValue z_reads) {
    LogicValue v;

    if (pin == SIZE_MAX) {
        return z_reads;
    }
    v = sim_read_pin(s->sim, s->inst, pin);
    return v == LOGIC_Z ? z_reads : v;
}

void cv_drive(CvState *s, size_t pin, LogicValue v, SimTime delay) {
    if (pin == SIZE_MAX) {
        return;
    }
    if (v == LOGIC_1 && s->b->open_drain[pin]) {
        v = LOGIC_Z;
    }
    if (s->drv[pin] == v) {
        return;
    }
    s->drv[pin] = v;
    sim_schedule_inertial_drive(s->sim, s->inst, pin, v, DRIVE_STRONG, delay);
}

uint32_t cv_field_get(const CvState *s, const CvField *f) {
    uint32_t v;

    if (f->reg == CV_REG_R) {
        v = cv_code(s);
    } else if (f->reg == CV_REG_CH) {
        v = (uint32_t)s->out_channel;
    } else {
        v = s->regs[f->reg];
    }
    return (v >> f->lo) & mask_of(f->hi - f->lo + 1);
}

/* Sets a field directly (internal: no read-only mask, no triggers). */
void cvc_field_put(CvState *s, const CvField *f, uint32_t value) {
    uint32_t m;

    if (f->reg < 0) {
        return;
    }
    m = mask_of(f->hi - f->lo + 1) << f->lo;
    s->regs[f->reg] = (s->regs[f->reg] & ~m) | ((value << f->lo) & m);
}

int cv_conds_hold(const CvState *s, const CvConds *c) {
    size_t i;

    for (i = 0; i < c->count; i++) {
        uint32_t v = cv_field_get(s, &c->items[i].field);

        if ((v == c->items[i].value) == c->items[i].negate) {
            return 0;
        }
    }
    return 1;
}

static uint32_t value_of(const CvValue *v, const uint32_t *caps) {
    return v->cap ? caps[v->cap - 1] : v->value;
}

uint32_t cv_choice(const CvState *s, const CvChoice *c) {
    uint32_t key;
    size_t i;

    if (c->pin != CV_NONE) {
        key = cv_level(s, cv_pin(s, c->pin), LOGIC_0) == LOGIC_1;
    } else if (c->has_field) {
        key = cv_field_get(s, &c->field);
    } else {
        return c->fixed;
    }
    for (i = 0; i < c->count; i++) {
        if (c->keys[i] == key) {
            return c->values[i];
        }
    }
    return c->fixed;
}

uint32_t cv_code(const CvState *s) {
    const CvDevice *d = s->dev;

    if (d->dac || d->rtc || d->bits == 0) {
        return 0;
    }
    return cv_choice(s, &d->code) == CVF_OFFSET ? (1u << (d->bits - 1)) : 0;
}

int cvc_reg_at(const CvState *s, int space, uint32_t address) {
    size_t i;

    for (i = 0; i < s->dev->reg_count; i++) {
        if (s->dev->regs[i].space == space && s->dev->regs[i].address == (int32_t)address) {
            return (int)i;
        }
    }
    /* a flat map: an address with no register in the default space may be in another
       space (an RTC's battery-backed RAM) */
    for (i = 0; space == 0 && i < s->dev->reg_count; i++) {
        if (s->dev->regs[i].address == (int32_t)address) {
            return (int)i;
        }
    }
    return CV_NONE;
}

int cv_reg_at(const CvState *s, int space, uint32_t address) {
    return cvc_reg_at(s, space, address);
}

/* The next register after `reg` in address order within its space (CV_NONE: none). */
int cvc_reg_next(const CvState *s, int reg) {
    const CvRegister *r = &s->dev->regs[reg];
    int best = CV_NONE;
    size_t i;

    for (i = 0; i < s->dev->reg_count; i++) {
        const CvRegister *o = &s->dev->regs[i];

        if (o->space == r->space && o->address > r->address &&
            (best == CV_NONE || o->address < s->dev->regs[best].address)) {
            best = (int)i;
        }
    }
    return best;
}

/* ---- Ready pins --------------------------------------------------------------------------- */

/* The level a ready pin shows now (LOGIC_Z: inactive open drain). */
LogicValue cv_ready_level(const CvState *s, int pin_name) {
    size_t i;
    int found = 0, active_level = 0, open_drain = 0;

    for (i = 0; i < s->dev->ready_count; i++) {
        const CvReady *r = &s->dev->ready[i];

        if (r->pin != pin_name) {
            continue;
        }
        found = 1;
        active_level = r->active;
        open_drain |= r->open_drain;
        if (cv_conds_hold(s, &r->enable) &&
            (r->asserts_done ? s->ready_on[i] : s->converting)) {
            return r->active ? LOGIC_1 : LOGIC_0;
        }
    }
    if (!found) {
        return LOGIC_Z;
    }
    if (open_drain) {
        return LOGIC_Z;
    }
    return active_level ? LOGIC_0 : LOGIC_1;
}

void cv_ready_refresh(CvState *s) {
    size_t i;

    for (i = 0; i < s->dev->ready_count; i++) {
        const CvReady *r = &s->dev->ready[i];

        if (r->on_miso) {
            continue;
        }
        cv_drive(s, cv_pin(s, r->pin), cv_ready_level(s, r->pin), 0);
    }
    cv_spi_refresh(s);
    if (s->dev->rtc) {
        rtc_refresh(s);
    }
    if (s->dev->disp != NULL) {
        disp_refresh(s);
    }
}

void cvc_ready_clear(CvState *s, unsigned why) {
    size_t i;

    for (i = 0; i < s->dev->ready_count; i++) {
        if (s->dev->ready[i].asserts_done && (s->dev->ready[i].clears & why)) {
            s->ready_on[i] = 0;
        }
    }
}

/* ---- Conversions ------------------------------------------------------------------------ */

static uint64_t conv_time(const CvState *s) {
    const CvTime *t = &s->dev->conversion.time;
    size_t i, k;

    if (t->count == 0) {
        return t->fixed_ps;
    }
    for (i = 0; i < t->count; i++) {
        for (k = 0; k < t->field_count; k++) {
            if (cv_field_get(s, &t->fields[k]) != t->keys[i * t->field_count + k]) {
                break;
            }
        }
        if (k == t->field_count) {
            return t->ps[i];
        }
    }
    return t->max_ps;
}

int cvc_continuous(const CvState *s) {
    const CvConversion *c = &s->dev->conversion;

    if (c->mode == 1) {
        return 1;
    }
    if (c->mode == 2) {
        uint32_t v = cv_field_get(s, &c->mode_field);

        return c->mode_single ? v != c->mode_value : v == c->mode_value;
    }
    if (s->dev->pins.has[CVP_START] && s->dev->pins.ctl[CVP_START].edge < 0 && s->start_level) {
        return 1;
    }
    return 0;
}

void cv_start_conversion(CvState *s, int channel) {
    const CvConversion *c = &s->dev->conversion;
    size_t i;

    if (!c->present || s->powered_down || s->in_reset) {
        return;
    }
    if (channel < 0) {
        channel = c->has_channel ? (int)cv_field_get(s, &c->channel) : s->conv_channel;
    }
    s->conv_channel = channel;
    s->converting = 1;
    s->conv_gen++;
    for (i = 0; i < c->busy_count; i++) {
        cvc_field_put(s, &c->busy[i].field, c->busy[i].value.value);
    }
    cvc_ready_clear(s, CVC_START);
    cv_ready_refresh(s);
    sim_schedule_timer(s->sim, s->inst, CV_TM_CONV | (s->conv_gen & CV_TM_GEN), conv_time(s));
}

void cvc_conv_stop(CvState *s) {
    s->converting = 0;
    s->conv_gen++;
    cv_ready_refresh(s);
}

void cvc_conv_done(CvState *s) {
    const CvConversion *c = &s->dev->conversion;
    uint32_t code = cv_code(s);
    size_t i;

    s->converting = 0;
    for (i = 0; i < s->dev->reg_count; i++) {
        const CvRegister *r = &s->dev->regs[i];
        uint32_t v = code;

        if (!r->result || (r->result_channel != CV_NONE && r->result_channel != s->conv_channel)) {
            continue;
        }
        if (r->slice) {
            v = (code >> r->slice_lo) & mask_of(r->slice_hi - r->slice_lo + 1);
        }
        if (r->result == 1) {
            s->regs[i] = v & mask_of(r->width);
        } else {
            uint32_t m = mask_of(r->result_hi - r->result_lo + 1) << r->result_lo;

            s->regs[i] = (s->regs[i] & ~m) | ((v << r->result_lo) & m);
        }
    }
    for (i = 0; i < c->done_count; i++) {
        cvc_field_put(s, &c->done[i].field, c->done[i].value.value);
    }
    for (i = 0; i < s->dev->ready_count; i++) {
        const CvReady *r = &s->dev->ready[i];

        if (r->asserts_done) {
            s->ready_on[i] = 1;
            s->ready_gen[i]++;
            if (r->clears & CVC_PULSE) {
                sim_schedule_timer(s->sim, s->inst,
                                   CV_TM_READY | ((uint32_t)i << 16) | (s->ready_gen[i] & 0xFFFFu),
                                   r->pulse_ps);
            }
        }
    }
    s->result_fresh = 1;
    cv_spi_conversion_done(s);
    cv_ready_refresh(s);
    if (cvc_continuous(s) && conv_time(s) > 0) {
        cv_start_conversion(s, s->conv_channel);
    }
}

/* The result was read (an R bit or a result register went out). */
void cv_result_read(CvState *s) {
    const CvConversion *c = &s->dev->conversion;
    size_t i;

    if (s->read_noted) {
        return;
    }
    s->read_noted = 1;
    for (i = 0; i < c->on_read_count; i++) {
        cvc_field_put(s, &c->on_read[i].field, c->on_read[i].value.value);
    }
    cvc_ready_clear(s, CVC_READ);
    cv_ready_refresh(s);
    if (c->starts & CVS_READ) {
        cv_start_conversion(s, -1);
    }
}

/* ---- DAC registers ------------------------------------------------------------------------ */

void cvc_dac_update(CvState *s, int ch) {
    const CvDac *d = &s->dev->dacinfo;
    size_t i;

    for (i = 0; i < d->count; i++) {
        if (ch < 0 || (size_t)ch == i) {
            s->regs[d->output[i]] = s->regs[d->input[i]];
        }
    }
}

void cvc_dac_clear(CvState *s) {
    const CvDac *d = &s->dev->dacinfo;
    uint32_t code = cv_choice(s, &d->clear_code);
    size_t i;

    for (i = 0; i < d->count; i++) {
        s->regs[d->input[i]] = code & mask_of(s->dev->regs[d->input[i]].width);
        s->regs[d->output[i]] = code & mask_of(s->dev->regs[d->output[i]].width);
    }
}

static int dac_input_channel(const CvState *s, int reg) {
    const CvDac *d = &s->dev->dacinfo;
    size_t i;

    for (i = 0; s->dev->dac && i < d->count; i++) {
        if (d->input[i] == reg) {
            return (int)i;
        }
    }
    return CV_NONE;
}

static int ldac_level_active(const CvState *s) {
    const CvPinCtl *c = &s->dev->pins.ctl[CVP_LDAC];

    return s->dev->pins.has[CVP_LDAC] && c->edge < 0 &&
           cv_level(s, cv_pin(s, c->pin), c->active ? LOGIC_0 : LOGIC_1) ==
               (c->active ? LOGIC_1 : LOGIC_0);
}

static void after_write(CvState *s, int reg, uint32_t raw, uint32_t old_mode_cont) {
    const CvConversion *c = &s->dev->conversion;
    const CvProtocol *p = &s->dev->protocol;
    size_t i;
    int ch;

    for (i = 0; i < c->write_count; i++) {
        const CvWriteTrigger *w = &c->writes[i];

        if (w->reg != reg) {
            continue;
        }
        if (w->any) {
            cv_start_conversion(s, -1);
        } else {
            const CvField *f = &w->cond.field;
            uint32_t v = (raw >> f->lo) & mask_of(f->hi - f->lo + 1);

            if ((v == w->cond.value) != w->cond.negate) {
                cv_start_conversion(s, -1);
            }
        }
    }
    if (c->present && c->mode == 2 && c->mode_field.reg == reg && !old_mode_cont &&
        cvc_continuous(s) && c->start_on_enter) {
        cv_start_conversion(s, -1);
    }
    if (c->present && c->mode == 2 && c->mode_field.reg == reg && old_mode_cont && !cvc_continuous(s)) {
        /* leaving continuous mode: the conversion in progress finishes */
    }
    for (i = 0; i < p->trigger_count; i++) {
        if (p->triggers[i].cond.field.reg == reg && cv_conds_hold(s, &(CvConds){&p->triggers[i].cond, 1})) {
            cvc_run_action(s, p->triggers[i].action, NULL, NULL);
        }
    }
    ch = dac_input_channel(s, reg);
    if (ch != CV_NONE && (s->dev->dacinfo.ldac == 2 || ldac_level_active(s))) {
        cvc_dac_update(s, ch);
    }
}

/* A bus write: read-only bits kept, write-1-to-clear bits cleared by a 1. */
void cv_reg_write(CvState *s, int reg, uint32_t value) {
    const CvRegister *r = &s->dev->regs[reg];
    uint32_t m = mask_of(r->width);
    uint32_t old = s->regs[reg];
    uint32_t nv;
    uint32_t cont = cvc_continuous(s);

    value &= m;
    nv = (old & r->ro) | (value & ~r->ro);
    if (r->w1c) {
        nv = (nv & ~r->w1c) | (old & r->w1c & ~value);
    }
    s->regs[reg] = nv & m;
    after_write(s, reg, value, cont);
    if (s->dev->rtc) {
        rtc_after_write(s, reg);
    }
    cv_ready_refresh(s);
}

void cvc_mark_read(CvState *s, int reg) {
    if (reg >= 0 && !s->read_mark[reg]) {
        s->read_mark[reg] = 1;
        s->read_marked = 1;
        if (s->dev->regs[reg].result) {
            cv_result_read(s);
        }
    }
}

void cv_reads_done(CvState *s) {
    size_t i;

    if (!s->read_marked) {
        return;
    }
    for (i = 0; i < s->dev->reg_count; i++) {
        if (s->read_mark[i]) {
            s->read_mark[i] = 0;
            if (s->dev->regs[i].cor) {
                s->regs[i] &= ~s->dev->regs[i].cor;
            }
        }
    }
    s->read_marked = 0;
    cv_ready_refresh(s);
}

/* A bus read of a register: clear-on-read bits; a result register counts as reading the result. */
void cv_reg_read(CvState *s, int reg) {
    const CvRegister *r = &s->dev->regs[reg];

    if (r->cor) {
        s->regs[reg] &= ~r->cor;
    }
    if (r->result) {
        cv_result_read(s);
    }
}

/* ---- Reset ------------------------------------------------------------------------------------ */

static void apply_levels(CvState *s) {
    const CvPins *p = &s->dev->pins;
    size_t i;

    for (i = 0; i < p->level_count; i++) {
        cvc_field_put(s, &p->levels[i].field,
                  cv_level(s, cv_pin(s, p->levels[i].pin), LOGIC_0) == LOGIC_1);
    }
}

void cv_reset(CvState *s) {
    const CvDevice *d = s->dev;
    size_t i, k;

    for (i = 0; i < d->reg_count; i++) {
        s->regs[i] = d->regs[i].reset;
    }
    for (i = 0; i < s->pd->reset_count; i++) {
        s->regs[s->pd->resets[i].reg] = s->pd->resets[i].value;
    }
    for (i = 0; i < d->pins.reset_select_count; i++) {
        const CvResetSelect *rs = &d->pins.reset_select[i];

        if ((cv_level(s, cv_pin(s, rs->pin), LOGIC_0) == LOGIC_1) == (rs->level == 1)) {
            for (k = 0; k < rs->count; k++) {
                s->regs[rs->values[k].reg] = rs->values[k].value;
            }
        }
    }
    apply_levels(s);
    s->converting = 0;
    s->conv_gen++;
    s->conv_channel = 0;
    s->powered_down = 0;
    s->pointer = 0;
    s->pointer_space = 0;
    s->pointer_inc = 0;
    s->has_next = 0;
    s->read_reg = CV_NONE;
    for (i = 0; i < d->ready_count; i++) {
        s->ready_on[i] = 0;
    }
    if (d->conversion.present && !s->in_reset &&
        ((d->conversion.starts & CVS_POWER_ON) || (cvc_continuous(s) && d->conversion.start_on_enter))) {
        cv_start_conversion(s, -1);
    }
}

/* ---- Actions --------------------------------------------------------------------------------- */

static void channels_of(const CvCommand *cmd, const uint32_t *caps, int *out, size_t *count,
                        int none) {
    uint32_t v;
    size_t i;

    *count = 0;
    if (cmd == NULL || !cmd->has_channel) {
        out[(*count)++] = none;
        return;
    }
    v = value_of(&cmd->channel, caps);
    for (i = 0; i < cmd->map_count; i++) {
        if (cmd->map[i].key == v) {
            size_t k;

            for (k = 0; k < cmd->map[i].count && *count < 256; k++) {
                out[(*count)++] = cmd->map[i].channels[k];
            }
            return;
        }
    }
    out[(*count)++] = (int)v;
}

static int target_reg(const CvState *s, const CvCommand *cmd, const uint32_t *caps) {
    if (cmd->reg != CV_NONE) {
        return cmd->reg;
    }
    if (cmd->has_address) {
        return cvc_reg_at(s, cmd->space, value_of(&cmd->address, caps));
    }
    return CV_NONE;
}

void cvc_set_out(CvState *s, CvSeg *seg, const CvPattern *pat, const uint32_t *caps,
                    const int *at, size_t at_count, unsigned at_word, int sdio) {
    memset(seg, 0, sizeof *seg);
    seg->pat = pat;
    if (caps != NULL) {
        memcpy(seg->caps, caps, sizeof seg->caps);
    }
    seg->at_count = at_count < CV_MAX_AT ? at_count : CV_MAX_AT;
    if (at_count) {
        memcpy(seg->at, at, seg->at_count * sizeof(*at));
    }
    seg->at_word = at_word;
    seg->sdio = sdio;
    seg->active = 1;
}

/* Registers `count` from register `first` up, in address order. */
size_t cvc_reg_run(const CvState *s, int first, size_t count, int *out) {
    size_t n = 0;
    int r = first;

    while (r != CV_NONE && n < count && n < CV_MAX_AT) {
        out[n++] = r;
        r = cvc_reg_next(s, r);
    }
    return n;
}

void cvc_run_action(CvState *s, uint8_t action, const CvCommand *cmd, const uint32_t *caps) {
    static const uint32_t ZERO[CV_CAPS];
    int chans[256];
    size_t nch = 0, i;
    int reg;

    if (caps == NULL) {
        caps = ZERO;
    }
    switch (action) {
    case CVA_NOP:
        break;
    case CVA_WREG: {
        size_t count = (size_t)value_of(&cmd->count, caps) + (size_t)cmd->count_offset;

        reg = target_reg(s, cmd, caps);
        if (reg == CV_NONE || count == 0) {
            break;
        }
        s->x.payload = CV_PAY_WREG;
        s->x.pay_count = cvc_reg_run(s, reg, count, s->x.pay_regs);
        s->x.pay_index = 0;
        s->x.pay_bits = 0;
        s->x.pay_value = 0;
        s->x.pay_word = cmd->word_bits;
        s->x.pay_cmd = cmd;
        break;
    }
    case CVA_RREG: {
        size_t count = (size_t)value_of(&cmd->count, caps) + (size_t)cmd->count_offset;
        int list[CV_MAX_AT];
        size_t n;

        reg = target_reg(s, cmd, caps);
        n = reg == CV_NONE ? 0 : cvc_reg_run(s, reg, count, list);
        s->at_count = n;
        memcpy(s->at, list, n * sizeof(*list));
        s->at_word = cmd->word_bits;
        if (!cmd->has_reply) {
            static const CvTok AT = {CVT_AT, 0, 0, 0};
            static const CvPattern AT_PATTERN = {&AT, 1, 0, 1};

            cvc_set_out(s, &s->seg, &AT_PATTERN, caps, list, n, cmd->word_bits, 1);
            s->seg_from = 1;
        }
        break;
    }
    case CVA_WRITE: {
        uint32_t data = cmd != NULL && cmd->has_data ? value_of(&cmd->data, caps) : 0;

        if (s->dev->dac && cmd != NULL && cmd->has_channel) {
            channels_of(cmd, caps, chans, &nch, 0);
            for (i = 0; i < nch; i++) {
                if (chans[i] >= 0 && (size_t)chans[i] < s->dev->dacinfo.count) {
                    cv_reg_write(s, s->dev->dacinfo.input[chans[i]], data);
                }
            }
        } else if (cmd != NULL && (reg = target_reg(s, cmd, caps)) != CV_NONE) {
            cv_reg_write(s, reg, data);
        }
        break;
    }
    case CVA_READ:
        reg = target_reg(s, cmd, caps);
        s->read_reg = reg;
        if (reg != CV_NONE) {
            s->pointer = (uint32_t)s->dev->regs[reg].address;
            s->pointer_space = s->dev->regs[reg].space;
            s->at[0] = reg;
            s->at_count = 1;
            s->at_word = cmd->word_bits;
        }
        if (cmd->has_increment) {
            s->pointer_inc = value_of(&cmd->increment, caps) != 0;
        }
        break;
    case CVA_POINTER:
        s->pointer = cmd->has_address ? value_of(&cmd->address, caps) :
                     (uint32_t)s->dev->regs[cmd->reg].address;
        s->pointer_space = cmd->space;
        s->pointer_inc = cmd->has_increment && value_of(&cmd->increment, caps) != 0;
        s->x.payload = CV_PAY_POINTER;
        s->x.pay_bits = 0;
        s->x.pay_value = 0;
        s->x.pay_cmd = cmd;
        memcpy(s->x.pay_caps, caps, sizeof s->x.pay_caps);
        break;
    case CVA_START:
        channels_of(cmd, caps, chans, &nch, -1);
        cv_start_conversion(s, chans[0]);
        break;
    case CVA_STOP:
        cvc_conv_stop(s);
        break;
    case CVA_RESET:
        cv_reset(s);
        break;
    case CVA_POWERDOWN:
        s->powered_down = 1;
        break;
    case CVA_WAKEUP:
        s->powered_down = 0;
        break;
    case CVA_UPDATE:
        channels_of(cmd, caps, chans, &nch, -1);
        for (i = 0; i < nch; i++) {
            cvc_dac_update(s, chans[i]);
        }
        break;
    case CVA_UPDATEALL:
        cvc_dac_update(s, -1);
        break;
    case CVA_CLEAR:
        cvc_dac_clear(s);
        break;
    case CVA_LCDLOAD:
        s->x.payload = CV_PAY_LCD;
        s->x.pay_bits = 0;
        s->x.pay_value = 0;
        break;
    case CVA_SETBITS:
    case CVA_CLEARBITS: {
        uint32_t data = value_of(&cmd->data, caps);

        reg = target_reg(s, cmd, caps);
        if (reg != CV_NONE) {
            cv_reg_write(s, reg, action == CVA_SETBITS ? (s->regs[reg] | data) :
                                                          (s->regs[reg] & ~data));
        }
        break;
    }
    default:
        break;
    }
}

/* The command's effects (after its `in` bits, or at the transaction end for commit end). */
void cvc_execute(CvState *s, const CvCommand *cmd, const uint32_t *caps) {
    size_t i;

    for (i = 0; i < cmd->set_count; i++) {
        cvc_field_put(s, &cmd->sets[i].field, value_of(&cmd->sets[i].value, caps));
    }
    cvc_run_action(s, cmd->action, cmd, caps);
    if (cmd->action != CVA_POINTER) {
        for (i = 0; i < cmd->then_count; i++) {
            cvc_run_action(s, cmd->then[i], cmd, caps);
        }
    }
    if (cmd->has_next) {
        cvc_set_out(s, &s->next, &cmd->next, caps, s->at, s->at_count, s->at_word, 1);
        s->has_next = 1;
    }
    if (cmd->action == CVA_START || (cmd->then_count && cmd->then[0] == CVA_START)) {
        /* "command" start trigger: the action is the trigger */
    }
    cv_ready_refresh(s);
}

/* ---- The kind -------------------------------------------------------------------------------- */

static const char *const DEVICE_KEYS[] = {
    "type", "bits", "channels", "codeFormat", "vcc", "inputMax", "analog", "bus", "configBus",
    "timing", "registers", "protocol", "conversion", "ready", "pins", "dac", NULL
};

static const char *const PART_KEYS[] = {"vcc", "inputMax", "i2cAddress", "i2c", "resets", NULL};

static void *conv_load_device(KindContext *ctx, const cJSON *json) {
    return cv_load_device(ctx, json, CV_MODE_CONVERTER);
}

int cv_pin_open_drain(const Part *part, size_t pin) {
    const CvBound *b = part_internal(part)->bound;

    return pin < b->pin_count && b->open_drain[pin];
}

int cv_pin_tristate(const Part *part, size_t pin) {
    const CvBound *b = part_internal(part)->bound;

    return pin < b->pin_count && b->tristate[pin];
}

int cv_pin_analog(const Part *part, size_t pin) {
    const CvBound *b = part_internal(part)->bound;

    return pin < b->pin_count && b->analog[pin];
}

const PartKindOps CONVERTER_KIND = {
    PART_CONVERTER, "converter", &CONVERTER_COMPONENT, DEVICE_KEYS, PART_KEYS,
    conv_load_device, cv_bind_pinout, cv_load_part, cv_pin_open_drain, cv_pin_analog, cv_pin_tristate
};
