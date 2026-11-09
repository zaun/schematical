# plan-30-A: Loose ends — deleting a block keeps its wires

Last updated: 2026-11-08
Overall Effort: large (3h–1d)
Effort: medium (1h–2h)
Depends on: nothing

plan-30 changes what happens to wires when the thing they end on goes away.
Today, erasing a block erases every wire on its pins. After plan-30, the
wires stay where they were and their ends become **loose ends** (A). Dropping
a block so that one of its pins lands exactly on a loose end connects the
wire to that pin again (B). A wire with a loose end is drawn gray on screen,
and black like any other wire in an export (C).

A builds the loose end and makes every removal that today deletes wires keep
them instead. The outcome: erase a block that has wires, and the same wires,
with the same routes, are still in the document. Each end that was on the
block's pins is now on a loose end at that pin's old tip. Saving and opening
the file keeps them.

References:

- `docs/file-format.md` § Junctions (:179), § Compatibility (:421, the
  optional-field rule).
- `schematical/schematical/Canvas/SchematicModel.swift`: `WireEnd` :785,
  `Junction` :793, `junction(at:)` :2484, `attaches` :2519, `pruneJunctions`
  :2528, `setHeaderSize` :1425, `setPortWires` :2784, `PendingRemoval`
  :2951, `requestPortWires` :2982, `removePort` :2996, `removeBusLine` :3010,
  `erase(at:)` :3068.
- `schematical/schematical/Document/SchematicFile.swift`: `Junction` :182,
  encode :407, decode :604.

## Prerequisites

None on other plans. plan-30 stands alone.

| Must be true | Command | Status |
|---|---|---|
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p30-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-08 at 48645a4, re-run in worktree-P-30: `** BUILD SUCCEEDED **`) |

Everything below is written against a world where this holds.

> **NOTE: the Status column is a snapshot; the Command column is the truth.**
> Re-run the command before you start. If you stop, report the status of
> every prerequisite.

## 1. Goal

- Erasing a block with wires leaves those wires in `doc.wires` with their
  routes unchanged. Every end that was on one of its pins is now
  `.junction(id)`, where that junction has `isLoose == true` and sits at the
  pin's old tip. Wires that were on the **same** pin share **one** loose
  junction, so they stay connected to each other.
- The same holds for the four other removals that delete wires today:
  shrinking a header, deleting a bus port, deleting a bus line, and
  unticking port pins.
- A loose junction round-trips through save and open as `"loose": true`.

### Non-goals (explicit constraints)

- The file format version stays 4. `loose` is an optional key, written only
  when true. Files without it open exactly as before.
- Erasing a **wire** still deletes the wire (`erase(at:)` :3084).
- No auto-connect (B) and no gray drawing (C) in this letter.
- No undo. The app has no undo system (`planning/todo.md`).
- Simulation, VoltageCheck, PCB and BOM code is unchanged. A loose junction is
  an ordinary junction to all of them (§ 2, Verified properties).

## 2. Current State

- A wire end is a pin, a junction or a bus port pin (`WireEnd` :785). There is
  no free end: a wire can't exist without something at both ends.
- A junction (`Junction` :793) is a fixed grid point with an id. It has no
  other fields. `pruneJunctions()` (:2528) drops a junction once no wire ends
  on it, so a junction with one wire is kept.
- `junction(at:)` (:2484) returns an existing junction at a point first, so a
  new line drawn to a junction's point attaches to it.
- In the file (`SchematicFile.Junction` :182) a junction is `{id, x, y}`,
  decoded by the synthesized `Codable`, so unknown keys are ignored. An older
  build that opens a file with `"loose": true` shows a plain junction.

### Measured populations

| What | Count | Command |
|---|---|---|
| `wires.removeAll` sites in the model | 6 | `grep -n "wires.removeAll" schematical/schematical/Canvas/SchematicModel.swift` → :1437, :2795, :2999, :3014, :3072, :3084 |
| …of them, removals of wires on pins (changed here) | 5 | :1437 header shrink, :2795 untick, :2999 port, :3014 bus line, :3072 block |
| …of them, erasing one wire (kept) | 1 | :3084 |
| Removal prompts whose text says wires go | 3 | `grep -n "connected to" schematical/schematical/Canvas/SchematicModel.swift` → :2988, :3077, :3092 |
| Places a `Junction` is built | 3 | `grep -rn "Junction(id:\|Junction(point:" schematical/schematical` → SchematicFile :407, :606, SchematicModel :2501 |
| App tests that call `erase(at:)` | 2 | `grep -rn "erase(at" schematical/schematicalTests` → SelectionModelTests :278, SchematicNoteTests :86 |
| Harness scripts that check wires being removed | 2 | `grep -rln "erase(at\|setPortWires\|confirmPendingRemoval\|setHeaderSize\|connected to" scripts/harness/*.swift` → 9 files; reading each, only `bus-erase.swift` (:37–69) and `generic-header.swift` (:152–158) check removed wires or the prompt text. The other 7 erase bus lines with no wired ports, or only count wires. |

### Verified properties

- **Simulation needs no change.** `Simulation.swift:48–95` union-finds wire
  ends. A junction is a node like any other, so wires on one loose junction
  form one net with no pins. Read the union loop.
- **SelectionModelTests :267 still passes.** It checks only that the
  selection holds no stale items after an erase. Its comment ("Erasing the
  lower LED takes its wire") becomes wrong and is updated in Phase 2.
- **Pin tips are grid points.** `PlacedChip.tip(of:)` (:732) is the origin
  (snapped in `placeChip` :1365) plus layout offsets. The loose junction
  lands on the grid, so B can compare points with `==`.

## 3. Design Overview

A loose end is a **junction with `isLoose = true`**. One helper,
`loosen(_ ends:)`, turns wire ends into loose junctions. Every removal calls
it before it removes the pin.

Correctness risk is in `removeBusLine` (:3010). It also splits the bus into
pieces and remaps ports by list position. The loosening must happen first, on
the ports of the removed line only, while their pin tips still resolve.

Rejected alternatives:

- **A new `WireEnd.open(CGPoint)` case.** Two wires off one pin would get two
  separate open ends at the same point. They would no longer be one net
  unless the simulation also merged ends by coordinate, which
  `docs/file-format.md:17` rules out ("Connectivity never depends on two
  coordinates touching"). It would also need a new file end type, plus
  changes to the hit-testing, selection and group-drag code that junctions
  already have.
- **A plain junction with no flag.** B can't tell a deleted pin's spot from a
  real branch point, and C can't tell which wires to draw gray.

## 4. Detailed Design

### 4.1 Model

`Junction` (:793) gets `var isLoose: Bool`, default `false` in `init`.

```swift
/// Turns every wire end that `matches` into a loose junction at that end's
/// tip. Ends with the same tip share one junction. A junction already at
/// that point is reused and marked loose. Call it while the tips still
/// resolve, before the pins go away.
private func loosen(where matches: (WireEnd) -> Bool)
```

Steps: for each wire, and for each of its two ends that `matches`, get the
tip with `endTip`. Find or create the junction for that tip in a
`[CGPoint: UUID]` map built during the call (seeded from existing junctions
at that point, which get `isLoose = true`). Then set the end to
`.junction(id)`. If `endTip` returns nil, leave the end alone. The removal
then deletes the wire, as it does today.

### 4.2 The five removals

| Site | Change |
|---|---|
| `erase(at:)` block branch :3072 | `loosen { attaches($0, to: hit.id) }` **before** `chips.removeAll`; drop the `wires.removeAll`. |
| `setHeaderSize` :1437 | Before `chips[index] = old.resized(…)`: `loosen` the ends on pins with `ref.index >= pinCount`; drop the `wires.removeAll`. |
| `setPortWires` :2795 | `loosen { endsOn($0, port: portID, wireIDs: removing) }`; drop the `wires.removeAll`. |
| `removePort` :2999 | `loosen { endsOn($0, port: portID) }` before `ports.remove`; drop the `wires.removeAll`. |
| `removeBusLine` :3014 | In the loop, `loosen { endsOn($0, port: port.id) }` before anything is removed; drop the `wires.removeAll`. |

`pruneJunctions()` calls stay where they are. They are now no-ops on these
paths, but `pruneSelection` still runs through them.

### 4.3 Prompt wording

The prompts stay, because the port or bus line itself is still lost. The new
text, with `counted` as today:

- :2988 `"Remove \(n pins)? The \(m wires) on them stay, with loose ends."`
- :3077 `"Delete this port? The \(m wires) on its pins stay, with loose ends."`
- :3092 `"Delete this bus line and its \(n ports)? The \(m wires) on their pins stay, with loose ends."`

### 4.4 File

`SchematicFile.Junction` gets `var loose: Bool?`. Encode `true` when
`isLoose`, `nil` otherwise. Synthesized `Codable` leaves out a nil optional.
Decode with `isLoose = loose ?? false`.

## Compatibility / Format Impact

- New optional junction key `loose` (boolean, written only when true). The
  version stays 4, per the optional-field rule (`docs/file-format.md:421`).
- Older builds open such files and show the loose junction as a plain one.
- No other key changes.

## Phases

> **NOTE: keep the checkboxes current.** Tick `- [x]` in the same commit as
> the work, `- [~]` for partial work, and strike a moot task rather than
> deleting it. Fill `Commit:` when a phase lands. An unticked box means NOT
> DONE.

### Phase 1 — Loose junction in the model and the file

Adds the flag and its round trip. Nothing sets it yet, so nothing in the app
changes.

- [x] `Canvas/SchematicModel.swift`: `Junction.isLoose` (§ 4.1).
- [x] `Document/SchematicFile.swift`: `Junction.loose`, encode :407, decode
      :606 (§ 4.4).
- [x] `docs/file-format.md` § Junctions: document `loose` ("true: the spot
      where a deleted pin was; wires on it have a loose end. Written only
      when true.") and change "The app removes unused junctions when wires
      are erased" to also say a junction stays while any wire ends on it.
- [x] Tests: new `schematicalTests/LooseEndTests.swift`,
      `testLooseJunctionRoundTrips` (a document with one loose and one plain
      junction → `SchematicFile` → JSON → back; the flags match, and the
      plain one's JSON has no `loose` key). Ran the Check: `Executed 1 test,
      with 0 failures`, `** TEST SUCCEEDED **`.

Acceptance: a loose junction survives save and open, and a plain one writes no `loose` key.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/LooseEndTests CODE_SIGNING_ALLOWED=NO` → all pass (est. 4 min).
Commit: bcb1c92

### Phase 2 — Removals keep their wires

- [x] `Canvas/SchematicModel.swift`: `loosen(where:)` (§ 4.1).
- [x] The five sites in § 4.2.
- [x] The three prompt texts in § 4.3.
- [x] `Canvas/SchematicCanvasView.swift:29` header comment: "Eraser: click
      a block, wire or bus line to remove it. A block's wires stay, with
      loose ends."
- [x] `schematicalTests/SelectionModelTests.swift:275` comment: "Erasing the
      lower LED leaves its wire with a loose end."
- [x] `scripts/harness/bus-erase.swift`: it checks the old behavior. Change
      the three prompt strings (:37, :53, :66) to § 4.3's. The "wire gone"
      checks (:44, :58, :69) become "wire count unchanged, and the wire
      that was on the pin now ends on a loose junction".
- [x] `scripts/harness/generic-header.swift` :155–158: the pin-8 wire is
      now kept with a loose end. Expect 3 wires; the pin-8 wire ends on a
      new loose junction at the old pin-8 tip, and its other junction
      (`junctions[2]`) stays, so there are 4 junctions.
- [x] Tests in `LooseEndTests.swift`:
  - `testErasingABlockKeepsItsWires`: two parts joined by one wire; erase
    one. The wire count is unchanged, the path is unchanged, and the erased
    end is a loose junction at the old tip.
  - `testWiresOnOnePinShareOneLooseEnd`: two wires on one pin; erase that
    part. There is exactly one new loose junction, and both wires end on it.
  - `testShrinkingAHeaderLoosensTheDroppedPins`: wire on pin 8 of a 1×8
    header, shrink to 1×4. The wire stays, with a loose end.
  - `testDeletingAPortKeepsItsWires`: port with a wire; `erase(at:)` the
    port, then `confirmPendingRemoval()`. The wire stays, with a loose end.
  - `testDeletingABusLineKeepsItsPortsWires`: same, for a bus line.
  - `testUntickingAPortPinKeepsItsWire`: `setPortWires` without the pin.
    The wire stays, with a loose end.
  - `testErasingAWireStillDeletesIt`: the :3084 branch is unchanged.
- [x] Observed: the unit Check → `Executed 33 tests, with 0 failures`,
      `** TEST SUCCEEDED **`. Harness (built with
      `APP=/tmp/p30-dd/.../schematical.app`, out dir `/tmp/p30-harness`):
      `bus-erase` 29 ok lines, 0 FAIL, `ALL OK`; `generic-header` 15 ok
      lines, 0 FAIL.

Acceptance: each removal leaves its wires with loose ends, and erasing a wire still deletes it.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test -only-testing:schematicalTests/LooseEndTests -only-testing:schematicalTests/SelectionModelTests -only-testing:schematicalTests/BusJoinTests CODE_SIGNING_ALLOWED=NO` → all pass (est. 4 min).
  Check: `scripts/harness/build.sh scripts/harness/bus-erase.swift && /tmp/schematical-harness/H.app/Contents/MacOS/harness`, then the same for `generic-header.swift` → no `FAIL` lines (est. 4 min).
Commit: b303c07

## Validation Plan

- Tests: `LooseEndTests` (new), plus `SelectionModelTests` and `BusJoinTests`,
  which touch the changed removals.
- Doc sync: `docs/file-format.md` § Junctions.
- The landing tier runs once, at the end of plan-30-C.

## Decisions

- Deleting a block keeps its wires, as loose ends — decided by Justin Zaun,
  2026-11-08. (§ 1)
- The same rule applies to header shrink, port delete, bus line delete and
  port pin untick — decided by Justin Zaun, 2026-11-08 ("agreed with
  everything"). (§ 4.2)
- plan-30 has no prerequisite plans — decided by Justin Zaun, 2026-11-08.
- A loose end is a flagged junction, not a new end type — author's call,
  rejected alternatives in § 3.
- The port and bus line prompts stay, reworded — author's call: the port or
  line itself is still lost. (§ 4.3)

## Corrections

- § 4.2 `setHeaderSize`: the old code called `pruneJunctions()` only when a
  wire was removed. With nothing removed any more, that call is dropped
  rather than kept as a no-op (no selection can go stale: no item is
  removed). The `loosen` call runs before `chips[index] = old.resized(…)`,
  while the dropped pins' tips still resolve.
- The plan's harness command writes to the shared `/tmp/schematical-harness`
  and builds its own app copy; runs here used `APP=` the plan's
  `/tmp/p30-dd` build and out dir `/tmp/p30-harness` so other sessions'
  harness runs can't collide. Same scripts, same checks.

## Summary

The risk is ordering inside `removeBusLine`: loosen before the line, its
ports or the bus split change anything. The simulation, PCB and file
version are untouched.
