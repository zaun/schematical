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

/*
 * The pin checks (plan-63-D): five of KiCad's rules that read pin types, on a
 * netlist of typed pins. A pure function over arrays, as voltage_check is.
 *
 * A net's members are its wired pins of every type but ERC_NO_CONNECT (a
 * no_connect-typed pin is never a connection). A Power Flag is an ordinary pin
 * of type ERC_POWER_OUT with no part behind it.
 *
 *   ERC_NO_CONNECT_CONNECTED   a wired pin of type no_connect; a pin with a No
 *                              Connect flag whose net has another member
 *   ERC_PIN_NOT_CONNECTED      a pin (not no_connect or free, no No Connect
 *                              flag) that is unwired, or alone on a net that
 *                              has no label or bus port
 *   ERC_PIN_TO_PIN             walking a net's members in order, each member
 *                              that conflicts (pin map cell non-zero) with an
 *                              earlier one, with the first earlier one it
 *                              conflicts with; level = the cell (1 warning,
 *                              2 error)
 *   ERC_POWER_PIN_NOT_DRIVEN   a net with a power_in member and no power_out
 *                              member (nor a pin that drives_power): one
 *                              problem, on its first power_in member
 *   ERC_PIN_NOT_DRIVEN         else a net with an input member and no output,
 *                              bidirectional, tri_state, passive or power_out
 *                              member (nor a drives_power pin): one problem,
 *                              on its first input member
 *
 * An unwired pin is a net of its own. A net with a No Connect flag on one of
 * its pins is not checked for "not driven".
 */
typedef struct {
    size_t net;              /* the pin's net, SIZE_MAX when no wire reaches it */
    ErcPinType type;
    int drives_power;        /* erc_pin_drives_power: a supply symbol's pin */
    int no_connect_mark;     /* a No Connect flag is on the pin's wire group */
} ErcPin;

typedef struct { int has_attachment; } ErcNet;   /* a label or a bus port is on the net */

typedef enum {
    ERC_PIN_TO_PIN, ERC_PIN_NOT_CONNECTED, ERC_PIN_NOT_DRIVEN, ERC_POWER_PIN_NOT_DRIVEN,
    ERC_NO_CONNECT_CONNECTED
} ErcProblemKind;

typedef struct {
    ErcProblemKind kind;
    size_t pin;              /* an index into the pins array */
    size_t other;            /* PIN_TO_PIN: the earlier pin it conflicts with; else = pin */
    size_t net;              /* the pin's net (SIZE_MAX: unwired) */
    int level;               /* PIN_TO_PIN: 1 warning, 2 error; else 0 */
} ErcProblem;

/* KiCad's default pin map (symmetric; 0 ok, 1 warning, 2 error), in ErcPinType order. */
extern const uint8_t ERC_DEFAULT_PIN_MAP[ERC_PIN_TYPE_COUNT][ERC_PIN_TYPE_COUNT];

/* Returns the number of problems (all of them); writes up to cap into out,
   ordered by net (unwired pins last), then pin, then kind. nets has net_count
   entries, or is NULL (no net has an attachment). */
size_t erc_check(const ErcPin *pins, size_t pin_count, size_t net_count, const ErcNet *nets,
                 const uint8_t pin_map[ERC_PIN_TYPE_COUNT][ERC_PIN_TYPE_COUNT],
                 ErcProblem *out, size_t cap);

#endif
