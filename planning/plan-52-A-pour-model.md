# plan-52-A: Copper Pour and Keep-out Model, Undo and Document API

Last updated: 2026-11-21
Overall Effort: x-large (1d–3d)
Effort: medium (1h–2h)
Depends on: nothing

The PCB has copper **Planes** (a whole inner layer on one net, `LayerStyle.type == .plane`) and nothing else that floods copper. It
cannot put a GND pour on Top or Bottom, round the traces and pads there, in only part of the board. plan-52 (A–H) adds the
**copper pour**: a rectangle on one trace layer (Top, Bottom or an inner Wire layer) carrying one net, flood filled with copper
that keeps its clearance from every other net's pads, vias and traces and from every **keep-out**, joins the pins of its net
through thermal reliefs, and is drawn, saved, exported and checked. A **keep-out** is a rectangle on chosen layers that pours stay
out of (it blocks pours only). This sub-plan lays the data and the edit API underneath all of it: the two value types, their place
in the document, undo, the selection item and the document functions that change them. No screen draws a pour yet (plan-52-E).

The single outcome: a document holds pours and keep-outs; every change is one named undo step and undoes and redoes exactly;
the new selection items are handled by every code path that switches over `BoardItem`; a layer that holds a pour cannot become a
Plane or be dropped by a layer-count change.

The whole plan-52, in letter order (each is one sitting):

| Plan | Delivers |
|---|---|
| A (this) | `BoardPour`, `BoardKeepout`, document state, undo, `BoardItem` cases, edit API |
| B | File: `board.pours`, `board.keepouts`, validation, `docs/file-format.md` |
| C | `PourFill`: the pure fill geometry (clearances, thermal reliefs, keep-outs, islands) |
| D | Connectivity: a pour joins its net's nodes; kept fills; the ratsnest and router see them |
| E | The PCB page draws pours and keep-outs; Pour and Keep-out tools; select, move, delete |
| F | Resize handles, the sidebar panels, Fit to Board, Board Preview |
| G | Gerber, KiCad (zone and rule area) and EasyEDA Pro output; `docs/export.md` |
| H | Design rule check, automation tools, `docs/agent-control.md`, landing |

References:

- `planning/todo.md` § PCB (the item this plan finishes), `planning/plan-52-data/README.md` (the probes this design rests on)
- `docs/file-format.md` § Board (`layerStyles`, line 162) and § Holes (line 229), `docs/export.md`, `docs/routing.md`
- `schematical/schematical/Canvas/BoardHole.swift` (the value type this mirrors), `Canvas/BoardSettings.swift` (`PlaneNet` at
  `:205`, `LayerStyle`, `traceLayers` at `:396`), `Canvas/SchematicModel.swift` (`holes` at `:1753`, `// MARK: Holes (PCB)` at
  `:4062`), `Canvas/UndoHistory.swift`, `Canvas/Selection.swift` (`BoardItem` at `:21`)
- Line numbers in this plan are as of 2026-11-21 and drift as other sessions edit these files; the symbol named beside each is the reference.

## Prerequisites

These are a precondition on the whole of plan-52, stated once here.

| Must be true | Command | Status |
|---|---|---|
| plan number 52 is this plan's alone | `ls planning planning/completed \| grep '^plan-52' \| grep -v -E '^plan-52-([A-H]-\|data)'` → no output | MET (2026-11-21) |
| plan-50 is complete (its `board.stackup` is already in the file; plan-52 adds keys beside it) | `ls planning/completed \| grep -c '^plan-50-[A-F]'` → 6 | MET (2026-11-21: 6) |
| Xcode builds and tests the app | `xcodebuild -version` → `Xcode 26.3` or later; `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD build CODE_SIGNING_ALLOWED=NO` → `** BUILD SUCCEEDED **` | MET (2026-11-21: `Xcode 26.3`; the build succeeded with the other session's uncommitted edits in the tree) |
| KiCad 10's CLI and python are installed (plan G checks against them) | `ls ~/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli` → the path | MET (2026-11-21) |
| the files plan-52 edits have no uncommitted changes of another session | `git status --porcelain -- schematical/schematical/Canvas/SchematicModel.swift schematical/schematical/Document/SchematicFile.swift schematical/schematical/Automation/AutomationRead.swift docs/file-format.md docs/export.md docs/agent-control.md` → no output | NOT MET (2026-11-21, re-run when `/follow-plan 52` was invoked: all six still show ` M`; they are another session's work, to be committed or finished before this plan edits the same files) |

Everything below is written against the world where these hold. `$DD` in the checks is a scratch derived-data folder (any empty
directory).

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again before you decide to stop. Never act on a status
> you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites** — not only the one that blocked you.

## 1. Goal

- `BoardPour` and `BoardKeepout` exist as value types with the ranges and defaults in § 4.
- `SchematicDocument` keeps `pours` and `keepouts` (drawing order); changing either bumps `copperRevision`.
- `addPour`, `setPourRect`, `setPourLayer`, `setPourNet`, `setPourClearance`, `setPourConnection`, `setPourThermalGap`,
  `setPourSpokeWidth`, `setPourMinWidth`, `deletePour` and the keep-out equivalents each make one named undo step; undo and redo
  restore the arrays exactly.
- `BoardItem` has `.pour(UUID)` and `.keepout(UUID)`; select, lock, group-move, delete, the selection list and the automation item
  names handle them.
- `canSetLayerType(_, .plane)` refuses a layer with a pour; `canSetLayerCount` refuses a count that drops a layer with a pour.

### Non-goals (explicit constraints)

- No file format (plan-52-B), no fill geometry (C), no connectivity change (D), no drawing, tool or sidebar (E, F). Before plan-52-E
  nothing in the UI can make a pour; only tests call the API.
- No change to Planes: `LayerStyle`, `planeNets`, `planeClearances` and their drawing, export and routing stay as they are.
- No change to existing files' bytes: a document with no pours and no keep-outs saves and behaves exactly as before.

## 2. Current State

- `BoardHole` (`Canvas/BoardHole.swift:67`) is the precedent: a `nonisolated struct … Identifiable, Hashable, Sendable` with
  `id`, geometry in mm, `net: PlaneNet?`, `locked`, range constants and a `problem(…)` validator.
- The document keeps `private(set) var holes: [BoardHole] = [] { didSet { copperRevision &+= 1 } }` (`SchematicModel.swift:1753`);
  `traces` (`:1748`) and `vias` (`:1750`) are the same. `copperRevision` (`:1345`) invalidates `keptConnectivity` and
  `keptRatsnest` (`Ratsnest.swift:56`).
- Undo: `DocumentState` (`UndoHistory.swift:10`), `UndoKey` (`:30`), `UndoValue` (`:46`), `UndoDiff.changes` (`:100`, hole at
  `:129`) and `place(…)` (`:229`). `recordEdit(.pcb, name) { … }` (`SchematicModel.swift:1877`) records one step.
- Holes' API is `// MARK: Holes (PCB)` (`SchematicModel.swift:4062`): `addHole`, `moveHole`, `setHole…`, `deleteHole`; lock in
  `isLocked`/`setLocked` (`:4262`); group drags in `boardGroupBase` (`:4286`) / `moveBoardGroup` (`:4311`); delete in
  `deleteBoardSelection` (`:3553`).
- Layers: `canSetLayerType` (`:3017`) and `canSetLayerCount` (under `// MARK: Board layers`) refuse a change that would orphan traces.
  `board.traceLayers` (`BoardSettings.swift:396`) is every board layer that is not a Plane.

### Measured populations

| What | Count | Command |
|---|---|---|
| places that mention a `.hole` case (the upper bound of the `BoardItem` switches to extend) | 26 matches in 8 files | `grep -rn "case let .hole\|case .hole\|\.hole(" schematical/schematical \| wc -l` → 26; `grep -rl …` → SchematicModel, PCBCanvasView, UndoHistory, SelectionList, DesignRuleCheck, PCBSidebarView, AutomationRead, AutomationSchematic |
| hole lines in `UndoHistory.swift` | 9 | `grep -c -i hole schematical/schematical/Canvas/UndoHistory.swift` → 9 |
| `holes` references in `SchematicModel.swift` | 45 | `grep -c holes schematical/schematical/Canvas/SchematicModel.swift` → 45 |
| test files today | 154 | `ls schematical/schematicalTests/*.swift \| wc -l` → 154 |

### Verified properties

- New source and test files need no project-file edit: `grep -c PBXFileSystemSynchronizedRootGroup schematical/schematical.xcodeproj/project.pbxproj` → 5, and `grep -c HoleTests …project.pbxproj` → 0 although `HoleTests.swift` exists.
- The check command form works on this tree: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/HoleTests -only-testing:schematicalTests/PlaneNetTests CODE_SIGNING_ALLOWED=NO` → `Executed 13 tests, with 0 failures`, 17 s on a warm `$DD` (2026-11-21); a cold `$DD` adds the build.
- `CopperLayer` is `Hashable` (it is a `[CopperLayer: Int]` key in `BoardConnectivity.planeNets`, `Ratsnest.swift:568`), so a `Set<CopperLayer>` works.

## 3. Design Overview (the whole plan-52)

**Terms.** A **Pour** is a rectangle, a layer and a net. A **Keep-out** is a rectangle and a set of layers (none set = every
copper layer). A **Plane** stays the whole-layer inner copper it is today.

**The fill (plan-52-C).** For a pour, on its layer: the region is its rectangle, less the board outline pulled back by the DRC's
copper-to-edge. Cut out of it: every pad, via and trace on that layer not on the pour's net, grown by the pour's clearance
(a non-plated hole by the copper-to-edge); every keep-out on the layer (no growth); the rectangle of an earlier pour with another
net on the layer, grown by the clearance; and, for each pad of the pour's net, a thermal ring (the pad grown by the thermal gap)
less four spokes. What is left is split into pieces; a piece that no pad, via or trace of the net touches (an island) or whose
bounding box is narrower than the minimum width is dropped.

**Connectivity (plan-52-D).** A pour joins every node of its net that touches a kept piece, so the PCB draws no unrouted line
between them and the router sees them as one component.

**Risk.** Correctness risk concentrates in C (the fill) and D (the connectivity join: it changes `root`, which the DRC, ratsnest
and router all read). Both are separate letters, behind tests, after the data (A, B) and before any screen (E, F).

**What was probed before writing** (`planning/plan-52-data/README.md`): `CGPath` booleans give no mismatch in 200 000 sample
points; the whole fill takes 24 / 110 / 275 ms for 300 / 700 / 1500 other-net obstacles; a width clean-up by erosion takes
seconds and is not used; KiCad 10 loads the zone and rule-area syntax plan-52-G writes. There are no unproven premises.

**Byte-identity is not this plan's gate**; behavior changes by design. Files with no pour or keep-out must save byte-identical to
before (checked in B), and Planes must draw and export as before (checked in G).

Rejected alternatives:

- *A pour as a polygon.* The owner chose rectangles (§ Decisions): a polygon needs vertex editing and an outline tool; a rectangle
  covers a GND pour over part of a board, and the keep-out cuts the rest.
- *Pours as a layer type.* A Plane owns its whole layer and has no traces on it; a pour shares its layer with traces, pads and
  other pours, so it is an object, not a layer property.
- *A refill command (KiCad's B key).* The fill is fast enough (measured) to run on every change.

## 4. Detailed Design — the value types

`Canvas/BoardPour.swift` (new):

```swift
nonisolated enum PourConnection: String, CaseIterable, Hashable, Sendable { case thermal, solid }  // raw = file value

nonisolated struct BoardPour: Identifiable, Hashable, Sendable {
    let id: UUID
    var rect: CGRect                    // mm; positive size; each side at least minimumSide
    var layer: CopperLayer              // a trace layer of the board
    var net: PlaneNet? = nil            // nil: carries no net, so it fills nothing
    var clearance: Double? = nil        // mm, LayerStyle.clearanceRange (0.05…10); nil = the DRC's min clearance
    var connection: PourConnection = .thermal
    var thermalGap: Double = 0.5        // mm, gapRange 0.1…5
    var spokeWidth: Double = 0.5        // mm, spokeRange 0.1…5
    var minWidth: Double = 0.2          // mm, minWidthRange 0.05…2
    var locked = false
    static let minimumSide = 0.5
    func clearance(rules: DesignRules) -> Double { clearance ?? rules.minClearance }
}

nonisolated struct BoardKeepout: Identifiable, Hashable, Sendable {
    let id: UUID
    var rect: CGRect
    var layers: Set<CopperLayer>? = nil // nil = every copper layer of the board
    var locked = false
    func applies(to layer: CopperLayer) -> Bool { layers?.contains(layer) ?? true }
}
```

Both rects are kept normalized (positive width and height) and moved and sized on the board's trace grid
(`BoardGrid.snap(_, spacing: board.traceGrid)`), like a hole's centre. A new pour takes the connection, gap, spoke, minimum width
and clearance of the last pour added (as `addHole` takes the last hole's settings), and its net: the last pour's, else ground
when `planeNetChoices()` offers it, else none.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work; `- [~]` for partial with what
> remains; mark a moot task `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands; add tasks you discover.
> An unticked box means NOT DONE.

### Phase 1 — The value types

- [ ] Add `Canvas/BoardPour.swift` with `PourConnection`, `BoardPour`, `BoardKeepout` exactly as § 4, plus static ranges, `normalized(_ rect:)`
      (positive size, each side at least `minimumSide`), and `static func problem(rect:…)` returning a sentence for a value outside
      its range (the file's validator uses it in plan-52-B).
- [ ] Tests: new `schematicalTests/PourModelTests.swift`: defaults, ranges, `normalized` of a reversed and a too-small rect,
      `applies(to:)` with nil and a set.

Acceptance: the types compile with the defaults and ranges of § 4 and the tests assert each.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/PourModelTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

### Phase 2 — Document state, undo and the selection item

- [ ] `SchematicModel.swift`: add `private(set) var pours: [BoardPour] = [] { didSet { copperRevision &+= 1 } }` and the same for
      `keepouts` beside `holes` (`:1753`); add both to the designated init (`:1787`), `state` (`:1846`) and `setState` (`:1863`).
- [ ] `UndoHistory.swift`: `DocumentState.pours`/`keepouts`; `UndoKey.pour(UUID)`, `.keepout(UUID)` (pages `[.pcb]`);
      `UndoValue.pour(BoardPour, index:)`, `.keepout(BoardKeepout, index:)`; one `diff(…)` line each in `UndoDiff.changes` and one
      `place(…)` line each in `apply`, as for holes.
- [ ] `Selection.swift`: `BoardItem.pour(UUID)`, `.keepout(UUID)`; `BoardGroupBase` gets `pours: [UUID: CGRect]` and
      `keepouts: [UUID: CGRect]` (rects at drag start) and counts them.
- [ ] Make every switch the compiler flags handle the two cases (the measured grep finds 8 files with `.hole` cases; some are not `BoardItem` switches): `isLocked`/`setLocked`, `boardGroupBase`,
      `moveBoardGroup` (each rect moves by the snapped delta; a locked one stays), `deleteBoardSelection`, `pruneSelection`,
      `SelectionList.swift` rows (titles "Pour" with detail "GND on Top" style text from the net and layer, and "Keep-out"),
      `AutomationRead.swift` and `AutomationSchematic.swift` item names `pour:ID` / `keepout:ID` (`:813`, `:851`), `DesignRuleCheck.swift`
      (no issue yet), and `PCBCanvasView.swift` (the selection ring draws the rect). `PCBSidebarView.swift` needs no case: its `Content` is its
      own enum, and a selected pour or keep-out falls into `.selection`, the Selection list, until plan-52-F (nothing can select one before plan-52-E).
- [ ] Tests: in `PourModelTests`: a pour added then undone and redone leaves `pours` equal each time; a keep-out likewise; a step that
      changes a pour and a hole undoes both on the PCB page.

Acceptance: the app builds with both cases handled everywhere the compiler flags, and undo/redo restores the arrays.
  Check: `xcodebuild … test -only-testing:schematicalTests/PourModelTests -only-testing:schematicalTests/PCBUndoTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 7 min; `PCBUndoTests` proves the hole undo path still works beside the new keys).
Commit: —

### Phase 3 — The edit API and the layer guards

- [ ] `SchematicModel.swift`, a `// MARK: Pours and keep-outs (PCB)` section after Holes: `addPour(rect:layer:)` (rect snapped
      and normalized, takes the last pour's settings and the net rule in § 4, selects it, undo name "Add Pour"), `movePour`,
      `setPourRect` ("Move" / "Resize"), `setPourLayer` (only a layer in `board.traceLayers`), `setPourNet`
      ("Change Pour Net"), `setPourClearance` (clamped, nil clears), `setPourConnection`, `setPourThermalGap`, `setPourSpokeWidth`,
      `setPourMinWidth` (each clamped to its range, "Change Pour"), `deletePour`; `addKeepout(rect:layers:)`, `setKeepoutRect`,
      `setKeepoutLayers`, `deleteKeepout`. Each guards a missing id, a locked item for moves, and an unchanged value (no step).
- [ ] `canSetLayerType`: `"Move or delete this layer's pours first."` when `type == .plane` and a pour is on it; `canSetLayerCount`:
      `"Delete or move the pours on \(layer.title) first."` for a dropped layer that holds a pour. A keep-out's `layers` may name a
      dropped layer; it is kept and ignored.
- [ ] Tests: `PourModelTests` for each setter (clamping, unchanged = no step, locked), the net rule for a new pour, and
      `BoardLayersTests` for both refusals.

Acceptance: each setter's undo name and result are asserted; both refusals return their sentence.
  Check: `xcodebuild … test -only-testing:schematicalTests/PourModelTests -only-testing:schematicalTests/BoardLayersTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 7 min).
Commit: —

## Validation Plan

- Tests: `PourModelTests` (new), `BoardLayersTests`, `PCBUndoTests` (existing suites for the paths touched).
- Coverage check: `grep -c "pour" schematical/schematical/Canvas/UndoHistory.swift` → at least 8 (the keys, values, diff and place lines are in the suite's reach).
- Runtime proof: none; nothing user-visible until plan-52-E.
- Doc sync: none here; the file format is plan-52-B's.
- Final gate (once, after Phase 3): `xcodebuild … test -only-testing:schematicalTests/PourModelTests -only-testing:schematicalTests/BoardLayersTests -only-testing:schematicalTests/PCBUndoTests -only-testing:schematicalTests/BoardCopperCacheTests -only-testing:schematicalTests/HoleTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 9 min).

## Decisions

- A pour is a rectangle on one layer, flood filled inside it; it keeps its clearance from other nets' pads, pins and traces and from keep-outs — decided by the owner, 2026-11-21. Rectangles only, no polygon tool. (§ 3, § 4)
- A keep-out blocks copper pours only; traces, vias and parts may still go there — decided by the owner, 2026-11-21. No DRC or router rule for it. (§ 3)
- A keep-out is a rectangle too, on chosen layers, none chosen meaning all — decided by the author: the same tool and handles as a pour; an L-shaped keep-out is two rectangles.
- A pour may go on any trace layer (Top, Bottom, an inner Wire layer), not a Plane — decided by the author: the todo names Top and Bottom, an inner Wire layer costs nothing extra.
- A pour with no net fills nothing — decided by the author: a floating flood has no pin to join; it stays drawn as its outline.
- Pours on one layer fill in drawing order; a later pour of another net gives way to an earlier one — decided by the author: no priority field, no reorder UI.
- New pours copy the last pour's settings; the net is the last pour's, else ground, else none — decided by the author, as `addHole` does.

## Corrections

(Filled in during execution.)

## Summary

The risk is not here: A is data and undo, mirroring holes line for line, behind the switch sites the compiler finds (at most the 26 matches in 8 files above). What stays
untouched: Planes, the file format, the router, every exporter.
