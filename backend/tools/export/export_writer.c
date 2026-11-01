#include "export.h"

#include <stdlib.h>
#include <string.h>

static cJSON *file_root;
static cJSON *file_devices;

void export_begin(const char *kind) {
    cJSON_Delete(file_root);
    file_root = cJSON_CreateObject();
    cJSON_AddStringToObject(file_root, "format", "schematical-parts");
    cJSON_AddNumberToObject(file_root, "version", 1);
    cJSON_AddStringToObject(file_root, "kind", kind);
    file_devices = cJSON_AddArrayToObject(file_root, "devices");
}

void export_device(cJSON *device) {
    cJSON_AddItemToArray(file_devices, device);
}

cJSON *export_find_device(const char *id) {
    cJSON *device;

    cJSON_ArrayForEach(device, file_devices) {
        if (strcmp(cJSON_GetObjectItemCaseSensitive(device, "id")->valuestring, id) == 0) {
            return device;
        }
    }
    return NULL;
}

cJSON *export_new_device(const char *id, const char *function, const char *description,
                         const char *drawer0, const char *drawer1, const char *note) {
    cJSON *device = cJSON_CreateObject();
    cJSON *drawer;

    cJSON_AddStringToObject(device, "id", id);
    cJSON_AddStringToObject(device, "function", function);
    cJSON_AddStringToObject(device, "description", description);
    drawer = cJSON_AddArrayToObject(device, "drawer");
    if (drawer0 != NULL) {
        cJSON_AddItemToArray(drawer, cJSON_CreateString(drawer0));
    }
    if (drawer1 != NULL) {
        cJSON_AddItemToArray(drawer, cJSON_CreateString(drawer1));
    }
    if (note != NULL) {
        cJSON_AddStringToObject(device, "note", note);
    }
    cJSON_AddObjectToObject(device, "pinouts");
    return device;
}

static const char *role_name(const ChipPin *pin) {
    if (pin->group != NULL && strcmp(pin->group, CHIP_PIN_GROUP_HIDDEN) == 0) {
        return "absent";
    }
    switch (pin->role) {
        case PIN_ROLE_INPUT: return "input";
        case PIN_ROLE_OUTPUT: return "output";
        case PIN_ROLE_INOUT: return "inout";
        case PIN_ROLE_POWER: return "power";
        case PIN_ROLE_GROUND: return "ground";
        case PIN_ROLE_NO_CONNECT: return "nc";
    }
    return "nc";
}

cJSON *export_pins(const ChipPin *pins, size_t count) {
    cJSON *array = cJSON_CreateArray();
    size_t i;

    for (i = 0; i < count; i++) {
        cJSON *pin = cJSON_CreateObject();

        cJSON_AddNumberToObject(pin, "number", pins[i].number);
        cJSON_AddStringToObject(pin, "name", pins[i].name);
        cJSON_AddStringToObject(pin, "role", role_name(&pins[i]));
        if (pins[i].group != NULL && strcmp(pins[i].group, CHIP_PIN_GROUP_HIDDEN) != 0 &&
            strcmp(pins[i].group, CHIP_PIN_GROUP_DEFAULT) != 0) {
            cJSON_AddStringToObject(pin, "group", pins[i].group);
        }
        cJSON_AddItemToArray(array, pin);
    }
    return array;
}

void export_add_pinout(cJSON *device, const char *name, cJSON *pins) {
    cJSON_AddItemToObject(cJSON_GetObjectItemCaseSensitive(device, "pinouts"), name, pins);
}

cJSON *export_parts_array(cJSON *device) {
    cJSON *parts = cJSON_GetObjectItemCaseSensitive(device, "parts");

    return parts != NULL ? parts : cJSON_AddArrayToObject(device, "parts");
}

cJSON *export_new_part(const char *name, const char *package, const char *pinout) {
    cJSON *part = cJSON_CreateObject();

    cJSON_AddStringToObject(part, "name", name);
    cJSON_AddStringToObject(part, "package", package);
    cJSON_AddStringToObject(part, "pinout", pinout);
    return part;
}

void export_part_flags(cJSON *part, unsigned flags) {
    static const char *const NAMES[] = {"obsolete", "military", "automotive", "enhanced"};
    cJSON *array;
    int i;

    if (flags == 0) {
        return;
    }
    array = cJSON_AddArrayToObject(part, "flags");
    for (i = 0; i < 4; i++) {
        if (flags & (1u << i)) {
            cJSON_AddItemToArray(array, cJSON_CreateString(NAMES[i]));
        }
    }
}

cJSON *export_ns(unsigned long long ps) {
    return cJSON_CreateNumber((double)ps / 1000.0);
}

int export_same_pins(const ChipPin *a, size_t a_count, const ChipPin *b, size_t b_count) {
    size_t i;

    if (a_count != b_count) {
        return 0;
    }
    for (i = 0; i < a_count; i++) {
        const char *ga = a[i].group != NULL ? a[i].group : CHIP_PIN_GROUP_DEFAULT;
        const char *gb = b[i].group != NULL ? b[i].group : CHIP_PIN_GROUP_DEFAULT;

        if (a[i].number != b[i].number || strcmp(a[i].name, b[i].name) != 0 ||
            a[i].role != b[i].role || strcmp(ga, gb) != 0) {
            return 0;
        }
    }
    return 1;
}

int export_same_json(const cJSON *a, const cJSON *b) {
    return cJSON_Compare(a, b, 1);
}

const char *export_package_name(int package) {
    switch ((PackageType)package) {
        case PACKAGE_TYPE_SOIC: return "SOIC";
        case PACKAGE_TYPE_SOP: return "SOP";
        case PACKAGE_TYPE_SSOP: return "SSOP";
        case PACKAGE_TYPE_DIP: return "DIP";
        case PACKAGE_TYPE_TSSOP: return "TSSOP";
        case PACKAGE_TYPE_TVSOP: return "TVSOP";
        case PACKAGE_TYPE_CDIP: return "CDIP";
        case PACKAGE_TYPE_CFP: return "CFP";
        case PACKAGE_TYPE_LCCC: return "LCCC";
        case PACKAGE_TYPE_WQFN: return "WQFN";
        case PACKAGE_TYPE_VQFN: return "VQFN";
        case PACKAGE_TYPE_VSSOP: return "VSSOP";
        case PACKAGE_TYPE_SOT23: return "SOT-23";
        case PACKAGE_TYPE_SC70: return "SC-70";
        case PACKAGE_TYPE_DSBGA: return "DSBGA";
        case PACKAGE_TYPE_SON: return "SON";
        case PACKAGE_TYPE_UQFN: return "UQFN";
        case PACKAGE_TYPE_X2SON: return "X2SON";
        case PACKAGE_TYPE_SOT5X3: return "SOT-5X3";
        case PACKAGE_TYPE_SOJ: return "SOJ";
        case PACKAGE_TYPE_TSOP_I: return "TSOP-I";
        case PACKAGE_TYPE_STSOP_I: return "sTSOP-I";
        case PACKAGE_TYPE_TSOP_II: return "TSOP-II";
        case PACKAGE_TYPE_TFBGA: return "TFBGA";
        case PACKAGE_TYPE_SMD: return "SMD";
    }
    fprintf(stderr, "unknown package %d\n", package);
    exit(2);
}

const char *export_family_name(int family) {
    static const char *const NAMES[] = {
        "HC", "LS", "ACT", "HCT", "F", "AC", "AHCT", "ALS", "AS", "AHC", "S", "C",
        "H", "HCS", "LV", "LVC", "AUP", "AUC", "ALVC", "TTL"
    };

    if (family >= 0 && (size_t)family < sizeof(NAMES) / sizeof(NAMES[0])) {
        return NAMES[family];
    }
    fprintf(stderr, "not a logic family: %d\n", family);
    exit(2);
}

/* ---- Writing --------------------------------------------------------------- */

static void indent(FILE *out, int level) {
    int i;

    for (i = 0; i < level; i++) {
        fputs("  ", out);
    }
}

static void write_inline(FILE *out, const cJSON *value) {
    char *text = cJSON_PrintUnformatted(value);

    fputs(text, out);
    free(text);
}

static int is_container(const cJSON *value) {
    return cJSON_IsArray(value) || cJSON_IsObject(value);
}

/* An object or array of containers: one element per line. Anything else inline. */
static int spread(const cJSON *value) {
    const cJSON *child;
    int count = 0;

    if (!is_container(value)) {
        return 0;
    }
    cJSON_ArrayForEach(child, value) {
        if (!is_container(child)) {
            return 0;
        }
        count++;
    }
    return count > 1;
}

static void write_key(FILE *out, const char *key) {
    cJSON *name = cJSON_CreateString(key);

    write_inline(out, name);
    fputs(": ", out);
    cJSON_Delete(name);
}

enum { MODE_INLINE, MODE_AUTO, MODE_LINES, MODE_PINOUTS };

static void write_value(FILE *out, const cJSON *value, int level, int mode);

/* One child per line; children are written in child_mode. */
static void write_lines(FILE *out, const cJSON *value, int level, int child_mode) {
    const cJSON *child;
    int object = cJSON_IsObject(value);

    fputs(object ? "{\n" : "[\n", out);
    cJSON_ArrayForEach(child, value) {
        indent(out, level + 1);
        if (object) {
            write_key(out, child->string);
        }
        write_value(out, child, level + 1, child_mode);
        fputs(child->next != NULL ? ",\n" : "\n", out);
    }
    indent(out, level);
    fputs(object ? "}" : "]", out);
}

/*
 * The value's first line is already indented to `level`.
 *   MODE_LINES    one element per line (parts, pins)
 *   MODE_PINOUTS  each pinout on its own lines, one pin per line
 *   MODE_AUTO     one element per line when every element is a container
 */
static void write_value(FILE *out, const cJSON *value, int level, int mode) {
    int has_children = is_container(value) && value->child != NULL;

    if (mode == MODE_PINOUTS && has_children) {
        write_lines(out, value, level, MODE_LINES);
    } else if (mode == MODE_LINES && has_children) {
        write_lines(out, value, level, MODE_INLINE);
    } else if (mode == MODE_AUTO && spread(value)) {
        write_lines(out, value, level, MODE_AUTO);
    } else {
        write_inline(out, value);
    }
}

static void write_device(FILE *out, const cJSON *device) {
    const cJSON *key;

    indent(out, 2);
    fputs("{\n", out);
    for (key = device->child; key != NULL; key = key->next) {
        int mode = strcmp(key->string, "pinouts") == 0 ? MODE_PINOUTS
                 : strcmp(key->string, "parts") == 0 ? MODE_LINES
                 : MODE_AUTO;

        indent(out, 3);
        write_key(out, key->string);
        write_value(out, key, 3, mode);
        fputs(key->next != NULL ? ",\n" : "\n", out);
    }
    indent(out, 2);
    fputs("}", out);
}

int export_write(const char *path) {
    FILE *out = fopen(path, "w");
    const cJSON *device;

    if (out == NULL) {
        perror(path);
        return -1;
    }
    fputs("{\n", out);
    fprintf(out, "  \"format\": \"schematical-parts\",\n  \"version\": 1,\n  \"kind\": ");
    write_inline(out, cJSON_GetObjectItemCaseSensitive(file_root, "kind"));
    fputs(",\n  \"devices\": [\n", out);
    cJSON_ArrayForEach(device, file_devices) {
        write_device(out, device);
        fputs(device->next != NULL ? ",\n" : "\n", out);
    }
    fputs("  ]\n}\n", out);
    return fclose(out) == 0 ? 0 : -1;
}
