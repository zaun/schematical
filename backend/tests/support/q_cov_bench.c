/* q_cov_bench: edited-device loading for the plan-68-Q coverage programs. */
#include "q_cov_bench.h"

#include "read_file.h"

int failures;

void qc_expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

static char *dequote(const char *text) {
    size_t n = strlen(text);
    char *copy = malloc(n + 1);
    size_t i;

    for (i = 0; i <= n; i++) {
        copy[i] = text[i] == '\'' ? '"' : text[i];
    }
    return copy;
}

static void die(const char *what, const char *detail) {
    fprintf(stderr, "q_cov_bench: %s %s\n", what, detail);
    exit(2);
}

/* The file's path, and the footprints file next to it. */
static void paths(const char *file, char *path, char *footprints, size_t size) {
    const char *slash;

    if (strncmp(file, "fx:", 3) == 0) {
        snprintf(path, size, "%s/../fixtures/%s", GOLDEN_DIR, file + 3);
    } else {
        snprintf(path, size, "%s/%s", PARTS_DIR, file);
    }
    snprintf(footprints, size, "%s", path);
    slash = strrchr(footprints, '/');
    snprintf(footprints + (slash - footprints) + 1, size - (size_t)(slash - footprints) - 1,
             "footprints.json");
}

static void apply_edit(cJSON *root, const char *path, const char *value) {
    char buffer[256];
    char *segment;
    char *next;
    cJSON *node = root;

    snprintf(buffer, sizeof buffer, "%s", path);
    segment = buffer;
    for (;;) {
        int is_array = cJSON_IsArray(node);
        int index = is_array ? atoi(segment) : -1;

        next = strchr(segment, '.');
        if (next != NULL) {
            *next = '\0';
            node = is_array ? cJSON_GetArrayItem(node, index)
                            : cJSON_GetObjectItemCaseSensitive(node, segment);
            if (node == NULL) {
                die("no path", path);
            }
            segment = next + 1;
            continue;
        }
        if (value == NULL) {
            if (is_array) {
                cJSON_DeleteItemFromArray(node, index);
            } else {
                cJSON_DeleteItemFromObjectCaseSensitive(node, segment);
            }
        } else {
            char *text = dequote(value);
            cJSON *item = cJSON_Parse(text);

            free(text);
            if (item == NULL) {
                die("bad value", value);
            }
            if (is_array && index >= cJSON_GetArraySize(node)) {
                cJSON_AddItemToArray(node, item);
            } else if (is_array) {
                cJSON_ReplaceItemInArray(node, index, item);
            } else if (cJSON_GetObjectItemCaseSensitive(node, segment) != NULL) {
                cJSON_ReplaceItemInObjectCaseSensitive(node, segment, item);
            } else {
                cJSON_AddItemToObject(node, segment, item);
            }
        }
        return;
    }
}

/* The parts file text holding only the edited device; *first gets its first part's name. */
static char *edited_file(const char *path, const char *id, const char *const *edits,
                         char *first, size_t first_size) {
    char *text = read_file(path);
    cJSON *root;
    cJSON *devices;
    cJSON *device;
    cJSON *keep = NULL;
    char *out;
    size_t i;

    if (text == NULL) {
        die("cannot read", path);
    }
    root = cJSON_Parse(text);
    free(text);
    devices = cJSON_GetObjectItemCaseSensitive(root, "devices");
    cJSON_ArrayForEach(device, devices) {
        const cJSON *dev_id = cJSON_GetObjectItemCaseSensitive(device, "id");

        if (cJSON_IsString(dev_id) && strcmp(dev_id->valuestring, id) == 0) {
            keep = cJSON_Duplicate(device, 1);
            break;
        }
    }
    if (keep == NULL) {
        die("no device", id);
    }
    {
        const cJSON *parts = cJSON_GetObjectItemCaseSensitive(keep, "parts");
        const cJSON *name = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(parts, 0), "name");

        snprintf(first, first_size, "%s", cJSON_IsString(name) ? name->valuestring : "");
    }
    for (i = 0; edits != NULL && edits[i] != NULL; i += 2) {
        apply_edit(keep, edits[i], edits[i + 1]);
    }
    cJSON_DeleteItemFromObjectCaseSensitive(root, "devices");
    devices = cJSON_AddArrayToObject(root, "devices");
    cJSON_AddItemToArray(devices, keep);
    out = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return out;
}

static PartLibrary *library_for(const char *file, const char *footprints) {
    PartLibrary *library = part_library_create();
    char *text = read_file(footprints);
    char *error = NULL;

    if (text == NULL || load_footprints(library, text, footprints, &error) != 0) {
        die("footprints", error != NULL ? error : footprints);
    }
    free(text);
    free(error);
    if (strncmp(file, "fx:", 3) != 0 && load_datasheets_file(library, PARTS_DIR) != 0) {
        die("datasheets", PARTS_DIR);
    }
    return library;
}

void qc_fail(const char *file, const char *id, const char *const *edits, const char *expect) {
    char path[1024];
    char footprints[1024];
    char first[128];
    PartLibrary *library;
    char *text;
    char *error = NULL;
    int result;

    paths(file, path, footprints, sizeof path);
    library = library_for(file, footprints);
    text = edited_file(path, id, edits, first, sizeof first);
    result = load_parts(library, text, "edited.json", &error);
    if (result != -1 || error == NULL || strstr(error, expect) == NULL ||
        part_library_count(library) != 0) {
        printf("FAIL: %s: result %d, error \"%s\" (wanted \"%s\")\n", id, result,
               error != NULL ? error : "(none)", expect);
        failures++;
    } else {
        printf("PASS: %s rejected: %s\n", id, error);
    }
    cJSON_free(text);
    free(error);
    part_library_destroy(library);
}

const Part *qc_load(PartLibrary **library, const char *file, const char *id,
                    const char *const *edits, const char *part) {
    char path[1024];
    char footprints[1024];
    char first[128];
    char *text;
    char *error = NULL;
    const Part *found = NULL;

    paths(file, path, footprints, sizeof path);
    *library = library_for(file, footprints);
    text = edited_file(path, id, edits, first, sizeof first);
    if (load_parts(*library, text, "edited.json", &error) != 0) {
        printf("FAIL: %s does not load: %s\n", id, error != NULL ? error : "(none)");
        failures++;
    } else {
        found = part_library_find(*library, part != NULL ? part : first);
        if (found == NULL) {
            printf("FAIL: %s has no part %s\n", id, part != NULL ? part : first);
            failures++;
        }
    }
    cJSON_free(text);
    free(error);
    return found;
}

Bench *qc_bench(PartLibrary **library, const char *file, const char *id,
                const char *const *edits, const char *part) {
    const Part *found = qc_load(library, file, id, edits, part);

    return found != NULL ? bench_create(found) : NULL;
}

Bench *qc_bench_joined(PartLibrary **library, const char *file, const char *id,
                       const char *const *edits, const char *const *joins) {
    const Part *found = qc_load(library, file, id, edits, NULL);

    return found != NULL ? bench_create_joined(found, joins) : NULL;
}

void qc_set(Bench *b, const char *pin, LogicValue v) {
    bench_set(b, pin, v);
}

void qc_step(Bench *b) {
    bench_run(b, 100000);
}

int qc_is(Bench *b, const char *pin, LogicValue v) {
    return bench_get(b, pin) == v;
}

void qc_set_bus(Bench *b, const char *const *names, unsigned n, unsigned value) {
    unsigned i;

    for (i = 0; i < n; i++) {
        bench_set(b, names[i], (value >> i) & 1u ? LOGIC_1 : LOGIC_0);
    }
}

int qc_bus(Bench *b, const char *const *names, unsigned n, unsigned value) {
    unsigned i;

    for (i = 0; i < n; i++) {
        if (bench_get(b, names[i]) != ((value >> i) & 1u ? LOGIC_1 : LOGIC_0)) {
            return 0;
        }
    }
    return 1;
}

int qc_bus_all(Bench *b, const char *const *names, unsigned n, LogicValue v) {
    unsigned i;

    for (i = 0; i < n; i++) {
        if (bench_get(b, names[i]) != v) {
            return 0;
        }
    }
    return 1;
}

void qc_pulse(Bench *b, const char *pin) {
    bench_set(b, pin, LOGIC_1);
    qc_step(b);
    bench_set(b, pin, LOGIC_0);
    qc_step(b);
}

size_t qc_pin(const Part *part, const char *name) {
    size_t i;

    for (i = 0; i < part->pin_count; i++) {
        if (strcmp(part->pins[i].name, name) == 0) {
            return i;
        }
    }
    die("no pin", name);
    return 0;
}
