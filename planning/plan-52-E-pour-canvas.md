# plan-52-E: Pours and Keep-outs on the PCB Page

Last updated: 2026-11-21
Effort: medium (1h–2h)
Depends on: plan-52-D (complete)

The document holds pours and keep-outs (A), saves them (B), fills them (C) and counts them as copper (D). This sub-plan is the first
screen: the PCB canvas draws the filled copper and the keep-outs, two new tools (Pour, Keep-out) drag out a rectangle to make one,
and the Select tool picks, moves and deletes them like a hole.

The single outcome: on the PCB page, dragging with the Pour tool over a board makes a pour that fills round the pads and traces of
other nets and joins its own; dragging with the Keep-out tool makes a hatched area the pour stays out of; both can be selected by
their edge, dragged, deleted (Delete tool, ⌘⌫) and undone.

References:

- plan-52-A § 4, plan-52-C § 4, plan-52-D § 1
- `schematical/schematical/Canvas/PCBCanvasView.swift`: `draw(in:size:)` (`:294`; its layer loop), `drawPlane` (`:459`), `Interaction` (`:109`),
  `handle(start:translation:)` (`:762`), `begin(at:)` (`:801`), `end(translation:)` (`:858`), `PCBTool` (`:1103`)
- `Canvas/SchematicModel.swift`: `boardItem(at:tolerance:visible:)` (`:4378`), `boardItems(in:visible:)` (`:1626`), `boxSelectBoard` (`:1655`), `selectAllBoard` (`:1680`),
  `deleteBoardItem(at:…)` (`:3495`), `moveBoardGroup` (`:4311`)
- `ContentView.swift:599` (the tool picker), `Help/KeyboardHelp.swift:147` (the tool list in Help)
- Line numbers in this plan are as of 2026-11-21 and drift as other sessions edit these files; the symbol named beside each is the reference.

## Prerequisites

The shared gate is in plan-52-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-52-D is complete | `ls planning/completed \| grep -c '^plan-52-D'` → 1 | NOT MET (2026-11-21: 0; A–D are not started) |

Everything below is written against the world where this holds.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update every status
> before you continue, and again before you decide to stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- A pour on a shown layer draws its filled copper (`copper.pourFills[pour.id].path`) in the layer's colour at opacity 0.5 (0.35 while the PCB
  fill shows), under that layer's traces and pads; a dashed outline of its rectangle in the layer's colour; a selected pour gets the
  selection ring. A pour with no net or no fill draws only its outline.
- A keep-out draws a red dashed outline and a diagonal hatch at opacity 0.15 while any layer it applies to is shown, and not in an image export
  (`exporting`); pours (copper) do draw in an export.
- Tools **Pour** and **Keep-out** (`PCBTool.pour`, `.keepout`): press, drag, release makes the item from the dragged rectangle
  (snapped to the trace grid; sides at least 0.5 mm; a click that does not drag makes nothing); a pour goes on `traceLayer`, a keep-out on every layer.
  While dragging, the rectangle is drawn as a rubber band.
- Select tool: a press on a pour's or keep-out's edge (within the click tolerance) selects it, the drag moves it (snapped by the board grid,
  as `moveBoardGroup` does); shift-box and Select All take items whose whole rectangle is in the box / on a shown layer; the Delete tool and ⌘⌫ delete them.
- Help lists the two tools.

### Non-goals (explicit constraints)

- No resize handles and no sidebar panel (plan-52-F); a pour's settings are the defaults of A § 4 until then.
- No Board Preview, no export, no DRC (F, G, H).
- A press inside a pour (not on its edge) does what it did before: pan or clear the selection. A pour never steals a click from another item.
- Planes' drawing, hit-testing and tools are unchanged.

## 2. Current State

- `draw` reads `document.connectivity()` only when a Plane is drawn (`planeCopper`); the layer loop draws the Plane tint, the traces
  (`drawTraces`, `:486`), then the side's pads.
- `boardItem(at:)` returns the first hit in the order text, via, hole, trace, footprint (`SchematicModel.swift:4378`).
- A tool press is decided in `begin(at:)` (`:801`): `.hole` places and returns `.moveGroup`; `.delete` calls `deleteBoardItem` and returns `.idle`.
  `handle(start:translation:)` (`:762`) runs each drag step; `end(translation:)` (`:858`) finishes (a trace is finished there, a group settled).
- The tool picker is `ForEach(PCBTool.allCases)` (`ContentView.swift:599`): a new case appears there with its `title`, `symbol` and `tooltip`.
- `KeyboardHelp.swift:147` has one `entry("Hole", …)` per tool.

### Measured populations

| What | Count | Command |
|---|---|---|
| tools today | 7 | `sed -n '1103,1115p' schematical/schematical/Canvas/PCBCanvasView.swift | grep -c "^    case "` → 7 (select, text, trace, via, hole, delete, route) |
| places the Help lists tools | 1 | `grep -n 'entry("Hole"' schematical/schematical/Help/KeyboardHelp.swift` → `:147` |

### Verified properties

- `ContentView` picks the tool from `PCBTool.allCases` with `.labelStyle(.iconOnly)` (`:599`–`:606`), so a new case needs only its three strings.
- `onPlaced` (`PCBCanvasView.swift:81`) is called after a Text is placed (`:836`) and nothing else; the new tools do not call it.

## 3. Design Overview

Layering: (1) document hit-testing and selection (`SchematicModel`), pure and tested without a view; (2) pure drawing helpers on `PCBCanvasView`
(`pourPaths`, `keepoutHatch`) tested by `Path.contains`; (3) the view: draw calls, the two tools, `Interaction.drawRect`.
**Hit rule:** distance from the point to the rectangle's boundary ≤ tolerance, on a shown layer (a keep-out: any applying layer shown); it is
the last item tried in `boardItem(at:)`, so every existing hit is unchanged. **Box select** takes an item whose rect lies wholly inside the box.
**Risk:** low; the only shared code is `begin`/`end`/`boardItem`, which have tests (`PadHoverTests`, `LockPositionTests`, `PCBUndoTests`).
Byte-identity is not a gate. The existing drawing must be pixel-unchanged for a board with no pours: guarded by the `guard !pours.isEmpty` in the new draw calls.
**Probed:** nothing beyond plan-52-C's measurements; no premise is unproven.

Rejected alternatives: *select a pour by pressing inside it* (it would take every press on a board-sized pour and break panning); *a
handle-based tool for drawing* (a drag is the way Via/Hole/Trace already work).

## 4. Detailed Design

- `SchematicDocument.pourItem(at:tolerance:visible:) -> BoardItem?` (new, in the Pours section): the last pour or keep-out whose boundary is
  within tolerance. Pours first then keep-outs, later items win. `boardItem(at:…)` returns it after the footprint.
- `boardItems(in:visible:)` (box) and `selectAllBoard` add `.pour`/`.keepout` for rects inside the box / on a shown layer.
- `deleteBoardItem(at:…)` falls through to `pourItem(at:)` after the existing checks.
- `PCBCanvasView.pourPaths(visible:board:copper:) -> [(pour: BoardPour, fill: Path)]` and `keepoutHatch(_ rect: CGRect, spacing: CGFloat) -> Path`
  (45° lines clipped to the rect, spacing 1 mm).
- `Interaction.drawRect(kind: PCBTool, from: CGPoint, to: CGPoint)` (board mm).
- Symbols: `rectangle.fill` (Pour) and `rectangle.badge.xmark` (Keep-out); a test checks both exist.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work; `- [~]` for partial; mark a moot
> task `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands. An unticked box means NOT DONE.

### Phase 1 — Hit-testing, box select, Select All, delete

- [ ] `SchematicModel.swift`: `pourItem(at:tolerance:visible:)`; call it last in `boardItem(at:…)` and in `deleteBoardItem(at:…)`; add pours and
      keep-outs to `boardItems(in:visible:)` (rect inside the box, layer shown) and `selectAllBoard`; `deleteBoardSelection` already handles them (A).
- [ ] Tests: new `schematicalTests/PourCanvasTests.swift`: (a) a press on a pour's edge hits it, 2 mm inside does not, on a hidden layer does not; (b) a
      footprint inside a pour is hit before the pour's edge when both are within tolerance; (c) box select takes a pour wholly inside, not one cut by the
      box; (d) Select All takes pours and keep-outs on shown layers; (e) `deleteBoardItem(at:)` on an edge deletes the pour, one undo step "Delete" restores it;
      (f) a keep-out on `[.bottom]` only is not hit with Bottom hidden; (g) a group drag moves a selected pour by the snapped delta and leaves a locked one.

Acceptance: (a)–(g) hold; existing hit tests unchanged.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/PourCanvasTests -only-testing:schematicalTests/LockPositionTests -only-testing:schematicalTests/PadHoverTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 7 min).
Commit: —

### Phase 2 — Drawing

- [ ] `PCBCanvasView.swift`: `planeCopper` also true when `!document.pours.isEmpty`; in the layer loop, after the Plane tint and before `drawTraces`,
      draw that layer's pours from `pourPaths`: fill at 0.5 / 0.35 opacity, dashed outline (`lineWidth 1/scale`, dash `[4/scale, 3/scale]`) unless `exporting`.
- [ ] After the copper loop, unless `exporting`, draw each keep-out whose layers include a shown layer: red (`Color.red`) dashed outline and `keepoutHatch` at opacity 0.15.
- [ ] The selection ring (the `switch` over `BoardItem` in `draw`, where `.hole` draws its ring): `.pour`/`.keepout` stroke the rect outset by `margin` in the accent colour (replaces A's placeholder).
- [ ] Tests (`PourCanvasTests`): (h) `pourPaths` for a ground pour over a ground pad: the path contains a point 5 mm from every other-net pad and not the point inside an
      other-net pad; (i) `pourPaths` is empty for a pour on a hidden layer and a pour's fill is empty with no net; (j) `keepoutHatch` is inside its rect and non-empty;
      (k) the image export (`ImageExport.data(for:_:)` on the PCB page; `SchematicExportActionTests` builds the options) of a board with a Top ground pour differs from the same board without it, of a board with only a keep-out equals the board without it (keep-outs are not exported), and of a board where a pour was added then undone equals the board without it byte for byte.

Acceptance: (h)–(k) hold.
  Check: `xcodebuild … test -only-testing:schematicalTests/PourCanvasTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

### Phase 3 — The two tools

- [ ] `PCBCanvasView.swift`: `PCBTool.pour`, `.keepout` (titles "Pour", "Keep-out"; tooltips "Pour — drag a rectangle to flood it with copper on the Trace layer; it keeps its
      clearance from other nets" and "Keep-out — drag a rectangle the pours stay out of"; symbols above); `Interaction.drawRect`; `begin(at:)` returns it for the two tools (and calls `document.beginGesture`
      as every press does); `handle` updates `to`; `end(translation:)` makes the item when the drag is at least 3 points and each side at least `BoardPour.minimumSide`: `document.addPour(rect:layer: traceLayer)` or `addKeepout(rect:layers: nil)`;
      the rubber band is drawn in `draw` while `interaction` is `.drawRect`.
- [ ] `Help/KeyboardHelp.swift`: two `entry` lines after Hole's (`:147`).
- [ ] Tests: `PourCanvasTests` (l) the tool strings are non-empty and both SF Symbols exist (`NSImage(systemSymbolName:accessibilityDescription:)` non-nil); (m) `KeyboardHelpTests`
      still passes with the two entries (it lists every tool); a view-level test of `handle`/`end`: drive `PCBCanvasView.handle(start:translation:)` then `end` as `PadHoverTests` does for Select
      (n) a 40 × 30 mm drag with the Pour tool makes one pour of that rect on `traceLayer`, one undo step "Add Pour"; (o) a click makes none; (p) the Keep-out tool makes a keep-out with `layers == nil`.

Acceptance: (l)–(p) hold.
  Check: `xcodebuild … test -only-testing:schematicalTests/PourCanvasTests -only-testing:schematicalTests/KeyboardHelpTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 7 min).
Commit: —

## Validation Plan

- Tests: `PourCanvasTests` (new, 16 cases), `LockPositionTests`, `PadHoverTests`, `KeyboardHelpTests`, `PCBUndoTests` (existing paths).
- Coverage check: `grep -c "pour" schematical/schematical/Canvas/PCBCanvasView.swift` → at least 12; the tool and draw paths are all reached by cases (h)–(p).
- Runtime proof: once, in plan-52-H (the real app, after F's panel exists).
- Doc sync: none here (`docs/file-format.md` is B's; the PCB page has no user manual file).
- Final gate (once, after Phase 3): `xcodebuild … test -only-testing:schematicalTests/PourCanvasTests -only-testing:schematicalTests/PourConnectivityTests -only-testing:schematicalTests/KeyboardHelpTests -only-testing:schematicalTests/LockPositionTests -only-testing:schematicalTests/PlaneShadingTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 9 min).

## Decisions

- Pours and keep-outs are made by dragging a rectangle with two new tools — decided by the owner, 2026-11-21 ("a rectangular region").
- Items are selected by their edge — decided by the author: a pour is as big as a board and would take every pan otherwise.
- A pour is drawn at 0.5 opacity in its layer's colour with a dashed outline; a keep-out red and hatched — decided by the author: pours are copper (they draw in an export); keep-outs are design marks (they do not).
- The tool picker gets nine segments — decided by the author: it is `ForEach(PCBTool.allCases)`; no layout work.

## Corrections

(Filled in during execution.)

## Summary

Low risk, visible: the only shared code is the hit order and the press/release path, both covered by existing tests. Untouched: Planes, the
sidebar, exports, the DRC.
