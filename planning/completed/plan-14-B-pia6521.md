# plan-14-B: The `pia6521` model and the W65C21 parts

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-14-A

B adds the W65C21 peripheral interface adapter: a `pio` model `pia6521` with
both ports, the four control lines, the two interrupt outputs and the
datasheet's bus and peripheral times, and the 4 W65C21 parts.

Outcome of B: `pio_test` cases B1–B10 pass; `pio.json` holds devices
`W65C21N` and `W65C21S` with 2 parts each; `parts.txt` and `traces.txt` gain
4 lines each, `footprints.txt` none.

References: `datasheets/pio/w65c21.pdf` — pinouts p.3, AC timing p.8,
timing figures p.9–10, Table 1 (control registers) and Table 2 (register
addressing) p.11, buffers p.11, signal and functional description p.12–17,
Table 3 (control line modes) p.18, ordering p.20; plan-14-A §4.2 (`pio`
kind), §4.3 (`bus65`).

## Prerequisites

plan-14-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-14-A landed | `grep -n 'pio_test' backend/CMakeLists.txt && ls backend/src/models/bus65.c` → a line, the path | MET (2026-11-07: `135:logic_test(pio_test)`, `backend/src/models/bus65.c`) |

## 1. Goal

- `pio_test` B1–B10 `ok`.
- Goldens: + 4 `parts.txt`, + 4 `traces.txt`; no removed or changed line.
- `ctest` 100%, T0 + 1 programs.

### Non-goals

- No `bus65` change (a rule that turns out wrong is a Correction in A and a
  fix there, with A's cases updated).
- No app change. No 6820/6821/6520 parts (not WDC, not in stock).

## 2. Current State

After A: kind `pio` with an empty model table; `bus65` tested on a bench.
`DRIVE_WEAK` exists in the engine (`backend/src/engine/sim.h:17`); plan-12-A
gives it its first users (pull parts) and the voltage check rules for it.

## 3. Design

### 3.1 Device keys (`pio.json`)

```json
{"id": "W65C21N", "function": "PIA", "drawer": ["Interface"],
 "description": "Peripheral interface adapter, NMOS-compatible (WDC W65C21N)",
 "note": "W65C21 (datasheets/pio/w65c21.pdf): pinouts p.3, AC p.8, registers p.11, control lines p.13-18",
 "model": "pia6521",
 "phi2": "PHI2", "rw": "RWB", "reset": "RESB",
 "data": ["D0","D1","D2","D3","D4","D5","D6","D7"],
 "csHigh": ["CS0","CS1"], "csLow": ["CS2B"], "rs": ["RS0","RS1"],
 "pa": ["PA0","…","PA7"], "pb": ["PB0","…","PB7"],
 "ca1": "CA1", "ca2": "CA2", "cb1": "CB1", "cb2": "CB2",
 "irqa": "IRQAB", "irqb": "IRQBB",
 "grades": {"14": {"tcyc": 70, "tc": 35, "tacr": 8, "tcar": 0, "tpcr": 10, "tcdr": 20,
                   "thr": 5, "tacw": 8, "tcaw": 0, "tdcw": 10, "thw": 5, "tcpw": 20,
                   "tca2": 20, "tca2h": 20, "trs2": 25, "tcb2": 70, "tcb2h": 20, "tdc": 5,
                   "pwi": 70, "trs3": 20, "tir": 25, "vcc": [4.5, 5.5]}},
 "pinouts": {"p40": […], "plcc44": […]}, "parts": [ … ]}
```

`W65C21S` is the same with description "Peripheral interface adapter,
speed and power improved (WDC W65C21S)". The two dies differ only in their
port buffers' drive currents (p.11 Figures 9–10, p.16), which the simulator
does not model; the timing table (p.8) is one table for both.

Timing names → datasheet (p.8): `tcyc` tCYC, `tc` PHI2 pulse width (high and
low minimum), `tacr`/`tcar`/`tpcr`/`tcdr`/`thr` read timing, `tacw`/`tcaw`/
`tdcw`/`thw`/`tcpw` write timing, `tca2` PHI2 low → CA2 low, `tca2h` PHI2 low
→ CA2 high (tRS1, pulse mode), `trs2` CA1/CB1 active → CA2/CB2 high (tRS2),
`tcb2` PHI2 high → CB2 low, `tcb2h` PHI2 high → CB2 high (tRS1 of the CB2
row), `tdc` peripheral data valid → CB2 low (minimum), `pwi` interrupt input
pulse width (minimum; printed in the Max column, read as a minimum as for
every other 65xx PIA), `trs3` interrupt response, `tir` interrupt clear.
`tcyc` and `tc` must be > 0. `PartPio.grade` = `"14"`, `max_hz` = 14285714.

Pinouts (p.3), pin: name.

- `p40`: 1 VSS, 2–9 PA0–PA7, 10–17 PB0–PB7, 18 CB1, 19 CB2, 20 VDD, 21 RWB,
  22 CS0, 23 CS2B, 24 CS1, 25 PHI2, 26 D7, 27 D6, 28 D5, 29 D4, 30 D3, 31 D2,
  32 D1, 33 D0, 34 RESB, 35 RS1, 36 RS0, 37 IRQBB, 38 IRQAB, 39 CA2, 40 CA1.
- `plcc44`: 1 VSS, 2 NC, 3 PA0, 4 PA1, 5 PA2, 6 PA3, 7 PA4, 8 PA5, 9 PA6,
  10 PA7, 11–17 PB0–PB6, 18 PB7, 19 CB1, 20 CB2, 21 VDD, 22 NC, 23 RWB,
  24 CS0, 25 CS2B (printed "CS2"), 26 CS1, 27 NC, 28 PHI2, 29 D7, 30 D6,
  31 D5, 32 D4, 33 D3, 34 D2, 35 D1, 36 D0, 37 RESB, 38 RS1, 39 RS0, 40 NC,
  41 IRQBB, 42 IRQAB, 43 CA2, 44 CA1.

Pin roles: IRQAB/IRQBB open-drain outputs (`pin_open_drain` → 1, p.15);
PA, PB, CA2, CB2 `inout`; D0–D7 `inout`; the rest inputs, power, ground, nc.
Groups per plan-14-A §4.2 (`data`, `port A`, `port B`).

Parts (p.20; package codes P = PDIP-40, PL = PLCC-44):

| Part | Device | package | pinout | footprint |
|---|---|---|---|---|
| W65C21N6TPG-14 | W65C21N | DIP | p40 | PDIP-40-600 |
| W65C21N6TPLG-14 | W65C21N | PLCC | plcc44 | PLCC-44 |
| W65C21S6TPG-14 | W65C21S | DIP | p40 | PDIP-40-600 |
| W65C21S6TPLG-14 | W65C21S | PLCC | plcc44 | PLCC-44 |

Each part: `"grade": "14"`.

### 3.2 Registers (p.11 Tables 1–2, p.13–14)

Per side (A, B): CR (control), DDR, OR (output register). RS1 picks the
side; RS0 = 1 → CR; RS0 = 0 → OR when CR bit 2 = 1, DDR when CR bit 2 = 0.
(The p.11 text says it the other way round once; Table 2 and every other
6821-family sheet agree with this rule, and the description on p.14 does
too.)

- CR bits: 0 C1 IRQ enable; 1 C1 active edge (1 = rising); 2 DDR/OR select;
  3–5 C2 control (§3.4); 6 IRQx2 flag; 7 IRQx1 flag. Bits 6–7 are read-only
  (writes ignore them).
- Writes reach the register at the PHI2 fall (`bus65`); a PA/PB output that
  changes goes to its new level at fall + `tcpw` (p.8).
- **Read OR A:** the PA pins, all 8 bits, sampled at the PHI2 rise; a pin that
  changed less than `tpcr` before the rise reads X (p.8, p.16–17: port A
  reads the pin, even on an output bit).
- **Read OR B:** output bits (DDRB = 1) read ORB; input bits read the PB pin
  as for port A (p.17).
- Reading OR A clears CRA bits 6 and 7; reading OR B clears CRB bits 6 and 7
  (p.15). IRQ goes high `tir` after the read cycle's PHI2 fall when no other
  enabled flag holds it.
- An unknown access (`bus65` known = 0) makes the addressed register X; an
  unknown read of OR makes the side's flags X.

### 3.3 Ports

- DDR bit 1: the pin is driven (strong) from OR. DDR bit 0: input.
- Port A inputs and CA2 as input: driven **weak 1** (`DRIVE_WEAK`): both dies
  have pull-ups on port A (p.16 "two active pull-up transistors", the
  W65C21S 50 µA input pull-up), so an open PA line reads 1 for every part on
  the net.
- Port B inputs and CB2 as input: not driven (p.16: high-impedance input).
- Z on an input the model reads: PA → 1 (its own pull-up; the net is never Z
  while the weak drive is on), PB/CB1/CB2/CA1 → X.

### 3.4 Control lines (Table 3, p.18; p.14)

- **CA1, CB1** (inputs): an active edge (CR bit 1) sets CR bit 7. IRQx goes
  low `trs3` after the edge when CR bit 0 = 1. A pulse narrower than `pwi`
  (the input returns within `pwi` of the active edge) sets bit 7 to X
  instead (the chip may or may not have seen it). CA1/CB1 X: bit 7 X.
- **CA2, CB2, CR bit 5 = 0** (input): CR bit 4 picks the active edge; it sets
  CR bit 6; CR bit 3 enables it onto IRQx.
- **CR bit 5 = 1** (output), CR bits 4–3:
  - `00` handshake. A side: CA2 goes low `tca2` after the PHI2 fall of a
    read of OR A; high `trs2` after the next active CA1 edge. B side: CB2
    goes low `tcb2` after the PHI2 rise that follows a write of OR B (the
    next cycle's rise); high `trs2` after the next active CB1 edge.
  - `01` pulse. A side: CA2 low `tca2` after the PHI2 fall of a read of OR A,
    high `tca2h` after the next PHI2 fall. B side: CB2 low `tcb2` after the
    PHI2 rise following a write of OR B, high `tcb2h` after the next PHI2 rise.
  - `10` / `11`: CA2/CB2 = CR bit 3, from the PHI2 fall of the CR write.
  CR bit 6 cannot be set while bit 5 = 1.
- **IRQx** (open drain, low = asserted): low while (bit 7 ∧ bit 0) ∨ (bit 6 ∧
  bit 3 ∧ ¬bit 5); otherwise Z. X when a term is X.

### 3.5 Reset and the clock

- RESB low: every register 0 (all lines inputs, CA2/CB2 inputs), IRQs Z
  (p.13). RESB X: every register X, IRQs X. Z reads 1 (RESB has no pull-up on
  this chip; an open RESB is the board's mistake, but X here would make every
  unconnected test board unknown — the W65C02S treats Z on its RESB the same
  way, plan-09-B).
- PHI2: a high or low phase shorter than `tc`, or falls closer than `tcyc`:
  every register X (as `bus65` rule 6). The W65C21 is static (p.3), so a
  stopped clock is legal.

## 4. Tests (added to `pio_test`)

Fixtures: the shipped `W65C21N` device (loaded from `pio.json`), on the
`bench` with `bus65`-style helper calls (`bench_bus65_write(reg, value)`,
`bench_bus65_read(reg)` added to `tests/support/bench.{h,c}` in B for B–D).
PHI2 1 MHz.

| Case | Checks |
|---|---|
| B1 reset | after RESB low: CRA/CRB/DDRA/DDRB read 0; with DDRA = DDRB = 0xFF and CR bit 2 set, ORA/ORB read 0 (Corrections); PA weak 1, PB Z, IRQs Z |
| B2 DDR/OR select | RS0 = 0 reads DDR with CR bit 2 = 0 and OR with bit 2 = 1, both sides |
| B3 port A out | DDRA = 0xFF, ORA = 0xA5: PA = A5 at PHI2 fall + 20 ns; a bench driver forcing PA2 low reads back 0xA5 & ~4 (Corrections) |
| B4 port B mixed | DDRB = 0x0F, ORB = 0x5A, bench drives PB7..4 = 1001: read → 0x9A |
| B5 CA1 edge + IRQ | CRA = 0x07 (rising, enabled; Corrections): CA1 rise → IRQAB low 20 ns later, CRA bit 7 = 1; read ORA → bit 7 0, IRQAB Z 25 ns after the fall |
| B6 CA1 narrow pulse | CA1 high for 50 ns → CRA bit 7 X |
| B7 CA2 handshake | CRA = 0x24: read ORA → CA2 low at fall + 20 ns; CA1 active edge → CA2 high 25 ns later |
| B8 CB2 pulse | CRB = 0x2C: write ORB → CB2 low at the next rise + 70 ns, high at the following rise + 20 ns |
| B9 manual outputs | CRA = 0x34 / 0x3C → CA2 0 / 1 |
| B10 inputs as interrupts | CA2 input falling edge, CRA = 0x08 → bit 6, IRQAB low; CRA = 0x00 → bit 6 set, IRQAB Z; PB peripheral setup: PB changes 5 ns before the read's rise → that bit X |

Also: `library_check` refuses a `pia6521` device missing `"ca2"` and one
with a `"pa"` list of 7 names.

## Phases

> **Status discipline:** as plan-14-A § Phases.

### Phase 1 — The model

- [x] `backend/src/models/pio_pia6521.c` (§3.2–§3.5) registered in `pio.c`'s
      table; device keys and timing names of §3.1; `pin_open_drain` for the
      IRQ pins; `backend/CMakeLists.txt`. The shared 65xx keys load and bind
      through new `pio_load_names`/`pio_bind_names` in `pio.c` (C reuses
      them). Build: no errors or warnings.
- [x] `tests/support/bench.{h,c}`: `bench_bus65_write`, `bench_bus65_read`
      (plus `bench_bus65_begin`, `bench_bus65_idle`, `bench_has_pin`).
- [x] `pio_test` B1–B10 with an inline copy of the `W65C21N` device for this
      phase (Phase 2 switches it to the shipped device); `library_check` the
      two refusals. → `ok B1 reset` … `ok B10 inputs as interrupts`;
      `ok   pia6521 missing ca2: … missing "ca2"`, `ok   pia6521 pa of 7: … "pa" needs exactly 8 pin names`.
- [x] `docs/parts-format.md` § `pio` → Model `pia6521` (keys, timing names,
      behavior of §3.2–§3.5, "Not modeled": port drive currents, the N/S
      buffer difference), plus the shared keys.
- [x] Acceptance: `ctest --test-dir /tmp/logic-p14` → `100% tests passed, 0 tests failed out of 21`.

Acceptance: `pio_test` B1–B10 `ok`; `ctest` 100%.
  Check: `cmake --build /tmp/logic-p14 -j8 && /tmp/logic-p14/pio_test` → every case `ok` (est. 1 min).
Commit: c5b9082

### Phase 2 — Parts

- [x] `schematical/schematical/Parts/pio.json` (new file, `"format"` and
      `"version"` as the other part files) with `W65C21N` and `W65C21S`
      (§3.1), 4 parts. Loads: `listing_check` → `pio 4`.
- [x] `pio_test` loads `W65C21N` from the library instead of the inline copy.
      → 18 cases `ok` (A1–A8, B1–B10).
- [x] Regenerate the goldens (`docs/backend.md` § Tests). Check: no removed
      line; + 4 `parts.txt`, + 4 `traces.txt`, 0 `footprints.txt`.
      → numstat `4 0 parts.txt`, `4 0 traces.txt`; `footprints.txt` unchanged.
- [x] Spot-check `W65C21N6TPLG-14`'s listing line: pin 25 `CS2B`, pin 2 `NC`,
      `|footprint=PLCC-44`. → `25:CS2B:input:default`, `2:NC:nc:default`,
      `|footprint=PLCC-44|vcc=4500-5500,in=5500`.
- [x] (added) `listing_check.c` sized its per-kind arrays at 17 kinds; the
      first `pio` part overflowed them (SegFault). Sized by `PART_PIO + 1`;
      `docs/backend.md` § Adding a kind names the third line.

Acceptance: `ctest` 100%; the golden diff is exactly the 8 added lines.
  Check: `ctest --test-dir /tmp/logic-p14` → `100% tests passed`; `git diff --numstat backend/tests/golden` → `4 0 parts.txt`, `4 0 traces.txt` (est. 2 min).
  Result: `100% tests passed, 0 tests failed out of 21`; numstat as above.
Commit: 807ae4c

## Validation Plan

`pio_test` B1–B10 (model) and the golden programs (parts). The landing tier
is plan-14-J's.

## Decisions

- Author: `W65C21N` and `W65C21S` are two devices with the same keys and
  timing; the difference (buffer currents) is outside the simulator (§3.1).
- Author: port A inputs drive weak 1 (the chip's pull-ups), port B inputs
  float (§3.3).
- Author: `pwi`, printed in the Max column, is a minimum (§3.1).

## Corrections

- §4 B5: `CRA = 0x05` has bit 1 = 0, the **falling** CA1 edge (Table 3,
  p.18: bit 1 "0 Negative, 1 Positive"). A rising, enabled edge with ORA
  selected is `0x07`; the case uses `0x07` (its CRA read-back is `0x87`,
  then `0x07` after the ORA read).
- §4 B3: `0xA5 & ~8` = `0xA5` (bit 3 of A5 is already 0), so forcing PA3
  low proved nothing. The case forces **PA2** low and reads `0xA1`.
- §4 B3 / §3.3: a bench driver can only force a port A output low if the
  PIA's high is weak; two strong drivers short (the engine's smoke rule).
  p.16 says port A's pull-ups are "resistive in nature" and that a read can
  differ from ORA when a pin is held below 2.0 V. So port A drives an
  output **0 strong and an output 1 weak**; port B outputs are strong.
- §4 B1: with CR bit 2 set, ORA reads the PA **pins** (0xFF: the
  pull-ups) and ORB's input bits read the floating pins (X), so "ORA/ORB
  read 0" cannot be observed directly. B1 sets DDRA = DDRB = 0xFF first;
  the pins then show ORA = ORB = 0x00.
- §3.4: the `pwi` narrow-pulse rule is applied to CA2/CB2 inputs as well
  (p.8 lists PWI for the interrupt inputs, CA2/CB2 among them).
- §3.4 (not stated): entering handshake (`100`) or pulse (`101`) mode sets
  CA2/CB2 high, as on the 6821; B7 and B8 check the line starts high.
- §3.2 "an unknown read of OR makes the side's flags X" needed a `bus65`
  hook: `read_unknown()` (plan-14-A Corrections). A side whose CR bit 2 is
  known 0 is left alone (its RS0 = 0 read is DDR, no side effect).
- `listing_check` had a fixed 17-entry per-kind array (not in
  `docs/backend.md`'s "Adding a kind"); the 18th kind segfaulted it. Fixed
  in Phase 2 and documented.
- The bench helpers are one 1 MHz cycle each and start and end at a PHI2
  fall; a manual edge between cycles uses the tests' `idle_begin` /
  `idle_end` so the low phase never drops under `tc`.
