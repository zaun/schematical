# plan-47-K: Parts — inductors, fuses and PTCs; the datasheets, footprints, goldens and editor lists

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-47-J

K fills `Parts/inductor.json` and `Parts/fuse.json`: one part per inductor,
fuse and PTC `add` row of `planning/plan-47-data/candidates.tsv` (1800 at
planning time), with the footprints D, E and F drew, the datasheet entries,
the editor's kind lists and the goldens. The behavioral outcome: the library
loads the two new part files; `grep -c '|inductor|'
backend/tests/golden/parts.txt` is the inductor `add` rows and `grep -c
'|fuse|'` the fuse and PTC `add` rows; the goldens grow by exactly the new
lines; `ctest` passes.

References:

- plan-47-C (`candidates.tsv` `add` rows, `specs.tsv`), plan-47-D, E, F
  (`footprints-D.json` … `-F.json` and their `-explicit` files,
  `footprint-map.tsv`), plan-47-I § 3.1 (the kinds' keys).
- `docs/parts-format.md` § The file, § Devices, § Parts, § Datasheets;
  `docs/backend.md` § Adding a kind (step 6, the Part Editor), § Tests (the
  golden regeneration).
- `planning/plan-40-data/census/gen-parts.py` (the generator K adapts),
  `planning/completed/plan-40-G-transistor-parts.md`,
  `planning/completed/plan-34-F-resistor-parts.md` (value bands in the
  drawer).
- `schematical/schematical/Editor/KindInfo.swift`, `KindSchema.swift`,
  `BundledPartFile.swift`; `scripts/part-editor-schema.py`;
  `schematicalTests/PartKindKeysTests.swift`, `PartKindRoundTripTests.swift`.

## Prerequisites

plan-47-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-47-J complete | `ls planning/completed/plan-47-J-*.md` → exists | NOT MET (2026-11-16: not started) |
| The kinds exist | `grep -c 'INDUCTOR_KIND\|FUSE_KIND' backend/src/library/kinds.c` → 4 | NOT MET (2026-11-16: 0) |
| Every D, E, F line has a footprint | `python3 -I planning/plan-47-data/census/fp-check.py D E F` → `<n> lines, 0 without a footprint` | NOT MET (2026-11-16: no map) |
| Golden sizes and test count re-measured | `wc -l backend/tests/golden/{parts,traces,footprints}.txt`; `grep -c '^logic_test' backend/CMakeLists.txt` | record P0 / R0 / F0 / T0 at K's start (at 349f25b3: 32897 / 32897 / 1646 / 37; 41 tests after I and J) |
| `datasheets.json` size recorded | `python3 -I -c "import json;print(len(json.load(open('schematical/schematical/Parts/datasheets.json'))['datasheets']))"` | 7993 at 349f25b3; record at K's start |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `ctest --test-dir /tmp/logic-p47-k -j8` → `100% tests passed`.
- `backend/tests/golden/parts.txt` and `traces.txt` grow by exactly the
  inductor, fuse and PTC `add` rows (1800 at planning time: 600 + 600 +
  600); `footprints.txt` by the footprints those parts use; no existing line
  changes (`git diff backend/tests/golden | grep -c '^-[^-]'` → 0).
- `PartKindRoundTripTests.testInductor` and `testFuse` pass with the shipped
  counts; `PartKindKeysTests` passes.
- `listing_check` reports no part of the two kinds without a datasheet.

### Non-goals (explicit constraints)

- No change to the backend models or the app's symbols and views (I, J, M,
  N).
- No existing part, footprint or datasheet entry changes.
- No generic part (owner decision, plan-47-A).
- No hand edit of a generated device: the generator's inputs are edited and
  it is re-run.

## 2. Current State

The kinds exist (I); `Parts/inductor.json` and `Parts/fuse.json` do not. The
footprints are in `planning/plan-47-data/footprints-*.json`, not in
`footprints.json`. `datasheets.json` (7993 entries) has none of these
datasheets. The editor's kind lists (`Editor/KindSchema.swift:202`
`PartKinds.all`, `PartKinds.schemas`; `Editor/KindInfo.swift`
`PartKinds.info` and `infoOrder` `:162`; `Editor/BundledPartFile.swift:118`
`name(of:)`; `scripts/part-editor-schema.py:30` `KIND_FILES`;
`PartKindKeysTests.swift:32` `files`; `PartKindRoundTripTests.swift:11`
`expected`) do not name the kinds; every kind has a generated schema
(`docs/backend.md` § Adding a kind, step 6).

### Measured populations

| What | Count | Command |
|---|---|---|
| `add` rows of K | the final pick's inductor, fuse and PTC rows (600 + 600 + 600 at planning time, before B's and C's verdicts) | `awk -F'\t' '$23=="add"{print $3}' planning/plan-47-data/candidates.tsv \| sort \| uniq -c` |
| …by class (planning pick) | inductor smd 480, tht 120; fuse smd 210, tht 180, cartridge 210; PTC smd 330, tht 270 | `awk -F'\t' '$23=="add"{print $3, $4}' … \| sort \| uniq -c` (`candidate` at planning time) |
| AEC-Q200 rows (planning pick) | PTC 296, inductor 132 | `awk -F'\t' '$23=="add" && $14=="AEC-Q200"{print $3}' … \| sort \| uniq -c` |
| Distinct datasheet URLs (download set) | inductor 416, fuse 204, PTC 101 | plan-47-B § 2 |
| Existing `datasheets.json` entries | 7993 | § Prerequisites |
| The four kinds in `PartKindRoundTripTests.expected` | none | `grep -c '"inductor"\|"fuse"' schematical/schematicalTests/PartKindRoundTripTests.swift` → 0 |

### Verified properties

- **`fp-merge.py` appends footprint objects, and a footprint no part uses
  fails `footprint_check`** ("unused footprint"), so footprints are merged
  in the phase whose parts use them (`planning/plan-40-data/census/fp-merge.py`
  header).
- **A part name is unique across the library**; DigiKey manufacturer part
  numbers are the names, as for every other kind (`docs/parts-format.md`
  § Parts, `name`).
- **`"flags": ["automotive"]` and `["military"]` are part keys the loader
  accepts on any kind** (`docs/parts-format.md:211`).
- **`tests/support/read_file.c:load_parts_dir` loads every `*.json` of the
  `Parts` directory**, so the new files need no list entry in the tests; the
  app bundles the `Parts` folder.

## 3. Design

### 3.1 The generator

`planning/plan-47-data/census/gen-parts.py <type>…` (adapted from plan-40's)
reads the `add` rows of `candidates.tsv`, their DigiKey parameters
(`raw/*.jsonl.gz`), `specs.tsv`, `footprint-map.tsv` and the families'
`datasheets.tsv`, and writes the kind's part file from the rows of the types
named so far (`inductor` → `inductor.json`; `fuse`, `ptc` → `fuse.json`;
plan-47-L adds `relay` and `buzzer`). Each run rewrites its files whole and
byte-stable (devices and parts sorted). It also appends each used PDF's
entry to `Parts/datasheets.json` (`{"id", "url"}`; id = the placed file's
stem in lower case, with `-<maker slug>` on a clash with an existing id; url
= the row's maker-site `url`; only if the id is absent).

Devices (one `function` is one drawer leaf):

| Kind | One device per | `drawer` | `function` (the value band, lower bound inclusive) |
|---|---|---|---|
| `inductor` | (class, pinout, band) | `["Discrete", "Inductors", "Surface mount"]` or `[…, "Through hole"]` | `< 10 nH`, `10 nH – 100 nH`, `100 nH – 1 µH`, `1 µH – 10 µH`, `10 µH – 100 µH`, `100 µH – 1 mH`, `≥ 1 mH` |
| `fuse`, type `fuse` | (class, pinout, band) | `["Discrete", "Fuses", "Surface mount"]`, `[…, "Through hole"]` or `[…, "Cartridge"]` | rated current: `< 500 mA`, `500 mA – 1 A`, `1 A – 2 A`, `2 A – 5 A`, `5 A – 10 A`, `≥ 10 A` |
| `fuse`, type `ptc` | (class, pinout, band) | `["Discrete", "Resettable fuses", "Surface mount"]` or `[…, "Through hole"]` | hold current: `< 200 mA`, `200 mA – 500 mA`, `500 mA – 1 A`, `1 A – 2 A`, `2 A – 5 A`, `≥ 5 A` |

- Device `id`: `ind_<smd|tht>_<pinout key>_<band slug>`,
  `fuse_<smd|tht|cart>_…`, `ptc_<smd|tht>_…`; `description`: "Fixed
  inductor", "Fuse", "PTC resettable fuse"; `"terminals": ["1", "2"]`;
  `pinouts`: the pinouts its parts use, key = the pin names joined by `-`
  (`1-2`, `1-nc-2-nc`, `1-1-2-2`); a cartridge device's `note` names the
  clip part and its datasheet for each maker (plan-47-A § 6).
- One part per row: `name` the part number; `manufacturer` (`Bourns`,
  `Murata Electronics`, `Würth Elektronik`, `Littelfuse`, `Eaton`,
  `SCHURTER`, `Bel Fuse`); `datasheet`; `package` the specs `label`;
  `pinout`; `footprint` from the map; the kind's part keys from the row
  (plan-47-B § 3.4: `henries`, `tolerance`, `amps`, `isat`, `dcr`,
  `shielding`; `amps`, `speed`, `voltsAc`, `voltsDc`, `breaking`; `hold`,
  `trip`, `volts`, `imax`), an optional key omitted when its fact is blank;
  `"flags": ["automotive"]` for an AEC-Q200 row.
- Values are written as JSON numbers in base units (henries, amps, volts,
  ohms) with the digits DigiKey gives (`4.7 µH` → `4.7e-06`).

### 3.2 The editor's kind lists

Per `docs/backend.md` § Adding a kind step 6, for `inductor` and `fuse`
(each line only if absent: `grep -c '"inductor"'
schematical/schematical/Editor/KindSchema.swift` → 0 before):

- `PartKinds.all` and `infoOrder` gain `"inductor"`, `"fuse"` (after
  `"transistor"` in `infoOrder`);
- `KindInfo(kind: "inductor", name: "Inductor", blurb: "Coils and chokes:
  two terminals that conduct like a wire in the simulation", examples: <the
  two most-stocked inductor parts>, group: .discretes)` and
  `KindInfo(kind: "fuse", name: "Fuse", blurb: "Fuses and resettable PTC
  fuses: a link you can blow in play mode", examples: <the most-stocked fuse
  and PTC>, group: .discretes)`;
- `BundledPartFile.name(of:)`: `case PART_INDUCTOR: "inductor"`, `case
  PART_FUSE: "fuse"`; the kinds' names in `PartFacts` (the editor's use of
  it);
- `"inductor": ["inductor.c"], "fuse": ["fuse.c"]` in
  `scripts/part-editor-schema.py` `KIND_FILES` and in
  `PartKindKeysTests.files`;
- `python3 scripts/part-editor-schema.py . inductor` and `… fuse` →
  `missing []` → `Editor/InductorDraft.swift`, `Editor/FuseDraft.swift`
  (a key no shipped part uses goes in `scripts/part-editor-schema-extra.py`
  `EXTRA`); both in `PartKinds.schemas`;
- `testInductor`, `testFuse` and `expected["inductor"]`, `expected["fuse"]`
  (`grep -o '"manufacturer"' schematical/schematical/Parts/inductor.json |
  wc -l`, and `fuse.json`) in `PartKindRoundTripTests`.

No curated wizard guide (plan-46 curates the simple logic kinds only; the
derived steps cover these kinds).

### 3.3 Docs

`docs/parts-format.md`: the `### inductor` and `### fuse` sections (I wrote
the keys) gain the library paragraph (makers, drawer paths, bands, the
cartridge clips, a JSON example of one device with one part each); § Parts
Flags: `automotive` reads "AEC-Q100 or AEC-Q200 qualified" (only if it does
not already); § Datasheets: a paragraph naming the maker sites that refuse
`scripts/check-datasheets.js`'s client (the hosts and statuses its run in
Phase 4 reports), so a later run reads them as expected.

### 3.4 Goldens and risk

`listing_check --print`, `trace_check --print` and `footprint_check
--print` regenerate the goldens (`docs/backend.md` § Tests). Where the risk
concentrates: the generator's byte stability and value formatting (a
`henries` that round-trips to another picohenry count), and a footprint id
in the map that a part's pinout does not fit (`has N pads, pinout has M
pins` at load). The checks are the exactly-the-new-lines diff, `ctest`, and
`gen-parts.py --check` (it re-reads the written file and compares each
part's values with its row).

Rejected alternatives: (1) hand-written part JSON (rejected: 1800 parts);
(2) one device per series (rejected: the drawer leaf is the value band, the
resistor precedent); (3) fuses and PTCs in two files (rejected: one kind,
one file, as `diode.json` holds every diode type).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The generator, the inductors, the editor lists for `inductor`

- [ ] `census/gen-parts.py` (§ 3.1) with `--check`; `gen-parts.py inductor`
      writes `Parts/inductor.json` and the datasheet entries.
- [ ] `fp-merge.py schematical/schematical/Parts/footprints.json
      planning/plan-47-data/footprints-D.json
      planning/plan-47-data/footprints-D-explicit.json` (only the ids an
      inductor part names: `--only <ids>` from the map).
- [ ] The editor lists for `inductor` (§ 3.2).
- [ ] Regenerate the goldens; review `git diff --stat backend/tests/golden`.

Acceptance: the inductors load with their footprints and datasheets; goldens grow by exactly the new lines.
  Check: `cmake -S backend -B /tmp/logic-p47-k && cmake --build /tmp/logic-p47-k -j8 && ctest --test-dir /tmp/logic-p47-k -j8` → `100% tests passed`; `grep -c '|inductor|' backend/tests/golden/parts.txt` = `awk -F'\t' '$23=="add" && $3=="inductor"' planning/plan-47-data/candidates.tsv \| wc -l`; `git diff backend/tests/golden \| grep -c '^-[^-]'` → 0; `python3 -I planning/plan-47-data/census/gen-parts.py inductor --check` → `0 differences` (est. 8 min: a build from scratch and the full `ctest`, which is the only check that loads every part).
Commit: —

### Phase 2 — The fuses

- [ ] `gen-parts.py inductor fuse` → `Parts/fuse.json` with the fuse
      devices; merge the E footprints the parts name (the six clip pairs
      among them); regenerate the goldens.

Acceptance: the fuses load; goldens grow by exactly the new lines.
  Check: `cmake --build /tmp/logic-p47-k -j8 && ctest --test-dir /tmp/logic-p47-k -j8` → `100% tests passed`; `grep -c '|fuse|' backend/tests/golden/parts.txt` = the fuse `add` rows; `git diff backend/tests/golden \| grep -c '^-[^-]'` → 0 (est. 4 min).
Commit: —

### Phase 3 — The PTCs, the editor lists for `fuse`

- [ ] `gen-parts.py inductor fuse ptc` → the PTC devices in `fuse.json`;
      merge the F footprints; regenerate the goldens.
- [ ] The editor lists for `fuse` (§ 3.2; the schema needs both types'
      parts shipped, so it is generated here).

Acceptance: the PTCs load; the editor knows both kinds.
  Check: `ctest --test-dir /tmp/logic-p47-k -j8` after a build → `100% tests passed`; `grep -c '|fuse|' backend/tests/golden/parts.txt` = the fuse + PTC `add` rows; `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p47-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartEditorWindowTests` → `** TEST SUCCEEDED **` (est. 12 min: the backend tests, an app build and three test classes).
Commit: —

### Phase 4 — Docs, the datasheet URL check

- [ ] `docs/parts-format.md` per § 3.3.
- [ ] `node scripts/check-datasheets.js --host <host>` for each of the nine
      maker hosts of K's parts (`www.bourns.com`, `pim.murata.com`,
      `www.murata.com`, `www.we-online.com`, `www.littelfuse.com`,
      `www.eaton.com`, `www.schurter.com`, `www.belfuse.com`,
      `belfuse.com`); record each host's result (ok, or the status its
      client gets) in § Corrections and in the § Datasheets paragraph.

Acceptance: the docs describe the library's inductors and fuses and the URL check's expected failures.
  Check: `grep -c 'Discrete → Inductors\|Discrete → Fuses\|Discrete → Resettable fuses' docs/parts-format.md` → at least 3; `grep -c 'we-online.com' docs/parts-format.md` → at least 1 (est. 10 min: the URL checks, 4 at a time).
Commit: —

## Validation Plan

- Tests: `ctest` (`listing_check`, `trace_check`, `footprint_check`,
  `library_check` load every new part and footprint); `PartKindRoundTripTests`,
  `PartKindKeysTests`, `PartEditorWindowTests`.
- Coverage check: the golden line counts equal the `add` rows; `gen-parts.py
  --check`.
- Runtime proof: n/a in K (M shows the parts in the app; the kinds' behavior
  is I's tests).
- Doc sync: § 3.3.
- Final gate: plan-47-O.

## Decisions

- **Drawer paths**: Discrete → Inductors / Fuses / Resettable fuses → the
  mounting class → the value band (§ 3.1), beside Discrete → Resistors and
  Capacitors.
- **Bands by decade for inductors, by 1–2–5 steps for currents** (§ 3.1):
  the planning pick's inductors span 0.3 nH to 100 mH (96, 82, 50, 201, 93,
  42 and 36 parts in the seven bands), its fuses 35 mA to 125 A (49, 59,
  101, 158, 102, 129 in the six bands) and its PTC hold currents 10 mA to
  15 A (127, 90, 109, 106, 100, 66).
- **A cartridge fuse's `package` is `Cartridge`** and its footprint its
  maker's clip pair; the clip is named in the device `note` only.
- **`manufacturer` names** as listed in § 3.1; Würth with its umlaut, as
  DigiKey prints it.
- **The editor schemas are generated** (plan-46's rule); no curated guide.
- **`automotive` covers AEC-Q200** (passives' qualification) as well as
  AEC-Q100.

## Corrections

## Summary

K's risk is the generator (value formatting, byte stability) and footprint
ids that do not fit a pinout; the exactly-the-new-lines golden diff, the
loader and `--check` bound them. Untouched: the backend models, the app's
views, every existing part, footprint and datasheet entry.
