#ifndef LCD_STATE_H
#define LCD_STATE_H

/*
 * A character LCD module's state as the simulation sees it (plan-11-B
 * section 4.7), read with circuit_lcd_state (circuit.h): the dot image the
 * display shows now, plus the RAM and busy flag for tests.
 */

#include <stdint.h>

#define LCD_STATE_MAX_COLUMNS 40
#define LCD_STATE_MAX_ROWS 2
#define LCD_STATE_CELL_WIDTH 5
#define LCD_STATE_CELL_HEIGHT 8
enum { LCD_DOT_OFF = 0, LCD_DOT_ON = 1, LCD_DOT_X = 2 };

typedef struct LcdState {
    unsigned columns, rows;               /* the part's, from PartCharLcd */
    int busy;                             /* 1, 0, or -1 for X */
    uint8_t ddram[80], ddram_known[80];   /* by physical index; known: 1 bit per defined bit */
    uint8_t cgram[64], cgram_known[64];
    /* dots[y][x]: y = row * 8 + dot row, x = column * 5 + dot column. Dots
       outside columns x rows are LCD_DOT_OFF. */
    uint8_t dots[LCD_STATE_MAX_ROWS * LCD_STATE_CELL_HEIGHT]
                [LCD_STATE_MAX_COLUMNS * LCD_STATE_CELL_WIDTH];
} LcdState;

#endif
