# plan-68-C: App — `SchematicDocument` covered to 100% before it is taken apart

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-68-B

Letters D, E and F take `SchematicDocument` apart into a sheet model, a board
model and a small core. Their only proof that nothing changed is the existing
test suite, so before the first cut every line of the document must be run by
a test that checks what it did. Today 313 of its 8,338 executable lines are
not. This letter writes those tests against the document's public API, the
API that D–F keep, so they stay valid through the split.

Prerequisites: plan-68-A's table, and plan-68-B complete (`head -20
schematical/schematical/Canvas/SchematicModel.swift | grep -c 'final class
SchematicDocument'` → 1).

References:

- `schematical/schematical/Canvas/SchematicModel.swift` (after plan-68-B: the
  document only).
- `planning/plan-68-data/app-missed.tsv` (`313 8338
  schematical/schematical/Canvas/SchematicModel.swift`: the full Debug suite
  with coverage at `e2f6811a`).
- plan-68-W § 3 (the app coverage method; this letter uses its unit-test and
  unreachable-code steps only: the document has no views).
- `schematicalTests/TestSupport.swift` (`TestSupport.newDocument()`),
  `FixtureSupport.swift` (`Fixture.open`).

## 1. Goal

- `node scripts/coverage.js --app --tests <the document's test classes> --only
  schematical/schematical/Canvas/SchematicModel.swift` prints no uncovered
  line; the file's coverage ratchet row is gone.
- Every new test calls only members that are used outside
  `SchematicModel.swift` today (the API D–F keep), or is in a test class that
  D–F list as moving with the member it tests.

### Non-goals

- No change to `SchematicModel.swift` except deleting or restructuring a line
  no caller can reach (plan-68-W § 3, step 2, last case), each named in its
  commit with the callers read.

## 2. Current State

313 uncovered lines (above). The lines move between files as plan-68-B lands
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

- [ ] The scoped list (§ 2); a test per block (§ 3), each with its undo check.
- [ ] The ratchet row for `SchematicModel.swift` removed.

Acceptance: the whole document is covered by tests on the API that survives
the split.
  Check: the § 1 scoped command → no uncovered line; `XB -only-testing` the
  classes it names → pass (est. 15 min: one scoped coverage run over every
  class that builds a document).
Commit: —

## Validation Plan

- Final gate (once): the Phase 1 check; `SZ`.

## Decisions

- (author) The document is covered before it is split, so D–F are guarded by
  a suite that runs every line they move.

## Corrections

(none yet)

## Summary

A small letter with a large job: it is the safety net for the riskiest
refactor in plan-68.
