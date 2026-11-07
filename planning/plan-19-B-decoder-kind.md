# plan-19-B: The `decoder` kind — models `binary`, `priority`, `seg7`

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-19-A

B adds the part kind `decoder` with three models and a model test,
`decoder_test`, that runs one case per function-table row of every plan-19
device shape. No part data yet (C), so no golden changes.

Outcome of B: `ctest` passes T0 + 1 (`decoder_test`); goldens unchanged.

References: plan-19-A (§3 design, data conventions, signal rules);
`backend/src/library/kind.h` (`PartKindOps`), `kinds.c` (`KINDS[]`),
`backend/src/models/seq.h` / `seq.c` (shared helpers), `latch.c` (a
multi-model kind to mirror), `docs/parts-format.md` § Sequential kinds,
`docs/backend.md` § Adding a kind.

## Prerequisites

plan-19-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-19-A complete | `ls planning/completed/plan-19-A-*.md` → exists | NOT MET (2026-11-07) |

## 1. Goal

- `ctest --test-dir /tmp/logic-p19` → `100% tests passed, 0 tests failed out of <T0+1>`.
- `decoder_test` prints one `PASS` line per case of §5 and no `FAIL`.
- `git diff --stat backend/tests/golden` → no change.

### Non-goals

- No part data, no app change, no change to any existing kind.
- The public C API in `backend/include/` gains only the enum value
  `PART_DECODER` (appended after the last value of the `PartKind` enum in
  `parts.h`; if another of plans 19–22 appended its own kind first, append
  after it).

## 2. Current State

- `PartKind` enum: `backend/include/parts.h:25-27` ends `… PART_CONNECTOR,
  PART_PIO`.
- `KINDS[]` in `backend/src/library/kinds.c` lists one `PartKindOps` per
  kind; `kind_find(name)` resolves the JSON `"kind"` string.
- `PartKindOps` (`kind.h:28-42`) has `load_device`, part keys, bind, the
  `SimComponentType`, and the optional `pin_open_drain(part, pin)`.
- `seq.h` provides `SeqTimingTable` (family entries with `vcc`,
  `PartLogic`), `SeqControlName`/`SeqControl` (`{"pin", "active"}`), level
  reading with plan-10 §3.5 rules, `SeqOutput` change-suppressed inertial
  drive with `SEQ_PUSH_PULL`/`SEQ_OPEN_DRAIN` and Z.
- Every source file is listed explicitly in `backend/CMakeLists.txt:8-29`
  (`add_library(logic …)`); tests are added at `backend/CMakeLists.txt:116`
  (`add_test(NAME ${NAME} …)` over a list of test names).

## 3. Design

- `backend/src/models/decoder.c` (+ `decoder.h` for the model table):
  `PartKindOps decoder_kind` with kind name `"decoder"`, device keys
  `model`, `units`, `timing`, `output`, plus the model's keys; part keys
  `family`, `vcc`, `inputMax` (gate precedent). Sets `Part.logic` (family,
  units) so the drawer's family/unit filters apply.
- One `SimComponentType`; `evaluate` recomputes every unit on any pin change
  and schedules outputs through `SeqOutput`.
- `pin_open_drain`: 1 for every output pin of a device with
  `"output": "open-drain"`, and for the `seg7` `rbo` pin.

## 4. Models

### 4.1 `binary`

Main path address → output. `units` = the number of unit objects.

| Key | Value |
|---|---|
| `"units"` | 1–4 objects: `address` (1–4 pin names, LSB first; units may share address pins: '155/'156/'BCT2414), `outputs` (1–16 pin names; `outputs[i]` is selected by code i; fewer than 2^n outputs = codes ≥ count select nothing: '42, '145, '445, 4028, '1G29 code 3), `enables` (0–3 controls `{"pin", "active"}`; the unit is enabled when every one is asserted; shared enables are repeated per unit), `latch` (optional pin name: a transparent latch on the address, polarity per `latchActive`), `clock` (optional pin, rising edge registers the address: '131) |
| `"outputsActive"` | `"high"` or `"low"`: the level of the selected output |
| `"unselected"` | `"inactive"` (default) or `"z"` (1G18: the unselected output is high-impedance) |
| `"disabled"` | `"inactive"` (default) or `"z"` (3-state decoders whose enable floats the outputs) |
| `"latchActive"` | `"high"` / `"low"`: the `latch` level that **holds** the address ('137/'237 LE (G̅L) holds while high; 4514/4515 STROBE passes while high, holds while low → `"low"`) |
| `"output"` | `"push-pull"` (default) / `"open-drain"` ('145, '45, '156, '159, '445) |
| `"monitor"` | optional pin name (BCT2414 VS): driven 1 (supply good) |

Rules: enabled and code = i → output i active; every other output inactive
(or Z per `unselected`); not enabled → all inactive (or Z per `disabled`);
an X enable → X on every output of the unit; an X address bit → X on exactly
the outputs that differ across its completions. The latch/register holds 0
at power-on. The `monitor` pin is driven 1 from start: the engine runs every
part at its supply's nominal level and has no VCC-fail event (Decisions).

### 4.2 `priority`

Main path input → output.

| Key | Value |
|---|---|
| `"inputs"` | 4–10 pin names; `inputs[i]` has priority i (highest index wins) |
| `"inputsActive"` | `"high"` / `"low"` |
| `"code"` | 2–4 pin names, LSB first: the binary code of the winning input |
| `"codeActive"` | `"high"` / `"low"` (the '148 outputs are active low: A = NOT index) |
| `"offset"` | integer added to the index before it is output ('147: inputs 1–9 → `offset` 1); default 0 |
| `"idle"` | the code value output when no input is active (before inversion); default 0 |
| `"enableIn"` | optional control (EI) |
| `"groupSelect"`, `"enableOut"` | optional output pin names with `"active"` (GS: enabled and some input active; EO: enabled and no input active) |
| `"disabled"` | `"inactive"` (default: code = idle, GS/EO inactive) or `"z"` ('348: code outputs 3-state when disabled or idle; GS/EO stay driven) |

X rules: an X input above every active input makes X exactly the code bits
that differ between the candidates; an X `enableIn` makes every output X.

### 4.3 `seg7`

Main path BCD input → segment.

| Key | Value |
|---|---|
| `"inputs"` | 4 pin names, LSB first |
| `"segments"` | 7 pin names a…g |
| `"segmentsActive"` | `"high"` / `"low"` (7447A: low = segment on, open-drain) |
| `"glyphs"` | 16 strings of 7 `0`/`1` (a…g on) for codes 0–15, from the datasheet's display figure (the `bcd7seg` key, same meaning) |
| `"lampTest"` | optional control: every segment on |
| `"blank"` | optional control (the '49 BI input, CD4056 has none) |
| `"rbi"`, `"rbo"` | optional pins (7447A): RBI low and code 0 and LT inactive → all segments off and RBO pulled low; RBO is `PIN_INOUT`, open-drain: driven low as above, else released; read as an input, low blanks every segment |
| `"latch"` | optional control: the code is held while asserted (CD4056 strobe) |
| `"phase"` | optional pin (CD4056 DF): every segment output is XORed with it (liquid-crystal drive) |
| `"output"` | `"push-pull"` / `"open-drain"` |

Priority: lamp test > blank (BI or RBO read low) > ripple blank > glyph.

### 4.4 Shared

`timing` entries per family `{rise, fall, vcc, inputMax, note}` via
`SeqTimingTable`; one main path per model as above. Device validation
errors use the `kind_error` style of `latch.c` (`"units[0]: \"address\" has
1 to 4 pins"`), each with a `library_check` failure case.

## 5. Tests — `backend/tests/decoder_test.c`

Fixture devices are inline JSON strings (the `latch_test.c` pattern), one
per shape; each case sets inputs, runs the engine past the delay and checks
every output.

| Fixture (shape of) | Cases |
|---|---|
| '138 (3 address, G1 high + G2A/G2B low, active-low out) | all 8 codes enabled; each enable deasserted alone; X on G1; X on address bit 1 with codes 0/2 (only Y0, Y2 X) |
| '139 (2 units, 2 address, one low enable each) | each unit independent; shared nothing |
| '42 (4 address, 10 outputs, no enable) | codes 0–9; codes 10–15 → all high |
| '137 (address latch, `latchActive` high) | transparent while LE low; held while LE high across an address change |
| '131 (clock) | address registered on rising edge only |
| 4514 (strobe, active-high out) | strobe high passes, low holds; INH disables |
| 1G18 (`unselected` z) | selected output follows D; the other is Z |
| '145 open-drain | selected output 0, others Z; `part_pin_open_drain` → 1 |
| BCT2414 | dual decode with SD and G; VS = 1 |
| '148 (8 active-low inputs, EI, GS, EO) | every row of the '148 function table (9 rows) |
| '348 (`disabled` z) | idle → code outputs Z, GS/EO driven |
| '147 (`offset` 1, `idle` 0, active-low) | every row of the '147 table |
| 7447A | codes 0–15 glyphs; LT; BI low; RBI with code 0 → RBO low; RBI with code 5 → RBO released; RBO driven low externally blanks |
| CD4056 | latch hold; DF high inverts every segment |

`library_check`: one failure case per validation error of §4.

## Phases

> **Status discipline:** as plan-19-A § Phases.

### Phase 1 — Kind skeleton and `binary`

- [ ] `parts.h` `PART_DECODER`; `kinds.c` `KINDS[]` entry; `decoder.c` with the kind ops and `binary`; `CMakeLists.txt` source line.
- [ ] `decoder_test.c` with the `binary` fixtures of §5; added to the test list in `backend/CMakeLists.txt`.
- [ ] `docs/parts-format.md` § `decoder` (kind intro, model `binary` key table); `docs/backend.md` kind list.

Acceptance: `ctest --test-dir /tmp/logic-p19 -R 'decoder_test|library_check'` → both pass; `decoder_test` prints the `binary` PASS lines (seconds).
Commit: —

### Phase 2 — `priority` and `seg7`

- [ ] `priority` and `seg7` in `decoder.c`; their §5 fixtures; docs key tables.
- [ ] `library_check` failure cases for every validation error.

Acceptance: `ctest --test-dir /tmp/logic-p19` → T0 + 1 of T0 + 1 passed; `git diff --stat backend/tests/golden` → empty (seconds).
Commit: —

## Validation Plan

B's gate is Phase 2's `ctest` (every model test and the golden comparisons,
about 13 s). Landing tier at the end of D.

## Decisions

- **VS/VCC-fail of the BCT2414 is not modeled** (author): the engine has no
  supply-fail event; VS reads 1. The decode function is complete.
- **One kind, three models** (plan-19-A §3.2).

## Corrections

## Summary

B delivers the decoder machine and proves every function-table row before
any of the 505 candidate parts uses it.
