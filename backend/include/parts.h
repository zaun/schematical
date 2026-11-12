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
    PART_SHIFT, PART_CPU, PART_CHARLCD, PART_SERIAL, PART_CONNECTOR, PART_PIO, PART_DECODER,
    PART_BUFFER, PART_MUX, PART_HEADER, PART_ARITH, PART_CONVERTER, PART_RTC, PART_DIODE
} PartKind;

/* PIN_ABSENT: a numbered position with no lead. Ball pinouts never use it. */
typedef enum {
    PIN_INPUT, PIN_OUTPUT, PIN_INOUT, PIN_POWER, PIN_GROUND, PIN_NC, PIN_ABSENT
} PinRole;

enum { PART_OBSOLETE = 1u << 0, PART_MILITARY = 1u << 1,
       PART_AUTOMOTIVE = 1u << 2, PART_ENHANCED = 1u << 3,
       PART_CERAMIC = 1u << 4 };

/* The block side a pin is drawn on: PIN_SIDE_AUTO follows its role (inputs
 * left; outputs and inouts right); "side" in the part file picks one. */
typedef enum { PIN_SIDE_AUTO, PIN_SIDE_IN, PIN_SIDE_OUT } PinSide;

/* A pin's position in Part.pins is its engine pin index. */
typedef struct {
    const char *designator;  /* the package's label: "14" (numbered) or "A1" (ball) */
    const char *name; PinRole role; const char *group; PinSide side;
} PartPin;

/* Exactly one of these is set on a Part, matching its kind. */
typedef struct { const char *family; unsigned units; } PartLogic;         /* gate, dff, jkff */
typedef struct { uint32_t words; unsigned bits; uint64_t access_ps; const char *grade; } PartSram;
/* rated_mv: the contacts' switching voltage rating. momentary: the button
   springs back (data for the app). normally_closed: positions start closed.
   joined: pairs of pins always connected inside the part (contacts numbered
   positions + 1 … positions + joined, not addressable). body_um: the
   nominal body, length then width (length >= width), in micrometres; both
   0 when the part file gives no "bodySize". */
typedef struct {
    unsigned positions; const char *actuator; uint32_t rated_mv;
    unsigned momentary; unsigned normally_closed; unsigned joined;
    uint32_t body_um[2];
} PartSwitch;
/* colors: the LED's colors, in file order; colors[0] is the default. */
typedef struct { const char *const *colors; size_t color_count; } PartLed;
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
/* Character LCD module (charlcd). panel: "yellow-green" (STN positive: dark
   dots on yellow-green) or "blue-negative" (STN negative: light dots on
   blue). backlight_color: NULL when the module has no backlight. */
typedef struct {
    unsigned columns, rows;
    const char *controller, *rom, *panel, *backlight_color;
    const size_t *bus_pins; size_t bus_pin_count;  /* RS, R/W, E, DB0-DB7: indices into Part.pins */
} PartCharLcd;
/* model: the serial model ("ns16550", "i8251", ...); channels: serial
   channels on the chip; grade: the part's speed/supply grade; host: the
   host-side interface, from the device's "bus" style: "spi", "i2c",
   "i2c-spi" (either, picked by a pin), "usb" (a USB-to-UART bridge), or
   "parallel" (any other bus). */
typedef struct { const char *model; unsigned channels; const char *grade; const char *host; } PartSerial;
/* A barrel or USB connector. usb: 'A', 'B' or 'C' for a USB connector, 0
   for a barrel jack. center_diameter_um is a barrel's mating plug centre-pin
   diameter (0 for USB); contacts is the number of normally-closed contacts
   (0 for USB). rated_mv / rated_ma: 0 = not given (USB only). */
typedef struct { uint32_t center_diameter_um, rated_mv, rated_ma; unsigned contacts; char usb; } PartConnector;
/* A supply domain of a dual-supply part (plan-20-F): the pins powered from
   one supply. power_pins: the supply's pins (one name, possibly several
   pins, e.g. two VCCB); pins: every other pin of the domain (not ground or
   NC); vcc: the domain's range and input max. Indices are into Part.pins. */
typedef struct {
    const size_t *power_pins; size_t power_pin_count;
    const PartVoltage *vcc;
    const size_t *pins; size_t pin_count;
} PartSupplyDomain;
/* Parallel I/O peripheral (plan-14): model "pia6521" or "via6522"; grade:
   the part's speed grade; max_hz = floor(1e12 / the grade's tcyc in ps). */
typedef struct { const char *model; const char *grade; uint64_t max_hz; } PartPio;
/* A generic pin header (plan-23-A): rows is 1 or 2; genders are the
   genders a placed header may have, genders[0] the default (app only). */
typedef struct { unsigned rows; const char *const *genders; size_t gender_count; } PartHeader;
/* An ADC or DAC (plan-24-D): type "adc" or "dac"; bits: resolution; channels:
   ADC inputs / DAC outputs. Only the digital side is simulated. */
typedef struct { const char *type; unsigned bits, channels; } PartConverter;
/* A diode part (plan-29-H): type "rectifier", "fast", "switching",
   "schottky", "zener" or "bridge"; diode_count: the device's (anode,
   cathode) pairs. vr_mv / io_ma: max repetitive reverse voltage and average
   forward current per diode (0 for a zener); vz_mv / pd_mw: nominal Zener
   voltage and max power dissipation (zener only, else 0). */
typedef struct { const char *type; size_t diode_count;
                 uint32_t vr_mv, io_ma, vz_mv, pd_mw; } PartDiode;

/* ---- Footprints (Parts/footprints.json, docs/parts-format.md) ----------- */

typedef enum { PAD_RECT, PAD_ROUND } PadShape;

/* Corners of a rect pad, as seen on the board (y down): a bit set. */
enum { PAD_CORNER_TL = 1, PAD_CORNER_TR = 2, PAD_CORNER_BR = 4, PAD_CORNER_BL = 8 };

/* One land: millimetres, origin = body centre, x right, y down. */
typedef struct {
    const char *designator;  /* "14" or "A1", matching PartPin.designator; NULL = unnumbered centre pad */
    double x, y;             /* centre */
    double width, height;
    double drill;            /* 0 = surface mount; a slot's width */
    double slot_length;      /* explicit only: a slotted hole's length along the pad's long side; 0 = round hole */
    int npth;                /* explicit only: 1 = a non-plated hole, no copper (the pad is the hole itself) */
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
    const FootprintPad *pads; size_t pad_count;   /* "1"…"N" order (grid: row-major); a pin with several pads (explicit only) lists them together, in file order; unnumbered last */
} PartFootprint;

/* One entry of a part's "footprintChoices": a package name and its footprint. */
typedef struct { const char *package; const PartFootprint *footprint; } PartFootprintChoice;

/* ---- Datasheets (Parts/datasheets.json, docs/parts-format.md) --------- */

/* One datasheet: an id parts name, the manufacturer's URL of the PDF, and
   the app's own Markdown datasheet ("md": a path under the bundled
   Datasheets folder, such as "gate-and/sn74hc08.md"; NULL = none). */
typedef struct { const char *id, *url, *md; } PartDatasheet;

typedef struct Part {
    PartKind kind;
    const char *name;
    const char *uuid;                  /* a single part file's "uuid" (load_part_file);
                                          NULL for every part of a load_parts file */
    const char *package, *function, *description;
    const char *const *drawer; size_t drawer_count;
    unsigned flags;
    const PartPin *pins; size_t pin_count;
    const PartLogic *logic;
    const PartSram *sram;
    const PartSwitch *dip_switch;
    const PartLed *led;
    const PartDiode *diode;
    const PartDisplay *display;
    const PartOscillator *oscillator;
    const PartSupply *supply;
    const PartVoltage *vcc;            /* recommended supply range (character LCDs have
                                          one); NULL only for supplies,
                                          LEDs, 7-seg displays and switches */
    const PartEeprom *eeprom;
    const PartCpu *cpu;
    const PartSerial *serial;
    const char *manufacturer;          /* NULL = not given; "Generic" = no one maker */
    const PartDatasheet *datasheet;    /* NULL = none (always for "Generic") */
    const PartFootprint *footprint;    /* NULL = none (supplies) */
    const PartFootprint *connector_footprint;  /* NULL = none; the PCB Style option's "Connector Only" */
    /* The footprints a placed part may use instead, in file order; one of
       them is `footprint` (the default). 0 = only footprint. */
    const PartFootprintChoice *footprint_choices; size_t footprint_choice_count;
    const PartCharLcd *charlcd;
    const PartConnector *connector;
    const PartPio *pio;
    const PartHeader *header;
    const PartConverter *converter;
    /* Dual-supply parts (plan-20-F): domain 0 is the A side (its vcc is
       Part.vcc), domain 1 the B side; NULL/0 for single-supply parts. */
    const PartSupplyDomain *domains; size_t domain_count;
    const void *internal;              /* model data; not for the app */
} Part;

PartLibrary *part_library_create(void);
void part_library_destroy(PartLibrary *library);

/* Parses one part file's text. 0 on success; -1 and a malloc'd message in
   *error (free it) on failure, with nothing from that file added. `source`
   names the file in messages, e.g. "gate.json". */
int load_parts(PartLibrary *library, const char *json, const char *source, char **error);

/*
 * Parses a single part file (format "schematical-part", docs/parts-format.md
 * § Single part files): one device with one part, a required "uuid", and
 * optional inline "footprints" in the footprints.json entry format. Same
 * contract as load_parts: on failure nothing from the file (no footprint, no
 * part) is added. Load footprints.json and datasheets.json first; the part's
 * footprint may name a bundled id or an inline one.
 */
int load_part_file(PartLibrary *library, const char *json, const char *source, char **error);

/*
 * Takes back the latest load_part_file: its part, name, uuid and inline
 * footprints leave the library and its memory is freed, so a checker can
 * load one draft after another into one library (the app's part editor).
 * Only while nothing else was loaded since: 0, or -1 (nothing changed) when
 * there is no such load. Pointers into that part are invalid afterwards.
 */
int part_library_unload_last_part_file(PartLibrary *library);

/*
 * Parses a footprints file (format "schematical-footprints"). Same contract
 * as load_parts. Load it before any part file that names its footprints;
 * ids are unique across calls.
 */
int load_footprints(PartLibrary *library, const char *json, const char *source, char **error);

/*
 * Parses a datasheets file (format "schematical-datasheets"). Same contract
 * as load_parts. Load it before any part file that names its datasheet;
 * ids are unique across calls.
 */
int load_datasheets(PartLibrary *library, const char *json, const char *source, char **error);

size_t part_library_datasheet_count(const PartLibrary *library);
const PartDatasheet *part_library_datasheet_at(const PartLibrary *library, size_t index);
const PartDatasheet *part_library_find_datasheet(const PartLibrary *library, const char *id);

size_t part_library_footprint_count(const PartLibrary *library);
const PartFootprint *part_library_footprint_at(const PartLibrary *library, size_t index);
const PartFootprint *part_library_find_footprint(const PartLibrary *library, const char *id);

size_t part_library_count(const PartLibrary *library);
const Part *part_library_at(const PartLibrary *library, size_t index);
const Part *part_library_find(const PartLibrary *library, const char *name);
/* The part load_part_file loaded with this "uuid"; NULL if none. */
const Part *part_library_find_uuid(const PartLibrary *library, const char *uuid);

#endif
