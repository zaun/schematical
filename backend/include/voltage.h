#ifndef VOLTAGE_H
#define VOLTAGE_H

/*
 * The voltage check: a static check over a netlist, separate from the logic
 * simulation (plan-07-B). It works out the voltage each net carries and
 * reports every pin whose voltage is out of range.
 *
 * A net's voltage comes from, in order: the supplies on it (the highest),
 * else the outputs of powered parts on it (a part with a "vcc" range is
 * powered at the highest voltage on its power pins), else what a generic
 * LED carries from its anode to its cathode. A net with none of these has
 * no known voltage and is not checked. Parts with no "vcc" data are neither
 * checked nor sources. A supply and GND on one net is a short, not a
 * voltage problem.
 *
 * circuit_start refuses a circuit with a problem; the app calls this
 * function directly on a candidate netlist to refuse an edit.
 */

#include <stddef.h>
#include <stdint.h>

#include "parts.h"

typedef enum {
    VOLTAGE_SUPPLY_CLASH,   /* two supplies of different non-zero voltage on one net */
    VOLTAGE_POWER_RANGE,    /* power pin outside the part's vcc range */
    VOLTAGE_INPUT_OVER,     /* input/inout pin above the part's input max */
    VOLTAGE_SWITCH_OVER     /* switch contact pin above its rating */
} VoltageProblemKind;

typedef struct { size_t part; size_t pin; size_t net; } VoltageConnection;  /* pin: index into Part.pins */
typedef struct { size_t part; unsigned position; } VoltageContact;          /* a closed switch position, 1-based */

typedef struct {
    VoltageProblemKind kind;
    size_t net;              /* the circuit net index */
    size_t part, pin;        /* the offending pin; for CLASH: one supply */
    size_t other_part;       /* CLASH: the other supply; else = part */
    uint32_t mv;             /* the net's voltage (CLASH: the higher) */
    uint32_t min_mv, max_mv; /* the allowed range (CLASH: the lower voltage in both) */
} VoltageProblem;

/* Returns the number of problems (all of them); writes up to cap into out,
   ordered by net, then part, then pin. parts[i] is the part at index i. */
size_t voltage_check(const Part *const *parts, size_t part_count, size_t net_count,
                     const VoltageConnection *connections, size_t connection_count,
                     const VoltageContact *closed, size_t closed_count,
                     VoltageProblem *out, size_t cap);

/* "voltage: …" text, e.g.
     voltage: 5 V on pin 14 (VCC) of SN74AUC00PWR; it needs 0.8-2.7 V
   Returns the length written (snprintf rules). */
int voltage_describe(const Part *const *parts, const VoltageProblem *problem,
                     char *buffer, size_t size);

#endif
