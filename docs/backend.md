# Backend

The backend is a C library: an event-driven logic simulator plus a part
library loaded from JSON. The app (and the tests) use three public headers in
`backend/include/`:

- `parts.h` — the part library: load the part files, look parts up, read
  their facts as C structs.
- `circuit.h` — build a circuit from library parts, run it, read nets,
  lights and memory back.
- `memory_image.h` — turn a load file (raw binary or Intel HEX) into a
  memory image for `circuit_memory_load`. It takes bytes; the backend never
  opens files.

Part data lives in `schematical/schematical/Parts/<kind>.json`, one file per
kind; the format is in [parts-format.md](parts-format.md).

## Using it

```c
#include "parts.h"
#include "circuit.h"

/* 1. Load footprints.json, then the part files (the app reads its bundle
      resources; tests use tests/support/read_file.c). Each call takes one
      file's text. */
PartLibrary *library = part_library_create();
char *error = NULL;
if (load_footprints(library, footprints_json_text, "footprints.json", &error) != 0) {
    fprintf(stderr, "%s\n", error);
    free(error);
}
if (load_parts(library, gate_json_text, "gate.json", &error) != 0) {
    fprintf(stderr, "%s\n", error);   /* e.g. gate.json: devices[3] (74_00): ... */
    free(error);
}

/* 2. Read a part's facts. */
const Part *part = part_library_find(library, "SN74HC00N");
/* part->kind == PART_GATE, part->logic->family == "HC", part->pins[...],
   part->footprint->pads[...] (mm, designator matches part->pins[i].designator) */

/* 3. Build and run a circuit. Pins are named by designator ("1", "A1"). */
Circuit *c = circuit_create(library);
size_t vcc, gnd, led, anode, cathode;
circuit_add_part(c, "VCC", &vcc);
circuit_add_part(c, "GND", &gnd);
circuit_add_part(c, "GENERIC-LED", &led);
circuit_add_net(c, &anode);
circuit_add_net(c, &cathode);
circuit_connect(c, vcc, "1", anode);
circuit_connect(c, led, "2", anode);    /* LED pin 2 = A */
circuit_connect(c, led, "1", cathode);  /* LED pin 1 = K */
circuit_connect(c, gnd, "1", cathode);
circuit_start(c);
circuit_advance(c, 1000000);            /* 1 us, in picoseconds */

/* 4. Read it back. */
CircuitValue v = circuit_net_value(c, anode);          /* CIRCUIT_1 */
unsigned char lit;
circuit_part_lights(c, led, &lit, 1);                  /* 1 */

circuit_destroy(c);
part_library_destroy(library);         /* the library outlives its circuits */
```

Notes:

- **Single part files** (plan-32-A). `load_part_file(library, text, source,
  &error)` loads one `.schpart` file (format `schematical-part`,
  `docs/parts-format.md` § Single part files) into a library that already
  holds `footprints.json`, `datasheets.json` and the bundled part files: one
  device with one part, a required `uuid`, and optional inline `footprints`
  (the `footprints.json` entry format, checked by the same code,
  `footprint_read_entries` in `footprints.c`). The part's `footprint` may
  name a bundled id or an inline one. All or nothing: the file is checked
  whole with the inline footprints staged (visible only to that load's
  footprint lookups), and its footprints, part, name and uuid are added only
  when every check passed. The device id is not checked against the
  library's devices, so a copy of a bundled device loads; the part name
  still must be new (the app renames a clash, plan-32-A § 4.3). The loaded
  `Part.uuid` is the file's `uuid` (NULL for every bundled part), and
  `part_library_find_uuid` finds the part by it. Errors read like
  `load_parts`': `x.schpart: devices[0] (1G00): parts[0] (MY-NAND): …`.
  `part_library_unload_last_part_file` takes back the latest
  `load_part_file` (its part, name, uuid, inline footprints and arena
  memory) while nothing else was loaded since; the Part Editor's validator
  (plan-32-C) loads each draft into one scratch library this way. A file
  may carry an `originId` (plan-53-A), the uuid its part's first version
  had; the loader checks that it is an uppercase UUID and ignores it.
  `part_library_retire_part(library, uuid, footprint_ids, n, datasheet_ids, n)`
  (plan-53-D) takes any one single-part-file part out of the library: its
  name, uuid and the inline footprint and datasheet ids the caller lists, so a
  new version with the same uuid, name and ids can be loaded. It is how the app
  replaces a loaded part. Unlike the unload above it frees no memory, so
  every `Part`, `PartFootprint` and `PartDatasheet` pointer already handed out
  stays valid until `part_library_destroy` (a retired part's memory is the cost
  of one part per update).
- Building calls (`circuit_add_part`, `_add_net`, `_connect`) work only
  before `circuit_start`; `circuit_set_switch` works before and after.
- `circuit_set_switch(…, position, on)`: ON = contact closed, for every
  switch. A normally-closed (tactile) switch starts closed, so `off` opens
  it. A tactile switch's joined pin pairs (`joined`) are always closed and
  are not positions (`"<name> has no position N"`); `circuit_start` gives
  each of its unwired pins a net of its own so the chain A1–B1–B2 holds
  (plan-27-D).
- `circuit_set_switch_state(circuit, part, state)` (plan-58-B): moves the knob
  of a slide switch, a `switch` device with `"states"` (see `docs/parts-format.md`
  § `switch`), to knob position `state` (1 … `PartSwitch.states`). The contacts
  the state does not list open first, then the ones it lists close (break before
  make), each exactly as `circuit_set_switch` would, and one voltage check
  follows when a contact closed during a run (the run stops on a failure, like
  a closing tactile switch). A new circuit starts the knob in the device's
  `"rest"` position: `circuit_add_part` records that state's contacts closed
  and `circuit_start` applies them once the nets exist, so the voltage check
  at start sees them. Errors: `"<name> is not a slide switch"`, `"<name> has no
  position N (1 … K)"`. `circuit_set_switch` still addresses single contacts of
  every switch; the backend keeps no knob position of its own, only the
  recorded contacts. A light is on when it was on at any time during the last
  `circuit_advance`, so the advance right after a move still shows the old
  state (`slide_test` settles with two).
- `circuit_advance` returns 0 once the run has stopped (a short circuit is
  `"smoke: ..."`, and `circuit_shorted_net` names the net).
- **Diodes** (plan-29-H). A diode part drives nothing; each of its
  (anode, cathode) pairs is a one-way link in the engine, reported by the
  model's `diodes` hook (`sim.h`) and collected once by `sim_start`. A 1
  passes from anode to cathode and a 0 from cathode to anode. Contact
  groups joined by links form a *cluster*, resolved as one unit by
  `resolve_cluster` (`sim.c`): each group gets the masks of its own
  drivers' values (strong and weak, `net_drivers` in `net.c`); strong
  values pass along the links until nothing changes; then weak values pass
  between groups no strong value reached. A group reads its strong values
  if any (one value, else X), else its weak ones, else Z; it is shorted
  when its strong values hold a 0 and a 1. Values are recomputed from the
  drivers every round, so nothing latches. A circuit with no diode (and no
  closed contact) takes the old per-net path unchanged. What it gives:

  | Circuit | Result |
  |---|---|
  | strong 1 on anode, cathode pulled down | cathode 1 |
  | strong 0 on anode, cathode pulled down | cathode 0 (the pull) |
  | two gates on two anodes, common cathode pulled down (diode-OR) | OR of the gates; the gates' nets do not change |
  | two gates on two cathodes, common anode pulled up (diode-AND) | AND of the gates |
  | strong 1 on anode, strong 0 on cathode | `smoke:` |
  | strong 0 on anode, strong 1 on cathode | no conduction; both keep their values |
  | two diodes in series, strong 1 at the first anode, pull-down at the end | end and middle read 1 |
  | antiparallel pair, a weak 1 on one side | both 1; both Z again when the 1 goes |
  | X driven strong on an anode | cathode X |
  | nothing on either side | both Z |

  Pins that share a name on one diode part (a tab and a lead) are tied by
  links both ways, so they pass every value.
- **Resistors** (plan-34-E). A resistor part drives nothing; each of its
  (a, b) pairs is a two-way *weak* link, reported by the model's
  `resistors` hook (`sim.h`) and collected by `sim_start` with the diode
  links (`collect_links`; the link struct's `weak` flag tells them apart).
  Resistor links join clusters as diode links do. In `resolve_cluster` the
  strong pass skips them; in the weak pass a resistor link gives each end
  the other end's level (its strong values if any, else its weak ones) as
  weak values, both ways, to a fixed point. So a resistor never passes a
  strong value and never makes a short; a resistor to VCC is a pull-up a
  real driver overrides. Ohms have no effect on logic. Pins that share a
  name on one resistor part (an array's two common pins) are tied by
  diode links both ways (the `diodes` hook), so they are one node. What it
  gives:

  | Circuit | Result |
  |---|---|
  | strong 1 at a, nothing at b | b reads 1 |
  | strong 0 at a, nothing at b | b reads 0 |
  | strong 1 at a, strong 0 at b | a 1, b 0, no `smoke:` |
  | strong 1 at a, pull-down (weak 0) at b | b reads X |
  | nothing at either end | both Z |
  | a – R – b – R – c, strong 1 at a | b and c read 1 |
  | strong 1 at a, b also driven strong 0 later | b follows to 0, then back to 1 when released |
  | diode-OR of two gates into node n, n – R – m, nothing else at m | m reads the OR |

  The voltage check carries each end's level to the other end's net, both
  ways (`resistor_pair` in `model_pins.h`).
- **Transistors** (plan-40-F). A transistor part is one or more elements
  (BJT or MOSFET), each a digital switch: its channel (collector–emitter,
  drain–source) is an engine **contact** (`sim_set_contact`), closed while
  the control pin reads 1 for an NPN / N-channel element and 0 for a PNP /
  P-channel one; X and Z leave it open. The model needs no new engine
  hook: `pin_changed` on a control pin schedules a timer 1 ns later
  (`TRANSISTOR_DELAY_PS`), and the timer re-reads the pin and sets the
  contact. So a channel change is always a later event than the read that
  caused it (a gate tied to its drain toggles every 1 ns and the run
  returns), and a pulse shorter than 1 ns never moves the channel. A
  conducting channel passes strong values both ways, as a closed switch
  does, and a strong 1 and 0 across it is `smoke:`. Same-named pins (a tab
  and its lead) are tied by contacts closed in `start`. What it gives:

  | Circuit | Result |
  |---|---|
  | NPN, base 1, collector pulled up, emitter GND | collector 0 after 1 ns; base 0: 1 again |
  | N-channel, the same with gate, drain, source | the same |
  | PNP / P-channel high side, emitter / source on VCC | base / gate 0: collector / drain 1; 1: Z |
  | base X, or not connected | channel open |
  | a 0.5 ns pulse on the base | the channel never moves |
  | base 1 across a strong 1 and a strong 0 | `smoke:`; base 0: no short |
  | emitter follower (collector VCC, base 1, emitter pulled down) | emitter 1 |
  | gate tied to drain, drain pulled up, source GND | toggles each ns; the run returns |
  | an inverter's output on an NPN's base, 5V – R – LED – collector | the LED lights while the inverter's output is 1 |
  | a tab named like its lead | the tab and the lead are one node |
  | a complementary pair | each element switches on its own base |

  The voltage check does not carry a level through a channel; it refuses a
  channel pin above `vceo` / `vds` and a MOSFET gate above `vgs`
  (`transistor_pin_limit_mv` in `model_pins.h`).
- **Inductors and fuses** (plan-47-I). An inductor, a fuse and a PTC
  resettable fuse conduct like a wire: the two terminals are joined by an
  engine **contact** (`sim_set_contact`) closed in `start`, and every further
  pin of a terminal's name (a four-pad inductor has two pads per terminal)
  gets a contact to every pin of the other terminal and to every other pin of
  its own name, all closed the same way (the engine leaves out a contact whose
  pin has no net, so a chain through an unwired first pad would drop the rest). An
  inductor's contact never opens. A fuse's is its **switch position 1**:
  `circuit_set_switch(c, part, 1, 0)` blows it, `(c, part, 1, 1)` restores
  it (a fuse blown before `circuit_start` starts open; position 2 and up
  answer `<name> has no position N`; an inductor answers `part N is not a
  switch`). A closed contact passes strong values both ways, so a gate output
  through an inductor lights an LED, and 5V and GND across an intact fuse is
  `smoke:`. The facts (inductance, currents, ratings) change nothing in the
  run. The voltage check unions the nets of every part's **fixed contacts**
  (`part_fixed_contact` in `model_pins.h`, `models/model_pins.c`) in step 1,
  blown or not, and refuses a fuse or PTC pin above its DC rating, else its
  AC one (`VOLTAGE_FUSE_OVER`).
- **Relays** (plan-47-J). A relay is coils plus poles of contacts, all engine
  **contacts** (`sim_set_contact`): the armature moves 1 ms (`RELAY_DELAY_PS`)
  after a coil pin last changed (`pin_changed` re-arms a timer of the coil's
  channel; the timer reads the coil and moves every pole of the channel, the
  open contacts first, in one event). A coil is driven forward at `a` = 1,
  `b` = 0 and backward at `a` = 0, `b` = 1 (a polarized coil and a solid-state
  input forward only); a latching relay keeps its position until driven the
  other way. `circuit_part_lights` / `circuit_part_light_time` report one
  light per channel (operated); a buzzer (`buzzer.c`) is one light, on while
  its positive pin reads 1 and its negative pin 0. The voltage check sees the
  relay at rest (`relay_fixed_contact`: the normally-closed contacts). What it
  gives:

  | Circuit | Result |
  |---|---|
  | coil between a gate output and GND, 5V on common | NO reads 1 and NC reads Z 1 ms after the output goes to 1; back 1 ms after it goes to 0 |
  | coil between 5V and an NPN's collector (emitter on GND) | operated while the base reads 1 |
  | a coil pin floating | at rest |
  | latching, 1 coil | a 2 ms forward pulse operates it; it stays until a 2 ms backward pulse |
  | latching, 2 coils | a pulse on the set coil operates it, one on the reset coil releases it |
  | a 0.5 ms pulse on a coil | nothing moves |
  | solid-state: anode 1, cathode 0 | the output contact closes after 1 ms; anode 0, cathode 1 does nothing |
  | a relay feeding its own coil through its normally-closed contact | toggles every 1 ms; the run returns |
- **Regulators** (plan-42-G). A regulator part is one to four elements,
  each a regulated supply: `regulator.c` schedules a timer 1 ns
  (`REGULATOR_DELAY_PS`) after any change of an element's input or enable
  pin, and the timer re-reads them and drives the element's output pins
  (`sim_schedule_drive`, strong) and its open-drain power-good pins. What it
  gives:

  | Input | Enable (active level, or no enable pin) | `out` | `pg` |
  |---|---|---|---|
  | 1 | active, or no pin | strong 1 | Z |
  | 1 | inactive, Z | Z | strong 0 |
  | 1 | X | strong X | strong X |
  | X | any | strong X | Z |
  | 0, Z, not connected | any | Z | Z |

  Several pins of the input name count as one input (any 1 and no X reads
  1). A 0.5 ns pulse on the enable never moves the output; the output on a
  net a strong 0 holds is `smoke:`. The output voltage is for the voltage
  check only (below). The kind has no `vcc`; `pin_open_drain` is the `pg`
  pins and `pin_analog` the `"analog"` names (`regulator_element`,
  `regulator_pin_in`, `regulator_pin_out`, `regulator_input_max_mv` in
  `model_pins.h`).
- **Power switches** (plan-55-A). A power switch part is one to four
  switches (elements), each a gate for a supply: `powerswitch.c` schedules a
  timer 1 ns (`POWERSWITCH_DELAY_PS`) after any change of a switch's input
  or enable pin, and the timer re-reads them and drives the switch's output
  pins (`sim_schedule_drive`, strong) and its open-drain flag pins. Two
  switches may name one output (or flag) pin: the pin takes the strongest of
  them (1 over X over Z for an output, 0 over X over Z for a flag). What it
  gives:

  | Input | Enable (`"logic"`, active level, or no enable pin) | `out` | `flag` (`"disabled"`) | `flag` (`"fault"`) |
  |---|---|---|---|---|
  | 1 | active, or no pin | strong 1 | Z | Z |
  | 1 | inactive, Z | Z | strong 0 | Z |
  | 1 | X | strong X | strong X | Z |
  | X | any | strong X | Z | Z |
  | 0, Z, not connected | any | Z | Z | Z |

  An enable that is a comparator against the input (`"enableRef":
  "input"`, an ideal diode's CE) never turns the switch off while the input
  reads 1, and with the input at 0 or Z a 1 on it turns a `"disabled"` flag
  low. A 0.5 ns pulse on the enable never moves the output; the output on a
  net a strong 0 holds is `smoke:`. The kind has no `vcc`; `pin_open_drain`
  is the flag pins and `pin_analog` the `"analog"` names
  (`powerswitch_element`, `powerswitch_pin_in`, `powerswitch_pin_out`,
  `powerswitch_input_max_mv` in `model_pins.h`).
- **Voltages** (`voltage.h`, plan-07). The simulation is logic-only; a
  separate static check, `voltage_check`, works out each net's voltage from
  its supplies (`PartSupply.mv`), the outputs of powered parts and what an
  LED or a diode (each link, `diode_pair` in `model_pins.h`) carries from
  anode to cathode and a resistor carries both ways, and reports every power pin outside
  its part's `vcc` range, input above its part's input max, switch contact
  above its rating, capacitor pin above its rated voltage, transistor
  rating (a channel pin above `vceo` / `vds`, a MOSFET gate above `vgs`),
  a regulator's or a power switch's input above its maximum, a fuse or PTC
  pin above its voltage rating, and two
  different non-zero supplies on one net (a regulator's regulated level
  counts as one; two power switches' levels never clash, the higher wins).
  `voltage_describe` gives the text, e.g. `voltage: 5 V on pin 14 (VCC) of
  SN74AUC00PWR; it needs 0.8-2.7 V`. It takes plain arrays (parts,
  pin→net connections, closed contacts), so the app runs it on a candidate
  netlist to refuse an edit.
  - **Regulated level** (plan-42-G). A regulator element whose input level
    has the headroom (output plus dropout for a linear one, `vinMin`) and
    whose enable is not inactive raises a regulated level R on its output
    nets: the order of levels is S, then R, then the driven D, the pulled P,
    the carried C. For an adjustable element R is Vref × (Rtop + Rbottom) /
    Rbottom (`adjustTo` ground) or / Rtop (`adjustTo` output), from the
    resistors of its feedback divider (`divider_mv`): the `PART_RESISTOR`
    links with an end on the adjust net, the other end on the output net
    (top, parallel links combine) or on a net a supply holds at 0 V
    (bottom); anything else or a 0 Ω link leaves the level unknown. R
    different from a supply level on the net, or two different R, is a
    `VOLTAGE_SUPPLY_CLASH`; an input or enable pin above `vinMax` is
    `VOLTAGE_REGULATOR_INPUT_OVER`. `voltage_check` takes an optional
    `milliohms` array (one entry per part, 0 = the part's own value): a
    placed `GENERIC-RESISTOR`'s value for the divider.
  - **Switched level** (plan-55-A). A power switch element whose input has
    a known non-zero level of at least `vinMin` and whose enable is not
    inactive (a logic enable at its inactive level; an `"input"` enable above
    the input) raises a switched level W on its output nets, the input's
    level. The order of levels is S, then R, then W, then the driven D, the
    pulled P, the carried C. Two switch outputs on one net never clash: the
    net reads the higher level (ORing). W different from a supply level or
    from a regulated level on the net is a `VOLTAGE_SUPPLY_CLASH` (one per
    net); an input or enable pin above `vinMax` is
    `VOLTAGE_POWERSWITCH_INPUT_OVER`.
  - `circuit_set_resistance(circuit, part, ohms)` sets that value for a
    resistor part whose manufacturer is `Generic` (more than 0 and at most
    1e9; `part N is not a generic resistor`, `the resistance is out of
    range`); it changes only the voltage check, the engine's resistor stays
    a weak link.
  - `circuit_start` runs it first and refuses a circuit with a problem
    (-1, `circuit_error` = the text; building calls stay usable).
  - A switch that closes during a run and makes the circuit fail stops the
    run like a short: `circuit_advance` returns 0 and `circuit_error` is the
    text (the contact is still applied).
  - A slide switch's closed contacts (plan-58-B) are the same recorded
    contacts: its rest state's are applied by `circuit_start` before the
    check, and `circuit_set_switch_state` runs the check once after the move.
  - `circuit_voltage_problem` returns the first problem (net, part, pin,
    voltages) after either refusal.
  - A supply and GND on one net is still a short (`smoke:`), not a voltage
    problem; a net with no known voltage, and a part with no `vcc` data,
    are not checked.
  - A pull-up part (`PartSupply.pull`) is not a supply here: it gives its
    group a level only when no supply and no powered output sets it
    (E = S, else D, else P, else C), and never clashes.
  - A dual-supply part (`Part.domain_count` 2, plan-20-F) is powered per
    domain: each side at the highest voltage on its own power pins, its
    outputs driving that side's level, and each pin checked against its
    own side's range and input max (`PartSupplyDomain`, docs/parts-format.md
    "Two supply domains"). Every other part takes the single-supply path
    unchanged.
- Lights (`circuit_part_lights`) are LEDs (one light), relays (one light per
  channel, on while operated), buzzers (one light, on while sounding), 7-segment
  displays (`PartDisplay.leds` order) and character LCD backlights (one
  light per LED in `backlight` order, e.g. three for an RGB backlight,
  each on while its anode reads 1 and its cathode 0; a module without a
  backlight has none); a light counts as on if it was on at any time during
  the last `circuit_advance`, so multiplexed digits show.
- Memory contents (SRAM, EEPROM, serial memories): `circuit_memory_size`,
  `circuit_memory_read` (a 0 bit in `known` is X).
- **Microcontrollers** (plan-62). An `mcu` device with a `"model"` runs
  firmware: the model names a register layout of the AVR core
  (`avr_chip.h`: `atmega328`, `atmega168pb`, `atmega328pb`), a C port of
  avr8js 0.21.1, and `mcu_avr.c` puts it on the engine.
  - **Firmware.** The part's memory is its flash, in bytes:
    `circuit_memory_size` gives the flash size and 8 bits,
    `circuit_memory_load` writes it before `circuit_start` (build the image
    with `memory_image_parse` from an Intel HEX or binary file), and
    `circuit_memory_read` reads it back. An unloaded flash is FF; FF words
    are no instruction and run as no-ops.
  - **Startup.** For 1 ms after `circuit_start` the chip is in reset, every
    port pin Z.
  - **Clock**, chosen once at the end of startup (a real chip chooses with
    fuses, which do not exist here). With 2 or more rising edges on PB6
    (XTAL1) during startup the chip takes that clock: one CPU cycle per
    rising edge, PB6 and PB7 are clock pins (never driven; they read 0 in
    PINB), CLKPR resets to 0. Otherwise it runs on its internal 8 MHz
    oscillator divided by 8 (CLKPR resets to 3, the CKDIV8 fuse): 1 us per
    cycle until the firmware writes CLKPR, and PB6 and PB7 are port pins.
  - **Reset.** PC6 is the reset pin, never a port pin: the chip pulls it up
    (weak 1), and while it reads 0 or X the chip is in reset, every port pin
    Z; back at 1 the program starts over (flash and SRAM kept). A watchdog
    reset restarts the chip the same way.
  - **Pins.** An output drives strong 0 or 1; an input is Z, or weak 1 with
    its pull-up. An input reads 1 for a 1 and 0 for 0, X and Z. A pin write
    lands at the time of the instruction that made it. A timer's compare
    output, and a pin toggled from a timer interrupt, change between
    instructions, so one edge can be a CPU cycle early or late; a whole
    period is exact (blink's 1000-cycle toggle comes at 1001, 999, 1001 …
    cycles in avr8js too).
  - **ADC.** There are no analog levels: an ADC input is 5 V when its net
    reads 1 and 0 V otherwise, against AVCC = AREF = 5 V, so a conversion
    gives 1023 or 0 (PC0–PC5; `ADC6`/`ADC7`, or PE2/PE3 on a PB device).
  - **State.** `circuit_cpu_state` reports it as it does a processor:
    `CPU_RESET` (startup, or the reset pin held) or `CPU_RUNNING`; the
    cycles since reset; 35 registers: `PC` (the byte address), `SP`, `SREG`,
    `R0`…`R31`; and the last instruction (byte address, its 2 or 4 bytes as
    in the HEX file, `avr_dasm`'s text, e.g. `004C  RJMP .-2`).
  - **Not modeled.** Fuses, lock bits, brown-out, sleep modes (SLEEP is a
    no-op), self-programming (SPM), the analog comparator, timer 2's
    asynchronous clock, the supply (the chip runs whatever VCC reads; the
    voltage check judges it). The serial peripherals' pins come with
    plan-62-F. Where avr8js's CPU disagrees with Microchip's instruction set
    manual the simulator follows the manual (7 places, each commented in
    `avr_instruction.c` / `avr_cpu.c`; `avr_core_test` § fixes).
  - **Speed.** A chip on a 16 MHz crystal costs an engine event per clock
    edge, and the crystal toggles its two nets besides.
- Processors (`cpu` kind): `circuit_cpu_state` fills a `CpuState`
  (`cpu_state.h`): status (running, lost, reset, waiting, stopped, halted,
  jammed, bus released), cycles since the last reset ended, the registers
  (none while lost or in reset) and the last completed instruction
  (address, bytes, disassembly). A simulated microcontroller answers too
  (above; up to `CPU_MAX_REGISTERS` = 40 registers). Any other part: -1,
  `part N is not a processor`. The app's processor window reads it on every simulation
  frame, as the memory window reads `circuit_memory_read`.
- Character LCDs (`charlcd` kind): `circuit_lcd_state` fills an
  `LcdState` (`lcd_state.h`): the dot image the display shows now
  (`dots[row * 8 + y][column * 5 + x]`, up to 4 rows, each `LCD_DOT_OFF`,
  `_ON` or `_X`; the cursor blink phase comes from the circuit time),
  `controllers` (1, or 2 for a 40 × 4 with E1 and E2), and per controller
  the busy flag (`busy[k]`: 1, 0 or -1 for X) and DDRAM and CGRAM with
  known masks (`ddram[k]`, `ddram_known[k]`, `cgram[k]`, `cgram_known[k]`;
  k = 0 is E1's, k = 1 E2's). The image does not depend on the backlight
  or V0; the app dims the face by the backlight lights (one per LED). Any
  other kind: -1, `part N is not a character LCD`.
- Converters and RTCs (`converter`, `rtc` kinds, plan-24; the `leddriver` and
  `lcddriver` kinds of plan-56-B run on the same engine and have registers
  too): `circuit_converter_registers` copies the part's registers
  (`ConverterRegister`, `converter_state.h`: name, bus address or -1 for an
  internal one, address space, width, value) and returns their count; any
  other kind: -1, `part N has no registers`. `circuit_rtc_time` fills an
  `RtcTime` with an RTC's own counters (year to second, hundredths when the
  part counts them, weekday 0 = Sunday, running), which a latch does not
  freeze; any other kind: -1, `part N is not an RTC`; before start, `the
  circuit is not running`. The app's register window reads both on every
  simulation frame. A refused query sets `circuit_error`, which the app
  reads as a stop, so it asks only parts of those kinds.
- Serial parts (`serial` kind, plan-12): `circuit_serial_channels` gives
  a part's channel count (0 for any other kind);
  `circuit_serial_sent_count` and `circuit_serial_sent` give the frames a
  channel has sent on its TX pin since start (each the frame's data bits),
  recorded on the line, so loopback, break and a stopped transmitter show
  as they are. `circuit_serial_send` types bytes into a channel: the model
  drives its own RX pin strong, one bit per 16 receive ticks in the
  channel's current format, and releases it after the last stop bit. It is
  refused before start, for a part that is not serial, for a channel that
  does not exist or is not asynchronous, and while another part's output,
  in/out or strong supply pin is on the RX net (pull parts are fine); the
  messages are in `circuit.h`. The app's terminal window is built on these.
- Memory load: `circuit_memory_load` writes words into an EEPROM, a serial
  memory or a simulated microcontroller's flash before `circuit_start`
  (refused after it, and on every other part, SRAM included); words it does not write stay
  erased (FF; an F-RAM's stay unknown). Build the words with
  `memory_image_parse` (`memory_image.h`): every rule and message is in
  `planning/completed/plan-04-B-memory-load.md` §4.3.
- `circuit_net_changes` counts how often a net's value has changed since
  `circuit_start`; compare two reads to see a net that toggled in between.
- `circuit_part_light_time` gives, per light (same order as
  `circuit_part_lights`), the picoseconds it has been on since start;
  divide the increase over a frame by the frame's length for brightness.
- Gate and flip-flop outputs are inertial: a newer drive cancels a pending
  one (pulses shorter than the delay vanish), except that a drive of the
  value already pending, no earlier than it, keeps the pending edge.

## Layout

```
backend/
  include/                 public API: parts.h, circuit.h, voltage.h,
                           memory_image.h
  src/
    engine/                sim.c sim.h net.c net.h (the event engine)
    library/               library.c (load_parts, load_footprints), kinds.c
                           (KINDS[]), kind.h + kind_util.c (the kind
                           interface), footprints.c + footprint.h (parse,
                           check and expand footprints.json)
    models/                one file per kind: gate.c dff.c jkff.c sram.c
                           eeprom.c switch.c led.c display7.c nixie.c dekatron.c lcd.c oscillator.c crystal.c
                           supply.c latch.c, decoder.c (decoders,
                           priority encoders, 7-segment decoders),
                           mux.c + mux.h (multiplexers: the kind and
                           model select), mux_stored.c (models stored,
                           regfile, universal, bus4),
                           arith.c + arith.h (comparators, parity:
                           the kind and models compare, parity),
                           arith_alu.c (models adder, alu181,
                           wallace),
                           charlcd.c (character LCD modules: loader and
                           the HD44780 controller model), charlcd_font.c
                           + charlcd_font.h (character ROM A00),
                           seq.c + seq.h (helpers the sequential kinds
                           share: timing entries, controls, X rules,
                           outputs), and lights.c (on-time tracking for
                           lights);
                           model_pins.h (LED, diode, resistor, transistor,
                           switch and open-drain pin facts for the voltage
                           check);
                           diode.c (the diode kind: loader, the
                           links-only model, plan-29-H);
                           resistor.c (the resistor kind: loader, the
                           weak-links-only model, plan-34-E);
                           transistor.c (the transistor kind: loader, the
                           contact switch model, the rating limits,
                           plan-40-F);
                           regulator.c (the regulator kind: loader, the
                           regulated-supply model, plan-42-G);
                           powerswitch.c (the power switch kind: loader,
                           the supply-gate model, plan-55-A);
                           crystal.c (the crystal kind: loader, the weak
                           clock-on-both-terminals model, plan-44-E);
                           pld.c (the programmable logic kind: loader,
                           passive model, facts only);
                           mcu.c + mcu.h (the mcu kind: loader, the
                           "model" key, the engine model that is passive
                           for a device without a model and forwards to
                           the adapter for one with), mcu_avr.c (the AVR
                           engine adapter: startup, the clock, reset,
                           GPIO, ADC, flash, the processor state);
                           socket.c (the IC socket kind: loader, passive
                           model, plan-51-A);
                           inductor.c, fuse.c, terminals.c + terminals.h and
                           model_pins.c (the inductor and fuse kinds: loaders,
                           the closed-contact models, the blowable position, the
                           shared contact sets and the fixed-contact
                           dispatch for the voltage check, plan-47-I);
                           relay.c, buzzer.c (the relay and buzzer kinds:
                           loaders, the 1 ms armature model, the lights
                           hooks, plan-47-J);
                           cpu.c + cpu.h (the cpu kind: loader, the model
                           that forwards to a core adapter, input sampling,
                           the instruction tracker), cpu6502.c (the
                           6502/65C02 bus adapter), cpuz80.c (the Z80 bus
                           adapter: a commit scheduler around the core),
                           cpu65816.c (the W65C816S bus adapter: the bank
                           on D0-D7, VDA/VPA, E and MX, ABORTB, on the
                           jsmooch core);
                           chips_m6502.c chips_w65c02.c chips_z80.c
                           chips_dasm.c (each instantiates one vendored
                           floooh/chips core, or the three disassemblers);
                           serial.c + serial.h (the serial kind: loader,
                           model table, the model that forwards to a serial
                           model, the terminal hooks), serial_line.c (baud
                           generator, transmitter, receiver, sent log,
                           terminal sender), serial_bus.c (Intel host bus),
                           ns16550.c (the 16450/16550 family);
                           pio.c + pio.h (the pio kind: loader, model
                           table, the model that forwards to a pio model),
                           bus65.c + bus65.h (the 65xx peripheral bus the
                           pio models share)
                           avr.h, avr_cpu.c, avr_instruction.c (plan-62: the
                           AVR microcontroller core, a C port of avr8js
                           0.21.1, MIT, LICENSE-avr8js; the CPU, its
                           interrupt queue and clock events, and every
                           instruction), avr_gpio.c (a port's PIN/DDR/PORT,
                           external and pin-change interrupts), avr_clock.c
                           (CLKPR), avr_timer.c (the 8- and 16-bit timers),
                           avr_usart.c, avr_spi.c, avr_twi.c (the serial
                           peripherals at register level: a byte out and a
                           byte in through callbacks, no pins), avr_adc.c,
                           avr_eeprom.c, avr_watchdog.c; avr_dasm.c (our
                           disassembler, for the processor window);
                           avr_chip.c + avr_chip.h (a chip: a CPU and the
                           peripherals of a register layout; the tables
                           of the layouts atmega328, atmega168pb and
                           atmega328pb, with vectors as numbers that
                           avr_layout_scale turns into word addresses for
                           a chip's vector size);
                           buffer.c + buffer.h (the buffer kind: models
                           tristate, transceiver and translator, supply
                           domains; the port read-back rule),
                           buffer_reg.c (model registered)
                           serialmem.c (the serialmem kind: loader and the
                           SPI EEPROM, F-RAM and flash models on the
                           serial_target.h SPI front end; the
                           "simulated": false DataFlash and page EEPROM
                           keep a passive model);
                           serial_target.c + serial_target.h (SPI and I2C
                           target front ends the serial and serialmem
                           models share)
    circuit.c
    voltage.c              the voltage check (voltage.h)
    memory_image.c         load files (binary, Intel HEX) to memory images
    third_party/cjson/     cJSON v1.7.18
    third_party/chips/     floooh/chips m6502.h w65c02.h z80.h and their
                           disassemblers (zlib), unmodified; README.md
                           names the commit
    third_party/jsmooch/   jsmooch's cycle-stepped WDC 65816 core and
                           disassembler (MIT), fixed through its code
                           generator, plus our helpers/ stubs;
                           README-jsmooch.md names the commit and lists
                           every change
  tools/jsmooch-gen/       the 65816 core's JavaScript code generator (MIT)
                           and run.js, which regenerates
                           wdc65816_generated_opcodes.c (`node run.js`);
                           outside src/, so the app does not bundle it
  tools/avr8js/            avr8js 0.21.1's JavaScript (MIT), unmodified:
                           the oracle the AVR core is checked against;
                           README.md names the tarball and its hash
  tools/avr8js-oracle/     oracle.js, which runs it and prints its behavior
                           in the tests' text formats; outside src/, so the
                           app bundles neither
  tests/                   CTest programs; golden/ holds the regression
                           fixtures (one listing line and one trace hash
                           per part); cpu/ holds the SingleStepTests
                           subsets (MIT) for cpu_core_test and the script
                           that made them
```

The app compiles `backend/src/` directly (an Xcode synchronized folder).

## Adding a kind

A new kind of part is added the same way every time:

1. `backend/include/parts.h`: a `PART_<KIND>` value and, if the app needs
   facts about it, an info struct and its pointer on `Part`.
2. `backend/src/models/<kind>.c`: the `PartKindOps` (device and part keys,
   `load_device`, `bind_pinout`, `load_part`, and optional `pin_open_drain`
   for a kind with open-drain outputs, which the voltage check asks through
   `part_pin_open_drain`) and the engine callbacks
   (`SimComponentType`; the engine config is the `const Part *`, model data
   through `part_internal(part)`). Use the `KindContext` helpers in
   `kind.h` for keys, pins by name, times and errors.
3. One line in `KINDS[]` (`backend/src/library/kinds.c`).
4. `schematical/schematical/Parts/<kind>.json`, and its section in
   `docs/parts-format.md`.
5. The app picks a symbol for the new `PartKind`.
6. **The Part Editor** (plan-46), so File › New Part… and Copy to Editor…
   work for the kind:
   - a `KindInfo` entry in `schematical/schematical/Editor/KindInfo.swift`
     (plain name, one-line blurb, one to three bundled example parts, the
     picker group, `simulated: false` for a facts-only kind) and the kind's
     name in `PartKinds.all` and `infoOrder`; `PART_<KIND>` in
     `BundledPartFile.name(of:)` and the kind's name in `PartFacts`;
   - `"<kind>": ["<kind>.c", …]` in `scripts/part-editor-schema.py`
     `KIND_FILES` and in `PartKindKeysTests.files` (the model sources whose
     `KEYS[]` arrays are the kind's keys);
   - `python3 scripts/part-editor-schema.py . <kind>` writes
     `Editor/<Kind>Draft.swift` from the sources and the shipped parts (it
     must print `missing []`; a key the sources accept and no shipped part
     uses goes in `scripts/part-editor-schema-extra.py` `EXTRA`; the script
     makes a `model` or `type` key every device has the schema's
     `variantKey`, which then needs a `variantBlurbs` entry per value, and a
     kind in its `NO_VARIANT` set, `relay`, gets none). A kind
     with no shipped parts gets a hand-written schema instead (the
     `GenericDraft.swift` precedent);
   - a kind whose device object has a section the loader checks inline (no
     `KEYS[]` array: the `display` of `leddriver` / `lcddriver`) is
     hand-written too, as a copy of the kind it shares a loader with, the
     section a required `.json` field (`LeddriverDraft.swift`), and
     `PartKindKeysTests.notCalled` skips the section's own arrays;
   - the schema in `PartKinds.schemas` (`Editor/KindSchema.swift`);
   - `test<Kind>` and the kind's shipped part count in
     `PartKindRoundTripTests` (0 mismatched, 0 with extras, 0 rejected).

   That is all the Part Wizard needs: its steps for the kind are derived
   from the schema (`WizardPlan.derivedSteps`: the model picker when the
   schema has a `variantKey`, Supply voltage, Essentials, one optional step
   per group or list key, Other settings), and `WizardStepsTests` checks
   that every key of every model lands in exactly one step. Optionally:
   - a curated guide, `Editor/Wizard/Guides/<Kind>Guide.swift`, registered
     in `PartKinds.guides`: named steps with intros and the datasheet
     section titles to look for, and a `FieldGuide` (plain label, help,
     where to find it, other names, an example) per key. A kind with
     models can lay its steps out with `GuideLayout.byRule`;
   - for a kind with models, one sentence per model in
     `PartKinds.variantBlurbs` (`Guides/VariantBlurbs.swift`);
     `KindGuideTests` requires one for every model of every kind;
   - a `FieldCondition` table in `PartKinds.conditions` when some keys of
     an object only matter for one value of a sibling key (the converter's
     `bus` keys by `style`).

**A new key on an existing kind.** Add it to the model's `KEYS[]` array and
its loader, then regenerate the kind's schema (`python3
scripts/part-editor-schema.py . <kind>`; add an `EXTRA` line when no
shipped part uses the key yet). `PartKindKeysTests` fails until the form has
a control for it. A kind with derived wizard steps needs nothing more. A
curated kind's `KindGuideTests` then fails with `<kind>: no step of the
guide has device.<key>`: add the key to a step of
`Guides/<Kind>Guide.swift` (and a `FieldGuide` for it; a required key must
say where to find it).

A kind with no simulation model (plan-46's `generic`, `pld`; an `mcu`
device without a `"model"`, plan-36)
sets `"simulated": false` on its devices, so `circuit_add_part` refuses its parts, and still
ships a passive model (no callbacks, the `header.c` shape) so the engine
never sees a NULL model.

Nothing else: the loader, the listing, the circuit API and the app's
drawer work from the kind table and the `Part` structs. The tests need
three lines: the kind's name and facts in `tests/support/listing.c`,
`TRACE_LAST_KIND` in `tests/trace_check.c` moved to the new last
`PartKind` (its per-kind count array is sized by it), and the same last
`PartKind` in the per-kind arrays of `tests/listing_check.c`.

## Tests

`cmake -S backend -B /tmp/logic-build && cmake --build /tmp/logic-build && ctest --test-dir /tmp/logic-build`

- `library_check` — the loader's rules, every rejection case.
- `listing_check`, `trace_check` — every part's facts and a hash of its
  simulated behavior against `tests/golden/` (recorded from the original C
  tables before the move to JSON). A change to a part file that changes
  facts or behavior, or adds a part, shows up here. For an intended change,
  regenerate the goldens and review the diff:

  ```
  /tmp/logic-build/listing_check --print > backend/tests/golden/parts.txt
  /tmp/logic-build/trace_check --print > backend/tests/golden/traces.txt
  git diff --stat backend/tests/golden
  ```

  Active-low flags (`"activeLow"`, plan-39) are reviewed with
  `scripts/pin-polarity.js` (`--help`), whose `golden-check` is the gate for
  a regeneration that should only add flags: it fails unless every changed
  `parts.txt` line differs only by `:low` / `:low=…` and `traces.txt` and
  `footprints.txt` did not move.
- `footprint_check` — every footprint in `footprints.json`, expanded to
  pads, against `tests/golden/footprints.txt` (one line per footprint:
  `id|pins=N|body=WxH|pads=<designator>:x,y,wxh,shape[,d<drill>][,s<slotLength>][,npth][,opp][,r<radius>][,c<chamfer>:<corners>];…[|edge]`,
  `s` a slotted hole's length, `npth` a non-plated hole, `opp` a pad on the other face of the board, `|edge` a footprint that straddles the board edge,
  corners `tl+tr+br+bl` order; `-`
  for an unnumbered pad), and every footprint used by some part (`footprint`
  or `connectorFootprint`). After an
  intended change: `/tmp/logic-build/footprint_check --print >
  backend/tests/golden/footprints.txt`, then hand-check the changed lines
  against the drawing. Listing lines in `parts.txt` end in
  `|footprint=<id>` for parts that name one, then `|connector=<id>` for
  parts with a `connectorFootprint`; the last field is `|unsimulated` for a
  part whose device says `"simulated": false` (plan-36-F). Each pin of a
  listing line is `designator:name:role:group`, then `:in`/`:out` for a
  `side`, then `:low` when its `activeLow` covers every alternative of the
  name or `:low=A;B` (the flagged alternatives, `;` separated, name order)
  when it names some (plan-39-A).
- `part_file_test` — `load_part_file` (plan-32-A) on top of the bundled
  library: an inline-footprint part and a bundled-footprint part load
  (found by name and by uuid), and each rejected file (no or bad uuid, two
  devices or parts, an inline id equal to a bundled one, a footprint pad
  missing, a part rejected after its footprint checked, an unknown key)
  reports its message and leaves the part and footprint counts unchanged.
- `switch_test`, `sram_test`, `display7_test` — model benches.
- `slide_test` (plan-58-B) — the switch kind's `states` / `rest` keys: the
  loader and its error messages, and an SPDT On-On, a DPDT On-Off-On and an
  SPST On-Off in a circuit (LEDs, nets), unwired pins, refused states, and the
  voltage check on a move.
- `nixie_test` — the `nixie` kind on fixture tubes: the loader's rules and
  rejections, the model (a glyph glows while the anode reads 1 and its
  cathode 0; two anode pins of one name are joined), and the voltage check
  (the anode's level carried to the cathode nets, `VOLTAGE_NIXIE_UNDER` for
  an anode below the firing voltage).
- `lcd_test` — the `lcd` kind on a fixture glass next to the real part files: the loader's rules
  and rejections, the model (a cell is lit while its segment and backplane pins are driven and
  differ; a Z backplane lights nothing) and every shipped glass.
- `display_out_test` — the `leddriver` and `lcddriver` kinds (plan-56-B) on
  fixture devices next to the real part files: an SPI 4-digit `digits` driver
  (scan order and slot time in simulated time, raw and decoded bytes, a decode
  switch per digit, blank, test and shutdown), the codeB, bcd and ascii7
  decodes, `static` groups, `ports` (direction field, invert, open drain),
  a `matrix`, and I2C `lcd` drivers in modes 1:4, 1:3 (backplane-major,
  MSB-first RAM) and static wired to the real GENERIC-LCD glass (the lit cells
  equal the RAM bits, the backplanes alternate and are released in turn, the
  enable bit releases every pin), then the `display` section's rejections
  (unknown driver, register, field or pin; counts; a `display` on a
  `converter` or `rtc`). A few of the same rejections are in `library_check`.
- `dekatron_test` — the `dekatron` kind: the loader's rules and rejections
  (on fixture tubes), the model on the real GC10B and OG-8 (a glow that
  steps one position for two overlapping guide pulses, counts backward for
  the other order, ignores a single or a non-overlapping pulse, goes out with
  the anode and strikes at cathode 0 again; a single-pulse tube counts one
  per pulse), every shipped dekatron's ten positions, and the voltage check
  (the anode's level carried to the cathodes, `VOLTAGE_NIXIE_UNDER`).
- `latch_test` — the `latch` kind's five models, one case per function-table
  row of the devices that use them (inline fixture devices copied from the
  real ones), plus loader errors and the open-drain answer. Its bench,
  `tests/support/bench.{h,c}`, drives any part's input pins by name and is
  shared by the sequential kinds' tests.
- `register_test` — the `register` kind's `parallel` model (plan-37-C):
  one case per function-table row of each device shape (inline fixture
  devices with TI's pin names: 74_173, 377, 379, 534, 575, 823, 16820,
  16823), the X rules, loader errors and the five plan-37 families; then
  real parts from `register.json` (plan-37-D, E).
- `mux_test` — the `mux` kind's models (plan-21-B, C): the function-table
  rows of each device shape (inline fixture devices copied from the real
  ones in `planning/plan-21-data/specs.md`), the X rules, storage and the
  bus turnaround. Its loader errors are in `library_check`.
- `arith_test` — the `arith` kind's models (plan-22-B, C): every word
  pair and cascade level of the 4-bit comparators, the 8-bit comparators'
  function tables, the '885 latch and L/A, every input pattern of the
  parity devices and the '286 bus, the '283 and '275 over every input and
  the '181 over every (S, M, Cn) row and (A, B) pair (inline fixture devices copied from the
  real ones in `planning/plan-22-data/specs.md`), each against a reference
  written from the datasheet, plus the X rule. Its loader errors are in
  `library_check`.
- `decoder_test` — the `decoder` kind's three models (plan-19-B): one case
  per function-table row of each device shape (inline fixture devices copied
  from the real ones, plus '131 and CD4056B shapes), X rules and the
  open-drain answers. Its loader errors are in `library_check`.
- `serialmem_test` — the `serialmem` kind (SPI EEPROM, F-RAM, flash) on
  inline fixture devices (no shipped part is needed), a bit-level SPI master
  in modes 0 and 3 on a bench: every opcode of every model, the page wrap,
  block and sector protection, WEL, WP#/WPEN rules, busy times (RDSR during a
  cycle, a second write ignored), writes cut off a byte boundary, HOLD#,
  power-up contents, X handling, the F-RAM sleep and ID, the flash locks,
  reset and deep power-down, the `"simulated": false` devices (facts, the
  `|unsimulated` listing line, `circuit_add_part` refusing them), the memory
  hooks through the circuit API, 60000 random pin changes per chip, and every
  loader rejection of the kind.
- `eeprom_test` — the EEPROM model's read window, page loads, write cycle,
  DATA# polling and toggle bit, timing violations and software data
  protection (an inline fixture device, then real parts).
- `engine_test` — engine timing with a real gate (SN74HC32N): an output
  edge that is already pending is kept, a too-short pulse is swallowed.
- `diode_test` — diode links (plan-29-H): the ten engine cases of the
  Diodes table above with an engine-only fixture diode, a diode-OR run,
  then the fixture diode parts (`tests/support/diode_fixture.h`) in
  circuits: a tab tied to its lead, a diode-OR in a library circuit, and a
  switch contact inside a cluster. The kind's loader errors are in
  `library_check`, its voltage carry in `voltage_test`.
- `resistor_test` — resistor links and the resistor kind (plan-34-E): the
  eight engine cases of the Resistors table above with engine-only fixture
  resistor and diode components; then the fixture parts of
  `tests/support/resistor_fixture.h`: facts (a two-pin part, a 4-element
  array's pairs, a 0 Ω bussed array), the loader's rejections, and
  circuits: a resistor pull-up into an SN74HC04N input, an SN74HC04N
  lighting a GENERIC-LED through a resistor, 5 V through a resistor
  refused by a 3.3 V input, and a bussed array's two common pins as one
  node.
- `transistor_test` — the transistor kind (plan-40-F): the fixture parts
  of `tests/support/transistor_fixture.h` load and their `PartTransistor`
  facts read back; the loader's rejections; the engine cases of the
  Transistors table above (each fixture part's model in a rig with a
  scripted driver); an SN74HC04N lighting a GENERIC-LED through an NPN;
  and the rating test (`VOLTAGE_TRANSISTOR_OVER`: a drain above `vds`, a
  gate above `vgs`; a BJT base and a part with no rating never refused).
- `regulator_test` — the regulator kind (plan-42-G): the fixture parts of
  `tests/support/regulator_fixture.h` load and their `PartRegulator` facts
  read back; the loader's rejections; the engine cases of the Regulators
  table above; and the voltage rules: an LDO's regulated level seen
  through the parts it powers and through a clash with a 1.2 V supply, a
  VIN above the maximum, a clash with a rail, the feedback divider from
  maker resistors and from `GENERIC-RESISTOR` values
  (`circuit_set_resistance`), a boost converter's `vinMin`, an enable at
  its inactive level, a chain of two regulators.
- `powerswitch_test` — the power switch kind (plan-55-A): the fixture parts
  of `tests/support/powerswitch_fixture.h` (a load switch, current-limited
  switches, an ideal diode, a dual ideal diode, a quad switch) load and their
  `PartPowerSwitch` facts read back; the loader's rejections; the engine
  cases of the Power switches table above; and the voltage rules: the
  switched level seen through a clash with a 1.2 V supply, ORing, a clash with
  a rail and with a regulator, chains, `vinMin`, a comparator enable, the
  input rating. `voltage_test` adds the cases of a circuit without a power
  switch (unchanged) and the same rules through `voltage_check`.
- `crystal_test` — the crystal kind (plan-44-E): the fixture parts of
  `tests/support/crystal_fixture.h` (a two-pin and a four-pad crystal, a
  100 MHz one, a three-pad resonator with built-in capacitors) load and
  their `PartCrystal` facts read back; the loader's rejections (a missing or
  out-of-range `hz`, an unknown type, wrong terminals, a terminal that is not
  `inout`, a non-terminal that is not `nc`, 5 pins, a fact out of range,
  `capacitors` on a crystal, an unlisted key, a footprint with the wrong pad
  count); and the clock: X1 at the level and X2 at the complement from
  time 0 (1 MHz, and 32768 Hz with its 15258789 ps half period), a strong
  GND on a terminal overriding it, a terminal wired to an SN74HC04N input
  (the output is the complement, 20 ± 1 changes in 10 us), 100 MHz for 1 us
  without an error, and an unwired crystal.
- `inductor_test` — the inductor kind (plan-47-I): the fixture parts of
  `tests/support/inductor_fixture.h` (a 10 uH inductor with every fact, a 1 nH
  one with none, a four-pad one) load and their `PartInductor` facts read
  back; the loader's rejections (a missing or out-of-range `henries`, an
  unknown shielding, an `amps` out of range, wrong terminals, a terminal that
  is not `inout`, a non-terminal that is not `nc`, an unlisted key); and the
  circuits: 5V or GND on pin 1 reads on pin 2, an SN74HC04N output through it
  lights an LED, 5V and GND across it is `smoke:`, the four pads are one pair
  of nodes, `circuit_set_switch` is refused, and 12 V through it onto a
  74HC04's VCC is a `VOLTAGE_POWER_RANGE` problem on the 74HC04.
- `fuse_test` — the fuse kind (plan-47-I): the fixture parts of
  `tests/support/fuse_fixture.h` (a 1 A fast fuse and a 6 V PTC) load and
  their `PartFuse` facts read back; the loader's rejections (an unknown type
  or speed, a missing `amps`, no voltage key, a key of the other type, `trip`
  not above `hold`, wrong terminals and pin roles, an unlisted key); and the
  circuits, for both parts: conducting, an LED behind it blown and restored
  during a run and blown before the start, position 2 refused, 5V and GND
  across it (`smoke:` intact, quiet blown), the `VOLTAGE_FUSE_OVER` rating
  (`voltage: 12 V on pin 1 (1) of TESTPTC; it is rated 6 V`) and the voltage
  group of a fuse, blown or not.
- `relay_test` — the relay kind (plan-47-J): the fixture parts of
  `tests/support/relay_fixture.h` (SPDT, DPDT, polarized, latching with one
  coil and with two, with a common coil pin, a normally-open-only pole, two pins
  named COM, a solid-state relay and a two-channel one) load and their
  `PartRelay` facts read back; the loader's rejections (device, pin and part);
  and the timed cases on an engine rig: at rest, operate and release at 1 ms, a
  0.5 ms pulse, reverse drive and a polarized coil, a floating coil pin, both
  poles of a DPDT in one event, the latching arrangements, the solid-state
  channels and their lights; and in circuits a tie between pins of one name, a
  relay that interrupts its own coil, a transistor driving the coil, and the
  voltage check at rest.
- `buzzer_test` — the buzzer kind (plan-47-J): the fixture parts of
  `tests/support/buzzer_fixture.h` (an indicator and a piezo transducer) load
  and their `PartBuzzer` facts read back; the loader's rejections; the light and
  its on-time for every combination of the two pins, between two gate outputs,
  and under a scripted 1 kHz square wave (5 ms of on-time in 10 ms).
- `capacitor_test` — the capacitor kind (plan-33-E): the fixture
  capacitors (`tests/support/capacitor_fixture.h`, one tantalum and one
  ceramic device) load and their `PartCapacitor` facts read back; a
  capacitor between 5V and GND drives nothing (both nets keep their values
  after 1 us). The kind's loader errors are in `library_check`, its rating
  check (`VOLTAGE_CAPACITOR_OVER`) in `voltage_test`.
- `mcu_test` — the mcu kind (plan-36-F): the fixture microcontroller
  (`tests/support/mcu_fixture.h`, `TESTMCU`) loads, its `PartMcu` facts
  read back (the part's clock over the device's), its listing line ends
  `|unsimulated`, and `circuit_add_part` refuses it with `part "TESTMCU"
  is not simulated`. The kind's loader errors are in `library_check`.
  Then the model devices (plan-62-E): `TESTAVR` and `TESTAVR48` (model
  `atmega328`, the p28 pinout) run firmware from `tests/avr/<name>.hex`
  (sources in `tests/avr/src/`, built by `build.sh`) in circuits of library
  parts, with times in picoseconds: every port pin Z during the 1 ms
  startup and PC6 at 1; `blink` toggling PB5 every 1000 cycles on average on
  the internal clock (1 us per cycle), on a `GENERIC-CRYSTAL-16M` (62.5 ns)
  and after CLKPR = 0 (125 ns), an LED on PB5 lit 50 ms of 100; a switch on
  PD2 against the pull-up; OC0A high 64 and low 192 of 256 cycles; INT0 on
  the falling edge; PC6 to GND releasing every pin and the program starting
  over; one-word vectors on the 4 KB chip; the flash through
  `circuit_memory_size` / `_read` / `_load` and an erased chip running; the
  ADC reading 1023 and 0; `circuit_cpu_state` in reset and running.
- `pld_test` — the pld kind: the shipped ATF22V10C, ATF750C and ATF16V8B
  parts read back their `PartPld` facts (macrocells, the part's tpd, a
  commercial part's own `vcc`), single part files cover every loader rule
  (`"simulated"`, `macrocells`, `tpd`, a supply range, a power and a ground
  pin), the listing line ends `|unsimulated`, and `circuit_add_part` refuses
  the part.
- `circuit_check` — the circuit API, including the voltage refusal at
  start, the running switch stop (fixture parts from
  `tests/support/voltage_fixture.h`) and the refusal of a part whose device
  says `"simulated": false` (`part "<name>" is not simulated`; a header
  device with the key, and the same device without it, which is added).
- `voltage_test` — `voltage_check` and `voltage_describe`, one case per
  rule (supplies, power range, input max, driven and carried voltages,
  open-drain, closed contacts, switch rating, capacitor rating, ordering, and the two supply
  domains of a translator: each side powered from its own pin, its pins
  judged against its own range), on the fixture parts and the real
  supplies.
- `memory_image_test` — the load-file parser: format by file name, raw
  binary, every Intel HEX rule and message.
- `cpu_core_test` — the vendored CPU cores (`third_party/chips/`) against
  `tests/cpu/`: the first SingleStepTests case of every opcode file (6502
  256, 65C02 254, Z80 1604; `make_sst_subset.py` regenerates them). The
  6502 family is checked cycle by cycle (address, data, R/W) and on its final
  registers and RAM; the Z80 on its final registers, RAM, T-state count and
  bus transactions. The test names the cores' known deviations (6502 JAM and
  SHA/SHY/SHX, Z80 SCF/CCF, interrupted block repeats, HALT). Each of those
  must still fail, so a core update that fixes one is noticed; any other
  failure, or a listed test that passes, fails the test. It prints
  `6502: 241 pass, 15 expected failures` and the same for 65C02 and Z80.
  The 65816 (`third_party/jsmooch/`, plan-14-G) runs the first 10 tests of
  each of the 512 SingleStepTests/65816 files (`sst-65816.jsonl`, 5120;
  `make_sst_subset.py … 10`), checking every cycle's 24-bit address, data
  and the eight flags VDA VPA VPB RWB E M X MLB, then the final registers
  and RAM: `65816: 5120 pass, 0 expected failures`. Then an ABORT case
  (`65816: abort ok`) and, when CMake finds `node`, a check that
  `tools/jsmooch-gen/run.js` reproduces the committed
  `wdc65816_generated_opcodes.c` (`65816: generator reproduces …`). Set
  `SST65816="<test name>"` to print one test's cycles side by side.
- `avr_core_test` — the AVR CPU core (plan-62-A; `avr_cpu.c`,
  `avr_instruction.c`) against avr8js. **step**: every opcode word 0…65535 in
  4 random states each (one xorshift32 seed, the same in the test and in
  `tools/avr8js-oracle/oracle.js step`), one instruction per state, one line
  per vector (opcode, pc, cycles, SREG, SP, a hash of the data memory). The
  lines must hash to the `step` line of `tests/avr/golden.txt` and, when
  CMake found `node`, equal avr8js's own lines; the first difference prints
  as `step: opcode XXXX state N: avr8js "…" ours "…"`. It prints `step:
  262144 vectors match avr8js` and `step: 99 of 99 decode branches ran`. The
  core runs in *exact* mode there (`AvrCpu.exact`): avr8js's behavior
  everywhere. **fixes**: the 7 places where the simulator follows Microchip's
  instruction set manual instead of avr8js, each against a reference written
  from the manual: H from the low nibble's carry (ADD, ADC, SUB, SUBI, SBC,
  SBCI, CP, CPC, CPI, NEG; avr8js uses bit 0), SBIW leaving H alone, Z of
  CPC/SBC/SBCI when the result wraps to 0, V of NEG, Z of FMULSU, CBI's 2
  cycles, an interrupt's 4 cycles (`fixes: 7 ok`). **unit**: clock events
  (order, one per tick, update, clear, more than avr8js's pool of 10) and the
  interrupt queue. `avr_core_test --print-golden` prints the golden line from
  the oracle's output.
- `avr_periph_test` — the AVR peripherals and the chip assembly (plan-62-B)
  against avr8js. **layout**: the `atmega328` layout printed as JSON
  (`avr_periph_test --layout atmega328 32768`) must equal avr8js's own
  ATmega328P configs (`oracle.js layout-check`; `layout atmega328 equal`).
  **scenarios**: `tests/avr/<name>.json` holds a program and pin stimuli; the
  C chip runs it in exact mode and prints a trace, one line per instruction
  (`i <pc> <cycles> <SREG> <SP> <hash of the data memory>`) and one per port
  output change (`p <cycles> <port> <value> <DDR>`). The trace must hash to
  the scenario's `run` line in `tests/avr/golden.txt` and, with `node`,
  equal the trace `oracle.js run` prints from avr8js on the same layout; the
  first differing line prints with its number. It prints `ok <name> (<lines>
  lines, <n> port events, <n> interrupts, <n> serial events)`. Scenarios:
  `gpio_out`, `gpio_int`, `clkpr`, `timer0`, `timer1`, `timer2`, `timer_ext`,
  `usart`, `spi`, `twi`, `adc`, `eeprom`, `watchdog`, and for the other
  devices `m48_vectors` (4 KB of flash: one word per vector), `m168pb_porte`
  (layout `atmega168pb`: port E) and `m328pb_extras` (layout `atmega328pb`:
  timers 3 and 4, USART1, SPI1, TWI1, PCINT3). A scenario may list vector
  numbers that must be taken (`m328pb_extras`: 27, 28, 30, 33, 35, 39, 40,
  42, 44). The serial peripherals'
  outside is scripted the same way on both sides: a USART byte prints `u
  <cycles> <n> <byte>`; an SPI byte prints `s …` and comes back XOR 0xA5
  after the transfer time; a TWI request prints `t <cycles> <n> <event>
  <argument>` and is answered 40 cycles later (address 0x50 acknowledges,
  reads return 0x10, 0x11, …). Stimuli: `pin`, `rx` (a byte arrives at a
  USART), `adc` (a channel's millivolts). One case runs outside exact mode:
  `ok watchdog_full_reset` (a watchdog reset restarts the whole chip; avr8js
  resets only the CPU). A scenario
  is built from `tests/avr/src/<name>.asm` (avr8js's assembler: `_LOC
  <byte address>`, labels, `;` comments) and `<name>.meta.json` (written by
  `src/make_meta.py`: layout, sizes, steps, stimuli); `sh
  backend/tests/avr/src/build.sh` rebuilds them all, `python3
  backend/tests/avr/src/regen_golden.py <build dir>` rewrites the `run`
  lines from the oracle, and with `node` the test fails when a committed
  `.json` is not what `oracle.js build` makes of its sources.
  `avr_periph_test --trace <name>` prints the C trace. **dasm**: `avr_dasm`'s
  text for every opcode word (`avr_periph_test --dasm`) must assemble back to
  the same word(s) in avr8js's assembler (`oracle.js dasm-check`) and hash to
  the `dasm` golden line: `dasm: 63902 words round-trip` (1570 words are no
  instruction and print `.DW 0x....`, erased flash 0xFFFF among them; 64 are
  a JMP or CALL above the assembler's 4 MB limit and are not checked).
- `cpu_test` — the cpu kind's bus adapters on inline fixture devices (the
  W65C02S and MOS 6502 data of `cpu.json`) and a bench that clocks the CPU
  at 1 MHz and serves a 64 KB memory (X where it was given nothing): the
  power-up LOST state, the reset sequence, the address and write windows at
  the datasheet times, setup violations and used unknowns (LOST), IRQ, NMI,
  RDY, SO, the register and instruction read-back, clock-width checks,
  and the W65C02S VPB, MLB, BE, WAI and STP and NMOS JAM rules; plus
  `circuit_cpu_state`. A second bench drives a Z84C00 fixture the same way
  and logs every machine cycle (type, address, data, T1, strobe times):
  the fetch, memory, I/O and int-ack waveforms at the PS0178 edges, WAIT,
  IM 0/1/2, NMI, HALT, BUSREQ, data setup and clock checks, and a program
  from reset to HALT. Last, three whole boards of library parts
  (`tests/support/cpu_board.{h,c}`: the CPU, an AT28C256-15PU, an
  AS6C62256-55PCN and an SN74HC00N decoder) run a program for 2 ms: the
  W65C02S6TPG-14 and MPS6502 count to 10 in SRAM, the Z84C0010PEG also
  CALLs a store and halts. The W65C816S cases (`w65c816s_h1` … `h13`,
  plan-14-H) run an inline W65C816S on a 24-bit bench that latches the
  bank at each PHI2 rise: reset and the vector pull, the bank on D0–D7 and
  the write window at the Table 4-2 edges, VDA/VPA/VPB equal to the bare
  core's, MX and E, MLB, ABORTB, RDY, WAI/STP, BE, setup, and the 24-bit
  state and tracker (`ok w65c816s_…`). The W65C816S board (plan-14-I:
  `cpu_board`'s `65816` layout, an SN74HC573AN bank latch and an
  AS6C1008-55PCN) runs a native-mode program that stores to banks 0 and 1
  and reads both back (`board_w65c816s ok`). The I/O board (`tests/support/io_board.{h,c}`,
  plan-14-E) adds a W65C22S VIA, a W65C21N PIA and a W65C51N ACIA (TxD
  looped to RxD, on an ECS-2100A-018) with three SN74HC00N decoders: its
  program sets the ports, sends `HI`, takes a VIA Timer 1 interrupt and
  reads the looped-back byte (`io_board ok`).

- `usbuart_test` — the USB-to-UART bridges (model `usbuart`) on the
  shipped parts, through the circuit API: terminal bytes out TXD and
  looped back into RXD, the 9600 8N1 default waveform, RTS/CTS flow
  control, two channels, reset and the TX LED, the USB supply domain in
  the voltage check, and `circuit_serial_configure`'s errors.

- `serial_test` — the serial kind on fixture devices
  (`tests/support/serial_fixture.h`: a PC16550D-shaped `ns16550` in plcc44
  and dip40, and a 16450 twin) with a bench that clocks XIN and wires
  BAUDOUT to RCLK: reset values, divisor and BAUDOUT, transmit frames and
  bit times, receive (false start, framing error and resync, parity,
  break), FIFO triggers and overrun, the timeout, interrupt priorities and
  the transmitter-interrupt delay, modem status, loopback, DMA modes, bus
  timing and strobes, the 16450, and the terminal (8N1 at 115200 on
  1.8432 MHz sends `H i`; typed `OK` reads back). `circuit_check` drives
  the same UART through DIP switches and pull-ups for the
  `circuit_serial_*` API and its error messages.
- `pio_test` — the pio kind (plan-14). A1–A8 drive the 65xx bus helper
  (`bus65`) through a test-only 16-register component on the bench at the
  W65C22S 14 MHz times: the read window (X at the rise, data at + `tcdr`,
  Z at fall + `thr`), write data setup and hold, select/RS setup and hold,
  not selected, PHI2 X, the per-fall `cycle()` hook and PHI2 width checks.

- `buffer_test` — the buffer kind (plan-20). Fixture devices with the
  real devices' pin names: '244, '240, '541, 1G125/1G126 and an open-drain
  1G125 (tristate); '245, '640, '641, '623 and a two-unit 16245
  (transceiver: DIR/OE, the turnaround read-back rule, X on DIR, the '623
  bus-hold loop); '646, '652, '543, '2952 and a two-unit 16952
  (registered: real-time and stored transfer, storing the bus the part
  drives, CE gating a latch and its outputs, clock enables, X on a clock);
  the loader's errors.

Not a ctest: `sim_bench` (`tests/sim_bench.c`) measures engine speed — a
D flip-flop ripple counter on an oscillator plus N idle SN74HC00N chips,
reported as a multiple of real time. Build it Release; wall-clock numbers
would make ctest flaky, so it has no `add_test`.

```
cmake -S backend -B /tmp/logic-rel -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/logic-rel --target sim_bench
/tmp/logic-rel/sim_bench LMK6CE001200CDLFR 16 10 400   # <osc> <stages> <sim ms> <idle chips>
```

`cpu_bench` (`tests/cpu_bench.c`, also not a ctest) runs the W65C02S board
of `cpu_test` with its program looping forever and reports the speed:

```
cmake --build /tmp/logic-rel --target cpu_bench
/tmp/logic-rel/cpu_bench 100   # <sim ms>
```

The bench runs on the `3V3` supply, so the oscillator must take 3.3 V
(LMK6CE001200CDLFR is a 1.2 MHz, 2.37-3.5 V part; the 1.8 V-option
LMK6CE001000DDLER is refused at start by the voltage check).
