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
| plan-33-C complete | `ls planning/completed/plan-33-C-*.md` → exists | NOT MET (2026-11-11: planning) |

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

- [ ] `footprints-generic.json`: `CAP-DISC-5.08`, `CAP-RADIAL-2.54` (§ 3).
- [ ] `footprints-D.json`: one object per ceramic radial, disc and axial row;
      fill the rows' columns.

Acceptance: those objects load.
  Check: `planning/plan-33-data/census/fp-load.sh planning/plan-33-data/footprints-C.json planning/plan-33-data/footprints-D.json planning/plan-33-data/footprints-generic.json` → only extra / unused lines (est. 3 min).
Commit: —

### Phase 2 — Cans and dipped tantalums

- [ ] `footprints-D.json`: one object per aluminum and aluminum-polymer
      can row (radial R8, SMD R9) and per dipped tantalum row (R8); fill the
      rows' columns.

Acceptance: § 1, both bullets.
  Check: the `awk` of § 1 → 0 and the Phase 1 `fp-load.sh` command → only extra / unused lines (est. 3 min).
Commit: —

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

(Filled in during execution.)

## Summary

D is the larger drawing letter (cans come in many sizes). Its rules are C's;
nothing in D changes shipped data.
