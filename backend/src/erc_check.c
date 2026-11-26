/*
 * The ERC pin checks (include/erc.h, plan-63-D): the rules of KiCad's ERC that
 * read pin types, on a netlist of typed pins. See erc.h for what each reports;
 * the counting follows what kicad-cli 10.0.8 reports (planning/plan-63-data).
 */

#include "erc.h"

#include <stdlib.h>
#include "logic_alloc.h"

/* The default pin map of KiCad 10.0.8 (its stock templates' "pin_map"), rows and
   columns in ErcPinType order. */
const uint8_t ERC_DEFAULT_PIN_MAP[ERC_PIN_TYPE_COUNT][ERC_PIN_TYPE_COUNT] = {
    /*               in out bi tri pas fre uns pwI pwO oc oe nc */
    /* input    */ { 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2},
    /* output   */ { 0, 2, 0, 1, 0, 0, 1, 0, 2, 2, 2, 2},
    /* bidi     */ { 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 2},
    /* tri      */ { 0, 1, 0, 0, 0, 0, 1, 1, 2, 1, 1, 2},
    /* passive  */ { 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2},
    /* free     */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2},
    /* unspec   */ { 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 2},
    /* power_in */ { 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 2},
    /* power_out*/ { 0, 2, 1, 2, 0, 0, 1, 0, 2, 2, 2, 2},
    /* open_coll*/ { 0, 2, 0, 1, 0, 0, 1, 0, 2, 0, 0, 2},
    /* open_emit*/ { 0, 2, 1, 1, 0, 0, 1, 0, 2, 0, 0, 2},
    /* no_conn  */ { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
};

typedef struct {
    ErcProblem *items;
    size_t count, capacity;
    int out_of_memory; /* a problem could not be kept */
} Found;

static void found_add(Found *found, ErcProblemKind kind, size_t pin, size_t other, size_t net, int level) {
    if (found->count == found->capacity) {
        size_t capacity = found->capacity ? found->capacity * 2 : 64;
        ErcProblem *grown = logic_realloc(found->items, capacity * sizeof(*grown));

        if (grown == NULL) {
            found->out_of_memory = 1;
            return;
        }
        found->items = grown;
        found->capacity = capacity;
    }
    found->items[found->count].kind = kind;
    found->items[found->count].pin = pin;
    found->items[found->count].other = other;
    found->items[found->count].net = net;
    found->items[found->count].level = level;
    found->count++;
}

/* One net: idx lists the pins on it (every type), in pin order. */
static void check_net(Found *found, const ErcPin *pins, const size_t *idx, size_t count, size_t net,
                      int has_attachment,
                      const uint8_t map[ERC_PIN_TYPE_COUNT][ERC_PIN_TYPE_COUNT]) {
    size_t members = 0, i, j;
    int marked = 0, power_driven = 0, input_driven = 0;
    size_t first_power_in = SIZE_MAX, first_input = SIZE_MAX;

    for (i = 0; i < count; i++) {
        const ErcPin *p = &pins[idx[i]];

        if (p->type != ERC_NO_CONNECT) {
            members++;
        }
        if (p->no_connect_mark) {
            marked = 1;
        }
    }
    for (i = 0; i < count; i++) {
        const ErcPin *p = &pins[idx[i]];

        /* A wired pin of type no_connect is no connection. */
        if (p->type == ERC_NO_CONNECT) {
            if (net != SIZE_MAX) {
                found_add(found, ERC_NO_CONNECT_CONNECTED, idx[i], idx[i], net, 0);
            }
            continue;
        }
        if (p->no_connect_mark && net != SIZE_MAX && members >= 2) {
            found_add(found, ERC_NO_CONNECT_CONNECTED, idx[i], idx[i], net, 0);
        }
        if (p->type != ERC_FREE && !p->no_connect_mark && members == 1 && !has_attachment) {
            found_add(found, ERC_PIN_NOT_CONNECTED, idx[i], idx[i], net, 0);
        }
    }
    /* Conflicts: each member against the first earlier member it conflicts with. */
    for (j = 0; j < count; j++) {
        if (pins[idx[j]].type == ERC_NO_CONNECT) {
            continue;
        }
        for (i = 0; i < j; i++) {
            int level;

            if (pins[idx[i]].type == ERC_NO_CONNECT) {
                continue;
            }
            level = map[pins[idx[i]].type][pins[idx[j]].type];
            if (level != 0) {
                found_add(found, ERC_PIN_TO_PIN, idx[j], idx[i], net, level);
                break;
            }
        }
    }
    if (marked) {
        return;
    }
    for (i = 0; i < count; i++) {
        const ErcPin *p = &pins[idx[i]];

        switch (p->type) {
        case ERC_POWER_IN:
            if (first_power_in == SIZE_MAX) {
                first_power_in = idx[i];
            }
            break;
        case ERC_INPUT:
            if (first_input == SIZE_MAX) {
                first_input = idx[i];
            }
            break;
        case ERC_POWER_OUT:
            power_driven = 1;
            input_driven = 1;
            break;
        case ERC_OUTPUT:
        case ERC_BIDIRECTIONAL:
        case ERC_TRI_STATE:
        case ERC_PASSIVE:
            input_driven = 1;
            break;
        default:
            break;
        }
        if (p->drives_power && p->type != ERC_NO_CONNECT) {
            power_driven = 1;
            input_driven = 1;
        }
    }
    if (first_power_in != SIZE_MAX) {
        if (!power_driven) {
            found_add(found, ERC_POWER_PIN_NOT_DRIVEN, first_power_in, first_power_in, net, 0);
        }
    } else if (first_input != SIZE_MAX && !input_driven) {
        found_add(found, ERC_PIN_NOT_DRIVEN, first_input, first_input, net, 0);
    }
}

typedef struct {
    size_t net, pin;
} Slot;

static int compare_slots(const void *a, const void *b) {
    const Slot *x = a, *y = b;

    if (x->net != y->net) {
        return x->net < y->net ? -1 : 1;
    }
    return x->pin < y->pin ? -1 : x->pin > y->pin;
}

static int compare_problems(const void *a, const void *b) {
    const ErcProblem *x = a, *y = b;

    if (x->net != y->net) {
        return x->net < y->net ? -1 : 1;
    }
    if (x->pin != y->pin) {
        return x->pin < y->pin ? -1 : 1;
    }
    return x->kind < y->kind ? -1 : x->kind > y->kind;
}

size_t erc_check(const ErcPin *pins, size_t pin_count, size_t net_count, const ErcNet *nets,
                 const uint8_t pin_map[ERC_PIN_TYPE_COUNT][ERC_PIN_TYPE_COUNT],
                 ErcProblem *out, size_t cap) {
    Found found = {NULL, 0, 0, 0};
    Slot *slots = logic_malloc((pin_count ? pin_count : 1) * sizeof(*slots));
    size_t *group = logic_malloc((pin_count ? pin_count : 1) * sizeof(*group));
    size_t i, start, wired = 0, total;

    if (slots == NULL || group == NULL) {
        logic_free(slots);
        logic_free(group);
        return ERC_CHECK_OUT_OF_MEMORY;
    }
    for (i = 0; i < pin_count; i++) {
        if (pins[i].net == SIZE_MAX) {
            size_t one = i;

            check_net(&found, pins, &one, 1, SIZE_MAX, 0, pin_map);
        } else {
            slots[wired].net = pins[i].net;
            slots[wired].pin = i;
            wired++;
        }
    }
    qsort(slots, wired, sizeof(*slots), compare_slots);
    for (start = 0; start < wired;) {
        size_t end = start, n = 0;

        while (end < wired && slots[end].net == slots[start].net) {
            group[n++] = slots[end].pin;
            end++;
        }
        check_net(&found, pins, group, n, slots[start].net,
                  nets != NULL && slots[start].net < net_count && nets[slots[start].net].has_attachment,
                  pin_map);
        start = end;
    }
    logic_free(slots);
    logic_free(group);
    if (found.out_of_memory) {
        logic_free(found.items);
        return ERC_CHECK_OUT_OF_MEMORY;
    }
    if (found.count > 1) {
        qsort(found.items, found.count, sizeof(*found.items), compare_problems);
    }
    total = found.count;
    for (i = 0; i < total && i < cap && out != NULL; i++) {
        out[i] = found.items[i];
    }
    logic_free(found.items);
    return total;
}
