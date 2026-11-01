/*
 * Template for the flip-flop exporters. Before including, include the old
 * model (ff-d.c or ff-jk.c) and define:
 *   FF_JK       1 for ff-jk.c
 *   FF_FN       the exporter function name
 *   FF_DEFS     the definitions, as an initializer list of pointers
 */

#include "export_ff.h"

static void add_pin(cJSON *unit, const ChipDefinition *def, const char *key, size_t pin) {
    if (pin != FF_NO_PIN) {
        cJSON_AddStringToObject(unit, key, def->pins[pin].name);
    }
}

/* The kind keys of a layout, in the order of plan-01-D sections 3.1/3.2. */
static cJSON *layout_keys(const ChipDefinition *def, const FfLayout *layout) {
    cJSON *keys = cJSON_CreateObject();
    cJSON *units = cJSON_AddArrayToObject(keys, "units");
    size_t i;

    for (i = 0; i < layout->unit_count; i++) {
        const FfUnit *u = &layout->units[i];
        cJSON *unit = cJSON_CreateObject();

#if FF_JK
        add_pin(unit, def, "j", u->j);
        add_pin(unit, def, "k", u->k);
#else
        add_pin(unit, def, "d", u->d);
#endif
        add_pin(unit, def, "clk", u->clk);
        add_pin(unit, def, "pre", u->pre);
        add_pin(unit, def, "clr", u->clr);
        add_pin(unit, def, "q", u->q);
        add_pin(unit, def, "qn", u->qn);
        cJSON_AddItemToArray(units, unit);
    }
#if FF_JK
    cJSON_AddBoolToObject(keys, "kInverted", layout->k_inverted);
#else
    if (layout->oe != FF_NO_PIN) {
        cJSON_AddStringToObject(keys, "outputEnable", def->pins[layout->oe].name);
    }
#endif
    cJSON_AddBoolToObject(keys, "presetActiveHigh", layout->pre_active_high);
    cJSON_AddBoolToObject(keys, "clearActiveHigh", layout->clr_active_high);
    cJSON_AddStringToObject(keys, "clockEdge", layout->negative_edge ? "falling" : "rising");
    return keys;
}

static int export_definition(const ChipDefinition *def) {
    cJSON *keys = NULL;
    cJSON *timing = cJSON_CreateObject();
    cJSON *parts = cJSON_CreateArray();
    char pinout[16];
    size_t i;

    export_ff_pinout(def, pinout, sizeof pinout);
    for (i = 0; i < def->part_number_count; i++) {
        SimLogicConfig config;
        const SimComponentType *type = def->lookup(def->part_numbers[i], &config);
        const char *family;
        FfState *state;
        cJSON *entry;
        cJSON *part;
        char note[96];

        if (type == NULL) {
            continue;
        }
        state = type->create(&config);
        family = export_family_name((int)config.family);
        if (keys == NULL) {
            keys = layout_keys(def, state->layout);
            if (state->layout->unit_count != def->gate_count) {
                fprintf(stderr, "%s: %zu units but gate_count %zu\n", def->id,
                        state->layout->unit_count, def->gate_count);
                return -1;
            }
        }
        entry = cJSON_CreateObject();
        cJSON_AddItemToObject(entry, "rise", export_ns(state->rise_delay));
        cJSON_AddItemToObject(entry, "fall", export_ns(state->fall_delay));
#if !FF_JK
        cJSON_AddBoolToObject(entry, "clearWins", state->clr_wins);
#endif
        snprintf(note, sizeof note, "@def:%s/%s", def->id, family);
        cJSON_AddStringToObject(entry, "note", note);
        if (cJSON_GetObjectItemCaseSensitive(timing, family) == NULL) {
            cJSON_AddItemToObject(timing, family, entry);
        } else {
            if (!cJSON_Compare(cJSON_GetObjectItemCaseSensitive(timing, family), entry, 1)) {
                fprintf(stderr, "%s: family %s has two timings\n", def->id, family);
                return -1;
            }
            cJSON_Delete(entry);
        }
        part = export_new_part(def->part_numbers[i], export_package_name((int)config.package),
                               pinout);
        cJSON_AddStringToObject(part, "family", family);
        export_part_flags(part, def->part_flags != NULL ? def->part_flags[i] : 0);
        cJSON_AddItemToArray(parts, part);
        type->destroy(state);
    }
    if (keys == NULL) {
        fprintf(stderr, "%s: no parts\n", def->id);
        return -1;
    }
    return export_ff_device(def, keys, timing, parts);
}

int FF_FN(void) {
    static const ChipDefinition *const DEFS[] = { FF_DEFS };
    size_t i;

    for (i = 0; i < sizeof DEFS / sizeof DEFS[0]; i++) {
        if (export_definition(DEFS[i]) != 0) {
            return -1;
        }
    }
    return 0;
}
