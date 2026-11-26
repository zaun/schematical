# plan-78-C: Potentiometer footprints: Bourns

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-78-B

C draws the land pattern of every Bourns id in `planning/plan-78-data/footprints-needed.txt`, from
the drawing page `specs.tsv` names, into `planning/plan-78-data/footprints-bourns.json`, and proves
they load. G merges them. Bourns is its own letter because it is the largest maker (3,252 in-stock
parts across the four categories at planning, capped at 300) and spans trimmers (3296, 3362,
3386, 3306, 3314, …), rotary pots (PTV09, PTV111, PDB18, …), slide pots (PTA, PTB, …) and
thumbwheels: the other three makers together go in D.

References:

- plan-78-B § 3.4 (`specs.tsv`, footprint ids, `drawing_page`); `docs/parts-format.md` § Footprints
  (§ Common keys, § `dual`, § `explicit`: slotted holes `slotLength`, unnumbered pads `"pin": 0`,
  non-plated holes `"plated": false`).
- `planning/plan-58-data/census/fp-load.sh`, `fp-merge.py`, `fp-render.py` (copied to
  `planning/plan-78-data/census/`, scratch `/tmp/p78-fp.XXXXXX`).
- Precedents: `planning/completed/plan-58-C-slide-switch-parts-ck.md` (switch footprints from
  drawings: through-hole pads plus slotted mounting legs); `planning/completed/plan-45-E-jst-footprints.md`.

## Prerequisites

plan-78-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-78-B complete | `ls planning/completed/plan-78-B-*.md` → exists, or every B box ticked | NOT MET (2026-11-25: B not started) |
| No pot footprint exists | `python3 -I -c "import json;print(sum(f['id'].startswith('POT-') for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` → 0 | MET (2026-11-25: 0) |
| Shared footprint ids are added once | before adding an id: `grep -c '"id": *"<id>"' schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json` → 0 | a rule, applied per footprint; `fp-merge.py` skips an id already present and prints `present <id>` |
| Footprint golden at C's start | `wc -l < backend/tests/golden/footprints.txt` | 3993 at planning (F0). C records its own F1 |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `footprints-bourns.json` holds one footprint per Bourns id of `footprints-needed.txt`
  (`grep -c '^POT-BOURNS-' planning/plan-78-data/footprints-needed.txt`), in the
  `schematical-footprints` format, each with a `note` naming its PDF and page.
- Every entry loads through `footprint_check` on a scratch copy of the tree: the only messages are
  `extra footprint` and `unused footprint` for exactly the new ids, with no load error and no
  overlapping pads.
- `footprints.json` and the goldens do not change in C.

### Non-goals (explicit constraints)

- No part is added and nothing is merged (G).
- No new footprint style or key: `explicit` (in-line rows, pin triangles, mixed pad sizes, slotted
  mounting tabs, board-lock holes) covers every pot drawing.

## 2. Current State

`footprints.json` has 3993 footprints (F0) and none for a pot. The closest precedents are the slide
switch footprints of plan-58 (`SW-…`: through-hole pins plus slotted, unnumbered mounting legs) and
the USB receptacles' shell legs (`docs/parts-format.md` § `explicit`, the `slotLength` example).

## 3. Design

- **Style.** `explicit` for every pot: each footprint is the drawing's pad list. `dual` places
  pins in two facing columns (`RES-0402`: one pin per side), so a 3-pin in-line row or a pin
  triangle does not fit it.
- **Pins and pads.** Pad `pin` numbers are the drawing's terminal numbers (1, 2 wiper, 3; a dual
  gang 1–6), in the order of `specs.tsv`'s `pin_names`. Mounting tabs and board locks that the
  drawing numbers are numbered pins with role `nc` in the part (`MT1`, `MT2`); those it does not
  number are `"pin": 0` pads. Non-plated locating holes are `"pin": 0`, `"plated": false`.
- **Geometry.** Pads, drills and slots from the drawing's recommended PCB layout. Where the PDF
  prints only the part's dimension drawing: pin positions from it, drill = the lead's maximum
  width (or diameter) + 0.2 mm, pad = drill + 1.0 mm round (0.5 mm annular ring), a rectangular
  lead's slot `slotLength` = the lead's maximum length + 0.2 mm (plan-78-B § Decisions). The
  `note` says `PCB layout` or `from dimensions (IPC-2222 B)`.
- **Origin and orientation.** The origin is the centre of the body outline; x right, y down,
  viewed from the component side with pin 1 to the left of the bottom row (the drawing's own view;
  a drawing seen from the solder side is mirrored and the `note` says `mirrored from bottom view`).
- **Body.** `body` = the body outline's width and height (the knob's projection excluded).
- **Check renders.** `fp-render.py <id>` draws each footprint over its drawing page
  (`pdftoppm -r 150 -f <page> -l <page>`) into `/tmp/p78-render/<id>.png` for a visual check.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Tools

- [ ] Copy `fp-load.sh`, `fp-merge.py`, `fp-render.py` from `planning/plan-58-data/census/` to
      `planning/plan-78-data/census/`, with the scratch prefix `/tmp/p78-fp.` and the `present <id>`
      skip in `fp-merge.py` (plan-74-D's change).

Acceptance: the tools load an empty file list cleanly.
  Check: `printf '[]' > /tmp/p78-empty.json && planning/plan-78-data/census/fp-load.sh /tmp/p78-empty.json` → exit 0, no output lines (est. 4 min: a scratch build of `footprint_check`).
  Result:
Commit:

### Phase 2 — Trimmers

- [ ] An entry in `footprints-bourns.json` for every Bourns trimmer id (`specs.tsv` rows with
      `maker` = `bourns` and `category` = `trimmer`).
- [ ] A render of each against its drawing page, looked at.

Acceptance: every trimmer entry loads, and its pad count and pin spacing match its drawing.
  Check: `planning/plan-78-data/census/fp-load.sh planning/plan-78-data/footprints-bourns.json` → exit 0, only `extra footprint` / `unused footprint` lines for the new trimmer ids (est. 4 min).
  Result:
Commit:

### Phase 3 — Rotary, slide and thumbwheel

- [ ] Entries for every other Bourns id.
- [ ] Every `POT-BOURNS-` id of `footprints-needed.txt` is in `footprints-bourns.json`.

Acceptance: the whole file loads.
  Check: `planning/plan-78-data/census/fp-load.sh planning/plan-78-data/footprints-bourns.json` → exit 0, only `extra footprint` / `unused footprint` lines, one pair per `POT-BOURNS-` id (`grep -c '^POT-BOURNS-' planning/plan-78-data/footprints-needed.txt`) (est. 4 min).
  Result:
Commit:

## Validation Plan

- Per-phase checks above. Final gate: plan-78-I.

## Decisions

- Author: **`explicit` for every pot footprint**: a pot is one row or a triangle of pins plus
  mounting pads of other sizes, which no parametric style expresses.
- Author: **numbered mounting tabs stay pins (`nc`), unnumbered ones are `"pin": 0`**: the part's
  pins follow the drawing's numbering, and the resistor kind allows `nc` pins.

## Corrections

## Summary

The land patterns of every Bourns pot series in scope, drawn from Bourns's drawings and checked to
load, ready for G to merge.
