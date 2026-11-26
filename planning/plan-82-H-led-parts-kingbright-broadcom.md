# plan-82-H: `gen-led.py`; Kingbright and Broadcom parts; goldens

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-82-G

H writes `planning/plan-82-data/gen-led.py` and ships the Kingbright and Broadcom `add` rows as
parts in `schematical/schematical/Parts/led.json`, with their `datasheets.json` entries, and
regenerates the goldens.

References: plan-82-C § 3 (spec rules), plan-82-F § 3 (keys); `docs/parts-format.md` § Parts,
§ Datasheets, § `led`; `docs/backend.md` § goldens; `planning/plan-74-F-ethernet-parts.md` (a
generator letter's precedent).

## Prerequisites

plan-82-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-82-G complete | `ls planning/completed/plan-82-G-*.md` → exists, or every G box ticked | NOT MET (2026-11-25) |
| Goldens at H's start | `wc -l < backend/tests/golden/parts.txt` / `traces.txt` | H records P2 and R2 |

## 1. Goal

- `led.json` holds `led-generic` plus one device per distinct (maker, pin map, dies, colours) of
  the Kingbright and Broadcom `add` rows, and one part per row.
- `listing_check` lists P2 + (Kingbright + Broadcom `add` rows) parts; `ctest` passes.

### Non-goals

- `led-generic` / `GENERIC-LED` unchanged (its listing line byte-identical).
- SunLED and QTB parts are I's.

## 2. Current State

After G: the `led` kind loads `dies` and facts, the app shows them; `specs.tsv` is complete and
every footprint id exists (E's `specs-check.py all`).

## 3. Design

- Device id `led-<maker>-<n>` (`led-kb-…`, `led-av-…`), grouped by identical (pinout, dies or
  single colour); `function` = the colour(s) (`Red`, `Red / Green`, `RGB`); `description` = maker +
  package (`Kingbright 1608 chip LED`); `drawer` = `["Displays & Indicators","LED","Single color"]`,
  `"Bi-color"` (2 dies) or `"RGB"` (3–4 dies, RGBW included).
- A single-die device uses `"colors": [<colour>]`, `"anode"`, `"cathode"`; a multi-die one
  `"dies"`.
- Pinout names: `two-pin`, `p<N>` for other pin counts.
- Each part: `name`, `manufacturer` (`Kingbright`, `Broadcom`), `datasheet` (the PDF's stem, entry
  added to `datasheets.json`), `package` (the maker's package name), `pinout`, `footprint`, and
  the fact keys present in `specs.tsv`.
- `gen-led.py` replaces only the devices whose id starts `led-kb-` / `led-av-` (rerunnable), keeps
  the file's other devices in place, and writes stable order (device id, then part name).

## 4. Phases

### Phase 1: the generator, Kingbright

- [ ] `gen-led.py`; run for Kingbright; `datasheets.json`.

Acceptance: `ctest --test-dir /tmp/logic-p82 -R library_check` passes; `python3 -I
planning/plan-82-data/gen-led.py --check kingbright` → `parts N = add rows N` — 3 min.

Commit:

### Phase 2: Broadcom; goldens

- [ ] Run for Broadcom; regenerate `parts.txt`, `traces.txt`, `footprints.txt`
      (`docs/backend.md` § goldens) and review the diff: only added `led` lines.

Acceptance: `git diff backend/tests/golden/parts.txt | grep '^-[^-]' | wc -l` → 0;
`ctest --test-dir /tmp/logic-p82` → `100% tests passed` — 3 min.

Commit:

## 5. Validation Plan

H's gate: Phase 2's full `ctest` and the no-removed-lines golden diff. The landing tier is J's.
