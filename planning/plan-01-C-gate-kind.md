# plan-01-C: The `gate` kind

Last updated: 2026-10-31
Effort: large (3h–1d)
Depends on: plan-01-B

One data-driven gate model replaces the six hand-written `74_*.c` files,
`gates.c` + `gates_defs.inc` + `gates_catalog.inc`, and `cd4000.c` +
`cd4000_parts.inc`. All 2,552 gate parts move into `Parts/gate.json`.

Outcome: `gate.json` loads, and all 2,552 gate parts match
`golden/parts.txt` and `golden/traces.txt`.

References:

- `planning/plan-01-A-parts-baseline.md` §3 (design), §3.6 (gate),
  §4.1/§4.2 (trace/listing).
- `planning/plan-01-B-library-core.md` §3.2 (`KindContext`), §3.6 (exporter, `notes.py`).
- Old code: `backend/74_00.c`, `74_02.c`, `74_08.c`, `74_32.c`, `74_86.c`,
  `74_266.c`, `gates.c`, `gates_defs.inc`, `cd4000.c`, `cd4000_parts.inc`.

## Prerequisites

See plan-01-A §Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-01-B complete | `ls planning/completed/plan-01-B-library-core.md` → exists | MET (2026-10-31: exists, eddebf1) |

## 1. Goal

- `gate` is a registered kind; `listing_check` and `trace_check` cover
  2,702 parts (150 from B + 2,552 gates) and pass.

### Non-goals

- Old gate files stay until plan-01-G (the old backend and app still use them).
- No change to gate behavior; traces must match.

## 2. Current State (read)

All three old gate engines compute the same thing:

- Value: AND/NAND/OR/NOR/XOR/XNOR over the inputs with X for unknown inputs
  unless a controlling value decides (`gates.c:gate_value`, `74_00.c:nand_value`,
  `cd4000.c:gate_value` — the same truth tables; CD4000 is 2-input only).
- Delay: rise for an output 1, fall for 0, `max(rise, fall)` for X; scheduled
  with `sim_schedule_inertial_drive(…, DRIVE_STRONG, delay)`.
- `start` evaluates every gate; `pin_changed` re-evaluates every gate fed by
  the pin (`gates.c:gt_pin_changed`; the hand files stop at the first match,
  which is the same because their gates share no inputs).
- CD4000: one delay for rise and fall per device (`cd4000.c:229-237`); family
  is `C`.

Two exceptions:

- `74_266.c:167-205` (open-drain XNOR): output 1 → drives **Z** with the rise
  delay; 0 → drives 0 with the fall delay; X → X with `max`.
- `74_86.c:280-338` (XOR): each family has two `{rise, fall}` pairs, chosen by
  the *other* input's current value: 0 → `other_low`, 1 → `other_high`, X →
  per-field max of the two. At `start` the other input counts as X.

### Measured populations

| What | Count | Command |
|---|---|---|
| Gate parts | 2,552 | plan-01-A §2 |
| Gate definitions (Logic minus 23 dff, 8 jkff) | 79 | 110 Logic definitions − `grep -c '^FF_DEFINE_CHIP('` (ff-d 23, ff-jk 8) |
| `_FK` gate definitions | 14 | 74_00/02/08/32/86: 1 each; gates.h: 9 |
| Gate timing rows with comments in `gates_defs.inc` | 123 of 123 | `grep -cE` (plan-01-A §2) |
| `case LOGIC_FAMILY` timing cases in hand files (with comment) | 80 (70) | `grep -c 'case LOGIC_FAMILY' 74_*.c` → 16+15+16+16+14+3; comments 6+15+16+16+14+3 |

## 3. Design

### 3.1 Kind keys

Device:

```json
"output": "push-pull",
"gates": [
  {"op": "nand", "inputs": ["1A", "1B"], "output": "1Y"}
],
"timing": {
  "HC": {"rise": 9, "fall": 9, "note": "Typical at 4.5 V, 50 pF"},
  "LS": {"otherLow": {"rise": 12, "fall": 10}, "otherHigh": {"rise": 20, "fall": 13}}
}
```

- `op`: `and`, `nand`, `or`, `nor`, `xor` (odd parity), `xnor` (even parity);
  1–13 inputs (`GT_MAX_INPUTS`).
- `output`: `push-pull` (default) or `open-drain` (1 → Z, per `74_266.c`).
- `timing` entry: either `{rise, fall}` or `{otherLow, otherHigh}` (allowed
  only when every gate has exactly 2 inputs; selection per `74_86.c`).
- Part: `family`, which must be a key of `timing` and a valid logic family.
- `PartLogic.units` = number of `gates`.

Each `inputs`/`output` name must resolve to exactly one pin in every pinout.

### 3.2 Model — `src/models/gate.c`

Bound pinout data: per gate, input pin indices and output pin index.
State: the part's timing entry and output mode. `start`/`pin_changed` as §2,
including the 74x86 "other input" rule and the open-drain rule. Its
`gate_value` is today's `gates.c:gate_value`.

### 3.3 Exporter — `tools/export/export_gate.c` and friends

One translation unit per old file (`export_gate_7400.c` includes `74_00.c`,
… `export_gate_gt.c` includes `gates.c`, `export_gate_cd.c` includes
`cd4000.c`); each appends devices to one shared `gate.json` writer.

- Device id = the old definition id (`74_00`, `74_10`, `4001B`).
- Pinout keys: `p<pin count>` for the base definition (e.g. `p14`, `p16`),
  `lccc20` for `_FK`.
- Gates from the old layout tables (`GatePins`, `GtGate`, `CdGate`), written
  by pin **name** through the definition's pin table.
- **Merging `_FK`**: an `_FK` definition becomes a second pinout of its base
  device when its gates (by name) and timing are identical to the base's;
  otherwise it stays its own device with id `<id>_FK`. Part pinout follows.
- Timing: hand files via their `family_timing()` for each family that some
  part uses; `74_86.c` writes `otherLow`/`otherHigh`, collapsing to
  `{rise, fall}` when the two pairs are equal; CD4000 writes
  `{"C": {rise: d, fall: d}}`.
- Notes: placeholders `@74_00.c/HC`, `@SN74_10_FAMILIES/HC`, … (plan-01-B §3.6).
- Device notes: `gates_defs.inc` "Pinout from …" comments.

### 3.4 Docs

`docs/parts-format.md`: add the `gate` section (§3.1).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` = partly done, say what remains. Moot tasks:
> `- [x] ~~text~~ — moot: <evidence>`. Fill `Commit:` when a phase lands.

### Phase 1 — Gate model and kind

- [x] `src/models/gate.c` (§3.1, §3.2); add `&GATE_KIND` to `KINDS[]`.
      Device key `units` added (optional, default = number of gates); see
      Corrections.
- [x] `tests/library_check.c`: gate cases — `otherLow` on a 3-input device
      rejected; `family` missing from `timing` rejected; gate input naming a
      pin absent from the second pinout rejected; `op` `"nandd"` rejected
      (+ a valid two-pinout gate file, and an unknown family key).

Acceptance: the loader takes valid gate files and rejects the four bad ones.
  Check: `ctest --test-dir /tmp/logic-build -R library_check --output-on-failure` → passed (est. 1 min).
  Result: `library_check: all passed` (all four gate cases print their path-style message).
Commit: —

### Phase 2 — Export `gate.json`

- [ ] `tools/export/export_gate_*.c` (§3.3) for 74_00, 74_02, 74_08, 74_32,
      74_86, 74_266, gates, cd4000.
- [ ] Generate `schematical/schematical/Parts/gate.json`; run `notes.py` on it.
- [ ] `docs/parts-format.md`: gate section.
- [ ] Move `planning/plan-01-C-gate-kind.md` to `planning/completed/` once every phase's `Commit:` is filled.

Acceptance: every gate part matches both goldens.
  Check: `ctest --test-dir /tmp/logic-build -R 'listing_check|trace_check' --output-on-failure` → passed, `2702 parts compared` (est. 1 min).
  Check: `grep -c '"@' schematical/schematical/Parts/gate.json` → `0` (no unreplaced note placeholder) (est. seconds).
Commit: —

## Validation Plan

- Tests: `library_check` gate cases; `listing_check` and `trace_check` over
  all 2,552 gate parts (covers the 74x86 and 74x266 special paths: those
  parts' hashes include their output nets).
- Coverage check: `listing_check` per-kind counts include `gate 2552`.
- Runtime proof: `trace_check --verbose SN74HC266N` shows an output net going
  to `Z`, never `1`, from the part's own drive.
- Doc sync: `docs/parts-format.md`.
- Final gate: `cmake --build /tmp/logic-build -j8 && ctest --test-dir /tmp/logic-build` → all pass (est. 2 min).

## Decisions

See plan-01-A §Decisions. Nothing new.

## Corrections

- plan-01-B's last commit (archive + Phase 3) is eddebf1.
- §3.1: `PartLogic.units` cannot always be the number of `gates`. The old
  `gate_count` of CD4068B and CD4078B is 1 ("8-input NAND/AND gate") while
  their layouts have 2 gate entries (complementary outputs J and K from the
  same 8 inputs; `gates_defs.inc:2550-2553`). Added an optional device key
  `units` (default: the number of gates); the exporter writes it only when
  the old `gate_count` differs from the layout's gate count.

## Summary

The risk is the two odd gates (open-drain 266, other-input-timed 86) and the
`_FK` merge rule; both are pinned by per-part trace hashes. Merging is
data-driven (identical model ⇒ one device), so no device is merged by hand.
