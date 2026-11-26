/*
 * oom_test: every allocation the backend makes, made to fail in turn
 * (plan-68-K). For each operation, k = 1, 2, … : the k-th allocation fails;
 * the operation must report failure (with an error message where it has
 * one) and leave no backend memory behind. The sweep ends when an operation
 * makes fewer than k allocations, which must be a success.
 *
 * Operations: creating a library; loading footprints, datasheets, a single
 * part file, and one device of every kind's part file; making a circuit of
 * one part of every kind and running it; reading an Intel HEX image.
 */
#include <dirent.h>
#include <mach-o/dyld.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "circuit.h"
#include "erc.h"
#include "kind.h"
#include "logic_alloc.h"
#include "memory_image.h"
#include "parts.h"
#include "read_file.h"

static int failures;

typedef struct {
    int ok;          /* the operation succeeded */
    int has_error;   /* an error message came back (operations with one) */
    int wants_error; /* the operation reports errors as text */
} Outcome;

typedef Outcome (*Operation)(void *arg);

/* The failed allocation's call site as an offset into this program's image,
   for `atos -o oom_test -l 0x100000000 <0x100000000 + offset>`. */
static unsigned long site_offset(void) {
    return (unsigned long)((const char *)logic_alloc_failed_site() - (const char *)_dyld_get_image_header(0));
}

/* Runs the sweep for one operation; returns how many allocations it makes. */
static long sweep(const char *what, Operation op, void *arg) {
    long k;

    for (k = 1;; k++) {
        long live = logic_alloc_live();
        Outcome out;
        long made;

        logic_alloc_fail_at(k);
        out = op(arg);
        made = logic_alloc_count();
        logic_alloc_fail_at(0);
        /* a run that succeeded may keep what it made (a library keeps what it loads) */
        if (made >= k && logic_alloc_live() != live) {
            printf("FAIL: %s: allocation %ld failing (called from image+0x%lx) leaves %ld allocations behind\n",
                   what, k, site_offset(), logic_alloc_live() - live);
            failures++;
            return k;
        }
        if (made < k) {
            if (!out.ok) {
                printf("FAIL: %s: fails with no allocation failing (%ld made)\n", what, made);
                failures++;
            }
            return made;
        }
        if (out.ok) {
            printf("FAIL: %s: allocation %ld failing (called from image+0x%lx) is not reported\n", what, k,
                   site_offset());
            failures++;
            return k;
        }
        if (out.wants_error && !out.has_error) {
            printf("FAIL: %s: allocation %ld failing gives no error message\n", what, k);
            failures++;
            return k;
        }
    }
}

/* ---- Operations ---- */

static Outcome create_library(void *arg) {
    PartLibrary *library = part_library_create();
    Outcome out = {library != NULL, 0, 0};

    (void)arg;
    part_library_destroy(library);
    return out;
}

typedef struct {
    int (*load)(PartLibrary *, const char *, const char *, char **);
    PartLibrary *library;
    const char *text;
    const char *source;
} Load;

static Outcome load_text(void *arg) {
    Load *l = arg;
    PartLibrary *library = l->library;
    char *error = NULL;
    Outcome out = {0, 0, 1};

    out.ok = l->load(library, l->text, l->source, &error) == 0;
    out.has_error = error != NULL;
    free(error);
    return out;
}

typedef struct {
    const PartLibrary *library;
    const char *part;
    int runs;  /* the part starts on its own (an MCU without a program does not) */
    int wired; /* its pins go on nets */
} OnePart;

/* Every pin of part `index` on a net of its own. 0, or -1 on a failure. */
static int wire_pins(Circuit *circuit, const Part *part, size_t index) {
    size_t i, net;

    for (i = 0; i < part->pin_count; i++) {
        if (part->pins[i].designator != NULL &&
            (circuit_add_net(circuit, &net) != 0 || circuit_connect(circuit, index, part->pins[i].designator, net) != 0)) {
            return -1;
        }
    }
    return 0;
}

static Outcome run_part(void *arg) {
    OnePart *p = arg;
    Outcome out = {0, 0, 0};
    const Part *part = part_library_find(p->library, p->part);
    Circuit *circuit = circuit_create(p->library);
    size_t index;

    /* add_part, connect, start: 0 on success; advance: nonzero while running */
    if (circuit != NULL && circuit_add_part(circuit, p->part, &index) == 0 &&
        (!p->wired || wire_pins(circuit, part, index) == 0) &&
        (!p->runs || (circuit_start(circuit) == 0 && circuit_advance(circuit, 1000000) != 0 &&
                      (part->kind != PART_SWITCH || circuit_set_switch(circuit, index, 1, 1) == 0) &&
                      circuit_advance(circuit, 1000000) != 0))) {
        out.ok = 1;
    }
    circuit_destroy(circuit);
    return out;
}

static Outcome parse_hex(void *arg) {
    static const char HEX[] = ":0400000001020304F2\n:00000001FF\n";
    uint32_t values[16];
    char *error = NULL;
    Outcome out = {0, 0, 1};

    (void)arg;
    out.ok = memory_image_parse((const unsigned char *)HEX, sizeof HEX - 1, MEMORY_IMAGE_INTEL_HEX, 16, 8,
                                values, &error) == 0;
    out.has_error = error != NULL;
    free(error);
    return out;
}

/* An ERC check with more problems than its first list holds: every pin an output, on one net. */
static Outcome erc_many(void *arg) {
    ErcPin pins[80];
    size_t i, count;
    Outcome out = {0, 0, 0};

    (void)arg;
    for (i = 0; i < 80; i++) {
        pins[i].net = 0;
        pins[i].type = ERC_OUTPUT;
        pins[i].drives_power = 0;
        pins[i].no_connect_mark = 0;
    }
    count = erc_check(pins, 80, 1, NULL, ERC_DEFAULT_PIN_MAP, NULL, 0);
    out.ok = count != ERC_CHECK_OUT_OF_MEMORY && count > 64;
    return out;
}

typedef struct {
    const PartLibrary *library;
    const char *names[64];
    size_t count;
    int (*after_start)(Circuit *, size_t first);
    int (*before_start)(Circuit *, size_t first);
} Several;

static int power_timer(Circuit *circuit, size_t timer);

/* Parts added, started, maybe poked, run. */
static Outcome run_several(void *arg) {
    Several *p = arg;
    Outcome out = {0, 0, 0};
    Circuit *circuit = circuit_create(p->library);
    size_t i, index, first = 0;
    int ok = circuit != NULL;

    for (i = 0; ok && i < p->count; i++) {
        ok = circuit_add_part(circuit, p->names[i], &index) == 0;
        if (i == 0) {
            first = index;
        }
    }
    ok = ok && (p->before_start == NULL || p->before_start(circuit, first) == 0) && circuit_start(circuit) == 0 &&
         (p->after_start == NULL || p->after_start(circuit, first) == 0) &&
         circuit_advance(circuit, 1000000) != 0;
    out.ok = ok;
    circuit_destroy(circuit);
    return out;
}

static int slide_to_state_3(Circuit *circuit, size_t part) {
    return circuit_set_switch_state(circuit, part, 3);
}

static int plug_in(Circuit *circuit, size_t part) {
    return circuit_set_connector_plug(circuit, part, 1);
}

static int type_bytes(Circuit *circuit, size_t part) {
    static const uint8_t TEXT[200] = {'h', 'e', 'l', 'l', 'o'};

    /* 60 bytes, some of them sent (the queue's head moves on), then 200 more: the
       queue grows while it wraps */
    return circuit_serial_send(circuit, part, 0, TEXT, 60) != 0 || circuit_advance(circuit, 100000000000ULL) == 0 ||
                   circuit_serial_send(circuit, part, 0, TEXT, sizeof TEXT) != 0
               ? -1
               : 0;
}

/* A pin of `part` found by its name: its designator, or NULL. */
static const char *pin_named(const Part *part, const char *name) {
    size_t i;

    for (i = 0; i < part->pin_count; i++) {
        if (part->pins[i].name != NULL && strcmp(part->pins[i].name, name) == 0) {
            return part->pins[i].designator;
        }
    }
    return NULL;
}

static const PartLibrary *loop_library;

/* A USB-UART bridge's TxD looped back to its RxD: what it sends, it receives and
   logs, more frames than the log starts with. */
static int loop_back(Circuit *circuit, size_t bridge) {
    const Part *part = part_library_find(loop_library, "CY7C64225-28PVXC");
    size_t net;

    return circuit_add_net(circuit, &net) != 0 || circuit_connect(circuit, bridge, pin_named(part, "TxD"), net) != 0 ||
                   circuit_connect(circuit, bridge, pin_named(part, "RxD"), net) != 0
               ? -1
               : 0;
}

/* Pins 1 and 2 of part `index` on nets of their own. */
static int wire_two(Circuit *circuit, size_t index) {
    size_t a, b;

    return circuit_add_net(circuit, &a) != 0 || circuit_add_net(circuit, &b) != 0 ||
                   circuit_connect(circuit, index, "1", a) != 0 || circuit_connect(circuit, index, "2", b) != 0
               ? -1
               : 0;
}

/* The message a refused load gives with memory to spare. */
static char *refused_message;

/* A timer powered from 5 V and GND, among more resistors than the RC links
   start with, so the analysis grows its link list. */
static int power_timer(Circuit *circuit, size_t timer) {
    size_t up, down, vcc, gnd, thres;

    /* the first capacitor (part timer + 2) from THRES to GND: an RC timer, not a latch */
    return circuit_add_part(circuit, "5V", &up) != 0 || circuit_add_part(circuit, "GND", &down) != 0 ||
                   circuit_add_net(circuit, &vcc) != 0 || circuit_add_net(circuit, &gnd) != 0 ||
                   circuit_connect(circuit, up, "1", vcc) != 0 || circuit_connect(circuit, down, "1", gnd) != 0 ||
                   circuit_connect(circuit, timer, "8", vcc) != 0 || circuit_connect(circuit, timer, "1", gnd) != 0 ||
                   circuit_add_net(circuit, &thres) != 0 || circuit_connect(circuit, timer, "6", thres) != 0 ||
                   circuit_connect(circuit, timer + 2, "1", thres) != 0 || circuit_connect(circuit, timer + 2, "2", gnd) != 0
               ? -1
               : 0;
}

/* A load the loader refuses: the message's own allocation fails in turn too. */
static Outcome load_refused(void *arg) {
    Load *l = arg;
    char *error = NULL;
    Outcome out = {0, 0, 1};
    int refused = l->load(l->library, l->text, l->source, &error) != 0;

    /* "success" is the refusal with its own message; a failed allocation gives another */
    out.ok = refused && error != NULL && strcmp(error, refused_message) == 0;
    out.has_error = error != NULL;
    free(error);
    return out;
}

/* Removing a part file's part (`retire` or the last file's unload), each
   allocation failing in turn: the call reports it, and the library keeps the
   part and leaks nothing. The part is loaded again before each try. */
/* The inline footprint the part file brings; retiring the part removes it too. */
static const char *const FOOTPRINT_IDS[] = {"CUSTOM-6F1C2E8F"};

static long sweep_removal(const char *what, PartLibrary *library, const char *text, const char *uuid, int retire) {
    long k;

    for (k = 1;; k++) {
        char *error = NULL;
        long live, made;
        int result;

        if (load_part_file(library, text, "oom.schpart", &error) != 0) {
            printf("FAIL: %s: the part does not load: %s\n", what, error != NULL ? error : "?");
            free(error);
            failures++;
            return k;
        }
        live = logic_alloc_live();
        logic_alloc_fail_at(k);
        result = retire ? part_library_retire_part(library, uuid, FOOTPRINT_IDS, 1, NULL, 0)
                        : part_library_unload_last_part_file(library);
        made = logic_alloc_count();
        logic_alloc_fail_at(0);
        if (made < k) {
            if (result != 0 || part_library_find_uuid(library, uuid) != NULL) {
                printf("FAIL: %s: fails with no allocation failing\n", what);
                failures++;
            }
            return made;
        }
        if (result == 0 || part_library_find_uuid(library, uuid) == NULL || logic_alloc_live() != live) {
            printf("FAIL: %s: allocation %ld failing (called from image+0x%lx): result %d, part %s, %ld left\n", what,
                   k, site_offset(), result, part_library_find_uuid(library, uuid) ? "kept" : "gone",
                   logic_alloc_live() - live);
            failures++;
            return k;
        }
        part_library_unload_last_part_file(library);
    }
}

/* ---- Inputs ---- */

static char *parts_path(const char *file) {
    static char path[4096];

    snprintf(path, sizeof path, "%s/%s", PARTS_DIR, file);
    return path;
}

/* A file of one entry per value of `key` (or the first `limit` entries when key is NULL). */
static char *subset(const char *file, const char *list, const char *key, int limit) {
    char *text = read_file(parts_path(file));
    cJSON *root = cJSON_Parse(text);
    cJSON *entries = cJSON_GetObjectItemCaseSensitive(root, list);
    cJSON *kept = cJSON_CreateArray();
    cJSON *seen = cJSON_CreateObject();
    cJSON *entry;
    char *out;
    int n = 0;

    cJSON_ArrayForEach(entry, entries) {
        const cJSON *value = key != NULL ? cJSON_GetObjectItemCaseSensitive(entry, key) : NULL;
        const char *name = cJSON_IsString(value) ? value->valuestring : "";

        if (key != NULL ? cJSON_GetObjectItemCaseSensitive(seen, name) == NULL : n < limit) {
            cJSON_AddItemToArray(kept, cJSON_Duplicate(entry, 1));
            cJSON_AddTrueToObject(seen, name);
            n++;
        }
    }
    cJSON_ReplaceItemInObjectCaseSensitive(root, list, kept);
    out = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    cJSON_Delete(seen);
    free(text);
    return out;
}

/* A device's shape: its sorted keys, with its "model" or "type" value. Devices
   of one shape go through the same loader and model code. */
static void shape_of(const cJSON *device, char *out, size_t cap) {
    const cJSON *key;
    const cJSON *model = cJSON_GetObjectItemCaseSensitive(device, "model");
    const cJSON *type = cJSON_GetObjectItemCaseSensitive(device, "type");

    snprintf(out, cap, "%s/%s:", cJSON_IsString(model) ? model->valuestring : "",
             cJSON_IsString(type) ? type->valuestring : "");
    cJSON_ArrayForEach(key, device) {
        if (strcmp(key->string, "parts") != 0 && strcmp(key->string, "id") != 0 &&
            strcmp(key->string, "description") != 0 && strcmp(key->string, "note") != 0 &&
            strlen(out) + strlen(key->string) + 2 < cap) {
            strcat(out, key->string);
            strcat(out, ",");
        }
    }
}

/* Device `at` of a kind's file, alone, with its first part. */
static char *nth_device(const char *file, int at, char **part_name) {
    char *text = read_file(parts_path(file));
    cJSON *root = cJSON_Parse(text);
    cJSON *devices = cJSON_GetObjectItemCaseSensitive(root, "devices");
    cJSON *device = cJSON_DetachItemFromArray(devices, at);
    cJSON *parts = cJSON_GetObjectItemCaseSensitive(device, "parts");
    const cJSON *name = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(parts, 0), "name");
    char *out;

    while (cJSON_GetArraySize(parts) > 1) {
        cJSON_DeleteItemFromArray(parts, 1);
    }
    *part_name = cJSON_IsString(name) ? strdup(name->valuestring) : NULL;
    while (cJSON_GetArraySize(devices) > 0) {
        cJSON_DeleteItemFromArray(devices, 0);
    }
    cJSON_AddItemToArray(devices, device);
    out = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    free(text);
    return out;
}

/* The first device of each shape in a kind's file: their indexes. */
static int shapes(const char *file, int *at, int cap) {
    char *text = read_file(parts_path(file));
    cJSON *root = cJSON_Parse(text);
    cJSON *devices = cJSON_GetObjectItemCaseSensitive(root, "devices");
    cJSON *seen = cJSON_CreateObject();
    int i, n = 0;

    for (i = 0; i < cJSON_GetArraySize(devices) && n < cap; i++) {
        cJSON *device = cJSON_GetArrayItem(devices, i);
        const cJSON *first = cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(device, "parts"), 0);
        char shape[2048];

        shape_of(device, shape, sizeof shape);
        if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(first, "name")) &&
            cJSON_GetObjectItemCaseSensitive(seen, shape) == NULL) {
            cJSON_AddTrueToObject(seen, shape);
            at[n++] = i;
        }
    }
    cJSON_Delete(seen);
    cJSON_Delete(root);
    free(text);
    return n;
}

/* A kind's file with only its first device, and that device's first part. */
static char *one_device(const char *file, char **part_name) {
    char *text = read_file(parts_path(file));
    cJSON *root = cJSON_Parse(text);
    cJSON *devices = cJSON_GetObjectItemCaseSensitive(root, "devices");
    cJSON *device = NULL;
    cJSON *parts = NULL;
    cJSON *part;
    char *out;
    int at;

    /* the first device with a named part (a device may list its parts
       elsewhere, as a supply's do) */
    for (at = 0; at < cJSON_GetArraySize(devices); at++) {
        cJSON *d = cJSON_GetArrayItem(devices, at);
        cJSON *p = cJSON_GetObjectItemCaseSensitive(d, "parts");
        cJSON *first = cJSON_GetArrayItem(p, 0);

        if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(first, "name"))) {
            device = cJSON_DetachItemFromArray(devices, at);
            parts = cJSON_GetObjectItemCaseSensitive(device, "parts");
            break;
        }
    }
    if (device == NULL) {
        cJSON_Delete(root);
        free(text);
        *part_name = NULL;
        return NULL;
    }

    while (cJSON_GetArraySize(parts) > 1) {
        cJSON_DeleteItemFromArray(parts, 1);
    }
    part = cJSON_GetArrayItem(parts, 0);
    *part_name = strdup(cJSON_GetObjectItemCaseSensitive(part, "name")->valuestring);
    while (cJSON_GetArraySize(devices) > 0) {
        cJSON_DeleteItemFromArray(devices, 0);
    }
    cJSON_AddItemToArray(devices, device);
    out = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    free(text);
    return out;
}

/* Whether a circuit takes the part (with nothing failing). */
static int simulated(const PartLibrary *library, const char *name) {
    Circuit *circuit = circuit_create(library);
    size_t index;
    int ok = circuit != NULL && circuit_add_part(circuit, name, &index) == 0;

    circuit_destroy(circuit);
    return ok;
}

/* The first part in a kind's file that a circuit takes, or NULL. */
static char *first_simulated(const PartLibrary *library, const char *file) {
    char *text = read_file(parts_path(file));
    cJSON *root = cJSON_Parse(text);
    cJSON *device;
    char *found = NULL;

    cJSON_ArrayForEach(device, cJSON_GetObjectItemCaseSensitive(root, "devices")) {
        cJSON *part;

        cJSON_ArrayForEach(part, cJSON_GetObjectItemCaseSensitive(device, "parts")) {
            const cJSON *name = cJSON_GetObjectItemCaseSensitive(part, "name");

            if (found == NULL && cJSON_IsString(name) && simulated(library, name->valuestring)) {
                found = strdup(name->valuestring);
            }
        }
    }
    cJSON_Delete(root);
    free(text);
    return found;
}

static PartLibrary *base_library(void) {
    PartLibrary *library = part_library_create();
    char *text = read_file(parts_path("footprints.json"));
    char *error = NULL;

    if (load_datasheets_file(library, PARTS_DIR) != 0 || load_footprints(library, text, "footprints.json", &error) != 0) {
        printf("FAIL: base library: %s\n", error != NULL ? error : "?");
        failures++;
    }
    free(error);
    free(text);
    return library;
}

static int by_name(const void *a, const void *b) {
    return strcmp(*(const char *const *)a, *(const char *const *)b);
}

int main(void) {
    /* part_file_test's case (a): a NAND on an inline 5-pad footprint. */
    static const char PART_FILE[] =
        "{\"format\":\"schematical-part\",\"version\":1,\"uuid\":\"6F1C2E8A-3B7D-4C55-9A61-0E2B4D7F9C1F\","
        "\"kind\":\"gate\",\"footprints\":[{\"id\":\"CUSTOM-6F1C2E8F\",\"description\":\"Test pads\","
        "\"style\":\"explicit\",\"pins\":5,\"body\":{\"width\":3.0,\"height\":3.0},\"pads\":["
        "{\"pin\":1,\"x\":-1,\"y\":-1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"
        "{\"pin\":2,\"x\":-1,\"y\":0,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"
        "{\"pin\":3,\"x\":-1,\"y\":1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"
        "{\"pin\":4,\"x\":1,\"y\":1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"
        "{\"pin\":5,\"x\":1,\"y\":-1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"}]}],"
        "\"devices\":[{\"id\":\"1G00\",\"function\":\"NAND\",\"description\":\"Single 2-input NAND gate\","
        "\"drawer\":[\"Custom\",\"Gates\"],"
        "\"pinouts\":{\"p5\":[{\"number\":1,\"name\":\"A\",\"role\":\"input\"},"
        "{\"number\":2,\"name\":\"B\",\"role\":\"input\"},{\"number\":3,\"name\":\"GND\",\"role\":\"ground\"},"
        "{\"number\":4,\"name\":\"Y\",\"role\":\"output\"},{\"number\":5,\"name\":\"VCC\",\"role\":\"power\"}]},"
        "\"gates\":[{\"op\":\"nand\",\"inputs\":[\"A\",\"B\"],\"output\":\"Y\"}],"
        "\"timing\":{\"HC\":{\"rise\":9,\"fall\":9,\"vcc\":[2,6]}},"
        "\"parts\":[{\"name\":\"OOM-NAND\",\"manufacturer\":\"Generic\",\"package\":\"SOT-23\","
        "\"pinout\":\"p5\",\"footprint\":\"CUSTOM-6F1C2E8F\",\"family\":\"HC\"}]}]}";
    DIR *dir = opendir(PARTS_DIR);
    struct dirent *entry;
    char *files[128];
    size_t count = 0, i;
    PartLibrary *all = part_library_create();
    long made;
    int shape_count = 0;

    logic_alloc_init();
    made = sweep("part_library_create", create_library, NULL);
    printf("ok   part_library_create (%ld allocations)\n", made);

    {
        char *footprints = subset("footprints.json", "footprints", "style", 0);
        char *datasheets = subset("datasheets.json", "datasheets", NULL, 8);
        PartLibrary *library = part_library_create();
        Load fp = {load_footprints, library, footprints, "footprints.json"};
        Load ds = {load_datasheets, library, datasheets, "datasheets.json"};
        Load pf;
        PartLibrary *base = base_library();

        /* The library's own loaders with every kind_alloc a chunk of its own,
           so each allocation failing in turn reaches each caller's NULL check
           (plan-68-L; the kinds' loaders get the same in their own letters). */
        kind_arena_chunk = 16;
        made = sweep("load_footprints (one per style)", load_text, &fp);
        printf("ok   load_footprints (%ld allocations)\n", made);
        made = sweep("load_datasheets (eight)", load_text, &ds);
        printf("ok   load_datasheets (%ld allocations)\n", made);
        pf.load = load_part_file;
        pf.library = base;
        pf.text = PART_FILE;
        pf.source = "oom.schpart";
        made = sweep("load_part_file", load_text, &pf);
        printf("ok   load_part_file (%ld allocations)\n", made);
        part_library_unload_last_part_file(base);
        {
            /* The same part from a maker, with its datasheet inline. */
            static const char MARK[] = "\"manufacturer\":\"Generic\",";
            static const char WITH[] = "\"manufacturer\":\"Maker\",\"datasheet\":\"oom-sheet\",";
            const char *at = strstr(PART_FILE, MARK);
            char sheet_file[4096];
            Load ds_file;

            snprintf(sheet_file, sizeof sheet_file, "%.*s%s%s", (int)(at - PART_FILE), PART_FILE, WITH, at + strlen(MARK));
            /* inline datasheets go before "devices" */
            {
                char *devices = strstr(sheet_file, "\"devices\"");
                char tail[4096];

                snprintf(tail, sizeof tail, "%s", devices);
                snprintf(devices, sizeof sheet_file - (size_t)(devices - sheet_file),
                         "\"datasheets\":[{\"id\":\"oom-sheet\",\"url\":\"https://example.com/a.pdf\"}],%s", tail);
            }
            ds_file.load = load_part_file;
            ds_file.library = base;
            ds_file.text = sheet_file;
            ds_file.source = "oom-sheet.schpart";
            made = sweep("load_part_file with an inline datasheet", load_text, &ds_file);
            printf("ok   load_part_file, inline datasheet (%ld allocations)\n", made);
            part_library_unload_last_part_file(base);
        }
        made = sweep_removal("part_library_retire_part", base, PART_FILE, "6F1C2E8A-3B7D-4C55-9A61-0E2B4D7F9C1F", 1);
        printf("ok   part_library_retire_part (%ld allocations)\n", made);
        made = sweep_removal("part_library_unload_last_part_file", base, PART_FILE,
                             "6F1C2E8A-3B7D-4C55-9A61-0E2B4D7F9C1F", 0);
        printf("ok   part_library_unload_last_part_file (%ld allocations)\n", made);
        {
            /* The common part keys: pins and balls, flags, footprint choices,
               a socket's fits, a part's maker and datasheet. */
            static const char *const KINDS[] = {"supply.json", "socket.json", "resistor.json", "header.json",
                                                "led.json", "crystal.json"};
            size_t k;

            for (k = 0; k < sizeof KINDS / sizeof KINDS[0]; k++) {
                char *name = NULL;
                char *text = one_device(KINDS[k], &name);
                PartLibrary *fresh = base_library();
                Load one = {load_parts, fresh, text, KINDS[k]};
                char what[128];

                snprintf(what, sizeof what, "load_parts %s, chunked", KINDS[k]);
                made = sweep(what, load_text, &one);
                printf("ok   %s (%ld allocations)\n", what, made);
                part_library_destroy(fresh);
                cJSON_free(text);
                free(name);
            }
        }
        kind_arena_chunk = 64 * 1024;
        part_library_destroy(base);
        part_library_destroy(library);
        cJSON_free(footprints);
        cJSON_free(datasheets);
    }

    while (dir != NULL && (entry = readdir(dir)) != NULL && count < 128) {
        size_t n = strlen(entry->d_name);

        if (n > 5 && strcmp(entry->d_name + n - 5, ".json") == 0 && strcmp(entry->d_name, "footprints.json") != 0 &&
            strcmp(entry->d_name, "datasheets.json") != 0) {
            files[count++] = strdup(entry->d_name);
        }
    }
    if (dir != NULL) {
        closedir(dir);
    }
    qsort(files, count, sizeof files[0], by_name);
    if (load_parts_dir(all, PARTS_DIR) != 0) {
        printf("FAIL: loading the part files\n");
        failures++;
    }
    for (i = 0; i < count; i++) {
        char *name = NULL;
        char *text = one_device(files[i], &name);
        PartLibrary *base;

        if (text == NULL) {
            /* generic.json ships no device of its own: nothing to load */
            printf("ok   %s: no device to load\n", files[i]);
            free(files[i]);
            continue;
        }
        base = base_library();
        Load ld = {load_parts, base, text, files[i]};
        OnePart one = {all, name, 1, 1};
        char what[256];

        snprintf(what, sizeof what, "load_parts %s", files[i]);
        made = sweep(what, load_text, &ld);
        {
            int at[512];
            int n = shapes(files[i], at, 512), j;

            for (j = 1; j < n; j++) {
                char *other = NULL;
                char *device = nth_device(files[i], at[j], &other);
                PartLibrary *fresh = base_library();
                Load each = {load_parts, fresh, device, files[i]};
                OnePart part = {all, other, 1, 1};

                snprintf(what, sizeof what, "load_parts %s device %d", files[i], at[j]);
                sweep(what, load_text, &each);
                if (other != NULL && simulated(all, other)) {
                    snprintf(what, sizeof what, "wired circuit of %s (%s)", other, files[i]);
                    part.runs = run_part(&part).ok;
                    if (!part.runs) {
                        part.wired = 0;
                        part.runs = run_part(&part).ok;
                    }
                    sweep(what, run_part, &part);
                }
                part_library_destroy(fresh);
                cJSON_free(device);
                free(other);
            }
            shape_count += n;
        }
        /* a part of this kind a circuit takes (some kinds' first part is not simulated) */
        if (!simulated(all, name)) {
            free(name);
            name = first_simulated(all, files[i]);
            one.part = name;
        }
        if (name != NULL) {
            snprintf(what, sizeof what, "circuit of %s (%s)", name, files[i]);
            /* found out once, with nothing failing: does it run, and run wired? */
            one.runs = run_part(&one).ok;
            if (!one.runs) {
                one.wired = 0;
                one.runs = run_part(&one).ok;
            }
            sweep(what, run_part, &one);
        }
        printf("ok   %s: one device loads, %s, every allocation failing in turn (%ld)\n", files[i],
               name != NULL ? name : "no part of it is simulated", made);
        part_library_destroy(base);
        cJSON_free(text);
        free(name);
        free(files[i]);
    }
    part_library_destroy(all);

    printf("ok   %d device shapes across the kinds\n", shape_count);
    made = sweep("erc_check with 3160 problems", erc_many, NULL);
    printf("ok   erc_check (%ld allocations)\n", made);
    {
        PartLibrary *lib = part_library_create();
        Several slide = {lib, {"MSN23ANW03"}, 1, slide_to_state_3};
        Several plug = {lib, {"PJ-002A"}, 1, plug_in};
        Several serial = {lib, {"N82510"}, 1, type_bytes};
        Several looped = {lib, {"CY7C64225-28PVXC"}, 1, type_bytes, loop_back};
        Several rc = {lib, {"ICM7555ESA+"}, 1, NULL, power_timer};
        /* A slide switch whose rest position closes contacts, and a tactile
           switch whose joined pins are left unwired (the circuit wires them). */
        Several resting = {lib, {"CS22BNW03"}, 1, NULL};
        Several joined = {lib, {"MJTP1212"}, 1, NULL};
        size_t n;

        load_parts_dir(lib, PARTS_DIR);
        made = sweep("a slide switch resting on closed contacts", run_several, &resting);
        printf("ok   a resting slide switch (%ld allocations)\n", made);
        made = sweep("a tactile switch with unwired joined pins", run_several, &joined);
        printf("ok   unwired joined pins (%ld allocations)\n", made);
        made = sweep("a slide switch set to a state", run_several, &slide);
        printf("ok   circuit_set_switch_state (%ld allocations)\n", made);
        made = sweep("a jack plugged in", run_several, &plug);
        printf("ok   circuit_set_connector_plug (%ld allocations)\n", made);
        made = sweep("200 bytes typed into a UART", run_several, &serial);
        printf("ok   circuit_serial_send (%ld allocations)\n", made);
        loop_library = lib;
        made = sweep("260 bytes through a looped-back USB-UART", run_several, &looped);
        printf("ok   a looped-back USB-UART's log (%ld allocations)\n", made);
        /* a timer with more diodes, capacitors and resistors than the RC lists start with */
        for (n = 0; n < 9; n++) {
            rc.names[rc.count++] = "1N4148-TAP";
            rc.names[rc.count++] = "AEA1616102M035R";
        }
        for (n = 0; n < 17; n++) {
            rc.names[rc.count++] = "CRCW06030000Z0EBC";
        }
        made = sweep("a timer among 9 diodes, 9 capacitors, 17 resistors", run_several, &rc);
        printf("ok   timer RC lists (%ld allocations)\n", made);
        part_library_destroy(lib);
    }
    {
        /* a diode part with 17 diodes: more links than the engine's 16 on the stack */
        static const char BIG[] =
            "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"diode\",\"devices\":[{\"id\":\"OOM-17\","
            "\"function\":\"Array\",\"description\":\"17 diodes\",\"drawer\":[\"Discrete\",\"Diodes\"],"
            "\"pinouts\":{\"p2\":[{\"number\":1,\"name\":\"A\",\"role\":\"inout\"},{\"number\":2,\"name\":\"K\",\"role\":\"inout\"}]},"
            "\"type\":\"switching\",\"diodes\":["
            "{\"anode\":\"A\",\"cathode\":\"K\"},{\"anode\":\"A\",\"cathode\":\"K\"},{\"anode\":\"A\",\"cathode\":\"K\"},"
            "{\"anode\":\"A\",\"cathode\":\"K\"},{\"anode\":\"A\",\"cathode\":\"K\"},{\"anode\":\"A\",\"cathode\":\"K\"},"
            "{\"anode\":\"A\",\"cathode\":\"K\"},{\"anode\":\"A\",\"cathode\":\"K\"},{\"anode\":\"A\",\"cathode\":\"K\"},"
            "{\"anode\":\"A\",\"cathode\":\"K\"},{\"anode\":\"A\",\"cathode\":\"K\"},{\"anode\":\"A\",\"cathode\":\"K\"},"
            "{\"anode\":\"A\",\"cathode\":\"K\"},{\"anode\":\"A\",\"cathode\":\"K\"},{\"anode\":\"A\",\"cathode\":\"K\"},"
            "{\"anode\":\"A\",\"cathode\":\"K\"},{\"anode\":\"A\",\"cathode\":\"K\"}],"
            "\"parts\":[{\"name\":\"OOM-DIODES-17\",\"manufacturer\":\"Generic\",\"package\":\"TH\",\"pinout\":\"p2\",\"vr\":50,\"io\":1,\"footprint\":\"DIODE-DO-35\"}]}]}";
        PartLibrary *lib = base_library();
        char *error = NULL;
        Several big = {lib, {"OOM-DIODES-17"}, 1, NULL, wire_two};
        static const char REFUSED[] =
            "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"gate\",\"devices\":[{\"id\":\"X\",\"bogus\":1}]}";
        Load refused = {load_parts, lib, REFUSED, "refused.json"};

        if (load_parts(lib, BIG, "big.json", &error) != 0) {
            printf("FAIL: the 17-diode part: %s\n", error != NULL ? error : "?");
            failures++;
        } else {
            made = sweep("a part with 17 diodes", run_several, &big);
            printf("ok   17 diode links (%ld allocations)\n", made);
        }
        free(error);
        load_parts(lib, REFUSED, "refused.json", &refused_message);
        made = sweep("a refused load", load_refused, &refused);
        free(refused_message);
        printf("ok   a refused load's message (%ld allocations)\n", made);
        part_library_destroy(lib);
    }
    made = sweep("memory_image_parse", parse_hex, NULL);
    printf("ok   memory_image_parse (%ld allocations)\n", made);

    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("PASS\n");
    return 0;
}
