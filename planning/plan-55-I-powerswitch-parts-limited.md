# plan-55-I: Parts — current-limited and USB switches, datasheets, goldens

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-55-H

I adds the `powerswitch` parts of the family `limited`: one part per `add` row (at most 706,
plan-55-A § 2), their devices, the footprints they name, the datasheet entries and the regenerated
goldens, with H's generator and kind lists. There is no generic part. The behavioral outcome: the
library loads with H's parts plus these, every new part has a manufacturer, a datasheet and a
footprint, `ctest` passes, and the goldens only gain lines.

References:

- `planning/completed/plan-55-H-powerswitch-parts-ideal-load.md` (the generator, the phases I repeats for another
  family); `planning/completed/plan-55-A-powerswitch-kind.md` § 3.2; `docs/parts-format.md` § `powerswitch`;
  `docs/backend.md` § Tests.

## Prerequisites

plan-55-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-55-H complete | `ls planning/completed/plan-55-H-*.md` → exists | MET (2026-11-23: archived at 3f375434) |
| The goldens' sizes recorded | re-measured at the start of I; I uses its own | at planning (7c7b4d31): 34375 / 34375 / 1895; at I's start (3f375434, after H): **34665 / 34665 / 1935** |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `Parts/powerswitch.json` gains one device per (series, package, pin assignment) and one part per
  `add` row of `limited`; `grep -o '"manufacturer"' schematical/schematical/Parts/powerswitch.json | wc -l` →
  H's count plus the `limited` `add` rows.
- `Parts/footprints.json` gains the footprints those parts name and no other; `datasheets.json` one
  entry per cited PDF.
- The goldens only gain lines; `PartKindRoundTripTests`'s `expected["powerswitch"]` is the new count.

### Non-goals (explicit constraints)

- No change to `backend/src`, to the generator's rules or to the editor beyond the counts the tests
  moved; no generic part.

## 2. Current State

H's parts are in; the editor knows the kind. The `limited` verdicts are D's and E's.

### Measured populations

| What | Count | Command |
|---|---|---|
| `add` rows of `limited` | M (at most 706) | `python3 -I planning/plan-55-data/census/verdicts.py --report` → the `limited` `add` lines (recorded here when I starts: **588**) |

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the
> moment a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Every `limited` `add` row, footprints, datasheets, goldens

- [x] `gen-parts.py --family limited` (appends to `Parts/powerswitch.json`); `fp-merge.py --only` merges the
      footprints these parts name; one `datasheets.json` entry per cited PDF; record the device and part
      counts here. Observed: `gen-parts.py --family limited` → `343 devices, 817 parts (240 devices, 588 parts this run); 132 datasheets used, 132 added to datasheets.json`; 39 footprints appended (31 in H; 70 in all).
- [x] Regenerate `parts.txt`, `traces.txt`, `footprints.txt` (as H Phase 2); review the diff: added lines only. Observed: `git diff --numstat backend/tests/golden` → `39 0 footprints.txt`, `588 0 parts.txt`, `588 0 traces.txt`.
- [x] `expected["powerswitch"]` in `PartKindRoundTripTests` and the footprint counts in
      `ParametricFootprintDraftTests` as the failing run prints them; re-run `python3
      scripts/part-editor-schema.py . powerswitch` → `missing []`. Observed: `expected["powerswitch"]` 229 → 817; `ParametricFootprintDraftTests` 1935 → 1974 entries and 1290 → 1324 parametric; `part-editor-schema.py . powerswitch` → `343 devices, 817 parts, variant type, 16 source keys, missing []`.
- [x] Spot-check five parts' listing lines against their PDFs (a single-channel, a dual, a quad, an
      active-low enable, a part with an adjustable limit): name, pinout, enable, `ilimit`, `vinMax`,
      footprint; record in § Corrections (Correction 1).

Acceptance: every `add` row is a loaded part and nothing else moved.
  Check: `ctest --test-dir /tmp/logic-p55 -R 'listing_check|trace_check|library_check|footprint_check|powerswitch_test'` → 5 of 5; `grep -c '|powerswitch|' backend/tests/golden/parts.txt` → H's N + M; `git diff --numstat backend/tests/golden | awk '{d+=$2} END{print d}'` → 0 (est. 4 min); `xcodebuild … test -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/ParametricFootprintDraftTests -only-testing:schematicalTests/PartEditorWindowTests` → `** TEST SUCCEEDED **` (est. 15 min: the counts the tests read move with the parts). Observed: `ctest -R 'listing_check|trace_check|library_check|footprint_check|powerswitch_test'` → `100% tests passed, 0 tests failed out of 5`; `grep -c '|powerswitch|' backend/tests/golden/parts.txt` → 817 = 229 + 588; the removed-lines sum is 0; the four app classes pass except `PartKindRoundTripTests.testDecoder` and `testLatch`, which fail on `main` without this plan (plan-55-H Correction 3).
Commit: (filled by the next commit)

## Validation Plan

- Tests: `library_check`, `listing_check` / `trace_check`, `footprint_check`, `powerswitch_test`, the four app classes.
- Coverage check: the `|powerswitch|` count equals N + M, so every `add` row is in the goldens' denominator.
- Runtime proof: plan-55-J.
- Doc sync: `docs/parts-format.md` § `powerswitch` (the per-maker counts).
- Final gate: plan-55-K.

## Decisions

- Owner, 2026-11-22 (plan-55-A § Decisions): no generic part.
- Author: **one phase**: the generator and the lists exist (H), so I is H's Phase 2 for the other family.

## Corrections

1. **Spot checks (2026-11-23), all agree with the PDFs**: TPS2051BDBVR (TI single-channel USB switch, `tps2042b.pdf` Table 5-1: OUT 1, GND 2, OC 3, EN 4 active high, IN 5; `ilimit` 1 A, `vinMax` 5.5; `SOT23-5`); TPS2041BDBVR (the same table, EN active low); TPS2042B-family 8-SOIC dual (Table 5-2: GND 1, IN 2, EN1 3, EN2 4, OC2 5, OUT2 6, OUT1 7, OC1 8); TPS2054BD (quad, 16-SOIC: GND 1 and 5, IN1 2, EN1 3, EN2 4, IN2 6, EN3 7, EN4 8, OC4 9 … OC1 16; Tables 5-3/5-4); TPS2554DRCR (adjustable limit: `tps2554.pdf` Pin Functions: GND 1, IN 2 and 3, ILIM_SEL 4, EN 5 active high, ILIM1 6, ILIM0 7, OUT 8 and 9, FAULT 10; `ilimit` empty, `vinMax` 5.5; `VSON-10-DRC-J`).
2. **The library holds 817 parts in 343 devices** (Load Switches 194, Current-Limited 590, Ideal Diodes 33; Texas Instruments 430, Diodes Incorporated 154, Microchip Technology 173, Analog Devices 28, onsemi 32): written into `docs/parts-format.md` § `powerswitch` (the device ids there are `psw-<maker>-<datasheet name>`, one device per datasheet and switch layout: the plan-55-A § 3.2 form `psw-<maker>-<series>-<package>` cannot be built, DigiKey's series is `-` for nearly every part and one datasheet serves several packages through its pinouts).
3. **The per-maker counts of plan-55-H Correction 4** are in the docs now (one paragraph, both families).

## Summary

The largest golden churn of the plan (up to 706 parts): added lines only, reviewed as H's are. The
risk is a wrong enable polarity or current limit on a part, which the five spot checks sample and the
checker of C to E already covered per spec row.
