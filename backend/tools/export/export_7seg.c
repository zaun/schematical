/*
 * 7seg.json from backend/led7.c + led7_parts.inc (plan-01-E section 3.3):
 * one device per part, id = part name, function "7-Segment", drawer
 * Indicators / LED 7-Seg, pinout "p<pins>". The device note is the
 * comment line above the part's pin table (e.g. "LTC-4727JR: C4727JR.pdf"); its macro
 * name comes from a second pass over led7_parts.inc.
 */

#include <stdio.h>

#include "../../led7.c"

#include "export.h"

typedef struct {
    const char *name;
    const char *id;
} DisplayName;

static DisplayName display_names[128];
static size_t display_name_count;

static void collect_names(void) {
#define LED7_PART(ID, NAME, DESCRIPTION, DIGITS, SIZE, COLOR, CHIP, CA, COLON, DOT, FLAGS) \
    display_names[display_name_count].name = NAME; \
    display_names[display_name_count].id = #ID; \
    display_name_count++;
#include "../../led7_parts.inc"
#undef LED7_PART
}

static const char *id_of(const char *name) {
    size_t i;

    for (i = 0; i < display_name_count; i++) {
        if (strcmp(display_names[i].name, name) == 0) {
            return display_names[i].id;
        }
    }
    fprintf(stderr, "%s: no table name\n", name);
    exit(2);
}

static cJSON *segment_list(unsigned segments) {
    cJSON *array = cJSON_CreateArray();
    unsigned s;

    for (s = 0; s < LED7_SEG_COUNT; s++) {
        if (segments & (1u << s)) {
            cJSON_AddItemToArray(array, cJSON_CreateString(SEGMENT_NAMES[s]));
        }
    }
    return array;
}

static const char *name_of(const ChipDefinition *def, unsigned number) {
    return def->pins[number - 1].name;
}

/* Pin numbers -> names, a name shared by several pins once; checks the expansion. */
static cJSON *side_names(const ChipDefinition *def, const unsigned *numbers, size_t count) {
    cJSON *array = cJSON_CreateArray();
    size_t expanded = 0;
    size_t i;

    for (i = 0; i < count; i++) {
        const char *name = name_of(def, numbers[i]);
        size_t p;

        if (i > 0 && strcmp(name, name_of(def, numbers[i - 1])) == 0) {
            continue;
        }
        cJSON_AddItemToArray(array, cJSON_CreateString(name));
        /* The loader expands the name to every pin with it, in number order. */
        for (p = 0; p < def->pin_count; p++) {
            if (strcmp(def->pins[p].name, name) == 0) {
                if (expanded >= count || numbers[expanded] != def->pins[p].number) {
                    fprintf(stderr, "%s: side order of \"%s\" is not its pin order\n", def->id, name);
                    exit(2);
                }
                expanded++;
            }
        }
    }
    if (expanded != count) {
        fprintf(stderr, "%s: side expansion has %zu pins, not %zu\n", def->id, expanded, count);
        exit(2);
    }
    return array;
}

int export_7seg(void) {
    size_t d;

    collect_names();
    for (d = 0; d < led7_definition_count(); d++) {
        const ChipDefinition *def = led7_definition_at(d);
        SimLogicConfig config;
        const Led7Part *part;
        Led7Led leds[LED7_MAX_LEDS];
        size_t led_count;
        unsigned numbers[64];
        size_t count;
        char pinout[16];
        char note[96];
        cJSON *device;
        cJSON *array;
        cJSON *object;
        cJSON *item;
        size_t i;

        if (def->lookup(def->part_numbers[0], &config) == NULL) {
            fprintf(stderr, "%s: no model\n", def->id);
            return -1;
        }
        part = config.model_data;
        snprintf(pinout, sizeof pinout, "p%zu", def->pin_count);
        snprintf(note, sizeof note, "@%s_PINS/", id_of(part->name));
        device = export_new_device(part->name, def->function, def->description, def->category,
                                   def->subcategory, note);
        export_add_pinout(device, pinout, export_pins(def->pins, def->pin_count));
        cJSON_AddNumberToObject(device, "digits", part->digits);
        cJSON_AddBoolToObject(device, "commonAnode", part->common_anode);
        cJSON_AddNumberToObject(device, "colonAfter", part->colon_after);
        cJSON_AddNumberToObject(device, "dotAfter", part->dot_after);
        cJSON_AddStringToObject(device, "color", part->color);
        cJSON_AddStringToObject(device, "size", part->size);
        cJSON_AddStringToObject(device, "chip", part->chip);

        array = cJSON_AddArrayToObject(device, "leds");
        led_count = build_leds(part, leds);
        for (i = 0; i < led_count; i++) {
            item = cJSON_CreateObject();
            cJSON_AddNumberToObject(item, "digit", leds[i].digit == LED7_EXTRAS ? 0 : leds[i].digit);
            cJSON_AddStringToObject(item, "segment", SEGMENT_NAMES[leds[i].segment]);
            cJSON_AddItemToArray(array, item);
        }

        object = cJSON_AddObjectToObject(device, "segmentPins");
        for (i = 0; i < part->pin_count; i++) {
            const Led7Pin *pin = &part->pins[i];
            const char *name = name_of(def, pin->number);
            cJSON *entry;

            if (pin->kind != LED7_KIND_SEG) {
                continue;
            }
            entry = cJSON_CreateObject();
            cJSON_AddNumberToObject(entry, "digit", pin->digit);
            cJSON_AddItemToObject(entry, "segments", segment_list(pin->segments));
            item = cJSON_GetObjectItemCaseSensitive(object, name);
            if (item != NULL) {
                if (!cJSON_Compare(item, entry, 1)) {
                    fprintf(stderr, "%s: segment pins named \"%s\" differ\n", def->id, name);
                    return -1;
                }
                cJSON_Delete(entry);
            } else {
                cJSON_AddItemToObject(object, name, entry);
            }
        }

        object = cJSON_AddObjectToObject(device, "commonPins");
        for (i = 0; i < part->pin_count; i++) {
            const Led7Pin *pin = &part->pins[i];
            const char *name = name_of(def, pin->number);
            cJSON *entry;

            if (pin->kind != LED7_KIND_COM) {
                continue;
            }
            entry = pin->digit == LED7_EXTRAS ? cJSON_CreateString("extras")
                                               : cJSON_CreateNumber(pin->digit);
            item = cJSON_GetObjectItemCaseSensitive(object, name);
            if (item != NULL) {
                if (!cJSON_Compare(item, entry, 1)) {
                    fprintf(stderr, "%s: common pins named \"%s\" differ\n", def->id, name);
                    return -1;
                }
                cJSON_Delete(entry);
            } else {
                cJSON_AddItemToObject(object, name, entry);
            }
        }

        count = led7_side_pins(&config, 0, numbers, 64);
        cJSON_AddItemToObject(device, "segmentSide", side_names(def, numbers, count));
        count = led7_side_pins(&config, 1, numbers, 64);
        cJSON_AddItemToObject(device, "commonSide", side_names(def, numbers, count));

        item = export_new_part(part->name, export_package_name((int)config.package), pinout);
        export_part_flags(item, def->part_flags != NULL ? def->part_flags[0] : 0);
        cJSON_AddItemToArray(export_parts_array(device), item);
        export_device(device);
    }
    return 0;
}
