# plan-70-H: Parts: the two generics, `isolator.json`, datasheets, footprints, goldens, the Part Editor

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-70-G

H writes `schematical/schematical/Parts/isolator.json` from the final pick
and the specs, and adds the datasheet entries and the footprints the parts
use. It regenerates the goldens and registers the kind in the Part Editor's
lists and schemas.

References:

- plan-70-A to -D (`candidates.tsv` `add` rows, `specs.tsv`, the PDFs and
  `datasheets.tsv` files); plan-70-E / -F (`footprints-E.json`,
  `footprints-F.json`, `footprint-map.tsv`); plan-70-G § 3.2–3.5 (the
  format).
- `docs/parts-format.md` § Devices, § Parts, § Datasheets, § The drawer's
  categories; `docs/backend.md` § Adding a kind step 6 (the Part Editor),
  § Tests (the goldens).
- `planning/completed/plan-48-G-timer-opamp-parts.md` and
  `planning/plan-48-data/census/gen-parts.py`, `fp-merge.py`: the shape H
  follows, and the scripts it adapts.

## Prerequisites

plan-70-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-70-G complete | `ls planning/completed/plan-70-G-*.md` → exists, or every G box ticked | NOT MET (2026-11-25: G not started) |
| Every `add` row is mapped and specified | `fp-check.py E` and `fp-check.py F` → `0 without a footprint`; `specs-check.py transistor triac logic digital` → `0 errors` | NOT MET (2026-11-25: no data) |
| Golden sizes at H's start | `wc -l backend/tests/golden/parts.txt backend/tests/golden/traces.txt backend/tests/golden/footprints.txt backend/tests/golden/erc_pins.txt` | recorded here at H's start as **P2**, **R2**, **F2**, **E2** |

Everything below is written against the world where these hold.

> **NOTE: the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `isolator.json` loads with no error. `grep -c '|isolator|' backend/tests/golden/parts.txt`
  → the `add` count + 2.
- Every new footprint is in `footprints.json`, and `footprint_check`
  passes with the regenerated golden.
- Every part's `datasheet` is a `datasheets.json` entry with the maker's
  URL. `node scripts/check-datasheets.js --host <host>` exits 0 for the
  plan-70 entries of each of the ten hosts.
- The Part Editor round-trips every shipped isolator part
  (`PartKindRoundTripTests.testIsolator`: 0 mismatched, 0 with extras, 0
  rejected).

### Non-goals (explicit constraints)

- No app view change (I).
- The goldens only grow: no existing line is removed or changed, apart from
  the per-kind count lines G § 3.7 describes.
- No part outside the final pick, and no part with stock 0 at listing time.

## 2. Current State

There is no `isolator.json`. `datasheets.json` is `{"format", "version",
"datasheets"}`. The newest kinds' editor lines are `scripts/part-editor-schema.py:44`
(`"timer": ["timer.c"], "opamp": ["opamp.c"]`),
`schematicalTests/PartKindKeysTests.swift:47`, the
`PartKindRoundTripTests.swift:31` shipped-count list and its `testOpamp`
(`:104`), and `Editor/KindInfo.swift:193`–`:223`.

## 3. Design

### 3.1 Devices and parts (`census/gen-parts.py`, from plan-48's)

- One device per (type, PDF, pinout, channels, `invert` pattern, `output`,
  `darlington` / `zeroCross` / `ac`). Its parts are every `add` row of the
  spec line. Device `id`: `iso_<type>_<maker>_<pdf stem>_<pinout key>`,
  with a counter on a clash.
- `function`: the channel count (`1 channel`, `2 channels`, `4 channels`).
  A digital isolator's also carries the split: `4 channels (3/1)`.
  `description`: `Transistor optocoupler`, `Darlington optocoupler`,
  `Triac driver optocoupler` (`zero-cross triac driver optocoupler` with
  `zeroCross`), `Logic optocoupler`, `Digital isolator`.
- `drawer`: `["Interface", "Optocoupler", "Transistor" | "Darlington" |
  "Triac" | "Logic"]` or `["Interface", "Digital isolator"]`.
- Pinout key: the pin names in pin order, lower case, `-` joined (as the
  relay kind's).
- Part keys: `name`, `manufacturer` (the maker's own name: `Isocom
  Components`, `Vishay`, `onsemi`, `Broadcom`, `Toshiba`, `Renesas`, `Würth
  Elektronik`, `Analog Devices`, `Texas Instruments`, `Skyworks`),
  `datasheet`, `package` (DigiKey's supplier package), `pinout`,
  `footprint` (from `footprint-map.tsv`), the § 3.5 facts of G with C § 3.2's
  overrides applied, `"flags": ["automotive"]` for Grade Automotive rows,
  and `"flags": ["obsolete"]` for `dead` rows (B § 3.2 rule 5).
- `--check` re-reads every `add` row and reports one with no device, no
  footprint or no datasheet entry.

### 3.2 The generics

- `GENERIC-OPTO`: type `transistor`, 4 pins `A`, `K`, `E`, `C` (the PC817
  order). `viso` 5000, `ctrMin` 50, `ctrMax` 600, `vceo` 80, `ifMax` 50.
  Footprint: the 4-pin DIP footprint `footprint-map.tsv` gives the
  most-stocked 4-pin through-hole transistor line. Drawer `["Interface",
  "Optocoupler"]`, function `Generic`.
- `GENERIC-ISOLATOR`: type `digital`, 2 channels 2/0, 8 pins `VDD1`, `VIA`,
  `VIB`, `GND1`, `GND2`, `VOB`, `VOA`, `VDD2` (the ADuM1200 order).
  `domains` side 1 `VDD1` + `VIA`, `VIB`, side 2 `VDD2` + `VOA`, `VOB`;
  `vcc` and `vccB` `[2.7, 5.5]`, `viso` 2500, `rate` 25, `failsafe` `high`.
  Footprint `SOIC-8`. Drawer `["Interface", "Digital isolator"]`, function
  `Generic`.

### 3.3 Datasheets and footprints

- A `datasheets.json` entry per distinct PDF of the `add` rows: id from the
  file name (the existing rule, `docs/parts-format.md` § Datasheets), the
  maker URL (`url-check.tsv` final, or `maker-urls.tsv`), and the path
  under `datasheets/isolator/<maker>/`.
- `fp-merge.py footprints-E.json footprints-F.json` appends the objects
  whose ids are not in `footprints.json` (the shared-id grep, plan-70-A §
  Prerequisites) and lists the ids already there.

### 3.4 The editor (`docs/backend.md` § Adding a kind step 6)

- `Editor/KindInfo.swift`: `KindInfo(kind: "isolator", name:
  "Optocoupler / isolator", …)`. The blurb is "Optocouplers and digital
  isolators: an input that drives an output across an isolation barrier".
  The examples are `GENERIC-OPTO`, `GENERIC-ISOLATOR` and the most-stocked
  6N137-style part. The picker group sits with `relay`. The kind's name
  goes in `PartKinds.all` and `infoOrder` after the last kind present.
- `BundledPartFile.name(of:)`: `case PART_ISOLATOR: "isolator"`;
  `PartFacts` kind names (I writes the rows).
- `scripts/part-editor-schema.py` `KIND_FILES` and
  `PartKindKeysTests.files`: `"isolator": ["isolator.c"]`.
- `python3 scripts/part-editor-schema.py . isolator` writes
  `Editor/IsolatorDraft.swift` and prints `missing []`. Keys no shipped
  part uses go in `scripts/part-editor-schema-extra.py` `EXTRA`. `type` is
  the `variantKey`, so `PartKinds.variantBlurbs` gets one sentence per type
  (transistor, triac, logic, digital).
- `PartKinds.schemas` (`Editor/KindSchema.swift`); `PartKindRoundTripTests`:
  `"isolator"` in the shipped-count list, and `testIsolator`.

## Phases

> **NOTE: keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1: The generics and the editor's lists

- [ ] `census/gen-parts.py` per § 3.1–3.3; `--generics` writes
      `isolator.json` with the two generics and their footprint.
- [ ] The editor steps of § 3.4 (each list line only if absent, after the
      last kind present).
- [ ] Regenerate `parts.txt`, `traces.txt`, `footprints.txt`,
      `erc_pins.txt` (`docs/backend.md` § Tests); review the diff.

Acceptance: the two generics load and round-trip.
  Check: `grep -c '^GENERIC-OPTO|\|^GENERIC-ISOLATOR|' backend/tests/golden/parts.txt`
  → 2; `ctest -R 'listing_check|trace_check|footprint_check|erc_pins'`
  passes. Then `xcodebuild … test -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartKindKeysTests`
  → `** TEST SUCCEEDED **`. About 10 min with the app test build: the
  editor lists live in the app.
Commit:

### Phase 2: Optocouplers

- [ ] `gen-parts.py --type transistor`, `--type triac`, `--type logic`:
      the `add` rows, their datasheet entries and footprints.
- [ ] Regenerate the goldens; `git diff backend/tests/golden | grep -c '^-[^-]'`
      → 0 apart from G's count lines.
- [ ] `node scripts/check-datasheets.js --host isocom.com`, `--host
      www.vishay.com`, `--host www.onsemi.com`, `--host docs.broadcom.com`,
      `--host toshiba.semicon-storage.com`, `--host www.renesas.com`,
      `--host www.we-online.com`. Each exits 0 for the plan-70 entries.

Acceptance: every optocoupler `add` row is a part.
  Check: `python3 -I planning/plan-70-data/census/gen-parts.py --check` →
  `transistor <n>, triac <n>, logic <n>: 0 missing` (the counts are D's
  final ones). `ctest -R 'listing_check|trace_check|footprint_check|erc_pins'`
  passes. About 5 min.
Commit:

### Phase 3: Digital isolators and the docs

- [ ] `gen-parts.py --type digital`; goldens; `check-datasheets.js --host
      www.analog.com`, `--host www.ti.com`, `--host www.skyworksinc.com`.
- [ ] `PartKindRoundTripTests` count re-set to the shipped one.
- [ ] `docs/parts-format.md` § `isolator`: a paragraph on the library's
      `isolator.json` (makers, drawer paths, the device id rule, the
      generics), as § `relay` has.

Acceptance: every `add` row is a part, and the editor round-trips all of
them.
  Check: `gen-parts.py --check` → `0 missing` for all four types;
  `grep -c '|isolator|' backend/tests/golden/parts.txt` → the `add` count +
  2; `xcodebuild … test -only-testing:schematicalTests/PartKindRoundTripTests`
  → `** TEST SUCCEEDED **`. About 10 min.
Commit:

## Validation Plan

- Tests: `listing_check`, `trace_check`, `footprint_check`, `erc_pins_check`;
  `PartKindRoundTripTests`, `PartKindKeysTests`; `gen-parts.py --check`;
  `check-datasheets.js` per host.
- Coverage check: the per-type part counts in `parts.txt` equal D's final
  counts.
- Doc sync: `docs/parts-format.md` § `isolator`.
- Final gate: plan-70-J.

## Decisions

- Author: **one device per spec line and truth table**, so a device's
  channels mean the same thing for every part in it.
- Author: **CTR ranks, trigger-current grades and default-output suffixes
  are part facts**, not separate devices (C § 3.2, D § 3.1).
- Author: **the generics' footprints are real ones** (AGENTS.md: every
  part names a footprint): the most-stocked 4-pin DIP and `SOIC-8`.

## Corrections

## Summary

Generation work from finished tables. The checks that catch mistakes are
the generator's `--check` and the round trip through the Part Editor.
