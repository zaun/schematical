# plan-47-F: Footprints — PTC resettable fuses

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-47-E

F draws the land patterns of the PTC resettable fuses of the final pick:
chip PTCs (0402 to 2920), the other surface-mount ones, and the radial
leaded discs, which are most of the lines because every hold current of a
radial series has its own body size and often its own lead spacing. The
behavioral outcome: every F line of `footprints-needed.txt` has a footprint
id in `footprint-map.tsv`, and the new objects of
`planning/plan-47-data/footprints-F.json` load through `footprint_check`.

References:

- plan-47-D § 3 (ids, reuse, the generator and `dims-X.tsv`, the map) and
  its tools.
- `docs/parts-format.md` § The `"footprint"` part key (R1; R8 radial leads:
  "a disc is D × T (its diameter by its thickness, as seen from above)").
- plan-47-B § 3.3 (PTC pinouts: `1=1,2=2`).
- `planning/completed/plan-33-C-capacitor-footprints-chip.md` § 4.1 (R8 on ceramic
  discs: the precedent for a radial disc).

## Prerequisites

plan-47-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-47-E complete | `ls planning/completed/plan-47-E-*.md` → exists | NOT MET (2026-11-16: not started) |
| No `PTC-` footprint yet | `python3 -I -c "import json;print(sum(f['id'].startswith('PTC-') for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` → 0 | MET (2026-11-16: 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-47-data/census/fp-check.py F` → `<n> lines, 0
  without a footprint` (287 lines at planning time, from the download set).
- `planning/plan-47-data/census/fp-load.sh planning/plan-47-data/footprints-F.json`
  → `<k> new footprints: load ok`.

### Non-goals (explicit constraints)

- No change to `Parts/footprints.json`, the part files, `backend/` or the
  app.
- No kinked-lead or tape-and-reel lead-form variant beyond what a picked
  part number orders: a part whose order code selects kinked leads gets the
  kinked spacing of its datasheet's table.

## 2. Current State

`footprints.json` has no PTC footprint. The radial series' datasheets
(Littelfuse RXEF, RUEF, RGEF, RHEF, RKEF, LVR, 30R, 60R, 72R; Bourns MF-R,
MF-RHT, MF-RX/72, MF-RG, MF-RM; Bel Fuse 0ZR…) each print one dimension
table: a row per part with the body's maximum width, height and thickness,
the lead spacing and the lead diameter.

### Measured populations

At planning time, from the download set (re-measure at F's start).

| What | Count | Command |
|---|---|---|
| F lines | 287 (710 parts): Littelfuse 117, Bel Fuse 91, Bourns 79 | `awk -F'\t' '!/^#/ && $1=="F"{print $3}' planning/plan-47-data/footprints-needed.txt \| sort \| uniq -c` |
| …radial discs | 231 lines, 317 parts (Littelfuse 88, Bel Fuse 82, Bourns 61) | `awk -F'\t' '!/^#/ && $1=="F" && $5=="Through Hole"' … \| wc -l` |
| …chip codes (`0402` … `2920`, "Concave") | 42 lines, 354 parts | `$5=="Surface Mount" && $6 ~ /^[0-9]+ \(/` |
| …other surface mount (`2-SMD`, `Nonstandard SMD`) | 14 lines, 39 parts | `$5=="Surface Mount" && $6 !~ /^[0-9]+ \(/` |
| Datasheets behind the lines | 101 distinct PDFs for the type | plan-47-B § 2 |
| Lead spacings DigiKey gives the radial rows | 5.10 mm, 5.05 mm, 10.20 mm, 5.08 mm, 10.15 mm, 2.51 mm, 5.00 mm, 2.90 mm among the listing's twelve values | `zcat < planning/plan-47-data/raw/ptc-*-tht.jsonl.gz \| python3 -I -c "import sys,json,collections;print(collections.Counter(json.loads(l)['params'].get('Lead Spacing') for l in sys.stdin).most_common(12))"` |

### Verified properties

- **R8 covers a radial disc** (`docs/parts-format.md`): `dual`, `pins` 2,
  `perSide` 1, `pitch` 1, `span` = the lead spacing (mid-value), `drill` and
  round pad by R5, body D × T as seen from above.
- **`fp-gen.py` writes R8 rows** (plan-47-D § 3.2, `family` `radial`).

## 3. Design

Ids (plan-47-D § 3.1): `PTC-<LLWW>-<MAKER>` for a surface-mount part;
`PTC-RAD-<W>-<P>-<MAKER>` for a radial disc, `W` the body's width (the
disc's diameter or its `A` dimension) and `P` the lead spacing, both mm ×
10 (`PTC-RAD-74-51-LITTELFUSE`). MAKER `LITTELFUSE`, `BOURNS`, `BELFUSE`.

Radial discs: one `dims-F.tsv` row per distinct (maker, body width, body
thickness, lead spacing, lead diameter) of the datasheets' dimension tables;
two parts whose rows are equal share the footprint, so the 231 lines
collapse to the distinct geometries (the count is recorded in § Corrections
when Phase 2 lands). Body: `body_w` = the width `A` (maximum, as the tables
give only a maximum), `body_h` = the thickness; `span` the nominal lead
spacing; `drill` by R5 from the lead diameter. The rows are filled from the
tables with `census/ptc-dims.py <pdf> <page>`, a helper F writes: it prints
the dimension table's rows from `pdftotext -layout` for the hand transfer
into `dims-F.tsv` (it does not write the table itself: the makers' column
orders differ).

Surface-mount PTCs: the datasheet's recommended pad layout (R1) per (series,
size); "Concave" is DigiKey's word for the terminations and changes nothing
in the land pattern. Reuse by `fp-compare.py` against the fuse footprints of
E and the library's chip footprints.

Where the risk concentrates: the volume (231 radial lines) and inch-first
tables (Littelfuse and Bourns print inches and millimetres; the millimetre
column is taken). The check is `fp-gen.py`'s own validation (span larger
than the drill, the body at least as wide as the span minus the pad) and a
hand check of three parts per radial series: the smallest, the largest and
one in the middle.

Rejected alternatives: (1) one footprint per (series, lead spacing) with
the largest body (rejected: a 0.5 A part would claim the board area of a
9 A part); (2) reading the tables fully by script (rejected: thirteen
layouts for 101 PDFs; the helper prints, a person transfers).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Surface-mount PTCs

- [ ] The 56 surface-mount lines (at planning time): `dims-F.tsv` rows or
      reuse; map rows; a hand check per size code.

Acceptance: those lines have footprints that load.
  Check: `python3 -I planning/plan-47-data/census/fp-gen.py planning/plan-47-data/dims-F.tsv > planning/plan-47-data/footprints-F.json && planning/plan-47-data/census/fp-load.sh planning/plan-47-data/footprints-F.json` → `<k> new footprints: load ok` (est. 3 min).
Commit: —

### Phase 2 — Radial discs: Littelfuse

- [ ] `census/ptc-dims.py`; the Littelfuse radial lines (88 at planning
      time): `dims-F.tsv` rows, map rows, the three-part hand check per
      series.

Acceptance: those lines have footprints that load.
  Check: the Phase 1 command → `<k> new footprints: load ok`; `python3 -I planning/plan-47-data/census/fp-check.py F` → the lines without a footprint are Bourns' and Bel Fuse's radial ones (est. 3 min).
Commit: —

### Phase 3 — Radial discs: Bourns and Bel Fuse

- [ ] The Bourns (61) and Bel Fuse (82) radial lines as Phase 2. Record the
      number of distinct radial footprints in § Corrections.

Acceptance: every F line has a footprint that loads.
  Check: `python3 -I planning/plan-47-data/census/fp-check.py F` → `<n> lines, 0 without a footprint`; the Phase 1 command → `<k> new footprints: load ok` (est. 3 min).
Commit: —

## Validation Plan

- Tests: `footprint_check` through `fp-load.sh`.
- Coverage check: `fp-check.py F`.
- Runtime proof: n/a (K puts the ids on parts).
- Doc sync: none.
- Final gate: plan-47-O.

## Decisions

- **A radial PTC's footprint is its own body and lead spacing**; equal
  geometries share an id (§ 3 rejected alternative 1).
- **The body width is the table's maximum** (the tables give no minimum);
  the `note` says so.
- **Kinked-lead part numbers get the kinked spacing** the datasheet gives
  for their order code.

## Corrections

## Summary

F's risk is volume: 231 radial lines read from dimension tables. The
generator's validation and the three-part check per series bound it.
Untouched: every existing footprint, the part files, the backend and the
app.
