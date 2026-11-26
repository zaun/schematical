# plan-81-G: Landing tier

Last updated: 2026-11-25
Effort: small (< 1h)
Depends on: plan-81-F

G runs plan-81's landing tier once on the landed tree on `main`, records the observed values, and
archives the plan.

References:

- plan-81-A § Prerequisites (T0 77, P0 = R0 40832, F0 3993, charlcd 72 devices / 72 parts) and
  § 1 of B–F.
- `planning/plan-74-K-ethernet-landing.md` (the precedent table).

## Prerequisites

plan-81-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-81-F complete | `ls planning/completed/plan-81-F-*.md` → exists | NOT MET (2026-11-25) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

Every gate of Phase 1's table shows its expected value in one run.

### Non-goals (explicit constraints)

- No new work. A failing gate is fixed in the letter that owns the area (recorded in that letter's
  § Corrections), and G re-runs.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> fill `Commit:` the moment a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Landing tier

- [ ] Run every gate on a fresh build dir (`/tmp/logic-p81-land`) and record the observed values here.
- [ ] Move `plan-81-A` … `plan-81-G` to `planning/completed/` (A–F as each completes; G with this commit).

| Gate | Command | Expected |
|---|---|---|
| Size and coverage rules (plan-68) | `node scripts/source-size.js --check && node scripts/coverage.js --check` | exit 0 |
| Backend tests | `cmake -S backend -B /tmp/logic-p81-land -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p81-land -j8 && ctest --test-dir /tmp/logic-p81-land -j8` | `100% tests passed`; B's T1 + 1 programs plus any other plan's since |
| Goldens only grew | `git diff <plan-81-E Phase 2 commit>^ <plan-81-E Phase 2 commit> -- backend/tests/golden \| grep -c '^-[^-]'` | 0 |
| OLED parts | `grep -c '\|oled\|' backend/tests/golden/parts.txt` | `awk -F'\t' '$5=="add" && $6 ~ /^oled-/' planning/plan-81-data/verdicts.tsv \| wc -l` |
| Character OLEDs in charlcd | `grep -c 'oled-' schematical/schematical/Parts/charlcd.json` | the `charlcd` specs rows (`awk -F'\t' '$4=="charlcd"' planning/plan-81-data/specs.tsv \| wc -l`); charlcd devices = 72 + that |
| Makers and cap | `awk -F'\t' '$5=="add"{n[$1]++} END{for(k in n)print k,n[k]}' planning/plan-81-data/verdicts.tsv` | only Newhaven, Display Visions, Orient, DLC; each ≤ 300 |
| Stock | `python3 -I planning/plan-81-data/check-specs.py --phase 1` | `ok …` (every `in` row's own In Stock line > 0) |
| Specs complete | `python3 -I planning/plan-81-data/check-specs.py --phase 4` | `ok` |
| Generator idempotent | `python3 -I planning/plan-81-data/gen-oled.py --check` | exit 0 |
| Datasheets on disk | `for f in datasheets/display-oled/*/*.pdf; do head -c5 "$f" \| grep -q '%PDF-' \|\| echo BAD $f; done` | no output |
| Datasheet URLs | `node scripts/check-datasheets.js --host newhavendisplay.com`, then `--host www.lcd-module.de`, `www.lcd-module.com` | exit 0 for each host (the script exits 1 on any failed URL); Orient and DLC refuse scripted fetches (measured at planning) and are not checked here |
| Skipped parts listed | `grep -c 'OLED displays (plan-81)' datasheets/skipped-parts.md` | 1 |
| App build | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p81-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` |
| App tests | `xcodebuild … test -only-testing:schematicalTests/OledTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/KindInfoTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/KindGuideTests -only-testing:schematicalTests/WizardStepsTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/DatasheetCatalogTests` | `** TEST SUCCEEDED **` |
| Runtime proof | `APP=/tmp/p81-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/oled-app.swift /tmp/p81-harness && /tmp/p81-harness/H.app/Contents/MacOS/harness /tmp/p81-oled.png` | `checkerboard ok` |

Acceptance: every gate's observed value matches.
  Check: the table's commands (≈ 40 min: the fresh backend build and suite, the app build and these
  test classes, the harness. This is the plan's one landing run; nothing smaller covers the
  backend, the part files and the app together).
  Result:
Commit:

## Validation Plan

- The table above is the plan's landing tier. The full suites run per the project's background
  policy, not as a plan step.

## Decisions

- Author: **`DatasheetCatalogTests` is in the app gate**: E adds datasheet entries.

## Corrections

## Summary

One run of every gate; no new work.
