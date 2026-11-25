# plan-48-B: 555 timers and common op-amps — pinouts and facts per datasheet, the final pick

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-48-A

B reads each `add` row's datasheet once per (PDF, package): the pinout, the
package label and, for timers, the timing facts; it checks each op-amp's
supply span against the datasheet. It then fixes the final pick
(`candidates.tsv` `state` = `add`), writes the skipped parts and the
all-obsolete PDFs to their two files, and regenerates
`footprints-needed.txt` for the final pick.

References:

- plan-48-A (§ 2 the populations, § 4 the verdict rules, Phase 3's
  `specs.tsv` skeleton and `verdicts.tsv`).
- `docs/parts-format.md` § Pinouts and pins, § Parts (the `"package"` label
  rule), § Datasheets.
- `planning/plan-40-data/census/pinout-check.py`, `specs-fill.py` (the
  scripts B adapts); `datasheets/skipped-parts.md`,
  `datasheets/obsolete-not-included.md` (formats).

## Prerequisites

plan-48-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-48-A complete | `ls planning/completed/plan-48-A-*.md` → exists, or every A box ticked | NOT MET (2026-11-24: Phases 2 and 3 done, Phase 1 `[~]`: 11 picked rows still need their stock lookup, both DigiKey keys are spent until 2026-11-26 00:00 UTC; the lookups can only drop rows or move a reserve row up, and B's steps are rerun for those (Corrections 5)) |
| The spec skeleton exists | `head -1 planning/plan-48-data/specs.tsv` → the header | MET (2026-11-24: `type company pdf package supplier_package circuits rows example scheme label pinout facts`; B adds `label` and `scheme` before `pinout`) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every row of `planning/plan-48-data/specs.tsv` has `scheme`, `pinout`,
  `label` and `facts` filled (or `pinout` `-`: the PDF shows no pinout for
  that package), and `python3 -I planning/plan-48-data/census/pinout-check.py`
  prints `0 errors`.
- `candidates.tsv` `state` is `add` for exactly the rows with verdict `add`
  and a spec row with a pinout; at planning time 662 rows are picked (331
  PDFs, 96 package lines in the download set: plan-48-A § 2).
- `datasheets/skipped-parts.md` has a `## Timers and op-amps (plan-48)`
  section naming every `skip` row and the five `no datasheet` rows;
  `datasheets/obsolete-not-included.md` has one row per PDF whose listed
  parts are all obsolete; no such PDF is in `datasheets/timer/` or
  `datasheets/opamp/`.
- `footprints-needed.txt` is regenerated with `--add`.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No footprint is drawn here (C, D); no part file is written (G).
- Op-amp gain bandwidth, slew rate, offset and rail-to-rail facts are
  DigiKey's parameters of each row (plan-40's rule for ratings); B checks
  only the supply span and the pinout against the datasheet.

## 2. Current State

`specs.tsv` (A Phase 3) has one row per (type, company, pdf, package,
supplier_package, circuits) with `rows` and `example`. At planning time the
picked rows are in 331 PDFs (timer 14: `na555`, `lm555`, `lmc555`, `tlc555`,
`tlc555-q1`, `tlc3555`, `tlc3555-q1`, `tlc551`, `tlc552`, `na556`, `ne556`,
`tlc556m`, ADI `icm7555-icm7556`, Renesas `icm7555-icm7556-datasheet`;
op-amp 317) and 83 package lines (`awk -F'\t' '!/^#/ && $8>0'
planning/plan-48-data/footprints-needed.txt | wc -l` → 83).

### Measured populations

| What | Count | Command |
|---|---|---|
| Picked rows / PDFs | 662 / 331 | plan-48-A § 2 |
| Picked op-amp rows by circuits | 1: 241, 2: 238, 4: 121 | `awk -F'\t' 'NR>1 && $22=="candidate" && $3=="opamp"{n[$5]++} END{for(k in n)print k,n[k]}' planning/plan-48-data/candidates.tsv` |
| Picked timer rows by class | single 42, dual 20 | plan-48-A § 2 |
| Package lines with a picked row | 83 (C 57, D 26) | `awk -F'\t' '!/^#/ && $8>0{n[$1]++} END{for(k in n)print k,n[k]}' planning/plan-48-data/footprints-needed.txt` |

## 3. Design

### 3.1 `specs.tsv` columns B fills

- `label`: the part file's `"package"` label, by `docs/parts-format.md`'s
  rule (the family only): `DIP`, `SOIC`, `SOP` (the 5.3 mm bodies), `SSOP`,
  `TSSOP`, `TVSOP`, `VSSOP` (MSOP, µMAX, MiniSO), `SOT-23` (also TSOT),
  `SC-70`, `TO-263`, `SON` (DFN, TDFN, WSON, USON, X1SON, 2-side LFCSP),
  `QFN` (4-side LFCSP), `LGA`, `DSBGA` (WLP, WLCSP, UCSP, FCBGA, µSMD).
- `scheme`: one of the named pinouts below when the datasheet's pin table
  for that package equals it pin for pin, else `explicit`.
- `pinout`: for `explicit`, `<designator>=<name>` pairs in pin order,
  comma separated (ball names for ball packages); `-` when the PDF has no
  pin table or drawing for the package.
- `facts`: `key=value` pairs, `;` separated — timers: `process` (`bipolar`
  or `cmos`), `maxHz` (the datasheet's maximum astable frequency, typical
  when only typical is given), `tpd` (trigger-to-output propagation delay
  in ns, typical), `vcc=<min>-<max>` (recommended supply). Op-amps:
  `vcc=<min>-<max>` (the recommended total supply span V+ − V−, single
  supply), `inputMax` when the datasheet allows inputs above V+.

Named pinouts (pin names are the library's; the datasheet's own spellings
map to them: `TRIGGER`→`TRIG`, `THRESHOLD`→`THRES`, `DISCHARGE`→`DISCH`,
`CONTROL VOLTAGE`→`CONT`, `OUTPUT`→`OUT`, `IN−`/`−IN`/`INV`→`IN-`,
`V−`/`VEE`/`VSS`/`GND` of an op-amp→`V-`, `VCC`/`VDD` of an op-amp→`V+`):

| Scheme | Pins |
|---|---|
| `555-8` | 1 GND, 2 TRIG, 3 OUT, 4 RESET, 5 CONT, 6 THRES, 7 DISCH, 8 VCC |
| `556-14` | 1 DISCH1, 2 THRES1, 3 CONT1, 4 RESET1, 5 OUT1, 6 TRIG1, 7 GND, 8 TRIG2, 9 OUT2, 10 RESET2, 11 CONT2, 12 THRES2, 13 DISCH2, 14 VCC |
| `single-8` | 1 NC, 2 IN-, 3 IN+, 4 V-, 5 NC, 6 OUT, 7 V+, 8 NC |
| `dual-8` | 1 OUT1, 2 IN1-, 3 IN1+, 4 V-, 5 IN2+, 6 IN2-, 7 OUT2, 8 V+ |
| `quad-14` | 1 OUT1, 2 IN1-, 3 IN1+, 4 V+, 5 IN2+, 6 IN2-, 7 OUT2, 8 OUT3, 9 IN3-, 10 IN3+, 11 V-, 12 IN4+, 13 IN4-, 14 OUT4 |
| `single-5a` | 1 OUT, 2 V-, 3 IN+, 4 IN-, 5 V+ |
| `single-5b` | 1 IN+, 2 V-, 3 IN-, 4 OUT, 5 V+ |

Other names an `explicit` pinout may use: `NC`; `OFFSET1`, `OFFSET2`
(offset null), `SHDN`, `SHDN1`…, `EN` (shutdown / enable: role `input`),
`REF`, `COMP`, `EP` (a numbered exposed pad: role `ground`), `VCC2` /
`GND2` for a second supply pin of the same rail. An exposed pad the
datasheet does not number is the footprint's `centre` pad and no pin.

### 3.2 `pinout-check.py` (from plan-40's)

Per spec row: `label`, `scheme`, `pinout`, `facts` filled; a named scheme's
pin count equals the DigiKey Package / Case's; an explicit pinout's
designators are `1…n` without gaps or ball names; a timer row names `TRIG`,
`THRES`, `DISCH`, `OUT` (with a digit when `circuits` is 2) once per
circuit, one `VCC` and one `GND`; an op-amp row names `IN<k>+`, `IN<k>-`,
`OUT<k>` once per circuit (no digit for a single), at least one `V+` and
one `V-`; `vcc` is `min-max` with 0 < min < max ≤ 100; a timer's `process`,
`maxHz` (1…100000000) and `tpd` (1…100000) are present. Prints `<n> rows,
<m> errors` and each error.

### 3.3 The final pick (`census/final.py`)

- A row whose spec row has `pinout` `-` gets verdict `skip`, `no pinout in
  the datasheet`, in `verdict-overrides.tsv`; the refill of plan-48-A
  Phase 3 runs for its maker (stock check, scan, verdict, placement, a new
  spec row read here).
- `final.py` sets `state` = `add` for every `candidate` row with verdict
  `add`; `--write-skipped` appends the section to
  `datasheets/skipped-parts.md` (one table: part, maker, type, reason; the
  `no datasheet` rows with reason `no datasheet on the maker's site`);
  `--write-obsolete` appends a row to `datasheets/obsolete-not-included.md`
  per PDF whose listed rows are all `datasheet status` (PDF name, family
  `timer` or `opamp`, group `Analog → Timers` or `Analog → Op-amps`, the
  URL); `--verify-skipped` prints `<n> skip rows, 0 missing from
  skipped-parts.md`.
- `place.py --prune` removes PDFs no `add` row cites.
- After `final.py`, `select.py` is not run again: it would rewrite `state`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Timers

- [x] `census/pinout-check.py` per § 3.2 (`planning/plan-48-data/census/pinout-check.py`; `--unfilled` lists rows with no scheme yet).
- [x] Fill the timer spec rows from the 14 PDFs under `datasheets/timer/`:
      `scheme` / `pinout`, `label`, `process`, `maxHz`, `tpd`, `vcc` (one row
      per package; a grade with its own supply range, e.g. the `-Q1` parts,
      has its own PDF and so its own rows). Read by one subagent from the PDFs (`/tmp/p48/brief.md`),
      merged with `census/specs-fill.py`; evidence per row in the slice's `note` column.

Acceptance: the timer rows are complete and consistent.
  Check: `python3 -I planning/plan-48-data/census/pinout-check.py --type timer` → `<n> rows, 0 errors` (est. < 1 min).
  Result: `34 rows, 0 errors`. MET.
Commit: —

### Phase 2 — Op-amps

- [x] Fill the op-amp spec rows, one maker at a time (TI, ADI, ST): `scheme`
      / `pinout`, `label`, `vcc` (and `inputMax` where given). The reading
      order is the `rows` column, largest first. Twelve subagents read slices of about 42 rows
      (whole PDFs per slice, `census/slice-specs.py`), the checker was run by each on its slice; a blind
      second reading of 60 random rows (`census/audit-slices.py`, `census/audit-compare.py`) found
      **0 pin differences in 60 rows** and 4 judgement differences in facts (`inputMax` 0.05 vs 0.1,
      `vcc` 2-16.5 vs 2-18, 2.5-5.5 vs 2.7-5.5, 3.6-12 vs 4-12: Corrections 3).

Acceptance: the op-amp rows are complete and consistent.
  Check: `python3 -I planning/plan-48-data/census/pinout-check.py --type opamp` → `<n> rows, 0 errors` (est. < 1 min).
  Result: `469 rows, 0 errors` (after the refill of Phase 3). MET.
Commit: —

### Phase 3 — The final pick, the two lists, the final package lines

- [x] `census/final.py` per § 3.3: the `-` rows skipped and refilled, then
      `state` = `add`; `--write-skipped`, `--write-obsolete`;
      `place.py --prune`. `final.py --pinouts` → 6 rows (`MAX44281OANS+T`, `AD8244BRMZ-R7`,
      `INA330AIDGST`, `TL084CN`, `TL084BCN`, `TL084ACN`); `/tmp/p48/refill.sh` → 6 reserve rows moved up
      (their 5 spec rows read); `final.py --apply` → `656 rows are state add`; `--write-skipped` → `59 skip rows in 9 table rows`;
      `--write-obsolete` → `0 PDFs added` (no PDF has only obsolete parts); `place.py --prune` deleted 4 PDFs.
- [x] `python3 -I census/footprint-lines.py candidates.tsv
      ../../schematical/schematical/Parts/footprints.json --add >
      footprints-needed.txt`.
- [x] Record in § Corrections: the `add` count per type and maker, the skip
      count per reason, the PDF count, the package-line count per letter.

Acceptance: the pick is final and both lists are written.
  Check: `python3 -I planning/plan-48-data/census/final.py --verify-skipped` → `<n> skip rows, 0 missing from skipped-parts.md`; `python3 -I planning/plan-48-data/census/place.py --verify planning/plan-48-data/candidates.tsv` → `0 missing, 0 unused`; `awk -F'\t' 'NR>1 && $22=="add"' planning/plan-48-data/candidates.tsv | wc -l` → at most 662, with at most 200 per op-amp maker (est. 2 min).
  Result: `59 skip rows, 0 missing from skipped-parts.md`; `0 missing, 0 unused`; 656 `add` rows (op-amp TI 200, ADI 200, ST 200; timer TI 48, ADI 5, Renesas 3). MET.
Commit: —

## Validation Plan

- Tests: `pinout-check.py`; no product code changes.
- Coverage check: `git status --short backend schematical` → empty after B.
- Doc sync: `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`,
  `planning/plan-48-data/README.md`.
- Final gate: plan-48-I.

## Decisions

- Author: **named pinout schemes** for the seven standard pinouts, so a
  datasheet is checked against a known table instead of typed again.
- Author: **op-amp pins are named `IN<k>+`, `IN<k>-`, `OUT<k>`, `V+`,
  `V-`** in every part, whatever the datasheet's spelling.
- Author: **op-amp speed and precision facts come from DigiKey's
  parameters**; the supply span and the pinout come from the datasheet.

## Corrections

1. **Scheme and label columns.** `specs.tsv` gained `label` and the column
   order is `scheme label pinout facts` (the skeleton had `pinout scheme
   facts`). A named scheme leaves `pinout` empty; an `explicit` pinout equal
   to a named scheme is rewritten to it (`census/normalize-specs.py`).
2. **Extra pin names.** `ISET` (TS27x programmable bias-current pin), `BAL1`,
   `BAL2` and `FLAG` (OPA452's thermal-shutdown output) are also allowed in
   an explicit pinout, besides the plan's list. A numbered exposed pad is
   written `n+1=EP`; an unnumbered pad is no pin. 101 of 503 spec rows are
   `explicit` (offset-null and shutdown pins, QFN/DFN/CSP packages,
   non-standard SOT-23-5 orders), 402 use a named scheme
   (`dual-8` 166, `quad-14` 72, `single-5a` 63, `single-8` 37, `single-5b` 32,
   `555-8` 21, `556-14` 11).
3. **Facts are judgement calls, recorded in each row's reading** (the slice
   `note` column is not kept in specs.tsv). Timers: `maxHz` is the datasheet's
   maximum astable frequency where it gives one, else its 100 kHz advice or
   the axis end of its nomograph (LM555, NE555, NE556 → 100000); `tpd` is
   the typical trigger-to-output delay where the datasheet has one (LMC555
   150 ns, TLC3555 75 ns), else the typical output rise time. Op-amps: `vcc`
   is the datasheet's operating total supply span; `inputMax` only where the
   datasheet states inputs beyond V+ (volts above V+). The blind re-read of
   60 rows differed only in four such numbers; neither reading is
   authoritative, the part files (G) take the first. The model uses `vcc`
   only for the voltage check's supply span, `inputMax` not at all in play mode.
4. **Rows with no pinout (skip, `no pinout in the datasheet`): 6** —
   `MAX44281OANS+T` (fixed-gain amp, one input), `AD8244BRMZ-R7` (quad
   unity-gain buffer, no IN-), `INA330AIDGST` (thermistor current conveyor),
   and `TL084CN`, `TL084BCN`, `TL084ACN` (the TL084 PDF draws no DIP-14:
   its revision history says that package information was removed). Six
   reserve rows moved up (ADI `AD823AARZ-RL`, `MAX4493AUK+T`; TI `TL031IP`,
   `TLV2322IDR`, `LMV341IDCKR`, `MC33078DGKR`); their 5 new spec rows were
   read in a second slice.
5. **The pick is final except for the DigiKey lookups A could not finish.**
   11 `add` rows have no stock lookup (`census/to-check.py raw candidates.tsv
   stock-check.tsv candidate verdicts.tsv`): `TLC3555IDR`, `TLC555CPWR`,
   `TLC555CPSR`, `NE555PS`, `LM555CMM/NOPB` (timers) and `AD823AARZ-RL`,
   `MAX4493AUK+T`, `TL031IP`, `TLV2322IDR`, `LMV341IDCKR`, `MC33078DGKR`
   (op-amps moved up in B). After 2026-11-26 00:00 UTC run A's loop again;
   a row that fails drops (op-amp: the next reserve row moves up, is
   scanned, read here and placed; `final.py` is run again).
6. **Counts.** `add` 656: op-amp TI 200, ADI 200, ST 200; timer TI 48, ADI 5,
   Renesas 3. `out`/`skip`: legacy label 55, maker 16, ordering table 48,
   no pinout 6, no datasheet 5, stock check 4. PDFs: 292 (timer 13, op-amp
   279). Package lines of `footprints-needed.txt --add`: C 58, D 28; four of
   them have no existing footprint of the same family and pin count (C:
   `SC-70-8`, `TO-263 (DDPAK-7)`; D: `14-WLCSP`, `6-UTLGA`).

## Summary

The work is reading 331 datasheets' pin tables; the named schemes and
`pinout-check.py` keep it mechanical. Nothing of the product changes in B.
