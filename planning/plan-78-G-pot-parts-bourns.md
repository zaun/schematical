# plan-78-G: Potentiometer parts: `gen-pot.py`, Bourns, footprints merged, goldens

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-78-F

G writes `planning/plan-78-data/census/gen-pot.py`, which turns `specs.tsv` and `verdicts.tsv` into
potentiometer devices in `schematical/schematical/Parts/resistor.json`, runs it for Bourns, merges
C's footprints, adds the datasheet entries, and regenerates the goldens. After G the library lists
every Bourns `add` row.

References:

- plan-78-B § 3.4 (`specs.tsv` columns), plan-78-E § 3.1 (keys), plan-78-C (footprint ids).
- `docs/parts-format.md` § Devices, § Parts, § Datasheets, § `resistor`; `docs/backend.md`
  § goldens (line 898: `listing_check --print`, `trace_check --print`, `footprint_check --print`).
- `planning/completed/plan-48-G-timer-opamp-parts.md` (the generator precedent: idempotent,
  `--check`); `planning/completed/plan-34-*` (`resistor.json`'s device grouping).
- `planning/plan-78-data/census/fp-merge.py` (C).

## Prerequisites

plan-78-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-78-F complete | `ls planning/completed/plan-78-F-*.md` → exists, or every F box ticked | NOT MET (2026-11-25: F not started) |
| No pot device yet | `grep -c '"wipers"' schematical/schematical/Parts/resistor.json` → 0 | MET (2026-11-25: 0) |
| Goldens at G's start | `wc -l < backend/tests/golden/parts.txt` / `traces.txt` / `footprints.txt` / `erc_pins.txt` | G records P1, R1, F1, E1 |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `resistor.json` gains one device per Bourns (series, layout) row of `specs.tsv`, holding one part
  per Bourns `add` row; the 56 fixed-resistor devices and their 1975 parts are unchanged.
- Every Bourns footprint of `footprints-bourns.json` is in `footprints.json`, each used by a part.
- `datasheets.json` has one entry per Bourns PDF; `ls datasheets/potentiometer/bourns/*.pdf | wc -l`
  equals the Bourns entries.
- Goldens: `parts.txt` grows by exactly the Bourns `add` count (each new line carries
  ` wipers=… pot=… taper=… turns=…` after `power=` and ends `|footprint=POT-BOURNS-…`); `footprints.txt` by the Bourns
  footprint count; `erc_pins.txt` by one line per new part; `traces.txt` grows by the same count
  as `parts.txt` (one trace line per part) and no existing line of any golden changes.

### Non-goals (explicit constraints)

- No change to the backend or the app; no new key.
- No other maker's parts (H).

## 2. Current State

`resistor.json` (plan-34): 56 devices, 1975 parts, grouped one device per (category, composition,
pinout, ohms group), drawer **Discrete → Resistors → Chip / Through-hole / Array**. Datasheet ids
are `<maker>-<file name in lower case, non-alphanumerics to ->` (`yageo-pyu-ac-51-rohs-l`).

## 3. Design

### 3.1 Devices (`gen-pot.py`)

One device per `specs.tsv` row:

- `id`: `pot-<maker>-<series>[-<layout>]` in lower case (`pot-bourns-3296-w`).
- `function`: the series and layout as the PDF names them (`3296W`); `description`: maker, size
  and type from the PDF title (`Bourns 3296 3/8" square multiturn trimmer, top adjust`).
- `drawer`: `["Discrete", "Potentiometers", "<Rotary|Trimmer|Slide|Thumbwheel>"]` from `category`.
- `pinouts`: one pinout, `pot<pins>` (`pot3`, `pot5` with two mounting tabs, `pot6` dual), pins in
  `pin_names` order: wiper pins `inout`, mounting tabs `nc`.
- `wipers`: from `specs.tsv`'s `wipers`; `pot`: the `category`.
- `parts`: one per `add` row of the series and layout, in part-number order: `name`,
  `manufacturer` `Bourns`, `datasheet` (the id of § 2's rule), `package` (`Through hole` or
  `Surface mount`, from `termination`), `pinout`, `footprint`, `ohms` (DigiKey's value, checked
  inside `specs.tsv`'s range), `tolerance` (percent), `power` (W), `taper` (omitted when
  `linear`), `turns` (omitted when 1), `flags: ["obsolete"]` where `specs.tsv` says so.
- Idempotent: rerunning replaces every `pot-<maker>-` device of the given maker and nothing else.
  `gen-pot.py --check <maker>` exits 0 when `resistor.json` already equals a fresh run.

### 3.2 Merge and datasheets

- `fp-merge.py schematical/schematical/Parts/footprints.json planning/plan-78-data/footprints-bourns.json`.
- `datasheets.json`: one `{"id", "url"}` per Bourns PDF of `datasheets/potentiometer/datasheets.tsv`,
  kept in the file's sort order.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Generator and Bourns parts

- [ ] `census/gen-pot.py` per § 3.1 (with `--check`).
- [ ] Run it for `bourns`; merge `footprints-bourns.json`; add the datasheet entries.

Acceptance: the library loads with every Bourns `add` row as a part.
  Check: `cmake --build /tmp/logic-p78 -j8 && /tmp/logic-p78/listing_check --print | grep -c '^[^|]*|resistor|.*pot='` → the Bourns `add` count (`awk -F'\t' '$2=="bourns" && $3=="add"' planning/plan-78-data/verdicts.tsv | wc -l`, the column layout of B's `verdicts.tsv`: `part maker verdict reason`) (est. 3 min).
  Result:
Commit:

### Phase 2 — Goldens

- [ ] `listing_check --print > backend/tests/golden/parts.txt`, `trace_check --print > traces.txt`,
      `footprint_check --print > footprints.txt`, the ERC golden (`docs/backend.md` line 916); review
      `git diff --stat backend/tests/golden` and hand-check five changed footprint lines against
      their drawings.

Acceptance: the goldens grow by exactly the new parts and footprints, and no old line changes.
  Check: `git diff -U0 backend/tests/golden | grep -c '^-[^-]'` → 0; `ctest --test-dir /tmp/logic-p78 -R 'resistor_test|listing_check|trace_check|footprint_check|erc' --output-on-failure` → all pass (est. 3 min).
  Result:
Commit:

## Validation Plan

- Per-phase checks above. Final gate: plan-78-I.

## Decisions

- Author: **one device per (series, layout)**, not per ohms group as for chip resistors: a pot
  series shares one drawing, one footprint and one pinout, which is what a device groups; the
  drawer's third level is the pot type.
- Author: **`taper` and `turns` omitted at their defaults**, keeping each part line short and the
  file diff small.

## Corrections

## Summary

A generator turns B's specs into Bourns pot devices in `resistor.json`, with their footprints and
datasheets, and the goldens grow by exactly the new parts.
