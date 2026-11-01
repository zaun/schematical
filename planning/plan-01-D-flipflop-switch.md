# plan-01-D: The `dff`, `jkff` and `switch` kinds

Last updated: 2026-10-31
Effort: large (3h–1d)
Depends on: plan-01-C

Ports the D flip-flop (`ff-d.c`, 1,795 lines, mostly tables), JK flip-flop
(`ff-jk.c`, 942 lines) and DIP switch (`switch.c`, 517 lines) models onto the
part library, with their data in `dff.json`, `jkff.json` and `switch.json`.

Outcome: all 672 D flip-flop, 186 JK flip-flop and 36 switch parts match
`golden/parts.txt` and `golden/traces.txt`.

References:

- `planning/plan-01-A-parts-baseline.md` §3 (design), §4 (trace/listing).
- `planning/plan-01-B-library-core.md` §3.2 (`KindContext`), §3.3 (hooks),
  §3.6 (exporter, `notes.py`).
- Old code: `backend/ff-d.c`, `ff-jk.c`, `switch.c`, `switch_check.c`.

## Prerequisites

See plan-01-A §Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-01-C complete | `ls planning/completed/plan-01-C-gate-kind.md` → exists | NOT MET |

## 1. Goal

- `dff`, `jkff`, `switch` are registered kinds; `listing_check` and
  `trace_check` cover 3,596 parts (2,702 + 672 + 186 + 36) and pass.
- `tests/switch_test.c` checks contact behavior on the new library.

### Non-goals

- Old files stay until plan-01-G. No behavior change.

## 2. Current State (read)

- `ff-d.c:11-38`: `FfUnit {d, clk, pre, clr, q, qn}` (pin indices, `FF_NO_PIN`
  when absent); `FfLayout {pin_count, unit_count, units, oe, pre_active_high,
  clr_active_high, negative_edge}` (`oe` = active-low output enable or none);
  `FfFamily {family, rise, fall, clr_wins}`. Up to 8 units.
- `ff-jk.c:5-45`: `FfUnit {j, k, clk, pre, clr, q, qn}`; `FfLayout {pin_count,
  unit_count, units, k_inverted, pre_active_high, clr_active_high,
  negative_edge}`; `FfFamily {family, rise, fall}`. Up to 2 units.
- Both: `FF_DEFINE_CHIP(NAME, ID, FUNCTION, DESCRIPTION, LAYOUT, PINS,
  FAMILIES, PART_LIST, GATES)`; families tables are sometimes shared between
  a base and its `_FK` definition (`ff-d.c:663` and `:1562` both use
  `FAMILIES_74_74`).
- `switch.c`: pin tables `PINS_2`…`PINS_10` (`iA`/`iB`, inout); series enum
  with actuator labels (`switch.c:102`); `SwitchPart {name, series,
  positions, package}`; position `p` closes the contact between pins `pA` and
  `pB` (`switch_set_position` → `sim_set_contact`). Definition id per
  (series, positions), e.g. `te_gds_04`.

### Measured populations

| What | Count | Command |
|---|---|---|
| D flip-flop definitions / parts | 23 / 672 | `grep -c '^FF_DEFINE_CHIP(' ff-d.c`; plan-01-A §2 |
| JK flip-flop definitions / parts | 8 / 186 | same for `ff-jk.c` |
| Flip-flop timing rows (with comment) | 71 (34) | `grep -cE '^\s*\{LOGIC_FAMILY_[A-Z]+, [0-9]+'` on ff-d.c 44, ff-jk.c 27; with `/*` 7 + 27 |
| `_FK` flip-flop definitions | 5 | ff-d 3, ff-jk 2 |
| Switch definitions / parts | 27 / 36 | plan-01-A §2 |

## 3. Design

### 3.1 `dff` kind keys

```json
"units": [{"d": "1D", "clk": "1CLK", "pre": "1PRE", "clr": "1CLR", "q": "1Q", "qn": "1QN"}],
"outputEnable": "OE",
"presetActiveHigh": false,
"clearActiveHigh": false,
"clockEdge": "rising",
"timing": {"HC": {"rise": 20, "fall": 20, "clearWins": false, "note": "…"}}
```

`pre`, `clr`, `q`, `qn`, `outputEnable` optional (absent = old `FF_NO_PIN`);
`clockEdge` `rising`|`falling`. Part key `family`. `units` = number of units
(≤ 8). Model: `ff-d.c` behavior moved to `src/models/dff.c` unchanged except
that pins come from the bound pinout and timing from the part's family entry.

### 3.2 `jkff` kind keys

Same shape with unit keys `j`, `k`, `clk`, `pre?`, `clr?`, `q?`, `qn?`, plus
`kInverted` (bool); timing `{rise, fall}`. ≤ 2 units. Model:
`src/models/jkff.c` from `ff-jk.c`.

### 3.3 `switch` kind keys

```json
"actuator": "Low profile",
"contacts": [["1A", "1B"], ["2A", "2B"], ["3A", "3B"], ["4A", "4B"]]
```

`PartSwitch.positions` = number of contacts; `actuator` from the old series
label. Model `src/models/switch.c`: `set_switch` hook closes/opens the
position's contact with `sim_set_contact`; no other behavior (as today).

### 3.4 Exporters

- `tools/export/export_dff.c` (includes `ff-d.c`), `export_jkff.c` (includes
  `ff-jk.c`), `export_switch.c` (includes `switch.c`).
- Device id = old id; pinouts `p<pin count>` and `lccc20` for `_FK`; the `_FK`
  merge rule of plan-01-C §3.3 (merge when units, polarity and edge settings, and timing are
  identical by name).
- Switch: device id = old id (`te_gds_04`), function = series (`GDS`),
  drawer `["Switch", "DIP"]`, pinout `p<pins>`.
- Notes: `@FAMILIES_74_74/S`-style placeholders.

### 3.5 Tests

- `tests/switch_test.c`: port of `switch_check.c:check_contacts` onto the new
  library (load `switch.json`, instance via the kind's model, drive one side,
  read the other with the position open and closed, chains of positions).
  `check_catalog` is covered by `listing_check`; `check_netlist` moves to
  `circuit_check` in plan-01-F.
- `tests/library_check.c`: `dff` unit naming a missing pin rejected;
  `clockEdge: "up"` rejected; `jkff` with 3 units rejected; switch contact
  naming one pin twice rejected.

### 3.6 Docs

`docs/parts-format.md`: `dff`, `jkff`, `switch` sections.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` = partly done, say what remains. Moot tasks:
> `- [x] ~~text~~ — moot: <evidence>`. Fill `Commit:` when a phase lands.

### Phase 1 — Flip-flops

- [ ] `src/models/dff.c`, `src/models/jkff.c`; add both kinds to `KINDS[]`.
- [ ] `tools/export/export_dff.c`, `export_jkff.c`; generate `dff.json`,
      `jkff.json`; run `notes.py` on both.
- [ ] `library_check` dff/jkff cases (§3.5); `docs/parts-format.md` sections.

Acceptance: 858 flip-flop parts match both goldens.
  Check: `ctest --test-dir /tmp/logic-build -R 'library_check|listing_check|trace_check' --output-on-failure` → passed, `3560 parts compared` (est. 1 min).
Commit: —

### Phase 2 — Switches

- [ ] `src/models/switch.c`; add to `KINDS[]`.
- [ ] `tools/export/export_switch.c`; generate `switch.json`.
- [ ] `tests/switch_test.c` + CTest `switch_test`; `library_check` switch case;
      `docs/parts-format.md` section.
- [ ] Move `planning/plan-01-D-flipflop-switch.md` to `planning/completed/` once every phase's `Commit:` is filled.

Acceptance: 36 switch parts match both goldens (the trace flips every
position at 1.5 µs, so contacts are exercised); contact tests pass.
  Check: `ctest --test-dir /tmp/logic-build -R 'library_check|listing_check|trace_check|switch_test' --output-on-failure` → passed, `3596 parts compared` (est. 1 min).
Commit: —

## Validation Plan

- Tests: `switch_test`; `library_check` cases; parity over 894 more parts.
- Coverage check: `listing_check` per-kind counts include `dff 672`,
  `jkff 186`, `switch 36`.
- Runtime proof: `trace_check --verbose CD74HC73E` shows Q changing only after
  falling clock edges (the 73 is negative-edge).
- Doc sync: `docs/parts-format.md`.
- Final gate: `cmake --build /tmp/logic-build -j8 && ctest --test-dir /tmp/logic-build` → all pass (est. 2 min).

## Decisions

See plan-01-A §Decisions. Nothing new.

## Corrections

(empty)

## Summary

Behavior code moves almost verbatim; the work is turning index-based layouts
into name-based kind keys. The `clearWins` and `kInverted` corner cases are
in the traces of the parts that use them.
