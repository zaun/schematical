/* models/converter_load_lcd.c: part of the model in models/converter_load.c (split by plan-68-I/J). */
#include "converter_load_int.h"
/* A mode name -> its backplane count (1-4), or 0. */
static unsigned lcd_mode_slots(const char *name) {
    static const char *const MODES[] = {"static", "1:2", "1:3", "1:4", NULL};
    unsigned i;

    for (i = 0; MODES[i] != NULL; i++) {
        if (strcmp(MODES[i], name) == 0) {
            return i + 1;
        }
    }
    return 0;
}

/* The index of the backplane line named `name` in c->lines, or -1. */
static int lcd_line_of(const Ld *ld, const DispCfg *c, const char *name) {
    size_t i;

    for (i = 0; i < c->line_count; i++) {
        if (strcmp(ld->names[c->lines[i]], name) == 0) {
            return (int)i;
        }
    }
    return -1;
}

/* "mode" as an object {"field", "values": {value: mode name}} with "slots": per mode name, one array
   per slot of the backplane pins driven in that slot (plan-56-E). */
static int load_lcd_modes(Dl *dl, const cJSON *o, DispCfg *c) {
    static const char *const MK[] = {"field", "values", NULL};
    Ld *ld = dl->ld;
    const cJSON *m = cvl_member(o, "mode"), *vals = cvl_member(m, "values"), *slots = cvl_member(o, "slots"), *e;
    char *end;
    size_t n = 0;

    if (cvl_check_keys(ld, m, MK) != 0 || cvl_dl_field_member(ld, m, "field", &c->mode_field) != 0) {
        return -1;
    }
    if (!cJSON_IsObject(vals) || cJSON_GetArraySize(vals) < 1 ||
        cJSON_GetArraySize(vals) > DISP_LCD_MODES) {
        return kind_error(ld->ctx, "\"mode\" \"values\" maps 1 to %d field values to mode names", DISP_LCD_MODES);
    }
    if (!cJSON_IsObject(slots)) {
        return kind_error(ld->ctx, "a \"mode\" field needs \"slots\": mode name -> the backplane pins of each slot");
    }
    cJSON_ArrayForEach(e, vals) {
        const cJSON *sl, *one;
        unsigned count, k = 0, j;
        unsigned long key = strtoul(e->string, &end, 10);

        if (*end != '\0' || e->string[0] == '\0' || !cJSON_IsString(e) ||
            (count = lcd_mode_slots(e->valuestring)) == 0) {
            return kind_error(ld->ctx, "\"mode\" \"values\" is field value -> \"static\", \"1:2\", \"1:3\" or \"1:4\"");
        }
        for (j = 0; j < n; j++) {
            if (c->mode_key[j] == key) {
                return kind_error(ld->ctx, "\"mode\" \"values\" names field value %lu twice", key);
            }
        }
        if (c->bp_count < count) {
            return kind_error(ld->ctx, "mode \"%s\" needs %u backplane pins, \"backplanes\" has %u",
                              e->valuestring, count, c->bp_count);
        }
        sl = cvl_member(slots, e->valuestring);
        if (!cJSON_IsArray(sl) || (unsigned)cJSON_GetArraySize(sl) != count) {
            return kind_error(ld->ctx, "\"slots\" \"%s\" is an array of %u slots", e->valuestring, count);
        }
        cJSON_ArrayForEach(one, sl) {
            const cJSON *p;
            unsigned mask = 0;

            if (!cJSON_IsArray(one) || cJSON_GetArraySize(one) < 1) {
                return kind_error(ld->ctx, "a slot of \"slots\" \"%s\" is an array of backplane pin names", e->valuestring);
            }
            cJSON_ArrayForEach(p, one) {
                int line = cJSON_IsString(p) ? lcd_line_of(ld, c, p->valuestring) : -1;

                if (line < 0) {
                    return kind_error(ld->ctx, "\"slots\" \"%s\": \"%s\" is not one of \"backplanes\"", e->valuestring,
                                      cJSON_IsString(p) ? p->valuestring : "?");
                }
                mask |= 1u << line;
            }
            c->slot_mask[n][k++] = (uint8_t)mask;
        }
        c->mode_key[n] = (uint32_t)key;
        c->mode_slots[n] = count;
        n++;
    }
    if ((size_t)cJSON_GetArraySize(slots) != n) {
        return kind_error(ld->ctx, "\"slots\" names a mode that \"mode\" \"values\" does not");
    }
    c->mode_count = (unsigned)n;
    c->mode_dynamic = 1;
    return 0;
}

/* "load": the data pointer, per-mode steps, banks and subaddress of an lcd driver (plan-56-E). */
static int load_lcd_load(Dl *dl, const cJSON *o, DispCfg *c) {
    static const char *const LK[] = {"pointer", "step", "bank", "subaddress", "hardware", "subaddresses", NULL};
    Ld *ld = dl->ld;
    const cJSON *l = cvl_member(o, "load"), *st, *e;
    unsigned i;
    uint32_t subs = 8;

    /* Only reached with "mode" as a field: load_display_lcd rejects "load" with a fixed mode. */
    kind_push(ld->ctx, "load");
    if (cvl_check_keys(ld, l, LK) != 0 || cvl_dl_field_member(ld, l, "pointer", &c->ld_pointer) != 0) {
        return -1;
    }
    st = cvl_member(l, "step");
    if (!cJSON_IsObject(st)) {
        return kind_error(ld->ctx, "\"step\" maps each mode name to the data pointer's increment per byte");
    }
    for (i = 0; i < c->mode_count; i++) {
        c->mode_step[i] = 0;
    }
    cJSON_ArrayForEach(e, st) {
        unsigned slots = lcd_mode_slots(e->string);

        for (i = 0; i < c->mode_count && c->mode_slots[i] != slots; i++) {
        }
        if (slots == 0 || i == c->mode_count || !cJSON_IsNumber(e) || e->valuedouble < 1 ||
            e->valuedouble > 64) {
            return kind_error(ld->ctx, "\"step\" is mode name -> 1 to 64, for the modes of \"mode\"");
        }
        c->mode_step[i] = (unsigned)e->valuedouble;
    }
    for (i = 0; i < c->mode_count; i++) {
        if (c->mode_step[i] == 0) {
            return kind_error(ld->ctx, "\"step\" has no entry for the mode with %u slots", c->mode_slots[i]);
        }
    }
    if (cvl_member(l, "bank") != NULL) {
        c->has_in_bank = 1;
        if (cvl_dl_field_member(ld, l, "bank", &c->ld_bank) != 0) {
            return -1;
        }
    }
    if (cvl_member(l, "subaddress") != NULL || cvl_member(l, "hardware") != NULL) {
        if (cvl_member(l, "subaddress") == NULL || cvl_member(l, "hardware") == NULL) {
            return kind_error(ld->ctx, "\"subaddress\" and \"hardware\" go together");
        }
        c->has_sub = 1;
        if (cvl_dl_field_member(ld, l, "subaddress", &c->ld_sub) != 0 ||
            cvl_dl_field_member(ld, l, "hardware", &c->ld_hw) != 0 ||
            cvl_u32_member(ld, l, "subaddresses", &subs) != 0) {
            return -1;
        }
        if (subs < 2 || subs > 256) {
            return kind_error(ld->ctx, "\"subaddresses\" is 2 to 256");
        }
    } else if (cvl_member(l, "subaddresses") != NULL) {
        return kind_error(ld->ctx, "\"subaddresses\" needs \"subaddress\"");
    }
    c->ld_subs = subs;
    c->has_load = 1;
    kind_pop(ld->ctx);
    return 0;
}

/* lcd: backplane and segment waveforms from a display RAM. */
static int load_display_lcd(Dl *dl, const cJSON *o, DispCfg *c) {
    static const char *const KEYS[] = {
        "driver", "mode", "backplanes", "segmentPins", "ram", "ramOrder", "bitOrder", "frameUs",
        "enable", "openDrain", "slots", "outputBank", "load", NULL
    };
    Ld *ld = dl->ld;
    const cJSON *mode;
    const char *s;
    uint64_t frame_ps;
    size_t i, k, bits = 0, need, idx = 0;
    int msb = 0, dynamic;
    uint16_t *bit_field;
    uint8_t *bit_pos;

    if (cvl_check_keys(ld, o, KEYS) != 0) {
        return -1;
    }
    mode = cvl_member(o, "mode");
    dynamic = cJSON_IsObject(mode);
    if (!dynamic) {
        if (kind_string(ld->ctx, o, "mode", &s) != 0) {
            return -1;
        }
        if ((c->bp_count = lcd_mode_slots(s)) == 0) {
            return kind_error(ld->ctx, "\"mode\" must be \"static\", \"1:2\", \"1:3\" or \"1:4\"");
        }
        if (cvl_member(o, "slots") != NULL || cvl_member(o, "outputBank") != NULL || cvl_member(o, "load") != NULL) {
            return kind_error(ld->ctx, "\"slots\", \"outputBank\" and \"load\" need \"mode\" as a field");
        }
    }
    if (cvl_dl_pins(dl, o, "backplanes", dynamic ? 1 : c->bp_count, dynamic ? DISP_LCD_ROWS : c->bp_count,
                &c->lines, &c->line_count) != 0 ||
        cvl_dl_pins(dl, o, "segmentPins", 1, DISP_MAX_LCD_SEGMENTS, &c->outs, &c->out_count) != 0) {
        return -1;
    }
    if (dynamic) {
        c->bp_count = (unsigned)c->line_count;
    }
    c->segments = (unsigned)c->out_count;
    s = "segment";
    if (cvl_member(o, "ramOrder") != NULL && kind_string(ld->ctx, o, "ramOrder", &s) != 0) {
        return -1;
    }
    if (strcmp(s, "backplane") != 0 && strcmp(s, "segment") != 0) {
        return kind_error(ld->ctx, "\"ramOrder\" must be \"segment\" or \"backplane\"");
    }
    c->bp_major = strcmp(s, "backplane") == 0;
    if (dynamic && c->bp_major) {
        return kind_error(ld->ctx, "\"mode\" as a field needs \"ramOrder\" \"segment\"");
    }
    s = "lsb";
    if (cvl_member(o, "bitOrder") != NULL && kind_string(ld->ctx, o, "bitOrder", &s) != 0) {
        return -1;
    }
    if (strcmp(s, "lsb") != 0 && strcmp(s, "msb") != 0) {
        return kind_error(ld->ctx, "\"bitOrder\" must be \"lsb\" or \"msb\"");
    }
    msb = strcmp(s, "msb") == 0;
    if (cvl_dl_ram(ld, o, 0, "", &c->ram, &k) != 0) {
        return -1;
    }
    c->ram_count = k;
    for (i = 0; i < k; i++) {
        bits += c->ram[i].hi - c->ram[i].lo + 1;
    }
    need = (size_t)c->segments * c->bp_count;
    if (bits < need) {
        return kind_error(ld->ctx, "\"ram\" holds %zu bits, %zu segment pins x %u backplanes need %zu",
                          bits, c->out_count, c->bp_count, need);
    }
    bit_field = kind_alloc(ld->ctx, need * sizeof(*bit_field));
    bit_pos = kind_alloc(ld->ctx, need);
    if (bit_field == NULL || bit_pos == NULL) {
        return -1;
    }
    for (i = 0; i < k && idx < need; i++) {
        unsigned w = c->ram[i].hi - c->ram[i].lo + 1, j;

        for (j = 0; j < w && idx < need; j++) {
            bit_field[idx] = (uint16_t)i;
            bit_pos[idx++] = (uint8_t)(msb ? w - 1 - j : j);
        }
    }
    c->bit_field = bit_field;
    c->bit_pos = bit_pos;
    if (cvl_member(o, "frameUs") == NULL) {
        return kind_error(ld->ctx, "missing \"frameUs\"");
    }
    if (cvl_dl_us(ld, cvl_member(o, "frameUs"), "\"frameUs\"", &frame_ps) != 0) {
        return -1;
    }
    c->frame_ps = frame_ps;
    c->slot_ps = frame_ps / c->bp_count;
    if (c->slot_ps == 0) {
        return kind_error(ld->ctx, "\"frameUs\" is shorter than 1 ps per backplane");
    }
    if (dynamic && load_lcd_modes(dl, o, c) != 0) {
        return -1;
    }
    if (cvl_member(o, "outputBank") != NULL) {
        c->has_out_bank = 1;
        if (cvl_dl_field_member(ld, o, "outputBank", &c->out_bank) != 0) {
            return -1;
        }
    }
    if (cvl_member(o, "load") != NULL && load_lcd_load(dl, o, c) != 0) {
        return -1;
    }
    return cvl_dl_cond(ld, o, "enable", &c->enable);
}

/* clock: a bus-less duplex LCD clock face (plan-56-F, PCF1179C; display_clock.c). */
static int clk_cell(Dl *dl, const cJSON *e, const DispCfg *c, ClkCell *out) {
    static const char *const CK[] = {"pin", "bp", NULL};
    Ld *ld = dl->ld;
    const char *pin, *bp;
    size_t g, b;

    out->seg = -1;
    out->bp = 0;
    if (e == NULL || cJSON_IsNull(e)) {
        return 0;
    }
    if (!cJSON_IsObject(e) || cvl_check_keys(ld, e, CK) != 0 || kind_string(ld->ctx, e, "pin", &pin) != 0 ||
        kind_string(ld->ctx, e, "bp", &bp) != 0) {
        return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "a cell is null or {pin, bp}");
    }
    for (g = 0; g < c->out_count && strcmp(ld->names[c->outs[g]], pin) != 0; g++) {
    }
    for (b = 0; b < c->line_count && strcmp(ld->names[c->lines[b]], bp) != 0; b++) {
    }
    if (g == c->out_count) {
        return kind_error(ld->ctx, "cell pin \"%s\" is not one of \"segmentPins\"", pin);
    }
    if (b == c->line_count) {
        return kind_error(ld->ctx, "cell bp \"%s\" is not one of \"backplanes\"", bp);
    }
    out->seg = (int)g;
    out->bp = (unsigned)b;
    return 0;
}

/* A time key: "name": milliseconds above 0, stored in picoseconds. */
static int clk_ms(Ld *ld, const cJSON *o, const char *key, uint64_t *ps) {
    const cJSON *v = cvl_member(o, key);

    if (v == NULL) {
        return 0;
    }
    if (!cJSON_IsNumber(v) || !(v->valuedouble > 0) || v->valuedouble > 1e6) {
        return kind_error(ld->ctx, "\"%s\" must be a time in milliseconds above 0", key);
    }
    *ps = (uint64_t)llround(v->valuedouble * 1e9);
    return 0;
}

static int load_display_clock(Dl *dl, const cJSON *o, DispCfg *c) {
    static const char *const KEYS[] = {
        "driver", "backplanes", "segmentPins", "frameUs", "mode", "flash", "enable", "fast", "data",
        "keys", "debounceMs", "firstRepeatMs", "repeatMs", "fastDivisor", "digits", "colon",
        "am", "pm", "openDrain", NULL
    };
    static const char *const DK[] = {"value", "cells", "glyphs", NULL};
    static const char *const VALUES[] = {"hourTens", "hourUnits", "minuteTens", "minuteUnits", NULL};
    static const char *const KK[] = {"pin", "unit", NULL};
    Ld *ld = dl->ld;
    ClockCfg *k = kind_alloc(ld->ctx, sizeof(*k));
    const cJSON *v, *e;
    uint64_t frame_ps;
    uint32_t n32;
    size_t i;

    if (k == NULL || cvl_check_keys(ld, o, KEYS) != 0) {
        return -1;
    }
    memset(k, 0, sizeof *k);
    k->mode_pin = k->flash_pin = k->enable_pin = k->fast_pin = k->data_pin = CV_NONE;
    k->key_pin[0] = k->key_pin[1] = CV_NONE;
    if (cvl_dl_pins(dl, o, "backplanes", 2, DISP_LCD_ROWS, &c->lines, &c->line_count) != 0 ||
        cvl_dl_pins(dl, o, "segmentPins", 1, DISP_MAX_LCD_SEGMENTS, &c->outs, &c->out_count) != 0) {
        return -1;
    }
    c->bp_count = (unsigned)c->line_count;
    c->segments = (unsigned)c->out_count;
    if (cvl_member(o, "frameUs") == NULL) {
        return kind_error(ld->ctx, "missing \"frameUs\"");
    }
    if (cvl_dl_us(ld, cvl_member(o, "frameUs"), "\"frameUs\"", &frame_ps) != 0) {
        return -1;
    }
    c->frame_ps = frame_ps;
    c->slot_ps = frame_ps / c->bp_count > 0 ? frame_ps / c->bp_count : 1;
    if (cvl_pin_member(ld, o, "mode", 0, &k->mode_pin) != 0 || cvl_pin_member(ld, o, "flash", 0, &k->flash_pin) != 0 ||
        cvl_pin_member(ld, o, "enable", 0, &k->enable_pin) != 0 || cvl_pin_member(ld, o, "fast", 0, &k->fast_pin) != 0 ||
        cvl_pin_member(ld, o, "data", 0, &k->data_pin) != 0) {
        return -1;
    }
    k->debounce_ps = 65000000000ull;
    k->first_ps = 1000000000000ull;
    k->repeat_ps = 250000000000ull;
    k->fast_div = 64;
    if (clk_ms(ld, o, "debounceMs", &k->debounce_ps) != 0 || clk_ms(ld, o, "firstRepeatMs", &k->first_ps) != 0 ||
        clk_ms(ld, o, "repeatMs", &k->repeat_ps) != 0) {
        return -1;
    }
    if (cvl_member(o, "fastDivisor") != NULL) {
        if (cvl_u32_member(ld, o, "fastDivisor", &n32) != 0 || n32 < 1) {
            return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"fastDivisor\" is 1 or more");
        }
        k->fast_div = n32;
    }
    v = cvl_member(o, "keys");
    if (!cJSON_IsArray(v) || cJSON_GetArraySize(v) < 1 || cJSON_GetArraySize(v) > 2) {
        return kind_error(ld->ctx, "\"keys\" is an array of 1-2 {pin, unit}");
    }
    cJSON_ArrayForEach(e, v) {
        const char *unit;

        kind_push(ld->ctx, "keys[%u]", k->key_count);
        if (cvl_check_keys(ld, e, KK) != 0 || cvl_pin_member(ld, e, "pin", 1, &k->key_pin[k->key_count]) != 0 ||
            kind_string(ld->ctx, e, "unit", &unit) != 0) {
            return -1;
        }
        if (strcmp(unit, "hour") != 0 && strcmp(unit, "minute") != 0) {
            return kind_error(ld->ctx, "\"unit\" is \"hour\" or \"minute\"");
        }
        k->key_minute[k->key_count] = strcmp(unit, "minute") == 0;
        k->key_count++;
        kind_pop(ld->ctx);
    }
    v = cvl_member(o, "digits");
    if (!cJSON_IsArray(v) || cJSON_GetArraySize(v) < 1 || cJSON_GetArraySize(v) > CLK_DIGITS) {
        return kind_error(ld->ctx, "\"digits\" is an array of 1-%d digits", CLK_DIGITS);
    }
    cJSON_ArrayForEach(e, v) {
        ClkDigit *d = &k->digit[k->digit_count];
        const char *val;
        const cJSON *cells = cvl_member(e, "cells"), *gl = cvl_member(e, "glyphs"), *x;
        size_t j = 0;

        kind_push(ld->ctx, "digits[%u]", k->digit_count);
        if (cvl_check_keys(ld, e, DK) != 0 || kind_string(ld->ctx, e, "value", &val) != 0) {
            return -1;
        }
        for (i = 0; VALUES[i] != NULL && strcmp(VALUES[i], val) != 0; i++) {
        }
        if (VALUES[i] == NULL) {
            return kind_error(ld->ctx, "\"value\" is hourTens, hourUnits, minuteTens or minuteUnits");
        }
        d->value = (ClkValue)i;
        if (!cJSON_IsArray(cells) || cJSON_GetArraySize(cells) != CLK_SEGS) {
            return kind_error(ld->ctx, "\"cells\" is an array of 7 cells (a-g, null = none)");
        }
        cJSON_ArrayForEach(x, cells) {
            if (clk_cell(dl, x, c, &d->seg[j++]) != 0) {
                return -1;
            }
        }
        if (!cJSON_IsArray(gl) || cJSON_GetArraySize(gl) < 1 || cJSON_GetArraySize(gl) > CLK_GLYPHS) {
            return kind_error(ld->ctx, "\"glyphs\" is an array of 1-%d 7-character a-g strings", CLK_GLYPHS);
        }
        j = 0;
        cJSON_ArrayForEach(x, gl) {
            size_t q;

            if (!cJSON_IsString(x) || strlen(x->valuestring) != CLK_SEGS) {
                return kind_error(ld->ctx, "a glyph is 7 characters, 0 or 1 for segments a-g");
            }
            for (q = 0; q < CLK_SEGS; q++) {
                char ch = x->valuestring[q];

                if (ch != '0' && ch != '1') {
                    return kind_error(ld->ctx, "a glyph is 7 characters, 0 or 1 for segments a-g");
                }
                d->glyph[j] |= (uint8_t)((ch == '1') << q);
            }
            j++;
        }
        d->glyph_count = (unsigned)j;
        k->digit_count++;
        kind_pop(ld->ctx);
    }
    if (clk_cell(dl, cvl_member(o, "colon"), c, &k->colon) != 0 || clk_cell(dl, cvl_member(o, "am"), c, &k->am) != 0 ||
        clk_cell(dl, cvl_member(o, "pm"), c, &k->pm) != 0) {
        return -1;
    }
    c->clock = k;
    return 0;
}

/* ccsi: a 48 x 16 RGB matrix driver on the continuous clock series interface (plan-56-D, TLC6983). */
static int load_display_ccsi(Dl *dl, const cJSON *o, DispCfg *c) {
    static const char *const KEYS[] = {
        "driver", "sclk", "sin", "sout", "lines", "red", "green", "blue", "scan", "openDrain", NULL
    };
    static const char *const PINS[] = {"sclk", "sin", "sout"};
    Ld *ld = dl->ld;
    CcsiCfg *k = kind_alloc(ld->ctx, sizeof(*k));
    int *dst[3];
    const int *grp[4];
    int *arr[4];
    size_t n, i, j;
    const char *const GROUPS[] = {"lines", "red", "green", "blue"};

    if (k == NULL || cvl_check_keys(ld, o, KEYS) != 0) {
        return -1;
    }
    dst[0] = &k->sclk;
    dst[1] = &k->sin;
    dst[2] = &k->sout;
    for (i = 0; i < 3; i++) {
        const char *name;

        if (kind_string(ld->ctx, o, PINS[i], &name) != 0 || cvl_dl_take(dl, name, dst[i]) != 0) {
            return -1;
        }
    }
    arr[0] = k->line;
    arr[1] = k->red;
    arr[2] = k->green;
    arr[3] = k->blue;
    for (i = 0; i < 4; i++) {
        if (cvl_dl_pins(dl, o, GROUPS[i], 16, 16, &grp[i], &n) != 0) {
            return -1;
        }
        for (j = 0; j < 16; j++) {
            arr[i][j] = grp[i][j];
        }
    }
    c->ccsi = k;
    return cvl_dl_scan(ld, o, &c->slot_ps);
}

int cvl_load_display(Ld *ld, const cJSON *d, CvDevice *dev) {
    static const char *const DRIVERS[] = {"digits", "static", "matrix", "ports", "lcd", "clock", "ccsi", NULL};
    Dl *dl;
    DispCfg *c;
    const char *name;
    size_t i;
    int rc;

    /* d is not NULL: cv_load_device rejects a display driver without "display" first. */
    kind_push(ld->ctx, "display");
    if (!cJSON_IsObject(d) || kind_string(ld->ctx, d, "driver", &name) != 0) {
        return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"display\" must be an object");
    }
    for (i = 0; DRIVERS[i] != NULL && strcmp(DRIVERS[i], name) != 0; i++) {
    }
    if (DRIVERS[i] == NULL) {
        return kind_error(ld->ctx, "unknown driver \"%s\" (digits, static, matrix, ports, lcd, clock, ccsi)", name);
    }
    dl = kind_alloc(ld->ctx, sizeof(*dl));
    c = kind_alloc(ld->ctx, sizeof(*c));
    if (dl == NULL || c == NULL) {
        return -1;
    }
    dl->ld = ld;
    c->driver = (DispDriver)i;
    switch (c->driver) {
    case DISP_DIGITS:
    case DISP_MATRIX:
        rc = cvl_load_display_scan(dl, d, c, c->driver == DISP_MATRIX);
        break;
    case DISP_STATIC:
        rc = cvl_load_display_static(dl, d, c);
        break;
    case DISP_PORTS:
        rc = cvl_load_display_ports(dl, d, c);
        break;
    case DISP_CLOCK:
        rc = load_display_clock(dl, d, c);
        break;
    case DISP_CCSI:
        rc = load_display_ccsi(dl, d, c);
        break;
    default:
        rc = load_display_lcd(dl, d, c);
        break;
    }
    if (rc != 0 || cvl_dl_open_drain(dl, d, c) != 0) {
        return -1;
    }
    kind_pop(ld->ctx);
    dev->disp = c;
    return 0;
}

