#ifndef SERIAL_COV_CLASSIC_H
#define SERIAL_COV_CLASSIC_H

/*
 * plan-68-P: variants of the classic-UART fixture devices
 * (support/serial_fixture_classic.h) for loader rejections and for benches
 * on an edited device. Header-only (static inline) so each program that
 * includes it needs no extra bench source.
 *
 * `edits` is a JSON object: each key is a path into the device object
 * ("bus.select", "parts.0.ctsA", "pinouts.so28.3.name"; dots between keys
 * and array indexes) and its value replaces what is there (or adds the
 * key); the string "-" deletes it. The variant gets a fresh id and part
 * names ("<id>-V<n>", "<part>-V<n>").
 */

#include "cJSON.h"
#include "serial_bench.h"

static int scv_counter;
static char scv_name[96];

static inline cJSON *scv_find_device(const cJSON *file, const char *id) {
    const cJSON *dev;

    cJSON_ArrayForEach(dev, cJSON_GetObjectItemCaseSensitive(file, "devices")) {
        const cJSON *i = cJSON_GetObjectItemCaseSensitive(dev, "id");

        if (cJSON_IsString(i) && strcmp(i->valuestring, id) == 0) {
            return (cJSON *)dev;
        }
    }
    return NULL;
}

static inline int scv_apply_edit(cJSON *root, const char *path, const cJSON *value) {
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

/* Loads a variant of fixture device `base_id`; 0, or -1 with the loader's
   message in *error (free it). *name_out: the first part's new name. */
static inline int scv_variant(const char *base_id, const char *edits, const char **name_out,
                              char **error) {
    cJSON *file = cJSON_Parse(SERIAL_FIXTURE_CLASSIC);
    cJSON *ed = cJSON_Parse(edits);
    cJSON *dev = file != NULL ? scv_find_device(file, base_id) : NULL;
    cJSON *out;
    cJSON *devices;
    cJSON *item;
    cJSON *part;
    char id[96];
    char *json;
    int status;
    int n = scv_counter++;

    *error = NULL;
    if (dev == NULL || ed == NULL) {
        fprintf(stderr, "scv_variant: no device %s or bad edits %s\n", base_id, edits);
        exit(2);
    }
    dev = cJSON_Duplicate(dev, 1);
    cJSON_ArrayForEach(item, ed) {
        if (scv_apply_edit(dev, item->string, item) != 0) {
            fprintf(stderr, "scv_variant: cannot apply %s\n", item->string);
            exit(2);
        }
    }
    snprintf(id, sizeof id, "%s-V%d", base_id, n);
    cJSON_ReplaceItemInObjectCaseSensitive(dev, "id", cJSON_CreateString(id));
    scv_name[0] = '\0';
    cJSON_ArrayForEach(part, cJSON_GetObjectItemCaseSensitive(dev, "parts")) {
        const cJSON *pn = cJSON_GetObjectItemCaseSensitive(part, "name");
        char name[96];

        snprintf(name, sizeof name, "%s-V%d", pn->valuestring, n);
        if (scv_name[0] == '\0') {
            snprintf(scv_name, sizeof scv_name, "%s", name);
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
    status = load_parts(library, json, "scv.json", error);
    cJSON_free(json);
    cJSON_Delete(out);
    cJSON_Delete(ed);
    cJSON_Delete(file);
    if (name_out != NULL) {
        *name_out = scv_name;
    }
    return status;
}

/* Expects the variant to be refused with a message containing `want`. */
static inline void scv_reject(const char *base_id, const char *edits, const char *want) {
    char *error = NULL;
    char what[700];
    int status = scv_variant(base_id, edits, NULL, &error);

    snprintf(what, sizeof what, "%s %s: refused with \"%s\" (got: %s)", base_id, edits, want,
             status == 0 ? "loaded" : (error != NULL ? error : "(null)"));
    check(status != 0 && error != NULL && strstr(error, want) != NULL, what);
    free(error);
}

/* Loads the variant (it must load); returns its first part's name. */
static inline const char *scv_part(const char *base_id, const char *edits) {
    char *error = NULL;
    const char *name = NULL;

    if (scv_variant(base_id, edits, &name, &error) != 0) {
        fprintf(stderr, "scv_part %s %s: %s\n", base_id, edits, error != NULL ? error : "?");
        exit(2);
    }
    free(error);
    return name;
}

#endif
