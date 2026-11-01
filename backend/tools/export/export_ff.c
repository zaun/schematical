/* dff.json / jkff.json: the shared device builder for the flip-flop exporters. */

#include "export_ff.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int ends_with(const char *text, const char *suffix) {
    size_t a = strlen(text);
    size_t b = strlen(suffix);

    return a >= b && strcmp(text + a - b, suffix) == 0;
}

static cJSON *without_notes(const cJSON *timing) {
    cJSON *copy = cJSON_Duplicate(timing, 1);
    cJSON *entry;

    cJSON_ArrayForEach(entry, copy) {
        cJSON_DeleteItemFromObjectCaseSensitive(entry, "note");
    }
    return copy;
}

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

/* Every kind key (all of `keys`) is on the base device with the same value. */
static int keys_agree(const cJSON *base, const cJSON *keys) {
    const cJSON *key;

    cJSON_ArrayForEach(key, keys) {
        const cJSON *other = cJSON_GetObjectItemCaseSensitive(base, key->string);

        if (other == NULL || !cJSON_Compare(key, other, 1)) {
            return 0;
        }
    }
    return 1;
}

const char *export_ff_pinout(const ChipDefinition *def, char *out, size_t cap) {
    if (ends_with(def->id, "_FK")) {
        snprintf(out, cap, "lccc20");
    } else {
        snprintf(out, cap, "p%zu", def->pin_count);
    }
    return out;
}

int export_ff_device(const ChipDefinition *def, cJSON *keys, cJSON *timing, cJSON *parts) {
    int fk = ends_with(def->id, "_FK");
    char base_id[64];
    char pinout[16];
    char note[160];
    cJSON *base;
    cJSON *device;
    cJSON *key;

    snprintf(base_id, sizeof base_id, "%.*s", (int)(strlen(def->id) - (fk ? 3 : 0)), def->id);
    export_ff_pinout(def, pinout, sizeof pinout);

    base = fk ? export_find_device(base_id) : NULL;
    if (base != NULL && keys_agree(base, keys) &&
        timings_agree(cJSON_GetObjectItemCaseSensitive(base, "timing"), timing)) {
        cJSON *base_timing = cJSON_GetObjectItemCaseSensitive(base, "timing");
        cJSON *base_parts = cJSON_GetObjectItemCaseSensitive(base, "parts");
        cJSON *base_note = cJSON_GetObjectItemCaseSensitive(base, "note");
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
        if (base_note != NULL) {
            snprintf(note, sizeof note, "%s | lccc20: @def:%s/", base_note->valuestring, def->id);
            cJSON_SetValuestring(base_note, note);
        }
        cJSON_Delete(keys);
        cJSON_Delete(timing);
        cJSON_Delete(parts);
        return 0;
    }
    if (fk) {
        fprintf(stderr, "note: %s stays its own device (model differs from %s)\n", def->id,
                base_id);
    }

    snprintf(note, sizeof note, "@def:%s/", def->id);
    device = export_new_device(def->id, def->function, def->description, "Logic", NULL, note);
    export_add_pinout(device, pinout, export_pins(def->pins, def->pin_count));
    while ((key = keys->child) != NULL) {
        cJSON_DetachItemViaPointer(keys, key);
        cJSON_AddItemToObject(device, key->string, key);
    }
    cJSON_Delete(keys);
    cJSON_AddItemToObject(device, "timing", timing);
    cJSON_AddItemToObject(device, "parts", parts);
    export_device(device);
    return 0;
}
