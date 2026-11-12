# plan-33-D: Footprints — leaded and can capacitors

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-33-C

D draws the land patterns of the `letter` D groups of `fpgroups.tsv`:
radial-lead aluminum cans (and aluminum polymer), SMD aluminum cans,
leaded (radial) MLCCs, ceramic discs, ceramic axials and dipped radial
tantalums, with plan-33-C § 4's rules (R5, R7, R8, R9). It writes them to
`planning/plan-33-data/footprints-D.json`, fills the D rows' columns, and
adds the two generic leaded footprints of `GENERIC-CAPACITOR`
(`CAP-DISC-5.08`, its default, and `CAP-RADIAL-2.54`) to
`footprints-generic.json`.

The single behavioral outcome: every D row of `fpgroups.tsv` names a
footprint of `footprints-D.json`, and loading `footprints-C.json`,
`footprints-D.json` and `footprints-generic.json` together through
`footprint_check` gives no error.

References:

- plan-33-C § 4 (the shared rules, ids, package names).
- `docs/parts-format.md` § Footprints (R7 axial; the R8 / R9 text C adds).
- `planning/completed/plan-29-B-diode-footprints-vishay-single.md` § 4.1
  (R7's arithmetic, the axial precedent).

## Prerequisites

plan-33-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-33-C complete | `ls planning/completed/plan-33-C-*.md` → exists | MET (2026-11-11, at execution: 8c8f82e) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every D row of `fpgroups.tsv` has `footprint`, `part_package`, `pinout`
  and `note` (`awk -F'\t' '$1=="D" && ($11=="" || $12=="" || $13=="")'
  fpgroups.tsv | wc -l` → 0), each `footprint` an id in `footprints-D.json`.
- `census/fp-load.sh footprints-C.json footprints-D.json
  footprints-generic.json` prints only "extra footprint" / "unused
  footprint" lines for exactly the new ids.

### Non-goals (explicit constraints)

- No change to `Parts/footprints.json`, any shipped footprint, or any part.

## 2. Current State

Planning-time scope (`planning/plan-33-data/footprints-needed.txt`; B
Phase 4 replaces it with the `add` rows' groups):

| What | Count | Source |
|---|---|---|
| Ceramic axial body sizes (KEMET), rank-1 rows of the 27 listed axial buckets | 9 (2.29 × 4.06 … 3.81 × 10.16 mm, Dia × L) | `footprints-needed.txt` section 1, letter D |
| Ceramic radial (65 buckets), disc (17) and the other 23 axial buckets | not listed at planning time (the day's quota ran out; plan-33-A Phase 1 lists them) | plan-33-A § 2 |
| Aluminum radial cans, upper bound | ≤ 24 (12 diameters × KEMET, AVX) | section 2 |
| Aluminum SMD cans, upper bound | ≤ 69 (23 sizes × 3 makers) | section 2 |
| Aluminum polymer radial / SMD, upper bound | ≤ 14 / ≤ 15 (the SMD bound includes 7343 chips, which are C) | section 2 |
| Dipped radial tantalum, upper bound | ≤ 54 (27 body sizes × KEMET, AVX) | section 2 |

## 3. Design

As C § 3, down the D rows: cans by R8 (radial) or R9 (SMD), leaded MLCCs,
discs and dipped tantalums by R8, axials by R7. The two generic objects:

| id | description | body | pad | span | drill |
|---|---|---|---|---|---|
| `CAP-DISC-5.08` | Ceramic disc capacitor, 5.08 mm (0.2 in) lead spacing | 5.5 × 3.0 | 1.6 × 1.6 round | 5.08 | 0.8 |
| `CAP-RADIAL-2.54` | Radial (leaded) capacitor, 2.54 mm lead spacing | 4.0 × 2.5 | 1.6 × 1.6 round | 2.54 | 0.8 |

Both `dual`, `pins` 2, `perSide` 1, `pitch` 1, no `pin1Shape` (not
polarized; the `RES-RADIAL-2.54` precedent), `note` `generic; plan-33-D § 3`.
The disc body is a 5 mm disc of 3 mm thickness seen from above, with 0.25 mm
on each side for the lead bend.

**Risk:** the SMD can table is the biggest (≤ 69); a can size listed by two
makers with different land tables gives two footprints, which is correct.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Ceramic leaded parts and the generic ones

- [x] `footprints-generic.json`: `CAP-DISC-5.08`, `CAP-RADIAL-2.54` (§ 3).
      (Committed with plan-33-C, 8c8f82e.)
- [x] `footprints-D.json`: one object per ceramic radial, disc and axial row;
      fill the rows' columns. `census/fp-build.py D` (`fp-build-d.py`).

Acceptance: those objects load.
  Check: `planning/plan-33-data/census/fp-load.sh planning/plan-33-data/footprints-C.json planning/plan-33-data/footprints-D.json planning/plan-33-data/footprints-generic.json` → only extra / unused lines (est. 3 min).
  Observed (with Phase 2's objects in the same file): `196 new footprints: load ok`.
Commit: (this commit; hash in the next)

### Phase 2 — Cans and dipped tantalums

- [x] `footprints-D.json`: one object per aluminum and aluminum-polymer
      can row (radial R8, SMD R9) and per dipped tantalum row (R8); fill the
      rows' columns. 152 D objects in all for the 246 D rows (§ Corrections).

Acceptance: § 1, both bullets.
  Check: the `awk` of § 1 → 0 and the Phase 1 `fp-load.sh` command → only extra / unused lines (est. 3 min).
  Observed: `0`; `152 footprints in footprints-D.json`; `196 footprints appended`, `196 new footprints: load ok`.
Commit: (this commit; hash in the next)

## Validation Plan

- Tests: `footprint_check` through `fp-load.sh`.
- Coverage check: § 1's `awk`.
- Runtime proof: F and G's `footprint_check` with the parts.
- Doc sync: none (C added R8 and R9).
- Final gate: plan-33-I.

## Decisions

- **Generic disc 5.08 mm, radial 2.54 mm** — author: the commonest lead
  spacings of ceramic discs and leaded MLCCs (the 0.2 in and 0.1 in
  grids); the owner chose disc as the default (plan-33-A § Decisions).

## Corrections

1. **D groups split by the maker's case code.** One diameter can come in
   cases with different terminals (KEMET EDK 8 mm: 9L C 9.5 P 2.2 W 0.65,
   9M C 10.0 P 3.1 W 0.9) or leads (ESY 5 mm: C2 d 0.45, C3 d 0.50), so
   `footprint-lines.py` keys a D row with no DigiKey size code on the case
   code in its part number (`maker_case`): 214 → 248 groups (246 after the
   corrections below).
2. **Out after B: military by series, and crown capacitors.** `CK12BX101K`
   (series `Military, MIL-C-11015`) was a pick; `census/military-out.py`
   (plan-33-B § Corrections 8). KEMET PHH227 / PHV227 (and their PHH223 /
   PHV223 alternates) are multi-terminal "crown" parts — a ring of tabs round
   the + pin (`KEM_A4115_PHH227.pdf` p.3), a four-pad land
   (`KEM_A4136_PHV227.pdf` p.8) with no pole named per pad — which the
   kind's two-pin pinouts cannot hold: `census/crown-out.py` set the 4 rows
   `out` (`crown terminals (not a two-lead part)`); their two buckets are
   empty (17 in all, `buckets-empty.tsv`), their PDFs pruned (`place.py
   --verify` → `82 URLs, 81 PDFs, 0 missing, 0 unused`). Picks: 1031.
3. **Tables parsed from the PDFs.** `fp-build-d.py` reads the KEMET radial
   (D L P d; polymer D L d P), V-chip (D A C P W) and polymer SMD (D W H C R
   P) tables, the KYOCERA AVX radial tables (D L d P) and the GOLDMAX style
   tables from the placed PDFs; picture tables (Cal-Chip cans, AVX SMD lands
   and plates, discs, AXIMAX, A/S series, SR, SE, TAP, T35X) are transcribed
   in the script with their page. The GOLDMAX C0G and X7R tables are
   identical; the high-voltage C1055 table is its own.
4. **R9 used R1 where a land is printed** (KYOCERA AVX APA, AHA, AEA) and
   R4 on the terminals elsewhere: terminal length = (C − P) / 2 where C is
   tip to tip (KEMET, CCJ), or the printed terminal length (Cal-Chip CV2 C,
   CVH / CVL A); inner pad edges stay at the gap P, so span = P + T + 0.3.
5. **TAP lead diameter** is not in `tap.pdf` (it points to catalog page
   222); KYOCERA AVX's TAP/TEP tape-and-reel sheet prints it: d 0.50 ±0.05
   (`TapTep-2.pdf` p.2, https://www.kyocera-avx.com/docs/techinfo/Tantalum-NiobiumCapacitors/TapTep-2.pdf,
   found through the catalog https://catalogs.kyocera-avx.com/TantalumNiobium.pdf).
   One C-wire part (`TAP476K006CRW`) has no DigiKey lead spacing and no
   wire-form spacing table in either sheet; C wires are 5.00 mm on 159 of
   the 161 C-wire TAP listings at DigiKey, so 5.0.
6. **Footprint ids carry what makes the geometry differ**: lead diameter
   (`CAPE-RAD-5-2-L0.45-KEMET`), the series for SMD cans and discs
   (`CAPE-SMD-10-9P-EXV-KEMET`: EXV prints C 13.0, EDK 12.0;
   `CAP-DISC-10-D10-KJN-KEMET`), the body diameter for axials
   (`CAP-AX-7.62-L4.32-D3.05-KEMET`). The generator merges identical
   geometry (plan-33-F § 3).
7. **`pin1Shape` only on polarized parts**: R8's text in
   `docs/parts-format.md` now says so (ceramic radials and discs follow the
   generic `CAP-RADIAL-2.54` / `RES-RADIAL-2.54`, which have none).
8. **Mid-values where a range is printed** (AVX REA's D +0.50 −0 → D +
   0.25; GOLDMAX lead 0.51 +0.10 −0.025 → 0.548); single-bound maxima
   (GOLDMAX L, T; disc D, T; TAP D) as printed; AXIMAX takes R7's maximum
   lead (0.535).

## Summary

D is the larger drawing letter (cans come in many sizes). Its rules are C's;
nothing in D changes shipped data.
