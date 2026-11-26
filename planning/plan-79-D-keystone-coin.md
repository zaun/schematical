# plan-79-D: Keystone coin cell holders, retainers and contacts; the footprint and part generators

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-79-C

D adds Keystone's coin cell parts (DigiKey functions "Coin Cell, Holder", "Coin Cell, Retainer",
"Coin Cell, Contact": the `in` rows of `candidates.tsv` with those functions, at most the 300
Keystone rows of plan-79-A's cap) with a footprint each, and writes the two tools E–H reuse:
`merge-fp.py` (footprints) and `gen-battery.py` (parts). It fixes the footprint rules every later
letter follows (§ 3.2).

References:

- `docs/parts-format.md` § Footprints (`explicit`, slots, unnumbered pads, non-plated holes),
  § Terminal-block and JST footprints (rule R5-T for a through-hole pad), § `connector` (the
  battery paragraph C adds), § Datasheets.
- `planning/plan-79-data/README.md`; `specs.tsv`, `drawings.tsv`, `verdicts.tsv` (A, B).
- `planning/plan-45-data/census/gen-parts.py` (the idempotent block-rewrite precedent: it drops and
  rewrites the blocks whose ids start with its prefixes and leaves every other block byte for byte).
- `docs/backend.md` § tests (regenerating `parts.txt`, `traces.txt`, `erc_pins.txt`,
  `footprints.txt`).

## Prerequisites

plan-79-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-79-C complete: the `"battery"` mode loads | `ls planning/completed/plan-79-C-*.md` → exists; `grep -c '"battery"' backend/src/models/connector.c` → above 0 | NOT MET (2026-11-25: 0) |
| Goldens at D's start | `wc -l < backend/tests/golden/parts.txt`, `… footprints.txt` | P0 = 40832, F0 = 3993 at planning; D records its own P1 and F1 |
| No `BAT-` footprint and no `battery-` device yet | `python3 -I -c "import json;print(sum(f['id'].startswith('BAT-') for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` → 0; `grep -c '"id": *"battery-' schematical/schematical/Parts/connector.json` → 0 | MET (2026-11-25: 0; 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every Keystone coin row of `verdicts.tsv` is `add` (with a footprint) or `skip` (with a reason,
  in `datasheets/skipped-parts.md` § "Battery holders (plan-79)").
- `Parts/footprints.json` has one `BAT-KEY-…` footprint per distinct land pattern of those parts.
- `Parts/connector.json` has the `battery-keystone-…` devices and their parts, written by
  `gen-battery.py`; `datasheets.json` has an entry per cited PDF.
- `listing_check`, `trace_check`, `erc_pins_check` and `footprint_check` pass with goldens grown by
  exactly the new parts and footprints, and no other line changed.

### Non-goals

- No change to `backend/` or the app. No other maker's or function's parts (E–H).

## 2. Current State

No `BAT-` footprint, no battery part (plan-79-A § Prerequisites). Keystone's product PDFs are
catalog pages; their PCB layouts are on the pages B fetched (`specs.tsv` `layout_page`).

## 3. Design

### 3.1 `specs.tsv` rows (filled here for Keystone coin)

For each row: `pins` (the pin count), `pin_names` (`+,-`, or one of `+`, `-`, `LINK` for a contact,
per plan-79-C § 3.3), `footprint` (the id), `note` (drawing file, page, every number read). A row
whose drawing gives neither a PCB layout nor the tab / pin dimensions is `skip` (`no layout`) and
promotes a reserve row (plan-79-A § 3.3).

### 3.2 Footprint rules (every maker, D–H)

- **Id:** `BAT-<maker>-<series or part>` with maker `KEY`, `MPD`, `ADAM`, `ASM`
  (`BAT-KEY-3000`, `BAT-MPD-BU2032SM`), upper case, `A-Z 0-9 . -`. Parts with one land pattern share
  one footprint; a variant that differs only in plating or packaging adds none.
- **Style:** `explicit`. Origin: the centre of the holder's top-view outline (the cell's centre for
  a coin holder); x right, y down, as the maker's top view shows it. A layout figure drawn from the
  solder side is mirrored left to right (plan-45's JST rule).
- **Pads from the maker's layout figure** (the recommended PCB layout), each number in the `note`.
- **A through-hole pin with no layout figure:** a round pad, rule R5-T (`docs/parts-format.md`):
  drill = the drawing's hole diameter (middle of min / max), or the pin's diagonal + 0.2 mm when the
  drawing gives only the pin; pad = drill + 0.7 mm. A flat tab gets a slot (`slotLength` = tab width
  + 0.2 mm, `drill` = tab thickness + 0.2 mm) on a pad 0.7 mm larger each way.
- **A surface-mount tab with no layout figure:** a rect pad of the tab's width + 0.4 mm and its
  contact length + 1.0 mm (0.8 mm outward, 0.2 mm inward), centred on the tab's contact area.
- **The PCB contact under a coin cell** (a retainer, and a holder whose drawing shows the cell's
  `-` touching the board): one round pad on pin `-`, the diameter the drawing gives, else the coin
  diameter − 4 mm (so the pad clears the holder's `+` tabs), never larger than the drawing's
  "keep-out" when it gives one.
- **Retention pegs / bosses:** non-plated holes (`"pin": 0`, `"plated": false`) of the drawing's
  diameter; a soldered retention tab with no polarity is a plated unnumbered pad (`"pin": 0`).
- **Body:** the holder's outline (`body` width × height), so the app's courtyard and silkscreen
  cover it.

### 3.3 `merge-fp.py`

`python3 -I planning/plan-79-data/merge-fp.py <footprints-<maker>.json>`: adds each footprint whose
id is not in `Parts/footprints.json` (checked against `planning/plan-*-data/footprints-*.json` too:
an id another plan adds is left to that plan, and the run prints it), keeping the file's id order
(the new ids after the last one, in the input's order). A second run changes nothing; `--check`
exits 1 when a footprint of the input is missing or differs.

### 3.4 `gen-battery.py`

Reads `specs.tsv` and `verdicts.tsv` and writes the devices of `Parts/connector.json` whose ids
start with `battery-`, dropping and rewriting only those blocks (plan-45's precedent); every other
block stays byte for byte. `--check` exits 1 when the file differs from what it would write.

- **Device** per (maker, drawer leaf, function, cell list, cells, pin names):
  id `battery-<maker>-<function>-<cells joined>-<n>` (`battery-keystone-holder-coin20-1`), with
  `"function"` per plan-79-C § 4, `"description"` from the maker's title ("Coin cell holder,
  20 mm, SMD"), `"drawer"` `["Connector", "Battery", "Coin"]` (or `"Cylindrical"`, `"9 V"`), the
  `"battery"`, `"cell"`, `"coinDiameter"`, `"cells"` keys, and one pinout per pin naming:
  `two-pin` (`1` `+`, `2` `-`), `contact` (`1` the contact's name), `cells<k>` (`1+`, `1-` …).
- **Part:** `name`, `manufacturer` (`Keystone Electronics`, `MPD`, `Adam Tech`,
  `Assmann WSW Components`, DigiKey's spelling), `datasheet` (the `datasheets.json` id), `package`
  (`Through hole` or `SMD`), `pinout`, `footprint`; `"flags": ["obsolete"]` for an obsolete part
  of a PDF that also covers active parts (AGENTS.md).
- **Drawer leaf:** `Coin` for `cell` `coin`; `9 V` for `9V`; `Cylindrical` for every other cell.

### 3.5 Datasheets

`datasheets.json` gets `{"id": <file stem, lower case>, "url": <the maker's URL>}` per cited PDF
(the maker's own host, never `mm.digikey.com`: `docs/parts-format.md` § Datasheets). A Keystone
layout page is cited in the footprint `note`, not by a part.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Tools

- [ ] `planning/plan-79-data/merge-fp.py` per § 3.3; `gen-battery.py` per § 3.4.
- [ ] Both run on an empty input with no change: `git diff --stat schematical/schematical/Parts` → empty.

Acceptance: the generators are idempotent and touch nothing else.
  Check: `python3 -I planning/plan-79-data/gen-battery.py --check && python3 -I planning/plan-79-data/merge-fp.py --check planning/plan-79-data/footprints-keystone.json` → exit 0 on the empty inputs (est. < 1 min).
  Result:
Commit:

### Phase 2 — Specs and footprints

- [ ] Read each Keystone coin drawing (`drawings.tsv` kind) and layout page; fill `specs.tsv` per § 3.1.
- [ ] `footprints-keystone.json` per § 3.2; merge; regenerate `footprints.txt`.

Acceptance: every new footprint expands and matches its drawing.
  Check: `cmake --build /tmp/logic-p79 -j8 && /tmp/logic-p79/footprint_check` → passes after
  `footprint_check --print > backend/tests/golden/footprints.txt`; `git diff backend/tests/golden/footprints.txt | grep -c '^-[^-]'` → 0 (only added lines). Hand check: for two footprints (one SMD, one
  through hole), every pad's x, y, size and drill in its new `footprints.txt` line equals the
  number its `note` cites from the layout figure; record the two ids and the compared numbers here
  (est. 10 min).
  Result:
Commit:

### Phase 3 — Parts

- [ ] `datasheets.json` per § 3.5; `gen-battery.py --write`.
- [ ] Regenerate `parts.txt`, `traces.txt`, `erc_pins.txt`.

Acceptance: the parts load and the goldens grew by exactly the new parts.
  Check: `ctest --test-dir /tmp/logic-p79 -R 'listing_check|trace_check|erc_pins_check|footprint_check|library_check|connector_test'` → `100% tests passed`; `git diff backend/tests/golden/parts.txt | grep -c '^-[^-]'` → 0 and `grep -c '|Connector/Battery/' backend/tests/golden/parts.txt` → the `add` count of this letter (est. 5 min).
  Result:
Commit:

## Validation Plan

- Phase checks above; `node scripts/check-datasheets.js --host keyelco.com` → every new entry
  answers with a PDF (est. 2 min).
- Final gate: plan-79-J.

## Decisions

- Author: **footprints from the maker's layout first, then fixed fallback rules.** A holder with no
  layout figure still gets a footprint from its tab and pin dimensions, by the rules of § 3.2,
  instead of a skip; each fallback is named in the footprint `note`.
- Author: **the coin cell's PCB contact is a pad of the footprint.** A retainer has no `-` metal of
  its own; the board is the contact, so the land pattern must carry it.
- Author: **one generator for all four makers**, so E–H only fill `specs.tsv`.

## Corrections

## Summary

Keystone's coin cell parts with drawing-true footprints, and the footprint rules and generators the
other battery letters reuse.
