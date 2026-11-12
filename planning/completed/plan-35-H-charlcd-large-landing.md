# plan-35-H: 20x4 / 40x2 / 40x4 LCDs — landing tier

Last updated: 2026-11-11
Effort: small (< 1h)
Depends on: plan-35-G

H runs plan-35's landing tier once, on the landed tree, records the
results and archives the eight plan-35 files. Each earlier letter used
only checks scoped to its own work. This is the one run of the gates for
every area plan-35 touched: backend, part files, footprints, datasheets
and the app.

References:

- plan-35-A § Prerequisites; every plan-35 letter's § 1.
- `planning/completed/plan-29-N-diode-landing.md`,
  `planning/plan-34-H-resistor-landing.md` (the landing-tier precedent).
- `docs/backend.md` (the test commands), `AGENTS.md`.

## Prerequisites

The gate is plan-35-A § Prerequisites. H adds:

| Must be true | Command | Status |
|---|---|---|
| plan-35-A..G complete | `grep -c '^Commit: —' planning/plan-35-[A-G]-*.md` → 0 for each (or the files are in `planning/completed/`) | MET (2026-11-11: 0 for each of A–G) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. **If you stop, report the current status of
> *all* prerequisites.**

## 1. Goal

Every gate of § 2 passes in one run on one commit, and that commit's hash
is recorded here.

### Non-goals (explicit constraints)

- No code change. A gate failure is a bug in the letter that owns the
  area: fix it there (in a new commit named for that letter), record it in
  that letter's Corrections, then re-run the whole tier.

## 2. The landing tier

| Gate | Command | Must show |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p35-land && cmake --build /tmp/logic-p35-land -j8 && ctest --test-dir /tmp/logic-p35-land` | `100% tests passed`, T0 programs, plus any program other plans added meanwhile (`git log --oneline <plan-35-A commit>.. -- backend/CMakeLists.txt \| grep -v plan-35`) |
| Character LCD parts | `grep -c '\|charlcd\|' backend/tests/golden/parts.txt` | 5 + the `add` rows of `candidates.tsv` (72 at planning) |
| Every-part check | `/tmp/logic-p35-land/charlcd_test \| grep -c '^FAIL'` | 0 |
| Footprints used | `/tmp/logic-p35-land/footprint_check` | passes (no unused, every new id in the golden) |
| New datasheets reachable | `node scripts/check-datasheets.js` on the 67 plan-35 ids (its `--help` option for a subset) | every id answers PDF |
| Skipped parts listed | the Phase 3 loop of plan-35-A over `skip` rows | every name once |
| App | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p35-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` |
| App tests | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p35-dd test -only-testing:schematicalTests/CharacterLCDTests -only-testing:schematicalTests/SchematicSVGTests -only-testing:schematicalTests/DatasheetCatalogTests CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` |

Estimated time: about 15 minutes, mostly the app build and tests. Each
gate is the smallest one that covers its area; the backend suite is the
cheapest way to run the goldens, `charlcd_test` and `voltage_test`
together.

## Phases

> Tick `- [x]` in the same commit as the work; `- [~]` for partial with one
> line of what remains. Fill each `Commit:` line when the phase lands. **An
> unticked box means NOT DONE.**

### Phase 1 — run the tier

- [x] Run every gate of § 2 on one commit; record each command's result
      line here. — all on b604b9d (plan-35 merged with main at 1be9390,
      plan-33 included), 2026-11-11:
      - Backend tests: `ctest --test-dir /tmp/logic-p35-land` → "100% tests
        passed, 0 tests failed out of 31" (T0 30 + plan-33's one program;
        `grep -c '^logic_test' backend/CMakeLists.txt` → 31).
      - Character LCD parts: `grep -c '|charlcd|' backend/tests/golden/parts.txt`
        → 72 (5 + 67 `add` rows).
      - Every-part check: `/tmp/logic-p35-land/charlcd_test | grep -c '^FAIL'` → 0.
      - Footprints used: `/tmp/logic-p35-land/footprint_check` → "1386
        footprints compared, 27828 parts loaded", no failure line.
      - New datasheets: `node scripts/check-datasheets.js <67 ids>` → "67 of 67
        URLs serve a PDF; 0 failed".
      - Skipped parts: `census/skipcheck.py` → "59 skip names; 59 found exactly once".
      - App: `xcodebuild … build` → `** BUILD SUCCEEDED **`.
      - App tests (CharacterLCDTests, SchematicSVGTests, DatasheetCatalogTests)
        → "Executed 15 tests, with 0 failures", `** TEST SUCCEEDED **`.
      - The merge's goldens are main's lines plus plan-35's only: parts 27761
        → 27828 (+67, 0 removed), traces the same, footprints 1352 → 1386
        (+34, 0 removed).

Commit: ecc13ba

### Phase 2 — close

- [x] Mark `planning/todo.md`'s item "Add 20x4 and 40x2 and 40x4
      character LCD modules" `[x]` (`grep -n '20x4 and 40x2' planning/todo.md`
      → line 46 on 2026-11-11). Commit only that line's change: other edits
      in that file may belong to the owner. — ticked in the main
      checkout's working copy (line 46); not committed: the line exists
      only in the owner's uncommitted `todo.md` (Corrections).
- [x] `git mv planning/plan-35-*.md planning/completed/`. The data folder
      `planning/plan-35-data/` stays where it is, as plan-29's does.

Check: `ls planning/plan-35-*.md` → no matches;
`ls planning/completed/plan-35-*.md | wc -l` → 8. — done in this commit
(its hash is in the merge report; a commit cannot name itself).

Commit: the archive commit after ecc13ba ("plan-35-H Phase 2"; Corrections)

## Validation Plan

§ 2 is the validation plan of all of plan-35.

## Decisions

- **One run of the tier, at the end** (author, write-plan rule): the
  earlier letters ran only checks scoped to their own work.

## Corrections

- **The todo item is not in the committed `todo.md`** (2026-11-11:
  `grep -n '20x4' planning/todo.md` at b604b9d → nothing; the main
  checkout's working copy has it at line 46, among the owner's
  uncommitted edits). Committing the tick would commit the owner's line
  too, so the tick is made in the working copy only and left for the
  owner to commit with the rest of their edits.
- **Phase 2's Commit line** stays as this archive commit: its hash cannot
  be written inside it, and the plan files leave `planning/` in it.

## Summary

A single, scripted pass/fail run over every area plan-35 touched, then
archive.
