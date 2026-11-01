/* led.json from backend/led.c: one device per color, pinouts two-pin and dip4. */

#include <ctype.h>
#include <stdio.h>

#include "../../led.c"

#include "export.h"

int export_led(void) {
    int color;

    for (color = 0; color < LED_COLOR_COUNT; color++) {
        char id[32];
        char description[64];
        size_t i;
        cJSON *device;
        cJSON *parts;
        const char *label = led_color_label((LedColor)color);

        snprintf(id, sizeof id, "led-%s", label);
        for (i = 0; id[i] != '\0'; i++) {
            id[i] = (char)tolower((unsigned char)id[i]);
        }
        snprintf(description, sizeof description, "%s LED", label);
        device = export_new_device(id, "LED", description, "Indicators", "LED", NULL);
        export_add_pinout(device, "two-pin", export_pins(PINS_2, led_pin_count(LED_PACKAGE_0402)));
        export_add_pinout(device, "dip4", export_pins(PINS_4, led_pin_count(LED_PACKAGE_DIP)));
        cJSON_AddStringToObject(device, "color", label);
        cJSON_AddStringToObject(device, "anode", "A");
        cJSON_AddStringToObject(device, "cathode", "K");
        parts = export_parts_array(device);
        for (i = 0; i < led_part_count(); i++) {
            LedConfig config;

            led_part_config_at(i, &config);
            if ((int)config.color != color) {
                continue;
            }
            cJSON_AddItemToArray(parts, export_new_part(
                led_part_name_at(i), led_package_label(config.package),
                config.package == LED_PACKAGE_DIP ? "dip4" : "two-pin"));
        }
        export_device(device);
    }
    return 0;
}
