#ifndef MODEL_PINS_H
#define MODEL_PINS_H

/*
 * Pin facts a few models keep private, for the voltage check (voltage.c).
 * Each returns 0 for a part of another kind.
 */

#include <stddef.h>

#include "parts.h"

/* An LED's anode and cathode, as indices into Part.pins. */
int led_pins(const Part *part, size_t *anode, size_t *cathode);

/* A nixie tube's k-th link (0-based, nixie.c): an anode pin and a cathode
   pin, as indices into Part.pins; every anode pin with every cathode. 0
   past the last link, and for a part that is no nixie. */
int nixie_link(const Part *part, size_t k, size_t *anode, size_t *cathode);
/* The least voltage a nixie's anode pin needs, in millivolts: the tube's
   firing ("strike") voltage. 0 for a cathode, and a part that is no nixie. */
uint32_t nixie_anode_min_mv(const Part *part, size_t pin);

/* A dekatron's k-th link (dekatron.c): the anode pin and its k-th distinct
   cathode pin, as indices into Part.pins. 0 past the last cathode pin, and
   for a part that is no dekatron. */
int dekatron_link(const Part *part, size_t k, size_t *anode, size_t *cathode);
/* The least voltage a dekatron's anode needs, in millivolts: the tube's
   minimum anode supply ("strike"). 0 for any other pin, and a part that is
   no dekatron. */
uint32_t dekatron_anode_min_mv(const Part *part, size_t pin);

/* A character LCD's backlight LED `index` (0-based, in the device's
   "backlight" order): its anode and cathode, as indices into Part.pins
   (charlcd.c). 0 past the last LED, and for a module without a backlight. */
int charlcd_backlight_pins(const Part *part, size_t index, size_t *anode, size_t *cathode);

/* A diode part's k-th link (0-based): its anode and cathode pins, as
   indices into Part.pins (diode.c). Links are the device's pairs, then the
   ties between same-named pins (both ways). 0 past the last link. */
int diode_pair(const Part *part, size_t k, size_t *anode, size_t *cathode);
/* A resistor part's k-th link (0-based, plan-34-E): its two ends, as
   indices into Part.pins (resistor.c). Links are the device's pairs, then
   the ties between same-named pins (both ways). 0 past the last link, and
   for a part that is no resistor. */
int resistor_pair(const Part *part, size_t k, size_t *a, size_t *b);

/* A transistor part's k-th element (0-based, plan-40-F): its type ("npn",
   "pnp", "nmos", "pmos"), its control pin (base, gate) and its first
   channel pins (collector and emitter, drain and source), as indices into
   Part.pins (transistor.c). Any output may be NULL. 0 past the last
   element, and for a part that is no transistor. */
int transistor_element(const Part *part, size_t k, const char **type, size_t *control,
                       size_t *a, size_t *b);
/* The voltage a transistor pin is rated for, in millivolts: Vceo / Vds for
   a channel pin, Vgs for a MOSFET gate; 0 (never refused) for a BJT base,
   a rating the part does not give, and a part that is no transistor. */
uint32_t transistor_pin_limit_mv(const Part *part, size_t pin);

/* A regulator part's k-th element (0-based, plan-42-G): its type, and the
   first pin (an index into Part.pins) of its input, output, enable,
   power-good and adjust names (SIZE_MAX when it has none); enable_low: the
   enable turns the element on at 0; adjust_to_output: Vref is held between
   the output and the adjust pin (LM317 style), else the adjust pin is Vref
   above ground. Any output may be NULL. 0 past the last element, and for a
   part that is no regulator (regulator.c). */
int regulator_element(const Part *part, size_t k, const char **type, size_t *in, size_t *out,
                      size_t *enable, int *enable_low, size_t *pg, size_t *adjust,
                      int *adjust_to_output);
/* 1 when pin carries element k's input (every pin of the name) / output name. */
int regulator_pin_in(const Part *part, size_t k, size_t pin);
int regulator_pin_out(const Part *part, size_t k, size_t pin);
/* The input maximum in millivolts: Part.regulator->vin_max_mv; 0 (never
   refused) when not given and for a part that is no regulator. */
uint32_t regulator_input_max_mv(const Part *part);

/* A power switch part's k-th switch (0-based, plan-55-A): the first pin (an
   index into Part.pins) of its input, output, enable and flag names (SIZE_MAX
   when it has none); enable_low: the enable turns the switch on at 0;
   enable_ref_input: the enable is a comparator against the input (the
   LM66100's CE); flag_disabled: the flag is low while the switch is off, else
   it is a fault flag. Any output may be NULL. 0 past the last switch, and for
   a part that is no power switch (powerswitch.c). */
int powerswitch_element(const Part *part, size_t k, size_t *in, size_t *out, size_t *enable,
                        int *enable_low, int *enable_ref_input, size_t *flag, int *flag_disabled);
/* 1 when pin carries switch k's input (every pin of the name) / output name. */
int powerswitch_pin_in(const Part *part, size_t k, size_t pin);
int powerswitch_pin_out(const Part *part, size_t k, size_t pin);
/* The input maximum in millivolts: Part.powerswitch->vin_max_mv; 0 (never
   refused) when not given and for a part that is no power switch. */
uint32_t powerswitch_input_max_mv(const Part *part);
/* An inductor's or a fuse's k-th fixed contact (0-based, plan-47-I): two pins
   joined inside the part whatever the circuit does, as indices into Part.pins
   (inductor.c, fuse.c). Contact 0 is the two terminals; then each tie between
   pins of one terminal's name. 0 past the last, and for a part of another
   kind. (A fuse blown in play still counts: the voltage check answers "may
   this board be powered".) */
int inductor_fixed_contact(const Part *part, size_t k, size_t *a, size_t *b);
int fuse_fixed_contact(const Part *part, size_t k, size_t *a, size_t *b);
/* A relay's k-th fixed contact (0-based, plan-47-J, relay.c): the relay at
   rest, every pole's common-nc pair, then the same-name ties. */
int relay_fixed_contact(const Part *part, size_t k, size_t *a, size_t *b);
/* A part's k-th fixed contact (model_pins.c): the kinds above, dispatched on
   part->kind. 0 past the last, and for every other kind. */
int part_fixed_contact(const Part *part, size_t k, size_t *a, size_t *b);

/* A switch position's (1-based) two contact pins, as indices into Part.pins. */
int switch_contact_pins(const Part *part, unsigned position, size_t *a, size_t *b);

/* A connector's normally-closed contact `position` (1-based): its two pins
   (indexes into Part.pins). 0 for another kind or position (plan-68-L: the
   voltage check joins a switched jack's nets while it is unplugged). */
int connector_contact_pins(const Part *part, unsigned position, size_t *a, size_t *b);

/*
 * 1 when pin (an index into Part.pins) is a PIN_OUTPUT or PIN_INOUT the part
 * never drives high: an open-drain output or port (gate, dff and buffer
 * "output": "open-drain"; the SN7447A's BI/RBO). Each
 * kind answers for its own parts through PartKindOps.pin_open_drain;
 * 0 for every kind without one.
 */
int part_pin_open_drain(const Part *part, size_t pin);

/* 1 when pin is a PIN_OUTPUT the part releases to Z through an output enable
   (3-state): the ERC's tri_state type (plan-63-B). Each kind answers for its
   own parts through PartKindOps.pin_tristate; 0 for every kind without one. */
int part_pin_tristate(const Part *part, size_t pin);

/* 1 when pin is an analog pin of a converter or RTC part (plan-24-D): the
   voltage check ignores it, and a power pin among them is not the part's
   supply. 0 for every kind without the hook. */
int part_pin_analog(const Part *part, size_t pin);

#endif
