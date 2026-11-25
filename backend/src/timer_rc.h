#ifndef TIMER_RC_H
#define TIMER_RC_H

/*
 * The timing analysis of the `timer` kind (plan-48-E section 4): what a
 * 555 section is wired as, and its times, from the resistors, capacitors,
 * diodes and pull-ups on its pins. circuit_start calls it for every section
 * of every timer once the voltage check has passed, and hands each result to
 * the model (timer_model_set). Pure: the parts and the netlist go in, a
 * TimerState comes out.
 */

#include <stddef.h>
#include <stdint.h>

#include "parts.h"
#include "timer_state.h"
#include "voltage.h"

/* values[part]: a placed resistor's or pull part's value in milliohms, or a
   capacitor's in femtofarads; 0 = the library part's own (a pull part
   without a value counts as 1 kilohm). */
void timer_rc_analyze(const Part *const *parts, const uint64_t *values, size_t part_count,
                      const VoltageConnection *connections, size_t connection_count,
                      size_t part, unsigned section, TimerState *out);

#endif
