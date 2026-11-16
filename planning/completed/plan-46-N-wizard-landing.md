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
| plan-46-A … M complete | `ls planning/completed/plan-46-[A-M]-*.md \| wc -l` → 13 | MET (2026-11-16 at 88409176: 13) |

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

- [x] `KeyboardHelp.swift` per § 1; `testHelpListsNewPart` also checks
      "Part Wizard" and "Guided". (And "All fields" and "Paste a pin
      table". `xcodebuild … test
      -only-testing:schematicalTests/PartEditorWindowTests` → `Executed 9
      tests, with 0 failures`.)
- [x] `docs/backend.md` § Adding a kind step 6 and the new-key paragraph.
      (`grep -c 'KindInfo' docs/backend.md` → 1. `docs/parts-format.md`
      § Single part files and § Custom footprints in a part file re-read
      against the code: the inline `"datasheets"` rules and the editor's
      `CUSTOM-<first 8 of the uuid>` ids are as A and J wrote them; nothing
      to change.)

Acceptance: the help names the wizard and the doc names every editor step.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p46-dd test -only-testing:schematicalTests/PartEditorWindowTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **`; `grep -c 'KindInfo' docs/backend.md` → at least 1 (est. 7 min).
Commit: 96bec7ef

### Phase 2 — Proofs, gate, archive

- [x] Extendability probe (throwaway, outside the repo or in a detached
      worktree; never committed): add `Field(key: "probe", label: "Probe",
      type: .bool)` to `DffDraft.schema.device` → `KindGuideTests` fails
      with a message naming `dff` and `device.probe`. Record the message
      in § Corrections. (§ Corrections 1.)
- [x] End-to-end walk (run skill) of the behavioral outcome above. Pointer
      to (0, 0).
      (2026-11-16, this plan's Debug build. File › New Part… → `What are
      you making?` (step 1 of 6) → Logic Gate → `MY-74HC00`, Texas
      Instruments, SOIC → Datasheet: `https://www.ti.com/lit/ds/symlink/sn74hc00.pdf`
      → Load → `Showing www.ti.com` → Pins: the TI pin table pasted, `14
      pins, 1 row skipped`, Apply → Supply voltage → Gates: four NAND gates
      `1A 1B → 1Y` … `4A 4B → 4Y` → Logic family and timing: `HC`, 9 ns, 2–6
      V → Footprint: `SOIC-14` from the thumbnails → Review: `The part
      loads: 14 pins`, `SOIC-14: 14 pads`, 14 pad rows, nothing still to
      do → Save Part… → `/tmp/p46-walk/MY-74HC00.schpart` (gates `nand:1A+1B>1Y`
      …, `"datasheets": [{"id": "CUSTOM-0B022F1F", "url": "https://www.ti.com/lit/ds/symlink/sn74hc00.pdf"}]`,
      `"datasheet": "CUSTOM-0B022F1F"`, footprint `SOIC-14`, family `HC`).
      Editor closed; File › Load Part… → the drawer listed MY-74HC00 under
      Custom › Logic gates and flip-flops › Logic Gate; dragged onto the
      schematic and selected → Open Datasheet → window `MY-74HC00 Datasheet
      – www.ti.com`. Pointer left at (0, 0). In the first pass the driving
      script left gates 2–4 as AND (its own substring match on "and");
      they were set to NAND in the app and the part saved again before the
      load.)
- [x] Final gate (below), once. (Backend: `100% tests passed, 0 tests
      failed out of 37`; `git diff --stat 5bccd0d7 -- backend/tests/golden`
      → empty. App, the 19 classes: `Executed 137 tests, with 7 failures`,
      all seven in `PartKindRoundTripTests` and all stale part counts
      (§ Corrections 2); with the counts corrected, `PartKindRoundTripTests`
      → `Executed 30 tests, with 0 failures`, `** TEST SUCCEEDED **`; the
      other 18 classes had 107 tests, 0 failures. `main` had not moved
      (`git rev-parse --short main` → 4218bb74, the worktree's base), so no
      merge and no second run were needed. The repo has no Rust workspace
      (no `Cargo.toml`), so there is nothing for `cargo fmt`.)
- [x] Move `planning/plan-46-*.md` to `planning/completed/` (each lettered
      file as it completes — A … M are already there; N last). (This
      commit moves N; `ls planning/completed/plan-46-*.md | wc -l` → 14.)

Acceptance: the final gate passes and the walk shows the outcome.
  Check: the final gate's two commands (est. 25 min, below).
Commit: 0e670d6b

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

1. **The extendability probe.** With `Field(key: "probe", label: "Probe",
   type: .bool)` added to `DffDraft.schema.device` (in the plan's worktree,
   the file copied aside first and copied back after; `git status` then
   showed it unchanged, and it was never staged), `KindGuideTests` failed:
   `testCuratedGuidesCoverEveryKeyOnce : XCTAssertEqual failed: ("["dff: no
   step of the guide has device.probe"]") is not equal to ("[]") - dff: no
   step of the guide has device.probe`, and
   `testEveryTopLevelKeyOfACuratedKindHasAGuide` with `dff: device.probe`.
   The other ten `KindGuideTests` cases passed. The probe ran in the
   worktree rather than a detached copy: a copy would have needed its own
   full app build, and the file restore is checked by `git status`.
2. **A prerequisite no row covered: `PartKindRoundTripTests` held seven
   stale part counts at the base.** The final gate's first app run failed
   seven of its cases on the count alone, e.g. `testDiode : XCTAssertEqual
   failed: ("Optional(11166)") is not equal to ("Optional(11152)") - diode`
   beside `checked 11166 parts of diode, 0 mismatched, 0 with extras, 0
   rejected`. Earlier plans added parts without moving the test's
   `expected` table: buffer 2135 → 2137, connector 374 → 376, converter
   1911 → 1914, diode 11152 → 11166, oscillator 1138 → 1145, rtc 14 → 16,
   switch 2067 → 2070. plan-46 changes no part file (`git diff --stat
   4218bb74 -- schematical/schematical/Parts` → only the new, empty
   `generic.json`), and every part of all 30 kinds round-trips 0 / 0 / 0.
   The table now holds the measured counts. (The same kind of stale count
   as plan-46-J § Corrections 1; plan-46-C's four new kinds were right.)
3. **§ Validation Plan says the round trip reads "31 kinds (all but
   `generic`)"; it is 30 test cases.** `gate` has no case in
   `PartKindRoundTripTests` (its hand-written schema is covered by
   `PartDraftRoundTripTests`, `checked 3438 parts, 0 mismatched, 0 with
   extras, 0 rejected`, which is in the gate too).

## Summary

Docs, help and one gate. The one claim N proves that no unit test can on
its own is extendability: a schema change surfaces as a named guide
failure, and an uncurated kind needs nothing at all.
