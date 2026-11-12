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
| plan-33-H complete | `ls planning/completed/plan-33-H-*.md` → exists | MET (2026-11-11, at execution: 23916c5) |

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

- [x] Run every gate of the table on a fresh build dir and record the
      observed values here (below the table).
- [x] `planning/todo.md`: tick "Add a Generic Capacitor…" and "Add top 3
      Capacitor manufactuerers…" with `(plan-33)`. The two items exist only
      in the main checkout's uncommitted `planning/todo.md` (lines 43–44;
      the committed file has no capacitor item), so they are ticked there
      after the merge and left uncommitted with the rest of that edit
      (§ Corrections 2).
- [x] Move `plan-33-A` … `plan-33-I` to `planning/completed/` (each earlier
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

Observed (2026-11-11, worktree-P-33 at 23916c5):

| Gate | Observed |
|---|---|
| Backend tests | fresh `/tmp/logic-p33-land`: `100% tests passed, 0 tests failed out of 31` (T0 30 + `capacitor_test`) |
| Goldens vs plan-33 start | `git diff d97e95d^ -- backend/tests/golden \| grep -c '^-[^-]'` → 0 |
| Part counts | `grep -c '\|capacitor\|'` → 1032 = 1031 `add` rows + `GENERIC-CAPACITOR` (≤ 1049) |
| One part per bucket | `uniq -d` → 0 |
| Parts total | 27761 = P0 26729 + 1032 (no other plan's part commit on this branch: `git log --oneline 656b6d1.. -- schematical/schematical/Parts \| grep -v -i plan-33` → 0 lines) |
| Datasheets | `ls datasheets/capacitor/*/*.pdf \| wc -l` → 81 = the distinct placed PDFs of the `add` rows (`place.py --verify` → `82 URLs, 81 PDFs, 0 missing, 0 unused`; plan-33-B § Corrections 4) |
| Skipped parts listed | `0 skip rows missing from skipped-parts.md` |
| Datasheet URLs answer | yageogroup.com 50 of 50, calchip.com 7 of 7, datasheets.kyocera-avx.com 23 of 23, ele.kyocera.com 1 of 1 (`N of N URLs serve a PDF; 0 failed`, exit 0 each) |
| App | `** BUILD SUCCEEDED **` |
| App tests | `Executed 22 tests, with 0 failures`, `** TEST SUCCEEDED **` |
| Runtime proof | harness rebuilt and run: `all checks ok`, exit 0 |

Acceptance: every gate's observed value matches.
  Check: the table's commands (est. 30 min: the fresh backend build, the app
  tests and the harness; this is the plan's one landing run).
Commit: 050d310

## Validation Plan

- Final gate: Phase 1 is it (run once).

## Decisions

- Landing tier as plan-29-N's, plus the one-part-per-bucket check (the
  owner's "1k–2k parts, no near-duplicates").

## Corrections

1. **Datasheet hosts.** The kept PDFs sit on yageogroup.com, calchip.com,
   datasheets.kyocera-avx.com and ele.kyocera.com (KGM_Series_e.pdf), so the
   URL gate ran on those four (81 URLs in all); the TAP link was moved to
   https on datasheets.kyocera-avx.com (plan-33-G § Corrections 2).
2. **The to-do items** are in the main checkout's uncommitted
   `planning/todo.md` only; they are ticked there after the merge, and that
   file is left for its owner to commit.
3. **Build directory** `/tmp/logic-p33-land` for the fresh build (plan-33-A
   § Corrections 1: `/tmp/logic-p33` belongs to the main checkout).

## Summary

I proves the whole plan once: tests, goldens that only grew, part and
datasheet counts that match the census, one part per bucket, the app build,
its tests and the harness.
