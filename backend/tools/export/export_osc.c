/*
 * oscillator.json from backend/osc.c: one device per (family, frequency
 * band), id "<family>-<band index>"; pinouts keyed by package label,
 * lower-cased with spaces -> '-'.
 */

#include <ctype.h>
#include <stdio.h>

#include "../../osc.c"

#include "export.h"

static void pinout_key(OscPackage package, char *out, size_t cap) {
    const char *label = osc_package_label(package);
    size_t i;

    for (i = 0; label[i] != '\0' && i + 1 < cap; i++) {
        out[i] = label[i] == ' ' ? '-' : (char)tolower((unsigned char)label[i]);
    }
    out[i] = '\0';
}

int export_osc(void) {
    int family;
    size_t band;

    for (family = 0; family < OSC_FAMILY_COUNT; family++) {
        for (band = 0; band < osc_group_count(); band++) {
            cJSON *device = NULL;
            cJSON *parts = NULL;
            size_t i;

            for (i = 0; i < osc_part_count(); i++) {
                OscConfig config;
                char key[32];
                const ChipPin *pins;

                osc_part_config_at(i, &config);
                if ((int)config.family != family || !osc_part_in_group(i, band)) {
                    continue;
                }
                pins = osc_pins(config.family, config.package);
                if (device == NULL) {
                    char id[32];
                    char description[96];
                    size_t outs[2];
                    size_t n = output_pins(config.family, config.package, outs);
                    cJSON *outputs;
                    size_t k;

                    snprintf(id, sizeof id, "%s-%zu", osc_family_label(config.family), band);
                    for (k = 0; id[k] != '\0'; k++) {
                        id[k] = (char)tolower((unsigned char)id[k]);
                    }
                    snprintf(description, sizeof description, "%s oscillator, %s",
                             osc_family_label(config.family), osc_group_label(band));
                    device = export_new_device(id, osc_group_label(band), description,
                                               "Oscillator", NULL, NULL);
                    cJSON_AddStringToObject(device, "family", osc_family_label(config.family));
                    cJSON_AddNumberToObject(device, "minHz", (double)osc_part_min_hz(i));
                    cJSON_AddNumberToObject(device, "maxHz", (double)osc_part_max_hz(i));
                    outputs = cJSON_AddArrayToObject(device, "outputs");
                    for (k = 0; k < n; k++) {
                        cJSON_AddItemToArray(outputs, cJSON_CreateString(pins[outs[k]].name));
                    }
                    parts = export_parts_array(device);
                }
                pinout_key(config.package, key, sizeof key);
                if (cJSON_GetObjectItemCaseSensitive(
                        cJSON_GetObjectItemCaseSensitive(device, "pinouts"), key) == NULL) {
                    export_add_pinout(device, key,
                                      export_pins(pins, osc_pin_count(config.family, config.package)));
                }
                {
                    cJSON *part = export_new_part(osc_part_name_at(i),
                                                  osc_package_label(config.package), key);

                    cJSON_AddNumberToObject(part, "defaultHz", (double)config.frequency_hz);
                    if (config.enable_pin >= 0) {
                        cJSON_AddNumberToObject(part, "enablePin", pins[config.enable_pin].number);
                    }
                    cJSON_AddItemToArray(parts, part);
                }
            }
            if (device != NULL) {
                export_device(device);
            }
        }
    }
    return 0;
}
