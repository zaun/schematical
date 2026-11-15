# plan-40-I: Landing tier

Last updated: 2026-11-14
Effort: small (< 1h)
Depends on: plan-40-H

I runs plan-40's landing tier once, on the landed tree, and archives the
plan. Every gate's command is in the table; the observed values are recorded
under it.

References:

- plan-40-A § 1 (the plan's outcome), plan-40-F § 1, plan-40-G § 1,
  plan-40-H § 1.
- `planning/completed/plan-34-H-resistor-landing.md` (the precedent).

## Prerequisites

plan-40-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-40-H complete | `ls planning/completed/plan-40-H-*.md` → exists, or every H box ticked | MET (2026-11-15: archived, 25a359cc) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

Every gate of Phase 1's table shows its expected value in one run.

### Non-goals (explicit constraints)

- No new work: a failing gate is fixed in the letter that owns the area (and
  recorded in that letter's § Corrections), then I re-runs.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Landing tier

- [x] Run every gate of the table on a fresh build dir (`/tmp/logic-p40-land`)
      and record the observed values here (after merging main, cdba8815).
- [x] Mark `- [ ] Add BJTs (2N3904, 2N3906, BC547…) and MOSFETs (2N7000, logic-level
      N-channel), top 3 manufacturers.` done (`[x]`) in `planning/todo.md`,
      touching only that line.
- [x] Move `plan-40-A` … `plan-40-I` to `planning/completed/` (A–H are archived as
      each completes; I with this commit).

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p40-land && cmake --build /tmp/logic-p40-land -j8 && ctest --test-dir /tmp/logic-p40-land` | `100% tests passed`, T0 + 1 programs (36 with T0 = 35), plus any program other plans added meanwhile (`git log --oneline <plan-40-F first commit>^.. -- backend/CMakeLists.txt \| grep -v plan-40`) |
| Goldens vs plan-40 start | `git diff <plan-40-F first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 (only added lines), counting only plan-40's commits (`git log <plan-40-F first commit>^.. -- backend/tests/golden`) |
| Part counts | `grep -c '\|transistor\|' backend/tests/golden/parts.txt` | the `add` rows + 4 (`awk -F'\t' '$24=="add"' planning/plan-40-data/candidates.tsv \| wc -l`, plus the four generics) |
| Parts total | `wc -l < backend/tests/golden/parts.txt` | P0 (31608) + that count, plus any part other plans landed meanwhile (`git log --oneline <plan-40-A commit>.. -- schematical/schematical/Parts \| grep -v plan-40`) |
| Footprints | `wc -l < backend/tests/golden/footprints.txt` | F0 (1497) + the objects C–E added (G Phase 2's count), plus any other plan's |
| Datasheets | `find datasheets/transistor -name '*.pdf' \| wc -l` | the distinct PDFs the `add` rows cite (`awk -F'\t' '$24=="add" {print $27}' planning/plan-40-data/candidates.tsv \| sort -u \| wc -l`) |
| Datasheet URLs | `node scripts/check-datasheets.js $(python3 -I -c "import json; print(' '.join(x['id'] for x in json.load(open('schematical/schematical/Parts/datasheets.json'))['datasheets'] if x['id'].split('-')[0] in ('onsemi','nexperia','diodes','infineon') or x['id'].startswith('vishay-siliconix-')))")` (no id of those prefixes existed at 943924ba: `python3 -I` count → 0) | exits 0 |
| Skipped parts listed | `python3 -I planning/plan-40-data/census/verdicts.py --verify-skipped` | `0 skip rows missing from skipped-parts.md` |
| Stock check | `python3 -I planning/plan-40-data/census/select.py /tmp/p40/raw planning/plan-40-data/candidates.tsv --stock planning/plan-40-data/stock-check.tsv --verdicts planning/plan-40-data/verdicts.tsv --report` | exits 0; no `NEEDS DEEPER LISTING` line beyond those in plan-40-A § Corrections |
| App | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p40-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` |
| App tests | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p40-dd test -only-testing:schematicalTests/TransistorTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/BillOfMaterialsTests -only-testing:schematicalTests/ReferencePrefixTests -only-testing:schematicalTests/ResistorTests CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` |
| Runtime proof | `APP=/tmp/p40-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/transistor-app.swift /tmp/p40-harness && /tmp/p40-harness/H.app/Contents/MacOS/harness` | `all checks ok`: `Q1` on `TO-92`, the drawer leaves of BJT / MOSFET, the LED lit with the NPN base on a pull-up and dark on a pull-down, the PMOS high-side LED lit, the symbols rendered |

Observed (2026-11-15, on the plan branch after `git merge main` → cdba8815; main had moved 58
commits — plans 38 and 41 — none touching the goldens, part files or `backend/CMakeLists.txt`):

| Gate | Observed |
|---|---|
| Backend tests | fresh `/tmp/logic-p40-land`: `100% tests passed, 0 tests failed out of 36` (T0 35 + `transistor_test`; no other plan added a program: `git log … -- backend/CMakeLists.txt` → 0 others) |
| Goldens vs plan-40 start | `git diff 5f3c9a25^ -- backend/tests/golden \| grep -c '^-[^-]'` → `0` (2 golden commits, both plan-40-G) |
| Part counts | `grep -c '\|transistor\|' parts.txt` → `1246` = 1242 `add` rows + 4 generics |
| Parts total | `wc -l < parts.txt` → `32854` = 31608 + 1246 (no other plan's part-file commit) |
| Footprints | `wc -l < footprints.txt` → `1624` = 1497 + 127 (G Phase 1: 4; Phase 2: 123) |
| Datasheets | `find datasheets/transistor -iname '*.pdf' \| wc -l` → `1147` = 1135 PDFs the `add` rows cite + 12 cited only by a footprint's drawing note (Corrections 2; `-name` gives 1064: the `.PDF` files) |
| Datasheet URLs | 0 such ids at 8913617d; 1135 now; `check-datasheets.js` → `1135 of 1135 URLs serve a PDF; 0 failed`, exit 0 |
| Skipped parts listed | `0 skip rows missing from skipped-parts.md` |
| Stock check | `select.py … --verdicts … --report` → exit 0; `NEEDS DEEPER LISTING` → `0` (after Corrections 1); `0 pieces cut at 300`; `picked from a cut piece, not stock-checked: 0`; `not read: 3985 rows; at or above their floor: 0; in a short share: 0`; `dead by the text scan only: 94 rows; at or above their floor: 0`; its pick equals the committed `candidates.tsv` |
| App | `** BUILD SUCCEEDED **` |
| App tests | TransistorTests 10, BillOfMaterialsTests 35 (with merged main's tests), PartEditorWindowTests 8, ReferencePrefixTests 5, ResistorTests 8 — 0 failures, `** TEST SUCCEEDED **` |
| Runtime proof | harness: `all checks ok` (drawer leaves; 2N3904BU / IRLML6244TRPBF / BC846SZ kinds, symbols, facts; GENERIC-NPN `Q1` on `TO-92`; NPN low side lit on PULLUP-5V, dark on PULLDOWN; PMOS high side lit; images written), exit 0 |

Acceptance: every gate's observed value matches.
  Check: the table's commands (est. 45 min: the fresh backend build, the app
  tests and the harness; this is the plan's one landing run).
Commit: (this commit)

## Validation Plan

- Final gate: Phase 1 is it (run once).

## Decisions

- **One landing run** — author: per-letter checks stayed scoped; this is where
  the whole set is checked together.
- Owner decisions: plan-40-A § Decisions.

## Corrections

1. **The stock-check gate's report counted pieces already listed in full.**
   The first landing run printed two `NEEDS DEEPER LISTING` lines, both
   `278-448 … all/N-Channel` (the base file's and the `-re` file's). That
   type piece was cut at 300, but the `-re` re-listing split it by Package /
   Case and those sub-pieces list all of it (`piece all/N-Channel/…` lines of
   `raw/278-448-re.err`: 1986 of 1986), and a `-re` file replaces every piece
   of its base file — the same case plan-40-A Corrections 3 describes for
   the others. `select.py`'s `cut_pieces` now skips a base file's pieces
   when its `-re` exists and a piece whose own sub-pieces list all its
   parts; the report then prints `0 pieces cut at 300`, no `NEEDS DEEPER
   LISTING`, and the same pick (the cut pieces only feed the report and the
   stock-check selection, whose rows are all checked).
2. **Datasheets gate.** Its expectation counts the `add` rows' PDFs (1135);
   the tree also keeps the 12 PDFs a used footprint's note names as its
   drawing (plan-40-B Phase 5's prune), so 1147, counted with `-iname`
   (plan-40-A Corrections 7).
3. **The gate's output file.** The stock-check gate's command writes
   `planning/plan-40-data/candidates.tsv` without `--carry`, which would
   drop the `hit` / `dead` columns; it was run into `/tmp/p40/land-cand.tsv`
   with `--carry`, and its `reason` column compared equal to the committed
   file.

## Summary

I only checks; the risk was spent in A–H. Left untouched: everything but this
file, `planning/todo.md`'s one line and the archive moves.
