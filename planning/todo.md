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
- [ ] Add a KiCad export for the schematic.
- [X] Add a text tool to the schematic view, to add text/comments in a
      block. It should support multiple lines.
- [ ] Add full undo/redo that works with all actions on the schematic.
- [ ] Add full undo/redo that works with all actions on the PCB.
- [X] Opening a file should open a new tab. It should only replace the
      default blank document if that document is empty.

## Datasheets

- [ ] Create a datasheet format (Markdown or JSON), based on Markdown,
      WaveDrom and simple tables. It should include the sections engineers
      need, and can skip marketing sections. Aim for 20–30k documents per
      IC for ICs that aren't microprocessors or microcontrollers. The app
      should render and print it.
