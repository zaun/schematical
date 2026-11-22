# plan-52-F: Resize, the Sidebar Panels and Board Preview

Last updated: 2026-11-21
Effort: medium (1h–2h)
Depends on: plan-52-E (complete)

After plan-52-E a pour exists, draws and moves, but its size is whatever was dragged and its settings are the defaults. This
sub-plan finishes the editing: eight resize handles on the one selected pour or keep-out, a sidebar panel for each with every
setting (layer, net, clearance, connection, thermal gap, spoke width, minimum width, position and size, Fit to Board, lock), and
the pour's copper in Board Preview.

The single outcome: select a pour, drag a handle to resize it, pick its net and layer in the sidebar, switch it to solid or change
the thermal gap, and see the fill follow each change in the canvas and in Board Preview; every change is one undo step.

References:

- plan-52-A § 4 (settings and ranges), plan-52-E § 4
- `schematical/schematical/Canvas/PCBSidebarView.swift`: `Content` (`:20`), `content(for:)`, `body` switch, `LockToggle` (`:119`), `HolePanel` and
  its `netPicker` (`:739`–`:810`), `Canvas/StackupSection.swift` (the precedent for a testable section file)
- `Canvas/SchematicModel.swift`: `selectedHoleID` (`:1531`), `planeNetChoices` (`:3067`)
- `BoardPreview/BoardRenderer.swift`: `drawSurface` (`:348`), `drawPlane` (`:473`); `BoardPreviewTests.testOpenViaChangesTheRender` (the pixel-compare precedent)
- `schematicalTests/SelectionSidebarTests.swift` (tests `PCBSidebarView.content(for:)`)
- Line numbers in this plan are as of 2026-11-21 and drift as other sessions edit these files; the symbol named beside each is the reference.

## Prerequisites

The shared gate is in plan-52-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-52-E is complete | `ls planning/completed \| grep -c '^plan-52-E'` → 1 | MET (2026-11-21: plan-52-E landed as cc4d5278) |

Everything below is written against the world where this holds.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update every status
> before you continue, and again before you decide to stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- With exactly one unlocked pour or keep-out selected, the canvas draws eight handles (corners and edge midpoints, 8 points square on screen);
  a press on a handle starts a resize drag that changes the rectangle (snapped to the trace grid, each side at least 0.5 mm, the opposite edges fixed)
  as one undo step "Resize".
- `PCBSidebarView.Content` has `.pour(UUID)` and `.keepout(UUID)`, chosen when one is the sole selection; the panels:
  - **Pour**: Lock Position; X, Y, Width, Height; Layer (the board's trace layers); Net (None, GND, rails, signal nets, "Missing net"; as a hole's); Clearance
    (blank = the DRC minimum; a field with the range 0.05–10); Connection (Thermal Relief / Solid); Thermal Gap and Spoke Width (shown for Thermal);
    Minimum Width; Fit to Board; a read-only line "Fill: N pieces, M pins joined" (or "No fill: choose a net" with no net).
  - **Keep-out**: Lock Position; X, Y, Width, Height; Layers: an "All layers" switch and one switch per board copper layer; Fit to Board.
- Board Preview draws each pour on the side's layer in the under-mask copper tint, with its fill (clearances, reliefs) cut out, as it does a Plane.

### Non-goals (explicit constraints)

- Inner-layer pours are not drawn in Board Preview (it shows only Top and Bottom copper, as it does today for inner Planes' absence).
- No reorder (priority) control, no copy/paste of pours, no "convert Plane to pour".
- Panels follow `HolePanel`'s conventions; no new view infrastructure (`LengthField`, `LockToggle`).

## 2. Current State

- `PCBSidebarView.Content` lists `board, footprint, text, trace, via, hole, selection` (`:20`); `content(for:)` returns a case for a sole selected item
  of each kind else `.selection`/`.board`; `body` switches over it.
- `HolePanel` (`:739`) is the model: a `Section` of `LengthField`s (`LengthField(title:millimetres:units:precise:)` with a setter that returns the clamped value),
  a `Picker` for the net built from `document.planeNetChoices()` with a "Missing net" entry (`netPicker`, `:790`).
- `BoardRenderer.drawSurface` draws, for the side's copper layer, the Plane tint (`drawPlane`, `:473`) then each trace in `underMask`; `BoardPreviewTests.testOpenViaChangesTheRender`
  compares rendered pixels.

### Measured populations

| What | Count | Command |
|---|---|---|
| `Content` cases | 7 | `sed -n 20,29p schematical/schematical/Canvas/PCBSidebarView.swift \| grep -c "case "` → 7 (board … selection) |
| tests that read `PCBSidebarView.content` | 3 files | `grep -rln "PCBSidebarView.content\|SidebarContent" schematical/schematicalTests` → ViaTests, SelectionSidebarTests, TraceToolTests |

### Verified properties

- `selectedHoleID` is `boardSelection.sole` matched on `.hole` (`SchematicModel.swift:1531`); the same two lines give `selectedPourID`/`selectedKeepoutID`.
- `planeNetChoices()` is the list a Plane's Net picker offers (ground, rails in use, signal nets), the one a pour uses (plan-52-A decided).

## 3. Design Overview

Three independent pieces: resize (document function + one `Interaction` case + handle drawing), panels (`Canvas/PourSection.swift`: two views and testable helpers), and Board Preview (one
draw call). Risk is low: UI over setters plan-52-A tested. The one subtlety is the resize drag: the fill is recomputed each frame while a pour changes (plan-52-D's rule, test (i)), at 24–275 ms.
Byte-identity is not a gate. A board with no pours must render in Board Preview exactly as before (test).
**Probed:** none needed; no premise is unproven.

Rejected alternatives: *edge-only resize (4 handles)* (corners are what people reach for); *a modal pour dialog* (the sidebar is where every other item's settings live).

## 4. Detailed Design

- `enum PourHandle: CaseIterable { case topLeft, top, topRight, right, bottomRight, bottom, bottomLeft, left }` with `position(in rect: CGRect) -> CGPoint` and `func resized(_ rect: CGRect, to point: CGPoint) -> CGRect`
  (moves the edge or corner the handle names, clamps to 0.5 mm, never inverts).
- `SchematicDocument.pourHandle(at:tolerance:) -> (item: BoardItem, handle: PourHandle)?`: for the sole selected, unlocked pour/keep-out.
  `resizePourItem(_ item: BoardItem, handle: PourHandle, to point: CGPoint)`: snap `point` to the trace grid, apply `resized`, record "Resize" (the gesture makes it one step).
- `Interaction.resize(item: BoardItem, handle: PourHandle)`; `begin(at:)` (Select tool) checks `pourHandle(at:)` before `boardItem(at:)`.
- `Canvas/PourSection.swift`: `PourPanel`, `KeepoutPanel` (SwiftUI), `PourSection.layerChoices(_ board: BoardSettings) -> [CopperLayer]` (= `board.traceLayers`), `PourSection.fillSummary(_ pour: BoardPour, copper: BoardConnectivity) -> String`.
- Fit to Board: sets the rect to the bounds of `board.outline`, for a pour and a keep-out alike; a pour's fill still stays back from the edge by the copper-to-edge rule.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work; `- [~]` for partial; mark a moot
> task `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands. An unticked box means NOT DONE.

### Phase 1 — Resize handles

- [x] `Canvas/PourSection.swift` (new): `PourHandle` as § 4. `SchematicModel.swift`: `pourHandle(at:tolerance:)` and `resizePourItem(_:handle:to:)`; `PCBCanvasView.swift`: `Interaction.resize`, `begin(at:)`
      (Select tool: a handle first), `handle(start:translation:)` calls `resizePourItem` with the pointer's board point, the handles drawn in `draw` for the sole selected unlocked item
      (white fill, accent stroke, 8 pt / scale).
- [x] Tests: `PourCanvasTests` (q) for each of the 8 handles, `resized` moves exactly the edges it names and keeps the others; (r) a drag past the opposite edge clamps at 0.5 mm; (s) `pourHandle(at:)`
      is nil for a locked or multiply selected item; (t) one resize drag (begin gesture, several `resizePourItem` calls, end) is one undo step named "Resize" and undo restores the rect.

Acceptance: (q)–(t) hold.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/PourCanvasTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min). Observed (all three phases' suites in one run, with `SelectionSidebarTests`, `ViaTests`, `TraceToolTests`, `PourConnectivityTests`, `PourModelTests`): `Executed 96 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: COMMIT_F

### Phase 2 — The panels

- [x] `SchematicModel.swift`: `selectedPourID`, `selectedKeepoutID`, `fitPourToBoard(_:)`, `fitKeepoutToBoard(_:)` ("Fit to Board" undo names).
- [x] `PCBSidebarView.swift`: `Content.pour(UUID)`, `.keepout(UUID)`, chosen in `content(for:)` after the hole; `body` cases show `PourPanel`/`KeepoutPanel` with `.id(…)`.
- [x] `PourSection.swift`: `PourPanel` (fields of § 1; Thermal Gap and Spoke Width only while `connection == .thermal`; Clearance blank means nil; a net the schematic no longer has is "Missing net"
      as `HolePanel`'s) and `KeepoutPanel` ("All layers" switch sets `layers = nil`; a layer switch with "All" on first turns it into the explicit set minus that layer); `fillSummary`.
- [x] `BoardDetailsSheet`/NetsSidebar: no change.
- [x] Tests: new `schematicalTests/PourPanelTests.swift`: (u) `content(for:)` is `.pour(id)` / `.keepout(id)` for a sole selection and `.selection` for two; (v) `layerChoices` is the trace layers (no Plane layer);
      (w) `fillSummary` for a ground pour over two ground pads is exactly "Fill: 1 piece, 2 pins joined" (plural: "2 pieces", "1 pin") and "No fill: choose a net" without a net; (x) the Fit to Board result equals the outline bounds and is one undo step;
      (y) the keep-out layer-switch logic: All on → switch Top off gives `[bottom]` on a 2-layer board; all switches on again gives `nil`.

Acceptance: (u)–(y) hold.
  Check: `xcodebuild … test -only-testing:schematicalTests/PourPanelTests -only-testing:schematicalTests/SelectionSidebarTests -only-testing:schematicalTests/ViaTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min). Observed: as Phase 1.
Commit: COMMIT_F

### Phase 3 — Board Preview

- [x] `BoardRenderer.swift` `drawSurface` (`:348`): after the Plane tint and before the traces, draw each pour on the side's layer from `document.connectivity().pourFills` in `underMask` (fill only; no outline).
- [x] Tests: `BoardPreviewTests` (z) a Top ground pour over a ground pad changes the front render (`differing(before, after) > 1000`, as the open-via test does) and leaves the back render within `change / 10`;
      (aa) a document with no pour renders byte-equal before and after a pour is added and deleted.

Acceptance: (z) and (aa) hold.
  Check: `xcodebuild … test -only-testing:schematicalTests/BoardPreviewTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min). Observed: as Phase 1.
Commit: COMMIT_F

## Validation Plan

- Tests: `PourCanvasTests` (extended), `PourPanelTests` (new), `BoardPreviewTests`, `SelectionSidebarTests`, `ViaTests`.
- Coverage check: `grep -c "PourPanel\|KeepoutPanel" schematical/schematical/Canvas/PCBSidebarView.swift` → at least 2 (both reachable from `content(for:)`, which (u) tests).
- Runtime proof: once, in plan-52-H.
- Doc sync: none here.
- Final gate (once, after Phase 3): `xcodebuild … test -only-testing:schematicalTests/PourCanvasTests -only-testing:schematicalTests/PourPanelTests -only-testing:schematicalTests/BoardPreviewTests -only-testing:schematicalTests/SelectionSidebarTests -only-testing:schematicalTests/PourConnectivityTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 10 min).

## Decisions

- Every pour setting (layer, net, clearance, thermal or solid, gap, spoke width, minimum width) is editable in the sidebar — decided by the author, from the owner's "thermal reliefs" and "keep the buffer" (2026-11-21).
- Keep-out layers are an "All layers" switch plus one per layer — decided by the author: nil-means-all in the file maps to one switch.
- Inner-layer pours are not in Board Preview — decided by the author: the preview draws Top and Bottom only.

## Corrections

- (aa) (a board with no pour renders as before) is the last assertion of `BoardPreviewTests.testAPourShowsInTheFrontRenderOnly` (added then deleted: within a tenth of the change), not a separate test; the preview suite compares amounts, not bytes, because two renders of one picture can differ by antialiasing (`BoardPreviewTests.differing`).
- The resize handles were first drawn in an image export too and changed its bytes; they are drawn only when `!exporting`, like the keep-out marks (`PourCanvasTests.testImageExportShowsPoursButNotKeepouts` caught it).
- `LockToggle` was `private` to `PCBSidebarView.swift`; the panels live in `PourSection.swift`, so it is now internal (no other change).
- `PourCanvasTests` now has 18 cases (14 from plan-52-E and four resize cases); `PourPanelTests` has 5 (`grep -c "func test"` → 18, 5).

## Summary

Low risk: UI over tested setters. The resize drag recomputes the fill per frame, which plan-52-D allows and measured at 24–275 ms. Untouched: exports, the DRC, automation.
