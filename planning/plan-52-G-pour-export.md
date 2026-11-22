# plan-52-G: Pours and Keep-outs in the Exports

Last updated: 2026-11-21
Effort: medium (1h–2h)
Depends on: plan-52-F (complete)

A pour that shows on the PCB page but not in the files the fab and the next tool get is not a pour. This sub-plan writes the pours
into the three exports: **Gerber** (the pour's filled copper as regions on its layer, with its net), **KiCad** (a zone per pour with
its thermal settings, a rule area per keep-out that blocks only copper pour) and **EasyEDA Pro** (a `POUR` per pour).

The single outcome: the Top and Bottom (and inner) Gerber files draw each pour's fill — clearances, thermal reliefs and keep-outs cut
out, the pour's net on every region — a KiCad 10 board opens with the pours as zones and the keep-outs as pour-blocking rule areas and
fills to the same shape, and a board with no pour exports byte-identical to before.

References:

- plan-52-C § 4 (`PourFill.pieces`), plan-52-D § 4 (`copper.pourFills`)
- `docs/export.md` § Gerber (the layer table at `:60`, the attributes table at `:80`–`:87`, "A grown shape" at `:99`), § KiCad (`:172` region), § EasyEDA Pro (`:245`)
- `schematical/schematical/Document/GerberExport.swift`: `drawCopper` (`:125`, the Plane block at `:135`–`:155`); `GerberWriter.swift`: `region` (`:187`), `setDark` (`:100`), `GerberOutline.contours` (`:308`)
- `Document/KiCadBoardWriter.swift`: `zones()` (`:554`, the Plane zone), `codes`/`names` (net codes), `layerName`, `xy`; `Document/EasyEDAProBoardWriter.swift` (`POUR` at `:223`)
- `planning/plan-52-data/README.md` § KiCad, `zone_thermal_keepout.kicad_pcb`, `zone_pouronly.kicad_pcb`, `probe_kicad_zones.py`; `planning/plan-50-data/probe_gerber_attributes.py`
- `~/.claude/projects/-Users-justinzaun-Development-logic/memory/export-checks-kicad-gerber.md` (how to run the real tools from the test host)
- Line numbers in this plan are as of 2026-11-21 and drift as other sessions edit these files; the symbol named beside each is the reference.

## Prerequisites

The shared gate is in plan-52-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-52-F is complete | `ls planning/completed \| grep -c '^plan-52-F'` → 1 | NOT MET (2026-11-21: 0; A–F are not started) |
| a Python venv with `gerbonara==1.6.3` | `$VENV/bin/python -c "import gerbonara"` → no error (create: `python3 -m venv $VENV && $VENV/bin/pip install gerbonara==1.6.3`; needs the network once) | NOT MET (2026-11-21: no venv in this session) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update every status
> before you continue, and again before you decide to stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- **Gerber:** in `drawCopper`, after a layer's Plane block and before its traces, each pour on the layer with a resolved net draws every kept piece of its fill: the outer loop as a dark region
  (`Conductor`, `%TO.N` = the pour's net name), each hole of the piece as a clear-polarity region, then dark again. Pads, vias and traces of the layer draw after, so they sit on the pour.
- **KiCad:** each pour with a resolved net is a `(zone …)` on its layer over its rectangle, `(connect_pads (clearance c))` for thermal or `(connect_pads yes (clearance c))` for solid,
  `(min_thickness minWidth)`, `(fill yes (thermal_gap g) (thermal_bridge_width w))`; each keep-out is a rule area `(zone (net 0) (layers …) (keepout (tracks allowed) (vias allowed) (pads allowed)
  (copperpour not_allowed) (footprints allowed)) …)` over its rectangle on its layers (all the board's copper layers when `layers == nil`). A pour with no net, or one the schematic no longer has, is not written.
- **EasyEDA Pro:** each pour with a resolved net is a `POUR` line like a Plane's, over its rectangle. Keep-outs are not written (§ Non-goals).
- `docs/export.md` describes all three.
- A document with no pour and no keep-out exports byte-identical files in all three formats.

### Non-goals (explicit constraints)

- The KiCad zone is not pre-filled: KiCad fills it with Fill All Zones (B), exactly as the Plane zone is written today. The app's fill and KiCad's differ in detail (corner rounding, spoke angle).
- EasyEDA Pro keep-outs: the format's keep-out record cannot be checked here (no file of it has been opened in EasyEDA Pro, `docs/export.md` § EasyEDA Pro), so a keep-out is not exported and a pour there floods over it when EasyEDA refills it.
- No change to Plane output, mask, paste, silkscreen, drill, the job file, the PDF export or the image export.
- No new Gerber apertures or attributes beyond the existing `Conductor` function and net.

## 2. Current State

- A Plane's Gerber (`GerberExport.swift:135`–`:155`): `file.setFunction("Conductor")`, `setObject(net:)`, `file.region(board.outline)`, then `setDark(false)` and the clearances as filled shapes grown by the clearance, `setDark(true)`.
  `setDark(false)` clears the function and object (`GerberWriter.swift:104`–`:107`), so they are set again after.
- `GerberWriter.region(_ contour:)` writes one `G36…G37` loop; y is converted inside the writer.
- `KiCadBoardWriter.zones()` (`:554`) writes one unfilled zone per net-carrying Plane, `(connect_pads (clearance c))`, `(min_thickness 0.2)`, `(fill yes (thermal_gap 0.5) (thermal_bridge_width 0.5))`, the board outline polygon.
- `EasyEDAProBoardWriter` (`:223`): `POUR` per Plane with the outline points, `["SOLID", 0]`.
- `ExportSamples` (`schematicalTests/KiCadExportTests.swift:396`) builds the boards the tests export and print (`vias(…)` at `:468`); `testPrintSamplesWhenAsked` (`:346`) prints `KICAD_FILE`/`GERBER_FILE` lines with `TEST_RUNNER_KICAD_OUT=1`.

### Measured populations

| What | Count | Command |
|---|---|---|
| export writers that write Planes | 3 | `grep -ln "planeNets" schematical/schematical/Document/*.swift` → GerberExport, KiCadBoardWriter, EasyEDAProBoardWriter (the KiCad writer through `copper.planeNets`) |

### Verified properties

- KiCad 10.0.8 loads the zone and rule-area syntax and reads back thermal/solid, gap, spoke width, minimum width and clearance, the rule area's layers and that it blocks only pour; after `ZONE_FILLER.Fill`
  the fill has no copper in the keep-out and keeps the clearance from a trace (`planning/plan-52-data/README.md` § KiCad, `probe_kicad_zones.py`, run 2026-11-21).
- `GerberOutline.contours(of:)` flattens a `CGPath` with curves to point loops, curves cut into steps of at most 0.05 mm (`docs/export.md:96`).
- A piece's outer loop is the contour of largest absolute area (`GerberWriter.area`, `:293`); the others are its holes: `componentsSeparated` gives one outer loop and its holes per piece (`probe_components.swift`, run 2026-11-21: 121 components from 800 obstacles, 2 with more than one loop, in both the largest loop's box holds every other loop's box; the most loops in one component is 157).

## 3. Design Overview

Three independent writers reading one input, `copper.pourFills` and the document's pours and keep-outs, so they cannot disagree with the screen. **Where risk concentrates:** the Gerber clear-polarity holes (a clear region
erases everything drawn earlier on the layer, so the pours are drawn before the traces and pads, and a piece's holes lie inside the piece) and KiCad's acceptance (checked with the real tool).
**Byte-identity IS the gate for boards with no pour** (class: provably-neutral for them): the three exports of every existing test fixture are byte-equal before and after; a diff is a bug to root-cause (`diff` the two files), not a stop.
For boards with pours the expected differences are exactly the new regions, zones and `POUR` lines.
**What was probed:** the KiCad syntax and behavior (above); the Gerber approach is the Plane's own (`region`, `setDark`) with the same writer.

Rejected alternatives: *KiCad zones pre-filled with `filled_polygon`* (a second source of truth; KiCad refills anyway); *the pour as one region less a clear-polarity shape per obstacle* (the writer would need every obstacle's clearance again, a second copy of plan-52-C); *a KiCad `(keepout (tracks not_allowed) …)`* (the owner chose that a keep-out blocks pours only).

## 4. Detailed Design

- Gerber helper `GerberExport.drawPours(on layer:, copper:, nets:, into file:)`: for each pour in `document.pours` with `pour.layer == layer` and a `copper.pourFills[pour.id]`, for each piece: `GerberOutline.contours(of: piece)`;
  sort by `abs(GerberWriter.area($0))` descending; the first is the outer: `setFunction("Conductor")`, `setObject(net)`, `file.region(outer)`; for the rest: `file.setDark(false)`, `file.region($0)`, `file.setDark(true)`, then function/object set again.
- KiCad `zones()` loops `document.pours` then `document.keepouts`; a pour's net code is `codes[net]`, name `names[net]`; uuids `id("pour/\(pour.id)")`, `id("keepout/\(keepout.id)")`; layer name `layerName(layer)`;
  the polygon is the four corners of the rect through `xy(_:)`, clockwise from the top-left (as the outline's).
- EasyEDA: the Plane's `POUR` line (`:225`–`:230`) with the four corners and the pour's layer id and net, named `"<layer> <net>"`.

## Compatibility / Format Impact

Gerber: new regions in copper files only where a pour is. KiCad: new `(zone …)` blocks. EasyEDA: new `POUR` lines. Unchanged: every file of a board with no pour, every non-copper Gerber file, the drill and job files.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work; `- [~]` for partial; mark a moot
> task `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands. An unticked box means NOT DONE.

### Phase 1 — Gerber

- [ ] `GerberExport.swift`: `drawPours` as § 4, called in `drawCopper` after the Plane block and before the traces.
- [ ] `docs/export.md`: the `.GTL` / `.G1`…`.G4` / `.GBL` rows and the attributes table (a pour's copper is `Conductor` with its net; its holes clear polarity) updated.
- [ ] Tests: new `schematicalTests/PourExportTests.swift`: a two-LED board (as `PlaneNetTests`) with a Top ground pour: (a) the `.GTL` has a region carrying `%TO.N,GND*%` (the net name the export uses) and a `%LPC*%` hole region after
      it and before the first trace or pad; (b) the `.GBL` has none; (c) a keep-out over the pour removes the region area under it: compare the flattened outer area of the piece with and without the keep-out (smaller by about the keep-out's area);
      (d) every existing fixture's Gerber files are byte-equal with a pour added then deleted.
- [ ] Gerber cross-check with gerbonara: export the board of (a) with `TEST_RUNNER_KICAD_OUT=1` (the existing `testPrintSamplesWhenAsked` path; add the pour board to `ExportSamples` as `pours()`), decode the `GERBER_FILE …GTL` line,
      run `$VENV/bin/python planning/plan-50-data/probe_gerber_attributes.py <file>`; record its output under this phase.

Acceptance: (a)–(d) hold and gerbonara reads the pour's region objects with `.N` = the pour's net.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/PourExportTests -only-testing:schematicalTests/GerberExportTests -only-testing:schematicalTests/GerberAttributeTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 7 min).
  And: the probe command above → a `Region` row with `Conductor` and `distinct .N values` including the pour's net (est. 3 min).
Commit: —

### Phase 2 — KiCad

- [ ] `KiCadBoardWriter.swift` `zones()`: pours then keep-outs as § 1 and § 4 (skip a pour with no resolved net).
- [ ] `docs/export.md` § KiCad: the pour zone, the thermal/solid mapping (`connect_pads` with and without `yes`), the rule area, and "KiCad fills it with Fill All Zones (B)".
- [ ] Tests: `PourExportTests` (e) the `.kicad_pcb` text has one `(zone` per net-carrying pour and one rule area per keep-out; a solid pour has `(connect_pads yes`, a thermal one `(connect_pads (clearance`; (f) a pour with no net writes no zone;
      (g) the output of a board with no pour is byte-equal to `KiCadExportTests`' existing determinism check (`testSameDocumentGivesSameBytes`); (h) two exports of one board are byte-equal.
- [ ] KiCad cross-check: print the pours sample (`TEST_RUNNER_KICAD_OUT=1`), decode `KICAD_FILE pours/…kicad_pcb` into `$OUT`, run `kicad-cli pcb drc` (loads; record `Found N violations`), then
      `PYTHONPATH=… python3 -I planning/plan-52-data/probe_kicad_zones.py $OUT/pours.kicad_pcb` and compare each zone's connection (thermal/solid), gap, spoke, minimum width and clearance, and each rule area's layers, with the app's values.

Acceptance: (e)–(h) hold; `probe_kicad_zones.py` prints the app's values for each zone and rule area; `kicad-cli pcb drc` loads the file.
  Check: `xcodebuild … test -only-testing:schematicalTests/PourExportTests -only-testing:schematicalTests/KiCadExportTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 7 min).
  And: `~/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli pcb drc --output $OUT/pours.rpt --format report $OUT/pours.kicad_pcb` → `Found N violations` with `Found 0 unconnected items` for the ground pads the pour joins, and the probe's lines as above (est. 4 min).
Commit: —

### Phase 3 — EasyEDA Pro

- [ ] `EasyEDAProBoardWriter.swift`: a `POUR` per net-carrying pour after the Plane pours (`:223`), four corner points, layer id, net, name `"<layer> <net>"`; keep-outs not written.
- [ ] `docs/export.md` § EasyEDA Pro: pours exported as `POUR`; keep-outs not exported, and why (the keep-out record cannot be checked here).
- [ ] Tests: `EasyEDAProExportTests` (i) the board with a pour has one more `POUR` line than without, with the pour's net and four points; (j) a board with no pour is byte-equal to a fresh export of the same document.

Acceptance: (i) and (j) hold.
  Check: `xcodebuild … test -only-testing:schematicalTests/EasyEDAProExportTests -only-testing:schematicalTests/PourExportTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

## Validation Plan

- Tests: `PourExportTests` (new, 10 cases), `GerberExportTests`, `GerberAttributeTests`, `KiCadExportTests`, `EasyEDAProExportTests` (existing, must pass unchanged).
- Coverage check: `grep -c "pour" schematical/schematical/Document/GerberExport.swift schematical/schematical/Document/KiCadBoardWriter.swift schematical/schematical/Document/EasyEDAProBoardWriter.swift` → each at least 1, all reached by (a), (e), (i).
- Runtime proof: gerbonara and KiCad 10 as in Phases 1 and 2; the app itself in plan-52-H.
- Doc sync: `docs/export.md` (Gerber layer table and attributes, KiCad, EasyEDA Pro), checked by `grep -c "pour" docs/export.md` → at least 6.
- Final gate (once, after Phase 3): `xcodebuild … test -only-testing:schematicalTests/PourExportTests -only-testing:schematicalTests/GerberExportTests -only-testing:schematicalTests/GerberAttributeTests -only-testing:schematicalTests/GerberJobTests -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/EasyEDAProExportTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 10 min).

## Decisions

- The Gerber carries the app's own fill (the pour cut by clearances, reliefs and keep-outs) — decided by the author: a fab needs the copper that exists; KiCad refills its own zones, a Gerber cannot.
- KiCad keep-outs block copper pour only, on the layers they name — decided by the owner, 2026-11-21 ("copper pours only"); the rule area's other four blocks are `allowed`.
- EasyEDA Pro gets pours but not keep-outs — decided by the author: the pour line is the Plane's, already written; the keep-out record is unverifiable here.
- A pour with no net is not exported — decided by the author: a net-0 KiCad zone would flood floating copper, which is not what the app draws.

## Corrections

(Filled in during execution.)

## Summary

The risk is acceptance by real tools, checked with KiCad 10 and gerbonara, and the clear-polarity holes in the Gerber. Untouched: Planes, mask, paste, silkscreen, drill, the job file, the PDF and image exports.
