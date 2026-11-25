# plan-63-G: Check our ERC against KiCad's, and land it

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-63-F

Plan 63's overview and Prerequisites are in `planning/completed/plan-63-A-power-flag.md`.

Plan 63 says every check is "based on KiCad". 63-A to 63-F built it from probes
of `kicad-cli`; 63-G holds the result to KiCad directly: the KiCad export
writes the same pin types the ERC uses, a test prints our violations for 3 fixture
schematics and 15 drawings built to trip each rule, and a script runs
`kicad-cli sch erc` on the exported KiCad projects and compares. The outcome
of 63-G: **on those 18 designs our ERC and KiCad's report the same violations**, except
where `scripts/erc-compare.py` names a difference with its reason; then the plan
lands (docs, the todo, the gates).

References:

- `planning/plan-63-data/README.md` (rules, deliberate differences).
- `schematical/schematicalTests/KiCadExportTests.swift` (`testPrintSamplesWhenAsked`,
  `KICAD_OUT`), `FixtureSupport.swift` (`Fixture.names`, `Fixture.open`),
  `ErcCases.swift` (63-E), `docs/export.md`.
- The test host is sandboxed, so its output reaches `kicad-cli` as printed lines:
  `KiCadExportTests.swift:381-385` explains `KICAD_FILE`, and an environment
  variable reaches the host with a `TEST_RUNNER_` prefix (`TEST_RUNNER_KICAD_OUT=1`).

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-63-F is complete | `grep -c 'run_erc' schematical/schematical/Automation/AutomationRead.swift` → a number above 0 | MET (`1`, 2026-11-25) |
| The rest as plan-63-A § Prerequisites | its four commands | MET (2026-11-25) |

If plan-63-F is not complete, this plan cannot start, full stop.

## 1. Goal

- `KiCadSchematicWriter` writes each library pin's type from `erc_pin_type`
  (`passive`, `tri_state`, `open_collector`, `power_in`, … as KiCad names them),
  so KiCad's own ERC on an export sees what ours sees.
- `scripts/erc-compare.py` runs `kicad-cli sch erc` over the exports of 18 designs,
  compares with our violations and exits 0 when they match up to its written
  list of deliberate differences.
- Plan 63 is documented, its todo item ticked, its gates run.

### Non-goals (explicit constraints)

- No new rule, and no change to the ERC's meaning, unless the comparison shows
  our rule departs from KiCad's (then the correction is made in the rule's own
  sub-plan's file, recorded in this plan's Corrections).
- Supply symbols stay `power_in` in the export (KiCad's own convention); the
  script accounts for the difference (§ 3).
- The ERC's real-window pass is the owner's, not a gate of this plan.

## 2. Current State

`KiCadSchematicWriter.pinType(_ role: ChipPinRole)` (`Document/KiCadSchematicWriter.swift:267`)
maps the role: `input`, `output`, `inOut` → `bidirectional`, `power` and `ground` →
`power_in`, `noConnect` → `no_connect`, `absent` → `passive`; a two-terminal part's
far pin is `passive` (`:154,319`). No test pins these strings (`grep -n
'bidirectional\|power_in' schematical/schematicalTests/KiCadExportTests.swift` → no match).

`KiCadExportTests.testPrintSamplesWhenAsked` (`:386`) prints `KICAD_FILE <name>
<base64>` for each fixture project when `KICAD_OUT` is set (run with `TEST_RUNNER_KICAD_OUT=1`,
`xcodebuild … test`); `Fixture.names` is `["AFU", "LFU", "RegFile"]`.

From the probes, KiCad's ERC on an export differs from ours in these ways, all
deliberate:

- A rail symbol (`5V`, `GND`) is `power_in` in KiCad and gets `power_pin_not_driven`
  unless a `PWR_FLAG` is on the net; here a supply symbol drives its own net (63-B § 3).
- KiCad reports `same_local_global_label` for a label named like a rail (supported
  here), `lib_symbol_issues` (the export's library is not installed) and
  `endpoint_off_grid` (mm rounding), none of which the ERC has.
- `kicad-cli` 10.0.8 reports no `duplicate_reference` (README).

## 3. Design Overview

Compared by rule, over the 16 implemented rules only:

- Pin rules (`pin_to_pin`, `pin_not_connected`, `pin_not_driven`,
  `power_pin_not_driven`, `no_connect_connected`): the multiset of `(rule,
  reference.pin)` (for `pin_to_pin` the sorted pair), taken from KiCad's item
  descriptions (`Symbol U2 Pin 14 [VCC, …]`) and from our violations' references.
  KiCad's `pin_not_driven` and `power_pin_not_driven` on a pin of a net that has a
  rail symbol are removed from KiCad's list (our ERC counts a rail symbol as a
  driver); which nets have one comes from our side (the test prints, per violation
  list, the set of pins on rail nets).
- All other rules: the count per rule.
- Dropped from KiCad's list: `lib_symbol_issues`, `endpoint_off_grid`,
  `same_local_global_label`, and every rule that is not one of the 16 (all are
  default-off or N/A in the README).

The 18 designs: the three fixtures (`Fixture.names`) and 15 drawings from
`ErcCases` (63-E): one per rule, the two wire rules (`wire_dangling`,
`unconnected_wire_endpoint`) sharing one (15 = 16 rules − 1). `duplicate_reference`
has no KiCad counterpart: its case is in the list and its KiCad count is 0 by the
whitelist.

Risk: the comparison finds departures we cannot predict. That is its purpose;
each is fixed or whitelisted with a reason in this plan's Corrections, and the
script's whitelist file is under review in the commit.

Rejected: comparing every `examples/*.schematical` (they change as the owner works
on them: `git status` at the start of this plan showed 6 modified or deleted, so
a comparison on them would not repeat; the fixtures are frozen copies);
a custom KiCad symbol library to give rails a `power_out` pin (KiCad joins power
symbols by name, so every symbol of one rail would conflict).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as
> the work; `- [~]` with one line on what remains; moot tasks keep their line
> as `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` as a phase lands. An
> unticked box means NOT DONE.

### Phase 1 — The KiCad export writes the ERC pin types

- [x] `Document/KiCadSchematicWriter.swift`: `pinType(of chip:, slot:)` returns the
      KiCad word from `erc_pin_type` + `erc_pin_type_name` (the library part found as
      `VoltageCheck` does, `part_library_find`), `no_connect` for a no_connect pin, and
      `passive` for a pin the part gives no type (absent); the `define(_:name:in:)`
      call at `:315` uses it. The far pin and the connector's pins stay `passive`;
      rail symbols stay `power_in`.
- [x] Tests in `KiCadExportTests`: a `SN74HC244N` exports its `Y` pins as `tri_state`
      and its `A` pins as `input`; a `SN74LS07N` output as `open_collector`;
      a `GENERIC-RESISTOR` / `GENERIC-LED` / a header pin as `passive`; an
      `ATMEGA328P-PU` GPIO as `bidirectional`; a `LM7805MPX/NOPB` output as `power_out`;
      the fixtures still export (the existing tests pass).
- [x] `docs/export.md` § KiCad: a pin's type is the ERC pin type (the table of
      `docs/backend.md` § ERC pin types).

Acceptance: the export carries the ERC pin types.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/KiCadExportTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 7 min).
  Result (2026-11-25, in the worktree): `KiCadExportTests` `Executed 22 tests, with 0 failures`, `** TEST SUCCEEDED **` (the new `testLibraryPinsCarryTheErcPinTypes`: `SN74HC244N` 8 `tri_state`, 10 `input`, 2 `power_in`; `SN74LS07N` `open_collector`; resistor and header `passive`; `ATMEGA328P-PU` `bidirectional`; `LM7805` `power_out`; with no library the pins keep their roles).
Commit: 59d923a2

### Phase 2 — The comparison

- [x] `schematicalTests/ErcCases.swift` (made in 63-E with the 15 cases of § 3) is the
      one source of the 15 designs; nothing is added to it here.
- [x] New `schematicalTests/ErcCompareTests.swift`: `testPrintSamplesWhenAsked`
      (returns unless `ERC_OUT` is set) prints, for each of the 18 designs,
      `KICAD_FILE erc/<name>/<file> <base64>` for its export and `ERC_RESULT <name>
      <json>`: our violations (`rule`, `severity`, `refs`) and `railPins` (the
      `reference.pin` of every pin on a net with a rail symbol).
- [x] New `scripts/erc-compare.py <log>`: decodes the `KICAD_FILE` lines into a
      temporary directory, runs `kicad-cli sch erc --format json --severity-all` on each
      `.kicad_sch` (KiCad 10.0.8 at `~/Applications/KiCad/KiCad.app`, `--kicad` to
      override), compares as § 3 says, prints a table (design, rule, ours, KiCad's) and the
      differences, and exits 1 when any remains. The whitelist is a dict at the top of
      the file (`DELIBERATE`), each entry with its reason string.

Acceptance: the script runs on a real log and prints the comparison.
  Check: `TEST_RUNNER_ERC_OUT=1 xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/ErcCompareTests > /tmp/erc.log 2>&1; python3 scripts/erc-compare.py /tmp/erc.log | tail -25` → the table (est. 9 min).
  Result (2026-11-25): `TEST_RUNNER_ERC_OUT=1 xcodebuild test … -only-testing:schematicalTests/ErcCompareTests` → `** TEST SUCCEEDED **`, 19 `ERC_RESULT` lines and 38 `KICAD_FILE` lines (.kicad_pro and .kicad_sch of 19 designs); `python3 scripts/erc-compare.py /tmp/erc.log` printed the comparison table of 304 rows.
Commit: 3cd00f8e

### Phase 3 — Reconcile

- [x] For every difference the script prints: if our rule departs from KiCad's, fix
      the ERC in `ErcChecker.swift` (or `erc_check.c`, `erc_pins.c`), add a test, and
      note it in Corrections; if KiCad's behavior is a deliberate difference of ours,
      add it to `DELIBERATE` with its reason. No difference is left unexplained.

Acceptance: the script exits 0.
  Check: `python3 scripts/erc-compare.py /tmp/erc.log; echo $?` (the log re-made after the fixes by Phase 2's command) → ends with `0 differences` and `0` (est. 10 min with the rebuild).
  Result (2026-11-25): `python3 scripts/erc-compare.py /tmp/erc.log` → `designs: 19; rows compared: 304`, `0 differences`, exit 0. Violations over the 19 designs, the app's ERC / `kicad-cli`: pin_not_connected 351 / 351, pin_not_driven 194 / 194, power_pin_not_driven 51 / 51, pin_to_pin 3 / 3, no_connect_connected 1 / 1, no_connect_dangling 1 / 1, wire_dangling 1 / 1, unconnected_wire_endpoint 3 / 3, label_dangling 1 / 1, isolated_pin_label 6 / 6, similar_labels 1 / 1, similar_label_and_power 1 / 1; the rest are the `DELIBERATE` entries: duplicate_reference 1 / 0 (`kicad-cli` does not report it), single_global_label 8 / 0 (the export writes local labels), multiple_net_names 1 / 5 (4 labels the export adds for bus wires named like a rail: AFU 2, LFU 2), four_way_junction 1 / 2 (AFU: a wire bends at a junction). Total 625 / 621. What had to change in the app to get there is in Corrections 1–3.
Commit: 3cd00f8e

### Phase 4 — Landing

- [x] `planning/todo.md`: ticked `Electrical rule check (ERC)` in the main checkout after the merge, staging only that line (`git update-index` with the committed file plus the tick: the owner's other uncommitted edits in that file stay uncommitted, Corrections 6): d2511e6e.
- [x] The README of `planning/plan-63-data` gets a "Results" line (the comparison table).
- [x] Archive each sub-plan not yet archived to `planning/completed/` (`git mv`; each is archived as it completes): A–F were archived as they landed, this one with this commit.
- [x] The final gate below.

Acceptance: the final gate passes.
  Check: see Validation Plan (est. 35 min).
  Result (2026-11-25): see the Validation Plan's final gate.
Commit: (this file's last commit, with the archive)

## Validation Plan

- Tests: all new classes of 63-A to 63-F, plus `KiCadExportTests` (Phase 1) and the comparison (Phase 3).
- Coverage check: `ctest --test-dir /tmp/logic-build -N | grep -c erc` → 3 (`erc_pins_test`, `erc_pins_check`, `erc_test`); the 16 rules each appear in `ErcCases` (`grep -c 'case ' ErcSettings.swift` → 16 and each rule name occurs in `ErcPinChecksTests` or `ErcDrawingChecksTests`).
- Runtime proof: the comparison table; and the owner's real-window pass (open AFU, ⌥⌘E, run, select, exclude, close).
- Doc sync: `docs/backend.md`, `docs/file-format.md`, `docs/export.md`, `docs/agent-control.md`.
- Final gate (run ONCE, after Phase 4): `cmake --build /tmp/logic-build && ctest --test-dir /tmp/logic-build -R 'erc_|voltage|listing|trace'` → 100% passed (est. 10 min); `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -only-testing:schematicalTests/PowerFlagTests -only-testing:schematicalTests/NetLabelTests -only-testing:schematicalTests/ErcSettingsTests -only-testing:schematicalTests/ErcPinChecksTests -only-testing:schematicalTests/ErcDrawingChecksTests -only-testing:schematicalTests/ErcResultsModelTests -only-testing:schematicalTests/ErcMarkerTests -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/AutomationTests -only-testing:schematicalTests/UndoHistoryTests -only-testing:schematicalTests/SchematicUndoTests -only-testing:schematicalTests/DesignRuleCheckTests -only-testing:schematicalTests/KeyboardHelpTests 2>&1 | tail -3` → `** TEST SUCCEEDED **` (est. 25 min). The whole Debug suite is not run: it stalls in `AutorouteModelTests testTheSummaryOnAFU` (plan-61-D). Result (2026-11-25): run on the worktree after merging main (plan-64): `ctest --test-dir /tmp/logic-build-wt63 -R 'erc_|voltage|listing|trace'` → `100% tests passed, 0 tests failed out of 6`; the xcodebuild classes (the 16 listed, with `SchematicNSDocumentTests` and `ErcCompareTests`): `Executed 272 tests, with 2 tests skipped and 0 failures`, `** TEST SUCCEEDED **`; `erc-compare.py` on the merged tree: `designs: 19; rows compared: 304`, `0 differences`. Before the merge the same gate: 246 tests, 0 failures, 1 skipped. The whole Debug suite was not run (it stalls in `AutorouteModelTests testTheSummaryOnAFU`).

## Decisions

- The comparison is by rule with pins identified by reference and number — decided by the author, 2026-11-25.
- A rail symbol counts as a driver in our ERC and the comparison removes KiCad's matching reports — decided by the author, 2026-11-25 (63-B § 3).
- Fixtures, not `examples/`, are the real-design inputs — decided by the author, 2026-11-25 (the fixtures are frozen; `examples/` is in flux).

## Corrections

1. **§ 2's and § 3's rail premise was wrong** (recorded in plan-63-A Corrections 3): the KiCad export already writes one `PWR_FLAG` per rail (`KiCadSchematicWriter.flagInstance`), so KiCad does not report `power_pin_not_driven` on a rail net, and the comparison needs neither `railPins` nor the removal of KiCad's reports on rail nets. `ErcCompareTests` prints only the violations. (A rail's flag and a regulator's `power_out` pin on one net are a `pin_to_pin` error in KiCad and nothing in the app; none of the 19 designs has one, so the comparison does not show it. The app's rule is that a rail symbol is a driver and not a `power_out` pin, 63-B § 3.)
2. **19 designs, not 18**: the 3 fixtures and 16 drawings (`wireToNowhere`, the single-loose-end case, is the sixteenth); the fixtures are opened with the part library.
3. **What the comparison found in the app, and fixed.** (a) A card's edge connector: the export writes it as a symbol of passive pins (`EDGE1`) on the edge bus's nets; KiCad therefore saw drivers and a second pin that the app did not (17 + 18 `pin_not_driven` and 24 `isolated_pin_label` in AFU and LFU). The app's ERC now counts each wired port of the edge bus as a passive pin on its net (`ErcChecker.checkPins`; it reports nothing on them), after which `pin_not_driven` and `pin_not_connected` agree to the pin on all three fixtures. (b) `isolated_pin_label`: the rule is "a label on a lone pin, and no other label of its name anywhere", not "a net with one pin" (`ErcDrawingRules.checkLabels`; a bus wire with a wired port counts as a label of its name); `planning/plan-63-data/README.md` states it so, and `ErcDrawingChecksTests.testALabelOnANetWithOnePinIsIsolated` has the second-label case. The AFU fixture now has 0 violations (plan-63-E Corrections 6 said 17 and 4).
4. **What the script compares differently from § 3**, each read off the first run: KiCad's reports on the export's own `EDGE1` / `HEADER1` symbol are dropped; `pin_to_pin` is compared by count and by the set of pins involved (three outputs on one net give two pairs either way, not the same two); `no_connect_connected` by count (a No Connect wired to a pin sits at the wire's far end, and KiCad names a pin of the net that is not always the flagged one: `noConnectConnected` has U1.1 for the app, U2.3 for KiCad).
5. **The `DELIBERATE` list** (`scripts/erc-compare.py`), each with its reason in the file: `duplicate_reference` (not reported by `kicad-cli`), `single_global_label` (the export writes local labels), `multiple_net_names` in AFU and LFU (labels the export adds), `four_way_junction` in AFU (verified in the export: four wire runs end at (193.04, 63.5), two of them the same segment).
7. **Merging main (plan-64, Document Info) found three things.** Both plans added a field to the same places (`DocumentState`, `UndoKey`, `replaceContents`, the file, `EditorMenus`, the child windows, the help, the docs): both fields are kept, and the two undo keys sit side by side. `schematicalApp.swift`'s `.commands { … }` then had 11 entries and a `CommandsBuilder` takes 10 ("extra argument in call"): the two checks' menu commands are one `CheckCommands` (`ElectricalRulesCheckWindow.swift`). The post-merge gate and the comparison were re-run (Validation Plan).
8. Phases 2 and 3 landed in one commit (the script and the fixes it asked for were written together), so their `Commit:` lines carry one hash.
6. **`planning/todo.md`** is ticked in the main checkout after the merge, by staging only that line: the owner has other uncommitted edits in that file there, and a commit in the worktree that touched it would make the merge refuse.

## Summary

The risk is what the comparison finds: nothing is predicted, so the phase is
open-ended by design and the output is a list of fixes and explained
differences. Untouched: the simulation, the PCB, the other exports.
