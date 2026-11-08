/*
 * mux_stored: the mux kind's models with storage (plan-21-C):
 *
 *   stored     selectors with data latches or registers, a select latch or
 *              an output register ('298, '399, '354, '355, '356, LS604)
 *   regfile    register files with independent write and read addresses
 *              ('670 4 x 4; up to two files of 16 words x 8 bits)
 *
 * The '857 (universal) and ABT16460 (bus4) models of plan-21-C have no
 * addable TI part, so they are not built (plan-21-C Corrections).
 *
 * Stored bits are 0 at power-on. Signal rules: docs/parts-format.md "mux".
 */

#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "model_pins.h"
#include "mux.h"
#include "seq.h"

#define RF_MAX_FILES 2
#define RF_MAX_ADDRESS 4
#define RF_MAX_BITS 8

/* hold (1 asserted, 0 not, -1 unknown) applied to a stored value. */
static LogicValue latch_value(int hold, LogicValue stored, LogicValue in) {
    if (hold == 0) {
        return in;
    }
    if (hold < 0 && stored != in) {
        return LOGIC_X;
    }
    return stored;
}

/* 1 when the change on pin is the clock's edge; -1 when it moved to X or Z
   (the stored bits it could load become X); 0 otherwise. */
static int clock_event(const SeqClock *c, size_t pin, LogicValue old_value, LogicValue new_value) {
    if (c->pin == SEQ_NO_PIN || pin != c->pin) {
        return 0;
    }
    if (seq_clock_edge(c, old_value, new_value)) {
        return 1;
    }
    if ((new_value == LOGIC_X || new_value == LOGIC_Z) &&
        old_value == (c->falling ? LOGIC_1 : LOGIC_0)) {
        return -1;
    }
    return 0;
}

/* Every control asserted: 1; one deasserted: 0; else -1. No controls: 1. */
static int all_enabled(const Sim *sim, const SimInstance *inst, const SeqControl *c, size_t n) {
    int result = 1;
    size_t i;

    for (i = 0; i < n; i++) {
        int level = seq_enable(sim, inst, &c[i]);

        if (level == 0) {
            return 0;
        }
        if (level < 0) {
            result = -1;
        }
    }
    return result;
}

/* ---- stored ---------------------------------------------------------------- */

#define ST_MAX_OES 3

static const char *const STORED_KEYS[] = {
    "units", "disabled", "invert", "output", "selectLatch", "dataLatch", "dataClock",
    "outputClock", "oes", NULL
};

typedef struct {
    MuxSelectNames sel;
    SeqControlName select_latch, data_latch;
    SeqClockName data_clock, output_clock;
    SeqControlName oes[ST_MAX_OES];
    size_t oe_count;
} StDevice;

typedef struct {
    MuxSelectBound sel;
    SeqControl select_latch, data_latch;   /* asserted = hold */
    SeqClock data_clock, output_clock;
    SeqControl oes[ST_MAX_OES];
    size_t oe_count;
} StBound;

typedef struct {
    MuxHeader h;
    const StBound *b;
    LogicValue data[MUX_MAX_UNITS][MUX_MAX_DATA];
    LogicValue sel[MUX_MAX_UNITS][MUX_MAX_SELECT];
    LogicValue reg[MUX_MAX_UNITS];
    MuxUnitOut out[MUX_MAX_UNITS];
} StState;

static void *st_load(KindContext *ctx, const cJSON *json, unsigned *units) {
    StDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const cJSON *oes;
    int storage;

    if (dev == NULL || mux_load_select(ctx, json, &dev->sel, units) != 0 ||
        seq_control_name(ctx, json, "selectLatch", 0, 0, &dev->select_latch) != 0 ||
        seq_control_name(ctx, json, "dataLatch", 0, 0, &dev->data_latch) != 0 ||
        seq_clock_name(ctx, json, "dataClock", 0, &dev->data_clock) != 0 ||
        seq_clock_name(ctx, json, "outputClock", 0, &dev->output_clock) != 0) {
        return NULL;
    }
    if (dev->data_latch.pin != NULL && dev->data_clock.pin != NULL) {
        kind_error(ctx, "a device cannot have both \"dataLatch\" and \"dataClock\"");
        return NULL;
    }
    storage = dev->select_latch.pin != NULL || dev->data_latch.pin != NULL ||
              dev->data_clock.pin != NULL || dev->output_clock.pin != NULL;
    if (!storage) {
        kind_error(ctx, "model \"stored\" needs \"selectLatch\", \"dataLatch\", \"dataClock\" "
                   "or \"outputClock\"");
        return NULL;
    }
    oes = cJSON_GetObjectItemCaseSensitive(json, "oes");
    dev->oe_count = 0;
    if (oes != NULL) {
        const cJSON *e;

        if (!cJSON_IsArray(oes) || cJSON_GetArraySize(oes) < 1 ||
            cJSON_GetArraySize(oes) > ST_MAX_OES) {
            kind_error(ctx, "\"oes\" must be an array of 1 to %d controls", ST_MAX_OES);
            return NULL;
        }
        cJSON_ArrayForEach(e, oes) {
            kind_push(ctx, "oes[%zu]", dev->oe_count);
            if (seq_control_object(ctx, e, 0, &dev->oes[dev->oe_count]) != 0) {
                return NULL;
            }
            kind_pop(ctx);
            dev->oe_count++;
        }
    }
    return dev;
}

static void *st_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const StDevice *dev = raw;
    StBound *b = kind_alloc(ctx, sizeof(*b));
    size_t i;

    if (b == NULL || mux_bind_select(ctx, &dev->sel, pins, count, &b->sel) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->select_latch, &b->select_latch) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->data_latch, &b->data_latch) != 0 ||
        seq_bind_clock(ctx, pins, count, &dev->data_clock, &b->data_clock) != 0 ||
        seq_bind_clock(ctx, pins, count, &dev->output_clock, &b->output_clock) != 0) {
        return NULL;
    }
    for (i = 0; i < dev->oe_count; i++) {
        if (seq_bind_control(ctx, pins, count, &dev->oes[i], &b->oes[i]) != 0) {
            return NULL;
        }
    }
    b->oe_count = dev->oe_count;
    return b;
}

static MuxHeader *st_create(const void *bound) {
    const StBound *b = bound;
    StState *s = calloc(1, sizeof(*s));
    size_t i, j;

    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    for (i = 0; i < MUX_MAX_UNITS; i++) {
        for (j = 0; j < MUX_MAX_DATA; j++) {
            s->data[i][j] = LOGIC_0;
        }
        for (j = 0; j < MUX_MAX_SELECT; j++) {
            s->sel[i][j] = LOGIC_0;
        }
        s->reg[i] = LOGIC_0;
    }
    mux_init_select_out(&b->sel, s->out);
    return &s->h;
}

static void st_eval(Sim *sim, SimInstance *inst, MuxHeader *h, size_t pin,
                    LogicValue old_value, LogicValue new_value) {
    StState *s = (StState *)h;
    const StBound *b = s->b;
    const MuxSelectBound *sb = &b->sel;
    int start = pin == SEQ_NO_PIN;
    /* Latches hold while asserted; at start they hold their power-on 0. */
    int sel_hold = b->select_latch.pin == SEQ_NO_PIN ? 0
                   : start ? 1 : seq_sync(sim, inst, &b->select_latch);
    int data_hold = b->data_latch.pin == SEQ_NO_PIN ? 0
                    : start ? 1 : seq_sync(sim, inst, &b->data_latch);
    int data_edge = clock_event(&b->data_clock, pin, old_value, new_value);
    int out_edge = clock_event(&b->output_clock, pin, old_value, new_value);
    int shared = all_enabled(sim, inst, b->oes, b->oe_count);
    size_t i, j;

    for (i = 0; i < sb->unit_count; i++) {
        const MuxUnit *u = &sb->units[i];
        int sel[MUX_MAX_SELECT];
        LogicValue v;

        for (j = 0; j < u->data_count; j++) {
            LogicValue in = seq_value(seq_read(sim, inst, u->data[j]));

            if (b->data_clock.pin != SEQ_NO_PIN) {
                s->data[i][j] = latch_value(data_edge > 0 ? 0 : data_edge < 0 ? -1 : 1,
                                            s->data[i][j], in);
            } else {
                s->data[i][j] = latch_value(data_hold, s->data[i][j], in);
            }
        }
        for (j = 0; j < u->select_count; j++) {
            s->sel[i][j] = latch_value(sel_hold, s->sel[i][j],
                                       seq_value(seq_read(sim, inst, u->select[j])));
            sel[j] = seq_level(s->sel[i][j]);
        }
        v = mux_choose(s->data[i], sel, u->select_count);
        if (b->output_clock.pin != SEQ_NO_PIN) {
            s->reg[i] = latch_value(out_edge > 0 ? 0 : out_edge < 0 ? -1 : 1, s->reg[i], v);
            v = s->reg[i];
        }
        mux_drive_unit(sim, inst, sb, i, &s->out[i], v, shared, h->timing);
    }
}

static int st_open_drain(const void *bound, size_t pin) {
    return mux_select_open_drain(&((const StBound *)bound)->sel, pin);
}

const MuxModel MUX_STORED = {
    "stored", STORED_KEYS, st_load, st_bind, st_create, st_eval, st_open_drain
};

/* ---- regfile --------------------------------------------------------------- */

static const char *const REGFILE_KEYS[] = {"words", "bits", "files", "output", NULL};

typedef struct {
    const char *const *d, *const *wa, *const *ra, *const *q;
    size_t address_count;
    SeqControlName we, re;
} RfFileNames;

typedef struct {
    RfFileNames files[RF_MAX_FILES];
    size_t file_count, bits;
    SeqOutputStyle style;
} RfDevice;

typedef struct {
    size_t d[RF_MAX_BITS], wa[RF_MAX_ADDRESS], ra[RF_MAX_ADDRESS], q[RF_MAX_BITS];
    SeqControl we, re;
} RfFile;

typedef struct {
    RfFile files[RF_MAX_FILES];
    size_t file_count, bits, address_count;
    SeqOutputStyle style;
} RfBound;

typedef struct {
    MuxHeader h;
    const RfBound *b;
    LogicValue word[RF_MAX_FILES][1u << RF_MAX_ADDRESS][RF_MAX_BITS];
    SeqOutput q[RF_MAX_FILES][RF_MAX_BITS];
} RfState;

static int rf_names(KindContext *ctx, const cJSON *item, const char *key, size_t want,
                    const char *const **names) {
    size_t n;

    if (seq_names(ctx, item, key, want, want, names, &n) != 0) {
        return -1;
    }
    return 0;
}

static void *rf_load(KindContext *ctx, const cJSON *json, unsigned *units) {
    static const char *const FILE_KEYS[] = {"d", "wa", "we", "ra", "re", "q", NULL};
    RfDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const cJSON *files, *item;
    uint64_t words, bits;
    size_t address = 0, n = 0;

    if (dev == NULL || kind_uint(ctx, json, "words", 1u << RF_MAX_ADDRESS, &words) != 0 ||
        kind_uint(ctx, json, "bits", RF_MAX_BITS, &bits) != 0 ||
        seq_output_style(ctx, json, &dev->style) != 0) {
        return NULL;
    }
    while (address < RF_MAX_ADDRESS && (1u << address) < words) {
        address++;
    }
    if (words < 2 || (1u << address) != words || bits < 1) {
        kind_error(ctx, "\"words\" must be 2, 4, 8 or 16 and \"bits\" 1 to %d", RF_MAX_BITS);
        return NULL;
    }
    files = kind_member(ctx, json, "files", 1);
    if (files == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(files) || cJSON_GetArraySize(files) < 1 ||
        cJSON_GetArraySize(files) > RF_MAX_FILES) {
        kind_error(ctx, "\"files\" must be an array of 1 to %d files", RF_MAX_FILES);
        return NULL;
    }
    cJSON_ArrayForEach(item, files) {
        RfFileNames *f = &dev->files[n];

        kind_push(ctx, "files[%zu]", n);
        if (kind_check_keys(ctx, item, FILE_KEYS, NULL) != 0 ||
            rf_names(ctx, item, "d", bits, &f->d) != 0 ||
            rf_names(ctx, item, "q", bits, &f->q) != 0 ||
            rf_names(ctx, item, "wa", address, &f->wa) != 0 ||
            rf_names(ctx, item, "ra", address, &f->ra) != 0 ||
            seq_control_name(ctx, item, "we", 1, 0, &f->we) != 0 ||
            seq_control_name(ctx, item, "re", 1, 0, &f->re) != 0) {
            return NULL;
        }
        f->address_count = address;
        kind_pop(ctx);
        n++;
    }
    dev->file_count = n;
    dev->bits = bits;
    *units = (unsigned)n;
    return dev;
}

static void *rf_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const RfDevice *dev = raw;
    RfBound *b = kind_alloc(ctx, sizeof(*b));
    size_t i;

    if (b == NULL) {
        return NULL;
    }
    for (i = 0; i < dev->file_count; i++) {
        const RfFileNames *n = &dev->files[i];
        RfFile *f = &b->files[i];

        kind_push(ctx, "files[%zu]", i);
        if (mux_bind_names(ctx, pins, count, n->d, dev->bits, f->d) != 0 ||
            mux_bind_names(ctx, pins, count, n->q, dev->bits, f->q) != 0 ||
            mux_bind_names(ctx, pins, count, n->wa, n->address_count, f->wa) != 0 ||
            mux_bind_names(ctx, pins, count, n->ra, n->address_count, f->ra) != 0 ||
            seq_bind_control(ctx, pins, count, &n->we, &f->we) != 0 ||
            seq_bind_control(ctx, pins, count, &n->re, &f->re) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    b->file_count = dev->file_count;
    b->bits = dev->bits;
    b->address_count = dev->files[0].address_count;
    b->style = dev->style;
    return b;
}

static MuxHeader *rf_create(const void *bound) {
    const RfBound *b = bound;
    RfState *s = calloc(1, sizeof(*s));
    size_t i, w, j;

    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    for (i = 0; i < RF_MAX_FILES; i++) {
        for (w = 0; w < (1u << RF_MAX_ADDRESS); w++) {
            for (j = 0; j < RF_MAX_BITS; j++) {
                s->word[i][w][j] = LOGIC_0;
            }
        }
    }
    for (i = 0; i < b->file_count; i++) {
        mux_init_outputs(s->q[i], b->files[i].q, b->bits);
    }
    return &s->h;
}

/* Known bits and their values of the address pins. */
static void address_bits(const Sim *sim, const SimInstance *inst, const size_t *pins, size_t n,
                         unsigned *known, unsigned *value, int *sel) {
    size_t i;

    *known = 0;
    *value = 0;
    for (i = 0; i < n; i++) {
        int level = seq_read(sim, inst, pins[i]);

        sel[i] = level;
        if (level >= 0) {
            *known |= 1u << i;
            *value |= (unsigned)level << i;
        }
    }
}

static void rf_eval(Sim *sim, SimInstance *inst, MuxHeader *h, size_t pin,
                    LogicValue old_value, LogicValue new_value) {
    RfState *s = (RfState *)h;
    const RfBound *b = s->b;
    unsigned words = 1u << b->address_count;
    size_t i, j;

    (void)old_value;
    (void)new_value;
    for (i = 0; i < b->file_count; i++) {
        const RfFile *f = &b->files[i];
        int we = pin == SEQ_NO_PIN ? 0 : seq_sync(sim, inst, &f->we);
        int re = seq_enable(sim, inst, &f->re);
        int sel[RF_MAX_ADDRESS];
        unsigned known, value, w;

        /* Write: the addressed word follows D while WE is asserted; an
           unknown WE or address bit makes X the bits that could change. */
        address_bits(sim, inst, f->wa, b->address_count, &known, &value, sel);
        for (w = 0; we != 0 && w < words; w++) {
            int sure = we == 1 && known == words - 1u;

            if ((w & known) != value) {
                continue;
            }
            for (j = 0; j < b->bits; j++) {
                s->word[i][w][j] = latch_value(sure ? 0 : -1, s->word[i][w][j],
                                               seq_value(seq_read(sim, inst, f->d[j])));
            }
        }
        /* Read: combinational from the array. */
        address_bits(sim, inst, f->ra, b->address_count, &known, &value, sel);
        for (j = 0; j < b->bits; j++) {
            LogicValue column[1u << RF_MAX_ADDRESS];

            for (w = 0; w < words; w++) {
                column[w] = s->word[i][w][j];
            }
            seq_drive(sim, inst, &s->q[i][j],
                      seq_gate(mux_choose(column, sel, b->address_count), re), b->style,
                      h->timing);
        }
    }
}

static int rf_open_drain(const void *bound, size_t pin) {
    const RfBound *b = bound;
    size_t i;

    for (i = 0; b->style == SEQ_OPEN_DRAIN && i < b->file_count; i++) {
        if (mux_pin_in(pin, b->files[i].q, b->bits)) {
            return 1;
        }
    }
    return 0;
}

const MuxModel MUX_REGFILE = {
    "regfile", REGFILE_KEYS, rf_load, rf_bind, rf_create, rf_eval, rf_open_drain
};
