# plan-63-C: Tri-state pins for the remaining kinds, and the KiCad symbol comparison

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-63-B

Plan 63's overview and Prerequisites are in `planning/plan-63-A-power-flag.md`.

63-B made `erc_pin_type` and the `pin_tristate` kind hook, and answered it for
the `buffer`, `latch` and `dff` kinds. The outcome of 63-C: **every kind that
gates outputs with an output enable answers the hook**: `register`, `shift`,
`counter`, `mux`, `converter` and `serial` (460 parts), so the golden
`erc_pins.txt` is final; and a script compares our types with the pin types of
KiCad's stock symbols for the 5254 parts that have one, and its result is
recorded here.

References:

- `planning/plan-63-B-erc-pin-types.md` (the function, the hook, the golden),
  `planning/plan-63-data/README.md`, `count_roles.txt`.
- `docs/parts-format.md` (the keys below), `scripts/kicad-compare.py`,
  `planning/kicad-compare.md`.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-63-B is complete | `/tmp/logic-build/erc_pins_check` → exit 0, and `grep -c '' backend/tests/golden/erc_pins.txt` → 40174 | NOT MET until B lands |
| The rest as plan-63-A § Prerequisites | its four commands | MET (2026-11-25) |

If plan-63-B is not complete, this plan cannot start, full stop.

## 1. Goal

- `part_pin_tristate` answers 1 for the output pins that the model of each of
  the six kinds releases through its output enable, and 0 for every other.
- `backend/tests/golden/erc_pins.txt` is regenerated; only lines of parts in
  those six kinds change, and only `o` becomes `t` in them.
- `scripts/kicad-pin-types.py` prints how our types compare with KiCad's, and
  the table is in § Results below.

### Non-goals (explicit constraints)

- No simulation change (`listing_check`, `trace_check`, `voltage_test` unchanged).
- eeprom and sram keep answering by role: their data pins are `inout`
  (bidirectional), which is what KiCad's symbols use.
- A part whose output enable gates only some outputs answers 1 for those and 0
  for the rest (`SN74HC595N`'s `QH'`).

## 2. Current State

The hook and the function exist (63-B). The six kinds name their output enable
in these keys (found by walking every device of the kind; `count_roles.py`'s
`oe` figure counts the parts):

| Kind | Key (path in the device) | Parts with it | Example part |
|---|---|---|---|
| `register` | `groups[].outputs[].oe` (a pin object, or a list of them) | 162 of 220 | `74ACT11374DWR` |
| `shift` | `units[].storage.outputEnable` | 170 of 597 | `SN74HC595N` |
| `counter` | `units[].outputEnable`, `units[].register.outputEnable` | 33 of 733 | `SN74ALS561AN` |
| `mux` | `units[].oe` | 25 of 346 | `CD74AC251M96` |
| `converter` | `bus.oe` | 63 of 1914 | `ADC08B200CIVS/NOPB` |
| `serial` | `tristate` (the TRI pin: "while high every output floats", `docs/parts-format.md:3006`), `hd6406.oe` | 7 of 247 | `TL16C552FN`, `HD3-6406-5` |

(Walk: `python3 -I` over `Parts/<kind>.json`, 2026-11-25. Total 460 = 162 + 170 + 33 + 25 + 63 + 7.)

`SN74HC595N` (device `74_595`): `storage.outputs` QA–QH gated by `outputEnable`
OE; `outputs` `[{"stage": 8, "pin": "QH'"}]` not gated.

`scripts/kicad-compare.py` loads KiCad's symbols (`load_symbols(root)` → `{key:
{props, pins{number: (name, type)}}}`), finds the symbol of a part number
(`build_symbol_keys`, `find_symbols`) and our parts (`load_ours()`); its `main()` is
guarded, so it imports as a module (`importlib`, the file name has a hyphen).

## 3. Design Overview

One answer per kind file, built on what the model already binds for driving Z;
the rule is the same everywhere: **an `output`-role pin is tri-state when the
model drives it to Z while its output enable is inactive.** Each kind's model
struct already has an `open_drain` callback (and, for `shift`, an `analog` one)
that the kind's table forwards (`shift_pin_open_drain`, `models/shift.c:1296`,
calls `dev->model->open_drain(dev->dev, internal->bound, pin)`); each gets a
`tristate` callback beside it and the kind's `pin_tristate` forwards it the
same way. A model without an output enable leaves it NULL.

Risk: a model whose Z set differs from its data key (a counter whose register
outputs only are gated). The golden shows each; the per-kind tests pin one
part of each.

Rejected: the same as 63-B (names; engine probing).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as
> the work; `- [~]` with one line on what remains; moot tasks keep their line
> as `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` as a phase lands. An
> unticked box means NOT DONE.

### Phase 1 — register, shift, counter (the sequential kinds)

- [ ] `backend/src/models/register.c`, `shift.c`, `counter.c` (and the
      files of their models that bind an output enable: `shift_special.c`,
      `counter_div.c`): in each kind's model struct a `tristate(const void
      *dev, const void *bound, size_t pin)` callback beside `open_drain`, set by
      every model with an output enable (§ 2's keys) to the pins it drives Z;
      each kind's table gets `<kind>_pin_tristate` forwarding it
      (as `shift_pin_open_drain` does, `shift.c:1296`).
- [ ] `erc_pins_test`: `SN74HC595N`: QA–QH tri_state, `QH'` output;
      `74ACT11374DWR`: every `output` pin tri_state; `SN74ALS561AN`: every
      `output` pin tri_state; a part of each kind with no enable
      (`SN74HC163N`, already asserted in 63-B) stays output.

Acceptance: the three kinds answer by their data.
  Check: `cmake --build /tmp/logic-build --target erc_pins_test listing_check trace_check && /tmp/logic-build/erc_pins_test && /tmp/logic-build/trace_check | tail -1` → every line `PASS`; `trace_check` as before (est. 8 min).
Commit: —

### Phase 2 — mux, converter, serial

- [ ] `backend/src/models/mux.c` (+ `mux_stored.c`): outputs gated by
      `units[].oe`. `converter.c` / `converter_load.c`: the data outputs gated
      by `bus.oe`. `serial.c` and the models that bind `tristate` / `hd6406.oe`
      (`ns16550.c`, `serial_hs82c52.c`; the same `tristate` callback beside `open_drain`): with `tristate`, every `output` pin; with
      `hd6406.oe`, the pins the OE pin releases. Each kind's table sets `pin_tristate`.
- [ ] `erc_pins_test`: `CD74AC251M96`: Y outputs tri_state; `ADC08B200CIVS/NOPB`:
      the data outputs tri_state; `TL16C552FN`: every `output` pin tri_state.
- [ ] Regenerate the golden: `/tmp/logic-build/erc_pins_check --print >
      backend/tests/golden/erc_pins.txt`; `git diff --stat` shows only
      `erc_pins.txt`; every changed line differs by `o` → `t` only
      (`git diff -U0 backend/tests/golden/erc_pins.txt | grep '^[-+][^-+]' | awk -F'|' '{print $2}' | sort | uniq -c`
      lists only the six kinds, and a script check that each +/- pair differs
      only at `o`/`t` positions is run once and its output recorded).
- [ ] `docs/backend.md` § ERC pin types: the table of § 2 (kind, key).

Acceptance: the six kinds answer; the golden is final.
  Check: `cmake --build /tmp/logic-build --target erc_pins_test erc_pins_check listing_check trace_check voltage_test && /tmp/logic-build/erc_pins_test && /tmp/logic-build/erc_pins_check && /tmp/logic-build/listing_check | tail -1 && /tmp/logic-build/trace_check | tail -1 && /tmp/logic-build/voltage_test | tail -1` → all pass, the last three as before (est. 9 min).
Commit: —

### Phase 3 — Compare with KiCad's symbols

- [ ] `scripts/kicad-pin-types.py` (`python3 scripts/kicad-pin-types.py
      [--kicad <SharedSupport dir>]`): imports `scripts/kicad-compare.py` by
      `importlib`, reads `backend/tests/golden/erc_pins.txt`, and for each part
      that `find_symbols` matches, compares pin by pin number our type with the
      symbol's `(pin <type>)`; prints (1) a table of (our type, KiCad's type)
      pair counts, (2) the parts where tri_state and output disagree, by kind
      (counts and the first three names). It writes nothing to `planning/kicad-compare.md`.
- [ ] Record the output in § Results below, and for each (our, KiCad) pair
      with more than 100 pins say in one line why it differs (a KiCad
      convention, such as `passive` for a connector, or a defect to fix); a defect
      found is fixed in the kind's hook and the golden regenerated in this phase.

Acceptance: the script runs and its table is recorded.
  Check: `python3 scripts/kicad-pin-types.py 2>&1 | tail -30` → the table (est. 3 min).
Commit: —

## Results

(Filled in by Phase 3: the pair table, and a line for each pair of more than 100 pins.)

## Validation Plan

- Tests: `erc_pins_test`, `erc_pins_check` (the golden).
- Coverage check: `ctest --test-dir /tmp/logic-build -R erc_pins` lists both tests; the six kinds' key paths in § 2 each appear in at least one asserted part.
- Runtime proof: the Phase 3 table.
- Doc sync: `docs/backend.md`.
- Final gate (once, after Phase 3): `ctest --test-dir /tmp/logic-build -R 'erc_pins|voltage|listing|trace'` → 100% passed (est. 8 min).

## Decisions

- One rule for every kind: tri-state means the model drives the pin to Z through its output enable — decided by the author, 2026-11-25.

## Corrections

(Filled in during execution.)

## Summary

The risk is a kind whose Z set is not what its data key suggests; the golden
diff and the Phase 3 table show it. Untouched: simulation, part files, the voltage check.
