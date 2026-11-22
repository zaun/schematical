# To-do

- [X] The osc block should have a square wave, not a sine wave.
- [X] Regroup logic ICs in the part picker. It's too much of a catch-all.
- [ ] Review non-gate logic blocks: pin grouping is missing or wrong. A lot
      of the time the pins are in pin order, not level order.
- [X] Delete should only work on click, not drag.
- [X] Remove the load and save buttons from the toolbar.
- [X] Add a space before the power tools in the toolbar. All the tools
      affected by the voltage groups should be together.
- [X] Add a 3rd filter, for voltage, at the top of the part picker, under
      the search input.
- [X] Add a text tool to the schematic view, to add text/comments in a
      block. It should support multiple lines.
- [x] Add full undo/redo that works with all actions on the schematic. (plan-31)
      Note (plan-30): a drop that auto-connects loose wire ends to pins must
      undo as one step with the move, and undoing a block delete must put
      its wires back on its pins.
- [x] Add full undo/redo that works with all actions on the PCB. (plan-31)
- [X] Opening a file should open a new tab. It should only replace the
      default blank document if that document is empty.
- [x] On the Diode PCB Footprint add the line so the direction is known
- [x] In the schematic view remove pin numbers from pins that are connected. Leave them for
      disconnected pins.
- [x] Make bus lines 2 pixels thinner
- [x] Multi-word functions should be multi-line in the block in the canvas
      breaking on spaces. "Bridget UART" should be "Bridge\nUART" for example
- [x] USB* receptical should be USB* Port, for example "USB-C Port"
- [x] Make junction dots +1 radius in size
- [x] Remove the Flip button, just use `f` to flip
- [x] Add a proper help screen for schematic and one for pcb with keyboard information
- [x] Add a net label tool, should have "net" in the side bar to chage, 4 chars max
      should have type/stype setting for input (point facing out), output (point facing in),
      and inout (points on both sides)
- [x] Add copy/paste to the schematic
- [x] Text control on schematic and PCB should be monospaced font.
- [x] On the Board Layer config, when Pane is slected add an option fo select the net/power/gound to use.
- [x] Panes should have proper clearence around unlined pins.
- [x] Schematic and PCB should use the same sidebar open/close flag
- [ ] Add a Generic Resitor. Axial default, 1k default
- [x] Add top 3 Resitor manufactuerers and their active resistors
- [ ] Add top 3 Potentiometer Resitor manufactuerers and their active resistors
- [x] Add a Generic Capacitor. Axial (ceramic) default, 0.1pf default (plan-33)
- [x] Add top 3 Capacitor manufactuerers and their active capacitors (plan-33)
- [ ] Add Slide Switches, top 3 manufactuerers and their active switches
- [x] Add 20x4 and 40x2 and 40x4 character LCD modules
- [x] Add a KiCad export for the schematic. (Save As ▸ Format ▸ KiCad Project: schematic and board)
- [x] Add a Gerber export for the PCB
- [ ] Add top 3 power management IC like (MAX803L, etc.)
- [x] Two BUSes with the same name should be able to connect.
- [x] BUS wire name and netlabels should be up to 8.
- [ ] Toggle a BUS name label in schematic view
- [ ] After paste select all that was pasted (including wire ends) so it can be dragged
- [ ] Bus ports are not selectable?
- [ ] Add hex, bcd, etc. decoders
- [ ] Flipped LED doesnt look correct, the "light" arrows look wrong.
- [ ] I would like Rotation to work in the Schematic view for LEDs, Diodes, Resistors, and Capasitors only.
- [ ] Two finger scroll in all tool modes
- [ ] The schematic view can get slow when a lot ot items are selected
- [x] The input can get sluggish/laggy when editing netlabels and or bus wire names when there are a lot already. Same with changing a netlable Type, or adding wires, etc. Dragging the canvas around (scrolling) is fine.
- [x] Change simulation to cmd-P and shift-cmd-R to rotate left
- [x] When a picker category closed, it should auto-close any sub-groups
- [x] Add default power net classes for common voltages (5V and 3V3)
- [x] Add default logic net classes for common voltages (5V and 3V3)
- [ ] For DIP and PLCC chips have a toggle to use a socket footprint. This means each of these needs a secondary part picker, when in socket mode then need a button to show a small dialog listing compatible sockets to choose from. A prereq for this is adding the socket parts to the part picker database.
- [ ] Add card edge sockets

## Manufacturing (a board can't be ordered without these)

- [x] Add an Excellon drill export with the Gerbers (plated and non-plated
      holes, slots), zipped in the layout fabs expect.
- [x] Add a pick-and-place (CPL) export, and a BOM export in the JLCPCB /
      PCBWay assembly format (see planning/jlcpcb.md).
- [x] Print / PDF the schematic, and each PCB layer at 1:1 (for checking
      footprints against real parts and for home etching).
- [ ] Later: an "Order this board" flow (JLCPCB / PCBWay API).

## Parts

- [x] Add BJTs (2N3904, 2N3906, BC547…) and MOSFETs (2N7000, logic-level
      N-channel), top 3 manufacturers.
- [x] Add voltage regulators: 78xx, AMS1117, LM317, LDOs, and common buck
      modules. (plan-42: TI, ADI, Microchip; AMS1117 and buck modules out of
      scope)
- [x] Add hobby microcontrollers: ATmega328P, ATtiny, RP2040, ESP32
      modules. The bus-CPU kind only covers 6502/65816/Z80. These 
      microcontrollers need a `simulated` property set to false. When any
      are added to a schematic the "play" button should be disabled. (plan-36)
- [ ] Add the 555 timer and common op-amps (LM358, TL07x…).
- [ ] Add crystals and resonators (oscillators exist, crystals don't).
- [ ] Add inductors, fuses / polyfuses, relays and buzzers.
- [ ] Add screw terminals and JST-XH/PH connectors (check what the 222
      existing connectors already cover first).
- [ ] Allow parts with a symbol, footprint and BOM data but no simulation
      model, so passives and analog parts don't each need a backend model.
- [ ] Add IO Extenders

## PCB

- [x] Add copper pours on the top/bottom layers (e.g. a GND pour with
      thermal reliefs), not only whole-layer planes. (plan-52)
- [ ] Custom board outlines: rounded corners, cutouts, DXF import.
- [x] Per-net trace widths (net classes): power nets wider by default. (plan-38)
- [ ] Import footprints from KiCad's footprint libraries.
- [ ] Later: teardrops, push-and-shove routing. (autorouter: plan-38)

## Schematic

- [ ] Multiple sheets per schematic, joined by net labels.
- [ ] Electrical rule check (ERC): unconnected inputs, two outputs on one
      net, missing power pins. Extends the voltage check.
- [ ] Renumber references (U1, U2… in order) before building the BOM.

## Examples (8-bit CPU, `examples/8bit-cpu`)

- [ ] Route the AFU PCB (115 unrouted connections in the design rule check).
- [ ] Route the LFU PCB (188 unrouted connections in the design rule check).
- [ ] Fix the AFU's 16 silkscreen design rule check warnings: references over
      0603 cap pads (C1–C9, R1, U44), cap pin-1 marks too close to chip
      pads, D1's pin-1 mark over R2's pad. The LFU had the same set; it
      was fixed by hiding those references and pin-1 marks.
- [ ] Rerun the LFU design rule check: R1's pin-1 mark was hidden after
      the last run, which had that as the only silkscreen warning left.
- [ ] D1 and R2 (the "selected" LED and its resistor) are 0.27 mm apart on
      both the AFU and the LFU. They pass the design rule check, but more room
      would make assembly easier.
- [ ] AFU drives CF and ZF all the time (not gated by /AFU), so no other
      card can use those lines. The LFU uses its own GT and EQ lines
      for that reason. Decide whether flags should be 3-state on a shared
      flag bus instead.
- [ ] FUNC-BUS pin 12 was changed from GND to EQ, and pins 37–40 (were
      unused) are now /LFU, OP0, OP1, GT. Any other card on the bus
      (RegFile, later units) must use the same names and must not ground
      pin 12.

## App issues found while building the LFU

- [ ] New 0603 passives fail the design rule check by default: the reference
      is placed over a pad and the pin-1 mark is too close to a pad. Place
      the reference off the pads (or hide it) for small footprints.
- [ ] Unpolarized parts (ceramic caps, resistors) get a pin-1 mark on the
      silkscreen by default. They shouldn't have one.
- [ ] Check whether a net label named `GND` or `5V` joins the supply rail
      (not only bus wires of that name); docs/file-format.md doesn't say.
      If it doesn't, either make it or say so in the docs.

## Datasheets

- [ ] Create a datasheet format (Markdown or JSON), based on Markdown,
      WaveDrom and simple tables. It should include the sections engineers
      need, and can skip marketing sections. Aim for 20–30k documents per
      IC for ICs that aren't microprocessors or microcontrollers. The app
      should render and print it.

## Reference-board parts still missing

plan-43 added only the 43 parts that fit a kind the library already had.
The parts below have an in-stock DigiKey match and a downloaded datasheet
(`datasheets/reference-boards/<family>/`, list in
`planning/plan-43-data/candidates.tsv`, column `family`) but were not
added. One item per family; 248 parts in all.

- [ ] regulator: 45 parts (plan-42).
- [ ] inductor: 26 parts.
- [ ] connector (other than the two USB-C receptacles plan-43 added): 22 parts.
- [ ] protection: 18 parts.
- [ ] bus-switch: 17 parts.
- [ ] serial-memory: 16 parts.
- [ ] mcu: 14 parts.
- [ ] power-management: 14 parts.
- [ ] amplifier: 13 parts.
- [ ] transceiver: 12 parts.
- [ ] sensor: 10 parts.
- [ ] crystal: 8 parts.
- [ ] driver: 8 parts.
- [ ] electromechanical: 6 parts.
- [ ] rf-passive: 5 parts.
- [ ] display: 4 parts.
- [ ] fuse: 4 parts.
- [ ] power-module: 3 parts.
- [ ] optocoupler: 2 parts.
- [ ] capacitor: 1 part.
- [ ] The 9 parts plan-43 skipped for their datasheets
      (`datasheets/skipped-parts.md` § Reference boards (plan-43)).
- [ ] About 600 more missing part names had no in-stock match with a
      datasheet (house part numbers, values such as `SMA_H` that name no
      part). Each needs a real part chosen for it, or to be left out
      (`routing-example/missing.csv`, `routing-example/datasheets-log.tsv`).
- [ ] No reference board has every part in the library yet
      (`python3 routing-example/parts_check.py --dry-run`, 2026-11-15:
      0 of 202 boards, 1668 distinct parts missing).
