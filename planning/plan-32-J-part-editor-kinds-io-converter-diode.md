# plan-32-J: Part editor kinds: cpu, pio, serial, charlcd, connector, converter, rtc, diode, header

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-32-I

This is the last kind sub-plan: it adds the forms for the nine remaining kinds, which include the
largest population in the library (the diode kind, 11152 parts) and the kinds with the most model
variants (serial, pio, converter, rtc, charlcd). It uses the same two checks as the earlier
sub-plans, and it is the one that completes the picker: after this sub-plan every one of the 26
kinds is enabled.

Kinds and measured populations (2026-11-11):

| Kind | Part file | Model source (KEYS arrays) | Shipped parts | Model-choosing key |
|---|---|---|---|---|
| cpu | `cpu.json` | `backend/src/models/cpu.c` → 2 | 21 | `core`, `grades` (`DEVICE_KEYS` at line 63) |
| pio | `pio.json` | `backend/src/models/pio.c` → 2 | 7 | `model` (`DEVICE_KEYS` at line 25; 6 lines use `"model"`) |
| serial | `serial.json` | `backend/src/models/serial.c` → 3 | 247 | `model` (`DEVICE_KEYS` at line 38; 5 lines use `"model"`) |
| charlcd | `charlcd.json` | `backend/src/models/charlcd.c` → 6 | 5 | `columns`, `rows`, `controller`, `rom`, `panel`, `bus`, `contrast`, `backlight`, `timing` (line 29) |
| connector | `connector.json` | `backend/src/models/connector.c` → 1 | 374 | `centerPin`, `ratedVolts`, `ratedAmps`, `contacts`, `usb` (line 11) |
| converter | `converter.json` | `backend/src/models/converter.c` → 2 (and `converter_load.c` 11, `converter_par.c`) | 1911 | `type`, `bits`, `channels`, `codeFormat`, `vcc`, `inputMax`, `analog`, `bus`, `configBus` (`DEVICE_KEYS` at line 1822) |
| rtc | `rtc.json` | `backend/src/models/rtc.c` → 2 | 14 | `vcc`, `inputMax`, `analog`, `bus`, `configBus`, `timing`, `registers`, `protocol`, `ready` (line 672) |
| diode | `diode.json` | `backend/src/models/diode.c` → 3 | 11152 | `type`, `diodes` (`DEVICE_KEYS` line 16); `PART_KEYS` `vr`, `io`, `vz`, `pd` (line 17) |
| header | `header.json` | `backend/src/models/header.c` → 1 | 80 | `rows`, `genders` (`DEVICE_KEYS` line 10) |

Sum: 22 KEYS arrays in the kind files (the `converter_load.c`, `converter_par.c` and `serial_*.c` /
`ns16550*.c` arrays are part of the same kinds and are included by the drift test's parse of
those kinds' source files), 13811 shipped parts.

Note: `serial`'s and `pio`'s models live in more than one file (`serial_*.c`, `pio_*.c`); the drift test
parses every `KEYS[]` array in each kind's own source files. The list of files is the set of
`backend/src/models/<kind>*.c` files that define a kind's `KEYS[]` arrays, measured with
`rg -l 'KEYS\[\] = ' backend/src/models/<kind>*.c` in Phase 1.

References:

- plan-32-C — `PartDraft`, extras, `PartValidator`, `BundledPartFile`, `enabledKinds`.
- plan-32-F — the drift test and round-trip test; the model-dependent pattern.
- `docs/parts-format.md` — § Parts, § Values (`family`), § Supply voltage, § Two supply domains, § Ball pinouts.
- `backend/src/models/{cpu,pio,serial,charlcd,connector,converter,rtc,diode,header}*.c`.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-32-I complete: the seven fixed-key kinds pass their round trip | `xcodebuild test … -only-testing:schematicalTests/PartKindRoundTripTests` → `passed` with the seven lines | NOT MET at writing; starts after plan-32-I |

Everything below is written against the world where this holds.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run it before starting, and report it if you stop.

## 1. Goal

- File › New Part… enables the nine kinds; after this sub-plan all 26 kinds are enabled.
- Every shipped part of the nine kinds (13811) round-trips with an empty `extras`, and the
  backend accepts each encoded file.

### Non-goals (explicit constraints)

- **Only these nine kinds change.** Shipped files and goldens are untouched.
- **The CPU's `grades` and the serial and pio models are the backend's.** The editor writes the
  grade and model names the backend accepts; it does not compute clock limits (the backend's
  `max_hz`).
- **Connector and header footprint choices stay bundled** (plan-32-D: `footprintChoices` and
  `connectorFootprint` are bundled-only), so the connector and header forms do not offer them.

## 2. Current State

- Of the nine kinds, five take a model-choosing key: `pio` and `serial` (`model`), `converter`
  (`type`), `diode` (`type`), and `charlcd`/`rtc`/`cpu` by their own variant keys.
- The largest population of the library is here: `diode.json` 11152 parts (`rg -o '"manufacturer"' schematical/schematical/Parts/diode.json | wc -l`).
- `converter.json` 1911 parts, `connector.json` 374, `serial.json` 247 (all `rg -o '"manufacturer"'`
  counts, 2026-11-11).

### Verified properties

- **The device key lists are as cited.** Each line number is from `rg -n -m1 'DEVICE_KEYS\[\] = \{' <file>`
  on the model file named in the table.

## 3. Design Overview

1. **Drift test**: `PartKindKeysTests` gains the nine kinds and their extra source files (§ Current State note).
2. **Drafts**: one draft per kind; `PioDraft`, `SerialDraft`, `ConverterDraft`, `DiodeDraft` with a
   model or type sub-draft; `CharLcdDraft`, `ConnectorDraft`, `RtcDraft`, `CpuDraft`, `HeaderDraft`.
3. **Round trip**: `PartKindRoundTripTests` gains the nine kinds.

Where correctness risk concentrates: the **diode population** (11152 parts). Its round trip is the
largest in this plan (roughly 80% of the shipped parts of the nine kinds). Phase 4 times it, and the
check is the round trip itself: every one of the 11152 must come back equal, with empty `extras`, and be
accepted by the backend.

### Rejected alternatives

- **Splitting the diode kind into its own sub-plan.** Rejected: its form is small (`type`, `diodes`,
  four part keys); the population is the cost, and the round trip covers it without a separate form.

## 4. Detailed Design

- **cpu**: `core`, `address`, `data`, `grades` (`cpu.c:63`) — the processor's core and speed grades.
- **pio**: `model` (`pio.c:25`) selects the parallel-I/O variant (`pia6521`, `via6522`) and its
  grades; each variant's keys are its own arrays.
- **serial**: `model` (`serial.c:38`) selects the serial model; `grades` and the channel keys are
  per model. The form shows the chosen model's sections.
- **charlcd**: `columns`, `rows`, `controller`, `rom`, `panel`, `bus`, `contrast`, `backlight`, `timing`
  (`charlcd.c:29`); the character display's bus pin roles.
- **connector**: `centerPin`, `ratedVolts`, `ratedAmps`, `contacts`, `usb` (`connector.c:11`); a USB
  connector sets `usb` to its type.
- **converter**: `type` (ADC or DAC), `bits`, `channels`, `codeFormat`, `vcc`, `inputMax`, `analog`,
  `bus`, `configBus` (`converter.c:1822`); the bus and config-bus keys are per type.
- **rtc**: `vcc`, `inputMax`, `analog`, `bus`, `configBus`, `timing`, `registers`, `protocol`, `ready`
  (`rtc.c:672`); the register list is a list editor.
- **diode**: `type` (`rectifier`, `fast`, `switching`, `schottky`, `zener`, `bridge`), `diodes` (the
  count of pairs), and the part keys `vr`, `io`, `vz`, `pd` (`diode.c:16–17`); the form shows `vz` and
  `pd` for a Zener only, as the backend reads them.
- **header**: `rows`, `genders` (`header.c:10`); a header's gender list is the backend's two names.

The per-array keys are the drift test's list (Phase 1), read from `rg -n 'KEYS\[\] = '` over each kind's
source files.

## Phases

### Phase 1 — Drift test: the nine kinds and their source files

One line: the drift test covers every `KEYS[]` array in the nine kinds' source files, and fails
listing the keys the form lacks.

- [ ] `PartKindKeysTests.swift`: add the nine kinds with their source-file globs (`<kind>*.c`).
- [ ] Measure the file set before the test is written: `rg -l 'KEYS\[\] = ' backend/src/models/{cpu,pio,serial,charlcd,connector,converter,rtc,diode,header}*.c` and record its output in Corrections.
- [ ] Run it; RED.

Acceptance: the test fails listing the missing keys of the nine kinds.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartKindKeysTests` → `failed`, listing the keys (est. 3 min).
Commit: —

### Phase 2 — Forms for the nine kinds

One line: each kind's form covers every key of Phase 1.

- [ ] One draft per kind in `schematical/schematical/Editor/` (`CpuDraft`, `PioDraft`, `SerialDraft`, `CharLcdDraft`, `ConnectorDraft`, `ConverterDraft`, `RtcDraft`, `DiodeDraft`, `HeaderDraft`), each with `init(device:part:)`, `encoded()`, `extras`; the model- or type-keyed kinds (`pio`, `serial`, `converter`, `diode`) hold one sub-draft per model or type.
- [ ] `PartEditorWindow.swift`: a section per kind.
- [ ] `PartDraft.enabledKinds` gains the nine kinds; after this, all 26 kinds are enabled.

Acceptance: `PartKindKeysTests` passes for the nine kinds.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartKindKeysTests` → `passed` (est. 6 min).
Commit: —

### Phase 3 — Round trip over every shipped part

One line: all 13811 shipped parts of the nine kinds round-trip with no key in `extras`, and the backend accepts each.

- [ ] `PartKindRoundTripTests.swift`: the nine kinds added to the loop.

Acceptance: zero mismatched, zero with extras, zero rejected, for cpu (21), pio (7), serial (247), charlcd (5), connector (374), converter (1911), rtc (14), diode (11152) and header (80).
  Check: `xcodebuild test … -only-testing:schematicalTests/PartKindRoundTripTests` → `passed`, with nine count lines (est. 15 min; the diode population dominates).
Commit: —

### Phase 4 — Completion check and runtime proof

One line: the picker lists all 26 kinds, enabled, and one part of a new kind is saved and loaded.

- [ ] `PartEditorWindowTests.swift` case (a) changes to `PartDraft.enabledKinds` equal to all 26 kinds (the list of Phase 1 of plan-32-C).
- [ ] No other code. From New Part…, create a diode (Zener) and a converter (ADC); save and load each.

Acceptance: `PartEditorWindowTests` passes with all 26 enabled; the Zener and ADC parts load back into the drawer.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartEditorWindowTests` → `passed` (est. 4 min); the app session is the run skill's (est. 15 min).
Commit: —

## Validation Plan

- Tests: `PartKindKeysTests` (Phases 1–2), `PartKindRoundTripTests` (Phase 3), `PartEditorWindowTests` (Phase 4).
- Coverage check: the Phase 1 file set (recorded in Corrections) is the set the drift test parses; the test prints its count of arrays parsed, which must equal the sum of the `rg -c 'KEYS\[\] = '` counts over the same files.
- Runtime proof: Phase 4.
- Doc sync: `docs/parts-format.md` § Parts — no change (the format is the backend's). The kind table (the five rows added by plan-32-C Phase 5) is the last doc change.
- Final gate (once, after Phase 4): `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical` (est. 15 min, background per the project's tiers) and the backend's `ctest --test-dir /tmp/logic-build` (est. 20 min, background).

## Decisions

- **The connector and header forms do not offer `connectorFootprint` or `footprintChoices`** — decided by the plan-32-D rule (custom parts have one footprint; bundled-only choices are not editable in v1).
- **The diode's `vz` and `pd` show only for a Zener** — the author's choice: `backend/include/parts.h`'s `PartDiode` comment says they are "zener only, else 0"; a form that showed them for every type would invite keys the backend ignores.

## Corrections

(Filled in during execution.)

## Summary

The largest risk is the diode population (11152 parts); the round trip covers all of them. This sub-plan
completes the picker: after it, all 26 kinds are enabled. Untouched: shipped files and the backend.
