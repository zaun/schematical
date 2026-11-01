/*
 * switch.json from backend/switch.c: one device per old definition (series
 * and position count), id = old id, function = series, drawer Switch/DIP,
 * pinout "p<pins>". Position k joins pins k and 2N + 1 - k.
 */

#include <stdio.h>

#include "../../switch.c"

#include "export.h"

int export_switch(void) {
    size_t d;

    for (d = 0; d < switch_definition_count(); d++) {
        const ChipDefinition *def = switch_definition_at(d);
        cJSON *device = NULL;
        cJSON *parts = NULL;
        char pinout[16];
        size_t i;

        snprintf(pinout, sizeof pinout, "p%zu", def->pin_count);
        for (i = 0; i < def->part_number_count; i++) {
            SimLogicConfig config;
            cJSON *part;

            if (def->lookup(def->part_numbers[i], &config) == NULL) {
                continue;
            }
            if (device == NULL) {
                size_t positions = switch_config_positions(&config);
                cJSON *contacts;
                size_t k;

                device = export_new_device(def->id, def->function, def->description, def->category,
                                           def->subcategory, NULL);
                export_add_pinout(device, pinout, export_pins(def->pins, def->pin_count));
                cJSON_AddStringToObject(device, "actuator", switch_family_label(&config));
                contacts = cJSON_AddArrayToObject(device, "contacts");
                for (k = 1; k <= positions; k++) {
                    cJSON *contact = cJSON_CreateArray();

                    cJSON_AddItemToArray(contact, cJSON_CreateString(def->pins[k - 1].name));
                    cJSON_AddItemToArray(contact,
                                         cJSON_CreateString(def->pins[2 * positions - k].name));
                    cJSON_AddItemToArray(contacts, contact);
                }
                parts = export_parts_array(device);
            }
            part = export_new_part(def->part_numbers[i], export_package_name((int)config.package),
                                   pinout);
            export_part_flags(part, def->part_flags != NULL ? def->part_flags[i] : 0);
            cJSON_AddItemToArray(parts, part);
        }
        if (device == NULL) {
            fprintf(stderr, "%s: no parts\n", def->id);
            return -1;
        }
        export_device(device);
    }
    return 0;
}
