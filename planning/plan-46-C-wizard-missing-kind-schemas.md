# plan-46-C: Part Wizard — editor schemas for resistor, capacitor, transistor, mcu and generic

Last updated: 2026-11-16
Effort: medium (1h–2h)
Depends on: plan-46-B

Four kinds have no Part Editor form: New Part… lists resistor, capacitor,
transistor and mcu as "Coming soon", and Copy to Editor… cannot open their
parts. B added a fifth, `generic`. C gives all five a `KindSchema`: the four
shipped kinds through `scripts/part-editor-schema.py` (the generator every
other generated kind uses), `generic` by hand (it has no shipped parts to
infer from). After C every kind is enabled, and D's derived wizard steps
have a schema for every kind.

Behavioral outcome: every one of the 5687 shipped resistor, capacitor,
transistor and mcu parts round-trips through the draft with 0 mismatched,
0 with extras and 0 rejected; the drift test covers the five kinds; the
New Part list has no "Coming soon" row.

References:

- `scripts/part-editor-schema.py` (`KIND_FILES` :14, `EXTRA`, the
  `part-editor-schema-extra.py` hook), `scripts/part-editor-schema-extra.py`.
- `backend/src/models/resistor.c:19–20`, `capacitor.c:16–17`,
  `transistor.c:27–28,90–91`, `mcu.c:15–17`, `generic.c` (B) — the `KEYS`
  arrays.
- `schematical/schematical/Editor/KindSchema.swift` (`PartKinds.schemas`,
  `SchemaValues.supply`), `Editor/GateDraft.swift` (the hand-written schema
  precedent).
- `schematicalTests/PartKindKeysTests.swift` (`files` :12–33, the subset
  assertion :77–80), `schematicalTests/PartKindRoundTripTests.swift`
  (`expected` :11–18), `schematicalTests/PartEditorWindowTests.swift:12–18`
  (`formless`).
- `planning/completed/plan-32-J-part-editor-kinds-io-converter-diode.md`
  (the last batch of generated kinds; its § Corrections lists the generator
  pitfalls).

## Prerequisites

plan-46-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-46-B complete | `ls planning/completed/plan-46-B-*.md` → exists | MET (2026-11-16: `planning/completed/plan-46-B-wizard-generic-kind.md`) |
| The four kinds are formless | `ls schematical/schematical/Editor/{Resistor,Capacitor,Transistor,Mcu,Generic}Draft.swift` → 5 × no such file | MET (2026-11-16 at 8e88dc3c: 5 `No such file` lines) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- `Editor/ResistorDraft.swift`, `CapacitorDraft.swift`,
  `TransistorDraft.swift`, `McuDraft.swift` generated; `GenericDraft.swift`
  hand-written; all five in `PartKinds.schemas`.
- `PartDraft.enabledKinds == PartDraft.kindNames` (32 kinds).
- Round trip: resistor 1975, capacitor 1032, transistor 1256, mcu 1424
  parts, each 0 / 0 / 0.

### Non-goals (explicit constraints)

- **No part file changes**; goldens byte-identical.
- **No hand edits to generated files.** Shape fixes go in
  `part-editor-schema-extra.py`, so a regeneration reproduces them.
- **No wizard UI** (D on).

## 2. Current State

- `PartKinds.schemas` holds 27 schemas (`GateDraft` hand-written, 26
  generated); `enabledKinds` filters `kindNames` by it
  (`PartDraft.swift:171`).
- The drift test asserts the backend `KEYS` ⊆ the form's keys
  (`PartKindKeysTests.swift:77`), so a form key the backend reads outside a
  `KEYS` array (mcu's `"simulated"`) is allowed.
- `PartDraft(newOf:)` writes every required field's empty value
  (`PartDraft.swift:217–224`): a `.bool` required field starts `false`.

### Measured populations

| What | Count | Command |
|---|---|---|
| Shipped parts per kind (the round-trip `expected`) | resistor 1975, capacitor 1032, transistor 1256, mcu 1424 | `grep -o '"manufacturer"' schematical/schematical/Parts/<kind>.json \| wc -l` |
| Generated fields (probe) | resistor 6, capacitor 7, transistor 21, mcu 11 (13 as landed: the two `inputMax` `EXTRA` lines, § Corrections 2) | probe below, `grep -c 'Field(' <scratch>/<Kind>Draft.swift` |
| Capacitor `type` variants | 6 | `python3 -I -c "import json;print(sorted({d['type'] for d in json.load(open('schematical/schematical/Parts/capacitor.json'))['devices']}))"` → aluminum, aluminum-polymer, ceramic, niobium-oxide, tantalum, tantalum-polymer |

### Verified properties

- **The generator handles the four kinds.** Probe (2026-11-16, scratch copy
  of `backend/src/models` and `Parts/`, the script with four `KIND_FILES`
  lines added, run as `python3 -I part-editor-schema.py <scratch> resistor
  capacitor transistor mcu`) printed: `resistor: 56 devices, 1975 parts,
  variant None, 6 source keys, missing []`; `capacitor: 138 devices, 1032
  parts, variant type, 7 source keys, missing []`; `transistor: 103
  devices, 1256 parts, variant None, 21 source keys, missing []`; `mcu: 299
  devices, 1424 parts, variant None, 8 source keys, missing ['inputMax']`.
  The mcu schema it wrote has `simulated` as a required `.bool` device
  field, so a new mcu draft starts `"simulated": false` and an mcu part
  round-trips with no extras.
- **mcu's device `inputMax`** is accepted (`mcu.c:15–16` `DEVICE_KEYS`) but
  no shipped mcu uses it, so it needs an `EXTRA` line: `("device",
  "inputMax", ("number", "V"), [])`.

## 3. Design

- `scripts/part-editor-schema.py` `KIND_FILES`: `"resistor":
  ["resistor.c"]`, `"capacitor": ["capacitor.c"]`, `"transistor":
  ["transistor.c"]`, `"mcu": ["mcu.c"]`.
- `scripts/part-editor-schema-extra.py` `EXTRA["mcu"]`: the device
  `inputMax` line above.
- `Editor/GenericDraft.swift` (hand-written, the `GateDraft` shape):
  device `[Field(key: "simulated", label: "Simulated", type: .bool,
  required: true, help: "Always off: this kind has no simulation")] +
  SchemaValues.supply`; part `SchemaValues.supply`.
- `PartKinds.schemas` gains the five.
- Tests: `PartKindKeysTests.files` gains the five kinds' sources;
  `PartKindRoundTripTests` gains `testResistor`, `testCapacitor`,
  `testTransistor`, `testMcu` and their `expected` counts;
  `PartEditorWindowTests.testEnabledKinds` drops `formless` and asserts
  `enabledKinds == kindNames`; a new `testNewGenericDraftValidates` makes a
  generic draft with two pins and the "Generic" manufacturer and a bundled
  footprint of 2 pads, and the validator accepts it.

Rejected: generating `GenericDraft` — the generator infers controls from
shipped values, and the kind ships none.

## Phases

> **NOTE — keep the checkboxes current as you go.** **An unticked box means NOT DONE.**

### Phase 1 — Generate and register

- [x] The two script edits of § 3; `python3 scripts/part-editor-schema.py .
      resistor capacitor transistor mcu` → four `missing []` lines.
      (`resistor: 56 devices, 1975 parts, variant None, 6 source keys, missing []`;
      `capacitor: 138 devices, 1032 parts, variant type, 7 source keys, missing []`;
      `transistor: 103 devices, 1256 parts, variant None, 21 source keys, missing []`;
      `mcu: 299 devices, 1424 parts, variant None, 8 source keys, missing []`.
      Plus the generator fix of § Corrections 1.)
- [x] `Editor/GenericDraft.swift`; `PartKinds.schemas` gains the five.
- [x] The three test files of § 3. (`xcodebuild … test
      -only-testing:schematicalTests/PartKindKeysTests
      -only-testing:schematicalTests/PartEditorWindowTests …` → `Executed 9
      tests, with 0 failures` (window) and `Executed 1 test, with 0 failures`
      (keys), `** TEST SUCCEEDED **`.)

Acceptance: every kind is enabled and the five kinds' keys are covered.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p46-dd test -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartEditorWindowTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min).
Commit: c5ecc4c6

### Phase 2 — Round trip

- [x] Run the four new round-trip tests; any mismatch or extra is fixed by
      an `EXTRA` line and a regeneration (plan-32-J § Corrections is the
      catalog of fixes), never by a hand edit.
      (`checked 1032 parts of capacitor, 0 mismatched, 0 with extras, 0 rejected`;
      `checked 1424 parts of mcu, 0 mismatched, 0 with extras, 0 rejected`;
      `checked 1975 parts of resistor, 0 mismatched, 0 with extras, 0 rejected`;
      `checked 1256 parts of transistor, 0 mismatched, 0 with extras, 0 rejected`;
      `Executed 4 tests, with 0 failures`, `** TEST SUCCEEDED **`. No fix was needed.)

Acceptance: 5687 parts, 0 mismatched, 0 with extras, 0 rejected.
  Check: `xcodebuild … test -only-testing:schematicalTests/PartKindRoundTripTests/testResistor -only-testing:schematicalTests/PartKindRoundTripTests/testCapacitor -only-testing:schematicalTests/PartKindRoundTripTests/testTransistor -only-testing:schematicalTests/PartKindRoundTripTests/testMcu CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` and four `checked N parts of <kind>, 0 mismatched, 0 with extras, 0 rejected` lines with N = 1975, 1032, 1256, 1424 (est. 10 min: the population is the whole of each kind, the smallest set that exercises every key).
Commit: —

## Validation Plan

- Tests: the drift, window and round-trip tests above.
- Coverage: the round trip runs every shipped part of the four kinds
  through the validator.
- Runtime proof: Copy to Editor… on a resistor opens a form (F's runtime
  check covers New Part… for generic).
- Doc sync: none (the help line "pick a kind" is unchanged until N).
- Final gate: plan-46-N.

## Decisions

plan-46-A § Decisions (the four kinds are in scope, owner).

## Corrections

1. **The generator wrote strings Swift does not compile.** `capacitor`'s
   `tolerance` suggestions hold `±` (`"±10%"`), and
   `scripts/part-editor-schema.py` wrote choice and suggested values with
   `json.dumps`, whose `\u00b1` is not a Swift escape: the first build
   failed `CapacitorDraft.swift:14:92: error: expected hexadecimal code in
   braces after unicode escape`. Fixed in the generator (`json.dumps(v,
   ensure_ascii=False)` for `.choice` and `.suggested` values), then
   regenerated; no hand edit. No earlier generated file changes: none holds
   a `\u` escape (`grep -l '\\u00' schematical/schematical/Editor/*Draft.swift`
   → none), so every shipped value so far was ASCII.
2. **`mcu` needs two `EXTRA` lines, not one.** `mcu.c`'s `PART_KEYS` also
   takes `inputMax`, which no shipped mcu part sets; the generator's
   `missing []` check goes by key name, so the device line hid the part's.
   `EXTRA["mcu"]` has both; `McuDraft.swift` has 13 `Field(` (8 device,
   5 part).

## Summary

The generator has done this 26 times; the risk is a shape the inference
misses, which Phase 2's 0/0/0 catches.
