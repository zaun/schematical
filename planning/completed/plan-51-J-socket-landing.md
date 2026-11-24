# plan-51-J: IC sockets — landing tier

Last updated: 2026-11-21
Effort: small (< 1h)
Depends on: plan-51-I

J runs the landing tier once for plan-51, archives the sub-plans and ticks the todo.
It changes no code.

Outcome of J: every gate below passes; `planning/todo.md`'s socket item is `[x]`;
plan-51-A … J are in `planning/completed/`.

References:

- `AGENTS.md` (§ Git: commit only your files, itemized; no branch changes),
  `docs/backend.md` § Tests, plan-51-A … I.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-51-I complete | `ls planning/completed/plan-51-I-*.md` → exists | MET (2026-11-24: `planning/completed/plan-51-I-socket-mode-ui.md`, commit 39961690) |
| Every earlier letter's boxes are ticked | `grep -c '^- \[ \]' planning/plan-51-[A-I]-*.md` → every count 0 | MET (2026-11-24: A to I have 0 `- [ ]`; I's real-window pass was skipped by the owner: plan-51-I § Corrections) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> If you stop, report the status of *all* prerequisites.

## 1. Goal

All of § 4 passes on one tree.

### Non-goals (explicit constraints)

- No new code; a failure is fixed in the letter that owns it (its box reopened, its
  Corrections filled), then this phase restarts.
- The full tier (every test class) is not a plan step; it runs in the background.

## 4. Phases

### Phase 1 — the gates

- [x] Backend: `cmake -S backend -B /tmp/logic-p51 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p51 -j8 && ctest --test-dir /tmp/logic-p51 -j8` → `100% tests passed, 0 tests failed out of 61`. — measured 2026-11-24 (build dir `/tmp/logic-p51-wt`): `100% tests passed, 0 tests failed out of 61`.
- [x] Generated data: `node scripts/gen-sockets.js --check` → exit 0; `node --test scripts/gen-sockets.test.js` → no `# fail`. — `gen-sockets --check: socket.json (575 parts), footprints.json and datasheets.json are up to date`; `ℹ pass 23`, `ℹ fail 0`; `series_check.py` → `ok`.
- [x] Datasheet URLs of the sockets: `node scripts/check-datasheets.js --host www.mill-max.com`, `--host precidip.com`, `--host www.assmann-wsw.com` → each exits 0 (est. 3 min; Mill-Max's PDF is 20 MB). — `1 of 1 URLs serve a PDF` (Mill-Max), `2 of 2` (Preci-Dip), `5 of 5` (Assmann), 0 failed.
- [x] App tests: `xcodebuild … test -only-testing:schematicalTests/SocketPartsTests -only-testing:schematicalTests/SocketEditorTests -only-testing:schematicalTests/SocketModeTests -only-testing:schematicalTests/SocketPickerTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/KindInfoTests -only-testing:schematicalTests/KindGuideTests -only-testing:schematicalTests/BillOfMaterialsTests -only-testing:schematicalTests/SchematicFileV5Tests -only-testing:schematicalTests/PCBUndoTests -only-testing:schematicalTests/AutomationTests -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/EasyEDAProExportTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/ReferencePrefixTests` → `** TEST SUCCEEDED **` (est. 25 min). — run with 9 more classes (§ Corrections): `Executed 297 tests, with 0 failures`, `** TEST SUCCEEDED **`.
- [x] Harness: the plan-51-I Phase 2 check → every line `ok`. — rebuilt on the final tree: `all ok` (10 of 10), exit 0; the sheet and sidebar images were read.
- [x] Docs: `grep -c 'socket' docs/parts-format.md docs/file-format.md docs/agent-control.md` → every count at least 1. — `docs/agent-control.md:3`, `docs/file-format.md:5`, `docs/parts-format.md:42`.

Acceptance: every item above passes.
  Check: the items themselves (est. 40 min in total).
Commit: 9b8cdecf

### Phase 2 — close out

- [x] `planning/todo.md`: the "For DIP and PLCC chips have a toggle to use a socket footprint…" line becomes `[x]` with `(plan-51)`. — made in the main tree after the merge, left uncommitted (§ Corrections: the file holds the owner's own uncommitted edits); the line reads `- [x] (plan-51) For DIP and PLCC chips …`.
- [x] `git mv planning/plan-51-[A-J]-*.md planning/completed/` (`plan-51-data/` stays). — A to I were archived as each finished; J with this commit.
- [x] One commit per AGENTS § Git, naming only the files changed. — one commit per letter's work on the branch (named files only), the branch merged into main without a conflict.

Acceptance: `ls planning/completed/plan-51-*.md | wc -l` → 10.
  Check: that command (est. 1 s).
Commit: —

## Validation Plan

- This plan is the validation. Final gate: Phase 1.

## Decisions

See plan-51-A § Decisions.

## Corrections

- **2026-11-24, the app-test gate ran more classes than the plan lists.** The plan's 16 classes plus `ParametricFootprintDraftTests`, `TestPointTests`, `WizardStepsTests`, `PartEditorWindowTests`, `PullResistorTests`, `HeaderBoardTests`, `PickAndPlaceExportTests`, `PartRegistrationTests` and `WizardIssueLocatorTests`: the classes whose counts or lists plan-51 moved (the footprint, kind and drawer lists) or whose code it touched. 297 tests, 0 failures. The whole suite was not run: in a Debug build it stalls for many minutes in the router's AFU test (plan-51-G § Corrections), and the full tier runs in the background by the project's policy.
- **2026-11-24, no code changed in J.** The failures found on the way were fixed in the letter that owns them (G: the footprint counts; I: the drawer list); a cosmetic change to the sheet's column widths after the first harness run was re-verified (the sheet tests and the harness, again `all ok`).
- **2026-11-24, `planning/todo.md` is ticked in the main tree, not on the branch.** The main tree already holds the owner's uncommitted edits to that file (and to the examples and the Xcode project), so a commit on the branch would collide with them at the merge, and a commit in main would carry their edits. The one-line tick is made in the working tree after the merge and left uncommitted.

## Summary

Nothing new is built; the risk is a gate that fails on a letter's work, fixed in that letter.
