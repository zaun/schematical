# plan-34-E: Backend — resistor links in the engine, the `resistor` kind

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-34-D (E reads no census output; it follows D because
letter order is implementation order)

E adds the `resistor` part kind and makes a resistor a **weak two-way
link** in the engine: each end sees the other end's level as a weak drive.
So a resistor from VCC to a net works like a pull-up, a gate driving that
net overrides it, and an inverter driving an LED through a series resistor
lights the LED. The behavioral outcome: `resistor_test` passes its engine
and kind cases (§ 6), `ctest` passes T0 + 1 programs, and every existing
golden is unchanged.

References:

- `backend/src/engine/sim.c` (`Diode` :51, `collect_diodes` :1013,
  the cluster build :746–790, `pass_link` :853, `resolve_cluster` :876),
  `backend/src/engine/sim.h` (the `diodes` hook :175–179).
- `backend/src/models/diode.c` (the kind E mirrors: `"diodes"` pairs,
  `bind_pinout`, the hook), `backend/src/models/model_pins.h`
  (`diode_pair`).
- `backend/src/voltage.c` (the diode carry :283–310).
- `backend/include/parts.h` (`PartKind` :24–29, `PartDiode` :133, `Part`).
- `docs/backend.md` § Adding a kind (:280), the Diodes paragraph (:79–111);
  `docs/parts-format.md` § `diode` (:693), the kind table (:9–29).
- `planning/completed/plan-29-H-diode-kind.md` (the engine-link precedent).
- `planning/plan-33-E-capacitor-kind.md` (open; also appends a `PartKind`
  and moves `TRACE_LAST_KIND`, § 2).

## Prerequisites

plan-34-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-34-D complete | `ls planning/completed/plan-34-D-*.md` → exists | NOT MET at writing (2026-11-11) |
| No `resistor` kind yet | `grep -c 'PART_RESISTOR' backend/include/parts.h` → 0 | MET (2026-11-11 at 8b1770b: 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `ctest --test-dir /tmp/logic-p34` → `100% tests passed` with T0 + 1
  programs (31 at planning time: `grep -c '^logic_test'
  backend/CMakeLists.txt` → 30 before E).
- `resistor_test` passes the § 6 cases.
- `backend/tests/golden/parts.txt`, `traces.txt`, `footprints.txt` are
  byte-identical (no part uses the kind yet; F adds them).

### Non-goals (explicit constraints)

- Ohms have no effect on logic: two weak values that disagree read X, the
  same as two pulls that disagree; there is no divider, no current, no
  power check.
- A resistor never passes a strong value: two strong drivers on its two
  ends are two separate nets, never a short (`smoke:`).
- Diode, contact and pull behavior is unchanged: every circuit without a
  resistor resolves exactly as before (the goldens are the check).
- The `supply` pulls stay `supply` parts (one pin, `"pull": true`); they
  are not turned into resistors.

## 2. Current State

- The engine knows two-pin links only as diodes: `Diode {anode, cathode}`
  collected from the `diodes` hook by `collect_diodes` at `sim_start`; nets
  joined by contacts or diode links form clusters; `resolve_cluster` passes
  strong values along the diode links to a fixed point, then weak values
  between nodes no strong value reaches, then reads each node (strong if
  any, else weak; `sim.c:876–937`).
- A pull drives `DRIVE_WEAK` (`supply.c`); every other driver is
  `DRIVE_STRONG`.
- `voltage.c` carries a diode's anode level to its cathode net (the
  `carried` levels, iterated until nothing changes).
- `PartKind` ends with `PART_DIODE` at planning time
  (`backend/include/parts.h:28`); `TRACE_LAST_KIND` is `PART_DIODE`
  (`tests/trace_check.c:22`); `listing_check.c:50–51` sizes two arrays by
  `PART_DIODE + 1`. **plan-33-E (open) appends `PART_CAPACITOR` and moves
  the same three lines.** E appends `PART_RESISTOR` after whatever value is
  last when E lands and moves the three lines to `PART_RESISTOR`; both
  plans' edits are "the new last kind", so either order works.

## 3. Design

### 3.1 Links

`sim.h`: a second hook, `size_t (*resistors)(const void *state, size_t *a,
size_t *b, size_t cap)` — the two-way links between pins of an instance;
called once, by `sim_start`; NULL = none. `sim.c`: the `Diode` struct gains
`int weak` (1 for a resistor link; `anode`/`cathode` are then its two ends
a/b). `collect_diodes` (renamed in place to `collect_links`) reads both
hooks; clusters are built from all links as now.

`resolve_cluster`:

1. Seed `strong[g]`, `weak[g]` from each node's own drivers (unchanged).
2. Strong pass over the **diode** links only (unchanged; resistor links
   are skipped).
3. Weak pass to a fixed point over all links: a diode link as now (weak
   values between nodes no strong value reaches); a **resistor** link
   (a, b): `src(x) = strong[x] ? strong[x] : weak[x]`; `weak[b] |= src(a)`
   and `weak[a] |= src(b)`; repeat while any mask grew. The masks only
   grow, so it ends.
4. Read each node as now (strong if any, else weak; shorted only on its own
   strong values).

So: a strong 1 at a with nothing at b reads 1 at b; a strong 1 at a and a
strong 0 at b read 1 and 0 (no short); a chain of resistors carries a value
weakly to its end; a weak 1 from a resistor meets a pull-down's weak 0 → X.

### 3.2 The kind

`backend/src/models/resistor.c`, `RESISTOR_KIND` (`"resistor"`):

- Device keys: `"resistors"` (required), a non-empty array of `{"a": <pin
  name>, "b": <pin name>}`, one per resistor element; each name names at
  least one pin of every pinout; `a` ≠ `b`.
- Pins: every pin `inout`, `nc` or `absent` (`resistor pin "X" must be
  inout`); every `inout` pin is in some pair (`pin "X" is in no resistor`).
- Part keys (numbers): `"ohms"` (required, at least 0 and at most 1e9; 0 is
  a jumper); `"tolerance"` (optional, percent, above 0 and at most 20; a
  jumper has none); `"power"` (optional, watts per element, above 0 and at
  most 100). Unknown keys are errors as for every kind.
- Facts: `typedef struct { size_t resistor_count; uint64_t milliohms;
  uint32_t tolerance_ppm; uint32_t power_mw; } PartResistor;` (0 = not
  given for tolerance and power), `const PartResistor *resistor;` on
  `Part`.
- Engine: drives nothing; the `resistors` hook returns the bound pin pairs
  (as `diode_bind_pinout` binds `"diodes"`).
- `model_pins.h`: `resistor_pair(part, k, &a, &b)` for `voltage.c`.

### 3.3 Voltage carry

`voltage.c`: for a `PART_RESISTOR` part, each pair carries each end's
effective level to the other end's `carried` level, both ways, inside the
existing fixed-point loop (the diode block's code with both directions).
So a 5 V rail through a resistor to a 3.3 V part's input is refused, as a
5 V rail wired straight to it is. A resistor has no vcc.

## 4. File format (parts-format § `resistor`)

```json
{"id": "chip-1k-5k", "function": "1 kΩ – 5 kΩ", "description": "Thick film chip resistor",
 "drawer": ["Resistors", "Chip"],
 "pinouts": {"two-pin": [{"number":1,"name":"1","role":"inout"},{"number":2,"name":"2","role":"inout"}]},
 "resistors": [{"a": "1", "b": "2"}],
 "parts": [{"name": "RC0603FR-074K7L", "manufacturer": "YAGEO", "package": "0603",
            "pinout": "two-pin", "footprint": "RES-0603-YAGEO", "datasheet": "yageo-rc",
            "ohms": 4700, "tolerance": 1, "power": 0.1}]}
```

## 5. Correctness risk

The cluster resolve is shared by every diode and switch circuit; a mistake
in step 3 changes their results. The goldens (`traces.txt`) hold every
part's simulated behavior and must stay byte-identical — that is the gate
for "unchanged without resistors". The new behavior is pinned by
`resistor_test`'s engine cases.

## 6. `resistor_test` cases

Engine (a scripted driver, one pin per net, and a fixture component whose
`resistors` hook returns one or more links, as `diode_test.c`):

| Circuit | Expected |
|---|---|
| strong 1 at a, nothing at b | b reads 1 |
| strong 0 at a, nothing at b | b reads 0 |
| strong 1 at a, strong 0 at b | a 1, b 0, no `smoke:` |
| strong 1 at a, pull-down (weak 0) at b | b reads X |
| nothing at either end | both Z |
| a – R – b – R – c, strong 1 at a | b and c read 1 |
| strong 1 at a, b also driven strong 0 later | b follows to 0, then back to 1 when released |
| diode-OR of two gates into node n, n – R – m, nothing else at m | m reads the OR |

Kind (fixture JSON loaded with `load_parts`): a two-pin part loads with
facts `resistors=1 milliohms=4700000 tol=10000 power=100`; a 4-element
isolated array binds four pairs; rejections: no `"resistors"`, `a` = `b`, a
pin that is `output`, an `inout` pin in no pair, `ohms` -1 and 2e9,
`tolerance` 0 and 25, `power` 0. Circuit (`circuit.h`): `VCC` → resistor →
an SN74HC04N input reads 1; an SN74HC04N output → resistor → `GENERIC-LED`
anode, cathode → `GND`: the LED lights while the output is 1; a 5 V supply
through a resistor to the input of a fixture part with `vcc` 3.3 V and
`inputMax` 3.6 V is refused by the voltage check.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Engine links

- [ ] `sim.h`: the `resistors` hook; `sim.c`: `weak` on the link struct,
      `collect_links`, step 2 skipping weak links, step 3 (§ 3.1).
- [ ] `backend/tests/resistor_test.c` with the engine cases of § 6;
      `logic_test(resistor_test)` in `backend/CMakeLists.txt`.

Acceptance: the engine cases pass and nothing else moves.
  Check: `cmake --build /tmp/logic-p34 -j8 && ctest --test-dir /tmp/logic-p34 -R 'resistor_test|diode_test|trace_check|switch_test'` → 4 passed (est. 2 min; `trace_check` is the byte-identity gate for every existing part).
Commit: —

### Phase 2 — The kind and the voltage carry

- [ ] `parts.h`: `PART_RESISTOR` appended as the last `PartKind` value
      (after the last value present when E lands: `grep -o 'PART_[A-Z0-9]*
      }' backend/include/parts.h`), `PartResistor`, `Part.resistor`.
- [ ] `backend/src/models/resistor.c` (§ 3.2); `model_pins.h`
      `resistor_pair`; the `KINDS[]` line in `library/kinds.c`;
      `src/models/resistor.c` in `backend/CMakeLists.txt`.
- [ ] `voltage.c`: the resistor carry (§ 3.3).
- [ ] `tests/support/listing.c`: `case PART_RESISTOR: return "resistor";`
      and the facts line `resistors=%zu milliohms=%llu tol=%u power=%u`;
      `TRACE_LAST_KIND` (`tests/trace_check.c`) and the two arrays of
      `tests/listing_check.c` → `PART_RESISTOR`.
- [ ] `resistor_test.c`: the kind and circuit cases of § 6 (fixture JSON in
      `tests/support/resistor_fixture.h`, as `diode_fixture.h`).

Acceptance: every case passes and the goldens are unchanged.
  Check: `cmake --build /tmp/logic-p34 -j8 && ctest --test-dir /tmp/logic-p34` → `100% tests passed` out of T0 + 1, and `git diff --stat backend/tests/golden` → empty (est. 2 min).
Commit: —

### Phase 3 — Docs

- [ ] `docs/parts-format.md`: the `resistor.json` | `resistor` row in the
      kind table; a § `resistor` section (§ 3.2, § 4's example, the
      behavior of § 3.1 in words).
- [ ] `docs/backend.md`: a **Resistors** paragraph beside **Diodes** (§ 3.1
      and the § 6 table), and `resistor.c` in the source map.

Acceptance: the docs name the kind and its keys.
  Check: `grep -c '### `resistor`' docs/parts-format.md` → 1, `grep -c 'resistor.json' docs/parts-format.md` → ≥ 1, `grep -c '\*\*Resistors\*\*' docs/backend.md` → 1 (est. 1 min).
Commit: —

## Validation Plan

- Tests: `resistor_test` (engine, kind, circuit, voltage); `library_check`
  unchanged; `trace_check` byte-identical.
- Coverage check: `resistor_test`'s case list matches § 6 (one `PASS` line
  per row).
- Runtime proof: § 6's circuit cases run the real library parts.
- Doc sync: `docs/parts-format.md`, `docs/backend.md`.
- Final gate (once, after Phase 3): `ctest --test-dir /tmp/logic-p34` →
  `100% tests passed` (est. 1 min).

## Decisions

- **Weak two-way link** — owner, 2026-11-11 (plan-34-A § Decisions): each
  end sees the other end's level as a weak drive.
- **A second hook, not a generalized link hook** — author: `diodes` keeps
  its contract and its tests; the link struct gains one flag. Rejected:
  renaming `diodes` to a typed `links` hook (touches `diode.c` and
  `diode_test` for no behavior).
- **Tolerance and power optional** — author: `GENERIC-RESISTOR` has
  neither (any value, any make); a jumper has no tolerance.
- **Facts in integers** (`milliohms`, `tolerance_ppm`, `power_mw`) — author:
  the listing golden prints integers, as `PartDiode`'s `vr_mv`.
- **The voltage check carries across a resistor** — author: a pull-up to a
  5 V rail does put 5 V on a 3.3 V input.

## Corrections

(none yet)

## Summary

E's risk is the shared cluster resolve; `trace_check`'s byte-identity over
every existing part guards the old behavior and `resistor_test` pins the
new. Left untouched: part files, footprints, the app.
