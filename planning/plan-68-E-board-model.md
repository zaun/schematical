# plan-68-E: App — the board model

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-D

The PCB page's content moves out of `SchematicDocument` the way the schematic
sheet did in plan-68-D: the board settings, texts, traces and vias, holes,
pours, keep-outs and cutouts are stored by one observable `BoardLayout`, and
the edit logic of every PCB mark moves into `mutating` methods of value
types that are tested without a document. The document's PCB methods stay,
as thin recorded calls.

Prerequisites: plan-68-A's table, and plan-68-D complete (`ls
schematical/schematical/Model/Sheet/SchematicSheet.swift` → the file, and
plan-68-D's Commit lines filled).

References:

- plan-68-D § 3.1 (the pattern: value types with the logic, an observable
  owner with `private(set)` arrays and compare-and-write-back `update…`
  functions, `EditScope`, thin document methods). This letter applies it
  unchanged.
- `SchematicModel.swift` marks: Board, Board layers, Stackup (PCB), Board
  texts, Traces (PCB), Routing primitives (plan-38-G), Vias (PCB), Design
  rules (PCB), Net classes (PCB), Net layer rules and groups (PCB), Pours and
  keep-outs (PCB), Cutouts (PCB), Holes (PCB), Trace geometry, Segment drags
  (PCB).
- `Canvas/BoardSettings.swift` (`struct BoardSettings`, `BoardText`),
  `Canvas/BoardTrace.swift` (`BoardTrace`, `BoardVia`), `Canvas/BoardHole.swift`,
  `Canvas/BoardPour.swift`; the document extensions that read the board:
  `Ratsnest.swift`, `PourConnectivity.swift`, `PourSection.swift`,
  `NetList.swift`, `DesignRuleCheck.swift`, `Routing/RoutingApply.swift`,
  `RouteCommands.swift`, `RoutingProblemBuilder.swift`, `RoutingLength.swift`,
  `MovableChains.swift` (all read through the document's public API, which
  stays).

## 1. Goal

- `BoardLayout` (`@Observable` final class) stores `settings`, `texts`,
  `traces`, `vias`, `holes`, `pours`, `keepouts`, `cutouts`, each
  `private(set)` with today's `didSet` revision bumps (all but `texts` bump
  copper); the document holds one `layout: BoardLayout` and no longer stores
  those arrays. The document's `board` property (today the `BoardSettings`) keeps
  its name and type as a forward to `layout.settings`; the owner is
  `document.layout`.
- The logic of the marks in References lives in the value types of § 3, each
  file at or under 800 code lines, each type with a test class that needs no
  document.
- Every existing test passes unchanged.

### Non-goals

- As plan-68-D: public API, `DocumentState`, file format, undo steps and
  observation granularity unchanged.

## 2. Current State

Stored on the document (`private(set)`): `board` (`BoardSettings`, bumps
copper), `texts` (no bump), `traces`, `vias`, `holes`, `pours`, `keepouts`,
`cutouts` (bump); caches `keptPourFills`, `keptConnectivity`, `keptRatsnest`
(read by `Ratsnest.swift` and `PourConnectivity.swift`). Code lines of the
marks (`sections.js` on `SchematicModel.swift`): Board 105, Board layers 31,
Stackup 124, Board texts 128, Traces 153, Routing primitives 183, Vias 72,
Design rules 11, Net classes 118, Net layer rules and groups 141, Pours and
keep-outs 131, Cutouts 80, Holes 84, Trace geometry 53, Segment drags 135.
Typical shape (Net classes): `func addNetClass(named:) -> UUID {
recordEdit(.pcb, "Add Net Class") { …; board.netClasses.append(netClass);
return netClass.id } }`.

## 3. Design

| Value type (file in `schematical/schematical/Model/Board/`) | Arrays it edits | Marks | Code |
|---|---|---|---|
| `BoardSettings` (existing struct; new `BoardSettings+Edits.swift`, `BoardSettings+NetRules.swift`) | `settings` | Board, Board layers, Stackup, Design rules → 271; Net classes, Net layer rules → 259 | 530 |
| `BoardTexts` (`BoardTexts.swift`) | `texts` | Board texts | 128 |
| `BoardCopper` (`BoardCopper.swift`, `BoardCopper+Routing.swift`) | `traces`, `vias` | Traces, Vias, Trace geometry → 278; Routing primitives, Segment drags → 318 | 596 |
| `BoardFeatures` (`BoardFeatures.swift`) | `holes`, `pours`, `keepouts`, `cutouts` | Pours and keep-outs, Cutouts, Holes | 295 |

`BoardLayout` (`Model/Board/BoardLayout.swift`): the eight arrays, the four
`update…` functions (compare-and-write-back per array, plan-68-D § 3.1-2),
`copperRevision`, the three caches (`@ObservationIgnored`, moved from the
document; the extensions that read them reach them through the document's
forwards), `restore(_:_:)`.

Board edits that need the parts (a trace from a pad: the pad positions) take
a `PadLookup` value the document builds from `chipPads`, as plan-68-D's
`PinTips`. The Footprints (PCB) mark (placing and moving footprints, which
are the parts' `board` placements), Lock Position and Group drags (PCB) edit
parts and copper together: they stay on the document for plan-68-F.

### Tests

`BoardSettingsEditTests`, `BoardTextsTests`, `BoardCopperTests`,
`BoardFeaturesTests` (new, no document); `BoardObservationTests` (a reader
of `document.vias` is not invalidated by a trace-only edit, a reader of
`document.pours` not by a hole edit, a reader of `document.texts` not by a
trace edit); the existing PCB suites (`PCBUndoTests`, `TraceJoinTests`,
`TraceSegmentDragTests`, `ViaTests`, `HoleTests`, `PourModelTests`,
`CutoutModelTests`, `NetClassTests`, `NetRulesTests`, `BoardStackupTests`,
`BoardLayersTests`, `LockPositionTests`, `RoutingApplyTests` …) unchanged.

### Where the risk is

The routing code (`RoutingApply.swift`) applies a whole routing result as one
step; it reads and writes traces and vias through document methods that now
call `layout.updateCopper`. `RoutingApplyTests` and the fixture router tests
are the gate.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` with what remains; moot tasks are
> struck through with evidence, never deleted; fill `Commit:` when a phase
> lands. **An unticked box means NOT DONE.**

### Phase 1 — the owner

- [ ] `Model/Board/BoardLayout.swift`; the document's eight stored arrays
  become forwards; every write through `layout.update…(scope)`;
  `setState`/`replaceContents` through `layout.restore`; the caches move.
- [ ] `BoardObservationTests`.

Acceptance: the layout owns the content; nothing else changed.
  Check: `XB -only-testing:schematicalTests/PCBUndoTests
  -only-testing:schematicalTests/BoardObservationTests
  -only-testing:schematicalTests/BoardCopperCacheTests
  -only-testing:schematicalTests/PourFileTests
  -only-testing:schematicalTests/CutoutFileTests` → all pass (est. 5 min).
Commit: —

### Phase 2 — the logic into the values

- [ ] Move each mark's logic into § 3's `mutating` methods (`PadLookup`
  added); document methods become one-liners.
- [ ] The four value-type test classes.
- [ ] Ratchets: size row lowered; new files' coverage at 0.

Acceptance: logic in the values, tested alone; behavior unchanged.
  Check: the full `XB` → no failure beyond the standing ones; `COV --app
  --only schematical/schematical/Model/Board/` → no uncovered line (est. the
  full Debug suite; every PCB edit moves).
Commit: —

## Validation Plan

- Final gate (once, after Phase 2): the full `XB`; `SZ`; `COV --app --check`.

## Decisions

- (author) As plan-68-D § 3.1; footprints, locks and PCB group drags stay on
  the document because they edit parts and copper together.

## Corrections

(none yet)

## Summary

The second cut, same pattern as T. Routing applies are the riskiest callers
and have their own tests.
