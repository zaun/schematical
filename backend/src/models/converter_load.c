/*
 * Loads a converter or rtc device (planning/plan-24-data/schema.md) into a
 * CvDevice, binds its pin names per pinout and loads its parts. Everything
 * comes from the KindContext arena.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "converter.h"
#include "rtc.h"

typedef struct {
    KindContext *ctx;
    CvDevice *dev;
    CvRegister *regs;
    size_t reg_count;
    const char **spaces;
    size_t space_count;
    const char **names;
    size_t name_count, name_cap;
} Ld;

/* A time in ns, as ps. Converter and RTC times reach minutes (slow data rates,
   watchdogs, periodic interrupts), past kind_ns's 1 s limit. */
static int cv_ns(KindContext *ctx, const cJSON *value, const char *what, uint64_t *ps) {
    if (!cJSON_IsNumber(value) || !(value->valuedouble >= 0) || value->valuedouble > 1e13) {
        return kind_error(ctx, "%s must be a time in ns from 0 to 1e13", what);
    }
    *ps = (uint64_t)llround(value->valuedouble * 1000.0);
    return 0;
}

static const cJSON *member(const cJSON *o, const char *key) {
    return cJSON_IsObject(o) ? cJSON_GetObjectItemCaseSensitive(o, key) : NULL;
}

static int check_keys(Ld *ld, const cJSON *o, const char *const *keys) {
    static const char *const NOTE[] = {"note", NULL};

    return kind_check_keys(ld->ctx, o, keys, NOTE);
}

static uint32_t mask_of(unsigned bits) {
    return bits >= 32 ? 0xFFFFFFFFu : ((1u << bits) - 1u);
}

static int u32(Ld *ld, const cJSON *v, const char *what, uint32_t *out) {
    if (!cJSON_IsNumber(v) || v->valuedouble < 0 || v->valuedouble > 4294967295.0 ||
        v->valuedouble != (double)(uint32_t)v->valuedouble) {
        return kind_error(ld->ctx, "%s must be a whole number 0-4294967295", what);
    }
    *out = (uint32_t)v->valuedouble;
    return 0;
}

static int u32_member(Ld *ld, const cJSON *o, const char *key, uint32_t *out) {
    const cJSON *v = member(o, key);

    if (v == NULL) {
        return 0;
    }
    return u32(ld, v, key, out);
}

static int level_member(Ld *ld, const cJSON *o, const char *key, int dflt, int *out) {
    uint32_t v = (uint32_t)dflt;

    if (u32_member(ld, o, key, &v) != 0) {
        return -1;
    }
    if (v > 1) {
        return kind_error(ld->ctx, "\"%s\" must be 0 or 1", key);
    }
    *out = (int)v;
    return 0;
}

/* Name ids: every pin name a key uses. */
static int name_id(Ld *ld, const char *name) {
    size_t i;

    for (i = 0; i < ld->name_count; i++) {
        if (strcmp(ld->names[i], name) == 0) {
            return (int)i;
        }
    }
    if (ld->name_count == ld->name_cap) {
        size_t cap = ld->name_cap ? ld->name_cap * 2 : 16;
        const char **n = kind_alloc(ld->ctx, cap * sizeof(*n));

        if (n == NULL) {
            return CV_NONE;
        }
        if (ld->name_count) {
            memcpy(n, ld->names, ld->name_count * sizeof(*n));
        }
        ld->names = n;
        ld->name_cap = cap;
    }
    ld->names[ld->name_count] = kind_strdup(ld->ctx, name);
    return (int)ld->name_count++;
}

static int pin_member(Ld *ld, const cJSON *o, const char *key, int required, int *out) {
    const cJSON *v = member(o, key);

    *out = CV_NONE;
    if (v == NULL) {
        return required ? kind_error(ld->ctx, "missing \"%s\"", key) : 0;
    }
    if (!cJSON_IsString(v) || v->valuestring[0] == '\0') {
        return kind_error(ld->ctx, "\"%s\" must be a pin name", key);
    }
    *out = name_id(ld, v->valuestring);
    return *out == CV_NONE ? -1 : 0;
}

static int space_index(Ld *ld, const char *name) {
    size_t i;

    for (i = 0; i < ld->space_count; i++) {
        if (strcmp(ld->spaces[i], name) == 0) {
            return (int)i;
        }
    }
    {
        const char **n = kind_alloc(ld->ctx, (ld->space_count + 1) * sizeof(*n));

        if (n == NULL) {
            return -1;
        }
        if (ld->space_count) {
            memcpy(n, ld->spaces, ld->space_count * sizeof(*n));
        }
        n[ld->space_count] = kind_strdup(ld->ctx, name);
        ld->spaces = n;
        return (int)ld->space_count++;
    }
}

/* ---- Fields and conditions ---------------------------------------------------------- */

static int field_ref(Ld *ld, const char *text, CvField *out) {
    char name[128];
    const char *dot;
    size_t i, n;

    if (strcmp(text, "R") == 0) {
        out->reg = CV_REG_R;
        out->hi = ld->dev->bits ? ld->dev->bits - 1 : 0;
        out->lo = 0;
        return 0;
    }
    if (strcmp(text, "CH") == 0) {
        out->reg = CV_REG_CH;
        out->hi = 7;
        out->lo = 0;
        return 0;
    }
    dot = strchr(text, '.');
    n = dot != NULL ? (size_t)(dot - text) : strlen(text);
    if (n >= sizeof name) {
        return kind_error(ld->ctx, "name \"%s\" is too long", text);
    }
    memcpy(name, text, n);
    name[n] = '\0';
    for (i = 0; i < ld->reg_count; i++) {
        const CvRegister *r = &ld->regs[i];
        size_t f;

        if (strcmp(r->name, name) != 0) {
            continue;
        }
        out->reg = (int)i;
        if (dot == NULL) {
            out->hi = r->width - 1;
            out->lo = 0;
            return 0;
        }
        for (f = 0; f < r->field_count; f++) {
            if (strcmp(r->fields[f].name, dot + 1) == 0) {
                out->hi = r->fields[f].hi;
                out->lo = r->fields[f].lo;
                return 0;
            }
        }
        return kind_error(ld->ctx, "register \"%s\" has no field \"%s\"", name, dot + 1);
    }
    return kind_error(ld->ctx, "unknown register \"%s\"", name);
}

static int field_member(Ld *ld, const cJSON *o, const char *key, CvField *out) {
    const char *s;

    if (kind_string(ld->ctx, o, key, &s) != 0) {
        return -1;
    }
    return field_ref(ld, s, out);
}

static int cond_text(Ld *ld, const char *s, CvCond *out) {
    char buf[160];
    char *eq, *end;

    if (strlen(s) >= sizeof buf) {
        return kind_error(ld->ctx, "condition \"%s\" is too long", s);
    }
    strcpy(buf, s);
    eq = strchr(buf, '=');
    if (eq == NULL || eq[1] == '\0') {
        return kind_error(ld->ctx, "condition \"%s\": want REG.FIELD=v or REG.FIELD!=v", s);
    }
    out->negate = eq > buf && eq[-1] == '!';
    if (out->negate) {
        eq[-1] = '\0';
    }
    *eq = '\0';
    out->value = (uint32_t)strtoul(eq + 1, &end, 0);
    if (*end != '\0') {
        return kind_error(ld->ctx, "condition \"%s\": the value must be a number", s);
    }
    return field_ref(ld, buf, &out->field);
}

static int conds(Ld *ld, const cJSON *v, CvConds *out) {
    CvCond *items;
    size_t n, i = 0;
    const cJSON *item;

    out->items = NULL;
    out->count = 0;
    if (v == NULL) {
        return 0;
    }
    if (cJSON_IsString(v)) {
        items = kind_alloc(ld->ctx, sizeof(*items));
        if (items == NULL || cond_text(ld, v->valuestring, items) != 0) {
            return -1;
        }
        out->items = items;
        out->count = 1;
        return 0;
    }
    if (!cJSON_IsArray(v)) {
        return kind_error(ld->ctx, "a condition must be a string or an array of strings");
    }
    n = (size_t)cJSON_GetArraySize(v);
    items = kind_alloc(ld->ctx, (n ? n : 1) * sizeof(*items));
    if (items == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(item, v) {
        if (!cJSON_IsString(item) || cond_text(ld, item->valuestring, &items[i++]) != 0) {
            return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "a condition must be a string");
        }
    }
    out->items = items;
    out->count = n;
    return 0;
}

/* A capture letter or a number. */
static int value_of(Ld *ld, const cJSON *v, const char *key, CvValue *out) {
    out->cap = 0;
    out->value = 0;
    if (cJSON_IsString(v)) {
        if (strlen(v->valuestring) != 1 || v->valuestring[0] < 'a' || v->valuestring[0] > 'y' ||
            v->valuestring[0] == 'x') {
            return kind_error(ld->ctx, "\"%s\": a capture letter a-y (not x) or a number", key);
        }
        out->cap = v->valuestring[0] - 'a' + 1;
        return 0;
    }
    return u32(ld, v, key, &out->value);
}

/* ---- Patterns -------------------------------------------------------------------------- */

typedef struct {
    CvTok *toks;
    size_t count, cap;
} TokBuf;

static int push_tok(Ld *ld, TokBuf *b, CvTok t) {
    if (b->count == b->cap) {
        size_t cap = b->cap ? b->cap * 2 : 32;
        CvTok *n = kind_alloc(ld->ctx, cap * sizeof(*n));

        if (n == NULL) {
            return -1;
        }
        if (b->count) {
            memcpy(n, b->toks, b->count * sizeof(*n));
        }
        b->toks = n;
        b->cap = cap;
    }
    b->toks[b->count++] = t;
    return 0;
}

/* in = 1: an input pattern; 0: an output pattern. caps (in only): the letters used. */
static int pattern(Ld *ld, const char *text, int in, CvPattern *out, uint32_t *caps) {
    TokBuf b = {NULL, 0, 0};
    const char *p = text;
    unsigned run = 0;
    unsigned bits = 0;
    size_t i;

    memset(out, 0, sizeof *out);
    if (caps != NULL) {
        *caps = 0;
    }
    while (*p != '\0') {
        CvTok one[32];
        size_t one_count = 0;
        CvTok *bracket = NULL;
        size_t bracket_count = 0;
        unsigned times = 1, k;
        int is_r = 0;

        if (*p == ' ' || *p == '_') {
            p++;
            continue;
        }
        memset(one, 0, sizeof one);
        if (*p == '[') {
            char name[128];
            const char *close = strchr(p, ']');
            char *c1, *c2 = NULL;
            CvField f;
            unsigned hi, lo, bit;
            size_t n;

            if (close == NULL || (size_t)(close - p - 1) >= sizeof name) {
                return kind_error(ld->ctx, "pattern \"%s\": bad [...]", text);
            }
            n = (size_t)(close - p - 1);
            memcpy(name, p + 1, n);
            name[n] = '\0';
            c1 = strchr(name, ':');
            if (c1 != NULL) {
                *c1++ = '\0';
                c2 = strchr(c1, ':');
                if (c2 == NULL) {
                    return kind_error(ld->ctx, "pattern \"%s\": [NAME:hi:lo]", text);
                }
                *c2++ = '\0';
            }
            if (field_ref(ld, name, &f) != 0) {
                return -1;
            }
            if (in && (f.reg == CV_REG_R || f.reg == CV_REG_CH)) {
                return kind_error(ld->ctx, "pattern \"%s\": [%s] in an input pattern", text, name);
            }
            hi = f.hi;
            lo = f.lo;
            if (c1 != NULL) {
                unsigned a = (unsigned)strtoul(c1, NULL, 10);
                unsigned z = (unsigned)strtoul(c2, NULL, 10);

                if (a < z || a > f.hi - f.lo) {
                    return kind_error(ld->ctx, "pattern \"%s\": [%s:%u:%u] is outside it", text,
                                      name, a, z);
                }
                hi = f.lo + a;
                lo = f.lo + z;
            }
            p = close + 1;
            bracket_count = hi - lo + 1;
            bracket = kind_alloc(ld->ctx, bracket_count * sizeof(*bracket));
            if (bracket == NULL) {
                return -1;
            }
            for (bit = hi + 1, k = 0; bit-- > lo; k++) {
                bracket[k].kind = f.reg == CV_REG_R ? CVT_R : f.reg == CV_REG_CH ? CVT_CH : CVT_REF;
                bracket[k].reg = (int16_t)f.reg;
                bracket[k].bit = (uint8_t)bit;
            }
        } else if (*p == '0' || *p == '1' || *p == 'x' || *p == 'z') {
            one[0].kind = *p == '0' ? CVT_0 : *p == '1' ? CVT_1 : *p == 'x' ? CVT_X : CVT_Z;
            if ((in && one[0].kind == CVT_Z) || (!in && one[0].kind == CVT_X)) {
                return kind_error(ld->ctx, "pattern \"%s\": '%c' in an %s pattern", text, *p,
                                  in ? "input" : "output");
            }
            one_count = 1;
            p++;
        } else if (*p == 'R') {
            if (in) {
                return kind_error(ld->ctx, "pattern \"%s\": 'R' in an input pattern", text);
            }
            one[0].kind = CVT_R;
            one_count = 1;
            is_r = 1;
            p++;
        } else if (*p == '@') {
            if (in) {
                return kind_error(ld->ctx, "pattern \"%s\": '@' in an input pattern", text);
            }
            one[0].kind = CVT_AT;
            one_count = 1;
            out->has_at = 1;
            p++;
        } else if (*p >= 'a' && *p <= 'y') {
            one[0].kind = CVT_CAP;
            one[0].cap = (uint8_t)(*p - 'a');
            one_count = 1;
            if (caps != NULL) {
                *caps |= 1u << one[0].cap;
            }
            p++;
        } else {
            return kind_error(ld->ctx, "pattern \"%s\": bad character '%c'", text, *p);
        }
        if (*p == '{') {
            char *end;

            times = (unsigned)strtoul(p + 1, &end, 10);
            if (*end != '}' || times == 0 || times > CV_MAX_BITS) {
                return kind_error(ld->ctx, "pattern \"%s\": bad {n}", text);
            }
            p = end + 1;
        }
        if (!is_r) {
            run = 0;
        }
        while (times-- > 0) {
            size_t n = bracket != NULL ? bracket_count : one_count;

            for (k = 0; k < n; k++) {
                CvTok c = bracket != NULL ? bracket[k] : one[k];

                if (is_r) {
                    if (run >= ld->dev->bits) {
                        return kind_error(ld->ctx, "pattern \"%s\": a run of R longer than "
                                          "\"bits\" (%u)", text, ld->dev->bits);
                    }
                    c.bit = (uint8_t)(ld->dev->bits - 1 - run++);
                }
                if (c.kind != CVT_AT && ++bits > CV_MAX_BITS) {
                    return kind_error(ld->ctx, "pattern \"%s\" is longer than %d bits", text,
                                      CV_MAX_BITS);
                }
                if (push_tok(ld, &b, c) != 0) {
                    return -1;
                }
            }
        }
    }
    /* A capture token's bit: how many tokens of its letter follow it. */
    for (i = 0; i < b.count; i++) {
        if (b.toks[i].kind == CVT_CAP) {
            size_t j;
            unsigned after = 0;

            for (j = i + 1; j < b.count; j++) {
                after += b.toks[j].kind == CVT_CAP && b.toks[j].cap == b.toks[i].cap;
            }
            if (after > 31) {
                return kind_error(ld->ctx, "pattern \"%s\": a capture longer than 32 bits", text);
            }
            b.toks[i].bit = (uint8_t)after;
        }
    }
    out->toks = b.toks;
    out->count = b.count;
    out->bits = bits;
    return 0;
}

static int pattern_member(Ld *ld, const cJSON *o, const char *key, int in, CvPattern *out,
                          uint32_t *caps) {
    const char *s;

    if (kind_string(ld->ctx, o, key, &s) != 0) {
        return -1;
    }
    return pattern(ld, s, in, out, caps);
}

/* ---- Registers -------------------------------------------------------------------------- */

static int load_fields(Ld *ld, const cJSON *v, CvRegister *g) {
    static const char *const FK[] = {"name", "bits", NULL};
    const cJSON *f;
    size_t fn, k = 0;
    CvRegField *fs;
    uint32_t used = 0;

    if (!cJSON_IsArray(v)) {
        return kind_error(ld->ctx, "\"fields\" must be an array");
    }
    fn = (size_t)cJSON_GetArraySize(v);
    fs = kind_alloc(ld->ctx, (fn ? fn : 1) * sizeof(*fs));
    if (fs == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(f, v) {
        const cJSON *b = member(f, "bits");
        size_t q;
        uint32_t m;

        if (check_keys(ld, f, FK) != 0 || kind_string(ld->ctx, f, "name", &fs[k].name) != 0) {
            return -1;
        }
        if (!cJSON_IsArray(b) || cJSON_GetArraySize(b) != 2 ||
            !cJSON_IsNumber(cJSON_GetArrayItem(b, 0)) || !cJSON_IsNumber(cJSON_GetArrayItem(b, 1))) {
            return kind_error(ld->ctx, "field \"%s\": \"bits\" is [hi, lo]", fs[k].name);
        }
        fs[k].hi = (unsigned)cJSON_GetArrayItem(b, 0)->valueint;
        fs[k].lo = (unsigned)cJSON_GetArrayItem(b, 1)->valueint;
        if (cJSON_GetArrayItem(b, 0)->valueint < 0 || cJSON_GetArrayItem(b, 1)->valueint < 0 ||
            fs[k].hi < fs[k].lo || fs[k].hi >= g->width) {
            return kind_error(ld->ctx, "field \"%s\" is outside the register's %u bits",
                              fs[k].name, g->width);
        }
        m = mask_of(fs[k].hi + 1) & ~mask_of(fs[k].lo);
        if (used & m) {
            return kind_error(ld->ctx, "field \"%s\" overlaps another field", fs[k].name);
        }
        used |= m;
        for (q = 0; q < k; q++) {
            if (strcmp(fs[q].name, fs[k].name) == 0) {
                return kind_error(ld->ctx, "duplicate field \"%s\"", fs[k].name);
            }
        }
        k++;
    }
    g->fields = fs;
    g->field_count = fn;
    return 0;
}

static int load_registers(Ld *ld, const cJSON *arr) {
    static const char *const KEYS[] = {"name", "address", "space", "width", "reset", "readOnly",
                                       "fields", "result", "resultBits", "channel",
                                       "clearOnRead", "writeOneClears", NULL};
    const cJSON *r;
    size_t n, i = 0, q;

    ld->regs = NULL;
    ld->reg_count = 0;
    if (space_index(ld, "reg") != 0) {
        return -1;
    }
    if (arr == NULL) {
        return 0;
    }
    if (!cJSON_IsArray(arr)) {
        return kind_error(ld->ctx, "\"registers\" must be an array");
    }
    n = (size_t)cJSON_GetArraySize(arr);
    ld->regs = kind_alloc(ld->ctx, (n ? n : 1) * sizeof(*ld->regs));
    if (ld->regs == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(r, arr) {
        CvRegister *g = &ld->regs[i];
        const cJSON *v;
        uint64_t w;
        uint32_t m;

        kind_push(ld->ctx, "registers[%zu]", i);
        if (check_keys(ld, r, KEYS) != 0 || kind_string(ld->ctx, r, "name", &g->name) != 0 ||
            kind_uint(ld->ctx, r, "width", 32, &w) != 0) {
            return -1;
        }
        if (w == 0) {
            return kind_error(ld->ctx, "\"width\" must be 1-32");
        }
        g->width = (unsigned)w;
        m = mask_of(g->width);
        g->address = CV_NONE;
        g->result_channel = CV_NONE;
        if ((v = member(r, "address")) != NULL) {
            uint32_t a;

            if (u32(ld, v, "\"address\"", &a) != 0) {
                return -1;
            }
            g->address = (int32_t)a;
        }
        if ((v = member(r, "space")) != NULL) {
            if (!cJSON_IsString(v)) {
                return kind_error(ld->ctx, "\"space\" must be a string");
            }
            if ((g->space = space_index(ld, v->valuestring)) < 0) {
                return -1;
            }
        }
        if (u32_member(ld, r, "reset", &g->reset) != 0 ||
            u32_member(ld, r, "clearOnRead", &g->cor) != 0 ||
            u32_member(ld, r, "writeOneClears", &g->w1c) != 0) {
            return -1;
        }
        if ((v = member(r, "readOnly")) != NULL) {
            if (cJSON_IsBool(v)) {
                g->ro = cJSON_IsTrue(v) ? m : 0;
            } else if (u32(ld, v, "\"readOnly\"", &g->ro) != 0) {
                return -1;
            }
        }
        if ((g->reset | g->ro | g->cor | g->w1c) & ~m) {
            return kind_error(ld->ctx, "a value is wider than the register's %u bits", g->width);
        }
        if ((v = member(r, "fields")) != NULL && load_fields(ld, v, g) != 0) {
            return -1;
        }
        if ((v = member(r, "result")) != NULL) {
            if (cJSON_IsTrue(v)) {
                g->result = 1;
            } else if (cJSON_IsString(v)) {
                for (q = 0; q < g->field_count; q++) {
                    if (strcmp(g->fields[q].name, v->valuestring) == 0) {
                        g->result = 2;
                        g->result_hi = g->fields[q].hi;
                        g->result_lo = g->fields[q].lo;
                    }
                }
                if (g->result != 2) {
                    return kind_error(ld->ctx, "\"result\": no field \"%s\"", v->valuestring);
                }
            } else if (!cJSON_IsFalse(v)) {
                return kind_error(ld->ctx, "\"result\" is true or a field name");
            }
        }
        if ((v = member(r, "resultBits")) != NULL) {
            if (!cJSON_IsArray(v) || cJSON_GetArraySize(v) != 2) {
                return kind_error(ld->ctx, "\"resultBits\" is [hi, lo]");
            }
            g->slice = 1;
            g->slice_hi = (unsigned)cJSON_GetArrayItem(v, 0)->valueint;
            g->slice_lo = (unsigned)cJSON_GetArrayItem(v, 1)->valueint;
            if (g->slice_hi < g->slice_lo || g->slice_hi > 31) {
                return kind_error(ld->ctx, "\"resultBits\" is [hi, lo], hi >= lo");
            }
        }
        if ((v = member(r, "channel")) != NULL) {
            uint32_t c;

            if (u32(ld, v, "\"channel\"", &c) != 0) {
                return -1;
            }
            g->result_channel = (int)c;
        }
        for (q = 0; q < i; q++) {
            if (strcmp(ld->regs[q].name, g->name) == 0) {
                return kind_error(ld->ctx, "duplicate register \"%s\"", g->name);
            }
            if (g->address != CV_NONE && ld->regs[q].address == g->address &&
                ld->regs[q].space == g->space) {
                return kind_error(ld->ctx, "registers \"%s\" and \"%s\" share address %d",
                                  ld->regs[q].name, g->name, (int)g->address);
            }
        }
        kind_pop(ld->ctx);
        ld->reg_count = ++i;
    }
    return 0;
}

/* ---- Commands -------------------------------------------------------------------------- */

static const char *const ACTION_NAMES[] = {
    "nop", "wreg", "rreg", "write", "read", "pointer", "start", "stop", "reset", "powerdown",
    "wakeup", "update", "updateall", "clear", "setbits", "clearbits", NULL
};

static int action_of(Ld *ld, const char *name, uint8_t *out) {
    size_t i;

    for (i = 0; ACTION_NAMES[i] != NULL; i++) {
        if (strcmp(ACTION_NAMES[i], name) == 0) {
            *out = (uint8_t)i;
            return 0;
        }
    }
    return kind_error(ld->ctx, "unknown action \"%s\"", name);
}

static int capture_ok(Ld *ld, const CvValue *v, uint32_t caps, const char *key) {
    if (v->cap && !(caps & (1u << (v->cap - 1)))) {
        return kind_error(ld->ctx, "\"%s\" '%c' is not a capture of \"in\"", key, 'a' + v->cap - 1);
    }
    return 0;
}

static int load_command(Ld *ld, const cJSON *c, CvCommand *cmd) {
    static const char *const KEYS[] = {
        "name", "in", "action", "address", "space", "register", "count", "countOffset", "data",
        "channel", "channelMap", "set", "reply", "next", "commit", "then", "wordBits", "when",
        "increment", NULL
    };
    const cJSON *v;
    uint32_t caps = 0;
    const char *s;

    memset(cmd, 0, sizeof *cmd);
    cmd->reg = CV_NONE;
    cmd->count.value = 1;
    if (check_keys(ld, c, KEYS) != 0 || pattern_member(ld, c, "in", 1, &cmd->in, &caps) != 0) {
        return -1;
    }
    if (cmd->in.bits == 0) {
        return kind_error(ld->ctx, "\"in\" is empty");
    }
    if ((v = member(c, "name")) != NULL && cJSON_IsString(v)) {
        cmd->name = kind_strdup(ld->ctx, v->valuestring);
    }
    if ((v = member(c, "action")) != NULL &&
        (!cJSON_IsString(v) || action_of(ld, v->valuestring, &cmd->action) != 0)) {
        return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"action\" must be a string");
    }
    if ((v = member(c, "address")) != NULL) {
        cmd->has_address = 1;
        if (value_of(ld, v, "address", &cmd->address) != 0 ||
            capture_ok(ld, &cmd->address, caps, "address") != 0) {
            return -1;
        }
    }
    if ((v = member(c, "space")) != NULL) {
        size_t i;

        if (!cJSON_IsString(v)) {
            return kind_error(ld->ctx, "\"space\" must be a string");
        }
        for (i = 0; i < ld->space_count && strcmp(ld->spaces[i], v->valuestring) != 0; i++) {
        }
        if (i == ld->space_count) {
            return kind_error(ld->ctx, "no register has space \"%s\"", v->valuestring);
        }
        cmd->space = (int)i;
    }
    if ((v = member(c, "register")) != NULL) {
        CvField f;

        if (!cJSON_IsString(v) || field_ref(ld, v->valuestring, &f) != 0 || f.reg < 0) {
            return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"register\" names a register");
        }
        cmd->reg = f.reg;
    }
    if ((v = member(c, "count")) != NULL &&
        (value_of(ld, v, "count", &cmd->count) != 0 || capture_ok(ld, &cmd->count, caps, "count"))) {
        return -1;
    }
    if ((v = member(c, "countOffset")) != NULL) {
        if (!cJSON_IsNumber(v)) {
            return kind_error(ld->ctx, "\"countOffset\" must be a number");
        }
        cmd->count_offset = v->valueint;
    }
    if ((v = member(c, "data")) != NULL) {
        cmd->has_data = 1;
        if (value_of(ld, v, "data", &cmd->data) != 0 || capture_ok(ld, &cmd->data, caps, "data")) {
            return -1;
        }
    }
    if ((v = member(c, "channel")) != NULL) {
        cmd->has_channel = 1;
        if (value_of(ld, v, "channel", &cmd->channel) != 0 ||
            capture_ok(ld, &cmd->channel, caps, "channel")) {
            return -1;
        }
    }
    if ((v = member(c, "increment")) != NULL) {
        cmd->has_increment = 1;
        if (value_of(ld, v, "increment", &cmd->increment) != 0 ||
            capture_ok(ld, &cmd->increment, caps, "increment")) {
            return -1;
        }
    }
    if ((v = member(c, "channelMap")) != NULL) {
        const cJSON *e;
        size_t n = 0;
        CvMapEntry *m;

        if (!cJSON_IsObject(v)) {
            return kind_error(ld->ctx, "\"channelMap\" must be an object");
        }
        cmd->map_count = (size_t)cJSON_GetArraySize(v);
        m = kind_alloc(ld->ctx, (cmd->map_count ? cmd->map_count : 1) * sizeof(*m));
        if (m == NULL) {
            return -1;
        }
        cJSON_ArrayForEach(e, v) {
            const cJSON *ch;
            size_t k = 0;
            uint8_t *list;

            m[n].key = (uint32_t)strtoul(e->string, NULL, 0);
            if (!cJSON_IsArray(e)) {
                return kind_error(ld->ctx, "\"channelMap\" values are arrays of channels");
            }
            m[n].count = (size_t)cJSON_GetArraySize(e);
            list = kind_alloc(ld->ctx, (m[n].count ? m[n].count : 1));
            if (list == NULL) {
                return -1;
            }
            cJSON_ArrayForEach(ch, e) {
                if (!cJSON_IsNumber(ch) || ch->valueint < 0 || ch->valueint > 255) {
                    return kind_error(ld->ctx, "\"channelMap\": a channel is 0-255");
                }
                list[k++] = (uint8_t)ch->valueint;
            }
            m[n].channels = list;
            n++;
        }
        cmd->map = m;
    }
    if ((v = member(c, "set")) != NULL) {
        const cJSON *e;
        size_t n = 0;
        CvSet *sets;

        if (!cJSON_IsObject(v)) {
            return kind_error(ld->ctx, "\"set\" must be an object");
        }
        cmd->set_count = (size_t)cJSON_GetArraySize(v);
        sets = kind_alloc(ld->ctx, (cmd->set_count ? cmd->set_count : 1) * sizeof(*sets));
        if (sets == NULL) {
            return -1;
        }
        cJSON_ArrayForEach(e, v) {
            if (field_ref(ld, e->string, &sets[n].field) != 0 || sets[n].field.reg < 0 ||
                value_of(ld, e, "set", &sets[n].value) != 0 ||
                capture_ok(ld, &sets[n].value, caps, "set")) {
                return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"set\" names fields");
            }
            n++;
        }
        cmd->sets = sets;
    }
    if (member(c, "reply") != NULL) {
        cmd->has_reply = 1;
        if (pattern_member(ld, c, "reply", 0, &cmd->reply, NULL) != 0) {
            return -1;
        }
    }
    if (member(c, "next") != NULL) {
        cmd->has_next = 1;
        if (pattern_member(ld, c, "next", 0, &cmd->next, NULL) != 0) {
            return -1;
        }
    }
    if ((v = member(c, "commit")) != NULL) {
        if (!cJSON_IsString(v) || (strcmp(v->valuestring, "now") && strcmp(v->valuestring, "end"))) {
            return kind_error(ld->ctx, "\"commit\" is \"now\" or \"end\"");
        }
        cmd->commit_end = strcmp(v->valuestring, "end") == 0;
    }
    if ((v = member(c, "then")) != NULL) {
        const cJSON *e;
        size_t n = 0;
        uint8_t *t;

        if (!cJSON_IsArray(v)) {
            return kind_error(ld->ctx, "\"then\" must be an array of actions");
        }
        cmd->then_count = (size_t)cJSON_GetArraySize(v);
        t = kind_alloc(ld->ctx, cmd->then_count ? cmd->then_count : 1);
        if (t == NULL) {
            return -1;
        }
        cJSON_ArrayForEach(e, v) {
            if (!cJSON_IsString(e) || action_of(ld, e->valuestring, &t[n++]) != 0) {
                return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"then\": action names");
            }
        }
        cmd->then = t;
    }
    if ((v = member(c, "wordBits")) != NULL) {
        uint32_t w;

        if (u32(ld, v, "\"wordBits\"", &w) != 0) {
            return -1;
        }
        cmd->word_bits = w;
    }
    if (conds(ld, member(c, "when"), &cmd->when) != 0) {
        return -1;
    }
    if ((cmd->action == CVA_WREG || cmd->action == CVA_RREG || cmd->action == CVA_READ ||
         cmd->action == CVA_POINTER) && !cmd->has_address && cmd->reg == CV_NONE) {
        return kind_error(ld->ctx, "action \"%s\" needs \"address\" or \"register\"",
                          ACTION_NAMES[cmd->action]);
    }
    if ((cmd->action == CVA_SETBITS || cmd->action == CVA_CLEARBITS) && !cmd->has_data) {
        return kind_error(ld->ctx, "action \"%s\" needs \"data\"", ACTION_NAMES[cmd->action]);
    }
    if (cmd->action == CVA_WRITE && !cmd->has_data && !ld->dev->dac) {
        return kind_error(ld->ctx, "action \"write\" needs \"data\"");
    }
    if (cmd->has_address && !cmd->address.cap) {
        size_t i;

        for (i = 0; i < ld->reg_count; i++) {
            if (ld->regs[i].space == cmd->space && ld->regs[i].address == (int32_t)cmd->address.value) {
                break;
            }
        }
        if (i == ld->reg_count) {
            return kind_error(ld->ctx, "no register at address %u", cmd->address.value);
        }
    }
    (void)s;
    return 0;
}

/* ---- Protocol --------------------------------------------------------------------------- */

static int load_read(Ld *ld, const cJSON *v, CvRead *out) {
    memset(out, 0, sizeof *out);
    if (v == NULL) {
        return 0;
    }
    if (cJSON_IsString(v)) {
        if (strcmp(v->valuestring, "pointer") == 0) {
            out->kind = CVR_POINTER;
            return 0;
        }
        out->kind = CVR_PATTERN;
        return pattern(ld, v->valuestring, 0, &out->pattern, NULL);
    }
    if (cJSON_IsObject(v)) {
        const cJSON *e;
        size_t n = 0;
        CvReadEntry *m;

        out->kind = CVR_MAP;
        out->map_count = (size_t)cJSON_GetArraySize(v);
        m = kind_alloc(ld->ctx, (out->map_count ? out->map_count : 1) * sizeof(*m));
        if (m == NULL) {
            return -1;
        }
        cJSON_ArrayForEach(e, v) {
            if (strcmp(e->string, "high") == 0 || strcmp(e->string, "low") == 0) {
                out->byte_keys = 1;
                m[n].key = strcmp(e->string, "high") == 0;
            } else {
                char *end;

                m[n].key = (uint32_t)strtoul(e->string, &end, 0);
                if (*end != '\0') {
                    return kind_error(ld->ctx, "\"read\" keys are addresses or \"high\"/\"low\"");
                }
            }
            if (!cJSON_IsString(e) || pattern(ld, e->valuestring, 0, &m[n].pattern, NULL) != 0) {
                return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"read\" values are patterns");
            }
            n++;
        }
        out->map = m;
        return 0;
    }
    return kind_error(ld->ctx, "\"read\" is a pattern, \"pointer\" or an address map");
}

static int load_protocol(Ld *ld, const cJSON *p, CvProtocol *out) {
    static const char *const KEYS[] = {"frame", "out", "idle", "commands", "read", "outs", "reads",
                                       "triggers", "generalCall", "align", NULL};
    const cJSON *v;
    size_t i = 0;

    memset(out, 0, sizeof *out);
    if (p == NULL) {
        return kind_error(ld->ctx, "missing \"protocol\"");
    }
    kind_push(ld->ctx, "protocol");
    if (check_keys(ld, p, KEYS) != 0) {
        return -1;
    }
    if ((v = member(p, "frame")) == NULL || !cJSON_IsString(v) ||
        (strcmp(v->valuestring, "fixed") && strcmp(v->valuestring, "stream"))) {
        return kind_error(ld->ctx, "\"frame\" is \"fixed\" or \"stream\"");
    }
    out->stream = strcmp(v->valuestring, "stream") == 0;
    if ((v = member(p, "align")) != NULL) {
        if (!cJSON_IsString(v) || (strcmp(v->valuestring, "start") && strcmp(v->valuestring, "end"))) {
            return kind_error(ld->ctx, "\"align\" is \"start\" or \"end\"");
        }
        out->align_end = strcmp(v->valuestring, "end") == 0;
    }
    if (member(p, "out") != NULL && pattern_member(ld, p, "out", 0, &out->out, NULL) != 0) {
        return -1;
    }
    if ((v = member(p, "idle")) != NULL) {
        if (!cJSON_IsString(v)) {
            return kind_error(ld->ctx, "\"idle\" is \"0\", \"1\", \"z\" or \"repeat\"");
        }
        if (strcmp(v->valuestring, "repeat") == 0) {
            out->idle = 'r';
        } else if (strlen(v->valuestring) == 1 && strchr("01z", v->valuestring[0])) {
            out->idle = v->valuestring[0];
        } else {
            return kind_error(ld->ctx, "\"idle\" is \"0\", \"1\", \"z\" or \"repeat\"");
        }
    }
    if ((v = member(p, "generalCall")) != NULL) {
        if (!cJSON_IsString(v) || strcmp(v->valuestring, "reset") != 0) {
            return kind_error(ld->ctx, "\"generalCall\" is \"reset\"");
        }
        out->general_reset = 1;
    }
    if (load_read(ld, member(p, "read"), &out->read) != 0) {
        return -1;
    }
    if ((v = member(p, "commands")) != NULL) {
        const cJSON *c;
        CvCommand *cmds;

        if (!cJSON_IsArray(v)) {
            return kind_error(ld->ctx, "\"commands\" must be an array");
        }
        out->command_count = (size_t)cJSON_GetArraySize(v);
        cmds = kind_alloc(ld->ctx, (out->command_count ? out->command_count : 1) * sizeof(*cmds));
        if (cmds == NULL) {
            return -1;
        }
        cJSON_ArrayForEach(c, v) {
            kind_push(ld->ctx, "commands[%zu]", i);
            if (load_command(ld, c, &cmds[i]) != 0) {
                return -1;
            }
            kind_pop(ld->ctx);
            i++;
        }
        out->commands = cmds;
    }
    for (i = 0; i < 2; i++) {
        const char *key = i == 0 ? "outs" : "reads";
        const cJSON *arr = member(p, key);
        const cJSON *e;
        size_t n = 0, count;
        CvAlt *alts;

        if (arr == NULL) {
            continue;
        }
        if (!cJSON_IsArray(arr)) {
            return kind_error(ld->ctx, "\"%s\" must be an array", key);
        }
        count = (size_t)cJSON_GetArraySize(arr);
        alts = kind_alloc(ld->ctx, (count ? count : 1) * sizeof(*alts));
        if (alts == NULL) {
            return -1;
        }
        cJSON_ArrayForEach(e, arr) {
            static const char *const AK[] = {"when", "out", "read", NULL};

            kind_push(ld->ctx, "%s[%zu]", key, n);
            if (check_keys(ld, e, AK) != 0 || conds(ld, member(e, "when"), &alts[n].when) != 0) {
                return -1;
            }
            if (i == 0) {
                if (pattern_member(ld, e, "out", 0, &alts[n].pattern, NULL) != 0) {
                    return -1;
                }
            } else if (load_read(ld, member(e, "read"), &alts[n].read) != 0) {
                return -1;
            } else if (alts[n].read.kind == CVR_NONE) {
                return kind_error(ld->ctx, "missing \"read\"");
            }
            kind_pop(ld->ctx);
            n++;
        }
        if (i == 0) {
            out->outs = alts;
            out->out_alt_count = count;
        } else {
            out->reads = alts;
            out->read_alt_count = count;
        }
    }
    if ((v = member(p, "triggers")) != NULL) {
        const cJSON *e;
        size_t n = 0;
        CvTrigger *t;

        if (!cJSON_IsArray(v)) {
            return kind_error(ld->ctx, "\"triggers\" must be an array");
        }
        out->trigger_count = (size_t)cJSON_GetArraySize(v);
        t = kind_alloc(ld->ctx, (out->trigger_count ? out->trigger_count : 1) * sizeof(*t));
        if (t == NULL) {
            return -1;
        }
        cJSON_ArrayForEach(e, v) {
            static const char *const TK[] = {"write", "action", NULL};
            const char *w, *a;

            kind_push(ld->ctx, "triggers[%zu]", n);
            if (check_keys(ld, e, TK) != 0 || kind_string(ld->ctx, e, "write", &w) != 0 ||
                kind_string(ld->ctx, e, "action", &a) != 0 || cond_text(ld, w, &t[n].cond) != 0 ||
                action_of(ld, a, &t[n].action) != 0) {
                return -1;
            }
            kind_pop(ld->ctx);
            n++;
        }
        out->triggers = t;
    }
    kind_pop(ld->ctx);
    return 0;
}

/* ---- Buses ------------------------------------------------------------------------------ */

static int ctl_member(Ld *ld, const cJSON *o, const char *key, const char *level_key, int dflt,
                      CvCtl *out, int *has) {
    static const char *const CK[] = {"pin", "active", "read", "high", NULL};
    const cJSON *v = member(o, key);

    *has = 0;
    out->pin = CV_NONE;
    if (v == NULL) {
        return 0;
    }
    kind_push(ld->ctx, "%s", key);
    if (check_keys(ld, v, CK) != 0 || pin_member(ld, v, "pin", 1, &out->pin) != 0 ||
        level_member(ld, v, level_key, dflt, &out->level) != 0) {
        return -1;
    }
    kind_pop(ld->ctx);
    *has = 1;
    return 0;
}

static int pin_list(Ld *ld, const cJSON *o, const char *key, int *out, size_t *count, size_t max) {
    const cJSON *v = member(o, key);
    const cJSON *e;

    *count = 0;
    if (v == NULL) {
        return 0;
    }
    if (!cJSON_IsArray(v) || (size_t)cJSON_GetArraySize(v) > max) {
        return kind_error(ld->ctx, "\"%s\" is an array of at most %zu pin names", key, max);
    }
    cJSON_ArrayForEach(e, v) {
        if (!cJSON_IsString(e)) {
            return kind_error(ld->ctx, "\"%s\" is an array of pin names", key);
        }
        if ((out[(*count)++] = name_id(ld, e->valuestring)) == CV_NONE) {
            return -1;
        }
    }
    return 0;
}

static int load_i2c_table(Ld *ld, const cJSON *o, int *straps, unsigned *strap_count, int *table,
                          unsigned *table_count) {
    const cJSON *t = member(o, "table");
    const cJSON *e;
    size_t n;

    if (pin_list(ld, o, "address", straps, &n, SERIAL_I2C_STRAPS) != 0) {
        return -1;
    }
    *strap_count = (unsigned)n;
    *table_count = 1u << (2 * n);
    if (!cJSON_IsArray(t) || (unsigned)cJSON_GetArraySize(t) != *table_count) {
        return kind_error(ld->ctx, "\"table\" needs %u entries (4 ^ address pins)", *table_count);
    }
    n = 0;
    cJSON_ArrayForEach(e, t) {
        if (cJSON_IsNull(e)) {
            table[n++] = -1;
        } else if (cJSON_IsNumber(e) && e->valueint >= 0 && e->valueint < 128) {
            table[n++] = e->valueint;
        } else {
            return kind_error(ld->ctx, "\"table\" entries are 7-bit addresses or null");
        }
    }
    return 0;
}

static int load_bus(Ld *ld, const cJSON *b, CvBus *out, int config) {
    static const char *const KEYS[] = {
        "style", "cs", "sclk", "mosi", "miso", "mode", "csActive", "scl", "sda", "address",
        "table", "selectPin", "spiLevel", "selectSampled", "spi", "i2c", "data", "rd", "wr", "rw",
        "byteSelect", "oe", "clock", "clockOut", "ale", NULL
    };
    const char *style;
    uint32_t mode = 0;

    memset(out, 0, sizeof *out);
    out->cs = out->sclk = out->mosi = out->miso = out->scl = out->sda = out->select = CV_NONE;
    out->clock = CV_NONE;
    if (b == NULL || !cJSON_IsObject(b)) {
        return kind_error(ld->ctx, "\"bus\" must be an object");
    }
    if (check_keys(ld, b, KEYS) != 0 || kind_string(ld->ctx, b, "style", &style) != 0) {
        return -1;
    }
    if (strcmp(style, "spi") == 0) {
        out->style = CVB_SPI;
        if (pin_member(ld, b, "cs", 0, &out->cs) != 0 || pin_member(ld, b, "sclk", 1, &out->sclk) ||
            pin_member(ld, b, "mosi", 0, &out->mosi) || pin_member(ld, b, "miso", 0, &out->miso) ||
            u32_member(ld, b, "mode", &mode) != 0 || level_member(ld, b, "csActive", 0, &out->cs_level)) {
            return -1;
        }
        if (mode > 3) {
            return kind_error(ld->ctx, "\"mode\" is 0-3");
        }
        out->mode = mode;
        return 0;
    }
    if (strcmp(style, "i2c") == 0) {
        out->style = CVB_I2C;
        return pin_member(ld, b, "scl", 1, &out->scl) || pin_member(ld, b, "sda", 1, &out->sda) ||
               load_i2c_table(ld, b, out->straps, &out->strap_count, out->table, &out->table_count);
    }
    if (strcmp(style, "i2c-spi") == 0) {
        const cJSON *s = member(b, "spi"), *i = member(b, "i2c");
        uint32_t lvl = 1;

        if (config) {
            return kind_error(ld->ctx, "\"configBus\" is \"spi\" or \"i2c\"");
        }
        out->style = CVB_I2C_SPI;
        if (s == NULL || i == NULL || pin_member(ld, b, "selectPin", 1, &out->select) ||
            u32_member(ld, b, "spiLevel", &lvl) != 0 ||
            pin_member(ld, s, "cs", 0, &out->cs) || pin_member(ld, s, "sclk", 1, &out->sclk) ||
            pin_member(ld, s, "mosi", 0, &out->mosi) || pin_member(ld, s, "miso", 0, &out->miso) ||
            u32_member(ld, s, "mode", &mode) != 0 || pin_member(ld, i, "scl", 1, &out->scl) ||
            pin_member(ld, i, "sda", 1, &out->sda) ||
            load_i2c_table(ld, i, out->straps, &out->strap_count, out->table, &out->table_count)) {
            return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "i2c-spi needs spi, i2c, selectPin");
        }
        out->spi_level = lvl != 0;
        out->mode = mode & 3u;
        return 0;
    }
    if (config) {
        return kind_error(ld->ctx, "\"configBus\" is \"spi\" or \"i2c\"");
    }
    if (strcmp(style, "parallel") == 0) {
        const cJSON *a = member(b, "ale");

        out->style = CVB_PARALLEL;
        if (pin_list(ld, b, "data", out->data, &out->data_count, CV_MAX_DATA) != 0 ||
            pin_list(ld, b, "address", out->address, &out->address_count, CV_MAX_DATA) != 0 ||
            ctl_member(ld, b, "cs", "active", 0, &out->pcs, &out->has_pcs) ||
            ctl_member(ld, b, "rd", "active", 0, &out->rd, &out->has_rd) ||
            ctl_member(ld, b, "wr", "active", 0, &out->wr, &out->has_wr) ||
            ctl_member(ld, b, "rw", "read", 1, &out->rw, &out->has_rw) ||
            ctl_member(ld, b, "byteSelect", "high", 1, &out->byte_sel, &out->has_byte)) {
            return -1;
        }
        if (out->data_count < 4) {
            return kind_error(ld->ctx, "\"data\" needs 4-%d pins", CV_MAX_DATA);
        }
        if (a != NULL) {
            static const char *const AK[] = {"pin", "edge", "bits", NULL};
            const char *e = "fall";
            uint32_t bits = 8;

            if (check_keys(ld, a, AK) != 0 || pin_member(ld, a, "pin", 1, &out->ale.pin) ||
                (member(a, "edge") && kind_string(ld->ctx, a, "edge", &e)) ||
                u32_member(ld, a, "bits", &bits) != 0) {
                return -1;
            }
            out->has_ale = 1;
            out->ale.fall = strcmp(e, "fall") == 0;
            out->ale.bits = bits;
        }
        return 0;
    }
    if (strcmp(style, "clocked") == 0) {
        const cJSON *c = member(b, "clock"), *co = member(b, "clockOut");
        const char *edge = "rise";

        out->style = CVB_CLOCKED;
        if (pin_list(ld, b, "data", out->data, &out->data_count, CV_MAX_DATA) != 0 ||
            ctl_member(ld, b, "oe", "active", 0, &out->oe, &out->has_oe)) {
            return -1;
        }
        if (out->data_count < 1) {
            return kind_error(ld->ctx, "\"data\" needs 1-%d pins", CV_MAX_DATA);
        }
        if (c == NULL || pin_member(ld, c, "pin", 1, &out->clock) != 0 ||
            (member(c, "edge") && kind_string(ld->ctx, c, "edge", &edge))) {
            return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"clock\" needs a pin");
        }
        out->clock_edge = strcmp(edge, "both") == 0 ? 2 : strcmp(edge, "fall") == 0 ? 0 : 1;
        if (co != NULL) {
            const cJSON *e;
            size_t n = 0;
            CvClockOut *outs;

            out->clock_out_count = cJSON_IsArray(co) ? (size_t)cJSON_GetArraySize(co) : 1;
            outs = kind_alloc(ld->ctx, out->clock_out_count * sizeof(*outs));
            if (outs == NULL) {
                return -1;
            }
            for (e = cJSON_IsArray(co) ? co->child : co; e != NULL && n < out->clock_out_count;
                 e = e->next) {
                static const char *const OK[] = {"pin", "invert", "divide", NULL};
                const cJSON *inv = member(e, "invert");
                uint32_t div = 1;

                if (check_keys(ld, e, OK) != 0 || pin_member(ld, e, "pin", 1, &outs[n].pin) != 0 ||
                    u32_member(ld, e, "divide", &div) != 0) {
                    return -1;
                }
                outs[n].invert = cJSON_IsTrue(inv);
                outs[n].divide = div ? div : 1;
                n++;
            }
            out->clock_out = outs;
        }
        return 0;
    }
    return kind_error(ld->ctx, "unknown bus style \"%s\"", style);
}

/* ---- Conversion, ready, pins, DAC, choices ---------------------------------------------- */

static int load_choice(Ld *ld, const cJSON *v, CvChoice *out, int formats) {
    static const char *const FMT[] = {"straight", "twos", "offset", NULL};

    memset(out, 0, sizeof *out);
    out->pin = CV_NONE;
    if (v == NULL) {
        return 0;
    }
    if (formats && cJSON_IsString(v)) {
        size_t i;

        for (i = 0; FMT[i] && strcmp(FMT[i], v->valuestring); i++) {
        }
        if (FMT[i] == NULL) {
            return kind_error(ld->ctx, "\"codeFormat\" is straight, twos or offset");
        }
        out->fixed = (uint32_t)i;
        return 0;
    }
    if (!formats && cJSON_IsNumber(v)) {
        return u32(ld, v, "\"clearCode\"", &out->fixed);
    }
    if (cJSON_IsObject(v)) {
        static const char *const KEYS[] = {"pin", "field", "values", NULL};
        const cJSON *vals = member(v, "values");
        const cJSON *e;
        size_t n = 0;
        uint32_t *keys, *values;

        if (check_keys(ld, v, KEYS) != 0) {
            return -1;
        }
        if (member(v, "pin") != NULL) {
            if (pin_member(ld, v, "pin", 1, &out->pin) != 0) {
                return -1;
            }
        } else if (member(v, "field") != NULL) {
            out->has_field = 1;
            if (field_member(ld, v, "field", &out->field) != 0) {
                return -1;
            }
        } else {
            return kind_error(ld->ctx, "a choice needs \"pin\" or \"field\"");
        }
        if (!cJSON_IsObject(vals) || vals->child == NULL) {
            return kind_error(ld->ctx, "a choice needs a non-empty \"values\" object");
        }
        out->count = (size_t)cJSON_GetArraySize(vals);
        keys = kind_alloc(ld->ctx, out->count * sizeof(*keys));
        values = kind_alloc(ld->ctx, out->count * sizeof(*values));
        if (keys == NULL || values == NULL) {
            return -1;
        }
        cJSON_ArrayForEach(e, vals) {
            keys[n] = (uint32_t)strtoul(e->string, NULL, 0);
            if (formats) {
                size_t i;

                for (i = 0; FMT[i] && (!cJSON_IsString(e) || strcmp(FMT[i], e->valuestring)); i++) {
                }
                if (FMT[i] == NULL) {
                    return kind_error(ld->ctx, "\"codeFormat\" values are straight, twos or offset");
                }
                values[n] = (uint32_t)i;
            } else if (u32(ld, e, "a \"clearCode\" value", &values[n]) != 0) {
                return -1;
            }
            n++;
        }
        out->keys = keys;
        out->values = values;
        out->fixed = values[0];
        return 0;
    }
    return kind_error(ld->ctx, formats ? "bad \"codeFormat\"" : "bad \"clearCode\"");
}

static int sets_member(Ld *ld, const cJSON *o, const char *key, const CvSet **out, size_t *count) {
    const cJSON *v = member(o, key);
    const cJSON *e;
    size_t n = 0;
    CvSet *s;

    *out = NULL;
    *count = 0;
    if (v == NULL) {
        return 0;
    }
    if (!cJSON_IsObject(v)) {
        return kind_error(ld->ctx, "\"%s\" must be an object of field values", key);
    }
    *count = (size_t)cJSON_GetArraySize(v);
    s = kind_alloc(ld->ctx, (*count ? *count : 1) * sizeof(*s));
    if (s == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(e, v) {
        if (field_ref(ld, e->string, &s[n].field) != 0 || s[n].field.reg < 0 ||
            u32(ld, e, key, &s[n].value.value) != 0) {
            return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"%s\" names fields", key);
        }
        n++;
    }
    *out = s;
    return 0;
}

static int load_time(Ld *ld, const cJSON *v, CvTime *out) {
    memset(out, 0, sizeof *out);
    if (v == NULL) {
        return 0;
    }
    if (cJSON_IsNumber(v)) {
        if (cv_ns(ld->ctx, v, "\"time\"", &out->fixed_ps) != 0) {
            return -1;
        }
        out->max_ps = out->fixed_ps;
        return 0;
    }
    if (cJSON_IsObject(v)) {
        static const char *const KEYS[] = {"field", "fields", "values", NULL};
        const cJSON *f = member(v, "fields"), *vals = member(v, "values");
        const cJSON *e;
        CvField *fields;
        uint32_t *keys;
        uint64_t *ps;
        size_t nf, n = 0;

        if (check_keys(ld, v, KEYS) != 0) {
            return -1;
        }
        nf = f != NULL ? (size_t)cJSON_GetArraySize(f) : 1;
        if (nf == 0 || nf > 4 || !cJSON_IsObject(vals) || vals->child == NULL) {
            return kind_error(ld->ctx, "\"time\" is ns or {field(s), values}");
        }
        fields = kind_alloc(ld->ctx, nf * sizeof(*fields));
        if (fields == NULL) {
            return -1;
        }
        if (f != NULL) {
            size_t i = 0;

            cJSON_ArrayForEach(e, f) {
                if (!cJSON_IsString(e) || field_ref(ld, e->valuestring, &fields[i++]) != 0) {
                    return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"fields\" are names");
                }
            }
        } else if (field_member(ld, v, "field", &fields[0]) != 0) {
            return -1;
        }
        out->count = (size_t)cJSON_GetArraySize(vals);
        keys = kind_alloc(ld->ctx, out->count * nf * sizeof(*keys));
        ps = kind_alloc(ld->ctx, out->count * sizeof(*ps));
        if (keys == NULL || ps == NULL) {
            return -1;
        }
        cJSON_ArrayForEach(e, vals) {
            const char *k = e->string;
            size_t i;

            for (i = 0; i < nf; i++) {
                char *end;

                keys[n * nf + i] = (uint32_t)strtoul(k, &end, 0);
                if (end == k || (i + 1 < nf && *end != ',') || (i + 1 == nf && *end != '\0')) {
                    return kind_error(ld->ctx, "\"time\" key \"%s\" needs %zu values", e->string, nf);
                }
                k = end + 1;
            }
            if (cv_ns(ld->ctx, e, "a time", &ps[n]) != 0) {
                return -1;
            }
            if (ps[n] > out->max_ps) {
                out->max_ps = ps[n];
            }
            n++;
        }
        out->fields = fields;
        out->field_count = nf;
        out->keys = keys;
        out->ps = ps;
        return 0;
    }
    return kind_error(ld->ctx, "\"time\" is ns or {field(s), values}");
}

static int load_conversion(Ld *ld, const cJSON *c, CvConversion *out) {
    static const char *const KEYS[] = {"time", "start", "mode", "busy", "done", "onRead", "channel",
                                       NULL};
    const cJSON *v, *e;

    memset(out, 0, sizeof *out);
    if (c == NULL) {
        return 0;
    }
    kind_push(ld->ctx, "conversion");
    out->present = 1;
    out->start_on_enter = 1;
    if (check_keys(ld, c, KEYS) != 0 || load_time(ld, member(c, "time"), &out->time) != 0 ||
        sets_member(ld, c, "busy", &out->busy, &out->busy_count) != 0 ||
        sets_member(ld, c, "done", &out->done, &out->done_count) != 0 ||
        sets_member(ld, c, "onRead", &out->on_read, &out->on_read_count) != 0) {
        return -1;
    }
    if ((v = member(c, "start")) != NULL) {
        size_t n = 0;
        CvWriteTrigger *w;

        if (!cJSON_IsArray(v)) {
            return kind_error(ld->ctx, "\"start\" must be an array");
        }
        w = kind_alloc(ld->ctx, ((size_t)cJSON_GetArraySize(v) + 1) * sizeof(*w));
        if (w == NULL) {
            return -1;
        }
        cJSON_ArrayForEach(e, v) {
            const char *s = cJSON_IsString(e) ? e->valuestring : "";

            if (strcmp(s, "cs-fall") == 0) {
                out->starts |= CVS_CS_FALL;
            } else if (strcmp(s, "cs-rise") == 0) {
                out->starts |= CVS_CS_RISE;
            } else if (strcmp(s, "command") == 0) {
                out->starts |= CVS_COMMAND;
            } else if (strcmp(s, "pin") == 0) {
                out->starts |= CVS_PIN;
            } else if (strcmp(s, "read") == 0) {
                out->starts |= CVS_READ;
            } else if (strcmp(s, "power-on") == 0) {
                out->starts |= CVS_POWER_ON;
            } else if (strncmp(s, "write:", 6) == 0) {
                if (strchr(s + 6, '=') == NULL) {
                    CvField f;

                    if (field_ref(ld, s + 6, &f) != 0 || f.reg < 0) {
                        return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"write:REG\"");
                    }
                    w[n].any = 1;
                    w[n].reg = f.reg;
                } else if (cond_text(ld, s + 6, &w[n].cond) != 0) {
                    return -1;
                } else {
                    w[n].reg = w[n].cond.field.reg;
                }
                n++;
            } else {
                return kind_error(ld->ctx, "unknown start trigger \"%s\"", s);
            }
        }
        out->writes = w;
        out->write_count = n;
    }
    if ((v = member(c, "mode")) != NULL) {
        if (cJSON_IsString(v)) {
            if (strcmp(v->valuestring, "continuous") == 0) {
                out->mode = 1;
            } else if (strcmp(v->valuestring, "single") != 0) {
                return kind_error(ld->ctx, "\"mode\" is single, continuous or {field}");
            }
        } else if (cJSON_IsObject(v)) {
            static const char *const MK[] = {"field", "continuous", "single", "startOnEnter", NULL};
            const cJSON *se = member(v, "startOnEnter");

            out->mode = 2;
            if (check_keys(ld, v, MK) != 0 || field_member(ld, v, "field", &out->mode_field) != 0) {
                return -1;
            }
            if (member(v, "continuous") != NULL) {
                if (u32_member(ld, v, "continuous", &out->mode_value) != 0) {
                    return -1;
                }
            } else if (member(v, "single") != NULL) {
                out->mode_single = 1;
                if (u32_member(ld, v, "single", &out->mode_value) != 0) {
                    return -1;
                }
            } else {
                return kind_error(ld->ctx, "\"mode\" needs \"continuous\" or \"single\"");
            }
            if (se != NULL) {
                out->start_on_enter = cJSON_IsTrue(se);
            }
        } else {
            return kind_error(ld->ctx, "\"mode\" is single, continuous or {field}");
        }
    }
    if (member(c, "channel") != NULL) {
        out->has_channel = 1;
        if (field_member(ld, c, "channel", &out->channel) != 0) {
            return -1;
        }
    }
    kind_pop(ld->ctx);
    return 0;
}

static int load_ready(Ld *ld, const cJSON *arr, CvDevice *dev) {
    static const char *const KEYS[] = {"pin", "active", "asserts", "clears", "openDrain", "onMiso",
                                       "enable", NULL};
    const cJSON *r;
    size_t i = 0;
    CvReady *rd;

    if (arr == NULL) {
        return 0;
    }
    if (!cJSON_IsArray(arr)) {
        return kind_error(ld->ctx, "\"ready\" must be an array");
    }
    dev->ready_count = (size_t)cJSON_GetArraySize(arr);
    rd = kind_alloc(ld->ctx, (dev->ready_count ? dev->ready_count : 1) * sizeof(*rd));
    if (rd == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(r, arr) {
        const char *as;
        const cJSON *cl = member(r, "clears");

        kind_push(ld->ctx, "ready[%zu]", i);
        if (check_keys(ld, r, KEYS) != 0 || pin_member(ld, r, "pin", 1, &rd[i].pin) != 0 ||
            level_member(ld, r, "active", 0, &rd[i].active) != 0 ||
            kind_string(ld->ctx, r, "asserts", &as) != 0 ||
            conds(ld, member(r, "enable"), &rd[i].enable) != 0) {
            return -1;
        }
        if (strcmp(as, "done") == 0) {
            rd[i].asserts_done = 1;
        } else if (strcmp(as, "busy") != 0) {
            return kind_error(ld->ctx, "\"asserts\" is \"busy\" or \"done\"");
        }
        rd[i].open_drain = cJSON_IsTrue(member(r, "openDrain"));
        rd[i].on_miso = cJSON_IsTrue(member(r, "onMiso"));
        if (cl == NULL) {
            rd[i].clears = CVC_START;
        } else {
            const cJSON *e;
            const cJSON *first = cJSON_IsArray(cl) ? cl->child : cl;

            for (e = first; e != NULL; e = cJSON_IsArray(cl) ? e->next : NULL) {
                if (cJSON_IsNumber(e)) {
                    rd[i].clears |= CVC_PULSE;
                    if (cv_ns(ld->ctx, e, "\"clears\"", &rd[i].pulse_ps) != 0) {
                        return -1;
                    }
                } else if (cJSON_IsString(e) && strcmp(e->valuestring, "read") == 0) {
                    rd[i].clears |= CVC_READ;
                } else if (cJSON_IsString(e) && strcmp(e->valuestring, "start") == 0) {
                    rd[i].clears |= CVC_START;
                } else if (cJSON_IsString(e) && strcmp(e->valuestring, "cs-fall") == 0) {
                    rd[i].clears |= CVC_CS_FALL;
                } else {
                    return kind_error(ld->ctx, "\"clears\" is read, start, cs-fall or ns");
                }
            }
        }
        kind_pop(ld->ctx);
        i++;
    }
    dev->ready = rd;
    return 0;
}

static int load_pins(Ld *ld, const cJSON *p, CvPins *out) {
    static const char *const NAMES[CVP_COUNT] = {"reset", "convst", "start", "powerdown", "ldac",
                                                 "clear"};
    static const char *const KEYS[] = {"reset", "convst", "start", "powerdown", "ldac", "clear",
                                       "levels", "resetSelect", NULL};
    const cJSON *v, *e;
    size_t i;

    memset(out, 0, sizeof *out);
    if (p == NULL) {
        return 0;
    }
    kind_push(ld->ctx, "pins");
    if (check_keys(ld, p, KEYS) != 0) {
        return -1;
    }
    for (i = 0; i < CVP_COUNT; i++) {
        static const char *const PK[] = {"pin", "active", "edge", NULL};
        const char *edge = NULL;

        if ((v = member(p, NAMES[i])) == NULL) {
            continue;
        }
        kind_push(ld->ctx, "%s", NAMES[i]);
        out->has[i] = 1;
        if (check_keys(ld, v, PK) != 0 || pin_member(ld, v, "pin", 1, &out->ctl[i].pin) != 0 ||
            level_member(ld, v, "active", i == CVP_START ? 1 : 0, &out->ctl[i].active) != 0 ||
            (member(v, "edge") && kind_string(ld->ctx, v, "edge", &edge))) {
            return -1;
        }
        out->ctl[i].edge = -1;
        if (edge != NULL) {
            if (strcmp(edge, "rise") == 0) {
                out->ctl[i].edge = 1;
            } else if (strcmp(edge, "fall") == 0) {
                out->ctl[i].edge = 0;
            } else if (strcmp(edge, "level") != 0) {
                return kind_error(ld->ctx, "\"edge\" is rise, fall or level");
            }
        } else if (i == CVP_CONVST) {
            out->ctl[i].edge = out->ctl[i].active ? 1 : 0;
        }
        if (i == CVP_CONVST && out->ctl[i].edge < 0) {
            return kind_error(ld->ctx, "\"convst\" needs an edge");
        }
        kind_pop(ld->ctx);
    }
    if ((v = member(p, "levels")) != NULL) {
        size_t n = 0;
        CvLevel *lv;

        if (!cJSON_IsArray(v)) {
            return kind_error(ld->ctx, "\"levels\" must be an array");
        }
        out->level_count = (size_t)cJSON_GetArraySize(v);
        lv = kind_alloc(ld->ctx, (out->level_count ? out->level_count : 1) * sizeof(*lv));
        if (lv == NULL) {
            return -1;
        }
        cJSON_ArrayForEach(e, v) {
            if (pin_member(ld, e, "pin", 1, &lv[n].pin) != 0 ||
                field_member(ld, e, "field", &lv[n].field) != 0 || lv[n].field.reg < 0) {
                return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"levels\": a field");
            }
            n++;
        }
        out->levels = lv;
    }
    if ((v = member(p, "resetSelect")) != NULL) {
        size_t n = 0;
        CvResetSelect *rs;

        if (!cJSON_IsArray(v)) {
            return kind_error(ld->ctx, "\"resetSelect\" must be an array");
        }
        out->reset_select_count = (size_t)cJSON_GetArraySize(v);
        rs = kind_alloc(ld->ctx, (out->reset_select_count ? out->reset_select_count : 1) * sizeof(*rs));
        if (rs == NULL) {
            return -1;
        }
        cJSON_ArrayForEach(e, v) {
            const cJSON *vals = member(e, "resets");
            const cJSON *x;
            size_t k = 0;
            CvRegValue *rv;

            if (pin_member(ld, e, "pin", 1, &rs[n].pin) != 0 ||
                level_member(ld, e, "level", 1, &rs[n].level) != 0 || !cJSON_IsObject(vals)) {
                return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"resetSelect\" entries");
            }
            rs[n].count = (size_t)cJSON_GetArraySize(vals);
            rv = kind_alloc(ld->ctx, (rs[n].count ? rs[n].count : 1) * sizeof(*rv));
            if (rv == NULL) {
                return -1;
            }
            cJSON_ArrayForEach(x, vals) {
                CvField f;

                if (field_ref(ld, x->string, &f) != 0 || f.reg < 0 ||
                    u32(ld, x, "a reset value", &rv[k].value) != 0) {
                    return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"resets\" names registers");
                }
                rv[k++].reg = f.reg;
            }
            rs[n].values = rv;
            n++;
        }
        out->reset_select = rs;
    }
    kind_pop(ld->ctx);
    return 0;
}

static int reg_names(Ld *ld, const cJSON *v, const char *key, int **out, size_t *count) {
    const cJSON *e;
    size_t n = 0;
    int *idx;

    if (!cJSON_IsArray(v)) {
        return kind_error(ld->ctx, "\"%s\" must be an array of register names", key);
    }
    *count = (size_t)cJSON_GetArraySize(v);
    idx = kind_alloc(ld->ctx, (*count ? *count : 1) * sizeof(*idx));
    if (idx == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(e, v) {
        CvField f;

        if (!cJSON_IsString(e) || field_ref(ld, e->valuestring, &f) != 0 || f.reg < 0) {
            return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"%s\" names registers", key);
        }
        idx[n++] = f.reg;
    }
    *out = idx;
    return 0;
}

static int load_dac(Ld *ld, const cJSON *d, CvDac *out) {
    static const char *const KEYS[] = {"input", "output", "clearCode", "ldac", NULL};
    int *in, *outp;
    size_t ni, no;
    const char *ldac;

    memset(out, 0, sizeof *out);
    if (d == NULL) {
        return ld->dev->dac ? kind_error(ld->ctx, "a DAC needs \"dac\"") : 0;
    }
    if (!ld->dev->dac) {
        return kind_error(ld->ctx, "\"dac\" is for type \"dac\"");
    }
    kind_push(ld->ctx, "dac");
    if (check_keys(ld, d, KEYS) != 0 || reg_names(ld, member(d, "input"), "input", &in, &ni) != 0 ||
        reg_names(ld, member(d, "output"), "output", &outp, &no) != 0 ||
        load_choice(ld, member(d, "clearCode"), &out->clear_code, 0) != 0 ||
        kind_string(ld->ctx, d, "ldac", &ldac) != 0) {
        return -1;
    }
    if (ni != no || ni != ld->dev->channels) {
        return kind_error(ld->ctx, "\"input\" and \"output\" need one register per channel (%u)",
                          ld->dev->channels);
    }
    if (strcmp(ldac, "pin") == 0) {
        out->ldac = 0;
    } else if (strcmp(ldac, "command") == 0) {
        out->ldac = 1;
    } else if (strcmp(ldac, "sync") == 0) {
        out->ldac = 2;
    } else {
        return kind_error(ld->ctx, "\"ldac\" is pin, command or sync");
    }
    out->input = in;
    out->output = outp;
    out->count = ni;
    kind_pop(ld->ctx);
    return 0;
}


/* ---- The rtc kind's clock (schema.md § 10) ------------------------------------------- */

static const char *const UNIT_NAMES[RU_COUNT] = {
    "hundredth", "second", "minute", "hour", "weekday", "date", "month", "year", "century"
};

static int unit_of(Ld *ld, const cJSON *o, int *out) {
    const char *u;
    int i;

    if (kind_string(ld->ctx, o, "unit", &u) != 0) {
        return -1;
    }
    for (i = 0; i < RU_COUNT; i++) {
        if (strcmp(UNIT_NAMES[i], u) == 0) {
            *out = i;
            return 0;
        }
    }
    return kind_error(ld->ctx, "unknown unit \"%s\"", u);
}

static int rtc_pin(Ld *ld, const cJSON *v, RtcPin *out) {
    out->pin = CV_NONE;
    out->active = 0;
    out->open_drain = 0;
    if (v == NULL) {
        return 0;
    }
    if (cJSON_IsString(v)) {
        out->pin = name_id(ld, v->valuestring);
        out->open_drain = -1;       /* attributes from another use of the pin */
        return out->pin == CV_NONE ? -1 : 0;
    }
    if (cJSON_IsObject(v)) {
        static const char *const K[] = {"pin", "active", "openDrain", "pulse", NULL};

        if (check_keys(ld, v, K) != 0 || pin_member(ld, v, "pin", 1, &out->pin) != 0 ||
            level_member(ld, v, "active", 0, &out->active) != 0) {
            return -1;
        }
        out->open_drain = cJSON_IsTrue(member(v, "openDrain"));
        return 0;
    }
    return kind_error(ld->ctx, "a pin is a name or {pin, active, openDrain}");
}

static int ns_map(Ld *ld, const cJSON *v, const char *key, const uint32_t **keys,
                  const uint64_t **ps, size_t *count) {
    const cJSON *e;
    size_t n = 0;
    uint32_t *k;
    uint64_t *t;

    if (!cJSON_IsObject(v) || v->child == NULL) {
        return kind_error(ld->ctx, "\"%s\" is a non-empty object value -> ns", key);
    }
    *count = (size_t)cJSON_GetArraySize(v);
    k = kind_alloc(ld->ctx, *count * sizeof(*k));
    t = kind_alloc(ld->ctx, *count * sizeof(*t));
    if (k == NULL || t == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(e, v) {
        k[n] = (uint32_t)strtoul(e->string, NULL, 0);
        if (cv_ns(ld->ctx, e, key, &t[n]) != 0) {
            return -1;
        }
        n++;
    }
    *keys = k;
    *ps = t;
    return 0;
}

static int opt_field(Ld *ld, const cJSON *o, const char *key, int *has, CvField *out) {
    *has = member(o, key) != NULL;
    return *has ? field_member(ld, o, key, out) : 0;
}

static int load_periodic(Ld *ld, const cJSON *e, RtcPeriodic *p) {
    static const char *const K[] = {"field", "periods", "periodNs", "flag", "enable", "summary",
                                    "pin", NULL};

    memset(p, 0, sizeof *p);
    if (check_keys(ld, e, K) != 0 || opt_field(ld, e, "flag", &p->has_flag, &p->flag) != 0 ||
        opt_field(ld, e, "enable", &p->has_enable, &p->enable) != 0 ||
        opt_field(ld, e, "summary", &p->has_summary, &p->summary) != 0 ||
        rtc_pin(ld, member(e, "pin"), &p->pin) != 0) {
        return -1;
    }
    if (member(e, "periodNs") != NULL) {
        p->fixed = 1;
        return cv_ns(ld->ctx, member(e, "periodNs"), "\"periodNs\"", &p->period_ps);
    }
    return field_member(ld, e, "field", &p->rate) ||
           ns_map(ld, member(e, "periods"), "periods", &p->keys, &p->ps, &p->count);
}

static int load_clock(Ld *ld, const cJSON *c, CvDevice *dev) {
    static const char *const KEYS[] = {"fields", "stop", "update", "alarms", "periodic",
                                       "watchdog", "ram", NULL};
    RtcClock *k = kind_alloc(ld->ctx, sizeof(*k));
    const cJSON *v, *e;
    size_t n;

    if (k == NULL) {
        return -1;
    }
    if (c == NULL) {
        return kind_error(ld->ctx, "an RTC needs \"clock\"");
    }
    kind_push(ld->ctx, "clock");
    if (check_keys(ld, c, KEYS) != 0) {
        return -1;
    }
    /* fields */
    v = member(c, "fields");
    if (!cJSON_IsArray(v) || v->child == NULL) {
        return kind_error(ld->ctx, "\"fields\" must be a non-empty array");
    }
    {
        RtcTimeField *tf;
        int seen[RU_COUNT] = {0};

        k->field_count = (size_t)cJSON_GetArraySize(v);
        tf = kind_alloc(ld->ctx, k->field_count * sizeof(*tf));
        if (tf == NULL) {
            return -1;
        }
        n = 0;
        cJSON_ArrayForEach(e, v) {
            static const char *const FK[] = {"field", "unit", "format", "h12", "h12Level", "pm",
                                             "first", "base", NULL};
            const cJSON *f = member(e, "format");
            uint32_t first = 0, base = 2000;

            kind_push(ld->ctx, "fields[%zu]", n);
            if (check_keys(ld, e, FK) != 0 || field_member(ld, e, "field", &tf[n].field) != 0 ||
                unit_of(ld, e, &tf[n].unit) != 0 ||
                opt_field(ld, e, "h12", &tf[n].has_h12, &tf[n].h12) != 0 ||
                opt_field(ld, e, "pm", &tf[n].has_pm, &tf[n].pm) != 0 ||
                level_member(ld, e, "h12Level", 1, &tf[n].h12_level) != 0 ||
                u32_member(ld, e, "first", &first) != 0 || u32_member(ld, e, "base", &base) != 0) {
                return -1;
            }
            tf[n].first = (int)first;
            tf[n].base = (int)base;
            if (f != NULL) {
                if (cJSON_IsString(f)) {
                    if (strcmp(f->valuestring, "binary") == 0) {
                        tf[n].format.fixed_binary = 1;
                    } else if (strcmp(f->valuestring, "bcd") != 0) {
                        return kind_error(ld->ctx, "\"format\" is bcd, binary or {field, binary}");
                    }
                } else if (cJSON_IsObject(f)) {
                    tf[n].format.has_field = 1;
                    if (field_member(ld, f, "field", &tf[n].format.field) != 0 ||
                        u32_member(ld, f, "binary", &tf[n].format.binary_value) != 0) {
                        return -1;
                    }
                } else {
                    return kind_error(ld->ctx, "\"format\" is bcd, binary or {field, binary}");
                }
            }
            seen[tf[n].unit] = 1;
            k->hundredths |= tf[n].unit == RU_HUNDREDTH;
            kind_pop(ld->ctx);
            n++;
        }
        if (!seen[RU_SECOND] || !seen[RU_MINUTE] || !seen[RU_HOUR]) {
            return kind_error(ld->ctx, "\"fields\" need second, minute and hour");
        }
        k->fields = tf;
    }
    /* stop */
    if ((v = member(c, "stop")) != NULL) {
        static const char *const SK[] = {"field", "stopped", "running", "powerOn", NULL};

        kind_push(ld->ctx, "stop");
        if (check_keys(ld, v, SK) != 0 || field_member(ld, v, "field", &k->stop) != 0) {
            return -1;
        }
        k->has_stop = 1;
        if (member(v, "running") != NULL) {
            k->stop_running = 1;
            if (u32_member(ld, v, "running", &k->stop_value) != 0) {
                return -1;
            }
        } else {
            k->stop_value = 1;
            if (u32_member(ld, v, "stopped", &k->stop_value) != 0) {
                return -1;
            }
        }
        kind_pop(ld->ctx);
    }
    /* update */
    if ((v = member(c, "update")) != NULL && !(cJSON_IsString(v) && strcmp(v->valuestring, "none") == 0)) {
        static const char *const UK[] = {"uip", "inhibit", "latch", "write", "writeHalts", NULL};

        kind_push(ld->ctx, "update");
        if (!cJSON_IsObject(v) || check_keys(ld, v, UK) != 0) {
            return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"update\" is none, {uip}, {latch}");
        }
        if (member(v, "uip") != NULL) {
            k->update.kind = RTC_UPDATE_UIP;
            if (field_member(ld, v, "uip", &k->update.uip) != 0 ||
                opt_field(ld, v, "inhibit", &k->update.has_inhibit, &k->update.inhibit) != 0) {
                return -1;
            }
        } else if (member(v, "latch") != NULL) {
            const cJSON *wh = member(v, "writeHalts");

            k->update.kind = RTC_UPDATE_LATCH;
            if (field_member(ld, v, "latch", &k->update.latch) != 0 ||
                opt_field(ld, v, "write", &k->update.has_write, &k->update.write) != 0) {
                return -1;
            }
            k->update.write_halts = wh == NULL || cJSON_IsTrue(wh);
        } else {
            return kind_error(ld->ctx, "\"update\" needs \"uip\" or \"latch\"");
        }
        kind_pop(ld->ctx);
    }
    /* alarms */
    if ((v = member(c, "alarms")) != NULL) {
        RtcAlarm *al;
        size_t i = 0;

        if (!cJSON_IsArray(v)) {
            return kind_error(ld->ctx, "\"alarms\" must be an array");
        }
        k->alarm_count = (size_t)cJSON_GetArraySize(v);
        al = kind_alloc(ld->ctx, (k->alarm_count ? k->alarm_count : 1) * sizeof(*al));
        if (al == NULL) {
            return -1;
        }
        cJSON_ArrayForEach(e, v) {
            static const char *const AK[] = {"match", "flag", "enable", "pin", NULL};
            const cJSON *m = member(e, "match"), *x;
            RtcMatch *mm;
            size_t j = 0;

            kind_push(ld->ctx, "alarms[%zu]", i);
            if (check_keys(ld, e, AK) != 0 || !cJSON_IsArray(m) ||
                opt_field(ld, e, "flag", &al[i].has_flag, &al[i].flag) != 0 ||
                opt_field(ld, e, "enable", &al[i].has_enable, &al[i].enable) != 0 ||
                rtc_pin(ld, member(e, "pin"), &al[i].pin) != 0) {
                return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"match\" must be an array");
            }
            al[i].match_count = (size_t)cJSON_GetArraySize(m);
            mm = kind_alloc(ld->ctx, (al[i].match_count ? al[i].match_count : 1) * sizeof(*mm));
            if (mm == NULL) {
                return -1;
            }
            cJSON_ArrayForEach(x, m) {
                static const char *const MK[] = {"field", "unit", "mask", "maskLevel", "dontCare",
                                                 NULL};
                const cJSON *dc = member(x, "dontCare");

                if (check_keys(ld, x, MK) != 0 || field_member(ld, x, "field", &mm[j].field) != 0 ||
                    unit_of(ld, x, &mm[j].unit) != 0 ||
                    opt_field(ld, x, "mask", &mm[j].has_mask, &mm[j].mask) != 0 ||
                    level_member(ld, x, "maskLevel", 1, &mm[j].mask_level) != 0) {
                    return -1;
                }
                if (dc != NULL) {
                    const char *t = cJSON_IsString(dc) ? dc->valuestring : "";
                    size_t q, len = strlen(t);

                    if (len == 0 || len > 32) {
                        return kind_error(ld->ctx, "\"dontCare\" is a bit pattern like \"11xxxxxx\"");
                    }
                    mm[j].has_dont_care = 1;
                    for (q = 0; q < len; q++) {
                        unsigned bit = (unsigned)(len - 1 - q);

                        if (t[q] == '1' || t[q] == '0') {
                            mm[j].dc_mask |= 1u << bit;
                            mm[j].dc_value |= (uint32_t)(t[q] == '1') << bit;
                        } else if (t[q] != 'x') {
                            return kind_error(ld->ctx, "\"dontCare\" is 0, 1 and x");
                        }
                    }
                }
                j++;
            }
            al[i].match = mm;
            kind_pop(ld->ctx);
            i++;
        }
        k->alarms = al;
    }
    /* periodic */
    if ((v = member(c, "periodic")) != NULL) {
        RtcPeriodic *pe;
        size_t i = 0;

        k->periodic_count = cJSON_IsArray(v) ? (size_t)cJSON_GetArraySize(v) : 1;
        pe = kind_alloc(ld->ctx, (k->periodic_count ? k->periodic_count : 1) * sizeof(*pe));
        if (pe == NULL) {
            return -1;
        }
        for (e = cJSON_IsArray(v) ? v->child : v; e != NULL && i < k->periodic_count; e = e->next) {
            kind_push(ld->ctx, "periodic[%zu]", i);
            if (load_periodic(ld, e, &pe[i]) != 0) {
                return -1;
            }
            kind_pop(ld->ctx);
            i++;
        }
        k->periodic = pe;
    }
    /* watchdog */
    if ((v = member(c, "watchdog")) != NULL) {
        static const char *const WK[] = {"field", "periods", "kick", "flag", "pin", "resetPin",
                                         "unitNs", NULL};
        RtcWatchdog *w = &k->watchdog;
        const cJSON *kk = member(v, "kick"), *rp = member(v, "resetPin");

        kind_push(ld->ctx, "watchdog");
        w->present = 1;
        if (check_keys(ld, v, WK) != 0 || field_member(ld, v, "field", &w->field) != 0 ||
            ns_map(ld, member(v, "periods"), "periods", &w->keys, &w->ps, &w->count) != 0 ||
            opt_field(ld, v, "flag", &w->has_flag, &w->flag) != 0 ||
            rtc_pin(ld, member(v, "pin"), &w->pin) != 0 || rtc_pin(ld, rp, &w->reset_pin) != 0) {
            return -1;
        }
        if (kk != NULL) {
            const char *edge = "both";

            w->has_kick = 1;
            if (pin_member(ld, kk, "pin", 1, &w->kick_pin) != 0 ||
                (member(kk, "edge") && kind_string(ld->ctx, kk, "edge", &edge))) {
                return -1;
            }
            w->kick_edge = strcmp(edge, "rise") == 0 ? 1 : strcmp(edge, "fall") == 0 ? 0 : 2;
        }
        if (rp != NULL && member(rp, "pulse") != NULL &&
            ns_map(ld, member(rp, "pulse"), "pulse", &w->pulse_keys, &w->pulse_ps,
                   &w->pulse_count) != 0) {
            return -1;
        }
        kind_pop(ld->ctx);
    }
    kind_pop(ld->ctx);
    dev->clock = k;
    return 0;
}

/* clock.ram: `bytes` byte registers from `address` in `space` (appended to the map). */
static int load_ram(Ld *ld, const cJSON *c) {
    const cJSON *r = member(c, "ram");
    uint32_t bytes = 0, address = 0, i;
    const char *space = "reg";
    CvRegister *regs;
    int sp;

    if (r == NULL) {
        return 0;
    }
    if (u32_member(ld, r, "bytes", &bytes) != 0 || u32_member(ld, r, "address", &address) != 0 ||
        (member(r, "space") && kind_string(ld->ctx, r, "space", &space))) {
        return -1;
    }
    if (bytes == 0 || bytes > 65536) {
        return kind_error(ld->ctx, "clock.ram: \"bytes\" is 1-65536");
    }
    if ((sp = space_index(ld, space)) < 0) {
        return -1;
    }
    regs = kind_alloc(ld->ctx, (ld->reg_count + bytes) * sizeof(*regs));
    if (regs == NULL) {
        return -1;
    }
    if (ld->reg_count) {
        memcpy(regs, ld->regs, ld->reg_count * sizeof(*regs));
    }
    for (i = 0; i < bytes; i++) {
        char name[24];
        CvRegister *g = &regs[ld->reg_count + i];

        snprintf(name, sizeof name, "RAM%u", (unsigned)(address + i));
        g->name = kind_strdup(ld->ctx, name);
        g->address = (int32_t)(address + i);
        g->space = sp;
        g->width = 8;
        g->result_channel = CV_NONE;
    }
    ld->regs = regs;
    ld->reg_count += bytes;
    return 0;
}

/* ---- The device ------------------------------------------------------------------------ */

CvDevice *cv_load_device(KindContext *ctx, const cJSON *json, int rtc) {
    Ld ld;
    CvDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const cJSON *v;
    const char *type;
    uint64_t bits = 0, channels = 1;
    size_t n;
    int i;

    if (dev == NULL) {
        return NULL;
    }
    memset(&ld, 0, sizeof ld);
    ld.ctx = ctx;
    ld.dev = dev;
    dev->rtc = rtc;
    if (!rtc) {
        if (kind_string(ctx, json, "type", &type) != 0) {
            return NULL;
        }
        if (strcmp(type, "dac") == 0) {
            dev->dac = 1;
        } else if (strcmp(type, "adc") != 0) {
            kind_error(ctx, "\"type\" is \"adc\" or \"dac\"");
            return NULL;
        }
        if (kind_uint(ctx, json, "bits", 32, &bits) != 0 ||
            kind_uint(ctx, json, "channels", 1024, &channels) != 0) {
            return NULL;
        }
        if (bits == 0 || channels == 0) {
            kind_error(ctx, "\"bits\" and \"channels\" must be at least 1");
            return NULL;
        }
        dev->info.type = dev->dac ? "dac" : "adc";
    }
    dev->bits = (unsigned)bits;
    dev->channels = (unsigned)channels;
    dev->info.bits = dev->bits;
    dev->info.channels = dev->channels;
    if (load_registers(&ld, member(json, "registers")) != 0 ||
        (rtc && load_ram(&ld, member(json, "clock")) != 0)) {
        return NULL;
    }
    dev->regs = ld.regs;
    dev->reg_count = ld.reg_count;
    if (!rtc && !dev->dac) {
        if (member(json, "codeFormat") == NULL) {
            kind_error(ctx, "an ADC needs \"codeFormat\"");
            return NULL;
        }
        kind_push(ctx, "codeFormat");
        if (load_choice(&ld, member(json, "codeFormat"), &dev->code, 1) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    if ((v = member(json, "analog")) != NULL) {
        const cJSON *e;
        int *a;

        if (!cJSON_IsArray(v)) {
            kind_error(ctx, "\"analog\" must be an array of pin names");
            return NULL;
        }
        dev->analog_count = (size_t)cJSON_GetArraySize(v);
        a = kind_alloc(ctx, (dev->analog_count ? dev->analog_count : 1) * sizeof(*a));
        if (a == NULL) {
            return NULL;
        }
        n = 0;
        cJSON_ArrayForEach(e, v) {
            if (!cJSON_IsString(e)) {
                kind_error(ctx, "\"analog\" must be an array of pin names");
                return NULL;
            }
            a[n++] = name_id(&ld, e->valuestring);
        }
        dev->analog = a;
    }
    kind_push(ctx, "bus");
    if (load_bus(&ld, member(json, "bus"), &dev->bus, 0) != 0) {
        return NULL;
    }
    kind_pop(ctx);
    if ((v = member(json, "configBus")) != NULL) {
        kind_push(ctx, "configBus");
        dev->has_config_bus = 1;
        if (load_bus(&ld, v, &dev->config_bus, 1) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    if ((v = member(json, "timing")) != NULL) {
        static const char *const TK[] = {"tValid", "tOut", "tFloat", "tAcc", "tDf", NULL};
        uint64_t *t[5];

        t[0] = &dev->timing.t_valid;
        t[1] = &dev->timing.t_out;
        t[2] = &dev->timing.t_float;
        t[3] = &dev->timing.t_acc;
        t[4] = &dev->timing.t_df;
        if (check_keys(&ld, v, TK) != 0) {
            return NULL;
        }
        for (i = 0; i < 5; i++) {
            const cJSON *x = member(v, TK[i]);

            if (x != NULL && cv_ns(ctx, x, TK[i], t[i]) != 0) {
                return NULL;
            }
        }
    }
    if (load_protocol(&ld, member(json, "protocol"), &dev->protocol) != 0 ||
        load_conversion(&ld, member(json, "conversion"), &dev->conversion) != 0 ||
        load_ready(&ld, member(json, "ready"), dev) != 0 ||
        load_pins(&ld, member(json, "pins"), &dev->pins) != 0) {
        return NULL;
    }
    if (!rtc) {
        if (load_dac(&ld, member(json, "dac"), &dev->dacinfo) != 0) {
            return NULL;
        }
        if (!dev->dac && !dev->conversion.present && dev->bus.style != CVB_CLOCKED) {
            kind_error(ctx, "an ADC needs \"conversion\"");
            return NULL;
        }
        if (dev->dac && dev->dacinfo.ldac == 0 && !dev->pins.has[CVP_LDAC]) {
            kind_error(ctx, "\"dac.ldac\" \"pin\" needs \"pins.ldac\"");
            return NULL;
        }
    } else if (load_clock(&ld, member(json, "clock"), dev) != 0) {
        return NULL;
    }
    if (kind_voltage(ctx, json, NULL, &dev->vcc) != 0) {
        return NULL;
    }
    dev->space_count = ld.space_count;
    dev->names = ld.names;
    dev->name_count = ld.name_count;
    return dev;
}

/* ---- Pinouts and parts ------------------------------------------------------------------- */

static int is_strap(const CvDevice *dev, int id) {
    unsigned i;

    for (i = 0; i < dev->bus.strap_count; i++) {
        if (dev->bus.straps[i] == id) {
            return 1;
        }
    }
    for (i = 0; dev->has_config_bus && i < dev->config_bus.strap_count; i++) {
        if (dev->config_bus.straps[i] == id) {
            return 1;
        }
    }
    return 0;
}

void *cv_bind_pinout(KindContext *ctx, const void *device, const PartPin *pins, size_t pin_count) {
    const CvDevice *dev = device;
    CvBound *b = kind_alloc(ctx, sizeof(*b));
    size_t *idx;
    uint8_t *analog, *od;
    size_t i, j;

    if (b == NULL) {
        return NULL;
    }
    idx = kind_alloc(ctx, (dev->name_count ? dev->name_count : 1) * sizeof(*idx));
    analog = kind_alloc(ctx, pin_count ? pin_count : 1);
    od = kind_alloc(ctx, pin_count ? pin_count : 1);
    if (idx == NULL || analog == NULL || od == NULL) {
        return NULL;
    }
    for (i = 0; i < dev->name_count; i++) {
        idx[i] = SIZE_MAX;
        for (j = 0; j < pin_count; j++) {
            if (strcmp(pins[j].name, dev->names[i]) == 0) {
                idx[i] = j;
                break;
            }
        }
        if (idx[i] == SIZE_MAX && !is_strap(dev, (int)i)) {
            kind_error(ctx, "no pin \"%s\" in this pinout", dev->names[i]);
            return NULL;
        }
    }
    for (i = 0; i < dev->analog_count; i++) {
        for (j = 0; j < pin_count; j++) {
            if (strcmp(pins[j].name, dev->names[dev->analog[i]]) == 0) {
                analog[j] = 1;
            }
        }
    }
    if (dev->bus.sda != CV_NONE && idx[dev->bus.sda] != SIZE_MAX) {
        od[idx[dev->bus.sda]] = 1;
    }
    if (dev->has_config_bus && dev->config_bus.sda != CV_NONE &&
        idx[dev->config_bus.sda] != SIZE_MAX) {
        od[idx[dev->config_bus.sda]] = 1;
    }
    for (i = 0; i < dev->ready_count; i++) {
        if (dev->ready[i].open_drain && idx[dev->ready[i].pin] != SIZE_MAX) {
            od[idx[dev->ready[i].pin]] = 1;
        }
    }
    b->dev = dev;
    b->pin = idx;
    b->pins = pins;
    b->pin_count = pin_count;
    b->analog = analog;
    b->open_drain = od;
    return b;
}

int cv_load_part(KindContext *ctx, const void *device, const void *bound, const cJSON *json,
                 Part *out) {
    const CvDevice *dev = device;
    const CvBound *b = bound;
    CvPart *data = kind_alloc(ctx, sizeof(*data));
    const cJSON *v;
    Ld ld;

    if (data == NULL) {
        return -1;
    }
    memset(&ld, 0, sizeof ld);
    ld.ctx = ctx;
    ld.dev = (CvDevice *)dev;
    ld.regs = (CvRegister *)dev->regs;
    ld.reg_count = dev->reg_count;
    data->i2c_address = CV_NONE;
    if ((v = member(json, "i2cAddress")) != NULL) {
        if (!cJSON_IsNumber(v) || v->valueint < 0 || v->valueint > 127) {
            return kind_error(ctx, "\"i2cAddress\" is a 7-bit address");
        }
        data->i2c_address = v->valueint;
    }
    if ((v = member(json, "i2c")) != NULL) {
        static const char *const IK[] = {"address", "table", NULL};
        size_t k;

        /* names resolve against this pinout directly */
        if (check_keys(&ld, v, IK) != 0) {
            return -1;
        }
        {
            const cJSON *a = member(v, "address"), *e;
            size_t n = 0;

            if (!cJSON_IsArray(a) || cJSON_GetArraySize(a) > SERIAL_I2C_STRAPS) {
                return kind_error(ctx, "\"i2c\": \"address\" is an array of strap pins");
            }
            cJSON_ArrayForEach(e, a) {
                if (!cJSON_IsString(e) || kind_pin(ctx, b->pins, b->pin_count, e->valuestring,
                                                   &k) != 0) {
                    return kind_failed(ctx) ? -1 : kind_error(ctx, "\"i2c\": strap pin names");
                }
                data->straps[n++] = (int)k;
            }
            data->strap_count = (unsigned)n;
        }
        {
            const cJSON *t = member(v, "table"), *e;
            size_t n = 0;

            data->table_count = 1u << (2 * data->strap_count);
            if (!cJSON_IsArray(t) || (unsigned)cJSON_GetArraySize(t) != data->table_count) {
                return kind_error(ctx, "\"i2c\": \"table\" needs %u entries", data->table_count);
            }
            cJSON_ArrayForEach(e, t) {
                data->table[n++] = cJSON_IsNumber(e) ? e->valueint : -1;
            }
        }
        data->has_bus_i2c = 1;
    }
    if ((v = member(json, "resets")) != NULL) {
        const cJSON *e;
        size_t n = 0;
        CvRegValue *rv;

        if (!cJSON_IsObject(v)) {
            return kind_error(ctx, "\"resets\" is an object of register values");
        }
        data->reset_count = (size_t)cJSON_GetArraySize(v);
        rv = kind_alloc(ctx, (data->reset_count ? data->reset_count : 1) * sizeof(*rv));
        if (rv == NULL) {
            return -1;
        }
        cJSON_ArrayForEach(e, v) {
            CvField f;

            if (field_ref(&ld, e->string, &f) != 0 || f.reg < 0 ||
                u32(&ld, e, "a reset value", &rv[n].value) != 0) {
                return -1;
            }
            rv[n++].reg = f.reg;
        }
        data->resets = rv;
    }
    if (!dev->rtc) {
        out->converter = &dev->info;
    }
    kind_set_part_data(out, data);
    return kind_part_voltage(ctx, json, dev->vcc, &out->vcc);
}
