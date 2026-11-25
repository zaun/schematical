# plan-54-O: The app — card slots in the drawer, the sidebar, the Part Editor and the Board Preview

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-54-N

O makes the app show the card slots: a **Connector → Card slot → <leaf>** branch in the parts
drawer (`SD`, `microSD`, `SIM`, `Smart card`, `Memory module`, `Card edge`) with the parts of a
function group in position order; the sidebar facts (card, pitch, pins); the kind name in the
inspector ("Card slot"); the schematic symbol (a plain block of pins, as USB connectors have, two
columns for a pinout of more than 40 pins); a look in the Board Preview window; the card-slot
fields in the Part Editor; and a harness that drives the real app. The behavioral outcome: in the
app, the memory / SIM, module and card-edge examples of `planning/plan-54-data/examples.tsv` are
found in their drawer leaves, place as `J1` with the pads of their footprints, the sidebar shows
`Card` and (for module and card-edge sockets) `Pitch`, and the Board Preview draws a dark socket
housing over the pads.

References:

- plan-54-L § 3 (the keys, `PartConnector.card`, the drawer / function / package conventions),
  plan-54-M and N (the parts); `docs/parts-format.md` § `connector`.
- `planning/completed/plan-44-G-crystal-app.md` and `planning/completed/plan-40-H-transistor-app.md`
  (the precedents for a kind's app letter, the harness and the tests).
- `schematical/schematical/Backend/PartCatalog.swift` (`PartDetails`, `CatalogPart`,
  `functionGroups`, the symbol), `Backend/PartFacts.swift`, `BoardPreview/PartLook.swift`,
  `Editor/ConnectorDraft.swift`, `scripts/part-editor-schema.py`, `scripts/harness/`
  (`generic-header.swift`, `build.sh`).

## Prerequisites

plan-54-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-54-N complete | `ls planning/completed/plan-54-N-*.md` → exists | NOT MET (2026-11-23: no such file; N's parts are in `Parts/`, the parent session lands them in parallel, § Corrections 6) |
| The library has the parts | `grep -c '|connector|' backend/tests/golden/parts.txt` → 376 + M's + N's parts; `wc -l < planning/plan-54-data/examples.tsv` → 5 (header and four examples) | MET (2026-11-23: 951 connector lines; examples.tsv 5 lines: MSD-1-A, 7-1734774-7, 2309413-1, 2387405-2; at O's start it had 3 lines and the library 552, so the work was written against the rows present and re-run) |
| No `cardSlot` case yet | `grep -c 'cardSlot' schematical/schematical/Backend/PartCatalog.swift` → 0 | MET (2026-11-23 at the start of O: 0) |
| `PartConnector` has the new fields | `grep -c 'card_pitch_um' backend/include/parts.h` → 1 | MET (2026-11-23: 2, the member and its comment; L is done, § Corrections 6) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `PartDetails` has a case `cardSlot(card: String, pitchUM: UInt32)` built from `part.connector`
  when its `card` is not NULL; barrel and USB parts keep `.connector(…)`.
- The drawer lists the new branches; the parts of a function group are in pin-count order then name;
  a part's row shows its pin count `(7)` as the generic header's does.
- The sidebar shows `Card`, `Pitch` (when above 0) and `Pins`.
- `kindName` is `Card slot`; the schematic symbol is `.block`; a chip's secondary labels are the card
  text and, when above 0, `1.00 mm`.
- The Board Preview window draws a card slot (a dark housing box over the footprint's pads).
- The Part Editor's connector kind has `card` and `cardPitch` fields.
- `CardSlotTests` passes; `DrawerPerformanceTests`, `PartEditorWindowTests`, `BoardPreviewTests` and
  `HeaderBoardTests` stay green; the harness prints `ok` on every line.

### Non-goals (explicit constraints)

- No change to barrel or USB connectors' details, symbol, facts or look; no change to the reference
  prefix (`M`, `ReferenceCategory.of(PART_CONNECTOR)` stays `.femaleConnector`); no change to the
  file format; no simulation change (the parts are passive).
- No new Settings entry; no new schematic symbol (a compact card-edge symbol is rejected, § 3).

## 2. Current State

`PartDetails.connector(centerPinUM:ratedMV:ratedMA:contacts:usb:)` carries a barrel jack (`usb` nil)
or a USB connector; it is built at `PartCatalog.swift:1028` and matched on 9 lines of 3 files
(`grep -rn '\.connector(' schematical/schematical --include='*.swift' | wc -l` → 9: `PartFacts.swift`,
`PartCatalog.swift`, `PartLook.swift`). `kindName` (`PartFacts.swift:313`) returns "USB connector"
for a USB part. The symbol (`PartCatalog.swift:1347`) is `.block` for USB and `.barrelJack`
otherwise; the default block layout puts every `inout` pin in one column on the right unless the pin
has `"side": "in"` (`SchematicModel.swift`, `ChipLayout.init(pins:)`), each row 20 pt
(`ChipLayout.pitch`). `PartLook.swift:93` picks `.usb(…)` or `.barrel`. `CatalogPart.count`
(`PartCatalog.swift:767`) is the drawer's count (nil for connectors); `functionGroups` lists a
function group's parts in natural name order. `ConnectorDraft.swift` is generated by
`scripts/part-editor-schema.py` from `connector.c` and `connector.json`.

### Measured populations

| What | Count | Command |
|---|---|---|
| `.connector(` matches in the app | 9 lines, 3 files | `grep -rn '\.connector(' schematical/schematical --include='*.swift' \| wc -l`; `… -l` |
| `PartDetails` switches the compiler lists after the new case | recorded in Phase 1 | `xcodebuild … build 2>&1 \| grep -c 'switch must be exhaustive'` |
| Part Editor kinds listing `connector` | 1 | `grep -c 'kind: "connector"' schematical/schematical/Editor/KindInfo.swift` |
| Harness scripts | the files of `scripts/harness/` | `ls scripts/harness \| wc -l` |
| Pin counts of the card slots | 6 to 694 (a 694-pin block is 347 rows of 20 pt with two columns) | `awk -F'\t' '$20=="add"{print $8}' planning/plan-54-data/candidates.tsv \| sort -n \| tail -1` |
| DigiKey's most common heights, memory / SIM and module sockets | 3.00 mm (14 of 230 listed memory / SIM rows; 1.85 mm is next with 12) and 5.20 mm (31 of 163 listed module rows) | `python3 -I -c "import sys,collections;sys.path.insert(0,'planning/plan-54-data/census');import common;L=common.read_listing('planning/plan-54-data/raw');print(collections.Counter(r['params'].get('Height Above Board') for r in L if r['category']=='414').most_common(3), collections.Counter(r['params'].get('Height') for r in L if r['category']=='413').most_common(3))"` |

### Verified properties

- A new `PartDetails` case needs no change to the 9 existing matches: they match `.connector(…)`,
  which stays; the compiler lists any exhaustive switch that needs the new case.
- `ChipLayout` has no pin-count limit (the default init stacks rows); a block of 428 or 694 pins is
  tall but valid, and the pinout's `"side": "in"` on its first half (plan-54-L § 3.2) gives two columns.
- The drawer sorts group names with `localizedStandardCompare` (`PartCatalog.swift:1887`,
  `1906`, `1966`; `functionGroups(_:rank:)` is at `1945`), which orders `0.50 mm`, `0.60 mm`, `1.00 mm` numerically inside a function
  name's text.

## 3. Design

- **Details.** `cardSlot` is a new case; `CatalogPart.details` is set from `part.connector.pointee`
  when `card != nil` (`card` the C string, `pitchUM` from `card_pitch_um`); the branch sits before the
  existing `part.connector` branch.
- **Facts and names.** `PartFacts`: the rows of § 1; `kindName` `Card slot`. Chip labels: `card`, then
  `<pitch> mm` when `pitchUM > 0`.
- **Order in a function group.** `functionGroups` sorts the parts of a group by pin count then name when
  every part of the group is a `cardSlot` (other kinds keep their order). `CatalogPart.count` is the pin
  count for a `cardSlot`, so the drawer's `(7)` shows.
- **Symbol.** `.block`, as USB; no new symbol. Rejected: a compact connector symbol (rejected: no part
  needs it before the block's two-column layout is judged by use, and a new symbol is a larger change
  than this plan's request).
- **Look.** `PartLook.Look` gains `cardSlot`; it draws on the footprint's pad bounds like the other tall
  looks: a dark (`Tint(0.16, 0.17, 0.19)`) housing box, height 3.0 mm for a memory / SIM socket, 5.2 mm
  for a memory module and 6.0 mm for a card-edge socket (chosen from the part's drawer leaf); `look(for:)`
  picks it from `.cardSlot`.
- **Part Editor.** `scripts/part-editor-schema.py` regenerates `ConnectorDraft.swift`: `card` (text),
  `cardPitch` (number, mm).
- **Risk**: low. The only edits to existing code are new `switch` cases and one conditional in
  `functionGroups`; the old connectors' cases are untouched. Plan-45's app letter also adds a `PartDetails`
  case and edits `functionGroups`, `PartFacts` and `PartLook`: the edits are different statements; apply on
  whichever state `main` has and re-run the drawer tests.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Details, facts, names, symbol, order

- [x] Re-read the `PartDetails` enum and the five matching sites on the state `main` has
      (`git log --oneline -5 -- schematical/schematical/Backend`). Result: the enum is `CatalogPart.Details`
      (§ Corrections 1); the `.connector(` sites are the 9 lines the plan counts, at new line numbers (§ Corrections 2).
- [x] `Backend/PartCatalog.swift`: the `cardSlot` case and its doc comment; the construction (§ 3); `count`;
      the chip-label `switch`; the symbol `switch` (`.block`); `functionGroups`' group order.
      Result: the case `cardSlot(card:pitchUM:)`, the branch before `part.connector`, `count` = `pins.count`,
      `detailLabels`, `symbol(for:)`, `functionGroups` (a leaf whose parts are all `isCardSlot` goes by
      `byCountThenName`), and `CatalogPart.isCardSlot`; `xcodebuild … build` → `** BUILD SUCCEEDED **`.
- [x] `Backend/PartFacts.swift`: the facts of § 1 and `kindName`; fix every exhaustive `switch` the build
      lists (record the number under § Corrections). Result: `Card`, `Pitch` (above 0; `%.2f mm`) and `Kind` `Card slot`;
      the build listed 0 exhaustive switches (§ Corrections 3).
- [x] `schematicalTests/CardSlotTests.swift`: for the four parts of `examples.tsv`: details, `kindName`,
      facts rows, chip labels, symbol `.block`, drawer path and function group (`Connector › Card slot ›
      microSD › microSD`), the group's parts in pin-count order, a barrel jack and a USB connector
      unchanged; every `add` part of the library has `details` `.cardSlot` and a footprint with at least its
      pin count of pads. Result: six tests (the examples of `examples.tsv`, whatever rows it has; one library part
      per drawer leaf; every `cardSlot` part of `catalog.allParts`, 176 now; the drawer branch; the microSD path; the
      barrel and USB parts unchanged); `DrawerPerformanceTests.testPartsAreInNaturalOrder` learned the pin-count order
      (§ Corrections 7).

Acceptance: the catalog and sidebar show the new parts.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p54-dd test -only-testing:schematicalTests/CardSlotTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min: app build and one test class).
  Result: `** TEST SUCCEEDED **`, `Executed 6 tests, with 0 failures`; `CardSlotTests: leaves checked 7, none in the library yet: []`
  (the four rows of `examples.tsv` and the SD, SIM and Smart card leaves; `testEveryCardSlotPart` looped over the library's 575 card slots).
  The first run, before the parent landed the card-edge parts (library 552 connector lines, examples.tsv 3 lines), was `Executed 6 tests,
  with 1 test skipped and 0 failures` (`leaves checked 5, none in the library yet: ["Card edge"]`: the test skips only when a leaf has no
  library part).
Commit: 487329b1

### Phase 2 — The Board Preview look and the Part Editor

- [x] `BoardPreview/PartLook.swift`: `cardSlot` in `Look`, `look(for:)`, `height`, and the draw `switch` (§ 3).
      Result: `PartLook.Kind.cardSlot(height:)` (the type is `Kind`, § Corrections 1), picked in `kind(of:)`, heights from
      `PartLook.cardSlotHeight(of:)` by the drawer leaf, the housing a box over the pads' bounds (`Tint(0.16, 0.17, 0.19)`).
- [x] `schematicalTests/BoardPreviewTests.swift`: a placed card slot of each leaf kind gets the new look with the
      heights of § 3; a USB connector is still `.usb`. Result: `testCardSlotLook` (the examples, then one library part per
      other leaf; the look's one solid sits on the pads' bounds at the leaf's height) and `testCardSlotShowsInTheRender`.
- [x] `python3 scripts/part-editor-schema.py . connector` regenerates `Editor/ConnectorDraft.swift`;
      `PartEditorWindowTests` gains a check that the connector schema has `card` and `cardPitch`.
      Result: `connector: 356 devices, 552 parts, variant None, 7 source keys, missing []`; `card` is `.suggested([…18 values])`, `cardPitch`
      `.number(unit: "mm")` (`"cardPitch": "mm"` added to the script's `UNITS`, § Corrections 8);
      `testConnectorSchemaHasTheCardSlotKeys` also opens a library card slot as a draft with no key left over.

Acceptance: the preview and the editor know the new parts.
  Check: `xcodebuild … test -only-testing:schematicalTests/BoardPreviewTests -only-testing:schematicalTests/PartEditorWindowTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 10 min).
  Result: `** TEST SUCCEEDED **`; BoardPreviewTests `Executed 9 tests, with 0 failures`, PartEditorWindowTests `Executed 11 tests, with 0 failures`
  (with the card-edge parts in the library: `card slot looks checked for ["microSD", "Card edge", "Memory module", "Card edge", "SD", "SIM",
  "Smart card"], none in the library yet: []`). The first run, before they landed, was 20 tests with 1 skipped (`testCardSlotLook`, no `Card edge`
  part) and 0 failures. Run together with `DrawerPerformanceTests`, `HeaderBoardTests`, `PartKindKeysTests`, `KindInfoTests`:
  `Executed 44 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: 487329b1

### Phase 3 — The harness

- [x] `scripts/harness/card-slot-app.swift` (from `generic-header.swift`): on the app's real catalog and
      document: the drawer branch and its six leaves exist; placing each example gives reference `J1`
      and PCB pads at its footprint's positions; the sidebar facts of § 1; a 0.6 s play run with pins 1
      and 2 of the card-edge example on two driven nets shows neither shorted; placing the straddle-mount
      example on a 2-layer board puts one pad on Top and one on Bottom (`connectivity()`) and the design-rule
      check has no placement issue although its housing hangs over the board edge; renders a placed
      socket, a module socket and a card-edge socket to `/tmp`.
      Result: the harness reads `examples.tsv` (every row) and takes one library part for each leaf with no example; a leaf with no library
      part is printed as `ok … (skipped)`, none was skipped in the final run. `/tmp/p54-harness/H.app/Contents/MacOS/harness` → 31 `ok` lines, 0 `FAIL`,
      `all checks ok`, exit 0 (§ Corrections 9: the harness build needed `-module-name schematical`). Play run: pins 1 / 2 of `7-1734774-7` →
      levels `1 / 0`, wire styles `value(green)`, `normal`. Straddle: `2387405-2` → pad sides `[top, bottom]`, body minY -1.795 over the edge,
      pads minY 0.105, no placement issue. Images `/tmp/p54-socket.png`, `p54-module.png`, `p54-edge.png`, `p54-sidebar.png` written and looked at
      (a dark box over the pads).
- [x] `DrawerPerformanceTests`: the drawer builds in the same time class with the new parts
      (the test's existing bound). Result: the existing `testBiggestOpenGroupBuildsFast` → 0.36 s (bound 2.0 s) with the library at 951
      connector parts; a new `testCardSlotGroupBuildsFast` opens Connector › Card slot down to its biggest function group (`1.00 mm PCI Express`,
      137 parts) → 0.36 s; `testPartsAreInNaturalOrder` learned the pin-count order (§ Corrections 7).

Acceptance: the real app shows card slots end to end.
  Check: `rm -rf /tmp/p54-harness && APP=/tmp/p54-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/card-slot-app.swift /tmp/p54-harness && /tmp/p54-harness/H.app/Contents/MacOS/harness` → every line `ok`, exit 0; and `xcodebuild … test -only-testing:schematicalTests/CardSlotTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/PartEditorWindowTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 15 min: app build, harness, three test classes).
  Result: the harness (built with the check's `rm -rf … && APP=… build.sh … && …/harness`, run from the repository root): 31 `ok`, 0 `FAIL`,
  `all checks ok`, exit 0. The test command: `** TEST SUCCEEDED **`; `DrawerPerformanceTests` 7 tests, `CardSlotTests` 6, `PartEditorWindowTests` 11,
  `Executed 24 tests, with 0 failures`.
Commit: 487329b1

## Validation Plan

- Tests: `CardSlotTests`, `BoardPreviewTests`, `PartEditorWindowTests`, `DrawerPerformanceTests`,
  `HeaderBoardTests` (existing, must stay green), the harness.
- Coverage check: `CardSlotTests`' "every `add` part" test makes the new parts part of the suite's
  denominator, not only four examples.
- Runtime proof: the harness drives the real app: placement, the sidebar, a play run, the renders.
- Doc sync: `docs/parts-format.md` § `connector` (L) already describes the keys; the Part Editor's help text
  is generated from the schema.
- Final gate: none; the landing tier is P's.

## Decisions

- Author: **`.block` symbol, no new symbol** (§ 3): the USB connectors' precedent; a 694-pin block is
  tall but valid.
- Author: **One look, three heights by drawer leaf** (§ 3): the housing is the pad bounds, the height the
  family's common one.
- Owner (2026-11-22): no generic part: every test and the harness use library parts named in
  `examples.tsv`.

## Corrections

1. **Names.** The plan's `PartDetails` is `CatalogPart.Details` (`PartCatalog.swift`, `enum Details`; `grep -rn PartDetails schematical` → nothing);
   `part.details` is of that type. The look's `Look` is `PartLook.Kind` (`case cardSlot(height:)`), `look(for:)` is `PartLook.kind(of:chip:footprint:pads:)`.
   The plan's "chip-label `switch`" is `CatalogPart.detailLabels` (the drawer's second line after the package).
2. **Line numbers moved** (found with `grep -rn '\.connector(' schematical/schematical --include='*.swift'`, still 9 lines in 3 files): the
   `Details.connector` case is `PartCatalog.swift:530` (the new case follows it), its construction `:1130` (plan: 1028), the symbol switch `:1517`
   (plan: 1347), `detailLabels` `:891` and `voltageLabel` `:659`, `PartFacts.swift:138`, `:142` and `kindName` `:330` (plan: 313), `PartLook.swift:95` (plan:
   93), `functionGroups` `:2392` (plan: 1945), the drawer's `localizedStandardCompare` sorts in `subgroups` / `functionGroups` (plan: 1887, 1906, 1966).
3. **Exhaustive switches after the new case: 0.** `xcodebuild … build` → `** BUILD SUCCEEDED **`, `grep -c 'error:'` → 0: every `switch` over `details` has a
   `default` or is an `if case`; the plan's "record the number" is 0. `PartLook.Kind` has one exhaustive switch each in `height` and `solids`, both edited.
4. **Reference prefix.** The plan's non-goal says the prefix is `M`; `ReferencePrefixes.swift:55` gives `.femaleConnector` the prefix `J`, which is
   what the plan's behavior (`J1`) and the tests and harness use. Unchanged.
5. **`CatalogPart.count` is `pins.count`** (not the non-hidden pins as the Pins fact counts): a card slot has no hidden pin, and the sort calls `count`
   per comparison. `count` feeds only the logic and switch filters (guarded by `isLogic` / `PART_SWITCH`), so a card slot adds nothing to them.
6. **Prerequisites.** plan-54-N is not complete (no `planning/completed/plan-54-N-*.md`; the parent session was adding the card-edge parts in
   parallel). O was written against the rows `examples.tsv` has at run time (3 lines at the start, 5 at the end), never a count of four, and every
   test and the harness takes the library part of a drawer leaf with no example, skipping only when the library has none (it had none for `Card
   edge` at the first runs, then 575 card slots in six leaves). `grep -c 'card_pitch_um' backend/include/parts.h` is 2 (the plan says 1): the member and its comment.
7. **`DrawerPerformanceTests.testPartsAreInNaturalOrder`** asserted every leaf's parts in natural name order; a card-slot leaf now goes by pin count
   then name, so the test has a branch for it (as for the crystals).
8. **Part Editor units.** `scripts/part-editor-schema.py` gave `cardPitch` the unit `""` (no entry in `UNITS`); `"cardPitch": "mm"` was added to
   `UNITS`, and the generated field is `.number(unit: "mm")`. `card` is generated as `.suggested([…])` (a text field with a menu of the 18 shipped
   values), which accepts any text of the backend's 1 to 40 bytes.
9. **The harness build was broken.** `scripts/harness/build.sh` failed on `SchematicModel.swift:2240: cannot find 'schematical' in scope` (the app source
   qualifies `schematical.symbol(for:)`, and `swiftc` named the module `main`); no harness had been built since that line landed. `swiftc -module-name
   schematical` is the one-word fix; the harness now builds (0 errors).
10. **Pitch text.** The facts' `Pitch` and the second chip label use `%.2f mm` (`0.50 mm`, `1.00 mm`, as the plan's `1.00 mm`); a card edge's `Function` is
    `1.00 mm PCI Express` (plan-54-L § 3.2), so the drawer's function group and the label repeat the pitch there.
11. **Harness play run and straddle part.** The play run uses the first `examples.tsv` row in the `Card edge` leaf (`7-1734774-7`) and wires `5V.VCC` to its
    pin 1 and `GND.GND` to pin 2 (a pin found by designator or name); the straddle check uses the row whose family is `straddle` (`2387405-2`).
    Neither example existed in the library when O started; the harness falls back to a library part (the leaf's smallest, the smallest with opposite pads) and
    skips with an `ok … (skipped)` line only when the library has none.
12. **Stale library counts in four existing tests**, failing in the first full run because the library grew (the card-slot parts and footprints of
    plan-54-H to N), not because of O; each fixed with the observed number: `ParametricFootprintDraftTests` `entries.count` 1823 → 2198 (`("2198") is not
    equal to ("1823")`) and its parametric count 1210 → 1266 (`("1266") is not equal to ("1210")`; 2198 = 1266 + 932 explicit); `CustomFootprintDraftTests`
    explicit footprints 638 → 932 (`("932") is not equal to ("638")`); `PartKindRoundTripTests` connector parts 376 → 951 (`("Optional(951)") is not equal to
    ("Optional(376)") - connector`; all 951 round trip through the regenerated `ConnectorDraft` with no mismatch or extra key).
13. **Full run** (`xcodebuild … test -skip-testing:schematicalTests/RouterFixtureTests`, ~10 min; `RouterFixtureTests` run for minutes each in Debug and
    are not touched by O): 1450 passed, 4 skipped, 8 failed, `** TEST FAILED **`; none concerns card slots, the drawer, the Part Editor schema or the Board
    Preview: `CustomSupplyTests` ×4 (a 301 V rail is made: the supply limit is now 500 V), `KindGuideTests.testEveryModelHasABlurb` (crystal has no
    blurb), `MarkdownDatasheetTests.testConvertedPartHasMarkdownDatasheet`, `UserLibraryCatalogTests.testAddListsUnderUserLibraryAndWritesTheFile` and
    `UserPartLibraryTests.testWriteThenFilesReturnsTheFile` (`/var/…` against `/private/var/…` file URLs). `ParametricFootprintDraftTests` and
    `PourCanvasTests` passed in this run. These eight are unrelated to O and were not fixed. The Part Editor's kind blurb for `connector`
    ("Power jacks and USB sockets that bring power onto the board.", `KindInfo.swift`) still omits card slots; no test or task of O names it, so it is unchanged.

## Summary

O is a new `PartDetails` case with its facts, order and look, the editor fields and a harness; the only
edits to existing code are new `switch` cases and one conditional in `functionGroups`. The risk is the
drawer's size with a thousand more parts, which `DrawerPerformanceTests` bounds.
