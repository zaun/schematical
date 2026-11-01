#ifndef LED_H
#define LED_H

/*
 * Generic LED. Instead of one catalog entry per vendor part number, every
 * combination of color and package is offered under a generic name:
 *
 *     LED-<COLOR>-<PACKAGE>      e.g. LED-RED-0603, LED-WHITE-RADIAL
 *
 * Colors: Red, Green, Blue, Amber, White.
 * Packages: 0402, 0603, 0805, 1204, 1205, 1206, 1209, 1411, 1414, DIP, Radial.
 *
 * Pins: pin 1 is the cathode (K) and pin 2 the anode (A), the usual KiCad
 * convention. DIP has two extra pins (3 and 4), which are no-connect.
 *
 * The model is passive. It drives nothing and does not model forward voltage
 * or current. Read its anode and cathode nets to see whether it should be lit
 * (anode high, cathode low).
 */

#include <stddef.h>

#include "chip_definition.h"
#include "sim.h"

typedef enum {
    LED_COLOR_RED,
    LED_COLOR_GREEN,
    LED_COLOR_BLUE,
    LED_COLOR_AMBER,
    LED_COLOR_WHITE,
    LED_COLOR_COUNT
} LedColor;

typedef enum {
    LED_PACKAGE_0402,
    LED_PACKAGE_0603,
    LED_PACKAGE_0805,
    LED_PACKAGE_1204,
    LED_PACKAGE_1205,
    LED_PACKAGE_1206,
    LED_PACKAGE_1209,
    LED_PACKAGE_1411,
    LED_PACKAGE_1414,
    LED_PACKAGE_DIP,
    LED_PACKAGE_RADIAL,
    LED_PACKAGE_COUNT
} LedPackage;

typedef struct {
    LedColor color;
    LedPackage package;
} LedConfig;

extern const SimComponentType LED_TYPE;

/* Number of pins for a package: 2, or 4 for DIP. */
size_t led_pin_count(LedPackage package);

/* Pin table for a package. Its length equals led_pin_count(package). */
const ChipPin *led_pins(LedPackage package);

const char *led_color_label(LedColor color);
const char *led_package_label(LedPackage package);

/* Orderable LED names, one per color and package combination. */
size_t led_part_count(void);

/*
 * Returns the simulation component for an orderable LED name and fills in
 * its configuration. Returns NULL when the name is not an LED part.
 */
const SimComponentType *led_lookup(const char *part_number, LedConfig *config);

/* Name of the orderable LED part at index, or NULL when out of range. */
const char *led_part_name_at(size_t index);

/* Configuration of the orderable LED part at index. Returns 0 on success. */
int led_part_config_at(size_t index, LedConfig *config);

#endif
