# plan-70-E: Footprints: optocoupler packages

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-70-D

E gives every `E` line of `footprints-needed.txt` a footprint. A line
reuses an existing footprint when the datasheet's land pattern equals it,
and otherwise gets a new one drawn from the datasheet's package drawing.
The `E` lines are the transistor-, triac- and logic-output optocouplers. E
writes the new objects to `planning/plan-70-data/footprints-E.json` and the
line → footprint map to `planning/plan-70-data/footprint-map.tsv`. H loads
them into `Parts/footprints.json` with the parts that use them.

References:

- plan-70-D (`footprints-needed.txt`, `specs.tsv`).
- `docs/parts-format.md` § Footprints (`dual`, `explicit`, `pin1Shape`,
  `drill`, units and axes).
- `planning/completed/plan-48-C-timer-opamp-footprints-leaded.md` § 3.1
  (ids, reuse, shared ids, ready objects, pin numbers are positions) and its
  Corrections 2 and 3 (outline-only datasheets, DIP bodies). E applies them
  unchanged. `planning/plan-48-data/census/fp-check.py`, `fp-load.sh`,
  `fp-merge.py`, `fp-show.py`, `fp-work.py`, `fp-collect.py` are the tools
  E copies.

## Prerequisites

plan-70-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-70-D complete | `ls planning/completed/plan-70-D-*.md` → exists, or every D box ticked | NOT MET (2026-11-25: D not started) |
| The final package lines exist | `awk -F'\t' '!/^#/ && $1=="E"' planning/plan-70-data/footprints-needed.txt \| wc -l` → at least 1, and `awk -F'\t' '!/^#/ && $9==0' planning/plan-70-data/footprints-needed.txt \| wc -l` → 0 | NOT MET (2026-11-25: no file) |

Everything below is written against the world where these hold.

> **NOTE: the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `E` line has a row in `footprint-map.tsv` naming a footprint in
  `footprints-E.json` or in `Parts/footprints.json`.
  `python3 -I planning/plan-70-data/census/fp-check.py E` →
  `<n> lines, 0 without a footprint, 0 spec rows whose pin designators differ`.
- `footprints-E.json` loads through `footprint_check` with no error
  (`fp-load.sh footprints-E.json` → `<k> new footprints: load ok`).

### Non-goals (explicit constraints)

- No existing footprint changes. One is reused or a new one is added,
  never edited.
- `Parts/footprints.json` and the goldens are not touched (H loads the
  objects).
- No footprint for a package no `add` row uses.

## 2. Current State

`footprints.json` has 3993 footprints (`wc -l < backend/tests/golden/footprints.txt`
→ 3993). The ones an optocoupler line starts its reuse check from
(`python3 -I` over `footprints.json`, 2026-11-25):

| Id | Pins, pitch, span (mm) | Body (mm) | Drill |
|---|---|---|---|
| `RELAY-DIP-6-PANASONIC` | 6, 2.54, 7.62 | 6.4 × 8.8 | 0.8 |
| `RELAY-SMD-6-PANASONIC` | 6, 2.54, 8.3 | 6.4 × 8.8 | – |
| `RELAY-SMD-4-PANASONIC` | 4, 2.54, 8.3 | 6.4 × 4.78 | – |
| `RELAY-SOP-4-PANASONIC` | 4, 2.54, 6.0 | 4.4 × 4.3 | – |
| `RELAY-SOP-6-PANASONIC` | 6, 2.54, 6.0 | 4.4 × 6.3 | – |
| `PDIP-8` | 8, 2.54, 7.62 | 6.35 × 9.59 | 0.75 |
| `PDIP-16` | 16, 2.54, 7.62 | 6.35 × 19.3 | 0.75 |
| `SOIC-4` | 4, 2.54, 6.5 | 3.9 × 4.725 | – |
| `SOIC-8`, `SOIC-16` | 8 / 16, 1.27, 5.4 | 3.9 × 4.9 / 9.91 | – |

The 4-pin, 2.54 mm dual footprints are `SW-DIP-4` (drill 0.85), `SW-SMD-4`,
`OSC-ECS-5032-4-MV`, `SOIC-4`, `OSC-MICROCHIP-5032-4-H6A`,
`RELAY-SMD-4-PANASONIC` and `RELAY-SOP-4-PANASONIC`. That is the full list
(`python3 -I` over `footprints.json`, `style` dual, `pins` 4, `pitch` 2.54).
The only through-hole one is `SW-DIP-4`, a TE DIP switch with 7.62 mm rows.
Its body is a switch's, outside plan-48-C's 0.1 mm body rule, so the 4-pin
DIP opto outline (PC817 / LTV-817 / VO617A) is drawn new. The wide-lead (10.16
mm, option `W` / `X` / `T`) and the long-creepage packages (Toshiba SO6L,
LSO, 11-4L1) have no existing footprint either.

## 3. Design

- **Ids, reuse, shared ids, ready objects, the map:** plan-48-C § 3.1,
  unchanged (`<outline>-<pins>-<MAKER>[-<code>]`, e.g. `DIP-4-VISHAY`,
  `SMD-6-ONSEMI-TYPE-S`, `SO6L-6-TOSHIBA-11-4L1`). Footprint `note`s name
  the drawing, the PDF path and the page.
- **Gull-wing and wide-lead options** of one body are separate footprints
  (their pads differ). A line whose datasheet prints several lead options
  maps each supplier package to its own footprint. The `supplier` column of
  the map carries DigiKey's supplier package.
- **Through-hole pin 1** gets `"pin1Shape": "rect"`, as the library's PDIP
  footprints do.
- **Würth's per-part PDFs print a "Recommended Land Pattern"** (B § 2):
  that pattern is the footprint, not IPC nominal.
- **A 4-pin part on a 6-pin outline** (some SOP-4 parts number 1, 3, 4, 6)
  takes the datasheet's numbering. The footprint has `pins` 4 and `slots`
  placing pins on the 6-slot outline (`docs/parts-format.md` § `slots`).

## Phases

> **NOTE: keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1: Tools; through-hole DIP lines

- [ ] `census/fp-check.py`, `fp-load.sh`, `fp-merge.py`, `fp-show.py`,
      `fp-work.py`, `fp-collect.py` (from plan-48's, letters `E`, `F`);
      `footprint-map.tsv` and `footprints-E.json` created.
- [ ] Every 4-, 6-, 8- and 16-pin through-hole DIP line (7.62 mm and 10.16
      mm rows), per maker: reuse check (`RELAY-DIP-6-PANASONIC`, `PDIP-8`,
      `PDIP-16`) or draw; map rows.

Acceptance: every through-hole `E` line is mapped and the new objects load.
  Check: `fp-check.py E --mounting through-hole` → `0 without a footprint`;
  `fp-load.sh planning/plan-70-data/footprints-E.json` → `load ok`. Takes
  about 4 min (one `footprint_check` build).
Commit:

### Phase 2: Surface-mount lines

- [ ] Every gull-wing SMD (7.62 / 10.16 mm), SOP-4 / SOP-5 / SOP-6 /
      SO-8 / SO-16, stretched SO and LSOP / SO6L line, per maker: reuse
      check (`RELAY-SMD-4/6-PANASONIC`, `RELAY-SOP-4/6-PANASONIC`, `SOIC-4`,
      `SOIC-8`, `SOIC-16`) or draw; map rows.

Acceptance: every `E` line is mapped; every new object loads.
  Check: `python3 -I planning/plan-70-data/census/fp-check.py E` →
  `<n> lines, 0 without a footprint, 0 spec rows whose pin designators differ`;
  `fp-load.sh planning/plan-70-data/footprints-E.json` → `<k> new
  footprints: load ok`. Takes about 4 min.
Commit:

## Validation Plan

- Tests: `fp-check.py`, `fp-load.sh` on the ready objects.
- Coverage check: the count of `footprints-E.json` objects is recorded in
  § Corrections; H's golden growth is compared with it.
- Doc sync: none (no new footprint style).
- Final gate: plan-70-J.

## Decisions

- Author: **plan-48-C's id, reuse and shared-id rules, and its Corrections
  2 and 3**, apply unchanged.
- Author: **the Panasonic PhotoMOS relay patterns are reuse candidates.**
  They are the same JEDEC-style 2.54 mm opto outlines, and a reuse still
  needs pads within 0.05 mm.

## Corrections

## Summary

Mostly drawing work: few optocoupler outlines exist in the library, and
each maker's lead options differ. The checks that catch mistakes are pin-1
position and `slots` on 4-on-6 outlines.
