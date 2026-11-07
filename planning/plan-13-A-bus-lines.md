# plan-13-A: Buses — the bus line

Last updated: 2026-11-07
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

- a bus is made of straight **bus lines** (horizontal or vertical, two ends,
  no nodes in between). Lines of one bus meet only at right angles, in one
  of two ways: an **L** (two line ends meet at a corner) or a **T** (a line
  end meets the middle of another line, marked with a dot);
- the **Bus** tool: drag from one grid point to another. The result is a
  straight line or an L, never a diagonal. Like the Line tool, a drag can start
  or end on an existing bus line, which joins the new lines to that bus (a T
  on its middle, an L at its free end), or extends it when the new line
  continues straight out of a free end. The line is 6 wide (the wire's
  1.5 × 4);
- buses are drawn, hovered and selected (Pointer), and erased one line at a
  time (Eraser);
- a line can be dragged sideways, and a free end, corner or T end dragged,
  with every line staying straight and every joint staying an L or T;
- the **Bus** inspector panel: the wire count (2–999) and each wire's
  display name (1–4 characters, default `B0`, `B1`, …, unique in its bus);
- buses are saved and opened (file format version 3).

Outcome of A: a bus of joined lines can be drawn, extended, branched,
edited, renamed, resized, erased, saved and reopened. Nothing connects to it
yet.

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
| plan-11 complete | `ls planning/plan-11-* 2>/dev/null` → no output | MET (2026-11-07: all in `planning/completed`) |
| plan-12 complete | `ls planning/plan-12-[A-K]-*.md 2>/dev/null` → no output | MET (2026-11-07: all in `planning/completed`) |
| The canvas change in progress is committed | `git status --short schematical/schematical/Canvas/SchematicCanvasView.swift` → no output | MET (2026-11-07: no output after the barrel-jack work was committed in `28abedd`) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p13-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-07: `** BUILD SUCCEEDED **`) |
| No bus code yet | `grep -rn "case bus\|struct Bus\b\|\"buses\"" schematical/schematical` → no output | MET (2026-11-07) |
| File format is version 2 | `grep -n "currentVersion = " schematical/schematical/Document/SchematicFile.swift` → `currentVersion = 2` | MET (2026-11-07) |

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

- With the Bus tool, a drag from grid point P to grid point Q (P ≠ Q), both
  off every bus, adds a bus with 8 wires named `B0`…`B7`. If P and Q share
  x or y, the bus is one line P–Q. Otherwise it is an L: the line P–(Q.x,
  P.y) and the line (Q.x, P.y)–Q. Lines are drawn 6 wide.
- A drag that starts or ends on a line of an existing bus adds its new
  line(s) to that bus instead (§ 4, "Joining"): a T on the middle of a line,
  an L at a free end, or a longer line when the new line continues straight
  out of a free end. A drag that would join two different buses, or make any
  joint other than a T or an L, is refused with a message.
- Clicking a bus line with the Pointer selects its bus, and the inspector
  shows the **Bus** panel. The wire count can be set from 2 to 999. Each
  wire's name can be changed to 1–4 non-blank characters, unique in the bus
  (case-insensitive).
- Dragging a line moves it sideways only. Dragging a free end lengthens or
  shortens its line. Dragging a corner moves both lines of the L. Dragging a
  T end slides it along the line it joins. Every line joined to a moved line
  stretches to stay joined. A step that would break a joint, shrink a line
  to 0, or touch another line is refused and the drag stays at the last good
  shape.
- The Eraser removes the line it touches. If what is left of the bus falls
  into separate pieces, each extra piece becomes a bus of its own, with the
  same wire names.
- Save writes `"version": 3` and a `"buses"` array. Opening the file restores
  every bus exactly (lines, wire IDs, names). Version 1 and 2 files still
  open.

### Non-goals (explicit constraints)

- **No undo.** The app has none today, and the owner decided not to add it in
  plan-13 (2026-11-01).
- **No change to wires, junctions or the Line tool.** `Wire`, `WireEnd`,
  `Junction`, `OrthogonalRoute` and wire editing behave exactly as today. Bus
  editing uses its own helper (`BusEdit`); the owner asked that it not be a
  copy of the wire logic, because buses keep right angles when dragged.
- **No change to the PCB page, the backend, or the part library.** A bus is
  not a part.
- **No connectivity in A.** Nothing electrical attaches to a bus line, now or
  later: wires attach only to port pins (plan-13-B). A wire crossing a bus
  does not connect to it.
- **No diagonal lines, and no nodes inside a line.** Every bus line is
  horizontal or vertical and straight from end to end. The Bus tool draws
  one line or one L. There is no double-click bend on a bus. Any shape made
  of such lines (an H, a comb, steps) is built from more drags, each joining
  at a T or an L (owner, 2026-11-07).
- **Two buses are never joined.** Their wire lists differ, so there is no
  sensible merge. Lines of different buses may cross, but no line end may
  touch another bus.

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
  no bends inside a line, and every line must stay horizontal or vertical
  while it is dragged.
- **Wire branches.** A wire branches through a `Junction` (`:611`), a fixed
  grid point. `junction(at:)` (`:1252`) splits the wire it lands on into two
  wires that both end there, and `pruneJunctions` (`:1295`) drops junctions
  no wire ends at. `WireLayer` draws a solid dot at each junction. Buses join
  the same way from the user's side (start or end a drag on a line), but a
  T doesn't split the line it joins: that line stays one straight line, so it
  can still be dragged (§ 3, rejected alternatives).
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

1. **Model** (`Canvas/BusModel.swift`, new): `Bus`, `BusLine`, `BusWire`,
   the joint rules (`Bus.joints`, `Bus.isValid`), `BusEdit` (drags), plus
   `SchematicDocument` methods for buses. This is pure value logic.
2. **Canvas and inspector**: the Bus tool, a `BusLayer` drawn under the
   wires, Pointer hit-testing for buses, the Eraser, and a `BusPanel` in
   `SchematicSidebarView`.
3. **File**: version 3, with the `buses` array.

A bus stores only its lines. Joints are not stored: they are worked out from
where line ends touch (§ 4). Every edit builds the new bus, then keeps it
only if it is valid and has the same joints as before (or, for the Bus tool,
exactly the joints the new lines were meant to make). So one rule set
decides what is allowed everywhere.

Risk sits in **`BusEdit` drags** (keeping every joined line attached while
one moves). plan-13-C adds port checks to the same "build, then check"
step, so its rules are stated exactly here (§ 5).

**Correctness gate:** the app build, runtime screenshots, and `jq` checks of a
saved file. Byte-identity does not apply: the file format changes on
purpose.

**Rejected alternatives:**

- *Reuse `Wire` for the bus line.* Wire ends must be pins or junctions, wires
  feed the netlist and voltage check, and wire bends may go diagonal while
  dragged. Each of those is wrong for a bus. A separate type keeps the wire
  code unchanged.
- *Bus routes as polylines with bends, like wires.* The owner allows only
  straight lines joined at a T or an L (2026-11-07). Polylines would also
  need rules for nodes inside a line.
- *Split a line at a T, like a wire junction.* The joined line would become
  two lines meeting straight on, which is neither a T nor an L, and the two
  halves could no longer be dragged as one.
- *Store joints explicitly (line ID + position).* Every drag would have to
  update the stored joints and keep them in step with the geometry. Working
  joints out from geometry and requiring them unchanged after a drag gives
  the same behaviour with nothing to keep in step.
- *Join two buses into one when a drag connects them.* The two wire lists
  would have to be merged (counts, names, port pins). Refusing keeps every
  bus's wires its own.
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

/// One straight piece of a bus. Horizontal or vertical, on the grid,
/// length > 0. Nothing lies between its two ends.
struct BusLine: Identifiable, Hashable {
    let id: UUID
    var a: CGPoint
    var b: CGPoint
}

struct Bus: Identifiable, Hashable {
    let id: UUID
    /// ≥ 1 line, all joined into one piece by T and L joints.
    var lines: [BusLine]
    /// The bus's wires in list order. 2…999 of them. Every line carries all
    /// of them.
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

**Joints.** `Bus.joints -> Set<BusJoint>`, worked out from the lines.
Wherever a line end touches another line of the same bus, there is a contact
point. Each one must be exactly one of:

- **L** (`.corner(line1, line2, at:)`): exactly two line ends meet there, the
  two lines are at a right angle, and no other line touches that point;
- **T** (`.tee(end: line, on: line, at:)`): exactly one line end lies
  strictly inside one other line, the two are at a right angle, and no other
  line touches that point.

Anything else at a contact point (two lines meeting straight on, three
ends, an end on a corner, an end on a T) is a **bad joint**. Lines that
cross in their middles do not touch: that is a crossing, not a joint, as
for wires.

**`Bus.isValid`**: at least one line; every line horizontal or vertical, on
the grid, length > 0; no two lines overlap along their length; no bad
joints; and every line reachable from every other through joints (one
piece). `SchematicDocument.busesAreApart(_ buses:)` adds the cross-bus rule:
no line end of one bus touches any line of another.

Helpers: `Bus.line(_ id:)`, `Bus.joints(of lineID:)`, `Bus.pieces() ->
[[BusLine]]` (lines grouped by joints, for erase), `BusJoint.point`.

**`SchematicDocument` additions** (`SchematicModel.swift`):

- `private(set) var buses: [Bus] = []`, included in `replaceContents` (new
  `buses:` parameter) and cleared/replaced there.
- `var selectedBusID: UUID?`. Its `didSet` clears `selectedChipID` and
  `selectedTextID`, and theirs clear it (extend the two existing `didSet`s).
- `@discardableResult func addBus(from: CGPoint, to: CGPoint) -> UUID?`: the
  Bus tool. See "Joining" below. On success it selects the bus it made or
  grew and returns its ID. On refusal it returns nil, sets `editorMessage`,
  and changes nothing.
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
- `@discardableResult func setBusLines(_ id: UUID, _ lines: [BusLine]) ->
  Bool`: the one way a drag changes a bus. It stores `lines` only if the new
  bus passes `isValid`, has the same `joints` as the stored bus (same kinds,
  same lines; points may move), and passes `busesAreApart`. Otherwise it
  changes nothing and returns false.
- `func busHit(at:tolerance:) -> (bus: UUID, line: UUID)?` and
  `func busEndHit(at:tolerance:) -> (bus: UUID, line: UUID, end: BusEnd)?`
  (`BusEnd` is `.a` / `.b`). They mirror `segmentHit` / `nodeHit`. A corner
  is two line ends at one point; `busEndHit` returns either, and the drag
  (§ 5) handles the pair.
- `erase(at:)` gains a last branch: a bus hit removes that line. If the bus
  has no lines left, the bus is removed (and `selectedBusID` cleared if it
  was selected). If the rest falls into more than one piece (`pieces()`),
  the piece holding the bus's first remaining line keeps the bus; each other
  piece becomes a new bus with a fresh ID and a copy of the wire list (same
  names, fresh wire IDs). (Order: chip → wire → bus. plan-13-B puts ports
  before wires and moves ports with their piece.)

**Joining** (`addBus(from: P, to: Q)`, both snapped). It refuses when P = Q.
It looks up what each of P and Q touches: nothing, the **middle** of a bus
line, a **free end** (a line end in no joint), or anything else (a corner, a
T, or a line end already joined). Then:

1. Anything else at P or Q → "A bus line can only join another at a T or an
   L.". P and Q on two different buses → "Two buses can't be joined.".
2. The new lines are `[P–Q]` if P and Q share x or y, else the L
   `[P–(Q.x, P.y), (Q.x, P.y)–Q]`. If that L makes a bad joint at P or Q,
   it tries the other L, `[P–(P.x, Q.y), (P.x, Q.y)–Q]`. The preview shows
   the same choice.
3. At a free end, a new line that continues straight out of it is merged
   into that line (the bus is **extended**) instead of meeting it straight
   on. A new line that runs back over it is refused as an overlap.
4. The new lines are added to the touched bus (or to a new bus with 8
   default wires, when P and Q touch nothing), then the result is checked:
   `isValid`, `busesAreApart`, and its joints are the old ones plus the ones
   made at P, Q and the L's corner. Anything else (for example the new line
   landing on another line along the way) → "A bus line can only join
   another at a T or an L.".

Both ends may touch the same bus: that closes a loop, which is allowed, as
it is for wires.

## 5. `BusEdit` (dragging, right angles kept)

`enum BusEdit` in `BusModel.swift`. Each function takes the bus as it was at
drag start (`base`) and returns the new line list, or nil when the step has
no valid meaning. The canvas passes non-nil results to `setBusLines`, which
does the joint check (§ 4). A nil result or a false return leaves the bus
as it was after the last good frame.

- `moveLine(_ bus:, line:, by offset:) -> [BusLine]?`: keeps only the
  component of `offset` across the line (y for a horizontal line, x for a
  vertical one), snapped, and moves both ends of the line. Then, for every
  joint on that line, the other line keeps up:
  - an **L** at one of its ends: the other line's end at that corner moves
    with it, so the other line gets longer or shorter;
  - a **T** where another line ends on its middle: that line's end moves
    with it, so that line gets longer or shorter;
  - a **T** where this line ends on another line's middle: this end now
    sits further along that line. Nothing else moves.
  Only the dragged line and one end of each line joined to it change. Lines
  further out never move.
- `moveEnd(_ bus:, line:, end:, to p:) -> [BusLine]?`, by what the end is:
  - **free end**: `p` is projected onto the line's own axis, so the line
    gets longer or shorter (this is how a line is extended with the
    Pointer);
  - **corner**: the corner goes to `p`. That is `moveLine` of each of the
    two lines by its across part of the move, applied one after the other;
  - **T end**: `p` is projected onto the line it joins, so the end slides
    along it. That is `moveLine` of this line by the along part of the move.

What `setBusLines` then refuses: a line shrinking to length 0 or turning
over, a T end sliding off the end of its line, a moved line touching a line
it was not joined to (its own bus or another), or any joint changing kind.

## 6. Canvas, tool and inspector

- `SchematicTool.bus`: title "Bus", symbol `line.3.horizontal`. Placed after
  `.line` in the enum, so the picker order is Pointer, Eraser, Line, Bus,
  Flip, VCC, GND, Pull-up, Pull-down (the last two from plan-12-B).
- **Drawing.** `Interaction.drawingBus(from: CGPoint, cursor: CGPoint)`. On
  press, the snapped press point is the start. The preview is the lines
  `addBus` would make to the snapped cursor (§ 4, Joining step 2), drawn 6
  wide, dashed, in the accent colour. On release, `document.addBus(from:to:)`.
  A refusal shows `editorMessage` through the existing toolbar message
  (`ContentView.swift:294`).
- **`BusLayer`** (private view in `SchematicCanvasView.swift`): a `Canvas`
  drawn **under** `WireLayer` (so wires stay visible on top), using the
  `worldCanvas` / `shiftToWorld` helpers. Each line is stroked
  `lineWidth: Bus.lineWidth`, `.butt` caps, in `.primary`. Each L corner gets
  a filled 6 × 6 square so the corner is closed. Each T gets a filled dot 12
  across (twice the line width), like a wire junction's dot. Hover and
  selection are per bus, every line included: the hovered bus gets a 12-wide
  accent underlay at 0.35 opacity; the selected bus is stroked in
  `.accentColor`.
- **Pointer.** `beginPointerInteraction` order becomes chip → wire bend →
  wire segment → bus end → bus line → pan. A bus hit selects the bus
  (`selectedBusID`) and starts `.dragBusEnd(bus:base:line:end:)` or
  `.dragBusLine(bus:base:line:)`, which call `BusEdit.moveEnd` /
  `moveLine` with the snapped cursor / delta and `setBusLines` when
  non-nil. Pressing empty space clears `selectedBusID` as it clears
  `selectedChipID`. Bus hit tolerance is `lineTolerance + 3 / zoom` (half
  the bus width added).
- **Hover.** `updateHover` checks buses after chips and wires
  (`hoveredBusID`).
- **Double-click.** Unchanged: it adds wire bends only. A bus has no bends
  inside a line.
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
  in its tool list ("drag to draw a bus line or L; start or end on a bus to
  join it at a T or L, or to extend it").

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
      "lines": [
        { "id": "…", "a": { "x": 100, "y": 200 }, "b": { "x": 300, "y": 200 } },
        { "id": "…", "a": { "x": 180, "y": 200 }, "b": { "x": 180, "y": 300 } }
      ],
      "wires": [ { "id": "…", "name": "B0" }, { "id": "…", "name": "B1" } ] }
  ]
  ```

  (The second line ends on the middle of the first: a T.) Joints are not
  written; they are worked out from the lines on open.
  `SchematicFile.buses: [Bus]?` (nil in older files, which means none).
  plan-13-B adds `"ports"` inside each bus, and a port wire end.
- Opening validates, and fails with `SchematicFileError.badBus(String)`
  ("The bus is not valid: <detail>."). Details:
  - `bus <id> has no lines`;
  - `line <id> is not a horizontal or vertical line on the grid`
    (diagonal, off the grid, or length 0);
  - `lines <id> and <id> overlap`;
  - `bus <id> has a joint at (<x>, <y>) that is not a T or an L`;
  - `bus <id> is not one piece`;
  - `buses <id> and <id> touch at (<x>, <y>)`;
  - `bus <id> has <n> wires; it must have 2 to 999`;
  - `bus <id> wire name "<name>" must be 1 to 4 characters with no spaces`;
  - `bus <id> uses the wire name "<name>" twice`.
- Bus IDs, bus line IDs and bus wire IDs are claimed in the same ID set as
  parts, junctions, wires and texts. A duplicate fails with the existing
  `duplicateID`.
- Unchanged: every version 2 key and its meaning, the parts library, the
  backend.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` for partial, with what remains. Mark moot tasks
> `- [x] ~~text~~ — moot: <evidence>`. Fill each `Commit:` line when the phase
> lands. **An unticked box means NOT DONE.**

### Phase 1 — Bus model and editing rules

Pure model code with no callers yet. It lands alone with no visible change.

- [x] Add `schematical/schematical/Canvas/BusModel.swift` with `BusWire`,
      `BusLine`, `BusEnd`, `BusJoint`, `Bus` (constants, `defaultName`,
      `joints`, `isValid`, `pieces`, helpers) and `BusEdit` (`moveLine`,
      `moveEnd`) as in § 4–5, plus `BusJoin` (the joining rules). ~~Add it to
      the `schematical` target in `project.pbxproj`.~~ — moot: the target uses
      a `PBXFileSystemSynchronizedRootGroup` (`grep -n
      PBXFileSystemSynchronized schematical/schematical.xcodeproj/project.pbxproj`
      → lines 13–25), so new files join it with no project edit.
- [x] `SchematicModel.swift`: `buses`, `selectedBusID` (with the three
      `didSet`s clearing each other), `replaceContents(…, buses:)` (all
      callers pass `[]` for now), `busesAreApart`, `addBus` (new bus, join,
      extend, and its refusals), `setBusWireCount`, `renameBusWire`,
      `setBusLines`, `busHit`, `busEndHit`, and the bus branch of `erase`
      (with the split into pieces).
- [x] Added: `scripts/harness/bus-lines.swift`, a harness check that runs
      Phase 2's steps through the model calls the gestures make →
      `ALL OK` (every step's shape, joint, refusal message and split as
      Phase 2 expects); `scripts/harness/open-file.swift` and
      `support/Harness.swift` to open a file and render the real canvas.

Acceptance: the app builds, and opening `test.schematical` still shows
its contents.
  Check: the build command from § Prerequisites → `** BUILD SUCCEEDED **`
  (est. 3 min); launch the app, open `test.schematical`, screenshot (est. 2 min).
  Result (2026-11-07): `** BUILD SUCCEEDED **`; harness `open-file.swift
  test.schematical` → "opened test.schematical: 5 parts, 6 wires, 0 buses",
  canvas PNG shows the oscillator, LED, 5V and both GNDs wired as before.
Commit: 8057e51

### Phase 2 — Bus tool, drawing, joining, editing, erasing

- [x] `SchematicModel.swift`: `SchematicTool.bus` (title, symbol).
- [x] `SchematicCanvasView.swift`: `BusLayer` under `WireLayer` (lines,
      corner squares, T dots); the `.drawingBus`, `.dragBusEnd` and
      `.dragBusLine` interactions; the Bus tool branch in `handleChanged` /
      `handleEnded`; bus hits in `beginPointerInteraction` and
      `updateHover`; the header comment. (`BusLayer` and `BusPreview` are
      internal, not private, so the harness can render a drag's preview;
      a refused drag previews in red.)

Acceptance: the Bus tool draws a straight line or an L, never a diagonal. A drag
from the middle of a line makes a T with a dot; from a free end, an L or a
longer line. Dragging a line, a free end, a corner and a T end keeps every
line straight and every joint a T or L, and joined lines stretch to follow.
A step that would break a joint stops at the last good shape. Joining two
buses and joining at a corner are refused with their messages. Erasing a
line that holds the bus together leaves two buses.
  Check: build; in the app, with the Bus tool:
  1. drag (100,100) → (300,200): an L, lines (100,100)–(300,100) and
     (300,100)–(300,200);
  2. Pointer: drag the vertical line right by 40 (the horizontal line
     stretches to x 340); drag the free end at (100,100) left to (60,100);
     drag the corner (340,100) to (360,80);
  3. Bus tool: drag (200,80) → (200,20) (a T with a dot); drag (200,50) →
     (250,50) (a T on that line); drag (60,80) → (20,80) (extends the first
     line: still one line);
  4. Pointer: drag the line at y 80 down by 20 (the T line at x 200 and the
     corner line both stretch; the T at y 50 stays); drag the T end at
     (200,50) up past (200,20) (it slides along the x 200 line and stops at
     y 30, one grid step inside its end);
  5. Bus tool: drag from the corner (360,100) → (420,100) (refused: corner);
     draw a second bus (100,300) → (200,300), then drag (150,300) → (150,100)
     (refused: two buses);
  6. Eraser: erase the line at x 200 (the line now at y 30 becomes its own
     bus; select it and the panel shows `B0`–`B7`); erase everything else.
  Screenshot after each numbered step (est. 12 min; no app test target, so
  screenshots are the smallest check of drag behaviour). If a step's shape
  differs, re-pick points on the drawn lines and note it in § Corrections.
  Result (2026-11-07): build → `** BUILD SUCCEEDED **`; harness
  `scripts/harness/bus-lines.swift` (each drag = its frames' `BusEdit` +
  `setBusLines` calls against the press-time bus) → `ALL OK`, 32 checks,
  e.g. 2c "(60,80)-(360,80) (360,80)-(360,200) joints: L(360,80)", 4b "T end
  stops at y 30", 5a "A bus line can only join another at a T or an L.",
  5b "Two buses can't be joined.", 6a three buses with the y 30 line alone,
  `B0`–`B7`, fresh wire ids. Canvas PNGs `a2-1` … `a2-6b` (plus the T and
  refused previews) show every shape as listed, corner squares and T dots
  drawn.
Commit: 1514cab

### Phase 3 — Bus inspector panel

- [x] Add `schematical/schematical/Drawer/BusPanels.swift` with `BusPanel`
      (§ 6) (in the target through the synchronized folder, as in Phase 1).
- [x] `SchematicSidebarView.swift`: show `BusPanel` for `selectedBusID`.
- [x] Added: `Harness.renderHosted` (an offscreen `NSHostingView`), since
      `ImageRenderer` draws a `Form` blank.

Acceptance: a new bus shows 8 wires `B0`–`B7`; setting the count to 12 adds
`B8`–`B11`; setting it to 1 clamps to 2; renaming `B1` to `D1` works;
renaming `B2` to `d1` shows "d1 is already used on this bus."; renaming to
`ADDR0` shows "A wire name is at most 4 characters."; clicking any line of a
joined bus shows the same panel.
  Check: build; perform each step and screenshot the panel after it (est. 6 min).
  Result (2026-11-07): build → `** BUILD SUCCEEDED **`; harness
  `scripts/harness/bus-panel.swift` → `ALL OK`: names `B0`–`B7`; count 12 →
  `B8`–`B11`; count 1 → `B0,B1`; `B1`→`D1` stored; `B2`→`d1` returns "d1 is
  already used on this bus." (B2 kept); `ADDR0` returns "A wire name is at
  most 4 characters." (B3 kept); all three lines of a joined bus hit the
  same bus. Sidebar PNGs `a3-1` … `a3-5` show the panel with each state
  (Wires 8 / 12 / 2, `D1`, "3 lines"). The red caption is field state the
  harness can't set; it shows the returned string, as `ReferenceSection` does.
Commit: —

### Phase 4 — Save and open (version 3)

- [ ] `SchematicFile.swift`: `currentVersion = 3`; `struct Bus` (`id`,
      `lines: [BusLineEntry]`, `wires: [BusWireEntry]`), `BusLineEntry`
      (`id`, `a`, `b`) and `BusWireEntry` (`id`, `name`);
      `var buses: [Bus]? = nil`; `init(document:)` writes `buses` (always, even
      empty); `apply` validates (§ Compatibility), claims IDs, and passes
      buses to `replaceContents`; `SchematicFileError.badBus`.
- [ ] `docs/file-format.md`: version 3 in the intro and the top-level table,
      a "Buses" section (fields, the T and L rule, validation messages), and
      version 2 → 3 in § Version 1 files (renamed "Older versions": version 2
      files open with no buses).

Acceptance: a saved file holds the bus exactly, and reopening it shows the
same lines, joints and names. A version 2 file still opens.
  Check: build; draw an L bus plus a T line on it, set 12 wires, rename `B1`
  to `D1`, save to `/tmp/p13a.schematical`;
  `jq '.version, (.buses|length), (.buses[0].lines|length), (.buses[0].wires|length), .buses[0].wires[1].name' /tmp/p13a.schematical`
  → `3`, `1`, `3`, `12`, `"D1"`; reopen and screenshot the canvas and the
  panel; open `test.schematical` (version 2) and screenshot (est. 6 min).
Commit: —

## Validation Plan

- Tests: no app test target. Each phase has a screenshot check, and
  Phase 4 adds a `jq` check of the saved file.
- Coverage check: Phase 2's steps cover drawing (line, L), every join kind
  (T, L, extend), both refusals, every drag (`moveLine`, and `moveEnd` on a
  free end, a corner and a T end), a refused step, and erase with a split.
  Phase 4 covers the save and open paths.
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
- Dragging keeps right angles (not a copy of wire dragging) — owner,
  2026-11-01.
- Nothing electrical connects to the bus line itself — owner, 2026-11-01.
- Bus lines join other bus lines the way wires join wires (start or end a
  drag on one), but only at a right angle: a T or an L. Lines are straight
  with no nodes in between; a line can be extended from its end; the Bus
  tool draws a line or an L; no line is ever diagonal; any shape built
  from such lines is allowed — owner, 2026-11-07 (replaces
  "never branch" and the double-click bend, 2026-11-01).
- A new bus has 8 wires; uniqueness is case-insensitive; names can't contain
  whitespace — author's call, 2026-11-01 (§ 4).
- Two buses are never joined; loops within one bus are allowed; erasing a
  line that splits a bus makes the extra pieces new buses with the same wire
  names; the L corner's direction is picked automatically (the other L is
  tried when the first makes a bad joint) — author's call, 2026-11-07 (§ 4).

## Corrections

- **Screenshots come from the harness, not a desktop session** (2026-11-07).
  This session has no tool that clicks or drags in a desktop GUI. As in
  plan-09-E and plan-11-D, app checks run in the headless harness, now kept
  at `scripts/harness/build.sh`: it compiles the app's own sources with the
  backend, drives the same `SchematicDocument` / `SchematicFile` methods the
  gestures call (a drag is replaced by the calls its frames make), and
  renders the real canvas and sidebar views offscreen to PNG.
- **Joints name line ends, not just lines** (2026-11-07). § 4 writes
  `.corner(line1, line2, at:)` and `.tee(end: line, on: line, at:)`. A line
  can have joints at both ends, so "same joints after a drag" needs to know
  *which* end: `BusJoint` holds `BusLineEnd(line, end)` for each joining end.
  The rule it checks is unchanged.
- **The other L is tried whenever the first fails** (2026-11-07). § 4 step 2
  tries the second L only when the first makes a bad joint at P or Q. The
  code (`BusJoin.join`) tries it whenever the first L fails any check
  (including the first L running back over the line it starts on, which is
  an overlap, not a bad joint). It only accepts what the full check allows,
  so nothing invalid gets in; the preview uses the same function.

## Summary

A delivers the bus as straight lines joined at T and L joints: drawn,
joined, extended, edited, named, saved, with no electrical meaning. Joints
are worked out from the lines, and every edit is kept only if its joints are
unchanged, which plan-13-C extends with port checks. Wires, the netlist, the
PCB and the backend are untouched.
