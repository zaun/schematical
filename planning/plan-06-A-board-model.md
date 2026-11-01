# plan-06-A: Board settings, footprint marks, board texts and references in the model and file

Last updated: 2026-11-01
Overall Effort: x-large (1d–3d)
Effort: large (3h–1d)
Depends on: nothing (plan-03-D is complete)

plan-06 gives the PCB page a context-aware sidebar, a board rectangle in
the board's colour, document/silkscreen marks per footprint, an origin point
per footprint, and free text on the board. This sub-plan builds the data
for all of it, with no UI: board settings (size, copper layers, colour,
units), per-footprint origin and mark layers, board texts, and editable part
references; all saved in the `.schematical` file.

Outcome: a file with a `board` object, `pcb` origin/marks and texts opens,
round-trips through Save unchanged, and an older file opens with the
defaults; a footprint with origin `center` snaps and rotates about its
centre; renaming a part to an existing reference is refused.

The sub-plans, in order:

- **A** (this): model and file.
- **B**: drawing — board rectangle and colour, mark colours, marks
  hidden/document/silkscreen, texts.
- **C**: the PCB sidebar — board panel, footprint panel, units switch.
- **D**: PCB toolbar (Select, Text), the text tool, the text panel, delete.

References:

- `planning/completed/plan-03-D-pcb-tab.md` (PCB page, `BoardPlacement`,
  file `pcb` field).
- `schematical/schematical/Canvas/BoardModel.swift`,
  `Canvas/SchematicModel.swift` (`PlacedChip`, `SchematicDocument`),
  `Document/SchematicFile.swift`.
- `docs/file-format.md` (§ Parts, § Validation on open, § Changing the
  format).

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-03-D complete | `ls planning/completed/plan-03-D-pcb-tab.md` → exists | MET (2026-11-01) |
| App builds at the start | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/schematical-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-01, at 72f95a7) |

Everything below is written against a tree where both hold. plan-04 and
plan-05 do not touch the PCB code (plan-05-C adds a control to the
`.navigation` toolbar group, which plan-06 does not edit); they are not
prerequisites.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue and before you stop; if you stop,
> report all prerequisites.

## 1. Goal

- `SchematicDocument.board: BoardSettings` (width, height, layers, colour,
  units) and `texts: [BoardText]`, saved under a new optional top-level
  `board` object.
- `BoardPlacement` gains `origin` (center / top left / center left / bottom
  left) and three mark layers (bounding box, pin 1, reference: hidden /
  document / silkscreen), saved in each part's `pcb`.
- Snapping, rotation and the X/Y the sidebar will edit all use the
  footprint's origin point.
- `PlacedChip.reference` can be changed as prefix + number; duplicates and
  bad values are refused with a message.

### Non-goals (explicit constraints)

- No drawing or UI change (B, C, D). The app must look and behave as at
  72f95a7 with a default board.
- File `version` stays `2`; files from this build still open in the
  previous build (it ignores the new keys); files from before open with the
  defaults below.
- The stored `pcb.x`/`pcb.y` keep meaning the footprint centre.
- Auto-placement of new footprints (`BoardLayout.nextSpot`) is unchanged.
- No backend, part-file or golden change.

## 2. Current State

- `BoardPlacement` (`BoardModel.swift`) holds `centre` (mm) and
  `rotation`. `BoardLayout.snapped(_:footprint:)` puts the top-left of the
  body-plus-pads box on the 0.635 mm grid (72f95a7).
  `SchematicDocument.rotateFootprint` (`SchematicModel.swift:738`) keeps the
  top-left of that box fixed, then snaps; `moveFootprint` sets the centre,
  then snaps.
- `PlacedChip.reference` is a `let` (`SchematicModel.swift:385`).
  `placeChip` names every new part `"U\(nextReference)"` and increments
  (`:694`). On open, `nextReference` is raised above
  `Int(reference.dropFirst())` of every part (`SchematicFile.swift:342`),
  which reads only one-letter prefixes.
- `SchematicDocument` keeps one selection, `selectedChipID`.
- `SchematicFile` is `Equatable`; unsaved changes are found by comparing
  snapshots (`SchematicFileController.swift:73`), so new fields in the file
  struct are covered without other changes.
- The schematic canvas does not draw references; they appear on the PCB
  (`PCBCanvasView.swift:99`) and in memory window titles
  (`MemoryWindow.swift:94,143`).
- The Xcode project uses file-system-synchronized groups
  (`grep -c SynchronizedRootGroup schematical/schematical.xcodeproj/project.pbxproj`
  → 4), so a new `.swift` file under `schematical/schematical/` is built
  without a project edit.
- No Swift unit-test target (plan-03-D § Current State). Swift behaviour is
  proven with the headless harness (§ Harness).

### Measured populations

| What | Count | Command |
|---|---|---|
| Uses of `.reference` on a chip or part | 9 (plus the init assignment) | `grep -rn '\.reference\b' schematical/schematical \| grep -c '\.swift:'` → 10 |
| Call sites of `BoardLayout.snapped` / `nextSpot` (incl. definitions) | 6 lines | `grep -rn 'BoardLayout.snapped\|nextSpot' schematical/schematical \| grep -c .` → 6 |
| `replaceContents` definition + callers | 2 | `grep -rn replaceContents schematical/schematical \| grep -c .` → 2 |
| Board colours | 7 | owner's list: green, black, red, blue, yellow, white, purple |

### Verified properties

- New `.swift` files join the build with no `project.pbxproj` edit — read:
  the two `PBXFileSystemSynchronizedRootGroup` entries in the project.
- Swift's `JSONDecoder` ignores unknown keys, so older builds read files
  with `board` and the new `pcb` keys — plan-03-D relied on the same rule
  for `pcb` and its runtime proof opened such files.
- The app builds in 12 s incrementally (`time xcodebuild … build` →
  11.6 s total, 2026-11-01).

## 3. Design Overview

1. **Types** (`Canvas/BoardSettings.swift`, new): `BoardColor`,
   `BoardUnits`, `MarkLayer`, `TextLayer`, `FootprintOrigin`,
   `BoardSettings`, `BoardText`, `ReferenceParts`.
2. **Placement**: `BoardPlacement` gains `origin` and the three mark
   layers; `BoardLayout.snapped` and rotation use the origin point.
3. **Document**: board settings, texts, text selection, renaming.
4. **File**: `board` object, new `pcb` keys, validation, docs.

Correctness risk: the origin change touches every snap — move, rotate,
place, open. With the default origin (top left) the results must equal
72f95a7's, which the harness checks against coordinates recorded before
the change. Byte-identity is not this plan's gate (the file gains keys);
the gate is the harness: defaults reproduce today's positions and every
new key round-trips.

Rejected alternatives:

- *Storing the origin point instead of the centre in `pcb.x`/`y`* — changes
  the meaning of an existing field (needs a version bump) for no gain; the
  centre is derived from either.
- *Swift enums decoded directly by `Codable`* — an unknown value then
  surfaces as a generic decoding error; decoding strings and checking them
  in `apply` gives a message that names the field.
- *A separate references list for the PCB* — one `reference` per part,
  shared by schematic, PCB and memory windows, cannot drift.

## 4. Detailed Design

### 4.1 Types (`Canvas/BoardSettings.swift`)

All `nonisolated`, `Hashable`, `Sendable`, string raw values = the file
values.

```swift
enum BoardColor: String, CaseIterable { case green, black, red, blue, yellow, white, purple }
enum BoardUnits: String, CaseIterable { case mm, inch = "in" }
enum MarkLayer: String, CaseIterable { case hidden, document, silkscreen }
enum TextLayer: String, CaseIterable { case document, silkscreen }
enum FootprintOrigin: String, CaseIterable { case center, topLeft, centerLeft, bottomLeft }

struct BoardSettings {
    var width: Double = 100      // mm, 1...1000
    var height: Double = 80      // mm, 1...1000
    var layers: Int = 2          // copper layers, one of 1, 2, 4, 6
    var color: BoardColor = .green
    var units: BoardUnits = .mm  // units the sidebar shows lengths in
    static let layerCounts = [1, 2, 4, 6]
    static let sizeRange: ClosedRange<Double> = 1...1000
}

struct BoardText: Identifiable {
    let id: UUID
    var text: String             // may hold line breaks; never empty
    var anchor: CGPoint          // mm; top-left of the text at rotation 0; on the grid
    var size: Int                // font size in mil, 10...1000
    var layer: TextLayer = .silkscreen
    var rotation: Int = 0        // 0, 90, 180, 270 clockwise, about `anchor`
    static let sizeRange = 10...1000
    static let defaultSize = 50
    var sizeMM: Double { Double(size) * 0.0254 }
}
```

`BoardColor` titles for the UI: "Green", "Black", "Red", "Blue", "Yellow",
"White", "Purple". `FootprintOrigin` titles: "Center", "Top Left",
"Center Left", "Bottom Left". `MarkLayer` titles: "Hidden", "Document",
"Silkscreen".

`FootprintOrigin.point(in rect: CGRect) -> CGPoint`: center (midX, midY),
topLeft (minX, minY), centerLeft (minX, midY), bottomLeft (minX, maxY). The
rectangle is the footprint's bounding box as turned (`boardBounds`).

`ReferenceParts`: `init(_ reference: String)` splits `^[A-Za-z]+[0-9]+$`
into `prefix` and `number: Int?`; any other string → `prefix` = the whole
string, `number` = nil. `static func validate(prefix:number:) ->
String?` returns the error message or nil: prefix not 1–8 ASCII letters →
"The prefix must be 1 to 8 letters."; number < 1 or absent → "The number
must be 1 or more.". The reference is `prefix.uppercased() + String(number)`.

Text geometry (`Canvas/BoardTextGeometry.swift`, new, `import AppKit`):
`BoardText.font` = `NSFont.systemFont(ofSize: sizeMM, weight: .medium)`;
`localSize` = `NSString(text).boundingRect(with: .greatestFiniteMagnitude,
options: .usesLineFragmentOrigin, attributes: [.font: font]).size`;
`boardRect` = that rectangle from `anchor`, turned about `anchor` by
`rotation` with `BoardPlacement.rotate` (the rectangle's four corners
rotated, then their bounding box). B draws with the same `NSFont`, so the
hit box matches the drawing.

### 4.2 Placement (`BoardModel.swift`)

```swift
struct BoardPlacement: Hashable, Sendable {
    var centre: CGPoint
    var rotation: Int
    var origin: FootprintOrigin = .topLeft
    var outline: MarkLayer = .document     // the body outline ("bounding box")
    var pin1: MarkLayer = .silkscreen
    var reference: MarkLayer = .silkscreen
}
```

- `BoardLayout.snapped(_:footprint:)`: the bounding box as turned →
  `p = placement.origin.point(in: bounds)`; shift the centre by
  `BoardGrid.snap(p) − p` on each axis. Top left reproduces today's rule.
- `BoardLayout.originPoint(of placement:, footprint:) -> CGPoint` — `p`
  above, for the sidebar's X/Y.
- `SchematicDocument.rotateFootprint`: keep the origin point (not the
  top-left) fixed across the turn, then snap.
- `SchematicDocument.moveFootprintOrigin(_ id:, to point: CGPoint)`: shift
  the centre so the origin point lands on `point`, then snap (C's X/Y
  fields).
- `SchematicDocument.setFootprintOrigin(_ id:, _ origin:)`: changes only
  `origin`; the footprint does not move and is not re-snapped.
- `SchematicDocument.setFootprintMarks(_ id:, outline:pin1:reference:)`
  (each optional; nil leaves it).
- `nextSpot` builds `BoardPlacement(centre:rotation: 0)` with the defaults,
  so new footprints get origin top left and the default marks.

### 4.3 Document (`SchematicModel.swift`)

- `PlacedChip.reference` becomes `var`.
- `var board = BoardSettings()`; `private(set) var texts: [BoardText] = []`;
  `var selectedTextID: UUID?`. `selectedChipID` and `selectedTextID` each
  clear the other in `didSet` when set non-nil.
- `renameChip(_ id: UUID, prefix: String, number: Int?) -> String?`:
  `ReferenceParts.validate` message, or "\(new) is already used by another
  part." when another chip has that reference; nil on success, which sets
  `reference`.
- `placeChip`: the reference is `"U\(n)"` for the first `n ≥
  nextReference` no chip uses; `nextReference = n + 1`.
- Texts: `addText(at point: CGPoint) -> UUID` (text "Text",
  `BoardText.defaultSize`, silkscreen, rotation 0, anchor snapped to the
  grid; selects it), `setText(_:_:)` (an empty string is refused and the
  text kept), `setTextSize(_:_:)` (clamped to `sizeRange`),
  `setTextLayer(_:_:)`, `moveText(_:to:)` (anchor snapped),
  `setTextRotation(_:_:)`, `rotateText(_:)` (+90 mod 360), `deleteText(_:)`
  (clears `selectedTextID` when it was selected), `text(at point:) ->
  BoardText?` (last whose `boardRect` contains the point).
- `setBoardSize(width:height:)` clamps each to `BoardSettings.sizeRange`.
- `replaceContents(chips:wires:junctions:nextReference:board:texts:)`
  also clears `selectedTextID`.

### 4.4 File (`SchematicFile.swift`, `docs/file-format.md`)

```json
"board": { "width": 100, "height": 80, "layers": 2, "color": "green", "units": "mm",
  "texts": [ { "id": "...", "text": "REV A", "x": 5.08, "y": 2.54,
               "size": 50, "layer": "silkscreen", "rotation": 0 } ] },
...
"pcb": { "x": 5.08, "y": 0, "rotation": 90, "origin": "topLeft",
         "outline": "document", "pin1": "silkscreen", "reference": "silkscreen" }
```

- `SchematicFile.board: Board? = nil` (`width`, `height`, `layers`,
  `color`, `units` as `String`/`Double`/`Int`, `texts: [Text]`).
  `Placement` gains optional `origin`, `outline`, `pin1`, `reference`
  (`String?`). Always written by this build; missing keys take the §4.1 /
  §4.2 defaults.
- `apply` checks, and on failure throws the new
  `SchematicFileError.badPCB(String)` — "The PCB settings are not valid:
  \(detail)." — leaving the document unchanged. Details:
  - `board width 0 must be 1 to 1000 mm.` (same for height);
  - `board layers 3 must be 1, 2, 4 or 6.`;
  - `board color "pink" must be green, black, red, blue, yellow, white or purple.`;
  - `board units "cm" must be mm or in.`;
  - `U2 origin "middle" must be center, topLeft, centerLeft or bottomLeft.`;
  - `U2 outline "on" must be hidden, document or silkscreen.` (same for
    `pin1`, `reference`);
  - `text <id> size 5 must be 10 to 1000 mil.`; `text <id> layer "x" must be
    document or silkscreen.`; `text <id> rotation 45 must be 0, 90, 180 or
    270.`; `text <id> is empty.`
- Text IDs join the duplicate-ID check (`claim`). Text anchors are snapped
  on open. Footprints are snapped by their stored origin.
- `nextReference` on open: above the highest `n` of every `U<n>` reference
  (`ReferenceParts`, prefix `U`), replacing `Int(reference.dropFirst())`.
- `docs/file-format.md`: § Top level gets `board`; a new § Board section
  (fields, defaults, the text object, units mm/in, sizes in mm, text size
  in mil); § Parts' `pcb` row lists the four new keys and their defaults;
  the origin note in § Coordinates (the origin point snaps to the grid;
  `x`/`y` stay the centre); § Validation on open lists the `badPCB` cases;
  § Example gains a `board` object.

## Harness

Swift behaviour is proven headlessly, as in plan-03-D: the app's sources
(all but `schematicalApp.swift`) and the backend compile into a CLI placed
inside a copy of the Debug `.app` so `Bundle.main` finds the part files.
Create `/tmp/p06/harness/build.sh`:

```sh
#!/bin/sh
# build.sh <repo root> <out dir> <Debug .app>   (MAIN=<main.swift>)
set -e
R=$1; O=$2; APP=$3; mkdir -p $O/obj
B=$R/backend
INC="-I$B/include -I$B/src -I$B/src/engine -I$B/src/library -I$B/src/models -I$B/src/third_party/cjson"
for f in $(find $B/src -name '*.c'); do clang -c -O0 $INC $f -o $O/obj/$(basename $f .c).o; done
S=$R/schematical/schematical
swiftc -swift-version 5 -default-isolation MainActor -enable-upcoming-feature MemberImportVisibility \
  -enable-upcoming-feature NonisolatedNonsendingByDefault -enable-upcoming-feature InferIsolatedConformances \
  -import-objc-header $S/schematical-Bridging-Header.h $INC \
  $(find $S -name '*.swift' ! -name schematicalApp.swift) $MAIN $O/obj/*.o -o $O/harness 2>&1 | grep -E "error" || true
rm -rf $O/H.app; cp -R "$APP" $O/H.app
cp $O/harness $O/H.app/Contents/MacOS/harness
```

Run: `MAIN=/tmp/p06/harness/a.swift sh /tmp/p06/harness/build.sh $PWD
/tmp/p06/out /tmp/schematical-dd/Build/Products/Debug/schematical.app &&
/tmp/p06/out/H.app/Contents/MacOS/harness <args>`. Each phase's
`main.swift` is written by the implementer; it builds a `PartCatalogStore`,
calls `load()`, and drives `SchematicDocument` / `SchematicFile` directly.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit;
> `- [~]` for partial; moot as `- [x] ~~text~~ — moot: <evidence>`; fill
> `Commit:` when a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Baseline record

Before any code change, record today's placements so Phase 2 can prove the
default origin reproduces them.

- [ ] `/tmp/p06/harness/a.swift` `baseline`: open `test.schematical`,
      place `SN74HC00N` and `LED-AMBER-0603` with `placeChip`, rotate the
      DIP once and the oscillator twice, move the LED by (+3.0, +2.2) mm;
      print every chip's reference, centre and rotation to
      `/tmp/p06/baseline.txt`, one line per chip.
- [ ] Same harness, `render` mode: render `PCBCanvasView` for that document
      with `ImageRenderer` (900 × 600) to `/tmp/p06/a-pcb-before.png`.

Acceptance: the baseline exists at 72f95a7's sources.
  Check: `wc -l < /tmp/p06/baseline.txt` → 7 (`test.schematical` has 5
  parts — `python3 -I -c 'import json;print(len(json.load(open("test.schematical"))["parts"]))'`
  → 5 — plus the two placed; VCC/GND lines read `no placement`) (est. 5 min).
Commit: — (no repo change)

### Phase 2 — Types, placement and document

- [ ] `Canvas/BoardSettings.swift` (new): §4.1 types, titles,
      `FootprintOrigin.point(in:)`, `ReferenceParts`.
- [ ] `Canvas/BoardTextGeometry.swift` (new): `font`, `localSize`,
      `boardRect` (§4.1).
- [ ] `Canvas/BoardModel.swift`: `BoardPlacement` fields; `snapped` by the
      origin point; `originPoint(of:footprint:)` (§4.2).
- [ ] `Canvas/SchematicModel.swift`: `reference` var, `board`, `texts`,
      `selectedTextID`, `renameChip`, `placeChip` reference rule, the
      footprint origin/mark setters, `moveFootprintOrigin`,
      `rotateFootprint` about the origin, text methods, `setBoardSize`,
      `replaceContents` signature (§4.2, §4.3).
- [ ] `Document/SchematicFile.swift`: pass `board: BoardSettings()` and
      `texts: []` to `replaceContents` (the file fields come in Phase 3).

Acceptance: default origin reproduces the baseline; other origins snap and
pivot on their point; renaming refuses bad and duplicate references.
  Check: rebuild the harness; `harness baseline | diff - /tmp/p06/baseline.txt`
  → no output (est. 3 min).
  Check: `harness origins`: an SN74HC00N with origin `center`, moved to
  (10.1, 10.1) → its bounding-box centre on the grid
  (`abs(x/0.635 - round(x/0.635)) < 1e-9` printed `true` for both axes);
  rotated → bounding-box centre unchanged (`true`); `bottomLeft` → (minX,
  maxY) on the grid and fixed across a rotate (`true`, `true`); changing
  the origin does not move the centre (`true`) (est. 2 min).
  Check: `harness rename`: place two parts (U1, U2); rename U2 to
  (`"r"`, 3) → nil, reference `R3`; U1 to (`"R"`, 3) → `R3 is already used
  by another part.`; U1 to (`"R2"`, 1) → `The prefix must be 1 to 8
  letters.`; U1 to (`"R"`, 0) → `The number must be 1 or more.`; rename U1
  to (`"U"`, 3), then place a part → it is named `U4` (est. 1 min).
  Check: `harness texts`: `addText(at: (1.0, 1.0))` → anchor (1.27, 1.27),
  selected, chip selection nil; `setText(id, "")` keeps "Text";
  `setTextSize(id, 5)` → 10; `rotateText` ×4 → 0; `text(at:)` inside the
  rect → the text, outside → nil; `deleteText` clears the selection (each
  printed `true`) (est. 1 min).
  Check: the §Prerequisites `xcodebuild` → `** BUILD SUCCEEDED **` (est.
  1 min).
Commit: —

### Phase 3 — File format and docs

- [ ] `Document/SchematicFile.swift`: `Board`, `Board.Text`, new
      `Placement` keys, write, read with defaults, `badPCB` checks, text IDs
      in `claim`, `nextReference` by `U<n>` (§4.4).
- [ ] `docs/file-format.md`: the §4.4 edits.

Acceptance: every new value round-trips; older files open with defaults;
every bad value gives its message and leaves the document unchanged.
  Check: `harness roundtrip`: open `test.schematical` (no `board`) → board
  100 × 80, 2 layers, green, mm, no texts, every placement topLeft /
  document / silkscreen / silkscreen; set board 120.5 × 60, 4 layers,
  purple, in; LED origin center, outline hidden, pin1 document; add two
  texts (one rotated 90, size 80, document); save to `/tmp/p06/rt.schematical`,
  reopen, save again → files byte-equal (`cmp` exit 0) and
  `SchematicFile(document:)` equal (`true`) (est. 2 min).
  Check: `harness bad`: each of the 10 §4.4 bad values, written into a copy
  of `rt.schematical` → the exact message, and the document's snapshot
  equal to before (`true` ×10); a text sharing a part's id →
  `duplicateID` (est. 2 min).
  Check: `grep -c '"board"\|origin\|badPCB\|not valid' docs/file-format.md`
  → ≥ 4, and the § Example block parses: `python3 -I` extracting the last
  ```json block of the file with `json.loads` → no error (est. 1 min).
  Check: the §Prerequisites `xcodebuild` → `** BUILD SUCCEEDED **` (est.
  1 min).
Commit: —

## Validation Plan

- Tests: no Swift test target; the harness modes above (`baseline`,
  `origins`, `rename`, `texts`, `roundtrip`, `bad`) are the tests. Backend
  untouched.
- Coverage check: each new document method and each `badPCB` branch is
  driven by a harness mode above.
- Runtime proof: the app opens `/tmp/p06/rt.schematical` and shows the PCB
  page as before (no UI change yet): harness `render` mode renders
  `PCBCanvasView` with `ImageRenderer` to `/tmp/p06/a-pcb.png`, compared by
  eye with a render at 72f95a7 (`/tmp/p06/a-pcb-before.png`, taken in
  Phase 1).
- Doc sync: `docs/file-format.md`.
- Final gate (once, after Phase 3): `cmake -S backend -B /tmp/logic-p06a &&
  cmake --build /tmp/logic-p06a -j8 && ctest --test-dir /tmp/logic-p06a`
  (est. 1 min; nothing in `backend/` changes, so it must still pass) + the
  `xcodebuild` (est. 1 min) + the harness modes (est. 5 min).

## Decisions

- Board size is an editable rectangle from (0, 0), default 100 × 80 mm,
  saved in the file — owner, 2026-11-01.
- "Layers" is the copper layer count: 1, 2, 4 or 6, default 2; stored only
  (top side stays the only drawn side) — owner, 2026-11-01.
- Origin point (center, top left, center left, bottom left) is the point
  that snaps to the grid, the rotation pivot, and the X/Y shown in the
  sidebar; per part, saved; default top left — owner, 2026-11-01.
- Mark defaults: bounding box (= the body outline drawn today) Document;
  pin 1 and reference Silkscreen — owner, 2026-11-01.
- The reference is edited as two fields, prefix and number — owner,
  2026-11-01. Author: prefix 1–8 letters (uppercased), number ≥ 1,
  duplicates refused.
- One mm/in switch for every length; font size stays in mil; the choice is
  saved with the board — owner, 2026-11-01.
- Texts have X/Y, rotation 0/90/180/270 and delete, plus text, layer
  (document/silkscreen) and font size in mil — owner, 2026-11-01.
- Author's choices: board size 1–1000 mm; text size 10–1000 mil, default
  50 mil, text "Text", silkscreen; text position is its top-left at
  rotation 0, rotation about that point; texts snap to the 0.635 mm grid;
  changing a footprint's origin does not move it; new parts take the first
  unused `U<n>`.

## Corrections

## Summary

Risk sits in moving every snap onto the origin point; the default must
reproduce today's positions, checked against a recorded baseline. The file
change is additive and keeps version 2.
