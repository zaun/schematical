# plan-21-E: The app — the `mux` kind; plan-21 landing

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-21-D

E makes the app treat `mux` parts as logic chips (plan-10-J pattern), then
runs plan-21's landing tier.

References: `schematical/schematical/Backend/PartCatalog.swift` (`isLogic`,
`:267-270`), `Backend/PartFacts.swift` (`kindName`, ends `case PART_PIO`
at `:121`), `Drawer/ChipDrawerView.swift` (`categoryHeader`, `:203-205`);
`planning/completed/plan-10-J-app.md`.

## Prerequisites

plan-21-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-21-D complete | `ls planning/completed/plan-21-D-*.md` → exists | NOT MET (2026-11-07) |

## 1. Goal

- `** BUILD SUCCEEDED **`; `grep -n PART_MUX` in the three files → `isLogic`, `kindName` (`"Multiplexer"`), drawer condition.
- Landing tier passes.

### Non-goals

- No backend change; generic block symbol.

## 3. Design

`isLogic` `|| kind == PART_MUX`; `kindName` `case PART_MUX: return
"Multiplexer"`; `categoryHeader` adds `PART_MUX.rawValue`. Where another of
plans 19–22 edited these lines first, add beside theirs.

## Phases

### Phase 1 — App

- [ ] §3 edits; `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p21-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (about 1 min).
- [ ] Runtime proof (`run` skill): Logic drawer, family `HC` → `SN74HC157N` and `CD74HC670E` listed; a placed '157 shows "Kind: Multiplexer", "Family: HC", "Units: 4".

Acceptance: build, grep, runtime proof.
Commit: —

## Validation Plan — plan-21 landing tier

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p21 && cmake --build /tmp/logic-p21 -j8 && ctest --test-dir /tmp/logic-p21` | `100% tests passed`, T0 + 1 (about 13 s) |
| Goldens vs plan-21 start | `git diff <plan-21-A first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 |
| Part count | `grep -c '\|mux\|' backend/tests/golden/parts.txt` | N21 |
| Datasheets | `ls datasheets/mux/*.pdf \| wc -l` | A's count |
| App | `xcodebuild` above | `** BUILD SUCCEEDED **` |

Then archive plan-21-A…E to `planning/completed/`.

## Decisions

- No new drawer filter (plan-10-J precedent).

## Corrections

## Summary

E wires `mux` into three app switches and closes plan-21.
