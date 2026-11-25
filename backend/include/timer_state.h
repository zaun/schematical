#ifndef TIMER_STATE_H
#define TIMER_STATE_H

/*
 * What a 555-type timer section is doing and why (plan-48-E section 3.3),
 * for circuit_timer_state (circuit.h). All times are picoseconds.
 *
 *   mode      TIMER_LATCH     no capacitor on its timing pins: a set/reset latch
 *                             on logic levels (TRIG sets, THRES resets)
 *             TIMER_ASTABLE   free running: OUT is 1 for first_ps, then 0 for
 *                             low_ps and 1 for high_ps, over and over
 *             TIMER_ONE_SHOT  a TRIG pulse makes OUT 1 for high_ps
 *             TIMER_STOPPED   the wiring is not recognized (see reason): OUT low
 *   reason    why a stopped section is stopped; TIMER_OK otherwise
 */

#include <stdint.h>

typedef enum { TIMER_LATCH, TIMER_ASTABLE, TIMER_ONE_SHOT, TIMER_STOPPED } TimerMode;
typedef enum {
    TIMER_OK, TIMER_NO_SUPPLY, TIMER_CONTROL, TIMER_SHAPE, TIMER_NO_VALUE, TIMER_TOO_FAST
} TimerReason;

typedef struct {
    TimerMode mode;
    TimerReason reason;
    uint64_t first_ps, high_ps, low_ps;
} TimerState;

/* The sentence for a reason, as the app shows it ("" for TIMER_OK). */
const char *timer_reason_text(TimerReason reason);

#endif
