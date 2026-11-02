# plan-10-I: Shift-register parts

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-10-H

I writes `schematical/schematical/Parts/shift.json`: 36 devices, 583 parts,
from `plan-10-data/shift-a.md`, `shift-b.md` and `parts.tsv`; adds the last
two new footprints (`LCCC-28`, `WQFN-16`); regenerates the goldens.

Outcome of I: the goldens reach their plan-10 size — `parts.txt` and
`traces.txt` 6108 lines, `footprints.txt` 158 — by added lines only;
`shift_test` also passes on real parts; `ctest` 15 of 15.

References: plan-10-A §3.4, plan-10-G §4 and plan-10-H §4 (models and
device keys), plan-10-C (procedure), `plan-10-data/footprints.md` §§ LCCC-28,
WQFN-16.

## Prerequisites

plan-10-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-10-H complete | `ls planning/completed/plan-10-H-shift-special.md` → exists | MET (2026-11-01: exists, archived at 4f87fba) |

## 1. Goal

- `shift.json`: 36 devices, 583 parts (C §1 one-liner → `36 583`).
- `wc -l < backend/tests/golden/parts.txt` → 6108 (5525 + 583), `traces.txt`
  → 6108, `footprints.txt` → 158 (156 + 2); added lines only.
- `grep -c '|shift|' backend/tests/golden/parts.txt` → 583.

### Non-goals (explicit constraints)

- No existing golden line changes; no model change (gaps → Corrections and a
  `shift_test` case in the owning sub-plan's model).

## 2. Current State

After H: the `shift` kind with six models and 43 cases; no `shift.json`.

### Measured populations

| What | Count | Command |
|---|---|---|
| Shift devices / parts | 36 / 583 | `awk -F'\t' 'NR>1 && $10 ~ /^shifters\//' planning/plan-10-data/parts.tsv \| wc -l` → 583; device count → 36 |
| Obsolete shift parts | 91 | the same filter with `$9=="Obsolete"` |
| FK parts | 74_164 5, 74_165 5, 74_166 5, 74_194 2, 74_595 2, 74_597 2 (FK-20, 14/16-pin devices); 74_299 4 (FK-20 = DIP numbering); 74_673 2, 74_674 2 (FK-28) | `awk -F'\t' '$7 ~ /^LCCC/' parts.tsv` (shift devices) |
| Two-pinout devices | TPIC6A595, TPIC6A596 (DW-24 and NE-20 differ) | `shift-b.md` § TPIC6A596 ("NE20 and DW24 are identical to TPIC6A595") and § TPIC6A595 |

## 3. Design Overview

Pure data, as C and F. File order: G §4.2 devices, then H's (74_673,
74_674, 4517B, 4034B, 40105B, 74_40105, 74_8153). `function` `"Shift
register"` for all 36 (the FIFO and the LV8153 come from `datasheets/
shifters/` and sit in the same drawer leaf).

## 4. Detailed Design

### 4.1 Devices

- **Main timing path** per G §4.1 / H §4; FIFO `fallThrough` per H §4.4.
- **FK-20** for the 14- and 16-pin devices of §2: `fk20` pinouts copied from
  each device's spec section map (16-pin: the standard map of plan-10-F
  §4.1; 14-pin 74_164: its printed map). 74_299 FK = DIP numbering.
- **FK-28** for 74_673 / 74_674: terminals 1, 8, 15, 22 NC; DIP 1–6 → 2–7,
  7–12 → 9–14, 13–18 → 16–21, 19–24 → 23–28 (`plan-10-data/footprints.md`
  § LCCC-28, from `sn74ls673.pdf` p.1).
- **TPIC6A595 / TPIC6A596**: `p24` for `SOIC-24W` parts and `p20` for
  `PDIP-20` (NE) parts, each from its spec table.
- **74_8153**: VCC1 and VCC2 both `power` (H §4.5); two GND pins `ground`.
- **74_595 LS names**: the LS595's printed names (G, RCK, SRCK) are unified
  to the device's (OE, RCLK, SRCLK) per plan-10-A §3.4.

### 4.2 Footprints

Add `LCCC-28` and `WQFN-16` from `plan-10-data/footprints.md` (verbatim)
after F's `VQFN-16`.

### 4.3 Real-part checks

`shift_test` gains: `SN74HC595N` (G cases 13–14), `SN74HC165N` (case 3),
`SN74LS299N` (case 7), `TPIC6B595N` (case 19), `CD4094BE` (case 21),
`CD74HC40105E` (case 34 with the HC fall-through), `SN74LV8153PW`
(case 40), `SN74LS673DW` (case 26).

## Phases

> **Status discipline:** as plan-10-A § Phases.

### Phase 1 — `shift.json` and two footprints

- [x] Footprints (§4.2), the 36 devices and 583 parts. (`/tmp/p10/gen_i.py`;
  the C §1 one-liner on `shift.json` → `36 583`. `/tmp/p10/check_fp2.py`:
  LCCC-28 28 pads, 7 per side at 1.27 mm, pads at ±5.23 = span 10.46;
  WQFN-16 16 pads, 6 per long side at 0.5 mm, pads at ±1.15 / ±1.65 = spans
  2.3 / 3.3, centre 1 × 2 — both as in `footprints.md`.)
- [x] Name/device check: `583 parts, 36 devices, names and devices match`.
  (Ran `/tmp/p10/check_names.py`: that line.)
- [x] Regenerate the goldens; added lines only; 583 / 583 / 2. (Ran: 0
  removed, 1168 added; 6108 / 6108 / 158; `grep -c '|shift|'` → 583.)
- [x] Spot checks, one line each here: `SN74HC595N` (16 pins, storage
  outputs QA–QH, `vcc=2000-6000`), `SNJ54HC595FK` (`fk20`, `LCCC-20`),
  `SN74AHC595BQBR` (`WQFN-16`), `SN74LS673DW` (24 pins, `SOIC-24W`), `SNJ54LS673FK` (`fk28`,
  `LCCC-28`, `military`), `TPIC6A595NE`
  (20 pins, `PDIP-20`), `SN74LV8153PW` (two `power` pins), `CD40105BE`
  (`C`, `vcc=3000-18000`). (Ran, from `parts.txt`: `SN74HC595N` 16 pins,
  `PDIP-16`, `vcc=2000-6000` (storage outputs QA–QH per `shift.json`);
  `SNJ54HC595FK` 20 pins (1 `NC`), `LCCC-20`, `military`; `SN74AHC595BQBR`
  `WQFN-16`; `SN74LS673DW` 24 pins, `SOIC-24W`; `SNJ54LS673FK` 28 pins
  (1 `NC`), `LCCC-28`, `military`, `vcc=4500-5500`; `TPIC6A595NE` 20 pins
  (1 `DRAIN2`, 17 `VCC`), `PDIP-20`; `SN74LV8153PW` power pins
  `1:VCC1`, `20:VCC2`; `CD40105BE` `C`, `vcc=3000-18000`.)

Acceptance: `ctest` 15 of 15; goldens 6108 / 6108 / 158, added only. (Ran:
`100% tests passed, 0 tests failed out of 15`.)
Commit: —

### Phase 2 — Real-part cases

- [ ] §4.3 cases in `shift_test`.

Acceptance: `shift_test` all `PASS`; `ctest` 15 of 15.
Commit: —

## Validation Plan

Phase 1 and 2 acceptance (`ctest`, about 7 s).

## Decisions

- The FIFO and the LV8153 sit under "Shift register" in the drawer (their
  datasheets are in the shifters folder; a one-device leaf each would add
  nothing).

## Corrections

- 2026-11-01, Phase 1: main timing paths follow G §4.1 per device: storage
  devices use the storage clock → Q (595/594/596 RCLK → Q; 4094B/74_4094
  STROBE → Q), PISO devices the shift clock → serial out (165, 166, 597,
  598), and the TPIC parts G → DRAIN, the only drain delay their sheets
  print (no RCK → DRAIN row). The 74_8153 entry is RESET → Y, its only
  printed Y delay besides the Pw/2 bit-timing ones.
- 2026-11-01, Phase 1: `docs/parts-format.md`'s file table gains
  `shift.json`.

## Summary

I adds the 583 shift-register parts and the last two footprints; the
goldens reach 6108 / 6108 / 158.
