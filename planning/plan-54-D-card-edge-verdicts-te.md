# plan-54-D: Card-edge sockets, TE Connectivity — ordering tables, datasheet status, pin designators and facts, the final pick

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-54-C

D is C's work for **TE Connectivity**'s card-edge sockets: 300 candidate rows in
230 specs rows (a customer drawing or data sheet per part or per small group of
part numbers, 228 distinct downloads), series `Standard Edge II`, `Standard Edge
.050`, `Sliver 2.0`, `Crown Edge, Elcon`, `PCIe Gen 5`, `Mini Cool Edge 0.60mm`
and others. It writes each specs row's scheme, reads its status, writes its pin
designators and facts, fills the final pick, records the skips and the obsolete
PDFs, deletes the PDFs no `add` row uses and adds TE's lines to
`footprints-needed.txt` for J. The behavioral outcome: `candidates.tsv` has one
`add` row per TE card-edge socket the library will get, every other TE card-edge
row is `skip` or `out` with a reason, and `specs.tsv` has every column filled on
TE's 230 card-edge rows.

References:

- plan-54-B § 3 (scheme, status, facts, the pin-name rules) and § Phases (the
  scripts and the readers' method); plan-54-C § 3.3 – 3.4 (card-edge pin
  designators and facts); plan-54-A § 3, § 4.
- `AGENTS.md` (§ Datasheets); `datasheets/skipped-parts.md`,
  `datasheets/obsolete-not-included.md`.

## Prerequisites

plan-54-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-54-C complete | `ls planning/completed/plan-54-C-*.md` → exists | NOT MET (2026-11-22: not started) |
| The PDFs are placed | `python3 -I planning/plan-54-data/census/place.py --verify planning/plan-54-data/candidates.tsv` → `0 missing, 0 unused` | NOT MET (2026-11-22: A Phase 3) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-54-data/census/scheme-check.py --family edge --maker "TE Connectivity"`
  prints `<n> rows: <k> ok, 0 unchecked` with `<n>` = TE's card-edge candidate rows
  after A (300 at planning time); every row not `ok` is `skip` or `out` with a
  `reason`.
- `specs.tsv` has `scheme`, `label`, `card`, `pinout`, `positions`, `pitch` and
  `status` filled on every TE card-edge row (230 at planning time).
- `candidates.tsv` has `state` `add` on every such row that passes the rules of
  plan-54-A § 3; `footprints-needed.txt` has TE's lines (letter `J`).
- `datasheets/skipped-parts.md` § `Card slots (plan-54)` lists TE's skips; every
  PDF whose rows are all out for datasheet status is deleted and listed in
  `obsolete-not-included.md`; every PDF under `datasheets/connector-card-edge/te/`
  is used by an `add` row.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app; no
  Samtec or Amphenol ICC row (C); no TE card-socket or module row (B).
- No obsolete part is added with a flag (owner).

## 2. Current State

TE publishes a customer drawing per part (`DocType=Customer Drawing`, the
part number in `PartCntxt`), data sheets and catalog sheets for series, and
application specifications (`108-60121`, `501-60088`) that several parts cite and
that hold no part drawing. C leaves `specs.py`, `scheme-check.py`,
`pinout-check.py`, `final.py` and `footprint-lines.py` handling the edge family.

### Measured populations

| What | Count | Command |
|---|---|---|
| TE card-edge candidate rows, specs rows, downloads | **300**, **230**, **228** | `python3 -I planning/plan-54-data/census/report.py planning/plan-54-data/raw planning/plan-54-data/candidates.tsv`; `python3 -I -c` over `candidates.tsv` (distinct (series, src), distinct src for `maker == "TE Connectivity"`, `family == "edge"`) |
| Rows by TE document type | Customer Drawing 149, Specification Or Standard 53, Data Sheet 31, CS 32, product datasheet 17, DS 12, SS 6 | `python3 -I -c` over `candidates.tsv`: `DocType` of each row's `src` query (`product-<n>.datasheet.pdf` links have none) |
| Series | 16 (DigiKey gives `-` for 148 rows) | `python3 -I -c` as above, `series` |
| Card types | dual edge 121, PCI Express 121, M.2 (NGFF) 16, PCI Express Mini Card 10, PCI 9, PCI 5 V 64 bit 5, others | `awk -F'\t' '$20=="candidate" && $4=="edge" && $2=="TE Connectivity"{print $13}' planning/plan-54-data/candidates.tsv \| sort \| uniq -c` |
| Pitch (mm) | 1.00 122, 2.54 55, 0.60 44, 1.27 30, 0.50 16, none 13, 0.80 10, 12.90 7 | same with `$9` |
| Odd position counts | 17 | `awk -F'\t' '$20=="candidate" && $4=="edge" && $2=="TE Connectivity" && $8%2==1' planning/plan-54-data/candidates.tsv \| wc -l` |
| Footprint keys | **115** | `census/report.py` |

### Verified properties

- A TE application specification (`108-60121`) prints no part number, ordering
  table or land pattern: its 53 + 6 rows (Specification Or Standard, SS) have, at
  most, a drawing elsewhere; the rule "the datasheet prints the part or its
  scheme" (plan-54-A § 3 rule 6) makes them `skip` unless the same part's own
  drawing is another row's PDF. `textscan.py`'s `hit` column marks the rows whose
  PDF prints the part number (A Phase 3).
- 7 rows give DigiKey Pitch 12.90 mm, far from the 0.50 – 2.54 mm of the rest;
  the reader compares each with its drawing and C § 3.4's rule applies (the
  drawing's pitch is the part's `cardPitch`; a difference beyond rounding is
  `skip`, reason `fact not in datasheet`).

## 3. Design

As plan-54-B § 3 and plan-54-C § 3 — with these TE particulars:

- **Scheme.** A TE drawing's part-number table (`2-1734774-1`, `-2`, …) is the
  alternation of its listed numbers (the part's number plus the table's dash
  numbers); a drawing that covers one part has the one number. A row whose number
  the PDF does not print is `skip`.
- **Status.** TE drawings carry a revision block and sometimes `OBSOLETE`, `NO
  LONGER AVAILABLE` or `SUPERSEDED BY`: all three are status words for the scan
  (plus the plan-54-B list).
- **Pin designators.** As C § 3.3; TE PCI Express and PCI drawings name contacts
  `A1 … A<n>`, `B1 … B<n>`: pinout `AB`, or an explicit list (odd counts, the 64-bit
  PCI A / B numbering with its key gap).
- **Facts.** `card` as C § 3.4 plus `PCI Express Mini Card`, `PCI`, `PCI 64-bit`,
  `Special`, `AGP`, `NLX`, `VRM` as DigiKey's Card Type says (`™` and `5V`/`3.3V`
  words dropped from the text, voltage kept as `PCI 5V 64 bit`).

**Risk**: the volume (230 drawings, six readers of 38 or 39 rows each) and the application
specifications that make a row unprovable; both are bounded by the skip rule.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Ordering-code schemes and facts

- [ ] Six parallel readers (brief `planning/plan-54-data/specs-parts/README.md`
      with this letter's § 3; fragments `edge-te-1` … `edge-te-6`, 38 or 39 rows
      each, split by series then by PDF) fill the seven columns of the 230 rows;
      `assign.py --merge`.
- [ ] Run `scheme-check.py`; every failing row is `skip`. Print the `--sample` and
      compare 20 rows by hand.
- [ ] Apply plan-54-A § 3 rule 9 to the drawings: a row whose land pattern needs a
      rotated slot or a keyhole shell slot is `skip` (`footprint not expressible: <feature>`).
      The `Board Edge, Straddle Mount` rows (TE Connectivity 32 candidates) are **not** skipped for
      their bottom-layer pads: mark each in `specs.tsv` `status` as `edge-mount` so the
      footprint readers (H to K) draw it with `opposite` pads and `edgeMount` (plan-54-E);
      record the skip count in § Corrections.

Acceptance: every TE card-edge row has a verdict on the ordering table and the facts.
  Check: `python3 -I planning/plan-54-data/census/scheme-check.py --family edge --maker "TE Connectivity"` → `<n> rows: <k> ok, 0 unchecked`; `… --sample \| wc -l` → 20 (est. 10 min of scripts).
Commit: —

### Phase 2 — Datasheet status

- [ ] As C Phase 2 for TE's rows (the extra status words of § 3); delete and list
      each PDF whose every row is skipped for status.

Acceptance: no `add`-eligible row is obsolete.
  Check: `awk -F'\t' '$4=="edge" && $2=="TE Connectivity" && $24==1 && $20!="out" && $20!="skip"' planning/plan-54-data/candidates.tsv \| wc -l` → 0 (est. 20 min).
Commit: —

### Phase 3 — Pin designators and pinouts

- [ ] Run `pinout-check.py --family edge --maker "TE Connectivity"`; read every
      difference again in the PDF and record each confirmed one in `specs.tsv`
      `status` and § Corrections.

Acceptance: every TE specs row has a pinout the kind will accept.
  Check: `python3 -I planning/plan-54-data/census/pinout-check.py --family edge --maker "TE Connectivity"` → `<n> rows, 0 bad` with `<n>` = TE's rows after Phase 2 (est. 5 min).
Commit: —

### Phase 4 — The final pick, the skips, the prune

- [ ] `census/final.py` for TE's rows; `--write-skipped`; `place.py --prune` on
      `datasheets/connector-card-edge/te/`; `final.py --verify-skipped`.
- [ ] `census/footprint-lines.py`: TE's lines, letter `J`, id stem `EDGE-TE-<series
      slug>`.

Acceptance: the pick is final, consistent and pruned.
  Check: `awk -F'\t' '$20=="add" && $4=="edge" && $2=="TE Connectivity"' planning/plan-54-data/candidates.tsv \| wc -l` → the final pick (≤ 300); `python3 -I planning/plan-54-data/census/place.py --verify planning/plan-54-data/candidates.tsv` → `0 missing, 0 unused`; `python3 -I planning/plan-54-data/census/final.py --verify-skipped` → `0 missing from skipped-parts.md` (est. 20 min).
Commit: —

## Validation Plan

- Tests: the scripts of § Phases; no code changes.
- Coverage check: every TE card-edge candidate row has a verdict
  (`awk -F'\t' '$4=="edge" && $2=="TE Connectivity" && $20=="candidate"' planning/plan-54-data/candidates.tsv | wc -l` → 0).
- Runtime proof: not applicable (data only).
- Doc sync: `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`.
- Final gate: none; the landing tier is P's.

## Decisions

- Owner (2026-11-22): everything the makers list, only Active (plan-54-A § Decisions).
- Author: **A row whose only PDF is an application specification is `skip`**, not
  added on DigiKey's facts alone: the library takes a part only when its datasheet
  prints it (plan-54-A § 3 rule 6).

## Corrections

## Summary

D is the largest reading of the three verdict letters by PDF count (230), and the
one with the most expected skips (the application-specification rows). It changes
no code; the skip rule keeps an unprovable row out.
