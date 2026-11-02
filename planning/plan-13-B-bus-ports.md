# plan-13-B: Buses — ports, port pins and nets

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-13-A

This sub-plan adds **bus ports**: boxes on a bus line with one pin per
ticked bus wire. Ordinary wires connect to those pins. Every pin for the same
bus wire, on every port of that bus, is one net in the simulation and the
voltage check.

- the **Bus Port** tool: a click on a straight part of a bus adds an empty
  port there and selects it. A click on a bend or end node is refused;
- the **Bus Port** inspector panel: a checkbox per bus wire, Select All /
  Deselect All, and a Side picker (Top/Bottom on a horizontal segment,
  Left/Right on a vertical one);
- ticked wires become pins, in bus order, at the normal 20-unit pin pitch.
  The port grows right (or down). If there is no room it grows left (or up),
  and if there is still no room it refuses with a message;
- the Line tool connects to port pins; the netlist, simulation colours and
  voltage check see bus nets;
- the Eraser removes a port, or a whole bus with its ports, after a confirm
  dialog whenever more than the clicked thing would go;
- unticking a wire that has wires attached asks first, then removes those
  wires;
- the bus's wire count can't go below its highest wire any port uses;
- ports and port wire ends are saved and opened.

Outcome of B: the plan-13 behavior — switch → port 1 `B0` … port 2 `B0` → LED
lights in play mode — works on a bus that is not edited after ports are
placed. Moving a bus that has ports, and sliding ports, is plan-13-C.

References:

- `planning/plan-13-A-bus-lines.md` (§ Prerequisites, § 4–6: `Bus`,
  `BusRoute`, the Bus panel, file version 3).
- `docs/file-format.md` (§ Buses, added in A; § Wires).
- `schematical/schematical/Simulation/Simulation.swift` (`NetlistBuilder`),
  `Simulation/VoltageCheck.swift`.

## Prerequisites

The gate is plan-13-A's table. Re-run it, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-13-A complete | `ls planning/plan-13-A-* 2>/dev/null` → no output; `ls planning/completed/plan-13-A-bus-lines.md` → exists | NOT MET (2026-11-01: not started) |
| Format is version 3 with buses | `grep -n "currentVersion = 3" schematical/schematical/Document/SchematicFile.swift` → one line | NOT MET (lands in A) |
| No port code yet | `grep -rn "BusPort\|busPin" schematical/schematical` → no output | MET (2026-11-01) |

## 1. Goal

- With the Bus Port tool, clicking a bus segment more than one tolerance away
  from every node adds an empty port there and selects it. Clicking near a
  node shows "A port can't go on a bend or the end of a bus.".
- Ticking wires in the port panel adds pins in bus order. The port always
  lies within its segment and never overlaps another port. When it can't fit,
  the tick is refused with "Not enough room on this bus segment for <n> pins.".
- A wire drawn from a chip pin to a port pin, plus a wire from another port's
  pin for the same bus wire to another chip pin, puts both chip pins on one
  net: the simulation drives one from the other, and the voltage check
  refuses a clash across the bus.
- Erasing a port removes it and the wires on its pins. If it has any such
  wires, a confirm dialog comes first. Erasing a bus that has ports asks first,
  then removes the bus, its ports and the wires on their pins.
- Setting the wire count below the highest wire any port uses is refused:
  "<name> is used by a port; this bus needs at least <k> wires.".
- Save and open keep every port (segment, position, side, wires) and every
  wire end on a port pin.

### Non-goals (explicit constraints)

- **No undo** (owner, 2026-11-01).
- **No bus line connectivity.** A wire never attaches to the bus line, only to
  port pins (`lineAnchor` never offers a bus point).
- **Named-net merging across buses is not a feature.** Two buses with a wire
  of the same name are not joined. A net joins two buses only when an ordinary
  wire connects their pins (owner: "a wire connecting A to B makes them the
  same net").
- **The PCB page, backend and part library are unchanged.** The PCB has no
  bus: its nets come from the same netlist.
- **No bus or port colouring in play mode.** The wires on port pins are
  coloured by their net, as every wire is today. The bus line and port bodies
  stay plain.

## 2. Current State

- **Wire ends.** `WireEnd` (`Canvas/SchematicModel.swift:516`) is `.pin(PinRef)`
  or `.junction(UUID)`. `LineAnchor` (`:535`) is `.pin` or `.point`. They are
  switched on in `endTip` (`:1001`), `anchorTip` (`:1011`), `standIn`
  (`:1090`), `endpoint` (`:1106`), and in the file at
  `SchematicFile.swift:119` (`End`), `:389` (`resolve`) and `:543`
  (`End.init(_:chips:)`).
- **Pins in Line mode.** `pinTips` (`:979`) feeds `PinMarkerLayer`;
  `pin(at:)` (`:988`) and `lineAnchor` (`:1023`) find pins.
- **Netlist.** `NetlistBuilder.build(chips:wires:)` (`Simulation.swift:35`)
  unions the two ends of each wire over `WireEnd`, numbers the roots, and
  records each wire's net and each chip pin's net. It is called from
  `SimulationController.start(document:)` (`Simulation.swift:194`) and from
  `VoltageCheck.sortedIssues` (`VoltageCheck.swift:61`). `firstNew` (`:107`)
  is called from `SchematicDocument.toggleSwitch` (`:957`) and `connect`
  (`:1071`).
- **Wire re-fit.** `settleWires(attachedTo chipID:)` (`:1182`) re-fits the
  bends next to a chip's pins after it moves. `attaches(_:to:)` (`:1166`)
  matches pin ends by chip.
- **Erase.** `erase(at:)` (`:1235`) removes a chip and its wires, or a wire,
  then prunes junctions. Nothing in the app asks before deleting. The only
  dialogs are the `NSAlert`s in `SchematicFileController.swift:141,160`.
- **Toolbar message.** `editorMessage` shows in the toolbar
  (`ContentView.swift:294`) and clears after a few seconds (`:305`).

### Measured populations

| What | Count | Command |
|---|---|---|
| `switch` sites over `WireEnd` / `LineAnchor` | 6, plus `End.encode` | `grep -rn "switch end\|switch anchor" schematical/schematical` → 6 (model `:1002,:1012,:1091,:1107`; file `:390,:543`); `End.encode`'s `switch self` (`SchematicFile.swift:144`) also gains a case |
| `NetlistBuilder.build` callers | 2 | `grep -rn "NetlistBuilder.build" schematical/schematical` → `Simulation.swift:195`, `VoltageCheck.swift:68` |
| `VoltageCheck.firstNew` callers | 2 | `grep -rn "VoltageCheck.firstNew" schematical/schematical` → `SchematicModel.swift:957,1071` |
| SwiftUI alerts / confirmation dialogs today | 0 | `grep -rn "\.alert(\|confirmationDialog" schematical/schematical` → no output |

### Verified properties

- **Wires never attach to a bus line.** `lineAnchor` returns a pin, or a grid
  point on a *wire* (`segmentHit`, which walks `wires` only). Buses live in
  `buses`, so this holds without a check, as long as `lineAnchor` is not
  taught about bus lines.
- **Wire colours follow the netlist with no change.** `wireStyle`
  (`Simulation.swift:397`) looks up `netlist.netOfWire[wireID]`. Once port
  pins join nets in the builder, wires on them colour correctly.
- **The voltage check sees every net through `NetlistBuilder`.** Read
  `sortedIssues`: it builds its connections only from
  `NetlistBuilder.build`. Passing buses to the builder covers the check.

## 3. Design Overview

1. **Model** (`BusModel.swift`): `BusPort`, its geometry, and the fit rule.
2. **Wire ends**: `WireEnd.busPin(BusPinRef)` and `LineAnchor.busPin`, with
   every switch site updated, plus pin tips and hits for the Line tool.
3. **Nets**: `NetlistBuilder.build(chips:wires:buses:)`, and the voltage check
   and simulation pass buses.
4. **Canvas and inspector**: the Bus Port tool, drawing ports, Pointer
   selection (no drag yet), the Port panel, and the confirm dialog.
5. **File**: `ports` in each bus and the `{port, wire}` end.

Correctness risk concentrates in **the netlist union** (a wrong union
silently merges or splits nets in the simulation) and in **the fit rule**
(plan-13-C reuses it for every drag). Both are stated exactly below. The
end-to-end runtime proof (§ Validation) exercises the netlist through a real
simulation.

**Correctness gate:** build, screenshots, `jq` checks, and the play-mode
proof. Not byte-identity.

**Rejected alternatives:**

- *Store a port by absolute point.* A port stores its segment index and its
  position along that segment. A segment drag then moves the port with no
  bookkeeping, because its across coordinate comes from the segment. Only
  inserting a node renumbers segments (plan-13-C).
- *Union every port pin of a bus wire, wired or not.* That adds a net per
  unwired pin, so the circuit gets empty nets. The builder unions only pins
  that some wire ends on.
- *Put the bus ID in `BusPinRef`.* The port knows its bus. Keeping only
  `(port, wire)` means a pin ref can't name a port and a bus that disagree.
- *A modal `NSAlert` for the confirm.* The file controller's alerts are for
  window-level prompts. A SwiftUI `.alert` driven by document state fits the
  canvas flow, and the eraser drag can check that state.

## 4. Port model and geometry

```swift
struct BusPort: Identifiable, Hashable {
    let id: UUID
    /// Index of the bus segment it sits on (points[segment] → points[segment+1]).
    var segment: Int
    /// Along-axis coordinate of its first pin slot: x on a horizontal
    /// segment, y on a vertical one. On the grid.
    var start: CGFloat
    /// Which side its pins come out: .top/.bottom on a horizontal segment,
    /// .left/.right on a vertical one (ChipLayout.Side).
    var side: ChipLayout.Side
    /// The ticked bus wire IDs. Pin order is always the bus's wire order.
    var wireIDs: Set<UUID>
}
```

`Bus` gains `var ports: [BusPort] = []`.

**Geometry** (`BusPort` helpers taking the bus). Let the segment have axis
coordinate range `[lo, hi]` and across coordinate `c` (y for horizontal, x
for vertical). Let `n` be the number of ticked wires, `slots = max(n, 1)` and
`pitch = ChipLayout.pitch` (20).

- Pin i (0-based, in bus order among the ticked wires) is at along-axis
  `start + i·pitch`.
- **Extent** along the axis: `[start − pitch/2, start − pitch/2 + slots·pitch]`.
  An empty port is one slot long.
- **Body**: the extent × 40 deep on the pin side of the bus line. On a
  horizontal bus with `.bottom` that is y from `c` to `c + 40`; with `.top`,
  `c − 40` to `c`. The vertical case is the same with x. The bus line runs
  along the body's inner edge.
- **Pin stub**: 10 (`ChipLayout.stubLength`) out from the body's outer edge.
  The **pin tip** is at across `c ± 50` and along `start + i·pitch`. Both are
  multiples of 10, so every tip is on the grid.
- **Label**: the wire's display name, inside the body next to its pin, at the
  chip pin-name font. It is rotated 90° on a horizontal bus, where pins are
  only 20 apart along x, and level on a vertical one.
- **Defaults**: `.bottom` on a horizontal segment, `.right` on a vertical one.

**Fit rule** (`Bus.fit(port:slots:preferredStart:) -> CGFloat?`). Free
space is `[lo, hi]` of the port's segment, minus the extents of the other
ports on the same segment (both sides). Extents may touch but not overlap.
With `length = slots·pitch`:

1. If the extent at `preferredStart` lies in the free interval that contains
   `preferredStart`, keep it.
2. Otherwise, if `preferredStart − pitch/2 + length` overruns that
   interval's right (lower) end, move the start left (up) just enough to end
   at that end. If the result still lies in the interval, use it.
3. Otherwise return nil.

The owner's rule: "grow right if can, if can't grow left, if can't error.
Grow down if can, if can't grow up." Note that a bus segment that is too short
is the only real failure.

## 5. Document operations

- `var selectedPortID: UUID?`, which clears and is cleared by the other three
  selections.
- `addPort(at:tolerance:) -> String?`: hits a bus segment (`busHit`). Refused
  within `tolerance` of any route node: "A port can't go on a bend or the end
  of a bus.". Snaps the along-axis coordinate to the grid as the preferred
  start, then runs `fit(slots: 1)`; nil → "Not enough room on this bus
  segment for a port.". It appends an empty port with the default side and
  selects it.
- `setPortWires(_ portID:, _ ids: Set<UUID>) -> String?`: the result of
  ticks, Select All, and Deselect All. Growing runs `fit` with the new slot
  count and the current start. On nil it changes nothing and returns "Not
  enough room on this bus segment for <n> pins.". It stores the fitted start
  (it may have moved left/up). Then `settleWires` re-fits wires on this port,
  because pins after an inserted one shift by one pitch.
- `wiresOnPins(port:, removing ids:) -> Int`: how many wires end on the pins
  that unticking `ids` would remove. The panel uses it to decide whether to
  ask first.
- Unticking removes the wires on those pins, then prunes junctions.
- `setPortSide(_:_:)`: stores the side if it matches the segment's
  orientation, then settles the port's wires.
- `setBusWireCount` minimum: `max(2, k)`, where k is 1 + the highest list
  index of any wire ticked on any port of the bus. A smaller request returns
  "<name of wire k−1> is used by a port; this bus needs at least <k>
  wires." and changes nothing.
- **Confirm.** `var pendingRemoval: PendingRemoval?` with `message: String`
  and a private action (`.port(UUID)`, `.bus(UUID)`, `.untick(port: UUID,
  wires: Set<UUID>)`), plus `confirmPendingRemoval()` and
  `cancelPendingRemoval()`. `erase(at:)` order becomes **chip → port → wire →
  bus**:
  - a port with wires on its pins → `pendingRemoval` "Delete this port and
    the <m> wire(s) connected to it?"; one with none → removed now;
  - a bus with ports → "Delete this bus, its <p> port(s) and the <m> wire(s)
    connected to them?"; one with none → removed now;
  - while `pendingRemoval` is set, `erase` does nothing, so one eraser sweep
    raises at most one dialog.
  The panel's untick asks through the same state when `wiresOnPins > 0`:
  "Remove <k> pin(s) and the <m> wire(s) connected to them?".
- `ContentView` shows `.alert(…, isPresented:)` bound to `pendingRemoval`,
  with a destructive "Delete" (→ `confirmPendingRemoval`) and "Cancel".

## 6. Wire ends, Line tool, nets

- `struct BusPinRef: Hashable { let portID: UUID; let wireID: UUID }`;
  `WireEnd.busPin(BusPinRef)`; `LineAnchor.busPin(BusPinRef)`.
- `busPinTip(_:) -> CGPoint?` (nil when the port is gone or the wire is not
  ticked). `endTip`, `anchorTip`, `standIn` and `endpoint` gain the case, as
  pass-throughs like `.pin`.
- `pinTips` adds every port pin tip, so Line mode marks them. `lineAnchor`
  tries chip pins, then port pins (`busPin(at:radius:)`), then wire points.
- `attaches` gains a port form. `settleWires(attachedToPort:)` re-fits wires
  with an end on that port's pins, the same body as the chip version.
- **`NetlistBuilder.build(chips:wires:buses:)`**: after the wire unions, for
  each bus, collect the `.busPin` ends that some wire uses, group them by
  `wireID`, and union each group's members with its first. Nothing else
  changes. Chip-pin connections are collected exactly as today. Port pins are
  not circuit pins, so they add no connection rows.
- `VoltageCheck.issues` / `sortedIssues` / `firstNew` take `buses:`.
  `toggleSwitch` and `connect` pass `buses`, and `SimulationController.start`
  passes `document.buses`.

## 7. Canvas, tool, panel

- `SchematicTool.busPort`: title "Bus Port", symbol
  `rectangle.connected.to.line.below`, after `.bus`.
- A click (press) with the tool calls `addPort`. A refusal sets
  `editorMessage`.
- **`PortLayer`**, drawn above `BusLayer` and under `WireLayer`: each port's
  body (control background fill, 1.5 outline; accent and 2 when selected or
  hovered), pin stubs, and labels. In Line mode the port pin tips get the same
  markers as chip pins (via `pinTips`).
- **Pointer**: chip → **port** → wire bend → wire segment → bus node → bus
  segment → pan. A port hit selects it. plan-13-C adds dragging.
- **Hover**: ports are checked after chips.
- **`BusPortPanel`** (`Drawer/BusPanels.swift`):
  - header "Bus Port";
  - Section "Pins": a `Picker("Side")` (Top/Bottom or Left/Right by segment
    orientation); `Button("Select All")` and `Button("Deselect All")`;
  - a `LazyVStack` of `Toggle(name, isOn:)`, one per bus wire in bus order.
    Turning a toggle on calls `setPortWires`. Turning one off checks
    `wiresOnPins`: 0 → `setPortWires` now; > 0 → `pendingRemoval`;
  - a refusal message in red caption text under the buttons;
  - `.id(port.id)`.
- `SchematicSidebarView` order: chip → port → bus → library.

## Compatibility / Format Impact

Still version 3 (A bumped it; no build has shipped version 3 without ports).

- Each bus gains `"ports"` (written always, empty array allowed; missing
  means none):

  ```json
  "ports": [ { "id": "…", "segment": 0, "start": 140, "side": "bottom",
               "wires": [ "<bus wire id>", "…" ] } ]
  ```

  `wires` is written in bus order.
- A new wire end: `{ "port": <port id>, "wire": <bus wire id> }`.
  `SchematicFile.End` decodes `port` first, then `junction`, then `part`.
- New validation details under `badBus`:
  - `port <id> segment <i> is not on bus <id>`;
  - `port <id> side "<s>" must be top or bottom on a horizontal segment`
    (left or right on a vertical one);
  - `port <id> wire <id> is not on bus <id>`;
  - `port <id> does not fit on its segment` (when `fit` at the stored start
    does not return that start: off the segment, or overlapping another port);
  - and for wire ends (new `SchematicFileError.missingPort(wire:port:)`,
    "Wire <id> connects to port <id>, which is not in the file."), plus
    `missingPortPin(wire:port:wire:)`, "Wire <id> connects to a pin port
    <id> does not have.".
- Port IDs are claimed in the shared ID set.
- `docs/file-format.md`: § Buses gains Ports; § Wires lists the third end;
  § Validation on open lists the new messages.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` for partial. Moot: `- [x] ~~text~~ — moot:
> <evidence>`. Fill `Commit:` when a phase lands. **An unticked box means NOT
> DONE.**

### Phase 1 — Port model, tool and drawing

- [ ] `BusModel.swift`: `BusPort`, `Bus.ports`, geometry helpers, `Bus.fit`
      (§ 4).
- [ ] `SchematicModel.swift`: `SchematicTool.busPort`, `selectedPortID`,
      `addPort`, `setPortWires` (grow and shrink without wire removal, since no
      wire can attach yet), `setPortSide`, and the `setBusWireCount` minimum.
- [ ] `SchematicCanvasView.swift`: `PortLayer`, the tool branch, Pointer
      selection and hover for ports.
- [ ] `BusPanels.swift`: `BusPortPanel`; `SchematicSidebarView`: show it.

Acceptance: a port placed mid-segment is empty and selected; ticking 4 wires
grows it right; ticking near the segment's right end grows it left; ticking
more than fit shows the room message and nothing changes; clicking a bend is
refused; switching Side moves the pins across; the wire count can't drop
below the highest ticked wire.
  Check: build; on a horizontal bus from (100,100) to (300,100), place a port at
  x 260, tick `B0`–`B3`, screenshot (it should have shifted left to end at
  300); tick all 8 → message, screenshot; set Side to Top, screenshot; set the
  wire count to 3 with `B3` ticked → message, screenshot (est. 8 min).
Commit: —

### Phase 2 — Port pins as wire ends; nets

- [ ] `SchematicModel.swift`: `BusPinRef`, the new `WireEnd` / `LineAnchor`
      cases in all 4 model switch sites, `busPinTip`, `busPin(at:)`, `pinTips`,
      `lineAnchor`, `settleWires(attachedToPort:)`, called from
      `setPortWires` / `setPortSide`.
- [ ] `Simulation.swift`: `NetlistBuilder.build(chips:wires:buses:)` (§ 6);
      `start(document:)` passes buses.
- [ ] `VoltageCheck.swift`: the `buses:` parameter; `toggleSwitch` and
      `connect` pass `buses`.

Acceptance: the plan-13 behavior. 5V → `TE-GDS0204` position 1 → port 1
`B0`; port 2 `B0` (same bus) → `LED-RED-0603` anode; cathode → GND. Run,
turn position 1 on, and the LED lights. Turn it off and the LED goes dark.
  Check: build; draw it, run (⌘R), screenshot with the switch off and on
  (est. 8 min; the smallest check that the union reaches the backend).
Commit: —

### Phase 3 — Erasing, unticking, confirm dialog

- [ ] `SchematicModel.swift`: `PendingRemoval`, `confirmPendingRemoval`,
      `cancelPendingRemoval`, `wiresOnPins`, the port and bus branches of
      `erase` (order chip → port → wire → bus), and untick removal of wires
      and junction pruning.
- [ ] `ContentView.swift`: the `.alert` for `pendingRemoval`.
- [ ] `BusPanels.swift`: unticking with wires goes through `pendingRemoval`.

Acceptance: erasing an unwired port removes it with no dialog; erasing a
wired port asks, and Cancel keeps everything, Delete removes the port and its
wires; erasing a bus with 2 ports asks, then removes the bus, both ports and
their wires; unticking a wired pin asks, then removes the pin and its wires.
  Check: build; on the Phase 2 drawing (re-drawn), do each step and screenshot
  the dialog and the result (est. 8 min).
Commit: —

### Phase 4 — Save and open ports

- [ ] `SchematicFile.swift`: `Bus.ports` (`PortEntry`), `End.port`
      (decode/encode), `End.init(_:chips:)` for `.busPin`, `resolve` for
      `.busPin`, the port validation (§ Compatibility), and the two new errors.
- [ ] `docs/file-format.md`: Ports, the third wire end, and the new messages.

Acceptance: the Phase 2 drawing saved and reopened still lights the LED, and
the file holds both ports and two port wire ends.
  Check: build; save the Phase 2 drawing as `/tmp/p13b.schematical`;
  `jq '[.buses[0].ports[] | .wires | length], [.wires[] | .from, .to | select(.port)] | length' /tmp/p13b.schematical`
  → `[1,1]` then `2`; reopen, run, turn the switch on, screenshot
  (est. 6 min).
Commit: —

## Validation Plan

- Tests: no app test target. Each phase has screenshot checks and Phase 4 a
  `jq` check.
- Coverage check: Phase 2's play-mode proof exercises the new
  `NetlistBuilder` branch (the LED can only light through it). Phase 3
  exercises every `erase` branch and both dialog buttons. Phase 4 exercises
  `End.port` both ways.
- Runtime proof: Phase 2 and Phase 4 (switch → bus → LED).
- Doc sync: `docs/file-format.md` (Phase 4); the canvas header comment gains
  the Bus Port tool (Phase 1).
- Final gate for B: build → `** BUILD SUCCEEDED **`; `git status --short
  backend` → no output (est. 4 min). The plan-13 landing tier runs at the end
  of C.

## Decisions

- A port is placed by clicking the bus with the Bus Port tool; bends and
  ends are invalid spots — owner, 2026-11-01.
- A new port starts empty and selected, so its panel opens — owner,
  2026-11-01.
- The panel lists every bus wire with a checkbox, plus Select All /
  Deselect All; ticked wires are pins — owner, 2026-11-01.
- Pin order is the bus list order; spacing is the normal schematic pin pitch
  (20) — owner, 2026-11-01.
- The pin side is picked from a dropdown: top/bottom on a horizontal bus,
  left/right on a vertical one — owner, 2026-11-01.
- A port grows right (down); else left (up); else it refuses — owner,
  2026-11-01.
- All pins for the same wire on the same bus are one net; there is no other
  bus naming — owner, 2026-11-01.
- Deleting a port deletes the wires on its pins; deleting a bus deletes its
  ports and their wires; both confirm first — owner, 2026-11-01.
- The wire count can't go below the highest wire any port uses — owner,
  2026-11-01.
- Port body 40 deep on the pin side, labels inside; confirm only when
  something beyond the clicked item would be removed; no bus colouring in
  play mode — author's call, 2026-11-01 (§ 4, § 5).

## Corrections

## Summary

B gives the bus its meaning: ports, pins and nets. The risk is the netlist
union (checked end-to-end through the simulator) and the fit rule, which C
reuses. The bus line itself still never connects to anything, and the PCB and
backend stay untouched.
