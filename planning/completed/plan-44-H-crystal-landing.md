# plan-44-H: Landing tier

Last updated: 2026-11-16
Effort: small (< 1h)
Depends on: plan-44-G

H runs plan-44's landing tier once, on the landed tree on `main`, ticks the
todo items and archives the plan. Every gate's command is in the table; the
observed values are recorded under it.

References:

- plan-44-A § 1 (the plan's outcome), plan-44-E § 1, plan-44-F § 1,
  plan-44-G § 1.
- `planning/completed/plan-40-I-transistor-landing.md` (the precedent) and
  `planning/plan-42-J-regulator-landing.md`.

## Prerequisites

plan-44-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-44-G complete | `ls planning/completed/plan-44-G-*.md` → exists, or every G box ticked | MET (2026-11-22: `planning/completed/plan-44-G-crystal-app.md`) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

Every gate of Phase 1's table shows its expected value in one run.

### Non-goals (explicit constraints)

- No new work: a failing gate is fixed in the letter that owns the area (and
  recorded in that letter's § Corrections), then H re-runs.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Landing tier

- [x] Run every gate of the table on a fresh build dir (`/tmp/logic-p44-land`)
      and record the observed values here. Observed 2026-11-22 on the worktree branch
      after `main` was merged into it (907f9547; `main` had only gained plan-53's
      planning documents and four `todo.md` lines): see the table's last column.
- [x] `planning/todo.md`: mark `Add crystals and resonators (oscillators exist,
      crystals don't).` done (`[x]`) with a note on the same item: "(plan-44:
      Abracon, ECS, Suntsu; SAW, overtone and thermistor parts out of scope)",
      touching only that item. Rewrite `- [ ] crystal: 8 parts.` as `- [ ]
      crystal: <N> parts.` with N the number of the family's plan-43 parts the
      library still lacks (`awk -F'\t' '$1=="crystal"{print $3}'
      planning/plan-43-data/candidates.tsv`, each name looked up with `grep
      -c "^<name>|" backend/tests/golden/parts.txt`); N is at least 3 (the
      Citizen Finedevice, Seiko Instruments and Würth Elektronik parts are
      outside the top three), so the item stays unticked. Done: the item is `[x]`
      with the note; `crystal: 8 parts.` is now `crystal: 7 parts.` (only
      `ABM7-25.000MHZ-D2Y-T` of plan-43's eight is in the library; the other seven
      are `ABM10-166-12.000MHZ-T3`, `ABM8-25.000MHZ-D2Y-T`, `ABM8-272-T3`,
      `CM315D32768DZFT`, `ECS-.327-7-12-C-TR9`, `SC32S-7PF20PPM`, `830108206909`).
- [x] Move `plan-44-A` … `plan-44-H` to `planning/completed/` (A–G are
      archived as each completes; H with this commit).

| Gate | Command | Expected | Observed (2026-11-22) |
|---|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p44-land && cmake --build /tmp/logic-p44-land -j8 && ctest --test-dir /tmp/logic-p44-land` | `100% tests passed`, T0 + 1 programs (37 with T0 = 36), plus any program other plans added meanwhile (`git log --oneline <plan-44-E first commit>^.. -- backend/CMakeLists.txt \| grep -v plan-44`) | **`100% tests passed, 0 tests failed out of 41`** (T0 = 40 at f442d31a; no other plan touched `CMakeLists.txt`) |
| Goldens vs plan-44 start | `git diff <plan-44-E first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 (only added lines), counting only plan-44's commits | **0** (`git diff 070876da^ -- backend/tests/golden \| grep -c '^-[^-]'`) |
| Part counts | `grep -c '\|crystal\|' backend/tests/golden/parts.txt` | the `add` rows + 4 (`awk -F'\t' '$25=="add"' planning/plan-44-data/candidates.tsv \| wc -l`, plus the four generics) | **604** = 600 + 4 |
| Parts total | `wc -l < backend/tests/golden/parts.txt` | P0 (32897) + that count, plus any part other plans landed meanwhile (`git log --oneline <plan-44-F first commit>^.. -- schematical/schematical/Parts \| grep -v plan-44`) | **34375** = P1 33771 (f442d31a; other plans landed 874 parts since P0) + 604 |
| Footprints | `wc -l < backend/tests/golden/footprints.txt` | F0 (1646) + the objects of `footprints-C.json` and `footprints-D.json` (`python3 -I -c "import json;print(sum(len(json.load(open('planning/plan-44-data/footprints-%s.json'))['footprints']) for s in 'CD'))"`, with `%s` filled by each letter), plus any other plan's | **1895** = F1 1823 + 72 (the same python → `72`) |
| Datasheets | `find datasheets/crystal -iname '*.pdf' \| wc -l` | the distinct PDFs the `add` rows cite (`awk -F'\t' '$25=="add"{print $28}' planning/plan-44-data/candidates.tsv \| sort -u \| wc -l`) | **179** = **179** |
| Datasheet URLs | `node scripts/check-datasheets.js --host abracon.com`, `--host ecsxtal.com` | exits 0; `--host suntsu.com` → every Suntsu entry fails with `HTTP 403` (documented in `docs/parts-format.md` § Datasheets) | **`83 of 83 URLs serve a PDF; 0 failed`**, **`158 of 158 …; 0 failed`**; `suntsu.com`: `2 of 11 URLs serve a PDF; 9 failed` (every failure `HTTP 403`; the two that answer are not blocked: F Corrections 5) |
| Skipped parts listed | `python3 -I planning/plan-44-data/census/final.py --verify-skipped` | `<n> skip rows, 0 missing from skipped-parts.md` | **`34 skip rows, 0 missing from skipped-parts.md`** |
| Stock check | `python3 -I planning/plan-44-data/census/select.py planning/plan-44-data/raw /tmp/p44-final.tsv --urls planning/plan-44-data/url-check.tsv --stock planning/plan-44-data/stock-check.tsv --report` | exits 0; every `add` row of `candidates.tsv` has `stock_checked` 1 (`awk -F'\t' '$25=="add" && $31!=1' planning/plan-44-data/candidates.tsv \| wc -l` → 0) | **exit 0** (`7019 parts listed`, `download set: 744 rows; 191 distinct URLs`), **`0`** |
| App | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p44-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` | **`** BUILD SUCCEEDED **`** |
| App tests | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p44-dd test -only-testing:schematicalTests/CrystalTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/DatasheetCatalogTests -only-testing:schematicalTests/CapacitorTests -only-testing:schematicalTests/ResistorTests CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` | **`Executed 41 tests, with 0 failures`, `** TEST SUCCEEDED **`** (CrystalTests, DrawerPerformanceTests, PartEditorWindowTests, DatasheetCatalogTests, CapacitorTests, ResistorTests) |
| Runtime proof | `APP=/tmp/p44-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/crystal-app.swift /tmp/p44-harness && /tmp/p44-harness/H.app/Contents/MacOS/harness` | every line `ok`: the drawer groups and leaves, `Y1` on `HC-49/US`, the LED lit by the crystal's clock and dark on a pull-up | **nine `ok` lines and `all checks ok`** (`GENERIC-CRYSTAL-32768 on SN74HC04N 1A: the LED lights`, `PULLUP-5V … the LED stays dark`) |

Acceptance: every gate's observed value matches (the last column above).
  Check: the table's commands (est. 45 min: the fresh backend build, the app
  tests and the harness; this is the plan's one landing run, and nothing
  smaller covers the backend, the part files and the app together).
Commit: c09901c5

## Validation Plan

- The table above is the plan's landing tier; the full suites run per the
  project's background policy, not as a plan step.

## Decisions

- Author: **CapacitorTests and ResistorTests are in the app gate**: G changes
  `functionGroups` (the drawer's value-ordered leaves are the capacitors' and
  resistors'), which those kinds' tests exercise.

## Corrections

1. **The baselines moved:** the table's P0 / F0 / T0 are plan-44's start values at
   7108a13e (32897 / 1646 / 36); `main` gained 874 parts, 177 footprints and four
   test programs from other plans before this plan's first commit, so the observed
   values are against P1 / F1 / T0 = 33771 / 1823 / 40 (recorded in E and F).
2. **`main` was merged into the worktree branch before the run** (it had gained
   plan-53's planning documents only), and the tier ran once, after the merge.
3. **There is no Rust in this repository**, so § 5's `cargo fmt` passes of the
   follow-plan procedure do not apply.

## Summary

One run of every gate; no new work.
