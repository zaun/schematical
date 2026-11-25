# plan-63-E: The ERC in Swift: settings, the file, the checks

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-63-A, plan-63-D

Plan 63's overview and Prerequisites are in `planning/completed/plan-63-A-power-flag.md`.

The outcome of 63-E: **`document.electricalRulesCheck()` returns the violations
of a schematic drawing, by KiCad's rules**: the five pin rules from the backend
(63-D) and eleven drawing rules computed here (dangling wires and labels, No
Connect flags, similar and duplicate names, reference clashes), each with the
severity the document's settings give it, minus the exclusions. The settings
(a severity per rule, the 12×12 pin map, the excluded violations) are part of the
document: undoable and saved in the file. There is no window yet (63-F).

References:

- `planning/plan-63-data/README.md` (rule table, messages, counting rules),
  `probe_wires_labels.txt`, `probe_nets.txt`, `probe_names.txt`.
- `schematical/schematical/Simulation/Simulation.swift` (`NetlistBuilder.build`),
  `Simulation/VoltageCheck.swift` (the precedent: a check over chips and wires
  with the library), `DesignRuleCheck/DesignRuleCheck.swift` (a check's issue type),
  `Canvas/UndoHistory.swift` (`DocumentState`, `UndoKey`), `Document/SchematicFile.swift`,
  `docs/file-format.md`.
- `backend/include/erc.h` (63-B, 63-D).

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-63-D is complete | `cmake --build /tmp/logic-build --target erc_test >/dev/null && /tmp/logic-build/erc_test \| tail -1` → `0 failures` | NOT MET until D lands |
| plan-63-A is complete | `grep -c powerFlag schematical/schematical/Canvas/NetLabel.swift` → a number above 0 | NOT MET until A lands |
| The rest as plan-63-A § Prerequisites | its four commands | MET (2026-11-25) |

If plan-63-A or plan-63-D is not complete, this plan cannot start, full stop.

## 1. Goal

- `ErcRule` has the 16 implemented rules of the README, raw value KiCad's rule name,
  each with a title and KiCad's default severity (error, warning, ignore).
- `ErcSettings` holds the changes from default: `severities`, `pinMap`, `exclusions`.
  It is part of `DocumentState`: an edit is one undo step, and the file saves it.
- `SchematicDocument.electricalRulesCheck()` returns `[ErcViolation]`: rule, severity
  (never `ignore`), message (KiCad's text), the `SchematicItem`s it names, the
  point to mark, and a stable `key`.
- For the probe cases in the README the set of rules reported equals KiCad's.

### Non-goals (explicit constraints)

- No window, menu, marker or automation tool (63-F).
- No change to what an edit is allowed to do: the ERC never refuses an edit
  (the voltage check still does).
- The file `version` does not change; a file with no `erc` key is byte-identical
  to what is written today.
- Exports do not change here.

## 2. Current State

`NetlistBuilder.build(chips:wires:buses:labels:)` (`Simulation/Simulation.swift:76`)
numbers the nets and returns `connections` (a part, a pin designator and a net for
every pin a wire reaches) and `netOfWire`. It does not list unwired pins, wire
groups apart from label merging, or which pin has a No Connect flag. Wire ends are
`WireEnd`: `.pin`, `.junction`, `.busPin`, `.label` (`SchematicModel.swift:1303`).
A loose end is a junction with `isLoose` (`:1313`). A No Connect is a label with
`!isNet` (63-A: `isNoConnect`); a Power Flag a label with `isPowerFlag`.

`VoltageCheck.sortedIssues` is the precedent for calling the backend: it finds
each chip's `Part` with `part_library_find(library.pointer, chip.partNumber)`
and maps connections to pin indices through the designator.

`DocumentState` (`Canvas/UndoHistory.swift:10`) is the saved document as a value;
undo is a diff of two states by `UndoKey` (`:30`), `UndoDiff.changes` (`:84`), and
an apply (`:192`). `SchematicDocument.state` / `setState`
(`SchematicModel.swift:2155-2185`) and `replaceContents` (`:2092`) move a state in
and out; `setDesignRules` (`:4257`) is the precedent for a setting edit, through
`recordEdit(page, name)`.

The file: `SchematicFile`'s top-level struct (`Document/SchematicFile.swift:580-600`)
has optional fields (`labels`, `customParts`) written only when there is something;
labels are read with `label(_:)` (`:1316`). `docs/file-format.md` documents each.

### Measured populations

| What | Count | Command |
|---|---|---|
| Sites that handle every `UndoKey` / `UndoValue` | 5 | `grep -n 'board' schematical/schematical/Canvas/UndoHistory.swift` → `DocumentState` (`:18`), `UndoKey` (`:33`), `UndoValue` (`:46`), `changes` (`:128`), the apply (`:194`) |
| Constructions of `DocumentState(` | 16 | `grep -rn 'DocumentState(' schematical/schematical schematical/schematicalTests \| wc -l` (all use defaults for the new field) |
| Callers of `replaceContents(` | 13, and its definition | `grep -rn 'replaceContents(' schematical/schematical schematical/schematicalTests \| wc -l` → 14 lines (the definition `SchematicModel.swift:2092`, 1 in `SchematicFile.swift`, 12 in tests); the new parameter has a default, so none changes |
| Implemented ERC rules | 16 | `planning/plan-63-data/README.md` |

### Verified properties

- KiCad's counting and exemptions for the five pin rules are in 63-D.
- A label's shape never changes a result (`probe_label_shapes.txt`), so a
  net label's type is not read.
- A label and a power symbol of different names on one wire report nothing
  (`probe_names.txt`); two labels of different names, or two power symbols, report
  `multiple_net_names`.
- A label whose wire-connected group has no pin is `label_dangling`, also when
  another label of its name is elsewhere (`probe_wires_labels.txt`).

## 3. Design Overview

**Settings.** `ErcRule` (`String` raw, `CaseIterable`), `ErcSeverity`
(`error`, `warning`, `ignore`), `ErcSettings` (`Codable`, `Hashable`): `severities:
[ErcRule: ErcSeverity]` (only changes from default), `pinMap: [[UInt8]]?` (nil =
`ERC_DEFAULT_PIN_MAP`), `exclusions: Set<String>`. `severity(of:)`, `level(_:_:)`.

**The check.** `ErcChecker` builds, from one `NetlistBuilder.build` and its
own union-find over wire ends (`wire groups`: ends joined by wires only, no
merging by label name, flags and No Connect markers included), the input of
`erc_check`:

- every chip pin that is not `absent` is an `ErcPin`; its type from
  `erc_pin_type`, `drives_power` from `erc_pin_drives_power`, `net` from
  `connections` (or `SIZE_MAX`), `no_connect_mark` when its wire group holds a No
  Connect marker;
- every Power Flag with a wire is an `ErcPin` of type `power_out` on its wire's net;
  one without a wire is an unwired one;
- `ErcNet.has_attachment` for every net with a named label or a bus pin on one of
  its wires.

Its problems map back to a chip pin (or a flag), then to an `ErcViolation`
with `items [.chip(id)]` (a flag: `[.label(id)]`) and the pin's tip as the point.

The drawing rules (§ 1 README, all in Swift): `no_connect_dangling` (a marker whose
group has no pin), `wire_dangling` and `unconnected_wire_endpoint` (a wire end
on a junction that no other wire end is on; `wire_dangling` when both ends are),
`label_dangling` (a named label whose group has no pin and no bus pin),
`isolated_pin_label` (a net, labels merged by name, with a named label and
exactly one pin), `multiple_net_names`, `similar_labels` (names equal ignoring
case but not equal), `similar_label_and_power` (a label and a power symbol's part
number so), `single_global_label` (a name on one label only), `four_way_junction`
(a junction with four wire ends), `duplicate_reference`.

`key`: the rule, `|`, and the sorted ids of what it names (`chip:<uuid>:<designator>`,
`wire:<uuid>`, `label:<uuid>`, `junction:<uuid>`, `part:<uuid>`). It survives
edits elsewhere, so an exclusion stays on its violation.

Where the risk concentrates: the mapping from the drawing to `erc_check`'s input
(an off-by-one on pin designators puts a problem on the wrong pin), and
`wire group` vs net (label merging must not hide a dangling label). The
per-rule tests are built from the README's probe rows.

Rejected: reusing `NetlistBuilder`'s nets for the drawing rules (they merge by
label name, which is exactly what `label_dangling` must not see). Storing the
settings on `BoardSettings` (they are not the board's, and `board` is an
undo key of the PCB page).

## Compatibility / Format Impact

New optional top-level key `erc`, written only when a severity, the pin map or an
exclusion differs from default:

```
"erc": {"severities": {"pin_not_connected": "warning"},
        "pinMap": [[0,0,…] × 12],
        "exclusions": ["pin_not_connected|chip:6F1C2E8A-…:3"]}
```

`severities` keys are rule names and values `error`, `warning` or `ignore`;
`pinMap` is 12 rows of 12 numbers 0, 1 or 2, symmetric; `exclusions` are keys as
above. An unknown rule name, a bad severity word, or a pin map that is not 12×12
symmetric 0–2 is refused (`SchematicFileError.badErc`, the message names the
field). An older build ignores the key (`docs/file-format.md:198`: "Older builds ignore it", of `designRules`).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as
> the work; `- [~]` with one line on what remains; moot tasks keep their line
> as `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` as a phase lands. An
> unticked box means NOT DONE.

### Phase 1 — Settings, undo and the file

- [ ] New `schematical/schematical/ElectricalRulesCheck/ErcSettings.swift`:
      `ErcRule` (16 cases, `title`, `defaultSeverity`: error for `pin_not_connected`,
      `pin_not_driven`, `power_pin_not_driven`, `wire_dangling`, `label_dangling`,
      `duplicate_reference`; warning for `no_connect_connected`, `no_connect_dangling`,
      `unconnected_wire_endpoint`, `isolated_pin_label`, `multiple_net_names`,
      `similar_labels`, `similar_label_and_power`; `pin_to_pin` has no severity of its own: the pin
      map's cell is the severity, 1 warning and 2 error, 0 off; ignore for `single_global_label` and
      `four_way_junction`), `ErcSeverity`, `ErcSettings`, and the default pin map as
      `[[UInt8]]` read from `ERC_DEFAULT_PIN_MAP`.
- [ ] `Canvas/UndoHistory.swift`: `erc` in `DocumentState`; `UndoKey.erc`,
      `UndoValue.erc(ErcSettings)`; the diff and the apply, as `.board` has them.
      `Canvas/SchematicModel.swift`: `private(set) var erc = ErcSettings()`, in
      `state` / `setState` / `replaceContents(…, erc: ErcSettings = ErcSettings())`;
      `setErcSeverity(_:_:)`, `setErcPinMap(_:)`, `resetErcSettings()`,
      `excludeErc(_ key:)`, `includeErc(_ key:)`, each `recordEdit(.schematic, "Change
      ERC Settings" / "Exclude Violation")`.
- [ ] `Document/SchematicFile.swift`: the `erc` key per § Compatibility, read and
      written (default settings write nothing); `badErc`. `docs/file-format.md`: a
      section "Electrical rules check" beside "Board design rules", and the top-level
      table row.
- [ ] Tests (new `schematicalTests/ErcSettingsTests.swift`): defaults for the 16 rules
      match the README table; an edit is one undo step and redo restores it; the file
      round trip keeps severities, a changed pin map and exclusions; a default
      document writes no `erc` key (the JSON has none); each bad input is refused
      with a message naming the field; opening a file with no `erc` gives defaults.

Acceptance: settings edit, undo, save and reopen.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/ErcSettingsTests -only-testing:schematicalTests/UndoHistoryTests -only-testing:schematicalTests/SchematicUndoTests -only-testing:schematicalTests/FileDesignRulesTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 8 min).
Commit: —

### Phase 2 — The pin rules on a drawing

- [ ] New `ElectricalRulesCheck/ErcChecker.swift`: `ErcViolation`, the wire-group
      union-find, and the call to `erc_check` per § 3 (types from `erc_pin_type`;
      the pin map from the settings), mapping each problem to a violation with
      KiCad's message (README § messages). `SchematicDocument.electricalRulesCheck()`
      returns them ordered by severity (errors first), then rule, then position
      (y, then x), and drops `ignore` rules and excluded keys.
- [ ] New `schematicalTests/ErcCases.swift`: one named function per case, each returning
      a `SchematicDocument` built with `TestSupport`; 63-G exports the same 15. This
      phase adds the five pin cases (`pinConflict`, `pinNotConnected`, `pinNotDriven`,
      `powerPinNotDriven`, `noConnectConnected`); Phase 3 adds the ten others
      (`noConnectDangling`, `wireDangling`, `labelDangling`, `isolatedPinLabel`,
      `multipleNetNames`, `similarLabels`, `similarLabelAndPower`, `singleLabel`,
      `fourWayJunction`, `duplicateReference`). The tests below use them.
- [ ] Tests (new `schematicalTests/ErcPinChecksTests.swift`): drawings built with
      `TestSupport`, the five cases above among them: an unwired `SN74HC00N` input reports `pin_not_connected`; a
      `5V` + `SN74HC00N` VCC wire reports nothing for VCC; a `SN74HC00N` output
      wired to a second output reports `pin_to_pin` (error); two `SN74HC244N`
      outputs wired together report nothing and a `SN74HC244N` output with a
      `SN74HC00N` output reports `pin_to_pin` (warning); `VCC` of an IC wired to nothing
      but a `GND`-less net reports `power_pin_not_driven`; a 12 V connector pin to
      an IC's VCC reports it, and a Power Flag on the wire clears it; a No Connect
      on an unused input clears `pin_not_connected`; a No Connect on a wired pin
      reports `no_connect_connected`; a pin map edit to 0 silences `pin_to_pin`;
      severity `ignore` drops a rule; an exclusion hides one violation and keeps
      the next.

Acceptance: pin rules agree with the README on the drawings above.
  Check: `xcodebuild test … -only-testing:schematicalTests/ErcPinChecksTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

### Phase 3 — The drawing rules

- [ ] `ErcChecker.swift`: the eleven drawing rules of § 3, with KiCad's messages
      (`duplicate_reference`: `Duplicate reference <R>`), the points and items each
      names, and the keys.
- [ ] Tests (new `schematicalTests/ErcDrawingChecksTests.swift`): one case per
      rule from `probe_wires_labels.txt`, `probe_names.txt` and `probe_nets.txt`:
      a wire loose at both ends (`wire_dangling` + 2 `unconnected_wire_endpoint`);
      a pin → wire → loose end (`unconnected_wire_endpoint`); a label alone and a
      label on a wire with a loose end (`label_dangling`); two same-name labels on pinless
      wires (two `label_dangling`); labels `C1` and `C2` on one wire
      (`multiple_net_names`); `CLK` and `clk` (`similar_labels`); a label `gnd` and
      a `GND` symbol (`similar_label_and_power`); one pin + one label
      (`isolated_pin_label`) and two pins + one label (nothing); a No Connect
      alone (`no_connect_dangling`); a name on one label with the rule on
      (`single_global_label`); a junction with four wire ends (`four_way_junction`)
      and with three (nothing); two chips with one reference (`duplicate_reference`,
      made by editing the chip array in the test, since `renameChip` refuses it).
      the `AFU` fixture (`Fixture.open("AFU")`, `schematicalTests/Fixtures/`)
      checked completes, reports no `duplicate_reference`, and the time it took is
      recorded in the Result line below.

Acceptance: each rule fires on its case and not on its near miss; the example runs.
  Result (recorded when run): the `AFU` fixture: violations by rule, and seconds: (filled when run).
  Check: `xcodebuild test … -only-testing:schematicalTests/ErcDrawingChecksTests -only-testing:schematicalTests/ErcPinChecksTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 8 min).
Commit: —

## Validation Plan

- Tests: `ErcSettingsTests`, `ErcPinChecksTests`, `ErcDrawingChecksTests` (new), plus the undo and file classes of Phase 1.
- Coverage check: `grep -c 'case ' schematical/schematical/ElectricalRulesCheck/ErcSettings.swift` shows 16 rules and each has a test case (one assertion per rule in `ErcDrawingChecksTests`/`ErcPinChecksTests`).
- Runtime proof: the AFU fixture run of Phase 3 (the owner's real-window pass is 63-F's).
- Doc sync: `docs/file-format.md`.
- Final gate (once, after Phase 3): the three new classes with `UndoHistoryTests`, `SchematicUndoTests`, `FileDesignRulesTests`, `NetLabelTests`, `PowerFlagTests` → `** TEST SUCCEEDED **` (est. 14 min).

## Decisions

- Settings live on the document, undoable, in the file under `erc` — decided by the owner (full parity, 2026-11-25).
- `same_local_global_label` is not implemented — decided by the author, 2026-11-25: a label named like a rail joins it on purpose (README).
- A net label's type is never read by the ERC — decided by the author, 2026-11-25, from `probe_label_shapes.txt`.

## Corrections

(Filled in during execution.)

## Summary

The risk is the drawing-to-input mapping and the wire group vs net distinction;
the per-rule tests are KiCad's probe rows rebuilt as drawings. Untouched: the
voltage check, simulation, exports and the PCB.
