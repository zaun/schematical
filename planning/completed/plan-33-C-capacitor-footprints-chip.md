# plan-33-C: Footprints — chip capacitors, and the shared capacitor footprint rules

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-33-B

C draws the land patterns of the chip groups in `fpgroups.tsv` (`letter`
C): MLCC 0402 / 0603 / 0805 / 1206, molded tantalum, tantalum-polymer and
niobium-oxide chips, and aluminum-polymer 2917 (7343) chips, one per
maker drawing. It writes them as ready objects in
`planning/plan-33-data/footprints-C.json` (F and G merge them into
`Parts/footprints.json` with the parts that use them, since
`footprint_check` rejects an unused footprint), fills the C rows' footprint
columns, and writes the four generic chip footprints of
`GENERIC-CAPACITOR` into `footprints-generic.json`. § 4 holds the rules D
uses too.

The single behavioral outcome: every C row of `fpgroups.tsv` names a
footprint of `footprints-C.json`, and loading those objects through
`footprint_check` gives no error.

References:

- `docs/parts-format.md` § Footprints (styles, mid-values, R7) and § The
  `"footprint"` part key.
- `planning/completed/plan-03-B-logic-footprints.md` § 3 (R1–R6; R4 :105,
  R5 :109, body :114).
- `planning/completed/plan-29-B-diode-footprints-vishay-single.md` § 4
  (rule choice, provisional ids, the ready-object flow) and
  `planning/plan-29-data/census/fp-load.sh` (loads ready objects through
  `footprint_check`).
- `planning/completed/plan-26-A-pull-resistor-data.md` § 4 (the generic
  `RES-*` chip numbers).

## Prerequisites

plan-33-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-33-B complete | `ls planning/completed/plan-33-B-*.md` → exists | MET (2026-11-11, at execution: 5b514d4) |
| No shipped footprint id starts with `CAP` | `grep -c '"id":"CAP' schematical/schematical/Parts/footprints.json` → 0 | MET (2026-11-11 at 8b1770b: 0; at execution: 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every C row of `fpgroups.tsv` has `footprint`, `part_package`, `pinout`
  and `note` (`awk -F'\t' '$1=="C" && ($11=="" || $12=="" || $13=="")'
  fpgroups.tsv | wc -l` → 0), and each `footprint` is an id in
  `footprints-C.json`.
- `planning/plan-33-data/census/fp-load.sh footprints-C.json
  footprints-generic.json` prints only "extra footprint" / "unused
  footprint" lines for exactly the new ids.

### Non-goals (explicit constraints)

- No change to `Parts/footprints.json` (F and G merge), to any shipped
  footprint, or to any part.

## 2. Current State

- No capacitor footprint exists (`grep -c '"id":"CAP'
  schematical/schematical/Parts/footprints.json` → 0). The generic chip
  footprints `RES-0402` … `RES-1206` (plan-26-A § 4) and `LED-0402` …
  (plan-03-A) are generic, not from a capacitor drawing.
- Planning-time scope (`planning/plan-33-data/footprints-needed.txt`,
  `census/footprint-proxy.py`; B Phase 4 replaces it with the `add` rows'
  groups):

| What | Count | Source |
|---|---|---|
| MLCC chip groups, rank-1 rows of the 273 listed ceramic buckets | 11 (maker × size): Cal-Chip 0402 0603 0805 1206; KEMET 0402 0603 0805 1206; AVX 0402 0603 0805 (AVX 0603 appears as 1.60 × 0.80 and 1.60 × 0.81 mm, one drawing) | `footprints-needed.txt` section 1, letter C |
| Molded chip groups, upper bound | tantalum ≤ 18 (6 metric cases × 3 makers), tantalum polymer ≤ 18, niobium oxide ≤ 4 (AVX only), aluminum polymer 7343 ≤ 3 | section 2 (facets of the unlisted queries) |
| Upper bound in all | ≤ 54 drawings | the sum |

## 3. Design

C works down the C rows of `fpgroups.tsv` maker by maker; per row it reads
the PDF's land-pattern table or case drawing, writes one ready object
(§ 4), and fills the row's columns. Identical geometry from two makers stays
two objects (two `note`s); the generator merges them (F § 3, plan-29-I's id
rule).

**Risk:** a land pattern copied wrong lands on every part of its group.
Each object is checked by `fp-load.sh` (malformed or overlapping pads) and
its numbers are read a second time from the drawing when its `note` is
written.

## 4. Capacitor footprint rules (shared by C and D)

### 4.1 Which rule

- **R1** — the datasheet prints a recommended land pattern ("Land Pattern",
  "Recommended Pad", "Soldering Pad", "Recommended Land" dimensions): its
  pad sizes and centres. When the table offers IPC-7351 density levels, use
  level B (nominal).
- **R4** (plan-03-B) — chip terminals from the outline only: pad length =
  terminal length + 0.3, pad width = terminal width + 0.05, pads extend 0.3
  outward. Molded tantalum chips use their termination drawing the same way.
- **R5** (plan-03-B) — through-hole drill and pad from the lead diameter.
- **R7** (`docs/parts-format.md`) — axial leads (ceramic axials, D).
- **R8 (new) — radial leads** (D): `dual`, `pins` 2, `perSide` 1, `pitch`
  1, `span` = the datasheet's lead spacing (F or LS; mid-value); `drill` and
  round `pad` per R5 from the lead diameter (d); body: a can or round
  dipped body is `{"width": D, "height": D, "round": true}` (D the body
  diameter, mid-value); a leaded MLCC box is W × T; a disc is D × T (its
  diameter by its thickness, as seen from above); `pin1Shape` `rect`.
- **R9 (new) — SMD cans (V-chip)** (D): R1 from the datasheet's land table
  for the can size; else R4 on the base plate's terminal drawing; body the
  base plate (A × B, mid-values).
- Mid-values per `docs/parts-format.md` § Footprints; the `note` records
  each value and its range.

### 4.2 Pins and polarity

Pad 1 is pin 1. Ceramic: pins `1`, `2`. Polarized parts: pin 1 is `+` (the
anode: a tantalum chip's band or bevelled end, a can's `+` terminal or long
lead), pin 2 is `-`; pad 1 is on the left (the origin rules of
`docs/parts-format.md`, x grows right).

### 4.3 Provisional ids

`<BASE>-<TAG>`, TAG `KEMET`, `CALCHIP` or `AVX`:

| Group | BASE | Example |
|---|---|---|
| MLCC chip | `CAP-<EIA>` | `CAP-0603-KEMET` |
| Molded chip (tantalum, polymer, niobium, aluminum polymer 7343) | `CAPM-<metric>-<case code>` | `CAPM-3216-A-KEMET` |
| SMD can (D) | `CAPE-SMD-<D>X<H>` | `CAPE-SMD-6.3X5.4-AVX` |
| Radial can (D) | `CAPE-RAD-<D>-<span>` | `CAPE-RAD-6.3-2.5-KEMET` |
| Leaded MLCC (D) | `CAP-RAD-<span>-<W>X<T>` | `CAP-RAD-5.08-5.08X3.18-KEMET` |
| Disc (D) | `CAP-DISC-<span>-D<D>` | `CAP-DISC-5.0-D12.5-KEMET` |
| Axial (D) | `CAP-AX-<span>-L<L>` | `CAP-AX-7.62-L4.32-KEMET` |
| Dipped tantalum (D) | `CAPT-RAD-<span>-D<D>` | `CAPT-RAD-2.54-D4.5-AVX` |

The generator (F) merges identical geometry and gives the first of a BASE
the bare id unless shipped; the generic ids (§ 4.6) ship first (F Phase 1),
so a maker's `CAP-0603` keeps its tag.

### 4.4 Part package names (`part_package`)

Chips: the EIA inch code (`0402`, `0603`, `0805`, `1206`, `1411`, `2312`,
`2917`); SMD cans `SMD can`; radial cans, leaded MLCCs and dipped
tantalums `Radial`; discs `Disc`; axials `Axial`. No size or material in
the name (`docs/parts-format.md` § Parts).

### 4.5 `fpgroups.tsv` columns C and D fill

`footprint` (provisional id), `part_package`, `pinout` (`two-pin` for
ceramic, `polar` otherwise), `note` (the drawing, file and page).

### 4.6 Generic chip footprints (`footprints-generic.json`)

`CAP-0402`, `CAP-0603`, `CAP-0805`, `CAP-1206`: the numbers of
`RES-0402` … `RES-1206` (plan-26-A § 4: bodies 1.0 × 0.5, 1.5 × 0.75,
2.05 × 1.25, 3.05 × 1.5; pads 0.55 × 0.75, 0.7 × 1.0, 0.85 × 1.5,
1.15 × 1.75 rect; spans 0.9, 1.2, 1.6, 2.3), `description` `0603 chip
capacitor (1.5 x 0.75 mm)`, `note` `generic; plan-33-C § 4.6`, no
`pin1Shape`. They keep their own ids although `RES-*` has the same
numbers (plan-26-A's precedent: `RES-*` repeats `LED-*`'s numbers under
its own ids), so the generator's reuse of shipped identical geometry
(F § 3) applies only to ids starting with `CAP`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Rules, generic chips, MLCC

- [x] `docs/parts-format.md` § Footprints: R8 and R9 after R7 (§ 4.1).
      (Landed in plan-33-E's commit d97e95d, § Corrections 1.)
- [x] `census/fp-load.sh` (from plan-29, `/tmp/p33-fp.XXXXXX`), with
      plan-29's `fp-merge.py`.
- [x] `footprints-generic.json`: the four chip objects (§ 4.6).
- [x] `footprints-C.json`: one object per MLCC row of `fpgroups.tsv`;
      fill the rows' columns. `census/fp-build.py C` (the numbers
      transcribed from each drawing, § Corrections 2): 11 MLCC objects
      (KEMET 4, Cal-Chip 4, AVX 3) for the 17 MLCC rows.

Acceptance: the MLCC and generic objects load.
  Check: `planning/plan-33-data/census/fp-load.sh planning/plan-33-data/footprints-C.json planning/plan-33-data/footprints-generic.json` → only extra / unused lines for the new ids (est. 3 min).
  Observed: `44 footprints appended`, `44 new footprints: load ok` (fp-merge.py's verdict: only extra / unused lines, for exactly the new ids).
Commit: 8c8f82e

### Phase 2 — Molded chips

- [x] `footprints-C.json`: one object per molded-chip row (tantalum,
      tantalum polymer, niobium oxide, aluminum polymer 7343); fill the
      rows' columns. 27 molded objects for the 71 molded rows (KEMET 7,
      KYOCERA AVX 14, Cal-Chip 6).

Acceptance: § 1, both bullets.
  Check: the `awk` of § 1 → 0, and the Phase 1 `fp-load.sh` command → only extra / unused lines (est. 3 min).
  Observed: `0`; `38 footprints in footprints-C.json`; `44 new footprints: load ok`.
Commit: 8c8f82e

## Validation Plan

- Tests: `footprint_check` through `fp-load.sh` (load rules, overlaps).
- Coverage check: § 1's `awk` (every C row has a footprint).
- Runtime proof: F and G's `footprint_check` with the parts.
- Doc sync: `docs/parts-format.md` (R8, R9).
- Final gate: plan-33-I.

## Decisions

- **One drawing per maker** — author: plan-29's precedent (per-maker land
  patterns, merged when identical); a maker's recommended land is what its
  parts were qualified on.
- **Pin 1 = `+`** — author: the convention of the KiCad and IPC libraries
  for polarized capacitors, and the band end of a tantalum chip.

## Corrections

1. **R8 / R9 text** went into `docs/parts-format.md` with plan-33-E's docs
   commit (d97e95d), which edited the same file.
2. **Numbers transcribed into a script.** `census/fp-build.py` holds each
   drawing's numbers (with page and range) and applies the rules; every
   object's `note` repeats them. One drawing per maker where its datasheets
   print the same table: KEMET's MLCC land table is identical in C1002,
   C1003, C1007, C1010 and C1091 (compared row by row); its tantalum /
   polymer IPC table gives the same level-B numbers for every case of one
   metric size (A/S2 3216, B/M/T 3528, C/U/L 6032, D/V/W/X/Y/Q 7343) in
   T491, T495, T510, T52X, T59X and A700, so one object per metric size; the
   AVX case tables (TAJ, TCJ, TPS, TPM, TLJ, NOJ, NOS, F92, F93, TAZ) print
   the same L / W / W1 / A per case.
3. **Rules used.** R1 for KEMET (IPC-7351 level B; JEITA RC-2371 min–max
   mid-values for cases P and S, which have no IPC row) and for the KYOCERA
   KGM 0603 (its catalog's recommended land, the `1.6 ±0.2` row: the
   0603 100 µF part is a high-capacitance build); R4 for Cal-Chip GMC and TC /
   PT, KYOCERA AVX MLCC (cx5r, X7R, KGM_X7R) and every AVX molded chip,
   whose datasheets print no land pattern. TAC case L prints only a minimum
   terminal spacing S (0.55), so its terminal length is (L − S) / 2.
4. **Rows without DigiKey's size code** (5, one part each) take the case from
   the part number (Cal-Chip `TCKOJ335AT` → A; KEMET `A700V…` → V,
   `T597S…` → S).
5. **No chip has `pin1Shape`**: § 4.2's pin 1 = `+` is pad 1 on the left;
   the shape marker is the through-hole rule (R5).

## Summary

C is drawing reading: at most 54 chip drawings plus 4 generic objects.
Done: 38 objects (11 MLCC, 27 molded) for the 88 C rows; the generator
merges the identical ones. The
rules (§ 4) are what D reuses; the only new ones are R8 and R9.
