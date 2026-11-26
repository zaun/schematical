/* models/converter_load_display.c: part of the model in models/converter_load.c (split by plan-68-I/J). */
#include "converter_load_int.h"

/* display.ram {"bytes", "address", "space"}: makes the byte registers (as clock.ram does),
   before the protocol reads the register names. */
int cvl_load_display_ram(Ld *ld, const cJSON *d) {
    static const char *const RK[] = {"bytes", "address", "space", NULL};
    const cJSON *r = cvl_member(d, "ram");
    size_t first = ld->reg_count;
    int rc;

    if (r == NULL || !cJSON_IsObject(r)) {
        return 0;
    }
    kind_push(ld->ctx, "display");
    kind_push(ld->ctx, "ram");
    if (cvl_check_keys(ld, r, RK) != 0) {
        return -1;
    }
    if (cvl_member(r, "bytes") == NULL) {
        return kind_error(ld->ctx, "missing \"bytes\"");
    }
    if (!cJSON_IsNumber(cvl_member(r, "bytes")) || cvl_member(r, "bytes")->valuedouble < 1 ||
        cvl_member(r, "bytes")->valuedouble > 65536) {
        return kind_error(ld->ctx, "\"bytes\" is 1-65536");
    }
    rc = cvl_load_ram(ld, d);
    kind_pop(ld->ctx);
    kind_pop(ld->ctx);
    ld->ram_first = first;
    ld->ram_count = ld->reg_count - first;
    return rc;
}

/* A pin name: its name id, taken for one job in the section. */
int cvl_dl_take(Dl *dl, const char *name, int *id) {
    size_t k;

    if (name[0] == '\0') {
        return kind_error(dl->ld->ctx, "a pin name is empty");
    }
    if ((*id = cvl_name_id(dl->ld, name)) == CV_NONE) {
        return -1;
    }
    for (k = 0; k < dl->used_count; k++) {
        if (dl->used[k] == *id) {
            return kind_error(dl->ld->ctx, "pin \"%s\" is used twice in \"display\"", name);
        }
    }
    if (dl->used_count == DISP_MAX_USED) {
        return kind_error(dl->ld->ctx, "too many pins in \"display\"");
    }
    dl->used[dl->used_count++] = *id;
    return 0;
}

/* An array of min..max pin names under `key`. */
int cvl_dl_pins(Dl *dl, const cJSON *o, const char *key, size_t min, size_t max,
                   const int **out, size_t *count) {
    Ld *ld = dl->ld;
    const cJSON *v = cvl_member(o, key), *e;
    size_t n = 0, size;
    int *ids;

    if (v == NULL) {
        return kind_error(ld->ctx, "missing \"%s\"", key);
    }
    if (!cJSON_IsArray(v)) {
        return kind_error(ld->ctx, "\"%s\" must be an array of pin names", key);
    }
    size = (size_t)cJSON_GetArraySize(v);
    if (size < min || size > max) {
        return min == max ? kind_error(ld->ctx, "\"%s\" must name %zu pins", key, min)
                          : kind_error(ld->ctx, "\"%s\" must name %zu to %zu pins", key, min, max);
    }
    ids = kind_alloc(ld->ctx, (size + 1) * sizeof(*ids));
    if (ids == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(e, v) {
        if (!cJSON_IsString(e)) {
            return kind_error(ld->ctx, "\"%s\" must be an array of pin names", key);
        }
        if (cvl_dl_take(dl, e->valuestring, &ids[n++]) != 0) {
            return -1;
        }
    }
    *out = ids;
    *count = n;
    return 0;
}

/* "high" -> 1, "low" -> 0. */
static int dl_level(Ld *ld, const cJSON *o, const char *key, int *out) {
    const char *s;

    if (kind_string(ld->ctx, o, key, &s) != 0) {
        return -1;
    }
    if (strcmp(s, "high") != 0 && strcmp(s, "low") != 0) {
        return kind_error(ld->ctx, "\"%s\" must be \"high\" or \"low\"", key);
    }
    *out = strcmp(s, "high") == 0;
    return 0;
}

/* A register or field name; the result code and the channel are no registers here. */
static int dl_field(Ld *ld, const char *text, CvField *out) {
    if (cvl_field_ref(ld, text, out) != 0) {
        return -1;
    }
    if (out->reg < 0) {
        return kind_error(ld->ctx, "\"%s\" is not a register or field", text);
    }
    return 0;
}

int cvl_dl_field_member(Ld *ld, const cJSON *o, const char *key, CvField *out) {
    const char *s;

    return kind_string(ld->ctx, o, key, &s) != 0 ? -1 : dl_field(ld, s, out);
}

/* "ram": an array of register / field names, or {"bytes", ...} (made by load_display_ram).
   `want` > 0: exactly that many, one per `noun`. */
int cvl_dl_ram(Ld *ld, const cJSON *o, size_t want, const char *noun, const CvField **out,
                  size_t *count) {
    const cJSON *v = cvl_member(o, "ram"), *e;
    CvField *f;
    size_t n = 0, size, i;

    if (v == NULL) {
        return kind_error(ld->ctx, "missing \"ram\"");
    }
    if (cJSON_IsObject(v)) {
        size = ld->ram_count;
    } else if (cJSON_IsArray(v)) {
        size = (size_t)cJSON_GetArraySize(v);
    } else {
        return kind_error(ld->ctx, "\"ram\" is an array of register names or {\"bytes\": n}");
    }
    if (size == 0) {
        return kind_error(ld->ctx, "\"ram\" is empty");
    }
    if (want > 0 && size != want) {
        return kind_error(ld->ctx, "\"ram\" has %zu registers for %zu %s", size, want, noun);
    }
    f = kind_alloc(ld->ctx, size * sizeof(*f));
    if (f == NULL) {
        return -1;
    }
    if (cJSON_IsObject(v)) {
        for (i = 0; i < size; i++) {
            f[i].reg = (int)(ld->ram_first + i);
            f[i].hi = ld->regs[ld->ram_first + i].width - 1;
            f[i].lo = 0;
        }
    } else {
        cJSON_ArrayForEach(e, v) {
            if (!cJSON_IsString(e)) {
                return kind_error(ld->ctx, "\"ram\" is an array of register names");
            }
            kind_push(ld->ctx, "ram[%zu]", n);
            if (dl_field(ld, e->valuestring, &f[n++]) != 0) {
                return -1;
            }
            kind_pop(ld->ctx);
        }
    }
    *out = f;
    *count = size;
    return 0;
}

/* An optional condition under `key` ("REG.FIELD=v", "!=v", or an array: all must hold). */
int cvl_dl_cond(Ld *ld, const cJSON *o, const char *key, CvConds *out) {
    const cJSON *v = cvl_member(o, key);

    if (v == NULL) {
        return 0;
    }
    kind_push(ld->ctx, "%s", key);
    if (cvl_conds(ld, v, out) != 0) {
        return -1;
    }
    kind_pop(ld->ctx);
    return 0;
}

/* Optional "<key>": [bit, ...], one per data pin (0-31, all different). Default: pin i is bit i. */
static int dl_bits(Ld *ld, const cJSON *o, const char *key, size_t count, const uint8_t **out) {
    const cJSON *v = cvl_member(o, key), *e;
    uint8_t *bits = kind_alloc(ld->ctx, count + 1);
    size_t n = 0, i;

    if (bits == NULL) {
        return -1;
    }
    for (i = 0; i < count; i++) {
        bits[i] = (uint8_t)i;
    }
    if (v != NULL) {
        uint32_t seen = 0;

        if (!cJSON_IsArray(v) || (size_t)cJSON_GetArraySize(v) != count) {
            return kind_error(ld->ctx, "\"%s\" must list %zu bit numbers, one per pin", key, count);
        }
        cJSON_ArrayForEach(e, v) {
            if (!cJSON_IsNumber(e) || e->valuedouble < 0 || e->valuedouble > 31 ||
                e->valuedouble != (double)(int)e->valuedouble) {
                return kind_error(ld->ctx, "\"%s\" holds bit numbers 0-31", key);
            }
            if (seen & (1u << (int)e->valuedouble)) {
                return kind_error(ld->ctx, "\"%s\" names bit %d twice", key, (int)e->valuedouble);
            }
            seen |= 1u << (int)e->valuedouble;
            bits[n++] = (uint8_t)(int)e->valuedouble;
        }
    }
    *out = bits;
    return 0;
}

/* A time in microseconds, > 0 and at most 1e7, as ps. */
int cvl_dl_us(Ld *ld, const cJSON *v, const char *what, uint64_t *ps) {
    if (!cJSON_IsNumber(v) || !(v->valuedouble > 0) || v->valuedouble > 1e7) {
        return kind_error(ld->ctx, "%s must be a time in microseconds above 0 and up to 1e7", what);
    }
    *ps = (uint64_t)llround(v->valuedouble * 1e6);
    if (*ps == 0) {
        return kind_error(ld->ctx, "%s is shorter than 1 ps", what);
    }
    return 0;
}

/* "scan": {"periodUs": n}, the time one scan slot lasts. */
int cvl_dl_scan(Ld *ld, const cJSON *o, uint64_t *ps) {
    static const char *const SK[] = {"periodUs", NULL};
    const cJSON *v = cvl_member(o, "scan");
    int rc;

    if (v == NULL) {
        return kind_error(ld->ctx, "missing \"scan\"");
    }
    kind_push(ld->ctx, "scan");
    if (cvl_check_keys(ld, v, SK) != 0 || cvl_member(v, "periodUs") == NULL) {
        return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "missing \"periodUs\"");
    }
    rc = cvl_dl_us(ld, cvl_member(v, "periodUs"), "\"periodUs\"", ps);
    kind_pop(ld->ctx);
    return rc;
}

static int dl_decode_name(Ld *ld, const cJSON *o, DispDecode *out) {
    static const char *const NAMES[] = {"none", "hex", "codeB", "bcd", "ascii7", "ascii16", NULL};
    const char *s = "none";
    size_t i;

    *out = DEC_NONE;
    if (cvl_member(o, "decode") == NULL) {
        return 0;
    }
    if (kind_string(ld->ctx, o, "decode", &s) != 0) {
        return -1;
    }
    for (i = 0; NAMES[i] != NULL; i++) {
        if (strcmp(NAMES[i], s) == 0) {
            *out = (DispDecode)i;
            return 0;
        }
    }
    return kind_error(ld->ctx, "\"decode\" must be \"none\", \"hex\", \"codeB\", \"bcd\", \"ascii7\" or \"ascii16\"");
}

/* decode, decodeField, decodeActive, dpBit of a digits or static driver. `data_pins`: how many
   data pins one digit has (decoding needs 7 or 8). `digits`: how many decodeFields an array has
   (0: no array form: static takes its field from each group). */
static int dl_decode(Ld *ld, const cJSON *o, DispCfg *c, size_t data_pins, size_t digits) {
    const cJSON *v;
    uint32_t dp = 7, active = 1;

    if (dl_decode_name(ld, o, &c->decode) != 0 ||
        cvl_u32_member(ld, o, "decodeActive", &active) != 0 || cvl_u32_member(ld, o, "dpBit", &dp) != 0) {
        return -1;
    }
    if (active > 1 || dp > 31) {
        return kind_error(ld->ctx, "\"decodeActive\" is 0 or 1 and \"dpBit\" is 0-31");
    }
    c->decode_active = (int)active;
    c->dp_bit = dp;
    if (c->decode == DEC_ASCII16 && data_pins != 16 && data_pins != 17) {
        return kind_error(ld->ctx, "\"decode\" \"ascii16\" needs 16 segment pins (a1 a2 b c d1 d2 e f g1 g2 h i j k l m) or 17 (and the point)");
    }
    if (c->decode != DEC_NONE && c->decode != DEC_ASCII16 && data_pins != 7 && data_pins != 8) {
        return kind_error(ld->ctx, "\"decode\" needs 7 segment pins (a-g) or 8 (and the point)");
    }
    if ((v = cvl_member(o, "decodeField")) != NULL) {
        CvField *f;
        size_t n = 0;

        if (c->decode == DEC_NONE) {
            return kind_error(ld->ctx, "\"decodeField\" needs a \"decode\" other than \"none\"");
        }
        /* digits > 0 here: static's keys have no top-level "decodeField" (it goes in each group). */
        c->has_decode_field = 1;
        if (cJSON_IsString(v)) {
            f = kind_alloc(ld->ctx, sizeof(*f));
            if (f == NULL || dl_field(ld, v->valuestring, f) != 0) {
                return -1;
            }
            n = 1;
        } else if (cJSON_IsArray(v) && (size_t)cJSON_GetArraySize(v) == digits) {
            const cJSON *e;

            f = kind_alloc(ld->ctx, digits * sizeof(*f));
            if (f == NULL) {
                return -1;
            }
            cJSON_ArrayForEach(e, v) {
                if (!cJSON_IsString(e) || dl_field(ld, e->valuestring, &f[n++]) != 0) {
                    return kind_failed(ld->ctx) ? -1
                                                : kind_error(ld->ctx, "\"decodeField\" holds field names");
                }
            }
        } else {
            return kind_error(ld->ctx, "\"decodeField\" is a field name or an array of %zu", digits);
        }
        c->decode_field = f;
        c->decode_field_count = n;
    }
    return 0;
}

/* openDrain: pins named by the driver that drive 0 or release. */
int cvl_dl_open_drain(Dl *dl, const cJSON *o, DispCfg *c) {
    Ld *ld = dl->ld;
    const cJSON *v = cvl_member(o, "openDrain"), *e;
    int *ids;
    size_t n = 0, k;

    if (v == NULL) {
        return 0;
    }
    if (!cJSON_IsArray(v)) {
        return kind_error(ld->ctx, "\"openDrain\" must be an array of pin names");
    }
    ids = kind_alloc(ld->ctx, ((size_t)cJSON_GetArraySize(v) + 1) * sizeof(*ids));
    if (ids == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(e, v) {
        int id = CV_NONE;

        if (cJSON_IsString(e)) {
            for (k = 0; k < ld->name_count; k++) {
                if (strcmp(ld->names[k], e->valuestring) == 0) {
                    id = (int)k;
                }
            }
        }
        for (k = 0; k < dl->used_count && id != CV_NONE; k++) {
            if (dl->used[k] == id) {
                break;
            }
        }
        if (id == CV_NONE || k == dl->used_count) {
            return kind_error(ld->ctx, "\"openDrain\": \"%s\" is not a pin this driver drives",
                              cJSON_IsString(e) ? e->valuestring : "?");
        }
        ids[n++] = id;
    }
    c->open_drain = ids;
    c->open_drain_count = n;
    return 0;
}

/* digits with shared digit / data pins (plan-56-D, MAX6950/51): "sharedPins" names 9 pins; digit k
   (k < "digits") is selected on pin k, the other 8 pins carry its segments, "segmentLines" naming
   the segment (a-g, dp) each of them shows in order. Optional "scanLimit" (a field: digits 0 to its
   value are scanned) and "blink" {"enable", "ram", "halfPeriodUs"}. */
static int load_display_shared(Dl *dl, const cJSON *o, DispCfg *c) {
    static const char *const BK[] = {"enable", "ram", "halfPeriodUs", NULL};
    static const char *const SEGS[] = {"a", "b", "c", "d", "e", "f", "g", "dp", NULL};
    Ld *ld = dl->ld;
    const cJSON *v, *e, *b;
    uint64_t n = 0;
    size_t count = 0, i, k;
    unsigned seen = 0;

    if (cvl_member(o, "digitPins") != NULL || cvl_member(o, "segmentPins") != NULL) {
        return kind_error(ld->ctx, "\"sharedPins\" replaces \"digitPins\" and \"segmentPins\"");
    }
    if (kind_uint(ld->ctx, o, "digits", DISP_MAX_LINES, &n) != 0 || n == 0) {
        return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"digits\" must be 1 to %d", DISP_MAX_LINES);
    }
    if (cvl_dl_pins(dl, o, "sharedPins", 9, 9, &c->lines, &c->line_count) != 0 ||
        dl_level(ld, o, "digitActive", &c->line_active) != 0 ||
        dl_level(ld, o, "segmentActive", &c->out_active) != 0) {
        return -1;
    }
    if (n > c->line_count) {
        return kind_error(ld->ctx, "\"digits\" is at most the %zu shared pins", c->line_count);
    }
    c->shared = 1;
    c->shared_digits = (unsigned)n;
    c->out_count = 8;
    v = cvl_member(o, "segmentLines");
    if (!cJSON_IsArray(v) || cJSON_GetArraySize(v) != 8) {
        return kind_error(ld->ctx, "\"segmentLines\" names 8 segments (a-g, dp), one per data line");
    }
    k = 0;
    cJSON_ArrayForEach(e, v) {
        for (i = 0; cJSON_IsString(e) && SEGS[i] != NULL && strcmp(SEGS[i], e->valuestring) != 0; i++) {
        }
        if (!cJSON_IsString(e) || SEGS[i] == NULL || (seen & (1u << i))) {
            return kind_error(ld->ctx, "\"segmentLines\" lists each of a b c d e f g dp once");
        }
        seen |= 1u << i;
        c->seg_of_line[k++] = (uint8_t)i;
    }
    if (dl_bits(ld, o, "segmentBits", 8, &c->out_bits) != 0 ||
        cvl_dl_ram(ld, o, n, "digits", &c->ram, &count) != 0 || dl_decode(ld, o, c, 8, n) != 0) {
        return -1;
    }
    if (cvl_dl_scan(ld, o, &c->slot_ps) != 0 || cvl_dl_cond(ld, o, "blank", &c->blank) != 0 ||
        cvl_dl_cond(ld, o, "test", &c->test) != 0 || cvl_dl_cond(ld, o, "shutdown", &c->shutdown) != 0) {
        return -1;
    }
    if (cvl_member(o, "scanLimit") != NULL) {
        if (cvl_dl_field_member(ld, o, "scanLimit", &c->limit) != 0) {
            return -1;
        }
        c->has_limit = 1;
    }
    if ((b = cvl_member(o, "blink")) != NULL) {
        const cJSON *h;

        kind_push(ld->ctx, "blink");
        if (cvl_check_keys(ld, b, BK) != 0 || cvl_dl_cond(ld, b, "enable", &c->blink_enable) != 0) {
            return -1;
        }
        if (cvl_member(b, "enable") == NULL || cvl_member(b, "ram") == NULL || !cJSON_IsArray(cvl_member(b, "ram"))) {
            return kind_error(ld->ctx, "\"blink\" needs \"enable\" and \"ram\" (an array of one field per digit)");
        }
        if (cvl_dl_ram(ld, b, n, "digits", &c->ram1, &count) != 0) {
            return -1;
        }
        h = cvl_member(b, "halfPeriodUs");
        if (cJSON_IsNumber(h)) {
            uint64_t ps;

            if (cvl_dl_us(ld, h, "\"halfPeriodUs\"", &ps) != 0) {
                return -1;
            }
            memset(&c->blink_half, 0, sizeof c->blink_half);
            c->blink_half.pin = CV_NONE;
            c->blink_half.fixed = (uint32_t)h->valuedouble;     /* microseconds, as the {field, values} form */
        } else if (cJSON_IsObject(h)) {
            if (cvl_load_choice(ld, h, &c->blink_half, 0) != 0) {
                return -1;
            }
        } else {
            return kind_error(ld->ctx, "\"halfPeriodUs\" is a time in microseconds or {\"field\", \"values\"}");
        }
        c->has_blink = 1;
        kind_pop(ld->ctx);
    }
    return 0;
}

/* digits and matrix: a scan line per ram field, the data pins carrying the selected line. */
int cvl_load_display_scan(Dl *dl, const cJSON *o, DispCfg *c, int matrix) {
    static const char *const DIGITS_KEYS[] = {
        "driver", "digits", "digitPins", "digitActive", "segmentPins", "segmentActive", "segmentBits",
        "ram", "decode", "decodeField", "decodeActive", "dpBit", "scan", "blank", "test", "shutdown",
        "openDrain", "sharedPins", "segmentLines", "scanLimit", "blink", NULL
    };
    static const char *const MATRIX_KEYS[] = {
        "driver", "rows", "rowActive", "columns", "columnActive", "columnBits", "ram", "scan",
        "blank", "test", "shutdown", "openDrain", NULL
    };
    Ld *ld = dl->ld;
    const char *line_key = matrix ? "rows" : "digitPins";
    const char *out_key = matrix ? "columns" : "segmentPins";
    uint64_t n = 0;
    size_t count = 0;

    if (cvl_check_keys(ld, o, matrix ? MATRIX_KEYS : DIGITS_KEYS) != 0) {
        return -1;
    }
    if (!matrix && cvl_member(o, "sharedPins") != NULL) {
        return load_display_shared(dl, o, c);
    }
    if (!matrix && (cvl_member(o, "segmentLines") != NULL || cvl_member(o, "scanLimit") != NULL ||
                    cvl_member(o, "blink") != NULL)) {
        return kind_error(ld->ctx, "\"segmentLines\", \"scanLimit\" and \"blink\" go with \"sharedPins\"");
    }
    if (!matrix && (kind_uint(ld->ctx, o, "digits", DISP_MAX_LINES, &n) != 0 || n == 0)) {
        return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"digits\" must be 1 to %d", DISP_MAX_LINES);
    }
    if (cvl_dl_pins(dl, o, line_key, matrix ? 1 : n, matrix ? DISP_MAX_LINES : n, &c->lines,
                &c->line_count) != 0 ||
        dl_level(ld, o, matrix ? "rowActive" : "digitActive", &c->line_active) != 0 ||
        cvl_dl_pins(dl, o, out_key, matrix ? 1 : 7, DISP_MAX_SEGMENTS, &c->outs, &c->out_count) != 0 ||
        dl_level(ld, o, matrix ? "columnActive" : "segmentActive", &c->out_active) != 0) {
        return -1;
    }
    if (!matrix && c->out_count != 7 && c->out_count != 8 && c->out_count != 14 &&
        c->out_count != 16 && c->out_count != 17) {
        return kind_error(ld->ctx, "\"segmentPins\" names 7 pins (a-g), 8 (and the point), 14 or 16");
    }
    if (dl_bits(ld, o, matrix ? "columnBits" : "segmentBits", c->out_count, &c->out_bits) != 0 ||
        cvl_dl_ram(ld, o, c->line_count, matrix ? "rows" : "digits", &c->ram, &count) != 0) {
        return -1;
    }
    if (!matrix && dl_decode(ld, o, c, c->out_count, c->line_count) != 0) {
        return -1;
    }
    if (cvl_dl_scan(ld, o, &c->slot_ps) != 0 || cvl_dl_cond(ld, o, "blank", &c->blank) != 0 ||
        cvl_dl_cond(ld, o, "test", &c->test) != 0 || cvl_dl_cond(ld, o, "shutdown", &c->shutdown) != 0) {
        return -1;
    }
    return 0;
}

/* static: groups of output pins, each following a ram field. */
int cvl_load_display_static(Dl *dl, const cJSON *o, DispCfg *c) {
    static const char *const KEYS[] = {
        "driver", "segmentActive", "segmentBits", "decode", "decodeActive", "dpBit", "groups",
        "openDrain", NULL
    };
    static const char *const GK[] = {"pins", "ram", "decodeField", NULL};
    Ld *ld = dl->ld;
    const cJSON *groups = cvl_member(o, "groups"), *g;
    DispGroup *out;
    size_t n = 0, width = 0;

    if (cvl_check_keys(ld, o, KEYS) != 0 || dl_level(ld, o, "segmentActive", &c->out_active) != 0 ||
        dl_decode_name(ld, o, &c->decode) != 0) {
        return -1;
    }
    if (!cJSON_IsArray(groups) || cJSON_GetArraySize(groups) < 1 ||
        cJSON_GetArraySize(groups) > DISP_MAX_LINES) {
        return kind_error(ld->ctx, "\"groups\" must be an array of 1 to %d groups", DISP_MAX_LINES);
    }
    out = kind_alloc(ld->ctx, (size_t)cJSON_GetArraySize(groups) * sizeof(*out));
    if (out == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(g, groups) {
        DispGroup *gr = &out[n];
        const cJSON *dec;

        kind_push(ld->ctx, "groups[%zu]", n);
        if (cvl_check_keys(ld, g, GK) != 0 || cvl_dl_pins(dl, g, "pins", 1, DISP_MAX_SEGMENTS, &gr->pins,
                                                  &gr->count) != 0 ||
            cvl_dl_field_member(ld, g, "ram", &gr->ram) != 0) {
            return -1;
        }
        if (n == 0) {
            width = gr->count;
        } else if (gr->count != width) {
            return kind_error(ld->ctx, "every group has the same number of pins (%zu)", width);
        }
        if ((dec = cvl_member(g, "decodeField")) != NULL) {
            if (c->decode == DEC_NONE) {
                return kind_error(ld->ctx, "\"decodeField\" needs a \"decode\" other than \"none\"");
            }
            if (!cJSON_IsString(dec) || dl_field(ld, dec->valuestring, &gr->decode) != 0) {
                return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"decodeField\" is a field name");
            }
            gr->has_decode = 1;
        }
        kind_pop(ld->ctx);
        n++;
    }
    c->groups = out;
    c->group_count = n;
    c->out_count = width;
    if (dl_decode(ld, o, c, width, 0) != 0 ||
        dl_bits(ld, o, "segmentBits", width, &c->out_bits) != 0) {
        return -1;
    }
    return 0;
}

/* A port's optional "<key>": a value or an array of 1 to 64 values (dirField values). */
static int dl_value_list(Ld *ld, const cJSON *p, const char *key, int has_dir, const uint32_t **out,
                         size_t *count) {
    const cJSON *v = cvl_member(p, key), *e;
    uint32_t *vals;
    size_t n = 0;

    *out = NULL;
    *count = 0;
    if (v == NULL) {
        return 0;
    }
    if (!has_dir) {
        return kind_error(ld->ctx, "\"%s\" needs \"dirField\"", key);
    }
    vals = kind_alloc(ld->ctx, 64 * sizeof(*vals));
    if (vals == NULL) {
        return -1;
    }
    if (cJSON_IsNumber(v)) {
        if (cvl_u32(ld, v, key, &vals[n++]) != 0) {
            return -1;
        }
    } else if (cJSON_IsArray(v) && cJSON_GetArraySize(v) >= 1 && cJSON_GetArraySize(v) <= 64) {
        cJSON_ArrayForEach(e, v) {
            if (cvl_u32(ld, e, key, &vals[n++]) != 0) {
                return -1;
            }
        }
    } else {
        return kind_error(ld->ctx, "\"%s\" is a value or an array of 1 to 64 values", key);
    }
    *out = vals;
    *count = n;
    return 0;
}

/* ports: pins driven per a direction and a data field. */
int cvl_load_display_ports(Dl *dl, const cJSON *o, DispCfg *c) {
    static const char *const KEYS[] = {"driver", "ports", "openDrain", "shutdown", "test", NULL};
    static const char *const PK[] = {"pin", "dirField", "dirOutput", "dataField", "invert", "sinkValues",
                                     "pullupValues", "readField", NULL};
    Ld *ld = dl->ld;
    const cJSON *ports = cvl_member(o, "ports"), *p;
    DispPort *out;
    size_t n = 0;

    if (cvl_check_keys(ld, o, KEYS) != 0) {
        return -1;
    }
    if (!cJSON_IsArray(ports) || cJSON_GetArraySize(ports) < 1 || cJSON_GetArraySize(ports) > 256) {
        return kind_error(ld->ctx, "\"ports\" must be an array of 1 to 256 ports");
    }
    out = kind_alloc(ld->ctx, (size_t)cJSON_GetArraySize(ports) * sizeof(*out));
    if (out == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(p, ports) {
        DispPort *po = &out[n];
        const cJSON *v;
        const char *name;
        uint32_t *vals;

        kind_push(ld->ctx, "ports[%zu]", n);
        if (cvl_check_keys(ld, p, PK) != 0 || kind_string(ld->ctx, p, "pin", &name) != 0 ||
            cvl_dl_take(dl, name, &po->pin) != 0 || cvl_dl_field_member(ld, p, "dataField", &po->data) != 0) {
            return -1;
        }
        if (cvl_member(p, "dirField") != NULL) {
            po->has_dir = 1;
            if (cvl_dl_field_member(ld, p, "dirField", &po->dir) != 0) {
                return -1;
            }
        }
        vals = kind_alloc(ld->ctx, 64 * sizeof(*vals));
        if (vals == NULL) {
            return -1;
        }
        vals[0] = 1;
        po->output_count = 1;
        if ((v = cvl_member(p, "dirOutput")) != NULL) {
            const cJSON *e;

            if (!po->has_dir) {
                return kind_error(ld->ctx, "\"dirOutput\" needs \"dirField\"");
            }
            po->output_count = 0;
            if (cJSON_IsNumber(v)) {
                if (cvl_u32(ld, v, "\"dirOutput\"", &vals[po->output_count++]) != 0) {
                    return -1;
                }
            } else if (cJSON_IsArray(v) && cJSON_GetArraySize(v) >= 1 && cJSON_GetArraySize(v) <= 64) {
                cJSON_ArrayForEach(e, v) {
                    if (cvl_u32(ld, e, "\"dirOutput\"", &vals[po->output_count++]) != 0) {
                        return -1;
                    }
                }
            } else {
                return kind_error(ld->ctx, "\"dirOutput\" is a value or an array of 1 to 64 values");
            }
        }
        po->outputs = vals;
        if (cvl_member(p, "invert") != NULL && kind_bool(ld->ctx, p, "invert", &po->invert) != 0) {
            return -1;
        }
        if (dl_value_list(ld, p, "sinkValues", po->has_dir, &po->sinks, &po->sink_count) != 0 ||
            dl_value_list(ld, p, "pullupValues", po->has_dir, &po->pullups, &po->pullup_count) != 0) {
            return -1;
        }
        if (cvl_member(p, "readField") != NULL) {
            if (cvl_dl_field_member(ld, p, "readField", &po->read) != 0) {
                return -1;
            }
            po->has_read = 1;
        }
        kind_pop(ld->ctx);
        n++;
    }
    c->ports = out;
    c->port_count = n;
    return cvl_dl_cond(ld, o, "shutdown", &c->shutdown) != 0 || cvl_dl_cond(ld, o, "test", &c->test) != 0 ? -1 : 0;
}

