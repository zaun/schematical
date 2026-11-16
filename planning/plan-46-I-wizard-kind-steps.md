# plan-46-I: Part Wizard — step 5 (the kind's own screens): model picker, derived pages, bus-style conditions

Last updated: 2026-11-16
Effort: medium (1h–2h)
Depends on: plan-46-H

The kind's keys are where a form becomes a wall: serial has 126 top-level
keys, converter 205 fields in all. I turns them into screens a person can
climb. For a kind with variants the first screen is **"Which chip does
yours behave like?"**: each model with a short description and real
example parts, because choosing the model is most of the work (an i8251
needs 12 top-level keys, not 126). Then the derived steps of D:
Essentials, then one optional screen per group with **Skip**. Inside the
converter and rtc `bus` object, the keys that belong to other bus styles
are hidden once a style is chosen, so an SPI converter shows six bus keys,
not twenty-five.

Behavioral outcome: a new serial part on the model step lists 11 models
with two example parts each; choosing `i8251` leaves only the i8251
screens; a new converter whose `bus.style` is `spi` shows `cs`, `sclk`,
`mosi`, `miso`, `mode`, `csActive` and a "More bus settings" disclosure,
and none of the parallel or clocked keys.

References:

- plan-46-D § 3.2 (derived steps, side-qualified keys), plan-46-E
  (`KindFieldsSection(keys:)`, the step page).
- `Editor/KindSchema.swift` (`Field.variants`, `KindSchema.variantKey`,
  `deviceFields(variant:)`), `Editor/FieldEditors.swift` (`FieldsEditor`
  :348, the nested object/list controls).
- `Editor/BundledPartFile.swift` (`file(kind:)` :12, the bundled kind file).
- `docs/parts-format.md` § `converter` › `bus` (:2624–2702: which keys each
  style uses).
- `Editor/ConverterDraft.swift`, `Editor/RtcDraft.swift` (`bus` has the
  same 25 sub-keys in both; `style` suggests clocked, i2c, parallel, spi).

## Prerequisites

plan-46-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-46-H complete | `ls planning/completed/plan-46-H-*.md` → exists | NOT MET (2026-11-16: not started) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- Variant page `VariantStepPage`: one row per value of the variant key's
  field (its `.choice`/`.suggested` list), each with the value, a blurb
  (`PartKinds.variantBlurbs[kind]?[value]`, empty until M; the row then
  shows the value alone) and up to 2 example part names
  (`BundledPartFile.examplesByVariant(kind:)`: the first 2 part names, in
  file order, of the bundled devices with that value). Choosing a value
  sets it; when the draft holds values for keys the new value does not
  have (`Field.variants`), an alert lists them and removes them on OK.
- `FieldCondition` (`Editor/Wizard/FieldConditions.swift`): `struct
  FieldCondition { let sibling: String; let values: Set<String> }` and
  `PartKinds.conditions: [String: [String: FieldCondition]]` — kind → dotted
  path (`"bus.cs"`) → condition. `FieldsEditor` gains `conditions:
  [String: FieldCondition]` and a `guided: Bool`; in guided mode, a field
  whose condition's sibling holds a value outside `values` is hidden
  (never cleared). An object with a condition table shows its
  conditioned fields that pass, then its unconditioned fields under a
  "More … settings" disclosure (below). In All fields mode nothing is
  hidden and there is no disclosure.
- The table for `converter` and `rtc`, from `docs/parts-format.md:2624–2702`
  (sibling `style` inside `bus`):
  - spi: `cs`, `sclk`, `mosi`, `miso`, `mode`, `csActive`
  - i2c: `scl`, `sda`, `address`, `table`
  - parallel: `data`, `cs`, `rd`, `wr`, `rw`, `address`, `byteSelect`
  - clocked: `clock`, `data`, `oe`, `clockOut`
  - (a key in several lists is shown for each of those styles)
  - `ale`, `i2c`, `selectPin`, `selectSampled`, `spi`, `spiLevel`: no
    condition, under "More bus settings"; `style` always first.
  The 18 listed keys + 6 + `style` = the 25 sub-keys of `bus` (planning
  probe over both generated files).
- Optional steps show "Optional — skip this if you don't need it in the
  simulation" and a Skip button (E's nav bar); a skipped step's values
  stay as they are.
- A derived step's fields show the schema's label and, small, the key
  (`csActive`), so a user can match the docs.

### Non-goals (explicit constraints)

- **No curated content** (blurbs, plain labels, help: L and M).
- **Hidden is not removed**: a hidden field's value is kept and saved;
  only a variant change removes values, and only after the alert.
- **No condition tables for other kinds** in this plan.

## 2. Current State

- The form shows the variant key as an ordinary control among the kind's
  fields and filters fields by the chosen variant
  (`PartEditorWindow.swift:89–95`, `deviceFields(variant:)`).
- `FieldsEditor` shows every field of an object it is given.
- A variant change keeps every value, including keys the new variant does
  not have; the backend then rejects them as unknown keys.

### Measured populations

| What | Count | Command |
|---|---|---|
| Kinds with a variant key (after C) | 13, 60 values | plan-46-D § 2 |
| Serial: top-level keys, all models / i8251 | 126 / 12 | plan-46-D § 2 and its i8251 probe |
| `bus` sub-keys (converter, rtc) | 25 each | planning probe: the `bus` field's object ids in `ConverterDraft.swift` / `RtcDraft.swift` → address, ale, byteSelect, clock, clockOut, cs, csActive, data, i2c, miso, mode, mosi, oe, rd, rw, scl, sclk, sda, selectPin, selectSampled, spi, spiLevel, style, table, wr |

## Phases

> **NOTE — keep the checkboxes current as you go.** **An unticked box means NOT DONE.**

### Phase 1 — Model rules

- [ ] `BundledPartFile.examplesByVariant(kind:)`; the variant-change
      cleanup (`PartDraft.keysOutside(variant:)`, `removing(keys:)`);
      `FieldConditions.swift` with the converter/rtc table.
- [ ] `schematicalTests/WizardKindStepsTests.swift`: serial
      `examplesByVariant` has 11 entries, each with 1–2 bundled names;
      a counter draft with `sections` set, switched from `ripple` to
      `sync`, reports `sections` as outside the variant; the condition
      table's keys are exactly the 25 `bus` sub-keys minus `style`, split
      18 conditioned + 6 unconditioned; with `style = "spi"` the
      conditioned keys shown are exactly `cs`, `sclk`, `mosi`, `miso`,
      `mode`, `csActive`, and the hidden ones exactly `address`, `data`,
      `rd`, `wr`, `rw`, `byteSelect`, `clock`, `oe`, `clockOut`, `scl`,
      `sda`, `table` (`address` belongs to i2c and parallel only).

Acceptance: the rules give the lists above.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p46-dd test -only-testing:schematicalTests/WizardKindStepsTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

### Phase 2 — The pages

- [ ] `Editor/Wizard/VariantStepPage.swift`; `FieldsEditor` conditions and
      the disclosure; the Optional header and the key captions.
- [ ] Runtime check (run skill): New Part… → Serial → the model step
      lists 11 models with examples → `i8251` → the step list shrinks to
      the i8251 steps; New Part… → Converter → `adc` → Essentials →
      `bus.style` `spi` → six bus keys and "More bus settings"; switch to
      All fields → every bus key is there. Pointer to (0, 0).

Acceptance: a complex kind is a short series of screens in Guided mode and unchanged in All fields.
  Check: the walk above (est. 10 min).
Commit: —

## Validation Plan

- Tests: `WizardKindStepsTests`; D's coverage invariant still holds
  (conditions hide controls, they do not change step keys).
- Runtime proof: Phase 2's walk.
- Doc sync: N.
- Final gate: plan-46-N.

## Decisions

plan-46-A § Decisions. Author's calls: hidden fields keep their values;
a variant change removes other-variant values only after an alert listing
them.

## Corrections

None yet.

## Summary

The cleanup on variant change is the only data-changing rule, and it asks
first. Conditions are display-only and cannot lose data.
