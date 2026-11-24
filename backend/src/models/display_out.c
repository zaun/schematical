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
#include "display_clock.h"

typedef struct {
    unsigned slot;
    uint32_t frame;
    struct ClockState *clk;     /* clock driver */
    /* ports: what each pin was last driven with (value; weak) */
    LogicValue *port_value;
    uint8_t *port_weak;
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

/* ASCII16: the 64-character 16-segment font of the ICM7244/7245 (datasheet Figure 9), indexed by the 6-bit
   code D5..D0 (= ASCII & 0x3F); bit 0 a1, 1 a2, 2 b, 3 c, 4 d1, 5 d2, 6 e, 7 f, 8 g1, 9 g2, 10 h, 11 i,
   12 j, 13 k, 14 l, 15 m, 16 DP. */
static const uint32_t ASCII16_FONT[64] = {
    0x00AF7, 0x003CF, 0x04A3F, 0x000F3, 0x0483F, 0x003F3, 0x001C3, 0x002FB,
    0x003CC, 0x04800, 0x0007C, 0x031C0, 0x000F0, 0x014CC, 0x024CC, 0x000FF,
    0x003C7, 0x020FF, 0x023C7, 0x003BB, 0x04803, 0x000FC, 0x090C0, 0x0A0CC,
    0x0B400, 0x05400, 0x09033, 0x04812, 0x02400, 0x04821, 0x09006, 0x00030,
    0x00000, 0x10800, 0x00084, 0x04B32, 0x04BBB, 0x0DB99, 0x0D83A, 0x00800,
    0x03000, 0x08400, 0x0FF00, 0x04B00, 0x08000, 0x00300, 0x10000, 0x09000,
    0x090FF, 0x04800, 0x00377, 0x0023F, 0x0038C, 0x021B3, 0x003FB, 0x0000F,
    0x003FF, 0x003BF, 0x10000, 0x08000, 0x08030, 0x00330, 0x02030, 0x04207
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
    case DEC_ASCII16:
        return ASCII16_FONT[value & 0x3Fu];
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
        if (c->decode != DEC_ASCII16 && pins > 7 && ((value >> c->dp_bit) & 1u)) {
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

static uint32_t digit_value(const CvState *s, const DispCfg *c, unsigned k);

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
                             c->driver == DISP_DIGITS ? digit_value(s, c, k) : cv_field_get(s, &c->ram[k]),
                             c->out_count);
    }
    for (i = 0; i < c->out_count; i++) {
        int lit = (pattern >> i) & 1u;

        put(s, c->outs[i], level(lit ? c->out_active : !c->out_active));
    }
}

/* The value shown for digit k now: its ram field, or its second plane in the second half of a blink
   period. */
static uint32_t digit_value(const CvState *s, const DispCfg *c, unsigned k) {
    const CvField *f = &c->ram[k];

    if (c->has_blink && holds(s, &c->blink_enable)) {
        uint64_t half = (uint64_t)cv_choice(s, &c->blink_half) * 1000000ull;

        if (half > 0 && ((uint64_t)sim_now(s->sim) / half) % 2 == 1) {
            f = &c->ram1[k];
        }
    }
    return cv_field_get(s, f);
}

/* digits with shared digit / data pins: slot k pulls shared pin k to the digit level and puts digit
   k's segments on the others (lit: the lit level, unlit: released). Shutdown releases every pin. */
static void apply_shared(CvState *s, const DispCfg *c, unsigned k) {
    int shut = holds(s, &c->shutdown);
    uint32_t pattern = 0;
    size_t q;

    if (!shut) {
        if (holds(s, &c->test)) {
            pattern = 0xFFu;
        } else if (!holds(s, &c->blank)) {
            pattern = pattern_of(c, is_decoded(s, c, k, NULL), digit_value(s, c, k), 8);
        }
    }
    for (q = 0; q < c->line_count; q++) {
        LogicValue v = LOGIC_Z;

        if (!shut && q == k) {
            v = level(c->line_active);
        } else if (!shut) {
            size_t j = q > k ? q - 1 : q;

            if ((pattern >> c->seg_of_line[j]) & 1u) {
                v = level(c->out_active);
            }
        }
        put(s, c->lines[q], v);
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

static int in_list(const uint32_t *list, size_t n, uint32_t v) {
    size_t k;

    for (k = 0; k < n; k++) {
        if (list[k] == v) {
            return 1;
        }
    }
    return 0;
}

/* Drives a port pin (strong or weak) unless it already is. */
static void drive_port(CvState *s, DispState *st, size_t i, int name, LogicValue v, int weak) {
    size_t pin = cv_pin(s, name);

    if (pin == SIZE_MAX || (st->port_value[i] == v && st->port_weak[i] == (uint8_t)weak)) {
        return;
    }
    st->port_value[i] = v;
    st->port_weak[i] = (uint8_t)weak;
    if (v == LOGIC_1 && s->b->open_drain[pin]) {
        v = LOGIC_Z;
    }
    s->drv[pin] = v;
    sim_schedule_inertial_drive(s->sim, s->inst, pin, v, weak ? DRIVE_WEAK : DRIVE_STRONG, 0);
}

static void put_field(CvState *s, const CvField *f, uint32_t value) {
    uint32_t m = (f->hi - f->lo + 1 >= 32 ? 0xFFFFFFFFu : ((1u << (f->hi - f->lo + 1)) - 1u)) << f->lo;

    s->regs[f->reg] = (s->regs[f->reg] & ~m) | ((value << f->lo) & m);
}

static void apply_ports(CvState *s, const DispCfg *c) {
    DispState *st = s->disp;
    int shut = holds(s, &c->shutdown), test = holds(s, &c->test);
    size_t i;

    for (i = 0; i < c->port_count; i++) {
        const DispPort *p = &c->ports[i];
        uint32_t d = p->has_dir ? cv_field_get(s, &p->dir) : 0;
        int data = cv_field_get(s, &p->data) != 0;
        int sink = p->has_dir && in_list(p->sinks, p->sink_count, d);
        int output = !p->has_dir || in_list(p->outputs, p->output_count, d);
        int pullup = p->has_dir && in_list(p->pullups, p->pullup_count, d);
        LogicValue v = LOGIC_Z;
        int weak = 0;

        if (sink) {
            v = test ? LOGIC_0 : (shut ? LOGIC_Z : (data ? LOGIC_0 : LOGIC_Z));
        } else if (!shut && output) {
            v = level(data != (p->invert != 0));
        } else if (!shut && pullup) {
            v = LOGIC_1;
            weak = 1;
        }
        drive_port(s, st, i, p->pin, v, weak);
        if (p->has_read) {
            put_field(s, &p->read, (sink || output) ? (uint32_t)data
                                                     : (uint32_t)(cv_level(s, cv_pin(s, p->pin), LOGIC_0) == LOGIC_1));
        }
    }
}

/* The lcd run-time mode now: its index into the mode tables (0 without a mode field). */
static unsigned lcd_mode(const CvState *s, const DispCfg *c) {
    uint32_t v;
    unsigned i;

    if (!c->mode_dynamic) {
        return 0;
    }
    v = cv_field_get(s, &c->mode_field);
    for (i = 0; i < c->mode_count; i++) {
        if (c->mode_key[i] == v) {
            return i;
        }
    }
    return 0;
}

/* The slots of one frame: a backplane each, or the mode's. */
static unsigned lcd_slots(const CvState *s, const DispCfg *c) {
    return c->mode_dynamic ? c->mode_slots[lcd_mode(s, c)] : c->bp_count;
}

static void apply_lcd(CvState *s, const DispCfg *c, unsigned k, uint32_t frame) {
    int ac = (int)(frame & 1u);
    unsigned b, g, mode = lcd_mode(s, c), slots = lcd_slots(s, c), row;
    unsigned mask;

    if (c->enable.count > 0 && !cv_conds_hold(s, &c->enable)) {
        for (b = 0; b < c->bp_count; b++) {
            put(s, c->lines[b], LOGIC_Z);
        }
        for (g = 0; g < c->segments; g++) {
            put(s, c->outs[g], LOGIC_Z);
        }
        return;
    }
    k %= slots;
    mask = c->mode_dynamic ? c->slot_mask[mode][k] : 1u << k;
    row = k;
    if (c->has_out_bank && slots <= 2 && cv_field_get(s, &c->out_bank) != 0) {
        row += 2;
    }
    for (b = 0; b < c->bp_count; b++) {
        put(s, c->lines[b], (mask >> b) & 1u ? level(ac) : LOGIC_Z);
    }
    for (g = 0; g < c->segments; g++) {
        size_t idx = c->bp_major ? (size_t)row * c->segments + g : (size_t)g * c->bp_count + row;
        int bit = 0;

        if (row < c->bp_count) {
            bit = (int)((cv_field_get(s, &c->ram[c->bit_field[idx]]) >> c->bit_pos[idx]) & 1u);
        }
        put(s, c->outs[g], level(bit ^ ac));
    }
}

/* clock: the lcd slot waveforms with the clock's lit cells in place of a RAM. */
static void apply_clock(CvState *s, const DispCfg *c, unsigned k, uint32_t frame) {
    const DispState *st = s->disp;
    int ac = (int)(frame & 1u);
    unsigned b, g;

    k %= c->bp_count;
    for (b = 0; b < c->bp_count; b++) {
        put(s, c->lines[b], b == k ? level(ac) : LOGIC_Z);
    }
    for (g = 0; g < c->segments; g++) {
        put(s, c->outs[g], level(clock_lit(s, c->clock, st->clk, g, k) ^ ac));
    }
}

/* The time one slot lasts now (an lcd's depends on the mode). */
static uint64_t slot_ps_of(const CvState *s, const DispCfg *c) {
    if (c->driver == DISP_LCD && c->mode_dynamic) {
        uint64_t ps = c->frame_ps / lcd_slots(s, c);

        return ps > 0 ? ps : 1;
    }
    return c->slot_ps;
}

static unsigned slots_of(const DispCfg *c) {
    return c->driver == DISP_LCD || c->driver == DISP_CLOCK ? c->bp_count : c->shared ? c->shared_digits : (unsigned)c->line_count;
}

/* The slots scanned now: all, or the scan limit's field + 1 digits (a test scans every digit). */
static unsigned active_slots(const CvState *s, const DispCfg *c) {
    unsigned n = c->driver == DISP_LCD ? lcd_slots(s, c) : slots_of(c);

    if (c->has_limit && !holds(s, &c->test)) {
        uint32_t lim = cv_field_get(s, &c->limit);

        if (lim + 1u < n) {
            n = lim + 1u;
        }
    }
    return n;
}

static int scans(const DispCfg *c) {
    return c->driver == DISP_DIGITS || c->driver == DISP_MATRIX || c->driver == DISP_LCD ||
           c->driver == DISP_CLOCK;
}

static void apply(CvState *s) {
    const DispCfg *c = s->dev->disp;
    DispState *st = s->disp;

    switch (c->driver) {
    case DISP_DIGITS:
    case DISP_MATRIX:
        if (st->slot >= active_slots(s, c)) {
            st->slot = 0;
        }
        if (c->shared) {
            apply_shared(s, c, st->slot);
        } else {
            apply_scan(s, c, st->slot);
        }
        break;
    case DISP_STATIC:
        apply_static(s, c);
        break;
    case DISP_PORTS:
        apply_ports(s, c);
        break;
    case DISP_CLOCK:
        if (st->slot >= c->bp_count) {
            st->slot = 0;
        }
        apply_clock(s, c, st->slot, st->frame);
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
    st->port_value = calloc(c->port_count + 1, sizeof *st->port_value);
    st->port_weak = calloc(c->port_count + 1, 1);
    if (st->port_value == NULL || st->port_weak == NULL) {
        free(st->port_value);
        free(st->port_weak);
        free(st);
        return;
    }
    {
        size_t i;

        for (i = 0; i < c->port_count; i++) {
            st->port_value[i] = LOGIC_X;    /* never driven yet: the first apply drives */
        }
    }
    s->disp = st;
    if (c->driver == DISP_CLOCK) {
        st->clk = clock_new(s, c->clock);
        if (st->clk == NULL) {
            s->disp = NULL;
            free(st->port_value);
            free(st->port_weak);
            free(st);
            return;
        }
    }
    apply(s);
    if (scans(c)) {
        sim_schedule_timer(s->sim, s->inst, CV_TM_DISP, slot_ps_of(s, c));
    }
}

void disp_timer(CvState *s, uint32_t id) {
    const DispCfg *c = s->dev->disp;
    DispState *st = s->disp;

    if (st == NULL || !scans(c)) {
        return;
    }
    if (st->clk != NULL && ((id >> 24) & 0xFu) != 0) {
        if (clock_timer(s, c->clock, st->clk, id)) {
            apply(s);
        }
        return;
    }
    if (++st->slot >= active_slots(s, c)) {
        st->slot = 0;
        st->frame++;
    }
    apply(s);
    sim_schedule_timer(s->sim, s->inst, CV_TM_DISP, slot_ps_of(s, c));
}

void disp_refresh(CvState *s) {
    if (s->disp != NULL) {
        apply(s);
    }
}

/* A pin changed: the clock driver reads its buttons and straps. */
void disp_pin(CvState *s, size_t pin) {
    DispState *st = s->disp;

    if (st != NULL && st->clk != NULL) {
        clock_pin(s, s->dev->disp->clock, st->clk, pin);
        apply(s);
    }
}

void disp_free(CvState *s) {
    DispState *st = s->disp;

    if (st != NULL) {
        clock_free(st->clk);
        free(st->port_value);
        free(st->port_weak);
    }
    free(s->disp);
    s->disp = NULL;
}

static void field_set(CvState *s, const CvField *f, uint32_t value) {
    uint32_t m = (f->hi - f->lo + 1 >= 32 ? 0xFFFFFFFFu : ((1u << (f->hi - f->lo + 1)) - 1u)) << f->lo;

    s->regs[f->reg] = (s->regs[f->reg] & ~m) | ((value << f->lo) & m);
}

/* A display data byte for the lcd's RAM (plan-56-E, PCF8576 family). In a mode of n slots the byte's
   bits, most significant first, go in groups of n down a column: bit j to RAM column pointer + j / n,
   row j % n (rows 2.. in an input bank of 1, in a mode of at most 2 slots). They are stored only
   while the subaddress counter equals the hardware subaddress; a bit past the last column falls into
   the next device (counter + 1, column 0...), which is this one only when that counter value is the
   hardware subaddress. Then the pointer steps by the mode's increment; past the last column it wraps
   and the counter increments. */
void disp_lcd_byte(CvState *s, uint8_t byte) {
    const DispCfg *c = s->dev->disp;
    unsigned mode, rows, offset = 0, j;
    uint32_t ptr, sub = 0, hw = 0;

    if (c == NULL || c->driver != DISP_LCD || !c->has_load) {
        return;
    }
    mode = lcd_mode(s, c);
    rows = c->mode_slots[mode];
    if (c->has_in_bank && rows <= 2 && cv_field_get(s, &c->ld_bank) != 0) {
        offset = 2;
    }
    ptr = cv_field_get(s, &c->ld_pointer);
    if (c->has_sub) {
        sub = cv_field_get(s, &c->ld_sub);
        hw = cv_field_get(s, &c->ld_hw);
    }
    for (j = 0; j < 8; j++) {
        uint32_t col = ptr + j / rows, owner = sub;
        unsigned row = j % rows + offset;
        int bit = (byte >> (7 - j)) & 1;

        if (col >= c->segments && !c->has_sub) {
            continue;                   /* a single device: the bits past the end are dropped */
        }
        while (col >= c->segments) {
            col -= c->segments;
            owner = (owner + 1) % c->ld_subs;
        }
        if (row >= c->bp_count || (c->has_sub && owner != hw)) {
            continue;
        }
        {
            size_t idx = (size_t)col * c->bp_count + row;
            const CvField *f = &c->ram[c->bit_field[idx]];
            uint32_t m = 1u << (f->lo + c->bit_pos[idx]);

            s->regs[f->reg] = (s->regs[f->reg] & ~m) | (bit ? m : 0);
        }
    }
    ptr += c->mode_step[mode];
    if (ptr >= c->segments) {
        ptr -= c->segments;
        if (c->has_sub) {
            sub = (sub + 1) % c->ld_subs;
            field_set(s, &c->ld_sub, sub);
        }
    }
    field_set(s, &c->ld_pointer, ptr);
    disp_refresh(s);
}
