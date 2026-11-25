# plan-65-E: The PCB Page Draws Cutouts and the Cutout Tool

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-65-D (complete)

Everything underneath exists (A–D); nothing is drawn. This sub-plan makes the PCB page show a cutout as a hole in the board (and a free-form outline as the board's shape), adds the **Cutout** tool
(drag a rectangle, a circle or a slot), and makes cutouts selectable, movable, resizable by handles and deletable, the way pours and keep-outs are.

The single outcome: in the PCB page a person drags a cutout, sees a hole in the board with the grid and the canvas background showing through, selects it by its edge, drags it, resizes it by its handles and deletes it, each as one undo step, and a board without cutouts draws exactly as before.

References:

- plan-65-A § 3, § 4.1, § 4.4; plan-52-E and plan-52-F (`planning/completed/plan-52-E-pour-canvas.md`, `plan-52-F-pour-edit.md`): the same job for pours and keep-outs
- `schematical/schematical/Canvas/PCBCanvasView.swift`: `draw` (`:336`), the board fill (`:348`–`:350`), `drawPlane` (`:590`), keep-out drawing (`:405`), the handles (`:412`–`:416`), the selection ring (`:478`–`:496`), `finishRect` (`:510`), the tool press (`:975`–`:988`), `pourHandle` use (`:1010`), `PCBTool` (`:1290`)
- `Canvas/PourSection.swift`: `PourHandle` (`:8`), `pourHandle(at:tolerance:)` (`:63`), `resizePourItem` (`:73`); `Canvas/SchematicModel.swift`: `pourItem(at:…)` (`:4791`), `distance(from:toBoundaryOf:)` (`:4813`)
- `ContentView.swift`: `pcbTool` (`:45`), the tool picker and trace-layer menu (`:623`–`:645`); `Help/KeyboardHelp.swift`
- `schematicalTests/PourCanvasTests.swift` (the test this one mirrors), `KeyboardHelpTests.swift`

Test command form (`XB`): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p65-dd CODE_SIGNING_ALLOWED=NO test`;
a Check below writes `XB -only-testing:schematicalTests/<Class>`. Debug, scoped to the classes named.

## Prerequisites

The shared gate is in plan-65-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-65-D is complete | `ls planning/completed \| grep -c '^plan-65-D'` → 1 | MET (2026-11-25: D landed, 1) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update every status
> before you continue, and again before you decide to stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- The board is drawn as `edge.path` filled even-odd: a cutout is a hole through which the canvas shows; the board edge stroke runs round the outline and every cutout; a Plane's tint is drawn even-odd too, so it stops at a cutout and keeps the copper-to-edge band round it.
- `PCBTool.cutout` ("Cutout", symbol `rectangle.dashed`) with a Rectangle / Circle / Slot menu beside the Trace layer menu; dragging makes the shape: Rectangle = the box; Slot = the box with radius half its short side; Circle = a square whose side is the longer drag side, in the drag's direction.
- A cutout is selected by pressing within the tolerance of its edge (every shape: a polygon's edge is its segments) and drawn with the selection ring on its bounds; it moves by dragging (alone, or in a group), by `moveBoardGroup`; a rounded cutout has the eight pour handles on its bounds and resizing is `setCutoutRect`.
- Undo, redo, the Delete key (A) and the Delete tool (a press on a cutout's edge deletes it, as it does a pour's or keep-out's: `deleteBoardItem(at:)` switches over `pourItem(at:)`, `SchematicModel.swift:3979`) work; hidden `.board` layer: the board fill is off but the outline and the cutout edges still stroke.

### Non-goals (explicit constraints)

- No sidebar panel or numeric fields (F); no board preview (F); no export (G); no polygon vertex editing and no polygon resize (the owner's choice); no handle for the radius (a field in F).
- No change to how a board without cutouts draws: the fill, the stroke, the planes and every hit test produce the same pixels and results (`PourCanvasTests` and the canvas tests pass unedited).

## 2. Current State

- `draw` builds `boardShape = Path { addLines(board.outline); closeSubpath }`, fills it with the board colour when `.board` is shown, strokes it 0.2 mm in the silkscreen colour, and passes it to `drawPlane(shape:)` (`PCBCanvasView.swift:348`–`:377`). `drawPlane` fills `shape`, then strokes it `2 * edge` wide in `.clear` blend to leave the edge band (`:590`–`:612`).
- The tools: `PCBTool` (`:1290`) has `select … pour, keepout, delete, route`; `title`, `tooltip` and `symbol` switch over it. A press with `.pour` or `.keepout` returns `.drawRect(kind:from:to:)` (`:984`–`:986`); the release calls `finishRect(document, kind:, from:, to:, moved:, layer:)` (`:510`–`:527`), which refuses a drag under 3 screen points or a side under `BoardPour.minimumSide`.
- Selecting a pour or keep-out: `pourItem(at:tolerance:visible:)` hits only the boundary; the handles are `PourHandle` (eight positions on the rect) with `pourHandle(at:)` and `resizePourItem`; the interaction is `.resize(item:handle:)` (`:137`).
- Keep-outs are drawn hatched in red with a red rect stroke (`:405`–`:408`).

### Measured populations

| What | Count | Command |
|---|---|---|
| places that switch on `.keepout` as a tool in `PCBCanvasView.swift` (each gets a `.cutout` case) | 5 (`finishRect` `:524`, the press `:984`, `title` `:1316`, `tooltip` `:1329`, `symbol` `:1344`) | `rg -n "case \.keepout\|\.keepout:\|, \.keepout" schematical/schematical/Canvas/PCBCanvasView.swift` → those 5 lines |
| lines that draw the board shape in `PCBCanvasView.swift` | 4 (`:348` build, `:349` fill, `:350` stroke, `:377` plane) | `rg -n "board\.outline\|boardShape" schematical/schematical/Canvas/PCBCanvasView.swift` → 4 lines |
| tests of the pour/keep-out canvas this one mirrors | 1 class | `ls schematical/schematicalTests/PourCanvasTests.swift` → exists |

### Verified properties

- `GraphicsContext.fill(_:with:style:)` takes `FillStyle(eoFill: true)`; `draw` already uses it for pour fills (`style: FillStyle(eoFill: true)`, `PCBCanvasView.swift:369`).
- `Path(CGPath)` and a compound `CGPath` of several closed loops are the same thing the pour fill uses (`pourPaths` returns `Path($0.path)`), so `edge.path` (A § 4.2) is drawn as is.
- `.blendMode = .clear` plus `plane.fill(shape, …)` with `eoFill` leaves the hole transparent (the same drawing operation as the existing pad cuts, `:593`–`:610`).

## 3. Design Overview

**Drawing.** `draw` builds `let edge = document.boardEdge` and `boardShape = Path(edge.path)` once; the fill uses `style: FillStyle(eoFill: true)`; the stroke is unchanged (it runs over every loop of the compound path); `drawPlane`'s two `fill`/`stroke` calls on `shape` use the even-odd style on the fill (the stroke of every loop already clears the edge band). A free-form outline needs nothing else: `edge.outline` is the polygon.

**Tool.** `PCBTool.cutout` after `keepout`. `enum CutoutShape: String, CaseIterable { rectangle, circle, slot }` (title, symbol) is `@State cutoutShape` in `ContentView`, passed to the canvas; the menu is `Menu(cutoutShape.title)` like the Trace layer menu, and picking one also picks the tool. `finishRect` gets `cutoutShape: CutoutShape = .rectangle` and, for `.cutout`, builds the shape per § 1 and calls `document.addCutout`.

**Selecting and moving.** `pourItem(at:…)` (which `boardItem(at:)`, the Select tool's hit test, and `deleteBoardItem(at:)` both call) gains the cutouts after keep-outs: the last cutout whose edge is within `tolerance` — for a rounded cutout `distance(from:toBoundaryOf:)` of its bounds is wrong at a circle's corner, so the distance is to the polygon's nearest segment (`BoardCutout.Shape.distance(to:)`, new, with the segment distance helper `SchematicDocument.distance(from:toSegment:_:)` that the ratsnest hit test already uses). Group moves are A.

**Handles.** `pourHandle(at:)` also offers the handles of the one selected rounded cutout (not a polygon) and `resizePourItem` calls `setCutoutRect` for `.cutout`. The handle drag keeps the radius within the new box (`setCutoutRect`, A).

**Drawing a cutout's selection and preview.** Selection ring: the shape's bounds, as for a keep-out. While dragging with the tool, the preview is the shape's polygon stroked in the accent colour (the same `.drawRect` interaction state; `finishRect`'s shape builder is the one function the preview and the release call).

Risk: the even-odd fill must not change any pixel of a board without cutouts (`edge.path` of an outline alone is one loop, so even-odd equals non-zero); the plane tint and the clear stroke must agree on where the band is.

Byte-identity is not a gate here (behaviour is added); the no-cutout check is the existing canvas tests. Rejected: drawing the hole by painting the background colour over the board (the grid and a Plane's tint would show wrong); one toolbar button per shape (three more buttons in a bar of nine).

## Phases

> **NOTE — keep the checkboxes current as you go. This is not bookkeeping; it is the only way anyone can see where the work actually is.**
>
> - Tick `- [x]` **in the same commit as the work it describes**; use `- [~]` for partially done and say what remains; mark a moot task `- [x] ~~text~~ — moot: <evidence>`; fill the phase's `Commit:` line with the hash(es) the moment it lands; add any task you discover you needed.
>
> **An unticked box means NOT DONE.**

### Phase 1 — Drawing

- [x] `Canvas/PCBCanvasView.swift`: `boardShape` from `document.boardEdge.path`; `FillStyle(eoFill: true)` on the board fill and in `drawPlane`; the selection ring for `.cutout` (bounds, as keep-out); a static `PCBCanvasView.boardPath(_ edge: BoardEdge) -> Path` that tests call.
- [x] Tests `schematicalTests/CutoutCanvasTests.swift` (first half): `boardPath` of an edge with a 10 × 10 cutout contains (even-odd) a material point and not the cutout's centre; with no cutouts it equals the outline path (`Path` equality of the two); a rendered image of a board with a cutout (`ImageRenderer` of the canvas view with `exporting`) has no opaque pixel at the cutout's centre and an opaque board-coloured pixel 3 mm outside it (`ImageExport.data(for:options)` with the PCB view and a transparent background, sampled with `NSBitmapImageRep`, as `PourCanvasTests.testImageExportShowsPoursButNotKeepouts` does).

Acceptance: a cutout is a hole in the drawing; a board without one draws as before.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/CutoutCanvasTests -only-testing:schematicalTests/PourCanvasTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 4 min; `PourCanvasTests` draws boards without cutouts, so it is the no-cutout check).
  Observed: CutoutCanvasTests 3 + PourCanvasTests 19, 0 failures, TEST SUCCEEDED.
Commit: d4a2d80b

### Phase 2 — The Cutout tool, selection, move and resize

- [x] `Canvas/PCBCanvasView.swift`: `PCBTool.cutout` (title `Cutout`, tooltip `Cutout — drag a rectangle, circle or slot to cut out of the board`, symbol `rectangle.dashed`); `CutoutShape`; the press returns `.drawRect(kind: .cutout, …)`; `finishRect(…cutoutShape:)` per § 3; the preview stroke.
- [x] `ContentView.swift`: `@State cutoutShape`, the menu beside the Trace layer menu, passed to the canvas.
- [x] `Canvas/BoardCutout.swift`: `Shape.distance(to point: CGPoint) -> Double` (to the nearest segment of `polygon`). Done in plan-65-A Phase 1 (`BoardCutoutTests.testDistanceToTheEdge`); the property is named `loop`, not `polygon`.
- [x] `Canvas/SchematicModel.swift`: `pourItem(at:…)` returns `.cutout`; `deleteBoardItem(at:…)` gets a `.cutout(id)?` case calling `deleteCutout`; `Canvas/PourSection.swift`: `pourHandle(at:)` and `resizePourItem` for a selected rounded cutout.
- [x] `Help/KeyboardHelp.swift`: a `Cutout` entry beside `Keep-out` (`:156`) and the Delete entry (`:157`) says `… pour, keep-out or cutout edge …`; `PCBTool.delete`'s tooltip (`PCBCanvasView.swift:1330`) the same.
- [x] Tests `CutoutCanvasTests` (second half): `finishRect` with `.cutout` and each `CutoutShape` makes a rounded cutout whose `kind` is `rectangle`, `circle` (square, radius half the side) and `slot`; a drag under 3 points or a side under 0.5 mm makes nothing; `pourItem(at:)` finds a polygon cutout and a circle on their edges and not 3 mm inside or outside; a handle drag changes `setCutoutRect` and one undo restores it; a Delete-tool press on a cutout's edge removes it (one step) and 3 mm inside it removes nothing; a group drag of a cutout and a keep-out moves both by one snapped delta; `KeyboardHelpTests` passes with the new entry.

Acceptance: each of the five interactions works through its function and is one undo step.
  Check: `XB -only-testing:schematicalTests/CutoutCanvasTests -only-testing:schematicalTests/PourCanvasTests -only-testing:schematicalTests/KeyboardHelpTests -only-testing:schematicalTests/CutoutModelTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 4 min).
  Observed: CutoutCanvasTests 10 + PourCanvasTests 19 + KeyboardHelpTests 6 + CutoutModelTests 14, 0 failures, TEST SUCCEEDED.
Commit: —

## Validation Plan

- Tests: `CutoutCanvasTests` both halves; the negative cases (a too-small drag, a press away from the edge).
- Coverage check: `PourCanvasTests` draws the same code (`draw`, `finishRect`, `pourItem`), so a break of the pour paths fails there.
- Runtime proof: J drives the real window (drag a cutout, move it, resize it, undo).
- Doc sync: the keyboard help (this sub-plan); `docs/agent-control.md` is J's.
- Final gate (run ONCE, after Phase 2): the Phase 2 check (est. 4 min).

## Decisions

- Cutouts are drawn as rectangle, circle and slot; a polygon (from a DXF) moves and deletes whole — decided by the owner, 2026-11-25. Settles the tool's three shapes and the absence of a vertex tool.
- One Cutout tool with a shape menu — decided by the author, 2026-11-25. Settles § 3.

## Corrections

- 2026-11-25: a circle or slot drawn with the tool came out a plain rectangle with a radius: `addCutout` snapped the box to the trace grid but kept the old radius, so `kind` (radius = half the short side, to 1e-9) no longer held. `addCutout` now keeps a circle square and a circle or slot fully rounded after snapping, and `setCutoutRect` keeps a circle or slot fully rounded when it is resized (found by `testFinishRectMakesEachShape`; plan-65-A's API section did not say this). A rectangle with a radius keeps its radius, clamped.
- 2026-11-25: `PCBTool.delete`'s tooltip now names pours, keep-outs and cutouts (it already deleted the first two; the keyboard help said so, the tooltip did not).

## Summary

Risk: pixels of ordinary boards (kept by the existing canvas tests) and the Plane band round a cutout. Left untouched: the sidebar, the board preview, the exports, the file.
