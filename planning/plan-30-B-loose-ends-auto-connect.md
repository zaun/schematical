# plan-30-B: Loose ends — auto-connect on drop

Last updated: 2026-11-08
Effort: medium (1h–2h)
Depends on: plan-30-A

B reconnects loose ends. When the user finishes placing, moving or flipping a
block, every pin tip of that block that sits exactly on a loose junction (A)
takes over that junction's wires: each end on the junction becomes an end on
the pin, and the junction goes away. The same happens when a bus port is slid
or its bus is dragged, and when a loose junction itself is dragged onto a
pin.

The outcome: erase a 74HC00 with wires, drop a 74HC08 at the same spot, and
every wire whose loose end matches one of its pin tips is on that pin again.

References:

- plan-30-A § 4.1 (`Junction.isLoose`, `loosen`).
- `schematical/schematical/Canvas/SchematicModel.swift`: `placeChip` :1361,
  `flipChip` :1505, `schematicGroupBase` :1516, `settleSchematicGroup` :1589,
  `pinTip` :2263, `busPinTip`, `connect` :2405 (its VoltageCheck use at
  :2413), `settleWires(attachedToBus:)` :2551.
- `schematical/schematical/Canvas/SchematicCanvasView.swift`: `handleEnded`
  :756 (the `.slidePort` case :777).
- `schematical/schematical/Simulation/VoltageCheck.swift`: `firstNew` :111,
  `message` :120.

## Prerequisites

plan-30-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-30-A complete | `grep -c "^Commit: —" planning/plan-30-A-loose-ends-keep-wires.md` → `0` (or the file is in `planning/completed/`) | NOT MET (2026-11-08: plan just written) |

## 1. Goal

- After a drop, each loose junction whose point equals a pin tip of a
  dropped thing is gone, and every wire end that was on it is
  `.pin(ref)` (or `.busPin(ref)`) for that pin. The wires' routes are
  unchanged, since the tip is the same point.
- A drop never connects anything except loose junctions. A pin landing on
  the middle of a wire, on a plain junction, or on another wire's pin end
  connects nothing, as today.

### Non-goals (explicit constraints)

- No undo. The app has none. `planning/todo.md`'s undo item notes that an
  auto-connect must be one step with the drop that caused it.
- Removals (A's five sites, plus `setPortWires`, `setPortSide` and
  `setHeaderSize`) never auto-connect, even though some of them move pins.
  Unticking a port pin can shift the next pin by a pitch onto the spot that
  was just loosened (`setPortWires` comment, :2802), and that must not
  connect the wire to a different bus wire.
- No drawing changes (C).

## 2. Current State

- A drag of parts, junctions or wire bends is a group drag. Each frame is
  `moveSchematicGroup`; on release, `handleEnded` calls
  `settleSchematicGroup(base)` (SchematicCanvasView :780). `base.chips` and
  `base.junctions` hold the ids that moved (:1520–1523).
- `placeChip` (:1361) appends the new chip and returns. It has no wire work.
- `flipChip` (:1505) toggles the flip and calls `settleWires(attachedTo:)`.
- Bus drags end in `settleWires(attachedToBus:)` (:2551, canvas :776).
  Port slides end in `settleWires(attachedToPort:)` (canvas :778), which is
  also called by `setPortWires` and `setPortSide`, so B must not hook it.
- `connect` (:2405) refuses a wire that would create a new voltage problem.
  It builds the candidate wire list, calls `VoltageCheck.firstNew(before:…
  after:…)`, and on an issue sets `editorMessage` and changes nothing.

### Measured populations

| What | Count | Command |
|---|---|---|
| Drop points that call the new hook | 5 | `placeChip` :1361, `flipChip` :1505, `settleSchematicGroup` :1589, `settleWires(attachedToBus:)` :2551, canvas `.slidePort` :777 (read `handleEnded` :756–795 for every case that ends a move) |
| Callers of `settleWires(attachedToPort:)` | 5 | `grep -n "settleWires(attachedToPort" schematical/schematical/Canvas/*.swift` → SchematicModel :2553, :2689, :2803, :2822, canvas :778 |

### Verified properties

- **Tips compare with `==`.** Chip origins are snapped (`placeChip` :1365,
  `moveSchematicGroup` :1546). Pin tips add layout offsets on the grid. A
  loosened junction sits at a former tip (plan-30-A § 2). Read the code
  above.
- **`base` identifies what moved.** `SchematicGroupBase.chips` and
  `.junctions` are keyed by the selected ids (:1517–1524). Read it.

## 3. Design Overview

One model function, called at the five drop points with the ids that moved:

```swift
/// Connects loose ends to pins after a drop. A loose junction joins a pin
/// when its point is that pin's tip and either the junction or the pin's
/// part or port is in the dropped set. Each candidate goes through the same
/// voltage check as `connect`. One that fails stays loose and sets
/// `editorMessage`.
func adoptLooseEnds(chips: Set<UUID> = [], ports: Set<UUID> = [], junctions: Set<UUID> = [])
```

Algorithm:

1. Build `dropped: [CGPoint: WireEnd]` from every pin of `chips` (`.pin`) and
   every pin of `ports` (`.busPin`). If `junctions` is not empty, also build
   `every: [CGPoint: WireEnd]` from every pin tip in the document. When two
   pins share a tip, keep the first in `self.chips` order, then bus order.
2. For each junction with `isLoose`, in array order, look up its point in
   `every` if the junction is in `junctions` (a dragged loose end may land
   on any pin), and in `dropped` otherwise. If there is no match, skip it.
3. Build `candidate = wires` with every `.junction(id)` end replaced by the
   pin end. If `library` is set and `VoltageCheck.firstNew(before: chips,
   wires, after: chips, candidate, buses: buses, library:)` returns an
   issue, set `editorMessage = VoltageCheck.message(issue, chips: chips,
   edit: .connect)` and skip this junction.
4. Otherwise `wires = candidate`, remove the junction, and continue.
5. At the end, if any junction was removed, call `pruneSelection()`.

Correctness risk is in step 2's scoping. A global scan would connect a loose
end left on an untouched part's pin tip the next time anything at all is
dropped. Scoping to what was dropped stops that.

Rejected alternatives:

- **Hook `settleWires(attachedTo…)`.** Removals call it (§ 1 Non-goals).
- **Scan every loose junction against every pin on every drop.** That is
  action at a distance (step 2).
- **Connect pins that land mid-wire or on plain junctions.** The owner
  wanted line ends only (§ Decisions).

## 4. Hook points

| Site | Call |
|---|---|
| `placeChip` :1361, after `chips.append` | `adoptLooseEnds(chips: [placed.id])` |
| `flipChip` :1505, after `settleWires` | `adoptLooseEnds(chips: [id])` |
| `settleSchematicGroup` :1589, at the end | `adoptLooseEnds(chips: Set(base.chips.keys), junctions: Set(base.junctions.keys))` |
| `settleWires(attachedToBus:)` :2551, at the end | `adoptLooseEnds(ports: Set(bus(busID)?.ports.map(\.id) ?? []))` |
| canvas `handleEnded` `.slidePort` :777, after `settleWires(attachedToPort:)` | `document.adoptLooseEnds(ports: [port])` |

## Phases

> **NOTE: keep the checkboxes current.** Tick in the same commit as the work.
> An unticked box means NOT DONE.

### Phase 1 — `adoptLooseEnds` and its hooks

- [ ] `Canvas/SchematicModel.swift`: `adoptLooseEnds` (§ 3).
- [ ] The four model hooks in § 4.
- [ ] `Canvas/SchematicCanvasView.swift`: the `.slidePort` hook in § 4.
- [ ] `planning/todo.md`: the undo note is already there (added with this
      plan). Leave it.
- [ ] Tests in `schematicalTests/LooseEndTests.swift`:
  - `testPlacingAPartOnLooseEndsConnectsThem`: erase a part with two wires,
    then `placeChip` the same part at the same centre. Both wires are back
    on the same pins, and no loose junction is left.
  - `testSwappingForAnotherPartConnectsMatchingPins`: erase a 74HC00 with
    wires, place a 74HC08 at the same centre. Every wire is on the pin at its
    tip.
  - `testMovingAPartOntoLooseEndsConnectsThem`: a group drag
    (`schematicGroupBase`, `moveSchematicGroup`, `settleSchematicGroup`)
    that lands a pin on a loose end connects it.
  - `testDraggingALooseEndOntoAPinConnectsIt`: a group drag of the loose
    junction itself onto an untouched part's pin.
  - `testPinOnAWireMiddleDoesNotConnect`: a part dropped so a pin tip is on a
    wire's interior. Wires and junctions are unchanged.
  - `testUnrelatedDropDoesNotConnect`: a loose end left on another part's
    pin tip stays loose when a third, far-away part is placed.
  - `testUntickDoesNotReconnect`: untick a port pin with a wire, while the
    next pin shifts onto that spot. The wire stays loose.
  - `testVoltageConflictStaysLoose`: in a document with the catalog library,
    two `5V` supplies are wired together. Erase one, then place a `3V3`
    supply at the same centre (same supply symbol, so its pin tip is the
    loose end). The junction stays loose, and `editorMessage` starts with
    "Can't connect" (`VOLTAGE_SUPPLY_CLASH`, VoltageCheck :127).
- [ ] `schematicalTests/TestSupport.swift`: `newDocument(library:)`, which
      keeps the document like `newDocument()` does (:22). No app test builds
      a document with a library today (`grep -rn "PartLibraryHandle\|library:"
      schematical/schematicalTests | wc -l` → 0). Pass
      `TestSupport.catalog.library` (PartCatalog.swift:820).

Acceptance: drops connect loose ends to the dropped pins and nothing else, and a voltage conflict refuses.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/LooseEndTests -only-testing:schematicalTests/SelectionGestureTests CODE_SIGNING_ALLOWED=NO` → all pass (est. 4 min).
Commit: —

## Validation Plan

- Tests: `LooseEndTests`, plus `SelectionGestureTests` (group drags go
  through `settleSchematicGroup`).
- Doc sync: none beyond A. The canvas header comment
  (`SchematicCanvasView.swift:5–29`) gains, under Pointer: "Dropping a part
  so a pin lands on a loose wire end connects it."
- The landing tier runs once, at the end of plan-30-C.

## Decisions

- Auto-connect on place and move when pins match loose line ends — decided by
  Justin Zaun, 2026-11-08.
- Only loose ends connect, never mid-wire points — decided by Justin Zaun,
  2026-11-08 ("agreed with everything").
- On drop only, not during the drag — decided by Justin Zaun, 2026-11-08.
- No undo work. The note goes in `planning/todo.md` — decided by Justin
  Zaun, 2026-11-08.
- Bus ports auto-connect too, and removals never do — author's call,
  matching A's rule that port deletion loosens. (§ 1)
- Auto-connect runs the `connect` voltage check — author's call: a drop must
  not create a wire the Line tool would refuse. (§ 3)

## Corrections

## Summary

The risk is scoping (§ 3, step 2). Connectivity rules, the file format and
the drawing are unchanged.
