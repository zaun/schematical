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
| plan-51-I complete | `ls planning/completed/plan-51-I-*.md` → exists | NOT MET (2026-11-21: not started) |
| Every earlier letter's boxes are ticked | `grep -c '^- \[ \]' planning/plan-51-[A-I]-*.md` → every count 0 | NOT MET |

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

- [ ] Backend: `cmake -S backend -B /tmp/logic-p51 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p51 -j8 && ctest --test-dir /tmp/logic-p51 -j8` → `100% tests passed, 0 tests failed out of 61`.
- [ ] Generated data: `node scripts/gen-sockets.js --check` → exit 0; `node --test scripts/gen-sockets.test.js` → no `# fail`.
- [ ] Datasheet URLs of the sockets: `node scripts/check-datasheets.js --host www.mill-max.com`, `--host precidip.com`, `--host www.assmann-wsw.com` → each exits 0 (est. 3 min; Mill-Max's PDF is 20 MB).
- [ ] App tests: `xcodebuild … test -only-testing:schematicalTests/SocketPartsTests -only-testing:schematicalTests/SocketEditorTests -only-testing:schematicalTests/SocketModeTests -only-testing:schematicalTests/SocketPickerTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/KindInfoTests -only-testing:schematicalTests/KindGuideTests -only-testing:schematicalTests/BillOfMaterialsTests -only-testing:schematicalTests/SchematicFileV5Tests -only-testing:schematicalTests/PCBUndoTests -only-testing:schematicalTests/AutomationTests -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/EasyEDAProExportTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/ReferencePrefixTests` → `** TEST SUCCEEDED **` (est. 25 min).
- [ ] Harness: the plan-51-I Phase 2 check → every line `ok`.
- [ ] Docs: `grep -c 'socket' docs/parts-format.md docs/file-format.md docs/agent-control.md` → every count at least 1.

Acceptance: every item above passes.
  Check: the items themselves (est. 40 min in total).
Commit: —

### Phase 2 — close out

- [ ] `planning/todo.md`: the "For DIP and PLCC chips have a toggle to use a socket footprint…" line becomes `[x]` with `(plan-51)`.
- [ ] `git mv planning/plan-51-[A-J]-*.md planning/completed/` (`plan-51-data/` stays).
- [ ] One commit per AGENTS § Git, naming only the files changed.

Acceptance: `ls planning/completed/plan-51-*.md | wc -l` → 10.
  Check: that command (est. 1 s).
Commit: —

## Validation Plan

- This plan is the validation. Final gate: Phase 1.

## Decisions

See plan-51-A § Decisions.

## Corrections

## Summary

Nothing new is built; the risk is a gate that fails on a letter's work, fixed in that letter.
