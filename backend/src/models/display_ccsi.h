#ifndef DISPLAY_CCSI_H
#define DISPLAY_CCSI_H

/* The ccsi display driver (plan-56-D, TLC6983): display_ccsi.c, run from display_out.c. */

#include "converter_int.h"
#include "display_out.h"

struct CcsiState;

struct CcsiState *ccsi_new(void);
void ccsi_free(struct CcsiState *c);
unsigned ccsi_lines(const struct CcsiState *c);
void ccsi_pin(CvState *s, const CcsiCfg *k, struct CcsiState *c, size_t pin);
void ccsi_apply(CvState *s, const CcsiCfg *k, const struct CcsiState *c, unsigned slot);

#endif
