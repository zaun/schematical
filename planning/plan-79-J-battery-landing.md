# plan-79-J: Battery holders: landing tier

Last updated: 2026-11-25
Effort: small (< 1h)
Depends on: plan-79-I

J runs the landing tier once for every area plan-79 touched, updates the docs' counts and
`planning/todo.md`, and archives the plan.

## Prerequisites

plan-79-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-79-A … I complete | `ls planning/completed/plan-79-{A,B,C,D,E,F,G,H,I}-*.md \| wc -l` → 9 | NOT MET (2026-11-25: none started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

The single behavioral outcome of plan-79 (plan-79-A) holds, shown by the checks below, and the plan
is archived.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Landing tier

- [ ] Backend: `cmake --build /tmp/logic-p79 -j8 && ctest --test-dir /tmp/logic-p79 -j8` →
      `100% tests passed` (est. 1 min).
- [ ] Generators: `python3 -I planning/plan-79-data/gen-battery.py --check` and
      `merge-fp.py --check` on each `footprints-<maker>.json` → exit 0 (est. < 1 min).
- [ ] Counts: `grep -c '|Connector/Battery/' backend/tests/golden/parts.txt` → the sum of
      `build.py --expect` over the four makers (est. < 1 min).
- [ ] Datasheets: `node scripts/check-datasheets.js --host keyelco.com`, `--host memoryprotectiondevices.com`,
      and the Adam Tech and Assmann hosts → every battery entry answers with a PDF (est. 5 min).
- [ ] App: `xcodebuild test … -only-testing:schematicalTests/BatteryHolderTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/BillOfMaterialsTests -only-testing:schematicalTests/KiCadExportTests` →
      `** TEST SUCCEEDED **`, and the plan-79-I harness → every line `ok` (est. 15 min: the app
      areas plan-79 touched, nothing wider).
- [ ] plan-68's checks: `node scripts/source-size.js --check` → exit 0;
      `node scripts/coverage.js --check --backend` → exit 0 (est. 6 min).

Acceptance: every line above passes.
  Result:
Commit:

### Phase 2 — Docs, todo, archive

- [ ] `docs/parts-format.md` § `connector`: the battery paragraph states the shipped count per maker
      (from the golden).
- [ ] `planning/todo.md`: under lines 143–144 (555 flasher, "a 9 V battery connector") and 153–154
      (Pi HAT, "an I2C RTC with a battery holder"), add a sub-line "battery holders: plan-79"; tick
      neither item (each needs more than a battery holder).
- [ ] Move `planning/plan-79-*.md` to `planning/completed/` (each letter as it completes; J last).

Acceptance: the docs and todo name plan-79, and no plan-79 letter is left in `planning/`.
  Check: `grep -c 'plan-79' docs/parts-format.md planning/todo.md` → above 0 each; `ls planning/plan-79-*.md 2>/dev/null | wc -l` → 0 (est. < 1 min).
  Result:
Commit:

## Decisions

- Author: the full suite is not a plan step; it runs in the background per the project's policy.

## Corrections

## Summary

The landing tier for plan-79 and its archive.
