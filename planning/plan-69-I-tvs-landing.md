# plan-69-I: Landing tier

Last updated: 2026-11-25
Effort: small (< 1h)
Depends on: plan-69-H

I runs plan-69's landing tier once, on the landed tree on `main`, records the
observed values and archives the plan. Every gate's command is in the table.

References:

- plan-69-A § 1 and § Prerequisites (T0 77, P0 = R0 = E0 40832, F0 3993),
  plan-69-E § 1, plan-69-F § 1, plan-69-G § 1, plan-69-H § 1.
- `planning/completed/plan-48-I-timer-opamp-landing.md` (the precedent).

## Prerequisites

plan-69-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-69-H complete | `ls planning/completed/plan-69-H-*.md` → exists, or every H box ticked | NOT MET (2026-11-25: H not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

Every gate of Phase 1's table shows its expected value in one run.

### Non-goals (explicit constraints)

- No new work: a failing gate is fixed in the letter that owns the area (and
  recorded in that letter's § Corrections), then I re-runs.
- `planning/todo.md` has no TVS item; I adds none and ticks none.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Landing tier

- [ ] Run every gate of the table on a fresh build dir (`/tmp/logic-p69-land`)
      and record the observed values here.
- [ ] Move `plan-69-A` … `plan-69-I` to `planning/completed/` (A–H are archived
      as each completes; I with this commit).

| Gate | Command | Expected |
|---|---|---|
| Size and coverage rules (plan-68; owner, 2026-11-25) | `node scripts/source-size.js --check && node scripts/coverage.js --check` | exit 0: no in-scope file over 1000 code lines and no uncovered line in the backend, app or scripts tiers (est. 30 min: the app tier is measured only by plan-68's full Debug-suite run, 1,634 s at plan-68's planning, and nothing smaller shows the new Swift lines run) |
| Backend tests | `cmake -S backend -B /tmp/logic-p69-land && cmake --build /tmp/logic-p69-land -j8 && ctest --test-dir /tmp/logic-p69-land` | `100% tests passed`, T0 + 1 programs (78 with T0 = 77), plus any program other plans added meanwhile (`git log --oneline <plan-69-E first commit>^.. -- backend/CMakeLists.txt \| grep -v plan-69`) |
| Goldens vs plan-69 start | `git diff <plan-69-E first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'`, counting only plan-69's commits | 0 (only added lines) |
| Part counts | `grep -c '\|tvs\|' backend/tests/golden/parts.txt` | the `add` rows (`awk -F'\t' 'NR>1 && $24=="add"' planning/plan-69-data/candidates.tsv \| wc -l`); at most 755 |
| Per-maker cap | `awk -F'\t' 'NR>1 && $24=="add"{n[$2" "$3]++} END{for(k in n)print k,n[k]}' planning/plan-69-data/candidates.tsv` | `zener littelfuse`, `zener vishay`, `zener yageo` each at most 200; `steering semtech`, `steering littelfuse`, `steering diodes`; no other company |
| ERC pins | `grep -c '\|tvs\|' backend/tests/golden/erc_pins.txt` | the same count as the parts, every pin passive (a line is `<part>\|<kind>\|<one letter per pin>`, `p` = passive, as the fuse lines `0001.1003.PT\|fuse\|pp`: `awk -F'\|' '$2=="tvs" && $3 !~ /^p+$/' backend/tests/golden/erc_pins.txt \| wc -l` → 0) |
| Footprints | `wc -l < backend/tests/golden/footprints.txt` | F1 (plan-69-F § Prerequisites) + the objects merged from `footprints-C.json` and `footprints-D.json`, plus any other plan's |
| Datasheets | `find datasheets/tvs -iname '*.pdf' \| wc -l` | the distinct PDFs the `add` rows cite (`awk -F'\t' 'NR>1 && $24=="add"{print $22}' planning/plan-69-data/candidates.tsv \| sort -u \| wc -l`) |
| Datasheet URLs | `node scripts/check-datasheets.js --host www.littelfuse.com`, `--host www.vishay.com`, `--host www.diodes.com`, and YAGEO's host | each exits 0 for plan-69's entries; the Semtech distribution URLs are the expected failures G recorded |
| Skipped parts listed | `python3 -I planning/plan-69-data/census/tvs-final.py --verify-skipped` | `<n> skip rows, 0 missing from skipped-parts.md` |
| Stock | `awk -F'\t' 'NR>1 && $24=="add" && !($6>0)' planning/plan-69-data/candidates.tsv \| wc -l` | 0: every `add` row had its own In Stock quantity above 0 |
| No excluded part | `awk -F'\t' 'NR>1 && $24=="add" && ($5!="Active" && $5!="Last Time Buy")' planning/plan-69-data/candidates.tsv \| wc -l`; `grep -c '"military"' schematical/schematical/Parts/tvs.json` | 0; 0 |
| App | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p69-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` |
| App tests | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p69-dd test -only-testing:schematicalTests/TvsTests -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/EasyEDAProExportTests -only-testing:schematicalTests/SchematicRotateTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/WizardStepsTests -only-testing:schematicalTests/KindGuideTests -only-testing:schematicalTests/KindInfoTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/DatasheetCatalogTests CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` |
| Runtime proof | `APP=/tmp/p69-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/tvs-app.swift /tmp/p69-harness && /tmp/p69-harness/H.app/Contents/MacOS/harness` | every line `ok`: the drawer groups, `D1`, the three symbols, the refused 12 V connection with its message, the accepted 5 V one, Run available |

Acceptance: every gate's observed value matches.
  Check: the table's commands (est. 45 min: the fresh backend build, the app build
  and its test classes, and the harness; this is the plan's one landing run, and
  nothing smaller covers the backend, the part files and the app together).
Commit:

## Validation Plan

- The table above is the plan's landing tier; the full suites run per the
  project's background policy, not as a plan step.

## Decisions

- Author: **`DrawerPerformanceTests` and `DatasheetCatalogTests` are in the app
  gate**: up to 755 new parts and their datasheet entries land in the drawer and
  the datasheet catalog.

## Corrections

## Summary

One run of every gate; no new work.
