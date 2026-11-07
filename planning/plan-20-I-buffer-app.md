# plan-20-I: The app — the `buffer` kind and dual-supply facts; plan-20 landing

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-20-H

I makes the app treat `buffer` parts as logic chips (plan-10-J pattern),
shows both supply ranges for dual-supply parts in the sidebar's Details, then
runs plan-20's landing tier.

References: `PartCatalog.swift` (`isLogic`, `:267-270`), `PartFacts.swift`
(`kindName`, `:121`; the supply/voltage rows), `ChipDrawerView.swift`
(`:203-205`); `planning/completed/plan-10-J-app.md`, `plan-07-G-voltage-app.md`
(how the app shows `vcc`).

## Prerequisites

plan-20-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-20-H complete | `ls planning/completed/plan-20-H-*.md` → exists | MET (2026-11-07: `planning/completed/plan-20-H-registered-translator-parts.md`) |

## 1. Goal

- `** BUILD SUCCEEDED **`; `grep -n PART_BUFFER` in the three files → `isLogic`, `kindName` (`"Buffer"`), drawer condition.
- A selected SN74LVC1T45 shows "Supply A: 1.65–5.5 V" and "Supply B: 1.65–5.5 V"; a single-supply part's Details are unchanged.
- Landing tier passes.

### Non-goals

- No backend change; generic block symbol.

## 2. Design

- `isLogic` `|| kind == PART_BUFFER`; `kindName` `case PART_BUFFER: return
  "Buffer"`; `categoryHeader` adds `PART_BUFFER.rawValue`; beside any other
  plan-19–22 additions.
- `PartFacts`: where the supply row is built from `part.vcc`, a part with
  `domain_count == 2` shows two rows, "Supply A" (domain 0, its power pin
  name in parentheses) and "Supply B" (domain 1), each `min–max V`.

## Phases

### Phase 1 — App

- [x] §2 edits: `isLogic` `|| kind == PART_BUFFER` (`PartCatalog.swift`), `kindName` `case PART_BUFFER: return "Buffer"` (`PartFacts.swift`), `categoryHeader` `|| kinds.contains(PART_BUFFER.rawValue)` (`ChipDrawerView.swift`); `CatalogPart.supplyDomains` (power pin name and range per side) and the two Details rows "Supply A (VCCA)" / "Supply B (VCCB)" with "Input max A/B" (Corrections 1). `xcodebuild … build` → `** BUILD SUCCEEDED **`.
- [x] Runtime proof with the app's own code in the headless harness (`scripts/harness/buffer-app.swift`: the real `PartCatalogStore`, drawer filter, `SchematicDocument`, `PartFacts` and the rendered sidebar; Corrections 2): `ok   1 Logic drawer, family LVC: SN74LVC245ANSR and SN74LVC1T45DBVR listed`; `ok   2 buffer parts are logic: family and units`; `ok   3 SN74LVC1T45DBVR Details: Supply A (VCCA) and Supply B (VCCB) 1.65–5.5 V, no single row` (rows `Kind: Buffer`, `Function: Level shifter`, …, `Supply A (VCCA): 1.65–5.5 V`, `Input max A: 5.5 V`, `Supply B (VCCB): 1.65–5.5 V`, `Input max B: 5.5 V`); `ok   4 SN74HC244N Details: Kind Buffer, one Supply range row 2–6 V`; `ok   5 render the sidebar for both parts` (the 1T45 image shows the two supply rows).

Acceptance: build, grep, runtime proof.
Commit: (this commit; hash in the next)

## Validation Plan — plan-20 landing tier

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p20 && cmake --build /tmp/logic-p20 -j8 && ctest --test-dir /tmp/logic-p20` | `100% tests passed`, T0 + 1 (about 15 s) |
| Goldens vs plan-20 start | `git diff <plan-20-B first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 |
| Part counts | `grep -c '\|buffer\|' parts.txt`; `grep -c 'vccb=' parts.txt`; the `buf` rows in `gate.json` | Ntri+Nxcvr+Nreg+Ntrans; Ntrans; Nbuf |
| Datasheets | `ls datasheets/{buffer,transceiver,translator}/*.pdf \| wc -l` | A's count |
| App | `xcodebuild` above | `** BUILD SUCCEEDED **` |

Then archive plan-20-A…I to `planning/completed/`.

## Decisions

- No new drawer filter (plan-10-J precedent).

## Corrections

1. **Input max per side**: the single-supply Details show "Input max" beside "Supply range"; a dual-supply part shows it per side too ("Input max A", "Input max B"), since the B side's limit differs from A's on many translators.
2. **Runtime proof in the harness, not by driving the GUI**: `scripts/harness/` compiles the app's sources with the backend and runs the real drawer filter, document and Details code, and renders the sidebar offscreen — the same code the window runs, checkable without moving the user's mouse. Landing tier (2026-11-07, worktree at the I commit): `ctest` → `100% tests passed, 0 tests failed out of 22` (T0 + 1); `git diff e06894a^ -- backend/tests/golden | grep -c '^-[^-]'` → `0`; `grep -c '|buffer|' parts.txt` → `2135` (Ntri 1331 + Nxcvr 418 + Nreg 155 + Ntrans 231); `grep -c 'vccb=' parts.txt` → `231`; gate lines added since plan-20 start → `221` (Nbuf); `ls datasheets/{buffer,transceiver,translator}/*.pdf | wc -l` → `817` = distinct `pdf` of `add` rows; `xcodebuild` → `** BUILD SUCCEEDED **`.

## Summary

I wires `buffer` into the app, shows dual supplies, and closes plan-20.
