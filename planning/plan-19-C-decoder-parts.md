# plan-19-C: Decoder, encoder and 7-segment parts

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-19-B

C writes `schematical/schematical/Parts/decoder.json` from
`planning/plan-19-data/parts.tsv` and `specs.md`, adds CD4511B to the
existing `latch.json` device family, and regenerates the goldens.

Outcome of C: `grep -c '|decoder|' backend/tests/golden/parts.txt` = the
number of `add` rows of `parts.tsv` other than CD4511B (N19 − n4511);
`parts.txt` grows by exactly N19 lines; no existing line changes.

References: plan-19-A §3.3 (data conventions), plan-19-B §4 (model keys),
`docs/parts-format.md` § Parts, § Footprints.

## Prerequisites

plan-19-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-19-B complete | `ls planning/completed/plan-19-B-*.md` → exists | MET (2026-11-07: B landed in 252d6a6 and is archived in this commit) |
| Part data present | `awk -F'\t' 'NR>1 && $11=="add"' planning/plan-19-data/parts.tsv \| wc -l` → N19 (recorded by A) | MET (2026-11-07: `322` = N19) |

## 1. Goal

- `decoder.json` loads; `ctest` T0 + 1 of T0 + 1.
- `wc -l < backend/tests/golden/parts.txt` = P(start of C) + N19; same for
  `traces.txt`; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0.
- Every device id in `specs.md` appears once in `decoder.json` or
  `latch.json`.

### Non-goals

- No model change (a part that needs one is a plan-19-B bug: fix it there,
  with a `decoder_test` case, and record it in Corrections).
- No change to existing `latch.json` devices other than adding CD4511B parts
  (§3).

## 2. Current State

- `latch.json` device `74_4511` (CD74HC4511/HCT4511, model `bcd7seg`,
  pinout p16). CD4511B has the same function and pin order (CD4511B
  datasheet pin map = CD74HC4511's; checked in A's `specs.md` § 4511B).
- Part lines are generated, not hand-written (plan-10 precedent: a script
  under `/tmp/p19/` reads `parts.tsv` and writes the `parts` arrays; it is
  not committed; the goldens and the counts below check its output).

## 3. Design

- `decoder.json`: `{"format": "logic-parts", "version": …, "kind":
  "decoder", "devices": [...]}` with the header of the other part files.
  Devices in function order of plan-19-A §3.1; each device's `model`,
  model keys, `pinouts`, `timing` from `specs.md`; `parts` from `parts.tsv`
  (`name`, `package`, `pinout`, `footprint`, `family`, `flags` when not
  empty, `vcc`/`inputMax` for SN54 parts).
- CD4511B: new device `4511B` in `latch.json` (CD4000 device ids are
  `<number>B`), model `bcd7seg`, keys copied from `74_4511` with the
  CD4511B pin names and its `C` family timing from `specs.md` § 4511B.
- `function` / `drawer` per plan-19-A §3.3.

## Phases

> **Status discipline:** as plan-19-A § Phases.

### Phase 1 — Part files

- [x] Generate `decoder.json` and the `4511B` device; load check: `ctest --test-dir /tmp/logic-p19 -R library_check` → pass. `decoder.json`: 32 devices, 322 parts (generator `/tmp/p19/scripts/gen_decoder.py`); `library_check` → `100% tests passed`; the whole library loads (`footprint_check` → `239 footprints compared, 8344 parts loaded`). ~~The `4511B` device~~ — moot: n4511 = 0, the census's only CD4511B part (CD4511BNS) is not in the cd4511b ordering table (plan-19-A Corrections), so `latch.json` is unchanged.
- [x] Device coverage: every `device` in `parts.tsv` `add` rows is a device id in the two files (python set difference → empty). `/tmp/p19/scripts/coverage.py` → `missing devices: []`, `add rows not in decoder.json: []`, `decoder.json parts not add rows: []`.

Acceptance: both checks (seconds).
Commit: —

### Phase 2 — Goldens and spot checks

- [x] Regenerate `parts.txt` and `traces.txt` (docs/backend.md § Tests). Check: removed lines 0; added lines N19 in each. `git diff backend/tests/golden | grep -c '^-[^-]'` → 0; `+` lines: parts.txt 322, traces.txt 322, footprints.txt 2; `wc -l` parts.txt and traces.txt → 8344 = P0 8022 + 322; `grep -c '|decoder|' parts.txt` → 322 = N19 − n4511.
- [x] Spot checks against `specs.md`, one listing line each: `SN74HC138N` (`|decoder|`, `HC`, `units=1`, `PDIP-16`, `vcc=2000-6000`), `SN74LS145N` (open-drain outputs), `SN74LS148N` (`Encoder`), `SN7447AN` (`7-segment decoder`), `CD4511BE` (`|latch|`), one `military` and one `automotive` part. `SN74HC138N|decoder|DIP|Logic|Decoder|||units=1|…|footprint=PDIP-16|vcc=2000-6000,in=6000` (the listing has no family field: `"family":"HC"` in decoder.json); `SN74LS148N|decoder|DIP|Logic|Encoder|…|vcc=4750-5250`; `SN7447AN|decoder|DIP|Logic|7-segment decoder|…|4:BI/RBO:inout|…`; `SNJ54LS148FK|…|military,ceramic|…|footprint=LCCC-20|vcc=4500-5500` (SN54 range); `SN74HC138QPWRQ1|…|automotive|…|TSSOP-16`; `SN74LV138ATDR|…|vcc=4500-5500` (part-level AT range); SN74LS145N open drain (listing has no such field; probe `/tmp/p19/tools/od`: `Y0 open_drain=1`, `Y9 open_drain=1`, `A open_drain=0`; SN7447AN `a` 1, `BI/RBO` 1, `LT` 0). ~~`CD4511BE`~~ — moot: no CD4511B part (n4511 = 0).
- [x] `trace_check --verbose SN74HC138N` shows exactly one of Y0…Y7 low whenever G1 = 1 and G2A = G2B = 0. `/tmp/p19/scripts/trace138.py` over the trace (states held longer than the 18 ns delay): `checked 53 settled states (7 enabled), 0 wrong` — enabled states show exactly Y<code> low, disabled ones every Y high.

Acceptance: `ctest` T0 + 1 of T0 + 1; golden counts as §1 (seconds).
Commit: —

## Validation Plan

C's gate: Phase 2 (`ctest` about 13 s, golden counts). Landing tier in D.

## Decisions

- CD4511B is a `latch` part (owner scope, plan-19-A Decisions; design
  plan-19-A §3.2).

## Corrections

- **No CD4511B** (§3, Phase 1): n4511 = 0 (plan-19-A Corrections), so no
  `4511B` device and `latch.json` is unchanged; `parts.txt` grows by N19 =
  322, all `|decoder|`.
- **plan-19-A's two footprints land here** (`SOT23-THIN-16`,
  `UQFN-16-RSV`; `footprint_check` rejects unused footprints), with their 2
  golden lines; the footprint check of plan-19-A Phase 2 is met here.
- **Part-level `vcc`** beyond the SN54 rule, from the spec sections' Supply
  lines: SN74AC138/238/139 -Q1 parts [1.5, 6]; SN74LV138AT parts [4.5, 5.5];
  LVC138A -Q1/-EP parts and CLVC138A [2, 3.6] (15 parts; the generator
  asserts each is used).
- **The listing line has no family field**: the spot check's `HC` is read
  from decoder.json, and open-drain from `part_pin_open_drain` (probe).

## Summary

C is data entry against a tested model; the risk is a wrong pin or polarity,
caught by the device coverage check, the spot checks and the trace check.
