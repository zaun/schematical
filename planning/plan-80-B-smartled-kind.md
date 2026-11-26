# plan-80-B: Backend: the `smartled` kind and model, the `colors` hook, tests, docs

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-80-A

B adds the `smartled` kind: a WS2812-class addressable LED, simulated. The model decodes DIN with the
device's timing, keeps the first frame(s), forwards the rest on DOUT, and latches what it kept when
DIN stays low for the reset time. A new optional `SimComponentType` hook, `colors`, and a public
`circuit_part_colors` give the app each LED's colour. No part file gets parts here: C adds them.

References:

- `docs/backend.md` § Adding a kind (line 802); `docs/parts-format.md` § `led` (line 1089, the
  lights rule B reuses), § `charlcd` (timing-violation and X rules, the precedent for § 3.3).
- `backend/src/models/led.c` (lights through `lights.c`), `backend/src/models/charlcd.c:878`
  (`now - ctl->rise_time < tpw`: pulse widths from `sim_now`), `backend/src/models/shift_special.c:975-1017`
  (SN74LV8153 auto-baud: edge times and sample timers).
- `backend/src/engine/sim.h:7` (`SimTime` is `uint64_t` picoseconds), `sim.h:96-176`
  (`SimComponentType` and its optional hooks), `backend/include/circuit.h:95-101`
  (`circuit_part_lights`, `circuit_part_light_time`), `circuit.h:118` and `backend/src/circuit.c:779`
  (`circuit_lcd_state`, the shape `circuit_part_colors` copies).
- `backend/include/lcd_state.h` (a public state header, the shape of `smartled_state.h`).
- `planning/plan-80-data/specs.tsv` (A): the key values the tests' fixture devices copy.
- `planning/plan-74-E-ethernet-kind-rj45.md` (a kind letter's precedent).

## Prerequisites

plan-80-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-68 complete (owner, 2026-11-25: backend letters of the part plans wait for it, as in plans 69–74) | `ls planning/completed/plan-68-AK-*.md` → exists | NOT MET (2026-11-25: plan-68 A–AK open, 0 archived) |
| plan-68's size and coverage checks run | `node scripts/source-size.js --check` → exit 0; `node scripts/coverage.js --check --backend` → exit 0 | NOT MET (2026-11-25: neither script exists; plan-68-A adds them) |
| plan-80-A complete | `ls planning/completed/plan-80-A-*.md` → exists, or every A box ticked | NOT MET (2026-11-25: A not started) |
| The kind and hook are absent | `grep -c 'PART_SMARTLED' backend/include/parts.h` → 0; `ls backend/src/models/smartled.c` → no such file; `grep -c 'colors' backend/src/engine/sim.h` → 0 | MET (2026-11-25: 0; no file; 0) |
| T and goldens at B's start | `grep -c '^logic_test' backend/CMakeLists.txt`; `wc -l < backend/tests/golden/parts.txt` | 77 and 40832 at planning. B records its own T1 and P1 |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `"kind": "smartled"` loads devices with the keys of § 3.1 and refuses bad ones with the messages
  of § 3.1.
- A fixture package on a bench, DIN driven at exact edge times, shows the colour the stream sent and
  forwards the rest on DOUT with the § 3.2 widths; three benches fed each other's DOUT show the
  three colours of the stream.
- `ctest` passes T1 + 1 programs. The goldens do not move (no shipped part changes).

### Non-goals (explicit constraints)

- No change to the `led` kind, its file or its parts.
- No two-wire clocked protocol (APA102-style) and no backup data input (WS2813-style): A skips those
  parts (plan-80-A § 3.3).
- No current, brightness curve, PWM ripple or gamma: a channel's value is the byte received.
- `circuit_part_lights` keeps its meaning (on/off per light); the colour is a separate call.

## 2. Current State

There is no addressable LED model (`grep -rli 'ws2812\|addressable' backend/src` → nothing,
2026-11-25). The engine is event-driven in picoseconds: `pin_changed(sim, instance, state, pin,
old, new)` is called per change and `sim_now()` is exact (`sim.h:7`, `SimTime` = `uint64_t` ps), so a
0.3 µs pulse is 300,000 ps. `charlcd.c` already checks pulse widths this way. The last kind is
`PART_OPAMP` (`parts.h:32`; `kinds.c` `KINDS[]` ends with `&OPAMP_KIND`), but plans 69–74 add kinds
after it first; B adds after whatever is last then.

## 3. Design

### 3.1 Device and part keys

```json
{"id": "inolux-in-pi55", "function": "IN-PI55", "description": "Addressable RGB LED, 5050, 4 pins",
 "drawer": ["Displays & Indicators", "Addressable LED"],
 "pinouts": {"p4": [{"number":1,"name":"GND","role":"ground"},{"number":2,"name":"DOUT","role":"output"},
                    {"number":3,"name":"DIN","role":"input"},{"number":4,"name":"VDD","role":"power"}]},
 "din": "DIN", "dout": "DOUT", "order": ["G", "R", "B"],
 "timing": {"t0h": 300, "t1h": 600, "t0l": 900, "t1l": 600, "tolerance": 150,
            "reset": 200000, "delay": 80},
 "vcc": [3.7, 5.4],
 "parts": [{"name": "IN-PI55TATPRPGPB", "manufacturer": "Inolux", "package": "5050",
            "pinout": "p4", "footprint": "LED-ADDR-5050-4-INOLUX", "datasheet": "inolux-in-pi55tat"}]}
```

| Key | Value |
|---|---|
| `"din"`, `"dout"` | pin names, exactly one pin each, roles `input` and `output` (`smartled "din" must name one input pin`, `… "dout" … one output pin`) |
| `"order"` | 3 or 4 strings, the frame's bytes in send order, each `"R"`, `"G"`, `"B"`, `"W"` or `"-"` (ignored byte); at least one colour, no colour twice (`"order" must be 3 or 4 of R, G, B, W, -`). A frame is 8 × the length bits, MSB first |
| `"leds"` | optional, 1–8 (default 1): frames one package keeps, in order, LED 0 first |
| `"timing"` | ns, every key required: `t0h t1h t0l t1l tolerance reset delay`; `t0h` < `t1h`, every value ≥ 0, `tolerance` < (`t1h` − `t0h`) / 2, `reset` > `t0l` + `t1l` (`smartled "timing" …` with the failing rule) |
| `"vcc"`, `"inputMax"` | the supply range (§ Supply voltage); also allowed on the part |

Every pin of every pinout is `input` (only DIN), `output` (only DOUT), `power`, `ground` or `nc`;
at least one `power` and one `ground` pin (`a smartled pinout needs a power pin` / `… a ground pin`).

`PartSmartLed` (in `parts.h`) holds `channels` (the `order` as a byte string), `channel_count`,
`leds`, the seven timing values in ps, and the DIN/DOUT pin indices. `Part` gets
`const PartSmartLed *smartled;`.

### 3.2 Decoding (the model, `backend/src/models/smartled.c`)

- **Bit.** DIN rising at `r`, falling at `f`: width `w = f − r`. Let `mid = (t0h + t1h) / 2`. The bit
  is 0 when `t0h − tolerance ≤ w < mid`, 1 when `mid ≤ w ≤ t1h + tolerance`, else **X** (a width
  violation).
- **Gap.** A low from `f` to the next rise longer than `max(t0l, t1l) + tolerance` and shorter than
  `reset` is a gap violation: the frame in progress becomes all X. (Datasheets define no behavior
  there; B states this rule, and docs say so.)
- **Reset.** DIN low for `reset` (a timer set at each fall): the package latches its kept frames to
  its LEDs, frames that did not arrive in full leave their LED as it was, and the package starts
  receiving again.
- **Keep and forward.** The first `leds` × frame bits after a reset are kept. Every later bit is
  forwarded: DOUT rises at `r + delay` and falls at `max(f, r + delay + t?h)`, `t0h` or `t1h` by the
  decoded bit; an X bit drives DOUT X from `r + delay` to `f + delay`. Before the package has its
  bits, DOUT stays low (the datasheets: "the DOUT port is always pulled at low level").
- **X on DIN.** DIN reading X or Z makes the bit in progress X and drives DOUT X while it lasts.
- **Supply.** While no power pin reads 1, or a ground pin does not read 0, every LED is off and
  known, DIN is ignored and DOUT is driven 0; when power comes back the package is at its power-on
  state: every LED off (0, known), receiving the first frame.

### 3.3 The colour state and the hooks

- `backend/include/smartled_state.h`: `typedef struct { uint8_t value[4]; uint8_t known; uint8_t
  present; } SmartLedColor;` — `value` indexed R, G, B, W; `present` has bit c set for each colour the
  device has; `known` bit c set when that channel is known. `SMARTLED_MAX_LEDS 8`.
- `SimComponentType` gets an optional hook after the last one present:
  `size_t (*colors)(const void *state, SmartLedColor *out, size_t cap);` returns the LED count.
- `circuit.h`: `size_t circuit_part_colors(const Circuit *circuit, size_t part, SmartLedColor *out,
  size_t cap);` — the count, 0 for a part without the hook (no error), like `circuit_part_lights`.
- `lights`: one light per LED, on when some known channel is above 0 (so `circuit_part_lights` and
  `circuit_part_light_time` work for a smartled as for an LED).
- ERC (`backend/src/erc_pins.c`): DIN is an input, DOUT an output: the existing role rules apply; no
  new problem.

### 3.4 Where the risk is

A chain works only when the forwarding rule (§ 3.2) is right: an off-by-one in the kept-bit count, or a
DOUT edge scheduled before the decision at `f`, breaks LED 2 onward while LED 1 looks right. The
tests (§ 4) check LED 3 of a chain with mixed 0/1 bits and the DOUT widths, not only LED 1.

### 3.5 Rejected alternatives

- **Extending the `led` kind.** `led` is a passive two-pin die with an instance colour; a smartled has
  a decoder, state, an output pin and a colour set by the circuit. Mixing them would put an engine
  model into every plain LED.
- **Sampling DIN at a fixed delay after each rise** (how real chips do it). It needs the same
  threshold and gives the same bit for every in-tolerance pulse; the width rule also reports
  out-of-tolerance pulses as X, which is what a user debugging firmware needs.
- **Reporting colour through `lights`** as 3–4 lights per LED: `lights` is on/off, and the app's
  `lightTimes` would lose the byte values.

## 4. Tests (`backend/tests/smartled_test.c`, `backend/tests/support/smartled_fixture.h`)

Fixture devices from inline JSON: a GRB one with the Inolux IN-PI55 timing of § 3.1, an RGBW one
(`["G","R","B","W"]`), a single-die one (`["-","W","-"]`), and a `"leds": 3` one. Each case puts one
package on a bench (`backend/tests/support/bench.h`: `bench_set` DIN at an exact time, `bench_run`
to the next edge, `bench_get` DOUT; `charlcd_test.c`'s `at()` / `lv()` are the precedent) and reads
`circuit`-level colours through the model's `colors` hook on `bench_state`. A chain of real packages
on nets runs in C (`avr_chain_case`, with shipped parts and firmware).

- Load: each key's error message (one case per rule of § 3.1).
- One LED: 24 bits of `0x00FF00` then a reset → R 255, G 0, B 0, all known; `circuit_part_lights` → on.
- Forwarding: 72 bits (red, green, blue) then reset → the package shows red; DOUT is low through
  the first 24 bits, then shows 48 bits, each rising `delay` after its DIN rise and falling at
  `max(f, r + delay + t?h)` (sampled 1 ps either side of each expected edge); a second bench fed that
  DOUT edge list shows green, and a third, fed the second's, blue.
- Width X: one 450 ns pulse is a 1 (`mid` = 450 ns); a 760 ns pulse (> t1h + tolerance) makes its
  channel X after the reset.
- Gap X: a 5 µs low inside the frame → the frame is X; the next frame after a reset is clean.
- Short frame: 16 bits then reset → the LED keeps its old colour.
- RGBW and single-die: values land in the right channel; `present` lists only the device's colours.
- `"leds": 3`: one package keeps three frames and forwards the fourth.
- Power: VDD at 0 → off and known; DOUT 0; back to 1 → power-on state.
- `circuit_part_colors`: a circuit from the fixture library with one package (VDD on a 5 V supply,
  GND, DIN on a net of its own) started and advanced 1 ms → returns 1, the LED off and known, R G B
  present; on the circuit's supply part (no `colors` hook) → 0; `cap` 0 → the count, nothing written.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> fill each `Commit:` line with the hash when the phase lands.

### Phase 1: the kind and its loader

- [ ] `backend/include/parts.h`: `PART_SMARTLED` after the last `PartKind` value (grep first);
      `PartSmartLed`; the `Part` pointer.
- [ ] `backend/src/models/smartled.c`: `PartKindOps` (`device_keys` `din dout order leds timing vcc
      inputMax`, `part_keys` `vcc inputMax`), `load_device`, `bind_pinout`, `load_part` (§ 3.1).
- [ ] `backend/src/library/kinds.c` (`extern`, `&SMARTLED_KIND` at the end of `KINDS[]`),
      `backend/CMakeLists.txt` (source, `logic_test(smartled_test)`), `tests/support/listing.c` (name
      `smartled`, facts line `|order=GRB|leds=1|t0h=…`), `tests/trace_check.c` `TRACE_LAST_KIND`,
      `tests/listing_check.c` arrays — each line grepped first and added after the last kind present.
- [ ] `schematical/schematical/Parts/smartled.json` with `"devices": []`.
- [ ] `smartled_test.c` load cases.

Acceptance: `ctest --test-dir /tmp/logic-p80 -R smartled_test --output-on-failure` → passes, every
load case `ok`. (≈ 3 min with the build.)

Commit:

### Phase 2: the model and the hooks

- [ ] `smartled.c`: the `SimComponentType` (decoding, keep/forward, reset timer, supply; § 3.2).
- [ ] `backend/include/smartled_state.h`; the `colors` hook in `sim.h` after the last hook present;
      `circuit_part_colors` in `circuit.h` / `circuit.c` (§ 3.3); the `lights` hook.
- [ ] `smartled_test.c`: the behavior cases of § 4.

Acceptance: `ctest --test-dir /tmp/logic-p80 -R smartled_test --output-on-failure` → passes with
every § 4 case `ok`; `ctest --test-dir /tmp/logic-p80 -j8` → `100% tests passed` out of T1 + 1;
`git diff --stat backend/tests/golden` → empty. (≈ 4 min.)

Commit:

### Phase 3: docs

- [ ] `docs/parts-format.md`: a `### \`smartled\`` section (keys, behavior of § 3.2 including the gap
      rule, the example) and the drawer row Displays & Indicators → Addressable LED.
- [ ] `docs/backend.md` § Layout (`smartled.c`, `smartled_state.h`) and the hook list
      (`colors`, `circuit_part_colors`).

Acceptance: `grep -c '### `smartled`' docs/parts-format.md` → 1; `grep -c 'circuit_part_colors'
docs/backend.md` → at least 1. (seconds)

Commit:

## Validation Plan

- `node scripts/source-size.js --check` → exit 0 and `node scripts/coverage.js --check --backend` →
  exit 0 (plan-68's gates: `smartled.c` under 1000 lines, 100% line coverage). (≈ 5 min.)
- The Phase 2 acceptance re-run on the final tree.

## Corrections
