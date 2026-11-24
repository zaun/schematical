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

typedef enum { DEC_NONE, DEC_HEX, DEC_CODEB, DEC_BCD, DEC_ASCII7 } DispDecode;

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
} DispPort;

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
    /* pins that drive 0 or release (never 1) */
    const int *open_drain;
    size_t open_drain_count;
} DispCfg;

/* The engine's hooks (disp_start and the others) are declared in converter_int.h. */

#endif
