#ifndef MODEL_PINS_H
#define MODEL_PINS_H

/*
 * Pin facts a few models keep private, for the voltage check (voltage.c).
 * Each returns 0 for a part of another kind.
 */

#include <stddef.h>

#include "parts.h"

/* An LED's anode and cathode, as indices into Part.pins; also a character
   LCD's backlight anode and cathode (0 for a module without a backlight). */
int led_pins(const Part *part, size_t *anode, size_t *cathode);

/* led_pins for charlcd parts (charlcd.c). */
int charlcd_backlight_pins(const Part *part, size_t *anode, size_t *cathode);

/* A switch position's (1-based) two contact pins, as indices into Part.pins. */
int switch_contact_pins(const Part *part, unsigned position, size_t *a, size_t *b);

/*
 * 1 when pin (an index into Part.pins) is a PIN_OUTPUT the part never drives
 * high: an open-drain output (gate and dff "output": "open-drain"). Each
 * kind answers for its own parts through PartKindOps.pin_open_drain;
 * 0 for every kind without one.
 */
int part_pin_open_drain(const Part *part, size_t pin);

#endif
