#include "led.h"

#include <string.h>

/* ---- Pins --------------------------------------------------------------- */

static const ChipPin PINS_2[] = {
    {1, "K", PIN_ROLE_INPUT},
    {2, "A", PIN_ROLE_INPUT},
};

static const ChipPin PINS_4[] = {
    {1, "K", PIN_ROLE_INPUT},
    {2, "A", PIN_ROLE_INPUT},
    {3, "NC", PIN_ROLE_NO_CONNECT},
    {4, "NC", PIN_ROLE_NO_CONNECT},
};

size_t led_pin_count(LedPackage package) {
    return package == LED_PACKAGE_4DIP ? sizeof(PINS_4) / sizeof(PINS_4[0])
                                       : sizeof(PINS_2) / sizeof(PINS_2[0]);
}

const ChipPin *led_pins(LedPackage package) {
    return package == LED_PACKAGE_4DIP ? PINS_4 : PINS_2;
}

/* ---- Labels ------------------------------------------------------------- */

static const char *const COLOR_LABELS[LED_COLOR_COUNT] = {
    "Red", "Green", "Blue", "Amber", "White",
};

static const char *const PACKAGE_LABELS[LED_PACKAGE_COUNT] = {
    "0402", "0603", "0805", "1204", "1205", "1206",
    "1209", "1411", "1414", "4-DIP", "Radial",
};

const char *led_color_label(LedColor color) {
    return color < LED_COLOR_COUNT ? COLOR_LABELS[color] : "unknown";
}

const char *led_package_label(LedPackage package) {
    return package < LED_PACKAGE_COUNT ? PACKAGE_LABELS[package] : "unknown";
}

/* ---- Orderable parts ---------------------------------------------------- */

typedef struct {
    const char *name;
    LedConfig config;
} LedPart;

/* One row per package, one entry per color. */
#define LED_PACKAGE_ROW(PKG, LABEL) \
    {"LED-RED-" LABEL,   {LED_COLOR_RED,   LED_PACKAGE_##PKG}}, \
    {"LED-GREEN-" LABEL, {LED_COLOR_GREEN, LED_PACKAGE_##PKG}}, \
    {"LED-BLUE-" LABEL,  {LED_COLOR_BLUE,  LED_PACKAGE_##PKG}}, \
    {"LED-AMBER-" LABEL, {LED_COLOR_AMBER, LED_PACKAGE_##PKG}}, \
    {"LED-WHITE-" LABEL, {LED_COLOR_WHITE, LED_PACKAGE_##PKG}},

static const LedPart PARTS[] = {
    LED_PACKAGE_ROW(0402, "0402")
    LED_PACKAGE_ROW(0603, "0603")
    LED_PACKAGE_ROW(0805, "0805")
    LED_PACKAGE_ROW(1204, "1204")
    LED_PACKAGE_ROW(1205, "1205")
    LED_PACKAGE_ROW(1206, "1206")
    LED_PACKAGE_ROW(1209, "1209")
    LED_PACKAGE_ROW(1411, "1411")
    LED_PACKAGE_ROW(1414, "1414")
    LED_PACKAGE_ROW(4DIP, "4-DIP")
    LED_PACKAGE_ROW(RADIAL, "RADIAL")
};

#undef LED_PACKAGE_ROW

size_t led_part_count(void) {
    return sizeof(PARTS) / sizeof(PARTS[0]);
}

const char *led_part_name_at(size_t index) {
    return index < led_part_count() ? PARTS[index].name : NULL;
}

int led_part_config_at(size_t index, LedConfig *config) {
    if (index >= led_part_count() || config == NULL) {
        return -1;
    }
    *config = PARTS[index].config;
    return 0;
}

/* ---- Simulation model --------------------------------------------------- */

static size_t led_type_pin_count(const void *config) {
    return led_pin_count(((const LedConfig *)config)->package);
}

/*
 * Passive: no state, no drive, no timers. Every callback is left NULL so the
 * simulator skips it.
 */
const SimComponentType LED_TYPE = {
    "LED",
    led_type_pin_count,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

const SimComponentType *led_lookup(const char *part_number, LedConfig *config) {
    size_t i;

    if (part_number == NULL) {
        return NULL;
    }

    for (i = 0; i < led_part_count(); i++) {
        if (strcmp(PARTS[i].name, part_number) == 0) {
            if (config != NULL) {
                *config = PARTS[i].config;
            }
            return &LED_TYPE;
        }
    }

    return NULL;
}
