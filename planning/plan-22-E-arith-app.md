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
| plan-22-D complete | `ls planning/completed/plan-22-D-*.md` → exists | NOT MET (2026-11-07) |

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

- [ ] §3 edits; `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p22-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (about 1 min).
- [ ] Runtime proof (`run` skill): Logic drawer, family `LS` → `SN74LS85N`, `SN74LS283N` listed; a placed '283 shows "Kind: Arithmetic", "Family: LS".

Acceptance: build, grep, runtime proof.
Commit: —

## Validation Plan — plan-22 landing tier

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p22 && cmake --build /tmp/logic-p22 -j8 && ctest --test-dir /tmp/logic-p22` | `100% tests passed`, T0 + 1 (about 13 s) |
| Goldens vs plan-22 start | `git diff <plan-22-A first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 |
| Part count | `grep -c '\|arith\|' backend/tests/golden/parts.txt` | N22 |
| Datasheets | `ls datasheets/comparator/*.pdf datasheets/arith/*.pdf datasheets/parity/*.pdf \| wc -l` | A's count |
| App | `xcodebuild` above | `** BUILD SUCCEEDED **` |

Then archive plan-22-A…E to `planning/completed/`.

## Decisions

- No state window: every plan-22 model is combinational (the ACT8832/8847,
  which would have needed one, are skipped for lack of a datasheet).

## Corrections

## Summary

E wires `arith` into the app and closes plan-22.
