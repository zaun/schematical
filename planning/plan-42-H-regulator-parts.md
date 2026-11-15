# plan-42-H: Parts — the generator, the three generics, every `add` row, goldens

Last updated: 2026-11-15
Effort: large (3h–1d)
Depends on: plan-42-G

H writes `schematical/schematical/Parts/regulator.json` from the final pick:
the three generic parts, then one device per (series, package, fixed /
adjustable pinout) with one part per `add` row, each naming a footprint from
D–F's ready objects (merged into `Parts/footprints.json` here) and its
datasheet (added to `Parts/datasheets.json`). It also adds the kind to the
app's editor lists, because the app test that compares the kinds with the
part files fails once `regulator.json` exists. The behavioral outcome: the
library loads P0 + 3 + N parts (N = B's `add` count), `listing_check` /
`trace_check` / `footprint_check` pass with goldens that only **add** lines,
and `PartEditorWindowTests` passes.

References:

- plan-42-G § 3.1 (device and part keys), plan-42-C § 3 (the spec-row
  columns), plan-42-D § 3.1 (`footprint-map.tsv`).
- `planning/completed/plan-40-G-transistor-parts.md` and
  `planning/plan-40-data/census/gen-parts.py`, `fp-merge.py` (the generator
  and merge H adapts).
- `docs/parts-format.md` (§ Parts, § Datasheets, § The `"footprint"` part
  key, `"footprintChoices"`); `docs/backend.md` § Tests (regenerating the
  goldens).
- `schematical/schematical/Editor/KindSchema.swift` (`PartKinds.all`,
  `displayNames`), `Editor/BundledPartFile.swift` (`PartKinds.name(of:)`
  :90–101), `schematical/schematicalTests/PartEditorWindowTests.swift:15`
  (the `formless` set).

## Prerequisites

plan-42-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-42-G complete | `ls planning/completed/plan-42-G-*.md` → exists | NOT MET (2026-11-15: not started) |
| The kind exists | `grep -c 'PART_REGULATOR' backend/include/parts.h` → at least 1 | NOT MET (2026-11-15: 0) |
| D–F's objects load | `for L in D E F; do python3 -I planning/plan-42-data/census/fp-check.py $L; done` → `0 without a footprint` three times | NOT MET (2026-11-15: no map yet) |
| The editor's kind list does not name the kind yet | `grep -c '"regulator"' schematical/schematical/Editor/KindSchema.swift` → 0 | MET (2026-11-15 at 40ccd5fc: 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `ctest --test-dir /tmp/logic-p42 -R 'listing_check|trace_check|library_check|footprint_check|regulator_test'`
  → 5 of 5, after the goldens are regenerated.
- `git diff --stat backend/tests/golden` shows only added lines:
  `parts.txt` and `traces.txt` + (3 + N) each, `footprints.txt` + the new
  footprint objects; removed lines 0.
- `grep -c '|regulator|' backend/tests/golden/parts.txt` → 3 + N.
- `xcodebuild … test -only-testing:schematicalTests/PartEditorWindowTests`
  → `** TEST SUCCEEDED **`.

### Non-goals (explicit constraints)

- No backend code change (G's); no app code beyond the three kind-list
  lines (I's).
- No existing part, footprint or datasheet entry changes (the goldens add
  lines only).
- No part outside the `add` rows and the three generics.

## 2. Current State

- `Parts/` has one file per kind and no `regulator.json`; the library loads
  32854 parts (P0 at 40ccd5fc).
- `PartKinds.all` lists 31 kinds (`KindSchema.swift:202–206`);
  `testKindNamesAreThePartFiles` compares them with `Parts/*.json`, so a new
  part file without a kind-list line fails it (plan-40-G Phase 1, the same
  step for `transistor`).
- DigiKey's "Voltage - Output (Min/Fixed)" of a multi-output part lists one
  value per output in order (`1.8V, 3.3V`, `2.8V, 2.8V`, `3.3V, 5V`: read in
  the planning raw listing, `raw/699-*.jsonl.gz`), so a fixed multi-output
  part's `vout` array comes from it.

## 3. Design

### 3.1 The generator (`census/gen-parts.py`)

Reads `candidates.tsv` (`add` rows), `specs.tsv` (B's `label`, C's
pinouts and keys), `footprint-map.tsv`, `datasheets/regulator/datasheets.tsv`,
and writes `Parts/regulator.json` (format `docs/parts-format.md`):

- **Devices.** One per (maker, `label`, spec row pinout, fixed /
  adjustable): id `reg-<maker slug>-<label lowercase>-<package code>[-adj]`;
  `function` = the drawer leaf (§ 3.3); `description` e.g. "LDO regulator,
  fixed output, 1 A" / "Buck regulator, adjustable, 3 A"; `drawer` (§ 3.3);
  `pinouts` (C's `pinout` / `pinout_adj`); `regulators` and `analog` (C's).
- **Parts.** One per `add` row: `name` = DigiKey's part number;
  `manufacturer` "Texas Instruments" / "Analog Devices" / "Microchip
  Technology"; `package` = the supplier device package; `pinout`;
  `footprint` (the map's id); `datasheet` (the id `datasheets.json` gets for
  its PDF); keys: `vout` (fixed: DigiKey's value, or its comma list for a
  multi-output part — a list whose length is not the element count stops
  the generator with the part's name), `vref` (adjustable: C's), `voutMax`
  (adjustable: DigiKey's "Voltage - Output (Max)"), `vinMin` (C's, else
  DigiKey's "Voltage - Input (Min)"), `vinMax`, `dropout` (linear: DigiKey's,
  else C's), `iout`, `iq`, `fsw` (switching) from DigiKey's columns.
- **Datasheets.** One `datasheets.json` entry per PDF an `add` row cites
  (id = the file stem; `url` = the URL `datasheets.tsv` records; `md` none).
- **Footprints.** `census/fp-merge.py --all footprints-D.json footprints-E.json
  footprints-F.json` appends each ready object the map uses, only if absent
  (a `"id"` already in `footprints.json` is skipped: shared ids).

### 3.2 The generics

| Part | Pinout (pin: name, role) | Keys | Footprint, choices |
|---|---|---|---|
| `GENERIC-REG-5V` | 1 IN power, 2 GND ground, 3 OUT output | `vout` 5, `dropout` 2 | `TO-220-3`; choices `TO-220` → `TO-220-3`, `DPAK` → `DPAK-3` |
| `GENERIC-REG-3V3` | the same | `vout` 3.3, `dropout` 2 | the same |
| `GENERIC-REG-ADJ` | 1 ADJ input (analog), 2 OUT output, 3 IN power | `vref` 1.25, `dropout` 2; element `adjust` ADJ, `adjustTo` output | the same |

Manufacturer `Generic`, no datasheet, no `vinMax` (no VIN refusal: a
generic has no rating), drawer leaf `Generic`. The 5V / 3V3 pin order is
the 78xx order (IN, GND, OUT) and the ADJ one the LM317 order (ADJ, OUT,
IN), both the order of the TO-220 and DPAK parts they stand for; the
footprints exist (`TO-220-3`, `DPAK-3`, `footprints.json`).

### 3.3 Drawer

`["Power", "Regulators", "Linear"]` with `function` `Fixed`, `Adjustable`,
`Multi-output` (two to four regulators) or `Generic`;
`["Power", "Regulators", "Switching"]` with `function` `Buck`, `Boost`,
`Buck-boost`, `Charge pump`, `SEPIC` (from the first element's `type`), or
`Multi-output`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The generator, the generics, the kind lists

- [ ] `planning/plan-42-data/census/gen-parts.py` (§ 3; plan-40's copy):
      `--generics-only` writes `Parts/regulator.json` with § 3.2's three
      devices.
- [ ] Load it: `listing_check` lists the three generics; regenerate
      `parts.txt` and `traces.txt` (`docs/backend.md` § Tests): `git diff
      --stat backend/tests/golden` → `parts.txt | 3 +`, `traces.txt | 3 +`,
      removed lines 0.
- [ ] The app's kind lists: `"regulator"` in `PartKinds.all` and
      `displayNames` ("Voltage Regulator", `Editor/KindSchema.swift`),
      `PART_REGULATOR` → `"regulator"` in `PartKinds.name(of:)`
      (`Editor/BundledPartFile.swift`), `regulator` in `PartEditorWindowTests`'
      `formless` set. Each line only if absent: `grep -c '"regulator"'
      schematical/schematical/Editor/KindSchema.swift` → 0 first.

Acceptance: the generics load and list, and the editor agrees with the part files.
  Check: `cmake --build /tmp/logic-p42 -j8 && ctest --test-dir /tmp/logic-p42 -R 'listing_check|trace_check|library_check|footprint_check'` → 4 passed; `grep -c '^GENERIC-REG-[A-Z0-9]*|regulator|' backend/tests/golden/parts.txt` → 3; `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p42-dd test -only-testing:schematicalTests/PartEditorWindowTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 12 min: the app test build; the kind list is the only app code here).
Commit: —

### Phase 2 — Every `add` row, footprints, datasheets, goldens

- [ ] `gen-parts.py` (all rows) and `fp-merge.py --all …`; add the
      datasheet entries; record the device and part counts here
      (`gen-parts.py` prints `<d> devices, <p> parts`; `p` = N + 3).
- [ ] Regenerate `parts.txt`, `traces.txt`, `footprints.txt`; review the
      diff: added lines only.
- [ ] Spot-check five parts' listing lines against their PDFs (one per
      family and one dual): name, pinout, `vout` / `vref`, `vinMax`,
      footprint; record in § Corrections.

Acceptance: every `add` row is a loaded part and nothing else moved.
  Check: `ctest --test-dir /tmp/logic-p42 -R 'listing_check|trace_check|library_check|footprint_check|regulator_test'` → 5 of 5; `grep -c '|regulator|' backend/tests/golden/parts.txt` → N + 3; `git diff --numstat backend/tests/golden \| awk '{d+=$2} END{print d}'` → 0 (est. 4 min).
Commit: —

## Validation Plan

- Tests: `library_check` (the kind's rules on every real part),
  `listing_check` / `trace_check` (goldens), `footprint_check` (every new
  footprint used, pads valid), `regulator_test` (unchanged, on top of the real
  files), `PartEditorWindowTests`.
- Coverage check: `grep -c '|regulator|'` equals N + 3, so every `add` row is
  in the goldens' denominator.
- Runtime proof: plan-42-I.
- Doc sync: `docs/parts-format.md` § `regulator` gets the generics' table and
  the drawer paths (G wrote the section; H fills those two tables).
- Final gate: plan-42-J.

## Decisions

- Owner: generic parts are in (plan-42-A § Decisions).
- Author: **three generics** — 5 V and 3.3 V fixed (the two supply levels the
  library's logic parts use most) and one LM317-style adjustable; TO-220 and
  DPAK choices only, because the 1117-style SOT-223 has another pin order
  (ADJ/GND, OUT, IN; `lm1117.pdf` Table 6-1) and a choice must keep the pin
  numbers.
- Author: **generics carry no VIN rating** (as plan-40's generic transistors
  carry no ratings).
- Author: **one device per (series, package, fixed / adjustable)**, so each
  device has one pinout per C § 3 rule 3.

## Corrections

## Summary

The risk is wrong facts in a part line (a swapped vout list, a missing
vref); the generator stops on a list that does not fit, `library_check`
refuses the kind's rule breaks, and the spot check covers each family.
Goldens only grow.
