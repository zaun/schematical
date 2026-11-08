# plan-22-B: The `arith` kind — models `compare` and `parity`

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-22-A

B adds the kind `arith` (`PART_ARITH`) with the comparator and parity
models, and `arith_test`. No part data, no golden change.

Outcome of B: `ctest` T0 + 1 (`arith_test`); goldens unchanged.

References: plan-22-A §3; `backend/src/library/kind.h`, `kinds.c`;
`backend/src/models/seq.h`; `latch.c` (multi-model pattern);
`backend/include/parts.h:25-27`; `backend/CMakeLists.txt:8-29`, `:116`.

## Prerequisites

plan-22-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-22-A complete | `ls planning/completed/plan-22-A-*.md` → exists | MET (2026-11-07: `planning/completed/plan-22-A-arith-data.md`) |

## 1. Goal

- `ctest --test-dir /tmp/logic-p22` → T0 + 1 of T0 + 1; `arith_test` all PASS; golden diff empty.

### Non-goals

- No change to existing kinds; public API gains only `PART_ARITH` (appended after any value another of plans 19–22 appended).

## 3. Design

`backend/src/models/arith.c` + `arith.h`: kind `"arith"`, device keys
`model`, `units`, `timing`, `output` + model keys; part keys `family`,
`vcc`, `inputMax`; sets `Part.logic`; `pin_open_drain` for open-drain
outputs. Multi-bit values use `SeqBits` (value + known mask); the X rule of
plan-22-A §3.3 is one helper, `arith_eval_unknown(fn, inputs, mask)`, used by
every model.

### 3.1 `compare`

| Key | Value |
|---|---|
| `"a"`, `"b"` | 4–8 pin names each, LSB first ('688: P, Q) |
| `"cascade"` | optional `{"gt","eq","lt"}` input pin names ('85, 4063, 4585: A>B, A=B, A<B in) |
| `"outputs"` | object, any of `"gt"`, `"eq"`, `"lt"` → `{"pin","active"}` |
| `"enable"` | optional control ('688 G̅, '518–'521): deasserted → every output inactive |
| `"latch"` | optional control ('885 per specs.md): holds the A input while asserted |
| `"signed"` | optional control pin ('885 L/A: two's complement or magnitude, per specs.md) |
| `"output"` | `"push-pull"` / `"open-drain"` ('518, '520 … open-collector variants) |

Rules: A > B → gt; A < B → lt; A = B → the cascade inputs decide, in the
'85 table form: eq-in high → eq; else gt-in/lt-in pass (the '85 rows with
several cascade inputs high are taken from its function table, specs.md
§ 74_85, and are each a test case). No cascade pins: A = B → eq.

### 3.2 `parity`

| Key | Value |
|---|---|
| `"inputs"` | 8–9 pin names |
| `"even"`, `"odd"` | output pin names (ΣE: high when an even number of inputs are high, ΣO the complement) |
| `"evenIn"`, `"oddIn"` | optional pins ('180: EVEN/ODD inputs per its function table) |
| `"bus"` | optional ('286/11286): `{"pin": PARITY I/O, "xmit": control}`: XMIT asserted → the part drives PARITY I/O with the generated parity (an output); deasserted → it reads PARITY I/O as the ninth input and `error` goes active on a mismatch |
| `"error"` | optional `{"pin","active"}` ('286 ERROR) |

## 4. Tests — `backend/tests/arith_test.c`

| Fixture | Cases |
|---|---|
| '85 | A>B, A<B for 6 value pairs; every A=B cascade row of its table |
| 4585 | cascade rows of the CD4585B table (its own: A<B, A=B, A>B in) |
| '682 | P>Q, P=Q active-low |
| '688 | equal/unequal; G̅ high → P=Q high |
| '521 open-drain | equal → 0 driven, unequal → Z; `part_pin_open_drain` → 1 |
| '885 | latch hold; signed vs magnitude ordering of 0x80 vs 0x01 |
| '280 | 9-bit patterns with 0–9 ones; X on one input → both outputs X |
| '180 | the 4 EVEN/ODD input rows |
| '286 | XMIT asserted: drives parity; deasserted: mismatch raises ERROR |

`library_check`: one failure case per validation error.

## Phases

### Phase 1 — Kind, `compare`, `parity`

- [x] `PART_ARITH`; `KINDS[]`; `arith.c`, `arith.h`; CMake lines; `arith_test.c` (§4) and its test entry; `library_check` cases. `PART_ARITH` appended after `PART_HEADER` (no other plan-19–22 value pending); test support: `listing.c` kind name/units/family, `TRACE_LAST_KIND` and `listing_check` arrays → `PART_ARITH`. `/tmp/logic-p22/arith_test` → 17 `PASS` lines, `arith_test: all passed`; `library_check` → 39 `ok` arith lines (2 valid devices + 37 rejections), 0 FAIL.
- [x] `docs/parts-format.md` § `arith` (intro, `compare`, `parity`); `docs/backend.md` kind list. § `arith` before § `counter`; `backend.md` models list (`arith.c + arith.h`) and test list (`arith_test`).

Acceptance: `ctest --test-dir /tmp/logic-p22` → T0 + 1 of T0 + 1; golden diff empty (seconds). Met: `100% tests passed, 0 tests failed out of 25` (T0 + 1 = 25); `git diff --stat backend/tests/golden` → empty.
Commit: e672958

## Validation Plan

B's gate: Phase 1's `ctest`. Landing tier at the end of E.

## Decisions

- One kind for every plan-22 model (plan-22-A §3.1).

## Corrections

- **Keys the datasheets needed beyond §3** (plan-22-A Corrections):
  `cascadeTable` rows are `{"when"?, "in", "out"}` with `in`/`out` as pin
  levels, and the rows of a relation must cover every cascade combination
  (the loader checks); `when` ("gt"/"eq"/"lt", default "eq") lets the
  CD4585B gate A>B out with its A>B cascade input. `bus` is `{"pin",
  "xmit", "drives"}` and `error` comes with `errorOn` ('286: XMIT active
  low, PARITY I/O = ΣE while transmitting, ERROR low on an even total).
  Device keys `output` and `openDrain` (per-pin open collector, the '181
  A = B) are shared by every model.
- **§3 `"output"` per model, not "'518, '520 … open-collector
  variants"**: only the '518 is open-collector (sn74als518 p.1); the '520
  and '521 are totem-pole. The §4 '521 open-drain case is the '518 case
  (equal → released, unequal → 0, `part_pin_open_drain` → 1).
- **The '180 case is moot**: no '180 part is added (plan-22-A
  Corrections), so `parity` has no `evenIn`/`oddIn` keys. The '682 row
  tests the '684 too, and the 4063B joins the 4-bit cascade tests.
- **`arith_eval_unknown` takes the input count** (found by `arith_test`:
  enumerating all 32 bit positions counted unused bits as unknown and made
  every compare output X).
- **Units**: `PartLogic.units` is 1 for every arith device (one function
  per package).

## Summary

B tests the comparator and parity rows before any part data uses them.
