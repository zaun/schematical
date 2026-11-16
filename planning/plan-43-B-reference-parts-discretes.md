# plan-43-B: Reference-board parts — 14 diodes and 13 transistors

Last updated: 2026-11-15
Effort: large (3h–1d)
Depends on: plan-43-A

B adds the 27 `letter` = `B` rows of `planning/plan-43-data/targets.tsv` to
`Parts/diode.json` and `Parts/transistor.json`, the footprints their
drawings need, and their `datasheets.json` entries. The behavioral outcome:
`listing_check` lists 27 more parts (32854 → 32881), each under the drawer
leaf of the device `targets.tsv` names, and `ctest` passes 36 programs.

References:

- `docs/parts-format.md` § `diode`, § `transistor`, § Parts, § Datasheets,
  § Footprints (pad rules R1–R6 in
  `planning/completed/plan-03-B-logic-footprints.md` § 3).
- `planning/completed/plan-29-A-diode-data.md` … `plan-29-N` (diode
  precedent: values from the datasheet), `planning/completed/plan-40-C-transistor-footprints-through-hole.md`
  § 3.1 (footprint ids and the reuse rule),
  `planning/completed/plan-40-G-transistor-parts.md` § 3 (device ids, part
  keys, datasheet ids).
- `planning/plan-43-data/targets.tsv`, `facts.tsv`.
- `docs/backend.md` § Tests (golden regeneration).

## Prerequisites

plan-43-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-43-A complete | `ls planning/completed/plan-43-A-*.md` → exists | MET (2026-11-15: archived in c94a9b47) |
| The 27 PDFs are filed | `awk -F'\t' '$1=="B"{print $13}' planning/plan-43-data/targets.tsv \| sort -u \| while read -r f; do test -f "$f" \|\| echo "missing $f"; done` → no output | MET (2026-11-15: no output; 26 rows after A's correction) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `grep -c '|diode|' backend/tests/golden/parts.txt` grows by 14 and
  `grep -c '|transistor|' …` by 13 (baselines measured at the start of
  Phase 1 and written into § Corrections).
- `wc -l < backend/tests/golden/parts.txt` → 32881; `traces.txt` the same.
- `git diff <start> -- backend/tests/golden | grep -c '^-[^-]'` → 0 (lines
  are only added).
- `ctest --test-dir /tmp/logic-p43` → `100% tests passed … out of 36`.

### Non-goals (explicit constraints)

- No existing device, part or footprint changes; no new device key, no
  loader change.
- `planning/plan-29-data/census/gen-parts.py` and
  `planning/plan-40-data/census/gen-parts.py` are not run and their inputs
  are not edited: both plans are archived, `docs/parts-format.md` names no
  owner for `diode.json` or `transistor.json`, and the 27 parts are edited
  into the files by hand.
- No TVS, ESD, optocoupler or ULN2003 part (plan-43-A § 5).

## 2. Current State

`diode.json` has one device per (type, arrangement): the 27 rows need
`schottky-1`, `switching-1`, `zener-1`, `rectifier-1` (pinout `two-pin`:
pin 1 `K`, pin 2 `A`) and `schottky-cc2` (pinout `p3-a1-a2-k`).
`transistor.json` has one device per (drawer leaf, pinout), id
`<bjt|mosfet>-<leaf slug>-p<N>-<pin letters in pin order>`; the single
parts need `bjt-npn-p3-bec`, `bjt-pnp-p3-bec`, `mosfet-n-channel-p3-gsd`,
`mosfet-n-channel-logic-level-p3-gsd`, `mosfet-p-channel-logic-level-p3-gsd`,
`mosfet-n-channel-logic-level-p8-sssgdddd`, the dual NPN
`bjt-array-npn-npn-p6-e2b2c1e1b1c2`. All exist (`python3 -I -c "import
json; print(sorted(d['id'] for d in
json.load(open('schematical/schematical/Parts/transistor.json'))['devices']))"`).

### Measured populations

| What | Count | Command |
|---|---|---|
| B rows | 27 (diode 14, transistor 13) | `awk -F'\t' '$1=="B"{print $2}' planning/plan-43-data/targets.tsv \| sort \| uniq -c` |
| Datasheet ids B uses / already present | 26 / 4 (`bzx384`, `bat54-g_bat54a-g_bat54c-g_bat54s-g`, `onsemi-bc807-16lt1-d`, `onsemi-bc817-16lt1-d`) | `targets.tsv` columns 12 and 11 |
| Rows with no existing footprint of their outline | 2 (`SRL23` DO-219AD, `CSD17311Q5` VSON-CLIP 5 × 6) | `targets.tsv` column 9 = `(none: new footprint)` |
| Makers new to these two files | Diodes Incorporated and Nexperia (diodes), Diotec, Infineon (diodes), Panjit, Slkor, Alpha & Omega, GOODWORK, MCC, Texas Instruments (transistors) | `targets.tsv` column 4 against the files' `manufacturer` values |

### Verified properties

- **Every device a single part needs exists**, and its pinout names are the
  ones above (read from the two files, 2026-11-15).
- **`BC807-40LT1G` and `BC817-25LT1G` have siblings in place**:
  `BC807-25LT1G` and `BC817-40LT1G` are in `bjt-pnp-p3-bec` / `bjt-npn-p3-bec`
  with footprint `SOT23-3-ONSEMI` and the same datasheet ids; the new parts
  copy every key but `name` and `hfe`.
- **Ratings are in `facts.tsv`** for all 27 (column `parameters`, DigiKey
  2026-11-15).

## 3. Design Overview

Twenty-seven hand edits, each from one datasheet. The risk is a wrong pin
order or a land pattern reused where the drawing differs; § 4 fixes the
order of work so that each part's pinout and drawing are read before its
JSON is written, and the goldens show every added line for review.

## 4. Detailed design

### 4.1 Diodes

Per part, in `targets.tsv` order:

1. **Pinout.** Read the datasheet's pinning figure. Two-pin parts go in the
   device's `two-pin` pinout (pin 1 cathode). `BAT54C-G3-08` goes in
   `schottky-cc2` / `p3-a1-a2-k` (pins 1 and 2 anodes, 3 common cathode).
2. **Values**, from the datasheet's limiting-values table (plan-29's
   rule), with `facts.tsv` as the cross-check: `vr` = repetitive peak
   reverse voltage, `io` = average forward current per diode; Zeners `vz` =
   nominal Zener voltage, `pd` = total power dissipation. A datasheet value
   that differs from DigiKey's is kept and noted in § Corrections.
3. **Package label**: column `package` of `targets.tsv` (`DO-219AD` is new;
   the others exist in `diode.json`).
4. **Flags**: `SDM03U40Q-7` gets `"flags": ["automotive"]`.
5. **Manufacturer**: `Diodes Incorporated`, `Diotec Semiconductor`,
   `Infineon Technologies`, `Nexperia`, `onsemi`, `Panjit`, `Slkor`,
   `Vishay` (the names `diode.json` and `transistor.json` already use for
   onsemi, Vishay, Nexperia, Diodes Incorporated and Infineon).

### 4.2 Transistors

1. **Pinout and device.** Read the pin order from the datasheet. A single
   part joins the device `targets.tsv` names when its pin order is the
   device's (`p3-gsd` = 1 gate, 2 source, 3 drain; `p3-bec` = 1 base,
   2 emitter, 3 collector; `p8-sssgdddd`); otherwise it joins or starts the
   device of its leaf with its own pin order, id by § 2's scheme. The leaf
   is fixed by the facts: MOSFETs with Rds(on) specified at 4.5 V or less
   are `… logic-level` (`facts.tsv` "Drive Voltage" or the "Rds On" gate
   voltage: all MOSFETs here but `2N7002PW` and `NX7002AK,215`).
   `SIL2308-TP` (N + P pair, SOT-23-6) joins
   `mosfet-array-nmos-pmos-p6-<pins>` for its pin order
   (`…-g1s2g2d2s1d1` and `…-s1g1d2s2g2d1` exist) or starts that device,
   copying the `elements` form of `mosfet-array-nmos-pmos-p6-s1g1d2s2g2d1`.
2. **Part keys**, from `facts.tsv` as plan-40-G § 3.2: BJT `vceo`, `ic`,
   `pd`, `hfe` (minimum gain); MOSFET `vds`, `id` (the Ta value), `rdson`
   (the maximum at the highest listed gate drive, ohms), `vgsth` (maximum),
   `vgs`, `pd` (from the datasheet when DigiKey lists none), `logicLevel`
   true for the logic-level leaves. An array's values are the first
   element's.
3. **Package label**: column `package` (`VSON-CLIP` is new).
4. **Manufacturer**: `Alpha & Omega Semiconductor`, `Diodes Incorporated`,
   `Diotec Semiconductor`, `GOODWORK`, `MCC`, `Nexperia`, `onsemi`, `Texas
   Instruments`, `Vishay Siliconix`.

### 4.3 Footprints

For each part, compare the datasheet's recommended land pattern (rule R1;
with no land pattern, the outline rules R2–R4) with the footprints of
`targets.tsv` column `footprint_candidates`, in the order listed. Reuse the
first whose every pad centre and pad size is within 0.05 mm of the drawing's
and whose pad `n` sits where the datasheet's pin `n` does (plan-40-C § 3.1).
Otherwise add a footprint: id `<existing base id>-<MAKER>` (`DIODE-SOD-123-DIODES`,
`SOT23-3-AOS`; `DIODE-DO-219AD` and `VSON-CLIP-8-5X6-TI` for the two new
outlines), `description`, `note` naming the drawing, file and page, style
`dual` for two-row packages and `explicit` where pads differ in size (the
drain pad of the 5 × 6 package). Append new footprints at the end of
`footprints.json`.

### 4.4 Datasheets

One `datasheets.json` entry per new id of `targets.tsv` column
`datasheet_id` (22 new ones), `url` = column `datasheet_url`. Parts name
the id with `"datasheet"`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Diodes

- [x] Record the baselines in § Corrections: `grep -c '|diode|'
      backend/tests/golden/parts.txt`, `grep -c '|transistor|' …`, `wc -l <
      backend/tests/golden/footprints.txt`. — 11152, 1246, 1624.
- [x] Add the 14 diode parts (§ 4.1), their footprints (§ 4.3) and their
      datasheet entries (§ 4.4). — `python3 -I planning/plan-43-data/add/apply.py
      planning/plan-43-data/add/diodes.json` → `datasheets +11`, `footprints
      +6`, `diode +14 parts`.
- [x] Regenerate the goldens (`listing_check --print`, `trace_check
      --print`, `footprint_check --print`; `docs/backend.md` § Tests) and
      hand-check each added `footprints.txt` line against its drawing. —
      32868 / 32868 / 1630 lines; the six added footprint lines read against
      the drawings named in their notes.

Acceptance: 14 more `|diode|` lines; no golden line removed.
  Check: `cmake --build /tmp/logic-p43 -j8 && ctest --test-dir /tmp/logic-p43 -R 'library_check|listing_check|trace_check|footprint_check|diode_test'`
  → `100% tests passed`; `git diff -- backend/tests/golden | grep -c '^-[^-]'` → 0 (est. 2 min).
  — `100% tests passed, 0 tests failed out of 5`; 0; `|diode|` 11166.
Commit: —

### Phase 2 — Transistors

- [ ] Add the 13 transistor parts (§ 4.2), their footprints and datasheet
      entries.
- [ ] Regenerate the goldens and hand-check the added footprint lines.

Acceptance: 13 more `|transistor|` lines; `parts.txt` has 32881 lines.
  Check: the Phase 1 command with `transistor_test` in place of
  `diode_test`; `wc -l < backend/tests/golden/parts.txt` → 32881 (est. 2 min).
Commit: —

## Validation Plan

- Tests: the loader (`library_check`), `listing_check`, `trace_check`,
  `footprint_check` cover every added line; no new test program.
- Coverage check: each added part appears as one new line in `parts.txt`
  and `traces.txt` (`git diff --stat backend/tests/golden`).
- Doc sync: `docs/parts-format.md` § `transistor` (the paragraph naming the
  makers of `transistor.json`: add the six new ones) — done in E with the
  other doc edits.
- Final gate: plan-43-E.

## Decisions

- The archived generators of plan-29 and plan-40 are not re-run; the parts
  are hand edits — author (§ Non-goals).
- New footprints carry the maker in their id, as plan-29 and plan-40 did —
  author.

## Corrections

- **Baselines** (2026-11-15 at c94a9b47): `|diode|` 11152, `|transistor|`
  1246, `footprints.txt` 1624 lines.
- **B adds 26 parts, not 27** (plan-43-A § Corrections: `MBT3904DW1T3G` is
  skipped). 14 diodes and 12 transistors; `parts.txt` ends at 32880. The
  Phase 2 acceptance reads 12 and 32880.
- **The edits are applied by a script, from a data file per phase**:
  `planning/plan-43-data/add/apply.py <spec>` with `add/diodes.json` and
  `add/transistors.json` (datasheets sorted by id, footprints appended,
  parts inserted by name). The values in the data files were read by hand
  from each datasheet, as § 4 says.
- **Six new diode footprints, not one.** Read against the drawings:
  `1N5819HW-7-F` reuses `DIODE-SOD-123-TSC-B` (pads 0.90 x 0.95, 4.05
  outer: the same numbers); `BAT60AE6327HTSA1` reuses `DIODE-SOD-323`
  (0.8 x 0.6, centres 2.5 against 2.4: each centre 0.05 off);
  `PMEG3050EP,115` reuses `DIODE-THIN-SMA` (1.4 x 2.1, centres 4.4: the
  same numbers, as the library's TSC SOD-128 parts); `NSR0530HT1G` reuses
  `DIODE-SOD-323-ONSEMI` (CASE 477 issue K); the two Vishay parts reuse
  `DIODE-SOD-323` and `SOT23-3`. New: `DIODE-SOD-523-DIODES`,
  `DIODE-SOD-323P-DIOTEC`, `DIODE-SOD-323F-NEXPERIA` (two parts),
  `DIODE-MINIMELF-NEXPERIA` (two parts), `DIODE-SOD-123FL-PANJIT`,
  `DIODE-SOD-123FL-SLKOR`.
- **`SRL23`'s package is `SOD-323P`, not `DO-219AD`.** The datasheet names
  it PowerSOD-323 / SOD-323P; DO-219AD is DigiKey's label. The footprint is
  `DIODE-SOD-323P-DIOTEC`, by rule R4: the drawing has no land pattern and
  R2 would leave no gap between the pads (the cathode terminal is 1.4 mm
  long under a 1.9 mm body).
- **Three parts carry `automotive`, not one**: `SDM03U40Q-7`,
  `BAT60AE6327HTSA1` (datasheet p.1 "Qualified according AEC Q101") and, in
  Phase 2, `BSH205G2AR`. DigiKey lists Grade = Automotive for exactly these
  three of the 46 (`raw/*.json`, parameters Grade and Qualification).
- **`SDM03U40Q-7` `vr` is 40 V** (datasheet VRM, peak reverse voltage);
  DigiKey lists the DC value, 30 V.
- **Part numbers with a packing suffix**: Nexperia's and Infineon's
  datasheets list the type number (`BAS16J`, `BZV55-B2V4`, `PMEG3050EP`,
  `PMEG4005EJ`, `BAT60A`), not the orderable code; the part is added under
  DigiKey's full number. That differs from `MBT3904DW1T3G`, whose
  datasheet lists the orderable numbers and omits it.

## Summary

The risk is in 27 pin orders and land patterns. Each is one datasheet
figure; the goldens list every line added.
