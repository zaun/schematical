# plan-19-D: The app — the `decoder` kind; plan-19 landing

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-19-C

D makes the app treat `decoder` parts as logic chips (the plan-10-J
pattern): family and unit-count filters in the Logic drawer, the drawer
leaves "Decoder", "Encoder", "7-segment decoder", "Kind: Decoder" in the
sidebar. Then it runs plan-19's landing tier.

References: `schematical/schematical/Backend/PartCatalog.swift`
(`isLogic`, `:267-270`), `Backend/PartFacts.swift` (`kindName`, the
`switch` ending `case PART_PIO: return "Peripheral I/O"` at `:121`),
`Drawer/ChipDrawerView.swift` (`categoryHeader` condition, `:203-205`);
`planning/completed/plan-10-J-app.md`.

## Prerequisites

plan-19-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-19-C complete | `ls planning/completed/plan-19-C-*.md` → exists | NOT MET (2026-11-07) |

## 1. Goal

- `xcodebuild … build` → `** BUILD SUCCEEDED **`.
- `grep -n 'PART_DECODER' schematical/schematical/Backend/PartCatalog.swift schematical/schematical/Backend/PartFacts.swift schematical/schematical/Drawer/ChipDrawerView.swift` → the `isLogic` line, the `kindName` case, the drawer condition.
- The landing tier passes.

### Non-goals

- No backend change. The canvas symbol stays the generic block
  (`symbol(for:)` default `.block`).

## 3. Design

- `isLogic` adds `|| kind == PART_DECODER`.
- `kindName`: `case PART_DECODER: return "Decoder"`.
- `ChipDrawerView.categoryHeader`'s condition adds `PART_DECODER.rawValue`.
- If another of plans 19–22 changed the same three lines first, add this
  kind beside theirs (merge, not replace).

## Phases

### Phase 1 — App

- [ ] The three edits of §3.
- [ ] `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p19-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (about 1 min).
- [ ] Runtime proof: launch the app (`run` skill), open Logic in the drawer, choose family `LS` → `SN74LS138N` and `SN74LS148N` are listed with the gates; select a placed '138 → sidebar shows "Kind: Decoder", "Family: LS", "Units: 1".

Acceptance: build succeeds; grep of §1 matches; the runtime proof as stated.
Commit: —

## Validation Plan — plan-19 landing tier

Run once, after Phase 1:

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p19 && cmake --build /tmp/logic-p19 -j8 && ctest --test-dir /tmp/logic-p19` | `100% tests passed`, T0 + 1 tests (about 13 s) |
| Goldens vs plan-19 start | `git diff <plan-19-A first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 |
| Part counts | `grep -c '\|decoder\|' backend/tests/golden/parts.txt` | N19 − n4511 (A's numbers) |
| Datasheets | `ls datasheets/decoder/*.pdf datasheets/encoder/*.pdf \| wc -l` | A's count |
| App | the `xcodebuild` above | `** BUILD SUCCEEDED **` |

Every gate is the cheapest that covers its area; the repo has no longer suite.
Then archive plan-19-A…D to `planning/completed/`.

## Decisions

- No new drawer filter: family and unit count cover decoders (plan-10-J precedent).

## Corrections

## Summary

D wires the new kind into three app switches and closes plan-19.
