# plan-82-F: Backend: multi-die LEDs (`"dies"`), the LED fact keys, `led_test`, docs

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-82-E

F teaches the `led` kind two things: a device may have several dies (bi-colour, RGB, bidirectional,
independent), and a part may carry its datasheet facts. No part file changes here: H and I add
the parts. The existing `GENERIC-LED` loads, lists and simulates exactly as before.

References: `docs/backend.md` § Adding a kind, § goldens; `docs/parts-format.md` § `led`,
§ `resistor` (several pins under one name: `kind_pins`); `backend/src/models/led.c` (144 lines),
`backend/src/models/lights.c`, `backend/include/parts.h:89` (`PartLed`),
`backend/tests/support/listing.c:367` (the `led` facts line); `planning/plan-74-E-ethernet-kind-rj45.md`
(a backend letter's precedent).

## Prerequisites

plan-82-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-68 complete (owner, 2026-11-25: backend letters wait for it) | `ls planning/completed/plan-68-AK-*.md` → exists | NOT MET (2026-11-25: plan-68 A–AK open) |
| plan-68's size and coverage checks run | `node scripts/source-size.js --check` → exit 0; `node scripts/coverage.js --check --backend` → exit 0 | NOT MET (2026-11-25: neither script exists; plan-68-A adds them) |
| plan-82-E complete | `ls planning/completed/plan-82-E-*.md` → exists, or every E box ticked | NOT MET (2026-11-25) |
| The keys are absent | `grep -c '"dies"' backend/src/models/led.c` → 0; `ls backend/tests/led_test.c` → no such file | MET (2026-11-25: 0; no file) |
| T and goldens at F's start | `grep -c '^logic_test' backend/CMakeLists.txt`; `wc -l < backend/tests/golden/parts.txt` | 77 and 40832 at planning; F records T1 and P1 |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- A device may give `"dies"` instead of `"colors"` + `"anode"` + `"cathode"`; it loads, binds and
  simulates one light per die.
- A part may carry the fact keys of § 3.2; `PartLed` exposes them.
- `listing_check` prints the dies and facts; the goldens do not move (no shipped part uses them).
- `led_test` (new, T1 + 1 programs) covers every rule and error of § 3.

### Non-goals (explicit constraints)

- `GENERIC-LED` and every schematic that places it load and run unchanged: its listing line and
  traces are byte-identical (the `parts.txt` / `traces.txt` gate below).
- No current, brightness or Vf model: the facts are facts. An LED is lit exactly when an anode
  reads 1 and its cathode 0, as today.
- No change to `lights.c`'s API.

## 2. Current State

`led.c`: `DEVICE_KEYS = {"colors","anode","cathode"}`; `led_bind_pinout` binds each name to exactly
one pin with `kind_pin`; the state has one `Light`; `led_lights` / `led_light_time` report 1.
`PartLed` is `{colors, color_count}` (`parts.h:89`). Part keys: none. `listing.c:367` prints
`colors=…`. There is no `led_test`; `library_check.c` and the goldens cover `GENERIC-LED`.

## 3. Design

### 3.1 `"dies"`

```json
"dies": [{"anode": "A", "cathode": "K1", "color": "Red"},
         {"anode": "A", "cathode": "K2", "color": "Green"},
         {"anode": "A", "cathode": "K3", "color": "Blue"}]
```

- 1 to 4 objects (`"dies" must have 1 to 4 dies`). Keys `anode`, `cathode`, `color`, all strings,
  all required.
- Exclusive with `colors`/`anode`/`cathode` (`an led has "dies" or "colors", "anode" and
  "cathode"`). A `dies` device's colours are fixed: `PartLed.colors` is the dies' colours in order,
  so the app's picker (shown only when `colors.count > 1` and the device has no dies, G) never
  shows for it.
- Pin names bind with `kind_pins`: a name may name several pins (a PLCC-4 single die with two K
  pads), which the die treats as one node, joined strongly as in `resistor`. A die's anode ≠ its
  cathode (`die N: anode and cathode must differ`). Two dies may share pins in either direction
  (common anode, common cathode, bidirectional `1>2` + `2>1`).
- Every pinout pin is `input`, `nc` or `absent`, and every `input` pin is in some die (`pin "X" is
  in no die`).
- Simulation: die *i* is lit while its anode reads 1 and its cathode reads 0; one `Light` per die
  (`lights_set(&state->lights, i, …)`); `led_lights` and `led_light_time` report `die_count`
  lights in die order. A one-die device (either shape) reports 1, as today.
- `PartLed` gains `die_count` and `dies` (`{anode_pins, anode_pin_count, cathode_pins,
  cathode_pin_count, color}`, pin indices into `Part.pins`); a `colors` device gets one die from
  its `anode`/`cathode` with `color` = NULL (the placed instance chooses).

### 3.2 Part fact keys (all optional)

| Key | Value | `PartLed` field |
|---|---|---|
| `"wavelength"` | dominant wavelength, nm, 360–780; a number, or an array with one entry per die | `wavelength_nm[4]` (0 = not given) |
| `"mcd"` | typical luminous intensity, mcd, > 0; number or per-die array | `mcd_x1000[4]` (thousandths of a mcd, integer) |
| `"vf"` | typical forward voltage, V, 0.5–10; number or per-die array | `vf_mv[4]` |
| `"if"` | the test current of those values, mA, > 0 and ≤ 1000 | `if_ua` |
| `"viewingAngle"` | 2θ½, degrees, > 0 and ≤ 180 | `viewing_angle_deg` |
| `"lens"` | `"water clear"`, `"diffused"`, `"white diffused"`, `"tinted"` or `"tinted diffused"` | `lens` |

Errors: an array whose length is not the die count (`"vf" needs one value per die`), a value out
of range (`"wavelength" must be 360 to 780`), an unknown lens (`"lens" must be water clear,
diffused, white diffused, tinted or tinted diffused`). `GENERIC-LED` carries none of them.

### 3.3 Listing

`listing.c`'s `led` line: unchanged for a `colors` device without facts (so `GENERIC-LED`'s line is
byte-identical); a `dies` device prints `dies=A>K1:Red,A>K2:Green,…`; facts print after, in § 3.2
order, only when given (`wl=625`, `mcd=150`, `vf=2.0`, `if=20`, `angle=120`, `lens=water clear`;
per-die values `/`-separated).

### Rejected alternatives

- **A new `rgbled` kind.** The dies rule is the only difference; a second kind would duplicate
  `led`'s loader, symbol and drawer, and a bi-colour LED fits neither.
- **Colour per part instead of per die.** A bi-colour part has two colours; per die is the only
  shape that covers every configuration.

## 4. Phases

### Phase 1: `"dies"` and the facts

- [ ] `led.c`: the § 3.1 loader, binder and N-light state; § 3.2 part keys.
- [ ] `parts.h`: `PartLed` fields.
- [ ] `listing.c`: § 3.3.

Acceptance: `ctest --test-dir /tmp/logic-p82 -R 'library_check|listing_check|trace_check'` passes
with `git diff --exit-code backend/tests/golden` (goldens unchanged) — 2 min.

Commit:

### Phase 2: `led_test`

- [ ] `backend/tests/led_test.c` (`logic_test(led_test)` in `CMakeLists.txt`, after the last
      `logic_test`): inline-JSON fixtures for a common-anode RGB, a common-cathode bi-colour, a
      bidirectional pair, a PLCC-4 single die with two K pins; each die lit only on its own pins,
      both directions of the bidirectional pair, every error message of § 3.1 and § 3.2, the
      fact fields.

Acceptance: `ctest --test-dir /tmp/logic-p82` → `100% tests passed` out of T1 + 1;
`node scripts/coverage.js --check --backend` exit 0; `node scripts/source-size.js --check` exit 0 —
5 min.

Commit:

### Phase 3: docs

- [ ] `docs/parts-format.md` § `led`: `"dies"`, the part keys, the drawer paths of H
      (Displays & Indicators → LED → Single color / Bi-color / RGB), an RGB example;
      `docs/backend.md` § tests: `led_test`.

Acceptance: `grep -c '"dies"' docs/parts-format.md` ≥ 2 and `grep -c 'led_test' docs/backend.md`
≥ 1 — 1 min.

Commit:

## 5. Validation Plan

F's gate: the full `ctest` of Phase 2 plus the unchanged goldens of Phase 1. The landing tier is
J's.
