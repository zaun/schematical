# plan-29-N: Landing tier — full gates, harness, runtime proof

Last updated: 2026-11-09
Effort: small (< 1h)
Depends on: plan-29-M

N runs plan-29's landing tier once, records what it observed, and archives
the sub-plans.

References: plan-29-A § 1 (the outcome), plan-29-H § 4.3 (the diode table),
plan-29-M § 1 (the harness proof), `planning/completed/plan-24-I-converter-app.md`
(the landing-tier precedent).

## Prerequisites

plan-29-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-29-M complete | `ls planning/completed/plan-29-M-*.md` → exists | MET (2026-11-09) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

Every gate of the table below passes on a fresh build, and the observed
values are recorded under it.

### Non-goals (explicit constraints)

- No new work: a failing gate is fixed in the letter that owns the area
  (and recorded in that letter's § Corrections), then N re-runs.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Landing tier

- [x] Run every gate of the table on a fresh build dir and record the
      observed values (2026-11-09, after merging main at 927c595; fresh
      `/tmp/logic-p29-land`; the commands are /tmp/p29-land.sh):
      - Backend tests: `100% tests passed, 0 tests failed out of 30`;
        30 programs (T0 29 + `diode_test`).
      - Goldens vs plan-29 start: `git diff 737b58d^ -- backend/tests/golden
        | grep -c '^-[^-]'` → 0.
      - Part counts: `|diode|` lines 11152 = add rows 11152
        (N_I 3179 + N_J 2730 + N_K 2492 + N_L 2751).
      - Parts total: 26366 = P0 15214 + 11152; other plans' Parts commits
        since a98aecb: 0.
      - Datasheets: 3010 PDFs = 3010 distinct add-row URLs (the first run
        showed 3011: `place.py` pruned `.pdf` only, so
        `onsemi/NSR0530H-D.PDF`, whose one row became skip, stayed; fixed
        and pruned, plan-29-A § Corrections).
      - Skipped parts listed: `0 skip rows missing from skipped-parts.md`.
      - App: `** BUILD SUCCEEDED **`.
      - Runtime proof: `scripts/harness/diode-app.swift` → the five
        symbol/fact lines ok (1N4001-E3/54 rectifier 50 V / 1 A plainDiode,
        1N5817-E3/54 Schottky, 1N4728A Zener 3.3 V / 1 W, BAS21SLT1G array
        block, 2W01G-E4/51 bridge block), diode-OR `00 lit, 01 lit, 10 lit,
        11 dark`, `all checks ok`.
- [x] Move `plan-29-A` … `plan-29-N` to `planning/completed/` (A–M were
      archived as each completed; N with this commit).

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p29-land && cmake --build /tmp/logic-p29-land -j8 && ctest --test-dir /tmp/logic-p29-land` | `100% tests passed`, T0 + 1 programs (30 with T0 = 29) |
| Goldens vs plan-29 start | `git diff <plan-29-H first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 (only added lines) |
| Part counts | `grep -c '\|diode\|' backend/tests/golden/parts.txt` | N_I + N_J + N_K + N_L = the `add` rows of `candidates.tsv` (`awk -F'\t' '$24=="add"' planning/plan-29-data/candidates.tsv \| wc -l`) |
| Parts total | `wc -l < backend/tests/golden/parts.txt` | P0 (15214) + that count, plus any part other plans landed meanwhile (`git log --oneline <plan-29-A commit>.. -- schematical/schematical/Parts \| grep -v plan-29`) |
| Datasheets | `ls datasheets/diode/*/*.pdf \| wc -l` | the distinct PDFs the `add` rows cite (`awk -F'\t' '$24=="add" {print $23}' planning/plan-29-data/candidates.tsv \| sort -u \| wc -l`) |
| Skipped parts listed | `python3 -I planning/plan-29-data/census/decide.py --verify-skipped` | `0 skip rows missing from skipped-parts.md` |
| App | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p29-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` |
| Runtime proof | plan-29-M Phase 2's harness, re-run on the landed tree | the LED lights for inverter inputs 00, 01, 10 and is dark for 11; the five symbol/fact lines as M § 1 |

Acceptance: every gate's observed value matches.
  Check: the table's commands (est. 25 min: the fresh backend build and the
  harness; this is the plan's one landing run).
  Result: every gate matches (above).
Commit: 16d9f5b

## Validation Plan

- Final gate: Phase 1 is it (run once).

## Decisions

- Landing tier as plan-24-I's, plus the runtime proof of M.

## Corrections

- **The datasheet gate found a prune bug** — `place.py` matched `.pdf`
  case-sensitively, so its `--prune` / `--verify` skipped onsemi's `.PDF`
  files; one (`NSR0530H-D.PDF`) had no add row left. Fixed in `place.py`
  (plan-29-A's script) and the gate re-run: 3010 = 3010.

## Summary

N proves the whole plan once: tests, goldens that only grew, part and
datasheet counts that match the census, the app build and the diode-OR
running in the app's own simulation code.
