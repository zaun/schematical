# plan-29-H: Backend — one-way diode links in the engine, the `diode` kind, tests, docs

Last updated: 2026-11-09
Effort: large (3h–1d)
Depends on: plan-29-A § Prerequisites only (corrected 2026-11-09, § Corrections)

H adds what a diode does: a one-way link in the net engine (a 1 passes from
anode to cathode, a 0 from cathode to anode, nothing the other way), the
`diode` part kind that loads `Parts/diode.json`, the voltage check's carry
through a diode, the listing line, and a test program `diode_test`. No real
part is added here (I–L add them); the tests use inline fixture devices.

The single behavioral outcome: a circuit with two gate outputs on the anodes
of two diodes whose cathodes share a net with a pull-down reads that net as
the OR of the two outputs (and the mirrored AND circuit with a pull-up reads
the AND); a strong 1 on an anode and a strong 0 on the cathode stops the run
with `smoke:`; every existing part's trace line is unchanged.

References:

- `backend/src/engine/sim.c` (`build_groups` :713, `resolve_group` :758,
  `resolve_nets` :773, `process_round` :924, `sim_start` :814),
  `backend/src/engine/net.c` (`net_resolve_group` :86), `sim.h`
  (`SimComponentType` :95–175, contacts :241–264).
- `backend/src/models/switch.c` (contacts through `sim_set_contact`),
  `backend/src/models/led.c` (the two-terminal precedent, `led_pins`).
- `backend/src/voltage.c` (step 3, the LED carry at :252–282),
  `backend/src/models/model_pins.h`.
- `backend/src/library/kind.h` (`PartKindOps`), `kind_util.c` (`kind_pins` :391).
- `backend/tests/support/listing.c`, `listing_check.c` (:50), `trace_check.c`
  (:22 `TRACE_LAST_KIND`), `support/trace_core.c` (inout pins get weak
  random drives, input pins strong ones).
- `docs/backend.md`, `docs/parts-format.md` (§ Kinds).

## Prerequisites

plan-29-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| ~~plan-29-G complete~~ (dropped 2026-11-09, § Corrections) | — | moot: H reads no B–G output |
| No `diode` kind yet | `grep -c 'PART_DIODE' backend/include/parts.h` → 0 | MET (2026-11-09 at a98aecb: 0) |
| No other open plan adds an engine link type or a kind after `PART_RTC` | `grep -l 'PART_[A-Z]* *after PART_RTC\|sim_.*diode\|"diodes" hook' planning/plan-*.md \| grep -v plan-29 \| wc -l` → 0 | MET (2026-11-09: 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `ctest` passes T0 + 1 programs (T0 = 29 at f68e13f, 28 at 0f87917, 27 at planning time, plan-29-A
  § Prerequisites); the new one is `diode_test`.
- `trace_check` and `listing_check` pass with the goldens unchanged:
  `git diff --stat backend/tests/golden` → empty after H (no shipped part is
  a diode yet; `Parts/diode.json` ships with `"devices": []`).
- `docs/parts-format.md` has a `### diode` section; `docs/backend.md`
  describes diode links and the voltage carry.

### Non-goals (explicit constraints)

- **No change to any existing part's behavior.** Without a diode in the
  circuit the engine takes exactly today's path (`closed_contact_count` /
  group code untouched in effect): every `traces.txt` line stays as it is.
  This is the plan's byte-identity gate for existing parts only; new diode
  behavior is checked by `diode_test`.
- LEDs stay passive (they drive and pass nothing; `led.c` unchanged).
- No analog model: no forward-voltage drop in the logic, no Zener
  breakdown, no reverse-voltage rating check (§ 3 Rejected).
- No app change (plan-29-M).

## 2. Current State

- Nets resolve in `net_resolve_group` (`net.c:86`): among the drivers of a
  net, or of all nets of a contact group, the strongest strength wins, ties
  that disagree give X, and a strong 1 with a strong 0 sets `shorted`, which
  `process_round` turns into `smoke: short circuit, strong drivers disagree
  on a net`. Strengths are `DRIVE_WEAK` and `DRIVE_STRONG` (`sim.h:16–19`).
- Contacts (`sim_set_contact`) join two pins' nets both ways; groups are a
  union-find over closed contacts (`build_groups`, `sim.c:713`), cached and
  rebuilt when a contact changes. A dirty net widens to its whole group
  (`process_round`, `sim.c:961–981`).
- There is nothing one-way. A model cannot fake a diode by driving its own
  pins: it reads the net value, which includes its own drive, so a 1 it
  passed to the cathode would hold itself up after the anode fell (latch-up),
  and a strong pass-through drive would fight the real driver (smoke). The
  pass must be computed by the resolver from the *other* drivers, which is why
  it lives in the engine.
- The voltage check carries an LED's anode level to its cathode group
  (`voltage.c:252–282`, through `led_pins` in `model_pins.h`).
- The trace harness (`trace_core.c:78`) drives input pins strong and inout
  pins weak with a seeded random stimulus, and hashes every net change.
- Kinds: `PartKind` ends with `PART_RTC` (`parts.h:28`); `listing.c:13–32`
  names kinds; `listing_check.c:50` and `trace_check.c:22` size arrays by
  the last kind.

### Measured populations

| What | Count | Command |
|---|---|---|
| Test programs | 27 | `grep -c '^logic_test' backend/CMakeLists.txt` → 27 (2026-11-08) |
| Kind files | 25 | `ls schematical/schematical/Parts/*.json \| wc -l` → 27, minus `datasheets.json`, `footprints.json` |
| Lines that size by the last kind | 3 | `grep -rn 'PART_RTC + 1\|TRACE_LAST_KIND PART_RTC' backend/tests \| wc -l` → 3 (`listing_check.c` ×2, `trace_check.c`) |
| Listing kind-name switch | 1 | `grep -n 'case PART_RTC: return "rtc"' backend/tests/support/listing.c` → 1 |

### Verified properties

- **The fast path stays fast and identical.** `process_round` branches on
  `sim->closed_contact_count == 0` (`sim.c:958`; `resolve_nets` the same at `:776`). H adds a second condition,
  `sim->diode_count == 0`; with both zero the code is today's. Read
  `process_round` and `resolve_nets` (`sim.c:773–790`).
- **A pin the stimulus drives weak can be overridden by a passed strong
  value**, so a diode part's trace (inout pins) records diode behavior, not
  an instant short. `trace_core.c:79`: `role == TRACE_INPUT ? DRIVE_STRONG :
  DRIVE_WEAK`. That is why diode pins are `inout` (the switch's precedent,
  `switch.json:14`).
- **`kind_pins` finds every pin of a name** (`kind_util.c:391`), so a
  device's anode or cathode may be several pins (a TO-220 tab and lead).

## 3. Design Overview

Three pieces, in landing order:

1. **Engine: diode links** (`sim.h`, `sim.c`, `net.c`). A model reports its
   links through a new optional hook; `sim_start` collects them; the
   resolver treats a *cluster* (contact groups joined by any link) as one
   resolution unit.
2. **The `diode` kind** (`models/diode.c`, `parts.h`, `kinds.c`,
   `model_pins.h`, `voltage.c`, listing). Loads devices with a list of
   (anode, cathode) pin-name pairs; same-named pins are tied.
3. **Tests and docs** (`diode_test.c`, `library_check.c`,
   `voltage_test.c`, docs).

**Where the correctness risk is:** piece 1. It runs inside every round of
every circuit that has a diode, and a mistake there either breaks other
parts' timing (dirty-set widening) or makes a diode latch, oscillate or fail
to short. It lands behind `diode_test`'s engine cases and the unchanged
`traces.txt` (the gate that no existing part moved).

**Gate class.** For existing parts this is provably neutral: `traces.txt`
and `parts.txt` must not change (byte-identical). Diode behavior is new and
is checked by `diode_test` against hand-written expected values. A trace
diff on an existing part means the fast path was disturbed: run
`trace_check --verbose <part>` on that one part, find the change, fix it.

**Rejected alternatives.**

- *A model that drives its own pins* (as a buffer does): latches up and
  shorts (§ 2). Rejected.
- *Diodes as contacts that close when forward biased*: a closed contact
  passes values both ways, so a diode-OR would back-drive the other gate's
  output through the common net. Rejected.
- *Iterating net values to a fixed point with ordinary resolution*: can
  oscillate on diode loops (antiparallel pairs). The two-pass reachability
  below always terminates in O(nodes + links) and needs no iteration.
- *Forward drop, Zener breakdown and reverse-rating checks in the voltage
  check*: logic circuits here run at 0.8–5.5 V while the in-scope parts are
  rated 2 V (some Zeners) to 1600 V; a drop model changes no verdict the
  check gives today and a rating check would almost never fire. Zener
  voltage is shown as a fact only (plan-29-M).

## 4. Engine: diode links

### 4.1 API

`sim.h`, `SimComponentType`, a new optional hook after `rtc_time`:

```c
/* Diodes (plan-29-H): the one-way links between pins of this instance,
   up to cap of them; returns how many there are. Link i passes a 1 from
   pin anode[i] to pin cathode[i] and a 0 from cathode[i] to anode[i].
   Called once, by sim_start. NULL = none. */
size_t (*diodes)(const void *state, size_t *anode, size_t *cathode, size_t cap);
```

`sim_start` (before `resolve_nets`) calls it for every instance whose type
has it, stores each link `{instance, anode pin, cathode pin}` whose two pins
are both on nets in `sim->diodes` (`Diode { SimNet *anode, *cathode; }`,
grown like `contacts`), and sets `sim->diode_count`. A link whose two pins
are on the same net is dropped. Links never change after start.

### 4.2 Clusters

`build_groups` keeps its contact union-find (`group_root`, `group_head`,
`group_next`: unchanged meaning). A second union-find over the group roots,
joined by every diode link, gives `cluster_root[i]` per net and a member chain
like the groups' (`cluster_head`, `cluster_next`). Allocated in
`alloc_round_arrays` only when `diode_count > 0`. Rebuilt whenever groups
are rebuilt (a contact change), since a contact can join two clusters.

`process_round`: when `diode_count > 0`, after the contact-group widening,
every dirty net's whole cluster is marked dirty; each cluster whose root is
dirty is resolved once by `resolve_cluster` (§ 4.3); a net in no cluster
with a diode (a cluster of one group and no links) resolves as today.
`resolve_nets` (start) does the same for every cluster.

### 4.3 `resolve_cluster` — the algorithm

Nodes are the contact groups of the cluster; links are directed node →
node (anode node → cathode node). Values are 0, 1, X; a node holds a set
of values per strength level.

1. **Own drivers.** For each node, from the drivers on its nets exactly as
   `net_resolve_group` reads them: the strong set `S[n]` ⊆ {0, 1, X} of
   values driven strong (X is in it when a driver drives X strong) and the
   weak set `W[n]` likewise. (`net.c` gains `net_drivers(nets, count,
   &strong_mask, &weak_mask)` and `net_resolve_group` is rewritten on top of
   it with identical results: a pure refactor, checked by the unchanged
   traces.)
2. **Strong pass.** Breadth-first over links: a 1 or an X in `S[a]` is
   added to `S[c]` for each link a → c; a 0 or an X in `S[c]` is added to
   `S[a]`. Repeat from every node that gained a value until nothing changes
   (each node gains at most three values: it terminates).
3. **Weak pass.** The same over `W`, but a value passes into or out of a
   node only when that node's `S` is empty (a strongly driven node blocks
   weak values, as a strong driver beats a pull on a plain net).
4. **Result per node.** If `S[n]` is non-empty: value = the one element, or
   X when it has two or more; `shorted` = `S[n]` holds both 0 and 1. Else if
   `W[n]` is non-empty: likewise from `W[n]`, never shorted. Else Z. Every
   net of the node gets the node's value and `shorted`.

What this gives (each is a `diode_test` case, § 6):

| Circuit | Result |
|---|---|
| strong 1 on anode, cathode net has a weak 0 (pull-down) | cathode 1 |
| strong 0 on anode, cathode pulled down | cathode 0 (the pull) |
| two gates → two anodes, common cathode, pull-down (diode-OR) | OR of the gates; neither gate's net changes |
| two gates on two cathodes, common anode, pull-up (diode-AND) | AND of the gates |
| strong 1 on anode, strong 0 on cathode | `smoke:` (forward-biased between two strong drivers) |
| strong 0 on anode, strong 1 on cathode | no conduction; both keep their values |
| two diodes in series, strong 1 at the first anode, pull-down at the end | end reads 1; the middle node reads 1 |
| antiparallel pair, a weak 1 on one side, nothing on the other | both 1; and back to Z on both when the 1 goes (no latch) |
| X driven strong on an anode | cathode X |
| nothing on either side | both Z |

## 5. The `diode` kind

### 5.1 File format (`Parts/diode.json`, `docs/parts-format.md` § `diode`)

Device keys:

| Key | Value |
|---|---|
| `"type"` | `"rectifier"`, `"fast"`, `"switching"`, `"schottky"`, `"zener"` or `"bridge"` (the drawer leaf follows it, plan-29-A § 3) |
| `"diodes"` | non-empty array of `{"anode": <pin name>, "cathode": <pin name>}`; each name names at least one pin of every pinout (`kind_pins`); a pin name that several pins carry ties them (a tab and a lead) |

Part keys (all volts / amps / watts as JSON numbers):

| Key | Parts | Value |
|---|---|---|
| `"vr"` | every type but `zener` (required); `zener` (absent) | max repetitive reverse voltage, 0 < v ≤ 3000 |
| `"io"` | every type but `zener` (required) | average forward current per diode, 0 < a ≤ 1000 |
| `"vz"` | `zener` only (required) | nominal Zener voltage, 0 < v ≤ 1000 |
| `"pd"` | `zener` only (required) | max power dissipation, 0 < w ≤ 100 |

Rules: every pin of every pinout is `inout`, `nc` or `absent` (`diode pin "X"
must be inout`); a pin that is `inout` is in some pair or tied to one (`pin
"X" is in no diode`). Messages follow the loader's `kind_error` style.

### 5.2 C side

- `parts.h`: `PART_DIODE` appended after the last kind present when H lands
  (`grep -n 'PART_RTC$\|PART_RTC }' backend/include/parts.h` shows it is
  `PART_RTC` at planning time); `typedef struct { const char *type; size_t
  diode_count; uint32_t vr_mv, io_ma, vz_mv, pd_mw; } PartDiode;` and
  `const PartDiode *diode;` in `Part` beside `led`.
- `models/diode.c`: `DEVICE_KEYS = {"type", "diodes", NULL}`, `PART_KEYS =
  {"vr", "io", "vz", "pd", NULL}`; `bind_pinout` resolves each pair with
  `kind_pins` into index lists; the model's `create` keeps the bound
  pairs; the `diodes` hook returns, per pair, one link per (anode pin,
  cathode pin) and, per pin name carried by several pins, two links (both
  ways) between the first pin and each other one (a tie passes everything).
  No drives, no timers, no lights.
- `kinds.c`: `&DIODE_KIND` appended to `KINDS[]`.
- `model_pins.h` / `diode.c`: `int diode_pair(const Part *part, size_t k,
  size_t *anode, size_t *cathode)` (the k-th link of the part, 0 past the
  end or for another kind).
- `voltage.c` step 3: beside the LED carry, for a `PART_DIODE` part every
  link carries its anode group's level to its cathode group (`level_raise`,
  the LED code's loop); a diode has no `vcc`, so the vcc checks skip it as
  they skip an LED.
- Listing (`tests/support/listing.c`): kind name `"diode"`; detail field
  `type=<type> diodes=<n>` then ` vr=<mV> io=<mA>` or ` vz=<mV> pd=<mW>`.
- `listing_check.c:50,51` and `trace_check.c:22`: size by `PART_DIODE`.
- `schematical/schematical/Parts/diode.json`: `{"format":
  "schematical-parts", "version": 1, "kind": "diode", "devices": []}`.
  Nothing lists the files: the app loads every bundled JSON but
  `datasheets.json` and `footprints.json` (`PartCatalog.load`,
  `PartCatalog.swift:626`; the `Parts` folder is a synchronized group,
  `grep -c 'rtc.json' schematical/schematical.xcodeproj/project.pbxproj` → 0)
  and the tests load every `*.json` in the folder (`load_parts_dir`,
  `tests/support/read_file.c:60`). The kind table in
  `docs/parts-format.md` gains the row.

## 6. Tests (`backend/tests/diode_test.c`, `logic_test(diode_test)`)

Inline fixture devices (a single diode `D1` pins K, A; a common-cathode pair
`D2` pins A1, A2, K; a two-lead + tab single `D3` pins K, A, K), built with
the fixture loader `circuit_check` uses, and the real `supply` parts for
VCC, GND and the pull parts. Cases: the ten rows of § 4.3, each checking net
values after `circuit_advance`, plus:

- a run: a gate toggling on a diode-OR's anode for 1 µs; the common net
  follows it with no extra delay and `circuit_net_changes` counts the edges;
- a tab-tied part: driving the tab K pin reaches the lead K pin;
- a contact in a cluster: a DIP switch between a diode's cathode and a
  pull-down; opening it leaves the cathode net Z after the anode's 1;
- loader errors (in `library_check.c`, beside the other kinds'): unknown
  type, empty `diodes`, an unknown pin name, an `input` pin, an inout pin in
  no diode, `vz` on a rectifier, missing `vr`;
- voltage (`voltage_test.c`): 5 V supply → anode, cathode net to an
  SN74HC00N input → its input check sees 5 V (carried); a 5 V anode into a
  3.3 V-max input reports the input problem.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Resolver refactor (neutral)

- [x] `net.c`: `net_drivers` (§ 4.3 step 1) and `net_resolve_group` on top
      of it, same results. Also `net_mask_value`, `net_mask_shorted` (the
      mask → value / short reading `resolve_cluster` shares). Checked:
      `trace_check` → `15214 parts compared, 0 skipped`, exit 0;
      `engine_test` → `0 failure(s)` (build dir /tmp/logic-p29w).

Acceptance: every trace unchanged.
  Check: `cmake --build /tmp/logic-p29 -j8 && /tmp/logic-p29/trace_check && /tmp/logic-p29/engine_test` → both pass (est. 2 min).
Commit: 737b58d

### Phase 2 — Diode links in the engine

- [x] `sim.h`: the `diodes` hook (§ 4.1). `sim.c`: `Diode` array, collection
      in `sim_start` (`collect_diodes`), clusters (§ 4.2, `build_clusters`,
      called from `build_groups`), `resolve_cluster` (§ 4.3; the passes are
      fixed-point loops over the cluster's link chain, the same result as
      the breadth-first wording), the `process_round` / `resolve_nets`
      branches.
- [x] `diode_test.c` with an engine-only fixture component (a
      `SimComponentType` whose `diodes` hook returns one link; no part
      library) for the ten § 4.3 rows, plus the § 6 diode-OR run case (it
      needs no part library, so it lives here); `logic_test(diode_test)` in
      `backend/CMakeLists.txt`. Checked: `diode_test` → 13 PASS,
      `0 failure(s)`; `trace_check` → `15214 parts compared, 0 skipped`,
      exit 0; `circuit_check`, `switch_test` exit 0.

Acceptance: the § 4.3 table holds and no existing trace moved.
  Check: `cmake --build /tmp/logic-p29 -j8 && /tmp/logic-p29/diode_test && /tmp/logic-p29/trace_check` → `0 failure(s)` and pass (est. 3 min).
Commit: ef417a0

### Phase 3 — The `diode` kind and the voltage carry

- [x] `parts.h`, `models/diode.c`, `kinds.c`, `model_pins.h`, `voltage.c`,
      `listing.c`, `listing_check.c`, `trace_check.c`, `Parts/diode.json`
      (empty) (§ 5.2). Also `backend/CMakeLists.txt` (`src/models/diode.c`
      in the library sources). Part keys are checked per type in
      `load_part`: a zener refuses `vr` / `io`, the others refuse `vz` /
      `pd`.
- [x] `diode_test.c`: the fixture-device cases of § 6 (run, tab tie,
      contact in a cluster); `library_check.c`: the loader errors;
      `voltage_test.c`: the two voltage cases. The fixtures (DFX-D1 K A,
      DFX-D2 A1 A2 K, DFX-D3 K A K-tab) are `tests/support/diode_fixture.h`,
      shared by both tests; the run case is the engine one of Phase 2, and
      a DFX-D2 diode-OR in a library circuit is added. The voltage cases use
      the plan-07-B fixture gates (VFX-HC1G00 on 5 V for "carried, fine",
      VFX-AUC1G00, 3.6 V input max, for the input problem) instead of
      SN74HC00N, plus the reverse case (cathode on 5 V carries nothing).
      Checked: `diode_test` → 17 PASS, `0 failure(s)`; `library_check` →
      `ok   valid diode parts` and 9 `ok` error cases (unknown type, empty
      `diodes`, unknown pin, `input` pin, inout pin in no diode, `vz` on a
      rectifier, missing `vr`, `vr` 0, zener with `vr`), exit 0;
      `voltage_test` → the 3 diode PASS lines, `all passed`.

Acceptance: the kind loads, its errors are named, and the voltage check
carries through a diode.
  Check: `cmake --build /tmp/logic-p29 -j8 && ctest --test-dir /tmp/logic-p29` → `100% tests passed`, 30 programs (T0 + 1, T0 = 29), and `git diff --stat backend/tests/golden` → empty (est. 5 min).
  Result 2026-11-09: `100% tests passed, 0 tests failed out of 30`; `grep -c '^logic_test' backend/CMakeLists.txt` → 30; golden diff empty.
Commit: f33a538

### Phase 4 — Docs

- [x] `docs/parts-format.md`: kind table row, `### diode` (§ 5.1),
      `"diode"` in the intro's list of kinds.
- [x] `docs/backend.md`: diode links (§ 4, the table of § 4.3), the voltage
      carry, `diode_test` in the test list, `models/diode.c` in Layout.
      Checked: `grep -c '### `diode`' docs/parts-format.md` → 1;
      `grep -c 'diode_test' docs/backend.md` → 1. Final gate:
      `ctest --test-dir /tmp/logic-p29w` → `100% tests passed, 0 tests
      failed out of 30`; `git diff --stat backend/tests/golden` → empty.

Acceptance: the docs name the kind and the links.
  Check: `grep -c '### `diode`' docs/parts-format.md` → 1 and `grep -c 'diode_test' docs/backend.md` → ≥ 1 (est. 1 min).
Commit: 9147d40

## Validation Plan

- Tests: `diode_test` (engine and kind), `library_check` (loader errors),
  `voltage_test` (carry).
- Coverage check: `diode_test` builds circuits through the public API, so
  `resolve_cluster` runs in it (its ten cases all need a link).
- Runtime proof: the diode-OR run case (§ 6) is the end-to-end proof at this
  layer; the app proof is plan-29-M.
- Doc sync: `docs/parts-format.md`, `docs/backend.md`.
- Final gate (once, after Phase 4): `ctest --test-dir /tmp/logic-p29` →
  `100% tests passed`, T0 + 1; goldens unchanged (est. 5 min).

## Decisions

- **One-way logic** — owner, 2026-11-08: a 1 passes anode → cathode, a 0
  cathode → anode; strong 1 against strong 0 across a forward diode is a
  short. Settles § 4.
- **In the engine, not the model** — author (§ 2, § 3 Rejected).
- **Pins `inout`** — author: the switch's precedent; the trace harness
  drives them weak, so a diode part's trace records its passing (§ 2).
- **Tied same-name pins** — author: a link both ways (§ 5.2), not a closed
  contact, so ties need no `sim_set_contact` at start.
- **No analog checks** — author (§ 3 Rejected).

## Corrections

- **H does not depend on G** (2026-11-09) — the chain A → … → G → H in A's
  header was a serialization, not a data edge: H's engine, kind and tests
  use inline fixture devices (§ 6) and read nothing B–G produce
  (`footprints-*.json`, `specs.tsv`). H runs after A's gate, beside A–G;
  I still needs H (the kind) and B (footprints). The G-complete
  prerequisite row is moot.
- **Build dir** — the worktree's build dir is `/tmp/logic-p29w` (the
  commands' `/tmp/logic-p29` is the main checkout's).

## Summary

The risk is the resolver: a new resolution unit (the cluster) with a
two-pass reachability that must short, block and release exactly as the
§ 4.3 table says, without touching the path every existing circuit takes.
Left untouched: every part file but the new empty `diode.json`, every
golden line, the app beyond loading the new file.
