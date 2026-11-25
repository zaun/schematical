# plan-63-D: The backend's pin checks (`erc_check`)

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-63-C

Plan 63's overview and Prerequisites are in `planning/completed/plan-63-A-power-flag.md`.

The outcome of 63-D: **`erc_check` reports the five rules of KiCad's ERC that
read pin types, on a netlist of typed pins**: `pin_to_pin` (by a 12×12 pin map
passed in), `pin_not_connected`, `pin_not_driven`, `power_pin_not_driven` and
`no_connect_connected` (for a pin). It is pure C over pin types, so a test can
give it every pair of the twelve types and compare with what KiCad 10.0.8 said
(`planning/plan-63-data/probe_pin_matrix.txt`, `probe_nets.txt`). The Swift
side that builds its input is 63-E.

References:

- `planning/plan-63-data/README.md` (the rule table, the pin map, § How KiCad
  counts), `probe_pin_matrix.py`, `probe_nets.py`.
- `backend/include/voltage.h`, `backend/src/voltage.c` (the same shape: a pure
  check over a netlist, `n` problems returned, up to `cap` written),
  `backend/tests/voltage_test.c`.
- `backend/include/erc.h`, `backend/src/erc_pins.c` (plan-63-B).

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-63-C is complete | `cmake --build /tmp/logic-build --target erc_pins_test >/dev/null && /tmp/logic-build/erc_pins_test \| grep -c 'TL16C552FN'` → a number above 0 (C's last kind is asserted and passes) | NOT MET until C lands |
| The rest as plan-63-A § Prerequisites | its four commands | MET (2026-11-25) |

If plan-63-C is not complete, this plan cannot start, full stop.

## 1. Goal

- `erc.h` declares `ErcPin`, `ErcNet`, `ErcProblem`, `ErcProblemKind`,
  `ERC_DEFAULT_PIN_MAP` and `erc_check`.
- For every pair of the twelve pin types joined on one net, `erc_check` reports
  exactly the pin-level rules KiCad reported for that pair (78 pairs, and the 12
  types alone), per § 2's table.
- With a pin map the caller edits, a cell 0 reports nothing, 1 a warning, 2 an error.

### Non-goals (explicit constraints)

- Nothing about wires, labels, flags' drawing or severities: those are Swift (63-E).
  `erc_check` returns what the pin map says as a level (0, 1, 2) for `pin_to_pin`
  and nothing else.
- `voltage_check` is unchanged and is not merged into this.
- No new part data.

## 2. Current State

`voltage_check` (`backend/src/voltage.c`) is the precedent: input arrays, a
count returned, up to `cap` written, ordered by net then part then pin; its
tests build small netlists (`tests/voltage_test.c`).

KiCad's behavior, from the probes (all 2026-11-25, KiCad 10.0.8):

- The default pin map: `README.md` § KiCad's pin types (37 error cells and 31
  warning cells in the stock template, both halves counted).
- `probe_pin_matrix.txt`: the 78 pairs and the 12 lone pins, each with the rules
  reported; `pin_to_pin` for the 24 pairs of non-zero cells outside the no_connect row/column.
- `probe_nets.txt`: three outputs on one net → 2 `pin_to_pin`; two outputs and
  an input → 1; three inputs → 1 `pin_not_driven`; two `power_in` and an input →
  1 `power_pin_not_driven`; a No Connect flag exempts a pin from the three
  "not connected / not driven" rules and, with another pin on its net, gives
  `no_connect_connected`.

### Measured populations

| What | Count | Command |
|---|---|---|
| Pairs of the twelve types, a type with itself included | 78 | 12·13/2 |
| Cells of the default pin map equal to 2 (error), to 1 (warning), both halves of the symmetric map counted | 37, 31 | `python3 -I -c 'import json;t=json.load(open("<KiCad>/SharedSupport/template/Arduino_Nano/Arduino_Nano.kicad_pro"))["erc"]["pin_map"];print(sum(c==2 for r in t for c in r),sum(c==1 for r in t for c in r))'` → `37 31` (2026-11-25) |

## 3. Design Overview

```c
typedef struct {
    size_t net;              /* the pin's net, SIZE_MAX when no wire reaches it */
    ErcPinType type;
    int drives_power;        /* erc_pin_drives_power: a supply symbol's pin */
    int no_connect_mark;     /* a No Connect flag is on the pin's wire group */
} ErcPin;
typedef struct { int has_attachment; } ErcNet;   /* a label or a bus port is on the net */
typedef enum { ERC_PIN_TO_PIN, ERC_PIN_NOT_CONNECTED, ERC_PIN_NOT_DRIVEN,
               ERC_POWER_PIN_NOT_DRIVEN, ERC_NO_CONNECT_CONNECTED } ErcProblemKind;
typedef struct { ErcProblemKind kind; size_t pin, other; size_t net; int level; } ErcProblem;
extern const uint8_t ERC_DEFAULT_PIN_MAP[ERC_PIN_TYPE_COUNT][ERC_PIN_TYPE_COUNT];
size_t erc_check(const ErcPin *pins, size_t pin_count, size_t net_count, const ErcNet *nets,
                 const uint8_t pin_map[ERC_PIN_TYPE_COUNT][ERC_PIN_TYPE_COUNT],
                 ErcProblem *out, size_t cap);
```

`pin` and `other` index `pins`. Returns the total count, writes up to `cap`,
ordered by net, then pin index (an unwired pin sorts as its own net after the
real ones). A net's **members** are the wired pins of every type but
`ERC_NO_CONNECT` (a no_connect-typed pin is never a connection) (a Power Flag is an ordinary pin of type `ERC_POWER_OUT`, with no part: KiCad's `PWR_FLAG` is a symbol with one `power_out` pin, and a lone one reports `pin_not_connected`, `probe_pin_matrix.txt` last block).

1. `no_connect_connected` (level 0): a wired pin of type `ERC_NO_CONNECT`; and
   a pin with `no_connect_mark` that is a member of a net with another member.
2. `pin_not_connected`: a pin that is not `ERC_NO_CONNECT`, not
   `ERC_FREE`, has no `no_connect_mark`, and is unwired, or whose net has only
   it as a member and no `has_attachment`.
3. `pin_to_pin`: per net, walking members in index order, each member that
   conflicts with an earlier one (pin map cell non-zero) is reported once with
   the first earlier member it conflicts with; `level` is the cell.
4. Driven checks, per net (an unwired pin is a net of one), skipped for a net
   that has a member with `no_connect_mark`: if a member is `ERC_POWER_IN`:
   unless a member is `ERC_POWER_OUT` or has `drives_power`, one
   `ERC_POWER_PIN_NOT_DRIVEN` on the first `power_in` member. Otherwise, if a
   member is `ERC_INPUT`: unless a member is `output`, `bidirectional`,
   `tri_state`, `passive`, `power_out` or has `drives_power`, one
   `ERC_PIN_NOT_DRIVEN` on the first input member.

Correctness risk: the exact counting of each rule; the 78-pair table and the
three-pin cases pin it. Byte-identity is not a gate (new code, no old output).

Rejected: passing `Part` pointers into `erc_check` (then a test needs fixture
parts of every pin type; with types in the input, the table test is exhaustive
and the type derivation is already tested in 63-B). Reporting one violation per
pin for the driven rules (KiCad reports one per net, `probe_nets.txt`).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as
> the work; `- [~]` with one line on what remains; moot tasks keep their line
> as `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` as a phase lands. An
> unticked box means NOT DONE.

### Phase 1 — The check

- [ ] `backend/include/erc.h`: the declarations of § 3 beside 63-B's.
      `backend/src/erc_check.c` (listed in `add_library(logic …)` in
      `backend/CMakeLists.txt` after `src/erc_pins.c`): `ERC_DEFAULT_PIN_MAP`
      (the 12×12 of `README.md`, rows in `ErcPinType` order) and `erc_check` per § 3.
- [ ] `docs/backend.md` § ERC pin types: the check, its input, the four rules and the
      level.

Acceptance: it builds and links into the library and the app's bridging header
exposes it (`erc.h` was added to `schematical-Bridging-Header.h` in 63-B).
  Check: `cmake -S backend -B /tmp/logic-build && cmake --build /tmp/logic-build --target logic 2>&1 | tail -2` → `[100%] Built target logic` (est. 3 min).
Commit: —

### Phase 2 — Tests against KiCad's own answers

- [ ] `planning/plan-63-data/probe_pin_matrix.py`: add `--c-table`, which prints
      the 78 pairs and 12 singles as C initialisers
      `{TYPE_A, TYPE_B, pin_to_pin_level, rules_mask}` (a bit per rule of § 3,
      taken from what `kicad-cli` reported, the five pin-level rules only), and
      generate `backend/tests/erc_pair_table.h` from it: `python3 -I
      planning/plan-63-data/probe_pin_matrix.py --c-table /tmp/pm >
      backend/tests/erc_pair_table.h`.
- [ ] `backend/tests/erc_test.c` (`logic_test(erc_test)`): (1) every pair of
      `erc_pair_table.h`: build a two-pin net of the types (a wire between them,
      no attachment), run `erc_check` with `ERC_DEFAULT_PIN_MAP`, and compare the set of rules and the
      `pin_to_pin` level; the 12 singles unwired; (2) `probe_nets.txt`'s six
      three-pin cases and six No Connect cases; (3) a net label's attachment
      makes a lone connected pin connected (`has_attachment`) and not driven
      when it is an input; (4) a Power Flag (a pin of type `power_out`) drives a
      `power_in` net and conflicts with an `output` (`power_out` + `output` = error);
      (5) a pin with `drives_power` drives both an input and a `power_in`;
      (6) a pin map with one cell changed from 2 to 0 reports nothing for that
      pair, and 0 to 1 a level-1 problem.

Acceptance: `erc_test` passes; its 90 table rows equal KiCad's reports.
  Check: `cmake --build /tmp/logic-build --target erc_test && /tmp/logic-build/erc_test | tail -3` → `FAIL` absent and the last line `0 failures` (est. 5 min).
Commit: —

## Validation Plan

- Tests: `erc_test` (the 90 KiCad rows, the net cases, the pin map edits).
- Coverage check: `ctest --test-dir /tmp/logic-build -R erc_` lists `erc_pins_test`, `erc_pins_check` and `erc_test`.
- Runtime proof: the table is KiCad's own output, regenerable with `kicad-cli`.
- Doc sync: `docs/backend.md`.
- Final gate (once, after Phase 2): `ctest --test-dir /tmp/logic-build -R 'erc_|voltage'` → 100% passed (est. 8 min).

## Decisions

- Driven rules report one problem per net, `pin_to_pin` one per later conflicting pin — decided by the author, 2026-11-25, from `probe_nets.txt`.
- A supply symbol drives its own net; a Power Flag is a `power_out` member — decided by the author, 2026-11-25 (63-B § 3; the owner chose the flag, 63-A).
- A net with a No Connect flag on a pin is skipped by the driven rules — decided by the author, 2026-11-25, from `probe_nets.txt`.

## Corrections

(Filled in during execution.)

## Summary

The risk is counting. The 90-row table is KiCad's own output, so a counting
error shows as a failing row. Untouched: the voltage check, every part, every
simulation.
