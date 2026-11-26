# plan-74-K: Landing tier

Last updated: 2026-11-25
Effort: small (< 1h)
Depends on: plan-74-J

K runs plan-74's landing tier once on the landed tree on `main`, records the observed values, and
archives the plan.

References:

- plan-74-A § 1 and § Prerequisites (T0 77, P0 = R0 40832, F0 3993, C0 1541), plan-74-E § 1,
  F § 1, G § 1, H § 1, I § 1, J § 1.
- `planning/completed/plan-48-I-timer-opamp-landing.md` (the precedent).

## Prerequisites

plan-74-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-74-J complete | `ls planning/completed/plan-74-J-*.md` → exists, or every J box ticked | NOT MET (2026-11-25: J not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

Every gate of Phase 1's table shows its expected value in one run.

### Non-goals (explicit constraints)

- No new work. A failing gate is fixed in the letter that owns the area (and recorded in that
  letter's § Corrections), and then K re-runs.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> fill `Commit:` the moment a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Landing tier

- [ ] Run every gate of the table on a fresh build dir (`/tmp/logic-p74-land`), and record the
      observed values here.
- [ ] Move `plan-74-A` … `plan-74-K` to `planning/completed/` (A–J as each completes; K with this
      commit).

| Gate | Command | Expected |
|---|---|---|
| Size and coverage rules (plan-68; owner, 2026-11-25) | `node scripts/source-size.js --check && node scripts/coverage.js --check` | exit 0: no in-scope file over 1000 code lines and no uncovered line in the backend, app or scripts tiers (est. 30 min: the app tier is measured only by plan-68's full Debug-suite run, 1,634 s at plan-68's planning, and nothing smaller shows the new Swift lines run) |
| Backend tests | `cmake -S backend -B /tmp/logic-p74-land -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p74-land -j8 && ctest --test-dir /tmp/logic-p74-land -j8` | `100% tests passed`; T0 + 1 programs (78 with T0 = 77), plus any program other plans added meanwhile (`git log --oneline <plan-74-E first commit>^.. -- backend/CMakeLists.txt \| grep -v plan-74`) |
| Goldens only grew | `git diff <plan-74-F first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` over plan-74's commits | 0 |
| IC parts | `grep -c '\|ethernet\|' backend/tests/golden/parts.txt` | the IC `add` rows (`awk -F'\t' 'NR>1 && ($1=="phy" \|\| $1=="controller") && $18=="add"' planning/plan-74-data/candidates.tsv \| wc -l`); at most 292 |
| Jack parts | `grep -c '\|connector\|.*rj45=' backend/tests/golden/parts.txt` | the `magjack` `add` rows; at most 505 |
| Per-maker cap | `awk -F'\t' 'NR>1 && $18=="add"{n[$1" "$2]++} END{for(k in n)print k,n[k]}' planning/plan-74-data/candidates.tsv` | every (family, maker) at most 200; makers only those of plan-74-A's table |
| Connector parts | `grep -c '\|connector\|' backend/tests/golden/parts.txt` | C0 (1541) + the jack rows, plus any connector other plans added |
| Stock check | `awk -F'\t' 'NR>1 && $18=="add" && ($4<=0 \|\| $21=="")' planning/plan-74-data/candidates.tsv \| wc -l` | 0 |
| Datasheets | `find datasheets/ethernet datasheets/connector-rj45 -name '*.pdf' \| wc -l` | the distinct `pdf` values of the `add` rows, + 1 (TE's 114-94447) |
| Datasheet URLs | `node scripts/check-datasheets.js --host ww1.microchip.com`, then `--host www.ti.com`, `www.analog.com`, `docs.wiznet.io`, `www.renesas.com`, `www.belfuse.com`, `yageogroup.com`, `www.te.com` | each passes for plan-74's entries. A failure of an older entry is listed and left to its own plan |
| Skipped parts listed | `python3 -I planning/plan-74-data/census/textscan.py --verify` | `0 skip missing from skipped-parts.md` |
| Generators idempotent | `python3 -I planning/plan-74-data/census/gen-ethernet.py --check && python3 -I planning/plan-74-data/census/gen-rj45.py --check` | exit 0 |
| App | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p74-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` |
| App tests | `xcodebuild … test` with `-only-testing:` `EthernetTests`, `PartKindRoundTripTests`, `PartKindKeysTests`, `KindGuideTests`, `WizardStepsTests`, `WizardModelTests`, `KindInfoTests`, `PartEditorWindowTests`, `DrawerPerformanceTests`, `DatasheetCatalogTests` | `** TEST SUCCEEDED **` |
| Runtime proof | `APP=/tmp/p74-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/ethernet-app.swift /tmp/p74-harness && /tmp/p74-harness/H.app/Contents/MacOS/harness` | `all checks ok` |

Acceptance: every gate's observed value matches.
  Check: the table's commands (est. 45 min: the fresh backend build, the app tests and the harness.
  This is the plan's one landing run, and nothing smaller covers the backend, the part files and the
  app together).
  Result:
Commit:

## Validation Plan

- The table above is the plan's landing tier. The full suites run per the project's background
  policy, not as a plan step.

## Decisions

- Author: **`DatasheetCatalogTests` is in the app gate**: F and G–I add datasheet entries (one per distinct `pdf` of the `add` rows).

## Corrections

## Summary

One run of every gate; no new work.
