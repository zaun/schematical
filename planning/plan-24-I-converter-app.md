# plan-24-I: The app — converters and RTCs; plan-24 landing

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-24-H

I shows the two new kinds in the drawer (Interface → ADC / DAC / RTC) and
sidebar ("Kind: ADC", "Bits", "Channels"; "Kind: RTC"), and adds a read-only
register window (double-click, like the memory window) that lists a
converter's or RTC's registers and, for an RTC, its current time. Then it
runs plan-24's landing tier.

References: `PartFacts.swift` (`kindName`, `:121`), `PartCatalog.swift`
(drawer leaves come from `drawer`/`function`, no change needed),
`planning/completed/plan-12-K-app.md` (terminal window: double-click a part
→ a window bound to a backend state API), `plan-04-C-eeprom-app.md`
(memory window).

## Prerequisites

plan-24-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-24-H complete | `ls planning/completed/plan-24-H-*.md` → exists | MET (`planning/completed/plan-24-H-dac-rtc-parts.md`, 2026-11-07 at c6bc840) |

## 1. Goal

- `** BUILD SUCCEEDED **`; `kindName` returns "ADC"/"DAC" (from `PartConverter.type`) and "RTC".
- Runtime: a running circuit with an ADS1115 written by a test program shows the written config value in the register window; an RTC's window shows the time advancing with simulated time.
- Landing tier passes.

## 2. Design

- Backend API: `circuit_converter_registers(circuit, part, out, max)` and
  `circuit_rtc_time(circuit, part, out)` in the public header (the
  `circuit_cpu_state` precedent, plan-09-B), each returning a snapshot.
- App: `RegisterWindow` (SwiftUI) bound to those calls on a timer while the
  simulation runs; opened by double-click on a `converter` or `rtc` part.
- `kindName`: `PART_CONVERTER` → "ADC"/"DAC"; `PART_RTC` → "RTC". Not
  `isLogic` (no family).

## Phases

> **Status discipline:** as plan-24-A.

### Phase 1 — API and window

- [x] Backend API + `circuit_check` cases for both calls; Swift window and double-click; `kindName`. `converter_state.h` (`ConverterRegister`, `RtcTime`), `circuit_converter_registers`, `circuit_rtc_time` (engine hooks `registers`, `rtc_time` on `CONVERTER_COMPONENT`; `docs/backend.md`); `circuit_check`: `registers: CONVERSION=0000 CONFIG=85A3 LO_THRESH=8000 HI_THRESH=7FFF` after the 6502 run, count with no room, `part 2 has no registers`, `no part 999`, `part 6 is not an RTC`, `the circuit is not running` before start, BQ32000DR `2000-01-01 00:00:00` Saturday → 2 s → `00:01:02`, SECONDS 02 / MINUTES 01 BCD — `0 failure(s)`. Swift: `Registers/RegisterWindow.swift` (`RegisterWindowTarget`, `RegisterSnapshot`, `RegisterInspector`), a `registers` WindowGroup, double-click on `hasRegisters` parts in `SchematicCanvasView` → `ContentView.onOpenRegisters`; `.converter`/`.rtc` details (they had fallen through to `.supply`), sidebar rows Kind/Bits/Channels; `kindName(part)` → `ADC`, `DAC` (harness `facts DAC8562SDGSR: kindName DAC; … Bits: 16; Channels: 2`), `RTC`.
- [x] `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p24-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (about 1 min). Observed `** BUILD SUCCEEDED **`.
- [x] Runtime proof (`run` skill), as §1. No project run skill; the plan-06-A harness (the app's Swift sources and backend, a custom `main.swift`, inside a copy of the Debug .app): the app's `SimulationController` runs §G's 6502/W65C22/ADS1115 board plus a BQ32000DR on 3.3 V; the app's `RegisterInspector` shows in an on-screen window, captured with `screencapture -l`: `U1 — ADS1115IDGSR`, CONFIG `85A3`, CONVERSION `0000`; `U2 — BQ32000DR` Time `2000-01-01 00:00:00 Saturday`, SECONDS `00` → 2 s of circuit time later `00:00:02`, SECONDS `02`; `error -` (no stop). PNGs looked at.

Acceptance: build, tests, runtime proof: `** BUILD SUCCEEDED **`; `100% tests passed, 0 tests failed out of 26`; the windows above.
Commit: —

## Validation Plan — plan-24 landing tier

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p24 && cmake --build /tmp/logic-p24 -j8 && ctest --test-dir /tmp/logic-p24` | `100% tests passed`, T0 + 2 |
| Goldens vs plan-24 start | `git diff <plan-24-C first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 |
| Part counts | `grep -c '\|converter\|' parts.txt`; `grep -c '\|rtc\|' parts.txt` | NADC + NDAC; NRTC |
| Datasheets | `ls datasheets/{adc,dac,rtc}/*.pdf \| wc -l` | A + B counts |
| App | `xcodebuild` above | `** BUILD SUCCEEDED **` |

Then archive plan-24-A…I to `planning/completed/`.

## Decisions

- A register window, not an analog display (owner: digital side only).

## Corrections

- **A refused state query stops the app's simulation**: `circuit_error` is
  sticky and `SimulationEngine.snapshotNow` reports it as the stop reason.
  The first harness run asked the ADS1115 for its clock and the run showed
  `error part 7 is not an RTC`. The window passes the part's kind down and
  asks for the clock only for `PART_RTC` (and registers only for
  `hasRegisters` parts); after the fix `error -`.
- **`RtcTime.hundredths`** was added so the window shows hundredths for
  the parts that count them, not whenever the counter is non-zero.
- **The runtime proof opens the window the double-click opens**
  (`RegisterInspector` for the chip) from the harness, rather than driving
  the mouse on a hand-built schematic; the double-click branch itself is
  the same `hasRegisters` test the harness prints (`hasRegisters true` for
  ADS1115IDGSR, DAC8562SDGSR, BQ32000DR). `ImageRenderer` drops the
  register table's `ScrollView` and a cached `NSHostingView` draws blank,
  so the PNGs are `screencapture -l` of the on-screen window.
- The circuit ran at about 0.13× real time with the CPU board, so the
  RTC's advance is shown over 2 s of circuit time.

## Summary

I makes the converters and clocks visible in the app and closes plan-24.
