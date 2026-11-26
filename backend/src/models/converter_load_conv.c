/* models/converter_load_conv.c: part of the model in models/converter_load.c (split by plan-68-I/J). */
#include "converter_load_int.h"
/* ---- Conversion, ready, pins, DAC, choices ---------------------------------------------- */

int cvl_load_choice(Ld *ld, const cJSON *v, CvChoice *out, int formats) {
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
        return cvl_u32(ld, v, "\"clearCode\"", &out->fixed);
    }
    if (cJSON_IsObject(v)) {
        static const char *const KEYS[] = {"pin", "field", "values", NULL};
        const cJSON *vals = cvl_member(v, "values");
        const cJSON *e;
        size_t n = 0;
        uint32_t *keys, *values;

        if (cvl_check_keys(ld, v, KEYS) != 0) {
            return -1;
        }
        if (cvl_member(v, "pin") != NULL) {
            if (cvl_pin_member(ld, v, "pin", 1, &out->pin) != 0) {
                return -1;
            }
        } else if (cvl_member(v, "field") != NULL) {
            out->has_field = 1;
            if (cvl_field_member(ld, v, "field", &out->field) != 0) {
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
            } else if (cvl_u32(ld, e, "a \"clearCode\" value", &values[n]) != 0) {
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
    const cJSON *v = cvl_member(o, key);
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
        if (cvl_field_ref(ld, e->string, &s[n].field) != 0 || s[n].field.reg < 0 ||
            cvl_u32(ld, e, key, &s[n].value.value) != 0) {
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
        if (cvl_cv_ns(ld->ctx, v, "\"time\"", &out->fixed_ps) != 0) {
            return -1;
        }
        out->max_ps = out->fixed_ps;
        return 0;
    }
    if (cJSON_IsObject(v)) {
        static const char *const KEYS[] = {"field", "fields", "values", NULL};
        const cJSON *f = cvl_member(v, "fields"), *vals = cvl_member(v, "values");
        const cJSON *e;
        CvField *fields;
        uint32_t *keys;
        uint64_t *ps;
        size_t nf, n = 0;

        if (cvl_check_keys(ld, v, KEYS) != 0) {
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
                if (!cJSON_IsString(e) || cvl_field_ref(ld, e->valuestring, &fields[i++]) != 0) {
                    return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"fields\" are names");
                }
            }
        } else if (cvl_field_member(ld, v, "field", &fields[0]) != 0) {
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
            if (cvl_cv_ns(ld->ctx, e, "a time", &ps[n]) != 0) {
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

int cvl_load_conversion(Ld *ld, const cJSON *c, CvConversion *out) {
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
    if (cvl_check_keys(ld, c, KEYS) != 0 || load_time(ld, cvl_member(c, "time"), &out->time) != 0 ||
        sets_member(ld, c, "busy", &out->busy, &out->busy_count) != 0 ||
        sets_member(ld, c, "done", &out->done, &out->done_count) != 0 ||
        sets_member(ld, c, "onRead", &out->on_read, &out->on_read_count) != 0) {
        return -1;
    }
    if ((v = cvl_member(c, "start")) != NULL) {
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

                    if (cvl_field_ref(ld, s + 6, &f) != 0 || f.reg < 0) {
                        return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"write:REG\"");
                    }
                    w[n].any = 1;
                    w[n].reg = f.reg;
                } else if (cvl_cond_text(ld, s + 6, &w[n].cond) != 0) {
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
    if ((v = cvl_member(c, "mode")) != NULL) {
        if (cJSON_IsString(v)) {
            if (strcmp(v->valuestring, "continuous") == 0) {
                out->mode = 1;
            } else if (strcmp(v->valuestring, "single") != 0) {
                return kind_error(ld->ctx, "\"mode\" is single, continuous or {field}");
            }
        } else if (cJSON_IsObject(v)) {
            static const char *const MK[] = {"field", "continuous", "single", "startOnEnter", NULL};
            const cJSON *se = cvl_member(v, "startOnEnter");

            out->mode = 2;
            if (cvl_check_keys(ld, v, MK) != 0 || cvl_field_member(ld, v, "field", &out->mode_field) != 0) {
                return -1;
            }
            if (cvl_member(v, "continuous") != NULL) {
                if (cvl_u32_member(ld, v, "continuous", &out->mode_value) != 0) {
                    return -1;
                }
            } else if (cvl_member(v, "single") != NULL) {
                out->mode_single = 1;
                if (cvl_u32_member(ld, v, "single", &out->mode_value) != 0) {
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
    if (cvl_member(c, "channel") != NULL) {
        out->has_channel = 1;
        if (cvl_field_member(ld, c, "channel", &out->channel) != 0) {
            return -1;
        }
    }
    kind_pop(ld->ctx);
    return 0;
}

int cvl_load_ready(Ld *ld, const cJSON *arr, CvDevice *dev) {
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
        const cJSON *cl = cvl_member(r, "clears");

        kind_push(ld->ctx, "ready[%zu]", i);
        if (cvl_check_keys(ld, r, KEYS) != 0 || cvl_pin_member(ld, r, "pin", 1, &rd[i].pin) != 0 ||
            cvl_level_member(ld, r, "active", 0, &rd[i].active) != 0 ||
            kind_string(ld->ctx, r, "asserts", &as) != 0 ||
            cvl_conds(ld, cvl_member(r, "enable"), &rd[i].enable) != 0) {
            return -1;
        }
        if (strcmp(as, "done") == 0) {
            rd[i].asserts_done = 1;
        } else if (strcmp(as, "busy") != 0) {
            return kind_error(ld->ctx, "\"asserts\" is \"busy\" or \"done\"");
        }
        rd[i].open_drain = cJSON_IsTrue(cvl_member(r, "openDrain"));
        rd[i].on_miso = cJSON_IsTrue(cvl_member(r, "onMiso"));
        if (cl == NULL) {
            rd[i].clears = CVC_START;
        } else {
            const cJSON *e;
            const cJSON *first = cJSON_IsArray(cl) ? cl->child : cl;

            for (e = first; e != NULL; e = cJSON_IsArray(cl) ? e->next : NULL) {
                if (cJSON_IsNumber(e)) {
                    rd[i].clears |= CVC_PULSE;
                    if (cvl_cv_ns(ld->ctx, e, "\"clears\"", &rd[i].pulse_ps) != 0) {
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

int cvl_load_pins(Ld *ld, const cJSON *p, CvPins *out) {
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
    if (cvl_check_keys(ld, p, KEYS) != 0) {
        return -1;
    }
    for (i = 0; i < CVP_COUNT; i++) {
        static const char *const PK[] = {"pin", "active", "edge", NULL};
        const char *edge = NULL;

        if ((v = cvl_member(p, NAMES[i])) == NULL) {
            continue;
        }
        kind_push(ld->ctx, "%s", NAMES[i]);
        out->has[i] = 1;
        if (cvl_check_keys(ld, v, PK) != 0 || cvl_pin_member(ld, v, "pin", 1, &out->ctl[i].pin) != 0 ||
            cvl_level_member(ld, v, "active", i == CVP_START ? 1 : 0, &out->ctl[i].active) != 0 ||
            (cvl_member(v, "edge") && kind_string(ld->ctx, v, "edge", &edge))) {
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
    if ((v = cvl_member(p, "levels")) != NULL) {
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
            if (cvl_pin_member(ld, e, "pin", 1, &lv[n].pin) != 0 ||
                cvl_field_member(ld, e, "field", &lv[n].field) != 0 || lv[n].field.reg < 0) {
                return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"levels\": a field");
            }
            n++;
        }
        out->levels = lv;
    }
    if ((v = cvl_member(p, "resetSelect")) != NULL) {
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
            const cJSON *vals = cvl_member(e, "resets");
            const cJSON *x;
            size_t k = 0;
            CvRegValue *rv;

            if (cvl_pin_member(ld, e, "pin", 1, &rs[n].pin) != 0 ||
                cvl_level_member(ld, e, "level", 1, &rs[n].level) != 0 || !cJSON_IsObject(vals)) {
                return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"resetSelect\" entries");
            }
            rs[n].count = (size_t)cJSON_GetArraySize(vals);
            rv = kind_alloc(ld->ctx, (rs[n].count ? rs[n].count : 1) * sizeof(*rv));
            if (rv == NULL) {
                return -1;
            }
            cJSON_ArrayForEach(x, vals) {
                CvField f;

                if (cvl_field_ref(ld, x->string, &f) != 0 || f.reg < 0 ||
                    cvl_u32(ld, x, "a reset value", &rv[k].value) != 0) {
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

        if (!cJSON_IsString(e) || cvl_field_ref(ld, e->valuestring, &f) != 0 || f.reg < 0) {
            return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"%s\" names registers", key);
        }
        idx[n++] = f.reg;
    }
    *out = idx;
    return 0;
}

int cvl_load_dac(Ld *ld, const cJSON *d, CvDac *out) {
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
    if (cvl_check_keys(ld, d, KEYS) != 0 || reg_names(ld, cvl_member(d, "input"), "input", &in, &ni) != 0 ||
        reg_names(ld, cvl_member(d, "output"), "output", &outp, &no) != 0 ||
        cvl_load_choice(ld, cvl_member(d, "clearCode"), &out->clear_code, 0) != 0 ||
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


