# plan-59-A: Spike — a `DocumentGroup` document core

Last updated: 2026-11-24
Overall Effort: x-large (1d–3d)
Effort: large (3h–1d)
Depends on: nothing

Plan 59 is a spike. Its deliverable is a plan, not a feature: a migration of
the app from the hand-built `SchematicFileController` to the system's document
architecture, chosen on evidence. The owner's direction: use the built-in
tooling properly, even when it is more work; this is first and foremost a
macOS app.

Two candidates are built in throwaway worktrees and put through one probe list
(`planning/plan-59-data/probes.md`): **G**, SwiftUI `DocumentGroup` with a
`ReferenceFileDocument`; **H**, an `NSDocument` subclass hosting the SwiftUI
view. This sub-plan builds the document core of G and answers the file and
undo/dirty probes (F1–F7, U1–U5; U3 is one `NSUndoManager` per document) for it. plan-59-B answers the app-integration
probes for G, plan-59-C builds and probes H, plan-59-D applies the selection
rule and writes the migration plan.

The single outcome of 59-A: `planning/plan-59-data/findings.md` has a verdict
and evidence for G on F1–F7 and U1–U5, plus the baselines B1–B3, and the code
that produced them is saved as a patch.

References:

- `planning/plan-59-data/probes.md` — probe catalogue, verdict rules,
  selection rule, how to drive the real app. Read first.
- `planning/plan-59-data/findings.md` — where results go.
- `schematical/schematical/Document/SchematicFileController.swift` — what is
  being replaced.
- `schematical/schematical/Canvas/UndoHistory.swift`,
  `Canvas/SchematicModel.swift:2021–2135` — the per-page undo.
- `docs/file-format.md` — the file format; this spike does not change it.
- AGENTS.md § Git and § Testing in the app.

## Prerequisites

These are a precondition on the whole of plan 59, stated once here; the other
letters point here.

| Must be true | Command | Status |
|---|---|---|
| The base commit exists | `git cat-file -t 2d10a029` → `commit` | MET |
| Xcode 26 toolchain | `xcodebuild -version \| head -1` → `Xcode 26.3` | MET |
| System Events scripting works | `osascript -e 'tell application "System Events" to count processes'` → a number | MET (125) |

Everything below is written against these holding.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites.**

## 1. Goal

- G exists as a signed, sandboxed, separately-identified build of the real
  app in `.claude/worktrees/spike-59-G`, opening and saving real
  `.schematical` files through `DocumentGroup`.
- `findings.md` has a verdict (`works`, `workaround`, `blocked`) and evidence
  for G on F1–F7 and U1–U5, and values for B1–B3.
- `planning/plan-59-data/spike-G-core.patch` holds the worktree's diff against
  the base commit.

### Non-goals (explicit constraints)

- Nothing under `schematical/`, `backend/`, `docs/` or `examples/` on `main`
  changes. Every commit of this plan touches only `planning/plan-59*`.
- No branch is created, switched or deleted (AGENTS.md § Git). The worktree is
  detached.
- The `.schematical` file format does not change. F3 proves it.
- The user's installed app is untouched: the spike has its own bundle id and
  product name, and `.schematical`'s default handler is never changed.
- Nobody is asked for anything and no decision between G and H is made here;
  plan-59-D makes it.

## 2. Current State

The file layer is `SchematicFileController` (844 lines,
`wc -l schematical/schematical/Document/SchematicFileController.swift` → 844),
one per window, held as `@State` in `ContentView` (`ContentView.swift:55`,
built in `ContentView.init` at `:98–113`). It does by hand what `NSDocument`
does:

- Dirty state: `hasUnsavedChanges` compares `SchematicFile(document:fileURL:)`
  with `savedSnapshot` (`SchematicFileController.swift:116`).
- Save, Save As (`:123`, `:134`), Open (`:196`, `:206`), the panels, and the
  Format accessory that mixes saves (Schematical) with exports (KiCad, EasyEDA
  Pro): `SaveFormatPicker`.
- Close: `WindowCloseGuard` (`:784`) stands in front of SwiftUI's window
  delegate to answer `windowShouldClose`. Quit: `confirmQuit()` (`:103`),
  called from `AppDelegate.applicationShouldTerminate`
  (`schematicalApp.swift:26–31`).
- Tabs: `pendingOpens`, `claimPendingOpen`, `tabNextWindow(into:)`
  (`:33–276`), which watches `didBecomeKey` to join a tab group.
- Windows are `WindowGroup(id: "schematic")` (`schematicalApp.swift:42`).
  `Info.plist` exports the types `jgz.schematical.schematic` and
  `jgz.schematical.part` but declares no document types, and nothing handles
  an open-file event (`rg -n 'onOpenURL|handlesExternalEvents|application\(_.*open' schematical`
  → no match).

Undo is the app's own, per page: `UndoHistory.swift` (a step is a per-item
diff; the Schematic and PCB pages each keep an undo and a redo list; the
owner has decided undo becomes per document on one `NSUndoManager`),
`SchematicDocument.undo(on:)` at `SchematicModel.swift:2096`, Edit › Undo
routed by `UndoCommands` (`Document/UndoCommands.swift`). `NSUndoManager` is
not used anywhere (`rg -c 'undoManager|UndoManager' schematical/schematical | wc -l` → 0).

`SchematicDocument` is a `@MainActor @Observable` class
(`SchematicModel.swift:1443–1446`), not a Combine `ObservableObject`.
`SchematicFile(document:fileURL:)` (`SchematicFile.swift:743–749`) and
`SchematicFile.apply(to:catalog:fileURL:)` (`:964`) both take the file's URL,
because an EEPROM's load-file path is stored relative to the schematic
(`:783`, `:1050`, `Memory/MemoryFileAccess.swift`).

The harness (`scripts/harness/build.sh`) compiles every app source except
`schematicalApp.swift` plus a `main.swift` into a CLI inside a copy of the
Debug app, so a harness main can call any non-scene code.

### Measured populations

| What | Count | Command |
|---|---|---|
| Lines in the controller | 844 | `wc -l schematical/schematical/Document/SchematicFileController.swift` → 844 |
| Files outside the controller that name it | 7 files, 10 lines | `rg -c 'SchematicFileController' schematical/schematical --glob '!Document/SchematicFileController.swift'` → 7 files, sum 10 |
| Scene declarations in the App | 14, of which 13 have `.restorationBehavior(.disabled)` | `rg -n 'WindowGroup\(\|^\s*Window\(' schematical/schematical/schematicalApp.swift \| wc -l` → 14; `rg -c 'restorationBehavior\(.disabled\)' …` → 13 |
| Example schematics | 9 | `ls examples/*.schematical examples/*/*.schematical \| wc -l` → 9 |
| Largest example | 242335 bytes (`examples/6502.schematical`) | `ls -l examples/6502.schematical` |
| Test constructor sites of the controller | 9 in 8 files | `rg -c 'SchematicFileController\(document:' schematical/schematicalTests` → sum 9, 8 files |

### Verified properties

| Claim | How verified |
|---|---|
| `ReferenceFileDocument` requires `Combine.ObservableObject` and `Sendable`; has `Snapshot`, `init(configuration:)`, `snapshot(contentType:)`, `fileWrapper(snapshot:configuration:)` | Read `SwiftUI.swiftinterface` (MacOSX26.2 SDK), lines 7979–7988 |
| Its read configuration holds only `contentType` and `file` (a `FileWrapper`); its write configuration only `contentType` and `existingFile`. Neither has a URL | Same file, lines 6058–6071 |
| The editor view's configuration has `fileURL`; `EnvironmentValues.documentConfiguration?.fileURL` exists (macOS 14) | Same file, lines 8007–8022 and 16046–16066 |
| `OpenDocumentAction` (`callAsFunction(at:) async throws`) and `NewDocumentAction` exist as environment values | Same file, lines 3134–3163 |
| An ad-hoc signed `app-sandbox` app runs sandboxed on this machine | Probe app built with `swiftc`, `codesign --sign - --entitlements`, launched: container `~/Library/Containers/jgz.sbxprobe` appeared and a read of `~/.zshrc` returned "blocked". (Its container could not be removed: `rm` → "Operation not permitted".) |
| The base project signs with team `8BU2Y8C5J7`, automatic style, sandbox on, deployment target 15.8 | `rg -n 'DEVELOPMENT_TEAM\|CODE_SIGN_STYLE\|ENABLE_APP_SANDBOX\|MACOSX_DEPLOYMENT_TARGET' schematical/schematical.xcodeproj/project.pbxproj` |
| `.claude/` is git-ignored, so a worktree there is not seen by `git status` | `git check-ignore -v .claude/worktrees/x` → `.gitignore:15:.claude/` |
| plan-58 is claimed by another session (untracked `planning/plan-58-data/`), plan-59 is free | `ls planning \| rg '^plan-5[89]'` → `plan-58-data` only |

## 3. Design Overview

Pieces, in order: (1) a detached worktree with a renamed, ad-hoc signed
build; (2) `SchematicDocumentModel`, a `ReferenceFileDocument` that owns what
`ContentView.init` builds today (`SchematicDocument`, `SimulationController`)
and serializes it through the unchanged `SchematicFile`; (3) a `DocumentGroup`
scene in place of `WindowGroup(id: "schematic")`; (4) probes.

Where the risk concentrates:

- The file URL. `init(configuration:)` and `fileWrapper(snapshot:configuration:)`
  receive no URL, but `SchematicFile` needs one for relative load-file paths
  (F1, F2). The spike's first mechanism is the simplest one: the editor view
  hands the model `configuration.fileURL` when it appears and when it changes;
  the probes say what that costs.
- Undo. SwiftUI marks a reference document edited through the undo manager in
  the environment (`EnvironmentValues.undoManager`, interface line 17514);
  the app has per-page lists of its own, which the owner's decision replaces
  with one `NSUndoManager` per document. U3 builds that and tests it
  (`probes.md`).

Rejected alternatives: plain `FileDocument` (a value type; `SchematicDocument`
is a reference type with a simulation attached); probing in the main working
tree (it holds uncommitted changes by the owner, and a second app with the
same bundle id would collide with the installed one); keeping
`SchematicFileController` and only registering document types (it does not use
the built-in tooling, which the owner asked for).

Byte-identity is not this plan's gate. F3 compares decoded values
(`SchematicFile` is `Equatable`), not bytes.

## 4. Detailed Design

### `SchematicDocumentModel`

New file in the worktree, `schematical/schematical/Document/SchematicDocumentModel.swift`:

- `final class SchematicDocumentModel: ReferenceFileDocument`, `@MainActor`.
- `typealias Snapshot = SchematicFile`; `static var readableContentTypes =
  [SchematicFile.contentType]` (`SchematicFile.swift:52`).
- `let document: SchematicDocument`, `let simulation: SimulationController`,
  built as `ContentView.init` builds them (`ContentView.swift:99–104`:
  `PartCatalogStore.shared.library`, `addStandardNetClasses()`).
- `var fileURL: URL?` — set from the view; the value `SchematicFile` gets.
- `func load(_ data: Data, fileURL: URL?) throws` — decode and
  `apply(to:catalog:fileURL:)`; `init(configuration:)` calls it with
  `configuration.file.regularFileContents`.
- `func snapshot(contentType:)` → `SchematicFile(document:fileURL:)`;
  `fileWrapper(snapshot:configuration:)` →
  `FileWrapper(regularFileWithContents: try snapshot.encoded())`.
- `func encodedSnapshot() throws -> Data` — the same two steps, callable from
  the harness (`FileDocumentReadConfiguration` has no public initializer).

### The scene

In the worktree's `schematicalApp.swift`, `WindowGroup(id: "schematic") {
ContentView() }` becomes `DocumentGroup(newDocument: { SchematicDocumentModel() })
{ configuration in ContentView(model: configuration.document, fileURL:
configuration.fileURL) }`, keeping `.windowToolbarStyle` and `.commands`.
`ContentView.init` takes the model and reads `document` and `simulation` from
it. The controller stays (exports and print use it); its save, open and close
members are simply unused by G.

## Compatibility / Format Impact

None on `main`. In the spike, `Info.plist` gains `CFBundleDocumentTypes` for
`jgz.schematical.schematic` (role Editor) and `jgz.schematical.part` (role
Viewer); the file format is unchanged.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes. Use `- [~]` for partial and say what
> remains. Mark a moot task `- [x] ~~text~~ — moot: <evidence>`. Fill the
> phase's `Commit:` line the moment it lands. An unticked box means NOT DONE.

### Phase 1 — Worktree, baselines, signed spike build

Delivers the place everything else runs and the numbers everything is judged against.

- [ ] Create the worktree from a script file in the scratchpad: `git worktree add --detach /Users/justinzaun/Development/logic/.claude/worktrees/spike-59-G 2d10a029`.
- [ ] Baselines B1, before any rename: from the worktree, `scripts/harness/build.sh scripts/harness/edit-lag.swift <scratch>/h-edit`, then run `<scratch>/h-edit/H.app/Contents/MacOS/harness examples/6502.schematical both` three times; the same with `drag-timing.swift`. Record median and p90 (median of the three runs) in `findings.md` § Baselines.
- [ ] Add `planning/plan-59-data/scripts/baselines.swift` (a harness main): prints B3 (median of 20 timings of `SchematicFile(document:fileURL:)` on `examples/6502.schematical`) and B2 (encode a design with an EEPROM whose load file is `…/A/rom.bin`, with `fileURL` `…/B/design.schematical`, and print the chip's `memoryFile.path`). Run it through `build.sh`; record B2 and B3.
- [ ] In the worktree's `schematical.xcodeproj/project.pbxproj`, app target, Debug and Release: `PRODUCT_NAME = SchematicalSpike59`, `PRODUCT_MODULE_NAME = schematical`, `PRODUCT_BUNDLE_IDENTIFIER = jgz.schematical.spike59`, `CODE_SIGN_STYLE = Manual`, `CODE_SIGN_IDENTITY = "-"`, `DEVELOPMENT_TEAM = ""`.
- [ ] Build signed: `xcodebuild -project <wt>/schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath <scratch>/dd-G CODE_SIGNING_ALLOWED=YES build`.
- [ ] Launch it once and record that the sandbox is on.

Acceptance: B1–B3 are in `findings.md`; the renamed app builds, runs, and has its own sandbox container.
  Check: `rg '^\| B[123].*\| — \|' planning/plan-59-data/findings.md | wc -l` → `0`; `codesign -d --entitlements - <scratch>/dd-G/Build/Products/Debug/SchematicalSpike59.app 2>&1 | rg app-sandbox` → one line; `ls ~/Library/Containers | rg spike59` → `jgz.schematical.spike59` (est. first build 10–15 min: the whole app plus the C backend, nothing smaller builds the app; baselines 5 min).
Commit: —

### Phase 2 — The `DocumentGroup` document

Delivers G as a running app that opens and saves files; no probes yet.

- [ ] Worktree `schematical/Info.plist`: add `CFBundleDocumentTypes` for the two types.
- [ ] Add `Document/SchematicDocumentModel.swift` as designed in § 4.
- [ ] Worktree `ContentView.swift`: `init(model:fileURL:)`; set `model.fileURL` from the view on appear and on change of `fileURL`.
- [ ] Worktree `schematicalApp.swift`: the `DocumentGroup` scene as in § 4.
- [ ] Build; fix compile errors only; record in `findings.md` § Notes per candidate how many lines the spike changed (`git -C <wt> diff --stat 2d10a029 | tail -1`).

Acceptance: the app starts with an Untitled document, and a file passed with `open -a` opens in a window named after it.
  Check: `open -a <spike.app> examples/6502.schematical; osascript -e 'tell application "System Events" to tell process "SchematicalSpike59" to get name of every window'` → output contains `6502` (est. 3 min incl. incremental build).
Commit: —

### Phase 3 — File probes F1–F7 on G

- [ ] Add `planning/plan-59-data/scripts/make-fixture.swift` (harness main) that writes `fixtures/eeprom-rel/design.schematical` (one EEPROM whose `memoryFile.path` is `rom.bin`) and a `rom.bin` beside it.
- [ ] F1: open the fixture in the spike app; read the chip's load file (Memory window shows its bytes). Record the verdict.
- [ ] F2: Save As into a second folder, Duplicate, and wait for autosave; compare each written `memoryFile.path` with B2. Record.
- [ ] F3: add `planning/plan-59-data/scripts/roundtrip.swift` (harness main) that, for each of the 9 examples, runs `SchematicDocumentModel.load` then `encodedSnapshot()` and compares `SchematicFile.decode` of both with `==`. Record.
- [ ] F4: time the snapshot step 20 times on 6502 in the app with autosave on, and type 30 characters in a net label. Record.
- [ ] F5: quit; `open -a` 6502 cold; compare the window's part count with the file's `parts` length. Record.
- [ ] F6: save the fixture, relaunch, open it, edit it from the shell, press Save. Record.
- [ ] F7: double-click-equivalent (`open -a`) a `.schpart`; record.
- [ ] Each verdict row: G cell, evidence cell (command and output, or screenshot under `evidence/`).

Acceptance: seven verdict rows for G.
  Check: `rg -c '^\| F[1-7] \| [^|]+\| (works|workaround|blocked) \|' planning/plan-59-data/findings.md` → `7` (est. 2–3 h of driving; the probes are the work).
Commit: —

### Phase 4 — Undo and dirty probes U1–U5 on G

- [ ] Add a hook in the worktree's `SchematicDocument` (`Canvas/SchematicModel.swift`, where `recordEdit` pushes a step, `:2021–2090`): `var onStep: ((UndoStep, EditorPage) -> Void)?`.
- [ ] U3: in `ContentView`, take `@Environment(\.undoManager)` and hand it to the model; on each step (`onStep`) register one undo on that manager whose handler applies the step's diff (the apply code `undo(on:)` uses, `SchematicModel.swift:2096–2125`) and registers the redo, with `setActionName(step.name)`. Remove the `UndoCommands` replacement from the worktree's `.commands` list so Edit › Undo and Redo are the system's. Run U3, then U1, U2, U4, U5 on it.
- [ ] Record the verdicts and the API names used.

Acceptance: five verdict rows (U1–U5) for G.
  Check: `rg -c '^\| U[1-5] \| [^|]+\| (works|workaround|blocked) \|' planning/plan-59-data/findings.md` → `5` (est. 3 h; the undo bridge is the work).
Commit: —

### Phase 5 — Save the evidence

- [ ] `git -C <wt> diff 2d10a029 > planning/plan-59-data/spike-G-core.patch` (the worktree stays for plan-59-B).
- [ ] Fill this plan's `Commit:` lines and tick its boxes.

Acceptance: the patch applies to a clean checkout of the base commit.
  Check: `git apply --check planning/plan-59-data/spike-G-core.patch` run in a second throwaway `git worktree add --detach <scratch>/chk 2d10a029` → exit 0, then `git worktree remove <scratch>/chk` (est. 2 min).
Commit: —

## Validation Plan

- Tests: none. This plan changes no code on `main`; its output is evidence.
- Coverage check: every probe in scope (F1–F7, U1–U5) has a row; the two `rg -c` checks in phases 3 and 4 are the denominator.
- Runtime proof: each verdict's evidence cell.
- Doc sync: none. `main`'s docs describe the app as it is.
- Final gate (run once, after the last phase): `git show --name-only --format= <each commit of this plan> | rg -v '^planning/plan-59' | wc -l` → `0` (est. 1 min).

## Decisions

- This is a spike whose deliverable is a migration plan — decided by the owner, 2026-11-24.
- Use the built-in tooling properly, even when it is more work; macOS first — decided by the owner, 2026-11-24. It settles the workaround rule in `probes.md`: no delegate proxying, swizzling, private API or notification patching.
- G is `DocumentGroup` + `ReferenceFileDocument`, as the owner asked; H (`NSDocument` hosting) is built as the comparison — decided by the author, 2026-11-24.
- The spike lives in detached worktrees and never merges; no branch — decided by AGENTS.md § Git.
- Undo is per document, on one `NSUndoManager`; the per-page undo lists go away — decided by the owner, 2026-11-24 ("I would prefer a per document undo with NSUndoManager if possible"). It settles U3 and removes the page-split routes from the probe list. The tests that assert page independence (`PCBUndoTests`) change in the migration plan.
- Ad-hoc signing and a separate bundle id — decided by the author, 2026-11-24, after the sandbox probe above.

## Corrections

Filled in during execution.

## Summary

The risk is in two places: the file URL (the document API hands it to neither
the reader nor the writer) and undo (the app's per-page lists become one
`NSUndoManager` per document; U3 tests the bridge from `UndoStep` to it). Nothing on `main` changes.
