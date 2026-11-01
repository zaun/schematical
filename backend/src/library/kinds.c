#include <string.h>

#include "kind.h"

/* One line per kind. Adding a kind: docs/backend.md, "Adding a kind". */
const PartKindOps *const KINDS[] = {
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
