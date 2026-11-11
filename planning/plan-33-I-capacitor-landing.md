# plan-33-I: Landing tier — full gates, harness, runtime proof

Last updated: 2026-11-11
Effort: small (< 1h)
Depends on: plan-33-H

I runs plan-33's landing tier once, records what it observed, ticks the
to-do items, and archives the sub-plans.

References: plan-33-A § 1 (the outcome), plan-33-H § 1 (the harness proof),
`planning/completed/plan-29-N-diode-landing.md` (the precedent).

## Prerequisites

plan-33-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-33-H complete | `ls planning/completed/plan-33-H-*.md` → exists | NOT MET (2026-11-11: planning) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

Every gate of the table below passes on a fresh build, and the observed
values are recorded under it.

### Non-goals (explicit constraints)

- No new work: a failing gate is fixed in the letter that owns the area
  (and recorded in that letter's § Corrections), then I re-runs.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Landing tier

- [ ] Run every gate of the table on a fresh build dir and record the
      observed values here.
- [ ] `planning/todo.md`: tick "Add a Generic Capacitor…" and "Add top 3
      Capacitor manufactuerers…" with `(plan-33)`.
- [ ] Move `plan-33-A` … `plan-33-I` to `planning/completed/` (each earlier
      letter is archived as it completes; I with this commit).

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p33-land && cmake --build /tmp/logic-p33-land -j8 && ctest --test-dir /tmp/logic-p33-land` | `100% tests passed`, T0 + 1 programs (31 with T0 = 30) |
| Goldens vs plan-33 start | `git diff <plan-33-E first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 (only added lines) |
| Part counts | `grep -c '\|capacitor\|' backend/tests/golden/parts.txt` | the `add` rows + 1 (`awk -F'\t' '$25=="add"' planning/plan-33-data/candidates.tsv \| wc -l`, plus `GENERIC-CAPACITOR`); at most 1049 (1048 buckets + 1, plan-33-A § 2) |
| One part per bucket | `awk -F'\t' '$25=="add" {print $1}' planning/plan-33-data/candidates.tsv \| sort \| uniq -d \| wc -l` | 0 |
| Parts total | `wc -l < backend/tests/golden/parts.txt` | P0 (26729) + that count, plus any part other plans landed meanwhile (`git log --oneline <plan-33-A commit>.. -- schematical/schematical/Parts \| grep -v plan-33`) |
| Datasheets | `ls datasheets/capacitor/*/*.pdf \| wc -l` | the distinct PDFs the `add` rows cite (`awk -F'\t' '$25=="add" {print $27}' planning/plan-33-data/candidates.tsv \| sort -u \| wc -l`) |
| Skipped parts listed | `python3 -I planning/plan-33-data/census/decide.py --verify-skipped` | `0 skip rows missing from skipped-parts.md` |
| Datasheet URLs answer | `for h in yageogroup.com calchip.com datasheets.kyocera-avx.com; do node scripts/check-datasheets.js --host $h; done` | exit 0 for each host (every capacitor datasheet URL answers with a PDF) |
| App | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p33-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` |
| App tests | `xcodebuild … test -only-testing:schematicalTests/CapacitorTests -only-testing:schematicalTests/PullResistorTests -only-testing:schematicalTests/BillOfMaterialsTests CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` |
| Runtime proof | `scripts/harness/build.sh scripts/harness/capacitor-app.swift` | `all checks ok` (plan-33-H § 1) |

Acceptance: every gate's observed value matches.
  Check: the table's commands (est. 30 min: the fresh backend build, the app
  tests and the harness; this is the plan's one landing run).
Commit: —

## Validation Plan

- Final gate: Phase 1 is it (run once).

## Decisions

- Landing tier as plan-29-N's, plus the one-part-per-bucket check (the
  owner's "1k–2k parts, no near-duplicates").

## Corrections

(Filled in during execution.)

## Summary

I proves the whole plan once: tests, goldens that only grew, part and
datasheet counts that match the census, one part per bucket, the app build,
its tests and the harness.
