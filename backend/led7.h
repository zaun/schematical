#ifndef LED7_H
#define LED7_H

/*
 * LED 7-segment displays (Lite-On), transcribed from datasheets/led-7/ into
 * led7_parts.inc. One catalog definition per part: category "Indicators",
 * subcategory "LED 7-Seg". The count (gate_count) is the number of digits.
 *
 * Each part is a set of LEDs. A digit has segments A-G and a decimal point
 * (DP; LTS-312AHR also has a left-hand LDP). Clock displays add L1/L2 (a
 * colon) and L3 (a raised dot), which have their own common pin and share
 * their other pin with segments A, B and C. The LTD-6x30/6x50 "+/-1
 * overflow" parts have a first digit of B, C, G, DP and HJ, a vertical bar
 * through the middle that turns the G bar into a plus sign.
 *
 * An LED is ON while its anode reads 1 and its cathode reads 0, the same
 * rule as the single LED. The model is passive: it drives nothing. Common
 * pins of one digit that the datasheet lists twice (e.g. pins 3 and 8) are
 * joined inside the part, as a closed contact.
 *
 * Multiplexed displays light one digit at a time, so the app asks whether an
 * LED was ON at any time since a given time (led7_instance_glow): a digit
 * that is scanned during the app's last step counts as lit.
 */

#include <stddef.h>

#include "chip_definition.h"
#include "sim.h"

typedef enum {
    LED7_SEG_A,
    LED7_SEG_B,
    LED7_SEG_C,
    LED7_SEG_D,
    LED7_SEG_E,
    LED7_SEG_F,
    LED7_SEG_G,
    LED7_SEG_DP,
    LED7_SEG_LDP,
    LED7_SEG_HJ,
    LED7_SEG_L1,
    LED7_SEG_L2,
    LED7_SEG_L3,
    LED7_SEG_COUNT
} Led7Segment;

/* The "digit" of the colon and dot LEDs (L1-L3). */
#define LED7_EXTRAS 9u

extern const SimComponentType LED7_TYPE;

/* Catalog definitions, one per part. */
size_t led7_definition_count(void);
const ChipDefinition *led7_definition_at(size_t index);

/*
 * Returns the simulation component for a display's part number and fills in
 * its configuration. Returns NULL when the name is not a 7-segment display.
 */
const SimComponentType *led7_lookup(const char *part_number, SimLogicConfig *config);

/* ---- Display layout, for the catalog ---------------------------------------
 * All of these take a configuration from led7_lookup.
 */

const char *led7_segment_name(Led7Segment segment);    /* "A", "DP", "L1", ... */
unsigned led7_digits(const SimLogicConfig *config);
int led7_common_anode(const SimLogicConfig *config);   /* 1 = common anode */
const char *led7_size_label(const SimLogicConfig *config);  /* "0.56 in" */
const char *led7_color_label(const SimLogicConfig *config); /* "Red", "Green", ... */
const char *led7_chip_label(const SimLogicConfig *config);  /* "AlInGaP Super Red" */
/* Digit the colon (L1, L2) and the dot (L3) sit after; 0 = none. */
unsigned led7_colon_after(const SimLogicConfig *config);
unsigned led7_dot_after(const SimLogicConfig *config);

/*
 * The LEDs, in a fixed order: digit by digit (digit 1 first), segments in
 * Led7Segment order, then L1-L3 (digit LED7_EXTRAS). led7_instance_glow
 * reports them in the same order.
 */
size_t led7_led_count(const SimLogicConfig *config);
int led7_led_at(const SimLogicConfig *config, size_t index, unsigned *digit, Led7Segment *segment);

/*
 * Pin numbers in drawing order, for one side of the symbol: segment pins
 * (commons = 0), digit by digit and in segment order, or common pins
 * (commons = 1), digit by digit with the L1-L3 common last. No-connect pins
 * and missing pins (the "hidden" group) are in neither list. Returns the number of pins written (at most cap).
 */
size_t led7_side_pins(const SimLogicConfig *config, int commons, unsigned *numbers, size_t cap);

/* ---- Simulation -------------------------------------------------------- */

/* Number of LEDs of an instance, or 0 when it is not a 7-segment display. */
size_t led7_instance_led_count(const SimInstance *instance);

/*
 * Writes '1' for every LED that is ON now or was ON at any time at or after
 * `since`, '0' for the others, in led7_led_at order. Writes at most cap
 * characters and no terminator. Returns the number of LEDs, 0 when the
 * instance is not a 7-segment display.
 */
size_t led7_instance_glow(const Sim *sim, const SimInstance *instance, SimTime since,
                          char *out, size_t cap);

#endif
