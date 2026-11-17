# plan-50-D: Gerber X2 Aperture and Net Attributes

Last updated: 2026-11-16
Effort: medium (1h–2h)
Depends on: plan-50-C (complete)

The Gerber export writes only file attributes (`FileFunction`, `FilePolarity`, `SameCoordinates`,
`GenerationSoftware`). A fab's CAM cannot tell a via pad from a component pad or a track from a pour, and nothing
says which net a piece of copper is on, so it cannot check the board against the netlist. This sub-plan adds the
attributes the Gerber spec asks for on copper: an aperture function on every copper object and a net (and, for
part pads, component pin) on every one.

The single outcome: every copper layer file of a board carries `.AperFunction` on each aperture and region and
`.N` on each object (`.P` on part pads), with net names equal to the KiCad export's, and every Gerber file has a
`.ProjectId` and `.Part`; and an independent reader (gerbonara) sees the same (reference, pin) → net map as KiCad
10's own Gerber output for the same board.

References:

- plan-50-A (prerequisites), plan-50-C
- Gerber Layer Format Specification Revision 2026.05, §§ 3.4.3–3.4.4 (escaping), 5.3–5.5 (attributes), 5.6.10
  (`.AperFunction`), 5.6.13 (`.N`, `.P`, `.C`), 5.6.8 (`.ProjectId`); the facts used are in
  `planning/plan-50-data/README.md`
- `schematical/schematical/Document/GerberWriter.swift`, `GerberExport.swift`, `KiCadBoardWriter.swift`
  (`assignNets`), `KiCadExport.swift` (`ExportNetNames`)
- `docs/export.md` § Gerber

## Prerequisites

The shared gate is in plan-50-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-50-C is complete | `ls planning/completed \| grep -c '^plan-50-C'` → 1 | MET (2026-11-16: 1) |

Everything below is written against the world where this holds.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update
> every status before you continue, and again before you decide to stop. If you stop, report the current status
> of *all* prerequisites.

## 1. Goal

- A shared `ExportNetNames.assign(context:)` gives the net of every pad (including pins with no wire, which get
  their own net) and each net's unique name; the KiCad writer uses it and its output does not change.
- `GerberWriter` can attach an aperture function to apertures and regions, and object attributes (`.N`, `.P`) to
  flashes, draws and regions, escaping field text per § 3.4.3.
- Copper files (`.GTL`, `.G1`…, `.GBL`) carry, per object:

  | Object | `.AperFunction` | `.N` | `.P` |
  |---|---|---|---|
  | surface-mount part pad | `SMDPad,CuDef` | its net | `<ref>,<pin>` |
  | unnumbered surface-mount part pad (centre pad) | `HeatsinkPad` | its net, else empty | `<ref>,` with an empty pin (`%TO.P,U1,*%`, as § 5.6.13's example) |
  | plated through-hole part pad, header pad | `ComponentPad` | its net | `<ref>,<pin>` (header: `HEADER1`) |
  | card edge finger | `ConnectorPad` | its net | `EDGE1,<finger name>` |
  | plated board hole land | `OtherPad,PlatedHole` | its net, else empty | — |
  | via | `ViaPad` | its net | — |
  | trace, Plane region | `Conductor` | its net (a Plane's net) | — |
  | clear-polarity clearances | none | none | — |

  A net with no pad or an empty net is `%TO.N,*%` (the spec's "no net").
- Every Gerber file (copper, mask, paste, legend, profile) has `%TF.ProjectId,<base>,<guid>,1*%` and
  `%TF.Part,Single*%`. The outline's pen has `%TA.AperFunction,Profile*%`.

### Non-goals (explicit constraints)

- Mask, paste and legend files get no per-object attributes (only the two file attributes above).
- No `.C`, `.CRot` or other component attributes; no `.MD5`; no `.CreationDate` (the output stays deterministic:
  one document always gives the same bytes).
- Drill files and the job file are plan-50-E. The EasyEDA export is untouched.
- Every file's geometry, file names and the zip layout are unchanged. The only new lines are attribute lines and
  the two new file attributes.

## 2. Current State

- `GerberWriter` (`Document/GerberWriter.swift`) builds one file: an `Aperture` enum (`circle`, `rect`) keys a
  dictionary so equal shapes share a code; primitives `flash`, `stroke`, `region`, `fill`, `glyph`; `data()` writes
  `G04`, the `%TF.<attributes>*%` lines, the format lines, one `%ADD…*%` per aperture, the body, `M02*`.
- `GerberExport.files(for:base:rules:)` (`Document/GerberExport.swift:30`) makes a `GerberWriter(attributes:top:)`
  per layer through a local `writer(_:polarity:)` function and `drawCopper` (`:99`) draws a layer's Plane,
  traces, pads and vias.
- `BoardConnectivity` (`Canvas/Ratsnest.swift:399`): `pads` (each with `chip` index or −1, `pad`, `side`,
  `label`, `hole`), `traces`, `vias`, `root`, `netOfPad`, `planeNets`, and node helpers `traceNode(_:)`,
  `viaNode(_:)`; `netOfRoot()` maps a root node to a net.
- `KiCadBoardWriter.assignNets()` (`Document/KiCadBoardWriter.swift:84`–`:118`) computes the pad nets (adding a
  net for each numbered pin no wire reaches, `unconnected-(U1-A0-Pad1)`), the unique names (`name`, `name_2`, …)
  and KiCad's integer codes, from `ExportNetNames` (`Document/KiCadExport.swift:364`) and
  `KiCadExport.Context`. The KiCad board's tracks and vias read the nets with `copper.netOfRoot()` and
  `copper.root[copper.traceNode(t)]` (`:482`–`:510`).
- `KiCad.uuid(_:)` (`KiCadExport.swift:319`) gives a deterministic RFC 4122 version-4 UUID from a string.
- KiCad 10.0.8's own Gerber for a fixture has, per object, `%TO.P,<ref>,<pin>*%` then `%TO.N,<net>*%` before a
  flash, and `%TA.AperFunction,SMDPad,CuDef*%` / `ViaPad` / `Conductor` / `ComponentPad` / `HeatsinkPad` before
  each `%ADD…*%` (`probe_gerber_attributes.py` output in the README; reference files made with `kicad-cli pcb
  export gerbers` on an exported board).

### Verified properties

- **The spec lets regions and plain apertures carry the function.** § 5.3: aperture attributes in the dictionary at
  `%ADD` are attached to that aperture, and at `G36` to the region; they cannot change afterwards; commands are not
  allowed inside a region (§ 5.4). So `%TA…*%` goes before the `%ADD` and before `G36`, and `%TO…*%` between objects.
- **Net values must follow the field rule**: `,` `*` `%` `\` escape as `,`, `*`, `%`, `\`
  (§ 3.4.3–3.4.4). KiCad net names like `/{slash}AFU` and `Net-(U1-A0)` contain none, but a net label can.
- **gerbonara 1.6.3 reads these attributes**: on KiCad's own `AFU-F_Cu.gbr` its objects show
  `attrs {'.P': ('R1', '1'), '.N': ('/{slash}AFU',)}` and aperture attrs `(('.AperFunction', ('SMDPad', 'CuDef')),)`.

## 3. Design Overview

**Net names.** Move the body of `assignNets()` into `ExportNetNames.assign(context:) -> ExportNets`
(`padNets: [Int: Int]`, `names: [Int: String]`, `codes: [Int: Int]`); `KiCadBoardWriter` stores the result in
the three properties it has now. The Gerber export builds `KiCadExport.Context(document:base:)` (the same call the
EasyEDA export makes, `Document/EasyEDAProExport.swift:160`) and calls the same function, so a net's Gerber name is
its KiCad name.

**Writer.** `GerberWriter.Aperture` gains the function: `.circle(Int, function: String?)`, `.rect(…)`. New
parameters `function: String? = nil` on `flash`-ing calls (`fill`, `stroke`, `region`) and
`object: GerberObject? = nil` (`struct GerberObject { var net: String?; var pin: (ref: String, number: String)? }`).
`data()` writes, in the aperture list, `%TA.AperFunction,<f>*%` before an aperture that has one and `%TD.AperFunction*%`
after it; the body writes `%TA.AperFunction,<f>*%` before `G36*` and `%TD.AperFunction*%` after `G37*`, and writes
object attributes before an object only when the set differs from the one in force (`%TO.N,<net>*%`,
`%TO.P,<ref>,<pin>*%`; changed or removed ones via a new `%TO` or `%TD.<name>*%`). Entering clear polarity
(`setDark(false)`) first removes every attribute; `setDark(true)` does not restore them (the next object does).
`func fieldEscaped(_ text: String) -> String` implements the four escapes.

**Header.** `writer(_:polarity:)` adds `ProjectId,<base>,<KiCad.uuid("gerber/<base>")>,1` and `Part,Single`.

**Where each object's net comes from** (the KiCad writer's own lookups): a pad's from `padNets[index in copper.pads]`;
a trace's from `copper.netOfRoot()[copper.root[copper.traceNode(t)]]`; a via's from `copper.viaNode(v)`; a
Plane's from `copper.planeNets[layer]`. The name is `names[net]`.

**Reference and pin of a pad.** `entry.chip >= 0`: `document.chips[entry.chip].reference` and `pad.designator`
(nil = the unnumbered centre pad). `entry.chip == -1` with a label starting `Edge.` or `Header.`: reference
`EDGE1` or `HEADER1` (the names the KiCad export gives the connector), pin the text after the dot. Other board
pads (mounting holes, drilled holes) have no `.P`.

**Risk.** The `assignNets` extraction touches the KiCad writer, so its output must not move: the gate is a
before/after comparison of every KiCad fixture file. The Gerber change itself only adds lines. This is a behaviour
change with tests as the gate; the expected diff is exactly the new attribute lines.

**Rejected.** *Net names from the `N/C` shortcut for single-pad nets*: KiCad names them and the names match the
KiCad export. *Per-object attributes on mask, paste and legend files*: nothing the fab needs and a large diff.
*`.CreationDate`*: it makes every export differ; the export is deterministic today.

## Compatibility / Format Impact

- Gerber files gain attribute lines and two file attributes; coordinates, apertures' shapes and codes, file names
  and the zip's order do not change. Existing assertions on file text that match a fragment (`C,0.254000*%`,
  `X10000000Y20000000D02*`) still hold.
- `ExportNetNames.assign` replaces private code in the KiCad writer; no KiCad output changes.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work. `- [~]` for
> partial, with one line on what remains. A moot task is `- [x] ~~text~~ — moot: <evidence>`. Fill the phase's
> `Commit:` line the moment it lands. An unticked box means NOT DONE.

### Phase 1 — One set of net names

- [x] Before touching code, record the KiCad baseline: run `KICAD_OUT=1 xcodebuild … test -only-testing:schematicalTests/KiCadExportTests/testPrintSamplesWhenAsked`
      and save the `KICAD_FILE` lines (name and base64) to `$OUT/before.txt`.
- [x] `Document/KiCadExport.swift`: add `struct ExportNets` and `ExportNetNames.assign(context:) -> ExportNets`
      (the body of `assignNets()`, parameterised on the context); `Document/KiCadBoardWriter.swift` calls it.
- [x] Tests: `KiCadExportTests` unchanged and green; new `testNetNamesAreShared` calls `ExportNetNames.assign` on
      `ExportSamples.vias()` and checks the names equal those in the KiCad board text's `(net <n> "<name>")` lines.

Acceptance: KiCad output is byte-identical.
  Check: rerun the baseline command after the change into `$OUT/after.txt`; `diff $OUT/before.txt $OUT/after.txt` → no output; then `xcodebuild … test -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/EasyEDAProExportTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 10 min: two test runs). Observed: `before.txt` and `after.txt` (the 12 `KICAD_FILE` lines of the fixtures, `turned` and `vias`) differ only in the random part UUIDs of the two boards the test builds itself (`turned`, `vias`; compared after replacing every UUID: identical); the three fixture projects (AFU, LFU, RegFile: 9 files) and `turned/app.drl` are byte-identical; `KiCadExportTests` and `EasyEDAProExportTests` `** TEST SUCCEEDED **`.
Commit: 69335a9f

### Phase 2 — The writer

- [x] `Document/GerberWriter.swift`: the function-keyed `Aperture`, the `function:` / `object:` parameters, the
      `%TA` / `%TD` / `%TO` output and the clear-polarity rule (§ 3), `GerberObject`, and `fieldEscaped`.
- [x] Tests: new `schematicalTests/GerberAttributeTests.swift` with a small reader helper `GerberAttributeReader`
      that walks a file's lines and yields, for each `D01`/`D03`/`G36` object, the aperture function in force
      (from the `%TA` before its `%ADD`, or before `G36`) and the object attributes in force; cases: two circles of
      one diameter and different functions get two apertures (`%ADD10`, `%ADD11`) each with its own `%TA`;
      a flash after `%TO.N,GND*%` is read with net `GND`; a region carries the function set before `G36*`; no
      `%T…` line sits between a `G36*` and its `G37*`; clear polarity drops attributes; `fieldEscaped("a,b*c%d\\e")`
      == `a,b*c%d\e`; a file with no attributes is byte-identical to today's output
      (compare `GerberWriter` output for a fixed drawing with a golden string made before the change).

Acceptance: attributes are written and read back as set; a file with none is unchanged.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/GerberAttributeTests -only-testing:schematicalTests/GerberExportTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min). Observed (run with Phase 3's suites): `Executed 52 tests, with 0 failures`.
Commit: 69335a9f

### Phase 3 — Copper layers, profile and file attributes

- [x] `Document/GerberExport.swift`: build the context and `ExportNets` once in `files(…)`; pass the net table,
      document and copper to `drawCopper`; give every pad, trace, via and Plane the function and object of the
      table in § 1 (the lookups in § 3); `writer(…)` adds `ProjectId` and `Part`; the profile's `stroke` uses
      function `Profile`.
- [x] `ExportSamples` (`KiCadExportTests.swift`): add `fourLayer()` — the `vias()` board set to 4 layers, its two
      inner layers Planes on ground and on `power:5000`, plus an `SN74ABT125RGYR` (footprint `VQFN-14`, whose centre
      pad is unnumbered: `python3` over `footprints.json` finds 96 footprints with a `centre` pad and no slot 0, and
      `buffer.json` lists `SN74ABT125RGYR` on `VQFN-14`). `vias()` already leaves most of its `SN74HC00N` pins unwired.
- [x] `docs/export.md` § Gerber: a paragraph and the § 1 table (the attributes on copper), the two file
      attributes, the net names ("the same as the KiCad export's"), and in the file table the Profile function.
- [x] Add `planning/plan-50-data/compare_gerber_nets.py`: given our `.gbr` and KiCad's,
      prints the (reference, pin) → net maps that differ (empty output = equal) and the counts of each
      `.AperFunction`.
- [x] Tests, in `GerberAttributeTests`, with `GerberAttributeReader` over `GerberExport.files(for: ExportSamples.vias())`:
      every object on `.GTL` and `.GBL` has a net (`%TO.N` present, possibly empty); each `SMDPad,CuDef` and
      `ComponentPad` flash has a `.P` whose reference is a chip reference of the document; the number of `ViaPad`
      flashes on `.GTL` equals the vias' count (4) and each `ViaPad` net equals the net of the trace it touches;
      the `SN74ABT125RGYR`'s centre pad is `HeatsinkPad` with `%TO.P,<ref>,*%`; on a board set to `.pci` the card's
      fingers are `ConnectorPad` with `.P` `EDGE1,<finger name>`, and on `.featherWing` the header pads are
      `ComponentPad` with `HEADER1`;
      traces are `Conductor` with the trace's net; on `fourLayer()`'s `.G1`/`.G2` the Plane region is
      `Conductor` with `GND` / `5V`, and the clearance flashes after `%LPC*%` have no `%TO` in force; every file has
      `%TF.ProjectId,` and `%TF.Part,Single*%`; `.GKO` has `%TA.AperFunction,Profile*%` before its `%ADD`; two
      exports of one document are byte-identical.

Acceptance: copper carries the attributes of § 1 and the files stay deterministic.
  Check: `xcodebuild … test -only-testing:schematicalTests/GerberAttributeTests -only-testing:schematicalTests/GerberExportTests -only-testing:schematicalTests/KiCadExportTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 7 min). Observed: `Executed 52 tests, with 0 failures`, `** TEST SUCCEEDED **`.
  And the cross-check: run `KICAD_OUT=1 … testPrintSamplesWhenAsked`, decode the `GERBER_FILE vias/…GTL` and `KICAD_FILE vias/vias.kicad_pcb` lines into `$OUT`; `~/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli pcb export gerbers --board-plot-params -o $OUT/kicad $OUT/vias.kicad_pcb`; `$VENV/bin/python planning/plan-50-data/compare_gerber_nets.py $OUT/vias.GTL $OUT/kicad/vias-F_Cu.gbr` → no differing (reference, pin) → net entries, and the `.AperFunction` counts line up for `SMDPad`, `ComponentPad`, `ViaPad` (est. 5 min). Observed with `kicad-cli pcb export gerbers` 10.0.8 and gerbonara 1.6.3 on `ExportSamples.vias()` and `fourLayer()`: `compare_gerber_nets.py` printed no `DIFFERENT` line for Top or Bottom of either board; function counts Top of `fourLayer`: ours `Conductor 2, ComponentPad 28, SMDPad 14, HeatsinkPad 1, ViaPad 4`, KiCad `ComponentPad 28, SMDPad 15, ViaPad 4, Conductor 2` (KiCad calls the centre pad `SMDPad`).
Commit: 69335a9f

## Validation Plan

- Tests: `GerberAttributeTests` (new), `GerberExportTests`, `KiCadExportTests`, `EasyEDAProExportTests`.
- Coverage check: the Phase 3 reader test fails if `drawCopper` stops passing the function for any one object
  kind (delete one in a scratch run and see the named object kind fail).
- Runtime proof: the KiCad cross-check above (gerbonara reading our file against KiCad 10's own).
- Doc sync: `docs/export.md` § Gerber (Phase 3).
- Final gate (once, after Phase 3): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/GerberAttributeTests -only-testing:schematicalTests/GerberExportTests -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/EasyEDAProExportTests -only-testing:schematicalTests/SchematicExportActionTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min).

## Decisions

- The Gerber net names are the KiCad export's names — decided by the author: one name for a net in every file the fab and the designer see.
- Copper only gets per-object attributes — decided by the author: § 5.6.13 asks for `.N` on conducting objects, and the spec's own "bare minimum" is to identify the via pads.
- No creation date — decided by the author: a re-export of an unchanged board stays byte-identical, as the EasyEDA export does.

## Corrections

- **Net name extraction left the KiCad output unchanged**, but two of the twelve baseline files cannot be compared byte for byte: the boards the test builds itself (`turned`, `vias`) get random part UUIDs per run (`TestSupport.newDocument`), which KiCad footprint paths carry. They were compared with every UUID replaced.
- **The function value is not a field to escape.** The first version escaped the function string with `fieldEscaped`, giving `SMDPad\u002CCuDef`; the gerbonara cross-check showed the function as the single value `SMDPad\u002CCuDef` instead of `(SMDPad, CuDef)`. A function is already several comma-separated fields (spec § 5.6.10); only net and pin text is escaped. Fixed in `GerberWriter`, and `GerberAttributeReader` no longer unescapes functions, so the tests would catch it.
- The sub-plan's final gate is run once for all of plan-50 at the end (see plan-50-A Corrections).

## Summary

The risk is in moving net naming out of the KiCad writer without changing it; the before/after diff of every
KiCad fixture file is the guard. Untouched: every geometry line, file names, the zip, the drill files and the
job file (plan-50-E), mask/paste/legend objects.
