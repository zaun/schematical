/* serial_bench_cov: edited variants of the shipped bridge devices (plan-68-O). */
#include "serial_bench_cov.h"

#include "cJSON.h"
#include "serial_bench_maxim.h"

static int cov_counter;
static char cov_name[96];

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

int cov_variant(const char *base_id, const char *edits, const char **name_out, char **error) {
    char *text = read_fixture("bridges.json");
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
    int n = cov_counter++;

    free(text);
    *error = NULL;
    if (dev == NULL || ed == NULL) {
        fprintf(stderr, "cov_variant: no device %s or bad edits %s\n", base_id, edits);
        exit(2);
    }
    dev = cJSON_Duplicate(dev, 1);
    cJSON_ArrayForEach(item, ed) {
        if (apply_edit(dev, item->string, item) != 0) {
            fprintf(stderr, "cov_variant: cannot apply %s\n", item->string);
            exit(2);
        }
    }
    snprintf(id, sizeof id, "%s-COV%d", base_id, n);
    cJSON_ReplaceItemInObjectCaseSensitive(dev, "id", cJSON_CreateString(id));
    cov_name[0] = '\0';
    cJSON_ArrayForEach(part, cJSON_GetObjectItemCaseSensitive(dev, "parts")) {
        const cJSON *pn = cJSON_GetObjectItemCaseSensitive(part, "name");
        char name[96];

        snprintf(name, sizeof name, "%s-COV%d", pn->valuestring, n);
        if (cov_name[0] == '\0') {
            snprintf(cov_name, sizeof cov_name, "%s", name);
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
    status = load_parts(library, json, "cov.json", error);
    cJSON_free(json);
    cJSON_Delete(out);
    cJSON_Delete(ed);
    cJSON_Delete(file);
    if (name_out != NULL) {
        *name_out = cov_name;
    }
    return status;
}

void cov_reject(const char *base_id, const char *edits, const char *want) {
    char *error = NULL;
    char what[600];
    int status = cov_variant(base_id, edits, NULL, &error);

    snprintf(what, sizeof what, "%s %s: refused with \"%s\" (got: %s)", base_id, edits, want,
             status == 0 ? "loaded" : (error != NULL ? error : "(null)"));
    check(status != 0 && error != NULL && strstr(error, want) != NULL, what);
    free(error);
}

const char *cov_part(const char *base_id, const char *edits) {
    char *error = NULL;
    const char *name = NULL;

    if (cov_variant(base_id, edits, &name, &error) != 0) {
        fprintf(stderr, "cov_part %s %s: %s\n", base_id, edits, error != NULL ? error : "?");
        exit(2);
    }
    free(error);
    return name;
}

/* ---- X-capable host helpers ----------------------------------------------------------- */

LogicValue lv(char c) {
    return c == '1' ? LOGIC_1 : c == 'X' ? LOGIC_X : LOGIC_0;
}

/* One SPI frame (mode 0) of the bits in `bits` ('0', '1', 'X'; spaces skipped);
   `miso` (may be NULL) gets what SO showed for each bit. */
void spx(MB *b, const char *bits, char *miso) {
    size_t n = 0;

    mb_set(b, b->cs, LOGIC_0);
    mb_run(b, SPI_HALF);
    for (; *bits != '\0'; bits++) {
        if (*bits == ' ') {
            continue;
        }
        mb_set(b, b->mosi, lv(*bits));
        mb_run(b, SPI_HALF);
        if (miso != NULL) {
            miso[n] = "01XZ"[mb_get(b, b->miso)];
        }
        n++;
        mb_set(b, b->sclk, LOGIC_1);
        mb_run(b, SPI_HALF);
        mb_set(b, b->sclk, LOGIC_0);
    }
    if (miso != NULL) {
        miso[n] = '\0';
    }
    mb_run(b, SPI_HALF);
    mb_set(b, b->cs, LOGIC_1);
    mb_run(b, SPI_HALF);
}

/* A register write over SPI with a value that may have X bits. */
void wx(Br *r, unsigned ch, unsigned reg, const char *value) {
    char bits[24];
    unsigned sub = br_sub(reg, ch);
    int i;

    for (i = 0; i < 8; i++) {
        bits[i] = (sub >> (7 - i)) & 1u ? '1' : '0';
    }
    snprintf(bits + 8, sizeof bits - 8, "%s", value);
    spx(r->b, bits, NULL);
}

/* An I2C byte from 8 bit characters; the ACK slot as the target left it. */
LogicValue i2c_bits(MB *b, const char *bits) {
    int i;

    for (i = 0; i < 8; i++) {
        mb_set(b, b->sda, bits[i] == '1' ? LOGIC_Z : lv(bits[i]));
        mb_run(b, I2C_Q);
        mb_set(b, b->scl, LOGIC_Z);
        mb_run(b, I2C_Q * 2);
        mb_set(b, b->scl, LOGIC_0);
        mb_run(b, I2C_Q);
    }
    return i2c_bit(b, 1);
}
