# plan-59-D: Spike — the decision and the migration plan

Last updated: 2026-11-24
Effort: medium (1h–2h)
Depends on: plan-59-C (prerequisites are stated in plan-59-A § Prerequisites)

Turns the findings of plan-59-A, -B and -C into the plan the spike exists to
produce: the migration of the app from `SchematicFileController` to the
system's document architecture. It applies the selection rule in
`planning/plan-59-data/probes.md` to `findings.md`, asks the owner the
questions the findings raise (once, in one batch), writes the migration plan
with the `write-plan` skill, and removes the spike worktrees.

The single outcome: a migration plan (`planning/plan-NN-*.md`, NN the next
free number when it is written) that an implementer can execute phase by
phase, whose design is the chosen candidate, and whose every question has
been answered by the owner.

References:

- `planning/plan-59-data/probes.md` (§ Selection rule), `findings.md`.
- The three patches in `planning/plan-59-data/` (`spike-G-core.patch`,
  `spike-G-app.patch`, `spike-H.patch`): the working code the migration plan
  cites.
- The `write-plan` skill (`~/.claude/skills/write-plan/SKILL.md`).
- AGENTS.md § Git (a new plan is one commit: all its lettered sub-plans
  together).

## Prerequisites

Stated in plan-59-A § Prerequisites. In addition:

| Must be true | Command | Status |
|---|---|---|
| plan-59-C is complete | `rg -c '^- \[ \]' planning/plan-59-C-spike-hosted-nsdocument.md` → no output | NOT MET until C lands |
| Every probe has both verdicts | `rg -c '^\| [FUA][0-9]+ \| [^|]+\| (works\|workaround\|blocked) \| [^|]+\| (works\|workaround\|blocked) \|' planning/plan-59-data/findings.md` → `27` | NOT MET until C lands |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run before you continue and before you stop; report all prerequisites if
> you stop.

## 1. Goal

- `findings.md` § Selection records the verdict counts per candidate and the
  candidate the rule picks, with the rule step that picked it.
- Every `blocked` row and every visible behavior change in the findings has an
  owner answer, recorded as a decision in the migration plan.
- The migration plan exists, passes the `write-plan` scrub, and names the
  spike patches as its reference implementation.
- Both spike worktrees are gone; plan-59-A–D are archived.

### Non-goals (explicit constraints)

- No code on `main` changes: the migration plan describes the work; executing
  it is that plan's job.
- No second spike. If a probe's verdict is `blocked`, it is an owner question
  or a documented limit of the migration plan, not a reason to probe again.
- `planning/todo.md` is not touched (the owner has uncommitted edits in it).

## 2. Current State

At this point `findings.md` is complete (plan-59-A–C). The base commit
`2d10a029` is no longer `main`'s head; the migration plan is written against
`main` as it is then, so its populations are measured again in phase 3, not
copied from plan-59-A.

### Measured populations (to measure again at writing time)

| What | Command |
|---|---|
| Controller lines | `wc -l schematical/schematical/Document/SchematicFileController.swift` |
| Files naming the controller, and test constructor sites | `rg -c 'SchematicFileController' schematical` |
| Docs that name it | `rg -l 'SchematicFileController' docs` (at 2d10a029: `docs/export.md`, `docs/agent-control.md`, `docs/file-format.md`) |
| Scene declarations and `sessionID` lines | `rg -c` as in plan-59-A § 2 |

### Verified properties

None beyond `findings.md`, which is the verified record.

## 3. Design Overview

Three steps, none conditional on a result nobody has seen: apply the
selection rule (a fixed procedure over a finished table), ask the owner the
questions the table produces, write the plan from the answers. The rule is in
`probes.md`; this plan does not restate or modify it.

Where the risk concentrates: step 2. A `blocked` row changes what the user sees; the migration plan must not carry it unanswered.

Rejected: writing the migration plan during plan-59-C (the H results would not
exist); choosing by taste instead of the rule.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` for partial. Moot tasks keep their line with the
> evidence. Fill `Commit:` when it lands. An unticked box means NOT DONE.

### Phase 1 — Apply the selection rule

- [ ] In `findings.md` add `## Selection`: per candidate, counts of `works`, `workaround`, `blocked` over the 27 rows (the `info` rows F7 and A15 reported separately), and the list of `blocked` rows.
- [ ] Apply `probes.md` § Selection rule steps 1–5 in order; write the candidate it picks and the step number.

Acceptance: § Selection names a candidate and a rule step.
  Check: `rg -c '^Chosen: (G|H) \(rule step [1-5]\)' planning/plan-59-data/findings.md` → `1` (est. 20 min).
Commit: —

### Phase 2 — The owner's questions, in one batch

- [ ] List every question the findings produce: one per `blocked` row of the chosen candidate; and one per `workaround` row of the chosen candidate whose evidence cell lists a visible behavior change (menu items, tabs, restoration, prompts, window titles).
- [ ] Ask them together with the question tool, each with a recommendation and the concrete consequence of each option, citing the evidence cell.
- [ ] Record each answer as a line in `findings.md` § Owner answers: `Q: … → A: … (date)`.

Acceptance: every listed question has an answer line.
  Check: `rg -c '^Q: .* → A: ' planning/plan-59-data/findings.md` → equals the number of questions listed in § Selection (est. 30 min with the owner).
Commit: —

### Phase 3 — Write the migration plan

- [ ] Follow the `write-plan` skill end to end. Re-check the next free plan number first (`ls planning planning/completed | rg '^plan-[0-9]+' | sort -u | tail -3`); other sessions land plans on `main`.
- [ ] Re-measure the populations in § 2 on current `main` and put them, with commands, in the new plan.
- [ ] Cover at least these areas, each as phases with acceptance checks from the code the patches show: the document class; undo moved to one `NSUndoManager` per document (the per-page lists in `UndoHistory.swift` and `UndoCommands` retired, the page-independence cases in `PCBUndoTests` rewritten); file I/O (relative load-file paths, sandbox, autosave); menus and commands; export moved to File › Export; child windows bound to the document; tabs, restoration, close and quit prompts replacing `WindowCloseGuard`, `tabNextWindow` and `confirmQuit`; automation; the 9 test constructor sites; Info.plist document types and `.schpart`; deleting `SchematicFileController`'s replaced members; doc sync (`docs/export.md`, `docs/agent-control.md`, `docs/file-format.md`).
- [ ] The new plan's reference section cites `planning/plan-59-data/` and the patches; its prerequisites include "plan-59 complete".
- [ ] Run the scrub: `rg -n 'UNVERIFIED|UNMEASURED|TBD|Open Decision|whether|confirm that|recommended' planning/plan-<NN>-*.md` → no output.
- [ ] Commit the new plan's files together, one commit (AGENTS.md § Git), staging only those files.

Acceptance: the plan exists and the scrub is clean.
  Check: `ls planning/plan-<NN>-*.md` → at least one file; the scrub command above → no output (est. 1.5 h).
Commit: —

### Phase 4 — Close out the spike

- [ ] Remove both worktrees with the scratchpad script: `git worktree remove --force /Users/justinzaun/Development/logic/.claude/worktrees/spike-59-G` and `…/spike-59-H`; `git worktree prune`.
- [ ] Delete the spike apps' sandbox data that can be removed: `ls ~/Library/Containers | rg 'spike59'`, remove what the system allows and name what it does not.
- [ ] `git mv planning/plan-59-A-spike-document-core.md planning/plan-59-B-spike-documentgroup-app.md planning/plan-59-C-spike-hosted-nsdocument.md planning/plan-59-D-spike-decision-and-migration-plan.md planning/completed/`; `planning/plan-59-data/` stays beside the migration plan that cites it.
- [ ] Tick every box in the four files and fill every `Commit:` line.

Acceptance: no spike worktree remains; the four plans are archived.
  Check: `git worktree list | rg spike-59 | wc -l` → `0`; `ls planning/completed/plan-59-*.md | wc -l` → `4` (est. 10 min).
Commit: —

## Validation Plan

- Tests: none; the product is a plan.
- Coverage check: the migration plan's area list above is compared with the
  A, F and U probe rows by eye once; each row of `findings.md` is cited by a
  phase or an owner answer.
- Runtime proof: not applicable.
- Doc sync: none in this plan; the migration plan carries it.
- Final gate (once): `git show --name-only --format= <each commit of this plan> | rg -v '^planning/' | wc -l` → `0` (est. 1 min).

## Decisions

- The candidate is chosen by the rule in `probes.md`, fixed before any probe ran — decided by the author, 2026-11-24, so the choice cannot follow the result it is judged on.
- The owner is asked once, after the findings are complete, before the migration plan is written — decided by the `write-plan` skill and the owner's request for a plan as the spike's end result, 2026-11-24.

## Corrections

Filled in during execution.

## Summary

The risk is the owner's questions: a `blocked` row changes what users see, and the migration plan must carry the answers, not the
questions. Nothing on `main` changes in this plan except `planning/`.
