# plan-67-G: Unit view — agent tools, docs, landing

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-67-F

Gives the agent (MCP) tools the unit view — switch a part's view, address one
unit as `U1-A`, move, flip and turn it, and read where every unit is — then
syncs the docs and runs the feature's one landing gate.

References:

- plan-67-A (the feature and its prerequisites) through plan-67-F.
- `schematical/schematical/Automation/AutomationTools.swift`: `chip(_:)` (264),
  `pin(_:)` (273), `centre(of:)` (317), `AutomationGuide.text` (338-376).
- `schematical/schematical/Automation/AutomationSchematic.swift`: `place_part`
  (13, `placePart` 275), `move_part` (51, `movePart` 576), `flip_part` (120),
  `rotate_part` (130), `set_part` (149, `setPart` 595), `item(_:_:forDelete:)`
  (906), `select` (998).
- `schematical/schematical/Automation/AutomationRead.swift`: `get_design`
  (`designText` 137-223, `designJSON` 251-330), `bounds(of:)` (133),
  `get_selection` (432).
- `docs/agent-control.md` (tools 52-118, conventions 120-136),
  `docs/file-format.md`, `docs/export.md`, `docs/parts-format.md`,
  `planning/todo.md` (§ Schematic, line 121).
- `AGENTS.md` § Git and § Testing in the app.

Test command form `XB`, `CT`: see plan-67-A.

## Prerequisites

The feature's prerequisites are in plan-67-A § Prerequisites; re-run them. This
letter adds one:

| Must be true | Command | Status |
|---|---|---|
| plan-67-F has landed | `grep -c 'instances(reference: String, unit: Int' schematical/schematical/Document/KiCadSchematicWriter.swift` → 1 | MET (2026-11-25: 1, `74fcd97f`) |

## 1. Goal

- `set_part reference=U1 view=gate` splits a gate IC; `view=block` joins it.
  `place_part … view=gate` places it split.
- `move_part`, `flip_part` and `rotate_part` accept a unit (`U1-A`) and act on
  that unit; `move_part U1` moves the whole part.
- A pin may be written `U1.3` (as today, whichever unit draws it) or `U1-A.3`.
- `get_design` lists every unit of a split part with its centre, size, body,
  flip and turn, in both the text and the JSON form.
- `select U1-A` picks that unit; `delete U1-A` is refused with a message that
  names the part to delete.
- The docs describe all of it, the todo list records it, and the landing gate
  passes.

### Non-goals (explicit constraints)

- No tool is added or removed; existing calls with a plain reference behave as
  before for unsplit parts.
- `get_nets`, `run_erc`, `run_drc`, `get_board`, `route` and every board tool
  keep printing `U1.3`: a pin belongs to the part.
- The agent's `delete` never waits on the confirm (done in letter E).

## 2. Current State

- A part is found by `AutomationContext.chip(_:)` (`AutomationTools.swift:264`),
  whole string, case-insensitive. A pin by `pin(_:)` (273): split at the
  **first `.`**, the left side through `chip(_:)`.
- `item(_:_:forDelete:)` (`AutomationSchematic.swift:906`) treats a spec with
  no `:` as a part reference and one with a `:` as `kind:id`. A reference is
  letters then digits (`ReferenceParts`, `Canvas/BoardSettings.swift:548`), so
  no reference contains `-`; `-` has no meaning in any parser under
  `Automation/` (checked by reading every parser: `chip`, `pin`, `anchor` 306,
  `item` 906, `portID` 992, `planeNet` in `AutomationBoard.swift:276`).
- `flip_part` (120) and `rotate_part` (130) are inline closures that check
  `chip.symbol.canFlip` / `canRotate` and call `flipChip` / `rotateChip`.
- `movePart` (576) computes a target origin from the centre or a delta using
  `chip.layout.size` and `chip.origin`, then `moveGroup(document, [.chip(id)], by:)`.
- `setPart` (595) applies each given field inside one
  `recordEdit(.schematic, "Change Part")` and returns the list of changes.
- `get_design` prints per part `centre=`, `size=WxH`, `body=(x1,y1)-(x2,y2)`
  from `origin` and `layout.size` (`AutomationRead.swift:119-160`), plus each
  pin's tip and side with `geometry=true`; the JSON form has `x`, `y`,
  `width`, `height`, `flipped`, `rotation`, `bounds`, and pins (259-301).
  After plan-67-C these values are the union box for a split part and the pin
  tips are already per piece.
- The connect-time instructions are `AutomationGuide.text` (338-376);
  `docs/agent-control.md:123` says the two are kept in step.

### Measured populations

| What | Count | Command |
|---|---|---|
| Tools whose behavior changes | 8: `place_part`, `move_part`, `flip_part`, `rotate_part`, `set_part`, `select`, `delete`, `get_design` (+ every tool taking a pin, through `pin(_:)`) | read of the tool tables in the three `Automation*.swift` files |
| Docs to sync | 4 files: `docs/agent-control.md`, `docs/file-format.md` (done in D), `docs/export.md` (done in F), `docs/parts-format.md` (done in A) | `ls docs/*.md` |

### Verified properties

- **`U1-A` cannot be mistaken for an existing address.** Read
  `ReferenceParts.validate` (566): a reference is 1–8 ASCII letters then a
  number; `setPart`'s own check is `/([A-Za-z]+)(\d+)/` (634). No reference
  holds `-`, and `item` only gives `:` a meaning.
- **A pin name after the dot may hold `-`** (`-IN`, `A-`: 1706 pin entries,
  counted over `Parts/*.json`). The unit is looked for only **left** of the
  first dot, so `U1.-IN` is unaffected.

## 3. Design Overview

One parsing function, then each tool uses it:

```swift
/// "U1" or "U1-A": the part, and the unit when one is named.
func unit(_ spec: String) throws -> (chip: PlacedChip, key: String?)
```

Split `spec` at the first `-`. No `-` → `(chip(spec), nil)`. With one: the
left side through `chip(_:)`; the right side must be a unit key of that part
(case-insensitive), else
`U1 has no unit Z; units: A B C D P`; and the part must be split, else
`U1 is in Block view; set_part reference=U1 view=gate first`.

Risk is low: every changed tool keeps its existing path for a plain reference.

Byte-identity is not this letter's gate.

Rejected:

- *New tools (`move_unit`, `rotate_unit`).* The unit is an address, and the
  existing tools already take an address.

## 4. The tools

| Tool | Change |
|---|---|
| `set_part` | new property `view`: `block` or the part's view name (`gate`), case-insensitive. Calls `setPartSplit`. A part with no units: `R1 has no other view`. Reported in the list of changes as `view gate`. |
| `place_part` | new optional property `view`, same values; applied after placing, in the same undo step. The result lists the units. |
| `move_part` | `reference` through `unit(_:)`. With a key: target from that piece's size and origin, then `moveUnit`. Without: unchanged (a split part moves whole). Result: `U1-A now centred at (x,y)`. |
| `flip_part` | with a key: `flipUnit`. Without a key on a split part: `U1 is drawn as units; name one: U1-A U1-B U1-C U1-D U1-P`. |
| `rotate_part` | with a key: the piece's symbol must turn (`U1-P can't be rotated: it is a block`), then `rotateUnit`; the result gives the unit's new angle. Without a key on a split part: the same message as `flip_part`. The tool summary adds "or one gate of a part in Gate view (`U1-A`)". |
| pins (`connect`, `disconnect`, `route`, `read_pins`, `set_layer pin:` …) | `pin(_:)`: the left side through `unit(_:)`; with a key, the pin found must be one of that unit's pins, else `U1-A has no pin 6; its pins: 1=1A 2=1B 3=1Y`. Output stays `U1.3`. |
| `select` / `move` / `lock` (via `item`) | a spec with no `:` goes through `unit(_:)`; with a key the item is `.chip(id)` and `select` then calls `pick`. |
| `delete` (via `item(forDelete: true)`) | a spec with a key is refused: `U1-A is one symbol of U1; delete U1 to remove the part`. |
| `get_design` text | a split part's line gains `view=gate`; under it one line per unit: `  U1-A centre=(x,y) size=WxH body=(x1,y1)-(x2,y2)` plus `flipped` / `rot=90` when set, and with `geometry=true` each pin line is prefixed by its unit. |
| `get_design` JSON | a split part gains `"view": "gate"` and `"units": [{"unit","x","y","width","height","flipped","rotation","bounds","pins":[designators]}]`; a part that has units but is a block gains `"view": "block"`; other parts gain nothing. |
| `get_selection` | a selected split part with a pick entry prints its picked units: `U1 (U1-B)`. |

`AutomationGuide.text` gains, after the addressing paragraph: "A gate IC can be
drawn as separate gates: `set_part` `view=gate`. Its gates are then `U1-A`,
`U1-B`, …, and `U1-P` is the block with its power pins; move, flip and rotate
them one at a time. Pins are still `U1.3`. Deleting takes the whole part."

## Compatibility / Format Impact

- Agent API: two new optional properties (`view` on `set_part` and
  `place_part`); references may carry `-<unit>`; `get_design` gains lines and
  keys for split parts only. Every existing call and every output for a
  document without a split part is unchanged.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as
> the work; `- [~]` for partial; mark moot tasks; fill `Commit:`; add tasks you
> discover. **An unticked box means NOT DONE.**

### Phase 1 — the tools

- [x] `Automation/AutomationTools.swift`: `unit(_:)` (§3); `pin(_:)` through it.
      **Done:** also `unitNames(_:)`. A part with no units answers `R1 has no units: it is one symbol`.
- [x] `Automation/AutomationSchematic.swift`: `set_part` and `place_part`
      `view`; `movePart`; the `flip_part` and `rotate_part` closures;
      `item(_:_:forDelete:)`; `select` picking (§4).
      **Done:** `place_part` checks `view` before placing and wraps the placing and the view in one `Add Part` step (the first run showed two steps). `view=units` is accepted for any part with units.
- [x] `Automation/AutomationRead.swift`: `designText`, `designJSON`,
      `selectionText` (§4).
      **Done:** as landed the text form keeps the part's one `pins:` line (its tips are already each unit's) and adds one line per unit ending in `pins=<designators>`; a JSON unit also has `reference`.
- [x] `AutomationGuide.text` (§4).
- [x] Tests: new `schematicalTests/SplitPartAutomationTests.swift`, in the
      style of `AutomationTests` — `testSetPartView` (gate, then block; one
      undo step each; `R1` refused with its message); `testPlacePartWithView`;
      `testMoveOneUnit` and `testMoveWholeSplitPart`; `testFlipAndRotateAUnit`
      (and the three refusals: no key on a split part, `U1-P` rotate, a key on
      a block-view part); `testPinAddresses` (`connect` with `U1-A.3` and with
      `U1.3` make the same wire end; `U1-A.6` refused with its message);
      `testSelectPicksAUnit`; `testDeleteAUnitIsRefused`;
      `testGetDesignListsUnits` (text: `view=gate` and five unit lines; JSON:
      `units` with five entries whose `pins` partition the 14 designators);
      `testUnsplitOutputIsUnchanged` (the `get_design` text and JSON of a
      document with a resistor and an LED have no `view` and no `units`);
      `testBlockViewGateICSaysBlock` (JSON `"view": "block"`).
      **Done:** 12 cases with `testEndToEnd`. `XB -only-testing:schematicalTests/SplitPartAutomationTests -only-testing:schematicalTests/AutomationTests` → `Executed 47 tests, with 0 failures` (12 + 35), `** TEST SUCCEEDED **`.

Acceptance: an agent can do with tools everything the window can.
  Check: `XB -only-testing:schematicalTests/SplitPartAutomationTests -only-testing:schematicalTests/AutomationTests`
  → `** TEST SUCCEEDED **` (est. 6 min).
Commit: 3a2b41ef

### Phase 2 — docs

- [x] `docs/agent-control.md`: the `view` property in the `set_part` and
      `place_part` rows; the unit address in the `move_part`, `flip_part`,
      `rotate_part`, `select` and `delete` rows; the `get_design` additions;
      a bullet under "Conventions an agent needs" matching
      `AutomationGuide.text` word for word.
      **Done:** the doc lists tools in paragraphs, not rows: a **Gate view** paragraph after the Schematic tools, and the convention bullet with the guide's words. `grep -c 'U1-A' docs/agent-control.md` → 9.
- [x] `docs/file-format.md`: re-read the `units` section written in letter D
      against the code as landed; correct any drift.
      **Done:** re-read against `SchematicFile.swift` as landed: keys, defaults, the unmatched-list rule and the older-build note all match. No drift. The doc writes the key as `` `units` ``, so the check is `grep -c '`units`' docs/file-format.md` → 2 (see Corrections).
- [x] `docs/export.md`: the same re-read for letter F's paragraphs.
      **Done:** matches the writers as landed. `grep -ci 'multi-unit\|drawn as units' docs/export.md` → 3.
- [x] `planning/todo.md` § Schematic (line 121): add and tick
      `- [x] Gate view: a gate IC drawn as separate gates (plan-67).`
      **Done:** added in the main checkout after the merge, under § Schematic, and left uncommitted: the file already carries an uncommitted edit that is not this plan's (see Corrections), so the line waits there for the owner's commit of that file.
- [x] Move `planning/plan-67-A…G` to `planning/completed/` (each letter as it
      completes; this box covers the ones still in `planning/`).
      **Done:** all seven letters moved together with `git mv` once the last one's gate had passed; `planning/plan-67-data/` stays where the other plans' data folders are.

Acceptance: the docs name every new address, property and key.
  Check: `grep -c 'U1-A' docs/agent-control.md` → at least 4;
  `grep -c '"units"' docs/file-format.md` → at least 1;
  `grep -ci 'multi-unit\|drawn as units' docs/export.md` → at least 1 (est. 1 min).
Commit: 3a2b41ef (the docs); the archive and the todo line follow the landing gate

### Phase 3 — landing

The one landing gate of plan-67. Run once, after everything above.

- [x] Backend: `CT` → `100% tests passed, 0 tests failed out of 75`, and
      `git diff --stat backend/tests/golden` against the commit before
      plan-67-A → no output.
      **Done:** 2026-11-25, in `/tmp/p67w-build`: `100% tests passed, 0 tests failed out of 75`; `git diff --stat aefaebcd -- backend/tests/golden` (the commit before plan-67-A) → no output.
- [x] App, every class this feature added or touched, one `-only-testing:`
      each: `PartUnitTests`, `GateSymbolTests`, `SplitPartModelTests`,
      `SplitPartEditTests`, `SplitPartFileTests`, `SplitPartCanvasTests`,
      `SplitPartExportTests`, `SplitPartAutomationTests`, `SchematicRotateTests`,
      `SchematicClipboardTests`, `SchematicUndoTests`, `SelectionModelTests`,
      `SelectionGestureTests`, `SelectionSidebarTests`, `SchematicFileV5Tests`,
      `DisplayReferenceTests`, `KiCadExportTests`, `EasyEDAProExportTests`,
      `AutomationTests`, `ErcCompareTests` → `** TEST SUCCEEDED **`.
      **Done:** 2026-11-25, one `xcodebuild … test` with the 20 `-only-testing:` classes → `Executed 263 tests, with 0 failures (0 unexpected)`, `** TEST SUCCEEDED **`; no restart of the test host (log `/tmp/p67/landing.log`).
- [x] End to end through the tools, in one test
      (`SplitPartAutomationTests.testEndToEnd`): `place_part SN74HC00N`,
      `set_part view=gate`, `move_part U1-B` to a new centre,
      `rotate_part U1-B`, `connect U1-B.6` to an LED, save to data, open the
      data in a new document, `get_design` there equals `get_design` before
      the save, and `render` returns a PNG. With `TEST_RUNNER_P67_OUT=1` the
      test prints the PNG as one `P67_PNG <base64>` line; decode it to
      `/tmp/p67-render.png`, open it, and record in this line what it shows
      (five symbols, B turned, the wire on B's output pin, the labels).
      **Done:** `SplitPartAutomationTests.testEndToEnd` passes: `place_part` ×3, `set_part view=gate`, `move_part U1-B`, `rotate_part U1-B`, three `connect`s (one `U1-A.3` → `U1-B.4`), save to data, open in a second window, `get_design geometry=true` equal apart from the window line, unit placements equal, `render` returns one PNG. With `TEST_RUNNER_P67_OUT=1` the PNG was decoded to `/tmp/p67-render.png` (1476 × 794) and looked at: five symbols — `U1-A`, `U1-C`, `U1-D` as NAND gates in a column with their pin numbers, the `U1-P` block reading SN74HC00N with VCC (14) and GND (7), and `U1-B` moved right, turned to 90° with its inputs (5, 4) on top and its bubbled output (6) below, its label beside it; the wire from pin 3 of `U1-A` ends on pin 4 of `U1-B`, the wire from pin 6 runs to the LED's anode, the LED's cathode to the resistor. The wire from pin 6 crosses gate B's own body on its way up, as any wire that is not given waypoints may cross a part.
- [x] Every example still opens and saves unchanged:
      `XB -only-testing:schematicalTests/SplitPartFileTests/testExamplesAreUnchanged`
      → pass.
      **Done:** `SplitPartFileTests` is one of the 20 classes above: 7 tests, 0 failures, `testExamplesAreUnchanged` among them (10 files under `examples/`).
- [x] The full app suite runs in the background per the project's practice; it
      is not a step of this plan. Record its result here when it finishes.
      **Done:** run 2026-11-25 on the branch (`/tmp/p67/full.log`): `Executed 1998 tests, with 6 tests skipped and 12 failures` in 1286 s, 11 test cases. **Seven were this plan's** (the Part Editor wizard did not know the new `input` key: `KindGuideTests` ×3, `WizardStepsTests` ×3, `WizardModelTests` ×1); fixed and re-run green, see plan-67-A § Corrections. **Four are not this plan's**: `AcknowledgementsTests.testEveryLicenceIsInTheBundle` and `testNamesTheThirdPartyCode` (the `avr8js` entry) and `UserLibraryCatalogTests.testAddListsUnderUserLibraryAndWritesTheFile` and `UserPartLibraryTests.testWriteThenFilesReturnsTheFile` (a `/var` against `/private/var` path comparison); they are checked against the main checkout in the next task.

- [x] Added: the wizard fix found by the full suite: `GateGuide.swift` and three test expectations (plan-67-A § Corrections); `KindGuideTests`, `WizardStepsTests`, `WizardModelTests` → 29 tests, 0 failures. These three classes belong in the landing list above; with them it is 23 classes.
- [x] Added: the four failures that are not this plan's fail the same way on the main checkout (run the three classes there).
      **Done:** `xcodebuild -project /Users/justinzaun/Development/logic/schematical/schematical.xcodeproj … -derivedDataPath /tmp/p67-main-dd test -only-testing:schematicalTests/AcknowledgementsTests -only-testing:schematicalTests/UserLibraryCatalogTests -only-testing:schematicalTests/UserPartLibraryTests` → `Executed 22 tests, with 4 failures`: the same four cases with the same messages (`avr8js`; `file:///var/…` against `file:///private/var/…`). They are main's own and are left as they are.
- [x] Added: `main` compared with the branch's base before the gate: `git log --oneline HEAD..main` → no output (main is still `aefaebcd`, the plan's own commit), so the gate above stands for the merge and nothing was merged in.

Acceptance: the feature works end to end through save, open, export and the
agent, and nothing that does not use it changed.
  Check: the three commands above → their stated results (est. 25 min for the
  20 app classes in Debug; no smaller check covers "nothing else changed"
  across the model, the file, the canvas, the exports and the tools at once).
Commit: 1f5ed38a (the gate); the archive, the full-suite record and the todo line follow it

## Validation Plan

- Tests: `SplitPartAutomationTests` (12 cases) plus the landing list.
- Coverage check: the landing list names every test class whose production
  code a letter of this plan edits (model, file, canvas, exports, tools).
- Runtime proof: `testEndToEnd` and the picture it prints. The live window is
  the owner's to look at; it is not a gate of this plan.
- Doc sync: `docs/agent-control.md`, `docs/file-format.md`, `docs/export.md`,
  `docs/parts-format.md`, `docs/backend.md`, `planning/todo.md`.
- Final gate (run ONCE): Phase 3 above (est. 30 min with the backend).

## Decisions

- **The unit address is `U1-A`, and `U1-A.VCC`-style pin addresses are
  accepted** — the owner asked for a separator and offered `:`, `/`, `#`, `-`
  or none, 2026-11-25, leaving the choice to the author. `-` is the one of the
  four with no meaning in any agent parser and none inside a reference. (§2, §3)
- **Deleting a unit is refused, not widened to the part** — the author's call:
  the owner's rule is that delete takes the whole IC, and an agent that names
  one symbol should be told so rather than lose five.

## Corrections

- **The merge.** `main` had not moved since the worktree was made (`git log --oneline HEAD..main` → no output), so `git merge worktree-P-67` in the main checkout was a fast-forward from `aefaebcd` to `891a2e3c`, 13 commits, no conflict. The main checkout's uncommitted files (five `examples/*.schematical`, the deleted `examples/slow PCB.schematical`, `planning/todo.md`, two untracked CSV files) are none of the branch's files and were not touched. The follow-plan skill's formatter step names `cargo fmt`; this repository has no Rust, so there was nothing to run.

- **The plan files were archived on the branch, before the merge.** The follow-plan sequence archives after the work is integrated. The seven files were moved to `planning/completed/` in a commit on `worktree-P-67` once the landing gate had passed, so the merge brings the archive with it and main never holds the plan in both places. The two boxes still open at that moment (the full-suite record and the todo line) were finished in the archived file.

- **`place_part … view=gate` was two undo steps on the first run** (`testPlacePartWithView`: one undo left the part placed as a block). The tool now checks `view` first and does the placing and the view inside one `recordEdit(.schematic, "Add Part")`.
- **`get_design` text.** §4 said each pin line would be prefixed by its unit with `geometry=true`. The part has one `pins:` line for all its pins, whose tips are already each unit's own; prefixing would change its shape for readers that parse it. Each unit line carries `pins=<designators>` instead, with or without `geometry`.
- **The doc check for the file format.** `grep -c '"units"' docs/file-format.md` → 0: the table writes keys in backticks. The check that measures the same thing is `grep -c '`units`' docs/file-format.md` → 2.
- **`planning/todo.md` is edited in the main checkout, not on the branch.** The main working tree has an uncommitted edit of `planning/todo.md` that is not this plan's (`git status` in the main checkout: ` M planning/todo.md`). A commit of that file on `worktree-P-67` would make the merge into main refuse (`Your local changes to the following files would be overwritten`). The line is therefore added to the main checkout's file after the merge and left for the owner to commit with their own change.

## Summary

Low risk: one parser function and per-tool branches, all behind a `-` no
existing address can contain. The landing gate is the feature's only wide
check, and it is run once.
