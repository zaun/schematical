# plan-50-E: Drill-Tool Functions, the Gerber Job File and the KiCad Stackup

Last updated: 2026-11-16
Effort: medium (1h–2h)
Depends on: plan-50-D (complete)

Three outputs still know nothing of the stackup or of what a hole is for. The Excellon drill files list tools
by diameter only, so a fab cannot tell a via drill from a component drill (the spec: "when they are separate
tools the compensation can be done per tool"). The zip has no job file, the standard way to hand a fab the
layer stack, thickness, finish and the board's design rules. And the KiCad board writes
`(general (thickness 1.6))` and no stackup, whatever the app's board is.

The single outcome: the drill files name each tool's function; the Gerber zip has a `<base>-job.gbrjob` that
validates against Ucamco's job-file JSON schema and carries the stackup; and the KiCad board has a
`(setup (stackup …))` that KiCad 10 reads back with the same copper, dielectric and finish values and a board
thickness equal to the stackup's.

References:

- plan-50-A (prerequisites and the stackup model), plan-50-D (Gerber attributes)
- `schematical/schematical/Document/DrillExport.swift`, `GerberExport.swift`, `KiCadBoardWriter.swift` (the
  `general` / `setup` block at `:127`–`:160`)
- Ucamco's Gerber job-file schema (`https://www.ucamco.com/files/downloads/file_en/397/gerber-job-file-schema_en.json`,
  draft-07, "according to revision 2023.06") and `planning/plan-50-data/README.md`
- Gerber spec § 5.6.10 (`ViaDrill`, `ComponentDrill`, `MechanicalDrill`); `docs/export.md` § Gerber, § Drill files, § KiCad

## Prerequisites

The shared gate is in plan-50-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-50-D is complete | `ls planning/completed \| grep -c '^plan-50-D'` → 1 | NOT MET until D is archived |
| the venv also has `jsonschema` | `$VENV/bin/pip install jsonschema && $VENV/bin/python -c "import jsonschema"` → no error | MET (2026-11-16, in the session venv) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update
> every status before you continue, and again before you decide to stop. If you stop, report the current status
> of *all* prerequisites.

## 1. Goal

- Each drill file tool is preceded by `; #@! TA.AperFunction,<Plated,PTH|NonPlated,NPTH>,<function>`, where the
  function is `ViaDrill` for a via, `ComponentDrill` for a part's hole (and a header pad's), `MechanicalDrill`
  for a board's own hole (a standard's mounting hole, a placed hole, plated or not). Two functions of one
  diameter are two tools.
- `GerberExport.files` ends with `<base>-job.gbrjob` (also in the zip), JSON with `Header`, `GeneralSpecs`,
  `DesignRules`, `FilesAttributes` and `MaterialStackup` (§ 3), valid against the schema above.
- `KiCadBoardWriter` writes `(general (thickness T))` and a `(stackup …)` in `(setup` from `board.stackup`, with
  `T` the sum of what it writes; KiCad 10 reads the copper and dielectric thicknesses, `epsilon_r`, finish and
  mask colour back unchanged.

### Non-goals (explicit constraints)

- No IPC-4761 protection field on `ViaDrill` (plan-50-C). No `.N` on drill files.
- Tool numbering order otherwise stays (smallest diameter first); a diameter with one function keeps its number.
- The job file has no `CreationDate` (deterministic output), no `ImpedanceControlled`, no `LossTangent`.
- The EasyEDA Pro export is unchanged (no stackup in a format nothing here can verify).
- The 3-D preview's slab thickness and every mask / copper Gerber layer are unchanged.

## 2. Current State

- `DrillExport.holes(_:)` (`Document/DrillExport.swift:20`) walks `copper.pads` (`pad.holeSegment`; plated or not by
  `pad.plated`) then `copper.vias`, returning `(plated, bare)` of `Hole(a, b, diameter)`. `file(_:plated:layers:top:software:)`
  writes `M48`, the header comments (`FileFunction` as `; #@! TF.FileFunction,Plated,1,<layers>,PTH`), one
  `T<n>C<d>` per diameter (to 1 µm, ascending), `%`, `G90`, `G05`, hits then slots, `T0`, `M30`.
- `GerberExport.File` is `{name, data}` (`Document/GerberExport.swift:14`); `files(…)` returns them in the order
  copper, mask, paste, legend, outline, `-PTH.drl`, `-NPTH.drl` (only when bare holes exist).
- `KiCadBoardWriter.text()` writes `(general (thickness 1.6) (legacy_teardrops no))` and, in `(setup`,
  `(pad_to_mask_clearance …)` and `(allow_soldermask_bridges_in_footprints no)` only (`:127`–`:160`). It declares
  `(version 20240108)`. A 1-layer board is written with the two copper layers KiCad needs.
- Three tests list the zip's file names: `GerberExportTests.swift:24`, `:48` and `:192`.
- KiCad's own 4-layer `.gbrjob` is not schema-valid: `'0.02' is not of type 'number'` at
  `MaterialStackup[8].LossTangent` (it writes numbers as strings), while its 2-layer one is valid (checked with
  `jsonschema` 4.x against the schema, 2026-11-16). The app's job file writes numbers as numbers.

### Verified properties

- **KiCad 10.0.8 loads and re-saves a `(setup (stackup …))` in a `(version 20240108)` board**, keeping each
  `copper_finish` string (`"HAL lead-free"`, `"HAL SnPb"`, `"ENIG"`, `"OSP"`, `"None"`) and mask `color` string
  (`"Green"`, `"Black"`, `"Red"`, `"Blue"`, `"Yellow"`, `"White"`, `"Purple"`) it was given
  (`planning/plan-50-data/probe_kicad_stackup_roundtrip.py`).
- **The KiCad thickness is the sum including both masks**: 4-layer `BoardThickness 1.6062` = `BoardStackup.thickness`
  of the standard 4-layer stackup (plan-50-A).
- **The schema allows what § 3 writes**: top-level `Header`, `GeneralSpecs`, `MaterialStackup`, `DesignRules`,
  `FilesAttributes`; `GeneralSpecs` keys `ProjectId{Name,GUID,Revision}`, `Size{X,Y}`, `LayerNumber`,
  `BoardThickness`, `Finish`; `DesignRules[]` keys `Layers`, `PadToPad`, `PadToTrack`, `TrackToTrack`,
  `TrackToRegion`, `RegionToRegion`, `MinLineWidth`, `MinRing`, `MinClearanceToProfile`; `MaterialStackup[]`
  `Type` (a free string), `Thickness`, `Name`, `Material`, `DielectricConstant`, `Color`; `FilesAttributes[]`
  `Path`, `FileFunction`, `FilePolarity` (`Positive` | `Negative`).

## 3. Design Overview

**Drill functions.** `DrillExport.Hole` gains `function: DrillFunction` (`.via`, `.component`, `.mechanical`):
a via is `.via`; a pad hole with `entry.chip >= 0` or a label starting `Header.` is `.component`; every other
pad hole is `.mechanical`. Tools are keyed `(micrometres, function)`, ordered by diameter then
`via < component < mechanical`. Before each `T<n>C<d>` line: `; #@! TA.AperFunction,Plated,PTH,ViaDrill` (or
`NonPlated,NPTH,…`).

**File metadata.** `GerberExport.File` gains `function: String?` and `polarity: String?` (defaults nil; the
`==` it has for tests compares `name` and `data` only, as now). `files(…)` fills them for every Gerber layer
(the strings of its own `TF.FileFunction` / `TF.FilePolarity`) and the drill files
(`"Plated,1,<n>,PTH"`, `"NonPlated,1,<n>,NPTH"`, no polarity).

**Job file** (`Document/GerberJob.swift`, `enum GerberJob { static func data(…) -> Data }`), written with
`JSONSerialization` `.prettyPrinted, .sortedKeys` (deterministic), named `<base>-job.gbrjob`, appended last:

| Key | Value |
|---|---|
| `Header.GenerationSoftware` | `{Vendor: "jgz", Application: "Schematical", Version: <app version>}` (the same words as `TF.GenerationSoftware`) |
| `GeneralSpecs` | `ProjectId{Name: base, GUID: KiCad.uuid("gerber/<base>"), Revision: "1"}` (the same as the files' `.ProjectId`), `Size{X, Y}` (outline box, mm), `LayerNumber`, `BoardThickness` (`stackup.thickness`), `Finish` (`finish.kiCadName`) |
| `DesignRules` | `Outer`: `PadToPad`, `PadToTrack`, `TrackToTrack`, `TrackToRegion`, `RegionToRegion` = `minClearance`; `MinLineWidth` = `minTraceWidth`; `MinRing` = `minAnnularRing`; `MinClearanceToProfile` = `minCopperToEdge`. An identical `Inner` entry when `layers > 2` |
| `FilesAttributes` | one per file in the zip but the job: `Path`, and `FileFunction` / `FilePolarity` from `File` when present |
| `MaterialStackup` | top to bottom: `Legend` ("Top Silk Screen"), `SolderPaste` ("Top Solder Paste"), `SolderMask` (0.01, "Top Solder Mask", `Color` = `board.color.title`), then `Copper` (thickness, name = the layer's title), `Dielectric` (thickness, `Material` "FR4", `DielectricConstant`, name "<upper> to <lower>") alternating, then the bottom `SolderMask`, `SolderPaste`, `Legend`. A 1-layer board has the top group, its one copper and one dielectric ("Substrate"), and no bottom group |

**KiCad stackup.** In `(setup`, before `pad_to_mask_clearance`, KiCad's own layout (the template's, read from
`SharedSupport/template/Arduino_Micro/Arduino_Micro.kicad_pcb`): `F.SilkS`, `F.Paste`, `F.Mask` (`color`,
`thickness 0.01`), then `F.Cu`, `dielectric 1`, `In1.Cu`, …, `B.Cu` (each copper `(type "copper") (thickness t)`,
each dielectric `(type "core"|"prepreg") (thickness d) (material "FR4") (epsilon_r e) (loss_tangent 0.02)`), then
`B.Mask`, `B.Paste`, `B.SilkS`, `(copper_finish "<kiCadName>")`, `(dielectric_constraints no)`. A 1-layer board's
`B.Cu` is a 0.035 mm layer taken from its dielectric (`d − 0.035`), so the layer count KiCad needs is met and
only the bottom mask is added; `(general (thickness T))` is the sum of the written layers (1.61 mm for the
standard 1-layer board, 1.6 / 1.6062 / 1.566 for 2, 4, 6).

**Risk.** Low. The drill change moves tool numbers only where a via and a part hole share a diameter. The
KiCad change adds a block KiCad was probed with (above). Tests plus real tools are the gate (the schema
validator for the job file, KiCad 10 for the stackup); the expected diff is the new lines and, for such
boards, an extra tool.

**Rejected.** *Copying KiCad's job-file spellings (`SolderMask`, `SolderPaste`)*: `FileFunction` in the schema
is a free string and the spec's own values (`Soldermask`, `Paste`) are what the files carry, so the job names
each file by the string in the file itself. *KiCad's string-valued numbers*: not schema-valid. *Adding
`LossTangent`*: the app does not model it.

## Compatibility / Format Impact

- The zip gains `<base>-job.gbrjob` as its last entry; drill files gain comment lines and, where one diameter
  has two functions, a tool. KiCad boards gain a `stackup` and a changed `general thickness`.
- Unchanged: every layer Gerber, the file names, the saved file.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work. `- [~]` for
> partial, with one line on what remains. A moot task is `- [x] ~~text~~ — moot: <evidence>`. Fill the phase's
> `Commit:` line the moment it lands. An unticked box means NOT DONE.

### Phase 1 — Drill functions

- [ ] `Document/DrillExport.swift`: `DrillFunction`, `Hole.function`, the keying and the `TA` comment lines (§ 3);
      `holes(_:)` sets the function from the pad's entry and from vias.
- [ ] `docs/export.md` § Drill files: the new comment lines and the function rule.
- [ ] Tests, in `GerberExportTests`: `testDrillFilesSplitPlatedAndBareHoles` also checks `; #@! TA.AperFunction,Plated,PTH,ViaDrill`
      precedes `T1C0.400` and `NonPlated,NPTH,MechanicalDrill` precedes the bare tool; a board with a via of
      drill 0.8 and a part whose hole is 0.8 has two `T…C0.800` tools, via first; an `SN74HC00N` board's PTH file has
      every tool under `ComponentDrill`; a `.featherWing` board's header holes are `ComponentDrill`.

Acceptance: every tool in every drill file has its function line and equal diameters of different functions are
separate tools.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/GerberExportTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

### Phase 2 — The job file

- [ ] `Document/GerberExport.swift`: `File.function` / `polarity` and `files(…)` filling them and appending the
      job; `Document/GerberJob.swift` (§ 3).
- [ ] Update the three tests that list names (`GerberExportTests.swift:24`, `:48`, `:192`) for the new last entry
      `b-job.gbrjob` (the `:192` one asserts `suffix(2)`, now `["b-NPTH.drl", "b-job.gbrjob"]`).
- [ ] `docs/export.md` § Gerber: the file table row `-job.gbrjob`, and what it carries.
- [ ] `KiCadExportTests.testPrintSamplesWhenAsked`: also print `KICAD_FILE fourLayer/…` and `GERBER_FILE fourLayer/…` lines
      (the `vias` ones were added in plan-50-C and D) for `ExportSamples.fourLayer()` (plan-50-D).
- [ ] Tests: new `schematicalTests/GerberJobTests.swift`: the 2-, 4- and 6-layer standard boards' job files parse
      as JSON; `GeneralSpecs.BoardThickness` equals 1.6, 1.6062 and 1.566; the copper `Thickness` values in
      `MaterialStackup` equal the stackup's in order; one `FilesAttributes` entry per other file in the zip with
      the same `Path`; an edited stackup (finish ENIG, copper 0.07) shows in `Finish` and `Thickness`; two exports
      of one document give equal job bytes; a 1-layer board has no bottom group.

Acceptance: the job file is complete, deterministic, and schema-valid.
  Check: `xcodebuild … test -only-testing:schematicalTests/GerberJobTests -only-testing:schematicalTests/GerberExportTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
  And: `KICAD_OUT=1 … testPrintSamplesWhenAsked` (it prints `GERBER_FILE …job.gbrjob` for `ExportSamples.vias()` and `fourLayer()`, added to it in this phase), decode the lines into `$OUT`, then `curl -sSL -o $OUT/job-schema.json "https://www.ucamco.com/files/downloads/file_en/397/gerber-job-file-schema_en.json"` and `$VENV/bin/python -c "import json,jsonschema,sys; s=json.load(open('$OUT/job-schema.json')); [jsonschema.validate(json.load(open(f)), s) for f in sys.argv[1:]]; print('valid')" $OUT/vias-job.gbrjob $OUT/fourLayer-job.gbrjob` → `valid` (est. 4 min).
Commit: —

### Phase 3 — The KiCad stackup

- [ ] `Document/KiCadBoardWriter.swift`: the `(stackup …)` in `(setup` and `general thickness` per § 3, taken from
      `document.board.stackup` (a small `KiCad.stackup(for:)` helper in `KiCadExport.swift` if it reads better).
- [ ] `docs/export.md` § KiCad: a sentence — the board's stackup, finish and mask colour go to `(setup (stackup))`
      and the board thickness is its sum.
- [ ] Add `planning/plan-50-data/compare_kicad_stackup.py`: given a `.kicad_pcb` saved by KiCad and the expected
      copper thicknesses, dielectric (kind, thickness, εr) triples and finish, prints each mismatch (empty output =
      equal).
- [ ] Tests, in `KiCadExportTests`: the 2-layer board's text has `(thickness 1.6)` in `general` and a stackup with
      `F.Cu`, one `dielectric 1` (`core`, 1.51, 4.5), `B.Cu`; `fourLayer()` has `In1.Cu`, `In2.Cu`, three
      dielectrics (`prepreg`, `core`, `prepreg`) and `(thickness 1.6062)`; a 1-layer board has `(thickness 1.61)`
      and `B.Cu` 0.035; an edited stackup (finish `.enig`, color black) shows `(copper_finish "ENIG")` and
      `(color "Black")` on both masks; every number is written without a trailing `.0` problem (same formatter as
      the rest of the file, `KiCad.n`).

Acceptance: KiCad 10 reads the stackup back as written.
  Check: `xcodebuild … test -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/EasyEDAProExportTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 7 min).
  And: decode `KICAD_FILE fourLayer/fourLayer.kicad_pcb` into `$OUT`, run `probe_kicad_stackup_roundtrip.py $OUT/fourLayer.kicad_pcb $OUT/fourLayer-saved.kicad_pcb` with KiCad's python (command line in the probe's docstring), then `python3 planning/plan-50-data/compare_kicad_stackup.py $OUT/fourLayer-saved.kicad_pcb` → no output (est. 4 min).
Commit: —

## Validation Plan

- Tests: `GerberExportTests`, `GerberJobTests` (new), `KiCadExportTests`, `EasyEDAProExportTests`.
- Coverage check: Phase 2's name-list tests fail if the job file is dropped from `files(…)`; Phase 3's
  thickness assertions fail if `general thickness` stays 1.6 on the 4-layer board.
- Runtime proof: the schema validation of the real job files and KiCad 10's round trip (named in the phases).
- Doc sync: `docs/export.md` § Gerber, § Drill files, § KiCad.
- Final gate (once, after Phase 3): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/GerberExportTests -only-testing:schematicalTests/GerberJobTests -only-testing:schematicalTests/GerberAttributeTests -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/EasyEDAProExportTests -only-testing:schematicalTests/SchematicExportActionTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 9 min).

## Decisions

- The job file names each file's function by the string in the file itself — decided by the author: the schema leaves it free, and equal strings can be matched by any tool.
- The stackup goes to KiCad and the job file but not to EasyEDA Pro — decided by the author: KiCad 10 and the schema can check the first two; nothing here can open EasyEDA Pro.
- A 1-layer board is exported to KiCad with a carved 0.035 mm bottom copper — decided by the author: KiCad needs two copper layers (as the export already writes), and this keeps the substrate's total.

## Corrections

(Filled in during execution.)

## Summary

Low risk: new lines in the drill files and the KiCad board and one new file in the zip, each checked by the real
consumer (the schema, KiCad 10). Untouched: every layer's geometry, the saved file, EasyEDA, the stackup model.
