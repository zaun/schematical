# plan-46-E: Part Wizard — the window shell, Guided | All fields, shared section views

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-46-D

E puts the wizard in the Part Editor window. A new part opens **Guided**:
a step list on the left (D's `WizardPlan.steps(for:)`), the current step
in the middle with Back / Next (and Skip on optional steps), the existing
Block | Footprint preview on the right. A segmented **Guided | All fields**
control in the toolbar switches to today's form and back with the draft
untouched. The form's sections become standalone views that both modes
use, so the two modes cannot drift apart. Each step page in E hosts the
existing section for its keys; F–K replace pages one at a time with their
guided versions.

Behavioral outcome: File › New Part… → choose Gate in today's picker → the
window opens Guided on Part number; Next walks Datasheet, Pins, Supply,
the gate's kind steps, Footprint and Review; switching to All fields shows
every value entered; Open Part… and Copy to Editor… open All fields.

References:

- `schematical/schematical/Editor/PartEditorWindow.swift` (the window, the
  form and its sections, `PartPreviewPane`), `Editor/PartEditorModel.swift`
  (window state), `Editor/PartEditorTarget.swift` (`Source`: new, copy,
  file), `Editor/PartKindPicker.swift`, `Editor/PartEditorCommands.swift`,
  `Editor/FieldEditors.swift` (`FieldsEditor`, the schema-driven controls).
- `schematical/schematicalApp.swift:146–156` (the picker `Window` and the
  editor `WindowGroup`).
- plan-46-D § 3.2 (steps), plan-46-A § Decisions (New Part only).
- `schematicalTests/PartEditorWindowTests.swift`.

## Prerequisites

plan-46-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-46-D complete | `ls planning/completed/plan-46-D-*.md` → exists | NOT MET (2026-11-16: not started) |
| The step model exists | `grep -c 'enum WizardPlan' schematical/schematical/Editor/Wizard/WizardSteps.swift` → 1 | NOT MET (2026-11-16: no file) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- `PartEditorModel.mode: EditorMode` (`.guided`, `.allFields`): `.guided`
  for `.new` targets, `.allFields` for `.copy` and `.file`; the toolbar
  picker sets it; it is window state, never written to the file.
- `PartEditorModel.currentStepID: String`; `steps` recomputed from the
  draft; when the current step disappears (the variant changed), the
  current step becomes the first step after the last one that still
  exists.
- Guided layout: `WizardStepList` (titles; a check when the step has no
  issues, a dot when it has), `WizardStepView` (the page), `WizardNavBar`
  (Back, Next, Skip on optional steps; Next on Review is absent). Next is
  never disabled: issues show, they do not trap the user.
- Section views extracted from `PartEditorForm` (behavior unchanged):
  `IdentitySection`, `VerdictSection`, `PinoutsSection` (exists),
  `FootprintSection`, `SupplySection`, `KindFieldsSection(keys:)`; the form
  is these stacked, exactly as today.
- Step pages in E: identity → `IdentitySection`; datasheet →
  `DatasheetField`; pinout → `PinoutsSection`; supply / kind steps →
  `KindFieldsSection(keys: step.keys)`; variant → the variant key's field;
  footprint → `FootprintSection`; review → `VerdictSection` + both
  previews. The `kind` step is not listed until F (the kind is chosen in
  today's picker).
- Step issues (`WizardStep.issues(in: PartDraft) -> [String]`): a required
  key with an empty value; a `.pin`/`.pins` value naming no pin; an empty
  pinout (pinout step); a missing name (identity); no footprint
  (footprint). Pure, tested.

### Non-goals (explicit constraints)

- **All fields is today's form.** Same sections, same order, same
  controls; `PartEditorWindowTests`' existing cases pass unchanged.
- **No new step content** (F–K).
- **No file change**: mode and step are window state.

## 2. Current State

- `PartEditorWindowView` = `HSplitView { PartEditorForm; PartPreviewPane }`
  with Open/Save/Add toolbar buttons (`PartEditorWindow.swift:16–36`).
- `PartEditorForm` (`:70–241`) builds the sections inline: identity
  (`:112–142`), verdict (`:144–161`), `PinoutsSection`, footprint
  (`:163–212`), Supply and the kind section (`:82–98`).
- `PartEditorTarget.Source` is `.new(kind:)`, `.copy(name:)`,
  `.file(URL)` (`PartEditorTarget.swift`); `PartEditorModel.init` builds the
  draft per source (`PartEditorModel.swift:73–96`).
- File › New Part… opens the `PartKindPicker` window
  (`PartEditorCommands.swift`), which opens the editor with `.new(kind:)`.

### Measured populations

| What | Count | Command |
|---|---|---|
| `PartEditorWindow.swift` lines | 454 | `wc -l schematical/schematical/Editor/PartEditorWindow.swift` |
| Editor test files touching the window model | 2 | `grep -l 'PartEditorForm\|PartEditorWindowView\|PartEditorModel' schematical/schematicalTests/*.swift` → PartEditorWindowTests, PartFileSaveOpenTests (at 9e13cbe4); PartEditorFootprintTests covers the footprint section's data |

## 3. Design

- `EditorMode` and `currentStepID` on `PartEditorModel`; `steps:
  [WizardStep]` computed (`WizardPlan.steps(for: draft)` filtered to drop
  `.kind` until F).
- `PartEditorWindowView`: toolbar `Picker("Mode", selection: $model.mode)`
  segmented "Guided" / "All fields" before Open/Save/Add; body:
  `.guided` → `HSplitView { WizardStepList (180–220 pt); WizardStepView +
  WizardNavBar; PartPreviewPane }`, `.allFields` → today's split.
- `WizardStepView` switches on `step.page` and shows the step's `title`,
  its `intro` (nil in derived steps until L), an "Optional" badge, then
  the page.
- `KindFieldsSection(keys:)` resolves side-qualified keys (`device.timing`)
  to the schema's fields for the current variant and shows them with
  `FieldsEditor` on `kindDevice` / `kindPart`.

Rejected alternatives:

- **A separate wizard window** — two windows editing one draft; the toggle
  in one window keeps one model.
- **Disabling Next on issues** — a DIY user may not know a value yet; they
  must be able to move on and come back (Review lists what is left).

## Phases

> **NOTE — keep the checkboxes current as you go.** **An unticked box means NOT DONE.**

### Phase 1 — Section views (no visible change)

- [ ] `Editor/PartEditorSections.swift`: the six section views moved out of
      `PartEditorForm` (code moves, no edits beyond parameters);
      `PartEditorForm` stacks them.

Acceptance: the form is unchanged.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p46-dd test -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/PartEditorFootprintTests -only-testing:schematicalTests/PartFileSaveOpenTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 10 min).
Commit: —

### Phase 2 — Mode, steps, issues

- [ ] `PartEditorModel`: `mode`, `currentStepID`, `steps`, `goNext()`,
      `goBack()`, `go(to:)`, the disappearing-step rule.
- [ ] `Editor/Wizard/WizardIssues.swift`: `WizardStep.issues(in:)`.
- [ ] `schematicalTests/WizardModelTests.swift`: `.new` → guided, `.copy`
      and `.file` → allFields; Next from the first step reaches Review in
      `steps.count - 1` calls; changing a counter draft's `model` from
      `ripple` to `sync` while on a step whose keys are ripple-only
      (`CounterDraft.swift` has one `variants: ["ripple"]` field) moves to the next
      surviving step; issues: an empty gate draft reports the name, the
      empty pinout, `gates` and the footprint.

Acceptance: navigation and issues behave as § 1 says.
  Check: `xcodebuild … test -only-testing:schematicalTests/WizardModelTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

### Phase 3 — The guided window

- [ ] `Editor/Wizard/WizardStepList.swift`, `WizardStepView.swift`,
      `WizardNavBar.swift`; `PartEditorWindowView` per § 3.
- [ ] Runtime check with the run skill: New Part… → Gate → the window
      opens Guided; Next through every step; All fields shows the name
      typed on step 1; Copy to Editor… on SN74HC00N opens All fields.
      Move the pointer to (0, 0) when done (AGENTS.md).

Acceptance: a gate part can be made start to finish in Guided mode and saved.
  Check: the run-skill walk above ends with Save Part… writing a `.schpart` that `PartValidator.shared.validate(text:)` accepts (est. 15 min: a GUI walk; no unit test sees the layout).
Commit: —

## Validation Plan

- Tests: `WizardModelTests`; the three existing editor test files.
- Coverage: Phase 1's move is covered by the existing form tests.
- Runtime proof: Phase 3's walk.
- Doc sync: none (N updates help).
- Final gate: plan-46-N.

## Decisions

plan-46-A § Decisions (New Part only; Guided | All fields everywhere).

## Corrections

None yet.

## Summary

The risk is the section move breaking the form; it lands first, alone,
behind the existing tests. The shell itself is layout plus a small,
tested navigation model.
