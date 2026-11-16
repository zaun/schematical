# plan-44-F: Parts — the four generics, every `add` row, the datasheets, the goldens

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-44-E

F fills `Parts/crystal.json` and the libraries around it: the four generic
parts with their footprint choices, one `crystal` part per `add` row of
`planning/plan-44-data/candidates.tsv` (600 at planning time), the
footprints C and D drew, the datasheet entries, the editor's kind lists and
the goldens. The behavioral outcome: the library loads the new part file;
`grep -c '|crystal|' backend/tests/golden/parts.txt` is the `add` rows + 4;
the goldens grow by exactly the new lines; every part's footprint, datasheet,
pinout and facts come from B's data; `ctest` passes.

References:

- plan-44-B (`candidates.tsv` `add` rows, `specs.tsv`), plan-44-C and D
  (`footprints-C.json`, `footprints-D.json`, `footprint-map.tsv`), plan-44-E
  (the kind's keys, § 3.1).
- `docs/parts-format.md` § Parts (keys, flags), § Datasheets, § Footprints
  (`footprintChoices`, the `"footprint"` part key); `docs/backend.md` § Tests
  (regenerating the goldens).
- `planning/completed/plan-40-G-transistor-parts.md` and
  `planning/plan-40-data/census/gen-parts.py` (the generator precedent: one
  device per group, one part per row, `fp-merge.py --only` for the footprints
  a phase uses).

## Prerequisites

plan-44-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-44-E complete | `ls planning/completed/plan-44-E-*.md` → exists | NOT MET (2026-11-16: not started) |
| plan-44-B, C, D complete | `ls planning/completed/plan-44-B-*.md planning/completed/plan-44-C-*.md planning/completed/plan-44-D-*.md` → three files | NOT MET (2026-11-16: not started) |
| The kind loads | `grep -c 'CRYSTAL_KIND' backend/src/library/kinds.c` → 2 | NOT MET (2026-11-16: 0) |
| Every `add` row has a footprint | `python3 -I planning/plan-44-data/census/fp-check.py C` and `… D` → `0 without a footprint` | NOT MET (2026-11-16: C and D not started) |
| Golden sizes recorded | `git show HEAD:backend/tests/golden/parts.txt \| wc -l` (and `traces.txt`, `footprints.txt`) | record P1 / R1 / F1 at F's start (at 7108a13e: 32897 / 32897 / 1646) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `ctest --test-dir /tmp/logic-p44-f` → `100% tests passed` (T0 + 1 programs,
  as E).
- `grep -c '|crystal|' backend/tests/golden/parts.txt` → `<add rows> + 4`
  (`awk -F'\t' '$25=="add"' planning/plan-44-data/candidates.tsv | wc -l`
  plus the four generics; 604 at planning time if every row passes B).
- `backend/tests/golden/parts.txt` and `traces.txt` each grow by exactly that
  many lines and `footprints.txt` by exactly the new footprints; no existing
  line changes (`git diff <F's first commit>^ -- backend/tests/golden | grep
  -c '^-[^-]'` → 0).
- Every crystal part's `datasheet` id is an entry of `datasheets.json` whose
  URL is on the maker's site (`url` of `candidates.tsv`); the PDFs are the
  ones under `datasheets/crystal/` (B's prune).

### Non-goals (explicit constraints)

- No change to the backend model or the app's symbols and views (E and G).
- No existing part, footprint or datasheet entry changes.
- No part-editor form for the kind (G's § Decisions); the editor's kind lists
  only name the kind so "Copy to Editor…" and the New Part list know it.

## 2. Current State

The kind exists (E); `Parts/crystal.json` does not. The footprints are in
`footprints-C.json` and `footprints-D.json`, not in `footprints.json`.
`datasheets.json` has entries for no crystal datasheet. The editor's kind
lists (`schematical/schematical/Editor/KindSchema.swift` `PartKinds.all` and
`displayNames`; `Editor/BundledPartFile.swift` `PartKinds.name(of:)`;
`schematicalTests/PartEditorWindowTests.swift` `formless`) do not name
`crystal`.

### Measured populations

| What | Count | Command |
|---|---|---|
| `add` rows | the final pick (600 at planning time before B) | `awk -F'\t' '$25=="add"' planning/plan-44-data/candidates.tsv \| wc -l` |
| …by class | kHz, MHz, resonator (120 / 414 / 66 at planning time) | `awk -F'\t' '$25=="add"{print $3}' … \| sort \| uniq -c` |
| Distinct datasheet URLs of the `add` rows | 188 at planning time (candidate rows only) | `awk -F'\t' '$25=="add"{print $22}' … \| sort -u \| wc -l` |
| AEC-Q200 rows | 131 at planning time | `awk -F'\t' '$25=="add" && $18=="AEC-Q200"' … \| wc -l` |
| Existing `datasheets.json` entries | the library's current count (record at F's start) | `python3 -I -c "import json;print(len(json.load(open('schematical/schematical/Parts/datasheets.json'))['datasheets']))"` |

### Verified properties

- **`fp-merge.py --only <ids>` appends only the named footprint objects**
  (its header: "append only the named objects (plan-40-G Phase 1: the
  generics' choices)"); a footprint no part uses fails `footprint_check`
  ("unused footprint"), so footprints are merged in the phase whose parts use
  them.
- **A part name is unique across the library** (`docs/parts-format.md`
  § Parts, `name`); the loader rejects a duplicate with the file and device.
  DigiKey manufacturer part numbers are the names, as for every other kind.
- **`flags: ["automotive"]` is a part key the loader accepts on any kind**
  (`docs/parts-format.md` § Parts, Flags).

## 3. Design

### 3.1 The generator

`planning/plan-44-data/census/gen-parts.py` (adapted from plan-40's) reads the
`add` rows of `candidates.tsv`, `specs.tsv` (`pinout`, `terminals`, `label`
(the package family), `capacitors`, `page`), `footprint-map.tsv` and
`datasheets/crystal/datasheets.tsv`, and writes `Parts/crystal.json`
(`--class khz|mhz|resonator` limits it; each run rewrites the file from the
rows of the classes done so far, byte-stable). Devices:

- one device per (class, frequency band), `"kind": "crystal"`, `type`
  `crystal` (kHz, MHz) or `resonator`, `terminals` `["X1","X2"]`, `drawer`
  `["Crystal","kHz"]`, `["Crystal","MHz"]` or `["Crystal","Ceramic
  resonator"]`, `function` the band, `pinouts` the pinouts its parts use (key
  = the pin names joined by `-`, lower case: `x1-x2`, `x1-nc-x2-nc`,
  `x1-gnd-x2`);
- bands (lower bound inclusive): kHz class: the frequency itself as text
  (`32.768 kHz`, `32 kHz`); MHz class: `Under 4 MHz`, `4 MHz to 8 MHz`, `8 MHz
  to 16 MHz`, `16 MHz to 32 MHz`, `32 MHz and over`; resonator: `Under 4 MHz`,
  `4 MHz to 8 MHz`, `8 MHz to 16 MHz`, `16 MHz and over`;
- one part per row: `name` the DigiKey part number, `manufacturer`
  (`Abracon`, `ECS Inc.`, `Suntsu Electronics`), `datasheet` the id of the
  datasheet entry, `package` the specs `label`, `pinout`, `footprint` from
  the map, `hz`, `loadPf`, `tolerancePpm`, `stabilityPpm`, `esrOhms` (omitted
  when blank), `capacitors` (resonators only), `"flags": ["automotive"]` for an
  AEC-Q200 row;
- `note` per device: the datasheets and pages the facts came from (`specs.tsv`
  `page`) and the `NC` / `GND` remark of B § 3.3.

Datasheet ids are the lower-cased file stem of the placed PDF; a clash with an
existing id gets `-<maker slug>`. `datasheets.json` gets one entry per
distinct id: `{"id": …, "url": <the maker-site URL of candidates.tsv>}`,
appended in id order after the existing entries (the file is a list; the
loader does not need sorting).

### 3.2 The generics

Four generic parts (`manufacturer` "Generic", no datasheet, `drawer`
`["Crystal"]`, function `Generic`, `hz` fixed in the part; the user cannot
change it, as for `GENERIC-RESISTOR`'s ohms):

| Part | Type | Frequency | Pinout | Default footprint | Other choices (picker order after the default) |
|---|---|---|---|---|---|
| `GENERIC-CRYSTAL-32768` | crystal | 32768 Hz | `x1-x2` | label `XTAL-3215-2` (package `SMD`) | `XTAL-2012-2`, `XTAL-1610-2` (`SMD`), `XTAL-CYL-2X6-2` (`Cylinder`) |
| `GENERIC-CRYSTAL-8M` | crystal | 8 MHz | `x1-x2` | label `XTAL-HC49US-2` (`HC-49/US`, through hole) | `XTAL-3225-2`, `XTAL-5032-2`, `XTAL-6035-2` (`SMD`) |
| `GENERIC-CRYSTAL-16M` | crystal | 16 MHz | `x1-x2` | as `GENERIC-CRYSTAL-8M` | as `GENERIC-CRYSTAL-8M` |
| `GENERIC-RESONATOR-8M` | resonator | 8 MHz | `x1-gnd-x2` | label `CRES-3731-3` (`SMD`) | `CRES-6030-3`, `CRES-3213-3`, `CRES-2520-3` (`SMD`) |

A choice names the footprint id of the label: the lowest id (ASCII order) of
the `footprint-map.tsv` rows with that label. Every choice has exactly the
pad count of its generic's pinout (2, 2, 2, 3): the loader requires a part's
own footprint to have one pad per pin and each choice a pad for every pin
(`library.c` `load_part_footprint`, `load_footprint_choices`), so the
two-pad crystals choose two-pad labels only. The part's own `footprint` is its
default choice and its `package` that choice's.

### 3.3 Goldens and risk

`listing_check --print`, `trace_check --print` and `footprint_check
--print` regenerate the goldens (`docs/backend.md` § Tests). Where the risk
concentrates: the traces (E's clock model hashed into each of the parts'
lines) and the generator's byte stability; the checks below are the
exactly-the-new-lines diff and `ctest`.

Rejected alternatives: (1) hand-written part JSON (rejected: 600 parts); (2)
one device per part (rejected: the drawer leaf and the file size); (3) a
frequency slider on the generics (rejected: the file format has no per-part
value input and `GENERIC-RESISTOR` is the precedent).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The generics, the footprints they choose, the editor's kind lists

- [ ] `census/gen-parts.py --generics` writes `Parts/crystal.json` with the
      four generics; `planning/plan-44-data/census/fp-load.sh`'s helper
      `fp-merge.py --only <the 12 ids of § 3.2> schematical/schematical/Parts/footprints.json
      planning/plan-44-data/footprints-C.json planning/plan-44-data/footprints-D.json`
      appends exactly those footprints.
- [ ] Editor lists: `Editor/KindSchema.swift` `PartKinds.all` gains
      `"crystal"` and `displayNames` `"crystal": "Crystal"` (each only if
      absent: `grep -c '"crystal"' schematical/schematical/Editor/KindSchema.swift`
      → 0 before); `Editor/BundledPartFile.swift` `PartKinds.name(of:)` gains
      `case PART_CRYSTAL: "crystal"`; `schematicalTests/PartEditorWindowTests.swift`
      `formless` gains `"crystal"`.
- [ ] Regenerate the goldens (`listing_check --print > backend/tests/golden/parts.txt`,
      `trace_check --print > …/traces.txt`, `footprint_check --print >
      …/footprints.txt`) and review `git diff --stat`.

Acceptance: the four generics load with their choices; goldens grow by 4, 4
and 12 lines.
  Check: `cmake -S backend -B /tmp/logic-p44-f && cmake --build /tmp/logic-p44-f -j8 && ctest --test-dir /tmp/logic-p44-f -j8` → `100% tests passed`, then `git diff --stat backend/tests/golden` → `+4 +4 +12` lines and `git diff backend/tests/golden | grep -c '^-[^-]'` → 0 (est. 6 min).
Commit: —

### Phase 2 — kHz crystals and ceramic resonators

- [ ] `census/gen-parts.py --class khz --class resonator` (the `add` rows of
      those classes; 186 at planning time) and `datasheets.json` entries for
      their datasheets; `fp-merge.py --only` the footprints they use (the map
      ids of those rows); regenerate the goldens.

Acceptance: the parts load and the goldens grow by exactly them.
  Check: `ctest --test-dir /tmp/logic-p44-f -j8` after a rebuild → `100% tests passed`; `grep -c '|crystal|' backend/tests/golden/parts.txt` → 4 + the kHz and resonator `add` rows; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0 (est. 6 min).
Commit: —

### Phase 3 — MHz crystals and the docs

- [ ] `census/gen-parts.py` for the MHz class (414 at planning time); the
      remaining footprints; `datasheets.json` entries; regenerate the goldens.
- [ ] `node scripts/check-datasheets.js --host abracon.com` and `--host
      ecsxtal.com` → no failure for the new entries; `--host suntsu.com` →
      HTTP 403 for every Suntsu entry (the maker's site refuses automated
      fetches; A § 2): record the counts in § Corrections.
- [ ] Docs: `docs/parts-format.md` § Datasheets: one paragraph naming the
      Suntsu 403 behavior of `check-datasheets.js`; § Parts, Flags:
      `automotive` also covers AEC-Q200 crystals; the `### crystal` section's
      library paragraph (counts, drawer paths, the four generics, the
      footprint choices).
- [ ] `datasheets/skipped-parts.md` and `obsolete-not-included.md` are B's;
      here only run `place.py --verify` → `0 missing, 0 unused`.

Acceptance: the whole pick loads; goldens grow by exactly it; docs name it.
  Check: `ctest --test-dir /tmp/logic-p44-f -j8` after a rebuild → `100% tests passed`; `grep -c '|crystal|' backend/tests/golden/parts.txt` → the `add` rows + 4; `git diff <F's first commit>^ -- backend/tests/golden | grep -c '^-[^-]'` → 0; `xcodebuild … test -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/DatasheetCatalogTests` → `** TEST SUCCEEDED **` (est. 12 min: backend build 4, app tests 8).
Commit: —

## Validation Plan

- Tests: `library_check` / `listing_check` / `trace_check` / `footprint_check`
  (the goldens), `PartEditorWindowTests` and `DatasheetCatalogTests` (the
  editor lists and the datasheet ids); `crystal_test` re-runs on the real
  `crystal.json` (it loads the whole `Parts` directory).
- Coverage check: `listing_check` lists every part, so the new parts are in the
  goldens' denominator by construction (`grep -c '|crystal|'`).
- Runtime proof: plan-44-G's harness (the app on the real catalog).
- Doc sync: `docs/parts-format.md` (Phase 3).
- Final gate: plan-44-H.

## Decisions

- **Generic frequencies 32.768 kHz, 8 MHz, 16 MHz and an 8 MHz resonator**: the
  three clocks of the boards the library serves (RTC, ATmega-class MCUs,
  USB / CPU boards); a user wanting another frequency picks a catalog part.
- **Generic choices by label** (§ 3.2): the labels are pinned in B so each
  has a footprint; the 8 and 16 MHz generics default to the through-hole
  HC-49/US can (the part a hobby board uses) and list three two-pad SMD sizes.
- **`package` is the package family** (`SMD`, `HC-49/US`, `HC-49/U`,
  `HC-46/X`, `SOJ`, `Cylinder`), as `docs/parts-format.md` requires; the
  size and pad count come from the footprint.
- **AEC-Q200 parts carry `"flags": ["automotive"]`** (the owner put the
  grade in scope; the flag lets the drawer's automotive filter find them).
- **The datasheet `url` is the maker's** even for Suntsu, whose PDFs came
  from the archive (A § Decisions).

## Corrections

## Summary

F's risk is volume and byte stability: 600 parts, 188 datasheet entries and
the new footprints in three commits, each checked by "goldens grow by exactly
the new lines". Untouched: the engine, the model, the app's views and every
existing part.
