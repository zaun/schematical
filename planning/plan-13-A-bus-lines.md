# plan-13-A: Buses — the bus line

Last updated: 2026-11-01
Overall Effort: x-large (1d–3d) — plan-13 is A → B → C
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-13 adds **buses** to the schematic editor. A bus is a thick line that
carries 2 to 999 named wires. **Bus ports** sit on it, and each port has one
pin per wire the user ticks. Ordinary wires connect to port pins. Every port
pin for the same bus wire, on any port of that bus, is one net. The PCB never
sees a bus, only the nets it makes.

The single behavioral outcome of plan-13: on a schematic with a 5V supply →
DIP switch → port 1 pin `B0` of a bus, and port 2 pin `B0` of the same bus →
LED → GND, turning the switch on in play mode lights the LED. The bus,
its ports and their pins survive save and open.

This sub-plan (A) delivers the bus line by itself, without ports:

- the **Bus** tool: drag from one grid point to another to draw a bus. The
  route has right-angle bends only. The line is 6 wide (the wire's 1.5 × 4);
- buses are drawn, hovered and selected (Pointer), and erased (Eraser);
- bends and segments can be dragged (right angles kept), and a double-click
  adds a bend;
- the **Bus** inspector panel: the wire count (2–999) and each wire's
  display name (1–4 characters, default `B0`, `B1`, …, unique in its bus);
- buses are saved and opened (file format version 3).

Outcome of A: a bus can be drawn, edited, renamed, resized, erased, saved
and reopened. Nothing connects to it yet.

References:

- `docs/file-format.md` (the format this plan changes; § Changing the format).
- `schematical/schematical/Canvas/SchematicModel.swift` (`SchematicTool`,
  `OrthogonalRoute`, `SchematicDocument`).
- `schematical/schematical/Canvas/SchematicCanvasView.swift` (tool gestures,
  `WireLayer`).
- `schematical/schematical/Document/SchematicFile.swift` (save/open).
- `schematical/schematical/Drawer/SchematicSidebarView.swift` (the inspector).
- `planning/plan-13-B-bus-ports.md`, `planning/plan-13-C-bus-editing.md`.

## Prerequisites

This table is the gate for all of plan-13. B and C point here and add their
own rows.

| Must be true | Command | Status |
|---|---|---|
| plan-11 complete | `ls planning/plan-11-* 2>/dev/null` → no output | NOT MET (2026-11-01: A–D open) |
| plan-12 complete | `ls planning/plan-12-[A-K]-*.md 2>/dev/null` → no output | NOT MET (2026-11-01: A–K open) |
| The canvas change in progress is committed | `git status --short schematical/schematical/Canvas/SchematicCanvasView.swift` → no output | NOT MET (2026-11-01: ` M`, the `worldCanvas` / `shiftToWorld` change, not this plan's) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p13-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-01 with the uncommitted canvas change) |
| No bus code yet | `grep -rn "case bus\|struct Bus\b\|\"buses\"" schematical/schematical` → no output | MET (2026-11-01) |
| File format is version 2 | `grep -n "currentVersion = " schematical/schematical/Document/SchematicFile.swift` → `currentVersion = 2` | MET (2026-11-01) |

plan-13 edits the same canvas, model and file code that plan-11-D edits
(`SchematicCanvasView.swift`, `SchematicFile.swift`), and the same
`SchematicTool` enum and toolbar that plan-12-B edits (it adds Pull-up and
Pull-down after GND). plan-12-K also edits `SchematicCanvasView.swift`. That
is why plan-11 and plan-12 must finish first. The uncommitted canvas change is the owner's work in
progress; plan-13 must build on it, never revert it.

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites.**

## 1. Goal

- With the Bus tool, a drag from grid point P to grid point Q (P ≠ Q) adds a
  bus whose route is `BusRoute.route(from: P, to: Q)`, drawn 6 wide, with 8
  wires named `B0`…`B7`.
- Clicking a bus with the Pointer selects it, and the inspector shows the
  **Bus** panel. The wire count can be set from 2 to 999. Each wire's name can
  be changed to 1–4 non-blank characters, unique in the bus (case-insensitive).
- Bends and segments drag with every segment staying horizontal or vertical.
- The Eraser removes a bus.
- Save writes `"version": 3` and a `"buses"` array. Opening the file restores
  every bus exactly (route, wire IDs, names). Version 1 and 2 files still open.

### Non-goals (explicit constraints)

- **No undo.** The app has none today, and the owner decided not to add it in
  plan-13 (2026-11-01).
- **No change to wires, junctions or the Line tool.** `Wire`, `WireEnd`,
  `Junction`, `OrthogonalRoute` and wire editing behave exactly as today. Bus
  editing uses its own helper (`BusRoute`); the owner asked that it not be a
  copy of the wire logic, because buses keep right angles when dragged.
- **No change to the PCB page, the backend, or the part library.** A bus is
  not a part.
- **No connectivity in A.** Nothing attaches to a bus line, now or later:
  wires attach only to port pins (plan-13-B). A wire crossing a bus does not
  connect to it.
- **No branching.** A bus is one open polyline with two free ends.

## 2. Current State

- **Tools.** `SchematicTool` (`Canvas/SchematicModel.swift:22`) is a
  `CaseIterable` enum (pointer, eraser, line, flip, vcc, ground; plan-12-B
  adds pullUp and pullDown). Line numbers in this plan are from 2026-11-01,
  before plan-11 and plan-12; re-find each symbol by name. The toolbar
  picker lists `allCases` (`ContentView.swift`, `Picker("Tool", selection:
  $tool)`), so a new case shows up there automatically.
- **Wires.** `Wire` (`SchematicModel.swift:542`) has two `WireEnd`s (a pin or
  a junction) and interior bends. Wires can only start and end on a pin or on
  an existing wire (`lineAnchor`, `:1023`). `OrthogonalRoute.route`
  (`:563`) makes the default H–V–H route. `moveNode` (`:640`) lets a bend go
  diagonal (wires re-fit around pins later). `moveSegment` (`:649`) moves a
  segment by any offset. Buses need different rules: they have no pin ends,
  and every segment must stay horizontal or vertical while it is dragged.
- **Canvas gestures.** `SchematicCanvasView.handleChanged` (`:256`) switches
  on the tool. Pointer hit order in `beginPointerInteraction` (`:377`) is
  chip → wire bend → wire segment → pan. The Eraser calls
  `document.erase(at:)` on every drag change (`:325`). Double-click adds a
  wire bend (`:109`). `WireLayer` (`:531`) draws wires 1.5 wide and the hovered
  wire with a 6-wide underlay.
- **Selection.** `SchematicDocument.selectedChipID` and `selectedTextID`
  (`:693`, `:704`) clear each other in `didSet`. The inspector
  (`Drawer/SchematicSidebarView.swift`) shows the chip panel when a chip is
  selected, and otherwise the parts library.
- **Inline text validation precedent.** `ReferenceSection`
  (`Canvas/PartInfoViews.swift`) commits a `TextField` on submit or when
  focus leaves it. The model returns an error string, which the panel shows
  as red caption text, and then the field is reset to the stored value. The bus
  wire-name fields copy this pattern.
- **File.** `SchematicFile` (`Document/SchematicFile.swift`) is version 2
  (`:14`). `decode` (`:271`) rejects newer versions before full decoding.
  `apply` (`:320`) claims every ID in one set (duplicate IDs fail) and builds
  the document through `replaceContents` (`SchematicModel.swift:728`).
  `JSONDecoder` ignores unknown keys, so a version 2 build would silently drop
  a `"buses"` key. That is why buses bump the version (§ Compatibility).
- **Unsaved changes.** `SchematicFileController.hasUnsavedChanges` compares
  `SchematicFile(document:)` against the saved snapshot. Anything the snapshot
  includes is tracked automatically.
- **No app test target.** App checks are the build plus runtime screenshots,
  as in plan-09-E and plan-11-D. The saved file's JSON can also be checked
  with `jq`.

### Measured populations

| What | Count | Command |
|---|---|---|
| `SchematicTool` cases | 6 today, 8 once plan-12-B lands (`.pullUp`, `.pullDown`) | `sed -n 22,29p schematical/schematical/Canvas/SchematicModel.swift \| grep -c "case "` → 6 (2026-11-01); re-measure at the gate |
| `selectedChipID` users (files) | 6 | `grep -rln selectedChipID schematical/schematical \| wc -l` → 6 |
| Existing schematic files at the repo root (untracked, owner's) | 2 | `ls *.schematical` → `be-6502.schematical test.schematical` |

### Verified properties

- **A new tool case needs no toolbar code.** Read the `Picker("Tool")` in
  `ContentView.swift`: it is `ForEach(SchematicTool.allCases)` with `title`
  and `symbol`.
- **Play mode blocks every edit gesture.** Read `handleChanged`: when
  `isRunning`, it pans and returns before the tool switch. A new tool case
  inside that switch is blocked too.
- **The snapshot drives "unsaved changes".** Read
  `SchematicFileController.hasUnsavedChanges` (`:72`): it compares
  `SchematicFile` values (`Equatable`), so adding `buses` to `SchematicFile`
  tracks bus edits with no other code.
- **Unknown JSON keys are ignored.** `SchematicFile` uses synthesized
  `Codable`, and Foundation's `JSONDecoder` skips keys a type does not
  declare. A version 2 build reading a file with `"buses"` would drop the buses.

## 3. Design Overview

Three layers, built in this order:

1. **Model** (`Canvas/BusModel.swift`, new): `Bus`, `BusWire`, `BusRoute`,
   plus `SchematicDocument` methods for buses. This is pure value logic.
2. **Canvas and inspector**: the Bus tool, a `BusLayer` drawn under the
   wires, Pointer hit-testing for buses, the Eraser, and a `BusPanel` in
   `SchematicSidebarView`.
3. **File**: version 3, with the `buses` array.

Risk sits in **`BusRoute` dragging** (keeping right angles, never creating a
zero-length segment). plan-13-C builds port fitting on top of it, so its rules
are stated exactly here (§ 5) and C only adds checks to them.

**Correctness gate:** the app build, runtime screenshots, and `jq` checks of a
saved file. Byte-identity does not apply: the file format changes on
purpose.

**Rejected alternatives:**

- *Reuse `Wire` for the bus line.* Wire ends must be pins or junctions, wires
  feed the netlist and voltage check, and wire bends may go diagonal while
  dragged. Each of those is wrong for a bus. A separate type keeps the wire
  code unchanged.
- *Reuse `OrthogonalRoute.moveNode` / `moveSegment`.* They allow diagonal
  segments (`moveNode`) and along-axis segment moves. The owner asked that
  bus dragging keep right angles and not be a straight copy.
- *Keep version 2 and add `buses` as an optional key.* An older build would
  open such a file and silently lose every bus on save. Bumping to 3 makes
  older builds refuse it with "saved by a newer version".
- *A single `Selection` enum replacing `selectedChipID` / `selectedTextID`.*
  That touches all 6 files that use `selectedChipID`, PCB code included.
  Two more optional IDs with the same clear-each-other `didSet` touch only
  the model, canvas and sidebar.

## 4. Model

New file `schematical/schematical/Canvas/BusModel.swift` (add it to the Xcode
target):

```swift
struct BusWire: Identifiable, Hashable {
    let id: UUID          // internal; ports and wire ends use this
    var name: String      // display name, 1–4 characters
}

struct Bus: Identifiable, Hashable {
    let id: UUID
    /// The route: ≥ 2 grid points. Consecutive points differ in exactly one
    /// of x / y (every segment is horizontal or vertical, length > 0).
    var points: [CGPoint]
    /// The bus's wires in list order. 2…999 of them.
    var wires: [BusWire]
    // plan-13-B adds `ports: [BusPort]`.
}
```

Constants on `Bus`: `wireCountRange = 2...999`, `defaultWireCount = 8`,
`nameLengthRange = 1...4`, `lineWidth: CGFloat = 6` (wire 1.5 × 4).

**Default names.** `Bus.defaultName(for index: Int, taken: Set<String>)`
returns `"B\(index)"`. If another wire already has that name
(case-insensitive), it returns `"B\(n)"` for the smallest n ≥ 0 that is free.
Index 998 gives `B998` (4 characters), so defaults always fit.

**`SchematicDocument` additions** (`SchematicModel.swift`):

- `private(set) var buses: [Bus] = []`, included in `replaceContents` (new
  `buses:` parameter) and cleared/replaced there.
- `var selectedBusID: UUID?`. Its `didSet` clears `selectedChipID` and
  `selectedTextID`, and theirs clear it (extend the two existing `didSet`s).
- `@discardableResult func addBus(from: CGPoint, to: CGPoint) -> UUID?`:
  snaps both points. It refuses (returns nil, sets `editorMessage`) when they
  are equal, or when either point is on an existing bus line
  (`busHit(at:tolerance: 0.5)`): "A bus can't start or end on another bus."
  Otherwise it appends a bus with `BusRoute.route(from:to:)` and 8 default
  wires, and selects it.
- `func setBusWireCount(_ id: UUID, _ count: Int) -> String?`: clamps to
  `2...999`. Growing appends wires with fresh UUIDs and default names.
  Shrinking drops wires from the end. plan-13-B adds the "a port uses it"
  minimum.
- `func renameBusWire(bus: UUID, wire: UUID, to name: String) -> String?`:
  trims whitespace and refuses with a message, changing nothing:
  - empty → "A wire name can't be empty.";
  - more than 4 characters → "A wire name is at most 4 characters.";
  - contains whitespace → "A wire name can't contain spaces.";
  - another wire in the same bus has it (case-insensitive) → "\(name) is
    already used on this bus.".
- `func setBusPoints(_ id: UUID, _ points: [CGPoint])`: stores a route that
  `BusRoute.isValid` accepts. Others are ignored.
- `func busHit(at:tolerance:) -> (bus: UUID, segment: Int)?` and
  `func busNodeHit(at:tolerance:) -> (bus: UUID, index: Int)?`. They mirror
  `segmentHit` / `nodeHit`, but take every point of the route, ends
  included, as a node.
- `func insertBusNode(at:tolerance:)`: on the segment hit, adds a node at the
  snapped projection when it is strictly inside the segment.
- `erase(at:)` gains a last branch: a bus hit removes the bus and clears
  `selectedBusID` if it was selected. (Order: chip → wire → bus. plan-13-B
  puts ports before wires.)

## 5. `BusRoute` (right-angle routing for buses)

`enum BusRoute` in `BusModel.swift`. Every function returns a new route or
nil. Nil means "refuse this step, keep the last good route".

- `route(from a: CGPoint, to b: CGPoint) -> [CGPoint]`: `[a, b]` when they
  share x or y. Otherwise H–V–H through `midX = SchematicGrid.snap((a.x+b.x)/2)`,
  the same shape as the wire default. If `midX` equals `a.x` or `b.x`, it
  uses the single L `[a, (b.x, a.y), b]`, so no segment has length 0.
- `isValid(_ points:) -> Bool`: at least 2 points, every point on the grid,
  each consecutive pair differs in exactly one coordinate, and no two
  consecutive segments run back over each other (a 180° fold). Collinear
  same-direction segments are allowed: that is a node added by double-click.
- `moveSegment(_ points:, at i:, by offset:) -> [CGPoint]?`: keeps only the
  component of `offset` perpendicular to segment i (y for a horizontal
  segment, x for a vertical one), snapped to the grid, and moves both of its
  points. It returns nil when the result fails `isValid` (a neighbour segment
  shrank to 0 or folded back).
- `moveNode(_ points:, at k:, to p:) -> [CGPoint]?`: moves point k to p, then
  keeps each neighbouring segment's direction by moving that segment's far
  point across: for segment (k−1, k), if horizontal set `points[k−1].y = p.y`,
  if vertical set `points[k−1].x = p.x`; the same for (k, k+1) with
  `points[k+1]`. An end node (k = 0 or last) has one neighbour, so its
  segment moves across and changes length. It returns nil when the result
  fails `isValid`. Unlike `OrthogonalRoute.moveNode`, no segment ever goes
  diagonal.
- `insertNode(_ points:, segment i:, at p:) -> [CGPoint]?`: projects p onto
  segment i, snaps it, and returns nil unless the point is strictly between
  the segment's ends.

Canvas drags call these against the route as it was at drag start (`base`),
exactly as wire drags do. A nil result leaves the bus as it was after the
last good frame.

## 6. Canvas, tool and inspector

- `SchematicTool.bus`: title "Bus", symbol `line.3.horizontal`. Placed after
  `.line` in the enum, so the picker order is Pointer, Eraser, Line, Bus,
  Flip, VCC, GND, Pull-up, Pull-down (the last two from plan-12-B).
- **Drawing.** `Interaction.drawingBus(from: CGPoint, cursor: CGPoint)`. On
  press, the snapped press point is the start. The preview is
  `BusRoute.route` to the snapped cursor, drawn 6 wide, dashed, in the accent
  colour. On release, `document.addBus(from:to:)`. A refusal shows
  `editorMessage` through the existing toolbar message
  (`ContentView.swift:294`).
- **`BusLayer`** (private view in `SchematicCanvasView.swift`): a `Canvas`
  drawn **under** `WireLayer` (so wires stay visible on top), using the
  `worldCanvas` / `shiftToWorld` helpers. Each bus is a polyline,
  `lineWidth: Bus.lineWidth`, `.butt` caps, `.miter` joins, in `.primary`.
  The hovered bus gets a 12-wide accent underlay at 0.35 opacity; the
  selected bus is stroked in `.accentColor`.
- **Pointer.** `beginPointerInteraction` order becomes chip → wire bend →
  wire segment → bus node → bus segment → pan. A bus hit selects it
  (`selectedBusID`) and starts `.dragBusNode(bus:base:index:)` or
  `.dragBusSegment(bus:base:index:)`, which call `BusRoute.moveNode` /
  `moveSegment` with the snapped cursor / delta and `setBusPoints` when
  non-nil. Pressing empty space clears `selectedBusID` as it clears
  `selectedChipID`. Bus hit tolerance is `lineTolerance + 3 / zoom` (half the
  bus width added).
- **Hover.** `updateHover` checks buses after chips and wires
  (`hoveredBusID`).
- **Double-click.** After the memory/CPU checks, try a wire node insert. If
  no wire was hit, call `document.insertBusNode`.
- **Eraser.** Handled by `document.erase(at:)` (§ 4).
- **Inspector.** `SchematicSidebarView` shows `BusPanel` when
  `selectedBusID` names a bus (chip panel first, then bus, then library).
  `BusPanel` (new, in `Drawer/BusPanels.swift`) is a grouped `Form`:
  - Section "Bus": a `LabeledContent("Wires")` with a `TextField` (number)
    plus `Stepper(in: 2...999)`. Submitting or leaving the field calls
    `setBusWireCount`, shows any message in red caption text, and resets the
    field to the stored count.
  - Section "Wire Names": a `LazyVStack` of rows (index label `0…n−1`, then
    a `TextField` for the name, committed on submit / focus loss through
    `renameBusWire`). Each row shows its own error under it, the
    `ReferenceSection` pattern.
  - `.id(bus.id)`, so each bus gets fresh field state.
- The file's header comment (`SchematicCanvasView.swift:3`) gains a Bus line
  in its tool list.

## Compatibility / Format Impact

- `SchematicFile.currentVersion` becomes **3**. Saving always writes 3.
  Versions 1 and 2 open as before, with no buses. A version 2 build refuses a
  version 3 file with "This schematic was saved by a newer version of
  Schematical (file format 3).", the existing `newerVersion` path.
- New top-level key, written by every version 3 save (empty array when there
  are no buses):

  ```json
  "buses": [
    { "id": "…",
      "points": [ { "x": 100, "y": 200 }, { "x": 300, "y": 200 } ],
      "wires": [ { "id": "…", "name": "B0" }, { "id": "…", "name": "B1" } ] }
  ]
  ```

  `SchematicFile.buses: [Bus]?` (nil in older files, which means none).
  plan-13-B adds `"ports"` inside each bus, and a port wire end.
- Opening validates, and fails with `SchematicFileError.badBus(String)`
  ("The bus is not valid: <detail>."). Details:
  - `bus <id> route is not made of horizontal and vertical segments on the grid`
    (when `BusRoute.isValid` fails);
  - `bus <id> has <n> wires; it must have 2 to 999`;
  - `bus <id> wire name "<name>" must be 1 to 4 characters with no spaces`;
  - `bus <id> uses the wire name "<name>" twice`.
- Bus IDs and bus wire IDs are claimed in the same ID set as parts,
  junctions, wires and texts. A duplicate fails with the existing
  `duplicateID`.
- Unchanged: every version 2 key and its meaning, the parts library, the
  backend.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` for partial, with what remains. Mark moot tasks
> `- [x] ~~text~~ — moot: <evidence>`. Fill each `Commit:` line when the phase
> lands. **An unticked box means NOT DONE.**

### Phase 1 — Bus model and routing

Pure model code with no callers yet. It lands alone with no visible change.

- [ ] Add `schematical/schematical/Canvas/BusModel.swift` with `BusWire`,
      `Bus` (constants, `defaultName`) and `BusRoute` (`route`, `isValid`,
      `moveSegment`, `moveNode`, `insertNode`) as in § 4–5. Add it to the
      `schematical` target in `project.pbxproj`.
- [ ] `SchematicModel.swift`: `buses`, `selectedBusID` (with the three
      `didSet`s clearing each other), `replaceContents(…, buses:)` (all
      callers pass `[]` for now), `addBus`, `setBusWireCount`,
      `renameBusWire`, `setBusPoints`, `busHit`, `busNodeHit`,
      `insertBusNode`, and the bus branch of `erase`.

Acceptance: the app builds, and opening `test.schematical` still shows
its contents.
  Check: the build command from § Prerequisites → `** BUILD SUCCEEDED **`
  (est. 3 min); launch the app, open `test.schematical`, screenshot (est. 2 min).
Commit: —

### Phase 2 — Bus tool, drawing, selection, editing, erasing

- [ ] `SchematicModel.swift`: `SchematicTool.bus` (title, symbol).
- [ ] `SchematicCanvasView.swift`: `BusLayer` under `WireLayer`; the
      `.drawingBus`, `.dragBusNode` and `.dragBusSegment` interactions; the
      Bus tool branch in `handleChanged` / `handleEnded`; bus hits in
      `beginPointerInteraction` and `updateHover`; the bus node insert on
      double-click; the header comment.

Acceptance: drawing an L-shaped bus, dragging its corner, dragging a
segment, and double-clicking to add a node all keep right angles. A drag
that would fold a segment back stops at the last good shape. Erasing
removes the bus.
  Check: build; in the app, draw a bus from (100,100) to (300,200), drag the
  middle vertical segment right by 40, drag the end node down by 30, and
  screenshot after each step; erase it and screenshot (est. 8 min; no app
  test target, so screenshots are the smallest check of drag behaviour).
Commit: —

### Phase 3 — Bus inspector panel

- [ ] Add `schematical/schematical/Drawer/BusPanels.swift` with `BusPanel`
      (§ 6) and add it to the target.
- [ ] `SchematicSidebarView.swift`: show `BusPanel` for `selectedBusID`.

Acceptance: a new bus shows 8 wires `B0`–`B7`; setting the count to 12 adds
`B8`–`B11`; setting it to 1 clamps to 2; renaming `B1` to `D1` works;
renaming `B2` to `d1` shows "d1 is already used on this bus."; renaming to
`ADDR0` shows "A wire name is at most 4 characters.".
  Check: build; perform each step and screenshot the panel after it (est. 6 min).
Commit: —

### Phase 4 — Save and open (version 3)

- [ ] `SchematicFile.swift`: `currentVersion = 3`; `struct Bus` (`id`,
      `points`, `wires: [BusWireEntry]`) and `BusWireEntry` (`id`, `name`);
      `var buses: [Bus]? = nil`; `init(document:)` writes `buses` (always, even
      empty); `apply` validates (§ Compatibility), claims IDs, and passes
      buses to `replaceContents`; `SchematicFileError.badBus`.
- [ ] `docs/file-format.md`: version 3 in the intro and the top-level table,
      a "Buses" section (fields, validation messages), and version 2 → 3 in
      § Version 1 files (renamed "Older versions": version 2 files open with
      no buses).

Acceptance: a saved file holds the bus exactly, and reopening it shows the
same bus with the same names. A version 2 file still opens.
  Check: build; draw a bus with 12 wires, rename `B1` to `D1`, save to
  `/tmp/p13a.schematical`; `jq '.version, (.buses|length), (.buses[0].wires|length), .buses[0].wires[1].name' /tmp/p13a.schematical`
  → `3`, `1`, `12`, `"D1"`; reopen and screenshot the panel; open
  `test.schematical` (version 2) and screenshot (est. 6 min).
Commit: —

## Validation Plan

- Tests: no app test target. Each phase has a screenshot check, and
  Phase 4 adds a `jq` check of the saved file.
- Coverage check: Phase 2's steps cover every `BusRoute` function (route,
  segment move, node move, insert, refusal); Phase 4 covers the save and open
  paths.
- Runtime proof: Phase 4's save → `jq` → reopen.
- Doc sync: `docs/file-format.md` (Phase 4); the canvas header comment
  (Phase 2).
- Final gate for A: the build → `** BUILD SUCCEEDED **`, and `git status
  --short backend` → no output (the backend is untouched) (est. 4 min). The
  landing tier for all of plan-13 runs once, at the end of plan-13-C.

## Decisions

- Undo is out of plan-13 — decided by the owner, 2026-11-01. The app has no
  undo today; bus edits get none either.
- A bus line is 4× a wire's width (1.5 → 6) — owner, 2026-11-01.
- Wire count 2–999 — owner, 2026-11-01.
- Each bus wire has an internal ID (UUID, used in the file and by ports) and
  a display name of at most 4 characters, shown on the schematic — owner,
  2026-11-01.
- Default names are `B0`, `B1`, … (numbering from 0) — owner, 2026-11-01.
- Names are unique within a bus — owner, 2026-11-01.
- Wires are only added or removed at the end of the list — owner, 2026-11-01.
- Bus routes have right-angle bends and never branch, and dragging keeps
  right angles (not a copy of wire dragging) — owner, 2026-11-01.
- Nothing connects to the bus line itself — owner, 2026-11-01.
- A new bus has 8 wires; uniqueness is case-insensitive; names can't contain
  whitespace — author's call, 2026-11-01 (§ 4).

## Corrections

## Summary

A delivers the bus as a drawn, editable, named, saved object with no
electrical meaning. The risk is in `BusRoute`'s drag rules, which plan-13-C
extends with port checks. Wires, the netlist, the PCB and the backend are
untouched.
