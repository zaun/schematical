# plan-37-C: Backend — the `register` kind, five logic families, `register_test`, docs

Last updated: 2026-11-12
Effort: large (3h–1d)
Depends on: plan-37-B (letter order; C reads no census or footprint data)

C adds the `register` part kind: `PART_REGISTER`, `backend/src/models/register.c`
with one model, `parallel` (multi-bit edge-triggered D registers: groups of
bits sharing a clock, an optional clock enable, an optional clear and one or
two output banks, each optionally inverted and 3-state), and an empty
`Parts/register.json`. It adds the logic families the D–F parts need and
the backend does not know: **LCX**, **LVX**, **VHC**, **VHCT** (onsemi) and
**ALVT** (Nexperia). It also makes the four app lines that list kinds know
`register`, so the Swift suite stays green once `register.json` exists.

The single behavioral outcome: a fixture `register` device copying the
SN74LS173A (`sn74ls173a.pdf` function table) loads, and `register_test`
drives it through every function-table row: CLR = H → Q = L at once; CLK ↑
with G1 = G2 = L → Q = D; G1 = H or G2 = H → hold; M = H or N = H → Z;
outputs back after M = N = L. A fixture part of family `VHC` loads (the
family is known). `ctest` passes T + 1 programs (T = the count when C
starts; T0 = 32), and `parts.txt`, `traces.txt`, `footprints.txt` are
byte-identical.

References:

- `docs/backend.md` § Adding a kind; `docs/parts-format.md` § Sequential
  kinds (shared device/part format, controls, signal rules), § `latch`
  (the multi-model precedent), § `buffer` (enables as one control or an
  array of 0–4), § Values (the family list at :203).
- `backend/include/parts.h` (`PartKind` :24–30, last value `PART_CAPACITOR`
  at cf40403; `PartLogic` :52; `Part.logic` :198).
- `backend/src/library/kinds.c` (`KINDS[]` :35–64), `kind_util.c`
  (`LOGIC_FAMILIES[]` :435–439, `kind_logic_family`).
- `backend/src/models/seq.h` / `seq.c` (controls, timing tables, `SeqBits`,
  clocks, inertial outputs), `latch.c` (`MODELS[]` :975, `LATCH_KIND`
  :1099–1102), `buffer_reg.c` (the `registered` model: `ce` on a register,
  enable arrays).
- `backend/tests/latch_test.c` (fixture + function-table pattern, `main`
  :1202), `support/listing.c` (`listing_kind_name` :6–34, the `units=` case
  :60–69, the `logic` condition :234–237), `trace_check.c:22`
  (`TRACE_LAST_KIND`), `listing_check.c:50–51`.
- `backend/CMakeLists.txt` (sources :20–70, `logic_test(...)` lines).
- App kind lists: `schematical/schematical/Editor/KindSchema.swift`
  (`PartKinds.all`, `displayNames`, `schemas`, :199–235),
  `Editor/BundledPartFile.swift` (`PartKinds.name(of:)` :66–95),
  `scripts/part-editor-schema.py` (`KIND_FILES` :14–20) and
  `scripts/part-editor-schema-extra.py` (`EXTRA`),
  `schematicalTests/PartEditorWindowTests.swift` (:12–32).

## Prerequisites

plan-37-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-37-B complete | `ls planning/completed/plan-37-B-*.md` → exists | MET (2026-11-13: `plan-37-B-register-footprints.md`) |
| No `PART_REGISTER` yet | `grep -c 'PART_REGISTER' backend/include/parts.h` → 0 | MET (2026-11-13 at 31f79ce, before C's edits: 0) |
| The five families are still unknown | `grep -o '"LCX"\|"LVX"\|"VHC"\|"VHCT"\|"ALVT"' backend/src/library/kind_util.c \| wc -l` → 0 | MET (2026-11-13 at 31f79ce, before C's edits: 0). A family another plan added first makes its Phase 1 line moot (the grep shows it); add only the absent ones |
| The owner's uncommitted edits are not in C's files | `git status --short backend docs/parts-format.md docs/backend.md schematical/schematical/Editor schematical/schematicalTests/PartEditorWindowTests.swift schematical/schematical/Parts` → empty | MET (2026-11-13: the worktree is forked from main at 31f79ce; the owner's uncommitted edits live in the main checkout only, none in these paths) |

**Shared with plan-34 (resistors) and plan-36 (microcontrollers), both
open:** each appends its own `PartKind` (`PART_RESISTOR`, `PART_MCU`) as the
last value and moves the three last-kind test references, and each changes
the kind lists and counts of `PartEditorWindowTests`. C appends
`PART_REGISTER` after whatever value is last when C runs
(`grep -n '} PartKind;' -B2 backend/include/parts.h`), moves the same three
references to it, and raises each count in `PartEditorWindowTests` by one
from the value it has then.

## 1. Goal

- `ctest` passes T + 1 programs; the new one is `register_test`, all
  cases `PASS`.
- `git diff --stat backend/tests/golden` → empty after C.
- `docs/parts-format.md` has a kind-table row (`register.json` |
  `register`), the five families in § Values, and a `### register`
  section; `docs/backend.md` lists `register_test`.
- The Swift tests `PartEditorWindowTests` and `PartKindKeysTests` pass with
  `register.json` present.

### Non-goals (explicit constraints)

- **No change to any existing part's behavior or facts**: `parts.txt`,
  `traces.txt` and `footprints.txt` stay byte-identical (the empty
  `register.json` adds no part).
- No part data (D–F), no footprint merge.
- No change to `dff`, `mux` or any other kind's model.
- No app behavior beyond the kind lists (G): no `kindName`, no drawer
  filter change.
- Bus hold (ALVCH, LVCH, LVTH) is not modeled, as in every sequential kind.

## 2. Current State

- `PartKind` ends with `PART_CAPACITOR` (`parts.h:29–30`); `KINDS[]` has 27
  kinds (`grep -c '_KIND,$' backend/src/library/kinds.c` → 27). The
  last-kind test references: `grep -rn 'TRACE_LAST_KIND PART_\|\[PART_[A-Z_]* + 1\]' backend/tests`
  → 3 lines (`trace_check.c:22`, `listing_check.c:50`, `:51`).
- Families: `LOGIC_FAMILIES[]` (`kind_util.c:435–439`) has 32 names, HC …
  TXV; none of LCX, LVX, VHC, VHCT, ALVT.
- No existing model fits a '173: `dff` units have one clock each and at
  most one active-low output enable for the whole device, no clock enable
  and no 3-state group of two enables (`dff.c` `DEVICE_KEYS`,
  `docs/parts-format.md` § `dff`); `latch` is level-sensitive; `buffer`'s
  `registered` model is a transceiver (two ports per unit).
- `scripts/part-editor-schema.py` builds a `<Kind>Draft.swift` from the
  kind's shipped values plus `EXTRA` (keys no shipped part uses). With
  `register.json` empty, every key comes from `EXTRA`.
- `PartEditorWindowTests` pins `enabledKinds == kindNames minus
  "capacitor"`, `enabledKinds.count == 26`, `files.count == 27`
  (`:14–15`, `:27`); a new `Parts/*.json` without its kind lines fails them.

## 3. Design

### 3.1 Kind and file

- `PART_REGISTER` appended last (§ Prerequisites). No new facts struct:
  parts carry `PartLogic` {family, units}; `units` = the number of stored
  bits.
- `backend/src/models/register.c`: `REGISTER_KIND = {PART_REGISTER,
  "register", &REGISTER_MODEL, DEVICE_KEYS, PART_KEYS, register_load_device,
  register_bind_pinout, register_load_part, NULL}`; one line in `KINDS[]`;
  the source in `backend/CMakeLists.txt`.
- `Parts/register.json`: `{"format": "schematical-parts", "version": 1,
  "kind": "register", "devices": []}` (the loader accepts an empty array:
  `library.c:839–850`).

### 3.2 Device format (a sequential kind)

`"model"` is required and is `parallel` (the sequential-kind rule, so a
later model needs no format change). `timing`: family → `{rise, fall}`
(ns, CLK → Q), plus `note`, `vcc`, `inputMax`. Part keys: `family`,
optional `vcc`, `inputMax`.

| Key | Value |
|---|---|
| `"groups"` | 1–4 objects (below); at most 64 stored bits in all |

| Group key | Value |
|---|---|
| `"d"` | 1–40 input pin names; one stored bit each |
| `"clk"` | required: `{"pin", "edge": "rising"\|"falling"}` |
| `"ce"` | optional enable: one control or an array of 0–4 controls; an edge loads only while every one is asserted ('173 G1·G2, '377 G, '823 CLKEN) |
| `"clear"` | optional control with `"when"`: `async` (acts at once: '173 CLR high, '823 CLR low, 4076B Reset high, 40174B CLR low) or `clock` (acts at the clock edge, whatever `ce` is: '575/'577 CLR low) |
| `"outputs"` | 1–2 banks: `{"q": [as many names as d], "invert": bool (default false), "oe": optional enable as `ce`}`; a bank without `oe` always drives ('377, '273-style); '379 has a `q` bank and an inverted bank; the 16820 two banks with 1OE / 2OE |

Every pin a group names must be exactly one pin of every pinout. Errors
name the place, e.g. `register.json: devices[0] (74_173): groups[0]:
"outputs"[0] "q" has 3 names, "d" has 4`.

### 3.3 Behavior

Per group, in this order on any pin change:

1. `clear` async asserted → every bit of the group 0; X on it → every bit
   that is not already 0 becomes X.
2. A clock edge (Verilog posedge/negedge, as `dff`): clocked `clear`
   asserted → 0; else every `ce` control asserted (or no `ce`) → bit = D
   (Z on D reads X); a `ce` control at X/Z (none deasserted) → bits whose
   D differs from the stored bit become X. An edge from X or Z, or a move
   into X/Z from the pre-edge level, may have been an edge: bits it would
   change go X (the `registered` model's rule).
3. Each bank drives `invert ? NOT bit : bit` while its `oe` is asserted, Z
   while one control is deasserted, X while one is X/Z and none is
   deasserted. OE gates only the outputs.

Outputs are inertial: fall delay for 0, rise for 1 and Z, the longer for X
(`seq.c` outputs). Power-on: every bit 0.

### 3.4 Families (`kind_util.c`, `docs/parts-format.md` § Values)

`LOGIC_FAMILIES[]` gains `"LCX", "LVX", "VHC", "VHCT", "ALVT"`. The doc
list gains them with one line each:

- `LCX`, `LVX`: onsemi low-voltage CMOS with 5 V-tolerant inputs
  (74LCX…, MC74LCX…, 74LVX…, MC74LVX…).
- `VHC`, `VHCT`: onsemi very-high-speed CMOS (74VHC…, MC74VHC…); VHCT has
  TTL-level inputs.
- `ALVT`: Nexperia low-voltage BiCMOS (74ALVT16823).

  The supply ranges are each part's `vcc`, from its datasheet (E, F).

### 3.5 Tests (`backend/tests/register_test.c`)

The `latch_test` shape: a fixture file (devices copied from the TI pin
names, 10 ns delays, levels checked 50 ns after each step) and one
`check_*` per device class:

| Case | Fixture | Covers |
|---|---|---|
| `check_173` | fx-74_173 (`sn74ls173a.pdf` p.1: 1 M, 2 N, 3–6 1Q–4Q, 7 CLK, 8 GND, 9 G1, 10 G2, 11–14 4D–1D, 15 CLR, 16 VCC) | every function-table row of `sn74ls173a.pdf`, two-control `ce` and `oe`, async clear high |
| `check_377` | fx-74_377 (G active low, no OE) | CE low loads, high holds; always-driven bank |
| `check_379` | fx-74_379 | `q` and inverted `qn` banks from one group |
| `check_534` | fx-74_534 | inverted 3-state bank |
| `check_575` | fx-74_575 | clocked clear (CLR low at ↑ clears; CLR low without ↑ holds) |
| `check_823` | fx-74_823 | async clear low, CLKEN, OE |
| `check_16820` | fx-74_16820 | two banks of one group, 1OE / 2OE independent |
| `check_16823` | fx-74_16823 | two groups, own CLK / CLKEN / CLR / OE |
| `check_unknowns` | fx-74_173 | Z on D stores X; X on G1 at an edge; X on CLR; X/Z on M; an X → 1 clock move |
| `check_loader` | inline JSON | each § 3.2 error: q/d count mismatch, > 64 bits, > 4 groups, missing `edge`, `clear` without `when`, 5 `ce` controls, unknown key, unknown family `"VHX"`; a part of family `VHC` (and LCX, LVX, VHCT, ALVT) loads |

Wired in: `logic_test(register_test)` in `backend/CMakeLists.txt`;
`listing.c`: `case PART_REGISTER: return "register";`, `PART_REGISTER` in the
`units=` case and the `logic` condition; `TRACE_LAST_KIND` and the two
`listing_check.c` arrays on the new last `PartKind`.

### 3.6 App kind lists (so `register.json` does not break the Swift suite)

- `KindSchema.swift`: `"register"` in `PartKinds.all` (alphabetical, after
  `"pio"`), `displayNames["register"] = "Register"`, `RegisterDraft.schema`
  in `schemas`.
- `BundledPartFile.swift`: `case PART_REGISTER: "register"`.
- `scripts/part-editor-schema.py` `KIND_FILES`: `"register":
  ["register.c", "seq.c"]`; `part-editor-schema-extra.py` `EXTRA["register"]`:
  every § 3.2 device and part key (no shipped part uses any yet); generate
  `Editor/RegisterDraft.swift` with `python3 scripts/part-editor-schema.py . register`.
- `PartEditorWindowTests`: both counts + 1 (from their value when C runs).
  The equality lines compare computed lists and need no edit.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Families

- [x] `kind_util.c` `LOGIC_FAMILIES[]`: append each of LCX, LVX, VHC, VHCT,
      ALVT that the § Prerequisites grep shows absent (all five).
- [x] `docs/parts-format.md` § Values: the names in the list and § 3.4's lines.

Check: `grep -o '"LCX"\|"LVX"\|"VHC"\|"VHCT"\|"ALVT"' backend/src/library/kind_util.c | wc -l`
→ 5; `ctest --test-dir /tmp/logic-p37 -R 'listing_check|library_check'` passes (est. 3 min).
Observed: `5`; `100% tests passed, 0 tests failed out of 2`.

Commit: (this commit)

### Phase 2 — The kind

- [x] `parts.h`: `PART_REGISTER` last (after `PART_CAPACITOR`; plan-34 and
      plan-36 had not appended theirs).
- [x] `models/register.c` (§ 3.1–3.3) on the `seq.h` helpers; `KINDS[]`;
      `CMakeLists.txt` source line.
- [x] `Parts/register.json` (empty `devices`).
- [x] `listing.c`, `trace_check.c`, `listing_check.c` (§ 3.5 last paragraph).

Check: `cmake --build /tmp/logic-p37 -j8 && ctest --test-dir /tmp/logic-p37 -j8`
→ `100% tests passed`; `git diff --stat backend/tests/golden` → empty (est. 4 min).
Observed: `100% tests passed, 0 tests failed out of 32` (before Phase 3);
golden diff empty.

Commit: (this commit)

### Phase 3 — `register_test`

- [x] `backend/tests/register_test.c` with the § 3.5 cases;
      `logic_test(register_test)`. The fixture literal is generated by
      `planning/plan-37-data/census/register-fixture.py`; cases 1–21.

Check: `/tmp/logic-p37/register_test | grep -c '^FAIL'` → 0 and its last
line `register_test: all passed` (est. 3 min).
Observed: `0` FAIL lines; `register_test: all passed`; full `ctest` →
`100% tests passed, 0 tests failed out of 33` (T0 + 1); golden diff empty.

Commit: (this commit)

### Phase 4 — Docs and app kind lists

- [x] `docs/parts-format.md`: kind-table row; `### register` (after
      `### shift`): § 3.2's tables, § 3.3's rules, an SN74LS173A example;
      the "Sequential kinds" heading names `register` (its one link updated).
- [x] `docs/backend.md` § Tests: `register_test`.
- [x] § 3.6's app lines; generate `RegisterDraft.swift`
      (`register: 0 devices, 0 parts, variant model, 28 source keys, missing ['op']`;
      `op` is seq.c's clock-gate key, which register.c never reads).
- [x] `PartKindKeysTests.files`: `"register": ["register.c", "seq.c"]`
      (added task, § Corrections).

Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p37-dd test -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/PartKindKeysTests CODE_SIGNING_ALLOWED=NO`
→ `** TEST SUCCEEDED **` (est. 8 min: the app build is most of it; no
narrower check runs Swift tests).
Observed: `** TEST SUCCEEDED **` (PartEditorWindowTests 5 cases,
PartKindKeysTests `testFormKeysCoverTheBackendKeys` passed).

Commit: (this commit)

## Decisions

- **One model now, `model` key kept.** Every register in the device list
  is a `parallel` register; the '670 register file already has the `mux`
  `regfile` model (plan-37-A § Decisions). The key is kept because the
  kind is a sequential kind (shared format, and a later model needs no
  format change).
- **Groups and banks, not per-bit units.** One group holds the bits that
  share clock, enable and clear; a bank is one row of outputs. This states
  the 16820 (one D, two outputs) and the '379 (Q and Q̄) directly, where
  `dff`-style units would repeat pins per bit.
- **A clocked clear wins over `ce`** at the edge. No device in the list has
  both ('575/'577 have no CE), so the rule only has to be stated, and this
  is the reading where CLR means "clear".
- **App kind-list lines live in C**, not G: the Swift suite fails the moment
  `register.json` exists without them (§ 2). G does the catalog, drawer,
  facts and form refinement.

## Corrections

- **2026-11-13 — the drift test covers `register` too (added task).**
  `PartKindKeysTests.files` lists each kind's sources; without a
  `register` line the drift test would never check `RegisterDraft`
  against `register.c`. Added `"register": ["register.c", "seq.c"]`; it
  passes.
- **2026-11-13 — C was written while A ran** (C reads no census or
  footprint data, § Depends on) and landed after B, as one commit for its
  four phases: Phases 1 and 4 both edit `docs/parts-format.md`, which a
  per-phase commit cannot split without interactive staging.
- **2026-11-13 — the doc example's LS timing** is `sn54173.pdf`'s (CLK to Q
  typ 17 / 22 ns at 5 V, 45 pF), the same values D's 74_173 spec uses.

## Summary

C makes `register` a loadable, tested kind with no parts, adds five
families, and keeps every existing golden unchanged.
