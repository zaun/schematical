# plan-59-B: Spike — `DocumentGroup` in the whole app

Last updated: 2026-11-24
Effort: large (3h–1d)
Depends on: plan-59-A (prerequisites are stated there, § Prerequisites)

Takes the G build from plan-59-A and answers the app-integration probes A1–A16
for it: menus, commands, exports, print, child windows, lifetime, title bar,
tabs, opening paths, restoration, close and quit prompts, versions,
automation, performance, other scenes and test seams. The single outcome:
`findings.md` has a G verdict and evidence for every A probe, and the code is
saved as a second patch.

References:

- `planning/plan-59-data/probes.md` (the A table, verdict rules, driving rules).
- `schematical/schematical/schematicalApp.swift`,
  `Document/SchematicFileController.swift:708–780` (`SchematicFileCommands`),
  `Document/SchematicExportCommands.swift`, `Document/UndoCommands.swift`,
  `Memory/MemoryWindow.swift:25` (`SchematicSessions`),
  `Automation/AutomationSession.swift`, `Automation/AutomationTools.swift:195–252`.
- plan-59-A, § 4 (the model and the scene).

## Prerequisites

Stated in plan-59-A § Prerequisites. In addition:

| Must be true | Command | Status |
|---|---|---|
| plan-59-A is complete | `rg -c '^- \[ \]' planning/plan-59-A-spike-document-core.md` → no output (no unticked box) | NOT MET until A lands |
| The G worktree exists | `git -C /Users/justinzaun/Development/logic/.claude/worktrees/spike-59-G rev-parse HEAD` → `2d10a029…` | NOT MET until A lands |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue and before you stop; report all
> prerequisites if you stop.

## 1. Goal

- A verdict and evidence for G on A1–A16 in `findings.md` (A16 is a table).
- `planning/plan-59-data/spike-G-app.patch`: the worktree's diff on top of
  `spike-G-core.patch`'s state.

### Non-goals (explicit constraints)

Same as plan-59-A: nothing outside `planning/plan-59*` changes on `main`, no
branch, no change to the file format, the installed app untouched. No
decision between G and H.

## 2. Current State

What the app does today that A1–A16 test, each read from the code:

- Windows: `WindowGroup(id: "schematic")` and 13 other scene declarations
  (`schematicalApp.swift`, 14 total; the 13 non-schematic ones have
  `.restorationBehavior(.disabled)`).
- Menus: `SchematicFileCommands` replaces `.newItem` with New Project and adds
  Open…, Open Part…, Save, Save As…, Print…, Export ▸ (Image, PDF, Gerber PCB),
  Part items, from `@FocusedValue(\.schematicFile)`; its comment records that
  `.saveItem` never showed in a `WindowGroup` app. `UndoCommands` replaces
  `.undoRedo`. The system's Print is removed in `SimulationCommands`.
- Child windows: Memory, Processor, Registers, Terminal, BOM, DRC and Board
  Preview are `WindowGroup(for: Target.self)` keyed by `sessionID`
  (27 lines, `rg -c sessionID schematical/schematical` summed), found through
  `SchematicSessions.shared` (`MemoryWindow.swift:25`), registered in
  `ContentView.onAppear` (`ContentView.swift:764`) and dismissed in
  `.onDisappear` (`:780–797`).
- Title bar: `.windowToolbarStyle(.unifiedCompact(showsTitle: false))`,
  `.navigationTitle(file.displayName)` (`ContentView.swift:813`).
- Automation: `AutomationTools.sessions()` maps `SchematicSessions` to
  contexts; `new_window`, `open`, `save` call `SchematicFileController`
  (`AutomationSession.swift:56–110`); `focus_window` uses `file.window`.
- Tests: 9 constructor sites in 8 files (measured in plan-59-A).

### Measured populations

| What | Count | Command |
|---|---|---|
| Child-window scene kinds keyed by `sessionID` | 7 | `rg -c 'let sessionID: UUID' schematical/schematical \| wc -l` → 7 |
| Automation lines using the file controller | 11 | `rg -c 'file\??\.' schematical/schematical/Automation` → 9 + 2 |
| Commands structs in the `.commands` list | 9 | `rg -c '^\s+[A-Z][A-Za-z]+Commands\(\)' schematical/schematical/schematicalApp.swift` → 9 |

### Verified properties

Those of plan-59-A § 2. Nothing else is claimed about G's behavior; the
probes establish it.

## 3. Design Overview

Four phases, each a group of probes that share setup; later phases need the
earlier ones' scene and command changes.

Risk concentrates in A2 (`Commands` acting on the focused window's document), A5 (child windows bound to a document without `sessionID`), and A8
and A10 (tabs and restoration done by the system, not by code). Rejected:
making `SchematicSessions` and `sessionID` survive as they are; the spike
replaces them with the document's identity so A5 is tested the way the
migration would be built.

A14's gate is B1 within 10%. The harness hosts the canvas without a scene, so
A14 guards against accidental regressions in views and environment, not
against scene cost; scene cost is read from A8/A10 behavior.

## 4. Detailed Design

- Document identity for child windows: a `Hashable` target carrying the model's
  `id` (a `UUID` the model creates); a child resolves its model through a
  weak registry keyed by that id, filled by the model's `init`, so a child
  never keeps a document alive.
- Commands: `@FocusedValue` of the model set from the editor view with
  `.focusedSceneValue`; A2's evidence names the mechanism that reached the
  `Commands`.
- Exports move to File › Export: Image, PDF, Gerber, KiCad Project, EasyEDA
  Pro. The save panel is the system's (owner's principle, § Decisions).
- Automation goes through `NSDocumentController.shared`, mapping each
  document to its model through the registry above.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` for partial. Moot tasks keep their line with the
> evidence. Fill `Commit:` when it lands. An unticked box means NOT DONE.

### Phase 1 — Menus, commands, export, print (A1–A4)

- [ ] Worktree `Document/SchematicFileController.swift` `SchematicFileCommands`: remove the New/Open/Save/Save As items so the system's File menu provides them; keep Part items, Print and Export; add KiCad Project and EasyEDA Pro to `SchematicExportMenu` (`Document/SchematicExportCommands.swift`), each through a system save panel.
- [ ] Publish the focused model to `Commands` and make `SchematicFileCommands` act on it. Keep `UndoCommands` out of the worktree's `.commands` list (plan-59-A phase 4 removed it) so Edit › Undo and Redo are the system's; list them in A1's evidence.
- [ ] A4: connect File › Print to the system print flow (`NSPrintOperation` from the document's window, producing `PrintExport`'s pages) and record the result.
- [ ] Run A1 (list the menu with `osascript`, keys unique), A2 (two documents), A3 (KiCad and EasyEDA in the sandbox), A4. Record verdicts and evidence.

Acceptance: verdicts for A1–A4.
  Check: `rg -c '^\| A[1-4] \| [^|]+\| (works|workaround|blocked) \|' planning/plan-59-data/findings.md` → `4` (est. 3 h).
Commit: —

### Phase 2 — Windows (A5–A8)

- [ ] Replace `sessionID` and `SchematicSessions` in the worktree with the model-id target and weak registry (§ 4) for the seven child windows.
- [ ] Delete from the worktree the title and tab code the system now provides: `.navigationTitle(file.displayName)`, `tabNextWindow`, `WindowCloseGuard` use.
- [ ] Run A5 (two documents, all seven children), A6 (close with a running simulation; Revert), A7 (before/after screenshots at `evidence/A7-before.png`, `evidence/A7-G.png`), A8. Record.

Acceptance: verdicts for A5–A8.
  Check: `rg -c '^\| A[5-8] \| [^|]+\| (works|workaround|blocked) \|' planning/plan-59-data/findings.md` → `4` (est. 4 h).
Commit: —

### Phase 3 — Lifecycle (A9–A12)

- [ ] Run A9 (cold, warm, two files, Open Recent after relaunch, Dock drop), A10 (two documents, one edited; quit; relaunch; record the page shown after relaunch, then add `SceneStorage` for the page and record it again), A11 (⌘W, ⌘Q, Cancel; with a running simulation), A12 (Revert To, Browse All Versions, Lock, Duplicate).
- [ ] Remove `AppDelegate.applicationShouldTerminate` and `confirmQuit` from the worktree so A11 tests the system's behavior; keep `applicationShouldTerminateAfterLastWindowClosed` as it is.

Acceptance: verdicts for A9–A12.
  Check: `rg -c '^\| A(9|10|11|12) \| [^|]+\| (works|workaround|blocked) \|' planning/plan-59-data/findings.md` → `4` (est. 4 h).
Commit: —

### Phase 4 — Automation, performance, other scenes, test seams (A13–A16)

- [ ] A13: rewrite `AutomationTools.sessions()`, `new_window`, `open`, `save`, `focus_window` in the worktree on `NSDocumentController.shared` and the registry; run `list_windows`, `new_window`, `open`, `save`, `focus_window` through the MCP server on `127.0.0.1:47800` (Settings › AI Agents; the spike's own defaults domain, so the server is switched on there once); compare outputs with base. Record.
- [ ] A14: re-run B1 on the worktree's sources. Record both numbers.
- [ ] A15: open Settings, User Library, Verify Datasheet, a Part Editor, Help. Record.
- [ ] A16: for each of the 9 constructor sites in 8 test files, list the members used (`rg -n 'controller\.|file\.' <file>`) and the seam that replaces them; fill the A16 table in `findings.md`.
- [ ] `git -C <wt> diff 2d10a029 > planning/plan-59-data/spike-G-app.patch`.

Acceptance: verdicts for A13–A15, the A16 table filled, the patch saved.
  Check: `rg -c '^\| A(13|14|15) \| [^|]+\| (works|workaround|blocked) \|' planning/plan-59-data/findings.md` → `3`; `sed -n '/^## A16/,/^## Notes/p' planning/plan-59-data/findings.md | rg -c '^\| — \|'` → no output (the placeholder row is gone) (est. 3 h).
Commit: —

## Validation Plan

- Tests: none; evidence only.
- Coverage check: A1–A16 each have a row (`rg -c` checks above).
- Runtime proof: the evidence cells.
- Doc sync: none.
- Final gate (once): `git show --name-only --format= <each commit of this plan> | rg -v '^planning/plan-59' | wc -l` → `0` (est. 1 min).

## Decisions

- Export formats (KiCad, EasyEDA Pro) move from the Save panel's Format menu to File › Export, and the save panel is the system's — decided by the owner's principle (use the built-in tooling properly), 2026-11-24. A3 tests the new path.
- Child windows are bound to the document's identity, not a session id — decided by the author, 2026-11-24.

## Corrections

Filled in during execution.

## Summary

The risk is in how much of today's window, tab, close and title code the
system replaces without help (A8, A10, A11), and in commands reaching the
focused document (A2). Nothing on `main` changes.
