/* gate.json: the shared device builder for the gate exporters. */

#include "export_gate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Every gate definition, in the old catalog's order. */
int export_gate(void) {
    return export_gate_7400() || export_gate_7402() || export_gate_7408() ||
           export_gate_7432() || export_gate_7486() || export_gate_74266() ||
           export_gate_cd() || export_gate_gt() ? -1 : 0;
}

static int ends_with(const char *text, const char *suffix) {
    size_t a = strlen(text);
    size_t b = strlen(suffix);

    return a >= b && strcmp(text + a - b, suffix) == 0;
}

static cJSON *gates_json(const ChipDefinition *def, const ExportGate *gates, size_t count) {
    cJSON *array = cJSON_CreateArray();
    size_t g;
    size_t i;

    for (g = 0; g < count; g++) {
        cJSON *gate = cJSON_CreateObject();
        cJSON *inputs;

        cJSON_AddStringToObject(gate, "op", gates[g].op);
        inputs = cJSON_AddArrayToObject(gate, "inputs");
        for (i = 0; i < gates[g].input_count; i++) {
            cJSON_AddItemToArray(inputs, cJSON_CreateString(def->pins[gates[g].inputs[i]].name));
        }
        cJSON_AddStringToObject(gate, "output", def->pins[gates[g].output].name);
        cJSON_AddItemToArray(array, gate);
    }
    return array;
}

static cJSON *delays_json(unsigned long long rise, unsigned long long fall) {
    cJSON *object = cJSON_CreateObject();

    cJSON_AddItemToObject(object, "rise", export_ns(rise));
    cJSON_AddItemToObject(object, "fall", export_ns(fall));
    return object;
}

static cJSON *timing_json(const ExportTiming *timing, const char *note) {
    cJSON *entry;

    if (timing->split && (timing->low_rise != timing->high_rise ||
                          timing->low_fall != timing->high_fall)) {
        entry = cJSON_CreateObject();
        cJSON_AddItemToObject(entry, "otherLow", delays_json(timing->low_rise, timing->low_fall));
        cJSON_AddItemToObject(entry, "otherHigh", delays_json(timing->high_rise, timing->high_fall));
    } else if (timing->split) {
        entry = delays_json(timing->low_rise, timing->low_fall);
    } else {
        entry = delays_json(timing->rise, timing->fall);
    }
    if (note[0] != '\0') {
        cJSON_AddStringToObject(entry, "note", note);
    }
    return entry;
}

/* A timing object without its notes, for comparing two devices. */
static cJSON *without_notes(const cJSON *timing) {
    cJSON *copy = cJSON_Duplicate(timing, 1);
    cJSON *entry;

    cJSON_ArrayForEach(entry, copy) {
        cJSON_DeleteItemFromObjectCaseSensitive(entry, "note");
    }
    return copy;
}

/* Every family both timings define has the same entry (notes aside). */
static int timings_agree(const cJSON *a, const cJSON *b) {
    cJSON *ca = without_notes(a);
    cJSON *cb = without_notes(b);
    const cJSON *entry;
    int same = 1;

    cJSON_ArrayForEach(entry, ca) {
        const cJSON *other = cJSON_GetObjectItemCaseSensitive(cb, entry->string);

        if (other != NULL && !cJSON_Compare(entry, other, 1)) {
            same = 0;
        }
    }
    cJSON_Delete(ca);
    cJSON_Delete(cb);
    return same;
}

static const char *json_string(const cJSON *object, const char *key) {
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(object, key);

    return cJSON_IsString(item) ? item->valuestring : "";
}

int export_gate_definition(const ExportGateSource *source) {
    const ChipDefinition *def = source->def;
    int fk = ends_with(def->id, "_FK");
    char base_id[64];
    char pinout[16];
    char note[160];
    cJSON *gates = NULL;
    cJSON *timing = cJSON_CreateObject();
    cJSON *parts = cJSON_CreateArray();
    cJSON *device;
    cJSON *base;
    size_t gate_count = 0;
    size_t i;

    snprintf(base_id, sizeof base_id, "%.*s", (int)(strlen(def->id) - (fk ? 3 : 0)), def->id);
    if (fk) {
        snprintf(pinout, sizeof pinout, "lccc20");
    } else {
        snprintf(pinout, sizeof pinout, "p%zu", def->pin_count);
    }

    for (i = 0; i < def->part_number_count; i++) {
        ExportGate part_gates[EXPORT_GATE_MAX];
        ExportTiming part_timing;
        size_t count = 0;
        int family;
        int package;
        const char *family_name;
        cJSON *part;
        cJSON *this_gates;

        memset(&part_timing, 0, sizeof part_timing);
        if (source->model(def, def->part_numbers[i], &family, &package, part_gates, &count, &part_timing) != 0) {
            continue;  /* not simulatable; the catalog skips it too */
        }
        family_name = export_family_name(family);
        this_gates = gates_json(def, part_gates, count);
        if (gates == NULL) {
            gates = this_gates;
            gate_count = count;
        } else {
            if (!cJSON_Compare(gates, this_gates, 1)) {
                fprintf(stderr, "%s: parts disagree on the gate layout\n", def->id);
                return -1;
            }
            cJSON_Delete(this_gates);
        }
        if (source->note_file != NULL) {
            snprintf(note, sizeof note, "@%s/%s", source->note_file, family_name);
        } else if (source->device_note) {
            snprintf(note, sizeof note, "@def:%s/%s", def->id, family_name);
        } else {
            note[0] = '\0';   /* no comment source (cd4000.c) */
        }
        {
            cJSON *entry = timing_json(&part_timing, note);
            const cJSON *existing = cJSON_GetObjectItemCaseSensitive(timing, family_name);

            if (existing == NULL) {
                cJSON_AddItemToObject(timing, family_name, entry);
            } else {
                if (!cJSON_Compare(existing, entry, 1)) {
                    fprintf(stderr, "%s: family %s has two timings\n", def->id, family_name);
                    return -1;
                }
                cJSON_Delete(entry);
            }
        }
        part = export_new_part(def->part_numbers[i], export_package_name(package), pinout);
        cJSON_AddStringToObject(part, "family", family_name);
        export_part_flags(part, def->part_flags != NULL ? def->part_flags[i] : 0);
        cJSON_AddItemToArray(parts, part);
    }
    if (gates == NULL) {
        fprintf(stderr, "%s: no parts\n", def->id);
        return -1;
    }

    /* _FK: becomes an "lccc20" pinout of the base device when the model agrees. */
    base = fk ? export_find_device(base_id) : NULL;
    if (base != NULL &&
        cJSON_Compare(cJSON_GetObjectItemCaseSensitive(base, "gates"), gates, 1) &&
        (strcmp(json_string(base, "output"), "open-drain") == 0) == (source->open_drain != 0) &&
        cJSON_GetObjectItemCaseSensitive(base, "units") == NULL ==
            (def->gate_count == gate_count) &&
        timings_agree(cJSON_GetObjectItemCaseSensitive(base, "timing"), timing)) {
        cJSON *base_timing = cJSON_GetObjectItemCaseSensitive(base, "timing");
        cJSON *base_parts = cJSON_GetObjectItemCaseSensitive(base, "parts");
        cJSON *entry;
        cJSON *part;

        export_add_pinout(base, "lccc20", export_pins(def->pins, def->pin_count));
        while ((entry = timing->child) != NULL) {
            cJSON_DetachItemViaPointer(timing, entry);
            if (cJSON_GetObjectItemCaseSensitive(base_timing, entry->string) == NULL) {
                cJSON_AddItemToObject(base_timing, entry->string, entry);
            } else {
                cJSON_Delete(entry);
            }
        }
        while ((part = parts->child) != NULL) {
            cJSON_DetachItemViaPointer(parts, part);
            cJSON_AddItemToArray(base_parts, part);
        }
        if (source->device_note) {
            cJSON *base_note = cJSON_GetObjectItemCaseSensitive(base, "note");

            snprintf(note, sizeof note, "%s | lccc20: @def:%s/",
                     base_note != NULL ? base_note->valuestring : "", def->id);
            if (base_note != NULL) {
                cJSON_SetValuestring(base_note, note);
            }
        }
        cJSON_Delete(gates);
        cJSON_Delete(timing);
        cJSON_Delete(parts);
        return 0;
    }
    if (fk) {
        fprintf(stderr, "note: %s stays its own device (model differs from %s)\n", def->id,
                base_id);
    }

    if (source->device_note) {
        snprintf(note, sizeof note, "@def:%s/", def->id);
    }
    device = export_new_device(def->id, def->function, def->description, "Logic", NULL,
                               source->device_note ? note : NULL);
    export_add_pinout(device, pinout, export_pins(def->pins, def->pin_count));
    cJSON_AddItemToObject(device, "gates", gates);
    if (source->open_drain) {
        cJSON_AddStringToObject(device, "output", "open-drain");
    }
    if (def->gate_count != gate_count) {
        cJSON_AddNumberToObject(device, "units", (double)def->gate_count);
    }
    cJSON_AddItemToObject(device, "timing", timing);
    cJSON_AddItemToObject(device, "parts", parts);
    export_device(device);
    return 0;
}
