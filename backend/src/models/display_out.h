#ifndef DISPLAY_OUT_H
#define DISPLAY_OUT_H

/*
 * The display output drivers of the leddriver and lcddriver kinds
 * (plan-56-B): the converter engine runs the bus and the registers, and the
 * device's "display" section names one output driver that turns register
 * contents into pin levels, on the engine's registers and on timers of
 * simulated time. docs/parts-format.md § leddriver, § lcddriver.
 *
 *   digits   a multiplexed digit scan: one digit pin active at a time, the
 *            segment pins carry that digit's byte (raw or decoded)
 *   static   every segment pin follows its digit's byte at all times
 *   matrix   rows scanned one at a time, the column pins carry the row's bits
 *   ports    pins driven (or released) per a direction and a data field
 *   lcd      backplane and segment waveforms from a display RAM
 */

#include "converter.h"

typedef enum { DISP_DIGITS, DISP_STATIC, DISP_MATRIX, DISP_PORTS, DISP_LCD } DispDriver;

typedef enum { DEC_NONE, DEC_HEX, DEC_CODEB, DEC_BCD, DEC_ASCII7, DEC_ASCII16 } DispDecode;

/* One group of a static driver: its output pins and the field that feeds them. */
typedef struct {
    const int *pins;            /* name ids */
    size_t count;
    CvField ram;
    int has_decode;
    CvField decode;             /* decodes while this field is nonzero (decode_active 1) */
} DispGroup;

/* One port of a ports driver. */
typedef struct {
    int pin;
    int has_dir;
    CvField dir;
    const uint32_t *outputs;    /* dir values that make the pin an output */
    size_t output_count;
    CvField data;
    int invert;
    /* plan-56-D (MAX6957): dir values that make the pin an open-drain LED sink (drives 0 while the
       data field is nonzero, else released), dir values that make it a pulled-up input (a weak 1),
       and the field that reads back as the port's register bit: the data bit while the pin is an
       output or a sink, the pin's level while it is an input. */
    const uint32_t *sinks;
    size_t sink_count;
    const uint32_t *pullups;
    size_t pullup_count;
    int has_read;
    CvField read;
} DispPort;

/* lcd run-time mode (plan-56-E): the mode is the value of a register field (PCF8576 mode-set M[1:0]). */
enum { DISP_LCD_MODES = 4, DISP_LCD_ROWS = 4 };

typedef struct DispCfg {
    DispDriver driver;
    /* digits (digit pins), matrix (row pins): the scan lines, one per ram field */
    const int *lines;
    size_t line_count;
    int line_active;            /* the level that selects a line */
    /* digits, matrix: the data pins carried while a line is selected; static: not used */
    const int *outs;
    size_t out_count;
    const uint8_t *out_bits;    /* per data pin: the bit of the ram value it follows */
    int out_active;             /* the level of a lit segment / set column */
    /* digits, matrix: one field per line; lcd: the display RAM as a bit stream */
    const CvField *ram;
    size_t ram_count;
    /* decode (digits, static) */
    DispDecode decode;
    int has_decode_field;
    const CvField *decode_field;    /* 1 (global) or one per digit */
    size_t decode_field_count;
    int decode_active;          /* decode while the field is nonzero (1) / zero (0) */
    unsigned dp_bit;            /* the bit of a decoded value that lights the decimal point */
    /* static groups, ports */
    const DispGroup *groups;
    size_t group_count;
    const DispPort *ports;
    size_t port_count;
    /* scan: the time one slot lasts */
    uint64_t slot_ps;
    /* conditions on the registers; count 0 = none given */
    CvConds blank, test, shutdown, enable;
    /* lcd */
    unsigned bp_count;          /* backplane pins in `lines`, 1-4 */
    unsigned segments;          /* segment pins in `outs` */
    const uint16_t *bit_field;  /* per RAM bit index: the ram field it lives in */
    const uint8_t *bit_pos;     /*                   and its bit position there */
    int bp_major;               /* RAM order: 1 backplane by backplane, 0 segment by segment */
    /* digits (plan-56-D, MAX6950/51): the digit pins are also the data pins. `lines` holds the
       line_count shared pins; digit k (k < shared_digits) selects shared pin k at line_active, the
       other pins carry the digit's 8 segments in order (seg_of_line[j] = the segment, a-g = 0-6
       and the point = 7, that line j shows); a lit segment drives out_active, an unlit one is
       released */
    int shared;
    unsigned shared_digits;
    uint8_t seg_of_line[8];
    /* digits: the number of digits scanned is the field + 1 (test scans all) */
    int has_limit;
    CvField limit;
    /* digits: a second RAM plane (ram1, one field per digit) shown instead of `ram` in the second
       half of each blink period while blink_enable holds; the half period (in us) is a choice */
    int has_blink;
    CvConds blink_enable;
    const CvField *ram1;
    CvChoice blink_half;
    uint64_t frame_ps;          /* the frame time: a slot lasts frame_ps / the mode's slot count */
    /* lcd, mode chosen by a field: up to 4 modes, each with its slot count (1-4) and, per slot, the
       mask of the backplane lines (bit b = lines[b]) driven in it; the others are released */
    int mode_dynamic;
    CvField mode_field;
    unsigned mode_count;
    uint32_t mode_key[DISP_LCD_MODES];          /* field value */
    unsigned mode_slots[DISP_LCD_MODES];        /* 1 static, 2 1:2, 3 1:3, 4 1:4 */
    uint8_t slot_mask[DISP_LCD_MODES][DISP_LCD_ROWS];
    unsigned mode_step[DISP_LCD_MODES];         /* load: the data pointer's increment per byte */
    int has_out_bank;
    CvField out_bank;           /* nonzero in a mode of at most 2 slots: slot k shows RAM row k + 2 */
    /* lcd "load": display data bytes written through the data pointer */
    int has_load;
    CvField ld_pointer;
    int has_in_bank;
    CvField ld_bank;            /* nonzero in a mode of at most 2 slots: rows + 2 receive the bits */
    int has_sub;
    CvField ld_sub, ld_hw;      /* storing needs the subaddress counter equal to the hardware address */
    unsigned ld_subs;           /* the counter's modulus */
    /* pins that drive 0 or release (never 1) */
    const int *open_drain;
    size_t open_drain_count;
} DispCfg;

/* The engine's hooks (disp_start and the others) are declared in converter_int.h. */

#endif
