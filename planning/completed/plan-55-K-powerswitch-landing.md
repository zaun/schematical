# plan-55-K: Landing tier

Last updated: 2026-11-22
Effort: small (< 1h)
Depends on: plan-55-J

K runs plan-55's landing tier once, on the landed tree on `main`, records the todo item and archives
the plan. Every gate's command is in the table; the observed values are recorded under it.

References:

- plan-55-A § 1 (the plan's outcome), plan-55-H § 1, plan-55-I § 1, plan-55-J § 1.
- `planning/completed/plan-42-J-regulator-landing.md` (the precedent).

## Prerequisites

plan-55-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-55-J complete | `ls planning/completed/plan-55-J-*.md` → exists | MET (2026-11-23: archived at 38be3f10) |
| The app-test classes of the gate below pass on `main` before the plan (**added 2026-11-23, a Prerequisites defect: the gate's `** TEST SUCCEEDED **` rests on it**) | `xcodebuild … test -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/PartKindKeysTests CODE_SIGNING_ALLOWED=NO` on the commit before plan-55 (`0978bb26`) | **NOT MET** (2026-11-23: `PartEditorWindowTests.testKindNamesAreThePartFiles` and `PartKindKeysTests.testFormKeysCoverTheBackendKeys` are red at `0978bb26` and on the landed tree with the same messages; the Part Editor recipe for plan-56's `leddriver` / `lcddriver` kinds and the display-driver keys of `rtc` / `converter` is plan-56-B's Correction "moved to plan-56-G", an open plan of another stream; plan-55 does not touch those files) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every gate of the table passes on a fresh build.
- `planning/todo.md` records plan-55 and `plan-55-A` … `plan-55-K` are in `planning/completed/`.

### Non-goals (explicit constraints)

- No new work: a gate that fails is fixed in the letter that owns it (a Correction there), then the table is re-run.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> fill `Commit:` the moment a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Landing tier

- [x] Run every gate of the table on a fresh build dir (`/tmp/logic-p55-land`) and record the
      observed values under it (Correction 1).
- [x] `planning/todo.md`, section Parts: add `- [x] Add power switches: integrated ideal diodes, load
      switches, current-limited and USB switches (plan-55: top three makers per family, Active only; power
      muxes, external-FET controllers, eFuses and hot-swap controllers out of scope)`, touching only that
      item.
- [x] Move `plan-55-A` … `plan-55-K` to `planning/completed/` (A to J are archived as each completes; K
      with this commit).

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p55-land && cmake --build /tmp/logic-p55-land -j8 && ctest --test-dir /tmp/logic-p55-land` | `100% tests passed`, T0 + 1 programs (42 with T0 = 41), plus any program other plans added meanwhile (`git log --oneline <plan-55-A first commit>^.. -- backend/CMakeLists.txt \| grep -v plan-55`) |
| Goldens vs plan-55 start | `git diff <plan-55-A first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 (only added lines), counting only plan-55's commits |
| Part count | `grep -c '\|powerswitch\|' backend/tests/golden/parts.txt` | the `add` rows (`python3 -I planning/plan-55-data/census/verdicts.py --report` → the sum of the three families' `add`) |
| Footprints | `wc -l < backend/tests/golden/footprints.txt` | F0 (the value at H's start) plus the objects H and I merged |
| Datasheets | `find datasheets/powerswitch -iname '*.pdf' \| wc -l` | the `add pdfs` of `verdicts.py --report` |
| Datasheet URLs | `node scripts/check-datasheets.js <every datasheets.json id H and I added>` | exits 0 |
| Skipped parts listed | `python3 -I planning/plan-55-data/census/verdicts.py --verify-skipped` | `0 skip rows missing from skipped-parts.md` |
| Stock check | `python3 -I planning/plan-55-data/census/select.py planning/plan-55-data/raw planning/plan-55-data/candidates.tsv --stock planning/plan-55-data/stock-check.tsv --carry --report` | exits 0; every `add` row `stock_checked` 1 |
| App | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p55-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` |
| App tests | `xcodebuild … test -only-testing:schematicalTests/PowerSwitchTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/BillOfMaterialsTests -only-testing:schematicalTests/KindInfoTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/ParametricFootprintDraftTests -only-testing:schematicalTests/WizardStepsTests -only-testing:schematicalTests/RegulatorTests CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` |
| Runtime proof | `APP=/tmp/p55-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/powerswitch-app.swift /tmp/p55-harness && /tmp/p55-harness/H.app/Contents/MacOS/harness` | `all checks ok` |

**Prerequisites re-run on the landed tree** (plan-55-A's table and B to K's rows; `main` at `e2cb8c69`, 2026-11-23):

| Row | Observed |
|---|---|
| Backend tests green | fresh `/tmp/logic-p55-final`: `100% tests passed, 0 tests failed out of 45` — MET |
| Golden sizes | P0 / R0 / F0 now 35835 / 35835 / 2271 (main's growth and plan-55's +817 / +817 / +70) — recorded |
| The app builds | `xcodebuild … build` from the main checkout: `** BUILD SUCCEEDED **` — MET |
| Regulator kind landed | `plan-42-J` archived, `REGULATOR_KIND` 2 lines — MET |
| `powerswitch` kind present (the plan's own outcome) | `PART_POWERSWITCH` 1 line in `parts.h`, `Parts/powerswitch.json` and `datasheets/powerswitch` exist — MET |
| No other open plan adds these parts | 0 files — MET |
| DigiKey credentials and quota (B only) | credentials 2 of 2 and 2 of 2 (`ID2` / `SECRET2`); `--opt2` call → 645 calls left — MET |
| plan-55-A to J complete | all ten archived in `planning/completed/` — MET |
| **App-test classes pass on `main` before the plan** | **NOT MET** (the row above) |

So plan-55's own work meets every row and gate except the one that depends on plan-56-G's Part Editor work; K's `** TEST SUCCEEDED **` is reached when plan-56-G lands that work, and the two tests are the only red ones in the nine classes of the gate plus `CustomFootprintDraftTests`.

Acceptance: every gate's observed value matches.
  Check: the table's commands (est. 60 min: the fresh backend build, the nine app test classes and the
  harness; this is the plan's one landing run, and nothing smaller covers the backend, the part files
  and the app together).
Commit: 3ab4e220

## Validation Plan

- The table above is the plan's landing tier; the full suites run per the project's background
  policy, not as a plan step.

## Decisions

- Author: **RegulatorTests is in the app gate**: A adds a level class beside the regulated one in
  `voltage.c`, and the regulator's voltage tests exercise the same file.

## Corrections

1. **The observed values** (2026-11-23, on `worktree-P-55` merged with `main` at 0978bb26, the tree that lands):

   | Gate | Observed |
   |---|---|
   | Backend tests | fresh `/tmp/logic-p55-land`: `100% tests passed, 0 tests failed out of 45` (main's 44 programs + `powerswitch_test`; main's count grew by the plan-56 `display_out_test` and `lcd_test`) |
   | Goldens | `git diff main --numstat -- backend/tests/golden` → `70 0 footprints.txt`, `817 0 parts.txt`, `817 0 traces.txt`: only added lines |
   | Part count | `grep -c '\|powerswitch\|' backend/tests/golden/parts.txt` → 817 = 32 + 197 + 588 (`verdicts.py --report`) |
   | Footprints | `wc -l < footprints.txt` → 2271 = main's 2201 + the 70 objects H and I merged |
   | Datasheets | `find datasheets/powerswitch -iname '*.pdf' \| wc -l` → 225 = `add pdfs: 225` |
   | Datasheet URLs | `node scripts/check-datasheets.js <224 ids>` → `224 of 224 URLs serve a PDF; 0 failed`, exit 0 |
   | Skipped parts listed | `verdicts.py --verify-skipped` → `0 skip rows missing from skipped-parts.md` |
   | Stock check | `select.py … --carry candidates.tsv --urls … --stock …` (written to /tmp: the gate's command with its `--carry` argument and an output file) exits 0; the 817 `add` rows all have `stock_checked` 1 |
   | App | `xcodebuild … build` → `** BUILD SUCCEEDED **` |
   | App tests | the nine classes of the table plus `CustomFootprintDraftTests` (its explicit-footprint count moved, 932 → 944): every test passes except two that fail on `main` without plan-55 (Correction 2) |
   | Runtime proof | `powerswitch-app.swift` → 15 `ok` lines, `all checks ok` |

2. **Two app tests fail on `main` itself**, not on plan-55: `PartEditorWindowTests.testKindNamesAreThePartFiles` (the Parts folder has `lcddriver.json` and `leddriver.json`, plan-56's empty kinds, that `PartKinds.all` does not list: 41 files against 39 kinds) and `PartKindKeysTests.testFormKeysCoverTheBackendKeys` (the display-driver keys plan-56 put into `rtc.c` and `converter.c` are in no form yet). `main`'s own commit 13b2b163 records the first as its one failing gate. **Proved on the commit before the plan** (2026-11-23, a detached worktree at `0978bb26`, `main` before the merge; `xcodebuild test -only-testing:schematicalTests/PartEditorWindowTests/testKindNamesAreThePartFiles -only-testing:schematicalTests/PartKindKeysTests/testFormKeysCoverTheBackendKeys` → `** TEST FAILED **`, both tests failed: 40 part files against 38 kinds, and the same `rtc` / `converter` display-driver keys uncovered). So the table's `** TEST SUCCEEDED **` is unreachable for plan-55 alone: the gate is met as "every test passes that passed before the plan, and every test of the plan", which these two tests prove by running red before and after with the same messages. Neither touches a file of plan-55; the other seven tests of `PartEditorWindowTests` and `PartKindKeysTests` pass, and so does everything plan-55 changed. The plan-55-H and I check lists named the decoder and latch round-trip tests (red at the start of H); `main` has fixed their counts since.
3. **The merge with `main`** (`9a14d9e9`) took plan-56's three kinds beside `powerswitch` in `parts.h`, `kinds.c`, `listing.c`, `listing_check.c`, `trace_check.c`, `KindSchema.swift`, `BundledPartFile.swift`, `PartKindKeysTests.swift` and `part-editor-schema.py` (main's lines first, the power switch last: `PART_LCD, PART_LEDDRIVER, PART_LCDDRIVER, PART_POWERSWITCH`); `datasheets.json`, `footprints.json` and the goldens were taken from main and regenerated (`gen-parts.py` on all three families, the three `--print` outputs), so they hold main's lines plus plan-55's; the editor's count assertions were measured again (`68354f88`): kinds 39, footprint entries 2271 (1327 parametric + 944 explicit), wizard combinations 93 (23 + 70).
4. **The Rust formatter step of the follow-plan skill does not apply**: the repository has no Rust workspace here (`ls repository` is empty, no `Cargo.toml`).
5. **`planning/todo.md`** is dirty in the main checkout (the owner's own edits), so the plan-55 item is added there after the merge and committed alone (its one hunk), not through the worktree branch, to leave the owner's other edits uncommitted.

## Summary

One run of every gate; no new work.
