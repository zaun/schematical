/* models/converter_load_rtc.c: part of the model in models/converter_load.c (split by plan-68-I/J). */
#include "converter_load_int.h"
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
        out->pin = cvl_name_id(ld, v->valuestring);
        out->open_drain = -1;       /* attributes from another use of the pin */
        return out->pin == CV_NONE ? -1 : 0;
    }
    if (cJSON_IsObject(v)) {
        static const char *const K[] = {"pin", "active", "openDrain", "pulse", NULL};

        if (cvl_check_keys(ld, v, K) != 0 || cvl_pin_member(ld, v, "pin", 1, &out->pin) != 0 ||
            cvl_level_member(ld, v, "active", 0, &out->active) != 0) {
            return -1;
        }
        out->open_drain = cJSON_IsTrue(cvl_member(v, "openDrain"));
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
        if (cvl_cv_ns(ld->ctx, e, key, &t[n]) != 0) {
            return -1;
        }
        n++;
    }
    *keys = k;
    *ps = t;
    return 0;
}

/* A register or field of the clock: R and CH (the conversion result and channel) hold no time. */
static int reg_field(Ld *ld, const cJSON *o, const char *key, CvField *out) {
    if (cvl_field_member(ld, o, key, out) != 0) {
        return -1;
    }
    return out->reg < 0 ? kind_error(ld->ctx, "\"%s\" names a register or a field, not R or CH", key) : 0;
}

static int opt_field(Ld *ld, const cJSON *o, const char *key, int *has, CvField *out) {
    *has = cvl_member(o, key) != NULL;
    return *has ? reg_field(ld, o, key, out) : 0;
}

static int load_periodic(Ld *ld, const cJSON *e, RtcPeriodic *p) {
    static const char *const K[] = {"field", "periods", "periodNs", "flag", "enable", "summary",
                                    "pin", NULL};

    memset(p, 0, sizeof *p);
    if (cvl_check_keys(ld, e, K) != 0 || opt_field(ld, e, "flag", &p->has_flag, &p->flag) != 0 ||
        opt_field(ld, e, "enable", &p->has_enable, &p->enable) != 0 ||
        opt_field(ld, e, "summary", &p->has_summary, &p->summary) != 0 ||
        rtc_pin(ld, cvl_member(e, "pin"), &p->pin) != 0) {
        return -1;
    }
    if (cvl_member(e, "periodNs") != NULL) {
        p->fixed = 1;
        return cvl_cv_ns(ld->ctx, cvl_member(e, "periodNs"), "\"periodNs\"", &p->period_ps);
    }
    return reg_field(ld, e, "field", &p->rate) ||
           ns_map(ld, cvl_member(e, "periods"), "periods", &p->keys, &p->ps, &p->count);
}

int cvl_load_clock(Ld *ld, const cJSON *c, CvDevice *dev) {
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
    if (cvl_check_keys(ld, c, KEYS) != 0) {
        return -1;
    }
    /* fields */
    v = cvl_member(c, "fields");
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
            const cJSON *f = cvl_member(e, "format");
            uint32_t first = 0, base = 2000;

            kind_push(ld->ctx, "fields[%zu]", n);
            if (cvl_check_keys(ld, e, FK) != 0 || reg_field(ld, e, "field", &tf[n].field) != 0 ||
                unit_of(ld, e, &tf[n].unit) != 0 ||
                opt_field(ld, e, "h12", &tf[n].has_h12, &tf[n].h12) != 0 ||
                opt_field(ld, e, "pm", &tf[n].has_pm, &tf[n].pm) != 0 ||
                cvl_level_member(ld, e, "h12Level", 1, &tf[n].h12_level) != 0 ||
                cvl_u32_member(ld, e, "first", &first) != 0 || cvl_u32_member(ld, e, "base", &base) != 0) {
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
                    if (reg_field(ld, f, "field", &tf[n].format.field) != 0 ||
                        cvl_u32_member(ld, f, "binary", &tf[n].format.binary_value) != 0) {
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
    if ((v = cvl_member(c, "stop")) != NULL) {
        static const char *const SK[] = {"field", "stopped", "running", "powerOn", NULL};

        kind_push(ld->ctx, "stop");
        if (cvl_check_keys(ld, v, SK) != 0 || reg_field(ld, v, "field", &k->stop) != 0) {
            return -1;
        }
        k->has_stop = 1;
        if (cvl_member(v, "running") != NULL) {
            k->stop_running = 1;
            if (cvl_u32_member(ld, v, "running", &k->stop_value) != 0) {
                return -1;
            }
        } else {
            k->stop_value = 1;
            if (cvl_u32_member(ld, v, "stopped", &k->stop_value) != 0) {
                return -1;
            }
        }
        kind_pop(ld->ctx);
    }
    /* update */
    if ((v = cvl_member(c, "update")) != NULL && !(cJSON_IsString(v) && strcmp(v->valuestring, "none") == 0)) {
        static const char *const UK[] = {"uip", "inhibit", "latch", "write", "writeHalts", NULL};

        kind_push(ld->ctx, "update");
        if (!cJSON_IsObject(v) || cvl_check_keys(ld, v, UK) != 0) {
            return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"update\" is none, {uip}, {latch}");
        }
        if (cvl_member(v, "uip") != NULL) {
            k->update.kind = RTC_UPDATE_UIP;
            if (reg_field(ld, v, "uip", &k->update.uip) != 0 ||
                opt_field(ld, v, "inhibit", &k->update.has_inhibit, &k->update.inhibit) != 0) {
                return -1;
            }
        } else if (cvl_member(v, "latch") != NULL) {
            const cJSON *wh = cvl_member(v, "writeHalts");

            k->update.kind = RTC_UPDATE_LATCH;
            if (reg_field(ld, v, "latch", &k->update.latch) != 0 ||
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
    if ((v = cvl_member(c, "alarms")) != NULL) {
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
            const cJSON *m = cvl_member(e, "match"), *x;
            RtcMatch *mm;
            size_t j = 0;

            kind_push(ld->ctx, "alarms[%zu]", i);
            if (cvl_check_keys(ld, e, AK) != 0 || !cJSON_IsArray(m) ||
                opt_field(ld, e, "flag", &al[i].has_flag, &al[i].flag) != 0 ||
                opt_field(ld, e, "enable", &al[i].has_enable, &al[i].enable) != 0 ||
                rtc_pin(ld, cvl_member(e, "pin"), &al[i].pin) != 0) {
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
                const cJSON *dc = cvl_member(x, "dontCare");

                if (cvl_check_keys(ld, x, MK) != 0 || reg_field(ld, x, "field", &mm[j].field) != 0 ||
                    unit_of(ld, x, &mm[j].unit) != 0 ||
                    opt_field(ld, x, "mask", &mm[j].has_mask, &mm[j].mask) != 0 ||
                    cvl_level_member(ld, x, "maskLevel", 1, &mm[j].mask_level) != 0) {
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
    if ((v = cvl_member(c, "periodic")) != NULL) {
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
    if ((v = cvl_member(c, "watchdog")) != NULL) {
        static const char *const WK[] = {"field", "periods", "kick", "flag", "pin", "resetPin",
                                         "unitNs", NULL};
        RtcWatchdog *w = &k->watchdog;
        const cJSON *kk = cvl_member(v, "kick"), *rp = cvl_member(v, "resetPin");

        kind_push(ld->ctx, "watchdog");
        w->present = 1;
        if (cvl_check_keys(ld, v, WK) != 0 || reg_field(ld, v, "field", &w->field) != 0 ||
            ns_map(ld, cvl_member(v, "periods"), "periods", &w->keys, &w->ps, &w->count) != 0 ||
            opt_field(ld, v, "flag", &w->has_flag, &w->flag) != 0 ||
            rtc_pin(ld, cvl_member(v, "pin"), &w->pin) != 0 || rtc_pin(ld, rp, &w->reset_pin) != 0) {
            return -1;
        }
        if (kk != NULL) {
            const char *edge = "both";

            w->has_kick = 1;
            if (cvl_pin_member(ld, kk, "pin", 1, &w->kick_pin) != 0 ||
                (cvl_member(kk, "edge") && kind_string(ld->ctx, kk, "edge", &edge))) {
                return -1;
            }
            w->kick_edge = strcmp(edge, "rise") == 0 ? 1 : strcmp(edge, "fall") == 0 ? 0 : 2;
        }
        if (rp != NULL && cvl_member(rp, "pulse") != NULL &&
            ns_map(ld, cvl_member(rp, "pulse"), "pulse", &w->pulse_keys, &w->pulse_ps,
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
int cvl_load_ram(Ld *ld, const cJSON *c) {
    const cJSON *r = cvl_member(c, "ram");
    uint32_t bytes = 0, address = 0, i;
    const char *space = "reg";
    CvRegister *regs;
    int sp;

    if (r == NULL) {
        return 0;
    }
    if (cvl_u32_member(ld, r, "bytes", &bytes) != 0 || cvl_u32_member(ld, r, "address", &address) != 0 ||
        (cvl_member(r, "space") && kind_string(ld->ctx, r, "space", &space))) {
        return -1;
    }
    if (bytes == 0 || bytes > 65536) {
        return kind_error(ld->ctx, "clock.ram: \"bytes\" is 1-65536");
    }
    if ((sp = cvl_space_index(ld, space)) < 0) {
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
        if (g->name == NULL) {
            return -1;
        }
        g->address = (int32_t)(address + i);
        g->space = sp;
        g->width = 8;
        g->result_channel = CV_NONE;
    }
    ld->regs = regs;
    ld->reg_count += bytes;
    return 0;
}

