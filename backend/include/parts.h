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
    PART_BUFFER, PART_MUX, PART_HEADER, PART_ARITH, PART_CONVERTER, PART_RTC, PART_DIODE,
    PART_CAPACITOR, PART_REGISTER, PART_MCU, PART_RESISTOR, PART_TRANSISTOR,
    PART_GENERIC, PART_SERIALMEM, PART_NIXIE, PART_REGULATOR, PART_CRYSTAL
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
    uint32_t active_low;     /* bit i: alternative i of `name` (split at `/`) is
                                active low; 0 = none */
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
    const char *type;   /* "led", "numitron" (incandescent) or "vfd" (grids as commons) */
} PartDisplay;
/* A nixie tube: one anode (on one or two pins) and one cathode per glyph.
   glyphs: what each cathode shows, in lights order: a numeral ("0"-"9"), a
   symbol ("%", "Hz", ...), "DP" (the right decimal point) or "LDP" (the
   left one). cathode_pins[i] is glyph i's pin. view: "side" (read through
   the envelope's side) or "end" (through its top). strike_mv: the firing
   voltage, the least the anode supply may be (checked); sustain_mv: the
   maintaining voltage; current_*_ua: the cathode current range (shown). */
typedef struct {
    const char *const *glyphs; size_t glyph_count;
    const size_t *cathode_pins;                          /* glyph_count indices into Part.pins */
    const size_t *anode_pins; size_t anode_pin_count;    /* indices into Part.pins */
    const char *color, *size, *view;
    uint32_t strike_mv, sustain_mv;
    uint32_t current_min_ua, current_max_ua;
} PartNixie;
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
/* Character LCD module (charlcd). rows: 1, 2 or 4. panel: "yellow-green"
   (STN positive: dark dots on yellow-green), "blue-negative" (STN negative:
   light dots on blue), "gray" (STN positive: dark dots on light gray) or
   "white" (FSTN positive: black dots on white). backlight_color: the first
   LED's color, NULL when the module has no backlight. enables: 1, or 2 (a
   40x4 with two controllers: E1 drives rows 0-1, E2 rows 2-3). */
typedef struct {
    unsigned columns, rows;
    const char *controller, *rom, *panel, *backlight_color;
    const size_t *bus_pins; size_t bus_pin_count;  /* RS, R/W, E, DB0-DB7: indices into Part.pins */
    unsigned enables;                          /* 1, or 2: bus_pins is RS, R/W, E1, E2, DB0-DB7 */
    size_t backlight_count;                    /* 0-3 LEDs */
    const char *const *backlight_colors;       /* backlight_count colors; [0] == backlight_color */
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
/* A capacitor part (plan-33-E): type "ceramic", "aluminum",
   "aluminum-polymer", "tantalum", "tantalum-polymer" or "niobium-oxide";
   polarized: 1 for every type but ceramic. capacitance_ff: femtofarads
   (100 nF = 100000000). rated_mv: the rated DC voltage, 0 = no rating (the
   generic part). dielectric (ceramic only) and tolerance: as the datasheet
   gives them, NULL when not given. */
typedef struct { const char *type; int polarized; uint64_t capacitance_ff;
                 uint32_t rated_mv; const char *dielectric;
                 const char *tolerance; } PartCapacitor;
/* A resistor part (plan-34-E): resistor_count: the device's (a, b) pairs,
   one per element. milliohms: the resistance (0 = a jumper); tolerance_ppm:
   the tolerance in parts per million (10000 = 1 %); power_mw: rated power
   per element. 0 = not given for tolerance and power (the generic part, a
   jumper's tolerance). */
typedef struct { size_t resistor_count; uint64_t milliohms;
                 uint32_t tolerance_ppm, power_mw; } PartResistor;
/* A transistor part (plan-40-F): one or more elements (BJT or MOSFET),
   each a digital switch. type: the first element's ("npn", "pnp", "nmos",
   "pmos"); mixed: 1 when the elements' types differ (a complementary pair);
   darlington: 1 when any element is one; mosfet: 1 for nmos / pmos.
   vmax_mv: Vceo or Vds; imax_ma: Ic or Id; power_mw: Pd; hfe: minimum DC
   gain; vgsth_mv, vgs_mv: max gate threshold and gate-source voltage;
   r1_mohm, r2_mohm: a pre-biased BJT's base and base-emitter resistors;
   rdson_mohm: max Rds(on); logic_level: Rds(on) specified at a gate drive
   of 4.5 V or less. An array's facts hold for every element. 0 = not given. */
typedef struct { const char *type; size_t element_count; uint8_t mixed; uint8_t darlington;
                 uint8_t mosfet; uint8_t logic_level; uint32_t vmax_mv, imax_ma, power_mw, hfe,
                 vgsth_mv, vgs_mv; uint64_t r1_mohm, r2_mohm, rdson_mohm; } PartTransistor;

/* A voltage regulator part (plan-42-G): one to four regulators (elements),
   linear or switching. type: the first element's ("linear", "buck", "boost",
   "buck-boost", "charge-pump", "sepic"); mixed: 1 when the types differ;
   adjustable: 1 when any element has an adjust pin. vout_mv / vref_mv: one
   per element, in element order (0 where not given: an adjustable element
   has no fixed output, a fixed one no reference). vout_max_mv: the highest
   output an adjustable part can be set to; vin_min_mv / vin_max_mv: the
   input range's ends; dropout_mv: a linear element's dropout; iout_ma: the
   output current per element; iq_ua: the quiescent current; fsw_hz: a
   switching element's frequency. All other values: 0 = not given. */
typedef struct { const char *type; size_t regulator_count; uint8_t mixed; uint8_t adjustable;
                 const uint32_t *vout_mv; const uint32_t *vref_mv;
                 uint32_t vout_max_mv, vin_min_mv, vin_max_mv, dropout_mv, iout_ma, iq_ua;
                 uint64_t fsw_hz; } PartRegulator;

/* A crystal or ceramic resonator (plan-44-E): a two-terminal frequency
   reference and, in the simulation, a clock source on its own two terminals
   (X1 at the level, X2 at the complement, both weak). type: "crystal" or
   "resonator". hz: the frequency, 1 to 1000000000. load_ff: the load
   capacitance in femtofarads (18 pF = 18000), 0 = not given (series
   resonant, or a resonator with built-in capacitors). tolerance_ppm and
   stability_ppm: parts per million, 0 = not given. esr_mohm: the ESR (a
   resonator's impedance) in milliohms, 0 = not given. capacitors: 1 for a
   resonator with built-in load capacitors (its third pad is the common pin).
   The facts are for the app; the simulation reads only hz. */
typedef struct { const char *type; uint64_t hz; uint64_t load_ff;
                 uint32_t tolerance_ppm, stability_ppm; uint64_t esr_mohm;
                 int capacitors; } PartCrystal;

/* A microcontroller (plan-36-F): facts only, no simulation model (its
   device says "simulated": false). core: "avr", "avrrc", "avrxt", "avrxm",
   "cortex-m0plus", "xtensa-lx6", "xtensa-lx7" or "riscv". max_hz: the
   highest clock. Sizes in bytes; 0 = none (eeprom, psram) or external
   (RP2040 flash). A part may override clock, flash and psram. */
typedef struct { const char *core; uint64_t max_hz; uint32_t flash, ram, eeprom, psram; } PartMcu;

/* A serial (SPI) memory chip (serialmem kind). type: "eeprom", "fram",
   "flash" (the three the engine simulates), "dataflash" or "pageeeprom"
   (facts only: their devices say "simulated": false). size: bytes.
   address_bytes: bytes after the opcode for READ/WRITE (0 = not given).
   page_bytes: the EEPROM write page / the flash program page (0 = none).
   write_ps: the EEPROM write cycle or the flash page-program time (0 =
   none: an F-RAM writes at bus speed). sector_erase_ps, chip_erase_ps:
   flash (0 = none). jedec_id: the RDID bytes as hex (NULL = none).
   max_clock_khz: the highest SCK (the part's own value over the device's);
   0 = not given. endurance: write cycles (0 = not given); retention_years
   (0 = not given). */
typedef struct {
    const char *type; uint32_t size; unsigned address_bytes; uint32_t page_bytes;
    uint64_t write_ps, sector_erase_ps, chip_erase_ps; const char *jedec_id;
    uint32_t max_clock_khz; uint64_t endurance; uint32_t retention_years;
} PartSerialmem;

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
    int simulated;                     /* 1 = the engine has a model for the part; 0 = the
                                          device says "simulated": false, and
                                          circuit_add_part refuses the part */
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
                                          LEDs, 7-seg displays, nixies and switches */
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
    const PartCapacitor *capacitor;
    const PartMcu *mcu;
    const PartResistor *resistor;
    const PartTransistor *transistor;
    const PartNixie *nixie;
    const PartSerialmem *serialmem;
    const PartRegulator *regulator;
    const PartCrystal *crystal;
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
 * Takes one single-part-file part out of the library (plan-53-D): its name,
 * its uuid, and the inline footprint and datasheet ids the caller lists (the
 * library does not record which belong to which file), so a new file with the
 * same uuid, name and ids can be loaded. Unlike the unload above this works
 * for any such part, not only the latest. The memory is NOT freed, so every
 * Part, PartFootprint and PartDatasheet pointer already handed out stays
 * valid until part_library_destroy. Ids that are not in the library are
 * ignored. Returns 0, or -1 (nothing changed) when no part has this uuid or
 * on out-of-memory. After it the latest-file unload no longer applies.
 */
int part_library_retire_part(PartLibrary *library, const char *uuid,
                             const char *const *footprint_ids, size_t footprint_count,
                             const char *const *datasheet_ids, size_t datasheet_count);

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
