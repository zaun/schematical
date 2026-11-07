# plan-14-D: The `via6522` model, part 2 — the shift register; the W65C22 parts

Last updated: 2026-11-01
Effort: medium (1h–2h)
Depends on: plan-14-C

D finishes the `via6522` model with the shift register in its seven modes
(ACR bits 4–2), and ships the W65C22N and W65C22S devices with their 3
parts.

Outcome of D: `pio_test` D1–D8 pass on the shipped devices; `pio.json`
gains `W65C22N` (1 part) and `W65C22S` (2 parts); `parts.txt` and
`traces.txt` gain 3 lines each; `footprints.txt` none.

References: `datasheets/pio/w65c22.pdf` — §2.11–2.13 shift register p.19–24
(Table 2-10 p.20; Figures 2-6, 2-7 p.21; 2-8, 2-9 p.22; 2-10, 2-11 p.23;
2-12 p.24), §5.2 shift clock p.48, pinouts Figures 3-1/3-2 p.27, Table 4-4
`tsr1`–`tsr3` p.42–43, ordering p.50; plan-14-C §3 (the model so far).

## Prerequisites

plan-14-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-14-C landed | `/tmp/logic-p14/pio_test \| grep -c '^ok C'` → 15 (C1–C14, C13 on both dies; Corrections) | MET (2026-11-07: `15`) |

## 1. Goal

- `pio_test` D1–D8 `ok`; A, B, C cases still `ok` (C now on the shipped
  devices).
- Goldens: + 3 `parts.txt`, + 3 `traces.txt`; no removed or changed line.

### Non-goals

- No W65C22N PLCC (0 WDC stock) and no W65C22S QFP (no outline):
  `datasheets/skipped-parts.md`.
- No app change.

## 2. Current State

After C: `via6522` has everything but the shift register; ACR shift modes
act as `000`. The devices exist only as inline test copies.

## 3. Design

### 3.1 The shift register (§2.11–2.13)

An 8-bit SR, a modulo-8 bit counter, and IFR2. Shifting out sends bit 7
first on CB2 and rotates it into bit 0 (Table 2-10 note 1); shifting in
enters at bit 0 and moves toward bit 7 (note 2). A read or write of the SR
(RS A) clears IFR2 and restarts the bit counter (all modes but `000`, `100`).

| ACR 4–2 | Mode | CB1 | Rule |
|---|---|---|---|
| `000` | disabled | per PCR | SR readable/writable; no shifting; IFR2 held 0 |
| `001` | in, T2 rate | output clock | CB1 falls N + 2 cycles after the SR access and every N + 2 cycles toggles (N = T2 low latch); CB2 sampled at each CB1 rising edge (setup `tsr2` before the PHI2 rise that follows it) and shifted in during that PHI2 cycle; after 8 rises IFR2 is set, CB1 stays high (Figure 2-6) |
| `010` | in, PHI2 rate | output clock | from the cycle after the SR access CB1 is low for one cycle and high for one, 8 times; a bit shifted in at each PHI2 fall where CB1 is high; IFR2 after the 8th, CB1 stays high (Figure 2-7) |
| `011` | in, external | input | each CB1 rising edge shifts CB2 in during the next PHI2 cycle; IFR2 after 8; keeps shifting (Figure 2-8) |
| `100` | out, free-running at T2 rate | output clock | as `101` but never stops and never sets IFR2 (Figure 2-9) |
| `101` | out, T2 rate | output clock | the SR write starts it: CB1 toggles every N + 2 cycles; CB2 changes `tsr1` after the PHI2 fall at each CB1 fall; after 8 bits IFR2 set, CB2 holds the last bit (Figure 2-10) |
| `110` | out, PHI2 rate | output clock | CB1 low one cycle, high one, 8 times; CB2 changes at each CB1 fall + `tsr1`; IFR2 after 8 (Figure 2-11) |
| `111` | out, external | input | CB2 changes `tsr1` after the PHI2 fall that follows each CB1 falling edge; IFR2 every 8 edges; keeps shifting (Figure 2-12) |

- In the T2 modes (`001`, `100`, `101`), T2's low counter reloads from the
  low latch at each toggle (the shift clock uses only T2's low byte); T2 does
  not set IFR5 in these modes. T2 in pulse-count mode (ACR bit 5 = 1) with a
  T2 shift mode: the shift clock still uses the low byte, as above.
- In shift modes CB2 is the serial line (output modes drive it; input modes
  read it) whatever PCR bits 7–5 say; CB1 likewise (output clock or input).
  `000` returns both to PCR control.
- **External clock (`011`, `111`) synchronisation** (the §2.11 note: a CB1
  edge "very close" to PHI2 low may not shift): a CB1 edge less than `tpcr`
  before a PHI2 fall makes that shift unknown — the SR and the bit counter go
  X — instead of picking shifted or not shifted.
- CB1 driven by another chip while the VIA outputs the shift clock
  (§5.2: may be overdriven): the VIA keeps shifting on its own clock; the net
  conflict is the engine's (two strong drivers → X on the net), unchanged.

### 3.2 Parts and pinouts

Device keys as plan-14-C §3.1, plus `"id"`, `"function": "VIA"`, `"drawer":
["Interface"]`, `"description"` ("Versatile interface adapter,
NMOS-compatible (WDC W65C22N)" / "Versatile interface adapter, CMOS (WDC
W65C22S)"), `"note"` (PDF and pages).

Pinouts (p.27):

- `p40`: 1 VSS, 2–9 PA0–PA7, 10–17 PB0–PB7, 18 CB1, 19 CB2, 20 VDD, 21 IRQB,
  22 RWB, 23 CS2B, 24 CS1, 25 PHI2, 26 D7, 27 D6, 28 D5, 29 D4, 30 D3, 31 D2,
  32 D1, 33 D0, 34 RESB, 35 RS3, 36 RS2, 37 RS1, 38 RS0, 39 CA2, 40 CA1.
- `plcc44` (W65C22S only): 1 VSS, 2–9 PA0–PA7, 10 PB0, 11 NC, 12–17
  PB1–PB6, 18 PB7, 19 CB1, 20 CB2, 21 VDD, 22 NC, 23 IRQB, 24 RWB, 25 CS2B,
  26 CS1, 27 PHI2, 28 D7, 29 D6, 30 D5, 31 D4, 32 D3, 33 NC, 34 D2, 35 D1,
  36 D0, 37 RESB, 38 NC, 39 RS3, 40 RS2, 41 RS1, 42 RS0, 43 CA2, 44 CA1.

Pin roles: IRQB output (open-drain on W65C22N via `pin_open_drain`); PA, PB,
CA2, CB1, CB2, D `inout`; CA1 and the bus controls inputs. Groups per
plan-14-A §4.2 (`data`, `port A`, `port B`).

| Part | Device | package | pinout | footprint |
|---|---|---|---|---|
| W65C22N6TPG-14 | W65C22N | DIP | p40 | PDIP-40-600 |
| W65C22S6TPG-14 | W65C22S | DIP | p40 | PDIP-40-600 |
| W65C22S6TPLG-14 | W65C22S | PLCC | plcc44 | PLCC-44 |

Each part `"grade": "14"`.

## 4. Tests (added to `pio_test`)

Shipped devices (C's cases switch from the inline copies to them).

| Case | Mode | Checks |
|---|---|---|
| D1 | `000` | SR read/write round trip; no CB1/CB2 activity; IFR2 stays 0 |
| D2 | `001` | T2 low latch = 2: CB1 toggles every 4 cycles; bench CB2 pattern 0xA5 shifts in; IFR2 after 8 rises; SR read clears it |
| D3 | `010` | 8 CB1 pulses of 2 cycles; 0x3C in |
| D4 | `011` | bench CB1 edges shift 0x81 in; an edge 5 ns before a PHI2 fall → SR X |
| D5 | `100` | 0x96 recirculates on CB2 for 24 bits; IFR2 never set |
| D6 | `101` | 0xC3 out MSB first, CB2 change at CB1 fall + 35 ns; stops after 8, CB2 holds bit 0 of the original byte |
| D7 | `110` | 0x5A out at the PHI2 rate; IFR2 after 8 |
| D8 | `111` | bench CB1 falls shift 0x0F out; IFR2 every 8 edges |

## Phases

> **Status discipline:** as plan-14-A § Phases.

### Phase 1 — The shift register

- [x] `pio_via6522.c`: §3.1. Build: no errors or warnings.
- [x] `pio_test` D1–D8 (inline devices still). → `ok D1 SR disabled (000)` …
      `ok D8 SR out, external (111)`; 41 cases `ok` in all.
- [x] `docs/parts-format.md` § `pio` → Model `via6522`: the shift register.
- [x] Acceptance: `ctest` → `100% tests passed, 0 tests failed out of 21`; goldens unchanged.

Acceptance: D1–D8 and C1–C14 `ok`.
  Check: `/tmp/logic-p14/pio_test` → every case `ok` (est. 1 min with the build).
Commit: f2aace0

### Phase 2 — Parts

- [x] `pio.json` + `W65C22N`, `W65C22S` (§3.2), 3 parts.
- [x] `pio_test` C and D cases load the shipped devices. → 41 cases `ok`
      (the inline copies and `inline_library` removed).
- [x] Regenerate the goldens. Check: no removed line; + 3, + 3, + 0.
      → numstat `3 0 parts.txt`, `3 0 traces.txt`; `footprints.txt` unchanged.
- [x] Spot-check `W65C22S6TPLG-14`: pin 11 `NC`, pin 23 `IRQB`,
      `|footprint=PLCC-44`; `W65C22N6TPG-14` IRQB open-drain in the listing.
      → `11:NC:nc:default`, `23:IRQB:output:default`,
      `|footprint=PLCC-44|vcc=1710-5500,in=5500`. The listing has no
      open-drain field (Corrections); `pio_test` → `ok open-drain IRQ pins`
      checks `part_pin_open_drain`: W65C22N IRQB 1, W65C22S IRQB 0, W65C21N
      IRQAB/IRQBB 1.
- [x] Acceptance: `ctest` → `100% tests passed, 0 tests failed out of 21`.

Acceptance: `ctest` 100%; the golden diff is exactly the 6 added lines.
  Check: `ctest --test-dir /tmp/logic-p14` → `100% tests passed`; `git diff --numstat backend/tests/golden` → `3 0 parts.txt`, `3 0 traces.txt` (est. 2 min).
Commit: —

## Validation Plan

`pio_test` D1–D8 and the golden programs. The landing tier is plan-14-J's.

## Decisions

- Author: an external shift clock edge within `tpcr` of a PHI2 fall makes
  the shift unknown (X) rather than choosing a side (§3.1).

## Corrections

- Prerequisites: `grep -c '^ok C'` → **15**, not 14: C13 runs on both the
  N and S dies and prints a line for each. Every C case is `ok`.
- §3.1 (not stated): CB1 edges still set IFR4 by PCR bit 4 in the shift
  modes, including the VIA's own shift clock (the flag follows the pin, as
  on the NMOS 6522); CB2's PCR edge flags are off while CB2 is the serial
  line. D5 and D7 therefore check IFR2 alone, not the whole IFR.
- §3.1 (not stated): before the first bit the output modes drive CB2 high
  (the sheet's figures leave it undefined), and the CB1 output clock idles
  high.
- §3.1: IFR2 in the output modes is set at the 8th CB1 rise (Figures 2-10,
  2-11 show IRQB falling there); in the input modes at the fall that
  shifts the 8th bit in.
- §3.1 external clock: an X edge on CB1 in `011`/`111` makes the SR and its
  count X, as the `tpcr` rule does.
- §4 D1: the bench releases CB1/CB2 to check the VIA does not drive them;
  on the W65C22N a released pin floats (X), which correctly sets the PCR
  flags X. D1 runs on the W65C22S (bus hold keeps the level).
- Phase 2 spot check: the listing line (`tests/support/listing.c`) has no
  open-drain field, so "IRQB open-drain in the listing" cannot be read
  there. The added `pio_test` case `open-drain IRQ pins` asks
  `part_pin_open_drain` (what the voltage check uses) instead.
- §4 D5: the bit sequence is checked over 24 bits, then two more CB1 rises
  prove it keeps running (`T2 low latch = 1`: a bit every 6 cycles).
