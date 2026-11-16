# plan-46-L: Part Wizard — guidance framework, shared guidance, the gate guide, the guide drift test

Last updated: 2026-11-16
Effort: medium (1h–2h)
Depends on: plan-46-K

Up to here the wizard is a guided form: it orders and groups the keys, but
labels are the schema's ("Csactive") and nothing says where a value comes
from. L adds the **guidance layer**: plain labels, one-line help, **where
to find it** (the datasheet section titles, with their other names) with a
Find button per field (G's finder), and example values. Guidance comes
from three places, first match wins: the kind's **curated guide**, the
**shared guides** (keys many kinds have: supply range, rise and fall
delays, logic family, notes), and nothing (the schema's label). L ships
the framework, the shared guides, the generic steps' intros, and the
**gate** guide as the worked example; M writes the other twelve simple
kinds. A **drift test** keeps every guide honest as kinds change.

Behavioral outcome: on a new gate part, the Supply step reads "Supply
voltage range" with "Usually under: Recommended Operating Conditions
(also: Operating Conditions)"; pressing its Find button jumps the PDF
there; the gate's steps are "Supply voltage", "Gates", "Logic family and
timing"; adding a key to the gate schema without a guide step fails
`KindGuideTests` naming `gate` and the key.

References:

- plan-46-D § 3.2 (`WizardStep`, `KindGuide`, the coverage invariant),
  plan-46-G (`DatasheetFinder`, `StepSearchTerms`), plan-46-I
  (`FieldsEditor` guided mode).
- `Editor/GateDraft.swift` (gate keys: device `gates` {op, inputs,
  output}, `output`, `timing` {rise, fall, otherLow, otherHigh, note,
  vcc, inputMax}, `units`; part `family`, `vcc`, `inputMax`),
  `Editor/KindSchema.swift` `SchemaValues` (`delays`, `supply`).
- `docs/parts-format.md` § `gate` (:1066–1105), § Supply voltage
  (:261–292).
- `schematical/schematical/Datasheets/gate-and/*.md` (83 converted gate
  datasheets: their `##` headings are the section names the gate guide
  cites).

## Prerequisites

plan-46-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-46-K complete | `ls planning/completed/plan-46-K-*.md` → exists | MET (2026-11-16: `planning/completed/plan-46-K-wizard-review-step.md`) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- `Editor/Wizard/Guidance.swift`:
  ```swift
  struct FieldGuide: Sendable {
      var label: String? = nil       // plain label; nil keeps the schema's
      var help: String               // one line: what it is, in plain words
      var whereToFind: [String] = [] // section titles, most common first
      var alsoCalled: [String] = []  // the datasheet's other names for the value
      var example: String? = nil     // shown as the placeholder
  }
  ```
  `KindGuide` (from D) gains `fields: [String: FieldGuide]` keyed by
  side-qualified dotted path (`"device.gates.op"`, `"part.family"`);
  `SharedGuides.fields: [String: FieldGuide]` keyed by bare key (`vcc`,
  `inputMax`, `rise`, `fall`, `note`, `family`, `otherLow`, `otherHigh`),
  applying at any depth; `Guidance.guide(kind:, path:) -> FieldGuide?`:
  the kind guide's path, else the shared key, else nil.
- `FieldsEditor` in guided mode shows, per field: the guide's label (else
  the schema's), the help line, "Usually under: A (also: B)" from
  `whereToFind` with a magnifier that calls `DatasheetFinder.find(whereToFind)`,
  "Also called: …" from `alsoCalled`, and the example as the placeholder.
- Generic steps' intros (`StepGuides`): identity, datasheet, pins,
  footprint, review — one or two sentences each, plain words (for pins:
  what power / ground / input / output / inout / nc / absent mean, and the
  top-view note).
- Shared guides (content, at minimum):
  - `vcc` — label "Supply voltage range", whereToFind ["Recommended
    Operating Conditions", "Operating Conditions"], alsoCalled ["VCC",
    "VDD", "V+", "Supply voltage"], example `2 to 6 V`;
  - `inputMax` — "Highest input voltage", whereToFind ["Recommended
    Operating Conditions", "Absolute Maximum Ratings"], alsoCalled
    ["VI", "VIN", "Input voltage"];
  - `rise` / `fall` — "Delay, output rising / falling", whereToFind
    ["Switching Characteristics", "AC Characteristics", "Timing
    Characteristics"], alsoCalled ["tPLH" / "tPHL", "tpd"], help: use the
    typical value at the supply you will run it at, in ns;
  - `family` — "Logic family", whereToFind ["Ordering Information",
    "Device Information"], help: the letters after 74 (`74HC00` → HC);
  - `note`, `otherLow`, `otherHigh` — help lines.
- `Editor/Wizard/Guides/GateGuide.swift`: steps "Supply voltage" (`part.vcc`,
  `part.inputMax`), "Gates" (`device.gates`, `device.output`,
  `device.units`), "Logic family and timing" (`part.family`,
  `device.timing`), each with an intro and searchTerms; field guides for
  `device.gates`, `device.gates.op`, `device.gates.inputs`,
  `device.gates.output`, `device.output`, `device.units`,
  `device.timing`; registered in `PartKinds.guides["gate"]`.
- `schematicalTests/KindGuideTests.swift` (the drift test):
  1. every curated guide, for every variant, covers the kind's
     side-qualified top-level keys exactly once (D's invariant, on the
     guide's own steps);
  2. every path in a guide's `fields` resolves to a schema field (walking
     `.object` / `.list` / `.map` / `.valueMap` / `.listOf` / `.oneOf`);
  3. every `SharedGuides` key is a field key in at least one schema;
  4. every curated step has at least one search term;
  5. for `gate`, every `whereToFind` string of every gate and shared guide
     it uses matches (case-insensitive) a `##` heading of at least one of
     the 83 `Datasheets/gate-and/*.md` files.

### Non-goals (explicit constraints)

- **All fields mode keeps the schema labels** and shows no guidance
  (it is the expert view).
- **No guide for kinds outside gate** in L (M).

## 2. Current State

- `Field.help` is the only guidance; 9 `help:` strings exist across the
  schemas, none in a generated file (`grep -c 'help:'` per file, planning
  census: 0 in each generated `…Draft.swift`; 9 in `GateDraft.swift` and
  `KindSchema.swift` together).
- Headings in the gate markdown datasheets include "Pin functions",
  "Function tables", "Recommended operating conditions", "Switching
  characteristics", "Ordering information", "Package"
  (`grep -n '^## ' Datasheets/gate-and/cd74hc08.md`).

## Phases

> **NOTE — keep the checkboxes current as you go.** **An unticked box means NOT DONE.**

### Phase 1 — Framework, shared guides, step intros

- [x] `Guidance.swift`, `SharedGuides`, `StepGuides`; `FieldsEditor`
      guided rows. (The rows are drawn by `FieldEditor` with
      `FieldGuideView`, from the `\.guidedFields` path plan-46-I set up;
      the Find button reaches the window's finder through
      `\.datasheetFind`; an example is the placeholder of a single text
      field (`\.fieldExample`) and a "For example: …" line under a control
      that is not one, such as the supply range.)
- [x] `KindGuideTests` cases 2–4 (they hold with no curated kind).
      (`testGuidePathsAreSchemaKeys`, `testSharedGuidesAreKeysOfSomeSchema`,
      `testCuratedStepsHaveSearchTerms`, plus `testGuidanceLookup`.)

Acceptance: shared guidance resolves and shows.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p46-dd test -only-testing:schematicalTests/KindGuideTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

### Phase 2 — The gate guide

- [x] `Guides/GateGuide.swift`; `KindGuideTests` cases 1 and 5.
      (`testCuratedGuidesCoverEveryKeyOnce`, `testGatePointersAreDatasheetHeadings`
      (§ Corrections 1), `testGateGuideSteps`, `testAMissingKeyIsNamed`:
      `xcodebuild … test -only-testing:schematicalTests/KindGuideTests` →
      `Executed 8 tests, with 0 failures`, `** TEST SUCCEEDED **`. The gate's
      steps are now `kind.supply`, `kind.gates`, `kind.timing`; the tests
      that named its derived steps (`kind.essentials`, `kind.other`) were
      updated, and the derived rule is still asserted on the gate schema by
      `WizardStepsTests.testDerivedGateSteps`: 54 tests over the seven
      wizard test classes, 0 failures.)
- [ ] Runtime check (run skill): New Part… → Gate → the three gate
      steps with their intros; with the SN74HC00 PDF loaded, every Find
      button on the Supply and timing steps lands on a page (none says
      "Not found"). Pointer to (0, 0).

Acceptance: the gate wizard reads as a guide and every pointer finds its section.
  Check: `xcodebuild … test -only-testing:schematicalTests/KindGuideTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min) and the walk (est. 10 min).
Commit: —

## Validation Plan

- Tests: `KindGuideTests` (the drift test for every future guide).
- Runtime proof: Phase 2's walk.
- Doc sync: N (how to add a guide).
- Final gate: plan-46-N.

## Decisions

plan-46-A § Decisions (curated guides for simple kinds first, gate is one
of them).

## Corrections

1. **Test (5) as written cannot hold together with § 1's own shared
   guides, so it is made exact rather than dropped.** § 1 (5) asks that
   *every* `whereToFind` string of the gate and shared guides match a `##`
   heading of the 83 converted gate datasheets. Those files' headings are
   the conversion format's own fixed set — `grep '^## '
   Datasheets/gate-and/*.md | sort | uniq -c` gives 16 distinct headings
   (Switching characteristics, Recommended operating conditions, Pinout,
   Pin functions, Package, Ordering information, Function tables,
   Electrical characteristics, Description, Absolute maximum ratings, …) —
   while § 1 itself lists, for the shared guides, titles other makers use
   that are not among them: "Operating Conditions", "AC Characteristics",
   "Timing Characteristics", "Device Information". The test now asserts,
   for every guide and step the gate wizard shows: (a) its *first* title
   (the one shown as "Usually under") is a heading of those datasheets,
   and (b) every other title is either such a heading or listed in
   `Guidance.alternateTitles`, the declared set of other makers' names.
   No title is unchecked: one that is neither fails, naming the guide.
2. **Find on a contents page.** In the walk, `Ordering Information` landed
   on page 2 of the TI PDF, its contents page: the PDF has no page titled
   so (the section is "Mechanical, Packaging, and Orderable Information"),
   so the only hit is the contents line. plan-46-G's contents-last order
   still put every title that has a real section on that section.
3. **The step-id change reaches Review.** K's "Go to Essentials" for a
   gate is now "Go to Gates" / "Go to Logic family and timing"
   (`WizardIssueLocatorTests` asserts `kind.gates` and `kind.timing`),
   which is the wording plan-46-K's behavioral outcome gave.

## Summary

A small lookup and a display change; the lasting value is the drift test,
which turns a schema change into a named failing guide instead of a
silently stale screen.
