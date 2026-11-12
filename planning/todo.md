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
- [ ] Add top 3 Resitor manufactuerers and their active resistors
- [ ] Add top 3 Potentiometer Resitor manufactuerers and their active resistors
- [x] Add a Generic Capacitor. Axial (ceramic) default, 0.1pf default (plan-33)
- [x] Add top 3 Capacitor manufactuerers and their active capacitors (plan-33)
- [ ] Add Slide Switches, top 3 manufactuerers and their active switches
- [ ] Add 20x4 and 40x2 and 40x4 character LCD modules
- [ ] Add a KiCad export for the schematic.
- [ ] Add a Gerber export for the PCB

## Datasheets

- [ ] Create a datasheet format (Markdown or JSON), based on Markdown,
      WaveDrom and simple tables. It should include the sections engineers
      need, and can skip marketing sections. Aim for 20–30k documents per
      IC for ICs that aren't microprocessors or microcontrollers. The app
      should render and print it.

## Future

- [*] Contact anson@pcbway.com about api/integration
