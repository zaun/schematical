# plan-50-B: Stackup Section in the PCB Sidebar, and One Board Finish

Last updated: 2026-11-16
Effort: medium (1h–2h)
Depends on: plan-50-A (complete)

plan-50-A put a `BoardStackup` on the board and made its edits undoable. This sub-plan lets the user see and
edit it, and removes the second, disconnected "Finish" choice: Board Preview and Image export each have their own
HASL / ENIG menu today; after this plan the finish is the board's, set once in the stackup, and both follow it.

The single outcome: the PCB sidebar, with nothing selected, has a Stackup section that edits every stackup value
through the plan-50-A setters; Board Preview and the Front / Back Render images draw bare copper in the board's
finish (HASL and lead-free HASL silver, ENIG gold, OSP bare copper) and have no Finish menu of their own.

References:

- plan-50-A (`planning/plan-50-A-stackup-model.md`) — the model, the setters, the standard values
- `schematical/schematical/Canvas/PCBSidebarView.swift` (the `.board` case at lines 93–106, `LayersSection`, `LengthField`)
- `schematical/schematical/BoardPreview/BoardRenderer.swift`, `BoardPreviewWindow.swift`
- `schematical/schematical/Document/ImageExport.swift`, `ImageExportDialog.swift`; `docs/export.md` § Image

## Prerequisites

The shared gate is in plan-50-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-50-A is complete | `ls planning/completed \| grep -c '^plan-50-A'` → 1 | MET (2026-11-16: 1) |
| the setters B calls exist | `grep -n "func setStackupFinish\|func setStackupTemperatureRise\|func setCopperThickness\|func setDielectric\|func resetStackup" schematical/schematical/Canvas/SchematicModel.swift \| wc -l` → 5 | MET (2026-11-16: 5) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update
> every status before you continue, and again before you decide to stop. If you stop, report the current status
> of *all* prerequisites.

## 1. Goal

- The PCB sidebar (nothing selected) has a **Stackup** section between **Layers** and **Planes**:
  - **Finish** menu: HASL (leaded), HASL (lead-free), ENIG, OSP.
  - **Temperature rise** field, °C (1–100), "for the current estimates".
  - One row per copper layer, Top first: `<Layer title> copper`, a weight menu (½ oz, 1 oz, 2 oz, Custom) and a
    thickness field in µm. The menu shows the weight the thickness matches (± 0.1 µm) and Custom otherwise.
  - One row between each two copper layers (one for a 1-layer board): a Core / Prepreg menu, a thickness field in
    the board's units, and an Er field.
  - A read-only **Thickness** row (`BoardStackup.thickness`, with "includes solder mask") and a **Reset to
    Standard** button (`resetStackup`).
- Board Preview draws bare pads in the board's finish; its toolbar has no Finish menu.
- Front / Back Render images use the board's finish; the Image export sheet has no Finish menu.

### Non-goals (explicit constraints)

- Stackup values and their ranges are plan-50-A's. This plan only adds a view and the finish wiring.
- The 3-D preview's slab thickness stays `BoardRenderer.thickness = 1.6`; it does not follow the stackup.
- The solder mask colour (`board.color`) stays where it is.
- Saved files are unchanged by this plan. The Image export's remembered options lose their `finish` key (see § 3).

## 2. Current State

- `BoardRenderFinish` (`.hasl`, `.enig`) is `BoardPreview/BoardRenderer.swift:29`. It is used in 4 files, 7
  references (`grep -rn --include='*.swift' "BoardRenderFinish" schematical/schematical schematical/schematicalTests | wc -l` → 7): `BoardRenderer.swift`
  (the `finish` property, `finishTint = finish == .enig ? Tint.gold : Tint.tin` at `:298`),
  `BoardPreviewWindow.swift` (a `@State private var finish` and a toolbar `Picker` at `:40`, `:66`, a
  `canvas(…, finish:)` parameter), `ImageExport.swift` (`ImageExportOptions.finish` at `:105`, read at `:133`,
  passed at `:199`) and `ImageExportDialog.swift:49`.
- One test passes a finish: `BoardPreviewTests.swift:41` (`finish: .enig`).
- Sidebar sections come from `PCBSidebarView`'s `.board` case: `GridSection`, `BoardPanel`, the card sections,
  `LayersSection`, `PlanesSection`, `NetSections` (`Canvas/PCBSidebarView.swift:93`–`:104`). Length fields use
  `LengthField(title:millimetres:units:precise:commit:)` (`Canvas/LengthField.swift:7`): `commit` takes the typed
  value in mm and returns the stored one.
- Tints: `Tint.tin`, `.gold`, `.solder`, `.hole` (`BoardRenderer.swift:68`–`:72`).

### Measured populations

| What | Count | Command |
|---|---|---|
| references to `BoardRenderFinish` | 7, in 4 files | `grep -rn --include='*.swift' "BoardRenderFinish" schematical/schematical schematical/schematicalTests \| wc -l` → 7 |
| tests that pass a finish | 1 | `grep -rn --include='*.swift' "finish:" schematical/schematicalTests` → `BoardPreviewTests.swift:41` |

### Verified properties

- **Removing `finish` from `ImageExportOptions` is safe for stored options.** Options are read through
  `value("finish", o.finish)`-style lookups (`ImageExport.swift:133`), which take a default for a missing key and
  ignore an extra one, so a stored `finish` from an older build is simply never read.

## 3. Design Overview

`SurfaceFinish` (plan-50-A) replaces `BoardRenderFinish` everywhere. `BoardRenderer.finish` is read from
`document.board.stackup.finish` by its callers; the tint is `.hasl, .leadFreeHasl → Tint.tin`, `.enig →
Tint.gold`, `.osp → Tint.bareCopper` (`Tint(0.72, 0.45, 0.30)`, new). `BoardPreviewView.canvas(document:face:angle:)`
loses its `finish:` parameter and reads the board's.

The sidebar section is a new file `Canvas/StackupSection.swift` (`struct StackupSection: View`), with its small
pure helpers in the same file so they can be tested without a view:

- `CopperWeight`: `.half = 0.0175`, `.one = 0.035`, `.two = 0.07` mm; `CopperWeight(thickness:)` matches within
  0.0001 mm; `title` is "½ oz", "1 oz", "2 oz".
- `StackupRow` / `StackupRows.rows(for: BoardStackup, layers: [CopperLayer]) -> [StackupRow]`: the ordered list
  the section draws (`.copper(index, title)`, `.dielectric(index)`), so the row order and titles are tested.

**Risk.** Low. The only behaviour change is the finish: the preview used to start on HASL regardless of the
board; it now draws the board's (lead-free HASL by default, which renders as the same tin tint). Tests, not
byte-identity, are the gate; the expected diff is the render of an ENIG or OSP board.

**Rejected.** *Keeping a preview-local Finish menu that defaults to the board's*: two places to disagree, which
the owner declined. *Following the stackup thickness in the 3-D slab*: separate visual change, no use for it yet.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work. `- [~]` for
> partial, with one line on what remains. A moot task is `- [x] ~~text~~ — moot: <evidence>`. Fill the phase's
> `Commit:` line the moment it lands. An unticked box means NOT DONE.

### Phase 1 — The finish becomes the board's

- [x] `BoardPreview/BoardRenderer.swift`: delete `BoardRenderFinish`; `BoardRenderer.finish` is `SurfaceFinish`;
      `finishTint` per § 3; add `Tint.bareCopper`.
- [x] `BoardPreview/BoardPreviewWindow.swift`: remove the `@State finish`, the toolbar `Picker("Finish")` and the
      `finish:` parameter of `canvas(…)`; both `canvas` and `saveImage` pass `document.board.stackup.finish` to
      `BoardRenderer`.
- [x] `Document/ImageExport.swift` and `ImageExportDialog.swift`: remove `ImageExportOptions.finish` (its
      `value("finish", …)` line and the dialog's `Picker`); the render uses `document.board.stackup.finish`.
- [x] `docs/export.md` § Image: the Front / Back Render row's extra options read "View (Top, Angled)"; add a
      sentence under the table: "Bare copper is drawn in the board's finish (Stackup ▸ Finish)."
- [x] Tests: `BoardPreviewTests.testRendersEveryView` sets `doc.setStackupFinish(.enig)` before rendering instead
      of passing `finish:`; new `testFinishFollowsTheBoard`: the same board rendered at 200 × 150 with `.enig`,
      `.leadFreeHasl` and `.osp` gives three different images (compare the three `CGImage` pixel buffers), and
      `.hasl` and `.leadFreeHasl` give identical ones.

Acceptance: the preview and image export read the board's finish and nothing else.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/BoardPreviewTests -only-testing:schematicalTests/SchematicExportActionTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (Observed with Phase 2's suites in one run: `Executed 29 tests, with 0 failures`, `** TEST SUCCEEDED **`; est. 6 min; `SchematicExportActionTests` is the suite that builds `ImageExportOptions`, `grep -c ImageExportOptions schematical/schematicalTests/SchematicExportActionTests.swift` → 11).
  Also: `grep -rn --include='*.swift' "BoardRenderFinish" schematical` → no output (observed: 0 lines).
Commit: 0727c4bd

### Phase 2 — The Stackup section

- [x] New `Canvas/StackupSection.swift` with `CopperWeight`, `StackupRows` (§ 3) and `StackupSection(document:)`:
      a `Section("Stackup")` with the Finish `Picker` (→ `setStackupFinish`), the temperature-rise field
      (→ `setStackupTemperatureRise`), the rows (copper: weight `Picker` + µm field → `setCopperThickness`;
      dielectric: kind `Picker`, `LengthField` in the board's units, Er field → `setDielectric`), the Thickness
      row, and a **Reset to Standard** button (→ `resetStackup`). Every edit field re-reads the stored value
      after a commit (so a clamp shows), as `LengthField` does.
- [x] `Canvas/PCBSidebarView.swift`: `StackupSection(document: document)` between `LayersSection` and
      `PlanesSection` in the `.board` case.
- [x] `docs/file-format.md` is unchanged (A owns it). No other doc describes the sidebar.
- [x] Tests: new `schematicalTests/StackupSectionTests.swift`: `CopperWeight(thickness: 0.035) == .one`,
      `0.0152` → nil (Custom), `0.0175` → `.half`; `StackupRows.rows` for 2 layers is
      [copper Top, dielectric 0, copper Bottom], for 4 layers has 4 copper and 3 dielectric rows interleaved
      Top-first, for 1 layer is [copper Top, dielectric 0]; the section's view builds for 1, 2, 4 and 6 layers
      (`ImageRenderer(content: Form { StackupSection(document:) }).cgImage != nil`).

Acceptance: the rows and weights are right for every layer count and the section renders.
  Check: `xcodebuild … test -only-testing:schematicalTests/StackupSectionTests -only-testing:schematicalTests/BoardStackupTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: 0727c4bd

## Validation Plan

- Tests: `StackupSectionTests` (new), `BoardPreviewTests`, `SchematicExportActionTests` (the Image export).
- Coverage check: Phase 1's `testFinishFollowsTheBoard` fails if `BoardRenderer` ignores the board (run once with
  the board's finish forced to `.hasl` in `BoardPreviewView.canvas`; the test must fail, then revert).
- Runtime proof: write the 2-layer example's Front Render at 4× for a board set to ENIG and one set to OSP from
  a test and open both PNGs; pads are visibly gold and copper-coloured. (`ImageExportOptions` default path; no GUI
  driving needed.)
- Doc sync: `docs/export.md` (Phase 1).
- Final gate (once, after Phase 2): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/StackupSectionTests -only-testing:schematicalTests/BoardStackupTests -only-testing:schematicalTests/BoardPreviewTests -only-testing:schematicalTests/SchematicExportActionTests -only-testing:schematicalTests/PCBUndoTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min).

## Decisions

- The finish is one board setting that the preview and renders follow — decided by the owner, 2026-11-16. The two Finish menus go. (§ 1, Phase 1)
- The weight menu offers ½, 1 and 2 oz and Custom, with a µm field — decided by the author (an internal UI choice, not a behaviour the owner asked about). The stackup stores thickness, so JLCPCB's 15.2 µm inner copper stays exact.
- OSP renders as bare copper — decided by the author. It is the finish's look.

## Corrections

- § 2's "tests that pass a finish: 1" was wrong: 2 — `BoardPreviewTests.swift:41` (`finish:`) and `SchematicExportActionTests.swift:125` (`options.finish = .enig`, a property assignment the plan's `grep "finish:"` could not see). Re-measured with `grep -rn --include='*.swift' "finish" schematical/schematicalTests | grep -i "enig\|hasl"`. Both updated; no other letter's scope used the number.
- Validation "Runtime proof" (a 4× Front Render PNG per finish, opened by eye) was replaced by `testFinishFollowsTheBoard`, which renders the same board at 300 × 220 in each finish and asserts the pixel buffers differ (ENIG, OSP, HASL) or are equal (leaded and lead-free HASL): it fails if the renderer ignores the board's finish, which is the same mistake the PNGs would show.
- The sub-plan's final gate is run once for all of plan-50 at the end (see plan-50-A Corrections).

## Summary

Risk is small: a view over A's setters and a type swap in the renderer. Untouched: the model, the file, the
exports (C and D), the 3-D slab thickness, and the mask colour.
