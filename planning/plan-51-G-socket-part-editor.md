# plan-51-G: IC sockets — the Part Editor (File › New Part…, Copy to Editor…)

Last updated: 2026-11-21
Effort: medium (1h–2h)
Depends on: plan-51-F

G makes the `socket` kind work in the Part Editor and the Part Wizard, as
`docs/backend.md` § Adding a kind step 6 requires of every kind: the kind's
information card, its form schema (generated from the sources and the shipped
parts F added), tests; plus the one thing no earlier kind needed: the form edits
the **`"fits"`** key, which is required on a socket and is a common part key (not
in the model's `KEYS[]`), as `footprintChoices` is.

Outcome of G: File › New Part… offers **IC socket**; a socket authored in the
editor (a custom footprint, `fits` naming bundled chip footprints) saves, reloads
and registers; Copy to Editor… on a bundled socket round-trips with no loss;
`PartKindKeysTests`, `PartKindRoundTripTests`, `KindInfoTests`, `KindGuideTests`
and the new `SocketEditorTests` pass.

References:

- `docs/backend.md` § Adding a kind (step 6: `KindInfo`, `PartKinds.all`, `infoOrder`,
  `scripts/part-editor-schema.py`, `PartKindKeysTests.files`, the generated `…Draft.swift`,
  `PartKinds.schemas`, `PartKindRoundTripTests`).
- `schematical/schematical/Editor/`: `KindInfo.swift`, `KindSchema.swift`, `PartDraft.swift`,
  `PartEditorSections.swift`, `Wizard/FootprintStepPage.swift`, `Wizard/WizardIssueLocator.swift`.
- `docs/parts-format.md` § Custom footprints in a part file ("A custom part has a
  `"footprint"` only …"), § Single part files.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-51-F complete | `ls planning/completed/plan-51-F-*.md` → exists | NOT MET (2026-11-21: not started) |
| Socket parts ship | `python3 -I -c "import json;print(json.load(open('schematical/schematical/Parts/socket.json'))['kind'])"` → `socket` | NOT MET (F's output) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> If you stop, report the status of *all* prerequisites.

## 1. Goal

- The § Outcome tests pass: `xcodebuild … test -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/KindInfoTests -only-testing:schematicalTests/KindGuideTests -only-testing:schematicalTests/SocketEditorTests` → `** TEST SUCCEEDED **`.

### Non-goals (explicit constraints)

- No backend change; the loader's `"fits"` rules (plan-51-A) are the validation.
- No change to other kinds' forms. `connectorFootprint` and `footprintChoices` stay off custom parts.
- No new wizard page: `fits` sits on the existing Footprint page.

## 2. Current State

- A kind needs, per `docs/backend.md`: a `KindInfo` (`KindInfo.swift`; the group is one
  of `KindGroup` — `connectors` is "Connectors and switches"), its name in `PartKinds.all`
  and `infoOrder`, `"socket": ["socket.c"]` in `scripts/part-editor-schema.py` `KIND_FILES`
  and in `PartKindKeysTests.files`, the generated `Editor/SocketDraft.swift`
  (`python3 scripts/part-editor-schema.py . socket` must print `missing []`), the schema
  in `PartKinds.schemas`, and `testSocket` + the shipped count in `PartKindRoundTripTests`.
- The generic part keys the form edits are a fixed set
  (`PartDraft.genericPartKeys`, `PartDraft.swift:208–209`: …, `connectorFootprint`,
  `footprintChoices`); `PartDraft` keeps `connectorFootprint` and `footprintChoices` as
  fields (`:178–181`), reads them (`:346–351`) and writes them (`:452–453`);
  `PartEditorSections.bundledFootprint` (`:120–160`) edits them; `WizardIssueLocator`
  sends a loader message that names them to the footprint page (`:51`).
- `KindInfoTests.testEveryGroupHasAKindAndEveryKindIsListedOnce` requires every kind in
  `PartKinds.all` in exactly one group (`KindInfoTests.swift:44–53`).

## 3. Design

- `KindInfo` for `socket`: group `.connectors`; name `IC socket`; blurb "A socket a DIP or
  PLCC chip plugs into: the socket's footprint replaces the chip's on the board."; examples
  `GENERIC-SOCKET-DIP-14-300`, `GENERIC-SOCKET-PLCC-44`; `simulated: true`.
- `fits`: `PartDraft.fits: [String]?`, in `genericPartKeys`; read (`part["fits"]` array of
  strings, removed from the dictionary) and written beside `footprintChoices`. The form
  shows a **Fits** list (a checkbox per bundled footprint whose `pins` equals the pinout's pin
  count, the part's own footprint excluded) in the Footprint section, for the `socket`
  kind only; the wizard's Footprint page shows the same list.
  `WizardIssueLocator`: `"fits"` → the footprint page.
- A custom socket keeps its own `"footprint"` (the Custom Footprint… path) and names
  bundled chip footprints in `"fits"`; this is the one case a custom part has a
  list of bundled footprints (`docs/parts-format.md` updated).

Rejected: leaving `fits` out of the form and letting a custom socket fail with
`missing "fits"`: the wizard would offer a kind whose every part is rejected.

**Risk.** Low: a new kind entry and one more optional field in a draft that already
carries two. `PartDraftRoundTripTests` and the kind round-trip test show a socket
survives open → edit → save unchanged.

## 4. Phases

### Phase 1 — the kind entry and schema

- [ ] `KindInfo.swift`: the `socket` entry, `PartKinds.all`, `infoOrder`; `scripts/part-editor-schema.py` `KIND_FILES` and `PartKindKeysTests.files`: `"socket": ["socket.c"]`.
- [ ] `python3 scripts/part-editor-schema.py . socket` → `Editor/SocketDraft.swift`, output ends `missing []`; `PartKinds.schemas` (`KindSchema.swift`) lists it.
- [ ] `PartKindRoundTripTests`: `testSocket` and the shipped count (every shipped socket: 0 mismatched, 0 with extras, 0 rejected).

Acceptance: the four existing test classes of § Goal pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p51-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/KindInfoTests -only-testing:schematicalTests/KindGuideTests` → `** TEST SUCCEEDED **` (est. 12 min).
Commit: —

### Phase 2 — `fits`

- [ ] `PartDraft.swift`: the `fits` field, `genericPartKeys`, read and write.
- [ ] `PartEditorSections.swift` and `Wizard/FootprintStepPage.swift`: the Fits list per § 3; `WizardIssueLocator.swift`: `"fits"`.
- [ ] `schematicalTests/SocketEditorTests.swift`: a draft from `GENERIC-SOCKET-DIP-14-300` encodes to the shipped JSON (`fits` equal); unchecking `CDIP-14` and saving gives `"fits": ["PDIP-14"]`; a draft with no fits is invalid with the loader's `missing "fits"` message and the locator sends it to the footprint page; the Fits list offers only 14-pad footprints.
- [ ] `docs/parts-format.md` § Custom footprints in a part file: a custom socket's `"fits"`.

Acceptance: `SocketEditorTests` and the Phase 1 classes pass.
  Check: the Phase 1 command with `-only-testing:schematicalTests/SocketEditorTests` added → `** TEST SUCCEEDED **` (est. 13 min).
Commit: —

## Validation Plan

- Tests: as § Goal; negative cases in `SocketEditorTests`.
- Coverage check: `PartKindRoundTripTests` loads every shipped socket, so a key the form drops fails.
- Runtime proof: none beyond the tests (the form is data-driven from the schema).
- Doc sync: `docs/parts-format.md`; `docs/backend.md` needs none.
- Final gate: plan-51-J.

## Decisions

See plan-51-A § Decisions. Author decisions:

- **The kind's group is "Connectors and switches"** (`KindGroup.connectors`).
- **`fits` is edited on the Footprint page and section**, not on a page of its own.

## Corrections

## Summary

G's risk is small. Untouched: the backend, the other kinds' forms, the app's canvas.
