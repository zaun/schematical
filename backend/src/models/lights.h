#ifndef LIGHTS_H
#define LIGHTS_H

/*
 * On-time tracking for lights (LEDs, display segments), shared by led.c and
 * display7.c. A light reads as lit when it is on now or was on at any time
 * at or after `since` (the start of the app's last step), so a multiplexed
 * digit scanned during the step counts as lit.
 */

#include <stddef.h>

#include "sim.h"

typedef struct {
    unsigned char on;
    unsigned char ever_on;
    SimTime last_on;   /* when the light last went off */
    SimTime on_total;  /* on-time up to changed_at */
    SimTime changed_at;
} Light;

void lights_set(Light *lights, size_t index, int on, SimTime now);

/* Writes 1/0 per light (at most cap); returns count. */
size_t lights_read(const Light *lights, size_t count, SimTime since,
                   unsigned char *out, size_t cap);

/* Writes each light's cumulative on-time up to `now` (at most cap); returns count. */
size_t lights_time(const Light *lights, size_t count, SimTime now,
                   uint64_t *out, size_t cap);

#endif
