/* supply.json from backend/supply.c. */

#include "../../supply.c"

#include "export.h"

int export_supply(void) {
    size_t i;

    for (i = 0; i < supply_part_count(); i++) {
        SupplyConfig config;
        const char *name = supply_part_name_at(i);
        int level;
        cJSON *device;

        supply_part_config_at(i, &config);
        level = config.kind == SUPPLY_VCC ? 1 : 0;
        device = export_new_device(name, name,
                                   level ? "Supply, drives logic 1" : "Ground, drives logic 0",
                                   NULL, NULL, NULL);
        export_add_pinout(device, "pin", export_pins(supply_pins(config.kind), 1));
        cJSON_AddNumberToObject(device, "level", level);
        cJSON_AddItemToArray(export_parts_array(device), export_new_part(name, "", "pin"));
        export_device(device);
    }
    return 0;
}
