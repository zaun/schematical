# plan-01-E: The `sram` and `7seg` kinds — library complete

Last updated: 2026-10-31
Effort: large (3h–1d)
Depends on: plan-01-D

Ports the asynchronous SRAM model (`sram.c` + `sram_parts.inc` +
`sram_catalog.inc`) and the LED 7-segment display model (`led7.c` +
`led7_parts.inc`) onto the part library. After this sub-plan every kind is
registered and the new library reproduces all 3,817 parts.

Outcome: `listing_check` and `trace_check` compare **all 3,817 lines** of
both goldens and pass.

References:

- `planning/plan-01-A-parts-baseline.md` §3 (design), §4 (trace/listing).
- `planning/plan-01-B-library-core.md` §3.3 (hooks), §3.4 (`lights.c`), §3.6 (exporter, `notes.py`).
- Old code: `backend/sram.c`, `sram_parts.inc`, `sram_catalog.inc`,
  `sram_check.c`, `led7.c`, `led7.h`, `led7_parts.inc`, `led7_check.c`.

## Prerequisites

See plan-01-A §Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-01-D complete | `ls planning/completed/plan-01-D-flipflop-switch.md` → exists | MET (2026-10-31: exists, 1dfa560) |

## 1. Goal

- `sram` and `7seg` are registered kinds; `listing_check` and `trace_check`
  compare 3,817 parts and pass with no golden line skipped.
- `tests/sram_test.c` and `tests/display7_test.c` carry the old model tests
  onto the new library.

### Non-goals

- Old files stay until plan-01-G. No behavior change.

## 2. Current State (read)

- SRAM pins: `SramPin {number, function, index}` with functions
  `A`, `DQ`, `CE_N`, `CE`, `OE_N`, `WE_N`, `BE_N` (lane index), `VCC`, `GND`,
  `NC` (`sram.c:21-32`). Timing `SramTiming` has 24 fields (read: `trc taa
  tace toe tba toh tclz tolz tblz tchz tohz tbhz`; write: `twc taw tcw tbw tas
  twp twp_oe_low twr tdw tdh twhz tow`), in ps via `NS()`; unset fields are 0.
- 37 `SRAM_DEFINE_CHIP(NAME, ID, SIZE, DESCRIPTION, ADDRESS_BITS, DATA_BITS,
  PINS, PARTS)`; layouts of one chip are separate definitions
  (`AS6C6264`, `AS6C6264_STSOP`). Parts point at a timing table named
  `<CHIP>_<grade>` (38 tables: grades 10, 12, 15, 20, 45, 55).
- Memory inspection: `sram_instance_size`, `sram_inspect` (`sram.h:59-72`).
- 7-segment: `led7_parts.inc` rows `X(pin, name, kind, digit, segments)` with
  kind `SEG` (digit 0 = shared), `COM` (digit or `LED7_EXTRAS` = colon/dot
  common), `NC`, `NP` (no lead → hidden group); `LED7_PART(NAME, part,
  description, digits, size, color, chip, common_anode, colon_after,
  dot_after, …)`; LED order from `led7_led_at`; side orders from
  `led7_side_pins`; glow from `led7_instance_glow` (`led7.c:420-435`).

### Measured populations

| What | Count | Command |
|---|---|---|
| SRAM definitions / parts / timing tables | 37 / 172 / 38 | `grep -c 'SRAM_DEFINE_CHIP(' sram_parts.inc`; plan-01-A §2; `grep -c '^static const SramTiming' sram_parts.inc` |
| SRAM timing tables with a comment above | 37 | plan-01-A §2 |
| 7-segment definitions / parts | 49 / 49 | plan-01-A §2 |
| 7-segment pinouts with a shared common name | 13 | plan-01-A §2 Verified properties |

## 3. Design

### 3.1 `sram` kind keys

```json
"address": ["A0", "A1", "A2"],
"data": ["DQ0", "DQ1"],
"chipEnable": ["CE#"],
"chipEnableHigh": ["CE2"],
"outputEnable": "OE#",
"writeEnable": "WE#",
"byteEnable": ["LB#", "UB#"],
"grades": {"55": {"trc": 55, "taa": 55, "tace": 55, "toe": 30, "note": "…"}}
```

- `address[i]` is address bit `i`; `data[i]` data bit `i`; `byteEnable[i]` is
  lane `i`. `chipEnableHigh` and `byteEnable` may be empty.
- `grades` entries take the 24 timing names (`twp_oe_low` → `twpOeLow`), in ns;
  a missing name is 0 (as the old designated initializers).
- Part key `grade` (a key of `grades`). `PartSram`: `words` = 2^len(address),
  `bits` = len(data), `access_ps` = the grade's `taa`, `grade`.
- Model `src/models/sram.c`: today's `sram.c` behavior; pins from the bound
  pinout; `memory_size`/`memory_read` hooks replace `sram_instance_size` /
  `sram_inspect`.

### 3.2 `7seg` kind keys

```json
"digits": 4, "commonAnode": false, "colonAfter": 2, "dotAfter": 0,
"color": "Red", "size": "0.4 in", "chip": "AlInGaP Super Red",
"leds": [{"digit": 1, "segment": "A"}],
"segmentPins": {"A/L1": {"digit": 0, "segments": ["A", "L1"]}},
"commonPins": {"CC1": 1, "CC L": "extras"},
"segmentSide": ["A/L1", "B/L2"],
"commonSide": ["CC1", "CC2", "CC L"]
```

- `leds` is the display order (`PartDisplay.leds`, the lights order); digit
  0 in `leds` = colon/dot (today's `LED7_EXTRAS` written as 0, matching the
  old catalog).
- `segmentPins` digit 0 = shared by all digits. `commonPins` value is a digit
  or `"extras"`. A common name shared by several pins means all of them.
- `segmentSide`/`commonSide` give drawing order by name; a shared name
  expands to its pins in number order (verified to equal today's order for
  all 13 such displays). `PartDisplay.segment_pins`/`common_pins` are the
  expanded pin numbers.
- `NP` pins: role `absent`; `NC`: role `nc`.
- Model `src/models/display7.c`: today's `led7.c` behavior with on-time
  tracking from `lights.c`; `lights` hook.

### 3.3 Exporters

- `tools/export/export_sram.c` (includes `sram.c`): device id = chip id
  without layout suffix; a layout definition merges into its chip as another
  pinout when its kind keys are identical by name, else stays its own device
  (id = old id); pinout key = old layout suffix lower-cased (`stsop`, `tsop2`,
  `bga`) or `p<pins>` for the base; function = old size label (`8 KB`);
  drawer `["Memory", "SRAM"]`; grade = timing table name after the last `_`.
  Notes: `@AS6C6264_55/` and the `---- … ----` device header.
- `tools/export/export_7seg.c` (includes `led7.c`): one device per part, id =
  part name, function `7-Segment`, drawer `["Indicators", "LED 7-Seg"]`,
  pinout `p<pins>`. Device note from the `/* LTC-4727JR: C4727JR.pdf */` line.

### 3.4 Tests

- `tests/sram_test.c`: port of `sram_check.c`'s bench (write/read cycles,
  timing windows, byte lanes, X on violations) onto the library: parts come
  from `sram.json`; pin names resolve through `Part.pins`. `check_catalog` and
  `check_size_labels` are covered by `listing_check`; `check_netlist_inspect`
  moves to `circuit_check` (plan-01-F).
- `tests/display7_test.c`: port of `led7_check.c:check_model` and
  `check_persistence` (glow persists within a step, clears after).
- `tests/library_check.c`: SRAM `grade` missing from `grades` rejected; a
  `7seg` common `"extras"` misspelled rejected; `segmentSide` naming a
  non-segment pin rejected.

### 3.5 Docs

`docs/parts-format.md`: `sram` and `7seg` sections.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` = partly done, say what remains. Moot tasks:
> `- [x] ~~text~~ — moot: <evidence>`. Fill `Commit:` when a phase lands.

### Phase 1 — SRAM

- [x] `src/models/sram.c` (§3.1); add to `KINDS[]`. The engine section
      (`/* ---- Storage` through `sram_timer`, 565 lines) is copied verbatim
      from the old `sram.c`; only the loader, `create` (pins from the bound
      pinout) and the two memory hooks are new.
- [x] `tools/export/export_sram.c`; generate `sram.json`; `notes.py`
      (`58 notes, 2 removed`, 0 placeholders). 21 devices: every layout
      merged except `AS7C34098A_BGA`, whose data pins are named
      `I/O0`–`I/O15` where the TSOP's are `I/O1`–`I/O16` (kind keys differ by
      name, so it stays its own device, id = old id).
- [x] `tests/sram_test.c` + CTest; `library_check` SRAM case; docs section.
      `sram_test` = the old bench (`check_as6c62256`, `check_as7c34098a`,
      verbatim apart from `sram_instance_size` → the `memory_size` hook) plus
      `check_memory_hooks` (size, clipping, X at power-on, past the end).

Acceptance: 172 SRAM parts match both goldens; the ported bench passes.
  Check: `ctest --test-dir /tmp/logic-build -R 'library_check|listing_check|trace_check|sram_test' --output-on-failure` → passed, `3768 parts compared` (est. 2 min).
  Result: `100% tests passed, 0 tests failed out of 4`; both parity checks
  `… sram 172 …`, `3768 parts compared, 49 skipped`; `sram_test` → 38 PASS, `0 failure(s)`.
Commit: 741697f

### Phase 2 — 7-segment displays; library complete

- [x] `src/models/display7.c` (§3.2); add to `KINDS[]`.
- [x] `tools/export/export_7seg.c`; generate `7seg.json`; `notes.py`
      (`49 notes, 0 removed`, 0 placeholders). The exporter checks that every
      side list collapses to names whose expansion is the old pin order.
- [x] `tests/display7_test.c` + CTest; `library_check` cases (+ a valid
      display file); docs section. `kind_check_keys` with both lists NULL
      now means "any key, duplicates only" (used for `segmentPins` /
      `commonPins`).
- [x] `listing_check`/`trace_check`: now that every kind is registered, fail
      if any golden line is skipped.
- [x] Move `planning/plan-01-E-sram-7seg.md` to `planning/completed/` once every phase's `Commit:` is filled.

Acceptance: all 3,817 parts match both goldens.
  Check: `ctest --test-dir /tmp/logic-build -R 'library_check|listing_check|trace_check|display7_test' --output-on-failure` → passed, `3817 parts compared, 0 skipped` (est. 2 min).
  Result: `100% tests passed, 0 tests failed out of 4`; both parity checks →
  `3817 parts compared, 0 skipped`; `display7_test: all passed`. Runtime proof:
  `trace_check --verbose LTC-4727JR` → 30 distinct light strings across the 30
  steps. Full `ctest` → `100% tests passed, 0 tests failed out of 13`.
Commit: this phase's commit is the one that moved this file to `planning/completed/` (hash in plan-01-F's Corrections).

## Validation Plan

- Tests: `sram_test`, `display7_test`, `library_check` cases; full parity.
- Coverage check: `0 skipped` in both parity checks.
- Runtime proof: `trace_check --verbose LTC-4727JR` shows light strings
  changing between 100 ns steps.
- Doc sync: `docs/parts-format.md` complete for all nine kinds.
- Final gate: `cmake --build /tmp/logic-build -j8 && ctest --test-dir /tmp/logic-build` → all pass (est. 3 min).

## Decisions

See plan-01-A §Decisions. Nothing new.

## Corrections

- plan-01-D's last commit (archive + Phase 2) is 1dfa560.
- §3.3 grade names: the compiled tables do not carry the timing tables'
  names, so `export_sram.c` includes `sram_parts.inc` a second time inside a
  function with `SRAM_DEFINE_CHIP` redefined to record each part's grade
  table name (`#grade`) and the chip's PINS macro name. The device note is
  `"<chip header> | <pinout>: <PINS comment> | …"` (the `---- … ----` header
  plus each pinout's comment line above `#define <NAME>_PINS(X)`).

## Summary

SRAM is the largest model (1,021 lines) and has the most timing data; it moves
almost unchanged, and its parity is pinned by both the traces and the ported
bench. 7-segment display data is the most irregular (shared commons, absent
pins, extras digit); the side-order expansion rule was verified against all 49
displays before writing this plan.
