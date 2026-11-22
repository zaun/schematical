# plan-54-M: Landing tier

Last updated: 2026-11-22
Effort: small (< 1h)
Depends on: plan-54-L

M runs plan-54's landing tier once, on the landed tree on `main`, ticks the todo item and archives the
plan. Every gate's command is in the table; the observed values are recorded under it.

References:

- plan-54-A § 1 (the plan's outcome), plan-54-I § 1, plan-54-J § 1, plan-54-K § 1, plan-54-L § 1.
- `planning/completed/plan-44-H-crystal-landing.md` (the precedent) and
  `planning/completed/plan-40-I-transistor-landing.md`.

## Prerequisites

plan-54-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-54-L complete | `ls planning/completed/plan-54-L-*.md` → exists, or every L box ticked | NOT MET (2026-11-22: not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

Every gate of Phase 1's table shows its expected value in one run.

### Non-goals (explicit constraints)

- No new work: a failing gate is fixed in the letter that owns the area (and recorded in that letter's
  § Corrections), then M re-runs.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Landing tier

- [ ] Run every gate of the table on a fresh build dir (`/tmp/logic-p54-land`) and record the observed
      values in a last column.
- [ ] `planning/todo.md`: mark `Add card edge sockets` (line 66 at planning time) done (`[x]`) with a note
      on the same item: "(plan-54: memory / SIM, module and card-edge sockets from Same Sky, Amphenol ICC,
      GCT, TE Connectivity, Molex and Samtec; Active parts only)", touching only that item.
- [ ] Move `plan-54-A` … `plan-54-M` to `planning/completed/` (A – L are archived as each completes; M with
      this commit).

| Gate | Command | Expected |
|---|---|---|
| Backend, fresh build | `rm -rf /tmp/logic-p54-land && cmake -S backend -B /tmp/logic-p54-land && cmake --build /tmp/logic-p54-land -j8 && ctest --test-dir /tmp/logic-p54-land -j8` | `100% tests passed`, T0 programs (41 at planning; no plan-54 program), plus any program other plans added meanwhile (`git log --oneline <plan-54-I first commit>^.. -- backend/CMakeLists.txt \| grep -v plan-54`) |
| Goldens only grew | `git diff <plan-54-I first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 (only added lines), counting only plan-54's commits |
| Connector parts | `grep -c '\|connector\|' backend/tests/golden/parts.txt` | 376 + the `add` rows of all three families (`awk -F'\t' '$20=="add"' planning/plan-54-data/candidates.tsv \| wc -l`), plus any connector part other plans landed meanwhile |
| All parts | `wc -l < backend/tests/golden/parts.txt` | P1 (the value J measured at its start) + the `add` rows, plus any part other plans landed meanwhile (`git log --oneline <plan-54-J first commit>^.. -- schematical/schematical/Parts \| grep -v plan-54`) |
| Footprints | `grep -cE '^(SOCK\|DIMM\|EDGE)-' backend/tests/golden/footprints.txt` | the objects of `footprints-E.json` … `-H.json` the library uses (`python3 -I -c` over them and the parts' `footprint` ids), 0 `unused footprint` or `missing` lines from `footprint_check` |
| Datasheet PDFs and entries | `find datasheets/connector-card-socket datasheets/connector-memory-module datasheets/connector-card-edge -iname '*.pdf' \| wc -l`; `python3 -I -c "import json;print(len(json.load(open('schematical/schematical/Parts/datasheets.json'))['datasheets']))"` | the distinct files the three `datasheets.tsv` list (every PDF is used: `place.py --verify` → `0 missing, 0 unused`); 8558 + the new entries |
| Datasheet URLs | `node scripts/check-datasheets.js --host www.te.com --jobs 3`; the same for `suddendocs.samtec.com`, `www.cuidevices.com`, `www.sameskydevices.com`, `gct.co`, `www.molex.com`; then `--host cdn.amphenol-cs.com` | each of the first set exits 0 (`0 failed`); the Amphenol hosts' failures are `HTTP 403` or `429`, documented in `docs/parts-format.md` § Datasheets (plan-54-J) |
| Skips recorded | `python3 -I planning/plan-54-data/census/final.py --verify-skipped` | `<n> skip rows, 0 missing from skipped-parts.md` |
| Stock check complete | `python3 -I planning/plan-54-data/census/select.py planning/plan-54-data/raw /tmp/p54-final.tsv --urls planning/plan-54-data/url-check.tsv --stock planning/plan-54-data/stock-check.tsv --report \| tail -3`; `awk -F'\t' '$20=="add" && $25!=1' planning/plan-54-data/candidates.tsv \| wc -l` | exit 0; `0` |
| App build | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p54-dd CODE_SIGNING_ALLOWED=NO build \| grep -E "BUILD (SUCCEEDED\|FAILED)"` | `** BUILD SUCCEEDED **` |
| App tests | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p54-dd test -only-testing:schematicalTests/CardSlotTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/BoardPreviewTests -only-testing:schematicalTests/HeaderBoardTests -only-testing:schematicalTests/DatasheetCatalogTests CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` |
| Runtime proof | `rm -rf /tmp/p54-harness && APP=/tmp/p54-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/card-slot-app.swift /tmp/p54-harness && /tmp/p54-harness/H.app/Contents/MacOS/harness` | every line `ok`, `all checks ok` |

Acceptance: every gate's observed value matches (the last column).
  Check: the commands of the table (est. 45 min: a fresh backend build and `ctest` about 8 min, the datasheet URL checks about 10 min, `xcodebuild` build and tests about 25 min, the harness 2 min).
Commit: —

## Validation Plan

- Tests: the table's backend, goldens, app and harness gates.
- Coverage check: the table's part, footprint, datasheet and skip counts each equal the `add` rows of
  `candidates.tsv`: a green gate means the whole pick landed, not only the examples.
- Runtime proof: the harness.
- Doc sync: `docs/parts-format.md` § `connector` (plan-54-I) and § Datasheets (plan-54-J) updated;
  `datasheets/skipped-parts.md` § `Card slots (plan-54)` and `datasheets/obsolete-not-included.md` current;
  `planning/todo.md`'s card-edge item ticked.
- Final gate: this letter is the final gate.

## Decisions

- Author: **The landing tier runs once, here**, with the targeted gates of every area plan-54 touched
  (backend, goldens, datasheets, app, harness); no per-letter repeat.
- Author: the full tier (the whole suite with its sweeps) is not a plan step; it runs in the background per
  the project's test-tier policy (write-bug).

## Corrections

## Summary

M proves the whole pick landed: counts that equal the `add` rows, goldens that only grew, datasheet URLs that
answer (except the Amphenol hosts that refuse scripts), an app that builds, tests and a harness that drive the
real app.
