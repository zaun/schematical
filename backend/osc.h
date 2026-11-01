#ifndef OSC_H
#define OSC_H

/*
 * Crystal oscillators from the datasheets in datasheets/osc/.
 *
 *   SIT1534  SiTime MEMS programmable oscillator, 1 Hz to 32.768 kHz, 4 pins.
 *            Packages: 2012 SMD (2.0 x 1.2 mm), 1508 CSP (1.5 x 0.8 mm).
 *   LMK6C    TI BAW oscillator, 1 MHz to 200 MHz, LVCMOS output, 4 pins.
 *            Packages: 3225 VSON-4 DLE, 2520 VSON-4 DLF.
 *   LMK6D/H/P TI BAW differential oscillators (LVDS, HCSL, LVPECL),
 *            1 MHz to 400 MHz, 6 pins. Packages: 3225 VSON-6 DLE, 2520 VSON-6 DLF.
 *
 * Orderable names are the datasheet part numbers. LMK6 names carry the
 * frequency, for example LMK6CA026214DDLFR is 26.214 MHz. SiT1534 names are
 * generic (one per package) because its ordering code combines temperature,
 * stability, output and reel options that are not listed as parts.
 *
 * Output: the primary output pin toggles at half the period. A differential
 * part's second output is the complement. Output enable and standby work as the
 * datasheet says: a LOW on the function pin (pin 1, or pin 2 for F/B options)
 * puts every output in high impedance. NC or HIGH keeps the clock running.
 * Power pins are not checked. The half period is rounded to whole picoseconds.
 *
 * Catalog groups are frequency bands. Each part sits in the band that holds its
 * own frequency (the frequency in its name for LMK6 parts).
 */

#include <stddef.h>
#include <stdint.h>

#include "chip_definition.h"
#include "sim.h"

typedef enum {
    OSC_FAMILY_SIT1534,
    OSC_FAMILY_LMK6C,
    OSC_FAMILY_LMK6D,
    OSC_FAMILY_LMK6H,
    OSC_FAMILY_LMK6P,
    OSC_FAMILY_COUNT
} OscFamily;

typedef enum {
    OSC_PACKAGE_2012,
    OSC_PACKAGE_1508,
    OSC_PACKAGE_3225,
    OSC_PACKAGE_2520,
    OSC_PACKAGE_COUNT
} OscPackage;

typedef struct {
    OscFamily family;
    OscPackage package;
    uint64_t frequency_hz;
    int enable_pin;  /* 0-based pin index of the OE/standby pin, -1 for none */
} OscConfig;

extern const SimComponentType OSC_TYPE;

/* Pin table for a family and package. */
const ChipPin *osc_pins(OscFamily family, OscPackage package);
size_t osc_pin_count(OscFamily family, OscPackage package);

/* Orderable names. */
size_t osc_part_count(void);
const char *osc_part_name_at(size_t index);
int osc_part_config_at(size_t index, OscConfig *config);

/* Frequency range the part's family can produce, in Hz. */
uint64_t osc_part_min_hz(size_t index);
uint64_t osc_part_max_hz(size_t index);

const char *osc_family_label(OscFamily family);
const char *osc_package_label(OscPackage package);

/* Returns the model for an orderable name, filling in its default config. */
const SimComponentType *osc_lookup(const char *part_number, OscConfig *config);

/* Frequency bands for the catalog. */
size_t osc_group_count(void);
const char *osc_group_label(size_t group);
int osc_part_in_group(size_t part, size_t group);

#endif
