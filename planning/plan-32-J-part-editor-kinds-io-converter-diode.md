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
| plan-32-I complete: the seven fixed-key kinds pass their round trip | `xcodebuild test … -only-testing:schematicalTests/PartKindRoundTripTests` → `passed` with the seven lines | MET (run 2026-11-11 in the P-32 worktree: `PartKindRoundTripTests` passed with the seven I lines 0/0/0) |

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

- [x] `PartKindKeysTests.swift`: add the nine kinds with their source-file globs (`<kind>*.c`).
- [x] Measure the file set before the test is written: `rg -l 'KEYS\[\] = ' backend/src/models/{cpu,pio,serial,charlcd,connector,converter,rtc,diode,header}*.c` and record its output in Corrections.
- [x] Run it; RED.

Acceptance: the test fails listing the missing keys of the nine kinds.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartKindKeysTests` → `failed`, listing the keys (est. 3 min).
  Result (2026-11-11, RED): failed, `["header": ["genders", "rows"], "convert…`, every key of the nine kinds; arrays parsed cpu 2, pio 4, serial 62, charlcd 6, connector 1, converter 13, rtc 13, diode 3, header 1.
Commit: 299b328

### Phase 2 — Forms for the nine kinds

One line: each kind's form covers every key of Phase 1.

- [x] One draft per kind in `schematical/schematical/Editor/` (`CpuDraft`, `PioDraft`, `SerialDraft`, `CharLcdDraft`, `ConnectorDraft`, `ConverterDraft`, `RtcDraft`, `DiodeDraft`, `HeaderDraft`), each with `init(device:part:)`, `encoded()`, `extras`; the model- or type-keyed kinds (`pio`, `serial`, `converter`, `diode`) hold one sub-draft per model or type.
- [x] `PartEditorWindow.swift`: a section per kind.
- [x] `PartDraft.enabledKinds` gains the nine kinds; after this, all 26 kinds are enabled.

Acceptance: `PartKindKeysTests` passes for the nine kinds.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartKindKeysTests` → `passed` (est. 6 min).
  Result (2026-11-11): passed, all 26 kinds (converter without `load_clock`'s array, rtc without `load_choice`/`load_time`/`load_conversion`/`load_dac`'s, each checked to exist; Corrections).
Commit: 299b328

### Phase 3 — Round trip over every shipped part

One line: all 13811 shipped parts of the nine kinds round-trip with no key in `extras`, and the backend accepts each.

- [x] `PartKindRoundTripTests.swift`: the nine kinds added to the loop.

Acceptance: zero mismatched, zero with extras, zero rejected, for cpu (21), pio (7), serial (247), charlcd (5), connector (374), converter (1911), rtc (14), diode (11152) and header (80).
  Check: `xcodebuild test … -only-testing:schematicalTests/PartKindRoundTripTests` → `passed`, with nine count lines (est. 15 min; the diode population dominates).
  Result (2026-11-11): passed: cpu 21, pio 7, serial 247, charlcd 5, connector 374, converter 1911, rtc 14, diode 11152, header 80 — each `0 mismatched, 0 with extras, 0 rejected`; the same run has every other kind 0/0/0 and the gate test `checked 3438 parts, 0 mismatched, 0 with extras, 0 rejected`.
Commit: 299b328

### Phase 4 — Completion check and runtime proof

One line: the picker lists all 26 kinds, enabled, and one part of a new kind is saved and loaded.

- [x] `PartEditorWindowTests.swift` case (a) changes to `PartDraft.enabledKinds` equal to all 26 kinds (the list of Phase 1 of plan-32-C).
- [x] No other code. From New Part…, create a diode (Zener) and a converter (ADC); save and load each.

Acceptance: `PartEditorWindowTests` passes with all 26 enabled; the Zener and ADC parts load back into the drawer.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartEditorWindowTests` → `passed` (est. 4 min); the app session is the run skill's (est. 15 min).
  Result (2026-11-11, `/tmp/p32/scripts/guarded.sh`, this build frontmost before and after every cycle): New Part… → the kind's row opened the kind's editor, Open Part… a copy of a shipped part filled it (the window took the file's name), ⌘S rewrote the file (new mtime), and File › Load Part… registered it. The drawer count went 26,636 → 26,646 over the ten loads of this session (G's three, then H's two, I's three, J's two): ZZJ-ZENER (1N4728A, type zener) and ZZJ-ADC (ADC081S021CIMF/NOPB, type adc); the last count reads 26,646 parts. `PartEditorWindowTests` passed with all 26 kinds enabled (plan-32-J Phase 3's run).
Commit: (this commit)

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

- **Phase 1 file set** (`rg -l 'KEYS\[\] = ' backend/src/models/{cpu,pio,serial,charlcd,connector,converter,rtc,diode,header}*.c backend/src/models/ns16550*.c`,
  2026-11-11): charlcd.c connector.c converter.c converter_load.c cpu.c diode.c header.c
  ns16550.c ns16550_bridge.c ns16550_exar.c pio.c pio_pia6521.c pio_via6522.c rtc.c
  serial.c serial_acia6551.c serial_ay1013.c serial_com78c80x.c serial_hs82c52.c
  serial_i8251.c serial_i82510.c serial_max3100.c serial_max310x.c serial_sc18is604.c
  serial_target.c serial_usbuart.c. `converter_par.c` has no arrays. rtc's loader is
  converter_load.c's `cv_load_device` (rtc.c calls it), so rtc's set is rtc.c +
  converter_load.c.
- **Shared loader, per-kind branches** (2026-11-11): `cv_load_device(ctx, json, rtc)` reads
  `clock` only for rtc and `dac`, `codeFormat` and `conversion` only for converters (rtc.c's
  `DEVICE_KEYS` has no "conversion"). The drift test skips the arrays in loader functions a
  kind's branch never calls (`notCalled`: converter skips `load_clock`; rtc skips
  `load_choice`, `load_time`, `load_conversion`, `load_dac`) and checks each named function
  still has an array, so a rename fails the test.
- **A comment quoted a string inside a `KEYS[]` array** (2026-11-11): ns16550.c's
  `BUS_KEYS` has `/* plan-12-E: "intel-motorola" */`; the drift test and the generator strip
  C comments before reading the arrays.
- **Generator additions** (scripts/part-editor-schema.py): speed-grade tables
  (`TIMING_6502`/`_Z80`/`_65816`, pio's `TIMING`, the grades' `EXTRA`) are keys, so cpu and
  pio grades are named maps of their timing fields; rtc and converter take shapes from
  each other's shipped parts for the keys they share (`bus`, `configBus`, `pins`,
  `protocol`, `ready`, `registers`, `timing`, `analog`, `vcc`, `inputMax`, and the part
  keys), so each form has controls for the shared loader's keys the other kind's parts
  use; an object whose keys all read as field names (`note`, `edge`, `tAcc`) is an object,
  not a map (one `note` had made whole converter structures maps). Hand-placed keys
  (part-editor-schema-extra.py): the `i2c-spi` bus's `selectPin`, `spiLevel`, `spi`,
  `i2c` and the never-read `selectSampled` (raw JSON control), commands' `wordBits`;
  cpu/pio/serial/charlcd part `vcc`/`inputMax` and grade `inputMax`; serial's `ctsA`.
  Re-running the generator over the F–I kinds changed only CounterDraft (its `clears`
  entries are now objects of `pin`/`active`/`to`/`when` fields), which still round-trips.
- **The diode's `vz` and `pd` show for Zeners only** (§ Decisions): the generator marks
  them `variants: ["zener"]` and the form filters part fields by the device's `type`
  (`KindSchema.partFields(variant:)`); no other part key is limited by model.
- **Connector and header footprint choices** (§ Non-goals): they have controls after
  all — plan-32-F Corrections (a copy of a bundled header keeps its `footprintChoices`, and
  this sub-plan's acceptance is zero parts with extras).

## Summary

The largest risk is the diode population (11152 parts); the round trip covers all of them. This sub-plan
completes the picker: after it, all 26 kinds are enabled. Untouched: shipped files and the backend.
