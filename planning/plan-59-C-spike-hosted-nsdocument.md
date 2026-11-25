# plan-59-C: Spike — a hosted `NSDocument`

Last updated: 2026-11-24
Effort: large (3h–1d)
Depends on: plan-59-B (prerequisites are stated in plan-59-A § Prerequisites)

Builds candidate **H** — an `NSDocument` subclass whose window controller hosts
`ContentView` in an `NSHostingController`, with the SwiftUI `App` keeping the
auxiliary scenes, Settings and Commands — and runs the whole probe list
(F, U, A) against it, so plan-59-D compares two measured candidates. The
single outcome: `findings.md` has an H verdict and evidence for every probe,
and H's code is saved as a patch.

References:

- `planning/plan-59-data/probes.md` (all tables), `findings.md`.
- plan-59-A § 2 and § 4 (the current state and the model H reuses).
- The spike's G results (`findings.md`, G columns): H's tasks reuse the G
  worktree's `SchematicDocumentModel` pieces and the child-window registry.
- `schematical/schematical/Document/SchematicFileController.swift`,
  `Document/UndoCommands.swift`, `Automation/`, `schematicalApp.swift`.

## Prerequisites

Stated in plan-59-A § Prerequisites. In addition:

| Must be true | Command | Status |
|---|---|---|
| plan-59-B is complete | `rg -c '^- \[ \]' planning/plan-59-B-spike-documentgroup-app.md` → no output | NOT MET until B lands |
| Every G probe has a verdict | `rg -c '^\| [FUA][0-9]+ \| [^|]+\| (works\|workaround\|blocked) \|' planning/plan-59-data/findings.md` → `27` (F1–F7, U1–U5, A1–A15) | NOT MET until B lands |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run before you continue and before you stop; report all prerequisites if
> you stop.

## 1. Goal

- H exists as a signed, sandboxed, separately-identified build in
  `.claude/worktrees/spike-59-H` (a second detached worktree of `2d10a029`).
- `findings.md` has an H verdict and evidence for every probe row (F1–F7,
  U1–U5, A1–A15) and the A16 table has H's seams.
- `planning/plan-59-data/spike-H.patch` holds the diff.

### Non-goals (explicit constraints)

Same as plan-59-A: nothing outside `planning/plan-59*` changes on `main`; no
branch; the file format unchanged; the installed app untouched; no decision
between G and H here.

## 2. Current State

H is a different integration with the same app. What is established (all
from code or the SDK, none from running H):

- `NSDocument` and `NSDocumentController` own file coordination, autosave,
  versions, recents, the save panel, the close and quit prompts and window
  restoration; their API takes `fileURL` directly on read and write
  (`read(from:ofType:)`, `write(to:ofType:)`), so F1 and F2 do not have G's
  URL gap.
- H keeps the SwiftUI `App` for `Window`, `WindowGroup(for:)`, `Settings` and
  `Commands`, with documents managed by `NSDocumentController`.
  `@Environment(\.openWindow)` and `.focusedSceneValue` are scene features;
  A2 and A5 test what replaces them.
- In H the schematic group and the 7 child-window groups leave the App (the
  documents and the child windows are AppKit windows); the other 6 scene
  declarations, `Settings` and the 9 `Commands` structs stay.

### Measured populations

| What | Count | Command |
|---|---|---|
| Scene declarations in the App | 14: the schematic group, 7 child windows keyed by `sessionID`, 6 others | `rg -n 'WindowGroup\(\|^\s*Window\(' schematical/schematical/schematicalApp.swift \| wc -l` → 14; `rg -c 'let sessionID: UUID' schematical/schematical \| wc -l` → 7 |
| Probe rows to run for H | 27, plus the A16 table | `rg -c '^\| [FUA][0-9]+ \|' planning/plan-59-data/findings.md` → 27 |

### Verified properties

- `NSDocument.h` (MacOSX26.2 SDK) documents `readFromURL:ofType:error:`, `updateChangeCount:` and the document's undo manager feeding the change count, so F1/F2 get the URL from the document and U1/U2 can be driven by its undo manager. Read at `AppKit.framework/Headers/NSDocument.h` lines 27, 122, 207, 226.
- `NSDocument` is not a SwiftUI type, so H's document is not `ObservableObject`
  and needs no bridge to `SchematicDocument`'s Observation: verified by
  `SchematicDocument` being `@Observable` (`SchematicModel.swift:1443–1446`),
  which a view reads directly.

## 3. Design Overview

Pieces: `SchematicNSDocument` (model, file I/O, undo manager),
`SchematicWindowController` (hosting), a document-controller bootstrap in the
App, and the same probe list. The 59-A model's pieces (the decode-and-apply
helper, `encodedSnapshot`) are copied, not shared: each worktree stands alone.

Risk concentrates in the SwiftUI features that assume a scene: A2 (commands
reaching the focused document), A5 (child windows opened without `openWindow`),
A7 (toolbar and title in an `NSHostingController` window), A10 (restoration).
Where a feature is a scene feature, H's workaround is AppKit: responder-chain
actions for commands, `NSHostingController` plus window controller APIs for
the toolbar, `NSDocument` restorable state. Those count as `workaround` under
the rule in `probes.md`; delegate proxying does not.

Rejected: sharing code between the two worktrees (the patches would no longer
be independent evidence).

## 4. Detailed Design

- `final class SchematicNSDocument: NSDocument`: `autosavesInPlace` true;
  `read(from:ofType:)` decodes and applies with `url`; `data(ofType:)` encodes
  `SchematicFile(document:fileURL: fileURL)`; `makeWindowControllers()` adds a
  `NSWindowController` whose content is `NSHostingController(rootView:
  ContentView(model: …))`; `undoManager` is the document's.
- `Info.plist`: `CFBundleDocumentTypes` with `NSDocumentClass`
  `$(PRODUCT_MODULE_NAME).SchematicNSDocument` for the schematic type.
- `schematicalApp.swift`: the schematic `WindowGroup` is removed; an
  `NSApplicationDelegateAdaptor` sets `NSDocumentController.shared` at launch
  so documents exist before SwiftUI scenes; Commands act on the frontmost
  document through `NSApp.sendAction` to the responder chain.
- The 7 child windows are `NSWindow`s with an `NSHostingController`, opened
  by a `ChildWindowCenter` that the hosted `ContentView` calls (there is no
  scene `openWindow` inside a hosted view); each is keyed by the model-id
  registry from plan-59-B § 4 and closes with its document.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` for partial. Moot tasks keep their line with the
> evidence. Fill `Commit:` when it lands. An unticked box means NOT DONE.

### Phase 1 — The H worktree and document

- [ ] `git worktree add --detach /Users/justinzaun/Development/logic/.claude/worktrees/spike-59-H 2d10a029` (script file in the scratchpad); apply the same product name, module name, bundle id (`jgz.schematical.spike59h`) and ad-hoc signing edits as plan-59-A phase 1, in this worktree's `project.pbxproj`; build to `<scratch>/dd-H`.
- [ ] Add `SchematicNSDocument`, the window controller and `Info.plist` document types (§ 4); wire the App.
- [ ] Build and launch; open `examples/6502.schematical` with `open -a`.

Acceptance: H shows 6502 in a window named after the file.
  Check: `osascript -e 'tell application "System Events" to tell process "SchematicalSpike59" to get name of every window'` (the process name is the product name; both worktrees use it, so run one app at a time) → contains `6502` (est. 15 min build, 3 min check).
Commit: —

### Phase 2 — File and undo probes (F1–F7, U1–U5)

- [ ] Run F1–F7 and U1–U5 against H exactly as `probes.md` defines them (the fixture, the harness mains and the baselines from plan-59-A are reused from `planning/plan-59-data/`). Record verdicts and evidence in the H columns.

Acceptance: 12 H verdicts.
  Check: `rg -c '^\| (F[1-7]|U[1-5]) \| [^|]+\| [^|]+\| [^|]+\| (works|workaround|blocked) \|' planning/plan-59-data/findings.md` → `12` (est. 4 h).
Commit: —

### Phase 3 — App probes (A1–A16)

- [ ] Run A1–A15 against H as `probes.md` defines them; fill the A16 table's H seams. Record verdicts and evidence.
- [ ] `git -C <wt-H> diff 2d10a029 > planning/plan-59-data/spike-H.patch`.

Acceptance: 15 H verdicts for A1–A15, the A16 table has an H seam per row, the patch exists.
  Check: `rg -c '^\| A[0-9]+ \| [^|]+\| [^|]+\| [^|]+\| (works|workaround|blocked) \|' planning/plan-59-data/findings.md` → `15`; `git apply --check planning/plan-59-data/spike-H.patch` in a throwaway detached worktree of `2d10a029` → exit 0 (est. 5–6 h).
Commit: —

## Validation Plan

- Tests: none; evidence only.
- Coverage check: the two `rg -c` totals above (12 + 15) equal the F/U/A rows.
- Runtime proof: the evidence cells.
- Doc sync: none.
- Final gate (once): `git show --name-only --format= <each commit of this plan> | rg -v '^planning/plan-59' | wc -l` → `0` (est. 1 min).

## Decisions

- H is `NSDocument` + `NSHostingController` inside the SwiftUI `App`; a full AppKit app lifecycle (`main.swift` + nibs) is not probed, because the app's 14 scenes and 9 `Commands` are SwiftUI — decided by the author, 2026-11-24.
- H gets the same probes and the same verdict rules as G — decided by the author, 2026-11-24.

## Corrections

Filled in during execution.

## Summary

H's risk is the SwiftUI features that expect a scene (commands, `openWindow`,
toolbar, restoration). H has no URL gap. Nothing on `main` changes.
