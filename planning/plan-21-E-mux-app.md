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
| plan-21-D complete | `ls planning/completed/plan-21-D-*.md` → exists | MET: `planning/completed/plan-21-D-mux-parts.md` (D at 07af7a0, archived in E's commit) |

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

- [x] §3 edits (beside plan-19's `PART_DECODER` in all three); `xcodebuild … -derivedDataPath /tmp/p21-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **`. `grep -n PART_MUX` → `PartCatalog.swift:278` (`isLogic`), `PartFacts.swift:123` (`case PART_MUX: return "Multiplexer"`), `ChipDrawerView.swift:206` (drawer condition).
- [x] Runtime proof on the /tmp/p21-dd build (a second app instance, driven with AppleScript and plan-19's CGEvent drag helper): the drawer search `HC157` shows Logic (with the "All families" / "All counts" menus) → Multiplexer → CD74HC157E, CD74HC157M96, SN74AHC157…, SN74HC157DBR/DR/N/NSR/PWR, each "(4)" with e.g. "DIP · HC · 2–6 V"; `HC670` shows Logic → Register file → CD74HC670E ("DIP · HC · 2–6 V", (1)) and CD74HC670M96. SN74HC157N dragged onto the canvas draws a "Multiplexer" block with pins A/B, 1A…4B, G, 1Y…4Y; selected, the sidebar shows Kind Multiplexer, Function Multiplexer, Package DIP, Footprint PDIP-16, Pins 16, Family HC, Units 4, Supply range 2–6 V. Test instance quit, mouse returned to (0, 0).

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

- **Runtime proof by search, not the family menu**: the drawer was narrowed with its search field (`HC157`, `HC670`) rather than the family menu; the parts listed carry their family (`HC`, `AHC`) in each row and the Logic header shows the family menu, so the check is the same.
- **CD74HC670E is under its own leaf "Register file"** (the device's `function`), beside "Multiplexer" in Logic.

## Summary

E wires `mux` into three app switches and closes plan-21.
