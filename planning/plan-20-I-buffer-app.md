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
| plan-20-H complete | `ls planning/completed/plan-20-H-*.md` → exists | NOT MET (2026-11-07) |

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

- [ ] §2 edits; `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p20-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (about 1 min).
- [ ] Runtime proof (`run` skill): Logic drawer, family `LVC` → `SN74LVC245ANSR` and `SN74LVC1T45DBVR` listed; a placed 1T45 shows the two supply rows; a placed `SN74HC244N` shows "Kind: Buffer" and one supply row.

Acceptance: build, grep, runtime proof.
Commit: —

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

## Summary

I wires `buffer` into the app, shows dual supplies, and closes plan-20.
