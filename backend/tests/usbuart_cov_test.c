/*
 * usbuart_cov_test (plan-68-P): the usbuart loader's rejections and a
 * channel without a CTS pin, on variants of the shipped XR21V1410 device
 * (schematical/schematical/Parts/serial.json) edited in place.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "parts.h"
#include "read_file.h"
#include "cJSON.h"

static PartLibrary *library;
static int failures;
static cJSON *shipped;   /* the shipped serial.json */
static unsigned variant_count;

static void check(int ok, const char *what) {
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

#define US(n) ((uint64_t)(n) * 1000000u)

/* ---- Variants of a shipped device ------------------------------------------------------------ */

/* The item at a dot path ("channels.0.cts") under `root`, its parent and last key. */
static cJSON *path_parent(cJSON *root, const char *path, char *last, size_t cap) {
    char buffer[128];
    char *key;
    char *dot;
    cJSON *at = root;

    snprintf(buffer, sizeof buffer, "%s", path);
    key = buffer;
    while ((dot = strchr(key, '.')) != NULL) {
        *dot = '\0';
        at = cJSON_IsArray(at) ? cJSON_GetArrayItem(at, atoi(key))
                               : cJSON_GetObjectItemCaseSensitive(at, key);
        if (at == NULL) {
            return NULL;
        }
        key = dot + 1;
    }
    snprintf(last, cap, "%s", key);
    return at;
}

/*
 * Loads a copy of device `id` with `edits` applied: pairs of a dot path and
 * a JSON value (the text "-" deletes the key), NULL-terminated. The copy gets
 * a fresh id and part names "<name>-V<n>"; *part_name gets the first one.
 * Returns load_parts' result; *error gets its message (free it).
 */
static int load_variant(const char *id, const char *const *edits, char *part_name, size_t cap,
                        char **error) {
    const cJSON *devices = cJSON_GetObjectItemCaseSensitive(shipped, "devices");
    const cJSON *item;
    cJSON *device = NULL;
    cJSON *file;
    cJSON *list;
    cJSON *part;
    char text[64];
    char *json;
    unsigned i;
    int result;

    cJSON_ArrayForEach(item, devices) {
        const cJSON *v = cJSON_GetObjectItemCaseSensitive(item, "id");

        if (cJSON_IsString(v) && strcmp(v->valuestring, id) == 0) {
            device = cJSON_Duplicate(item, 1);
        }
    }
    if (device == NULL) {
        *error = NULL;
        return -2;
    }
    variant_count++;
    snprintf(text, sizeof text, "%s-V%u", id, variant_count);
    cJSON_ReplaceItemInObjectCaseSensitive(device, "id", cJSON_CreateString(text));
    cJSON_ArrayForEach(part, cJSON_GetObjectItemCaseSensitive(device, "parts")) {
        const cJSON *name = cJSON_GetObjectItemCaseSensitive(part, "name");

        snprintf(text, sizeof text, "%s-V%u", name->valuestring, variant_count);
        cJSON_ReplaceItemInObjectCaseSensitive(part, "name", cJSON_CreateString(text));
    }
    part = cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(device, "parts"), 0);
    snprintf(part_name, cap, "%s", cJSON_GetObjectItemCaseSensitive(part, "name")->valuestring);
    for (i = 0; edits[i] != NULL; i += 2) {
        char last[64];
        cJSON *parent = path_parent(device, edits[i], last, sizeof last);

        if (parent == NULL) {
            continue;
        }
        if (strcmp(edits[i + 1], "-") == 0) {
            cJSON_DeleteItemFromObjectCaseSensitive(parent, last);
        } else if (cJSON_GetObjectItemCaseSensitive(parent, last) != NULL) {
            cJSON_ReplaceItemInObjectCaseSensitive(parent, last, cJSON_Parse(edits[i + 1]));
        } else {
            cJSON_AddItemToObject(parent, last, cJSON_Parse(edits[i + 1]));
        }
    }
    file = cJSON_CreateObject();
    cJSON_AddStringToObject(file, "format", "schematical-parts");
    cJSON_AddNumberToObject(file, "version", 1);
    cJSON_AddStringToObject(file, "kind", "serial");
    list = cJSON_AddArrayToObject(file, "devices");
    cJSON_AddItemToArray(list, device);
    json = cJSON_PrintUnformatted(file);
    cJSON_Delete(file);
    *error = NULL;
    result = load_parts(library, json, "usbuart_variant.json", error);
    cJSON_free(json);
    return result;
}

/* The variant is refused with a message containing `want`. */
static void reject(const char *id, const char *const *edits, const char *want) {
    char name[64];
    char *error = NULL;
    int result = load_variant(id, edits, name, sizeof name, &error);
    char message[400];

    snprintf(message, sizeof message, "refused (%s): %s", want, error != NULL ? error : "(loaded)");
    check(result != 0 && error != NULL && strstr(error, want) != NULL, message);
    free(error);
}

/* ---- Loader rejections ------------------------------------------------------------------------ */

static void case_rejections(void) {
    static const char *const NO_CHANNELS[] = {"channels", "-", NULL};
    static const char *const EMPTY[] = {"channels", "[]", NULL};
    static const char *const NINE[] = {"channels", "[{},{},{},{},{},{},{},{},{}]", NULL};
    static const char *const NOT_ARRAY[] = {"channels", "{}", NULL};
    static const char *const UNKNOWN_KEY[] = {"channels.0.dtrLed", "\"DTR\"", NULL};
    static const char *const BAD_NAME[] = {"channels.0.rts", "7", NULL};
    static const char *const NO_TXD[] = {"channels.0.txd", "-", NULL};
    static const char *const NO_RXD[] = {"channels.0.rxd", "-", NULL};
    static const char *const BAD_RESET[] = {"reset", "true", NULL};
    static const char *const UNBOUND_PIN[] = {"channels.0.cts", "\"NOPE\"", NULL};
    static const char *const UNBOUND_RESET[] = {"reset", "\"NOPE\"", NULL};

    reject("XR21V1410", NO_CHANNELS, "channels");
    reject("XR21V1410", EMPTY, "\"channels\" must be an array of 1 to");
    reject("XR21V1410", NINE, "\"channels\" must be an array of 1 to");
    reject("XR21V1410", NOT_ARRAY, "\"channels\" must be an array of 1 to");
    reject("XR21V1410", UNKNOWN_KEY, "dtrLed");
    reject("XR21V1410", BAD_NAME, "rts");
    reject("XR21V1410", NO_TXD, "missing \"txd\"");
    reject("XR21V1410", NO_RXD, "missing \"rxd\"");
    reject("XR21V1410", BAD_RESET, "reset");
    reject("XR21V1410", UNBOUND_PIN, "NOPE");
    reject("XR21V1410", UNBOUND_RESET, "NOPE");
}

/* ---- A channel without CTS ---------------------------------------------------------------------- */

/* XR21V1410 (QFN-16) without its "cts" key: 9 TX, 16 VCC, 1 and 13 GND. With
   RTS/CTS flow control a frame starts only while CTS reads 0; an absent CTS
   reads 1 like an open one (the pull-up), so nothing goes until flow is off. */
static void case_no_cts(void) {
    static const char *const NO_CTS[] = {"channels.0.cts", "-", NULL};
    char name[64];
    char *error = NULL;
    Circuit *c;
    size_t chip = 0;
    size_t vcc = 0, gnd = 0, tx = 0, n = 0;

    check(load_variant("XR21V1410", NO_CTS, name, sizeof name, &error) == 0,
          "XR21V1410 without cts loads");
    free(error);
    c = circuit_create(library);
    circuit_add_part(c, name, &chip);
    circuit_add_net(c, &vcc);
    circuit_add_part(c, "3V3", &n);
    circuit_connect(c, n, "1", vcc);
    circuit_add_net(c, &gnd);
    circuit_add_part(c, "GND", &n);
    circuit_connect(c, n, "1", gnd);
    circuit_connect(c, chip, "16", vcc);
    circuit_connect(c, chip, "1", gnd);
    circuit_connect(c, chip, "13", gnd);
    circuit_add_net(c, &tx);
    circuit_connect(c, chip, "9", tx);
    check(circuit_serial_configure(c, chip, 0, 115200, 8, 0, 2, 1) == 0, "RTS/CTS flow on");
    check(circuit_start(c) == 0, "the variant starts");
    circuit_serial_send(c, chip, 0, (const uint8_t *)"A", 1);
    circuit_advance(c, US(200));
    check(circuit_net_value(c, tx) == CIRCUIT_1, "no CTS pin reads 1: flow control holds TX");
    check(circuit_serial_configure(c, chip, 0, 115200, 8, 0, 2, 0) == 0, "flow off");
    circuit_advance(c, US(30));
    check(circuit_net_value(c, tx) == CIRCUIT_0, "flow off: the queued byte starts");
    circuit_destroy(c);
}

int main(void) {
    char path[512];
    char *text;

    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL: loading parts\n");
        return 1;
    }
    snprintf(path, sizeof path, "%s/serial.json", PARTS_DIR);
    text = read_file(path);
    shipped = text != NULL ? cJSON_Parse(text) : NULL;
    free(text);
    if (shipped == NULL) {
        printf("FAIL: reading %s\n", path);
        part_library_destroy(library);
        return 1;
    }
    case_rejections();
    case_no_cts();
    cJSON_Delete(shipped);
    part_library_destroy(library);
    printf(failures ? "%d FAILED\n" : "all passed\n", failures);
    return failures != 0;
}
