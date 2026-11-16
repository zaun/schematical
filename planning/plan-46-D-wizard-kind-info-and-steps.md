# plan-46-D: Part Wizard — the `KindInfo` registry and the step model

Last updated: 2026-11-16
Effort: medium (1h–2h)
Depends on: plan-46-C

D builds the wizard's data, with no UI: a **`KindInfo` registry** (one
entry per kind: plain-language name, one-line blurb, example parts, picker
group, whether it simulates) that replaces `PartKinds.displayNames`, and a
**step model** that turns any draft into its ordered list of wizard steps.
A kind's steps come from its curated guide when it has one (L, M) and are
otherwise **derived from its `KindSchema`**, so a kind with only a
generated schema gets a complete wizard. D also adds the pin-rename
rewrite the pinout step needs (kind fields hold pin names).

Behavioral outcome: for every kind and every variant (79 combinations),
`WizardPlan.steps(for:)` covers each top-level kind key exactly once;
every kind has a `KindInfo` whose example parts exist in the catalog; a
pin renamed with `renamePin` is renamed in every kind field that names it.

References:

- `schematical/schematical/Editor/KindSchema.swift` (`Field`, `FieldType`,
  `KindSchema.deviceFields(variant:)`/`partFields(variant:)`,
  `PartKinds.all` / `displayNames` / `schemas`).
- `schematical/schematical/Editor/PartDraft.swift` (`displayName(of:)`
  :175, `variant` :197, `kindDevice`/`kindPart`, `pinNames`).
- `schematical/schematical/Editor/PartEditorWindow.swift:110`
  (`supplyKeys` = vcc, inputMax: the form's own Supply section).
- `schematical/schematical/Backend/PartCatalog.swift:1479`
  (`part(named:)`, for the examples test).
- plan-46-A § plan-46 at a glance (the steps).

## Prerequisites

plan-46-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-46-C complete | `ls planning/completed/plan-46-C-*.md` → exists | MET (2026-11-16: `planning/completed/plan-46-C-wizard-missing-kind-schemas.md`) |
| Every kind has a schema | `grep -o 'Draft.schema' schematical/schematical/Editor/KindSchema.swift \| wc -l` → 32 | MET (2026-11-16 at a70374b1: 32) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- `Editor/KindInfo.swift`: `KindInfo` and `PartKinds.info` for all 32
  kinds; `PartKinds.displayNames` is gone and `PartDraft.displayName(of:)`
  reads `info[kind]?.name`.
- `Editor/Wizard/WizardSteps.swift`: `WizardStep`, `WizardPage`,
  `KindGuide` (steps only; L adds field guidance), `PartKinds.guides`
  (empty in D), `WizardPlan.steps(for: PartDraft) -> [WizardStep]`.
- `PartDraft.renamePin(from:to:)`.

### Non-goals (explicit constraints)

- **No UI change.** The form looks the same; display names are the same
  strings (moved, not reworded), except where § 3.1 rewords a name the
  picker shows.
- **No file format change.**
- **No content beyond names, blurbs and examples** (curated steps are L, M).

## 2. Current State

- `PartKinds.displayNames` (`KindSchema.swift`) maps 31 kinds (32 after B)
  to a display name; `PartDraft.displayName(of:)` falls back to the raw kind.
- Kind fields are `schema.deviceFields(variant:)` then
  `schema.partFields(variant:)`; the form shows `vcc`/`inputMax` in its own
  Supply section and everything else in the kind section
  (`PartEditorWindow.swift:82–95`).
- A field's `variants` restricts it to some values of `variantKey`
  (`KindSchema.swift:110–112`).
- No code rewrites pin names in kind fields: a pin renamed in the pin table
  leaves `.pin`/`.pins` values pointing at the old name (the loader then
  fails `unknown pin`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Kinds (after B) | 32 | `PartKinds.all` (31 at 9e13cbe4 + `generic`) |
| Kinds with a `variantKey` (after C) | 13 | `grep -l 'variantKey: "' schematical/schematical/Editor/*Draft.swift` → 12 at 9e13cbe4 (arith, buffer, converter, counter, decoder, diode, latch, mux, pio, register, serial, shift) + capacitor (C, `type`) |
| Variant values | 60 | arith 5, buffer 4, converter 2, counter 6, decoder 3, diode 6, latch 5, mux 3, pio 2, register 1, serial 11, shift 6 (each kind's `variantKey` field's choice list, `grep -o "key: \"<vk>\", label: \"[^\"]*\", type: \.[a-z]*(\[[^]]*\])" <Kind>Draft.swift`), capacitor 6 (§ C) |
| Kind × variant combinations to cover | 79 | 19 kinds without a variant + 60 |
| Generated top-level fields | 523 over 26 generated kinds; serial 126 (119 device + 7 part) | `python3 -I census.py schematical/schematical/Editor` (planning scratch script: per generated file, the `device:`/`part:` arrays of `KindSchema(…)`) |

### Verified properties

- **A serial model is small once chosen.** Of serial's 119 top-level device
  fields, 117 carry `variants:`; per model: ns16550 89, ay1013 16, max310x
  7, max3100 6, com78c80x 5, sc18is604 5, usbuart 5, acia6551 4, hs82c52 4,
  i8251 3, i82510 3, plus 2 shared (count of `variants: [...]` membership
  over `SerialDraft.swift`'s device list, planning probe). This is why the
  derived steps filter by variant before grouping.

## 3. Design

### 3.1 `KindInfo`

```swift
struct KindInfo: Sendable {
    let kind: String
    let name: String         // "Logic gate"
    let blurb: String        // "AND, OR, NAND, NOR, XOR and inverters: 74HC00, CD4011 …"
    let examples: [String]   // bundled part names
    let group: KindGroup
    let simulated: Bool      // false: mcu, generic
}
enum KindGroup: String, CaseIterable, Sendable {
    case gates = "Logic gates and flip-flops", logic = "Logic functions",
         memory = "Memory", processors = "Processors and interface chips",
         displays = "Displays and lights", discretes = "Passives and discretes",
         connectors = "Connectors and switches", power = "Power and clocks",
         other = "Other"
}
```

Groups: gates — gate, buffer, dff, jkff, latch; logic — mux, decoder,
arith, counter, shift, register; memory — sram, eeprom; processors — cpu,
mcu, pio, serial, converter, rtc; displays — led, 7seg, charlcd; discretes
— resistor, capacitor, diode, transistor; connectors — connector, header,
switch; power — supply, oscillator; other — generic (32).

Names: today's display names, except `gate` "Logic Gate" and `generic`
"Other part (no simulation)". Each kind gets 1–3 examples, each a bundled
part name (test-enforced); `generic` has none.

### 3.2 Steps

```swift
enum WizardPage: Hashable, Sendable {
    case kind, identity, datasheet, pinout, variant, supply, kindFields, footprint, review
}
struct WizardStep: Identifiable, Hashable, Sendable {
    let id: String            // "identity", "kind.essentials", "kind.timing", …
    let title: String
    let intro: String?
    let page: WizardPage
    let keys: [String]        // top-level kind keys this step edits (kindFields, supply)
    let optional: Bool        // shows Skip
    var searchTerms: [String] = []   // G: Find in datasheet
}
struct KindGuide: Sendable { let steps: (String?) -> [WizardStep] }  // variant → steps
enum WizardPlan {
    static func steps(for draft: PartDraft) -> [WizardStep]
}
```

`steps(for:)` = kind, identity, datasheet, pinout, then the kind steps,
then footprint (omitted for `supply` parts that are not pulls: they have no
footprint, `docs/parts-format.md` § The "footprint" part key), then review.

Kind steps for a draft of kind K and variant V:

1. Curated: `PartKinds.guides[K]?.steps(V)` when present.
2. Derived otherwise:
   - `variant` step when the schema has a `variantKey` (title "Which chip
     does yours behave like?").
   - `supply` step with the variant's `vcc`/`inputMax` fields (device or
     part), when there are any.
   - `kind.essentials`: the variant's required top-level fields, minus the
     variant key and the supply keys; omitted when empty.
   - one optional step per remaining top-level field whose type is
     `.object`, `.list`, `.map`, `.valueMap` or `.listOf` (title = the
     field's label), in schema order, device before part;
   - `kind.other`: the remaining optional scalar fields, optional; omitted
     when empty.

Keys are top-level and unique per side; a key on both sides (mcu's
`clock`, `flash`, `vcc`) is two entries `device.clock` / `part.clock` —
`keys` holds side-qualified names (`"device.gates"`, `"part.family"`).

The coverage invariant (test): for each of the 79 combinations, the
multiset of `keys` over the kind steps equals the set of side-qualified
top-level keys of `deviceFields(V)` + `partFields(V)`, the variant key
counted in the `variant` step.

### 3.3 Pin renames

`PartDraft.renamePin(from old: String, to new: String)`: when `old` names
exactly one pin of the part's pinout and `new` names none, walks every kind
field with the schema and rewrites values: `.pin` strings equal to `old`,
`.pins`' elements equal to `old`, recursing into `.object`, `.list`,
`.map`, `.valueMap`, `.listOf` and the matching `.oneOf` alternative;
`.json` values are left (no schema to read them by). Returns the count of
rewritten values. Exact full-name matches only. The pin table calls it
when a name edit is committed (H), never per keystroke.

Rejected alternatives:

- **Steps as a fixed list per kind** — every new kind would need wizard
  code; the derived list is what makes the wizard extend for free.
- **Rewriting pin names on every keystroke** — "A" → "AB" would capture
  references meant for another pin named "AB".

## Phases

> **NOTE — keep the checkboxes current as you go.** **An unticked box means NOT DONE.**

### Phase 1 — `KindInfo`

- [x] `Editor/KindInfo.swift` per § 3.1, 32 entries; `PartKinds.displayNames`
      deleted; `PartDraft.displayName(of:)` reads `PartKinds.info`.
      (`grep -rn displayNames schematical/` → none. `PartKinds.kinds(in:)`
      lists a group's kinds in § 3.1's order. `PartEditorWindowTests`'
      title assertion is now `New Logic Gate Part`, § Corrections 1.)
- [x] `schematicalTests/KindInfoTests.swift`: every `PartKinds.all` kind
      has an entry and no entry is for an unknown kind; names and blurbs
      non-empty; every example is `PartCatalogStore.shared.part(named:)`
      non-nil; `simulated == false` exactly for mcu and generic; every
      `KindGroup` has at least one kind.
      (Also: each example is a part of that kind, names are distinct.
      `xcodebuild … test -only-testing:schematicalTests/KindInfoTests
      -only-testing:schematicalTests/PartEditorWindowTests …` → `Executed 4
      tests, with 0 failures` and `Executed 9 tests, with 0 failures`,
      `** TEST SUCCEEDED **`.)

Acceptance: the registry is complete and names real parts.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p46-dd test -only-testing:schematicalTests/KindInfoTests -only-testing:schematicalTests/PartEditorWindowTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min).
Commit: 1cb1e8cb

### Phase 2 — Steps

- [x] `Editor/Wizard/WizardSteps.swift` per § 3.2.
- [x] `schematicalTests/WizardStepsTests.swift`: the coverage invariant
      over the 79 combinations; gate's steps are kind, identity,
      datasheet, pinout, supply, kind.essentials, kind.timing … footprint,
      review (the exact list asserted); a non-pull supply has no footprint
      step; serial with model `i8251` has 12 side-qualified top-level kind
      keys over its steps (device bus, clock, grades, model, serial; part
      ctsA, fiveBitStops, grade, inputMax, intPin, resetClearsData, vcc —
      planning probe over `SerialDraft.swift`).
      (79 combinations asserted: 19 + 60; the gate list per § Corrections 2;
      serial with no model has 126 keys. `xcodebuild … test
      -only-testing:schematicalTests/WizardStepsTests` → `Executed 7 tests,
      with 0 failures`, `** TEST SUCCEEDED **`.)

Acceptance: every kind × variant is covered exactly once.
  Check: `xcodebuild … test -only-testing:schematicalTests/WizardStepsTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: d91a5e00

### Phase 3 — Pin renames

- [x] `PartDraft.renamePin(from:to:)` per § 3.3.
- [x] `schematicalTests/PinRenameTests.swift`: a gate draft (`gates[0].inputs
      = ["A","B"]`, `output "Y"`) renaming `A`→`1A` rewrites 1 value; a
      converter draft's `bus.cs` (object `{pin, active}`) and `bus.data`
      (pins) are rewritten; renaming to a name another pin has rewrites 0;
      a `.json` value is untouched.
      (`xcodebuild … test -only-testing:schematicalTests/PinRenameTests` →
      `Executed 5 tests, with 0 failures`, `** TEST SUCCEEDED **`.)

Acceptance: renames reach nested fields and refuse collisions.
  Check: `xcodebuild … test -only-testing:schematicalTests/PinRenameTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

## Validation Plan

- Tests: `KindInfoTests`, `WizardStepsTests`, `PinRenameTests`.
- Coverage: the invariant runs over every schema, generated or hand-written.
- Runtime proof: E shows the steps in the window.
- Doc sync: none in D (N documents the registry in `docs/backend.md`).
- Final gate: plan-46-N.

## Decisions

plan-46-A § Decisions.

## Corrections

1. **One existing assertion changes, as § 3.1's rename implies.**
   `PartEditorWindowTests.testNewGateDraft` asserted the window title `New
   Gate Part`; with `gate` named "Logic Gate" it is `New Logic Gate Part`.
   The Non-goal ("display names are the same strings … except where § 3.1
   rewords a name") already allowed it; the test was the one place that
   held the old string.
2. **§ Phase 2's gate list was illustrative, not what § 3.2's rule gives.**
   It reads "supply, kind.essentials, kind.timing … footprint". By the
   rule, `timing` is a *required* gate key (`GateDraft.schema`), so it is in
   `kind.essentials` with `gates` and `family`; the optional keys left
   (`output`, `units`) are scalars, so they are `kind.other`. The exact list
   `WizardStepsTests.testGateSteps` asserts is `kind, identity, datasheet,
   pinout, kind.supply, kind.essentials, kind.other, footprint, review`.
   A key gets a step of its own (`kind.<key>`) when it is optional and
   holds a group or a list: `testACompositeOptionalKeyHasItsOwnStep`
   asserts `kind.sections` for a ripple counter. Step ids for the kind
   steps are `kind.variant`, `kind.supply`, `kind.essentials`, `kind.<key>`
   (`kind.part.<key>` when a device key's step took the name),
   `kind.other`. E's `WizardModelTests` and L's gate guide are written
   against these.
3. **`renamePin` does not change the pin table.** § 3.3 says when it
   applies (`old` names exactly one pin, `new` none), which is the pinout
   *before* the rename, so the caller (H's pin table) calls it first and
   then sets the pin's name. Stated in its doc comment and asserted in
   `PinRenameTests.testAGateInputIsRenamed`.

## Summary

Pure data and functions behind tests. The design weight is the coverage
invariant: it is what lets a new kind ship a wizard without wizard code.
