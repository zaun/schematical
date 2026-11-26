# plan-69-D: TVS footprints — array packages

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-69-C

D gives every `D` line of `footprints-needed.txt` (a package with three or more
leads, or a multi-pad DFN / chip-scale package) a footprint, by C's rules. New
objects go to `planning/plan-69-data/footprints-D.json`, the map rows to
`footprint-map.tsv`; F and G load them.

References:

- plan-69-C § 3.1 (ids, reuse, shared ids, ready objects, the map) and its
  `census/tvs-fp-*` tools.
- `docs/parts-format.md` § Footprints (`dual`, `quad`, `explicit`, `centre`,
  `slots`).
- `planning/completed/plan-48-D-timer-opamp-footprints-leadless.md` (the DFN / QFN /
  chip-scale precedent: exposed pads, `centre`, maker-coded ids).

## Prerequisites

plan-69-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-69-C complete | `ls planning/completed/plan-69-C-*.md` → exists, or every C box ticked | NOT MET (2026-11-25: C not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-69-data/census/tvs-fp-check.py D` → `<n> lines, 0 without
  a footprint`.
- `census/tvs-fp-load.sh planning/plan-69-data/footprints-C.json planning/plan-69-data/footprints-D.json`
  shows only "extra footprint" / "unused footprint" lines for exactly the new ids.

### Non-goals (explicit constraints)

- No existing footprint changes; no footprint for a package no `add` row uses.
- `Parts/footprints.json` and the goldens are not touched here.

## 2. Current State

The small-outline footprints a reuse check starts from include `SOT23-3`,
`SOT23-3-ONSEMI`, `SOT23-3-NEXPERIA`, `SOT23-5`, `SOT23-6`,
`SOT23-6-DIODES-SOT26-A1`, `SOT26-6-DIODES`, `SC70-5`, `SC70-6`, `SC70-6-VISHAY`,
`SC88-6`, `SC88A-5`, `SOT323-3`, `SOT323-3-DIODES`, `SOT363-6`, `SOT563-6`,
`SOT563-6-DIODES`, `SOT666-6-NEXPERIA`, `SOT723-3`, `SOT923F-3-ONSEMI`,
`SOT963-6-DIODES`, `SOIC-8`, `SOIC-14`, `SOIC-16`, the `X1DFN1006-3-DIODES`,
`X2DFN1006-3-DIODES`, `UDFN-6-DIODES-U-DFN2018-6`, `UDFN-6-DIODES-U-DFN2020-6`,
`DFN3020-8-DIODES` Diodes outlines (read from `footprints.json`, 2026-11-25). There
is no Semtech footprint (`grep -c '"id": *"[^"]*SEMTECH' schematical/schematical/Parts/footprints.json`
→ 0); the 112 ids with `SEMTECH` or `LITTELFUSE` in them are all Littelfuse fuse
outlines (`FUSE-CLIP-5X20-LITTELFUSE`, `FUSE-1005-LITTELFUSE`, …).
In the steering census (all makers) the packages are SOT-23-6 55, 10-UFDFN 53,
8-SOIC 28, SOT-363 24, 6-UFDFN EP 17, TO-253-4 16, SOT-563/666 15, 6-UFDFN 14,
10-MSOP 14, 9-UDFN 13 and 52 more values
(`python3 -I planning/plan-69-data/census/show.py planning/plan-69-data/raw/tvss.json 'Package / Case'`).

## 3. Design

C § 3.1 holds. In addition:

- **Ids.** `<outline>-<pins>-<MAKER>-<code>` with the maker's package code when the
  drawing has one (`DFN-10-SEMTECH-SLP2510P8`, `UDFN-10-LITTELFUSE-UTG`,
  `SOT23-6-LITTELFUSE`), else `<outline>-<pins>-<MAKER>`.
- **Exposed pads.** A numbered pad is a pin (`slots` `0` puts it on `centre`); an
  unnumbered one is `centre` with no pin (B § 3.1 leaves it out of the pinout).
- **Flow-through DFNs** (the 10-lead Semtech and Littelfuse arrays whose line pins
  pair across the body): pads by the drawing, `explicit` when no `dual` row fits.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Reuse pass

- [ ] For every `D` line with an `existing` id: compare per C § 3.1; matches are
      `reuse` rows.

Acceptance: every reusable line is mapped.
  Check: `python3 -I planning/plan-69-data/census/tvs-fp-check.py D --reuse` → `<n> lines reused, <m> lines need a new footprint` (est. < 1 min).
Commit:

### Phase 2 — New footprints

- [ ] For every remaining `D` line: a new object in `footprints-D.json` from its
      drawing, after the shared-id grep; a `new` map row.
- [ ] `census/tvs-fp-load.sh` on both ready files.

Acceptance: every `D` line has a footprint and the new ones load with C's.
  Check: `python3 -I planning/plan-69-data/census/tvs-fp-check.py D` → `0 without a footprint`; `census/tvs-fp-load.sh planning/plan-69-data/footprints-C.json planning/plan-69-data/footprints-D.json` → only the new ids' lines (est. 3 min).
Commit:

## Validation Plan

- Tests: `footprint_check` through `tvs-fp-load.sh`.
- Coverage check: the `new` D rows of `footprint-map.tsv` equal the object count of
  `footprints-D.json`.
- Final gate: plan-69-I.

## Decisions

- Author: **array footprints are split from two-terminal ones** by the effort rule:
  the arrays' DFN drawings are maker-specific and each is drawn, where C mostly
  reuses.

## Corrections

## Summary

The maker-specific DFNs of the Semtech and Littelfuse arrays are the work; the
SOT and SC outlines mostly reuse.
