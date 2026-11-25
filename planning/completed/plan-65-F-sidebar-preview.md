# plan-65-F: Sidebar Panels, Reset to Rectangle and the Board Preview

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-65-E (complete)

A cutout can be drawn and moved (E) but has no panel, a free-form board still shows editable Width and Height fields that do nothing (B), and the 3D board preview draws the board as a solid slab.
This sub-plan adds the Cutout panel (position, size, radius, lock), the free-form board's read-only size and a Reset to Rectangle button, and makes the board preview (front, back, angled) show holes and the free-form outline.

The single outcome: selecting a cutout shows its panel and its fields change it as undo steps; a free-form board's Board panel shows its size read-only with a working "Reset to Rectangle"; the front and angled renders show the cutout as a hole through the board with its inside wall.

References:

- plan-65-A § 4.4; plan-65-B § 4.2 (`resetBoardOutline`, `isFreeForm`); plan-65-E
- `Canvas/PCBSidebarView.swift`: the item enum (`:30`), the selected item (`:53`), the panels (`:102`), `BoardPanel` (`:442`–`:462`); `Canvas/PourSection.swift`: `KeepoutPanel` (`:243`), `LockToggle`; `Canvas/LengthField.swift`
- `BoardPreview/BoardRenderer.swift`: `boardShape` (`:290`), the shadow (`:307`), `walls` (`:170`) and its calls (`:312`–`:314`), `drawSurface` (`:319`); `RenderProjection.extent` (`:115`)
- `schematicalTests/BoardPreviewTests.swift` (`pixels`, `testAPourShowsInTheFrontRenderOnly`), `PourPanelTests.swift`, `BoardDetailsSheetTests.swift`

Test command form (`XB`): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p65-dd CODE_SIGNING_ALLOWED=NO test`;
a Check below writes `XB -only-testing:schematicalTests/<Class>`. Debug, scoped to the classes named.

## Prerequisites

The shared gate is in plan-65-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-65-E is complete | `ls planning/completed \| grep -c '^plan-65-E'` → 1 | MET (2026-11-25: E landed, 1) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update every status
> before you continue, and again before you decide to stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- Selecting one cutout shows `CutoutPanel`: a Lock toggle; X and Y (bounds' top-left); for a rounded cutout Width, Height and Radius (fields, precise, board units) and a kind line (`Rectangle`, `Circle`, `Slot`); for a polygon a read-only `Polygon, N points` and its Width and Height read-only.
- The Board panel of a free-form board shows Width and Height as read-only text and a `Reset to Rectangle` button (one undo step, `resetBoardOutline`).
- The board preview (Front, Back, the angled views) draws the board face with every cutout a hole, the board's shadow and edge walls round the outline and round every cutout (a hole's wall on the side facing into the hole).
- The selection list row text and the multi-selection panels handle `.cutout` (A did the row; this adds `MultiPositionSection`'s Lock for it).

### Non-goals (explicit constraints)

- No new tool or canvas change (E); no export (G); no polygon vertex editing.
- No change to how a board without cutouts looks in the preview or in the sidebar: the existing preview and panel tests pass unedited.

## 2. Current State

- `PCBSidebarView` chooses a panel from the selected item (`:30`–`:105`): `KeepoutPanel(document:keepout:)` for a sole keep-out. After E a sole cutout hits the minimal no-op case A added.
- `KeepoutPanel` (`PourSection.swift:243`): `LockToggle`, `LengthField(title:millimetres:units:precise:)` for X, Y, Width, Height calling `setKeepoutRect`, disabled while locked, `Fit to Board`.
- `BoardPanel` (`PCBSidebarView.swift:442`–`:462`): a fixed-size card shows `LabeledContent("Width")` text; otherwise a `LengthField` calling `setBoardSize`; Height is a field for Custom and Custom Card, else read-only text.
- `BoardRenderer.draw` fills `boardShape = Path(addLines(outline))` for the shadow and the surface (`:290`, `:307`, `:319`) and calls `painter.walls(outline, …)` three times for the edge (`:312`–`:314`); `walls` draws only walls whose outward normal faces the viewer, the outward side taken from the polygon's own on-screen winding (`:170`–`:195`).

### Measured populations

| What | Count | Command |
|---|---|---|
| lines in `BoardRenderer.swift` that mention `outline` or `boardShape` (comments included) | 19 | `rg -n "outline\|boardShape" schematical/schematical/BoardPreview/BoardRenderer.swift \| wc -l` → 19; the ones that change are in `draw` (`:289`–`:290`, `:307`, `:312`–`:314`, `:319`) |
| places a `.cutout` selection must reach in the sidebar | 2: the sole-selection panel (`KeepoutPanel` use, `PCBSidebarView.swift:115`) and `MultiPositionSection` (`:121`) | `rg -n "KeepoutPanel\|MultiPositionSection" schematical/schematical --glob '*.swift'` → those two uses and the two definitions |
| the test this mirrors | `PourPanelTests.testTheSidebarShowsAPourOrAKeepoutWhenItIsTheSoleSelection` | `rg -n "func test" schematical/schematicalTests/PourPanelTests.swift` → `:27` |

### Verified properties

- `walls(polygon, …)` picks "outward" from the polygon's screen winding, so passing a cutout loop reversed makes its visible walls the ones that face into the hole (the visible side of a hole's wall is the one a viewer looks at from above through the hole).
- `walls` reads the winding from each polygon itself, so the renderer reverses a cutout loop exactly when its y-down shoelace sign equals the outline's (a DXF polygon can be either winding; `BoardCutout.Shape.polygon` is clockwise by A § 4.1, so for the shapes the app makes the rule is "always reverse").

## 3. Design Overview

- **Panel.** `CutoutPanel(document:cutout:)` in `Canvas/CutoutSection.swift`, mirroring `KeepoutPanel`. Rounded: X, Y, Width, Height call `setCutoutRect`; Radius calls `setCutoutRadius` and shows the radius in the board units (max half the short side). Polygon: X and Y call `moveCutout(_:to:)`; Width and Height are `LabeledContent` text. Locked disables all fields. No Fit to Board (a cutout is not the board's size).
- **Board panel.** When `board.isFreeForm`, Width and Height are `LabeledContent` text (`Fixed by the board outline`) and a `Button("Reset to Rectangle")` calls `resetBoardOutline()`; Height's existing `type == .custom` branch yields to this one.
- **Preview.** `boardShape` is `Path(edge.path)`; the shadow and the surface fill use `FillStyle(eoFill: true)`; the three `walls` calls run for the outline and, per cutout loop, for the loop reversed. `RenderProjection.extent` and `fitting` keep using the outline's bounds.

Risk: the preview's wall drawing order for a hole (the hole's far wall is under the surface plane; the surface face is drawn after the walls, with its hole). The test below checks a pixel, and J looks at the angled render in the real window.

Byte-identity is not a gate; the no-cutout check is the existing preview tests. Rejected: drawing a cutout's walls with a separate routine (`walls` already takes any polygon).

## Phases

> **NOTE — keep the checkboxes current as you go. This is not bookkeeping; it is the only way anyone can see where the work actually is.**
>
> - Tick `- [x]` **in the same commit as the work it describes**; use `- [~]` for partially done and say what remains; mark a moot task `- [x] ~~text~~ — moot: <evidence>`; fill the phase's `Commit:` line with the hash(es) the moment it lands; add any task you discover you needed.
>
> **An unticked box means NOT DONE.**

### Phase 1 — Panels

- [x] `Canvas/CutoutSection.swift` (new): `CutoutPanel`; `PCBSidebarView.swift`: the `.cutout(UUID)` item case, `selectedCutoutID` (beside `selectedKeepoutID`, `PourSection.swift:94`), the panel route, the multi-selection Lock.
- [x] `PCBSidebarView.swift` `BoardPanel`: the free-form branch of § 3.
- [x] Tests `schematicalTests/CutoutPanelTests.swift`: for a 10 × 4 rounded cutout the panel's setters (the closures the fields call, as `PourPanelTests` tests the keep-out panel's) change rect and radius as named undo steps and refuse a radius over 2 mm by clamping; a locked cutout's setters change nothing; a polygon's X/Y move it and its size is unchanged; `BoardPanel`'s reset on a free-form board gives a rectangle of the old bounds and one undo step restores the polygon; on a plain board no reset button is offered (the visibility predicate the view reads, tested as a static function).

Acceptance: every panel field changes the model as one named undo step and the free-form reset works.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/CutoutPanelTests -only-testing:schematicalTests/PourPanelTests -only-testing:schematicalTests/BoardDetailsSheetTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 4 min; the two existing classes are the no-cutout check for the panels).
  Observed: CutoutPanelTests 5 + PourPanelTests 5 + BoardDetailsSheetTests 4, 0 failures, TEST SUCCEEDED.
Commit: 18d5f400

### Phase 2 — The board preview

- [x] `BoardPreview/BoardRenderer.swift`: `boardShape` from `document.boardEdge.path`; the shadow and surface fills even-odd; walls for each cutout loop reversed, with the same three tints as the outline.
- [x] Tests `schematicalTests/BoardPreviewTests.swift` (add): a 60 × 40 board with a 20 × 20 cutout at its middle rendered Front (`ImageRenderer`, as `pixels` does) has a centre pixel that differs from the same pixel of the render without the cutout by at least 100 in the sum of the channel differences (the board colour against the backdrop), and a pixel 3 mm outside the cutout that is identical between the two renders; an angled render of the same document completes and differs from the render without the cutout.

Acceptance: the preview shows a hole; existing preview tests pass unedited.
  Check: `XB -only-testing:schematicalTests/BoardPreviewTests -only-testing:schematicalTests/CutoutPanelTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 5 min; `BoardPreviewTests` also renders boards without cutouts).
  Observed: BoardPreviewTests 13 (2 new) + CutoutPanelTests 5, 0 failures, TEST SUCCEEDED.
Commit: 261ac03e

## Validation Plan

- Tests: `CutoutPanelTests`, the new `BoardPreviewTests` cases; negative: locked, polygon size read-only, no reset on a plain board.
- Coverage check: the new panel is reached from `PCBSidebarView`, which `PourPanelTests` already builds for keep-outs.
- Runtime proof: J opens the app, selects a cutout, edits it and looks at the angled preview.
- Doc sync: none (the sidebar is not documented in `docs/`).
- Final gate (run ONCE, after Phase 2): the Phase 2 check plus `-only-testing:schematicalTests/CutoutCanvasTests` (est. 6 min). Observed: BoardPreviewTests 13, CutoutPanelTests 5, CutoutCanvasTests 10 (28 in all), 0 failures, `** TEST SUCCEEDED **`.

## Decisions

- A polygon cutout is not resizable and has no vertex editing — decided by the owner, 2026-11-25. Settles the polygon panel.
- A free-form board's size is read-only with a Reset to Rectangle button — decided by the author, 2026-11-25. Settles § 3.

## Corrections

- 2026-11-25: `MultiPositionSection` needed no change (it locks by `isLocked`/`setLocked`, which A made handle `.cutout`), so that task is done by A's work: `PCBSidebarView.swift` only got the `.cutout` content, `selectedCutoutID` and the panel route.
- 2026-11-25: the reset button's "visibility predicate" is `BoardSettings.isFreeForm` itself (the view reads it directly); `CutoutPanelTests.testResetToRectangleOnAFreeFormBoard` checks it is false on a plain board and true on a free-form one.
- 2026-11-25: the panel's field closures are `CutoutPanelActions` (in `CutoutSection.swift`), so tests call what the fields call; the plan did not name that type.

## Summary

Risk: the hole's wall drawing in the angled view, checked by a pixel test and by J's real look. Left untouched: the canvas, the exports, the file.
