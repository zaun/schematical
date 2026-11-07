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
| plan-24-E complete | `ls planning/completed/plan-24-E-*.md` → exists | NOT MET (2026-11-07) |

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

- [ ] `PART_RTC`; `KINDS[]`; `rtc.c` (+ CMake); `rtc_test.c` timekeeping cases; test entry; docs § `rtc`.

Acceptance: `ctest -R rtc_test` passes (seconds).
Commit: —

### Phase 2 — Alarms, periodic, watchdog, RAM, update mechanisms

- [ ] The rest of §2; remaining §3 cases; `library_check` cases.

Acceptance: `ctest --test-dir /tmp/logic-p24` → T0 + 2 of T0 + 2; golden diff empty (seconds).
Commit: —

## Validation Plan

F's gate: Phase 2. Landing tier at the end of I.

## Decisions

- Simulated time, start 2000-01-01 00:00:00 (author: deterministic goldens).

## Corrections

## Summary

F covers the 17 addable RTC parts with one parametric clock model.
