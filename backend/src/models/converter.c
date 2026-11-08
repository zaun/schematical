/*
 * The converter kind (plan-24-D): runs a CvDevice (converter.h,
 * planning/plan-24-data/schema.md) on the engine.
 *
 *   Protocol engine   bits in from the bus are matched against the
 *                     device's command patterns (fixed or stream frames,
 *                     end-aligned decoding, commit at the end); bits out are
 *                     shifted from the output pattern, a command's reply, or
 *                     the pending `next` pattern.
 *   SPI front end     CS (optional, either polarity), SCLK in modes 0-3, MOSI
 *                     and MISO (either optional, or one SDIO pin), MISO
 *                     doubling as a ready pin. Bit level, so frames of any
 *                     length work. CS-less 2-wire parts frame on conversions.
 *   I2C front end     the plan-12 SerialI2c target, unchanged: the device
 *                     claims its address (strap table, a part's fixed
 *                     address, the general call), write bytes feed the
 *                     engine, reads shift its output.
 *   Conversions       start triggers, timing by setting, single/continuous,
 *                     result registers, busy/done fields, ready pins.
 *   DAC registers     input -> DAC copies on LDAC (pin, command or sync),
 *                     clear codes.
 *   Control pins      reset, convst, start, powerdown, ldac, clear, level
 *                     copies, strap-selected reset values.
 *
 * Signal rules: an X or Z on a data input arrives as an unknown bit, which
 * makes the rest of the transaction X (nothing is executed, MISO drives X).
 * CS and control pins at Z read inactive. The result code is the 0 V code
 * (plan-24-A §3.3). Parallel and clocked buses: converter_par.c (plan-24-E).
 */

#include "converter.h"

#include <stdlib.h>
#include <string.h>

#include "converter_int.h"
#include "model_pins.h"

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
static void field_put(CvState *s, const CvField *f, uint32_t value) {
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

static int reg_at(const CvState *s, int space, uint32_t address) {
    size_t i;

    for (i = 0; i < s->dev->reg_count; i++) {
        if (s->dev->regs[i].space == space && s->dev->regs[i].address == (int32_t)address) {
            return (int)i;
        }
    }
    return CV_NONE;
}

/* The next register after `reg` in address order within its space (CV_NONE: none). */
static int reg_next(const CvState *s, int reg) {
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
}

static void ready_clear(CvState *s, unsigned why) {
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

static int continuous(const CvState *s) {
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
        field_put(s, &c->busy[i].field, c->busy[i].value.value);
    }
    ready_clear(s, CVC_START);
    cv_ready_refresh(s);
    sim_schedule_timer(s->sim, s->inst, CV_TM_CONV | (s->conv_gen & CV_TM_GEN), conv_time(s));
}

static void conv_stop(CvState *s) {
    s->converting = 0;
    s->conv_gen++;
    cv_ready_refresh(s);
}

static void conv_done(CvState *s) {
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
        field_put(s, &c->done[i].field, c->done[i].value.value);
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
    if (continuous(s) && conv_time(s) > 0) {
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
        field_put(s, &c->on_read[i].field, c->on_read[i].value.value);
    }
    ready_clear(s, CVC_READ);
    cv_ready_refresh(s);
    if (c->starts & CVS_READ) {
        cv_start_conversion(s, -1);
    }
}

/* ---- DAC registers ------------------------------------------------------------------------ */

static void dac_update(CvState *s, int ch) {
    const CvDac *d = &s->dev->dacinfo;
    size_t i;

    for (i = 0; i < d->count; i++) {
        if (ch < 0 || (size_t)ch == i) {
            s->regs[d->output[i]] = s->regs[d->input[i]];
        }
    }
}

static void dac_clear(CvState *s) {
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

/* ---- Register writes ------------------------------------------------------------------------ */

static void run_action(CvState *s, uint8_t action, const CvCommand *cmd, const uint32_t *caps);

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
        continuous(s) && c->start_on_enter) {
        cv_start_conversion(s, -1);
    }
    if (c->present && c->mode == 2 && c->mode_field.reg == reg && old_mode_cont && !continuous(s)) {
        /* leaving continuous mode: the conversion in progress finishes */
    }
    for (i = 0; i < p->trigger_count; i++) {
        if (p->triggers[i].cond.field.reg == reg && cv_conds_hold(s, &(CvConds){&p->triggers[i].cond, 1})) {
            run_action(s, p->triggers[i].action, NULL, NULL);
        }
    }
    ch = dac_input_channel(s, reg);
    if (ch != CV_NONE && (s->dev->dacinfo.ldac == 2 || ldac_level_active(s))) {
        dac_update(s, ch);
    }
}

/* A bus write: read-only bits kept, write-1-to-clear bits cleared by a 1. */
void cv_reg_write(CvState *s, int reg, uint32_t value) {
    const CvRegister *r = &s->dev->regs[reg];
    uint32_t m = mask_of(r->width);
    uint32_t old = s->regs[reg];
    uint32_t nv;
    uint32_t cont = continuous(s);

    value &= m;
    nv = (old & r->ro) | (value & ~r->ro);
    if (r->w1c) {
        nv = (nv & ~r->w1c) | (old & r->w1c & ~value);
    }
    s->regs[reg] = nv & m;
    after_write(s, reg, value, cont);
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
        field_put(s, &p->levels[i].field,
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
        ((d->conversion.starts & CVS_POWER_ON) || (continuous(s) && d->conversion.start_on_enter))) {
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
        return reg_at(s, cmd->space, value_of(&cmd->address, caps));
    }
    return CV_NONE;
}

static void set_out(CvState *s, CvSeg *seg, const CvPattern *pat, const uint32_t *caps,
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
static size_t reg_run(const CvState *s, int first, size_t count, int *out) {
    size_t n = 0;
    int r = first;

    while (r != CV_NONE && n < count && n < CV_MAX_AT) {
        out[n++] = r;
        r = reg_next(s, r);
    }
    return n;
}

static void run_action(CvState *s, uint8_t action, const CvCommand *cmd, const uint32_t *caps) {
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
        s->x.pay_count = reg_run(s, reg, count, s->x.pay_regs);
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
        n = reg == CV_NONE ? 0 : reg_run(s, reg, count, list);
        s->at_count = n;
        memcpy(s->at, list, n * sizeof(*list));
        s->at_word = cmd->word_bits;
        if (!cmd->has_reply) {
            static const CvTok AT = {CVT_AT, 0, 0, 0};
            static const CvPattern AT_PATTERN = {&AT, 1, 0, 1};

            set_out(s, &s->seg, &AT_PATTERN, caps, list, n, cmd->word_bits, 1);
            s->seg_from = 1;
        }
        for (i = 0; i < n; i++) {
            cv_reg_read(s, list[i]);
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
        conv_stop(s);
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
            dac_update(s, chans[i]);
        }
        break;
    case CVA_UPDATEALL:
        dac_update(s, -1);
        break;
    case CVA_CLEAR:
        dac_clear(s);
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
static void execute(CvState *s, const CvCommand *cmd, const uint32_t *caps) {
    size_t i;

    for (i = 0; i < cmd->set_count; i++) {
        field_put(s, &cmd->sets[i].field, value_of(&cmd->sets[i].value, caps));
    }
    run_action(s, cmd->action, cmd, caps);
    if (cmd->action != CVA_POINTER) {
        for (i = 0; i < cmd->then_count; i++) {
            run_action(s, cmd->then[i], cmd, caps);
        }
    }
    if (cmd->has_next) {
        set_out(s, &s->next, &cmd->next, caps, s->at, s->at_count, s->at_word, 1);
        s->has_next = 1;
    }
    if (cmd->action == CVA_START || (cmd->then_count && cmd->then[0] == CVA_START)) {
        /* "command" start trigger: the action is the trigger */
    }
    cv_ready_refresh(s);
}

/* ---- The protocol engine: bits in ------------------------------------------------------------ */

static int literal_ok(const CvTok *t, uint8_t bit) {
    return !((t->kind == CVT_0 && bit != 0) || (t->kind == CVT_1 && bit != 1));
}

static void captures(const CvPattern *p, const uint8_t *bits, uint32_t *caps) {
    size_t i;

    memset(caps, 0, CV_CAPS * sizeof(*caps));
    for (i = 0; i < p->bits; i++) {
        if (p->toks[i].kind == CVT_CAP) {
            caps[p->toks[i].cap] = (caps[p->toks[i].cap] << 1) | bits[i];
        }
    }
}

static void boundary(CvState *s) {
    const CvProtocol *p = &s->dev->protocol;
    size_t i;

    s->x.n = 0;
    for (i = 0; i < p->command_count; i++) {
        s->x.alive[i] = cv_conds_hold(s, &p->commands[i].when);
    }
}

static void fire(CvState *s, const CvCommand *cmd, const uint32_t *caps) {
    if (cmd->has_reply) {
        set_out(s, &s->seg, &cmd->reply, caps, s->at, s->at_count, s->at_word, 1);
        s->seg_from = 1;
    }
    if (cmd->commit_end) {
        s->x.deferred = cmd;
        memcpy(s->x.deferred_caps, caps, sizeof s->x.deferred_caps);
    } else {
        execute(s, cmd, caps);
        if (cmd->has_reply && cmd->action == CVA_RREG) {
            /* the rreg list was built during execute: the reply's @ follows it */
            set_out(s, &s->seg, &cmd->reply, caps, s->at, s->at_count, s->at_word, 1);
        }
    }
    if (s->x.payload == CV_PAY_NONE) {
        if (s->dev->protocol.stream) {
            boundary(s);
        } else {
            s->x.done = 1;
        }
    }
}

/* Re-run matching over the bits now in the buffer (after a drop). */
static void rematch(CvState *s);

static void match_bit(CvState *s) {
    const CvProtocol *p = &s->dev->protocol;
    size_t n = s->x.n, i;
    int any = 0;

    for (i = 0; i < p->command_count; i++) {
        const CvCommand *c = &p->commands[i];

        if (!s->x.alive[i]) {
            continue;
        }
        if (n > c->in.bits || !literal_ok(&c->in.toks[n - 1], s->x.bits[n - 1])) {
            s->x.alive[i] = 0;
            continue;
        }
        if (n == c->in.bits) {
            uint32_t caps[CV_CAPS];

            captures(&c->in, s->x.bits, caps);
            fire(s, c, caps);
            return;
        }
        any = 1;
    }
    if (!any) {
        if (p->stream) {
            if (n >= 8) {
                memmove(s->x.bits, s->x.bits + 8, n - 8);
                s->x.n = n - 8;
                rematch(s);
            }
        } else {
            s->x.done = 1;
        }
    }
}

static void rematch(CvState *s) {
    size_t n = s->x.n, k;
    uint8_t tmp[CV_MAX_BITS];

    memcpy(tmp, s->x.bits, n);
    boundary(s);
    for (k = 0; k < n; k++) {
        s->x.bits[s->x.n++] = tmp[k];
        match_bit(s);
        if (s->x.done || s->x.payload != CV_PAY_NONE || s->x.n == 0) {
            /* a command fired: the rest of the bits are lost (rare: a drop mid-stream) */
            return;
        }
    }
}

static void payload_bit(CvState *s, uint8_t bit) {
    CvXfer *x = &s->x;

    if (x->payload == CV_PAY_WREG) {
        int reg = x->pay_regs[x->pay_index];
        unsigned width = s->dev->regs[reg].width;
        unsigned word = x->pay_word > width ? x->pay_word : width;

        x->pay_value = (x->pay_value << 1) | bit;
        if (++x->pay_bits == word) {
            cv_reg_write(s, reg, x->pay_value >> (word - width));
            x->pay_bits = 0;
            x->pay_value = 0;
            if (++x->pay_index >= x->pay_count) {
                x->payload = CV_PAY_NONE;
                if (s->dev->protocol.stream) {
                    boundary(s);
                } else {
                    x->done = 1;
                }
            }
        }
        return;
    }
    if (x->payload == CV_PAY_POINTER) {
        int reg = reg_at(s, s->pointer_space, s->pointer);
        unsigned width = reg == CV_NONE ? 8 : s->dev->regs[reg].width;
        size_t i;

        x->pay_value = (x->pay_value << 1) | bit;
        if (++x->pay_bits == width) {
            if (reg != CV_NONE) {
                cv_reg_write(s, reg, x->pay_value);
                for (i = 0; x->pay_cmd != NULL && i < x->pay_cmd->then_count; i++) {
                    run_action(s, x->pay_cmd->then[i], x->pay_cmd, x->pay_caps);
                }
            }
            x->pay_bits = 0;
            x->pay_value = 0;
            if (s->pointer_inc && reg != CV_NONE) {
                int nx = reg_next(s, reg);

                if (nx != CV_NONE) {
                    s->pointer = (uint32_t)s->dev->regs[nx].address;
                }
            }
        }
    }
}

void cv_bit_in(CvState *s, LogicValue v) {
    CvXfer *x = &s->x;

    if (!x->open || x->xed) {
        return;
    }
    if (v != LOGIC_0 && v != LOGIC_1) {
        x->xed = 1;
        return;
    }
    if (x->payload != CV_PAY_NONE) {
        payload_bit(s, v == LOGIC_1);
        return;
    }
    if (s->dev->protocol.align_end) {
        if (x->n == CV_MAX_BITS) {
            memmove(x->bits, x->bits + 1, CV_MAX_BITS - 1);
            x->n--;
        }
        x->bits[x->n++] = v == LOGIC_1;
        return;
    }
    if (x->done || x->n >= CV_MAX_BITS) {
        return;
    }
    x->bits[x->n++] = v == LOGIC_1;
    match_bit(s);
}

/* ---- The protocol engine: bits out ------------------------------------------------------------ */

static const CvPattern *alt_out(const CvState *s) {
    const CvProtocol *p = &s->dev->protocol;
    size_t i;

    for (i = 0; i < p->out_alt_count; i++) {
        if (cv_conds_hold(s, &p->outs[i].when)) {
            return &p->outs[i].pattern;
        }
    }
    return &p->out;
}

void cv_xfer_begin(CvState *s, int reading) {
    CvXfer *x = &s->x;
    uint8_t *alive = x->alive;

    memset(x, 0, sizeof *x);
    x->alive = alive;
    x->open = 1;
    x->reading = reading;
    s->read_noted = 0;
    s->out_channel = s->conv_channel;
    boundary(s);
    if (s->has_next) {
        s->seg = s->next;
        s->seg_from = 1;
        s->has_next = 0;
    } else if (!reading) {
        set_out(s, &s->seg, alt_out(s), NULL, NULL, 0, 0, 0);
        s->seg_from = 0;
    } else {
        cv_read_begin(s);
    }
    s->last = s->seg;
}

void cv_xfer_end(CvState *s) {
    CvXfer *x = &s->x;
    const CvProtocol *p = &s->dev->protocol;

    if (!x->open) {
        return;
    }
    if (!x->xed && p->align_end && !x->reading) {
        size_t i;

        for (i = 0; i < p->command_count; i++) {
            const CvCommand *c = &p->commands[i];
            size_t k, off;
            int ok = 1;

            if (c->in.bits > x->n || !cv_conds_hold(s, &c->when)) {
                continue;
            }
            off = x->n - c->in.bits;
            for (k = 0; k < c->in.bits && ok; k++) {
                ok = literal_ok(&c->in.toks[k], x->bits[off + k]);
            }
            if (ok) {
                uint32_t caps[CV_CAPS];

                captures(&c->in, x->bits + off, caps);
                execute(s, c, caps);
                break;
            }
        }
    }
    if (!x->xed && x->deferred != NULL) {
        execute(s, x->deferred, x->deferred_caps);
    }
    x->open = 0;
    x->payload = CV_PAY_NONE;
}

static LogicValue tok_value(CvState *s, CvSeg *g, const CvTok *t) {
    switch (t->kind) {
    case CVT_0:
        return LOGIC_0;
    case CVT_1:
        return LOGIC_1;
    case CVT_Z:
        return LOGIC_Z;
    case CVT_CAP:
        return (g->caps[t->cap] >> t->bit) & 1u ? LOGIC_1 : LOGIC_0;
    case CVT_R:
        cv_result_read(s);
        return (cv_code(s) >> t->bit) & 1u ? LOGIC_1 : LOGIC_0;
    case CVT_CH:
        return ((uint32_t)s->out_channel >> t->bit) & 1u ? LOGIC_1 : LOGIC_0;
    case CVT_REF:
        return (s->regs[t->reg] >> t->bit) & 1u ? LOGIC_1 : LOGIC_0;
    default:
        return LOGIC_0;
    }
}

/* The next output bit; *sdio: it belongs to a reply (driven on a shared SDIO pin). */
LogicValue cv_bit_out(CvState *s, int *sdio) {
    CvSeg *g = &s->seg;
    int guard = 0;

    *sdio = 0;
    while (g->active && guard++ < 4) {
        while (g->pos < g->pat->count) {
            const CvTok *t = &g->pat->toks[g->pos];

            if (t->kind == CVT_AT) {
                if (g->at_index < g->at_count) {
                    int reg = g->at[g->at_index];
                    unsigned width = s->dev->regs[reg].width;
                    unsigned word = g->at_word > width ? g->at_word : width;
                    unsigned bit = word - 1 - g->at_bit;
                    LogicValue v;

                    if (bit >= word - width) {
                        v = (s->regs[reg] >> (bit - (word - width))) & 1u ? LOGIC_1 : LOGIC_0;
                    } else {
                        v = LOGIC_0;
                    }
                    if (s->dev->regs[reg].result && g->at_bit == 0) {
                        cv_result_read(s);
                    }
                    if (++g->at_bit == word) {
                        g->at_bit = 0;
                        g->at_index++;
                    }
                    *sdio = g->sdio;
                    return v;
                }
                g->pos++;
                g->at_index = 0;
                continue;
            }
            g->pos++;
            *sdio = g->sdio;
            return tok_value(s, g, t);
        }
        /* the pattern ran out: the idle rule */
        {
            int idle = s->dev->protocol.idle ? s->dev->protocol.idle : s->idle_default;

            if (idle == 'r' && g->pat->count > 0) {
                g->pos = 0;
                g->at_index = 0;
                g->at_bit = 0;
                continue;
            }
            g->active = 0;
            *sdio = 0;
            return idle == '1' ? LOGIC_1 : idle == 'z' ? LOGIC_Z : LOGIC_0;
        }
    }
    {
        int idle = s->dev->protocol.idle ? s->dev->protocol.idle : s->idle_default;

        return idle == '1' ? LOGIC_1 : idle == 'z' || idle == 'r' ? (s->idle_default == '1' ?
               LOGIC_1 : LOGIC_0) : LOGIC_0;
    }
}

/* I2C / parallel reads: the read pattern (next, reads alternatives, read, pointer). */
void cv_read_begin(CvState *s) {
    const CvProtocol *p = &s->dev->protocol;
    const CvRead *rd = &p->read;
    size_t i;

    for (i = 0; i < p->read_alt_count; i++) {
        if (cv_conds_hold(s, &p->reads[i].when)) {
            rd = &p->reads[i].read;
            break;
        }
    }
    s->seg_from = 1;
    if (rd->kind == CVR_PATTERN) {
        set_out(s, &s->seg, &rd->pattern, NULL, NULL, 0, 0, 1);
    } else if (rd->kind == CVR_POINTER) {
        static const CvTok AT = {CVT_AT, 0, 0, 0};
        static const CvPattern AT_PATTERN = {&AT, 1, 0, 1};
        int reg = reg_at(s, s->pointer_space, s->pointer);
        int list[CV_MAX_AT];
        size_t n = 0;

        if (reg != CV_NONE) {
            if (s->pointer_inc) {
                n = reg_run(s, reg, CV_MAX_AT, list);
            } else {
                list[0] = reg;
                n = 1;
            }
            for (i = 0; i < n; i++) {
                cv_reg_read(s, list[i]);
            }
        }
        set_out(s, &s->seg, &AT_PATTERN, NULL, list, n, 0, 1);
    } else if (rd->kind == CVR_MAP) {
        uint32_t key = s->read_key;

        for (i = 0; i < rd->map_count; i++) {
            if (rd->map[i].key == key) {
                set_out(s, &s->seg, &rd->map[i].pattern, NULL, NULL, 0, 0, 1);
                return;
            }
        }
        s->seg.active = 0;
    } else {
        s->seg.active = 0;
    }
}

/* ---- SPI front end -------------------------------------------------------------------------- */

static const CvBus *spi_bus(const CvState *s) {
    if (s->dev->bus.style == CVB_SPI || s->dev->bus.style == CVB_I2C_SPI) {
        return &s->dev->bus;
    }
    if (s->dev->has_config_bus && s->dev->config_bus.style == CVB_SPI) {
        return &s->dev->config_bus;
    }
    return NULL;
}

static int on_miso_ready(const CvState *s, const CvBus *b) {
    size_t i;

    for (i = 0; i < s->dev->ready_count; i++) {
        if (s->dev->ready[i].on_miso && s->dev->ready[i].pin == b->miso) {
            return 1;
        }
    }
    return 0;
}

/* Drives MISO from the SPI state. */
void cv_spi_refresh(CvState *s) {
    const CvBus *b = spi_bus(s);
    LogicValue v;
    size_t miso;

    if (b == NULL || b->miso == CV_NONE || s->spi_off) {
        return;
    }
    miso = cv_pin(s, b->miso);
    if (s->in_reset || s->powered_down_bus) {
        v = LOGIC_Z;
    } else if (!s->spi_selected) {
        v = (b->cs == CV_NONE && on_miso_ready(s, b)) ? cv_ready_level(s, b->miso) : LOGIC_Z;
    } else if (!s->spi_shifting && on_miso_ready(s, b)) {
        v = cv_ready_level(s, b->miso);
    } else {
        v = s->spi_out;
    }
    cv_drive(s, miso, v, s->spi_delay);
    s->spi_delay = 0;
}

static void spi_shift(CvState *s, const CvBus *b, SimTime delay) {
    int sdio;
    LogicValue v = cv_bit_out(s, &sdio);

    if (s->x.xed && v != LOGIC_Z) {
        v = LOGIC_X;
    }
    if (b->mosi != CV_NONE && b->mosi == b->miso && !sdio) {
        v = LOGIC_Z;
    }
    s->spi_out = v;
    s->spi_delay = delay;
    cv_spi_refresh(s);
}

static void spi_select(CvState *s, const CvBus *b) {
    const CvConversion *c = &s->dev->conversion;

    s->spi_selected = 1;
    s->spi_shifting = 0;
    s->spi_bits = 0;
    cv_xfer_begin(s, 0);
    ready_clear(s, CVC_CS_FALL);
    if (c->starts & CVS_CS_FALL) {
        cv_start_conversion(s, -1);
    }
    if ((b->mode & 1u) == 0) {
        spi_shift(s, b, s->dev->timing.t_valid);
    } else {
        s->spi_out = LOGIC_Z;
    }
    cv_ready_refresh(s);
}

static void spi_deselect(CvState *s) {
    const CvConversion *c = &s->dev->conversion;

    s->spi_selected = 0;
    s->spi_shifting = 0;
    cv_xfer_end(s);
    s->spi_delay = s->dev->timing.t_float;
    if (c->starts & CVS_CS_RISE) {
        cv_start_conversion(s, -1);
    }
    cv_ready_refresh(s);
}

/* 2-wire parts: a finished conversion starts a new frame. */
void cv_spi_conversion_done(CvState *s) {
    const CvBus *b = spi_bus(s);

    if (b == NULL || b->cs != CV_NONE) {
        return;
    }
    if (s->spi_selected) {
        cv_xfer_end(s);
    }
    s->spi_selected = 1;
    s->spi_shifting = 0;
    cv_xfer_begin(s, 0);
    s->spi_out = LOGIC_Z;
    if ((b->mode & 1u) == 0) {
        int sdio;

        s->pending_first = cv_bit_out(s, &sdio);
        s->has_pending_first = 1;
    }
}

static void spi_pin(CvState *s, const CvBus *b, size_t pin) {
    size_t cs = cv_pin(s, b->cs), sclk = cv_pin(s, b->sclk);

    if (b->cs != CV_NONE && pin == cs) {
        LogicValue v = cv_level(s, cs, b->cs_level ? LOGIC_0 : LOGIC_1);
        int sel = v == (b->cs_level ? LOGIC_1 : LOGIC_0);

        if (v == LOGIC_X) {
            if (s->spi_selected) {
                s->x.xed = 1;
            }
            return;
        }
        if (sel && !s->spi_selected) {
            spi_select(s, b);
        } else if (!sel && s->spi_selected) {
            spi_deselect(s);
        }
        return;
    }
    if (pin == sclk) {
        LogicValue v = cv_level(s, sclk, LOGIC_X);
        int idle = (int)(b->mode >> 1);
        int leading, sample;

        if (v == LOGIC_X) {
            if (s->spi_selected) {
                s->x.xed = 1;
            }
            s->sclk = LOGIC_X;
            return;
        }
        if (s->sclk == v) {
            return;
        }
        s->sclk = v;
        if (!s->spi_selected) {
            return;
        }
        leading = (v == LOGIC_1) != (idle == 1);
        sample = (b->mode & 1u) ? !leading : leading;
        if (sample) {
            s->spi_shifting = 1;
            if (b->mosi != CV_NONE) {
                cv_bit_in(s, cv_level(s, cv_pin(s, b->mosi), LOGIC_X));
            }
            s->spi_bits++;
        } else {
            if (!s->spi_shifting && s->has_pending_first) {
                /* 2-wire mode 0: the first bit is already queued */
                s->has_pending_first = 0;
                s->spi_shifting = 1;
                s->spi_out = s->pending_first;
                s->spi_delay = s->dev->timing.t_out;
                cv_spi_refresh(s);
                return;
            }
            if ((b->mode & 1u) || s->spi_bits > 0) {
                s->spi_shifting = 1;
                spi_shift(s, b, s->dev->timing.t_out);
            }
        }
    }
}

/* ---- I2C front end --------------------------------------------------------------------------- */

static const CvBus *i2c_bus(const CvState *s) {
    if (s->dev->bus.style == CVB_I2C || s->dev->bus.style == CVB_I2C_SPI) {
        return &s->dev->bus;
    }
    if (s->dev->has_config_bus && s->dev->config_bus.style == CVB_I2C) {
        return &s->dev->config_bus;
    }
    return NULL;
}

static int i2c_my_address(const CvState *s) {
    const CvBus *b = i2c_bus(s);
    const int *table = s->pd->has_bus_i2c ? s->pd->table : b->table;
    unsigned count = s->pd->has_bus_i2c ? s->pd->strap_count : b->strap_count;
    unsigned k, index = 0, mult = 1, present = 0;

    if (s->pd->i2c_address != CV_NONE) {
        return s->pd->i2c_address;
    }
    for (k = 0; k < count; k++) {
        size_t pin = s->pd->has_bus_i2c ? (size_t)s->pd->straps[k] : cv_pin(s, b->straps[k]);
        int tie = 0;

        if (pin != SIZE_MAX) {
            tie = serial_i2c_strap(&s->i2c, present++);
            if (tie < 0) {
                return -2;
            }
        }
        index += (unsigned)tie * mult;
        mult *= 4;
    }
    return table[index];
}

static void i2c_begin(void *ctx, int repeated) {
    CvState *s = ctx;

    if (repeated && s->x.open) {
        cv_xfer_end(s);
    }
}

static LogicValue i2c_address(void *ctx, uint8_t byte, uint8_t known) {
    CvState *s = ctx;
    int mine;

    if (known != 0xFF || s->in_reset) {
        return known != 0xFF ? LOGIC_X : LOGIC_0;
    }
    s->i2c_general = 0;
    if (byte == 0x00 && s->dev->protocol.general_reset) {
        s->i2c_general = 1;
        return LOGIC_1;
    }
    mine = i2c_my_address(s);
    if (mine < 0 || (byte >> 1) != (unsigned)mine) {
        return LOGIC_0;
    }
    if (s->x.open) {
        cv_xfer_end(s);
    }
    cv_xfer_begin(s, byte & 1u);
    return LOGIC_1;
}

static LogicValue i2c_received(void *ctx, unsigned index, uint8_t data, uint8_t known) {
    CvState *s = ctx;
    int i;

    (void)index;
    if (s->i2c_general) {
        if (known == 0xFF && data == 0x06) {
            cv_reset(s);
            cv_ready_refresh(s);
        }
        return LOGIC_1;
    }
    for (i = 7; i >= 0; i--) {
        cv_bit_in(s, ((known >> i) & 1u) ? (((data >> i) & 1u) ? LOGIC_1 : LOGIC_0) : LOGIC_X);
    }
    return LOGIC_1;
}

static void i2c_next(void *ctx, unsigned index, uint8_t *data, uint8_t *known) {
    CvState *s = ctx;
    int i;

    (void)index;
    *data = 0;
    *known = 0xFF;
    for (i = 7; i >= 0; i--) {
        int sdio;
        LogicValue v = cv_bit_out(s, &sdio);

        if (v == LOGIC_1 || v == LOGIC_Z) {
            *data |= (uint8_t)(1u << i);
        } else if (v == LOGIC_X) {
            *known &= (uint8_t)~(1u << i);
        }
    }
}

static void i2c_end(void *ctx, int stop) {
    CvState *s = ctx;

    (void)stop;
    s->i2c_general = 0;
    cv_xfer_end(s);
    cv_ready_refresh(s);
}

/* ---- The model -------------------------------------------------------------------------------- */

static size_t cv_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *cv_create(const void *config) {
    const Part *part = config;
    const PartInternal *in = part_internal(part);
    CvState *s = calloc(1, sizeof *s);
    size_t i;

    if (s == NULL) {
        return NULL;
    }
    s->part = part;
    s->dev = in->device;
    s->b = in->bound;
    s->pd = in->data;
    s->regs = calloc(s->dev->reg_count ? s->dev->reg_count : 1, sizeof(*s->regs));
    s->drv = malloc(part->pin_count * sizeof(*s->drv));
    s->ready_on = calloc(s->dev->ready_count ? s->dev->ready_count : 1, 1);
    s->ready_gen = calloc(s->dev->ready_count ? s->dev->ready_count : 1, sizeof(*s->ready_gen));
    s->x.alive = calloc(s->dev->protocol.command_count ? s->dev->protocol.command_count : 1, 1);
    if (s->regs == NULL || s->drv == NULL || s->ready_on == NULL || s->ready_gen == NULL ||
        s->x.alive == NULL) {
        cv_free_state(s);
        return NULL;
    }
    s->alive_store = s->x.alive;
    for (i = 0; i < part->pin_count; i++) {
        s->drv[i] = LOGIC_Z;
    }
    s->read_reg = CV_NONE;
    s->sclk = LOGIC_X;
    s->idle_default = '0';
    if (i2c_bus(s) != NULL) {
        const CvBus *b = i2c_bus(s);
        unsigned k;

        s->i2c_pins.scl = cv_pin(s, b->scl);
        s->i2c_pins.sda = cv_pin(s, b->sda);
        s->i2c_pins.strap_count = 0;
        if (s->pd->has_bus_i2c) {
            for (k = 0; k < s->pd->strap_count; k++) {
                s->i2c_pins.strap[s->i2c_pins.strap_count++] = (size_t)s->pd->straps[k];
            }
        } else {
            for (k = 0; k < b->strap_count; k++) {
                if (cv_pin(s, b->straps[k]) != SIZE_MAX) {
                    s->i2c_pins.strap[s->i2c_pins.strap_count++] = cv_pin(s, b->straps[k]);
                }
            }
        }
        serial_i2c_init(&s->i2c, &s->i2c_pins);
        s->i2c.t_out = s->dev->timing.t_out;
        s->i2c_ev.context = s;
        s->i2c_ev.begin = i2c_begin;
        s->i2c_ev.address = i2c_address;
        s->i2c_ev.received = i2c_received;
        s->i2c_ev.next = i2c_next;
        s->i2c_ev.end = i2c_end;
        s->has_i2c = 1;
    }
    return s;
}

void cv_free_state(CvState *s) {
    if (s == NULL) {
        return;
    }
    free(s->regs);
    free(s->drv);
    free(s->ready_on);
    free(s->ready_gen);
    free(s->alive_store);
    free(s);
}

static void cv_destroy(void *state) {
    cv_free_state(state);
}

static void control_pins_start(CvState *s) {
    const CvPins *p = &s->dev->pins;
    size_t i;

    for (i = 0; i < CVP_COUNT; i++) {
        if (p->has[i]) {
            s->ctl_level[i] = cv_level(s, cv_pin(s, p->ctl[i].pin),
                                       p->ctl[i].active ? LOGIC_0 : LOGIC_1);
        }
    }
    s->in_reset = p->has[CVP_RESET] &&
                  s->ctl_level[CVP_RESET] == (p->ctl[CVP_RESET].active ? LOGIC_1 : LOGIC_0);
    s->powered_down_bus = p->has[CVP_POWERDOWN] &&
                          s->ctl_level[CVP_POWERDOWN] ==
                              (p->ctl[CVP_POWERDOWN].active ? LOGIC_1 : LOGIC_0);
    s->start_level = p->has[CVP_START] &&
                     s->ctl_level[CVP_START] == (p->ctl[CVP_START].active ? LOGIC_1 : LOGIC_0);
}

static void cv_start(Sim *sim, SimInstance *instance, void *state) {
    CvState *s = state;
    const CvBus *b;

    s->sim = sim;
    s->inst = instance;
    control_pins_start(s);
    cv_reset(s);
    if (s->has_i2c) {
        serial_i2c_start(&s->i2c, sim, instance);
        s->idle_default = 'r';
    }
    b = spi_bus(s);
    if (b != NULL) {
        s->sclk = cv_level(s, cv_pin(s, b->sclk), LOGIC_X);
        if (b->cs != CV_NONE) {
            LogicValue v = cv_level(s, cv_pin(s, b->cs), b->cs_level ? LOGIC_0 : LOGIC_1);

            if (v == (b->cs_level ? LOGIC_1 : LOGIC_0) && !s->in_reset) {
                spi_select(s, b);
            }
        }
    }
    if (s->dev->bus.style == CVB_I2C_SPI) {
        LogicValue v = cv_level(s, cv_pin(s, s->dev->bus.select), LOGIC_0);

        s->spi_off = (v == LOGIC_1) != (s->dev->bus.spi_level == 1);
        serial_i2c_enable(&s->i2c, sim, instance, s->spi_off);
    }
    cv_par_start(s);
    cv_ready_refresh(s);
}

static int is_level_active(const CvPinCtl *c, LogicValue v) {
    return v == (c->active ? LOGIC_1 : LOGIC_0);
}

static void control_pin(CvState *s, size_t pin) {
    const CvPins *p = &s->dev->pins;
    size_t i;

    for (i = 0; i < p->level_count; i++) {
        if (cv_pin(s, p->levels[i].pin) == pin) {
            field_put(s, &p->levels[i].field, cv_level(s, pin, LOGIC_0) == LOGIC_1);
        }
    }
    for (i = 0; i < CVP_COUNT; i++) {
        const CvPinCtl *c = &p->ctl[i];
        LogicValue v, old;
        int rising, falling, edge_hit;

        if (!p->has[i] || cv_pin(s, c->pin) != pin) {
            continue;
        }
        v = cv_level(s, pin, c->active ? LOGIC_0 : LOGIC_1);
        old = s->ctl_level[i];
        if (v == old) {
            continue;
        }
        s->ctl_level[i] = v;
        rising = old == LOGIC_0 && v == LOGIC_1;
        falling = old == LOGIC_1 && v == LOGIC_0;
        edge_hit = c->edge == 1 ? rising : c->edge == 0 ? falling : 0;
        switch (i) {
        case CVP_RESET:
            if (is_level_active(c, v)) {
                s->in_reset = 1;
                if (s->x.open) {
                    s->x.open = 0;
                }
                s->spi_selected = 0;
                cv_reset(s);
            } else if (s->in_reset) {
                s->in_reset = 0;
                cv_reset(s);
            }
            break;
        case CVP_CONVST:
            if (edge_hit && (s->dev->conversion.starts & CVS_PIN)) {
                cv_start_conversion(s, -1);
            }
            break;
        case CVP_START:
            if (c->edge >= 0) {
                if (edge_hit) {
                    cv_start_conversion(s, -1);
                }
            } else {
                s->start_level = is_level_active(c, v);
                if (s->start_level) {
                    cv_start_conversion(s, -1);
                } else if (s->converting && continuous(s) == 0 && s->dev->conversion.mode != 1) {
                    /* the conversion in progress completes */
                }
            }
            break;
        case CVP_POWERDOWN:
            s->powered_down_bus = is_level_active(c, v);
            s->powered_down = s->powered_down_bus;
            if (s->powered_down_bus) {
                conv_stop(s);
            }
            break;
        case CVP_LDAC:
            if (edge_hit || (c->edge < 0 && is_level_active(c, v))) {
                dac_update(s, -1);
            }
            break;
        case CVP_CLEAR:
            if (edge_hit || (c->edge < 0 && is_level_active(c, v))) {
                dac_clear(s);
            }
            break;
        default:
            break;
        }
    }
    cv_ready_refresh(s);
}

static void cv_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    CvState *s = state;
    const CvBus *b = spi_bus(s);

    (void)old_value;
    (void)new_value;
    s->sim = sim;
    s->inst = instance;
    control_pin(s, pin);
    if (s->dev->bus.style == CVB_I2C_SPI && pin == cv_pin(s, s->dev->bus.select)) {
        LogicValue v = cv_level(s, pin, LOGIC_0);

        s->spi_off = (v == LOGIC_1) != (s->dev->bus.spi_level == 1);
        serial_i2c_enable(&s->i2c, sim, instance, s->spi_off);
    }
    if (s->in_reset || s->powered_down_bus) {
        return;
    }
    if (b != NULL && !s->spi_off) {
        spi_pin(s, b, pin);
    }
    if (s->has_i2c) {
        serial_i2c_pin(&s->i2c, sim, instance, pin, &s->i2c_ev);
    }
    cv_par_pin(s, pin);
}

static void cv_timer(Sim *sim, SimInstance *instance, void *state, uint32_t id) {
    CvState *s = state;

    s->sim = sim;
    s->inst = instance;
    if ((id & CV_TM_KIND) == CV_TM_CONV) {
        if ((id & CV_TM_GEN) == (s->conv_gen & CV_TM_GEN) && s->converting) {
            conv_done(s);
        }
    } else if ((id & CV_TM_KIND) == CV_TM_READY) {
        size_t i = (id >> 16) & 0xFFFu;

        if (i < s->dev->ready_count && (id & 0xFFFFu) == (s->ready_gen[i] & 0xFFFFu)) {
            s->ready_on[i] = 0;
            cv_ready_refresh(s);
        }
    } else {
        cv_par_timer(s, id);
    }
}

const SimComponentType CONVERTER_COMPONENT = {
    "converter", cv_pin_count, cv_create, cv_destroy, cv_start, cv_pin_changed, cv_timer,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

/* ---- The kind -------------------------------------------------------------------------------- */

static const char *const DEVICE_KEYS[] = {
    "type", "bits", "channels", "codeFormat", "vcc", "inputMax", "analog", "bus", "configBus",
    "timing", "registers", "protocol", "conversion", "ready", "pins", "dac", NULL
};

static const char *const PART_KEYS[] = {"vcc", "inputMax", "i2cAddress", "i2c", "resets", NULL};

static void *conv_load_device(KindContext *ctx, const cJSON *json) {
    return cv_load_device(ctx, json, 0);
}

static int cv_pin_open_drain(const Part *part, size_t pin) {
    const CvBound *b = part_internal(part)->bound;

    return pin < b->pin_count && b->open_drain[pin];
}

static int cv_pin_analog(const Part *part, size_t pin) {
    const CvBound *b = part_internal(part)->bound;

    return pin < b->pin_count && b->analog[pin];
}

const PartKindOps CONVERTER_KIND = {
    PART_CONVERTER, "converter", &CONVERTER_COMPONENT, DEVICE_KEYS, PART_KEYS,
    conv_load_device, cv_bind_pinout, cv_load_part, cv_pin_open_drain, cv_pin_analog
};
