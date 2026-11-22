# plan-54-G: The app — opposite-side pads in the exports and the 3D preview

Last updated: 2026-11-22
Effort: medium (1h–2h)
Depends on: plan-54-F

G makes the exports and the Board Preview window put an opposite-side pad (plan-54-E,
plan-54-F) on the right face: the Gerber copper, solder-mask and paste files, the KiCad
board's pad layers, the EasyEDA Pro footprint pads, the printed copper pages and the 3D
preview's pads and solder. The behavioral outcome: a board with a Top-side part whose
footprint has one top pad and one opposite pad exports a Gerber set in which the Top copper,
mask and paste files hold the first pad and the Bottom files the second, a KiCad board whose
pads are on `F.Cu` and `B.Cu` and passes `kicad-cli pcb drc` with no pad-layer error, and a
Board Preview that draws the second pad's copper on the underside.

References:

- plan-54-E § 3 (the keys), plan-54-F § 2 – 3 (`PlacedPad.layerSide(on:)`, the one assignment
  in `Ratsnest.swift`, the sites already converted).
- `schematical/schematical/Document/GerberExport.swift` (lines 85, 168, 249),
  `Document/KiCadBoardWriter.swift` (`kpad` line 230, `padText` line 249, `padLine`),
  `Document/EasyEDAProBoardWriter.swift` (`spec` line 115, the pad layer at line 133),
  `Document/PrintExport.swift` (line 401, through the Gerber artwork),
  `BoardPreview/BoardRenderer.swift` (the pad passes at lines 394 – 436).
- `planning/plan-50-data/README.md` and its probes (`probe_gerber_attributes.py` reads a Gerber with gerbonara;
  `probe_kicad_stackup_roundtrip.py` runs KiCad 10.0.8): the precedent for checking an export with the real tool.
  `KiCadExportTests.testPrintSamplesWhenAsked` (line 346) prints `KICAD_FILE` / `GERBER_FILE` lines when
  `KICAD_OUT` is set; `TEST_RUNNER_KICAD_OUT=1 xcodebuild … test` passes it to the test host.

## Prerequisites

plan-54-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-54-F complete | `ls planning/completed/plan-54-F-*.md` → exists | NOT MET (2026-11-22: not started) |
| `PlacedPad.layerSide` exists | `grep -c 'func layerSide' schematical/schematical/Canvas/BoardModel.swift` → 1 | NOT MET (2026-11-22: F adds it) |
| KiCad's CLI is installed | `test -x ~/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli && echo ok` → `ok` | MET (2026-11-22: re-run the command) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- **Gerber.** For a document with an opposite-pad part: `GerberExport`'s copper file of a
  layer holds exactly the pads whose layer is that layer (surface-mount pads by `entry.side`,
  already the pad's layer after F), its mask file the same pads grown by the mask expansion,
  its paste file the same pads; no code change is needed in the three sites (§ 2) and a test
  proves it.
- **KiCad.** A pad's `(layers …)` are the pad's layer's (`F.Cu F.Paste F.Mask` or `B.Cu B.Paste
  B.Mask`) while its x and corners are mirrored by the footprint's side.
- **EasyEDA Pro.** A footprint's pad rows carry layer 2 (bottom) for an opposite pad and layer 1 for
  an ordinary pad of the (top-defined) footprint.
- **Print.** The Bottom copper page holds the opposite pad of a Top part (it follows the Gerber
  artwork).
- **Board Preview.** The pads and solder of a surface-mount pad are drawn on the face of the pad's
  layer.
- `kicad-cli pcb drc` on the exported board reports no pad-layer error; `gerbonara` reads the
  Gerber copper files and finds the pad on the expected layer.

### Non-goals (explicit constraints)

- No change to through-hole pads, holes, fingers, silkscreen, courtyards or the pick-and-place
  file (a placement is by the part, not by its pads).
- No change to a part's own export when it has no opposite pad: every existing export test passes
  unchanged.

## 2. Current State

The Gerber writer loops `copper.pads` (`BoardConnectivity.Pad`) and tests `entry.side == side`:
paste (`GerberExport.swift:85`), copper (`:168`), mask (`:249`); after F assigns the pad's layer to
`entry.side`, these three are right without edits. The KiCad writer builds each `KPad` from a
`Footprint.Pad` with `kpad(_:side:)` (`KiCadBoardWriter.swift:230`): it mirrors x and corners when
`side == .bottom` and stores `side` in the pad; `padText` chooses the layers from `pad.side`
(`:284`). The EasyEDA writer builds each `PadSpec` from a footprint pad with `side: .top`
(`EasyEDAProBoardWriter.swift:117`) and writes layer 2 when `pad.side == .bottom` (`:133`). The
3D renderer draws a surface-mount pad's copper and solder from the parts whose own side is the
side being drawn (`BoardRenderer.swift:393` – `:395`) and through-hole pads on both (`:405`,
`:436`). The print export reads the Gerber artwork's copper per layer (`PrintExport.swift:401`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Writers and renderers that read a pad's layer | 3 exporters + the renderer + print | `grep -rln 'boardPads' --include='*.swift' schematical/schematical/Document schematical/schematical/BoardPreview` |
| Export test classes | Gerber 3 (`GerberExportTests`, `GerberAttributeTests`, `GerberJobTests`), KiCad 1, EasyEDA 1, Print 1, Board Preview 1 | `ls schematical/schematicalTests \| grep -i -E 'gerber\|kicad\|easyeda\|print\|boardpreview'` |

### Verified properties

- The Gerber pad loops need no edit: they read the connectivity pad's `side`, which F sets.
- The KiCad writer mirrors by the **footprint's** side and names layers by the **pad's** side; today the
  two are the same value (`side` is passed to both), so G separates them.
- `PickAndPlaceExport.swift:72` uses `boardPads` only to find pad 1's centre for the origin, not a layer.

## 3. Design

- **KiCad.** `kpad(_:side:)` keeps mirroring by `side` and sets the stored `KPad.side` to
  `pad.opposite ? other(side) : side`; `padText` is unchanged. The footprint's own layer
  (`copperSide` in `footprintText`) is unchanged.
- **EasyEDA Pro.** `spec(_ pad: Footprint.Pad)` sets `side: pad.opposite ? .bottom : .top`; the board
  writer's `spec(placed:side:)` is for the card's own pads and is unchanged.
- **Board Preview.** `BoardRenderer`'s surface-mount pad pass iterates every part and draws a pad whose
  `layerSide(on: part side)` equals the side being drawn; the through-hole passes are unchanged.
- **Gerber, print:** tests only.
- **Risk** is small: one assignment in each of three writers; the checks are a Gerber, KiCad and EasyEDA
  export of a synthetic opposite-pad board read back by the real tools.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Gerber and print: the tests

- [ ] `schematicalTests/GerberExportTests.swift`: a document with the plan-54-F test part (a top pad 1 and an
      opposite pad 2) placed on Top: the Top copper, mask and paste files hold pad 1's flash and not pad 2's;
      the Bottom files hold pad 2's and not pad 1's; the same part placed on Bottom swaps them; a part
      without opposite pads exports byte for byte as before (the existing tests).
- [ ] `schematicalTests/PrintExportTests.swift`: the Bottom copper page of that document has one pad mark
      (pad 2), the Top copper page one (pad 1).

Acceptance: the Gerber files and the printed copper pages follow the pad's layer.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p54-dd test -only-testing:schematicalTests/GerberExportTests -only-testing:schematicalTests/GerberAttributeTests -only-testing:schematicalTests/GerberJobTests -only-testing:schematicalTests/PrintExportTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 12 min).
Commit: —

### Phase 2 — KiCad, EasyEDA Pro and the 3D preview

- [ ] `Document/KiCadBoardWriter.swift`, `Document/EasyEDAProBoardWriter.swift`,
      `BoardPreview/BoardRenderer.swift`: § 3.
- [ ] `schematicalTests/KiCadExportTests.swift`: the exported board's opposite pad has `(layers "B.Cu" "B.Paste"
      "B.Mask")` and its top pad `"F.Cu" "F.Paste" "F.Mask"` on a Top part; on a Bottom part the other way round;
      x mirrored for the Bottom part as before.
- [ ] `schematicalTests/EasyEDAProExportTests.swift`: the opposite pad's `PAD` row has layer 2 and the ordinary
      pad layer 1.
- [ ] `schematicalTests/BoardPreviewTests.swift`: a Top part's opposite pad is drawn in the Bottom face's
      copper pass and not the Top face's (the renderer test's existing probe of drawn pads).

Acceptance: KiCad, EasyEDA Pro and the preview follow the pad's layer.
  Check: `xcodebuild … test -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/EasyEDAProExportTests -only-testing:schematicalTests/BoardPreviewTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 12 min).
Commit: —

### Phase 3 — The real tools

- [ ] Run the Phase 2 KiCad test's board through `kicad-cli pcb drc` and the Gerber files through `gerbonara`
      (the method of `planning/plan-50-data/README.md`: `TEST_RUNNER_KICAD_OUT=1 xcodebuild … test -only-testing:schematicalTests/KiCadExportTests`,
      decode the `KICAD_FILE` / `GERBER_FILE` lines, run the tools): `kicad-cli pcb drc` reports no
      "pad layer" or "no copper" violation for the opposite pad and `gerbonara` finds the pad in `-B_Cu.gbr`
      (and not in `-F_Cu.gbr`); record the commands and their output under § Corrections.

Acceptance: the real consumers accept the exports.
  Check: the commands of the task (est. 20 min: one test run, three tool runs).
Commit: —

## Validation Plan

- Tests: the Gerber, print, KiCad, EasyEDA and preview classes above; every existing export test unchanged.
- Coverage check: `grep -rn 'boardPads' --include='*.swift' schematical/schematical/Document schematical/schematical/BoardPreview`
  lists no site that decides a pad's layer from the part's side without `layerSide` (recorded under § Corrections).
- Runtime proof: Phase 3's real-tool runs; O's harness opens a straddle-mount socket in the real app.
- Doc sync: `docs/export.md` names pad layers if it describes them (`grep -n 'layer' docs/export.md`; update the
  sentence that ties a pad's layer to its part's side, if there is one).
- Final gate: none; the landing tier is P's.

## Decisions

- Owner (2026-11-22): straddle-mount parts are supported (plan-54-E).
- Author: **Gerber and print need no code change** (§ 2): they follow the connectivity pad's side; the tests are
  the proof.

## Corrections

## Summary

G is three one-line assignments (KiCad, EasyEDA, the renderer) and the tests that prove all five outputs, with
the real KiCad and gerbonara tools as the last check.
