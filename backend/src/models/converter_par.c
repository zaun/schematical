/*
 * Parallel and clocked buses of the converter kind (plan-24-E;
 * planning/plan-24-data/schema.md § 3).
 *
 *   parallel   a microprocessor bus: data pins (MSB first), CS / RD / WR or a
 *              combined R/W, address pins or a multiplexed address latched by
 *              ALE, a byte select. A read (CS and RD active, or CS with R/W =
 *              read, or CS alone when the bus has neither) drives the read
 *              pattern for the current address (or byte) tAcc after it
 *              starts and floats tDf after it ends. A write (CS and WR
 *              active, or CS with R/W = write) is one transaction of the
 *              address bits then the data bits, decoded when it ends. A bus
 *              with neither CS nor WR (unlatched DACs) writes on every change
 *              of the data pins.
 *   clocked    a free-running output bus: each clock edge (rise, fall or both)
 *              drives the next data-width bits of the read pattern, repeating,
 *              tOut later while OE is active (Z otherwise); clock outputs follow
 *              the clock, inverted or divided.
 *
 * Inputs at Z: CS, RD, WR and OE read inactive; data and address bits read X.
 */

#include "converter_int.h"

#include <string.h>

static int active(const CvState *s, const CvCtl *c) {
    LogicValue v = cv_level(s, cv_pin(s, c->pin), c->level ? LOGIC_0 : LOGIC_1);

    return v == (c->level ? LOGIC_1 : LOGIC_0);
}

/* The address bits of an access (pins MSB first, or the ALE latch); known in *ok. */
static uint32_t address_bits(const CvState *s, const CvBus *b, unsigned *bits, int *ok) {
    uint32_t v = 0;
    size_t i;

    *ok = 1;
    if (b->has_ale) {
        *bits = b->ale.bits;
        return s->par_latched;
    }
    *bits = (unsigned)b->address_count;
    for (i = 0; i < b->address_count; i++) {
        LogicValue l = cv_level(s, cv_pin(s, b->address[i]), LOGIC_X);

        if (l == LOGIC_X) {
            *ok = 0;
        }
        v = (v << 1) | (l == LOGIC_1 ? 1u : 0u);
    }
    return v;
}

static void drive_data(CvState *s, const CvBus *b, const LogicValue *v, SimTime delay) {
    size_t i;

    for (i = 0; i < b->data_count; i++) {
        cv_drive(s, cv_pin(s, b->data[i]), v[i], delay);
    }
}

static void release_data(CvState *s, const CvBus *b, SimTime delay) {
    LogicValue z[CV_MAX_DATA];
    size_t i;

    for (i = 0; i < b->data_count; i++) {
        z[i] = LOGIC_Z;
    }
    drive_data(s, b, z, delay);
}

/* Shifts the next data-width bits of the segment onto the data pins. */
static void shift_word(CvState *s, const CvBus *b, CvSeg *seg, int idle, SimTime delay) {
    LogicValue v[CV_MAX_DATA];
    size_t i;

    for (i = 0; i < b->data_count; i++) {
        int sdio;

        v[i] = seg->active ? cv_seg_bit(s, seg, idle, &sdio) : LOGIC_Z;
    }
    drive_data(s, b, v, delay);
}

/* ---- parallel ---------------------------------------------------------------------------- */

static int par_selected(const CvState *s, const CvBus *b) {
    return !b->has_pcs || active(s, &b->pcs);
}

static int par_read_access(const CvState *s, const CvBus *b) {
    if (!par_selected(s, b) || (!b->has_pcs && !b->has_rd && !b->has_rw)) {
        return 0;
    }
    if (b->has_rd) {
        return active(s, &b->rd);
    }
    if (b->has_rw) {
        return cv_level(s, cv_pin(s, b->rw.pin), LOGIC_X) == (b->rw.level ? LOGIC_1 : LOGIC_0);
    }
    return !b->has_wr;
}

static int par_write_access(const CvState *s, const CvBus *b) {
    if (!par_selected(s, b) || (!b->has_pcs && !b->has_wr && !b->has_rw)) {
        return 0;
    }
    if (b->has_wr) {
        return active(s, &b->wr);
    }
    if (b->has_rw) {
        return cv_level(s, cv_pin(s, b->rw.pin), LOGIC_X) == (b->rw.level ? LOGIC_0 : LOGIC_1);
    }
    return 0;
}

static void read_key(CvState *s, const CvBus *b) {
    const CvRead *rd = &s->dev->protocol.read;
    unsigned bits;
    int ok;

    if (rd->byte_keys && b->has_byte) {
        s->read_key = cv_level(s, cv_pin(s, b->byte_sel.pin), LOGIC_0) ==
                      (b->byte_sel.level ? LOGIC_1 : LOGIC_0);
    } else {
        s->read_key = address_bits(s, b, &bits, &ok);
    }
}

static void par_read_drive(CvState *s, const CvBus *b, SimTime delay) {
    const CvPattern *pat;

    read_key(s, b);
    if (s->has_next) {
        s->clk_seg = s->next;
        s->has_next = 0;
    } else if ((pat = cv_read_pattern(s)) != NULL) {
        cv_seg_set(&s->clk_seg, pat);
    } else if (s->dev->protocol.read.kind == CVR_POINTER) {
        cv_read_begin(s);
        s->clk_seg = s->seg;
    } else if (s->dev->protocol.read.kind == CVR_MAP && cv_reg_at(s, 0, s->read_key) != CV_NONE) {
        /* an address the map does not list: the register at that address (RTC RAM) */
        static const CvTok AT = {CVT_AT, 0, 0, 0};
        static const CvPattern AT_PATTERN = {&AT, 1, 0, 1};

        cv_seg_set(&s->clk_seg, &AT_PATTERN);
        s->clk_seg.at[0] = cv_reg_at(s, 0, s->read_key);
        s->clk_seg.at_count = 1;
    } else {
        s->clk_seg.active = 0;
    }
    s->read_noted = 0;
    shift_word(s, b, &s->clk_seg, '0', delay);
}

static void par_write_done(CvState *s, const CvBus *b) {
    unsigned abits, i;
    int ok;
    uint32_t a = address_bits(s, b, &abits, &ok);

    cv_xfer_begin(s, 0);
    for (i = abits; i-- > 0;) {
        cv_bit_in(s, ok ? (((a >> i) & 1u) ? LOGIC_1 : LOGIC_0) : LOGIC_X);
    }
    for (i = 0; i < b->data_count; i++) {
        cv_bit_in(s, cv_level(s, cv_pin(s, b->data[i]), LOGIC_X));
    }
    cv_xfer_end(s);
    cv_ready_refresh(s);
}

static uint32_t data_value(const CvState *s, const CvBus *b, int *known) {
    uint32_t v = 0;
    size_t i;

    *known = 1;
    for (i = 0; i < b->data_count; i++) {
        LogicValue l = cv_level(s, cv_pin(s, b->data[i]), LOGIC_X);

        if (l == LOGIC_X) {
            *known = 0;
        }
        v = (v << 1) | (l == LOGIC_1 ? 1u : 0u);
    }
    return v;
}

static int is_bus_pin(const CvState *s, const CvBus *b, size_t pin) {
    size_t i;

    for (i = 0; i < b->data_count; i++) {
        if (cv_pin(s, b->data[i]) == pin) {
            return 1;
        }
    }
    for (i = 0; i < b->address_count; i++) {
        if (cv_pin(s, b->address[i]) == pin) {
            return 1;
        }
    }
    return (b->has_pcs && cv_pin(s, b->pcs.pin) == pin) || (b->has_rd && cv_pin(s, b->rd.pin) == pin) ||
           (b->has_wr && cv_pin(s, b->wr.pin) == pin) || (b->has_rw && cv_pin(s, b->rw.pin) == pin) ||
           (b->has_byte && cv_pin(s, b->byte_sel.pin) == pin) ||
           (b->has_ale && cv_pin(s, b->ale.pin) == pin);
}

static void par_pin(CvState *s, const CvBus *b, size_t pin) {
    int reading, writing;

    if (!is_bus_pin(s, b, pin)) {
        return;
    }
    if (b->has_ale && cv_pin(s, b->ale.pin) == pin) {
        LogicValue v = cv_level(s, pin, LOGIC_X);

        if ((b->ale.fall && v == LOGIC_0) || (!b->ale.fall && v == LOGIC_1)) {
            int known;
            uint32_t d = data_value(s, b, &known);

            s->par_latched = d & (b->ale.bits >= 32 ? 0xFFFFFFFFu : ((1u << b->ale.bits) - 1u));
            s->par_has_latched = known;
        }
        return;
    }
    if (!b->has_pcs && !b->has_wr && !b->has_rw && !b->has_rd) {
        /* an unlatched DAC: every change of the data pins is a write */
        int known;
        uint32_t d = data_value(s, b, &known);

        if (known && d != s->par_data_last) {
            s->par_data_last = d;
            par_write_done(s, b);
        }
        return;
    }
    reading = par_read_access(s, b);
    writing = par_write_access(s, b);
    if (reading && !s->par_reading) {
        s->par_reading = 1;
        par_read_drive(s, b, s->dev->timing.t_acc);
    } else if (reading) {
        /* the address or byte select changed during the read */
        uint32_t old = s->read_key;

        read_key(s, b);
        if (s->read_key != old) {
            par_read_drive(s, b, s->dev->timing.t_acc);
        }
    } else if (s->par_reading) {
        s->par_reading = 0;
        release_data(s, b, s->dev->timing.t_df);
        cv_reads_done(s);
        if (s->dev->conversion.starts & CVS_READ) {
            cv_start_conversion(s, -1);
        }
    }
    if (writing && !s->par_writing) {
        s->par_writing = 1;
    } else if (!writing && s->par_writing) {
        s->par_writing = 0;
        par_write_done(s, b);
    }
}

/* ---- clocked ----------------------------------------------------------------------------- */

static void clock_outs(CvState *s, const CvBus *b, LogicValue clk) {
    size_t i;

    for (i = 0; i < b->clock_out_count; i++) {
        const CvClockOut *o = &b->clock_out[i];
        LogicValue v;

        if (clk == LOGIC_X) {
            v = LOGIC_X;
        } else if (o->divide <= 1) {
            v = clk;
        } else {
            unsigned half = o->divide / 2 ? o->divide / 2 : 1;

            v = ((s->clk_count / half) & 1u) ? LOGIC_1 : LOGIC_0;
        }
        if (o->invert && v != LOGIC_X) {
            v = v == LOGIC_1 ? LOGIC_0 : LOGIC_1;
        }
        cv_drive(s, cv_pin(s, o->pin), v, s->dev->timing.t_out);
    }
}

static void clocked_pin(CvState *s, const CvBus *b, size_t pin) {
    if (b->has_oe && cv_pin(s, b->oe.pin) == pin) {
        if (!active(s, &b->oe)) {
            release_data(s, b, s->dev->timing.t_df);
        }
        return;
    }
    if (cv_pin(s, b->clock) == pin) {
        LogicValue v = cv_level(s, pin, LOGIC_X);
        int edge;

        if (v == s->par_clock) {
            return;
        }
        edge = (b->clock_edge == 2 && v != LOGIC_X) || (b->clock_edge == 1 && v == LOGIC_1) ||
               (b->clock_edge == 0 && v == LOGIC_0);
        s->par_clock = v;
        if (v == LOGIC_1) {
            s->clk_count++;
        }
        clock_outs(s, b, v);
        if (!edge || s->powered_down || s->in_reset) {
            return;
        }
        if (!s->clk_seg.active) {
            cv_seg_set(&s->clk_seg, cv_read_pattern(s));
        }
        if (!b->has_oe || active(s, &b->oe)) {
            shift_word(s, b, &s->clk_seg, 'r', s->dev->timing.t_out);
        } else {
            /* the word still advances while the outputs are off */
            LogicValue dummy;
            size_t i;
            int sdio;

            for (i = 0; i < b->data_count && s->clk_seg.active; i++) {
                dummy = cv_seg_bit(s, &s->clk_seg, 'r', &sdio);
                (void)dummy;
            }
        }
    }
}

/* ---- hooks --------------------------------------------------------------------------------- */

void cv_par_start(CvState *s) {
    const CvBus *b = &s->dev->bus;

    if (b->style == CVB_PARALLEL) {
        int known;

        release_data(s, b, 0);
        s->par_data_last = data_value(s, b, &known);
        s->par_reading = par_read_access(s, b);
        s->par_writing = par_write_access(s, b);
        if (s->par_reading) {
            par_read_drive(s, b, s->dev->timing.t_acc);
        }
    } else if (b->style == CVB_CLOCKED) {
        s->par_clock = cv_level(s, cv_pin(s, b->clock), LOGIC_X);
        cv_seg_set(&s->clk_seg, cv_read_pattern(s));
        release_data(s, b, 0);
        clock_outs(s, b, s->par_clock);
    }
}

void cv_par_pin(CvState *s, size_t pin) {
    const CvBus *b = &s->dev->bus;

    if (b->style == CVB_PARALLEL) {
        par_pin(s, b, pin);
    } else if (b->style == CVB_CLOCKED) {
        clocked_pin(s, b, pin);
    }
}

void cv_par_timer(CvState *s, uint32_t id) {
    (void)s;
    (void)id;
}
