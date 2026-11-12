# plan-32-I: Part editor kinds: sram, eeprom, switch, led, 7seg, oscillator, supply

Last updated: 2026-11-11
Effort: medium (1h–2h)
Depends on: plan-32-H

This sub-plan adds the forms for seven kinds that have no `model` switch: their device
keys are fixed per kind. They use the same two checks as the earlier sub-plans: the drift test
against the backend's key arrays, and the round trip over every shipped part with empty `extras`.

Kinds and measured populations (2026-11-11):

| Kind | Part file | Model source (KEYS arrays) | Shipped parts | First device keys |
|---|---|---|---|---|
| sram | `sram.json` | `backend/src/models/sram.c` → 2 | 172 | `address`, `data`, `chipEnable`, `chipEnableHigh`, `outputEnable`, `writeEnable` (line 740) |
| eeprom | `eeprom.json` | `backend/src/models/eeprom.c` → 2 | 174 | `address`, `data`, `chipEnable`, `outputEnable`, `writeEnable`, `pageSize` (line 859) |
| switch | `switch.json` | `backend/src/models/switch.c` → 1 | 2067 | `actuator`, `contacts`, `ratedVolts`, `momentary`, `normallyClosed`, `joined`, `bodySize` (line 23) |
| led | `led.json` | `backend/src/models/led.c` → 1 | 1 | `colors`, `anode`, `cathode` (line 13) |
| 7seg | `7seg.json` | `backend/src/models/display7.c` → 3 | 49 | `digits`, `commonAnode`, `colonAfter`, `dotAfter`, `color`, `size`, `chip`, `leds` (line 22) |
| oscillator | `oscillator.json` | `backend/src/models/oscillator.c` → 2 | 1138 | `family`, `minHz`, `maxHz`, `outputs`, `vcc`, `inputMax` (line 15) |
| supply | `supply.json` | `backend/src/models/supply.c` → 1 | 14 | `volts`, `pull` (line 11) |

Sum: 14 KEYS arrays, 3615 shipped parts. (`display7.c` is the 7seg kind's model, and its 3 arrays
are the 7seg kind's keys; the decoder kind's own segment-driver keys are a different array in
`decoder.c`, plan-32-F, and are not part of this kind.)

Note: `led.json` has one shipped part, and `supply.json` has 14. The population counts are the
round-trip sets; the single LED part is still checked.

References:

- plan-32-C — `PartDraft`, extras, `PartValidator`, `BundledPartFile`, and the gate form as the
  pattern for a fixed-key kind.
- plan-32-F — the drift test and round-trip test.
- `docs/parts-format.md` — § Supply voltage, § Values (`family`), the LED colors.
- `backend/src/models/{sram,eeprom,switch,led,display7,oscillator,supply}.c`.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-32-H complete: counter and shift pass their round trip | `xcodebuild test … -only-testing:schematicalTests/PartKindRoundTripTests` → `passed` with the counter and shift lines | MET (run 2026-11-11 in the P-32 worktree: `PartKindRoundTripTests` passed with the counter and shift lines 0/0/0) |

Everything below is written against the world where this holds.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run it before starting, and report it if you stop.

## 1. Goal

- File › New Part… enables the seven kinds; each form fills every key of its device and part
  objects (the fixed key sets in § table above).
- Every shipped part of the seven kinds (3615) round-trips with an empty `extras`, and the backend
  accepts each encoded file.

### Non-goals (explicit constraints)

- **Only these seven kinds change.** Shipped files and goldens are untouched.
- **The `switch` kind's tactile and DIP forms are one form.** Its keys are the same set; the form
  shows `momentary`, `normallyClosed` and `joined` for both, as the backend accepts them.
- **No LED color names beyond the backend's `colors`.**

## 2. Current State

- The seven kinds have one device-level key list each (no `model` key), so one form per kind
  covers all of its parts.
- `switch.json` is the largest population here (2067 parts, 184 devices), so its round trip is the
  check that matters most in this sub-plan.
- `supply.json` has `volts` and `pull` (`supply.c:11`); its parts are the rails (VCC, GND, 3.3 V …).

### Verified properties

- **The device key lists are as cited.** Read the lines in the table above in each model file (the
  `DEVICE_KEYS` line numbers come from `rg -n -m1 'DEVICE_KEYS\[\] = \{' backend/src/models/<kind>.c`).

## 3. Design Overview

1. **Drift test**: `PartKindKeysTests` gains the seven kinds (the 7seg kind's model is `display7.c`).
2. **Drafts**: `SramDraft`, `EepromDraft`, `SwitchDraft`, `LedDraft`, `SevenSegDraft`,
   `OscillatorDraft`, `SupplyDraft`, each typed for its fixed keys, with `extras` for the rest.
3. **Round trip**: `PartKindRoundTripTests` gains the seven kinds.

Where correctness risk concentrates: the `switch` kind's population (2067 parts) and the
`oscillator` (1138), which between them are most of this sub-plan's shipped parts.

### Rejected alternatives

- **One generic "fixed keys" draft for these seven.** Rejected: each kind's controls differ (a
  switch's contacts, an oscillator's frequency range, a 7-segment's digit layout); a generic draft
  would be a key grid.

## 4. Detailed Design

- **sram**: the address, data and enable pins (pin names), the `grades` the backend reads from the
  part's `grade`, `vcc` and `inputMax` (`PART_KEYS`, `sram.c:744`).
- **eeprom**: the same pin roles plus `pageSize`; the part's grade keys.
- **switch**: `actuator` (`Standard`, `Round Button` and the others the backend lists), `contacts`,
  `ratedVolts`, `momentary`, `normallyClosed`, `joined`, `bodySize` (`switch.c:23`).
- **led**: `colors` (a list of color names, the first is the default), `anode`, `cathode`.
- **7seg**: `digits`, `commonAnode`, `colonAfter`, `dotAfter`, `color`, `size`, `chip`, `leds`
  (`display7.c:22`), where `leds` is the segment-to-digit map.
- **oscillator**: `family`, `minHz`, `maxHz`, `outputs`, `vcc`, `inputMax` (`oscillator.c:15`).
- **supply**: `volts` and `pull` (`supply.c:11`).

The per-array keys are the drift test's list (Phase 1), read from `rg -n 'KEYS\[\] = '` over the
seven files.

## Phases

### Phase 1 — Drift test: the seven kinds

One line: the drift test fails listing the backend keys of the seven kinds that the form lacks.

- [x] `PartKindKeysTests.swift`: add the seven kinds; `7seg` maps to `display7.c`.
- [x] Run it; RED.

Acceptance: the test fails listing the missing keys.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartKindKeysTests` → `failed`, listing the keys (est. 3 min).
  Result (2026-11-11, RED): failed, `["supply": ["pull", "volts"], "sram": ["address", "byteEnable", "chipEnable", …], …]`; arrays parsed sram 2, eeprom 2, switch 1, led 1, 7seg 3, oscillator 2, supply 1 (as § Kinds).
Commit: cee14eb

### Phase 2 — Forms for the seven kinds

One line: each kind's form covers every key of Phase 1.

- [x] One draft per kind in `schematical/schematical/Editor/` (`SramDraft.swift`, `EepromDraft.swift`, `SwitchDraft.swift`, `LedDraft.swift`, `SevenSegDraft.swift`, `OscillatorDraft.swift`, `SupplyDraft.swift`), each with `init(device:part:)`, `encoded()`, `extras`.
- [x] `PartEditorWindow.swift`: a section per kind.
- [x] `PartDraft.enabledKinds` gains the seven kinds.

Acceptance: `PartKindKeysTests` passes for the seven kinds.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartKindKeysTests` → `passed` (est. 5 min).
  Result (2026-11-11): passed.
Commit: cee14eb

### Phase 3 — Round trip over every shipped part

One line: all 3615 shipped parts round-trip with no key in `extras`, and the backend accepts each.

- [x] `PartKindRoundTripTests.swift`: the seven kinds added to the loop.

Acceptance: zero mismatched, zero with extras, zero rejected, for sram (172), eeprom (174), switch (2067), led (1), 7seg (49), oscillator (1138) and supply (14).
  Check: `xcodebuild test … -only-testing:schematicalTests/PartKindRoundTripTests` → `passed`, with seven count lines (est. 8 min).
  Result (2026-11-11): passed: sram 172, eeprom 174, switch 2067, led 1, 7seg 49, oscillator 1138, supply 14 — each `0 mismatched, 0 with extras, 0 rejected` (and every F–H kind still 0/0/0 in the same run).
Commit: cee14eb

### Phase 4 — Runtime proof

One line: one part of a seven-kind set is created, saved and loaded back in the app.

- [ ] No code. From New Part…, create an SRAM, a 7-segment display and an oscillator; save and load each with File › Load Part….

Acceptance: each loads back into the drawer.
  Check: the run skill's session (est. 20 min).
Commit: —

## Validation Plan

- Tests: `PartKindKeysTests` (Phases 1–2), `PartKindRoundTripTests` (Phase 3).
- Coverage check: `rg -c 'KEYS\[\] = ' backend/src/models/{sram,eeprom,switch,led,display7,oscillator,supply}.c` → 2, 2, 1, 1, 3, 2, 1; the test prints its parsed count.
- Runtime proof: Phase 4.
- Doc sync: none (the format is unchanged).
- Final gate (once, after Phase 4): `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical` (est. 15 min, background per the project's tiers).

## Decisions

- **The tactile and DIP switch forms are one form** — the author's choice (§ Non-goals): their key set is one list in the backend, so one form covers both.
- **Each fixed-key kind gets its own draft** — the author's choice (§ 3, rejected: a generic draft).

## Corrections

- **Speed-grade keys are a `TIMING_NAMES[]` table** (found 2026-11-11). sram and eeprom
  `grades` are objects keyed by grade name, each entry keyed by the kind's
  `TIMING_NAMES[]` (`trc`, `taa`, … 24 for sram) plus `vcc`, `inputMax`, `note`. The
  generator reads `TIMING_NAMES[]` as keys, so a grade is a named map of those fields;
  timing names no shipped grade uses (sram `tas`, `tdh`, `twr`; eeprom `tas`, `tch`, `tcs`,
  `toes`, `toh`) are placed by hand so every accepted key has a control.
- **Hand-placed keys** (`scripts/part-editor-schema-extra.py`): sram/eeprom part `vcc`,
  `inputMax` and grade `inputMax` (`PART_KEYS`, `kind_part_voltage`; the grades'
  `kind_voltage`); oscillator device and part `inputMax` (`DEVICE_KEYS`, `PART_KEYS`).
- **The LED's and pull supplies' `footprintChoices`** are the generic part field of
  plan-32-F's Corrections, so led (1 part) and supply (14) round-trip with no extras.
- **Forms are generated** (plan-32-F Corrections): `SramDraft`, `EepromDraft`, `SwitchDraft`,
  `LedDraft`, `SevenSegDraft`, `OscillatorDraft`, `SupplyDraft`.

## Summary

The largest risk is the switch population (2067 parts); its round trip checks it. Untouched:
shipped files, the backend, and the other kinds.
