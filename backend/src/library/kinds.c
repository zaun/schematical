#include <string.h>

#include "kind.h"

extern const PartKindOps GATE_KIND;
extern const PartKindOps SUPPLY_KIND;
extern const PartKindOps LED_KIND;
extern const PartKindOps OSCILLATOR_KIND;

/* One line per kind. Adding a kind: docs/backend.md, "Adding a kind". */
const PartKindOps *const KINDS[] = {
    &GATE_KIND,
    &SUPPLY_KIND,
    &LED_KIND,
    &OSCILLATOR_KIND,
    NULL
};

const size_t KIND_COUNT = sizeof(KINDS) / sizeof(KINDS[0]) - 1;

const PartKindOps *kind_find(const char *name) {
    size_t i;

    for (i = 0; i < KIND_COUNT; i++) {
        if (strcmp(KINDS[i]->name, name) == 0) {
            return KINDS[i];
        }
    }
    return NULL;
}
