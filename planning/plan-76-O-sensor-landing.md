# plan-76-O: Landing tier

Last updated: 2026-11-25
Effort: small (< 1h)
Depends on: plan-76-N

O runs plan-76's landing tier once on the landed tree on `main`, records the observed values,
updates `planning/todo.md`, and archives the plan.

References:

- plan-76-A § 1 and § Prerequisites (T0 77, P0 = R0 40832, F0 3993), plan-76-L § 1, M § 1, N § 1.
- `planning/plan-74-K-ethernet-landing.md`, `planning/completed/plan-48-I-timer-opamp-landing.md`
  (the precedents).

## Prerequisites

plan-76-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-76-N complete | `ls planning/completed/plan-76-N-*.md` → exists, or every N box ticked | NOT MET (2026-11-25: N not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

Every gate of Phase 1's table shows its expected value in one run, and `planning/todo.md` records
what plan-76 closed.

### Non-goals (explicit constraints)

- No new work. A failing gate is fixed in the letter that owns the area (and recorded in that
  letter's § Corrections), and then O re-runs.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> fill `Commit:` the moment a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Landing tier

- [ ] Run every gate of the table on a fresh build dir (`/tmp/logic-p76-land`), and record the
      observed values here.

| Gate | Command | Expected |
|---|---|---|
| Size and coverage rules (plan-68; owner, 2026-11-25) | `node scripts/source-size.js --check && node scripts/coverage.js --check` | exit 0 (est. 30 min: the instrumented backend, app and scripts runs; the only check that covers every changed line) |
| Backend tests | `cmake -S backend -B /tmp/logic-p76-land -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p76-land -j8 && ctest --test-dir /tmp/logic-p76-land -j8` | `100% tests passed`; T at L's start + 1 programs (`sensor_test`), plus any other plan's |
| Goldens only grew | `git diff <plan-76-D first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` over plan-76's commits | 0 |
| Sensor parts | `grep -c '\|sensor\|' backend/tests/golden/parts.txt` | the `add` rows (`awk -F'\t' 'NR>1 && $16=="add"' planning/plan-76-data/candidates.tsv \| wc -l`); at most 3,380 |
| Per-maker cap | `awk -F'\t' 'NR>1 && $16=="add"{n[$1" "$2]++} END{for(k in n)print k,n[k]}' planning/plan-76-data/candidates.tsv` | every (family, maker) at most 300; makers only those of plan-76-A's table |
| Stock check | `awk -F'\t' 'NR>1 && $16=="add" && ($4<=0 \|\| $19=="")' planning/plan-76-data/candidates.tsv \| wc -l` | 0 |
| Every part refused by a circuit | `grep '\|sensor\|' backend/tests/golden/parts.txt \| grep -vc '\|unsimulated$'` | 0 |
| Datasheets | `find datasheets/sensor -name '*.pdf' \| wc -l` | the distinct `pdf` values of the `add` rows |
| App tests | `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p76-land-dd CODE_SIGNING_ALLOWED=NO -only-testing:schematicalTests/SensorTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/KindInfoTests` | `** TEST SUCCEEDED **` (est. 8 min) |
| Harness | `scripts/harness/build.sh scripts/harness/sensor-app.swift /tmp/p76-land-h && /tmp/p76-land-h/H.app/Contents/MacOS/harness` | every line `ok` |

Commit:

### Phase 2 — todo and archive

- [ ] `planning/todo.md:150-152` (the I2C sensor breakout): replace "Needs a sensor kind (none
      today)" with "The sensor kind exists (plan-76)"; the item itself stays open (it is a whole
      board).
- [ ] `planning/todo.md:224` ("sensor: 10 parts"): the count drops by the reference-board parts
      plan-76 added (`TCN75AVOA713`, `VEML7700-TT` when they are `add` rows:
      `grep -c -E '^(TCN75AVOA713|VEML7700-TT)\|' backend/tests/golden/parts.txt`), and the line
      names the ones left.
- [ ] Move `plan-76-A` … `plan-76-O` to `planning/completed/` (A–N as each completes; O with this
      commit).

Acceptance: the todo lines say what landed, and no plan-76 file is left in `planning/`.
  Check: `grep -c 'plan-76' planning/todo.md` → above 0; `ls planning/plan-76-*.md 2>/dev/null | wc -l` → 0 (est. < 1 min).
  Result:
Commit:

## Validation Plan

- Phase 1's table is the landing tier of plan-76.

## Decisions

- Author: **the breakout todo stays open**: plan-76 adds the kind and parts the item lacked, not
  the board.

## Corrections

## Summary

O runs the landing tier once, updates the two todo lines plan-76 affects, and archives the plan.
