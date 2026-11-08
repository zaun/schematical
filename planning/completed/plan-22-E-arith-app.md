# plan-22-E: The app — the `arith` kind; plan-22 landing

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-22-D

E makes the app treat `arith` parts as logic chips (plan-10-J pattern), then
runs plan-22's landing tier.

References: `PartCatalog.swift` (`isLogic`, `:267-270`), `PartFacts.swift`
(`kindName`, `:121`), `ChipDrawerView.swift` (`:203-205`);
`planning/completed/plan-10-J-app.md`.

## Prerequisites

plan-22-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-22-D complete | `ls planning/completed/plan-22-D-*.md` → exists | MET (2026-11-07: `planning/completed/plan-22-D-arith-parts.md`) |

## 1. Goal

- `** BUILD SUCCEEDED **`; `grep -n PART_ARITH` in the three files → `isLogic`, `kindName` (`"Arithmetic"`), drawer condition; landing tier passes.

### Non-goals

- No backend change; generic block symbol; no state window (the models are combinational).

## 3. Design

`isLogic` `|| kind == PART_ARITH`; `kindName` `case PART_ARITH: return
"Arithmetic"`; `categoryHeader` adds `PART_ARITH.rawValue`; beside any other
plan-19–22 additions already there.

## Phases

### Phase 1 — App

- [x] §3 edits; `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p22-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (about 1 min). `grep -n PART_ARITH`: `PartCatalog.swift:285` (`isLogic`), `PartFacts.swift:137` (`case PART_ARITH: return "Arithmetic"`), `ChipDrawerView.swift:208` (drawer family/count menus); `** BUILD SUCCEEDED **`.
- [x] Runtime proof (`run` skill): Logic drawer, family `LS` → `SN74LS85N`, `SN74LS283N` listed; a placed '283 shows "Kind: Arithmetic", "Family: LS". The app's headless harness (plan-20-I precedent), `scripts/harness/arith-app.swift` on the Debug build: `ok   1 Logic drawer, family LS: SN74LS85N and SN74LS283N listed`, `ok   2 arith parts are logic: family LS, 1 unit`, `ok   3 SN74LS283N Details: Kind Arithmetic, Family LS` (rows: Kind Arithmetic, Function Adder, Package DIP, Footprint PDIP-16, Pins 16, Family LS, Units 1, Supply range 4.75–5.25 V), `ok   4 render the sidebar` (`/tmp/p22/p22-sidebar-ls283.png`: the app's sidebar view shows Kind Arithmetic, Family LS).

Acceptance: build, grep, runtime proof. Met: each on its task line.
Commit: 72ea759

## Validation Plan — plan-22 landing tier

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p22 && cmake --build /tmp/logic-p22 -j8 && ctest --test-dir /tmp/logic-p22` | `100% tests passed`, T0 + 1 (about 13 s) |
| Goldens vs plan-22 start | `git diff <plan-22-A first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 |
| Part count | `grep -c '\|arith\|' backend/tests/golden/parts.txt` | N22 |
| Datasheets | `ls datasheets/comparator/*.pdf datasheets/arith/*.pdf datasheets/parity/*.pdf \| wc -l` | A's count |
| App | `xcodebuild` above | `** BUILD SUCCEEDED **` |

Then archive plan-22-A…E to `planning/completed/`.

Landing tier run 2026-11-07 on the `worktree-P-22` branch after merging
`main` (a801cf9, plan-27 planning) into it (merge 38b5755):

| Gate | Result |
|---|---|
| Backend tests (fresh `/tmp/logic-p22`) | `100% tests passed, 0 tests failed out of 25` (T0 + 1 = 25) |
| Goldens vs plan-22 start | `git diff ce855d9 -- backend/tests/golden \| grep -c '^-[^-]'` → 0 (base corrected, see Corrections) |
| Part count | `grep -c '\|arith\|' backend/tests/golden/parts.txt` → 107 = N22 |
| Datasheets | `ls datasheets/comparator/*.pdf datasheets/arith/*.pdf datasheets/parity/*.pdf \| wc -l` → 26 = A's count |
| App | `** BUILD SUCCEEDED **` |

plan-22-A…D are in `planning/completed/`; E joins them with this commit.

## Decisions

- No state window: every plan-22 model is combinational (the ACT8832/8847,
  which would have needed one, are skipped for lack of a datasheet).

## Corrections

- **The golden gate's base is ce855d9, not `<plan-22-A first commit>^`.**
  plan-22-A's first commit (07ba691) sits on 624159a; the worktree then
  merged `main` (76e4f46) before any plan-22 work, and main's own golden
  changes between those points remove 10 lines
  (`git diff 07ba691^ ce855d9 -- backend/tests/golden | grep -c '^-[^-]'` →
  10, and the gate as written also gives 10). Measured from the first
  plan-22 commit after that merge, ce855d9, plan-22 removes 0 lines.

- **Runtime proof through the app's harness**, not by driving the mouse:
  `scripts/harness/build.sh` compiles the app's own Swift (drawer filter,
  `PartFacts`, sidebar view) against the bundled part files, as plan-20-I
  did for buffers; the checks are the same ones the task names.

## Summary

E wires `arith` into the app and closes plan-22.
