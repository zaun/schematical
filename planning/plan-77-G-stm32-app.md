# plan-77-G: App: Cortex-M core names, `core2`, the Part Editor, the STM32 drawer

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-77-F

G shows the new cores in the app: the facts panel names each core ("Cortex-M4", "Cortex-M7 +
Cortex-M4", "Dual Cortex-M0+"), the Part Editor offers the new values and a `core2` field, and the
drawer shows Microcontroller → STM32 → its families.

References:

- `schematical/schematical/Backend/PartCatalog.swift:761` (`case mcu(core:maxHz:flash:ram:
  eeprom:psram:)`), `:1640-1642` (built from `part.mcu?.pointee`).
- `schematical/schematical/Backend/PartFacts.swift:210` (`add("Core", mcuCoreName(core))`),
  `:754-766` (`mcuCoreName`; `"cortex-m0plus"` → `"Dual Cortex-M0+"`).
- `schematical/schematical/Editor/McuDraft.swift:10` (the `core` field's suggestions),
  `scripts/part-editor-schema.py` (generates the draft), `schematical/schematicalTests/
  PartKindKeysTests.swift` (drift test), `McuTests.swift`, `PartKindRoundTripTests.swift`
  (shipped counts).
- Precedent: `planning/completed/plan-36-H-mcu-app.md`.

## Prerequisites

plan-77-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-77-F complete | `ls planning/completed/plan-77-F-*.md` → exists | NOT MET (2026-11-25) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p77-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | not run at planning by this plan; G runs it at its start |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- `PartFacts.mcuCoreName` maps `cortex-m0` "Cortex-M0", `cortex-m0plus` "Cortex-M0+",
  `cortex-m3` "Cortex-M3", `cortex-m4` "Cortex-M4", `cortex-m7` "Cortex-M7", `cortex-m33`
  "Cortex-M33", `cortex-m55` "Cortex-M55". The Core row shows "Dual <name>" when `core2` equals
  `core`, "<core> + <core2>" when it differs, "<core>" without it. So the RP2040 still reads
  "Dual Cortex-M0+", a Cortex-M3 STM32 "Cortex-M3", an M7 + M4 device "Cortex-M7 + Cortex-M4".
- `.mcu` details carry `core2: String?`.
- `McuDraft`: the `core` suggestions list every `CORES` value; a `core2` field with the same
  suggestions, optional. `PartKindKeysTests` passes.
- The drawer lists the 300 parts under Microcontroller → STM32 → each family, and a placed
  STM32 part turns Simulation › Run off (the existing facts-only path, `isSimulatedMcu` false).

### Non-goals

- No new view, symbol or board look (the generic chip block draws an MCU).

## 2. Current State

The app reads `PartMcu.core` only; "Dual" is hard-coded on `cortex-m0plus` (PartFacts.swift:760).
`McuDraft.swift:10` suggests `["avr", "avrrc", "avrxm", "avrxt", "cortex-m0plus", "riscv",
"xtensa-lx6", "xtensa-lx7"]`.

## 3. Design

- `PartCatalog.swift`: `.mcu` gains `core2`; `:1641` passes `mcu.core2.map(string)`.
- `PartFacts.swift`: `mcuCoreName` without "Dual"; a `mcuCoreRow(core:core2:)` builds the row
  text (§ 1).
- `McuDraft.swift`: regenerated with `python3 scripts/part-editor-schema.py . mcu`, then the
  suggestion lists as § 1.
- Tests: `McuTests` — the three Core row texts above: the shipped RP2040, the shipped part of
  `verdicts.tsv`'s first `add` row, and an inline-JSON fixture device with `"core": "cortex-m7",
  "core2": "cortex-m4"`; `PartKindRoundTripTests` — the mcu shipped count + 300.

## 4. Phases

### Phase 1 — Facts and editor

- [ ] `PartCatalog.swift`, `PartFacts.swift`, `McuDraft.swift`, tests (§ 3).

Acceptance: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical
-derivedDataPath /tmp/p77-dd CODE_SIGNING_ALLOWED=NO -only-testing:schematicalTests/McuTests
-only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartKindRoundTripTests`
→ `** TEST SUCCEEDED **`.

Commit:

### Phase 2 — The drawer in the running app

- [ ] Launch the Debug build (`scripts/harness/build.sh` harness or the app), open the drawer at
      Microcontroller → STM32, place the part of `verdicts.tsv`'s first `add` row; screenshot to
      `planning/plan-77-data/g-app-check.png`; pointer to (0, 0) after (AGENTS.md).

Acceptance: the screenshot shows the STM32 families in the drawer, the placed part's facts panel
reading its `facts.tsv` core and clock, and Simulation › Run disabled.

Commit:

## 5. Validation Plan

G's acceptance commands: three test classes (about 2 min after the build) and one screenshot.
The landing tier is H's.
