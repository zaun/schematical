# plan-68-M: Backend coverage — converters and the RTC

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-L

The converter kind (ADCs, DACs, display drivers through `converter_load_display.c`/`converter_load_lcd.c`) and the RTC kind. Every line of these files ends up run by a test that checks what the
line did. 998 lines are uncovered in them today (4 files, 5959
instrumented lines).

Prerequisites: plan-68-A's table, and plan-68-L complete (its ratchet rows are gone: `grep -c . scripts/coverage-ratchet.tsv` is lower than before it, and its Commit lines are filled).

References:

- The files in § 2 and the test programs that exercise them (§ 2).
- `planning/plan-68-data/backend-missed-by-letter.tsv` (the per-file census
  this letter is written from: `llvm-cov report` of the instrumented `CT` at
  `e2f6811a`).
- `docs/backend.md` § Tests (each program); `docs/parts-format.md` (each
  kind's keys, for loader rejection cases).
- `AGENTS.md` § Datasheets (goldens) and § Source size and coverage.

## 1. Goal

- `node scripts/coverage.js --backend --only <file>` prints no uncovered line
  for any file in § 2.
- Their rows are gone from `scripts/coverage-ratchet.tsv`; `COV --backend
  --check` → exit 0.
- `CT` passes; `SZ` passes (no test program over 1000; new ones at or under
  800).

### Non-goals

- No change to simulated behavior except a bug a new test proves (§ 3,
  step 6), each listed under Corrections with its golden diff.
- No production file grows past 800 code lines; no new test helper is added
  to `logic_test_support` (benches stay per area).

## 2. Current State

Uncovered lines at `e2f6811a` (`planning/plan-68-data/backend-missed-by-letter.tsv`,
letter M). The numbers are from before plan-68-K, which covers the
out-of-memory lines; the letter starts by re-running `COV --backend --only`
on these files, and that list — not this table — is the work. A file that
plan-68-I or plan-68-J split appears here under its old name; its row stands
for all of its pieces.

| File | Uncovered | Lines |
|---|---|---|
| `src/models/converter_load.c` | 779 | 3522 |
| `src/models/converter.c` | 160 | 1570 |
| `src/models/rtc.c` | 47 | 579 |
| `src/models/converter_par.c` | 12 | 288 |
| **Total** | **998** | **5959** |

Where the tests go: Loader rejections in `library_check` programs or `converter_test`; model paths in `converter_test`, `rtc_test`, and the display-driver programs (`as1115_test`, `icm72*_test`, `max69*_test`, `pcf*_test`, `tlc6983_test`, `lcd_spi_i2c_test`, `cd22105_icm7224_test`).

## 3. Design

The method is plan-68-L § 3, unchanged: list, classify (loader rejection,
undriven model state, API misuse, unreachable), assert an observable result,
put the test in the program named in § 2, lower the ratchet row in the same
commit, fix any bug a test finds with that test as its regression.

### Where the risk is

A new case that pins today's behavior where today's behavior is wrong. The
assertion comes from the datasheet section the model cites, not from what the
model happens to do; where the two differ it is a bug (§ 3, step 6), never a
test written to match.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` with what remains; moot tasks are
> struck through with evidence, never deleted; fill `Commit:` when a phase
> lands. **An unticked box means NOT DONE.**

### Phase 1 — `converter_load.c`

- [x] Re-run `COV --backend --only` for each file; close every line per § 3.
  `LOGIC_COV_WORK=/tmp/logic-cov-M node scripts/coverage.js --backend --only backend/src/models/`
  → the loader pieces list only out-of-memory returns (a `kind_alloc` /
  `kind_strdup` NULL check, or `cvl_name_id` / `cvl_space_index` failing on
  one) and their closing braces: `converter_load.c` 24, `_conv.c` 20,
  `_display.c` 20, `_lcd.c` 4, `_proto.c` 24, `_rtc.c` 18 (110 lines, 55
  returns). Those are left to the parent's `kind_arena_chunk` sweep in
  oom_test (plan-68-K follow-up), not tested here.
- [x] Ratchet rows removed for each file at 0. Not edited by this letter (the
  parent owns `scripts/coverage-ratchet.tsv`): the rows of `converter.c`,
  `converter_model.c`, `converter_par.c`, `converter_proto.c` and `rtc.c` can
  go (0 uncovered); the six loader rows drop to the counts above until the
  out-of-memory sweep lands.
  Done by the parent (plan-68-L Phase 3): oom_test sweeps every kind's loader with
  `kind_arena_chunk` 16, so the `kind_alloc` NULL returns left here run;
  `LOGIC_COV_WORK=/tmp/p68-covB node scripts/coverage.js --backend` →
  `backend: 60195 lines, 0 uncovered, 100.00%` (no ctest failure); this
  letter's 11 coverage-ratchet rows removed.

Acceptance: these files have no uncovered line.
  Check: `node scripts/coverage.js --backend --only <file>` → no `path:line`
  output for each file in this phase; `CT` → all pass (est. 2 min per run).
Commit: —

### Phase 2 — `converter.c`, `rtc.c`, `converter_par.c`

- [x] As Phase 1, for these files. Same command → no `path:line` for
  `converter.c`, `converter_model.c`, `converter_par.c`, `converter_proto.c`,
  `rtc.c` (`backend: 53602 lines, 394 uncovered, 99.26%` over the tree);
  `ctest --test-dir /tmp/p68-build-M` → `100% tests passed, 0 tests failed
  out of 162`; `node scripts/source-size.js --check` → exit 0 (new test files
  168-391 code lines).

Acceptance: as Phase 1, for these files.
  Check: as Phase 1; `SZ` → exit 0 (est. 2 min per run).
Commit: —

## Validation Plan

- Tests: the cases added to the programs in § 2 (and any `_more_test`).
- Coverage check: `COV --backend --check` → exit 0 with every § 2 row gone.
- Final gate (once, after Phase 2): `CT`; `SZ`; `COV --backend --check`
  (est. 3 min).

## Decisions

- (owner, plan-68-A) 100% line coverage, no exclusion list.

## Corrections

- **Where the tests went.** `converter_test.c` (915 lines) and `rtc_test.c`
  have no room, so the cases are in new programs: `converter_load_cov_test`
  (converter loader rejections: `converter_load.c`, `_conv.c`, `_proto.c`),
  `converter_disp_load_cov_test` (display / lcd / clock / ccsi / rtc-clock
  rejections: `_display.c`, `_lcd.c`, `_rtc.c`), `converter_more_test` and
  `converter_more2_test` (model paths on fixture devices), `rtc_more_test`
  (rtc.c). The pin-by-pin bench they share is `tests/support/converter_bench.c`
  (a copy of converter_test's helpers plus pull-downs; converter_test keeps
  its own).
- **The file table was out of date.** plan-68-I/J split `converter_load.c`
  into `_conv`, `_display`, `_lcd`, `_proto`, `_rtc` and `converter.c` into
  `converter_model.c`, `converter_proto.c`: 979 uncovered lines in 11 files at
  the start of this letter (`COV --backend --only backend/src/models/`).
- **Bug: a part's `"resets": {"R": …}` failed with no message.** The loader
  returned -1 for a reset naming R (or CH) without setting an error, so
  load_parts failed silently. It now reports `"resets" names registers`
  (regression: converter_load_cov_test "part resets R").
- **Bug: NULL handling under out-of-memory in `cvl_name_id`.** A failed
  `kind_strdup` left a NULL name in the table (the next lookup's strcmp
  crashed); it now returns CV_NONE, and `"analog"` checks it like the other
  callers. (`rtc` clock.ram also checks its `kind_strdup` now.) Those NULL
  returns are left for the parent's `kind_arena_chunk` sweep.
- **Loader tightened.** An rtc clock field (time fields, format, stop, uip,
  inhibit, latch, write, alarm / periodic / watchdog fields) naming R or CH
  is now rejected (`"…" names a register or a field, not R or CH`): R and CH
  hold no time, and rtc.c's `put` guard for them was the only consumer. A
  `periodUs` / `frameUs` too short to be 1 ps (per backplane for lcd) is now
  rejected instead of silently becoming 1 ps (`… is shorter than 1 ps`).
  No catalog part changes (listing_check / trace_check pass unchanged).
- **Unreachable code removed (callers read):**
  - `cv_reg_read` (converter.c): no caller anywhere in src/.
  - `cv_par_timer` and `CV_TM_PAR`: nothing schedules a CV_TM_PAR timer
    (timers come from converter.c, rtc.c, display_out.c, display_clock.c);
    cv_timer's last branch is now display's.
  - `cvc_field_put`'s and rtc.c `put`'s `reg < 0` guard: every caller's field
    is loader-checked to a real register (levels, busy, done, onRead, a
    command's set; rtc fields after the change above).
  - `cvc_run_action`'s `default:` (every CvAction has a case; the switch is
    on `(CvAction)action`, so -Wswitch checks it) and `control_pin`'s
    `default:` (i < CVP_COUNT, all six handled).
  - `cv_free_state(NULL)`: cv_create frees only a state it allocated and the
    engine destroys only states create returned (sim.c sim_add_instance).
  - `cv_ready_level`'s "no ready entry" Z: both callers pass a ready entry's
    pin (cv_ready_refresh, cv_spi_refresh's on-MISO ready).
  - `tok_value`'s `default:` LOGIC_0 (output patterns have no x; cv_seg_bit
    handles @) folded into CVT_REF; `cvc_i2c_next`'s X branch (cv_seg_bit
    never returns X).
  - The start-time "CS already active → select" (converter_model.c) and
    "read already active → drive" (converter_par.c): sim_start resolves every
    net to Z before any start function and applies no drive until the run, so
    CS / RD always read inactive at start.
  - `cvl_load_display`'s `d == NULL` (cv_load_device rejects a display driver
    without "display" first), `load_lcd_load`'s `!mode_dynamic` (a fixed mode
    with "load" is rejected before), `dl_decode`'s `digits == 0`
    "decodeField goes in each group" (static's key list has no top-level
    decodeField).
  - rtc.c `drive_pin`'s CV_NONE (ADD skips it), the `r == NULL` guards of
    rtc_refresh, rtc_timer, rtc_after_write, rtc_pin (all run after
    rtc_start: an rtc has no "conversion", so cv_start's reset refreshes
    nothing) and rtc_snapshot (its one caller, circuit_rtc_time, asks only an
    rtc part of a started circuit).
  - The empty "leaving continuous mode" `if` in after_write (no statement).
  - Two bare `{ … }` blocks in cv_load_part's part `"i2c"` (flattened: clang
    mapped the lines after them to count 0 although they ran).
- **Simplified:** blink `halfPeriodUs` as a number no longer allocates a
  word to hold the value it copies straight back out.
- **My expectations corrected, not the model:** a `wordBits` read word holds
  the register in its top bits and zeros below (docs/parts-format.md
  `"wordBits"`), so an 8-bit R1 = 0x3C reads 0x3C00 in 16 bits; a strap tied
  high is tie 1, so table[1].
- **Golden lines:** none change (listing_check and trace_check pass).
- **(parent) Two more loader fixes in this letter's files.** plan-68-L
  Phase 3's chunked sweep crashed in `cvl_space_index`: a failed
  `kind_strdup` was stored and counted, and the next lookup `strcmp`ed it.
  Both allocations are now checked before either is stored. And the
  "`lcdload` needs an lcd display with a load section" check ran only for a
  device with a display, so a converter's `lcdload` command loaded and only
  `disp_lcd_byte`'s NULL guard stood between it and a NULL `disp`
  (plan-68-R's Corrections). The check now runs for every device, and the
  guard is gone (plan-68-R). Regression: converter_load_cov_test "lcdload
  without a display".
- **(parent) One commit for both phases.** The letter's two phases landed
  together, so both Commit lines name the same commit.

## Summary

Test writing, file by file. The risk is tests that copy today's behavior
instead of the datasheet's; a disagreement is a bug to fix, not a number to
record.
