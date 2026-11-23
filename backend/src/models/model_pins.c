/*
 * Pin facts the voltage check asks of every part, whatever its kind
 * (model_pins.h). plan-47-I: the fixed contacts of inductors and fuses.
 */

#include "model_pins.h"

int part_fixed_contact(const Part *part, size_t k, size_t *a, size_t *b) {
    if (part == NULL) {
        return 0;
    }
    switch (part->kind) {
        case PART_INDUCTOR:
            return inductor_fixed_contact(part, k, a, b);
        case PART_FUSE:
            return fuse_fixed_contact(part, k, a, b);
        default:
            return 0;
    }
}
