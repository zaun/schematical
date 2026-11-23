# plan-47-I: Backend — the `inductor` and `fuse` kinds, fixed contacts, the blowable fuse

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-47-H (I reads no census output; it follows H because letter
order is implementation order)

I adds two part kinds. An **inductor** is a two-terminal part that conducts
like a wire in the logic simulation: its two terminals are joined by an
engine contact that is closed from the start and never opens. A **fuse**
(`"type": "fuse"`) and a **PTC resettable fuse** (`"type": "ptc"`) are the
same, with one difference: the contact is a switch position, so
`circuit_set_switch(c, part, 1, 0)` opens it (the fuse is blown) and
`…, 1, 1)` closes it again. Both kinds carry facts for the app (inductance,
currents, ratings). The voltage check treats both as a closed contact (their
two nets are one group), and refuses a fuse pin above the part's DC voltage
rating. The behavioral outcome: `inductor_test` and `fuse_test` pass the
cases of § 6, `ctest` passes T0 + 2 programs, and every existing golden is
unchanged (no part uses the kinds yet; K adds them).

References:

- `backend/src/models/transistor.c` (`transistor_start`: contacts closed in
  `start` with `sim_set_contact`), `backend/src/models/switch.c`
  (`set_switch`, `switch_contact_pins`), `backend/src/models/capacitor.c` (a
  passive two-pin kind's loader and facts, the rating test).
- `backend/src/engine/sim.h` (`sim_set_contact`: "the nets on those pins act
  as one net … strong drivers that disagree across it short"; the
  `set_switch` hook), `backend/src/circuit.c:313` (`circuit_set_switch`),
  `backend/src/voltage.c:139–165` (step 1, the groups of closed contacts),
  `:501–521` (the rating tests), `:568` (the message texts).
- `backend/include/parts.h`, `backend/src/library/kind.h`, `kinds.c`,
  `backend/src/models/model_pins.h`.
- `docs/backend.md` § Adding a kind; `docs/parts-format.md` § Kinds.
- `planning/completed/plan-33-E-capacitor-kind.md`,
  `planning/completed/plan-40-F-transistor-kind.md` (the precedents).

## Prerequisites

plan-47-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-47-H complete | `ls planning/completed/plan-47-H-*.md` → exists | NOT MET as written (2026-11-22: H not started: it waits on C's final pick, and C on A's four quota days). I's header says it reads no census output and follows H only because letter order is implementation order, so I is worked out of order while A's Phase 2 waits for DigiKey's quota to reset; § Corrections |
| No `inductor` or `fuse` kind yet | `grep -c 'PART_INDUCTOR\|PART_FUSE' backend/include/parts.h` → 0; `ls backend/src/models/inductor.c backend/src/models/fuse.c` → no such file | MET (2026-11-22 at 7c7b4d31: 0, no file; `PartKind` ends `PART_REGULATOR, PART_CRYSTAL`, I appends after them) |
| Golden sizes and test count re-measured | `git show HEAD:backend/tests/golden/parts.txt \| wc -l` (and `traces.txt`, `footprints.txt`); `git show HEAD:backend/CMakeLists.txt \| grep -c '^logic_test'` | record P0 / R0 / F0 / T0 at I's start; at 349f25b3: 32897 / 32897 / 1646 / 37; at 7c7b4d31 (2026-11-22): **34375 / 34375 / 1895 / 41** |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `ctest --test-dir /tmp/logic-p47-i` → `100% tests passed` with T0 + 2
  programs (T0 = 37 at 349f25b3).
- `inductor_test` and `fuse_test` pass the § 6 cases.
- `backend/tests/golden/parts.txt`, `traces.txt` and `footprints.txt` are
  byte-identical (no part uses the kinds yet).

### Non-goals (explicit constraints)

- No analog behavior: no inductance, no current, no energy, no filter, no
  flyback; a fuse never blows by itself (there is no current in the logic
  simulation) and a PTC never trips. The facts change nothing in the run.
- No new engine function and no change to `sim.c` / `net.c`: the models use
  `sim_set_contact` and the existing `set_switch` hook.
- `circuit_set_switch`'s contract is unchanged for switches; a fuse answers
  it for position 1 only (`"<name> has no position 2"`), an inductor does
  not (`part N is not a switch`).
- The schematic file format is unchanged: a blown fuse is play-mode state
  (plan-47-M), never saved.
- Existing kinds are unchanged: every circuit without an inductor or fuse
  resolves exactly as before (the goldens are the check).
- No voltage rule for an inductor (it has no voltage rating in the catalog
  data), no current rule for either kind.

## 2. Current State

- `PartKind` ends with `PART_GENERIC` (`backend/include/parts.h:30`);
  `TRACE_LAST_KIND` is `PART_GENERIC` (`tests/trace_check.c:22`);
  `tests/listing_check.c:50–51` size two arrays by `PART_GENERIC + 1`;
  `tests/support/listing.c:40, :171` name and print each kind. Open plans
  append other kinds (plan-42 `PART_REGULATOR`, plan-44 `PART_CRYSTAL`): I
  appends after whatever is last at its start.
- `KINDS[]` (`backend/src/library/kinds.c`) ends `&TRANSISTOR_KIND,
  &GENERIC_KIND, NULL`.
- A contact is an engine primitive (`sim.h:253–271`): `sim_set_contact(sim,
  instance, pin_a, pin_b, closed)`; before `sim_start` the state is set at
  once, contacts start open, a pin with no net is left out.
  `transistor_start` closes its same-named-pin ties this way (`transistor.c:439`).
- `circuit_set_switch` (`circuit.c:313`) calls the model's `set_switch` hook,
  fails with `part %zu is not a switch` when the hook is NULL and with `%s
  has no position %u` when the hook returns 0, then records the position in
  `circuit->closed` and re-runs the voltage check when a contact closes
  during a run.
- The voltage check's step 1 (`voltage.c:139–165`) unions the nets of each
  `VoltageContact` in `closed` through `switch_contact_pins`, which answers
  only for `PART_SWITCH` (`switch.c:192`); an entry of another kind is
  skipped (`continue`). The app passes its own `closed` list
  (`SchematicNetlist.switchesOn`, `Simulation/Simulation.swift:27`), built
  from switches only.
- `voltage.h:27–32` lists the six `VoltageProblemKind` values; no Swift file
  names one (`grep -rn 'VOLTAGE_CAPACITOR_OVER' --include='*.swift' schematical` → 0),
  the app shows `voltage_problem_text`.

### Measured populations

| What | Count | Command |
|---|---|---|
| Test programs T0 | 37 | `grep -c '^logic_test' backend/CMakeLists.txt` → 37; `ctest --test-dir /tmp/logic-p47` → `100% tests passed, 0 tests failed out of 37` (2026-11-16 at 349f25b3) |
| Golden sizes P0 / R0 / F0 | 32897 / 32897 / 1646 | `wc -l backend/tests/golden/{parts,traces,footprints}.txt` |
| Kinds in `PartKind` | 32 | `awk '/PART_GATE/,/} PartKind;/' backend/include/parts.h \| grep -o 'PART_[A-Z0-9_]*' \| wc -l` (re-measure at I's start) |
| `VoltageProblemKind` values | 6 | `grep -c '^    VOLTAGE_' backend/include/voltage.h` → 6 |
| Callers of `switch_contact_pins` | 1 outside `switch.c` (`voltage.c:149`) | `grep -rn 'switch_contact_pins' backend/src` |

### Verified properties

- **A closed contact makes two nets one**, both ways, and shorts on
  disagreeing strong drivers (`sim.h:253–259`); `switch_test` and
  `transistor_test` exercise it. An inductor between a gate output and an
  LED therefore lights the LED, and an inductor across 5V and GND is
  `smoke:`.
- **`set_switch` works before and after start** (`circuit.h:44–48`), and
  `sim_set_contact` before `sim_start` sets the state at once (`sim.h:260`):
  a fuse blown before start starts open.
- **A `closed` entry for a part that is no switch is ignored by the voltage
  check** (`voltage.c:148–151`: `!switch_contact_pins(…)` → `continue`), so
  `circuit_set_switch` recording a fuse's position 1 does not disturb it.

## 3. Design

### 3.1 The kinds

`parts.h` (each appended after the last kind present; each only if absent):

```c
PART_INDUCTOR, PART_FUSE
typedef struct {
    uint64_t ph;                 /* inductance, picohenries (100 pH .. 100 H) */
    const char *tolerance;       /* as the datasheet prints it ("±20%", "±0.1nH"), NULL = not given */
    uint32_t rated_ma, isat_ma;  /* rated (heating) and saturation current; 0 = not given */
    uint64_t dcr_uohm;           /* DC resistance, max, micro-ohms; 0 = not given */
    const char *shielding;       /* "shielded", "semi-shielded", "unshielded", NULL = not given */
} PartInductor;
typedef struct {
    const char *type;            /* "fuse" or "ptc" */
    uint32_t rated_ma;           /* a fuse's rated current; a PTC's hold current */
    uint32_t trip_ma;            /* a PTC's trip current; 0 for a fuse */
    uint32_t ac_mv, dc_mv;       /* voltage ratings; a PTC's maximum voltage is dc_mv; 0 = not given */
    const char *speed;           /* a fuse: "fast", "medium" or "slow"; NULL for a PTC */
    uint32_t breaking_a;         /* a fuse's breaking capacity, a PTC's maximum current, amps; 0 = not given */
} PartFuse;
/* Part: const PartInductor *inductor; const PartFuse *fuse; */
```

Both kinds share the pin rules: the device key `"terminals"` names the two
terminal pins (`"terminals" must name 2 pins`, two different names); each
name names at least one pin of every pinout; every pin so named is `inout`
(`inductor pin "X" must be inout`); every other pin is `nc` (`inductor pin
"X" must be nc`; a shielded inductor's dummy pads, a holder's mounting
pins); the first pin of each name is the contact's end, and further pins of
the same name (a four-pad inductor with two pads per terminal) are tied to
it by contacts closed in `start`, as a transistor's tab is. The messages say
`fuse pin` for the fuse kind.

`inductor` (`backend/src/models/inductor.c`, `INDUCTOR_KIND`, file
`Parts/inductor.json`): device key `"terminals"`. Part keys:

| Key | Value |
|---|---|
| `"henries"` | required; above 0 and at most 100 (`"henries" must be above 0 and at most 100`); `ph` = llround(H × 1e12), at least 1 |
| `"tolerance"` | optional string, as the datasheet prints it |
| `"amps"`, `"isat"` | optional; above 0 and at most 1000 (`"amps" must be above 0 and at most 1000`) |
| `"dcr"` | optional; ohms, above 0 and at most 100000 |
| `"shielding"` | optional; `"shielded"`, `"semi-shielded"` or `"unshielded"` (`unknown shielding "X"`) |

`fuse` (`backend/src/models/fuse.c`, `FUSE_KIND`, file `Parts/fuse.json`):
device keys `"type"` (`"fuse"` or `"ptc"`; `unknown fuse type "X"`) and
`"terminals"`. Part keys:

| Key | Types | Value |
|---|---|---|
| `"amps"` | `fuse` (required) | rated current, above 0 and at most 1000 |
| `"speed"` | `fuse` (required) | `"fast"`, `"medium"` or `"slow"` (`unknown fuse speed "X"`) |
| `"voltsAc"`, `"voltsDc"` | `fuse` (at least one: `a fuse needs "voltsAc" or "voltsDc"`) | above 0 and at most 100000 |
| `"breaking"` | `fuse` (optional) | breaking capacity in amps, above 0 and at most 1000000 |
| `"hold"`, `"trip"` | `ptc` (required) | amps, above 0 and at most 1000; `trip` above `hold` (`"trip" must be above "hold"`) |
| `"volts"` | `ptc` (required) | maximum voltage, above 0 and at most 100000 |
| `"imax"` | `ptc` (optional) | maximum fault current in amps, above 0 and at most 1000000 |

A key of the other type is an error (`"hold" is only for a ptc`, `"amps" is
not a ptc key`), the diode kind's rule.

### 3.2 The models

`INDUCTOR_MODEL`: `create` returns the bound pins; `start` closes the
contact between the two terminals and every same-name tie
(`sim_set_contact(…, 1)`); no `pin_changed`, no `timer`, no `set_switch`.

`FUSE_MODEL`: the same, plus a state byte `intact` (1 at `create`) and the
`set_switch` hook: position 1 sets `intact = on` and
`sim_set_contact(terminal a, terminal b, on)`; any other position returns 0.
`start` applies `intact` (so a fuse blown before start starts open) and
closes the ties. A blown fuse's ties stay closed: the two pads of one
terminal are one piece of metal.

### 3.3 The voltage check

`model_pins.h` gains

```c
/* A part's k-th fixed contact (0-based): two pins joined inside the part
   whatever the circuit does: an inductor's and a fuse's terminals and
   same-name ties (plan-47-I); a relay's normally-closed contacts and ties
   (plan-47-J). 0 past the last, and for every other kind. */
int part_fixed_contact(const Part *part, size_t k, size_t *a, size_t *b);
```

implemented in a new `backend/src/models/model_pins.c` that dispatches on
`part->kind` to `inductor_fixed_contact` / `fuse_fixed_contact` (each in its
model file, declared in `model_pins.h`). `voltage.c` step 1, after the loop
over `closed`, unions the nets of every part's fixed contacts (the same
inner loops, over `parts[i]` and `part_fixed_contact(parts[i], k, &a, &b)`).
A fuse is closed for the voltage check whether it is blown or not: the check
answers "may this board be powered", and a blown fuse is replaced.

The rating test: `VOLTAGE_FUSE_OVER` appended to `VoltageProblemKind`
(`/* fuse or PTC pin above its DC voltage rating (plan-47-I) */`). In the
chain at `voltage.c:501–521`: `part->kind == PART_FUSE && part->fuse != NULL
&& limit > 0 && e.mv > limit`, `limit` = `dc_mv` when given, else `ac_mv`
(a fuse rated only for AC is held to the same number); either terminal pin,
by the capacitor's reasoning (`voltage.c:507–510`). The text joins the
`VOLTAGE_CAPACITOR_OVER` case (`voltage.c:568`): `voltage: 12 V on pin 1 (1)
of TESTPTC; it is rated 6 V`.

Where the correctness risk concentrates: step 1 of the voltage check now
runs for every part of every circuit. `part_fixed_contact` returns 0 at `k`
= 0 for every existing kind, so existing circuits take the old path; the
check is `voltage_test` unchanged and green, and the goldens.

### 3.4 Byte-identity

This is **not** a byte-identity plan. The goldens are expected unchanged
only because no part uses the kinds yet; a diff is a bug to root-cause
(`listing_check --print | diff` one line), never a stop.

Rejected alternatives: (1) an inductor as a pair of two-way diode links
(rejected: the fuse needs a contact it can open, and one mechanism for both
kinds keeps one test bench); (2) a resistor-like weak link for the inductor
(rejected: a gate could then not drive an LED through an inductor against a
pull, and the owner asked for "conducts like a wire"); (3) recording the
fixed contacts in `circuit->closed` at `circuit_add_part` (rejected: the
app's own voltage check passes its own `closed` list and would not see
them; the check must find them from the part).

## 4. Docs

`docs/parts-format.md`: the kind list and table get `inductor.json` /
`inductor` and `fuse.json` / `fuse`; `### inductor` and `### fuse` sections
after `### resistor` (keys, pin rules, the contact model, the blowable
position, the rating test, what is not modeled). `docs/backend.md`: § Using
it gains an **Inductors and fuses** bullet after **Transistors** (the fixed
contact, `circuit_set_switch` on a fuse, the voltage group); the `models/`
list gets `inductor.c`, `fuse.c`, `model_pins.c`; § Tests gets the two test
lines; the voltage-check paragraph (`:190–192`) names the fuse rating.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The inductor kind

- [x] (2026-11-22: `grep -c 'PART_INDUCTOR' backend/include/parts.h` was 0 before; now `PART_INDUCTOR, PART_FUSE` after `PART_CRYSTAL`) `backend/include/parts.h`: `PART_INDUCTOR` after the current last
      `PartKind` (`grep -n 'PART_' backend/include/parts.h`), `PartInductor`,
      `const PartInductor *inductor;` on `Part` (each only if absent:
      `grep -c 'PART_INDUCTOR' backend/include/parts.h` → 0 before).
- [x] (2026-11-22: `inductor.c` with the shared pin rules in `terminals.c` / `terminals.h`) `backend/src/models/inductor.c`: `INDUCTOR_KIND` (`load_device`,
      `bind_pinout`, `load_part`), `INDUCTOR_MODEL` (§ 3.2),
      `inductor_fixed_contact`.
- [x] (2026-11-22) `backend/src/models/model_pins.c` and `model_pins.h`:
      `part_fixed_contact` (§ 3.3); `backend/src/library/kinds.c`:
      `&INDUCTOR_KIND` in `KINDS[]`; `backend/CMakeLists.txt`: the two
      sources in `logic` and `logic_test(inductor_test)`.
- [x] (2026-11-22: `TRACE_LAST_KIND` was `PART_CRYSTAL`, now `PART_FUSE`; the two `listing_check.c` arrays likewise) `backend/tests/support/listing.c`: `case PART_INDUCTOR: return
      "inductor"` and the facts line `ph=%llu tol=%s amps=%u isat=%u
      dcr=%llu shield=%s`; `tests/trace_check.c` `TRACE_LAST_KIND` and
      `tests/listing_check.c`'s two array sizes → the last kind (moot if a
      later kind is already there: `grep -n 'TRACE_LAST_KIND'
      backend/tests/trace_check.c`).
- [x] (2026-11-22: step 1 now uses a `join_pins` helper for the switches and the fixed contacts) `backend/src/voltage.c` step 1: the fixed-contact unions (§ 3.3).
- [x] (2026-11-22: `inductor_test` all PASS, every § 6 a–h case) `backend/tests/support/inductor_fixture.h`, `backend/tests/inductor_test.c`
      (§ 6 a–h).

Acceptance: the inductor loads, conducts and joins its nets in the voltage check; goldens unchanged.
  Check: `cmake -S backend -B /tmp/logic-p47-i && cmake --build /tmp/logic-p47-i -j8 && /tmp/logic-p47-i/inductor_test && /tmp/logic-p47-i/voltage_test` → every line `PASS`, and `/tmp/logic-p47-i/listing_check && /tmp/logic-p47-i/trace_check` → exit 0 (est. 4 min: a build from scratch).
Commit: 1c9f6088

### Phase 2 — The fuse kind

- [x] (2026-11-22) `parts.h`: `PART_FUSE`, `PartFuse`, `const PartFuse *fuse;` (only if
      absent); `backend/src/models/fuse.c`: `FUSE_KIND`, `FUSE_MODEL` with
      `set_switch` (§ 3.2), `fuse_fixed_contact`; `model_pins.c` dispatch;
      `kinds.c`; `CMakeLists.txt` (`fuse.c`, `logic_test(fuse_test)`).
- [x] (2026-11-22) `listing.c`: `case PART_FUSE: return "fuse"` and the facts line
      `type=%s amps=%u trip=%u ac=%u dc=%u speed=%s break=%u`; the last-kind
      lines of `trace_check.c` and `listing_check.c`.
- [x] (2026-11-22) `voltage.h`: `VOLTAGE_FUSE_OVER`; `voltage.c`: the rating branch and
      the text case (§ 3.3).
- [x] (2026-11-22: `fuse_test` all PASS, every § 6 i–t case, for the fuse and the PTC) `backend/tests/support/fuse_fixture.h`, `backend/tests/fuse_test.c`
      (§ 6 i–t).

Acceptance: the fuse loads, conducts, blows and restores; goldens unchanged.
  Check: `cmake --build /tmp/logic-p47-i -j8 && /tmp/logic-p47-i/fuse_test && /tmp/logic-p47-i/voltage_test` → every line `PASS`; `/tmp/logic-p47-i/listing_check && /tmp/logic-p47-i/trace_check` → exit 0 (est. 2 min).
Commit: 1c9f6088

### Phase 3 — Docs

- [x] (2026-11-22: `grep -c '^### `inductor`\|^### `fuse`' docs/parts-format.md` → 2, `grep -c 'inductor_test\|fuse_test' docs/backend.md` → 2) `docs/parts-format.md` and `docs/backend.md` per § 4.

Acceptance: the docs name both kinds.
  Check: `grep -c '^### `inductor`\|^### `fuse`' docs/parts-format.md` → 2 and `grep -c 'inductor_test\|fuse_test' docs/backend.md` → 2 (est. < 1 min).
Commit: f08a6bd6

## 6. Test cases

Fixtures (loaded next to the real part files). `inductor_fixture.h`: an
`inductor` device with pinouts `two-pin` (`1`, `2`) and `four-pad` (`1=1`,
`2=1`, `3=2`, `4=2`), `"terminals": ["1","2"]`; parts `TESTIND-10U` (10 µH,
`"tolerance": "±20%"`, `amps` 1.2, `isat` 1.5, `dcr` 0.15, `shielding`
`shielded`), `TESTIND-1N` (1 nH, two-pin) and `TESTIND-4PAD`. `fuse_fixture.h`:
a `fuse` device and a `ptc` device, two-pin; parts `TESTFUSE-1A` (`amps` 1,
`speed` `fast`, `voltsAc` 250, `voltsDc` 125, `breaking` 35) and `TESTPTC`
(`hold` 0.5, `trip` 1, `volts` 6, `imax` 40). Every fixture part names a
library footprint with the right pad count (`RES-0603` for two pins;
`OSC-3215-4` for the four-pad).

`inductor_test`:

- (a) `TESTIND-10U`: kind `PART_INDUCTOR`, 2 pins, both `PIN_INOUT`, `vcc`
  NULL, `ph` 10000000, tolerance "±20%", `rated_ma` 1200, `isat_ma` 1500,
  `dcr_uohm` 150000, shielding "shielded". `TESTIND-1N`: `ph` 1000, the
  optional facts 0 / NULL.
- (b) rejections: no `"henries"` → `missing "henries"`; `"henries": 0` and
  `200` → `"henries" must be above 0 and at most 100`; `"shielding":
  "open"` → `unknown shielding "open"`; `"terminals"` with one name →
  `"terminals" must name 2 pins`; a terminal pin with role `input` →
  `inductor pin "1" must be inout`; an extra `inout` pin → `inductor pin "3"
  must be nc`; a part key the kind does not list (`"volts": 5`) → `unknown
  key "volts"`.
- (c) 5V on pin 1, nothing else on pin 2's net: pin 2's net reads 1 after
  1 µs; GND instead: 0.
- (d) an `SN74HC04N` output (VCC, GND wired, input on GND) on pin 1, pin 2
  to a `GENERIC-LED` anode, cathode to GND: the LED's light is on
  (`circuit_part_lights`); with the input on 5V it is off.
- (e) 5V on pin 1 and GND on pin 2: `circuit_advance` returns 0,
  `circuit_error` starts with `smoke:`.
- (f) `TESTIND-4PAD`: 5V on pad 2, a net on pad 3 reads 1 (the tie 1–2 and
  the contact); pads 1 and 4 with no net change nothing.
- (g) `circuit_set_switch(c, inductor, 1, 0)` → -1, `part N is not a
  switch`.
- (h) the voltage check: `12V` on pin 1 of `TESTIND-10U`, pin 2 on the VCC
  pin of a 5 V part (`SN74HC04N`) → `circuit_start` -1 with a
  `VOLTAGE_POWER_RANGE` problem naming the SN74HC04N (the inductor's two
  nets are one group); the same with 5V starts.

`fuse_test`:

- (i) `TESTFUSE-1A`: kind `PART_FUSE`, `type` "fuse", `rated_ma` 1000,
  `trip_ma` 0, `ac_mv` 250000, `dc_mv` 125000, `speed` "fast", `breaking_a`
  35. `TESTPTC`: `type` "ptc", `rated_ma` 500, `trip_ma` 1000, `dc_mv` 6000,
  `ac_mv` 0, `speed` NULL, `breaking_a` 40.
- (j) rejections: `"type": "breaker"` → `unknown fuse type "breaker"`; a
  fuse with no `"amps"` → `missing "amps"`; `"speed": "quick"` → `unknown
  fuse speed "quick"`; no voltage key → `a fuse needs "voltsAc" or
  "voltsDc"`; `"hold": 1` on a fuse → `"hold" is only for a ptc`; `"amps":
  1` on a PTC → `"amps" is not a ptc key`; `"trip"` ≤ `"hold"` → `"trip"
  must be above "hold"`.
- (k) intact: 5V on pin 1, pin 2's net reads 1; the LED circuit of (d) with
  the fuse in place of the inductor lights.
- (l) blown during a run: after `circuit_set_switch(c, fuse, 1, 0)` and
  1 µs, pin 2's net reads Z and the LED is off; after `…, 1, 1)` it reads 1
  and the LED is on.
- (m) blown before start: `circuit_set_switch(…, 1, 0)` then
  `circuit_start`: pin 2's net reads Z.
- (n) `circuit_set_switch(c, fuse, 2, 1)` → -1, `TESTFUSE-1A has no position 2`.
- (o) 5V and GND across an intact fuse: `smoke:`; across a blown one: the
  run goes on, both nets keep their values.
- (p) `TESTPTC` behaves as (k)–(o).
- (q) rating: `12V` on pin 1 of `TESTPTC` → `circuit_start` -1,
  `VOLTAGE_FUSE_OVER`, text `voltage: 12 V on pin 1 (1) of TESTPTC; it is
  rated 6 V`; `5V` starts.
- (r) `TESTFUSE-1A` on `12V` starts (125 V DC rating).
- (s) the voltage group: the (h) circuit with a fuse → the same
  `VOLTAGE_POWER_RANGE`; it is the same with the fuse blown (§ 3.3).
- (t) a fuse closing during a run (`…, 1, 1)`) with the (s) wiring on 5V
  does not stop the run (`circuit_advance` returns 1).

## Validation Plan

- Tests: `inductor_test` (a–h), `fuse_test` (i–t); `voltage_test`,
  `switch_test` and `transistor_test` unchanged and green.
- Coverage check: the cases above reach each branch of the two loaders,
  `start`, `set_switch`, `part_fixed_contact` and the new voltage branch.
- Runtime proof: cases (d) and (l), a logic gate lighting an LED through the
  part, and the LED going dark when the fuse is blown.
- Doc sync: § 4.
- Final gate (run ONCE, in plan-47-O): the landing tier.

## Decisions

Owner decisions (asked once, 2026-11-16, before this plan was written):

- **An inductor and an intact fuse or PTC conduct like a wire** in play mode.
- **A fuse can be blown and restored** in play mode ("Blowable fuse").

Author decisions (no user-visible fork; recorded so nobody re-litigates):

- **Two kinds, not one**: `inductor` and `fuse` have different facts and a
  different model (one has a position); fuses and PTC resettable fuses share
  the `fuse` kind with a `type`, as diodes share theirs.
- **The contact, not a link** (§ 3.4 rejected alternatives 1 and 2).
- **`circuit_set_switch` is the blow API**: position 1, ON = intact. No new
  circuit function, and the app's switch path carries it (plan-47-M).
- **A blown fuse is still closed for the voltage check** (§ 3.3).
- **`VOLTAGE_FUSE_OVER` uses the DC rating, else the AC one**: the library's
  supplies are DC, 0 to 30 V (`docs/parts-format.md` § `supply`); a 6 V PTC
  on a 12 V rail is the mistake the rule catches.
- **Facts are integers** in the capacitor's and resistor's style
  (picohenries, milliamps, micro-ohms, millivolts), so the app reads them
  without floats.

## Corrections

- **2026-11-22, order.** Letter I was worked before H (and before A's Phase 2
  finished) because A's quota days block the data letters and I's header says
  it reads no census output. The prerequisite row "plan-47-H complete" stays
  NOT MET and is not a dependency of the code.
- **2026-11-22, § 2 "Current State".** `PartKind` already ended
  `PART_REGULATOR, PART_CRYSTAL` (plans 42 and 44 landed); I appended
  `PART_INDUCTOR, PART_FUSE`. `voltage.c`'s step 1 is at line 268 (not
  139–165) and the rating chain at line 770 onwards: the fixed-contact unions
  and the `VOLTAGE_FUSE_OVER` branch went there; the switch loop and the new
  loop share a `join_pins` helper.
- **2026-11-22, case (l).** `circuit_part_lights` answers "on at any time
  during the last step" (`circuit_check` / `lights.h`), and a contact opens at
  the first instant of the next step, so the LED behind a fuse blown during a
  run still reports on for that step (`dbg`: `blown: anode 3 (Z) … led 1`).
  The test steps a second microsecond before it checks the LED is off; the
  net reads Z after the first. The app shows a light after each step, so the
  LED goes dark on the step after the click.
- **2026-11-22, shared code.** The two kinds' pin rules live in
  `src/models/terminals.c` / `terminals.h` (not duplicated), and
  `part_fixed_contact` in the new `src/models/model_pins.c`, as § 3.3 says.
- **2026-11-22, goldens.** None changed: `ctest` 43 of 43 (T0 41 + 2),
  `listing_check` and `trace_check` pass without a golden edit.

## Summary

I's engine risk is nil (no engine file changes); its one wide change is the
voltage check's group step, which now asks every part for fixed contacts and
is bounded by `voltage_test` and the unchanged goldens. Untouched: the
engine, every other kind, the circuit API's signatures, the file format and
the app.
