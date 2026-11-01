#ifndef MODEL_PINS_H
#define MODEL_PINS_H

/*
 * Pin facts a few models keep private, for the voltage check (voltage.c).
 * Each returns 0 for a part of another kind.
 */

#include <stddef.h>

#include "parts.h"

/* An LED's anode and cathode, as indices into Part.pins. */
int led_pins(const Part *part, size_t *anode, size_t *cathode);

/* A switch position's (1-based) two contact pins, as indices into Part.pins. */
int switch_contact_pins(const Part *part, unsigned position, size_t *a, size_t *b);

/* 1 for a gate whose outputs are open-drain (74x266). */
int gate_open_drain(const Part *part);

#endif
