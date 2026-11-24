/*
 * The output drivers of the leddriver and lcddriver kinds (plan-56-B;
 * display_out.h, docs/parts-format.md § leddriver, § lcddriver). The
 * converter engine runs the bus and the registers; this file turns the
 * registers into pin levels. Nothing is stored here but the scan position:
 * every output is computed from the registers when a slot starts and again
 * after any register or pin change (disp_refresh), so a write shows at once
 * and a reset blanks the display.
 *
 *   digits, matrix  one scan line (a digit pin, a row pin) is active in each
 *                   slot, in order, and the data pins carry that line's
 *                   bits. Shutdown: every pin inactive; else test: every
 *                   data pin lit; else blank: every data pin unlit.
 *   static          each group's pins follow its field at all times.
 *   ports           each pin is driven from its data field, or released
 *                   when its direction field says input.
 *   lcd             one backplane slot after another; the backplane of the
 *                   slot is driven with the frame's level, the other
 *                   backplanes are released, each segment pin carries
 *                   (its RAM bit for that backplane) XOR that level. The
 *                   level alternates on consecutive frames (AC), so a cell
 *                   (segment pin != backplane pin, see lcd.c) is lit exactly
 *                   in the slot of its backplane and exactly when its bit
 *                   is 1.
 */

#include "display_out.h"

#include <stdlib.h>

#include "converter_int.h"

typedef struct {
    unsigned slot;
    uint32_t frame;
} DispState;

/* ---- Decoding --------------------------------------------------------------------------- */

/* Segments a-g as bits 0-6. */
static const uint8_t HEX_FONT[16] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71
};

/* Code B: 0-9, then - E H L P and a blank. */
static const uint8_t CODEB_FONT[16] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x40, 0x79, 0x76, 0x38, 0x73, 0x00
};

/* ASCII 32-127; lower case reads as upper case, what has no 7-segment shape is blank. */
static const uint8_t ASCII_FONT[96] = {
    0x00, 0x06, 0x22, 0x00, 0x6D, 0x00, 0x00, 0x02, 0x39, 0x0F, 0x00, 0x00, 0x04, 0x40, 0x00, 0x52,
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00, 0x00, 0x00, 0x48, 0x00, 0x53,
    0x5F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x3D, 0x76, 0x30, 0x1E, 0x75, 0x38, 0x37, 0x54, 0x3F,
    0x73, 0x67, 0x50, 0x6D, 0x78, 0x3E, 0x1C, 0x2A, 0x76, 0x6E, 0x5B, 0x39, 0x64, 0x0F, 0x23, 0x08,
    0x20, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x3D, 0x76, 0x30, 0x1E, 0x75, 0x38, 0x37, 0x54, 0x3F,
    0x73, 0x67, 0x50, 0x6D, 0x78, 0x3E, 0x1C, 0x2A, 0x76, 0x6E, 0x5B, 0x39, 0x30, 0x0F, 0x01, 0x00
};

static uint32_t segments_of(DispDecode d, uint32_t value) {
    switch (d) {
    case DEC_HEX:
        return HEX_FONT[value & 0xFu];
    case DEC_CODEB:
        return CODEB_FONT[value & 0xFu];
    case DEC_BCD:
        return (value & 0xFu) <= 9 ? HEX_FONT[value & 0xFu] : 0;
    case DEC_ASCII7: {
        uint32_t c = value & 0x7Fu;

        return c < 32 ? 0 : ASCII_FONT[c - 32];
    }
    default:
        return 0;
    }
}

/* The lit data pins (bit j = pin j) of one value: decoded to segments a-g (pins 0-6) and the
   point (pin 7, from bit dp_bit), or raw: pin j follows bit out_bits[j]. */
static uint32_t pattern_of(const DispCfg *c, int decoded, uint32_t value, size_t pins) {
    uint32_t out = 0;
    size_t j;

    if (decoded) {
        out = segments_of(c->decode, value);
        if (pins > 7 && ((value >> c->dp_bit) & 1u)) {
            out |= 1u << 7;
        }
        return out;
    }
    for (j = 0; j < pins; j++) {
        if ((value >> c->out_bits[j]) & 1u) {
            out |= 1u << j;
        }
    }
    return out;
}

/* Is the digit (or group) decoded: a decode is set, and its field (own, or the driver's) says so. */
static int is_decoded(const CvState *s, const DispCfg *c, size_t index, const CvField *own) {
    const CvField *f = own;

    if (c->decode == DEC_NONE) {
        return 0;
    }
    if (f == NULL && c->has_decode_field) {
        f = &c->decode_field[c->decode_field_count > 1 ? index : 0];
    }
    return f == NULL || (cv_field_get(s, f) != 0) == c->decode_active;
}

/* ---- Pins -------------------------------------------------------------------------------- */

static LogicValue level(int on) {
    return on ? LOGIC_1 : LOGIC_0;
}

static void put(CvState *s, int name, LogicValue v) {
    cv_drive(s, cv_pin(s, name), v, 0);
}

static int holds(const CvState *s, const CvConds *c) {
    return c->count > 0 && cv_conds_hold(s, c);
}

/* ---- The drivers ------------------------------------------------------------------------ */

static void apply_scan(CvState *s, const DispCfg *c, unsigned k) {
    int shut = holds(s, &c->shutdown);
    uint32_t pattern = 0;
    size_t i;

    for (i = 0; i < c->line_count; i++) {
        int on = i == k && !shut;

        put(s, c->lines[i], level(on ? c->line_active : !c->line_active));
    }
    if (shut) {
        pattern = 0;
    } else if (holds(s, &c->test)) {
        pattern = (1u << c->out_count) - 1u;
    } else if (!holds(s, &c->blank)) {
        pattern = pattern_of(c, c->driver == DISP_DIGITS && is_decoded(s, c, k, NULL),
                             cv_field_get(s, &c->ram[k]), c->out_count);
    }
    for (i = 0; i < c->out_count; i++) {
        int lit = (pattern >> i) & 1u;

        put(s, c->outs[i], level(lit ? c->out_active : !c->out_active));
    }
}

static void apply_static(CvState *s, const DispCfg *c) {
    size_t g, j;

    for (g = 0; g < c->group_count; g++) {
        const DispGroup *gr = &c->groups[g];
        uint32_t pattern = pattern_of(c, is_decoded(s, c, g, gr->has_decode ? &gr->decode : NULL),
                                      cv_field_get(s, &gr->ram), gr->count);

        for (j = 0; j < gr->count; j++) {
            put(s, gr->pins[j], level(((pattern >> j) & 1u) ? c->out_active : !c->out_active));
        }
    }
}

static void apply_ports(CvState *s, const DispCfg *c) {
    size_t i, k;

    for (i = 0; i < c->port_count; i++) {
        const DispPort *p = &c->ports[i];
        int output = !p->has_dir;

        if (p->has_dir) {
            uint32_t d = cv_field_get(s, &p->dir);

            for (k = 0; k < p->output_count; k++) {
                output |= p->outputs[k] == d;
            }
        }
        put(s, p->pin, output ? level((cv_field_get(s, &p->data) != 0) != (p->invert != 0)) : LOGIC_Z);
    }
}

static void apply_lcd(CvState *s, const DispCfg *c, unsigned k, uint32_t frame) {
    int ac = (int)(frame & 1u);
    unsigned b, g;

    if (c->enable.count > 0 && !cv_conds_hold(s, &c->enable)) {
        for (b = 0; b < c->bp_count; b++) {
            put(s, c->lines[b], LOGIC_Z);
        }
        for (g = 0; g < c->segments; g++) {
            put(s, c->outs[g], LOGIC_Z);
        }
        return;
    }
    for (b = 0; b < c->bp_count; b++) {
        put(s, c->lines[b], b == k ? level(ac) : LOGIC_Z);
    }
    for (g = 0; g < c->segments; g++) {
        size_t idx = c->bp_major ? (size_t)k * c->segments + g : (size_t)g * c->bp_count + k;
        int bit = (int)((cv_field_get(s, &c->ram[c->bit_field[idx]]) >> c->bit_pos[idx]) & 1u);

        put(s, c->outs[g], level(bit ^ ac));
    }
}

static unsigned slots_of(const DispCfg *c) {
    return c->driver == DISP_LCD ? c->bp_count : (unsigned)c->line_count;
}

static int scans(const DispCfg *c) {
    return c->driver == DISP_DIGITS || c->driver == DISP_MATRIX || c->driver == DISP_LCD;
}

static void apply(CvState *s) {
    const DispCfg *c = s->dev->disp;
    const DispState *st = s->disp;

    switch (c->driver) {
    case DISP_DIGITS:
    case DISP_MATRIX:
        apply_scan(s, c, st->slot);
        break;
    case DISP_STATIC:
        apply_static(s, c);
        break;
    case DISP_PORTS:
        apply_ports(s, c);
        break;
    default:
        apply_lcd(s, c, st->slot, st->frame);
        break;
    }
}

/* ---- Hooks from the engine ------------------------------------------------------------------ */

void disp_start(CvState *s) {
    const DispCfg *c = s->dev->disp;
    DispState *st = calloc(1, sizeof *st);

    if (st == NULL) {
        return;
    }
    s->disp = st;
    apply(s);
    if (scans(c)) {
        sim_schedule_timer(s->sim, s->inst, CV_TM_DISP, c->slot_ps);
    }
}

void disp_timer(CvState *s, uint32_t id) {
    const DispCfg *c = s->dev->disp;
    DispState *st = s->disp;

    (void)id;
    if (st == NULL || !scans(c)) {
        return;
    }
    if (++st->slot >= slots_of(c)) {
        st->slot = 0;
        st->frame++;
    }
    apply(s);
    sim_schedule_timer(s->sim, s->inst, CV_TM_DISP, c->slot_ps);
}

void disp_refresh(CvState *s) {
    if (s->disp != NULL) {
        apply(s);
    }
}

void disp_free(CvState *s) {
    free(s->disp);
    s->disp = NULL;
}
