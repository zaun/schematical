#ifndef TRACE_CORE_H
#define TRACE_CORE_H

/*
 * Per-part simulation trace, shared by the old baseline tool and trace_check.
 * It drives a fixed pseudo-random stimulus into one part for 3 us and hashes
 * every net change (plan-01-A section 4.1). Only the engine (sim.h) is used.
 */

#include <stdint.h>
#include <stdio.h>

#include "sim.h"

typedef enum { TRACE_NONE, TRACE_INPUT, TRACE_INOUT, TRACE_POWER, TRACE_GROUND } TraceRole;

typedef struct {
    const char *name;                 /* part name: seeds the stimulus */
    const SimComponentType *type;
    const void *config;
    const TraceRole *roles;
    size_t pin_count;
    /* NULL if not a switch. Position is 1-based. */
    int (*set_switch)(Sim *sim, SimInstance *instance, size_t position, int on);
    size_t switch_positions;
    /* NULL unless a 7-segment display: '0'/'1' per light, lit since `since`. */
    size_t (*lights)(const Sim *sim, const SimInstance *instance, SimTime since,
                     char *out, size_t cap);
} TraceSubject;

uint64_t trace_fnv1a64(const char *text);

/* Runs the trace; prints every change to `verbose` when it is not NULL. */
uint64_t trace_run(const TraceSubject *subject, FILE *verbose);

#endif
