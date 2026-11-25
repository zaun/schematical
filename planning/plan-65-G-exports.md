# plan-65-G: Cutouts in Gerber, KiCad, EasyEDA Pro, PDF and Print

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-65-F (complete)

A fab makes the board from the Gerber profile; a cutout that is only on screen is a board that arrives without its window. This sub-plan puts the cutouts into every export of the board: the Gerber profile
and the inner-layer Planes (which must clear the cutout and keep the copper-to-edge band), KiCad's `Edge.Cuts`, EasyEDA Pro's board outline layer, and the PDF/print board pages. The image export
needs no change (it draws the PCB page, plan-65-E). A free-form outline already reaches every export because each reads `board.outline`.

The single outcome: the exported Gerber profile, the KiCad board and the EasyEDA Pro board each contain the outline and one closed loop per cutout, a Plane's copper has a hole and an edge band round each cutout, and a document without cutouts exports byte for byte what it does today.

References:

- plan-65-A § 3 (byte-identity for no cutouts), § 4.2 (`BoardEdge.loops`, `segments`)
- `Document/GerberExport.swift`: the profile (`:99`–`:101`), `drawCopper` and the Plane branch (`:135`–`:150`); `GerberWriter.swift` (`setDark` `:100`, `stroke` `:171`, `region` `:187`, `fill` `:202`); `GerberJob.swift:15` (size from the outline's bounds, which a cutout does not change)
- `Document/KiCadBoardWriter.swift`: `extent` (`:48`), the write order (`:203`), `outline()` (`:490`), the Plane zone (`:555`–`:575`); `Document/EasyEDAProBoardWriter.swift`: the outline `POLY` (`:217`–`:224`); `Document/PrintExport.swift`: `BoardArtwork` (`:129`), its construction (`:289`–`:291`), the Board page (`:421`–`:427`)
- `docs/export.md` § Gerber (`:56`), § PDF and print (`:182`), § KiCad (`:223`), § EasyEDA Pro (`:304`); `planning/plan-65-data/README.md` (KiCad's zone fill and nested Edge.Cuts)
- `schematicalTests/GerberExportTests.swift`, `KiCadExportTests.swift` (`ExportSamples` at `:456`, `testPrintSamplesWhenAsked` at `:386`), `EasyEDAProExportTests.swift`, `PourExportTests.swift`, `PrintExportTests.swift`
- Memory *Export checks (KiCad/Gerber)*: `KICAD_OUT=1` (passed as `TEST_RUNNER_KICAD_OUT=1` to `xcodebuild`) makes the sample tests print their files; `kicad-cli` and KiCad's python check them

Test command form (`XB`): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p65-dd CODE_SIGNING_ALLOWED=NO test`;
a Check below writes `XB -only-testing:schematicalTests/<Class>`. Debug, scoped to the classes named.

## Prerequisites

The shared gate is in plan-65-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-65-F is complete | `ls planning/completed \| grep -c '^plan-65-F'` → 1 | MET (2026-11-25: F landed, 1) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update every status
> before you continue, and again before you decide to stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- Gerber: the profile file (`.GKO`) strokes the outline and every cutout loop, each closed, each one `D02` move then `D01` draws; a Plane layer's copper is the outline region less every cutout region and less a `2 × minCopperToEdge` stroke round every loop; no other file changes.
- KiCad: every cutout loop is `gr_line`s on `Edge.Cuts` with the same width as the outline (0.1 mm); the Plane zone's polygon stays the outline (KiCad's fill keeps out of an inner loop and keeps its clearance: `planning/plan-65-data/README.md`).
- EasyEDA Pro: each cutout loop is a closed `POLY` on the board outline layer (11), after the outline's.
- PDF and print: the Board page strokes every loop; copper pages that carry a Plane inherit the Gerber's cleared copper.
- `docs/export.md` says so in the four sections.

### Non-goals (explicit constraints)

- No change for a document without cutouts: every Gerber file, the KiCad file and the EasyEDA project are the same bytes as before (the existing export tests pass unedited).
- No NPTH/drill-file entry for a cutout (it is routed, so it is the profile's); no cutout in the pick-and-place or the job file's size (the job file's size is the outline's bounds).
- No arcs: every loop is the polygon of `BoardCutout.Shape.polygon` (A § 3).
- No change to the Plane zone's polygon or to the pour export (a pour's fill already excludes cutouts, C).

## 2. Current State

- `profile.stroke(board.outline, width: profileWidth, closed: true)` is the whole profile (`GerberExport.swift:100`).
- The Plane branch of `drawCopper` writes `file.region(board.outline)`, `file.setDark(false)`, and, when `edge > 0`, `file.stroke(board.outline, width: 2 * edge, closed: true)`, then clears pad and trace clearances (`:140`–`:150`).
- KiCad: `outline()` returns one `gr_line` per outline segment, uuid seed `edge/<i>`, layer `Edge.Cuts`, width 0.1 (`:490`–`:497`). `extent(of:)` is the outline's and the items' box.
- EasyEDA Pro: one `POLY` on layer 11 from the outline, closed by repeating the first point (`EasyEDAProBoardWriter.swift:217`–`:224`), width `mil(0.1)`.
- PDF and print: `BoardArtwork.outline: [CGPoint]`, stroked closed on the Board page (`PrintExport.swift:421`–`:427`); copper pages replay the Gerber's recorded marks (`:289`).

### Measured populations

| What | Count | Command |
|---|---|---|
| lines that read the outline in the exporters | 15 lines in 6 files (`PrintExport`, `KiCadBoardWriter`, `GerberExport`, `ImageExport`, `EasyEDAProBoardWriter`, `GerberJob`) | `rg -n "board\.outline\|artwork\.outline" schematical/schematical/Document --glob '*.swift' \| wc -l` → 15 |
| export test classes the no-cutout check uses | 5 (`PrintExportTests` is a sixth, run in Phase 1) | `ls schematical/schematicalTests \| grep -E "^(GerberExportTests\|GerberJobTests\|KiCadExportTests\|EasyEDAProExportTests\|PourExportTests)\.swift"` → all five |
| `ExportSamples` builders (`cutouts()` is added beside `pours()`) | 6: `turned`, `routed`, `oppositePads`, `fourLayer`, `pours`, `vias` | `rg -n "static func" schematical/schematicalTests/KiCadExportTests.swift \| sed -n 1,8p` → those six at `:458`–`:580` |

### Verified properties

- KiCad 10.0.8 loads a board with nested `Edge.Cuts` loops with no violation, and fills a zone round one with its clearance: `planning/plan-65-data/README.md`.
- KiCad's own EasyEDA Pro importer (`PCB_IO_MGR.EASYEDAPRO`) is how the EasyEDA output has been checked (`docs/export.md`:`What was checked`); it is the check used for the cutout `POLY`s in Phase 2 below, run on the app's own output.

## 3. Design Overview

One helper: `GerberExport.profileLoops(board edge: BoardEdge) -> [[CGPoint]]` is `edge.loops` (outline first). Everything else reads `document.boardEdge`.

- **Gerber.** Profile: `for loop in edge.loops { profile.stroke(loop, width: profileWidth, closed: true) }`. Plane: after the existing outline lines, `for loop in edge.cutouts { file.region(loop); if edge > 0 { file.stroke(loop, width: 2 * clearance, closed: true) } }` — with `setDark(false)` still in force the cutout region is cleared and the band round it is cleared, in the polarity the outline's own band already uses.
- **KiCad.** `outline()` loops over `edge.loops`; the outline keeps seed `edge/<i>` and cutout `k` uses `cutout/<k>/<i>` (so the outline's uuids do not change). The Plane zone and `extent` are unchanged.
- **EasyEDA Pro.** After the outline's `POLY`, one `POLY` per cutout with the same fields, ids from `ids.id()` in order (the outline's id is still first, so existing ids do not move).
- **Print.** `BoardArtwork.outline` becomes `loops: [[CGPoint]]` (outline first); the Board page strokes each; `bounds` is the outline's.

Risk: byte-identity of the no-cutout case through every id counter and uuid seed (the new ids are drawn only after the old ones, and only when there are cutouts), and the Plane polarity order (clear after dark). Byte-identity is the no-cutout gate (five classes); the cutout cases have their own tests and real-tool checks.

Rejected: writing a cutout as an NPTH drill slot (the fab's profile router makes it; a drill file would double it); drawing a cutout as `gr_circle`/`gr_arc` in KiCad (every consumer here is polygonal; KiCad accepts the polyline, checked).

## Phases

> **NOTE — keep the checkboxes current as you go. This is not bookkeeping; it is the only way anyone can see where the work actually is.**
>
> - Tick `- [x]` **in the same commit as the work it describes**; use `- [~]` for partially done and say what remains; mark a moot task `- [x] ~~text~~ — moot: <evidence>`; fill the phase's `Commit:` line with the hash(es) the moment it lands; add any task you discover you needed.
>
> **An unticked box means NOT DONE.**

### Phase 1 — Gerber and print

- [x] `Document/GerberExport.swift`: the profile loop and the Plane clearing of § 3. `Document/PrintExport.swift`: `BoardArtwork.loops`, its construction and the Board page.
- [x] `schematicalTests/KiCadExportTests.swift`: `ExportSamples.cutouts()` (a 60 × 40 board, a 6 mm circle, a 10 × 2 slot, a 4-point polygon, and one inner Plane on GND).
- [x] Tests `schematicalTests/CutoutExportTests.swift` (Gerber half): the `.GKO` text has `1 + 3` `D02` moves (`stroke` writes one per loop) and each loop closes (last `D01` returns to the first point); a Plane layer's data holds a clear-polarity region for each cutout and a clear stroke of width `2 × minCopperToEdge` for each loop; the other Gerber files (copper without a Plane, mask, silkscreen, drill) are identical bytes to those of the same document with `cutouts` empty; `GerberJob`'s size is unchanged; the PDF's Board page (`PrintExportTests`' way of reading the pages) has 4 closed subpaths; `PrintExportTests` itself passes unedited.

Acceptance: the profile and the Plane carry the cutouts, nothing else in the Gerber set moves, and the five existing export classes pass unedited.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/CutoutExportTests -only-testing:schematicalTests/GerberExportTests -only-testing:schematicalTests/GerberJobTests -only-testing:schematicalTests/PourExportTests -only-testing:schematicalTests/PrintExportTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 5 min).
  Observed: CutoutExportTests 3 + GerberExportTests 24 + GerberJobTests 7 + PourExportTests + PrintExportTests, 0 failures, TEST SUCCEEDED.
Commit: —

### Phase 2 — KiCad and EasyEDA Pro, checked with KiCad

- [ ] `Document/KiCadBoardWriter.swift`: `outline()` over `edge.loops` with the uuid seeds of § 3. `Document/EasyEDAProBoardWriter.swift`: the cutout `POLY`s.
- [ ] `CutoutExportTests` (KiCad/EasyEDA half): the KiCad text has `gr_line` count = outline segments + the cutouts' segments, all on `Edge.Cuts`; uuids unique; with no cutouts the file equals the file of the same document built before the change (the existing `KiCadExportTests` and `EasyEDAProExportTests` pass unedited); the EasyEDA `PCB/<id>.epcb` has `1 + 3` `POLY` lines on layer 11, the outline's id first.
- [ ] Real-tool check, run once and recorded in this file's Commit line: `TEST_RUNNER_KICAD_OUT=1 XB -only-testing:schematicalTests/KiCadExportTests/testPrintSamplesWhenAsked` with `ExportSamples.cutouts()` added to the printed set; decode the `cutouts.kicad_pcb` and run `kicad-cli pcb drc --format json` → `violations 0` for the edge group, and `kicad-cli pcb export dxf -l Edge.Cuts` → a DXF whose `LINE` count equals the segment count (4 loops closed); load the EasyEDA project's `.epro` with KiCad's importer (`pcbnew` `PCB_IO_MGR.EASYEDAPRO`, as in `docs/export.md`) → Edge.Cuts holds 4 closed loops.
- [ ] `docs/export.md`: Gerber (the profile holds the cutouts; a Plane clears them and keeps the copper-to-edge band), PDF and print, KiCad (`Edge.Cuts`, and that the zone's fill keeps out of them, checked with `planning/plan-65-data/probe_zone_edge_cuts.py`), EasyEDA Pro (the extra outline `POLY`s, checked by KiCad's importer), and the keep-outs line left alone.

Acceptance: KiCad accepts the board with no edge violation and sees every loop; KiCad's EasyEDA importer sees every loop; no-cutout output is unchanged.
  Check: `XB -only-testing:schematicalTests/CutoutExportTests -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/EasyEDAProExportTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 5 min), and the real-tool commands above print the numbers they name (est. 3 min).
Commit: —

## Validation Plan

- Tests: `CutoutExportTests` (positive, and the identical-bytes negative for every file a cutout must not touch).
- Coverage check: the five existing classes build the same files through the changed functions without cutouts, so a changed byte fails there.
- Runtime proof: the real-tool commands of Phase 2; J opens the exported KiCad board in the real KiCad window only if the owner wants (not a step).
- Doc sync: `docs/export.md` (four sections) in Phase 2.
- Final gate (run ONCE, after Phase 2): both phases' test classes (est. 8 min).

## Decisions

- Cutouts export as part of the profile, not as drills — decided by the author (no user-visible fork: the fab routes the profile), 2026-11-25. Settles § 3.
- A Plane's copper clears every cutout with the copper-to-edge band, as the pours do (the owner's "copper must clear a cutout like the board edge", 2026-11-25).

## Corrections

<Filled in during execution.>

## Summary

Risk: byte identity of the no-cutout exports, and the Plane's clear polarity. Left untouched: the drill files, pick and place, the job file, the pour export, the schematic exports, and the image export (it draws the PCB page).
