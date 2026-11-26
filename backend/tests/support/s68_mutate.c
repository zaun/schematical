/* s68_mutate: loader cases as one change to a shipped device (plan-68-S). */
#include "s68_mutate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "read_file.h"

static PartLibrary *library;
static unsigned serial;
static char last_name[256];

PartLibrary *s68_library(void) {
    if (library == NULL) {
        library = part_library_create();
        if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
            printf("FAIL: cannot load the part library\n");
            exit(1);
        }
    }
    return library;
}

void s68_free(void) {
    if (library != NULL) {
        part_library_destroy(library);
        library = NULL;
    }
}

/* The member a step names in `node`: an array index or an object key. */
static cJSON *step_in(cJSON *node, const char *step) {
    char *end;
    long index = strtol(step, &end, 10);

    if (cJSON_IsArray(node) && *end == '\0' && end != step) {
        return cJSON_GetArrayItem(node, (int)index);
    }
    return cJSON_GetObjectItemCaseSensitive(node, step);
}

/* Applies one "a.b.c": value to `root`; -1 when a middle step is missing. */
static int apply(cJSON *root, const char *path, const cJSON *value) {
    char buffer[256];
    char *step = buffer;
    char *dot;
    cJSON *node = root;

    snprintf(buffer, sizeof buffer, "%s", path);
    while ((dot = strchr(step, '.')) != NULL) {
        *dot = '\0';
        node = step_in(node, step);
        if (node == NULL) {
            return -1;
        }
        step = dot + 1;
    }
    if (cJSON_IsArray(node)) {
        int index = atoi(step);

        if (cJSON_IsNull(value)) {
            cJSON_DeleteItemFromArray(node, index);
        } else if (index >= cJSON_GetArraySize(node)) {
            cJSON_AddItemToArray(node, cJSON_Duplicate(value, 1));
        } else {
            cJSON_ReplaceItemInArray(node, index, cJSON_Duplicate(value, 1));
        }
        return 0;
    }
    if (!cJSON_IsObject(node)) {
        return -1;
    }
    if (step[0] != '\0' && step[strlen(step) - 1] == '+') {
        /* "key+": a second member of that name (a duplicate key). */
        step[strlen(step) - 1] = '\0';
        cJSON_AddItemToObject(node, step, cJSON_Duplicate(value, 1));
        return 0;
    }
    cJSON_DeleteItemFromObjectCaseSensitive(node, step);
    if (!cJSON_IsNull(value)) {
        cJSON_AddItemToObject(node, step, cJSON_Duplicate(value, 1));
    }
    return 0;
}

/* A patch with ' for " (C strings stay readable). */
static cJSON *parse_patch(const char *patch) {
    char *text = strdup(patch != NULL ? patch : "{}");
    char *c;
    cJSON *json;

    for (c = text; *c != '\0'; c++) {
        if (*c == '\'') {
            *c = '"';
        }
    }
    json = cJSON_Parse(text);
    free(text);
    return json;
}

int s68_load(const char *kind, const char *device, const char *patch, const char **name,
             char **error) {
    char path[1024];
    char *text;
    cJSON *file, *devices, *found = NULL, *copy, *out, *list, *changes, *item;
    char *json;
    int result;

    if (error != NULL) {
        *error = NULL;
    }
    s68_library();
    snprintf(path, sizeof path, "%s/%s.json", PARTS_DIR, kind);
    text = read_file(path);
    file = text != NULL ? cJSON_Parse(text) : NULL;
    free(text);
    devices = cJSON_GetObjectItemCaseSensitive(file, "devices");
    cJSON_ArrayForEach(item, devices) {
        const cJSON *id = cJSON_GetObjectItemCaseSensitive(item, "id");

        if (cJSON_IsString(id) && strcmp(id->valuestring, device) == 0) {
            found = item;
        }
    }
    if (found == NULL) {
        printf("FAIL: s68: no device %s in %s.json\n", device, kind);
        exit(1);
    }
    copy = cJSON_Duplicate(found, 1);
    cJSON_Delete(file);
    serial++;
    snprintf(path, sizeof path, "s68-%u", serial);
    cJSON_ReplaceItemInObjectCaseSensitive(copy, "id", cJSON_CreateString(path));
    changes = parse_patch(patch);
    if (changes == NULL) {
        printf("FAIL: s68: bad patch %s\n", patch);
        exit(1);
    }
    cJSON_ArrayForEach(item, changes) {
        if (apply(copy, item->string, item) != 0) {
            printf("FAIL: s68: patch path %s not in %s/%s\n", item->string, kind, device);
            exit(1);
        }
    }
    cJSON_Delete(changes);
    cJSON_ArrayForEach(item, cJSON_GetObjectItemCaseSensitive(copy, "parts")) {
        const cJSON *part = cJSON_GetObjectItemCaseSensitive(item, "name");

        if (cJSON_IsString(part)) {
            snprintf(path, sizeof path, "%s-S68-%u", part->valuestring, serial);
            cJSON_ReplaceItemInObjectCaseSensitive(item, "name", cJSON_CreateString(path));
        }
    }
    item = cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(copy, "parts"), 0);
    item = cJSON_GetObjectItemCaseSensitive(item, "name");
    snprintf(last_name, sizeof last_name, "%s", cJSON_IsString(item) ? item->valuestring : "");

    out = cJSON_CreateObject();
    cJSON_AddStringToObject(out, "format", "schematical-parts");
    cJSON_AddNumberToObject(out, "version", 1);
    cJSON_AddStringToObject(out, "kind", kind);
    list = cJSON_AddArrayToObject(out, "devices");
    cJSON_AddItemToArray(list, copy);
    json = cJSON_PrintUnformatted(out);
    cJSON_Delete(out);
    result = load_parts(library, json, "s68.json", error);
    cJSON_free(json);
    if (name != NULL) {
        *name = last_name;
    }
    return result;
}

int s68_rejects(const char *kind, const char *device, const char *patch, const char *want) {
    char *error = NULL;
    size_t before = part_library_count(s68_library());
    int status = s68_load(kind, device, patch, NULL, &error);
    int ok = status != 0 && error != NULL && strstr(error, want) != NULL &&
             part_library_count(library) == before;

    if (!ok) {
        printf("FAIL: %s %s %s\n  want: %s\n  got:  %s\n", kind, device, patch, want,
               status == 0 ? "(loaded)" : error != NULL ? error : "(null)");
    }
    free(error);
    return ok;
}

const Part *s68_loads(const char *kind, const char *device, const char *patch) {
    char *error = NULL;
    const char *name = NULL;

    if (s68_load(kind, device, patch, &name, &error) != 0) {
        printf("FAIL: %s %s %s did not load: %s\n", kind, device, patch,
               error != NULL ? error : "(null)");
        free(error);
        return NULL;
    }
    return part_library_find(library, name);
}
