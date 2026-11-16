# plan-46-N: Part Wizard — landing: help, docs, the end-to-end proof, the final gate

Last updated: 2026-11-16
Effort: medium (1h–2h)
Depends on: plan-46-M

N finishes plan-46: the in-app help describes the wizard, the docs say how
to extend it (what a new kind or a new key needs in the editor), the whole
plan's behavioral outcome is shown once in the real app, the final gate
runs once, and the sub-plans are archived.

Behavioral outcome: plan-46-A's outcome, end to end — a 74HC00-style
gate made in the wizard from kind to Review, saved, loaded with File ›
Load Part…, placed, with Open Datasheet opening the pasted URL — plus the
extendability claim, shown by a throwaway probe: a scratch copy of the
editor with one key added to the `dff` schema fails `KindGuideTests`
naming `dff` and the key.

References:

- `schematical/schematical/Help/KeyboardHelp.swift:103–107` (the New
  Part…, Save Part…, Open Part…, Copy to Editor…, Load Part… entries),
  `schematicalTests/PartEditorWindowTests.swift` `testHelpListsNewPart`.
- `docs/backend.md` § Adding a kind (:370–399), `docs/parts-format.md`
  § Single part files, § Custom footprints in a part file.
- Every plan-46 sub-plan's Validation Plan.

## Prerequisites

plan-46-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-46-A … M complete | `ls planning/completed/plan-46-[A-M]-*.md \| wc -l` → 13 | NOT MET (2026-11-16: 0) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- Help: "⇧⌘N — New Part…: the Part Wizard walks you through a new part
  (kind, part number, datasheet, pins, the kind's settings, footprint,
  review)"; a new entry "Guided | All fields (Part Editor toolbar) —
  switch between the wizard and the full form"; "Paste a pin table (Part
  Wizard › Pins) — copy the datasheet's pin table and paste it".
- `docs/backend.md` § Adding a kind gains step 6, **the Part Editor**:
  a `KindInfo` entry (`Editor/KindInfo.swift`); `"<kind>": [...]` in
  `scripts/part-editor-schema.py` `KIND_FILES` and
  `PartKindKeysTests.files`; `python3 scripts/part-editor-schema.py .
  <kind>` (or a hand-written schema for a kind with no shipped parts, the
  `GenericDraft` precedent); the schema in `PartKinds.schemas`;
  `test<Kind>` and its count in `PartKindRoundTripTests`; optionally a
  curated guide and, for a kind with variants, its blurbs. And a short
  "a new key on an existing kind" paragraph: regenerate the schema; a
  curated kind's `KindGuideTests` failure names the key and the guide to
  add it to.
- `docs/parts-format.md`: the § Single part files and § Custom footprints
  text already updated by A and J is re-read once against the final code.

### Non-goals (explicit constraints)

- **No new features.** Anything found missing goes to a new plan or bug.

## Phases

> **NOTE — keep the checkboxes current as you go.** **An unticked box means NOT DONE.**

### Phase 1 — Help and docs

- [ ] `KeyboardHelp.swift` per § 1; `testHelpListsNewPart` also checks
      "Part Wizard" and "Guided".
- [ ] `docs/backend.md` § Adding a kind step 6 and the new-key paragraph.

Acceptance: the help names the wizard and the doc names every editor step.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p46-dd test -only-testing:schematicalTests/PartEditorWindowTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **`; `grep -c 'KindInfo' docs/backend.md` → at least 1 (est. 7 min).
Commit: —

### Phase 2 — Proofs, gate, archive

- [ ] Extendability probe (throwaway, outside the repo or in a detached
      worktree; never committed): add `Field(key: "probe", label: "Probe",
      type: .bool)` to `DffDraft.schema.device` → `KindGuideTests` fails
      with a message naming `dff` and `device.probe`. Record the message
      in § Corrections.
- [ ] End-to-end walk (run skill) of the behavioral outcome above. Pointer
      to (0, 0).
- [ ] Final gate (below), once.
- [ ] Move `planning/plan-46-*.md` to `planning/completed/` (each lettered
      file as it completes — A … M are already there; N last).

Acceptance: the final gate passes and the walk shows the outcome.
  Check: the final gate's two commands (est. 25 min, below).
Commit: —

## Validation Plan

- Final gate (run once, here):
  - Backend: `cmake -S backend -B /tmp/logic-p46 && cmake --build /tmp/logic-p46 -j8 && ctest --test-dir /tmp/logic-p46 -j8` → `100% tests passed, 0 tests failed out of 37` (A's and B's areas; the suite is 9 s at planning, so the whole of it is the targeted gate); `git diff --stat 5bccd0d7 -- backend/tests/golden` → empty (no plan-46 sub-plan changes a part).
  - App, every area plan-46 touched: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p46-dd test -only-testing:schematicalTests/PartFileSaveOpenTests -only-testing:schematicalTests/GenericPartTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartDraftRoundTripTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/PartEditorFootprintTests -only-testing:schematicalTests/CustomFootprintDraftTests -only-testing:schematicalTests/KindInfoTests -only-testing:schematicalTests/WizardStepsTests -only-testing:schematicalTests/PinRenameTests -only-testing:schematicalTests/WizardModelTests -only-testing:schematicalTests/WizardIdentityTests -only-testing:schematicalTests/DatasheetFinderTests -only-testing:schematicalTests/PinTableParserTests -only-testing:schematicalTests/WizardKindStepsTests -only-testing:schematicalTests/ParametricFootprintDraftTests -only-testing:schematicalTests/WizardIssueLocatorTests -only-testing:schematicalTests/KindGuideTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 20 min: the round-trip tests read every shipped part of 31 kinds (all but `generic`), which is the only check that every schema still matches its parts).
  - The full app suite runs in the background per the project's policy; it is not a plan step.
- Runtime proof: Phase 2's walk.
- Doc sync: Phase 1.

## Decisions

plan-46-A § Decisions.

## Corrections

None yet.

## Summary

Docs, help and one gate. The one claim N proves that no unit test can on
its own is extendability: a schema change surfaces as a named guide
failure, and an uncurated kind needs nothing at all.
