/*
 * The two terminals of an inductor or a fuse (terminals.h, plan-47-I).
 */

#include <string.h>

#include "terminals.h"

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
    size_t *first = kind_alloc(ctx, 2 * sizeof(*first));
    size_t *ends = kind_alloc(ctx, (pin_count + 1) * sizeof(*ends));
    size_t i;
    size_t side;

    if (t == NULL || first == NULL || ends == NULL) {
        return NULL;
    }
    t->tie_from = kind_alloc(ctx, (pin_count + 1) * sizeof(*t->tie_from));
    t->tie_to = kind_alloc(ctx, (pin_count + 1) * sizeof(*t->tie_to));
    if (t->tie_from == NULL || t->tie_to == NULL) {
        return NULL;
    }
    for (side = 0; side < 2; side++) {
        size_t n = kind_pins(ctx, pins, pin_count, names[side], ends, pin_count);
        size_t k;

        if (n == 0) {
            return NULL;
        }
        first[side] = ends[0];
        for (k = 1; k < n; k++) {
            t->tie_from[t->tie_count] = ends[0];
            t->tie_to[t->tie_count++] = ends[k];
        }
    }
    t->a = first[0];
    t->b = first[1];
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
    return t;
}

int terminals_contact(const TerminalPins *terminals, size_t k, size_t *a, size_t *b) {
    if (terminals == NULL) {
        return 0;
    }
    if (k == 0) {
        *a = terminals->a;
        *b = terminals->b;
        return 1;
    }
    k -= 1;
    if (k < terminals->tie_count) {
        *a = terminals->tie_from[k];
        *b = terminals->tie_to[k];
        return 1;
    }
    return 0;
}

void terminals_close_ties(Sim *sim, SimInstance *instance, const TerminalPins *terminals) {
    size_t i;

    for (i = 0; i < terminals->tie_count; i++) {
        sim_set_contact(sim, instance, terminals->tie_from[i], terminals->tie_to[i], 1);
    }
}
