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
| plan-24-H complete | `ls planning/completed/plan-24-H-*.md` → exists | NOT MET (2026-11-07) |

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

- [ ] Backend API + `circuit_check` cases for both calls; Swift window and double-click; `kindName`.
- [ ] `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p24-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (about 1 min).
- [ ] Runtime proof (`run` skill), as §1.

Acceptance: build, tests, runtime proof.
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

## Summary

I makes the converters and clocks visible in the app and closes plan-24.
