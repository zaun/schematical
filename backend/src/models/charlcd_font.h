#ifndef CHARLCD_FONT_H
#define CHARLCD_FONT_H

/* Character ROM A00 (charlcd_font.c): [code][row], 5 dots, bit 4 = left. */

#include <stdint.h>

extern const uint8_t CHARLCD_ROM_A00[256][8];

#endif
