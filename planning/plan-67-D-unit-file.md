# plan-67-D: Unit view — the file format

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-67-C

Saves and opens a split part: a `.schematical` file keeps, for a part drawn as
its units, where each unit is and how it is flipped and turned. A file with no
split part is byte-for-byte what it was.

References:

- plan-67-A (the feature and its prerequisites), plan-67-C (`UnitPlacement`,
  `setPlacements`).
- `schematical/schematical/Document/SchematicFile.swift`: `Part` (70-118),
  writing (825-858), reading (1131-1145), the wire resolver (1236-1257),
  versions (18-23), `decode` (988-1023).
- `docs/file-format.md`: Parts (99-167), Changing the format (930-940).
- `schematical/schematicalTests/SchematicRotateTests.swift`
  (`testSaveAndOpen` 212, `testBadRotationInAFileIsIgnored` 230),
  `SchematicFileV5Tests.swift` (`testSaveOpenSaveIsStable` 301).
- `AGENTS.md` § Git.

Test command form `XB`: see plan-67-A.

## Prerequisites

The feature's prerequisites are in plan-67-A § Prerequisites; re-run them. This
letter adds one:

| Must be true | Command | Status |
|---|---|---|
| plan-67-C has landed | `grep -c 'func setPartSplit' schematical/schematical/Canvas/SchematicModel.swift` → 1 | NOT MET (2026-11-25: 0) |

## 1. Goal

- A document with a split part, saved and opened, has the same placements, and
  every wire is on the same pin.
- A part that is not split writes no new key: saving a file that has no split
  part gives the bytes it gave before this letter.
- A `units` list that does not match the part's units is ignored and the part
  opens as a block; the file still opens.

### Non-goals (explicit constraints)

- No version bump: `currentVersion` stays 5. The key is optional, and the
  format's own rule (`docs/file-format.md:930-940`) is that an optional field
  needs none.
- Wire ends keep their form (`{"part": <uuid>, "pin": "<designator>"}`); a unit
  is never named in a wire.
- The clipboard is not a file format (`SchematicClipboard` holds live values)
  and was handled in letter C.

## 2. Current State

- `SchematicFile.Part` (70-118) is a synthesized `Codable` struct. Optional
  fields are written only when they differ from the default: `rotation` is
  `chip.rotation == 0 ? nil : chip.rotation` (833) and read as
  `PlacedChip.normalized(part.rotation ?? 0)` (1138); `showReference` is
  written only as `false`.
- `x`, `y` are the part's top-left (`chip.origin`); `flipped` is always written.
- Unknown keys are ignored on read (plain `JSONDecoder`); a file whose
  `version` is above `currentVersion` is refused (`decode`, 988-1023). An older
  build therefore opens a file with the new key as blocks at `x`, `y`.
- A wire end is resolved after the chips are built, by designator through
  `chip.layout.slots` (1248). After letter C a split chip's `layout.slots` is
  the union of its pieces' slots, so the resolver needs no change once the
  placements are applied before it runs.
- A bad optional value is dropped, not an error: `PlacedChip.init` clamps a
  rotation the symbol cannot take (`SchematicModel.swift:1104`;
  `testBadRotationInAFileIsIgnored`).

### Measured populations

| What | Count | Command |
|---|---|---|
| `.schematical` files in the repo (none has a split part) | 17 | `find . -name '*.schematical' -not -path './.claude/*' -not -path './.git/*' \| wc -l` |
| Of those, under `examples/` (the ones the byte comparison test walks) | 9 | `find examples -name '*.schematical' \| wc -l` |

### Verified properties

- **`origin` of a split part is the union's top-left** (plan-67-C §4), so `x`,
  `y` written from `chip.origin` are right for an older build's block and need
  no special case when writing.
- **The chips are built before the wires are resolved.** Read
  `SchematicFile.apply`: parts at 1131-1145, wires through `resolve` from 1236.

## 3. Design Overview

One optional array on a part:

```json
"units": [
  { "unit": "A", "x": 200, "y": 200 },
  { "unit": "B", "x": 200, "y": 260, "flipped": true, "rotation": 90 },
  { "unit": "P", "x": 200, "y": 440 }
]
```

- `unit`: the unit's key. `x`, `y`: the top-left of its body.
- `flipped`: written only when true. `rotation`: written only when not 0.
- Present exactly when the part is split; it lists every unit of the part, in
  unit order.
- With `units` present, the part's own `x`, `y` are the top-left of the box
  around all units, `flipped` is false and `rotation` is omitted.

Reading: build the chip from `x`, `y` as a block, then call
`setPlacements` with the list. It accepts only a list whose keys are exactly
the part's unit keys, each once; otherwise the list is dropped and the part
stays a block at `x`, `y`.

Risk: low. The one thing that could go wrong silently is an existing file
changing on save; a byte comparison over the examples guards it.

Byte-identity **is** a gate here, for one class of input: files without a split
part. A diff there is a bug to root-cause (print the two encodings of one part),
not a reason to stop.

Rejected:

- *Bump the version to 6.* An older build would refuse every file saved after
  the option is used once, though it can show the part correctly as a block.
- *Write the unit list for unsplit parts too.* Every file would change.

## Compatibility / Format Impact

- New optional key `units` on a part, as above. Version stays 5.
- An older build opens such a file with the part as a block at `x`, `y`, wires
  attached to the block's pins, and drops the key if it saves.
- Unchanged: every other key, wire ends, the PCB section.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as
> the work; `- [~]` for partial; mark moot tasks; fill `Commit:`; add tasks you
> discover. **An unticked box means NOT DONE.**

### Phase 1 — write and read

- [ ] `Document/SchematicFile.swift`: add
      `struct Unit: Codable, Equatable, Sendable { var unit: String; var x: Double; var y: Double; var flipped: Bool?; var rotation: Int? }`
      inside `Part`, and `var units: [Unit]? = nil` with a doc comment in the
      style of `rotation`'s.
- [ ] Writing (825-858): `units` = nil when `!chip.isSplit`, else one entry per
      `chip.unitPlacements` (`flipped` nil unless true, `rotation` nil when 0);
      for a split chip write `flipped: false` and no `rotation`.
- [ ] Reading (1131-1145): after the chip is built, when `part.units` is not
      nil, `chip.setPlacements(…)` from it; the result is not an error either way.
- [ ] `docs/file-format.md`: a `units` row in the Parts table after `rotation`,
      the sub-table of a unit's keys, the rule for `x`/`y`/`flipped` when it is
      present, what an older build does, and what an unmatched list does.
- [ ] Tests: new `schematicalTests/SplitPartFileTests.swift` —
      `testSaveAndOpen` (a split `SN74HC00N` with B moved, flipped and turned
      to 90, wired to an LED: after encode and decode the placements are equal,
      the wire's end tip is equal); `testKeyOmittedForABlock` (the encoded JSON
      of a document with an unsplit 7400 has no `"units"`); `testWrittenShape`
      (the split part's JSON: 5 entries in order A, B, C, D, P; `flipped` and
      `rotation` only on B; the part's `x`, `y` equal `chip.origin`);
      `testUnmatchedListIsIgnored` (three hand-made files: a missing unit, an
      unknown key `"Z"`, a duplicate `"A"` → each opens, the part is a block at
      `x`, `y`, its wires resolve); `testBadUnitRotationIsDropped` (rotation 45
      on A → 0; rotation 90 on P → 0); `testSaveOpenSaveIsStable` (the second
      save's bytes equal the first's); `testExamplesAreUnchanged` (for every
      `.schematical` under `examples/`, open and encode: the bytes equal a
      second open-and-encode of that result, and no encoding contains
      `"units"`).

Acceptance: a split part survives a save, and files without one are untouched.
  Check: `XB -only-testing:schematicalTests/SplitPartFileTests -only-testing:schematicalTests/SchematicFileV5Tests -only-testing:schematicalTests/SchematicRotateTests`
  → `** TEST SUCCEEDED **` (est. 5 min).
Commit: —

## Validation Plan

- Tests: `SplitPartFileTests` (7 cases); the two existing file round-trip
  classes re-run.
- Coverage check: `testExamplesAreUnchanged` runs the writer over every example,
  so the no-new-key claim is measured on real files, not on one fixture.
- Runtime proof: in plan-67-G (save from the app, open again).
- Doc sync: `docs/file-format.md`.
- Final gate: in plan-67-G, once.

## Decisions

- **Existing files stay in Block view** — decided by the owner, 2026-11-25 (the
  answer to "which view does a new gate IC start in": Block, and existing files
  are never converted). Settles that absence of `units` means a block.

## Corrections

## Summary

One optional array. The risk is an unintended change to files that do not use
it, guarded by a byte comparison over the examples. Wire ends, the version and
every other key are untouched.
