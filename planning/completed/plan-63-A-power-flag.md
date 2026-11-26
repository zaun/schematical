# plan-63-A: The Power Flag net label type

Last updated: 2026-11-25
Overall Effort: x-large (1d–3d)
Effort: medium (1h–2h)
Depends on: nothing

Plan 63 adds KiCad's electrical rules check (ERC) to the schematic view. Seven
sub-plans, in landing order: **A** the Power Flag, a sixth net label type
beside No Connect (this plan); **B** the pin types: the backend derives a
KiCad pin type for every pin of every part (plain kinds, and the tri-state hook
for the buffer, latch and dff kinds); **C** the tri-state hook for the
remaining kinds; **D** the backend's pin checks (`erc_check`: pin-to-pin map,
pin not connected, pin not driven, power pin not driven, No Connect);
**E** the drawing checks in Swift, the settings (severities, the pin map,
exclusions) and their place in the file; **F** the window, the menu, the
markers on the schematic, exclusions and the automation tool; **G** the
KiCad cross-check (`kicad-cli sch erc` against our result on exported
schematics), the exporter's pin types, and the landing gates.

The outcome of 63-A: **a net label can be a Power Flag.** It is placed with
the net label tool like a No Connect (the sidebar's Type list gains "Power
Flag"), drawn as a small flag on its point, saved in the file, and exported to
KiCad as `PWR_FLAG`. It names nothing and joins nothing but the wire that ends
on it, so no net changes. Plan 63-D reads it: a net with a flag is driven (KiCad's
meaning); nothing in 63-A reads it for that.

References:

- `planning/plan-63-data/README.md` (the KiCad facts every sub-plan rests on),
  `probe_pin_matrix.txt`, `probe_wires_labels.txt`.
- `docs/file-format.md` § Net labels (line 585 on), `docs/export.md`.
- `schematical/schematical/Canvas/NetLabel.swift`, `Simulation/Simulation.swift`
  (`NetlistBuilder.build`), `Document/KiCadSchematicWriter.swift`,
  `Drawer/NetLabelPanel.swift`.

## Prerequisites

These are a precondition on the whole of plan 63, stated once here; B–G point here.

| Must be true | Command | Status |
|---|---|---|
| KiCad 10.0.8 is installed | `~/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli version` → `10.0.8` | MET (`10.0.8`, re-run 2026-11-25 at execution) |
| Xcode 26 | `xcodebuild -version \| head -1` → `Xcode 26.3` | MET (`Xcode 26.3`, re-run at execution) |
| The backend configures | `cmake -S backend -B /tmp/logic-build >/dev/null && echo ok` → `ok` | MET (`ok`, re-run at execution, in `/tmp/logic-build-wt63`: see Corrections 1) |
| No other plan 63 is on main | `git log origin/main --oneline -- 'planning/plan-63*' \| head -1` → no output until this plan's own commit | MET (no output, re-run at execution) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites.**

## 1. Goal

- `NetLabel.Style` has a sixth case, `.powerFlag`; a label of that style is
  drawn as a flag on its point, selectable, movable, deletable, copyable, saved
  as `"type": "powerFlag"` and read back.
- A Power Flag changes no net: the netlist of a drawing is the same with the flag
  as without it, except that the flag's point is one more end of the wire it is on.
- The KiCad export writes each flag as a `PWR_FLAG` power symbol whose pin has
  type `power_out`.

### Non-goals (explicit constraints)

- No ERC yet; nothing reads a flag for a rule (63-D).
- A No Connect keeps its meaning exactly: it joins nothing, not even the
  wires that end on it (`NetLabel.swift:12`, `Simulation.swift:95`).
- Named net labels keep theirs: the name joins labels of one name (case
  sensitive).
- The file `version` stays what it is (4, or 5 with custom parts). A file
  that holds a flag is refused by an older build with "The net label is not
  valid" (`SchematicFile.label`), the same as any unknown label type.
- EasyEDA Pro export: a flag is left out (that format has no such symbol).

## 2. Current State

`NetLabel.Style` (`Canvas/NetLabel.swift:17`) has `input`, `output`, `inout`,
`passive` and `noConnect`; `isNet` is `self != .noConnect` (`:40`) and means
"a named net label", so a No Connect is the one label with no name drawn and
no joining. The drawer's Net Label panel (`Drawer/NetLabelPanel.swift`) lists
`Style.allCases` as the Type choices; `addNetLabel(at:style:)`
(`Canvas/SchematicModel.swift:2961`) places a label of the chosen style and
gives it a name (`N1`, …) that a No Connect keeps but never shows.

The netlist: `NetlistBuilder.build` (`Simulation/Simulation.swift:76`)
unions the two ends of every wire, except a wire with an end on a No Connect
(`:95-102`, "A No Connect joins nothing"), then unions every net label of one
name (`:128`). A label is a `WireEnd.label(id)`.

The file: `SchematicFile.label` (`Document/SchematicFile.swift:1316`) reads
`type` through `NetLabel.Style(rawValue:)` and refuses anything else with the
message `must be input, output, inout, passive or noConnect`; the labels are
written with `label.style.rawValue` (`:832`). `docs/file-format.md:585-600`
documents the `type` values and the `"noConnect"` paragraph.

The KiCad export: `KiCadSchematicWriter` writes a net label for each
`isNet` label (`:580`), supply symbols as `power` library symbols with a
hidden pin (`powerInstance`, `:432-480`, reference `#PWR<n>`), and pins with
`pinType` (`:267`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Lines that read `isNet` | 20, in 9 files | `grep -rn isNet schematical/schematical \| wc -l` → 20; `grep -rln isNet schematical/schematical \| wc -l` → 9 |
| Of them, lines where `!isNet` / `else` means "this is a No Connect" | 10 | `Simulation.swift:95`; `NetLabel.swift:86,129,173`; `NetLabelPanel.swift:21,23,51`; `SelectionList.swift:40,99`; `KiCadSchematicWriter.swift:580` |
| Lines about joining by name (stay `isNet`) | 7 | `Simulation.swift:128`; `Ratsnest.swift:393`; `SchematicModel.swift:2987`; `KiCadExport.swift:221,244`; `EasyEDAProSchematicWriter.swift:290`; `NetLabelPanel.swift:16` |
| The remaining one | 1 | `SchematicModel.swift:3010`, "only becoming or leaving No Connect changes what joins what" → `isNoConnect` |

(10 + 7 + 1 + the two definitions `NetLabel.swift:40,49` = 20.)

### Verified properties

- A flag needs no netlist change: with `isNet` false it would be taken for a
  No Connect at `Simulation.swift:95` (`!$0.isNet`), which would stop the wire
  joining. The fix is to test `style == .noConnect` there (this plan's task),
  after which a wire ending on a flag unions its two ends like any wire. Read:
  `NetlistBuilder.build` lines 95-102 and `netOfWire` (`:157-163`).
- A label in a file with `type` missing is `.input`; a new value only widens
  `Style(rawValue:)` (read: `SchematicFile.swift:1323-1330`).
- KiCad's own `PWR_FLAG` is a power symbol with one `power_out` pin;
  `probe_pin_matrix.txt` shows `power_in` + `power_out` on a net reports nothing and
  `power_in` + anything else reports `power_pin_not_driven`.

## 3. Design Overview

`Style.powerFlag` with two predicates: `isNoConnect` (`self == .noConnect`) and
`isPowerFlag`. `isNet` becomes `self != .noConnect && self != .powerFlag`,
i.e. still "a named net label". The ten lines that mean "No Connect" are
split: the netlist, the type-change check and the KiCad writer's `(no_connect` use `isNoConnect`; the drawing,
the panel and the selection list treat both marker styles
(no name) and say which.

The drawing: a stem of 20 units up from the point and a solid pennant 14
wide, 10 high, at the stem's top, in the label colour. A flag has no name and no
flip. Its hit area is the same square as a No Connect's (`crossHit`).

Correctness risk: low and local (the netlist line). The check against
regression is that a drawing's netlist with a flag equals the one without
(Phase 1 test). Byte-identity is not a gate: a file with no flag is unchanged by
construction (nothing writes `powerFlag` unless a flag exists).

Rejected: a separate `PowerFlag` type next to `NetLabel` (the owner chose "in the
net label options list, like a No Connect"; a separate type would need its
own selection item, undo, copy and file array). A `WireEnd` case of its own
(same cost, no gain).

## Compatibility / Format Impact

`type` gains `"powerFlag"`. A flag still stores `name` (ignored, as for a No
Connect). Nothing else in the file changes; a file without a flag is
byte-identical. The KiCad export gains `PWR_FLAG` symbols and `#FLG<n>`
references; a design without a flag exports as before.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` with one line on what remains; a moot task is
> `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands;
> add any task you discover. An unticked box means NOT DONE.

### Phase 1 — The style, the drawing, the file

The flag exists in the editor and the file; the exports do not know it yet.

- [x] `Canvas/NetLabel.swift`: add `case powerFlag` to `Style` (title "Power
      Flag"); add `isNoConnect` and `isPowerFlag` on `Style` and on `NetLabel`;
      make `Style.isNet` `self != .noConnect && self != .powerFlag`. Draw a
      flag as § 3 says in the code paths at `:86` (bounds), `:129` (outline)
      and `:173` (drawing), beside the No Connect's X; a flag's `bounds` is
      the stem and pennant, its hit area the No Connect's square.
- [x] `Simulation/Simulation.swift:95`: `noConnects` is the labels with
      `isNoConnect`.
- [x] `Canvas/SchematicModel.swift:3010`: compare `isNoConnect`, not `isNet`
      (turning a net label into a flag does not join or split nets, so it needs
      no voltage check; turning a No Connect into a flag joins its wire's two ends,
      so it does).
- [x] `Drawer/NetLabelPanel.swift:21,23,51`: the title is "Power Flag" for a flag;
      a flag shows no name field, no flip, and no "joins N labels" line.
- [x] `Canvas/SelectionList.swift:40,99`: the row text of a flag is "power
      flag" (a No Connect's is "no connect").
- [x] `Document/SchematicFile.swift:1316-1330`: accept `powerFlag`; the error text
      lists all six types.
- [x] `docs/file-format.md` § Net labels: the `type` row gains `"powerFlag"`; a
      paragraph after the `"noConnect"` one says a flag is drawn as a flag,
      names nothing, joins nothing but its own wire, marks a net as driven for
      the ERC (63-D), and that an older build refuses the file.
- [x] Tests, new `schematicalTests/PowerFlagTests.swift` (conventions:
      `TestSupport`, as `NetLabelTests`): placing a flag; moving, deleting,
      copy and paste, undo and redo keep it; `SchematicFile` round trip writes
      `"type": "powerFlag"` and reads it back; a bad type's message names all six;
      `NetlistBuilder.build` of a drawing (two pins wired, a flag on the wire) has the same
      `netCount` and the same `connections` as without the flag, and a
      No Connect on the same wire still splits it; a document with no flag writes no `powerFlag` text, and the existing `NetLabelTests` file round-trip tests stay green unchanged.

Acceptance: a flag can be placed, saved, reopened and undone; no net changes;
a No Connect still joins nothing.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/PowerFlagTests -only-testing:schematicalTests/NetLabelTests -only-testing:schematicalTests/SchematicClipboardTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 7 min, most of it the build).
  Result: `PowerFlagTests` 6 tests, `NetLabelTests` 18, `SchematicClipboardTests` 14: `Executed 38 tests, with 0 failures`, `** TEST SUCCEEDED **` (2026-11-25, in the worktree; a flag's rect is the stem, pennant and the No Connect's square together, so a click on the drawn flag hits it).
Commit: c8ed577c

### Phase 2 — The exports

- [x] `Document/KiCadSchematicWriter.swift`: each flag is written as a power
      symbol of library name `PWR_FLAG`, value `PWR_FLAG`, reference
      `#FLG<n>` (numbered from 1 in the order of `netLabels`), with one
      hidden `power_out` pin at the flag's point and a flag drawing; defined once
      in `lib_symbols` and instanced as `powerInstance` does for rails
      (`:432-480`). In `labels()` (`:575-590`) the `else` branch that writes
      `(no_connect …)` becomes `else if label.isNoConnect`, so a flag is not
      written as a `(no_connect` or a `(label`.
- [x] No change in `Document/KiCadExport.swift:221,244` (the label ranking and the
      `labelled` set take `isNet` labels, which a flag is not) or in
      `Document/EasyEDAProSchematicWriter.swift:290` (it takes `label.isNet`
      too, so a flag is skipped); the tests below pin both. `Help/KeyboardHelp.swift:100`
      names no label type, so it needs no change either.
- [x] `docs/export.md` § KiCad: a flag becomes `PWR_FLAG`.
- [x] Tests in `KiCadExportTests`: a two-pin net with a flag exports one `PWR_FLAG`
      library symbol and one `#FLG1` instance, a `(pin power_out`, no `(label` for the flag;
      the export of a design with no flag is unchanged (an existing test
      asserting it stays green); in `EasyEDAProExportTests`: a drawing with a flag exports the same as without.

Acceptance: the KiCad export carries each flag as `PWR_FLAG`; nothing else in any export changes.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/EasyEDAProExportTests -only-testing:schematicalTests/PowerFlagTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 7 min).
  Result: `KiCadExportTests` 21, `EasyEDAProExportTests` 11, `PowerFlagTests` 6: `Executed 38 tests, with 0 failures`, `** TEST SUCCEEDED **` (2026-11-25). The KiCad test: the flag is one `(lib_id "Schematical:PWR_FLAG")`, one `(pin power_out line`, reference `#FLG01`, no `(no_connect`, no `(label `; the EasyEDA test: an unwired flag leaves every `.esch` and `.epcb` file byte-equal.
Commit: abf06e54

## Validation Plan

- Tests: `PowerFlagTests` (new), `NetLabelTests`, `SchematicClipboardTests`, `KiCadExportTests`, `EasyEDAProExportTests`.
- Coverage check: the ten "No Connect" lines are the ones the new tests drive; `grep -rn 'isNet' schematical/schematical | wc -l` is re-run and each remaining line is a joins-by-name line (§ 2's table).
- Runtime proof: place a flag from the Type list in the running app, save, reopen, undo (the owner's real-window pass; the unit tests cover the same steps).
- Doc sync: `docs/file-format.md`, `docs/export.md`.
- Final gate (once, after Phase 2): the two Phase checks together (est. 10 min). The ERC plans' own landing gate is 63-G's.

## Decisions

- The Power Flag is a net label type, added to the Type list like a No Connect — decided by the owner, 2026-11-25. It settles where the user marks an externally fed rail as driven.
- A supply symbol and a regulator's output are drivers without a flag — decided by the author (that is what the owner's choice of a flag was asked against, § 2 of the question put to them), 2026-11-25. See 63-B and 63-D.
- The file version does not change — decided by the author, 2026-11-25: the file-format policy is optional fields and new values keep the version (`docs/file-format.md:161,163`, the `showReference` and `socket` rows: "An optional field, so the version stays 4").

## Corrections

1. **Build directory.** Every cmake command of plan 63 names `/tmp/logic-build`; its cache was made from the main checkout (`CMake Error: … does not match the source`), so in the integration worktree they run with `/tmp/logic-build-wt63` instead (`cmake -S backend -B /tmp/logic-build-wt63`). Same commands, other directory.
2. **Phase 1's "same `connections` as without the flag" test.** With the flag wired to a pin that already has a wire, `connections` and `netCount` are unchanged (`testAFlagOnAWireChangesNoNet`); the "a No Connect on the same wire still splits it" claim only holds when the flag is a joint between two pins, so the test for it is `testAFlagJoinsItsWiresAndANoConnectDoesNot` (K and A wired to one flag: one net; as a No Connect: two).

3. **The KiCad export already wrote `PWR_FLAG`s.** § 2 said the writer has supply symbols and pins; it also writes one `PWR_FLAG` per rail (`KiCadSchematicWriter.flagInstance`, `power()` in `text()`: "one power flag on each rail"), numbered `#FLG01…` by rail. A drawing's own flags reuse that symbol and `flagInstance` and are numbered after the rails' (`flagged.count` + 1…); the test asserts `#FLG01` on a drawing with no rail. Consequence for plan-63-G (§ 2: "a rail symbol gets `power_pin_not_driven` in KiCad unless a PWR_FLAG is on the net"): an exported design already has a flag per rail, so KiCad does not report it, and instead reports `pin_to_pin` power_out + power_out where that flag shares a net with a regulator's output pin. G's comparison is re-planned in G's Corrections when G starts.

## Summary

The risk is one line: the netlist's No Connect test. Everything else is a
new case in an enum and the places that list the cases. Untouched: every
net, every export of a design without a flag, the file version.
