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
| plan-46-D complete | `ls planning/completed/plan-46-D-*.md` → exists | MET (2026-11-16: `planning/completed/plan-46-D-wizard-kind-info-and-steps.md`) |
| The step model exists | `grep -c 'enum WizardPlan' schematical/schematical/Editor/Wizard/WizardSteps.swift` → 1 | MET (2026-11-16 at b13a36f2: 1) |

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

- [x] `Editor/PartEditorSections.swift`: the six section views moved out of
      `PartEditorForm` (code moves, no edits beyond parameters);
      `PartEditorForm` stacks them.
      (Five move: `IdentitySection`, `VerdictSection`, `FootprintSection`,
      `SupplySection`, `KindFieldsSection`; `PinoutsSection` was already a
      view and stays in `PartEditorWindow.swift`. `xcodebuild … test
      -only-testing:schematicalTests/PartEditorWindowTests
      -only-testing:schematicalTests/PartEditorFootprintTests
      -only-testing:schematicalTests/PartFileSaveOpenTests …` → `Executed 9`,
      `Executed 3` and `Executed 6 tests, with 0 failures`, `** TEST SUCCEEDED **`.)

Acceptance: the form is unchanged.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p46-dd test -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/PartEditorFootprintTests -only-testing:schematicalTests/PartFileSaveOpenTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 10 min).
Commit: e67e9572

### Phase 2 — Mode, steps, issues

- [x] `PartEditorModel`: `mode`, `currentStepID`, `steps`, `goNext()`,
      `goBack()`, `go(to:)`, the disappearing-step rule.
- [x] `Editor/Wizard/WizardIssues.swift`: `WizardStep.issues(in:)`.
- [x] `schematicalTests/WizardModelTests.swift`: `.new` → guided, `.copy`
      and `.file` → allFields; Next from the first step reaches Review in
      `steps.count - 1` calls; changing a counter draft's `model` from
      `ripple` to `sync` while on a step whose keys are ripple-only
      (`CounterDraft.swift` has one `variants: ["ripple"]` field) moves to the next
      surviving step; issues: an empty gate draft reports the name, the
      empty pinout, `gates` and the footprint.
      (Ripple → sync on `kind.sections` moves to `kind.byteBus`, the step
      after `kind.essentials`; the empty gate draft's issues per
      § Corrections 1. `xcodebuild … test
      -only-testing:schematicalTests/WizardModelTests` → `Executed 8 tests,
      with 0 failures`, `** TEST SUCCEEDED **`.)

Acceptance: navigation and issues behave as § 1 says.
  Check: `xcodebuild … test -only-testing:schematicalTests/WizardModelTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: abf5d0ba

### Phase 3 — The guided window

- [x] `Editor/Wizard/WizardStepList.swift`, `WizardStepView.swift`,
      `WizardNavBar.swift`; `PartEditorWindowView` per § 3. (The mode
      picker is the toolbar's principal item; the editor's default size is
      1120 × 760 so the three panes fit; § Corrections 3.)
- [x] Runtime check with the run skill: New Part… → Gate → the window
      opens Guided; Next through every step; All fields shows the name
      typed on step 1; Copy to Editor… on SN74HC00N opens All fields.
      Move the pointer to (0, 0) when done (AGENTS.md).
      (2026-11-16, this plan's Debug build, pid-driven through the
      Accessibility API: New Part… → Logic Gate → window `New Logic Gate
      Part`, Guided selected, steps 1–8 listed. Name `MY-NAND` typed on step
      1 → the title became `MY-NAND` and the step's dot a check. Five pins
      added and named on Pins; Supply; Essentials (a NAND gate, family HC
      with rise/fall 9 and 2–6 V) → `5. Essentials` checked; Other settings
      (Optional, with Skip); Footprint `SOT23-5`; Review: `The part loads: 5
      pins` with the block and footprint previews. All fields → Name
      `MY-NAND`, Package `SOT-23`, … and the five form sections. Save Part…
      → status `Saved MY-NAND.schpart`, `/tmp/p46-walk/MY-NAND.schpart`
      holds the gate, timing and footprint (`write(to:)` saves only what
      `PartValidator` accepts). Copy to Editor… on SN74HC00N → a second
      editor window with `All fields` selected. Pointer left at (0, 0).)

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

1. **An empty gate draft has six issues, not four.** § Phase 2 lists "the
   name, the empty pinout, `gates` and the footprint". `timing` and
   `family` are required gate keys too and a new draft holds them empty
   (`{}` and `""`), so by § 1's own rule ("a required key with an empty
   value") they are reported as well: `WizardModelTests` asserts exactly
   `The part has no part number`; `The part has no pins`; `Gates is empty`,
   `Timing is empty`, `Family is empty`; `No footprint is chosen`. "Empty"
   is: missing, `""`, `[]`, `{}`, or an untouched supply range `[0, 0]`; a
   number, a yes/no and a fixed choice are never empty (0 and "no" are
   answers). Issues also reach nested keys (`Gates 2 › Output is empty`,
   `Gates 1 › Inputs: there is no pin named “B”`).
3. **Two layout fixes found by the runtime check.** The first build drew
   the guided `HSplitView` at its ideal height, pinned to the bottom of
   the window (a screenshot showed the top two thirds empty): the split
   view and its three panes now take `maxHeight: .infinity`. With that, the
   preview pane centred vertically; it is top-aligned again. Neither is
   visible to a unit test.
2. **The step ids are D's as landed** (plan-46-D § Corrections 2): a gate's
   listed steps are `identity, datasheet, pinout, kind.supply,
   kind.essentials, kind.other, footprint, review`.

## Summary

The risk is the section move breaking the form; it lands first, alone,
behind the existing tests. The shell itself is layout plus a small,
tested navigation model.
