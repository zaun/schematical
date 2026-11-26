# plan-68-F: App — the document core; `SchematicModel.swift` gone

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-E

After plan-68-D and plan-68-E, `SchematicDocument` still holds the parts and
their edit logic, the undo machinery, both pages' selections, and the edits
that span parts, sheet and board (erase, group drags, paste, locks). This
letter gives the parts the same treatment — a `PartList` value with the
logic, stored by the document — and files the rest by job, so that
`SchematicModel.swift` no longer exists and `SchematicDocument.swift` is the
small core: stored state, undo, selection. Afterwards the only app files
over 1000 code lines are the six that plan-68-V splits.

Prerequisites: plan-68-A's table, and plan-68-E complete (`ls
schematical/schematical/Model/Board/BoardContent.swift` → the file, and
plan-68-E's Commit lines filled).

References:

- plan-68-D § 3.1 (the pattern).
- `SchematicModel.swift` marks left after T and U: Document, Selecting, Box
  select, Undo, Chips, Units (plan-67), Group drags (Schematic), Copy and
  paste (Schematic), Document info, Title block, Footprints (PCB), Electrical
  rules check settings, Lock Position (PCB), Group drags (PCB), Pins,
  Erasing, Geometry; and the thin sheet and board methods T and U left.
- `Canvas/PlacedChip.swift` (plan-68-B), `Canvas/SchematicClipboard.swift`
  (an existing `extension SchematicDocument`).

## 1. Goal

- `schematical/schematical/Canvas/SchematicModel.swift` does not exist.
- `SchematicDocument` is the files of § 3, each at or under 800 code lines;
  `PartList` holds the part edit logic and has its own test class.
- `SchematicModel.swift`'s row is gone from `scripts/source-size-ratchet.tsv`
  and `SZ` passes.
- Every existing test passes unchanged.

### Non-goals

- As plan-68-D: public API, `DocumentState`, file format, undo steps,
  observation granularity unchanged. `chips` and `nextReference` stay stored
  on the document (owner: the document keeps the parts), `private(set)`.

## 2. Current State

Code lines of the marks this letter files (`sections.js` on
`SchematicModel.swift` at `e2f6811a`): Document 140, Selecting 38, Box
select 215, Undo 164, Chips 217, Units 128, Group drags (Schematic) 135,
Copy and paste 118, Document info 33, Title block 22, Footprints 115, ERC
settings 37, Lock Position 54, Group drags (PCB) 97, Pins 108, Erasing 124,
Geometry 14. Plus the one-line document methods plan-68-D and plan-68-E
left (counted when they land; one file per value type below, so no file's
size depends on that count).

## 3. Design

| File (`schematical/schematical/Model/Document/`) | Contents |
|---|---|
| `SchematicDocument.swift` | the class: stored state (`chips`, `nextReference`, `sheet`, `layout`, `documentInfo`, `erc`, selections and their mirroring, editor state, caches), `EditScope`, `edit`, `recordEdit`, undo (`setState`, `replaceContents`), `updateParts`, Document info, Title block, ERC settings — the marks Document, Selecting, Undo, Document info, Title block, ERC settings (434) |
| `SchematicDocument+Selection.swift` | Box select (215): queries over parts, sheet and board; it sets the selection through the core's selection methods |
| `SchematicDocument+Parts.swift` | the part methods: thin `edit { updateParts { $0.… } }` calls for Chips, Units, Footprints |
| `SchematicDocument+Wiring.swift`, `+Buses.swift`, `+Labels.swift`, `+Notes.swift` | the thin sheet methods from plan-68-D, one file per sheet value type |
| `SchematicDocument+BoardSettings.swift`, `+BoardTexts.swift`, `+BoardCopper.swift`, `+BoardFeatures.swift` | the thin board methods from plan-68-E, one file per board value type |
| `SchematicDocument+GroupDrags.swift` | Group drags (Schematic) and (PCB): 232 |
| `SchematicDocument+Erase.swift` | Erasing, Lock Position: 178 |
| `SchematicDocument+Pins.swift` | Pins, Geometry (queries): 122 |
| `Model/Parts/PartList.swift`, `PartList+Units.swift`, `PartList+Footprints.swift` | `struct PartList { var chips; var nextReference }` with the logic of Chips (217), Units (128), Footprints (115) |

`Copy and paste (Schematic)` (118) joins the existing
`Canvas/SchematicClipboard.swift` extension (its other half).

Cross-cutting edits (erase, delete selection, group drags, paste, removing a
part with its wires and traces) call `updateParts`, `sheet.update…` and
`layout.update…` inside one `edit`, so they stay one undo step.

`SchematicModel.swift`'s last content moves into the files above; the file is
deleted, and its size row leaves the ratchet.

### Tests

`PartListTests` (new): every `PartList` method on hand-built values, no
document. The existing suites are the proof the API did not move.

### Where the risk is

Undo for cross-cutting edits: a delete that removes a part, its wires and its
traces must undo as one step that puts all three back. `SchematicUndoTests`,
`PCBUndoTests`, `UndoHistoryTests` and `LooseEndTests` (deleting a part
leaves loose ends; undo restores the pins) cover it.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` with what remains; moot tasks are
> struck through with evidence, never deleted; fill `Commit:` when a phase
> lands. **An unticked box means NOT DONE.**

### Phase 1 — `PartList`

- [x] `Model/Parts/PartList*.swift` with the Chips, Units, Footprints logic;
  `updateParts(scope)` on the document (compare-and-write-back of `chips` and
  `nextReference`); the part methods become thin.
- [x] `PartListTests`.

Acceptance: the part logic is in `PartList`, tested alone.
  Check: `XB -only-testing:schematicalTests/PartListTests
  -only-testing:schematicalTests/SplitPartEditTests
  -only-testing:schematicalTests/SplitPartModelTests
  -only-testing:schematicalTests/ReferencePrefixTests
  -only-testing:schematicalTests/SocketModeTests
  -only-testing:schematicalTests/SchematicUndoTests` → all pass (est. 5 min).
  Observed: `node scripts/coverage.js --app --only schematical/schematical/Model/Parts/
  --tests` with those six classes → 0 failed cases, `322 lines, 0 uncovered`.
Commit: COMMIT_F1

### Phase 2 — file the document; delete `SchematicModel.swift`

- [ ] Create the `Model/Document/` files of § 3 and move every remaining
  member; delete `SchematicModel.swift`; Copy and paste into
  `SchematicClipboard.swift`.
- [ ] `scripts/source-size-ratchet.tsv`: the `SchematicModel.swift` row
  removed.

Acceptance: no document file over 800; the document behaves as before.
  Check: `ls schematical/schematical/Canvas/SchematicModel.swift` → no such
  file; `node scripts/source-size.js --all | grep -E
  'schematical/schematical/Model/' | awk '$1>800'` → no output; `SZ` → exit 0; the full `XB` → no failure beyond
  the standing ones (est. the full Debug suite: the whole document moves).
Commit: —

## Validation Plan

- Final gate (once, after Phase 2): the full `XB`; `SZ`; `COV --app --check`.

## Decisions

- The document keeps the parts, the undo and the selection mirroring (owner,
  plan-68-A).
- (author) Thin document methods filed one file per value type, so file sizes
  do not depend on how many there are.

## Corrections

- **Switch inputs go through `updateInputs`**, a core method that makes its
  own scope: a tactile button held while the simulation plays, and the
  positions put back when it stops, are inputs, not edits, and were never
  recorded (`setButton`, `restoreSwitches`, `releaseAllButtons`).
- **A paste takes its references from a copy of the parts** (`var references
  = parts`): a refused paste used to put `nextReference` back by hand; now
  nothing is written until the paste is accepted.
- **The switch, slide and button edits share one voltage check**
  (`refusesSwitches(at:_:)`), and their state changes moved into `PartList`
  (`toggled`, `setSwitches`, `setSlide`).

- **The prerequisite names `Model/Board/BoardContent.swift`**, not
  `BoardLayout.swift`: plan-68-E named the owner `BoardContent` (the name
  `BoardLayout` was taken; plan-68-E Corrections). The `layout:` row of
  § 3 is that class.

## Summary

The last structural cut. After it `SchematicDocument` is a core class plus
job-sized extensions over value types, and both multi-sheet (more
`SchematicSheet`s) and multi-unit (`PartList+Units`) have one place each.
