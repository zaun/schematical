# plan-37-B: Registers — the four new footprints

Last updated: 2026-11-12
Effort: medium (1h–2h)
Depends on: plan-37-A

B draws the land patterns the plan-37 `add` rows need and the library does
not have: **DHVQFN-20**, **DHVQFN-24** (Nexperia), **DQFN-20-ONSEMI**
(onsemi) and **TSSOP-64** (TI DGG). They are kept as ready objects in
`planning/plan-37-data/footprints-B.json`. The parts letters merge each one
into `Parts/footprints.json` together with the first parts that use it
(DHVQFN-24 in D, TSSOP-64 in E, DHVQFN-20 and DQFN-20-ONSEMI in F), because
`footprint_check` fails on a footprint no part uses
(`backend/tests/footprint_check.c:4`, `unused footprint: %s` at :172).
Every other package reuses an existing id (`census/footprints.py`, the
outline table in its header).

Outcome of B: `census/fp-load.sh planning/plan-37-data/footprints-B.json`
reports only `extra footprint` / `unused footprint` lines, for exactly the
four new ids, and no load error.

References:

- plan-37-A § Prerequisites (the gate for all of plan-37).
- `docs/parts-format.md` § Footprints (§ Common keys, § `quad`, § `dual`,
  `centre`, `slots`, mid-values, the `note`), § The `"footprint"` part key.
- `planning/completed/plan-03-B-logic-footprints.md` § 3 (pad rules R1–R6).
- `planning/plan-35-data/census/fp-load.sh`, `fp-merge.py` (copied here,
  paths changed).
- Existing precedents: `VQFN-20` (TI RKS, 2.5 × 4.5 mm, `quad`, `sides`
  `[8, 2, 8, 2]`, `centre` 1 × 3, `slots`), `TSSOP-56` (TI DGG0056A,
  `dual`, pitch 0.5, span 7.5, pad 1.5 × 0.3).

## Prerequisites

plan-37-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-37-A complete | `ls planning/completed/plan-37-A-*.md` → exists | MET (2026-11-13: `planning/completed/plan-37-A-register-data.md`) |
| The four ids are still new | `grep -c '"id": *"\(DHVQFN-20\|DHVQFN-24\|DQFN-20-ONSEMI\|TSSOP-64\)"' schematical/schematical/Parts/footprints.json` → 0 | MET (2026-11-13, B start: 0). Another plan adding one of them first makes that footprint's Phase 2 line moot: reuse it, and record it in § Corrections |
| Each new id still has an `add` row after A | `awk -F'\t' '$19=="add"{print $20}' planning/plan-37-data/candidates.tsv \| sort \| uniq -c \| grep -E 'DHVQFN\|DQFN\|TSSOP-64'` → four lines | MET (2026-11-13 after A: DHVQFN-20 9, DHVQFN-24 1, DQFN-20-ONSEMI 1, TSSOP-64 1 `add` rows). Planning: the same counts as in-scope rows (`footprints-needed.txt`). An id whose rows are all `skip`/`out` after A is not drawn (it would be unused): record it in § Corrections |

## 1. Goal

- `footprints-B.json` holds one object per id with an `add` row, in the
  `footprints.json` format, each with a `note` naming its source document,
  page and rule.
- `fp-load.sh footprints-B.json` → only `extra footprint` / `unused
  footprint` lines naming exactly those ids.

### Non-goals (explicit constraints)

- `Parts/footprints.json`, the part files and the goldens are not touched
  here (D, E, F merge).
- No change to an existing footprint, and no new id for a package an
  existing id already covers (the JEDEC / JEITA outline table in
  `census/footprints.py`).
- No change to the footprint format.

## 2. Current State

- 1386 footprints (`wc -l < backend/tests/golden/footprints.txt` → 1386 at
  cf40403). All logic packages are TI land patterns; no Nexperia footprint
  exists (`grep -c -i nexperia schematical/schematical/Parts/footprints.json`
  → 0); onsemi ones exist only for diodes (`SOT23-3-ONSEMI`,
  `DIODE-SMA-ONSEMI`, …).
- Packages the plan-37 rows use, with part counts: `footprints-needed.txt`
  (64 lines, 4 `NEW`: `grep -vc "^#" footprints-needed.txt` → 64).

### Verified properties (sources read at planning, 2026-11-12)

| Id | Source | Rule | What the source gives |
|---|---|---|---|
| DHVQFN-20 | Nexperia `package-information/SOT764-1.pdf` (5 pages, 143400 bytes, fetched with `curl_cffi`) | R1 | "Fig. 2. Reflow soldering footprint for DHVQFN20 (SOT764-1)" (solder land, paste, footprint 5.75 × 3.75 mm); body 4.5 × 2.5 × 1 mm, pitch 0.5 |
| DHVQFN-24 | Nexperia `package-information/SOT815-1.pdf` (`%PDF-`) | R1 | "Fig. 2. Reflow soldering footprint for DHVQFN24 (SOT815-1)", footprint 6.75 × 4.75 mm; body 3.5 × 5.5 × 0.85 mm (`74LVC823A.pdf`: `74LVC823ABQ … DHVQFN24 … SOT815-1`) |
| DQFN-20-ONSEMI | onsemi `74lcx374-d.pdf` (from `https://www.onsemi.com/download/data-sheet/pdf/74lcx374-d.pdf`, `%PDF-`, 938318 bytes) | R1 | ordering line `74LCX374BQX(2) MLP20B 20-Terminal Depopulated Quad Very-Thin Flat Pack No Leads (DQFN), JEDEC MO-241, 2.5 x 4.5mm`; "Pad Assignments for DQFN"; "LAND PATTERN RECOMMENDATION" (Figure 5) |
| TSSOP-64 | TI `sn74avc16722.pdf` | R2 | `DGG (R-PDSO-G**)` outline 4040078/F 12/97 (outline only; no board layout); PACKAGING INFORMATION `SN74AVC16722DGGR Active … TSSOP (DGG) \| 64` |

The Nexperia package PDFs sit at
`https://assets.nexperia.com/documents/package-information/<SOT>.pdf`
(the `…/reflow-soldering/<SOT>_fr.pdf` form answers 404). onsemi answers
`/pdf/datasheet/74lcx374-d.pdf` with an HTML page on some requests; the
`/download/data-sheet/pdf/` form gave the PDF every time.

## 3. Design

- **DHVQFN-20, DHVQFN-24** — `quad`, pads and centre pad from the reflow
  footprint figure (R1, mid-values), `sides` and `slots` from the package
  drawing's terminal numbering, so pin numbers match the SO/TSSOP numbering
  the parts' `p20` / `p24` pinouts use (the `VQFN-20` precedent: the exposed
  pad is `centre`, not a pin).
- **DQFN-20-ONSEMI** — `quad`, from Figure 5 and the "Pad Assignments for
  DQFN" view (R1); `-ONSEMI` suffix as the onsemi diode footprints. It is a
  separate id from DHVQFN-20 and VQFN-20 although all three are 2.5 × 4.5 mm
  MO-241 bodies: each maker's recommended lands differ, and R1 takes the
  maker's.
- **TSSOP-64** — `dual`, pitch 0.5; pad and span by R2 from the DGG outline
  (lead width, foot length and E mid-values), body 6.1 × 17.0 mid; the
  `TSSOP-56` precedent for the note's form.
- Each `note` cites: maker, document, page, figure, rule.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Tools

- [x] Copy `planning/plan-35-data/census/fp-load.sh` and `fp-merge.py` to
      `planning/plan-37-data/census/`, changing `p35` to `p37` in temp-dir
      names and the header comments.
- [x] Download the two Nexperia package PDFs with `census/nxget.py` into
      `datasheets/register/` (`sot764-1.pdf`, `sot815-1.pdf`) and add their
      lines to `planning/plan-37-data/datasheets.tsv`. They are footprint
      sources, not part datasheets: no `datasheets.json` entry.
      (`200 application/pdf 143400` and `200 application/pdf 148147`;
      5 pages each.)

Check: `echo '{"footprints":[]}' > /tmp/p37-fp0.json && planning/plan-37-data/census/fp-load.sh /tmp/p37-fp0.json`
→ `0 new footprints: load ok` (est. 3 min, one footprint_check build).
Observed: `0 footprints appended` / `0 new footprints: load ok`.

Commit: (this commit)

### Phase 2 — The footprints

- [x] DHVQFN-20 (SOT764-1 Fig. 2): `quad` [8, 2, 8, 2], pad 1.0 × 0.29,
      spanX 2.7, spanY 4.5, centre 0.9 × 2.9, VQFN-20's `slots`.
- [x] DHVQFN-24 (SOT815-1 Fig. 2): `quad` [10, 4, 10, 4] (§ Corrections:
      the ends are 1.5 pitch), pad 0.9 × 0.29, spanX 3.6, spanY 5.6,
      centre 2.0 × 4.0.
- [x] DQFN-20-ONSEMI (`74lcx374-d.pdf` Figure 5, PDF p.13; the PDF is A's,
      under `datasheets/ff-d/`): `quad` [8, 2, 8, 2], pad 0.9 × 0.24,
      spanX 2.6, spanY 4.6, centre 1.0 × 2.9.
- [x] TSSOP-64 (`sn74avc16722.pdf` DGG outline p.13, R2 mid-values as the
      SSOP-56 / TVSOP-48 notes apply it): pad 1.43 × 0.27, span 7.67,
      body 6.1 × 17.0.
- [x] Write them to `planning/plan-37-data/footprints-B.json`; set the
      `footprint` column of every row of these packages in `candidates.tsv`
      (already set by `census/footprints.py`: `git diff planning/plan-37-data/candidates.tsv` → no `footprint` change).

Check: `planning/plan-37-data/census/fp-load.sh planning/plan-37-data/footprints-B.json`
→ only `extra footprint` / `unused footprint` lines naming exactly the
ids written, no overlapping pads (`fp-merge.py --expect`) (est. 3 min).
Observed: `4 footprints appended` / `4 new footprints: load ok`;
`git diff a789481 -- planning/plan-37-data/candidates.tsv` → empty (no
`footprint` change).

Commit: (this commit)

## Decisions

- **Four new ids, not five.** Planning mapped SN74AS575NT (24-pin 300 mil
  NT PDIP) to a new `PDIP-24`; that part is not in its datasheet's
  PACKAGING INFORMATION table (`sn74als575a.pdf`, read at planning), so it
  is `out` and no part would use a PDIP-24.
- **Reuse by outline.** Nexperia SOT109-1 / SOT163-1 / SOT338-1 /
  SOT339-1 / SOT403-1 / SOT360-1 / SOT370-1 / SOT362-1 / SOT371-1 /
  SOT364-1 and onsemi 751B / 751D / 948F / 948E / 738 / SOEIAJ cases are
  the JEDEC / JEITA outlines (MS-012, MS-013, MO-150, MO-153, MO-118,
  MS-001, EIAJ type II) the existing TI-drawn ids follow; they reuse them
  (SOT109-1 → `MS-012` is printed on its outline page in `74HC_HCT173.pdf`).
- **Staging, not merging**: the plan-35-B / plan-36-C precedent.

## Corrections

- **2026-11-13 — DHVQFN-24's end terminals are 1.5 mm apart.** SOT815-1
  Fig. 1 (p.2) gives e2 = 1.5 for terminals 1/24 and 12/13, while the long
  rows are at e = 0.5; `quad` has one pitch. The footprint gives each end
  4 slots at 0.5 pitch and puts pins on the outer two (±0.75 = 1.5 apart);
  the middle slots name no pin and get no pad (`docs/parts-format.md`
  § Common keys: "A slot no pin names gets no pad").
- **2026-11-13 — the Phase 2 check's diff base.** `git diff
  planning/plan-37-data/candidates.tsv` is empty against A's last
  commit (a789481), which is the base B's rows are checked against.

## Summary

Four footprints, each from its maker's drawing, staged for D, E and F.
