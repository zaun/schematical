# plan-38-G: Apply a routing result — one undo step, clear existing, unroute, the background session

Last updated: 2026-11-13
Effort: medium (1h–2h)
Depends on: plan-38-F

Prerequisites: see plan-38-A § Prerequisites (re-run its commands). Also every earlier letter (A, B, C, D, E, F) is complete: for each letter X, `grep -c "Commit: —" planning/plan-38-X-*.md planning/completed/plan-38-X-*.md 2>/dev/null` prints 0 (a letter is archived to `planning/completed/` when it finishes, so either path may exist).

The engine (E, F) returns a `RoutingResult`. This letter connects it to the
document: the result becomes `auto`-flagged traces and vias in **one undo step**;
the options' *clear existing* choice and the **unroute** commands delete copper
by net; the selection becomes a routing scope; and a `RoutingSession` runs the
engine on a background task with progress, **Stop** (keep what is routed) and
**Cancel** (discard), refusing to apply onto a board that changed while it ran.
It is the last piece before the UI (K) and is tested without any UI.

References:

- plan-38-A (§ Decisions: `auto`), plan-38-D (§ 3 scope and existing copper,
  `removedTraces`), plan-38-F (cancel/time return a legal ledger).
- `Canvas/SchematicModel.swift`: `recordEdit` (`:1607`), `addTrace(points:layer:)`
  (`:2952`), `join` (`:2966`), `addVia(at:)` (`:3226`), `deleteTrace`, `deleteVia`,
  `boardSelection` / `select(_:)` (`:1288`), `isPlaying` (`:1576`);
  `Canvas/Selection.swift:ItemSelection`.

## 1. Goal

- `SchematicDocument.applyRouting(_ result: RoutingResult, problem: RoutingProblem)
  -> RoutingSummary` adds the result's traces and vias with `auto = true`, deletes
  `problem.removedTraces` / `removedVias`, and is exactly one undo step named
  "Autoroute"; Undo restores the document (`DocumentState` equal to before), Redo
  reapplies.
- After applying the F result for AFU, `ratsnest().count == 0` and the DRC lists
  0 `.unrouted` and 0 `.short` issues. (Clearance cleanup is H's gate, not this
  letter's.)
- `unroute(...)` deletes the unlocked copper of the given nets (all, or only
  `auto`), one undo step; `RoutingScope.fromSelection(document)` turns the
  selection into a scope.
- `RoutingSession` runs a problem off the main actor, reports progress, honors
  Stop and Cancel, and discards a result whose document changed meanwhile.

### Non-goals (explicit constraints)

- No UI (K); no engine change; no cleanup (H); no groups (I, J).
- Locked traces and vias are never deleted by any command here.
- The edit goes through the document's existing edit functions' rules: no trace
  on a plane or an absent layer (`traceLayerRefusal`), vias only on boards with
  more than one layer.
- Applying never changes copper the result does not name, except the vertex that
  `join` inserts where a new trace touches an existing one (geometry unchanged).

## 2. Current State

- All PCB edits are `recordEdit(.pcb, name) { … }`; nested calls fold into the
  outermost one, so one wrapper makes one step (`SchematicModel.swift:1607`).
- `addTrace(points:layer:)` appends a `BoardTrace` at the default width and calls
  `join`; it is the document's trace-from-points entry point; `BoardTrace.width` is
  mil, `RoutedTrace.width` (plan-38-D) is mm (`mil × 0.0254`).
- `deleteTrace(id)` / `deleteVia(id)` delete by id and do not look at `locked`
  (read `SchematicModel.swift:deleteTrace`; `locked` only stops dragging, X/Y and
  rotate), so this letter's commands filter `locked` themselves.
- `isPlaying` makes `recordEdit` run the body unrecorded (`:1607`); a command that
  edits during the simulation must refuse instead.
- Selection is `ItemSelection<BoardItem>` with `.footprint/.text/.trace/.via/.hole`.

### Measured populations

| What | Count | Command |
|---|---|---|
| `BoardItem` cases | 5 | `Canvas/Selection.swift:21-27` (footprint, text, trace, via, hole) |
| `recordEdit(.pcb` call sites in the document | 56 | `grep -c "recordEdit(.pcb" schematical/schematical/Canvas/SchematicModel.swift` → 56 |

### Verified properties

- `join(id)` makes shared vertices only on the new trace's layer and keeps every
  segment's direction (inserts a vertex on a straight segment): read
  `SchematicModel.swift:2966-2986` (`Self.inserting`), so applying cannot reshape
  a kept trace.

## 3. Design Overview

`applyRouting`:

1. `recordEdit(.pcb, "Autoroute") { … }` (a result with no new copper and nothing
   to delete records nothing, as `recordEdit` does for an unchanged state).
2. Delete `problem.removedTraces` and `removedVias` (skipping any now missing).
3. For each `RoutedVia` (in result order): append a `BoardVia(id: UUID(), centre,
   drill, diameter, auto: true)`.
4. For each `RoutedTrace` (in order): `BoardTrace(id: UUID(), layer, width:
   Int((mm / 0.0254).rounded()), points, auto: true)`, appended, then `join(id)`.
   A trace whose layer fails `traceLayerRefusal` is skipped and listed in the
   summary (cannot happen for engine output; guards a stale problem).
5. Select every added trace and via (`boardSelection.set`).
6. Return `RoutingSummary { added traces, vias; removed; unrouted: [(net name,
   count)] from `result.nets`; skipped }`.

The new ids are fresh `UUID()`s, so a re-run adds new copper rather than reusing
ids; `auto` is what ties it to the router.

`unroute(nets: Set<Int>, onlyAuto: Bool)` (`@MainActor`): the traces and vias whose
**net** (root's net in `connectivity()`) is in `nets`, not locked, and `auto` when
`onlyAuto`; deleted in one `recordEdit(.pcb, "Unroute")`. `unrouteAll(onlyAuto:)`
also removes copper on no net unless `onlyAuto`. Both refuse (return 0) while
`isPlaying`.

`RoutingScope.fromSelection(document)`: footprints selected → `.footprints(ids)`
(plus the nets of any selected trace/via); only traces/vias selected → `.nets` of
their nets; nothing selected → `.all`.

`RoutingSession` (`@MainActor @Observable`): `state: idle | running(RoutingProgress)
| finished(RoutingSummary) | discarded(reason)`. `start(document:options:rules:)`
snapshots `document.state` and the problem, then `Task.detached` calls
`Router.route(problem, progress:, isCancelled:)`; progress hops to the main actor
at most every 0.1 s and carries a throttled (≥ 0.5 s) copper snapshot for a live
preview. `stop()` sets a flag the engine reads (it returns its legal ledger, the
session applies it); `cancel()` sets the same flag and the session discards the
result. On completion on the main actor: if `document.state != snapshot`, the
result is discarded with the reason "The board changed while routing"; else
`applyRouting`. The PCB is read-only while a session runs (the UI in K blocks
edits; the session also refuses to start while `isPlaying` or while another
session runs on the same document).

Where correctness risk concentrates: the stale-board check and the join of new
traces into kept ones. Acceptance is behavior; no byte-identity.

Rejected: applying copper incrementally while routing (each connection an undo
step — one Autoroute must be one Undo); keeping engine ids (a re-run would
collide with kept ids); a version counter on the document (`DocumentState` equality
is exact and already exists).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` partial. Moot: `- [x] ~~text~~ — moot: <evidence>`.
> Fill the `Commit:` line when the phase lands. Unticked means NOT DONE.

> **How to read the Check lines.** `xcodebuild` below means `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test CODE_SIGNING_ALLOWED=NO`; a Check that starts with `-only-testing:…` is that command with those arguments. Tests are classes in `schematical/schematicalTests/` (file-system-synchronized: a new file there is in the test target with no project edit).

### Phase 1 — `applyRouting`

- [ ] `Routing/RoutingApply.swift`: `RoutingSummary`; `extension SchematicDocument { func applyRouting(_:problem:) -> RoutingSummary }` per § 3.
- [ ] Tests: `schematicalTests/RoutingApplyTests.swift` — a hand-made result of one trace and one via lands `auto`, at the mil width of its class (a 0.508 mm Power net → 20 mil), and is selected; one `undo(on: .pcb)` returns `DocumentState` equal to the saved before-state and `redo` restores it; a `removedTraces` entry is deleted and a locked kept trace is untouched; a trace on a plane layer is skipped and reported; applying an empty result records no undo step; AFU: `Router.route` (`.balanced`) then apply → `ratsnest().count == 0`, DRC `.unrouted` 0 and `.short` 0.

Acceptance: as listed.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test -only-testing:schematicalTests/RoutingApplyTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 2 min; the AFU case adds about 2 s of routing, plan-38-F).
Commit: —

### Phase 2 — unroute and scope from selection

- [ ] `Routing/RoutingApply.swift`: `unroute(nets:onlyAuto:)`, `unrouteAll(onlyAuto:)`, `RoutingScope.fromSelection(_:)`.
- [ ] Tests (`RoutingApplyTests`): after applying the AFU result, `unroute(nets: [one net], onlyAuto: true)` deletes only that net's traces and vias and `ratsnest().count` equals that net's connection count; a hand-drawn trace on the same net survives `onlyAuto` and is deleted by `onlyAuto: false`; a locked `auto` trace survives both; `unrouteAll(onlyAuto: true)` after apply restores `traces.count == 0` on a board that had none; one Undo restores; `isPlaying` makes each return 0; selection: a footprint → `.footprints`, a trace → its net, nothing → `.all`.

Acceptance: as listed.
  Check: `-only-testing:schematicalTests/RoutingApplyTests` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: —

### Phase 3 — `RoutingSession`

- [ ] `Routing/RoutingSession.swift`: states, `start`, `stop`, `cancel`, throttled progress and preview snapshot, the stale check, the `isPlaying` / already-running refusals.
- [ ] Tests (`RoutingSessionTests`, async): start on a small synthetic document, await `finished`, copper present; `stop()` right after start finishes with a legal partial result applied (no overlaps by `CopperGeometry.gap`); `cancel()` leaves the document `DocumentState`-equal to before; editing the document (move a footprint) while running yields `discarded("The board changed while routing")` and no copper; starting while `isPlaying` yields `idle` and an `editorMessage`; progress reports are non-decreasing in routed count between rounds.

Acceptance: as listed; no data race (the engine only reads the `Sendable` problem).
  Check: `-only-testing:schematicalTests/RoutingSessionTests -only-testing:schematicalTests/RoutingApplyTests` → `** TEST SUCCEEDED **` (est. 3 min).
Commit: —

## Validation Plan

- Tests: `RoutingApplyTests`, `RoutingSessionTests`, `PCBUndoTests` (unchanged, must stay green).
- Coverage check: each new file under `Routing/` named here is exercised; the stale-board branch and the locked-copper filters fail a test if removed.
- Runtime proof: Phase 1's AFU apply (`ratsnest` 0) and Phase 3's session tests; the real-app proof is plan-38-K.
- Doc sync: none yet.
- Final gate: plan-38-M.

## Decisions

See plan-38-A § Decisions. Author decisions: Stop keeps the routed set, Cancel discards; a changed board discards the result; the result is selected after applying; `auto` copper has fresh ids on every run; `unroute` protects locked copper in every mode.

## Corrections

(Filled in during execution.)

## Summary

Small code, high consequence: it is the only place the engine edits the
document. One wrapper gives one undo step; the stale check stops a result from
landing on a board it wasn't computed for. Untouched: the engine and the UI.
