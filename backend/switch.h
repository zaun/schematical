#ifndef SWITCH_H
#define SWITCH_H

/*
 * Switches. Each position is a contact between two pins: while it is ON the
 * nets on its two pins act as one net (see sim_set_contact), so a value passes
 * through in both directions. Every position starts OFF.
 *
 * DIP switches
 * ------------
 * TE Connectivity (Tyco Electronics) DIP switches, transcribed from
 * datasheets/switch/ENG_CS_1308111-1_SWITCHES_CORE_PROGRAM_CATALOG_0308.pdf
 * ("Preferred Part Numbers - DIP Switches", page 14). Only parts sold in
 * tubes are listed. Parts are named by the datasheet's product key with a
 * "TE-" prefix, e.g. TE-GDS0404, not by the TE part number (1825006-5).
 *
 *   GDS   low profile             2, 4, 6, 8, 10 positions   THT and SMT
 *   GDP   piano actuator          2, 4, 6, 8, 10             THT
 *   GDR   recessed rocker         4, 8                       THT and SMT
 *   GDH   low profile, half pitch 2, 4, 8                    SMT
 *   GDHL  ultra low profile       4, 8                       SMT
 *   ADE   extended actuator       2, 4, 6, 8, 10             THT (4 and 8 also SMT)
 *   ADF   flush actuator          2, 4, 6, 8, 10             THT
 *
 * A product key with "S" after the position count is the SMT version.
 *
 * Pins: a switch with N positions has 2N pins, numbered like a DIP package.
 * Position k joins pin k (named "kA") and pin 2N + 1 - k (named "kB"). So on
 * an 8-position switch, position 1 is pins 1 and 16, position 8 pins 8 and 9.
 */

#include <stddef.h>

#include "chip_definition.h"
#include "sim.h"

extern const SimComponentType SWITCH_TYPE;

/* Catalog definitions: one per series and position count. */
size_t switch_definition_count(void);
const ChipDefinition *switch_definition_at(size_t index);

/*
 * Returns the simulation component for an orderable switch name and fills in
 * its configuration. Returns NULL when the name is not a switch.
 */
const SimComponentType *switch_lookup(const char *part_number, SimLogicConfig *config);

/* Switch positions (1 to N) of a configuration, 0 when it is not a switch. */
size_t switch_config_positions(const SimLogicConfig *config);

/* Readable actuator style of a switch configuration, e.g. "Piano actuator". */
const char *switch_family_label(const SimLogicConfig *config);

/* Number of positions of an instance, or 0 when the instance is not a switch. */
size_t switch_instance_positions(const SimInstance *instance);

/*
 * Turns a position (1-based) ON or OFF. Before sim_start this sets the
 * starting state; while running it takes effect at the current time.
 * Returns 1 on success, 0 when the instance is not a switch or the position
 * is out of range.
 */
int switch_set_position(Sim *sim, SimInstance *instance, size_t position, int on);

/* Whether a position (1-based) is ON. 0 when it is OFF or out of range. */
int switch_position_on(const Sim *sim, const SimInstance *instance, size_t position);

#endif
