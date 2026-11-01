/*
 * old_listing: one line per orderable part of the old backend, in the
 * plan-01-A section 4.2 format, sorted by name:
 *   name|kind|package|drawer|function|family|flags|details|pins
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "led7.h"
#include "old_parts.h"
#include "sim.h"
#include "sram.h"
#include "switch.h"

/* Same names as catalog.c's package_name (static there). */
static const char *package_name(PackageType package) {
    switch (package) {
        case PACKAGE_TYPE_SOIC: return "SOIC";
        case PACKAGE_TYPE_SOP: return "SOP";
        case PACKAGE_TYPE_SSOP: return "SSOP";
        case PACKAGE_TYPE_DIP: return "DIP";
        case PACKAGE_TYPE_TSSOP: return "TSSOP";
        case PACKAGE_TYPE_TVSOP: return "TVSOP";
        case PACKAGE_TYPE_CDIP: return "CDIP";
        case PACKAGE_TYPE_CFP: return "CFP";
        case PACKAGE_TYPE_LCCC: return "LCCC";
        case PACKAGE_TYPE_WQFN: return "WQFN";
        case PACKAGE_TYPE_VQFN: return "VQFN";
        case PACKAGE_TYPE_VSSOP: return "VSSOP";
        case PACKAGE_TYPE_SOT23: return "SOT-23";
        case PACKAGE_TYPE_SC70: return "SC-70";
        case PACKAGE_TYPE_DSBGA: return "DSBGA";
        case PACKAGE_TYPE_SON: return "SON";
        case PACKAGE_TYPE_UQFN: return "UQFN";
        case PACKAGE_TYPE_X2SON: return "X2SON";
        case PACKAGE_TYPE_SOT5X3: return "SOT-5X3";
        case PACKAGE_TYPE_SOJ: return "SOJ";
        case PACKAGE_TYPE_TSOP_I: return "TSOP-I";
        case PACKAGE_TYPE_STSOP_I: return "sTSOP-I";
        case PACKAGE_TYPE_TSOP_II: return "TSOP-II";
        case PACKAGE_TYPE_TFBGA: return "TFBGA";
        case PACKAGE_TYPE_SMD: return "SMD";
    }
    return "unknown";
}

/* Real logic families only; the kind tags (SRAM, Switch, LED7) never reach here. */
static const char *family_name(LogicFamily family) {
    static const char *const NAMES[] = {
        "HC", "LS", "ACT", "HCT", "F", "AC", "AHCT", "ALS", "AS", "AHC", "S", "C",
        "H", "HCS", "LV", "LVC", "AUP", "AUC", "ALVC", "TTL"
    };

    if ((size_t)family < sizeof(NAMES) / sizeof(NAMES[0])) {
        return NAMES[family];
    }
    fprintf(stderr, "not a logic family: %d\n", (int)family);
    exit(2);
}

static const char *role_name(const ChipPin *pin) {
    if (pin->group != NULL && strcmp(pin->group, CHIP_PIN_GROUP_HIDDEN) == 0) {
        return "absent";
    }
    switch (pin->role) {
        case PIN_ROLE_INPUT: return "input";
        case PIN_ROLE_OUTPUT: return "output";
        case PIN_ROLE_INOUT: return "inout";
        case PIN_ROLE_POWER: return "power";
        case PIN_ROLE_GROUND: return "ground";
        case PIN_ROLE_NO_CONNECT: return "nc";
    }
    return "nc";
}

static const char *group_name(const ChipPin *pin) {
    if (pin->group == NULL || strcmp(pin->group, CHIP_PIN_GROUP_HIDDEN) == 0) {
        return CHIP_PIN_GROUP_DEFAULT;
    }
    return pin->group;
}

static void print_flags(unsigned flags) {
    static const char *const NAMES[] = {"obsolete", "military", "automotive", "enhanced"};
    int first = 1;
    int i;

    for (i = 0; i < 4; i++) {
        if (flags & (1u << i)) {
            printf("%s%s", first ? "" : ",", NAMES[i]);
            first = 0;
        }
    }
}

static void print_numbers(const unsigned *numbers, size_t count) {
    size_t i;

    for (i = 0; i < count; i++) {
        printf(i > 0 ? ",%u" : "%u", numbers[i]);
    }
}

static void print_display(const SimLogicConfig *config) {
    unsigned numbers[64];
    size_t i;

    printf("digits=%u commonAnode=%d colonAfter=%u dotAfter=%u color=%s size=%s chip=%s leds=",
           led7_digits(config), led7_common_anode(config) ? 1 : 0, led7_colon_after(config),
           led7_dot_after(config), led7_color_label(config), led7_size_label(config),
           led7_chip_label(config));
    for (i = 0; i < led7_led_count(config); i++) {
        unsigned digit;
        Led7Segment segment;

        led7_led_at(config, i, &digit, &segment);
        printf("%s%u:%s", i > 0 ? "," : "", digit == LED7_EXTRAS ? 0u : digit,
               led7_segment_name(segment));
    }
    printf(" segmentPins=");
    print_numbers(numbers, led7_side_pins(config, 0, numbers, 64));
    printf(" commonPins=");
    print_numbers(numbers, led7_side_pins(config, 1, numbers, 64));
}

/* SRAM words and bits come from a throwaway instance. */
static void print_sram(const OldPart *part) {
    Sim *sim = sim_create();
    SimInstance *instance = sim_add_instance(sim, part->type, part->name, &part->logic);
    uint32_t words = 0;
    unsigned bits = 0;

    if (instance == NULL || !sram_instance_size(instance, &words, &bits)) {
        fprintf(stderr, "%s: no SRAM size\n", part->name);
        exit(2);
    }
    printf("words=%u bits=%u access_ps=%llu", words, bits,
           (unsigned long long)sram_access_time(&part->logic));
    sim_destroy(sim);
}

static void print_chip(const OldPart *part) {
    const ChipDefinition *chip = part->chip;
    int logic = strcmp(chip->category, "Logic") == 0;

    printf("%s|%s|%s|%s%s%s|%s|%s|", part->name, part->kind, package_name(part->logic.package),
           chip->category, chip->subcategory != NULL ? "/" : "",
           chip->subcategory != NULL ? chip->subcategory : "", chip->function,
           logic ? family_name(part->logic.family) : "");
    print_flags(chip->part_flags != NULL ? chip->part_flags[part->part_index] : 0);
    putchar('|');
    if (logic) {
        printf("units=%zu", chip->gate_count);
    } else if (strcmp(part->kind, "sram") == 0) {
        print_sram(part);
    } else if (strcmp(part->kind, "switch") == 0) {
        printf("positions=%zu actuator=%s", switch_config_positions(&part->logic),
               switch_family_label(&part->logic));
    } else {
        print_display(&part->logic);
    }
}

static void print_part(const OldPart *part) {
    size_t i;

    switch (part->source) {
        case OLD_CHIP:
            print_chip(part);
            break;
        case OLD_LED:
            printf("%s|led|%s|Indicators/LED|LED|||color=%s", part->name,
                   led_package_label(part->led.package), led_color_label(part->led.color));
            break;
        case OLD_OSC: {
            size_t group;
            const char *band = NULL;

            for (group = 0; group < osc_group_count(); group++) {
                if (osc_part_in_group(part->part_index, group)) {
                    band = osc_group_label(group);
                }
            }
            if (band == NULL) {
                fprintf(stderr, "%s: no band\n", part->name);
                exit(2);
            }
            printf("%s|oscillator|%s|Oscillator|%s|||family=%s minHz=%llu maxHz=%llu defaultHz=%llu enablePin=",
                   part->name, osc_package_label(part->osc.package), band,
                   osc_family_label(part->osc.family),
                   (unsigned long long)osc_part_min_hz(part->part_index),
                   (unsigned long long)osc_part_max_hz(part->part_index),
                   (unsigned long long)part->osc.frequency_hz);
            if (part->osc.enable_pin >= 0) {
                printf("%u", part->pins[part->osc.enable_pin].number);
            } else {
                printf("none");
            }
            break;
        }
        case OLD_SUPPLY:
            printf("%s|supply|||%s|||level=%d", part->name, part->name,
                   part->supply.kind == SUPPLY_VCC ? 1 : 0);
            break;
    }

    putchar('|');
    for (i = 0; i < part->pin_count; i++) {
        const ChipPin *pin = &part->pins[i];

        printf("%s%u:%s:%s:%s", i > 0 ? "," : "", pin->number, pin->name, role_name(pin),
               group_name(pin));
    }
    putchar('\n');
}

int main(void) {
    OldPart *parts;
    size_t count = old_parts_collect(&parts);
    size_t i;

    for (i = 0; i < count; i++) {
        print_part(&parts[i]);
    }
    return 0;
}
