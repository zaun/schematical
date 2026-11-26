/*
 * kind_reject_cov_test (plan-68-R): loader rejections of the memory,
 * passive, power, timer and display kinds, as data.
 *
 * Each case in tests/fixtures/kind_reject/<kind>.json takes one real device
 * from a bundled part file, keeps only its first part, applies a patch and
 * loads the result as a one-device part file into a library that holds only
 * the bundled datasheets and footprints. The load must fail, the message
 * must contain the case's "want" text (the rule docs/parts-format.md states
 * for that key), and the library must be unchanged. A case:
 *
 *   {"label": "...", "file": "capacitor.json", "device": "<id>",
 *    "set": {"<path>": <value>, ...}, "want": "<message text>"}
 *
 * A path is dot-separated from the device object ("farads",
 * "parts.0.volts", "pinouts.polar.1.name"); a number segment indexes an
 * array; the value null deletes the member (or array element). "keep": n
 * keeps the first n parts instead of one. A kind with no bundled device
 * (generic) gives the device itself: {"kind": "generic", "inline": {...}}.
 * A case with "text" loads that file text as it is (for what a patch cannot
 * write, such as a duplicate key).
 * A case with "loads": true must load instead, and "want" is then text of
 * the loaded first part's listing line (tests/support/listing.h); such a
 * case renames its device ("id") and part ("parts.0.name") so they do not
 * clash.
 */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "listing.h"
#include "parts.h"
#include "read_file.h"

#define CASE_DIR GOLDEN_DIR "/../fixtures/kind_reject"

static int failures;
static int cases;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

/* One parsed bundled part file per name, read once. */
#define MAX_FILES 64
static struct {
    char name[64];
    cJSON *json;
} files[MAX_FILES];
static size_t file_count;

static const cJSON *part_file(const char *name) {
    char path[4096];
    char *text;
    size_t i;

    for (i = 0; i < file_count; i++) {
        if (strcmp(files[i].name, name) == 0) {
            return files[i].json;
        }
    }
    if (file_count == MAX_FILES || strlen(name) >= sizeof files[0].name) {
        return NULL;
    }
    snprintf(path, sizeof path, "%s/%s", PARTS_DIR, name);
    text = read_file(path);
    if (text == NULL) {
        return NULL;
    }
    files[file_count].json = cJSON_Parse(text);
    free(text);
    if (files[file_count].json == NULL) {
        return NULL;
    }
    snprintf(files[file_count].name, sizeof files[0].name, "%s", name);
    return files[file_count++].json;
}

static int is_index(const char *segment) {
    return segment[0] >= '0' && segment[0] <= '9';
}

/* Sets (value != NULL, taken over) or deletes (value == NULL) the member at
   `path` under `root`. Returns 0, or -1 when a parent is missing. */
static int patch(cJSON *root, const char *path, cJSON *value) {
    char buffer[256];
    char *segment, *next;
    cJSON *node = root;

    if (strlen(path) >= sizeof buffer) {
        cJSON_Delete(value);
        return -1;
    }
    snprintf(buffer, sizeof buffer, "%s", path);
    segment = buffer;
    for (;;) {
        next = strchr(segment, '.');
        if (next == NULL) {
            break;
        }
        *next = '\0';
        node = cJSON_IsArray(node) && is_index(segment) ? cJSON_GetArrayItem(node, atoi(segment))
                                                        : cJSON_GetObjectItemCaseSensitive(node, segment);
        if (node == NULL) {
            cJSON_Delete(value);
            return -1;
        }
        segment = next + 1;
    }
    if (cJSON_IsArray(node) && is_index(segment)) {
        int at = atoi(segment);

        if (value == NULL) {
            cJSON_DeleteItemFromArray(node, at);
        } else if (at >= cJSON_GetArraySize(node)) {
            cJSON_AddItemToArray(node, value);
        } else {
            cJSON_ReplaceItemInArray(node, at, value);
        }
        return 0;
    }
    if (!cJSON_IsObject(node)) {
        cJSON_Delete(value);
        return -1;
    }
    cJSON_DeleteItemFromObjectCaseSensitive(node, segment);
    if (value != NULL) {
        cJSON_AddItemToObject(node, segment, value);
    }
    return 0;
}

/* Builds the one-device file text for a case, or NULL (and says why). */
static char *case_text(const cJSON *c, const char *label) {
    const cJSON *inline_device = cJSON_GetObjectItem(c, "inline");
    const cJSON *file = inline_device != NULL ? c : part_file(cJSON_GetStringValue(cJSON_GetObjectItem(c, "file")));
    const char *id = cJSON_GetStringValue(cJSON_GetObjectItem(c, "device"));
    const cJSON *keep = cJSON_GetObjectItem(c, "keep");
    const cJSON *set = cJSON_GetObjectItem(c, "set");
    const cJSON *device = inline_device, *list, *d, *item;
    cJSON *copy, *out, *parts, *devices;
    char *text;
    int k;

    if (inline_device != NULL) {
        id = "";
    } else if (file == NULL || id == NULL) {
        printf("FAIL: %s: no such part file or no device id\n", label);
        return NULL;
    }
    list = inline_device != NULL ? NULL : cJSON_GetObjectItem(file, "devices");
    cJSON_ArrayForEach(d, list) {
        const char *did = cJSON_GetStringValue(cJSON_GetObjectItem(d, "id"));

        if (did != NULL && strcmp(did, id) == 0) {
            device = d;
        }
    }
    if (device == NULL) {
        printf("FAIL: %s: no device \"%s\"\n", label, id);
        return NULL;
    }
    copy = cJSON_Duplicate(device, 1);
    parts = cJSON_GetObjectItem(copy, "parts");
    k = cJSON_IsNumber(keep) ? keep->valueint : 1;
    while (cJSON_IsArray(parts) && cJSON_GetArraySize(parts) > k) {
        cJSON_DeleteItemFromArray(parts, cJSON_GetArraySize(parts) - 1);
    }
    cJSON_ArrayForEach(item, set) {
        cJSON *value = cJSON_IsNull(item) ? NULL : cJSON_Duplicate(item, 1);

        if (patch(copy, item->string, value) != 0) {
            printf("FAIL: %s: cannot patch \"%s\"\n", label, item->string);
            cJSON_Delete(copy);
            return NULL;
        }
    }
    out = cJSON_CreateObject();
    cJSON_AddStringToObject(out, "format", "schematical-parts");
    cJSON_AddNumberToObject(out, "version", 1);
    cJSON_AddItemToObject(out, "kind", cJSON_Duplicate(cJSON_GetObjectItem(file, "kind"), 1));
    devices = cJSON_AddArrayToObject(out, "devices");
    cJSON_AddItemToArray(devices, copy);
    text = cJSON_PrintUnformatted(out);
    cJSON_Delete(out);
    return text;
}

static void run_case(PartLibrary *library, const cJSON *c, const char *source) {
    const char *label = cJSON_GetStringValue(cJSON_GetObjectItem(c, "label"));
    const char *want = cJSON_GetStringValue(cJSON_GetObjectItem(c, "want"));
    size_t before = part_library_count(library);
    char what[1024];
    char *error = NULL;
    char *text;
    int status;

    if (label == NULL || want == NULL) {
        printf("FAIL: %s: a case without \"label\" or \"want\"\n", source);
        failures++;
        return;
    }
    cases++;
    if (cJSON_IsString(cJSON_GetObjectItem(c, "text"))) {
        /* Raw file text, for what a patch cannot write (a duplicate key). */
        const char *raw = cJSON_GetObjectItem(c, "text")->valuestring;

        text = cJSON_malloc(strlen(raw) + 1);
        if (text != NULL) {
            memcpy(text, raw, strlen(raw) + 1);
        }
    } else {
        text = case_text(c, label);
    }
    if (text == NULL) {
        failures++;
        return;
    }
    status = load_parts(library, text, "case.json", &error);
    if (cJSON_IsTrue(cJSON_GetObjectItem(c, "loads"))) {
        cJSON *json = cJSON_Parse(text);
        const char *name = cJSON_GetStringValue(cJSON_GetObjectItem(
            cJSON_GetArrayItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(json, "devices"), 0),
                                                   "parts"), 0), "name"));
        const Part *part = status == 0 && name != NULL ? part_library_find(library, name) : NULL;
        char *line = part != NULL ? listing_line(part) : NULL;

        snprintf(what, sizeof what, "%s: %s: loads, listing has \"%s\" (got: %s)", source, label, want,
                 line != NULL ? line : (error != NULL ? error : "(no part)"));
        expect(line != NULL && strstr(line, want) != NULL, what);
        free(line);
        cJSON_Delete(json);
        free(error);
        cJSON_free(text);
        return;
    }
    snprintf(what, sizeof what, "%s: %s: refused with \"%s\" (got: %s)", source, label, want,
             status == 0 ? "loaded" : (error != NULL ? error : "(null)"));
    expect(status != 0 && error != NULL && strstr(error, want) != NULL &&
               part_library_count(library) == before,
           what);
    free(error);
    cJSON_free(text);
}

static int by_name(const void *a, const void *b) {
    return strcmp(*(char *const *)a, *(char *const *)b);
}

int main(void) {
    PartLibrary *library = part_library_create();
    char *names[64];
    size_t name_count = 0, i;
    DIR *dir;
    struct dirent *entry;

    if (library == NULL || load_datasheets_file(library, PARTS_DIR) != 0) {
        printf("FAIL: cannot load datasheets\n");
        return 1;
    }
    {
        char *text = read_file(PARTS_DIR "/footprints.json");
        char *error = NULL;

        if (text == NULL || load_footprints(library, text, "footprints.json", &error) != 0) {
            printf("FAIL: cannot load footprints: %s\n", error != NULL ? error : "(none)");
            return 1;
        }
        free(text);
    }
    dir = opendir(CASE_DIR);
    if (dir == NULL) {
        printf("FAIL: cannot open %s\n", CASE_DIR);
        return 1;
    }
    while ((entry = readdir(dir)) != NULL && name_count < 64) {
        size_t n = strlen(entry->d_name);

        if (n > 5 && strcmp(entry->d_name + n - 5, ".json") == 0) {
            names[name_count++] = strdup(entry->d_name);
        }
    }
    closedir(dir);
    qsort(names, name_count, sizeof names[0], by_name);
    for (i = 0; i < name_count; i++) {
        char path[4096];
        char *text;
        cJSON *list, *c;

        snprintf(path, sizeof path, "%s/%s", CASE_DIR, names[i]);
        text = read_file(path);
        list = text != NULL ? cJSON_Parse(text) : NULL;
        free(text);
        if (!cJSON_IsArray(list)) {
            printf("FAIL: %s is not a JSON array of cases\n", names[i]);
            failures++;
        }
        cJSON_ArrayForEach(c, list) {
            run_case(library, c, names[i]);
        }
        cJSON_Delete(list);
        free(names[i]);
    }
    for (i = 0; i < file_count; i++) {
        cJSON_Delete(files[i].json);
    }
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d of %d cases\n", failures, cases);
        return 1;
    }
    printf("kind_reject_cov_test: all %d cases passed\n", cases);
    return 0;
}
