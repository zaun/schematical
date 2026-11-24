# plan-45-I: Landing tier

Last updated: 2026-11-16
Effort: small (< 1h)
Depends on: plan-45-H

I runs plan-45's landing tier once, on the landed tree on `main`, ticks the
todo items and archives the plan. Every gate's command is in the table; the
observed values are recorded under it.

References:

- plan-45-A § 1 (the plan's outcome), plan-45-F § 1, plan-45-G § 1,
  plan-45-H § 1.
- `planning/completed/plan-40-I-transistor-landing.md` (the precedent) and
  `planning/plan-44-H-crystal-landing.md`.

## Prerequisites

plan-45-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-45-H complete | `ls planning/completed/plan-45-H-*.md` → exists, or every H box ticked | MET (2026-11-24: every H box ticked: `grep -c '^- \[ \]' planning/plan-45-H-*.md` → 0) |

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

- [x] Run every gate of the table on a fresh build dir (`/tmp/logic-p45-land`)
      and record the observed values here.
      Observed (2026-11-24, tree at f274adb4 plus docs; `main` has not moved: base `ed9b9765` is its tip):
      Backend: fresh `cmake -S backend -B /tmp/logic-p45-land`, build, `ctest` → `100% tests passed, 0 tests failed out of 57`.
      Goldens vs start: `git diff main -- backend/tests/golden | grep -c '^-[^-]'` → `0`.
      Connector parts: `1541` (main 952) = 952 + 88 + 501 `add` rows.
      Parts total: `36510` (main 35921) = 35921 + 589.
      Footprints: `2773` (main 2293) = 2293 + 480 `TB-`/`JST-` ids.
      Generators: `node --test scripts/gen-terminals.test.js` → `tests 10, pass 10, fail 0`; `gen-terminals.js --check` → `ok: 480 TB-/JST- footprints`, exit 0; `gen-parts.py --check` → exit 0.
      Datasheets: `308` PDFs = `308` distinct PDFs of the `add` rows; `place.py --verify` → `ok: 308 files, 0 rows without a file, 0 unused, 0 changed`.
      Datasheet URLs: `datasheet.weidmueller.com` 149 of 149, `www.we-online.com` 219 of 219, `www.jst-mfg.com` 7 of 7 serve a PDF; `www.jst.com` 0 of 2 (`HTTP 403`, recorded); `www.phoenixcontact.com` 68 of 68 on G's first run, 20 of 68 on the landing run and 0 of 68 on the rerun after it (`HTTP 403`, `curl` too: the host throttles the script; recorded in § Datasheets).
      Skipped parts: `12 skip rows, 0 missing from skipped-parts.md`.
      Stock check: `select.py … --report` exit 0 (`candidates: 513; distinct URLs 333; variants 54`); `awk -F'\t' '$22=="add" && $27!=1' … | wc -l` → `0`.
      App: `** BUILD SUCCEEDED **`; app tests (six classes: TerminalConnectorTests, DrawerPerformanceTests, PartEditorWindowTests, BoardPreviewTests, HeaderBoardTests, DatasheetCatalogTests): `** TEST SUCCEEDED **`, every `Executed` line `0 failures`.
      Runtime proof: harness `all checks ok`, exit 0 (10 `ok` lines, a `J1` terminal block with three pads 5.08 mm apart, a JST header from `B2B-XH-A`, levels `1 / 0` in play mode with no short).
- [x] `planning/todo.md`: mark `Add screw terminals and JST-XH/PH connectors
      (check what the 222 existing connectors already cover first).` done
      (`[x]`) with a note on the same item: "(plan-45: Phoenix Contact,
      Weidmüller, Würth Elektronik terminal blocks; JST XH, PH, EH, ZH, SH,
      GH, VH headers; Reliance North America and the XH/PH-style headers of
      Pololu have no maker datasheet, so are out)", touching only that item.
      Rewrite `- [ ] connector (other than the two USB-C receptacles plan-43
      added): 22 parts.` as `- [ ] connector (…): <N> parts.` with N the number
      of the family's plan-43 `later` rows the library still lacks (`awk
      -F'\t' '$1=="connector" && $2=="later"{print $3}'
      planning/plan-43-data/candidates.tsv`, each name looked up with `grep -c
      "^<name>|" backend/tests/golden/parts.txt`); N is at most 15 (the seven
      JST headers among the 22 are plan-45 candidates), so the item stays
      unticked.
      The first item is `[x]` with the note; the family item reads `15 parts`: the 22 `later` rows minus the 7 JST headers the library now has (`later-connector` count: 22 rows, 15 still missing).
- [x] Move `plan-45-A` … `plan-45-I` to `planning/completed/` (A–H are
      archived as each completes; I with this commit).
      Done at the landing, with the two references to plan-45-A (`plan-51-B`, `plan-54-A`) following; `planning/plan-45-data/` stays beside the other `plan-NN-data` directories.

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p45-land && cmake --build /tmp/logic-p45-land -j8 && ctest --test-dir /tmp/logic-p45-land` | `100% tests passed`, T0 programs (36; no program is added), plus any program other plans added meanwhile (`git log --oneline <plan-45-F first commit>^.. -- backend/CMakeLists.txt \| grep -v plan-45`) |
| Goldens vs plan-45 start | `git diff <plan-45-F first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 (only added lines), counting only plan-45's commits |
| Connector parts | `grep -c '\|connector\|' backend/tests/golden/parts.txt` | 376 + 88 + the `add` rows (`awk -F'\t' '$22=="add"' planning/plan-45-data/candidates.tsv \| wc -l`), plus any connector other plans landed meanwhile (`git log --oneline <plan-45-G first commit>^.. -- schematical/schematical/Parts/connector.json \| grep -v plan-45`) |
| Parts total | `wc -l < backend/tests/golden/parts.txt` | P0 (32897) + 88 + the `add` rows, plus any part other plans landed meanwhile (`git log --oneline <plan-45-G first commit>^.. -- schematical/schematical/Parts \| grep -v plan-45`) |
| Footprints | `wc -l < backend/tests/golden/footprints.txt` | F0 (1646) + the `TB-` / `JST-` ids the parts name (`python3 -I -c "import json;print(sum(f['id'].startswith(('TB-','JST-')) for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"`), plus any other plan's |
| Generators | `node --test scripts/gen-terminals.test.js && node scripts/gen-terminals.js --check && python3 -I planning/plan-45-data/census/gen-parts.py --check` | all tests pass; both checks exit 0 (a second run changes nothing) |
| Datasheets | `find datasheets/connector-terminal datasheets/connector-jst -iname '*.pdf' \| wc -l` | the distinct PDFs the `add` rows cite (`awk -F'\t' '$22=="add"{print $24}' planning/plan-45-data/candidates.tsv \| sort -u \| wc -l`) |
| Datasheet URLs | `node scripts/check-datasheets.js --host www.jst-mfg.com`, `--host datasheet.weidmueller.com`, `--host www.we-online.com`, `--host www.phoenixcontact.com`, `--host www.jst.com` | the results G recorded in `docs/parts-format.md` § Datasheets (each host's entries pass, or fail with the recorded HTTP status) |
| Skipped parts listed | `python3 -I planning/plan-45-data/census/final.py --verify-skipped` | `<n> skip rows, 0 missing from skipped-parts.md` |
| Stock check | `python3 -I planning/plan-45-data/census/select.py planning/plan-45-data/raw /tmp/p45-final.tsv --urls planning/plan-45-data/url-check.tsv --stock planning/plan-45-data/stock-check.tsv --report` | exits 0; every `add` row of `candidates.tsv` has `stock_checked` 1 (`awk -F'\t' '$22=="add" && $27!=1' planning/plan-45-data/candidates.tsv \| wc -l` → 0) |
| App | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p45-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` |
| App tests | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p45-dd test -only-testing:schematicalTests/TerminalConnectorTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/BoardPreviewTests -only-testing:schematicalTests/HeaderBoardTests -only-testing:schematicalTests/DatasheetCatalogTests CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` |
| Runtime proof | `APP=/tmp/p45-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/terminal-app.swift /tmp/p45-harness && /tmp/p45-harness/H.app/Contents/MacOS/harness` | every line `ok`: the drawer branches and leaves, `J1` on `GENERIC-TERMINAL-5.08-03` with three pads 5.08 mm apart, a JST header from `B2B-XH-A`, the sidebar facts, no short in play mode |

Acceptance: every gate's observed value matches.
  Check: the table's commands (est. 45 min: the fresh backend build, the app
  tests and the harness; this is the plan's one landing run, and nothing
  smaller covers the backend, the part files, the generators and the app
  together). Result: every gate as observed above; the planning-time numbers in the table (376, P0 32897, F0 1646) were stale and are corrected in § Corrections.
Commit: e2dd4de3

## Validation Plan

- The table above is the plan's landing tier; the full suites run per the
  project's background policy, not as a plan step.

## Decisions

- Author: **`PartEditorWindowTests`, `BoardPreviewTests` and `HeaderBoardTests`
  are in the app gate**: H changes the connector schema, `PartLook` and the
  drawer's leaf order, which those tests exercise.

## Corrections

- The table's planning-time baselines were stale: the connector kind held
  952 parts on `main` (not 376), the parts golden 35921 lines (not 32897) and
  the footprints golden 2293 (not 1646), because other plans landed meanwhile;
  the gates are measured against `main`'s tip (`git show main:backend/tests/golden/...`),
  which is also plan-45's base, so the deltas are exactly 589 parts (88
  generics + 501 `add` rows) and 480 footprints.
- Landing: the main tree had another session's uncommitted edit to
  `planning/todo.md` (it deletes the "Examples (8-bit CPU)" section), so
  `git merge worktree-P-45` refused. The branch got a commit taking
  `todo.md` back to `main`'s version (`78bc8114`), merged cleanly
  (fast-forward, `main` had not moved), and the two plan-45 `todo.md` hunks
  were committed alone on `main` (`9b76c708`, via `git apply --cached`); the
  other session's deletion is still uncommitted in the main tree, untouched.
  The worktree and `worktree-P-45` were then removed.
- Plan A's Prerequisites rows are start-of-plan conditions ("no terminal / JST
  parts yet" and the like) and are consumed by the landing; the two that stay
  true (backend tests green, the app builds) were re-run in this letter's gates.
- `www.phoenixcontact.com` throttles the script (403) after a few runs; the
  host check is recorded as "serves all 68 on a first run".

## Summary

One run of every gate; no new work.
