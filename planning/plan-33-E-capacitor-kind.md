# plan-33-E: Backend — the `capacitor` kind, the rating check, tests, docs

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-33-D

E adds the `capacitor` part kind: `PART_CAPACITOR`, the facts struct
`PartCapacitor`, the model file `backend/src/models/capacitor.c` (passive: it
drives nothing and lights nothing), the voltage check's new over-rating
problem, the listing line, a test program `capacitor_test`, and the docs.
`Parts/capacitor.json` ships with `"devices": []`. F and G add the parts.

The single behavioral outcome: a fixture capacitor rated 6.3 V with a pin on
a 12 V net gives exactly one voltage problem, kind `VOLTAGE_CAPACITOR_OVER`,
described `voltage: 12 V on pin 1 (+) of TESTCAP; it is rated 6.3 V`; the
same capacitor on 5 V gives none; every existing part's listing and trace
line stays unchanged.

References:

- `docs/backend.md` § Adding a kind (:280) and § Tests (:305).
- `backend/include/parts.h` (`PartKind` :24–28, flags :36, `PartLed` :65,
  `PartDiode` :131, the `Part` pointers :181–183).
- `backend/src/models/led.c` (the two-pin passive kind), `header.c` (:60, a
  model with no callbacks), `diode.c` (`optional_number`, `diode_load_part`
  :192).
- `backend/src/library/kinds.c` (`KINDS[]`), `kind.h` (helpers :99–161).
- `backend/src/voltage.c` (step 3 LED/diode carry :245–313, step 4 checks
  :420–466, `voltage_describe` :486–515), `backend/include/voltage.h` (:25–31).
- `backend/tests/support/listing.c` (:13–33 names, :85 facts),
  `listing_check.c` (:50–51), `trace_check.c` (:22), `library_check.c`,
  `voltage_test.c` (:338 the switch-rating cases), `CMakeLists.txt` (:174).
- `schematical/schematical/Simulation/VoltageCheck.swift` (:143–149: the
  `default` message the app gives an unknown kind).

## Prerequisites

plan-33-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-33-D complete | `ls planning/completed/plan-33-D-*.md` → exists | NOT MET (2026-11-11: planning) |
| No `capacitor` kind yet | `grep -c 'PART_CAPACITOR' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/capacitor.json` → no such file | MET (2026-11-11 at 8b1770b: 0, no file) |
| `PART_CAPACITOR` and `VOLTAGE_CAPACITOR_OVER` absent | `grep -rc 'PART_CAPACITOR\|VOLTAGE_CAPACITOR_OVER' backend/include` → 0 per file | MET (2026-11-11 at 8b1770b) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `ctest` passes T0 + 1 programs (T0 = 30 at 8b1770b, plan-33-A
  § Prerequisites); the new one is `capacitor_test`.
- `listing_check` and `trace_check` pass with the goldens unchanged:
  `git diff --stat backend/tests/golden` → empty after E (no shipped part is
  a capacitor yet).
- `docs/parts-format.md` has a `### capacitor` section and a kind-table row;
  `docs/backend.md` names the capacitor rating check.

### Non-goals (explicit constraints)

- **No change to any existing part's behavior or facts**: `parts.txt`,
  `traces.txt` and `footprints.txt` stay byte-identical. This is E's
  byte-identity gate for existing parts; the new behavior is checked by
  `capacitor_test` and `voltage_test`.
- No analog model: no charge, no RC delay, no ripple, no reverse-polarity
  check. A capacitor drives nothing in the logic simulation (§ 3 Rejected).
- No app change (plan-33-H). The app's voltage message for the new problem
  kind is its existing `default` case (`VoltageCheck.swift:146`), which
  reads "`<ref> pin 1 (+) gets 12 V; <part> is rated 6.3 V.`".
- No part-editor support (plan-33-A § Decisions).

## 2. Current State

- `PartKind` ends with `PART_DIODE` today (`parts.h:28`); `KINDS[]` lists 26
  kinds (`grep -c '_KIND,$' backend/src/library/kinds.c` → 26); the
  tests size their per-kind arrays by the last kind: `listing_check.c:50–51`
  (`PART_DIODE + 1`, twice) and `trace_check.c:22` (`TRACE_LAST_KIND
  PART_DIODE`) — `grep -rn 'PART_DIODE + 1\|TRACE_LAST_KIND PART_DIODE'
  backend/tests | wc -l` → 3.
- The LED (`led.c`) is the two-pin precedent: pins role `input`, `vcc`
  NULL, so the voltage check's step 3 skips it (`voltage.c:315` `if
  (part->vcc == NULL) continue;`) and step 4 finds nothing for it
  (`voltage.c:447–466`: every branch needs `vcc`, or a DIP switch).
- `header.c:60` is a model with no callbacks at all (pin count and create
  only); a capacitor needs the same.
- The DIP switch's contact rating is the precedent for a rating check:
  `VOLTAGE_SWITCH_OVER` (`voltage.h:30`), raised in step 4 for an `inout`
  pin above `dip_switch->rated_mv` (`voltage.c:456–461`), described at
  `voltage.c:509–512`, tested at `voltage_test.c:338–348`.
- Test programs: `grep -c '^logic_test' backend/CMakeLists.txt` → 30
  (2026-11-11 at 8b1770b).

### Verified properties

- **A passive two-pin part with `input` pins traces cleanly.** The trace
  harness drives `input` pins strong (`trace_core.c`, plan-29-H § 2), and
  `GENERIC-LED` (two `input` pins, passive) has a trace line today
  (`grep -c '^GENERIC-LED	' backend/tests/golden/traces.txt` → 1, a tab after the name).
- **The app needs no code for a new voltage problem kind.** Read
  `VoltageCheck.message` (`VoltageCheck.swift:122–149`): unknown kinds fall
  to `default`, which prints `is rated <max> V` from `issue.maxMV`.
- **The rating check is new code only.** Step 4's new branch is reached only
  for `part->kind == PART_CAPACITOR` (no existing part has that kind), so no
  existing problem list can change.

## 3. Design

### 3.1 Facts (`parts.h`)

```c
typedef struct { const char *type; int polarized; uint64_t capacitance_ff;
                 uint32_t rated_mv; const char *dielectric;
                 const char *tolerance; } PartCapacitor;
```

`PART_CAPACITOR` appended as the **last** `PartKind` (after `PART_DIODE`
today, or after any kind another plan appended first: plan-34, resistors,
was being planned on 2026-11-11); `const PartCapacitor *capacitor;` last
among the `Part` kind pointers. `capacitance_ff` is femtofarads (0.1 pF = 100 fF;
4700 µF = 4.7e12 fF fits `uint64_t`). `rated_mv` 0 = no rating (the generic
part). `dielectric` and `tolerance` are NULL when not given.

### 3.2 Device and part keys (`models/capacitor.c`)

Device keys:

| Key | Value |
|---|---|
| `"type"` | `"ceramic"`, `"aluminum"`, `"aluminum-polymer"`, `"tantalum"`, `"tantalum-polymer"` or `"niobium-oxide"`; anything else is `unknown capacitor type "X"` |
| `"farads"` | the capacitance, a number above 0 and at most 10 (`"farads" must be above 0 and at most 10`); one device is one value (its `function` is that value, plan-33-F § 3) |
| `"positive"`, `"negative"` | pin names (exactly one pin each): required for every type but `ceramic` (`missing "positive"`), rejected for `ceramic` (`a ceramic capacitor has no polarity`) |

Every pinout has exactly 2 pins (`a capacitor has 2 pins`), both `input`
(`capacitor pin "X" must be input`). `polarized` = type ≠ ceramic.

Part keys:

| Key | Value |
|---|---|
| `"volts"` | optional: rated DC voltage, above 0 and at most 10000 → `rated_mv` |
| `"dielectric"` | optional string, `ceramic` only (`"dielectric" is only for a ceramic capacitor`): the code as DigiKey and the datasheet give it, e.g. `"C0G"`, `"X7R"`, `"Y5V"` |
| `"tolerance"` | optional string as the datasheet prints it, e.g. `"±10%"`, `"±0.25 pF"`, `"-20%, +80%"` |

The model is `CAPACITOR_MODEL = {"capacitor", capacitor_pin_count,
capacitor_create, NULL, …}` (header's shape): it reads and drives nothing.

### 3.3 The rating check (`voltage.h`, `voltage.c`)

- `VOLTAGE_CAPACITOR_OVER` appended last to `VoltageProblemKind`, so
  existing values keep their numbers.
- Step 4, a new `else if` after the switch branch: `part->kind ==
  PART_CAPACITOR && part->capacitor->rated_mv > 0 && e.mv >
  part->capacitor->rated_mv` → `p.kind = VOLTAGE_CAPACITOR_OVER`, `min_mv`
  0, `max_mv` the rating. Either pin counts (a pin's net level against
  ground is the voltage across a part whose other pin is on ground, the
  decoupling case; § 3 Rejected).
- `voltage_describe`: `voltage: %g V on pin %s (%s) of %s; it is rated %g
  V` (the switch's text).

### 3.4 Listing (`tests/support/listing.c`)

`case PART_CAPACITOR: return "capacitor";` and the facts `type=<type>
ff=<capacitance_ff> rated=<rated_mv>`, then ` dielectric=<d>` and ` tol=<t>`
when set: `type=ceramic ff=100000000 rated=50000 dielectric=X7R tol=±10%`.

**Where the risk is:** small. The kind is passive; the one behavior is the
new check, reached only for capacitors. The goldens prove no existing part
moved.

### Rejected

- **An RC model** (delay or charge): the engine is discrete logic with no
  analog state; a capacitor on a logic net would need a time constant from
  a resistance the netlist does not have.
- **Checking the difference between the two pins' levels**: the voltage
  check knows net levels, not which pin is the reference, and a rating
  above the higher pin's level is never wrong for a part between a rail and
  ground. The single-pin rule cannot miss a real over-voltage on such a
  part, which is every decoupling and bulk capacitor in the app's circuits.
- **A reverse-polarity check**: a `+` pin on ground and `-` pin on a rail
  is a mistake worth catching, but it needs both pins' nets, which step 4
  checks one connection at a time; left for a later plan.
- **`farads` as a part key**: the drawer groups by value (owner decision,
  plan-33-A § Decisions), and the leaf is the device's `function`, so a
  device is one value.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The kind

- [ ] `backend/include/parts.h`: `PART_CAPACITOR`, `PartCapacitor`, the
      `capacitor` pointer (§ 3.1).
- [ ] `backend/src/models/capacitor.c`: keys, `load_device` (type, farads,
      polarity pins), `bind_pinout` (2 pins, both `input`; `positive` /
      `negative` bound with `kind_pin`), `load_part` (volts, dielectric,
      tolerance), the empty model (§ 3.2). Add it to the backend sources in
      `backend/CMakeLists.txt` beside `diode.c`.
- [ ] `backend/src/library/kinds.c`: `extern const PartKindOps
      CAPACITOR_KIND;` and `&CAPACITOR_KIND,` as the last `KINDS[]` entry.
- [ ] `schematical/schematical/Parts/capacitor.json`: `{"format":
      "schematical-parts", "version": 1, "kind": "capacitor", "devices":
      []}`; the app bundles every file in `Parts/` (the diode precedent,
      plan-29-H).
- [ ] Tests' kind arrays: `listing.c` name and facts (§ 3.4); the three
      last-kind references (`listing_check.c` `<last> + 1` twice,
      `trace_check.c` `TRACE_LAST_KIND <last>`) move from the current last
      kind to `PART_CAPACITOR` (`grep -rn 'TRACE_LAST_KIND PART_\|\[PART_[A-Z_]* + 1\]'
      backend/tests` shows them: 3 lines today).

Acceptance: the existing tests pass with the goldens untouched.
  Check: `cmake --build /tmp/logic-p33 -j8 && ctest --test-dir /tmp/logic-p33 -R 'listing_check|trace_check|library_check'` → 3 passed, and `git diff --stat backend/tests/golden` → empty (est. 3 min).
Commit: —

### Phase 2 — The rating check

- [ ] `backend/include/voltage.h`: `VOLTAGE_CAPACITOR_OVER` last.
- [ ] `backend/src/voltage.c`: the step-4 branch and the description (§ 3.3).
- [ ] `backend/tests/voltage_test.c`: an inline fixture capacitor
      (`TESTCAP`, type tantalum, `+`/`-` pins, `volts` 6.3): `+` on a 12 V
      supply net, `-` on GND → one `VOLTAGE_CAPACITOR_OVER`, `mv` 12000,
      `max_mv` 6300, pin 0, and `voltage_describe` gives the § Goal text;
      on 5 V → no problem; a ceramic fixture with no `volts` on 12 V → no
      problem.

Acceptance: the new cases pass and the old ones still do.
  Check: `cmake --build /tmp/logic-p33 -j8 && ctest --test-dir /tmp/logic-p33 -R voltage_test` → passed (est. 2 min).
Commit: —

### Phase 3 — `capacitor_test`, loader cases, docs

- [ ] `backend/tests/capacitor_test.c` and `logic_test(capacitor_test)` in
      `backend/CMakeLists.txt`: an inline device file with one ceramic and
      one tantalum device loads; `PartCapacitor` facts read back
      (`capacitance_ff` 100000000 for `"farads": 1e-7`, `rated_mv` 50000,
      `polarized` 0 / 1, `dielectric` `X7R`, `tolerance` `±10%`); a sim with
      the capacitor between two driven nets runs 1 µs and its nets keep the
      drivers' values (it drives nothing).
- [ ] `backend/tests/library_check.c`: one rejection case per message of
      § 3.2 (unknown type, farads 0 and 11, ceramic with `positive`,
      tantalum without `positive`, 3 pins, an `inout` pin, `dielectric` on a
      tantalum part, `volts` 0).
- [ ] `docs/parts-format.md`: `| capacitor.json | capacitor |` in the kind
      table (add the row only if absent: `grep -c '^| `capacitor.json`'
      docs/parts-format.md` → 0 before; plan-32-C Phase 5 also edits that
      table and adds only missing rows); a `### capacitor` section after
      `### diode` with § 3.2's tables and the rating check; the file list in
      the first paragraph gains "capacitor".
- [ ] `docs/backend.md`: the `voltage_test` entry (:369–374, "…, switch
      rating, ordering, …") gains "capacitor rating"; the `capacitor_test`
      entry is added to the test list beside `diode_test`'s.

Acceptance: T0 + 1 programs pass.
  Check: `cmake -S backend -B /tmp/logic-p33 && cmake --build /tmp/logic-p33 -j8 && ctest --test-dir /tmp/logic-p33` → `100% tests passed, 0 tests failed out of 31` (est. 3 min).
Commit: —

## Validation Plan

- Tests: `capacitor_test` (load, facts, passive run), `library_check`
  (every rejection), `voltage_test` (the rating check both ways).
- Coverage check: `ctest -N --test-dir /tmp/logic-p33 | grep -c
  capacitor_test` → 1 (the new program is in the suite).
- Runtime proof: Phase 2's `voltage_test` case is the circuit-level proof;
  the app-level one is plan-33-H.
- Doc sync: `docs/parts-format.md`, `docs/backend.md` (§ Tests list).
- Final gate: plan-33-I.

## Decisions

- **Passive, with a rating check** — author: a capacitor is not a logic
  element; the rating check mirrors the DIP switch's (the one existing
  rating) and catches the one mistake a logic schematic can make with a
  capacitor's facts. Rejected alternatives in § 3.
- **Pins `input`, names `1`/`2` or `+`/`-`** — author: `input` is the LED's
  passive role (never driven, never an output clash); `+`/`-` are what the
  datasheets print.
- **Femtofarads** — author: integers in the facts struct like every other
  kind (`vr_mv`, `io_ma`); 1 fF resolves 0.1 pF.

## Corrections

(Filled in during execution.)

## Summary

E is a small, passive kind plus one new voltage-check branch reached only by
capacitors; the goldens prove nothing else moved. Untouched: the engine, the
app, every existing part file.
