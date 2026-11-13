# plan-37-H: Landing tier

Last updated: 2026-11-12
Effort: small (< 1h)
Depends on: plan-37-G

H runs plan-37's landing tier once, on the landed tree, and archives the
plan. Every gate's command is in the table; the observed values are
recorded under it.

References:

- plan-37-A § 1 (the plan's outcome), C § 1, D–F § 1, G § 1.
- `planning/completed/plan-35-H-charlcd-large-landing.md` (the precedent).

## Prerequisites

plan-37-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-37-A..G complete | `ls planning/completed/plan-37-[A-G]-*.md \| wc -l` → 7 | MET (2026-11-13: 7) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

Every gate of Phase 1's table shows its expected value in one run.

### Non-goals (explicit constraints)

- No new work: a failing gate is fixed in the letter that owns the area
  (and recorded in that letter's § Corrections), then H re-runs.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Landing tier

- [x] Run every gate of the table on a fresh build dir
      (`/tmp/logic-p37-land`) and record the observed values here (below
      the table). main had not moved from 31f79ce, so no merge preceded it.
- [x] Move `plan-37-H` to `planning/completed/` (A–G are archived as each
      completes).

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p37-land && cmake --build /tmp/logic-p37-land -j8 && ctest --test-dir /tmp/logic-p37-land` | `100% tests passed`, T0 + 1 programs (33 with T0 = 32), plus any program other plans added meanwhile (`git log --oneline <plan-37-A commit>.. -- backend/CMakeLists.txt \| grep -v plan-37`) |
| Goldens vs plan-37 start | `git diff <plan-37-C first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'`, counting only plan-37's commits (`git log <plan-37-C first commit>^.. -- backend/tests/golden`) | 0 (only added lines) |
| Register parts | `grep -c '\|register\|' backend/tests/golden/parts.txt` | D's + E's `register` `add` rows (`awk -F'\t' '$5=="register" && $19=="add"' planning/plan-37-data/candidates.tsv \| wc -l`; 272 in scope at planning) |
| `dff` parts | `grep -c '\|dff\|' backend/tests/golden/parts.txt` | 706 + F's `add` rows (`awk -F'\t' '$5=="dff" && $19=="add"' … \| wc -l`; 198 in scope at planning) |
| 74_670 parts | `grep -c '\|mux\|' backend/tests/golden/parts.txt` | 346 + the `mux` `add` rows (2 in scope at planning) |
| Parts total | `wc -l < backend/tests/golden/parts.txt` | P0 (27828) + the three counts above' additions, plus parts other plans landed meanwhile (`git log --oneline <plan-37-A commit>.. -- schematical/schematical/Parts \| grep -v plan-37`) |
| Register behavior | `/tmp/logic-p37-land/register_test \| tail -1` | `register_test: all passed` |
| Footprints | `/tmp/logic-p37-land/footprint_check` | passes; `grep -c '^\(DHVQFN-20\|DHVQFN-24\|DQFN-20-ONSEMI\|TSSOP-64\)\|' backend/tests/golden/footprints.txt` → the ids B drew (4 at planning) |
| New datasheets reachable | `node scripts/check-datasheets.js <the plan-37 ids>` (the ids of `planning/plan-37-data/datasheets.tsv`) | every id answers PDF |
| Skipped and obsolete records | `python3 -I planning/plan-37-data/census/verdicts.py --verify-records` | `0 skip rows missing from skipped-parts.md, 0 obsolete PDFs left` |
| App | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p37-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` |
| App tests | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p37-dd test -only-testing:schematicalTests/RegisterTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/DatasheetCatalogTests CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` |

Observed (2026-11-13, worktree at dc1e2f2):

| Gate | Observed |
|---|---|
| Backend tests | `100% tests passed, 0 tests failed out of 33` (T0 + 1; no other plan's commits since 31f79ce) |
| Goldens vs plan-37 start | `git diff 31f79ce -- backend/tests/golden \| grep -c '^-[^-]'` → 0 |
| Register parts | 220 = D 153 + E 67 `register` `add` rows (A, D, E § Corrections) |
| `dff` parts | 867 = 706 + 161 (F § Corrections) |
| 74_670 parts | 346 (no `mux` `add` row; A, D § Corrections) |
| Parts total | 28209 = 27828 + 220 + 161 (no other plan's parts) |
| Register behavior | `register_test: all passed` |
| Footprints | `1390 footprints compared, 28209 parts loaded`; the four new ids → 4 lines |
| New datasheets reachable | `125 of 125 URLs serve a PDF; 0 failed` |
| Skipped and obsolete records | `0 skip rows missing from skipped-parts.md, 0 obsolete PDFs left` |
| App | `** BUILD SUCCEEDED **` |
| App tests | `** TEST SUCCEEDED **` (RegisterTests, PartKindRoundTripTests, PartKindKeysTests, PartEditorWindowTests, DatasheetCatalogTests all passed) |

Est. 20 min (the backend build and ctest 5, the app build and the five
test classes 12, the datasheet check 3). The full Swift suite is not a
gate here; it runs in the background per the project's practice.

Commit: a4074ae (archived), with its record in the next commit

## Decisions

- **The tier covers every area plan-37 touched**: backend kind and
  goldens, footprints, datasheets and their records, the app's catalog,
  form and editor lists.

## Corrections

(none yet)

## Summary

One run of the gates above, recorded here, then the plan is archived.
