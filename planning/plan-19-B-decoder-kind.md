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
| plan-19-A's data and families landed (corrected from "plan-19-A complete": A's footprints can only land with C's parts) | `git log --oneline \| grep -c 'plan-19-A phase'` → 2; `grep -c '^## ' planning/plan-19-data/specs.md` → 32 | MET (2026-11-07: `2`, `32`; commits 6ff7393, a9a16bf) |

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
| `"units"` | 1–4 objects: `address` (1–4 pin names, LSB first; units may share address pins: '155/'156 — corrected: the BCT2414 units have separate address pins and share only G and SD), `outputs` (1–16 pin names; `outputs[i]` is selected by code i; fewer than 2^n outputs = codes ≥ count select nothing: '42, '145, '445, 4028 — corrected: '1G29 code 3 selects Y2, see Corrections), `enables` (0–3 controls `{"pin", "active"}`; the unit is enabled when every one is asserted; shared enables are repeated per unit), `latch` (optional pin name: a transparent latch on the address, polarity per `latchActive`), `clock` (optional pin, rising edge registers the address: '131) |
| `"outputsActive"` | `"high"` or `"low"`: the level of the selected output |
| `"unselected"` | `"inactive"` (default) or `"z"` (1G18: the unselected output is high-impedance) |
| `"disabled"` | `"inactive"` (default) or `"z"` (3-state decoders whose enable floats the outputs) |
| `"latchActive"` | `"high"` / `"low"`: the `latch` level that **holds** the address ('137/'237 LE (G̅L) holds while high; 4514/4515 STROBE passes while high, holds while low → `"low"`) |
| `"output"` | `"push-pull"` (default) / `"open-drain"` ('145, '45, '156, '159, '445) |
| `"monitor"` | optional pin name (BCT2414 VS): open collector, driven low (supply good) — corrected from "driven 1" |

Rules: enabled and code = i → output i active; every other output inactive
(or Z per `unselected`); not enabled → all inactive (or Z per `disabled`);
an X enable → X on every output of the unit; an X address bit → X on exactly
the outputs that differ across its completions. The latch/register holds 0
at power-on. The `monitor` pin is driven low (open drain) from start: the
engine runs every part at its supply's nominal level and has no VCC-fail
event (Decisions). With `"unselected": "z"`, an output that is unselected
for every completion of the address stays Z under an X enable (1G18).

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

Priority (corrected): blank (BI or RBO read low) > lamp test > ripple blank > glyph — the SN7447A's BI wins over LT (sn7447a p.3 notes 2 and 4).

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
| BCT2414 | dual decode with SD and G; VS = 0, open drain (corrected) |
| '148 (8 active-low inputs, EI, GS, EO) | every row of the '148 function table (10 rows: disabled, idle and the 8 inputs) |
| '348 (`disabled` z) | idle → code outputs Z, GS/EO driven |
| '147 (`offset` 1, `idle` 0, active-low) | every row of the '147 table |
| 7447A | codes 0–15 glyphs; LT; BI low; RBI with code 0 → RBO low; RBI with code 5 → RBO released; RBO driven low externally blanks |
| CD4056 | latch hold; DF high inverts every segment |

`library_check`: one failure case per validation error of §4.

## Phases

> **Status discipline:** as plan-19-A § Phases.

### Phase 1 — Kind skeleton and `binary`

- [x] `parts.h` `PART_DECODER`; `kinds.c` `KINDS[]` entry; `decoder.c` with the kind ops and `binary`; `CMakeLists.txt` source line. Also the three test-support lines docs/backend.md § Adding a kind asks for (`listing.c` kind name and `units=`, `TRACE_LAST_KIND`, `listing_check` arrays → `PART_DECODER`), and `part_pin_open_drain` / the voltage check now accept an inout pin (the 7447A BI/RBO; no existing kind answers 1 for an inout pin: the only inout pins named IRQ/RDY/INT are the CPU's RDY, and `cpu.c` has no `pin_open_drain`).
- [x] `decoder_test.c` with the `binary` fixtures of §5; added to the test list in `backend/CMakeLists.txt`. Fixtures generated from the spec JSON (specs.md); cases 1–15 PASS ('138 ×4, '139, '42, 4028B, '137 ×2, '131, 4514B, 1G18, 1G29, '145, BCT2414).
- [x] `docs/parts-format.md` § `decoder` (kind intro, model `binary` key table); `docs/backend.md` kind list. Also the file table (`decoder.json`) and the `decoder_test` entry in docs/backend.md § Tests.

Acceptance: `ctest --test-dir /tmp/logic-p19 -R 'decoder_test|library_check'` → both pass; `decoder_test` prints the `binary` PASS lines (seconds). Met: both pass; PASS 1–15.
Commit: — (one commit with Phase 2; Corrections)

### Phase 2 — `priority` and `seg7`

- [x] `priority` and `seg7` in `decoder.c`; their §5 fixtures; docs key tables. `decoder_test` cases 16–25 PASS ('148 10 rows, '348, '147 10 rows, 40147B, 4532B with X input and X EI, 7447A ×4, CD4056B shape); `decoder_test: all passed` (25 PASS).
- [x] `library_check` failure cases for every validation error. 25 `expect_fail` cases + one valid 2-unit device (`ok   valid decoder (binary, 2 units)` … `ok   decoder seg7 rbo not inout`); `library_check: all passed`.

Acceptance: `ctest --test-dir /tmp/logic-p19` → T0 + 1 of T0 + 1 passed; `git diff --stat backend/tests/golden` → empty (seconds). Met: `100% tests passed, 0 tests failed out of 22`; golden diff empty.
Commit: —

## Validation Plan

B's gate is Phase 2's `ctest` (every model test and the golden comparisons,
about 13 s). Landing tier at the end of D.

## Decisions

- **VS/VCC-fail of the BCT2414 is not modeled** (author): the engine has no
  supply-fail event; VS stays low (supply good; corrected from "reads 1").
  The decode function is complete.
- **One kind, three models** (plan-19-A §3.2).

## Corrections

- **Prerequisite row** corrected to "A's data and families landed": A's new
  footprints fail `footprint_check` until C adds the parts that use them
  (plan-19-A Corrections), so "plan-19-A complete" cannot precede B.
- **SN74BCT2414 VS** (§4.1): open collector, low while VCC is good
  (sn74bct2414 p.1 "forces … VS … to the high level" when VCC < 3.65 V;
  p.5 VOL(VS) at IOL = 20 mA; p.3 output transistor). The model drives the
  `monitor` pin 0 through an open drain and `pin_open_drain` answers 1 for
  it (decoder_test case 15).
- **SN74BCT2414 units** do not share address pins (1A/1B vs 2A/2B); they
  share G and SD (specs.md § 74_2414).
- **'1G29** (§4.1): the function table has no unused code: A1 = L selects
  Y0 whatever A0 is, A1 = H selects Y1/Y2 by A0 (sn74lvc1g29 p.2). Expressed
  as two units sharing G, the second enabled by A1 high (specs.md
  § 74_1G29; decoder_test case 13).
- **seg7 priority** (§4.3): blank beats lamp test on the SN7447A
  (sn7447a p.3 notes 2 and 4); the model and docs use blank > lamp test >
  ripple blank > glyph (decoder_test case 22).
- **X enable with `"unselected": "z"`**: an output Z for every address
  completion stays Z under an X enable (the 1G18's unselected output does
  not depend on A); every other output gated by the X enable is X, as §4.1
  says (decoder_test cases 3 and 12).
- **One commit for both phases**: `decoder.c` was written with all three
  models at once, so Phases 1 and 2 landed together; both Commit lines carry
  the same hash.
- **'148 function table has 10 rows** (§5 said 9): disabled, idle, and one
  per input (sn74ls148 function table).

## Summary

B delivers the decoder machine and proves every function-table row before
any of the 505 candidate parts uses it.
