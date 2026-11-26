#include "converter_load_int.h"

/* A time in ns, as ps. Converter and RTC times reach minutes (slow data rates,
   watchdogs, periodic interrupts), past kind_ns's 1 s limit. */
int cvl_cv_ns(KindContext *ctx, const cJSON *value, const char *what, uint64_t *ps) {
    if (!cJSON_IsNumber(value) || !(value->valuedouble >= 0) || value->valuedouble > 1e13) {
        return kind_error(ctx, "%s must be a time in ns from 0 to 1e13", what);
    }
    *ps = (uint64_t)llround(value->valuedouble * 1000.0);
    return 0;
}

const cJSON *cvl_member(const cJSON *o, const char *key) {
    return cJSON_IsObject(o) ? cJSON_GetObjectItemCaseSensitive(o, key) : NULL;
}

int cvl_check_keys(Ld *ld, const cJSON *o, const char *const *keys) {
    static const char *const NOTE[] = {"note", NULL};

    return kind_check_keys(ld->ctx, o, keys, NOTE);
}

uint32_t cvl_mask_of(unsigned bits) {
    return bits >= 32 ? 0xFFFFFFFFu : ((1u << bits) - 1u);
}

int cvl_u32(Ld *ld, const cJSON *v, const char *what, uint32_t *out) {
    if (!cJSON_IsNumber(v) || v->valuedouble < 0 || v->valuedouble > 4294967295.0 ||
        v->valuedouble != (double)(uint32_t)v->valuedouble) {
        return kind_error(ld->ctx, "%s must be a whole number 0-4294967295", what);
    }
    *out = (uint32_t)v->valuedouble;
    return 0;
}

int cvl_u32_member(Ld *ld, const cJSON *o, const char *key, uint32_t *out) {
    const cJSON *v = cvl_member(o, key);

    if (v == NULL) {
        return 0;
    }
    return cvl_u32(ld, v, key, out);
}

int cvl_level_member(Ld *ld, const cJSON *o, const char *key, int dflt, int *out) {
    uint32_t v = (uint32_t)dflt;

    if (cvl_u32_member(ld, o, key, &v) != 0) {
        return -1;
    }
    if (v > 1) {
        return kind_error(ld->ctx, "\"%s\" must be 0 or 1", key);
    }
    *out = (int)v;
    return 0;
}

/* Name ids: every pin name a key uses. */
int cvl_name_id(Ld *ld, const char *name) {
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

int cvl_pin_member(Ld *ld, const cJSON *o, const char *key, int required, int *out) {
    const cJSON *v = cvl_member(o, key);

    *out = CV_NONE;
    if (v == NULL) {
        return required ? kind_error(ld->ctx, "missing \"%s\"", key) : 0;
    }
    if (!cJSON_IsString(v) || v->valuestring[0] == '\0') {
        return kind_error(ld->ctx, "\"%s\" must be a pin name", key);
    }
    *out = cvl_name_id(ld, v->valuestring);
    return *out == CV_NONE ? -1 : 0;
}

int cvl_space_index(Ld *ld, const char *name) {
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

int cvl_field_ref(Ld *ld, const char *text, CvField *out) {
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

int cvl_field_member(Ld *ld, const cJSON *o, const char *key, CvField *out) {
    const char *s;

    if (kind_string(ld->ctx, o, key, &s) != 0) {
        return -1;
    }
    return cvl_field_ref(ld, s, out);
}

int cvl_cond_text(Ld *ld, const char *s, CvCond *out) {
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
    return cvl_field_ref(ld, buf, &out->field);
}

int cvl_conds(Ld *ld, const cJSON *v, CvConds *out) {
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
        if (items == NULL || cvl_cond_text(ld, v->valuestring, items) != 0) {
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
        if (!cJSON_IsString(item) || cvl_cond_text(ld, item->valuestring, &items[i++]) != 0) {
            return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "a condition must be a string");
        }
    }
    out->items = items;
    out->count = n;
    return 0;
}

/* A capture letter or a number. */
int cvl_value_of(Ld *ld, const cJSON *v, const char *key, CvValue *out) {
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
    return cvl_u32(ld, v, key, &out->value);
}

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
int cvl_pattern(Ld *ld, const char *text, int in, CvPattern *out, uint32_t *caps) {
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
            if (cvl_field_ref(ld, name, &f) != 0) {
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

int cvl_pattern_member(Ld *ld, const cJSON *o, const char *key, int in, CvPattern *out,
                          uint32_t *caps) {
    const char *s;

    if (kind_string(ld->ctx, o, key, &s) != 0) {
        return -1;
    }
    return cvl_pattern(ld, s, in, out, caps);
}

/* ---- The device ------------------------------------------------------------------------ */

CvDevice *cv_load_device(KindContext *ctx, const cJSON *json, CvMode mode) {
    int rtc = mode == CV_MODE_RTC, display = mode == CV_MODE_DISPLAY;
    int converter = mode == CV_MODE_CONVERTER;
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
    ld.display = display;
    dev->rtc = rtc;
    if (display && cvl_member(json, "display") == NULL) {
        kind_error(ctx, "a display driver needs \"display\"");
        return NULL;
    }
    if (converter) {
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
    if (cvl_load_registers(&ld, cvl_member(json, "registers")) != 0 ||
        (rtc && cvl_load_ram(&ld, cvl_member(json, "clock")) != 0) ||
        (display && cvl_load_display_ram(&ld, cvl_member(json, "display")) != 0)) {
        return NULL;
    }
    dev->regs = ld.regs;
    dev->reg_count = ld.reg_count;
    if (converter && !dev->dac) {
        if (cvl_member(json, "codeFormat") == NULL) {
            kind_error(ctx, "an ADC needs \"codeFormat\"");
            return NULL;
        }
        kind_push(ctx, "codeFormat");
        if (cvl_load_choice(&ld, cvl_member(json, "codeFormat"), &dev->code, 1) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    if ((v = cvl_member(json, "analog")) != NULL) {
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
            a[n++] = cvl_name_id(&ld, e->valuestring);
        }
        dev->analog = a;
    }
    kind_push(ctx, "bus");
    if (cvl_load_bus(&ld, cvl_member(json, "bus"), &dev->bus, 0) != 0) {
        return NULL;
    }
    kind_pop(ctx);
    if ((v = cvl_member(json, "configBus")) != NULL) {
        kind_push(ctx, "configBus");
        dev->has_config_bus = 1;
        if (cvl_load_bus(&ld, v, &dev->config_bus, 1) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    if ((v = cvl_member(json, "timing")) != NULL) {
        static const char *const TK[] = {"tValid", "tOut", "tFloat", "tAcc", "tDf", NULL};
        uint64_t *t[5];

        t[0] = &dev->timing.t_valid;
        t[1] = &dev->timing.t_out;
        t[2] = &dev->timing.t_float;
        t[3] = &dev->timing.t_acc;
        t[4] = &dev->timing.t_df;
        if (cvl_check_keys(&ld, v, TK) != 0) {
            return NULL;
        }
        for (i = 0; i < 5; i++) {
            const cJSON *x = cvl_member(v, TK[i]);

            if (x != NULL && cvl_cv_ns(ctx, x, TK[i], t[i]) != 0) {
                return NULL;
            }
        }
    }
    if ((dev->bus.style == CVB_NONE ? (memset(&dev->protocol, 0, sizeof dev->protocol), 0)
                                     : cvl_load_protocol(&ld, cvl_member(json, "protocol"), &dev->protocol)) != 0 ||
        cvl_load_conversion(&ld, cvl_member(json, "conversion"), &dev->conversion) != 0 ||
        cvl_load_ready(&ld, cvl_member(json, "ready"), dev) != 0 ||
        cvl_load_pins(&ld, cvl_member(json, "pins"), &dev->pins) != 0) {
        return NULL;
    }
    if (converter) {
        if (cvl_load_dac(&ld, cvl_member(json, "dac"), &dev->dacinfo) != 0) {
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
    } else if (rtc) {
        if (cvl_load_clock(&ld, cvl_member(json, "clock"), dev) != 0) {
            return NULL;
        }
    } else if (cvl_load_display(&ld, cvl_member(json, "display"), dev) != 0) {
        return NULL;
    }
    if (dev->disp != NULL) {
        size_t ci, ti;

        for (ci = 0; ci < dev->protocol.command_count; ci++) {
            const CvCommand *cmd = &dev->protocol.commands[ci];
            int uses = cmd->action == CVA_LCDLOAD;

            for (ti = 0; ti < cmd->then_count; ti++) {
                uses |= cmd->then[ti] == CVA_LCDLOAD;
            }
            if (uses && !(dev->disp->driver == DISP_LCD && dev->disp->has_load)) {
                kind_error(ctx, "action \"lcdload\" needs an lcd \"display\" with a \"load\" section");
                return NULL;
            }
        }
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
    uint8_t *analog, *od, *tri;
    size_t i, j;

    if (b == NULL) {
        return NULL;
    }
    idx = kind_alloc(ctx, (dev->name_count ? dev->name_count : 1) * sizeof(*idx));
    analog = kind_alloc(ctx, pin_count ? pin_count : 1);
    od = kind_alloc(ctx, pin_count ? pin_count : 1);
    tri = kind_alloc(ctx, pin_count ? pin_count : 1);
    if (idx == NULL || analog == NULL || od == NULL || tri == NULL) {
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
    for (i = 0; dev->disp != NULL && i < dev->disp->open_drain_count; i++) {
        if (idx[dev->disp->open_drain[i]] != SIZE_MAX) {
            od[idx[dev->disp->open_drain[i]]] = 1;
        }
    }
    /* A parallel bus with an "oe" releases its data pins (clocked_pin: release_data). */
    for (i = 0; dev->bus.has_oe && i < dev->bus.data_count; i++) {
        if (dev->bus.data[i] != CV_NONE && idx[dev->bus.data[i]] != SIZE_MAX) {
            tri[idx[dev->bus.data[i]]] = 1;
        }
    }
    b->dev = dev;
    b->pin = idx;
    b->pins = pins;
    b->pin_count = pin_count;
    b->analog = analog;
    b->open_drain = od;
    b->tristate = tri;
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
    if ((v = cvl_member(json, "i2cAddress")) != NULL) {
        if (!cJSON_IsNumber(v) || v->valueint < 0 || v->valueint > 127) {
            return kind_error(ctx, "\"i2cAddress\" is a 7-bit address");
        }
        data->i2c_address = v->valueint;
    }
    if ((v = cvl_member(json, "i2c")) != NULL) {
        static const char *const IK[] = {"address", "table", NULL};
        size_t k;

        /* names resolve against this pinout directly */
        if (cvl_check_keys(&ld, v, IK) != 0) {
            return -1;
        }
        {
            const cJSON *a = cvl_member(v, "address"), *e;
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
            const cJSON *t = cvl_member(v, "table"), *e;
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
    if ((v = cvl_member(json, "resets")) != NULL) {
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

            if (cvl_field_ref(&ld, e->string, &f) != 0 || f.reg < 0 ||
                cvl_u32(&ld, e, "a reset value", &rv[n].value) != 0) {
                return -1;
            }
            rv[n++].reg = f.reg;
        }
        data->resets = rv;
    }
    if (!dev->rtc && dev->disp == NULL) {
        out->converter = &dev->info;
    }
    kind_set_part_data(out, data);
    return kind_part_voltage(ctx, json, dev->vcc, &out->vcc);
}
