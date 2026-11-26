# plan-71-E: Line transceivers — footprints for leaded packages (SOIC, DIP, SSOP, TSSOP, MSOP, SOT-23)

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-71-D

E gives every `E` (leaded) line of `footprints-needed.txt` a footprint. It
reuses an existing one when the datasheet's land pattern equals it, and
draws a new one from the datasheet's package drawing otherwise. It writes
the new objects to `planning/plan-71-data/footprints-E.json` and the
line → footprint map to `planning/plan-71-data/footprint-map.tsv`. H loads
them into `Parts/footprints.json`.

References:

- plan-71-D (the final `footprints-needed.txt`, `specs.tsv`).
- `docs/parts-format.md` § Footprints (`dual`, `explicit`, `centre`,
  `slots`, units and axes).
- `planning/completed/plan-48-C-timer-opamp-footprints-leaded.md` § 3.1
  (ids, reuse, shared ids, ready objects, the map, pin positions) and its
  Corrections 2 and 3 (the outline-only reuse rule, DIP bodies). E applies
  them unchanged. `planning/plan-48-data/census/fp-check.py`, `fp-load.sh`,
  `fp-merge.py`, `fp-show.py`, `fp-work.py`, `fp-collect.py` (the tools E
  copies).

## Prerequisites

plan-71-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-71-D complete | `ls planning/completed/plan-71-D-*.md` → exists, or every D box ticked | NOT MET (2026-11-25: D not started) |
| The final package lines exist | `awk -F'\t' '!/^#/ && $8==0' planning/plan-71-data/footprints-needed.txt \| wc -l` → 0 | NOT MET (written by D) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `E` line of `footprints-needed.txt` has a row in
  `footprint-map.tsv` naming a footprint that is in `footprints-E.json` or
  in `Parts/footprints.json`.
  `python3 -I planning/plan-71-data/census/fp-check.py E` → `<n> lines, 0 without a footprint`.
- `footprints-E.json` loads through `footprint_check` with no error.

### Non-goals (explicit constraints)

- No existing footprint changes: one is reused or a new one is added, never
  edited.
- `Parts/footprints.json` and the goldens are not touched here (H loads the
  objects).
- No footprint for a package no `add` row uses.

## 2. Current State

`footprints.json` has 3993 footprints (`wc -l < backend/tests/golden/footprints.txt`
→ 3993 at 63e2f7d2). 133 of them are SOIC, PDIP, SSOP, TSSOP, MSOP, VSSOP
or SOT-23 outlines (`cut -d'|' -f1 backend/tests/golden/footprints.txt | grep -cE '^(SOIC|PDIP|SSOP|TSSOP|MSOP|VSSOP|SOT23)'`
→ 133). These are the starting points of the reuse checks: `SOIC-8`,
`SOIC-14`, `SOIC-16`, `SOIC-16W`, `SOIC-20W`, `SOIC-24W`, `SOIC-28W`,
`PDIP-8`, `PDIP-14`, `PDIP-16`, `PDIP-20`, `SSOP-16`, `SSOP-20`,
`SSOP-24`, `SSOP-28`, `TSSOP-8` … `TSSOP-28`, `MSOP-8-ADI-MS8`,
`MSOP-10-ADI-MS`, `MSOP-8-EP-ADI-MS8E`, `MSOP-10-EP-ADI-MSE`, `VSSOP-8`,
`VSSOP-10`, `SOT23-6`, `SOT23-8-DCN`, `SOT23-8-ADI-RJ-8`.

At planning, the four protocols' DigiKey listings (all makers) show 70
leaded Package / Case values on 3,430 part listings
(`planning/plan-71-data/README.md` § The pick). The most common are 8-SOIC
3.9 mm (CAN 254, RS-485 680, LIN 55), 16-SOIC 3.9 / 7.5 mm, 28-SSOP 5.3 mm
(RS-232 153), 8-DIP (RS-485 141) and 14-SOIC. D's `footprints-needed.txt`
gives the lines of the final pick.

## 3. Design

plan-48-C § 3.1 and Corrections 2–3, unchanged, with this plan's paths:

- ids: `<outline>-<pins>-<TI designator>` or `<outline>-<pins>-<MAKER>[-<code>]`;
- reuse within 0.05 mm per pad and 0.1 mm body, on the same pin positions;
- outline-only drawings reuse the existing footprint of the same JEDEC
  outline (`note`: `no land pattern in the datasheet, same JEDEC outline`);
- before adding an id, `grep -c '"id": *"<id>"' schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json`
  (plans 69, 70, 72, 73 and 74 add footprints in parallel; a hit is reused);
- exposed pads: a numbered pad is a pin, an unnumbered one is `centre`.

Tools: `census/fp-check.py`, `fp-load.sh`, `fp-merge.py`, `fp-show.py`,
`fp-work.py`, `fp-collect.py`, copied from plan-48's with the paths and the
letter set (`E`, `F`) changed. `fp-load.sh` builds `footprint_check` in a
fresh `/tmp/p71-fp.XXXXXX`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Tools; SOIC and DIP

- [ ] The § 3 tools; `footprint-map.tsv` and `footprints-E.json` created.
- [ ] Every SOIC (3.9, 5.3 and 7.5 mm bodies, with and without exposed pad)
      and DIP line: reuse check or draw; map rows.

Acceptance: every SOIC and DIP line is mapped, and the new objects load.
  Check: `python3 -I planning/plan-71-data/census/fp-check.py E | grep -E 'SOIC|DIP' | grep -c missing` → 0; `planning/plan-71-data/census/fp-load.sh planning/plan-71-data/footprints-E.json` → `<n> new footprints: load ok` (est. 4 min: one `footprint_check` build).
  Result:
Commit:

### Phase 2 — SSOP, TSSOP, MSOP / VSSOP, SOT-23

- [ ] Every SSOP, TSSOP (with and without exposed pad), MSOP / VSSOP /
      µMAX and SOT-23 / TSOT-23 line: reuse check or draw; map rows.

Acceptance: every `E` line is mapped, and every new object loads.
  Check: `python3 -I planning/plan-71-data/census/fp-check.py E` → `<n> lines, 0 without a footprint`, `0 spec rows whose pin designators differ from their footprint`; `fp-load.sh footprints-E.json` → `load ok` (est. 4 min).
  Result:
Commit:

## Validation Plan

- `fp-check.py E` (every line mapped) and `fp-load.sh` (the loader's
  rules) on the ready objects.
- The new-object count goes in § Corrections, and H's footprint golden
  growth is compared with it.
- Final gate: plan-71-J.

## Decisions

- Author: plan-48-C's id, reuse and shared-id rules apply unchanged. They
  are the library's rules for every recent part plan, and a different rule
  would give two footprints for one outline.

## Corrections

(none yet)
