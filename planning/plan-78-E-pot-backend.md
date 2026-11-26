# plan-78-E: Backend: potentiometers in the `resistor` kind (`"wipers"`, `"pot"`, `"taper"`, `"turns"`)

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-78-D

E teaches the `resistor` kind potentiometers. A pot device names its gangs with a new device key,
`"wipers"`: each `{"a", "w", "b"}` loads as the two resistor pairs a–w and w–b, so the engine sees
the same weak links a fixed resistor already is, and Simulation › Run stays enabled. New facts:
the pot's type, taper and turns. No part file changes here: G and H add the parts.

References:

- `docs/parts-format.md` § `resistor` (line 1364); `docs/backend.md` § Resistors (line 153), § tests
  (`resistor_test`), § goldens (line 898).
- `backend/src/models/resistor.c` (285 lines: `DEVICE_KEYS` :19, `PART_KEYS` :20,
  `resistor_load_device` :38, `resistor_bind_pinout` :82, `resistor_load_part` :189);
  `backend/include/parts.h:218-225` (`PartResistor`); `backend/tests/resistor_test.c` (482 lines;
  `fact_cases`, `rejection_cases`, `circuit_cases`, the inline fixture `load_resistor_fixture`);
  `backend/tests/support/listing.c:189-193` (the resistor facts line).
- Precedent: `planning/completed/plan-34-E-*` (the resistor kind).

## Prerequisites

plan-78-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-68 complete (owner, 2026-11-25: backend letters wait for it, as for plans 69–75) | `ls planning/completed/plan-68-AK-*.md` → exists | NOT MET (2026-11-25: plan-68 A–AK open, 0 of its boxes ticked) |
| plan-68's size and coverage checks run | `node scripts/source-size.js --check` → exit 0; `node scripts/coverage.js --check --backend` → exit 0 | NOT MET (2026-11-25: neither script exists; plan-68-A adds them) |
| plan-78-D complete | `ls planning/completed/plan-78-D-*.md` → exists, or every D box ticked | NOT MET (2026-11-25: D not started) |
| The keys are absent | `grep -c 'wipers\|taper\|turns' backend/src/models/resistor.c` → 0 | MET (2026-11-25: 0) |
| `resistor.c` has room under plan-68's 1000-line rule | `wc -l < backend/src/models/resistor.c` | 285 at planning (limit 1000). Phase 3's `source-size.js --check` is the gate |
| T and goldens at E's start | `grep -c '^logic_test' backend/CMakeLists.txt`; `wc -l < backend/tests/golden/parts.txt` | 77 and 40832 at planning. E records its own T1 and P1 |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- A `resistor` device may give `"wipers"` instead of `"resistors"`; its parts load with the new
  facts, and its engine links are the a–w and w–b pairs.
- `resistor_test` covers every new key and error on fixture devices loaded from inline JSON, and a
  pot in a circuit: VCC on `a` gives a weak 1 on `w` (an LED from `w` to GND lights); VCC on `a`
  and GND on `b` gives X on `w` (two disagreeing weak values, the existing rule); the voltage check
  carries a 5 V `a` to a 3.3 V input on `w` and refuses it.
- `listing_check` prints the pot facts on a pot part's line only; every shipped line is unchanged:
  `parts.txt` and `traces.txt` do not move.
- `docs/parts-format.md` § `resistor` documents the keys; `docs/backend.md` notes that a pot's
  wiper is two resistor links.

### Non-goals (explicit constraints)

- No wiper position, no divider, no analog value: ohms keep having no effect on logic (owner
  decision: no knob).
- No new kind, no new `PartKind` value, no new engine hook. `PartResistor`'s existing fields keep
  their meaning for every fixed resistor.
- No change to `Parts/resistor.json` (G, H).

## 2. Current State

`resistor.c` loads `"resistors"` (non-empty array of `{"a","b"}`, a ≠ b) into
`ResistorDevice.pairs`; `resistor_bind_pinout` requires every pin to be `inout`, `nc` or `absent`
and every `inout` pin to be in a pair (`pin "X" is in no resistor`); pin names that several pins
carry are tied into one node. `PartResistor` = `{resistor_count, milliohms, tolerance_ppm,
power_mw}`. A pair whose ends share a pin with another pair already works (a bussed array's common
pin: the `RFX-RB3` fixture case at `resistor_test.c:448`), so a–w plus w–b needs no new engine code.

## 3. Design

### 3.1 Keys

Device keys (`DEVICE_KEYS` gains `"wipers"` and `"pot"`):

| Key | Value |
|---|---|
| `"wipers"` | non-empty array of 1 or 2 `{"a": <pin>, "w": <pin>, "b": <pin>}`, one per gang, the three names distinct (`a wiper's a, w and b must be different pins`). Errors: `"wipers" must be 1 or 2 {"a", "w", "b"}`; `a wiper is {"a": <pin name>, "w": <pin name>, "b": <pin name>}` |
| `"pot"` | with `"wipers"`, required: `"rotary"`, `"trimmer"`, `"slide"` or `"thumbwheel"` (`"pot" must be rotary, trimmer, slide or thumbwheel`) |

A device has exactly one of `"resistors"` and `"wipers"` (`a resistor device has "resistors" or
"wipers"`), and `"pot"` only with `"wipers"` (`"pot" needs "wipers"`). `resistor_load_device`
expands each wiper into two pairs, `{a, w}` and `{w, b}`, in gang order, so `bind_pinout`, the
links and the ties are unchanged.

Part keys (`PART_KEYS` gains `"taper"` and `"turns"`; both only on a part of a `"wipers"` device:
`"taper" is for a potentiometer`, `"turns" is for a potentiometer`):

| Key | Value |
|---|---|
| `"ohms"` | as today: required; for a pot, the end-to-end resistance a–b of each gang |
| `"tolerance"`, `"power"` | as today |
| `"taper"` | optional, `"linear"` (default), `"log"` or `"reverse-log"` (`"taper" must be linear, log or reverse-log`) |
| `"turns"` | optional, above 0 and at most 100 (default 1): the adjustment's turns, a decimal where the datasheet prints one (3.75) |

### 3.2 Facts (`parts.h`)

`PartResistor` gains, after `power_mw`:

```c
size_t wiper_count;    /* 0 for a fixed resistor, else 1 or 2 (gangs) */
const char *pot;       /* NULL, or "rotary" / "trimmer" / "slide" / "thumbwheel" */
const char *taper;     /* NULL for a fixed resistor, else "linear" / "log" / "reverse-log" */
uint32_t turns_centi;  /* turns × 100 (100 = one turn); 0 for a fixed resistor */
const char *const *wiper_pins; /* 3 × wiper_count pin names, a w b per gang; NULL when 0 */
```

The app's symbol (plan-78-F) reads `wiper_pins` to place a, w and b.

`resistor_count` is the loaded pair count (2 per gang), so code that reads it as "elements" sees a
pot as 2 or 4; the app (F) reads `wiper_count` first.

### 3.3 Listing (`backend/tests/support/listing.c`)

The resistor line keeps its fields and, only when `wiper_count > 0`, appends
` wipers=<n> pot=<type> taper=<taper> turns=<turns_centi>`. Fixed resistors print as today, so the
goldens do not move in E.

### 3.4 Tests (`backend/tests/resistor_test.c`)

The inline fixture gains three devices: `POT-FIX-1` (one gang, pins `1`, `2`, `3`, plus `MT1` `nc`;
`pot` `trimmer`; a part with `turns` 25), `POT-FIX-2` (two gangs, pins 1–6, `rotary`, `taper`
`log`), `POT-FIX-S` (`slide`, no `taper`, so `linear`). New cases:

- `fact_cases`: each part's `wiper_count`, `pot`, `taper`, `turns_centi`, `wiper_pins`,
  `resistor_count` (2 and 4), `milliohms`.
- `rejection_cases`: every error string of § 3.1 from a minimal inline device (both keys; neither;
  3 gangs; a repeated pin; `w` naming no pin; `pot` missing / unknown / without wipers; `taper` and
  `turns` on a fixed resistor; `turns` 0 and 101; an unknown taper).
- `circuit_cases`: the three circuit outcomes of § 1, on `POT-FIX-1`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Loader and facts

- [ ] `parts.h`: the four `PartResistor` fields and their comment.
- [ ] `resistor.c`: `"wipers"` and `"pot"` (expanded into pairs), `"taper"` and `"turns"`, every
      error of § 3.1.
- [ ] `listing.c`: the pot fields of § 3.3.

Acceptance: the shipped library loads unchanged.
  Check: `cmake --build /tmp/logic-p78 -j8 && /tmp/logic-p78/listing_check && /tmp/logic-p78/trace_check` → both pass with no diff against `backend/tests/golden/` (est. 2 min).
  Result:
Commit:

### Phase 2 — Tests

- [ ] The fixture devices and the `fact_cases`, `rejection_cases`, `circuit_cases` of § 3.4.

Acceptance: every new case passes, and so does every old one.
  Check: `cmake --build /tmp/logic-p78 -j8 && ctest --test-dir /tmp/logic-p78 -R '^resistor_test$' --output-on-failure` → `100% tests passed, 0 tests failed out of 1` (est. 1 min).
  Result:
Commit:

### Phase 3 — Docs and plan-68's gates

- [ ] `docs/parts-format.md` § `resistor`: the two device keys, the two part keys, the error
      strings, a pot example device; § The drawer's categories: **Discrete → Potentiometers →
      Rotary / Trimmer / Slide / Thumbwheel**.
- [ ] `docs/backend.md` § Resistors: one sentence, a pot's wiper is the two links a–w and w–b.

Acceptance: the docs name every new key, and plan-68's size and coverage rules hold.
  Check: `grep -c '"wipers"\|"pot"\|"taper"\|"turns"' docs/parts-format.md` → at least 4; `node scripts/source-size.js --check && node scripts/coverage.js --check --backend` → exit 0 (est. 6 min: the coverage build).
  Result:
Commit:

## Validation Plan

- Per-phase checks above. Final gate: plan-78-I.

## Decisions

- Owner, 2026-11-25: **no simulation knob**; a pot is passive, like a fixed resistor.
- Author: **pots join `resistor`, not a new kind.** The engine already gives a resistor's pairs the
  right passive behavior and lets a pair share a pin with another (bussed arrays), so a–w plus w–b
  is a pot with no engine change, and Simulation › Run stays enabled. A new facts-only kind would
  refuse the circuit (`"simulated": false`), so any hobby schematic with a volume pot would stop
  running.
- Author: **`"pot"` on the device, `"taper"` and `"turns"` on the part**: a datasheet series is
  one type, while taper and turns vary by ordering code (Bourns PTV09A `-A` / `-B` tapers).
- Author: **at most 2 gangs**, matching plan-78-A's scope.
- Author: **listing fields only for pots**, so every shipped golden line stays byte-identical in E.

## Corrections

## Summary

The resistor kind learns potentiometers: a wiper key that loads as two resistor links, and the
pot's type, taper and turns as facts, with tests and docs; no shipped part changes.
