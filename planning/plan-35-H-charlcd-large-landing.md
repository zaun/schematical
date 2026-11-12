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
| plan-35-A..G complete | `grep -c '^Commit: —' planning/plan-35-[A-G]-*.md` → 0 for each (or the files are in `planning/completed/`) | NOT MET (2026-11-11) |

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

- [ ] Run every gate of § 2 on one commit; record each command's result
      line here.

Commit: —

### Phase 2 — close

- [ ] Mark `planning/todo.md`'s item "Add 20x4 and 40x2 and 40x4
      character LCD modules" `[x]` (`grep -n '20x4 and 40x2' planning/todo.md`
      → line 46 on 2026-11-11). Commit only that line's change: other edits
      in that file may belong to the owner.
- [ ] `git mv planning/plan-35-*.md planning/completed/`. The data folder
      `planning/plan-35-data/` stays where it is, as plan-29's does.

Check: `ls planning/plan-35-*.md` → no matches;
`ls planning/completed/plan-35-*.md | wc -l` → 8.

Commit: —

## Validation Plan

§ 2 is the validation plan of all of plan-35.

## Decisions

- **One run of the tier, at the end** (author, write-plan rule): the
  earlier letters ran only checks scoped to their own work.

## Corrections

(none yet)

## Summary

A single, scripted pass/fail run over every area plan-35 touched, then
archive.
