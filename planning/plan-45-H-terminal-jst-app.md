# plan-45-H: The app — terminals and JST headers in the drawer, the sidebar, the Part Editor and the Board Preview

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-45-G

H makes the app show the new parts: a **Connector → Terminal block → <pitch>
mm** and **Connector → JST → <series>** branch in the parts drawer, with the
parts of a leaf in position order; the sidebar facts (type, pitch, clamp or
series, positions, rating); the kind name in the inspector ("Terminal block",
"JST connector"); the schematic symbol (a plain block of pins, as USB
connectors have); a 3-D look in the Board Preview window; the connector
fields in the Part Editor; and a harness that drives the real app. The
behavioral outcome: in the app, `GENERIC-TERMINAL-5.08-03` is found under
Connector → Terminal block → 5.08 mm → Generic, places as `J1` with three
pads 5.08 mm apart, its sidebar shows `Pitch 5.08 mm`, its Board Preview look
is a green terminal block, and a placed `B2B-XH-A` is a JST header.

References:

- plan-45-F § 1, plan-45-G § 3.2 (the keys, drawer, function, package of every
  part); `docs/parts-format.md` § `connector`.
- `planning/plan-44-G-crystal-app.md` and `planning/completed/plan-40-H-transistor-app.md`
  (the precedents for a kind's app letter, the harness and the tests).
- `schematical/schematical/Backend/PartCatalog.swift` (`PartDetails`, `CatalogPart`,
  `functionGroups`, the symbol), `Backend/PartFacts.swift`,
  `BoardPreview/PartLook.swift`, `Editor/ConnectorDraft.swift`,
  `scripts/part-editor-schema.py`, `scripts/harness/` (`generic-header.swift`,
  `build.sh`).

## Prerequisites

plan-45-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-45-G complete | `grep -c '^- \[ \]' planning/plan-45-G-*.md` → 0 (the archive move is the plan's last step, so `planning/completed/` is not the test while plan-45 runs in its integration worktree) | MET (2026-11-23 at 79c30358: 0 unticked boxes) |
| The library has the parts | `grep -c 'GENERIC-TERMINAL-5.08-03\|GENERIC-JST-XH-04' backend/tests/golden/parts.txt` → 2 | MET (2026-11-23 at 79c30358: 2) |
| No `wireConnector` case yet | `grep -c 'wireConnector' schematical/schematical/Backend/PartCatalog.swift` → 0 | MET (2026-11-23 at 79c30358: 0) |
| `PartConnector` has the new fields | `grep -c 'pitch_um' backend/include/parts.h` → 1 | MET (2026-11-23 at 79c30358: 3 lines mention it: the field and two comments) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `PartDetails` has a case `wireConnector(kind: "terminal" | "jst", variant:
  String, pitchUM: UInt32, ratedMV: UInt32, ratedMA: UInt32)` built from
  `part.connector` when its `terminal` or `jst` is set; barrel and USB parts
  keep `.connector(…)`.
- The drawer lists the new branches, the parts of a leaf by position count
  then name; the part's row shows its position count `(3)` as the generic
  header's does.
- The sidebar shows `Type`, `Pitch`, `Clamp` (terminal blocks) or `Series`
  (JST), `Positions`, `Rated voltage`, `Rated current` (when above 0).
- `kindName` is `Terminal block` or `JST connector`; the schematic symbol is
  `.block`; a chip's secondary labels are `5.08 mm`, `Screw`, `15 A` (terminal)
  or `XH`, `2.50 mm` (JST); `voltageLabel` is `<V> V max` when rated.
- The Board Preview window draws a terminal block (a green housing with a
  dark wire entry per pad) and a JST header (a natural-white housing with a
  polarization notch).
- The Part Editor's connector kind has `terminal`, `jst` and `pitch` fields.
- `TerminalConnectorTests` passes; `DrawerPerformanceTests`,
  `PartEditorWindowTests`, `BoardPreviewTests` and `HeaderBoardTests` stay
  green; the harness prints `ok` on every line.

### Non-goals (explicit constraints)

- No change to barrel or USB connectors' details, symbol, facts or look; no
  change to the reference prefix (`J`, `ReferenceCategory.of(PART_CONNECTOR)`
  stays `.femaleConnector`); no change to the file format; no simulation
  change (the parts are passive).
- No new Settings entry.

## 2. Current State

`PartDetails.connector(centerPinUM:ratedMV:ratedMA:contacts:usb:)` carries a
barrel jack (`usb` nil) or a USB connector; it is built at
`PartCatalog.swift:845` and matched on 9 lines of 3 files (`grep -rn
'\.connector(' schematical/schematical --include='*.swift' | wc -l` → 9:
`PartFacts.swift`, `PartCatalog.swift`, `PartLook.swift`). `kindName`
(`PartFacts.swift:223`) returns "USB connector" for a USB part and, for any
other connector, `kindName(PART_CONNECTOR)` = "Barrel connector". The symbol
(`PartCatalog.swift:1139`) is `.block` for USB and `.barrelJack` otherwise.
`PartLook.swift:85` picks `.usb(…)` or `.barrel`; its `height` and `draw`
switches cover each look. `CatalogPart.count` (`PartCatalog.swift:614`) is
the drawer's count (nil for connectors); `functionGroups` lists a leaf's
parts in natural name order. `ConnectorDraft.swift` is generated by
`scripts/part-editor-schema.py` from `connector.c` and `connector.json`.

### Measured populations

| What | Count | Command |
|---|---|---|
| `.connector(` matches in the app | 9 lines, 3 files | `grep -rn '\.connector(' schematical/schematical --include='*.swift' \| wc -l`; `… -l` |
| `PartDetails` switches the compiler lists after the new case | recorded in Phase 1 | `xcodebuild … build 2>&1 \| grep -c 'switch must be exhaustive'` |
| Part Editor kinds listing `connector` | 1 (`KindSchema.swift`, `PartKinds.all`) | `grep -c '"connector"' schematical/schematical/Editor/KindSchema.swift` → 1 after plan-46-D, which moved the display name into `Editor/KindInfo.swift` (`grep -c 'kind: "connector"' schematical/schematical/Editor/KindInfo.swift` → 1); 2 before it. plan-46 lands before plan-45's app work (owner, 2026-11-16); regenerating `ConnectorDraft.swift` with the script is unchanged by it |
| Harness scripts | 33 files in `scripts/harness/` | `ls scripts/harness \| wc -l` |

### Verified properties

- A new `PartDetails` case needs no change to the 9 existing matches: they
  match `.connector(…)`, which stays; the compiler lists any exhaustive
  switch that needs the new case.
- The drawer's order inside a leaf is `functionGroups`' `parts` as given
  (natural name order, the `functionGroups` comment at `PartCatalog.swift:1664`), so a position order
  is a sort in `functionGroups`, not in the catalog's name order.
- The drawer sorts group names with `localizedStandardCompare`
  (`PartCatalog.swift:1613`, `1632`, `1686`), which orders `"3.50 mm"`, `"3.81 mm"`,
  `"5.00 mm"`, `"5.08 mm"`, `"7.50 mm"`, `"7.62 mm"` numerically.

## 3. Design

- **Details.** `wireConnector` is a new case; `CatalogPart.details` is set
  from `part.connector.pointee` when `terminal != 0` (`variant` the clamp
  word: `screw`, `spring`, `push-in`, `pluggable` from `'s'`, `'c'`, `'p'`,
  `'h'`) or `jst != 0` (`variant` the series: the first letter mapped back
  to `XH`, `PH`, `EH`, `ZH`, `SH`, `GH`, `VH`), with `pitchUM`, `ratedMV` and
  `ratedMA`.
- **Facts and names.** `PartFacts`: the rows of § 1; clamp words `Screw`,
  `Spring-cage`, `Push-in`, `Plug-in header`. `kindName`: `Terminal block`,
  `JST connector`. Chip labels and `voltageLabel` as § 1.
- **Order in a leaf.** `functionGroups` sorts the parts of a leaf by pin count
  then name when every part of the leaf is a `wireConnector` (generic header
  and every other kind keep their order). `CatalogPart.count` is the pin
  count for a `wireConnector`, so the drawer's `(3)` shows.
- **Symbol.** `.block`, as USB.
- **Looks.** `PartLook.Look` gains `terminalBlock` and `jstHeader`; both draw
  on the footprint's pads like the other tall looks: `terminalBlock` a green
  (`Tint(0.17, 0.58, 0.32)`) box over the footprint's body with a dark rect
  (`Tint(0.08, 0.08, 0.09)`) per pad on the wire-entry side and height 9.5
  mm; `jstHeader` a natural-white (`Tint(0.93, 0.91, 0.84)`) box with a
  polarization notch at pin 1 and height 6 mm; `look(for:)` picks them from
  `.wireConnector`.
- **Part Editor.** `scripts/part-editor-schema.py` regenerates
  `ConnectorDraft.swift`: `terminal` (suggested `screw`, `spring`, `push-in`,
  `pluggable`), `jst` (suggested the seven series), `pitch` (number, mm).
- **Risk**: low. The only edits to existing code are new `switch` cases and
  one conditional in `functionGroups`; the old connectors' cases are
  untouched. `functionGroups` is also edited by plan-44-G (its crystal
  frequency order): the edits are different statements; apply on whichever
  state `main` has and re-run the drawer tests.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Details, facts, names, symbol, order

- [x] `Backend/PartCatalog.swift`: the `wireConnector` case and its doc
      comment; the construction at the `part.connector` branch (§ 3);
      `count`; the chip-label `switch`; `voltageLabel`; the symbol `switch`
      (`.block`); `functionGroups`' leaf order.
      Done, with `isWireConnector`; the leaf order reuses `byCountThenName` (the card slots' sort, plan-54-O) for a leaf whose parts are all card slots or wire connectors.
- [x] `Backend/PartFacts.swift`: the facts of § 1 and `kindName`; fix every
      exhaustive `switch` the build lists (record the number under
      § Corrections).
      `xcodebuild … build 2>&1 | grep -c 'switch must be exhaustive'` → `0`, `** BUILD SUCCEEDED **` (every existing `PartDetails` switch has a `default`); helpers `clampWord`, `clampName`, `jstSeries`.
- [x] `schematicalTests/TerminalConnectorTests.swift`: for `GENERIC-TERMINAL-5.08-03`,
      `GENERIC-JST-XH-04` and one Phoenix and one Würth part: details,
      `kindName`, facts rows, chip labels, `voltageLabel`, symbol `.block`,
      drawer path and leaf (`Connector › Terminal block › 5.08 mm › Generic`),
      the leaf's parts in position order (`-02` before `-03` before `-10`),
      the groups `3.50 mm` … `7.62 mm` in numeric order; a barrel jack and a
      USB connector unchanged.
      4 tests (`testExamples`, `testEveryWireConnectorPart` over all 589 parts, `testDrawerBranches`, `testOtherConnectorsAreUnchanged`) pass.

Acceptance: the catalog and sidebar show the new parts.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p45-dd test -only-testing:schematicalTests/TerminalConnectorTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min: app build and one test class). Result: `** TEST SUCCEEDED **`, 4 of 4 passed.
Commit: 834de11c

### Phase 2 — The Board Preview looks and the Part Editor

- [x] `BoardPreview/PartLook.swift`: `terminalBlock` and `jstHeader` in `Look`,
      `look(for:)`, `height`, and the draw `switch` (§ 3).
      The enum is `PartLook.Kind` (not `Look`) and the chooser `kind(of:)`; heights 9.5 and 6; a green housing with a dark entry over each pole, a natural-white housing with a pin-1 notch.
- [x] `schematicalTests/BoardPreviewTests.swift`: a placed terminal block and a
      JST header get the new looks; a USB connector still `.usb`.
      `testTerminalBlockAndJstLooks` (also: a barrel jack still `.barrel`; each look changes the front render).
- [x] `python3 scripts/part-editor-schema.py . connector` regenerates
      `Editor/ConnectorDraft.swift`; `PartEditorWindowTests` gains a check
      that the connector schema has `terminal`, `jst` and `pitch`.
      `connector: 567 devices, 1541 parts, variant None, 11 source keys, missing []`; new fields `jst` (the 7 series), `pitch` (`.number(unit: "mm")`), `terminal` (4 words); `testConnectorSchemaHasTheTerminalKeys`.

Acceptance: the preview and the editor know the new parts.
  Check: `xcodebuild … test -only-testing:schematicalTests/BoardPreviewTests -only-testing:schematicalTests/PartEditorWindowTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 10 min). Result: `** TEST SUCCEEDED **`.
Commit: f209d66c

### Phase 3 — The harness

- [x] `scripts/harness/terminal-app.swift` (from `generic-header.swift`): on
      the app's real catalog and document: the drawer branch and leaves
      exist; placing `GENERIC-TERMINAL-5.08-03` gives reference `J1` and
      three PCB pads 5.08 mm apart; placing `B2B-XH-A` gives a JST header;
      the sidebar facts of § 1; a 0.6 s play run with two of the terminal's
      pins on `5V` and `GND` nets runs without a short; renders a placed
      terminal block and a JST header's Board Preview to `/tmp`.
      Built on `card-slot-app.swift`, the nearer precedent. Output: 10 `ok` lines (drawer 3, placement 2, sidebar 2, looks 1, play run 1, images 1) and `all checks ok`, exit 0: `GENERIC-TERMINAL-5.08-03 is J1 on TB-WEI-OMNIMATE-LM-5.08-HT-03 ... three pads 5.08 mm apart`, `B2B-XH-A ... JST-XH-2.50-VT-02, two pads 2.50 mm apart`, pins 1 / 2 on 5V / GND at levels `1 / 0`, no wire shorted; the two Board Preview images were viewed (a green housing with three dark entries; a white one with a dark notch).
- [x] Tests: `DrawerPerformanceTests` and `HeaderBoardTests` stay green.
      18 tests in the four classes, 0 failures.

Acceptance: the app works end to end.
  Check: `rm -rf /tmp/p45-harness && APP=/tmp/p45-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/terminal-app.swift /tmp/p45-harness && /tmp/p45-harness/H.app/Contents/MacOS/harness` → every line `ok`, exit 0; and `xcodebuild … test -only-testing:schematicalTests/TerminalConnectorTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/HeaderBoardTests` → `** TEST SUCCEEDED **` (est. 20 min: app build, harness, three classes). Result: harness `all checks ok`, exit 0; `** TEST SUCCEEDED **`, `Executed 18 tests, with 0 failures`.
Commit: f274adb4

## Validation Plan

- Tests: `TerminalConnectorTests`, `BoardPreviewTests`, `PartEditorWindowTests`,
  `DrawerPerformanceTests`, `HeaderBoardTests`.
- Coverage check: `TerminalConnectorTests` loads the real catalog (the parts
  are in `Parts/connector.json`, which the catalog loads).
- Runtime proof: the harness drives the real app.
- Doc sync: none beyond G's (`docs/parts-format.md`, `docs/file-format.md`).
- Final gate: none; the landing tier is I's.

## Decisions

- Author: **A new `PartDetails` case, not more associated values on
  `.connector`**: nine existing matches stay untouched and a barrel or USB
  part cannot be mistaken for a terminal block.
- Author: **Position order inside a leaf**: a terminal block is found by its
  number of poles, so the leaf lists 2, 3, 4 … 12 (the generic header's
  precedent); only this kind's leaves change order.
- Author: **The reference prefix stays `J`** (the docs already say `J` for
  USB/barrel connectors and female headers; G adds terminal blocks and JST).
- Author: **Looks are simple boxes**: the library has no 3-D body data; a
  coloured housing with the wire entries is what a buyer recognises.

## Corrections

- Phase 1: no existing `PartDetails` switch was exhaustive (all have a
  `default`), so the compiler listed 0 switches (measured: `grep -c 'switch
  must be exhaustive'` → 0). The Type row reads `Terminal block` / `JST
  header`, the Kind row `Terminal block` / `JST connector`; the sidebar also
  keeps the generic `Pins` row beside `Positions`.
- Phase 3: the harness prints the plan's `B2B-XH-A` check as a JST header on
  `JST-XH-2.50-VT-02`; the generic 5.08 mm part's footprint is the
  representative's (a Weidmüller OMNIMATE LM 5.08/90 horizontal one), as G
  decided, so its Board Preview housing is the long side-entry body.
- Phase 2: regenerating `ConnectorDraft.swift` turned the `card` field into
  free text (`.text`): plan-54 put its distinct values at 27, over the
  generator's 24-value limit for a suggested list, and the existing
  `testConnectorSchemaHasTheCardSlotKeys` needs it suggested. The generator
  now always suggests `card` (`key == "card"`), and `pitch` has the unit
  `mm` in `UNITS`, so the diff of `ConnectorDraft.swift` is the card list
  (grown from 18 to 27 values), the three new fields and their renumbering.
- The Prerequisites row "plan-45-G complete" tested `planning/completed/`,
  which only fills when the whole plan lands; the command is now "no unticked
  box in plan-45-G", measured 0.

## Summary

H is the app's connector branch: a new details case, five `switch` cases, a
sort, two looks and a generated Part Editor schema. Risk is exhaustive
switches elsewhere in the app, which the compiler lists; the old connectors
are untouched.
