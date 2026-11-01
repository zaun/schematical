#include "old_parts.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "catalog.h"

static OldPart *items;
static size_t count;
static size_t capacity;

static OldPart *add(void) {
    if (count == capacity) {
        capacity = capacity == 0 ? 4096 : capacity * 2;
        items = realloc(items, capacity * sizeof(*items));
        if (items == NULL) {
            fprintf(stderr, "out of memory\n");
            exit(2);
        }
    }
    memset(&items[count], 0, sizeof(items[count]));
    return &items[count++];
}

static const char *chip_kind(const ChipDefinition *chip) {
    if (strcmp(chip->category, "Logic") == 0) {
        if (strcmp(chip->function, "D flip-flop") == 0) {
            return "dff";
        }
        if (strcmp(chip->function, "JK flip-flop") == 0) {
            return "jkff";
        }
        return "gate";
    }
    if (strcmp(chip->category, "Memory") == 0) {
        return "sram";
    }
    if (strcmp(chip->category, "Switch") == 0) {
        return "switch";
    }
    if (strcmp(chip->category, "Indicators") == 0) {
        return "7seg";
    }
    fprintf(stderr, "unknown category %s\n", chip->category);
    exit(2);
}

static int by_name(const void *a, const void *b) {
    return strcmp(((const OldPart *)a)->name, ((const OldPart *)b)->name);
}

size_t old_parts_collect(OldPart **parts) {
    size_t d;
    size_t i;

    for (d = 0; d < catalog_definition_count(); d++) {
        const ChipDefinition *chip = catalog_definition_at(d);

        for (i = 0; i < chip->part_number_count; i++) {
            SimLogicConfig config;
            const SimComponentType *type = chip->lookup(chip->part_numbers[i], &config);
            OldPart *part;

            if (type == NULL) {
                continue;  /* the catalog skips these too */
            }
            part = add();
            part->name = chip->part_numbers[i];
            part->kind = chip_kind(chip);
            part->source = OLD_CHIP;
            part->chip = chip;
            part->part_index = i;
            part->type = type;
            part->logic = config;
            part->pins = chip->pins;
            part->pin_count = chip->pin_count;
        }
    }

    for (i = 0; i < led_part_count(); i++) {
        OldPart *part = add();

        part->name = led_part_name_at(i);
        part->kind = "led";
        part->source = OLD_LED;
        part->part_index = i;
        part->type = led_lookup(part->name, &part->led);
        part->pins = led_pins(part->led.package);
        part->pin_count = led_pin_count(part->led.package);
    }

    for (i = 0; i < osc_part_count(); i++) {
        OldPart *part = add();

        part->name = osc_part_name_at(i);
        part->kind = "oscillator";
        part->source = OLD_OSC;
        part->part_index = i;
        part->type = osc_lookup(part->name, &part->osc);
        part->pins = osc_pins(part->osc.family, part->osc.package);
        part->pin_count = osc_pin_count(part->osc.family, part->osc.package);
    }

    for (i = 0; i < supply_part_count(); i++) {
        OldPart *part = add();

        part->name = supply_part_name_at(i);
        part->kind = "supply";
        part->source = OLD_SUPPLY;
        part->part_index = i;
        part->type = supply_lookup(part->name, &part->supply);
        part->pins = supply_pins(part->supply.kind);
        part->pin_count = 1;
    }

    for (i = 0; i < count; i++) {
        if (items[i].type == NULL) {
            fprintf(stderr, "no model for %s\n", items[i].name);
            exit(2);
        }
    }

    qsort(items, count, sizeof(*items), by_name);
    *parts = items;
    return count;
}

const void *old_part_config(const OldPart *part) {
    switch (part->source) {
        case OLD_CHIP: return &part->logic;
        case OLD_LED: return &part->led;
        case OLD_OSC: return &part->osc;
        case OLD_SUPPLY: return &part->supply;
    }
    return NULL;
}
