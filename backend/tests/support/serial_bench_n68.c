/* serial_bench_n68: edited variants of the ns16550 and bridge fixture devices (plan-68-N). */
#include "serial_bench_n68.h"

#include "cJSON.h"

static int n68_counter;
static char n68_name[96];

static cJSON *find_device(const cJSON *file, const char *id) {
    const cJSON *dev;

    cJSON_ArrayForEach(dev, cJSON_GetObjectItemCaseSensitive(file, "devices")) {
        const cJSON *i = cJSON_GetObjectItemCaseSensitive(dev, "id");

        if (cJSON_IsString(i) && strcmp(i->valuestring, id) == 0) {
            return (cJSON *)dev;
        }
    }
    return NULL;
}

/* Sets (or deletes, for the string "-") the item at a dotted path. */
static int apply_edit(cJSON *root, const char *path, const cJSON *value) {
    char buf[160];
    char *key;
    char *dot;
    cJSON *at = root;
    int del = cJSON_IsString(value) && strcmp(value->valuestring, "-") == 0;

    snprintf(buf, sizeof buf, "%s", path);
    key = buf;
    while ((dot = strchr(key, '.')) != NULL) {
        *dot = '\0';
        at = cJSON_IsArray(at) ? cJSON_GetArrayItem(at, atoi(key))
                               : cJSON_GetObjectItemCaseSensitive(at, key);
        if (at == NULL) {
            return -1;
        }
        key = dot + 1;
    }
    if (cJSON_IsArray(at)) {
        int i = atoi(key);

        if (del) {
            cJSON_DeleteItemFromArray(at, i);
            return 0;
        }
        if (i == cJSON_GetArraySize(at)) {
            cJSON_AddItemToArray(at, cJSON_Duplicate(value, 1));
            return 0;
        }
        return cJSON_ReplaceItemInArray(at, i, cJSON_Duplicate(value, 1)) ? 0 : -1;
    }
    if (del) {
        cJSON_DeleteItemFromObjectCaseSensitive(at, key);
        return 0;
    }
    if (cJSON_GetObjectItemCaseSensitive(at, key) != NULL) {
        return cJSON_ReplaceItemInObjectCaseSensitive(at, key, cJSON_Duplicate(value, 1)) ? 0 : -1;
    }
    cJSON_AddItemToObject(at, key, cJSON_Duplicate(value, 1));
    return 0;
}

int n68_variant(const char *base_id, const char *edits, const char **name_out, char **error) {
    char *text = read_fixture("ns16550.json");
    cJSON *file = text != NULL ? cJSON_Parse(text) : NULL;
    cJSON *ed = cJSON_Parse(edits);
    cJSON *dev = file != NULL ? find_device(file, base_id) : NULL;
    cJSON *out;
    cJSON *devices;
    cJSON *item;
    cJSON *part;
    char id[96];
    char *json;
    int status;
    int n = n68_counter++;

    free(text);
    if (dev == NULL) {   /* the bridges (bridges.json) */
        cJSON_Delete(file);
        text = read_fixture("bridges.json");
        file = text != NULL ? cJSON_Parse(text) : NULL;
        dev = file != NULL ? find_device(file, base_id) : NULL;
        free(text);
    }
    *error = NULL;
    if (dev == NULL || ed == NULL) {
        fprintf(stderr, "n68_variant: no device %s or bad edits %s\n", base_id, edits);
        exit(2);
    }
    dev = cJSON_Duplicate(dev, 1);
    cJSON_ArrayForEach(item, ed) {
        if (apply_edit(dev, item->string, item) != 0) {
            fprintf(stderr, "n68_variant: cannot apply %s\n", item->string);
            exit(2);
        }
    }
    snprintf(id, sizeof id, "%s-N68-%d", base_id, n);
    cJSON_ReplaceItemInObjectCaseSensitive(dev, "id", cJSON_CreateString(id));
    n68_name[0] = '\0';
    cJSON_ArrayForEach(part, cJSON_GetObjectItemCaseSensitive(dev, "parts")) {
        const cJSON *pn = cJSON_GetObjectItemCaseSensitive(part, "name");
        char name[96];

        snprintf(name, sizeof name, "%s-N68-%d", pn->valuestring, n);
        if (n68_name[0] == '\0') {
            snprintf(n68_name, sizeof n68_name, "%s", name);
        }
        cJSON_ReplaceItemInObjectCaseSensitive(part, "name", cJSON_CreateString(name));
    }
    out = cJSON_CreateObject();
    cJSON_AddStringToObject(out, "format", "schematical-parts");
    cJSON_AddNumberToObject(out, "version", 1);
    cJSON_AddStringToObject(out, "kind", "serial");
    devices = cJSON_AddArrayToObject(out, "devices");
    cJSON_AddItemToArray(devices, dev);
    json = cJSON_PrintUnformatted(out);
    status = load_parts(library, json, "n68.json", error);
    cJSON_free(json);
    cJSON_Delete(out);
    cJSON_Delete(ed);
    cJSON_Delete(file);
    if (name_out != NULL) {
        *name_out = n68_name;
    }
    return status;
}

void n68_reject(const char *base_id, const char *edits, const char *want) {
    char *error = NULL;
    char what[700];
    int status = n68_variant(base_id, edits, NULL, &error);

    snprintf(what, sizeof what, "%s %s: refused with \"%s\" (got: %s)", base_id, edits, want,
             status == 0 ? "loaded" : (error != NULL ? error : "(null)"));
    check(status != 0 && error != NULL && strstr(error, want) != NULL, what);
    free(error);
}

const char *n68_part(const char *base_id, const char *edits) {
    char *error = NULL;
    const char *name = NULL;

    if (n68_variant(base_id, edits, &name, &error) != 0) {
        fprintf(stderr, "n68_part %s %s: %s\n", base_id, edits, error != NULL ? error : "?");
        exit(2);
    }
    free(error);
    return name;
}

size_t n68_sent(Bench *b, unsigned ch, uint16_t *frames, size_t cap) {
    const SimComponentType *type = sim_instance_type(b->chip);

    return type->serial_sent(sim_instance_state(b->chip), ch, 0, frames, cap);
}

size_t n68_sent_count(Bench *b, unsigned ch) {
    uint16_t frames[64];

    return n68_sent(b, ch, frames, 64);
}

int n68_sent_has(Bench *b, unsigned ch, size_t from, unsigned want) {
    uint16_t frames[64];
    size_t n = n68_sent(b, ch, frames, 64);
    size_t i;

    for (i = from; i < n; i++) {
        if (frames[i] == want) {
            return 1;
        }
    }
    return 0;
}

void n68_write_x(Bench *b, unsigned a, unsigned v, unsigned xmask) {
    unsigned i;

    set_address(b, a);
    for (i = 0; i < 8; i++) {
        set(b, b->cfg->data[i], (xmask >> i) & 1u ? LOGIC_X : (v >> i) & 1u ? LOGIC_1 : LOGIC_0);
    }
    run(b, NS(40));
    set(b, b->cfg->write, LOGIC_0);
    run(b, NS(120));
    set(b, b->cfg->write, LOGIC_1);
    run(b, NS(b->cfg->recovery_ns > 40 ? b->cfg->recovery_ns : 40));
    set_data(b, 0, 0);
    run(b, NS(160));
}

int n68_toggles(Bench *b, const char *pin, unsigned n, SimTime step) {
    int saw0 = 0;
    int saw1 = 0;
    unsigned i;

    for (i = 0; i < n; i++) {
        run(b, step);
        saw0 |= get(b, pin) == LOGIC_0;
        saw1 |= get(b, pin) == LOGIC_1;
    }
    return saw0 && saw1;
}

SimTime n68_low_len(Bench *b, const char *pin, SimTime limit) {
    SimTime spent = 0;
    SimTime t0;

    while (get(b, pin) != LOGIC_0) {
        if (spent >= limit) {
            return 0;
        }
        run(b, NS(10));
        spent += NS(10);
    }
    t0 = sim_now(b->sim);
    while (get(b, pin) == LOGIC_0 && sim_now(b->sim) - t0 < limit) {
        run(b, NS(10));
    }
    return sim_now(b->sim) - t0;
}
