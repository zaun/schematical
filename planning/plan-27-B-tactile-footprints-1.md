# plan-27-B: Tactile switch footprints and specs — C&K, E-Switch, Omron

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-27-A

A downloaded the datasheets and decided every row it could. B reads the
datasheets of **C&K, E-Switch and Omron (DigiKey: Aratas)** once each and
produces, per (maker, series, mounting): the part-number key (so `pending`
rows become `add` or `skip`), the land pattern (a footprint object), and the
pinout (a `specs.tsv` row). It also fixes the design that C reuses (§ 4).

Outcome: every C&K, E-Switch and Omron row of `candidates.tsv` is `add`,
`skip` or `out`; `planning/plan-27-data/footprints-B.json` holds one ready
footprint object per distinct land pattern those `add` rows use; their
`specs.tsv` rows are complete.

References:

- `planning/plan-27-A-tactile-data.md` (§ 3 rules, § 4 placement),
  `planning/plan-27-data/candidates.tsv`, `footprints-needed.txt`, `specs.tsv`.
- `docs/parts-format.md` § Footprints (styles `dual`, `explicit`; pad rules;
  mid-values; no overlapping pads).
- `planning/completed/plan-20-B-buffer-families-footprints.md` § Corrections 2
  (precedent: footprints kept as ready objects until their parts land).
- Datasheets: `datasheets/switch/tactile/{ck,eswitch,omron}/`.

## Prerequisites

The gate for the whole feature is in plan-27-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-27-A is complete | `ls planning/completed/plan-27-A-*` → one file | MET 2026-11-08 (59aa52b) |
| No `pending` or `candidate` row of A | `awk -F'\t' 'NR>1 && ($18=="candidate" \|\| $18=="check")' planning/plan-27-data/candidates.tsv \| wc -l` → 0 | MET 2026-11-08: 0 |
| Datasheets in the tree | `ls datasheets/switch/tactile/ck/*.pdf \| wc -l` ≥ 1 | MET 2026-11-08 |

If plan-27-A is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- C&K, E-Switch and Omron: no row is `pending`; each is `add`, `skip` or `out`.
- `footprints-B.json` holds the footprints of the `add` rows; `footprint_check`
  loads them (§ 4.4 check).
- `specs.tsv` rows of these makers are complete; `specs-check.py` passes.

### Non-goals (explicit constraints)

- `Parts/footprints.json`, `Parts/switch.json`, goldens and the backend are
  not touched: footprints stay ready objects until the parts that use them
  land in E/F (`footprint_check` fails on an unused footprint).
- No part is added.

## 2. Current State

### Measured populations

| What | Count | Command |
|---|---|---|
| Candidate rows with a PDF: C&K / E-Switch / Aratas | 487 / 330 / 148 | `awk -F'\t' '$18=="candidate" && $20==1 && index($1,"C&K")==1' planning/plan-27-data/candidates.tsv \| wc -l` (A's `decision` is `add`/`pending` after A; the column-20 test is unchanged) |
| …of which verbatim (`hit`=1) | 440 / 14 / 145 | `awk … $21==1` |
| …`pending` (no verbatim hit) | 47 / 316 / 3 | `awk … $21==0` |
| Footprint proxy lines: C&K / E-Switch / Aratas | 68 / 67 / 13 | `grep -c '^ *[0-9]*  C&K' planning/plan-27-data/footprints-needed.txt` etc. (68, 67, 13) |
| Distinct datasheet URLs of those rows: C&K / E-Switch / Aratas | 76 / 70 / 19 | `awk -F'\t' '$18=="candidate" && $20==1 && index($1,"C&K")==1 {print $17}' planning/plan-27-data/candidates.tsv \| sort -u \| wc -l` (likewise E-Switch, Aratas) |
| `check` rows (blank DigiKey parameters): C&K 17, Aratas 5 | 22 | `awk -F'\t' '$18=="check"' planning/plan-27-data/candidates.tsv \| cut -c1-10 \| sort \| uniq -c` |
| Existing tactile footprints | 0 | `grep -c 'SW-TACT' schematical/schematical/Parts/footprints.json` → 0 |

### Verified properties

- E-Switch publishes one datasheet per series on S3
  (`configured-product-images.s3.amazonaws.com/Datasheets/TL1105.pdf` is
  `SERIES TL1105 SWITCHES`; its option list starts at text line 276,
  `TL1105`, `TL1105T`, `TL1105J`, `TL1105JA`, …), and a code such as
  `TL1105JAF100Q` is that series name plus option fields, which is why 316 of
  its 330 rows have no verbatim hit. B reads each series' key (§ 4.3).
- C&K datasheets are Littelfuse-hosted (`littelfuse.com/assetdocs/…`) but
  print the C&K name: `pdftotext` of the PTS636 file begins `C&K® Switch
  Datasheet  PTS636 Series` and mentions Littelfuse 11 times, C&K 9 times, so
  plan-27-A § 3 rule 3 (the maker's name on the first page) holds.
- The footprint loader accepts `dual` with `perSide`/`slots`, `explicit`
  pads with `drill`, `pin1Shape`, `radius`/`chamfer` corners
  (`docs/parts-format.md` § Footprints; `backend/tests/footprint_check.c`).

## 3. Design Overview

Per series, in order: key → decide rows → drawing → footprint → spec row. A
`check` row (DigiKey parameters blank) takes its circuit and illumination from
the same datasheet entry: not a non-illuminated SPST-NO/NC → `out` (`circuit` or
`lit`); otherwise it follows the same route as the rest of its series, and
its blank `circuit`, `actuator` (DigiKey's vocabulary: Standard, Plunger for
Cap, …) and `rating` (`<amps>A @ <volts>VDC`) columns in `candidates.tsv` are
filled from the datasheet's specification table, because E–I read them.
Risk concentrates in the drawings (a wrong pad coordinate is silent); the
backstop is `footprint_check` (malformed or overlapping pads) and the review
of the rendered footprint image in K. Byte-identity is not a gate here
(new data); the gate is the loader plus `specs-check.py`.

## 4. Detailed Design

### 4.1 Footprint rules for tactile switches

1. **Source.** The datasheet's recommended PCB land pattern / "PC board
   layout" drawing, top view. A datasheet with only a hole layout (through
   hole) gives hole centres; the pad is the drawing's land diameter if
   printed, else 1.7 × drill rounded to 0.05 mm.
2. **Pads** are the electrical terminals only, numbered as the datasheet
   numbers its terminals (1…N, N = 2 or 4). Non-plated locating pegs, mounting
   holes and shield tabs are not drawn.
3. **Style.** `dual` (with `perSide` 2 and `slots`) when the pads form two
   columns of two; `dual` with `pins` 2 and `perSide` 1 for a pair; otherwise
   `explicit` (pad `x`, `y` in mm, origin at the body centre, y down).
4. **Body** is the drawing's body outline width × height; mid-values for
   ranges (`docs/parts-format.md` "Mid-values"); `note` says which.
5. **Orientation** is the datasheet's top view, unrotated.
6. **Surface-mount** pads: `drill` omitted. **Through-hole**: `drill` = hole
   diameter (mid-value); terminal 1 `pin1Shape` `"rect"` only when the
   datasheet marks it.

### 4.2 Sharing and ids

Two land patterns are the same when every pad matches within 0.05 mm
(position, size, drill), the pad numbers agree, and the bodies match within
0.1 mm. They share one footprint. Id: `SW-TACT-<series>` for the first series
(alphabetical by maker, then series) that needs it, with `-<mounting>`
(`SMD`, `THT`, `SMD-RA`, `THT-RA`, `EDGE`) when one series needs several; its
`note` lists the datasheet PDF and page of the drawing and every other series
that shares it. Before adding an id, `grep -c '"id":"<ID>"'` over
`Parts/footprints.json` and every `footprints-*.json` of this plan; if it
exists the step is moot (shared with C).

### 4.3 `schemes.tsv` and `specs.tsv`

`planning/plan-27-data/schemes.tsv` (header `maker series regex source`):
`regex` is a Python `re` pattern that fully matches exactly the part numbers
the datasheet's part-number key allows for that (maker, series) (the key's
fields spelled out, not a catch-all); `source` is `file p.<page>`.
`scheme-check.py` (written in Phase 1) sets a `pending` row of a series to
`add` when its part number `re.fullmatch`es and to `skip` (reason `not in the
datasheet's ordering table`) otherwise.

`specs.tsv` row (A created the skeleton): `maker series mounting mpn_regex
terminals pins footprint note`.

- `mpn_regex`: which part numbers of the group this row covers (`.*` for all);
  several rows per group when the drawing or pin numbering differs.
- `terminals`: `2` or `4`.
- `pins`: the terminals by side, `A=<n>[,<n>];B=<n>[,<n>]`, pin numbers as
  the datasheet numbers them. Side A and side B are the two sides of the
  contact; pins listed on one side are joined inside the part (`A=1,2;B=3,4`:
  1–2 always connected, 3–4 always connected, the button joins the sides).
  A two-terminal switch is `A=1;B=2`.
- `footprint`: the id from § 4.2.

`specs-check.py` (Phase 1): every `add` row of the named maker matches the
`mpn_regex` of exactly one `specs.tsv` row of its (maker, series, mounting);
that row has `terminals`, `pins` and `footprint`; `pins` lists exactly
`terminals` distinct numbers; `footprint` is an id in a `footprints-*.json`;
the footprint has a pad for every listed number.

### 4.4 Ready objects and the load check

`footprints-B.json` is `{"footprints":[…]}` of ready objects.
`census/fp-load.sh` (Phase 1): copies `backend/` and the `Parts/` directory to
`/tmp/p27-fp`, appends every object of the `footprints-*.json` named on the
command line to the copy's `footprints.json`, builds `footprint_check` there
and runs it. Expected output: failures only of the form `extra footprint (not
in the golden)` and `unused footprint` for exactly the new ids, no load error.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Checkers

- [x] `census/scheme-check.py`, `census/specs-check.py`, `census/fp-load.sh`
      (+ its helper `census/fp-merge.py`) per § 4.3, § 4.4, with `--maker` to
      scope them; `--tsv` / `--schemes` / `--specs` / `--footprints` point them
      at a reader's work files (Correction 2). `fp-load.sh` also rejects
      overlapping pads (Correction 1): `/tmp/p27/fp-bad.json` (pads 1 and 2
      overlap) → `other failure: SW-TACT-SELFTEST: pads 1 and 2 overlap`, exit 1;
      `/tmp/p27/fp-malformed.json` (no `pins`) → `1 new footprints: FAILED`,
      exit 1; `/tmp/p27/fp-good.json` → `1 new footprints: load ok`, exit 0.
- [x] Self-test on two rows: a fake `schemes.tsv` regex for `TS02` run on a
      copy of `candidates.tsv` in `/tmp` turns a matching pending row `add`.
      `python3 -I /tmp/p27/scripts/b1selftest.py planning/plan-27-data/candidates.tsv`
      → `add   TS02-66-100-BK-100-LCR-D  (fake p.1)`, `skip
      TS02-66-100-BK-100-SCR-D  (TS02)`, `1 add, 1 skip, 0 still pending`,
      `selftest ok`.

Acceptance: the three tools run and reject a bad input.
  Check: `python3 -I planning/plan-27-data/census/specs-check.py --maker 'C&K'` → exits non-zero listing the unfinished `C&K` rows (est. 1 min).
  Result 2026-11-08 (before A's decisions): `… Y78B62324FP: still candidate`, `512 problems in 0 add rows`, exit 1.
Commit: 9e04003

### Phase 2 — C&K (68 proxy lines, 487 rows)

- [x] For each (series) in `footprints-needed.txt` lines starting `C&K`: open
      its PDF(s) in `datasheets/switch/tactile/ck/`; if the series has
      `pending` rows add its `schemes.tsv` row (§ 4.3). Done by the C&K reader
      (`work/ck/`, two passes; Correction 2): 30 scheme rows.
- [x] Run `scheme-check.py --maker 'C&K'`; record the add/skip counts in
      § Summary. After `merge-work.py B ck`: `44 add, 9 skip, 0 still pending`.
- [x] For each (series, mounting) with an `add` row: draw the land pattern into
      `footprints-B.json` (§ 4.1, § 4.2) and add its `specs.tsv` row(s).
      55 footprints, 101 spec rows. Seven C&K parts are `skip` from
      `work/ck/cannot-draw.tsv`: PTS831 ×3 (the product drawing the PDF refers
      to is not in it), SFS12 ×2 and G01743323 (no terminal numbers or circuit
      diagram), PTS645VK582LFS (DigiKey's datasheet for it is the PTS525 one).
      Three-terminal parts with two common terminals (PTS845, PTS847) are
      2-pin footprints whose pin 1 has two pads (Correction 3).
- [x] `planning/plan-27-data/census/fp-load.sh planning/plan-27-data/footprints-B.json` → `55 new footprints: load ok`.

Acceptance: no C&K row is `pending`; `specs-check.py` passes; the objects load.
  Check: `python3 -I planning/plan-27-data/census/specs-check.py --maker 'C&K' && planning/plan-27-data/census/fp-load.sh planning/plan-27-data/footprints-B.json` → `specs ok`, then only extra/unused failures for the new ids (est. 3 min).
  Result 2026-11-08: `specs ok (488 add rows)`; `55 new footprints: load ok`; C&K rows: 488 add, 24 skip, 60 out, 0 pending.
Commit: —

### Phase 3 — E-Switch (67 proxy lines, 330 rows)

- [ ] Same tasks as Phase 2 for `eswitch/` (key per series from the
      series datasheet's option list: the code's fields).
- [ ] `footprints-B.json` gains the E-Switch objects; ids shared with C&K's
      are reused, not redrawn (§ 4.2).

Acceptance: no E-Switch row is `pending`; `specs-check.py --maker E-Switch` passes; objects load.
  Check: the Phase 2 command with `--maker E-Switch` (est. 3 min).
Commit: —

### Phase 4 — Omron / Aratas (13 proxy lines, 148 rows)

- [ ] Same tasks for `omron/` (datasheet series B3F, B3W, B3FS, …).
- [ ] Write § Summary: add / skip counts per maker, number of footprints.

Acceptance: no Omron row is `pending`; `specs-check.py` passes for all three makers; objects load.
  Check: `python3 -I planning/plan-27-data/census/specs-check.py --maker 'Aratas' && awk -F'\t' '$1 ~ /^(C&K|E-Switch|Aratas)/ && ($18=="pending" || $18=="candidate")' planning/plan-27-data/candidates.tsv | wc -l` → `specs ok`, `0` (est. 3 min).
Commit: —

## Validation Plan

- Tests: `footprint_check` load via `fp-load.sh` (Phases 2–4); `specs-check.py`.
- Coverage check: Phase 4's `awk` → 0 shows every row of the three makers is decided.
- Runtime proof: none (data only); the footprints are drawn in the PCB view in K.
- Doc sync: none (format unchanged).
- Final gate (once, after Phase 4): `ctest --test-dir /tmp/logic-p27` → `100% tests passed` (est. 5 min; nothing was added to the library, so the count is T0).

## Decisions

- **Ready objects, not `footprints.json`** — author: `footprint_check` fails on
  an unused footprint (`backend/tests/footprint_check.c`), so the objects wait
  for their parts (E, F).
- **Terminals only, no pegs** — author (§ 4.1 rule 2).
- **One land pattern per series** — author: the land pattern of a series does
  not vary with option digits (height, force, colour); for makers that publish
  a drawing per code (TE, Würth: plan-27-C) the first code (by part number)
  of each (series, mounting) is the source. Rejected: reading every drawing
  (option digits change the cap, not the pads).
- **Scope, drawer, reuse of `switch` kind** — owner, 2026-11-07 (plan-27-A § Decisions).

## Corrections

1. 2026-11-08 — **§ 3's backstop claim was false**: "`footprint_check`
   (malformed or overlapping pads)". The loader has no overlap check: a
   footprint whose pads 1 and 2 overlap loaded clean (`/tmp/p27-fp/out.txt`:
   only `extra footprint` / `unused footprint` lines). `fp-merge.py --expect`
   (run by `fp-load.sh`) now tests every new footprint's pads, from the pad
   list `footprint_check` prints, for overlapping areas (touching is fine).
2. 2026-11-08 — the datasheets are read by parallel readers, one per maker
   group (plan-27-data/work/BRIEF.md), each writing
   `work/<slug>/{schemes,specs,checks}.tsv`, `footprints.json`, `notes.md`;
   the main session merges them into `schemes.tsv`, `specs.tsv`,
   `footprints-B.json` / `-C.json` and `candidates.tsv`, sharing identical land
   patterns across makers (§ 4.2). The checkers gained `--tsv`, `--schemes`,
   `--specs`, `--footprints` for that. Phase 1 (the checkers) does not read
   A's results and ran while A's census waited on the network.
3. 2026-11-08 — **§ 4.1 rule 2 / § 4.3 had no shape for three-terminal
   switches** (two terminals common, one the other side: C&K PTS845 /
   PTS847, E-Switch TL3340, Alps SKRT). They are modelled as two-terminal
   switches (`terminals` 2, `A=1;B=2`) whose footprint is `explicit` with two
   pads for pin 1 (`docs/parts-format.md` § `explicit`: a pin may have several
   pads), so both common terminals are soldered and joined.
4. 2026-11-08 — readers may find a part they cannot draw (no land pattern or
   no terminal numbering in its only datasheet) or a part whose DigiKey
   datasheet is the wrong document; they list it in
   `work/<slug>/cannot-draw.tsv` and `merge-work.py` makes it `skip` with that
   reason (it lands in `skipped-parts.md`).
5. 2026-11-08 — `fp-load.sh` relied on `footprint_check`'s failure lines,
   which stop after 10, so any batch over 10 new footprints "failed"; it now
   takes the new footprints from `footprint_check --print` minus the golden.

## Summary

The risk is silent drawing errors and the one-pattern-per-series shortcut; the
loader and the K images catch the first, not the second. Untouched:
`Parts/*.json`, goldens, backend, app.
