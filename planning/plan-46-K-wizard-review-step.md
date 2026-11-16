# plan-46-K: Part Wizard — step 7 (review): previews, pin ↔ pad table, the verdict pointed at a step

Last updated: 2026-11-16
Effort: medium (1h–2h)
Depends on: plan-46-J

The last step is where the user checks the part before saving it. K shows
the block and the footprint side by side, a table of every pin with its
name, role and the pad it lands on, everything still missing from earlier
steps (E's step issues) with **Go to …** buttons, and the backend's
verdict. When the backend rejects the part, the wizard says **which step**
has the problem and offers to go there, instead of leaving a user to
decode `devices[0] (custom): parts[0] (MY-NAND): …`. Save Part… and Add to
Library sit on this page.

Behavioral outcome: a gate draft whose gate names a pin `Q` that the
pinout does not have shows the loader's message with "Go to Gates" (the
step that holds `gates`); one whose footprint has 8 pads for a 14-pin
pinout shows "Go to Footprint"; a valid part shows both previews, 14 rows
of pin ↔ pad, and enabled Save Part… / Add to Library.

References:

- `Editor/PartValidator.swift` (`Failure.message`: the loader's text),
  `Editor/PartEditorModel.swift` (`validation`, `lastValidPart`,
  `save()`, `addToLibraryFromMenu()`).
- Message shape: `<source>: devices[0] (<device id>): parts[0] (<name>):
  <message>` — `backend/tests/part_file_test.c:172` (`"devices[0]
  (1G00): parts[0] (MY-NAND): duplicate part name \"MY-NAND\""`); places
  are pushed with `kind_push` (`"devices[%zu]"`, `"datasheets[%zu]"`, …).
- `Editor/BlockPreviewView.swift`, `Editor/FootprintPreviewView.swift`,
  `CatalogPart` (pins, footprint pads).
- plan-46-D (`WizardStep.keys`), plan-46-E (`issues(in:)`).

## Prerequisites

plan-46-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-46-J complete | `ls planning/completed/plan-46-J-*.md` → exists | MET (2026-11-16: `planning/completed/plan-46-J-wizard-footprint-step.md`) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- `Editor/Wizard/WizardIssueLocator.swift`:
  `step(for message: String, steps: [WizardStep]) -> WizardStep.ID?`:
  1. Split the message at `": "` into places and the text (the last part).
  2. A place starting `footprints[` → footprint; `datasheets[` →
     datasheet.
  3. A place `pinouts` or starting with a pinout name the draft has, or
     text containing `pin "` with one of `unknown pin`, `duplicate pin`,
     `pin count` → pinout when no kind key is quoted in the text.
  4. The first double-quoted word in the text that is a top-level kind key
     (side-qualified lookup over `steps[].keys`, device first) → the step
     holding it; a quoted key nested in an object → the step holding its
     top-level parent (`FieldType.nestedKeys`).
  5. Quoted `name`, `manufacturer`, `package`, `datasheet` → identity
     (`datasheet` → datasheet); `footprint`, `connectorFootprint` →
     footprint; `vcc`, `inputMax` → supply.
  6. Otherwise nil (the message shows with no Go button).
- `ReviewStepPage`: Block and Footprint previews (of `lastValidPart`), the
  pin ↔ pad table (designator, name, role, pad found yes/no from the
  validated part's footprint pads), "Still to do" (every step's issues,
  each with Go to), the verdict with Go to (the locator), Save Part… and
  Add to Library.

### Non-goals (explicit constraints)

- **The backend's messages are not changed or reworded**; the wizard adds
  a pointer beside them.
- **No auto-fixes.**

## 2. Current State

- The form shows the loader's message in red at the top of the form
  (`PartEditorWindow.swift:144–161`) with no pointer to a section.
- Save and Add are toolbar buttons, disabled while the backend rejects the
  draft.

## 3. Tests (`WizardIssueLocatorTests`)

Each case mutates a valid gate draft (14 pins, 4 NAND gates, family HC,
`SOIC-14`), validates it with `PartValidator.shared`, and runs the locator
on the real message:

1. a gate's `output` set to `Q` (no such pin) → the step holding
   `device.gates`;
2. footprint `SOIC-8` (8 pads, 14 pins) → footprint;
3. two pins both numbered `3` → pinout;
4. `family` `"ZZ"` (not a timing family) → the step holding `part.family`;
5. an inline datasheet with an `http://` URL → datasheet;
6. empty name → identity;
7. `vcc` `[5, 2]` (min above max) → supply.

Each expected step id is asserted; the test prints every message, so a
loader wording change that breaks a rule shows which.

## Phases

> **NOTE — keep the checkboxes current as you go.** **An unticked box means NOT DONE.**

### Phase 1 — The locator

- [x] `WizardIssueLocator.swift` per § 1 (§ Corrections 1).
- [x] `schematicalTests/WizardIssueLocatorTests.swift` per § 3. (The seven
      messages and where they point, as printed: `pinouts.default:
      gates[0]: no pin named "Q"` → kind.essentials; `footprint "SOIC-8"
      has 8 pads, pinout "default" has 14 pins` → footprint;
      `pinouts.default: pin 4: pin numbers must be 1..N in order (found 3)`
      → pinout; `unknown family "ZZ"` → kind.essentials; `datasheets[0]:
      datasheet "CUSTOM-X": "url" must start with https://` → datasheet;
      `parts[0] (): empty part name` → identity; `"vcc": min must be below
      max` → kind.supply. Seven more messages are asserted too. `xcodebuild
      … test -only-testing:schematicalTests/WizardIssueLocatorTests` →
      `Executed 5 tests, with 0 failures`, `** TEST SUCCEEDED **`.)

Acceptance: all seven real messages point at the right step.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p46-dd test -only-testing:schematicalTests/WizardIssueLocatorTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

### Phase 2 — The page

- [ ] `Editor/Wizard/ReviewStepPage.swift`, the pin ↔ pad table, "Still to
      do", Save Part… and Add to Library on the page.
- [ ] Runtime check (run skill): make case 1 by hand → Review → "Go to
      Gates" goes there → fix → Review shows both previews and 14 rows →
      Add to Library → the part is in the drawer. Pointer to (0, 0).

Acceptance: the review leads the user to the problem and then to a saved part.
  Check: the walk above (est. 10 min).
Commit: —

## Validation Plan

- Tests: `WizardIssueLocatorTests` (real loader messages, not fixtures).
- Runtime proof: Phase 2's walk.
- Doc sync: N.
- Final gate: plan-46-N.

## Decisions

plan-46-A § Decisions. Author's call: messages stay the backend's own;
the wizard points, it does not reword.

## Corrections

1. **The locator's rules, as the real messages needed them.** § 1 wrote the
   rules from one known message shape; the loader's actual texts differ in
   three ways, so the rules are ordered and widened (the tests assert the
   outcome on real messages, which is the criterion):
   - A *place* can be inside a kind key: `pinouts.default: gates[0]: no pin
     named "Q"`. Places are checked for a kind key (`gates[0]` → `gates`,
     `timing.HC` → `timing`) *before* the pinout rule, or case 1 would go
     to Pins because of its `pinouts.default` place.
   - The text is `no pin named "Q"`, not `unknown pin`; and a bad pin
     number reads `pin numbers must be 1..N in order`. The pinout rule goes
     by the `pinouts`/`pinouts.<name>` place first.
   - Several messages name their subject in words, not as a quoted key:
     `footprint "SOIC-8" has 8 pads`, `unknown family "ZZ"`, `empty part
     name`, `unknown datasheet "x"`. After the quoted-key rules, the
     words `footprint`/`pads`, `datasheet`, `part name` and any bare word
     that is a kind key (`family`) are tried.
   The step ids are D's (`kind.essentials`, `kind.supply`): the gate's
   derived steps hold `gates`, `timing` and `family` in Essentials until
   L's gate guide renames them.
2. **Review lists the wizard's own findings as well as the backend's.**
   "Still to do" is E's step issues; for case 1 it reads `Gates 1 ›
   Output: there is no pin named “Q”` with its own Go to, above the
   loader's message and its Go to. The pin ↔ pad table shows `No lead`
   for an `absent` pin and `—` while the part does not load (its
   footprint is then not known).

## Summary

String rules over real messages; a rule that misses only loses a Go
button — the message is always shown.
