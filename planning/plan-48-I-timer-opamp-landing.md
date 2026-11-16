# plan-48-I: Landing tier

Last updated: 2026-11-16
Effort: small (< 1h)
Depends on: plan-48-H

I runs plan-48's landing tier once, on the landed tree on `main`, ticks the
todo item and archives the plan. Every gate's command is in the table; the
observed values are recorded under it.

References:

- plan-48-A § 1 and § Prerequisites (the baselines T0 37, P0 32897, R0
  32897, F0 1646), plan-48-E § 1, plan-48-F § 1, plan-48-G § 1, plan-48-H § 1.
- `planning/plan-44-H-crystal-landing.md`,
  `planning/completed/plan-40-I-transistor-landing.md` (the precedents).

## Prerequisites

plan-48-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-48-H complete | `ls planning/completed/plan-48-H-*.md` → exists, or every H box ticked | NOT MET (2026-11-16: not started) |

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

- [ ] Run every gate of the table on a fresh build dir
      (`/tmp/logic-p48-land`) and record the observed values here.
- [ ] `planning/todo.md`: mark `Add the 555 timer and common op-amps (LM358,
      TL07x…).` done (`[x]`) with a note on the same item: "(plan-48: Texas
      Instruments, Analog Devices, STMicroelectronics, Renesas; op-amps are
      placed and checked, not simulated; military parts and comparators out
      of scope)", touching only that item.
- [ ] Move `plan-48-A` … `plan-48-I` to `planning/completed/` (A–H are
      archived as each completes; I with this commit).

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p48-land && cmake --build /tmp/logic-p48-land -j8 && ctest --test-dir /tmp/logic-p48-land` | `100% tests passed`, T0 + 2 programs (39 with T0 = 37), plus any program other plans added meanwhile (`git log --oneline <plan-48-E first commit>^.. -- backend/CMakeLists.txt \| grep -v plan-48`) |
| Goldens vs plan-48 start | `git diff <plan-48-E first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 (only added lines), counting only plan-48's commits |
| Part counts | `grep -c '\|timer\|' backend/tests/golden/parts.txt`; `grep -c '\|opamp\|' backend/tests/golden/parts.txt` | the timer `add` rows + 1 and the op-amp `add` rows + 1 (`awk -F'\t' 'NR>1 && $22=="add"{n[$3]++} END{for(k in n)print k,n[k]}' planning/plan-48-data/candidates.tsv`); at most 63 and 601 |
| Per-maker cap | `awk -F'\t' 'NR>1 && $22=="add" && $3=="opamp"{n[$2]++} END{for(k in n)print k,n[k]}' planning/plan-48-data/candidates.tsv` | `ti`, `adi`, `st`, each at most 200; no other company |
| Parts total | `wc -l < backend/tests/golden/parts.txt` | P1 (plan-48-G § Prerequisites) + the two counts above, plus any part other plans landed meanwhile (`git log --oneline <plan-48-G first commit>^.. -- schematical/schematical/Parts \| grep -v plan-48`) |
| Footprints | `wc -l < backend/tests/golden/footprints.txt` | F1 + the objects of `footprints-C.json` and `footprints-D.json` that were merged (`python3 -I -c "import json;print(sum(len(json.load(open('planning/plan-48-data/footprints-%s.json' % s))['footprints']) for s in 'CD'))"`, less the ids G recorded as already present), plus any other plan's |
| Datasheets | `find datasheets/timer datasheets/opamp -iname '*.pdf' \| wc -l` | the distinct PDFs the `add` rows cite (`awk -F'\t' 'NR>1 && $22=="add"{print $25}' planning/plan-48-data/candidates.tsv \| sort -u \| wc -l`) |
| Datasheet URLs | `node scripts/check-datasheets.js --host www.ti.com`, `--host www.analog.com`, `--host www.st.com`, `--host www.renesas.com` | each exits 0 for plan-48's entries (a failure of an older entry is listed and left to its own plan) |
| Skipped parts listed | `python3 -I planning/plan-48-data/census/final.py --verify-skipped` | `<n> skip rows, 0 missing from skipped-parts.md` |
| Stock check | `awk -F'\t' 'NR>1 && $22=="add" && $27==""' planning/plan-48-data/candidates.tsv \| wc -l` | 0: every `add` row has its own stock-check date |
| No excluded part | `awk -F'\t' 'NR>1 && $22=="add" && ($6!="Active" && $6!="Last Time Buy")' planning/plan-48-data/candidates.tsv \| wc -l`; `grep -c '"military"\|"ceramic"' schematical/schematical/Parts/timer.json schematical/schematical/Parts/opamp.json` | 0; 0 and 0 |
| App | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p48-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` |
| App tests | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p48-dd test -only-testing:schematicalTests/TimerOpampTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/DatasheetCatalogTests -only-testing:schematicalTests/CapacitorTests -only-testing:schematicalTests/ResistorTests -only-testing:schematicalTests/PullResistorTests CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` |
| Runtime proof | `APP=/tmp/p48-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/timer-app.swift /tmp/p48-harness && /tmp/p48-harness/H.app/Contents/MacOS/harness` | every line `ok`: the drawer groups, `U1`, the LED blinking from the placed 1 kΩ / 10 kΩ / 100 nF, `Not timing` with its reason on the mis-wired circuit, Run on with an op-amp placed |

Acceptance: every gate's observed value matches.
  Check: the table's commands (est. 45 min: the fresh backend build, the app
  tests and the harness; this is the plan's one landing run, and nothing
  smaller covers the backend, the part files and the app together).
Commit: —

## Validation Plan

- The table above is the plan's landing tier; the full suites run per the
  project's background policy, not as a plan step.

## Decisions

- Author: **the resistor, pull-resistor and capacitor tests are in the app
  gate**: H changes the netlist those kinds' values travel in.

## Corrections

## Summary

One run of every gate; no new work.
