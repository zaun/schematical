# plan-69-C: TVS footprints — two-terminal packages

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-69-B

C gives every `C` line of `footprints-needed.txt` (a part with two terminals, or a
two-pad chip package) a footprint: an existing one when the datasheet's land
pattern equals it, a new one drawn from the datasheet's package drawing otherwise.
It writes the new objects to `planning/plan-69-data/footprints-C.json` and the line
→ footprint map to `planning/plan-69-data/footprint-map.tsv`; F loads them into
`Parts/footprints.json` with the parts that use them.

References:

- plan-69-A § 2 (the planning package spread), plan-69-B (the final
  `footprints-needed.txt`, `specs.tsv`'s `pads`).
- `docs/parts-format.md` § Footprints (`dual`, `explicit`, `centre`, units, axes;
  the `DIODE-*` precedents).
- `planning/completed/plan-48-C-timer-opamp-footprints-leaded.md` § 3.1 (ids, reuse,
  shared ids, ready objects, the map: the same rules, restated in § 3.1 here);
  `planning/plan-48-data/census/fp-check.py`, `fp-load.sh`, `fp-merge.py` (the tools
  C copies as `tvs-fp-check.py`, `tvs-fp-load.sh`, `tvs-fp-merge.py`).
- `planning/completed/plan-29-B-diode-footprints-vishay-single.md` and
  `plan-29-E-diode-footprints-zener.md` (the diode outlines the library already has).

## Prerequisites

plan-69-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-69-B complete | `ls planning/completed/plan-69-B-*.md` → exists, or every B box ticked | NOT MET (2026-11-25: B not started) |
| The final package lines exist | `head -1 planning/plan-69-data/footprints-needed.txt` → the header; `awk -F'\t' '!/^#/ && $1=="C" && $7==0' planning/plan-69-data/footprints-needed.txt \| wc -l` → 0 (only lines with an `add` row) | NOT MET (2026-11-25: B not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `C` line has a row in `footprint-map.tsv` naming a footprint in
  `footprints-C.json` or in `Parts/footprints.json`:
  `python3 -I planning/plan-69-data/census/tvs-fp-check.py C` → `<n> lines, 0 without
  a footprint`.
- `census/tvs-fp-load.sh planning/plan-69-data/footprints-C.json` shows only
  "extra footprint" and "unused footprint" lines for exactly the new ids.

### Non-goals (explicit constraints)

- No existing footprint changes: one is reused or a new one added, never edited.
- `Parts/footprints.json` and the goldens are not touched here (F loads the objects).
- No footprint for a package no `add` row uses.

## 2. Current State

`footprints.json` has 3993 footprints. The two-terminal outlines a reuse check
starts from (`python3 -c "import json;print([f['id'] for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints'] if f['id'].startswith('DIODE-')])"`):
`DIODE-SMA`, `DIODE-SMA-ONSEMI`, `DIODE-SMA-403AE`, `DIODE-SMA-TSC`, `DIODE-SMA-BZG`,
`DIODE-SMAF`, `DIODE-SMA-FL`, `DIODE-SLIMSMA`, `DIODE-THIN-SMA`, `DIODE-SMB`,
`DIODE-SMB-ONSEMI`, `DIODE-SMB-403AF`, `DIODE-SMB-HV`, `DIODE-SMC`,
`DIODE-SMC-ONSEMI`, `DIODE-SMC-403AG`, `DIODE-SMF`, `DIODE-MICROSMF`,
`DIODE-SOD-123`, `DIODE-SOD-123F`, `DIODE-SOD-123FL`, `DIODE-SOD-123W`,
`DIODE-SOD-123HE`, `DIODE-SOD-128`, `DIODE-SOD-323`, `DIODE-SOD-323F`,
`DIODE-SOD-523`, `DIODE-SOD-723`, `DIODE-SOD-923`, `DIODE-DFN1006`,
`DIODE-CLP1006`, `DIODE-DO-15`, `DIODE-DO-35`, `DIODE-DO-41`, `DIODE-DO-201AD`,
`DIODE-DO-218AC`, `DIODE-SOD-57`, `DIODE-SOD-64`, `DIODE-MELF`, `DIODE-MINIMELF`.
No footprint is named for P600 axial, DO-201AE, DO-218AB, DO-219AA/AB, SOD-882,
0201 or 2-SMD flat-lead outlines (none of `P600`, `DO-201AE`, `DO-218AB`,
`DO-219`, `SOD-882` in any id: `python3 -c "import json;ids=[f['id'] for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']];print([k for k in ['P600','DO-201AE','DO-218AB','DO-219','SOD-882'] if any(k in i for i in ids)])"` → `[]`).
In the Zener-type census (all makers) those packages hold P600 1,359,
DO-201AE 173, DO-218AB 256, DO-219AB 170, DO-219AA 83, SOD-882 195, 0201 355 rows
(`python3 -I planning/plan-69-data/census/show.py planning/plan-69-data/raw/tvsz.json 'Package / Case'`);
which of them the picked makers use is A's output (`footprints-needed.txt`).

## 3. Design

### 3.1 Ids, reuse, ready objects

- **Ids.** `DIODE-<outline>` for a generic JEDEC outline the maker's drawing
  matches (`DIODE-P600`, `DIODE-DO-201AE`, `DIODE-DO-218AB`); `DIODE-<outline>-<MAKER>`
  when the maker's land pattern differs from an existing one (`DIODE-SMB-LITTELFUSE`,
  `DIODE-SMA-VISHAY`); chip packages `DIODE-0201-<MAKER>`, `DIODE-0402-<MAKER>`;
  characters `A-Z 0-9 . -` only.
- **Reuse.** A drawing whose land pattern equals an existing footprint's (same pin
  count, every pad's designator, centre and size within 0.05 mm, same drill, body
  within 0.1 mm) reuses it; a datasheet with a mechanical drawing and no land
  pattern takes the IPC-7351 nominal pattern of its dimensions' mid-values, as the
  library's `note` fields record for earlier footprints.
- **Pin numbers are positions.** Pad 1 is the cathode-band end (B § 3.1); a
  bidirectional part uses the same pads (its pins `1` and `2`).
- **Shared ids.** Before adding an id: `grep -c '"id": *"<id>"'
  schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json`;
  a hit means reuse it (checked as any reuse), and the add step is marked moot with
  that grep's output.
- **Ready objects.** `footprints-C.json` is `{"footprints":[…]}` in
  `footprints.json`'s format; each object's `note` names the drawing, the PDF path
  under `datasheets/tvs/` and the page.
- **The map.** `footprint-map.tsv`: `letter`, `company`, `package`, `supplier`,
  `pdf`, `footprint`, `note` (`reuse` or `new`); one row per (line, land pattern).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Tools and the reuse pass

- [ ] Copy plan-48's `fp-check.py`, `fp-load.sh`, `fp-merge.py` to
      `planning/plan-69-data/census/tvs-fp-*.{py,sh}` with this plan's paths, the
      letter set `C`, `D`, and the build dir `/tmp/p69-fp.XXXXXX`.
- [ ] For every `C` line with an `existing` id: compare the datasheet's land pattern
      with that footprint per § 3.1; a match is a `reuse` row of `footprint-map.tsv`.

Acceptance: every reusable line is mapped.
  Check: `python3 -I planning/plan-69-data/census/tvs-fp-check.py C --reuse` → `<n> lines reused, <m> lines need a new footprint` (est. < 1 min).
Commit:

### Phase 2 — New footprints

- [ ] For every remaining `C` line: a new object in `footprints-C.json` from its
      drawing per § 3.1, after the shared-id grep; a `new` row in the map.
- [ ] `census/tvs-fp-load.sh planning/plan-69-data/footprints-C.json`.

Acceptance: every `C` line has a footprint and the new ones load.
  Check: `python3 -I planning/plan-69-data/census/tvs-fp-check.py C` → `0 without a footprint`; `census/tvs-fp-load.sh planning/plan-69-data/footprints-C.json` → only "extra footprint" / "unused footprint" lines, one per new id (est. 3 min: one `footprint_check` build).
Commit:

## Validation Plan

- Tests: `footprint_check` through `tvs-fp-load.sh` (the new objects load).
- Coverage check: `python3 -I -c "import json;print(len(json.load(open('planning/plan-69-data/footprints-C.json'))['footprints']))"` equals the map's `new` row count of letter C.
- Doc sync: none; each new outline's source is in its object's `note`.
- Final gate: plan-69-I.

## Decisions

- Author: **generic JEDEC ids first, a maker id only when the land pattern
  differs**, so the 40 existing `DIODE-*` outlines are reused where they fit.

## Corrections

## Summary

A drawing pass; the work is in comparing land patterns, so a reuse is never
assumed from a package name.
