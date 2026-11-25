# plan-48-G: Parts — the two generics, timers, op-amps; datasheets, footprints, goldens, the editor's kind lists

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-48-F

G writes `Parts/timer.json` and `Parts/opamp.json` from the final pick, adds
their datasheet entries and the footprints they use, regenerates the
goldens, and registers both kinds in the Part Editor's lists and schemas.

References:

- plan-48-A / -B (`candidates.tsv` `add` rows, `specs.tsv`, the PDFs and
  `datasheets.tsv` files), plan-48-C / -D (`footprints-C.json`,
  `footprints-D.json`, `footprint-map.tsv`), plan-48-E § 3.1 and plan-48-F
  § 3.1 (the two formats).
- `docs/parts-format.md` § Devices, § Parts, § Datasheets; `docs/backend.md`
  § Adding a kind step 6 (the Part Editor), § Tests (the goldens).
- `planning/plan-40-data/census/gen-parts.py`, `fp-merge.py` (the scripts G
  adapts); `scripts/part-editor-schema.py`.

## Prerequisites

plan-48-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-48-F complete | `ls planning/completed/plan-48-F-*.md` → exists, or every F box ticked | MET (2026-11-24: every F box ticked) |
| Every `add` row is mapped to a footprint and a pinout | `python3 -I planning/plan-48-data/census/fp-check.py C` and `… D` → `0 without a footprint`; `python3 -I planning/plan-48-data/census/pinout-check.py` → `0 errors` | MET (2026-11-24: `fp-check.py C` → `58 lines, 0 without a footprint`, `fp-check.py D` → `28 lines, 0 without a footprint` (and `0 spec rows whose pin designators differ`); `pinout-check.py` → `503 rows, 0 errors`) |
| Golden sizes at G's start | `wc -l backend/tests/golden/parts.txt backend/tests/golden/traces.txt backend/tests/golden/footprints.txt` | recorded here at G's start as **P1**, **R1**, **F1** (P0 32897, R0 32897, F0 1646 at planning; other plans may have landed parts): **P1 = 39903, R1 = 39903, F1 = 3825** (2026-11-24, at 0b5788f5 + plan-48 A–F) |
| The part files are absent | `ls schematical/schematical/Parts/{timer,opamp}.json` → no such file | MET (2026-11-16; 2026-11-24 before this letter) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `Parts/timer.json` holds `GENERIC-555` and one part per timer `add` row;
  `Parts/opamp.json` holds `GENERIC-OPAMP` and one part per op-amp `add`
  row (at planning time 62 and 600 picked rows).
- `backend/tests/golden/parts.txt` and `traces.txt` grow by exactly those
  parts (P1 + 2 + the `add` rows); `footprints.txt` grows by exactly the
  objects of `footprints-C.json` and `footprints-D.json`; no golden line is
  removed or changed.
- `Parts/datasheets.json` has an entry for every PDF an `add` row cites.
- The Part Editor knows both kinds: `PartKindRoundTripTests` passes with
  `testTimer` and `testOpamp`.

### Non-goals (explicit constraints)

- No change to an existing part, footprint or datasheet entry.
- No hand-written part: every part comes from `gen-parts.py`; a wrong part
  is fixed in `specs.tsv`, `footprint-map.tsv` or the script.
- No app view changes beyond the editor's lists (H).

## 2. Current State

`Parts/` has 34 JSON files (32 part files, `datasheets.json`, `footprints.json`) and no `timer.json` or `opamp.json`. The drawer's
categories come from the devices' `"drawer"` paths, sorted by name
(`Backend/PartCatalog.swift`, `groups(matching:)`); the only `"Analog"` parts are the 767
ADC and DAC converters of the drawer regrouping, listed at `["Analog", "Data converters"]`
(`grep -l '"Analog"' schematical/schematical/Parts/*.json | wc -l` → 1, `converter.json`;
`grep -c '"Analog"' …/converter.json` → 767); the timers and op-amps join that category as
the groups Timers and Op-amps. None of the 330 datasheet ids G adds exists
(`datasheets.json` ids ∩ the picked rows' PDF names → 0 at planning time).
A part's flag for an AEC-Q100 grade is `"automotive"`
(`docs/parts-format.md:224`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Picked rows | timer 62, op-amp 600 | plan-48-A § 2 |
| …AEC-Q100 (flag `automotive`) | timer 3, op-amp 118 | `awk -F'\t' 'NR>1 && $22=="candidate" && $11!=""{n[$3]++} END{for(k in n)print k,n[k]}' planning/plan-48-data/candidates.tsv` |
| Distinct datasheet names | 330 (of 331 PDFs: the ADI and Renesas ICM7555 documents have different names) | `python3 -I` over `candidates.tsv` column 21, lower case, `.pdf` dropped |
| Existing datasheet ids among them | 0 | the same script ∩ `Parts/datasheets.json` |
| Editor kinds today | 30 kind names in `KIND_FILES` | `python3 -I -c "import re;s=open('scripts/part-editor-schema.py').read();print(len(re.findall(r'\"(\\w+)\": \\[', s.split('KIND_FILES = {')[1].split('}')[0])))"` → 30 |

## 3. Design

### 3.1 Devices and parts (`census/gen-parts.py`, from plan-40's)

- **One device per spec group**: (type, company, PDF, circuits, and for
  op-amps the DigiKey amplifier type). Its `pinouts` are the group's spec
  rows (one per package: the named scheme expanded, or the explicit
  pinout), keyed `<label lower case>-<pins>` (`soic-8`, `sot-23-5`), with a
  `-b`, `-c` suffix when two packages of one label and pin count differ in
  pinout.
- **Ids**: `<datasheet id>-<circuits>` (`na555-1`, `lm358-2`); with the
  amplifier type appended when one PDF holds two types.
- **Drawer and function**: timers `["Analog", "Timers"]`, function `"Single
  (555)"` or `"Dual (556)"`; op-amps `["Analog", "Op-amps", "Single" |
  "Dual" | "Quad"]`, function `"General purpose"`, `"JFET input"` or
  `"CMOS"`. `description`: `"<base name> timer, <process>"` /
  `"<base name> <dual> op-amp, <type>"`.
- **Timer device keys**: `process`, `timers` (from the pinout's names),
  `maxHz`, `tpd`, `vcc` from the spec row's `facts`.
- **Op-amp device keys**: `type` from the class (`gp` → `general`),
  `amplifiers` (from the pinout's names), `vcc` and `inputMax` from the spec
  row; `gbwHz`, `slewVus`, `railIn`, `railOut` from the device's
  most-stocked row's DigiKey parameters (Gain Bandwidth Product, Slew Rate,
  Output Type containing `Rail-to-Rail`; `railIn` when the Amplifier
  description says rail-to-rail input); `vosMv` from Voltage - Input Offset,
  on the device when all its parts agree, else on each part.
- **Parts**: `name` (the part number), `manufacturer` (`Texas Instruments`,
  `Analog Devices`, `STMicroelectronics`, `Renesas Electronics`),
  `datasheet` (the id), `package` (the spec row's label), `pinout`,
  `footprint` (`footprint-map.tsv`), `"flags": ["automotive"]` for an
  AEC-Q100 row. A part's `vcc` only when its PDF gives its grade another
  range than the device's.
- **Pin roles**: by name — `VCC`, `V+` `power`; `GND`, `V-`, `EP` `ground`;
  `OUT…`, `DISCH…` `output`; `NC` `nc`; every other name `input`.
- **Order**: devices by company, then datasheet id; parts by name; the
  generic device last in each file.

### 3.2 The generics

| Part | Device | Facts | Footprint |
|---|---|---|---|
| `GENERIC-555` | id `timer-generic`, drawer `["Analog", "Timers"]`, function `"Generic"`, pinout `dip-8` = `555-8` | `process` `bipolar`, `maxHz` 500000, `tpd` 100, `vcc` [4.5, 16] | `PDIP-8`; `footprintChoices` `DIP` → `PDIP-8`, `SOIC` → `SOIC-8` |
| `GENERIC-OPAMP` | id `opamp-generic`, drawer `["Analog", "Op-amps"]`, function `"Generic"`, pinout `dip-8` = `dual-8` | `type` `general`, two `amplifiers`, `gbwHz` 1000000, `vcc` [3, 32] | `PDIP-8`; `footprintChoices` `DIP` → `PDIP-8`, `SOIC` → `SOIC-8` |

Both have `"manufacturer": "Generic"` and no datasheet.

### 3.3 Datasheets and footprints

- `datasheets.json`: one entry per PDF an `add` row cites, `id` = the
  file's name in lower case without `.pdf`, `url` = the folder's
  `datasheets.tsv` URL (TI in the `https://www.ti.com/lit/ds/symlink/<name>.pdf`
  form); an id that exists is not added again.
- `footprints.json`: `fp-merge.py --only <ids>` appends the objects of
  `footprints-C.json` / `-D.json` the phase's parts name; an id that is
  already in `footprints.json` (another plan added it) is skipped and
  recorded in § Corrections.

### 3.4 The editor (plan-46's rule, `docs/backend.md` § Adding a kind step 6)

Each added only if absent (`grep -c '"timer"\|"opamp"'
schematical/schematical/Editor/KindSchema.swift
schematical/schematical/Editor/KindInfo.swift` → 0 before):

- `Editor/KindInfo.swift`: `KindInfo(kind: "timer", name: "Timer", blurb:
  "555-type timers: one or two timer sections that take their timing from a
  resistor and a capacitor.", examples: ["GENERIC-555", "NE555P"], group:
  .power)` and `KindInfo(kind: "opamp", name: "Op-amp", blurb: "Operational
  amplifiers with one, two or four amplifiers: LM358, TL072. Not simulated;
  they drive nothing in play mode.", examples: ["GENERIC-OPAMP", "LM358P"],
  group: .other)`; both names in `PartKinds.all` and `infoOrder`. An
  example part that is not an `add` row is replaced by the device's
  most-stocked `add` row.
- `Editor/BundledPartFile.swift` `PartKinds.name(of:)`: `case PART_TIMER:
  "timer"`, `case PART_OPAMP: "opamp"`.
- `scripts/part-editor-schema.py` `KIND_FILES`: `"timer": ["timer.c"]`,
  `"opamp": ["opamp.c"]`; the same in
  `schematicalTests/PartKindKeysTests.swift` `files`.
- `python3 scripts/part-editor-schema.py . timer` and `… opamp` →
  `missing []` → `Editor/TimerDraft.swift`, `Editor/OpampDraft.swift`,
  added to `PartKinds.schemas`.
- `PartKindRoundTripTests`: `testTimer`, `testOpamp` with the shipped part
  counts (`grep -o '"manufacturer"' schematical/schematical/Parts/timer.json | wc -l`).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The generics and the editor's kind lists

- [x] `census/gen-parts.py` per § 3.1–3.3; `--generics` writes the two files
      with only the generic devices (`gen-parts.py --generics` → `1 device, 1 part (the generic)` per file; the full run followed at once).
- [x] The editor steps of § 3.4 (Corrections 4).
- [x] Regenerate the goldens (`listing_check --print >
      backend/tests/golden/parts.txt`, `trace_check --print > …/traces.txt`)
      and review `git diff --stat`.

Acceptance: the two generics load; goldens grow by 2 and 2 lines; the editor
round-trips both.
  Check: `cmake -S backend -B /tmp/logic-p48-g && cmake --build /tmp/logic-p48-g -j8 && ctest --test-dir /tmp/logic-p48-g -j8` → `100% tests passed`; `git diff --stat backend/tests/golden` → `+2`, `+2`; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0; `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p48-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/WizardStepsTests -only-testing:schematicalTests/KindGuideTests` → `** TEST SUCCEEDED **` (est. 14 min: backend build 4, app build and four test classes 10).
  Result: the generics, the timers and the op-amps were generated together (one run per kind), so the goldens grew once, by the whole pick (see Phase 3); `ctest` → `100% tests passed, 0 tests failed out of 67`; the app: `PartKindRoundTripTests`, `PartKindKeysTests`, `WizardStepsTests`, `KindGuideTests`, `PartEditorWindowTests` → `** TEST SUCCEEDED **` (after Corrections 4's count updates). MET.
Commit: —

### Phase 2 — Timers

- [x] `gen-parts.py --type timer`: the timer `add` rows, their
      `datasheets.json` entries, `fp-merge.py --only` the footprints they
      name; regenerate the three goldens (`footprint_check --print >
      …/footprints.txt`).

Acceptance: the timers load and the goldens grow by exactly them.
  Check: `ctest --test-dir /tmp/logic-p48-g -j8` after a rebuild → `100% tests passed`; `grep -c '|timer|' backend/tests/golden/parts.txt` → 1 + the timer `add` rows (`awk -F'\t' 'NR>1 && $22=="add" && $3=="timer"' planning/plan-48-data/candidates.tsv | wc -l`); `git diff backend/tests/golden | grep -c '^-[^-]'` → 0 (est. 6 min).
  Result: `gen-parts.py --type timer` → `15 devices, 57 parts; 13 datasheets used, 13 added to datasheets.json`; `gen-parts.py --verify` → `timer: 56 add rows, 57 parts (1 generic)`; `grep -c '|timer|' parts.txt` → 57; removed or changed golden lines 0. MET.
Commit: —

### Phase 3 — Op-amps and the docs

- [x] `gen-parts.py --type opamp`: the op-amp `add` rows, their datasheet
      entries, the remaining footprints; regenerate the goldens.
- [x] `node scripts/check-datasheets.js --host www.ti.com`, `--host
      www.analog.com`, `--host www.st.com`, `--host www.renesas.com` → no
      failure for the new entries (run once on the 292 new ids: `node scripts/check-datasheets.js --jobs 4 <ids>` → `292 of 292 URLs serve a PDF; 0 failed`).
- [x] `PartKindRoundTripTests`: the two counts re-set to the shipped ones.
- [x] Docs: `docs/parts-format.md` — the `### timer` and `### opamp`
      sections gain their library paragraphs (counts, drawer paths, the
      generics and their footprint choices).

Acceptance: the whole pick loads; goldens grow by exactly it; docs name it.
  Check: `ctest --test-dir /tmp/logic-p48-g -j8` after a rebuild → `100% tests passed`; `grep -c '|opamp|' backend/tests/golden/parts.txt` → 1 + the op-amp `add` rows; `wc -l < backend/tests/golden/footprints.txt` → F1 + the objects merged; `git diff <G's first commit>^ -- backend/tests/golden | grep -c '^-[^-]'` → 0; `xcodebuild … test -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/DatasheetCatalogTests -only-testing:schematicalTests/DrawerPerformanceTests` → `** TEST SUCCEEDED **` (est. 14 min: backend build 4, app tests 10).
  Result: `gen-parts.py --type opamp` → `378 devices, 601 parts; 279 datasheets used, 279 added`; `gen-parts.py --verify` → `opamp: 600 add rows, 601 parts (1 generic)`; `grep -c '|opamp|' parts.txt` → 601; `wc -l`: parts 40561 (P1 39903 + 658), traces 40561, footprints 3859 (F1 3825 + 34); removed or changed golden lines 0 in all three; `ctest` → `100% tests passed, 0 tests failed out of 67`; `xcodebuild … test` with the four classes named here (and the Phase 1 five): `** TEST SUCCEEDED **`. MET.
Commit: —

## Validation Plan

- Tests: `listing_check`, `trace_check`, `footprint_check` against the
  regenerated goldens; `library_check`; the app's editor and catalog tests
  named in the phases.
- Coverage check: the goldens' line counts against the `add` rows (the
  phase checks); `python3 -I planning/plan-48-data/census/gen-parts.py
  --verify` → every `add` row is a part, every part an `add` row or a
  generic.
- Runtime proof: plan-48-H's harness on the real catalog.
- Doc sync: `docs/parts-format.md`.
- Final gate: plan-48-I.

## Decisions

- Author: **drawer paths** Analog → Timers and Analog → Op-amps → Single /
  Dual / Quad, with the amplifier type as the leaf.
- Author: **two generics**, an 8-pin 555 and a dual op-amp, each with DIP
  and SOIC footprint choices.
- Author: **every orderable packing is its own part** (plan-48-A § 2).
- Author: **AEC-Q100 rows carry `"flags": ["automotive"]`**, so the drawer's
  Automotive filter covers them.
- Author: **timers in the editor's "Power and clocks" group, op-amps in
  "Other"**.

## Corrections

1. **Counts.** 56 timer and 600 op-amp `add` rows (B's final pick), not 62 and 600; 15 and 378 devices
   (generics included); 292 datasheet ids added (13 timer, 279 op-amp; plan: 330 of 331 PDFs). The three goldens grow by
   658, 658 and 34 lines (56 + 600 + 2 generics; the 34 footprints of `footprints-C.json` / `-D.json`, none of whose ids
   was already in `footprints.json`).
2. **`gen-parts.py` rules measured, not designed.** A device's timers/op-amp facts come from its most-stocked
   spec row; a part gets its own `vcc` (and an op-amp its `inputMax`, `vosMv`) where its spec row or DigiKey's offset differs. A timer device is split on `process`, `maxHz`
   and `tpd`. `railOut` is DigiKey's `Output Type` containing Rail-to-Rail; DigiKey's descriptions say nothing of
   rail-to-rail inputs, so `railIn` is set where B's reading of the datasheet allowed the inputs beyond `V+`
   (`inputMax`). `inputMax` = `vcc` max + that margin (dropped above 100 V). `FLAG` pins are `output`,
   `ISET`, `BAL1/2`, `OFFSET*`, `SHDN*`, `COMP`, `REF` `input`.
3. **Footprints.** `fp-merge.py` (without `--only`) appended all 34 objects: every one is used by a part.
4. **The editor (§ 3.4) needed more than the plan listed.** `scripts/part-editor-schema-extra.py` gets `timer`'s
   `inputMax` (device and part); the op-amp kind's `type` is a model variant, so
   `PartKinds.variantBlurbs` gets an `opamp` entry (general, jfet, cmos); and four tests hold counts of the
   kinds: `KindGuideTests` (75 models, 18 kinds), `PartEditorWindowTests` (49 enabled kinds), `WizardStepsTests`
   (106 combinations, 31 without a model, 75 with one). `PartKinds.all`, the schemas array and `infoOrder` name `timer`
   and `opamp` (`timer` in "Power and clocks" after `crystal`, `opamp` in "Other" before `generic`).
5. **Examples.** The editor's example parts are `GENERIC-555` / `NE555P` and `GENERIC-OPAMP` / `LM358DR`
   (`LM358P` is not in the pick).
6. **Open item from A and B**: 11 `add` rows still wait for their DigiKey stock lookup (A Phase 1); if one fails it
   leaves `candidates.tsv`'s `add` set, and `gen-parts.py`, the goldens and `datasheets.json` are rerun.

## Summary

Generated files only. The check is arithmetic: goldens grow by the `add`
rows plus two, and nothing else changes.
