/* models/converter_load_proto.c: part of the model in models/converter_load.c (split by plan-68-I/J). */
#include "converter_load_int.h"
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
        const cJSON *b = cvl_member(f, "bits");
        size_t q;
        uint32_t m;

        if (cvl_check_keys(ld, f, FK) != 0 || kind_string(ld->ctx, f, "name", &fs[k].name) != 0) {
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
        m = cvl_mask_of(fs[k].hi + 1) & ~cvl_mask_of(fs[k].lo);
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

int cvl_load_registers(Ld *ld, const cJSON *arr) {
    static const char *const KEYS[] = {"name", "address", "space", "width", "reset", "readOnly",
                                       "fields", "result", "resultBits", "channel",
                                       "clearOnRead", "writeOneClears", NULL};
    const cJSON *r;
    size_t n, i = 0, q;

    ld->regs = NULL;
    ld->reg_count = 0;
    if (cvl_space_index(ld, "reg") != 0) {
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
        if (cvl_check_keys(ld, r, KEYS) != 0 || kind_string(ld->ctx, r, "name", &g->name) != 0 ||
            kind_uint(ld->ctx, r, "width", 32, &w) != 0) {
            return -1;
        }
        if (w == 0) {
            return kind_error(ld->ctx, "\"width\" must be 1-32");
        }
        g->width = (unsigned)w;
        m = cvl_mask_of(g->width);
        g->address = CV_NONE;
        g->result_channel = CV_NONE;
        if ((v = cvl_member(r, "address")) != NULL) {
            uint32_t a;

            if (cvl_u32(ld, v, "\"address\"", &a) != 0) {
                return -1;
            }
            g->address = (int32_t)a;
        }
        if ((v = cvl_member(r, "space")) != NULL) {
            if (!cJSON_IsString(v)) {
                return kind_error(ld->ctx, "\"space\" must be a string");
            }
            if ((g->space = cvl_space_index(ld, v->valuestring)) < 0) {
                return -1;
            }
        }
        if (cvl_u32_member(ld, r, "reset", &g->reset) != 0 ||
            cvl_u32_member(ld, r, "clearOnRead", &g->cor) != 0 ||
            cvl_u32_member(ld, r, "writeOneClears", &g->w1c) != 0) {
            return -1;
        }
        if ((v = cvl_member(r, "readOnly")) != NULL) {
            if (cJSON_IsBool(v)) {
                g->ro = cJSON_IsTrue(v) ? m : 0;
            } else if (cvl_u32(ld, v, "\"readOnly\"", &g->ro) != 0) {
                return -1;
            }
        }
        if ((g->reset | g->ro | g->cor | g->w1c) & ~m) {
            return kind_error(ld->ctx, "a value is wider than the register's %u bits", g->width);
        }
        if ((v = cvl_member(r, "fields")) != NULL && load_fields(ld, v, g) != 0) {
            return -1;
        }
        if ((v = cvl_member(r, "result")) != NULL) {
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
        if ((v = cvl_member(r, "resultBits")) != NULL) {
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
        if ((v = cvl_member(r, "channel")) != NULL) {
            uint32_t c;

            if (cvl_u32(ld, v, "\"channel\"", &c) != 0) {
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
    "wakeup", "update", "updateall", "clear", "setbits", "clearbits", "lcdload", NULL
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
    if (cvl_check_keys(ld, c, KEYS) != 0 || cvl_pattern_member(ld, c, "in", 1, &cmd->in, &caps) != 0) {
        return -1;
    }
    if (cmd->in.bits == 0) {
        return kind_error(ld->ctx, "\"in\" is empty");
    }
    if ((v = cvl_member(c, "name")) != NULL && cJSON_IsString(v)) {
        cmd->name = kind_strdup(ld->ctx, v->valuestring);
    }
    if ((v = cvl_member(c, "action")) != NULL &&
        (!cJSON_IsString(v) || action_of(ld, v->valuestring, &cmd->action) != 0)) {
        return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"action\" must be a string");
    }
    if ((v = cvl_member(c, "address")) != NULL) {
        cmd->has_address = 1;
        if (cvl_value_of(ld, v, "address", &cmd->address) != 0 ||
            capture_ok(ld, &cmd->address, caps, "address") != 0) {
            return -1;
        }
    }
    if ((v = cvl_member(c, "space")) != NULL) {
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
    if ((v = cvl_member(c, "register")) != NULL) {
        CvField f;

        if (!cJSON_IsString(v) || cvl_field_ref(ld, v->valuestring, &f) != 0 || f.reg < 0) {
            return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"register\" names a register");
        }
        cmd->reg = f.reg;
    }
    if ((v = cvl_member(c, "count")) != NULL &&
        (cvl_value_of(ld, v, "count", &cmd->count) != 0 || capture_ok(ld, &cmd->count, caps, "count"))) {
        return -1;
    }
    if ((v = cvl_member(c, "countOffset")) != NULL) {
        if (!cJSON_IsNumber(v)) {
            return kind_error(ld->ctx, "\"countOffset\" must be a number");
        }
        cmd->count_offset = v->valueint;
    }
    if ((v = cvl_member(c, "data")) != NULL) {
        cmd->has_data = 1;
        if (cvl_value_of(ld, v, "data", &cmd->data) != 0 || capture_ok(ld, &cmd->data, caps, "data")) {
            return -1;
        }
    }
    if ((v = cvl_member(c, "channel")) != NULL) {
        cmd->has_channel = 1;
        if (cvl_value_of(ld, v, "channel", &cmd->channel) != 0 ||
            capture_ok(ld, &cmd->channel, caps, "channel")) {
            return -1;
        }
    }
    if ((v = cvl_member(c, "increment")) != NULL) {
        cmd->has_increment = 1;
        if (cvl_value_of(ld, v, "increment", &cmd->increment) != 0 ||
            capture_ok(ld, &cmd->increment, caps, "increment")) {
            return -1;
        }
    }
    if ((v = cvl_member(c, "channelMap")) != NULL) {
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
    if ((v = cvl_member(c, "set")) != NULL) {
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
            if (cvl_field_ref(ld, e->string, &sets[n].field) != 0 || sets[n].field.reg < 0 ||
                cvl_value_of(ld, e, "set", &sets[n].value) != 0 ||
                capture_ok(ld, &sets[n].value, caps, "set")) {
                return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"set\" names fields");
            }
            n++;
        }
        cmd->sets = sets;
    }
    if (cvl_member(c, "reply") != NULL) {
        cmd->has_reply = 1;
        if (cvl_pattern_member(ld, c, "reply", 0, &cmd->reply, NULL) != 0) {
            return -1;
        }
    }
    if (cvl_member(c, "next") != NULL) {
        cmd->has_next = 1;
        if (cvl_pattern_member(ld, c, "next", 0, &cmd->next, NULL) != 0) {
            return -1;
        }
    }
    if ((v = cvl_member(c, "commit")) != NULL) {
        if (!cJSON_IsString(v) || (strcmp(v->valuestring, "now") && strcmp(v->valuestring, "end"))) {
            return kind_error(ld->ctx, "\"commit\" is \"now\" or \"end\"");
        }
        cmd->commit_end = strcmp(v->valuestring, "end") == 0;
    }
    if ((v = cvl_member(c, "then")) != NULL) {
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
    if ((v = cvl_member(c, "wordBits")) != NULL) {
        uint32_t w;

        if (cvl_u32(ld, v, "\"wordBits\"", &w) != 0) {
            return -1;
        }
        cmd->word_bits = w;
    }
    if (cvl_conds(ld, cvl_member(c, "when"), &cmd->when) != 0) {
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
        return cvl_pattern(ld, v->valuestring, 0, &out->pattern, NULL);
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
            if (!cJSON_IsString(e) || cvl_pattern(ld, e->valuestring, 0, &m[n].pattern, NULL) != 0) {
                return kind_failed(ld->ctx) ? -1 : kind_error(ld->ctx, "\"read\" values are patterns");
            }
            n++;
        }
        out->map = m;
        return 0;
    }
    return kind_error(ld->ctx, "\"read\" is a pattern, \"pointer\" or an address map");
}

int cvl_load_protocol(Ld *ld, const cJSON *p, CvProtocol *out) {
    static const char *const KEYS[] = {"frame", "out", "idle", "commands", "read", "outs", "reads",
                                       "triggers", "generalCall", "align", NULL};
    const cJSON *v;
    size_t i = 0;

    memset(out, 0, sizeof *out);
    if (p == NULL) {
        return kind_error(ld->ctx, "missing \"protocol\"");
    }
    kind_push(ld->ctx, "protocol");
    if (cvl_check_keys(ld, p, KEYS) != 0) {
        return -1;
    }
    if ((v = cvl_member(p, "frame")) == NULL || !cJSON_IsString(v) ||
        (strcmp(v->valuestring, "fixed") && strcmp(v->valuestring, "stream"))) {
        return kind_error(ld->ctx, "\"frame\" is \"fixed\" or \"stream\"");
    }
    out->stream = strcmp(v->valuestring, "stream") == 0;
    if ((v = cvl_member(p, "align")) != NULL) {
        if (!cJSON_IsString(v) || (strcmp(v->valuestring, "start") && strcmp(v->valuestring, "end"))) {
            return kind_error(ld->ctx, "\"align\" is \"start\" or \"end\"");
        }
        out->align_end = strcmp(v->valuestring, "end") == 0;
    }
    if (cvl_member(p, "out") != NULL && cvl_pattern_member(ld, p, "out", 0, &out->out, NULL) != 0) {
        return -1;
    }
    if ((v = cvl_member(p, "idle")) != NULL) {
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
    if ((v = cvl_member(p, "generalCall")) != NULL) {
        if (!cJSON_IsString(v) || strcmp(v->valuestring, "reset") != 0) {
            return kind_error(ld->ctx, "\"generalCall\" is \"reset\"");
        }
        out->general_reset = 1;
    }
    if (load_read(ld, cvl_member(p, "read"), &out->read) != 0) {
        return -1;
    }
    if ((v = cvl_member(p, "commands")) != NULL) {
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
        const cJSON *arr = cvl_member(p, key);
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
            if (cvl_check_keys(ld, e, AK) != 0 || cvl_conds(ld, cvl_member(e, "when"), &alts[n].when) != 0) {
                return -1;
            }
            if (i == 0) {
                if (cvl_pattern_member(ld, e, "out", 0, &alts[n].pattern, NULL) != 0) {
                    return -1;
                }
            } else if (load_read(ld, cvl_member(e, "read"), &alts[n].read) != 0) {
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
    if ((v = cvl_member(p, "triggers")) != NULL) {
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
            if (cvl_check_keys(ld, e, TK) != 0 || kind_string(ld->ctx, e, "write", &w) != 0 ||
                kind_string(ld->ctx, e, "action", &a) != 0 || cvl_cond_text(ld, w, &t[n].cond) != 0 ||
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
    const cJSON *v = cvl_member(o, key);

    *has = 0;
    out->pin = CV_NONE;
    if (v == NULL) {
        return 0;
    }
    kind_push(ld->ctx, "%s", key);
    if (cvl_check_keys(ld, v, CK) != 0 || cvl_pin_member(ld, v, "pin", 1, &out->pin) != 0 ||
        cvl_level_member(ld, v, level_key, dflt, &out->level) != 0) {
        return -1;
    }
    kind_pop(ld->ctx);
    *has = 1;
    return 0;
}

static int pin_list(Ld *ld, const cJSON *o, const char *key, int *out, size_t *count, size_t max) {
    const cJSON *v = cvl_member(o, key);
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
        if ((out[(*count)++] = cvl_name_id(ld, e->valuestring)) == CV_NONE) {
            return -1;
        }
    }
    return 0;
}

static int load_i2c_table(Ld *ld, const cJSON *o, int *straps, unsigned *strap_count, int *table,
                          unsigned *table_count) {
    const cJSON *t = cvl_member(o, "table");
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

int cvl_load_bus(Ld *ld, const cJSON *b, CvBus *out, int config) {
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
    if (cvl_check_keys(ld, b, KEYS) != 0 || kind_string(ld->ctx, b, "style", &style) != 0) {
        return -1;
    }
    if (strcmp(style, "none") == 0) {
        if (config || !ld->display) {
            return kind_error(ld->ctx, "\"none\" is a bus style of a display driver only");
        }
        out->style = CVB_NONE;
        return 0;
    }
    if (strcmp(style, "spi") == 0) {
        out->style = CVB_SPI;
        if (cvl_pin_member(ld, b, "cs", 0, &out->cs) != 0 || cvl_pin_member(ld, b, "sclk", 1, &out->sclk) ||
            cvl_pin_member(ld, b, "mosi", 0, &out->mosi) || cvl_pin_member(ld, b, "miso", 0, &out->miso) ||
            cvl_u32_member(ld, b, "mode", &mode) != 0 || cvl_level_member(ld, b, "csActive", 0, &out->cs_level)) {
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
        return cvl_pin_member(ld, b, "scl", 1, &out->scl) || cvl_pin_member(ld, b, "sda", 1, &out->sda) ||
               load_i2c_table(ld, b, out->straps, &out->strap_count, out->table, &out->table_count);
    }
    if (strcmp(style, "i2c-spi") == 0) {
        const cJSON *s = cvl_member(b, "spi"), *i = cvl_member(b, "i2c");
        uint32_t lvl = 1;

        if (config) {
            return kind_error(ld->ctx, "\"configBus\" is \"spi\" or \"i2c\"");
        }
        out->style = CVB_I2C_SPI;
        if (s == NULL || i == NULL || cvl_pin_member(ld, b, "selectPin", 1, &out->select) ||
            cvl_u32_member(ld, b, "spiLevel", &lvl) != 0 ||
            cvl_pin_member(ld, s, "cs", 0, &out->cs) || cvl_pin_member(ld, s, "sclk", 1, &out->sclk) ||
            cvl_pin_member(ld, s, "mosi", 0, &out->mosi) || cvl_pin_member(ld, s, "miso", 0, &out->miso) ||
            cvl_u32_member(ld, s, "mode", &mode) != 0 || cvl_pin_member(ld, i, "scl", 1, &out->scl) ||
            cvl_pin_member(ld, i, "sda", 1, &out->sda) ||
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
        const cJSON *a = cvl_member(b, "ale");

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

            if (cvl_check_keys(ld, a, AK) != 0 || cvl_pin_member(ld, a, "pin", 1, &out->ale.pin) ||
                (cvl_member(a, "edge") && kind_string(ld->ctx, a, "edge", &e)) ||
                cvl_u32_member(ld, a, "bits", &bits) != 0) {
                return -1;
            }
            out->has_ale = 1;
            out->ale.fall = strcmp(e, "fall") == 0;
            out->ale.bits = bits;
        }
        return 0;
    }
    if (strcmp(style, "clocked") == 0) {
        const cJSON *c = cvl_member(b, "clock"), *co = cvl_member(b, "clockOut");
        const char *edge = "rise";

        out->style = CVB_CLOCKED;
        if (pin_list(ld, b, "data", out->data, &out->data_count, CV_MAX_DATA) != 0 ||
            ctl_member(ld, b, "oe", "active", 0, &out->oe, &out->has_oe)) {
            return -1;
        }
        if (out->data_count < 1) {
            return kind_error(ld->ctx, "\"data\" needs 1-%d pins", CV_MAX_DATA);
        }
        if (c == NULL || cvl_pin_member(ld, c, "pin", 1, &out->clock) != 0 ||
            (cvl_member(c, "edge") && kind_string(ld->ctx, c, "edge", &edge))) {
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
                const cJSON *inv = cvl_member(e, "invert");
                uint32_t div = 1;

                if (cvl_check_keys(ld, e, OK) != 0 || cvl_pin_member(ld, e, "pin", 1, &outs[n].pin) != 0 ||
                    cvl_u32_member(ld, e, "divide", &div) != 0) {
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

