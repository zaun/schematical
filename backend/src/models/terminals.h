#ifndef TERMINALS_H
#define TERMINALS_H

/*
 * The two terminals of a two-terminal part whose terminals are joined inside
 * the part (inductor, fuse; plan-47-I). The device's "terminals" key names the
 * two terminal pins; a name may be on several pins of a pinout (a four-pad
 * inductor has two pads per terminal), and the first pin of each name is the
 * contact's end. Every further pin of a name is tied to the first pin by a
 * contact that is always closed.
 */

#include <stddef.h>

#include "kind.h"

typedef struct {
    size_t a, b;                 /* the first pin of each terminal name */
    size_t tie_count;
    size_t *tie_from, *tie_to;   /* same-name ties: first pin -> another pin of the name */
} TerminalPins;

/* Reads the device's "terminals" (two different pin names) into names[0..1]. */
int terminals_names(KindContext *ctx, const cJSON *json, const char *names[2]);

/*
 * Binds the names to one pinout. `what` ("inductor", "fuse") names the kind in
 * messages. Every pin with a terminal name must be inout and every other pin
 * nc. NULL (error set) on failure.
 */
TerminalPins *terminals_bind(KindContext *ctx, const char *what, const char *const names[2],
                             const PartPin *pins, size_t pin_count);

/* The k-th fixed contact (0-based) of a part's terminals: the terminals
   themselves, then each tie. 0 past the last. The terminals' own contact is
   k = 0 (a fuse may open it; the ties are closed whatever the fuse does). */
int terminals_contact(const TerminalPins *terminals, size_t k, size_t *a, size_t *b);

/* Closes every tie of the part (called from a model's start). */
void terminals_close_ties(Sim *sim, SimInstance *instance, const TerminalPins *terminals);

#endif
