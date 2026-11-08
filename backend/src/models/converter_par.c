/*
 * Parallel and clocked buses of the converter kind: plan-24-E. Until then the
 * hooks do nothing.
 */

#include "converter_int.h"

void cv_par_start(CvState *s) {
    (void)s;
}

void cv_par_pin(CvState *s, size_t pin) {
    (void)s;
    (void)pin;
}

void cv_par_timer(CvState *s, uint32_t id) {
    (void)s;
    (void)id;
}
