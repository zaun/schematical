#ifndef DISPLAY_CLOCK_H
#define DISPLAY_CLOCK_H

/* The clock display driver (plan-56-F, PCF1179C): display_clock.c, run from display_out.c. */

#include "converter_int.h"
#include "display_out.h"

struct ClockState;

struct ClockState *clock_new(CvState *s, const ClockCfg *k);
void clock_free(struct ClockState *c);
void clock_pin(CvState *s, const ClockCfg *k, struct ClockState *c, size_t pin);
int clock_timer(CvState *s, const ClockCfg *k, struct ClockState *c, uint32_t id);
int clock_lit(const CvState *s, const ClockCfg *k, const struct ClockState *c, unsigned seg,
              unsigned bp);

#endif
