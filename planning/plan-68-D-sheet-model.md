# plan-68-D: App — the schematic sheet model

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-C

`SchematicDocument` owns everything drawn on the Schematic page directly, as
stored arrays of the class, with every edit's logic written inline in a
`recordEdit { … }` closure. This letter moves the schematic page's content —
wires and junctions, buses and their ports, net labels, text notes — into a
**schematic sheet model**: value types whose `mutating` methods hold the edit
logic and can be tested without a document, owned by one observable
`SchematicSheet` object. The document keeps its public API: every
`document.addWire(…)`-style method stays, now a thin recorded call into the
sheet. That is the seam multi-sheet will need (a document with several
`SchematicSheet`s); this plan does not add multi-sheet.

Prerequisites: plan-68-A's table, and plan-68-C complete
(`schematical/schematical/Canvas/SchematicModel.swift` has no row in
`scripts/coverage-ratchet.tsv`: every line of the document is covered before
it is taken apart).

References:

- `schematical/schematical/Canvas/SchematicModel.swift` — `SchematicDocument`'s
  stored state (§ 2) and the marks Wires, Buses, Bus ports, Removing ports and
  bus lines, Net labels (Schematic), Text blocks (Schematic), Undo.
- `Canvas/BusModel.swift` (`Bus`), `Canvas/NetLabel.swift`,
  `Canvas/SchematicNote.swift`, `Canvas/Wire.swift` (plan-68-B),
  `Canvas/UndoHistory.swift` (`DocumentState`).
- `Document/SchematicFile+Document.swift` (plan-68-V: `apply(to:)`, which
  calls `replaceContents`).

## 1. Goal

- `SchematicSheet` (an `@Observable` final class) stores `wires`,
  `junctions`, `buses`, `netLabels` and `notes`, each `private(set)`, exactly
  as the document stores them today (same `didSet` revision bumps); the
  document holds one `sheet` and no longer stores those arrays.
- The edit logic of the six marks above lives in `mutating` methods of those
  four value types, in files of their own, each at or under 800 code lines.
- Each value type has a test class that exercises it **without** a
  `SchematicDocument` (`WiringTests`, `BusNetworkTests`, `NetLabelsTests`,
  `SheetNotesTests`), and `COV --app` shows their files at 100%.
- Every existing test passes unchanged; `document.wires`, `.junctions`,
  `.buses`, `.netLabels`, `.notes` still read the same values.

### Non-goals

- `SchematicDocument`'s public API (every method and property used outside
  `SchematicModel.swift`, by views, automation, routing, exports and tests)
  is unchanged. `DocumentState` and the file format are unchanged.
- Observation is not coarsened: each array stays its own observed stored
  property, and an edit writes back only the arrays it changed (§ 3.1-2), so
  a view reading `notes` is not invalidated by a wire edit — as today (§ 3.4
  test).
- No undo behavior changes: one `recordEdit` call is still one undo step with
  the same name.

## 2. Current State

- Stored on the document (`private(set)`, `SchematicModel.swift` Document
  mark): `wires` (`didSet` bumps `copperRevision`), `junctions` (no bump),
  `buses` (bump), `netLabels` (bump), `notes` (no bump); the route cache
  `keptPaths`; `editingNoteID`, `selectedBusID`, `selectedPortID` (editor
  state).
- Every edit is `func name(…) { recordEdit(page, "Name") { <mutate stored
  arrays> } }` — 179 `recordEdit(` calls in the file (`grep -c`), 29 more
  outside it that group several document calls into one step (in
  `NetList.swift`, `PCBSidebarView.swift`, `AutomationBoard.swift`,
  `AutomationSchematic.swift`, `RoutingApply.swift`; none of them touches a
  stored array: `grep -rnE 'doc(ument)?\.(chips|wires|traces|vias) *(=|\.append|\.remove)'
  schematical/schematical` → no output).
- Undo is a snapshot: `recordEdit` takes `state` (a `DocumentState` of every
  saved array) before and after, and `setState` writes each array back when it
  differs.
- Code lines of the marks this letter moves (`sections.js`): Wires 338, Buses
  260, Bus ports 163, Removing ports and bus lines 126, Net labels 75, Text
  blocks 39.

## 3. Design

### 3.1 The pattern (letters D, E and F all use it)

1. **Value types hold the logic.** A `struct` with plain `var` fields and
   `mutating` methods; it may be extended in several files (a struct's
   `var`s are settable from any file), so a type can grow past one file
   without loosening anything. It reads what it needs from outside through
   parameters (a pin-tip lookup, the parts), never through a back-reference.
2. **An observable owner stores the arrays.** `@Observable final class`
   with each array a separate `private(set)` stored property (as today, so
   SwiftUI observes each one separately), and nothing outside the owner's
   file can mutate them — except through `update`, which builds the value
   type from the arrays it covers, runs the edit on it, and writes back only
   the arrays that changed:
   ```swift
   func updateWiring<T>(_ scope: EditScope, _ body: (inout Wiring) throws -> T) rethrows -> T {
       var value = Wiring(wires: wires, junctions: junctions)
       defer {
           if value.wires != wires { wires = value.wires }
           if value.junctions != junctions { junctions = value.junctions }
       }
       return try body(&value)
   }
   ```
   (the same compare-before-assign `setState` does today).
3. **`EditScope` proves a change is recorded.** `struct EditScope` has a
   `fileprivate init()` in the file that holds `recordEdit`, so only the
   document's undo code can make one. The document gains
   ```swift
   func edit<T>(_ page: EditorPage, _ name: String, coalescing: String? = nil,
                _ body: (EditScope) throws -> T) rethrows -> T
   ```
   which is `recordEdit` handing its body a scope. Every mutation of saved
   content therefore happens inside a recorded edit — checked by the
   compiler, not by convention. `recordEdit(page, name) { … }` stays as it is
   for the external callers that group several document calls.
4. **The document's methods become thin.** `func addWire(…) { edit(.schematic,
   "Add Wire") { scope in sheet.updateWiring(scope) { $0.add(…, tips:
   pinTips) } } }`. Their names, parameters and results do not change.

### 3.2 The sheet's value types

| Type (file in `schematical/schematical/Model/Sheet/`) | Holds | From the marks | Code |
|---|---|---|---|
| `Wiring` (`Wiring.swift`, `Wiring+Edits.swift`) | `wires`, `junctions`; `path(for:)` routing via `OrthogonalRoute` given a `PinTips` lookup | Wires | 338 |
| `BusNetwork` (`BusNetwork.swift`, `BusNetwork+Ports.swift`) | `buses` with their ports | Buses; Bus ports; Removing ports and bus lines | 549 |
| `NetLabels` (`NetLabels.swift`) | `labels` | Net labels | 75 |
| `SheetNotes` (`SheetNotes.swift`) | `notes` | Text blocks | 39 |
| `PinTips` (`PinTips.swift`) | a value the document builds from `chips` + `chipIndex`: where each `PinRef`/`BusPinRef`/label end is | the tip lookups in Wires and Pins | — |

`SchematicSheet` (`Model/Sheet/SchematicSheet.swift`): the five arrays
`private(set)`, the four `update…` functions (`updateWiring`, `updateBuses`,
`updateLabels`, `updateNotes`), `copperRevision` (bumped in the `didSet` of
`wires`, `buses` and `netLabels`, exactly the arrays that bump the
document's counter today; `junctions` and `notes` do not), the `keptPaths`
cache (`@ObservationIgnored`), and `restore(_ state:, _ scope:)` for undo and
file load.

The document's `copperRevision` becomes `parts revision &+ sheet.copperRevision
&+ <board's>` (the board part arrives in plan-68-E; until then the document's
own counter for its remaining arrays): every component counter only grows,
so the sum changes exactly when one of them does.

Edits that span the sheet and the parts — erase, delete selection, group
drags, paste, removing a part with its wires — stay on the document (plan-68-F
files them) and call `sheet.update…` and the parts' update in one `edit`.

### 3.3 What the moved logic may not do

A `mutating` method on a sheet value reads only its own fields and its
parameters. Where today's inline code reads `chips` (a wire's end on a pin),
the method takes `PinTips`. Where it changed the selection (`selectedBusID =
nil` after deleting a bus), it returns what it removed, and the document
updates its selection from the result.

### 3.4 Tests

- `WiringTests`, `BusNetworkTests`, `NetLabelsTests`, `SheetNotesTests`
  (new): each `mutating` method on hand-built values, no document.
- `SheetObservationTests` (new): with `withObservationTracking`, a reader of
  `document.notes` is not invalidated by `addWire`, a reader of
  `document.buses` not by `addNote`, a reader of `document.wires` not by
  `addNetLabel` — the granularity today's separate stored properties give.
- The existing document tests (`SchematicUndoTests`, `BusJoinTests`,
  `NetLabelTests`, `WireRefitTests`, `WireSettleTests`, `LooseEndTests`,
  `ConnectPathTests`, `SchematicClipboardTests`, `SchematicNoteTests` …) pass
  unchanged: they are the proof the API did not move.

### Rejected alternatives

- **Extension files of `SchematicDocument` with internal setters** — the
  owner chose real model types.
- **The value types as the owner's stored properties** (one observed
  `wiring`, or one `SchematicSheet` struct) — coarser observation: a view
  reading any of the merged arrays would redraw on an edit to another (the
  owner's todo list already has "the schematic view can get slow when a lot
  of items are selected"). Compare-and-write-back keeps today's granularity
  for the cost of one array comparison per edit, which `setState` already
  pays.
- **Owners with a weak back-reference to the document** — the values could
  no longer be tested alone, and the dependency would be hidden.
- **Leaving mutation internal by convention** — nothing would stop an
  unrecorded edit; `EditScope` makes it a compile error.

### Where the risk is

Undo: `setState` today writes each stored array; it now calls
`sheet.restore`. A missed field would make undo lose that content. The
existing undo tests cover each kind of edit (plan-68-C brought the document
to 100%), and `SchematicUndoTests` round-trips every edit kind.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` with what remains; moot tasks are
> struck through with evidence, never deleted; fill `Commit:` when a phase
> lands. **An unticked box means NOT DONE.**

### Phase 1 — `EditScope`, `edit`, the sheet owner

- [x] `EditScope` and `edit(_:_:coalescing:_:)` beside `recordEdit` (§ 3.1).
- [x] `Model/Sheet/SchematicSheet.swift` with the four values as thin
  structs holding the arrays (no logic yet); the document's stored `wires`,
  `junctions`, `buses`, `netLabels`, `notes` become read-only computed
  forwards to `sheet`; every write in the document goes through
  `sheet.update…(scope)`; `setState`/`replaceContents` through
  `sheet.restore`. (Landed with Phase 2's logic in one commit: Corrections.)
- [x] `SheetObservationTests`.

Acceptance: the sheet owns the content and nothing else changed.
  Check: `XB -only-testing:schematicalTests/SchematicUndoTests
  -only-testing:schematicalTests/DocumentUndoTests
  -only-testing:schematicalTests/SheetObservationTests
  -only-testing:schematicalTests/SchematicFileV5Tests` → all pass (est. 5 min).
  Observed: SchematicUndoTests 50, DocumentUndoTests, SchematicFileV5Tests 8,
  SheetObservationTests 2 → `TEST SUCCEEDED`, 0 failures.
Commit: COMMIT_D

### Phase 2 — the logic into the values

- [x] Move the Wires, Buses, Bus ports, Removing ports, Net labels and Text
  blocks logic into the `mutating` methods of § 3.2 (`PinTips` added); the
  document methods become § 3.1-4 one-liners.
- [x] `WiringTests`, `BusNetworkTests`, `NetLabelsTests`, `SheetNotesTests`.
- [x] Ratchets: `SchematicModel.swift`'s size row lowered (4312 → 3813,
  `node scripts/source-size.js --check` → exit 0); coverage rows for the new
  files at 0 (no row: `Model/Sheet/` is fully covered).

Acceptance: the logic is in the values, tested alone, and the document's
behavior is unchanged.
  Check: the full `XB` (the blast radius is every schematic edit) → no
  failure beyond the standing ones recorded in
  `planning/plan-68-data/app-coverage.txt`; `COV --app --only
  schematical/schematical/Model/Sheet/` → no uncovered line (est. the full
  Debug suite, as measured in `app-coverage.txt`; nothing smaller covers every
  caller of the moved methods).
  Observed: the full suite under `node scripts/coverage.js --app` → 9 failed cases: the 8 standing ones and `PartKindKeysTests.testFormKeysCoverTheBackendKeys`, a plan-68-J regression (it read the files J split by their old names; fixed here: Corrections), which then passed (`-only-testing` PartKindKeysTests, SheetObservationTests → 0 failures). `Model/Sheet/` → no uncovered line (`--only schematical/schematical/Model/Sheet/` with the four value classes → `758 lines, 0 uncovered`). SchematicModel.swift had 6 uncovered (an unused `wire(at:)`, now deleted, and three port refusals on nothing, now tested in `SheetObservationTests.testPortEditsOnNothing`); four ratchet rows went down and were lowered.
Commit: COMMIT_D

## Validation Plan

- Tests: four value-type classes, `SheetObservationTests`, the whole
  existing suite.
- Coverage: `COV --app --check` → exit 0; `Model/Sheet/` at 100%.
- Final gate (once, after Phase 2): the full `XB`; `SZ`; `COV --app --check`.

## Decisions

- `SchematicDocument` is split into real model types (owner, plan-68-A).
- (author) Values with `mutating` logic, observable owners with `private(set)`
  storage, and a compile-time `EditScope` (§ 3.1).

## Corrections

- **Phases 1 and 2 landed as one commit.** Phase 1 alone would have threaded
  an `EditScope` through every private helper of the moved marks only for
  Phase 2 to replace them; the values were written with their logic at once.
  Both phases' checks ran on the result.
- **The route cache moved to the sheet as a shared class** (`WireRoutes`,
  in `PinTips.swift`): every `Wiring` the sheet hands out holds the same
  cache, so routes are still worked out once per change of a wire or its
  tips. It needs `nonisolated deinit {}` like the document: with this
  module's default main-actor isolation an isolated deinit freed memory it
  did not own (`malloc: pointer being freed was not allocated` in every
  `WiringTests` case before the fix).
- **`PinTips` is three closures**, not a table the document builds per
  edit: the document's closures read its `chips` (through `chipIndex`) and
  the sheet's committed buses and labels, so a tip costs what it did; tests
  build one from dictionaries.
- **A sheet value never calls back into the document mid-update.** An update
  writes its arrays back when its closure ends, so a nested update of the
  same arrays would be overwritten. Edits that touched wires and buses in
  one go (removing a bus line, absorbing a joined bus, ticking port pins)
  became sequences of updates in the order the old code changed things.
- **`adoptLooseEnds`' voltage check** reads the problems the drawing has
  before each candidate from that candidate's own `before` wires when an
  earlier adoption in the same call changed them (the old code re-read the
  document, whose wires had changed); otherwise the kept issues.
- **Refusals came back as values**: `BusNetwork.addPort` returns the port or
  a `PortRefusal`, `setPortSide` a `PortSideResult`, `moveWire` its message
  and the ports to re-fit, `removeLine` the wire-id remaps; the document sets
  `editorMessage` and the selection from them (§ 3.3).
- **`PartKindKeysTests` was broken by plan-68-J** (it lists the backend files
  that hold each kind's `KEYS[]` arrays, and J split `converter_load.c` and
  others into pieces with model-prefixed shared functions). It now reads a
  listed file's pieces too (files `<stem>_…c` including `<stem>_int.h`) and
  matches a skipped loader by its prefixed name (`cvl_load_clock`).
- **`nodeHit`'s loop** was rewritten with `first(where:)`: the per-line
  counter left the closing lines of the `for … where` loop uncovered though
  the tests ran them.


## Summary

The first real cut through the document. Risk sits in undo restore and in
observation granularity; both have dedicated tests, and the whole suite is
the gate because every schematic edit moves.
