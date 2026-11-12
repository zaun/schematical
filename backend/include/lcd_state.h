#ifndef LCD_STATE_H
#define LCD_STATE_H

/*
 * A character LCD module's state as the simulation sees it (plan-11-B
 * section 4.7), read with circuit_lcd_state (circuit.h): the dot image the
 * display shows now, plus each controller's RAM and busy flag for tests. A
 * 40x4 module has two controllers (plan-35-D): [0] behind E1 (rows 0-1),
 * [1] behind E2 (rows 2-3); every other module has one, [0].
 */

#include <stdint.h>

#define LCD_STATE_MAX_COLUMNS 40
#define LCD_STATE_MAX_ROWS 4
#define LCD_STATE_MAX_CONTROLLERS 2
#define LCD_STATE_CELL_WIDTH 5
#define LCD_STATE_CELL_HEIGHT 8
enum { LCD_DOT_OFF = 0, LCD_DOT_ON = 1, LCD_DOT_X = 2 };

typedef struct LcdState {
    unsigned columns, rows;               /* the part's, from PartCharLcd */
    unsigned controllers;                 /* 1 or 2 (PartCharLcd.enables); the arrays' used length */
    int busy[LCD_STATE_MAX_CONTROLLERS];  /* 1, 0, or -1 for X */
    /* By physical index; known: 1 bit per defined bit. */
    uint8_t ddram[LCD_STATE_MAX_CONTROLLERS][80], ddram_known[LCD_STATE_MAX_CONTROLLERS][80];
    uint8_t cgram[LCD_STATE_MAX_CONTROLLERS][64], cgram_known[LCD_STATE_MAX_CONTROLLERS][64];
    /* dots[y][x]: y = row * 8 + dot row, x = column * 5 + dot column. Dots
       outside columns x rows are LCD_DOT_OFF. */
    uint8_t dots[LCD_STATE_MAX_ROWS * LCD_STATE_CELL_HEIGHT]
                [LCD_STATE_MAX_COLUMNS * LCD_STATE_CELL_WIDTH];
} LcdState;

#endif
