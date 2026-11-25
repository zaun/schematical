#ifndef TIMER_H
#define TIMER_H

/*
 * What circuit.c needs from the `timer` kind (plan-48-E): where a timer
 * section's pins are, and the hook that hands a section the timing the
 * analysis (timer_rc.h) found. Internal, in the model_pins.h style.
 */

#include <stddef.h>

#include "parts.h"
#include "timer_state.h"

/* Pin indices into Part.pins; SIZE_MAX = the section has no such pin. */
typedef struct { size_t vcc, gnd, trig, thres, disch, out, reset, cont; } TimerPins;

/* Section `section`'s pins (0-based). 1, or 0 past the last section and for a
   part that is no timer. */
int timer_section_pins(const Part *part, unsigned section, TimerPins *out);

/* Sets / reads a section's timing in the model's state (the instance's
   sim_instance_state). timer_model_set runs before sim_start. */
void timer_model_set(void *state, unsigned section, const TimerState *timing);
int timer_model_get(const void *state, unsigned section, TimerState *out);

#endif
