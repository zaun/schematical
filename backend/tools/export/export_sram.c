/*
 * sram.json from backend/sram.c + sram_parts.inc (plan-01-E section 3.3).
 *
 * Device id = chip id without its layout suffix (_STSOP, _TSOP, _TSOP2,
 * _BGA). A layout definition merges into its chip's device as another
 * pinout when every kind key (pins by name, grades) is equal; otherwise it
 * stays its own device with the old id. Pinout key = the layout suffix
 * lower-cased, or "p<pins>" for the base layout. Grade = the timing table's
 * name after the last '_'.
 *
 * The compiled tables do not carry the timing tables' names, so the part
 * lists are walked a second time: sram_parts.inc is included again inside a
 * function, with SRAM_DEFINE_CHIP redefined to record, per part, its grade
 * table name and the chip's PINS macro name (for the pinout note).
 */

#include <ctype.h>
#include <stdio.h>

#include "../../sram.c"

#include "export.h"

/* ---- Names from a second pass over sram_parts.inc ------------------------- */

typedef struct {
    const char *part;
    const char *table;   /* e.g. "AS6C6264_55" */
} GradeName;

typedef struct {
    const char *id;
    const char *pins;    /* the PINS macro, e.g. "AS6C6264_PINS" */
} PinsName;

static GradeName grade_names[512];
static size_t grade_name_count;
static PinsName pins_names[64];
static size_t pins_name_count;

static void add_grades(const char *id, const char *pins, const GradeName *names, size_t count) {
    size_t i;

    for (i = 0; i < count; i++) {
        grade_names[grade_name_count++] = names[i];
    }
    pins_names[pins_name_count].id = id;
    pins_names[pins_name_count].pins = pins;
    pins_name_count++;
}

static void collect_names(void) {
#undef SRAM_DEFINE_CHIP
#define GRADE_ENTRY(name, grade, package, flags) {name, #grade},
#define SRAM_DEFINE_CHIP(NAME, ID, SIZE, DESCRIPTION, ADDRESS_BITS, DATA_BITS, PINS, PARTS) \
    { \
        static const GradeName NAME##_GRADES[] = { PARTS(GRADE_ENTRY) }; \
        add_grades(ID, #PINS, NAME##_GRADES, sizeof NAME##_GRADES / sizeof NAME##_GRADES[0]); \
    }
#include "../../sram_parts.inc"
#undef SRAM_DEFINE_CHIP
#undef GRADE_ENTRY
}

static const char *table_of(const char *part) {
    size_t i;

    for (i = 0; i < grade_name_count; i++) {
        if (strcmp(grade_names[i].part, part) == 0) {
            return grade_names[i].table;
        }
    }
    fprintf(stderr, "%s: no grade table name\n", part);
    exit(2);
}

static const char *pins_of(const char *id) {
    size_t i;

    for (i = 0; i < pins_name_count; i++) {
        if (strcmp(pins_names[i].id, id) == 0) {
            return pins_names[i].pins;
        }
    }
    fprintf(stderr, "%s: no pins macro name\n", id);
    exit(2);
}

/* ---- Devices ------------------------------------------------------------ */

static const char *const SUFFIXES[] = {"_STSOP", "_TSOP2", "_TSOP", "_BGA"};

/* Splits "AS6C6264_STSOP" into chip "AS6C6264" and layout "stsop" ("" for the base). */
static void split_id(const char *id, char *chip, size_t chip_cap, char *layout, size_t layout_cap) {
    size_t length = strlen(id);
    size_t s;

    for (s = 0; s < sizeof SUFFIXES / sizeof SUFFIXES[0]; s++) {
        size_t n = strlen(SUFFIXES[s]);

        if (length > n && strcmp(id + length - n, SUFFIXES[s]) == 0) {
            size_t i;

            snprintf(chip, chip_cap, "%.*s", (int)(length - n), id);
            for (i = 1; i < n && i < layout_cap; i++) {
                layout[i - 1] = (char)tolower((unsigned char)SUFFIXES[s][i]);
            }
            layout[i - 1] = '\0';
            return;
        }
    }
    snprintf(chip, chip_cap, "%s", id);
    layout[0] = '\0';
}

/* The pin name for a table entry, from the definition's ChipPin table. */
static const char *pin_name(const ChipDefinition *def, unsigned number) {
    size_t i;

    for (i = 0; i < def->pin_count; i++) {
        if (def->pins[i].number == number) {
            return def->pins[i].name;
        }
    }
    fprintf(stderr, "%s: no pin %u\n", def->id, number);
    exit(2);
}

/* address[i] / data[i] / byteEnable[i]: the pin with that function and index. */
static cJSON *indexed(const ChipDefinition *def, const SramLayout *layout, SramFunction function,
                      unsigned count) {
    cJSON *array = cJSON_CreateArray();
    unsigned index;
    size_t i;

    for (index = 0; index < count; index++) {
        const char *name = NULL;

        for (i = 0; i < layout->pin_count; i++) {
            if (layout->pins[i].function == function && layout->pins[i].index == index) {
                if (name != NULL) {
                    fprintf(stderr, "%s: two pins for function %d index %u\n", def->id, function, index);
                    exit(2);
                }
                name = pin_name(def, layout->pins[i].number);
            }
        }
        if (name == NULL) {
            fprintf(stderr, "%s: no pin for function %d index %u\n", def->id, function, index);
            exit(2);
        }
        cJSON_AddItemToArray(array, cJSON_CreateString(name));
    }
    return array;
}

/* Every pin with a function, in pin-table order (CE#, CE2: the old model's order). */
static cJSON *ordered(const ChipDefinition *def, const SramLayout *layout, SramFunction function) {
    cJSON *array = cJSON_CreateArray();
    size_t i;

    for (i = 0; i < layout->pin_count; i++) {
        if (layout->pins[i].function == function) {
            cJSON_AddItemToArray(array, cJSON_CreateString(pin_name(def, layout->pins[i].number)));
        }
    }
    return array;
}

static const char *one(const ChipDefinition *def, const SramLayout *layout, SramFunction function) {
    const char *name = NULL;
    size_t i;

    for (i = 0; i < layout->pin_count; i++) {
        if (layout->pins[i].function == function) {
            if (name != NULL) {
                fprintf(stderr, "%s: two pins for function %d\n", def->id, function);
                exit(2);
            }
            name = pin_name(def, layout->pins[i].number);
        }
    }
    if (name == NULL) {
        fprintf(stderr, "%s: no pin for function %d\n", def->id, function);
        exit(2);
    }
    return name;
}

static unsigned lanes(const SramLayout *layout) {
    unsigned count = 0;
    size_t i;

    for (i = 0; i < layout->pin_count; i++) {
        if (layout->pins[i].function == SRAM_FN_BE_N && layout->pins[i].index + 1 > count) {
            count = layout->pins[i].index + 1;
        }
    }
    return count;
}

static cJSON *grade_json(const SramTiming *t, const char *table) {
    const SimTime values[] = {
        t->trc, t->taa, t->tace, t->toe, t->tba, t->toh, t->tclz, t->tolz, t->tblz, t->tchz,
        t->tohz, t->tbhz, t->twc, t->taw, t->tcw, t->tbw, t->tas, t->twp, t->twp_oe_low, t->twr,
        t->tdw, t->tdh, t->twhz, t->tow
    };
    static const char *const NAMES[] = {
        "trc", "taa", "tace", "toe", "tba", "toh", "tclz", "tolz", "tblz", "tchz", "tohz", "tbhz",
        "twc", "taw", "tcw", "tbw", "tas", "twp", "twpOeLow", "twr", "tdw", "tdh", "twhz", "tow"
    };
    cJSON *grade = cJSON_CreateObject();
    char note[96];
    size_t i;

    for (i = 0; i < sizeof values / sizeof values[0]; i++) {
        if (values[i] != 0) {
            cJSON_AddItemToObject(grade, NAMES[i], export_ns(values[i]));
        }
    }
    snprintf(note, sizeof note, "@%s/", table);
    cJSON_AddStringToObject(grade, "note", note);
    return grade;
}

static cJSON *without_notes(const cJSON *grades) {
    cJSON *copy = cJSON_Duplicate(grades, 1);
    cJSON *entry;

    cJSON_ArrayForEach(entry, copy) {
        cJSON_DeleteItemFromObjectCaseSensitive(entry, "note");
    }
    return copy;
}

static int same_keys(const cJSON *base, const cJSON *keys) {
    const cJSON *key;

    cJSON_ArrayForEach(key, keys) {
        const cJSON *other = cJSON_GetObjectItemCaseSensitive(base, key->string);
        int same;

        if (other == NULL) {
            return 0;
        }
        if (strcmp(key->string, "grades") == 0) {
            cJSON *a = without_notes(key);
            cJSON *b = without_notes(other);

            same = cJSON_Compare(a, b, 1);
            cJSON_Delete(a);
            cJSON_Delete(b);
        } else {
            same = cJSON_Compare(key, other, 1);
        }
        if (!same) {
            return 0;
        }
    }
    return 1;
}

static int export_definition(const ChipDefinition *def) {
    char chip[64];
    char layout_name[16];
    char pinout[16];
    char note[256];
    cJSON *keys = cJSON_CreateObject();
    cJSON *grades = NULL;
    cJSON *parts = cJSON_CreateArray();
    cJSON *base;
    cJSON *key;
    const SramLayout *layout = NULL;
    size_t i;

    split_id(def->id, chip, sizeof chip, layout_name, sizeof layout_name);
    if (layout_name[0] != '\0') {
        snprintf(pinout, sizeof pinout, "%s", layout_name);
    } else {
        snprintf(pinout, sizeof pinout, "p%zu", def->pin_count);
    }

    for (i = 0; i < def->part_number_count; i++) {
        SimLogicConfig config;
        const SramDefinitionData *data;
        const SramPart *sram_part;
        const char *table;
        const char *grade_name;
        cJSON *part;

        if (def->lookup(def->part_numbers[i], &config) == NULL) {
            continue;
        }
        data = config.model_data;
        sram_part = &data->parts[config.variant];
        if (layout == NULL) {
            layout = data->layout;
            grades = cJSON_CreateObject();
        }
        table = table_of(def->part_numbers[i]);
        grade_name = strrchr(table, '_') + 1;
        if (cJSON_GetObjectItemCaseSensitive(grades, grade_name) == NULL) {
            cJSON_AddItemToObject(grades, grade_name, grade_json(sram_part->timing, table));
        }
        part = export_new_part(def->part_numbers[i], export_package_name((int)config.package),
                               pinout);
        cJSON_AddStringToObject(part, "grade", grade_name);
        export_part_flags(part, def->part_flags != NULL ? def->part_flags[i] : 0);
        cJSON_AddItemToArray(parts, part);
    }
    if (layout == NULL) {
        fprintf(stderr, "%s: no parts\n", def->id);
        return -1;
    }

    cJSON_AddItemToObject(keys, "address", indexed(def, layout, SRAM_FN_A, layout->address_bits));
    cJSON_AddItemToObject(keys, "data", indexed(def, layout, SRAM_FN_DQ, layout->data_bits));
    cJSON_AddItemToObject(keys, "chipEnable", ordered(def, layout, SRAM_FN_CE_N));
    cJSON_AddItemToObject(keys, "chipEnableHigh", ordered(def, layout, SRAM_FN_CE));
    cJSON_AddStringToObject(keys, "outputEnable", one(def, layout, SRAM_FN_OE_N));
    cJSON_AddStringToObject(keys, "writeEnable", one(def, layout, SRAM_FN_WE_N));
    cJSON_AddItemToObject(keys, "byteEnable", indexed(def, layout, SRAM_FN_BE_N, lanes(layout)));
    cJSON_AddItemToObject(keys, "grades", grades);

    base = export_find_device(chip);
    if (base != NULL && same_keys(base, keys) &&
        strcmp(cJSON_GetObjectItemCaseSensitive(base, "function")->valuestring, def->function) == 0) {
        cJSON *base_parts = cJSON_GetObjectItemCaseSensitive(base, "parts");
        cJSON *base_note = cJSON_GetObjectItemCaseSensitive(base, "note");
        cJSON *part;

        export_add_pinout(base, pinout, export_pins(def->pins, def->pin_count));
        while ((part = parts->child) != NULL) {
            cJSON_DetachItemViaPointer(parts, part);
            cJSON_AddItemToArray(base_parts, part);
        }
        snprintf(note, sizeof note, "%s | %s: @%s/", base_note->valuestring, pinout,
                 pins_of(def->id));
        cJSON_SetValuestring(base_note, note);
        cJSON_Delete(keys);
        cJSON_Delete(parts);
        return 0;
    }
    if (base != NULL) {
        fprintf(stderr, "note: %s stays its own device (differs from %s)\n", def->id, chip);
    }

    snprintf(note, sizeof note, "@%s/ | %s: @%s/", chip, pinout, pins_of(def->id));
    base = export_new_device(base != NULL ? def->id : chip, def->function, def->description,
                             def->category, def->subcategory, note);
    export_add_pinout(base, pinout, export_pins(def->pins, def->pin_count));
    while ((key = keys->child) != NULL) {
        cJSON_DetachItemViaPointer(keys, key);
        cJSON_AddItemToObject(base, key->string, key);
    }
    cJSON_Delete(keys);
    cJSON_AddItemToObject(base, "parts", parts);
    export_device(base);
    return 0;
}

int export_sram(void) {
    static const ChipDefinition *const DEFS[] = {
#define SRAM_CHIP(NAME) &SRAM_##NAME##_DEFINITION,
#include "../../sram_catalog.inc"
#undef SRAM_CHIP
    };
    size_t i;

    collect_names();
    for (i = 0; i < sizeof DEFS / sizeof DEFS[0]; i++) {
        if (export_definition(DEFS[i]) != 0) {
            return -1;
        }
    }
    return 0;
}
