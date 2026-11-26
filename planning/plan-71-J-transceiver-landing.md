# plan-71-J: Landing tier

Last updated: 2026-11-25
Effort: small (< 1h)
Depends on: plan-71-I

J runs plan-71's landing tier once, on the landed tree on `main`, and
archives the plan. Every gate's command is in the table, and the observed
values are recorded under it.

References:

- plan-71-A § 1 and § Prerequisites (the baselines T0 77, P0 40832, R0
  40832, F0 3993, E0 40832), plan-71-G § 1, plan-71-H § 1, plan-71-I § 1.
- `planning/completed/plan-48-I-timer-opamp-landing.md` (the precedent).

## Prerequisites

plan-71-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-71-I complete | `ls planning/completed/plan-71-I-*.md` → exists, or every I box ticked | NOT MET (2026-11-25: I not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

Every gate of Phase 1's table shows its expected value in one run.

### Non-goals (explicit constraints)

- No new work. A failing gate is fixed in the letter that owns the area
  (and recorded in that letter's § Corrections), then J re-runs.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Landing tier

- [ ] Run every gate of the table on a fresh build dir
      (`/tmp/logic-p71-land`, `/tmp/p71-land-dd`) and record the observed
      values here.
- [ ] Move `plan-71-A` … `plan-71-J` to `planning/completed/` (A–I are
      archived as each completes; J goes with this commit).

| Gate | Command | Expected |
|---|---|---|
| Size and coverage rules (plan-68; owner, 2026-11-25) | `node scripts/source-size.js --check && node scripts/coverage.js --check` | exit 0: no in-scope file over 1000 code lines and no uncovered line in the backend, app or scripts tiers (est. 30 min: the app tier is measured only by plan-68's full Debug-suite run, 1,634 s at plan-68's planning, and nothing smaller shows the new Swift lines run) |
| Backend suite | `cmake -S backend -B /tmp/logic-p71-land && cmake --build /tmp/logic-p71-land -j8 && ctest --test-dir /tmp/logic-p71-land -j8` | `100% tests passed`; programs = T0 + 1, plus any other plan's since |
| Goldens only grew | `git diff <plan-71-A start>..HEAD -- backend/tests/golden/ \| grep -c '^-[^-]'` | 0 |
| Parts per protocol | `grep '\|transceiver\|' backend/tests/golden/parts.txt \| grep -o 'proto=[a-z0-9]*' \| sort \| uniq -c` | four lines, each 1 (generic) + that protocol's `add` rows |
| Per-maker cap | `awk -F'\t' 'NR>1 && $NF=="add"{n[$1" "$2]++} END{for(k in n)print n[k],k}' planning/plan-71-data/candidates.tsv` | every value ≤ 200 |
| Every cited PDF is present and listed | `python3 -I planning/plan-71-data/census/verdicts.py --check` | `0 add rows without a placed PDF` |
| Datasheet URLs | `python3 -I planning/plan-71-data/census/fetch.py --check-urls` | `<n> of <n> URLs serve a PDF; 0 failed` |
| Skipped parts recorded | `python3 -I planning/plan-71-data/census/final.py --check` | `0 skip/out rows missing from skipped-parts.md` |
| In stock | `awk -F'\t' 'NR>1 && $4<=0' planning/plan-71-data/stock-check.tsv \| wc -l` | 0 |
| App build and tests | `xcodebuild … test -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/KindGuideTests -only-testing:schematicalTests/WizardStepsTests -only-testing:schematicalTests/KindInfoTests -only-testing:schematicalTests/TransceiverTests` | `** TEST SUCCEEDED **` |
| Harness | `/tmp/p71-harness/H.app/Contents/MacOS/harness` (I Phase 2) | `all checks ok` |

## Corrections

(none yet)
