#ifndef TERMINALS_H
#define TERMINALS_H

/*
 * Contacts between the pins of parts whose terminals are joined inside the
 * part (inductor, fuse, relay; plan-47-I and J). A terminal's name may be on
 * several pins of a pinout (a four-pad inductor has two pads per terminal, a
 * power relay two pins per contact) and every pin of a name is one piece of
 * metal. The engine leaves out a contact whose pin has no net, so a chain
 * through a first pin that nobody wired would drop the others: instead every
 * pin of one name gets a contact to every other pin of the name (the ties), and
 * every pin of the two names a part joins gets a contact to every pin of the
 * other (the main contacts), so any pins that are wired are joined whichever
 * pads the circuit uses.
 */

#include <stddef.h>

#include "kind.h"

/* A set of contacts: contact i joins pins a[i] and b[i] (indices into Part.pins). */
typedef struct {
    size_t count;
    size_t *a, *b;
} ContactSet;

/* Reserves room for `capacity` contacts in the arena; 0, or -1 (error set). */
int contacts_new(KindContext *ctx, ContactSet *set, size_t capacity);

/* Adds a contact between every pin of ga and every pin of gb (no more than the
   capacity given to contacts_new). */
void contacts_cross(ContactSet *set, const size_t *ga, size_t na, const size_t *gb, size_t nb);

/* Adds a contact between every two pins of g. */
void contacts_within(ContactSet *set, const size_t *g, size_t n);

/* Closes or opens every contact of the set (sim_set_contact). */
void contacts_set(Sim *sim, SimInstance *instance, const ContactSet *set, int closed);

/* The k-th contact of the set (0-based); 0 past the last. */
int contacts_at(const ContactSet *set, size_t k, size_t *a, size_t *b);

/* The two terminals of a part (inductor, fuse). */
typedef struct {
    ContactSet main;   /* every pin of terminal 1 with every pin of terminal 2: a fuse opens these */
    ContactSet ties;   /* the pins of each terminal with each other: always closed */
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

/* The k-th fixed contact (0-based) of a part's terminals: the main contacts,
   then the ties. 0 past the last. */
int terminals_contact(const TerminalPins *terminals, size_t k, size_t *a, size_t *b);

#endif
