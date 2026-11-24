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

- `planning/plan-55-H-powerswitch-parts-ideal-load.md` (the generator, the phases I repeats for another
  family); `planning/completed/plan-55-A-powerswitch-kind.md` § 3.2; `docs/parts-format.md` § `powerswitch`;
  `docs/backend.md` § Tests.

## Prerequisites

plan-55-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-55-H complete | `ls planning/completed/plan-55-H-*.md` → exists | NOT MET (2026-11-22: not started) |
| The goldens' sizes recorded | re-measured at the start of I; I uses its own | at planning (7c7b4d31): 34375 / 34375 / 1895 |

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
| `add` rows of `limited` | M (at most 706) | `python3 -I planning/plan-55-data/census/verdicts.py --report` → the `limited` `add` lines (recorded here when I starts) |

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the
> moment a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Every `limited` `add` row, footprints, datasheets, goldens

- [ ] `gen-parts.py --family limited` (appends to `Parts/powerswitch.json`); `fp-merge.py --only` merges the
      footprints these parts name; one `datasheets.json` entry per cited PDF; record the device and part
      counts here.
- [ ] Regenerate `parts.txt`, `traces.txt`, `footprints.txt` (as H Phase 2); review the diff: added lines only.
- [ ] `expected["powerswitch"]` in `PartKindRoundTripTests` and the footprint counts in
      `ParametricFootprintDraftTests` as the failing run prints them; re-run `python3
      scripts/part-editor-schema.py . powerswitch` → `missing []`.
- [ ] Spot-check five parts' listing lines against their PDFs (a single-channel, a dual, a quad, an
      active-low enable, a part with an adjustable limit): name, pinout, enable, `ilimit`, `vinMax`,
      footprint; record in § Corrections.

Acceptance: every `add` row is a loaded part and nothing else moved.
  Check: `ctest --test-dir /tmp/logic-p55 -R 'listing_check|trace_check|library_check|footprint_check|powerswitch_test'` → 5 of 5; `grep -c '|powerswitch|' backend/tests/golden/parts.txt` → H's N + M; `git diff --numstat backend/tests/golden | awk '{d+=$2} END{print d}'` → 0 (est. 4 min); `xcodebuild … test -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/ParametricFootprintDraftTests -only-testing:schematicalTests/PartEditorWindowTests` → `** TEST SUCCEEDED **` (est. 15 min: the counts the tests read move with the parts).
Commit: —

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

(none yet)

## Summary

The largest golden churn of the plan (up to 706 parts): added lines only, reviewed as H's are. The
risk is a wrong enable polarity or current limit on a part, which the five spot checks sample and the
checker of C to E already covered per spec row.
