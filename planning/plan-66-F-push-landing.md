# plan-66-F: Push and shove — the result line, the docs, the landing tier

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-66-E

F is what makes the feature finished for a user and checkable as one: the
Autoroute sheet says how much existing copper a run moved, `docs/routing.md`
describes the option, the engine step, the limits and the measured numbers, a
pass in the real app shows a push on a real board, and the landing tier runs
once on the finished tree. No design; a gate that fails is root-caused in the
letter that owns the code and recorded in that letter's Corrections.

References:

- `planning/plan-66-A-push-option-anchors.md` … `plan-66-E-push-integration.md`
  (each letter's Acceptance and Validation Plan; A § 3 is the design).
- `docs/routing.md` (§ The Autoroute sheet, § The engine, § Limits, § Results,
  § Code map); `AGENTS.md` (§ Git, § Testing in the app).
- `planning/completed/plan-62-I-avr-landing.md` (the precedent landing letter).

Test command form (`XB`): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p66-dd CODE_SIGNING_ALLOWED=NO test`;
a Check below writes `XB -only-testing:schematicalTests/<Class>`. Debug, scoped to the classes named.

## Prerequisites

The gate for the whole feature is in plan-66-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-66-E is complete and archived | `ls planning/completed/plan-66-E-* \| wc -l` → 1 | NOT MET (2026-11-25: 0) |
| A–D are archived | `ls planning/completed/plan-66-[A-D]-* \| wc -l` → 4 | NOT MET (2026-11-25: 0) |

If plan-66-E is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- A finished run that moved existing copper says so in the sheet's Result
  section ("Moved 3 traces and 1 via aside"); one that moved none says nothing
  about it.
- `docs/routing.md` describes everything plan-66 added, and every number it
  states about push and shove comes from a run recorded here.
- The gates of Phase 3 pass on one tree; the real-app pass shows a push, a clean
  design rule check and one undo restoring the board.

### Non-goals (explicit constraints)

- No engine or option change; a fix for a failed gate belongs to the owning
  letter (and its Corrections).
- No merge or branch operation: the work stays on the branch it was done on until
  the owner asks (`AGENTS.md` § Git).
- No teardrops (plan-66-A § Decisions); `planning/todo.md` keeps them as a later
  item.

## 2. Current State

The sheet's Result section prints "Routed N of M connections, V vias, W mm, S s"
from `model.session.result.stats` (`Routing/AutorouteSheet.swift:162-169`) and
a row per unrouted net. `docs/routing.md` says "No push-and-shove: kept copper
never moves (a deliberate scope decision)" (line 219) and its option table has
no row for the option. `planning/todo.md:118`: "Later: teardrops, push-and-shove
routing. (autorouter: plan-38)". `planning/todo.md` has uncommitted edits by the
owner in the working tree (`git status --short planning/todo.md` → ` M`), so it is
committed by hunk.

### Measured populations

| What | Count | Command |
|---|---|---|
| Router test classes (Debug landing tier) | 25 files | `ls schematical/schematicalTests \| grep -E '^(Rout\|Router\|Autoroute\|Assist)' \| wc -l` → 25 (plus A–E's new files: `RoutingAnchorTests`, `RoutingChainTests`, `RoutingChainEngineTests`, `RoutingPushTests`, `RoutingPushIntegrationTests`, which the count includes once they exist) |
| Doc lines that say push-and-shove is absent | 1 | `grep -n 'No push-and-shove' docs/routing.md` → line 219 |

## 3. Design Overview

Nothing new. The sheet line reads `RoutingStats.movedTraces` and `movedVias`
(set from `result.moved`, plan-66-C) through `AutorouteModel.movedLine: String?`
(nil when both are 0). The docs follow the code's own words: the option's field
name, `RoutingProblem.movable`, `tryPush`, `pushReach`.

**Gate class:** the landing tier is the whole plan's. Option off: byte-identity
(the A anchors) in the Debug tier. Option on: the Release quality runs and the
real-app pass; numbers are **recorded**, with the bounds the tests hold.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. An unticked box means NOT DONE.

### Phase 1 — the sheet line

- [ ] `Routing/AutorouteModel.swift`: `var movedLine: String?` from
      `session.result?.stats`: "Moved 3 traces and 1 via aside", singular/plural as
      the sheet's other lines, nil when both counts are 0.
- [ ] `Routing/AutorouteSheet.swift`: in `results`, under the "Routed N of M"
      text, `if let line = model.movedLine { Text(line).foregroundStyle(.secondary) }`.
- [ ] `schematicalTests/AutorouteModelTests.swift`: `testTheMovedLine`
      (counts 0/0 → nil; 1/0 → "Moved 1 trace aside"; 3/1 → "Moved 3 traces and 1
      via aside"; 0/2 → "Moved 2 vias aside").

Acceptance: the test passes; the sheet compiles.
  Check: `XB -only-testing:schematicalTests/AutorouteModelTests`
  → `** TEST SUCCEEDED **` (est. 2 min).
Commit: —

### Phase 2 — the documents

- [ ] `docs/routing.md`: (a) the option table gets "Push and shove
      (`pushAndShove`) | On / Off | Off | When a connection has no legal way, move
      the unlocked traces and vias of other nets aside, hand-drawn or not (locked
      copper never moves), instead of ripping up and giving up."; (b) § The engine
      gets a step after Negotiation: "Push (`RoutingPush`; with the option): … " in
      the words of plan-66-A § 3 and D § 3 (the corridor, `pushReach`, the all-or-
      nothing restore, the accept rule); (c) § Limits' "No push-and-shove: kept
      copper never moves" sentence is replaced by what is true now: the run's own
      nets' existing copper stays where it is; only copper of other nets and the
      run's own new connections can be pushed; a pushed trace is laid again on the
      grid and cleaned, so a hand-drawn trace that is pushed comes out tidy rather
      than nudged; a connection that ends on another connection (a branch) is
      ripped, not pushed; locked copper, copper touched by other copper away from
      its ends, and copper on a disabled layer is never pushed; the Trace tool's
      Assist does not push; (d) the lead paragraph and "Plan:" line mention plan-66;
      (e) § Code map lists `RoutingPush.swift`, `RoutingCorridor.swift`,
      `MovableChains.swift`; § Tests lists the five new classes.
- [ ] `docs/routing.md` § Results: a "Push and shove" table of AFU, RegFile and LFU
      with the option on and off — routed, vias, corners, wire, seconds, pushes
      tried and kept, pieces moved — from the Phase 3 Release runs (the same
      layout as "Now (2026-11)"); one sentence on what the three boards show.
- [ ] `planning/todo.md`, by hunk (the file has the owner's uncommitted edits):
      replace line 118 with two lines — `- [x] Push-and-shove routing in the
      autorouter. (plan-66)` and `- [ ] Later: teardrops (a board option, not an
      autorouter option; not planned yet).` Commit only those hunks: copy `git show
      HEAD:planning/todo.md` to a temporary file, make the two-line edit there,
      write the diff with `a/planning/todo.md` and `b/planning/todo.md` paths to a
      patch, `git apply --cached` it, and `git commit` with no path argument so the
      owner's other edits stay unstaged; make the same edit in the working file with
      `Edit`.

Acceptance: `grep -c 'No push-and-shove' docs/routing.md` → 0; `grep -c
'pushAndShove' docs/routing.md` → at least 2; the three code-map files and five
test classes each appear (`grep -c 'RoutingPush\|RoutingCorridor\|MovableChains'
docs/routing.md` → at least 3); `git diff --cached --stat` before the commit shows
only `planning/todo.md`.
  Check: the four commands (est. 2 min).
Commit: —

### Phase 3 — the landing tier (once, on the finished tree)

- [ ] Debug, every router test class: `xcodebuild -project
      schematical/schematical.xcodeproj -scheme schematical -derivedDataPath
      /tmp/p66-dd CODE_SIGNING_ALLOWED=NO test` with one `-only-testing:` per
      class in the files `ls schematical/schematicalTests | grep -E
      '^(Rout|Router|Autoroute|Assist)'` lists (the 25 that exist plus A–E's five;
      `Rout…` includes `RouteToolTests`, `RouteDraftTests`, `RouteLegTests`) and
      `TraceRouteTests`, `TraceToolTests` → `** TEST SUCCEEDED **`, `0 failures`;
      record the counts.
- [ ] The A anchors, alone, as the off-identity proof: `… test
      -only-testing:schematicalTests/RoutingAnchorTests` → pass; the literals in the
      file are those A recorded on the pre-plan tree (`git log -p
      schematical/schematicalTests/RoutingAnchorTests.swift` shows no edit to a
      literal after A's first commit).
- [ ] Release quality: `xcodebuild -project schematical/schematical.xcodeproj
      -scheme schematical -configuration Release ENABLE_TESTABILITY=YES
      ONLY_ACTIVE_ARCH=YES CODE_SIGNING_ALLOWED=NO -derivedDataPath /tmp/p66-rel
      test -only-testing:schematicalTests/RouterQualityFixtureTests` → all pass;
      record the `QUALITY` lines (AFU, RegFile, LFU; off and on; Thorough) for the
      docs table.
- [ ] Export and file checks that the change could touch: `… test
      -only-testing:schematicalTests/DesignRuleGoldenTests
      -only-testing:schematicalTests/PCBUndoTests
      -only-testing:schematicalTests/TraceJoinTests` → pass (the DRC, the undo history
      and the join rule read the traces and vias a splice rewrites).
- [ ] Real app, per `AGENTS.md` § Testing in the app and the owner's GUI notes
      (one probe at a time, no modal left up, the app killed after each command):
      build the app; open a copy of `schematical/schematicalTests/Fixtures/RegFile.schematical`
      kept in the scratchpad; Route ▸ Autoroute (⌥⌘R) with the defaults; Close.
      Run `XB -only-testing:schematicalTests/RoutingPushIntegrationTests/testPushOnARoutedBoardKeepsItsConnections`
      and take the first `PUSHCASE` line it prints (net and width). With the Select tool click one trace of that net; open the sheet
      (⌥⌘R; its scope is the selection); set **Push and shove** on, **Existing** to
      "Clear unlocked", **Width** to that width in mil; Route. Read the Result
      section; run the DRC (⇧⌘D); Edit ▸ Undo once. Record: the sheet's "Moved …"
      line, the DRC's router-caused count (0), and that Undo returned the board
      (every trace and via where it was, by the DRC's `unrouted` count and a
      screenshot of the same zoom before and after). Move the pointer to (0, 0)
      when done.
- [ ] Docs name what exists: Phase 2's four commands re-run.

Acceptance: every box above ticked with its observed output recorded beside it.
  Check: the commands above (est. 55 min in all: the Debug tier 15 min of build
  and tests; the Release run about 25 min, of which the Release build is 8 min and
  Thorough on LFU 5 min — nothing smaller runs Balanced on the fixtures, since a
  Debug build is some 50 times slower; the real-app pass 10 min).
Commit: —

### Phase 4 — close-out

- [ ] Fill every phase's `Commit:` in A–F and tick every box; move
      `planning/plan-66-[A-F]-*.md` to `planning/completed/` (A–E as each
      completed; F last); keep any `planning/plan-66-data/` in place.

Acceptance: `ls planning/plan-66-*.md 2>/dev/null | wc -l` → 0; `ls
planning/completed/plan-66-* | wc -l` → 6.
  Check: the two commands (est. 1 min).
Commit: —

## Validation Plan

- This letter is the plan's validation: Phase 3 is the final gate, run once.
- Doc sync: Phase 2 (`docs/routing.md`, `planning/todo.md`). `docs/agent-control.md`
  and `docs/file-format.md` do not change (the option is a user default, not in the
  file; autorouting is not in the automation surface).

## Decisions

plan-66-A § Decisions apply. Added by the author, 2026-11-25:

- **The sheet reports what moved, from the engine's own counters, in one
  sentence** — a user who enabled the option sees what it did. (§ 3)
- **Landing runs once, here, in two builds** — Debug for the router classes,
  Release for the quality runs. (§ Phases, Phase 3)

## Corrections

<!-- Filled during execution: each gate's observed output, and any defect found
     and the letter that owned its fix. -->

## Summary

F carries no design. Its value is the numbers: the first measurement of the
option on the three fixtures, recorded in the docs, and a pass on the real app
that a unit test cannot give.
