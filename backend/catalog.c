#include "catalog.h"

#include <stdbool.h>
#include <stdio.h>

#include "74_00.h"
#include "74_02.h"
#include "74_08.h"
#include "74_32.h"
#include "74_86.h"
#include "74_266.h"
#include "ff-d.h"
#include "ff-jk.h"
#include "cd4000.h"
#include "gates.h"
#include "led.h"
#include "osc.h"
#include "sram.h"
#include "supply.h"
#include "switch.h"

static const ChipDefinition *const DEFINITIONS[] = {
    &SN74_00_DEFINITION,
    &SN74_00_FK_DEFINITION,
    &SN74_02_DEFINITION,
    &SN74_02_FK_DEFINITION,
    &SN74_08_DEFINITION,
    &SN74_08_FK_DEFINITION,
    &SN74_32_DEFINITION,
    &SN74_32_FK_DEFINITION,
    &SN74_86_DEFINITION,
    &SN74_86_FK_DEFINITION,
    &SN74_266_DEFINITION,
    &FF_74_72_DEFINITION,
    &FF_74_74_DEFINITION,
    &FF_74_174_DEFINITION,
    &FF_74_175_DEFINITION,
    &FF_74_273_DEFINITION,
    &FF_74_374_DEFINITION,
    &FF_74_574_DEFINITION,
    &FF_4013B_DEFINITION,
    &FF_40175B_DEFINITION,
    &FF_1G74_DEFINITION,
    &FF_1G79_DEFINITION,
    &FF_1G175_DEFINITION,
    &FF_1G374_DEFINITION,
    &FF_2G79_DEFINITION,
    &FF_74_74_FK_DEFINITION,
    &FF_74_174_FK_DEFINITION,
    &FF_74_175_FK_DEFINITION,
    &FF_1G79_SON_DEFINITION,
    &FF_1G80_DEFINITION,
    &FF_1G80_SON_DEFINITION,
    &FF_2G80_DEFINITION,
    &FF_2G80_RSE_DEFINITION,
    &FF_2G74_DEFINITION,
    &FF_74_73_DEFINITION,
    &FF_74_76_DEFINITION,
    &FF_74_107_DEFINITION,
    &FF_74_109_DEFINITION,
    &FF_74_112_DEFINITION,
    &FF_4027B_DEFINITION,
    &FF_74_109_FK_DEFINITION,
    &FF_74_112_FK_DEFINITION,
    &CD4001B_DEFINITION,
    &CD4011B_DEFINITION,
    &CD4030B_DEFINITION,
    &CD4070B_DEFINITION,
    &CD4071B_DEFINITION,
    &CD4077B_DEFINITION,
    &CD4081B_DEFINITION,
    &CD4093B_DEFINITION,
#include "gates_catalog.inc"
#define SRAM_CHIP(NAME) &SRAM_##NAME##_DEFINITION,
#include "sram_catalog.inc"
#undef SRAM_CHIP
};

#define CHIP_DEFINITION_COUNT (sizeof(DEFINITIONS) / sizeof(DEFINITIONS[0]))

/* The chips above, then the switches (switch.c). */
size_t catalog_definition_count(void) {
    return CHIP_DEFINITION_COUNT + switch_definition_count();
}

const ChipDefinition *catalog_definition_at(size_t index) {
    if (index < CHIP_DEFINITION_COUNT) {
        return DEFINITIONS[index];
    }
    return switch_definition_at(index - CHIP_DEFINITION_COUNT);
}

static const char *role_name(PinRole role) {
    switch (role) {
        case PIN_ROLE_INPUT:
            return "input";
        case PIN_ROLE_OUTPUT:
            return "output";
        case PIN_ROLE_INOUT:
            return "inout";
        case PIN_ROLE_POWER:
            return "power";
        case PIN_ROLE_GROUND:
            return "ground";
        case PIN_ROLE_NO_CONNECT:
            return "noConnect";
    }

    return "noConnect";
}

static void write_json_string(FILE *out, const char *text);

/* Ends a pin object: its role and group. */
static void write_pin_tail(FILE *out, const ChipPin *pin) {
    fprintf(out, ",\"role\":\"%s\",\"group\":", role_name(pin->role));
    write_json_string(out, pin->group != NULL ? pin->group : CHIP_PIN_GROUP_DEFAULT);
    fputc('}', out);
}

static void write_json_string(FILE *out, const char *text) {
    const unsigned char *p = (const unsigned char *)text;

    fputc('"', out);
    for (; *p != '\0'; p++) {
        switch (*p) {
            case '"':
                fputs("\\\"", out);
                break;
            case '\\':
                fputs("\\\\", out);
                break;
            default:
                if (*p < 0x20) {
                    fprintf(out, "\\u%04x", *p);
                } else {
                    fputc(*p, out);
                }
        }
    }
    fputc('"', out);
}

/* Names as the app shows them. They come from the model's own config. */
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

static const char *family_name(LogicFamily family) {
    switch (family) {
        case LOGIC_FAMILY_HC: return "HC";
        case LOGIC_FAMILY_LS: return "LS";
        case LOGIC_FAMILY_ACT: return "ACT";
        case LOGIC_FAMILY_HCT: return "HCT";
        case LOGIC_FAMILY_F: return "F";
        case LOGIC_FAMILY_AC: return "AC";
        case LOGIC_FAMILY_AHCT: return "AHCT";
        case LOGIC_FAMILY_ALS: return "ALS";
        case LOGIC_FAMILY_AS: return "AS";
        case LOGIC_FAMILY_AHC: return "AHC";
        case LOGIC_FAMILY_S: return "S";
        case LOGIC_FAMILY_C: return "C";
        case LOGIC_FAMILY_H: return "H";
        case LOGIC_FAMILY_HCS: return "HCS";
        case LOGIC_FAMILY_LV: return "LV";
        case LOGIC_FAMILY_LVC: return "LVC";
        case LOGIC_FAMILY_AUP: return "AUP";
        case LOGIC_FAMILY_AUC: return "AUC";
        case LOGIC_FAMILY_ALVC: return "ALVC";
        case LOGIC_FAMILY_TTL: return "TTL";
        case LOGIC_FAMILY_SRAM: return "SRAM";
        case LOGIC_FAMILY_SWITCH: return "Switch";
    }
    return "unknown";
}

static void write_chip(FILE *out, const ChipDefinition *chip) {
    size_t i;
    bool first = true;

    fputs("{\"id\":", out);
    write_json_string(out, chip->id);
    fputs(",\"function\":", out);
    write_json_string(out, chip->function);
    fputs(",\"description\":", out);
    write_json_string(out, chip->description);
    fputs(",\"category\":", out);
    write_json_string(out, chip->category);
    fprintf(out, ",\"gateCount\":%zu", chip->gate_count);
    if (chip->subcategory != NULL) {
        fputs(",\"subcategory\":", out);
        write_json_string(out, chip->subcategory);
    }

    fputs(",\"parts\":[", out);
    for (i = 0; i < chip->part_number_count; i++) {
        SimLogicConfig config;
        const char *part = chip->part_numbers[i];

        if (chip->lookup(part, &config) == NULL) {
            continue;
        }
        if (!first) {
            fputc(',', out);
        }
        fputs("{\"name\":", out);
        write_json_string(out, part);
        fprintf(out, ",\"package\":\"%s\"", package_name(config.package));
        if (config.family == LOGIC_FAMILY_SRAM) {
            /* An SRAM's family is its speed grade, the address access time. */
            fprintf(out, ",\"family\":\"%llu ns\"",
                    (unsigned long long)(sram_access_time(&config) / 1000));
        } else if (config.family == LOGIC_FAMILY_SWITCH) {
            /* A switch's family is its actuator style, e.g. "Piano actuator". */
            fputs(",\"family\":", out);
            write_json_string(out, switch_family_label(&config));
        } else {
            fprintf(out, ",\"family\":\"%s\"", family_name(config.family));
        }
        {
            unsigned flags = chip->part_flags != NULL ? chip->part_flags[i] : 0;

            fprintf(out, ",\"obsolete\":%s,\"military\":%s,\"automotive\":%s,\"enhanced\":%s}",
                    (flags & PART_FLAG_OBSOLETE) ? "true" : "false",
                    (flags & PART_FLAG_MILITARY) ? "true" : "false",
                    (flags & PART_FLAG_AUTOMOTIVE) ? "true" : "false",
                    (flags & PART_FLAG_ENHANCED) ? "true" : "false");
        }
        first = false;
    }

    fputs("],\"pins\":[", out);
    for (i = 0; i < chip->pin_count; i++) {
        const ChipPin *pin = &chip->pins[i];

        if (i > 0) {
            fputc(',', out);
        }
        fprintf(out, "{\"number\":%u,\"name\":", pin->number);
        write_json_string(out, pin->name);
        write_pin_tail(out, pin);
    }
    fputs("]}", out);
}

static void write_oscillator(FILE *out, size_t index) {
    OscConfig config;
    const ChipPin *pins;
    size_t count;
    size_t i;

    if (osc_part_config_at(index, &config) != 0) {
        return;
    }
    pins = osc_pins(config.family, config.package);
    count = osc_pin_count(config.family, config.package);

    fputs("{\"name\":", out);
    write_json_string(out, osc_part_name_at(index));
    fputs(",\"family\":", out);
    write_json_string(out, osc_family_label(config.family));
    fputs(",\"package\":", out);
    write_json_string(out, osc_package_label(config.package));
    fprintf(out, ",\"minHz\":%llu,\"maxHz\":%llu,\"defaultHz\":%llu",
            (unsigned long long)osc_part_min_hz(index),
            (unsigned long long)osc_part_max_hz(index),
            (unsigned long long)config.frequency_hz);
    if (config.enable_pin >= 0) {
        fprintf(out, ",\"enablePin\":%u", pins[config.enable_pin].number);
    }

    fputs(",\"pins\":[", out);
    for (i = 0; i < count; i++) {
        if (i > 0) {
            fputc(',', out);
        }
        fprintf(out, "{\"number\":%u,\"name\":", pins[i].number);
        write_json_string(out, pins[i].name);
        write_pin_tail(out, &pins[i]);
    }
    fputs("]}", out);
}

static void write_led(FILE *out, size_t index) {
    LedConfig config;
    size_t count;
    size_t i;

    if (led_part_config_at(index, &config) != 0) {
        return;
    }
    count = led_pin_count(config.package);

    fputs("{\"name\":", out);
    write_json_string(out, led_part_name_at(index));
    fputs(",\"color\":", out);
    write_json_string(out, led_color_label(config.color));
    fputs(",\"package\":", out);
    write_json_string(out, led_package_label(config.package));

    fputs(",\"pins\":[", out);
    for (i = 0; i < count; i++) {
        const ChipPin *pin = &led_pins(config.package)[i];

        if (i > 0) {
            fputc(',', out);
        }
        fprintf(out, "{\"number\":%u,\"name\":", pin->number);
        write_json_string(out, pin->name);
        write_pin_tail(out, pin);
    }
    fputs("]}", out);
}

void catalog_write_json(FILE *out) {
    size_t i;

    fputs("{\"chips\":[", out);
    for (i = 0; i < catalog_definition_count(); i++) {
        if (i > 0) {
            fputc(',', out);
        }
        write_chip(out, catalog_definition_at(i));
    }

    /* Generic LEDs: one entry per orderable color and package. */
    fputs("],\"leds\":[", out);
    for (i = 0; i < led_part_count(); i++) {
        if (i > 0) {
            fputc(',', out);
        }
        write_led(out, i);
    }

    /* Constant sources: VCC (always 1) and GND (sink to 0). */
    fputs("],\"supplies\":[", out);
    for (i = 0; i < supply_part_count(); i++) {
        SupplyConfig config;
        const ChipPin *pin;

        if (supply_part_config_at(i, &config) != 0) {
            continue;
        }
        pin = &supply_pins(config.kind)[0];
        if (i > 0) {
            fputc(',', out);
        }
        fputs("{\"name\":", out);
        write_json_string(out, supply_part_name_at(i));
        fprintf(out, ",\"pins\":[{\"number\":%u,\"name\":", pin->number);
        write_json_string(out, pin->name);
        write_pin_tail(out, pin);
        fputs("]}", out);
    }
    fputs("],\"oscillators\":{\"category\":\"Oscillator\",\"groups\":[", out);
    for (i = 0; i < osc_group_count(); i++) {
        size_t p;
        bool first = true;

        if (i > 0) {
            fputc(',', out);
        }
        fputs("{\"range\":", out);
        write_json_string(out, osc_group_label(i));
        fputs(",\"parts\":[", out);
        for (p = 0; p < osc_part_count(); p++) {
            if (!osc_part_in_group(p, i)) {
                continue;
            }
            if (!first) {
                fputc(',', out);
            }
            write_oscillator(out, p);
            first = false;
        }
        fputs("]}", out);
    }
    fputs("]}}\n", out);
}

char *catalog_json_copy(void) {
    char *buffer = NULL;
    size_t length = 0;
    FILE *out = open_memstream(&buffer, &length);

    if (out == NULL) {
        return NULL;
    }
    catalog_write_json(out);
    fclose(out);
    return buffer;
}
