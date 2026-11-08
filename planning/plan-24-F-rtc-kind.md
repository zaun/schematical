# plan-24-F: The `rtc` kind

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-24-E

F adds the part kind `rtc` (`PART_RTC`): timekeeping from simulated time,
calendar registers, alarms, periodic interrupts, watchdog, battery-backed
RAM where present, on I2C (BQ32000, BQ32002) and parallel buses (BQ3285,
BQ4802, BQ4845, DP8570, DP8573), with `rtc_test`.

Outcome of F: `ctest` T0 + 2 (`rtc_test`); goldens unchanged.

References: plan-24-A §3.4, plan-24-B §2 (RTC `specs.md` sections);
`serial_target.h` (I2C front end); plan-24-E §2 (parallel bus handling,
reused); `backend/src/models/eeprom.c` / `sram.c` (byte arrays).

## Prerequisites

plan-24-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-24-E complete | `ls planning/completed/plan-24-E-*.md` → exists | MET (`planning/completed/plan-24-E-parallel-converters.md`, 2026-11-07 at 4a9a48b) |

## 1. Goal

- `ctest --test-dir /tmp/logic-p24` → T0 + 2 of T0 + 2; `rtc_test` all PASS; golden diff empty.

### Non-goals

- No wall-clock time: an RTC counts simulated time, so traces and goldens
  stay deterministic. No battery switchover (no supply levels in the engine);
  RAM and time are kept while the circuit runs and start fresh at load.

## 2. Design

`backend/src/models/rtc.c`: kind `"rtc"`, device keys `bus` (the I2C object
or plan-24-E's parallel object), `registers` (plan-24-D schema plus a
`"time"` field role per register: seconds, minutes, hours, day, date, month,
year, century, with `"format": "bcd"\|"binary"` and the 12/24-hour bit),
`alarms` (match registers and mask bits), `periodic` (rate field → period
table, interrupt pin), `watchdog`, `ram` (byte count and address range),
`stop` (oscillator stop/start bit), `update` (the datasheet's read-latch /
update-inhibit mechanism: `"uip"` (BQ3285 UIP flag), `"latch"` (BQ4802
read/write bits halt updates), `"none"`). Power-on time: 2000-01-01
00:00:00, oscillator running unless the datasheet's power-on state says the
stop bit is set (then stopped). The model ticks once per simulated second
(`sim_schedule`) and on each tick updates the counters with carries, leap
years per the datasheet's year range, alarm matches and interrupts.

## 3. Tests — `backend/tests/rtc_test.c`

| Fixture | Cases |
|---|---|
| BQ32000-shape I2C | set time, run 2 simulated seconds, read seconds +2; 59 → 00 carry into minutes; Feb 28 → 29 in 2024, → Mar 1 in 2023; oscillator stop bit halts counting |
| BQ3285-shape parallel | UIP flag set around the update; 12-hour mode with PM bit; periodic interrupt at the selected rate; alarm interrupt on match |
| BQ4802-shape | read latch freezes the read registers while counting continues |
| DP8570-shape | RAM write/read; timer interrupt |

## Phases

> **Status discipline:** as plan-24-A.

### Phase 1 — Kind and timekeeping

- [x] `PART_RTC`; `KINDS[]` (`RTC_KIND`); `rtc.h`, `rtc.c` (+ CMake), the clock and RAM loader in `converter_load.c`; `rtc_test.c` timekeeping cases (power-on 2000-01-01 Saturday; +2 s; 59 → 00 carries into minutes, hours and the day; 2024-02-28 → 02-29, 2023-02-28 → 03-01; the stop bit halts and resumes); test entry; docs § `rtc` (`docs/parts-format.md`); listing kind `rtc`. `ctest -R rtc_test` → `rtc_test: all passed`.

Acceptance: `ctest -R rtc_test` passes (seconds): `rtc_test: all passed`.
Commit: —

### Phase 2 — Alarms, periodic, watchdog, RAM, update mechanisms

- [x] The rest of §2 (alarms with masks and don't-care bytes, periodic interrupts in both forms, the watchdog with kick edges, battery-backed RAM, UIP/inhibit and latch update mechanisms, 12/24-hour, BCD/binary by a field, the multiplexed-bus ALE latch); remaining §3 cases: BQ3285-shape UIP set 244 µs before the update and clear after, 12-hour 11:59:59 PM → 12 AM and AM → 12 PM, periodic INT after 500 ms and released by reading REGC, alarm on a seconds match with don't-care minutes/hours, RAM 14 and 127; BQ4802-shape UTI freezes reads while counting continues and a write under UTI loads at release; watchdog (a case added: WDI edges, WDO); DP8570-shape RAM and the 1 ms timer interrupt, hundredths — all `ok`; `library_check`: no clock, unknown unit, no hour field, alarm on an unknown register — each `ok`.

Acceptance: `ctest --test-dir /tmp/logic-p24` → T0 + 2 of T0 + 2 (`100% tests passed, 0 tests failed out of 26`); golden diff empty (0 lines) (seconds). The 7 RTC specs load (`loaded 14 parts`).
Commit: —

## Validation Plan

F's gate: Phase 2. Landing tier at the end of I.

## Decisions

- Simulated time, start 2000-01-01 00:00:00 (author: deterministic goldens).

## Corrections

- **Both phases landed in one commit**: the timekeeping, alarm, periodic,
  watchdog and latch paths share the tick and register-write hooks, so they
  were written and tested together; both Commit lines name it.
- **The clock form grew** with the RTC specs (plan-24-B Corrections):
  `h12Level`, `stop.running`, alarm `maskLevel`/`dontCare`,
  `update.latch.writeHalts`, the periodic array form, the watchdog object
  with kick and reset pins, and the parallel bus's `ale` latch (BQ3285).
- **Engine rules found while testing**: a register counts as read when its
  bits are shifted out, and clear-on-read bits clear when the read ends
  (they had been cleared before the bits went out, or not at all for
  pattern reads); a read address the map lacks reads the register at that
  address, and an address lookup in the default space falls back to the
  other spaces (BQ3285 RAM shares the register map).
- The rtc kind runs on the converter's engine model (`CONVERTER_COMPONENT`)
  with hooks into `rtc.c`, not a second model.

## Summary

F covers the 17 addable RTC parts with one parametric clock model.
