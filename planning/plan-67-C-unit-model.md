# plan-67-C: Unit view — the model

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-67-B

Lets a placed part be **split**: drawn as its units, each with its own
position, flip and turn. This letter is the model: where the placements are
stored, how a split part answers every geometry question the rest of the app
asks (pin tips, hit tests, bounds), how wires follow, the document's edit
functions, and which units of a selected part are **picked**. It has no user
interface: after it lands a test (and nothing else) can split a part.

References:

- plan-67-A (the feature and its prerequisites), plan-67-B (`piece(…)`, `.gate`).
- `schematical/schematical/Canvas/SchematicModel.swift`: `PlacedChip`
  (1026-1302), selection (1709-2060), `flipChip` / `rotateChip` (2719-2739),
  `chip(at:)` (2741), group drags (2753-2900), `pasteSchematic` (2903),
  `swapPart` (2540), pin lookups (5720-5860), `adoptLooseEnds` (6183),
  `settleWires(attachedTo:)` (6244).
- `schematical/schematical/Canvas/Selection.swift` (`SchematicItem`,
  `SchematicGroupBase`), `Canvas/UndoHistory.swift`.
- `AGENTS.md` § Git.

Test command form `XB`: see plan-67-A.

## Prerequisites

The feature's prerequisites are in plan-67-A § Prerequisites; re-run them. This
letter adds one:

| Must be true | Command | Status |
|---|---|---|
| plan-67-B has landed | `grep -c 'func piece(_ unit: PartUnit' schematical/schematical/Canvas/SchematicModel.swift` → 1 | NOT MET (2026-11-25: 0) |

## 1. Goal

- `document.setPartSplit(id, true)` turns a part with units into its pieces,
  laid out from where the block was; `false` turns it back. Both are one
  undoable edit and every wire stays on its pin.
- A split part's pins are where its pieces put them: `pinTip`, `pin(at:)`,
  `pinTips`, `adoptLooseEnds`, wire paths and ERC markers all follow with no
  change at their call sites.
- `moveUnit`, `flipUnit` and `rotateUnit` change one unit and re-fit the wires
  on that part.
- Setting a split part's `origin` moves all its units together, so every
  existing whole-part move (group drag of a whole part, paste, the agent's
  `move_part`) keeps working.
- The document knows which units of a selected split part are picked, and a
  group drag moves only those.

### Non-goals (explicit constraints)

- `SchematicItem` gets no new case. A split part is selected as `.chip(id)`,
  so delete, copy, the sidebar, the PCB mirror and undo keep treating it as one
  part.
- `PinRef` and `WireEnd` do not change: a wire end is still a part id and a
  pin index.
- The netlist, the simulation, the board, the BOM and the ERC rules read
  nothing new.
- Nothing is saved (letter D), drawn (E) or exported (F) differently yet.

## 2. Current State

- A part has one `origin`, `isFlipped`, `rotation` (`SchematicModel.swift:1035-1042`)
  and one `layout` (1244). `tip(of:)` (1266) is `origin` + the slot's offset +
  a stub on the slot's side, which assumes every pin is on the one body's edge.
- Pin lookups go through `chip.layout.slots` and then `chip.tip(of:)`:
  `pinTip` (5720), `pinTips` (5740), `pin(at:)` (5772), `anchorAxis` (5815,
  reads `slot.side`), `adoptLooseEnds` (6191), and the file's wire resolver
  (`SchematicFile.swift:1248`).
- Hit tests use `hitRect`: `chip(at:)` (2741) and `schematicItems(in:)` (1910).
- A group drag records `base.chips[id] = chip.origin` (2757) for each selected
  part and treats a wire end as moving when its part is in `base.chips`
  (`moves`, 2789); `moveSchematicGroup` (2821) writes `chips[i].origin`.
- `flipChip` / `rotateChip` (2719, 2731) change the field, then
  `settleWires(attachedTo:)` and `adoptLooseEnds(chips:)`.
- `pasteSchematic` (2903) rebuilds each chip through `PlacedChip.init`, copying
  fields one by one (2918-2928); `resized(to:symbol:)` (1192) does the same for
  a part swap.
- Undo snapshots `chips` in `DocumentState` and diffs each chip under
  `UndoKey.part(id)` (`UndoHistory.swift:112`): any stored field of
  `PlacedChip` other than the board placement and the reference is covered
  with no further code.
- Selection is `schematicSelection: ItemSelection<SchematicItem>` (1709), which
  already has a property observer; `pruneSelection` (2044) drops ids that are gone.

### Measured populations

| What | Count | Command |
|---|---|---|
| `layout.slots` call sites outside symbol views and exporters (they run on a whole part and must see every unit's slots) | 9: `SchematicModel.swift:5723, 5742, 5774, 5819, 6191`; `SchematicFile.swift:1248, 1251`; `AutomationRead.swift:148, 265` | `grep -n 'layout\.slots' schematical/schematical/Canvas/SchematicModel.swift schematical/schematical/Document/SchematicFile.swift schematical/schematical/Automation/AutomationRead.swift` |
| `.hitRect` call sites | 4 in 3 files (`SchematicModel.swift:1912, 2742`; KiCad writer; EasyEDA writer) | `grep -rn '\.hitRect' schematical/schematical --include='*.swift'` |
| `.tip(of:` call sites | 6 in 4 files | `grep -rn '\.tip(of:' schematical/schematical --include='*.swift'` |
| `.chip(` uses (selection items; none changes) | 76 in 15 files | `grep -rnE '\.chip\(' schematical/schematical --include='*.swift' -c` |

### Verified properties

- **All 9 whole-part `layout.slots` sites only match a slot by pin and then call
  `chip.tip(of:)` or read `slot.side`.** Read each: 5723 (index → `tip`), 5742
  (`tip` of every slot), 5774 (distance to `tip`), 5819 (`slot.side` → axis),
  6191 (`tip` → map), `SchematicFile` 1248/1251 (designator or index → `PinRef`),
  `AutomationRead` 148/265 (`tip` and `side`). None reads `centerX`/`centerY`
  against the part's origin. So a split part can return the union of its
  pieces' slots from `layout.slots` and dispatch `tip(of:)` by pin, and all 9
  are right without an edit.
- **A slot's `side` on a piece is already the flipped and turned side** (read
  `flipped()` 703 and `rotated(by:)` 729), so `anchorAxis` gets the right axis.
- **Undo covers a new stored field of `PlacedChip`** (read
  `UndoHistory.swift:97-130`: the `.part` key compares the chip with `board`
  and `reference` blanked).
- **Selection is not part of undo** (read `DocumentState`,
  `UndoHistory.swift:10-32`: no selection field), so picks need no undo work.

## 3. Design Overview

1. **Storage** — `PlacedChip.unitPlacements`; empty means Block view.
2. **Geometry** — `pieces`, and `origin` / `layout` / `tip(of:)` /
   `bodyRect` / `hitRect` answering for a split part.
3. **Edits** — `setPartSplit`, `moveUnit`, `flipUnit`, `rotateUnit`, and the
   places that copy a chip field by field.
4. **Picks** — which units of a selected split part a drag, flip or turn acts on.

Correctness risk concentrates in geometry dispatch: a wire end resolved against
the wrong body is a wire drawn to empty space. It is why the geometry lives in
five members of `PlacedChip` rather than at the call sites, and why the tests
check every pin of a split part against its piece.

Byte-identity is not this letter's gate. What must not change is every existing
test: an unsplit part takes the old path in each member.

Rejected:

- *A `.unit(UUID, String)` selection item.* 76 `.chip(` sites would each need a
  decision, and the owner's rule is that select, copy and delete take the whole
  IC. Picks sit beside the selection instead.
- *A stored union origin kept in step with the placements.* Two sources of
  truth; `origin` is computed for a split part instead.
- *Units as separate `PlacedChip`s in `chips` sharing a "package" id.* The
  netlist, board, BOM, ERC and every exporter iterate `chips` as parts.

## 4. Storage and geometry

```swift
nonisolated struct UnitPlacement: Hashable, Sendable {
    let key: String            // PartUnit.key
    var origin: CGPoint        // top-left of the unit's body, on the grid
    var isFlipped = false
    var rotation = 0           // 0 unless the unit's symbol turns
}
nonisolated struct UnitRef: Hashable, Sendable { let chipID: UUID; let key: String }
```

`PlacedChip`:

- `private(set) var unitPlacements: [UnitPlacement] = []`, in `part.units`
  order when not empty. `var isSplit: Bool { !unitPlacements.isEmpty }`.
- The stored origin is renamed `private var blockOrigin`; `origin` becomes
  computed. Get: `blockOrigin` when not split, else the top-left of the union
  of the pieces' `bodyRect`s. Set: `blockOrigin = newValue` when not split,
  else every placement's origin shifts by `newValue − origin`.
- `var pieces: [PlacedChip]` — `[self]` when not split; else one
  `piece(unit, origin:isFlipped:rotation:)` per placement.
- `layout` — when split: `ChipLayout(size: <union size>, slots: pieces.flatMap { $0.layout.slots })`
  (a new internal memberwise `ChipLayout.init(size:slots:)`). Its slot offsets
  are piece-relative and must only be used through `tip(of:)`.
- `tip(of:)` — when split: the piece whose layout has `slot.pin.index`, and
  that piece's own `tip(of:)` for its slot.
- `bodyRect` — when split: the union of the pieces' `bodyRect`s.
  `hitRect` — the union of the pieces' `hitRect`s (exporter extents use it).
- `func piece(at point: CGPoint) -> PlacedChip?` — the last piece whose
  `hitRect` contains the point; `func pieces(in rect: CGRect) -> [PlacedChip]`.
- `isFlipped` and `rotation` of a split part stay what they were as a block and
  are not read while it is split.
- `mutating func split()` — no-op unless `part.units` is not empty and the part
  is not split. Placements in unit order, in a column: the first at
  `blockOrigin`; each next one 20 below the previous unit's body
  (`y += previous size.height + 20`), same x; every origin snapped with
  `SchematicGrid.snap`; flips false, rotations 0.
- `mutating func join()` — `blockOrigin = origin` (the union's top-left), then
  `unitPlacements = []`.
- `mutating func setPlacements(_:)` — used by the file reader and paste:
  accepts a list only when its keys are exactly `part.units`' keys, each once;
  reorders it to unit order; normalizes each rotation and forces it to 0 for a
  block unit; returns whether it was accepted.
- `mutating func updatePlacement(_ key: String, _ body: (inout UnitPlacement) -> Void)`.

Document (`SchematicDocument`):

- `chip(at:)` uses `chips.last { $0.piece(at: point) != nil }`;
  `func piece(at point: CGPoint) -> PlacedChip?` returns that piece.
- `schematicItems(in:)` inserts `.chip(id)` when `!chip.pieces(in: rect).isEmpty`.

## 5. Edits

All through `recordEdit(.schematic, …)`, each followed by
`settleWires(attachedTo: id)` and `adoptLooseEnds(chips: [id])`, as `flipChip`
does:

| Function | Undo name | Does |
|---|---|---|
| `setPartSplit(_ id: UUID, _ split: Bool)` | "Change View" | `split()` / `join()`; no-op when the part has no units or is already so |
| `moveUnit(_ ref: UnitRef, to origin: CGPoint)` | "Move" | sets the snapped origin |
| `flipUnit(_ ref: UnitRef)` | "Flip" | toggles `isFlipped` (every unit symbol flips) |
| `rotateUnit(_ ref: UnitRef, counterclockwise: Bool = false)` | "Rotate" | a quarter turn; no-op for a block unit |

`flipChip` and `rotateChip` return without change for a split part.

Field-by-field copies learn the new field:

- `pasteSchematic` (2918): after building `placed`, when `chip.isSplit`,
  `placed.setPlacements(chip.unitPlacements shifted by the paste offset)`.
- `resized(to:symbol:)` (1192): keeps the placements when the new part's unit
  keys equal the old ones (`setPlacements` accepts), else the result is a block
  at the old `origin`.

## 6. Picks

`SchematicDocument`:

- `private(set) var pickedUnits: [UUID: Set<String>] = [:]`. An entry names the
  units of a selected split part that are picked. **No entry means every unit
  is picked.**
- `func picked(_ chip: PlacedChip) -> Set<String>` — the entry intersected with
  the chip's unit keys when the chip is selected and split and the entry is not
  empty; otherwise all its unit keys.
- `func pick(_ ref: UnitRef, extend: Bool)` — sets the entry to `[key]`, or
  with `extend` toggles the key in it (an entry that becomes empty is removed).
- `func pickUnits(_ refs: Set<UnitRef>)` — replaces the entries for those chips.
- The `schematicSelection` observer (1709) and `pruneSelection` drop the entry
  of every chip that is no longer selected, so a part selected again by Select
  All, a PCB click or the agent has all units picked.

Group drags: `SchematicGroupBase` gains `var units: [UnitRef: CGPoint] = [:]`.
`schematicGroupBase()` (2753), for a selected split chip, records each picked
unit's origin in `base.units` and does **not** put the chip in `base.chips`.
`moves` (2789) is true for a `.pin` end when its part is in `base.chips` **or**
the unit that owns the pin index is in `base.units`. `moveSchematicGroup`
(2821) applies the snapped delta to those placements with `updatePlacement`;
`settleSchematicGroup` (2871) includes those chips in its `adoptLooseEnds`.

## Compatibility / Format Impact

None outside the app's memory: files, exports and the agent API are untouched.
A split part cannot exist in a saved file until letter D.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as
> the work; `- [~]` for partial; mark moot tasks; fill `Commit:`; add tasks you
> discover. **An unticked box means NOT DONE.**

### Phase 1 — storage and geometry

- [ ] `Canvas/SchematicModel.swift`: add `UnitPlacement` and `UnitRef` above
      `PlacedChip`; add `ChipLayout.init(size:slots:)`.
- [ ] `PlacedChip`: `unitPlacements`, `isSplit`, `blockOrigin` with the computed
      `origin`, `pieces`, the split branches of `layout`, `tip(of:)`,
      `bodyRect`, `hitRect`, and `piece(at:)`, `pieces(in:)`, `split()`,
      `join()`, `setPlacements(_:)`, `updatePlacement` (§4). `init` keeps its
      `origin:` parameter and stores it in `blockOrigin`.
- [ ] `SchematicDocument.chip(at:)` and `schematicItems(in:)` per §4; add
      `piece(at:)`.
- [ ] Tests: new `schematicalTests/SplitPartModelTests.swift` —
      `testSplitLaysUnitsInAColumn` (`SN74HC00N` placed at (200, 200) via
      `TestSupport.place`, then `split()` on a copy: 5 placements keyed
      A, B, C, D, P at x 200 and y 200, 260, 320, 380, 440; `origin` (200, 200));
      `testEveryPinHasATipOnItsPiece` (for that split chip, each
      `layout.slots` entry's `tip(of:)` equals the tip the owning piece gives,
      and all 14 pins are present once); `testSettingOriginMovesEveryUnit`;
      `testJoinPutsTheBlockAtTheUnionCorner`; `testHitTests` (a point on unit C
      hits, a point in the gap between two units does not, `pieces(in:)` of a
      box over B and C returns those two); `testSetPlacementsRefusesWrongKeys`
      (missing key, extra key, duplicate key → false and the chip stays a
      block; a block unit's rotation 90 becomes 0); `testUnsplitPartIsUnchanged`
      (`pieces == [self]`, `origin`, `layout`, `tip(of:)`, `bodyRect`, `hitRect`
      equal the values computed from the stored fields as before).

Acceptance: a split part answers every geometry question from its pieces, and
an unsplit part answers exactly as before.
  Check: `XB -only-testing:schematicalTests/SplitPartModelTests -only-testing:schematicalTests/SchematicRotateTests`
  → `** TEST SUCCEEDED **` (est. 5 min).
Commit: —

### Phase 2 — edits and wires

- [ ] `SchematicDocument`: `setPartSplit`, `moveUnit`, `flipUnit`, `rotateUnit`
      (§5), beside `flipChip`; the early return in `flipChip` / `rotateChip`.
- [ ] `pasteSchematic` and `PlacedChip.resized(to:symbol:)` per §5.
- [ ] Tests: new `schematicalTests/SplitPartEditTests.swift` —
      `testWiresFollowTheSplit` (a 7400 with pin `1Y` wired to an LED and `VCC`
      to a supply: after `setPartSplit(id, true)` each wire's `path(for:)` ends
      at `pinTip` of its pin, which is on unit A / unit P; after `false` again
      at the block's pins); `testUndoAndRedo` (one undo step named
      "Change View" restores the block and its origin; redo splits again with
      the same placements); `testMoveFlipRotateOneUnit` (`moveUnit` of B leaves
      A's tips where they were and moves B's by the delta; `flipUnit` swaps
      B's input and output sides; `rotateUnit` of B gives sides top/bottom;
      `rotateUnit` of P changes nothing; each is one undo step with its name);
      `testFlipChipIgnoresASplitPart`; `testPasteKeepsTheArrangement` (copy a
      split part, paste at an offset: the paste is split, each unit origin is
      the original plus the snapped offset, flips and turns kept, new
      reference); `testPartsWithoutUnitsRefuse` (`setPartSplit` on a resistor
      and on `SN74HC74N` changes nothing and records no undo step).

Acceptance: splitting, joining and editing one unit keep every wire on its pin
and are each one undo step.
  Check: `XB -only-testing:schematicalTests/SplitPartEditTests -only-testing:schematicalTests/SchematicClipboardTests -only-testing:schematicalTests/SchematicUndoTests`
  → `** TEST SUCCEEDED **` (est. 5 min).
Commit: —

### Phase 3 — picks and group drags

- [ ] `SchematicDocument`: `pickedUnits`, `picked(_:)`, `pick(_:extend:)`,
      `pickUnits(_:)`, and the clean-up in the `schematicSelection` observer and
      `pruneSelection` (§6).
- [ ] `Canvas/Selection.swift`: `SchematicGroupBase.units`.
- [ ] `schematicGroupBase`, `moves`, `moveSchematicGroup`,
      `settleSchematicGroup` per §6.
- [ ] Tests in `SplitPartEditTests` — `testPicksDefaultToEveryUnit` (select the
      chip: `picked` is all five keys; `pick(A)` → `["A"]`; `pick(B, extend:
      true)` → A and B; deselect and select again → all five);
      `testGroupDragMovesOnlyPickedUnits` (pick B, `schematicGroupBase` then
      `moveSchematicGroup(by: (40, 0))` and `settleSchematicGroup`: B's origin
      moved by 40, A, C, D, P unmoved, a wire on B's output still ends on its
      tip); `testGroupDragOfAWholeSplitPart` (no pick entry: all five move);
      `testMixedGroupDrag` (a picked unit and a selected resistor move by the
      same delta).

Acceptance: a drag moves the picked units and nothing else of that part.
  Check: `XB -only-testing:schematicalTests/SplitPartEditTests -only-testing:schematicalTests/SelectionModelTests -only-testing:schematicalTests/SelectionGestureTests`
  → `** TEST SUCCEEDED **` (est. 5 min).
Commit: —

## Validation Plan

- Tests: `SplitPartModelTests` (7 cases), `SplitPartEditTests` (10 cases); the
  existing rotate, clipboard, undo and selection classes are re-run in the
  phase that touches their code.
- Coverage check: `testUnsplitPartIsUnchanged` pins the old path of each
  changed member; the re-run classes exercise that path through the document.
- Runtime proof: none; this letter has no user interface.
- Doc sync: none in this letter.
- Final gate: in plan-67-G, once.

## Decisions

- **Select, copy and delete take the whole IC** — decided by the owner,
  2026-11-25. Settles that the selection item stays `.chip(id)`. (§3)
- **A click targets the clicked gate: rotate, flip and drag act on it, its
  siblings are shown faintly; a marquee over several gates moves those gates**
  — decided by the owner, 2026-11-25. Settles picks. (§6)
- **Unused gates are always on the sheet** — decided by the owner, 2026-11-25:
  "it's up to the user to position them and mark the I/O". A split part always
  has a placement for every unit; there is no hidden unit. (§4 `setPlacements`)
- **Switching views keeps wires on their pins and lays the units in a column
  from the block's position** — the author's call; the owner was told on
  2026-11-25 and raised no objection.

## Corrections

## Summary

The engineering risk is the geometry dispatch in `PlacedChip`: five members
gain a split branch, and every existing caller keeps its code. What is left
untouched: selection items, wire ends, the netlist, the board, undo's machinery.
