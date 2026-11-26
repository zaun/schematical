/* models/converter_proto.c: part of the model in models/converter.c (split by plan-68-I/J). */
#include "converter_core_int.h"
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
        cvc_set_out(s, &s->seg, &cmd->reply, caps, s->at, s->at_count, s->at_word, 1);
        s->seg_from = 1;
    }
    if (cmd->commit_end) {
        s->x.deferred = cmd;
        memcpy(s->x.deferred_caps, caps, sizeof s->x.deferred_caps);
    } else {
        cvc_execute(s, cmd, caps);
        if (cmd->has_reply && cmd->action == CVA_RREG) {
            /* the rreg list was built during execute: the reply's @ follows it */
            cvc_set_out(s, &s->seg, &cmd->reply, caps, s->at, s->at_count, s->at_word, 1);
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
    if (x->payload == CV_PAY_LCD) {
        x->pay_value = (x->pay_value << 1) | bit;
        if (++x->pay_bits == 8) {
            disp_lcd_byte(s, (uint8_t)x->pay_value);
            x->pay_bits = 0;
            x->pay_value = 0;
        }
        return;
    }
    if (x->payload == CV_PAY_POINTER) {
        int reg = cvc_reg_at(s, s->pointer_space, s->pointer);
        unsigned width = reg == CV_NONE ? 8 : s->dev->regs[reg].width;
        size_t i;

        x->pay_value = (x->pay_value << 1) | bit;
        if (++x->pay_bits == width) {
            if (reg != CV_NONE) {
                cv_reg_write(s, reg, x->pay_value);
                for (i = 0; x->pay_cmd != NULL && i < x->pay_cmd->then_count; i++) {
                    cvc_run_action(s, x->pay_cmd->then[i], x->pay_cmd, x->pay_caps);
                }
            }
            x->pay_bits = 0;
            x->pay_value = 0;
            if (s->pointer_inc && reg != CV_NONE) {
                int nx = cvc_reg_next(s, reg);

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
        cvc_set_out(s, &s->seg, alt_out(s), NULL, NULL, 0, 0, 0);
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
                cvc_execute(s, c, caps);
                break;
            }
        }
    }
    if (!x->xed && x->deferred != NULL) {
        cvc_execute(s, x->deferred, x->deferred_caps);
    }
    cv_reads_done(s);
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
    default:
        /* CVT_REF: an output pattern has no x (cvl_pattern) and cv_seg_bit handles @ itself */
        cvc_mark_read(s, t->reg);
        return (s->regs[t->reg] >> t->bit) & 1u ? LOGIC_1 : LOGIC_0;
    }
}

/* The next output bit of segment g; *sdio: it belongs to a reply (driven on a
   shared SDIO pin). idle: what follows the pattern ('0', '1', 'z', 'r'). */
LogicValue cv_seg_bit(CvState *s, CvSeg *g, int idle, int *sdio) {
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
                    if (g->at_bit == 0) {
                        cvc_mark_read(s, reg);
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
    return idle == '1' ? LOGIC_1 : idle == 'z' ? LOGIC_Z : LOGIC_0;
}

LogicValue cv_bit_out(CvState *s, int *sdio) {
    int idle = s->dev->protocol.idle ? s->dev->protocol.idle : s->idle_default;

    return cv_seg_bit(s, &s->seg, idle, sdio);
}

void cv_seg_set(CvSeg *seg, const CvPattern *pat) {
    memset(seg, 0, sizeof *seg);
    seg->pat = pat;
    seg->sdio = 1;
    seg->active = pat != NULL;
}

/* The read pattern for the current read key: a `reads` alternative, else `read`
   (a pattern or the address map). NULL for none or a pointer read. */
const CvPattern *cv_read_pattern(CvState *s) {
    const CvProtocol *p = &s->dev->protocol;
    const CvRead *rd = &p->read;
    size_t i;

    for (i = 0; i < p->read_alt_count; i++) {
        if (cv_conds_hold(s, &p->reads[i].when)) {
            rd = &p->reads[i].read;
            break;
        }
    }
    if (rd->kind == CVR_PATTERN) {
        return &rd->pattern;
    }
    if (rd->kind == CVR_MAP) {
        for (i = 0; i < rd->map_count; i++) {
            if (rd->map[i].key == s->read_key) {
                return &rd->map[i].pattern;
            }
        }
    }
    return NULL;
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
        cvc_set_out(s, &s->seg, &rd->pattern, NULL, NULL, 0, 0, 1);
    } else if (rd->kind == CVR_POINTER) {
        static const CvTok AT = {CVT_AT, 0, 0, 0};
        static const CvPattern AT_PATTERN = {&AT, 1, 0, 1};
        int reg = cvc_reg_at(s, s->pointer_space, s->pointer);
        int list[CV_MAX_AT];
        size_t n = 0;

        if (reg != CV_NONE) {
            if (s->pointer_inc) {
                n = cvc_reg_run(s, reg, CV_MAX_AT, list);
            } else {
                list[0] = reg;
                n = 1;
            }
        }
        cvc_set_out(s, &s->seg, &AT_PATTERN, NULL, list, n, 0, 1);
    } else if (rd->kind == CVR_MAP) {
        uint32_t key = s->read_key;

        for (i = 0; i < rd->map_count; i++) {
            if (rd->map[i].key == key) {
                cvc_set_out(s, &s->seg, &rd->map[i].pattern, NULL, NULL, 0, 0, 1);
                return;
            }
        }
        s->seg.active = 0;
    } else {
        s->seg.active = 0;
    }
}

/* ---- SPI front end -------------------------------------------------------------------------- */

const CvBus *cvc_spi_bus(const CvState *s) {
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
    const CvBus *b = cvc_spi_bus(s);
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

void cvc_spi_select(CvState *s, const CvBus *b) {
    const CvConversion *c = &s->dev->conversion;

    s->spi_selected = 1;
    s->spi_shifting = 0;
    s->spi_bits = 0;
    cv_xfer_begin(s, 0);
    cvc_ready_clear(s, CVC_CS_FALL);
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
    const CvBus *b = cvc_spi_bus(s);

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

void cvc_spi_pin(CvState *s, const CvBus *b, size_t pin) {
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
            cvc_spi_select(s, b);
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

const CvBus *cvc_i2c_bus(const CvState *s) {
    if (s->dev->bus.style == CVB_I2C || s->dev->bus.style == CVB_I2C_SPI) {
        return &s->dev->bus;
    }
    if (s->dev->has_config_bus && s->dev->config_bus.style == CVB_I2C) {
        return &s->dev->config_bus;
    }
    return NULL;
}

static int i2c_my_address(const CvState *s) {
    const CvBus *b = cvc_i2c_bus(s);
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

void cvc_i2c_begin(void *ctx, int repeated) {
    CvState *s = ctx;

    if (repeated && s->x.open) {
        cv_xfer_end(s);
    }
}

LogicValue cvc_i2c_address(void *ctx, uint8_t byte, uint8_t known) {
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

LogicValue cvc_i2c_received(void *ctx, unsigned index, uint8_t data, uint8_t known) {
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

void cvc_i2c_next(void *ctx, unsigned index, uint8_t *data, uint8_t *known) {
    CvState *s = ctx;
    int i;

    (void)index;
    *data = 0;
    *known = 0xFF;
    for (i = 7; i >= 0; i--) {
        int sdio;
        LogicValue v = cv_bit_out(s, &sdio);

        /* cv_seg_bit gives 0, 1 or Z, never X: a Z bit is a released line, a 1 */
        if (v == LOGIC_1 || v == LOGIC_Z) {
            *data |= (uint8_t)(1u << i);
        }
    }
}

void cvc_i2c_end(void *ctx, int stop) {
    CvState *s = ctx;

    (void)stop;
    s->i2c_general = 0;
    cv_xfer_end(s);
    cv_ready_refresh(s);
}

