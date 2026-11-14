# plan-38-A: Net classes — data model, file format, net resolution, `auto` flag

Last updated: 2026-11-13
Overall Effort: huge (> 3d) — plan-38 is A → B → C → D → E → F → G → H → I → J → K → L → M
Effort: medium (1h–2h)
Depends on: nothing (§ Prerequisites)

plan-38 adds a **professional net-routing system** to the PCB page: an
**Autoroute…** command (all nets, selected nets, selected parts, only the
unrouted connections; keep or clear existing copper; which layers; strategy),
**net classes** (trace width, clearance, via size), per-net **layer rules**
(only on these layers / prefer these layers), **net groups** (equal-length
groups, and coupled differential pairs), route/unroute commands for the
selection, and a router-assisted Trace tool. The single behavioral outcome of
plan-38: with `planning/plan-38-data/fixtures/AFU.schematical` open, Autoroute
(all nets, Balanced) completes all 115 unrouted connections on the 2-layer
board, one Undo removes everything it added, and the design rule check shows 0
unrouted, 0 short and 0 clearance, edge, via and trace-width issues that it
caused; on LFU it completes at least 127 of 188 (the spike's best, a floor) and
reports the rest by name; a net with a layer rule never leaves its layers; a
net group's nets end within the group's tolerance of each other; a
differential pair runs side by side at its gap.

| Letter | Content | Effort |
|---|---|---|
| **A** | net classes: data model, file format, net resolution and names; the `auto` flag on traces and vias | medium |
| B | per-net layer rules and net groups: data model, file format, edit functions, DRC check of layer rules | medium |
| C | PCB sidebar: Nets section (classes, layer rules, groups); Trace and Via tools use the net's class | large |
| D | routing model: shared copper geometry (extracted from the DRC), `RoutingProblem` snapshot, options, fixtures | large |
| E | routing engine: grid, obstacles, octilinear A\*, via and layer costs, layer rules, plane fan-out | large |
| F | routing engine: negotiated rip-up and reroute, net ordering, strategies, cancellation, determinism | large |
| G | apply a result to the document as one undo step; clear-existing and unroute; the background session | medium |
| H | routing engine: cleanup (45° shortcuts, via minimization, stub removal) and exact verification with repair | large |
| I | net groups: length measurement, meander tuning, DRC length-match check | large |
| J | net groups: coupled differential-pair routing | large |
| K | PCB UI: Autoroute… sheet, Route menu, net status list, help | large |
| L | router-assisted Trace tool (walk-around, via key, live legality) | medium |
| M | landing tier, docs, todo ticks | small |

Letter order is implementation order. Each letter is landable alone and
leaves the app working; nothing before K changes what the user sees except
C's Nets section.

Outcome of A: the document holds named **net classes** (each: trace width,
clearance, via drill and diameter, and the nets it covers, named stably by a
pin or a supply rail as a Plane's net is), saved in the file; every net on the
board resolves to its class and to a display name; traces and vias carry an
optional `auto` flag. Nothing routes yet and nothing is drawn differently.

References:

- `AGENTS.md`; `docs/file-format.md` (§ Board, § Traces, § Vias);
  `docs/boards-format.md` is not touched.
- `planning/plan-38-data/spike/README.md` and `results.txt` — the routing
  spike this plan's engine design rests on; `planning/plan-38-data/fixtures/` —
  the three boards it measured (sha256 in `SHA256SUMS`).
- Code read for this plan: `schematical/schematical/Canvas/BoardTrace.swift`
  (`BoardTrace`, `BoardVia`, `TraceRoute`), `Canvas/BoardSettings.swift`
  (`BoardSettings`, `PlaneNet`, `LayerStyle`), `Canvas/Ratsnest.swift`
  (`connectivity()`, `BoardConnectivity`), `Canvas/SchematicModel.swift`
  (`recordEdit`, `addTrace`, `pasteTraces`), `Canvas/UndoHistory.swift`
  (`DocumentState`), `Document/SchematicFile.swift` (`Board.TraceEntry`,
  `boardTrace`), `DesignRuleCheck/DesignRuleCheck.swift`.

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
(Stated once here; every other letter points here.)

| Must be true | Command | Status |
|---|---|---|
| plan-37 is archived (no other plan is mid-flight in the PCB code this plan edits) | `ls planning/completed/plan-37-H-register-landing.md` → the file | MET (2026-11-14: file present) |
| The app builds at the starting commit | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-14: `** BUILD SUCCEEDED **` at 945720a) |
| No other plan-38 file exists apart from this plan's | `ls planning \| grep '^plan-38'` → only plan-38-A … M and `plan-38-data` | MET (2026-11-14: exactly those 14 entries) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites.**

## 1. Goal

- `BoardSettings.netClasses: [NetClass]` exists, is undoable (it is part of
  `DocumentState.board`), round-trips through the file, and is written only when
  non-empty.
- `SchematicDocument.connectivity()` returns, for every net, its display name
  (`BoardConnectivity.netNames`) and the net each `PlaneNet` names
  (`BoardConnectivity.net(of:)`); `SchematicDocument.netClass(ofNet:in:)` returns
  the class a net is in, or nil for the implicit Default class.
- `BoardTrace.auto` and `BoardVia.auto` exist (default false), round-trip
  through the file (written only when true), and a pasted trace is never auto.

### Non-goals (explicit constraints)

- No UI (plan-38-C), no routing, no layer rules or groups (plan-38-B).
- The file `version` stays 4 (or 5 when custom parts are present), exactly as
  today: both additions are optional fields an older build ignores, the
  precedent set by `locked` and `showReference` in `docs/file-format.md`.
- A file with no net classes and no `auto` traces is byte-identical to what the
  current build writes for the same document (a test pins this).
- `BoardTrace.defaultWidth` (10 mil) and `BoardVia.defaultDrill/defaultDiameter`
  (0.4 / 0.8 mm) stay what they are; they are the implicit Default class.
- DRC rules (`DesignRules`) are not changed.

## 2. Current State

- A trace is `BoardTrace { id, layer, width (mil, 5…100), points (mm), locked }`
  (`Canvas/BoardTrace.swift:BoardTrace`); a via is `BoardVia { id, centre,
  drill, diameter, locked }` (same file). `Trace` and `Via` tools create them
  with the constants above: `SchematicDocument.addTrace(points:layer:)`
  (`SchematicModel.swift:2952`) and `addVia(at:)` (`:3226`).
- Nets are integers: `NetlistBuilder.build(...)` (`Simulation/Simulation.swift:42`)
  numbers them; they have no stored name. A net's stable identity elsewhere is
  `PlaneNet` (`BoardSettings.swift:PlaneNet`): `.rail(mv:)` or `.pin(chip:
  designator:)`, file text `"ground"`, `"power:<mV>"`, `"pin:<uuid>/<designator>"`,
  resolved inside `connectivity()` by a local `resolve(_:)`
  (`Ratsnest.swift`, "The schematic net a Plane or plated hole names").
- `connectivity()` (`Ratsnest.swift:connectivity`) already unions every supply
  symbol of one voltage into one rail net (`railNet`, `alias`, `merged`) and
  gives each pad its net (`netOfPad`).
- Board state is one value: `DocumentState.board: BoardSettings`
  (`UndoHistory.swift:10`), so a new `BoardSettings` field is undone and redone
  with no extra code, and every edit goes through `recordEdit(.pcb, name)`
  (`SchematicModel.swift:1607`).
- The file writes optional fields only when set: `locked` on traces
  (`SchematicFile.swift:boardTrace`, `Board.TraceEntry`).
- `pasteTraces` builds each copy with explicit fields
  (`SchematicModel.swift:3112`), so a new field with a default is false on a
  paste without a code change; a test makes that a guarantee.

### Measured populations

| What | Count | Command |
|---|---|---|
| `BoardTrace(` constructor sites (all compile unchanged with a defaulted new field) | 13 in 9 files | `grep -rn "BoardTrace(" schematical/schematical schematical/schematicalTests \| wc -l` → 13 |
| `BoardVia(` constructor sites | 3 | `grep -rn "BoardVia(" schematical/schematical schematical/schematicalTests \| wc -l` → 3 |
| Existing trace/via/ratsnest/DRC test methods that must stay green | 45 | `grep -c "func test" TraceToolTests TraceRouteTests ViaTests DesignRuleCheckTests RatsnestTests` → 45 total |
| Xcode target membership | automatic: new files under `schematical/schematical/` and `schematical/schematicalTests/` join their targets (file-system-synchronized groups) | `grep -c "isa = PBXFileSystemSynchronizedRootGroup" schematical/schematical.xcodeproj/project.pbxproj` → 3 |

### Verified properties

- `BoardSettings` is `Hashable`; adding a `Hashable` field keeps `DocumentState`
  equality, which is what `UndoDiff` compares (read: `UndoHistory.swift:10-23`,
  `recordEdit` at `SchematicModel.swift:1607` compares `state` before and after).
- `SchematicFile.Board` is a synthesized-`Codable` struct
  (`SchematicFile.swift:145`), and a `JSONDecoder` ignores keys the struct has no
  property for, so an older build reads a newer file's extra `netClasses` / `auto`
  keys and drops them (the behavior `docs/file-format.md` states for `locked`).
- A cold `xcodebuild … test -only-testing:schematicalTests/<one class>` took 31 s
  (spike worktree: `stat -f %SB /tmp/route-probe-dd` 09:59:39 → log mtime 10:00:09).

## 3. Design Overview

Class model. `NetClass { id: UUID, name: String, width: Int (mil,
`BoardTrace.widthRange`), clearance: Double? (mm, `LayerStyle.clearanceRange`;
nil = the DRC's minimum), viaDrill: Double, viaDiameter: Double (checked by
`BoardVia.problem(drill:diameter:)`), members: [PlaneNet] }`. A net is in the
**first class, in list order, any of whose members resolves to it**; a net in no
class is in the implicit **Default** class (10 mil, DRC-minimum clearance, via
0.4 / 0.8 mm). Members use `PlaneNet` as their key because it already names a
net by a pin or a supply rail and survives rewiring of the other pins; the
file text is `PlaneNet.fileValue`. A class whose every member no longer resolves
stays (it is the user's data); the sidebar (plan-38-C) shows it empty.

Names. `BoardConnectivity.netNames: [Int: String]`, by this priority: a supply
symbol's part name (`"GND"`, `"5V"`; a rail merged from several symbols uses the
first in chip order), else the text of a net label wired onto the net, else the
name of a bus wire whose port pin is on the net, else the net's first pad as
`"U1.3"` (lowest chip index, then designator in natural order). Names are made
unique by appending `" (2)"`, `" (3)"` in net order.

`auto`. A `Bool` on `BoardTrace` and `BoardVia`, default false; set by the router
(plan-38-G); read by Clear and by selection filters.

Where correctness risk concentrates: the file round trip and the
byte-identical guarantee for documents that use none of it (Phase 2); name and
class resolution on merged rails (Phase 3).

Probed before writing: `planning/plan-38-data/spike/DumpScene.swift.txt` opened
AFU, LFU and RegFile through `SchematicFile.decode(...).apply(...)` and read
`connectivity()` — 54 / 59 / 8 nets with a pad, so resolution has real input.

Byte-identity IS the gate for "documents that use none of it are written exactly
as before" (Phase 2, a test); everything else is behavior-gated.

Rejected: naming a net by its integer (renumbers on any rewire); a class stored
per trace (a trace's class is its net's, and a retyped net would orphan it); one
class per net (classes exist to share).

## 4. Detailed Design

- `Canvas/NetClass.swift` (new, `nonisolated`, `Hashable`, `Sendable`): the struct
  above; `static let defaultWidth = BoardTrace.defaultWidth`, `defaultViaDrill`,
  `defaultViaDiameter` = the `BoardVia` constants; `static func problem(width:
  clearance:viaDrill:viaDiameter:name:) -> String?` (width in range; clearance in
  range; the via rule of `BoardVia.problem`; name 1…40 characters, not blank).
- `BoardSettings.netClasses: [NetClass] = []`.
- Edit functions on `SchematicDocument`, each `recordEdit(.pcb, name)`:
  `addNetClass(named:) -> UUID`, `removeNetClass(_:)`, `renameNetClass(_:to:)`,
  `setNetClassWidth(_:_:)`, `setNetClassClearance(_:_:)` (nil = DRC minimum),
  `setNetClassVia(_:drill:diameter:)` (clamped as `setViaDrill/Diameter` clamp),
  `assignNet(_ net: PlaneNet, to classID: UUID?)` (removes the member from every
  class, then appends it to `classID`'s members; nil = back to Default),
  `moveNetClass(_:to:)` (order is priority).
- `BoardConnectivity`: new stored `railNets: [UInt32: Int]`, `pinNets:
  [String: Int]` (key `"<chip UUID>/<designator>"`), `netNames: [Int: String]`;
  `func net(of: PlaneNet) -> Int?`. `connectivity()` fills them from the values
  its local `resolve(_:)` already reads; `resolve` becomes a call to `net(of:)`.
- `SchematicDocument.netClass(ofNet:in:) -> NetClass?`.
- `Document/SchematicFile.swift`: `Board.netClasses: [NetClassEntry]?`
  (`id`, `name`, `width`, `clearance?`, `viaDrill`, `viaDiameter`, `members:
  [String]`), written only when non-empty; decode checks each with
  `NetClass.problem`, rejects duplicate ids and a member string
  `PlaneNet(fileValue:)` does not parse, as file errors in the style of
  `boardTrace` ("net class <id> …"). `TraceEntry.auto` and the via entry's
  `auto`: `Bool?`, written only when true.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` partial (say what remains). A moot task is
> `- [x] ~~text~~ — moot: <evidence>`. Fill the `Commit:` line when the phase
> lands. An unticked box means NOT DONE.

> **How to read the Check lines.** `xcodebuild` below means `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test CODE_SIGNING_ALLOWED=NO`; a Check that starts with `-only-testing:…` is that command with those arguments. Tests are classes in `schematical/schematicalTests/` (file-system-synchronized: a new file there is in the test target with no project edit).

### Phase 1 — `NetClass` and its edit functions

- [x] `Canvas/NetClass.swift`: the struct, constants and `problem(...)` of § 4.
- [x] `Canvas/BoardSettings.swift:BoardSettings`: add `var netClasses: [NetClass] = []` after `styles`.
- [x] `Canvas/SchematicModel.swift`: the eight edit functions of § 4, next to `setViaDiameter`.
- [x] Tests: `schematicalTests/NetClassTests.swift` — add/rename/remove; width and via clamps; `assignNet` moves a member between classes and back to Default; one `undo(on: .pcb)` reverts each edit; `NetClass.problem` rejects width 4 and 101 mil, a 0.04 mm clearance, a via diameter less than drill + 0.1 mm, a blank name. — `-only-testing:schematicalTests/NetClassTests` → 5 tests passed, `** TEST SUCCEEDED **`.

Acceptance: every edit is one undo step and every clamp holds.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test -only-testing:schematicalTests/NetClassTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 1–2 min).
Commit: —

### Phase 2 — file format

- [ ] `Document/SchematicFile.swift`: `Board.netClasses` entries, encode (only when non-empty), decode with the checks of § 4.
- [ ] `docs/file-format.md`: § Board table row `netClasses`; new § Net classes (fields, "written only when there are some", older builds ignore it, the member text formats shared with a Plane's `net`).
- [ ] Tests (`NetClassTests`): a document with two classes saves and reopens equal; a document with none writes no `netClasses` key and its bytes equal a pre-change snapshot (encode a fixed small document, compare with a literal in the test); a bad width, a duplicate id and an unparsable member each throw with a message naming the class.

Acceptance: round trip equal; the no-class file is byte-identical to the literal.
  Check: same command with `-only-testing:schematicalTests/NetClassTests -only-testing:schematicalTests/SchematicFileV5Tests` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: —

### Phase 3 — net resolution and names

- [ ] `Canvas/Ratsnest.swift`: `BoardConnectivity` gains `railNets`, `pinNets`, `netNames`, `net(of:)`; `connectivity()` fills them; its local `resolve` calls `net(of:)`.
- [ ] `Canvas/Ratsnest.swift` (or `Canvas/NetNames.swift`): the name rule of § 3, unique-ified.
- [ ] `SchematicModel.swift`: `netClass(ofNet:in:)`.
- [ ] Tests (`NetClassTests`): in a small document built as `RatsnestTests` builds its boards — a net with a `GND` symbol is named "GND" and a class member `.ground` resolves to it; two `5V` symbols are one net named "5V"; a labelled net takes the label; a plain net is named by its first pad ("U1.3"); a net in two classes is in the first.

Acceptance: names and classes as listed; every existing Ratsnest and PlaneNet test stays green (the `resolve` rewrite changed no behavior).
  Check: `-only-testing:schematicalTests/NetClassTests -only-testing:schematicalTests/RatsnestTests -only-testing:schematicalTests/PlaneNetTests` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: —

### Phase 4 — the `auto` flag

- [ ] `Canvas/BoardTrace.swift`: `BoardTrace.auto = false`, `BoardVia.auto = false` (defaulted; the 16 constructor sites compile unchanged).
- [ ] `Document/SchematicFile.swift`: read and write `auto` (only when true); `docs/file-format.md` § Traces and § Vias rows.
- [ ] Tests (`NetClassTests`): an `auto` trace and via round trip; a file without the key reads false; `pasteTraces` of an `auto` trace is not `auto`.

Acceptance: round trip and paste behave as listed; the existing 45 trace/via/DRC/ratsnest tests stay green.
  Check: `-only-testing:schematicalTests/NetClassTests -only-testing:schematicalTests/TraceToolTests -only-testing:schematicalTests/TraceRouteTests -only-testing:schematicalTests/ViaTests -only-testing:schematicalTests/DesignRuleCheckTests -only-testing:schematicalTests/RatsnestTests` → `** TEST SUCCEEDED **` (est. 3 min).
Commit: —

## Validation Plan

- Tests: `NetClassTests` (new), `SchematicFileV5Tests`, the five existing suites above.
- Coverage check: `NetClass.swift`, the `netClasses`/`auto` file paths and `BoardConnectivity.netNames` are exercised by `NetClassTests` (the tests fail if the field is not written or read).
- Runtime proof: none beyond the unit tests; no UI changes in this letter.
- Doc sync: `docs/file-format.md` (Phases 2 and 4).
- Final gate: not run here; the landing tier runs once in plan-38-M.

## Decisions

All owner decisions for plan-38, made before writing (the later letters point here):

- Scope: the batch autorouter plus interactive aids (route/unroute selection commands, router-assisted Trace tool); **no push-and-shove** — decided by the owner, 2026-11-13.
- Net classes (width, clearance, via size) are in this plan, stored in the file — decided by the owner, 2026-11-13. (§ 3)
- Autorouted copper is marked `auto` in the file so Clear can remove only it — decided by the owner, 2026-11-13. (Phase 4; plan-38-G)
- A net's layer setting is both: **Only on** a set of layers (hard) and **Prefer** a set (soft), per net and per class — decided by the owner, 2026-11-13. (plan-38-B, E)
- Net groups do **length matching** (meander tuning) and **coupled differential-pair routing** (two nets side by side at a gap) — decided by the owner, 2026-11-13. (plan-38-I, J)

Author decisions, with their one-line reason, are under each letter's design.

## Corrections

(Filled in during execution.)

## Summary

The risk here is the file contract: two new optional fields must leave every
existing file byte-identical, and the class lookup must follow nets through
merged supply rails. Everything else in plan-38 reads classes through
`netClass(ofNet:in:)` and names through `netNames`, so those two must be right
before B.
