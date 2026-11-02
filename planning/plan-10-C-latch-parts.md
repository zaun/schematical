# plan-10-C: Latch parts and the footprints they need

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-10-B

C writes `schematical/schematical/Parts/latch.json`: 28 devices, 750 parts,
from the spec sections in `plan-10-data/latch-a.md` and `latch-b.md` and the
part rows in `plan-10-data/parts.tsv`; adds the 15 new footprints the latch
parts are the first to use; and regenerates the goldens.

Outcome of C: the goldens grow by exactly 750 part lines and 15 footprint
lines, nothing else changes; `latch_test` also passes on real parts; `ctest`
13 of 13.

References:

- plan-10-A §3.4 (data conventions), plan-10-B §5 (the latch models and the
  device → model table).
- `plan-10-data/latch-a.md`, `latch-b.md`, `parts.tsv`, `footprints.md`.
- `docs/parts-format.md` § Footprints (rules for new footprints) and
  `docs/backend.md` § Tests (regenerating goldens).

## Prerequisites

plan-10-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-10-B complete | `ls planning/completed/plan-10-B-latch-kind.md` → exists | MET (2026-11-01: exists, archived at 0cd9047) |

## 1. Goal

- `python3 -I -c "import json; d=json.load(open('schematical/schematical/Parts/latch.json')); print(len(d['devices']), sum(len(x['parts']) for x in d['devices']))"` → `28 750`.
- `wc -l < backend/tests/golden/parts.txt` → 4795 (4045 + 750), `traces.txt` →
  4795, `footprints.txt` → 155 (140 + 15); the golden diff is added lines only.
- `grep -c '|latch|' backend/tests/golden/parts.txt` → 750.
- `latch_test` → every B case plus the 6 real-part cases of §4.3 `PASS`.

### Non-goals (explicit constraints)

- No existing listing, trace or footprint line changes.
- No model change. A spec section that the B models cannot express is a B
  bug: fix it in a Corrections entry here with a `latch_test` case, not by
  bending the data.

## 2. Current State

- `latch.json` does not exist; the `latch` kind and its models do (B).
- Footprints: 140 in `footprints.json`; the 15 of §4.2 are missing.

### Measured populations

| What | Count | Command |
|---|---|---|
| Latch devices / parts | 28 / 750 | `awk -F'\t' 'NR>1 && $10 ~ /^latch\// && $2 !~ /^(74_374\|TPIC6B?273)$/' planning/plan-10-data/parts.tsv \| wc -l` → 750; `… \| cut -f2 \| sort -u \| wc -l` → 28 |
| … per model | transparent 17 / 640, addressable 6 / 62, sr 3 / 34, bcd7seg 1 / 12, bus-mux 1 / 2 | `parts.tsv` grouped by the B §5 device tables |
| Obsolete latch parts | 74 | `awk -F'\t' 'NR>1 && $10 ~ /^latch\// && $2 !~ /^(74_374\|TPIC6B?273)$/ && $9=="Obsolete"' planning/plan-10-data/parts.tsv \| wc -l` |
| New footprints first used by latches | 15 | §4.2 |

## 3. Design Overview

Pure data. The order inside the file: transparent devices (B §5.1 table
order), then addressable, sr, bcd7seg, bus-mux. Each device: `id`,
`function` `"Latch"`, `description`, `drawer` `["Logic"]`, `note`, `model`,
`pinouts`, the model keys of B §5, `timing` (one entry per family present in
`parts.tsv` for the device), `parts`.

Risk: transcription — wrong pin numbers, a dropped family's VCC, a part on
the wrong pinout. Checks: the loader (every name resolves in every pinout,
footprint pin counts match), the listing count per device against
`parts.tsv`, and the spot checks of §4.3.

## 4. Detailed Design

### 4.1 Devices

For every device, the spec section gives the pinouts, the pin names and the
per-family timing and VCC. Apply plan-10-A §3.4. Per-device notes beyond the
spec sections:

- **Main timing path** (B §5): D→Q (transparent, addressable), S→Q (sr),
  input→segment (bcd7seg), B→A (bus-mux). Where the section gives only LE→Q
  or only max values, use what it gives and say so in the `note`.
- **74_373 / 74_573 names**: `OE`, `LE`, `1D`…`8D`, `1Q`…`8Q` for every family
  (the LS `OC`/`C` and CD74AC 0-based names are the same pins).
- **74_533 / 74_563 / 74_580 / CD74HC533 / 563**: outputs named `1Q`…`8Q` (SN
  numbering; the CD `Q0`…`Q7` are the same pins) and bound as `qn`.
- **74_75 / 74_375**: the pair enables take the spec's names; `QBAR` outputs
  bound as `qn`.
- **74_32373**: ball pinout `nmj` from the GKE ball map in the spec section
  (the only printed map; the NMJ package shares it). Balls listed row-major.
- **74_1G373**: pinouts `p6` (DBV, DCK) and `yzp` (ball map), footprints from
  `parts.tsv`.
- **4508B**: 24-pin; footprints `CDIP-24-600`, `SOIC-24W`, `SOP-24`,
  `TSSOP-24`.
- **TPIC6A259**: two pinouts (DW-24 and NE-20 differ, per the spec section):
  `p24` for `SOIC-24W` parts, `p20` for `PDIP-20` parts.
- **FK packages**: `LCCC-20` parts of the 20-pin devices (74_373, 74_573,
  74_533, 74_580) use the DIP pinout (FK pin n = DIP pin n, per the spec
  sections). The two 16-pin devices with FK parts, `74_259` (2 parts) and
  `74_279` (2), get an `fk20` pinout with NC terminals from the maps in
  `latch-b.md` § 74_259 ("fk20") and § 74_279 ("FK (SNJ54LS279AFK …)")
  (`awk -F'\t' '$7=="LCCC-20"' parts.tsv`, latch devices: these six).
- **SN54 VCC**: per plan-10-A §3.4 (e.g. 74_373 LS: SN74 4.75–5.25 V, SN54
  parts `[4.5, 5.5]`; 74_373 LVC: SN54 parts `[2, 3.6]`).

### 4.2 Footprints

Add these 15 objects from `plan-10-data/footprints.md`, verbatim, after the
existing entries (in this order): `CDIP-24-600`, `CDIP-24`, `CFP-48`,
`NFBGA-96`, `SOIC-16W`, `SOIC-24W`, `SOP-24`, `SSOP-24`, `SSOP-48`,
`SSOP-56`, `TSSOP-24`, `TSSOP-48`, `TSSOP-56`, `TVSOP-48`, `VQFN-20-RGY`.
Each has its `note` (drawing, file, page, mid-values). `LCCC-28`, `VQFN-16`
and `WQFN-16` wait for their first parts (I and F).

### 4.3 Real-part checks

`latch_test` gains six cases on real parts (B's bench): `SN74HC373N` (B
cases 2–4 again), `SN74LVC1G373DBVR` (transparent and hold),
`SN74HC259N` (demux mode), `TPIC6B259N` (drain polarity), `CD74HC4511E`
(code 7 → `1110000`), `SN74ALVCH16260DL` (SEL selects 1B onto A).

## Phases

> **Status discipline:** as plan-10-A § Phases.

### Phase 1 — Footprints

- [x] Add the 15 footprints (§4.2). Check: `footprint_check` fails only with
  "footprint not used by any part" for these 15 ids (expected until Phase 2)
  — so this phase is not committed alone; it lands with Phase 2. (Added by
  `/tmp/p10/add_footprints.py` straight from the `footprints.md` JSON blocks;
  the unused-footprint failure was not run on its own because latch.json was
  generated in the same step; the joint check is Phase 2's `ctest` 13 of 13.)

Commit: (with Phase 2)

### Phase 2 — `latch.json`

- [x] Write the 28 devices (§4.1, B §5 tables) and their 750 parts.
  (`/tmp/p10/gen_c.py`; the §1 python line → `28 750`.)
- [x] Load check: `trace_check` reports no load error, and
  `/tmp/logic-p10/listing_check --print | grep -c '|latch|'` → 750. (Ran: no
  load error, 750.)
- [x] Name and device check: a /tmp script reads `latch.json` and
  `parts.tsv` and prints `750 parts, 28 devices, names and devices match`
  (the set of (part name, device id) pairs in the file equals the TSV's latch
  rows). (Ran `/tmp/p10/check_names.py`, which also compares family and
  footprint per part: `750 parts, 28 devices, names and devices match`.)
- [x] Regenerate `parts.txt`, `traces.txt`, `footprints.txt`. Check: added
  lines only (`git diff backend/tests/golden | grep -c '^-[^-]'` → 0);
  750 / 750 / 15 added. (Ran: 0 removed, 1515 added; 4795 / 4795 / 155
  lines; `grep -c '|latch|' parts.txt` → 750.)
- [x] Spot checks against the spec sections, one line each in this plan:
  `SN74HC573AN` (pins 2–9 `1D`–`8D`, 12–19 `8Q`–`1Q`, `vcc=2000-6000`),
  `SNJ54LVC373AJ` (`military`, `vcc=2000-3600,in=5500`),
  `SN74ALVCH16373DGGR` (48 pins, `ALVC`, `TSSOP-48`), `SN74LVCH32373ANMJR`
  (96 balls, `NFBGA-96`), `CD4508BM` (24 pins, `SOIC-24W`, `vcc=3000-18000`),
  `TPIC6A259DW` (24 pins, `SOIC-24W`) and `TPIC6A259NE` (20 pins, `PDIP-20`).
  (Ran, from `parts.txt`:
  `SN74HC573AN` 20 pins, 2–9 `1D`…`8D`, 12–19 `8Q`…`1Q`, `PDIP-20`, `vcc=2000-6000,in=6000`;
  `SNJ54LVC373AJ` `LVC|military`, `CDIP-20`, `vcc=2000-3600,in=5500`;
  `SN74ALVCH16373DGGR` 48 pins, `ALVC`, `TSSOP-48`, `units=16`;
  `SN74LVCH32373ANMJR` 96 balls (A1 `1Q2` …), `NFBGA-96`, `units=32`;
  `CD4508BM` 24 pins, `SOIC-24W`, `vcc=3000-18000,in=18000`;
  `TPIC6A259DW` 24 pins (1 `DRAIN2`), `SOIC-24W`; `TPIC6A259NE` 20 pins, `PDIP-20`.)
- [x] `footprint_check --print` lines of the 15 new footprints hand-checked
  against `footprints.md` (pad count, pitch, span). (`/tmp/p10/check_fp.py`:
  13 dual footprints match pins / pitch / span; SSOP-56 pitch reads 0.634
  from coordinates printed to 3 decimals (±8.5725), i.e. 0.635; NFBGA-96 96
  balls; VQFN-20-RGY 20 pads, 8 per long side and 2 per short side at
  0.5 mm, centre pad 2.05 x 3.05 unnumbered.)

Acceptance: `ctest` 13 of 13; goldens 4795 / 4795 / 155, added only (about
5 s). (Ran: `100% tests passed, 0 tests failed out of 13`; 4795 / 4795 / 155;
0 removed lines.)
Commit: dfc9d48

### Phase 3 — Real-part cases

- [x] The six cases of §4.3 in `latch_test`. (Cases 29–34; the real-part
  library is `load_parts_dir(PARTS_DIR)`; steps are 1 µs because real delays
  reach 625 ns.)

Acceptance: `latch_test` all `PASS`; `ctest` 13 of 13. (Ran: 34 `PASS`, 0
`FAIL`; `100% tests passed, 0 tests failed out of 13`.)
Commit: —

## Validation Plan

Phase 2's acceptance plus Phase 3's (`ctest`, about 5 s). The golden
regeneration is reviewed by the added-only diff and the per-device count
script; behavior is covered by `latch_test`.

## Decisions

- Footprints land with their first parts (`footprint_check` rejects unused
  footprints), so Phase 1 and Phase 2 share a commit.
- The per-family names of one device are unified to the spec section's names
  (plan-10-A §3.4): a part's pins are named by function, not by its datasheet
  revision's spelling.

## Corrections

- 2026-11-01, Phase 2: §4.1 left the reference condition for the LV family
  open. Per plan-10-A §3.4 (LV at 3.3 V), 74_373 and 74_573 LV entries use
  the 3.3 V typical (7.3 and 7.7 ns), not the 5 V one the spec sections list
  first.
- 2026-11-01, Phase 2: where one family has SN and CD datasheets, the entry's
  timing is the SN typical (the §3.4 order: typical before maximum) and its
  `vcc` the SN range; CD74AC/CD54AC parts of 74_373 and 74_573 carry their
  own `"vcc": [1.5, 5.5]`. 74_573 LVC `-Q1` parts carry `[2, 3.6]` like the
  SN54 parts (latch-a.md § 74_573 VCC). A part-level `vcc` is only written
  where the SN54/CD/-Q1 range differs from the entry (LS, S, AC, LVC).
- 2026-11-01, Phase 2: TPIC `inputMax` omitted (absolute maximum only, the
  plan-10-A Phase 3 precedent). The 74_533/563/580 description says
  "inverting" (their CD title; the SN title is the 373's, which would hide
  the inversion in the drawer).
- 2026-11-01, Phase 2: `docs/parts-format.md`'s file table gains
  `latch.json`.

## Summary

C turns the latch spec sections into 28 devices and 750 parts and adds 15
footprints; the goldens grow by added lines only.
