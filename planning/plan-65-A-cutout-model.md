# plan-65-A: Board Cutout Model, Board Edge, Undo and Document API

Last updated: 2026-11-25
Overall Effort: x-large (1d–3d)
Effort: medium (1h–2h)
Depends on: nothing

The PCB's edge is one polygon, `BoardSettings.outline` (a rectangle, or a standard card's shape). It cannot have a hole in it,
and its shape cannot come from a drawing. plan-65 (A–J) adds the **cutout**: a region of the board with no material (a window, a
round hole too big to drill, a milled slot), that the design rule check, the pours, the router, the canvas, the board preview and
every export treat as board edge; a **free-form outline** (a board whose edge is any simple polygon); and **File ▸ Import ▸ DXF…**,
which reads the closed loops of a DXF drawing and makes them the board's edge and its cutouts, or only cutouts in the board it has.
This sub-plan lays the data and the edit API underneath all of it: the cutout value type, the one geometry type every consumer
asks "is this point board material, and where are the edges" of, the cutout's place in the document, undo, the selection item and
the document functions that change cutouts. No screen draws a cutout yet (plan-65-E), and nothing reads one yet (C, D, G).

The single outcome: a document holds cutouts; every change is one named undo step and undoes and redoes exactly; the new
selection item is handled by every code path that switches over `BoardItem`; `BoardEdge` answers point-in-material and gives the
edge segments of the outline plus every cutout.

The whole plan-65, in letter order (each is one sitting):

| Plan | Delivers |
|---|---|
| A (this) | `BoardCutout`, `BoardEdge`, document state, undo, `BoardItem.cutout`, edit API |
| B | `BoardSettings.customOutline` (free-form outline), the file keys `board.outline` and `board.cutouts`, validation, `docs/file-format.md` |
| C | Pour fill and design rule check read `BoardEdge`; two new DRC checks (cutout crosses the edge, cutouts overlap) |
| D | The autorouter reads `BoardEdge` (grid, cleanup, verifier) |
| E | The PCB page draws cutouts and the free-form outline; the Cutout tool (rectangle, circle, slot); select, move, resize, delete |
| F | Sidebar panels, Reset to Rectangle, board preview (3D), selection list text, keyboard help |
| G | Gerber, KiCad, EasyEDA Pro, PDF and image output; `docs/export.md` |
| H | `DXFReader`: a pure parser and loop builder (LINE, ARC, CIRCLE, LWPOLYLINE, POLYLINE, ELLIPSE, SPLINE) |
| I | File ▸ Import ▸ DXF…: the import sheet, the two modes, one undo step |
| J | Automation tools, `docs/agent-control.md`, todo, landing, the real app driven once |

References:

- `planning/todo.md` § PCB (the "Custom board outlines: rounded corners, cutouts, DXF import" item; this plan does cutouts and DXF
  import, not rounded corners of the rectangle), `planning/plan-65-data/README.md` (the probes this design rests on)
- `docs/file-format.md` § Board (`:167`), § Holes (`:254`), § Keep-outs (`:311`); `docs/export.md`; `docs/routing.md`
- `schematical/schematical/Canvas/BoardPour.swift` (`BoardKeepout`, the value type this mirrors), `Canvas/BoardHole.swift`,
  `Canvas/BoardSettings.swift` (`outline` at `:340`), `Canvas/SchematicModel.swift` (`keepouts` at `:2060`,
  `// MARK: Pours and keep-outs (PCB)` at `:4612`), `Canvas/UndoHistory.swift`, `Canvas/Selection.swift` (`BoardItem` at `:21`)
- plan-52-A (`planning/completed/plan-52-A-pour-model.md`): the same job for pours and keep-outs; this sub-plan follows its steps
- Line numbers in this plan are as of 2026-11-25 and drift as other sessions edit these files; the symbol named beside each is the reference.

Test command form (`XB`): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p65-dd CODE_SIGNING_ALLOWED=NO test`;
a Check below writes `XB -only-testing:schematicalTests/<Class>`. Debug, scoped to the classes named.

## Prerequisites

These are a precondition on the whole of plan-65, stated once here.

| Must be true | Command | Status |
|---|---|---|
| plan number 65 is this plan's alone | `ls planning planning/completed \| grep '^plan-65' \| grep -v -E '^plan-65-([A-J]-\|data)'` → no output | MET (2026-11-25) |
| plan-52 is complete (pours, keep-outs and the fill that reads the outline) | `ls planning/completed \| grep -c '^plan-52-[A-H]'` → 8 | MET (2026-11-25: 8) |
| Xcode builds and tests the app | `xcodebuild -version` → `Xcode 26.3` or later; `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/HoleTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` | MET (2026-11-25: `Xcode 26.3`; `Executed 6 tests, with 0 failures`, 49 s with a cold `$DD`; re-run in the worktree 2026-11-25: `** TEST SUCCEEDED **`) |
| KiCad 10's CLI and python are installed (plan-65-G and H check against them) | `ls ~/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli` → the path | MET (2026-11-25) |
| the files plan-65 edits have no uncommitted changes of another session | `git status --porcelain -- schematical/schematical/Canvas schematical/schematical/Document schematical/schematical/DesignRuleCheck schematical/schematical/Routing schematical/schematical/Automation schematical/schematical/BoardPreview schematical/schematical/Help docs` → no output | MET (2026-11-25: no output) |

Everything below is written against the world where these hold. `$DD` in the checks is a scratch derived-data folder (any empty
directory). Every `xcodebuild … test` in plan-65 runs with the Bash tool's `timeout` parameter set (macOS has no `timeout` command: `timeout: command not found`, 2026-11-25) (the test host is the real app; a hung host on the owner's
screen is killed, not waited for: memory *GUI probe etiquette*), and `…` in a check stands for `-project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD`.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again before you decide to stop. Never act on a status
> you did not just verify — a prerequisite recorded NOT MET may have landed since, and one recorded MET may have regressed.
>
> **If you stop, report the current status of *all* prerequisites** — not only the one that blocked you.

## 1. Goal

- `BoardCutout` exists as a value type with the shapes, ranges and flattening in § 4; `BoardEdge` exists with the operations in § 4.
- `SchematicDocument` keeps `cutouts` (drawing order) and offers `boardEdge`; changing `cutouts` bumps `copperRevision`.
- `addCutout`, `moveCutout`, `setCutoutRect`, `setCutoutRadius` and `deleteCutout` each make one named undo step; undo and redo
  restore the array exactly.
- `BoardItem` has `.cutout(UUID)`; select, lock, group-move, delete, the selection list and the automation item names handle it.

### Non-goals (explicit constraints)

- No file format (plan-65-B), no reader of `boardEdge` other than tests (C, D, G), no drawing, tool or sidebar (E, F). Before
  plan-65-E nothing in the UI can make a cutout; only tests call the API.
- No free-form outline here (B). `BoardSettings` is not changed.
- No change to existing files' bytes or to any export: a document with no cutouts saves and behaves exactly as before.
- No cutout is checked against the outline or against another cutout here (that is the design rule check, plan-65-C); `addCutout`
  accepts any valid shape wherever it is.

## 2. Current State

- `BoardKeepout` (`Canvas/BoardPour.swift:89`) is the precedent: a `nonisolated struct … Identifiable, Hashable, Sendable` with
  `id`, a `rect` in mm, `locked`, and a `problem(rect:)` validator that returns a file-error detail.
- The document keeps `private(set) var keepouts: [BoardKeepout] = [] { didSet { copperRevision &+= 1 } }`
  (`SchematicModel.swift:2060`); `pours`, `holes`, `vias` and `traces` are the same. `copperRevision` invalidates `keptConnectivity`
  and `keptRatsnest`.
- Undo: `DocumentState` (`UndoHistory.swift:10`, `keepouts` at `:24`), `UndoKey` (`:30`), `UndoValue` (`:46`), `UndoDiff.changes`
  (`keepouts` diffed at `:123`) and `place(…)` (`:229`). `recordEdit(.pcb, name) { … }` records one step.
- The keep-out API is `// MARK: Pours and keep-outs (PCB)` (`SchematicModel.swift:4612`): `addKeepout` (`:4737`), `moveKeepout`
  (`:4747`), `setKeepoutRect` (`:4755`), `deleteKeepout` (`:4771`); lock in `isLocked`/`setLocked` (`:5002`, `:5025`); group drags in
  `boardGroupBase` (`:5060`) / `moveBoardGroup`; delete in `deleteBoardSelection` (`:4070`); rubber-band and select-all
  (`:1934`, `:1991`); `pruneSelection` (`:2030`).
- `BoardGroupBase` (`Selection.swift:170`) holds one dictionary per item kind (`keepouts: [UUID: CGRect]`) and sums them in `count`.
- The edge today is `BoardSettings.outline: [CGPoint]` (`BoardSettings.swift:340`), clockwise from the top-left, y down, in mm. Its
  consumers each build their own segments and point-in-polygon test: `DesignRuleCheck.swift:88` (`outline`, `inside` at `:380`),
  `Routing/RoutingGrid.swift:228` (`stampEdge`), `RoutingVerifier.swift:191` (`inside`), `PourFill.swift:98`.

### Measured populations

| What | Count | Command |
|---|---|---|
| lines that read the board outline (the consumers § 3 lists) | 28 lines in 15 files | `rg -n "board\.outline\|settings\.outline\|problem\.outline\|artwork\.outline\|\.board\.outline" schematical/schematical --glob '*.swift' \| wc -l` → 28; `rg -l "board\.outline\|settings\.outline\|problem\.outline\|artwork\.outline\|outlinePoints" schematical/schematical --glob '*.swift' \| wc -l` → 15 |
| lines that name the `.keepout` selection item (the upper bound of the `BoardItem` switches to extend) | 31 lines in 9 files | `rg -n "\.keepout\(\|\.keepout:\|case \.keepout" schematical/schematical --glob '*.swift' \| wc -l` → 31; `rg -l …` → AutomationRead, PCBSidebarView, AutomationBoard, UndoHistory, SchematicModel, PourSection, PCBCanvasView, AutomationSchematic, SelectionList |
| lines about keep-outs in `SchematicModel.swift` | 60 | `rg -ic 'keepout\|keep-out' schematical/schematical/Canvas/SchematicModel.swift` → 60 |
| test files today | 193 | `ls schematical/schematicalTests/*.swift \| wc -l` → 193 |

### Verified properties

- New source and test files need no project-file edit: `grep -c PBXFileSystemSynchronizedRootGroup schematical/schematical.xcodeproj/project.pbxproj` → 5.
- The check command form works on this tree: `XB -only-testing:schematicalTests/HoleTests CODE_SIGNING_ALLOWED=NO` → `Executed 6 tests, with 0 failures`, `** TEST SUCCEEDED **`, 49 s wall with a cold `$DD` (2026-11-25).
- `CGPath.subtracting`, `union` and `componentsSeparated` give exact results on shapes like these: `planning/plan-52-data/README.md` (`mismatch 0 of 200000` sample points). C and G rely on it; A does not call them.
- The flattening rule in § 4 gives, for a 0.01 mm sagitta, 16 segments for a 0.5 mm radius, 32 for 1.6 mm, 52 for 5 mm and 160 for 50 mm (per quarter turn: 4, 8, 13, 40)
  (`python3 -c "import math; print(max(4, math.ceil((math.pi/2)/(2*math.acos(1-0.01/1.6)))))"` → 8 per quarter), so a 3.2 mm round hole is a 32-gon.

## 3. Design Overview (the whole plan-65)

**Terms.** A **cutout** is a closed region of the board with no material. Its shape is *rounded* (a rectangle with a corner
radius: with radius 0 a rectangle, with a square box and radius half its side a circle, with radius half the short side of a longer box a slot) or
*polygon* (the loop a DXF gave). The **outline** is the board's outer edge. The **board edge** is the outline plus every cutout's loop.
**Material** is what is inside the outline and outside every cutout.

**One geometry type, one rule.** `BoardEdge` (`Canvas/BoardEdge.swift`) is a `nonisolated struct … Hashable, Sendable` built from the outline and the
cutouts' polygons. Every reader of the edge takes one: `contains(point)` is true on material (inside the outline by the even-odd
rule, and inside no cutout), `segments` are the closed segments of every loop, `loops` every loop, `path` is one `CGPath` of all loops (fill it with the even-odd
rule and the cutouts are holes), `bounds` is the outline's. The union rule (inside the outline and not inside *any* cutout), not
even-odd over all loops, so a cutout that overlaps another or crosses the outline still gives a defined answer; the design rule check reports those two states (C).

**Free-form outline (B).** `BoardSettings.customOutline` is a clockwise y-down polygon with its bounds' top-left at (0, 0), used while
`type == .custom`. `BoardSettings.outline` returns it, so every existing reader of `outline` sees it with no change; width and height
are its bounds.

**Where the correctness risk concentrates.** (1) The pour fill's cache (`keptPourFills`, read in `pourFills(for:plain:)`, `PourConnectivity.swift`) compares the pours
and keep-outs; it must also compare the edge, or a moved cutout leaves a stale fill (C). (2) The design rule check's single `inside`
serves two jobs that must now differ: bodies may overhang a cutout (placement uses the outline alone) but copper may not be in one (C). (3) The router's
edge stamp decides "outside" by crossing parity per row; it must use `BoardEdge.contains` or a cutout is routable (D). (4) Every
export must stay byte-identical for a document without cutouts and without a free-form outline (G).

**What was probed before writing.** KiCad's DXF output and the nested-Edge.Cuts check (`planning/plan-65-data/README.md`: KiCad writes
LINE/ARC/CIRCLE with `$INSUNITS` 4, and loads an inner loop as a cutout with no violation); KiCad's zone fill keeps out of an inner Edge.Cuts
loop and its 0.5 mm clearance (`probe_zone_edge_cuts.py`: `False` at the cutout's centre and 0.1 mm from its edge, `True` 5 mm away), so the Plane zone's
polygon stays the outline; five DXF files written by ezdxf with the loop areas the reader must reproduce.

**Byte-identity is not this plan's correctness gate** (behaviour is added). It is the check that nothing changes for a document with
no cutouts: the existing goldens and exporter tests (`GerberExportTests`, `KiCadExportTests`, `DesignRuleGoldenTests`,
`PourFillTests`, `RoutingPlanTests`) pass unchanged in C, D and G. A diff there is a bug in the new code, to be root-caused.

**Decisions made by the author (no user-visible fork), with the alternatives rejected:**

- Cutout shapes are *rounded* and *polygon* only; circle and slot are presets of rounded. Rejected: a separate shape case each (three more
  cases in every switch for no extra geometry).
- A polygon cutout is moved and deleted whole, never edited vertex by vertex (the owner's choice, below), and is never resized.
- Curves are flattened once, to a polygon, at a sagitta of 0.01 mm (`BoardCutout.flatness`), with at least 4 segments per quarter turn; every consumer
  uses `polygon`, none flattens its own. Rejected: true arcs in the file and the Gerber (no consumer of the edge is arc-aware; the outline is already a polygon with arcs flattened by `boards.json`).
- Cutouts do not belong to a layer or a net (they go through the whole board). Rejected: per-layer cutouts (a blind pocket is not a PCB cutout).
- The union rule, not even-odd, decides material (above).
- Overlapping cutouts and a cutout outside the outline are legal states of the model and the file; the design rule check names them (C).

## 4. Detailed Design

### 4.1 `BoardCutout` (`Canvas/BoardCutout.swift`)

```swift
nonisolated struct BoardCutout: Identifiable, Hashable, Sendable {
    let id: UUID
    var shape: Shape
    var locked = false

    enum Shape: Hashable, Sendable {
        case rounded(rect: CGRect, radius: Double)   // mm
        case polygon([CGPoint])                      // mm, clockwise (y down), closed implicitly
    }
}
```

- `static let minimumSide = BoardPour.minimumSide` (0.5 mm); `static let flatness = 0.01` (mm); `static let minimumArcSegments = 4`.
- `Shape.bounds: CGRect`. `Shape.translated(by: CGSize) -> Shape`.
- `Shape.polygon: [CGPoint]` (a method on the case: `loop`), the flattened loop, clockwise in y-down, no repeated end point.
  Rounded: radius 0 → the four corners from the top-left; radius r > 0 → four corner arcs, each with
  `n = max(4, ceil((π/2) / (2·acos(1 − flatness/r))))` segments, starting at the top-left corner's arc, points on the arc (not the chord midpoints). A polygon case returns its points.
- `kind: Kind` (`rectangle`, `circle`, `slot`, `polygon`) for names in lists and panels: `rounded` with a square box and radius within 1e-9 of half
  the side is `circle`; radius within 1e-9 of half the short side of a non-square box is `slot`; else `rectangle`.
- `static func problem(_ shape: Shape) -> String?`: rounded: each side at least `minimumSide`, `0 ≤ radius ≤ min(width, height)/2`, finite
  numbers; polygon: at least 3 points, finite, non-zero area, and no two non-adjacent edges cross. Returns the detail after
  "cutout <id> " (the file reader, B, uses it).
- `static func normalized(_ rect: CGRect, radius: Double) -> (CGRect, Double)`: standardized, each side raised to `minimumSide`, radius clamped.
- Circle and slot constructors: `static func circle(centre: CGPoint, diameter: Double) -> Shape` and `slot(rect:)` (radius = half the short side).

### 4.2 `BoardEdge` (`Canvas/BoardEdge.swift`)

`nonisolated struct BoardEdge: Hashable, Sendable { let outline: [CGPoint]; let cutouts: [[CGPoint]] }` with `loops`, `segments: [(CGPoint, CGPoint)]`
(each loop's consecutive pairs and the closing pair), `bounds` (the outline's), `path: CGPath` (every loop, closed), `contains(_ point: CGPoint) -> Bool`
(the outline's even-odd crossing test, then false inside any cutout's even-odd test) and `static func contains(_ point:, in loop:) -> Bool`
(the one even-odd loop test, so DRC, router and exports stop keeping private copies). `SchematicDocument.boardEdge` builds it from
`board.outline` and `cutouts.map { $0.shape.polygon }`.

### 4.3 Document state, undo, selection

Mirror plan-52-A § 4 for keep-outs, replacing the keep-out with the cutout: `DocumentState.cutouts`, `UndoKey.cutout(UUID)`,
`UndoValue.cutout(BoardCutout, index: Int)`, a `diff(…)` line in `UndoDiff.changes` and a `place(…)` line. `BoardItem.cutout(UUID)`.
`BoardGroupBase.cutouts: [UUID: BoardCutout.Shape]`, counted in `count`.

### 4.4 Edit API (`SchematicModel.swift`, `// MARK: Cutouts (PCB)`)

| Function | Undo step name | Behaviour |
|---|---|---|
| `addCutout(_ shape: BoardCutout.Shape) -> UUID?` | `Add Cutout` | Refuses (nil) a shape `problem` rejects. A rounded shape's rect is snapped to the trace grid (`snappedRect` rounds a rect's four sides; reuse it, then `BoardCutout.normalized`); a polygon is stored clockwise (`BoardGeometry.clockwise`). Appends, selects it, returns its id. |
| `moveCutout(_ id: UUID, to origin: CGPoint)` | `Move` | Moves the shape so its bounds' top-left is `origin` snapped to the trace grid. No-op when locked. |
| `setCutoutRect(_ id: UUID, _ rect: CGRect, name: String = "Resize")` | `name` | Rounded only (no-op on a polygon, or when locked); `normalized`; keeps the radius within the new box. |
| `setCutoutRadius(_ id: UUID, _ radius: Double)` | `Change Cutout` | Rounded only; clamped to `0…min(w,h)/2`. |
| `deleteCutout(_ id: UUID)` | `Delete` | Removes it and prunes the selection. |

`isLocked`/`setLocked`, `boardGroupBase`, `moveBoardGroup` (one item: `moveCutout`; several: every selected cutout by the
snapped `offset`, via `Shape.translated`), `deleteBoardSelection`, rubber-band selection (a cutout is taken whole, its bounds
inside the box) and select-all, `pruneSelection`, and the selection list rows (`SelectionList.swift`: title `Cutout`, detail `Rectangle 10 × 5 mm`,
`Circle ⌀3.2 mm`, `Slot 8 × 2 mm` or `Polygon, 24 points`) handle `.cutout`. A `switch` over `BoardItem` in `PCBCanvasView` and
`PCBSidebarView` gets a minimal `.cutout` case that draws and shows nothing (E and F fill it in); the automation item name is `cutout:ID`
(`AutomationSchematic.swift`, `AutomationRead.swift`: the same spots as `keepout:ID`).

## Phases

> **NOTE — keep the checkboxes current as you go. This is not bookkeeping; it is the only way anyone can see where the work actually is.**
>
> - Tick `- [x]` **in the same commit as the work it describes**.
> - Use `- [~]` for partially done and say in one line what remains.
> - Mark a task moot with `- [x] ~~text~~ — moot: <evidence>` rather than deleting it.
> - Fill the phase's `Commit:` line with the hash(es) the moment it lands.
> - Add any task you discover you needed.
>
> **An unticked box means NOT DONE.**

### Phase 1 — Value types and `BoardEdge` (no callers)

- [x] `Canvas/BoardCutout.swift`: `BoardCutout`, `Shape`, `Kind`, `flatness`, `polygon`, `bounds`, `translated`, `problem`, `normalized`, circle and slot constructors, per § 4.1.
- [x] `Canvas/BoardEdge.swift`: `BoardEdge` per § 4.2.
- [x] Tests `schematicalTests/BoardCutoutTests.swift` (17 tests; `BoardCutout.Shape.distance(to:)`, which plan-65-E lists, was written here with its test): a rectangle's four corners from the top-left, clockwise; a 3.2 mm circle has 32 points, every point 1.6 mm ± 1e-9 from its centre and every chord's midpoint within 0.01 mm of the circle; a slot's end caps; a radius larger than half the short side is refused by `normalized` and by `problem`; a bow-tie polygon is refused; `kind` of a circle, slot, rectangle and polygon; `BoardGeometry.clockwise` reverses a counter-clockwise square and leaves a clockwise one; `BoardGeometry.crossesItself` is true for a bow-tie and false for an L-shape; `BoardEdge.contains` for a point in the outline, in a cutout, outside the outline, on a cutout that overlaps another and on one that crosses the outline; `segments` count is the sum of the loops' point counts; an empty outline gives an edge that contains nothing.

Acceptance: the types exist, are `Sendable` (the build has no concurrency diagnostics) and the tests pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/BoardCutoutTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 2 min: a build and one fast test class). Observed: `Executed 17 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: bb26d125

### Phase 2 — Document state, undo, selection and the edit API

- [x] `Canvas/SchematicModel.swift`: `private(set) var cutouts: [BoardCutout] = [] { didSet { copperRevision &+= 1 } }` beside `keepouts`; `var boardEdge: BoardEdge`; `replaceContents(…)` (`:2092`, what opening a file calls) takes `cutouts: [BoardCutout] = []` after `keepouts`; `state` (`:2161`) and `setState` (`:2168`) carry `cutouts`; the `// MARK: Cutouts (PCB)` functions of § 4.4; `isLocked`/`setLocked`, `boardGroupBase`, `moveBoardGroup`, `deleteBoardSelection`, rubber-band, select-all and `pruneSelection` handle `.cutout`.
- [x] `Canvas/Selection.swift`: `BoardItem.cutout(UUID)`; `BoardGroupBase.cutouts`, counted.
- [x] `Canvas/UndoHistory.swift`: `DocumentState.cutouts`, `UndoKey.cutout`, `UndoValue.cutout`, the diff and place lines; `Document/SchematicNSDocument.swift:231` (the "state is empty" test) counts `cutouts`.
- [x] `Canvas/SelectionList.swift`: the cutout rows of § 4.4. `Automation/AutomationSchematic.swift`, `Automation/AutomationRead.swift`: the item name `cutout:ID` where `keepout:ID` is handled.
- [x] `Canvas/PCBCanvasView.swift`, `Canvas/PCBSidebarView.swift`, `Canvas/PourSection.swift`: the minimal `.cutout` cases so every `switch` over `BoardItem` compiles (the compiler lists them; the census bound is the 31 lines above).
- [x] Tests `schematicalTests/CutoutModelTests.swift`: each API call is one named step (`document.undoManager` step name) and undo/redo restores `cutouts` exactly; a locked cutout ignores move, rect and radius; `addCutout` refuses a bad shape and leaves no step; the grid snap of a rounded cutout; group move of a cutout with a hole and a keep-out moves each by the same snapped delta; deleting a selection holding a cutout removes it; `copperRevision` rises on add and on move; `isLocked`/`setLocked`; a document with no cutouts has `boardEdge.cutouts` empty and `boardEdge.outline == board.outline`.

Acceptance: every cutout change undoes and redoes exactly; the whole app target builds with the new `BoardItem` case handled everywhere; existing undo, selection and hole tests still pass.
  Check: `XB -only-testing:schematicalTests/CutoutModelTests -only-testing:schematicalTests/PourModelTests -only-testing:schematicalTests/HoleTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 3 min; `PourModelTests` is the nearest existing test of the same `BoardItem`/undo paths, so a missed case there fails). Observed: `CutoutModelTests` 14 tests, `PourModelTests` 18, `HoleTests` 6, 0 failures, `** TEST SUCCEEDED **`.
Commit: —

## Validation Plan

- Tests: `BoardCutoutTests` (the value, flattening, `BoardEdge`) and `CutoutModelTests` (document, undo, selection), including the refusal cases.
- Coverage check: the new files are in the app target (`PBXFileSystemSynchronizedRootGroup` picks them up) and the test files in the test target; the Phase 2 check runs both.
- Runtime proof: none beyond the tests; no UI exists yet (plan-65-E is the first sub-plan a person can see).
- Doc sync: none here; the file format and the agent tools are documented in B and J.
- Final gate (run ONCE, after Phase 2): the two Phase checks' classes together plus `-only-testing:schematicalTests/KeyboardHelpTests` (`BoardItem` is read by the help text) (est. 4 min). The full unit bundle is not a plan step. Observed (BoardCutoutTests 17, CutoutModelTests 14, KeyboardHelpTests 6): 0 failures, `** TEST SUCCEEDED **`.

## Decisions

- Cutouts are drawn as rectangle, circle and slot, plus polygons from a DXF that move and delete whole but are not edited by vertex — decided by the owner, 2026-11-25. Settles § 3 and 4.1.
- The design rule check holds copper, not bodies, off a cutout: pads, traces, vias, holes and pour copper keep the board-edge clearance; a body outline may overlap a cutout — decided by the owner, 2026-11-25. Settles plan-65-C.
- DXF import offers two modes in its sheet, outline plus cutouts, and cutouts only — decided by the owner, 2026-11-25. Settles B (free-form outline) and I.

## Corrections

- 2026-11-25: the plan said to run checks under `timeout 1800`; `timeout` does not exist on this Mac (`zsh: command not found: timeout`). Checks run with the Bash tool's own `timeout` parameter instead (plan-65-J's `timeout 3000` likewise).
- 2026-11-25: `Shape.polygon` (the flattened loop, plan text in § 3, 4.1, 4.4 and plans C, D, G, I) is named `Shape.loop` in the code, because `.polygon` is an enum case of `Shape`; wherever a later plan says `shape.polygon`, read `shape.loop`.
- 2026-11-25: the selection ring for `.cutout` (plan-65-E Phase 1) was written in A Phase 2 because the exhaustive `switch` needed a case anyway; E keeps its test and the rest of its drawing.
- 2026-11-25: the plan counted a 3.2 mm circle as a 29-gon, using the segments per full circle; the rule it states (at least 4 per quarter turn, `ceil((π/2)/(2·acos(1 − 0.01/r)))` per quarter) gives 8 per quarter, 32 in all (`python3 -c "import math; print(max(4, math.ceil((math.pi/2)/(2*math.acos(1-0.01/1.6)))))"` → 8). A, C and § 2 corrected: 16, 32, 52, 160 segments for radii 0.5, 1.6, 5, 50 mm.

## Summary

The risk in A is small and structural: one more `BoardItem` case that every exhaustive `switch` must handle, and undo that must be exact. The
decision that matters for the rest of plan-65 is `BoardEdge`: one type every reader of the edge asks, so C, D and G change what they
call, not what they compute. Left untouched: `BoardSettings`, every file, every export, every reader of the outline.
