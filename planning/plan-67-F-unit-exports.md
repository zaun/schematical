# plan-67-F: Unit view — exports

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-67-E

Makes every Schematic export show a split part as it is on the canvas:

- **Image, SVG, PDF and print** draw each piece with its symbol and its
  `U1-A` label.
- **KiCad** gets a real multi-unit symbol: one library symbol with a unit per
  piece, one instance per piece, all with the reference `U1`, so KiCad shows
  `U1A` … and still sees one part with one footprint.
- **EasyEDA Pro** gets one component whose symbol holds every piece's box and
  pins where they are on the sheet.

References:

- plan-67-A (the feature and its prerequisites), plan-67-B (`paint`,
  `drawnReference`), plan-67-C (`pieces`, the split `origin` / `layout` /
  `tip(of:)`).
- `schematical/schematical/Canvas/SchematicExportScene.swift` (`SchematicScene`,
  `paint(into:)` 38-100).
- `schematical/schematical/Document/KiCadSchematicWriter.swift`: `extent` (48),
  `libraryName(of:)` (266), `define` (329), `bodyBox(of:)` (372), `instance`
  (397), `instances(reference:)` (439).
- `schematical/schematical/Document/EasyEDAProSchematicWriter.swift`:
  `bodyBox(of:)` (110), `symbolID` (130), `partSymbolFile` (207), `sheet` (231),
  `edgeConnector` (387).
- `docs/export.md` § KiCad (226-310).
- `schematical/schematicalTests/KiCadExportTests.swift` (`ExportSamples` 542,
  `testPrintSamplesWhenAsked` 468), `EasyEDAProExportTests.swift`.
- `AGENTS.md` § Git.

Test command form `XB`: see plan-67-A. `KC` is
`~/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli`.

## Prerequisites

The feature's prerequisites are in plan-67-A § Prerequisites (KiCad's
command-line tool among them); re-run them. This letter adds one:

| Must be true | Command | Status |
|---|---|---|
| plan-67-E has landed | `grep -c 'case parts(name: String)' schematical/schematical/Canvas/SchematicModel.swift` → 1 | NOT MET (2026-11-25: 0) |

## 1. Goal

- `SchematicExport.svg` of a document with a split 7400 contains the texts
  `U1-A`, `U1-B`, `U1-C`, `U1-D`, `U1-P` and no plain `U1` label.
- The KiCad schematic of that document has one library symbol with units
  1 to 5 and five instances with `(reference "U1")` and `(unit 1)` … `(unit 5)`;
  `kicad-cli sch export netlist` lists one component `U1` whose 14 pins are on
  the same nets as the app's netlist, and `kicad-cli sch erc` reports no
  multi-unit error.
- The EasyEDA Pro schematic has one component for the part, its symbol holding
  five rectangles and 14 pins at the sheet positions of the pieces' pins.
- A document with no split part exports exactly as before, in every format.

### Non-goals (explicit constraints)

- The KiCad and EasyEDA symbols stay what they are for every part today: a
  rectangle and pins. No gate outline is written into either. (The image, SVG,
  PDF and print exports do draw the gate shapes: they use the canvas's own
  drawing code.)
- EasyEDA Pro gets no multi-part symbol; the pieces cannot be moved apart in
  EasyEDA.
- Board, Gerber, pick-and-place and BOM exports read nothing new: a split part
  is one part with one reference in all of them.

## 2. Current State

- `SchematicScene.paint(into:)` (`SchematicExportScene.swift:38`) loops
  `for chip in chips`, translates to `chip.viewOrigin`, switches on
  `chip.symbol` to each view's static `paint`, then `ReferenceLabel.paint`.
  Image, SVG, PDF and print all go through it (`Document/ImageExport.swift:170`,
  `:257`; `Document/PrintExport.swift:166, 257, 527`).
- **KiCad** (`KiCadSchematicWriter.swift`): one library symbol per
  (part, flip, turn) — `libraryName(of:)` (266) appends `_flipped` and
  `_r<deg>`. `define` (329) writes a `<name>_0_1` sub-symbol with the body
  rectangle and a `<name>_1_1` sub-symbol with the pins at
  `tip(of:) − origin`, the flip and turn baked in. `instance` (397) writes one
  `(symbol …)` at the origin with `(unit 1)`, the chip's id as its uuid, a
  `(pin …)` uuid line per slot, and `instances(reference:)` (439), which
  hard-codes `(unit 1)`. `extent` (48) sizes the sheet from `chip.hitRect`.
- **EasyEDA Pro** (`EasyEDAProSchematicWriter.swift`): one `.esym` per
  (part, flip, turn) key (`symbolID`, 130); `partSymbolFile` (207) writes one
  `RECT` from `bodyBox(of:)` and the pins at `tip − origin`; `sheet` (231)
  writes one `COMPONENT` at `chip.origin` with its Designator.
- Neither writer has a multi-unit path. `planning/plan-63-data/README.md:50`
  records that KiCad's multi-unit ERC rules were skipped because "a part here
  is one whole package".
- The KiCad export is checked by `KiCadExportTests` and, by hand, with
  `kicad-cli` on files a test prints as `KICAD_FILE <folder>/<name> <base64>`
  lines when `TEST_RUNNER_KICAD_OUT=1` is set (`testPrintSamplesWhenAsked`,
  468; the test host is sandboxed and cannot write where the shell can read).

### Measured populations

| What | Count | Command |
|---|---|---|
| Hard-coded `(unit 1)` in the KiCad writer | 5 lines: 195 (edge connector), 416 (part instance), 444 (`instances`), 500 (power symbol), 550 (flag) | `grep -n '(unit 1)' schematical/schematical/Document/KiCadSchematicWriter.swift` |
| Painters that draw a chip for an export | 1 (`SchematicScene.paint`) | `grep -rn 'switch chip.symbol' schematical/schematical --include='*.swift'` → the scene, the canvas layer, `ReferenceLabel`, the two writers' `bodyBox` |

### Verified properties

- **For a split part, the pin positions both writers compute are already
  right.** Read `define` (333-337) and `partSymbolFile` (217-221): each pin is
  `chip.tip(of: slot) − chip.origin` over `chip.layout.slots`. After
  plan-67-C, a split chip's `layout.slots` is every piece's slots, `tip(of:)`
  dispatches to the piece, and `origin` is the union's top-left.
- **The board's link to the schematic symbol is the chip's id.** Read
  `instance` (417): `(uuid <chip.id>)`. Keeping that uuid on the first unit's
  instance leaves the board file's path unchanged.

## 3. Design Overview

1. **The scene** iterates pieces. This is the same change the canvas layer got
   in letter E and carries all five picture formats.
2. **KiCad**: a split chip gets its own library symbol and one instance per
   piece.
3. **EasyEDA Pro**: a split chip gets one symbol with a rectangle per piece.

Risk concentrates in the KiCad file: it must load, and KiCad must still see one
component. It is checked with KiCad itself, not only by reading the text.

Byte-identity **is** a gate for one class: a document with no split part must
export the same bytes as before in every format. A diff there is a bug to
root-cause on one fixture, not a reason to stop.

Rejected:

- *KiCad: one composite symbol, as for EasyEDA.* KiCad has real units; a
  composite would be one immovable lump there.
- *KiCad: an instance per piece with references `U1-A`, `U1-B`.* KiCad would
  see five components and five footprints.
- *EasyEDA: a component per piece.* The same fault, in the other tool.

## 4. KiCad

For a split chip:

- **Library name**: `libraryName(of:)` returns the part's base name plus
  `_units` and, per placement in order, `_<key><f|n><rotation>`
  (e.g. `SN74HC00N_units_An0_Bf90_Cn0_Dn0_Pn0`), so two parts arranged
  differently never share a symbol whose pins are baked in differently.
- **Definition**: no `<name>_0_1`. For piece number `k` (1-based, placement
  order) a sub-symbol `<name>_<k>_1` holding that piece's body rectangle
  (`bodyBox(of: piece)`) and that piece's pins at
  `piece.tip(of: slot) − piece.origin`, side `slot.side`.
- **Instances**: one `(symbol …)` per piece at the piece's origin, with
  `(unit k)`; the Reference property is the part's reference (`U1`); the Value
  and Footprint properties as today; the uuid is the chip's id for `k == 1` and
  `id("unit/<chip.id>/<key>")` for the others; the `(pin …)` uuid lines list
  only that piece's pins; `instances(reference:unit:)` writes `(unit k)`.
- The reference and value property positions are computed from the piece's
  origin and `bodyBox(of: piece)` exactly as they are from a whole part's.
- `extent` already covers a split part: `hitRect` is the union (plan-67-C §4).

An unsplit chip takes today's path untouched.

## 5. EasyEDA Pro

For a split chip:

- **Key**: `symbolID`'s key appends the same per-placement string as the KiCad
  name.
- **Symbol**: `partSymbolFile` writes one `RECT` per piece, each
  `bodyBox(of: piece)` offset by `piece.origin − chip.origin`; the `PART`
  bounding box and the Designator attribute use the union of those rectangles;
  the pin loop is unchanged (§2 Verified).
- **Sheet**: unchanged — one `COMPONENT` at `chip.origin`, the Designator
  placed from the union box.
- `edgeConnector` reads `hitRect.maxX`, which is the union's.

## Compatibility / Format Impact

- KiCad schematic: a split part is written as a multi-unit symbol. Files for
  documents without a split part are byte-identical.
- EasyEDA Pro schematic: a split part's symbol has several rectangles. Same
  identity rule.
- Picture exports show a split part as the canvas does.
- Board, Gerber, pick-and-place, BOM: unchanged.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as
> the work; `- [~]` for partial; mark moot tasks; fill `Commit:`; add tasks you
> discover. **An unticked box means NOT DONE.**

### Phase 1 — picture exports

- [ ] `Canvas/SchematicExportScene.swift`: `paint(into:)` loops
      `chips.flatMap(\.pieces)`; `connectedPins` is still looked up by the
      part's id.
- [ ] `schematicalTests/KiCadExportTests.swift` `ExportSamples`: add
      `static func split() throws -> SchematicDocument` — an `SN74HC00N` at
      (200, 200) split, unit B moved to (400, 200), flipped and turned to 90,
      `1Y` wired to an LED and a resistor to ground, `VCC` and `GND` on supply
      symbols, the unused inputs wired to ground.
- [ ] Tests: new `schematicalTests/SplitPartExportTests.swift` —
      `testSVGNamesEveryPiece` (the SVG text of `ExportSamples.split()`
      contains `>U1-A<` … `>U1-P<` and no `>U1<`); `testBoundsCoverEveryPiece`
      (`SchematicExport.bounds` contains each piece's `bodyRect`);
      `testUnsplitSceneIsUnchanged` (the SVG of the same document joined back
      to a block equals the SVG of a document built the same way without ever
      splitting).

Acceptance: the picture exports show the pieces.
  Check: `XB -only-testing:schematicalTests/SplitPartExportTests` → `** TEST SUCCEEDED **` (est. 4 min).
Commit: —

### Phase 2 — KiCad

- [ ] `Document/KiCadSchematicWriter.swift`: the split branches of
      `libraryName(of:)`, `define` and `instance`, and
      `instances(reference:unit:)` with a default `unit: 1` so the four other
      callers are unchanged (§4).
- [ ] `KiCadExportTests.testPrintSamplesWhenAsked`: also print
      `KiCadExport.files(for: ExportSamples.split(), base: "split")` under the
      folder `split`.
- [ ] Tests in `SplitPartExportTests` — `testKiCadUnits` (the schematic text of
      `ExportSamples.split()`: one `(symbol "Schematical:SN74HC00N_units_…"`
      definition with sub-symbols `_1_1` … `_5_1` and no `_0_1`; exactly five
      instances with that `lib_id`, with `(unit 1)` … `(unit 5)`, each with
      `(reference "U1")`; the first has the chip's uuid; each instance's
      `(at …)` is its piece's origin through the writer's scale);
      `testKiCadPinsOnce` (each of the 14 pin numbers appears in exactly one
      sub-symbol); `testKiCadUnsplitUnchanged` (for every `Fixture.names`
      entry the schematic text has no `_units` and every part instance has
      `(unit 1)`).
- [ ] Check with KiCad: run
      `TEST_RUNNER_KICAD_OUT=1 XB -only-testing:schematicalTests/KiCadExportTests/testPrintSamplesWhenAsked`,
      decode the `KICAD_FILE split/…` lines into `/tmp/p67-kicad/split/`, then
      `KC sch export netlist -o /tmp/p67-kicad/split.net /tmp/p67-kicad/split/split.kicad_sch`
      and `KC sch erc -o /tmp/p67-kicad/split.rpt /tmp/p67-kicad/split/split.kicad_sch`.
      Record here: the count of `(comp (ref "U1")` in the netlist (must be 1),
      that each of U1's 14 pins is on the net the app's `get_nets` form gives
      (compare with `KiCadExportTests`' own net table for the sample), and the
      count of `missing_unit`, `extra_units`, `different_unit_net` and
      `different_unit_footprint` lines in the ERC report (each must be 0).
      Save the netlist, the report and the commands in
      `planning/plan-67-data/kicad/` with a short `README.md`.

Acceptance: KiCad loads the file and sees one part with five units on the
right nets.
  Check: `XB -only-testing:schematicalTests/SplitPartExportTests -only-testing:schematicalTests/KiCadExportTests`
  → `** TEST SUCCEEDED **`; `grep -c '(comp (ref "U1")' /tmp/p67-kicad/split.net`
  → 1; `grep -cE 'missing_unit|extra_units|different_unit' /tmp/p67-kicad/split.rpt`
  → 0 (est. 8 min).
Commit: —

### Phase 3 — EasyEDA Pro

- [ ] `Document/EasyEDAProSchematicWriter.swift`: the split branches of
      `symbolID`'s key and `partSymbolFile`; the Designator position in `sheet`
      from the union box (§5).
- [ ] Tests in `SplitPartExportTests` — `testEasyEDAOneComponent` (the sheet
      of `ExportSamples.split()` has one `COMPONENT` for U1 with Designator
      `U1`); `testEasyEDASymbolHoldsEveryPiece` (its `.esym` has 5 `RECT`
      lines and 14 pins; each pin's position plus the component's origin is
      that pin's `pinTip` on the sheet, in the writer's y-up coordinates);
      `testEasyEDAUnsplitUnchanged` (for every `Fixture.names` entry no symbol
      key contains `_units`; the existing `EasyEDAProExportTests`, which pin
      the files' shape, pass unedited).
- [ ] `docs/export.md`: under § KiCad a paragraph "Parts drawn as units"
      (multi-unit symbol, one reference, the first unit carries the part's
      uuid, the leftover block is the last unit, what was checked with
      `kicad-cli` and the numbers recorded in Phase 2); under the EasyEDA
      section the composite symbol and its limit; under the picture exports
      one sentence.

Acceptance: EasyEDA sees one component with every pin where the wires end.
  Check: `XB -only-testing:schematicalTests/SplitPartExportTests -only-testing:schematicalTests/EasyEDAProExportTests`
  → `** TEST SUCCEEDED **` (est. 5 min).
Commit: —

## Validation Plan

- Tests: `SplitPartExportTests` (9 cases); `KiCadExportTests` and
  `EasyEDAProExportTests` re-run.
- Coverage check: the three `…Unchanged` cases run the writers over the
  existing fixtures, so the unsplit path is measured, not assumed.
- Runtime proof: KiCad's own netlist and ERC over the exported file (Phase 2).
- Doc sync: `docs/export.md`.
- Final gate: in plan-67-G, once.

## Decisions

- **The part stays one part in every other tool** — follows the owner's "this
  is a schematic view option only", 2026-11-25. Settles one reference and one
  footprint in KiCad and EasyEDA.
- **KiCad and EasyEDA symbols stay rectangles** — the author's call: every
  part's exported symbol is a rectangle and pins today (a resistor too), and
  the gate outline is a drawing detail of this app.

## Corrections

## Summary

The risk is the KiCad multi-unit file, so KiCad itself is the check. Exports of
documents that never use the option are byte-identical, and the board-side
exports are not touched.
