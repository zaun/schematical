#ifndef ERC_H
#define ERC_H

/*
 * The electrical rules check's pin types (plan-63-B): the KiCad pin type
 * each pin of each part stands for. A part file gives a pin one of seven
 * roles (input, output, inout, power, ground, nc, absent); KiCad's ERC
 * reasons about twelve types. The type comes from the role, the part's kind
 * and what the kind already knows of its pins (open drain, 3-state through
 * an output enable).
 *
 *   absent pin                      no type
 *   nc pin                          ERC_NO_CONNECT
 *   a supply's pull-up/-down pin    ERC_PASSIVE
 *   a supply's power/ground pin     ERC_POWER_IN, and it drives its own net
 *                                   (erc_pin_drives_power): a rail symbol is
 *                                   a driver without a Power Flag
 *   input/output/inout of a kind whose pins are passive in KiCad's own
 *   libraries (connector, header, socket, resistor, capacitor, inductor,
 *   diode, led, fuse, crystal, relay, buzzer, switch, transistor, 7seg,
 *   nixie, dekatron, lcd)           ERC_PASSIVE
 *   power/ground                    ERC_POWER_IN
 *   output of a regulator/power switch's output name   ERC_POWER_OUT
 *   output, open drain              ERC_OPEN_COLLECTOR
 *   output, 3-state (output enable) ERC_TRI_STATE
 *   output                          ERC_OUTPUT
 *   inout, open drain               ERC_OPEN_COLLECTOR
 *   inout                           ERC_BIDIRECTIONAL
 *   input                           ERC_INPUT
 *
 * ERC_FREE, ERC_UNSPECIFIED and ERC_OPEN_EMITTER exist for the pin map and
 * are never returned: no part file says so.
 */

#include <stddef.h>
#include <stdint.h>

#include "parts.h"

/* KiCad's order (the order of its pin map). */
typedef enum {
    ERC_INPUT, ERC_OUTPUT, ERC_BIDIRECTIONAL, ERC_TRI_STATE, ERC_PASSIVE, ERC_FREE,
    ERC_UNSPECIFIED, ERC_POWER_IN, ERC_POWER_OUT, ERC_OPEN_COLLECTOR, ERC_OPEN_EMITTER,
    ERC_NO_CONNECT,
    ERC_PIN_TYPE_COUNT
} ErcPinType;

/* The KiCad type of pin (an index into Part.pins). Returns 1 and sets *type;
   0 for an absent pin and for pin >= pin_count (*type is untouched). */
int erc_pin_type(const Part *part, size_t pin, ErcPinType *type);

/* 1 when pin is a supply symbol's rail pin: it drives its own net, as a
   Power Flag would. 0 for every other pin. */
int erc_pin_drives_power(const Part *part, size_t pin);

/* KiCad's word for a type: "input", "tri_state", "power_in", "no_connect". */
const char *erc_pin_type_name(ErcPinType type);

#endif
