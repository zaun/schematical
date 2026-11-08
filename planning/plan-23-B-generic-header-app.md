# plan-23-B: Generic header in the app: catalog, placed part, size swap, file and symbol

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-23-A

plan-23-A put 80 header parts (`GENERIC-HEADER-1X01` … `-2X40`, kind
`header`) and their vertical / right-angle footprints in the library. This
sub-plan makes them one generic header in the app:

- The drawer shows a single **GENERIC-HEADER** row under Connector ▸ Header.
  Dropping it places a 1×2 male vertical header.
- A placed header has a gender (Male/Female) and a footprint choice, saved in
  the file, and is drawn with a header symbol: pins for male, sockets for
  female.
- The document can change a header's size, which swaps the placed part for
  the part of the new size. Wires on pins that no longer exist are removed,
  the rest follow their pins, and pad 1 stays where it was on the PCB.

plan-23-C adds the sidebar controls that call these setters, and the
Settings defaults.

Outcome: a harness places the header, resizes it from 1×8 to 2×4 and back
down to 1×2, and changes its gender and footprint. Every check passes:
wires on pins 3–8 are gone, pad 1 has not moved, and the saved file reopens
equal.

References:

- plan-23-A (the parts and footprints), `planning/completed/plan-18-A-generic-led-data.md`
  and `-B-generic-led-ui.md` (the per-part option pattern: `PlacedChip.color`,
  `footprintID`, `setPartFootprint`'s pad-1 anchor, `badPartOption`)
- `docs/file-format.md` § Parts
- `schematical/schematical/Backend/PartCatalog.swift` (`CatalogPart`,
  `Details`, `BackendCatalog.load`, `PartCatalogStore.entries`, `symbol(for:)`)
- `schematical/schematical/Canvas/SchematicModel.swift` (`PartSymbol` :94,
  `ChipLayout.init(part:symbol:)` :188, `PlacedChip` :492, `placeChip`,
  `setPartFootprint` :1055, `flipChip` :1093, `pruneJunctions` :1654,
  `settleWires(attachedTo:)` :1685)
- `schematical/schematical/Canvas/SchematicCanvasView.swift:211-235` (symbol → view)
- `schematical/schematical/Document/SchematicFile.swift` (`Part`, `apply`)
- `schematical/schematical/Drawer/ChipDrawerView.swift:318-329` (row title, drag item)

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-23-A complete | `ls planning/completed/plan-23-A-*` → 1 file | MET (2026-11-07: `planning/completed/plan-23-A-generic-header-data.md`) |
| The header parts load | `grep -c '"name":"GENERIC-HEADER-' schematical/schematical/Parts/header.json` → 80 | MET (2026-11-07: 80) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- The drawer lists one header row, titled `GENERIC-HEADER`, and dropping it
  places `GENERIC-HEADER-1X02`, Male, `PINHDR-1X02-V`.
- `SchematicDocument.setHeaderSize(id, to: part)`,
  `setPartGender(id, _:)` and the existing `setPartFootprint(id, _:)`
  change a placed header. A size change keeps the reference, the schematic
  origin, the gender, the footprint type (vertical or right angle), the
  board rotation, origin and marks, and pad 1's board point.
- A file saves each header's `gender` and `footprint` and reopens the same.

### Non-goals (explicit constraints)

- No sidebar controls and no Settings (plan-23-C).
- No backend, part data or footprint changes (plan-23-A owns them).
- File format stays version 4. `gender` is a new optional part field, and a
  file with headers names parts an older build doesn't have, so that build
  refuses the file with its existing unknown-parts message anyway
  (`docs/file-format.md` § Changing the format: an optional field keeps the
  version).
- New headers get a `U<n>` reference like every other part. A `J` prefix is
  not part of this plan; the sidebar's Reference field renames it.
- Simulation does not change: a header is a passive part, and its options
  don't feed the netlist.
- No undo (the app has none).

## 2. Current State

- **Per-part options (plan-18).** `PlacedChip` has `color` and `footprintID`,
  normalized in `init` (`SchematicModel.swift:492-560`). `footprintChoice`
  and the board `footprint` (`BoardModel.swift:95`) follow the choice.
  `setPartFootprint` (:1055) swaps the footprint and moves `board.centre` so
  pad 1 keeps its board point. `SchematicFile.Part` has `color` and
  `footprint`, and `apply` checks them, failing with
  `SchematicFileError.badPartOption`.
- **The part is fixed on a placed chip.** `PlacedChip.part` is a `let`
  (:498), and `baseLayout` is a `private let` built in `init` from the part
  and symbol (:520, :531). A size change therefore builds a new `PlacedChip`
  with the same id.
- **Wire ends.** A wire end on a pin is `PinRef(chipID:index:)`, where index
  is the pin's position in its pinout (:630-634). Header pinouts list pins
  1…N in order (plan-23-A § Data), so pin n is index n − 1 in every size,
  and a wire on pin n stays valid in any size with at least n pins.
- **Re-fitting wires.** `flipChip` (:1093) changes the layout and calls
  `settleWires(attachedTo:)` (:1685), which re-routes the wires on that
  chip. `erase(at:)` (:2153) removes a part's wires with `wires.removeAll`.
  `pruneJunctions` (:1654) drops junctions no wire ends at, then calls
  `pruneSelection` (:954), which drops selected ids that are gone.
- **Symbols.** `symbol(for:)` (`PartCatalog.swift:629`) maps kind to
  `PartSymbol`. `ChipLayout.init(part:symbol:)` (:188) builds the layout,
  for example `barrelJack(_:)` (:330), and `SchematicCanvasView`
  (:211-235) picks the view per symbol (`BarrelJackSymbolView` is the
  closest precedent). Layout constants: `pitch` 20 (two grid cells),
  `stubLength` 10, `labelMargin` 30 (:153-160).
- **Drawer.** `PartCatalogStore.entries` is every part with a drawer path
  (`PartCatalog.swift:868`), so the 80 header parts would show as 80 rows.
  The row title is `part.name`, and its drag item carries `part.name`
  (`ChipDrawerView.swift:318, 329`). The drop places
  `catalog.part(named: item.partNumber)` (`SchematicCanvasView.swift:145-157`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Header parts / sizes per row count | 80 / 40 | plan-23-A § Goal |
| Symbol → view switch sites | 2 | `grep -n 'case .barrelJack' schematical/schematical -r` → `SchematicModel.swift:193`, `SchematicCanvasView.swift:220` |
| `kind` switches that list kinds by name | 2 | `symbol(for:)` (`PartCatalog.swift:629`), `PartFacts.kindName` (`:103`, done in A) |

### Verified properties

- **A smaller size keeps every lower wire valid.** Pin index n − 1 is pin
  n in every header pinout (generated 1…N in order, plan-23-A), and
  `PinRef` stores the index. So swapping the part keeps a wire on pin n
  attached to pin n. (Read `PinRef` and plan-23-A's generator spec.)
- **The pad-1 anchor works across sizes.** Every `PINHDR-` footprint has pad
  `1` (plan-23-A's `header` style numbers pads 1…pins), so the anchor in
  `setPartFootprint` always finds both pads.
- **Removed wires leave the selection.** `pruneJunctions` → `pruneSelection`
  drops ids no longer in `wires` (read :954-970, :1654-1659).

## 3. Design Overview

**Correctness risk** is in the size swap: wires on removed pins, the pad-1
anchor, and keeping the footprint type when the footprint id changes with
the size. Phase 2 tests all three in the harness before any UI exists.

**Gate class:** behavior change; harness checks, not byte-identity.

### Catalog (`PartCatalog.swift`)

- `Details.header(rows: Int, perRow: Int, genders: [String])`, from
  `part.header` (rows, genders) and `pin_count / rows`.
- `enum HeaderFamily` (new, `Backend/HeaderFamily.swift`):
  `static let title = "GENERIC-HEADER"`,
  `static let defaultPart = "GENERIC-HEADER-1X02"`, `static let maxPerRow = 40`,
  `static func name(rows: Int, perRow: Int) -> String` (`"GENERIC-HEADER-2X08"`).
- `PartCatalogStore.entries` drops every header part except
  `HeaderFamily.defaultPart`, so the drawer has one header row. `part(named:)`
  still finds all 80.
- Drawer row (`ChipDrawerView`): a header part's title is
  `HeaderFamily.title` and its subtitle is
  `"1 or 2 rows · 1–40 pins per row · 2 footprints"`. The drag item still
  carries the part's own name (`GENERIC-HEADER-1X02`).
- `symbol(for:)`: `PART_HEADER` → `.header`.
- `detailLabels` for a header: `["\(rows)×\(perRow)"]`.

### Placed part

- `PlacedChip.gender: String?`: for a header, a nil or unknown value becomes
  `genders[0]` (Male); nil for other parts. `init` takes `gender:` (default
  nil), as it takes `color:`.
- `PlacedChip.resized(to part: CatalogPart) -> PlacedChip`: a new chip with
  the same id, reference, symbol, origin, isFlipped, board, gender and
  memory file. Its `footprintID` is the new part's choice whose `package`
  equals the old choice's (`Straight` stays `Straight`).
- `ChipLayout.header(_ pins:, rows:)`: body width 40, height perRow × 20.
  Pin of column c (0-based) has `centerY` 10 + 20c. With 1 row, every pin
  is on `.left`. With 2 rows, odd pins are on `.left` and even pins on
  `.right`, side by side in the same column. Flip swaps sides as for blocks.
- `HeaderSymbolView` (new, `Canvas/HeaderSymbolView.swift`): the body
  rectangle, then for each pin at its row a male pin (filled 6×6 square at
  the stub end) or a female socket (open 8-wide U), and the pin number
  beside it inside the body. Under the body, the label
  `"\(rows)×\(perRow) \(gender.lowercased())"`, for example `1×8 male`.
  `SchematicCanvasView` draws it for `.header`.

### Document setters (`SchematicModel.swift`, `// MARK: Chips`)

- `setPartGender(_ id: UUID, _ gender: String)`: sets it when the part is a
  header and lists the gender; otherwise does nothing.
- `setHeaderSize(_ id: UUID, to newPart: CatalogPart)`: the caller looks up
  `HeaderFamily.name(rows:perRow:)` in its catalog and passes the part, as
  `placeChip(part:…)` takes a part (`SchematicDocument` holds the C library,
  not the catalog). Does nothing unless the chip and `newPart` are both
  headers and `newPart` differs from the chip's part. Otherwise:
  1. note pad 1's board point (when placed on the board);
  2. `chips[i] = chips[i].resized(to: newPart)`;
  3. remove every wire with an end `.pin(PinRef(chipID: id, index: k))`
     where k ≥ the new pin count, then `pruneJunctions()`;
  4. move `board.centre` so pad 1 is back on its noted point, as in
     `setPartFootprint`;
  5. `settleWires(attachedTo: id)`.

### File (`SchematicFile.swift`)

- `Part.gender: String? = nil`, written for headers only (always), read
  back. A header entry whose `gender` the part doesn't list fails with
  `badPartOption`: `U4: "Neutral" is not a gender of GENERIC-HEADER-1X08`.
  A missing gender is the part's default.
- `footprint` uses plan-18's handling (`footprintChoices`), unchanged.

Rejected alternatives:

- **One drawer row per size.** 80 rows for one kind of part. The owner
  asked for a single generic header with a size option.
- **Storing `GENERIC-HEADER` plus rows and pins in the file.** It would need
  a name mapping in the netlist, the file and the sidebar header. The sized
  part number is the backend part, so the file, the netlist
  (`partNames: chips.map(\.partNumber)`) and the circuit need no mapping.
- **Refusing a shrink that would orphan wires.** The owner chose to remove
  those wires.
- **Making `PlacedChip.part` a `var`.** `baseLayout` is derived from the part
  in `init`, and a mutable part would let them drift apart. `resized(to:)`
  builds a fresh chip through the same `init`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` for partial, with what remains. Mark moot tasks
> `- [x] ~~text~~ — moot: <evidence>`. Fill each `Commit:` line as it lands.
> An unticked box means NOT DONE.

### Phase 1 — Catalog, drawer row, symbol, gender and file

- [x] `Backend/PartCatalog.swift`: `.header(rows:perRow:genders:)`,
      `symbol(for:)` `.header`, `detailLabels`, the one-row `entries` filter.
- [x] New `Backend/HeaderFamily.swift` (§ Catalog).
- [x] `Drawer/ChipDrawerView.swift`: header row title and subtitle.
- [x] `Canvas/SchematicModel.swift`: `PartSymbol.header`,
      `ChipLayout.header`, its case in `init(part:symbol:)`;
      `PlacedChip.gender` with normalization and the `gender:` init
      parameter; `placeChip(… gender:)` (default nil).
- [x] New `Canvas/HeaderSymbolView.swift`; `SchematicCanvasView` case `.header`.
- [x] `Backend/PartFacts.swift:rows(for chip:)`: for a header, `Rows`,
      `Pins per row`, `Gender` (Package and Footprint already follow the
      choice).
- [x] `Document/SchematicFile.swift`: `Part.gender`, written and read, and
      the `badPartOption` gender check.
- [x] `docs/file-format.md` § Parts: the `gender` row; the header in the
      `partNumber` row's list; the error in § Validation on open.
- [x] New `scripts/harness/generic-header.swift` (kept). It prints one
      `ok`/`FAIL` line per check and exits 1 on any FAIL. First part:
  1. The drawer groups (`groups(matching: DrawerFilter())`) list exactly one
     header-kind part, `GENERIC-HEADER-1X02`;
     `catalog.part(named: "GENERIC-HEADER-2X40")` exists.
  2. `placeChip(GENERIC-HEADER-1X02)` → `gender == "Male"`,
     `footprint?.id == "PINHDR-1X02-V"`, 2 `boardPads`, and the layout has
     2 slots on `.left` at `centerY` 10 and 30.
  3. A placed `GENERIC-HEADER-2X04`'s layout puts pins 1, 3, 5, 7 on `.left`
     and 2, 4, 6, 8 on `.right`, with pin 2 at pin 1's `centerY`.
  4. Encoding a document with a Female `GENERIC-HEADER-1X08` on
     `PINHDR-1X08-RA` gives `"gender" : "Female"` and `"footprint" : "PINHDR-1X08-RA"`,
     and decoding and applying it gives an equal `SchematicFile`. A copy with
     `"gender": "Neutral"` fails with
     `U1: "Neutral" is not a gender of GENERIC-HEADER-1X08`.
  5. `Harness.renderCanvas` of a male 1×4 and a female 2×3 →
     `/tmp/p23-headers.png`.

Acceptance: one drawer row, the right default, both genders drawn, and the
file round trip.
  Check: `rm -rf /tmp/p23-h && scripts/harness/build.sh scripts/harness/generic-header.swift /tmp/p23-h && /tmp/p23-h/H.app/Contents/MacOS/harness`
  → 5 `ok` lines, exit 0. Then read `/tmp/p23-headers.png`: the 1×4 shows
  four pins down the left with filled pin marks, numbers 1–4 and `1×4 male`;
  the 2×3 shows pins 1, 3, 5 left and 2, 4, 6 right, socket marks and
  `2×3 female` (est. 2 min).
  Result (2026-11-07): harness → checks 1–5 `ok`, exit 0. `/tmp/p23-headers.png`
  read: the 1×4 has four filled pin marks down the left, numbers 1–4, `1×4
  male`; the 2×3 has 1, 3, 5 left and 2, 4, 6 right with open-U sockets and
  `2×3 female`.
Commit: —

### Phase 2 — Size swap

- [x] `Canvas/SchematicModel.swift`: `PlacedChip.resized(to:)`;
      `setHeaderSize(_:to:)` and `setPartGender` (§ Document setters).
- [x] `generic-header.swift`, add:
  6. A 1×8 header with wires from pins 1, 2 and 8 to three junctions.
     `setHeaderSize(id, to: <GENERIC-HEADER-1X04>)` → part `GENERIC-HEADER-1X04`,
     the wire from pin 8 and its junction are gone, the wires on pins 1 and
     2 end at the new pin positions (`doc.path(forWire:)`'s first point is
     the pin tip), and the reference and origin are unchanged.
  7. For each of the vertical and right-angle footprints and each rotation
     0/90/180/270: from 1×8, `setHeaderSize` to 2×4, then to 1×2. After each
     step, pad 1's board point is within 1e-9 mm of before, and the
     footprint keeps its type (`-V` stays `-V`). That's 16 cases, reported
     as one `ok` line with the count.
  8. `setHeaderSize` to `SN74HC00N`, and to the header's current part →
     nothing changes (`chips` equal before and after). `HeaderFamily.name(rows: 1, perRow: 41)`
     names no catalog part (`catalog.part(named:)` → nil).
  9. `setPartGender(id, "Female")` → Female; `"Neutral"` → unchanged; on a
     `SN74HC00N` → `gender` stays nil. A Female 1×8 resized to 1×4 is still
     Female.

Acceptance: a resize keeps everything but the removed pins and their wires,
and pad 1 never moves.
  Check: the Phase 1 command → 9 `ok` lines, exit 0 (est. 2 min).
  Result (2026-11-07): 9 `ok` lines, exit 0; check 7 prints `(16 pad-1 cases)`.
Commit: —

## Validation Plan

- Tests: `generic-header.swift` (9 checks): the drawer, the defaults, layout,
  file round trip and bad gender, resize with wire removal, the 16 pad-1
  cases, no-op cases, gender. plan-18's `generic-led-file.swift` and
  `generic-led-ui.swift` must still pass (the shared option code is
  touched).
- Coverage check: check 7 prints 16 cases.
- Runtime proof: check 5's image.
- Doc sync: `docs/file-format.md` (Phase 1).
- Final gate (once, after Phase 2): `ctest --test-dir /tmp/p23-build -j8` →
  21/21; `generic-header.swift` 9 `ok`, `generic-led-file.swift` 7 `ok`,
  `generic-led-ui.swift` 10 `ok`, each from a fresh out dir;
  `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p23-dd CODE_SIGNING_ALLOWED=NO build`
  → `BUILD SUCCEEDED` (est. 7 min).

## Decisions

- One drawer row, `GENERIC-HEADER`, for all 80 sizes. Owner, 2026-11-07
  ("similar to the generic LED").
- Male/Female is shown on the symbol and saved; the footprint doesn't
  change. Owner, 2026-11-07.
- Shrinking removes the wires on pins that go away. Owner, 2026-11-07.
- Zigzag pins: odd on the left, even on the right for 2 rows. Owner,
  2026-11-07. Settles `ChipLayout.header`.
- New headers start 1×2, Male, vertical. Owner, 2026-11-07. Here: the drawer
  row is `GENERIC-HEADER-1X02`; Settings is plan-23-C.

## Corrections

- Phases 1 and 2 landed in one commit: the setters (`resized(to:)`,
  `setHeaderSize`, `setPartGender`) were written with the model changes
  they extend, and the harness was run with all 9 checks at once. Each
  phase's own check was still run and is recorded on its Result line.
- Added: `PartCatalogStore.listsPartsDirectly` includes `PART_HEADER`, so
  the drawer shows Connector ▸ Header ▸ GENERIC-HEADER, as for the LED,
  rather than an extra "Header" function level. Check 1 passes either way.
- `HeaderSymbolView`'s frame is `HeaderSymbolView.labelHeight` (16) taller
  than the body, so the size label under it is inside the view. The layout,
  hit rect and pin tips use the body alone (`ChipLayout.header`).
- The harness builds with `APP=/tmp/p23-dd/Build/Products/Debug/schematical.app`
  (the app built from this worktree), which `build.sh` copies, rather than
  building the app again into the out dir.

## Summary

Most of this sub-plan copies plan-18's per-part options. The new part is the
size swap, which replaces the placed part. It is tested headlessly for wire
removal, wire re-fit, pad-1 anchoring across 16 footprint × rotation cases,
and no-op inputs before any control can call it.
