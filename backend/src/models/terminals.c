/*
 * Contacts between the pins of inductors, fuses and relays (terminals.h,
 * plan-47-I and J).
 */

#include <string.h>

#include "terminals.h"

int contacts_new(KindContext *ctx, ContactSet *set, size_t capacity) {
    set->count = 0;
    set->a = kind_alloc(ctx, (capacity + 1) * sizeof(*set->a));
    set->b = kind_alloc(ctx, (capacity + 1) * sizeof(*set->b));
    return set->a != NULL && set->b != NULL ? 0 : -1;
}

void contacts_cross(ContactSet *set, const size_t *ga, size_t na, const size_t *gb, size_t nb) {
    size_t i;
    size_t j;

    for (i = 0; i < na; i++) {
        for (j = 0; j < nb; j++) {
            set->a[set->count] = ga[i];
            set->b[set->count++] = gb[j];
        }
    }
}

void contacts_within(ContactSet *set, const size_t *g, size_t n) {
    size_t i;
    size_t j;

    for (i = 0; i < n; i++) {
        for (j = i + 1; j < n; j++) {
            set->a[set->count] = g[i];
            set->b[set->count++] = g[j];
        }
    }
}

void contacts_set(Sim *sim, SimInstance *instance, const ContactSet *set, int closed) {
    size_t i;

    for (i = 0; i < set->count; i++) {
        sim_set_contact(sim, instance, set->a[i], set->b[i], closed);
    }
}

int contacts_at(const ContactSet *set, size_t k, size_t *a, size_t *b) {
    if (k >= set->count) {
        return 0;
    }
    *a = set->a[k];
    *b = set->b[k];
    return 1;
}

int terminals_names(KindContext *ctx, const cJSON *json, const char *names[2]) {
    const char *const *list;
    size_t count;

    if (kind_names(ctx, json, "terminals", &list, &count) != 0) {
        return -1;
    }
    if (count != 2) {
        return kind_error(ctx, "\"terminals\" must name 2 pins");
    }
    if (strcmp(list[0], list[1]) == 0) {
        return kind_error(ctx, "\"terminals\" must name 2 different pins");
    }
    names[0] = list[0];
    names[1] = list[1];
    return 0;
}

TerminalPins *terminals_bind(KindContext *ctx, const char *what, const char *const names[2],
                             const PartPin *pins, size_t pin_count) {
    TerminalPins *t = kind_alloc(ctx, sizeof(*t));
    size_t *group[2];
    size_t n[2];
    size_t i;
    size_t side;

    if (t == NULL) {
        return NULL;
    }
    for (side = 0; side < 2; side++) {
        group[side] = kind_alloc(ctx, (pin_count + 1) * sizeof(*group[side]));
        if (group[side] == NULL) {
            return NULL;
        }
        n[side] = kind_pins(ctx, pins, pin_count, names[side], group[side], pin_count);
        if (n[side] == 0) {
            return NULL;
        }
    }
    for (i = 0; i < pin_count; i++) {
        int terminal = strcmp(pins[i].name, names[0]) == 0 || strcmp(pins[i].name, names[1]) == 0;

        if (terminal && pins[i].role != PIN_INOUT) {
            kind_error(ctx, "%s pin \"%s\" must be inout", what, pins[i].name);
            return NULL;
        }
        if (!terminal && pins[i].role != PIN_NC) {
            kind_error(ctx, "%s pin \"%s\" must be nc", what, pins[i].name);
            return NULL;
        }
    }
    if (contacts_new(ctx, &t->main, n[0] * n[1]) != 0 ||
        contacts_new(ctx, &t->ties, n[0] * n[0] + n[1] * n[1]) != 0) {
        return NULL;
    }
    contacts_cross(&t->main, group[0], n[0], group[1], n[1]);
    contacts_within(&t->ties, group[0], n[0]);
    contacts_within(&t->ties, group[1], n[1]);
    return t;
}

int terminals_contact(const TerminalPins *terminals, size_t k, size_t *a, size_t *b) {
    if (terminals == NULL) {
        return 0;
    }
    if (k < terminals->main.count) {
        return contacts_at(&terminals->main, k, a, b);
    }
    return contacts_at(&terminals->ties, k - terminals->main.count, a, b);
}
