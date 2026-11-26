/* models/charlcd_int.h: what the pieces of models/charlcd.c share (plan-68-I/J split it). */
#ifndef MODELS_CHARLCD_INT_H
#define MODELS_CHARLCD_INT_H

/*
 * charlcd: HD44780-compatible character LCD modules (plan-11-B). Each module
 * has its controller, character ROM, glass and backlight.
 *
 * Loader: the device keys of docs/parts-format.md § charlcd.
 *
 * Model (plan-11-B sections 4.3-4.7): the 8- and 4-bit bus on E edges, the
 * instruction set, the busy flag and execution times, power-on reset, the
 * bus timing checks and the X rules (an access whose effect is unknown makes
 * everything it could change X; "LOST" is every field X). The backend
 * renders the dot image (circuit_lcd_state); each backlight LED is a light.
 * plan-35-D: 4-row modules (one controller's two 40-cell lines, each split
 * in two rows) and 40x4 modules with two controllers on one bus, E1 for
 * rows 0-1 and E2 for rows 2-3, each with its own state.
 * Not modeled: V0 and contrast, the 5x10 font (F is stored), ROM A02, tR/tF,
 * serial interfaces.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "charlcd_font.h"
#include "kind.h"
#include "lcd_state.h"
#include "lights.h"
#include "model_pins.h"

#define LCD_DATA_BITS 8
#define LCD_MAX_BUS_PINS (4 + LCD_DATA_BITS)   /* RS, R/W, E1, E2, DB0-DB7 */
#define LCD_MAX_ENABLES 2
#define LCD_MAX_LEDS 3

/* Bus timing and execution times, in picoseconds. */
typedef struct {
    SimTime tc, tpw, tas, tah, tdsw, th, tddr, tdhr;
    SimTime instruction, data, clear_home, power_on;
} CharLcdTiming;

typedef struct {
    unsigned columns, rows, enables;
    const char *controller, *rom, *panel, *backlight_color;
    const char *rs, *rw, *e[LCD_MAX_ENABLES];   /* e[1]: NULL with one enable */
    const char *const *data;
    const char *contrast;
    size_t leds;                                 /* backlight LEDs, 0-3 */
    const char *anode[LCD_MAX_LEDS], *cathode[LCD_MAX_LEDS];
    const char *colors[LCD_MAX_LEDS];
    CharLcdTiming t;
    const PartVoltage *vcc;
} CharLcdDevice;

/* Pin indices for one pinout. */
typedef struct {
    size_t rs, rw, e[LCD_MAX_ENABLES], db[LCD_DATA_BITS], contrast;
    unsigned enables;
    size_t anode[LCD_MAX_LEDS], cathode[LCD_MAX_LEDS];
    size_t leds;
    size_t bus[LCD_MAX_BUS_PINS];
    PartCharLcd info;
    const CharLcdDevice *device;
} CharLcdPins;

/* ---- Model ---------------------------------------------------------------- */

/*
 * One state machine driven by E edges, input changes and timers. Registers
 * are tracked one field at a time, each known or X (-1). Both bus widths
 * share one byte path; 4-bit mode only assembles nibbles in front of it.
 */

#define LCD_NEVER UINT64_MAX
#define LCD_BLINK_PS 379259259ull   /* 409.6 ms x 250 kHz / 270 kHz */

/* A timer id: the kind in bits 0-1, the access's seq in bits 2-30, the
   controller in bit 31. */
enum { TIMER_READ_DATA, TIMER_READ_OFF, TIMER_WRITE };
#define TIMER_SEQ_MASK 0x1FFFFFFFu

/* One controller: an HD44780 with its own registers, RAM, busy flag,
   enable pin and access record (a 40x4 has two on one bus). */
typedef struct {
    /* Registers: 0/1 (or a value), -1 = X. */
    int dl, n, f, d, c, b, id, s;
    int ac;                  /* 0-127 */
    int ram;                 /* 0 DDRAM, 1 CGRAM */
    int shift;               /* 0-79 */
    int phase;               /* 4-bit: 0 = the high nibble comes next, 1 = the low */
    uint8_t high, high_known;   /* a 4-bit write's first nibble */
    int high_violated;
    SimTime busy_until;
    SimTime bf_x_until;      /* BF reads X before this (after LOST) */
    uint8_t ddram[80], ddram_known[80];
    uint8_t cgram[64], cgram_known[64];

    /* Its enable. */
    int e;                   /* E as last seen: 0, 1, -1 */
    SimTime rise_time, prev_rise;
    int have_rise;

    /* The access in progress (E high). */
    int acc_rs, acc_rw, acc_violated;
    uint32_t seq;            /* counts this controller's accesses; stale timers carry an old one */

    /* Read drive. */
    int driving, drive_x;
    uint8_t drive_mask, read_value, read_known;
    uint8_t read_byte, read_byte_known;   /* 4-bit: the byte whose nibbles a read returns */
    uint8_t out_mask, out_value, out_known;   /* what it drives on DB now (out_mask 0: nothing) */

    /* A write waiting out its hold window. */
    int pending, pend_rs, pend_violated;
    SimTime pend_fall;
    uint8_t pend_value, pend_known, pend_mask;
} CharLcdCtl;

/* The module: the shared bus inputs, the controllers and the backlight. */
typedef struct {
    CharLcdPins pins;
    const CharLcdTiming *t;

    /* Shared inputs: RS, R/W and DB change times. */
    SimTime rs_time, rw_time, db_time[LCD_DATA_BITS];

    CharLcdCtl ctl[LCD_MAX_ENABLES];   /* ctl[1] only with two enables */
    Light light[LCD_MAX_LEDS];         /* one per backlight LED */
} CharLcdState;

extern const char *const clcd_BUS_KEYS[5];
extern const char *const clcd_BACKLIGHT_KEYS[4];
extern const char *const clcd_TIMING_KEYS[9];
extern const char *const clcd_EXEC_KEYS[5];
void *clcd_charlcd_load_device(KindContext *ctx, const cJSON *json);
void *clcd_charlcd_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                 size_t pin_count);
int clcd_charlcd_load_part(KindContext *ctx, const void *raw, const void *bound,
                             const cJSON *json, Part *out);

#endif
