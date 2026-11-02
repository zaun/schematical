# plan-10-J: The app — new kinds in the drawer and the sidebar; plan-10 landing

Last updated: 2026-11-01
Effort: medium (1h–2h)
Depends on: plan-10-I

J makes the app treat the three new kinds as logic chips: their kind names
in the sidebar, the drawer's logic-family and unit-count filters applied to
them, and the drawer leaves "Latch", "Counter", "Divider", "Shift register"
under Logic. Then it runs plan-10's landing tier.

Outcome of J: the app builds; filtering the Logic drawer by family (for
example "LS") lists LS counters, latches and shift registers with the gates;
the sidebar's Details show "Kind: Counter" (etc.), "Family", "Units"; the
plan-10 landing tier passes.

References: `schematical/schematical/Backend/PartCatalog.swift`,
`Backend/PartFacts.swift`, `Drawer/ChipDrawerView.swift`;
`planning/completed/plan-04-C-eeprom-app.md` (the last app sub-plan for a
new kind).

## Prerequisites

plan-10-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-10-I complete | `ls planning/completed/plan-10-I-shift-parts.md` → exists | MET (2026-11-01: exists, archived at 97d007d) |

## 1. Goal

- `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p10-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **`.
- `grep -n 'PART_LATCH\|PART_COUNTER\|PART_SHIFT' schematical/schematical/Backend/PartCatalog.swift schematical/schematical/Backend/PartFacts.swift` → the `isLogic` line and three `kindName` cases.
- The landing tier (§ Validation Plan) passes.

### Non-goals (explicit constraints)

- No backend change; no golden change.
- The canvas symbol stays the generic block (`symbol(for:)` default `.block`
  already draws any pin count).
- No new drawer filter: the existing family menu and count menu cover the new
  kinds.

## 2. Current State

- `CatalogPart.isLogic` (`PartCatalog.swift:207`) = gate, dff, jkff. It decides
  whether the family filter and the count filter apply to a part
  (`PartCatalog.swift:633-639`) and which parts `logicCounts(in:)` counts
  (`:579`).
- `CatalogPart.details` is `.logic(family:units:)` whenever the backend part
  has `logic` (`PartCatalog.swift`, `if let logic = part.logic?.pointee`), so
  the new kinds already carry family and units.
- `PartFacts.kindName` (`PartFacts.swift:67-80`) switches on the ten kinds;
  default `""` (the "Kind" row is then omitted).
- `ChipDrawerView.categoryHeader` shows the family and count menus for a
  category containing gates, dffs or jkffs (`ChipDrawerView.swift:186-189`);
  "Logic" contains gates, so the menus already show there.
- `symbol(for:)` (`PartCatalog.swift:491-502`) returns `.block` for every
  kind it does not name.

## 3. Design

- `isLogic`: `kind == PART_GATE || kind == PART_DFF || kind == PART_JKFF ||
  kind == PART_LATCH || kind == PART_COUNTER || kind == PART_SHIFT`; its doc
  comment says "a logic chip with a family and a unit count".
- `kindName`: `PART_LATCH` → "Latch", `PART_COUNTER` → "Counter",
  `PART_SHIFT` → "Shift register".
- `ChipDrawerView.categoryHeader`'s condition lists the three kinds too (so a
  future category of only counters still gets the menus).

## Phases

> **Status discipline:** as plan-10-A § Phases.

### Phase 1 — App

- [x] The three edits of §3.
- [x] `xcodebuild … build` → `** BUILD SUCCEEDED **` (about 1 min). (Ran:
  `** BUILD SUCCEEDED **`.)

Acceptance: the build succeeds and the grep of §1 matches. (Ran: the grep
prints `PartCatalog.swift:210` (the `isLogic` condition, now on its own line
of the wrapped property) and `PartFacts.swift:79-81` (the three
`kindName` cases).)
Commit: —

## Validation Plan — plan-10 landing tier

Run once, at the end of plan-10, with the targeted gates for everything it
touched (the loader, four kinds' models, the voltage check, the part files,
the footprints and the app):

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p10 && cmake --build /tmp/logic-p10 -j8 && ctest --test-dir /tmp/logic-p10` | `100% tests passed, 0 tests failed out of 15` (about 8 s) |
| Golden sizes | `wc -l < backend/tests/golden/parts.txt`, `traces.txt`, `footprints.txt` | 6108, 6108, 158 |
| Goldens vs the start of plan-10 | `git diff <plan-10-A's first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 (no existing line changed over the whole plan) |
| Part counts | `grep -c '\|latch\|' parts.txt`; `'\|counter\|'`; `'\|shift\|'` | 750, 730, 583 |
| Datasheets | `ls datasheets/{counters,divider,latch,shifters}/*.pdf \| wc -l`; `grep -c '\.pdf' datasheets/not-included.md` | 234, 10 |
| App | the `xcodebuild` above | `** BUILD SUCCEEDED **` |

Each check is the cheapest that would fail if its area were wrong: `ctest`
runs every model test and the golden comparisons in seconds; there is no
longer-running suite for this repo.

## Decisions

- No new drawer filter (§1 non-goals): family and unit count are what the
  existing menus offer, and the leaves already separate the four functions.

## Corrections

(none yet)

## Summary

J adds the three kinds to the app's logic handling and closes plan-10 with
its landing tier.
