# plan-74-G: MagJacks, Bel Fuse: pinouts, footprints, gen-rj45.py, parts, goldens

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-74-F

G adds Bel Fuse's in-scope RJ45 jacks with magnetics (at most 185, plan-74-A § 2) to
`Parts/connector.json`, under the `"rj45"` mode of plan-74-E. Each drawing is read: its schematic
gives the pins, and its recommended PCB layout gives the footprint. G also writes the generator,
`gen-rj45.py`, that H (Pulse) and I (TE) extend.

References:

- plan-74-E § 3.2 (`"rj45"`, `"magnetics"`, `"poe"`); plan-74-A § 3.4 (`pending` rows),
  § Decisions.
- `docs/parts-format.md` § `connector` (USB connectors: `SHIELD` as one last pin with a pad per
  leg; origin and orientation of an `explicit` connector footprint), § `explicit`.
- Precedents: the USB connector footprints (`USB-…`, explicit, from each drawing's recommended
  layout, every number in the `note`); `planning/plan-45-data/census/gen-parts.py` (an idempotent
  generator that owns its id prefix in `connector.json`).

## Prerequisites

plan-74-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-74-F complete | `ls planning/completed/plan-74-F-*.md` → exists, or every F box ticked | NOT MET (2026-11-25: F not started) |
| No `rj45-` device yet | `grep -c '"id": "rj45-' schematical/schematical/Parts/connector.json` → 0 | MET (2026-11-25: 0) |
| Goldens at G's start | `wc -l < backend/tests/golden/parts.txt` / `footprints.txt`; `grep -c '\|connector\|' backend/tests/golden/parts.txt` | G records its own (C0 = 1541 connector parts at planning) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every Bel `add` row and every Bel `pending` row whose drawing confirms the part number (title
  block, part-number pattern) is one part in `connector.json`, with its drawing's pins and
  footprint. A `pending` row whose drawing does not confirm it becomes `skip` (`not in PDF`), in
  `verdicts.tsv` and `datasheets/skipped-parts.md`.
- `planning/plan-74-data/footprints-rj45-bel.json` holds one footprint per distinct PCB layout, merged
  into `footprints.json`.
- `connector.json`'s other blocks are byte for byte unchanged (`gen-rj45.py` owns only the
  `rj45-` ids).
- The goldens grow by exactly the new parts and footprints.

### Non-goals (explicit constraints)

- No LED model: LED pins are terminals (plan-74-E § Non-goals).
- No board cut-out and no keep-out: a jack's footprint is its pads and holes.
- No Pulse or TE part (H, I).

## 2. Current State

`connector.json` has 567 device ids (C0 = 1541 parts) and no `rj45-` device. A Bel drawing is an
image-only PDF (plan-74-A § 2: `pdftotext … | wc -w` → 8 for `dr-mag-0826-1x1t-23-f.pdf`). Its
first page, read at planning, shows: a schematic with pins 1–12 named `TRD1+ 11`, `TRCT1 12`,
`TRD1- 10`, `TRD2+ 4`, `TRCT2 6`, `TRD2- 5`, `TRD3+ 3`, `TRCT3 1`, `TRD3- 2`, `TRD4+ 8`,
`TRCT4 7`, `TRD4- 9`; LED1 on pins 13/14 and LED2 on 15/16, with a polarity table; `SHIELD`. There
are five pages in all. Bel's file names put `x` where the LED-option letter goes
(`0826-1X1T-23-F` covers `-1C1T-`, `-1A1T-`, …).

## 3. Design

### 3.1 Pins (the rule for G, H and I)

- One pin per numbered contact of the drawing's board side, numbered as the drawing numbers them
  (Bel 1…16; TE P1…Pn are numbered 1…n; Pulse's numbers as printed).
- Name: the drawing's own label for the PHY-side pin (`TRD1+`, `TRCT1`, `TD+`, `TCT`, `RD+`,
  `RCT`, …). An unlabeled contact the drawing ties to no winding is `NC`, role `nc`.
- LED pins: `LED<n>A` / `LED<n>K` (anode, cathode), `n` the drawing's LED number (Bel LED1 / LED2;
  left = 1 where the drawing says left / right). A bicolour LED's two pins are named by its first
  colour's polarity in the drawing's table; the `note` gives both colours. LED pins take
  `"group": "led"`.
- Shield: one last pin `SHIELD`, with a pad per shield tab (the USB rule).
- Every other pin is `inout`. Groups: `mdi` for the pairs and centre taps, `led`, `shield`.

### 3.2 Footprints (`RJ45-BEL-<drawing>`)

`explicit`, from the drawing's recommended PCB layout:
- the origin is the centre of the housing's top-view outline;
- a right-angle jack's opening points up (−y), as for USB;
- plated round pads for the contacts and LED leads, the drill the drawing's hole diameter;
- the plastic locating posts are non-plated holes (`"pin": 0`, `"plated": false`);
- the shield tabs are plated pads on `SHIELD`.

The pad is the drill + 0.5 mm, unless the drawing gives a pad size. The `note` names the PDF, page
and every number used. Several Bel part numbers share a drawing, and so a footprint. The id is the
drawing's series (`RJ45-BEL-0826-1X1T-23`).

### 3.3 Parts (`census/gen-rj45.py`)

The generator reads `specs-rj45.tsv` plus one table file per maker,
`planning/plan-74-data/rj45-<maker>.json`: drawing → pins, LED colours, footprint id. It writes one
device per (maker, drawing):

- `id`: `rj45-<maker>-<drawing series>` in lower case;
- `function`: `RJ45 jack, magnetics`, or `RJ45 jack, magnetics, PoE`;
- `description`: `10/100 RJ45 jack with magnetics, LEDs` (and the like);
- `drawer`: `["Connector", "RJ45"]`;
- `rj45`, `magnetics: true`, `poe`, `pinouts`, `note`.

Each part has `name`, `manufacturer` (`Bel Fuse`, `Pulse Electronics`, `TE Connectivity`),
`package` (`Through hole right angle` / `Through hole vertical`), `footprint` and `datasheet`. The
generator drops and rewrites only the blocks whose id starts `rj45-` (the plan-45 rule). `--check`
fails when the file differs.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Drawings read

- [ ] Render each Bel drawing (`pdftoppm -r 150`), and decide every Bel `pending` row (title block
      and part-number pattern): `add`, or `skip` with its `skipped-parts.md` line.
- [ ] `planning/plan-74-data/rj45-bel.json`: pins per § 3.1 and LED colours, per drawing; the
      `specs-rj45.tsv` Bel rows filled.
- [ ] `planning/plan-74-data/footprints-rj45-bel.json` per § 3.2.

Acceptance: every Bel row is decided, and every footprint loads.
  Check: `awk -F'\t' 'NR>1 && $2=="bel" && $18=="pending"' planning/plan-74-data/candidates.tsv | wc -l` → 0; `planning/plan-74-data/census/fp-load.sh planning/plan-74-data/footprints-rj45-bel.json` → exit 0, only `extra footprint` / `unused footprint` lines for the new ids (est. 5 min).
  Result:
Commit:

### Phase 2 — Generator, parts, goldens

- [ ] `planning/plan-74-data/census/gen-rj45.py` per § 3.3; write the Bel devices.
- [ ] `fp-merge.py` with `footprints-rj45-bel.json`; `datasheets.json` entries; regenerate the
      goldens, and review: only added lines.

Acceptance: `connector.json` holds exactly the Bel `add` rows as `rj45-bel-` parts, and the suite
passes.
  Check: `cmake --build /tmp/logic-p74 -j8 && ctest --test-dir /tmp/logic-p74 -j8` → `100% tests passed`; `grep -c '|connector|.*rj45=' backend/tests/golden/parts.txt` equals `awk -F'\t' 'NR>1 && $2=="bel" && $18=="add"' planning/plan-74-data/candidates.tsv | wc -l`; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0; `python3 -I planning/plan-74-data/census/gen-rj45.py --check` → exit 0 (est. 6 min).
  Result:
Commit:

## Validation Plan

- Per-phase checks. A hand check of three footprints (one right angle with LEDs, one without,
  one vertical) against their drawings at 1:1, recorded in Phase 1's Result.
- Final gate: plan-74-K.

## Decisions

- Author: **pins are named by the drawing's PHY-side labels**, not by a fixed RJ45 scheme. The
  labels differ by maker and speed (`TD+` / `TRD1+`), and a designer wires the jack from its
  drawing.
- Author: **LED pins are `LED<n>A` / `LED<n>K`.** Drawings mark LED pins by number and polarity
  only, and anode / cathode is what the board needs.

## Corrections

## Summary

Bel's jacks, read from their image-only drawings, with the pin and footprint rules and the generator
that H and I reuse.
