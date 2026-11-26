# plan-68-C: App — `SchematicDocument` covered to 100% before it is taken apart

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-68-B

Letters D, E and F take `SchematicDocument` apart into a sheet model, a board
model and a small core. Their only proof that nothing changed is the existing
test suite, so before the first cut every line of the document must be run by
a test that checks what it did. Today 221 of its 5,420 executable lines are
not (per line; plan-68-A Corrections). This letter writes those tests against
the document's public API, the
API that D–F keep, so they stay valid through the split.

Prerequisites: plan-68-A's table, and plan-68-B complete (`head -20
schematical/schematical/Canvas/SchematicModel.swift | grep -c 'final class
SchematicDocument'` → 1).

References:

- `schematical/schematical/Canvas/SchematicModel.swift` (after plan-68-B: the
  document only).
- `planning/plan-68-data/app-missed.tsv` (`221 5420
  schematical/schematical/Canvas/SchematicModel.swift`: the full Debug suite
  with coverage at `e2f6811a`, per line).
- plan-68-W § 3 (the app coverage method; this letter uses its unit-test and
  unreachable-code steps only: the document has no views).
- `schematicalTests/TestSupport.swift` (`TestSupport.newDocument()`),
  `FixtureSupport.swift` (`Fixture.open`).

## 1. Goal

- `node scripts/coverage.js --app --tests <the document's test classes> --only
  <file>` prints no uncovered line for `SchematicModel.swift` and for the five
  files plan-68-B moved out of it (`SchematicTool.swift`, `ChipLayout.swift`,
  `PlacedChip.swift`, `Wire.swift`, `OrthogonalRoute.swift`, all in
  `schematical/schematical/Canvas/`); their coverage ratchet rows are gone.
- Every new test calls only members that are used outside
  `SchematicModel.swift` today (the API D–F keep), or is in a test class that
  D–F list as moving with the member it tests.

### Non-goals

- No change to `SchematicModel.swift` except deleting or restructuring a line
  no caller can reach (plan-68-W § 3, step 2, last case), each named in its
  commit with the callers read.

## 2. Current State

221 uncovered lines, per line (above): 148 in the document and 73 in the
value types plan-68-B moved out (`SchematicTool.swift` 9, `ChipLayout.swift`
35, `PlacedChip.swift` 9, `OrthogonalRoute.swift` 20, `Wire.swift` 0;
mapped from the old file's line numbers to the B boundaries by
`planning/plan-68-data/c_lines.py`, run in the main checkout before B). The lines move between files as plan-68-B lands
(it moves only the value types, and their lines go with them), so the letter
starts with the scoped list:
`node scripts/coverage.js --app --tests <classes> --only
schematical/schematical/Canvas/SchematicModel.swift`, where `<classes>` are
the test classes that construct a document: `grep -lE
'TestSupport.newDocument|Fixture.open' schematical/schematicalTests/*.swift`.

## 3. Design

Each uncovered block is reached through the document's API from a test in the
class for its area (`SchematicUndoTests`, `PCBUndoTests`, `NetLabelTests`,
`BusJoinTests`, `TraceSegmentDragTests`, `LockPositionTests`,
`SplitPartEditTests`, `SchematicClipboardTests` …; a new
`SchematicDocumentEdgeTests` for blocks no area class fits), asserting the
document's state after the call and, for every edit, that `undo` restores the
state before it — so the same tests also pin the undo snapshot D–F rewire.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` with what remains; moot tasks are
> struck through with evidence, never deleted; fill `Commit:` when a phase
> lands. **An unticked box means NOT DONE.**

### Phase 1 — every document line run and checked

- [x] The scoped list (§ 2); a test per block (§ 3), each with its undo check.
- [x] The ratchet row for `SchematicModel.swift` removed.

Acceptance: the whole document is covered by tests on the API that survives
the split.
  Check: the § 1 scoped command → no uncovered line; `XB -only-testing` the
  classes it names → pass (est. 15 min: one scoped coverage run over every
  class that builds a document).
  Observed, by the union check that replaces it (Corrections):
  `planning/plan-68-data/c_union.py /tmp/logic-cov/app/result.xcresult
  /tmp/p68-c.xcresult` maps each of the 221 lines the full-suite baseline
  left uncovered to its file and line after plan-68-B and reads its count
  in the run of the four new classes (`SchematicValueTypesTests`,
  `SchematicDocumentEdgeTests`, `SchematicDocumentEdgeMoreTests`,
  `SchematicVoltageRefusalTests`: `Executed 37 tests, with 0 failures`):
  219 covered, and the 2 it flags are its own mapping misses, checked by
  hand — `let id = UUID()` became `PendingRemoval`'s explicit init (lines
  5496-5500, count 1) and erase's `else { return }` is line 5719 (count 1).
  The lines this letter changed are covered too (`ChipLayout.supply` 10,
  `setBusLines`' guard 3).
Commit: —

## Validation Plan

- Final gate (once): the Phase 1 check; `SZ`.

## Decisions

- (author) The document is covered before it is split, so D–F are guarded by
  a suite that runs every line they move.

## Corrections

- **The acceptance's scoped run became a union check.** Running every class
  that builds a document (`grep -lE 'TestSupport.newDocument|Fixture.open'`)
  is most of the 27-minute suite. A line covered by the baseline full run
  stays covered (tests and code unchanged but for this letter's edits), so
  only the baseline's 221 uncovered lines need the new classes: the union
  check reads exactly those, in minutes.
- **Two restructures for the coverage counters, not for reachability.**
  `setBusLines`' `for … where … { return false }` loop left the lines after
  it with count 0 and then 2^48-1 although the test proves they ran (it
  asserts the `true` only the end of the function returns): Swift's
  counters for that shape inside a closure are wrong. It is now the
  equivalent `guard !zip(…).contains(where: …) else { return false }`, and
  the counts are right (3, 2, 1). And `PendingRemoval`'s `let id = UUID()`
  default is never counted (a synthesized init's default value has no
  counter of its own), though removals are made; it is an explicit
  `fileprivate init`. Both keep behavior exactly.
- **`ChipLayout.supply`'s fallback was unreachable** (a supply symbol on a
  part with no visible pin: no such part exists), so it is total now: the
  symbol at its size, with the part's first pin if it has one.
- **Scope widened to the five value files.** The letter was written as
  "`SchematicModel.swift` to 100%", but plan-68-B moved 73 of the file's 221
  uncovered lines into five new files before this letter starts. Those lines
  were part of `SchematicModel.swift` when the plan was measured and are in
  no other letter's table, so they are this letter's: the goal now names the
  six files.

## Summary

A small letter with a large job: it is the safety net for the riskiest
refactor in plan-68.
