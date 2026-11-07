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
| plan-19-C complete | `ls planning/completed/plan-19-C-*.md` → exists | MET (2026-11-07: C landed in ec27dfa, every box resolved; archived in D's commit) |

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

- [x] The three edits of §3. `grep -n 'PART_DECODER'` → `PartCatalog.swift:277` (`isLogic`), `PartFacts.swift:122` (`case PART_DECODER: return "Decoder"`), `ChipDrawerView.swift:206` (`categoryHeader`); no other of plans 19–22 had touched these lines.
- [x] `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p19-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (about 1 min). Output: `** BUILD SUCCEEDED **`.
- [x] Runtime proof: launch the app (`run` skill), open Logic in the drawer, choose family `LS` → `SN74LS138N` and `SN74LS148N` are listed with the gates; select a placed '138 → sidebar shows "Kind: Decoder", "Family: LS", "Units: 1". Done on the /tmp/p19-dd build, driven with AppleScript and a CGEvent drag helper (no project run skill): Logic shows the leaves "7-segment decoder", "Decoder", "Encoder" and FCT in the family menu; with LS chosen, Decoder lists SN74LS42D/N, SN74LS138D/DR/N/NSR, SN74LS139ADR … and Encoder SN74LS148DR, SN74LS148N ("DIP · LS · 4.75–5.25 V"); SN74LS138N dragged onto the canvas draws a "Decoder" block, and selected shows Kind Decoder, Function Decoder, Package DIP, Footprint PDIP-16, Family LS, Units 1, Supply range 4.75–5.25 V. Mouse returned to (0, 0) after.

Acceptance: build succeeds; grep of §1 matches; the runtime proof as stated. Met (all three).
Commit: — (recorded in the archive commit)

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

- **Landing tier results** (2026-11-07, before merging main): `ctest` →
  `100% tests passed, 0 tests failed out of 22`; `git diff c7d2ba0 --
  backend/tests/golden | grep -c '^-[^-]'` → 0 (c7d2ba0 = the plan-19 start,
  the parent of A's first commit 6ff7393); `grep -c '|decoder|' parts.txt`
  → 322 = N19 − n4511; datasheets → 89; app `** BUILD SUCCEEDED **`.
  Repeated after merging main (follow-plan §5).
- **No Rust formatter step**: the repo has no `Cargo.toml` (`ls Cargo.toml
  repository/Cargo.toml` → no such file), so follow-plan's `cargo fmt` does
  not apply.

## Summary

D wires the new kind into three app switches and closes plan-19.
