# plan-44-E: Backend — the `crystal` kind, the clock model, `crystal_test`

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-44-D (E reads no census output; it follows D because letter
order is implementation order)

E adds the `crystal` part kind: a two-terminal frequency reference, either a
quartz crystal or a ceramic resonator (a resonator with built-in load
capacitors has a third, common pin). A crystal part carries its frequency and
facts (load capacitance, tolerance, stability, ESR). In the logic simulation
it is a **clock source on its own two terminals**: from the start it drives a
square wave at its frequency, weakly, `X1` at the level and `X2` at the
complement, so a clock input wired to either terminal counts it, and any
strong driver on a terminal overrides it. The behavioral outcome:
`crystal_test` passes the cases of § 6, `ctest` passes T0 + 1 programs, and
every existing golden is unchanged (no part uses the kind yet; F adds them).

References:

- `backend/src/models/oscillator.c` (the clock-source precedent: half period
  in picoseconds, `sim_schedule_timer`, `sim_schedule_drive`),
  `backend/src/models/capacitor.c` (the passive two-pin kind E mirrors for its
  loader and facts), `backend/src/models/resistor.c` (`inout` pins).
- `backend/include/parts.h` (`PartKind`, `PartCapacitor`, `Part`),
  `backend/src/library/kind.h` (`PartKindOps`, `kind_*` helpers),
  `backend/src/library/kinds.c` (`KINDS[]`), `backend/src/engine/sim.h`
  (`sim_schedule_drive` with `DRIVE_WEAK`, `sim_schedule_timer`).
- `docs/backend.md` § Adding a kind; `docs/parts-format.md` § Kinds
  (`oscillator`, `capacitor`, `resistor`).
- `planning/completed/plan-33-E-capacitor-kind.md` and
  `planning/completed/plan-34-E-resistor-kind.md` (the kind precedents).

## Prerequisites

plan-44-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-44-D complete | `ls planning/completed/plan-44-D-*.md` → exists | NOT MET (2026-11-16: not started) |
| No `crystal` kind yet | `grep -c 'PART_CRYSTAL' backend/include/parts.h` → 0; `ls backend/src/models/crystal.c` → no such file | MET (2026-11-16 at 7108a13e: 0, no file) |
| Golden sizes re-measured | `git show HEAD:backend/tests/golden/parts.txt \| wc -l` (and `traces.txt`, `footprints.txt`) | record P0 / R0 / F0 at E's start, as of 7108a13e: 32897 / 32897 / 1646 (C's and D's footprints are not in `footprints.json` yet, F adds them) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `ctest --test-dir /tmp/logic-p44-e` → `100% tests passed` with T0 + 1
  programs (T0 = 36 at 7108a13e: `git show HEAD:backend/CMakeLists.txt | grep -c '^logic_test'`
  → 36; re-measure at E's start).
- `crystal_test` passes the § 6 cases.
- `backend/tests/golden/parts.txt`, `traces.txt`, `footprints.txt` are
  byte-identical (no part uses the kind yet).

### Non-goals (explicit constraints)

- No analog behavior: no motional model, no load-capacitance matching, no
  drive level, no start-up time, no frequency pulling or tolerance: the clock
  is exactly `hz`, from time 0. The facts (`loadPf`, `tolerancePpm`,
  `stabilityPpm`, `esrOhms`) are for the app and change nothing in the
  simulation.
- No discrete gate oscillator: a crystal across an inverter's input and
  output (a Pierce circuit with a feedback resistor) does not behave as on
  silicon: the resistor passes the output's weak level into the crystal's net,
  where it fights the crystal's weak clock. The supported circuit is a clock
  input wired to a terminal; this is documented in `docs/parts-format.md`.
- The crystal drives only weakly: it never shorts a net, and a strong driver
  on a terminal overrides it (`circuit_net_value` reads the strong value).
- No new engine hook and no engine change: the model uses
  `sim_schedule_drive` / `sim_schedule_timer` only.
- Existing kinds are unchanged: every circuit without a crystal resolves
  exactly as before (the goldens are the check).
- No part-editor form for the `crystal` kind (plan-44-G § Decisions).
- The voltage check has no crystal rule: no rating, no supply (a crystal has
  no `vcc`; its pins are `inout`, like a resistor's).

## 2. Current State

- `PartKind` ends with `PART_TRANSISTOR` (`backend/include/parts.h:29`);
  `TRACE_LAST_KIND` is `PART_TRANSISTOR` (`tests/trace_check.c:22`);
  `listing_check.c:50–51` size two arrays by `PART_TRANSISTOR + 1`;
  `tests/support/listing.c` prints every kind's name (`case PART_TRANSISTOR:
  return "transistor"` :39) and facts (`case PART_TRANSISTOR:` :170).
- `KINDS[]` (`backend/src/library/kinds.c:66–70`) lists the ops; the loader,
  the listing and the circuit API work from the kind table and the `Part`
  structs (`docs/backend.md` § Adding a kind).
- A model is a `SimComponentType`; `start`, `pin_changed` and `timer` may be
  NULL (`capacitor.c:CAPACITOR_MODEL` sets none; `sim.c:1127, 1325, 1342`
  guard each). The oscillator drives its output with `sim_schedule_drive(…,
  DRIVE_STRONG, 0)` and re-arms `sim_schedule_timer(…, half_period)`.
- A net with only a weak driver reads that driver's value, and a gate input on
  it reads it: `sim.c:resolve_cluster` ("A node reads its strong values if
  any, else its weak ones"); two disagreeing weak drivers read X. The pull-up
  part is the existing weak driver.
- `tests/support/read_file.c:load_parts_dir` loads every `*.json` of the
  `Parts` directory, so a new `crystal.json` (F) needs no list entry.

### Measured populations

| What | Count | Command |
|---|---|---|
| Existing part files (one per kind) | 31 | `ls schematical/schematical/Parts/*.json \| grep -vc 'footprints\|datasheets'` → 31 |
| Test programs T0 | 36 | `git show HEAD:backend/CMakeLists.txt \| grep -c '^logic_test'` → 36 |
| Golden sizes P0 / R0 / F0 | 32897 / 32897 / 1646 | `git show HEAD:backend/tests/golden/{parts,traces,footprints}.txt \| wc -l` |
| Kinds in `PartKind` | 31 (`PART_GATE` … `PART_TRANSISTOR`) | `awk '/PART_GATE/,/} PartKind;/' backend/include/parts.h \| grep -o 'PART_[A-Z0-9_]*' \| wc -l` → 31 |

### Verified properties

- **A weak-only net is readable by logic.** Probe (`/tmp/p44-probe/probe.c`,
  linked against `/tmp/logic-p44/liblogic.a`): `PULLUP-5V` and the input pin 1
  of an `SN74HC04N` on one net, VCC on pin 14 and GND on pin 7 → after 1 µs
  `in=1 out=0` (the weak 1 is read, the inverter inverts it).
- **`kind_pin` rejects a missing or duplicate name** (`kind.h`: "an error
  when there is none or several"), so the two terminals named in the device
  are found once each; other pins may share a name (`NC`).
- **A pin's `inout` role without `vcc` is not rated or driven by the voltage
  check** (`voltage.c` only raises a level for an `inout` / `output` pin of a
  part with a power pin, and checks pins against `part->vcc` when not NULL;
  the resistor kind has the same shape).

## 3. Design

### 3.1 The kind

`backend/src/models/crystal.c`, `CRYSTAL_KIND` (`"crystal"`), file
`Parts/crystal.json` (F). `parts.h`:

```c
PART_CRYSTAL                       /* appended to PartKind, after the last kind */
typedef struct {
    const char *type;              /* "crystal" or "resonator" (a ceramic resonator) */
    uint64_t hz;                   /* the frequency, 1 .. 1e9 */
    uint64_t load_ff;              /* load capacitance, femtofarads; 0 = not given (series resonant) */
    uint32_t tolerance_ppm, stability_ppm;   /* 0 = not given */
    uint64_t esr_mohm;             /* ESR (a resonator: impedance), milliohms; 0 = not given */
    int capacitors;                /* 1 = a resonator with built-in load capacitors */
} PartCrystal;
/* Part: const PartCrystal *crystal; */
```

Device keys:

| Key | Value |
|---|---|
| `"type"` | `"crystal"` or `"resonator"`; anything else is `unknown crystal type "X"` |
| `"terminals"` | the two terminal pin names, `["X1", "X2"]` order = clock level / complement (`"terminals" must name 2 pins`) |

Every pinout names each terminal once (`kind_pin`), both `inout`
(`crystal pin "X" must be inout`); every other pin is `nc` (`crystal pin "X"
must be nc`); the pinout has 2 to 4 pins (`a crystal has 2 to 4 pins`).

Part keys (`"hz"` required, the rest optional):

| Key | Value |
|---|---|
| `"hz"` | whole hertz, 1 to 1000000000 (`"hz" must be a whole number from 0 to 1000000000`, `"hz" must be at least 1`) |
| `"loadPf"` | picofarads, above 0 and at most 1000 (`"loadPf" must be above 0 and at most 1000`); `load_ff` = llround(pF × 1000) |
| `"tolerancePpm"`, `"stabilityPpm"` | ppm, above 0 and at most 100000 (`"tolerancePpm" must be above 0 and at most 100000`) |
| `"esrOhms"` | ohms, above 0 and at most 1000000; `esr_mohm` = llround(Ω × 1000) |
| `"capacitors"` | `true` for a resonator with built-in load capacitors; `"capacitors" is only for a resonator` on a crystal |

### 3.2 The clock model

`CRYSTAL_MODEL`: `create` returns a state with the two terminal pin indices
(`part_internal(part)->bound`), `half_period = (1000000000000ULL + hz) / (2 *
hz)` (the oscillator's rounding), `level = LOGIC_0`. `start` drives
`X1` = level and `X2` = complement with `sim_schedule_drive(…, DRIVE_WEAK,
0)` and arms a timer `half_period`; `timer` flips the level, drives again and
re-arms. No `pin_changed`. Both terminals are driven because a crystal is
symmetric: which of its two pins reaches a chip's clock input is the user's
wiring, so either one must carry a clock; the complement on the other
terminal keeps the crystal a single two-terminal source rather than two
independent ones. The `Part` facts are not read by the model.

Decision on weakness: a strong drive would short against a chip output wired
to the same net (`serial` XTAL2 pins never drive, but a gate would); a weak
drive loses to any strong driver and ties disagreeing weak drivers to X, as a
pull does.

Where the correctness risk concentrates: nowhere in the engine (no engine
code changes). The risk is the golden churn F causes: every crystal part adds
a `trace_check` line whose hash covers the clock, so the model's timing
(`half_period` rounding, the 0 start level) is frozen into 600+ goldens;
`crystal_test` pins the exact values first.

Probes before writing: the weak-only net probe above. There are no unproven
premises left.

### 3.3 Byte-identity

This is **not** a byte-identity plan: it adds a kind and a model. The
goldens are expected unchanged only because no part uses the kind yet; a diff
in them is a bug to root-cause (`listing_check --print | diff` one line),
never a stop.

Rejected alternatives: (1) a strong clock on one terminal (rejected: the
terminals are interchangeable, so a user wiring the "other" terminal to the
clock input would get nothing); (2) a clock only on a terminal whose net has
another part's pin (rejected: the model sees only its own pins, and adding a
net-inspection hook to the engine is a bigger change than the whole kind);
(3) making the chips with XTAL pins read the crystal's frequency by name
(rejected: it couples every serial and RTC model to a new pin role).

## 4. Docs

`docs/parts-format.md`: the first paragraph's kind list and the kinds table
get `crystal.json` / `crystal`; a `### crystal` section after `### oscillator`
(device keys, part keys, the clock model, the Pierce limitation, the 4-pad
and 3-pad pin rules of plan-44-B § 3.3). `docs/backend.md`: § Adding a
kind's list is unchanged; the model list (`models/` section, where
`oscillator.c` and `transistor.c` are listed) gets `crystal.c`; § Tests gets
the `crystal_test` line.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The kind: loader, facts, tests of the loader

- [ ] `backend/include/parts.h`: `PART_CRYSTAL` after the current last
      `PartKind` (`grep -n 'PART_' backend/include/parts.h`; if plan-42 landed,
      after `PART_REGULATOR`), `PartCrystal`, `const PartCrystal *crystal;` on
      `Part`.
- [ ] `backend/src/models/crystal.c`: `CRYSTAL_KIND` with `load_device`,
      `bind_pinout` (the two terminal indices as `size_t[2]`, the pin rules of
      § 3.1), `load_part` (`out->crystal`), no model yet (the passive shape of
      `capacitor.c`: `.name`, `.pin_count`, `.create` returning the `Part`'s
      `crystal`).
- [ ] `backend/src/library/kinds.c`: `extern const PartKindOps CRYSTAL_KIND;`
      and `&CRYSTAL_KIND` in `KINDS[]` (`grep -c 'CRYSTAL_KIND'
      backend/src/library/kinds.c` → 0 before, 2 after); `backend/CMakeLists.txt`:
      `src/models/crystal.c` in `logic` and `logic_test(crystal_test)`.
- [ ] `backend/tests/support/listing.c`: `case PART_CRYSTAL: return "crystal"`
      and the facts line `type=%s hz=%llu load=%llu tol=%u stab=%u esr=%llu
      cap=%d`; `tests/trace_check.c` `TRACE_LAST_KIND` and
      `tests/listing_check.c` the two array sizes → `PART_CRYSTAL` (the last
      kind at that time; moot if already past it — `grep -c 'PART_CRYSTAL'
      backend/tests/trace_check.c backend/tests/listing_check.c` → 1 each).
- [ ] `backend/tests/support/crystal_fixture.h` and `backend/tests/crystal_test.c`
      fact cases and rejection cases (§ 6 a–j).

Acceptance: the kind loads and reads back; goldens unchanged.
  Check: `cmake -S backend -B /tmp/logic-p44-e && cmake --build /tmp/logic-p44-e -j8 && /tmp/logic-p44-e/crystal_test` → every line `PASS`, and `/tmp/logic-p44-e/listing_check` and `/tmp/logic-p44-e/trace_check` → exit 0 (est. 4 min: a build from scratch).
Commit: —

### Phase 2 — The clock model

- [ ] `crystal.c`: `CRYSTAL_MODEL` per § 3.2; the clock cases of § 6 k–p in
      `crystal_test.c`.

Acceptance: the clock cases pass; goldens unchanged.
  Check: `cmake --build /tmp/logic-p44-e -j8 && /tmp/logic-p44-e/crystal_test` → every line `PASS`; `/tmp/logic-p44-e/listing_check` and `/tmp/logic-p44-e/trace_check` → exit 0 (est. 2 min).
Commit: —

### Phase 3 — Docs

- [ ] `docs/parts-format.md` and `docs/backend.md` per § 4.

Acceptance: the docs name the kind.
  Check: `grep -c 'crystal' docs/parts-format.md docs/backend.md` → both above 0, and `grep -n '^### `crystal`' docs/parts-format.md` → 1 line (est. < 1 min).
Commit: —

## 6. `crystal_test` cases

Fixture (`crystal_fixture.h`, loaded next to the real part files): a
`crystal` device `"type": "crystal"` with pinouts `two-pin` (`1=X1`, `2=X2`)
and `four-pad` (`1=X1`, `2=NC`, `3=X2`, `4=NC`), parts `TESTXTAL-1M` (1 MHz,
`loadPf` 18, `tolerancePpm` 20, `stabilityPpm` 30, `esrOhms` 80),
`TESTXTAL-32K` (32768 Hz, four-pad) and `TESTXTAL-FAST` (100000000 Hz); a
`resonator` device with the `three-pad` pinout (`1=X1`, `2=GND`, `3=X2`) and
`TESTRES-8M` (8 MHz, `capacitors` true).

Facts and loader:

- (a) `TESTXTAL-1M`: kind `PART_CRYSTAL`, 2 pins, both `PIN_INOUT`, `vcc`
  NULL, `type` "crystal", `hz` 1000000, `load_ff` 18000, tolerance 20,
  stability 30, `esr_mohm` 80000, `capacitors` 0.
- (b) `TESTXTAL-32K`: 4 pins, pins 2 and 4 `PIN_NC`, `hz` 32768.
- (c) `TESTRES-8M`: `type` "resonator", 3 pins, pin 2 named `GND` `PIN_NC`,
  `capacitors` 1.
- (d) a part with no `"hz"` is rejected with `missing "hz"`; `"hz": 0` → `"hz" must be at least 1`; `"hz": 2000000000` →
  `"hz" must be a whole number from 0 to 1000000000`.
- (e) `"type": "quartz"` → `unknown crystal type "quartz"`; `"terminals"`
  with 1 name → `"terminals" must name 2 pins`.
- (f) a terminal pin with role `input` → `crystal pin "X1" must be inout`; a
  non-terminal pin with role `inout` → `crystal pin "NC" must be nc`; a
  pinout with 5 pins → `a crystal has 2 to 4 pins`.
- (g) `"loadPf": 0` → `"loadPf" must be above 0 and at most 1000`;
  `"tolerancePpm": 200000` → `"tolerancePpm" must be above 0 and at most
  100000`.
- (h) `"capacitors": true` on a `crystal` device → `"capacitors" is only for a
  resonator`.
- (i) a part key the kind does not list (`"volts": 5`) → `unknown key "volts"`.
- (j) the fixture parts name library footprints with the right pad counts
  (`RES-0402` for the two-pin parts, `OSC-3215-4` for the four-pad,
  `SOT23-3` for the three-pad: `python3 -I` over `footprints.json` → pins 2,
  4, 3); a part naming a footprint with another pad count is rejected by the
  loader's `has N pads, pinout "…" has M pins` rule.

Clock (in `circuit_*` rigs, times in picoseconds):

- (k) `TESTXTAL-1M` with X1 on net A and X2 on net B, nothing else: after
  100 ns A = 0 and B = 1; after 600 ns A = 1 and B = 0; after 1100 ns A = 0
  and B = 1.
- (l) `TESTXTAL-32K`: half period `(1000000000000 + 32768) / 65536` =
  15258789 ps; at 15 µs A = 0, at 16 µs A = 1 (the 0.26 µs margin), at 31 µs
  A = 0 (period 30.517578 µs).
- (m) the same crystal with a `GND` supply on net A: A reads 0 at every
  time (strong beats weak) and B still toggles.
- (n) crystal X1 on the input pin of an `SN74HC04N` (VCC 5 V, GND wired):
  the inverter's output is the complement of A one gate delay after each
  toggle; the output net's `circuit_net_changes` after 10 µs of the 1 MHz
  crystal is within 1 of 20 (`circuit_net_changes` counts the output's
  changes).
- (o) `TESTXTAL-FAST` (100 MHz): half period 5000 ps; the run to 1 µs ends
  with `circuit_error` NULL and A toggled 200 times (`circuit_net_changes`
  200 ± 1).
- (p) a crystal with neither terminal wired starts and advances 1 µs without
  an error (`circuit_start` 0, `circuit_error` NULL).

## Validation Plan

- Tests: `crystal_test` (cases a–p, including the rejections); `library_check`
  has no crystal case of its own (the loader's key and range rules are
  `crystal_test`'s).
- Coverage check: `crystal.c` is in the `logic` library, so every test that
  loads the library covers it; `crystal_test` runs every function of it
  (`gcov` is not part of the project; the cases above hit each branch of
  `load_device`, `bind_pinout`, `load_part`, `create`, `start`, `timer`).
- Runtime proof: case (n), a crystal clocking a real logic gate.
- Doc sync: § 4.
- Final gate (run ONCE, in plan-44-H): the landing tier.

## Decisions

Owner decision (asked once, 2026-11-16, before this plan was written):

- **A crystal is a simulated clock source** ("Simulated clock from a
  crystal"): wired to a clock input it drives play mode.

Author decisions (no user-visible fork; recorded so nobody re-litigates):

- **Weak, both terminals** (§ 3.2): a strong or single-terminal clock was
  rejected in § 3.3.
- **The start level is 0 at time 0**, like the oscillator part: the traces and
  the goldens are reproducible, and a clock input wired to X1 sees a rising
  edge at `half_period`.
- **`hz` up to 1 GHz** in the loader (the catalog only reaches 76.8 MHz,
  A § Decisions caps a pick at 100 MHz): the half period at 1 GHz is 500 ps,
  still a whole picosecond count.
- **`PartCrystal` facts are integers in the oscillator's and capacitor's
  units** (hertz, femtofarads, parts per million, milliohms), so the app reads
  them without floats.

## Corrections

## Summary

E has no engine risk (no engine file changes) and one golden risk: the model
fixes the clock's timing into the traces F records. Untouched: the engine,
every other kind, the voltage check, the circuit API and the app.
