#ifndef PARTS_H
#define PARTS_H

/*
 * The part library: every orderable part, loaded from the JSON part files
 * (schematical/schematical/Parts/<kind>.json, format in docs/parts-format.md).
 *
 *     PartLibrary *library = part_library_create();
 *     char *error = NULL;
 *     if (load_parts(library, json_text, "gate.json", &error) != 0) {
 *         ... show error; free(error);
 *     }
 *     const Part *part = part_library_find(library, "SN74HC00N");
 *
 * Everything a Part points to is owned by the library and lives until
 * part_library_destroy.
 */

#include <stddef.h>
#include <stdint.h>

typedef struct PartLibrary PartLibrary;

typedef enum {
    PART_GATE, PART_DFF, PART_JKFF, PART_SRAM, PART_SWITCH,
    PART_LED, PART_7SEG, PART_OSCILLATOR, PART_SUPPLY, PART_EEPROM, PART_LATCH, PART_COUNTER,
    PART_SHIFT, PART_CPU
} PartKind;

/* PIN_ABSENT: a numbered position with no lead. Ball pinouts never use it. */
typedef enum {
    PIN_INPUT, PIN_OUTPUT, PIN_INOUT, PIN_POWER, PIN_GROUND, PIN_NC, PIN_ABSENT
} PinRole;

enum { PART_OBSOLETE = 1u << 0, PART_MILITARY = 1u << 1,
       PART_AUTOMOTIVE = 1u << 2, PART_ENHANCED = 1u << 3,
       PART_CERAMIC = 1u << 4 };

/* A pin's position in Part.pins is its engine pin index. */
typedef struct {
    const char *designator;  /* the package's label: "14" (numbered) or "A1" (ball) */
    const char *name; PinRole role; const char *group;
} PartPin;

/* Exactly one of these is set on a Part, matching its kind. */
typedef struct { const char *family; unsigned units; } PartLogic;         /* gate, dff, jkff */
typedef struct { uint32_t words; unsigned bits; uint64_t access_ps; const char *grade; } PartSram;
/* rated_mv: the contacts' switching voltage rating. */
typedef struct { unsigned positions; const char *actuator; uint32_t rated_mv; } PartSwitch;
typedef struct { const char *color; } PartLed;
typedef struct { unsigned digit; const char *segment; } PartDisplayLed;  /* digit 0 = colon/dot */
typedef struct {
    unsigned digits; int common_anode; unsigned colon_after, dot_after;
    const char *color, *size, *chip;
    const PartDisplayLed *leds; size_t led_count;
    const size_t *segment_pins; size_t segment_pin_count;  /* indices into Part.pins */
    const size_t *common_pins; size_t common_pin_count;    /* indices into Part.pins */
    uint32_t vf_min_mv, vf_max_mv;   /* segment forward voltage (shown, not checked) */
} PartDisplay;
typedef struct { const char *family; uint64_t min_hz, max_hz, default_hz;
                 const char *enable_pin; /* the pin's designator, NULL = none */ } PartOscillator;
/* mv: the supply's voltage; level = mv > 0 (drives logic 1, else 0).
   pull: 1 for a pull-up/pull-down part (drives weakly; "pull": true). */
typedef struct { int level; uint32_t mv; int pull; } PartSupply;
/* The recommended supply range (millivolts) and the highest voltage an input
   takes: input_max_mv = the part's "inputMax", or max_mv when it has none. */
typedef struct { uint32_t min_mv, max_mv, input_max_mv; } PartVoltage;
/* page_bytes: the page-load size; write_cycle_ps: tWC, the internal write time. */
typedef struct { uint32_t words; unsigned bits; uint64_t access_ps; const char *grade;
                 unsigned page_bytes; uint64_t write_cycle_ps; } PartEeprom;
/* core: "6502", "65c02" or "z80". max_hz = floor(1e12 / the grade's clock period in ps). */
typedef struct { const char *core; const char *grade; uint64_t max_hz; } PartCpu;

/* ---- Footprints (Parts/footprints.json, docs/parts-format.md) ----------- */

typedef enum { PAD_RECT, PAD_ROUND } PadShape;

/* Corners of a rect pad, as seen on the board (y down): a bit set. */
enum { PAD_CORNER_TL = 1, PAD_CORNER_TR = 2, PAD_CORNER_BR = 4, PAD_CORNER_BL = 8 };

/* One land: millimetres, origin = body centre, x right, y down. */
typedef struct {
    const char *designator;  /* "14" or "A1", matching PartPin.designator; NULL = unnumbered centre pad */
    double x, y;             /* centre */
    double width, height;
    double drill;            /* 0 = surface mount */
    PadShape shape;
    double radius;           /* rect only: every corner rounded by this radius; 0 = square */
    double chamfer;          /* rect only: `corners` cut at 45 deg, this far along each edge; 0 = none */
    unsigned corners;        /* PAD_CORNER_* bits the chamfer cuts */
} FootprintPad;

/* A footprint, expanded to a flat pad list by the loader. */
typedef struct {
    const char *id, *description;
    unsigned pins;                      /* designated pads */
    double body_width, body_height;
    int body_round;
    const FootprintPad *pads; size_t pad_count;   /* "1"…"N" order (grid: row-major), unnumbered last */
} PartFootprint;

typedef struct Part {
    PartKind kind;
    const char *name, *package, *function, *description;
    const char *const *drawer; size_t drawer_count;
    unsigned flags;
    const PartPin *pins; size_t pin_count;
    const PartLogic *logic;
    const PartSram *sram;
    const PartSwitch *dip_switch;
    const PartLed *led;
    const PartDisplay *display;
    const PartOscillator *oscillator;
    const PartSupply *supply;
    const PartVoltage *vcc;            /* recommended supply range; NULL only for supplies,
                                          LEDs, 7-seg displays and switches */
    const PartEeprom *eeprom;
    const PartCpu *cpu;
    const PartFootprint *footprint;    /* NULL = none (supplies) */
    const void *internal;              /* model data; not for the app */
} Part;

PartLibrary *part_library_create(void);
void part_library_destroy(PartLibrary *library);

/* Parses one part file's text. 0 on success; -1 and a malloc'd message in
   *error (free it) on failure, with nothing from that file added. `source`
   names the file in messages, e.g. "gate.json". */
int load_parts(PartLibrary *library, const char *json, const char *source, char **error);

/*
 * Parses a footprints file (format "schematical-footprints"). Same contract
 * as load_parts. Load it before any part file that names its footprints;
 * ids are unique across calls.
 */
int load_footprints(PartLibrary *library, const char *json, const char *source, char **error);

size_t part_library_footprint_count(const PartLibrary *library);
const PartFootprint *part_library_footprint_at(const PartLibrary *library, size_t index);
const PartFootprint *part_library_find_footprint(const PartLibrary *library, const char *id);

size_t part_library_count(const PartLibrary *library);
const Part *part_library_at(const PartLibrary *library, size_t index);
const Part *part_library_find(const PartLibrary *library, const char *name);

#endif
