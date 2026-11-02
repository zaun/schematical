# plan-14-A: WDC peripherals and the W65C816S — groundwork (datasheets, the `pio` kind, the 65xx bus helper)

Last updated: 2026-11-01
Overall Effort: huge (> 3d) — plan-14 is A → B → C → D → E → F → G → H → I → J
Effort: medium (1h–2h)
Depends on: plans 11, 12 and 13 complete (§ Prerequisites)

plan-14 adds the WDC 65xx parts that go with the W65C02S already in the
catalog: the **W65C21** PIA, the **W65C22** VIA, the **W65C51N** ACIA and the
**W65C816S** 16-bit processor. Each is a full pin-level model at the
datasheet's times. A W65C02S board with a VIA, a PIA and an ACIA runs a
program that drives the ports, counts with the timers and prints through the
ACIA to a terminal; a W65C816S board runs a native-mode program that writes
above 64 KB through a bank latch. The W65C134S and W65C265S
microcontrollers are skipped for now (`datasheets/skipped-parts.md`).

The single behavioral outcome of plan-14: the library gains 12 parts (4
W65C21, 3 W65C22, 3 W65C51N, 2 W65C816S); each runs its datasheet behavior
in a model test and a golden trace; `ctest` passes T0 + 1 programs (the new
`pio_test`); and the app lists the PIA and VIA under **Interface** and shows a
W65C816S's registers in the processor window.

| Letter | Content | Effort |
|---|---|---|
| **A** | 4 datasheets; the `pio` kind (no models yet); the 65xx bus helper `bus65`; `pio_test` | medium |
| B | `pia6521` model (W65C21N/S); 4 parts | large |
| C | `via6522` model part 1: ports, handshakes, timers, interrupts | large |
| D | `via6522` part 2: the shift register; 3 W65C22 parts | medium |
| E | W65C51N on plan-12's `acia6551` (`"w65c51n"` key), `LQFP-32-7X7`, 3 parts; the W65C02S I/O board | large |
| F | app: the `pio` kind (catalog, facts, Interface drawer, symbol) | medium |
| G | the vendored 65816 core (jsmooch, MIT), fixed in its generator; SingleStepTests/65816 subset; `cpu_core_test` | large |
| H | the `65816` bus adapter in the `cpu` kind; 24-bit state and tracker; `cpu_test` cases | large |
| I | W65C816S parts (2); the W65C816S board; docs | medium |
| J | app: the W65C816S in the processor window and facts; plan-14 landing tier | medium |

Outcome of A: `datasheets/pio/w65c21.pdf`, `datasheets/pio/w65c22.pdf`,
`datasheets/controller-uart/w65c51n.pdf` and `datasheets/cpu/w65c816s.pdf`
match §4.1; the loader knows kind `pio` (a device whose `"model"` is not a
registered model is refused); `bus65.c` turns PHI2, the chip selects, RWB
and the register selects into register read and write events at the
datasheet's times, proven by `pio_test` on a test bench that calls the
helper directly.

References:

- `AGENTS.md` (datasheets, in-stock rule, part files, footprints, goldens).
- `docs/backend.md` (§ Adding a kind, § Tests), `docs/parts-format.md`
  (§ Devices, § Parts, § `cpu`, § `serial` as plan-12 leaves it).
- `planning/completed/plan-09-A-cpu-cores.md` … `plan-09-E-cpu-app.md` (the
  cpu kind, the vendored cores, the processor window; the WDC precedents:
  no QFP without an outline, DigiKey stock exception).
- `planning/completed/plan-12-C-serial-core.md` (§3.1 a kind with a model
  table — the pattern `pio` copies; §3.3 host bus helpers) and
  `plan-12-H-classic.md` (§3.4 the `acia6551` model E extends).
- `datasheets/skipped-parts.md` (what plan-14 leaves out, and why).

## Prerequisites

This table is the gate for all of plan-14. B–J point here and add only rows
that check the letter before them.

| Must be true | Command | Status |
|---|---|---|
| plan-11 complete | `ls planning/plan-11-*.md 2>/dev/null` → no output; `ls planning/completed/plan-11-D-charlcd-app.md` → exists | NOT MET (2026-11-01: `planning/plan-11-A-groundwork.md` … `plan-11-D-charlcd-app.md` open) |
| plan-12 complete (the `serial` kind, `acia6551`, `PLCC-28`, the terminal window) | `ls planning/plan-12-*.md 2>/dev/null` → no output; `ls planning/completed/plan-12-K-app.md` → exists | NOT MET (2026-11-01: plan-12-A … K open) |
| plan-13 complete | `ls planning/plan-13-*.md 2>/dev/null` → no output; `ls planning/completed/plan-13-C-bus-editing.md` → exists | NOT MET (2026-11-01: plan-13-A … C open) |
| The `acia6551` model and `PLCC-28` exist | `ls backend/src/models/serial_acia6551.c && grep -c '"id":"PLCC-28"' schematical/schematical/Parts/footprints.json` → the path, `1` | NOT MET (2026-11-01: no such file) |
| No `pio` kind yet | `ls backend/src/models/pio.c schematical/schematical/Parts/pio.json 2>/dev/null` → no output | MET (2026-11-01) |
| Backend tests green | `cmake -S backend -B /tmp/logic-p14 && cmake --build /tmp/logic-p14 -j8 && ctest --test-dir /tmp/logic-p14` → `100% tests passed`; record the program count as **T0** | not run (plans 11–13 change the count) |

At the start of A also record **P0** = `wc -l < backend/tests/golden/parts.txt`
(it equals `traces.txt`) and **F0** = `wc -l < backend/tests/golden/footprints.txt`.
Plans 11–13 change them, so they are measured then (at 403651a they are
7157 and 195).

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify — a
> prerequisite recorded NOT MET may have landed since, and one recorded MET may
> have regressed.
>
> **If you stop, report the current status of *all* prerequisites** — not only
> the one that blocked you.

## 1. Goal

- `shasum -a 256` of the four PDFs matches §4.1.
- `library_check` refuses `{"kind":"pio","model":"nope",…}` with
  `unknown pio model "nope"` and a `pio` device without `"model"` with
  `"model" is required`.
- `pio_test` → every `bus65` case of §5 `ok`.
- `ctest` → T0 + 1 programs, all passing; goldens unchanged.

### Non-goals (explicit constraints)

- No part, no model, no golden line, no app change in A.
- `serial_bus.c` (plan-12) and the `acia6551` model's own bus code are not
  changed or moved onto `bus65`. The ACIA stays a `serial` part.
- The vendored floooh/chips headers stay unmodified (plan-09-A); the 65816
  core is a separate vendored directory (G).

## 2. Current State

(As of 403651a, before plans 11–13; the prerequisites make plans 11–13's
state the starting point.)

- Kinds: `KINDS[]` in `backend/src/library/kinds.c` lists 14 kinds
  (`grep -c '_KIND,' backend/src/library/kinds.c` → 14); plan-11 adds
  `charlcd`, plan-12 `serial` and the supply `pull` model. `PartKind` in
  `backend/include/parts.h:25-27` ends at `PART_CPU`.
- The pattern for a kind with several models is plan-12-C §3.1: `serial.c`
  checks `"model"`, hands the device JSON to the model (unknown keys refused
  with the model's name), and forwards every `SimComponentType` hook to the
  instance's model. `pio` copies it.
- No 65xx peripheral exists. The only PHI2-bus slave planned before plan-14
  is plan-12-H's `acia6551`, whose bus logic lives in its own file.
- WDC chips are not sold by DigiKey: `scripts/digikey.js "W65C02S6TPG-14"`
  → 0 parts, and `"W65C22"`, `"W65C21"` → only WDC boards (W65C02EDU,
  W65C816EDU), `"W65C51"` → 0 parts (2026-11-01, 4 API calls). plan-09-D
  met the same and the owner allowed it.

### Measured populations

| What | Count | Command / source |
|---|---|---|
| Parts plan-14 adds | 12: W65C21N6TPG-14, W65C21N6TPLG-14, W65C21S6TPG-14, W65C21S6TPLG-14; W65C22N6TPG-14, W65C22S6TPG-14, W65C22S6TPLG-14; W65C51N6TPG-14, W65C51N6TPLG-14, W65C51N6TQG-14; W65C816S6TPG-14, W65C816S6TPLG-14 | wdc65xx.com/where-to-buy (WDC stock 01/01/2024, read 2026-11-01): 19 codes for the six devices, minus the 4 MCU codes, the 2 zero-stock codes (W65C22N6TPLG-14, W65C265S8QG-8, the latter an MCU code), and the 2 QFP-44 codes without an outline (W65C22S6TQG-14, W65C816S6TQG-14) |
| Footprints the parts use | PDIP-40-600 (5 parts), PLCC-44 (4), PDIP-28-600 (1), PLCC-28 (1, from plan-12-H), LQFP-32-7X7 (1, new in E) | `grep -o '"id":"[^"]*"' footprints.json` → PDIP-40-600, PLCC-44, PDIP-28-600 present; LQFP-32 absent |
| Golden lines plan-14 adds | parts +12, traces +12, footprints +1 | from the rows above |
| ctest programs plan-14 adds | 1 (`pio_test`) | B–D add cases to it; E, H, I add cases to `serial_test` and `cpu_test`; G to `cpu_core_test` |

### Verified properties

- **PDF facts** (read with `pdftotext -layout` and page images, 2026-11-01):
  pinouts `w65c21.pdf` p.3, `w65c22.pdf` p.27 (PDIP, PLCC) and p.28 (QFP),
  `w65c51n.pdf` p.6, `w65c816s.pdf` p.10 (PDIP, PLCC) and p.11 (QFP); package
  outlines: only `w65c21.pdf` p.19 (PDIP-40) and `w65c51n.pdf` p.23 (PDIP-28),
  p.23 (PLCC-28), p.25 (LQFP-32). `w65c22.pdf` and `w65c816s.pdf` have no
  outline page (`grep -n -i "dimension" *.txt` → none in either).
- **WDC parts exist as named.** Each of the 12 codes is on wdc65xx.com's
  where-to-buy table with WDC stock > 0 (§ Measured populations).

## 3. Design Overview

Three independent pieces, each separately testable:

1. **Datasheets** placed where the kinds that use them look (§4.1).
2. **The `pio` kind** (`backend/src/models/pio.c`, `pio.h`): `PART_PIO`,
   `PartPio { const char *model; const char *grade; uint64_t max_hz; }` on
   `Part`, a model
   table (`PioModel`), and `"grades"` like `serial`. A has no model in the
   table; B registers `pia6521`, C `via6522`.
3. **`bus65`** (`backend/src/models/bus65.c`, `bus65.h`): the 65xx
   peripheral bus every `pio` model uses. It is a helper the model calls from
   its `pin_changed` and timer hooks, not a component.

Risk is low: nothing in the library uses `pio` until B. The risk in A is
`bus65`'s timing rules (§4.3); `pio_test` checks each rule on a bench, and
B–D reuse them unchanged.

Rejected alternatives:

- **Put the PIA and VIA in the `serial` kind.** The VIA's shift register is
  synchronous and neither chip has a UART channel; `serial`'s terminal API
  and line engine would be dead weight, and its facts (channels) mean
  nothing for them.
- **One kind per chip** (`pia`, `via`). Two kinds with the same grades,
  bus, loader and app details; the model table gives the same result with
  one kind.
- **Move `acia6551` onto `bus65`.** It would edit a finished plan-12 model
  for no behavior change; E only adds the W65C51N variant to it.

## 4. Detailed Design

### 4.1 Datasheets

| File | Document | URL | Bytes | SHA-256 |
|---|---|---|---|---|
| `datasheets/pio/w65c21.pdf` | WDC W65C21 datasheet, March 18 2024, 20 pp | https://www.wdc65xx.com/wdc/documentation/w65c21.pdf | 456912 | dd5670efabe6513b5c323ca2d32f33600bc6b42f472371a72dc95befb6432297 |
| `datasheets/pio/w65c22.pdf` | WDC W65C22 datasheet, February 16 2024, 50 pp | https://www.wdc65xx.com/wdc/documentation/w65c22.pdf | 612281 | 8827a42999a1aa368a3c5a782f61d4c2e235f925790b34a4f3a265ad33cbbd1b |
| `datasheets/controller-uart/w65c51n.pdf` | WDC W65C51N datasheet, March 18 2024, 26 pp | https://www.wdc65xx.com/wdc/documentation/w65c51n.pdf | 799941 | 248096fb285a9050d356bf20aab22c2ba1d7132bac66a0cd14a40c925220e3a8 |
| `datasheets/cpu/w65c816s.pdf` | WDC W65C816S datasheet, March 13 2024, 55 pp | https://www.wdc65xx.com/wdc/documentation/w65c816s.pdf | 1532025 | b9177e1b045d2c8a801d1b23619abb4e5b29b88868fa491df7296dbc0b13447e |

`datasheets/pio/` is a new group: parallel I/O peripherals. The W65C51N
goes with the other UARTs, the W65C816S with the CPUs. `w65c134s.pdf` and
`w65c265s.pdf` are not added (`datasheets/skipped-parts.md`).

### 4.2 The `pio` kind

- `parts.h`: `PART_PIO` appended after the last kind plan-12 leaves;
  `typedef struct { const char *model; const char *grade; uint64_t max_hz; } PartPio;`
  and `const PartPio *pio;` on `Part`. `max_hz` = floor(10^12 / `tcyc` in ps),
  as `PartCpu.max_hz`; every model's first timing name is `tcyc`.
- Device keys: `"model"` (required) plus the model's keys; `"grades"`
  (required): grade name → the model's timing names in ns (a missing name is
  0) plus optional `"vcc"`, `"inputMax"`, `"note"`. Part keys: `"grade"` (a
  key of `grades`, required), optional `"vcc"`, `"inputMax"` — the same as
  `serial` (plan-12-C §3.1), through the same `KindContext` helpers.
- `PioModel { name; device_keys; load_device; bind; timing_names;
  SimComponentType callbacks; pin_open_drain }`. `pio.c` checks `"model"`
  against the table (`unknown pio model "<m>"`), checks the device's keys
  against the kind's common keys plus the model's, and forwards every hook.
- Pin groups (presentation, `docs/parts-format.md` § Pinouts): D0–D7
  `"data"`, PA0–PA7 `"port A"`, PB0–PB7 `"port B"`; every other pin the
  default group.
- `listing_check`: kind column `pio`, facts column `model=<m> grade=<g> max_hz=<n>`
  (as `cpu`'s `core=65c02 grade=14 max_hz=14285714`);
  `listing_kind_name(PART_PIO)` → `pio`; `TRACE_LAST_KIND` → `PART_PIO`.
- `docs/parts-format.md` gains § `pio` with the keys above and "Models: B
  and C add `pia6521` and `via6522`".
- The app is not touched until F. A `PART_PIO` value the app does not know
  falls to its default block symbol, as `PART_SERIAL` did before plan-12-K.

### 4.3 `bus65` — the 65xx peripheral bus

The W65C21 and W65C22 datasheets give the same bus: chip selects and
register selects set up before PHI2 rises, a cycle that happens while PHI2
is high, read data valid `tCDR` after PHI2 rises and held `tHR` after it
falls, write data latched at PHI2 fall with setup `tDCW` and hold `tHW`
(`w65c21.pdf` p.8; `w65c22.pdf` p.40–41, Figures 4-1, 4-2 p.44).

```c
typedef struct {
    size_t phi2, rwb, data[8];
    size_t cs_high[2], cs_high_count;   /* active-high selects (W65C21 CS0, CS1; W65C22 CS1) */
    size_t cs_low[2], cs_low_count;     /* active-low selects (CS2B) */
    size_t rs[4], rs_count;             /* register selects, RS0 first */
} Bus65Pins;

typedef struct {
    SimTime tacr, tcar, tcdr, thr, tacw, tcaw, tdcw, thw;  /* ps, from the grade */
} Bus65Timing;

typedef struct {
    int (*read)(void *model, unsigned reg, uint8_t *value, uint8_t *known); /* 1 = drive */
    void (*write)(void *model, unsigned reg, uint8_t value, uint8_t known);
    void (*cycle)(void *model);   /* every PHI2 fall, after read/write: timers count here */
} Bus65Ops;
```

Rules, each a `pio_test` case (§5):

1. **The cycle.** At each PHI2 rise the helper samples the selects, RWB and
   RS. Selected = every `cs_high` reads 1 and every `cs_low` reads 0. A
   select, RWB or RS that changed less than `tacr` (read) / `tacw` (write)
   before the rise, or reads X, makes the cycle **unknown**.
2. **Read** (selected, RWB 1): `read()` is called at the rise; the data pins
   go X at rise + 0 and to the value at rise + `tcdr` (bits `known` = 0 stay
   X); at PHI2 fall they stay driven until fall + `thr`, then go Z. An unknown
   read cycle drives X for the same window and does not call `read()` (a
   read can have side effects: clearing flags).
3. **Write** (selected, RWB 0): the data pins are sampled at PHI2 fall; a
   data bit that changed less than `tdcw` before the fall is X. `write()` is
   called at the fall with the byte and its known mask. An unknown write
   cycle calls `write()` on every register with known = 0 (the model then
   makes that register X).
4. **Hold.** A select, RWB or RS that changes within `tcar` (read) /
   `tcaw` (write) after the PHI2 rise makes the cycle unknown from then on
   (a read already in progress drives X until fall + `thr`). The W65C21
   lists `tCAR = tCAW = 0`: no hold check. A data bit that changes within
   `thw` after the fall on a write is X in the byte passed.
5. **Not selected:** the data pins stay Z; nothing is called.
6. **PHI2 X:** every register the model owns goes unknown via
   `write(reg, 0, 0)` for each register; the data pins go X until PHI2 is
   known again.
7. `cycle()` runs at every PHI2 fall, selected or not, after `read`/`write`
   (the VIA's timers decrement on PHI2, `w65c22.pdf` §2.6).

Z on a select, RWB or RS reads as X (these pins have no defined open level
on either chip). The models handle RESB themselves.

## 5. Tests (`backend/tests/pio_test.c`, new ctest program)

`logic_test(pio_test)` in `backend/CMakeLists.txt`. A's cases drive
`bus65` through a test-only `SimComponentType` defined in the test file
(16 registers in an array; `read` returns the array, `write` stores it),
on the shared bench `tests/support/bench.{h,c}`, with the W65C22S 14 MHz
timing (`tacr` 10, `tcar` 10, `tcdr` 20, `thr` 10, `tacw` 10, `tcaw` 10,
`tdcw` 10, `thw` 10 ns; `w65c22.pdf` p.41).

| Case | Checks |
|---|---|
| A1 read | write 0x5A to reg 3, read reg 3: D0–D7 X at the rise, 0x5A at rise + 20 ns, Z at fall + 10 ns |
| A2 write | data valid 15 ns before the fall → stored; valid 5 ns before → X bits stored as unknown |
| A3 setup | RS changes 5 ns before the rise → unknown cycle: read drives X, `read()` not called |
| A4 hold | CS1 drops 5 ns after the rise on a read → X until fall + 10 ns |
| A5 not selected | CS2B high → bus Z, no callback |
| A6 PHI2 X | every register unknown; bus X while PHI2 X |
| A7 cycle hook | `cycle()` called once per PHI2 fall with and without a selected access |

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` with what remains for partial work; mark a moot
> task `- [x] ~~text~~ — moot: <evidence>`; fill each `Commit:` line when the
> phase lands; add any task you discover. **An unticked box means NOT DONE.**

### Phase 1 — Datasheets

- [ ] Download the four PDFs of §4.1 to their paths (`curl -sfL -o`).
- [ ] `shasum -a 256` each → the §4.1 value.

Acceptance: the four files exist and match.
  Check: `shasum -a 256 datasheets/pio/w65c2*.pdf datasheets/controller-uart/w65c51n.pdf datasheets/cpu/w65c816s.pdf` → the four §4.1 hashes (est. seconds).
Commit: —

### Phase 2 — The `pio` kind and `bus65`

- [ ] `backend/include/parts.h`: `PART_PIO`, `PartPio`, `Part.pio` (§4.2).
- [ ] `backend/src/models/pio.c`, `pio.h` (§4.2), empty model table;
      `bus65.c`, `bus65.h` (§4.3); `backend/src/library/kinds.c` +
      `&PIO_KIND`; `backend/CMakeLists.txt` adds the three sources.
- [ ] `listing_check`/`trace_check` kind name and `TRACE_LAST_KIND` (§4.2).
- [ ] `library_check`: the two refusals of §1.
- [ ] `backend/tests/pio_test.c` cases A1–A7; `logic_test(pio_test)`.
- [ ] `docs/parts-format.md` § `pio`; `docs/backend.md` § Layout (`pio.c`,
      `bus65.c`) and § Tests (`pio_test`).

Acceptance: `pio_test` A1–A7 pass; `library_check` refuses both devices;
`ctest` T0 + 1, goldens unchanged.
  Check: `cmake --build /tmp/logic-p14 -j8 && ctest --test-dir /tmp/logic-p14` → `100% tests passed`, T0 + 1 programs; `git diff --stat backend/tests/golden` → empty (est. 2 min).
Commit: —

## Validation Plan

- Tests: `pio_test` A1–A7 (`bus65`), `library_check` (`pio` refusals).
- Doc sync: `docs/parts-format.md` § `pio`; `docs/backend.md` layout and
  tests.
- The landing tier is plan-14-J's.

## Decisions

- **Stock source: WDC's own list.** DigiKey sells no WDC chips. Decided by
  the owner, 2026-11-01: use wdc65xx.com/where-to-buy as the stock source (as
  plan-09-D's W65C02S exception) and leave out the codes WDC shows with 0
  stock (W65C22N6TPLG-14, W65C265S8QG-8), recorded in
  `datasheets/skipped-parts.md`.
- **plan-14 starts after plans 11, 12 and 13.** Decided by the owner,
  2026-11-01. One gate; the W65C51N needs plan-12's `serial` kind,
  `acia6551` and `PLCC-28`.
- **The W65C816S is included,** on the MIT jsmooch core fixed through its
  generator (G). Decided by the owner, 2026-11-01. Third-party CPU code must
  be MIT or zlib (owner).
- **The W65C134S and W65C265S are skipped for now.** Decided by the owner,
  2026-11-01; reasons in `datasheets/skipped-parts.md`.
- **No QFP-44 parts** (W65C22S6TQG-14, W65C816S6TQG-14): WDC publishes no
  outline; plan-09-A's owner decision for W65C02S6TQG-14 applies.
- Author: the PIA and VIA are one kind `pio` with a model table, in the
  drawer group **Interface** (plan-12-K's group for bus peripherals) with
  functions `PIA` and `VIA` (§3).

## Corrections

(none yet)

## Summary

A is file drops plus a small kind shell and one helper. The engineering risk
of plan-14 sits in C–D (the VIA's timers and shift register, many modes) and
G–H (fixing a third-party core and a bus adapter that multiplexes the bank
address on the data bus). Nothing existing changes behavior in A.
