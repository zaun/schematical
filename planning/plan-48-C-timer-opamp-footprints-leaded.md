# plan-48-C: Footprints — leaded packages (DIP, SOIC, SOP, SSOP, TSSOP, MSOP / VSSOP, SOT-23, TSOT, SC-70, TO-263)

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-48-B

C gives every leaded package line of the final pick a footprint: an existing
one when the datasheet's land pattern equals it, a new one drawn from the
datasheet's package drawing otherwise. It writes the new objects to
`planning/plan-48-data/footprints-C.json` and the line → footprint map to
`planning/plan-48-data/footprint-map.tsv`; G loads them into
`Parts/footprints.json` with the parts that use them.

References:

- plan-48-A § 2 (the package lines), plan-48-B (the final
  `footprints-needed.txt`, `specs.tsv`).
- `docs/parts-format.md` § Footprints (`dual`, `explicit`, `centre`,
  `slots`, units and axes).
- `planning/plan-42-D-regulator-footprints-leaded.md` § 3.1 (ids, reuse,
  shared ids, ready objects: the same rules, restated in § 3.1 here);
  `planning/plan-40-data/census/fp-check.py`, `fp-load.sh`, `fp-merge.py`
  (the tools C copies).

## Prerequisites

plan-48-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-48-B complete | `ls planning/completed/plan-48-B-*.md` → exists, or every B box ticked | NOT MET (2026-11-16: not started) |
| The final package lines exist | `head -1 planning/plan-48-data/footprints-needed.txt` → the header, and `awk -F'\t' '!/^#/ && $8==0' planning/plan-48-data/footprints-needed.txt \| wc -l` → 0 (the `--add` form lists only lines with an `add` row) | NOT MET (B Phase 3 writes it; the planning-time file lists the download set: 13 lines without a picked row) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `C` line of `footprints-needed.txt` has a row in
  `footprint-map.tsv` naming a footprint that is in `footprints-C.json` or
  in `Parts/footprints.json`: `python3 -I
  planning/plan-48-data/census/fp-check.py C` → `<n> lines, 0 without a
  footprint`.
- `footprints-C.json` loads through `footprint_check` with no error.

At planning time: 57 `C` lines carry a picked row (619 picked parts); 54 of
them have an existing footprint of the same family and pin count to check
for reuse, 3 have none (`SC-70-8`, `TSOT-23-5`, `TO-263 (DDPAK-7)`).

### Non-goals (explicit constraints)

- No existing footprint changes: one is reused or a new one added, never
  edited.
- `Parts/footprints.json` and the goldens are not touched here (G loads the
  objects).
- No footprint for a package no `add` row uses.

## 2. Current State

`footprints.json` has 1646 footprints (`wc -l < backend/tests/golden/footprints.txt`
→ 1646). The ones a reuse check starts from, by line (the `existing` column
of `footprints-needed.txt`): `PDIP-8`, `PDIP-14`; `SOIC-8` (TI D),
`SOIC-8-MAXIM-S8`, `SOIC-14` (TI D), `SOIC-14-NPA`, `SOIC-16`; `SOP-8-PSA`,
`SOP-14` (the 5.3 mm bodies); `SSOP-14`, `SSOP-16`, `QSOP-16`; `TSSOP-8`,
`TSSOP-14`, `TSSOP-16`, `TSSOP-20`, `HTSSOP-16-PWP`; `TVSOP-14`; `VSSOP-8`,
`VSSOP-8-DGK`, `VSSOP-10`; `SOT23-5` (TI DBV), `SOT23-6`, `SOT23-8-DCN`,
`SOT23-THIN-6`, `SOT23-THIN-8`; `SC70-5` (TI DCK), `SC70-6`,
`SC70-6-ADI-KS-6`.

### Measured populations

| What | Count | Command |
|---|---|---|
| `C` lines, download set / with a picked row | 61 / 57 (619 picked parts) | `awk -F'\t' '!/^#/ && $1=="C"{n++; if($8>0){c++; p+=$8}} END{print n,c,p}' planning/plan-48-data/footprints-needed.txt` |
| …with no existing footprint of the family and pin count | 3 | `awk -F'\t' '!/^#/ && $1=="C" && $8>0 && $12==""' planning/plan-48-data/footprints-needed.txt \| wc -l` |
| Picked parts per supplier package (lines) | 8-SOIC 165 (4), SOT-23-5 77 (3), 8-PDIP 46 (3), 14-TSSOP 39 (3), 14-SOIC 39 (2), 14-SO 34 (3), SC-70-5 27 (3), 8-MiniSO 26, 8-SO 25 (3), TSOT-23-5 22, 8-MSOP 22, 8-TSSOP 19 (3), 14-PDIP 16 (2), TSOT-23-6 7, SOT-23-8 6, 8-VSSOP 6, 10-VSSOP 6, SC-70-6 5 (2), 8-uMAX 5, SOT-23-6 4 (2), 10-MSOP 4, and 15 lines with 1 or 2 parts | `awk -F'\t' '!/^#/ && $8>0 && $1=="C"{n[$5]+=$8; l[$5]++} END{for(k in n)print n[k], l[k], k}' planning/plan-48-data/footprints-needed.txt \| sort -rn` |

## 3. Design

### 3.1 Ids, reuse, ready objects (C and D)

- **Ids.** A TI drawing takes TI's package designator, as the library's TI
  footprints do: `<outline>-<pins>-<designator>` (`SOT23-5` is TI DBV
  already; `VSSOP-8-DGK`); other makers `<outline>-<pins>-<MAKER>` with the
  maker's package code when the drawing has one (`TSOT-5-ADI-UJ`,
  `SOT23-8-ADI-RJ-8`, `MINISO-8-ST`, `SOIC-8-RENESAS-M8.15`); characters
  `A-Z 0-9 . -` only.
- **Reuse.** A drawing whose land pattern equals an existing footprint's
  (same pin count, every pad's designator, centre and size within 0.05 mm,
  same drill, body within 0.1 mm) reuses it; `Parts/footprints.json` is
  checked first. A datasheet with a mechanical drawing and no land pattern
  takes the IPC-7351 nominal pattern of its dimensions' mid-values, as the
  library's `note` fields record for earlier footprints.
- **Shared ids.** Before adding an id: `grep -c '"id": *"<id>"'
  schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json`;
  a hit means reuse it (checked as any reuse) and the add step is marked
  moot with that grep's output.
- **Ready objects.** `footprints-C.json` is `{"footprints":[…]}` in
  `footprints.json`'s format; each object's `note` names the drawing, the
  PDF path under `datasheets/` and the page.
- **The map.** `footprint-map.tsv`: `letter`, `company`, `package`,
  `supplier` (one supplier package of the line, or `*`), `footprint`,
  `note` (`reuse` or `new`). One row per (line, land pattern): a line whose
  datasheets show two land patterns for one DigiKey package gets two rows,
  each with a `pdf` column naming the datasheet it holds for.
- **Pin numbers are positions.** A footprint is reused only when its pad
  `n` sits where the datasheet's pin `n` does. Exposed pads: a numbered pad
  is a pin (`pins` counts it; `slots` with `0` puts it on `centre`); an
  unnumbered one is `centre` without a pin.

### 3.2 Tools

`census/fp-check.py`, `fp-load.sh`, `fp-merge.py`: copies of plan-40's with
the paths and the letter set (`C`, `D`) changed; `fp-load.sh` builds
`footprint_check` in a fresh `/tmp/p48-fp.XXXXXX`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Tools; DIP, SOIC, SOP

- [ ] `census/fp-check.py`, `fp-load.sh`, `fp-merge.py` per § 3.2;
      `footprint-map.tsv` and `footprints-C.json` created empty.
- [ ] The `8-PDIP`, `14-PDIP` lines (TI, ADI, Renesas): reuse check against
      `PDIP-8` / `PDIP-14`, or draw; map rows.
- [ ] The `8-SOIC`, `8-SO`, `14-SOIC`, `14-SO`, `16-SO` (3.9 mm), `8-SO` /
      `14-SO` / `14-SOP` (5.3 mm) and the `8-SO PowerPad` lines, per maker:
      reuse check or draw; map rows.

Acceptance: every DIP, SOIC and SOP line is mapped and the new objects load.
  Check: `python3 -I planning/plan-48-data/census/fp-check.py C | grep -c 'DIP\|SOIC\|-SO\b\|SOP'` → 0 missing lines of these packages, and `planning/plan-48-data/census/fp-load.sh planning/plan-48-data/footprints-C.json` → only `extra footprint` / `unused footprint` lines for the new ids (est. 4 min: one `footprint_check` build).
Commit: —

### Phase 2 — SSOP, TSSOP, TVSOP, MSOP / VSSOP / µMAX / MiniSO

- [ ] The `14-SSOP`, `16-SSOP`, `8-TSSOP`, `14-TSSOP`, `16-TSSOP`,
      `20-HTSSOP`, `14-TVSOP` lines: reuse check or draw; map rows.
- [ ] The 8- and 10-lead `VSSOP`, `HVSSOP`, `MSOP`, `uMAX/uSOP`, `MiniSO`
      lines (3.0 mm bodies; the `H` forms with their exposed pad): reuse
      check against `VSSOP-8`, `VSSOP-8-DGK`, `VSSOP-10`, or draw; map rows.

Acceptance: these lines are mapped and the objects load.
  Check: `fp-check.py C | grep -c 'SSOP\|TVSOP\|MSOP\|uMAX\|MiniSO'` → 0, and `fp-load.sh footprints-C.json` as in Phase 1 (est. 4 min).
Commit: —

### Phase 3 — SOT-23, TSOT, SC-70, TO-263

- [ ] The `SOT-23-5`, `SOT-23-6`, `SOT-23-8` lines (TI, ADI, ST): reuse check
      against `SOT23-5`, `SOT23-6`, `SOT23-8-DCN`, or draw; map rows.
- [ ] The `TSOT-23-5`, `TSOT-23-6`, `TSOT-6`, `TSOT-23-8` lines: reuse check
      against `SOT23-THIN-6`, `SOT23-THIN-8` and, when present,
      `TSOT-5-ADI-UJ` (plan-42's id; § 3.1 shared ids); else draw; map rows.
- [ ] The `SC-70-5`, `SC-70-6`, `SC-70-8` lines: reuse check against
      `SC70-5`, `SC70-6`, `SC70-6-ADI-KS-6`; `SC-70-8` is drawn (no 8-lead
      SC-70 exists); map rows.
- [ ] The `TO-263 (DDPAK-7)` line: drawn from its TI drawing (`explicit`
      style: seven leads and the tab), when it still carries an `add` row.

Acceptance: every `C` line is mapped; every new object loads.
  Check: `python3 -I planning/plan-48-data/census/fp-check.py C` → `<n> lines, 0 without a footprint`; `fp-load.sh footprints-C.json` → only `extra footprint` / `unused footprint` lines for exactly the ids of `footprints-C.json` (est. 4 min).
Commit: —

## Validation Plan

- Tests: `fp-check.py` (every line mapped), `fp-load.sh` (the loader's
  rules: pad counts, overlaps, key checks) on the ready objects.
- Coverage check: `python3 -I -c "import json;print(len(json.load(open('planning/plan-48-data/footprints-C.json'))['footprints']))"`
  is recorded in § Corrections; G's golden growth is compared with it.
- Doc sync: none (no new footprint style).
- Final gate: plan-48-I.

## Decisions

- Author: **plan-42-D's id, reuse and shared-id rules** apply unchanged.
- Author: **a line is drawn only while it carries an `add` row** of the
  final pick (the TO-263 line has one picked part at planning time).

## Corrections

## Summary

Mostly reuse checks: 54 of the 57 lines start from an existing footprint of
the same outline. The care is in pin-1 position and exposed pads.
