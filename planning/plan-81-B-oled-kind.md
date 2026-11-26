# plan-81-B: Backend: the `oled` kind (facts), `charlcd`'s OLED panels, `oled_test`, docs

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-81-A

B adds the `oled` kind with its facts-only devices and every key a facts device takes, and lets
`charlcd` carry an emissive OLED panel. No part file gains a part here (E adds them). C adds the
simulated devices (`"model"`), D their parallel interfaces.

References:

- `docs/backend.md` § Adding a kind (line 802); `docs/parts-format.md` § `mcu` (the
  `"simulated": false` shape and its errors), § `charlcd` (line 4822), § Supply voltage.
- `backend/src/models/mcu.c:58-124` (facts-only loading: passive engine model, refused by
  `circuit_add_part`); `backend/src/models/charlcd.c:205-210` (`panel`), `:248` (`contrast`).
- `backend/include/parts.h` (the `PartKind` list ends `PART_TIMER, PART_OPAMP` at :32;
  `PartCharLcd` at :165).
- `backend/src/library/kinds.c` (`KINDS[]` ends `&OPAMP_KIND`, :106), `backend/CMakeLists.txt`
  (sources :67, tests :282), `backend/tests/support/listing.c` (:23 kind names, :428 the charlcd
  facts line), `backend/tests/trace_check.c:22` (`TRACE_LAST_KIND`),
  `backend/tests/listing_check.c:50-51`.
- Precedent letters: `planning/plan-74-E-ethernet-kind-rj45.md` (a facts-only kind),
  `planning/completed/plan-11-B-charlcd-kind.md`.

## Prerequisites

plan-81-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-68 complete (owner, 2026-11-25, for plans 69–75's code letters; plan-81 follows the same rule) | `ls planning/completed/plan-68-AK-*.md` → exists | NOT MET (2026-11-25: plan-68 A–AK open, 126 boxes unticked) |
| plan-68's size and coverage checks run | `node scripts/source-size.js --check` → exit 0; `node scripts/coverage.js --check --backend` → exit 0 | NOT MET (2026-11-25: neither script exists; plan-68-A adds them) |
| plan-81-A complete | `ls planning/completed/plan-81-A-*.md` → exists | NOT MET (2026-11-25: A not started) |
| The kind is absent | `grep -c 'PART_OLED' backend/include/parts.h` → 0; `ls backend/src/models/oled.c` → no such file; `grep -c 'oled' backend/src/models/charlcd.c` → 0 | MET (2026-11-25) |
| T and goldens at B's start | `grep -c '^logic_test' backend/CMakeLists.txt`; `wc -l < backend/tests/golden/parts.txt` | 77 and 40832 at planning. B records its own **T1** and **P1** (plans 69–80 may have added kinds first) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `"kind": "oled"` loads facts-only devices with the keys of § 3.1; every error of § 3.1 is
  reported with its exact message; a circuit refuses an `oled` part whose device is facts only
  (`circuit_add_part` returns the same refusal an `mcu` without a model gets).
- `charlcd` accepts `panel` values `oled-white`, `oled-yellow`, `oled-green`, `oled-blue`,
  `oled-red`, and with one of them the `contrast` key may be absent.
- `ctest` passes T1 + 1 programs; the goldens do not move (no part changes).

### Non-goals (explicit constraints)

- No simulated `oled` device: the `"model"`, `"bus"`, `"glass"` and `"timing"` keys are C's and D's.
  In B they are unknown keys (the loader's usual `unknown key` error).
- The HD44780 model is not changed beyond `panel` and `contrast`; OLED character timing comes from
  each device's own `timing`/`exec` keys as for any charlcd.
- No app change (F).

## 2. Current State

- `PartKind` ends `PART_TIMER, PART_OPAMP` (`parts.h:32`); plans 69–80 add kinds after it, each by
  the rule "add only if absent and after whatever is last". B does the same.
- `charlcd.c:208-210` rejects any `panel` but the four LCD ones with
  `"panel" must be yellow-green, blue-negative, gray or white`; `:248` requires `contrast`.

## 3. Design

### 3.1 `oled` device and part keys (facts)

File `backend/src/models/oled.c` (the loader and the passive engine model; C adds
`oled_ctl.c`, D `oled_bus.c`, so no file passes plan-68's 1000-line rule). Facts struct
`PartOled` in `parts.h`, pointer `const PartOled *oled;` on `Part`.

| Key | Value | Error |
|---|---|---|
| `"simulated"` | required on a facts device: `false` | `an oled has no simulation model: "simulated" must be false` |
| `"controller"` | required: the controller as the datasheet names it, e.g. `"SSD1322"`; shown as is | `missing "controller"` |
| `"format"` | required: `"graphic"` or `"character"` | `"format" must be graphic or character` |
| `"width"`, `"height"` | graphic: pixels, 1–1024 each | `"width" must be 1 to 1024` |
| `"columns"`, `"rows"` | character: 1–40 and 1–4 | `"columns" must be 1 to 40`, `"rows" must be 1 to 4` |
| `"panel"` | required: emitted colour: `"white"`, `"yellow"`, `"green"`, `"blue"`, `"red"`, `"rgb"` (colour glass) | `"panel" must be white, yellow, green, blue, red or rgb` |
| `"zones"` | optional, graphic monochrome only: a two-colour glass, 2 objects `{"panel", "from", "to"}` (pixel rows, inclusive), covering 0…height−1 without gap or overlap | `"zones" must cover rows 0 to <h-1> once` |
| `"depth"` | required: bits per pixel, 1, 4, 16 or 18 | `"depth" must be 1, 4, 16 or 18` |
| `"interfaces"` | required: non-empty array of `"i2c"`, `"spi4"`, `"spi3"`, `"8080"`, `"6800"`, `"uart"`, `"usb"`, `"rs232"`, `"qspi"` | `unknown interface "X"` |
| `"vcc"`, `"inputMax"` | the logic supply (§ Supply voltage); device or part | `missing "vcc"` |
| `"panelVolts"` | optional: `[min, max]` of the panel's VCC, a fact | `"panelVolts" must be [min, max]` |

Part keys: `"vcc"`, `"inputMax"`, `"panel"` (overrides the device's: one Display Visions sheet,
several colours). `PartOled`: `format`, `width`, `height` (character: `columns`, `rows`),
`controller`, `panel`, `zone_count` + `zones`, `depth`, `interfaces` (bit mask, `OLED_IF_*`),
`simulated` (0 in B), `panel_min_mv`, `panel_max_mv`.

Pins: any roles; each pinout needs a `power` and a `ground` pin (`an oled pinout needs a power pin`
/ `… a ground pin`), as `mcu`. The engine model is passive (no callbacks), like `mcu` facts
devices; `circuit_add_part` refuses the part, and the listing line is
`|oled|<format>|<w>x<h>|<controller>|<panel>|depth=<d>|if=<list>|sim=0|`.

### 3.2 `charlcd`: OLED panels

- `panel` gains `oled-white`, `oled-yellow`, `oled-green`, `oled-blue`, `oled-red`. The error
  becomes `"panel" must be yellow-green, blue-negative, gray, white, oled-white, oled-yellow,
  oled-green, oled-blue or oled-red`.
- `contrast` is optional when `panel` starts `oled-` (an OLED has no V0); still required with an
  LCD panel (`missing "contrast"`, unchanged). `PartCharLcd.contrast_pin` is `SIZE_MAX` when absent.
- The model is unchanged: a panel is a fact the app draws (F). The `PartCharLcd.panel` string
  carries the value.

### 3.3 Tests

`backend/tests/oled_test.c` with `backend/tests/support/oled_fixture.h` (inline JSON devices):
one load per key and per error of § 3.1; the passive model (a placed facts part is refused by
`circuit_add_part`); the listing line. `charlcd_test.c` gains: each `oled-*` panel loads;
`contrast` absent with `oled-yellow` loads and absent with `white` fails `missing "contrast"`.

### Rejected alternatives

- **Facts-only OLEDs in the `generic` kind.** `generic` carries no display facts (format, size,
  panel, depth, interfaces), and C needs one kind for the simulated and facts devices, as `mcu` has.
- **Character OLEDs as their own kind.** The HD44780-compatible modules are exactly what `charlcd`
  models; only the panel differs.

## 4. Phases

### Phase 1 — the kind, facts only

- [ ] `backend/include/parts.h`: `PART_OLED` after the last value (grep first), `PartOled`, the `Part.oled` pointer.
- [ ] `backend/src/models/oled.c`: `OLED_KIND` with § 3.1's keys and errors; passive engine model.
- [ ] `backend/src/library/kinds.c` (extern + `KINDS[]` entry, last), `backend/CMakeLists.txt`
      (source, `logic_test(oled_test)`), `backend/tests/support/listing.c` (name `oled`, facts line),
      `backend/tests/trace_check.c` (`TRACE_LAST_KIND` = the last kind), `backend/tests/listing_check.c` arrays.
- [ ] `backend/tests/oled_test.c`, `backend/tests/support/oled_fixture.h` (§ 3.3).
- [ ] `schematical/schematical/Parts/oled.json` with `"devices": []` (72-D precedent).

**Acceptance:** `cmake --build /tmp/logic-p81 -j8 && ctest --test-dir /tmp/logic-p81 -R 'oled_test|listing_check|trace_check' --output-on-failure` → 3 passed. (≈ 1 min.)

Commit:

### Phase 2 — charlcd OLED panels

- [ ] `backend/src/models/charlcd.c`: § 3.2 (`panel` values and message, optional `contrast`).
- [ ] `backend/tests/charlcd_test.c`: § 3.3's charlcd cases.

**Acceptance:** `ctest --test-dir /tmp/logic-p81 -R charlcd_test --output-on-failure` → passed;
`diff <(git show HEAD:backend/tests/golden/parts.txt) backend/tests/golden/parts.txt` → no output. (≈ 30 s.)

Commit:

### Phase 3 — docs

- [ ] `docs/parts-format.md`: new `### \`oled\`` section (§ 3.1, facts devices; C and D extend it),
      the drawer table row "Displays & Indicators | … OLED (plan-81)" (line 135), § `charlcd`'s
      `panel` row and `contrast` row.
- [ ] `docs/backend.md` § Layout: `oled.c`.

**Acceptance:** `grep -c '### `oled`' docs/parts-format.md` → 1; `grep -c 'oled-yellow' docs/parts-format.md` → ≥ 1. (≈ 1 s.)

Commit:

## 5. Validation Plan

At the end of B: `ctest --test-dir /tmp/logic-p81 -j8` → `100% tests passed` out of T1 + 1;
`node scripts/source-size.js --check` and `node scripts/coverage.js --check --backend` → exit 0
(`oled.c` fully covered by `oled_test`). Goldens unchanged (`git diff --stat backend/tests/golden` → empty).
