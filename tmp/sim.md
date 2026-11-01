# Play/Stop and simulator hookup

Plan for adding a Play/Stop control that runs the backend simulator on the
schematic, colors nets and LEDs by their state, and locks editing while running.

Status: phases 1 to 5 implemented; phase 6 partly done.

- Built: `xcodebuild` for the macOS scheme succeeds. CMake build plus `catalog_check`
  and `netlist_check` pass.
- Not done: the app has not been launched, so the play-mode lock and the colors
  are unverified by hand.
- Deviations: pins are not colored separately (each pin's color comes from the
  wire at its end). Simulated time runs 10 us per 16 ms tick, so fast clocks
  flicker rather than show every edge.

## Current state

- The Swift app never calls the simulator. The only callers are
  `backend/catalog_check.c` and `backend/catalog_validate.c`.
- `sim_run_until` is a one-shot event run. There is no clock part. The only
  sources are VCC and GND, which drive once at time 0, so a run settles once and
  stops.
- Nets cannot be added after `sim_start`. Each run needs a fresh `Sim`.
- A strong-driver short sets `smoke:` and `sim_shorted_net()` returns the net.
- A circuit that never settles stops with "circuit did not settle"
  (`SIM_MAX_ROUNDS` = 10000).
- LEDs are passive. They don't limit current or burn out. Lit means anode
  (pin 2, `A`) is `LOGIC_1` and cathode (pin 1, `K`) is `LOGIC_0`.

## Color rules

Net values (display only, decided in Swift):

| Value      | Color                         |
|------------|-------------------------------|
| `LOGIC_1`  | green                         |
| `LOGIC_0`  | neutral (primary)             |
| `LOGIC_X`  | orange                        |
| `LOGIC_Z`  | dashed gray (floating)        |
| shorted    | red, plus an error banner     |

LEDs use their own part color, not the net color. The color comes from the
catalog JSON `color` field (`ChipCatalog.swift:44-48`).

- Lit: full strength with a glow.
- Dark: the part color at low opacity.
- X or Z on the anode: not lit.

The green used for "valid connect target" in Line mode
(`SchematicCanvasView.swift:392`) does not conflict. Line mode is disabled while
running.

## Phases

### Phase 1: C bridge (`backend/`)

- Add `sim_run_netlist(...)`. Input: a flat netlist with nets, instances (part
  name plus pin-to-net list), and a run time. Output: per-net values, per-LED
  lit flags, the error string, and the shorted net index.
- Implement it on the existing `sim_*` calls. Look up parts with `led_lookup`,
  `supply_lookup`, and the chip lookup.
- Test in the style of `catalog_check.c`: LED lit, LED dark, anode X, and a
  short.

Done when: the test passes on the command line.

### Phase 2: netlist extraction (Swift)

- Union-find over wire endpoints, junctions, and part pins in `SchematicModel`
  to produce nets.
- Map each placed part's pins to `ChipPin` numbers.
- Build the netlist JSON for the bridge from phase 1.

Done when: a unit test shows the expected nets for a LED on VCC/GND and for a
wire with a junction.

### Phase 3: state and toolbar

- Add an `isRunning` flag to `ContentView`. Keep it separate from
  `SchematicTool`, since it is a mode and not a tool.
- Add a Play/Stop button in `.primaryAction` next to the zoom controls, with a
  keyboard shortcut.
- On Play: disable the tool picker and remember the current tool. On Stop:
  restore it and destroy the sim.

Done when: Play and Stop toggle the mode and the previous tool comes back.

### Phase 4: canvas gating

- In `SchematicCanvasView`, return early from `canvasGesture`, the double-tap
  insert, and `dropDestination` while running.
- Disable drag from the drawer in `ChipDrawerView` while running.
- Keep hover and pan/zoom.

Done when: no document change is possible while running, and pan/zoom still
works.

### Phase 5: rendering

- Color wires and pins from net values using the table above.
- Draw LEDs in their part color: lit with a glow, dark at low opacity.
- Show an error banner for a short and highlight the shorted net in red.

Done when: a VCC-LED-GND circuit shows a lit LED of the right color, and a
shorted circuit shows the red net and banner.

### Phase 6: verification

- Run the phase 1 test.
- Build the Xcode project.
- Run a VCC-LED-GND circuit by hand and check the colors.

## Play-mode behavior

- Locked while running: tool picker, pointer edits, line, eraser, flip, VCC/GND
  placement, drop from drawer, double-tap insert.
- Still allowed while running: hover, pan, zoom.
- Stop restores the tool that was selected before Play.

## Open questions

1. Add a clock or switch part to the catalog? Without one, play is a single
   settle, not a running circuit.
2. Should pan and zoom be locked while running?
3. Should a lit LED with no series resistor show a warning? The model does not
   cover current, so the plan leaves it out.

## Out of scope

- Animation over time (needs a clock or input part first).
- Current, forward voltage, and LED burnout.
- Saving simulation state into the document.
