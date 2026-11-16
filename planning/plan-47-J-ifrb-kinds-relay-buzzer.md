# plan-47-J: Backend — the `relay` and `buzzer` kinds

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-47-I

J adds two part kinds. A **relay** has a coil (or, for a solid-state relay,
an input LED) and one or more poles of contacts. While its coil is driven
the armature moves 1 ms later: every normally-open contact closes and every
normally-closed one opens; a latching relay (one or two coils) keeps its
position until it is driven the other way; a package of several independent
relays (a dual solid-state relay) is one part with one channel per coil. The contacts are engine
contacts, as a switch's and a transistor's channel. A **buzzer** is a
two-terminal sounder: it is *on* while its positive pin reads 1 and its
negative pin reads 0, and reports that through the lights hooks, as an LED
does, so the app can draw and play it (plan-47-N). The behavioral outcome:
`relay_test` and `buzzer_test` pass the cases of § 6, `ctest` passes T0 + 2
programs (T0 measured at J's start, after I's two), and every existing golden
is unchanged (no part uses the kinds yet; L adds them).

References:

- `backend/src/models/transistor.c` (the model J mirrors for the relay: a
  control read in `pin_changed`, a timer, `sim_set_contact` in `timer`;
  `TRANSISTOR_DELAY_PS`; the same-named-pin ties),
  `backend/src/models/led.c` (the model J mirrors for the buzzer:
  `led_lights`, `led_light_time`, `lights.c`'s on-time tracking).
- `backend/src/models/model_pins.h` and `model_pins.c`
  (`part_fixed_contact`, plan-47-I § 3.3).
- `backend/src/engine/sim.h` (`sim_set_contact`, `sim_schedule_timer`, the
  `lights` and `light_time` hooks), `backend/include/circuit.h:68–74`
  (`circuit_part_lights`, `circuit_part_light_time`).
- `docs/backend.md` § Adding a kind, § Using it (**Transistors**, Lights);
  `docs/parts-format.md` § `transistor`, § `led`.
- `planning/completed/plan-40-F-transistor-kind.md`.

## Prerequisites

plan-47-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-47-I complete | `ls planning/completed/plan-47-I-*.md` → exists | NOT MET (2026-11-16: not started) |
| `part_fixed_contact` exists | `grep -c 'part_fixed_contact' backend/src/models/model_pins.h` → 1 | NOT MET (2026-11-16: 0; plan-47-I Phase 1 adds it) |
| No `relay` or `buzzer` kind yet | `grep -c 'PART_RELAY\|PART_BUZZER' backend/include/parts.h` → 0; `ls backend/src/models/relay.c backend/src/models/buzzer.c` → no such file | MET (2026-11-16 at 349f25b3: 0, no file) |
| Golden sizes and test count re-measured | `git show HEAD:backend/tests/golden/parts.txt \| wc -l` (and `traces.txt`, `footprints.txt`); `git show HEAD:backend/CMakeLists.txt \| grep -c '^logic_test'` | record P0 / R0 / F0 / T0 at J's start (at 349f25b3: 32897 / 32897 / 1646 / 37, so 39 tests after I) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `ctest --test-dir /tmp/logic-p47-j` → `100% tests passed` with T0 + 2
  programs.
- `relay_test` and `buzzer_test` pass the § 6 cases.
- `backend/tests/golden/parts.txt`, `traces.txt` and `footprints.txt` are
  byte-identical.

### Non-goals (explicit constraints)

- No analog behavior: no coil current, pick-up or drop-out voltage, contact
  bounce, coil inductance or back-EMF (a flyback diode across a coil is
  inert: a diode passes a 1 only from anode to cathode, `docs/backend.md`
  **Diodes**); no on-resistance, leakage or zero-cross timing of a
  solid-state relay; no sound pressure, no tone generation in the backend.
- The operate and release time is one fixed 1 ms for every relay; the
  datasheet's operate and release times are facts only.
- A coil is not a conductor: the two pins of a coil are inputs and nothing
  passes between them.
- No new engine function, no change to `sim.c` / `net.c`, no new circuit
  function: the app reads a relay's position and a buzzer's state with
  `circuit_part_lights` / `circuit_part_light_time`.
- No voltage rule for a coil, a contact or a buzzer; the voltage check sees
  a relay at rest (§ 3.3) and does not follow it during a run, as it does
  not follow a transistor.
- `circuit_set_switch` does not address a relay (`part N is not a switch`).
- Existing kinds are unchanged (the goldens are the check).

## 2. Current State

- The transistor model (`transistor.c:434–489`) is the precedent for a part
  that moves a contact from a pin: `pin_changed` on a control pin schedules
  timer `k` after `TRANSISTOR_DELAY_PS` (1000 ps); `timer` re-reads the pin
  and calls `sim_set_contact`. A timer scheduled again while pending is
  replaced (`sim.h:273–284`), so a pulse shorter than the delay never moves
  the contact.
- The LED model (`led.c:111–138`) is the precedent for a light: it tracks
  on-time through `lights.c` and answers `lights` and `light_time`;
  `circuit_part_lights` and `circuit_part_light_time` (`circuit.h:68–74`)
  expose them for any part whose model has the hooks.
- `part_fixed_contact` (plan-47-I) lets a kind name contacts that are closed
  inside the part for the voltage check.
- `tests/trace_check.c` stimulates a part by pin role (`trace_role`:
  `PIN_INPUT` → `TRACE_INPUT`, `PIN_INOUT` → `TRACE_INOUT`), so the new kinds
  need no trace code beyond the last-kind line.

### Measured populations

| What | Count | Command |
|---|---|---|
| Test programs at 349f25b3 | 37 (39 after plan-47-I) | `grep -c '^logic_test' backend/CMakeLists.txt` |
| Golden sizes P0 / R0 / F0 | 32897 / 32897 / 1646 | `wc -l backend/tests/golden/{parts,traces,footprints}.txt` |
| Relay contact forms in the planning pick (600 relays) | SPDT 206, SPST-NO 184 (+ 4 spelled `SPST (1 Form A)`), DPDT 139, SPST-NC 15, DPST-NO 14, 1 NO + 1 NC 10, 2 × SPST-NO 10, 2 × SPDT 5, 4PDT 3, 2 NO + 2 NC 2, 2 NO + 1 NC 2, 4 × SPST-NO 2, 4PST-NO 1, 4 NO + 1 NC 1, 1 Form X 1, none given 1 | `awk -F'\t' '$23=="candidate" && $3=="relay"{print $9}' planning/plan-47-data/candidates.tsv \| sort \| uniq -c \| sort -rn` |
| …coil types (electromechanical and reed, 556 rows) | non-latching 481, latching dual coil 31, latching single coil 17; the 44 solid-state rows have an input LED | `awk -F'\t' '$23=="candidate" && $3=="relay"{split($15,a,";"); print a[1]}' planning/plan-47-data/candidates.tsv \| sort \| uniq -c` |
| Most poles in the pick | 5 (one 4 NO + 1 NC part) | the forms above |
| Buzzers in the planning pick | 553: indicator (built-in driver) 222, transducer (external drive) 331 | `awk -F'\t' '$23=="candidate" && $3=="buzzer"{print $4}' planning/plan-47-data/candidates.tsv \| sort \| uniq -c` |

### Verified properties

- **A timer re-armed while pending is replaced, and a contact set in a timer
  takes effect at that time** (`sim.h:260–262, 273–284`; `transistor_test`'s
  "a pulse shorter than 1 ns never moves the channel" case).
- **A model with `lights` and `light_time` is all `circuit_part_lights`
  needs** (`circuit.c` calls the instance type's hooks; the 7-segment and
  character-LCD models are the second and third users).
- **`SimTime` is picoseconds in 64 bits** (`sim.h`), so a 1 ms delay is
  1 000 000 000 ps and a run of seconds does not overflow.

## 3. Design

### 3.1 The relay kind

`backend/src/models/relay.c`, `RELAY_KIND` (`"relay"`), file
`Parts/relay.json`. `parts.h` (appended after the last kind; only if
absent):

```c
PART_RELAY
typedef struct {
    const char *type;            /* "electromechanical", "reed" or "solid-state" */
    unsigned poles, coils;       /* contacts entries; coils entries */
    int latching;
    unsigned channels;           /* armatures: 1 for a latching relay, else one per coil */
    uint32_t coil_mv;            /* nominal coil voltage; 0 for a solid-state relay */
    uint64_t coil_mohm;          /* coil resistance, milliohms; 0 = not given */
    uint32_t contact_ma, contact_mv;   /* contact (load) current; max switching DC voltage, 0 = not given */
    uint32_t operate_us, release_us;   /* datasheet operate / release time; 0 = not given */
    uint64_t on_mohm;            /* a solid-state relay's on-resistance, max; 0 = not given */
} PartRelay;
/* Part: const PartRelay *relay; */
```

Device keys:

| Key | Value |
|---|---|
| `"type"` | `"electromechanical"`, `"reed"` or `"solid-state"` (`unknown relay type "X"`) |
| `"coils"` | 1 to 8 entries `{"a": <pin name>, "b": <pin name>}` (`"coils" must have 1 to 8 entries`; `a` ≠ `b`). Optional `"polarized": true` on an entry: only `a` = 1, `b` = 0 drives it. A solid-state relay's entries are its input LEDs (`a` the anode, `b` the cathode), always polarized. Without `"latching"` each entry is one **channel**: an independent relay in the package |
| `"latching"` | optional, default `false`; the relay then has one channel and 1 or 2 coils (`a latching relay has 1 or 2 coils`). With 1 coil: `a` = 1, `b` = 0 sets, `a` = 0, `b` = 1 resets (`"polarized"` is then an error: `a single-coil latching relay is not "polarized"`). With 2 coils: the first sets, the second resets; the two entries may share one pin (a three-terminal coil with a common). A solid-state relay is never latching (`a solid-state relay is not latching`) |
| `"contacts"` | 1 to 8 entries `{"common": <pin name>, "no": <pin name>, "nc": <pin name>}`, one per pole; `no`, `nc` or both (`a contact needs "no" or "nc"`); the three names of an entry are different. Optional `"coil": <n>`, 1-based, default 1: the channel that moves the pole (`"coil" must be 1 to <coils>`; an error on a latching relay); every channel has at least one pole (`coil 2 has no contact`) |

Every name names at least one pin of every pinout. A coil pin is `input`
(`relay pin "A1" must be input`), a contact pin `inout`, every other pin
`nc`; no pin is in two entries (`pin "X" is in two entries`), except the
common pin of a latching relay's two coils; every `input` and `inout` pin is
in an entry. Pins that share a name (a power relay's two
pins per contact) are tied by contacts closed in `start`.

Part keys:

| Key | Types | Value |
|---|---|---|
| `"coilVolts"` | electromechanical, reed (required) | nominal coil voltage, above 0 and at most 1000 (`"coilVolts" is not a solid-state relay key`) |
| `"coilOhms"` | electromechanical, reed (optional) | above 0 and at most 10000000 |
| `"contactAmps"` | all (required) | contact rating or load current, above 0 and at most 1000 |
| `"contactVolts"` | all (optional) | maximum switching (load) voltage, DC, above 0 and at most 100000 |
| `"operateMs"`, `"releaseMs"` | all (optional) | above 0 and at most 10000 |
| `"onOhms"` | solid-state (optional) | above 0 and at most 100000 (`"onOhms" is only for a solid-state relay`) |

### 3.2 The relay model

State: per channel, `armature` (0 = at rest, 1 = operated) and a lights
tracker. `start`: every `armature` = 0; every pole's `common`–`nc` contact
closed, every `common`–`no` contact open, the ties closed; one evaluation
per channel is scheduled (`sim_schedule_timer(…, <channel>,
RELAY_DELAY_PS)`, `RELAY_DELAY_PS` = 1 000 000 000) so a coil that is driven
from time 0 operates at 1 ms. `pin_changed` on a coil pin schedules the
timer of its channel (a latching relay's coils are all channel 0). `timer`
reads the channel's coil pins and computes the wanted position:

- a coil entry is *driven forward* when `a` reads 1 and `b` reads 0, *driven
  backward* when `a` reads 0 and `b` reads 1 (never, for a polarized entry);
  any X or Z on either pin is not driven;
- non-latching: a channel is operated while its coil is driven (either way);
- latching, 1 coil: forward → operated, backward → at rest, else unchanged;
- latching, 2 coils: only the first driven → operated, only the second
  driven → at rest, else unchanged.

When a channel's position changes, the model opens the contacts of the old
position and closes those of the new one for that channel's poles in the
same event (`sim_set_contact`), and records the light. Because the timer is re-armed by every coil-pin change,
the armature moves only after the coil has held its state for 1 ms.

The lights hooks report one light per channel, in coil order: on while that
armature is operated (`lights`: on at any time since `since`; `light_time`:
cumulative operated time), through `lights.c`'s tracker as `led.c` does.

A relay wired to break its own coil (the coil's `b` on GND, its `a` fed from
VCC through its own normally-closed contact) toggles every 1 ms and the run
returns: each change is an event 1 ms after the read that caused it.

### 3.3 The relay in the voltage check

`relay_fixed_contact(part, k, &a, &b)` (in `relay.c`, dispatched by
`part_fixed_contact`): every pole's `common`–`nc` pair, then the same-name
ties. The voltage check therefore joins a normally-closed contact's two nets
and leaves a normally-open contact's apart: the relay at rest.

### 3.4 The buzzer kind and model

`backend/src/models/buzzer.c`, `BUZZER_KIND` (`"buzzer"`), file
`Parts/buzzer.json`:

```c
PART_BUZZER
typedef struct {
    const char *driver;          /* "indicator" (built-in driver) or "transducer" (external drive) */
    const char *technology;      /* "piezo" or "magnetic" */
    uint32_t hz;                 /* rated (resonant) frequency */
    uint32_t rated_mv, min_mv, max_mv;   /* rated voltage and operating range; 0 = not given */
    uint32_t ua;                 /* supply current at the rated voltage, microamps; 0 = not given */
    uint32_t spl_cdb;            /* sound pressure level, hundredths of a dB; 0 = not given */
} PartBuzzer;
/* Part: const PartBuzzer *buzzer; */
```

Device keys: `"driver"` (`unknown buzzer driver "X"`), `"technology"`
(`unknown buzzer technology "X"`), `"positive"` and `"negative"` (pin names,
exactly one pin each, `kind_pin`). The two pins are `input` (`buzzer pin "+"
must be input`); every other pin is `nc` (a feedback electrode, a mounting
pad). Part keys: `"hz"` (required, a whole number 1 to 100000), `"volts"`,
`"voltsMin"`, `"voltsMax"` (optional, above 0 and at most 1000; `voltsMin`
at most `voltsMax`), `"ma"` (optional, above 0 and at most 10000), `"db"`
(optional, above 0 and at most 200).

`BUZZER_MODEL` is `led.c`'s shape: `start` and `pin_changed` evaluate on =
(`positive` reads 1 and `negative` reads 0) and feed the lights tracker;
`lights` and `light_time` report one light. The buzzer drives nothing and
has no timer. For a transducer driven by a square wave the light's on-time
is the wave's high time; the app derives the tone from the net's change
count (plan-47-N), the backend adds nothing for it.

Where the correctness risk concentrates: the relay's timer logic for the
three coil arrangements (a wrong hold rule for a latching relay) and the
break-before-make order in one event. `relay_test` has one case per
arrangement and per transition.

### 3.5 Byte-identity

This is **not** a byte-identity plan. The goldens are expected unchanged
only because no part uses the kinds yet; a diff is a bug to root-cause,
never a stop.

Rejected alternatives: (1) the datasheet's operate and release times as the
delay (rejected: a corrected datasheet value would move every trace golden
of that part, and 3 ms against 10 ms shows nothing in a logic simulation);
(2) a 1 ns delay like the transistor's (rejected: a self-interrupting relay
would toggle a million times per simulated millisecond; 1 ms is a relay's
order of magnitude and gives a buzzer circuit an audible 500 Hz); (3) a coil
as a weak link between its pins (rejected: the standard low-side driver
works without it, and a weak 1 leaking from a coil into a floating
open-collector net would change what existing parts read); (4) a new
`circuit_part_state` function (rejected: the lights hooks already carry one
bit per part with on-time).

## 4. Docs

`docs/parts-format.md`: the kind list and table get `relay.json` / `relay`
and `buzzer.json` / `buzzer`; `### relay` after `### switch` (keys, the
drive rules of § 3.2, the 1 ms delay, what is not modeled, the voltage
check's at-rest view) and `### buzzer` after `### led`. `docs/backend.md`: §
Using it gains a **Relays** bullet after **Inductors and fuses** with the
result table below, and the Lights bullet (`:218`) names relays (operated)
and buzzers (on); the `models/` list gets `relay.c`, `buzzer.c`; § Tests
gets the two test lines.

| Circuit | Result |
|---|---|
| coil between a gate output and GND, 5V on common | NO reads 1 and NC reads Z 1 ms after the output goes to 1; back 1 ms after it goes to 0 |
| coil between 5V and an NPN's collector (emitter on GND) | operated while the base reads 1 |
| a coil pin floating | at rest |
| latching, 1 coil | a 2 ms forward pulse operates it; it stays until a 2 ms backward pulse |
| latching, 2 coils | a pulse on the set coil operates it, one on the reset coil releases it |
| a 0.5 ms pulse on a coil | nothing moves |
| solid-state: anode 1, cathode 0 | the output contact closes after 1 ms; anode 0, cathode 1 does nothing |

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The relay kind: loader and facts

- [ ] `backend/include/parts.h`: `PART_RELAY` after the current last
      `PartKind`, `PartRelay`, `const PartRelay *relay;` (only if absent:
      `grep -c 'PART_RELAY' backend/include/parts.h` → 0 before).
- [ ] `backend/src/models/relay.c`: `RELAY_KIND` (`load_device`,
      `bind_pinout`, `load_part`, § 3.1) with a passive model for now;
      `kinds.c`; `CMakeLists.txt` (`relay.c`, `logic_test(relay_test)`).
- [ ] `backend/tests/support/listing.c`: `case PART_RELAY: return "relay"`
      and the facts line `type=%s poles=%u coils=%u latch=%d ch=%u coil=%u
      ohms=%llu amps=%u volts=%u op=%u rel=%u on=%llu`; the last-kind lines
      of `tests/trace_check.c` and `tests/listing_check.c`.
- [ ] `backend/tests/support/relay_fixture.h`, `backend/tests/relay_test.c`:
      the fact and rejection cases (§ 6 a–d).

Acceptance: relays load and read back; goldens unchanged.
  Check: `cmake -S backend -B /tmp/logic-p47-j && cmake --build /tmp/logic-p47-j -j8 && /tmp/logic-p47-j/relay_test` → every line `PASS`, and `/tmp/logic-p47-j/listing_check && /tmp/logic-p47-j/trace_check` → exit 0 (est. 4 min: a build from scratch).
Commit: —

### Phase 2 — The relay model and its fixed contacts

- [ ] `relay.c`: `RELAY_MODEL` (§ 3.2), `relay_fixed_contact` and its
      dispatch in `model_pins.c` (§ 3.3); the cases § 6 e–q.

Acceptance: the relay cases pass; goldens unchanged.
  Check: `cmake --build /tmp/logic-p47-j -j8 && /tmp/logic-p47-j/relay_test && /tmp/logic-p47-j/voltage_test` → every line `PASS`; `/tmp/logic-p47-j/listing_check && /tmp/logic-p47-j/trace_check` → exit 0 (est. 2 min).
Commit: —

### Phase 3 — The buzzer kind

- [ ] `parts.h`: `PART_BUZZER`, `PartBuzzer`, `const PartBuzzer *buzzer;`
      (only if absent); `backend/src/models/buzzer.c` (`BUZZER_KIND`,
      `BUZZER_MODEL`, § 3.4); `kinds.c`; `CMakeLists.txt` (`buzzer.c`,
      `logic_test(buzzer_test)`).
- [ ] `listing.c`: `case PART_BUZZER: return "buzzer"` and the facts line
      `driver=%s tech=%s hz=%u volts=%u min=%u max=%u ua=%u spl=%u`; the
      last-kind lines.
- [ ] `backend/tests/support/buzzer_fixture.h`, `backend/tests/buzzer_test.c`
      (§ 6 r–w).

Acceptance: the buzzer cases pass; goldens unchanged.
  Check: `cmake --build /tmp/logic-p47-j -j8 && /tmp/logic-p47-j/buzzer_test` → every line `PASS`; `/tmp/logic-p47-j/listing_check && /tmp/logic-p47-j/trace_check` → exit 0 (est. 2 min).
Commit: —

### Phase 4 — Docs

- [ ] `docs/parts-format.md` and `docs/backend.md` per § 4.

Acceptance: the docs name both kinds.
  Check: `grep -c '^### `relay`\|^### `buzzer`' docs/parts-format.md` → 2 and `grep -c 'relay_test\|buzzer_test' docs/backend.md` → 2 (est. < 1 min).
Commit: —

## 6. Test cases

`relay_fixture.h` (loaded next to the real part files; every part names a
library footprint with its pad count: `PINHDR-1X04-V` … `PINHDR-1X08-V`,
which `footprints.json` has for 1 to 9 pads). The timed cases run on the
engine rig of `backend/tests/transistor_test.c` (`Rig`, `Script`,
`rig_build`, `rig_run`, `value`: an engine-level source that drives nets at
scripted times), copied into `relay_test.c`; the library cases use
`circuit_*`.

- `TESTRLY-SPDT`: electromechanical, coil `A1`/`A2`, one pole `COM`, `NO`,
  `NC` (5 pins); `coilVolts` 5, `coilOhms` 125, `contactAmps` 10,
  `contactVolts` 30, `operateMs` 10, `releaseMs` 5.
- `TESTRLY-DPDT`: two poles (8 pins); `TESTRLY-POL`: as SPDT with
  `"polarized": true`; `TESTRLY-L1`: latching, 1 coil; `TESTRLY-L2`:
  latching, 2 coils (`S1`/`S2`, `R1`/`R2`, one pole: 7 pins);
  `TESTRLY-NO`: one pole with `no` only (4 pins); `TESTRLY-TIE`: SPDT with
  two pins named `COM` (6 pins); `TESTSSR`: solid-state, `A`/`K`, one pole
  `D1`–`D2` as `common`/`no` (4 pins), `contactAmps` 0.12, `contactVolts`
  60, `onOhms` 25; `TESTSSR2`: solid-state, two channels (`A1`/`K1`,
  `A2`/`K2`; poles `D1A`–`D1B` with `"coil": 1` and `D2A`–`D2B` with
  `"coil": 2`; 8 pins); `TESTRLY-L2C`: as `TESTRLY-L2` with a common coil
  pin (`S`, `C`, `R`: 6 pins).

`relay_test`:

- (a) facts of `TESTRLY-SPDT` (kind, 5 pins, roles input/input/inout×3,
  `type`, `poles` 1, `coils` 1, `latching` 0, `coil_mv` 5000, `coil_mohm`
  125000, `contact_ma` 10000, `contact_mv` 30000, `operate_us` 10000,
  `release_us` 5000, `on_mohm` 0, `channels` 1), of `TESTRLY-L2` (`coils` 2,
  `latching` 1, `channels` 1), of `TESTSSR` (`coil_mv` 0, `on_mohm` 25000)
  and of `TESTSSR2` (`coils` 2, `poles` 2, `channels` 2).
- (b) device rejections: `"type": "contactor"` → `unknown relay type
  "contactor"`; no coil entry or nine → `"coils" must have 1 to 8 entries`;
  a latching relay with 3 coils → `a latching relay has 1 or 2 coils`; a
  latching 1-coil entry with `polarized` → `a single-coil latching relay is
  not "polarized"`; a contact with neither `no` nor `nc` → `a contact needs
  "no" or "nc"`; a latching solid-state relay → `a solid-state relay is not
  latching`; `"coil": 3` on a 2-coil relay → `"coil" must be 1 to 2`; a
  2-coil non-latching relay whose contacts all name coil 1 → `coil 2 has no
  contact`.
- (c) pin rejections: a coil pin with role `inout` → `relay pin "A1" must be
  input`; a pin in two entries → `pin "COM" is in two entries`; an `inout`
  pin in no entry → `pin "X" is in no entry`.
- (d) part rejections: no `"coilVolts"` on an electromechanical part →
  `missing "coilVolts"`; `"coilVolts"` on `TESTSSR` → `"coilVolts" is not a
  solid-state relay key`; `"onOhms"` on an electromechanical part →
  `"onOhms" is only for a solid-state relay`; no `"contactAmps"` → `missing
  "contactAmps"`.
- (e) at rest (A1 and A2 on GND, 5V on COM): NC's net reads 1, NO's net Z;
  the light is off.
- (f) operate: A1 on a net the rig's script drives, A2 at 0: 0.9 ms after
  A1 goes to 1 nothing has moved; at 1.1 ms NO reads 1, NC reads Z, the
  light is on.
- (g) release: 1.1 ms after A1 goes back to 0, NC reads 1 and NO reads Z.
- (h) a 0.5 ms pulse on A1 moves nothing (checked at 2 ms).
- (i) reverse drive (A1 = 0, A2 = 1) operates `TESTRLY-SPDT` and does not
  operate `TESTRLY-POL`.
- (j) a floating coil pin (A2 on no net, A1 = 1): at rest.
- (k) `TESTRLY-DPDT`: both poles change in the same event (both NO nets read
  1 at 1.1 ms).
- (l) `TESTRLY-L1`: a 2 ms forward pulse operates it; with both pins back at
  0 it stays operated at 10 ms; a 2 ms backward pulse releases it.
- (m) `TESTRLY-L2`: a 2 ms pulse on the set coil operates it; a pulse on the
  reset coil releases it; both coils driven at once changes nothing;
  `TESTRLY-L2C` (common pin `C` at 0): the same with `S` and `R` pulsed.
  `TESTSSR2`: driving `A1`/`K1` closes only `D1A`–`D1B` and turns on only
  the first of the part's two lights; `A2`/`K2` the second.
- (n) `TESTRLY-NO`: common and NO read as one net only while operated.
- (o) `TESTRLY-TIE`: 5V on the second `COM` pin reaches NC through the tie.
- (p) the self-interrupting relay (5V → NC, COM → A1, A2 on GND):
  `circuit_advance` for 10 ms returns 1 and COM's `circuit_net_changes` is
  within 1 of 10; the low-side driver (rig): the NPN fixture part `TFX-Q1`
  (`tests/support/transistor_fixture.h`) with its collector on A2, A1 at 1,
  its emitter at 0: operated 1 ms (plus the transistor's 1 ns) after the
  base goes to 1.
- (q) `TESTSSR`: A = 1, K = 0 → `D1` and `D2` read as one net after 1 ms; A
  = 0, K = 1 → open. The voltage check: 12V on NC of `TESTRLY-SPDT` and COM
  on the VCC pin of an `SN74HC04N` → `circuit_start` -1
  (`VOLTAGE_POWER_RANGE`, the NC contact is a fixed contact); 12V on NO
  instead → starts. `circuit_set_switch` on a relay → `part N is not a
  switch`.

`buzzer_fixture.h`: `TESTBUZ-IND` (indicator, magnetic, `+`/`-`, `hz` 2300,
`volts` 5, `voltsMin` 3, `voltsMax` 7, `ma` 30, `db` 85) and `TESTBUZ-PZ`
(transducer, piezo, a third pin `F` with role `nc`, `hz` 4000).

`buzzer_test`:

- (r) facts: kind `PART_BUZZER`, `driver`, `technology`, `hz` 2300,
  `rated_mv` 5000, `min_mv` 3000, `max_mv` 7000, `ua` 30000, `spl_cdb`
  8500; `TESTBUZ-PZ`: 3 pins, pin `F` `PIN_NC`, the optional facts 0.
- (s) rejections: `"driver": "self"` → `unknown buzzer driver "self"`;
  `"technology": "dynamic"` → `unknown buzzer technology "dynamic"`; no
  `"hz"` → `missing "hz"`; `"hz": 0` → `"hz" must be at least 1`; a `+` pin
  with role `inout` → `buzzer pin "+" must be input`; `voltsMin` above
  `voltsMax` → `"voltsMin" must be at most "voltsMax"`.
- (t) `+` on 5V and `-` on GND: the light is on (`circuit_part_lights`) and
  `circuit_part_light_time` after 1 ms is 1 000 000 000.
- (u) reversed, and with `+` floating: off, on-time 0.
- (v) rig: `+` driven with a 1 kHz square wave (0.5 ms high, 0.5 ms low) for
  10 ms, `-` at 0: the on-time (`light_time` hook) is 5 ms.
- (w) a buzzer between two gate outputs follows `+` = 1 and `-` = 0 only.

## Validation Plan

- Tests: `relay_test` (a–q), `buzzer_test` (r–w); `voltage_test`,
  `transistor_test` and `switch_test` unchanged and green.
- Coverage check: the cases reach each coil arrangement, each transition,
  both lights hooks and `relay_fixed_contact`.
- Runtime proof: case (p), a relay driven by a transistor, and case (v), a
  buzzer driven by a clock.
- Doc sync: § 4.
- Final gate (run ONCE, in plan-47-O): the landing tier.

## Decisions

Owner decisions (asked once, 2026-11-16, before this plan was written):

- **A relay switches in play mode**: its contacts change over when its coil
  is driven.
- **A buzzer is "on" in play mode** so the app can sound it (plan-47-N).
- **Latching relays, relays of up to four poles, reed relays and solid-state
  relays are in.**

Author decisions (no user-visible fork; recorded so nobody re-litigates):

- **One `relay` kind for electromechanical, reed and solid-state relays**:
  the model is the same (an input that moves contacts); the `type` and the
  facts tell them apart.
- **1 ms, fixed, both ways** (§ 3.5 rejected alternatives 1 and 2).
- **A non-polarized coil is driven by either polarity**; a coil with a
  built-in diode and a solid-state relay's LED are `polarized`.
- **Up to 8 poles and 8 channels** in the loader (the pick's largest part has
  5 poles; its multi-channel parts are 2 × and 4 × SPST-NO solid-state
  relays and 2 × SPDT relays, 17 rows).
- **The armature and the buzzer state travel through the lights hooks** (§
  3.5 rejected alternative 4).
- **No coil, contact or buzzer voltage rule**: coil and buzzer voltages are
  nominal operating points, not absolute limits the data gives uniformly;
  the facts are shown in the sidebar instead.

## Corrections

## Summary

J's risk is the relay's timer logic across three coil arrangements; the
test's per-transition cases bound it, and no engine file changes. Untouched:
the engine, every other kind, the circuit API, the file format and the app.
